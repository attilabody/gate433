/*
 * decode433.h
 *
 *  Created on: Dec 18, 2015
 *      Author: compi
 */

#ifndef DECODE433_H_
#define DECODE433_H_

#include <Arduino.h>
#include "config.h"

//#define FAILSTATS

#define SHORT_MIN_TIME	220
#define SHORT_MAX_TIME	800
#define LONG_MIN_TIME	580
#define LONG_MAX_TIME	1400
#define CYCLE_MAX_TIME	( SHORT_MAX_TIME + LONG_MAX_TIME )
#define CYCLE_MIN_TIME	( SHORT_MIN_TIME + LONG_MIN_TIME )
#define	STOP_MIN_TIME	10000

extern volatile bool 		g_codeready, g_code2ready;//, g_lrready;
extern volatile uint16_t 	g_code;
extern volatile uint16_t 	g_code2;	//free running code
extern volatile uint16_t	g_lrcode;
extern volatile uint32_t 	g_codetime;
extern volatile uint32_t 	g_lastedge;

//////////////////////////////////////////////////////////////////////////////
enum RCVSTATUS : uint8_t {
	  START
	, DATA
	, STOP
};

void 	isr();
void 	setup433();

#ifdef DECODE433_REVERSE
inline uint16_t getid( uint16_t code ) { return code >> 2; }
inline uint8_t getbutton( uint16_t code ) { return code & 3; }
#else
inline uint16_t getid( uint16_t code ) { return code & 0x3ff; }
inline uint8_t getbutton( uint16_t code ) { return code >> 10; }
#endif

//////////////////////////////////////////////////////////////////////////////
#ifdef FAILSTATS
struct stats
{
	stats() { memset( this, 0, sizeof *this ); }// startabort = dataabort = stopabort = stopdeltat = success = 0;}
	bool operator==( const stats &o ) {
		return startabort == o.startabort && dataabort == o.dataabort && stopabort == o.stopabort && success == o.success;
	}
	stats& operator=( const stats &o ) {
		startabort = o.startabort; dataabort = o.dataabort; stopabort = o.stopabort; success = o.success; return *this;
	}
	unsigned long startabort, dataabort, stopabort, success, stopdeltat;
};

extern volatile stats g_stats;
#endif	//	FAILSTATS

#endif /* DECODE433_H_ */
