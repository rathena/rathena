// $Id: npc.h,v 1.5 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _NPC_H_
#define _NPC_H_

#define START_NPC_NUM 110000000

#define WARP_CLASS 45
#define WARP_DEBUG_CLASS 722
#define INVISIBLE_CLASS 32767

int npc_event_dequeue(struct map_session_data *sd);
int npc_event_timer(int tid,unsigned int tick,int id,int data);
int npc_event(struct map_session_data *sd,const char *npcname,int);
int npc_timer_event(const char *eventname);				// Added by RoVeRT
int npc_command(struct map_session_data *sd,char *npcname,char *command);
int npc_touch_areanpc(struct map_session_data *,int,int,int);
int npc_click(struct map_session_data *,int);
int npc_scriptcont(struct map_session_data *,int);
int npc_checknear(struct map_session_data *,int);
int npc_buysellsel(struct map_session_data *,int,int);
int npc_buylist(struct map_session_data *,int,unsigned short *);
int npc_selllist(struct map_session_data *,int,unsigned short *);
int npc_parse_mob(char *w1,char *w2,char *w3,char *w4);
int npc_parse_warp(char *w1,char *w2,char *w3,char *w4);

int npc_enable(const char *name,int flag);
struct npc_data* npc_name2id(const char *name);

int npc_walktoxy(struct npc_data *nd,int x,int y,int easy); // npc walking [Valaris]
int npc_stop_walking(struct npc_data *nd,int type);
int npc_changestate(struct npc_data *nd,int state,int type);

int npc_get_new_npc_id(void);

void npc_addsrcfile(char *);
void npc_delsrcfile(char *);
int do_final_npc(void);
int do_init_npc(void);
int npc_event_do_oninit(void);
int npc_do_ontimer(int,struct map_session_data *,int);

int npc_event_doall(const char *name);
int npc_event_do(const char *name);

int npc_timerevent_start(struct npc_data *nd);
int npc_timerevent_stop(struct npc_data *nd);
int npc_gettimerevent_tick(struct npc_data *nd);
int npc_settimerevent_tick(struct npc_data *nd,int newtimer);
int npc_delete(struct npc_data *nd);

#endif

