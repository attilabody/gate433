/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef THINDB_H_
#define THINDB_H_

#include <Arduino.h>
#include "database.h"
#include <SdFat.h>


class thindb: public database
{
public:
	thindb( SdFat &sd, bool initialize = false );
	virtual ~thindb();
	bool	init();
	SdFat&	getsdfat() { return m_sd; }

	bool isinitsucceeded() { return true; }

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();

private:
	SdFat		&m_sd;
	uint16_t	m_dirindex;
};

#endif /* INTDB_H_ */
