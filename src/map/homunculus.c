// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"

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

#include "homunculus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


//Better equiprobability than rand()% [orn]
#define rand(a, b) (a+(int) ((float)(b-a+1)*rand()/(RAND_MAX+1.0)))

struct s_homunculus_db homunculus_db[MAX_HOMUNCULUS_CLASS];	//[orn]
struct skill_tree_entry hskill_tree[MAX_HOMUNCULUS_CLASS][MAX_SKILL_TREE];

static int merc_hom_hungry(int tid, unsigned int tick, int id, intptr_t data);

static unsigned int hexptbl[MAX_LEVEL];

//For holding the view data of npc classes. [Skotlex]
static struct view_data hom_viewdb[MAX_HOMUNCULUS_CLASS];

struct view_data* merc_get_hom_viewdata(int class_)
{	//Returns the viewdata for homunculus
	if (homdb_checkid(class_))
		return &hom_viewdb[class_-HM_CLASS_BASE];
	return NULL;
}

void merc_damage(struct homun_data *hd,struct block_list *src,int hp,int sp)
{
	clif_hominfo(hd->master,hd,0);
}

int merc_hom_dead(struct homun_data *hd, struct block_list *src)
{
	//There's no intimacy penalties on death (from Tharis)
	struct map_session_data *sd = hd->master;

	clif_emotion(&hd->bl, E_WAH);

	//Delete timers when dead.
	merc_hom_hungry_timer_delete(hd);
	hd->homunculus.hp = 0;

	if (!sd) //unit remove map will invoke unit free
		return 3;

	clif_emotion(&sd->bl, E_SOB);
	//Remove from map (if it has no intimacy, it is auto-removed from memory)
	return 3;
}

//Vaporize a character's homun. If flag, HP needs to be 80% or above.
int merc_hom_vaporize(struct map_session_data *sd, int flag)
{
	struct homun_data *hd;

	nullpo_ret(sd);

	hd = sd->hd;
	if (!hd || hd->homunculus.vaporize)
		return 0;
	
	if (status_isdead(&hd->bl))
		return 0; //Can't vaporize a dead homun.

	if (flag && get_percentage(hd->battle_status.hp, hd->battle_status.max_hp) < 80)
		return 0;

	hd->regen.state.block = 3; //Block regen while vaporized.
	//Delete timers when vaporized.
	merc_hom_hungry_timer_delete(hd);
	hd->homunculus.vaporize = 1;
	if(battle_config.hom_setting&0x40)
		memset(hd->blockskill, 0, sizeof(hd->blockskill));
	clif_hominfo(sd, sd->hd, 0);
	merc_save(hd);
	return unit_remove_map(&hd->bl, CLR_OUTSIGHT);
}

//delete a homunculus, completely "killing it". 
//Emote is the emotion the master should use, send negative to disable.
int merc_hom_delete(struct homun_data *hd, int emote)
{
	struct map_session_data *sd;
	nullpo_ret(hd);
	sd = hd->master;

	if (!sd)
		return unit_free(&hd->bl,CLR_DEAD);

	if (emote >= 0)
		clif_emotion(&sd->bl, emote);

	//This makes it be deleted right away.
	hd->homunculus.intimacy = 0;
	// Send homunculus_dead to client
	hd->homunculus.hp = 0;
	clif_hominfo(sd, hd, 0);
	return unit_remove_map(&hd->bl,CLR_OUTSIGHT);
}

int merc_hom_calc_skilltree(struct homun_data *hd)
{
	int i,id=0 ;
	int j,f=1;
	int c=0;

	nullpo_ret(hd);
	c = hd->homunculus.class_ - HM_CLASS_BASE;
	
	for(i=0;i < MAX_SKILL_TREE && (id = hskill_tree[c][i].id) > 0;i++)
	{
		if(hd->homunculus.hskill[id-HM_SKILLBASE].id)
			continue; //Skill already known.
		if(!battle_config.skillfree)
		{
			for(j=0;j<MAX_PC_SKILL_REQUIRE;j++)
			{
				if( hskill_tree[c][i].need[j].id &&
					merc_hom_checkskill(hd,hskill_tree[c][i].need[j].id) < hskill_tree[c][i].need[j].lv) 
				{
					f=0;
					break;
				}
			}
		}
		if (f)
			hd->homunculus.hskill[id-HM_SKILLBASE].id = id ;
	}
	return 0;
}

int merc_hom_checkskill(struct homun_data *hd,int skill_id)
{
	int i = skill_id - HM_SKILLBASE;
	if(!hd)
		return 0;

	if(hd->homunculus.hskill[i].id == skill_id)
		return (hd->homunculus.hskill[i].lv);

	return 0;
}

int merc_skill_tree_get_max(int id, int b_class){
	int i, skillid;
	b_class -= HM_CLASS_BASE;
	for(i=0;(skillid=hskill_tree[b_class][i].id)>0;i++)
		if (id == skillid)
			return hskill_tree[b_class][i].max;
	return skill_get_max(id);
}

