// Do not remove the include below
#include "gk_simple.h"
#include "decode433.h"
#include "toolbox.h"
#include "commsyms.h"
#include "sdfatlogwriter.h"
#include "globals.h"
#include <ds3231.h>

ts			g_dt;
char		g_iobuf[32];
uint8_t		g_inidx(0);
uint16_t	g_lastcheckpoint;

void processinput();


//The setup function is called once at startup of the sketch
void setup()
{
	bool loginit(false);

	Serial.begin( BAUDRATE );
	delay(10);
	Serial.print( CMNT );
	for( char c = 0; c < 79; ++c ) Serial.print('-');
	Serial.println();

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

	g_logger.log( logwriter::INFO, g_dt, F("Reset") );

	digitalWrite( IN_YELLOW, RELAY_ON );
	delay( 500 );
	digitalWrite( OUT_YELLOW, RELAY_ON );
}

// The loop function is called in an endless loop
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
				g_logger.log( logwriter::DEBUG, g_dt, F("Abort"), id, btn );
				Serial.print( F("Aborting ") );
				Serial.print( code );
				Serial.print( ' ' );
				Serial.println( g_code );
			}
			code = g_code;
			cnt = 0;
		} else if( cnt++ > 3 ) {
			digitalWrite( IN_YELLOW, RELAY_OFF );
			digitalWrite( OUT_YELLOW, RELAY_OFF );

			Serial.print( id );
			if( id > 3 && id < 1020 )
			{
				digitalWrite( IN_GREEN, RELAY_ON );
				digitalWrite( OUT_GREEN, RELAY_ON );
				Serial.println(F(" -> opening gate."));
				digitalWrite( PIN_GATE, RELAY_ON );
				g_logger.log( logwriter::INFO, g_dt, F("Ack"), id, btn );
				delay(1000);
				digitalWrite( PIN_GATE, RELAY_OFF );
				delay(20000);
				digitalWrite( IN_GREEN, RELAY_OFF );
				digitalWrite( OUT_GREEN, RELAY_OFF );
			} else {
				Serial.println(F(" -> ignoring."));
				digitalWrite( IN_RED, RELAY_ON );
				digitalWrite( OUT_RED, RELAY_ON );
				g_logger.log( logwriter::INFO, g_dt, F("Deny"), id, btn );
				delay(3000);
				digitalWrite( IN_RED, RELAY_OFF );
				digitalWrite( OUT_RED, RELAY_OFF );
			}
			digitalWrite( IN_YELLOW, RELAY_ON );
			digitalWrite( OUT_YELLOW, RELAY_ON );
			cnt = 0;
		} else Serial.print('.');
		g_codeready = false;
	}
}

//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.println( g_iobuf );

	if( iscommand( inptr, F("dl"))) {
		g_logger.dump( &Serial );
	} else if( iscommand( inptr, F("tl"))) {	// truncate log
		g_logger.truncate();

	} else {
		Serial.println( F(ERRS "CMD"));
	}
	g_inidx = 0;
}

