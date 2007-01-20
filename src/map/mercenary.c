#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../common/cbasetypes.h"
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
#include "charsave.h"

//Better equiprobability than rand()% [orn]
#define rand(a, b) a+(int) ((float)(b-a+1)*rand()/(RAND_MAX+1.0))

struct homunculus_db homunculus_db[MAX_HOMUNCULUS_CLASS];	//[orn]
struct skill_tree_entry hskill_tree[MAX_HOMUNCULUS_CLASS][MAX_SKILL_TREE];

static int merc_hom_hungry(int tid,unsigned int tick,int id,int data);

static unsigned int hexptbl[MAX_LEVEL];

void merc_damage(struct homun_data *hd,struct block_list *src,int hp,int sp)
{
	clif_hominfo(hd->master,hd,0);
}

int merc_hom_dead(struct homun_data *hd, struct block_list *src)
{
	//There's no intimacy penalties on death (from Tharis)
	struct map_session_data *sd = hd->master;

	clif_emotion(&hd->bl, 16) ;	//wah

	//Delete timers when dead.
	merc_hom_hungry_timer_delete(hd);
	hd->homunculus.hp = 0;

	if (!sd) //unit remove map will invoke unit free
		return 3;

	clif_hominfo(sd,hd,0); // Send dead flag
	clif_emotion(&sd->bl, 28) ; //sob
	//Remove from map (if it has no intimacy, it is auto-removed from memory)
	return 3;
}

//Vaporize a character's homun. If flag, HP needs to be 80% or above.
int merc_hom_vaporize(struct map_session_data *sd, int flag)
{
	struct homun_data *hd;

	nullpo_retr(0, sd);

	hd = sd->hd;
	if (!hd || hd->homunculus.vaporize)
		return 0;
	
	if (status_isdead(&hd->bl))
		return 0; //Can't vaporize a dead homun.

	if (flag && hd->battle_status.hp < (hd->battle_status.max_hp*80/100))
		return 0;

	hd->regen.state.block = 3; //Block regen while vaporized.
	//Delete timers when vaporized.
	merc_hom_hungry_timer_delete(hd);
	hd->homunculus.vaporize = 1;
	clif_hominfo(sd, sd->hd, 0);
	merc_save(hd);
	return unit_remove_map(&hd->bl, 0);
}

//delete a homunculus, completely "killing it". 
//Emote is the emotion the master should use, send negative to disable.
int merc_hom_delete(struct homun_data *hd, int emote)
{
	struct map_session_data *sd;
	nullpo_retr(0, hd);
	sd = hd->master;

	if (!sd)
		return unit_free(&hd->bl,1);

	if (emote >= 0)
		clif_emotion(&sd->bl, emote);

	//This makes it be deleted right away.
	hd->homunculus.intimacy = 0;
	// Send homunculus_dead to client
	hd->homunculus.hp = 0;
	clif_hominfo(sd, hd, 0);
	return unit_remove_map(&hd->bl,0);
}

