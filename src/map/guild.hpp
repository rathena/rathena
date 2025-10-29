// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef GUILD_HPP
#define GUILD_HPP

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/mmo.hpp>

#include "map.hpp" // NAME_LENGTH

struct mmo_guild;
struct guild_member;
struct guild_position;
struct guild_castle;
class map_session_data;
struct mob_data;

//For quick linking to a guardian's info. [Skotlex]
struct guardian_data {
	int32 number; //0-MAX_GUARDIANS-1 = Guardians. MAX_GUARDIANS = Emperium.
	int32 guild_id;
	int32 emblem_id;
	int32 guardup_lv; //Level of GD_GUARDUP skill.
	char guild_name[NAME_LENGTH];
	std::shared_ptr<guild_castle> castle;
};

class MapGuild {
public:
	struct mmo_guild guild;
	struct Channel *channel;
	int32 instance_id;
	int32 chargeshout_flag_id;
};

uint16 guild_skill_get_max(uint16 id);

int32 guild_checkskill(const struct mmo_guild &g,int32 id);
bool guild_check_skill_require(const struct mmo_guild &g,uint16 id); // [Komurka]
int32 guild_checkcastles(const struct mmo_guild &g); // [MouseJstr]
bool guild_isallied(int32 guild_id, int32 guild_id2); //Checks alliance based on guild Ids. [Skotlex]
bool guild_has_permission( map_session_data& sd, enum e_guild_permission permission );

void do_init_guild(void);
std::shared_ptr<MapGuild> guild_search(int32 guild_id);
std::shared_ptr<MapGuild> guild_searchname(const char *str);
std::shared_ptr<MapGuild> guild_searchnameid(const char *str);

map_session_data* guild_getavailablesd(const struct mmo_guild &g);
int32 guild_getindex(const struct mmo_guild &g, uint32 account_id, uint32 char_id);
int32 guild_getposition(const map_session_data &sd);
t_exp guild_payexp(map_session_data *sd,t_exp exp);
t_exp guild_getexp(map_session_data *sd,t_exp exp); // [Celest]

bool guild_create( map_session_data& sd, const char* name );
int32 guild_created(uint32 account_id,int32 guild_id);
int32 guild_request_info(int32 guild_id);
int32 guild_recv_noinfo(int32 guild_id);
int32 guild_recv_info(const struct mmo_guild &sg);
int32 guild_npc_request_info(int32 guild_id,const char *ev);
bool guild_invite( map_session_data& sd, map_session_data* tsd );
bool guild_reply_invite( map_session_data& sd, int32 guild_id, int32 flag );
void guild_member_joined(map_session_data *sd);
int32 guild_member_added(int32 guild_id,uint32 account_id,uint32 char_id,int32 flag);
bool guild_leave( map_session_data& sd, int32 guild_id, uint32 account_id, uint32 char_id, const char *mes );
int32 guild_member_withdraw(int32 guild_id,uint32 account_id,uint32 char_id,int32 flag,
	const char *name,const char *mes);
bool guild_expulsion( map_session_data& sd, int32 guild_id, uint32 account_id, uint32 char_id, const char *mes );
void guild_skillup(map_session_data* sd, uint16 skill_id);
void guild_block_skill(map_session_data *sd, int32 time);
int32 guild_reqalliance(map_session_data *sd,map_session_data *tsd);
int32 guild_reply_reqalliance(map_session_data *sd,uint32 account_id,int32 flag);
int32 guild_allianceack(int32 guild_id1,int32 guild_id2,uint32 account_id1,uint32 account_id2,
	int32 flag,const char *name1,const char *name2);
int32 guild_delalliance(map_session_data *sd,int32 guild_id,int32 flag);
int32 guild_opposition(map_session_data *sd,map_session_data *tsd);
int32 guild_check_alliance(int32 guild_id1, int32 guild_id2, int32 flag);

int32 guild_send_memberinfoshort(map_session_data *sd,int32 online);
int32 guild_recv_memberinfoshort(int32 guild_id,uint32 account_id,uint32 char_id,int32 online,int32 lv,int32 class_);
int32 guild_change_memberposition(int32 guild_id,uint32 account_id,uint32 char_id,int16 idx);
int32 guild_memberposition_changed(struct mmo_guild &g,int32 idx,int32 pos);
int32 guild_change_position(int32 guild_id,int32 idx,int32 mode,int32 exp_mode,const char *name);
int32 guild_position_changed(int32 guild_id,int32 idx,struct guild_position *p);
int32 guild_change_notice(map_session_data *sd,int32 guild_id,const char *mes1,const char *mes2);
int32 guild_notice_changed(int32 guild_id,const char *mes1,const char *mes2);
int32 guild_change_emblem( map_session_data& sd, int32 len, const char* data );
int32 guild_change_emblem_version( map_session_data& sd, int32 version );
int32 guild_emblem_changed(int32 len,int32 guild_id,int32 emblem_id,const char *data);
int32 guild_send_message(map_session_data *sd, const char *mes, size_t len);
int32 guild_recv_message( int32 guild_id, uint32 account_id, const char *mes, size_t len );
int32 guild_send_dot_remove(map_session_data *sd);
int32 guild_skillupack(int32 guild_id,uint16 skill_id,uint32 account_id);
int32 guild_break( map_session_data& sd, const char* name );
int32 guild_broken(int32 guild_id,int32 flag);
bool guild_gm_change(int32 guild_id, uint32 char_id, bool showMessage = false );
int32 guild_gm_changed(int32 guild_id, uint32 account_id, uint32 char_id, time_t time);

void guild_castle_map_init(void);
int32 guild_castledatasave(int32 castle_id,int32 index,int32 value);
int32 guild_castledataloadack(int32 len, struct guild_castle *gc);
void guild_castle_reconnect(int32 castle_id, int32 index, int32 value);

bool guild_agit_start(void);
bool guild_agit_end(void);

bool guild_agit2_start(void);
bool guild_agit2_end(void);

bool guild_agit3_start(void);
bool guild_agit3_end(void);

/* guild flag cachin */
void guild_flag_add(npc_data *nd);
void guild_flag_remove(npc_data *nd);
void guild_flags_clear(void);

void guild_guildaura_refresh(map_session_data *sd, uint16 skill_id, uint16 skill_lv);
#ifdef BOUND_ITEMS
void guild_retrieveitembound(uint32 char_id,uint32 account_id,int32 guild_id);
#endif

void do_final_guild(void);

class CastleDatabase : public TypesafeYamlDatabase <int32, guild_castle> {
public:
	CastleDatabase() : TypesafeYamlDatabase("CASTLE_DB", 2, 1) {

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
	void loadingFinished() override;

	// Additional
	std::shared_ptr<guild_castle> mapname2gc(const char* mapname);
	std::shared_ptr<guild_castle> mapindex2gc(int16 mapindex);
	std::shared_ptr<guild_castle> find_by_clientid( uint16 client_id );
};

extern CastleDatabase castle_db;

#endif /* GUILD_HPP */
