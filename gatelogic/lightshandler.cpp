/*
 * lightshandler.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: abody
 */

#include "lightshandler.h"

uint8_t lightshandler::m_outerpins[3] = {4,5,6};
uint8_t lightshandler::m_innerpins[3] = {7,8,9};

lightshandler::lightshandler()
	: m_prevstatus( NONE )
{
}

lightshandler::~lightshandler()
{
}

lightshandler::LOOPSTATUS lightshandler::update()
{
	LOOPSTATUS	curstatus(NONE);
	if( loopcheck( true )) curstatus = ((LOOPSTATUS)( curstatus | INNER ));
	if( loopcheck( false )) curstatus = ((LOOPSTATUS)( curstatus | OUTER ));

	if( curstatus == BOTH ) {
		if( m_prevstatus == NONE )
			m_prevstatus = INNER;
	} else
		m_prevstatus = curstatus;

	//......

	return m_prevstatus;
}
