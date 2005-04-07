
// ƒXƒe[ƒ^ƒXŒvZAó‘ÔˆÙíˆ—
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "pc.h"
#include "map.h"
#include "pet.h"
#include "mob.h"
#include "clif.h"
#include "guild.h"
#include "skill.h"
#include "itemdb.h"
#include "battle.h"
#include "chrif.h"
#include "status.h"

#include "timer.h"
#include "nullpo.h"
#include "script.h"
#include "showmsg.h"

/* ƒXƒLƒ‹”Ô?„ƒXƒe?ƒ^ƒXˆÙí”Ô??Š·ƒe?ƒuƒ‹ */
int SkillStatusChangeTable[]={	/* status.h‚Ìenum‚ÌSC_***‚Æ‚ ‚í‚¹‚é‚±‚Æ */
/* 0- */
	-1,-1,-1,-1,-1,-1,
	SC_PROVOKE,			/* ƒvƒƒ{ƒbƒN */
	-1, 1,-1,
/* 10- */
	SC_SIGHT,			/* ƒTƒCƒg */
	-1,
	SC_SAFETYWALL,		/* ƒZ[ƒtƒeƒB[ƒEƒH[ƒ‹ */
	-1,-1,-1,
	SC_FREEZE,			/* ƒtƒƒXƒgƒ_ƒCƒo? */
	SC_STONE,			/* ƒXƒg?ƒ“ƒJ?ƒX */
	-1,-1,
/* 20- */
	-1,-1,-1,-1,
	SC_RUWACH,			/* ƒ‹ƒAƒt */
	SC_PNEUMA,			/* ƒjƒ…[ƒ} */
	-1,-1,-1,
	SC_INCREASEAGI,		/* ‘¬“x?‰Á */
/* 30- */
	SC_DECREASEAGI,		/* ‘¬“xŒ¸­ */
	-1,
	SC_SIGNUMCRUCIS,	/* ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX */
	SC_ANGELUS,			/* ƒGƒ“ƒWƒFƒ‰ƒX */
	SC_BLESSING,		/* ƒuƒŒƒbƒVƒ“ƒO */
	-1,-1,-1,-1,-1,
/* 40- */
	-1,-1,-1,-1,-1,
	SC_CONCENTRATE,		/* W’†—ÍŒüã */
	-1,-1,-1,-1,
/* 50- */
	-1,
	SC_HIDING,			/* ƒnƒCƒfƒBƒ“ƒO */
	-1,-1,-1,-1,-1,-1,-1,-1,
/* 60- */
	SC_TWOHANDQUICKEN,	/* 2HQ */
	SC_AUTOCOUNTER,
	-1,-1,-1,-1,
	SC_IMPOSITIO,		/* ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX */
	SC_SUFFRAGIUM,		/* ƒTƒtƒ‰ƒMƒEƒ€ */
	SC_ASPERSIO,		/* ƒAƒXƒyƒ‹ƒVƒI */
	SC_BENEDICTIO,		/* ¹?~•Ÿ */
/* 70- */
	-1,
	SC_SLOWPOISON,
	-1,
	SC_KYRIE,			/* ƒLƒŠƒGƒGƒŒƒCƒ\ƒ“ */
	SC_MAGNIFICAT,		/* ƒ}ƒOƒjƒtƒBƒJ?ƒg */
	SC_GLORIA,			/* ƒOƒƒŠƒA */
	SC_DIVINA,			/* ƒŒƒbƒNƒXƒfƒBƒr?ƒi */
	-1,
	SC_AETERNA,			/* ƒŒƒbƒNƒXƒG?ƒeƒ‹ƒi */
	-1,
/* 80- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 90- */
	-1,-1,
	SC_QUAGMIRE,		/* ƒNƒ@ƒOƒ}ƒCƒA */
	-1,-1,-1,-1,-1,-1,-1,
/* 100- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 110- */
	-1,
	SC_ADRENALINE,		/* ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ… */
	SC_WEAPONPERFECTION,/* ƒEƒFƒ|ƒ“ƒp?ƒtƒFƒNƒVƒ‡ƒ“ */
	SC_OVERTHRUST,		/* ƒI?ƒo?ƒgƒ‰ƒXƒg */
	SC_MAXIMIZEPOWER,	/* ƒ}ƒLƒVƒ}ƒCƒYƒpƒ? */
	-1,-1,-1,-1,-1,
/* 120- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 130- */
	-1,-1,-1,-1,-1,
	SC_CLOAKING,		/* ƒNƒ?ƒLƒ“ƒO */
	SC_STAN,			/* ƒ\ƒjƒbƒNƒuƒ? */
	-1,
	SC_ENCPOISON,		/* ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“ */
	SC_POISONREACT,		/* ƒ|ƒCƒYƒ“ƒŠƒAƒNƒg */
/* 140- */
	SC_POISON,			/* ƒxƒmƒ€ƒ_ƒXƒg */
	SC_SPLASHER,		/* ƒxƒiƒ€ƒXƒvƒ‰ƒbƒVƒƒ? */
	-1,
	SC_TRICKDEAD,		/* €‚ñ‚¾‚Ó‚è */
	-1,-1,SC_AUTOBERSERK,-1,-1,-1,
/* 150- */
	-1,-1,-1,-1,-1,
	SC_LOUD,			/* ƒ‰ƒEƒhƒ{ƒCƒX */
	-1,
	SC_ENERGYCOAT,		/* ƒGƒiƒW?ƒR?ƒg */
	-1,-1,
/* 160- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,
	SC_SELFDESTRUCTION,
	-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,
	SC_KEEPING,
	-1,-1,
	SC_BARRIER,
	-1,-1,
	SC_HALLUCINATION,
	-1,-1,
/* 210- */
	-1,-1,-1,-1,-1,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR,
	SC_STRIPHELM,
	-1,
/* 220- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 230- */
	-1,-1,-1,-1,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	-1,-1,
/* 240- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,
	SC_AUTOGUARD,
/* 250- */
	-1,-1,
	SC_REFLECTSHIELD,
	-1,-1,
	SC_DEVOTION,
	SC_PROVIDENCE,
	SC_DEFENDER,
	SC_SPEARSQUICKEN,
	-1,
/* 260- */
	-1,-1,-1,-1,-1,-1,-1,-1,
	SC_STEELBODY,
	SC_BLADESTOP_WAIT,
/* 270- */
	SC_EXPLOSIONSPIRITS,
	SC_EXTREMITYFIST,
	-1,-1,-1,-1,
	SC_MAGICROD,
	-1,-1,-1,
/* 280- */
	SC_FLAMELAUNCHER,
	SC_FROSTWEAPON,
	SC_LIGHTNINGLOADER,
	SC_SEISMICWEAPON,
	-1,
	SC_VOLCANO,
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_LANDPROTECTOR,
	-1,
/* 290- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 300- */
	-1,-1,-1,-1,-1,-1,
	SC_LULLABY,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
/* 310- */
	SC_NIBELUNGEN,
	SC_ROKISWEIL,
	SC_INTOABYSS,
	SC_SIEGFRIED,
	-1,-1,-1,
	SC_DISSONANCE,
	-1,
	SC_WHISTLE,
/* 320- */
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	-1,-1,
	SC_UGLYDANCE,
	-1,
	SC_HUMMING,
	SC_DONTFORGETME,
	SC_FORTUNE,
/* 330- */
	SC_SERVICE4U,
	SC_SELFDESTRUCTION,
	-1,-1,-1,-1,-1,-1,-1,-1,
/* 340- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
/* 350- */
	-1,-1,-1,-1,-1,
	SC_AURABLADE,
	SC_PARRYING,
	SC_CONCENTRATION,
	SC_TENSIONRELAX,
	SC_BERSERK,
/* 360- */
	SC_BERSERK,
	SC_ASSUMPTIO,
	SC_BASILICA,
	-1,-1,-1,
	SC_MAGICPOWER,
	-1,
	SC_SACRIFICE,
	SC_GOSPEL,
/* 370- */
	-1,-1,-1,-1,-1,-1,-1,-1,
	SC_EDP,
	-1,
/* 380- */
	SC_TRUESIGHT,
	-1,-1,
	SC_WINDWALK,
	SC_MELTDOWN,
	-1,-1,
	SC_CARTBOOST,
	-1,
	SC_CHASEWALK,
/* 390- */
	SC_REJECTSWORD,
	-1,-1,-1,-1,
	SC_MOONLIT,
	SC_MARIONETTE,
	-1,
	SC_BLEEDING,
	SC_JOINTBEAT,
/* 400 */
	-1,-1,
	SC_MINDBREAKER,
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	-1,-1,
	SC_BABY,
	-1,
/* 410- */
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

static int max_weight_base[MAX_PC_CLASS];
static int hp_coefficient[MAX_PC_CLASS];
static int hp_coefficient2[MAX_PC_CLASS];
static int hp_sigma_val[MAX_PC_CLASS][MAX_LEVEL];
static int sp_coefficient[MAX_PC_CLASS];
static int aspd_base[MAX_PC_CLASS][20];
static int refinebonus[5][3];	// ¸˜Bƒ{[ƒiƒXƒe[ƒuƒ‹(refine_db.txt)
int percentrefinery[5][10];	// ¸˜B¬Œ÷—¦(refine_db.txt)
static int atkmods[3][20];	// •ŠíATKƒTƒCƒYC³(size_fix.txt)
static char job_bonus[3][MAX_PC_CLASS][MAX_LEVEL];

int current_equip_item_index; //Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
//we need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only
//to avoid cards exploits

/*==========================================
 * ¸˜Bƒ{[ƒiƒX
 *------------------------------------------
 */
int status_getrefinebonus(int lv,int type)
{
	if (lv >= 0 && lv < 5 && type >= 0 && type < 3)
		return refinebonus[lv][type];
	return 0;
}

/*==========================================
 * ¸˜B¬Œ÷—¦
 *------------------------------------------
 */
int status_percentrefinery(struct map_session_data *sd,struct item *item)
{
	int percent;

	nullpo_retr(0, item);
	percent=percentrefinery[itemdb_wlv(item->nameid)][(int)item->refine];

	percent += pc_checkskill(sd,BS_WEAPONRESEARCH);	// •ŠíŒ¤‹†ƒXƒLƒ‹Š

	// Šm—¦‚Ì—LŒø”ÍˆÍƒ`ƒFƒbƒN
	if( percent > 100 ){
		percent = 100;
	}
	if( percent < 0 ){
		percent = 0;
	}

	return percent;
}

/*==========================================
 * ƒpƒ‰ƒ[ƒ^ŒvZ
 * first==0‚ÌAŒvZ‘ÎÛ‚Ìƒpƒ‰ƒ[ƒ^‚ªŒÄ‚Ño‚µ‘O‚©‚ç
 * •Ï ‰»‚µ‚½ê‡©“®‚Åsend‚·‚é‚ªA
 * ”\“®“I‚É•Ï‰»‚³‚¹‚½ƒpƒ‰ƒ[ƒ^‚Í©‘O‚Åsend‚·‚é‚æ‚¤‚É
 *------------------------------------------
 */

int status_calc_pc(struct map_session_data* sd,int first)
{
	int b_speed,b_max_hp,b_max_sp,b_hp,b_sp,b_weight,b_max_weight,b_paramb[6],b_parame[6],b_hit,b_flee;
	int b_aspd,b_watk,b_def,b_watk2,b_def2,b_flee2,b_critical,b_attackrange,b_matk1,b_matk2,b_mdef,b_mdef2,b_class;
	int b_base_atk;
	struct skill b_skill[MAX_SKILL];
	int i,bl,index;
	int skill,aspd_rate,wele,wele_,def_ele,refinedef=0;
	int pele=0,pdef_ele=0;
	int str,dstr,dex;
	struct pc_base_job s_class;

	nullpo_retr(0, sd);

	//?¶‚â—{q‚Ìê‡‚ÌŒ³‚ÌE‹Æ‚ğZo‚·‚é
	s_class = pc_calc_base_job(sd->status.class_);

	b_speed = sd->speed;
	b_max_hp = sd->status.max_hp;
	b_max_sp = sd->status.max_sp;
	b_hp = sd->status.hp;
	b_sp = sd->status.sp;
	b_weight = sd->weight;
	b_max_weight = sd->max_weight;
	memcpy(b_paramb,&sd->paramb,sizeof(b_paramb));
	memcpy(b_parame,&sd->paramc,sizeof(b_parame));
	memcpy(b_skill,&sd->status.skill,sizeof(b_skill));
	b_hit = sd->hit;
	b_flee = sd->flee;
	b_aspd = sd->aspd;
	b_watk = sd->watk;
	b_def = sd->def;
	b_watk2 = sd->watk2;
	b_def2 = sd->def2;
	b_flee2 = sd->flee2;
	b_critical = sd->critical;
	b_attackrange = sd->attackrange;
	b_matk1 = sd->matk1;
	b_matk2 = sd->matk2;
	b_mdef = sd->mdef;
	b_mdef2 = sd->mdef2;
	b_class = sd->view_class;
	sd->view_class = sd->status.class_;
	b_base_atk = sd->base_atk;

	pc_calc_skilltree(sd);	// ƒXƒLƒ‹ƒcƒŠ?‚ÌŒvZ

	sd->max_weight = max_weight_base[s_class.job]+sd->status.str*300;

	if(first&1) {
		sd->weight=0;
		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid==0 || sd->inventory_data[i] == NULL)
				continue;
			sd->weight += sd->inventory_data[i]->weight*sd->status.inventory[i].amount;
		}
		sd->cart_max_weight=battle_config.max_cart_weight;
		sd->cart_weight=0;
		sd->cart_max_num=MAX_CART;
		sd->cart_num=0;
		for(i=0;i<MAX_CART;i++){
			if(sd->status.cart[i].nameid==0)
				continue;
			sd->cart_weight+=itemdb_weight(sd->status.cart[i].nameid)*sd->status.cart[i].amount;
			sd->cart_num++;
		}
	}

	memset(sd->paramb,0,sizeof(sd->paramb));
	memset(sd->parame,0,sizeof(sd->parame));
	sd->hit = 0;
	sd->flee = 0;
	sd->flee2 = 0;
	sd->critical = 0;
	sd->aspd = 0;
	sd->watk = 0;
	sd->def = 0;
	sd->mdef = 0;
	sd->watk2 = 0;
	sd->def2 = 0;
	sd->mdef2 = 0;
	sd->status.max_hp = 0;
	sd->status.max_sp = 0;
	sd->attackrange = 0;
	sd->attackrange_ = 0;
	sd->atk_ele = 0;
	sd->def_ele = 0;
	sd->star =0;
	sd->overrefine =0;
	sd->matk1 =0;
	sd->matk2 =0;
	sd->speed = DEFAULT_WALK_SPEED ;
	sd->hprate=battle_config.hp_rate;
	sd->sprate=battle_config.sp_rate;
	sd->castrate=100;
	sd->delayrate=100;
	sd->dsprate=100;
	sd->base_atk=0;
	sd->arrow_atk=0;
	sd->arrow_ele=0;
	sd->arrow_hit=0;
	sd->arrow_range=0;
	sd->nhealhp=sd->nhealsp=sd->nshealhp=sd->nshealsp=sd->nsshealhp=sd->nsshealsp=0;
	memset(sd->addele,0,sizeof(sd->addele));
	memset(sd->addrace,0,sizeof(sd->addrace));
	memset(sd->addsize,0,sizeof(sd->addsize));
	memset(sd->addele_,0,sizeof(sd->addele_));
	memset(sd->addrace_,0,sizeof(sd->addrace_));
	memset(sd->addsize_,0,sizeof(sd->addsize_));
	memset(sd->subele,0,sizeof(sd->subele));
	memset(sd->subrace,0,sizeof(sd->subrace));
	memset(sd->addeff,0,sizeof(sd->addeff));
	memset(sd->addeff2,0,sizeof(sd->addeff2));
	memset(sd->reseff,0,sizeof(sd->reseff));
	memset(&sd->special_state,0,sizeof(sd->special_state));
	memset(sd->weapon_coma_ele,0,sizeof(sd->weapon_coma_ele));
	memset(sd->weapon_coma_race,0,sizeof(sd->weapon_coma_race));
	memset(sd->weapon_atk,0,sizeof(sd->weapon_atk));
	memset(sd->weapon_atk_rate,0,sizeof(sd->weapon_atk_rate));

	sd->watk_ = 0;			//“ñ“—¬—p(?)
	sd->watk_2 = 0;
	sd->atk_ele_ = 0;
	sd->star_ = 0;
	sd->overrefine_ = 0;

	sd->aspd_rate = 100;
	sd->speed_rate = 100;
	sd->hprecov_rate = 100;
	sd->sprecov_rate = 100;
	sd->critical_def = 0;
	sd->double_rate = 0;
	sd->near_attack_def_rate = sd->long_attack_def_rate = 0;
	sd->atk_rate = sd->matk_rate = 100;
	sd->ignore_def_ele = sd->ignore_def_race = 0;
	sd->ignore_def_ele_ = sd->ignore_def_race_ = 0;
	sd->ignore_mdef_ele = sd->ignore_mdef_race = 0;
	sd->arrow_cri = 0;
	sd->magic_def_rate = sd->misc_def_rate = 0;
	memset(sd->arrow_addele,0,sizeof(sd->arrow_addele));
	memset(sd->arrow_addrace,0,sizeof(sd->arrow_addrace));
	memset(sd->arrow_addsize,0,sizeof(sd->arrow_addsize));
	memset(sd->arrow_addeff,0,sizeof(sd->arrow_addeff));
	memset(sd->arrow_addeff2,0,sizeof(sd->arrow_addeff2));
	memset(sd->magic_addele,0,sizeof(sd->magic_addele));
	memset(sd->magic_addrace,0,sizeof(sd->magic_addrace));
	memset(sd->magic_subrace,0,sizeof(sd->magic_subrace));
	sd->perfect_hit = 0;
	sd->critical_rate = sd->hit_rate = sd->flee_rate = sd->flee2_rate = 100;
	sd->def_rate = sd->def2_rate = sd->mdef_rate = sd->mdef2_rate = 100;
	sd->def_ratio_atk_ele = sd->def_ratio_atk_ele_ = 0;
	sd->def_ratio_atk_race = sd->def_ratio_atk_race_ = 0;
	sd->get_zeny_num = 0;
	sd->add_damage_class_count = sd->add_damage_class_count_ = sd->add_magic_damage_class_count = 0;
	sd->add_def_class_count = sd->add_mdef_class_count = 0;
	sd->monster_drop_item_count = 0;
	memset(sd->add_damage_classrate,0,sizeof(sd->add_damage_classrate));
	memset(sd->add_damage_classrate_,0,sizeof(sd->add_damage_classrate_));
	memset(sd->add_magic_damage_classrate,0,sizeof(sd->add_magic_damage_classrate));
	memset(sd->add_def_classrate,0,sizeof(sd->add_def_classrate));
	memset(sd->add_mdef_classrate,0,sizeof(sd->add_mdef_classrate));
	memset(sd->monster_drop_race,0,sizeof(sd->monster_drop_race));
	memset(sd->monster_drop_itemrate,0,sizeof(sd->monster_drop_itemrate));
	sd->speed_add_rate = sd->aspd_add_rate = 100;
	sd->double_add_rate = sd->perfect_hit_add = sd->get_zeny_add_num = 0;
	sd->splash_range = sd->splash_add_range = 0;
	sd->autospell_id = sd->autospell_lv = sd->autospell_rate = 0;
	sd->hp_drain_rate = sd->hp_drain_per = sd->sp_drain_rate = sd->sp_drain_per = 0;
	sd->hp_drain_rate_ = sd->hp_drain_per_ = sd->sp_drain_rate_ = sd->sp_drain_per_ = 0;
	sd->short_weapon_damage_return = sd->long_weapon_damage_return = 0;
	sd->magic_damage_return = 0; //AppleGirl Was Here
	sd->random_attack_increase_add = sd->random_attack_increase_per = 0;
	sd->hp_drain_value = sd->hp_drain_value_ = sd->sp_drain_value = sd->sp_drain_value_ = 0;
	sd->unbreakable_equip = 0;

	sd->break_weapon_rate = sd->break_armor_rate = 0;
	sd->add_steal_rate = 0;
	sd->crit_atk_rate = 0;
	sd->no_regen = 0;
	sd->unstripable_equip = 0;
	sd->autospell2_id = sd->autospell2_lv = sd->autospell2_rate = 0;
	memset(sd->critaddrace,0,sizeof(sd->critaddrace));
	memset(sd->addeff3,0,sizeof(sd->addeff3));
	memset(sd->addeff3_type,0,sizeof(sd->addeff3_type));
	memset(sd->skillatk,0,sizeof(sd->skillatk));
	sd->add_damage_class_count = sd->add_damage_class_count_ = sd->add_magic_damage_class_count = 0;
	sd->add_def_class_count = sd->add_mdef_class_count = 0;
	sd->add_damage_class_count2 = 0;
	memset(sd->add_damage_classid,0,sizeof(sd->add_damage_classid));
	memset(sd->add_damage_classid_,0,sizeof(sd->add_damage_classid_));
	memset(sd->add_magic_damage_classid,0,sizeof(sd->add_magic_damage_classid));
	memset(sd->add_damage_classrate,0,sizeof(sd->add_damage_classrate));
	memset(sd->add_damage_classrate_,0,sizeof(sd->add_damage_classrate_));
	memset(sd->add_magic_damage_classrate,0,sizeof(sd->add_magic_damage_classrate));
	memset(sd->add_def_classid,0,sizeof(sd->add_def_classid));
	memset(sd->add_def_classrate,0,sizeof(sd->add_def_classrate));
	memset(sd->add_mdef_classid,0,sizeof(sd->add_mdef_classid));
	memset(sd->add_mdef_classrate,0,sizeof(sd->add_mdef_classrate));
	memset(sd->add_damage_classid2,0,sizeof(sd->add_damage_classid2));
	memset(sd->add_damage_classrate2,0,sizeof(sd->add_damage_classrate2));
	sd->sp_gain_value = 0;
	sd->ignore_def_mob = sd->ignore_def_mob_ = 0;
	sd->hp_loss_rate = sd->hp_loss_value = sd->hp_loss_type = 0;
	memset(sd->addrace2,0,sizeof(sd->addrace2));
	memset(sd->addrace2_,0,sizeof(sd->addrace2_));
	sd->hp_gain_value = sd->sp_drain_type = 0;
	memset(sd->subsize,0,sizeof(sd->subsize));
	memset(sd->unequip_losehp,0,sizeof(sd->unequip_losehp));
	memset(sd->unequip_losesp,0,sizeof(sd->unequip_losesp));
	memset(sd->subrace2,0,sizeof(sd->subrace2));
	memset(sd->expaddrace,0,sizeof(sd->expaddrace));
	memset(sd->sp_gain_race,0,sizeof(sd->sp_gain_race));

	if(!sd->disguiseflag && sd->disguise) {
		sd->disguise=0;
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
		clif_changelook(&sd->bl,LOOK_HEAD_BOTTOM,sd->status.head_bottom);
		clif_changelook(&sd->bl,LOOK_HEAD_TOP,sd->status.head_top);
		clif_changelook(&sd->bl,LOOK_HEAD_MID,sd->status.head_mid);
		clif_clearchar(&sd->bl, 9);
		pc_setpos(sd, sd->mapname, sd->bl.x, sd->bl.y, 3);
	}

	if (sd->status.guild_id > 0) {
		struct guild *g = guild_search(sd->status.guild_id);
		if (g && strcmp(sd->status.name,g->master)==0)
			sd->state.gmaster_flag = (int)g;
	}

	for(i=0;i<10;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == 9 && sd->equip_index[8] == index)
			continue;
		if(i == 5 && sd->equip_index[4] == index)
			continue;
		if(i == 6 && (sd->equip_index[5] == index || sd->equip_index[4] == index))
			continue;

		if(sd->inventory_data[index]) {
			if(sd->inventory_data[index]->type == 4) {
				if(sd->status.inventory[index].card[0]!=0x00ff && sd->status.inventory[index].card[0]!=0x00fe && sd->status.inventory[index].card[0]!=(short)0xff00) {
					int j;
					for(j=0;j<sd->inventory_data[index]->slot;j++){	// ƒJ?ƒh
						int c=sd->status.inventory[index].card[j];
						if(c>0){
							if(i == 8 && sd->status.inventory[index].equip == 0x20)
								sd->state.lr_flag = 1;
							run_script(itemdb_equipscript(c),0,sd->bl.id,0);
							sd->state.lr_flag = 0;
						}
					}
				}
			}
			else if(sd->inventory_data[index]->type==5){ // –h‹ï
				if(sd->status.inventory[index].card[0]!=0x00ff && sd->status.inventory[index].card[0]!=0x00fe && sd->status.inventory[index].card[0]!=(short)0xff00) {
					int j;
					for(j=0;j<sd->inventory_data[index]->slot;j++){	// ƒJ?ƒh
						int c=sd->status.inventory[index].card[j];
						if(c>0)
							run_script(itemdb_equipscript(c),0,sd->bl.id,0);
					}
				}
			}
		}
	}
	wele = sd->atk_ele;
	wele_ = sd->atk_ele_;
	def_ele = sd->def_ele;
	if(sd->status.pet_id > 0) {
		struct pet_data *pd=sd->pd;
		if((pd && battle_config.pet_status_support==1) && (battle_config.pet_equip_required==0 || (battle_config.pet_equip_required && pd->equip > 0))) {
			if(sd->status.pet_id > 0 && sd->petDB && sd->pet.intimate > 0 &&
				pd->state.skillbonus == 1) {
				pc_bonus(sd,pd->skillbonustype,pd->skillbonusval);
//				run_script(sd->petDB->script,0,sd->bl.id,0);
			}
			pele = sd->atk_ele;
			pdef_ele = sd->def_ele;
			sd->atk_ele = sd->def_ele = 0;
		}
	}
	memcpy(sd->paramcard,sd->parame,sizeof(sd->paramcard));

	// ?”õ•i‚É‚æ‚éƒXƒe?ƒ^ƒX?‰»‚Í‚±‚±‚Å?s
	for(i=0;i<10;i++) {
		current_equip_item_index = index = sd->equip_index[i]; //We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		if(index < 0)
			continue;
		if(i == 9 && sd->equip_index[8] == index)
			continue;
		if(i == 5 && sd->equip_index[4] == index)
			continue;
		if(i == 6 && (sd->equip_index[5] == index || sd->equip_index[4] == index))
			continue;
		if(sd->inventory_data[index]) {
			sd->def += sd->inventory_data[index]->def;
			if(sd->inventory_data[index]->type == 4) {
				int r,wlv = sd->inventory_data[index]->wlv;
				if(i == 8 && sd->status.inventory[index].equip == 0x20) {
					//“ñ“—¬—pƒf?ƒ^“ü—Í
					sd->watk_ += sd->inventory_data[index]->atk;
					sd->watk_2 = (r=sd->status.inventory[index].refine)*	// ¸?U?—Í
						refinebonus[wlv][0];
					if( (r-=refinebonus[wlv][2])>0 )	// ‰ß?¸?ƒ{?ƒiƒX
						sd->overrefine_ = r*refinebonus[wlv][1];

					if(sd->status.inventory[index].card[0]==0x00ff){	// »‘¢•Ší
						sd->star_ = (sd->status.inventory[index].card[1]>>8);	// ¯‚Ì‚©‚¯‚ç
						wele_= (sd->status.inventory[index].card[1]&0x0f);	// ? «
					}
					sd->attackrange_ += sd->inventory_data[index]->range;
					sd->state.lr_flag = 1;
					run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				}
				else {	//“ñ“—¬•ŠíˆÈŠO
					sd->watk += sd->inventory_data[index]->atk;
					sd->watk2 += (r=sd->status.inventory[index].refine)*	// ¸?U?—Í
						refinebonus[wlv][0];
					if( (r-=refinebonus[wlv][2])>0 )	// ‰ß?¸?ƒ{?ƒiƒX
						sd->overrefine += r*refinebonus[wlv][1];

					if(sd->status.inventory[index].card[0]==0x00ff){	// »‘¢•Ší
						sd->star += (sd->status.inventory[index].card[1]>>8);	// ¯‚Ì‚©‚¯‚ç
						wele = (sd->status.inventory[index].card[1]&0x0f);	// ? «
					}
					sd->attackrange += sd->inventory_data[index]->range;
					run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
				}
			}
			else if(sd->inventory_data[index]->type == 5) {
				sd->watk += sd->inventory_data[index]->atk;
				refinedef += sd->status.inventory[index].refine*refinebonus[0][0];
				run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
			}
		}
	}

	if(sd->equip_index[10] >= 0){ // –î
		index = sd->equip_index[10];
		if(sd->inventory_data[index]){		//‚Ü‚¾?«‚ª“ü‚Á‚Ä‚¢‚È‚¢
			sd->state.lr_flag = 2;
			run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			sd->arrow_atk += sd->inventory_data[index]->atk;
		}
	}
	sd->def += (refinedef+50)/100;

	if(sd->attackrange < 1) sd->attackrange = 1;
	if(sd->attackrange_ < 1) sd->attackrange_ = 1;
	if(sd->attackrange < sd->attackrange_)
		sd->attackrange = sd->attackrange_;
	if(sd->status.weapon == 11)
		sd->attackrange += sd->arrow_range;
	if(wele > 0)
		sd->atk_ele = wele;
	if(wele_ > 0)
		sd->atk_ele_ = wele_;
	if(def_ele > 0)
		sd->def_ele = def_ele;
	if(battle_config.pet_status_support) {
		if(pele > 0 && !sd->atk_ele)
			sd->atk_ele = pele;
		if(pdef_ele > 0 && !sd->def_ele)
			sd->def_ele = pdef_ele;
	}
	sd->double_rate += sd->double_add_rate;
	sd->perfect_hit += sd->perfect_hit_add;
	sd->get_zeny_num += sd->get_zeny_add_num;
	sd->splash_range += sd->splash_add_range;
	if(sd->speed_add_rate != 100)
		sd->speed_rate += sd->speed_add_rate - 100;
	if(sd->aspd_add_rate != 100)
		sd->aspd_rate += sd->aspd_add_rate - 100;

	// •ŠíATKƒTƒCƒY•â³ (‰Eè)
	sd->atkmods[0] = atkmods[0][sd->weapontype1];
	sd->atkmods[1] = atkmods[1][sd->weapontype1];
	sd->atkmods[2] = atkmods[2][sd->weapontype1];
	//•ŠíATKƒTƒCƒY•â³ (¶è)
	sd->atkmods_[0] = atkmods[0][sd->weapontype2];
	sd->atkmods_[1] = atkmods[1][sd->weapontype2];
	sd->atkmods_[2] = atkmods[2][sd->weapontype2];

	// jobƒ{?ƒiƒX•ª
	for(i=0;i<sd->status.job_level && i<MAX_LEVEL;i++){
		if(job_bonus[s_class.upper][s_class.job][i])
			sd->paramb[job_bonus[s_class.upper][s_class.job][i]-1]++;
	}

	if( (skill=pc_checkskill(sd,MC_INCCARRY))>0 )	// skill can be used with an item now, thanks to orn [Valaris]
		sd->max_weight += skill*2000;

	if( (skill=pc_checkskill(sd,AC_OWL))>0 )	// ‚Ó‚­‚ë‚¤‚Ì–Ú
		sd->paramb[4] += skill;

	if((skill=pc_checkskill(sd,BS_HILTBINDING))>0) {   // Hilt binding gives +1 str +4 atk
		sd->paramb[0] ++;
		sd->base_atk += 4;
	}
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0 ){ // Dragonology increases +1 int every 2 levels
		sd->paramb[3] += (int) ((skill+1)*0.5);
	}

	// ƒXƒe?ƒ^ƒX?‰»‚É‚æ‚éŠî–{ƒpƒ‰ƒ?ƒ^•â³
	if(sd->sc_count){
		if(sd->sc_data[SC_CONCENTRATE].timer!=-1 && sd->sc_data[SC_QUAGMIRE].timer == -1){	// W’†—ÍŒüã
			sd->paramb[1]+= (sd->status.agi+sd->paramb[1]+sd->parame[1]-sd->paramcard[1])*(2+sd->sc_data[SC_CONCENTRATE].val1)/100;
			sd->paramb[4]+= (sd->status.dex+sd->paramb[4]+sd->parame[4]-sd->paramcard[4])*(2+sd->sc_data[SC_CONCENTRATE].val1)/100;
		}
		if(sd->sc_data[SC_INCREASEAGI].timer!=-1){	// ‘¬“x?‰Á
			sd->paramb[1]+= 2+sd->sc_data[SC_INCREASEAGI].val1;
			sd->speed -= sd->speed *25/100;
		}
		if(sd->sc_data[SC_DECREASEAGI].timer!=-1) {	// ‘¬“xŒ¸­(agi‚Íbattle.c‚Å)
			sd->speed = sd->speed *125/100;
			sd->paramb[1] -= 2 + sd->sc_data[SC_DECREASEAGI].val1;	// reduce agility [celest]
		}
		if(sd->sc_data[SC_CLOAKING].timer!=-1) {
			sd->critical_rate += 100; // critical increases
			sd->speed = sd->speed * (sd->sc_data[SC_CLOAKING].val3-sd->sc_data[SC_CLOAKING].val1*3) /100;
		}
		if(sd->sc_data[SC_CHASEWALK].timer!=-1) {
			sd->speed = sd->speed * sd->sc_data[SC_CHASEWALK].val3 /100; // slow down by chasewalk
			if(sd->sc_data[SC_CHASEWALK].val4)
				sd->paramb[0] += (1<<(sd->sc_data[SC_CHASEWALK].val1-1)); // increases strength after 10 seconds
		}
		if(sd->sc_data[SC_SLOWDOWN].timer!=-1)
			sd->speed = sd->speed*150/100;
		if(sd->sc_data[SC_SPEEDUP0].timer!=-1 && sd->sc_data[SC_INCREASEAGI].timer==-1)
			sd->speed -= sd->speed*25/100;
		if(sd->sc_data[SC_BLESSING].timer!=-1){	// ƒuƒŒƒbƒVƒ“ƒO
			sd->paramb[0]+= sd->sc_data[SC_BLESSING].val1;
			sd->paramb[3]+= sd->sc_data[SC_BLESSING].val1;
			sd->paramb[4]+= sd->sc_data[SC_BLESSING].val1;
		}
		if(sd->sc_data[SC_GLORIA].timer!=-1)	// ƒOƒƒŠƒA
			sd->paramb[5]+= 30;
		if(sd->sc_data[SC_LOUD].timer!=-1 && sd->sc_data[SC_QUAGMIRE].timer == -1)	// ƒ‰ƒEƒhƒ{ƒCƒX
			sd->paramb[0]+= 4;
		if(sd->sc_data[SC_QUAGMIRE].timer!=-1){	// ƒNƒ@ƒOƒ}ƒCƒA
			//int agib = (sd->status.agi+sd->paramb[1]+sd->parame[1])*(sd->sc_data[SC_QUAGMIRE].val1*10)/100;
			//int dexb = (sd->status.dex+sd->paramb[4]+sd->parame[4])*(sd->sc_data[SC_QUAGMIRE].val1*10)/100;
			//sd->paramb[1]-= agib > 50 ? 50 : agib;
			//sd->paramb[4]-= dexb > 50 ? 50 : dexb;
			sd->paramb[1]-= sd->sc_data[SC_QUAGMIRE].val1*5;
			sd->paramb[4]-= sd->sc_data[SC_QUAGMIRE].val1*5;
			sd->speed = sd->speed*3/2;
		}
		if(sd->sc_data[SC_TRUESIGHT].timer!=-1){	// ƒgƒDƒ‹?ƒTƒCƒg
			sd->paramb[0]+= 5;
			sd->paramb[1]+= 5;
			sd->paramb[2]+= 5;
			sd->paramb[3]+= 5;
			sd->paramb[4]+= 5;
			sd->paramb[5]+= 5;
		}
		if(sd->sc_data[SC_MARIONETTE].timer!=-1){
			// skip partner checking -- should be handled in status_change_timer
			//struct map_session_data *psd = map_id2sd(sd->sc_data[SC_MARIONETTE2].val3);
			//if (psd) {	// if partner is found
				sd->paramb[0]-= sd->status.str/2;	// bonuses not included
				sd->paramb[1]-= sd->status.agi/2;
				sd->paramb[2]-= sd->status.vit/2;
				sd->paramb[3]-= sd->status.int_/2;
				sd->paramb[4]-= sd->status.dex/2;
				sd->paramb[5]-= sd->status.luk/2;
			//}
		}
		else if(sd->sc_data[SC_MARIONETTE2].timer!=-1){
			struct map_session_data *psd = map_id2sd(sd->sc_data[SC_MARIONETTE2].val3);
			if (psd) {	// if partner is found
				sd->paramb[0] += sd->status.str+psd->status.str/2 > 99 ? 99-sd->status.str : psd->status.str/2;
				sd->paramb[1] += sd->status.agi+psd->status.agi/2 > 99 ? 99-sd->status.agi : psd->status.agi/2;
				sd->paramb[2] += sd->status.vit+psd->status.vit/2 > 99 ? 99-sd->status.vit : psd->status.vit/2;
				sd->paramb[3] += sd->status.int_+psd->status.int_/2 > 99 ? 99-sd->status.int_ : psd->status.int_/2;
				sd->paramb[4] += sd->status.dex+psd->status.dex/2 > 99 ? 99-sd->status.dex : psd->status.dex/2;
				sd->paramb[5] += sd->status.luk+psd->status.luk/2 > 99 ? 99-sd->status.luk : psd->status.luk/2;
			}
		}
		if(sd->sc_data[SC_GOSPEL].timer!=-1 && sd->sc_data[SC_GOSPEL].val4 == BCT_PARTY){
			if (sd->sc_data[SC_GOSPEL].val3 == 6) {
				sd->paramb[0]+= 2;
				sd->paramb[1]+= 2;
				sd->paramb[2]+= 2;
				sd->paramb[3]+= 2;
				sd->paramb[4]+= 2;
				sd->paramb[5]+= 2;
			}
		}
		// New guild skills - Celest
		if (sd->sc_data[SC_BATTLEORDERS].timer != -1) {
			sd->paramb[0]+= 5;
			sd->paramb[3]+= 5;
			sd->paramb[4]+= 5;
		}
		if (sd->sc_data[SC_GUILDAURA].timer != -1) {
			if (sd->sc_data[SC_GUILDAURA].val4 & 1<<0)
				sd->paramb[0] += 2;
			if (sd->sc_data[SC_GUILDAURA].val4 & 1<<1)
				sd->paramb[2] += 2;
			if (sd->sc_data[SC_GUILDAURA].val4 & 1<<2)
				sd->paramb[1] += 2;
			if (sd->sc_data[SC_GUILDAURA].val4 & 1<<3)
				sd->paramb[4] += 2;				
		}
	}

	//1“x‚à€‚ñ‚Å‚È‚¢Job70ƒXƒpƒmƒr‚É+10
	if(s_class.job == 23 && sd->die_counter == 0 && sd->status.job_level >= 70){
		sd->paramb[0]+= 15;
		sd->paramb[1]+= 15;
		sd->paramb[2]+= 15;
		sd->paramb[3]+= 15;
		sd->paramb[4]+= 15;
		sd->paramb[5]+= 15;
	}
	sd->paramc[0]=sd->status.str+sd->paramb[0]+sd->parame[0];
	sd->paramc[1]=sd->status.agi+sd->paramb[1]+sd->parame[1];
	sd->paramc[2]=sd->status.vit+sd->paramb[2]+sd->parame[2];
	sd->paramc[3]=sd->status.int_+sd->paramb[3]+sd->parame[3];
	sd->paramc[4]=sd->status.dex+sd->paramb[4]+sd->parame[4];
	sd->paramc[5]=sd->status.luk+sd->paramb[5]+sd->parame[5];
	for(i=0;i<6;i++)
		if(sd->paramc[i] < 0) sd->paramc[i] = 0;

	if (sd->sc_count) {
		if (sd->sc_data[SC_CURSE].timer!=-1)
			sd->paramc[5] = 0;
	}

	if(sd->status.weapon == 11 || sd->status.weapon == 13 || sd->status.weapon == 14) {
		str = sd->paramc[4];
		dex = sd->paramc[0];
	}
	else {
		str = sd->paramc[0];
		dex = sd->paramc[4];
	}
	dstr = str/10;
	sd->base_atk += str + dstr*dstr + dex/5 + sd->paramc[5]/5;
	sd->matk1 += sd->paramc[3]+(sd->paramc[3]/5)*(sd->paramc[3]/5);
	sd->matk2 += sd->paramc[3]+(sd->paramc[3]/7)*(sd->paramc[3]/7);
	if(sd->matk1 < sd->matk2) {
		int temp = sd->matk2;
		sd->matk2 = sd->matk1;
		sd->matk1 = temp;
	}
	sd->hit += sd->paramc[4] + sd->status.base_level;
	sd->flee += sd->paramc[1] + sd->status.base_level;
	sd->def2 += sd->paramc[2];
	sd->mdef2 += sd->paramc[3];
	sd->flee2 += sd->paramc[5]+10;
	sd->critical += (sd->paramc[5]*3)+10;

	if(sd->base_atk < 1)
		sd->base_atk = 1;
	if(sd->critical_rate != 100)
		sd->critical = (sd->critical*sd->critical_rate)/100;
	if(sd->critical < 10) sd->critical = 10;
	if(sd->hit_rate != 100)
		sd->hit = (sd->hit*sd->hit_rate)/100;
	if(sd->hit < 1) sd->hit = 1;
	if(sd->flee_rate != 100)
		sd->flee = (sd->flee*sd->flee_rate)/100;
	if(sd->flee < 1) sd->flee = 1;
	if(sd->flee2_rate != 100)
		sd->flee2 = (sd->flee2*sd->flee2_rate)/100;
	if(sd->flee2 < 10) sd->flee2 = 10;
	if(sd->def_rate != 100)
		sd->def = (sd->def*sd->def_rate)/100;
	if(sd->def < 0) sd->def = 0;
	if(sd->def2_rate != 100)
		sd->def2 = (sd->def2*sd->def2_rate)/100;
	if(sd->def2 < 1) sd->def2 = 1;
	if(sd->mdef_rate != 100)
		sd->mdef = (sd->mdef*sd->mdef_rate)/100;
	if(sd->mdef < 0) sd->mdef = 0;
	if(sd->mdef2_rate != 100)
		sd->mdef2 = (sd->mdef2*sd->mdef2_rate)/100;
	if(sd->mdef2 < 1) sd->mdef2 = 1;

	// “ñ“—¬ ASPD C³
	if (sd->status.weapon <= 16)
		sd->aspd += aspd_base[s_class.job][sd->status.weapon]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[s_class.job][sd->status.weapon]/1000;
	else
		sd->aspd += (
			(aspd_base[s_class.job][sd->weapontype1]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[s_class.job][sd->weapontype1]/1000) +
			(aspd_base[s_class.job][sd->weapontype2]-(sd->paramc[1]*4+sd->paramc[4])*aspd_base[s_class.job][sd->weapontype2]/1000)
			) * 140 / 200;

	aspd_rate = sd->aspd_rate;

	//U?‘¬“x?‰Á

	if((skill=pc_checkskill(sd,AC_VULTURE))>0){	// ƒƒV‚Ì–Ú
		sd->hit += skill;
		if(sd->status.weapon == 11)
			sd->attackrange += skill;
	}

	if( (skill=pc_checkskill(sd,BS_WEAPONRESEARCH))>0)	// •Ší?‹†‚Ì–½’†—¦?‰Á
		sd->hit += skill*2;
	if(sd->status.option&2 && (skill = pc_checkskill(sd,RG_TUNNELDRIVE))>0 )	// ƒgƒ“ƒlƒ‹ƒhƒ‰ƒCƒu	// ƒgƒ“ƒlƒ‹ƒhƒ‰ƒCƒu
		sd->speed += (100-16*skill)*DEFAULT_WALK_SPEED/100;
		//sd->speed += (1.2*DEFAULT_WALK_SPEED - skill*9);		
	if (pc_iscarton(sd) && (skill=pc_checkskill(sd,MC_PUSHCART))>0)	// ƒJ?ƒg‚É‚æ‚é‘¬“x’á‰º
		sd->speed += (short) ((10-skill) * (DEFAULT_WALK_SPEED * 0.1));
	else if (pc_isriding(sd)) {	// ƒyƒRƒyƒR?‚è‚É‚æ‚é‘¬“x?‰Á
		sd->speed -= (short) ((0.25 * DEFAULT_WALK_SPEED));
		sd->max_weight += 10000;
	}
	if((skill=pc_checkskill(sd,CR_TRUST))>0) { // ƒtƒFƒCƒX
		sd->status.max_hp += skill*200;
		sd->subele[6] += skill*5;
	}
	if((skill=pc_checkskill(sd,BS_SKINTEMPER))>0) {
		sd->subele[0] += skill;
		sd->subele[3] += skill*5;
	}
	if((skill=pc_checkskill(sd,SA_ADVANCEDBOOK))>0 )
		aspd_rate -= (int) (skill*0.5);

	bl=sd->status.base_level;

	sd->status.max_hp += (3500 + bl*hp_coefficient2[s_class.job] + hp_sigma_val[s_class.job][(bl > 0)? bl-1:0])/100 * (100 + sd->paramc[2])/100 + (sd->parame[2] - sd->paramcard[2]);
		if (s_class.upper==1) // [MouseJstr]
			sd->status.max_hp = sd->status.max_hp * 130/100;
		else if (s_class.upper==2)
			sd->status.max_hp = sd->status.max_hp * 70/100;

	if (sd->hprate <= 0)
		sd->hprate = 1;
	if(sd->hprate!=100)
		sd->status.max_hp = sd->status.max_hp*sd->hprate/100;

	if(sd->sc_count && sd->sc_data[SC_BERSERK].timer!=-1){	// ƒo?ƒT?ƒN
		sd->status.max_hp = sd->status.max_hp * 3;
		// sd->status.hp = sd->status.hp * 3;
		if(sd->status.max_hp > battle_config.max_hp) // removed negative max hp bug by Valaris
			sd->status.max_hp = battle_config.max_hp;
		if(sd->status.hp > battle_config.max_hp) // removed negative max hp bug by Valaris
			sd->status.hp = battle_config.max_hp;
	}
	if(s_class.job == 23 && sd->status.base_level >= 99){
		sd->status.max_hp = sd->status.max_hp + 2000;
	}

	if(sd->status.max_hp > battle_config.max_hp) // removed negative max hp bug by Valaris
		sd->status.max_hp = battle_config.max_hp;
	if(sd->status.max_hp <= 0) sd->status.max_hp = 1; // end

	// Å‘åSPŒvZ
	sd->status.max_sp += ((sp_coefficient[s_class.job] * bl) + 1000)/100 * (100 + sd->paramc[3])/100 + (sd->parame[3] - sd->paramcard[3]);
		if (s_class.upper==1) // [MouseJstr]
			sd->status.max_sp = sd->status.max_sp * 130/100;
		else if (s_class.upper==2)
			sd->status.max_sp = sd->status.max_sp * 70/100;
	if (sd->sprate <= 0)
		sd->sprate = 1;
	if(sd->sprate!=100)
		sd->status.max_sp = sd->status.max_sp*sd->sprate/100;

	if((skill=pc_checkskill(sd,HP_MEDITATIO))>0) // ƒƒfƒBƒeƒCƒeƒBƒI
		sd->status.max_sp += sd->status.max_sp*skill/100;
	if((skill=pc_checkskill(sd,HW_SOULDRAIN))>0) /* ƒ\ƒEƒ‹ƒhƒŒƒCƒ“ */
		sd->status.max_sp += sd->status.max_sp*2*skill/100;

	if(sd->status.max_sp < 0 || sd->status.max_sp > battle_config.max_sp)
		sd->status.max_sp = battle_config.max_sp;

	//©‘R‰ñ•œHP
	sd->nhealhp = 1 + (sd->paramc[2]/5) + (sd->status.max_hp/200);
	if((skill=pc_checkskill(sd,SM_RECOVERY)) > 0) {	/* HP‰ñ•œ—ÍŒüã */
		sd->nshealhp = skill*5 + (sd->status.max_hp*skill/500);
		if(sd->nshealhp > 0x7fff) sd->nshealhp = 0x7fff;
	}
	//©‘R‰ñ•œSP
	sd->nhealsp = 1 + (sd->paramc[3]/6) + (sd->status.max_sp/100);
	if(sd->paramc[3] >= 120)
		sd->nhealsp += ((sd->paramc[3]-120)>>1) + 4;
	if((skill=pc_checkskill(sd,MG_SRECOVERY)) > 0) { /* SP‰ñ•œ—ÍŒüã */
		sd->nshealsp = skill*3 + (sd->status.max_sp*skill/500);
		if(sd->nshealsp > 0x7fff) sd->nshealsp = 0x7fff;
	}

	if((skill = pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0) {
		sd->nsshealhp = skill*4 + (sd->status.max_hp*skill/500);
		sd->nsshealsp = skill*2 + (sd->status.max_sp*skill/500);
		if(sd->nsshealhp > 0x7fff) sd->nsshealhp = 0x7fff;
		if(sd->nsshealsp > 0x7fff) sd->nsshealsp = 0x7fff;
	}
	if(sd->hprecov_rate != 100) {
		sd->nhealhp = sd->nhealhp*sd->hprecov_rate/100;
		if(sd->nhealhp < 1) sd->nhealhp = 1;
	}
	if(sd->sprecov_rate != 100) {
		sd->nhealsp = sd->nhealsp*sd->sprecov_rate/100;
		if(sd->nhealsp < 1) sd->nhealsp = 1;
	}
	/* if((skill=pc_checkskill(sd,HP_MEDITATIO)) > 0) { // f?fffBfefCfefBfI,I'SPR,A*,I',E`,¡©Z((c)¡®R¢¶n~.©«,E',(c),(c),e'
		sd->nhealsp += 3*skill*(sd->status.max_sp)/100;
		if(sd->nhealsp > 0x7fff) sd->nhealsp = 0x7fff;
		} Increase natural SP regen instead of colossal SP Recovery effect [DracoRPG]*/

	// í‘°‘Ï«i‚±‚ê‚Å‚¢‚¢‚ÌH ƒfƒBƒoƒCƒ“ƒvƒƒeƒNƒVƒ‡ƒ“‚Æ“¯‚¶?—‚ª‚¢‚é‚©‚àj
	if( (skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0 ){ // ƒhƒ‰ƒSƒmƒƒW?
		skill = skill*4;
		sd->addrace[9]+=skill;
		sd->addrace_[9]+=skill;
		sd->subrace[9]+=skill;
		sd->magic_addrace[9]+=skill;
		sd->magic_subrace[9]-=skill;
	}

	//Fleeã¸
	if( (skill=pc_checkskill(sd,TF_MISS))>0 ){	// ‰ñ”ğ—¦?‰Á
		sd->flee += skill*((sd->status.class_==12 || sd->status.class_==17 || sd->status.class_==4013 || sd->status.class_==4018) ? 4 : 3);
		if((sd->status.class_==12 || sd->status.class_==4013) && (sd->sc_count && sd->sc_data[SC_CLOAKING].timer==-1))
			sd->speed -= (short)(skill*1.5/100 * DEFAULT_WALK_SPEED);
	}
	if( (skill=pc_checkskill(sd,MO_DODGE))>0 )	// Œ©Ø‚è
		sd->flee += (skill*3)>>1;

	// ƒXƒLƒ‹‚âƒXƒe?ƒ^ƒXˆÙí‚É‚æ‚é?‚è‚Ìƒpƒ‰ƒ?ƒ^•â³
	if(sd->sc_count){
		// ATK/DEF?‰»Œ`
		if(sd->sc_data[SC_ANGELUS].timer!=-1)	// ƒGƒ“ƒWƒFƒ‰ƒX
			sd->def2 = sd->def2*(110+5*sd->sc_data[SC_ANGELUS].val1)/100;
		if(sd->sc_data[SC_IMPOSITIO].timer!=-1)	{// ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX
			sd->watk += sd->sc_data[SC_IMPOSITIO].val1*5;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == 4)
				sd->watk_ += sd->sc_data[SC_IMPOSITIO].val1*5;
		}
		if(sd->sc_data[SC_PROVOKE].timer!=-1){	// ƒvƒƒ{ƒbƒN
			sd->def2 = sd->def2*(100-6*sd->sc_data[SC_PROVOKE].val1)/100;
			sd->base_atk = sd->base_atk*(100+2*sd->sc_data[SC_PROVOKE].val1)/100;
			sd->watk = sd->watk*(100+2*sd->sc_data[SC_PROVOKE].val1)/100;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == 4)
				sd->watk_ = sd->watk_*(100+2*sd->sc_data[SC_PROVOKE].val1)/100;
		}
		if(sd->sc_data[SC_ENDURE].timer!=-1)
			sd->mdef2 += sd->sc_data[SC_ENDURE].val1;
		if(sd->sc_data[SC_MINDBREAKER].timer!=-1){	// ƒvƒƒ{ƒbƒN
			sd->mdef2 = sd->mdef2*(100-6*sd->sc_data[SC_MINDBREAKER].val1)/100;
			sd->matk1 = sd->matk1*(100+2*sd->sc_data[SC_MINDBREAKER].val1)/100;
			sd->matk2 = sd->matk2*(100+2*sd->sc_data[SC_MINDBREAKER].val1)/100;
		}
		if(sd->sc_data[SC_POISON].timer!=-1)	// “Å?‘Ô
			sd->def2 = sd->def2*75/100;
		if(sd->sc_data[SC_CURSE].timer!=-1){
			sd->base_atk = sd->base_atk*75/100;
			sd->watk = sd->watk*75/100;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == 4)
				sd->watk_ = sd->watk_*75/100;
		}
		if(sd->sc_data[SC_DRUMBATTLE].timer!=-1){	// ?‘¾ŒÛ‚Ì‹¿‚«
			sd->watk += sd->sc_data[SC_DRUMBATTLE].val2;
			sd->def  += sd->sc_data[SC_DRUMBATTLE].val3;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == 4)
				sd->watk_ += sd->sc_data[SC_DRUMBATTLE].val2;
		}
		if(sd->sc_data[SC_NIBELUNGEN].timer!=-1) {	// ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö
			index = sd->equip_index[9];
			/*if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 3)
				sd->watk += sd->sc_data[SC_NIBELUNGEN].val3;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 3)
				sd->watk_ += sd->sc_data[SC_NIBELUNGEN].val3;
			index = sd->equip_index[9];*/
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				sd->watk2 += sd->sc_data[SC_NIBELUNGEN].val3;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				sd->watk_2 += sd->sc_data[SC_NIBELUNGEN].val3;
		}

		if(sd->sc_data[SC_VOLCANO].timer!=-1 && sd->def_ele==3){	// ƒ{ƒ‹ƒP?ƒm
			sd->watk += sd->sc_data[SC_VOLCANO].val3;
		}

		if(sd->sc_data[SC_SIGNUMCRUCIS].timer!=-1)
			sd->def = sd->def * (100 - sd->sc_data[SC_SIGNUMCRUCIS].val2)/100;
		if(sd->sc_data[SC_ETERNALCHAOS].timer!=-1)	// ƒGƒ^?ƒiƒ‹ƒJƒIƒX
			sd->def=0;

		if(sd->sc_data[SC_CONCENTRATION].timer!=-1){ //ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“
			sd->watk = sd->watk * (100 + 5*sd->sc_data[SC_CONCENTRATION].val1)/100;
			index = sd->equip_index[8];
			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == 4)
				sd->watk_ = sd->watk * (100 + 5*sd->sc_data[SC_CONCENTRATION].val1)/100;
			sd->def = sd->def * (100 - 5*sd->sc_data[SC_CONCENTRATION].val1)/100;
		}

		if(sd->sc_data[SC_MAGICPOWER].timer!=-1){ //–‚–@—Í?•
			sd->matk1 = sd->matk1*(100+5*sd->sc_data[SC_MAGICPOWER].val1)/100;
			sd->matk2 = sd->matk2*(100+5*sd->sc_data[SC_MAGICPOWER].val1)/100;
		}
		if(sd->sc_data[SC_ATKPOT].timer!=-1)
			sd->watk += sd->sc_data[SC_ATKPOT].val1;
		if(sd->sc_data[SC_MATKPOT].timer!=-1){
			sd->matk1 += sd->sc_data[SC_MATKPOT].val1;
			sd->matk2 += sd->sc_data[SC_MATKPOT].val1;
		}

		// ASPD/ˆÚ“®‘¬“x?‰»Œn
		if(sd->sc_data[SC_TWOHANDQUICKEN].timer != -1 && sd->sc_data[SC_QUAGMIRE].timer == -1 && sd->sc_data[SC_DONTFORGETME].timer == -1)	// 2HQ
			aspd_rate -= 30;
		if(sd->sc_data[SC_ADRENALINE].timer != -1 && sd->sc_data[SC_TWOHANDQUICKEN].timer == -1 &&
			sd->sc_data[SC_QUAGMIRE].timer == -1 && sd->sc_data[SC_DONTFORGETME].timer == -1) {	// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
			if(sd->sc_data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)
				aspd_rate -= 30;
			else
				aspd_rate -= 25;
		}
		if(sd->sc_data[SC_SPEARSQUICKEN].timer != -1 && sd->sc_data[SC_ADRENALINE].timer == -1 &&
			sd->sc_data[SC_TWOHANDQUICKEN].timer == -1 && sd->sc_data[SC_QUAGMIRE].timer == -1 && sd->sc_data[SC_DONTFORGETME].timer == -1)	// ƒXƒsƒAƒNƒBƒbƒPƒ“
			aspd_rate -= sd->sc_data[SC_SPEARSQUICKEN].val2;
		if(sd->sc_data[SC_ASSNCROS].timer!=-1 && // —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
			sd->sc_data[SC_TWOHANDQUICKEN].timer==-1 && sd->sc_data[SC_ADRENALINE].timer==-1 && sd->sc_data[SC_SPEARSQUICKEN].timer==-1 &&
			sd->sc_data[SC_DONTFORGETME].timer == -1)
				aspd_rate -= 5+sd->sc_data[SC_ASSNCROS].val1+sd->sc_data[SC_ASSNCROS].val2+sd->sc_data[SC_ASSNCROS].val3;
		if(sd->sc_data[SC_DONTFORGETME].timer!=-1){		// „‚ğ–Y‚ê‚È‚¢‚Å
			aspd_rate += sd->sc_data[SC_DONTFORGETME].val1*3 + sd->sc_data[SC_DONTFORGETME].val2 + (sd->sc_data[SC_DONTFORGETME].val3>>16);
			sd->speed= sd->speed*(100+sd->sc_data[SC_DONTFORGETME].val1*2 + sd->sc_data[SC_DONTFORGETME].val2 + (sd->sc_data[SC_DONTFORGETME].val3&0xffff))/100;
		}
		if(	sd->sc_data[i=SC_SPEEDPOTION3].timer!=-1 ||
			sd->sc_data[i=SC_SPEEDPOTION2].timer!=-1 ||
			sd->sc_data[i=SC_SPEEDPOTION1].timer!=-1 ||
			sd->sc_data[i=SC_SPEEDPOTION0].timer!=-1)	// ? ‘¬ƒ|?ƒVƒ‡ƒ“
			aspd_rate -= sd->sc_data[i].val2;
		if(sd->sc_data[SC_WINDWALK].timer!=-1 && sd->sc_data[SC_INCREASEAGI].timer==-1)	//ƒEƒBƒ“ƒhƒEƒH?ƒNbÍLv*2%Œ¸Z
			sd->speed -= sd->speed *(sd->sc_data[SC_WINDWALK].val1*2)/100;
		if(sd->sc_data[SC_CARTBOOST].timer!=-1)	// ƒJ?ƒgƒu?ƒXƒg
		sd->speed -= (DEFAULT_WALK_SPEED * 20)/100;
		if(sd->sc_data[SC_BERSERK].timer!=-1)	//ƒo?ƒT?ƒN’†‚ÍIA‚Æ“¯‚¶‚®‚ç‚¢‘¬‚¢H
			sd->speed -= sd->speed *25/100;
		if(sd->sc_data[SC_WEDDING].timer!=-1)	//Œ‹¥’†‚Í?‚­‚Ì‚ª?‚¢
			sd->speed = 2*DEFAULT_WALK_SPEED;

		// HIT/FLEE?‰»Œn
		if(sd->sc_data[SC_WHISTLE].timer!=-1){  // Œû“J
			sd->flee += sd->flee * (sd->sc_data[SC_WHISTLE].val1
					+sd->sc_data[SC_WHISTLE].val2+(sd->sc_data[SC_WHISTLE].val3>>16))/100;
			sd->flee2+= (sd->sc_data[SC_WHISTLE].val1+sd->sc_data[SC_WHISTLE].val2+(sd->sc_data[SC_WHISTLE].val3&0xffff)) * 10;
		}
		if(sd->sc_data[SC_HUMMING].timer!=-1)  // ƒnƒ~ƒ“ƒO
			sd->hit += (sd->sc_data[SC_HUMMING].val1*2+sd->sc_data[SC_HUMMING].val2
					+sd->sc_data[SC_HUMMING].val3) * sd->hit/100;
		if(sd->sc_data[SC_VIOLENTGALE].timer!=-1 && sd->def_ele==4){	// ƒoƒCƒIƒŒƒ“ƒgƒQƒCƒ‹
			sd->flee += sd->flee*sd->sc_data[SC_VIOLENTGALE].val3/100;
		}
		if(sd->sc_data[SC_BLIND].timer!=-1){	// ˆÃ?
			sd->hit -= sd->hit*25/100;
			sd->flee -= sd->flee*25/100;
		}
		if(sd->sc_data[SC_WINDWALK].timer!=-1) // ƒEƒBƒ“ƒhƒEƒH?ƒN
			sd->flee += sd->flee*(sd->sc_data[SC_WINDWALK].val2)/100;
		if(sd->sc_data[SC_SPIDERWEB].timer!=-1) //ƒXƒpƒCƒ_?ƒEƒFƒu
			sd->flee -= sd->flee*50/100;
		if(sd->sc_data[SC_TRUESIGHT].timer!=-1) //ƒgƒDƒ‹?ƒTƒCƒg
			sd->hit += 3*(sd->sc_data[SC_TRUESIGHT].val1);
		if(sd->sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“
			sd->hit += (10*(sd->sc_data[SC_CONCENTRATION].val1));

		// ‘Ï«
		if(sd->sc_data[SC_SIEGFRIED].timer!=-1){  // •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh
			sd->subele[1] += sd->sc_data[SC_SIEGFRIED].val2;	// …
			sd->subele[2] += sd->sc_data[SC_SIEGFRIED].val2;	// …
			sd->subele[3] += sd->sc_data[SC_SIEGFRIED].val2;	// ‰Î
			sd->subele[4] += sd->sc_data[SC_SIEGFRIED].val2;	// …
			sd->subele[5] += sd->sc_data[SC_SIEGFRIED].val2;	// …
			sd->subele[6] += sd->sc_data[SC_SIEGFRIED].val2;	// …
			sd->subele[7] += sd->sc_data[SC_SIEGFRIED].val2;	// …
			sd->subele[8] += sd->sc_data[SC_SIEGFRIED].val2;	// …
			sd->subele[9] += sd->sc_data[SC_SIEGFRIED].val2;	// …
		}
		if(sd->sc_data[SC_PROVIDENCE].timer!=-1){	// ƒvƒƒ”ƒBƒfƒ“ƒX
			sd->subele[6] += sd->sc_data[SC_PROVIDENCE].val2;	// ? ¹?«
			sd->subrace[6] += sd->sc_data[SC_PROVIDENCE].val2;	// ? ?–‚
		}

		// ‚»‚Ì‘¼
		if(sd->sc_data[SC_APPLEIDUN].timer!=-1){	// ƒCƒhƒDƒ“‚Ì—ÑŒç
			sd->status.max_hp += ((5+sd->sc_data[SC_APPLEIDUN].val1*2+((sd->sc_data[SC_APPLEIDUN].val2+1)>>1)
						+sd->sc_data[SC_APPLEIDUN].val3/10) * sd->status.max_hp)/100;
			if(sd->status.max_hp < 0 || sd->status.max_hp > battle_config.max_hp)
				sd->status.max_hp = battle_config.max_hp;
		}
		if(sd->sc_data[SC_DELUGE].timer!=-1 && sd->def_ele==1){	// ƒfƒŠƒ…?ƒW
			sd->status.max_hp += sd->status.max_hp * deluge_eff[sd->sc_data[SC_DELUGE].val1-1]/100;
			if(sd->status.max_hp < 0 || sd->status.max_hp > battle_config.max_hp)
				sd->status.max_hp = battle_config.max_hp;
		}
		if(sd->sc_data[SC_SERVICE4U].timer!=-1) {	// ƒT?ƒrƒXƒtƒH?ƒ†?
			sd->status.max_sp += sd->status.max_sp*(10+sd->sc_data[SC_SERVICE4U].val1+sd->sc_data[SC_SERVICE4U].val2
						+sd->sc_data[SC_SERVICE4U].val3)/100;
			if(sd->status.max_sp < 0 || sd->status.max_sp > battle_config.max_sp)
				sd->status.max_sp = battle_config.max_sp;
			sd->dsprate-=(10+sd->sc_data[SC_SERVICE4U].val1*3+sd->sc_data[SC_SERVICE4U].val2
					+sd->sc_data[SC_SERVICE4U].val3);
			if(sd->dsprate<0)sd->dsprate=0;
		}

		if(sd->sc_data[SC_FORTUNE].timer!=-1)	// K‰^‚ÌƒLƒX
			sd->critical += (10+sd->sc_data[SC_FORTUNE].val1+sd->sc_data[SC_FORTUNE].val2
						+sd->sc_data[SC_FORTUNE].val3)*10;

		if(sd->sc_data[SC_EXPLOSIONSPIRITS].timer!=-1){	// ”š—ô”g“®
			if(s_class.job==23)
				sd->critical += sd->sc_data[SC_EXPLOSIONSPIRITS].val1*100;
			else
			sd->critical += sd->sc_data[SC_EXPLOSIONSPIRITS].val2;
		}

		if(sd->sc_data[SC_STEELBODY].timer!=-1){	// ‹à„
			sd->def = 90;
			sd->mdef = 90;
			aspd_rate += 25;
			sd->speed = (sd->speed * 125) / 100;
		}
		if(sd->sc_data[SC_DEFENDER].timer != -1) {
			//sd->aspd += (550 - sd->sc_data[SC_DEFENDER].val1*50);
			aspd_rate += (25 - sd->sc_data[SC_DEFENDER].val1*5);
			sd->speed = (sd->speed * (155 - sd->sc_data[SC_DEFENDER].val1*5)) / 100;
		}
		if(sd->sc_data[SC_ENCPOISON].timer != -1)
			sd->addeff[4] += sd->sc_data[SC_ENCPOISON].val2;

		if( sd->sc_data[SC_DANCING].timer!=-1 ){		// ‰‰‘t/ƒ_ƒ“ƒXg—p’†
			sd->speed = (short) ((double)sd->speed * (6.- 0.4 * pc_checkskill(sd, ((s_class.job == 19) ? BA_MUSICALLESSON : DC_DANCINGLESSON))));
			//sd->speed*=4;
			sd->nhealsp = 0;
			sd->nshealsp = 0;
			sd->nsshealsp = 0;
		}
		if(sd->sc_data[SC_CURSE].timer!=-1)
			sd->speed += 450;

		if(sd->sc_data[SC_TRUESIGHT].timer!=-1) //ƒgƒDƒ‹?ƒTƒCƒg
			sd->critical += sd->critical*(sd->sc_data[SC_TRUESIGHT].val1)/100;

/*		if(sd->sc_data[SC_VOLCANO].timer!=-1)	// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“(?«‚Íbattle.c‚Å)
			sd->addeff[2]+=sd->sc_data[SC_VOLCANO].val2;//% of granting
		if(sd->sc_data[SC_DELUGE].timer!=-1)	// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“(?«‚Íbattle.c‚Å)
			sd->addeff[0]+=sd->sc_data[SC_DELUGE].val2;//% of granting
		*/
		if(sd->sc_data[SC_BERSERK].timer!=-1) {	//All Def/MDef reduced to 0 while in Berserk [DracoRPG]
			sd->def = sd->def2 = 0;
			sd->mdef = sd->mdef2 = 0;
			sd->flee -= sd->flee*50/100;
			aspd_rate -= 30;
			//sd->base_atk *= 3;
		}
		if(sd->sc_data[SC_KEEPING].timer!=-1)
			sd->def = 100;
		if(sd->sc_data[SC_BARRIER].timer!=-1)
			sd->mdef = 100;

		if(sd->sc_data[SC_JOINTBEAT].timer!=-1) { // Random break [DracoRPG]
			switch(sd->sc_data[SC_JOINTBEAT].val2) {
			case 1: //Ankle break
				sd->speed_rate += 50;
				break;
			case 2:	//Wrist	break
				sd->aspd_rate += 25;
				break;
			case 3:	//Knee break
				sd->speed_rate += 30;
				sd->aspd_rate += 10;
				break;
			case 4:	//Shoulder break
				sd->def2 -= sd->def2*50/100;
				break;
			case 5:	//Waist	break
				sd->def2 -= sd->def2*50/100;
				sd->base_atk -= sd->base_atk*25/100;
				break;
			}
		}

		if(sd->sc_data[SC_GOSPEL].timer!=-1) {
			if (sd->sc_data[SC_GOSPEL].val4 == BCT_PARTY){
				switch (sd->sc_data[SC_GOSPEL].val3)
				{
				case 4:
					sd->status.max_hp += sd->status.max_hp * 25 / 100;
					if(sd->status.max_hp > battle_config.max_hp)
						sd->status.max_hp = battle_config.max_hp;
					break;
				case 5:
					sd->status.max_sp += sd->status.max_sp * 25 / 100;
					if(sd->status.max_sp > battle_config.max_sp)
						sd->status.max_sp = battle_config.max_sp;
					break;
				case 11:
					sd->def += sd->def * 25 / 100;
					sd->def2 += sd->def2 * 25 / 100;
					break;
				case 12:
					sd->base_atk += sd->base_atk * 8 / 100;
					break;
				case 13:
					sd->flee += sd->flee * 5 / 100;
					break;
				case 14:
					sd->hit += sd->hit * 5 / 100;
					break;
				}
			} else if (sd->sc_data[SC_GOSPEL].val4 == BCT_ENEMY){
				switch (sd->sc_data[SC_GOSPEL].val3)
				{
					case 5:
						sd->def = 0;
						sd->def2 = 0;
						break;
					case 6:
						sd->base_atk = 0;
						sd->watk = 0;
						sd->watk2 = 0;
						break;
					case 7:
						sd->flee = 0;
						break;
					case 8:
						sd->speed_rate += 75;
						aspd_rate += 75;
						break;
				}
			}
		}
		// custom stats, since there's no info on how much it actually gives ^^; [Celest]
		if (sd->sc_data[SC_GUILDAURA].timer != -1) {
			if (sd->sc_data[SC_GUILDAURA].val4 & 1<<4) {
				sd->hit += 10;
				sd->flee += 10;
			}
		}
	}

	if (sd->speed_rate <= 0)
		sd->speed_rate = 1;

	if(sd->speed_rate != 100)
		sd->speed = sd->speed*sd->speed_rate/100;
	if(sd->speed < 1) sd->speed = 1;
	if(aspd_rate != 100)
		sd->aspd = sd->aspd*aspd_rate/100;
	if(pc_isriding(sd))							// ‹R•ºC—û
		sd->aspd = sd->aspd*(100 + 10*(5 - pc_checkskill(sd,KN_CAVALIERMASTERY)))/ 100;
	if(sd->aspd < battle_config.max_aspd) sd->aspd = battle_config.max_aspd;
	sd->amotion = sd->aspd;
	sd->dmotion = 800-sd->paramc[1]*4;
	if(sd->dmotion<400)
		sd->dmotion = 400;
	if(sd->skilltimer != -1 && (skill = pc_checkskill(sd,SA_FREECAST)) > 0) {
		sd->prev_speed = sd->speed;
		sd->speed = sd->speed*(175 - skill*5)/100;
	}

	if(sd->status.hp>sd->status.max_hp)
		sd->status.hp=sd->status.max_hp;
	if(sd->status.sp>sd->status.max_sp)
		sd->status.sp=sd->status.max_sp;

	if(first&4)
		return 0;
	if(first&3) {
		clif_updatestatus(sd,SP_SPEED);
		clif_updatestatus(sd,SP_MAXHP);
		clif_updatestatus(sd,SP_MAXSP);
		if(first&1) {
			clif_updatestatus(sd,SP_HP);
			clif_updatestatus(sd,SP_SP);
		}
		return 0;
	}

	if(b_class != sd->view_class) {
		clif_changelook(&sd->bl,LOOK_BASE,sd->view_class);
#if PACKETVER < 4
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
#else
		clif_changelook(&sd->bl,LOOK_WEAPON,0);
#endif
	}

	if( memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill)) || b_attackrange != sd->attackrange)
		clif_skillinfoblock(sd);	// ƒXƒLƒ‹‘—M

	if(b_speed != sd->speed)
		clif_updatestatus(sd,SP_SPEED);
	if(b_weight != sd->weight)
		clif_updatestatus(sd,SP_WEIGHT);
	if(b_max_weight != sd->max_weight) {
		clif_updatestatus(sd,SP_MAXWEIGHT);
		pc_checkweighticon(sd);
	}
	for(i=0;i<6;i++)
		if(b_paramb[i] + b_parame[i] != sd->paramb[i] + sd->parame[i])
			clif_updatestatus(sd,SP_STR+i);
	if(b_hit != sd->hit)
		clif_updatestatus(sd,SP_HIT);
	if(b_flee != sd->flee)
		clif_updatestatus(sd,SP_FLEE1);
	if(b_aspd != sd->aspd)
		clif_updatestatus(sd,SP_ASPD);
	if(b_watk != sd->watk || b_base_atk != sd->base_atk)
		clif_updatestatus(sd,SP_ATK1);
	if(b_def != sd->def)
		clif_updatestatus(sd,SP_DEF1);
	if(b_watk2 != sd->watk2)
		clif_updatestatus(sd,SP_ATK2);
	if(b_def2 != sd->def2)
		clif_updatestatus(sd,SP_DEF2);
	if(b_flee2 != sd->flee2)
		clif_updatestatus(sd,SP_FLEE2);
	if(b_critical != sd->critical)
		clif_updatestatus(sd,SP_CRITICAL);
	if(b_matk1 != sd->matk1)
		clif_updatestatus(sd,SP_MATK1);
	if(b_matk2 != sd->matk2)
		clif_updatestatus(sd,SP_MATK2);
	if(b_mdef != sd->mdef)
		clif_updatestatus(sd,SP_MDEF1);
	if(b_mdef2 != sd->mdef2)
		clif_updatestatus(sd,SP_MDEF2);
	if(b_attackrange != sd->attackrange)
		clif_updatestatus(sd,SP_ATTACKRANGE);
	if(b_max_hp != sd->status.max_hp)
		clif_updatestatus(sd,SP_MAXHP);
	if(b_max_sp != sd->status.max_sp)
		clif_updatestatus(sd,SP_MAXSP);
	if(b_hp != sd->status.hp)
		clif_updatestatus(sd,SP_HP);
	if(b_sp != sd->status.sp)
		clif_updatestatus(sd,SP_SP);

