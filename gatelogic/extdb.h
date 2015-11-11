/*
 * extdb.h
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#ifndef EXTDB_H_
#define EXTDB_H_

#include "database.h"

class extdb: public database
{
public:
	extdb();
	virtual ~extdb();

	virtual bool getParams( int code, int &inStart, int &inEnd, int &outStart, int &outEnd, uint8_t &days, POS &pos );
	virtual bool setParams( int code, int inStart, int inEnd, int outStart, int outEnd, uint8_t days, POS pos );
	virtual bool setParams( int code, POS pos );
};

#endif /* EXTDB_H_ */
