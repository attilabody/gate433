/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include "intdb.h"

#include "config.h"
#include "interface.h"

intdb::intdb()
	: m_initok( false )
{
	if( m_sd.begin( SS, SPI_HALF_SPEED )) {
		m_info = m_sd.open( "info.txt", FILE_READ );
		m_initok = m_info;
	}

}

intdb::~intdb()
{
	m_info.close();
	m_status.close();
}

bool intdb::getParams( int code, dbrecord &recout )
{
	bool ret = false;
	static char linebuffer[INFORECORD_WIDTH + STATUSRECORD_WIDTH + 1];
	const char	*bufptr( linebuffer );
	int16_t		tmp1, tmp2;

	if( (m_status = m_sd.open( "status.txt", FILE_READ )).isOpen()
		&& m_info.seek( code * INFORECORD_WIDTH )
		&& m_status.seek( code * STATUSRECORD_WIDTH )
		&& m_info.read( linebuffer, INFORECORD_WIDTH ) == INFORECORD_WIDTH
		&& m_status.read( linebuffer + INFORECORD_WIDTH, STATUSRECORD_WIDTH ) == STATUSRECORD_WIDTH )
	{
		linebuffer[INFORECORD_WIDTH] = ' ';						//	replace \n with ' '
		linebuffer[INFORECORD_WIDTH + STATUSRECORD_WIDTH] = 0;	//	clamp trailing \n
		ret = true;
	}

	if( m_status )
		m_status.close();

	if( !ret ) {
		recout.in_start = -1;
		return false;
	}

	if( (recout.in_start = getintparam( bufptr, false, true )) == - 1
			|| (recout.in_end = getintparam( bufptr, false, true )) ==  -1
			|| (recout.out_start = getintparam( bufptr, false, true )) ==  -1
			|| (recout.out_end = getintparam( bufptr, false, true )) ==  -1
			|| (tmp1 = getintparam( bufptr, false, true )) ==  -1
			|| (tmp2 = getintparam( bufptr, false, true )) ==  -1 )
	{
		recout.in_start = -1;
		return false;
	}
	recout.days = tmp1 & 0x7f;
	recout.position = (dbrecord::POSITION)(tmp2 & 3);
	return true;
}


bool intdb::setParams( int code, const dbrecord &recin )
{
	bool ret( false );
	char infobuffer[INFORECORD_WIDTH];
	char statusbuffer[STATUSRECORD_WIDTH];
	char *ibptr(infobuffer), *sbptr(statusbuffer);

	recin.serializeinfo( ibptr );
	recin.serializestatus( sbptr );

	m_info.close();
	if( !(m_info = m_sd.open( "info.txt", FILE_WRITE ))) {
		m_initok = false;
		return false;
	}
	if( (m_status = m_sd.open( "status.txt", FILE_WRITE )).isOpen()
		&& m_info.seek( code * INFORECORD_WIDTH )
		&& m_status.seek(code * STATUSRECORD_WIDTH)
		&& m_info.write( infobuffer, INFORECORD_WIDTH - 1 ) == INFORECORD_WIDTH - 1
		&& m_status.write( statusbuffer, STATUSRECORD_WIDTH -1 ) == STATUSRECORD_WIDTH -1 )
	{ ret = true; }

	if( m_status) m_status.close();

	if( m_info ) {
		m_info.close();
		if( !(m_info = m_sd.open( "info.txt", FILE_READ ))) {
			ret = m_initok = false;
		}
	}

	return ret;
}


bool intdb::setStatus( int code, dbrecord::POSITION pos )
{
	char	statusbuffer[STATUSRECORD_WIDTH];
	char	*sbptr(statusbuffer);
	bool	ret( false );

	uitohex( sbptr, (uint16_t) pos, 3 );

	if( (m_status = m_sd.open( "status.txt", FILE_WRITE ))
		&& m_status.seek(code * STATUSRECORD_WIDTH)
		&& m_status.write( statusbuffer, STATUSRECORD_WIDTH -1 ) == STATUSRECORD_WIDTH -1 ) {
		ret = true;
	}
	if( m_status ) m_status.close();
	return ret;
}
