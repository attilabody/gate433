/*
 * gatehandler.cpp
 *
 *  Created on: Oct 29, 2015
 *      Author: compi
 */

#include "config.h"
#include "gatehandler.h"

gatehandler::gatehandler( database &db
			, trafficlights &lights
			, inductiveloop &loop
			, bool enforcepos
			, bool enforcedt )
: m_db( db )
, m_lights( lights )
, m_loop( loop )
, m_enforcepos( enforcepos )
, m_enforcedt( enforcedt )
, m_status( CLOSED )
{
}

