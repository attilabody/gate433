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
inline void StrRev(char *first, char *last)
{
	while(last > first) std::swap(*first++, *last--);
}

//////////////////////////////////////////////////////////////////////////////
inline char ToChar(const uint8_t in, const bool upper = true)
{
	return in + ((in < 10) ? '0' : (upper ? 'A' : 'a') - 10);
}

//////////////////////////////////////////////////////////////////////////////
char FromChar( char c, bool decimal = true );

//////////////////////////////////////////////////////////////////////////////
template< typename T> size_t ToDec(char* buffer, T data, uint8_t digits = 0, char padding = '0')
{
	bool neg = data < 0;

	char *b2 = buffer;
	uint8_t remainig = digits ? digits: -1;

	if(neg)
		data = -data;

	do {
		*b2++ = (data % 10) + '0';
		data /= 10;
	} while(data && --remainig);

	size_t ret = b2 - buffer;

	while(ret < digits) {
		*b2++ = padding;
		++ret;
	}

	if(neg)
		*b2++ = '-';

	*b2-- = 0;

	StrRev(buffer, b2);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
template< typename T> size_t ToHex(char* buffer, T data, uint8_t digits = 0, char padding = '0')
{
	char *b2 = buffer;
	uint8_t	remaining = digits ? digits : -1;

	do {
		uint8_t curval = data & 0x0f;
		*b2++ = ToChar(curval, true);
		data >>= 4;
	} while(data && --remaining);

	size_t ret = b2 - buffer;

	while(ret < digits) {
		*b2++ = padding;
		++ret;
	}

	*b2-- = 0;

	StrRev(buffer, b2);
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
int32_t GetIntParam( const char* &input, bool decimal = true, bool trimstart = true, bool acceptneg = false );

}	//	namespace sg
#endif /* INC_STRUTIL_H_ */
