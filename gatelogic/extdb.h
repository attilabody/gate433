/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef EXTDB_H_
#define EXTDB_H_

#include <Arduino.h>
#include "database.h"

class extdb: public database
{
public:
	extdb( char *buffer, uint16_t buflen );
	virtual ~extdb();

	virtual bool getParams( int code, dbrecord &out );
	virtual bool setParams( int code, const dbrecord &in );
	virtual bool setParams( int code, dbrecord::POSITION pos );

private:
	char		*m_buffer;
	const char	*m_bufptr;
	uint16_t	m_buflen;
};

#endif /* EXTDB_H_ */
