/*
 * RFDecoder.cpp
 *
 *  Created on: Jan 13, 2018
 *      Author: compi
 */

#include <tim.h>
#include <gpio.h>
#include <rfdecoder.h>

////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef RFDecoder::Init(IDecoderCallback &callback)
{
	HAL_StatusTypeDef	res = HAL_OK;

	m_callback = &callback;

	if((res = HAL_TIM_Base_Start_IT(&htim1)) != HAL_OK)
		return res;
	if((res = HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1)) != HAL_OK)
		return res;
	return HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
}

////////////////////////////////////////////////////////////////////
void RFDecoder::PeriodEllapsed(TIM_HandleTypeDef *htim)
{
	if(m_overflow < 0xff)
		++m_overflow;
}

////////////////////////////////////////////////////////////////////
void RFDecoder::CaptureCallback(TIM_HandleTypeDef *htim)
{
	uint16_t	currentCapture, length;
	bool		level;

	switch(htim->Channel)
	{
	case HAL_TIM_ACTIVE_CHANNEL_1:
		currentCapture = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
		level = true;
		break;

	case HAL_TIM_ACTIVE_CHANNEL_2:
		currentCapture = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
		level = false;
		break;

	default:
		return;
	}

	length = ((m_overflow == 1 && currentCapture > m_lastCapture) ||  m_overflow > 1) ? 0xffff : currentCapture - m_lastCapture;

	ProcessPeriod(level, m_overflow < 2 ? length : 0xffff);

	m_lastCapture = currentCapture;
	m_lastLength = length;
	m_overflow = 0;
}

////////////////////////////////////////////////////////////////////
void RFDecoder::ProcessPeriod(bool level, uint16_t length)
{
	bool highLong;

	if(!m_syncLength) {	//looking for sync
		if(level && length > SYNCLENGTH_MIN && length < SYNCLENGTH_MAX && m_lastLength == 0xffff)
		{	//high pulse with appropriate length
			m_syncLength = length;
			m_code = 0;
			m_bits = 0;
			m_minShort = length >> 1;								// 0.5
			m_maxShort = length + (length >> 2);					// 1 + 0.25 = 1.25
			m_minLong = length + (length >> 1) - (length >> 3);		// 1 + 0.5 - 0.125 = 1.325
			m_maxLong = length << 1;								// 2
		}
	}
	else
	{
		if(length < m_minShort || (length > m_maxShort && length < m_minLong) || length > m_maxLong) {
			//invalid length, back to sync seek
			m_syncLength = 0;
		}
		else
		{
			if( !level ) {	//	low pulse
				m_lowLong = length > m_maxShort;
			} else
			{	//	high pulse
				highLong = length >= m_minLong;
				if(m_lowLong == highLong)
					m_syncLength = 0;	//both half are the same => invalid
				else {
					if( !m_lowLong )	//	1
						m_code |= 1 << m_bits;
					if(++m_bits == 12) {
						m_lastDecoded = m_code;
						m_syncLength = 0;
						if(m_callback)
							m_callback->CodeReceived(m_lastDecoded);
						//TODO: notify
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	RFDecoder::Instance().PeriodEllapsed(htim);
}

////////////////////////////////////////////////////////////////////
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	RFDecoder::Instance().CaptureCallback(htim);
}
