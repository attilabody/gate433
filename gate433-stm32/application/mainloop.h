/*
 * mainloop.h
 *
 *  Created on: Feb 1, 2018
 *      Author: compi
 */

#ifndef MAINLOOP_H_
#define MAINLOOP_H_
#include <rfdecoder.h>
#include <sdfile.h>
#include <sg/singleton.h>
#include <sg/usart.h>
#include <sg/i2cmaster.h>
#include <sg/i2ceeprom.h>
#include <sg/ds3231.h>
#include <sg/i2c_lcd.h>
#include <sg/strutil.h>
#include <smartlights.h>

#include "i2cdb.h"
#include "commsyms.h"


class MainLoop : public sg::Singleton<MainLoop>, public RFDecoder::IDecoderCallback, public sg::Usart::IReceiverCallback
{
	friend class sg::Singleton<MainLoop>;

public:
	MainLoop();

	_Noreturn void TestLoop();
	_Noreturn void Loop();
	void Tick(uint32_t now);

private:
	char			m_serialOutRingBuffer[32];
	char			m_serialBuffer[32];
	sg::Usart		m_com;
	sg::I2cMaster	m_i2c;
	sg::I2cEEPROM	m_dbEeprom;
	i2cdb			m_db;
	sg::I2cEEPROM	m_configEeprom;
	sg::DS3231_DST	m_rtc;
	sg::I2cLcd		m_lcd;

	RFDecoder		&m_decoder;
	SmartLights		m_lights;

	static const uint16_t	MAX_CODE = 1023;


	virtual void LineReceived(char *buffer, uint16_t count);
	volatile bool	m_lineReceived = false;

	virtual void CodeReceived(uint16_t code);
	uint16_t		m_code = 0;
	volatile bool	m_codeReceived = false;

	//	utility functions
	void Fail(const char * file, int line);
	void DumpBufferLine(uint8_t *buffer, uint16_t base, uint16_t offset, uint8_t count);

	class CommandProcessor
	{
		friend class MainLoop;
	public:
		CommandProcessor(MainLoop &parent);
		void Process(const char *input);

	private:
		MainLoop &m_parent;
		static const uint16_t	MAX_CODE = MainLoop::MAX_CODE;

		const char *m_bufPtr = nullptr;
		bool IsCommand(const char *command);
		void PrintResp(const char *resps) { m_parent.m_com << RESP << resps << sg::Usart::endl; }
		void PrintErr() { m_parent.m_com << ERR << sg::Usart::endl; }
		void PrintErr(const char *errs) { m_parent.m_com << ERR << errs << sg::Usart::endl; }
		void PrintRespOk() { m_parent.m_com << "OK" << sg::Usart::endl; }
		int32_t GetParam(bool decimal = true, bool trimstart = true, bool acceptneg = false) { return sg::getintparam(m_bufPtr, decimal, trimstart, acceptneg); }
		bool GetDateTime(sg::DS3231_DST::Ts &t);
		uint16_t GetLine( SdFile &f, char* buffer, uint8_t buflen );
	};

	CommandProcessor	m_proc;

	sg::DS3231::Ts	m_ts;
};

#endif /* MAINLOOP_H_ */
