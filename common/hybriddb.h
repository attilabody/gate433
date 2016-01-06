/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef HYBRIDDB_H_
#define HYBRIDDB_H_

#include <Arduino.h>
#include "database.h"
#include <Wire.h>
#include <SdFat.h>
#include <I2C_eeprom.h>
#include "config.h"

class hybriddb: public database
{
public:
	hybriddb( SdFat &sd, uint8_t eepromaddress, bool initialize = false );
	virtual ~hybriddb();
	bool	init();
	SdFat&	getsdfat() { return m_sd; }

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();

private:
	SdFat		&m_sd;
	uint8_t		m_eepromaddress;
	uint16_t	m_dirindex;
};

#endif /* INTDB_H_ */
