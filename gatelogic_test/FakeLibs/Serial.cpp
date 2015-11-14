#include <Serial.h>

fakeserial	Serial;

fakeserial::~fakeserial()
{
	// TODO Auto-generated destructor stub
}

void fakeserial::setdata( const std::string &s )
{
	m_input.clear();
	m_input.assign(s.c_str(), s.c_str() + s.length());
	m_inidx = 0;
}
