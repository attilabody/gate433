/*
 * fakedruino.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: abody
 */
#include "fakedruino.h"
#include <vector>


using namespace std;
vector< unsigned long >	g_times;
unsigned long	g_times_idx(0);

int				g_instatus( HIGH );

void set_time_table( unsigned long *table, unsigned long count )
{
	g_times.assign( table, table + count );
	g_times_idx = 0;
}

void set_time_table_delta( unsigned long *table, unsigned long count )
{
	g_times.clear();
	unsigned long	sum(0);

	for( unsigned long pos = 0; pos < count; ++pos ) {
		sum += table[pos];
		g_times.push_back( sum );
	}
	g_times_idx = 0;
}

unsigned long micros()
{
	if( !g_times.size() )
		return 0;

	return g_times[ g_times_idx < g_times.size() ? g_times_idx++ : g_times_idx - 1 ];
}

int digitalRead( uint8_t pin)
{
	int ret(g_instatus);
	g_instatus = !g_instatus;
	return ret;
}

void pinMode(uint8_t, uint8_t) {}
void attachInterrupt(uint8_t, void (*)(void), int mode) {}
void noInterrupts() {}
void interrupts() {}
void digitalWrite(uint8_t, uint8_t) {}
unsigned char TIMSK0;


