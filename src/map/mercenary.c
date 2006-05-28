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
#include "../common/core.h"
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
void merc_save(struct map_session_data *sd);
int mercskill_castend_id( int tid, unsigned int tick, int id,int data );

int do_init_merc (void)
{
	merc_load_exptables();
	return 0;
}

static int dirx[8]={0,-1,-1,-1,0,1,1,1};
static int diry[8]={1,1,0,-1,-1,-1,0,1};



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
		fscanf(fl,"%u,",&(hexptbl[i]));
	}
	fclose(fl);
}

char *merc_skill_get_name(int id)
{
	return merc_skillname[id-8000];
}

void merc_die(struct map_session_data *sd)
{
	if(sd->hd)
	{
		sd->hd->alive=0;
		merc_save(sd);
		clif_clearchar_area(&sd->hd->bl,0);
		map_delblock(&sd->hd->bl);	
	}
}

int merc_damage(struct block_list *src,struct homun_data *hd,int damage,int type)
{
	if(damage > hd->hp) damage = hd->hp;
	hd->hp -= damage;

	clif_homuninfo(hd->master);

	if(hd->hp == 0)
	{
		//dead lol
		clif_clearchar((struct block_list*)hd,1);
		hd->bl.m = 0;
		hd->bl.x = 0;
		hd->bl.y = 0;	//send it somewhere where it doesn't bother us
		merc_save(hd->master);
		merc_die(hd->master);
		return damage;
	}

	merc_save(hd->master);
	return damage;
}

void merc_calc_status(struct homun_data *hd)
{
	int dstr; 
	int atk_rate=100,aspd_rate=100,speed_rate=100;

	// attack
	dstr = hd->str/10;
	hd->atk = hd->str + dstr*dstr + hd->dex/5;

	// matk
	hd->matk = hd->int_+(hd->int_/6)*(hd->int_/6);
	
	// crit
	hd->crit = (hd->luk*3)+10; //x.1

	// hit
	hd->hit = hd->dex + hd->level;

	// flee
	hd->flee = hd->agi + hd->level;

	// lucky flee
	hd->flee2 = hd->luk+10;	//x.1

	// def
	hd->def = hd->vit;

	// mdef
	hd->mdef = hd->int_;

	// hp recovery
	hd->regenhp = 1 + (hd->vit/5) + (hd->max_hp/200);

	// sp recovery
	hd->regensp = 1 + (hd->int_/6) + (hd->max_sp/100);
	if(hd->int_ >= 120)
		hd->regensp += ((hd->int_-120)>>1) + 4;

	if(hd->sc_count && hd->sc.data)
	{

		if(hd->sc.data[SC_AVOID].timer!=-1)
			speed_rate -= hd->sc.data[SC_AVOID].val1*10;
	
		if(hd->sc.data[SC_CHANGE].timer!=-1)
			hd->int_ += 60;
		
		if(hd->sc.data[SC_BLOODLUST].timer!=-1)
			atk_rate += hd->sc.data[SC_BLOODLUST].val1*10+20;

		if(hd->sc.data[SC_FLEET].timer!=-1)
		{
			aspd_rate -= hd->sc.data[SC_FLEET].val1*3;
			atk_rate+=5+hd->sc.data[SC_FLEET].val1*5;
		}
	}

	hd->amotion	= 1800 - (1800 * hd->agi / 250 + 1800 * hd->dex / 1000);
	hd->amotion	-= 200;
	hd->dmotion=hd->amotion;
}

void merc_skillup(struct map_session_data *sd,short skillnum)
{
	nullpo_retv(sd);
	nullpo_retv(sd->hd);
	if(!sd->hd->skillpts) return;	//no skill points left

	sd->hd->hskill[(skillnum-8001)%4].id=skillnum;
	sd->hd->hskill[(skillnum-8001)%4].level+=1;
	sd->hd->skillpts-=1;

	clif_homuninfo(sd);
	clif_homunskillinfoblock(sd);
	clif_skillup(sd, skillnum);

	merc_save(sd);
}

