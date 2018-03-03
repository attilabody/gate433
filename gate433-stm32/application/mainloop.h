/*
 * mainloop.h
 *
 *  Created on: Feb 1, 2018
 *      Author: compi
 */

#ifndef MAINLOOP_H_
#define MAINLOOP_H_
#include <display.h>
#include <rfdecoder.h>
#include <sdfile.h>
#include <sdlogwriter.h>
#include <sg/singleton.h>
#include <sg/usart.h>
#include <sg/i2cmaster.h>
#include <sg/i2ceeprom.h>
#include <sg/ds3231.h>
#include <sg/i2c_lcd.h>
#include <sg/strutil.h>
#include "states.h"
#include "trafficlights.h"
#include "i2cdb.h"
#include "inductiveloop.h"
#include "commsyms.h"


class MainLoop : public sg::Singleton<MainLoop>, public RFDecoder::IDecoderCallback, public sg::Usart::IReceiverCallback
{
	friend class sg::Singleton<MainLoop>;

public:
	MainLoop();
	virtual ~MainLoop() = default;


	_Noreturn void TestLoop();
	_Noreturn void Loop();
	void Tick(uint32_t now);

private:
	States Authorize(uint16_t id, bool inner);
	void ChangeState(States newStatus, bool inner, uint32_t now);//, bool ilChanged);

	TrafficLights	m_lights;
	sg::Usart		m_com;
	sg::Usart		m_wifi;
	sg::I2cMaster	m_i2c;
	sg::I2cEEPROM	m_dbEeprom;
	i2cdb			m_db;
	sg::I2cEEPROM	m_configEeprom;
	sg::DS3231_DST	m_rtc;
	Display			m_lcd;

	RFDecoder		&m_decoder;
	InductiveLoop	m_loop;
	SdLogWriter		m_log;

	States			m_state = States::OFF;
	uint32_t		m_stateStartedTick = 0;
	bool			m_cycleInner = false;

	static const uint8_t	LINEBUFFER_SIZE = 32;
	char			m_serialOutRingBuffer[32];
	char			m_serialBuffer[LINEBUFFER_SIZE];
	char			m_wifiOutRingBuffer[32];
	char			m_wifiBuffer[LINEBUFFER_SIZE];

	static const uint16_t	MAX_CODE = 1023;

	virtual void LineReceived(char *buffer, uint16_t count);
	volatile bool	m_serialLineReceived = false;
	volatile bool	m_wifiLineReceived = false;

	virtual void CodeReceived(uint16_t code);
	uint16_t		m_code = 0;
	uint16_t		m_countedCode = 0xffff;
	volatile bool	m_codeReceived = false;

	static const uint8_t CODE_LOG_QUEUE_SIZE = 4;
	uint16_t		m_codeLogQueue[CODE_LOG_QUEUE_SIZE];
	uint8_t			m_codeLogQueueIndex = 0;
	uint16_t		m_lastCodeReceived = 0xffff;
	uint32_t		m_lastCodeReceivedTick = 0;

	sg::DS3231::Ts	m_rtcDateTime;
	bool			m_rtcDesync = false;
	uint32_t		m_rtcTick = 0;

	InductiveLoop::STATUS	m_ilStatus = InductiveLoop::NONE;
	bool					m_ilConflict = false;


	//	utility functions
	void Fail(const char * file, int line);
	//void DumpBufferLine(uint8_t *buffer, uint16_t base, uint16_t offset, uint8_t count);
	bool CheckDateTime(uint32_t now);

	class CommandProcessor
	{
		friend class MainLoop;
	public:
		CommandProcessor(MainLoop &parent);
		void Process(sg::Usart &com, char * const buffer);

	private:
		MainLoop &m_parent;
		static const uint16_t	MAX_CODE = MainLoop::MAX_CODE;

		const char *m_bufPtr = nullptr;

		void PrintResp(sg::Usart &com, const char *resps) { com << RESP << resps << sg::Usart::endl; }
		void PrintRespErr(sg::Usart &com) { com << ERR << "ERR" << sg::Usart::endl; }
		void PrintRespErr(sg::Usart &com, const char *errs) { com << ERR << "ERR " << errs << sg::Usart::endl; }
		void PrintRespOk(sg::Usart &com) { com << RESP << "OK" << sg::Usart::endl; }

		bool IsCommand(const char *command);
		int32_t GetParam(bool decimal = true, bool trimstart = true, bool acceptneg = false) { return sg::getintparam(m_bufPtr, decimal, trimstart, acceptneg); }
		bool GetDateTime(sg::DS3231_DST::Ts &t);
		uint16_t GetLine(SdFile &f, char* buffer, uint8_t buflen);
	};

	CommandProcessor	m_proc;

};

#endif /* MAINLOOP_H_ */
