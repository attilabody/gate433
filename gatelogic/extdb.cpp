/*
 * extdb.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: abody
 */

#include "config.h"
#include "extdb.h"

extdb::extdb()
{
	// TODO Auto-generated constructor stub

}

extdb::~extdb()
{
	// TODO Auto-generated destructor stub
}

bool extdb::getParams( int code, int &inStart, int &inEnd, int &outStart, int &outEnd, uint8_t &days, POS &pos )
{
	return false;
}


bool extdb::setParams( int code, int inStart, int inEnd, int outStart, int outEnd, uint8_t days, POS pos )
{
	return false;
}


bool extdb::setParams( int code, POS pos )
{
	return false;
}
