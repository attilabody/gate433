/*
 * database.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef DATABASE_H_
#define DATABASE_H_
#include <Arduino.h>

class database
{
public:
	enum POS { UNKNOWN = 0, OUT, IN };
	virtual bool getParams( int code, int &inStart, int &inEnd, int &outStart, int &outEnd, uint8_t &days, POS &pos ) = 0;
	virtual bool setParams( int code, int inStart, int inEnd, int outStart, int outEnd, uint8_t days, POS pos ) = 0;
	virtual bool setParams( int code, POS pos ) = 0;
};

#endif /* DATABASE_H_ */
