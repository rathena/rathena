// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/grfio.h"
#include "../common/showmsg.h"
#include "../common/ers.h"
#include "../common/db.h"
#include "map.h"
#include "log.h"
#include "npc.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "status.h"
#include "itemdb.h"
#include "script.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "skill.h"
#include "unit.h"

#ifdef _WIN32
#undef isspace
#define isspace(x)  (x == ' ' || x == '\t')
#endif

struct npc_src_list {
	struct npc_src_list * next;
//	struct npc_src_list * prev; //[Shinomori]
	char name[4];
};

static struct npc_src_list *npc_src_first=NULL;
static struct npc_src_list *npc_src_last=NULL;
static int npc_id=START_NPC_NUM;
static int npc_warp=0;
static int npc_shop=0;
static int npc_script=0;
static int npc_mob=0;
static int npc_delay_mob=0;
static int npc_cache_mob=0;
char *current_file = NULL;
int npc_get_new_npc_id(void){ return npc_id++; }

static struct dbt *ev_db;
static struct dbt *npcname_db;

struct event_data {
	struct npc_data *nd;
	int pos;
};
static struct tm ev_tm_b;	// 時計イベント用

static struct eri *timer_event_ers; //For the npc timer data. [Skotlex]

//For holding the view data of npc classes. [Skotlex]
static struct view_data npc_viewdb[MAX_NPC_CLASS];

static struct
{	//Holds pointers to the commonly executed scripts for speedup. [Skotlex]
	struct npc_data *nd;
	struct event_data *event[UCHAR_MAX];
	unsigned char *event_name[UCHAR_MAX];
	unsigned char event_count;
} script_event[NPCE_MAX];

struct view_data* npc_get_viewdata(int class_)
{	//Returns the viewdata for normal npc classes.
	if (class_ == INVISIBLE_CLASS)
		return &npc_viewdb[0];
	if (npcdb_checkid(class_) || class_ == WARP_CLASS)
		return &npc_viewdb[class_];
	return NULL;
}
/*==========================================
 * NPCの無効化/有効化
 * npc_enable
 * npc_enable_sub 有効時にOnTouchイベントを実行
 *------------------------------------------
 */
int npc_enable_sub( struct block_list *bl, va_list ap )
{
	struct map_session_data *sd;
	struct npc_data *nd;
	//char *name=(char *)aCallocA(50,sizeof(char)); // fixed [Shinomori]

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, nd=va_arg(ap,struct npc_data *));
	if(bl->type == BL_PC && (sd=(struct map_session_data *)bl)){
		char name[50]; // need 24 + 9 for the "::OnTouch"

		if (nd->sc.option&OPTION_INVISIBLE)	// 無効化されている
			return 1;

		if(sd->areanpc_id==nd->bl.id)
			return 1;
		sd->areanpc_id=nd->bl.id;

		snprintf(name, 50, "%s::OnTouch", nd->exname); // exname to be specific. exname is the unique identifier for script events. [Lance]
		npc_event(sd,name,0);
	}
	//aFree(name);
	return 0;
}
int npc_enable(const char *name,int flag)
{
	struct npc_data *nd= strdb_get(npcname_db,(unsigned char*)name);
	if (nd==NULL)
		return 0;

	if (flag&1)
		nd->sc.option&=~OPTION_INVISIBLE;
	else if (flag&2)
		nd->sc.option&=~OPTION_HIDE;
	else if (flag&4)
		nd->sc.option|= OPTION_HIDE;
	else	//Can't change the view_data to invisible class because the view_data for all npcs is shared! [Skotlex]
		nd->sc.option|= OPTION_INVISIBLE;

	if (nd->class_ == WARP_CLASS)
	{	//Client won't display option changes for warp portals [Toms]
		if (nd->sc.option&(OPTION_HIDE|OPTION_INVISIBLE))
			clif_clearchar(&nd->bl, 0);
		else
			clif_spawn(&nd->bl);
	} else
		clif_changeoption(&nd->bl);
		
	if(flag&3 && (nd->u.scr.xs > 0 || nd->u.scr.ys >0))
		map_foreachinarea( npc_enable_sub,nd->bl.m,nd->bl.x-nd->u.scr.xs,nd->bl.y-nd->u.scr.ys,nd->bl.x+nd->u.scr.xs,nd->bl.y+nd->u.scr.ys,BL_PC,nd);

	return 0;
}

/*==========================================
 * NPCを名前で探す
 *------------------------------------------
 */
struct npc_data* npc_name2id(const char *name)
{
	return (struct npc_data *) strdb_get(npcname_db,(unsigned char*)name);
}

/*==========================================
 * イベントキューのイベント処理
 *------------------------------------------
 */
int npc_event_dequeue(struct map_session_data *sd)
{
	nullpo_retr(0, sd);

	if(sd->npc_id)
	{	//Current script is aborted.
		if(sd->state.using_fake_npc){
			clif_clearchar_id(sd->npc_id, 0, sd->fd);
			sd->state.using_fake_npc = 0;
		}
		if (sd->st) {
			sd->st->pos = -1;
			script_free_stack(sd->st->stack);
			aFree(sd->st);
			sd->st = NULL;
		}
		sd->npc_id = 0;
	}

	if (!sd->eventqueue[0][0])
		return 0; //Nothing to dequeue

	if (!pc_addeventtimer(sd,100,sd->eventqueue[0]))
	{	//Failed to dequeue, couldn't set a timer.
		ShowWarning("npc_event_dequeue: event timer is full !\n");
		return 0;
	}
	//Event dequeued successfully, shift other elements.
	memmove(sd->eventqueue[0], sd->eventqueue[1], (MAX_EVENTQUEUE-1)*sizeof(sd->eventqueue[0]));
	sd->eventqueue[MAX_EVENTQUEUE-1][0]=0;
	return 1;
}

/*==========================================
 * イベントの遅延実行
 *------------------------------------------
 */
int npc_event_timer(int tid,unsigned int tick,int id,int data)
{
	unsigned char *eventname = (unsigned char *)data;
	struct event_data *ev = strdb_get(ev_db,eventname);
	struct npc_data *nd;
	struct map_session_data *sd=map_id2sd(id);
	size_t i;

	if((ev==NULL || (nd=ev->nd)==NULL))
	{
		if(battle_config.error_log)
			ShowWarning("npc_event: event not found [%s]\n",eventname);
	}
	else
	{
		for(i=0;i<MAX_EVENTTIMER;i++) {
			if( nd->eventtimer[i]==tid ) {
				nd->eventtimer[i]=-1;
				npc_event(sd,eventname,0); // sd NULL check is within
				break;
			}
		}
		if(i==MAX_EVENTTIMER && battle_config.error_log)
			ShowWarning("npc_event_timer: event timer not found [%s]!\n",eventname);
	}

	aFree(eventname);
	return 0;
}

int npc_timer_event(const unsigned char *eventname)	// Added by RoVeRT
{
	struct event_data *ev=strdb_get(ev_db,(unsigned char*)eventname);
	struct npc_data *nd;
//	int xs,ys;

	if((ev==NULL || (nd=ev->nd)==NULL)){
		ShowWarning("npc_timer_event: event not found [%s]\n",eventname);
		return 0;
	}

	run_script(nd->u.scr.script,ev->pos,nd->bl.id,nd->bl.id);

	return 0;
}
/*
int npc_timer_sub_sub(DBKey key,void *data,va_list ap)	// Added by RoVeRT
{
	char *p=(char *)key;
	struct event_data *ev=(struct event_data *)data;
	int *c=va_arg(ap,int *);
	int tick=0,ctick=gettick();
	char temp[10];
	char event[100];

	if(ev->nd->bl.id==(int)*c && (p=strchr(p,':')) && p && strncasecmp("::OnTimer",p,8)==0 ){
		sscanf(&p[9],"%s",temp);
		tick=atoi(temp);

		strcpy( event, ev->nd->name);
		strcat( event, p);

		if (ctick >= ev->nd->lastaction && ctick - ev->nd->timer >= tick) {
			npc_timer_event(event);
			ev->nd->lastaction = ctick;
		}
	}
	return 0;
}

int npc_timer_sub(DBKey key,void *data,va_list ap)	// Added by RoVeRT
{
	struct npc_data *nd=(struct npc_data*)data;

	if(nd->timer == -1)
		return 0;

	sv_db->foreach(ev_db,npc_timer_sub_sub,&nd->bl.id);

	return 0;
}

int npc_timer(int tid,unsigned int tick,int id,int data)	// Added by RoVeRT
{
	npcname_db->foreach(npcname_db,npc_timer_sub);

	aFree((void*)data);
	return 0;
}*/
/*==========================================
 * イベント用ラベルのエクスポート
 * npc_parse_script->strdb_foreachから呼ばれる
 *------------------------------------------
 */
int npc_event_export(char *lname,void *data,va_list ap)
{
	int pos=(int)data;
	struct npc_data *nd=va_arg(ap,struct npc_data *);

	if ((lname[0]=='O' || lname[0]=='o')&&(lname[1]=='N' || lname[1]=='n')) {
		struct event_data *ev;
		unsigned char buf[51];
		char *p=strchr(lname,':');
		// エクスポートされる
		ev=(struct event_data *) aMalloc(sizeof(struct event_data));
		if (ev==NULL) {
			ShowFatalError("npc_event_export: out of memory !\n");
			exit(1);
		}else if (p==NULL || (p-lname)>NAME_LENGTH) {
			ShowFatalError("npc_event_export: label name error !\n");
			exit(1);
		}else{
			ev->nd=nd;
			ev->pos=pos;
			*p='\0';
			sprintf(buf,"%s::%s",nd->exname,lname);
			*p=':';
			strdb_put(ev_db,buf,ev);
		}
	}
	return 0;
}

int npc_event_sub(struct map_session_data *, struct event_data *, const unsigned char *); //[Lance]
/*==========================================
 * 全てのNPCのOn*イベント実行
 *------------------------------------------
 */
int npc_event_doall_sub(DBKey key,void *data,va_list ap)
{
	unsigned char*p = key.str;
	struct event_data *ev;
	int *c;
	int rid;
	unsigned char *name;

	ev=(struct event_data *)data;
	c=va_arg(ap,int *);
	name=va_arg(ap,unsigned char *);
	rid=va_arg(ap, int);

	if( (p=strchr(p,':')) && p && strcmpi(name,p)==0 ){
		if(rid)
			npc_event_sub(((struct map_session_data *)map_id2bl(rid)),ev,key.str);
		else
			run_script(ev->nd->u.scr.script,ev->pos,rid,ev->nd->bl.id);
		(*c)++;
	}

	return 0;
}
int npc_event_doall(const unsigned char *name)
{
	int c=0;
	unsigned char buf[64]="::";

	strncpy(buf+2,name,62);
	ev_db->foreach(ev_db,npc_event_doall_sub,&c,buf,0);
	return c;
}
int npc_event_doall_id(const unsigned char *name, int rid)
{
	int c=0;
	unsigned char buf[64]="::";

	strncpy(buf+2,name,62);
	ev_db->foreach(ev_db,npc_event_doall_sub,&c,buf,rid);
	return c;
}

int npc_event_do_sub(DBKey key,void *data,va_list ap)
{
	unsigned char *p = key.str;
	struct event_data *ev;
	int *c;
	const unsigned char *name;

	nullpo_retr(0, ev=(struct event_data *)data);
	nullpo_retr(0, ap);
	nullpo_retr(0, c=va_arg(ap,int *));

	name=va_arg(ap,const unsigned char *);

	if (p && strcmpi(name,p)==0 ) {
		run_script(ev->nd->u.scr.script,ev->pos,0,ev->nd->bl.id);
		(*c)++;
	}

	return 0;
}
int npc_event_do(const unsigned char *name)
{
	int c=0;

	if (*name==':' && name[1]==':') {
		return npc_event_doall(name+2);
	}

	ev_db->foreach(ev_db,npc_event_do_sub,&c,name);
	return c;
}

/*==========================================
 * 時計イベント実行
 *------------------------------------------
 */
int npc_event_do_clock(int tid,unsigned int tick,int id,int data)
{
	time_t timer;
	struct tm *t;
	char buf[64];
        char *day="";
	int c=0;

	time(&timer);
	t=localtime(&timer);

        switch (t->tm_wday) {
	case 0: day = "Sun"; break;
	case 1: day = "Mon"; break;
	case 2: day = "Tue"; break;
	case 3: day = "Wed"; break;
	case 4: day = "Thu"; break;
	case 5: day = "Fri"; break;
	case 6: day = "Sat"; break;
	}

	if (t->tm_min != ev_tm_b.tm_min ) {
		sprintf(buf,"OnMinute%02d",t->tm_min);
		c+=npc_event_doall(buf);
		sprintf(buf,"OnClock%02d%02d",t->tm_hour,t->tm_min);
		c+=npc_event_doall(buf);
		sprintf(buf,"On%s%02d%02d",day,t->tm_hour,t->tm_min);
		c+=npc_event_doall(buf);
	}
	if (t->tm_hour!= ev_tm_b.tm_hour) {
		sprintf(buf,"OnHour%02d",t->tm_hour);
		c+=npc_event_doall(buf);
	}
	if (t->tm_mday!= ev_tm_b.tm_mday) {
		sprintf(buf,"OnDay%02d%02d",t->tm_mon+1,t->tm_mday);
		c+=npc_event_doall(buf);
	}
	memcpy(&ev_tm_b,t,sizeof(ev_tm_b));
	return c;
}
/*==========================================
 * OnInitイベント実行(&時計イベント開始)
 *------------------------------------------
 */
