//============================================================================
// Name        : interface_test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string.h>
#include <Arduino.h>
#include <Serial.h>
#include "interface.cpp"
#include "interface_main.cpp"

using namespace std;
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

const char 	cmdline[] = "GET 1";
const char	response[] = ":000 59F 000 59F 000007F";
int main()
{
	const char*	bufptr( cmdline );
	int cmd = findcommand( bufptr, g_commands );
	int code = getintparam( bufptr, true );

	dbrecord	rec( response + 1 );
	int dummy(0);
	++dummy;
}

