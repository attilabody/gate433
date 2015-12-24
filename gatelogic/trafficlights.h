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
	enum COLORS { GREEN=0, YELLOW=1, RED=2 };

	trafficlight();
	void 	init( const uint8_t *pins, bool highon );
	void	loop( unsigned long currmillis );
	void	set( COLORS color, bool on, unsigned long cyclelen, unsigned long currmillis = 0 );
	void 	set( bool r, unsigned long rc, bool y, unsigned long yc, bool g, unsigned long gc, unsigned long currmillis = 0 );

protected:
	light			m_lights[3];


};

//////////////////////////////////////////////////////////////////////////////
class trafficlights
{
public:
	enum STATES { OFF=0, NEEDCODE, CONFLICT, ACCEPTED, DENIED, PASS, NUMSTATES };

	trafficlights();
	void 	init( const uint8_t *innerpins, const uint8_t *outerpins, bool highon, unsigned long cyclelen );
	void	loop( unsigned long currmillis ) { m_inner.loop( currmillis ), m_outer.loop( currmillis ); }
	void	set( STATES state, bool inner );
	void	set( uint16_t state, bool inner );

protected:
	trafficlight			m_inner, m_outer;
	STATES					m_state;
	static const uint16_t	m_compstates[NUMSTATES];
	unsigned long			m_cyclelen;
};
#endif /* TRAFFICLIGHTS_H_ */
