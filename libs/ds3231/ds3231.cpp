
/*
  DS3231 library for the Arduino.

  This library implements the following features:

   - read/write of current time, both of the alarms, 
   control/status registers, aging register
   - read of the temperature register, and of any address from the chip.

  Author:          Petre Rodan <petre.rodan@simplex.ro>
  Available from:  https://github.com/rodan/ds3231
 
  The DS3231 is a low-cost, extremely accurate I2C real-time clock 
  (RTC) with an integrated temperature-compensated crystal oscillator 
  (TCXO) and crystal.

  GNU GPLv3 license:
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
   
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
   
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
   
*/

#include <I2C.h>
#include <stdio.h>
#include "ds3231.h"

#define Skip_DS3231Extra	1

#ifdef __AVR__
 #include <avr/pgmspace.h>
 // Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
 #ifdef PROGMEM
  #undef PROGMEM
  #define PROGMEM __attribute__((section(".progmem.data")))
 #endif
#else
 #define PROGMEM
 #define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif

#define CheckLevel_Mon		2
#define CheckLevel_Day		1
#define CheckLevel_Hour		0

/* control register 0Eh/8Eh
bit7 EOSC   Enable Oscillator (1 if oscillator must be stopped when on battery)
bit6 BBSQW  Battery Backed Square Wave
bit5 CONV   Convert temperature (1 forces a conversion NOW)
bit4 RS2    Rate select - frequency of square wave output
bit3 RS1    Rate select
bit2 INTCN  Interrupt control (1 for use of the alarms and to disable square wave)
bit1 A2IE   Alarm2 interrupt enable (1 to enable)
bit0 A1IE   Alarm1 interrupt enable (1 to enable)
*/

uint8_t DS3231::init(const uint8_t ctrl_reg)
{
    return set_creg(ctrl_reg);
}

uint8_t DS3231::set(struct ts t)
{
    uint8_t i;

    t.year_s = t.year - 2000;

    uint8_t TimeDate[7] = { t.sec, t.min, t.hour, t.wday, t.mday, t.mon, t.year_s };

    for (i = 0; i <= 6; i++) {
        TimeDate[i] = dectobcd(TimeDate[i]);
        if (i == 5)
            TimeDate[5] |= 0x80;
    }
    if ((i=I2c.write(DS3231_I2C_ADDR,DS3231_TIME_CAL_ADDR,TimeDate,7)))
    	return i;
    i=set_sreg(0x0F);	// OSF clear
   	return i;
}

uint8_t DS3231::set(const char *buf)
{
	struct ts t;
	strtots(buf,&t);
	return set(t);
}

uint8_t DS3231::get(struct ts *t)
{
    uint8_t TimeDate[7];        //second,minute,hour,dow,day,month,year
    uint8_t i, n, ret, osf_error;

    if ((ret=get_sreg(&i)))
    	return ret;

    osf_error=(i & DS3231_OSF)?0xFF:0;

    if ((ret=I2c.read(DS3231_I2C_ADDR,DS3231_TIME_CAL_ADDR,7)))
    	return ret;

    for (i = 0; i <= 6; i++) {
        n = I2c.receive();
        if (i == 5) {
            TimeDate[5] = bcdtodec(n & 0x1F);
        } else
            TimeDate[i] = bcdtodec(n);
    }

    t->sec = TimeDate[0];
    t->min = TimeDate[1];
    t->hour = TimeDate[2];
    t->mday = TimeDate[4];
    t->mon = TimeDate[5];
    t->year = 2000 + TimeDate[6];
    t->wday = TimeDate[3];
    t->year_s = TimeDate[6];
#ifdef CONFIG_UNIXTIME
    t->unixtime = get_unixtime(*t);
#endif
    return osf_error;
}

uint8_t DS3231::set_addr(const uint8_t addr, const uint8_t val)
{
    return I2c.write(DS3231_I2C_ADDR,addr,val);
}

uint8_t DS3231::get_addr(const uint8_t addr, uint8_t * val)
{
    uint8_t rv;

    rv=I2c.read(DS3231_I2C_ADDR,addr,1);
    if (rv)
    	return rv;

    *val= I2c.receive();

    return 0;
}

