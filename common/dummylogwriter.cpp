/*
 * dummylogwriter.cpp
 *
 *  Created on: Feb 15, 2016
 *      Author: compi
 */

#include <Arduino.h>
#include <dummylogwriter.h>

dummylogwriter::dummylogwriter()
{
	// TODO Auto-generated constructor stub

}

dummylogwriter::~dummylogwriter()
{
	// TODO Auto-generated destructor stub
}

void dummylogwriter::log( CATEGORY category, ts& datetime, const char* message,
		uint16_t rid, uint8_t button, uint8_t dbpos, uint8_t loop,
		uint8_t decision )
{
}

void dummylogwriter::log( CATEGORY category, ts& datetime,
		const __FlashStringHelper* message, uint16_t rid, uint8_t button,
		uint8_t dbpos, uint8_t loop, uint8_t decision )
{
}
