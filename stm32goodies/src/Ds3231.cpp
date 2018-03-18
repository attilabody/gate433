
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

#include <sg/Ds3231.h>
#include <sg/Strutil.h>
#include <stdio.h>

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
using namespace sg;

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231::Ts::YMDToString(char *buffer, uint8_t yearDigits, uint8_t size) const
{
	uint8_t	count = 0;
	if(size && size < 7 + yearDigits) return 0;
	buffer += count = todec(buffer, year, yearDigits);
	*buffer++ = '.';
	++count;
	return count + MDToString(buffer);
}

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231::Ts::MDToString(char *buffer, uint8_t size) const
{
	uint8_t	count = 0, tmp;
	if(size && size < 6) return 0;
	buffer += count = todec(buffer, mon, 2);
	*buffer++ = '.';
	++count;
	tmp = todec(buffer, mday, 2);
	buffer += tmp;
	count += tmp;
	*buffer = 0;
	return count;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231::Ts::TimeToString(char *buffer, uint8_t size, bool seconds) const
{
	uint8_t	count = 0, tmp;
	if(size && size < (seconds ? 9: 6)) return 0;
	buffer += count = todec(buffer, hour, 2);
	*buffer++ = ':';
	++count;
	tmp = todec(buffer, min, 2);
	buffer += tmp;
	count += tmp;
	if(seconds) {
		*buffer++ = ':';
		++count;
		tmp = todec(buffer, sec, 2);
		buffer += tmp;
		count += tmp;
	}
	*buffer = 0;
	return count;
}

//////////////////////////////////////////////////////////////////////////////
DS3231::DS3231(I2cMaster &i2c, I2cMaster::Mode mode)
	: m_i2c(i2c), m_mode(mode)
{}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::Init(const uint8_t ctrl_reg)
{
    return SetCreg(ctrl_reg);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::Set(Ts t)
{
    uint8_t i, *_t((uint8_t*)&t);
    HAL_StatusTypeDef	st;

    t.year_s = t.year - 2000;
	t.wday=Dow(t.year_s,t.mon,t.mday);

    for (i = 0; i <= 6; i++, _t++) {
    	*_t = (i == 5 ? 0x80 : 0) | DecToBcd(*_t);
	}

    if((st = m_i2c.WriteMem(DS3231_I2C_ADDR, DS3231_TIME_CAL_ADDR, 1, &t, 7, m_mode)) != HAL_OK)
    	return st;

    st = SetSreg(0x0F);	// OSF clear
   	return st;
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::Set(const char *buf)
{
	Ts t;
	StrToTs(buf,t);
	return Set(t);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::Get(Ts &t, bool &desync)
{
    uint8_t i, *_t((uint8_t*)&t);
    HAL_StatusTypeDef ret;
    uint8_t	buffer[7];

    if((ret = GetSreg(i)) != HAL_OK)
    	return ret;

    desync = (i & DS3231_OSF) != 0;

    if((ret = m_i2c.ReadMem(DS3231_I2C_ADDR, DS3231_TIME_CAL_ADDR, 1, buffer, 7)) != HAL_OK)
    	return ret;

    m_i2c.WaitCallback();

    for (i = 0; i <= 6; i++) {
        *(_t++) = BcdToDec(buffer[i] & (i == 5 ? 0x1f : 0xff));
    }

    t.year = t.year_s + 2000;
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::SetAddr(const uint8_t addr, const uint8_t val)
{
    return m_i2c.WriteMem(DS3231_I2C_ADDR,addr,1, &val, 1, m_mode);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::GetAddr(const uint8_t addr, uint8_t &val)
{
	HAL_StatusTypeDef ret;
    ret =  m_i2c.ReadMem(DS3231_I2C_ADDR, addr, 1, &val, 1);
    m_i2c.WaitCallback();
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
// control register
//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::SetCreg(const uint8_t val)
{
    return SetAddr(DS3231_CONTROL_ADDR, val);
}

// status register 0Fh/8Fh

/*
bit7 OSF      Oscillator Stop Flag (if 1 then oscillator has stopped and date might be innacurate)
bit3 EN32kHz  Enable 32kHz output (1 if needed)
bit2 BSY      Busy with TCXO functions
bit1 A2F      Alarm 2 Flag - (1 if alarm2 was triggered)
bit0 A1F      Alarm 1 Flag - (1 if alarm1 was triggered)
*/

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::SetSreg(const uint8_t val)
{
    return SetAddr(DS3231_STATUS_ADDR, val);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::GetSreg(uint8_t &val)
{
	return GetAddr(DS3231_STATUS_ADDR, val);
}

// aging register

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::SetAging(const int8_t val)
{
    uint8_t reg;

    if (val >= 0)
        reg = val;
    else
        reg = ~(-val) + 1;      // 2C

    return SetAddr(DS3231_AGING_OFFSET_ADDR, reg);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::GetAging(int8_t &val)
{
    uint8_t reg;
    HAL_StatusTypeDef ret;

    if((ret = GetAddr(DS3231_AGING_OFFSET_ADDR, reg)) != HAL_OK)
    	return ret;

    if ((reg & 0x80) != 0)
        val = reg | ~((1 << 8) - 1);     // if negative get two's complement
    else
        val = reg;

    return ret;
}
//////////////////////////////////////////////////////////////////////////////
// temperature register
//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::GetTreg(int16_t &temp)
{
    HAL_StatusTypeDef rv;
    uint8_t	buf[2];

    if((rv = m_i2c.ReadMem(DS3231_I2C_ADDR, DS3231_TEMPERATURE_ADDR, 1, buf, 2)) != HAL_OK)
    	return rv;
    m_i2c.WaitCallback();

    temp = *reinterpret_cast<int8_t*>(buf);
    temp <<= 2;
    temp |= buf[1] >> 6;

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// alarms
// flags are: A1M1 (seconds), A1M2 (minutes), A1M3 (hour), 
// A1M4 (day) 0 to enable, 1 to disable, DY/DT (dayofweek == 1/dayofmonth == 0)
//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::SetA1(const uint8_t s, const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t * flags)
{
    uint8_t t[4] = { s, mi, h, d };
    uint8_t i;

    for (i = 0; i <= 3; i++) {
        if (i == 3) {
            t[i]=DecToBcd(t[3]) | (flags[3] << 7) | (flags[4] << 6);
        } else
            t[i]=DecToBcd(t[i]) | (flags[i] << 7);
    }

    return m_i2c.WriteMem(DS3231_I2C_ADDR, DS3231_ALARM1_ADDR, 1, t, 4);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::GetA1(char *buf, const uint8_t len)
{
    uint8_t n[4];
    uint8_t t[4];               //second,minute,hour,day
    uint8_t f[5];               // flags
    uint8_t i;

    HAL_StatusTypeDef ret;

    if((ret = m_i2c.ReadMem(DS3231_I2C_ADDR, DS3231_ALARM1_ADDR, 1, n, 4)) == HAL_OK)
    {
		for (i = 0; i <= 3; i++) {
			f[i] = (n[i] & 0x80) >> 7;
			t[i] = BcdToDec(n[i] & 0x7F);
		}

		f[4] = (n[3] & 0x40) >> 6;
		t[3] = BcdToDec(n[3] & 0x3F);

		//Hat a faszom nem kene?!
		snprintf(buf, len,
				 "s%02d m%02d h%02d d%02d fs%d m%d h%d d%d wm%d %d %d %d %d",
				 t[0], t[1], t[2], t[3], f[0], f[1], f[2], f[3], f[4], n[0],
				 n[1], n[2], n[3]);
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
// when the alarm flag is cleared the pulldown on INT is also released
HAL_StatusTypeDef DS3231::ClearA1f(void)
{
    uint8_t reg_val;
    HAL_StatusTypeDef ret;

    if(( ret = GetSreg(reg_val)) != HAL_OK)
    	return ret;
    return SetSreg(reg_val & ~DS3231_A1F);

}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::TriggeredA1(bool &b)
{
    uint8_t reg_val;
    HAL_StatusTypeDef ret;

    if((ret = GetSreg(reg_val)) == HAL_OK)
    	b = (reg_val & DS3231_A1F) != 0;

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
/* flag bits are:
 * 0:	DY/DT (dayofweek == 1/dayofmonth == 0)
 * 1:	A2M4 (day) 0 to enable
 * 2:	A2M3 (hour) 0 to enable
 * 3:	A2M2 (minutes) 0 to enable
 */
//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::SetA2(const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t flags)
{
    uint8_t t[3] = { mi, h, d };
    uint8_t i;

    for (i = 0; i <= 2; i++) {
        if (i == 2) {
            t[i]=DecToBcd(t[2]) | ((flags << 6) & 0xC0);
        } else
            t[i]=DecToBcd(t[i]) | ((flags << (4-i)) & 0x80);
    }

    return m_i2c.WriteMem(DS3231_I2C_ADDR, DS3231_ALARM2_ADDR, 1, t,3);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::GetA2(char *buf, const uint8_t len)
{
    uint8_t n[3];
    uint8_t t[3];               //second,minute,hour,day
    uint8_t f[4];               // flags
    uint8_t i;
    HAL_StatusTypeDef ret;

    if((ret = m_i2c.ReadMem(DS3231_I2C_ADDR, DS3231_ALARM2_ADDR, 1, n, 3)) != HAL_OK)
    	return ret;

    for (i = 0; i <= 2; i++) {
        f[i] = (n[i] & 0x80) >> 7;
        t[i] = BcdToDec(n[i] & 0x7F);
    }

    f[3] = (n[2] & 0x40) >> 6;
    t[2] = BcdToDec(n[2] & 0x3F);

    //Nebassza
    snprintf(buf, len, "m%02d h%02d d%02d fm%d h%d d%d wm%d %d %d %d", t[0],
             t[1], t[2], f[0], f[1], f[2], f[3], n[0], n[1], n[2]);

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
// when the alarm flag is cleared the pulldown on INT is also released
HAL_StatusTypeDef DS3231::ClearA2f(void)
{
    uint8_t reg_val;
    HAL_StatusTypeDef	ret;

    if((ret = GetSreg(reg_val)) != HAL_OK)
        	return ret;
    return SetSreg(reg_val & ~DS3231_A2F);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231::TriggeredA2(bool &b)
{
    uint8_t reg_val;
    HAL_StatusTypeDef ret;

    if((ret = GetSreg(reg_val)) == HAL_OK)
    	b =  (reg_val & DS3231_A2F) != 0;

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
// helpers
//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231::DecToBcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231::BcdToDec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231::Inp2Toi(const char * &c)
{
    uint8_t rv;
    rv = (*c++ - 48) * 10;
    rv += *c++ - 48;
    return rv;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231::Dow(uint8_t y, uint8_t m, uint8_t d)
{
	static uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	y -= m < 3;
	return ((y + (y >> 2) + t[m-1] + d + 6) % 7) +1;
}

//////////////////////////////////////////////////////////////////////////////
void DS3231::StrToTs(const char *buf, Ts &t)
{
	buf+=2;
	t.year=2000+Inp2Toi(buf);
	t.mon=Inp2Toi(buf);
	t.mday=Inp2Toi(buf);
	t.hour=Inp2Toi(buf);
	t.min=Inp2Toi(buf);
	t.sec=0;
	t.wday = Dow(t.year-2000,t.mon,t.mday);
}



//////////////////////////////////////////////////////////////////////////////
// ----------------------------- DS3231_DST -------------------------------
//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231_DST::Set(Ts t)
{
    HAL_StatusTypeDef ret;

    t.year_s = t.year - 2000;

    m_dst=CheckDst(t.year_s,t.mon,t.mday,t.hour);
    if (m_dst) {									// Just hour correction!
    	if(!t.hour) return HAL_ERROR;
    	t.hour--;
    }

    if((ret = DS3231::Set(t)) != HAL_OK)
    {
    	m_year=0;
    	return ret;
    }

    m_year=t.year_s;
    m_mon=t.mon;
    m_mday=t.mday;
    m_hour=t.hour;

    return ret;
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231_DST::Set(const char *buf)
{
	Ts t;
	StrToTs(buf,t);
	return Set(t);
}

//////////////////////////////////////////////////////////////////////////////
HAL_StatusTypeDef DS3231_DST::Get(Ts &t, bool &desync)
{
    bool check = false;
    HAL_StatusTypeDef error;

    if((error = DS3231::Get(t, desync)) != HAL_OK)
    	return error;

    switch (m_checklevel)
    {
    case CheckLevel_Hour:
		if (t.hour != m_hour)
			check = true;
		break;

    case CheckLevel_Day:
    	if(t.wday != m_mday)
			check = true;
    	break;

    case CheckLevel_Mon:
    	if ((t.mon!=m_mon) ||
    		(t.year_s!=m_year))
    		check = true;
    }

    if (check)
    {
    	m_year=t.year_s;
    	m_mon=t.mon;
    	m_mday=t.mday;
    	m_hour=t.hour;
    	m_dst=CheckDst(m_year,m_mon,m_mday,m_hour);
    }
    if (m_dst)
    	FixSummerTime(t);

    return error;
}

//////////////////////////////////////////////////////////////////////////////
void DS3231_DST::FixSummerTime(Ts &t)
{
    if (++t.hour<24)
    	return;
    t.hour=0;
    t.mday++;
    t.wday++;
    if (t.wday>7)
    	t.wday=1;
    if (t.mday<=MonLastDay(t.year_s,t.mon))
    	return;
    t.mday=1;
    if (++t.mon<13)
    	return;
    t.mon=1;
    t.year_s++;
    t.year++;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231_DST::LastSunOfMonth31(uint8_t y, uint8_t m)
{
	uint8_t d = 31;
	uint8_t f;
	f=Dow(y,m,31);
	while (f!=7)
	{
		d--;
		if (!(--f))
			break;
	}
	return d;
}

//////////////////////////////////////////////////////////////////////////////
uint8_t DS3231_DST::MonLastDay(uint8_t y, uint8_t m)
{
	if (m!=2)
		return 31 - (m - 1) % 7 % 2;
	return ((y & 3)?28:29);
}

//////////////////////////////////////////////////////////////////////////////
bool DS3231_DST::CheckDst(uint8_t year, uint8_t mon, uint8_t day, uint8_t hour)
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
	temp=LastSunOfMonth31(year,mon);
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

