// Do not remove the include below
#include <Arduino.h>
#include <I2C.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

LiquidCrystal_I2C	g_display(LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT);

//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin(BAUDRATE);
	I2c.begin();
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
}
