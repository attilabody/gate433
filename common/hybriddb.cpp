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
hybriddb::hybriddb( SdFat &sd, uint8_t eepromaddress,  bool initialize )
: m_sd( sd )
, m_eepromaddress( eepromaddress )
{
}

//////////////////////////////////////////////////////////////////////////////
hybriddb::~hybriddb()
{
}

//////////////////////////////////////////////////////////////////////////////
bool hybriddb::init()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool hybriddb::getParams( int code, dbrecord &recout )
{
	bool ret = false;
	static char linebuffer[INFORECORD_WIDTH + 1];
	const char	*bufptr( linebuffer );
	int16_t		tmp1, tmp2;
	File		f;

	f = m_sd.open( "info.txt", FILE_READ );

	if( f.isOpen() )
	{
		if( f.seek( code * INFORECORD_WIDTH )
			&& f.read( linebuffer, INFORECORD_WIDTH ) == INFORECORD_WIDTH )
		{
			linebuffer[INFORECORD_WIDTH] = 0;						//	replace \n with ' '
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
			|| (tmp1 = getintparam( bufptr, false, true )) ==  -1 )
	{
		recout.in_start = -1;
		return false;
	}

	recout.days = tmp1 & 0x7f;
	recout.position = (dbrecord::POSITION)(i2c_eeprom_read_byte( m_eepromaddress, HYBRIDDB_EEPROM_OFFSET + code ) & 3);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
bool hybriddb::setParams( int code, const dbrecord &recin )
{
	bool ret( false );
	char infobuffer[DBRECORD_WIDTH + 1];

	recin.serialize( infobuffer );
	Serial.println( infobuffer );

	File		f;

	f = m_sd.open( "info.txt", FILE_WRITE );

	if( f.isOpen() )
	{
		if( f.seek( code * INFORECORD_WIDTH )
			&& f.write( infobuffer, INFORECORD_WIDTH - 1 ) == INFORECORD_WIDTH - 1 )
		{
			ret = true;
		}
		f.close();
	}
	i2c_eeprom_write_byte( m_eepromaddress, HYBRIDDB_EEPROM_ADDRESS + code, (uint8_t)recin.position);
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
bool hybriddb::setStatus( int code, dbrecord::POSITION pos )
{
	i2c_eeprom_write_byte( m_eepromaddress, HYBRIDDB_EEPROM_ADDRESS + code, (uint8_t)pos);

	return true;
}
