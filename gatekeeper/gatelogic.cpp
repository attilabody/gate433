#include "config.h"
#include <Wire.h>
#include <ds3231.h>
#include "globals.h"
#include "interface.h"
#include "serialbuf.h"
#include "decode433.h"
#include "gatelogic.h"


#include "gatehandler.h"
#include "intdb.h"


enum SYSSTATUS { INHIBITED, CODE, PASS };

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
void setup()
{
	Serial.begin( BAUDRATE );


#ifdef PIN_LED
	pinMode( PIN_LED, OUTPUT );
#endif
	pinMode( PIN_RFIN, INPUT );

	g_lcd.init();
	g_lcd.backlight();

	if( g_sd.begin( SS ))
		g_dbinitfail = !g_db.init();

	if( g_dbinitfail ) {
		g_lcd.print( "DB init FAILED!!" );
	}

	g_indloop.init( PIN_INNERLOOP, PIN_OUTERLOOP, LOW );
	g_lights.init( g_innerlightspins, g_outerlightspins, RELAY_ON == HIGH, 500 );
	setuprelaypins( g_otherrelaypins, sizeof(g_otherrelaypins));

#ifdef PIN_LED
	noInterrupts();
	// disable all interrupts
	TIMSK0 |= ( 1 << OCIE0A );  // enable timer compare interrupt
	interrupts();
	// enable all interrupts
#endif	//	PIN_LED

	attachInterrupt( digitalPinToInterrupt( PIN_RFIN ), isr, CHANGE );
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
//	if( getlinefromserial( g_inbuf, sizeof(g_inbuf ), g_inidx) )
//		processInput();
	static gatehandler				handler( g_db, g_lights, g_indloop, g_lcd, ENFORCE_POS, ENFORCE_DT );

	handler.loop( millis() );
}

#ifdef PIN_LED
//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect ) {
	digitalWrite( PIN_LED, ( micros() - g_codetime < 500000 ) ? HIGH : LOW );
}
#endif

