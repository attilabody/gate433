#ifndef __ds3231_h_
#define __ds3231_h_

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include "config.h"

#define SECONDS_FROM_1970_TO_2000 946684800

// i2c slave address of the DS3231 chip
#define DS3231_I2C_ADDR             0x68

// timekeeping registers
#define DS3231_TIME_CAL_ADDR        0x00
#define DS3231_ALARM1_ADDR          0x07
#define DS3231_ALARM2_ADDR          0x0B
#define DS3231_CONTROL_ADDR         0x0E
#define DS3231_STATUS_ADDR          0x0F
#define DS3231_AGING_OFFSET_ADDR    0x10
#define DS3231_TEMPERATURE_ADDR     0x11

// control register bits
#define DS3231_A1IE     0x1
#define DS3231_A2IE     0x2
#define DS3231_INTCN    0x4

// status register bits
#define DS3231_A1F      0x1
#define DS3231_A2F      0x2
#define DS3231_OSF      0x80

// DO NOT MODIFY first 7 members of this struct as they are representing internal
//registers of DS3231. DOING SO SURELY WILL BREAK HIGHLY OPTIMIZED DS3231 CODE
// !!!YOU HAVE BEEN WARNED!!!
struct ts {
    uint8_t sec;         /* seconds */
    uint8_t min;         /* minutes */
    uint8_t hour;        /* hours */
    uint8_t wday;        /* day of the week */
    uint8_t mday;        /* day of the month */
    uint8_t mon;         /* month */
    uint8_t year_s;      /* year in short notation */
    int16_t year;        /* year */
    uint8_t yday;        /* day in the year */
    uint8_t isdst;       /* daylight saving time */
#ifdef CONFIG_UNIXTIME
    uint32_t unixtime;   /* seconds since 01.01.1970 00:00:00 UTC*/
#endif
};

class DS3231
{
  public:
	static uint8_t init(const uint8_t creg);
	static uint8_t set(struct ts t);
	static uint8_t set(const char *buf);
	static uint8_t get(struct ts *t);

	// temperature register
	static float get_treg(void);

	// alarms
	static void set_a1(const uint8_t s, const uint8_t mi, const uint8_t h, const uint8_t d,
	                   const uint8_t * flags);
	static void get_a1(char *buf, const uint8_t len);
	static void clear_a1f(void);
	static uint8_t triggered_a1(void);

	static void set_a2(const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t flags);
	static void get_a2(char *buf, const uint8_t len);
	static void clear_a2f(void);
	static uint8_t triggered_a2(void);

	// helpers
	static uint32_t get_unixtime(struct ts t);

  protected:
	static uint8_t set_addr(const uint8_t addr, const uint8_t val);
	static uint8_t get_addr(const uint8_t addr, uint8_t * val);

	// control/status register
	static uint8_t set_creg(const uint8_t val);
	static uint8_t set_sreg(const uint8_t val);
	static uint8_t get_sreg(uint8_t * val);

	// aging offset register
	static uint8_t set_aging(const int8_t val);
	static uint8_t get_aging(int8_t * val);

	static uint8_t dectobcd(const uint8_t val);
	static uint8_t bcdtodec(const uint8_t val);
	static uint8_t inp2toi(const char * &c);

	static uint8_t dow(uint8_t y, uint8_t m, uint8_t d);
	static void strtots(const char *buf, struct ts *t);
};

class DS3231_DST: public DS3231
{
  public:
	static uint8_t set(struct ts t);
	static uint8_t set(const char *buf);
	static uint8_t get(struct ts *t);

	static void fix_summer_time(struct ts *t);

	// helpers
	static uint8_t last_sun_of_month31(uint8_t y, uint8_t m);
	static uint8_t mon_last_day(uint8_t y, uint8_t m);
	static uint8_t check_dst(uint8_t y, uint8_t m, uint8_t d, uint8_t h);
	static uint8_t m_checklevel;
  private:

	static uint8_t m_year;
	static uint8_t m_mon;
	static uint8_t m_mday;
	static uint8_t m_hour;
	static uint8_t dst;
};

#endif
