/*
 * tostr.h
 *
 *  Created on: Nov 28, 2016
 *      Author: compi
 */

#ifndef INC_STRUTIL_H_
#define INC_STRUTIL_H_

#include <stddef.h>
#include <inttypes.h>
#include <algorithm>

namespace sg {

//////////////////////////////////////////////////////////////////////////////
inline void strrev(char *first, char *last)
{
	while(last > first) std::swap(*first++, *last--);
}

//////////////////////////////////////////////////////////////////////////////
inline char tochr(const uint8_t in, const bool upper = true)
{
	return in + ((in < 10) ? '0' : (upper ? 'A' : 'a') - 10);
}

//////////////////////////////////////////////////////////////////////////////
char fromchr( char c, bool decimal = true );

//////////////////////////////////////////////////////////////////////////////
template< typename T> size_t todec(char* buffer, T data, uint8_t minDigits = 0, char pading = '0')
{
	char *b2 = buffer;
	do {
		*b2++ = (data % 10) + '0';
		data /= 10;
	} while(data);

	size_t ret = b2 - buffer;

	while(ret < minDigits) {
		*b2++ = pading;
		++ret;
	}

	*b2-- = 0;

	strrev(buffer, b2);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
template< typename T> size_t tohex(char* buffer, T data, uint8_t minDigits = 0, char pading = '0')
{
	char *b2 = buffer;

	do {
		uint8_t curval = data & 0x0f;
		*b2++ = tochr(curval, true);
		data >>= 4;
	} while(data);

	size_t ret = b2 - buffer;

	while(ret < minDigits) {
		*b2++ = pading;
		++ret;
	}

	*b2-- = 0;

	strrev(buffer, b2);
    return ret;
}

int32_t getintparam( const char* &input, bool decimal = true, bool trimstart = true, bool acceptneg = false );

}	//	namespace sg
#endif /* INC_STRUTIL_H_ */
