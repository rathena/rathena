// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAPREG_HPP
#define MAPREG_HPP

#include <common/cbasetypes.hpp>
#include <common/db.hpp>
#include <string>

extern bool skip_insert;

void mapreg_reload(void);
void mapreg_final(void);
void mapreg_init(void);
bool mapreg_config_read(const char* w1, const char* w2);

int64 mapreg_readreg(int64 uid);
std::string mapreg_readregstr(int64 uid);
bool mapreg_setreg(int64 uid, int64 val);
bool mapreg_setregstr(int64 uid, std::string str);

#endif /* MAPREG_HPP */
