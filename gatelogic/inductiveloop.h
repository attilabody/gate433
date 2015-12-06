/*
 * lightshandler.h
 *
 *  Created on: Nov 18, 2015
 *      Author: abody
 */

#ifndef INDUCTIVELOOP_H_
#define INDUCTIVELOOP_H_
#include <Arduino.h>

class inductiveloop
{
public:
	enum LOOPSTATUS : uint8_t { NONE = 0, INNER = 1, OUTER = 2, BOTH = 3 };

	inductiveloop( uint8_t innerpin, uint8_t outerpin, uint8_t activelevel );
	virtual ~inductiveloop();
	LOOPSTATUS	update( LOOPSTATUS &rawstatus );


private:
	uint8_t	getstatus( uint8_t pin ) {
		if( pin != A6 && pin != A7 ) return digitalRead( pin );
		else return analogRead( pin ) > 256 ? HIGH : LOW;
	}

	LOOPSTATUS		m_prevstatus;
	uint8_t			m_innerpin, m_outerpin;
	const uint8_t	m_activelevel;
};

#endif /* INDUCTIVELOOP_H_ */
