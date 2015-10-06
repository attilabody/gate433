//============================================================================
// Name        : gatedemo_test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "fakedruino.h"
#include "../../gatedemo/gatedemo.ino"
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

using namespace std;

unsigned long timetable_zeros[] = {
	0, 436, 940, 444, 944, 440, 940, 440, 948, 436,944, 440, 944, 440,
	944, 440, 944, 440, 944, 440, 944, 440, 944, 436,948, 436, 16156
};

int main() {
	set_time_table_delta( timetable_zeros, ITEMCOUNT( timetable_zeros ));
	for( unsigned int cycle = 0; cycle < ITEMCOUNT( timetable_zeros) - 1 ; ++cycle ) {
		isr();
	}
	return 0;
}
