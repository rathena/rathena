// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_GUILD_HPP
#define INT_GUILD_HPP

#include "../common/cbasetypes.hpp"

enum e_guild_action : uint32 {
	GS_BASIC = 0x0001,
	GS_MEMBER = 0x0002,
	GS_POSITION = 0x0004,
	GS_ALLIANCE = 0x0008,
	GS_EXPULSION = 0x0010,
	GS_SKILL = 0x0020,
	GS_EMBLEM = 0x0040,
	GS_CONNECT = 0x0080,
	GS_LEVEL = 0x0100,
	GS_MES = 0x0200,
	GS_MASK = 0x03FF,
	GS_BASIC_MASK = (GS_BASIC | GS_EMBLEM | GS_CONNECT | GS_LEVEL | GS_MES),
	GS_REMOVE = 0x8000,
};

struct guild;
struct guild_castle;

int inter_guild_parse_frommap(int fd);
int inter_guild_sql_init(void);
void inter_guild_sql_final(void);
int inter_guild_leave(int guild_id,uint32 account_id,uint32 char_id);
int mapif_parse_BreakGuild(int fd,int guild_id);
int inter_guild_broken(int guild_id);
int inter_guild_sex_changed(int guild_id,uint32 account_id,uint32 char_id, short gender);
int inter_guild_charname_changed(int guild_id,uint32 account_id, uint32 char_id, char *name);
int inter_guild_CharOnline(uint32 char_id, int guild_id);
int inter_guild_CharOffline(uint32 char_id, int guild_id);
uint16 inter_guild_storagemax(int guild_id);

#endif /* INT_GUILD_HPP */
