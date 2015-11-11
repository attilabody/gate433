/*
 * gatehandler.h
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */

#ifndef GATEHANDLER_H_
#define GATEHANDLER_H_

#include <Arduino.h>

class gatehandler {
public:
	gatehandler( bool use_loops = false ) : m_use_loops( use_loops ) {};
	virtual ~gatehandler();

	enum GATESTATUS : uint8_t {
		  closed = 0
		, opening
		, open
		, closing
	};

	void rfevent( uint16_t code, const char *dbresponse, bool innerloop, bool outerloop );
protected:

	bool		canopen();
	GATESTATUS	m_state;
	bool		m_use_loops;
};

#endif /* GATEHANDLER_H_ */
