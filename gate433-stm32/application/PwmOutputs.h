/*
 * PwmOutputs.h
 *
 *  Created on: Feb 2, 2018
 *      Author: compi
 */

#ifndef PWMOUTPUTS_H_
#define PWMOUTPUTS_H_

#include <inttypes.h>
#include "tim.h"

class PwmOutputs {
public:
	PwmOutputs();
	bool Set(uint8_t channel, uint16_t value);

private:
	struct AnalogOutput {
		TIM_HandleTypeDef*	handle;
		uint32_t			channel;
	};
	static const AnalogOutput	m_analogOuts[6];
	static constexpr uint8_t	MAX_TIMERS = 4;


public:
	static constexpr uint8_t	COUNT = sizeof(m_analogOuts) / sizeof(m_analogOuts[0]);

};

#endif /* PWMOUTPUTS_H_ */
