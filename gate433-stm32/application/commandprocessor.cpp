/*
 * callbackprocessor.cpp
 *
 *  Created on: Feb 5, 2018
 *      Author: compi
 */
#include "mainloop.h"
#include "commands.h"
//#include "commsyms.h" from mainloop.h
#include <ctype.h>
#include <sg/strutil.h>

#include "database.h"
#include "thindb.h"

////////////////////////////////////////////////////////////////////
MainLoop::CommandProcessor::CommandProcessor(MainLoop &parent) : m_parent(parent)
{
}

////////////////////////////////////////////////////////////////////
bool MainLoop::CommandProcessor::IsCommand(const char *command)
{
	uint16_t n = 0;
	char c;
	while((c = *command++) && c == m_bufPtr[n] )
			++n;
	char b = m_bufPtr[n];
	if(!c && (isspace(b) || ispunct(b) || !b )) {
		m_bufPtr += n;
		while( *m_bufPtr && (isspace(*m_bufPtr)))
			++m_bufPtr;
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////
void MainLoop::CommandProcessor::Process(const char* input)
{
	m_bufPtr = input;

	if(IsCommand(CMD_GDB)) {
		database::dbrecord	rec;
		int32_t id = GetParam();
		if( id != -1 && m_parent.m_db.getParams(id, rec)) {
			rec.serialize(m_parent.m_serialBuffer);
			PrintResp(m_parent.m_serialBuffer);
		} else
			PrintRespErr();

	} else if(IsCommand(CMD_SDB)) {
		database::dbrecord	rec;
		int id = GetParam();
		if( id != -1 && rec.parse(m_bufPtr)) {
			if( m_parent.m_db.setParams( id, rec ))
				PrintRespOk();
			else
				PrintRespErr();
		} else
			PrintRespErr();

	} else if(IsCommand(CMD_EDB)) {
		thindb		tdb;
		uint16_t	from = GetParam();
		uint16_t	to = GetParam();
		uint16_t	id = 0;
		database::dbrecord	rec;
		if(from == 0xffff) from = 0;
		if(to == 0xffff) to = MAX_CODE;
		if(tdb.init("DB.TXT", true)) {
			for(id = from; id <= to; ++id) {
				//CHECKPOINT;
				if(!m_parent.m_db.getParams(id, rec) || !tdb.setParams(id, rec))
					break;
			}
			tdb.close();
		}
		if(id == to + 1)
			PrintRespOk();
		else
			PrintRespErr();

	} else if(IsCommand(CMD_IDB)) {
		uint16_t changed = 0, id = 0;
		uint16_t from = GetParam();
		uint16_t to = GetParam();
		if(to > MAX_CODE) to = MAX_CODE;
		if(from == 0xffff) from = 0;
		else if(from > to) from = to;
		//uint16_t	imported(importdb(from, to));
		thindb tdb;
		database::dbrecord rec, old;
		if(tdb.init("DB.TXT", true)) {
			for(id = from; id <= to; ++id) {
				if(!tdb.getParams(id, rec) || !m_parent.m_db.getParams(id, old))
					break;
				if(!rec.infoequal(old))
				{
					m_parent.m_com << '+';
					if(!m_parent.m_db.setParams(id, rec))
						break;
					else
						changed++;
				}
			}
			if(id != to+1)
				id = 0;
			tdb.close();
		}
		m_parent.m_com << sg::Usart::endl;
		if(id)
			PrintRespOk();
		else
			PrintRespErr();
		m_parent.m_com << changed << sg::Usart::endl;

	} else if(IsCommand(CMD_DDB)) {
		database::dbrecord	rec;
		uint16_t from = GetParam();
		uint16_t to = GetParam();
		uint16_t id;

		if(from > MAX_CODE) from = 0;
		if(to > MAX_CODE ) to = MAX_CODE;
		if(from > to) from = to;

		for( id = from; id <= to; ++id ) {
			//CHECKPOINT;
			if( m_parent.m_db.getParams(id, rec)) {
				sg::todec(m_parent.m_serialBuffer, id, 4);
				m_parent.m_serialBuffer[4] = ' ';
				rec.serialize( m_parent.m_serialBuffer + 5);
				m_parent.m_com << DATA << m_parent.m_serialBuffer << sg::Usart::endl;
			} else break;
		}
		if( id == to + 1 ) PrintRespOk();
		else PrintRespErr();

	} else if(IsCommand(CMD_GDT)) {
		m_parent.m_com << RESP << (uint16_t)m_parent.m_rtcDateTime.year << '.' << (uint16_t)m_parent.m_rtcDateTime.mon << '.' <<
				(uint16_t)m_parent.m_rtcDateTime.mday << '/' << (uint16_t)m_parent.m_rtcDateTime.wday << "  " <<
				m_parent.m_rtcDateTime.hour << ':' << m_parent.m_rtcDateTime.min << ':' << (uint16_t)m_parent.m_rtcDateTime.sec <<
				sg::Usart::endl;
	} else if(IsCommand(CMD_SDT)) {
		sg::DS3231_DST::Ts	t;
		if(GetDateTime(t)) {
			m_parent.m_rtc.Set(t);
			PrintRespOk();
		} else
			m_parent.m_com << ERR << " DTFMT" << sg::Usart::endl;

	} else if(IsCommand(CMD_CS)) {
		uint16_t from(GetParam());
		uint16_t to( GetParam());
		uint16_t id;

		if(from > MAX_CODE) from = 0;
		if(to > MAX_CODE ) to = MAX_CODE;
		if(from > to) from = to;

		for(id = from; id <= to; ++id) {
			//CHECKPOINT;
			if(!m_parent.m_db.setStatus(id, database::dbrecord::UNKNOWN ))
				break;
		}
		if(id != to + 1)
			PrintRespErr();
		else
			PrintRespOk();

	} else if(IsCommand(CMD_DS)) {
		SdFile	f;
		char buffer[16];
		if( f.Open("SHUFFLE.TXT", static_cast<SdFile::OpenMode>(SdFile::OPEN_EXISTING | SdFile::READ)) == FR_OK) {
			while( GetLine( f, buffer, sizeof(buffer)) != -1 ) {
				//CHECKPOINT;
				PrintResp(buffer);
			}
			PrintResp("");
			f.Close();
		} else {
			PrintRespErr("CANTOPEN");
		}
	} else if(IsCommand(CMD_DL)) {
		if(m_parent.m_log.dump(m_parent.m_com, false))
			PrintRespOk();
		else
			PrintRespErr();
	} else if(IsCommand(CMD_TL)) {
		//TODO: truncate log
	} else if(IsCommand(CMD_IL)) {
		//TODO: infinite loop (watchdog test)
	} else if(IsCommand(CMD_SL)) {
		bool inner = false;
		bool valid = false;
		if(*m_bufPtr == 'i' || *m_bufPtr == 'I' ) {
			inner = valid = true;
			++m_bufPtr;
		} else if(*m_bufPtr == 'o' || *m_bufPtr == 'O' ) {
			valid = true;
			++m_bufPtr;
		}
		if(valid) {
			uint8_t mode(GetParam());
			m_parent.m_lights.SetMode(static_cast<States>(mode), inner);
			PrintRespOk();
		} else
			PrintRespErr();
	} else {
		m_parent.m_com << ERR << " CMD" << sg::Usart::endl;
	}
}

//////////////////////////////////////////////////////////////////////////////
bool MainLoop::CommandProcessor::GetDateTime(sg::DS3231_DST::Ts &t)
{
	//	"2015.10.28 16:37:05"
	t.year = GetParam();
	if( t.year == (decltype(t.year))-1 ) return false;
	t.mon = GetParam();
	if( t.mon == (decltype(t.mon))-1 ) return false;
	t.mday = GetParam();
	if( t.mday == (decltype(t.mday))-1 ) return false;
	t.hour = GetParam();
	if( t.hour == (decltype(t.hour))-1 ) return false;
	t.min = GetParam();
	if( t.min == (decltype(t.min))-1 ) return false;
	t.sec = GetParam();
	if( t.sec == (decltype(t.sec))-1 ) t.sec = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////////////
uint16_t MainLoop::CommandProcessor::GetLine( SdFile &f, char* buffer, uint8_t buflen )
{
	uint32_t		curpos( f.Ftell());
	char 			*src( buffer );
	unsigned int	rb( f.Read( buffer, buflen - 1 ));
	uint16_t		ret(0);

	if(!rb) return -1;

	while(rb--) {
		char inc = *src;
		if(!inc || inc == '\r' || inc == '\n') {
			ret = src - buffer;
			*src++ = 0;
			if(inc && rb) {	//if the termination was not zero and there is at least one character in the buffer
				char prev = inc;
				inc = *src;
				if(!inc || (prev != inc &&(inc == '\r' || inc == '\n')))
					++src;
			}
			break;
		}
		++src;
	}

	if(rb == (unsigned int)-1) {
		*src = 0;
		ret = src - buffer;
	}
	f.Seek(curpos + (src - buffer));
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

