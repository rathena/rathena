// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NPC_H_
#define _NPC_H_

//#include "map.h"
struct block_list;
struct npc_data;
struct view_data;

#define START_NPC_NUM 110000000

#define WARP_CLASS 45
#define WARP_DEBUG_CLASS 722
#define FLAG_CLASS 722
#define INVISIBLE_CLASS 32767

#define MAX_NPC_CLASS 1000
//Checks if a given id is a valid npc id. [Skotlex]
//Since new npcs are added all the time, the max valid value is the one before the first mob (Scorpion = 1001)
#define npcdb_checkid(id) ((id >=  46 && id <= 125) || id == 139 || (id >= 700 && id <= MAX_NPC_CLASS) || id == INVISIBLE_CLASS)

#ifdef PCRE_SUPPORT
void npc_chat_finalize(struct npc_data* nd);
int mob_chat_sub(struct block_list* bl, va_list ap);
#endif

//Script NPC events.
enum npce_event {
	NPCE_LOGIN,
	NPCE_LOGOUT,
	NPCE_LOADMAP,
	NPCE_BASELVUP,
	NPCE_JOBLVUP,
	NPCE_DIE,
	NPCE_KILLPC,
	NPCE_KILLNPC,
	NPCE_MAX
};
struct view_data* npc_get_viewdata(int class_);
int npc_chat_sub(struct block_list* bl, va_list ap);
int npc_event_dequeue(struct map_session_data* sd);
int npc_event(struct map_session_data* sd, const char* eventname, int mob_kill);
int npc_touch_areanpc(struct map_session_data* sd, int m, int x, int y);
int npc_touch_areanpc2(struct block_list* bl); // [Skotlex]
int npc_check_areanpc(int flag, int m, int x, int y, int range);
int npc_click(struct map_session_data* sd, struct npc_data* nd);
int npc_scriptcont(struct map_session_data* sd, int id);
struct npc_data* npc_checknear(struct map_session_data* sd, struct block_list* bl);
int npc_checknear2(struct map_session_data* sd, struct block_list* bl);
int npc_buysellsel(struct map_session_data* sd, int id, int type);
int npc_buylist(struct map_session_data* sd,int n, unsigned short* item_list);
int npc_selllist(struct map_session_data* sd, int n, unsigned short* item_list);
int npc_parse_mob2(struct spawn_data* mob, int index); // [Wizputer]
struct npc_data* npc_add_warp(short from_mapid, short from_x, short from_y, short xs, short ys, unsigned short to_mapindex, short to_x, short to_y);
int npc_globalmessage(const char* name,const char* mes);

void npc_setcells(struct npc_data* nd);
void npc_unsetcells(struct npc_data* nd);
void npc_movenpc(struct npc_data* nd, int x, int y);
int npc_enable(const char* name, int flag);
void npc_setdisplayname(struct npc_data* nd, const char* newname);
void npc_setclass(struct npc_data* nd, short class_);
struct npc_data* npc_name2id(const char* name);

int npc_get_new_npc_id(void);

void npc_addsrcfile(const char* name);
void npc_delsrcfile(const char* name);
void npc_parsesrcfile(const char* filepath);
int do_final_npc(void);
int do_init_npc(void);
void npc_event_do_oninit(void);
int npc_do_ontimer(int npc_id, int option);

int npc_event_doall(const char* name);
int npc_event_do(const char* name);
int npc_event_doall_id(const char* name, int id);

int npc_timerevent_start(struct npc_data* nd, int rid);
int npc_timerevent_stop(struct npc_data* nd);
void npc_timerevent_quit(struct map_session_data* sd);
int npc_gettimerevent_tick(struct npc_data* nd);
int npc_settimerevent_tick(struct npc_data* nd, int newtimer);
int npc_remove_map(struct npc_data* nd);
void npc_unload_duplicates (struct npc_data* nd);
int npc_unload(struct npc_data* nd);
int npc_reload(void);
void npc_read_event_script(void);
int npc_script_event(struct map_session_data* sd, enum npce_event type);

extern struct npc_data* fake_nd;

#endif /* _NPC_H_ */
