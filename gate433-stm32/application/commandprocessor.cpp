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
#include <sg/Strutil.h>

#include "database.h"
#include "thindb.h"
#include "config.h"

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
void MainLoop::CommandProcessor::Process(sg::Usart &com, char * const buffer)
{
	m_bufPtr = buffer;

	if(IsCommand(CMD_GDB)) {	// get dbrecord
		database::dbrecord	rec;
		int32_t id = GetParam();
		if( id != -1 && m_parent.m_db.getParams(id, rec)) {
			rec.serialize(static_cast<char * const>(buffer));
			PrintResp(com, buffer);
		} else
			PrintRespErr(com);

	} else if(IsCommand(CMD_SDB)) {	//	set dbrecord
		database::dbrecord	rec;
		int id = GetParam();
		if( id != -1 && rec.parse(m_bufPtr)) {
			if( m_parent.m_db.setParams( id, rec ))
				PrintRespOk(com);
			else
				PrintRespErr(com);
		} else
			PrintRespErr(com);

	} else if(IsCommand(CMD_EDB)) {	//	export database
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
			PrintRespOk(com);
		else
			PrintRespErr(com);

	} else if(IsCommand(CMD_IDB)) {	//	import database
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
					com << '+';
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
		com << sg::Usart::endl;
		if(id)
			PrintRespOk(com);
		else
			PrintRespErr(com);
		com << changed << sg::Usart::endl;

	} else if(IsCommand(CMD_DDB)) {	//	dump database
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
				sg::ToDec(buffer, id, 4);
				buffer[4] = ' ';
				rec.serialize( buffer + 5);
				com << DATA << buffer << sg::Usart::endl;
			} else break;
		}
		if(id == to + 1)
			PrintRespOk(com);
		else
			PrintRespErr(com);

	} else if(IsCommand(CMD_GDT)) {	//	get datetime
		auto &dt = m_parent.m_rtcDateTime;
		com << com.dec << com.nopad << RESP << (uint16_t)dt.year << '.' << sg::Usart::Pad(2) << (uint16_t)dt.mon << '.' <<
			(uint16_t)dt.mday << '/' << (uint16_t)dt.wday << "  " << dt.hour << ':' << dt.min << ':' << (uint16_t)dt.sec <<
			sg::Usart::endl << com.nopad;
	} else if(IsCommand(CMD_SDT)) {	// set datetime
		sg::DS3231_DST::Ts	t;
		if(GetDateTime(t)) {
			m_parent.m_rtc.Set(t);
			PrintRespOk(com);
		} else
			com << ERR << " DTFMT" << sg::Usart::endl;

	} else if(IsCommand(CMD_CS)) {	// clear statuses
		uint16_t from(GetParam());
		uint16_t to( GetParam());
		uint16_t id;

		if(from > MAX_CODE && to > MAX_CODE) {
			from = 0;
			to = MAX_CODE;
		} else {
			if(from > MAX_CODE) from = 0;
			if(to > MAX_CODE ) to = from;
		}
		if(from > to) from = to;

		for(id = from; id <= to; ++id) {
			//CHECKPOINT;
			if(!m_parent.m_db.setStatus(id, database::dbrecord::UNKNOWN ))
				break;
		}
		if(id != to + 1)
			PrintRespErr(com);
		else
			PrintRespOk(com);

	} else if(IsCommand(CMD_DS)) {	// dump shuffle
		SdFile	f;
		char buffer[16];
		if( f.Open("SHUFFLE.TXT", static_cast<SdFile::OpenMode>(SdFile::OPEN_EXISTING | SdFile::READ)) == FR_OK) {
			while( GetLine( f, buffer, sizeof(buffer)) != -1 ) {
				//CHECKPOINT;
				PrintResp(com, buffer);
			}
			PrintResp(com, "");
			f.Close();
		} else {
			PrintRespErr(com, "CANTOPEN");
		}

	} else if(IsCommand(CMD_DL)) {	// dump log
		if(m_parent.m_log.dump(com, false))
			PrintRespOk(com);
		else
			PrintRespErr(com);

	} else if(IsCommand(CMD_TL)) {	//	truncate log
		if(m_parent.m_log.truncate())
			PrintRespOk(com);
		else
			PrintRespErr(com);

	} else if(IsCommand(CMD_IL)) {
		//TODO: infinite loop (watchdog test)

	} else if(IsCommand(CMD_SL)) {	//set lights
		bool inner = false;
		bool valid = false;
		if(*m_bufPtr == 'i') {
			inner = valid = true;
			++m_bufPtr;
		} else if(*m_bufPtr == 'o') {
			valid = true;
			++m_bufPtr;
		}
		if(valid) {
			uint8_t mode(GetParam());
			m_parent.m_lights.SetMode(static_cast<States>(mode), inner);
			PrintRespOk(com);
		} else
			PrintRespErr(com);

	} else if(IsCommand(CMD_GT)) {
		int16_t	rawTemp;
		if(m_parent.m_rtc.GetTreg(rawTemp) == HAL_OK) {
			com << RESP << (int16_t)(rawTemp >> 2) << com.endl;
		} else
			PrintRespErr(com);

	} else {
		com << ERR << " CMD " << buffer << sg::Usart::endl;
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

