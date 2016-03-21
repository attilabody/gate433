/*
 * flash.cpp
 *
 *  Created on: Jan 8, 2016
 *      Author: abody
 */

#include "i2cdb.h"
#include "config.h"
#include "toolbox.h"
#include "I2C_eeprom.h"

//////////////////////////////////////////////////////////////////////////////
i2cdb::i2cdb( uint8_t i2caddress, uint8_t eeaddressbits,
		uint8_t eepagesize, uint16_t eeoffset )
: i2c_eeprom( i2caddress, eeaddressbits, eepagesize )
, m_eeoffset( eeoffset )
{
}

//////////////////////////////////////////////////////////////////////////////
i2cdb::~i2cdb()
{
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::init()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::getParams( int code, dbrecord &recout )
{
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	read_page( m_eeoffset + code * PACKEDDBRECORD_WIDTH, buffer, sizeof(buffer) );
	recout.unpack( buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setParams( int code, const dbrecord &recin )
{
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	recin.pack( buffer );
	write_page( m_eeoffset + code * PACKEDDBRECORD_WIDTH, buffer, sizeof(buffer) );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setStatus( int code, dbrecord::POSITION pos )
{
	write_byte( m_eeoffset + (code + 1) * PACKEDDBRECORD_WIDTH - 1, (uint8_t)pos);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setInfo( int code, const dbrecord& recin )
{
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	recin.pack( buffer );
	write_page( m_eeoffset + code * PACKEDDBRECORD_WIDTH, buffer, sizeof(buffer)-1 );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
void i2cdb::cleanstatuses()
{
	for( uint16_t rec = 0; rec < 1024; ++rec )
		setStatus( rec, (dbrecord::POSITION) 0 );
}
