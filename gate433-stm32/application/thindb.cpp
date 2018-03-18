/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include <sg/Strutil.h>
#include "thindb.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
bool thindb::init(const char *name, bool open)
{
	bool ret = m_file.Open(name, static_cast<SdFile::OpenMode>(SdFile::OPEN_ALWAYS | SdFile::WRITE | SdFile::READ)) == FR_OK;
	if(ret && !open) m_file.Close();
	m_name = name;
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
bool thindb::open()
{
	return m_file.Open(m_name, static_cast<SdFile::OpenMode>(SdFile::OPEN_ALWAYS | SdFile::WRITE | SdFile::READ)) == FR_OK;
}

//////////////////////////////////////////////////////////////////////////////
bool thindb::close()
{
	return m_file.IsOpen() ? m_file.Close() == FR_OK : false;
}

//////////////////////////////////////////////////////////////////////////////
bool thindb::getParams( int code, dbrecord &recout )
{
	bool 			ret = false;
	bool 			wasOpen = m_file.IsOpen();
	static char 	linebuffer[DBRECORD_WIDTH + 1];
	const char		*bufptr(linebuffer);
	int16_t			tmp1, tmp2;
	unsigned int	read;

	if(wasOpen || m_file.Open(m_name, static_cast<SdFile::OpenMode>(SdFile::OPEN_ALWAYS | SdFile::READ)) == FR_OK)
	{
		if(m_file.Seek(code * DBRECORD_WIDTH) == FR_OK
			&& m_file.Read(linebuffer, DBRECORD_WIDTH, &read ) == FR_OK
			&& read == DBRECORD_WIDTH)
		{
			linebuffer[DBRECORD_WIDTH] = 0;						//	replace \n with ' '
			ret = true;
		}
		if(!wasOpen)
			m_file.Close();
	}

	if( !ret ) {
		recout.in_start = -1;
		return false;
	}

	if( (recout.in_start = sg::GetIntParam( bufptr, false, true )) == (uint16_t) -1
			|| (recout.in_end = sg::GetIntParam( bufptr, false, true )) == (uint16_t) -1
			|| (recout.out_start = sg::GetIntParam( bufptr, false, true )) == (uint16_t) -1
			|| (recout.out_end = sg::GetIntParam( bufptr, false, true )) == (uint16_t) -1
			|| (tmp1 = sg::GetIntParam( bufptr, false, true )) ==  -1
			|| (tmp2 = sg::GetIntParam( bufptr, false, true )) ==  -1 )
	{
		recout.in_start = -1;
		return false;
	}
	recout.days = tmp1;
	recout.position = (dbrecord::POSITION)(tmp2 & 3);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
bool thindb::setParams( int code, const dbrecord &recin )
{
	return set(code, recin, DBRECORD_WIDTH );
}


//////////////////////////////////////////////////////////////////////////////
bool thindb::set( int code, const dbrecord &recin, uint8_t size )
{
	bool ret = false;
	bool wasOpen = m_file.IsOpen();
	char infobuffer[DBRECORD_WIDTH + 1];

	recin.serialize( infobuffer );

	if(wasOpen || m_file.Open(m_name, static_cast<SdFile::OpenMode>(SdFile::OPEN_ALWAYS | SdFile::WRITE | SdFile::READ)) == FR_OK) {
		if(m_file.Seek(code * DBRECORD_WIDTH ) == FR_OK && m_file.Write(infobuffer, size-1) == FR_OK)	//static_cast<uint8_t>(size-1)) // trailing ' '
			ret = true;
		if(!wasOpen)
			m_file.Close();
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
bool thindb::setInfo( int code, const dbrecord& recin )
{
	return set(code, recin, INFORECORD_WIDTH );
}

//////////////////////////////////////////////////////////////////////////////
bool thindb::setStatus( int code, dbrecord::POSITION pos )
{
	char			statusbuffer[STATUS_WIDTH + 1];
	unsigned int	written;
	bool			ret( false );

	sg::ToHex(statusbuffer, (uint16_t) pos, 3);

	if(m_file.Open(m_name, static_cast<SdFile::OpenMode>(SdFile::OPEN_ALWAYS | SdFile::WRITE | SdFile::READ)) == FR_OK) {
		if(m_file.Seek(code * DBRECORD_WIDTH + STATUS_OFFSET ) == FR_OK
				&& m_file.Write( statusbuffer, STATUS_WIDTH, &written ) == FR_OK
				&& written == STATUS_WIDTH)
			ret = true;
		m_file.Close();
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
void thindb::cleanstatuses()
{
	//TODO: implement it!
}
