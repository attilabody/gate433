#include <Arduino.h>
#include <ds3231.h>
#include <SdFat.h>
#include <MemoryFree.h>
#include "config.h"
#include "globals.h"
#include "toolbox.h"
#include "dthelpers.h"
#include <commsyms.h>
#include "serialout.h"
#include "serialbuf.h"
#include "decode433.h"
#include "gatehandler.h"
#include "intdb.h"
#include "sdfatlogwriter.h"

//////////////////////////////////////////////////////////////////////////////
void setuprelaypins( const uint8_t *pins, uint8_t size );
void processinput();
int getlinefromfile( SdFile &f, char* buffer, uint8_t buflen );
void updatedow( ts &t );
uint16_t importdb(uint16_t start, uint16_t end);

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( BAUDRATE );
#ifdef VERBOSE
	delay(10);
	Serial.print( CMNT );
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();
#endif

	memset( &g_time, 0, sizeof( g_time ));

	I2c.begin();
	I2c.timeOut(1000);

#ifndef USE_IOEXTENDER_OUTPUTS
	const uint8_t	g_alloutputpins[8] = { ALL_RAW_OUTPUT_PINS };
	g_outputs.init(g_alloutputpins, RELAY_OFF);
#endif
	g_display.init();		//	calls Wire.begin()

	g_display.print( freeMemory() );

	g_sdpresent = g_sd.begin( SS );
	g_display.print( ' ' );
	g_display.print( g_sdpresent );

#ifdef USE_FLASHDB
	bool dbinitsucc = g_db.init();
#else
	bool dbinitsucc = g_sdpresent && g_db.init();
#endif

	g_display.print( ' ' );
	g_display.print( dbinitsucc );

	g_indloop.init( PIN_INNERLOOP, PIN_OUTERLOOP, LOW );

	noInterrupts();
	TIMSK0 |= ( 1 << OCIE0A );  // enable timer compare interrupt
	interrupts();

	setup433();

	g_clk.init( DS3231_INTCN );
	g_lastdtupdate = millis();
	g_timevalid = g_clk.get( &g_time ) == 0;
	updatedow( g_time );

	bool loginitsucc = g_sdpresent && g_logger.init();
	g_display.print( ' ' );
	g_display.print( loginitsucc );

	g_outputs.write8(0xf8);
	delay(500);
	g_outputs.write8(0x37);
	delay(500);
	g_outputs.write8(0xff);

	g_display.clear();
	if(g_sdpresent)
	{
		SdFile	f;
		if(f.open("IMPORT"))
		{
			f.close();
			g_display.print(F("IMPORTING "));
			uint16_t	imported(importdb(0, 1023));
			if(imported != (uint16_t) -1) {
				g_display.print(imported);
				g_sd.remove("IMPORT");
			} else
				g_display.print(F("FAIL"));

			delay(2000);
			g_display.clear();
		}
	}

	g_logger.log( logwriter::WARNING, g_time, F("Restart"), -1 );
	g_display.clear();
	g_display.updatedt( g_time, 0xff, g_timevalid );
	g_display.updateloopstatus( false, false );
	g_display.updatelastreceivedid( 9999 );
	g_display.updatelastdecision( 'X', 9999 );

#ifdef VERBOSE
	Serial.begin( BAUDRATE );
	delay(10);
	Serial.print( CMNT );
	for( char c = 0; c < 79; ++c ) Serial.print('<');
	Serial.println();
#endif
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	static gatehandler	handler( g_db, g_lights, g_indloop, g_display, 500 );
	unsigned long		now( millis() );
	uint8_t				dtcmask(0);
	bool				timevalid;
	bool				tvchanged;

	CHECKPOINT;

	if( now - g_lastdtupdate > 980 )
	{
		ts	t;
		timevalid = g_clk.get( &t ) == 0;
		tvchanged = timevalid != g_timevalid;
		if(tvchanged){
			g_logger.log( timevalid ? logwriter::INFO : logwriter::ERROR, g_time, timevalid ? F("TV") : F("TIV"), -1 );
			g_timevalid = timevalid;
		}

		if( t.sec != g_time.sec )
			dtcmask |= 1;
		if( t.min != g_time.min )
			dtcmask |= 2;
		if( t.hour != g_time.hour )
			dtcmask |= 4;
		if( t.mday != g_time.mday )
			dtcmask |= 8;
		if( t.mon != g_time.mon )
			dtcmask |= 0x10;
		if( t.year != g_time.year )
			dtcmask |= 0x20;
		if( tvchanged )
			dtcmask |= 0x40;

		if( dtcmask & 0x38 ) {
				g_time = t;
				updatedow( g_time );
		} else if( dtcmask &7) {
			g_time.sec = t.sec;
			g_time.min = t.min;
			g_time.hour = t.hour;
		}
		if( dtcmask ) {
			g_lastdtupdate = now;
			g_display.updatedt( g_time, dtcmask, g_timevalid );
		}
	}

	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();

	handler.loop( now );

	if( g_codedisplayed != g_lrcode ) {
		uint16_t	id( getid( g_lrcode ));
		g_display.updatelastreceivedid( getid( id ));
		g_logger.log( logwriter::DEBUG, g_time, F("NewCode"), id );
		g_codedisplayed = g_lrcode;
	}
}

