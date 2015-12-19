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
	m_cyclelen = 0;
	m_lastmilli = millis();
	if( m_iopin != 0xff ) {
		pinMode( m_iopin, OUTPUT );
		digitalWrite( m_iopin, RELAY_OFF );
	}
}
//////////////////////////////////////////////////////////////////////////////
void lamp::loop( unsigned long curmilli )
{
	if( !m_cyclelen ) return;
	if( m_lastmilli + m_cyclelen <= curmilli )
	{
		if( m_cyclecount ) {
			m_on = !m_on;
			digitalWrite( m_iopin, m_on ? RELAY_ON : RELAY_OFF);
			if( m_cyclecount != 0xff ) --m_cyclecount;
		} else {
			if( m_on && m_endoff ) {
				m_on = false;
				digitalWrite( m_iopin, RELAY_OFF );
			}
			m_cyclelen = 0;
		}
		m_lastmilli = curmilli;
	}
}

//////////////////////////////////////////////////////////////////////////////
void lamp::set( bool on, unsigned long cyclelen, uint8_t cyclecount, bool endoff, unsigned long currmillis )
{
	m_lastmilli = currmillis ? currmillis : millis();
	m_cyclelen = cyclelen;
	m_cyclecount = cyclecount;
	m_on = on;
	m_endoff = endoff;
	digitalWrite( m_iopin, on ? RELAY_ON : RELAY_OFF);
}
