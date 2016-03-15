// Do not remove the include below
#include "config.h"
#include "globals.h"
#include "gk_simple.h"
#include "decode433.h"
#include "toolbox.h"
#include "commsyms.h"
#include <serialout.h>
#include "sdfatlogwriter.h"
#include "inductiveloop.h"
#include <I2C.h>
#include <ds3231.h>
#include "dthelpers.h"
#include "display.h"
#include <MemoryFree.h>

ts				g_dt;
char			g_iobuf[32];
uint8_t			g_inidx(0);
uint16_t		g_lastcheckpoint;

void processinput();
inline void updateloopstatus(inductiveloop::STATUS status, bool conflict) {
	g_display.updateloopstatus(conflict || status == inductiveloop::INNER, conflict || status == inductiveloop::OUTER );
}

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	bool loginit(false);
	bool sdinit(false);
	bool dbinit(false);

	uint8_t	tlpins[] = { INNER_LIGHTS_PINS, OUTER_LIGHTS_PINS };

	Serial.begin( BAUDRATE );
#ifdef VERBOSE
	delay(10);
	Serial.print(CMNT);
	for( char c = 0; c < 79; ++c ) Serial.print('>');
	Serial.println();
#endif
	I2c.begin();
	I2c.timeOut(1000);

	g_display.init();		//	calls Wire.begin()

	g_display.print( freeMemory() );
	g_loop.init( PIN_INNERLOOP, PIN_OUTERLOOP, LOOP_ACTIVE );

	setup433();
	g_codeready = false;
	g_code = 0;

#ifdef USE_IOEXTENDER_OUTPUTS
	g_outputs.write8(0xff);
#else
	{
		uint8_t	all_output_pins[8] = { ALL_RAW_OUTPUT_PINS };
		g_outputs.init(all_output_pins, RELAY_OFF);
	}
#endif
	if((sdinit = g_sd.begin( SS, SPI_HALF_SPEED ))) {
		if( !(loginit = g_logger.init()) )
			Serial.println(F("Logger fail"));
	} else
		Serial.println(F("SD fail"));

	dbinit = g_db.init();

	g_display.print( ' ' );
	g_display.print( sdinit );
	g_display.print( ' ' );
	g_display.print( loginit );
	g_display.print( ' ' );
	g_display.print( dbinit );

	//setting up traffic lights pins
	for( uint8_t pin = 0; pin < sizeof(tlpins) + 3; ++pin ) {
		if(pin < sizeof(tlpins)) {
			g_outputs.write( tlpins[pin], RELAY_ON);
		}
		if(pin > 2)
			g_outputs.write(tlpins[pin-3], RELAY_OFF);
		delay(150);
	}

	if( !loginit )
	{
		delay(100);
		for( int i = 0; i < 3;  ++i ) {
			g_outputs.write( PIN_IN_RED, RELAY_ON );
			g_outputs.write( PIN_OUT_RED, RELAY_ON );
			delay( 500 );
			g_outputs.write( PIN_IN_RED, RELAY_OFF );
			g_outputs.write( PIN_OUT_RED, RELAY_OFF );
			delay( 500 );
		}
	}

	DS3231_init( DS3231_INTCN );
	DS3231_get( &g_dt );
	g_logger.log( logwriter::INFO, g_dt, F("Reset") );

	g_display.clear();

#ifdef VERBOSE
	Serial.print(CMNT);
	for( char c = 0; c < 79; ++c ) Serial.print('<');
	Serial.println();
