/*
 * SmartLights.cpp
 *
 *  Created on: Feb 2, 2018
 *      Author: compi
 */

#include <SmartLights.h>
#include <sg/itlock.h>

////////////////////////////////////////////////////////////////////
SmartLights::SmartLights() {
	m_lastTick = HAL_GetTick();
}

////////////////////////////////////////////////////////////////////
void SmartLights::Tick(uint32_t now)
{
	for(uint8_t li = 0; li < COUNT; ++li)
		if(m_states[li].active)
			Tick(li, now);
	m_lastTick = now;
}

////////////////////////////////////////////////////////////////////
void SmartLights::Tick(uint8_t li, uint32_t now)
{
	State		&light = m_states[li];
	uint32_t	deltaT = now - m_lastTick;
	uint16_t	delta;

	if(deltaT == 1)
		delta = light.step;
	else if(deltaT ==2)
		delta = light.step << 1;
	else
		delta = (now - m_lastTick) * light.step;

	switch(light.mode)
	{
	case OFF:
		if(light.value <= delta) {
			light.value = 0;
			light.active = false;
		} else
			light.value -= delta;
		Set(li, light.value);
		break;

	case ON:
		if(light.value >= m_maxVal || m_maxVal - light.value <= delta) {
			light.value = m_maxVal;
			light.active = false;
		} else
			light.value += delta;
		Set(li, light.value);
		break;

	case BLINK:
		if(light.up) {
			if(light.value >= m_maxVal || m_maxVal - light.value <= delta) {
				light.value = m_maxVal;
				light.up = false;
			} else
				light.value += delta;
		} else {
			if(light.value <= delta) {
				light.value = 0;
				light.up = true;
			} else
				light.value -= delta;
		}
		Set(li, light.value);
		break;
	}
}

////////////////////////////////////////////////////////////////////
bool SmartLights::SetMode(uint8_t index, Mode mode, uint16_t step)
{
	if(index >= COUNT) return false;
	sg::ItLock	lock;
	State &light = m_states[index];
	light.mode = mode;
	light.step = step;
	light.active = true;

	return true;
}

////////////////////////////////////////////////////////////////////
