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

extern volatile bool 		g_codeready;
extern volatile uint16_t 	g_code;
extern volatile uint16_t 	g_shadowcode;
extern volatile uint32_t 	g_codetime;
extern volatile uint32_t 	g_lastedge;

//////////////////////////////////////////////////////////////////////////////
enum RcvState : uint8_t {
	  START
	, DATA
	, STOP
};

void isr();
void setup433();


//////////////////////////////////////////////////////////////////////////////
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

extern volatile stats g_stats;
#endif	//	FAILSTATS

#endif /* DECODE433_H_ */
