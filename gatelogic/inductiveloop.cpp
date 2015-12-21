/*
 * lightshandler.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: abody
 */

#include "inductiveloop.h"

inductiveloop::inductiveloop( uint8_t innerpin, uint8_t outerpin, uint8_t activelevel )
	: m_prevstatus( NONE )
	, m_innerpin( innerpin )
	, m_outerpin( outerpin )
	, m_activelevel( activelevel )
{
}

inductiveloop::~inductiveloop()
{
}

bool inductiveloop::update( LOOPSTATUS &status )
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
		m_prevstatus = (LOOPSTATUS)rawstatus;

	return rawstatus == ( INNER | OUTER );
}