/*	if(before.cart_num != before.cart_num || before.cart_max_num != before.cart_max_num ||
		before.cart_weight != before.cart_weight || before.cart_max_weight != before.cart_max_weight )
		clif_updatestatus(sd,SP_CARTINFO);*/

	//if(sd->status.hp<sd->status.max_hp>>2 && pc_checkskill(sd,SM_AUTOBERSERK)>0 &&
	if(sd->status.hp<sd->status.max_hp>>2 && sd->sc_data[SC_AUTOBERSERK].timer != -1 &&
		(sd->sc_data[SC_PROVOKE].timer==-1 || sd->sc_data[SC_PROVOKE].val2==0 ) && !pc_isdead(sd))
		// ƒI?ƒgƒo?ƒT?ƒN?“®
		status_change_start(&sd->bl,SC_PROVOKE,10,1,0,0,0,0);

	return 0;
}

/*==========================================
 * For quick calculating [Celest]
 *------------------------------------------
 */
int status_calc_speed (struct map_session_data *sd)
{
	int b_speed, skill;
	struct pc_base_job s_class;

	nullpo_retr(0, sd);

	s_class = pc_calc_base_job(sd->status.class_);

	b_speed = sd->speed;
	sd->speed = DEFAULT_WALK_SPEED ;

	if(sd->sc_count){
		if(sd->sc_data[SC_INCREASEAGI].timer!=-1 && sd->sc_data[SC_QUAGMIRE].timer == -1 && sd->sc_data[SC_DONTFORGETME].timer == -1){	// ‘¬“x?‰Á
			sd->speed -= sd->speed *25/100;
		}
		if(sd->sc_data[SC_DECREASEAGI].timer!=-1) {
			sd->speed = sd->speed *125/100;
		}
		if(sd->sc_data[SC_CLOAKING].timer!=-1) {
			sd->speed = sd->speed * (sd->sc_data[SC_CLOAKING].val3-sd->sc_data[SC_CLOAKING].val1*3) /100;
		}
		if(sd->sc_data[SC_CHASEWALK].timer!=-1) {
			sd->speed = sd->speed * sd->sc_data[SC_CHASEWALK].val3 /100;
		}
		if(sd->sc_data[SC_QUAGMIRE].timer!=-1){
			sd->speed = sd->speed*3/2;
		}
		if(sd->sc_data[SC_WINDWALK].timer!=-1 && sd->sc_data[SC_INCREASEAGI].timer==-1) {
			sd->speed -= sd->speed *(sd->sc_data[SC_WINDWALK].val1*2)/100;
		}
		if(sd->sc_data[SC_CARTBOOST].timer!=-1) {
			sd->speed -= (DEFAULT_WALK_SPEED * 20)/100;
		}
		if(sd->sc_data[SC_BERSERK].timer!=-1) {
			sd->speed -= sd->speed *25/100;
		}
		if(sd->sc_data[SC_WEDDING].timer!=-1) {
			sd->speed = 2*DEFAULT_WALK_SPEED;
		}
		if(sd->sc_data[SC_DONTFORGETME].timer!=-1){
			sd->speed= sd->speed*(100+sd->sc_data[SC_DONTFORGETME].val1*2 + sd->sc_data[SC_DONTFORGETME].val2 + (sd->sc_data[SC_DONTFORGETME].val3&0xffff))/100;
		}
		if(sd->sc_data[SC_STEELBODY].timer!=-1){
			sd->speed = (sd->speed * 125) / 100;
		}
		if(sd->sc_data[SC_DEFENDER].timer != -1) {
			sd->speed = (sd->speed * (155 - sd->sc_data[SC_DEFENDER].val1*5)) / 100;
		}
		if( sd->sc_data[SC_DANCING].timer!=-1 ){
			sd->speed = (int) ((double)sd->speed * (6.- 0.4 * pc_checkskill(sd, ((s_class.job == 19) ? BA_MUSICALLESSON : DC_DANCINGLESSON))));
		}
		if(sd->sc_data[SC_CURSE].timer!=-1)
			sd->speed += 450;
		if(sd->sc_data[SC_SLOWDOWN].timer!=-1)
			sd->speed = sd->speed*150/100;
		if(sd->sc_data[SC_SPEEDUP0].timer!=-1)
			sd->speed -= sd->speed*25/100;
	}

	if(sd->status.option&2 && (skill = pc_checkskill(sd,RG_TUNNELDRIVE))>0 )
		sd->speed += (100-16*skill)*DEFAULT_WALK_SPEED/100;
	if (pc_iscarton(sd) && (skill=pc_checkskill(sd,MC_PUSHCART))>0)
		sd->speed += (short) ((10-skill) * (DEFAULT_WALK_SPEED * 0.1));
	else if (pc_isriding(sd)) {
		sd->speed -= (short) ((0.25 * DEFAULT_WALK_SPEED));
	}
	if((skill=pc_checkskill(sd,TF_MISS))>0)
		if(s_class.job==12)
			sd->speed -= (short) (sd->speed *(skill*1.5)/100);

	if(sd->speed_rate != 100)
		sd->speed = sd->speed*sd->speed_rate/100;
	if(sd->speed < 1) sd->speed = 1;

	if(sd->skilltimer != -1 && (skill = pc_checkskill(sd,SA_FREECAST)) > 0) {
		sd->prev_speed = sd->speed;
		sd->speed = sd->speed*(175 - skill*5)/100;
	}

	if(b_speed != sd->speed)
		clif_updatestatus(sd,SP_SPEED);

	return 0;
}

