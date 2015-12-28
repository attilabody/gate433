#include <config.h>
#include <Wire.h>
#include <ds3231.h>
#include <interface.h>
#include "serialbuf.h"
#include "decode433.h"
#include "inductiveloop.h"
#include "trafficlights.h"
#include "gatelogic.h"


#include "gatehandler.h"
#include "intdb.h"


intdb		g_db;
gatehandler	g_gatehadler( g_db, true );

uint8_t		g_innerlightspins[3] = INNER_LIGHTS_PINS;
uint8_t		g_outerlightspins[3] = OUTER_LIGHTS_PINS;
uint8_t		g_otherrelaypins[] = { PIN_GATE, PIN_RELAY_SPARE };

trafficlights	g_lights;
inductiveloop	g_indloop( PIN_INNERLOOP, PIN_OUTERLOOP, LOW );

enum SYSSTATUS { INHIBITED, CODE, PASS };

//////////////////////////////////////////////////////////////////////////////
void setuprelaypins( uint8_t *pins, uint8_t size )
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
#ifdef USE_DS3231
#ifdef VERBOSE
	delay(100);
	Serial.println("Initializing DS3231");
#endif
	Wire.begin();
	DS3231_init( DS3231_INTCN );
#ifdef VERBOSE
	Serial.println("Done.");
#endif
#endif	//	USE_DS3231

#ifdef PIN_LED
	pinMode( PIN_LED, OUTPUT );
#endif
	pinMode( PIN_RFIN, INPUT );

	g_lights.init( g_innerlightspins, g_outerlightspins, RELAY_ON == HIGH, 500 );
	setuprelaypins( g_otherrelaypins, sizeof(g_otherrelaypins));

	noInterrupts();
	// disable all interrupts
	TIMSK0 |= ( 1 << OCIE0A );  // enable timer compare interrupt
	interrupts();
	// enable all interrupts

#if defined(VERBOSE) && defined(USE_DS3231)
	ts t;
	DS3231_get( &t );
	datetimetoserial( t );
#endif	//	defined(VERBOSE) && defined(USE_DS3231)
#ifdef FAILSTATS
	memset( (void*) &g_stats, sizeof( g_stats ), 0 );
#endif

	attachInterrupt( digitalPinToInterrupt( PIN_RFIN ), isr, CHANGE );
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
#ifdef FAILSTATS
	static stats prevstats;
	static stats *pp, *ps;
#endif

	if( g_codeready ){
		//process received info here
		g_gatehadler.codereceived( g_code >> 2, false );
		g_codeready = false;
	}
#ifdef FAILSTATS
	else
	{
		ps = (stats*)&g_stats;
		if( !(prevstats == *ps) )
		{
			String s( String( g_stats.startabort )
					+ String( " " ) + String( g_stats.dataabort )
					+ String( " " ) + String( g_stats.stopabort )
					+ String( " " ) + String( g_stats.stopdeltat )
			);
			Serial.println( s );
			prevstats = *ps;
		}
	}
#endif	//	FAILSTATS

//	if( getlinefromserial( g_inbuf, sizeof(g_inbuf ), g_inidx) )
//		processInput();
	static inductiveloop::STATUS	prevls( inductiveloop::NONE );
	static bool						prevconflict( false );

	inductiveloop::STATUS			ls;
	bool							conflict( false );

	conflict = g_indloop.update( ls );
	if( ls != prevls || conflict != prevconflict )
	{
		serialoutlncs( (int) ls, conflict );
		prevls = ls;
		prevconflict = conflict;
	}

	g_lights.loop();
}

//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect ) {
#ifdef PIN_LED
	digitalWrite( PIN_LED, ( micros() - g_codetime < 500000 ) ? HIGH : LOW );
#endif
}

