/*
 * gatehandler.h
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */

#ifndef GATEHANDLER_H_
#define GATEHANDLER_H_

#include <Arduino.h>
#include "config.h"
#include "database.h"
#include "trafficlights.h"
#include "inductiveloop.h"
#include "serialbuf.h"

class gatehandler {
public:
	gatehandler( database &db
			, trafficlights &lights
			, inductiveloop &loop
			, bool enforcepos
			, bool enforcedt );

	enum STATUS : uint8_t {
		  CLOSED = 0
		, OPENING
		, PASSTHRU
	};

protected:

	database		&m_db;
	trafficlights	&m_lights;
	inductiveloop	&m_loop;
	bool			m_enforcepos;
	bool			m_enforcedt;
	STATUS			m_status;
};

#endif /* GATEHANDLER_H_ */
