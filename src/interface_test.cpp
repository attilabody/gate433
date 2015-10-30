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

using namespace std;
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

const char 		*g_commands[] = {
	  "CODE"
	, "GET"
	, "SET"
	, "SETF"
	, "LOG"
	, ""
};

const char cmdline[] = "CODE 1";

int main()
{
	const char*	bufptr( cmdline );
	int cmd = findcommand( bufptr, g_commands );
	int code = getintparam( bufptr, true );

	int dummy(0);
	++dummy;
}

