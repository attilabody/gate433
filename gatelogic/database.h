/*
 * database.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef DATABASE_H_
#define DATABASE_H_
#include <Arduino.h>
#include "interface.h"

class database
{
public:
	virtual bool getParams( int code, dbrecord &out ) = 0;
	virtual bool setParams( int code, const dbrecord &in ) = 0;
	virtual bool setParams( int code, dbrecord::POSITION pos ) = 0;
};

#endif /* DATABASE_H_ */
