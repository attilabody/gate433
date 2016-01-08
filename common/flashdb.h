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

class flashdb: public database, public i2c_eeprom
{
public:
	flashdb( uint8_t eepromaddress, uint8_t pagesize, bool initialize = false );
	virtual ~flashdb();
	bool	init();

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();

private:

};

#endif /* FLASHDB_H_ */