void merc_hom_skillup(struct homun_data *hd,int skillnum)
{
	int i = 0 ;
	nullpo_retv(hd);

	if(hd->homunculus.vaporize)
		return;
	
	i = skillnum - HM_SKILLBASE;
	if(hd->homunculus.skillpts > 0 &&
		hd->homunculus.hskill[i].id &&
		hd->homunculus.hskill[i].flag == SKILL_FLAG_PERMANENT && //Don't allow raising while you have granted skills. [Skotlex]
		hd->homunculus.hskill[i].lv < merc_skill_tree_get_max(skillnum, hd->homunculus.class_)
		)
	{
		hd->homunculus.hskill[i].lv++;
		hd->homunculus.skillpts-- ;
		status_calc_homunculus(hd,0);
		if (hd->master) {
			clif_homskillup(hd->master, skillnum);
			clif_hominfo(hd->master,hd,0);
			clif_homskillinfoblock(hd->master);
		}
	}
}

int merc_hom_levelup(struct homun_data *hd)
{
	struct s_homunculus *hom;
	struct h_stats *min, *max;
	int growth_str, growth_agi, growth_vit, growth_int, growth_dex, growth_luk ;
	int growth_max_hp, growth_max_sp ;
	char output[256] ;

	if (hd->homunculus.level == MAX_LEVEL || !hd->exp_next || hd->homunculus.exp < hd->exp_next)
		return 0 ;
	
	hom = &hd->homunculus;
	hom->level++ ;
	if (!(hom->level % 3)) 
		hom->skillpts++ ;	//1 skillpoint each 3 base level

	hom->exp -= hd->exp_next ;
	hd->exp_next = hexptbl[hom->level - 1] ;
	
	max  = &hd->homunculusDB->gmax;
	min  = &hd->homunculusDB->gmin;

	growth_max_hp = rand(min->HP, max->HP);
	growth_max_sp = rand(min->SP, max->SP);
	growth_str = rand(min->str, max->str);
	growth_agi = rand(min->agi, max->agi);
	growth_vit = rand(min->vit, max->vit);
	growth_dex = rand(min->dex, max->dex);
	growth_int = rand(min->int_,max->int_);
	growth_luk = rand(min->luk, max->luk);

	//Aegis discards the decimals in the stat growth values!
	growth_str-=growth_str%10;
	growth_agi-=growth_agi%10;
	growth_vit-=growth_vit%10;
	growth_dex-=growth_dex%10;
	growth_int-=growth_int%10;
	growth_luk-=growth_luk%10;

	hom->max_hp += growth_max_hp;
	hom->max_sp += growth_max_sp;
	hom->str += growth_str;
	hom->agi += growth_agi;
	hom->vit += growth_vit;
	hom->dex += growth_dex;
	hom->int_+= growth_int;
	hom->luk += growth_luk;
	
	if ( battle_config.homunculus_show_growth ) {
		sprintf(output,
			"Growth: hp:%d sp:%d str(%.2f) agi(%.2f) vit(%.2f) int(%.2f) dex(%.2f) luk(%.2f) ",
			growth_max_hp, growth_max_sp,
			growth_str/10.0, growth_agi/10.0, growth_vit/10.0,
			growth_int/10.0, growth_dex/10.0, growth_luk/10.0);
		clif_disp_onlyself(hd->master,output,strlen(output));
	}
	return 1 ;
}

int merc_hom_change_class(struct homun_data *hd, short class_)
{
	int i;
	i = search_homunculusDB_index(class_,HOMUNCULUS_CLASS);
	if(i < 0)
		return 0;
	hd->homunculusDB = &homunculus_db[i];
	hd->homunculus.class_ = class_;
	status_set_viewdata(&hd->bl, class_);
	merc_hom_calc_skilltree(hd);
	return 1;
}

