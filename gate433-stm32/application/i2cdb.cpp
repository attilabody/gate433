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
	HAL_StatusTypeDef	res = HAL_OK;

	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	if((res = m_eeprom.Read(buffer, m_offset + code * PACKEDDBRECORD_WIDTH, sizeof(buffer))) == HAL_OK) {
		if(m_eeprom.Sync() == HAL_I2C_ERROR_NONE) {
			recout.unpack( buffer );
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setParams( int code, const dbrecord &recin )
{
	HAL_StatusTypeDef	ret;
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	recin.pack( buffer );
	ret = m_eeprom.Write(buffer, m_offset + code * PACKEDDBRECORD_WIDTH, sizeof(buffer));
	return ret == HAL_OK && m_eeprom.Sync() == HAL_I2C_ERROR_NONE;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setStatus( int code, dbrecord::POSITION pos )
{
	HAL_StatusTypeDef	ret;
	ret = m_eeprom.Write(&pos, m_offset + (code + 1) * PACKEDDBRECORD_WIDTH - 1, 1);
	return ret == HAL_OK && m_eeprom.Sync() == HAL_I2C_ERROR_NONE;
}

//////////////////////////////////////////////////////////////////////////////
bool i2cdb::setInfo( int code, const dbrecord& recin )
{
	HAL_StatusTypeDef	ret;
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	recin.pack( buffer );
	ret = m_eeprom.Write(buffer, m_offset + code * PACKEDDBRECORD_WIDTH, sizeof(buffer)-1 );
	return ret == HAL_OK && m_eeprom.Sync() == HAL_I2C_ERROR_NONE;
}

//////////////////////////////////////////////////////////////////////////////
void i2cdb::cleanstatuses()
{
	for( uint16_t rec = 0; rec < 1024; ++rec )
		setStatus( rec, (dbrecord::POSITION) 0 );
}
