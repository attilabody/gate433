/*
 * sdfatlogwriter.h
 *
 *  Created on: Jan 26, 2016
 *      Author: abody
 */

#ifndef SDFATLOGWRITER_H_
#define SDFATLOGWRITER_H_

#include <Arduino.h>
#include <logwriter.h>

class SdFat;

class sdfatlogwriter: public logwriter {
public:
	sdfatlogwriter( SdFat &sd );
	bool init();

	virtual void log( CATEGORY category, ts &datetime, uint16_t rid, const char* message );
	virtual void log( CATEGORY category, ts &datetime, uint16_t rid, const __FlashStringHelper *message );

	bool dump( Print *p );
	bool truncate();

protected:
	SdFat		&m_sd;
	uint16_t	m_dirindex;

	const char * 	getcatpstr( CATEGORY c );
	char *			startline( char *bptr, CATEGORY c, ts &datetime, uint16_t rid );
};

#endif /* SDFATLOGWRITER_H_ */
