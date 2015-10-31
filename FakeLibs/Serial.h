/*
 * fakeserial.h
 *
 *  Created on: Oct 30, 2015
 *      Author: compi
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>

class fakeserial
{
public:
	fakeserial() : m_index(0) {}
	virtual ~fakeserial();

	template<typename T> size_t print(T input) { std::cout << input; return 0; }
	template<typename T> size_t println(T input) { std::cout << input << std::endl; return 0; }

	void	setdata( const std::string &s );
	void	setdata( const std::vector<char> &v ) { m_data = v; }

	bool 	available() { return m_index < m_data.size(); }
	char	read() { return ( m_index < m_data.size() ) ? m_data.at( m_index++ ) : -1; }
	void	begin( unsigned long speed, uint8_t config = 0 ) {}

private:
	std::vector<char>	m_data;
	uint16_t			m_index;
};

extern fakeserial Serial;

#endif /* SERIAL_H_ */
