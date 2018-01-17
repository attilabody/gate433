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

#include "sd_diskio.h"
#include "diskio.h"

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

FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */
FIL MyFile;     /* File object */

uint8_t	g_sdBuffer[512];


////////////////////////////////////////////////////////////////////
#define TICK() 	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13)
#define MSG(x)	g_com.Send(x);g_com.Send("\r\n",2);

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
	RFDecoder::Instance().Init();

	uint8_t	ret;

	for(uint8_t cnt = 0; cnt < 16; ++cnt) {
	  uint32_t start = SysTick->VAL;
	  while(SysTick->VAL - start < 1000 );
	  TICK();
	}

	if((ret = FATFS_LinkDriver(&SD_Driver, SDPath)) == 0)
	{
		uint32_t byteswritten, bytesread;                     /* File write/read counts */
		uint8_t wtext[] = "This is STM32 working with FatFs\r\n"; /* File write buffer */
		uint8_t rtext[100];                                   /* File read buffer */

		TICK();
		if((ret = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0)) != FR_OK) { /* FatFs Initialization Error */
			Error_Handler();
		}

		MSG("openw");
		TICK();
		if((ret = f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK)
		{
			MSG("write1");
			TICK();
			if((ret = f_write(&MyFile, wtext, sizeof(wtext), (UINT *)&byteswritten)) != FR_OK
					|| byteswritten == 0)
			{
				f_close(&MyFile);
				MSG("write1fail");
				Error_Handler();
			}

			MSG("openr1");
			TICK();

			FIL MyFile2;     /* File object */

			if((ret = f_open(&MyFile2, "FILE.TXT", FA_READ)) == FR_OK)
			{
				MSG("read1");
				TICK();
				ret = f_read(&MyFile2, rtext, sizeof(rtext), (UINT*)&bytesread);

				if((bytesread == 0) || (ret != FR_OK)) { /* 'STM32.TXT' file Read or EOF Error */
					MSG("read1fail")
				}
				else
				{
					rtext[bytesread] = 0;
					g_com << sg::DbgUsart::Buffer(rtext, bytesread) << "\r\n";
				}
				MSG("closer1");
				TICK();
				if((ret = f_close(&MyFile2)) != FR_OK) {
					MSG("closer1fai");
					Error_Handler();
				}
			}
			else
			{
				MSG("openr1fail")
			}

			MSG("write2");
			TICK();
			if((ret = f_write(&MyFile, wtext, sizeof(wtext), (UINT *)&byteswritten)) != FR_OK
					|| byteswritten == 0)
			{
				f_close(&MyFile);
				MSG("write2fail");
				Error_Handler();
			}

			MSG("closew");
			TICK();
			if (f_close(&MyFile) != FR_OK ) {
				MSG("closewfail");
				Error_Handler();
			}

			MSG("openr2");
			TICK();
			if(f_open(&MyFile, "STM32.TXT", FA_READ) == FR_OK)
			{
				MSG("read2");
				TICK();
				ret = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);

				if((bytesread == 0) || (ret != FR_OK)) {
					MSG("read2fail");
				} else {
					g_com << sg::DbgUsart::Buffer(rtext, bytesread) << "\r\n";
				}

				MSG("closer2");
				TICK();
				if((ret = f_close(&MyFile)) != FR_OK) {
					MSG("closer2fail");
					Error_Handler();
				}
			}
			else
			{
				MSG("openr2fail");
			}

			MSG("unlink");
			TICK();
			if(( ret = f_unlink("STM32.TXT")) != FR_OK) {
				MSG("unlinkfail");
			}
		}
		else
		{
			MSG("openwfail");
			Error_Handler();
		}

		TICK();
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
