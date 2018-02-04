/*
 * PwmOutputs.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: compi
 */

#include <PwmOutputs.h>
#include <string.h>

////////////////////////////////////////////////////////////////////
const PwmOutputs::AnalogOutput PwmOutputs::m_analogOuts[6] =  {
		{ &htim2, TIM_CHANNEL_1 },
		{ &htim2, TIM_CHANNEL_2 },
		{ &htim2, TIM_CHANNEL_3 },
		{ &htim2, TIM_CHANNEL_4 },
		{ &htim3, TIM_CHANNEL_3 },
		{ &htim3, TIM_CHANNEL_4 },
};


////////////////////////////////////////////////////////////////////
PwmOutputs::PwmOutputs()
{
	TIM_HandleTypeDef*	usedTimers[MAX_TIMERS];
	TIM_HandleTypeDef*	lastTimer = nullptr;

	memset(usedTimers, 0, sizeof(usedTimers));

	for(auto &out : m_analogOuts) {
		bool found = lastTimer == out.handle;
		if(!found) {
			for(auto &handle : usedTimers) {
				if(out.handle == handle) {
					found = true;
					break;
				}
				if(!handle) {
					handle = out.handle;
					break;
				}
			}
			lastTimer = out.handle;
		}
		if(!found)
			HAL_TIM_Base_Start(out.handle);

		HAL_TIM_PWM_Start(out.handle, out.channel);
		__HAL_TIM_SET_COMPARE(out.handle, out.channel, 0);
	}
}

////////////////////////////////////////////////////////////////////
bool PwmOutputs::Set(uint8_t channel, uint16_t value)
{
	if(channel >= COUNT) return false;
	__HAL_TIM_SET_COMPARE(m_analogOuts[channel].handle, m_analogOuts[channel].channel, value);
	return true;
}