int merc_hom_evolution(struct homun_data *hd)
{
	struct s_homunculus *hom;
	struct h_stats *max, *min;
	struct map_session_data *sd;
	nullpo_ret(hd);

	if(!hd->homunculusDB->evo_class || hd->homunculus.class_ == hd->homunculusDB->evo_class)
	{
		clif_emotion(&hd->bl, E_SWT);
		return 0 ;
	}
	sd = hd->master;
	if (!sd)
		return 0;
	
	if (!merc_hom_change_class(hd, hd->homunculusDB->evo_class)) {
		ShowError("merc_hom_evolution: Can't evolve homunc from %d to %d", hd->homunculus.class_, hd->homunculusDB->evo_class);
		return 0;
	}

	//Apply evolution bonuses
	hom = &hd->homunculus;
	max = &hd->homunculusDB->emax;
	min = &hd->homunculusDB->emin;
	hom->max_hp += rand(min->HP, max->HP);
	hom->max_sp += rand(min->SP, max->SP);
	hom->str += 10*rand(min->str, max->str);
	hom->agi += 10*rand(min->agi, max->agi);
	hom->vit += 10*rand(min->vit, max->vit);
	hom->int_+= 10*rand(min->int_,max->int_);
	hom->dex += 10*rand(min->dex, max->dex);
	hom->luk += 10*rand(min->luk, max->luk);
	hom->intimacy = 500;

	unit_remove_map(&hd->bl, CLR_OUTSIGHT);
	map_addblock(&hd->bl);

	clif_spawn(&hd->bl);
	clif_emotion(&sd->bl, E_NO1);
	clif_specialeffect(&hd->bl,568,AREA);

	//status_Calc flag&1 will make current HP/SP be reloaded from hom structure
	hom->hp = hd->battle_status.hp;
	hom->sp = hd->battle_status.sp;
	status_calc_homunculus(hd,1);

	if (!(battle_config.hom_setting&0x2))
		skill_unit_move(&sd->hd->bl,gettick(),1); // apply land skills immediately
				
	return 1 ;
}

int merc_hom_gainexp(struct homun_data *hd,int exp)
{
	if(hd->homunculus.vaporize)
		return 1;

	if( hd->exp_next == 0 ) {
		hd->homunculus.exp = 0 ;
		return 0;
	}

	hd->homunculus.exp += exp;

	if(hd->homunculus.exp < hd->exp_next) {
		clif_hominfo(hd->master,hd,0);
		return 0;
	}

 	//levelup
	do
	{
		merc_hom_levelup(hd) ;
	}
	while(hd->homunculus.exp > hd->exp_next && hd->exp_next != 0 );
		
	if( hd->exp_next == 0 )
		hd->homunculus.exp = 0 ;

	clif_specialeffect(&hd->bl,568,AREA);
	status_calc_homunculus(hd,0);
	status_percent_heal(&hd->bl, 100, 100);
	return 0;
}

// Return the new value
int merc_hom_increase_intimacy(struct homun_data * hd, unsigned int value)
{
	if (battle_config.homunculus_friendly_rate != 100)
		value = (value * battle_config.homunculus_friendly_rate) / 100;

	if (hd->homunculus.intimacy + value <= 100000)
		hd->homunculus.intimacy += value;
	else
		hd->homunculus.intimacy = 100000;
	return hd->homunculus.intimacy;
}

// Return 0 if decrease fails or intimacy became 0 else the new value
int merc_hom_decrease_intimacy(struct homun_data * hd, unsigned int value)
{
	if (hd->homunculus.intimacy >= value)
		hd->homunculus.intimacy -= value;
	else
		hd->homunculus.intimacy = 0;

	return hd->homunculus.intimacy;
}

void merc_hom_heal(struct homun_data *hd,int hp,int sp)
{
	clif_hominfo(hd->master,hd,0);
}

void merc_save(struct homun_data *hd)
{
	// copy data that must be saved in homunculus struct ( hp / sp )
	TBL_PC * sd = hd->master;
	//Do not check for max_hp/max_sp caps as current could be higher to max due
	//to status changes/skills (they will be capped as needed upon stat 
	//calculation on login)
	hd->homunculus.hp = hd->battle_status.hp;
	hd->homunculus.sp = hd->battle_status.sp;
	intif_homunculus_requestsave(sd->status.account_id, &hd->homunculus);
}

int merc_menu(struct map_session_data *sd,int menunum)
{
	nullpo_ret(sd);
	if (sd->hd == NULL)
		return 1;
	
	switch(menunum) {
		case 0:
			break;
		case 1:
			merc_hom_food(sd, sd->hd);
			break;
		case 2:
			merc_hom_delete(sd->hd, -1);
			break;
		default:
			ShowError("merc_menu : unknown menu choice : %d\n", menunum) ;
			break;
	}
	return 0;
}

int merc_hom_food(struct map_session_data *sd, struct homun_data *hd)
{
	int i, foodID, emotion;

	if(hd->homunculus.vaporize)
		return 1 ;

	foodID = hd->homunculusDB->foodID;
	i = pc_search_inventory(sd,foodID);
	if(i < 0) {
		clif_hom_food(sd,foodID,0);
		return 1;
	}
	pc_delitem(sd,i,1,0,0,LOG_TYPE_CONSUME);

	if ( hd->homunculus.hunger >= 91 ) {
		merc_hom_decrease_intimacy(hd, 50);
		emotion = E_WAH;
	} else if ( hd->homunculus.hunger >= 76 ) {
		merc_hom_decrease_intimacy(hd, 5);
		emotion = E_SWT2;
	} else if ( hd->homunculus.hunger >= 26 ) {
		merc_hom_increase_intimacy(hd, 75);
		emotion = E_HO;
	} else if ( hd->homunculus.hunger >= 11 ) {
		merc_hom_increase_intimacy(hd, 100);
		emotion = E_HO;
	} else {
		merc_hom_increase_intimacy(hd, 50);
		emotion = E_HO;
	}

	hd->homunculus.hunger += 10;	//dunno increase value for each food
	if(hd->homunculus.hunger > 100)
		hd->homunculus.hunger = 100;

	clif_emotion(&hd->bl,emotion);
	clif_send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	clif_send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	clif_hom_food(sd,foodID,1);
       	
	// Too much food :/
	if(hd->homunculus.intimacy == 0)
		return merc_hom_delete(sd->hd, E_OMG);

	return 0;
}

