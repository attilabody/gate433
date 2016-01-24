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
#include "globals.h"

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
const char *gatehandler::m_authcodes("GUDTP");

//////////////////////////////////////////////////////////////////////////////
gatehandler::gatehandler( database &db
			, trafficlights &lights
			, inductiveloop &loop
			, LiquidCrystal_I2C &lcd
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
, m_prevdecision( (AUTHRES) -1 )
, m_openstart(0)
{
}

//////////////////////////////////////////////////////////////////////////////
char gatehandler::loop( unsigned long currmillis )
{
	inductiveloop::STATUS	ilstatus;

	bool	conflict( m_indloop.update( ilstatus ));
	bool	ilchanged((ilstatus != m_ilstatus) || (conflict != m_conflict ));
	bool	inner( ilstatus == inductiveloop::INNER );
	char	ret( 0 );

	if( ilchanged ) {
#ifdef VERBOSE
		serialoutln( F("inductive loop status changed: "), ilstatus, ", ", conflict );
		Serial.println( freeMemory());
#endif	//	VERBOSE
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
#ifdef VERBOSE
			serialoutln( F(CMNTS "Code received: "), g_code >> 2);
			Serial.println( freeMemory());
#endif	//	VERBOSE
			AUTHRES	ar( authorize( g_code, inner ));
			ret = (char)(m_authcodes[ar] + (inner ? 'a'-'A' : 0));
			if( ar == GRANTED ) {
				m_openstart = currmillis ? currmillis : 1;
				topass( inner );
#ifndef ENFORCING
			} else if( ar ==  POSITION || ar == DAY || ar == TIME) {
				m_openstart = currmillis ? currmillis : 1;
				topass_warn( inner );
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
			m_db.setStatus( g_code >> 2, m_inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE );
			m_dbupdated = true;
#ifdef VERBOSE
			serialoutsepln( ", ", F("setdbstatus "), ilstatus, m_inner, m_inner ? database::dbrecord::OUTSIDE : database::dbrecord::INSIDE);
#endif	//	VERBOSE
		}
		if( conflict ) {
			m_lights.set( trafficlights::PASS, inner );
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
	}

	m_ilstatus = ilstatus;
	m_conflict = conflict;
	if( m_openstart && currmillis - m_openstart > GATE_OPEN_PULSE_WIDTH ) {
		g_i2cio.write( PIN_GATE, RELAY_OFF );
	}

	m_lights.loop( currmillis );
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
gatehandler::AUTHRES gatehandler::authorize( uint16_t code, bool inner )
{
	uint16_t			id( code >> 2 );
	ts					dt;
	database::dbrecord	rec;
	AUTHRES				ret( GRANTED );

	DS3231_get( &dt );
	uint16_t	mod( dt.min + dt.hour * 60 );
	uint8_t		dow( 1<<(dt.wday - 1));	//TODO exceptions, holidays
	if( !m_db.getParams(id, rec ) )
		return GRANTED;

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

//	updatelcd( id, inner, ret );
	return ret;
}

/*//////////////////////////////////////////////////////////////////////////////
void gatehandler::updatelcd( uint16_t id, bool inner, AUTHRES decision )
{
	static const char authcodes[] = "GUDTP";
	char buf[5];
	char*bp(buf);

	uitodec( bp, id, 4);
	*bp = 0;

	m_lcd.setCursor( 0, 0 );
	m_lcd.print((char)(authcodes[decision] + (inner ? 'a'-'A' : 0)));
	m_lcd.print( ' ' );
	m_lcd.print( buf );
	if( (char)m_prevdecision != -1 )
	{
		bp = buf;
		uitodec( bp, m_previd, 4);
		m_lcd.print( ' ' );
		m_lcd.print( ' ' );
		m_lcd.print((char)(authcodes[m_prevdecision] + (m_previnner ? 'a'-'A' : 0)));
		m_lcd.print( ' ' );
		m_lcd.print( buf );
	}
	m_prevdecision = decision;
	m_previd = id;
	m_previnner = inner;
}
*/
