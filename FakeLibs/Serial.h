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

// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

class FakeSerial
{
  public:
    inline FakeSerial(
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr) {}
    inline FakeSerial() {}
    void begin(unsigned long baud) { begin(baud, SERIAL_8N1); }
    void begin(unsigned long, uint8_t) {};
    void end();
    virtual int available(void) { return 0; }
    virtual int peek(void) { return 0; }
    virtual int read(void) { return 0; }
    int availableForWrite(void);
    virtual void flush(void) {}
    virtual size_t write(uint8_t) { return 0; }
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
//    template< typename T > void print(T&, int) {}
//    template< typename T > void println(T&, int) {}
    template< typename T > void print(const T&) {}
    template< typename T > void println(const T&) {}
//    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }

    // Interrupt handlers - Not intended to be called externally
};

extern FakeSerial Serial;

class fakeserial
{
public:
	fakeserial() : m_index(0) {}
	virtual ~fakeserial();

	template<typename T> size_t print(T input) { std::cout << input; return 0; }
	template<typename T> size_t println(T input) { std::cout << input << std::endl; return 0; }
	template<typename T> size_t print(T input, int format) { std::cout << input; return 0; }
	template<typename T> size_t println(T input, int format) { std::cout << input << std::endl; return 0; }

	void	setdata( const std::string &s );
	void	setdata( const std::vector<char> &v ) { m_data = v; }

	bool 	available() { return m_index < m_data.size(); }
	char	read() { return ( m_index < m_data.size() ) ? m_data.at( m_index++ ) : -1; }
	void	begin( unsigned long speed, uint8_t config = 0 ) {}

private:
	std::vector<char>	m_data;
	uint16_t			m_index;
};

extern FakeSerial Serial;

#endif /* SERIAL_H_ */
