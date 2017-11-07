// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_CNSLIF_HPP_
#define _CHAR_CNSLIF_HPP_

int cnslif_parse(const char* buf);
void do_init_chcnslif(void);

extern "C" {
void display_helpscreen(bool do_exit);
}

#endif	/* _CHAR_CNSLIF_HPP_ */

