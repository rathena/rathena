// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _NPC_H_
#define _NPC_H_

#define START_NPC_NUM 110000000

#define WARP_CLASS 45
#define WARP_DEBUG_CLASS 722
#define INVISIBLE_CLASS 32767

//Checks if a given id is a valid npc id. [Skotlex]
//Since new npcs are added all the time, the max valid value is the one before the first mob (Scorpion = 1001)
#define npcdb_checkid(id) ((id >=  46 && id <= 125) || (id >= 700 && id <= 1000))

#ifdef PCRE_SUPPORT
void npc_chat_finalize(struct npc_data *nd);
#endif
int npc_chat_sub(struct block_list *bl, va_list ap);
int npc_event_dequeue(struct map_session_data *sd);
int npc_event_timer(int tid,unsigned int tick,int id,int data);
int npc_event(struct map_session_data *sd,const unsigned char *npcname,int);
int npc_timer_event(const unsigned char *eventname);				// Added by RoVeRT
int npc_command(struct map_session_data *sd,const unsigned char *npcname,char *command);
int npc_touch_areanpc(struct map_session_data *,int,int,int);
int npc_click(struct map_session_data *,int);
int npc_scriptcont(struct map_session_data *,int);
int npc_checknear(struct map_session_data *,int);
int npc_buysellsel(struct map_session_data *,int,int);
int npc_buylist(struct map_session_data *,int,unsigned short *);
int npc_selllist(struct map_session_data *,int,unsigned short *);
int npc_parse_mob(char *w1,char *w2,char *w3,char *w4);
int npc_parse_mob2 (struct mob_list *, int cached); // [Wizputer]
int npc_parse_warp(char *w1,char *w2,char *w3,char *w4);
int npc_globalmessage(const char *name,char *mes);

int npc_enable(const char *name,int flag);
int npc_changename(const char *name, const char *newname, short look); // [Lance]
struct npc_data* npc_name2id(const char *name);

int npc_walktoxy(struct npc_data *nd,int x,int y,int easy); // npc walking [Valaris]
int npc_stop_walking(struct npc_data *nd,int type);
int npc_changestate(struct npc_data *nd,int state,int type);

int npc_get_new_npc_id(void);

void npc_addsrcfile(char *);
void npc_delsrcfile(char *);
void npc_parsesrcfile(char *);
int do_final_npc(void);
int do_init_npc(void);
int npc_event_do_oninit(void);
int npc_do_ontimer(int,int);

int npc_event_doall(const unsigned char *name);
int npc_event_do(const unsigned char *name);
int npc_event_doall_id(const unsigned char *name, int id);

int npc_timerevent_start(struct npc_data *nd, int rid);
int npc_timerevent_stop(struct npc_data *nd);
void npc_timerevent_quit(struct map_session_data *sd);
int npc_gettimerevent_tick(struct npc_data *nd);
int npc_settimerevent_tick(struct npc_data *nd,int newtimer);
int npc_remove_map(struct npc_data *nd);
int npc_unload(struct npc_data *nd);
int npc_reload(void);

extern char *current_file;

#endif

