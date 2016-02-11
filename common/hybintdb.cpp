/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include "hybintdb.h"
#include "config.h"
#include "toolbox.h"
#include "EEPROM.h"

//////////////////////////////////////////////////////////////////////////////
hybintdb::hybintdb( SdFat &sd, bool initialize )
: m_sd( sd )
{
	if( initialize ) init();
}

//////////////////////////////////////////////////////////////////////////////
hybintdb::~hybintdb()
{
}

//////////////////////////////////////////////////////////////////////////////
bool hybintdb::init()
{
	SdFile	file;
	if( !file.open( m_sd.vwd(), "info.txt", FILE_READ ))
		return false;
	m_dirindex = m_sd.vwd()->curPosition()/32 -1;
	file.close();
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool hybintdb::getParams( int code, dbrecord &recout )
{
	bool ret = false;
	static char linebuffer[INFORECORD_WIDTH + 1];
	const char	*bufptr( linebuffer );
	int16_t		tmp;
	SdFile		f;


	if( f.open( m_sd.vwd(), m_dirindex, FILE_READ ))
	{
		if( f.seekSet( code * INFORECORD_WIDTH )
			&& f.read( linebuffer, INFORECORD_WIDTH ) == INFORECORD_WIDTH )
		{
			linebuffer[INFORECORD_WIDTH] = 0;						//	replace \n with \0
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
			|| (tmp = getintparam( bufptr, false, true )) ==  -1 )
	{
		recout.in_start = -1;
		return false;
	}

	recout.days = tmp & 0x7f;
	recout.position = (dbrecord::POSITION)( EEPROM.read(code ) & 3);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool hybintdb::setParams( int code, const dbrecord &recin )
{
	bool ret( false );
	char infobuffer[DBRECORD_WIDTH + 1];

	recin.serialize( infobuffer );
	Serial.println( infobuffer );

	SdFile		f;

	if( f.open( m_sd.vwd(), m_dirindex, FILE_WRITE ) )
	{
		if( f.seekSet( code * INFORECORD_WIDTH )
			&& f.write( infobuffer, INFORECORD_WIDTH - 1 ) == INFORECORD_WIDTH - 1 )
		{
			ret = true;
		}
		f.close();
	}
	EEPROM.write( code, (uint8_t)recin.position);
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
bool hybintdb::setStatus( int code, dbrecord::POSITION pos )
{
	EEPROM.write( code, (uint8_t)pos);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
void hybintdb::cleanstatuses()
{
	for(uint16_t offset = 0; offset < 1024; ++offset )
		EEPROM.update( offset, 0 );
}
