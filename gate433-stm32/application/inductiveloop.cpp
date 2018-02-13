/*
 * lightshandler.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: abody
 */

#include <inductiveloop.h>

////////////////////////////////////////////////////////////////////
InductiveLoop::InductiveLoop(GPIO_TypeDef* innerPort, uint16_t innerPin, GPIO_TypeDef* outerPort, uint16_t outerPin, bool activeHigh)
: m_status(NONE)
, m_conflict(false)
, m_innerPort(innerPort)
, m_outerPort(outerPort)
, m_innerPin(innerPin)
, m_outerPin(outerPin)
, m_activeHigh(activeHigh)
, m_debounced {false, false}
, m_lastEquals {0, 0}
{
}

////////////////////////////////////////////////////////////////////
void InductiveLoop::Tick(uint32_t now)
{
	uint8_t rawstatus = 0;

	if(Debounce(true, now) == m_activeHigh) rawstatus |= (uint8_t) INNER;
	if(Debounce(false, now) == m_activeHigh) rawstatus |= (uint8_t) OUTER;

	if( rawstatus == ( INNER | OUTER )) {
		if( m_status == NONE )
			m_status = INNER;
	} else
		m_status = (STATUS)rawstatus;

	m_conflict = (rawstatus == ( INNER | OUTER ));
}

////////////////////////////////////////////////////////////////////
bool InductiveLoop::Debounce(bool inner, uint32_t now)
{
	bool level = HAL_GPIO_ReadPin(inner ? m_innerPort : m_outerPort, inner ? m_innerPin : m_outerPin) == GPIO_PIN_SET;

	if(m_debounced[inner] == level) {
		 m_lastEquals[inner] = now;
	} else if(now - m_lastEquals[inner] > DEBOUNCE_MILLISECONDS) {
		m_debounced[inner] = level;
		m_lastEquals[inner] = now;
	}
	return m_debounced[inner];
}