/*==========================================
 * ‘ÎÛ‚ÌClass‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_class(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->class_;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.class_;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return ((struct pet_data *)bl)->class_;
	else
		return 0;
}
/*==========================================
 * ‘ÎÛ‚Ì•ûŒü‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_dir(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->dir;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->dir;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return ((struct pet_data *)bl)->dir;
	else
		return 0;
}
/*==========================================
 * ‘ÎÛ‚ÌƒŒƒxƒ‹‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_lv(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->level;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.base_level;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return ((struct pet_data *)bl)->msd->pet.level;
	else
		return 0;
}

/*==========================================
 * ‘ÎÛ‚ÌË’ö‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_range(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].range;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->attackrange;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].range;
	else
		return 0;
}
/*==========================================
 * ‘ÎÛ‚ÌHP‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_hp(struct block_list *bl)
{
	nullpo_retr(1, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->hp;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.hp;
	else
		return 1;
}
/*==========================================
 * ‘ÎÛ‚ÌMHP‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_max_hp(struct block_list *bl)
{
	nullpo_retr(1, bl);

	if(bl->type==BL_PC && ((struct map_session_data *)bl))
		return ((struct map_session_data *)bl)->status.max_hp;
	else {
		struct status_change *sc_data;		
		int max_hp = 1;

		if(bl->type == BL_MOB) {
			struct mob_data *md;
			nullpo_retr(1, md = (struct mob_data *)bl);
			max_hp = mob_db[md->class_].max_hp;

			if(battle_config.mobs_level_up) // mobs leveling up increase [Valaris]
				max_hp += (md->level - mob_db[md->class_].lv) * status_get_vit(bl);

			if(mob_db[md->class_].mexp > 0) { //MVP Monsters
				if(battle_config.mvp_hp_rate != 100) {
					double hp = (double)max_hp * battle_config.mvp_hp_rate / 100.0;
					max_hp = (hp > 0x7FFFFFFF ? 0x7FFFFFFF : (int)hp);
				}
			}
			else {	//Common MONSTERS
				if(battle_config.monster_hp_rate != 100) {
					double hp = (double)max_hp * battle_config.monster_hp_rate / 100.0;
					max_hp = (hp > 0x7FFFFFFF ? 0x7FFFFFFF : (int)hp);
				}
			}
		}
		else if(bl->type == BL_PET) {
			struct pet_data *pd;
			nullpo_retr(1, pd = (struct pet_data*)bl);
			max_hp = mob_db[pd->class_].max_hp;

			if(mob_db[pd->class_].mexp > 0) { //MVP Monsters 
				if(battle_config.mvp_hp_rate != 100)
					max_hp = (max_hp * battle_config.mvp_hp_rate)/100;
			}
			else {	//Common MONSTERS
				if(battle_config.monster_hp_rate != 100)
					max_hp = (max_hp * battle_config.monster_hp_rate)/100;
			}
		}

		sc_data = status_get_sc_data(bl);
		if(sc_data) {
			if(sc_data[SC_APPLEIDUN].timer != -1)
				max_hp += ((5 + sc_data[SC_APPLEIDUN].val1 * 2 + ((sc_data[SC_APPLEIDUN].val2 + 1) >> 1)
					+ sc_data[SC_APPLEIDUN].val3 / 10) * max_hp)/100;
			if(sc_data[SC_GOSPEL].timer != -1 &&
				sc_data[SC_GOSPEL].val4 == BCT_PARTY &&
				sc_data[SC_GOSPEL].val3 == 4)
				max_hp += max_hp * 25 / 100;
		}
		if(max_hp < 1) max_hp = 1;
		return max_hp;
	}
	return 1;
}
/*==========================================
 * ‘ÎÛ‚ÌStr‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_str(struct block_list *bl)
{
	int str = 0;
	nullpo_retr(0, bl);

	if (bl->type == BL_PC && ((struct map_session_data *)bl))
		return ((struct map_session_data *)bl)->paramc[0];
	else {		
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);

		if(bl->type == BL_MOB && ((struct mob_data *)bl)) {
			str = mob_db[((struct mob_data *)bl)->class_].str;
			if(battle_config.mobs_level_up) // mobs leveling up increase [Valaris]
				str += ((struct mob_data *)bl)->level - mob_db[((struct mob_data *)bl)->class_].lv;
		}
		else if(bl->type == BL_PET && ((struct pet_data *)bl))
			str = mob_db[((struct pet_data *)bl)->class_].str;

		if(sc_data) {
			if(sc_data[SC_LOUD].timer != -1 && sc_data[SC_QUAGMIRE].timer == -1)
				str += 4;
			if( sc_data[SC_BLESSING].timer != -1){	// ƒuƒŒƒbƒVƒ“ƒO
				int race = status_get_race(bl);
				if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6)
					str >>= 1;	// ˆ« –‚/•s€
				else str += sc_data[SC_BLESSING].val1;	// ‚»‚Ì‘¼
			}
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				str += 5;
		}
	}
	if(str < 0) str = 0;
	return str;
}
/*==========================================
 * ‘ÎÛ‚ÌAgi‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */

