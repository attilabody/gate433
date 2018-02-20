/*
 * SmartLights.h
 *
 *  Created on: Feb 2, 2018
 *      Author: compi
 */

#ifndef SMARTLIGHTS_H_
#define SMARTLIGHTS_H_

#include <pwmoutputs.h>
#include <tick.h>

class SmartLights: public PwmOutputs, public ITick
{
public:
	SmartLights();

	void Tick(uint32_t now);

	enum Mode  : uint8_t { OFF = 0, ON = 1, BLINK = 2 };

	bool SetMode(uint8_t index, Mode mode, uint16_t step);
	void SetMax(uint16_t m) { m_maxVal = m; }

private:
		uint16_t	m_maxVal = 0xffff;
		uint32_t	m_lastTick;

	struct State {
		Mode		mode = OFF;
		bool		up;		//blink
		bool		active = false;
		uint16_t	value = 0;
		uint16_t	step;
	} m_states[COUNT];

	void Tock(uint8_t li, uint32_t now);
};

#endif /* SMARTLIGHTS_H_ */
