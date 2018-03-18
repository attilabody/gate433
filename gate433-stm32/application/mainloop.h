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
#include <sg/Singleton.h>
#include <sg/Usart.h>
#include <sg/I2cMaster.h>
#include <sg/Ds3231.h>
#include <sg/I2cLcd.h>
#include <sg/I2cEeprom.h>
#include <sg/Strutil.h>
#include "states.h"
#include "trafficlights.h"
#include "pulsingoutput.h"
#include "i2cdb.h"
#include "inductiveloop.h"
#include "RotaryDecoder.h"
#include "commsyms.h"


class MainLoop
		: public sg::Singleton<MainLoop>
		, public RFDecoder::IDecoderCallback
		, public sg::Usart::IReceiverCallback
		, public RotaryDecoder
		, public RotaryDecoder::IRotaryConsumer
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
	PulsingOutput	m_gate;
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

	// IReceiverCallback
	virtual void LineReceived(char *buffer, uint16_t count);
	volatile bool	m_serialLineReceived = false;
	volatile bool	m_wifiLineReceived = false;

	// IDecoderCallback
	virtual void CodeReceived(uint16_t code);
	uint16_t		m_code = 0xffff;
	uint16_t		m_countedCode = 0xffff;
	volatile bool	m_codeReceived = false;

	// IRotaryDecoder
	virtual void Step(bool up);
	virtual void Click(bool on);
	volatile int32_t	m_rotaryCounter = 0;
	volatile bool 		m_rotaryButton = false;

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
	bool CheckDateTime(uint32_t now);
	void UpdateDow(sg::DS3231::Ts &ts);

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
		int32_t GetParam(bool decimal = true, bool trimstart = true, bool acceptneg = false) { return sg::GetIntParam(m_bufPtr, decimal, trimstart, acceptneg); }
		bool GetDateTime(sg::DS3231_DST::Ts &t);
		uint16_t GetLine(SdFile &f, char* buffer, uint8_t buflen);
	};

	CommandProcessor	m_proc;

};

#endif /* MAINLOOP_H_ */