static int merc_hom_hungry(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd;
	struct homun_data *hd;

	sd=map_id2sd(id);
	if(!sd)
		return 1;

	if(!sd->status.hom_id || !(hd=sd->hd))
		return 1;

	if(hd->hungry_timer != tid){
		ShowError("merc_hom_hungry_timer %d != %d\n",hd->hungry_timer,tid);
		return 0;
	}

	hd->hungry_timer = INVALID_TIMER;
	
	hd->homunculus.hunger-- ;
	if(hd->homunculus.hunger <= 10) {
		clif_emotion(&hd->bl, E_AN);
	} else if(hd->homunculus.hunger == 25) {
		clif_emotion(&hd->bl, E_HMM);
	} else if(hd->homunculus.hunger == 75) {
		clif_emotion(&hd->bl, E_OK);
	}  
	
	if(hd->homunculus.hunger < 0) {
		hd->homunculus.hunger = 0;
		// Delete the homunculus if intimacy <= 100
		if ( !merc_hom_decrease_intimacy(hd, 100) )
			return merc_hom_delete(hd, E_OMG);
		clif_send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	}

	clif_send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	hd->hungry_timer = add_timer(tick+hd->homunculusDB->hungryDelay,merc_hom_hungry,sd->bl.id,0); //simple Fix albator
	return 0;
}

int merc_hom_hungry_timer_delete(struct homun_data *hd)
{
	nullpo_ret(hd);
	if(hd->hungry_timer != INVALID_TIMER) {
		delete_timer(hd->hungry_timer,merc_hom_hungry);
		hd->hungry_timer = INVALID_TIMER;
	}
	return 1;
}

int merc_hom_change_name(struct map_session_data *sd,char *name)
{
	int i;
	struct homun_data *hd;
	nullpo_retr(1, sd);

	hd = sd->hd;
	if (!merc_is_hom_active(hd))
		return 1;
	if(hd->homunculus.rename_flag && !battle_config.hom_rename)
		return 1;

	for(i=0;i<NAME_LENGTH && name[i];i++){
		if( !(name[i]&0xe0) || name[i]==0x7f)
			return 1;
	}

	return intif_rename_hom(sd, name);
}

int merc_hom_change_name_ack(struct map_session_data *sd, char* name, int flag)
{
	struct homun_data *hd = sd->hd;
	if (!merc_is_hom_active(hd)) return 0;

	normalize_name(name," ");//bugreport:3032
	
	if ( !flag || !strlen(name) ) {
		clif_displaymessage(sd->fd, msg_txt(280)); // You cannot use this name
		return 0;
	}
	strncpy(hd->homunculus.name,name,NAME_LENGTH);
	clif_charnameack (0,&hd->bl);
	hd->homunculus.rename_flag = 1;
	clif_hominfo(sd,hd,0);
	return 1;
}

int search_homunculusDB_index(int key,int type)
{
	int i;

	for(i=0;i<MAX_HOMUNCULUS_CLASS;i++) {
		if(homunculus_db[i].base_class <= 0)
			continue;
		switch(type) {
			case HOMUNCULUS_CLASS:
				if(homunculus_db[i].base_class == key ||
					homunculus_db[i].evo_class == key)
					return i;
				break;
			case HOMUNCULUS_FOOD:
				if(homunculus_db[i].foodID == key)
					return i;
				break;
			default:
				return -1;
		}
	}
	return -1;
}

