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
class outputpin
{
public:
				outputpin() {};
				outputpin( uint8_t iopin, bool highon );
	bool 		init( uint8_t iopin, bool highon );
	void 		loop( unsigned long curmillis );
	bool 		get() { return m_on; }
	operator	bool() { return m_on; }

	void set( bool on, uint16_t	cyclelen, uint8_t cyclecount, bool endoff, unsigned long currmillis = 0 );

private:
	uint8_t			m_iopin;
	bool			m_highon;
	bool			m_on;
	uint16_t		m_cyclelen;
	uint8_t			m_cyclecount;
	bool			m_endoff;
	unsigned long	m_lastmilli;

	inline uint8_t onval() const { return m_highon ? HIGH : LOW; }
	inline uint8_t offval() const { return m_highon ? LOW : HIGH; }
	inline uint8_t onoffval( bool on ) const { return (m_highon == on) ? HIGH : LOW; }
};

//////////////////////////////////////////////////////////////////////////////
class trafficlight
{
public:
	enum COLORS { GREEN=0, YELLOW=1, RED=2 };

			trafficlight() {}
			trafficlight( const uint8_t pins[], bool highon );
	bool 	init( const uint8_t pins[], bool highon );
	void	loop( unsigned long currmillis );
	void	set( COLORS color, bool on, uint16_t cyclelen, unsigned long currmillis = 0 );
	void 	set( bool r, uint16_t rc, bool y, uint16_t yc, bool g, uint16_t gc, unsigned long currmillis = 0 );

protected:
	outputpin			m_lights[3];
};

//////////////////////////////////////////////////////////////////////////////
class trafficlights
{
public:
	enum STATUS { OFF=0, CODEWAIT, CONFLICT, ACCEPTED, WARNED, DENIED, PASS, NUMSTATES };

	trafficlights() {}
	trafficlights( const uint8_t innerpins[], const uint8_t outerpins[], uint16_t cyclelen );
	bool 		init( const uint8_t innerpins[], const uint8_t outerpins[], uint16_t cyclelen );
	void		loop( unsigned long currmillis = 0 );
	uint16_t	set( STATUS status, bool inner );
	void		set( uint16_t state, bool inner );
	STATUS		getstatus() const { return m_status; }
	operator STATUS() const { return m_status; }

protected:
	trafficlight			m_inner, m_outer;
	STATUS					m_status;
	static const uint16_t	m_compstates[NUMSTATES];
	uint16_t				m_cyclelen;
};
#endif /* TRAFFICLIGHTS_H_ */
