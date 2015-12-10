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
bool commontest_main()
{
	return test_datetostring();
}
