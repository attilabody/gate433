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
	enum AUTHRES : uint8_t { GRANTED = 0, WARN, UNREGISTERED, DAY, TIME, POSITION, AUTHRESCNT };

	AUTHRES 	authorize( uint16_t id, bool inner );
	void		tocodewait( bool inner );
	bool 		dbup(bool inner, uint16_t id);

	database			&m_db;
	trafficlights		&m_lights;
	inductiveloop		&m_indloop;
	display				&m_display;
	outputpin			m_gate;

	STATUS					m_status;
	trafficlights::STATUS	m_tlstatus;
	inductiveloop::STATUS	m_ilstatus;
	bool					m_conflict;
	bool					m_inner;

	unsigned long			m_phasestart;
	uint16_t				m_previd;
	AUTHRES					m_prevdecision;
	bool					m_previnner;
	uint16_t				m_code;
	uint8_t					m_codecount;
	uint16_t				m_id;

	static const char PROGMEM 		m_authcodes[AUTHRESCNT];
};

#endif /* GATEHANDLER_H_ */
