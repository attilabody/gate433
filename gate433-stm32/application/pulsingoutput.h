/*
 * PulsingOutput.h
 *
 *  Created on: Mar 10, 2018
 *      Author: compi
 */

#ifndef PULSINGOUTPUT_H_
#define PULSINGOUTPUT_H_

#include "stm32f1xx_hal.h"
#include <tick.h>

class PulsingOutput: public ITick {
public:
	PulsingOutput(GPIO_TypeDef *port, uint16_t pin, bool activeHigh = true);
	virtual ~PulsingOutput() = default;

	virtual void Tick(uint32_t now) override;

	void Set(uint16_t pulseLen = 0, uint16_t cycleLen = 0, uint32_t now = 0);
	void Reset();

private:
	GPIO_TypeDef	*m_port;
	uint16_t		m_pin;
	bool			m_activeHigh;

	bool			m_active = false;
	uint16_t		m_pulseLen = 0;
	uint16_t		m_cycleLen = 0;
	uint32_t		m_lastTick = 0;
	uint32_t		m_changedAt = 0;
};

#endif /* PULSINGOUTPUT_H_ */
