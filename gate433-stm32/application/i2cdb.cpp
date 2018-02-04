/*
 * flash.cpp
 *
 *  Created on: Jan 8, 2016
 *      Author: abody
 */

#include "i2cdb.h"

//////////////////////////////////////////////////////////////////////////////
i2cdb::i2cdb(sg::I2cEEPROM &eeprom, uint16_t offset)
: m_eeprom(eeprom)
, m_offset(offset)
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
	m_eeprom.Read(buffer, m_offset + code * PACKEDDBRECORD_WIDTH, sizeof(buffer));
	recout.unpack( buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setParams( int code, const dbrecord &recin )
{
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	recin.pack( buffer );
	m_eeprom.Write(buffer, m_offset + code * PACKEDDBRECORD_WIDTH, sizeof(buffer));
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setStatus( int code, dbrecord::POSITION pos )
{
	m_eeprom.Write(&pos, m_offset + (code + 1) * PACKEDDBRECORD_WIDTH - 1, 1);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setInfo( int code, const dbrecord& recin )
{
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	recin.pack( buffer );
	m_eeprom.Write(buffer, m_offset + code * PACKEDDBRECORD_WIDTH, sizeof(buffer)-1 );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
void i2cdb::cleanstatuses()
{
	for( uint16_t rec = 0; rec < 1024; ++rec )
		setStatus( rec, (dbrecord::POSITION) 0 );
}
