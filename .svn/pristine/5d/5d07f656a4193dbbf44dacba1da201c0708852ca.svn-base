// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GUILD_H_
#define _GUILD_H_

//#include "../common/mmo.h"
struct guild;
struct guild_member;
struct guild_position;
struct guild_castle;
#include "map.h" // NAME_LENGTH
struct map_session_data;
struct mob_data;

//For quick linking to a guardian's info. [Skotlex]
struct guardian_data {
	int number; //0-MAX_GUARDIANS-1 = Guardians. MAX_GUARDIANS = Emperium.
	int guild_id;
	int emblem_id;
	int guardup_lv; //Level of GD_GUARDUP skill.
	char guild_name[NAME_LENGTH];
	struct guild_castle* castle;
};

int guild_skill_get_max(int id);

int guild_checkskill(struct guild *g,int id);
int guild_check_skill_require(struct guild *g,int id); // [Komurka]
int guild_checkcastles(struct guild *g); // [MouseJstr]
bool guild_isallied(int guild_id, int guild_id2); //Checks alliance based on guild Ids. [Skotlex]

void do_init_guild(void);
struct guild *guild_search(int guild_id);
struct guild *guild_searchname(char *str);
struct guild_castle *guild_castle_search(int gcid);

struct guild_castle* guild_mapname2gc(const char* mapname);
struct guild_castle* guild_mapindex2gc(short mapindex);

struct map_session_data *guild_getavailablesd(struct guild *g);
int guild_getindex(struct guild *g,uint32 account_id,uint32 char_id);
int guild_getposition(struct guild *g, struct map_session_data *sd);
unsigned int guild_payexp(struct map_session_data *sd,unsigned int exp);
int guild_getexp(struct map_session_data *sd,int exp); // [Celest]

int guild_create(struct map_session_data *sd, const char *name);
int guild_created(uint32 account_id,int guild_id);
int guild_request_info(int guild_id);
int guild_recv_noinfo(int guild_id);
int guild_recv_info(struct guild *sg);
int guild_npc_request_info(int guild_id,const char *ev);
int guild_invite(struct map_session_data *sd,struct map_session_data *tsd);
int guild_reply_invite(struct map_session_data *sd,int guild_id,int flag);
void guild_member_joined(struct map_session_data *sd);
int guild_member_added(int guild_id,uint32 account_id,uint32 char_id,int flag);
int guild_leave(struct map_session_data *sd,int guild_id,
	uint32 account_id,uint32 char_id,const char *mes);
int guild_member_withdraw(int guild_id,uint32 account_id,uint32 char_id,int flag,
	const char *name,const char *mes);
int guild_expulsion(struct map_session_data *sd,int guild_id,
	uint32 account_id,uint32 char_id,const char *mes);
void guild_skillup(struct map_session_data* sd, uint16 skill_id);
void guild_block_skill(struct map_session_data *sd, int time);
int guild_reqalliance(struct map_session_data *sd,struct map_session_data *tsd);
int guild_reply_reqalliance(struct map_session_data *sd,uint32 account_id,int flag);
int guild_alliance(int guild_id1,int guild_id2,uint32 account_id1,uint32 account_id2);
int guild_allianceack(int guild_id1,int guild_id2,uint32 account_id1,uint32 account_id2,
	int flag,const char *name1,const char *name2);
int guild_delalliance(struct map_session_data *sd,int guild_id,int flag);
int guild_opposition(struct map_session_data *sd,struct map_session_data *tsd);
int guild_check_alliance(int guild_id1, int guild_id2, int flag);

int guild_send_memberinfoshort(struct map_session_data *sd,int online);
int guild_recv_memberinfoshort(int guild_id,uint32 account_id,uint32 char_id,int online,int lv,int class_);
int guild_change_memberposition(int guild_id,uint32 account_id,uint32 char_id,short idx);
int guild_memberposition_changed(struct guild *g,int idx,int pos);
int guild_change_position(int guild_id,int idx,int mode,int exp_mode,const char *name);
int guild_position_changed(int guild_id,int idx,struct guild_position *p);
int guild_change_notice(struct map_session_data *sd,int guild_id,const char *mes1,const char *mes2);
int guild_notice_changed(int guild_id,const char *mes1,const char *mes2);
int guild_change_emblem(struct map_session_data *sd,int len,const char *data);
int guild_emblem_changed(int len,int guild_id,int emblem_id,const char *data);
int guild_send_message(struct map_session_data *sd,const char *mes,int len);
int guild_recv_message(int guild_id,uint32 account_id,const char *mes,int len);
int guild_send_dot_remove(struct map_session_data *sd);
int guild_skillupack(int guild_id,uint16 skill_id,uint32 account_id);
int guild_break(struct map_session_data *sd,char *name);
int guild_broken(int guild_id,int flag);
int guild_gm_change(int guild_id, struct map_session_data *sd);
int guild_gm_changed(int guild_id, uint32 account_id, uint32 char_id);

void guild_castle_map_init(void);
int guild_castledatasave(int castle_id,int index,int value);
int guild_castledataloadack(int len, struct guild_castle *gc);
void guild_castle_reconnect(int castle_id, int index, int value);

void guild_agit_start(void);
void guild_agit_end(void);

void guild_agit2_start(void);
void guild_agit2_end(void);
/* guild flag cachin */
void guild_flag_add(struct npc_data *nd);
void guild_flag_remove(struct npc_data *nd);
void guild_flags_clear(void);

void guild_guildaura_refresh(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv);
#ifdef BOUND_ITEMS
void guild_retrieveitembound(uint32 char_id,uint32 account_id,int guild_id);
#endif

void do_final_guild(void);

#endif /* _GUILD_H_ */
