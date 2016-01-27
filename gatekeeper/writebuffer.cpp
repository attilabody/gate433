/*
 * writebuffer.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: abody
 */

#include <writebuffer.h>
#include "interface.h"

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////
writebuffer::writebuffer( void *buf, uint8_t size )
: m_buffer( reinterpret_cast<char*>(buf) )
, m_size( size )
, m_index( 0 )
{
}

/////////////////////////////////////////////////////////////////////////////
writebuffer::~writebuffer()
{
}

/////////////////////////////////////////////////////////////////////////////
bool writebuffer::write( const void *ptr, uint8_t size )
{
	Serial.print("% ");
	Serial.write( reinterpret_cast<const char *>(ptr), size );
	serialoutsepln( ", ", ' ', size, m_size, m_index );

	while( size )
	{
		uint8_t tocopy( min( size, m_size - m_index ));
		memcpy( m_buffer + m_index, ptr, tocopy );
		m_index += tocopy;
		if( m_index == m_size && !flush() ) return false;
		ptr = reinterpret_cast<const uint8_t*>(ptr) + tocopy;
		size -= tocopy;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool writebuffer::write( const char *ptr )
{
	char *bptr( m_buffer + m_index );
	while( *ptr ) {
		*bptr++ = *ptr++;
		++m_index;
		if( m_index == m_size ) {
			if( !flush() ) return false;
			else bptr = m_buffer;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool writebuffer::write( char c )
{
	m_buffer[ m_index++ ] = c;
	if( m_index == m_size && !flush() ) return false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool writebuffer::write_P( PGM_P ptr, uint8_t size )
{
	while( size )
	{
		uint8_t tocopy( min( size, m_size - m_index ));
		memcpy_P( m_buffer + m_index, ptr, tocopy );
		m_index += tocopy;
		ptr += tocopy;
		size -= tocopy;
		if( m_index == m_size && !flush() ) return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool writebuffer::write_P( PGM_P pptr )
{
	char *bptr( m_buffer + m_index );
	while( true ) {
		char c( pgm_read_byte( pptr++ ) );
		if( !c ) return true;
		*bptr++ = c;
		++m_index;
		if( m_index == m_size ) {
			if( !flush() ) return false;
			else bptr = m_buffer;
		}
	}
}

