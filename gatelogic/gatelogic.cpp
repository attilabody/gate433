#include <config.h>
#include <Wire.h>
#include <ds3231.h>
#include <interface.h>
#include "serialbuf.h"
#include "decode433.h"
#include "gatelogic.h"

#include "gatehandler.h"
#include "intdb.h"


#ifdef FAILSTATS
struct stats
{
	stats() {startabort = dataabort = stopabort = stopdeltat = 0;}
	bool operator==( const stats &o ) {
		return startabort == o.startabort && dataabort == o.dataabort && stopabort == o.stopabort;
	}
	bool operator==( stats &o ) {
		return startabort == o.startabort && dataabort == o.dataabort && stopabort == o.stopabort;
	}
	stats& operator=( const stats &o ) {
		startabort = o.startabort; dataabort = o.dataabort; stopabort = o.stopabort; return *this;
	}
	unsigned long startabort, dataabort, stopabort, stopdeltat;
};

volatile stats g_stats;
#endif	//	FAILSTATS

intdb		g_db;
gatehandler	g_gatehadler( g_db );

uint8_t		g_relayports[8] = RELAY_PORTS;
void isr();

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
	pinMode( PIN_INNERLOOP, INPUT );
	pinMode( PIN_OUTERLOOP, INPUT );
	//activating pullups
	digitalWrite( PIN_INNERLOOP, HIGH );
	digitalWrite( PIN_OUTERLOOP, HIGH );

	for( uint8_t pin = 0 ; pin < sizeof(g_relayports); ++pin ) {
		pinMode( g_relayports[pin], OUTPUT);
		digitalWrite( g_relayports[pin], HIGH );
	}

	noInterrupts();
	// disable all interrupts
	TIMSK0 |= ( 1 << OCIE0A );  // enable timer compare interrupt
	interrupts();
	// enable all interrupts5 utan

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
}

//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect ) {
#ifdef PIN_LED
	digitalWrite( PIN_LED, ( micros() - g_codetime < 500000 ) ? HIGH : LOW );
#endif
}

