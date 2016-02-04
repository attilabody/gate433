/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef HYBINTDB_H_
#define HYBINTDB_H_

#include <Arduino.h>
#include "database.h"
#include <Wire.h>
#include <SdFat.h>
#include <EEPROM.h>
#include "config.h"

class hybintdb: public database
{
public:
	hybintdb( SdFat &sd, bool initialize = false );
	virtual ~hybintdb();
	bool	init();
	SdFat&	getsdfat() { return m_sd; }

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();

private:
	SdFat		&m_sd;
	uint16_t	m_dirindex;
};

#endif /* HYBINTDB_H_ */
