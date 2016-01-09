/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include "hybriddb.h"
#include "config.h"
#include "interface.h"
#include "I2C_eeprom.h"

//////////////////////////////////////////////////////////////////////////////
hybriddb::hybriddb( SdFat &sd, uint8_t i2caddress, uint8_t eeaddressbits, uint8_t eepagesize, bool initialize )
: i2c_eeprom( i2caddress, eeaddressbits, eepagesize )
, m_sd( sd )
{
	if( initialize ) init();
}

//////////////////////////////////////////////////////////////////////////////
hybriddb::~hybriddb()
{
}

//////////////////////////////////////////////////////////////////////////////
bool hybriddb::init()
{
	SdFile	file;
	if( !file.open( m_sd.vwd(), "info.txt", FILE_READ ))
		return false;
	m_dirindex = m_sd.vwd()->curPosition()/32 -1;
	file.close();
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool hybriddb::getParams( int code, dbrecord &recout )
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
	recout.position = (dbrecord::POSITION)( read_byte( HYBRIDDB_EEPROM_OFFSET + code ) & 3);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool hybriddb::setParams( int code, const dbrecord &recin )
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
	write_byte( HYBRIDDB_EEPROM_OFFSET + code, (uint8_t)recin.position);
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
bool hybriddb::setStatus( int code, dbrecord::POSITION pos )
{
	write_byte( HYBRIDDB_EEPROM_OFFSET + code, (uint8_t)pos);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
void hybriddb::cleanstatuses()
{
	fill_page( HYBRIDDB_EEPROM_OFFSET , 0, 1024 );
}
