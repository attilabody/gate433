/*
 * trafficlights.h
 *
 *  Created on: Dec 1, 2015
 *      Author: compi
 */

#ifndef TRAFFICLIGHTS_H_
#define TRAFFICLIGHTS_H_

#include <Arduino.h>

//////////////////////////////////////////////////////////////////////////////
class light
{
public:
	light( uint8_t iopin = 0xff, bool highon = true );
	void 		init( uint8_t iopin, bool highon );
	void 		loop( unsigned long curmillis );
	bool 		get() { return m_on; }
	operator	bool() { return m_on; }

	void set( bool on, unsigned long cyclelen, uint8_t cyclecount, bool endoff, unsigned long currmillis = 0 );

private:
	uint8_t			m_iopin;
	uint8_t			m_onvalue;
	uint8_t			m_offvalue;
	bool			m_on;
	unsigned long	m_cyclelen;
	uint8_t			m_cyclecount;
	bool			m_endoff;
	unsigned long	m_lastmilli;
};

//////////////////////////////////////////////////////////////////////////////
class trafficlight
{
public:
	enum COLORS { RED=0, YELLOW=1, GREEN=2 };

	trafficlight();
	void 	init( const uint8_t *pins, bool highon );
	void	loop( unsigned long currmillis );
	void	set( COLORS color, bool on, unsigned long cyclelen );
	void 	set( bool r, unsigned long rc, bool y, unsigned long yc, bool g, unsigned long gc );

private:
	light			m_lights[3];


};

//////////////////////////////////////////////////////////////////////////////
class trafficlights
{
public:
	enum STATES { OFF=0, NEEDCODE, CONFLICT, ACCEPTED, DENIED, PASS };

	trafficlights();
	void 	init( const uint8_t *innerpins, const uint8_t *outerpins, bool highon );
	void	set( STATES state, bool side );	//	true = inner, false = outer

private:
	trafficlight	m_inner, m_outer;
	STATES			m_state;
};
#endif /* TRAFFICLIGHTS_H_ */
