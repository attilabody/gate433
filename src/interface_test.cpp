//============================================================================
// Name        : interface_test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string.h>
using namespace std;
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

void	printCode( int code );
void	processInput();
long	getintparam(unsigned char &sbindex, bool decimal = true );

char 			g_serbuf[64];
unsigned char	g_serptr(0);
const char 		*g_commands[] = {
	  "GET"
	, "SET"
	, "SETFLAGS"
	, "SHOW"
};

const char cmdline[] = "SHOW 1\n";

int main()
{
	unsigned char pos = 0;
	unsigned char len = (unsigned char)strlen(cmdline);

	while (pos<len)
	{
		char inc = cmdline[pos++];
		g_serbuf[g_serptr++] = ( inc == '\n' ? 0 : inc );
		if (inc == '\n' || g_serptr >= sizeof( g_serbuf ) -1 ) {
			g_serbuf[g_serptr] = 0;
			processInput();
			g_serptr = 0;
		}
	}


	strcpy( g_serbuf, "SHOW 1\n");
	g_serptr = strlen( g_serbuf );
	processInput();
	return 0;
}

inline char convertdigit( char c, bool decimal = true )
{
	if( decimal )
		return (c >= '0' && c <= '9') ? c - '0' : -1;
	else {
		if( c >= '0' && c <= '9' ) return c - '0';
		if( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
		if( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
		return -1;
	}
}

long getintparam(unsigned char &sbidx, bool decimal)
{
	long	retval(0);
	char	converted;
	bool	found(false);

	while( sbidx < g_serptr ) {
		if(( converted = convertdigit( g_serbuf[sbidx++])) == -1) break;
		retval *=  decimal ? 10 : 16;
		retval += converted;
		found = true;
	}
	while (sbidx < g_serptr
			&& (g_serbuf[sbidx] == ' ' || g_serbuf[sbidx] == '\n'
					|| g_serbuf[sbidx] == ','))
		++sbidx;

	return found ? retval : -1;
}

void printCode( int code )
{
	int dummy(code);
	++dummy;
}

char findcommand(unsigned char &inptr)
{
	while (inptr < g_serptr && g_serbuf[inptr] != ' ' && g_serbuf[inptr] != ','
			&& g_serbuf[inptr] != '\n')
		++inptr;

	if (inptr == g_serptr) return -1;

	for (char i = 0; i < ITEMCOUNT(g_commands); ++i)
	{
		if (!strncmp(g_serbuf, g_commands[i], inptr))
		{
			++inptr;
			while (inptr < g_serptr
					&& (g_serbuf[inptr] == ' ' || g_serbuf[inptr] == '\n')
					|| g_serbuf[inptr] == ',')
				++inptr;
			return i;
		}
	}
	return -1;
}

void processInput()
{
	static char linebuffer[25];

	g_serbuf[ g_serptr ] = 0;

	unsigned char inptr(0);
	int param(0);

	char command = findcommand(inptr);

	switch (command) {
	case 0:		//	GET <CODE>
		{
			int code( getintparam(inptr));
			if( code == -1 ) break;
		}
		break;

	case 1:		//	SET <CODE> 000 59F 000 59F 0000000
		break;

	case 2:		//	SETFLAGS <CODE> 0000000
		break;

	case 3:		//	SHOW <CODE>3
		int code( getintparam(inptr));
		printCode( code );
		break;
	}
}
