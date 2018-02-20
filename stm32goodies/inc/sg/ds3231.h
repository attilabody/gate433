#ifndef __ds3231_h_
#define __ds3231_h_

#include <inttypes.h>
#include <sg/i2cmaster.h>

#define SECONDS_FROM_1970_TO_2000 946684800

// i2c slave address of the DS3231 chip
#define DS3231_I2C_ADDR             0xD0

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

namespace sg {


class DS3231
{
  public:
	DS3231(I2cMaster &i2c, I2cMaster::Mode mode = I2cMaster::Poll);

	// DO NOT MODIFY first 7 members of this struct as they are representing internal
	// registers of DS3231. DOING SO SURELY WILL BREAK HIGHLY OPTIMIZED DS3231 CODE
	// !!!YOU HAVE BEEN WARNED!!!
	struct Ts {
	    uint8_t sec;         /* seconds */
	    uint8_t min;         /* minutes */
	    uint8_t hour;        /* hours */
	    uint8_t wday;        /* day of the week */
	    uint8_t mday;        /* day of the month */
	    uint8_t mon;         /* month */
	    uint8_t year_s;      /* year in short notation */
	    int16_t year;        /* year */
	    uint8_t YMDToString(char *buffer, uint8_t yearDigits = 4, uint8_t size = 0) const;
	    uint8_t MDToString(char *buffer, uint8_t size = 0) const;
	    uint8_t TimeToString(char *buffer, uint8_t size = 0, bool seconds = false) const;
	};

	HAL_StatusTypeDef Init(const uint8_t creg = DS3231_INTCN);
	HAL_StatusTypeDef Set(struct Ts t);
	HAL_StatusTypeDef Set(const char *buf);
	HAL_StatusTypeDef Get(struct Ts &t, bool &desync);

	// temperature register
	HAL_StatusTypeDef GetTreg(float &temp);

	// alarms
	HAL_StatusTypeDef SetA1(const uint8_t s, const uint8_t mi, const uint8_t h, const uint8_t d,
			const uint8_t * flags);
	HAL_StatusTypeDef GetA1(char *buf, const uint8_t len);
	HAL_StatusTypeDef ClearA1f(void);
	HAL_StatusTypeDef TriggeredA1(bool &b);

	HAL_StatusTypeDef SetA2(const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t flags);
	HAL_StatusTypeDef GetA2(char *buf, const uint8_t len);
	HAL_StatusTypeDef ClearA2f(void);
	HAL_StatusTypeDef TriggeredA2(bool &b);

  protected:
	HAL_StatusTypeDef SetAddr(const uint8_t addr, const uint8_t val);
	HAL_StatusTypeDef GetAddr(const uint8_t addr, uint8_t &val);

	// control/status register
	HAL_StatusTypeDef SetCreg(const uint8_t val);
	HAL_StatusTypeDef SetSreg(const uint8_t val);
	HAL_StatusTypeDef GetSreg(uint8_t &val);

	// aging offset register
	HAL_StatusTypeDef SetAging(const int8_t val);
	HAL_StatusTypeDef GetAging(int8_t &val);

	uint8_t DecToBcd(const uint8_t val);
	uint8_t BcdToDec(const uint8_t val);
	uint8_t Inp2Toi(const char * &c);

	uint8_t Dow(uint8_t y, uint8_t m, uint8_t d);
	void StrToTs(const char *buf, Ts &t);

  private:
	I2cMaster				&m_i2c;
	const I2cMaster::Mode	m_mode;
};

class DS3231_DST: public DS3231
{
public:
	DS3231_DST(sg::I2cMaster &i2c, sg::I2cMaster::Mode mode = I2cMaster::Poll) : DS3231(i2c, mode) {}
	HAL_StatusTypeDef Set(struct Ts t);
	HAL_StatusTypeDef Set(const char *buf);
	HAL_StatusTypeDef Get(struct Ts &t, bool &desync);

	void FixSummerTime(struct Ts &t);

	// helpers
	uint8_t LastSunOfMonth31(uint8_t y, uint8_t m);
	uint8_t MonLastDay(uint8_t y, uint8_t m);
	bool CheckDst(uint8_t y, uint8_t m, uint8_t d, uint8_t h);

	uint8_t m_checklevel = CheckLevel_Mon;
private:

	uint8_t m_year = 0;
	uint8_t m_mon = 0;
	uint8_t m_mday = 0;
	uint8_t m_hour = 0;
	bool	m_dst = false;

	static const uint8_t	CheckLevel_Mon = 2;
	static const uint8_t	CheckLevel_Day = 1;
	static const uint8_t	CheckLevel_Hour = 0;
};

}	//	namespace sg
#endif
