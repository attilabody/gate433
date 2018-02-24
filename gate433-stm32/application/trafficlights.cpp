/*
 * trafficlights.cpp
 *
 *  Created on: Feb 17, 2018
 *      Author: compi
 */

#include <trafficlights.h>

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// values: 0: off, 1: on, 2: blink
// bits:
// 0,1 (0x0003): primary green
// 2,3 (0x000c): primary yellow
// 4,5 (0x0030): primary red
// 8,9 (0x0300): secondary green
// a,b (0x0c00): secondary yellow
// c,d (0x3000): secondary red
//////////////////////////////////////////////////////////////////////////////
const uint16_t	TrafficLights::COMPSTATES[States::NUMSTATES] = {
	  0x0000	//	OFF
	, 0x1004	//	CODEWAIT -		PY1SR1
	, 0x2008	//	CONFLLICT - 	PY2SR2
	, 0x1001	//	ACCEPTE -		PG1SR1
	, 0x1002	//	WARN -			PG2SR1
	, 0x1010	//	DENY -			PR1SR1
	, 0x1020	//	UNREGISTERED	PR2SR1
	, 0x1002	//	HURRY -			PG2SR1
	, 0x1010	//	PASSING -		PR1SR1
};

////////////////////////////////////////////////////////////////////
TrafficLights::TrafficLights(uint16_t switchStep, uint16_t blinkStep)
: m_switchStep(switchStep)
, m_blinkStep(blinkStep)
{}

////////////////////////////////////////////////////////////////////
void TrafficLights::SetMode(States mode, bool inner)
{
	if(mode < States::NUMSTATES)
	{
		uint8_t primaryOffset = inner ? 0 : 3;
		uint8_t secondaryOffset = inner ? 3 : 0;
		uint8_t primaryVals = COMPSTATES[mode];
		uint8_t secondaryVals = COMPSTATES[mode] >> 8;

		for(uint8_t light = 0; light < 3; ++light) {
			auto primaryMode = (SmartLights::Mode)((primaryVals >> (light << 1)) & 3);
			auto secondaryMode = (SmartLights::Mode)((secondaryVals >> (light << 1)) & 3);
			SmartLights::SetMode(light+primaryOffset, primaryMode, primaryMode < SmartLights::BLINK ? m_switchStep : m_blinkStep );
			SmartLights::SetMode(light+secondaryOffset, secondaryMode, secondaryMode < SmartLights::BLINK ? m_switchStep : m_blinkStep );
		}
	}
}

