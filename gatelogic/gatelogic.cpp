#include "config.h"
#include <Wire.h>
#include <ds3231.h>
#include <interface.h>
#include "serialbuf.h"
#include "gatelogic.h"
#include "gatehandler.h"
#include "extdb.h"


enum RcvState : uint8_t {
	  START
	, DATA
	, STOP
};

volatile bool 		g_codeready( false );
volatile uint16_t 	g_code(0x55aa);
volatile uint32_t 	g_codetime( 0 );
volatile uint32_t 	g_lastedge;

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

extdb		g_db( g_inbuf, sizeof( g_inbuf ));
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
void isr()
{
	static int8_t curbit;
	static uint32_t lastedge( micros() ), curedge;
	static bool lastlevel( digitalRead( PIN_RFIN ) == HIGH ), in;
	static RcvState state( START );
	static uint16_t code, deltat, cyclet;
	static int timediff;
	static uint16_t prevcode( 0 );
	static uint32_t prevcodetime( 0 );

	static uint32_t highdeltat, lowdeltat;

	curedge = micros();
	in = ( digitalRead( PIN_RFIN ) == HIGH );
	deltat = curedge - lastedge;

	switch( state ) {
	case START:
		if( !g_codeready
				&& lastlevel
				&& !in
				&& deltat >= SHORT_MIN_TIME
		        && deltat <= SHORT_MAX_TIME )
		{	// h->l
			state = DATA;
			curbit = code = 0;
		}
#ifdef FAILSTATS
		else
		++g_stats.startabort;
#endif

		break;

	case DATA:
		if( deltat < SHORT_MIN_TIME || deltat > LONG_MAX_TIME ) {
			state = START;
#ifdef FAILSTATS
			++g_stats.dataabort;
#endif
		} else if( in ) { 	//	l->h
			lowdeltat = deltat;
		} else {			//	h->l
			highdeltat = deltat;
			cyclet = highdeltat + lowdeltat;
			timediff = (int)highdeltat - (int)lowdeltat;
			if( timediff < 0 )
				timediff = -timediff;
			if( cyclet < CYCLE_MIN_TIME || cyclet > CYCLE_MAX_TIME
			        || (unsigned int)timediff < ( cyclet >> 4 ) ) {
				state = START;
#ifdef FAILSTATS
				++g_stats.dataabort;
#endif
				break;
			}
			code <<= 1;
			if( lowdeltat < highdeltat )
				code |= 1;
			if( ++curbit == 12 )
				state = STOP;
		}
		break;

	case STOP:
		if( in
			&& deltat > STOP_MIN_TIME
			&& ( !g_codeready )
		    && ( ( code != g_code ) || ( lastedge - g_codetime > 500000 ) ) )
		{	// l->h => stop end
			g_code = code;
			g_codeready = true;
			g_codetime = lastedge;
		}
#ifdef FAILSTATS
		else {
			++g_stats.stopabort;
			g_stats.stopdeltat = deltat;
		}
#endif
		state = START;
		break;
	}

	lastlevel = in;
	g_lastedge = lastedge = curedge;
}

//////////////////////////////////////////////////////////////////////////////
ISR( TIMER0_COMPA_vect ) {
#ifdef PIN_LED
	digitalWrite( PIN_LED, ( micros() - g_codetime < 500000 ) ? HIGH : LOW );
#endif
}

