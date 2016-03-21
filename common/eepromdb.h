/*
 * eepromdb.h
 *
 *  Created on: Feb 6, 2016
 *      Author: compi
 */

#ifndef COMMON_EEPROMDB_H_
#define COMMON_EEPROMDB_H_

#include <database.h>

class eepromdb: public database
{
public:
	eepromdb();
	virtual ~eepromdb();
	bool	init();

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setInfo( int code, const dbrecord& recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();

	uint8_t	read_byte( uint16_t address );
};

#endif /* COMMON_EEPROMDB_H_ */
