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

inductiveloop::LOOPSTATUS inductiveloop::update( LOOPSTATUS &rawstatus )
{
	rawstatus = NONE;
	if( getstatus( m_innerpin ) == m_activelevel ) {
		rawstatus = ((LOOPSTATUS)( rawstatus | INNER ));
	}
	if( getstatus( m_outerpin ) == m_activelevel ) {
		rawstatus = ((LOOPSTATUS)( rawstatus | OUTER ));
	}

	if( rawstatus == BOTH ) {
		if( m_prevstatus == NONE )
			m_prevstatus = INNER;
	} else
		m_prevstatus = rawstatus;

	//......

	return m_prevstatus;
}