int status_get_agi(struct block_list *bl)
{
	int agi=0;
	nullpo_retr(0, bl);

	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->paramc[1];
	else {
		struct status_change *sc_data;	
		sc_data = status_get_sc_data(bl);
		if(bl->type == BL_MOB && (struct mob_data *)bl) {
			agi = mob_db[((struct mob_data *)bl)->class_].agi;
			if(battle_config.mobs_level_up) // increase of mobs leveling up [Valaris]
				agi += ((struct mob_data *)bl)->level - mob_db[((struct mob_data *)bl)->class_].lv;
		}
		else if(bl->type == BL_PET && (struct pet_data *)bl)
			agi = mob_db[((struct pet_data *)bl)->class_].agi;

		if(sc_data) {
			if(sc_data[SC_INCREASEAGI].timer!=-1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// ‘¬“x‘‰Á(PC‚Ípc.c‚Å)
				agi += 2 + sc_data[SC_INCREASEAGI].val1;
			if(sc_data[SC_CONCENTRATE].timer!=-1 && sc_data[SC_QUAGMIRE].timer == -1)
				agi += agi * (2 + sc_data[SC_CONCENTRATE].val1)/100;
			if(sc_data[SC_DECREASEAGI].timer!=-1)	// ‘¬“xŒ¸­
				agi -= 2 + sc_data[SC_DECREASEAGI].val1;
			if(sc_data[SC_QUAGMIRE].timer!=-1 ) {	// ƒNƒ@ƒOƒ}ƒCƒA
				//agi >>= 1;
				//int agib = agi*(sc_data[SC_QUAGMIRE].val1*10)/100;
				//agi -= agib > 50 ? 50 : agib;
				agi -= sc_data[SC_QUAGMIRE].val1*10;
			}
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				agi += 5;
		}
	}
	if(agi < 0) agi = 0;
	return agi;
}
/*==========================================
 * ‘ÎÛ‚ÌVit‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_vit(struct block_list *bl)
{
	int vit = 0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->paramc[2];
	else {
		struct status_change *sc_data;	
		sc_data = status_get_sc_data(bl);
		if(bl->type == BL_MOB && (struct mob_data *)bl) {
			vit = mob_db[((struct mob_data *)bl)->class_].vit;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				vit += ((struct mob_data *)bl)->level - mob_db[((struct mob_data *)bl)->class_].lv;
		}	
		else if(bl->type == BL_PET && (struct pet_data *)bl)
			vit = mob_db[((struct pet_data *)bl)->class_].vit;
		if(sc_data) {
			if(sc_data[SC_STRIPARMOR].timer != -1)
				vit = vit*60/100;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				vit += 5;
		}
	}
	if(vit < 0) vit = 0;
	return vit;
}
/*==========================================
 * ‘ÎÛ‚ÌInt‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_int(struct block_list *bl)
{
	int int_=0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->paramc[3];
	else {
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);
		if(bl->type == BL_MOB && (struct mob_data *)bl){
			int_ = mob_db[((struct mob_data *)bl)->class_].int_;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				int_ += ((struct mob_data *)bl)->level - mob_db[((struct mob_data *)bl)->class_].lv;
		}		
		else if(bl->type == BL_PET && (struct pet_data *)bl)
			int_ = mob_db[((struct pet_data *)bl)->class_].int_;

		if(sc_data) {
			if(sc_data[SC_BLESSING].timer != -1){	// ƒuƒŒƒbƒVƒ“ƒO
				int race = status_get_race(bl);
				if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6 )
					int_ >>= 1;	// ˆ« –‚/•s€
				else
					int_ += sc_data[SC_BLESSING].val1;	// ‚»‚Ì‘¼
			}
			if(sc_data[SC_STRIPHELM].timer != -1)
				int_ = int_*60/100;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				int_ += 5;
		}
	}
	if(int_ < 0) int_ = 0;
	return int_;
}
/*==========================================
 * ‘ÎÛ‚ÌDex‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_dex(struct block_list *bl)
{
	int dex = 0;
	nullpo_retr(0, bl);

	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->paramc[4];
	else {
		struct status_change *sc_data;	
		sc_data = status_get_sc_data(bl);
		if(bl->type == BL_MOB && (struct mob_data *)bl) {
			dex = mob_db[((struct mob_data *)bl)->class_].dex;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				dex += ((struct mob_data *)bl)->level - mob_db[((struct mob_data *)bl)->class_].lv;
		}		
		else if(bl->type == BL_PET && (struct pet_data *)bl)
			dex = mob_db[((struct pet_data *)bl)->class_].dex;

		if(sc_data) {
			if(sc_data[SC_CONCENTRATE].timer!=-1 && sc_data[SC_QUAGMIRE].timer == -1)
				dex += dex*(2+sc_data[SC_CONCENTRATE].val1)/100;
			if(sc_data[SC_BLESSING].timer != -1){	// ƒuƒŒƒbƒVƒ“ƒO
				int race = status_get_race(bl);
				if(battle_check_undead(race,status_get_elem_type(bl)) || race == 6 )
					dex >>= 1;	// ˆ« –‚/•s€
				else dex += sc_data[SC_BLESSING].val1;	// ‚»‚Ì‘¼
			}
			if(sc_data[SC_QUAGMIRE].timer!=-1)	{ // ƒNƒ@ƒOƒ}ƒCƒA
				// dex >>= 1;
				//int dexb = dex*(sc_data[SC_QUAGMIRE].val1*10)/100;
				//dex -= dexb > 50 ? 50 : dexb;
				dex -= sc_data[SC_QUAGMIRE].val1*10;
			}
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				dex += 5;
		}
	}
	if(dex < 0) dex = 0;
	return dex;
}
/*==========================================
 * ‘ÎÛ‚ÌLuk‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_luk(struct block_list *bl)
{
	int luk = 0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->paramc[5];
	else {
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);
		if(bl->type == BL_MOB && (struct mob_data *)bl) {
			luk = mob_db[((struct mob_data *)bl)->class_].luk;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				luk += ((struct mob_data *)bl)->level - mob_db[((struct mob_data *)bl)->class_].lv;
		}		
		else if(bl->type == BL_PET && (struct pet_data *)bl)
			luk = mob_db[((struct pet_data *)bl)->class_].luk;
		if(sc_data) {
			if(sc_data[SC_GLORIA].timer!=-1)	// ƒOƒƒŠƒA(PC‚Ípc.c‚Å)
				luk += 30;
			if(sc_data[SC_TRUESIGHT].timer!=-1)	// ƒgƒDƒ‹[ƒTƒCƒg
				luk += 5;
			if(sc_data[SC_CURSE].timer!=-1)		// ô‚¢
				luk = 0;
		}
	}
	if(luk < 0) luk = 0;
	return luk;
}

/*==========================================
 * ‘ÎÛ‚ÌFlee‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_flee(struct block_list *bl)
{
	int flee = 1;
	nullpo_retr(1, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->flee;
	else {
		struct status_change *sc_data;	
		sc_data = status_get_sc_data(bl);
		flee = status_get_agi(bl) + status_get_lv(bl);

		if(sc_data){
			if(sc_data[SC_WHISTLE].timer!=-1)
				flee += flee*(sc_data[SC_WHISTLE].val1+sc_data[SC_WHISTLE].val2
					+(sc_data[SC_WHISTLE].val3>>16))/100;
			if(sc_data[SC_BLIND].timer!=-1)
				flee -= flee*25/100;
			if(sc_data[SC_WINDWALK].timer!=-1) // ƒEƒBƒ“ƒhƒEƒH[ƒN
				flee += flee*(sc_data[SC_WINDWALK].val2)/100;
			if(sc_data[SC_SPIDERWEB].timer!=-1) //ƒXƒpƒCƒ_[ƒEƒFƒu
				flee -= flee*50/100;
			if(sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4 == BCT_PARTY &&
					sc_data[SC_GOSPEL].val3 == 13)
					flee += flee*5/100;
				else if (sc_data[SC_GOSPEL].val4 == BCT_ENEMY &&
					sc_data[SC_GOSPEL].val3 == 7)
					flee = 0;
			}
		}
	}
	if(flee < 1) flee = 1;
	return flee;
}
/*==========================================
 * ‘ÎÛ‚ÌHit‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_hit(struct block_list *bl)
{
	int hit = 1;
	nullpo_retr(1, bl);
	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->hit;
	else {
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);		
		hit = status_get_dex(bl) + status_get_lv(bl);

		if(sc_data) {
			if(sc_data[SC_HUMMING].timer!=-1)	//
				hit += hit*(sc_data[SC_HUMMING].val1*2+sc_data[SC_HUMMING].val2
					+sc_data[SC_HUMMING].val3)/100;
			if(sc_data[SC_BLIND].timer!=-1)		// ô‚¢
				hit -= hit*25/100;
			if(sc_data[SC_TRUESIGHT].timer!=-1)		// ƒgƒDƒ‹[ƒTƒCƒg
				hit += 3*(sc_data[SC_TRUESIGHT].val1);
			if(sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				hit += (hit*(10*(sc_data[SC_CONCENTRATION].val1)))/100;
			if(sc_data[SC_GOSPEL].timer!=-1 &&
				sc_data[SC_GOSPEL].val4 == BCT_PARTY &&
				sc_data[SC_GOSPEL].val3 == 14)
				hit += hit*5/100;
			if(sc_data[SC_EXPLOSIONSPIRITS].timer!=-1)
				hit += 20*sc_data[SC_EXPLOSIONSPIRITS].val1;
		}
	}
	if(hit < 1) hit = 1;
	return hit;
}
/*==========================================
 * ‘ÎÛ‚ÌŠ®‘S‰ñ”ğ‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_flee2(struct block_list *bl)
{
	int flee2 = 1;
	nullpo_retr(1, bl);

	if(bl->type==BL_PC && (struct map_session_data *)bl){
		return ((struct map_session_data *)bl)->flee2;
	} else {
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);
		flee2 = status_get_luk(bl)+1;

		if(sc_data) {
			if(sc_data[SC_WHISTLE].timer!=-1)
				flee2 += (sc_data[SC_WHISTLE].val1+sc_data[SC_WHISTLE].val2
					+(sc_data[SC_WHISTLE].val3&0xffff))*10;
		}
	}
	if(flee2 < 1) flee2 = 1;
	return flee2;
}
/*==========================================
 * ‘ÎÛ‚ÌƒNƒŠƒeƒBƒJƒ‹‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_critical(struct block_list *bl)
{
	int critical = 1;
	nullpo_retr(1, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl){
		return ((struct map_session_data *)bl)->critical;
	} else {
		struct status_change *sc_data;
		sc_data = status_get_sc_data(bl);		
		critical = status_get_luk(bl)*3 + 1;

		if(sc_data) {
			if(sc_data[SC_FORTUNE].timer!=-1)
				critical += (10+sc_data[SC_FORTUNE].val1+sc_data[SC_FORTUNE].val2
					+ sc_data[SC_FORTUNE].val3)*10;
			if(sc_data[SC_EXPLOSIONSPIRITS].timer!=-1)
				critical += sc_data[SC_EXPLOSIONSPIRITS].val2;
			if(sc_data[SC_TRUESIGHT].timer!=-1) //ƒgƒDƒ‹[ƒTƒCƒg
				critical += critical*sc_data[SC_TRUESIGHT].val1/100;
		}
	}
	if(critical < 1) critical = 1;
	return critical;
}
/*==========================================
 * base_atk‚Ìæ“¾
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_baseatk(struct block_list *bl)
{
	int batk = 1;
	nullpo_retr(1, bl);
	
	if(bl->type==BL_PC && (struct map_session_data *)bl) {
		batk = ((struct map_session_data *)bl)->base_atk; //İ’è‚³‚ê‚Ä‚¢‚ébase_atk
		if (((struct map_session_data *)bl)->status.weapon < 16)
			batk += ((struct map_session_data *)bl)->weapon_atk[((struct map_session_data *)bl)->status.weapon];
	} else { //‚»‚êˆÈŠO‚È‚ç
		struct status_change *sc_data;
		int str,dstr;
		str = status_get_str(bl); //STR
		dstr = str/10;
		batk = dstr*dstr + str; //base_atk‚ğŒvZ‚·‚é
		sc_data = status_get_sc_data(bl);

		if(sc_data) { //ó‘ÔˆÙí‚ ‚è
			if(sc_data[SC_PROVOKE].timer!=-1) //PC‚Åƒvƒƒ{ƒbƒN(SM_PROVOKE)ó‘Ô
				batk = batk*(100+2*sc_data[SC_PROVOKE].val1)/100; //base_atk‘‰Á
			if(sc_data[SC_CURSE].timer!=-1) //ô‚í‚ê‚Ä‚¢‚½‚ç
				batk -= batk*25/100; //base_atk‚ª25%Œ¸­
			if(sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				batk += batk*(5*sc_data[SC_CONCENTRATION].val1)/100;
		}
	}
	if(batk < 1) batk = 1; //base_atk‚ÍÅ’á‚Å‚à1
	return batk;
}
/*==========================================
 * ‘ÎÛ‚ÌAtk‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk(struct block_list *bl)
{
	int atk = 0;
	nullpo_retr(0, bl);

	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data*)bl)->watk;
	else {
		struct status_change *sc_data;
		sc_data=status_get_sc_data(bl);
		
		if(bl->type == BL_MOB && (struct mob_data *)bl)
			atk = mob_db[((struct mob_data*)bl)->class_].atk1;
		else if(bl->type == BL_PET && (struct pet_data *)bl)
			atk = mob_db[((struct pet_data*)bl)->class_].atk1;

		if(sc_data) {
			if(sc_data[SC_PROVOKE].timer!=-1)
				atk = atk*(100+2*sc_data[SC_PROVOKE].val1)/100;
			if(sc_data[SC_CURSE].timer!=-1)
				atk -= atk*25/100;
			if(sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				atk += atk*(5*sc_data[SC_CONCENTRATION].val1)/100;
			if(sc_data[SC_EXPLOSIONSPIRITS].timer!=-1)
				atk += (1000*sc_data[SC_EXPLOSIONSPIRITS].val1);
			if(sc_data[SC_STRIPWEAPON].timer!=-1)
				atk -= atk*10/100;

			if(sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4 == BCT_PARTY &&
					sc_data[SC_GOSPEL].val3 == 12)
					atk += atk*8/100;
				else if (sc_data[SC_GOSPEL].val4 == BCT_ENEMY &&
					sc_data[SC_GOSPEL].val3 == 6)
					atk = 0;
			}
		}
	}
	if(atk < 0) atk = 0;
	return atk;
}
/*==========================================
 * ‘ÎÛ‚Ì¶èAtk‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk_(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl){
		int atk=((struct map_session_data*)bl)->watk_;
		return atk;
	}
	else
		return 0;
}
/*==========================================
 * ‘ÎÛ‚ÌAtk2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data*)bl)->watk2;
	else {
		struct status_change *sc_data=status_get_sc_data(bl);
		int atk2=0;
		if(bl->type==BL_MOB && (struct mob_data *)bl)
			atk2 = mob_db[((struct mob_data*)bl)->class_].atk2;
		else if(bl->type==BL_PET && (struct pet_data *)bl)
			atk2 = mob_db[((struct pet_data*)bl)->class_].atk2;
		if(sc_data) {
			if( sc_data[SC_IMPOSITIO].timer!=-1)
				atk2 += sc_data[SC_IMPOSITIO].val1*5;
			if( sc_data[SC_PROVOKE].timer!=-1 )
				atk2 = atk2*(100+2*sc_data[SC_PROVOKE].val1)/100;
			if( sc_data[SC_CURSE].timer!=-1 )
				atk2 -= atk2*25/100;
			if(sc_data[SC_DRUMBATTLE].timer!=-1)
				atk2 += sc_data[SC_DRUMBATTLE].val2;
			if(sc_data[SC_NIBELUNGEN].timer!=-1 && (status_get_element(bl)/10) >= 8 )
				atk2 += sc_data[SC_NIBELUNGEN].val3;
			if(sc_data[SC_STRIPWEAPON].timer!=-1)
				atk2 = atk2*sc_data[SC_STRIPWEAPON].val2/100;
			if(sc_data[SC_CONCENTRATION].timer!=-1) //ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“
				atk2 += atk2*(5*sc_data[SC_CONCENTRATION].val1)/100;
			if(sc_data[SC_EXPLOSIONSPIRITS].timer!=-1)
				atk2 += (1000*sc_data[SC_EXPLOSIONSPIRITS].val1);
		}
		if(atk2 < 0) atk2 = 0;
		return atk2;
	}
	return 0;
}
/*==========================================
 * ‘ÎÛ‚Ì¶èAtk2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_atk_2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data*)bl)->watk_2;
	else
		return 0;
}
/*==========================================
 * ‘ÎÛ‚ÌMAtk1‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_matk1(struct block_list *bl)
{
	int matk = 0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->matk1;
	else {
		struct status_change *sc_data;
		int int_ = status_get_int(bl);
		matk = int_+(int_/5)*(int_/5);

		sc_data = status_get_sc_data(bl);
		if(sc_data) {
			if(sc_data[SC_MINDBREAKER].timer!=-1)
				matk = matk*(100+2*sc_data[SC_MINDBREAKER].val1)/100;
		}
	}
	return matk;
}
/*==========================================
 * ‘ÎÛ‚ÌMAtk2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_matk2(struct block_list *bl)
{
	int matk = 0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->matk2;
	else {
		struct status_change *sc_data = status_get_sc_data(bl);
		int int_ = status_get_int(bl);
		matk = int_+(int_/7)*(int_/7);

		if(sc_data) {
			if(sc_data[SC_MINDBREAKER].timer!=-1)
				matk = matk*(100+2*sc_data[SC_MINDBREAKER].val1)/100;
		}
	}
	return matk;
}
/*==========================================
 * ‘ÎÛ‚ÌDef‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_def(struct block_list *bl)
{
	struct status_change *sc_data;
	int def=0,skilltimer=-1,skillid=0;

	nullpo_retr(0, bl);
	sc_data=status_get_sc_data(bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl){
		def = ((struct map_session_data *)bl)->def;
		skilltimer = ((struct map_session_data *)bl)->skilltimer;
		skillid = ((struct map_session_data *)bl)->skillid;
	}
	else if(bl->type==BL_MOB && (struct mob_data *)bl) {
		def = mob_db[((struct mob_data *)bl)->class_].def;
		skilltimer = ((struct mob_data *)bl)->skilltimer;
		skillid = ((struct mob_data *)bl)->skillid;
	}
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		def = mob_db[((struct pet_data *)bl)->class_].def;

	if(def < 1000000) {
		if(sc_data) {
			//“€Œ‹AÎ‰»‚Í‰EƒVƒtƒg
			if(sc_data[SC_FREEZE].timer != -1 || (sc_data[SC_STONE].timer != -1 && sc_data[SC_STONE].val2 == 0))
				def >>= 1;

			if (bl->type != BL_PC) {
				//ƒL[ƒsƒ“ƒO‚ÍDEF100
				if( sc_data[SC_KEEPING].timer!=-1)
					def = 100;
				//ƒvƒƒ{ƒbƒN‚ÍŒ¸Z
				if( sc_data[SC_PROVOKE].timer!=-1)
					def = (def*(100 - 6*sc_data[SC_PROVOKE].val1)+50)/100;
				//í‘¾ŒÛ‚Ì‹¿‚«‚Í‰ÁZ
				if( sc_data[SC_DRUMBATTLE].timer!=-1)
					def += sc_data[SC_DRUMBATTLE].val3;
				//“Å‚É‚©‚©‚Á‚Ä‚¢‚é‚ÍŒ¸Z
				if(sc_data[SC_POISON].timer!=-1)
					def = def*75/100;
				//ƒXƒgƒŠƒbƒvƒV[ƒ‹ƒh‚ÍŒ¸Z
				if(sc_data[SC_STRIPSHIELD].timer!=-1)
					def = def*sc_data[SC_STRIPSHIELD].val2/100;
				//ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX‚ÍŒ¸Z
				if(sc_data[SC_SIGNUMCRUCIS].timer!=-1)
					def = def * (100 - sc_data[SC_SIGNUMCRUCIS].val2)/100;
				//‰i‰“‚Ì¬“×‚ÍDEF0‚É‚È‚é
				if(sc_data[SC_ETERNALCHAOS].timer!=-1)
					def = 0;
				//ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“‚ÍŒ¸Z
				if( sc_data[SC_CONCENTRATION].timer!=-1)
					def = (def*(100 - 5*sc_data[SC_CONCENTRATION].val1))/100;

				if(sc_data[SC_GOSPEL].timer!=-1) {
					if (sc_data[SC_GOSPEL].val4 == BCT_PARTY &&
						sc_data[SC_GOSPEL].val3 == 11)
						def += def*25/100;
					else if (sc_data[SC_GOSPEL].val4 == BCT_ENEMY &&
						sc_data[SC_GOSPEL].val3 == 5)
						def = 0;
				}
				if(sc_data[SC_JOINTBEAT].timer!=-1) {
					if (sc_data[SC_JOINTBEAT].val2 == 4)
						def -= def*50/100;
					else if (sc_data[SC_JOINTBEAT].val2 == 5)
						def -= def*25/100;
				}
			}
		}
		//‰r¥’†‚Í‰r¥Œ¸Z—¦‚ÉŠî‚Ã‚¢‚ÄŒ¸Z
		if(skilltimer != -1) {
			int def_rate = skill_get_castdef(skillid);
			if(def_rate != 0)
				def = (def * (100 - def_rate))/100;
		}
	}
	if(def < 0) def = 0;
	return def;
}
/*==========================================
 * ‘ÎÛ‚ÌMDef‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_mdef(struct block_list *bl)
{
	struct status_change *sc_data;
	int mdef=0;

	nullpo_retr(0, bl);
	sc_data=status_get_sc_data(bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		mdef = ((struct map_session_data *)bl)->mdef;
	else if(bl->type==BL_MOB && (struct mob_data *)bl)
		mdef = mob_db[((struct mob_data *)bl)->class_].mdef;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		mdef = mob_db[((struct pet_data *)bl)->class_].mdef;

	if(mdef < 1000000) {
		if(sc_data) {
			//ƒoƒŠƒA[ó‘Ô‚ÍMDEF100
			if(sc_data[SC_BARRIER].timer != -1)
				mdef = 100;
			//“€Œ‹AÎ‰»‚Í1.25”{
			if(sc_data[SC_FREEZE].timer != -1 || (sc_data[SC_STONE].timer != -1 && sc_data[SC_STONE].val2 == 0))
				mdef = mdef*125/100;
			if( sc_data[SC_MINDBREAKER].timer!=-1 && bl->type != BL_PC)
				mdef -= (mdef*6*sc_data[SC_MINDBREAKER].val1)/100;
		}
	}
	if(mdef < 0) mdef = 0;
	return mdef;
}
/*==========================================
 * ‘ÎÛ‚ÌDef2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 *------------------------------------------
 */
int status_get_def2(struct block_list *bl)
{
	int def2 = 1;
	nullpo_retr(1, bl);
	
	if(bl->type==BL_PC)
		return ((struct map_session_data *)bl)->def2;
	else {
		struct status_change *sc_data;

		if(bl->type==BL_MOB)
			def2 = mob_db[((struct mob_data *)bl)->class_].vit;
		else if(bl->type==BL_PET)
			def2 = mob_db[((struct pet_data *)bl)->class_].vit;

		sc_data = status_get_sc_data(bl);
		if(sc_data) {
			if(sc_data[SC_ANGELUS].timer != -1)
				def2 = def2*(110+5*sc_data[SC_ANGELUS].val1)/100;
			if(sc_data[SC_PROVOKE].timer!=-1)
				def2 = (def2*(100 - 6*sc_data[SC_PROVOKE].val1)+50)/100;
			if(sc_data[SC_POISON].timer!=-1)
				def2 = def2*75/100;
			//ƒRƒ“ƒZƒ“ƒgƒŒ[ƒVƒ‡ƒ“‚ÍŒ¸Z
			if( sc_data[SC_CONCENTRATION].timer!=-1)
				def2 = def2*(100 - 5*sc_data[SC_CONCENTRATION].val1)/100;

			if(sc_data[SC_GOSPEL].timer!=-1) {
				if (sc_data[SC_GOSPEL].val4 == BCT_PARTY &&
					sc_data[SC_GOSPEL].val3 == 11)
					def2 += def2*25/100;
				else if (sc_data[SC_GOSPEL].val4 == BCT_ENEMY &&
					sc_data[SC_GOSPEL].val3 == 5)
					def2 = 0;
			}
		}
	}
	if(def2 < 1) def2 = 1;
	return def2;
}
/*==========================================
 * ‘ÎÛ‚ÌMDef2‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å0ˆÈã
 *------------------------------------------
 */
int status_get_mdef2(struct block_list *bl)
{
	int mdef2 = 0;
	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		return ((struct map_session_data *)bl)->mdef2 + (((struct map_session_data *)bl)->paramc[2]>>1);
	else {
		struct status_change *sc_data = status_get_sc_data(bl);
		if(bl->type == BL_MOB)
			mdef2 = mob_db[((struct mob_data *)bl)->class_].int_ + (mob_db[((struct mob_data *)bl)->class_].vit>>1);
		else if(bl->type == BL_PET)
			mdef2 = mob_db[((struct pet_data *)bl)->class_].int_ + (mob_db[((struct pet_data *)bl)->class_].vit>>1);
		if(sc_data) {
			if(sc_data[SC_MINDBREAKER].timer!=-1)
				mdef2 -= (mdef2*6*sc_data[SC_MINDBREAKER].val1)/100;
		}
	}
	if(mdef2 < 0) mdef2 = 0;
		return mdef2;
}
/*==========================================
 * ‘ÎÛ‚ÌSpeed(ˆÚ“®‘¬“x)‚ğ•Ô‚·(”Ä—p)
 * –ß‚è‚Í®”‚Å1ˆÈã
 * Speed‚Í¬‚³‚¢‚Ù‚¤‚ªˆÚ“®‘¬“x‚ª‘¬‚¢
 *------------------------------------------
 */
int status_get_speed(struct block_list *bl)
{
	nullpo_retr(1000, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->speed;
	else {
		struct status_change *sc_data=status_get_sc_data(bl);
		int speed = 1000;
		if(bl->type==BL_MOB && (struct mob_data *)bl) {
			speed = ((struct mob_data *)bl)->speed;
			if(battle_config.mobs_level_up) // increase from mobs leveling up [Valaris]
				speed-=((struct mob_data *)bl)->level - mob_db[((struct mob_data *)bl)->class_].lv;
		}
		else if(bl->type==BL_PET && (struct pet_data *)bl)
			speed = ((struct pet_data *)bl)->msd->petDB->speed;

		if(sc_data) {
			//‘¬“x‘‰Á‚Í25%Œ¸Z
			if(sc_data[SC_INCREASEAGI].timer!=-1)
				speed -= speed*25/100;
			//ƒEƒBƒ“ƒhƒEƒH[ƒN‚ÍLv*2%Œ¸Z
			else if(sc_data[SC_WINDWALK].timer!=-1)
				speed -= (speed*(sc_data[SC_WINDWALK].val1*2))/100;
			//‘¬“xŒ¸­‚Í25%‰ÁZ
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				speed = speed*125/100;
			//ƒNƒ@ƒOƒ}ƒCƒA‚Í50%‰ÁZ
			if(sc_data[SC_QUAGMIRE].timer!=-1)
				speed = speed*3/2;
			//„‚ğ–Y‚ê‚È‚¢‚Åc‚Í‰ÁZ
			if(sc_data[SC_DONTFORGETME].timer!=-1)
				speed = speed*(100+sc_data[SC_DONTFORGETME].val1*2 + sc_data[SC_DONTFORGETME].val2 + (sc_data[SC_DONTFORGETME].val3&0xffff))/100;
			//‹à„‚Í25%‰ÁZ
			if(sc_data[SC_STEELBODY].timer!=-1)
				speed = speed*125/100;
			//ƒfƒBƒtƒFƒ“ƒ_[‚Í‰ÁZ
			if(sc_data[SC_DEFENDER].timer!=-1)
				speed = (speed * (155 - sc_data[SC_DEFENDER].val1*5)) / 100;
			//—x‚èó‘Ô‚Í4”{’x‚¢
			if(sc_data[SC_DANCING].timer!=-1 )
				speed *= 6;
			//ô‚¢‚Í450‰ÁZ
			if(sc_data[SC_CURSE].timer!=-1)
				speed = speed + 450;
			if(sc_data[SC_SLOWDOWN].timer!=-1)
				speed = speed*150/100;
			if(sc_data[SC_SPEEDUP0].timer!=-1)
				speed -= speed*25/100;
			if(sc_data[SC_GOSPEL].timer!=-1 &&
				sc_data[SC_GOSPEL].val4 == BCT_ENEMY &&
				sc_data[SC_GOSPEL].val3 == 8)
				speed = speed*125/100;
			if(sc_data[SC_JOINTBEAT].timer!=-1) {
				if (sc_data[SC_JOINTBEAT].val2 == 1)
					speed = speed*150/100;
				else if (sc_data[SC_JOINTBEAT].val2 == 3)
					speed = speed*130/100;				
			}
		}
		if(speed < 1) speed = 1;
		return speed;
	}

	return 1000;
}
/*==========================================
 * ‘ÎÛ‚ÌaDelay(UŒ‚ƒfƒBƒŒƒC)‚ğ•Ô‚·(”Ä—p)
 * aDelay‚Í¬‚³‚¢‚Ù‚¤‚ªUŒ‚‘¬“x‚ª‘¬‚¢
 *------------------------------------------
 */
int status_get_adelay(struct block_list *bl)
{
	nullpo_retr(4000, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return (((struct map_session_data *)bl)->aspd<<1);
	else {
		struct status_change *sc_data=status_get_sc_data(bl);
		int adelay=4000,aspd_rate = 100,i;
		if(bl->type==BL_MOB && (struct mob_data *)bl)
			adelay = mob_db[((struct mob_data *)bl)->class_].adelay;
		else if(bl->type==BL_PET && (struct pet_data *)bl)
			adelay = mob_db[((struct pet_data *)bl)->class_].adelay;

		if(sc_data) {
			//ƒc[ƒnƒ“ƒhƒNƒCƒbƒPƒ“g—p‚ÅƒNƒ@ƒOƒ}ƒCƒA‚Å‚à„‚ğ–Y‚ê‚È‚¢‚Åc‚Å‚à‚È‚¢‚Í3Š„Œ¸Z
			if(sc_data[SC_TWOHANDQUICKEN].timer != -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// 2HQ
				aspd_rate -= 30;
			//ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…g—p‚Åƒc[ƒnƒ“ƒhƒNƒCƒbƒPƒ“‚Å‚àƒNƒ@ƒOƒ}ƒCƒA‚Å‚à„‚ğ–Y‚ê‚È‚¢‚Åc‚Å‚à‚È‚¢‚Í
			if(sc_data[SC_ADRENALINE].timer != -1 && sc_data[SC_TWOHANDQUICKEN].timer == -1 &&
				sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1) {	// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
				//g—pÒ‚Æƒp[ƒeƒBƒƒ“ƒo[‚ÅŠi·‚ªo‚éİ’è‚Å‚È‚¯‚ê‚Î3Š„Œ¸Z
				if(sc_data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)
					aspd_rate -= 30;
				//‚»‚¤‚Å‚È‚¯‚ê‚Î2.5Š„Œ¸Z
				else
					aspd_rate -= 25;
			}
			//ƒXƒsƒAƒNƒBƒbƒPƒ“‚ÍŒ¸Z
			if(sc_data[SC_SPEARSQUICKEN].timer != -1 && sc_data[SC_ADRENALINE].timer == -1 &&
				sc_data[SC_TWOHANDQUICKEN].timer == -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// ƒXƒsƒAƒNƒBƒbƒPƒ“
				aspd_rate -= sc_data[SC_SPEARSQUICKEN].val2;
			//—[“ú‚ÌƒAƒTƒVƒ“ƒNƒƒX‚ÍŒ¸Z
			if(sc_data[SC_ASSNCROS].timer!=-1 && // —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
				sc_data[SC_TWOHANDQUICKEN].timer==-1 && sc_data[SC_ADRENALINE].timer==-1 && sc_data[SC_SPEARSQUICKEN].timer==-1 &&
				sc_data[SC_DONTFORGETME].timer == -1)
				aspd_rate -= 5+sc_data[SC_ASSNCROS].val1+sc_data[SC_ASSNCROS].val2+sc_data[SC_ASSNCROS].val3;
			//„‚ğ–Y‚ê‚È‚¢‚Åc‚Í‰ÁZ
			if(sc_data[SC_DONTFORGETME].timer!=-1)		// „‚ğ–Y‚ê‚È‚¢‚Å
				aspd_rate += sc_data[SC_DONTFORGETME].val1*3 + sc_data[SC_DONTFORGETME].val2 + (sc_data[SC_DONTFORGETME].val3>>16);
			//‹à„25%‰ÁZ
			if(sc_data[SC_STEELBODY].timer!=-1)	// ‹à„
				aspd_rate += 25;
			//‘‘¬ƒ|[ƒVƒ‡ƒ“g—p‚ÍŒ¸Z
			if(	sc_data[i=SC_SPEEDPOTION3].timer!=-1 || sc_data[i=SC_SPEEDPOTION2].timer!=-1 || sc_data[i=SC_SPEEDPOTION1].timer!=-1 || sc_data[i=SC_SPEEDPOTION0].timer!=-1)
				aspd_rate -= sc_data[i].val2;
			//ƒfƒBƒtƒFƒ“ƒ_[‚Í‰ÁZ
			if(sc_data[SC_DEFENDER].timer != -1)
				aspd_rate += (25 - sc_data[SC_DEFENDER].val1*5);
				//adelay += (1100 - sc_data[SC_DEFENDER].val1*100);
			if(sc_data[SC_GOSPEL].timer!=-1 &&
				sc_data[SC_GOSPEL].val4 == BCT_ENEMY &&
				sc_data[SC_GOSPEL].val3 == 8)
				aspd_rate = aspd_rate*125/100;
			if(sc_data[SC_JOINTBEAT].timer!=-1) {
				if (sc_data[SC_JOINTBEAT].val2 == 2)
					aspd_rate = aspd_rate*125/100;
				else if (sc_data[SC_JOINTBEAT].val2 == 3)
					aspd_rate = aspd_rate*110/100;
			}
		}
		if(aspd_rate != 100)
			adelay = adelay*aspd_rate/100;
		if(adelay < battle_config.monster_max_aspd<<1) adelay = battle_config.monster_max_aspd<<1;
		return adelay;
	}
	return 4000;
}
int status_get_amotion(struct block_list *bl)
{
	nullpo_retr(2000, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->amotion;
	else {
		struct status_change *sc_data=status_get_sc_data(bl);
		int amotion=2000,aspd_rate = 100,i;
		if(bl->type==BL_MOB && (struct mob_data *)bl)
			amotion = mob_db[((struct mob_data *)bl)->class_].amotion;
		else if(bl->type==BL_PET && (struct pet_data *)bl)
			amotion = mob_db[((struct pet_data *)bl)->class_].amotion;

		if(sc_data) {
			if(sc_data[SC_TWOHANDQUICKEN].timer != -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// 2HQ
				aspd_rate -= 30;
			if(sc_data[SC_ADRENALINE].timer != -1 && sc_data[SC_TWOHANDQUICKEN].timer == -1 &&
				sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1) {	// ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ…
				if(sc_data[SC_ADRENALINE].val2 || !battle_config.party_skill_penalty)
					aspd_rate -= 30;
				else
					aspd_rate -= 25;
			}
			if(sc_data[SC_SPEARSQUICKEN].timer != -1 && sc_data[SC_ADRENALINE].timer == -1 &&
				sc_data[SC_TWOHANDQUICKEN].timer == -1 && sc_data[SC_QUAGMIRE].timer == -1 && sc_data[SC_DONTFORGETME].timer == -1)	// ƒXƒsƒAƒNƒBƒbƒPƒ“
				aspd_rate -= sc_data[SC_SPEARSQUICKEN].val2;
			if(sc_data[SC_ASSNCROS].timer!=-1 && // —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX
				sc_data[SC_TWOHANDQUICKEN].timer==-1 && sc_data[SC_ADRENALINE].timer==-1 && sc_data[SC_SPEARSQUICKEN].timer==-1 &&
				sc_data[SC_DONTFORGETME].timer == -1)
				aspd_rate -= 5+sc_data[SC_ASSNCROS].val1+sc_data[SC_ASSNCROS].val2+sc_data[SC_ASSNCROS].val3;
			if(sc_data[SC_DONTFORGETME].timer!=-1)		// „‚ğ–Y‚ê‚È‚¢‚Å
				aspd_rate += sc_data[SC_DONTFORGETME].val1*3 + sc_data[SC_DONTFORGETME].val2 + (sc_data[SC_DONTFORGETME].val3>>16);
			if(sc_data[SC_STEELBODY].timer!=-1)	// ‹à„
				aspd_rate += 25;
			if(	sc_data[i=SC_SPEEDPOTION3].timer!=-1 || sc_data[i=SC_SPEEDPOTION2].timer!=-1 || sc_data[i=SC_SPEEDPOTION1].timer!=-1 || sc_data[i=SC_SPEEDPOTION0].timer!=-1)
				aspd_rate -= sc_data[i].val2;
			if(sc_data[SC_DEFENDER].timer != -1)
				aspd_rate += (25 - sc_data[SC_DEFENDER].val1*5);
				//amotion += (550 - sc_data[SC_DEFENDER].val1*50);
		}
		if(aspd_rate != 100)
			amotion = amotion*aspd_rate/100;
		if(amotion < battle_config.monster_max_aspd) amotion = battle_config.monster_max_aspd;
		return amotion;
	}
	return 2000;
}
int status_get_dmotion(struct block_list *bl)
{
	int ret;
	struct status_change *sc_data;

	nullpo_retr(0, bl);
	sc_data = status_get_sc_data(bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl){
		ret=mob_db[((struct mob_data *)bl)->class_].dmotion;
		if(battle_config.monster_damage_delay_rate != 100)
			ret = ret*battle_config.monster_damage_delay_rate/400;
	}
	else if(bl->type==BL_PC && (struct map_session_data *)bl){
		ret=((struct map_session_data *)bl)->dmotion;
		if(battle_config.pc_damage_delay_rate != 100)
			ret = ret*battle_config.pc_damage_delay_rate/400;
	}
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		ret=mob_db[((struct pet_data *)bl)->class_].dmotion;
	else
		return 2000;

	if((sc_data && (sc_data[SC_ENDURE].timer!=-1 || sc_data[SC_BERSERK].timer!=-1)) ||
		(bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.infinite_endure))
		ret=0;

	return ret;
}
int status_get_element(struct block_list *bl)
{
	int ret = 20;
	struct status_change *sc_data;

	nullpo_retr(ret, bl);
	sc_data = status_get_sc_data(bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)	// 10‚ÌˆÊLv*2A‚P‚ÌˆÊ‘®«
		ret=((struct mob_data *)bl)->def_ele;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		ret=20+((struct map_session_data *)bl)->def_ele;	// –hŒä‘®«Lv1
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		ret = mob_db[((struct pet_data *)bl)->class_].element;

	if(sc_data) {
		if( sc_data[SC_BENEDICTIO].timer!=-1 )	// ¹‘Ì~•Ÿ
			ret=26;
		if( sc_data[SC_FREEZE].timer!=-1 )	// “€Œ‹
			ret=21;
		if( sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2==0)
			ret=22;
	}

	return ret;
}

int status_get_attack_element(struct block_list *bl)
{
	int ret = 0;
	struct status_change *sc_data=status_get_sc_data(bl);

	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		ret=0;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		ret=((struct map_session_data *)bl)->atk_ele;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		ret=0;

	if(sc_data) {
		if( sc_data[SC_FROSTWEAPON].timer!=-1)	// ƒtƒƒXƒgƒEƒFƒ|ƒ“
			ret=1;
		if( sc_data[SC_SEISMICWEAPON].timer!=-1)	// ƒTƒCƒYƒ~ƒbƒNƒEƒFƒ|ƒ“
			ret=2;
		if( sc_data[SC_FLAMELAUNCHER].timer!=-1)	// ƒtƒŒ[ƒ€ƒ‰ƒ“ƒ`ƒƒ[
			ret=3;
		if( sc_data[SC_LIGHTNINGLOADER].timer!=-1)	// ƒ‰ƒCƒgƒjƒ“ƒOƒ[ƒ_[
			ret=4;
		if( sc_data[SC_ENCPOISON].timer!=-1)	// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“
			ret=5;
		if( sc_data[SC_ASPERSIO].timer!=-1)		// ƒAƒXƒyƒ‹ƒVƒI
			ret=6;
	}

	return ret;
}
int status_get_attack_element2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl) {
		int ret = ((struct map_session_data *)bl)->atk_ele_;
		struct status_change *sc_data = ((struct map_session_data *)bl)->sc_data;

		if(sc_data) {
			if( sc_data[SC_FROSTWEAPON].timer!=-1)	// ƒtƒƒXƒgƒEƒFƒ|ƒ“
				ret=1;
			if( sc_data[SC_SEISMICWEAPON].timer!=-1)	// ƒTƒCƒYƒ~ƒbƒNƒEƒFƒ|ƒ“
				ret=2;
			if( sc_data[SC_FLAMELAUNCHER].timer!=-1)	// ƒtƒŒ[ƒ€ƒ‰ƒ“ƒ`ƒƒ[
				ret=3;
			if( sc_data[SC_LIGHTNINGLOADER].timer!=-1)	// ƒ‰ƒCƒgƒjƒ“ƒOƒ[ƒ_[
				ret=4;
			if( sc_data[SC_ENCPOISON].timer!=-1)	// ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“
				ret=5;
			if( sc_data[SC_ASPERSIO].timer!=-1)		// ƒAƒXƒyƒ‹ƒVƒI
				ret=6;
		}
		return ret;
	}
	return 0;
}
int status_get_party_id(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.party_id;
	else if(bl->type==BL_MOB && (struct mob_data *)bl){
		struct mob_data *md=(struct mob_data *)bl;
		if( md->master_id>0 )
			return -md->master_id;
		return -md->bl.id;
	}
	else if(bl->type==BL_SKILL && (struct skill_unit *)bl)
		return ((struct skill_unit *)bl)->group->party_id;
	else
		return 0;
}
int status_get_guild_id(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data *)bl)->status.guild_id;
	else if(bl->type==BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->class_;
	else if(bl->type==BL_SKILL && (struct skill_unit *)bl)
		return ((struct skill_unit *)bl)->group->guild_id;
	else
		return 0;
}
int status_get_race(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].race;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return 7;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].race;
	else
		return 0;
}
int status_get_size(struct block_list *bl)
{
	nullpo_retr(1, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].size;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].size;
	else if(bl->type==BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		//if (pc_isriding(sd))	// fact or rumour?
		//	return 2;
		if (pc_calc_upper(sd->status.class_) == 2)
			return 0;
		return 1;
	} else
		return 1;
}
int status_get_mode(struct block_list *bl)
{
	nullpo_retr(0x01, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].mode;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].mode;
	else
		return 0x01;	// ‚Æ‚è‚ ‚¦‚¸“®‚­‚Æ‚¢‚¤‚±‚Æ‚Å1
}