// Create homunc structure
int merc_hom_alloc(struct map_session_data *sd, struct s_homunculus *hom)
{
	struct homun_data *hd;
	int i = 0;

	nullpo_retr(1, sd);

	Assert((sd->status.hom_id == 0 || sd->hd == 0) || sd->hd->master == sd); 

	i = search_homunculusDB_index(hom->class_,HOMUNCULUS_CLASS);
	if(i < 0) {
		ShowError("merc_hom_alloc: unknown class [%d] for homunculus '%s', requesting deletion.\n", hom->class_, hom->name);
		sd->status.hom_id = 0;
		intif_homunculus_requestdelete(hom->hom_id);
		return 1;
	}
	sd->hd = hd = (struct homun_data*)aCalloc(1,sizeof(struct homun_data));
	hd->bl.type = BL_HOM;
	hd->bl.id = npc_get_new_npc_id();

	hd->master = sd;
	hd->homunculusDB = &homunculus_db[i];
	memcpy(&hd->homunculus, hom, sizeof(struct s_homunculus));
	hd->exp_next = hexptbl[hd->homunculus.level - 1];

	status_set_viewdata(&hd->bl, hd->homunculus.class_);
	status_change_init(&hd->bl);
	unit_dataset(&hd->bl);
	hd->ud.dir = sd->ud.dir;

	// Find a random valid pos around the player
	hd->bl.m = sd->bl.m;
	hd->bl.x = sd->bl.x;
	hd->bl.y = sd->bl.y;
	unit_calc_pos(&hd->bl, sd->bl.x, sd->bl.y, sd->ud.dir);
	hd->bl.x = hd->ud.to_x;
	hd->bl.y = hd->ud.to_y;
	
	map_addiddb(&hd->bl);
	status_calc_homunculus(hd,1);

	hd->hungry_timer = INVALID_TIMER;
	return 0;
}

void merc_hom_init_timers(struct homun_data * hd)
{
	if (hd->hungry_timer == INVALID_TIMER)
		hd->hungry_timer = add_timer(gettick()+hd->homunculusDB->hungryDelay,merc_hom_hungry,hd->master->bl.id,0);
	hd->regen.state.block = 0; //Restore HP/SP block.
}

int merc_call_homunculus(struct map_session_data *sd)
{
	struct homun_data *hd;

	if (!sd->status.hom_id) //Create a new homun.
		return merc_create_homunculus_request(sd, HM_CLASS_BASE + rand(0, 7)) ;

	// If homunc not yet loaded, load it
	if (!sd->hd)
		return intif_homunculus_requestload(sd->status.account_id, sd->status.hom_id);

	hd = sd->hd;

	if (!hd->homunculus.vaporize)
		return 0; //Can't use this if homun wasn't vaporized.

	merc_hom_init_timers(hd);
	hd->homunculus.vaporize = 0;
	if (hd->bl.prev == NULL)
	{	//Spawn him
		hd->bl.x = sd->bl.x;
		hd->bl.y = sd->bl.y;
		hd->bl.m = sd->bl.m;
		map_addblock(&hd->bl);
		clif_spawn(&hd->bl);
		clif_send_homdata(sd,SP_ACK,0);
		clif_hominfo(sd,hd,1);
		clif_hominfo(sd,hd,0); // send this x2. dunno why, but kRO does that [blackhole89]
		clif_homskillinfoblock(sd);
		if (battle_config.slaves_inherit_speed&1)
			status_calc_bl(&hd->bl, SCB_SPEED);
		merc_save(hd); 
	} else
		//Warp him to master.
		unit_warp(&hd->bl,sd->bl.m, sd->bl.x, sd->bl.y,CLR_OUTSIGHT);
	return 1;
}

// Recv homunculus data from char server
int merc_hom_recv_data(int account_id, struct s_homunculus *sh, int flag)
{
	struct map_session_data *sd;
	struct homun_data *hd;

	sd = map_id2sd(account_id);
	if(!sd)
		return 0;
	if (sd->status.char_id != sh->char_id)
	{
		if (sd->status.hom_id == sh->hom_id)
			sh->char_id = sd->status.char_id; //Correct char id.
		else
			return 0;
	}
	if(!flag) { // Failed to load
		sd->status.hom_id = 0;
		return 0;
	}

	if (!sd->status.hom_id) //Hom just created.
		sd->status.hom_id = sh->hom_id;
	if (sd->hd) //uh? Overwrite the data.
		memcpy(&sd->hd->homunculus, sh, sizeof(struct s_homunculus));
	else
		merc_hom_alloc(sd, sh);
	
	hd = sd->hd;
	if(hd && hd->homunculus.hp && !hd->homunculus.vaporize && hd->bl.prev == NULL && sd->bl.prev != NULL)
	{
		map_addblock(&hd->bl);
		clif_spawn(&hd->bl);
		clif_send_homdata(sd,SP_ACK,0);
		clif_hominfo(sd,hd,1);
		clif_hominfo(sd,hd,0); // send this x2. dunno why, but kRO does that [blackhole89]
		clif_homskillinfoblock(sd);
		merc_hom_init_timers(hd);
	}
	return 1;
}

