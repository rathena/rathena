// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _DATE_H_
#define _DATE_H_

#ifdef	__cplusplus
extern "C" {
#endif
int date_get_year(void);
int date_get_month(void);
int date_get_day(void);
int date_get_hour(void);
int date_get_min(void);
int date_get_sec(void);

int is_day_of_sun(void);
int is_day_of_moon(void);
int is_day_of_star(void);

#ifdef	__cplusplus
}
#endif
#endif /* _DATE_H_ */
