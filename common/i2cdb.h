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
#include <Wire.h>
#include <I2C_eeprom.h>
#include "config.h"

class i2cdb: public database, public i2c_eeprom
{
public:
	i2cdb( uint8_t i2caddress, uint8_t eeaddressbits, uint8_t eepagesize );
	virtual ~i2cdb();
	bool	init();

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();
};

#endif /* FLASHDB_H_ */
