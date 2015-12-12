/*
 * common_test.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: compi
 */
#include <Arduino.h>
#include "interface.h"

bool test_datetostring()
{
	char	buffer[32];
	char	*bufptr( buffer );
	memset( buffer, 0, sizeof(buffer));

	datetostring( bufptr, 1234, 56, 78, 9, '.', '/' ); *bufptr++=0;

	bufptr = buffer;
	memset( buffer, 0, sizeof(buffer));
	timetostring( bufptr, 12, 34, 56, ':' );
	return true;
}

bool test_parsedatetime()
{	ts	t;
	const char	textdt[] = "2015.10.28-3 16:37:05";
	const char* ptr(textdt);
	return parsedatetime( t, ptr );
}
bool commontest_main()
{
	bool ret( true );
	ret &= test_datetostring();
	ret &= test_parsedatetime();

	return ret;
}
