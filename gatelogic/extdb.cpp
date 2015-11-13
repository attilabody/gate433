/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include "config.h"
#include "extdb.h"
#include "interface.h"

extdb::extdb( char *buffer, uint16_t buflen )
	: m_buffer( buffer )
	, m_buflen( buflen )
{
	// TODO Auto-generated constructor stub
}

extdb::~extdb()
{
	// TODO Auto-generated destructor stub
}

bool extdb::getParams( int code, dbrecord &out )
{
	uint16_t	bufidx;
	int16_t		tmp1, tmp2;

	m_bufptr = m_buffer + 1;
	Serial.print( CMD_GET );
	Serial.println( code );
	do
	{
		bufidx = 0;
		while( ! getlinefromserial( m_buffer, m_buflen, bufidx ) );
		if( *m_buffer == ERR )
			return false;
	}
	while( *m_buffer != RESP );
	if( (out.in_start = getintparam( m_bufptr, false, true )) == - 1
			|| (out.in_end = getintparam( m_bufptr, false, true )) ==  -1
			|| (out.out_start = getintparam( m_bufptr, false, true )) ==  -1
			|| (out.out_end = getintparam( m_bufptr, false, true )) ==  -1
			|| (tmp1 = getintparam( m_bufptr, false, true )) ==  -1
			|| (tmp2 = getintparam( m_bufptr, false, true )) ==  -1
	) {
		out.in_start = -1;
		return false;
	}
}


bool extdb::setParams( int code, const dbrecord &in )
{
	return false;
}


bool extdb::setParams( int code, dbrecord::POSITION pos )

{
	return false;
}
