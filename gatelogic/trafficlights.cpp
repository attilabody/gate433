/*
 * trafficlights.cpp
 *
 *  Created on: Dec 1, 2015
 *      Author: compi
 */

#include "trafficlights.h"
//#include "config.h"

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
trafficlight::trafficlight()
{
}

//////////////////////////////////////////////////////////////////////////////
void trafficlight::init( const uint8_t *pins, bool highon )
{
	for( uint8_t idx = 0; idx<3; ++idx ) {
		m_lights[idx].init( *pins++, highon );
	}
}

//////////////////////////////////////////////////////////////////////////////
void trafficlight::loop( unsigned long currmillis )
{
	for( uint8_t idx = 0; idx < 3; ++idx ) {
		m_lights[idx].loop( currmillis );
	}
}

//////////////////////////////////////////////////////////////////////////////
void trafficlight::set( COLORS color, bool on, unsigned long cyclelen )
{
	m_lights[color].set( on, cyclelen, 0xff, true );
}

//////////////////////////////////////////////////////////////////////////////
void trafficlight::set( bool r, unsigned long rc, bool y, unsigned long yc, bool g, unsigned long gc )
{
	set( RED, r, rc );
	set( YELLOW, y, yc );
	set( GREEN, g, gc );
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
light::light( uint8_t iopin, bool highon )
{
	init( iopin, highon );
}

//////////////////////////////////////////////////////////////////////////////
void light::init( uint8_t iopin, bool highon )
{
	m_iopin = iopin;
	m_onvalue = highon ? HIGH : LOW;
	m_offvalue = highon ? LOW : HIGH;
	m_on = false;
	m_cyclelen = 0;
	m_lastmilli = millis();
	if( m_iopin != 0xff ) {
		pinMode( m_iopin, OUTPUT );
		digitalWrite( m_iopin, m_offvalue );
	}
}
//////////////////////////////////////////////////////////////////////////////
void light::loop( unsigned long curmillis )
{
	if( !m_cyclelen ) return;
	if( m_lastmilli + m_cyclelen <= curmillis )
	{
		if( m_cyclecount ) {
			m_on = !m_on;
			digitalWrite( m_iopin, m_on ? m_onvalue : m_offvalue );
			if( m_cyclecount != 0xff ) --m_cyclecount;
		} else {
			if( m_on && m_endoff ) {
				m_on = false;
				digitalWrite( m_iopin, m_offvalue );
			}
			m_cyclelen = 0;
		}
		m_lastmilli = curmillis;
	}
}

//////////////////////////////////////////////////////////////////////////////
void light::set( bool on, unsigned long cyclelen, uint8_t cyclecount, bool endoff, unsigned long currmillis )
{
	m_lastmilli = currmillis ? currmillis : millis();
	m_cyclelen = cyclelen;
	m_cyclecount = cyclecount;
	m_on = on;
	m_endoff = endoff;
	digitalWrite( m_iopin, on ? m_onvalue : m_offvalue );
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
trafficlights::trafficlights() : m_state( OFF )
{
}

//////////////////////////////////////////////////////////////////////////////
void trafficlights::init( const uint8_t *innerpins, const uint8_t *outerpins, bool highon )
{
	m_inner.init( innerpins, highon );
	m_outer.init( outerpins, highon );
	m_state = OFF;
}

//////////////////////////////////////////////////////////////////////////////
void trafficlights::set( STATES state, bool side )
{
	trafficlight	&master( side ? m_inner : m_outer);
	trafficlight	&slave( side ? m_outer : m_inner);
}