int npc_event_do_oninit(void)
{
//	int c = npc_event_doall("OnInit");
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"
	CL_WHITE"%d"CL_RESET"' NPCs.\n",npc_event_doall("OnInit"));

	add_timer_interval(gettick()+100,
		npc_event_do_clock,0,0,1000);

	return 0;
}
/*==========================================
 * OnTimer NPC event - by RoVeRT
 *------------------------------------------
 */
int npc_addeventtimer(struct npc_data *nd,int tick,const char *name)
{
	int i;
	unsigned char *evname;
	
	for(i=0;i<MAX_EVENTTIMER;i++)
		if( nd->eventtimer[i]==-1 )
			break;
	if(i<MAX_EVENTTIMER){
		if (!strdb_get(ev_db,(unsigned char*)name)) {
			if (battle_config.error_log)
				ShowError("npc_addeventimer: Event %s does not exists.\n", name);
			return 1; //Event does not exists!
		}
		evname =(unsigned char *) aMallocA(NAME_LENGTH*sizeof(char));
		if(evname==NULL){
			ShowFatalError("npc_addeventtimer: out of memory !\n");exit(1);
		}
		memcpy(evname,name,NAME_LENGTH-1);
		evname[NAME_LENGTH-1] = '\0';
		nd->eventtimer[i]=add_timer(gettick()+tick,
			npc_event_timer,nd->bl.id,(int)evname);
	}else
		ShowWarning("npc_addtimer: event timer is full !\n");

	return 0;
}

int npc_deleventtimer(struct npc_data *nd,const unsigned char *name)
{
	int i;
	for(i=0;i<MAX_EVENTTIMER;i++)
		if( nd->eventtimer[i]!=-1 && strcmp(
			(unsigned char *)(get_timer(nd->eventtimer[i])->data), name)==0 ){
				delete_timer(nd->eventtimer[i],npc_event_timer);
				nd->eventtimer[i]=-1;
				break;
		}

	return 0;
}

int npc_cleareventtimer(struct npc_data *nd)
{
	int i;
	for(i=0;i<MAX_EVENTTIMER;i++)
		if( nd->eventtimer[i]!=-1 ){
			delete_timer(nd->eventtimer[i],npc_event_timer);
			nd->eventtimer[i]=-1;
		}

	return 0;
}

int npc_do_ontimer_sub(DBKey key,void *data,va_list ap)
{
	unsigned char *p = key.str;
	struct event_data *ev = (struct event_data *)data;
	int *c = va_arg(ap,int *);
//	struct map_session_data *sd=va_arg(ap,struct map_session_data *);
	int option = va_arg(ap,int);
	int tick = 0;
	char temp[10];
	char event[50];

	if(ev->nd->bl.id == (int)*c && (p = strchr(p,':')) && strnicmp("::OnTimer",p,8) == 0){
		sscanf(&p[9], "%s", temp);
		tick = atoi(temp);

		strcpy(event, ev->nd->name);
		strcat(event, p);

		if (option!=0) {
			npc_addeventtimer(ev->nd, tick, event);
		} else {
			npc_deleventtimer(ev->nd, event);
		}
	}
	return 0;
}
int npc_do_ontimer(int npc_id, int option)
{
	ev_db->foreach(ev_db, npc_do_ontimer_sub, &npc_id, option);
	return 0;
}
/*==========================================
 * タイマーイベント用ラベルの取り込み
 * npc_parse_script->strdb_foreachから呼ばれる
 *------------------------------------------
 */
int npc_timerevent_import(char *lname,void *data,va_list ap)
{
	int pos=(int)data;
	struct npc_data *nd=va_arg(ap,struct npc_data *);
	int t=0,i=0;

	if(sscanf(lname,"OnTimer%d%n",&t,&i)==1 && lname[i]==':') {
		// タイマーイベント
		struct npc_timerevent_list *te=nd->u.scr.timer_event;
		int j,i=nd->u.scr.timeramount;
		if(te==NULL) te=(struct npc_timerevent_list*)aMallocA(sizeof(struct npc_timerevent_list));
		else te= (struct npc_timerevent_list*)aRealloc( te, sizeof(struct npc_timerevent_list) * (i+1) );
		if(te==NULL){
			ShowFatalError("npc_timerevent_import: out of memory !\n");
			exit(1);
		}
		for(j=0;j<i;j++){
			if(te[j].timer>t){
				memmove(te+j+1,te+j,sizeof(struct npc_timerevent_list)*(i-j));
				break;
			}
		}
		te[j].timer=t;
		te[j].pos=pos;
		nd->u.scr.timer_event=te;
		nd->u.scr.timeramount++;
	}
	return 0;
}
struct timer_event_data {
	int rid; //Attached player for this timer.
	int next; //timer index (starts with 0, then goes up to nd->u.scr.timeramount
	int time; //holds total time elapsed for the script since time 0 (whenthe timers started)
	unsigned int otick; //Holds tick value at which timer sequence was started (that is, it stores the tick value for which T= 0
};

/*==========================================
 * タイマーイベント実行
 *------------------------------------------
 */
int npc_timerevent(int tid,unsigned int tick,int id,int data)
{
	int next,t,old_rid,old_timer;
	unsigned int old_tick;
	struct npc_data* nd=(struct npc_data *)map_id2bl(id);
	struct npc_timerevent_list *te;
	struct timer_event_data *ted = (struct timer_event_data*)data;
	struct map_session_data *sd=NULL;
	
	if( nd==NULL ){
		ShowError("npc_timerevent: NPC not found??\n");
		return 0;
	}
	if (ted->rid) {
		sd = map_id2sd(ted->rid);
		if (!sd) {
			if(battle_config.error_log)
				ShowError("npc_timerevent: Attached player not found.\n");
			ers_free(timer_event_ers, ted);
			return 0;
		}
	}
	old_rid = nd->u.scr.rid; //To restore it later.
	nd->u.scr.rid = sd?sd->bl.id:0;
	
	old_tick = nd->u.scr.timertick;
	nd->u.scr.timertick=ted->otick;
	te=nd->u.scr.timer_event+ ted->next;
	
	old_timer = nd->u.scr.timer;
	t = nd->u.scr.timer=ted->time;
	ted->next++;
	
	if( nd->u.scr.timeramount> ted->next){
		next= nd->u.scr.timer_event[ ted->next ].timer
			- nd->u.scr.timer_event[ ted->next-1 ].timer;
		ted->time+=next;
		if (sd)
			sd->npc_timer_id = add_timer(tick+next,npc_timerevent,id,(int)ted);
		else
			nd->u.scr.timerid = add_timer(tick+next,npc_timerevent,id,(int)ted);
	} else {
		if (sd)
			sd->npc_timer_id = -1;
		else
			nd->u.scr.timerid = -1;
		ers_free(timer_event_ers, ted);
	}
	run_script(nd->u.scr.script,te->pos,nd->u.scr.rid,nd->bl.id);
	//Restore previous data, only if this timer is a player-attached one.
	if (sd) {
		nd->u.scr.rid = old_rid;
		nd->u.scr.timer = old_timer;
		nd->u.scr.timertick = old_tick;
	}
	return 0;
}
/*==========================================
 * タイマーイベント開始
 *------------------------------------------
 */
int npc_timerevent_start(struct npc_data *nd, int rid)
{
	int j,n, next;
	struct map_session_data *sd=NULL; //Player to whom script is attached.
	struct timer_event_data *ted;
		
	nullpo_retr(0, nd);

	n=nd->u.scr.timeramount;
	if( n==0 )
		return 0;

	for(j=0;j<n;j++){
		if( nd->u.scr.timer_event[j].timer > nd->u.scr.timer )
			break;
	}
	if(j>=n) // check if there is a timer to use !!BEFORE!! you write stuff to the structures [Shinomori]
		return 0;
	if (nd->u.scr.rid > 0) {
		//Try to attach timer to this player.
		sd = map_id2sd(nd->u.scr.rid);
		if (!sd) {
			if(battle_config.error_log)
				ShowError("npc_timerevent_start: Attached player not found!\n");
			return 1;
		}
	}
	//Check if timer is already started.
	if (sd) {
		if (sd->npc_timer_id != -1)
			return 0;
	} else if (nd->u.scr.timerid != -1)
		return 0;
		
	ted = ers_alloc(timer_event_ers, struct timer_event_data);
	ted->next = j;
	nd->u.scr.timertick=ted->otick=gettick();

	//Attach only the player if attachplayerrid was used.
	ted->rid = sd?sd->bl.id:0;

// Do not store it to make way to two types of timers: globals and personals.	
//	if (rid >= 0) nd->u.scr.rid=rid;	// changed to: attaching to given rid by default [Shinomori]
	// if rid is less than 0 leave it unchanged [celest]

	next = nd->u.scr.timer_event[j].timer - nd->u.scr.timer;
	ted->time = nd->u.scr.timer_event[j].timer;
	if (sd)
		sd->npc_timer_id = add_timer(gettick()+next,npc_timerevent,nd->bl.id,(int)ted);
	else
		nd->u.scr.timerid = add_timer(gettick()+next,npc_timerevent,nd->bl.id,(int)ted);
	return 0;
}
/*==========================================
 * タイマーイベント終了
 *------------------------------------------
 */
int npc_timerevent_stop(struct npc_data *nd)
{
	struct map_session_data *sd =NULL;
	struct TimerData *td = NULL;
	int *tid;
	nullpo_retr(0, nd);
	if (nd->u.scr.rid) {
		sd = map_id2sd(nd->u.scr.rid);
		if (!sd) {
			if(battle_config.error_log)
				ShowError("npc_timerevent_stop: Attached player not found!\n");
			return 1;
		}
	}
	
	tid = sd?&sd->npc_timer_id:&nd->u.scr.timerid;
	
	if (*tid == -1) //Nothing to stop
		return 0;
	td = get_timer(*tid);
	if (td && td->data) 
		ers_free(timer_event_ers, (struct event_timer_data*)td->data);
	delete_timer(*tid,npc_timerevent);
	*tid = -1;
	//Set the timer tick to the time that has passed since the beginning of the timers and now.
	nd->u.scr.timer = DIFF_TICK(gettick(),nd->u.scr.timertick);
//	nd->u.scr.rid = 0; //Eh? why detach?
	return 0;
}
/*==========================================
 * Aborts a running npc timer that is attached to a player.
 *------------------------------------------
 */
void npc_timerevent_quit(struct map_session_data *sd) {
	struct TimerData *td;
	if (sd->npc_timer_id == -1)
		return;
	td = get_timer(sd->npc_timer_id);
	if (!td) {
		sd->npc_timer_id = -1;
		return; //??
	}
	delete_timer(sd->npc_timer_id,npc_timerevent);
	sd->npc_timer_id = -1;
	ers_free(timer_event_ers, (struct event_timer_data*)td->data);
}

/*==========================================
 * タイマー値の所得
 *------------------------------------------
 */
int npc_gettimerevent_tick(struct npc_data *nd)
{
	int tick;
	nullpo_retr(0, nd);

	tick=nd->u.scr.timer;
	if (nd->u.scr.timertick)
		tick+=DIFF_TICK(gettick(), nd->u.scr.timertick);
	return tick;
}
/*==========================================
 * タイマー値の設定
 *------------------------------------------
 */
int npc_settimerevent_tick(struct npc_data *nd,int newtimer)
{
	int flag;
	struct map_session_data *sd=NULL;

	nullpo_retr(0, nd);

	if (nd->u.scr.rid) {
		sd = map_id2sd(nd->u.scr.rid);
		if (!sd) {
			if(battle_config.error_log)
				ShowError("npc_settimerevent_tick: Attached player not found!\n");
			return 1;
		}
		flag= sd->npc_timer_id != -1 ;
	} else
		flag= nd->u.scr.timerid != -1 ;

	if(flag)
		npc_timerevent_stop(nd);
	nd->u.scr.timer=newtimer;
	if(flag)
		npc_timerevent_start(nd, -1);
	return 0;
}

int npc_event_sub(struct map_session_data *sd, struct event_data *ev, const unsigned char *eventname){

	if ( sd->npc_id!=0) {
		//Enqueue the event trigger.
		int i;
		for(i=0;i<MAX_EVENTQUEUE && sd->eventqueue[i][0];i++);
		
		if (i==MAX_EVENTQUEUE) {
			if (battle_config.error_log)
				ShowWarning("npc_event: event queue is full !\n");
		}else //Event enqueued.
			memcpy(sd->eventqueue[i],eventname,50);
		return 1;
	}
	if (ev->nd->sc.option&OPTION_INVISIBLE) {
		//Disabled npc, shouldn't trigger event.
		npc_event_dequeue(sd);
		return 2;
	}
	run_script(ev->nd->u.scr.script,ev->pos,sd->bl.id,ev->nd->bl.id);
	return 0;
}

/*==========================================
 * イベント型のNPC処理
 *------------------------------------------
 */
