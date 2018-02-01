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
public:
	struct IDecoderCallback {
		virtual void CodeReceived(uint16_t code) = 0;
	};
	HAL_StatusTypeDef Init(IDecoderCallback* callback);

private:
	RFDecoder() = default;

	void PeriodEllapsed(TIM_HandleTypeDef *htim);
	void CaptureCallback(TIM_HandleTypeDef *htim);

	void ProcessPeriod(bool level, uint16_t length);

	TIM_HandleTypeDef &m_htim = htim1;

	uint16_t	m_lastCapture = 0;
	uint16_t	m_lastHLength = 0;
	uint16_t	m_lastLLength = 0;
	uint8_t		m_overflow = 0;

	uint16_t	m_syncLength = 0;
	static const uint16_t SYNCLENGTH_MIN = 8000;
	static const uint16_t SYNCLENGTH_MAX = 15000;
	uint16_t	m_minShort;
	uint16_t	m_maxShort;
	uint16_t	m_minLong;
	uint16_t	m_maxLong;

	bool		m_lowLong;
	uint16_t	m_code, m_bits;
	uint16_t	m_lastDecoded;
	IDecoderCallback *m_callback = nullptr;
};

#endif /* __cplusplus */
#endif /* RFDECODER_H_ */
