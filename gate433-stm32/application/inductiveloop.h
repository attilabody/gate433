/*
 * lightshandler.h
 *
 *  Created on: Nov 18, 2015
 *      Author: abody
 */

#ifndef INDUCTIVELOOP_H_
#define INDUCTIVELOOP_H_
#include <sg/stm32_hal.h>
#include <tick.h>

class InductiveLoop : public ITick
{
public:
	enum STATUS : uint8_t { NONE = 0, OUTER = 1, INNER = 2 };

			InductiveLoop(GPIO_TypeDef* innerPort, uint16_t innerPin, GPIO_TypeDef* outerPort, uint16_t outerPin, bool activeHigh);
	// returns true in case of conflict (both loops are active)
	const STATUS&	GetStatus() { return m_status; }
	bool GetConflict() { return m_conflict; }
	virtual void Tick(uint32_t now);


private:
	bool	Debounce(bool inner, uint32_t now);

	STATUS			m_status;
	bool			m_conflict;
	GPIO_TypeDef	*m_innerPort, *m_outerPort;
	uint16_t		m_innerPin, m_outerPin;
	bool			m_activeHigh;
	bool			m_debounced[2];
	unsigned long	m_lastEquals[2];

	static const uint16_t	DEBOUNCE_MILLISECONDS = 250;
};

#endif /* INDUCTIVELOOP_H_ */
