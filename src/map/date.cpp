// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "date.hpp"

#include <ctime>

/*
 * Get the current year
 */
int date_get_year(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return lt->tm_year+1900;
}

/*
 * Get the current month
 */
enum e_month date_get_month(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return (enum e_month)(lt->tm_mon+1);
}

/*
 * Get the day of the month
 */
int date_get_dayofmonth(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return lt->tm_mday;
}

/*
 * Get the day of the week
 */
enum e_dayofweek date_get_dayofweek(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return (enum e_dayofweek)lt->tm_wday;
}

/*
 * Get the day of the year
 */
int date_get_dayofyear(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return lt->tm_yday;
}

/*
 * Get the current hours
 */
int date_get_hour(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return lt->tm_hour;
}

/*
 * Get the current minutes
 */
int date_get_min(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return lt->tm_min;
}

/*
 * Get the current seconds
 */
int date_get_sec(void)
{
	time_t t;
	struct tm * lt;
	t = time(nullptr);
	lt = localtime(&t);
	return lt->tm_sec;
}

/*
 * Get the value for the specific type
 */
int date_get( enum e_date_type type )
{
	switch( type ){
		case DT_SECOND:
			return date_get_sec();
		case DT_MINUTE:
			return date_get_min();
		case DT_HOUR:
			return date_get_hour();
		case DT_DAYOFWEEK:
			return date_get_dayofweek();
		case DT_DAYOFMONTH:
			return date_get_dayofmonth();
		case DT_MONTH:
			return date_get_month();
		case DT_YEAR:
			return date_get_year();
		case DT_DAYOFYEAR:
			return date_get_dayofyear();
		case DT_YYYYMMDD:
			return date_get( DT_YEAR ) * 10000 + date_get( DT_MONTH ) * 100 + date_get(DT_DAYOFMONTH);
		default:
			return -1;
	}
}

/*
 * Is today a day of the Sun for Star Gladiators?
 */
bool is_day_of_sun(void)
{
	return (date_get_dayofyear()+1)%2 == 0;
}

/*
 * Is today a day of the Moon for Star Gladiators?
 */
bool is_day_of_moon(void)
{
	return (date_get_dayofyear()+1)%2 == 1;
}

/*
 * Is today a day of the Star for Star Gladiators?
 */
bool is_day_of_star(void)
{
	return (date_get_dayofyear()+1)%5 == 0;
}
