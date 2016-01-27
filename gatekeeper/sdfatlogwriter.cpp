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
//
/////////////////////////////////////////////////////////////////////////////
sdfatlogwriter::sdfwbuffer::sdfwbuffer( FatFile* dir, uint16_t dirindex, void *buf, uint8_t size )
: writebuffer( buf, size )
{
	m_f.open( dir, dirindex, FILE_WRITE );
}

/////////////////////////////////////////////////////////////////////////////
sdfatlogwriter::sdfwbuffer::~sdfwbuffer()
{
	flush();
	m_f.close();
}

/////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::sdfwbuffer::flush()
{
	bool ret( true );
	if( m_index ) {
		int wr( m_f.write( m_buffer, m_index ));
		ret = ( wr == m_index );
		m_buffer[ m_size -1 ] = 0;
#ifdef VERBOSE
		serialout( '#', ret, ',', wr, ',', m_f.getError(), ',', m_index, ',', m_buffer );
#endif	//	VERBOSE
		m_index = 0;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::sdfwbuffer::write( ts &dt )
{
	write( dt.year , 4 );
	writebuffer::write( '.' );
	write( dt.mon, 2 );
	writebuffer::write( '.' );
	write( dt.mday, 2 );
	writebuffer::write( ' ' );
	write( dt.hour, 2 );
	writebuffer::write(':' );
	write( dt.min, 2 );
	writebuffer::write( ':' );
	write( dt.sec, 2 );
}

//////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::sdfwbuffer::write( uint16_t data, uint8_t digits )
{
	char buf[5];
	if( digits > 5 ) digits = 5;
	char *ptr( buf + digits - 1 );
	uint8_t	cntr( digits );
	while( cntr-- ) {
		*ptr-- = ( data%10 ) + '0';
		data /= 10;
	}
	buf[digits] = 0;
	Serial.println( buf );
	return writebuffer::write( buf, digits );
}

/////////////////////////////////////////////////////////////////////////////
//
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
	serialoutln( '@', m_dirindex );
	file.close();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void sdfatlogwriter::log( CATEGORY category, ts &datetime, uint16_t rid, const char* message )
{
	char		buffer[32];

	sdfwbuffer	b( m_sd.vwd(), m_dirindex, buffer, sizeof(buffer) );
	writelinehdr( b, category, datetime, rid );
	b.writebuffer::write( message );
	b.writebuffer::write( '\n' );
}

/////////////////////////////////////////////////////////////////////////////
void sdfatlogwriter::log( CATEGORY category, ts &datetime, uint16_t rid, const __FlashStringHelper *message )
{
	char		buffer[32];

	sdfwbuffer	b( m_sd.vwd(), m_dirindex, buffer, sizeof(buffer) );
	writelinehdr( b, category, datetime, rid );
	b.writebuffer::write( message );
	b.writebuffer::write( '\n' );
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
char* sdfatlogwriter::writelinehdr( sdfwbuffer &wb, CATEGORY c, ts &datetime, uint16_t remoteid )
{
	wb.write( datetime );
	wb.writebuffer::write( ' ' );
	writecatstr( wb, c );
	wb.writebuffer::write( ' ' );
	if( remoteid != -1 ) {
		wb.write( remoteid, 4 );
		wb.writebuffer::write(' ');
	}
}


/////////////////////////////////////////////////////////////////////////////
const char __catsrts[] PROGMEM = { "DBGINFWRNWRRWTF" };

/////////////////////////////////////////////////////////////////////////////
const char* sdfatlogwriter::writecatstr( sdfwbuffer &wb, CATEGORY c )
{
	wb.write_P( __catsrts + c * 3, 3 );
}