int npc_event (struct map_session_data *sd, const unsigned char *eventname, int mob_kill)
{
	struct event_data *ev=strdb_get(ev_db,(unsigned char*)eventname);
	struct npc_data *nd;
	int xs,ys;
	unsigned char mobevent[100];

	if (sd == NULL) {
		nullpo_info(NLP_MARK);
		return 0;
	}

	if (ev == NULL && eventname && strcmp(((eventname)+strlen(eventname)-9),"::OnTouch") == 0)
		return 1;

	if (ev == NULL || (nd = ev->nd) == NULL) {
		if (mob_kill) {
			strcpy( mobevent, eventname);
			strcat( mobevent, "::OnMyMobDead");
			ev = strdb_get(ev_db, mobevent);
			if (ev == NULL || (nd = ev->nd) == NULL) {
				if (strnicmp(eventname, "GM_MONSTER",10) != 0)
					ShowError("npc_event: (mob_kill) event not found [%s]\n", mobevent);
				return 0;
			}
		} else {
			if (battle_config.error_log)
				ShowError("npc_event: event not found [%s]\n", eventname);
			return 0;
		}
	}

	xs=nd->u.scr.xs;
	ys=nd->u.scr.ys;
	if (xs>=0 && ys>=0 && (strcmp(((eventname)+strlen(eventname)-6),"Global") != 0) )
	{
		if (nd->bl.m >= 0) { //Non-invisible npc
		  	if (nd->bl.m != sd->bl.m )
				return 1;
			if ( xs>0 && (sd->bl.x<nd->bl.x-xs/2 || nd->bl.x+xs/2<sd->bl.x) )
				return 1;
			if ( ys>0 && (sd->bl.y<nd->bl.y-ys/2 || nd->bl.y+ys/2<sd->bl.y) )
				return 1;
		}
	}
	
	return npc_event_sub(sd,ev,eventname);
}


int npc_command_sub(DBKey key,void *data,va_list ap)
{
	unsigned char *p = key.str;
	struct event_data *ev=(struct event_data *)data;
	unsigned char *npcname=va_arg(ap,char *);
	char *command=va_arg(ap,char *);
	unsigned char temp[100];

	if(strcmp(ev->nd->name,npcname)==0 && (p=strchr(p,':')) && p && strnicmp("::OnCommand",p,10)==0 ){
		sscanf(&p[11],"%s",temp);

		if (strcmp(command,temp)==0)
			run_script(ev->nd->u.scr.script,ev->pos,0,ev->nd->bl.id);
	}

	return 0;
}

int npc_command(struct map_session_data *sd,const unsigned char *npcname,char *command)
{
	ev_db->foreach(ev_db,npc_command_sub,npcname,command);

	return 0;
}
/*==========================================
 * 接触型のNPC処理
 *------------------------------------------
 */
int npc_touch_areanpc(struct map_session_data *sd,int m,int x,int y)
{
	int i,f=1;
	int xs,ys;

	nullpo_retr(1, sd);

	if(sd->npc_id)
		return 1;

	for(i=0;i<map[m].npc_num;i++) {
		if (map[m].npc[i]->sc.option&OPTION_INVISIBLE) {	// 無効化されている
			f=0;
			continue;
		}

		switch(map[m].npc[i]->bl.subtype) {
		case WARP:
			xs=map[m].npc[i]->u.warp.xs;
			ys=map[m].npc[i]->u.warp.ys;
			break;
		case SCRIPT:
			xs=map[m].npc[i]->u.scr.xs;
			ys=map[m].npc[i]->u.scr.ys;
			break;
		default:
			continue;
		}
		if (x >= map[m].npc[i]->bl.x-xs/2 && x < map[m].npc[i]->bl.x-xs/2+xs &&
		   y >= map[m].npc[i]->bl.y-ys/2 && y < map[m].npc[i]->bl.y-ys/2+ys)
			break;
	}
	if (i==map[m].npc_num) {
		if (f) {
			if (battle_config.error_log)
				ShowError("npc_touch_areanpc : some bug \n");
		}
		return 1;
	}
	switch(map[m].npc[i]->bl.subtype) {
		case WARP:
			// hidden chars cannot use warps -- is it the same for scripts too?
			if (sd->sc.option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) ||
				(!battle_config.duel_allow_teleport && sd->duel_group)) // duel rstrct [LuzZza]
				break;
			pc_setpos(sd,map[m].npc[i]->u.warp.mapindex,map[m].npc[i]->u.warp.x,map[m].npc[i]->u.warp.y,0);
			break;
		case SCRIPT:
		{
			//char *name=(char *)aCallocA(50,sizeof(char));  // fixed [Shinomori]
			char name[50]; // need 24 max + 9 for "::OnTouch"

			if(sd->areanpc_id == map[m].npc[i]->bl.id)
				return 1;
			sd->areanpc_id = map[m].npc[i]->bl.id;

			sprintf(name,"%s::OnTouch", map[m].npc[i]->exname); // It goes here too. exname being the unique identifier. [Lance]

			if( npc_event(sd,name,0)>0 ) {
				pc_stop_walking(sd,1); //Make it stop walking!
				npc_click(sd,map[m].npc[i]);
			}
			//aFree(name);
			break;
		}
	}
	return 0;
}

int npc_touch_areanpc2(struct block_list *bl)
{
	int i,m=bl->m;
	int xs,ys;

	for(i=0;i<map[m].npc_num;i++) {
		if (map[m].npc[i]->sc.option&OPTION_INVISIBLE)
			continue;

		if (map[m].npc[i]->bl.subtype!=WARP)
			continue;
	
		xs=map[m].npc[i]->u.warp.xs;
		ys=map[m].npc[i]->u.warp.ys;

		if (bl->x >= map[m].npc[i]->bl.x-xs/2 && bl->x < map[m].npc[i]->bl.x-xs/2+xs &&
		   bl->y >= map[m].npc[i]->bl.y-ys/2 && bl->y < map[m].npc[i]->bl.y-ys/2+ys)
			break;
	}
	if (i==map[m].npc_num)
		return 0;
	
	xs = map_mapindex2mapid(map[m].npc[i]->u.warp.mapindex);
	if (xs < 0) // Can't warp object between map servers...
		return 0;

	if (unit_warp(bl, xs, map[m].npc[i]->u.warp.x,map[m].npc[i]->u.warp.y,0))
		return 0; //Failed to warp.

	return 1;
}

/*==========================================
 * 近くかどうかの判定
 *------------------------------------------
 */
int npc_checknear2(struct map_session_data *sd,struct block_list *bl)
{
	nullpo_retr(1, sd);
	if(bl == NULL) return 1;
	
	if(sd->state.using_fake_npc && sd->npc_id == bl->id)
		return 0;

	if (status_get_class(bl)<0) //Class-less npc, enable click from anywhere.
		return 0;

	if (bl->m!=sd->bl.m ||
	   bl->x<sd->bl.x-AREA_SIZE-1 || bl->x>sd->bl.x+AREA_SIZE+1 ||
	   bl->y<sd->bl.y-AREA_SIZE-1 || bl->y>sd->bl.y+AREA_SIZE+1)
		return 1;

	return 0;
}

TBL_NPC *npc_checknear(struct map_session_data *sd,struct block_list *bl)
{
	struct npc_data *nd;

	nullpo_retr(NULL, sd);
	if(bl == NULL) return NULL;
	if(bl->type != BL_NPC) return NULL;
	nd = (TBL_NPC*)bl;

	if(sd->state.using_fake_npc && sd->npc_id == bl->id)
		return nd;

	if (nd->class_<0) //Class-less npc, enable click from anywhere.
		return nd;

	if (bl->m!=sd->bl.m ||
	   bl->x<sd->bl.x-AREA_SIZE-1 || bl->x>sd->bl.x+AREA_SIZE+1 ||
	   bl->y<sd->bl.y-AREA_SIZE-1 || bl->y>sd->bl.y+AREA_SIZE+1)
		return NULL;

	return nd;
}

/*==========================================
 * NPCのオープンチャット発言
 *------------------------------------------
 */
int npc_globalmessage(const char *name,char *mes)
{
	struct npc_data *nd=(struct npc_data *) strdb_get(npcname_db,(unsigned char*)name);
	char temp[100];

	if (!nd)
		return 0;

	snprintf(temp, sizeof temp ,"%s : %s",name,mes);
	clif_GlobalMessage(&nd->bl,temp);

	return 0;
}

/*==========================================
 * クリック時のNPC処理
 *------------------------------------------
 */
