/*
 * decode433.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: compi
 */
#include "decode433.h"

volatile bool 		g_codeready( false );
volatile uint16_t 	g_code(-1);
volatile uint16_t 	g_frcode(-1);
volatile uint32_t 	g_codetime( 0 );
volatile uint32_t 	g_lastedge;

#ifdef FAILSTATS
volatile stats 		g_stats;
#endif

//////////////////////////////////////////////////////////////////////////////
void setup433()
{
	pinMode( PIN_RFIN, INPUT );
	attachInterrupt( digitalPinToInterrupt( PIN_RFIN ), isr, CHANGE );
}

//////////////////////////////////////////////////////////////////////////////
void isr()
{
	static int8_t curbit;
	static uint32_t lastedge( micros() ), curedge;
	static bool lastlevel( digitalRead( PIN_RFIN ) == HIGH ), in;
	static RCVSTATUS state( START );
	static uint16_t code, deltat, cyclet;

	static uint16_t highdeltat, lowdeltat, timediff;

	curedge = micros();
	in = ( digitalRead( PIN_RFIN ) == HIGH );
	deltat = curedge - lastedge;

	switch( state ) {
	case START:
		if( //!g_codeready &&
			lastlevel &&
			!in &&
			deltat >= SHORT_MIN_TIME &&
			deltat <= SHORT_MAX_TIME
		) {	// h->l
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
			timediff = highdeltat > lowdeltat ? highdeltat - lowdeltat : lowdeltat - highdeltat;
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
		if( in &&
			deltat > STOP_MIN_TIME //&&
		) {	// l->h => stop end
			if( !g_codeready ) {
				g_code = code;
				g_codeready = true;
				g_codetime = lastedge;
			}
			g_frcode = code;
#ifdef FAILSTATS
			g_stats.stopdeltat = deltat;
			++g_stats.success;
#endif
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



