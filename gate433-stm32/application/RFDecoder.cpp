/*
 * RFDecoder.cpp
 *
 *  Created on: Jan 13, 2018
 *      Author: compi
 */

#include "RFDecoder.h"
#include <tim.h>
#include <gpio.h>

#include <sg/usart.h>

extern sg::DbgUsart	&g_com;

////////////////////////////////////////////////////////////////////
RFDecoder::RFDecoder()
{
	HAL_StatusTypeDef	res = HAL_OK;

	res = HAL_TIM_Base_Start_IT(&htim1);
	res = HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
	res = HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
}

////////////////////////////////////////////////////////////////////
void RFDecoder::PeriodEllapsed(TIM_HandleTypeDef *htim)
{
	static uint32_t	callCount = 0;
	++callCount;
}

////////////////////////////////////////////////////////////////////
void RFDecoder::CaptureCallback(TIM_HandleTypeDef *htim)
{
	uint32_t	currentCapture;

	switch(htim->Channel)
	{
	case HAL_TIM_ACTIVE_CHANNEL_1:
		currentCapture = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1);
		break;

	case HAL_TIM_ACTIVE_CHANNEL_2:
		currentCapture = __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_2);
		break;

	default:
		return;
	}

	g_com.Send(currentCapture, true, false, false);
	g_com.Send("\r\n", 2);

//	if(m_syncing)
//	{
//		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
//		{
//
//		}
//	}
//	else
//	{
//
//	}
	m_lastCapture = currentCapture;
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
