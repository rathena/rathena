/*
 * File:   char_cnslif.h
 * Author: lighta
 *
 * Created on June 15, 2013, 12:07 PM
 */

#ifndef CHAR_CNSLIF_H
#define	CHAR_CNSLIF_H

#ifdef	__cplusplus
extern "C" {
#endif

void display_helpscreen(bool do_exit);
int cnslif_parse(const char* buf);
void do_init_chcnslif(void);


#ifdef	__cplusplus
}
#endif

#endif	/* CHAR_CNSLIF_H */