#endif
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	static uint16_t					code(0);
	static uint8_t					cnt(0);
	static inductiveloop::STATUS	previls(inductiveloop::NONE);
	static bool						prevconflict(false);
	inductiveloop::STATUS			ils;
	bool							conflict;
	bool							dtupdated(false);

	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();

	conflict = g_loop.update( ils );

	if( ils != previls || conflict != prevconflict )
	{
		updateloopstatus(ils, conflict);
#ifdef VERBOSE
		Serial.print(CMNT);
		serialoutsepln(", ", "ilschange", (int)ils, (int)previls, conflict, prevconflict );
#endif	//	VERBOSE
		if( ils == inductiveloop::NONE ) {
			g_outputs.write( PIN_IN_YELLOW, RELAY_OFF );
			g_outputs.write( PIN_OUT_YELLOW, RELAY_OFF );
			g_outputs.write( PIN_IN_RED, RELAY_OFF );
			g_outputs.write( PIN_OUT_RED, RELAY_OFF );
		}
		else if( ils == inductiveloop::INNER ) {
			g_outputs.write( PIN_IN_YELLOW, RELAY_ON );
			g_outputs.write( PIN_OUT_RED, RELAY_ON );
			g_outputs.write( PIN_IN_RED, RELAY_OFF );
			g_outputs.write( PIN_OUT_YELLOW, RELAY_OFF );
		} else {
			g_outputs.write( PIN_IN_RED, RELAY_ON );
			g_outputs.write( PIN_OUT_YELLOW, RELAY_ON );
			g_outputs.write( PIN_IN_YELLOW, RELAY_OFF );
			g_outputs.write( PIN_OUT_RED, RELAY_OFF );
		}
		previls = ils;
		prevconflict = conflict;
	}

	if( g_codeready && ils != inductiveloop::NONE )
	{
		uint16_t	id( getid( g_code ));
		uint8_t		btn( getbutton( g_code ));
		bool		inner(ils == inductiveloop::INNER);

		if( code != g_code ) {
			if( cnt )
			{
				DS3231_get( &g_dt );
				dtupdated = true;
				g_logger.log( logwriter::DEBUG, g_dt, F("Abort"), id, btn );
#ifdef VERBOSE
				Serial.print( F("  Aborting ") );
				Serial.print( code );
				Serial.print( ' ' );
				Serial.println( g_code );
#endif
			}
			code = g_code;
			cnt = 0;
		} else if( cnt++ > 1 ) {
			database::dbrecord	rec;

#ifdef VERBOSE
			Serial.print(' ');
			Serial.print( id );
#endif
			DS3231_get( &g_dt );
			dtupdated = true;
			g_db.getParams( id, rec );
			bool enabled(rec.enabled());
			g_display.updatedt(g_dt, 0xff);
			g_display.updatelastdecision( enabled ? '+' : 'X', id );

			g_outputs.write( inner ? PIN_IN_YELLOW : PIN_OUT_YELLOW, RELAY_OFF );
			g_outputs.write( enabled? (inner ? PIN_IN_GREEN : PIN_OUT_GREEN) : (inner ? PIN_IN_RED : PIN_OUT_RED), RELAY_ON );

			unsigned long timeout;
			if( enabled )
			{
				g_logger.log( logwriter::INFO, g_dt, F("Ack"), id, btn );
#ifdef VERBOSE
				Serial.println(F(" -> opening gate."));
#endif
				g_outputs.write( PIN_GATE, RELAY_ON );
				delay(1000);
				g_outputs.write( PIN_GATE, RELAY_OFF );
				timeout = 60000;

			} else {
				g_logger.log( logwriter::INFO, g_dt, F("Deny"), id, btn );
#ifndef ENFORCE
				g_outputs.write( PIN_GATE, RELAY_ON );
				delay(1000);
				g_outputs.write( PIN_GATE, RELAY_OFF );
#endif	//	__HARD__
#ifdef VERBOSE
				Serial.println(F(" -> ignoring."));
#endif
				timeout = 10000;
			}

			inductiveloop::STATUS	ilstmp(ils);
			bool 					conflicttmp(conflict);
			unsigned long			waitstart(millis());

			while(conflict == conflicttmp && ils == ilstmp && millis()-waitstart < timeout )
				conflicttmp = g_loop.update(ilstmp);
			updateloopstatus(ilstmp, conflicttmp );

			if(enabled) {
				g_outputs.write(inner ? PIN_IN_GREEN : PIN_OUT_GREEN, RELAY_OFF);
				if(ilstmp != inductiveloop::NONE)
					g_outputs.write(inner ? PIN_IN_RED : PIN_OUT_RED, RELAY_ON);
			}

			while(ilstmp != inductiveloop::NONE && millis()-waitstart < timeout )
				conflicttmp = g_loop.update(ilstmp);
			updateloopstatus(ilstmp, conflicttmp);

			g_outputs.write( PIN_IN_RED, RELAY_OFF );
			g_outputs.write( PIN_OUT_RED, RELAY_OFF );

			previls = inductiveloop::NONE;
			prevconflict = false;
			cnt = 0;
		}
#ifdef VERBOSE
		else
		{
			if(cnt == 1) Serial.print(CMNT);
			Serial.print(id);
			Serial.print('.');
		}
#endif	//	VERBOSE
		g_codeready = false;
	}

	if( g_codedisplayed != g_lrcode ) {
		if(!dtupdated) {
			DS3231_get(&g_dt);
			dtupdated = true;
		}
		g_display.updatelastreceivedid( getid( g_lrcode ));
		g_logger.log( logwriter::DEBUG, g_dt, F("NewCode"), g_lrcode >> 2);
		g_codedisplayed = g_lrcode;
	}
}

//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.print(CMNT);
	Serial.println( g_iobuf );

	if( iscommand( inptr, F("dl"))) {
		g_logger.dump( &Serial );
	} else if( iscommand( inptr, F("tl"))) {	// truncate log
		g_logger.truncate();
	} else if( iscommand( inptr, F("get") )) {	//	get
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && g_db.getParams( id, rec )) {
			rec.serialize( g_iobuf );
			serialoutln( RESP, g_iobuf );
		} else Serial.println( F(ERRS "ERR"));

	} else if( iscommand( inptr, F("set") )) {	//	set
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && rec.parse( inptr )) {
			if( g_db.setParams( id, rec ))
				Serial.println( F( RESPS "OK"));
			else Serial.println( F(ERRS "ERR"));
		} else Serial.println( F(ERRS "ERR"));

	} else if( iscommand( inptr, F("imp"))) {	//	import
		thindb				tdb( g_sd );
		uint16_t			from( getintparam( inptr ));
		uint16_t			to( getintparam( inptr ));
		uint16_t			id(-1);
		database::dbrecord	rec;
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		if( tdb.init()) {
			for( id = from; id <= to; ++id ) {
				CHECKPOINT;
				if( !tdb.getParams( id, rec ) || !g_db.setParams( id, rec ))
					break;
			}
		}
		if( id == to + 1 )  Serial.println(F(RESPS "OK"));
		else serialoutln( F(ERRS "ERR "), id );

	} else if( iscommand( inptr, F("dmp"))) {	//	dump
		database::dbrecord	rec;
		uint16_t			from( getintparam( inptr ));
		uint16_t			to( getintparam( inptr ));
		uint16_t			id;

		g_iobuf[3] = ' ';
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		for( id = from; id <= to; ++id ) {
			CHECKPOINT;
			if( g_db.getParams( id, rec )) {
				uitohex( g_iobuf, id, 3 );
				rec.serialize( g_iobuf + 4 );
				serialoutln( RESP, g_iobuf );
			} else break;
		}
		if( id == to + 1 ) Serial.println( RESP );
		else Serial.println( F(ERRS "ERR" ));

	} else {
		Serial.println( F(ERRS "CMD"));
	}
	g_inidx = 0;
}