int status_get_mexp(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].mexp;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].mexp;
	else
		return 0;
}
int status_get_race2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return mob_db[((struct mob_data *)bl)->class_].race2;
	else if(bl->type==BL_PET && (struct pet_data *)bl)
		return mob_db[((struct pet_data *)bl)->class_].race2;
	else
		return 0;
}
int status_isdead(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type == BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data *)bl)->state.state == MS_DEAD;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return pc_isdead((struct map_session_data *)bl);
	else
		return 0;
}

// StatusChangeŒn‚ÌŠ“¾
struct status_change *status_get_sc_data(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return ((struct mob_data*)bl)->sc_data;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return ((struct map_session_data*)bl)->sc_data;
	return NULL;
}
short *status_get_sc_count(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return &((struct mob_data*)bl)->sc_count;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return &((struct map_session_data*)bl)->sc_count;
	return NULL;
}
short *status_get_opt1(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return &((struct mob_data*)bl)->opt1;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return &((struct map_session_data*)bl)->opt1;
	else if(bl->type==BL_NPC && (struct npc_data *)bl)
		return &((struct npc_data*)bl)->opt1;
	return 0;
}
short *status_get_opt2(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return &((struct mob_data*)bl)->opt2;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return &((struct map_session_data*)bl)->opt2;
	else if(bl->type==BL_NPC && (struct npc_data *)bl)
		return &((struct npc_data*)bl)->opt2;
	return 0;
}
short *status_get_opt3(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return &((struct mob_data*)bl)->opt3;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return &((struct map_session_data*)bl)->opt3;
	else if(bl->type==BL_NPC && (struct npc_data *)bl)
		return &((struct npc_data*)bl)->opt3;
	return 0;
}
short *status_get_option(struct block_list *bl)
{
	nullpo_retr(0, bl);
	if(bl->type==BL_MOB && (struct mob_data *)bl)
		return &((struct mob_data*)bl)->option;
	else if(bl->type==BL_PC && (struct map_session_data *)bl)
		return &((struct map_session_data*)bl)->status.option;
	else if(bl->type==BL_NPC && (struct npc_data *)bl)
		return &((struct npc_data*)bl)->option;
	return 0;
}

int status_get_sc_def(struct block_list *bl, int type)
{
	int sc_def;
	nullpo_retr(0, bl);
	
	switch (type)
	{
	case SP_MDEF1:	// mdef
		sc_def = 100 - (3 + status_get_mdef(bl) + status_get_luk(bl)/3);
		break;
	case SP_MDEF2:	// int
		sc_def = 100 - (3 + status_get_int(bl) + status_get_luk(bl)/3);
		break;
	case SP_DEF1:	// def
		sc_def = 100 - (3 + status_get_def(bl) + status_get_luk(bl)/3);
		break;
	case SP_DEF2:	// vit
		sc_def = 100 - (3 + status_get_vit(bl) + status_get_luk(bl)/3);
		break;
	case SP_LUK:	// luck
		sc_def = 100 - (3 + status_get_luk(bl));
		break;

	case SC_STONE:
	case SC_FREEZE:
		sc_def = 100 - (3 + status_get_mdef(bl) + status_get_luk(bl)/3);
		break;
	case SC_STAN:
	case SC_POISON:
	case SC_SILENCE:
		sc_def = 100 - (3 + status_get_vit(bl) + status_get_luk(bl)/3);
		break;	
	case SC_SLEEP:
	case SC_CONFUSION:
	case SC_BLIND:
		sc_def = 100 - (3 + status_get_int(bl) + status_get_luk(bl)/3);
		break;
	case SC_CURSE:
		sc_def = 100 - (3 + status_get_luk(bl));
		break;	

	default:
		sc_def = 100;
		break;
	}

	if(bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)bl;
		if (md && md->class_ == 1288)
			return 0;
		if (sc_def < 50)
			sc_def = 50;
	} else if(bl->type == BL_PC) {
		struct status_change* sc_data = status_get_sc_data(bl);
		if (sc_data && sc_data[SC_GOSPEL].timer != -1 &&
			sc_data[SC_GOSPEL].val4 == BCT_PARTY &&
			sc_data[SC_GOSPEL].val3 == 3)
			sc_def -= 25;
	}

	return (sc_def < 0) ? 0 : sc_def;
}

/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíŠJn
 *------------------------------------------
 */
