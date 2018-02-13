/*
 * writebuffer.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: abody
 */

#include <writebuffer.h>
//#include "toolbox.h"
#include <algorithm>
#include <cstring>

using namespace std;

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
	while(size)
	{
		uint8_t tocopy(min(size, (uint8_t)(m_size - m_index)));
		memcpy(m_buffer + m_index, ptr, tocopy);
		m_index += tocopy;
		if(m_index == m_size && !flush()) return false;
		ptr = reinterpret_cast<const uint8_t*>(ptr) + tocopy;
		size -= tocopy;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool writebuffer::write(const char *ptr)
{
	char *bptr(m_buffer + m_index);
	while(*ptr) {
		*bptr++ = *ptr++;
		++m_index;
		if(m_index == m_size) {
			if(!flush()) return false;
			else bptr = m_buffer;
		}
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool writebuffer::write(char c)
{
	m_buffer[ m_index++ ] = c;
	if(m_index == m_size)
		return flush();
	return true;
}
