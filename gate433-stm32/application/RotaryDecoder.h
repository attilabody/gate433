/*
 * rotarydecoder.h
 *
 *  Created on: Mar 15, 2018
 *      Author: compi
 */

#ifndef ROTARYDECODER_H_
#define ROTARYDECODER_H_

#include "stm32f1xx_hal.h"

class RotaryDecoder {
public:
	struct IRotaryConsumer {
		virtual void Step(bool up) = 0;
		virtual void Click(bool on) = 0;
	};

	RotaryDecoder(
			IRotaryConsumer *consumer, uint8_t pulsesPerStep,
			GPIO_TypeDef *clkPort, uint16_t clkPin,
			GPIO_TypeDef *dataPort, uint16_t dataPin,
			GPIO_TypeDef *switchPort, uint16_t switchPin
	);

	void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

private:
	IRotaryConsumer	*m_consumer;
	GPIO_TypeDef	*m_clkPort, *m_dataPort, *m_switchPort;
	uint16_t		m_clkPim, m_dataPin, m_switchPin;
	uint8_t			m_pulsesPerStep;

	uint32_t		m_lastClkTick = 0;
	uint32_t		m_lastSwTick = 0;
	bool			m_lastStepWasUp = true;
	uint8_t			m_stepCounter = 0;
};

#endif /* ROTARYDECODER_H_ */
