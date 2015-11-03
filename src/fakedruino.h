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