// control register

uint8_t DS3231::set_creg(const uint8_t val)
{
    return set_addr(DS3231_CONTROL_ADDR, val);
}

// status register 0Fh/8Fh

/*
bit7 OSF      Oscillator Stop Flag (if 1 then oscillator has stopped and date might be innacurate)
bit3 EN32kHz  Enable 32kHz output (1 if needed)
bit2 BSY      Busy with TCXO functions
bit1 A2F      Alarm 2 Flag - (1 if alarm2 was triggered)
bit0 A1F      Alarm 1 Flag - (1 if alarm1 was triggered)
*/

uint8_t DS3231::set_sreg(const uint8_t val)
{
    return set_addr(DS3231_STATUS_ADDR, val);
}

uint8_t DS3231::get_sreg(uint8_t * val)
{
	return get_addr(DS3231_STATUS_ADDR, val);
}

#ifndef Skip_DS3231Extra
// aging register

uint8_t DS3231::set_aging(const int8_t val)
{
    uint8_t reg;

    if (val >= 0)
        reg = val;
    else
        reg = ~(-val) + 1;      // 2C

    return set_addr(DS3231_AGING_OFFSET_ADDR, reg);
}

uint8_t DS3231::get_aging(int8_t * val)
{
    uint8_t ret,reg;

    ret=get_addr(DS3231_AGING_OFFSET_ADDR,&reg);
    if (!ret)
    	return ret;

    if ((reg & 0x80) != 0)
        *val = reg | ~((1 << 8) - 1);     // if negative get two's complement
    else
        *val = reg;

    return 0;
}

// temperature register

float DS3231::get_treg()
{
    float rv;
    uint8_t temp_msb, temp_lsb;
    int8_t nint;

    I2c.read(DS3231_I2C_ADDR,DS3231_TEMPERATURE_ADDR,2);	// No error check!

    temp_msb = I2c.receive();
    temp_lsb = I2c.receive() >> 6;

    if ((temp_msb & 0x80) != 0)
        nint = temp_msb | ~((1 << 8) - 1);      // if negative get two's complement
    else
        nint = temp_msb;

    rv = 0.25 * temp_lsb + nint;

    return rv;
}

// alarms

// flags are: A1M1 (seconds), A1M2 (minutes), A1M3 (hour), 
// A1M4 (day) 0 to enable, 1 to disable, DY/DT (dayofweek == 1/dayofmonth == 0)
void DS3231::set_a1(const uint8_t s, const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t * flags)
{
    uint8_t t[4] = { s, mi, h, d };
    uint8_t i;

    for (i = 0; i <= 3; i++) {
        if (i == 3) {
            t[i]=dectobcd(t[3]) | (flags[3] << 7) | (flags[4] << 6);
        } else
            t[i]=dectobcd(t[i]) | (flags[i] << 7);
    }

    I2c.write(DS3231_I2C_ADDR,DS3231_ALARM1_ADDR,t,4);	// No error check!
}

void DS3231::get_a1(char *buf, const uint8_t len)
{
    uint8_t n[4];
    uint8_t t[4];               //second,minute,hour,day
    uint8_t f[5];               // flags
    uint8_t i;

    I2c.read(DS3231_I2C_ADDR,DS3231_ALARM1_ADDR,4);	// No error check!

    for (i = 0; i <= 3; i++) {
        n[i] = I2c.receive();
        f[i] = (n[i] & 0x80) >> 7;
        t[i] = bcdtodec(n[i] & 0x7F);
    }

    f[4] = (n[3] & 0x40) >> 6;
    t[3] = bcdtodec(n[3] & 0x3F);

    snprintf(buf, len,
             "s%02d m%02d h%02d d%02d fs%d m%d h%d d%d wm%d %d %d %d %d",
             t[0], t[1], t[2], t[3], f[0], f[1], f[2], f[3], f[4], n[0],
             n[1], n[2], n[3]);

}

