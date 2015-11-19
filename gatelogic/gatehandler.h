/*
 * gatehandler.h
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */

#ifndef GATEHANDLER_H_
#define GATEHANDLER_H_

#include <Arduino.h>
#include "database.h"
#include "serialbuf.h"

class gatehandler {
public:
	gatehandler( database &db, bool use_loops = false ) : m_use_loops( use_loops ), m_db( db ) {};
	virtual ~gatehandler();

	enum GATESTATUS : uint8_t {
		  closed = 0
		, opening
		, open
		, closing
	};

	void codereceived( uint16_t code, bool inside );
protected:

	database	&m_db;
	bool		canopen();
	GATESTATUS	m_state;
	bool		m_use_loops;
};

#endif /* GATEHANDLER_H_ */
