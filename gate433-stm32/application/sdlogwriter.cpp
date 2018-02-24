/*
 * sdfatlogwriter.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: abody
 */

#include <sdfile.h>
#include <sdlogwriter.h>
#include <sg/ds3231.h>
#include "commsyms.h"

//extern uint16_t	g_lastcheckpoint;

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
SdLogWriter::sdfwbuffer::sdfwbuffer(const char *name,  void *buf, uint8_t size)
: writebuffer( buf, size )
{
	if(m_f.Open(name, static_cast<SdFile::OpenMode>(SdFile::OPEN_ALWAYS|SdFile::WRITE)) == FR_OK)
		m_f.Seek(m_f.Size());
	//TODO: Error handling
}

/////////////////////////////////////////////////////////////////////////////
SdLogWriter::sdfwbuffer::~sdfwbuffer()
{
	flush();
	m_f.Close();
}

/////////////////////////////////////////////////////////////////////////////
bool SdLogWriter::sdfwbuffer::flush()
{
	bool ret = true;
	if( m_index ) {
		unsigned int written;
		ret = m_f.Write( m_buffer, m_index, &written) == FR_OK && written == m_index;
		m_index = 0;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
bool SdLogWriter::sdfwbuffer::write(sg::DS3231::Ts &dt )
{
	return
		   write( dt.year , 4 )
		&& writebuffer::write( '.' )
		&& write( dt.mon, 2 )
		&& writebuffer::write( '.' )
		&& write( dt.mday, 2 )
		&& writebuffer::write( ' ' )
		&& write( dt.hour, 2 )
		&& writebuffer::write(':' )
		&& write( dt.min, 2 )
		&& writebuffer::write( ':' )
		&& write( dt.sec, 2 );
}

//////////////////////////////////////////////////////////////////////////////
bool SdLogWriter::sdfwbuffer::write( uint16_t data, uint8_t digits )
{
	char buf[5];
	if( digits > 5 ) digits = 5;
	char *ptr( buf + digits - 1 );
	uint8_t	cntr( digits );
	while( cntr-- ) {
		*ptr-- = ( data%10 ) + '0';
		data /= 10;
	}
	return writebuffer::write( buf, digits );
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
SdLogWriter::SdLogWriter(const char *name)
: m_name(name)
{};

/////////////////////////////////////////////////////////////////////////////
void SdLogWriter::log(CATEGORY category, sg::DS3231::Ts &datetime, const char* message, uint16_t rid, uint8_t btn, uint8_t dbpos, uint8_t loop,  uint8_t decision, char reason )
{
	char		buffer[32];

	sdfwbuffer	b(m_name, buffer, sizeof(buffer));
	writelinehdr(b, category, datetime, rid, btn, dbpos, loop, decision, reason);
	b.writebuffer::write( message );
	b.writebuffer::write( '\n' );
}

///////////////////////////////////////////////////////////////////////////////
bool SdLogWriter::dump(sg::Usart &com, bool trunc)
{
	SdFile			f;
	char			buffer[32], *bptr;
	unsigned int	nio;
	char			lastPrinted = 0, prevPrinted = '\n';
	FRESULT			fr = FR_OK;

	if(f.Open(m_name, static_cast<SdFile::OpenMode>(SdFile::OPEN_EXISTING | SdFile::READ )) == FR_OK)
	{
		do
		{
			if((fr = f.Read( buffer, sizeof( buffer ), &nio)) != FR_OK || !nio )
				break;
			bptr = buffer;
			for(uint8_t bc = nio; bc != 0; --bc)
			{
				if(prevPrinted == '\n') {
					//CHECKPOINT;
					com << DATA;
				}
				lastPrinted = *bptr++;
				if(lastPrinted == '\n' && prevPrinted != '\r')
					com << '\r';
				com << lastPrinted;
				prevPrinted = lastPrinted;
			}
		} while(fr == FR_OK && nio);
		f.Close();

		if(lastPrinted != '\n' ) {
			if(prevPrinted != '\r')
				com << '\r';
			com << '\n';
		}
	}
	return fr == FR_OK;
}

/////////////////////////////////////////////////////////////////////////////
bool SdLogWriter::truncate()
{
	SdFile		f;
	if(f.Open(m_name, static_cast<SdFile::OpenMode>(SdFile::CREATE_ALWAYS|SdFile::WRITE)) == FR_OK) {
		f.Close();
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
const char* SdLogWriter::__catsrts = "DBG" "INF" "WRN" "ERR" "WTF";
const char* SdLogWriter::__positions = "UOI";
const char* SdLogWriter::__states = "OFF" "CDW" "CON" "ACC" "WRN" "DNY" "UNR" "HRY" "PAS";

/////////////////////////////////////////////////////////////////////////////
bool SdLogWriter::writelinehdr(sdfwbuffer &wb, CATEGORY c, sg::DS3231::Ts &datetime, uint16_t remoteid, uint8_t btn, uint8_t dbpos, uint8_t loop, uint8_t decision, char reason )
{
	bool ret( true );
	ret &= wb.write( datetime );
	ret &= wb.writebuffer::write( ' ' );
	ret &= wb.writebuffer::write( __catsrts + c * 3, 3 );
	ret &= wb.writebuffer::write( ' ' );
	if( remoteid != 0xffff ) {
		ret &= wb.write( remoteid, 4 );
		ret &= wb.writebuffer::write(' ');
	}
	if( btn != 0xff ) {
		ret &= wb.write( btn, 1 );
		ret &= wb.writebuffer::write(' ');
	}
	if( dbpos != 0xff ) {
		ret &= wb.writebuffer::write( __positions + dbpos, 1 );
		ret &= wb.writebuffer::write(' ');
	}
	if( loop != 0xff ) {
		ret &= wb.writebuffer::write( loop ? 'I' : 'O' );
		ret &= wb.writebuffer::write(' ');
	}
	if( decision != 0xff ) {
		ret &= wb.writebuffer::write( __states + decision * 3, 3 );
		ret &= wb.writebuffer::write(' ');
	}
	if(reason != ' ') {
		ret &= wb.writebuffer::write(' ');
		ret &= wb.writebuffer::write(reason);
	}
	return ret;
}
