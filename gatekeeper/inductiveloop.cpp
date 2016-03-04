/*
 * lightshandler.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: abody
 */

#include "inductiveloop.h"

inductiveloop::inductiveloop( uint8_t innerpin, uint8_t outerpin, uint8_t activelevel )
{
	init( innerpin, outerpin, activelevel );
}

bool inductiveloop::init( uint8_t innerpin, uint8_t outerpin, uint8_t activelevel )
{
	m_prevstatus = NONE;
	m_innerpin = innerpin;
	m_outerpin = outerpin;
	m_activelevel = activelevel;

	pinMode( m_innerpin, INPUT );
	digitalWrite( m_innerpin, HIGH );	//	activate pullup;
	pinMode( m_outerpin, INPUT );
	digitalWrite( m_outerpin, HIGH );	//	activate pullup;
	return true;
}

bool inductiveloop::update( STATUS &status )
{
	bool ret( update() );
	status = m_prevstatus;
	return ret;
}

bool inductiveloop::update()
{
	uint8_t rawstatus( 0 );

	if( getstatus( true ) == m_activelevel ) rawstatus |= (uint8_t) INNER;
	if( getstatus( false ) == m_activelevel ) rawstatus |= (uint8_t) OUTER;

	if( rawstatus == ( INNER | OUTER )) {
		if( m_prevstatus == NONE )
			m_prevstatus = INNER;
	} else
		m_prevstatus = (STATUS)rawstatus;

	return rawstatus == ( INNER | OUTER );
}

uint8_t inductiveloop::getstatus(bool pos)
{
	uint8_t			pin(pos ? m_innerpin : m_outerpin);
	bool 			level;
	unsigned long 	now(millis());

	if( pin != A6 && pin != A7 ) level = digitalRead( pin ) == HIGH;
	else level = analogRead( pin ) > 256;
	if(m_debounced[pos] == level) {
		 m_lastequals[pos] = now;
	} else {
		if( now - m_lastequals[pos] > 500 ) {
			m_debounced[pos] = level;
			 m_lastequals[pos] = now;
		}
	}
	return m_debounced[pos] ? HIGH : LOW;

}
