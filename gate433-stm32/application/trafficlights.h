/*
 * trafficlights.h
 *
 *  Created on: Feb 17, 2018
 *      Author: compi
 */

#ifndef TRAFFICLIGHTS_H_
#define TRAFFICLIGHTS_H_

#include "smartlights.h"
#include "states.h"

class TrafficLights : protected SmartLights
{
public:
	using SmartLights::Tick;

	TrafficLights(uint16_t switchStep, uint16_t blinkStep);
	virtual ~TrafficLights() = default;


	void SetMode(States mode, bool inner);
private:

	uint16_t m_switchStep, m_blinkStep;
	static const uint16_t	COMPSTATES[States::NUMSTATES];

};

#endif /* TRAFFICLIGHTS_H_ */
