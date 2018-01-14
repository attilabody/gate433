/*
 * RFDecoder.h
 *
 *  Created on: Jan 13, 2018
 *      Author: compi
 */

#ifndef RFDECODER_H_
#define RFDECODER_H_

#include "stm32f1xx_hal.h"
#include <sg/singleton.h>

#ifdef __cplusplus
extern "C" {
#endif

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

extern "C" TIM_HandleTypeDef htim1;

class RFDecoder : public sg::Singleton<RFDecoder>
{
	friend class Singleton<RFDecoder>;
	friend void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
	friend void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);

private:
	RFDecoder();

	void PeriodEllapsed(TIM_HandleTypeDef *htim);
	void CaptureCallback(TIM_HandleTypeDef *htim);

	TIM_HandleTypeDef &m_htim = htim1;

	uint16_t	m_lastCapture = 0;
	uint16_t	m_lastHLength = 0;
	uint16_t	m_lastLLength = 0;
};

#endif /* __cplusplus */
#endif /* RFDECODER_H_ */
