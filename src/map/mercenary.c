#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"

#include "log.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"
#include "script.h"
#include "npc.h"
#include "trade.h"
#include "unit.h"

#include "mercenary.h"
// Homunculus and future Mercenary system code go here [Celest]
typedef char char32[32];

// everything below is crappy code by [blackhole89].

/*
HLIF_HEAL#Ä¡À¯ÀÇ _Õ+æ(Èú)#
HLIF_AVOID#+ä+ÞÈ¸ÇÇ#
HLIF_BRAIN#_ú_ö_ú#
HLIF_CHANGE#¸àÅ> Ã_ÀÎÁö#
HAMI_CASTLE#Ä___¸÷#
HAMI_DEFENCE#÷ðÆæ_º#
HAMI_SKIN#_Æ_Ù¸¸Æ_¿ò _ºÅ_#
HAMI_BLOODLUST#ºí·¯÷å ·¯_ºÆR#
HFLI_MOON#¹R¶óÀÌÆR#
HFLI_FLEET#ÇÃ¸_ ¹<ºê#
HFLI_SPEED#¿À¹ö÷å _ºÇÇ÷å#
HFLI_SBR44#S.B.R.44#
HVAN_CAPRICE#Ä<ÇÁ¸R_º#
HVAN_CHAOTIC#Ä<¿ÀÆ_ º__×÷ñ_Ç#
HVAN_INSTRUCT#Ã_ÀÎÁö ÀÎ_ºÆR·°_Ç#
HVAN_EXPLOSION#¹ÙÀÌ¿À ÀÍ_ºÇÃ·ÎÁ¯#
*/
char32 merc_skillname[20] = {"NULL","HLIF_HEAL","HLIF_AVOID","HLIF_BRAIN","HLIF_CHANGE",
							"HAMI_CASTLE","HAMI_DEFENCE","HAMI_SKIN","HAMI_BLOODLUST",
							"HFLI_MOON","HFLI_FLEET","HFLI_SPEED","HFLI_SBR44",
							"HVAN_CAPRICE","HVAN_CHAOTIC","HVAN_INSTRUCT","HVAN_EXPLOSION"};

void merc_load_exptables(void);
int mercskill_castend_id( int tid, unsigned int tick, int id,int data );

int do_init_merc (void)
{
	merc_load_exptables();
	return 0;
}

static unsigned long hexptbl[126];

void merc_load_exptables(void)
{
	FILE *fl;
	int i;

	fl=fopen("db/hexptbl.txt","rb");
	if(!fl) return;
	
	ShowInfo("reading db/hexptbl.txt\n");

	for(i=0;i<125;++i)
	{
		fscanf(fl,"%lu,",&(hexptbl[i]));
	}
	fclose(fl);
}

char *merc_skill_get_name(int id)
{
	return merc_skillname[id-HM_SKILLBASE];
}

void merc_damage(struct homun_data *hd,struct block_list *src,int hp,int sp)
{
	clif_homuninfo(hd->master);
}

int merc_dead(struct homun_data *hd, struct block_list *src)
{
	//dead lol
	merc_save(hd);
	return 3; //Remove it from map.
}

void merc_skillup(struct map_session_data *sd,short skillnum)
{
	nullpo_retv(sd);
	nullpo_retv(sd->hd);
	if(!sd->hd->skillpts) return;	//no skill points left

	sd->hd->hskill[(skillnum-HM_SKILLBASE)%4].id=skillnum;
	sd->hd->hskill[(skillnum-HM_SKILLBASE)%4].level+=1;
	sd->hd->skillpts-=1;

	clif_homuninfo(sd);
	clif_homunskillinfoblock(sd);
	clif_skillup(sd, skillnum);

	merc_save(sd->hd);
}

int merc_gainexp(struct homun_data *hd,int exp)
{
	hd->exp += exp;

	if(hd->exp < hd->exp_next)
		return 0;
	  	//levelup
	do
	{
		hd->exp-=hd->exp_next;
		hd->exp_next=hexptbl[hd->level];
		hd->level++;
	}
	while(hd->exp > hd->exp_next);
		
	clif_misceffect(&hd->bl,0);
	status_calc_homunculus(hd,0);
	status_percent_heal(&hd->bl, 100, 100);
	clif_homuninfo(hd->master);
	return 0;
}

void merc_heal(struct homun_data *hd,int hp,int sp)
{
	clif_homuninfo(hd->master);
}

