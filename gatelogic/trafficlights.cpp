/*
 * trafficlights.cpp
 *
 *  Created on: Dec 1, 2015
 *      Author: compi
 */

#include "trafficlights.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////////
uint8_t trafficlights::m_outerpins[3] = {4,5,6};
uint8_t trafficlights::m_innerpins[3] = {7,8,9};


//////////////////////////////////////////////////////////////////////////////
trafficlights::trafficlights()
{
	// TODO Auto-generated constructor stub

}

//////////////////////////////////////////////////////////////////////////////
trafficlights::~trafficlights()
{
	// TODO Auto-generated destructor stub
}

//////////////////////////////////////////////////////////////////////////////
lamp::lamp( uint8_t iopin )
{
	init( iopin );
}

//////////////////////////////////////////////////////////////////////////////
void lamp::init( uint8_t iopin )
{
	m_iopin = iopin;
	m_on = false;
	m_cycle = 0;
	m_lastmilli = millis();
	if( m_iopin != 0xff ) {
		pinMode( m_iopin, OUTPUT );
		digitalWrite( m_iopin, RELAY_OFF );
	}
}
//////////////////////////////////////////////////////////////////////////////
void lamp::loop( unsigned long curmilli )
{
	if( !m_cycle ) return;
	if( m_lastmilli + m_cycle <= curmilli )
	{
		if( m_repeat ) {
			m_on = !m_on;
			digitalWrite( m_iopin, m_on ? RELAY_ON : RELAY_OFF);
			if( m_repeat != 0xff ) --m_repeat;
		} else if( m_on ) {
			m_on = false;
			digitalWrite( m_iopin, RELAY_OFF );
			m_cycle = 0;
		}
		m_lastmilli = curmilli;
	}
}

//////////////////////////////////////////////////////////////////////////////
void lamp::set( bool on, unsigned long cycle, uint8_t repeat )
{
	m_lastmilli = millis();
	m_cycle = cycle;
	m_repeat = repeat;
	m_on = on;
	digitalWrite( m_iopin, on ? RELAY_ON : RELAY_OFF);
}
