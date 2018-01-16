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
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */

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
		uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
		uint8_t rtext[100];                                   /* File read buffer */

		TICK();
		/*##-2- Register the file system object to the FatFs module ##############*/
		if((ret = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0)) != FR_OK) { /* FatFs Initialization Error */
			Error_Handler();
		}
		else
		{
			MSG("openr1");
			TICK();
			/*##-7- Open the text file object with read access ###############*/
			if((ret = f_open(&MyFile, "FILE.TXT", FA_READ)) == FR_OK)
			{
				MSG("read1");
				TICK();
				/*##-8- Read data from the text file ###########################*/
				ret = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);

				if((bytesread == 0) || (ret != FR_OK)) { /* 'STM32.TXT' file Read or EOF Error */
					MSG("read1fail")
				}
				else
				{
					/*##-9- Close the open text file #############################*/
					rtext[bytesread] = 0;
					g_com.Send(rtext, bytesread);
					g_com.Send("\r\n", 2);
				}
				MSG("closer1");
				TICK();
				if((ret = f_close(&MyFile)) != FR_OK) {
					MSG("closer1fai");
					Error_Handler();
				}
			}
			else
			{
				MSG("openr1fail")
			}

			/*##-4- Create and Open a new text file object with write access #####*/
			MSG("openw");
			TICK();
			if((ret = f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE)) == FR_OK)
			{
				MSG("write");
				TICK();
				/*##-5- Write data to the text file ################################*/
				ret = f_write(&MyFile, wtext, sizeof(wtext), (UINT *)&byteswritten);

				/*##-6- Close the open text file #################################*/
				MSG("closew");
				TICK();
				if (f_close(&MyFile) != FR_OK ) {
					MSG("closewfail");
					Error_Handler();
				}

				if((byteswritten == 0) || (ret != FR_OK)) { /* 'STM32.TXT' file Write or EOF Error */
					MSG("writefail");
				}
				else
				{
					MSG("openr2");
					TICK();
					/*##-7- Open the text file object with read access ###############*/
					if(f_open(&MyFile, "STM32.TXT", FA_READ) == FR_OK)
					{ /* 'STM32.TXT' file Open for read Error */
						MSG("read2");
						TICK();
						/*##-8- Read data from the text file ###########################*/
						ret = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);

						if((bytesread == 0) || (ret != FR_OK)) { /* 'STM32.TXT' file Read or EOF Error */
							MSG("read2fail");
						}
						else
						{ /*##-9- Close the open text file #############################*/
							g_com.Send(rtext, bytesread);
							g_com.Send("\r\n", 2);
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
