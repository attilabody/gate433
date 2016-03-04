// Do not remove the include below
#include "config.h"
#include "gk_simple.h"
#include "decode433.h"
#include "toolbox.h"
#include "commsyms.h"
#include <serialout.h>
#include <eepromdb.h>
#include <thindb.h>
#include "sdfatlogwriter.h"
#include "globals.h"
#include "inductiveloop.h"
#include <Wire.h>
#include <ds3231.h>

ts				g_dt;
char			g_iobuf[32];
uint8_t			g_inidx(0);
uint16_t		g_lastcheckpoint;

eepromdb		g_db;
inductiveloop	g_loop;

void processinput();



//////////////////////////////////////////////////////////////////////////////
void setup()
{
	bool loginit(false);

	uint8_t	tlpins[] = {
			IN_GREEN, IN_YELLOW, IN_RED,
			OUT_GREEN, OUT_YELLOW, OUT_RED
	};

	Serial.begin( BAUDRATE );
#ifdef VERBOSE
	delay(10);
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();
#endif

	pinMode( PIN_GATE, OUTPUT );
	digitalWrite( PIN_GATE, RELAY_OFF );

	g_loop.init( PIN_INNERLOOP, PIN_OUTERLOOP, LOOP_ACTIVE );

	setup433();
	g_codeready = false;
	g_code = 0;

	//setting up traffic lights pins
	for( uint8_t pin = 0; pin < sizeof(tlpins) + 3; ++pin ) {
		if(pin < sizeof(tlpins)) {
			pinMode( tlpins[pin], OUTPUT );
			digitalWrite(tlpins[pin], RELAY_ON);
		}
		if(pin > 2)
			digitalWrite(tlpins[pin-3], RELAY_OFF);
		delay(150);
	}

	if( g_sd.begin( SS, SPI_HALF_SPEED )) {
		if( !(loginit = g_logger.init()) )
			Serial.println(F("Logger fail"));
	} else
		Serial.println(F("SD fail"));

	if( !loginit )
	{
		delay(100);
		for( int i = 0; i < 3;  ++i ) {
			digitalWrite( IN_RED, RELAY_ON );
			digitalWrite( OUT_RED, RELAY_ON );
			delay( 500 );
			digitalWrite( IN_RED, RELAY_OFF );
			digitalWrite( OUT_RED, RELAY_OFF );
			delay( 500 );
		}
	}

	Wire.begin();
	DS3231_init( DS3231_INTCN );
	DS3231_get( &g_dt );
	g_logger.log( logwriter::INFO, g_dt, F("Reset") );
	g_db.init();
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

	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();

	conflict = g_loop.update( ils );

	if( ils != previls || conflict != prevconflict )
	{
		if( ils == inductiveloop::NONE ) {
			digitalWrite( IN_YELLOW, RELAY_OFF );
			digitalWrite( OUT_YELLOW, RELAY_OFF );
			digitalWrite( IN_RED, RELAY_OFF );
			digitalWrite( OUT_RED, RELAY_OFF );
		}
		else if( ils == inductiveloop::INNER ) {
			digitalWrite( IN_YELLOW, RELAY_ON );
			digitalWrite( OUT_RED, RELAY_ON );
			digitalWrite( IN_RED, RELAY_OFF );
			digitalWrite( OUT_YELLOW, RELAY_OFF );
		} else {
			digitalWrite( IN_RED, RELAY_ON );
			digitalWrite( OUT_YELLOW, RELAY_ON );
			digitalWrite( IN_YELLOW, RELAY_OFF );
			digitalWrite( OUT_RED, RELAY_OFF );
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
			g_db.getParams( id, rec );
			bool enabled(rec.enabled());

			digitalWrite( inner ? IN_YELLOW : OUT_YELLOW, RELAY_OFF );
			digitalWrite( enabled? (inner ? IN_GREEN : OUT_GREEN) : (inner ? IN_RED : OUT_RED), RELAY_ON );

			unsigned long timeout;
			if( rec.enabled() )
			{
				g_logger.log( logwriter::INFO, g_dt, F("Ack"), id, btn );
#ifdef VERBOSE
				Serial.println(F(" -> opening gate."));
#endif
				digitalWrite( PIN_GATE, RELAY_ON );
				delay(1000);
				digitalWrite( PIN_GATE, RELAY_OFF );
				timeout = 60000;

			} else {
				g_logger.log( logwriter::INFO, g_dt, F("Deny"), id, btn );
#ifndef __HARD__
				digitalWrite( PIN_GATE, RELAY_ON );
				delay(1000);
				digitalWrite( PIN_GATE, RELAY_OFF );
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

			digitalWrite( enabled ? (inner ? IN_GREEN : OUT_GREEN) : (inner ? IN_RED : OUT_RED), RELAY_OFF );
			digitalWrite( inner ? OUT_RED : IN_RED, RELAY_OFF );

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