int npc_click(struct map_session_data *sd,struct npc_data *nd)
{
	nullpo_retr(1, sd);

	if (sd->npc_id != 0) {
		if (battle_config.error_log)
			ShowError("npc_click: npc_id != 0\n");
		return 1;
	}

	if(!nd) return 1;
	if ((nd = npc_checknear(sd,&nd->bl)) == NULL)
		return 1;
	//Hidden/Disabled npc.
	if (nd->class_ < 0 || nd->sc.option&OPTION_INVISIBLE)
		return 1;

	switch(nd->bl.subtype) {
	case SHOP:
		clif_npcbuysell(sd,nd->bl.id);
		break;
	case SCRIPT:
		run_script(nd->u.scr.script,0,sd->bl.id,nd->bl.id);
		break;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_scriptcont(struct map_session_data *sd,int id)
{
	nullpo_retr(1, sd);

	if (id!=sd->npc_id){
		ShowWarning("npc_scriptcont: sd->npc_id (%d) is not id (%d).\n", sd->npc_id, id);
		return 1;
	}
	
	if(id != fake_nd->bl.id) { // Not item script
		if ((npc_checknear(sd,map_id2bl(id))) == NULL){
			ShowWarning("npc_scriptcont: failed npc_checknear test.\n");
			return 1;
		}
	}
	run_script_main(sd->st);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_buysellsel(struct map_session_data *sd,int id,int type)
{
	struct npc_data *nd;

	nullpo_retr(1, sd);

	if ((nd = npc_checknear(sd,map_id2bl(id))) == NULL)
		return 1;
	
	if (nd->bl.subtype!=SHOP) {
		if (battle_config.error_log)
			ShowError("no such shop npc : %d\n",id);
		if (sd->npc_id == id)
			sd->npc_id=0;
		return 1;
	}
	if (nd->sc.option&OPTION_INVISIBLE)	// 無効化されている
		return 1;

	sd->npc_shopid=id;
	if (type==0) {
		clif_buylist(sd,nd);
	} else {
		clif_selllist(sd);
	}
	return 0;
}

//npc_buylist for script-controlled shops.
static int npc_buylist_sub(
	struct map_session_data *sd,int n,
	unsigned short *item_list, struct npc_data *nd)
{
	unsigned char npc_ev[51];
	int i;
	int regkey = add_str("@bought_nameid");
	int regkey2 = add_str("@bought_quantity");
	sprintf(npc_ev, "%s::OnBuyItem", nd->exname);
	for(i=0;i<n;i++){
		pc_setreg(sd,regkey+(i<<24),(int)item_list[i*2+1]);
		pc_setreg(sd,regkey2+(i<<24),(int)item_list[i*2]);
	}
	npc_event(sd, npc_ev, 0);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_buylist(struct map_session_data *sd,int n,unsigned short *item_list)
{
	struct npc_data *nd;
	double z;
	int i,j,w,skill,itemamount=0,new_=0;

	nullpo_retr(3, sd);
	nullpo_retr(3, item_list);

	if ((nd = npc_checknear(sd,map_id2bl(sd->npc_shopid))) == NULL)
		return 3;

	if (nd->master_nd) //Script-based shops.
		return npc_buylist_sub(sd,n,item_list,nd->master_nd);

	if (nd->bl.subtype!=SHOP)
		return 3;

	for(i=0,w=0,z=0;i<n;i++) {
		for(j=0;nd->u.shop_item[j].nameid;j++) {
			if (nd->u.shop_item[j].nameid==item_list[i*2+1] || //Normal items
				itemdb_viewid(nd->u.shop_item[j].nameid)==item_list[i*2+1]) //item_avail replacement
				break;
		}
		if (nd->u.shop_item[j].nameid==0)
			return 3;
		
		if (!itemdb_isstackable(nd->u.shop_item[j].nameid) && item_list[i*2] > 1)
		{	//Exploit? You can't buy more than 1 of equipment types o.O
			ShowWarning("Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %d!\n",
				sd->status.name, sd->status.account_id, sd->status.char_id, item_list[i*2], nd->u.shop_item[j].nameid);
			item_list[i*2] = 1;
		}
		if (itemdb_value_notdc(nd->u.shop_item[j].nameid))
			z+=(double)nd->u.shop_item[j].value * item_list[i*2];
		else
			z+=(double)pc_modifybuyvalue(sd,nd->u.shop_item[j].value) * item_list[i*2];
		itemamount+=item_list[i*2];

		switch(pc_checkadditem(sd,item_list[i*2+1],item_list[i*2])) {
		case ADDITEM_EXIST:
			break;
		case ADDITEM_NEW:
			new_++;
			break;
		case ADDITEM_OVERAMOUNT:
			return 2;
		}

		w+=itemdb_weight(item_list[i*2+1]) * item_list[i*2];
	}
	if (z > (double)sd->status.zeny)
		return 1;	// zeny不足
	if (w+sd->weight > sd->max_weight)
		return 2;	// 重量超過
	if (pc_inventoryblank(sd)<new_)
		return 3;	// 種類数超過

	//Logs (S)hopping Zeny [Lupus]
	if(log_config.zeny > 0 )
		log_zeny(sd, "S", sd, -(int)z);
	//Logs

	pc_payzeny(sd,(int)z);
	for(i=0;i<n;i++) {
		struct item item_tmp;

		malloc_set(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid = item_list[i*2+1];
		item_tmp.identify = 1;	// npc販売アイテムは鑑定済み

		pc_additem(sd,&item_tmp,item_list[i*2]);

		//Logs items, Bought in NPC (S)hop [Lupus]
		if(log_config.enable_logs&0x20)
			log_pick_pc(sd, "S", item_tmp.nameid, item_list[i*2], NULL);
		//Logs
	}

	//商人経験値
	if (battle_config.shop_exp > 0 && z > 0 && (skill = pc_checkskill(sd,MC_DISCOUNT)) > 0) {
		if (sd->status.skill[MC_DISCOUNT].flag != 0)
			skill = sd->status.skill[MC_DISCOUNT].flag - 2;
		if (skill > 0) {
			z = z * (double)skill * (double)battle_config.shop_exp/10000.;
			if (z < 1)
				z = 1;
			pc_gainexp(sd,NULL,0,(int)z);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_selllist(struct map_session_data *sd,int n,unsigned short *item_list)
{
	double z;
	int i,skill,itemamount=0;
	struct npc_data *nd;
	
	nullpo_retr(1, sd);
	nullpo_retr(1, item_list);

	if ((nd = npc_checknear(sd,map_id2bl(sd->npc_shopid))) == NULL)
		return 1;
	nd = nd->master_nd; //For OnSell triggers.

	for(i=0,z=0;i<n;i++) {
		int nameid, idx, qty;
		idx = item_list[i*2]-2;
		qty = item_list[i*2+1];
		
		if (idx <0 || idx >=MAX_INVENTORY || qty < 0)
			break;
		
		nameid=sd->status.inventory[idx].nameid;
		if (nameid == 0 || !sd->inventory_data[idx] ||
		   sd->status.inventory[idx].amount < qty)
			break;
		
		if (sd->inventory_data[idx]->flag.value_notoc)
			z+=(double)qty*sd->inventory_data[idx]->value_sell;
		else
			z+=(double)qty*pc_modifysellvalue(sd,sd->inventory_data[idx]->value_sell);

		if(sd->inventory_data[idx]->type==7 && sd->status.inventory[idx].card[0] == (short)0xff00)
		{
			if(search_petDB_index(sd->status.inventory[idx].nameid, PET_EGG) >= 0)
				intif_delete_petdata(MakeDWord(sd->status.inventory[idx].card[1],sd->status.inventory[idx].card[2]));
		}

		if(log_config.enable_logs&0x20) //Logs items, Sold to NPC (S)hop [Lupus]
			log_pick_pc(sd, "S", nameid, -qty, &sd->status.inventory[idx]);

		if(nd) {
			pc_setreg(sd,add_str("@sold_nameid")+(i<<24),(int)sd->status.inventory[idx].nameid);
			pc_setreg(sd,add_str("@sold_quantity")+(i<<24),qty);
		}
		itemamount+=qty;
		pc_delitem(sd,idx,qty,0);
	}

	if (z > MAX_ZENY) z = MAX_ZENY;

	if(log_config.zeny) //Logs (S)hopping Zeny [Lupus]
		log_zeny(sd, "S", sd, (int)z);

	pc_getzeny(sd,(int)z);
	
	if (battle_config.shop_exp > 0 && z > 0 && (skill = pc_checkskill(sd,MC_OVERCHARGE)) > 0) {
		if (sd->status.skill[MC_OVERCHARGE].flag != 0)
			skill = sd->status.skill[MC_OVERCHARGE].flag - 2;
		if (skill > 0) {
			z = z * (double)skill * (double)battle_config.shop_exp/10000.;
			if (z < 1)
				z = 1;
			pc_gainexp(sd,NULL,0,(int)z);
		}
	}
		
	if(nd) {
		unsigned char npc_ev[51];
	  	sprintf(npc_ev, "%s::OnSellItem", nd->exname);
		npc_event(sd, npc_ev, 0);
	}
	
	if (i<n) {
		//Error/Exploit... of some sort. If we return 1, the client will not mark
		//any item as deleted even though a few were sold. In such a case, we
		//have no recourse but to kick them out so their inventory will refresh
		//correctly on relog. [Skotlex]
		if (i) clif_setwaitclose(sd->fd);
		return 1;
	}
	return 0;
}

int npc_remove_map(struct npc_data *nd)
{
	int m,i;
	nullpo_retr(1, nd);

	if(nd->bl.prev == NULL || nd->bl.m < 0)
		return 1; //Not assigned to a map.
  	m = nd->bl.m;
#ifdef PCRE_SUPPORT
	npc_chat_finalize(nd);
#endif
	clif_clearchar_area(&nd->bl,2);
	strdb_remove(npcname_db, (nd->bl.subtype < SCRIPT) ? nd->name : nd->exname);
	//Remove corresponding NPC CELLs
	if (nd->bl.subtype == WARP) {
		int j, xs, ys, x, y;
		x = nd->bl.x;
		y = nd->bl.y;
		xs = nd->u.warp.xs;
		ys = nd->u.warp.ys;

		for (i = 0; i < ys; i++) {
			for (j = 0; j < xs; j++) {
				if (map_getcell(m, x-xs/2+j, y-ys/2+i, CELL_CHKNPC))
					map_setcell(m, x-xs/2+j, y-ys/2+i, CELL_CLRNPC);
			}
		}
	}
	map_delblock(&nd->bl);
	map_deliddb(&nd->bl);
	//Remove npc from map[].npc list. [Skotlex]
	for(i=0;i<map[m].npc_num && map[m].npc[i] != nd;i++);
	if (i >= map[m].npc_num) return 2; //failed to find it?

	map[m].npc_num--;
	for(; i<map[m].npc_num; i++)
		map[m].npc[i]=map[m].npc[i+1];
	return 0;
}

static int npc_unload_ev(DBKey key,void *data,va_list ap) {
	struct event_data *ev=(struct event_data *)data;
	unsigned char *npcname=va_arg(ap,unsigned char *);

	if(strcmp(ev->nd->exname,npcname)==0){
		db_remove(ev_db, key);
		return 1;
	}
	return 0;
}

static int npc_unload_dup_sub(DBKey key,void * data,va_list ap)
{
	struct npc_data *nd = (struct npc_data *)data;
	int src_id;

	if(nd->bl.type!=BL_NPC || nd->bl.subtype != SCRIPT)
		return 0;

	src_id=va_arg(ap,int);
	if (nd->u.scr.src_id == src_id)
		npc_unload(nd);
	return 0;
}
//Removes all npcs that are duplicates of the passed one. [Skotlex]
void npc_unload_duplicates (struct npc_data *nd)
{
	map_foreachiddb(npc_unload_dup_sub,nd->bl.id);
}

int npc_unload(struct npc_data *nd)
{
	nullpo_ret(nd);

	npc_remove_map(nd);
	map_deliddb(&nd->bl);

	if (nd->chat_id) {
		struct chat_data *cd = (struct chat_data*)map_id2bl(nd->chat_id);
		if (cd) aFree (cd);
		cd = NULL;
	}
	if (nd->bl.subtype == SCRIPT) {
		ev_db->foreach(ev_db,npc_unload_ev,nd->exname); //Clean up all events related.
		if (nd->u.scr.timerid != -1) {
			struct TimerData *td = NULL;
			td = get_timer(nd->u.scr.timerid);
			if (td && td->data) 
				ers_free(timer_event_ers, (struct event_timer_data*)td->data);
			delete_timer(nd->u.scr.timerid, npc_timerevent);
		}
		npc_cleareventtimer (nd);
		if (nd->u.scr.timer_event)
			aFree(nd->u.scr.timer_event);
		if (nd->u.scr.src_id == 0) {
			if(nd->u.scr.script) {
				script_free_code(nd->u.scr.script);
				nd->u.scr.script = NULL;
			}
			if (nd->u.scr.label_list) {
				aFree(nd->u.scr.label_list);
				nd->u.scr.label_list = NULL;
			}
		}
	}
	aFree(nd);

	return 0;
}

//
// 初期化関係
//

/*==========================================
 * 読み込むnpcファイルのクリア
 *------------------------------------------
 */
void npc_clearsrcfile (void)
{
	struct npc_src_list *p = npc_src_first, *p2;

	while (p) {
		p2 = p;
		p = p->next;
		aFree(p2);
	}
	npc_src_first = NULL;
	npc_src_last = NULL;
}
/*==========================================
 * 読み込むnpcファイルの追加
 *------------------------------------------
 */
void npc_addsrcfile (char *name)
{
	struct npc_src_list *nsl;

	if (strcmpi(name, "clear") == 0) {
		npc_clearsrcfile();
		return;
	}

	// prevent multiple insert of source files
	nsl = npc_src_first;
	while (nsl)
	{   // found the file, no need to insert it again
		if (0 == strcmp(name, nsl->name))
			return;
		nsl = nsl->next;
	}

	nsl = (struct npc_src_list *) aMalloc (sizeof(*nsl) + strlen(name));
	nsl->next = NULL;
	strncpy(nsl->name, name, strlen(name) + 1);
	if (npc_src_first == NULL)
		npc_src_first = nsl;
	if (npc_src_last)
		npc_src_last->next = nsl;
	npc_src_last = nsl;
}
/*==========================================
 * 読み込むnpcファイルの削除
 *------------------------------------------
 */
void npc_delsrcfile (char *name)
{
	struct npc_src_list *p = npc_src_first, *pp = NULL, **lp = &npc_src_first;

	if (strcmpi(name, "all") == 0) {
		npc_clearsrcfile();
		return;
	}

	while (p) {
		if (strcmp(p->name, name) == 0) {
			*lp = p->next;
			if (npc_src_last == p)
				npc_src_last = pp;
			aFree(p);
			break;
		}
		lp = &p->next;
		pp = p;
		p = p->next;
	}
}

/*==========================================
 * warp行解析
 *------------------------------------------
 */
int npc_parse_warp (char *w1,char *w2,char *w3,char *w4)
{
	int x, y, xs, ys, to_x, to_y, m;
	int i, j;
	char mapname[MAP_NAME_LENGTH], to_mapname[MAP_NAME_LENGTH];
	struct npc_data *nd;

	// 引数の個数チェック
	if (sscanf(w1, "%15[^,],%d,%d", mapname, &x, &y) != 3 ||
	   sscanf(w4, "%d,%d,%15[^,],%d,%d", &xs, &ys, to_mapname, &to_x, &to_y) != 5) {
		ShowError("bad warp line : %s\n", w3);
		return 1;
	}

	m = map_mapname2mapid(mapname);
	i = mapindex_name2id(to_mapname);
	if (!i) {
		ShowError("bad warp line (destination map not found): %s\n", w3);
		return 1;
	}
		
	nd = (struct npc_data *) aCalloc (1, sizeof(struct npc_data));

	nd->bl.id = npc_get_new_npc_id();
	nd->n = map_addnpc(m, nd);
	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	memcpy(nd->name, w3, NAME_LENGTH-1);
	memcpy(nd->exname, w3, NAME_LENGTH-1);

	if (!battle_config.warp_point_debug)
		nd->class_ = WARP_CLASS;
	else
		nd->class_ = WARP_DEBUG_CLASS;
	nd->speed = 200;
	
	nd->u.warp.mapindex = (short)i;
	xs += 2;
	ys += 2;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = ys;

	for (i = 0; i < ys; i++) {
		for (j = 0; j < xs; j++) {
			if (map_getcell(m, x-xs/2+j, y-ys/2+i, CELL_CHKNOPASS))
				continue;
			map_setcell(m, x-xs/2+j, y-ys/2+i, CELL_SETNPC);
		}
	}

	npc_warp++;
	nd->bl.type = BL_NPC;
	nd->bl.subtype = WARP;
	map_addblock(&nd->bl);
	status_set_viewdata(&nd->bl, nd->class_);
	status_change_init(&nd->bl);
	unit_dataset(&nd->bl);
	clif_spawn(&nd->bl);
	strdb_put(npcname_db, nd->name, nd);

	return 0;
}

/*==========================================
 * shop行解析
 *------------------------------------------
 */
static int npc_parse_shop (char *w1, char *w2, char *w3, char *w4)
{
	#define MAX_SHOPITEM 100
	char *p;
	int x, y, dir, m, pos = 0;
	char mapname[MAP_NAME_LENGTH];
	struct npc_data *nd;

	if (strcmp(w1, "-") == 0) {
		x = 0; y = 0; dir = 0; m = -1;
	} else {
		// 引数の個数チェック
		if (sscanf(w1, "%15[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ||
	   	 strchr(w4, ',') == NULL) {
			ShowError("bad shop line : %s\n", w3);
			return 1;
		}
		m = map_mapname2mapid(mapname);
	}

	nd = (struct npc_data *) aCalloc (1, sizeof(struct npc_data) +
		sizeof(nd->u.shop_item[0]) * (MAX_SHOPITEM + 1));
	p = strchr(w4, ',');

	while (p && pos < MAX_SHOPITEM) {
		int nameid, value;
		struct item_data *id;
		p++;
		if (sscanf(p, "%d:%d", &nameid, &value) != 2)
			break;
		nd->u.shop_item[pos].nameid = nameid;
		id = itemdb_search(nameid);
		if (value < 0)
		{
			if (id->value_buy == 20)
				ShowWarning ("Selling item %s [%d] with no buying price (defaults to %d) at %s\n",
					id->name, id->nameid, id->value_buy, current_file);
			value = id->value_buy;
		}
		nd->u.shop_item[pos].value = value;
		// check for bad prices that can possibly cause exploits
		if (value/124. < id->value_sell/75.) {  //Clened up formula to prevent overflows.
			printf("\r"); //Carriage return to clear the 'loading..' line. [Skotlex]
			if (value < id->value_sell)
				ShowWarning ("Item %s [%d] buying price (%d) is less than selling price (%d) at %s\n",
					id->name, id->nameid, value, id->value_sell, current_file);
			else
				ShowWarning ("Item %s [%d] discounted buying price (%d) is less than overcharged selling price (%d) at %s\n",
					id->name, id->nameid, value/100*75, id->value_sell/100*124, current_file);
		}
		//for logs filters, atcommands and iteminfo script command
		if (id->maxchance<=0)
			id->maxchance = 10000; //10000 (100% drop chance)would show that the item's sold in NPC Shop

		pos++;
		p = strchr(p, ',');
	}
	if (pos == 0) {
		aFree(nd);
		return 1;
	}
	nd->u.shop_item[pos++].nameid = 0;

	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	nd->bl.id = npc_get_new_npc_id();
	memcpy(nd->name, w3, NAME_LENGTH-1);
	nd->name[NAME_LENGTH-1] = '\0';
	nd->class_ = m==-1?-1:atoi(w4);
	nd->speed = 200;
	
	nd = (struct npc_data *)aRealloc(nd,
		sizeof(struct npc_data) + sizeof(nd->u.shop_item[0]) * pos);

	npc_shop++;
	nd->bl.type = BL_NPC;
	nd->bl.subtype = SHOP;
	if (m >= 0) {
		nd->n = map_addnpc(m,nd);
		map_addblock(&nd->bl);
		status_set_viewdata(&nd->bl, nd->class_);
		status_change_init(&nd->bl);
		unit_dataset(&nd->bl);
		nd->ud.dir = dir;
		clif_spawn(&nd->bl);
	} else
		// we skip map_addnpc, but still add it to the list of ID's
		map_addiddb(&nd->bl);
	strdb_put(npcname_db, nd->name,nd);

	return 0;
}

/*==========================================
 * NPCのラベルデータコンバート
 *------------------------------------------
 */
int npc_convertlabel_db (DBKey key, void *data, va_list ap)
{
	unsigned char *lname = key.str;
	int pos = (int)data;
	struct npc_data *nd;
	struct npc_label_list *lst;
	int num;
	char *p;
	char c;

	nullpo_retr(0, ap);
	nullpo_retr(0, nd = va_arg(ap,struct npc_data *));

	lst = nd->u.scr.label_list;
	num = nd->u.scr.label_list_num;
	if (!lst) {
		lst = (struct npc_label_list *) aCallocA (1, sizeof(struct npc_label_list));
		num = 0;
	} else
		lst = (struct npc_label_list *) aRealloc (lst, sizeof(struct npc_label_list)*(num+1));

	// In case of labels not terminated with ':', for user defined function support
	p = lname;
	while(isalnum(*(unsigned char*)p) || *p == '_') { p++; }
	c = *p;
	*p='\0';

	// here we check if the label fit into the buffer
	if (strlen(lname) > 23) {
		ShowError("npc_parse_script: label name longer than 23 chars! '%s'\n (%s)", lname, current_file);
		exit(1);
	}
	memcpy(lst[num].name, lname, strlen(lname)+1); //including EOS

	*p = c;
	lst[num].pos = pos;
	nd->u.scr.label_list = lst;
	nd->u.scr.label_list_num = num+1;

	return 0;
}

/*==========================================
 * script行解析
 *------------------------------------------
 */
static void npc_parse_script_line(unsigned char *p,int *curly_count,int line) {
	int i = strlen((char *)p),j;
	int string_flag = 0;
	static int comment_flag = 0;
	for(j = 0; j < i ; j++) {
		if(comment_flag) {
			if(p[j] == '*' && p[j+1] == '/') {
				// マルチラインコメント終了
				j++;
				(*curly_count)--;
				comment_flag = 0;
			}
		} else if(string_flag) {
			if(p[j] == '"') {
				string_flag = 0;
			} else if(p[j] == '\\' && p[j-1]<=0x7e) {
				// エスケープ
				j++;
			}
		} else {
			if(p[j] == '"') {
				string_flag = 1;
			} else if(p[j] == '}') {
				if(*curly_count == 0) {
					break;
				} else {
					(*curly_count)--;
				}
			} else if(p[j] == '{') {
				(*curly_count)++;
			} else if(p[j] == '/' && p[j+1] == '/') {
				// コメント
				break;
			} else if(p[j] == '/' && p[j+1] == '*') {
				// マルチラインコメント
				j++;
				(*curly_count)++;
				comment_flag = 1;
			}
		}
	}
	if(string_flag) {
		printf("Missing '\"' at file %s line %d\n",current_file,line);
		exit(1);
	}
}

// Like npc_parse_script, except it's sole use is to skip the contents of a script. [Skotlex]
static int npc_skip_script (char *w1,char *w2,char *w3,char *w4,char *first_line,FILE *fp,int *lines)
{
	unsigned char *srcbuf = NULL;
	int srcsize = 65536;
	int startline = 0;
	unsigned char line[1024];
	int curly_count = 0;
	
	srcbuf = (unsigned char *)aMallocA(srcsize*sizeof(char));
	if (strchr(first_line, '{')) {
		strcpy((char *)srcbuf, strchr(first_line, '{'));
		startline = *lines;
	} else
		srcbuf[0] = 0;
	npc_parse_script_line(srcbuf,&curly_count,*lines);
	while (curly_count > 0) {
		fgets ((char *)line, 1020, fp);
		(*lines)++;
		npc_parse_script_line(line,&curly_count,*lines);
		if (feof(fp))
			break;
		if (strlen((char *)srcbuf) + strlen((char *)line) + 1 >= (size_t)srcsize) {
			srcsize += 65536;
			srcbuf = (unsigned char *)aRealloc(srcbuf, srcsize);
			malloc_tsetdword(srcbuf + srcsize - 65536, '\0', 65536);
		}
		if (srcbuf[0] != '{') {
			if (strchr((char *) line,'{')) {
				strcpy((char *) srcbuf, strchr((const char *) line, '{'));
				startline = *lines;
			}
		} else
			strcat((char *) srcbuf, (const char *) line);
	}
	if(curly_count > 0)
		ShowError("Missing right curly at file %s, line %d\n",current_file, *lines);
	aFree(srcbuf);
	return 0;
}

static int npc_parse_script(char *w1,char *w2,char *w3,char *w4,char *first_line,FILE *fp,int *lines,const char* file)
{
	int x, y, dir = 0, m, xs = 0, ys = 0, class_ = 0;	// [Valaris] thanks to fov
	char mapname[MAP_NAME_LENGTH];
	unsigned char *srcbuf = NULL;
	struct script_code *script;
	int srcsize = 65536;
	int startline = 0;
	unsigned char line[1024];
	int i;
	struct npc_data *nd, *dnd;
	struct dbt *label_db;
	char *p;
	struct npc_label_list *label_dup = NULL;
	int label_dupnum = 0;
	int src_id = 0;

	if (strcmp(w1, "-") == 0) {
		x = 0; y = 0; m = -1;
	} else {
		// 引数の個数チェック
		if (sscanf(w1, "%15[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ||
			(strcmp(w2, "script") == 0 && strchr(w4,',') == NULL)) {
			ShowError("bad script line (in file %s): %s\n", file, w3);
			return 1;
		}
		m = map_mapname2mapid(mapname);
	}

	if (strcmp(w2, "script") == 0){
		// parsing script with curly
		int curly_count = 0;
		srcbuf = (unsigned char *)aMallocA(srcsize*sizeof(char));
		if (strchr(first_line, '{')) {
			strcpy((char *)srcbuf, strchr(first_line, '{'));
			startline = *lines;
		} else
			srcbuf[0] = 0;
		npc_parse_script_line(srcbuf,&curly_count,*lines);
		while (curly_count > 0) {
			fgets ((char *)line, 1020, fp);
			(*lines)++;
			npc_parse_script_line(line,&curly_count,*lines);
			if (feof(fp))
				break;
			if (strlen((char *)srcbuf) + strlen((char *)line) + 1 >= (size_t)srcsize) {
				srcsize += 65536;
				srcbuf = (unsigned char *)aRealloc(srcbuf, srcsize);
				malloc_tsetdword(srcbuf + srcsize - 65536, '\0', 65536);
			}
			if (srcbuf[0] != '{') {
				if (strchr((char *) line,'{')) {
					strcpy((char *) srcbuf, strchr((const char *) line, '{'));
					startline = *lines;
				}
			} else
				strcat((char *) srcbuf, (const char *) line);
		}
		if(curly_count > 0) {
			ShowError("Missing right curly at file %s, line %d\n",file, *lines);
			script = NULL;
		} else {
			// printf("Ok line %d\n",*lines);
			script = parse_script((unsigned char *) srcbuf, file, startline);
		}
		if (script == NULL) {
			// script parse error?
			aFree(srcbuf);
			return 1;
		}
	} else {
		// duplicateする
		char srcname[128];
		struct npc_data *dnd;
		if (sscanf(w2, "duplicate(%[^)])", srcname) != 1) {
			ShowError("bad duplicate name (in %s)! : %s", file, w2);
			return 0;
		}
		if ((dnd = npc_name2id(srcname)) == NULL) {
			ShowError("bad duplicate name (in %s)! (not exist) : %s\n", file, srcname);
			return 0;
		}
		script = dnd->u.scr.script;
		label_dup = dnd->u.scr.label_list;
		label_dupnum = dnd->u.scr.label_list_num;
		src_id = dnd->bl.id;

	}// end of スクリプト解析

	nd = (struct npc_data *)aCalloc(1, sizeof(struct npc_data));

	if (sscanf(w4, "%d,%d,%d", &class_, &xs, &ys) == 3) {
		// 接触型NPC
		int i, j;

		if (xs >= 0) xs = xs * 2 + 1;
		if (ys >= 0) ys = ys * 2 + 1;

		if (m >= 0) {
			for (i = 0; i < ys; i++) {
				for (j = 0; j < xs; j++) {
					if (map_getcell(m, x - xs/2 + j, y - ys/2 + i, CELL_CHKNOPASS))
						continue;
					map_setcell(m, x - xs/2 + j, y - ys/2 + i, CELL_SETNPC);
				}
			}
		}
		nd->u.scr.xs = xs;
		nd->u.scr.ys = ys;
	} else {
		// クリック型NPC
		class_ = atoi(w4);
		nd->u.scr.xs = 0;
		nd->u.scr.ys = 0;
	}

	while ((p = strchr(w3,':'))) {
		if (p[1] == ':') break;
	}
	if (p) {
		*p = 0;
		memcpy(nd->name, w3, NAME_LENGTH-1);
		memcpy(nd->exname, p+2, NAME_LENGTH-1);
	} else {
		memcpy(nd->name, w3, NAME_LENGTH-1);
		memcpy(nd->exname, w3, NAME_LENGTH-1);
	}

	if((dnd = npc_name2id(nd->exname))){
		if(battle_config.etc_log)
			ShowInfo("npc_parse_script: Overriding NPC '%s::%s' to '%s::%d'.. in file '%s' (Duplicated System Name - Lazy scripters >_>) \n",nd->name,nd->exname,nd->name,npc_script,file);
		sprintf(nd->exname, "%d", npc_script);
	}

	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	nd->bl.id = npc_get_new_npc_id();
	nd->class_ = class_;
	nd->speed = 200;
	nd->u.scr.script = script;
	nd->u.scr.src_id = src_id;

	npc_script++;
	nd->bl.type = BL_NPC;
	nd->bl.subtype = SCRIPT;

	for (i = 0; i < MAX_EVENTTIMER; i++)
		nd->eventtimer[i] = -1;
	if (m >= 0) {
		nd->n = map_addnpc(m, nd);
		status_change_init(&nd->bl);
		unit_dataset(&nd->bl);
		nd->ud.dir = dir;
		map_addblock(&nd->bl);
		// Unused. You can always use xxx::OnXXXX events. Have this removed to improve perfomance.
		/*if (evflag) {	// イベント型
			struct event_data *ev = (struct event_data *)aCalloc(1, sizeof(struct event_data));
			ev->nd = nd;
			ev->pos = 0;
			strdb_put(ev_db, nd->exname, ev);
		} else {
			clif_spawn(&nd->bl);
		}*/
		if (class_ >= 0){
			status_set_viewdata(&nd->bl, nd->class_);
			clif_spawn(&nd->bl);
		}
	} else {
		// we skip map_addnpc, but still add it to the list of ID's
		map_addiddb(&nd->bl);
	}
	strdb_put(npcname_db, nd->exname, nd);

	//-----------------------------------------
	// ラベルデータの準備
	if (srcbuf){
		// script本体がある場合の処理
		// ラベルデータのコンバート
		label_db = script_get_label_db();
		label_db->foreach(label_db, npc_convertlabel_db, nd);

		// もう使わないのでバッファ解放
		aFree(srcbuf);
	} else {
		// duplicate
		nd->u.scr.label_list = label_dup;	// ラベルデータ共有
		nd->u.scr.label_list_num = label_dupnum;
	}

	//-----------------------------------------
	// イベント用ラベルデータのエクスポート
	for (i = 0; i < nd->u.scr.label_list_num; i++){
		char *lname = nd->u.scr.label_list[i].name;
		int pos = nd->u.scr.label_list[i].pos;

		if ((lname[0] == 'O' || lname[0] == 'o') && (lname[1] == 'N' || lname[1] == 'n')) {
			// this check is useless here because the buffer is only 24 chars
			// and already overwritten if this is here is reached
			// I leave the check anyway but place it correctly to npc_convertlabel_db
			if (strlen(lname)>NAME_LENGTH-1) {
				ShowError("npc_parse_script: label name longer than %d chars! '%s' (%s)\n", NAME_LENGTH-1, lname, file);
				exit(1);
			} else {
				struct event_data *ev;
				unsigned char buf[51];
				// 51 comes from: 24 for npc name + 24 for label + 2 for a "::" and 1 for EOS
				sprintf(buf,"%s::%s",nd->exname,lname);

				// remember the label is max 50 chars + eos; see the strdb_init below
				// generate the data and insert it
				ev=(struct event_data *)aMalloc(sizeof(struct event_data));
				ev->nd=nd;
				ev->pos=pos;
				if (strdb_put(ev_db,buf,ev) != NULL) //There was already another event of the same name?
					ShowWarning("npc_parse_script : duplicate event %s (%s)\n",buf, file);
			}
		}
	}

	//-----------------------------------------
	// ラベルデータからタイマーイベント取り込み
	for (i = 0; i < nd->u.scr.label_list_num; i++){
		int t = 0, k = 0;
		char *lname = nd->u.scr.label_list[i].name;
		int pos = nd->u.scr.label_list[i].pos;
		if (sscanf(lname, "OnTimer%d%n", &t, &k) == 1 && lname[k] == '\0') {
			// タイマーイベント
			struct npc_timerevent_list *te = nd->u.scr.timer_event;
			int j, k = nd->u.scr.timeramount;
			if (te == NULL)
				te = (struct npc_timerevent_list *)aMallocA(sizeof(struct npc_timerevent_list));
			else
				te = (struct npc_timerevent_list *)aRealloc( te, sizeof(struct npc_timerevent_list) * (k+1) );
			for (j = 0; j < k; j++){
				if (te[j].timer > t){
					memmove(te+j+1, te+j, sizeof(struct npc_timerevent_list)*(k-j));
					break;
				}
			}
			te[j].timer = t;
			te[j].pos = pos;
			nd->u.scr.timer_event = te;
			nd->u.scr.timeramount++;
		}
	}
	nd->u.scr.timerid = -1;

	return 0;
}

/*==========================================
 * function行解析
 *------------------------------------------
 */
static int npc_parse_function (char *w1, char *w2, char *w3, char *w4, char *first_line, FILE *fp, int *lines,const char* file)
{
	unsigned char *srcbuf, *p;
	struct script_code *script;
	struct script_code *oldscript;
	int srcsize = 65536;
	int startline = 0;
	char line[1024];
	int curly_count = 0;
	struct dbt *user_db;
	
	// スクリプトの解析
	srcbuf = (unsigned char *) aMallocA (srcsize*sizeof(char));
	if (strchr(first_line,'{')) {
		strcpy(srcbuf, strchr(first_line,'{'));
		startline = *lines;
	} else
		srcbuf[0] = 0;
	npc_parse_script_line(srcbuf,&curly_count,*lines);

	while (curly_count > 0) {
		fgets(line, sizeof(line) - 1, fp);
		(*lines)++;
		npc_parse_script_line(line,&curly_count,*lines);
		if (feof(fp))
			break;
		if (strlen(srcbuf)+strlen(line)+1 >= (unsigned int)srcsize) {
			srcsize += 65536;
			srcbuf = (char *)aRealloc(srcbuf, srcsize);
			malloc_tsetdword(srcbuf + srcsize - 65536, '\0', 65536);
		}
		if (srcbuf[0]!='{') {
			if (strchr(line,'{')) {
				strcpy(srcbuf, strchr(line,'{'));
				startline = *lines;
			}
		} else
			strcat(srcbuf,line);
	}
	if(curly_count > 0) {
		ShowError("Missing right curly at file %s, line %d\n",file, *lines);
		script = NULL;
	} else {
		script = parse_script(srcbuf, file, startline);
	}
	if (script == NULL) {
		// script parse error?
		aFree(srcbuf);
		return 1;
	}

	p = (char *) aMallocA (50*sizeof(char));
	strncpy(p, w3, 50);

	user_db = script_get_userfunc_db();
	oldscript = (struct script_code *)strdb_get(user_db, p);
	if(oldscript != NULL) {
		printf("\r"); //Carriage return to clear the 'loading..' line. [Skotlex]
		ShowInfo("parse_function: Overwriting user function [%s] (%s:%d)\n", p, file, *lines);
		script_free_code(oldscript);
		user_db->remove(user_db,str2key(p));
		strdb_put(user_db, p, script);
	} else
		strdb_put(user_db, p, script);

	// もう使わないのでバッファ解放
	aFree(srcbuf);

//	printf("function %s => %p\n",p,script);

	return 0;
}


/*==========================================
 * Parse Mob 1 - Parse mob list into each map
 * Parse Mob 2 - Actually Spawns Mob
 * [Wizputer]
 * If cached =1, it is a dynamic cached mob
 * index points to the index in the mob_list of the map_data cache.
 * -1 indicates that it is not stored on the map.
 *------------------------------------------
 */
int npc_parse_mob2 (struct spawn_data *mob, int index)
{
	int i;
	struct mob_data *md;

	for (i = 0; i < mob->num; i++) {
		md = mob_spawn_dataset(mob);
		md->spawn = mob;
		md->spawn_n = index;
		md->special_state.cached = (index>=0);	//If mob is cached on map, it is dynamically removed
		mob_spawn(md);
	}

	return 1;
}

int npc_parse_mob (char *w1, char *w2, char *w3, char *w4)
{
	int level, num, class_, mode, x,y,xs,ys;
	char mapname[MAP_NAME_LENGTH];
	char mobname[NAME_LENGTH];
	struct spawn_data mob, *data;

	malloc_set(&mob, 0, sizeof(struct spawn_data));

	// 引数の個数チェック
	if (sscanf(w1, "%15[^,],%d,%d,%d,%d", mapname, &x, &y, &xs, &ys) < 3 ||
		sscanf(w4, "%d,%d,%u,%u,%49[^\r\n]", &class_, &num, &mob.delay1, &mob.delay2, mob.eventname) < 2 ) {
		ShowError("bad monster line : %s %s %s (file %s)\n", w1, w3, w4, current_file);
		return 1;
	}
	if (!mapindex_name2id(mapname)) {
		ShowError("wrong map name : %s %s (file %s)\n", w1,w3, current_file);
		return 1;
	}
	mode =  map_mapname2mapid(mapname);
	if (mode < 0) //Not loaded on this map-server instance.
		return 1;
	mob.m = (unsigned short)mode;

	if (x < 0 || map[mob.m].xs <= x || y < 0 || map[mob.m].ys <= y) {
		ShowError("Out of range spawn coordinates: %s (%d,%d), map size is (%d,%d) - %s %s (file %s)\n", map[mob.m].name, x, y, map[mob.m].xs-1, map[mob.m].ys-1, w1,w3, current_file);
		return 1;
	}

	// check monster ID if exists!
	if (mobdb_checkid(class_)==0) {
		ShowError("bad monster ID : %s %s (file %s)\n", w3, w4, current_file);
		return 1;
	}

	if (num < 1 || num>1000 ) {
		ShowError("wrong number of monsters : %s %s (file %s)\n", w3, w4, current_file);
		return 1;
	}

	mob.num = (unsigned short)num;
	mob.class_ = (short) class_;
	mob.x = (unsigned short)x;
	mob.y = (unsigned short)y;
	mob.xs = (signed short)xs;
	mob.ys = (signed short)ys;

	if (mob.num > 1 && battle_config.mob_count_rate != 100) {
		if ((mob.num = mob.num * battle_config.mob_count_rate / 100) < 1)
			mob.num = 1;
	}

	if (battle_config.force_random_spawn || (mob.x == 0 && mob.y == 0))
	{	//Force a random spawn anywhere on the map.
		mob.x = mob.y = 0;
		mob.xs = mob.ys = -1;
	}

	//Apply the spawn delay fix [Skotlex]
	mode = mob_db(class_)->status.mode;
	if (mode & MD_BOSS) {	//Bosses
		if (battle_config.boss_spawn_delay != 100)
		{
			mob.delay1 = mob.delay1*battle_config.boss_spawn_delay/100;
			mob.delay2 = mob.delay2*battle_config.boss_spawn_delay/100;
		}
	} else if (mode&MD_PLANT) {	//Plants
		if (battle_config.plant_spawn_delay != 100)
		{
			mob.delay1 = mob.delay1*battle_config.plant_spawn_delay/100;
			mob.delay2 = mob.delay2*battle_config.plant_spawn_delay/100;
		}
	} else if (battle_config.mob_spawn_delay != 100)
	{	//Normal mobs
		mob.delay1 = mob.delay1*battle_config.mob_spawn_delay/100;
		mob.delay2 = mob.delay2*battle_config.mob_spawn_delay/100;
	}

	// parse MOB_NAME,[MOB LEVEL]
	if (sscanf(w3, "%23[^,],%d", mobname, &level) > 1)
		mob.level = level;

	if(mob.delay1>0xfffffff || mob.delay2>0xfffffff) {
		ShowError("wrong monsters spawn delays : %s %s (file %s)\n", w3, w4, current_file);
		return 1;
	}

	//Use db names instead of the spawn file ones.
	if(battle_config.override_mob_names==1)
		strcpy(mob.name,"--en--");
	else if (battle_config.override_mob_names==2)
		strcpy(mob.name,"--ja--");
	else
		strncpy(mob.name, mobname, NAME_LENGTH-1);

	if (!mob_parse_dataset(&mob)) //Verify dataset.
		return 1;

	//Now that all has been validated. We allocate the actual memory
	//that the re-spawn data will use.
	data = aMalloc(sizeof(struct spawn_data));
	memcpy(data, &mob, sizeof(struct spawn_data));
	
	if( !battle_config.dynamic_mobs || mob.delay1 || mob.delay2 ) {
		npc_parse_mob2(data,-1);
		npc_delay_mob += mob.num;
	} else {
		int index = map_addmobtolist(data->m, data);
		if( index >= 0 ) {
			// check if target map has players
			// (usually shouldn't occur when map server is just starting,
			// but not the case when we do @reloadscript
			if (map[mob.m].users > 0)
				npc_parse_mob2(data,index);
			npc_cache_mob += mob.num;
		} else {
			// mobcache is full
			// create them as delayed with one second
			mob.delay1 = 1000;
			mob.delay2 = 1000;
			npc_parse_mob2(data,-1);
			npc_delay_mob += mob.num;
		}
	}

	npc_mob++;

	return 0;
}

/*==========================================
 * マップフラグ行の解析
 *------------------------------------------
 */
static int npc_parse_mapflag (char *w1, char *w2, char *w3, char *w4)
{
	int m;
	char mapname[MAP_NAME_LENGTH];
	int state = 1;

	// 引数の個数チェック
	if (sscanf(w1, "%15[^,]",mapname) != 1)
		return 1;

	m = map_mapname2mapid(mapname);
	if (m < 0)
		return 1;
	if (w4 && strcmpi(w4, "off") == 0)
		state = 0;	//Disable mapflag rather than enable it. [Skotlex]
	
//マップフラグ
	if (strcmpi(w3, "nosave") == 0) {
		char savemap[MAP_NAME_LENGTH];
		int savex, savey;
		if (state == 0)
			; //Map flag disabled.
		else if (strcmp(w4, "SavePoint") == 0) {
			map[m].save.map = 0;
			map[m].save.x = -1;
			map[m].save.y = -1;
		} else if (sscanf(w4, "%15[^,],%d,%d", savemap, &savex, &savey) == 3) {
			map[m].save.map = mapindex_name2id(savemap);
			map[m].save.x = savex;
			map[m].save.y = savey;
			if (!map[m].save.map) {
				ShowWarning("Specified save point map '%s' for mapflag 'nosave' not found (file %s), using 'SavePoint'.\n",savemap,current_file);
				map[m].save.x = -1;
				map[m].save.y = -1;
			}
		}
		map[m].flag.nosave = state;
	}
	else if (strcmpi(w3,"nomemo")==0) {
		map[m].flag.nomemo=state;
	}
	else if (strcmpi(w3,"noteleport")==0) {
		map[m].flag.noteleport=state;
	}
	else if (strcmpi(w3,"nowarp")==0) {
		map[m].flag.nowarp=state;
	}
	else if (strcmpi(w3,"nowarpto")==0) {
		map[m].flag.nowarpto=state;
	}
	else if (strcmpi(w3,"noreturn")==0) {
		map[m].flag.noreturn=state;
	}
	else if (strcmpi(w3,"monster_noteleport")==0) {
		map[m].flag.monster_noteleport=state;
	}
	else if (strcmpi(w3,"nobranch")==0) {
		map[m].flag.nobranch=state;
	}
	else if (strcmpi(w3,"nopenalty")==0) {
		map[m].flag.noexppenalty=state;
		map[m].flag.nozenypenalty=state;
	}
	else if (strcmpi(w3,"pvp")==0) {
		map[m].flag.pvp=state;
		if (state) {
			map[m].flag.gvg=0;
			map[m].flag.gvg=0;
			map[m].flag.gvg_dungeon=0;
			map[m].flag.gvg_castle=0;
		}
	}
	else if (strcmpi(w3,"pvp_noparty")==0) {
		map[m].flag.pvp_noparty=state;
	}
	else if (strcmpi(w3,"pvp_noguild")==0) {
		map[m].flag.pvp_noguild=state;
	}
	else if (strcmpi(w3, "pvp_nightmaredrop") == 0) {
		char drop_arg1[16], drop_arg2[16];
		int drop_id = 0, drop_type = 0, drop_per = 0;
		if (sscanf(w4, "%[^,],%[^,],%d", drop_arg1, drop_arg2, &drop_per) == 3) {
			int i;
			if (strcmp(drop_arg1, "random") == 0)
				drop_id = -1;
			else if (itemdb_exists((drop_id = atoi(drop_arg1))) == NULL)
				drop_id = 0;
			if (strcmp(drop_arg2, "inventory") == 0)
				drop_type = 1;
			else if (strcmp(drop_arg2,"equip") == 0)
				drop_type = 2;
			else if (strcmp(drop_arg2,"all") == 0)
				drop_type = 3;

			if (drop_id != 0){
				for (i = 0; i < MAX_DROP_PER_MAP; i++) {
					if (map[m].drop_list[i].drop_id == 0){
						map[m].drop_list[i].drop_id = drop_id;
						map[m].drop_list[i].drop_type = drop_type;
						map[m].drop_list[i].drop_per = drop_per;
						break;
					}
				}
				map[m].flag.pvp_nightmaredrop = 1;
			}
		} else if (!state) //Disable
			map[m].flag.pvp_nightmaredrop = 0;
	}
	else if (strcmpi(w3,"pvp_nocalcrank")==0) {
		map[m].flag.pvp_nocalcrank=state;
	}
	else if (strcmpi(w3,"gvg")==0) {
		map[m].flag.gvg=state;
		if (state) map[m].flag.pvp=0;
	}
	else if (strcmpi(w3,"gvg_noparty")==0) {
		map[m].flag.gvg_noparty=state;
	}
	else if (strcmpi(w3,"gvg_dungeon")==0) {
		map[m].flag.gvg_dungeon=state;
		if (state) map[m].flag.pvp=0;
	}
	else if (strcmpi(w3,"gvg_castle")==0) {
		map[m].flag.gvg_castle=state;
		if (state) map[m].flag.pvp=0;
	}
	else if (strcmpi(w3,"noexppenalty")==0) {
		map[m].flag.noexppenalty=state;
	}
	else if (strcmpi(w3,"nozenypenalty")==0) {
		map[m].flag.nozenypenalty=state;
	}
	else if (strcmpi(w3,"notrade")==0) {
		map[m].flag.notrade=state;
	}
	else if (strcmpi(w3,"novending")==0) {
		map[m].flag.novending=state;
	}
	else if (strcmpi(w3,"nodrop")==0) {
		map[m].flag.nodrop=state;
	}
	else if (strcmpi(w3,"noskill")==0) {
		map[m].flag.noskill=state;
	}
	else if (strcmpi(w3,"noicewall")==0) { // noicewall [Valaris]
		map[m].flag.noicewall=state;
	}
	else if (strcmpi(w3,"snow")==0) { // snow [Valaris]
		map[m].flag.snow=state;
	}
	else if (strcmpi(w3,"clouds")==0) {
		map[m].flag.clouds=state;
	}
	else if (strcmpi(w3,"clouds2")==0) { // clouds2 [Valaris]
		map[m].flag.clouds2=state;
	}
	else if (strcmpi(w3,"fog")==0) { // fog [Valaris]
		map[m].flag.fog=state;
	}
	else if (strcmpi(w3,"fireworks")==0) {
		map[m].flag.fireworks=state;
	}
	else if (strcmpi(w3,"sakura")==0) { // sakura [Valaris]
		map[m].flag.sakura=state;
	}
	else if (strcmpi(w3,"leaves")==0) { // leaves [Valaris]
		map[m].flag.leaves=state;
	}
	else if (strcmpi(w3,"rain")==0) { // rain [Valaris]
		map[m].flag.rain=state;
	}
	else if (strcmpi(w3,"indoors")==0) { // celest
		map[m].flag.indoors=state;
	}
	else if (strcmpi(w3,"nightenabled")==0) { // Skotlex
		map[m].flag.nightenabled=state;
	}
	else if (strcmpi(w3,"nogo")==0) { // celest
		map[m].flag.nogo=state;
	}
	else if (strcmpi(w3,"noexp")==0) { // Lorky
		map[m].flag.nobaseexp=state;
		map[m].flag.nojobexp=state;
	}
	else if (strcmpi(w3,"nobaseexp")==0) { // Lorky
		map[m].flag.nobaseexp=state;
	}
	else if (strcmpi(w3,"nojobexp")==0) { // Lorky
		map[m].flag.nojobexp=state;
	}
	else if (strcmpi(w3,"noloot")==0) { // Lorky
		map[m].flag.nomobloot=state;
		map[m].flag.nomvploot=state;
	}
	else if (strcmpi(w3,"nomobloot")==0) { // Lorky
		map[m].flag.nomobloot=state;
	}
	else if (strcmpi(w3,"nomvploot")==0) { // Lorky
		map[m].flag.nomvploot=state;
	}
	else if (strcmpi(w3,"nocommand")==0) { // Skotlex
		if (state) {
			if (sscanf(w4, "%d", &state) == 1)
				map[m].nocommand =state;
			else //No level specified, block everyone.
				map[m].nocommand =100;
		} else
			map[m].nocommand=0;
	}
	else if (strcmpi(w3,"restricted")==0) { // Komurka
		if (state) {
			map[m].flag.restricted=1;
			sscanf(w4, "%d", &state);
			map[m].zone |= 1<<(state+1);
		} else {
			map[m].flag.restricted=0;
			map[m].zone = 0;
		}
	}
	else if (strcmpi(w3,"jexp")==0) {
		map[m].jexp = (state) ? atoi(w4) : 100;
		if( map[m].jexp < 0 ) map[m].jexp = 100;
		map[m].flag.nojobexp = (map[m].jexp==0)?1:0;
	}
	else if (strcmpi(w3,"bexp")==0) {
		map[m].bexp = (state) ? atoi(w4) : 100;
		if( map[m].bexp < 0 ) map[m].bexp = 100;
		 map[m].flag.nobaseexp = (map[m].bexp==0)?1:0;
	}
	else if (strcmpi(w3,"loadevent")==0) { // Skotlex
		map[m].flag.loadevent=state;
	}
	else if (strcmpi(w3,"nochat")==0) { // Skotlex
		map[m].flag.nochat=state;
	}
	else if (strcmpi(w3,"partylock")==0) { // Skotlex
		map[m].flag.partylock=state;
	}
	else if (strcmpi(w3,"guildlock")==0) { // Skotlex
		map[m].flag.guildlock=state;
	}

	return 0;
}

/*==========================================
 * Setting up map cells
 *------------------------------------------
 */
static int npc_parse_mapcell (char *w1, char *w2, char *w3, char *w4)
{
	int m, cell, x, y, x0, y0, x1, y1;
	char type[24], mapname[MAP_NAME_LENGTH];

	if (sscanf(w1, "%15[^,]", mapname) != 1)
		return 1;

	m = map_mapname2mapid(mapname);
	if (m < 0)
		return 1;

	if (sscanf(w3, "%23[^,],%d,%d,%d,%d", type, &x0, &y0, &x1, &y1) < 4) {
		ShowError("Bad setcell line : %s\n",w3);
		return 1;
	}
	cell = strtol(type, (char **)NULL, 0);
	//printf ("0x%x %d %d %d %d\n", cell, x0, y0, x1, y1);

	if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
	if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }

	for (x = x0; x <= x1; x++) {
		for (y = y0; y <= y1; y++) {
			map_setcell(m, x, y, cell);
			//printf ("setcell 0x%x %d %d %d\n", cell, m, x, y);
		}
	}

	return 0;
}

void npc_parsesrcfile (char *name)
{
	int m, lines = 0;
	char line[2048];

	FILE *fp = fopen (name,"r");
	if (fp == NULL) {
		ShowError ("File not found : %s\n", name);
		return;
	}
	current_file = name;

	while (fgets(line, sizeof(line) - 1, fp)) {
		char w1[2048], w2[2048], w3[2048], w4[2048], mapname[2048];
		int i, w4pos, count;
		lines++;

		if (line[0] == '/' && line[1] == '/')
			continue;

		if (!sscanf(line, " %n", &i) && i == strlen(line)) // just whitespace
			continue;

		// 最初はタブ区切りでチェックしてみて、ダメならスペース区切りで確認
		w1[0] = w2[0] = w3[0] = w4[0] = '\0'; //It's best to initialize values
		//to prevent passing previously parsed values to the parsers when not all
		//fields are specified. [Skotlex]
		if ((count = sscanf(line, "%[^\t\r\n]\t%[^\t\r\n]\t%[^\t\r\n]\t%n%[^\r\n]", w1, w2, w3, &w4pos, w4)) < 3)
		{
			if ((count = sscanf(line, "%s %s %[^\t]\t %n%[^\n]", w1, w2, w3, &w4pos, w4)) == 4 ||
			(count = sscanf(line, "%s %s %s %n%[^\n]\n", w1, w2, w3, &w4pos, w4)) >= 3)
			{
				ShowWarning("\r");
				ShowWarning("Incorrect separator syntax in file '%s', line '%i'. Use tabs instead of spaces!\n * %s %s %s %s\n",current_file,lines,w1,w2,w3,w4);
			} else {
				ShowError("\r"); //Erase the npc spinner.
				ShowError("Could not parse file '%s', line '%i'.\n * %s %s %s %s\n",current_file,lines,w1,w2,w3,w4);
				continue;
			}
		}

		// マップの存在確認
		if (strcmp(w1,"-") !=0 && strcmpi(w1,"function") != 0 ){
			sscanf(w1,"%[^,]",mapname);
			if (!mapindex_name2id(mapname)) { //Incorrect map
				ShowError("Invalid map '%s' in line %d, file %s\n", mapname, lines, current_file);
				if (strcmpi(w2,"script") == 0 && count > 3)	//we must skip the script info...
					npc_skip_script(w1,w2,w3,w4,line+w4pos,fp,&lines);
				continue;
			}
			if ((m = map_mapname2mapid(mapname)) < 0) {
			// "mapname" is not assigned to this server
			// we must skip the script info...
				if (strcmpi(w2,"script") == 0 && count > 3)
					npc_skip_script(w1,w2,w3,w4,line+w4pos,fp,&lines);
				continue;
			}
		}
		if (strcmpi(w2,"warp") == 0 && count > 3) {
			npc_parse_warp(w1,w2,w3,w4);
		} else if (strcmpi(w2,"shop") == 0 && count > 3) {
			npc_parse_shop(w1,w2,w3,w4);
		} else if (strcmpi(w2,"script") == 0 && count > 3) {
			if (strcmpi(w1,"function") == 0) {
				npc_parse_function(w1,w2,w3,w4,line+w4pos,fp,&lines,name);
			} else {
				npc_parse_script(w1,w2,w3,w4,line+w4pos,fp,&lines,name);
			}
		} else if ((i = 0, sscanf(w2,"duplicate%n",&i), (i > 0 && w2[i] == '(')) && count > 3) {
			npc_parse_script(w1,w2,w3,w4,line+w4pos,fp,&lines,name);
		} else if (strcmpi(w2,"monster") == 0 && count > 3) {
			npc_parse_mob(w1,w2,w3,w4);
		} else if (strcmpi(w2,"mapflag") == 0 && count >= 3) {
			npc_parse_mapflag(w1,w2,w3,w4);
		} else if (strcmpi(w2,"setcell") == 0 && count >= 3) {
			npc_parse_mapcell(w1,w2,w3,w4);
		} else {
			ShowError("Probably TAB is missing in file '%s', line '%i':\n * %s %s %s %s\n",current_file,lines,w1,w2,w3,w4);
		}
	}
	fclose(fp);

	return;
}

int npc_script_event(TBL_PC* sd, int type) {
	int i;
	if (type < 0 || type >= NPCE_MAX)
		return 0;
	if (!sd) {
		if (battle_config.error_log)
			ShowError("npc_script_event: NULL sd. Event Type %d\n", type);
		return 0;
	}
	if (script_event[type].nd) {
		TBL_NPC *nd = script_event[type].nd;
		run_script(nd->u.scr.script,0,sd->bl.id,nd->bl.id);
		return 1;
	} else if (script_event[type].event_count) {
		for (i = 0; i<script_event[type].event_count; i++) {
			npc_event_sub(sd,script_event[type].event[i],script_event[type].event_name[i]);
		}
		return i;
	} 
	return 0;
}

static int npc_read_event_script_sub(DBKey key,void *data,va_list ap)
{
	unsigned char *p = key.str;
	unsigned char *name = va_arg(ap,unsigned char *);
	struct event_data **event_buf = va_arg(ap,struct event_data**);
	unsigned char **event_name = va_arg(ap,unsigned char **);
	unsigned char *count = va_arg(ap,char *);;

	if (*count >= UCHAR_MAX) return 0;
	
	if((p=strchr(p,':')) && p && strcmpi(name,p)==0 )
	{
		event_buf[*count] = (struct event_data *)data;
		event_name[*count] = key.str;
		(*count)++;
		return 1;
	}
	return 0;
}

static void npc_read_event_script(void)
{
	int i;
	unsigned char buf[64]="::";
	struct {
		char *name;
		char *event_name;
	} config[] = {
		{"Login Event",script_config.login_event_name},
		{"Logout Event",script_config.logout_event_name},
		{"Load Map Event",script_config.loadmap_event_name},
		{"Base LV Up Event",script_config.baselvup_event_name},
		{"Job LV Up Event",script_config.joblvup_event_name},
		{"Die Event",script_config.die_event_name},
		{"Kill PC Event",script_config.kill_pc_event_name},
		{"Kill NPC Event",script_config.kill_mob_event_name},
	};

	for (i = 0; i < NPCE_MAX; i++) {
		if (script_event[i].nd)
			script_event[i].nd = NULL;
		if (script_event[i].event_count)
			script_event[i].event_count = 0;
		if (!script_config.event_script_type) {
			//Use a single NPC as event source.
			script_event[i].nd = npc_name2id(config[i].event_name);
		} else {
			//Use an array of Events
			strncpy(buf+2,config[i].event_name,62);
			ev_db->foreach(ev_db,npc_read_event_script_sub,buf,
				&script_event[i].event,
				&script_event[i].event_name,
				&script_event[i].event_count);
		}
	}
	if (battle_config.etc_log) {
		//Print summary.
		for (i = 0; i < NPCE_MAX; i++) {
			if(!script_config.event_script_type) {
				if (script_event[i].nd)
					ShowInfo("%s: Using NPC named '%s'.\n", config[i].name, config[i].event_name);
				else
					ShowInfo("%s: No NPC found with name '%s'.\n", config[i].name, config[i].event_name);
			} else
				ShowInfo("%s: %d '%s' events.\n", config[i].name, script_event[i].event_count, config[i].event_name);
		}
	}
}
static int npc_read_indoors (void)
{
	char *buf, *p;
	int s, m;

	buf = (char *)grfio_reads("data\\indoorrswtable.txt",&s);
	if (buf == NULL)
		return -1;
	buf[s] = 0;

	for (p = buf; p - buf < s; ) {
		char map_name[64];
		if (sscanf(p, "%15[^#]#", map_name) == 1) {
			size_t pos = strlen(map_name) - 4;	// replace '.xxx' extension
			memcpy(map_name+pos,".gat",4);		// with '.gat'
			if ((m = map_mapname2mapid(map_name)) >= 0)
				map[m].flag.indoors = 1;
		}

		p = strchr(p, 10);
		if (!p) break;
		p++;
	}
	aFree(buf);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","data\\indoorrswtable.txt");

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */

static int npc_cleanup_sub (struct block_list *bl, va_list ap) {
	nullpo_retr(0, bl);

	switch(bl->type) {
	case BL_NPC:
		npc_unload((struct npc_data *)bl);
		break;
	case BL_MOB:
		//This is used only on reloading npcs, so let's not free spawn-once mobs. [Skotlex]
		if (((TBL_MOB*)bl)->spawn)
			unit_free(bl,0);
		break;
	}

	return 0;
}

static int npc_cleanup_dbsub(DBKey key,void * data,va_list ap) {
	return npc_cleanup_sub((struct block_list*)data, 0);
}

int npc_reload (void)
{
	struct npc_src_list *nsl;
	int m, i;
	time_t last_time = time(0);
	int busy = 0, npc_new_min = npc_id;
	char c = '-';

	//Remove all npcs/mobs. [Skotlex]
	map_foreachiddb(npc_cleanup_dbsub);
	for (m = 0; m < map_num; m++) {
		if(battle_config.dynamic_mobs) {	//dynamic check by [random]
			for (i = 0; i < MAX_MOB_LIST_PER_MAP; i++)
				if (map[m].moblist[i]) aFree(map[m].moblist[i]);
			malloc_set (map[m].moblist, 0, sizeof(map[m].moblist));
		}
		if (map[m].npc_num > 0 && battle_config.error_log)
			ShowWarning("npc_reload: %d npcs weren't removed at map %s!\n", map[m].npc_num, map[m].name);
	}

	// anything else we should cleanup?
	// Reloading npc's now
	ev_db->clear(ev_db,NULL);
	npcname_db->clear(npcname_db,NULL);
	npc_warp = npc_shop = npc_script = 0;
	npc_mob = npc_cache_mob = npc_delay_mob = 0;

	for (nsl = npc_src_first; nsl; nsl = nsl->next) {
		npc_parsesrcfile(nsl->name);
		if (script_config.verbose_mode) {
			printf("\r");
			ShowStatus("Loading NPCs... %-53s", nsl->name);
		} else {
			if (last_time != time(0)) {
				printf("\r");
				ShowStatus("Loading NPCs... Working: ");
				last_time = time(0);
				switch(busy) {
					case 0: c='\\'; busy++; break;
					case 1: c='|'; busy++; break;
					case 2: c='/'; busy++; break;
					case 3: c='-'; busy=0;
				}
				printf("[%c]",c);
			}
		}
		fflush(stdout);
	}
	printf("\r");
	ShowInfo ("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:%30s\n\t-'"
		CL_WHITE"%d"CL_RESET"' Warps\n\t-'"
		CL_WHITE"%d"CL_RESET"' Shops\n\t-'"
		CL_WHITE"%d"CL_RESET"' Scripts\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Cached\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Not Cached\n",
		npc_id - npc_new_min, "", npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);

	//Re-read the NPC Script Events cache.
	npc_read_event_script();
	
	//Execute the OnInit event for freshly loaded npcs. [Skotlex]
	ShowStatus("Event '"CL_WHITE"OnInit"CL_RESET"' executed with '"
	CL_WHITE"%d"CL_RESET"' NPCs.\n",npc_event_doall("OnInit"));
	// Execute rest of the startup events if connected to char-server. [Lance]
	if(!CheckForCharServer()){
		ShowStatus("Event '"CL_WHITE"OnCharIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnCharIfInit"));
		ShowStatus("Event '"CL_WHITE"OnInterIfInit"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnInterIfInit"));
		ShowStatus("Event '"CL_WHITE"OnInterIfInitOnce"CL_RESET"' executed with '"CL_WHITE"%d"CL_RESET"' NPCs.\n", npc_event_doall("OnInterIfInitOnce"));
	}
	return 0;
}

/*==========================================
 * 終了
 *------------------------------------------
 */
int do_final_npc(void)
{
	int i;
	struct block_list *bl;

	for (i = START_NPC_NUM; i < npc_id; i++){
		if ((bl = map_id2bl(i))){
			if (bl->type == BL_NPC)
				npc_unload((struct npc_data *)bl);
			else if (bl->type&(BL_MOB|BL_PET))//# why BL_PET? [FlavioJS] //# Because this is invoked after saving/wiping all players, which would include all pets. This bit of code will take care of any pets without a master that are still lingering in the map. [Skotlex]
				unit_free(bl, 0);
		}
	}

	ev_db->destroy(ev_db, NULL);
	//There is no free function for npcname_db because at this point there shouldn't be any npcs left!
	//So if there is anything remaining, let the memory manager catch it and report it.
	npcname_db->destroy(npcname_db, NULL);
	ers_destroy(timer_event_ers);
	npc_clearsrcfile();

	return 0;
}

static void npc_debug_warps_sub(struct npc_data *nd)
{
	int m;
	if (nd->bl.type != BL_NPC || nd->bl.subtype != WARP || nd->bl.m < 0)
		return;

	m = map_mapindex2mapid(nd->u.warp.mapindex);
	if (m < 0) return; //Warps to another map, nothing to do about it.

	if (map_getcell(m, nd->u.warp.x, nd->u.warp.y, CELL_CHKNPC)) {
		ShowWarning("Warp %s at %s(%d,%d) warps directly on top of an area npc at %s(%d,%d)\n",
			nd->name,
			map[nd->bl.m].name, nd->bl.x, nd->bl.y,
			map[m].name, nd->u.warp.x, nd->u.warp.y
			);
	}
	if (map_getcell(m, nd->u.warp.x, nd->u.warp.y, CELL_CHKNOPASS)) {
		ShowWarning("Warp %s at %s(%d,%d) warps to a non-walkable tile at %s(%d,%d)\n",
			nd->name,
			map[nd->bl.m].name, nd->bl.x, nd->bl.y,
			map[m].name, nd->u.warp.x, nd->u.warp.y
			);
	}
}

static void npc_debug_warps(void)
{
	int m, i;
	for (m = 0; m < map_num; m++)
		for (i = 0; i < map[m].npc_num; i++)
			npc_debug_warps_sub(map[m].npc[i]);
}

/*==========================================
 * npc初期化
 *------------------------------------------
 */
int do_init_npc(void)
{
	struct npc_src_list *nsl;
	time_t last_time = time(0);
	int busy, i;
	char c = '-';

	//Stock view data for normal npcs.
	malloc_set(&npc_viewdb, 0, sizeof(npc_viewdb));
	npc_viewdb[0].class_ = INVISIBLE_CLASS; //Invisible class is stored here.
	for (busy = 1; busy < MAX_NPC_CLASS; busy++) 
		npc_viewdb[busy].class_ = busy;
	busy = 0;
	// indoorrswtable.txt and etcinfo.txt [Celest]
	if (battle_config.indoors_override_grffile)
		npc_read_indoors();

	// comparing only the first 24 chars of labels that are 50 chars long isn't that nice
	// will cause "duplicated" labels where actually no dup is...
	ev_db = db_alloc(__FILE__,__LINE__,DB_STRING,DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA,51);
	npcname_db = db_alloc(__FILE__,__LINE__,DB_STRING,DB_OPT_BASE,NAME_LENGTH);

	malloc_set(&ev_tm_b, -1, sizeof(ev_tm_b));
	timer_event_ers = ers_new(sizeof(struct timer_event_data));

	for (nsl = npc_src_first; nsl; nsl = nsl->next) {
		npc_parsesrcfile(nsl->name);
		current_file = NULL;
		printf("\r");
		if (script_config.verbose_mode)
			ShowStatus ("Loading NPCs... %-53s", nsl->name);
		else {
			ShowStatus("Loading NPCs... Working: ");
			if (last_time != time(0)) {
				last_time = time(0);
				switch(busy) {
					case 0: c='\\'; busy++; break;
					case 1: c='|'; busy++; break;
					case 2: c='/'; busy++; break;
					case 3: c='-'; busy=0;
				}
			}
			printf("[%c]",c);
		}
		fflush(stdout);
	}
	printf("\r");
	ShowInfo ("Done loading '"CL_WHITE"%d"CL_RESET"' NPCs:%30s\n\t-'"
		CL_WHITE"%d"CL_RESET"' Warps\n\t-'"
		CL_WHITE"%d"CL_RESET"' Shops\n\t-'"
		CL_WHITE"%d"CL_RESET"' Scripts\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Cached\n\t-'"
		CL_WHITE"%d"CL_RESET"' Mobs Not Cached\n",
		npc_id - START_NPC_NUM, "", npc_warp, npc_shop, npc_script, npc_mob, npc_cache_mob, npc_delay_mob);

	malloc_set(script_event, 0, sizeof(script_event));
	npc_read_event_script();
	//Debug function to locate all endless loop warps.
	if (battle_config.warp_point_debug)
		npc_debug_warps();
	
	add_timer_func_list(npc_event_timer,"npc_event_timer");
	add_timer_func_list(npc_event_do_clock,"npc_event_do_clock");
	add_timer_func_list(npc_timerevent,"npc_timerevent");

	// Init dummy NPC
	fake_nd = (struct npc_data *)aCalloc(sizeof(struct npc_data),1);
	fake_nd->bl.prev = fake_nd->bl.next = NULL;
	fake_nd->bl.m = -1;
	fake_nd->bl.x = 0;
	fake_nd->bl.y = 0;
	fake_nd->bl.id = npc_get_new_npc_id();
	fake_nd->class_ = -1;
	fake_nd->speed = 200;
	fake_nd->u.scr.script = NULL;
	fake_nd->u.scr.src_id = 0;
	fake_nd->chatdb = NULL;
	for (i = 0; i < MAX_EVENTTIMER; i++)
		fake_nd->eventtimer[i] = -1;
	strcpy(fake_nd->name,"FAKE_NPC");
	memcpy(fake_nd->exname, fake_nd->name, 9);

	npc_script++;
	fake_nd->bl.type = BL_NPC;
	fake_nd->bl.subtype = SCRIPT;

	strdb_put(npcname_db, fake_nd->exname, fake_nd);
	fake_nd->u.scr.timerid = -1;
	map_addiddb(&fake_nd->bl);
	// End of initialization

	return 0;
}
// [Lance]
int npc_changename(const char *name, const char *newname, short look){
	struct npc_data *nd= (struct npc_data *) strdb_remove(npcname_db,(unsigned char*)name);
	if (nd==NULL)
		return 0;
	npc_enable(name,0);
	strcpy(nd->name,newname);
	nd->class_ = look;
	npc_enable(newname,1);
	return 0;
}