int merc_hom_calc_skilltree(struct homun_data *hd)
{
	int i,id=0 ;
	int j,f=1;
	int c=0;

	nullpo_retr(0, hd);
	c = hd->homunculus.class_ - HM_CLASS_BASE;
	
	for(i=0;i < MAX_SKILL_TREE && (id = hskill_tree[c][i].id) > 0;i++)
	{
		if(hd->homunculus.hskill[id-HM_SKILLBASE-1].id)
			continue; //Skill already known.
		if(!battle_config.skillfree)
		{
			for(j=0;j<5;j++)
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
			hd->homunculus.hskill[id-HM_SKILLBASE-1].id = id ;
	}
	return 0;
}

int merc_hom_checkskill(struct homun_data *hd,int skill_id)
{
	int i = skill_id - HM_SKILLBASE - 1;
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

	if(!hd->homunculus.vaporize)
	{
		i = skillnum - HM_SKILLBASE - 1 ;
		if( hd->homunculus.skillpts > 0 &&
			hd->homunculus.hskill[i].id &&
			hd->homunculus.hskill[i].flag == 0 && //Don't allow raising while you have granted skills. [Skotlex]
			hd->homunculus.hskill[i].lv < merc_skill_tree_get_max(skillnum, hd->homunculus.class_)
			)
		{
			hd->homunculus.hskill[i].lv++ ;
			hd->homunculus.skillpts-- ;
			status_calc_homunculus(hd,0) ;
			if (hd->master) {
				clif_homskillup(hd->master, skillnum);
				clif_hominfo(hd->master,hd,0);
				clif_homskillinfoblock(hd->master);
			}
		}
	}
}

int merc_hom_levelup(struct homun_data *hd)
{
	int growth_str, growth_agi, growth_vit, growth_int, growth_dex, growth_luk ;
	int growth_max_hp, growth_max_sp ;
	char output[256] ;

	if (hd->homunculus.level == MAX_LEVEL || !hd->exp_next || hd->homunculus.exp < hd->exp_next)
		return 0 ;
	
	hd->homunculus.level++ ;
	if (!(hd->homunculus.level % 3)) 
		hd->homunculus.skillpts++ ;	//1 skillpoint each 3 base level

	hd->homunculus.exp -= hd->exp_next ;
	hd->exp_next = hexptbl[hd->homunculus.level - 1] ;
	
	growth_max_hp = rand(hd->homunculusDB->gminHP, hd->homunculusDB->gmaxHP) ;
	growth_max_sp = rand(hd->homunculusDB->gminSP, hd->homunculusDB->gmaxSP) ;
	growth_str = rand(hd->homunculusDB->gminSTR, hd->homunculusDB->gmaxSTR) ;
	growth_agi = rand(hd->homunculusDB->gminAGI, hd->homunculusDB->gmaxAGI) ;
	growth_vit = rand(hd->homunculusDB->gminVIT, hd->homunculusDB->gmaxVIT) ;
	growth_dex = rand(hd->homunculusDB->gminDEX, hd->homunculusDB->gmaxDEX) ;
	growth_int = rand(hd->homunculusDB->gminINT, hd->homunculusDB->gmaxINT) ;
	growth_luk = rand(hd->homunculusDB->gminLUK, hd->homunculusDB->gmaxLUK) ;

	//Aegis discards the decimals in the stat growth values!
	growth_str-=growth_str%10;
	growth_agi-=growth_agi%10;
	growth_vit-=growth_vit%10;
	growth_dex-=growth_dex%10;
	growth_int-=growth_int%10;
	growth_luk-=growth_luk%10;

	hd->homunculus.max_hp += growth_max_hp;
	hd->homunculus.max_sp += growth_max_sp;
	hd->homunculus.str += growth_str;
	hd->homunculus.agi += growth_agi;
	hd->homunculus.vit += growth_vit;
	hd->homunculus.dex += growth_dex;
	hd->homunculus.int_+= growth_int;
	hd->homunculus.luk += growth_luk;
	
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
	struct map_session_data *sd;
	nullpo_retr(0, hd);

	if(!hd->homunculusDB->evo_class)
	{
		clif_emotion(&hd->bl, 4) ;	//swt
		return 0 ;
	}
	sd = hd->master;
	if (!sd)
		return 0;

	merc_hom_vaporize(sd, 0);

	if (!merc_hom_change_class(hd, hd->homunculusDB->evo_class)) {
		ShowError("merc_hom_evolution: Can't evoluate homunc from %d to %d", hd->homunculus.class_, hd->homunculusDB->evo_class);
		return 0;
	}
	hd->homunculus.intimacy = 500;
	merc_call_homunculus(sd);
	clif_emotion(&sd->bl, 21);	//no1
	clif_misceffect2(&hd->bl,568);
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

	clif_misceffect2(&hd->bl,568);
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
	intif_homunculus_requestsave(sd->status.account_id, &hd->homunculus) ;
}

int merc_menu(struct map_session_data *sd,int menunum)
{
	nullpo_retr(0, sd);
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
	pc_delitem(sd,i,1,0);

	if ( hd->homunculus.hunger >= 91 ) {
		merc_hom_decrease_intimacy(hd, 50);
		emotion = 16;
	} else if ( hd->homunculus.hunger >= 76 ) {
		merc_hom_decrease_intimacy(hd, 5);
		emotion = 19;
	} else if ( hd->homunculus.hunger >= 26 ) {
		merc_hom_increase_intimacy(hd, 75);
		emotion = 2;
	} else if ( hd->homunculus.hunger >= 11 ) {
		merc_hom_increase_intimacy(hd, 100);
		emotion = 2;
	} else {
		merc_hom_increase_intimacy(hd, 50);
		emotion = 2;
	}

	hd->homunculus.hunger += 10;	//dunno increase value for each food
	if(hd->homunculus.hunger > 100)
		hd->homunculus.hunger = 100;

	clif_emotion(&hd->bl,emotion) ;
	clif_send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	clif_send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	clif_hom_food(sd,foodID,1);
       	
	// Too much food :/
	if(hd->homunculus.intimacy == 0)
		return merc_hom_delete(sd->hd, 23); //omg  

	return 0;
}

static int merc_hom_hungry(int tid,unsigned int tick,int id,int data)
{
	struct map_session_data *sd;
	struct homun_data *hd;

	sd=map_id2sd(id);
	if(!sd)
		return 1;

	if(!sd->status.hom_id || !(hd=sd->hd))
		return 1;

	if(hd->hungry_timer != tid){
		if(battle_config.error_log)
			ShowError("merc_hom_hungry_timer %d != %d\n",hd->hungry_timer,tid);
		return 0;
	}

	hd->hungry_timer = -1;
	
	hd->homunculus.hunger-- ;
	if(hd->homunculus.hunger <= 10) {
		clif_emotion(&hd->bl, 6) ;	//an
	} else if(hd->homunculus.hunger == 25) {
		clif_emotion(&hd->bl, 20) ;	//hmm
	} else if(hd->homunculus.hunger == 75) {
		clif_emotion(&hd->bl, 33) ;	//ok
	}  
	
	if(hd->homunculus.hunger < 0) {
		hd->homunculus.hunger = 0;
		// Delete the homunculus if intimacy <= 100
		if ( !merc_hom_decrease_intimacy(hd, 100) )
			return merc_hom_delete(hd, 23); //omg  
		clif_send_homdata(sd,SP_INTIMATE,hd->homunculus.intimacy / 100);
	}

	clif_send_homdata(sd,SP_HUNGRY,hd->homunculus.hunger);
	hd->hungry_timer = add_timer(tick+hd->homunculusDB->hungryDelay,merc_hom_hungry,sd->bl.id,0); //simple Fix albator
	return 0;
}

int merc_hom_hungry_timer_delete(struct homun_data *hd)
{
	nullpo_retr(0, hd);
	if(hd->hungry_timer != -1) {
		delete_timer(hd->hungry_timer,merc_hom_hungry);
		hd->hungry_timer = -1;
	}
	return 1;
}

int search_homunculusDB_index(int key,int type)
{
	int i;

	for(i=0;i<MAX_HOMUNCULUS_CLASS;i++) {
		if(homunculus_db[i].class_ <= 0)
			continue;
		switch(type) {
			case HOMUNCULUS_CLASS:
				if(homunculus_db[i].class_ == key)
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
	short x,y;

	nullpo_retr(1, sd);

	Assert((sd->status.hom_id == 0 || sd->hd == 0) || sd->hd->master == sd); 

	i = search_homunculusDB_index(hom->class_,HOMUNCULUS_CLASS);
	if(i < 0) {
		ShowError("merc_hom_alloc: unknown homunculus class [%d]", hom->class_);
		sd->status.hom_id = 0;
		intif_homunculus_requestdelete(hom->hom_id);
		return 1;
	}
	sd->hd = hd = aCalloc(1,sizeof(struct homun_data));
	hd->bl.subtype = MONS;
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
	x = sd->bl.x + 1;
	y = sd->bl.y + 1;
	map_random_dir(&hd->bl, &x, &y);
	hd->bl.x = x;
	hd->bl.y = y;
	
	map_addiddb(&hd->bl);
	status_calc_homunculus(hd,1);

	hd->hungry_timer = -1;
	return 0;
}

void merc_hom_init_timers(struct homun_data * hd)
{
	if (hd->hungry_timer == -1)
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
		unit_warp(&hd->bl,sd->bl.m, sd->bl.x, sd->bl.y,0);
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
	if(hd->homunculus.hp && !hd->homunculus.vaporize &&
		hd->bl.prev == NULL && sd->bl.prev != NULL)
	{
		map_addblock(&hd->bl);
		clif_spawn(&hd->bl);
		clif_hominfo(sd,hd,1);
		clif_hominfo(sd,hd,0); // send this x2. dunno why, but kRO does that [blackhole89]
		clif_homskillinfoblock(sd);
		clif_hominfo(sd,hd,0);
		clif_send_homdata(sd,SP_ACK,0);
		merc_hom_init_timers(hd);
	}
	return 1;
}

// Ask homunculus creation to char server
int merc_create_homunculus_request(struct map_session_data *sd, int class_)
{
	struct s_homunculus homun;
	int i;

	nullpo_retr(1, sd);

	i = search_homunculusDB_index(class_,HOMUNCULUS_CLASS);
	if(i < 0) return 0;
	
	memset(&homun, 0, sizeof(struct s_homunculus));
	//Initial data
	strncpy(homun.name, homunculus_db[i].name, NAME_LENGTH-1);
	homun.class_ = class_;
	homun.level = 1;
// FIXME: Commented value is what the map-server had as initial value,
// Uncommented value is what the char-server was overwriting it with
// So which one is correct?
//	homun.hunger = 50;
	homun.hunger = 32;
//	homun.intimacy = 500;
	homun.intimacy = 21;
	homun.char_id = sd->status.char_id;
	
	homun.hp = 10 ;
	homun.max_hp = homunculus_db[i].basemaxHP;
	homun.max_sp = homunculus_db[i].basemaxSP;
	homun.str = homunculus_db[i].baseSTR * 10;
	homun.agi = homunculus_db[i].baseAGI * 10;
	homun.vit = homunculus_db[i].baseVIT * 10;
	homun.int_ = homunculus_db[i].baseINT * 10;
	homun.dex = homunculus_db[i].baseDEX * 10;
	homun.luk = homunculus_db[i].baseLUK * 10;

	// Request homunculus creation
	intif_homunculus_create(sd->status.account_id, &homun); 
	return 1;
}

int merc_resurrect_homunculus(struct map_session_data *sd, unsigned char per, short x, short y)
{
	struct homun_data *hd;
	nullpo_retr(0, sd);
	if (!sd->status.hom_id)
		return 0;

	if (!sd->hd) //Load homun data;
		return intif_homunculus_requestload(sd->status.account_id, sd->status.hom_id);
	
	hd = sd->hd;

  	if (hd->homunculus.vaporize)
		return 0;

	if (!status_isdead(&hd->bl))
		return 0;

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
	struct homunculus_db *db;
	struct s_homunculus *hom;
	hom = &hd->homunculus;
	db = hd->homunculusDB;
	hom->level = 1;
	hom->hp = 10;
	hom->max_hp = db->basemaxHP;
	hom->max_sp = db->basemaxSP;
	hom->str = db->baseSTR*10;
	hom->agi = db->baseAGI*10;
	hom->vit = db->baseVIT*10;
	hom->int_= db->baseINT*10;
	hom->dex = db->baseDEX*10;
	hom->luk = db->baseLUK*10;
	hom->exp = 0;
	hd->exp_next = hexptbl[0];
	memset(&hd->homunculus.hskill, 0, sizeof hd->homunculus.hskill);
	hd->homunculus.skillpts = 0;
}

int read_homunculusdb(void)
{
	FILE *fp;
	char line[1024], *p;
	int i, k, classid; 
	int j = 0;
	char *filename[]={"homunculus_db.txt","homunculus_db2.txt"};
	char *str[36];
	struct homunculus_db *db;

	memset(homunculus_db,0,sizeof(homunculus_db));
	for(i = 0; i<2; i++)
	{
		sprintf(line, "%s/%s", db_path, filename[i]);
		fp = fopen(line,"r");
		if(!fp){
			if(i != 0)
				continue;
			ShowError("read_homunculusdb : can't read %s\n", line);
			return -1;
		}

		while(fgets(line,sizeof(line)-1,fp) && j < MAX_HOMUNCULUS_CLASS)
		{
			if(line[0] == '/' && line[1] == '/')
				continue;

			k = 0;
			p = strtok (line,",");
			while (p != NULL && k < 36)
			{
				str[k++] = p;
				p = strtok (NULL, ",");
			}
			
			classid = atoi(str[0]);
			if (k != 36 || classid < HM_CLASS_BASE || classid > HM_CLASS_MAX)
			{
				ShowError("read_homunculusdb : Error reading %s", filename[i]);
				continue;
			}

			//Class,Homunculus,HP,SP,ATK,MATK,HIT,CRI,DEF,MDEF,FLEE,ASPD,STR,AGI,VIT,INT,DEX,LUK
			db = &homunculus_db[j];
			db->class_ = classid;
			strncpy(db->name,str[1],NAME_LENGTH-1);
			db->basemaxHP = atoi(str[2]);
			db->basemaxSP = atoi(str[3]);
			db->baseSTR = atoi(str[4]);
			db->baseAGI = atoi(str[5]);
			db->baseVIT = atoi(str[6]);
			db->baseINT = atoi(str[7]);
			db->baseDEX = atoi(str[8]);
			db->baseLUK = atoi(str[9]);
			db->baseIntimacy = atoi(str[10]);
			db->baseHungry = atoi(str[11]);
			db->hungryDelay = atoi(str[12]);
			db->foodID = atoi(str[13]);
			db->gminHP = atoi(str[14]);
			db->gmaxHP = atoi(str[15]);
			db->gminSP = atoi(str[16]);
			db->gmaxSP = atoi(str[17]);
			db->gminSTR = atoi(str[18]);
			db->gmaxSTR = atoi(str[19]);
			db->gminAGI = atoi(str[20]);
			db->gmaxAGI = atoi(str[21]);
			db->gminVIT = atoi(str[22]);
			db->gmaxVIT = atoi(str[23]);
			db->gminINT = atoi(str[24]);
			db->gmaxINT = atoi(str[25]);
			db->gminDEX = atoi(str[26]);
			db->gmaxDEX = atoi(str[27]);
			db->gminLUK = atoi(str[28]);
			db->gmaxLUK = atoi(str[29]);
			db->evo_class = atoi(str[30]);
			db->baseASPD = atoi(str[31]);
			db->size = atoi(str[32]);
			db->race = atoi(str[33]);
			db->element = atoi(str[34]);
			db->accessID = atoi(str[35]);
			//Check that the min/max values really are below the other one.
			if (db->gmaxHP <= db->gminHP)
				db->gmaxHP = db->gminHP+1;
			if (db->gmaxSP <= db->gminSP)
				db->gmaxSP = db->gminSP+1;
			if (db->gmaxSTR <= db->gminSTR)
				db->gmaxSTR = db->gminSTR+1;
			if (db->gmaxAGI <= db->gminAGI)
				db->gmaxAGI = db->gminAGI+1;
			if (db->gmaxVIT <= db->gminVIT)
				db->gmaxVIT = db->gminVIT+1;
			if (db->gmaxINT <= db->gminINT)
				db->gmaxINT = db->gminINT+1;
			if (db->gmaxDEX <= db->gminDEX)
				db->gmaxDEX = db->gminDEX+1;
			if (db->gmaxLUK <= db->gminLUK)
				db->gmaxLUK = db->gminLUK+1;
			j++;
		}
		if (j > MAX_HOMUNCULUS_CLASS)
			ShowWarning("read_homunculusdb: Reached max number of homunculus [%d]. Remaining homunculus were not read.\n ", MAX_HOMUNCULUS_CLASS);
		fclose(fp);
		ShowStatus("Done reading '"CL_WHITE"%d"CL_RESET"' homunculus in '"CL_WHITE"db/%s"CL_RESET"'.\n",j,filename[i]);
	}
	return 0;
}

int read_homunculus_skilldb(void)
{
	FILE *fp;
	char line[1024], *p;
	int k, classid; 
	int j = 0;
	char *split[15];

	memset(hskill_tree,0,sizeof(hskill_tree));
	sprintf(line, "%s/homun_skill_tree.txt", db_path);
	fp=fopen(line,"r");
	if(fp==NULL){
		ShowError("can't read %s\n", line);
		return 1;
	}

	while(fgets(line, sizeof(line)-1, fp))
	{
		int minJobLevelPresent = 0;

		if(line[0]=='/' && line[1]=='/')
			continue;

		k = 0;
		p = strtok(line,",");
		while (p != NULL && k < 15)
		{
			split[k++] = p;
			p = strtok(NULL, ",");
		}

		if(k < 13)
			continue;

		if (k == 14)
			minJobLevelPresent = 1;	// MinJobLvl has been added

		// check for bounds [celest]
		classid = atoi(split[0]) - HM_CLASS_BASE;
		if ( classid >= MAX_HOMUNCULUS_CLASS )
			continue;

		k = atoi(split[1]); //This is to avoid adding two lines for the same skill. [Skotlex]
		// Search an empty line or a line with the same skill_id (stored in j)
		for(j = 0; j < MAX_SKILL_TREE && hskill_tree[classid][j].id && hskill_tree[classid][j].id != k; j++);

		if (j == MAX_SKILL_TREE)
		{
			ShowWarning("Unable to load skill %d into homunculus %d's tree. Maximum number of skills per class has been reached.\n", k, classid);
			continue;
		}

		hskill_tree[classid][j].id=k;
		hskill_tree[classid][j].max=atoi(split[2]);
		if (minJobLevelPresent)
			hskill_tree[classid][j].joblv=atoi(split[3]);

		for(k=0;k<5;k++){
			hskill_tree[classid][j].need[k].id=atoi(split[3+k*2+minJobLevelPresent]);
			hskill_tree[classid][j].need[k].lv=atoi(split[3+k*2+minJobLevelPresent+1]);
		}
	}

	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","homun_skill_tree.txt");
	return 0;
}

void read_homunculus_expdb(void)
{
	FILE *fp;
	char line[1024];
	int i, j=0;
	char *filename[]={"exp_homun.txt","exp_homun2.txt"};

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
		while(fgets(line,sizeof(line)-1,fp) && j < MAX_LEVEL)
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
	read_homunculusdb();
	read_homunculus_expdb();
	read_homunculus_skilldb();
	// Add homunc timer function to timer func list [Toms]
	add_timer_func_list(merc_hom_hungry, "merc_hom_hungry");
	return 0;
}

int do_final_merc(void);
