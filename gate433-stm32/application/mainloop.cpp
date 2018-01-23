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
#include <sg/i2ceeprom.h>

#include "SdFile.h"

#include "config.h"

//#define TEST_SDCARD

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

sg::I2cMaster	g_i2c(&hi2c1, &sg::I2cCallbackDispatcher::Instance());
sg::I2cEEPROM	g_eeprom(g_i2c, EEPROM_I2C_ADDRESS, EEPROM_I2C_ADDRESS_LENGTH, 128);

uint8_t	g_bigFancyBuffer[256];


////////////////////////////////////////////////////////////////////
#define TICK(x) HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); g_com << x << "\r\n"
#define MSG(x)	g_com << x << "\r\n";

////////////////////////////////////////////////////////////////////
void Fail(const char * file, int line)
{
	g_com << file << ": " << (uint32_t)line << "\r\n";
	while(1) {
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(100);
	}
}

#define FAIL() Fail(__FILE__, __LINE__)

////////////////////////////////////////////////////////////////////
void MainLoop()
{
	HAL_StatusTypeDef	ret = HAL_OK;

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);

	for(auto &out : AnalogOuts) {
		HAL_TIM_PWM_Start(out.handle, out.channel);
		__HAL_TIM_SET_COMPARE(out.handle, out.channel, 0);
	}

	g_com.Init(sg::UsartCallbackDispatcher::Instance(), &huart1, g_serialBuffer, sizeof(g_serialBuffer), true);
	RFDecoder::Instance().Init();


	for(uint8_t cnt = 0; cnt < 16; ++cnt) {
	  uint32_t start = SysTick->VAL;
	  while(SysTick->VAL - start < 1000 );
	  TICK('.');
	}

#ifdef TEST_SDCARD
	uint32_t byteswritten, bytesread;                     /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs\r\n"; /* File write buffer */
	uint8_t rtext[100];                                   /* File read buffer */

	uint32_t fpos = 0;
	SdFile sdFileW;

	TICK("openw1");
	if(sdFileW.Open("STM32.TXT", SdFile::OpenMode::WRITE_TRUNC)) {
		SdFile sdFileR;
		TICK("openr1");
		if(sdFileR.Open("FILE.TXT", SdFile::OpenMode::READ)) {
			TICK("read1");
			if((bytesread = sdFileR.Read(rtext, sizeof(rtext))) != 0) {
				g_com << sg::DbgUsart::Buffer(rtext, bytesread) << "\r\n";
				fpos = sdFileR.Ftell();
				TICK("write1.1");
				byteswritten = sdFileW.Write(wtext, sizeof(wtext)-1);
				TICK("closer1");
				sdFileR.Close();
				if(byteswritten) {
					TICK("closew1");
					sdFileW.Close();
					if(sdFileW.Open("STM32.TXT", SdFile::OpenMode::WRITE_APPEND)) {
						TICK("write1.2");
						byteswritten = sdFileW.Write(rtext, bytesread);
						TICK("closew1");
						sdFileW.Close();
					}
					TICK("openr2");
					if(sdFileR.Open("STM32.TXT", SdFile::OpenMode::READ)) {
						do {
							TICK("read2");
							if((bytesread = sdFileR.Read(rtext, sizeof(rtext))))
								g_com.Send(rtext, bytesread);
						} while(bytesread != 0);
						TICK("closer2");
						sdFileR.Close();
					} else FAIL();
				} else FAIL();
			} else FAIL();
		} else FAIL();
	} else FAIL();
#endif

	for(uint16_t i=0; i < sizeof(g_bigFancyBuffer); ++i)
		g_bigFancyBuffer[i] = 0xff-i;

	ret = g_eeprom.Write(sizeof(g_bigFancyBuffer), g_bigFancyBuffer, sizeof(g_bigFancyBuffer));
	g_eeprom.Sync();
	memset(g_bigFancyBuffer, 0xaa, sizeof(g_bigFancyBuffer));
	ret = g_eeprom.Read(0, g_bigFancyBuffer, sizeof(g_bigFancyBuffer));
	g_eeprom.Sync();
	ret = g_eeprom.Read(sizeof(g_bigFancyBuffer), g_bigFancyBuffer, sizeof(g_bigFancyBuffer));
	g_eeprom.Sync();

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
		if(counter >= 8192) {
			counter -= 8192;
			TICK('#');
		}
	}
}