#ifndef TXT_ONLY
void merc_save(struct homun_data *hd)
{
	sprintf(tmp_sql, "UPDATE `homunculus` SET `class`='%d',`name`='%s',`level`='%d',`exp`='%lu',`hunger`='%d',`hp`='%u',`sp`='%u',`skill1lv`='%d',`skill2lv`='%d',`skill3lv`='%d',`skill4lv`='%d',`skillpts`='%d' WHERE `id` = '%d'", 
		hd->class_,hd->name,hd->level,hd->exp,hd->hunger_rate,
		hd->battle_status.hp,hd->battle_status.sp,
		hd->hskill[0].level,hd->hskill[1].level,hd->hskill[2].level,hd->hskill[3].level,
		hd->skillpts,hd->id);
 	if(mysql_query(&mmysql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return;
	}
}
#else
void merc_save(struct homun_data *hd)
{
	//Not implemented...
}
#endif

static void merc_load_sub(struct homun_data *hd, struct map_session_data *sd)
{
	hd->bl.m=sd->bl.m;
	hd->bl.x=sd->bl.x;
	hd->bl.y=sd->bl.y;
	hd->bl.type=BL_HOMUNCULUS;
	hd->bl.id= npc_get_new_npc_id();
	hd->bl.prev=NULL;
	hd->bl.next=NULL;

	status_set_viewdata(&hd->bl, hd->class_);
	status_change_init(&hd->bl);
	unit_dataset(&hd->bl);

	map_addiddb(&hd->bl);
	status_calc_homunculus(hd,1);	//this function will have more sense later on
}
#ifndef TXT_ONLY
void merc_load(struct map_session_data *sd)
{
	struct homun_data *hd;
	sd->hd=NULL;
	
	sprintf(tmp_sql, "SELECT `id`,`class`,`name`,`level`,`exp`,`hunger`,`hp`,`sp`,`skill1lv`,`skill2lv`,`skill3lv`,`skill4lv`,`skillpts` FROM `homunculus` WHERE `char_id` = '%d'", sd->char_id);
	if(mysql_query(&mmysql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return;
	}

	sql_res = mysql_store_result(&mmysql_handle);
	if(!sql_res)
		return;

	if(mysql_num_rows(sql_res) <= 0){
		mysql_free_result(sql_res);
		return;	//no homunculus for this char
	}
	sql_row = mysql_fetch_row(sql_res);
	
	//dummy code
	hd=(struct homun_data *) aCalloc(1, sizeof(struct homun_data));
	sd->hd=hd;		//pointer from master to homunculus
	memset(hd,0,sizeof(struct homun_data));
	hd->master=sd;	//pointer from homunculus to master
	hd->id=atoi(sql_row[0]);
	hd->class_=atoi(sql_row[1]);
	hd->level=atoi(sql_row[3]);
	hd->battle_status.hp=atoi(sql_row[6]);
	hd->battle_status.sp=atoi(sql_row[7]);
	hd->exp=atoi(sql_row[4]);
	hd->hunger_rate=atoi(sql_row[5]);
	hd->hskill[0].level=atoi(sql_row[8]);
	hd->hskill[1].level=atoi(sql_row[9]);
	hd->hskill[2].level=atoi(sql_row[10]);
	hd->hskill[3].level=atoi(sql_row[11]);
	hd->skillpts=atoi(sql_row[12]);
	hd->exp_next=hexptbl[hd->level-1];
	strncpy(hd->name,sql_row[2],NAME_LENGTH);
	mysql_free_result(sql_res);
	merc_load_sub(hd, sd);
}
#else 
void merc_load(struct map_session_data *sd)
{
	struct homun_data *hd;
	int id,charid,class_,level,exp,hunger,hp,sp;
	char name[24];
	FILE *fl=fopen("save/homunculus.txt","r");
	sd->hd=NULL;

	if(!fl) return; //Unable to open file.	
	ShowInfo("Looking up Homunculus for %d...\n",sd->char_id);
	do {
		fscanf(fl,"%d,%d,%d,%s ,%d,%d,%d,%d,%d\n",&id,&charid,&class_,name,&level,&exp,&hunger,&hp,&sp);
		ShowInfo("%d",charid);
		if(charid==sd->char_id) break;
	} while(charid!=0);
	if (!charid)
		return;	//none found
	ShowInfo("found it!\n");
	
	//dummy code
	hd=(struct homun_data *) aCalloc(1, sizeof(struct homun_data));
	sd->hd=hd;		//pointer from master to homunculus
	memset(hd,0,sizeof(struct homun_data));
	hd->master=sd;	//pointer from homunculus to master
	hd->id=id;
	hd->class_=class_;
	hd->level=level;
	hd->exp=exp;
	hd->hunger_rate=hunger;
	hd->battle_status.hp=hp;
	hd->battle_status.sp=sp;
	hd->exp_next=hexptbl[hd->level-1];
	strncpy(hd->name,name,NAME_LENGTH);
	merc_load_sub(hd, sd);
}
#endif	

int merc_create_homunculus(struct map_session_data *sd,int id,int m,int x,int y)
{
/*	struct homun_data *hd;
	//dummy code
	hd=(struct homun_data *) aCalloc(1, sizeof(struct homun_data));
	sd->hd=hd;		//pointer from master to homunculus
	memset(hd,0,sizeof(struct homun_data));
	hd->master=sd;	//pointer from homunculus to master
	hd->class_=id;
	hd->speed=0x96;
	hd->level=1;
	hd->bl.m=m;
	hd->bl.x=hd->to_x=x;
	hd->bl.y=hd->to_y=y;
	hd->to_x+=2;
	hd->bl.type=BL_HOMUNCULUS;
	hd->bl.id= npc_get_new_npc_id();
	hd->bl.prev=NULL;
	hd->bl.next=NULL;
	map_addiddb(&hd->bl);
	hd->max_hp=500;
	hd->hp=400;
	hd->max_sp=300;
	hd->sp=200;
	hd->atk=15;
	hd->matk=2;
	hd->hit=3;
	hd->crit=90;
	hd->def=5;
	hd->mdef=6;
	hd->flee=7;
	hd->exp=10;
	hd->exp_next=100;
	hd->hunger_rate=32;
	hd->walktimer=-1;
	memcpy(hd->name,"Homunculus\0",11);
	merc_calc_stats(hd);
	hd->attackabletime=0;

	merc_save(hd);

	clif_spawnhomun(hd);
	clif_homunack(sd);
	clif_homuninfo(sd);
	clif_homuninfo(sd);*/ // send this x2. dunno why, but kRO does that [blackhole89]
	return 0;
}

int do_final_merc (void);
