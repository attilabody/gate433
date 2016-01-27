/*
 * logwriter.h
 *
 *  Created on: Jan 26, 2016
 *      Author: abody
 */

#ifndef LOGWRITER_H_
#define LOGWRITER_H_

struct ts;

class logwriter {
public:
	enum CATEGORY { DEBUG, INFO, WARNING, ERROR };
	//virtual void log( CATEGORY category, ts &datetime, uint16_t rid, const char* message ) = 0;
};

#endif /* LOGWRITER_H_ */