void merc_calc_stats(struct homun_data *hd)
{
	/* very proprietary */
	int l,i;
	l=hd->level;
	hd->max_hp=500+l*10+l*l;
	hd->max_sp=300+l*11+l*l*90/100;
/*	hd->atk=10+l*5+l*l/125;
	hd->matk=6+l*6+l*l/120;
	hd->hit=3+l+l*l/200;
	hd->crit=1+l*69/125;
	hd->def=5+l+l*l/119;
	hd->mdef=1+l+l*l/80;
	hd->flee=7+l+l*l/150; */ // obsolete
	switch(hd->class_)
	{
	case 6001:	//LIF ~ int,dex,vit
		hd->str = 3+l/7;
		hd->agi = 3+2*l/5;
		hd->vit = 4+l;
		hd->int_ = 4+3*l/4;
		hd->dex = 4+2*l/3;
		hd->luk = 3+l/4;
		for(i=8001;i<8005;++i)
		{
			hd->hskill[i-8001].id=i;
			//hd->hskill[i-8001].level=1;
		}
		break;
	case 6003:	//FILIR ~ str,agi,dex
		hd->str = 4+3*l/4;
		hd->agi = 4+2*l/3;
		hd->vit = 3+2*l/5;
		hd->int_ = 3+l/4;
		hd->dex = 4+l;
		hd->luk = 3+l/7;
		for(i=8009;i<8013;++i)
		{
			hd->hskill[i-8009].id=i;
			//hd->hskill[i-8009].level=1;
		}
		break;
	case 6002:	//AMISTR ~ str,vit,luk
		hd->str = 4+l;
		hd->agi = 3+l/4;
		hd->vit = 3+3*l/4;
		hd->int_ = 3+1/10;
		hd->dex = 3+2*l/5;
		hd->luk = 4+2*l/3;
		for(i=8005;i<8009;++i)
		{
			hd->hskill[i-8005].id=i;
			//hd->hskill[i-8005].level=1;
		}
		break;
	case 6004:	//VANILMIRTH ~ int,dex,luk
		hd->str = 3+l/4;
		hd->agi = 3+l/7;
		hd->vit = 3+2*l/5;
		hd->int_ = 4+l;
		hd->dex = 4+2*l/3;
		hd->luk = 4+3*l/4;
		for(i=8013;i<8017;++i)
		{
			hd->hskill[i-8013].id=i;
			//hd->hskill[i-8013].level=1;
		}
		break;
	}
	merc_calc_status(hd);
	hd->exp_next=hexptbl[l-1];
}

int merc_gainexp(struct homun_data *hd,int exp)
{
	hd->exp += exp;

	if(hd->exp > hd->exp_next) //levelup
	{
		while(hd->exp > hd->exp_next)
		{
			hd->exp-=hd->exp_next;
			hd->level += 1;
			
			clif_misceffect(&hd->bl,0);
			merc_calc_stats(hd);
			hd->hp=hd->max_hp;
			hd->sp=hd->max_sp;
		}
	}

	merc_save(hd->master);
	clif_homuninfo(hd->master);
	return 0;
}

int merc_heal(struct homun_data *hd,int hp,int sp)
{
	hd->hp+=hp;
	hd->sp+=sp;
	if(hd->max_hp < hd->hp) hd->hp = hd->max_hp;
	if(hd->max_sp < hd->sp) hd->sp = hd->max_sp;

	merc_save(hd->master);
	clif_homuninfo(hd->master);

	return hp+sp;
}

void merc_save(struct map_session_data *sd)
{
#ifndef TXT_ONLY
	sprintf(tmp_sql, "UPDATE `homunculus` SET `class`='%d',`name`='%s',`level`='%d',`exp`='%d',`hunger`='%d',`hp`='%d',`sp`='%d',`skill1lv`='%d',`skill2lv`='%d',`skill3lv`='%d',`skill4lv`='%d',`skillpts`='%d' WHERE `id` = '%d'", 
		sd->hd->class_,sd->hd->name,sd->hd->level,sd->hd->exp,sd->hd->hunger_rate,sd->hd->hp,sd->hd->sp,
		sd->hd->hskill[0].level,sd->hd->hskill[1].level,sd->hd->hskill[2].level,sd->hd->hskill[3].level,
		sd->hd->skillpts,sd->hd->id);
    if(mysql_query(&mmysql_handle, tmp_sql)){
			ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return;
	}
#endif
}

