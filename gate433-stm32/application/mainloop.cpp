/*
 * mainloop.cpp
 *
 *  Created on: Jan 11, 2018
 *      Author: compi
 */
#include "stm32f1xx_hal.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

#include <RFDecoder.h>
#include <sg/itlock.h>


#include "config.h"

#include "mainloop.h"

//#define TESTLOOP
#define TEST_SDCARD
#define TEST_EEPROM
#define TEST_RTC

#if defined(TEST_SDCARD)
#include "SdFile.h"
#endif	//	TEST_SDCARD


extern "C" void _MainLoop();
extern "C" void HAL_SYSTICK_Callback(void);

bool g_mainLoppReady = false;

////////////////////////////////////////////////////////////////////
void _MainLoop()
{
#ifdef TESTLOOP
	MainLoop::Instance().TestLoop();
#else
	MainLoop::Instance().Loop();
#endif
}

////////////////////////////////////////////////////////////////////
void HAL_SYSTICK_Callback(void)
{
	uint32_t	now = HAL_GetTick();
	if(g_mainLoppReady)
		MainLoop::Instance().Tick(now);
}

////////////////////////////////////////////////////////////////////
MainLoop::MainLoop()
: m_com(sg::DbgUsart::Instance())
, m_i2c(&hi2c1, sg::I2cCallbackDispatcher::Instance())
, m_eeprom(m_i2c, EEPROM_I2C_ADDRESS, EEPROM_ADDRESS_LENGTH, 128)
, m_rtc(m_i2c)
, m_lcd(m_i2c, LCD_I2C_ADDRESS)
, m_decoder(RFDecoder::Instance())
{
	m_com.Init(sg::UsartCallbackDispatcher::Instance(), &huart1, m_serialBuffer, sizeof(m_serialBuffer), true);
	m_lcd.Init();
	m_decoder.Init(*this);
}

////////////////////////////////////////////////////////////////////
#define TICK(x) HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); m_com << x << "\r\n"
#define MSG(x)	m_com << x << "\r\n";

////////////////////////////////////////////////////////////////////
void MainLoop::Fail(const char * file, int line)
{
	m_com << file << ": " << (uint32_t)line << "\r\n";
	while(1) {
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(100);
	}
}

#define FAIL() Fail(__FILE__, __LINE__)

////////////////////////////////////////////////////////////////////
void MainLoop::DumpBufferLine(uint16_t base, uint16_t offset, uint8_t count)
{
	m_com << sg::DbgUsart::hex << sg::DbgUsart::pad << (uint16_t)(base + offset);
	while(count--) {
		m_com << ' ' << m_eepromBuffer[offset++];
	}
	m_com << sg::DbgUsart::endl;

}

////////////////////////////////////////////////////////////////////
void MainLoop::LineReceived(uint16_t count)
{
	uint8_t	*bufPtr = m_eepromBuffer + count -1;
	while(bufPtr != m_eepromBuffer && (*bufPtr == '\r' || *bufPtr == '\n' || !*bufPtr))
		--bufPtr;
	if(bufPtr < m_eepromBuffer + count -1)
		*(bufPtr + 1) = 0;
	m_lineReceived = true;
}

////////////////////////////////////////////////////////////////////
void MainLoop::CodeReceived(uint16_t code)
{
	if(!m_codeReceived) {
		m_code = code;
		m_codeReceived = true;
	}
}

