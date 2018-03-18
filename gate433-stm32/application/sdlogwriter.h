/*
 * sdfatlogwriter.h
 *
 *  Created on: Jan 26, 2016
 *      Author: abody
 */

#ifndef SDLOGWRITER_H_
#define SDLOGWRITER_H_

#include <sdfile.h>
#include <sg/Ds3231.h>
#include <sg/Usart.h>
#include "logwriter.h"
#include "writebuffer.h"


class SdFile;

class SdLogWriter : public logwriter
{
public:
	SdLogWriter(const char *name);

	void log(CATEGORY category, const sg::DS3231::Ts &dateTime, const char* message,
		uint16_t rid = 0xffff, uint8_t button = 0xff, uint8_t dbPos = 0xff,
		uint8_t loop = 0xff, uint8_t decision = 0xff, char reason = ' ' );

	bool dump(sg::Usart &com, bool trunc = false);
	bool truncate();

	bool Queue(CATEGORY category, sg::DS3231::Ts &dateTime, const char* message,
			uint16_t rid = 0xffff, uint8_t button = 0xff, uint8_t dbPos = 0xff,
			uint8_t loop = 0xff, uint8_t decision = 0xff, char reason = ' ' );

	bool LogFromQueue(uint8_t maxCount);

protected:
	class sdfwbuffer: public writebuffer
	{
	public:
		sdfwbuffer(const char *name, void *buf, uint8_t size);
		~sdfwbuffer();
		bool write( uint16_t data, uint8_t digits );
		bool write( const sg::DS3231::Ts &dt );
		virtual bool flush();
	private:
		SdFile	m_f;
	};

	const char 	*m_name;

	struct LogData {
		CATEGORY category; sg::DS3231::Ts dateTime; char message[64];
		uint16_t rid; uint8_t button; uint8_t dbPos;
		uint8_t loop; uint8_t decision; char reason;
	};

	static const uint8_t LOGBUFFERCOUNT = 8;
	LogData m_logBuffer[LOGBUFFERCOUNT];
	uint8_t	m_readPos = 0;
	uint8_t	m_free = LOGBUFFERCOUNT;

	void Log(const LogData &l);

	inline uint8_t WritePos();

	bool writelinehdr(
			sdfwbuffer &wb, CATEGORY c, const sg::DS3231::Ts &datetime, uint16_t remoteid,
			uint8_t btn, uint8_t dbpos, uint8_t loop, uint8_t decision, char reason);

	static const char* __catsrts;
	static const char* __positions;
	static const char* __states;

};

#endif /* SDLOGWRITER_H_ */
