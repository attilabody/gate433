//============================================================================
// Name        : gatedemo_test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <Arduino.h>

#include "../../common/interface.cpp"
#define ITEMCOUNT(A) (sizeof(A)/sizeof(A[0]))

using namespace std;

extern volatile unsigned int	g_code;
extern volatile bool			g_codeready;

unsigned long timetable_combined[] = {
	0,
	120, 1300, 14000, 5,
	436, 948, 436, 16156,
	436, 940, 444, 944, 440, 940, 440, 948, 436, 944, 440, 944, 440,
	944, 440, 944, 440, 944, 440, 944, 440, 944, 436, 948, 436, 16156,
	432, 492, 892, 492, 892, 492, 896, 488, 892, 492, 896, 492, 892,
	492, 892, 492, 892, 492, 892, 492, 892, 956, 428, 956, 428, 16168,
	436, 948, 436, 948, 436, 952, 432, 948, 436, 948, 436, 488, 896,
	488, 900, 484, 896, 492, 896, 488, 896, 948, 436, 952, 432, 16164,
};

unsigned long timetable_zeros[] = {
	0,
//	120,1300,14000,5,
//	436,948,436,16156,
	436,940,444,944,440,940,440,948,436,944,440,944,440,
	944,440,944,440,944,440,944,440,944,436,948,436,16156
};

unsigned long timetable_ones[] = {
	0,432,492,892,492,892,492,896,488,892,492,896,492,892,
	492,892,492,892,492,892,492,892,956,428,956,428,16168
};

unsigned long timetable_mixed[] = {
	0,436,948,436,948,436,952,432,948,436,948,436,488,896,
	488,900,484,896,492,896,488,896,948,436,952,432,16164
};

unsigned long timetable_extreme[] = { 0
		, 268, 464, 624, 828, 260, 460, 628, 812, 276, 820, 264, 456, 632, 820, 268
		, 820, 268, 456, 632, 816, 272, 824, 264, 824, 264, 12792
};

#define TIMETABLE timetable_extreme

int main() {
	cout << hex;
	set_time_table_delta( TIMETABLE, ITEMCOUNT( TIMETABLE ));
	for( unsigned int cycle = 0; cycle < ITEMCOUNT( TIMETABLE ) - 1 ; ++cycle ) {
		isr();
		if( g_codeready ) {
			cout << g_code << endl;
			g_codeready = false;
		}
	}

	return 0;
}
