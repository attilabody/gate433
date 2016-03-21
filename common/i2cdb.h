/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef FLASHDB_H_
#define FLASHDB_H_

#include <Arduino.h>
#include "database.h"
#include <I2C_eeprom.h>
#include "config.h"

class i2cdb: public database, public i2c_eeprom
{
public:
	i2cdb( uint8_t i2caddress, uint8_t eeaddressbits, uint8_t eepagesize, uint16_t eeoffset = 0 );
	virtual ~i2cdb();
	bool	init();

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setInfo( int code, const dbrecord& recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();
private:
	uint16_t	m_eeoffset;
};

#endif /* FLASHDB_H_ */
