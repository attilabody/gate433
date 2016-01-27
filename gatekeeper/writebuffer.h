/*
 * writebuffer.h
 *
 *  Created on: Jan 27, 2016
 *      Author: abody
 */
#include <Arduino.h>

#ifndef WRITEBUFFER_H_
#define WRITEBUFFER_H_

class writebuffer
{
public:
	writebuffer( void *buf, uint8_t size );
	virtual ~writebuffer();

	bool write( const void *ptr, uint8_t size );
	bool write( const char *ptr );
	bool write( char c );
	bool write_P( PGM_P ptr, uint8_t size );
	bool write_P( PGM_P ptr );
	bool write( const __FlashStringHelper *ptr, uint8_t size ) {return write_P( reinterpret_cast<PGM_P>( ptr ), size ); }
	bool write( const __FlashStringHelper *ptr ) { return write_P( reinterpret_cast<PGM_P>( ptr )); }
	virtual bool flush() = 0;
protected:
	char	*m_buffer;
	uint8_t	m_size;
	uint8_t	m_index;
};

#endif /* WRITEBUFFER_H_ */
