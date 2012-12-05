// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAPREG_H_
#define _MAPREG_H_

void mapreg_reload(void);
void mapreg_final(void);
void mapreg_init(void);
bool mapreg_config_read(const char* w1, const char* w2);

int mapreg_readreg(int uid);
char* mapreg_readregstr(int uid);
bool mapreg_setreg(int uid, int val);
bool mapreg_setregstr(int uid, const char* str);

#endif /* _MAPREG_H_ */
