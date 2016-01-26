/*
 * sdfatlogwriter.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: abody
 */

#include <sdfatlogwriter.h>
#include <SdFat.h>
#include <ds3231.h>
#include "interface.h"

/////////////////////////////////////////////////////////////////////////////
sdfatlogwriter::sdfatlogwriter( SdFat &sd )
: m_sd( sd )
, m_dirindex(0)
{};

/////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::init()
{
	SdFile	file;
	if( !file.open( m_sd.vwd(), "LOG.TXT", FILE_WRITE ))
		return false;
	m_dirindex = m_sd.vwd()->curPosition()/32 -1;
	file.close();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void sdfatlogwriter::log( CATEGORY category, ts &datetime, uint16_t rid, const char* message )
{
	SdFile		f;
	char		buffer[32], *bptr( buffer );

	if( f.open( m_sd.vwd(), m_dirindex, FILE_WRITE ))
	{
		f.write( buffer, startline( buffer, category, datetime, rid ) - buffer );
		f.println( message );

		f.close();
	}
}

/////////////////////////////////////////////////////////////////////////////
void sdfatlogwriter::log( CATEGORY category, ts &datetime, uint16_t rid, const __FlashStringHelper *message )
{
	SdFile		f;
	char		buffer[32], *bptr( buffer );

	if( f.open( m_sd.vwd(), m_dirindex, FILE_WRITE ))
	{
		f.write( buffer, startline( buffer, category, datetime, rid ) - buffer );

		bptr = buffer;
		PGM_P 	p( reinterpret_cast<PGM_P>( message ));
		uint8_t	bfree( sizeof( buffer ));
		char	c;

		do
		{
			c = pgm_read_byte( p++ );
			*bptr++ = c ? c : '\n';
			--bfree;
			if( !bfree || !c) {
				f.write( buffer, sizeof( buffer ) - bfree);
				bfree = sizeof( buffer );
				bptr = buffer;
			}
		}
		while( c );

		f.close();
	}
}

/////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::dump( Print *p )
{
	SdFile		f;
	char		buffer[32], *bptr;
	int			nio;
	char		c(0);


	if( f.open( m_sd.vwd(), m_dirindex, FILE_READ ))
	{
		do
		{
			nio = f.read( buffer, sizeof( buffer ));
			if( nio == -1 ) break;
			bptr = buffer;
			for( uint8_t bc = nio; bc != 0; --bc ) {
				if( c == '\n' ) p->print( RESP );
				c = *bptr++;
				p->print( c );
			}
		} while( nio == sizeof( buffer ) );

		if( nio == -1 ) p->println( F( ERRS "ERR" ));
		else if( c == '\n' ) p->println();
		else p->println( RESP );
		f.close();
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::truncate()
{
	SdFile		f;
	if( f.open( m_sd.vwd(), m_dirindex, O_WRITE | O_CREAT | O_TRUNC )) {
		f.close();
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
char* sdfatlogwriter::startline( char *bptr, CATEGORY c, ts &datetime, uint16_t rid)
{
	datetostring( bptr, datetime.year, datetime.mon, datetime.mday, 0, 4, false, '.', '/' );
	*bptr++ = ' ';
	timetostring( bptr, datetime.hour, datetime.min, datetime.sec, ':' );
	*bptr++ = ' ';
	strcpy_P( bptr, getcatpstr( c ) );
	bptr += strlen(bptr);
	*bptr++ = ' ';
	if( rid != -1 ) {
		String	s(rid);
		strcpy( bptr, s.c_str() );
		bptr += s.length();
		*bptr++ = ' ';
	}
	return bptr;
}

/////////////////////////////////////////////////////////////////////////////
const char* sdfatlogwriter::getcatpstr( CATEGORY c )
{
	switch( c ) {
		case DEBUG:
			return PSTR("DBG");
			break;
		case INFO:
			return PSTR("INF");
			break;
		case WARNING:
			return PSTR("WRN");
			break;
		case ERROR:
			return PSTR("ERR");
			break;
		default:
			return PSTR("WTF");
			break;
	}
}
