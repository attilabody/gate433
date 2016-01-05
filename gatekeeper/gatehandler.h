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
			, LiquidCrystal_I2C &lcd
			, bool enforcepos
			, bool enforcedt );
	void loop( unsigned long currmillis );

	enum STATUS : uint8_t { WAITSETTLE, CODEWAIT, PASS, RETREAT };

protected:
	enum AUTHRES : uint8_t { GRANTED = 0, UNREGISTERED, DAY, TIME, POSITION };

	AUTHRES 	authorize( uint16_t code, bool inner );
	void		updatelcd( uint16_t id, bool inner, AUTHRES decision );
	inline void	tocodewait( bool inner ) {
		m_lights.set( trafficlights::CODEWAIT, inner ); g_codeready = false; m_status = CODEWAIT;
	}
	inline void topass( bool inner ) {
		m_lights.set( trafficlights::ACCEPTED, inner );
		m_inner = inner; m_dbupdated = false; m_status = PASS;
	}

	database			&m_db;
	trafficlights		&m_lights;
	inductiveloop		&m_indloop;
	LiquidCrystal_I2C	&m_lcd;
	bool				m_enforcepos;
	bool				m_enforcedt;

//		char					m_lcdbuf[LCD_WIDTH + 1];
	STATUS					m_status;
	trafficlights::STATUS	m_tlstatus;
	inductiveloop::STATUS	m_ilstatus;
	bool					m_conflict;
	bool					m_inner;
	bool					m_dbupdated;

	uint16_t				m_previd;
	AUTHRES					m_prevdecision;
	bool					m_previnner;
};

#endif /* GATEHANDLER_H_ */
