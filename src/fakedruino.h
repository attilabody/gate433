/*
 * fakedruino.h
 *
 *  Created on: Oct 6, 2015
 *      Author: abody
 */

#ifndef FAKEDRUINO_H_
#define FAKEDRUINO_H_

#include <stdlib.h>
#include <stdint.h>

#define CHANGE 1
#define FALLING 2
#define RISING 3

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define digitalPinToInterrupt(p)  ((p) == 2 ? 0 : ((p) == 3 ? 1 : -1))


void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);
int analogRead(uint8_t);
void analogReference(uint8_t mode);
void analogWrite(uint8_t, int);

unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long);
void delayMicroseconds(unsigned int us);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout);

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

void attachInterrupt(uint8_t, void (*)(void), int mode);
void detachInterrupt(uint8_t);



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

class HardwareSerial
{
  public:
    inline HardwareSerial(
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr) {}
    inline HardwareSerial() {}
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
    template< typename T > void print(T&) {}
    template< typename T > void println(T&) {}
//    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }

    // Interrupt handlers - Not intended to be called externally
};

extern HardwareSerial Serial;


#define DEC 1

class String
{
public:
	template< typename T > String( T ) {}
	template< typename T, typename U > String( T, U ) {}
	template< typename T > String& operator+( T ) { return *this; }
};

// Helper functions
void set_time_table( unsigned long *table, unsigned long count );
void set_time_table_delta( unsigned long *table, unsigned long count );

#define TIMER0_COMPA_vect	void
#define ISR(vect) void interruphadler()

void noInterrupts();
void interrupts();
extern unsigned char TIMSK0;
#define OCIE0A 1

#endif /* FAKEDRUINO_H_ */
