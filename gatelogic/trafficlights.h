/*
 * trafficlights.h
 *
 *  Created on: Dec 1, 2015
 *      Author: compi
 */

#ifndef TRAFFICLIGHTS_H_
#define TRAFFICLIGHTS_H_

#include <Arduino.h>

class trafficlights {
public:
	enum COLORINDEX { IDX_GREEN = 0, IDX_YELLOW = 1, IDX_RED = 2 };

	trafficlights();
	virtual ~trafficlights();
private:
	static uint8_t	m_outerpins[3];
	static uint8_t	m_innerpins[3];
};

#endif /* TRAFFICLIGHTS_H_ */
