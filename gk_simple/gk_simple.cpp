// Do not remove the include below
#include "gk_simple.h"
#include "decode433.h"
#include "toolbox.h"
#include "commsyms.h"
#include <serialout.h>
#include <eepromdb.h>
#include "sdfatlogwriter.h"
#include "globals.h"
#include <I2C.h>
#include <ds3231.h>

#define VERBOSE

ts			g_dt;
char		g_iobuf[32];
uint8_t		g_inidx(0);
uint16_t	g_lastcheckpoint;

eepromdb	g_db;

void processinput();



//////////////////////////////////////////////////////////////////////////////
void setup()
{
	bool loginit(false);

#ifdef VERBOSE
	Serial.begin( BAUDRATE );
	delay(10);
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();
#endif

	pinMode( PIN_GATE, OUTPUT );
	digitalWrite( PIN_GATE, RELAY_OFF );
	setup433();
	g_codeready = false;
	g_code = 0;

	for( uint8_t pin = 4; pin <10; ++pin ) {
		pinMode( pin, OUTPUT );
		digitalWrite( pin, RELAY_OFF );
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
			digitalWrite( 5, RELAY_ON );
			delay( 500 );
			digitalWrite( 5, RELAY_OFF );
			delay( 500 );
		}
	}

	I2c.begin();
	DS3231_init( DS3231_INTCN );
	DS3231_get( &g_dt );
	g_logger.log( logwriter::INFO, g_dt, F("Reset") );

	digitalWrite( IN_YELLOW, RELAY_ON );
	delay( 500 );
	digitalWrite( OUT_YELLOW, RELAY_ON );

	g_db.init();
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	static uint16_t	code(0);
	static uint8_t	cnt(0);

	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();

	if( g_codeready )
	{
		uint16_t	id( getid( g_code ));
		uint8_t		btn( getbutton( g_code ));

		if( code != g_code ) {
			if( cnt )
			{
				DS3231_get( &g_dt );
				g_logger.log( logwriter::DEBUG, g_dt, F("Abort"), id, btn );
#ifdef VERBOSE
				Serial.print( F("Aborting ") );
				Serial.print( code );
				Serial.print( ' ' );
				Serial.println( g_code );
#endif
			}
			code = g_code;
			cnt = 0;
		} else if( cnt++ > 2 ) {
			database::dbrecord	rec;

#ifdef VERBOSE
			Serial.print(' ');
			Serial.print( id );
#endif
			DS3231_get( &g_dt );
			g_db.getParams( id, rec );
			bool enabled(rec.enabled());

			digitalWrite( IN_YELLOW, RELAY_OFF );
			digitalWrite( OUT_YELLOW, RELAY_OFF );

#ifdef __HARD__
			digitalWrite( enabled? IN_GREEN : IN_RED, RELAY_ON );
			digitalWrite( enabled? OUT_GREEN : OUT_RED, RELAY_ON );

			if( rec.enabled() )
			{
#ifdef VERBOSE
				Serial.print(CMNT);
				Serial.println(F(" -> opening gate."));
#endif
				if()
				digitalWrite( PIN_GATE, RELAY_ON );
				g_logger.log( logwriter::INFO, g_dt, F("Ack"), id, btn );
				delay(1000);
				digitalWrite( PIN_GATE, RELAY_OFF );
				delay(20000);
			} else {
#ifdef VERBOSE
				Serial.print(CMNT);
				Serial.println(F(" -> ignoring."));
#endif
				digitalWrite( IN_RED, RELAY_ON );
				digitalWrite( OUT_RED, RELAY_ON );
				g_logger.log( logwriter::INFO, g_dt, F("Deny"), id, btn );
				delay(2000);
			}

			digitalWrite( enabled ? IN_GREEN : IN_RED, RELAY_OFF );
			digitalWrite( enabled ? OUT_GREEN : OUT_RED, RELAY_OFF );
#else
			bool idok( id >= ID_MIN && id <= ID_MAX );

			digitalWrite( (enabled && idok) ? IN_GREEN : IN_RED, RELAY_ON );
			digitalWrite( (enabled && idok) ? OUT_GREEN : OUT_RED, RELAY_ON );
			if( idok )
			{
#ifdef VERBOSE
				Serial.print(CMNT);
				Serial.println(F(" -> opening gate."));
#endif	//	VERBOSE
				digitalWrite( PIN_GATE, RELAY_ON );
				g_logger.log( logwriter::INFO, g_dt, enabled ? F("Ack") : F("Deny"), id, btn );
				delay(1000);
				digitalWrite( PIN_GATE, RELAY_OFF );
				delay(20000);
			}
			else
			{
#ifdef VERBOSE
				Serial.println(F(" -> Range"));
#endif	//	VERBOSE
				digitalWrite( IN_RED, RELAY_ON );
				digitalWrite( OUT_RED, RELAY_ON );
				g_logger.log( logwriter::INFO, g_dt, F("Range"), id, btn );
				delay(2000);
			}
			digitalWrite( (enabled && idok) ? IN_GREEN : IN_RED, RELAY_OFF );
			digitalWrite( (enabled && idok) ? OUT_GREEN : OUT_RED, RELAY_OFF );
#endif	//	__HARD__
			digitalWrite( IN_YELLOW, RELAY_ON );
			digitalWrite( OUT_YELLOW, RELAY_ON );
			cnt = 0;
		}
#ifdef VERBOSE
		else
		{
			if(cnt == 1) Serial.print(CMNT);
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

	} else {
		Serial.println( F(ERRS "CMD"));
	}
	g_inidx = 0;
}

