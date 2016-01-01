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

	if( getstatus( m_innerpin ) == m_activelevel ) rawstatus |= (uint8_t) INNER;
	if( getstatus( m_outerpin ) == m_activelevel ) rawstatus |= (uint8_t) OUTER;

	if( rawstatus == ( INNER | OUTER )) {
		if( m_prevstatus == NONE )
			m_prevstatus = INNER;
	} else
		m_prevstatus = (STATUS)rawstatus;

	return rawstatus == ( INNER | OUTER );
}