// Ask homunculus creation to char server
int merc_create_homunculus_request(struct map_session_data *sd, int class_)
{
	struct s_homunculus homun;
	struct h_stats *base;
	int i;

	nullpo_retr(1, sd);

	i = search_homunculusDB_index(class_,HOMUNCULUS_CLASS);
	if(i < 0) return 0;
	
	memset(&homun, 0, sizeof(struct s_homunculus));
	//Initial data
	strncpy(homun.name, homunculus_db[i].name, NAME_LENGTH-1);
	homun.class_ = class_;
	homun.level = 1;
	homun.hunger = 32; //32%
	homun.intimacy = 2100; //21/1000
	homun.char_id = sd->status.char_id;
	
	homun.hp = 10 ;
	base = &homunculus_db[i].base;
	homun.max_hp = base->HP;
	homun.max_sp = base->SP;
	homun.str = base->str *10;
	homun.agi = base->agi *10;
	homun.vit = base->vit *10;
	homun.int_= base->int_*10;
	homun.dex = base->dex *10;
	homun.luk = base->luk *10;

	// Request homunculus creation
	intif_homunculus_create(sd->status.account_id, &homun); 
	return 1;
}

int merc_resurrect_homunculus(struct map_session_data* sd, unsigned char per, short x, short y)
{
	struct homun_data* hd;
	nullpo_ret(sd);

	if (!sd->status.hom_id)
		return 0; // no homunculus

	if (!sd->hd) //Load homun data;
		return intif_homunculus_requestload(sd->status.account_id, sd->status.hom_id);
	
	hd = sd->hd;

  	if (hd->homunculus.vaporize)
		return 0; // vaporized homunculi need to be 'called'

	if (!status_isdead(&hd->bl))
		return 0; // already alive

	merc_hom_init_timers(hd);

	if (!hd->bl.prev)
	{	//Add it back to the map.
		hd->bl.m = sd->bl.m;
		hd->bl.x = x;
		hd->bl.y = y;
		map_addblock(&hd->bl);
		clif_spawn(&hd->bl);
	}
	status_revive(&hd->bl, per, 0);
	return 1;
}

void merc_hom_revive(struct homun_data *hd, unsigned int hp, unsigned int sp)
{
	struct map_session_data *sd = hd->master;
	hd->homunculus.hp = hd->battle_status.hp;
	if (!sd)
		return;
	clif_send_homdata(sd,SP_ACK,0);
	clif_hominfo(sd,hd,1);
	clif_hominfo(sd,hd,0);
	clif_homskillinfoblock(sd);
}

void merc_reset_stats(struct homun_data *hd)
{	//Resets a homunc stats back to zero (but doesn't touches hunger or intimacy)
	struct s_homunculus_db *db;
	struct s_homunculus *hom;
	struct h_stats *base;
	hom = &hd->homunculus;
	db = hd->homunculusDB;
	base = &db->base;
	hom->level = 1;
	hom->hp = 10;
	hom->max_hp = base->HP;
	hom->max_sp = base->SP;
	hom->str = base->str *10;
	hom->agi = base->agi *10;
	hom->vit = base->vit *10;
	hom->int_= base->int_*10;
	hom->dex = base->dex *10;
	hom->luk = base->luk *10;
	hom->exp = 0;
	hd->exp_next = hexptbl[0];
	memset(&hd->homunculus.hskill, 0, sizeof hd->homunculus.hskill);
	hd->homunculus.skillpts = 0;
}

int merc_hom_shuffle(struct homun_data *hd)
{
	struct map_session_data *sd;
	int lv, i, skillpts;
	unsigned int exp;
	struct s_skill b_skill[MAX_HOMUNSKILL];

	if (!merc_is_hom_active(hd))
		return 0;

	sd = hd->master;
	lv = hd->homunculus.level;
	exp = hd->homunculus.exp;
	memcpy(&b_skill, &hd->homunculus.hskill, sizeof(b_skill));
	skillpts = hd->homunculus.skillpts;
	//Reset values to level 1.
	merc_reset_stats(hd);
	//Level it back up
	for (i = 1; i < lv && hd->exp_next; i++){
		hd->homunculus.exp += hd->exp_next;
		merc_hom_levelup(hd);
	}

	if(hd->homunculus.class_ == hd->homunculusDB->evo_class) {
		//Evolved bonuses
		struct s_homunculus *hom = &hd->homunculus;
		struct h_stats *max = &hd->homunculusDB->emax, *min = &hd->homunculusDB->emin;
		hom->max_hp += rand(min->HP, max->HP);
		hom->max_sp += rand(min->SP, max->SP);
		hom->str += 10*rand(min->str, max->str);
		hom->agi += 10*rand(min->agi, max->agi);
		hom->vit += 10*rand(min->vit, max->vit);
		hom->int_+= 10*rand(min->int_,max->int_);
		hom->dex += 10*rand(min->dex, max->dex);
		hom->luk += 10*rand(min->luk, max->luk);
	}

	hd->homunculus.exp = exp;
	memcpy(&hd->homunculus.hskill, &b_skill, sizeof(b_skill));
	hd->homunculus.skillpts = skillpts;
	clif_homskillinfoblock(sd);
	status_calc_homunculus(hd,0);
	status_percent_heal(&hd->bl, 100, 100);
	clif_specialeffect(&hd->bl,568,AREA);

	return 1;
}

