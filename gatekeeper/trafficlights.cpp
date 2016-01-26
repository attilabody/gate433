/*
 * trafficlights.cpp
 *
 *  Created on: Dec 1, 2015
 *      Author: compi
 */

#include <PCF8574.h>
#include "config.h"
#include "globals.h"
#include "trafficlights.h"
//#define DEBUG_LIGHT
//#define DEBUG_LIGHT_INIT

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
light::light( uint8_t iopin, bool highon )
{
	init( iopin, highon );
}

//////////////////////////////////////////////////////////////////////////////
bool light::init( uint8_t iopin, bool highon )
{
	m_iopin = iopin;
	m_onvalue = highon ? HIGH : LOW;
	m_offvalue = highon ? LOW : HIGH;
	m_on = false;
	m_cyclelen = 0;
	m_lastmilli = millis();
	bool ret(false);

	if( m_iopin != 0xff ) {
		g_i2cio.write( m_iopin, m_offvalue );
		ret = true;
	}
#ifdef DEBUG_LIGHT_INIT
	Serial.print( m_iopin );
	Serial.print(F(": light::init return "));
	Serial.println( ret ? F("true") : F("false"));
#endif
	return ret;
}
//////////////////////////////////////////////////////////////////////////////
void light::loop( unsigned long curmillis )
{
	if( !m_cyclelen ) return;
	if( m_lastmilli + m_cyclelen <= curmillis )
	{
		if( m_cyclecount ) {
			m_on = !m_on;
			g_i2cio.write( m_iopin, m_on ? m_onvalue : m_offvalue );
			if( m_cyclecount != 0xff ) --m_cyclecount;
		} else {
			if( m_on && m_endoff ) {
				m_on = false;
				g_i2cio.write( m_iopin, m_offvalue );
			}
			m_cyclelen = 0;
		}
		m_lastmilli = curmillis;
	}
}

//////////////////////////////////////////////////////////////////////////////
void light::set( bool on, unsigned long cyclelen, uint8_t cyclecount, bool endoff, unsigned long currmillis )
{
#ifdef DEBUG_LIGHT
	Serial.print( m_iopin ); Serial.print( ": ");
	Serial.print( on ? F(" on ") : F("off "));
	Serial.print( cyclelen ); Serial.print( ' ' );
	Serial.print( cyclecount ); Serial.println( endoff ? F(" true") : F(" false") );
#endif	//	DEBUG_LIGHT
	m_lastmilli = currmillis ? currmillis : millis();
	m_cyclelen = cyclelen;
	m_cyclecount = cyclecount;
	m_on = on;
	m_endoff = endoff;
	g_i2cio.write( m_iopin, on ? m_onvalue : m_offvalue );
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
trafficlight::trafficlight( const uint8_t *pins, bool highon )
{
	init( pins, highon );
}

//////////////////////////////////////////////////////////////////////////////
bool trafficlight::init( const uint8_t *pins, bool highon )
{
	bool ret( true );
	for( uint8_t idx = 0; idx<3; ++idx ) {
		ret &= m_lights[idx].init( *pins++, highon );
	}
	return ret;
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
	  0x0000	//	OFF
	, 0x0204	//	NEEDCODE
	, 0x2244	//	CONFLLICT
	, 0x0104	//	ACCEPTED
	, 0x1104	//	WARNED
	, 0x4404	//	DENIED
	, 0x0404	//	PASS
};

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
trafficlights::trafficlights( const uint8_t *innerpins, const uint8_t *outerpins, bool highon, unsigned long cyclelen )
{
	init( innerpins, outerpins, highon, cyclelen );
}

//////////////////////////////////////////////////////////////////////////////
bool trafficlights::init( const uint8_t *innerpins, const uint8_t *outerpins, bool highon, unsigned long cyclelen )
{
	bool ret( true );
	m_status = OFF;
	m_cyclelen = cyclelen;
	ret = m_inner.init( innerpins, highon ) & ret;
	ret = m_outer.init( outerpins, highon ) & ret;
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
uint16_t trafficlights::set( STATUS status, bool inner )
{
	set( m_compstates[status], inner );
	m_status = status;
	return m_compstates[status];
}

//////////////////////////////////////////////////////////////////////////////
void trafficlights::set( uint16_t state, bool inner )
{
	unsigned long	currmillis( millis());
	trafficlight	&primary( inner ? m_inner : m_outer);
	trafficlight	&secondary( inner ? m_outer : m_inner);

	uint8_t	primaryvals( (state >> 8 ) &0xf );
	uint8_t	primaryblinkmask( (state >> 12 ) &0xf );
	uint8_t	secondaryvals( state &0xf );
	uint8_t	secondaryblinkmask( ( state >> 4 ) &0xf );
	uint8_t	curcolormask(1);

	for( uint8_t color = trafficlight::GREEN; color <= trafficlight::RED; ++color, curcolormask <<= 1 ) {
		primary.set( (trafficlight::COLORS) color, (primaryvals & curcolormask) != 0, (primaryblinkmask & curcolormask) != 0 ? m_cyclelen : 0, currmillis );
		secondary.set( (trafficlight::COLORS) color, (secondaryvals & curcolormask) != 0, (secondaryblinkmask & curcolormask) != 0 ? m_cyclelen : 0, currmillis );
	}
	m_status = NUMSTATES;
}

//////////////////////////////////////////////////////////////////////////////
void trafficlights::loop( unsigned long currmillis )
{
	if( !currmillis ) currmillis = millis();
	m_inner.loop( currmillis );
	m_outer.loop( currmillis );
}
