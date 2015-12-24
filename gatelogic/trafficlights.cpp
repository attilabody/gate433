/*
 * trafficlights.cpp
 *
 *  Created on: Dec 1, 2015
 *      Author: compi
 */

#include "trafficlights.h"
#include "config.h"

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
#ifdef DEBUG_TRAFFICLIGHTS
	Serial.print( m_iopin ); Serial.print( ": ");
	Serial.print( on ? " on " : "off ");
	Serial.print( cyclelen ); Serial.print( ' ' );
	Serial.print( cyclecount ); Serial.println( endoff ? " true" : " false" );
#endif	//	DEBUG_TRAFFICLIGHTS
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
void trafficlight::set( COLORS color, bool on, unsigned long cyclelen, unsigned long currmillis )
{
	if( !currmillis ) currmillis = millis();
	m_lights[color].set( on, cyclelen, 0xff, true );
}

//////////////////////////////////////////////////////////////////////////////
void trafficlight::set( bool r, unsigned long rc, bool y, unsigned long yc, bool g, unsigned long gc, unsigned long currmillis )
{
	if( !currmillis ) currmillis = millis();
	set( RED, r, rc );
	set( YELLOW, y, yc );
	set( GREEN, g, gc );
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//OFF=0, NEEDCODE, CONFLICT, ACCEPTED, DENIED, PASS
const uint16_t	trafficlights::m_compstates[trafficlights::NUMSTATES] = {
	  0x0000
	, 0x0204
	, 0x2244
	, 0x0104
	, 0x4404
	, 0x0404
};

//////////////////////////////////////////////////////////////////////////////
trafficlights::trafficlights() : m_state( OFF ), m_cyclelen( 500 )
{
}

//////////////////////////////////////////////////////////////////////////////
void trafficlights::init( const uint8_t *innerpins, const uint8_t *outerpins, bool highon, unsigned long cyclelen )
{
	m_inner.init( innerpins, highon );
	m_outer.init( outerpins, highon );
	m_state = OFF;
	m_cyclelen = cyclelen;
}

//////////////////////////////////////////////////////////////////////////////
void trafficlights::set( STATES state, bool inner )
{
	set( m_compstates[state], inner );
}

//////////////////////////////////////////////////////////////////////////////
void trafficlights::set( uint16_t state, bool inner )
{
	unsigned long	currmillis( millis());
	trafficlight	&master( inner ? m_inner : m_outer);
	trafficlight	&slave( inner ? m_outer : m_inner);

	uint8_t	mastervals( (state >> 8 ) &0xf );
	uint8_t	masterblinks( (state >> 12 ) &0xf );
	uint8_t	slavevals( state &0xf );
	uint8_t	slaveblinks( ( state >> 4 ) &0xf );
	uint8_t	mask(1);

	for( uint8_t color = trafficlight::GREEN; color <= trafficlight::RED; ++color, mask <<= 1 ) {
		master.set( (trafficlight::COLORS) color, (mastervals & mask) != 0, (masterblinks & mask) != 0 ? m_cyclelen : 0, currmillis );
		slave.set( (trafficlight::COLORS) color, (slavevals & mask) != 0, (slaveblinks & mask) != 0 ? m_cyclelen : 0, currmillis );
	}
}

