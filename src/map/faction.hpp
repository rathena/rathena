// Complete Faction System (c) Lilith
// Skype: amurov4shtefan
// ICQ: 450327002
// Gmail: amurov.ro@gmail.com
// MSN: amurov.ro@hotmail.com

#ifndef _FACTION_H_
#define _FACTION_H_

#include "../common/mmo.hpp"
#include "unit.hpp"

// Faction data
struct faction_data {
	int id;
	char name[NAME_LENGTH];
	char pl_name[NAME_LENGTH];
	char map[MAP_NAME_LENGTH];
	uint16 x, y, ccolor;
	int race, ele, ele_lvl, size;
	int aura[MAX_AURA_EFF];
	unsigned long chat_color;
	struct script_code *script;

	int emblem_len, emblem_id;
	char emblem_data[2048];

	struct Channel *channel;
};

#define faction_check_chat(sd) ( (sd)->status.faction_id>0 && map_getmapflag((sd)->bl.m, MF_FVF) && battle_config.faction_chat_settings )

#define faction_check_hp(sd,dstsd) ( (sd)->status.faction_id>0 && map_getmapflag((dstsd)->bl.m, MF_FVF) && battle_config.fvf_hp_bar && \
		(sd)->status.faction_id == (dstsd)->status.faction_id )

#define faction_check_name(src,tbl) ( faction_get_id(src) && faction_get_id(tbl) && \
		battle_config.faction_chat_settings&2 && \
		(status_get_party_id(src)>0 && status_get_party_id(src) == status_get_party_id(tbl) || \
		status_get_guild_id(src)>0 && status_get_guild_id(src) == status_get_guild_id(tbl)) )

#define faction_check_skill_use(src,tbl) ( battle_config.faction_heal_settings && battle_config.faction_heal_bl&(tbl)->type && \
		((battle_config.faction_heal_settings&1 && faction_get_id(src) == faction_get_id(tbl)) || \
		(status_get_party_id(src)>0 && status_get_party_id(src) == status_get_party_id(tbl)) || \
		(status_get_guild_id(src)>0 && status_get_guild_id(src) == status_get_guild_id(tbl))) )

#define faction_check(sd,dstsd) ( (sd)->status.faction_id>0 && (sd)->status.faction_id == (dstsd)->status.faction_id )

extern struct guild factions[];
void faction_update_data(struct map_session_data *sd);
int faction_reload_fvf_sub(struct block_list *,va_list);

void faction_factionaura(struct map_session_data *);
void faction_calc(struct block_list *);
void faction_hp(struct map_session_data *);
void faction_spawn(struct map_session_data *);
void faction_show_aura(struct block_list *);
void faction_getareachar_unit(struct map_session_data *, struct block_list *);
int faction_aura_clear(struct block_list *,va_list);
struct faction_data *faction_search(int);
int faction_get_id(struct block_list *);
void faction_leave( struct map_session_data *sd); //biali

void do_reload_faction(void);
void do_init_faction(void);
void do_final_faction(void);

#endif /* _FACTION_H_ */