//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect )
{
	if( ++g_lastcheckpoint > 5000 ) {
		pinMode( PIN_RESET, OUTPUT );
		digitalWrite( PIN_RESET, LOW );
	}
}

//////////////////////////////////////////////////////////////////////////////
void setuprelaypins( const uint8_t *pins, uint8_t size )
{
	while( size > 0 ) {
		pinMode( *pins, OUTPUT );
		digitalWrite( *pins++, RELAY_OFF );
		--size;
	}
}

//////////////////////////////////////////////////////////////////////////////
void printrespok() {
	Serial.println( F( RESPS "OK"));
}

//////////////////////////////////////////////////////////////////////////////
void printresperr() {
	Serial.println( F( ERRS "ERR"));
}

//////////////////////////////////////////////////////////////////////////////
const char PROGMEM CMD_GET[] = "get";	//	get db record
const char PROGMEM CMD_SET[] = "set";	//	set db record
const char PROGMEM CMD_EXP[] = "edb";	//	export database
const char PROGMEM CMD_IMP[] = "idb";	//	import database
const char PROGMEM CMD_DMP[] = "ddb";	//	dump database
const char PROGMEM CMD_GDT[] = "gdt";	//	get datetime
const char PROGMEM CMD_SDT[] = "sdt";	//	set datetime
const char PROGMEM CMD_CS[] = "cs";		//	clear statuses
const char PROGMEM CMD_DS[] = "ds";		//	dump shuffle
const char PROGMEM CMD_DL[] = "dl";		//	dump log
const char PROGMEM CMD_TL[] = "tl";		//	reuncate log
const char PROGMEM CMD_IL[] = "il";		//	infinite loop


