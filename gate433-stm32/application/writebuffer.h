/*
 * writebuffer.h
 *
 *  Created on: Jan 27, 2016
 *      Author: abody
 */
#ifndef WRITEBUFFER_H_
#define WRITEBUFFER_H_
#include <inttypes.h>

class writebuffer
{
public:
	writebuffer( void *buf, uint8_t size );
	virtual ~writebuffer();

	bool write( const void *ptr, uint8_t size );
	bool write( const char *ptr );
	bool write( char c );
	virtual bool flush() = 0;
protected:
	char	*m_buffer;
	uint8_t	m_size;
	uint8_t	m_index;
};

#endif /* WRITEBUFFER_H_ */
