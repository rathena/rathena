// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#ifndef CHAR_CNSLIF_H
#define	CHAR_CNSLIF_H


int cnslif_parse(const char* buf);
void do_init_chcnslif(void);
#ifdef	__cplusplus
extern "C" {
#endif
void display_helpscreen(bool do_exit);
#ifdef	__cplusplus
}
#endif

#endif	/* CHAR_CNSLIF_H */