//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.print( CMNT );
	Serial.println( g_iobuf );

	if( iscommand( inptr, CMD_GET )) {	//	get
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && g_db.getParams( id, rec )) {
			rec.serialize( g_iobuf );
			serialoutln( RESP, g_iobuf );
		} else printresperr();

	} else if( iscommand( inptr, CMD_SET )) {	//	set
		database::dbrecord	rec;
		int 				id( getintparam( inptr ));
		if( id != -1 && rec.parse( inptr )) {
			if( g_db.setParams( id, rec ))
				printrespok();
			else printresperr();
		} else printresperr();

	} else if( iscommand( inptr, CMD_EXP )) {	//	export
		thindb		tdb( g_sd );
		uint16_t	from( getintparam( inptr ));
		uint16_t	to( getintparam( inptr ));
		uint16_t	id(0);
		database::dbrecord	rec;
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		if( tdb.init()) {
			for( id = from; id <= to; ++id ) {
				CHECKPOINT;
				if( !g_db.getParams( id, rec ) || !tdb.setParams( id, rec ))
					break;
			}
		}
		if( id == to + 1 )  printrespok();
		else printresperr();

	} else if( iscommand( inptr, CMD_IMP)) {	//	import
		uint16_t	from( getintparam( inptr ));
		uint16_t	to( getintparam( inptr ));
		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		uint16_t	imported(importdb(from, to));
		if(imported != (uint16_t)-1) {
			Serial.print(F(RESPS "OK "));
			Serial.println(imported);
		}
		else printresperr();

	} else if( iscommand( inptr, CMD_DMP )) {	//	dump
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
		if( id == to + 1 ) printrespok();
		else printresperr();

	} else if( iscommand( inptr, CMD_CS )) {	//	clear statuses
		uint16_t			from( getintparam( inptr ));
		uint16_t			to( getintparam( inptr ));
		uint16_t			id;

		if( from == 0xffff ) from = 0;
		if( to == 0xffff ) to = 1023;
		for( id = from; id <= to; ++id ) {
			CHECKPOINT;
			g_db.setStatus( id, database::dbrecord::UNKNOWN );
		}
		printrespok();

	} else if( iscommand( inptr, CMD_GDT )) {	// get datetime
		serialout( RESP, (uint16_t)g_time.year, F("."));
		serialout( (uint16_t)g_time.mon,F("."));
		serialout( (uint16_t)g_time.mday, F("/" ),(uint16_t)g_time.wday);
		serialout( F("    "), (uint16_t)g_time.hour);
		serialout(F(":" ), (uint16_t)(g_time.min));
		serialoutln(F(":" ), (uint16_t)(g_time.sec));

	} else if( iscommand( inptr, CMD_SDT )) {	//	set datetime
		ts	t;
		if( parsedatetime( t, inptr )) {
			g_clk.set( t );
			printrespok();
		} else
			Serial.println( F(ERRS " DTFMT"));

	} else if( iscommand( inptr, CMD_DS )) {	// dump shuffle
		SdFile	f;
		char buffer[16];
		if( f.open( "SHUFFLE.TXT", FILE_READ )) {
			while( getlinefromfile( f, buffer, sizeof(buffer)) != -1 ) {
				CHECKPOINT;
				serialoutln( RESP, buffer );
			}
			Serial.println( RESP );
			f.close();
		} else {
			Serial.println( F(ERRS "CANTOPEN"));
		}

	} else if( iscommand( inptr, CMD_DL )) {	// dump log
		g_logger.dump( &Serial );
		printrespok();

	} else if( iscommand( inptr, CMD_TL )) {	// truncate log
		g_logger.truncate();
		printrespok();

	} else if( iscommand( inptr, CMD_IL )) {	// infinite loop
		while( true ) {
			Serial.println( g_lastcheckpoint );
			delay(500);
		}

	} else {
		Serial.println( F(ERRS "CMD"));
	}

	g_inidx = 0;
}

//////////////////////////////////////////////////////////////////////////////
int getlinefromfile( SdFile &f, char* buffer, uint8_t buflen )
{
	uint32_t	curpos( f.curPosition());
	char *src( buffer );
	int rb( f.read( buffer, buflen - 1 ));
	if( !rb ) return -1;

	int ret(0);

	while( rb-- ) {
		char inc = *src;
		if( !inc || inc == '\r' || inc == '\n' ) {
			ret = src - buffer;
			*src++ = 0;
			if( inc && rb ) {
				inc = *src;
				if( !inc || inc == '\r' || inc == '\n' )
					++src;
			}
			break;
		}
		++src;
	}

	if( rb == -1 ) {
		*src = 0;
		ret = src - buffer;
	}
	f.seekSet( curpos + ( src - buffer ));
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
void updatedow( ts &t )
{
	char buffer[12];
	const char *bp;
	uint8_t	month, day, dow;
	SdFile	f;

	if( !g_sdpresent ) return;

	if( f.open( "SHUFFLE.TXT", FILE_READ )) {
		while( getlinefromfile( f, buffer, sizeof(buffer)) != -1 ) {
			bp = buffer;
			if( (month = getintparam( bp, true, true, false )) != 0xff &&
				(day = getintparam( bp, true, true, false )) != 0xff &&
				(dow = getintparam( bp, true, true, false )) != 0xff &&
				month == g_time.mon && day == g_time.mday )
			{
				g_time.wday = dow;
				break;
			}
		}
		f.close();
	}
}

//////////////////////////////////////////////////////////////////////////////
uint16_t importdb(uint16_t start, uint16_t end)
{
	if(end > 1023) return -1;

	thindb				tdb( g_sd );
	uint16_t			id(-1);
	database::dbrecord	rec,old;
	uint16_t			imported(0);
	if( tdb.init()) {
		for( id = start; id <= end; ++id ) {
			if( !tdb.getParams( id, rec ) || !g_db.getParams(id, old)) {
				break;
			}
			if(!rec.infoequal(old)){
#ifdef VERBOSE
				serialoutln(F(CMNTS "Importing "), id);
#endif
				if(!g_db.setParams( id, rec ))
					break;
				else
					++imported;
			}
#ifdef VERBOSE
			else {
				serialoutln(F(CMNTS "Skipping "), id);
			}
#endif
		}
		if(id != end+1 ) return -1;
	}
	return imported;
}
