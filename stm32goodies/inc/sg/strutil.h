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

namespace sg {


//////////////////////////////////////////////////////////////////////////////
size_t uitodec(unsigned int data, char* buffer);
size_t uitohex(unsigned int data, char* buffer);

//////////////////////////////////////////////////////////////////////////////
inline void strrev(char *first, char *last)
{
	char tmp;
	while(last > first) {
		tmp = *first;
		*first++ = *last;
		*last-- = tmp;
	}
}

//////////////////////////////////////////////////////////////////////////////
inline char tochr(const uint8_t in, const bool upper = true)
{
	return in + ((in < 10) ? '0' : (upper ? 'A' : 'a') - 10);
}

}	//	namespace sg

#endif /* INC_STRUTIL_H_ */
