// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAPREG_HPP_
#define _MAPREG_HPP_

#include "../common/cbasetypes.hpp"
#include "../common/db.hpp"

struct mapreg_save {
	int64 uid;         ///< Unique ID
	union {
		int i;         ///< Numeric value
		char *str;     ///< String value
	} u;
	bool is_string;    ///< true if it's a string, false if it's a number
	bool save;         ///< Whether a save operation is pending
};

extern struct reg_db regs;
extern bool skip_insert;

void mapreg_reload(void);
void mapreg_final(void);
void mapreg_init(void);
bool mapreg_config_read(const char* w1, const char* w2);

int mapreg_readreg(int64 uid);
char* mapreg_readregstr(int64 uid);
bool mapreg_setreg(int64 uid, int val);
bool mapreg_setregstr(int64 uid, const char* str);
int mapreg_destroyreg(DBKey key, DBData *data, va_list ap);

#endif /* _MAPREG_HPP_ */
