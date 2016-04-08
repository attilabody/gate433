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
#include <PCF8574.h>
#include "config.h"
#include "globals.h"
#include "database.h"
#include "trafficlights.h"
#include "inductiveloop.h"
#include "serialbuf.h"
#include "decode433.h"
#include "display.h"

class gatehandler {
public:
	gatehandler( database &db
			, trafficlights &lights
			, inductiveloop &loop
			, display &disp
			, unsigned long cyclelen );
	void loop( unsigned long currmillis );

	enum STATUS : uint8_t { WAITSETTLE, CODEWAIT, PASS, RETREAT };

protected:
	enum AUTHRES : uint8_t { GRANTED = 0, UNREGISTERED, DAY, TIME, POSITION, AUTHRESCNT };

	AUTHRES 	authorize( uint16_t id, bool inner );
	void		tocodewait( bool inner );
	inline void topass( bool inner, unsigned long currmillis ) {
		m_lights.set( trafficlights::ACCEPTED, inner );
		m_gate.set( true, 0, 0, true, currmillis );
		m_inner = inner; m_status = PASS; m_phasestart = currmillis;
	}
	inline void topass_warn( bool inner, unsigned long currmillis ) {
		m_lights.set( trafficlights::WARNED, inner );
		m_gate.set( true, 0, 0, true, currmillis );
		m_inner = inner; m_status = PASS; m_phasestart = currmillis;
	}

	database			&m_db;
	trafficlights		&m_lights;
	inductiveloop		&m_indloop;
	display				&m_display;
	outputpin			m_gate;

//		char					m_lcdbuf[LCD_WIDTH + 1];
	STATUS					m_status;
	trafficlights::STATUS	m_tlstatus;
	inductiveloop::STATUS	m_ilstatus;
	bool					m_conflict;
	bool					m_inner;

	unsigned long			m_phasestart;
	uint16_t				m_previd;
	AUTHRES					m_prevdecision;
	bool					m_previnner;

	static const char PROGMEM 		m_authcodes[AUTHRESCNT];
};

#endif /* GATEHANDLER_H_ */