static bool read_homunculusdb_sub(char* str[], int columns, int current)
{
	int classid; 
	struct s_homunculus_db *db;

	//Base Class,Evo Class
	classid = atoi(str[0]);
	if (classid < HM_CLASS_BASE || classid > HM_CLASS_MAX)
	{
		ShowError("read_homunculusdb : Invalid class %d\n", classid);
		return false;
	}
	db = &homunculus_db[current];
	db->base_class = classid;
	classid = atoi(str[1]);
	if (classid < HM_CLASS_BASE || classid > HM_CLASS_MAX)
	{
		db->base_class = 0;
		ShowError("read_homunculusdb : Invalid class %d\n", classid);
		return false;
	}
	db->evo_class = classid;
	//Name, Food, Hungry Delay, Base Size, Evo Size, Race, Element, ASPD
	strncpy(db->name,str[2],NAME_LENGTH-1);
	db->foodID = atoi(str[3]);
	db->hungryDelay = atoi(str[4]);
	db->base_size = atoi(str[5]);
	db->evo_size = atoi(str[6]);
	db->race = atoi(str[7]);
	db->element = atoi(str[8]);
	db->baseASPD = atoi(str[9]);
	//base HP, SP, str, agi, vit, int, dex, luk
	db->base.HP = atoi(str[10]);
	db->base.SP = atoi(str[11]);
	db->base.str = atoi(str[12]);
	db->base.agi = atoi(str[13]);
	db->base.vit = atoi(str[14]);
	db->base.int_= atoi(str[15]);
	db->base.dex = atoi(str[16]);
	db->base.luk = atoi(str[17]);
	//Growth Min/Max HP, SP, str, agi, vit, int, dex, luk
	db->gmin.HP = atoi(str[18]);
	db->gmax.HP = atoi(str[19]);
	db->gmin.SP = atoi(str[20]);
	db->gmax.SP = atoi(str[21]);
	db->gmin.str = atoi(str[22]);
	db->gmax.str = atoi(str[23]);
	db->gmin.agi = atoi(str[24]);
	db->gmax.agi = atoi(str[25]);
	db->gmin.vit = atoi(str[26]);
	db->gmax.vit = atoi(str[27]);
	db->gmin.int_= atoi(str[28]);
	db->gmax.int_= atoi(str[29]);
	db->gmin.dex = atoi(str[30]);
	db->gmax.dex = atoi(str[31]);
	db->gmin.luk = atoi(str[32]);
	db->gmax.luk = atoi(str[33]);
	//Evolution Min/Max HP, SP, str, agi, vit, int, dex, luk
	db->emin.HP = atoi(str[34]);
	db->emax.HP = atoi(str[35]);
	db->emin.SP = atoi(str[36]);
	db->emax.SP = atoi(str[37]);
	db->emin.str = atoi(str[38]);
	db->emax.str = atoi(str[39]);
	db->emin.agi = atoi(str[40]);
	db->emax.agi = atoi(str[41]);
	db->emin.vit = atoi(str[42]);
	db->emax.vit = atoi(str[43]);
	db->emin.int_= atoi(str[44]);
	db->emax.int_= atoi(str[45]);
	db->emin.dex = atoi(str[46]);
	db->emax.dex = atoi(str[47]);
	db->emin.luk = atoi(str[48]);
	db->emax.luk = atoi(str[49]);

	//Check that the min/max values really are below the other one.
	if(db->gmin.HP > db->gmax.HP)
		db->gmin.HP = db->gmax.HP;
	if(db->gmin.SP > db->gmax.SP)
		db->gmin.SP = db->gmax.SP;
	if(db->gmin.str > db->gmax.str)
		db->gmin.str = db->gmax.str;
	if(db->gmin.agi > db->gmax.agi)
		db->gmin.agi = db->gmax.agi;
	if(db->gmin.vit > db->gmax.vit)
		db->gmin.vit = db->gmax.vit;
	if(db->gmin.int_> db->gmax.int_)
		db->gmin.int_= db->gmax.int_;
	if(db->gmin.dex > db->gmax.dex)
		db->gmin.dex = db->gmax.dex;
	if(db->gmin.luk > db->gmax.luk)
		db->gmin.luk = db->gmax.luk;

	if(db->emin.HP > db->emax.HP)
		db->emin.HP = db->emax.HP;
	if(db->emin.SP > db->emax.SP)
		db->emin.SP = db->emax.SP;
	if(db->emin.str > db->emax.str)
		db->emin.str = db->emax.str;
	if(db->emin.agi > db->emax.agi)
		db->emin.agi = db->emax.agi;
	if(db->emin.vit > db->emax.vit)
		db->emin.vit = db->emax.vit;
	if(db->emin.int_> db->emax.int_)
		db->emin.int_= db->emax.int_;
	if(db->emin.dex > db->emax.dex)
		db->emin.dex = db->emax.dex;
	if(db->emin.luk > db->emax.luk)
		db->emin.luk = db->emax.luk;

	return true;
}

