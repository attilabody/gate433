/*
 * display.h
 *
 *  Created on: Jan 30, 2016
 *      Author: compi
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

struct ts;

class display : public LiquidCrystal_I2C
{
public:
	display( byte i2c_address, byte width, byte height );
	~display();
	void init();

	void updatedt( const ts &dt, byte updatemask, bool timevalid );
	void updateloopstatus( bool inner, bool outer );
	void updatelastreceivedid( uint16_t id );
	void updatelastdecision( char decision, uint16_t id );

protected:

};

#endif /* DISPLAY_H_ */
