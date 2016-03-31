/*
 * PCF8574outputs.cpp
 *
 *  Created on: Mar 31, 2016
 *      Author: compi
 */

#include <PCF8574outputs.h>
#include <sdfatlogwriter.h>
#include "globals.h"

PCF8574outputs::PCF8574outputs(uint8_t i2caddress)
: PCF8574(i2caddress)
{
}

uint8_t PCF8574outputs::write( uint8_t pin, uint8_t value )
{
	uint8_t	error(PCF8574::write(pin, value));
	if(error)
		g_logger.log( logwriter::ERROR, g_time, F("OUTFAIL"), -1 );
	return error;
}

uint8_t PCF8574outputs::write8( uint8_t value )
{
	uint8_t	error(PCF8574::write8(value));
	if(error)
		g_logger.log( logwriter::ERROR, g_time, F("OUTFAIL"), -1 );

	return error;
}
