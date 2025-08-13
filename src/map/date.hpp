// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef DATE_HPP
#define DATE_HPP

#include <common/cbasetypes.hpp>

enum e_month{
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
};

enum e_dayofweek{
	SUNDAY = 0,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
};

enum e_date_type{
	DT_MIN = 0,
	DT_SECOND,
	DT_MINUTE,
	DT_HOUR,
	DT_DAYOFWEEK,
	DT_DAYOFMONTH,
	DT_MONTH,
	DT_YEAR,
	DT_DAYOFYEAR,
	DT_YYYYMMDD,
	DT_MAX
};

int32 date_get_year(void);
enum e_month date_get_month(void);
int32 date_get_dayofmonth(void);
enum e_dayofweek date_get_dayofweek(void);
int32 date_get_dayofyear(void);
int32 date_get_day(void);
int32 date_get_hour(void);
int32 date_get_min(void);
int32 date_get_sec(void);

int32 date_get( enum e_date_type type );

bool is_day_of_sun(void);
bool is_day_of_moon(void);
bool is_day_of_star(void);

#endif /* DATE_HPP */