int status_change_start(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag)
{
	struct map_session_data *sd = NULL;
	struct status_change* sc_data;
	short *sc_count, *option, *opt1, *opt2, *opt3;
	int opt_flag = 0, calc_flag = 0,updateflag = 0, save_flag = 0, race, mode, elem, undead_flag;
	int scdef = 0;
	int type2 = type;

	nullpo_retr(0, bl);
	if(bl->type == BL_SKILL)
		return 0;
	if(bl->type == BL_MOB)
		if (status_isdead(bl)) return 0;

	nullpo_retr(0, sc_data=status_get_sc_data(bl));
	nullpo_retr(0, sc_count=status_get_sc_count(bl));
	nullpo_retr(0, option=status_get_option(bl));
	nullpo_retr(0, opt1=status_get_opt1(bl));
	nullpo_retr(0, opt2=status_get_opt2(bl));
	nullpo_retr(0, opt3=status_get_opt3(bl));


	race=status_get_race(bl);
	mode=status_get_mode(bl);
	elem=status_get_elem_type(bl);
	undead_flag=battle_check_undead(race,elem);

	if(type == SC_AETERNA && (sc_data[SC_STONE].timer != -1 || sc_data[SC_FREEZE].timer != -1) )
		return 0;

	switch(type){
		case SC_STONE:
		case SC_FREEZE:
			scdef=3+status_get_mdef(bl)+status_get_luk(bl)/3;
			break;
		case SC_STAN:
		case SC_SILENCE:
		case SC_POISON:
		case SC_DPOISON:
			scdef=3+status_get_vit(bl)+status_get_luk(bl)/3;
			break;
		case SC_SLEEP:
		case SC_BLIND:
			scdef=3+status_get_int(bl)+status_get_luk(bl)/3;
			break;
		case SC_CURSE:
			scdef=3+status_get_luk(bl);
			break;

//		case SC_CONFUSION:
		default:
			scdef=0;
	}
	if(scdef>=100)
		return 0;
	if(bl->type==BL_PC){
		sd=(struct map_session_data *)bl;
		if( sd && type == SC_ADRENALINE && !(skill_get_weapontype(BS_ADRENALINE)&(1<<sd->status.weapon)))
			return 0;

		if(SC_STONE<=type && type<=SC_BLIND){	/* ƒJ?ƒh‚É‚æ‚é‘Ï« */
			if( sd && sd->reseff[type-SC_STONE] > 0 && rand()%10000<sd->reseff[type-SC_STONE]){
				if(battle_config.battle_log)
					printf("PC %d skill_sc_start: card‚É‚æ‚éˆÙí‘Ï«?“®\n",sd->bl.id);
				return 0;
			}
		}
	}
	else if(bl->type == BL_MOB) {		
	}
	else {
		if(battle_config.error_log)
			printf("status_change_start: neither MOB nor PC !\n");
		return 0;
	}

	if(type==SC_FREEZE && undead_flag && !(flag&1))
		return 0;

	if (type==SC_BLESSING && (bl->type==BL_PC || (!undead_flag && race!=6))) {
		if (sc_data[SC_CURSE].timer!=-1)
			status_change_end(bl,SC_CURSE,-1);
		if (sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2==0)
			status_change_end(bl,SC_STONE,-1);
	}

	if((type == SC_ADRENALINE || type == SC_WEAPONPERFECTION || type == SC_OVERTHRUST) &&
		sc_data[type].timer != -1 && sc_data[type].val2 && !val2)
		return 0;

	if(mode & 0x20 && (type==SC_STONE || type==SC_FREEZE ||
		type==SC_STAN || type==SC_SLEEP || type==SC_SILENCE || type==SC_QUAGMIRE || type == SC_DECREASEAGI || type == SC_SIGNUMCRUCIS || type == SC_PROVOKE ||
		(type == SC_BLESSING && (undead_flag || race == 6))) && !(flag&1)){
		/* ƒ{ƒX‚É‚Í?‚©‚È‚¢(‚½‚¾‚µƒJ?ƒh‚É‚æ‚é?‰Ê‚Í“K—p‚³‚ê‚é) */
		return 0;
	}
	if(type==SC_FREEZE || type==SC_STAN || type==SC_SLEEP)
		battle_stopwalking(bl,1);

	if(sc_data[type].timer != -1){	/* ‚·‚Å‚É“¯‚¶ˆÙí‚É‚È‚Á‚Ä‚¢‚éê‡ƒ^ƒCƒ}‰ğœ */
		if(sc_data[type].val1 > val1 && type != SC_COMBO && type != SC_DANCING && type != SC_DEVOTION &&
			type != SC_SPEEDPOTION0 && type != SC_SPEEDPOTION1 && type != SC_SPEEDPOTION2 && type != SC_SPEEDPOTION3
			&& type != SC_ATKPOT && type != SC_MATKPOT) // added atk and matk potions [Valaris]
			return 0;

		if ((type >=SC_STAN && type <= SC_BLIND) || type == SC_DPOISON)
			return 0;/* ?‚¬‘«‚µ‚ª‚Å‚«‚È‚¢?‘ÔˆÙí‚Å‚ ‚é‚Í?‘ÔˆÙí‚ğs‚í‚È‚¢ */

		(*sc_count)--;
		delete_timer(sc_data[type].timer, status_change_timer);
		sc_data[type].timer = -1;
	}

	// ƒNƒAƒOƒ}ƒCƒA/„‚ğ–Y‚ê‚È‚¢‚Å’†‚Í–³Œø‚ÈƒXƒLƒ‹
	if ((sc_data[SC_QUAGMIRE].timer!=-1 || sc_data[SC_DONTFORGETME].timer!=-1) &&
		(type==SC_CONCENTRATE || type==SC_INCREASEAGI ||
		type==SC_TWOHANDQUICKEN || type==SC_SPEARSQUICKEN ||
		type==SC_ADRENALINE || type==SC_LOUD || type==SC_TRUESIGHT ||
		type==SC_WINDWALK || type==SC_CARTBOOST || type==SC_ASSNCROS))
	return 0;

	switch(type){	/* ˆÙí‚Ìí—Ş‚²‚Æ‚Ì?— */
		case SC_PROVOKE:			/* ƒvƒƒ{ƒbƒN */
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	/* (ƒI?ƒgƒo?ƒT?ƒN) */
			break;
		case SC_ENDURE:				/* ƒCƒ“ƒfƒ…ƒA */
			if(tick <= 0) tick = 1000 * 60;
			calc_flag = 1; // for updating mdef
#ifdef TWILIGHT
			val2 = 40; // [Celest]
#else
			val2 = 7; // [Celest]
#endif
			break;
		case SC_AUTOBERSERK:
			{
				tick = 60*1000;
				if (bl->type == BL_PC && sd->status.hp<sd->status.max_hp>>2 &&
					(sc_data[SC_PROVOKE].timer==-1 || sc_data[SC_PROVOKE].val2==0))
					status_change_start(bl,SC_PROVOKE,10,1,0,0,0,0);
			}
			break;

		case SC_CONCENTRATE:		/* W’†—ÍŒüã */
		case SC_BLESSING:			/* ƒuƒŒƒbƒVƒ“ƒO */
		case SC_ANGELUS:			/* ƒAƒ“ƒ[ƒ‹ƒX */
			calc_flag = 1;
			break;
		
		case SC_INCREASEAGI:		/* ‘¬“xã¸ */
			calc_flag = 1;
			if(sc_data[SC_DECREASEAGI].timer!=-1 )
				status_change_end(bl,SC_DECREASEAGI,-1);
			// the effect will still remain [celest]
//			if(sc_data[SC_WINDWALK].timer!=-1 )	/* ƒEƒCƒ“ƒhƒEƒH?ƒN */
//				status_change_end(bl,SC_WINDWALK,-1);
			break;
		case SC_DECREASEAGI:		/* ‘¬“xŒ¸­ */
			if (bl->type == BL_PC)	// Celest
				tick>>=1;
			calc_flag = 1;
			if(sc_data[SC_INCREASEAGI].timer!=-1 )
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			break;
		case SC_SIGNUMCRUCIS:		/* ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX */
			calc_flag = 1;
//			val2 = 14 + val1;
			val2 = 10 + val1*2;
			tick = 600*1000;
			clif_emotion(bl,4);
			break;
		case SC_SLOWPOISON:
			if (sc_data[SC_POISON].timer == -1 && sc_data[SC_DPOISON].timer == -1)
				return 0;
			break;
		case SC_TWOHANDQUICKEN:		/* 2HQ */
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				return 0;
			*opt3 |= 1;
			calc_flag = 1;
			break;
		case SC_ADRENALINE:			/* ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ… */
			if(sc_data[SC_DECREASEAGI].timer!=-1)
				return 0;
			if(bl->type == BL_PC)
				if(pc_checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			calc_flag = 1;
			break;
		case SC_WEAPONPERFECTION:	/* ƒEƒFƒ|ƒ“ƒp?ƒtƒFƒNƒVƒ‡ƒ“ */
			if(bl->type == BL_PC)
				if(pc_checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			break;
		case SC_OVERTHRUST:			/* ƒI?ƒo?ƒXƒ‰ƒXƒg */
			if(bl->type == BL_PC)
				if(pc_checkskill(sd,BS_HILTBINDING)>0)
					tick += tick / 10;
			*opt3 |= 2;
			break;
		case SC_MAXIMIZEPOWER:		/* ƒ}ƒLƒVƒ}ƒCƒYƒpƒ?(SP‚ª1Œ¸‚éŠÔ,val2‚É‚à) */
			if(bl->type == BL_PC)
				val2 = tick;
			else
				tick = 5000*val1;
			break;
		case SC_ENCPOISON:			/* ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“ */
			calc_flag = 1;
			val2=(((val1 - 1) / 2) + 3)*100;	/* “Å•t?Šm—¦ */
			skill_enchant_elemental_end(bl,SC_ENCPOISON);
			break;
		case SC_EDP:	// [Celest]
			val2 = val1 + 2;			/* –Ò“Å•t?Šm—¦(%) */
			calc_flag = 1;
			break;
		case SC_POISONREACT:	/* ƒ|ƒCƒYƒ“ƒŠƒAƒNƒg */
			val2=val1/2 + val1%2; // [Celest]
			break;
		case SC_IMPOSITIO:			/* ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX */
			calc_flag = 1;
			break;
		case SC_ASPERSIO:			/* ƒAƒXƒyƒ‹ƒVƒI */
			skill_enchant_elemental_end(bl,SC_ASPERSIO);
			break;
		case SC_SUFFRAGIUM:			/* ƒTƒtƒ‰ƒMƒ€ */
		case SC_BENEDICTIO:			/* ¹? */
		case SC_MAGNIFICAT:			/* ƒ}ƒOƒjƒtƒBƒJ?ƒg */
		case SC_AETERNA:			/* ƒG?ƒeƒ‹ƒi */
			break;
		case SC_ENERGYCOAT:			/* ƒGƒiƒW?ƒR?ƒg */
			*opt3 |= 4;
			break;
		case SC_MAGICROD:
			val2 = val1*20;
			break;
		case SC_KYRIE:				/* ƒLƒŠƒGƒGƒŒƒCƒ\ƒ“ */
			val2 = status_get_max_hp(bl) * (val1 * 2 + 10) / 100;/* ‘Ï‹v“x */
			val3 = (val1 / 2 + 5);	/* ‰ñ? */
// -- moonsoul (added to undo assumptio status if target has it)
			if(sc_data[SC_ASSUMPTIO].timer!=-1 )
				status_change_end(bl,SC_ASSUMPTIO,-1);
			break;
		case SC_MINDBREAKER:
			calc_flag = 1;
			if(tick <= 0) tick = 1000;	/* (ƒI?ƒgƒo?ƒT?ƒN) */
		case SC_GLORIA:				/* ƒOƒƒŠƒA */
			calc_flag = 1;
			break;
		case SC_LOUD:				/* ƒ‰ƒEƒhƒ{ƒCƒX */
			calc_flag = 1;
			break;
		case SC_TRICKDEAD:			/* €‚ñ‚¾‚Ó‚è */
			if (bl->type == BL_PC) {
				pc_stopattack((struct map_session_data *)sd);
			}
			break;
		case SC_QUAGMIRE:			/* ƒNƒ@ƒOƒ}ƒCƒA */
			calc_flag = 1;
			if(sc_data[SC_CONCENTRATE].timer!=-1 )	/* W’†—ÍŒüã‰ğœ */
				status_change_end(bl,SC_CONCENTRATE,-1);
			if(sc_data[SC_INCREASEAGI].timer!=-1 )	/* ‘¬“xã¸‰ğœ */
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_LOUD].timer!=-1 )
				status_change_end(bl,SC_LOUD,-1);
			if(sc_data[SC_TRUESIGHT].timer!=-1 )	/* ƒgƒDƒ‹?ƒTƒCƒg */
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc_data[SC_WINDWALK].timer!=-1 )	/* ƒEƒCƒ“ƒhƒEƒH?ƒN */
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc_data[SC_CARTBOOST].timer!=-1 )	/* ƒJ?ƒgƒu?ƒXƒg */
				status_change_end(bl,SC_CARTBOOST,-1);
			break;
		case SC_MAGICPOWER:
			calc_flag = 1;
			val2 = 1;
			break;
		case SC_SACRIFICE:
			val2 = 5;
			break;
		case SC_FLAMELAUNCHER:		/* ƒtƒŒ?ƒ€ƒ‰ƒ“ƒ`ƒƒ? */
			skill_enchant_elemental_end(bl,SC_FLAMELAUNCHER);
			break;
		case SC_FROSTWEAPON:		/* ƒtƒƒXƒgƒEƒFƒ|ƒ“ */
			skill_enchant_elemental_end(bl,SC_FROSTWEAPON);
			break;
		case SC_LIGHTNINGLOADER:	/* ƒ‰ƒCƒgƒjƒ“ƒOƒ?ƒ_? */
			skill_enchant_elemental_end(bl,SC_LIGHTNINGLOADER);
			break;
		case SC_SEISMICWEAPON:		/* ƒTƒCƒYƒ~ƒbƒNƒEƒFƒ|ƒ“ */
			skill_enchant_elemental_end(bl,SC_SEISMICWEAPON);
			break;
		case SC_DEVOTION:			/* ƒfƒBƒ{?ƒVƒ‡ƒ“ */
			calc_flag = 1;
			break;
		case SC_PROVIDENCE:			/* ƒvƒƒ”ƒBƒfƒ“ƒX */
			calc_flag = 1;
			val2=val1*5;
			break;
		case SC_REFLECTSHIELD:
			val2=10+val1*3;
			break;
		case SC_STRIPWEAPON:
			if (val2==0) val2=90;
			break;
		case SC_STRIPSHIELD:
			if (val2==0) val2=85;
			break;
		case SC_STRIPARMOR:
		case SC_STRIPHELM:
		case SC_CP_WEAPON:
		case SC_CP_SHIELD:
		case SC_CP_ARMOR:
		case SC_CP_HELM:
			break;

		case SC_AUTOSPELL:			/* ƒI?ƒgƒXƒyƒ‹ */
			val4 = 5 + val1*2;
			break;

		case SC_VOLCANO:
			calc_flag = 1;
			val3 = val1*10;
			//val4 = val1>=5?20: (val1==4?19: (val1==3?17: ( val1==2?14:10 ) ) );
			break;
		case SC_DELUGE:
			calc_flag = 1;
			//val3 = val1>=5?15: (val1==4?14: (val1==3?12: ( val1==2?9:5 ) ) );
			//val4 = val1>=5?20: (val1==4?19: (val1==3?17: ( val1==2?14:10 ) ) );
			if (sc_data[SC_FOGWALL].timer != -1 && sc_data[SC_BLIND].timer != -1)
				status_change_end(bl,SC_BLIND,-1);
			break;
		case SC_VIOLENTGALE:
			calc_flag = 1;
			val3 = val1*3;
			//val4 = val1>=5?20: (val1==4?19: (val1==3?17: ( val1==2?14:10 ) ) );
			break;

		case SC_SPEARSQUICKEN:		/* ƒXƒsƒAƒNƒCƒbƒPƒ“ */
			calc_flag = 1;
			val2 = 20+val1;
			*opt3 |= 1;
			break;
		case SC_COMBO:
			break;
		case SC_BLADESTOP_WAIT:		/* ”’næ‚è(‘Ò‚¿) */
			break;
		case SC_BLADESTOP:		/* ”’næ‚è */
			if(val2==2) clif_bladestop((struct block_list *)val3,(struct block_list *)val4,1);
			*opt3 |= 32;
			break;

		case SC_LULLABY:			/* qç‰S */
			val2 = 11;
			break;
		case SC_RICHMANKIM:
			break;
		case SC_ETERNALCHAOS:		/* ƒGƒ^?ƒiƒ‹ƒJƒIƒX */
			calc_flag = 1;
			break;
		case SC_DRUMBATTLE:			/* ?‘¾ŒÛ‚Ì‹¿‚« */
			calc_flag = 1;
			val2 = (val1+1)*25;
			val3 = (val1+1)*2;
			break;
		case SC_NIBELUNGEN:			/* ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö */
			calc_flag = 1;
			//val2 = (val1+2)*50;
			val3 = (val1+2)*25;
			break;
		case SC_ROKISWEIL:			/* ƒƒL‚Ì‹©‚Ñ */
			break;
		case SC_INTOABYSS:			/* [•£‚Ì’†‚É */
			break;
		case SC_SIEGFRIED:			/* •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh */
			calc_flag = 1;
			val2 = 55 + val1*5;
			val3 = val1*10;
			break;
		case SC_DISSONANCE:			/* •s‹¦˜a‰¹ */
			val2 = 10;
			break;
		case SC_WHISTLE:			/* Œû“J */
			calc_flag = 1;
			break;
		case SC_ASSNCROS:			/* —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX */
			calc_flag = 1;
			break;
		case SC_POEMBRAGI:			/* ƒuƒ‰ƒM‚Ì */
			break;
		case SC_APPLEIDUN:			/* ƒCƒhƒDƒ“‚Ì—ÑŒç */
			calc_flag = 1;
			break;
		case SC_UGLYDANCE:			/* ©•ªŸè‚Èƒ_ƒ“ƒX */
			val2 = 10;
			break;
		case SC_HUMMING:			/* ƒnƒ~ƒ“ƒO */
			calc_flag = 1;
			break;
		case SC_DONTFORGETME:		/* „‚ğ–Y‚ê‚È‚¢‚Å */
			calc_flag = 1;
			if(sc_data[SC_INCREASEAGI].timer!=-1 )	/* ‘¬“xã¸‰ğœ */
				status_change_end(bl,SC_INCREASEAGI,-1);
			if(sc_data[SC_TWOHANDQUICKEN].timer!=-1 )
				status_change_end(bl,SC_TWOHANDQUICKEN,-1);
			if(sc_data[SC_SPEARSQUICKEN].timer!=-1 )
				status_change_end(bl,SC_SPEARSQUICKEN,-1);
			if(sc_data[SC_ADRENALINE].timer!=-1 )
				status_change_end(bl,SC_ADRENALINE,-1);
			if(sc_data[SC_ASSNCROS].timer!=-1 )
				status_change_end(bl,SC_ASSNCROS,-1);
			if(sc_data[SC_TRUESIGHT].timer!=-1 )	/* ƒgƒDƒ‹?ƒTƒCƒg */
				status_change_end(bl,SC_TRUESIGHT,-1);
			if(sc_data[SC_WINDWALK].timer!=-1 )	/* ƒEƒCƒ“ƒhƒEƒH?ƒN */
				status_change_end(bl,SC_WINDWALK,-1);
			if(sc_data[SC_CARTBOOST].timer!=-1 )	/* ƒJ?ƒgƒu?ƒXƒg */
				status_change_end(bl,SC_CARTBOOST,-1);
			break;
		case SC_FORTUNE:			/* K‰^‚ÌƒLƒX */
			calc_flag = 1;
			break;
		case SC_SERVICE4U:			/* ƒT?ƒrƒXƒtƒH?ƒ†? */
			calc_flag = 1;
			break;
		case SC_MOONLIT:
			val2 = bl->id;
			break;
		case SC_DANCING:			/* ƒ_ƒ“ƒX/‰‰‘t’† */
			calc_flag = 1;
			val3= tick / 1000;
			tick = 1000;
			break;

		case SC_EXPLOSIONSPIRITS:	// ”š—ô”g“®
			calc_flag = 1;
			val2 = 75 + 25*val1;
			*opt3 |= 8;
			break;
		case SC_STEELBODY:			// ‹à„
			calc_flag = 1;
			*opt3 |= 16;
			break;
		case SC_EXTREMITYFIST:		/* ˆ¢C—…”e™€Œ */
			break;
		case SC_AUTOCOUNTER:
			val3 = val4 = 0;
			break;

		case SC_SPEEDPOTION0:		/* ?‘¬ƒ|?ƒVƒ‡ƒ“ */
		case SC_SPEEDPOTION1:
		case SC_SPEEDPOTION2:
		case SC_SPEEDPOTION3:
			calc_flag = 1;
			tick = 1000 * tick;
			val2 = 5*(2+type-SC_SPEEDPOTION0);
			break;

		/* atk & matk potions [Valaris] */
		case SC_ATKPOT:
		case SC_MATKPOT:
			calc_flag = 1;
			tick = 1000 * tick;
			break;
		case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
			{
				time_t timer;

				calc_flag = 1;
				tick = 10000;
				if(!val2)
					val2 = time(&timer);
			}
			break;
		case SC_NOCHAT:	//ƒ`ƒƒƒbƒg‹Ö~?‘Ô
			{
				time_t timer;

				if(!battle_config.muting_players)
					break;

				tick = 60000;
				if(!val2)
					val2 = time(&timer);
				updateflag = SP_MANNER;
				save_flag = 1; // celest
			}
			break;
		case SC_SELFDESTRUCTION: //©”š
			clif_skillcasting(bl,bl->id, bl->id,0,0,331,skill_get_time(val2,val1));
			val3 = tick / 1000;
			tick = 1000;
			break;

		/* option1 */
		case SC_STONE:				/* Î‰» */
			if(!(flag&2)) {
				int sc_def = status_get_mdef(bl)*200;
				tick = tick - sc_def;
			}
			val3 = tick/1000;
			if(val3 < 1) val3 = 1;
			tick = 5000;
			val2 = 1;
			break;
		case SC_SLEEP:				/* ‡–° */
			if(!(flag&2)) {
//				int sc_def = 100 - (status_get_int(bl) + status_get_luk(bl)/3);
//				tick = tick * sc_def / 100;
//				if(tick < 1000) tick = 1000;
				tick = 30000;//‡–°‚ÍƒXƒe?ƒ^ƒX‘Ï«‚É?‚í‚ç‚¸30•b
			}
			break;
		case SC_FREEZE:				/* “€Œ‹ */
			if(!(flag&2)) {
				int sc_def = 100 - status_get_mdef(bl);
				tick = tick * sc_def / 100;
			}
			break;
		case SC_STAN:				/* ƒXƒ^ƒ“ival2‚Éƒ~ƒŠ•bƒZƒbƒgj */
			if(!(flag&2)) {
				int sc_def = status_get_sc_def_vit(bl);
				tick = tick * sc_def / 100;
			}
			break;

			/* option2 */
		case SC_DPOISON:			/* –Ò“Å */
		{
			int mhp = status_get_max_hp(bl);
			int hp = status_get_hp(bl);
			// MHP?1/4????????
			if (hp > mhp>>2) {
				if(bl->type == BL_PC) {
					int diff = mhp*10/100;
					if (hp - diff < mhp>>2)
						hp = hp - (mhp>>2);
					pc_heal((struct map_session_data *)bl, -hp, 0);
				} else if(bl->type == BL_MOB) {
					struct mob_data *md = (struct mob_data *)bl;
					hp -= mhp*15/100;
					if (hp > mhp>>2)
						md->hp = hp;
					else
						md->hp = mhp>>2;
				}
			}
		}	// fall through
		case SC_POISON:				/* “Å */
			calc_flag = 1;
			if(!(flag&2)) {
				int sc_def = 100 - (status_get_vit(bl) + status_get_luk(bl)/5);
				tick = tick * sc_def / 100;
			}
			val3 = tick/1000;
			if(val3 < 1) val3 = 1;
			tick = 1000;
			break;
		case SC_SILENCE:			/* ’¾?iƒŒƒbƒNƒXƒfƒr?ƒij */
			if (sc_data && sc_data[SC_GOSPEL].timer!=-1) {
				skill_delunitgroup((struct skill_unit_group *)sc_data[SC_GOSPEL].val3);
				status_change_end(bl,SC_GOSPEL,-1);
				break;
			}
			if(!(flag&2)) {
				int sc_def = 100 - status_get_vit(bl);
				tick = tick * sc_def / 100;
			}
			break;
		case SC_CONFUSION:
			val2 = tick;
			tick = 100;
			clif_emotion(bl,1);
			if (sd) {
				pc_stop_walking (sd, 0);
			}
			break;
		case SC_BLIND:				/* ˆÃ? */
			calc_flag = 1;
			if(!(flag&2)) {
				int sc_def = status_get_lv(bl)/10 + status_get_int(bl)/15;
				tick = 30000 - sc_def;
			}
			break;
		case SC_CURSE:
			calc_flag = 1;
			if(!(flag&2)) {
				int sc_def = 100 - status_get_vit(bl);
				tick = tick * sc_def / 100;
			}
			break;

		/* option */
		case SC_HIDING:		/* ƒnƒCƒfƒBƒ“ƒO */
			calc_flag = 1;
			if(bl->type == BL_PC) {
				val2 = tick / 1000;		/* ?ŠÔ */
				tick = 1000;
			}
			break;
		case SC_CHASEWALK:
		case SC_CLOAKING:		/* ƒNƒ?ƒLƒ“ƒO */
			if(bl->type == BL_PC) {
				calc_flag = 1; // [Celest]
				val2 = tick;
				val3 = type==SC_CLOAKING ? 130-val1*3 : 135-val1*5;
			}
			else
				tick = 5000*val1;
			break;
		case SC_SIGHT:			/* ƒTƒCƒg/ƒ‹ƒAƒt */
		case SC_RUWACH:
			val2 = tick/250;
			tick = 10;
			break;

		/* ƒZ?ƒtƒeƒBƒEƒH?ƒ‹Aƒjƒ…?ƒ} */
		case SC_SAFETYWALL:	case SC_PNEUMA:
			tick=((struct skill_unit *)val2)->group->limit;
			break;

		/* ƒAƒ“ƒNƒ‹ */
		case SC_ANKLE:
			break;

		/* ƒXƒLƒ‹‚¶‚á‚È‚¢/ŠÔ‚É?ŒW‚µ‚È‚¢ */
		case SC_RIDING:
			calc_flag = 1;
			tick = 600*1000;
			break;
		case SC_FALCON:
		case SC_WEIGHT50:
		case SC_WEIGHT90:
		case SC_BROKNWEAPON:
		case SC_BROKNARMOR:
			tick=600*1000;
			break;

		case SC_AUTOGUARD:
			{
				int i,t;
				for(i=val2=0;i<val1;i++) {
					t = 5-(i>>1);
					val2 += (t < 0)? 1:t;
				}
			}
			break;

		case SC_DEFENDER:
			calc_flag = 1;
			val2 = 5 + val1*15;
			break;

		case SC_KEEPING:
		case SC_BARRIER:
			calc_flag = 1;

		case SC_HALLUCINATION:
			break;

		case SC_CONCENTRATION:	/* ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“ */
			*opt3 |= 1;
			calc_flag = 1;
			break;

		case SC_TENSIONRELAX:	/* ƒeƒ“ƒVƒ‡ƒ“ƒŠƒ‰ƒbƒNƒX */
			if(bl->type == BL_PC) {
				tick = 10000;
			} else return 0;
			break;

		case SC_AURABLADE:		/* ƒI?ƒ‰ƒuƒŒ?ƒh */
		case SC_PARRYING:		/* ƒpƒŠƒCƒ“ƒO */
