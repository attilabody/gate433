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

class gatehandler {
public:
	gatehandler( database &db
			, unsigned long cyclelen
			, inductiveloop &loop
			, LiquidCrystal_I2C &lcd );
	char loop( unsigned long currmillis );

	enum STATUS : uint8_t { WAITSETTLE, CODEWAIT, PASS, RETREAT };

protected:
	enum AUTHRES : uint8_t { GRANTED = 0, UNREGISTERED, DAY, TIME, POSITION, AUTHRESCNT };

	AUTHRES 	authorize( uint16_t code, bool inner );
	void		updatelcd( uint16_t id, bool inner, AUTHRES decision );
	void		tocodewait( bool inner );
	inline void topass( bool inner, unsigned long curmillis ) {
		m_lights.set( trafficlights::ACCEPTED, inner );
		m_gate.set( true, GATE_OPEN_PULSE_WIDTH, 0, true, curmillis );
		m_inner = inner; m_dbupdated = false; m_status = PASS;
	}
	inline void topass_warn( bool inner, unsigned long curmillis ) {
		m_lights.set( trafficlights::WARNED, inner );
		m_gate.set( true, GATE_OPEN_PULSE_WIDTH, 0, true, curmillis );
		m_inner = inner; m_dbupdated = false; m_status = PASS;
	}

	database			&m_db;
	trafficlights		m_lights;
	outputpin			m_gate;
	inductiveloop		&m_indloop;
	LiquidCrystal_I2C	&m_lcd;

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

	static const char PROGMEM 		m_authcodes[AUTHRESCNT];
	static const uint8_t PROGMEM	m_innerlightspins[3];
	static const uint8_t PROGMEM	m_outerlightspins[3];
};

#endif /* GATEHANDLER_H_ */
