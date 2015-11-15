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
{}

extdb::~extdb()
{}

bool extdb::getresponse()
{
	uint16_t	bufidx;

	do {
		bufidx = 0;
		while( ! getlinefromserial( m_buffer, m_buflen, bufidx ) );
		if( *m_buffer == ERR )
			return false;
	} while( *m_buffer != RESP );
	return true;
}

bool extdb::getParams( int code, dbrecord &out )
{
	int16_t		tmp1, tmp2;

	m_bufptr = m_buffer + 1;
	serialoutln( CMD_GET, ' ', code );

	if( !getresponse() ) return false;

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
	out.days = tmp1 & 0x7f;
	out.position = (dbrecord::POSITION)(tmp2 & 3);
	return true;
}


bool extdb::setParams( int code, const dbrecord &in )
{
	serialout( CMD_SET, ' ', code );
	hex2serial( in.in_start, 3, " " );
	hex2serial( in.in_end, 3, " " );
	hex2serial( in.out_start, 3, " " );
	hex2serial( in.out_end, 3, " " );
	hex2serial( (uint16_t)in.days, 3, " " );
	hex2serial( (uint16_t)in.position, 3, " " );
	Serial.println();

	return getresponse();
}


bool extdb::setStatus( int code, dbrecord::POSITION pos )
{
	serialout( CMD_SET, ' ', code );
	hex2serial( (uint16_t)pos, 3, " " );
	Serial.println();
	return getresponse();
}
