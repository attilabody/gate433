/*
 * strutil.cpp
 *
 *  Created on: Nov 28, 2016
 *      Author: compi
 */
#include "sg/strutil.h"
#include <stdint.h>

using namespace sg;

//////////////////////////////////////////////////////////////////////////////
size_t sg::uitodec( unsigned int data, char* buffer)
{
	char *b2 = buffer;
	if(!data) {
		*b2++ = '0';
		*b2 = '\0';
		return 1;
	}

	while(data) {
		*b2++ = (data % 10) + '0';
		data /= 10;
	}
	size_t ret = b2 - buffer;

	*b2-- = 0;

	strrev(buffer, b2);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
size_t sg::uitohex(unsigned int data, char* buffer)
{
	char *b2 = buffer;

	if(!data) {
		*b2++ = '0';
		*b2 = '\0';
		return 1;
	}

	while(data) {
		uint8_t curval = data & 0x0f;
		*b2++ = tochr(curval, true);
		data >>= 4;
	}
	size_t ret = b2 - buffer;

	*b2-- = 0;

	strrev(buffer, b2);
    return ret;
}




