/*
 * rotarydecoder.cpp
 *
 *  Created on: Mar 15, 2018
 *      Author: compi
 */
#include "stm32f1xx_hal.h"
#include <RotaryDecoder.h>

////////////////////////////////////////////////////////////////////
RotaryDecoder::	RotaryDecoder(
		IRotaryConsumer *consumer,
		uint8_t pulsesPerStep,
		GPIO_TypeDef *clkPort,
		uint16_t clkPin,
		GPIO_TypeDef *dataPort,
		uint16_t dataPin,
		GPIO_TypeDef *switchPort,
		uint16_t switchPin
)
: m_consumer(consumer)
, m_clkPort(clkPort)
, m_dataPort(dataPort)
, m_switchPort(switchPort)
, m_clkPim(clkPin)
, m_dataPin(dataPin)
, m_switchPin(switchPin)
, m_pulsesPerStep(pulsesPerStep)
{}

////////////////////////////////////////////////////////////////////
void RotaryDecoder::HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint32_t now = HAL_GetTick();

	if(GPIO_Pin == BTN_CLK_Pin) {
		bool rising = HAL_GPIO_ReadPin(BTN_CLK_GPIO_Port, BTN_CLK_Pin) == GPIO_PIN_SET;

		if(now - m_lastClkTick > 1) {
			bool data = HAL_GPIO_ReadPin(BTN_DATA_GPIO_Port, BTN_DATA_Pin) == GPIO_PIN_SET;
			bool dir = rising != data;
			if(m_lastStepWasUp == dir)
				++m_stepCounter;
			else
				m_stepCounter = 1;
			m_lastStepWasUp = dir;

			if(m_stepCounter == m_pulsesPerStep) {
				m_stepCounter = 0;
				m_consumer->Step(dir);
			}
		}
		m_lastClkTick = now;
	}
	else if(GPIO_Pin == BTN_SW_Pin) {
		bool rising = HAL_GPIO_ReadPin(BTN_SW_GPIO_Port, BTN_SW_Pin) == GPIO_PIN_SET;

		if(now - m_lastSwTick > 1)
			m_consumer->Click(!rising);

		m_lastSwTick = now;

	}
}

