/*
 * fakeserial.cpp
 *
 *  Created on: Oct 30, 2015
 *      Author: compi
 */

#include <Serial.h>

fakeserial	Serial;

fakeserial::~fakeserial()
{
	// TODO Auto-generated destructor stub
}

void fakeserial::setdata( const std::string &s )
{
	m_data.clear();
	m_data.insert(begin(m_data), s.c_str(), s.c_str() + s.length());
}