// when the alarm flag is cleared the pulldown on INT is also released
void DS3231::clear_a1f(void)
{
    uint8_t reg_val;

    if (!get_sreg(&reg_val))
    	return;
    set_sreg(reg_val & ~DS3231_A1F);
}

uint8_t DS3231::triggered_a1(void)
{
    uint8_t reg_val;
    if (!get_sreg(&reg_val))
    	reg_val=0;
	return  reg_val & DS3231_A1F;
}

/* flag bits are:
 * 0:	DY/DT (dayofweek == 1/dayofmonth == 0)
 * 1:	A2M4 (day) 0 to enable
 * 2:	A2M3 (hour) 0 to enable
 * 3:	A2M2 (minutes) 0 to enable
 */
void DS3231::set_a2(const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t flags)
{
    uint8_t t[3] = { mi, h, d };
    uint8_t i;

    for (i = 0; i <= 2; i++) {
        if (i == 2) {
            t[i]=dectobcd(t[2]) | ((flags << 6) & 0xC0);
        } else
            t[i]=dectobcd(t[i]) | ((flags << (4-i)) & 0x80);
    }

    I2c.write(DS3231_I2C_ADDR,DS3231_ALARM2_ADDR,t,3);	// No error check!
}

void DS3231::get_a2(char *buf, const uint8_t len)
{
    uint8_t n[3];
    uint8_t t[3];               //second,minute,hour,day
    uint8_t f[4];               // flags
    uint8_t i;

    I2c.read(DS3231_I2C_ADDR,DS3231_ALARM2_ADDR,3);	// No error check!

    for (i = 0; i <= 2; i++) {
        n[i] = I2c.receive();
        f[i] = (n[i] & 0x80) >> 7;
        t[i] = bcdtodec(n[i] & 0x7F);
    }

    f[3] = (n[2] & 0x40) >> 6;
    t[2] = bcdtodec(n[2] & 0x3F);

    snprintf(buf, len, "m%02d h%02d d%02d fm%d h%d d%d wm%d %d %d %d", t[0],
             t[1], t[2], f[0], f[1], f[2], f[3], n[0], n[1], n[2]);

}

// when the alarm flag is cleared the pulldown on INT is also released
void DS3231::clear_a2f(void)
{
    uint8_t reg_val;

    if (!get_sreg(&reg_val))
        	return;
    set_sreg(reg_val & ~DS3231_A2F);
}

uint8_t DS3231::triggered_a2(void)
{
    uint8_t reg_val;
    if (!get_sreg(&reg_val))
    	reg_val=0;
	return  reg_val & DS3231_A2F;
}

#endif

// helpers

#ifdef CONFIG_UNIXTIME
const uint8_t days_in_month [12] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };

// returns the number of seconds since 01.01.1970 00:00:00 UTC, valid for 2000..FIXME
uint32_t DS3231::get_unixtime(struct ts t)
{
    uint8_t i;
    uint16_t d;
    int16_t y;
    uint32_t rv;

    if (t.year >= 2000) {
        y = t.year - 2000;
    } else {
        return 0;
    }

    d = t.mday - 1;
    for (i=1; i<t.mon; i++) {
        d += pgm_read_byte(days_in_month + i - 1);
    }
    if (t.mon > 2 && y % 4 == 0) {
        d++;
    }
    // count leap days
    d += (365 * y + (y + 3) / 4);
    rv = ((d * 24UL + t.hour) * 60 + t.min) * 60 + t.sec + SECONDS_FROM_1970_TO_2000;
    return rv;
}
#endif

uint8_t DS3231::dectobcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

uint8_t DS3231::bcdtodec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

uint8_t DS3231::inp2toi(const char * &c)
{
    uint8_t rv;
    rv = (*c++ - 48) * 10;
    rv += *c++ - 48;
    return rv;
}

uint8_t DS3231::dow(uint8_t y, uint8_t m, uint8_t d)
{
	static uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	y -= m < 3;
	return ((y + (y >> 2) + t[m-1] + d + 6) % 7) +1;
}