//not nice, but works
void merc_res(struct map_session_data *sd,int skilllv)
{
	sprintf(tmp_sql, "UPDATE `homunculus` SET `hp`='10' WHERE `char_id` = '%d' AND `hp`=0",sd->char_id);
	if(mysql_query(&mmysql_handle, tmp_sql)){
			ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return;
	}
	if(sd->hd)
	{
		sd->hd->hp=skilllv*sd->hd->max_hp/10;
		sd->hd->alive=1;
	}
}

void merc_load(struct map_session_data *sd)
{
	struct homun_data *hd;
	int alive=1;

	sd->hd=NULL;

#ifndef TXT_ONLY
	sprintf(tmp_sql, "SELECT `id`,`class`,`name`,`level`,`exp`,`hunger`,`hp`,`sp`,`skill1lv`,`skill2lv`,`skill3lv`,`skill4lv`,`skillpts` FROM `homunculus` WHERE `char_id` = '%d'", sd->char_id);
    if(mysql_query(&mmysql_handle, tmp_sql)){
			ShowSQL("DB error - %s\n",mysql_error(&mmysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return;
	}

	sql_res = mysql_store_result(&mmysql_handle);
	if(mysql_num_rows(sql_res) <= 0){
		mysql_free_result(sql_res);
		return;	//no homunculus for this char
	}

	sql_row = mysql_fetch_row(sql_res);
	if(atoi(sql_row[6])==0) alive=0; //it is dead
#else
	int id,charid,class_,level,exp,hunger,hp,sp;
	char name[24];
	FILE *fl=fopen("save/homunculus.txt","r");
	
	ShowInfo("Looking up Homunculus for %d...\n",sd->char_id);
	charid=1;
	while(charid!=0)
	{
		fscanf(fl,"%d,%d,%d,%s ,%d,%d,%d,%d,%d\n",&id,&charid,&class_,name,&level,&exp,&hunger,&hp,&sp);
		ShowInfo("%d",charid);
		if(charid==sd->char_id) goto gotit;
	}
	return;	//none found
gotit:
	ShowInfo("found it!\n");
#endif
	
	//dummy code
	hd=(struct homun_data *) aCalloc(1, sizeof(struct homun_data));
	sd->hd=hd;		//pointer from master to homunculus
	memset(hd,0,sizeof(struct homun_data));
	hd->master=sd;	//pointer from homunculus to master
#ifndef TXT_ONLY
	hd->id=atoi(sql_row[0]);
	hd->class_=atoi(sql_row[1]);
	hd->level=atoi(sql_row[3]);
	hd->hp=atoi(sql_row[6]);
	hd->sp=atoi(sql_row[7]);
	hd->exp=atoi(sql_row[4]);
	hd->hunger_rate=atoi(sql_row[5]);
	hd->hskill[0].level=atoi(sql_row[8]);
	hd->hskill[1].level=atoi(sql_row[9]);
	hd->hskill[2].level=atoi(sql_row[10]);
	hd->hskill[3].level=atoi(sql_row[11]);
	hd->skillpts=atoi(sql_row[12]);
	memcpy(hd->name,sql_row[2],strlen(sql_row[2])<24?strlen(sql_row[2]):24);
#else
	hd->id=id;
	hd->class_=class_;
	hd->level=level;
	hd->exp=exp;
	hd->hunger_rate=hunger;
	hd->hp=hp;
	hd->sp=sp;
	memcpy(hd->name,name,strlen(name)<24?strlen(name):24);
#endif
	hd->alive=alive;
	
	hd->speed=0x96;
	
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

	merc_calc_stats(hd);	//this function will have more sense later on

	mysql_free_result(sql_res);
	
//clif_spawnhomun(hd);
//	clif_homunack(sd);
//	clif_homuninfo(sd);
//	clif_homuninfo(sd); // send this x2. dunno why, but kRO does that [blackhole89]
}

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

	merc_save(hd->master);

	clif_spawnhomun(hd);
	clif_homunack(sd);
	clif_homuninfo(sd);
	clif_homuninfo(sd);*/ // send this x2. dunno why, but kRO does that [blackhole89]
}

int do_final_merc (void);
