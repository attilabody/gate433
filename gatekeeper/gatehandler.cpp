/*
 * gatehandler.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */

#include "config.h"
#include "gatehandler.h"
#include "decode433.h"

gatehandler::gatehandler( database &db
			, trafficlights &lights
			, inductiveloop &loop
			, LiquidCrystal_I2C lcd
			, bool enforcepos
			, bool enforcedt )
: m_db( db )
, m_lights( lights )
, m_indloop( loop )
, m_lcd( lcd )
, m_enforcepos( enforcepos )
, m_enforcedt( enforcedt )
, m_status( WAITSETTLE )
, m_tlstatus( trafficlights::OFF )
, m_ilstatus( inductiveloop::NONE )
, m_conflict( false )
, m_inner( false )
//, m_innersaved( false )
{
}

void gatehandler::loop( unsigned long currmillis )
{
	static trafficlights::STATUS	tlstatus(trafficlights::OFF );
	static bool						inner;

	inductiveloop::STATUS			ilstatus;
	bool							conflict, ilchanged;

	conflict = m_indloop.update( ilstatus );
	ilchanged = (ilstatus != m_ilstatus) || (conflict != m_conflict );

	switch( m_status )
	{
	case WAITSETTLE:
		if( !ilchanged ) break;
		if( ilstatus == inductiveloop::NONE && tlstatus != trafficlights::OFF ) {
			m_lights.set( trafficlights::OFF, inner );
		} else if( conflict ) {
			inner = ilstatus == inductiveloop::INNER;
			m_lights.set( trafficlights::CONFLICT, inner );
		} else if( ilstatus != inductiveloop::NONE ) {
			inner = ilstatus == inductiveloop::INNER;
			m_lights.set( trafficlights::CODEWAIT, inner );
			m_status = CODEWAIT;
		}
		break;

	case CODEWAIT:
		if( ilchanged ) {
			if( conflict ) {
				m_lights.set( trafficlights::CONFLICT, inner );
				m_status = WAITSETTLE;
			} else if( ilstatus == inductiveloop::NONE ) {
				m_lights.set( trafficlights::OFF, inner );
				m_status = WAITSETTLE;
			} else {
				m_lights.set( trafficlights::OFF, inner );
				m_status = CODEWAIT;
			}
			break;
		}
		if( g_codeready ) {
			if( g_code & 1 ) {
				m_status = RETREAT;
				m_lights.set( trafficlights::DENIED, inner );
			} else {
				m_status = PASS;
				m_lights.set( trafficlights::ACCEPTED, inner );
			}
			g_codeready = false;
		}
		break;

	case PASS:
		if( !ilchanged ) break;
		if( conflict ) {
			m_lights.set( trafficlights::PASS, inner );
		} else if( ilstatus == inductiveloop::NONE ) {
			m_lights.set( trafficlights::OFF, inner );
			m_status = WAITSETTLE;
		}
		break;

	case RETREAT:
		if( !ilchanged ) break;
		//TODO: lights
		if( ilstatus != (inner ? inductiveloop::INNER : inductiveloop::OUTER)) {
			if( ilstatus == inductiveloop::NONE ) {
				m_lights.set( trafficlights::OFF, inner );
				m_status = WAITSETTLE;
			} else {
				m_lights.set( trafficlights::CODEWAIT, !inner );
				m_status = CODEWAIT;
			}
		}
		break;
	}

	m_ilstatus = ilstatus;
	m_conflict = conflict;

	m_lights.loop( currmillis );

}
