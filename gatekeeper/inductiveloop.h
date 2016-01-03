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
	enum STATUS : uint8_t { NONE = 0, OUTER = 1, INNER = 2 };

			inductiveloop() {};
			inductiveloop( uint8_t innerpin, uint8_t outerpin, uint8_t activelevel );
	// returns true in case of conflict (both loops signaled)
	bool	init( uint8_t innerpin, uint8_t outerpin, uint8_t activelevel );
	bool	update( STATUS &status );
	bool	update();


private:
	uint8_t	getstatus( uint8_t pin ) {
		if( pin != A6 && pin != A7 ) return digitalRead( pin );
		else return analogRead( pin ) > 256 ? HIGH : LOW;
	}

	STATUS	m_prevstatus;
	uint8_t		m_innerpin, m_outerpin;
	uint8_t		m_activelevel;
};

#endif /* INDUCTIVELOOP_H_ */
