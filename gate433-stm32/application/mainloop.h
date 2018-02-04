/*
 * mainloop.h
 *
 *  Created on: Feb 1, 2018
 *      Author: compi
 */

#ifndef MAINLOOP_H_
#define MAINLOOP_H_
#include <sg/singleton.h>
#include <sg/usart.h>
#include <sg/i2cmaster.h>
#include <sg/i2ceeprom.h>
#include <sg/ds3231.h>
#include <sg/i2c_lcd.h>

#include "RFDecoder.h"
#include "SmartLights.h"

class MainLoop : public sg::Singleton<MainLoop>, public RFDecoder::IDecoderCallback, public sg::DbgUsart::IReceiverCallback
{
	friend class sg::Singleton<MainLoop>;

public:
	MainLoop();

	_Noreturn void TestLoop();
	_Noreturn void Loop();
	void Tick(uint32_t now);

private:
	uint8_t			m_serialBuffer[128];
	sg::DbgUsart	&m_com;
	sg::I2cMaster	m_i2c;
	sg::I2cEEPROM	m_eeprom;
	sg::DS3231_DST	m_rtc;
	sg::I2cLcd		m_lcd;

	RFDecoder		&m_decoder;
	SmartLights		m_lights;


	uint8_t			m_eepromBuffer[256];

	virtual void LineReceived(uint16_t count);
	volatile bool	m_lineReceived = false;

	virtual void CodeReceived(uint16_t code);
	uint16_t		m_code = 0;
	volatile bool	m_codeReceived = false;

	//	utility functions
	void Fail(const char * file, int line);
	void DumpBufferLine(uint16_t base, uint16_t offset, uint8_t count);

};

#endif /* MAINLOOP_H_ */
