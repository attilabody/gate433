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
				light() {};
				light( uint8_t iopin, bool highon );
	bool 		init( uint8_t iopin, bool highon );
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

			trafficlight() {}
			trafficlight( const uint8_t *pins, bool highon );
	bool 	init( const uint8_t *pins, bool highon );
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
	enum STATUS { OFF=0, CODEWAIT, CONFLICT, ACCEPTED, WARNED, DENIED, PASS, NUMSTATES };

	trafficlights() {}
	trafficlights( const uint8_t *innerpins, const uint8_t *outerpins, bool highon, unsigned long cyclelen );
	bool 		init( const uint8_t *innerpins, const uint8_t *outerpins, bool highon, unsigned long cyclelen );
	void		loop( unsigned long currmillis = 0 );
	uint16_t	set( STATUS status, bool inner );
	void		set( uint16_t state, bool inner );
	STATUS		getstatus() const { return m_status; }
	operator STATUS() const { return m_status; }

protected:
	trafficlight			m_inner, m_outer;
	STATUS					m_status;
	static const uint16_t	m_compstates[NUMSTATES];
	unsigned long			m_cyclelen;
};
#endif /* TRAFFICLIGHTS_H_ */
