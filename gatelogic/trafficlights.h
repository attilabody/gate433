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
class lamp
{
public:
	lamp( uint8_t iopin = 0xff, bool highon = true );
	void 		init( uint8_t iopin, bool highon );
	void 		loop( unsigned long curmilli );
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
class trafficlights
{
public:
	enum COLORINDEX { IDX_GREEN = 0, IDX_YELLOW = 1, IDX_RED = 2 };

	trafficlights();
	virtual ~trafficlights();
private:
	static uint8_t	m_outerpins[3];
	static uint8_t	m_innerpins[3];
};

#endif /* TRAFFICLIGHTS_H_ */