//		case SC_ASSUMPTIO:		/*  */
//		case SC_HEADCRUSH:		/* ƒwƒbƒhƒNƒ‰ƒbƒVƒ… */
//		case SC_JOINTBEAT:		/* ƒWƒ‡ƒCƒ“ƒgƒr?ƒg */
//		case SC_MARIONETTE:		/* ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹ */

			//‚Æ‚è‚ ‚¦‚¸è?‚«
			break;

// -- moonsoul	(for new upper class related skill status effects)
/*
		case SC_AURABLADE:
			val2 = val1*10;
			break;
		case SC_PARRYING:
			val2=val1*3;
			break;
		case SC_CONCENTRATION:
			calc_flag=1;
			val2=val1*10;
			val3=val1*5;
			break;
		case SC_TENSIONRELAX:
//			val2 = 10;
//			val3 = 15;
			break;
		case SC_BERSERK:
			calc_flag=1;
			break;
		case SC_ASSUMPTIO:
			if(sc_data[SC_KYRIE].timer!=-1 )
				status_change_end(bl,SC_KYRIE,-1);
				break;*/

		case SC_WINDWALK:		/* ƒEƒCƒ“ƒhƒEƒH?ƒN */
			calc_flag = 1;
			val2 = (val1 / 2); //Fleeã¸—¦
			break;
		
		case SC_JOINTBEAT: // Random break [DracoRPG]
			calc_flag = 1;
			val2 = rand()%6 + 1;
			if (val2 == 6) status_change_start(bl,SC_BLEEDING,val1,0,0,0,skill_get_time2(type,val1),0);
			break;

		case SC_BERSERK:		/* ƒo?ƒT?ƒN */
			if(sd){
				sd->status.hp = sd->status.max_hp * 3;
				sd->status.sp = 0;
				clif_updatestatus(sd,SP_HP);
				clif_updatestatus(sd,SP_SP);
				clif_status_change(bl,SC_INCREASEAGI,1);	/* ƒAƒCƒRƒ“•\¦ */
				sd->canregen_tick = gettick() + 300000;
			}
			*opt3 |= 128;
			tick = 10000;
			calc_flag = 1;
			break;

		case SC_ASSUMPTIO:		/* ƒAƒXƒ€ƒvƒeƒBƒI */
			if(sc_data[SC_KYRIE].timer!=-1 )
				status_change_end(bl,SC_KYRIE,-1);
				break;
			*opt3 |= 2048;
			break;

		case SC_BASILICA: // [celest]
			break;

		case SC_GOSPEL:
			if (val4 == BCT_SELF) {	// self effect
				int i;
				if (sd) {
					sd->canact_tick += tick;
					sd->canmove_tick += tick;
				}
				val2 = tick;
				tick = 1000;
				for (i=0; i<=26; i++) {
					if(sc_data[i].timer!=-1)
						status_change_end(bl,i,-1);
				}
				for (i=58; i<=62; i++) {
					if(sc_data[i].timer!=-1)
						status_change_end(bl,i,-1);
				}
				for (i=132; i<=136; i++) {
					if(sc_data[i].timer!=-1)
						status_change_end(bl,i,-1);
				}
			}
			break;

		case SC_MARIONETTE:		/* ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹ */
		case SC_MARIONETTE2:
			val2 = tick;
			if (!val3)
				return 0;
			tick = 1000;
			calc_flag = 1;
			*opt3 |= 1024;
			break;

		case SC_MELTDOWN:		/* ƒƒ‹ƒgƒ_ƒEƒ“ */
		case SC_CARTBOOST:		/* ƒJ?ƒgƒu?ƒXƒg */
		case SC_TRUESIGHT:		/* ƒgƒDƒ‹?ƒTƒCƒg */
		case SC_SPIDERWEB:		/* ƒXƒpƒCƒ_?ƒEƒFƒbƒu */
			calc_flag = 1;
			break;

		case SC_REJECTSWORD:	/* ƒŠƒWƒFƒNƒgƒ\?ƒh */
			val2 = 3; //3‰ñU?‚ğ’µ‚Ë•Ô‚·
			break;

		case SC_MEMORIZE:		/* ƒƒ‚ƒ‰ƒCƒY */
			val2 = 5; //‰ñ‰r¥‚ğ1/3‚É‚·‚é
			break;

		case SC_SPLASHER:		/* ƒxƒiƒ€ƒXƒvƒ‰ƒbƒVƒƒ? */
			break;

		case SC_FOGWALL:
			break;

		case SC_PRESERVE:
			break;

		case SC_BLEEDING:
			{
				// every 1 vit deducts 1 second
				val3 = tick - status_get_vit(bl) * 1000;
				// minimum 50 seconds
				if (val3 < 50000)
					val3 = 50000;
				val4 = 10000;
				tick = 1000;
			}
			break;

		case SC_SLOWDOWN:
		case SC_SPEEDUP0:
			calc_flag = 1;
			break;

		case SC_REGENERATION:
			val1 = 2;
		case SC_BATTLEORDERS:
			tick = 60000; // 1 minute
			calc_flag = 1;
			break;

		case SC_GUILDAURA:
			calc_flag = 1;
			tick = 1000;
			break;

		case SC_BABY:
			type2 = _SC_BABY;
			break;

		default:
			if(battle_config.error_log)
				printf("UnknownStatusChange [%d]\n", type);
			return 0;
	}

	if(bl->type==BL_PC &&
		(type<SC_SENDMAX || type==SC_PRESERVE || type==SC_BATTLEORDERS || type==SC_BABY))
		clif_status_change(bl,type2,1);	/* ƒAƒCƒRƒ“•\¦ */

	/* option‚Ì?X */
	switch(type){
		case SC_STONE:
		case SC_FREEZE:
		case SC_STAN:
		case SC_SLEEP:
			battle_stopattack(bl);	/* U?’â~ */
			skill_stop_dancing(bl,0);	/* ‰‰‘t/ƒ_ƒ“ƒX‚Ì’†? */
			{	/* “¯‚ÉŠ|‚©‚ç‚È‚¢ƒXƒe?ƒ^ƒXˆÙí‚ğ‰ğœ */
				int i;
				for(i = SC_STONE; i <= SC_SLEEP; i++){
					if(sc_data[i].timer != -1){
						(*sc_count)--;
						delete_timer(sc_data[i].timer, status_change_timer);
						sc_data[i].timer = -1;
					}
				}
			}
			if(type == SC_STONE)
				*opt1 = 6;
			else
				*opt1 = type - SC_STONE + 1;
			opt_flag = 1;
			break;
		case SC_POISON:
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			*opt2 |= 1<<(type-SC_POISON);
			opt_flag = 1;
			break;
		case SC_DPOISON:	// b’è‚Å“Å‚ÌƒGƒtƒFƒNƒg‚ğg—p
			*opt2 |= 1;
			opt_flag = 1;
			break;
		case SC_SIGNUMCRUCIS:
			*opt2 |= 0x40;
			opt_flag = 1;
			break;
		case SC_HIDING:
		case SC_CLOAKING:
			battle_stopattack(bl);	/* U?’â~ */
			*option |= ((type==SC_HIDING)?2:4);
			opt_flag =1 ;
			break;
		case SC_CHASEWALK:
			battle_stopattack(bl);	/* U?’â~ */
			*option |= 16388;
			opt_flag =1 ;
			break;
		case SC_SIGHT:
			*option |= 1;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			*option |= 8192;
			opt_flag = 1;
			break;
		case SC_WEDDING:
			*option |= 4096;
			opt_flag = 1;
	}

	if(opt_flag)	/* option‚Ì?X */
		clif_changeoption(bl);

	(*sc_count)++;	/* ƒXƒe?ƒ^ƒXˆÙí‚Ì? */

	sc_data[type].val1 = val1;
	sc_data[type].val2 = val2;
	sc_data[type].val3 = val3;
	sc_data[type].val4 = val4;
	/* ƒ^ƒCƒ}?İ’è */
	sc_data[type].timer = add_timer(
		gettick() + tick, status_change_timer, bl->id, type);

	if(bl->type==BL_PC && calc_flag)
		status_calc_pc(sd,0);	/* ƒXƒe?ƒ^ƒXÄŒvZ */

	if(bl->type==BL_PC && save_flag)
		chrif_save(sd); // save the player status

	if(bl->type==BL_PC && updateflag)
		clif_updatestatus(sd,updateflag);	/* ƒXƒe?ƒ^ƒX‚ğƒNƒ‰ƒCƒAƒ“ƒg‚É‘—‚é */

	return 0;
}
/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙí‘S‰ğœ
 *------------------------------------------
 */
int status_change_clear(struct block_list *bl,int type)
{
	struct status_change* sc_data;
	short *sc_count, *option, *opt1, *opt2, *opt3;
	int i;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc_data = status_get_sc_data(bl));
	nullpo_retr(0, sc_count = status_get_sc_count(bl));
	nullpo_retr(0, option = status_get_option(bl));
	nullpo_retr(0, opt1 = status_get_opt1(bl));
	nullpo_retr(0, opt2 = status_get_opt2(bl));
	nullpo_retr(0, opt3 = status_get_opt3(bl));

	if (*sc_count == 0)
		return 0;
	for(i = 0; i < MAX_STATUSCHANGE; i++){
		if(sc_data[i].timer != -1){	/* ˆÙí‚ª‚ ‚é‚È‚çƒ^ƒCƒ}?‚ğíœ‚·‚é */
			status_change_end(bl, i, -1);
		}
	}
	*sc_count = 0;
	*opt1 = 0;
	*opt2 = 0;
	*opt3 = 0;
	*option &= OPTION_MASK;

	if (night_flag == 1 && type == BL_PC && !map[bl->m].flag.indoors && // by [Yor]
		!map[bl->m].flag.indoors && battle_config.night_darkness_level <= 0) // [celest]
		*opt2 |= STATE_BLIND;

	if(!type || type&2)
		clif_changeoption(bl);

	return 0;
}

/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíI—¹
 *------------------------------------------
 */
int status_change_end( struct block_list* bl , int type,int tid )
{
	struct status_change* sc_data;
	int opt_flag=0, calc_flag = 0;
	short *sc_count, *option, *opt1, *opt2, *opt3;
	int type2 = type;

	nullpo_retr(0, bl);
	if(bl->type!=BL_PC && bl->type!=BL_MOB) {
		if(battle_config.error_log)
			printf("status_change_end: neither MOB nor PC !\n");
		return 0;
	}
	nullpo_retr(0, sc_data = status_get_sc_data(bl));
	nullpo_retr(0, sc_count = status_get_sc_count(bl));
	nullpo_retr(0, option = status_get_option(bl));
	nullpo_retr(0, opt1 = status_get_opt1(bl));
	nullpo_retr(0, opt2 = status_get_opt2(bl));
	nullpo_retr(0, opt3 = status_get_opt3(bl));

	if ((*sc_count) > 0 && sc_data[type].timer != -1 && (sc_data[type].timer == tid || tid == -1)) {

		if (tid == -1)	// ƒ^ƒCƒ}‚©‚çŒÄ‚Î‚ê‚Ä‚¢‚È‚¢‚È‚çƒ^ƒCƒ}íœ‚ğ‚·‚é
			delete_timer(sc_data[type].timer,status_change_timer);

		/* ŠY?‚ÌˆÙí‚ğ³í‚É?‚· */
		sc_data[type].timer=-1;
		(*sc_count)--;

		switch(type){	/* ˆÙí‚Ìí—Ş‚²‚Æ‚Ì?— */
			case SC_PROVOKE:			/* ƒvƒƒ{ƒbƒN */
			case SC_ENDURE: // celest
			case SC_CONCENTRATE:		/* W’†—ÍŒüã */
			case SC_BLESSING:			/* ƒuƒŒƒbƒVƒ“ƒO */
			case SC_ANGELUS:			/* ƒAƒ“ƒ[ƒ‹ƒX */
			case SC_INCREASEAGI:		/* ‘¬“xã¸ */
			case SC_DECREASEAGI:		/* ‘¬“xŒ¸­ */
			case SC_SIGNUMCRUCIS:		/* ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX */
			case SC_HIDING:
			case SC_TWOHANDQUICKEN:		/* 2HQ */
			case SC_ADRENALINE:			/* ƒAƒhƒŒƒiƒŠƒ“ƒ‰ƒbƒVƒ… */
			case SC_ENCPOISON:			/* ƒGƒ“ƒ`ƒƒƒ“ƒgƒ|ƒCƒYƒ“ */
			case SC_IMPOSITIO:			/* ƒCƒ“ƒ|ƒVƒeƒBƒIƒ}ƒkƒX */
			case SC_GLORIA:				/* ƒOƒƒŠƒA */
			case SC_LOUD:				/* ƒ‰ƒEƒhƒ{ƒCƒX */
			case SC_QUAGMIRE:			/* ƒNƒ@ƒOƒ}ƒCƒA */
			case SC_PROVIDENCE:			/* ƒvƒƒ”ƒBƒfƒ“ƒX */
			case SC_SPEARSQUICKEN:		/* ƒXƒsƒAƒNƒCƒbƒPƒ“ */
			case SC_VOLCANO:
			case SC_DELUGE:
			case SC_VIOLENTGALE:
			case SC_ETERNALCHAOS:		/* ƒGƒ^?ƒiƒ‹ƒJƒIƒX */
			case SC_DRUMBATTLE:			/* ?‘¾ŒÛ‚Ì‹¿‚« */
			case SC_NIBELUNGEN:			/* ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö */
			case SC_SIEGFRIED:			/* •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh */
			case SC_WHISTLE:			/* Œû“J */
			case SC_ASSNCROS:			/* —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX */
			case SC_HUMMING:			/* ƒnƒ~ƒ“ƒO */
			case SC_DONTFORGETME:		/* „‚ğ–Y‚ê‚È‚¢‚Å */
			case SC_FORTUNE:			/* K‰^‚ÌƒLƒX */
			case SC_SERVICE4U:			/* ƒT?ƒrƒXƒtƒH?ƒ†? */
			case SC_EXPLOSIONSPIRITS:	// ”š—ô”g“®
			case SC_STEELBODY:			// ‹à„
			case SC_DEFENDER:
			case SC_SPEEDPOTION0:		/* ?‘¬ƒ|?ƒVƒ‡ƒ“ */
			case SC_SPEEDPOTION1:
			case SC_SPEEDPOTION2:
			case SC_SPEEDPOTION3:
			case SC_APPLEIDUN:			/* ƒCƒhƒDƒ“‚Ì—ÑŒç */
			case SC_RIDING:
			case SC_BLADESTOP_WAIT:
			case SC_CONCENTRATION:		/* ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“ */
			case SC_ASSUMPTIO:			/* ƒAƒVƒƒƒ“ƒvƒeƒBƒI */
			case SC_WINDWALK:		/* ƒEƒCƒ“ƒhƒEƒH?ƒN */
			case SC_TRUESIGHT:		/* ƒgƒDƒ‹?ƒTƒCƒg */
			case SC_SPIDERWEB:		/* ƒXƒpƒCƒ_?ƒEƒFƒbƒu */
			case SC_MAGICPOWER:		/* –‚–@—Í?• */
			case SC_CHASEWALK:
			case SC_ATKPOT:		/* attack potion [Valaris] */
			case SC_MATKPOT:		/* magic attack potion [Valaris] */
			case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
			case SC_MELTDOWN:		/* ƒƒ‹ƒgƒ_ƒEƒ“ */
			case SC_MINDBREAKER:		/* ƒ}ƒCƒ“ƒhƒuƒŒ[ƒJ[ */
			// Celest
			case SC_EDP:
			case SC_SLOWDOWN:
			case SC_SPEEDUP0:
			case SC_BATTLEORDERS:
			case SC_REGENERATION:
			case SC_GUILDAURA:
				calc_flag = 1;
				break;
			case SC_AUTOBERSERK:
				if (sc_data[SC_PROVOKE].timer != -1)
					status_change_end(bl,SC_PROVOKE,-1);
				break;
			case SC_BERSERK:			/* ƒo?ƒT?ƒN */
				calc_flag = 1;
				clif_status_change(bl,SC_INCREASEAGI,0);	/* ƒAƒCƒRƒ“Á‹ */
				break;
			case SC_DEVOTION:		/* ƒfƒBƒ{?ƒVƒ‡ƒ“ */
				{
					struct map_session_data *md = map_id2sd(sc_data[type].val1);
					sc_data[type].val1=sc_data[type].val2=0;
					skill_devotion(md,bl->id);
					calc_flag = 1;
				}
				break;
			case SC_BLADESTOP:
				{
					struct status_change *t_sc_data = status_get_sc_data((struct block_list *)sc_data[type].val4);
					//•Ğ•û‚ªØ‚ê‚½‚Ì‚Å‘Šè‚Ì”’n?‘Ô‚ªØ‚ê‚Ä‚È‚¢‚Ì‚È‚ç‰ğœ
					if(t_sc_data && t_sc_data[SC_BLADESTOP].timer!=-1)
						status_change_end((struct block_list *)sc_data[type].val4,SC_BLADESTOP,-1);

					if(sc_data[type].val2==2)
						clif_bladestop((struct block_list *)sc_data[type].val3,(struct block_list *)sc_data[type].val4,0);
				}
				break;
			case SC_DANCING:
				{
					struct map_session_data *dsd;
					struct status_change *d_sc_data;
					if(sc_data[type].val4 && (dsd=map_id2sd(sc_data[type].val4))){
						d_sc_data = dsd->sc_data;
						//‡‘t‚Å‘Šè‚ª‚¢‚éê‡‘Šè‚Ìval4‚ğ0‚É‚·‚é
						if(d_sc_data && d_sc_data[type].timer!=-1)
							d_sc_data[type].val4=0;
					}
				}
				calc_flag = 1;
				break;
			case SC_NOCHAT:	//ƒ`ƒƒƒbƒg‹Ö~?‘Ô
				{
					struct map_session_data *sd=NULL;
					if(bl->type == BL_PC && (sd=(struct map_session_data *)bl)){
						if (sd->status.manner >= 0) // weeee ^^ [celest]
							sd->status.manner = 0;
						clif_updatestatus(sd,SP_MANNER);
					}
				}
				break;
			case SC_SPLASHER:		/* ƒxƒiƒ€ƒXƒvƒ‰ƒbƒVƒƒ? */
				{
					struct block_list *src=map_id2bl(sc_data[type].val3);
					if(src && tid!=-1){
						//©•ª‚Éƒ_ƒ?ƒW•ü?3*3‚Éƒ_ƒ?ƒW
						skill_castend_damage_id(src, bl,sc_data[type].val2,sc_data[type].val1,gettick(),0 );
					}
				}
				break;
			case SC_SELFDESTRUCTION:		/* ©”š */
				{
					//©•ª‚Ìƒ_ƒ?ƒW‚Í0‚É‚µ‚Ä
					struct mob_data *md=NULL;
					if(bl->type == BL_MOB && (md=(struct mob_data*)bl))
						skill_castend_damage_id(bl, bl,sc_data[type].val2,sc_data[type].val1,gettick(),0 );
				}
				break;
		/* option1 */
			case SC_FREEZE:
				sc_data[type].val3 = 0;
				break;

		/* option2 */
			case SC_POISON:				/* “Å */
			case SC_BLIND:				/* ˆÃ? */
			case SC_CURSE:
				calc_flag = 1;
				break;

			// celest
			case SC_CONFUSION:
				{
					struct map_session_data *sd=NULL;
					if(bl->type == BL_PC && (sd=(struct map_session_data *)bl)){
						sd->next_walktime = -1;
					}
				}
				break;

			case SC_MARIONETTE:		/* ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹ */
			case SC_MARIONETTE2:	/// Marionette target
				{
					// check for partner and end their marionette status as well
					int type2 = (type == SC_MARIONETTE) ? SC_MARIONETTE2 : SC_MARIONETTE;
					struct block_list *pbl = map_id2bl(sc_data[type].val3);
					if (pbl) {
						struct status_change* sc_data;
						if (*status_get_sc_count(pbl) > 0 &&
							(sc_data = status_get_sc_data(pbl)) &&
							sc_data[type2].timer != -1)
							status_change_end(pbl, type2, -1);
					}
					calc_flag = 1;
				}
				break;

			case SC_BABY:
				type2 = _SC_BABY;
				break;
			}

		if(bl->type==BL_PC &&
			(type<SC_SENDMAX || type==SC_PRESERVE || type==SC_BATTLEORDERS || type==SC_BABY))
			clif_status_change(bl,type2,0);	/* ƒAƒCƒRƒ“Á‹ */

		switch(type){	/* ³í‚É?‚é‚Æ‚«‚È‚É‚©?—‚ª•K—v */
		case SC_STONE:
		case SC_FREEZE:
		case SC_STAN:
		case SC_SLEEP:
			*opt1 = 0;
			opt_flag = 1;
			break;

		case SC_POISON:
			if (sc_data[SC_DPOISON].timer != -1)	//
				break;						// DPOISON—p‚ÌƒIƒvƒVƒ‡ƒ“
			*opt2 &= ~1;					// ‚ª?—p‚É—pˆÓ‚³‚ê‚½ê‡‚É‚Í
			opt_flag = 1;					// ‚±‚±‚Ííœ‚·‚é
			break;							//
		case SC_CURSE:
		case SC_SILENCE:
		case SC_BLIND:
			*opt2 &= ~(1<<(type-SC_POISON));
			opt_flag = 1;
			break;
		case SC_DPOISON:
			if (sc_data[SC_POISON].timer != -1)	// DPOISON—p‚ÌƒIƒvƒVƒ‡ƒ“‚ª
				break;							// —pˆÓ‚³‚ê‚½‚çíœ
			*opt2 &= ~1;	// “Å?‘Ô‰ğœ
			opt_flag = 1;
			break;
		case SC_SIGNUMCRUCIS:
			*opt2 &= ~0x40;
			opt_flag = 1;
			break;

		case SC_HIDING:
		case SC_CLOAKING:
			*option &= ~((type == SC_HIDING) ? 2 : 4);
			calc_flag = 1;	// orn
			opt_flag = 1 ;
			break;

		case SC_CHASEWALK:
			*option &= ~16388;
			opt_flag = 1 ;
			break;

		case SC_SIGHT:
			*option &= ~1;
			opt_flag = 1;
			break;
		case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
			*option &= ~4096;
			opt_flag = 1;
			break;
		case SC_RUWACH:
			*option &= ~8192;
			opt_flag = 1;
			break;

		//opt3
		case SC_TWOHANDQUICKEN:		/* 2HQ */
		case SC_SPEARSQUICKEN:		/* ƒXƒsƒAƒNƒCƒbƒPƒ“ */
		case SC_CONCENTRATION:		/* ƒRƒ“ƒZƒ“ƒgƒŒ?ƒVƒ‡ƒ“ */
			*opt3 &= ~1;
			break;
		case SC_OVERTHRUST:			/* ƒI?ƒo?ƒXƒ‰ƒXƒg */
			*opt3 &= ~2;
			break;
		case SC_ENERGYCOAT:			/* ƒGƒiƒW?ƒR?ƒg */
			*opt3 &= ~4;
			break;
		case SC_EXPLOSIONSPIRITS:	// ”š—ô”g“®
			*opt3 &= ~8;
			break;
		case SC_STEELBODY:			// ‹à„
			*opt3 &= ~16;
			break;
		case SC_BLADESTOP:		/* ”’næ‚è */
			*opt3 &= ~32;
			break;
		case SC_BERSERK:		/* ƒo?ƒT?ƒN */
			*opt3 &= ~128;
			break;
		case SC_MARIONETTE:		/* ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹ */
		case SC_MARIONETTE2:
			*opt3 &= ~1024;
			break;
		case SC_ASSUMPTIO:		/* ƒAƒXƒ€ƒvƒeƒBƒI */
			*opt3 &= ~2048;
			break;
		}

		if (night_flag == 1 && (*opt2 & STATE_BLIND) == 0 && bl->type == BL_PC && // by [Yor]
			!map[bl->m].flag.indoors && battle_config.night_darkness_level <= 0) { // [celest]
			*opt2 |= STATE_BLIND;
			opt_flag = 1;
		}

		if(opt_flag)	/* option‚Ì?X‚ğ?‚¦‚é */
			clif_changeoption(bl);

		if (bl->type == BL_PC && calc_flag)
			status_calc_pc((struct map_session_data *)bl,0);	/* ƒXƒe?ƒ^ƒXÄŒvZ */
	}

	return 0;
}


