/*
 * flash.cpp
 *
 *  Created on: Jan 8, 2016
 *      Author: abody
 */

#include "flashdb.h"
#include "config.h"
#include "interface.h"
#include "I2C_eeprom.h"

//////////////////////////////////////////////////////////////////////////////
flashdb::flashdb( uint8_t eepromaddress, uint8_t pagesize, bool initialize )
: i2c_eeprom( eepromaddress, pagesize )
{
	if( initialize ) init();
}

//////////////////////////////////////////////////////////////////////////////
flashdb::~flashdb()
{
}

//////////////////////////////////////////////////////////////////////////////
bool flashdb::init()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool flashdb::getParams( int code, dbrecord &recout )
{
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	read_page( FLASHDB_EEPROM_OFFSET + code * PACKEDDBRECORD_WIDTH, buffer, sizeof(buffer) );
	recout.unpack( buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool flashdb::setParams( int code, const dbrecord &recin )
{
	uint8_t	buffer[ PACKEDDBRECORD_WIDTH ];
	recin.pack( buffer );
	write_page( FLASHDB_EEPROM_OFFSET + code * PACKEDDBRECORD_WIDTH, buffer, sizeof(buffer) );
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool flashdb::setStatus( int code, dbrecord::POSITION pos )
{
	write_byte( FLASHDB_EEPROM_OFFSET + (code + 1) * PACKEDDBRECORD_WIDTH - 1, (uint8_t)pos);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
void flashdb::cleanstatuses()
{
	for( uint16_t rec = 0; rec < 1024; ++rec )
		setStatus( rec, (dbrecord::POSITION) 0 );
}
