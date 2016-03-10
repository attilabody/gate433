/*
 * gatehandler.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */
#include <ds3231.h>
#include <MemoryFree.h>
#include "config.h"
#include "gatehandler.h"
#include "decode433.h"
#include "sdfatlogwriter.h"
#include "globals.h"
#include "toolbox.h"
#include <serialout.h>
#include <commsyms.h>

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
const char gatehandler::m_authcodes[AUTHRESCNT] = { 'G', 'U', 'D', 'T', 'P' };

//////////////////////////////////////////////////////////////////////////////
gatehandler::gatehandler( database &db
			, unsigned long cyclelen
			, inductiveloop &loop
			, display &disp )
: m_db( db )
, m_lights()
, m_gate( PIN_GATE, RELAY_ON == HIGH )
, m_indloop( loop )
, m_display( disp )
, m_status( WAITSETTLE )
, m_tlstatus( trafficlights::OFF )
, m_ilstatus( inductiveloop::NONE )
, m_conflict( false )
, m_inner( false )
, m_prevdecision( (AUTHRES) -1 )
{
	const uint8_t innerlightspins[3] = { INNER_LIGHTS_PINS };
	const uint8_t outerlightspins[3] = { OUTER_LIGHTS_PINS };
	m_lights.init( innerlightspins, outerlightspins, cyclelen );
}

//////////////////////////////////////////////////////////////////////////////
void gatehandler::loop( unsigned long currmillis )
{
	inductiveloop::STATUS	ilstatus;

	bool	conflict( m_indloop.update( ilstatus ));
	bool	ilchanged((ilstatus != m_ilstatus) || (conflict != m_conflict ));
	bool	inner( ilstatus == inductiveloop::INNER );

	if( g_code2ready ) {
		uint16_t	id( getid( g_code2 ));
		if( id >= GODMODE_MIN && id <= GODMODE_MAX ) {
			g_outputs.write( PIN_GATE, RELAY_ON );
		}
		g_code2ready = false;
	}

	if( ilchanged ) {
#ifdef __GATEHANDLER_VERBOSE
		serialoutln( F( CMNTS "il changed: "), ilstatus, ", ", conflict );
		Serial.println( freeMemory());
#endif	//	__GATEHANDLER_VERBOSE
		bool il(false), ol(false);
		if( conflict ) il = ol = true;
		else if( ilstatus == inductiveloop::INNER ) il = true;
		else if( ilstatus == inductiveloop::OUTER ) ol = true;
		m_display.updateloopstatus( il, ol );
	}

	switch( m_status )
	{
	case WAITSETTLE:
		if( !ilchanged ) break;
		if( ilstatus == inductiveloop::NONE && m_lights != trafficlights::OFF ) {
			m_lights.set( trafficlights::OFF, inner );
		} else if( conflict ) {
			m_lights.set( trafficlights::CONFLICT, inner );
		} else if( ilstatus != inductiveloop::NONE ) {
			tocodewait( inner );
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
				tocodewait( inner );	// wtf???
			}
			break;
		}
		if( g_codeready ) {
			uint16_t	id( getid( g_code ) );
#ifdef __GATEHANDLER___GATEHANDLER_VERBOSE
			serialoutln( F(CMNTS "Code received: "), id );
			Serial.println( freeMemory());
#endif	//	__GATEHANDLER_VERBOSE
			AUTHRES	ar( authorize( id, inner ));
			m_display.updatelastdecision( pgm_read_byte( m_authcodes+ ar ) + (inner ? 'a'-'A' : 0), id );
			if( ar == GRANTED ) {
				topass( inner, currmillis );
#ifndef ENFORCING
			} else if( ar ==  POSITION || ar == DAY || ar == TIME) {
				topass_warn( inner, currmillis );
#endif	//	ENFORCING
			} else {
				m_lights.set( trafficlights::DENIED, inner );
				m_inner = inner;
				m_status = RETREAT;
			}
		}
		break;

	case PASS:
		if( !ilchanged ) break;
		if( !m_dbupdated && ilstatus != (m_inner ? inductiveloop::INNER : inductiveloop::OUTER) ) {
			g_logger.log( logwriter::DEBUG, g_t, F("DBUP1"), -1 );
			m_db.setStatus( getid( g_code ), m_inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE );
			g_logger.log( logwriter::DEBUG, g_t, F("DBUP2"), -1 );
			m_dbupdated = true;
#ifdef __GATEHANDLER_VERBOSE
			serialoutsepln( ", ", F("setdbstatus "), ilstatus, m_inner, m_inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE);
#endif	//	__GATEHANDLER_VERBOSE
		}
		if( conflict ) {
//			m_lights.set( trafficlights::PASS, inner );
		} else if( ilstatus == inductiveloop::NONE ) {
			m_lights.set( trafficlights::OFF, inner );
			m_status = WAITSETTLE;
		}
		break;

	case RETREAT:
		if( !ilchanged ) break;
		if( ilstatus != ( m_inner ? inductiveloop::INNER : inductiveloop::OUTER)) {
			if( ilstatus == inductiveloop::NONE ) {
				m_lights.set( trafficlights::OFF, inner );
				m_status = WAITSETTLE;
			} else {
				tocodewait( inner );
			}
		}
		break;
	default:		// Invalid state
		// TODO debug
		m_lights.set( trafficlights::OFF, inner );
		m_status = WAITSETTLE;
		break;
	}

	m_ilstatus = ilstatus;
	m_conflict = conflict;

	m_lights.loop( currmillis );
	m_gate.loop( currmillis );
}

//////////////////////////////////////////////////////////////////////////////
gatehandler::AUTHRES gatehandler::authorize( uint16_t id, bool inner )
{
	database::dbrecord	rec;
	AUTHRES				ret( GRANTED );

	uint16_t	mod( g_t.min + g_t.hour * 60 );
	uint8_t		dow( 1<<(g_t.wday - 1));
	if( !m_db.getParams(id, rec ) )
		return ret;
	if( rec.days & 0x80 )
		return ret;

	if( !rec.in_start && !rec.in_end )
		ret = UNREGISTERED;
	else if( rec.position == ( inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE ) )
		ret = POSITION;
	else if( !( rec.days & dow ))
		ret = DAY;
	else {
		uint16_t	start( inner ? rec.out_start : rec.in_start );
		uint16_t	end( inner ? rec.out_end : rec.in_end );

		if( mod < start || mod > end )
			ret = TIME;
	}

	g_logger.log( logwriter::INFO, g_t, F("Auth"), id, rec.position, inner, ret );
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
void gatehandler::tocodewait( bool inner )
{
	m_lights.set( trafficlights::CODEWAIT, inner );
	g_codeready = false;
	m_status = CODEWAIT;
}

