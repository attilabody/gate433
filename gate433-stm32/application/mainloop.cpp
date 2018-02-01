/*
 * mainloop.cpp
 *
 *  Created on: Jan 11, 2018
 *      Author: compi
 */
#include "stm32f1xx_hal.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include <RFDecoder.h>
#include <sg/itlock.h>
#include <sg/usart.h>


#include "config.h"

#define TEST_SDCARD
#define TEST_EEPROM
#define TEST_RTC

#if defined(TEST_SDCARD)
#include "SdFile.h"
#endif	//	TEST_SDCARD

#if defined(TEST_EEPROM)
#include <sg/i2ceeprom.h>
#endif

#if defined(TEST_RTC)
#include <sg/ds3231.h>
#endif

#include "RFDecoder.h"

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

#if defined(TEST_EEPROM) || defined(TEST_RTC)
sg::I2cMaster	g_i2c(&hi2c1, &sg::I2cCallbackDispatcher::Instance());
#endif

#ifdef TEST_EEPROM
sg::I2cEEPROM	g_eeprom(g_i2c, EEPROM_I2C_ADDRESS, EEPROM_I2C_ADDRESS_LENGTH, 128);
#endif	//	TEST_EEPROM

#ifdef TEST_RTC
sg::DS3231_DST	g_rtc(g_i2c);
#endif	//	TEST_RTC

uint8_t			g_buffer[256];
volatile bool	g_lineReceived = false;

uint16_t		g_code;
bool			g_codeReceived;

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
void DumpBufferLine(uint16_t base, uint16_t offset, uint8_t count)
{
	g_com << sg::DbgUsart::hex << sg::DbgUsart::pad << (uint16_t)(base + offset);
	while(count--) {
		g_com << ' ' << g_buffer[offset++];
	}
	g_com << sg::DbgUsart::endl;

}

////////////////////////////////////////////////////////////////////
struct ReceiverCallback : public sg::DbgUsart::IReceiverCallback
{
	virtual void LineReceived(uint16_t count)
	{
		uint8_t	*bufPtr = g_buffer + count -1;
		while(bufPtr != g_buffer && (*bufPtr == '\r' || *bufPtr == '\n' || !*bufPtr))
			--bufPtr;
		if(bufPtr < g_buffer + count -1)
			*(bufPtr + 1) = 0;
		g_lineReceived = true;
	}
} g_lrcb;

struct CodeReceivedCallback : public RFDecoder::IDecoderCallback
{
	virtual void CodeReceived(uint16_t code) {
		if(!g_codeReceived) {
			g_code = code;
			g_codeReceived = true;
		}
	}
} g_crcb;

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
	RFDecoder::Instance().Init(&g_crcb);


	for(uint8_t cnt = 0; cnt < 16; ++cnt) {
	  uint32_t start = SysTick->VAL;
	  while(SysTick->VAL - start < 1000 );
	  TICK('.');
	}
	g_com << "\r\n";

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
	(void)fpos;
#endif

#ifdef TEST_EEPROM
	for(uint16_t i=0; i < sizeof(g_buffer); ++i)
		g_buffer[i] = 0xff-i;

	ret = g_eeprom.Write(sizeof(g_buffer), g_buffer, sizeof(g_buffer));
	g_eeprom.Sync();
	memset(g_buffer, 0xaa, sizeof(g_buffer));
	ret = g_eeprom.Read(0, g_buffer, sizeof(g_buffer));
	g_eeprom.Sync();
	for(uint16_t offset = 0; offset < sizeof(g_buffer); offset += 32)
		DumpBufferLine(0, offset, 32);
	g_com << sg::DbgUsart::endl;

	ret = g_eeprom.Read(sizeof(g_buffer), g_buffer, sizeof(g_buffer));
	g_eeprom.Sync();
	for(uint16_t offset = 0; offset < sizeof(g_buffer); offset += 32)
		DumpBufferLine(sizeof(g_buffer), offset, 32);
	g_com << sg::DbgUsart::endl;

	uint16_t	base = 0;
#endif	//	TEST_EEPROM

#ifdef TEST_RTC
	using Ts = sg::DS3231::Ts;
	ret = g_rtc.Init();
	Ts		ts;
	bool	desync = false;
#endif	//	TEST_RTC
	ret = g_com.Receive(g_buffer, sizeof(g_buffer), &g_lrcb);

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

		auto firstTick = HAL_GetTick();
		do {
			if(g_lineReceived) {
				g_com << g_buffer << "\r\n";
				g_lineReceived = false;
				ret = g_com.Receive(g_buffer, sizeof(g_buffer), &g_lrcb);
			}
		} while(HAL_GetTick() - firstTick < 20 );

		if(counter >= 8192) {
			counter -= 8192;
#ifdef TEST_RTC
			ret = g_rtc.Get(ts, desync);
			g_com << sg::DbgUsart::dec << (ts.hour < 10 ? "0" : "") << ts.hour << (ts.min < 10 ? ":0" : ":") << ts.min << (ts.sec < 10 ? ":0" : ":") << ts.sec << "  ";
#else
			TICK('#');
#endif
			if(g_codeReceived) {
				g_com << g_code;
				g_codeReceived = false;
			}
			g_com << sg::DbgUsart::endl;

			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
#if defined(TEST_EEPROM)
			g_com << sg::DbgUsart::endl;
			ret = g_eeprom.Read(base, g_buffer, sizeof(g_buffer));
			g_eeprom.Sync();
			for(uint16_t offset = 0; offset < sizeof(g_buffer); offset += 32)
				DumpBufferLine(base, offset, 32);
			g_com << sg::DbgUsart::endl << sg::DbgUsart::endl;
			base += 256;
#endif
		}
	}
	(void)ret;
}
