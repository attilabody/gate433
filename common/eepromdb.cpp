/*
 * eepromdb.cpp
 *
 *  Created on: Feb 6, 2016
 *      Author: compi
 */

#include <EEPROM.h>
#include "eepromdb.h"

#define POS_MASK	3	//0: unknown, 1:outside, 2: inside 3:disabled
#define POS_SHIFT	0
#define TYPE_MASK	4	//0: owner, 1:tenant
#define TYPE_SHIFT	2

#define TENANT_IN_START		(7*60+30)
#define TENANT_OUT_START	(7*60+30)
#define	TENANT_IN_END		(17*60+30)
#define	TENANT_OUT_END		(18*60+5)
#define TENANT_DAYS			0x3f

//////////////////////////////////////////////////////////////////////////////
eepromdb::eepromdb()
{
}

//////////////////////////////////////////////////////////////////////////////
eepromdb::~eepromdb()
{
}

//////////////////////////////////////////////////////////////////////////////
bool eepromdb::init()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool eepromdb::getParams( int code, dbrecord &recout )
{
	uint8_t rawdata( EEPROM.read( code ));
	bool owner( (rawdata & TYPE_MASK) == 0 );
	if((rawdata & POS_MASK) == 3 ) {
		recout.settimes( 0,0,0,0 );
		recout.days = 0;
		recout.position = dbrecord::UNKNOWN;
		return true;
	} else if( (rawdata & TYPE_MASK) != 0 ) {
		recout.settimes( TENANT_IN_START, TENANT_IN_END, TENANT_OUT_START, TENANT_OUT_END );
		recout.days = TENANT_DAYS;
		recout.position = (dbrecord::POSITION) (rawdata & 3);
	} else {
		recout.settimes( 0, 0xfff, 0, 0xfff );
		recout.days = 0x7f;
		recout.position = (dbrecord::POSITION) (rawdata & 3);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool eepromdb::setParams( int code, const dbrecord &recin )
{
	uint8_t rawdata(0);
	if( recin.enabled() )
	{
		if( recin.in_start > 0 || recin.in_end < 24 * 60
				|| recin.out_start > 0 || recin.out_end < 24 * 60
				|| (recin.days & 0x7f) != 0x7f )
		{
			rawdata |= 1 << TYPE_SHIFT;
		}
		rawdata |= ((uint8_t) recin.position) << POS_SHIFT;
	} else {
		rawdata = 3 << POS_SHIFT;
	}
	EEPROM.update( code, rawdata);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool eepromdb::setInfo( int code, const dbrecord& recin )
{
	uint8_t pos( EEPROM.read( code ) & POS_MASK), rawdata(0);

	if( recin.enabled() )
	{
		if( recin.in_start > 0 || recin.in_end < 24 * 60
				|| recin.out_start > 0 || recin.out_end < 24 * 60
				|| (recin.days & 0x7f) != 0x7f )
		{
			rawdata |= 1 << TYPE_SHIFT;
		}
		rawdata |= pos << POS_SHIFT;
	} else {
		rawdata = 3 << POS_SHIFT;
	}
	EEPROM.update( code, rawdata);
	return true;

}

//////////////////////////////////////////////////////////////////////////////
bool eepromdb::setStatus( int code, dbrecord::POSITION pos )
{
	uint8_t rawdata( EEPROM.read( code )), shadow( rawdata );
	rawdata &= ~POS_MASK;
	rawdata |= ((uint8_t) pos) << POS_SHIFT;
	if( rawdata != shadow )
		EEPROM.write( code, rawdata);
	return true;
}

//////////////////////////////////////////////////////////////////////////////
void eepromdb::cleanstatuses()
{
	for( uint16_t rec = 0; rec < 1024; ++rec )
		setStatus( rec, (dbrecord::POSITION) 0 );
}

//////////////////////////////////////////////////////////////////////////////
uint8_t eepromdb::read_byte( uint16_t address )
{
	return EEPROM.read( address );
}
