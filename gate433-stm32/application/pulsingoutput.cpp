/*
 * PulsingOutput.cpp
 *
 *  Created on: Mar 10, 2018
 *      Author: compi
 */

#include <pulsingoutput.h>

PulsingOutput::PulsingOutput(GPIO_TypeDef *port, uint16_t pin, bool activeHigh)
: m_port(port)
, m_pin(pin)
, m_activeHigh(activeHigh)
{
	HAL_GPIO_WritePin(m_port, m_pin, m_activeHigh ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

////////////////////////////////////////////////////////////////////
void PulsingOutput::Set(uint16_t pulseLen, uint16_t cycleLen, uint32_t now)
{
	if(pulseLen && cycleLen > pulseLen) {
		m_pulseLen = pulseLen;
		m_cycleLen = cycleLen;
	} else
		m_cycleLen = m_pulseLen = 0;

	m_changedAt = now;

	HAL_GPIO_WritePin(m_port, m_pin, m_activeHigh ? GPIO_PIN_SET : GPIO_PIN_RESET);
	m_active = true;
}

////////////////////////////////////////////////////////////////////
void PulsingOutput::Reset()
{
	HAL_GPIO_WritePin(m_port, m_pin, m_activeHigh ? GPIO_PIN_RESET : GPIO_PIN_SET);
	m_active = false;
}

////////////////////////////////////////////////////////////////////
void PulsingOutput::Tick(uint32_t now)
{
	if(m_active && m_pulseLen)
	{
		bool active = HAL_GPIO_ReadPin(m_port, m_pin) == (m_activeHigh ? GPIO_PIN_SET : GPIO_PIN_RESET);
		if(active && now - m_changedAt >= m_pulseLen) {
			m_changedAt += m_pulseLen;
			HAL_GPIO_TogglePin(m_port, m_pin);
		} else if(!active && now - m_changedAt >= (uint32_t)m_cycleLen - (uint32_t)m_pulseLen) {
			m_changedAt += m_cycleLen - m_pulseLen;
			HAL_GPIO_TogglePin(m_port, m_pin);
		}
	}
}