/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíI—¹ƒ^ƒCƒ}[
 *------------------------------------------
 */
int status_change_timer(int tid, unsigned int tick, int id, int data)
{
	int type = data;
	struct block_list *bl;
	struct map_session_data *sd=NULL;
	struct status_change *sc_data;
	//short *sc_count; //g‚Á‚Ä‚È‚¢H

// security system to prevent forgetting timer removal
	int temp_timerid;

        bl=map_id2bl(id);
#ifndef _WIN32
	nullpo_retr_f(0, bl, "id=%d data=%d",id,data);
#endif
	nullpo_retr(0, sc_data=status_get_sc_data(bl));

	if(bl->type==BL_PC)
		nullpo_retr(0, sd=(struct map_session_data *)bl);

	//sc_count=status_get_sc_count(bl); //g‚Á‚Ä‚È‚¢H

	if(sc_data[type].timer != tid) {
		if(battle_config.error_log)
			printf("status_change_timer %d != %d\n",tid,sc_data[type].timer);
		return 0;
	}

	// security system to prevent forgetting timer removal
	// you shouldn't be that careless inside the switch here
	temp_timerid = sc_data[type].timer;
	sc_data[type].timer = -1;

	switch(type){	/* “Áê‚È?—‚É‚È‚éê‡ */
	case SC_MAXIMIZEPOWER:	/* ƒ}ƒLƒVƒ}ƒCƒYƒpƒ? */
	case SC_CLOAKING:
		if(sd){
			if( sd->status.sp > 0 ){ /* SPØ‚ê‚é‚Ü‚Å? */
				sd->status.sp--;
				clif_updatestatus(sd,SP_SP);
				sc_data[type].timer=add_timer( /* ƒ^ƒCƒ}?Äİ’è */
				sc_data[type].val2+tick, status_change_timer, bl->id, data);
				return 0;
			}
		}
		break;

	case SC_CHASEWALK:
		if(sd){
			int sp = 10+sc_data[SC_CHASEWALK].val1*2;
			if (map[sd->bl.m].flag.gvg) sp *= 5;
			if( sd->status.sp > sp){
				sd->status.sp -= sp; // update sp cost [Celest]
				clif_updatestatus(sd,SP_SP);
				sc_data[type].timer=add_timer( /* ƒ^ƒCƒ}?Äİ’è */
					sc_data[type].val2+tick, status_change_timer, bl->id, data);
				sc_data[SC_CHASEWALK].val4++;
				if (sc_data[SC_CHASEWALK].val4 > 3)
					sc_data[SC_CHASEWALK].val4 = 0;
				status_calc_pc (sd, 0);
				return 0;
			}
		}
	break;

	case SC_HIDING:		/* ƒnƒCƒfƒBƒ“ƒO */
		if(sd){		/* SP‚ª‚ ‚Á‚ÄAŠÔ§ŒÀ‚ÌŠÔ‚Í? */
			if( sd->status.sp > 0 && (--sc_data[type].val2)>0 ){
				if(sc_data[type].val2 % (sc_data[type].val1+3) ==0 ){
					sd->status.sp--;
					clif_updatestatus(sd,SP_SP);
				}
				sc_data[type].timer=add_timer(	/* ƒ^ƒCƒ}?Äİ’è */
					1000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
	break;

	case SC_SIGHT:	/* ƒTƒCƒg */
	case SC_RUWACH:	/* ƒ‹ƒAƒt */
		{
			int range = 5;
			if ( type == SC_SIGHT ) range = 7;
			map_foreachinarea( status_change_timer_sub,
				bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,0,
				bl,type,tick);

			if( (--sc_data[type].val2)>0 ){
				sc_data[type].timer=add_timer(	/* ƒ^ƒCƒ}?Äİ’è */
					250+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;

	case SC_SIGNUMCRUCIS:		/* ƒVƒOƒiƒ€ƒNƒ‹ƒVƒX */
		{
			int race = status_get_race(bl);
			if(race == 6 || battle_check_undead(race,status_get_elem_type(bl))) {
				sc_data[type].timer=add_timer(1000*600+tick,status_change_timer, bl->id, data );
				return 0;
			}
		}
		break;

	case SC_PROVOKE:	/* ƒvƒƒ{ƒbƒN/ƒI?ƒgƒo?ƒT?ƒN */
		if(sc_data[type].val2!=0){	/* ƒI?ƒgƒo?ƒT?ƒNi‚P•b‚²‚Æ‚ÉHPƒ`ƒFƒbƒNj */
			if(sd && sd->status.hp>sd->status.max_hp>>2)	/* ’â~ */
				break;
			sc_data[type].timer=add_timer( 1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_ENDURE:	/* ƒCƒ“ƒfƒ…ƒA */
	case SC_AUTOBERSERK: // Celest
		if(sd && sd->special_state.infinite_endure) {
#ifdef TWILIGHT
			sc_data[type].timer=add_timer( 1000*600+tick,status_change_timer, bl->id, data );
#else
			sc_data[type].timer=add_timer( 1000*60+tick,status_change_timer, bl->id, data );
#endif
			//sc_data[type].val2=1;
			return 0;
		}
		break;

	case SC_DISSONANCE:	/* •s‹¦˜a‰¹ */
		if( (--sc_data[type].val2)>0){
			struct skill_unit *unit=
				(struct skill_unit *)sc_data[type].val4;
			struct block_list *src;
			/*if(!unit || !unit->group)
				break;
			src=map_id2bl(unit->group->src_id);
			if(!src)
				break;*/
			nullpo_retb(unit);
			nullpo_retb(unit->group);
			nullpo_retb(src=map_id2bl(unit->group->src_id));
			skill_attack(BF_MISC,src,&unit->bl,bl,unit->group->skill_id,sc_data[type].val1,tick,0);
			if( (bl->type==BL_MOB) && (MS_DEAD==((struct mob_data *)bl)->state.state) )
				break;
			sc_data[type].timer=add_timer(skill_get_time2(unit->group->skill_id,unit->group->skill_lv)+tick,
				status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_LULLABY:	/* qç‰S */
		if( (--sc_data[type].val2)>0){
			struct skill_unit *unit=
				(struct skill_unit *)sc_data[type].val4;
			nullpo_retb(unit);
			nullpo_retb(unit->group);
			if(unit->group->src_id == bl->id)
				break;
			skill_additional_effect(bl,bl,unit->group->skill_id,sc_data[type].val1,BF_LONG|BF_SKILL|BF_MISC,tick);
			if (unit->group != 0)
			{
				sc_data[type].timer=add_timer(skill_get_time(unit->group->skill_id,unit->group->skill_lv)/10+tick,
					status_change_timer, bl->id, data );
				return 0;
			}// dont forget the brackets
		}
		break;

	case SC_STONE:
		if(sc_data[type].val2 != 0) {
			short *opt1 = status_get_opt1(bl);
			sc_data[type].val2 = 0;
			sc_data[type].val4 = 0;
			battle_stopwalking(bl,1);
			if(opt1) {
				*opt1 = 1;
				clif_changeoption(bl);
			}
			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		else if( (--sc_data[type].val3) > 0) {
			int hp = status_get_max_hp(bl);
			if((++sc_data[type].val4)%5 == 0 && status_get_hp(bl) > hp>>2) {
				hp = hp/100;
				if(hp < 1) hp = 1;
				if(sd)
					pc_heal(sd,-hp,0);
				else if(bl->type == BL_MOB){
					struct mob_data *md;
					if((md=((struct mob_data *)bl)) == NULL)
						break;
					md->hp -= hp;
				}
			}
			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			return 0;
		}
		break;

	case SC_POISON:
	case SC_DPOISON:
		if (sc_data[SC_SLOWPOISON].timer == -1 && (--sc_data[type].val3) > 0) {
			int hp = status_get_max_hp(bl);
			if (type == SC_POISON && status_get_hp(bl) < hp>>2)
				break;
			if(sd) {
				hp = (type == SC_DPOISON) ? 3 + hp/50 : 3 + hp*3/200;
				pc_heal(sd, -hp, 0);
			} else if (bl->type == BL_MOB) {
				struct mob_data *md;
				nullpo_retr(0, md=(struct mob_data *)bl);
				hp = (type == SC_DPOISON) ? 3 + hp/100 : 3 + hp/200;
				md->hp -= hp;
			}
		}
		if (sc_data[type].val3 > 0)
		{
			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			// hmm setting up a timer and breaking then to call status_change_end just right away?
			// I think you're missing brackets and a:
			return 0;
		}
		break;

	case SC_TENSIONRELAX:	/* ƒeƒ“ƒVƒ‡ƒ“ƒŠƒ‰ƒbƒNƒX */
		if(sd){		/* SP‚ª‚ ‚Á‚ÄAHP‚ª?ƒ^ƒ“‚Å‚È‚¯‚ê‚Î?? */
			if( sd->status.sp > 12 && sd->status.max_hp > sd->status.hp ){
/*				if(sc_data[type].val2 % (sc_data[type].val1+3) ==0 ){
					sd->status.sp -= 12;
					clif_updatestatus(sd,SP_SP);
				}						*/
				sc_data[type].timer=add_timer(	/* ƒ^ƒCƒ}?Äİ’è */
					10000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
			if(sd->status.max_hp <= sd->status.hp)
			{
				status_change_end(&sd->bl,SC_TENSIONRELAX,-1);
				// calling status_change_end then break and call it again might not be that what is necessary
				// or am I wrong?
				// so I just add brackets and a:
				return 0;
			}
		}
		break;
	case SC_BLEEDING:	// [celest]
		// i hope i haven't interpreted it wrong.. which i might ^^;
		// Source:
		// - 10õ©ª´ªÈªËHPª¬Êõá´
		// - õóúìªÎªŞªŞ«µ?«Ğì¹ÔÑªä«ê«í«°ª·ªÆªâ?ÍıªÏá¼ª¨ªÊª¤
		if((sc_data[type].val3 -= 1000) > 0) {
			if((sc_data[type].val4 -= 1000) > 0) {
				int hp = rand()%300+400;
				if(sd) {
					pc_heal(sd,-hp,0);
					sd->canmove_tick = tick+1000;
				}
				else if(bl->type == BL_MOB) {
					struct mob_data *md;
					nullpo_retr(0, md=(struct mob_data *)bl);
					md->hp -= hp;
				}
			}
			if (sd) {				
				sd->canact_tick = tick+1000;
			}

			sc_data[type].timer=add_timer(1000+tick,status_change_timer, bl->id, data );
			// hmm setting up a timer and breaking then to call status_change_end just right away?
			// I think you're missing a:
			return 0;
		}
		break;

	/* ŠÔØ‚ê–³‚µHH */
	case SC_AETERNA:
	case SC_TRICKDEAD:
	case SC_RIDING:
	case SC_FALCON:
	case SC_WEIGHT50:
	case SC_WEIGHT90:
	case SC_MAGICPOWER:		/* –‚–@—Í?• */
	case SC_REJECTSWORD:	/* ƒŠƒWƒFƒNƒgƒ\?ƒh */
	case SC_MEMORIZE:	/* ƒƒ‚ƒ‰ƒCƒY */
	case SC_BROKNWEAPON:
	case SC_BROKNARMOR:
	case SC_SACRIFICE:
		sc_data[type].timer=add_timer( 1000*600+tick,status_change_timer, bl->id, data );
		return 0;

	case SC_DANCING: //ƒ_ƒ“ƒXƒXƒLƒ‹‚ÌŠÔSPÁ”ï
		{
			int s=0;
			if(sd){
				if(sd->status.sp > 0 && (--sc_data[type].val3)>0){
					switch(sc_data[type].val1){
					case BD_RICHMANKIM:				/* ƒjƒˆƒ‹ƒh‚Ì‰ƒ 3•b‚ÉSP1 */
					case BD_DRUMBATTLEFIELD:		/* ?‘¾ŒÛ‚Ì‹¿‚« 3•b‚ÉSP1 */
					case BD_RINGNIBELUNGEN:			/* ƒj?ƒxƒ‹ƒ“ƒO‚Ìw—Ö 3•b‚ÉSP1 */
					case BD_SIEGFRIED:				/* •s€g‚ÌƒW?ƒNƒtƒŠ?ƒh 3•b‚ÉSP1 */
					case BA_DISSONANCE:				/* •s‹¦˜a‰¹ 3•b‚ÅSP1 */
					case BA_ASSASSINCROSS:			/* —[—z‚ÌƒAƒTƒVƒ“ƒNƒƒX 3•b‚ÅSP1 */
					case DC_UGLYDANCE:				/* ©•ªŸè‚Èƒ_ƒ“ƒX 3•b‚ÅSP1 */
						s=3;
						break;
					case BD_LULLABY:				/* qç‰Ì 4•b‚ÉSP1 */
					case BD_ETERNALCHAOS:			/* ‰i‰“‚Ì¬“× 4•b‚ÉSP1 */
					case BD_ROKISWEIL:				/* ƒƒL‚Ì‹©‚Ñ 4•b‚ÉSP1 */
					case DC_FORTUNEKISS:			/* K‰^‚ÌƒLƒX 4•b‚ÅSP1 */
						s=4;
						break;
					case BD_INTOABYSS:				/* [•£‚Ì’†‚É 5•b‚ÉSP1 */
					case BA_WHISTLE:				/* Œû“J 5•b‚ÅSP1 */
					case DC_HUMMING:				/* ƒnƒ~ƒ“ƒO 5•b‚ÅSP1 */
					case BA_POEMBRAGI:				/* ƒuƒ‰ƒM‚Ì 5•b‚ÅSP1 */
					case DC_SERVICEFORYOU:			/* ƒT?ƒrƒXƒtƒH?ƒ†? 5•b‚ÅSP1 */
						s=5;
						break;
					case BA_APPLEIDUN:				/* ƒCƒhƒDƒ“‚Ì—ÑŒç 6•b‚ÅSP1 */
						s=6;
						break;
					case DC_DONTFORGETME:			/* „‚ğ–Y‚ê‚È‚¢‚Åc 10•b‚ÅSP1 */
					case CG_MOONLIT:				/* Œ–¾‚è‚Ìò‚É—‚¿‚é‰Ô‚Ñ‚ç 10•b‚ÅSP1H */
						s=10;
						break;
					}
					if(s && ((sc_data[type].val3 % s) == 0)){
						sd->status.sp--;
						clif_updatestatus(sd,SP_SP);
					}
					sc_data[type].timer=add_timer(	/* ƒ^ƒCƒ}?Äİ’è */
						1000+tick, status_change_timer,
						bl->id, data);
					return 0;
				}
			}
		}
		break;
	case SC_BERSERK:		/* ƒo?ƒT?ƒN */
		if(sd){		/* HP‚ª100ˆÈã‚È‚ç?? */
			if( (sd->status.hp - sd->status.max_hp*5/100) > 100 ){	// 5% every 10 seconds [DracoRPG]
				sd->status.hp -= sd->status.max_hp*5/100;	// changed to max hp [celest]
				clif_updatestatus(sd,SP_HP);
				sc_data[type].timer = add_timer(	/* ƒ^ƒCƒ}?Äİ’è */
					10000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;
	case SC_WEDDING:	//Œ‹¥—p(Œ‹¥ˆßÖ‚É‚È‚Á‚Ä?‚­‚Ì‚ª?‚¢‚Æ‚©)
		if(sd){
			time_t timer;
			if(time(&timer) < ((sc_data[type].val2) + 3600)){	//1ŠÔ‚½‚Á‚Ä‚¢‚È‚¢‚Ì‚Å??
				sc_data[type].timer=add_timer(	/* ƒ^ƒCƒ}?Äİ’è */
					10000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;
	case SC_NOCHAT:	//ƒ`ƒƒƒbƒg‹Ö~?‘Ô
		if(sd && battle_config.muting_players){
			time_t timer;
			if((++sd->status.manner) && time(&timer) < ((sc_data[type].val2) + 60*(0-sd->status.manner))){	//ŠJn‚©‚çstatus.manner•ª?‚Á‚Ä‚È‚¢‚Ì‚Å??
				clif_updatestatus(sd,SP_MANNER);
				sc_data[type].timer=add_timer(	/* ƒ^ƒCƒ}?Äİ’è(60•b) */
					60000+tick, status_change_timer,
					bl->id, data);
				return 0;
			}
		}
		break;
	case SC_SELFDESTRUCTION:		/* ©”š */
		if(--sc_data[type].val3>0){
			struct mob_data *md;
			if(bl->type==BL_MOB && (md=(struct mob_data *)bl) && md->speed > 250){
				md->speed -= 250;
				md->next_walktime=tick;
			}
			sc_data[type].timer=add_timer(	/* ƒ^ƒCƒ}?Äİ’è */
				1000+tick, status_change_timer,
				bl->id, data);
				return 0;
		}
		break;

	case SC_SPLASHER:
		if (sc_data[type].val4 % 1000 == 0) {
			char timer[2];
			sprintf (timer, "%d", sc_data[type].val4/1000);
			clif_message(bl, timer);
		}
		if((sc_data[type].val4 -= 500) > 0) {
			sc_data[type].timer = add_timer(
				500 + tick, status_change_timer,
				bl->id, data);
				return 0;
		}
		break;

	case SC_MARIONETTE:		/* ƒ}ƒŠƒIƒlƒbƒgƒRƒ“ƒgƒ?ƒ‹ */
	case SC_MARIONETTE2:
		{
			struct block_list *pbl = map_id2bl(sc_data[type].val3);
			if (pbl && battle_check_range(bl, pbl, 7) &&
				(sc_data[type].val2 -= 1000)>0) {
				sc_data[type].timer = add_timer(
					1000 + tick, status_change_timer,
					bl->id, data);
					return 0;
			}
		}
		break;

	// Celest
	case SC_CONFUSION:
		{
			int i = 3000;
			//struct mob_data *md;
			if (sd) {
				pc_randomwalk (sd, gettick());
				sd->next_walktime = tick + (i=1000 + rand()%1000);
			} /*else if (bl->type==BL_MOB && (md=(struct mob_data *)bl) && md->mode&1 && mob_can_move(md)) {
				md->state.state=MS_WALK;
				if( DIFF_TICK(md->next_walktime,tick) > + 7000 &&
					(md->walkpath.path_len==0 || md->walkpath.path_pos>=md->walkpath.path_len) )
					md->next_walktime = tick + 3000*rand()%2000;
				mob_randomwalk(md,tick);
			}*/
			if ((sc_data[type].val2 -= 1000) > 0) {
				sc_data[type].timer = add_timer(
					i + tick, status_change_timer,
					bl->id, data);
					return 0;
			}
		}
		break;

	case SC_GOSPEL:
		{
			int calc_flag = 0;
			if (sc_data[type].val3 > 0) {
				sc_data[type].val3 = 0;
				calc_flag = 1;
			}
			if(sd && sc_data[type].val4 == BCT_SELF){
				int hp, sp;
				hp = (sc_data[type].val1 > 5) ? 45 : 30;
				sp = (sc_data[type].val1 > 5) ? 35 : 20;
				if(sd->status.hp - hp > 0 &&
					sd->status.sp - sp > 0){
					sd->status.hp -= hp;
					sd->status.sp -= sp;
					clif_updatestatus(sd,SP_HP);
					clif_updatestatus(sd,SP_SP);
					if ((sc_data[type].val2 -= 10000) > 0) {
						sc_data[type].timer = add_timer(
							10000+tick, status_change_timer,
							bl->id, data);
						return 0;
					}
				}
			} else if (sd && sc_data[type].val4 == BCT_PARTY) {
				int i;
				switch ((i = rand() % 12)) {
				case 1: // heal between 100-1000
					{
						struct block_list tbl;
						int heal = rand() % 900 + 100;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(&tbl,bl,AL_HEAL,heal,1);
						battle_heal(NULL,bl,heal,0,0);
					}
					break;
				case 2: // end negative status
					{
						int j;
						for (j=0; j<4; j++)
							if(sc_data[i + SC_POISON].timer!=-1) {
								status_change_end(bl,j,-1);
								break;
							}
					}
					break;
				case 3:	// +25% resistance to negative status
				case 4: // +25% max hp
				case 5: // +25% max sp
				case 6: // +2 to all stats
				case 11: // +25% armor and vit def
				case 12: // +8% atk
				case 13: // +5% flee
				case 14: // +5% hit
					sc_data[type].val3 = i;
					if (i == 6 ||
						(i >= 11 && i <= 14))
						calc_flag = 1;
					break;
				case 7: // level 5 bless
					{
						struct block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(&tbl,bl,AL_BLESSING,5,1);
						status_change_start(bl,SkillStatusChangeTable[AL_BLESSING],5,0,0,0,10000,0 );
					}
					break;
				case 8: // level 5 increase agility
					{
						struct block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(&tbl,bl,AL_INCAGI,5,1);
						status_change_start(bl,SkillStatusChangeTable[AL_INCAGI],5,0,0,0,10000,0 );
					}
					break;
				case 9: // holy element to weapon
					{
						struct block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(&tbl,bl,PR_ASPERSIO,1,1);
						status_change_start(bl,SkillStatusChangeTable[PR_ASPERSIO],1,0,0,0,10000,0 );
					}
					break;
				case 10: // holy element to armour
					{
						struct block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(&tbl,bl,PR_BENEDICTIO,1,1);
						status_change_start(bl,SkillStatusChangeTable[PR_BENEDICTIO],1,0,0,0,10000,0 );
					}
					break;
				default:
					break;
				}
			} else if (sc_data[type].val4 == BCT_ENEMY) {
				int i;
				switch ((i = rand() % 8)) {
				case 1: // damage between 300-800
				case 2: // damage between 150-550 (ignore def)
					battle_damage(NULL, bl, rand() % 500,0); // temporary damage
					break;
				case 3: // random status effect
					{
						int effect[3] = {
							SC_CURSE,
							SC_BLIND,
							SC_POISON };
						status_change_start(bl,effect[rand()%3],1,0,0,0,10000,0 );
					}
					break;
				case 4: // level 10 provoke
					{
						struct block_list tbl;
						tbl.id = 0;
						tbl.m = bl->m;
						tbl.x = bl->x;
						tbl.y = bl->y;
						clif_skill_nodamage(&tbl,bl,SM_PROVOKE,1,1);
						status_change_start(bl,SkillStatusChangeTable[SM_PROVOKE],10,0,0,0,10000,0 );
					}
					break;
				case 5: // 0 def
				case 6: // 0 atk
				case 7: // 0 flee
				case 8: // -75% move speed and aspd
					sc_data[type].val3 = i;
					calc_flag = 1;
					break;
				default:
					break;
				}
			}
			if (sd && calc_flag)
				status_calc_pc (sd, 0);
		}
		break;

	case SC_GUILDAURA:
		{
			struct block_list *tbl = map_id2bl(sc_data[type].val2);
			
			if (tbl && battle_check_range(bl, tbl, 2)){
				sc_data[type].timer = add_timer(
					1000 + tick, status_change_timer,
					bl->id, data);
					return 0;
			}// ugh, don't  forget the brackets
		}
		break;
	}
	
	// default for all non-handled control paths
	// security system to prevent forgetting timer removal

	// if we reach this point we need the timer for the next call, 
	// so restore it to have status_change_end handle a valid timer
	sc_data[type].timer = temp_timerid; 

	return status_change_end( bl,type,tid );
}

/*==========================================
 * ƒXƒe[ƒ^ƒXˆÙíƒ^ƒCƒ}[”ÍˆÍˆ—
 *------------------------------------------
 */
int status_change_timer_sub(struct block_list *bl, va_list ap )
{
	struct block_list *src;
	int type;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, src=va_arg(ap,struct block_list*));
	type=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);

	if(bl->type!=BL_PC && bl->type!=BL_MOB)
		return 0;

	switch( type ){
	case SC_SIGHT:	/* ƒTƒCƒg */
	case SC_CONCENTRATE:
		if( (*status_get_option(bl))&6 ){
			status_change_end( bl, SC_HIDING, -1);
			status_change_end( bl, SC_CLOAKING, -1);
		}
		break;
	case SC_RUWACH:	/* ƒ‹ƒAƒt */
		if( (*status_get_option(bl))&6 ){
			struct status_change *sc_data = status_get_sc_data(bl);	// check whether the target is hiding/cloaking [celest]
			if (sc_data && (sc_data[SC_HIDING].timer != -1 ||	// if the target is using a special hiding, i.e not using normal hiding/cloaking, don't bother
				sc_data[SC_CLOAKING].timer != -1)) {
				status_change_end( bl, SC_HIDING, -1);
				status_change_end( bl, SC_CLOAKING, -1);
			}
			if(battle_check_target( src,bl, BCT_ENEMY ) > 0)
				skill_attack(BF_MAGIC,src,src,bl,AL_RUWACH,sc_data[type].val1,tick,0);
		}
		break;
	}
	return 0;
}


static int status_calc_sigma(void)
{
	int i,j,k;

	for(i=0;i<MAX_PC_CLASS;i++) {
		memset(hp_sigma_val[i],0,sizeof(hp_sigma_val[i]));
		for(k=0,j=2;j<=MAX_LEVEL;j++) {
			k += hp_coefficient[i]*j + 50;
			k -= k%100;
			hp_sigma_val[i][j-1] = k;
		}
	}
	return 0;
}

int status_readdb(void) {
	int i,j,k;
	FILE *fp;
	char line[1024],*p;

	// JOB•â³?’l‚P
	fp=fopen("db/job_db1.txt","r");
	if(fp==NULL){
		printf("can't read db/job_db1.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		char *split[50];
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<21 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(j<21)
			continue;
		max_weight_base[i]=atoi(split[0]);
		hp_coefficient[i]=atoi(split[1]);
		hp_coefficient2[i]=atoi(split[2]);
		sp_coefficient[i]=atoi(split[3]);
		for(j=0;j<17;j++)
			aspd_base[i][j]=atoi(split[j+4]);
		i++;
// -- moonsoul (below two lines added to accommodate high numbered new class ids)
		if(i==24)
			i=4001;
		if(i==MAX_PC_CLASS)
			break;
	}
	fclose(fp);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/job_db1.txt");
	ShowStatus(tmp_output);

	// JOBƒ{?ƒiƒX
	memset(job_bonus,0,sizeof(job_bonus));
	fp=fopen("db/job_db2.txt","r");
	if(fp==NULL){
		printf("can't read db/job_db2.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<MAX_LEVEL && p;j++){
			if(sscanf(p,"%d",&k)==0)
				break;
			job_bonus[0][i][j]=k;
			job_bonus[2][i][j]=k; //—{qE‚Ìƒ{?ƒiƒX‚Í•ª‚©‚ç‚È‚¢‚Ì‚Å?
			p=strchr(p,',');
			if(p) p++;
		}
		i++;
// -- moonsoul (below two lines added to accommodate high numbered new class ids)
		if(i==24)
			i=4001;
		if(i==MAX_PC_CLASS)
			break;
	}
	fclose(fp);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/job_db2.txt");
	ShowStatus(tmp_output);

	// JOBƒ{?ƒiƒX2 ?¶E—p
	fp=fopen("db/job_db2-2.txt","r");
	if(fp==NULL){
		printf("can't read db/job_db2-2.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		for(j=0,p=line;j<MAX_LEVEL && p;j++){
			if(sscanf(p,"%d",&k)==0)
				break;
			job_bonus[1][i][j]=k;
			p=strchr(p,',');
			if(p) p++;
		}
		i++;
		if(i==MAX_PC_CLASS)
			break;
	}
	fclose(fp);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/job_db2-2.txt");
	ShowStatus(tmp_output);

	// ƒTƒCƒY•â³ƒe?ƒuƒ‹
	for(i=0;i<3;i++)
		for(j=0;j<20;j++)
			atkmods[i][j]=100;
	fp=fopen("db/size_fix.txt","r");
	if(fp==NULL){
		printf("can't read db/size_fix.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		char *split[20];
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<20 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		for(j=0;j<20 && split[j];j++)
			atkmods[i][j]=atoi(split[j]);
		i++;
	}
	fclose(fp);
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/size_fix.txt");
	ShowStatus(tmp_output);

	// ¸?ƒf?ƒ^ƒe?ƒuƒ‹
	for(i=0;i<5;i++){
		for(j=0;j<10;j++)
			percentrefinery[i][j]=100;
		refinebonus[i][0]=0;
		refinebonus[i][1]=0;
		refinebonus[i][2]=10;
	}
	fp=fopen("db/refine_db.txt","r");
	if(fp==NULL){
		printf("can't read db/refine_db.txt\n");
		return 1;
	}
	i=0;
	while(fgets(line, sizeof(line)-1, fp)){
		char *split[16];
		if(line[0]=='/' && line[1]=='/')
			continue;
		if(atoi(line)<=0)
			continue;
		memset(split,0,sizeof(split));
		for(j=0,p=line;j<16 && p;j++){
			split[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		refinebonus[i][0]=atoi(split[0]);	// ¸?ƒ{?ƒiƒX
		refinebonus[i][1]=atoi(split[1]);	// ‰ß?¸?ƒ{?ƒiƒX
		refinebonus[i][2]=atoi(split[2]);	// ˆÀ‘S¸?ŒÀŠE
		for(j=0;j<10 && split[j];j++)
			percentrefinery[i][j]=atoi(split[j+3]);
		i++;
	}
	fclose(fp); //Lupus. close this file!!!
	sprintf(tmp_output,"Done reading '"CL_WHITE"%s"CL_RESET"'.\n","db/refine_db.txt");
	ShowStatus(tmp_output);

	return 0;
}

/*==========================================
 * ƒXƒLƒ‹ŠÖŒW‰Šú‰»ˆ—
 *------------------------------------------
 */
int do_init_status(void)
{
	add_timer_func_list(status_change_timer,"status_change_timer");
	status_readdb();
	status_calc_sigma();
	return 0;
}
