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
#ifdef VERBOSE
	buf[digits] = 0;
	Serial.println( buf );
#endif	//	VERBOSE
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
	file.close();
	return true;
}


/////////////////////////////////////////////////////////////////////////////
void sdfatlogwriter::log( CATEGORY category, ts &datetime, const char* message, uint16_t rid, uint8_t dbpos, uint8_t loop,  uint8_t decision )
{
	char		buffer[32];

	sdfwbuffer	b( m_sd.vwd(), m_dirindex, buffer, sizeof(buffer) );
	writelinehdr( b, category, datetime, rid, dbpos, loop, decision );
	b.writebuffer::write( message );
	b.writebuffer::write( '\n' );
}

/////////////////////////////////////////////////////////////////////////////
void sdfatlogwriter::log( CATEGORY category, ts &datetime, const __FlashStringHelper *message, uint16_t rid, uint8_t dbpos, uint8_t loop, uint8_t decision )
{
	char		buffer[32];

	sdfwbuffer	b( m_sd.vwd(), m_dirindex, buffer, sizeof(buffer) );
	writelinehdr( b, category, datetime, rid, dbpos, loop, decision );
	b.writebuffer::write( message );
	b.writebuffer::write( '\n' );
}

/////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::dump( Print *p )
{
	SdFile		f;
	char		buffer[32], *bptr;
	int			nio;
	char		c('\n');


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
		else {
			if( c != '\n' ) p->println();
			p->println( RESP );
		}
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
const char __catsrts[] PROGMEM = "DBG" "INF" "WRN" "ERR" "WTF";
const char __positions[] PROGMEM = "UOI";
const char __decisions[] PROGMEM = "ACK" "UNR" "DAY" "TME" "POS";

/////////////////////////////////////////////////////////////////////////////
bool sdfatlogwriter::writelinehdr( sdfwbuffer &wb, CATEGORY c, ts &datetime, uint16_t remoteid, uint8_t dbpos, uint8_t loop, uint8_t decision )
{
	bool ret( true );
	ret &= wb.write( datetime );
	ret &= wb.writebuffer::write( ' ' );
	ret &= wb.write_P( __catsrts + c * 3, 3 );
	ret &= wb.writebuffer::write( ' ' );
	if( remoteid != 0xffff ) {
		ret &= wb.write( remoteid, 4 );
		ret &= wb.writebuffer::write(' ');
	}
	if( dbpos != 0xff ) {
		ret &= wb.write_P( __positions + dbpos, 1 );
		ret &= wb.writebuffer::write(' ');
	}
	if( loop != 0xff ) {
		ret &= wb.writebuffer::write( loop ? 'I' : 'O' );
		ret &= wb.writebuffer::write(' ');
	}
	if( decision != 0xff ) {
		ret &= wb.write_P( __decisions + decision * 3, 3 );
		ret &= wb.writebuffer::write(' ');
	}
	return ret;
}
