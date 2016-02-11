/*
 * dthelpers.h
 *
 *  Created on: Feb 8, 2016
 *      Author: abody
 */

#ifndef COMMON_DTHELPERS_H_
#define COMMON_DTHELPERS_H_

#include <Arduino.h>


struct ts;

void datetostring( char* &buffer, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t yeardigits = 2, bool showdow = false, char datesep = '.', char dowsep = '/' );
void timetostring( char* &buffer, uint8_t hour, uint8_t min, uint8_t sec, char sep );
void printdate( Print *p, uint16_t year, uint8_t month, uint8_t day, uint8_t dow, uint8_t yeardigits = 2, bool showdow = false, char datesep = '.', char dowsep = '/' );
void printtime( Print *p, uint8_t hour, uint8_t min, uint8_t sec, char sep );
bool parsedatetime( ts &t, const char *&inptr );

#endif /* COMMON_DTHELPERS_H_ */

