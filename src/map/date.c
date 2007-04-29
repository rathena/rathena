// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "date.h"
#include <time.h>

int date_get_year(void)
{
	time_t t;
	struct tm * lt;
	t = time(NULL);
	lt = localtime(&t);
	return lt->tm_year+1900;
}
int date_get_month(void)
{
	time_t t;
	struct tm * lt;
	t = time(NULL);
	lt = localtime(&t);
	return lt->tm_mon+1;
}
int date_get_day(void)
{
	time_t t;
	struct tm * lt;
	t = time(NULL);
	lt = localtime(&t);
	return lt->tm_mday;
}
int date_get_hour(void)
{
	time_t t;
	struct tm * lt;
	t = time(NULL);
	lt = localtime(&t);
	return lt->tm_hour;
}

int date_get_min(void)
{
	time_t t;
	struct tm * lt;
	t = time(NULL);
	lt = localtime(&t);
	return lt->tm_min;
}

int date_get_sec(void)
{
	time_t t;
	struct tm * lt;
	t = time(NULL);
	lt = localtime(&t);
	return lt->tm_sec;
}

int is_day_of_sun(void)
{
	return date_get_day()%2 == 0;
}

int is_day_of_moon(void)
{
	return date_get_day()%2 == 1;
}

int is_day_of_star(void)
{
	return date_get_day()%5 == 0;
}
