/*
 * dummylogwriter.h
 *
 *  Created on: Feb 15, 2016
 *      Author: compi
 */

#ifndef COMMON_DUMMYLOGWRITER_H_
#define COMMON_DUMMYLOGWRITER_H_

#include <logwriter.h>

class dummylogwriter: public logwriter
{
public:
	dummylogwriter();
	~dummylogwriter();

	bool init();


	void log( 	CATEGORY category, ts &datetime, const char* message,
				uint16_t rid = 0xffff, uint8_t button = 0xff, uint8_t dbpos = 0xff,
				uint8_t loop = 0xff, uint8_t decision = 0xff );

	void log( 	CATEGORY category, ts &datetime,
				const __FlashStringHelper *message,
				uint16_t rid = 0xffff, uint8_t button = 0xff, uint8_t dbpos = 0xff,
				uint8_t loop = 0xff, uint8_t decision = 0xff );

	bool dump( Print *p );
	bool truncate();

};

#endif /* COMMON_DUMMYLOGWRITER_H_ */