void DS3231::strtots(const char *buf, struct ts *t)
{
	buf+=2;
	t->year=2000+inp2toi(buf);
	t->mon=inp2toi(buf);
	t->mday=inp2toi(buf);
	t->hour=inp2toi(buf);
	t->min=inp2toi(buf);
	t->sec=0;
	t->wday=dow(t->year-2000,t->mon,t->mday);
}

// ----------------------------- DS3231_DST -------------------------------

uint8_t DS3231_DST::set(struct ts t)
{
    uint8_t i;

    t.year_s = t.year - 2000;

    dst=check_dst(t.year_s,t.mon,t.mday,t.hour);
    if (dst)									// Just hour correction!
    	t.hour--;

    i=DS3231::set(t);

    if (i)
    {
    	m_year=0;
    	return i;
    }

    m_year=t.year_s;
    m_mon=t.mon;
    m_mday=t.mday;
    m_hour=t.hour;

    return 0;
}

uint8_t DS3231_DST::set(const char *buf)
{
	struct ts t;
	strtots(buf,&t);
	return set(t);
}

uint8_t DS3231_DST::get(struct ts *t)
{
    uint8_t i,error;

    error=DS3231::get(t);

    if ((error) &&
    	(error!=0xFF))
    	return error;

    i=0;
    switch (m_checklevel)
    {
    case CheckLevel_Hour:
    	if (t->hour!=m_hour)
    	{
    		i=1;
    		break;
    	}
    case CheckLevel_Day:
    	if (t->wday!=m_mday)
    	{
    		i=1;
    		break;
    	}
    case CheckLevel_Mon:
    	if ((t->mon!=m_mon) ||
    		(t->year_s!=m_year))
    		i=1;
    }
    if (i)
    {
    	m_year=t->year_s;
    	m_mon=t->mon;
    	m_mday=t->mday;
    	m_hour=t->hour;
    	dst=check_dst(m_year,m_mon,m_mday,m_hour);
    }
    if (dst)
    	fix_summer_time(t);
   	return error;
}

void DS3231_DST::fix_summer_time(struct ts *t)
{
    if (++t->hour<24)
    	return;
    t->hour=0;
    t->mday++;
    t->wday++;
    if (t->wday>7)
    	t->wday=1;
    if (t->mday<=mon_last_day(t->year_s,t->mon))
    	return;
    t->mday=1;
    if (++t->mon<13)
    	return;
    t->mon=1;
    t->year_s++;
    t->year++;
}

uint8_t DS3231_DST::last_sun_of_month31(uint8_t y, uint8_t m)
{
	uint8_t d = 31;
	uint8_t f;
	f=dow(y,m,31);
	while (f!=7)
	{
		d--;
		if (!(--f))
			break;
	}
	return d;
}

uint8_t DS3231_DST::mon_last_day(uint8_t y, uint8_t m)
{
	if (m!=2)
		return 31 - (m - 1) % 7 % 2;
	return ((y & 3)?28:29);
}

uint8_t DS3231_DST::check_dst(uint8_t year, uint8_t mon, uint8_t day, uint8_t hour)
{
	uint8_t temp;
	uint8_t res;
	if ((mon<3) ||
		(mon>10))
	{
		m_checklevel=CheckLevel_Mon;
		return 0;
	}
	if ((mon>3) &&
		(mon<10))
	{
		m_checklevel=CheckLevel_Mon;
		return 1;
	}
	res=(mon==3)?0:1;
	temp=last_sun_of_month31(year,mon);
	if (day<temp)
	{
		m_checklevel=CheckLevel_Day;
		return res;
	}
	if ((day>temp) ||
		(hour>=2))
	{
		m_checklevel=CheckLevel_Mon;
		return res^1;
	}
	m_checklevel=CheckLevel_Hour;
	return res;
}

uint8_t DS3231_DST::m_year=0;
uint8_t DS3231_DST::m_mon=0;
uint8_t DS3231_DST::m_mday=0;
uint8_t DS3231_DST::m_hour=0;
uint8_t DS3231_DST::m_checklevel=0;
uint8_t DS3231_DST::dst=0;
