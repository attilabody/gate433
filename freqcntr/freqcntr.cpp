// Do not remove the include below
#include <Arduino.h>
#include <I2C.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

volatile unsigned long g_timerCounts;
volatile boolean g_counterReady;

// internal to counting routine
unsigned long g_overflowCount;
unsigned int g_timerTicks;
unsigned int g_timerPeriod;

#ifdef USE_I2CLCD
LiquidCrystal_I2C	g_lcd(LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
#endif	//	USE_I2CLCD

void setup()
{
	Serial.begin(BAUD);
#ifdef	USE_I2CLCD
	I2c.begin();
	g_lcd.init();
	g_lcd.backlight();
#endif	//	USE_I2CLCD
}


void startCounting(unsigned int ms)
{
	g_counterReady = false;         // time not up yet
	g_timerPeriod = ms;             // how many 1 ms counts to do
	g_timerTicks = 0;               // reset interrupt counter
	g_overflowCount = 0;            // no overflows yet

	// reset Timer 1 and Timer 2
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR2A = 0;
	TCCR2B = 0;

	// Timer 1 - counts events on pin D5
	TIMSK1 = bit(TOIE1);   // interrupt on Timer 1 overflow

	// Timer 2 - gives us our 1 ms counting interval
	// 16 MHz clock (62.5 ns per tick) - prescaled by 128
	//  counter increments every 8 µs.
	// So we count 125 of them, giving exactly 1000 µs (1 ms)
	TCCR2A = bit(WGM21);   // CTC mode
	OCR2A = 124;            // count up to 125  (zero relative!!!!)

	// Timer 2 - interrupt on match (ie. every 1 ms)
	TIMSK2 = bit(OCIE2A);   // enable Timer2 Interrupt

	TCNT1 = 0;      // Both counters to zero
	TCNT2 = 0;

	// Reset prescalers
	GTCCR = bit(PSRASY);        // reset prescaler now
	// start Timer 2
	TCCR2B = bit (CS20) | bit(CS22);  // prescaler of 128
	// start Timer 1
	// External clock source on T1 pin (D5). Clock on rising edge.
	TCCR1B = bit (CS10) | bit(CS11) | bit(CS12);
}  // end of startCounting

////******************************************************************
ISR (TIMER1_OVF_vect)
{
	++g_overflowCount;               // count number of Counter1 overflows
}  // end of TIMER1_OVF_vect

//******************************************************************
//  Timer2 Interrupt Service is invoked by hardware Timer 2 every 1 ms = 1000 Hz
//  16Mhz / 128 / 125 = 1000 Hz

ISR (TIMER2_COMPA_vect)
{
	// grab counter value before it changes any more
	unsigned int timer1CounterValue;
	timer1CounterValue = TCNT1; // see datasheet, page 117 (accessing 16-bit registers)
	unsigned long overflowCopy = g_overflowCount;

	// see if we have reached timing period
	if (++g_timerTicks < g_timerPeriod)
		return;  // not yet

	// if just missed an overflow
	if ((TIFR1 & bit(TOV1)) && timer1CounterValue < 256)
		overflowCopy++;

	// end of gate time, measurement ready

	TCCR1A = 0;    // stop timer 1
	TCCR1B = 0;

	TCCR2A = 0;    // stop timer 2
	TCCR2B = 0;

	TIMSK1 = 0;    // disable Timer1 Interrupt
	TIMSK2 = 0;    // disable Timer2 Interrupt

	// calculate total count
	g_timerCounts = (overflowCopy << 16) + timer1CounterValue; // each overflow is 65536 more
	g_counterReady = true;              // set global flag for end count period
}  // end of TIMER2_COMPA_vect

//******************************************************************
void loop()
{
	// stop Timer 0 interrupts from throwing the count out
	byte oldTCCR0A = TCCR0A;
	byte oldTCCR0B = TCCR0B;
	TCCR0A = 0;    // stop timer 0
	TCCR0B = 0;

	startCounting(500);  // how many ms to count for

	while (!g_counterReady) {
	}  // loop until count over

	// adjust counts by counting interval to give frequency in Hz
	float frq = (g_timerCounts * 1000.0) / g_timerPeriod;

	long lf((unsigned long)frq);
#ifdef USE_I2CLCD
	g_lcd.setCursor(0,0);
	g_lcd.print(lf);
	g_lcd.print(F(" Hz        "));
#endif	//	USE_I2CLCD

	Serial.print("Frequency: ");
	Serial.print(lf);
	Serial.println(" Hz.");

	// restart timer 0
	TCCR0A = oldTCCR0A;
	TCCR0B = oldTCCR0B;

	// let serial stuff finish
	delay(200);
}   // end of loop
