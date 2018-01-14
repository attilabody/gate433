/*
 * mainloop.cpp
 *
 *  Created on: Jan 11, 2018
 *      Author: compi
 */
#include "stm32f1xx_hal.h"
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include <RFDecoder.h>
#include <sg/itlock.h>
#include <sg/usart.h>

struct AnalogOutput {
	TIM_HandleTypeDef*	handle;
	uint32_t			channel;
};

AnalogOutput AnalogOuts[6] = {
		{ &htim2, TIM_CHANNEL_1 },
		{ &htim2, TIM_CHANNEL_2 },
		{ &htim2, TIM_CHANNEL_3 },
		{ &htim2, TIM_CHANNEL_4 },
		{ &htim3, TIM_CHANNEL_3 },
		{ &htim3, TIM_CHANNEL_4 },
};

extern "C" void MainLoop();

////////////////////////////////////////////////////////////////////
uint8_t			g_serialBuffer[64];
sg::DbgUsart	&g_com = sg::DbgUsart::Instance();

////////////////////////////////////////////////////////////////////
void MainLoop()
{
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);

	for(auto &out : AnalogOuts) {
		HAL_TIM_PWM_Start(out.handle, out.channel);
		__HAL_TIM_SET_COMPARE(out.handle, out.channel, 0);
	}

	g_com.Init(sg::UsartCallbackDispatcher::Instance(), &huart1, g_serialBuffer, sizeof(g_serialBuffer), true);

	{
		sg::SafeSingletonInitializer<RFDecoder>	si;
	}

	uint32_t	counter = 0;
	while(true)
	{
		uint32_t offset = 0;
		for(auto &out : AnalogOuts) {
			uint32_t curval = (counter+offset) & 0x1fff;
			__HAL_TIM_SET_COMPARE(out.handle, out.channel, curval > 4096 ? 8192 - curval : curval);
			offset += 4096/5;
		}
		counter += 256;
		HAL_Delay(20);
	}
}
