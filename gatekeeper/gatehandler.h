/*
 * gatehandler.h
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */

#ifndef GATEHANDLER_H_
#define GATEHANDLER_H_

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "database.h"
#include "trafficlights.h"
#include "inductiveloop.h"
#include "serialbuf.h"
#include "decode433.h"

class gatehandler {
public:
	gatehandler( database &db
			, trafficlights &lights
			, inductiveloop &loop
			, LiquidCrystal_I2C lcd
			, bool enforcepos
			, bool enforcedt );
	void loop( unsigned long currmillis );

	enum STATUS : uint8_t { WAITSETTLE, CODEWAIT, PASS, RETREAT };

protected:
	bool		authorize( uint16_t code, bool inner );
	inline void	startcodewait( bool inner ) {
		m_lights.set( trafficlights::CODEWAIT, inner ); g_codeready = false; m_status = CODEWAIT;
	}

	database			&m_db;
	trafficlights		&m_lights;
	inductiveloop		&m_indloop;
	LiquidCrystal_I2C	&m_lcd;
	bool				m_enforcepos;
	bool				m_enforcedt;

	STATUS					m_status;
	trafficlights::STATUS	m_tlstatus;
	inductiveloop::STATUS	m_ilstatus;
	bool					m_conflict;
	bool					m_inner;
};

#endif /* GATEHANDLER_H_ */
