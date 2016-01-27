/*
 * loggertest.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: abody
 */
#include <Arduino.h>
#include <SdFat.h>
#include <Wire.h>
#include <ds3231.h>
#include "globals.h"
#include "interface.h"
#include "sdfatlogwriter.h"

SdFat			g_sd;
sdfatlogwriter	g_log( g_sd );
ts				g_dt;

//////////////////////////////////////////////////////////////////////////////
void processinput();

//////////////////////////////////////////////////////////////////////////////
void setup()
{
	Serial.begin( 115200 );
	g_sd.begin( SS );
	Wire.begin();
	DS3231_init( DS3231_INTCN );
	DS3231_get( &g_dt );

	Serial.println( g_log.init() ? F("Log init succeeded") : F("Log init FAILED!"));
	g_log.log( logwriter::INFO, g_dt, 0, "Lofaszbingo");
}

//////////////////////////////////////////////////////////////////////////////
void loop()
{
	if( getlinefromserial( g_iobuf, sizeof( g_iobuf ), g_inidx) )
		processinput();
}

//////////////////////////////////////////////////////////////////////////////
void processinput()
{
	const char	*inptr( g_iobuf );

	Serial.println( g_iobuf );

	if( iscommand( inptr, F("dl"))) {
		g_log.dump( &Serial );
	} else if( iscommand( inptr, F("tl"))) {	// truncate log
		g_log.truncate();

	} else {
		Serial.println( F(ERRS "CMD"));
	}
	g_inidx = 0;
}