int read_homunculusdb(void)
{
	int i;
	const char *filename[]={"homunculus_db.txt","homunculus_db2.txt"};

	memset(homunculus_db,0,sizeof(homunculus_db));
	for(i = 0; i<ARRAYLENGTH(filename); i++)
	{
		char path[256];

		if( i > 0 )
		{
			sprintf(path, "%s/%s", db_path, filename[i]);

			if( !exists(path) )
			{
				continue;
			}
		}

		sv_readdb(db_path, filename[i], ',', 50, 50, MAX_HOMUNCULUS_CLASS, &read_homunculusdb_sub);
	}

	return 0;
}

static bool read_homunculus_skilldb_sub(char* split[], int columns, int current)
{// <hom class>,<skill id>,<max level>[,<job level>],<req id1>,<req lv1>,<req id2>,<req lv2>,<req id3>,<req lv3>,<req id4>,<req lv4>,<req id5>,<req lv5>
	int k, classid; 
	int j;
	int minJobLevelPresent = 0;

	if( columns == 14 )
		minJobLevelPresent = 1;	// MinJobLvl has been added

	// check for bounds [celest]
	classid = atoi(split[0]) - HM_CLASS_BASE;
	if ( classid >= MAX_HOMUNCULUS_CLASS )
	{
		ShowWarning("read_homunculus_skilldb: Invalud homunculus class %d.\n", atoi(split[0]));
		return false;
	}

	k = atoi(split[1]); //This is to avoid adding two lines for the same skill. [Skotlex]
	// Search an empty line or a line with the same skill_id (stored in j)
	ARR_FIND( 0, MAX_SKILL_TREE, j, !hskill_tree[classid][j].id || hskill_tree[classid][j].id == k );
	if (j == MAX_SKILL_TREE)
	{
		ShowWarning("Unable to load skill %d into homunculus %d's tree. Maximum number of skills per class has been reached.\n", k, classid);
		return false;
	}

	hskill_tree[classid][j].id = k;
	hskill_tree[classid][j].max = atoi(split[2]);
	if (minJobLevelPresent)
		hskill_tree[classid][j].joblv = atoi(split[3]);

	for( k = 0; k < MAX_PC_SKILL_REQUIRE; k++ )
	{
		hskill_tree[classid][j].need[k].id = atoi(split[3+k*2+minJobLevelPresent]);
		hskill_tree[classid][j].need[k].lv = atoi(split[3+k*2+minJobLevelPresent+1]);
	}

	return true;
}

int read_homunculus_skilldb(void)
{
	memset(hskill_tree,0,sizeof(hskill_tree));
	sv_readdb(db_path, "homun_skill_tree.txt", ',', 13, 14, -1, &read_homunculus_skilldb_sub);

	return 0;
}

void read_homunculus_expdb(void)
{
	FILE *fp;
	char line[1024];
	int i, j=0;
	char *filename[]={
#if REMODE
		"re/exp_homun.txt",
#else
		"pre-re/exp_homun.txt",
#endif
		"exp_homun2.txt"};

	memset(hexptbl,0,sizeof(hexptbl));
	for(i=0; i<2; i++){
		sprintf(line, "%s/%s", db_path, filename[i]);
		fp=fopen(line,"r");
		if(fp == NULL){
			if(i != 0)
				continue;
			ShowError("can't read %s\n",line);
			return;
		}
		while(fgets(line, sizeof(line), fp) && j < MAX_LEVEL)
		{
			if(line[0] == '/' && line[1] == '/')
				continue;

			hexptbl[j] = strtoul(line, NULL, 10);
			if (!hexptbl[j++])
				break;
		}
		if (hexptbl[MAX_LEVEL - 1]) // Last permitted level have to be 0!
		{
			ShowWarning("read_hexptbl: Reached max level in exp_homun [%d]. Remaining lines were not read.\n ", MAX_LEVEL);
			hexptbl[MAX_LEVEL - 1] = 0;
		}
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' levels in '"CL_WHITE"%s"CL_RESET"'.\n", j, filename[i]);
	}
}

void merc_reload(void)
{
	read_homunculusdb();
	read_homunculus_expdb();
}

void merc_skill_reload(void)
{
	read_homunculus_skilldb();
}

int do_init_merc(void)
{
	int class_;
	read_homunculusdb();
	read_homunculus_expdb();
	read_homunculus_skilldb();
	// Add homunc timer function to timer func list [Toms]
	add_timer_func_list(merc_hom_hungry, "merc_hom_hungry");

	//Stock view data for homuncs
	memset(&hom_viewdb, 0, sizeof(hom_viewdb));
	for (class_ = 0; class_ < ARRAYLENGTH(hom_viewdb); class_++) 
		hom_viewdb[class_].class_ = HM_CLASS_BASE+class_;
	return 0;
}

int do_final_merc(void);
