/*
 * RFDecoder.cpp
 *
 *  Created on: Jan 13, 2018
 *      Author: compi
 */

#include "RFDecoder.h"
#include <tim.h>
#include <gpio.h>

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
	static uint32_t	callCount = 0;
	++callCount;
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
