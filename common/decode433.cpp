/*
 * decode433.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: compi
 */
#include "decode433.h"

volatile bool 		g_codeready( false ), g_code2ready( false );//, g_lrready( false );
volatile uint16_t 	g_code(-1);
volatile uint16_t 	g_code2(-1);
volatile uint16_t 	g_lrcode(-1);
volatile uint32_t 	g_codetime( 0 );
volatile uint32_t 	g_lastedge;

#ifdef FAILSTATS
volatile stats 		g_stats;
#endif

enum RCVSTATUS : uint8_t {
	  START
	, DATA
};


//////////////////////////////////////////////////////////////////////////////
void setup433()
{
	pinMode( PIN_RFIN, INPUT );
	attachInterrupt( digitalPinToInterrupt( PIN_RFIN ), isr, CHANGE );
}

//////////////////////////////////////////////////////////////////////////////
inline void newbit( uint16_t &code, bool bit )
{
#ifdef DECODE433_REVERSE
	code <<=1;
	code |= bit ? 1 : 0;
#else
	code >>= 1;
	code |= bit ? 0x800 : 0;
#endif
}

//////////////////////////////////////////////////////////////////////////////
void isr()
{
	static int8_t curbit;
	static uint32_t lastedge( micros() ), curedge;
	// we are at the closing edge of the current half cycle. level after the
	// edge is opposite what the level was during the cycle
	static bool curlevel;
	static RCVSTATUS state( START );
	static uint16_t code, deltat, cyclet, spwidth, diff;

	static uint16_t highdeltat, lowdeltat;

	curedge = micros();
	curlevel = digitalRead( PIN_RFIN ) != HIGH;
	deltat = curedge - lastedge;
	(curlevel ? highdeltat : lowdeltat) = deltat;

	switch( state ) {
	case START:
		if(	curlevel &&						// high half
			deltat >= SHORT_MIN_TIME &&		// short start
			deltat <= SHORT_MAX_TIME &&
			lowdeltat >= STOP_MIN_TIME		// long low
		) {	// h->l
			state = DATA;
			curbit = code = 0;
			spwidth = deltat;
		}
#ifdef FAILSTATS
		else
		++g_stats.probes[stats::START];
#endif
		break;

	case DATA:
		if( deltat < (spwidth >> 1) ||	//	/2
			deltat > (spwidth << 2 )) {	//	*4
			state = START;
#ifdef FAILSTATS
			++g_stats.probes[stats::DATA1];
			g_stats.probes[stats::DT] = deltat;
			g_stats.probes[stats::ECT] = spwidth;
#endif
		} else if( curlevel ) 	//	high half
		{
			cyclet = highdeltat + lowdeltat;
			diff = highdeltat > lowdeltat ? highdeltat - lowdeltat : lowdeltat - highdeltat;
			if( cyclet < spwidth || cyclet > (spwidth  << 3) || diff < (cyclet >> 3))
			{
				state = START;
#ifdef FAILSTATS
				++g_stats.probes[stats::DATA2];
				g_stats.probes[stats::CT] = cyclet;
				g_stats.probes[stats::ECT] = spwidth;
#endif
				break;
			}
			newbit( code, lowdeltat < highdeltat );
			if( ++curbit == 12 ) {
#ifdef FAILSTATS
				++g_stats.probes[stats::SUCCESS];
#endif
				state = START;
				if( !g_codeready ) {
					g_code = code;
					g_codeready = true;
				}
				if( !g_code2ready ) {
					g_code2 = code;
					g_code2ready = true;
				}

				g_lrcode = code;
				//g_lrready = true;
				g_codetime = lastedge;
			}
		}
		break;
	}

	g_lastedge = lastedge = curedge;
}



