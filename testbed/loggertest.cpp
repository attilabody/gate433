/*
 * loggertest.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: abody
 */
#include <Arduino.h>
#include "SdFat.h"
#include "sdfatlogwriter.h"
#include <ds3231.h>

SdFat			g_sd;
sdfatlogwriter	g_log( g_sd );
ts				g_dt;

void setup()
{
	Serial.begin( 115200 );
	g_sd.begin( SS );
	g_log.init();
	g_log.log( logwriter::INFO, g_dt, 0, F("Lofaszbingo") );
}

void loop()
{

}
