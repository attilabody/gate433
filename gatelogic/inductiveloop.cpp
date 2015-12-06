/*
 * lightshandler.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: abody
 */

#include "inductiveloop.h"

uint8_t inductiveloop::m_outerpins[3] = {4,5,6};
uint8_t inductiveloop::m_innerpins[3] = {7,8,9};

inductiveloop::inductiveloop()
	: m_prevstatus( NONE )
{
}

inductiveloop::~inductiveloop()
{
}

inductiveloop::LOOPSTATUS inductiveloop::update()
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
