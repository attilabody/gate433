/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef INTDB_H_
#define INTDB_H_

#include <Arduino.h>
#include "database.h"
#include <I2C.h>
#include <SdFat.h>


class intdb: public database
{
public:
	intdb( SdFat &sd, bool initialize = false );
	virtual ~intdb();
	bool	init();
	SdFat&	getsdfat() { return m_sd; }

	bool isinitsucceeded() { return m_initok; }

	virtual bool getParams( int code, dbrecord &recout );
	virtual bool setParams( int code, const dbrecord &recin );
	virtual bool setStatus( int code, dbrecord::POSITION pos );
	virtual void cleanstatuses();

private:
	SdFat	&m_sd;
	File	m_info;
	File	m_status;
	bool	m_initok;

	static const char	*m_statusname;
	static const char	*m_infoname;
};

#endif /* INTDB_H_ */
