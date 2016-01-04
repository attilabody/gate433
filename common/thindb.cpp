/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include "thindb.h"

#include "config.h"
#include "interface.h"

//////////////////////////////////////////////////////////////////////////////
thindb::thindb( SdFat &sd, bool initialize )
: m_sd( sd )
{
}

//////////////////////////////////////////////////////////////////////////////
thindb::~thindb()
{
}

//////////////////////////////////////////////////////////////////////////////
bool thindb::init()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool thindb::getParams( int code, dbrecord &recout )
{
	bool ret = false;
	static char linebuffer[DBRECORD_WIDTH + 1];
	const char	*bufptr( linebuffer );
	int16_t		tmp1, tmp2;
	File		f;

	f = m_sd.open( "info.txt", FILE_READ );

	if( f.isOpen() )
	{
		if( f.seek( code * DBRECORD_WIDTH )
			&& f.read( linebuffer, DBRECORD_WIDTH ) == DBRECORD_WIDTH )
		{
			linebuffer[DBRECORD_WIDTH] = 0;						//	replace \n with ' '
			ret = true;
		}
		f.close();
	}

	if( !ret ) {
		recout.in_start = -1;
		return false;
	}

	if( (recout.in_start = getintparam( bufptr, false, true )) == - 1
			|| (recout.in_end = getintparam( bufptr, false, true )) ==  -1
			|| (recout.out_start = getintparam( bufptr, false, true )) ==  -1
			|| (recout.out_end = getintparam( bufptr, false, true )) ==  -1
			|| (tmp1 = getintparam( bufptr, false, true )) ==  -1
			|| (tmp2 = getintparam( bufptr, false, true )) ==  -1 )
	{
		recout.in_start = -1;
		return false;
	}
	recout.days = tmp1 & 0x7f;
	recout.position = (dbrecord::POSITION)(tmp2 & 3);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
bool thindb::setParams( int code, const dbrecord &recin )
{
	bool ret( false );
	char infobuffer[DBRECORD_WIDTH + 1];

	recin.serialize( infobuffer );
	Serial.println( infobuffer );

	File		f;

	f = m_sd.open( "info.txt", FILE_WRITE );

	if( f.isOpen() )
	{
		if( f.seek( code * DBRECORD_WIDTH )
			&& f.write( infobuffer, DBRECORD_WIDTH - 1 ) == DBRECORD_WIDTH - 1 )
		{
			ret = true;
		}
		f.close();
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
bool thindb::setStatus( int code, dbrecord::POSITION pos )
{
	char	statusbuffer[STATUS_WIDTH + 1];
	char	*sbptr(statusbuffer);
	bool	ret( false );

	uitohex( sbptr, (uint16_t) pos, 3 );
	File		f;

	f = m_sd.open( "info.txt", FILE_WRITE );

	if( f.isOpen() )
	{
		if( f.seek(code * DBRECORD_WIDTH + STATUS_OFFSET )
			&& f.write( statusbuffer, STATUS_WIDTH ) == STATUS_WIDTH )
		{
			ret = true;
		}
		f.close();
	}
	return ret;
}
