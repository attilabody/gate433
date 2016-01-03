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
{
//	memset( m_lcdbuf, ' ', sizeof(m_lcdbuf)-1 );
//	m_lcdbuf[sizeof(m_lcdbuf)-1] = 0;
	m_lcd.setCursor( 0, 0 );
	m_lcd.print( F("lofaszbingo "));
	Serial.println( freeMemory());
}

//////////////////////////////////////////////////////////////////////////////
void gatehandler::loop( unsigned long currmillis )
{
	inductiveloop::STATUS	ilstatus;

	bool	conflict( m_indloop.update( ilstatus ));
	bool	ilchanged((ilstatus != m_ilstatus) || (conflict != m_conflict ));
	bool	inner( ilstatus == inductiveloop::INNER );

	if( ilchanged ) {
#ifdef VERBOSE
		serialoutln( F("inductive loop status changed: "), ilstatus, ", ", conflict );
#endif	//	VERBOSE
		Serial.println( freeMemory());
		delay( 500 );
		m_lcd.setCursor( 0, 0 );
		m_lcd.print( F("lofaszbingo "));
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
#endif	//	VERBOSE
			Serial.println( freeMemory());
			if( authorize( g_code, inner ) == GRANTED ) {
				topass( inner );
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
			m_db.setStatus( g_code >> 2, m_inner ? database::dbrecord::outside : database::dbrecord::inside );
			m_dbupdated = true;
#ifdef VERBOSE
			serialoutsepln( ", ", F("setdbstatus "), ilstatus, m_inner, m_inner ? database::dbrecord::outside : database::dbrecord::inside);
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

	m_lights.loop( currmillis );
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

//	rec.serialize( g_iobuf );
//	Serial.println( g_iobuf );

	if( !rec.in_start && !rec.in_end )
		ret = UNREGISTERED;
	else if( rec.position == ( inner ? database::dbrecord::outside : database::dbrecord::inside ) )
		ret = POSITION;
	else if( !( rec.days & dow ))
		ret = DAY;
	else {
		uint16_t	start( inner ? rec.out_start : rec.in_start );
		uint16_t	end( inner ? rec.out_end : rec.in_end );

		if( mod < start || mod > end )
			ret = TIME;
	}

	m_lcd.setCursor( 0, 0 );
	m_lcd.print( F("lofaszbingo"));
	//updatelcd( id, inner, ret );
	return ret;
}

//////////////////////////////////////////////////////////////////////////////
void gatehandler::updatelcd( uint16_t id, bool inner, AUTHRES decision )
{
	static const char authcodes[] = "GUDTP";

	m_lcd.setCursor( 0, 0 );
	m_lcd.print((char)(authcodes[decision] + (inner ? 'a'-'A' : 0)));
	m_lcd.print( id );

//	serialoutsepln( ", ", id, inner ? 'I':'O', (char)(authcodes[decision] + (inner ? 'a'-'A' : '\0')) );
//	char	*dst( m_lcdbuf + sizeof(m_lcdbuf) - 2 );
//	char	*src( dst - 6 );
//	do *dst-- = *src--; while( src >= m_lcdbuf );
//	++src;
//	*src++ = (char)(authcodes[decision] + (inner ? 'a'-'A' : 0));
//	uitodec( src, id, 4 );
//	Serial.println( m_lcdbuf );
//	m_lcd.setCursor( 0, 0 );
//	m_lcd.print( m_lcdbuf );
}
