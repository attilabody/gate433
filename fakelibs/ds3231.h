#ifndef	__ds3231_h_included__
#define	__ds3231_h_included__
struct ts {
    uint8_t sec;         /* seconds */
    uint8_t min;         /* minutes */
    uint8_t hour;        /* hours */
    uint8_t mday;        /* day of the month */
    uint8_t mon;         /* month */
    int16_t year;        /* year */
    uint8_t wday;        /* day of the week */
    uint8_t yday;        /* day in the year */
    uint8_t isdst;       /* daylight saving time */
    uint8_t year_s;      /* year in short notation*/
#ifdef CONFIG_UNIXTIME
    uint32_t unixtime;   /* seconds since 01.01.1970 00:00:00 UTC*/
#endif
};

#endif	//	__ds3231_h_included__
