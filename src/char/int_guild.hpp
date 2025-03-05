// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_GUILD_HPP
#define INT_GUILD_HPP

#include <string>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/mmo.hpp>

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

struct mmo_guild;
struct guild_castle;

struct s_guild_exp_db {
	uint16 level;
	t_exp exp;
};

class GuildExpDatabase : public TypesafeYamlDatabase<uint16, s_guild_exp_db> {
public:
	GuildExpDatabase() : TypesafeYamlDatabase("GUILD_EXP_DB", 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void loadingFinished() override;

	// Additional
	t_exp get_nextexp(uint16 level);
};

class CharGuild {
public:
	struct mmo_guild guild;
	uint16 save_flag;
};

int32 inter_guild_parse_frommap(int32 fd);
void inter_guild_sql_init(void);
void inter_guild_sql_final(void);
int32 inter_guild_leave(int32 guild_id,uint32 account_id,uint32 char_id);
int32 mapif_parse_BreakGuild(int32 fd,int32 guild_id);
int32 inter_guild_broken(int32 guild_id);
int32 inter_guild_sex_changed(int32 guild_id,uint32 account_id,uint32 char_id, int16 gender);
int32 inter_guild_charname_changed(int32 guild_id,uint32 account_id, uint32 char_id, char *name);
int32 inter_guild_CharOnline(uint32 char_id, int32 guild_id);
int32 inter_guild_CharOffline(uint32 char_id, int32 guild_id);
uint16 inter_guild_storagemax(int32 guild_id);

#endif /* INT_GUILD_HPP */
