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
const char gatehandler::m_authcodes[AUTHRESCNT] = { 'G', 'W', 'U', 'D', 'T', 'P' };

//////////////////////////////////////////////////////////////////////////////
gatehandler::gatehandler( database &db
		, trafficlights &lights
		, inductiveloop &loop
		, display &disp
		, unsigned long cyclelen)
: m_db( db )
, m_lights(lights)
, m_indloop( loop )
, m_display( disp )
, m_gate( g_outputs, PIN_GATE, RELAY_ON == HIGH )
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
	static	uint16_t	code(-1);
	static	uint8_t		codecount(0);

	bool	conflict( m_indloop.update( ilstatus ));
	bool	ilchanged((ilstatus != m_ilstatus) || (conflict != m_conflict ));
	bool	inner( ilstatus == inductiveloop::INNER );

#ifdef ENABLE_GODMODE
	if( g_code2ready ) {
		uint16_t	id( getid( g_code2 ));
		if( id >= GODMODE_MIN && id <= GODMODE_MAX ) {
			g_outputs.write( PIN_GATE, RELAY_ON );
		}
		g_code2ready = false;
	}
#endif	//	ENABLE_GODMODE

	if( ilchanged ) {
#ifdef __GATEHANDLER_VERBOSE
		serialoutln( F( CMNTS "il changed: "), ilstatus, ", ", conflict, " - ", freeMemory() );
#endif	//	__GATEHANDLER_VERBOSE
		g_logger.log(logwriter::DEBUG, g_time, F("ILS"), (uint16_t)ilstatus, (uint8_t)conflict);
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
		}
		else if( g_codeready )
		{
			if(code != g_code) {
				code = g_code;
				codecount = 0;
				g_codeready = false;
#ifdef __GATEHANDLER_VERBOSE
				serialoutln( F(CMNTS "Aborted code: "), getid(g_code), ", ", getid(code), " - ", freeMemory() );
#endif	//	__GATEHANDLER_VERBOSE
			}
			else if( codecount++ )
			{
				uint16_t	id( getid( g_code ) );
#ifdef __GATEHANDLER_VERBOSE
				serialoutln( F(CMNTS "Code received: "), id, " - ", freeMemory() );
#endif	//	__GATEHANDLER_VERBOSE
				AUTHRES	ar( authorize( id, inner ));
				m_display.updatelastdecision( pgm_read_byte( m_authcodes+ ar ) + (inner ? 'a'-'A' : 0), id );
				if( ar == GRANTED ) {
					topass( inner, currmillis );
				} else if( ar ==  WARN ) {
					topass_warn( inner, currmillis );
				} else {
					m_lights.set( trafficlights::DENIED, inner );
					m_inner = inner;
					m_status = RETREAT;
					m_phasestart = currmillis;
				}
			}
		}
		break;

	case PASS:
		if( !ilchanged && currmillis - m_phasestart <= PASS_TIMEOUT )
			break;
		if( ilstatus == inductiveloop::NONE || currmillis - m_phasestart > PASS_TIMEOUT ) {
			m_db.setStatus( getid( g_code ), m_inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE );
			g_logger.log( logwriter::DEBUG, g_time, F("DBUP"), -1 );
#ifdef __GATEHANDLER_VERBOSE
			serialoutsepln( ", ", F("setdbstatus "), ilstatus, m_inner, m_inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE);
#endif	//	__GATEHANDLER_VERBOSE
			m_lights.set( trafficlights::OFF, inner );
			m_gate.set( false, 0, 0, true, currmillis );
			ilstatus = inductiveloop::NONE;
			m_status = WAITSETTLE;
		}
		else if( conflict ) {
			m_lights.set( trafficlights::PASS, inner );
		}
		break;

	case RETREAT:
		if( ilchanged || currmillis - m_phasestart > RETREAT_TIMEOUT ) {
			m_lights.set( trafficlights::OFF, inner );
			m_gate.set( false, 0, 0, true, currmillis );
			ilstatus = inductiveloop::NONE;
			m_status = WAITSETTLE;
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

	uint16_t	mod( g_time.min + g_time.hour * 60 );
	uint8_t		dow( 1<<(g_time.wday - 1));
	if( !m_db.getParams(id, rec ) )
		return ret;

	if( !rec.in_start && !rec.in_end )
		ret = UNREGISTERED;
	else if( rec.position == ( inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE ) )
		ret = POSITION;
	else if(g_timevalid)
	{
		if( !( rec.days & dow ))
			ret = DAY;
		else {
			uint16_t	start( inner ? rec.out_start : rec.in_start );
			uint16_t	end( inner ? rec.out_end : rec.in_end );

			if( mod < start || mod > end )
				ret = TIME;
		}
	}
	g_logger.log( logwriter::INFO, g_time, F("Auth"), id, rec.position, inner, ret );
#if defined(RELAXED_POS) && defined(RELAXED_TIME)
	if(ret == DAY || ret == TIME || ret == POSITION )
#elif defined(RELAXED_POS)
	if(((rec.days & 0x80) && ret > UNREGISTERED) || ret == POSITION)
#elif defined(RELAXED_TIME)
	if(((rec.days & 0x80) && ret > UNREGISTERED) || ret == DAY || ret == TIME)
#else
	if((rec.days & 0x80) && ret > UNREGISTERED)
#endif
		ret = WARN;
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
void gatehandler::tocodewait( bool inner )
{
	m_lights.set( trafficlights::CODEWAIT, inner );
	g_codeready = false;
	m_status = CODEWAIT;
}