////////////////////////////////////////////////////////////////////
#ifdef TESTLOOP
void MainLoop::TestLoop()
{
	HAL_StatusTypeDef	ret = HAL_OK;
	bool				firstCode = true;

	for(uint8_t cnt = 0; cnt < 16; ++cnt) {
	  uint32_t start = SysTick->VAL;
	  while(SysTick->VAL - start < 1000 );
	  TICK('.');
	}
	m_com << "\r\n";

#ifdef TEST_SDCARD
	uint32_t byteswritten, bytesread;                     /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs\r\n"; /* File write buffer */
	uint8_t rtext[100];                                   /* File read buffer */

	uint32_t fpos = 0;
	SdFile sdFileW;

	m_lcd.Print("SD card");

	TICK("openw1");
	if(sdFileW.Open("STM32.TXT", SdFile::OpenMode::WRITE_TRUNC)) {
		SdFile sdFileR;
		TICK("openr1");
		if(sdFileR.Open("FILE.TXT", SdFile::OpenMode::READ)) {
			TICK("read1");
			if((bytesread = sdFileR.Read(rtext, sizeof(rtext))) != 0) {
				m_com << sg::DbgUsart::Buffer(rtext, bytesread) << "\r\n";
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
								m_com.Send(rtext, bytesread);
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

	m_lcd.Clear();
	m_lcd.Print("EEPROM");

#ifdef TEST_EEPROM
	for(uint16_t i=0; i < sizeof(m_eepromBuffer); ++i)
		m_eepromBuffer[i] = 0xff-i;

	ret = m_eeprom.Write(sizeof(m_eepromBuffer), m_eepromBuffer, sizeof(m_eepromBuffer));
	m_eeprom.Sync();
	memset(m_eepromBuffer, 0xaa, sizeof(m_eepromBuffer));
	ret = m_eeprom.Read(0, m_eepromBuffer, sizeof(m_eepromBuffer));
	m_eeprom.Sync();
	for(uint16_t offset = 0; offset < sizeof(m_eepromBuffer); offset += 32)
		DumpBufferLine(0, offset, 32);
	m_com << sg::DbgUsart::endl;

	ret = m_eeprom.Read(sizeof(m_eepromBuffer), m_eepromBuffer, sizeof(m_eepromBuffer));
	m_eeprom.Sync();
	for(uint16_t offset = 0; offset < sizeof(m_eepromBuffer); offset += 32)
		DumpBufferLine(sizeof(m_eepromBuffer), offset, 32);
	m_com << sg::DbgUsart::endl;

	uint16_t	base = 0;
#endif	//	TEST_EEPROM

#ifdef TEST_RTC
	using Ts = sg::DS3231::Ts;
	ret = m_rtc.Init();
	Ts		ts;
	bool	desync = false;
#endif	//	TEST_RTC
	ret = m_com.Receive(m_eepromBuffer, sizeof(m_eepromBuffer), *this);

	uint32_t	counter = 0;
	while(true)
	{
		uint32_t offset = 0;
		for(uint8_t u = 0; u < m_lights.COUNT; ++u) {
			uint32_t curval = (counter+offset) & 0x1fff;
			m_lights.Set(u, curval > 4096 ? 8192 - curval : curval);
			offset += 4096/5;
		}
		counter += 256;

		auto firstTick = HAL_GetTick();
		do {
			if(m_lineReceived) {
				m_com << m_eepromBuffer << "\r\n";
				m_lineReceived = false;
				ret = m_com.Receive(m_eepromBuffer, sizeof(m_eepromBuffer), *this);
			}
		} while(HAL_GetTick() - firstTick < 20 );

		if(counter >= 8192)
		{
			counter -= 8192;
#ifdef TEST_RTC
			ret = m_rtc.Get(ts, desync);
			m_com << sg::DbgUsart::dec << (ts.hour < 10 ? "0" : "") << ts.hour << (ts.min < 10 ? ":0" : ":") << ts.min << (ts.sec < 10 ? ":0" : ":") << ts.sec << "  ";
#else
			TICK('#');
#endif
			if(m_codeReceived) {
				auto code = m_code;
				m_codeReceived = false;
				m_com << code;
				if(firstCode) {
					m_lcd.Clear();
					firstCode = false;
				} else
					m_lcd.SetCursor(0, 0);
				m_lcd.Print(code, false, 4);
			}
			m_com << sg::DbgUsart::endl;

			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
#if defined(TEST_EEPROM)
			m_com << sg::DbgUsart::endl;
			ret = m_eeprom.Read(base, m_eepromBuffer, sizeof(m_eepromBuffer));
			m_eeprom.Sync();
			for(uint16_t offset = 0; offset < sizeof(m_eepromBuffer); offset += 32)
				DumpBufferLine(base, offset, 32);
			m_com << sg::DbgUsart::endl << sg::DbgUsart::endl;
			base += 256;
#endif
		}
	}
	(void)ret;
}
#endif	//	TESTLOOP

////////////////////////////////////////////////////////////////////
void MainLoop::Tick(uint32_t now)
{
	m_lights.Tick(now);
}

////////////////////////////////////////////////////////////////////
void MainLoop::Loop()
{
	uint32_t		oldTick;
	uint32_t		now, lastLedTick, tsTick;
	bool			desync;
	sg::DS3231::Ts	ts, old;
	char			timebuf[10];
	
	MainLoop::Instance();
	g_mainLoppReady = true;

	{
		m_rtc.Get(old, desync);
		do {
			m_rtc.Get(ts, desync);
		} while(ts.sec == old.sec);
		tsTick = HAL_GetTick();
		old = ts;
	}
	oldTick = tsTick;
	lastLedTick = tsTick;

	m_lights.SetMode(0, SmartLights::BLINK, 128);
	m_lights.SetMode(1, SmartLights::BLINK, 120);
	m_lights.SetMode(2, SmartLights::BLINK, 150);
	m_lights.SetMode(3, SmartLights::BLINK, 100);
	m_lights.SetMode(4, SmartLights::BLINK, 180);
	m_lights.SetMode(5, SmartLights::BLINK, 220);

	while(true)
	{
		now = HAL_GetTick();

		if(m_codeReceived) {
			auto code = m_code;
			m_codeReceived = false;
			m_com << code;
			m_lcd.SetCursor(0, 0);
			m_lcd.Print(code, false, 4);
		}

		if(now != oldTick)
		{
			if(now - tsTick > 980) {
				m_rtc.Get(ts, desync);
				if(old.sec != ts.sec) {
					tsTick = now;
					old = ts;
					m_lcd.SetCursor(0, 1);
					ts.TimeToString(timebuf, sizeof(timebuf), true);
					m_lcd.Print(timebuf);
				}
			}

			if(now - lastLedTick >= 500) {
				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
				lastLedTick = now;
			}

			oldTick = now;
		}
	}
}
