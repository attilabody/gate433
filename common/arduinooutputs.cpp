/*
 * outputs.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: compi
 */
#include <arduinooutputs.h>
#include <avr/pgmspace.h>
#include "config.h"

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
bool arduinooutputs::init(const uint8_t pins[], uint8_t value)
{
#if defined(__ARDUINOOUTPUTS_VERBOSE)
	Serial.print(F("arduinooutputs::init: "));
#endif
	uint8_t	rawpin;
	for(uint8_t pin = 0; pin < 8; ++pin)
	{
		rawpin = pins[pin];
		pinMode(rawpin, value);
		digitalWrite(rawpin, value);
		m_pins[pin] = rawpin;
#if defined(__ARDUINOOUTPUTS_VERBOSE)
		if(pin) Serial.print(F(", "));
		Serial.print(rawpin);
	}
	Serial.println();
#else
	}
#endif
	return true;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t arduinooutputs::set(uint8_t pin, uint8_t value)
{
#if defined(__ARDUINOOUTPUTS_VERBOSE)
	Serial.print(F("arduinooutputs::write: "));
	Serial.print(F(", "));
	Serial.print(pin);
	Serial.print(F(", ("));
	Serial.print(m_pins[pin]);
	Serial.print(F("), "));
	Serial.println(value);
#endif
	if(pin < 8) {
		digitalWrite(m_pins[pin], value);
		return 0;
	}
	else return 0xff;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t arduinooutputs::set(uint8_t value)
{
	uint8_t	mask(1);
	for(uint8_t pin = 0; pin < 8; ++pin) {
		set(pin, (value & mask) ? HIGH : LOW);
		mask <<= 1;
	}
	return 0;
}
