#include "Arduino.h"
#include "HardwareSerial.h"
#include <I2C.h>
#include <ds3231.h>

DS3231 Time;

void TestDST(uint16_t y, uint8_t m, uint8_t d, uint8_t h, uint8_t w)
{
	Serial.print(y);
	Serial.print('-');
	Serial.print(m);
	Serial.print('-');
	Serial.print(d);
	Serial.print(' ');
	Serial.print(h);
	Serial.print('=');
	Serial.print(w);
	Serial.print('?');
	Serial.print(Time.check_dst(y-2000,m,d,h));
	Serial.print("\n");
}

//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin(115200);
	I2c.begin();
	I2c.timeOut(100);
	Time.init(DS3231_INTCN);
	for (int i=1;i<=12;i++) {
		if (i>1)
			Serial.print(',');
		Serial.print(Time.dow(16,i,1));
	}
	Serial.print("\n");
	for (int i=1;i<=12;i++) {
		if (i>1)
			Serial.print(',');
		Serial.print(Time.mon_last_day(16,i));
	}
	Serial.print("\n");
	TestDST(2016,1,1,12,0);
	TestDST(2016,3,1,12,0);
	TestDST(2016,3,26,12,0);
	TestDST(2016,3,27,0,0);
	TestDST(2016,3,27,1,0);
	TestDST(2016,3,27,2,1);
	TestDST(2016,3,27,3,1);
	TestDST(2016,3,27,12,1);
	TestDST(2016,3,28,12,1);
	TestDST(2016,4,1,12,1);
	TestDST(2016,9,1,12,1);
	TestDST(2016,10,1,12,1);
	TestDST(2016,10,29,12,1);
	TestDST(2016,10,30,0,1);
	TestDST(2016,10,30,1,1);
	TestDST(2016,10,30,2,0);
	TestDST(2016,10,30,3,0);
	TestDST(2016,10,30,12,0);
	TestDST(2016,10,31,12,0);
	TestDST(2016,12,31,12,0);
	Time.set("201603270159");
	//Time.set("201610300159");
}

ts			g_dt;
// The loop function is called in an endless loop
void loop()
{
	Time.get(&g_dt);
	delay(1000);
	Serial.print(g_dt.year);
	Serial.print('-');
	Serial.print(g_dt.mon);
	Serial.print('-');
	Serial.print(g_dt.mday);
	Serial.print(' ');
	Serial.print(g_dt.hour);
	Serial.print(':');
	Serial.print(g_dt.min);
	Serial.print(':');
	Serial.print(g_dt.sec);
	Serial.print(' ');
	Serial.print(g_dt.error);
	Serial.print("\n");
}


