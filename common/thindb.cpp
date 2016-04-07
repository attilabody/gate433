/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include "thindb.h"

#include "config.h"
#include "toolbox.h"

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
	SdFile	file;
	if( !file.open( m_sd.vwd(), "DB.TXT", FILE_READ ))
		return false;
	m_dirindex = m_sd.vwd()->curPosition()/32 -1;
	file.close();
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool thindb::getParams( int code, dbrecord &recout )
{
	bool ret = false;
	static char linebuffer[DBRECORD_WIDTH + 1];
	const char	*bufptr( linebuffer );
	int16_t		tmp1, tmp2;
	SdFile		f;

	if( f.open( m_sd.vwd(), m_dirindex, FILE_READ ))
	{
		if( f.seekSet( code * DBRECORD_WIDTH )
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

	if( (recout.in_start = getintparam( bufptr, false, true )) == (uint16_t) -1
			|| (recout.in_end = getintparam( bufptr, false, true )) == (uint16_t) -1
			|| (recout.out_start = getintparam( bufptr, false, true )) == (uint16_t) -1
			|| (recout.out_end = getintparam( bufptr, false, true )) == (uint16_t) -1
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
	return set(code, recin, DBRECORD_WIDTH );
}


//////////////////////////////////////////////////////////////////////////////
bool thindb::set( int code, const dbrecord &recin, uint8_t size )
{
	bool 		ret( false );
	char 		infobuffer[DBRECORD_WIDTH + 1];
	SdFile		f;

	recin.serialize( infobuffer );

	if( f.open( m_sd.vwd(), m_dirindex, O_RDWR | O_CREAT ) )
	{
		if( f.seekSet( code * DBRECORD_WIDTH )
			&& f.write( infobuffer, size - 1 ) == size - 1 )	//	trailing ' '
		{
			ret = true;
		}
		f.close();
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
	char	statusbuffer[STATUS_WIDTH + 1];
	bool	ret( false );

	uitohex( statusbuffer, (uint16_t) pos, 3 );
	SdFile		f;

	if( f.open( m_sd.vwd(), m_dirindex, O_RDWR | O_CREAT ) )
	{
		if( f.seekSet(code * DBRECORD_WIDTH + STATUS_OFFSET )
			&& f.write( statusbuffer, STATUS_WIDTH ) == STATUS_WIDTH )
		{
			ret = true;
		}
		f.close();
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
void thindb::cleanstatuses()
{
	//TODO: implement it!
}
