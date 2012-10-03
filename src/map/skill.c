// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/malloc.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"
#include "../common/ers.h"

#include "map.h"
#include "path.h"
#include "clif.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "pet.h"
#include "homunculus.h"
#include "mercenary.h"
#include "elemental.h"
#include "mob.h"
#include "npc.h"
#include "battle.h"
#include "battleground.h"
#include "party.h"
#include "itemdb.h"
#include "script.h"
#include "intif.h"
#include "log.h"
#include "chrif.h"
#include "guild.h"
#include "date.h"
#include "unit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>


#define SKILLUNITTIMER_INTERVAL	100

// ranges reserved for mapping skill ids to skilldb offsets
#define HM_SKILLRANGEMIN 700
#define HM_SKILLRANGEMAX HM_SKILLRANGEMIN + MAX_HOMUNSKILL
#define MC_SKILLRANGEMIN HM_SKILLRANGEMAX + 1
#define MC_SKILLRANGEMAX MC_SKILLRANGEMIN + MAX_MERCSKILL
#define EL_SKILLRANGEMIN MC_SKILLRANGEMAX + 1
#define EL_SKILLRANGEMAX EL_SKILLRANGEMIN + MAX_ELEMENTALSKILL
#define GD_SKILLRANGEMIN EL_SKILLRANGEMAX + 1
#define GD_SKILLRANGEMAX GD_SKILLRANGEMIN + MAX_GUILDSKILL

#if GD_SKILLRANGEMAX > 999
	#error GD_SKILLRANGEMAX is greater than 999
#endif
static struct eri *skill_unit_ers = NULL; //For handling skill_unit's [Skotlex]
static struct eri *skill_timer_ers = NULL; //For handling skill_timerskills [Skotlex]

DBMap* skillunit_db = NULL; // int id -> struct skill_unit*

DBMap* skilldb_name2id = NULL;

/**
 * Skill Cool Down Delay Saving
 * Struct skill_cd is not a member of struct map_session_data
 * to keep cooldowns in memory between player log-ins.
 * All cooldowns are reset when server is restarted.
 **/
DBMap* skillcd_db = NULL; // char_id -> struct skill_cd
struct skill_cd {
	int duration[MAX_SKILL_TREE];//milliseconds
	short skidx[MAX_SKILL_TREE];//the skill index entries belong to
	short nameid[MAX_SKILL_TREE];//skill id
	unsigned char cursor;
};

/**
 * Skill Unit Persistency during endack routes (mostly for songs see bugreport:4574)
 **/
DBMap* skillusave_db = NULL; // char_id -> struct skill_usave
struct skill_usave {
	int skill_num, skill_lv;
};

struct s_skill_db skill_db[MAX_SKILL_DB];
struct s_skill_produce_db skill_produce_db[MAX_SKILL_PRODUCE_DB];
struct s_skill_arrow_db skill_arrow_db[MAX_SKILL_ARROW_DB];
struct s_skill_abra_db skill_abra_db[MAX_SKILL_ABRA_DB];
struct s_skill_improvise_db {
	int skillid;
	short per;//1-10000
};
struct s_skill_improvise_db skill_improvise_db[MAX_SKILL_IMPROVISE_DB];
bool skill_reproduce_db[MAX_SKILL_DB];
struct s_skill_changematerial_db {
	int itemid;
	short rate;
	int qty[5];
	short qty_rate[5];
};
struct s_skill_changematerial_db skill_changematerial_db[MAX_SKILL_PRODUCE_DB];

//Warlock
struct s_skill_spellbook_db {
	int nameid;
	int skillid;
	int point;
};

struct s_skill_spellbook_db skill_spellbook_db[MAX_SKILL_SPELLBOOK_DB];
//Guillotine Cross
struct s_skill_magicmushroom_db skill_magicmushroom_db[MAX_SKILL_MAGICMUSHROOM_DB];

struct s_skill_unit_layout skill_unit_layout[MAX_SKILL_UNIT_LAYOUT];
int firewall_unit_pos;
int icewall_unit_pos;
int earthstrain_unit_pos;
//early declaration
int skill_block_check(struct block_list *bl, enum sc_type type, int skillid);
static int skill_check_unit_range (struct block_list *bl, int x, int y, int skillid, int skilllv);
static int skill_check_unit_range2 (struct block_list *bl, int x, int y, int skillid, int skilllv);
static int skill_destroy_trap( struct block_list *bl, va_list ap );
//Since only mob-casted splash skills can hit ice-walls
static inline int splash_target(struct block_list* bl)
{
#ifndef RENEWAL
	return ( bl->type == BL_MOB ) ? BL_SKILL|BL_CHAR : BL_CHAR;
#else // Some skills can now hit ground skills(traps, ice wall & etc.)
	return BL_SKILL|BL_CHAR;
#endif
}

/// Returns the id of the skill, or 0 if not found.
int skill_name2id(const char* name)
{
	if( name == NULL )
		return 0;

	return strdb_iget(skilldb_name2id, name);
}

/// Maps skill ids to skill db offsets.
/// Returns the skill's array index, or 0 (Unknown Skill).
int skill_get_index( int id )
{
	// avoid ranges reserved for mapping guild/homun/mercenary skills
	if( (id >= GD_SKILLRANGEMIN && id <= GD_SKILLRANGEMAX)
	||  (id >= HM_SKILLRANGEMIN && id <= HM_SKILLRANGEMAX)
	||  (id >= MC_SKILLRANGEMIN && id <= MC_SKILLRANGEMAX)
	||  (id >= EL_SKILLRANGEMIN && id <= EL_SKILLRANGEMAX) )
		return 0;
	
	// map skill id to skill db index
	if( id >= GD_SKILLBASE )
		id = GD_SKILLRANGEMIN + id - GD_SKILLBASE;
	else if( id >= EL_SKILLBASE )
		id = EL_SKILLRANGEMIN + id - EL_SKILLBASE;
	else if( id >= MC_SKILLBASE )
		id = MC_SKILLRANGEMIN + id - MC_SKILLBASE;
	else if( id >= HM_SKILLBASE )
		id = HM_SKILLRANGEMIN + id - HM_SKILLBASE;

	// validate result
	if( id <= 0 || id >= MAX_SKILL_DB )
		return 0;

	return id;
}

const char* skill_get_name( int id )
{
	return skill_db[skill_get_index(id)].name;
}

const char* skill_get_desc( int id )
{
	return skill_db[skill_get_index(id)].desc;
}

// out of bounds error checking [celest]
static void skill_chk(int* id, int  lv)
{
	*id = skill_get_index(*id); // checks/adjusts id
	if( lv <= 0 || lv > MAX_SKILL_LEVEL ) *id = 0;
}

#define skill_get(var,id,lv) { skill_chk(&id,lv); if(!id) return 0; return var; }

// Skill DB
int	skill_get_hit( int id )               { skill_get (skill_db[id].hit, id, 1); }
int	skill_get_inf( int id )               { skill_get (skill_db[id].inf, id, 1); }
int	skill_get_ele( int id , int lv )      { skill_get (skill_db[id].element[lv-1], id, lv); }
int	skill_get_nk( int id )                { skill_get (skill_db[id].nk, id, 1); }
int	skill_get_max( int id )               { skill_get (skill_db[id].max, id, 1); }
int	skill_get_range( int id , int lv )    { skill_get (skill_db[id].range[lv-1], id, lv); }
int	skill_get_splash( int id , int lv )   { skill_chk (&id, lv); return (skill_db[id].splash[lv-1]>=0?skill_db[id].splash[lv-1]:AREA_SIZE); }
int	skill_get_hp( int id ,int lv )        { skill_get (skill_db[id].hp[lv-1], id, lv); }
int	skill_get_sp( int id ,int lv )        { skill_get (skill_db[id].sp[lv-1], id, lv); }
int	skill_get_hp_rate(int id, int lv )    { skill_get (skill_db[id].hp_rate[lv-1], id, lv); }
int	skill_get_sp_rate(int id, int lv )    { skill_get (skill_db[id].sp_rate[lv-1], id, lv); }
int	skill_get_state(int id)               { skill_get (skill_db[id].state, id, 1); }
int	skill_get_spiritball(int id, int lv)  { skill_get (skill_db[id].spiritball[lv-1], id, lv); }
int	skill_get_itemid(int id, int idx)     { skill_get (skill_db[id].itemid[idx], id, 1); }
int	skill_get_itemqty(int id, int idx)    { skill_get (skill_db[id].amount[idx], id, 1); }
int	skill_get_zeny( int id ,int lv )      { skill_get (skill_db[id].zeny[lv-1], id, lv); }
int	skill_get_num( int id ,int lv )       { skill_get (skill_db[id].num[lv-1], id, lv); }
int	skill_get_cast( int id ,int lv )      { skill_get (skill_db[id].cast[lv-1], id, lv); }
int	skill_get_delay( int id ,int lv )     { skill_get (skill_db[id].delay[lv-1], id, lv); }
int	skill_get_walkdelay( int id ,int lv ) { skill_get (skill_db[id].walkdelay[lv-1], id, lv); }
int	skill_get_time( int id ,int lv )      { skill_get (skill_db[id].upkeep_time[lv-1], id, lv); }
int	skill_get_time2( int id ,int lv )     { skill_get (skill_db[id].upkeep_time2[lv-1], id, lv); }
int	skill_get_castdef( int id )           { skill_get (skill_db[id].cast_def_rate, id, 1); }
int	skill_get_weapontype( int id )        { skill_get (skill_db[id].weapon, id, 1); }
int	skill_get_ammotype( int id )          { skill_get (skill_db[id].ammo, id, 1); }
int	skill_get_ammo_qty( int id, int lv )  { skill_get (skill_db[id].ammo_qty[lv-1], id, lv); }
int	skill_get_inf2( int id )              { skill_get (skill_db[id].inf2, id, 1); }
int	skill_get_castcancel( int id )        { skill_get (skill_db[id].castcancel, id, 1); }
int	skill_get_maxcount( int id ,int lv )  { skill_get (skill_db[id].maxcount[lv-1], id, lv); }
int	skill_get_blewcount( int id ,int lv ) { skill_get (skill_db[id].blewcount[lv-1], id, lv); }
int	skill_get_mhp( int id ,int lv )       { skill_get (skill_db[id].mhp[lv-1], id, lv); }
int	skill_get_castnodex( int id ,int lv ) { skill_get (skill_db[id].castnodex[lv-1], id, lv); }
int	skill_get_delaynodex( int id ,int lv ){ skill_get (skill_db[id].delaynodex[lv-1], id, lv); }
int	skill_get_nocast ( int id )           { skill_get (skill_db[id].nocast, id, 1); }
int	skill_get_type( int id )              { skill_get (skill_db[id].skill_type, id, 1); }
int	skill_get_unit_id ( int id, int flag ){ skill_get (skill_db[id].unit_id[flag], id, 1); }
int	skill_get_unit_interval( int id )     { skill_get (skill_db[id].unit_interval, id, 1); }
int	skill_get_unit_range( int id, int lv ){ skill_get (skill_db[id].unit_range[lv-1], id, lv); }
int	skill_get_unit_target( int id )       { skill_get (skill_db[id].unit_target&BCT_ALL, id, 1); }
int	skill_get_unit_bl_target( int id )    { skill_get (skill_db[id].unit_target&BL_ALL, id, 1); }
int	skill_get_unit_flag( int id )         { skill_get (skill_db[id].unit_flag, id, 1); }
int	skill_get_unit_layout_type( int id ,int lv ){ skill_get (skill_db[id].unit_layout_type[lv-1], id, lv); }
int	skill_get_cooldown( int id ,int lv )     { skill_get (skill_db[id].cooldown[lv-1], id, lv); }
#ifdef RENEWAL_CAST
int	skill_get_fixed_cast( int id ,int lv ){ skill_get (skill_db[id].fixed_cast[lv-1], id, lv); }
#endif
int skill_tree_get_max(int id, int b_class)
{
	int i;
	b_class = pc_class2idx(b_class);

	ARR_FIND( 0, MAX_SKILL_TREE, i, skill_tree[b_class][i].id == 0 || skill_tree[b_class][i].id == id );
	if( i < MAX_SKILL_TREE && skill_tree[b_class][i].id == id )
		return skill_tree[b_class][i].max;
	else
		return skill_get_max(id);
}

int skill_frostjoke_scream(struct block_list *bl,va_list ap);
int skill_attack_area(struct block_list *bl,va_list ap);
struct skill_unit_group *skill_locate_element_field(struct block_list *bl); // [Skotlex]
int skill_graffitiremover(struct block_list *bl, va_list ap); // [Valaris]
int skill_greed(struct block_list *bl, va_list ap);
static void skill_toggle_magicpower(struct block_list *bl, short skillid);
static int skill_cell_overlap(struct block_list *bl, va_list ap);
static int skill_trap_splash(struct block_list *bl, va_list ap);
struct skill_unit_group_tickset *skill_unitgrouptickset_search(struct block_list *bl,struct skill_unit_group *sg,int tick);
static int skill_unit_onplace(struct skill_unit *src,struct block_list *bl,unsigned int tick);
static int skill_unit_onleft(int skill_id, struct block_list *bl,unsigned int tick);
static int skill_unit_effect(struct block_list *bl,va_list ap);

int enchant_eff[5] = { 10, 14, 17, 19, 20 };
int deluge_eff[5] = { 5, 9, 12, 14, 15 };

int skill_get_casttype (int id)
{
	int inf = skill_get_inf(id);
	if (inf&(INF_GROUND_SKILL))
		return CAST_GROUND;
	if (inf&INF_SUPPORT_SKILL)
		return CAST_NODAMAGE;
	if (inf&INF_SELF_SKILL) {
		if(skill_get_inf2(id)&INF2_NO_TARGET_SELF)
			return CAST_DAMAGE; //Combo skill.
		return CAST_NODAMAGE;
	}
	if (skill_get_nk(id)&NK_NO_DAMAGE)
		return CAST_NODAMAGE;
	return CAST_DAMAGE;
}

//Returns actual skill range taking into account attack range and AC_OWL [Skotlex]
int skill_get_range2 (struct block_list *bl, int id, int lv)
{
	int range;
	if( bl->type == BL_MOB && battle_config.mob_ai&0x400 )
		return 9; //Mobs have a range of 9 regardless of skill used.

	range = skill_get_range(id, lv);

	if( range < 0 )
	{
		if( battle_config.use_weapon_skill_range&bl->type )
			return status_get_range(bl);
		range *=-1;
	}

	//TODO: Find a way better than hardcoding the list of skills affected by AC_VULTURE
	switch( id )
	{
	case AC_SHOWER:			case MA_SHOWER:
	case AC_DOUBLE:			case MA_DOUBLE:
	case HT_BLITZBEAT:
	case AC_CHARGEARROW:
	case MA_CHARGEARROW:
	case SN_FALCONASSAULT:
	case HT_POWER:
	/**
	 * Ranger
	 **/
	case RA_ARROWSTORM:
	case RA_AIMEDBOLT:
	case RA_WUGBITE:
		if( bl->type == BL_PC )
			range += pc_checkskill((TBL_PC*)bl, AC_VULTURE);
		else
			range += 10; //Assume level 10?
		break;
	// added to allow GS skills to be effected by the range of Snake Eyes [Reddozen]
	case GS_RAPIDSHOWER:
	case GS_PIERCINGSHOT:
	case GS_FULLBUSTER:
	case GS_SPREADATTACK:
	case GS_GROUNDDRIFT:
		if (bl->type == BL_PC)
			range += pc_checkskill((TBL_PC*)bl, GS_SNAKEEYE);
		else
			range += 10; //Assume level 10?
		break;
	case NJ_KIRIKAGE:
		if (bl->type == BL_PC)
			range = skill_get_range(NJ_SHADOWJUMP,pc_checkskill((TBL_PC*)bl,NJ_SHADOWJUMP));
		break;
	/**
	 * Warlock
	 **/
	case WL_WHITEIMPRISON:
	case WL_SOULEXPANSION:
	case WL_FROSTMISTY:
	case WL_MARSHOFABYSS:
	case WL_SIENNAEXECRATE:
	case WL_DRAINLIFE:
	case WL_CRIMSONROCK:
	case WL_HELLINFERNO:
	case WL_COMET:
	case WL_CHAINLIGHTNING:
	case WL_TETRAVORTEX:
	case WL_RELEASE:
			if( bl->type == BL_PC )
				range += pc_checkskill((TBL_PC*)bl, WL_RADIUS);
			break;
	/**
	 * Ranger Bonus
	 **/
	case HT_LANDMINE:
	case HT_FREEZINGTRAP:
	case HT_BLASTMINE:
	case HT_CLAYMORETRAP:
	case RA_CLUSTERBOMB:
	case RA_FIRINGTRAP:
	case RA_ICEBOUNDTRAP:
		if( bl->type == BL_PC )
			range += (1 + pc_checkskill((TBL_PC*)bl, RA_RESEARCHTRAP))/2;
	}

	if( !range && bl->type != BL_PC )
		return 9; // Enable non players to use self skills on others. [Skotlex]
	return range;
}

int skill_calc_heal(struct block_list *src, struct block_list *target, int skill_id, int skill_lv, bool heal) {
	int skill, hp;
	struct map_session_data *sd = BL_CAST(BL_PC, src);
	struct map_session_data *tsd = BL_CAST(BL_PC, target);
	struct status_change* sc;

	switch( skill_id ) {
	case BA_APPLEIDUN:
	#ifdef RENEWAL
		hp = 100+5*skill_lv+5*(status_get_vit(src)/10); // HP recovery
	#else
		hp = 30+5*skill_lv+5*(status_get_vit(src)/10); // HP recovery
	#endif
		if( sd )
				hp += 5*pc_checkskill(sd,BA_MUSICALLESSON);
			break;
		case PR_SANCTUARY:
			hp = (skill_lv>6)?777:skill_lv*100;
			break;
		case NPC_EVILLAND:
			hp = (skill_lv>6)?666:skill_lv*100;
			break;
		default:
			if (skill_lv >= battle_config.max_heal_lv)
				return battle_config.max_heal;
		#ifdef RENEWAL
			/**
			 * Renewal Heal Formula (from Doddler)
			 * TODO: whats that( 1+ %Modifier / 100 ) ? currently using 'x1' (100/100) until found out
			 * - Min = ( [ ( BaseLvl + INT ) / 5 ] * 30 ) * (1+( %Modifier / 100)) * (HealLvl * 0.1) + StatusMATK + EquipMATK - [(WeaponMATK * WeaponLvl) / 10]
			 * - Max = ( [ ( BaseLvl + INT ) / 5 ] * 30 ) * (1+( %Modifier / 100)) * (HealLvl * 0.1) + StatusMATK + EquipMATK + [(WeaponMATK * WeaponLvl) / 10] 
			 **/
			hp = ( ( ( ( status_get_lv(src) + status_get_int(src) ) / 5 ) * 3 ) * skill_lv  + status_get_matk_min(src) + status_get_matk_max(src) - ( ( status_get_matk_max(src) * status_get_wlv(src) ) / 10 ) ) + rnd()%( ( ( ( status_get_lv(src) + status_get_int(src) ) / 5 ) * 3 ) * skill_lv + status_get_matk_min(src) + status_get_matk_max(src) + ( ( status_get_matk_max(src) * status_get_wlv(src) ) / 10 ) );
		#else
			hp = ( status_get_lv(src) + status_get_int(src) ) / 8 * (4 + ( skill_id == AB_HIGHNESSHEAL ? ( sd ? pc_checkskill(sd,AL_HEAL) : 10 ) : skill_lv ) * 8);
		#endif
			if( sd && ((skill = pc_checkskill(sd, HP_MEDITATIO)) > 0) )
				hp += hp * skill * 2 / 100;
			else if( src->type == BL_HOM && (skill = merc_hom_checkskill(((TBL_HOM*)src), HLIF_BRAIN)) > 0 )
				hp += hp * skill * 2 / 100;
			break;
	}

	if( ( (target && target->type == BL_MER) || !heal ) && skill_id != NPC_EVILLAND )
		hp >>= 1;

	if( sd && (skill = pc_skillheal_bonus(sd, skill_id)) )
		hp += hp*skill/100;
	
	if( tsd && (skill = pc_skillheal2_bonus(tsd, skill_id)) )
		hp += hp*skill/100;

	sc = status_get_sc(target);
	if( sc && sc->count ) {
		if( sc->data[SC_CRITICALWOUND] && heal ) // Critical Wound has no effect on offensive heal. [Inkfish]
			hp -= hp * sc->data[SC_CRITICALWOUND]->val2/100;
		if( sc->data[SC_DEATHHURT] && heal )
			hp -= hp * 20/100;
		if( sc->data[SC_INCHEALRATE] && skill_id != NPC_EVILLAND && skill_id != BA_APPLEIDUN )
			hp += hp * sc->data[SC_INCHEALRATE]->val1/100; // Only affects Heal, Sanctuary and PotionPitcher.(like bHealPower) [Inkfish]
		if( sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2)
			hp += hp / 10;
	}

	return hp;
}

// Making plagiarize check its own function [Aru]
int can_copy (struct map_session_data *sd, int skillid, struct block_list* bl)
{
	// Never copy NPC/Wedding Skills
	if (skill_get_inf2(skillid)&(INF2_NPC_SKILL|INF2_WEDDING_SKILL))
		return 0;

	// High-class skills
	if((skillid >= LK_AURABLADE && skillid <= ASC_CDP) || (skillid >= ST_PRESERVE && skillid <= CR_CULTIVATION))
	{
		if(battle_config.copyskill_restrict == 2)
			return 0;
		else if(battle_config.copyskill_restrict)
			return (sd->status.class_ == JOB_STALKER);
	}

	//Added so plagarize can't copy agi/bless if you're undead since it damages you
	if ((skillid == AL_INCAGI || skillid == AL_BLESSING || 
		skillid == CASH_BLESSING || skillid == CASH_INCAGI || 
		skillid == MER_INCAGI || skillid == MER_BLESSING))
		return 0;

	// Couldn't preserve 3rd Class skills except only when using Reproduce skill. [Jobbie]
	if( !(sd->sc.data[SC__REPRODUCE]) && (skillid >= RK_ENCHANTBLADE && skillid <= SR_RIDEINLIGHTNING) )
		return 0;
	// Reproduce will only copy skills according on the list. [Jobbie]
	else if( sd->sc.data[SC__REPRODUCE] && !skill_reproduce_db[skillid] )
		return 0;

	return 1;
}

// [MouseJstr] - skill ok to cast? and when?
int skillnotok (int skillid, struct map_session_data *sd)
{
	int i,m;
	nullpo_retr (1, sd);
	m = sd->bl.m;
	i = skill_get_index(skillid);

	if (i == 0)
		return 1; // invalid skill id

	if (pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL))
		return 0; // can do any damn thing they want

	if( skillid == AL_TELEPORT && sd->skillitem == skillid && sd->skillitemlv > 2 )
		return 0; // Teleport lv 3 bypasses this check.[Inkfish]

	// Epoque:
	// This code will compare the player's attack motion value which is influenced by ASPD before
	// allowing a skill to be cast. This is to prevent no-delay ACT files from spamming skills such as
	// AC_DOUBLE which do not have a skill delay and are not regarded in terms of attack motion.
	if( !sd->state.autocast && sd->skillitem != skillid && sd->canskill_tick &&
		DIFF_TICK(gettick(), sd->canskill_tick) < (sd->battle_status.amotion * (100 + battle_config.skill_amotion_leniency) / 100) )
	{// attempted to cast a skill before the attack motion has finished
		return 1;
	}

	if (sd->blockskill[i] > 0){
		clif_skill_fail(sd, skillid, USESKILL_FAIL_SKILLINTERVAL, 0);
		return 1;
	}
	/**
	 * It has been confirmed on a official server (thanks to Yommy) that item-cast skills bypass all the restrictions above
	 * Also, without this check, an exploit where an item casting + healing (or any other kind buff) isn't deleted after used on a restricted map
	 **/
	if( sd->skillitem == skillid )
		return 0;
	// Check skill restrictions [Celest]
	if( (!map_flag_vs(m) && skill_get_nocast (skillid) & 1) ||
		(map[m].flag.pvp && skill_get_nocast (skillid) & 2) ||
		(map_flag_gvg(m) && skill_get_nocast (skillid) & 4) ||
		(map[m].flag.battleground && skill_get_nocast (skillid) & 8) ||
		(map[m].flag.restricted && map[m].zone && skill_get_nocast (skillid) & (8*map[m].zone)) ){
			clif_msg(sd, 0x536); // This skill cannot be used within this area
			return 1;
	}

	if( sd->sc.option&OPTION_MOUNTING )
		return 1;//You can't use skills while in the new mounts (The client doesn't let you, this is to make cheat-safe)

	switch (skillid) {
		case AL_WARP:
		case RETURN_TO_ELDICASTES:
		case ALL_GUARDIAN_RECALL:
			if(map[m].flag.nowarp) {
				clif_skill_teleportmessage(sd,0);
				return 1;
			}
			return 0;
		case AL_TELEPORT:
		case SC_FATALMENACE:
		case SC_DIMENSIONDOOR:
			if(map[m].flag.noteleport) {
				clif_skill_teleportmessage(sd,0);
				return 1;
			}
			return 0; // gonna be checked in 'skill_castend_nodamage_id'
		case WE_CALLPARTNER:
		case WE_CALLPARENT:
		case WE_CALLBABY:
			if (map[m].flag.nomemo) {
				clif_skill_teleportmessage(sd,1);
				return 1;
			}
			break;
		case MC_VENDING:
		case MC_IDENTIFY:
		case ALL_BUYING_STORE:
			return 0; // always allowed
		case WZ_ICEWALL:
			// noicewall flag [Valaris]
			if (map[m].flag.noicewall) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;
		case GC_DARKILLUSION:
			if( map_flag_gvg(m) ) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;
		case GD_EMERGENCYCALL:
			if (
				!(battle_config.emergency_call&((agit_flag || agit2_flag)?2:1)) ||
				!(battle_config.emergency_call&(map[m].flag.gvg || map[m].flag.gvg_castle?8:4)) ||
				(battle_config.emergency_call&16 && map[m].flag.nowarpto && !map[m].flag.gvg_castle)
			)	{
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;
		case BS_GREED:
		case WS_CARTBOOST:
		case BS_HAMMERFALL:
		case BS_ADRENALINE:
		case MC_CARTREVOLUTION:
		case MC_MAMMONITE:
		case WS_MELTDOWN:
		case MG_SIGHT:
		case TF_HIDING:
			/**
			 * These skills cannot be used while in mado gear (credits to Xantara)
			 **/
			if( pc_ismadogear(sd) ) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			break;
		case WM_LULLABY_DEEPSLEEP:
		case WM_SIRCLEOFNATURE:
			if( !map_flag_vs(m) ) {
				clif_skill_teleportmessage(sd,2); // This skill uses this msg instead of skill fails.
				return 1;
			}
			break;

	}
	return (map[m].flag.noskill);
}

int skillnotok_hom(int skillid, struct homun_data *hd)
{
	int i = skill_get_index(skillid);
	nullpo_retr(1,hd);

	if (i == 0)
		return 1; // invalid skill id

	if (hd->blockskill[i] > 0)
		return 1;

	//Use master's criteria.
	return skillnotok(skillid, hd->master);
}

int skillnotok_mercenary(int skillid, struct mercenary_data *md)
{
	int i = skill_get_index(skillid);
	nullpo_retr(1,md);

	if( i == 0 )
		return 1; // Invalid Skill ID
	if( md->blockskill[i] > 0 )
		return 1;

	return skillnotok(skillid, md->master);
}

struct s_skill_unit_layout* skill_get_unit_layout (int skillid, int skilllv, struct block_list* src, int x, int y)
{
	int pos = skill_get_unit_layout_type(skillid,skilllv);
	int dir;

	if (pos < -1 || pos >= MAX_SKILL_UNIT_LAYOUT) {
		ShowError("skill_get_unit_layout: unsupported layout type %d for skill %d (level %d)\n", pos, skillid, skilllv);
		pos = cap_value(pos, 0, MAX_SQUARE_LAYOUT); // cap to nearest square layout
	}

	if (pos != -1) // simple single-definition layout
		return &skill_unit_layout[pos];

	dir = (src->x == x && src->y == y) ? 6 : map_calc_dir(src,x,y); // 6 - default aegis direction

	if (skillid == MG_FIREWALL)
		return &skill_unit_layout [firewall_unit_pos + dir];
	else if (skillid == WZ_ICEWALL)
		return &skill_unit_layout [icewall_unit_pos + dir];
	else if( skillid == WL_EARTHSTRAIN ) //Warlock
		return &skill_unit_layout [earthstrain_unit_pos + dir];

	ShowError("skill_get_unit_layout: unknown unit layout for skill %d (level %d)\n", skillid, skilllv);
	return &skill_unit_layout[0]; // default 1x1 layout
}

/*==========================================
 *
 *------------------------------------------*/
int skill_additional_effect (struct block_list* src, struct block_list *bl, int skillid, int skilllv, int attack_type, int dmg_lv, unsigned int tick)
{
	struct map_session_data *sd, *dstsd;
	struct mob_data *md, *dstmd;
	struct status_data *sstatus, *tstatus;
	struct status_change *sc, *tsc;

	enum sc_type status;
	int skill;
	int rate;

	nullpo_ret(src);
	nullpo_ret(bl);

	if(skillid < 0) return 0;
	if(skillid > 0 && skilllv <= 0) return 0;	// don't forget auto attacks! - celest

	if( dmg_lv < ATK_BLOCK ) // Don't apply effect if miss.
		return 0;

	sd = BL_CAST(BL_PC, src);
	md = BL_CAST(BL_MOB, src);
	dstsd = BL_CAST(BL_PC, bl);
	dstmd = BL_CAST(BL_MOB, bl);

	sc = status_get_sc(src);
	tsc = status_get_sc(bl);
	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(bl);
	if (!tsc) //skill additional effect is about adding effects to the target...
		//So if the target can't be inflicted with statuses, this is pointless.
		return 0;

	if( sd )
	{ // These statuses would be applied anyway even if the damage was blocked by some skills. [Inkfish]
		if( skillid != WS_CARTTERMINATION && skillid != AM_DEMONSTRATION && skillid != CR_REFLECTSHIELD && skillid != MS_REFLECTSHIELD && skillid != ASC_BREAKER )
		{ // Trigger status effects
			enum sc_type type;
			int i;
			for( i = 0; i < ARRAYLENGTH(sd->addeff) && sd->addeff[i].flag; i++ )
			{
				rate = sd->addeff[i].rate;
				if( attack_type&BF_LONG ) // Any ranged physical attack takes status arrows into account (Grimtooth...) [DracoRPG]
					rate += sd->addeff[i].arrow_rate;
				if( !rate ) continue;

				if( (sd->addeff[i].flag&(ATF_WEAPON|ATF_MAGIC|ATF_MISC)) != (ATF_WEAPON|ATF_MAGIC|ATF_MISC) ) 
				{ // Trigger has attack type consideration. 
					if( (sd->addeff[i].flag&ATF_WEAPON && attack_type&BF_WEAPON) ||
						(sd->addeff[i].flag&ATF_MAGIC && attack_type&BF_MAGIC) ||
						(sd->addeff[i].flag&ATF_MISC && attack_type&BF_MISC) ) ;
					else
						continue; 
				}

				if( (sd->addeff[i].flag&(ATF_LONG|ATF_SHORT)) != (ATF_LONG|ATF_SHORT) )
				{ // Trigger has range consideration.
					if((sd->addeff[i].flag&ATF_LONG && !(attack_type&BF_LONG)) ||
						(sd->addeff[i].flag&ATF_SHORT && !(attack_type&BF_SHORT)))
						continue; //Range Failed.
				}

				type =  sd->addeff[i].id;
				skill = skill_get_time2(status_sc2skill(type),7);

				if (sd->addeff[i].flag&ATF_TARGET)
					status_change_start(bl,type,rate,7,0,0,0,skill,0);

				if (sd->addeff[i].flag&ATF_SELF)
					status_change_start(src,type,rate,7,0,0,0,skill,0);
			}
		}

		if( skillid )
		{ // Trigger status effects on skills
			enum sc_type type;
			int i;
			for( i = 0; i < ARRAYLENGTH(sd->addeff3) && sd->addeff3[i].skill; i++ )
			{
				if( skillid != sd->addeff3[i].skill || !sd->addeff3[i].rate )
					continue;
				type = sd->addeff3[i].id;
				skill = skill_get_time2(status_sc2skill(type),7);

				if( sd->addeff3[i].target&ATF_TARGET )
					status_change_start(bl,type,sd->addeff3[i].rate,7,0,0,0,skill,0);
				if( sd->addeff3[i].target&ATF_SELF )
					status_change_start(src,type,sd->addeff3[i].rate,7,0,0,0,skill,0);
			}
		}
	}

	if( dmg_lv < ATK_DEF ) // no damage, return;
		return 0;

	switch(skillid)
	{
	case 0: // Normal attacks (no skill used)
	{
		if( attack_type&BF_SKILL )
			break; // If a normal attack is a skill, it's splash damage. [Inkfish]
		if(sd) {
			// Automatic trigger of Blitz Beat
			if (pc_isfalcon(sd) && sd->status.weapon == W_BOW && (skill=pc_checkskill(sd,HT_BLITZBEAT))>0 &&
				rnd()%1000 <= sstatus->luk*10/3+1 ) {
				rate=(sd->status.job_level+9)/10;
				skill_castend_damage_id(src,bl,HT_BLITZBEAT,(skill<rate)?skill:rate,tick,SD_LEVEL);
			}
			// Automatic trigger of Warg Strike [Jobbie]
			if( pc_iswug(sd) && (sd->status.weapon == W_BOW || sd->status.weapon == W_FIST) && (skill=pc_checkskill(sd,RA_WUGSTRIKE)) > 0 && rnd()%1000 <= sstatus->luk*10/3+1 )
				skill_castend_damage_id(src,bl,RA_WUGSTRIKE,skill,tick,0);
			// Gank
			if(dstmd && sd->status.weapon != W_BOW &&
				(skill=pc_checkskill(sd,RG_SNATCHER)) > 0 &&
				(skill*15 + 55) + pc_checkskill(sd,TF_STEAL)*10 > rnd()%1000) {
				if(pc_steal_item(sd,bl,pc_checkskill(sd,TF_STEAL)))
					clif_skill_nodamage(src,bl,TF_STEAL,skill,1);
				else
					clif_skill_fail(sd,RG_SNATCHER,USESKILL_FAIL_LEVEL,0);
			}
			// Chance to trigger Taekwon kicks [Dralnu]
			if(sc && !sc->data[SC_COMBO]) {
				if(sc->data[SC_READYSTORM] &&
					sc_start(src,SC_COMBO, 15, TK_STORMKICK,
						(2000 - 4*sstatus->agi - 2*sstatus->dex)))
					; //Stance triggered
				else if(sc->data[SC_READYDOWN] &&
					sc_start(src,SC_COMBO, 15, TK_DOWNKICK,
						(2000 - 4*sstatus->agi - 2*sstatus->dex)))
					; //Stance triggered
				else if(sc->data[SC_READYTURN] &&
					sc_start(src,SC_COMBO, 15, TK_TURNKICK,
						(2000 - 4*sstatus->agi - 2*sstatus->dex)))
					; //Stance triggered
				else if(sc->data[SC_READYCOUNTER])
				{	//additional chance from SG_FRIEND [Komurka]
					rate = 20;
					if (sc->data[SC_SKILLRATE_UP] && sc->data[SC_SKILLRATE_UP]->val1 == TK_COUNTER) {
						rate += rate*sc->data[SC_SKILLRATE_UP]->val2/100;
						status_change_end(src, SC_SKILLRATE_UP, INVALID_TIMER);
					}
					sc_start4(src,SC_COMBO, rate, TK_COUNTER, bl->id,0,0,
						(2000 - 4*sstatus->agi - 2*sstatus->dex));
				}
			}
		}

		if (sc) {
			struct status_change_entry *sce;
			// Enchant Poison gives a chance to poison attacked enemies
			if((sce=sc->data[SC_ENCPOISON])) //Don't use sc_start since chance comes in 1/10000 rate.
				status_change_start(bl,SC_POISON,sce->val2, sce->val1,src->id,0,0,
					skill_get_time2(AS_ENCHANTPOISON,sce->val1),0);
			// Enchant Deadly Poison gives a chance to deadly poison attacked enemies
			if((sce=sc->data[SC_EDP]))
				sc_start4(bl,SC_DPOISON,sce->val2, sce->val1,src->id,0,0,
					skill_get_time2(ASC_EDP,sce->val1));
			// Cancels on normal attack but benefits with the bonuses
			status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);
		}
	}
	break;

	case SM_BASH:
		if( sd && skilllv > 5 && pc_checkskill(sd,SM_FATALBLOW)>0 ){
			//TODO: How much % per base level it actually is?
			sc_start(bl,SC_STUN,(5*(skilllv-5)+(int)sd->status.base_level/10),
				skilllv,skill_get_time2(SM_FATALBLOW,skilllv));
		}
		break;

	case MER_CRASH:
		sc_start(bl,SC_STUN,(6*skilllv),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case AS_VENOMKNIFE:
		if (sd) //Poison chance must be that of Envenom. [Skotlex]
			skilllv = pc_checkskill(sd, TF_POISON);
	case TF_POISON:
	case AS_SPLASHER:
		if(!sc_start2(bl,SC_POISON,(4*skilllv+10),skilllv,src->id,skill_get_time2(skillid,skilllv))
			&&	sd && skillid==TF_POISON
		)
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		break;

	case AS_SONICBLOW:
		sc_start(bl,SC_STUN,(2*skilllv+10),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case WZ_FIREPILLAR:
		unit_set_walkdelay(bl, tick, skill_get_time2(skillid, skilllv), 1);
		break;

	case MG_FROSTDIVER:
#ifndef RENEWAL
	case WZ_FROSTNOVA:
#endif
		sc_start(bl,SC_FREEZE,skilllv*3+35,skilllv,skill_get_time2(skillid,skilllv));
		break;

#ifdef RENEWAL
	case WZ_FROSTNOVA:
		sc_start(bl,SC_FREEZE,skilllv*5+33,skilllv,skill_get_time2(skillid,skilllv));
		break;
#endif
			
	case WZ_STORMGUST:
	/**
	 * Storm Gust counter was dropped in renewal
	 **/
	#ifdef RENEWAL
		sc_start(bl,SC_FREEZE,65-(5*skilllv),skilllv,skill_get_time2(skillid,skilllv));
	#else
		 //Tharis pointed out that this is normal freeze chance with a base of 300%
		if(tsc->sg_counter >= 3 &&
			sc_start(bl,SC_FREEZE,300,skilllv,skill_get_time2(skillid,skilllv)))
			tsc->sg_counter = 0;
		/**
		 * being it only resets on success it'd keep stacking and eventually overflowing on mvps, so we reset at a high value
		 **/
		else if( tsc->sg_counter > 250 )
			tsc->sg_counter = 0;
	#endif
		break;

	case WZ_METEOR:
		sc_start(bl,SC_STUN,3*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case WZ_VERMILION:
		sc_start(bl,SC_BLIND,4*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case HT_FREEZINGTRAP:
	case MA_FREEZINGTRAP:
		sc_start(bl,SC_FREEZE,(3*skilllv+35),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case HT_FLASHER:
		sc_start(bl,SC_BLIND,(10*skilllv+30),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case HT_LANDMINE:
	case MA_LANDMINE:
		sc_start(bl,SC_STUN,(5*skilllv+30),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case HT_SHOCKWAVE:
		status_percent_damage(src, bl, 0, 15*skilllv+5, false);
		break;

	case HT_SANDMAN:
	case MA_SANDMAN:
		sc_start(bl,SC_SLEEP,(10*skilllv+40),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case TF_SPRINKLESAND:
		sc_start(bl,SC_BLIND,20,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case TF_THROWSTONE:
		sc_start(bl,SC_STUN,3,skilllv,skill_get_time(skillid,skilllv));
		sc_start(bl,SC_BLIND,3,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case NPC_DARKCROSS:
	case CR_HOLYCROSS:
		sc_start(bl,SC_BLIND,3*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case CR_GRANDCROSS:
	case NPC_GRANDDARKNESS:
		//Chance to cause blind status vs demon and undead element, but not against players
		if(!dstsd && (battle_check_undead(tstatus->race,tstatus->def_ele) || tstatus->race == RC_DEMON))
			sc_start(bl,SC_BLIND,100,skilllv,skill_get_time2(skillid,skilllv));
		attack_type |= BF_WEAPON;
		break;

	case AM_ACIDTERROR:
		sc_start(bl,SC_BLEEDING,(skilllv*3),skilllv,skill_get_time2(skillid,skilllv));
		if (skill_break_equip(bl, EQP_ARMOR, 100*skill_get_time(skillid,skilllv), BCT_ENEMY))
			clif_emotion(bl,E_OMG);
		break;

	case AM_DEMONSTRATION:
		skill_break_equip(bl, EQP_WEAPON, 100*skilllv, BCT_ENEMY);
		break;

	case CR_SHIELDCHARGE:
		sc_start(bl,SC_STUN,(15+skilllv*5),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case PA_PRESSURE:
		status_percent_damage(src, bl, 0, 15+5*skilllv, false);
		break;

	case RG_RAID:
		sc_start(bl,SC_STUN,(10+3*skilllv),skilllv,skill_get_time(skillid,skilllv));
		sc_start(bl,SC_BLIND,(10+3*skilllv),skilllv,skill_get_time2(skillid,skilllv));

#ifdef RENEWAL
		sc_start(bl,SC_RAID,100,7,5000);
		break;

	case RG_BACKSTAP:
		sc_start(bl,SC_STUN,(5+2*skilllv),skilllv,skill_get_time(skillid,skilllv));
#endif
		break;

	case BA_FROSTJOKER:
		sc_start(bl,SC_FREEZE,(15+5*skilllv),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case DC_SCREAM:
		sc_start(bl,SC_STUN,(25+5*skilllv),skilllv,skill_get_time2(skillid,skilllv));
		break;

	case BD_LULLABY:
		sc_start(bl,SC_SLEEP,15,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case DC_UGLYDANCE:
		rate = 5+5*skilllv;
		if(sd && (skill=pc_checkskill(sd,DC_DANCINGLESSON)))
		    rate += 5+skill;
		status_zap(bl, 0, rate);
  		break;
	case SL_STUN:
		if (tstatus->size==SZ_MEDIUM) //Only stuns mid-sized mobs.
			sc_start(bl,SC_STUN,(30+10*skilllv),skilllv,skill_get_time(skillid,skilllv));
		break;

	case NPC_PETRIFYATTACK:
		sc_start4(bl,status_skill2sc(skillid),50+10*skilllv,
			skilllv,0,0,skill_get_time(skillid,skilllv),
			skill_get_time2(skillid,skilllv));
		break;
	case NPC_CURSEATTACK:
	case NPC_SLEEPATTACK:
	case NPC_BLINDATTACK:
	case NPC_POISON:
	case NPC_SILENCEATTACK:
	case NPC_STUNATTACK:
	case NPC_HELLPOWER:
		sc_start(bl,status_skill2sc(skillid),50+10*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case NPC_ACIDBREATH:
	case NPC_ICEBREATH:
		sc_start(bl,status_skill2sc(skillid),70,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case NPC_BLEEDING:
		sc_start(bl,SC_BLEEDING,(20*skilllv),skilllv,skill_get_time2(skillid,skilllv));
		break;
	case NPC_MENTALBREAKER:
	{	//Based on observations by Tharis, Mental Breaker should do SP damage
	  	//equal to Matk*skLevel.
		rate = sstatus->matk_min;
		if (rate < sstatus->matk_max)
			rate += rnd()%(sstatus->matk_max - sstatus->matk_min);
		rate*=skilllv;
		status_zap(bl, 0, rate);
		break;
	}
	// Equipment breaking monster skills [Celest]
	case NPC_WEAPONBRAKER:
		skill_break_equip(bl, EQP_WEAPON, 150*skilllv, BCT_ENEMY);
		break;
	case NPC_ARMORBRAKE:
		skill_break_equip(bl, EQP_ARMOR, 150*skilllv, BCT_ENEMY);
		break;
	case NPC_HELMBRAKE:
		skill_break_equip(bl, EQP_HELM, 150*skilllv, BCT_ENEMY);
		break;
	case NPC_SHIELDBRAKE:
		skill_break_equip(bl, EQP_SHIELD, 150*skilllv, BCT_ENEMY);
		break;

	case CH_TIGERFIST:
		sc_start(bl,SC_STOP,(10+skilllv*10),0,skill_get_time2(skillid,skilllv));
		break;

	case LK_SPIRALPIERCE:
	case ML_SPIRALPIERCE:
		sc_start(bl,SC_STOP,(15+skilllv*5),0,skill_get_time2(skillid,skilllv));
		break;

	case ST_REJECTSWORD:
		sc_start(bl,SC_AUTOCOUNTER,(skilllv*15),skilllv,skill_get_time(skillid,skilllv));
		break;

	case PF_FOGWALL:
		if (src != bl && !tsc->data[SC_DELUGE])
			status_change_start(bl,SC_BLIND,10000,skilllv,0,0,0,skill_get_time2(skillid,skilllv),8);
		break;

	case LK_HEADCRUSH: //Headcrush has chance of causing Bleeding status, except on demon and undead element
		if (!(battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON))
			sc_start(bl, SC_BLEEDING,50, skilllv, skill_get_time2(skillid,skilllv));
		break;

	case LK_JOINTBEAT:
		status = status_skill2sc(skillid);
		if (tsc->jb_flag) {
			sc_start2(bl,status,(5*skilllv+5),skilllv,tsc->jb_flag&BREAK_FLAGS,skill_get_time2(skillid,skilllv));
			tsc->jb_flag = 0;
		}
		break;
	case ASC_METEORASSAULT:
		//Any enemies hit by this skill will receive Stun, Darkness, or external bleeding status ailment with a 5%+5*SkillLV% chance.
		switch(rnd()%3) {
			case 0:
				sc_start(bl,SC_BLIND,(5+skilllv*5),skilllv,skill_get_time2(skillid,1));
				break;
			case 1:
				sc_start(bl,SC_STUN,(5+skilllv*5),skilllv,skill_get_time2(skillid,2));
				break;
			default:
				sc_start(bl,SC_BLEEDING,(5+skilllv*5),skilllv,skill_get_time2(skillid,3));
  		}
		break;

	case HW_NAPALMVULCAN:
		sc_start(bl,SC_CURSE,5*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case WS_CARTTERMINATION:	// Cart termination
		sc_start(bl,SC_STUN,5*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case CR_ACIDDEMONSTRATION:
		skill_break_equip(bl, EQP_WEAPON|EQP_ARMOR, 100*skilllv, BCT_ENEMY);
		break;

	case TK_DOWNKICK:
		sc_start(bl,SC_STUN,100,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case TK_JUMPKICK:
		if( dstsd && dstsd->class_ != MAPID_SOUL_LINKER && !tsc->data[SC_PRESERVE] )
		{// debuff the following statuses
			status_change_end(bl, SC_SPIRIT, INVALID_TIMER);
			status_change_end(bl, SC_ADRENALINE2, INVALID_TIMER);
			status_change_end(bl, SC_KAITE, INVALID_TIMER);
			status_change_end(bl, SC_KAAHI, INVALID_TIMER);
			status_change_end(bl, SC_ONEHAND, INVALID_TIMER);
			status_change_end(bl, SC_ASPDPOTION2, INVALID_TIMER);
		}
		break;
	case TK_TURNKICK:
	case MO_BALKYOUNG: //Note: attack_type is passed as BF_WEAPON for the actual target, BF_MISC for the splash-affected mobs.
		if(attack_type&BF_MISC) //70% base stun chance...
			sc_start(bl,SC_STUN,70,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case GS_BULLSEYE: //0.1% coma rate.
		if(tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN)
			status_change_start(bl,SC_COMA,10,skilllv,0,src->id,0,0,0);
		break;
	case GS_PIERCINGSHOT:
		sc_start(bl,SC_BLEEDING,(skilllv*3),skilllv,skill_get_time2(skillid,skilllv));
		break;
	case NJ_HYOUSYOURAKU:
		sc_start(bl,SC_FREEZE,(10+10*skilllv),skilllv,skill_get_time2(skillid,skilllv));
		break;
	case GS_FLING:
		sc_start(bl,SC_FLING,100, sd?sd->spiritball_old:5,skill_get_time(skillid,skilllv));
		break;
	case GS_DISARM:
		rate = 3*skilllv;
		if (sstatus->dex > tstatus->dex)
			rate += (sstatus->dex - tstatus->dex)/5; //TODO: Made up formula
		skill_strip_equip(bl, EQP_WEAPON, rate, skilllv, skill_get_time(skillid,skilllv));
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case NPC_EVILLAND:
		sc_start(bl,SC_BLIND,5*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case NPC_HELLJUDGEMENT:
		sc_start(bl,SC_CURSE,100,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case NPC_CRITICALWOUND:
		sc_start(bl,SC_CRITICALWOUND,100,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case RK_HUNDREDSPEAR:
		if( !sd || pc_checkskill(sd,KN_SPEARBOOMERANG) == 0 )
			break; // Spear Boomerang auto cast chance only works if you have mastered Spear Boomerang.
		rate = 10 + 3 * skilllv;
		if( rnd()%100 < rate )
			skill_castend_damage_id(src,bl,KN_SPEARBOOMERANG,1,tick,0);
		break;
	case RK_WINDCUTTER:
		sc_start(bl,SC_FEAR,3+2*skilllv,skilllv,skill_get_time(skillid,skilllv));
		break;
	case RK_DRAGONBREATH:
		sc_start4(bl,SC_BURNING,5+5*skilllv,skilllv,1000,src->id,0,skill_get_time(skillid,skilllv));
		break;
	case AB_ADORAMUS:
		if( tsc && !tsc->data[SC_DECREASEAGI] ) //Prevent duplicate agi-down effect.
			sc_start(bl, SC_ADORAMUS, 100, skilllv, skill_get_time(skillid, skilllv));
		break;
	case WL_CRIMSONROCK:
		sc_start(bl, SC_STUN, 40, skilllv, skill_get_time(skillid, skilllv));
		break;
	case WL_COMET:
		sc_start4(bl,SC_BURNING,100,skilllv,1000,src->id,0,skill_get_time(skillid,skilllv));
		break;
	case WL_EARTHSTRAIN:
		{
			int rate = 0, i;
			const int pos[5] = { EQP_WEAPON, EQP_HELM, EQP_SHIELD, EQP_ARMOR, EQP_ACC };
			rate = 6 * skilllv + sstatus->dex / 10 + (sd? sd->status.job_level / 4 : 0) - tstatus->dex /5;// The tstatus->dex / 5 part is unofficial, but players gotta have some kind of way to have resistance. [Rytech]
			//rate -= rate * tstatus->dex / 200; // Disabled until official resistance is found.

			for( i = 0; i < skilllv; i++ )
				skill_strip_equip(bl,pos[i],rate,skilllv,skill_get_time2(skillid,skilllv));
		}
		break;
	case WL_JACKFROST:
		sc_start(bl,SC_FREEZE,100,skilllv,skill_get_time(skillid,skilllv));
		break;
	case RA_WUGBITE:
		sc_start(bl, SC_BITE,  (sd ? pc_checkskill(sd,RA_TOOTHOFWUG)*2 : 0), skilllv, (skill_get_time(skillid,skilllv) + (sd ? pc_checkskill(sd,RA_TOOTHOFWUG)*500 : 0)) );
		break;
	case RA_SENSITIVEKEEN:
		if( rnd()%100 < 8 * skilllv )
			skill_castend_damage_id(src, bl, RA_WUGBITE, sd ? pc_checkskill(sd, RA_WUGBITE):skilllv, tick, SD_ANIMATION);
		break;
	case RA_FIRINGTRAP:
	case RA_ICEBOUNDTRAP:
		sc_start(bl, (skillid == RA_FIRINGTRAP) ? SC_BURNING:SC_FREEZING, 40 + 10 * skilllv, skilllv, skill_get_time2(skillid, skilllv));
		break;
	case NC_PILEBUNKER:
		if( rnd()%100 < 5 + 15*skilllv )
		{ //Deactivatable Statuses: Kyrie Eleison, Auto Guard, Steel Body, Assumptio, and Millennium Shield
			status_change_end(bl, SC_KYRIE, INVALID_TIMER);
			status_change_end(bl, SC_AUTOGUARD, INVALID_TIMER);
			status_change_end(bl, SC_STEELBODY, INVALID_TIMER);
			status_change_end(bl, SC_ASSUMPTIO, INVALID_TIMER);
			status_change_end(bl, SC_MILLENNIUMSHIELD, INVALID_TIMER);
		}
		break;
	case NC_FLAMELAUNCHER:
		sc_start4(bl, SC_BURNING, 50 + 10 * skilllv, skilllv, 1000, src->id, 0, skill_get_time2(skillid, skilllv));
		break;
	case NC_COLDSLOWER:
		sc_start(bl, SC_FREEZE, 10 * skilllv, skilllv, skill_get_time(skillid, skilllv));
		sc_start(bl, SC_FREEZING, 20 + 10 * skilllv, skilllv, skill_get_time(skillid, skilllv));
		break;
	case NC_POWERSWING:
		sc_start(bl, SC_STUN, 5*skilllv, skilllv, skill_get_time(skillid, skilllv));
		if( rnd()%100 < 5*skilllv )
			skill_castend_damage_id(src, bl, NC_AXEBOOMERANG, pc_checkskill(sd, NC_AXEBOOMERANG), tick, 1);
		break;
	case GC_WEAPONCRUSH:
		skill_castend_nodamage_id(src,bl,skillid,skilllv,tick,BCT_ENEMY);
		break;
	case LG_SHIELDPRESS:
		sc_start(bl, SC_STUN, 30 + 8 * skilllv, skilllv, skill_get_time(skillid,skilllv));
		break;
	case LG_PINPOINTATTACK:
		rate = 30 + (((5 * (sd?pc_checkskill(sd,LG_PINPOINTATTACK):skilllv)) + (sstatus->agi + status_get_lv(src))) / 10);
		switch( skilllv ) {
			case 1:
				sc_start(bl,SC_BLEEDING,rate,skilllv,skill_get_time(skillid,skilllv));
				break;
			case 2:
				if( dstsd && dstsd->spiritball && rnd()%100 < rate )
					pc_delspiritball(dstsd, dstsd->spiritball, 0);
				break;
			default:
				skill_break_equip(bl,(skilllv == 3) ? EQP_SHIELD : (skilllv == 4) ? EQP_ARMOR : EQP_WEAPON,rate * 100,BCT_ENEMY);
				break;
		}
		break;
	case LG_MOONSLASHER:
		rate = 32 + 8 * skilllv;
		if( rnd()%100 < rate && dstsd ) // Uses skill_addtimerskill to avoid damage and setsit packet overlaping. Officially clif_setsit is received about 500 ms after damage packet.
			skill_addtimerskill(src,tick+500,bl->id,0,0,skillid,skilllv,BF_WEAPON,0);
		else if( dstmd && !is_boss(bl) )
			sc_start(bl,SC_STOP,100,skilllv,skill_get_time(skillid,skilllv));
		break;
	case LG_RAYOFGENESIS:	// 50% chance to cause Blind on Undead and Demon monsters.
		if ( battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON )
			sc_start(bl, SC_BLIND,50, skilllv, skill_get_time(skillid,skilllv));
		break;
	case LG_EARTHDRIVE:
		skill_break_equip(src, EQP_SHIELD, 500, BCT_SELF);
		sc_start(bl, SC_EARTHDRIVE, 100, skilllv, skill_get_time(skillid, skilllv));
		break;
	case SR_DRAGONCOMBO:
		sc_start(bl, SC_STUN, 1 + skilllv, skilllv, skill_get_time(skillid, skilllv));
		break;
	case SR_FALLENEMPIRE:
		sc_start(bl, SC_STOP, 100, skilllv, skill_get_time(skillid, skilllv));
		break;
	case SR_WINDMILL:
		if( dstsd )
			skill_addtimerskill(src,tick+status_get_amotion(src),bl->id,0,0,skillid,skilllv,BF_WEAPON,0);
		else if( dstmd && !is_boss(bl) )
			sc_start(bl, SC_STUN, 100, skilllv, 1000 + 1000 * (rnd() %3));
		break;
	case SR_GENTLETOUCH_QUIET:  //  [(Skill Level x 5) + (Caster痴 DEX + Caster痴 Base Level) / 10]
		sc_start(bl, SC_SILENCE, 5 * skilllv + (sstatus->dex + status_get_lv(src)) / 10, skilllv, skill_get_time(skillid, skilllv));
		break;
	case SR_EARTHSHAKER:
		sc_start(bl,SC_STUN, 25 + 5 * skilllv,skilllv,skill_get_time(skillid,skilllv));
		break;
	case SR_HOWLINGOFLION:
		sc_start(bl, SC_FEAR, 5 + 5 * skilllv, skilllv, skill_get_time(skillid, skilllv));
		break;
	case WM_SOUND_OF_DESTRUCTION:
		if( rnd()%100 < 5 + 5 * skilllv ) { // Temporarly Check Until We Get the Official Formula
			status_change_end(bl, SC_DANCING, INVALID_TIMER);
			status_change_end(bl, SC_RICHMANKIM, INVALID_TIMER);
			status_change_end(bl, SC_ETERNALCHAOS, INVALID_TIMER);
			status_change_end(bl, SC_DRUMBATTLE, INVALID_TIMER);
			status_change_end(bl, SC_NIBELUNGEN, INVALID_TIMER);
			status_change_end(bl, SC_INTOABYSS, INVALID_TIMER);
			status_change_end(bl, SC_SIEGFRIED, INVALID_TIMER);
			status_change_end(bl, SC_WHISTLE, INVALID_TIMER);
			status_change_end(bl, SC_ASSNCROS, INVALID_TIMER);
			status_change_end(bl, SC_POEMBRAGI, INVALID_TIMER);
			status_change_end(bl, SC_APPLEIDUN, INVALID_TIMER);
			status_change_end(bl, SC_HUMMING, INVALID_TIMER);
			status_change_end(bl, SC_FORTUNE, INVALID_TIMER);
			status_change_end(bl, SC_SERVICE4U, INVALID_TIMER);
			status_change_end(bl, SC_LONGING, INVALID_TIMER);
			status_change_end(bl, SC_SWINGDANCE, INVALID_TIMER);
			status_change_end(bl, SC_SYMPHONYOFLOVER, INVALID_TIMER);
			status_change_end(bl, SC_MOONLITSERENADE, INVALID_TIMER);
			status_change_end(bl, SC_RUSHWINDMILL, INVALID_TIMER);
			status_change_end(bl, SC_ECHOSONG, INVALID_TIMER);
			status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
			status_change_end(bl, SC_WINKCHARM, INVALID_TIMER);
			status_change_end(bl, SC_SONGOFMANA, INVALID_TIMER);
			status_change_end(bl, SC_DANCEWITHWUG, INVALID_TIMER);
			status_change_end(bl, SC_LERADSDEW, INVALID_TIMER);
			status_change_end(bl, SC_MELODYOFSINK, INVALID_TIMER);
			status_change_end(bl, SC_BEYONDOFWARCRY, INVALID_TIMER);
			status_change_end(bl, SC_UNLIMITEDHUMMINGVOICE, INVALID_TIMER);
		}
		break;
	case SO_EARTHGRAVE:
		sc_start(bl, SC_BLEEDING, 5 * skilllv, skilllv, skill_get_time2(skillid, skilllv));	// Need official rate. [LimitLine]
		break;
	case SO_DIAMONDDUST:
		rate = 5 + 5 * skilllv;
		if( sc && sc->data[SC_COOLER_OPTION] )
			rate += rate * sc->data[SC_COOLER_OPTION]->val2 / 100;
		sc_start(bl, SC_CRYSTALIZE, rate, skilllv, skill_get_time2(skillid, skilllv));
		break;
	case SO_VARETYR_SPEAR:
		sc_start(bl, SC_STUN, 5 + 5 * skilllv, skilllv, skill_get_time2(skillid, skilllv));
		break;
	case GN_SLINGITEM_RANGEMELEEATK:
		if( sd ) {
			switch( sd->itemid ) {	// Starting SCs here instead of do it in skill_additional_effect to simplify the code.
				case 13261:
					sc_start(bl, SC_STUN, 100, skilllv, skill_get_time2(GN_SLINGITEM, skilllv));
					sc_start(bl, SC_BLEEDING, 100, skilllv, skill_get_time2(GN_SLINGITEM, skilllv));
					break;
				case 13262:					
					sc_start(bl, SC_MELON_BOMB, 100, skilllv, skill_get_time(GN_SLINGITEM, skilllv));	// Reduces ASPD and moviment speed
					break;
				case 13264:
					sc_start(bl, SC_BANANA_BOMB, 100, skilllv, skill_get_time(GN_SLINGITEM, skilllv));	// Reduces LUK ﾀ?Needed confirm it, may be it's bugged in kRORE?
					sc_start(bl, SC_BANANA_BOMB_SITDOWN, 75, skilllv, skill_get_time(GN_SLINGITEM_RANGEMELEEATK,skilllv)); // Sitdown for 3 seconds.
					break;
			}
			sd->itemid = -1;
		}
		break;
	case GN_HELLS_PLANT_ATK:
		sc_start(bl, SC_STUN,  5 + 5 * skilllv, skilllv, skill_get_time2(skillid, skilllv));
		sc_start(bl, SC_BLEEDING, 20 + 10 * skilllv, skilllv, skill_get_time2(skillid, skilllv));
		break;
	case EL_WIND_SLASH:	// Non confirmed rate.
		sc_start(bl, SC_BLEEDING, 25, skilllv, skill_get_time(skillid,skilllv));
		break;
	case EL_STONE_HAMMER:
		rate = 10 * skilllv;
		sc_start(bl, SC_STUN, rate, skilllv, skill_get_time(skillid,skilllv));
		break;
	case EL_ROCK_CRUSHER:
	case EL_ROCK_CRUSHER_ATK:
		sc_start(bl,status_skill2sc(skillid),50,skilllv,skill_get_time(EL_ROCK_CRUSHER,skilllv));
		break;
	case EL_TYPOON_MIS:
		sc_start(bl,SC_SILENCE,10*skilllv,skilllv,skill_get_time(skillid,skilllv));
		break;
	case MH_LAVA_SLIDE:
		sc_start4(bl,SC_BURNING,10*skilllv,skilllv,1000,src->id,0,skill_get_time(skillid,skilllv));
		break;			
	case MH_STAHL_HORN:
		sc_start(bl,SC_STUN,(20 + 4 * skilllv),skilllv,skill_get_time2(skillid,skilllv));
		break;
	case KO_JYUMONJIKIRI: // needs more info
		sc_start(bl,SC_JYUMONJIKIRI,25,skilllv,skill_get_time(skillid,skilllv));
		break;
	case KO_MAKIBISHI:
		sc_start(bl, SC_STUN, 100, skilllv, skill_get_time2(skillid,skilllv)); 
		break;
	}

	if (md && battle_config.summons_trigger_autospells && md->master_id && md->special_state.ai)
	{	//Pass heritage to Master for status causing effects. [Skotlex]
		sd = map_id2sd(md->master_id);
		src = sd?&sd->bl:src;
	}

	if( attack_type&BF_WEAPON )
	{ // Coma, Breaking Equipment
		if( sd && sd->special_state.bonus_coma )
		{
			rate  = sd->weapon_coma_ele[tstatus->def_ele];
			rate += sd->weapon_coma_race[tstatus->race];
			rate += sd->weapon_coma_race[tstatus->mode&MD_BOSS?RC_BOSS:RC_NONBOSS];
			if (rate)
				status_change_start(bl, SC_COMA, rate, 0, 0, src->id, 0, 0, 0);
		}
		if( sd && battle_config.equip_self_break_rate )
		{	// Self weapon breaking
			rate = battle_config.equip_natural_break_rate;
			if( sc )
			{
				if(sc->data[SC_OVERTHRUST])
					rate += 10;
				if(sc->data[SC_MAXOVERTHRUST])
					rate += 10;
			}
			if( rate )
				skill_break_equip(src, EQP_WEAPON, rate, BCT_SELF);
		}
		if( battle_config.equip_skill_break_rate && skillid != WS_CARTTERMINATION && skillid != ITM_TOMAHAWK )
		{	// Cart Termination/Tomahawk won't trigger breaking data. Why? No idea, go ask Gravity.
			// Target weapon breaking
			rate = 0;
			if( sd )
				rate += sd->bonus.break_weapon_rate;
			if( sc && sc->data[SC_MELTDOWN] )
				rate += sc->data[SC_MELTDOWN]->val2;
			if( rate )
				skill_break_equip(bl, EQP_WEAPON, rate, BCT_ENEMY);

			// Target armor breaking
			rate = 0;
			if( sd )
				rate += sd->bonus.break_armor_rate;
			if( sc && sc->data[SC_MELTDOWN] )
				rate += sc->data[SC_MELTDOWN]->val3;
			if( rate )
				skill_break_equip(bl, EQP_ARMOR, rate, BCT_ENEMY);
		}
	}

	// Autospell when attacking
	if( sd && !status_isdead(bl) && sd->autospell[0].id )
	{
		struct block_list *tbl;
		struct unit_data *ud;
		int i, skilllv, type, notok;

		for (i = 0; i < ARRAYLENGTH(sd->autospell) && sd->autospell[i].id; i++) {

			if(!(sd->autospell[i].flag&attack_type&BF_WEAPONMASK &&
				 sd->autospell[i].flag&attack_type&BF_RANGEMASK &&
				 sd->autospell[i].flag&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled

			skill = (sd->autospell[i].id > 0) ? sd->autospell[i].id : -sd->autospell[i].id;

			sd->state.autocast = 1;
			notok = skillnotok(skill, sd);
			sd->state.autocast = 0;

			if ( notok )
				continue;

			skilllv = sd->autospell[i].lv?sd->autospell[i].lv:1;
			if (skilllv < 0) skilllv = 1+rnd()%(-skilllv);

			rate = (!sd->state.arrow_atk) ? sd->autospell[i].rate : sd->autospell[i].rate / 2;

			if (rnd()%1000 >= rate)
				continue;

			tbl = (sd->autospell[i].id < 0) ? src : bl;

			if( (type = skill_get_casttype(skill)) == CAST_GROUND ) {
				int maxcount = 0;
				if( !(BL_PC&battle_config.skill_reiteration) &&
					skill_get_unit_flag(skill)&UF_NOREITERATION &&
					skill_check_unit_range(src,tbl->x,tbl->y,skill,skilllv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.skill_nofootset &&
					skill_get_unit_flag(skill)&UF_NOFOOTSET &&
					skill_check_unit_range2(src,tbl->x,tbl->y,skill,skilllv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.land_skill_limit &&
					(maxcount = skill_get_maxcount(skill, skilllv)) > 0
				  ) {
					int v;
					for(v=0;v<MAX_SKILLUNITGROUP && sd->ud.skillunit[v] && maxcount;v++) {
						if(sd->ud.skillunit[v]->skill_id == skill)
							maxcount--;
					}
					if( maxcount == 0 ) {
						continue;
					}
				}
			}
			if( battle_config.autospell_check_range &&
				!battle_check_range(src, tbl, skill_get_range2(src, skill,skilllv) + (skill == RG_CLOSECONFINE?0:1)) )
				continue;

			if (skill == AS_SONICBLOW)
				pc_stop_attack(sd); //Special case, Sonic Blow autospell should stop the player attacking.
			if (skill == PF_SPIDERWEB) //Special case, due to its nature of coding.
				type = CAST_GROUND;

			sd->state.autocast = 1;
			skill_consume_requirement(sd,skill,skilllv,1);
			skill_toggle_magicpower(src, skill);
			switch (type) {
				case CAST_GROUND:
					skill_castend_pos2(src, tbl->x, tbl->y, skill, skilllv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill_castend_nodamage_id(src, tbl, skill, skilllv, tick, 0);
					break;
				case CAST_DAMAGE:
					skill_castend_damage_id(src, tbl, skill, skilllv, tick, 0);
					break;
			}
			sd->state.autocast = 0;
			//Set canact delay. [Skotlex]
			ud = unit_bl2ud(src);
			if (ud) {
				rate = skill_delayfix(src, skill, skilllv);
				if (DIFF_TICK(ud->canact_tick, tick + rate) < 0){
					ud->canact_tick = tick+rate;
					if ( battle_config.display_status_timers && sd )
						clif_status_change(src, SI_ACTIONDELAY, 1, rate, 0, 0, 0);
				}
			}
		}
	}

	//Autobonus when attacking
	if( sd && sd->autobonus[0].rate )
	{
		int i;
		for( i = 0; i < ARRAYLENGTH(sd->autobonus); i++ )
		{
			if( rnd()%1000 >= sd->autobonus[i].rate )
				continue;
			if( sd->autobonus[i].active != INVALID_TIMER )
				continue;
			if(!(sd->autobonus[i].atk_type&attack_type&BF_WEAPONMASK &&
				 sd->autobonus[i].atk_type&attack_type&BF_RANGEMASK &&
				 sd->autobonus[i].atk_type&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled
			pc_exeautobonus(sd,&sd->autobonus[i]);
		}
	}

	//Polymorph
	if(sd && sd->bonus.classchange && attack_type&BF_WEAPON &&
		dstmd && !(tstatus->mode&MD_BOSS) &&
		(rnd()%10000 < sd->bonus.classchange))
	{
		struct mob_db *mob;
		int class_;
		skill = 0;
		do {
			do {
				class_ = rnd() % MAX_MOB_DB;
			} while (!mobdb_checkid(class_));

			rate = rnd() % 1000000;
			mob = mob_db(class_);
		} while (
			(mob->status.mode&(MD_BOSS|MD_PLANT) || mob->summonper[0] <= rate) &&
		  	(skill++) < 2000);
		if (skill < 2000)
			mob_class_change(dstmd,class_);
	}

	return 0;
}

int skill_onskillusage(struct map_session_data *sd, struct block_list *bl, int skillid, unsigned int tick) {
	int skill, skilllv, i, type, notok;
	struct block_list *tbl;

	if( sd == NULL || skillid <= 0 )
		return 0;

	for( i = 0; i < ARRAYLENGTH(sd->autospell3) && sd->autospell3[i].flag; i++ ) {
		if( sd->autospell3[i].flag != skillid )
			continue;

		if( sd->autospell3[i].lock )
			continue;  // autospell already being executed

		skill = (sd->autospell3[i].id > 0) ? sd->autospell3[i].id : -sd->autospell3[i].id;

		sd->state.autocast = 1;
		notok = skillnotok(skill, sd);
		sd->state.autocast = 0;

		if ( notok )
			continue;

		skilllv = sd->autospell3[i].lv ? sd->autospell3[i].lv : 1;
		if( skilllv < 0 ) skilllv = 1 + rnd()%(-skilllv);

		if( sd->autospell3[i].id >= 0 && bl == NULL )
			continue; // No target
		if( rnd()%1000 >= sd->autospell3[i].rate )
			continue;

		tbl = (sd->autospell3[i].id < 0) ? &sd->bl : bl;

		if( (type = skill_get_casttype(skill)) == CAST_GROUND ) {
			int maxcount = 0;
			if( !(BL_PC&battle_config.skill_reiteration) &&
				skill_get_unit_flag(skill)&UF_NOREITERATION &&
				skill_check_unit_range(&sd->bl,tbl->x,tbl->y,skill,skilllv)
			  ) {
				continue;
			}
			if( BL_PC&battle_config.skill_nofootset &&
				skill_get_unit_flag(skill)&UF_NOFOOTSET &&
				skill_check_unit_range2(&sd->bl,tbl->x,tbl->y,skill,skilllv)
			  ) {
				continue;
			}
			if( BL_PC&battle_config.land_skill_limit &&
				(maxcount = skill_get_maxcount(skill, skilllv)) > 0
			  ) {
				int v;
				for(v=0;v<MAX_SKILLUNITGROUP && sd->ud.skillunit[v] && maxcount;v++) {
					if(sd->ud.skillunit[v]->skill_id == skill)
						maxcount--;
				}
				if( maxcount == 0 ) {
					continue;
				}
			}
		}
		if( battle_config.autospell_check_range &&
			!battle_check_range(&sd->bl, tbl, skill_get_range2(&sd->bl, skill,skilllv) + (skill == RG_CLOSECONFINE?0:1)) )
			continue;

		sd->state.autocast = 1;
		sd->autospell3[i].lock = true;
		skill_consume_requirement(sd,skill,skilllv,1);
		switch( type )
		{
			case CAST_GROUND:   skill_castend_pos2(&sd->bl, tbl->x, tbl->y, skill, skilllv, tick, 0); break;
			case CAST_NODAMAGE: skill_castend_nodamage_id(&sd->bl, tbl, skill, skilllv, tick, 0); break;
			case CAST_DAMAGE:   skill_castend_damage_id(&sd->bl, tbl, skill, skilllv, tick, 0); break;
		}
		sd->autospell3[i].lock = false;
		sd->state.autocast = 0;
	}

	if( sd && sd->autobonus3[0].rate )
	{
		for( i = 0; i < ARRAYLENGTH(sd->autobonus3); i++ )
		{
			if( rnd()%1000 >= sd->autobonus3[i].rate )
				continue;
			if( sd->autobonus3[i].active != INVALID_TIMER )
				continue;
			if( sd->autobonus3[i].atk_type != skillid )
				continue;
			pc_exeautobonus(sd,&sd->autobonus3[i]);
		}
	}

	return 1;
}

/* Splitted off from skill_additional_effect, which is never called when the
 * attack skill kills the enemy. Place in this function counter status effects
 * when using skills (eg: Asura's sp regen penalty, or counter-status effects
 * from cards) that will take effect on the source, not the target. [Skotlex]
 * Note: Currently this function only applies to Extremity Fist and BF_WEAPON
 * type of skills, so not every instance of skill_additional_effect needs a call
 * to this one.
 */
int skill_counter_additional_effect (struct block_list* src, struct block_list *bl, int skillid, int skilllv, int attack_type, unsigned int tick)
{
	int rate;
	struct map_session_data *sd=NULL;
	struct map_session_data *dstsd=NULL;

	nullpo_ret(src);
	nullpo_ret(bl);

	if(skillid < 0) return 0;
	if(skillid > 0 && skilllv <= 0) return 0;	// don't forget auto attacks! - celest

	sd = BL_CAST(BL_PC, src);
	dstsd = BL_CAST(BL_PC, bl);

	if(dstsd && attack_type&BF_WEAPON)
	{	//Counter effects.
		enum sc_type type;
		int i, time;
		for(i=0; i < ARRAYLENGTH(dstsd->addeff2) && dstsd->addeff2[i].flag; i++)
		{
			rate = dstsd->addeff2[i].rate;
			if (attack_type&BF_LONG)
				rate+=dstsd->addeff2[i].arrow_rate;
			if (!rate) continue;

			if ((dstsd->addeff2[i].flag&(ATF_LONG|ATF_SHORT)) != (ATF_LONG|ATF_SHORT))
			{	//Trigger has range consideration.
				if((dstsd->addeff2[i].flag&ATF_LONG && !(attack_type&BF_LONG)) ||
					(dstsd->addeff2[i].flag&ATF_SHORT && !(attack_type&BF_SHORT)))
					continue; //Range Failed.
			}
			type = dstsd->addeff2[i].id;
			time = skill_get_time2(status_sc2skill(type),7);

			if (dstsd->addeff2[i].flag&ATF_TARGET)
				status_change_start(src,type,rate,7,0,0,0,time,0);

			if (dstsd->addeff2[i].flag&ATF_SELF && !status_isdead(bl))
				status_change_start(bl,type,rate,7,0,0,0,time,0);
		}
	}

	switch(skillid){
	case MO_EXTREMITYFIST:
		sc_start(src,SC_EXTREMITYFIST,100,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case GS_FULLBUSTER:
		sc_start(src,SC_BLIND,2*skilllv,skilllv,skill_get_time2(skillid,skilllv));
		break;
	case HFLI_SBR44:	//[orn]
	case HVAN_EXPLOSION:
		if(src->type == BL_HOM){
			TBL_HOM *hd = (TBL_HOM*)src;
			hd->homunculus.intimacy = 200;
			if (hd->master)
				clif_send_homdata(hd->master,SP_INTIMATE,hd->homunculus.intimacy/100);
		}
		break;
	case CR_GRANDCROSS:
	case NPC_GRANDDARKNESS:
		attack_type |= BF_WEAPON;
		break;
	}

	if(sd && (sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR &&
		rnd()%10000 < battle_config.sg_miracle_skill_ratio)	//SG_MIRACLE [Komurka]
		sc_start(src,SC_MIRACLE,100,1,battle_config.sg_miracle_skill_duration);

	if(sd && skillid && attack_type&BF_MAGIC && status_isdead(bl) &&
	 	!(skill_get_inf(skillid)&(INF_GROUND_SKILL|INF_SELF_SKILL)) &&
		(rate=pc_checkskill(sd,HW_SOULDRAIN))>0
	){	//Soul Drain should only work on targetted spells [Skotlex]
		if (pc_issit(sd)) pc_setstand(sd); //Character stuck in attacking animation while 'sitting' fix. [Skotlex]
		clif_skill_nodamage(src,bl,HW_SOULDRAIN,rate,1);
		status_heal(src, 0, status_get_lv(bl)*(95+15*rate)/100, 2);
	}

	if( sd && status_isdead(bl) ) {
		int sp = 0, hp = 0;
		if( attack_type&BF_WEAPON ) {
			sp += sd->bonus.sp_gain_value;
			sp += sd->sp_gain_race[status_get_race(bl)];
			sp += sd->sp_gain_race[is_boss(bl)?RC_BOSS:RC_NONBOSS];
			hp += sd->bonus.hp_gain_value;
		}
		if( attack_type&BF_MAGIC ) {
			sp += sd->bonus.magic_sp_gain_value;
			hp += sd->bonus.magic_hp_gain_value;
			if( skillid == WZ_WATERBALL ) {//(bugreport:5303)
				struct status_change *sc = NULL;
				if( ( sc = status_get_sc(src) ) ) {
					if(sc->data[SC_SPIRIT] &&
								sc->data[SC_SPIRIT]->val2 == SL_WIZARD &&
								sc->data[SC_SPIRIT]->val3 == WZ_WATERBALL)
								sc->data[SC_SPIRIT]->val3 = 0; //Clear bounced spell check.
				}
			}
		}
		if( hp || sp ) { // updated to force healing to allow healing through berserk
			status_heal(src, hp, sp, battle_config.show_hp_sp_gain ? 3 : 1);
		}
	}

	// Trigger counter-spells to retaliate against damage causing skills.
	if(dstsd && !status_isdead(bl) && dstsd->autospell2[0].id &&
		!(skillid && skill_get_nk(skillid)&NK_NO_DAMAGE))
	{
		struct block_list *tbl;
		struct unit_data *ud;
		int i, skillid, skilllv, rate, type, notok;

		for (i = 0; i < ARRAYLENGTH(dstsd->autospell2) && dstsd->autospell2[i].id; i++) {

			if(!(dstsd->autospell2[i].flag&attack_type&BF_WEAPONMASK &&
				 dstsd->autospell2[i].flag&attack_type&BF_RANGEMASK &&
				 dstsd->autospell2[i].flag&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled

			skillid = (dstsd->autospell2[i].id > 0) ? dstsd->autospell2[i].id : -dstsd->autospell2[i].id;
			skilllv = dstsd->autospell2[i].lv?dstsd->autospell2[i].lv:1;
			if (skilllv < 0) skilllv = 1+rnd()%(-skilllv);

			rate = dstsd->autospell2[i].rate;
			if (attack_type&BF_LONG)
				 rate>>=1;

			dstsd->state.autocast = 1;
			notok = skillnotok(skillid, dstsd);
			dstsd->state.autocast = 0;

			if ( notok )
				continue;

			if (rnd()%1000 >= rate)
				continue;

			tbl = (dstsd->autospell2[i].id < 0) ? bl : src;

			if( (type = skill_get_casttype(skillid)) == CAST_GROUND ) {
				int maxcount = 0;
				if( !(BL_PC&battle_config.skill_reiteration) &&
					skill_get_unit_flag(skillid)&UF_NOREITERATION &&
					skill_check_unit_range(bl,tbl->x,tbl->y,skillid,skilllv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.skill_nofootset &&
					skill_get_unit_flag(skillid)&UF_NOFOOTSET &&
					skill_check_unit_range2(bl,tbl->x,tbl->y,skillid,skilllv)
				  ) {
					continue;
				}
				if( BL_PC&battle_config.land_skill_limit &&
					(maxcount = skill_get_maxcount(skillid, skilllv)) > 0
				  ) {
					int v;
					for(v=0;v<MAX_SKILLUNITGROUP && dstsd->ud.skillunit[v] && maxcount;v++) {
						if(dstsd->ud.skillunit[v]->skill_id == skillid)
							maxcount--;
					}
					if( maxcount == 0 ) {
						continue;
					}
				}
			}

			if( !battle_check_range(src, tbl, skill_get_range2(src, skillid,skilllv) + (skillid == RG_CLOSECONFINE?0:1)) && battle_config.autospell_check_range )
				continue;

			dstsd->state.autocast = 1;
			skill_consume_requirement(dstsd,skillid,skilllv,1);
			switch (type) {
				case CAST_GROUND:
					skill_castend_pos2(bl, tbl->x, tbl->y, skillid, skilllv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill_castend_nodamage_id(bl, tbl, skillid, skilllv, tick, 0);
					break;
				case CAST_DAMAGE:
					skill_castend_damage_id(bl, tbl, skillid, skilllv, tick, 0);
					break;
			}
			dstsd->state.autocast = 0;
			//Set canact delay. [Skotlex]
			ud = unit_bl2ud(bl);
			if (ud) {
				rate = skill_delayfix(bl, skillid, skilllv);
				if (DIFF_TICK(ud->canact_tick, tick + rate) < 0){
					ud->canact_tick = tick+rate;
					if ( battle_config.display_status_timers && dstsd )
						clif_status_change(bl, SI_ACTIONDELAY, 1, rate, 0, 0, 0);
				}
			}
		}
	}

	//Autobonus when attacked
	if( dstsd && !status_isdead(bl) && dstsd->autobonus2[0].rate && !(skillid && skill_get_nk(skillid)&NK_NO_DAMAGE) )
	{
		int i;
		for( i = 0; i < ARRAYLENGTH(dstsd->autobonus2); i++ )
		{
			if( rnd()%1000 >= dstsd->autobonus2[i].rate )
				continue;
			if( dstsd->autobonus2[i].active != INVALID_TIMER )
				continue;
			if(!(dstsd->autobonus2[i].atk_type&attack_type&BF_WEAPONMASK &&
				 dstsd->autobonus2[i].atk_type&attack_type&BF_RANGEMASK &&
				 dstsd->autobonus2[i].atk_type&attack_type&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled
			pc_exeautobonus(dstsd,&dstsd->autobonus2[i]);
		}
	}

	return 0;
}
/*=========================================================================
 Breaks equipment. On-non players causes the corresponding strip effect.
 - rate goes from 0 to 10000 (100.00%)
 - flag is a BCT_ flag to indicate which type of adjustment should be used
   (BCT_ENEMY/BCT_PARTY/BCT_SELF) are the valid values.
--------------------------------------------------------------------------*/
int skill_break_equip (struct block_list *bl, unsigned short where, int rate, int flag)
{
	const int where_list[4]     = {EQP_WEAPON, EQP_ARMOR, EQP_SHIELD, EQP_HELM};
	const enum sc_type scatk[4] = {SC_STRIPWEAPON, SC_STRIPARMOR, SC_STRIPSHIELD, SC_STRIPHELM};
	const enum sc_type scdef[4] = {SC_CP_WEAPON, SC_CP_ARMOR, SC_CP_SHIELD, SC_CP_HELM};
	struct status_change *sc = status_get_sc(bl);
	int i,j;
	TBL_PC *sd;
	sd = BL_CAST(BL_PC, bl);
	if (sc && !sc->count)
		sc = NULL;

	if (sd) {
		if (sd->bonus.unbreakable_equip)
			where &= ~sd->bonus.unbreakable_equip;
		if (sd->bonus.unbreakable)
			rate -= rate*sd->bonus.unbreakable/100;
		if (where&EQP_WEAPON) {
			switch (sd->status.weapon) {
				case W_FIST:	//Bare fists should not break :P
				case W_1HAXE:
				case W_2HAXE:
				case W_MACE: // Axes and Maces can't be broken [DracoRPG]
				case W_2HMACE:
				case W_STAFF:
				case W_2HSTAFF:
				case W_BOOK: //Rods and Books can't be broken [Skotlex]
				case W_HUUMA:
					where &= ~EQP_WEAPON;
			}
		}
	}
	if (flag&BCT_ENEMY) {
		if (battle_config.equip_skill_break_rate != 100)
			rate = rate*battle_config.equip_skill_break_rate/100;
	} else if (flag&(BCT_PARTY|BCT_SELF)) {
		if (battle_config.equip_self_break_rate != 100)
			rate = rate*battle_config.equip_self_break_rate/100;
	}

	for (i = 0; i < 4; i++) {
		if (where&where_list[i]) {
			if (sc && sc->count && sc->data[scdef[i]])
				where&=~where_list[i];
			else if (rnd()%10000 >= rate)
				where&=~where_list[i];
			else if (!sd && !(status_get_mode(bl)&MD_BOSS)) //Cause Strip effect.
				sc_start(bl,scatk[i],100,0,skill_get_time(status_sc2skill(scatk[i]),1));
		}
	}
	if (!where) //Nothing to break.
		return 0;
	if (sd) {
		for (i = 0; i < EQI_MAX; i++) {
			j = sd->equip_index[i];
			if (j < 0 || sd->status.inventory[j].attribute == 1 || !sd->inventory_data[j])
				continue;

			switch(i) {
				case EQI_HEAD_TOP: //Upper Head
					flag = (where&EQP_HELM);
					break;
				case EQI_ARMOR: //Body
					flag = (where&EQP_ARMOR);
					break;
				case EQI_HAND_R: //Left/Right hands
				case EQI_HAND_L:
					flag = (
						(where&EQP_WEAPON && sd->inventory_data[j]->type == IT_WEAPON) ||
						(where&EQP_SHIELD && sd->inventory_data[j]->type == IT_ARMOR));
					break;
				case EQI_SHOES:
					flag = (where&EQP_SHOES);
					break;
				case EQI_GARMENT:
					flag = (where&EQP_GARMENT);
					break;
				default:
					continue;
			}
			if (flag) {
				sd->status.inventory[j].attribute = 1;
				pc_unequipitem(sd, j, 3);
			}
		}
		clif_equiplist(sd);
	}

	return where; //Return list of pieces broken.
}

int skill_strip_equip(struct block_list *bl, unsigned short where, int rate, int lv, int time)
{
	struct status_change *sc;
	const int pos[5]             = {EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HELM, EQP_ACC};
	const enum sc_type sc_atk[5] = {SC_STRIPWEAPON, SC_STRIPSHIELD, SC_STRIPARMOR, SC_STRIPHELM, SC__STRIPACCESSORY};
	const enum sc_type sc_def[5] = {SC_CP_WEAPON, SC_CP_SHIELD, SC_CP_ARMOR, SC_CP_HELM, 0};
	int i;

	if (rnd()%100 >= rate)
		return 0;

	sc = status_get_sc(bl);
	if (!sc || sc->option&OPTION_MADOGEAR ) //Mado Gear cannot be divested [Ind]
		return 0;

	for (i = 0; i < ARRAYLENGTH(pos); i++) {
		if (where&pos[i] && sc->data[sc_def[i]])
			where&=~pos[i];
	}
	if (!where) return 0;

	for (i = 0; i < ARRAYLENGTH(pos); i++) {
		if (where&pos[i] && !sc_start(bl, sc_atk[i], 100, lv, time))
			where&=~pos[i];
	}
	return where?1:0;
}
//Early declaration
static int skill_area_temp[8];
/*=========================================================================
 Used to knock back players, monsters, traps, etc
 - 'count' is the number of squares to knock back
 - 'direction' indicates the way OPPOSITE to the knockback direction (or -1 for default behavior)
 - if 'flag&0x1', position update packets must not be sent.
 - if 'flag&0x2', skill blown ignores players' special_state.no_knockback
 -------------------------------------------------------------------------*/
int skill_blown(struct block_list* src, struct block_list* target, int count, int direction, int flag)
{
	int dx = 0, dy = 0;
	struct skill_unit* su = NULL;

	nullpo_ret(src);

	if (src != target && (map_flag_gvg(target->m) || map[target->m].flag.battleground))
		return 0; //No knocking back in WoE
	if (count == 0)
		return 0; //Actual knockback distance is 0.

	switch (target->type) {
		case BL_MOB: {
				struct mob_data* md = BL_CAST(BL_MOB, target);
				if( md->class_ == MOBID_EMPERIUM )
					return 0;
				if(src != target && is_boss(target)) //Bosses can't be knocked-back
					return 0;
			}
			break;
		case BL_PC: {
				struct map_session_data *sd = BL_CAST(BL_PC, target);
				if( sd->sc.data[SC_BASILICA] && sd->sc.data[SC_BASILICA]->val4 == sd->bl.id && !is_boss(src))
					return 0; // Basilica caster can't be knocked-back by normal monsters.
				if( !(flag&0x2) && src != target && sd->special_state.no_knockback )
					return 0;
			}
			break;
		case BL_SKILL:
			su = (struct skill_unit *)target;
			if( su && su->group && su->group->unit_id == UNT_ANKLESNARE )
				return 0; // ankle snare cannot be knocked back
			break;
	}
	
	if (direction == -1) // <optimized>: do the computation here instead of outside
		direction = map_calc_dir(target, src->x, src->y); // direction from src to target, reversed

	if (direction >= 0 && direction < 8)
	{	// take the reversed 'direction' and reverse it
		dx = -dirx[direction];
		dy = -diry[direction];
	}

	return unit_blown(target, dx, dy, count, flag);	// send over the proper flag
}


//Checks if 'bl' should reflect back a spell cast by 'src'.
//type is the type of magic attack: 0: indirect (aoe), 1: direct (targetted)
static int skill_magic_reflect(struct block_list* src, struct block_list* bl, int type)
{
	struct status_change *sc = status_get_sc(bl);
	struct map_session_data* sd = BL_CAST(BL_PC, bl);

	if( sc && sc->data[SC_KYOMU] ) // Nullify reflecting ability
		return  0;

	// item-based reflection
	if( sd && sd->bonus.magic_damage_return && type && rnd()%100 < sd->bonus.magic_damage_return )
		return 1;

	if( is_boss(src) )
		return 0;

	// status-based reflection
	if( !sc || sc->count == 0 )
		return 0;

	if( sc->data[SC_MAGICMIRROR] && rnd()%100 < sc->data[SC_MAGICMIRROR]->val2 )
		return 1;

	if( sc->data[SC_KAITE] && (src->type == BL_PC || status_get_lv(src) <= 80) )
	{// Kaite only works against non-players if they are low-level.
		clif_specialeffect(bl, 438, AREA);
		if( --sc->data[SC_KAITE]->val2 <= 0 )
			status_change_end(bl, SC_KAITE, INVALID_TIMER);
		return 2;
	}

	return 0;
}

/*
 * =========================================================================
 * Does a skill attack with the given properties.
 * src is the master behind the attack (player/mob/pet)
 * dsrc is the actual originator of the damage, can be the same as src, or a BL_SKILL
 * bl is the target to be attacked.
 * flag can hold a bunch of information:
 * flag&0xFFF is passed to the underlying battle_calc_attack for processing
 *      (usually holds number of targets, or just 1 for simple splash attacks)
 * flag&0x1000 is used to tag that this is a splash-attack (so the damage
 *      packet shouldn't display a skill animation)
 * flag&0x2000 is used to signal that the skilllv should be passed as -1 to the
 *      client (causes player characters to not scream skill name)
 *-------------------------------------------------------------------------*/
int skill_attack (int attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct Damage dmg;
	struct status_data *sstatus, *tstatus;
	struct status_change *sc;
	struct map_session_data *sd, *tsd;
	int type,damage,rdamage=0;

	if(skillid > 0 && skilllv <= 0) return 0;

	nullpo_ret(src);	//Source is the master behind the attack (player/mob/pet)
	nullpo_ret(dsrc); //dsrc is the actual originator of the damage, can be the same as src, or a skill casted by src.
	nullpo_ret(bl); //Target to be attacked.

	if (src != dsrc) {
		//When caster is not the src of attack, this is a ground skill, and as such, do the relevant target checking. [Skotlex]
		if (!status_check_skilluse(battle_config.skill_caster_check?src:NULL, bl, skillid, 2))
			return 0;
	} else if ((flag&SD_ANIMATION) && skill_get_nk(skillid)&NK_SPLASH) {
		//Note that splash attacks often only check versus the targetted mob, those around the splash area normally don't get checked for being hidden/cloaked/etc. [Skotlex]
		if (!status_check_skilluse(src, bl, skillid, 2))
			return 0;
	}

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, bl);

	sstatus = status_get_status_data(src);
	tstatus = status_get_status_data(bl);
	sc= status_get_sc(bl);
	if (sc && !sc->count) sc = NULL; //Don't need it.

	// Is this check really needed? FrostNova won't hurt you if you step right where the caster is?
	if(skillid == WZ_FROSTNOVA && dsrc->x == bl->x && dsrc->y == bl->y)
		return 0;
	 //Trick Dead protects you from damage, but not from buffs and the like, hence it's placed here.
	if (sc && sc->data[SC_TRICKDEAD] && !(sstatus->mode&MD_BOSS))
		return 0;

	dmg = battle_calc_attack(attack_type,src,bl,skillid,skilllv,flag&0xFFF);

	//Skotlex: Adjusted to the new system
	if(src->type==BL_PET)
	{ // [Valaris]
		struct pet_data *pd = (TBL_PET*)src;
		if (pd->a_skill && pd->a_skill->div_ && pd->a_skill->id == skillid)
		{
			int element = skill_get_ele(skillid, skilllv);
			if (skillid == -1)
				element = sstatus->rhw.ele;
			if (element != ELE_NEUTRAL || !(battle_config.attack_attr_none&BL_PET))
				dmg.damage=battle_attr_fix(src, bl, skilllv, element, tstatus->def_ele, tstatus->ele_lv);
			else
				dmg.damage= skilllv;
			dmg.damage2=0;
			dmg.div_= pd->a_skill->div_;
		}
	}

	if( dmg.flag&BF_MAGIC && ( skillid != NPC_EARTHQUAKE || (battle_config.eq_single_target_reflectable && (flag&0xFFF) == 1) ) )
	{ // Earthquake on multiple targets is not counted as a target skill. [Inkfish]
		if( (dmg.damage || dmg.damage2) && (type = skill_magic_reflect(src, bl, src==dsrc)) )
		{	//Magic reflection, switch caster/target
			struct block_list *tbl = bl;
			bl = src;
			src = tbl;
			sd = BL_CAST(BL_PC, src);
			tsd = BL_CAST(BL_PC, bl);
			sc = status_get_sc(bl);
			if (sc && !sc->count)
				sc = NULL; //Don't need it.
			/* bugreport:2564 flag&2 disables double casting trigger */
			flag |= 2;

			//Spirit of Wizard blocks Kaite's reflection
			if( type == 2 && sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_WIZARD )
			{	//Consume one Fragment per hit of the casted skill? [Skotlex]
			  	type = tsd?pc_search_inventory (tsd, 7321):0;
				if (type >= 0) {
					if ( tsd ) pc_delitem(tsd, type, 1, 0, 1, LOG_TYPE_CONSUME);
					dmg.damage = dmg.damage2 = 0;
					dmg.dmg_lv = ATK_MISS;
					sc->data[SC_SPIRIT]->val3 = skillid;
					sc->data[SC_SPIRIT]->val4 = dsrc->id;
				}
			}
		/**
		 * Official Magic Reflection Behavior : damage reflected depends on gears caster wears, not target
		 **/
		#if MAGIC_REFLECTION_TYPE
			if( dmg.dmg_lv != ATK_MISS )//Wiz SL cancelled and consumed fragment
				dmg = battle_calc_attack(BF_MAGIC,bl,bl,skillid,skilllv,flag&0xFFF);
		#endif
		}
		if(sc && sc->data[SC_MAGICROD] && src == dsrc) {
			int sp = skill_get_sp(skillid,skilllv);
			dmg.damage = dmg.damage2 = 0;
			dmg.dmg_lv = ATK_MISS; //This will prevent skill additional effect from taking effect. [Skotlex]
			sp = sp * sc->data[SC_MAGICROD]->val2 / 100;
			if(skillid == WZ_WATERBALL && skilllv > 1)
				sp = sp/((skilllv|1)*(skilllv|1)); //Estimate SP cost of a single water-ball
			status_heal(bl, 0, sp, 2);
		}
	}

	damage = dmg.damage + dmg.damage2;

	if( (skillid == AL_INCAGI || skillid == AL_BLESSING || 
		skillid == CASH_BLESSING || skillid == CASH_INCAGI ||
		skillid == MER_INCAGI || skillid == MER_BLESSING) && tsd->sc.data[SC_CHANGEUNDEAD] )
		damage = 1;

	if( damage > 0 && (( dmg.flag&BF_WEAPON && src != bl && ( src == dsrc || ( dsrc->type == BL_SKILL && ( skillid == SG_SUN_WARM || skillid == SG_MOON_WARM || skillid == SG_STAR_WARM ) ) ))
			|| (sc && sc->data[SC_REFLECTDAMAGE])) )
		rdamage = battle_calc_return_damage(bl,src, &damage, dmg.flag, skillid);

	if( damage && sc && sc->data[SC_GENSOU] && dmg.flag&BF_MAGIC ){
		struct block_list *nbl = NULL;
		nbl = battle_getenemyarea(bl,bl->x,bl->y,2,BL_CHAR,bl->id);
		if( nbl ){ // Only one target is chosen.
			damage = damage / 2; // Deflect half of the damage to a target nearby
			clif_skill_damage(bl, nbl, tick, status_get_amotion(src), 0, status_fix_damage(bl,nbl,damage,0), dmg.div_, OB_OBOROGENSOU_TRANSITION_ATK, -1, 6);
		}
	}

	//Skill hit type
	type=(skillid==0)?5:skill_get_hit(skillid);

	if(damage < dmg.div_
		//Only skills that knockback even when they miss. [Skotlex]
		&& skillid != CH_PALMSTRIKE)
		dmg.blewcount = 0;

	if(skillid == CR_GRANDCROSS||skillid == NPC_GRANDDARKNESS) {
		if(battle_config.gx_disptype) dsrc = src;
		if(src == bl) type = 4;
		else flag|=SD_ANIMATION;
	}
	if(skillid == NJ_TATAMIGAESHI) {
		dsrc = src; //For correct knockback.
		flag|=SD_ANIMATION;
	}

	if(sd) {
		int flag = 0; //Used to signal if this skill can be combo'ed later on.
		struct status_change_entry *sce;
		if ((sce = sd->sc.data[SC_COMBO])) {//End combo state after skill is invoked. [Skotlex]
			switch (skillid) {
			case TK_TURNKICK:
			case TK_STORMKICK:
			case TK_DOWNKICK:
			case TK_COUNTER:
				if (pc_famerank(sd->status.char_id,MAPID_TAEKWON)) {//Extend combo time.
					sce->val1 = skillid; //Update combo-skill
					sce->val3 = skillid;
					if( sce->timer != INVALID_TIMER )
						delete_timer(sce->timer, status_change_timer);
					sce->timer = add_timer(tick+sce->val4, status_change_timer, src->id, SC_COMBO);
					break;
				}
				unit_cancel_combo(src); // Cancel combo wait
				break;
			default:
				if( src == dsrc ) // Ground skills are exceptions. [Inkfish]
					status_change_end(src, SC_COMBO, INVALID_TIMER);
			}
		}
		switch(skillid) {
			case MO_TRIPLEATTACK:
				if (pc_checkskill(sd, MO_CHAINCOMBO) > 0 || pc_checkskill(sd, SR_DRAGONCOMBO) > 0)
					flag=1;
				break;
			case MO_CHAINCOMBO:
				if(pc_checkskill(sd, MO_COMBOFINISH) > 0 && sd->spiritball > 0)
					flag=1;
				break;
			case MO_COMBOFINISH:
				if (sd->status.party_id>0) //bonus from SG_FRIEND [Komurka]
					party_skill_check(sd, sd->status.party_id, MO_COMBOFINISH, skilllv);
				if (pc_checkskill(sd, CH_TIGERFIST) > 0 && sd->spiritball > 0)
					flag=1;
			case CH_TIGERFIST:
				if (!flag && pc_checkskill(sd, CH_CHAINCRUSH) > 0 && sd->spiritball > 1)
					flag=1;
			case CH_CHAINCRUSH:
				if (!flag && pc_checkskill(sd, MO_EXTREMITYFIST) > 0 && sd->spiritball > 0 && sd->sc.data[SC_EXPLOSIONSPIRITS])
					flag=1;
				break;
			case AC_DOUBLE:
				if( (tstatus->race == RC_BRUTE || tstatus->race == RC_INSECT) && pc_checkskill(sd, HT_POWER))
				{
					//TODO: This code was taken from Triple Blows, is this even how it should be? [Skotlex]
					sc_start2(src,SC_COMBO,100,HT_POWER,bl->id,2000);
					clif_combo_delay(src,2000);
				}
				break;
			case TK_COUNTER:
			{	//bonus from SG_FRIEND [Komurka]
				int level;
				if(sd->status.party_id>0 && (level = pc_checkskill(sd,SG_FRIEND)))
					party_skill_check(sd, sd->status.party_id, TK_COUNTER,level);
			}
				break;
			case SL_STIN:
			case SL_STUN:
				if (skilllv >= 7 && !sd->sc.data[SC_SMA])
					sc_start(src,SC_SMA,100,skilllv,skill_get_time(SL_SMA, skilllv));
				break;
			case GS_FULLBUSTER:
				//Can't attack nor use items until skill's delay expires. [Skotlex]
				sd->ud.attackabletime = sd->canuseitem_tick = sd->ud.canact_tick;
				break;
			case SR_DRAGONCOMBO:
				if( pc_checkskill(sd, SR_FALLENEMPIRE) > 0 )
					flag = 1;
				break;
			case SR_FALLENEMPIRE:
				if( pc_checkskill(sd, SR_TIGERCANNON) > 0 || pc_checkskill(sd, SR_GATEOFHELL) > 0 )
					flag = 1;
				break;
		}	//Switch End
		if (flag) { //Possible to chain
			flag = DIFF_TICK(sd->ud.canact_tick, tick);
			if (flag < 1) flag = 1;
			sc_start2(src,SC_COMBO,100,skillid,bl->id,flag);
			clif_combo_delay(src, flag);
		}
	}

	//Display damage.
	switch( skillid )
	{
	case PA_GOSPEL: //Should look like Holy Cross [Skotlex]
		dmg.dmotion = clif_skill_damage(dsrc,bl,tick,dmg.amotion,dmg.dmotion, damage, dmg.div_, CR_HOLYCROSS, -1, 5);
		break;
	//Skills that need be passed as a normal attack for the client to display correctly.
	case HVAN_EXPLOSION:
	case NPC_SELFDESTRUCTION:
		if(src->type==BL_PC)
			dmg.blewcount = 10;
		dmg.amotion = 0; //Disable delay or attack will do no damage since source is dead by the time it takes effect. [Skotlex]
		// fall through
	case KN_AUTOCOUNTER:
	case NPC_CRITICALSLASH:
	case TF_DOUBLE:
	case GS_CHAINACTION:
		dmg.dmotion = clif_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,dmg.type,dmg.damage2);
		break;

	case AS_SPLASHER:
		if( flag&SD_ANIMATION ) // the surrounding targets
			dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, -1, 5); // needs -1 as skill level
		else // the central target doesn't display an animation
			dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, -2, 5); // needs -2(!) as skill level
		break;
	case WL_HELLINFERNO:
	case SR_EARTHSHAKER:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,skillid,-2,6);
		break;
	case WL_SOULEXPANSION:
	case WL_COMET:
	case KO_MUCHANAGE:
	case NJ_HUUMA:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skillid,skilllv,8);
		break;
	case WL_CHAINLIGHTNING_ATK:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,WL_CHAINLIGHTNING,-2,6);
		break;
	case WL_TETRAVORTEX_FIRE:
	case WL_TETRAVORTEX_WATER:
	case WL_TETRAVORTEX_WIND:
	case WL_TETRAVORTEX_GROUND:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,WL_TETRAVORTEX_FIRE,-2,type);
		break;
	case LG_OVERBRAND_BRANDISH:
	case LG_OVERBRAND_PLUSATK:
	case EL_FIRE_BOMB:
	case EL_FIRE_BOMB_ATK:
	case EL_FIRE_WAVE:
	case EL_FIRE_WAVE_ATK:
	case EL_FIRE_MANTLE:
	case EL_CIRCLE_OF_FIRE:
	case EL_FIRE_ARROW:
	case EL_ICE_NEEDLE:
	case EL_WATER_SCREW:
	case EL_WATER_SCREW_ATK:
	case EL_WIND_SLASH:
	case EL_TIDAL_WEAPON:
	case EL_ROCK_CRUSHER:
	case EL_ROCK_CRUSHER_ATK:
	case EL_HURRICANE:
	case EL_HURRICANE_ATK:
	case EL_TYPOON_MIS:
	case EL_TYPOON_MIS_ATK:
	case KO_BAKURETSU:
	case GN_CRAZYWEED_ATK:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skillid,-1,5);
		break;
	case GN_SLINGITEM_RANGEMELEEATK:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,GN_SLINGITEM,-2,6);
		break;
	case EL_STONE_RAIN:
		dmg.dmotion = clif_skill_damage(dsrc,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skillid,-1,(flag&1)?8:5);
		break;	
	case WM_SEVERE_RAINSTORM_MELEE:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,WM_SEVERE_RAINSTORM,skilllv,5);
		break;
	case WM_REVERBERATION_MELEE:
	case WM_REVERBERATION_MAGIC:
		dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,WM_REVERBERATION,-2,6);
		break;
	case HT_CLAYMORETRAP:
	case HT_BLASTMINE:
	case HT_FLASHER:
	case HT_FREEZINGTRAP:
	case RA_CLUSTERBOMB:
	case RA_FIRINGTRAP:
	case RA_ICEBOUNDTRAP:
		dmg.dmotion = clif_skill_damage(src,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid,flag&SD_LEVEL?-1:skilllv, 5);
		if( dsrc != src ) // avoid damage display redundancy
			break;
	case HT_LANDMINE:
		dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, -1, type);
		break;
	case AB_DUPLELIGHT_MELEE:
	case AB_DUPLELIGHT_MAGIC:
		dmg.amotion = 300;/* makes the damage value not overlap with previous damage (when displayed by the client) */
	default:
		if( flag&SD_ANIMATION && dmg.div_ < 2 ) //Disabling skill animation doesn't works on multi-hit.
			type = 5;
		if( bl->type == BL_SKILL ){
			TBL_SKILL *su = (TBL_SKILL*)bl;
			if( su->group && skill_get_inf2(su->group->skill_id)&INF2_TRAP )// show damage on trap targets
				clif_skill_damage(src,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, flag&SD_LEVEL?-1:skilllv, 5);
		}
		dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, flag&SD_LEVEL?-1:skilllv, type);
		break;
	}

	map_freeblock_lock();

	if(damage > 0 && dmg.flag&BF_SKILL && tsd
		&& pc_checkskill(tsd,RG_PLAGIARISM)
	  	&& (!sc || !sc->data[SC_PRESERVE])
		&& damage < tsd->battle_status.hp)
	{	//Updated to not be able to copy skills if the blow will kill you. [Skotlex]
		int copy_skill = skillid;
		/**
		 * Copy Referal: dummy skills should point to their source upon copying
		 **/
		switch( skillid ) {
			case AB_DUPLELIGHT_MELEE:
			case AB_DUPLELIGHT_MAGIC:
				copy_skill = AB_DUPLELIGHT;
				break;
			case WL_CHAINLIGHTNING_ATK:
				copy_skill = WL_CHAINLIGHTNING;
				break;
			case WM_REVERBERATION_MELEE:
			case WM_REVERBERATION_MAGIC:
				copy_skill = WM_REVERBERATION;
				break;
			case WM_SEVERE_RAINSTORM_MELEE:
				copy_skill = WM_SEVERE_RAINSTORM;
			break;
			case GN_CRAZYWEED_ATK:
				copy_skill = GN_CRAZYWEED;
				break;
			case GN_HELLS_PLANT_ATK:
				copy_skill = GN_HELLS_PLANT;
				break;				
			case LG_OVERBRAND_BRANDISH:
			case LG_OVERBRAND_PLUSATK:
				copy_skill = LG_OVERBRAND;
				break;
		}

		if ((tsd->status.skill[copy_skill].id == 0 || tsd->status.skill[copy_skill].flag == SKILL_FLAG_PLAGIARIZED) &&
			can_copy(tsd,copy_skill,bl))	// Split all the check into their own function [Aru]
		{
			int lv;
			if( sc && sc->data[SC__REPRODUCE] && (lv = sc->data[SC__REPRODUCE]->val1) ) {
				//Level dependent and limitation.
				lv = min(lv,skill_get_max(copy_skill));
				if( tsd->reproduceskill_id && tsd->status.skill[tsd->reproduceskill_id].flag == SKILL_FLAG_PLAGIARIZED ) {
					tsd->status.skill[tsd->reproduceskill_id].id = 0;
					tsd->status.skill[tsd->reproduceskill_id].lv = 0;
					tsd->status.skill[tsd->reproduceskill_id].flag = 0;
					clif_deleteskill(tsd,tsd->reproduceskill_id);
				}

				tsd->reproduceskill_id = copy_skill;
				pc_setglobalreg(tsd, "REPRODUCE_SKILL", copy_skill);
				pc_setglobalreg(tsd, "REPRODUCE_SKILL_LV", lv);

				tsd->status.skill[copy_skill].id = copy_skill;
				tsd->status.skill[copy_skill].lv = lv;
				tsd->status.skill[copy_skill].flag = SKILL_FLAG_PLAGIARIZED;
				clif_addskill(tsd,copy_skill);
			} else {
				lv = skilllv;
				if (tsd->cloneskill_id && tsd->status.skill[tsd->cloneskill_id].flag == SKILL_FLAG_PLAGIARIZED){
					tsd->status.skill[tsd->cloneskill_id].id = 0;
					tsd->status.skill[tsd->cloneskill_id].lv = 0;
					tsd->status.skill[tsd->cloneskill_id].flag = 0;
					clif_deleteskill(tsd,tsd->cloneskill_id);
				}

				if ((type = pc_checkskill(tsd,RG_PLAGIARISM)) < lv)
					lv = type;

				tsd->cloneskill_id = copy_skill;
				pc_setglobalreg(tsd, "CLONE_SKILL", copy_skill);
				pc_setglobalreg(tsd, "CLONE_SKILL_LV", lv);

				tsd->status.skill[skillid].id = copy_skill;
				tsd->status.skill[skillid].lv = lv;
				tsd->status.skill[skillid].flag = SKILL_FLAG_PLAGIARIZED;
				clif_addskill(tsd,skillid);
			}
		}
	}

	if (dmg.dmg_lv >= ATK_MISS && (type = skill_get_walkdelay(skillid, skilllv)) > 0)
	{	//Skills with can't walk delay also stop normal attacking for that
		//duration when the attack connects. [Skotlex]
		struct unit_data *ud = unit_bl2ud(src);
		if (ud && DIFF_TICK(ud->attackabletime, tick + type) < 0)
			ud->attackabletime = tick + type;
	}

	if( !dmg.amotion )
	{ //Instant damage
		if( !sc || (!sc->data[SC_DEVOTION] && skillid != CR_REFLECTSHIELD) )
			status_fix_damage(src,bl,damage,dmg.dmotion); //Deal damage before knockback to allow stuff like firewall+storm gust combo.
		if( !status_isdead(bl) )
			skill_additional_effect(src,bl,skillid,skilllv,dmg.flag,dmg.dmg_lv,tick);
		if( damage > 0 ) //Counter status effects [Skotlex]
			skill_counter_additional_effect(src,bl,skillid,skilllv,dmg.flag,tick);
	}
	// Hell Inferno burning status only starts if Fire part hits.
	if( skillid == WL_HELLINFERNO && dmg.damage > 0 )
		sc_start4(bl,SC_BURNING,55+5*skilllv,skilllv,1000,src->id,0,skill_get_time(skillid,skilllv));
	// Apply knock back chance in SC_TRIANGLESHOT skill.
	else if( skillid == SC_TRIANGLESHOT && rnd()%100 > (1 + skilllv) )
		dmg.blewcount = 0;

	//Only knockback if it's still alive, otherwise a "ghost" is left behind. [Skotlex]
	//Reflected spells do not bounce back (bl == dsrc since it only happens for direct skills)
	if (dmg.blewcount > 0 && bl!=dsrc && !status_isdead(bl)) {
		int direction = -1; // default
		switch(skillid) {//direction
			case MG_FIREWALL:
			case PR_SANCTUARY:
			case SC_TRIANGLESHOT:
			case LG_OVERBRAND:
			case SR_KNUCKLEARROW:
			case GN_WALLOFTHORN:
			case EL_FIRE_MANTLE:				
				direction = unit_getdir(bl);// backwards
				break;
			// This ensures the storm randomly pushes instead of exactly a cell backwards per official mechanics.
			case WZ_STORMGUST:
				direction = rand()%8;
				break;
			case WL_CRIMSONROCK:
				direction = map_calc_dir(bl,skill_area_temp[4],skill_area_temp[5]);
				break;

		}
		//blown-specific handling
		switch( skillid ) {
			case LG_OVERBRAND:
				if( skill_blown(dsrc,bl,dmg.blewcount,direction,0) ) {
					short dir_x, dir_y;
					dir_x = dirx[(direction+4)%8];
					dir_y = diry[(direction+4)%8];
					if( map_getcell(bl->m, bl->x+dir_x, bl->y+dir_y, CELL_CHKNOPASS) != 0 )
						skill_addtimerskill(src, tick + status_get_amotion(src), bl->id, 0, 0, LG_OVERBRAND_PLUSATK, skilllv, BF_WEAPON, flag );
				} else
					skill_addtimerskill(src, tick + status_get_amotion(src), bl->id, 0, 0, LG_OVERBRAND_PLUSATK, skilllv, BF_WEAPON, flag );
				break;
			case SR_KNUCKLEARROW:
				if( skill_blown(dsrc,bl,dmg.blewcount,direction,0) && !(flag&4) ) {
					short dir_x, dir_y;
					dir_x = dirx[(direction+4)%8];
					dir_y = diry[(direction+4)%8];
					if( map_getcell(bl->m, bl->x+dir_x, bl->y+dir_y, CELL_CHKNOPASS) != 0 )
						skill_addtimerskill(src, tick + 300 * ((flag&2) ? 1 : 2), bl->id, 0, 0, skillid, skilllv, BF_WEAPON, flag|4);	
				}			
				break;
			case GN_WALLOFTHORN:
				unit_stop_walking(bl,1);
				skill_blown(dsrc,bl,dmg.blewcount,direction, 0x2 );
				clif_fixpos(bl);
				break;
			default:
				skill_blown(dsrc,bl,dmg.blewcount,direction, 0x0 );
				if ( !dmg.blewcount && bl->type == BL_SKILL && damage > 0 ){
					TBL_SKILL *su = (TBL_SKILL*)bl;
					if( su->group && su->group->skill_id == HT_BLASTMINE)
						skill_blown(src, bl, 3, -1, 0);
				}
				break;
		}
	}

	//Delayed damage must be dealt after the knockback (it needs to know actual position of target)
	if (dmg.amotion)
		battle_delay_damage(tick, dmg.amotion,src,bl,dmg.flag,skillid,skilllv,damage,dmg.dmg_lv,dmg.dmotion);

	if( sc && sc->data[SC_DEVOTION] && skillid != PA_PRESSURE )
	{
		struct status_change_entry *sce = sc->data[SC_DEVOTION];
		struct block_list *d_bl = map_id2bl(sce->val1);

		if( d_bl && (
			(d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == bl->id) ||
			(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce->val2] == bl->id)
			) && check_distance_bl(bl, d_bl, sce->val3) )
		{
			/**
			 * Check for devotion and change targetted dmg.
			 * [d_bl = paladin; bl = player; src = source of dmg]
			 **/
			bool devo_flag = false; /* false = paladin devoing; true = player */
			if ( src )
			{
				struct status_change *tsc;
				tsc = status_get_sc(src);

				/* Per official standards, following skills should reflect at the bl */
				if( (tsc->data[SC_KAITE] && attack_type == BF_MAGIC) ||
					(tsc->data[SC_REFLECTDAMAGE] && attack_type != BF_MAGIC)
				  )
					devo_flag = true;
			}

			clif_damage(
				( (devo_flag) ? bl:d_bl), 
				( (devo_flag) ? bl:d_bl), gettick(), 0, 0, damage, 0, 0, 0);
			status_fix_damage(
				( (devo_flag) ? bl:NULL), 
				( (devo_flag) ? bl:d_bl), damage, 0);
		}
		else {
			status_change_end(bl, SC_DEVOTION, INVALID_TIMER);
			if( !dmg.amotion )
				status_fix_damage(src,bl,damage,dmg.dmotion);
		}
	}

	if(damage > 0 && !(tstatus->mode&MD_BOSS)) {
		if( skillid == RG_INTIMIDATE ) {
			int rate = 50 + skilllv * 5;
			rate = rate + (status_get_lv(src) - status_get_lv(bl));
			if(rnd()%100 < rate)
				skill_addtimerskill(src,tick + 800,bl->id,0,0,skillid,skilllv,0,flag);
		} else if( skillid == SC_FATALMENACE )
			skill_addtimerskill(src,tick + 800,bl->id,skill_area_temp[4],skill_area_temp[5],skillid,skilllv,0,flag);
	}

	if(skillid == CR_GRANDCROSS || skillid == NPC_GRANDDARKNESS)
		dmg.flag |= BF_WEAPON;

	if( sd && src != bl && damage > 0 && ( dmg.flag&BF_WEAPON || 
		(dmg.flag&BF_MISC && (skillid == RA_CLUSTERBOMB || skillid == RA_FIRINGTRAP || skillid == RA_ICEBOUNDTRAP || skillid == RK_DRAGONBREATH)) ) )
	{
		if (battle_config.left_cardfix_to_right)
			battle_drain(sd, bl, dmg.damage, dmg.damage, tstatus->race, tstatus->mode&MD_BOSS);
		else
			battle_drain(sd, bl, dmg.damage, dmg.damage2, tstatus->race, tstatus->mode&MD_BOSS);
	}

	if( rdamage > 0 ) {
		if( sc && sc->data[SC_REFLECTDAMAGE] ) {
			if( src != bl )// Don't reflect your own damage (Grand Cross)
				map_foreachinshootrange(battle_damage_area,bl,skill_get_splash(LG_REFLECTDAMAGE,1),BL_CHAR,tick,bl,dmg.amotion,sstatus->dmotion,rdamage,tstatus->race);
		} else {
			if( dmg.amotion )
				battle_delay_damage(tick, dmg.amotion,bl,src,0,CR_REFLECTSHIELD,0,rdamage,ATK_DEF,0);
			else
				status_fix_damage(bl,src,rdamage,0);
			clif_damage(src,src,tick, dmg.amotion,0,rdamage,1,4,0); // in aegis damage reflected is shown in single hit.
			//Use Reflect Shield to signal this kind of skill trigger. [Skotlex]
			if( tsd && src != bl )
				battle_drain(tsd, src, rdamage, rdamage, sstatus->race, is_boss(src));
			skill_additional_effect(bl, src, CR_REFLECTSHIELD, 1, BF_WEAPON|BF_SHORT|BF_NORMAL,ATK_DEF,tick);
		}
	}
	if( damage > 0 ) {
		/**
		 * Post-damage effects
		 **/
		switch( skillid ) {
			case RK_CRUSHSTRIKE:
				skill_break_equip(src,EQP_WEAPON,2000,BCT_SELF); // 20% chance to destroy the weapon.
				break;
			case GC_VENOMPRESSURE: {
					struct status_change *ssc = status_get_sc(src);
					if( ssc && ssc->data[SC_POISONINGWEAPON] && rnd()%100 < 70 + 5*skilllv ) {
						sc_start(bl,ssc->data[SC_POISONINGWEAPON]->val2,100,ssc->data[SC_POISONINGWEAPON]->val1,skill_get_time2(GC_POISONINGWEAPON, 1));
						status_change_end(src,SC_POISONINGWEAPON,INVALID_TIMER);
						clif_skill_nodamage(src,bl,skillid,skilllv,1);
					}
				}
				break;
			case WM_METALICSOUND:
				status_zap(bl, 0, damage*100/(100*(110-pc_checkskill(sd,WM_LESSON)*10)));
				break;
			case SR_TIGERCANNON:
				status_zap(bl, 0, damage/10); // 10% of damage dealt
				break;
		}
		if( sd )
			skill_onskillusage(sd, bl, skillid, tick);
	}

	if (!(flag&2) &&
		(
			skillid == MG_COLDBOLT || skillid == MG_FIREBOLT || skillid == MG_LIGHTNINGBOLT
		) &&
		(sc = status_get_sc(src)) &&
		sc->data[SC_DOUBLECAST] &&
		rnd() % 100 < sc->data[SC_DOUBLECAST]->val2)
	{
//		skill_addtimerskill(src, tick + dmg.div_*dmg.amotion, bl->id, 0, 0, skillid, skilllv, BF_MAGIC, flag|2);
		skill_addtimerskill(src, tick + dmg.amotion, bl->id, 0, 0, skillid, skilllv, BF_MAGIC, flag|2);
	}

	map_freeblock_unlock();

	return damage;
}

/*==========================================
 * スキル範??U?用(map_foreachinareaから呼ばれる)
 * flagについて?F16?i?を確認
 * MSB <- 00fTffff ->LSB
 *	T	=タ?ゲット選?用(BCT_*)
 *  ffff=自由に使用可能
 *  0	=予約?B0に固定
 *------------------------------------------*/
typedef int (*SkillFunc)(struct block_list *, struct block_list *, int, int, unsigned int, int);
int skill_area_sub (struct block_list *bl, va_list ap)
{
	struct block_list *src;
	int skill_id,skill_lv,flag;
	unsigned int tick;
	SkillFunc func;

	nullpo_ret(bl);

	src=va_arg(ap,struct block_list *);
	skill_id=va_arg(ap,int);
	skill_lv=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);
	flag=va_arg(ap,int);
	func=va_arg(ap,SkillFunc);

	if(battle_check_target(src,bl,flag) > 0)
	{
		// several splash skills need this initial dummy packet to display correctly
		if (flag&SD_PREAMBLE && skill_area_temp[2] == 0)
			clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, 6);

		if (flag&(SD_SPLASH|SD_PREAMBLE))
			skill_area_temp[2]++;

		return func(src,bl,skill_id,skill_lv,tick,flag);
	}
	return 0;
}

static int skill_check_unit_range_sub (struct block_list *bl, va_list ap)
{
	struct skill_unit *unit;
	int skillid,g_skillid;

	unit = (struct skill_unit *)bl;

	if(bl->prev == NULL || bl->type != BL_SKILL)
		return 0;

	if(!unit->alive)
		return 0;

	skillid = va_arg(ap,int);
	g_skillid = unit->group->skill_id;

	switch (skillid) {
		case MG_SAFETYWALL:
		case AL_PNEUMA:
		case SC_MAELSTROM:
			if(g_skillid != MG_SAFETYWALL && g_skillid != AL_PNEUMA && g_skillid != SC_MAELSTROM)
				return 0;
			break;
		case AL_WARP:
		case HT_SKIDTRAP:
		case MA_SKIDTRAP:
		case HT_LANDMINE:
		case MA_LANDMINE:
		case HT_ANKLESNARE:
		case HT_SHOCKWAVE:
		case HT_SANDMAN:
		case MA_SANDMAN:
		case HT_FLASHER:
		case HT_FREEZINGTRAP:
		case MA_FREEZINGTRAP:
		case HT_BLASTMINE:
		case HT_CLAYMORETRAP:
		case HT_TALKIEBOX:
		case HP_BASILICA:
		case RA_ELECTRICSHOCKER:
		case RA_CLUSTERBOMB:
		case RA_MAGENTATRAP:
		case RA_COBALTTRAP:
		case RA_MAIZETRAP:
		case RA_VERDURETRAP:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
		case SC_DIMENSIONDOOR:
			//Non stackable on themselves and traps (including venom dust which does not has the trap inf2 set)
			if (skillid != g_skillid && !(skill_get_inf2(g_skillid)&INF2_TRAP) && g_skillid != AS_VENOMDUST && g_skillid != MH_POISON_MIST)
				return 0;
			break;
		default: //Avoid stacking with same kind of trap. [Skotlex]
			if (g_skillid != skillid)
				return 0;
			break;
	}

	return 1;
}

static int skill_check_unit_range (struct block_list *bl, int x, int y, int skillid, int skilllv)
{
	//Non players do not check for the skill's splash-trigger area.
	int range = bl->type==BL_PC?skill_get_unit_range(skillid, skilllv):0;
	int layout_type = skill_get_unit_layout_type(skillid,skilllv);
	if (layout_type==-1 || layout_type>MAX_SQUARE_LAYOUT) {
		ShowError("skill_check_unit_range: unsupported layout type %d for skill %d\n",layout_type,skillid);
		return 0;
	}

	range += layout_type;
	return map_foreachinarea(skill_check_unit_range_sub,bl->m,x-range,y-range,x+range,y+range,BL_SKILL,skillid);
}

static int skill_check_unit_range2_sub (struct block_list *bl, va_list ap)
{
	int skillid;

	if(bl->prev == NULL)
		return 0;

	skillid = va_arg(ap,int);

	if( status_isdead(bl) && skillid != AL_WARP )
		return 0;

	if( skillid == HP_BASILICA && bl->type == BL_PC )
		return 0;

	if( skillid == AM_DEMONSTRATION && bl->type == BL_MOB && ((TBL_MOB*)bl)->class_ == MOBID_EMPERIUM )
		return 0; //Allow casting Bomb/Demonstration Right under emperium [Skotlex]
	return 1;
}

static int skill_check_unit_range2 (struct block_list *bl, int x, int y, int skillid, int skilllv)
{
	int range, type;

	switch (skillid) {	// to be expanded later
	case WZ_ICEWALL:
		range = 2;
		break;
	default:
		{
			int layout_type = skill_get_unit_layout_type(skillid,skilllv);
			if (layout_type==-1 || layout_type>MAX_SQUARE_LAYOUT) {
				ShowError("skill_check_unit_range2: unsupported layout type %d for skill %d\n",layout_type,skillid);
				return 0;
			}
			range = skill_get_unit_range(skillid,skilllv) + layout_type;
		}
		break;
	}

	// if the caster is a monster/NPC, only check for players
	// otherwise just check characters
	if (bl->type == BL_PC)
		type = BL_CHAR;
	else
		type = BL_PC;

	return map_foreachinarea(skill_check_unit_range2_sub, bl->m,
		x - range, y - range, x + range, y + range,
		type, skillid);
}

int skill_guildaura_sub (struct map_session_data* sd, int id, int strvit, int agidex)
{
	if(id == sd->bl.id && battle_config.guild_aura&16)
		return 0;  // Do not affect guild leader

	if (sd->sc.data[SC_GUILDAURA]) {
		struct status_change_entry *sce = sd->sc.data[SC_GUILDAURA];
		if( sce->val3 != strvit || sce->val4 != agidex ) {
			sce->val3 = strvit;
			sce->val4 = agidex;
			status_calc_bl(&sd->bl, status_sc2scb_flag(SC_GUILDAURA));
		}
		return 0;
	}
	sc_start4(&sd->bl, SC_GUILDAURA,100, 1, id, strvit, agidex, 1000);
	return 1;
}

/*==========================================
 * Checks that you have the requirements for casting a skill for homunculus/mercenary.
 * Flag:
 * &1: finished casting the skill (invoke hp/sp/item consumption)
 * &2: picked menu entry (Warp Portal, Teleport and other menu based skills)
 *------------------------------------------*/
static int skill_check_condition_mercenary(struct block_list *bl, int skill, int lv, int type)
{
	struct status_data *status;
	struct map_session_data *sd = NULL;
	int i, j, hp, sp, hp_rate, sp_rate, state, mhp;
	int itemid[MAX_SKILL_ITEM_REQUIRE],amount[ARRAYLENGTH(itemid)],index[ARRAYLENGTH(itemid)];

	if( lv < 1 || lv > MAX_SKILL_LEVEL )
		return 0;
	nullpo_ret(bl);

	switch( bl->type )
	{
		case BL_HOM: sd = ((TBL_HOM*)bl)->master; break;
		case BL_MER: sd = ((TBL_MER*)bl)->master; break;
	}

	status = status_get_status_data(bl);
	if( (j = skill_get_index(skill)) == 0 )
		return 0;

	// Requeriments
	for( i = 0; i < ARRAYLENGTH(itemid); i++ )
	{
		itemid[i] = skill_db[j].itemid[i];
		amount[i] = skill_db[j].amount[i];
	}
	hp = skill_db[j].hp[lv-1];
	sp = skill_db[j].sp[lv-1];
	hp_rate = skill_db[j].hp_rate[lv-1];
	sp_rate = skill_db[j].sp_rate[lv-1];
	state = skill_db[j].state;
	if( (mhp = skill_db[j].mhp[lv-1]) > 0 )
		hp += (status->max_hp * mhp) / 100;
	if( hp_rate > 0 )
		hp += (status->hp * hp_rate) / 100;
	else
		hp += (status->max_hp * (-hp_rate)) / 100;
	if( sp_rate > 0 )
		sp += (status->sp * sp_rate) / 100;
	else
		sp += (status->max_sp * (-sp_rate)) / 100;

	if( bl->type == BL_HOM )
	{ // Intimacy Requeriments
		struct homun_data *hd = BL_CAST(BL_HOM, bl);
		switch( skill )
		{
			case HFLI_SBR44:
				if( hd->homunculus.intimacy <= 200 )
					return 0;
				break;
			case HVAN_EXPLOSION:
				if( hd->homunculus.intimacy < (unsigned int)battle_config.hvan_explosion_intimate )
					return 0;
				break;
		}
	}

	if( !(type&2) )
	{
		if( hp > 0 && status->hp <= (unsigned int)hp )
		{
			clif_skill_fail(sd, skill, USESKILL_FAIL_HP_INSUFFICIENT, 0);
			return 0;
		}
		if( sp > 0 && status->sp <= (unsigned int)sp )
		{
			clif_skill_fail(sd, skill, USESKILL_FAIL_SP_INSUFFICIENT, 0);
			return 0;
		}
	}

	if( !type )
		switch( state )
		{
			case ST_MOVE_ENABLE:
				if( !unit_can_move(bl) )
				{
					clif_skill_fail(sd, skill, USESKILL_FAIL_LEVEL, 0);
					return 0;
				}
				break;
		}
	if( !(type&1) )
		return 1;

	// Check item existences
	for( i = 0; i < ARRAYLENGTH(itemid); i++ )
	{
		index[i] = -1;
		if( itemid[i] < 1 ) continue; // No item
		index[i] = pc_search_inventory(sd, itemid[i]);
		if( index[i] < 0 || sd->status.inventory[index[i]].amount < amount[i] )
		{
			clif_skill_fail(sd, skill, USESKILL_FAIL_LEVEL, 0);
			return 0;
		}
	}

	// Consume items
	for( i = 0; i < ARRAYLENGTH(itemid); i++ )
	{
		if( index[i] >= 0 ) pc_delitem(sd, index[i], amount[i], 0, 1, LOG_TYPE_CONSUME);
	}

	if( type&2 )
		return 1;

	if( sp || hp )
		status_zap(bl, hp, sp);

	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_area_sub_count (struct block_list *src, struct block_list *target, int skillid, int skilllv, unsigned int tick, int flag)
{
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_timerskill(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *src = map_id2bl(id),*target;
	struct unit_data *ud = unit_bl2ud(src);
	struct skill_timerskill *skl = NULL;
	int range;

	nullpo_ret(src);
	nullpo_ret(ud);
	skl = ud->skilltimerskill[data];
	nullpo_ret(skl);
	ud->skilltimerskill[data] = NULL;

	do {
		if(src->prev == NULL)
			break;
		if(skl->target_id) {
			target = map_id2bl(skl->target_id);
			if( ( skl->skill_id == RG_INTIMIDATE || skl->skill_id == SC_FATALMENACE ) && (!target || target->prev == NULL || !check_distance_bl(src,target,AREA_SIZE)) )
				target = src; //Required since it has to warp.
			if(target == NULL)
				break;
			if(target->prev == NULL)
				break;
			if(src->m != target->m)
				break;
			if(status_isdead(src))
				break;
			if(status_isdead(target) && skl->skill_id != RG_INTIMIDATE && skl->skill_id != WZ_WATERBALL)
				break;

			switch(skl->skill_id) {
				case RG_INTIMIDATE:
					if (unit_warp(src,-1,-1,-1,CLR_TELEPORT) == 0) {
						short x,y;
						map_search_freecell(src, 0, &x, &y, 1, 1, 0);
						if (target != src && !status_isdead(target))
							unit_warp(target, -1, x, y, CLR_TELEPORT);
					}
					break;
				case BA_FROSTJOKER:
				case DC_SCREAM:
					range= skill_get_splash(skl->skill_id, skl->skill_lv);
					map_foreachinarea(skill_frostjoke_scream,skl->map,skl->x-range,skl->y-range,
						skl->x+range,skl->y+range,BL_CHAR,src,skl->skill_id,skl->skill_lv,tick);
					break;
				case NPC_EARTHQUAKE:
					if( skl->type > 1 )
						skill_addtimerskill(src,tick+250,src->id,0,0,skl->skill_id,skl->skill_lv,skl->type-1,skl->flag);
					skill_area_temp[0] = map_foreachinrange(skill_area_sub, src, skill_get_splash(skl->skill_id, skl->skill_lv), BL_CHAR, src, skl->skill_id, skl->skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
					skill_area_temp[1] = src->id;
					skill_area_temp[2] = 0;
					map_foreachinrange(skill_area_sub, src, skill_get_splash(skl->skill_id, skl->skill_lv), splash_target(src), src, skl->skill_id, skl->skill_lv, tick, skl->flag, skill_castend_damage_id);
					break;
				case WZ_WATERBALL:					
					skill_toggle_magicpower(src, skl->skill_id); // only the first hit will be amplify
					if (!status_isdead(target))
						skill_attack(BF_MAGIC,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
					if (skl->type>1 && !status_isdead(target) && !status_isdead(src)) {
						skill_addtimerskill(src,tick+125,target->id,0,0,skl->skill_id,skl->skill_lv,skl->type-1,skl->flag);
					} else {
						struct status_change *sc = status_get_sc(src);
						if(sc) {
							if(sc->data[SC_SPIRIT] &&
								sc->data[SC_SPIRIT]->val2 == SL_WIZARD &&
								sc->data[SC_SPIRIT]->val3 == skl->skill_id)
								sc->data[SC_SPIRIT]->val3 = 0; //Clear bounced spell check.
						}
					}
					break;
				/**
				 * Warlock
				 **/
				case WL_CHAINLIGHTNING_ATK:
					{
						struct block_list *nbl = NULL; // Next Target of Chain
						skill_attack(BF_MAGIC,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag); // Hit a Lightning on the current Target
						skill_toggle_magicpower(src, skl->skill_id); // only the first hit will be amplify
						if( skl->type > 1 )
						{ // Remaining Chains Hit
							nbl = battle_getenemyarea(src,target->x,target->y,2,BL_CHAR|BL_SKILL,target->id); // Search for a new Target around current one...
							if( nbl == NULL && skl->x > 1 )
							{
								nbl = target;
								skl->x--;
							}
							else skl->x = 3;
						}

						if( nbl )
							skill_addtimerskill(src,tick+status_get_adelay(src),nbl->id,skl->x,0,WL_CHAINLIGHTNING_ATK,skl->skill_lv,skl->type-1,skl->flag);
					}
					break;
				case WL_TETRAVORTEX_FIRE:
				case WL_TETRAVORTEX_WATER:
				case WL_TETRAVORTEX_WIND:
				case WL_TETRAVORTEX_GROUND:
					skill_attack(BF_MAGIC,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
					skill_toggle_magicpower(src, skl->skill_id); // only the first hit will be amplify
					if( skl->type >= 3 )
					{ // Final Hit
						if( !status_isdead(target) )
						{ // Final Status Effect
							int effects[4] = { SC_BURNING, SC_FREEZING, SC_BLEEDING, SC_STUN },
								applyeffects[4] = { 0, 0, 0, 0 },
								i, j = 0, k = 0;
							for( i = 1; i <= 8; i = i + i )
							{
								if( skl->x&i )
								{
									applyeffects[j] = effects[k];
									j++;
								}
								k++;
							}
							if( j )
							{
								i = applyeffects[rnd()%j];
								status_change_start(target, i, 10000, skl->skill_lv,
									(i == SC_BURNING ? 1000 : 0),
									(i == SC_BURNING ? src->id : 0),
									0, skill_get_time(WL_TETRAVORTEX,skl->skill_lv), 0);
							}
						}
					}
					break;
				case WM_REVERBERATION_MELEE:
				case WM_REVERBERATION_MAGIC:
					skill_castend_damage_id(src, target, skl->skill_id, skl->skill_lv, tick, skl->flag|SD_LEVEL); // damage should split among targets
					break;
				case SC_FATALMENACE:
					if( src == target ) // Casters Part
						unit_warp(src, -1, skl->x, skl->y, 3);
					else { // Target's Part
						short x = skl->x, y = skl->y;
						map_search_freecell(NULL, target->m, &x, &y, 2, 2, 1);
						unit_warp(target,-1,x,y,3);
					}
					break;
				case LG_MOONSLASHER:
				case SR_WINDMILL:
					if( target->type == BL_PC ) {
						struct map_session_data *tsd = NULL;
						if( (tsd = ((TBL_PC*)target)) && !pc_issit(tsd) ) {
							pc_setsit(tsd);
							skill_sit(tsd,1);
							clif_sitting(&tsd->bl);
						}
					}
					break;
				case LG_OVERBRAND_BRANDISH:
				case LG_OVERBRAND_PLUSATK:
				case SR_KNUCKLEARROW:
					skill_attack(BF_WEAPON, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag|SD_LEVEL);
					break;
				case GN_SPORE_EXPLOSION:
					map_foreachinrange(skill_area_sub, target, skill_get_splash(skl->skill_id, skl->skill_lv), BL_CHAR,
									   src, skl->skill_id, skl->skill_lv, 0, skl->flag|1|BCT_ENEMY, skill_castend_damage_id);
					break;	
				case CH_PALMSTRIKE:
					{
						struct status_change* tsc = status_get_sc(target);
						if( tsc && tsc->option&OPTION_HIDE ){
							skill_blown(src,target,skill_get_blewcount(skl->skill_id, skl->skill_lv), -1, 0x0 );
							break;
						}
					}
				default:
					skill_attack(skl->type,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
					break;
			}
		}
		else {
			if(src->m != skl->map)
				break;
			switch( skl->skill_id )
			{
				case WZ_METEOR:
					if( skl->type >= 0 )
					{
						int x = skl->type>>16, y = skl->type&0xFFFF;
						if( path_search_long(NULL, src->m, src->x, src->y, x, y, CELL_CHKWALL) )
							skill_unitsetting(src,skl->skill_id,skl->skill_lv,x,y,skl->flag);
						if( path_search_long(NULL, src->m, src->x, src->y, skl->x, skl->y, CELL_CHKWALL) )
							clif_skill_poseffect(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,tick);
					}
					else if( path_search_long(NULL, src->m, src->x, src->y, skl->x, skl->y, CELL_CHKWALL) )
						skill_unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,skl->flag);
					break;
				case GN_CRAZYWEED_ATK:
					{
						int dummy = 1, i = skill_get_unit_range(skl->skill_id,skl->skill_lv);
						map_foreachinarea(skill_cell_overlap, src->m, skl->x-i, skl->y-i, skl->x+i, skl->y+i, BL_SKILL, skl->skill_id, &dummy, src);
					}
				case WL_EARTHSTRAIN:
					skill_unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,(skl->type<<16)|skl->flag);
					break;

			}
		}
	} while (0);
	//Free skl now that it is no longer needed.
	ers_free(skill_timer_ers, skl);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_addtimerskill (struct block_list *src, unsigned int tick, int target, int x,int y, int skill_id, int skill_lv, int type, int flag)
{
	int i;
	struct unit_data *ud;
	nullpo_retr(1, src);
	if (src->prev == NULL)
		return 0;
	ud = unit_bl2ud(src);
	nullpo_retr(1, ud);

	ARR_FIND( 0, MAX_SKILLTIMERSKILL, i, ud->skilltimerskill[i] == 0 );
	if( i == MAX_SKILLTIMERSKILL ) return 1;

	ud->skilltimerskill[i] = ers_alloc(skill_timer_ers, struct skill_timerskill);
	ud->skilltimerskill[i]->timer = add_timer(tick, skill_timerskill, src->id, i);
	ud->skilltimerskill[i]->src_id = src->id;
	ud->skilltimerskill[i]->target_id = target;
	ud->skilltimerskill[i]->skill_id = skill_id;
	ud->skilltimerskill[i]->skill_lv = skill_lv;
	ud->skilltimerskill[i]->map = src->m;
	ud->skilltimerskill[i]->x = x;
	ud->skilltimerskill[i]->y = y;
	ud->skilltimerskill[i]->type = type;
	ud->skilltimerskill[i]->flag = flag;
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_cleartimerskill (struct block_list *src)
{
	int i;
	struct unit_data *ud;
	nullpo_ret(src);
	ud = unit_bl2ud(src);
	nullpo_ret(ud);

	for(i=0;i<MAX_SKILLTIMERSKILL;i++) {
		if(ud->skilltimerskill[i]) {
			delete_timer(ud->skilltimerskill[i]->timer, skill_timerskill);
			ers_free(skill_timer_ers, ud->skilltimerskill[i]);
			ud->skilltimerskill[i]=NULL;
		}
	}
	return 1;
}
static int skill_ative_reverberation( struct block_list *bl, va_list ap) {
	struct skill_unit *su = (TBL_SKILL*)bl;
	struct skill_unit_group *sg;
	if( bl->type != BL_SKILL )
		return 0;
	if( su->alive && (sg = su->group) && sg->skill_id == WM_REVERBERATION ) {
		map_foreachinrange(skill_trap_splash, bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, bl, gettick());
		su->limit=DIFF_TICK(gettick(),sg->tick);
		sg->unit_id = UNT_USED_TRAPS;
	}
	return 0;
}

static int skill_reveal_trap (struct block_list *bl, va_list ap)
{
	TBL_SKILL *su = (TBL_SKILL*)bl;
	if (su->alive && su->group && skill_get_inf2(su->group->skill_id)&INF2_TRAP)
	{	//Reveal trap.
		//Change look is not good enough, the client ignores it as an actual trap still. [Skotlex]
		//clif_changetraplook(bl, su->group->unit_id);
		clif_skill_setunit(su);
		return 1;
	}
	return 0;
}

/*==========================================
 *
 *
 *------------------------------------------*/
int skill_castend_damage_id (struct block_list* src, struct block_list *bl, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct map_session_data *sd = NULL;
	struct status_data *tstatus;
	struct status_change *sc;

	if (skillid > 0 && skilllv <= 0) return 0;

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->m != bl->m)
		return 1;

	if (bl->prev == NULL)
		return 1;

	sd = BL_CAST(BL_PC, src);

	if (status_isdead(bl))
		return 1;

	if (skillid && skill_get_type(skillid) == BF_MAGIC && status_isimmune(bl) == 100)
	{	//GTB makes all targetted magic display miss with a single bolt.
		sc_type sct = status_skill2sc(skillid);
		if(sct != SC_NONE)
			status_change_end(bl, sct, INVALID_TIMER);
		clif_skill_damage(src, bl, tick, status_get_amotion(src), status_get_dmotion(bl), 0, 1, skillid, skilllv, skill_get_hit(skillid));
		return 1;
	}

	sc = status_get_sc(src);
	if (sc && !sc->count)
		sc = NULL; //Unneeded

	tstatus = status_get_status_data(bl);

	map_freeblock_lock();

	switch(skillid)
	{
	case MER_CRASH:
	case SM_BASH:
	case MS_BASH:
	case MC_MAMMONITE:
	case TF_DOUBLE:
	case AC_DOUBLE:
	case MA_DOUBLE:
	case AS_SONICBLOW:
	case KN_PIERCE:
	case ML_PIERCE:
	case KN_SPEARBOOMERANG:
	case TF_POISON:
	case TF_SPRINKLESAND:
	case AC_CHARGEARROW:
	case MA_CHARGEARROW:
	case RG_INTIMIDATE:
	case AM_ACIDTERROR:
	case BA_MUSICALSTRIKE:
	case DC_THROWARROW:
	case BA_DISSONANCE:
	case CR_HOLYCROSS:
	case NPC_DARKCROSS:
	case CR_SHIELDCHARGE:
	case CR_SHIELDBOOMERANG:
	case NPC_PIERCINGATT:
	case NPC_MENTALBREAKER:
	case NPC_RANGEATTACK:
	case NPC_CRITICALSLASH:
	case NPC_COMBOATTACK:
	case NPC_GUIDEDATTACK:
	case NPC_POISON:
	case NPC_RANDOMATTACK:
	case NPC_WATERATTACK:
	case NPC_GROUNDATTACK:
	case NPC_FIREATTACK:
	case NPC_WINDATTACK:
	case NPC_POISONATTACK:
	case NPC_HOLYATTACK:
	case NPC_DARKNESSATTACK:
	case NPC_TELEKINESISATTACK:
	case NPC_UNDEADATTACK:
	case NPC_ARMORBRAKE:
	case NPC_WEAPONBRAKER:
	case NPC_HELMBRAKE:
	case NPC_SHIELDBRAKE:
	case NPC_BLINDATTACK:
	case NPC_SILENCEATTACK:
	case NPC_STUNATTACK:
	case NPC_PETRIFYATTACK:
	case NPC_CURSEATTACK:
	case NPC_SLEEPATTACK:
	case LK_AURABLADE:
	case LK_SPIRALPIERCE:
	case ML_SPIRALPIERCE:
	case LK_HEADCRUSH:
	case CG_ARROWVULCAN:
	case HW_MAGICCRASHER:
	case ITM_TOMAHAWK:
	case MO_TRIPLEATTACK:
	case CH_CHAINCRUSH:
	case CH_TIGERFIST:
	case PA_SHIELDCHAIN:	// Shield Chain
	case PA_SACRIFICE:
	case WS_CARTTERMINATION:	// Cart Termination
	case AS_VENOMKNIFE:
	case HT_PHANTASMIC:
	case HT_POWER:
	case TK_DOWNKICK:
	case TK_COUNTER:
	case GS_CHAINACTION:
	case GS_TRIPLEACTION:
	case GS_MAGICALBULLET:
	case GS_TRACKING:
	case GS_PIERCINGSHOT:
	case GS_RAPIDSHOWER:
	case GS_DUST:
	case GS_DISARM:				// Added disarm. [Reddozen]
	case GS_FULLBUSTER:
	case NJ_SYURIKEN:
	case NJ_KUNAI:
	case ASC_BREAKER:
	case HFLI_MOON:	//[orn]
	case HFLI_SBR44:	//[orn]
	case NPC_BLEEDING:
	case NPC_CRITICALWOUND:
	case NPC_HELLPOWER:
	case RK_SONICWAVE:
	case RK_HUNDREDSPEAR:
	case AB_DUPLELIGHT_MELEE:
	case RA_AIMEDBOLT:
	case NC_AXEBOOMERANG:
	case NC_POWERSWING:
	case GC_CROSSIMPACT:
	case GC_VENOMPRESSURE:
	case SC_TRIANGLESHOT:
	case SC_FEINTBOMB:
	case LG_BANISHINGPOINT:
	case LG_SHIELDPRESS:
	case LG_RAGEBURST:
	case LG_RAYOFGENESIS:
	case LG_HESPERUSLIT:
	case SR_FALLENEMPIRE:
	case SR_CRESCENTELBOW_AUTOSPELL:
	case SR_GATEOFHELL:
	case SR_GENTLETOUCH_QUIET:
	case WM_SEVERE_RAINSTORM_MELEE:
	case WM_GREAT_ECHO:
	case GN_SLINGITEM_RANGEMELEEATK:
	case MH_STAHL_HORN:
	case KO_JYUMONJIKIRI:
	case KO_SETSUDAN:
	case KO_KAIHOU:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
	break;

	/**
	 * Mechanic (MADO GEAR)
	 **/
	case NC_BOOSTKNUCKLE:
	case NC_PILEBUNKER:
	case NC_VULCANARM:
	case NC_COLDSLOWER:
	case NC_ARMSCANNON:
		if (sd) pc_overheat(sd,1);
	case RK_WINDCUTTER:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag|SD_ANIMATION);
		break;

	case LK_JOINTBEAT: // decide the ailment first (affects attack damage and effect)
		switch( rnd()%6 ){
		case 0: flag |= BREAK_ANKLE; break;
		case 1: flag |= BREAK_WRIST; break;
		case 2: flag |= BREAK_KNEE; break;
		case 3: flag |= BREAK_SHOULDER; break;
		case 4: flag |= BREAK_WAIST; break;
		case 5: flag |= BREAK_NECK; break;
		}
		//TODO: is there really no cleaner way to do this?
		sc = status_get_sc(bl);
		if (sc) sc->jb_flag = flag;
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case MO_COMBOFINISH:
		if (!(flag&1) && sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_MONK)
		{	//Becomes a splash attack when Soul Linked.
			map_foreachinrange(skill_area_sub, bl,
				skill_get_splash(skillid, skilllv),splash_target(src),
				src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
				skill_castend_damage_id);
		} else
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case TK_STORMKICK: // Taekwon kicks [Dralnu]
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		skill_area_temp[1] = 0;
		map_foreachinrange(skill_attack_area, src,
			skill_get_splash(skillid, skilllv), splash_target(src),
			BF_WEAPON, src, src, skillid, skilllv, tick, flag, BCT_ENEMY);
		break;

	case KN_CHARGEATK:
		{
		bool path = path_search_long(NULL, src->m, src->x, src->y, bl->x, bl->y,CELL_CHKWALL);
		unsigned int dist = distance_bl(src, bl);
		unsigned int dir = map_calc_dir(bl, src->x, src->y);

		// teleport to target (if not on WoE grounds)
		if( !map_flag_gvg(src->m) && !map[src->m].flag.battleground && unit_movepos(src, bl->x, bl->y, 0, 1) )
			clif_slide(src, bl->x, bl->y);

		// cause damage and knockback if the path to target was a straight one
		if( path )
		{
			skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, dist);
			skill_blown(src, bl, dist, dir, 0);
			//HACK: since knockback officially defaults to the left, the client also turns to the left... therefore,
			// make the caster look in the direction of the target
			unit_setdir(src, (dir+4)%8);
		}

		}
		break;

	case NC_FLAMELAUNCHER:
		if (sd) pc_overheat(sd,1);
	case SN_SHARPSHOOTING:
	case MA_SHARPSHOOTING:
	case NJ_KAMAITACHI:
	case LG_CANNONSPEAR:
		//It won't shoot through walls since on castend there has to be a direct
		//line of sight between caster and target.
		skill_area_temp[1] = bl->id;
		map_foreachinpath (skill_attack_area,src->m,src->x,src->y,bl->x,bl->y,
			skill_get_splash(skillid, skilllv),skill_get_maxcount(skillid,skilllv), splash_target(src),
			skill_get_type(skillid),src,src,skillid,skilllv,tick,flag,BCT_ENEMY);
		break;

	case NPC_ACIDBREATH:
	case NPC_DARKNESSBREATH:
	case NPC_FIREBREATH:
	case NPC_ICEBREATH:
	case NPC_THUNDERBREATH:
		skill_area_temp[1] = bl->id;
		map_foreachinpath(skill_attack_area,src->m,src->x,src->y,bl->x,bl->y,
			skill_get_splash(skillid, skilllv),skill_get_maxcount(skillid,skilllv), splash_target(src),
			skill_get_type(skillid),src,src,skillid,skilllv,tick,flag,BCT_ENEMY);
		break;

	case MO_INVESTIGATE:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
		break;

	case RG_BACKSTAP:
		{
			int dir = map_calc_dir(src, bl->x, bl->y), t_dir = unit_getdir(bl);
			if ((!check_distance_bl(src, bl, 0) && !map_check_dir(dir, t_dir)) || bl->type == BL_SKILL) {
				status_change_end(src, SC_HIDING, INVALID_TIMER);
				skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
				dir = dir < 4 ? dir+4 : dir-4; // change direction [Celest]
				unit_setdir(bl,dir);
			}
			else if (sd)
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;

	case MO_FINGEROFFENSIVE:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		if (battle_config.finger_offensive_type && sd) {
			int i;
			for (i = 1; i < sd->spiritball_old; i++)
				skill_addtimerskill(src, tick + i * 200, bl->id, 0, 0, skillid, skilllv, BF_WEAPON, flag);
		}
		status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
		break;

	case MO_CHAINCOMBO:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
		break;

	case NJ_ISSEN:
		status_change_end(src, SC_NEN, INVALID_TIMER);
		status_change_end(src, SC_HIDING, INVALID_TIMER);
		// fall through
	case MO_EXTREMITYFIST:
		if( skillid == MO_EXTREMITYFIST )
		{
			status_change_end(src, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
			status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
#ifdef RENEWAL
			sc_start(src,SC_EXTREMITYFIST2,100,skilllv,skill_get_time(skillid,skilllv));
#endif
		}
		//Client expects you to move to target regardless of distance
		{
			struct unit_data *ud = unit_bl2ud(src);
			short dx,dy;
			int i,speed;
			i = skillid == MO_EXTREMITYFIST?1:2; //Move 2 cells for Issen, 1 for Asura
			dx = bl->x - src->x;
			dy = bl->y - src->y;
			if (dx < 0) dx-=i;
			else if (dx > 0) dx+=i;
			if (dy < 0) dy-=i;
			else if (dy > 0) dy+=i;
			if (!dx && !dy) dy++;
			if (map_getcell(src->m, src->x+dx, src->y+dy, CELL_CHKNOPASS))
			{
				dx = bl->x;
				dy = bl->y;
			} else {
				dx = src->x + dx;
				dy = src->y + dy;
			}

			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);

			if(unit_walktoxy(src, dx, dy, 2) && ud) {
				//Increase can't walk delay to not alter your walk path
				ud->canmove_tick = tick;
				speed = status_get_speed(src);
				for (i = 0; i < ud->walkpath.path_len; i ++)
				{
					if(ud->walkpath.path[i]&1)
						ud->canmove_tick+=7*speed/5;
					else
						ud->canmove_tick+=speed;
				}
			}
		}
		break;

	//Splash attack skills.
	case AS_GRIMTOOTH:
	case MC_CARTREVOLUTION:
	case NPC_SPLASHATTACK:
		flag |= SD_PREAMBLE; // a fake packet will be sent for the first target to be hit
	case AS_SPLASHER:
	case SM_MAGNUM:
	case MS_MAGNUM:
	case HT_BLITZBEAT:
	case AC_SHOWER:
	case MA_SHOWER:
	case MG_NAPALMBEAT:
	case MG_FIREBALL:
	case RG_RAID:
	case HW_NAPALMVULCAN:
	case NJ_HUUMA:
	case NJ_BAKUENRYU:
	case ASC_METEORASSAULT:
	case GS_DESPERADO:
	case GS_SPREADATTACK:
	case NPC_EARTHQUAKE:
	case NPC_PULSESTRIKE:
	case NPC_HELLJUDGEMENT:
	case NPC_VAMPIRE_GIFT:
	case RK_IGNITIONBREAK:
	case AB_JUDEX:
	case WL_SOULEXPANSION:
	case WL_CRIMSONROCK:
	case WL_COMET:
	case WL_JACKFROST:
	case RA_ARROWSTORM:
	case RA_WUGDASH:
	case NC_SELFDESTRUCTION:
	case NC_AXETORNADO:
	case GC_ROLLINGCUTTER:
	case GC_COUNTERSLASH:
	case LG_MOONSLASHER:
	case LG_EARTHDRIVE:
	case SR_TIGERCANNON:
	case SR_RAMPAGEBLASTER:
	case SR_SKYNETBLOW:
	case SR_WINDMILL:
	case SR_RIDEINLIGHTNING:
	case WM_SOUND_OF_DESTRUCTION:
	case WM_REVERBERATION_MELEE:
	case WM_REVERBERATION_MAGIC:
	case SO_VARETYR_SPEAR:
	case GN_CART_TORNADO:
	case GN_CARTCANNON:
	case MH_LAVA_SLIDE:
	case KO_HAPPOKUNAI:
	case KO_HUUMARANKA:
	case KO_MUCHANAGE:
	case KO_BAKURETSU:
		if( flag&1 ) {//Recursive invocation
			// skill_area_temp[0] holds number of targets in area
			// skill_area_temp[1] holds the id of the original target
			// skill_area_temp[2] counts how many targets have already been processed
			int sflag = skill_area_temp[0] & 0xFFF, heal;
			if( flag&SD_LEVEL )
				sflag |= SD_LEVEL; // -1 will be used in packets instead of the skill level
			if( skill_area_temp[1] != bl->id && !(skill_get_inf2(skillid)&INF2_NPC_SKILL) )
				sflag |= SD_ANIMATION; // original target gets no animation (as well as all NPC skills)

			heal = skill_attack(skill_get_type(skillid), src, src, bl, skillid, skilllv, tick, sflag);
			if( skillid == NPC_VAMPIRE_GIFT && heal > 0 ) {
				clif_skill_nodamage(NULL, src, AL_HEAL, heal, 1);
				status_heal(src,heal,0,0);
			}
		} else {
			switch ( skillid ) {
				case NJ_BAKUENRYU:
				case LG_EARTHDRIVE:
				case GN_CARTCANNON:
					clif_skill_nodamage(src,bl,skillid,skilllv,1);
					break;
				case LG_MOONSLASHER:
					clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
					break;
				case NPC_EARTHQUAKE://FIXME: Isn't EarthQuake a ground skill after all?
					skill_addtimerskill(src,tick+250,src->id,0,0,skillid,skilllv,2,flag|BCT_ENEMY|SD_SPLASH|1);
				default:
					break;
			}
			
			skill_area_temp[0] = 0;
			skill_area_temp[1] = bl->id;
			skill_area_temp[2] = 0;
			if( skillid == WL_CRIMSONROCK ) {
				skill_area_temp[4] = bl->x;
				skill_area_temp[5] = bl->y;
			}
			if( skillid == WM_REVERBERATION_MELEE || skillid == WM_REVERBERATION_MAGIC )
				skill_area_temp[1] = 0;
			// if skill damage should be split among targets, count them
			//SD_LEVEL -> Forced splash damage for Auto Blitz-Beat -> count targets
			//special case: Venom Splasher uses a different range for searching than for splashing
			if( flag&SD_LEVEL || skill_get_nk(skillid)&NK_SPLASHSPLIT )
				skill_area_temp[0] = map_foreachinrange(skill_area_sub, bl, (skillid == AS_SPLASHER)?1:skill_get_splash(skillid, skilllv), BL_CHAR, src, skillid, skilllv, tick, BCT_ENEMY, skill_area_sub_count);

			// recursive invocation of skill_castend_damage_id() with flag|1
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), ( skillid == WM_REVERBERATION_MELEE || skillid == WM_REVERBERATION_MAGIC )?BL_CHAR:splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		}
		break;

	case KN_BRANDISHSPEAR:
	case ML_BRANDISH:
		//Coded apart for it needs the flag passed to the damage calculation.
		if (skill_area_temp[1] != bl->id)
			skill_attack(skill_get_type(skillid), src, src, bl, skillid, skilllv, tick, flag|SD_ANIMATION);
		else
			skill_attack(skill_get_type(skillid), src, src, bl, skillid, skilllv, tick, flag);
		break;

	case KN_BOWLINGBASH:
	case MS_BOWLINGBASH:
		if(flag&1){
			if(bl->id==skill_area_temp[1])
				break;
			//two hits for 500%
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,SD_ANIMATION);
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,SD_ANIMATION);
		} else {
			int i,c;
			c = skill_get_blewcount(skillid,skilllv);
			// keep moving target in the direction that src is looking, square by square
			for(i=0;i<c;i++){
				if (!skill_blown(src,bl,1,(unit_getdir(src)+4)%8,0x1))
					break; //Can't knockback
				skill_area_temp[0] = map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), BL_CHAR, src, skillid, skilllv, tick, flag|BCT_ENEMY, skill_area_sub_count);
				if( skill_area_temp[0] > 1 ) break; // collision
			}
			clif_blown(bl); //Update target pos.
			if (i!=c) { //Splash
				skill_area_temp[1] = bl->id;
				map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
			}
			//Weirdo dual-hit property, two attacks for 500%
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0);
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0);
		}
		break;

	case KN_SPEARSTAB:
		if(flag&1) {
			if (bl->id==skill_area_temp[1])
				break;
			if (skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,SD_ANIMATION))
				skill_blown(src,bl,skill_area_temp[2],-1,0);
		} else {
			int x=bl->x,y=bl->y,i,dir;
			dir = map_calc_dir(bl,src->x,src->y);
			skill_area_temp[1] = bl->id;
			skill_area_temp[2] = skill_get_blewcount(skillid,skilllv);
			// all the enemies between the caster and the target are hit, as well as the target
			if (skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0))
				skill_blown(src,bl,skill_area_temp[2],-1,0);
			for (i=0;i<4;i++) {
				map_foreachincell(skill_area_sub,bl->m,x,y,BL_CHAR,
					src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
				x += dirx[dir];
				y += diry[dir];
			}
		}
		break;

	case TK_TURNKICK:
	case MO_BALKYOUNG: //Active part of the attack. Skill-attack [Skotlex]
	{
		skill_area_temp[1] = bl->id; //NOTE: This is used in skill_castend_nodamage_id to avoid affecting the target.
		if (skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag))
			map_foreachinrange(skill_area_sub,bl,
				skill_get_splash(skillid, skilllv),BL_CHAR,
				src,skillid,skilllv,tick,flag|BCT_ENEMY|1,
				skill_castend_nodamage_id);
	}
		break;
	case CH_PALMSTRIKE: //	Palm Strike takes effect 1sec after casting. [Skotlex]
	//	clif_skill_nodamage(src,bl,skillid,skilllv,0); //Can't make this one display the correct attack animation delay :/
		clif_damage(src,bl,tick,status_get_amotion(src),0,-1,1,4,0); //Display an absorbed damage attack.
		skill_addtimerskill(src, tick + 1000, bl->id, 0, 0, skillid, skilllv, BF_WEAPON, flag);
		break;

	case PR_TURNUNDEAD:
	case ALL_RESURRECTION:
		if (!battle_check_undead(tstatus->race, tstatus->def_ele))
			break;
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case MG_SOULSTRIKE:
	case NPC_DARKSTRIKE:
	case MG_COLDBOLT:
	case MG_FIREBOLT:
	case MG_LIGHTNINGBOLT:
	case WZ_EARTHSPIKE:
	case AL_HEAL:
	case AL_HOLYLIGHT:
	case WZ_JUPITEL:
	case NPC_DARKTHUNDER:
	case PR_ASPERSIO:
	case MG_FROSTDIVER:
	case WZ_SIGHTBLASTER:
	case WZ_SIGHTRASHER:
	case NJ_KOUENKA:
	case NJ_HYOUSENSOU:
	case NJ_HUUJIN:
	case AB_ADORAMUS:
	case AB_RENOVATIO:
	case AB_HIGHNESSHEAL:
	case AB_DUPLELIGHT_MAGIC:
	case WM_METALICSOUND:
	case MH_ERASER_CUTTER:
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case NPC_MAGICALATTACK:
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		sc_start(src,status_skill2sc(skillid),100,skilllv,skill_get_time(skillid,skilllv));
		break;

	case HVAN_CAPRICE: //[blackhole89]
		{
			int ran=rnd()%4;
			int sid = 0;
			switch(ran)
			{
			case 0: sid=MG_COLDBOLT; break;
			case 1: sid=MG_FIREBOLT; break;
			case 2: sid=MG_LIGHTNINGBOLT; break;
			case 3: sid=WZ_EARTHSPIKE; break;
			}
			skill_attack(BF_MAGIC,src,src,bl,sid,skilllv,tick,flag|SD_LEVEL);
		}
		break;
	case WZ_WATERBALL:
		{
			int range = skilllv / 2;
			int maxlv = skill_get_max(skillid); // learnable level
			int count = 0;
			int x, y;
			struct skill_unit* unit;

			if( skilllv > maxlv )
			{
				if( src->type == BL_MOB && skilllv == 10 )
					range = 4;
				else
					range = maxlv / 2;
			}

			for( y = src->y - range; y <= src->y + range; ++y )
				for( x = src->x - range; x <= src->x + range; ++x )
				{
					if( !map_find_skill_unit_oncell(src,x,y,SA_LANDPROTECTOR,NULL,1) )
					{
						if( src->type != BL_PC || map_getcell(src->m,x,y,CELL_CHKWATER) ) // non-players bypass the water requirement
							count++; // natural water cell
						else if( (unit = map_find_skill_unit_oncell(src,x,y,SA_DELUGE,NULL,1)) != NULL || (unit = map_find_skill_unit_oncell(src,x,y,NJ_SUITON,NULL,1)) != NULL )
						{
							count++; // skill-induced water cell
							skill_delunit(unit); // consume cell
						}
					}
				}

			if( count > 1 ) // queue the remaining count - 1 timerskill Waterballs
				skill_addtimerskill(src,tick+150,bl->id,0,0,skillid,skilllv,count-1,flag);
		}
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case PR_BENEDICTIO:
		//Should attack undead and demons. [Skotlex]
		if (battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON)
			skill_attack(BF_MAGIC, src, src, bl, skillid, skilllv, tick, flag);
	break;

	case SL_SMA:
		status_change_end(src, SC_SMA, INVALID_TIMER);
	case SL_STIN:
	case SL_STUN:
		if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
			status_change_start(src,SC_STUN,10000,skilllv,0,0,0,500,10);
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case NPC_DARKBREATH:
		clif_emotion(src,E_AG);
	case SN_FALCONASSAULT:
	case PA_PRESSURE:
	case CR_ACIDDEMONSTRATION:
	case TF_THROWSTONE:
	case NPC_SMOKING:
	case GS_FLING:
	case NJ_ZENYNAGE:
	case GN_THORNS_TRAP:
	case GN_HELLS_PLANT_ATK:			
		skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag);
		break;
	/**
	 * Rune Knight
	 **/
	case RK_DRAGONBREATH: {
			struct status_change *tsc = NULL;
			if( (tsc = status_get_sc(bl)) && (tsc->data[SC_HIDING] )) {
				clif_skill_nodamage(src,src,skillid,skilllv,1);
			} else 
				skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag);
		}
		break;

	case NPC_SELFDESTRUCTION: {
		struct status_change *tsc = NULL;
		if( (tsc = status_get_sc(bl)) && tsc->data[SC_HIDING] )
			break;
		}
	case HVAN_EXPLOSION:
		if (src != bl)
			skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	// Celest
	case PF_SOULBURN:
		if (rnd()%100 < (skilllv < 5 ? 30 + skilllv * 10 : 70)) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if (skilllv == 5)
				skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
			status_percent_damage(src, bl, 0, 100, false);
		} else {
			clif_skill_nodamage(src,src,skillid,skilllv,1);
			if (skilllv == 5)
				skill_attack(BF_MAGIC,src,src,src,skillid,skilllv,tick,flag);
			status_percent_damage(src, src, 0, 100, false);
		}
		break;

	case NPC_BLOODDRAIN:
	case NPC_ENERGYDRAIN:
		{
			int heal = skill_attack( (skillid == NPC_BLOODDRAIN) ? BF_WEAPON : BF_MAGIC,
					src, src, bl, skillid, skilllv, tick, flag);
			if (heal > 0){
				clif_skill_nodamage(NULL, src, AL_HEAL, heal, 1);
				status_heal(src, heal, 0, 0);
			}
		}
		break;

	case GS_BULLSEYE:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case NJ_KASUMIKIRI:
		if (skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag) > 0)
			sc_start(src,SC_HIDING,100,skilllv,skill_get_time(skillid,skilllv));
		break;
	case NJ_KIRIKAGE:
		if( !map_flag_gvg(src->m) && !map[src->m].flag.battleground )
		{	//You don't move on GVG grounds.
			short x, y;
			map_search_freecell(bl, 0, &x, &y, 1, 1, 0);
			if (unit_movepos(src, x, y, 0, 0))
				clif_slide(src,src->x,src->y);
		}
		status_change_end(src, SC_HIDING, INVALID_TIMER);
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;
	case RK_PHANTOMTHRUST:
		unit_setdir(src,map_calc_dir(src, bl->x, bl->y));
		clif_skill_nodamage(src,bl,skillid,skilllv,1);

		skill_blown(src,bl,distance_bl(src,bl)-1,unit_getdir(src),0);
		if( battle_check_target(src,bl,BCT_ENEMY) )
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;
		
	case RK_STORMBLAST:
	case RK_CRUSHSTRIKE:
		if( sd ) {
			if( pc_checkskill(sd,RK_RUNEMASTERY) >= ( skillid == RK_CRUSHSTRIKE ? 7 : 3 ) )
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
			else
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		} else //non-sd support
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;
	case GC_DARKILLUSION:
		{
			short x, y;
			short dir = map_calc_dir(src,bl->x,bl->y);

			if( dir > 4 ) x = -1;
			else if( dir > 0 && dir < 4 ) x = 1;
			else x = 0;
			if( dir < 3 || dir > 5 ) y = -1;
			else if( dir > 3 && dir < 5 ) y = 1;
			else y = 0;

			if( unit_movepos(src, bl->x+x, bl->y+y, 1, 1) )
			{
				clif_slide(src,bl->x+x,bl->y+y);
				clif_fixpos(src); // the official server send these two packts.
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
				if( rnd()%100 < 4 * skilllv )
					skill_castend_damage_id(src,bl,GC_CROSSIMPACT,skilllv,tick,flag);
			}

		}
		break;

	case GC_WEAPONCRUSH:
		if( sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == GC_WEAPONBLOCKING )
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		else if( sd )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_GC_WEAPONBLOCKING,0);
		break;

	case GC_CROSSRIPPERSLASHER:
		if( sd && !(sc && sc->data[SC_ROLLINGCUTTER]) )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_CONDITION,0);
		else
		{
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
			status_change_end(src,SC_ROLLINGCUTTER,INVALID_TIMER);
		}
		break;

	case GC_PHANTOMMENACE:
		if( flag&1 )
		{ // Only Hits Invisible Targets
			struct status_change *tsc = status_get_sc(bl);
			if(tsc && (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || tsc->data[SC__INVISIBILITY]) )
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		}
		break;
	case WL_CHAINLIGHTNING:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		skill_addtimerskill(src,tick + 150,bl->id,3,0,WL_CHAINLIGHTNING_ATK,skilllv,4+skilllv,flag);
		break;
	case WL_DRAINLIFE:
		{
		    int heal = skill_attack(skill_get_type(skillid), src, src, bl, skillid, skilllv, tick, flag);
		    int rate = 70 + 5 * skilllv;

		    heal = heal * (5 + 5 * skilllv) / 100;

		    if( bl->type == BL_SKILL )
				heal = 0; // Don't absorb heal from Ice Walls or other skill units.

		    if( heal && rnd()%100 < rate )
			{
				status_heal(src, heal, 0, 0);
				clif_skill_nodamage(NULL, src, AL_HEAL, heal, 1);
			}
		}
		break;

	case WL_TETRAVORTEX:
		if( sd )
		{
			int spheres[5] = { 0, 0, 0, 0, 0 },
				positions[5] = {-1,-1,-1,-1,-1 },
				i, j = 0, k, subskill = 0;

			for( i = SC_SPHERE_1; i <= SC_SPHERE_5; i++ )
				if( sc && sc->data[i] )
				{
					spheres[j] = i;
					positions[j] = sc->data[i]->val2;
					j++; // 
				}

			if( j < 4 )
			{ // Need 4 spheres minimum
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}

			// Sphere Sort, this time from new to old
			for( i = 0; i <= j - 2; i++ )
				for( k = i + 1; k <= j - 1; k++ )
					if( positions[i] < positions[k] )
					{
						swap(positions[i],positions[k]);
						swap(spheres[i],spheres[k]);
					}

			k = 0;
			for( i = 0; i < 4; i++ )
			{
				switch( sc->data[spheres[i]]->val1 )
				{
					case WLS_FIRE:  subskill = WL_TETRAVORTEX_FIRE; k |= 1; break;
					case WLS_WIND:  subskill = WL_TETRAVORTEX_WIND; k |= 4; break;
					case WLS_WATER: subskill = WL_TETRAVORTEX_WATER; k |= 2; break;
					case WLS_STONE: subskill = WL_TETRAVORTEX_GROUND; k |= 8; break;
				}

				skill_addtimerskill(src,tick+status_get_adelay(src)*i,bl->id,k,0,subskill,skilllv,i,flag);
				status_change_end(src, spheres[i], INVALID_TIMER);
			}
		}
		break;

	case WL_RELEASE:
		if( sd )
		{
			int i;
			// Priority is to release SpellBook
			if( sc && sc->data[SC_READING_SB] )
			{ // SpellBook
				int skill_id, skill_lv, point, s = 0;
				int spell[SC_MAXSPELLBOOK-SC_SPELLBOOK1 + 1];

				for(i = SC_MAXSPELLBOOK; i >= SC_SPELLBOOK1; i--) // List all available spell to be released
					if( sc->data[i] ) spell[s++] = i;

				if ( s == 0 )
					break;

				i = spell[s==1?0:rand()%s];// Random select of spell to be released.
				if( s && sc->data[i] ){// Now extract the data from the preserved spell
					skill_id = sc->data[i]->val1; 
					skill_lv = sc->data[i]->val2;
					point = sc->data[i]->val3;
					status_change_end(src, (sc_type)i, INVALID_TIMER);
				}else //something went wrong :(
					break;

				if( sc->data[SC_READING_SB]->val2 > point )
					sc->data[SC_READING_SB]->val2 -= point;
				else // Last spell to be released 
					status_change_end(src, SC_READING_SB, INVALID_TIMER);

				clif_skill_nodamage(src, bl, skillid, skilllv, 1);
				if( !skill_check_condition_castbegin(sd, skill_id, skill_lv) )
					break;

				switch( skill_get_casttype(skill_id) )
				{
					case CAST_GROUND:
						skill_castend_pos2(src, bl->x, bl->y, skill_id, skill_lv, tick, 0);
						break;
					case CAST_NODAMAGE:
						skill_castend_nodamage_id(src, bl, skill_id, skill_lv, tick, 0);
						break;
					case CAST_DAMAGE:
						skill_castend_damage_id(src, bl, skill_id, skill_lv, tick, 0);
						break;
				}

				sd->ud.canact_tick = tick + skill_delayfix(src, skill_id, skill_lv);
				clif_status_change(src, SI_ACTIONDELAY, 1, skill_delayfix(src, skill_id, skill_lv), 0, 0, 0);
			}
			else
			{ // Summon Balls
				int j = 0, k, skele;
				int spheres[5] = { 0, 0, 0, 0, 0 },
					positions[5] = {-1,-1,-1,-1,-1 };

				for( i = SC_SPHERE_1; i <= SC_SPHERE_5; i++ )
					if( sc && sc->data[i] )
					{
						spheres[j] = i;
						positions[j] = sc->data[i]->val2;
						sc->data[i]->val2--; // Prepares for next position
						j++;
					}

				if( j == 0 )
				{ // No Spheres
					clif_skill_fail(sd,skillid,USESKILL_FAIL_SUMMON_NONE,0);
					break;
				}
				
				// Sphere Sort
				for( i = 0; i <= j - 2; i++ )
					for( k = i + 1; k <= j - 1; k++ )
						if( positions[i] > positions[k] )
						{
							swap(positions[i],positions[k]);
							swap(spheres[i],spheres[k]);
						}

				if( skilllv == 1 ) j = 1; // Limit only to one ball
				for( i = 0; i < j; i++ )
				{
					skele = WL_RELEASE - 5 + sc->data[spheres[i]]->val1 - WLS_FIRE; // Convert Ball Element into Skill ATK for balls
					// WL_SUMMON_ATK_FIRE, WL_SUMMON_ATK_WIND, WL_SUMMON_ATK_WATER, WL_SUMMON_ATK_GROUND
					skill_addtimerskill(src,tick+status_get_adelay(src)*i,bl->id,0,0,skele,sc->data[spheres[i]]->val3,BF_MAGIC,flag|SD_LEVEL);
					status_change_end(src, spheres[i], INVALID_TIMER); // Eliminate ball
				}
				clif_skill_nodamage(src,bl,skillid,0,1);
			}
		}
		break;
	case WL_FROSTMISTY:
		// Causes Freezing status through walls.
		sc_start(bl,status_skill2sc(skillid),20+12*skilllv+(sd ? sd->status.job_level : 50)/5,skilllv,skill_get_time(skillid,skilllv));
		// Doesn't deal damage through non-shootable walls.
		if( path_search(NULL,src->m,src->x,src->y,bl->x,bl->y,1,CELL_CHKWALL) )
			skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag|SD_ANIMATION);
		break;
	case WL_HELLINFERNO:
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag|ELE_DARK);
		break;
	case RA_WUGSTRIKE:
		if( sd && pc_isridingwug(sd) ){
			short x[8]={0,-1,-1,-1,0,1,1,1};
			short y[8]={1,1,0,-1,-1,-1,0,1};
			int dir = map_calc_dir(bl, src->x, src->y);

			if( unit_movepos(src, bl->x+x[dir], bl->y+y[dir], 1, 1) )
			{
				clif_slide(src, bl->x+x[dir], bl->y+y[dir]);
				clif_fixpos(src);
				skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
			}
			break;
		}
	case RA_WUGBITE:
		if( path_search(NULL,src->m,src->x,src->y,bl->x,bl->y,1,CELL_CHKNOREACH) ) {
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		}else if( sd && skillid == RA_WUGBITE ) // Only RA_WUGBITE has the skill fail message.
			clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);

		break;

	case RA_SENSITIVEKEEN:
		if( bl->type != BL_SKILL ) { // Only Hits Invisible Targets
			struct status_change * tsc = status_get_sc(bl);
			if( tsc && tsc->option&(OPTION_HIDE|OPTION_CLOAK) ){
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
				status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
			}
		}
		else
		{
			struct skill_unit *su = BL_CAST(BL_SKILL,bl);
			struct skill_unit_group* sg;

			if( su && (sg=su->group) && skill_get_inf2(sg->skill_id)&INF2_TRAP )
			{
				if( !(sg->unit_id == UNT_USED_TRAPS || (sg->unit_id == UNT_ANKLESNARE && sg->val2 != 0 )) )
				{ 
					struct item item_tmp;
					memset(&item_tmp,0,sizeof(item_tmp));
					item_tmp.nameid = sg->item_id?sg->item_id:ITEMID_TRAP;
					item_tmp.identify = 1;
					if( item_tmp.nameid )
						map_addflooritem(&item_tmp,1,bl->m,bl->x,bl->y,0,0,0,0);
				}
				skill_delunit(su);
			}
		}
		break;
	case NC_INFRAREDSCAN:
		if( flag&1 )
		{ //TODO: Need a confirmation if the other type of hidden status is included to be scanned. [Jobbie]
			if( rnd()%100 < 50 )
				sc_start(bl, SC_INFRAREDSCAN, 10000, skilllv, skill_get_time(skillid, skilllv));
			status_change_end(bl, SC_HIDING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER); // Need confirm it.
		}
		else
		{
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
			clif_skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			if( sd ) pc_overheat(sd,1);
		}
		break;

	case NC_MAGNETICFIELD:
		sc_start2(bl,SC_MAGNETICFIELD,100,skilllv,src->id,skill_get_time(skillid,skilllv));
		break;
	case SC_FATALMENACE:
		if( flag&1 )
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		else
		{
			short x, y;
			map_search_freecell(src, 0, &x, &y, -1, -1, 0);
			// Destination area
			skill_area_temp[4] = x;
			skill_area_temp[5] = y;
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
			skill_addtimerskill(src,tick + 800,src->id,x,y,skillid,skilllv,0,flag); // To teleport Self
			clif_skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skillid,skilllv,6);
		}
		break;
	case LG_PINPOINTATTACK:
		if( !map_flag_gvg(src->m) && !map[src->m].flag.battleground && unit_movepos(src, bl->x, bl->y, 1, 1) )
			clif_slide(src,bl->x,bl->y);
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case LG_SHIELDSPELL:
		// flag&1: Phisycal Attack, flag&2: Magic Attack.
		skill_attack((flag&1)?BF_WEAPON:BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case LG_OVERBRAND:
		skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag|SD_LEVEL);
		break;

	case LG_OVERBRAND_BRANDISH:
		skill_addtimerskill(src, tick + status_get_amotion(src)*8/10, bl->id, 0, 0, skillid, skilllv, BF_WEAPON, flag|SD_LEVEL);
		break;
	case SR_DRAGONCOMBO:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case SR_KNUCKLEARROW:
			if( !map_flag_gvg(src->m) && !map[src->m].flag.battleground && unit_movepos(src, bl->x, bl->y, 1, 1) ) {
				clif_slide(src,bl->x,bl->y);
				clif_fixpos(src); // Aegis send this packet too.
			}

			if( flag&1 )
				skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag|SD_LEVEL);
			else
				skill_addtimerskill(src, tick + 300, bl->id, 0, 0, skillid, skilllv, BF_WEAPON, flag|SD_LEVEL|2);
		break;

	case SR_HOWLINGOFLION:
			status_change_end(bl, SC_SWINGDANCE, INVALID_TIMER);
			status_change_end(bl, SC_SYMPHONYOFLOVER, INVALID_TIMER);
			status_change_end(bl, SC_MOONLITSERENADE, INVALID_TIMER);
			status_change_end(bl, SC_RUSHWINDMILL, INVALID_TIMER);
			status_change_end(bl, SC_ECHOSONG, INVALID_TIMER);
			status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
			status_change_end(bl, SC_SIRCLEOFNATURE, INVALID_TIMER);
			status_change_end(bl, SC_SATURDAYNIGHTFEVER, INVALID_TIMER);
			status_change_end(bl, SC_DANCEWITHWUG, INVALID_TIMER);
			status_change_end(bl, SC_LERADSDEW, INVALID_TIMER);
			status_change_end(bl, SC_MELODYOFSINK, INVALID_TIMER);
			status_change_end(bl, SC_BEYONDOFWARCRY, INVALID_TIMER);
			status_change_end(bl, SC_UNLIMITEDHUMMINGVOICE, INVALID_TIMER);
			skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag|SD_ANIMATION);
		break;

	case SR_EARTHSHAKER:
		if( flag&1 ) { //by default cloaking skills are remove by aoe skills so no more checking/removing except hiding and cloaking exceed.
			status_change_end(bl, SC_HIDING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
			skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
		} else{
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			}
		break;

	case WM_LULLABY_DEEPSLEEP:
		if( bl != src && rnd()%100 < 88 + 2 * skilllv )
			sc_start(bl,status_skill2sc(skillid),100,skilllv,skill_get_time(skillid,skilllv));
		break;

	case SO_POISON_BUSTER: {
			struct status_change *tsc = status_get_sc(bl);
			if( tsc && tsc->data[SC_POISON] ) {
				skill_attack(skill_get_type(skillid), src, src, bl, skillid, skilllv, tick, flag);
				status_change_end(bl, SC_POISON, INVALID_TIMER);
			}
			else if( sd )
				clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);
		}
		break;
		
	case GN_SPORE_EXPLOSION:
		if( flag&1 )
			skill_attack(skill_get_type(skillid), src, src, bl, skillid, skilllv, tick, flag);
		else {
			clif_skill_nodamage(src, bl, skillid, 0, 1);
			skill_addtimerskill(src, gettick() + skill_get_time(skillid, skilllv) - 1000, bl->id, 0, 0, skillid, skilllv, 0, 0);
		}
		break;
				
	case EL_FIRE_BOMB:
	case EL_FIRE_WAVE:
	case EL_WATER_SCREW:
	case EL_HURRICANE:
	case EL_TYPOON_MIS:
		if( flag&1 )
			skill_attack(skill_get_type(skillid+1),src,src,bl,skillid+1,skilllv,tick,flag);
		else {
			int i = skill_get_splash(skillid,skilllv);
			clif_skill_nodamage(src,battle_get_master(src),skillid,skilllv,1);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			if( rnd()%100 < 30 )
				map_foreachinrange(skill_area_sub,bl,i,BL_CHAR,src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
			else
				skill_attack(skill_get_type(skillid),src,src,bl,skillid,skilllv,tick,flag);
		}
		break;
		
	case EL_ROCK_CRUSHER:
		clif_skill_nodamage(src,battle_get_master(src),skillid,skilllv,1);
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		if( rnd()%100 < 50 )
			skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		else
			skill_attack(BF_WEAPON,src,src,bl,EL_ROCK_CRUSHER_ATK,skilllv,tick,flag);
		break;
		
	case EL_STONE_RAIN:
		if( flag&1 )
			skill_attack(skill_get_type(skillid),src,src,bl,skillid,skilllv,tick,flag);
		else {
			int i = skill_get_splash(skillid,skilllv);
			clif_skill_nodamage(src,battle_get_master(src),skillid,skilllv,1);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			if( rnd()%100 < 30 )
				map_foreachinrange(skill_area_sub,bl,i,BL_CHAR,src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
			else
				skill_attack(skill_get_type(skillid),src,src,bl,skillid,skilllv,tick,flag);
		}
		break;
		
	case EL_FIRE_ARROW:
	case EL_ICE_NEEDLE:
	case EL_WIND_SLASH:
	case EL_STONE_HAMMER:
		clif_skill_nodamage(src,battle_get_master(src),skillid,skilllv,1);
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		skill_attack(skill_get_type(skillid),src,src,bl,skillid,skilllv,tick,flag);
		break;
		
	case EL_TIDAL_WEAPON:
		if( src->type == BL_ELEM ) {
			struct elemental_data *ele = BL_CAST(BL_ELEM,src);
			struct status_change *sc = status_get_sc(&ele->bl);
			struct status_change *tsc = status_get_sc(bl);
			sc_type type = status_skill2sc(skillid), type2;
			type2 = type-1;
			
			clif_skill_nodamage(src,battle_get_master(src),skillid,skilllv,1);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			if( (sc && sc->data[type2]) || (tsc && tsc->data[type]) ) {
				elemental_clean_single_effect(ele, skillid);
			}
			if( rnd()%100 < 50 )
				skill_attack(skill_get_type(skillid),src,src,bl,skillid,skilllv,tick,flag);
			else {
				sc_start(src,type2,100,skilllv,skill_get_time(skillid,skilllv));
				sc_start(battle_get_master(src),type,100,ele->bl.id,skill_get_time(skillid,skilllv));
			}
			clif_skill_nodamage(src,src,skillid,skilllv,1);
		}
		break;
			
			
	case 0:/* no skill - basic/normal attack */
		if(sd) {
			if (flag & 3){
				if (bl->id != skill_area_temp[1])
					skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, SD_LEVEL|flag);
			} else {
				skill_area_temp[1] = bl->id;
				map_foreachinrange(skill_area_sub, bl,
					sd->bonus.splash_range, BL_CHAR,
					src, skillid, skilllv, tick, flag | BCT_ENEMY | 1,
					skill_castend_damage_id);
				flag|=1; //Set flag to 1 so ammo is not double-consumed. [Skotlex]
			}
			status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);
		}
		break;

	default:
		if( skillid >= HM_SKILLBASE && skillid <= HM_SKILLBASE + MAX_HOMUNSKILL ) {
			if( src->type == BL_HOM && ((TBL_HOM*)src)->master->fd )
				clif_colormes(((TBL_HOM*)src)->master, COLOR_RED, "This skill is not yet supported");
		} else /* temporary until all the homun-s skills are supported otherwise console would fill up with pointless warnings */
			ShowWarning("skill_castend_damage_id: Unknown skill used:%d\n",skillid);
		clif_skill_damage(src, bl, tick, status_get_amotion(src), tstatus->dmotion,
			0, abs(skill_get_num(skillid, skilllv)),
			skillid, skilllv, skill_get_hit(skillid));
		map_freeblock_unlock();
		return 1;
	}

	if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] ) //Should only remove after the skill has been casted.
		status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);

	map_freeblock_unlock();
	
	if( sd && !(flag&1) )
	{// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = gettick();

		if( sd->state.arrow_atk )
		{// consume arrow on last invocation to this skill.
			battle_consume_ammo(sd, skillid, skilllv);
		}

		// perform skill requirement consumption
		skill_consume_requirement(sd,skillid,skilllv,2);
	}
	
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_nodamage_id (struct block_list *src, struct block_list *bl, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct map_session_data *sd, *dstsd;
	struct mob_data *md, *dstmd;
	struct homun_data *hd;
	struct mercenary_data *mer;
	struct status_data *sstatus, *tstatus;
	struct status_change *tsc;
	struct status_change_entry *tsce;

	int i = 0;
	enum sc_type type;

	if(skillid > 0 && skilllv <= 0) return 0;	// celest

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->m != bl->m)
		return 1;

	sd = BL_CAST(BL_PC, src);
	hd = BL_CAST(BL_HOM, src);
	md = BL_CAST(BL_MOB, src);
	mer = BL_CAST(BL_MER, src);

	dstsd = BL_CAST(BL_PC, bl);
	dstmd = BL_CAST(BL_MOB, bl);

	if(bl->prev == NULL)
		return 1;
	if(status_isdead(src))
		return 1;

	if( src != bl && status_isdead(bl) ) {
		/**
		 * Skills that may be cast on dead targets
		 **/
		switch( skillid ) {
			case NPC_WIDESOULDRAIN:
			case PR_REDEMPTIO:
			case ALL_RESURRECTION:
			case WM_DEADHILLHERE:
				break;
			default:
				return 1;
		}
	}

	tstatus = status_get_status_data(bl);
	sstatus = status_get_status_data(src);

	//Check for undead skills that convert a no-damage skill into a damage one. [Skotlex]
	switch (skillid) {
		case HLIF_HEAL:	//[orn]
			if (bl->type != BL_HOM) {
				if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0) ;
	        break ;
			}
 		case AL_HEAL:
		case ALL_RESURRECTION:
		case PR_ASPERSIO:
		/**
		 * Arch Bishop
		 **/
		case AB_RENOVATIO:
		case AB_HIGHNESSHEAL:
			//Apparently only player casted skills can be offensive like this.
			if (sd && battle_check_undead(tstatus->race,tstatus->def_ele)) {
				if (battle_check_target(src, bl, BCT_ENEMY) < 1) {
				  	//Offensive heal does not works on non-enemies. [Skotlex]
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
				return skill_castend_damage_id (src, bl, skillid, skilllv, tick, flag);
			}
			break;
		case NPC_SMOKING: //Since it is a self skill, this one ends here rather than in damage_id. [Skotlex]
			return skill_castend_damage_id (src, bl, skillid, skilllv, tick, flag);
		default:
			//Skill is actually ground placed.
			if (src == bl && skill_get_unit_id(skillid,0))
				return skill_castend_pos2(src,bl->x,bl->y,skillid,skilllv,tick,0);
	}

	type = status_skill2sc(skillid);
	tsc = status_get_sc(bl);
	tsce = (tsc && type != -1)?tsc->data[type]:NULL;

	if (src!=bl && type > -1 &&
		(i = skill_get_ele(skillid, skilllv)) > ELE_NEUTRAL &&
		skill_get_inf(skillid) != INF_SUPPORT_SKILL &&
		battle_attr_fix(NULL, NULL, 100, i, tstatus->def_ele, tstatus->ele_lv) <= 0)
		return 1; //Skills that cause an status should be blocked if the target element blocks its element.

	map_freeblock_lock();
	switch(skillid)
	{
	case HLIF_HEAL:	//[orn]
	case AL_HEAL:
	/**
	 * Arch Bishop
	 **/
	case AB_HIGHNESSHEAL:
		{
			int heal = skill_calc_heal(src, bl, (skillid == AB_HIGHNESSHEAL)?AL_HEAL:skillid, (skillid == AB_HIGHNESSHEAL)?10:skilllv, true);
			int heal_get_jobexp;
			//Highness Heal: starts at 1.5 boost + 0.5 for each level 
			if( skillid == AB_HIGHNESSHEAL ) {
				heal = heal * ( 15 + 5 * skilllv ) / 10;
			}
			if( status_isimmune(bl) ||
					(dstmd && (dstmd->class_ == MOBID_EMPERIUM || mob_is_battleground(dstmd))) ||
					(dstsd && pc_ismadogear(dstsd)) )//Mado is immune to heal
				heal=0;

			if( sd && dstsd && sd->status.partner_id == dstsd->status.char_id && (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.sex == 0 )
				heal = heal*2;

			if( tsc && tsc->count )
			{
				if( tsc->data[SC_KAITE] && !(sstatus->mode&MD_BOSS) )
				{ //Bounce back heal
					if (--tsc->data[SC_KAITE]->val2 <= 0)
						status_change_end(bl, SC_KAITE, INVALID_TIMER);
					if (src == bl)
						heal=0; //When you try to heal yourself under Kaite, the heal is voided.
					else {
						bl = src;
						dstsd = sd;
					}
				} else
				if (tsc->data[SC_BERSERK])
					heal = 0; //Needed so that it actually displays 0 when healing.
			}
			clif_skill_nodamage (src, bl, skillid, heal, 1);
			if( tsc && tsc->data[SC_AKAITSUKI] && heal && skillid != HLIF_HEAL )
				heal = ~heal + 1;
			heal_get_jobexp = status_heal(bl,heal,0,0);

			if(sd && dstsd && heal > 0 && sd != dstsd && battle_config.heal_exp > 0){
				heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
				if (heal_get_jobexp <= 0)
					heal_get_jobexp = 1;
				pc_gainexp (sd, bl, 0, heal_get_jobexp, false);
			}
		}
		break;

	case PR_REDEMPTIO:
		if (sd && !(flag&1)) {
			if (sd->status.party_id == 0) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			skill_area_temp[0] = 0;
			party_foreachsamemap(skill_area_sub,
				sd,skill_get_splash(skillid, skilllv),
				src,skillid,skilllv,tick, flag|BCT_PARTY|1,
				skill_castend_nodamage_id);
			if (skill_area_temp[0] == 0) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			skill_area_temp[0] = 5 - skill_area_temp[0]; // The actual penalty...
			if (skill_area_temp[0] > 0 && !map[src->m].flag.noexppenalty) { //Apply penalty
				sd->status.base_exp -= min(sd->status.base_exp, pc_nextbaseexp(sd) * skill_area_temp[0] * 2/1000); //0.2% penalty per each.
				sd->status.job_exp -= min(sd->status.job_exp, pc_nextjobexp(sd) * skill_area_temp[0] * 2/1000);
				clif_updatestatus(sd,SP_BASEEXP);
				clif_updatestatus(sd,SP_JOBEXP);
			}
			status_set_hp(src, 1, 0);
			status_set_sp(src, 0, 0);
			break;
		} else if (status_isdead(bl) && flag&1) { //Revive
			skill_area_temp[0]++; //Count it in, then fall-through to the Resurrection code.
			skilllv = 3; //Resurrection level 3 is used
		} else //Invalid target, skip resurrection.
			break;

	case ALL_RESURRECTION:
		if(sd && (map_flag_gvg(bl->m) || map[bl->m].flag.battleground))
		{	//No reviving in WoE grounds!
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		if (!status_isdead(bl))
			break;
		{
			int per = 0, sper = 0;
			if (tsc && tsc->data[SC_HELLPOWER])
				break;

			if (map[bl->m].flag.pvp && dstsd && dstsd->pvp_point < 0)
				break;

			switch(skilllv){
			case 1: per=10; break;
			case 2: per=30; break;
			case 3: per=50; break;
			case 4: per=80; break;
			}
			if(dstsd && dstsd->special_state.restart_full_recover)
				per = sper = 100;
			if (status_revive(bl, per, sper))
			{
				clif_skill_nodamage(src,bl,ALL_RESURRECTION,skilllv,1); //Both Redemptio and Res show this skill-animation.
				if(sd && dstsd && battle_config.resurrection_exp > 0)
				{
					int exp = 0,jexp = 0;
					int lv = dstsd->status.base_level - sd->status.base_level, jlv = dstsd->status.job_level - sd->status.job_level;
					if(lv > 0 && pc_nextbaseexp(dstsd)) {
						exp = (int)((double)dstsd->status.base_exp * (double)lv * (double)battle_config.resurrection_exp / 1000000.);
						if (exp < 1) exp = 1;
					}
					if(jlv > 0 && pc_nextjobexp(dstsd)) {
						jexp = (int)((double)dstsd->status.job_exp * (double)lv * (double)battle_config.resurrection_exp / 1000000.);
						if (jexp < 1) jexp = 1;
					}
					if(exp > 0 || jexp > 0)
						pc_gainexp (sd, bl, exp, jexp, false);
				}
			}
		}
		break;

	case AL_DECAGI:
	case MER_DECAGI:
		clif_skill_nodamage (src, bl, skillid, skilllv,
			sc_start(bl, type, (40 + skilllv * 2 + (status_get_lv(src) + sstatus->int_)/5), skilllv, skill_get_time(skillid,skilllv)));
		break;

	case AL_CRUCIS:
		if (flag&1)
			sc_start(bl,type, 23+skilllv*4 +status_get_lv(src) -status_get_lv(bl), skilllv,skill_get_time(skillid,skilllv));
		else {
			map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid, skilllv), BL_CHAR,
				src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;

	case PR_LEXDIVINA:
	case MER_LEXDIVINA:
		if( tsce )
			status_change_end(bl,type, INVALID_TIMER);
		else
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		clif_skill_nodamage (src, bl, skillid, skilllv, 1);
		break;

	case SA_ABRACADABRA:
		{
			int abra_skillid = 0, abra_skilllv;
			do {
				i = rnd() % MAX_SKILL_ABRA_DB;
				abra_skillid = skill_abra_db[i].skillid;
			} while (abra_skillid == 0 ||
				skill_abra_db[i].req_lv > skilllv || //Required lv for it to appear
				rnd()%10000 >= skill_abra_db[i].per
			);
			abra_skilllv = min(skilllv, skill_get_max(abra_skillid));
			clif_skill_nodamage (src, bl, skillid, skilllv, 1);

			if( sd )
			{// player-casted
				sd->state.abra_flag = 1;
				sd->skillitem = abra_skillid;
				sd->skillitemlv = abra_skilllv;
				clif_item_skill(sd, abra_skillid, abra_skilllv);
			}
			else
			{// mob-casted
				struct unit_data *ud = unit_bl2ud(src);
				int inf = skill_get_inf(abra_skillid);
				int target_id = 0;
				if (!ud) break;
				if (inf&INF_SELF_SKILL || inf&INF_SUPPORT_SKILL) {
					if (src->type == BL_PET)
						bl = (struct block_list*)((TBL_PET*)src)->msd;
					if (!bl) bl = src;
					unit_skilluse_id(src, bl->id, abra_skillid, abra_skilllv);
				} else {	//Assume offensive skills
					if (ud->target)
						target_id = ud->target;
					else switch (src->type) {
						case BL_MOB: target_id = ((TBL_MOB*)src)->target_id; break;
						case BL_PET: target_id = ((TBL_PET*)src)->target_id; break;
					}
					if (!target_id)
						break;
					if (skill_get_casttype(abra_skillid) == CAST_GROUND) {
						bl = map_id2bl(target_id);
						if (!bl) bl = src;
						unit_skilluse_pos(src, bl->x, bl->y, abra_skillid, abra_skilllv);
					} else
						unit_skilluse_id(src, target_id, abra_skillid, abra_skilllv);
				}
			}
		}
		break;

	case SA_COMA:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time2(skillid,skilllv)));
		break;
	case SA_FULLRECOVERY:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (status_isimmune(bl))
			break;
		status_percent_heal(bl, 100, 100);
		break;
	case NPC_ALLHEAL:
		{
			int heal;
			if( status_isimmune(bl) )
				break;
			heal = status_percent_heal(bl, 100, 0);
			clif_skill_nodamage(NULL, bl, AL_HEAL, heal, 1);
			if( dstmd )
			{ // Reset Damage Logs
				memset(dstmd->dmglog, 0, sizeof(dstmd->dmglog));
				dstmd->tdmg = 0;
			}
		}
		break;
	case SA_SUMMONMONSTER:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (sd) mob_once_spawn(sd,src->m,src->x,src->y,"--ja--",-1,1,"");
		break;
	case SA_LEVELUP:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (sd && pc_nextbaseexp(sd)) pc_gainexp(sd, NULL, pc_nextbaseexp(sd) * 10 / 100, 0, false);
		break;
	case SA_INSTANTDEATH:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_set_hp(bl,1,0);
		break;
	case SA_QUESTION:
	case SA_GRAVITY:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case SA_CLASSCHANGE:
	case SA_MONOCELL:
		if (dstmd)
		{
			int class_;
			if ( sd && dstmd->status.mode&MD_BOSS )
			{
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			class_ = skillid==SA_MONOCELL?1002:mob_get_random_id(4, 1, 0);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			mob_class_change(dstmd,class_);
			if( tsc && dstmd->status.mode&MD_BOSS )
			{
				const enum sc_type scs[] = { SC_QUAGMIRE, SC_PROVOKE, SC_ROKISWEIL, SC_GRAVITATION, SC_SUITON, SC_STRIPWEAPON, SC_STRIPSHIELD, SC_STRIPARMOR, SC_STRIPHELM, SC_BLADESTOP };
				for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++)
					if (tsc->data[i]) status_change_end(bl, (sc_type)i, INVALID_TIMER);
				for (i = 0; i < ARRAYLENGTH(scs); i++)
					if (tsc->data[scs[i]]) status_change_end(bl, scs[i], INVALID_TIMER);
			}
		}
		break;
	case SA_DEATH:
		if ( sd && dstmd && dstmd->status.mode&MD_BOSS )
		{
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_kill(bl);
		break;
	case SA_REVERSEORCISH:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid, skilllv)));
		break;
	case SA_FORTUNE:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(sd) pc_getzeny(sd,status_get_lv(bl)*100);
		break;
	case SA_TAMINGMONSTER:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (sd && dstmd) {
			ARR_FIND( 0, MAX_PET_DB, i, dstmd->class_ == pet_db[i].class_ );
			if( i < MAX_PET_DB )
				pet_catch_process1(sd, dstmd->class_);
		}
		break;

	case CR_PROVIDENCE:
		if(sd && dstsd){ //Check they are not another crusader [Skotlex]
			if ((dstsd->class_&MAPID_UPPERMASK) == MAPID_CRUSADER) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock();
				return 1;
			}
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;

	case CG_MARIONETTE:
		{
			struct status_change* sc = status_get_sc(src);

			if( sd && dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER && dstsd->status.sex == sd->status.sex )
			{// Cannot cast on another bard/dancer-type class of the same gender as caster
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock();
				return 1;
			}

			if( sc && tsc )
			{
				if( !sc->data[SC_MARIONETTE] && !tsc->data[SC_MARIONETTE2] )
				{
					sc_start(src,SC_MARIONETTE,100,bl->id,skill_get_time(skillid,skilllv));
					sc_start(bl,SC_MARIONETTE2,100,src->id,skill_get_time(skillid,skilllv));
					clif_skill_nodamage(src,bl,skillid,skilllv,1);
				}
				else
				if(  sc->data[SC_MARIONETTE ] &&  sc->data[SC_MARIONETTE ]->val1 == bl->id &&
					tsc->data[SC_MARIONETTE2] && tsc->data[SC_MARIONETTE2]->val1 == src->id )
				{
					status_change_end(src, SC_MARIONETTE, INVALID_TIMER);
					status_change_end(bl, SC_MARIONETTE2, INVALID_TIMER);
				}
				else
				{
					if( sd )
						clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);

					map_freeblock_unlock();
					return 1;
				}
			}
		}
		break;

	case RG_CLOSECONFINE:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start4(bl,type,100,skilllv,src->id,0,0,skill_get_time(skillid,skilllv)));
		break;
	case SA_FLAMELAUNCHER:	// added failure chance and chance to break weapon if turned on [Valaris]
	case SA_FROSTWEAPON:
	case SA_LIGHTNINGLOADER:
	case SA_SEISMICWEAPON:
		if (dstsd) {
			if(dstsd->status.weapon == W_FIST ||
				(dstsd->sc.count && !dstsd->sc.data[type] &&
				(	//Allow re-enchanting to lenghten time. [Skotlex]
					dstsd->sc.data[SC_FIREWEAPON] ||
					dstsd->sc.data[SC_WATERWEAPON] ||
					dstsd->sc.data[SC_WINDWEAPON] ||
					dstsd->sc.data[SC_EARTHWEAPON] ||
					dstsd->sc.data[SC_SHADOWWEAPON] ||
					dstsd->sc.data[SC_GHOSTWEAPON] ||
					dstsd->sc.data[SC_ENCPOISON]
				))
				) {
				if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				clif_skill_nodamage(src,bl,skillid,skilllv,0);
				break;
			}
		}
		// 100% success rate at lv4 & 5, but lasts longer at lv5
		if(!clif_skill_nodamage(src,bl,skillid,skilllv, sc_start(bl,type,(60+skilllv*10),skilllv, skill_get_time(skillid,skilllv)))) {
			if (sd)
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			if (skill_break_equip(bl, EQP_WEAPON, 10000, BCT_PARTY) && sd && sd != dstsd)
				clif_displaymessage(sd->fd, msg_txt(669));
		}
		break;

	case PR_ASPERSIO:
		if (sd && dstmd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,0);
			break;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;

	case ITEM_ENCHANTARMS:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start2(bl,type,100,skilllv,
				skill_get_ele(skillid,skilllv), skill_get_time(skillid,skilllv)));
		break;

	case TK_SEVENWIND:
		switch(skill_get_ele(skillid,skilllv)) {
			case ELE_EARTH : type = SC_EARTHWEAPON;  break;
			case ELE_WIND  : type = SC_WINDWEAPON;   break;
			case ELE_WATER : type = SC_WATERWEAPON;  break;
			case ELE_FIRE  : type = SC_FIREWEAPON;   break;
			case ELE_GHOST : type = SC_GHOSTWEAPON;  break;
			case ELE_DARK  : type = SC_SHADOWWEAPON; break;
			case ELE_HOLY  : type = SC_ASPERSIO;     break;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));

		sc_start(bl,SC_SEVENWIND,100,skilllv,skill_get_time(skillid,skilllv));

		break;

	case PR_KYRIE:
	case MER_KYRIE:
		clif_skill_nodamage(bl,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;
	//Passive Magnum, should had been casted on yourself.
	case SM_MAGNUM:
	case MS_MAGNUM:
		skill_area_temp[1] = 0;
		map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid, skilllv), BL_SKILL|BL_CHAR,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
		clif_skill_nodamage (src,src,skillid,skilllv,1);
		// Initiate 10% of your damage becomes fire element.
		sc_start4(src,SC_WATK_ELEMENT,100,3,20,0,0,skill_get_time2(skillid, skilllv));
		if( sd )
			skill_blockpc_start(sd, skillid, skill_get_time(skillid, skilllv));
		else if( bl->type == BL_MER )
			skill_blockmerc_start((TBL_MER*)bl, skillid, skill_get_time(skillid, skilllv));
		break;

	case TK_JUMPKICK:
		/* Check if the target is an enemy; if not, skill should fail so the character doesn't unit_movepos (exploitable) */
		if( battle_check_target(src, bl, BCT_ENEMY) > 0 )
		{
			if( unit_movepos(src, bl->x, bl->y, 1, 1) )
			{
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
				clif_slide(src,bl->x,bl->y);
			}
		}
		else
			clif_skill_fail(sd,skillid,USESKILL_FAIL,0);
		break;

	case AL_INCAGI:
	case AL_BLESSING:
	case MER_INCAGI:
	case MER_BLESSING:
		if (dstsd != NULL && tsc->data[SC_CHANGEUNDEAD]) {
			skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag);
			break;
		}
	case PR_SLOWPOISON:
	case PR_IMPOSITIO:
	case PR_LEXAETERNA:
	case PR_SUFFRAGIUM:
	case PR_BENEDICTIO:
	case LK_BERSERK:
	case MS_BERSERK:
	case KN_AUTOCOUNTER:
	case KN_TWOHANDQUICKEN:
	case KN_ONEHAND:
	case MER_QUICKEN:
	case CR_SPEARQUICKEN:
	case CR_REFLECTSHIELD:
	case MS_REFLECTSHIELD:
	case AS_POISONREACT:
	case MC_LOUD:
	case MG_ENERGYCOAT:
	case MO_EXPLOSIONSPIRITS:
	case MO_STEELBODY:
	case MO_BLADESTOP:
	case LK_AURABLADE:
	case LK_PARRYING:
	case MS_PARRYING:
	case LK_CONCENTRATION:
	case WS_CARTBOOST:
	case SN_SIGHT:
	case WS_MELTDOWN:
	case WS_OVERTHRUSTMAX:
	case ST_REJECTSWORD:
	case HW_MAGICPOWER:
	case PF_MEMORIZE:
	case PA_SACRIFICE:
	case ASC_EDP:
	case PF_DOUBLECASTING:
	case SG_SUN_COMFORT:
	case SG_MOON_COMFORT:
	case SG_STAR_COMFORT:
	case NPC_HALLUCINATION:
	case GS_MADNESSCANCEL:
	case GS_ADJUSTMENT:
	case GS_INCREASING:
	case NJ_KASUMIKIRI:
	case NJ_UTSUSEMI:
	case NJ_NEN:
	case NPC_DEFENDER:
	case NPC_MAGICMIRROR:
	case ST_PRESERVE:
	case NPC_INVINCIBLE:
	case NPC_INVINCIBLEOFF:
	case RK_DEATHBOUND:
	case AB_RENOVATIO:
	case AB_EXPIATIO:
	case AB_DUPLELIGHT:
	case AB_SECRAMENT:
	case NC_ACCELERATION:
	case NC_HOVERING:
	case NC_SHAPESHIFT:
	case WL_RECOGNIZEDSPELL:
	case GC_VENOMIMPRESS:
	case SC_DEADLYINFECT:
	case LG_EXEEDBREAK:
	case LG_PRESTIGE:
	case SR_CRESCENTELBOW:
	case SR_LIGHTNINGWALK:
	case SR_GENTLETOUCH_ENERGYGAIN:
	case GN_CARTBOOST:	
	case KO_MEIKYOUSISUI:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;
			
	case SO_STRIKING:
		if (sd) {
			int bonus = 25 + 10 * skilllv;
			bonus += (pc_checkskill(sd, SA_FLAMELAUNCHER)+pc_checkskill(sd, SA_FROSTWEAPON)+pc_checkskill(sd, SA_LIGHTNINGLOADER)+pc_checkskill(sd, SA_SEISMICWEAPON))*5;
			clif_skill_nodamage( src, bl, skillid, skilllv,
								battle_check_target(src,bl,BCT_PARTY) ?
								sc_start2(bl, type, 100, skilllv, bonus, skill_get_time(skillid,skilllv)) :
								0
				);
		}
		break;			
			
	case NPC_STOP:
		if( clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start2(bl,type,100,skilllv,src->id,skill_get_time(skillid,skilllv)) ) )
			sc_start2(src,type,100,skilllv,bl->id,skill_get_time(skillid,skilllv));
		break;
	case HP_ASSUMPTIO:
		if( sd && dstmd )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		else
			clif_skill_nodamage(src,bl,skillid,skilllv,
				sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;
	case MG_SIGHT:
	case MER_SIGHT:
	case AL_RUWACH:
	case WZ_SIGHTBLASTER:
	case NPC_WIDESIGHT:
	case NPC_STONESKIN:
	case NPC_ANTIMAGIC:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start2(bl,type,100,skilllv,skillid,skill_get_time(skillid,skilllv)));
		break;
	case HLIF_AVOID:
	case HAMI_DEFENCE:
		i = skill_get_time(skillid,skilllv);
		clif_skill_nodamage(bl,bl,skillid,skilllv,sc_start(bl,type,100,skilllv,i)); // Master
		clif_skill_nodamage(src,src,skillid,skilllv,sc_start(src,type,100,skilllv,i)); // Homunc
		break;
	case NJ_BUNSINJYUTSU:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		status_change_end(bl, SC_NEN, INVALID_TIMER);
		break;
/* Was modified to only affect targetted char.	[Skotlex]
	case HP_ASSUMPTIO:
		if (flag&1)
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		else
		{
			map_foreachinrange(skill_area_sub, bl,
				skill_get_splash(skillid, skilllv), BL_PC,
				src, skillid, skilllv, tick, flag|BCT_ALL|1,
				skill_castend_nodamage_id);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
*/
	case SM_ENDURE:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		if (sd)
			skill_blockpc_start (sd, skillid, skill_get_time2(skillid,skilllv));
		break;

	case AS_ENCHANTPOISON: // Prevent spamming [Valaris]
		if (sd && dstsd && dstsd->sc.count) {
			if (dstsd->sc.data[SC_FIREWEAPON] ||
				dstsd->sc.data[SC_WATERWEAPON] ||
				dstsd->sc.data[SC_WINDWEAPON] ||
				dstsd->sc.data[SC_EARTHWEAPON] ||
				dstsd->sc.data[SC_SHADOWWEAPON] ||
				dstsd->sc.data[SC_GHOSTWEAPON]
			//	dstsd->sc.data[SC_ENCPOISON] //People say you should be able to recast to lengthen the timer. [Skotlex]
			) {
					clif_skill_nodamage(src,bl,skillid,skilllv,0);
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
					break;
			}
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;

	case LK_TENSIONRELAX:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start4(bl,type,100,skilllv,0,0,skill_get_time2(skillid,skilllv),
				skill_get_time(skillid,skilllv)));
		break;

	case MC_CHANGECART:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case TK_MISSION:
		if (sd) {
			int id;
			if (sd->mission_mobid && (sd->mission_count || rnd()%100)) { //Cannot change target when already have one
				clif_mission_info(sd, sd->mission_mobid, sd->mission_count);
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			id = mob_get_random_id(0,0xF, sd->status.base_level);
			if (!id) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			sd->mission_mobid = id;
			sd->mission_count = 0;
			pc_setglobalreg(sd,"TK_MISSION_ID", id);
			clif_mission_info(sd, id, 0);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case AC_CONCENTRATION:
		{
			clif_skill_nodamage(src,bl,skillid,skilllv,
				sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
			map_foreachinrange( status_change_timer_sub, src,
				skill_get_splash(skillid, skilllv), BL_CHAR,
				src,NULL,type,tick);
		}
		break;

	case SM_PROVOKE:
	case SM_SELFPROVOKE:
	case MER_PROVOKE:
		if( (tstatus->mode&MD_BOSS) || battle_check_undead(tstatus->race,tstatus->def_ele) )
		{
			map_freeblock_unlock();
			return 1;
		}
		//TODO: How much does base level affects? Dummy value of 1% per level difference used. [Skotlex]
		clif_skill_nodamage(src,bl,skillid == SM_SELFPROVOKE ? SM_PROVOKE : skillid,skilllv,
			(i = sc_start(bl,type, skillid == SM_SELFPROVOKE ? 100:( 50 + 3*skilllv + status_get_lv(src) - status_get_lv(bl)), skilllv, skill_get_time(skillid,skilllv))));
		if( !i )
		{
			if( sd )
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			map_freeblock_unlock();
			return 0;
		}
		unit_skillcastcancel(bl, 2);

		if( tsc && tsc->count )
		{
			status_change_end(bl, SC_FREEZE, INVALID_TIMER);
			if( tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE )
				status_change_end(bl, SC_STONE, INVALID_TIMER);
			status_change_end(bl, SC_SLEEP, INVALID_TIMER);
			status_change_end(bl, SC_TRICKDEAD, INVALID_TIMER);
		}

		if( dstmd )
		{
			dstmd->state.provoke_flag = src->id;
			mob_target(dstmd, src, skill_get_range2(src,skillid,skilllv));
		}
		break;

	case ML_DEVOTION:
	case CR_DEVOTION:
		{
			int count, lv;
			if( !dstsd || (!sd && !mer) )
			{ // Only players can be devoted
				if( sd )
					clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);
				break;
			}

			if( (lv = status_get_lv(src) - dstsd->status.base_level) < 0 )
				lv = -lv;
			if( lv > battle_config.devotion_level_difference || // Level difference requeriments
				(dstsd->sc.data[type] && dstsd->sc.data[type]->val1 != src->id) || // Cannot Devote a player devoted from another source
				(skillid == ML_DEVOTION && (!mer || mer != dstsd->md)) || // Mercenary only can devote owner
				(dstsd->class_&MAPID_UPPERMASK) == MAPID_CRUSADER || // Crusader Cannot be devoted
				(dstsd->sc.data[SC_HELLPOWER])) // Players affected by SC_HELLPOWERR cannot be devoted.
			{
				if( sd )
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock();
				return 1;
			}

			i = 0;
			count = (sd)? min(skilllv,5) : 1; // Mercenary only can Devote owner
			if( sd )
			{ // Player Devoting Player
				ARR_FIND(0, count, i, sd->devotion[i] == bl->id );
				if( i == count )
				{
					ARR_FIND(0, count, i, sd->devotion[i] == 0 );
					if( i == count )
					{ // No free slots, skill Fail
						clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);
						map_freeblock_unlock();
						return 1;
					}
				}

				sd->devotion[i] = bl->id;
			}
			else
				mer->devotion_flag = 1; // Mercenary Devoting Owner

			clif_skill_nodamage(src, bl, skillid, skilllv,
				sc_start4(bl, type, 100, src->id, i, skill_get_range2(src,skillid,skilllv),0, skill_get_time2(skillid, skilllv)));
			clif_devotion(src, NULL);
		}
		break;

	case MO_CALLSPIRITS:
		if(sd) {
			int limit = skilllv;
			if( sd->sc.data[SC_RAISINGDRAGON] )
				limit += sd->sc.data[SC_RAISINGDRAGON]->val1;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			pc_addspiritball(sd,skill_get_time(skillid,skilllv),limit);
		}
		break;

	case CH_SOULCOLLECT:
		if(sd) {
			int limit = 5;
			if( sd->sc.data[SC_RAISINGDRAGON] )
				limit += sd->sc.data[SC_RAISINGDRAGON]->val1;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			for (i = 0; i < limit; i++)
				pc_addspiritball(sd,skill_get_time(skillid,skilllv),limit);
		}
		break;

	case MO_KITRANSLATION:
		if(dstsd && (dstsd->class_&MAPID_BASEMASK)!=MAPID_GUNSLINGER) {
			pc_addspiritball(dstsd,skill_get_time(skillid,skilllv),5);
		}
		break;

	case TK_TURNKICK:
	case MO_BALKYOUNG: //Passive part of the attack. Splash knock-back+stun. [Skotlex]
		if (skill_area_temp[1] != bl->id) {
			skill_blown(src,bl,skill_get_blewcount(skillid,skilllv),-1,0);
			skill_additional_effect(src,bl,skillid,skilllv,BF_MISC,ATK_DEF,tick); //Use Misc rather than weapon to signal passive pushback
		}
		break;

	case MO_ABSORBSPIRITS:
		i = 0;
		if (dstsd && dstsd->spiritball && (sd == dstsd || map_flag_vs(src->m)) && (dstsd->class_&MAPID_BASEMASK)!=MAPID_GUNSLINGER)
		{	// split the if for readability, and included gunslingers in the check so that their coins cannot be removed [Reddozen]
			i = dstsd->spiritball * 7;
			pc_delspiritball(dstsd,dstsd->spiritball,0);
		} else if (dstmd && !(tstatus->mode&MD_BOSS) && rnd() % 100 < 20)
		{	// check if target is a monster and not a Boss, for the 20% chance to absorb 2 SP per monster's level [Reddozen]
			i = 2 * dstmd->level;
			mob_target(dstmd,src,0);
		}
		if (i) status_heal(src, 0, i, 3);
		clif_skill_nodamage(src,bl,skillid,skilllv,i?1:0);
		break;

	case AC_MAKINGARROW:
		if(sd) {
			clif_arrow_create_list(sd);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case AM_PHARMACY:
		if(sd) {
			clif_skill_produce_mix_list(sd,skillid,22);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case SA_CREATECON:
		if(sd) {
			clif_elementalconverter_list(sd);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case BS_HAMMERFALL:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,SC_STUN,(20 + 10 * skilllv),skilllv,skill_get_time2(skillid,skilllv)));
		break;
	case RG_RAID:
		skill_area_temp[1] = 0;
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		map_foreachinrange(skill_area_sub, bl,
			skill_get_splash(skillid, skilllv), splash_target(src),
			src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		status_change_end(src, SC_HIDING, INVALID_TIMER);
		break;

	case ASC_METEORASSAULT:
	case GS_SPREADATTACK:
	case RK_STORMBLAST:
	case NC_AXETORNADO:
	case GC_COUNTERSLASH:
	case SR_SKYNETBLOW:
	case SR_RAMPAGEBLASTER:
	case SR_HOWLINGOFLION:
	case KO_HAPPOKUNAI:
		skill_area_temp[1] = 0;
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		i = map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), splash_target(src), 
			src, skillid, skilllv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		if( !i && ( skillid == NC_AXETORNADO || skillid == SR_SKYNETBLOW || skillid == KO_HAPPOKUNAI ) )
			clif_skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		break;

	case NC_EMERGENCYCOOL:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_change_end(src,SC_OVERHEAT_LIMITPOINT,INVALID_TIMER);
		status_change_end(src,SC_OVERHEAT,INVALID_TIMER);
		break;
	case SR_WINDMILL:
	case GN_CART_TORNADO:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
	case SR_EARTHSHAKER:
	case NC_INFRAREDSCAN:
	case NPC_EARTHQUAKE:
	case NPC_VAMPIRE_GIFT:
	case NPC_HELLJUDGEMENT:
	case NPC_PULSESTRIKE:
	case LG_MOONSLASHER:
		skill_castend_damage_id(src, src, skillid, skilllv, tick, flag);
		break;

	case KN_BRANDISHSPEAR:
	case ML_BRANDISH:
		skill_brandishspear(src, bl, skillid, skilllv, tick, flag);
		break;

	case WZ_SIGHTRASHER:
		//Passive side of the attack.
		status_change_end(src, SC_SIGHT, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		map_foreachinrange(skill_area_sub,src,
			skill_get_splash(skillid, skilllv),BL_CHAR|BL_SKILL,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		break;

	case NJ_HYOUSYOURAKU:
	case NJ_RAIGEKISAI:
	case WZ_FROSTNOVA:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		skill_area_temp[1] = 0;
		map_foreachinrange(skill_attack_area, src,
			skill_get_splash(skillid, skilllv), splash_target(src),
			BF_MAGIC, src, src, skillid, skilllv, tick, flag, BCT_ENEMY);
		break;

	case HVAN_EXPLOSION:	//[orn]
	case NPC_SELFDESTRUCTION:
		//Self Destruction hits everyone in range (allies+enemies)
		//Except for Summoned Marine spheres on non-versus maps, where it's just enemy.
		i = ((!md || md->special_state.ai == 2) && !map_flag_vs(src->m))?
			BCT_ENEMY:BCT_ALL;
		clif_skill_nodamage(src, src, skillid, -1, 1);
		map_delblock(src); //Required to prevent chain-self-destructions hitting back.
		map_foreachinrange(skill_area_sub, bl,
			skill_get_splash(skillid, skilllv), splash_target(src),
			src, skillid, skilllv, tick, flag|i,
			skill_castend_damage_id);
		map_addblock(src);
		status_damage(src, src, sstatus->max_hp,0,0,1);
		break;

	case AL_ANGELUS:
	case PR_MAGNIFICAT:
	case PR_GLORIA:
	case SN_WINDWALK:
	case CASH_BLESSING:
	case CASH_INCAGI:
	case CASH_ASSUMPTIO:
		if( sd == NULL || sd->status.party_id == 0 || (flag & 1) )
			clif_skill_nodamage(bl, bl, skillid, skilllv, sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;
	case MER_MAGNIFICAT:
		if( mer != NULL )
		{
			clif_skill_nodamage(bl, bl, skillid, skilllv, sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
			if( mer->master && mer->master->status.party_id != 0 && !(flag&1) )
				party_foreachsamemap(skill_area_sub, mer->master, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
			else if( mer->master && !(flag&1) )
				clif_skill_nodamage(src, &mer->master->bl, skillid, skilllv, sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		}
		break;

	case BS_ADRENALINE:
	case BS_ADRENALINE2:
	case BS_WEAPONPERFECT:
	case BS_OVERTHRUST:
		if (sd == NULL || sd->status.party_id == 0 || (flag & 1)) {
			clif_skill_nodamage(bl,bl,skillid,skilllv,
				sc_start2(bl,type,100,skilllv,(src == bl)? 1:0,skill_get_time(skillid,skilllv)));
		} else if (sd) {
			party_foreachsamemap(skill_area_sub,
				sd,skill_get_splash(skillid, skilllv),
				src,skillid,skilllv,tick, flag|BCT_PARTY|1,
				skill_castend_nodamage_id);
		}
		break;

	case BS_MAXIMIZE:
	case NV_TRICKDEAD:
	case CR_DEFENDER:
	case ML_DEFENDER:
	case CR_AUTOGUARD:
	case ML_AUTOGUARD:
	case TK_READYSTORM:
	case TK_READYDOWN:
	case TK_READYTURN:
	case TK_READYCOUNTER:
	case TK_DODGE:
	case CR_SHRINK:
	case SG_FUSION:
	case GS_GATLINGFEVER:
		if( tsce )
		{
			clif_skill_nodamage(src,bl,skillid,skilllv,status_change_end(bl, type, INVALID_TIMER));
			map_freeblock_unlock();
			return 0;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;
	case SL_KAITE:
	case SL_KAAHI:
	case SL_KAIZEL:
	case SL_KAUPE:
		if (sd) {
			if (!dstsd || !(
				(sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_SOULLINKER) ||
				(dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER ||
				dstsd->status.char_id == sd->status.char_id ||
				dstsd->status.char_id == sd->status.partner_id ||
				dstsd->status.char_id == sd->status.child
			)) {
				status_change_start(src,SC_STUN,10000,skilllv,0,0,0,500,8);
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid, skilllv)));
		break;
	case SM_AUTOBERSERK:
	case MER_AUTOBERSERK:
		if( tsce )
			i = status_change_end(bl, type, INVALID_TIMER);
		else
			i = sc_start(bl,type,100,skilllv,60000);
		clif_skill_nodamage(src,bl,skillid,skilllv,i);
		break;
	case TF_HIDING:
	case ST_CHASEWALK:
	case KO_YAMIKUMO:
		if (tsce)
		{
			clif_skill_nodamage(src,bl,skillid,-1,status_change_end(bl, type, INVALID_TIMER)); //Hide skill-scream animation.
			map_freeblock_unlock();
			return 0;
		} else if( tsc && tsc->option&OPTION_MADOGEAR ) {
			//Mado Gear cannot hide
			if( sd ) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			map_freeblock_unlock();
			return 0;
		}
		clif_skill_nodamage(src,bl,skillid,-1,sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;
	case TK_RUN:
		if (tsce)
		{
			clif_skill_nodamage(src,bl,skillid,skilllv,status_change_end(bl, type, INVALID_TIMER));
			map_freeblock_unlock();
			return 0;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,sc_start4(bl,type,100,skilllv,unit_getdir(bl),0,0,0));
		if (sd) // If the client receives a skill-use packet inmediately before a walkok packet, it will discard the walk packet! [Skotlex]
			clif_walkok(sd); // So aegis has to resend the walk ok.
		break;
	case AS_CLOAKING:
	case GC_CLOAKINGEXCEED:
	case LG_FORCEOFVANGUARD:
	case SC_REPRODUCE:
	case SC_INVISIBILITY:
		if (tsce) {
			i = status_change_end(bl, type, INVALID_TIMER);
			if( i )
				clif_skill_nodamage(src,bl,skillid,( skillid == LG_FORCEOFVANGUARD ) ? skilllv : -1,i);
			else if( sd )
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			map_freeblock_unlock();
			return 0;
		}
	case RA_CAMOUFLAGE:
		i = sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		if( i )
			clif_skill_nodamage(src,bl,skillid,( skillid == LG_FORCEOFVANGUARD ) ? skilllv : -1,i);
		else if( sd )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		break;

	case BD_ADAPTATION:
		if(tsc && tsc->data[SC_DANCING]){
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			status_change_end(bl, SC_DANCING, INVALID_TIMER);
		}
		break;

	case BA_FROSTJOKER:
	case DC_SCREAM:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		skill_addtimerskill(src,tick+2000,bl->id,src->x,src->y,skillid,skilllv,0,flag);

		if (md) {
			// custom hack to make the mob display the skill, because these skills don't show the skill use text themselves
			//NOTE: mobs don't have the sprite animation that is used when performing this skill (will cause glitches)
			char temp[70];
			snprintf(temp, sizeof(temp), "%s : %s !!",md->name,skill_db[skillid].desc);
			clif_message(&md->bl,temp);
		}
		break;

	case BA_PANGVOICE:
		clif_skill_nodamage(src,bl,skillid,skilllv, sc_start(bl,SC_CONFUSION,50,7,skill_get_time(skillid,skilllv)));
		break;

	case DC_WINKCHARM:
		if( dstsd )
			clif_skill_nodamage(src,bl,skillid,skilllv, sc_start(bl,SC_CONFUSION,30,7,skill_get_time2(skillid,skilllv)));
		else
		if( dstmd )
		{
			if( status_get_lv(src) > status_get_lv(bl)
			&&  (tstatus->race == RC_DEMON || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_ANGEL)
			&&  !(tstatus->mode&MD_BOSS) )
				clif_skill_nodamage(src,bl,skillid,skilllv, sc_start2(bl,type,70,skilllv,src->id,skill_get_time(skillid,skilllv)));
			else
			{
				clif_skill_nodamage(src,bl,skillid,skilllv,0);
				if(sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			}
		}
		break;

	case TF_STEAL:
		if(sd) {
			if(pc_steal_item(sd,bl,skilllv))
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
			else
				clif_skill_fail(sd,skillid,USESKILL_FAIL,0);
		}
		break;

	case RG_STEALCOIN:
		if(sd) {
			if(pc_steal_coin(sd,bl))
			{
				dstmd->state.provoke_flag = src->id;
				mob_target(dstmd, src, skill_get_range2(src,skillid,skilllv));
				clif_skill_nodamage(src,bl,skillid,skilllv,1);

			} 
			else
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;

	case MG_STONECURSE:
		{
			if (tstatus->mode&MD_BOSS) {
				if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			if(status_isimmune(bl) || !tsc)
				break;

			if (tsc->data[SC_STONE]) {
				status_change_end(bl, SC_STONE, INVALID_TIMER);
				if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			if (sc_start4(bl,SC_STONE,(skilllv*4+20),
				skilllv, 0, 0, skill_get_time(skillid, skilllv),
				skill_get_time2(skillid,skilllv)))
					clif_skill_nodamage(src,bl,skillid,skilllv,1);
			else if(sd) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				// Level 6-10 doesn't consume a red gem if it fails [celest]
				if (skilllv > 5)
				{ // not to consume items
					map_freeblock_unlock();
					return 0;
				}
			}
		}
		break;

	case NV_FIRSTAID:
		clif_skill_nodamage(src,bl,skillid,5,1);
		status_heal(bl,5,0,0);
		break;

	case AL_CURE:
		if(status_isimmune(bl)) {
			clif_skill_nodamage(src,bl,skillid,skilllv,0);
			break;
		}
		status_change_end(bl, SC_SILENCE, INVALID_TIMER);
		status_change_end(bl, SC_BLIND, INVALID_TIMER);
		status_change_end(bl, SC_CONFUSION, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case TF_DETOXIFY:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_change_end(bl, SC_POISON, INVALID_TIMER);
		status_change_end(bl, SC_DPOISON, INVALID_TIMER);
		break;

	case PR_STRECOVERY:
		if(status_isimmune(bl)) {
			clif_skill_nodamage(src,bl,skillid,skilllv,0);
			break;
		}
		if (tsc && tsc->opt1) {
			status_change_end(bl, SC_FREEZE, INVALID_TIMER);
			status_change_end(bl, SC_STONE, INVALID_TIMER);
			status_change_end(bl, SC_SLEEP, INVALID_TIMER);
			status_change_end(bl, SC_STUN, INVALID_TIMER);
			status_change_end(bl, SC_WHITEIMPRISON, INVALID_TIMER);
		}
		//Is this equation really right? It looks so... special.
		if(battle_check_undead(tstatus->race,tstatus->def_ele))
		{
			status_change_start(bl, SC_BLIND,
				100*(100-(tstatus->int_/2+tstatus->vit/3+tstatus->luk/10)),
				1,0,0,0,
				skill_get_time2(skillid, skilllv) * (100-(tstatus->int_+tstatus->vit)/2)/100,0);
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(dstmd)
			mob_unlocktarget(dstmd,tick);
		break;

	// Mercenary Supportive Skills
	case MER_BENEDICTION:
		status_change_end(bl, SC_CURSE, INVALID_TIMER);
		status_change_end(bl, SC_BLIND, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case MER_COMPRESS:
		status_change_end(bl, SC_BLEEDING, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case MER_MENTALCURE:
		status_change_end(bl, SC_CONFUSION, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case MER_RECUPERATE:
		status_change_end(bl, SC_POISON, INVALID_TIMER);
		status_change_end(bl, SC_SILENCE, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case MER_REGAIN:
		status_change_end(bl, SC_SLEEP, INVALID_TIMER);
		status_change_end(bl, SC_STUN, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case MER_TENDER:
		status_change_end(bl, SC_FREEZE, INVALID_TIMER);
		status_change_end(bl, SC_STONE, INVALID_TIMER);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case MER_SCAPEGOAT:
		if( mer && mer->master )
		{
			status_heal(&mer->master->bl, mer->battle_status.hp, 0, 2);
			status_damage(src, src, mer->battle_status.max_hp, 0, 0, 1);
		}
		break;

	case MER_ESTIMATION:
		if( !mer )
			break;
		sd = mer->master;
	case WZ_ESTIMATION:
		if( sd == NULL )
			break;
		if( dstsd )
		{ // Fail on Players
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		if( dstmd && dstmd->class_ == MOBID_EMPERIUM )
			break; // Cannot be Used on Emperium

		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		clif_skill_estimation(sd, bl);
		if( skillid == MER_ESTIMATION )
			sd = NULL;
		break;

	case BS_REPAIRWEAPON:
		if(sd && dstsd)
			clif_item_repair_list(sd,dstsd,skilllv);
		break;

	case MC_IDENTIFY:
		if(sd)
			clif_item_identify_list(sd);
		break;

	// Weapon Refining [Celest]
	case WS_WEAPONREFINE:
		if(sd)
			clif_item_refine_list(sd);
		break;

	case MC_VENDING:
		if(sd)
		{	//Prevent vending of GMs with unnecessary Level to trade/drop. [Skotlex]
			if ( !pc_can_give_items(sd) )
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			else {
				sd->state.prevend = 1;
				clif_openvendingreq(sd,2+skilllv);
			}
		}
		break;

	case AL_TELEPORT:
		if(sd)
		{
			if (map[bl->m].flag.noteleport && skilllv <= 2) {
				clif_skill_teleportmessage(sd,0);
				break;
			}
			if(!battle_config.duel_allow_teleport && sd->duel_group && skilllv <= 2) { // duel restriction [LuzZza]
				clif_displaymessage(sd->fd, "Duel: Can't use teleport in duel.");
				break;
			}

			if( sd->state.autocast || ( (sd->skillitem == AL_TELEPORT || battle_config.skip_teleport_lv1_menu) && skilllv == 1 ) || skilllv == 3 )
			{
				if( skilllv == 1 )
					pc_randomwarp(sd,CLR_TELEPORT);
				else
					pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
				break;
			}

			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if( skilllv == 1 )
				clif_skill_warppoint(sd,skillid,skilllv, (unsigned short)-1,0,0,0);
			else
				clif_skill_warppoint(sd,skillid,skilllv, (unsigned short)-1,sd->status.save_point.map,0,0);
		} else
			unit_warp(bl,-1,-1,-1,CLR_TELEPORT);
		break;

	case NPC_EXPULSION:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		unit_warp(bl,-1,-1,-1,CLR_TELEPORT);
		break;

	case AL_HOLYWATER:
		if(sd) {
			if (skill_produce_mix(sd, skillid, 523, 0, 0, 0, 1))
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
			else
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;

	case TF_PICKSTONE:
		if(sd) {
			int eflag;
			struct item item_tmp;
			struct block_list tbl;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			memset(&item_tmp,0,sizeof(item_tmp));
			memset(&tbl,0,sizeof(tbl)); // [MouseJstr]
			item_tmp.nameid = ITEMID_STONE;
			item_tmp.identify = 1;
			tbl.id = 0;
			clif_takeitem(&sd->bl,&tbl);
			eflag = pc_additem(sd,&item_tmp,1,LOG_TYPE_PRODUCE);
			if(eflag) {
				clif_additem(sd,0,0,eflag);
				map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
		}
		break;
	case ASC_CDP:
     	if(sd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			skill_produce_mix(sd, skillid, 678, 0, 0, 0, 1); //Produce a Deadly Poison Bottle.
		}
		break;

	case RG_STRIPWEAPON:
	case RG_STRIPSHIELD:
	case RG_STRIPARMOR:
	case RG_STRIPHELM:
	case ST_FULLSTRIP:
	case GC_WEAPONCRUSH:
	case SC_STRIPACCESSARY: {
		unsigned short location = 0;
		int d = 0;
		
		//Rate in percent
		if ( skillid == ST_FULLSTRIP ) {
			i = 5 + 2*skilllv + (sstatus->dex - tstatus->dex)/5;
		} else if( skillid == SC_STRIPACCESSARY ) {
			i = 12 + 2 * skilllv + (sstatus->dex - tstatus->dex)/5;
		} else {
			i = 5 + 5*skilllv + (sstatus->dex - tstatus->dex)/5;
		}

		if (i < 5) i = 5; //Minimum rate 5%

		//Duration in ms
		if( skillid == GC_WEAPONCRUSH){
			d = skill_get_time(skillid,skilllv);
			if(bl->type == BL_PC)
				d += skilllv * 15 + (sstatus->dex - tstatus->dex);
			else
				d += skilllv * 30 + (sstatus->dex - tstatus->dex) / 2;
		}else
			d = skill_get_time(skillid,skilllv) + (sstatus->dex - tstatus->dex)*500;

		if (d < 0) d = 0; //Minimum duration 0ms

		switch (skillid) {
		case RG_STRIPWEAPON:
		case GC_WEAPONCRUSH:
			location = EQP_WEAPON;
			break;
		case RG_STRIPSHIELD:
			location = EQP_SHIELD;
			break;
		case RG_STRIPARMOR:
			location = EQP_ARMOR;
			break;
		case RG_STRIPHELM:
			location = EQP_HELM;
			break;
		case ST_FULLSTRIP:
			location = EQP_WEAPON|EQP_SHIELD|EQP_ARMOR|EQP_HELM;
			break;
		case SC_STRIPACCESSARY:
			location = EQP_ACC;
			break;
		}

		//Special message when trying to use strip on FCP [Jobbie]
		if( sd && skillid == ST_FULLSTRIP && tsc && tsc->data[SC_CP_WEAPON] && tsc->data[SC_CP_HELM] && tsc->data[SC_CP_ARMOR] && tsc->data[SC_CP_SHIELD])
		{
			clif_gospel_info(sd, 0x28);
			break;
		}

		//Attempts to strip at rate i and duration d
		if( (i = skill_strip_equip(bl, location, i, skilllv, d)) || (skillid != ST_FULLSTRIP && skillid != GC_WEAPONCRUSH ) )
			clif_skill_nodamage(src,bl,skillid,skilllv,i); 

		//Nothing stripped.
		if( sd && !i )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		break;
	}

	case AM_BERSERKPITCHER:
	case AM_POTIONPITCHER: {
			int i,x,hp = 0,sp = 0,bonus=100;
			if( dstmd && dstmd->class_ == MOBID_EMPERIUM ) {
				map_freeblock_unlock();
				return 1;
			}
			if( sd ) {
				x = skilllv%11 - 1;
				i = pc_search_inventory(sd,skill_db[skillid].itemid[x]);
				if( i < 0 || skill_db[skillid].itemid[x] <= 0 ) {
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
					map_freeblock_unlock();
					return 1;
				}
				if(sd->inventory_data[i] == NULL || sd->status.inventory[i].amount < skill_db[skillid].amount[x]) {
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
					map_freeblock_unlock();
					return 1;
				}
				if( skillid == AM_BERSERKPITCHER ) {
					if( dstsd && dstsd->status.base_level < (unsigned int)sd->inventory_data[i]->elv ) {
						clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
						map_freeblock_unlock();
						return 1;
					}
				}
				potion_flag = 1;
				potion_hp = potion_sp = potion_per_hp = potion_per_sp = 0;
				potion_target = bl->id;
				run_script(sd->inventory_data[i]->script,0,sd->bl.id,0);
				potion_flag = potion_target = 0;
				if( sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_ALCHEMIST )
					bonus += sd->status.base_level;
				if( potion_per_hp > 0 || potion_per_sp > 0 ) {
					hp = tstatus->max_hp * potion_per_hp / 100;
					hp = hp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
					if( dstsd ) {
						sp = dstsd->status.max_sp * potion_per_sp / 100;
						sp = sp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
					}
				} else {
					if( potion_hp > 0 ) {
						hp = potion_hp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
						hp = hp * (100 + (tstatus->vit<<1)) / 100;
						if( dstsd )
							hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10) / 100;
					}
					if( potion_sp > 0 ) {
						sp = potion_sp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
						sp = sp * (100 + (tstatus->int_<<1)) / 100;
						if( dstsd )
							sp = sp * (100 + pc_checkskill(dstsd,MG_SRECOVERY)*10) / 100;
					}
				}

				if (sd->itemgrouphealrate[IG_POTION]>0) {
					hp += hp * sd->itemgrouphealrate[IG_POTION] / 100;
					sp += sp * sd->itemgrouphealrate[IG_POTION] / 100;
				}

				if( (i = pc_skillheal_bonus(sd, skillid)) ) {
					hp += hp * i / 100;
					sp += sp * i / 100;
				}
			} else {
				hp = (1 + rnd()%400) * (100 + skilllv*10) / 100;
				hp = hp * (100 + (tstatus->vit<<1)) / 100;
				if( dstsd )
					hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10) / 100;
			}
			if( dstsd && (i = pc_skillheal2_bonus(dstsd, skillid)) ) {
				hp += hp * i / 100;
				sp += sp * i / 100;
			}
			if( tsc && tsc->count ) {
				if( tsc->data[SC_CRITICALWOUND] ) {
					hp -= hp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
					sp -= sp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
				}
				if( tsc->data[SC_DEATHHURT] ) {
					hp -= hp * 20 / 100;
					sp -= sp * 20 / 100;
				}
				if( tsc->data[SC_WATER_INSIGNIA] && tsc->data[SC_WATER_INSIGNIA]->val1 == 2 ) {
					hp += hp / 10;
					sp += sp / 10;
				}
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if( hp > 0 || (skillid == AM_POTIONPITCHER && sp <= 0) )
				clif_skill_nodamage(NULL,bl,AL_HEAL,hp,1);
			if( sp > 0 )
				clif_skill_nodamage(NULL,bl,MG_SRECOVERY,sp,1);
			status_heal(bl,hp,sp,0);
		}
		break;
	case AM_CP_WEAPON:
	case AM_CP_SHIELD:
	case AM_CP_ARMOR:
	case AM_CP_HELM:
		{
			unsigned int equip[] = {EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP};

			if( sd && ( bl->type != BL_PC || ( dstsd && pc_checkequip(dstsd,equip[skillid - AM_CP_WEAPON]) < 0 ) ) ){
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock(); // Don't consume item requirements
				return 0;
			}

			clif_skill_nodamage(src,bl,skillid,skilllv,
				sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		}
		break;
	case AM_TWILIGHT1:
		if (sd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			//Prepare 200 White Potions.
			if (!skill_produce_mix(sd, skillid, 504, 0, 0, 0, 200))
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;
	case AM_TWILIGHT2:
		if (sd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			//Prepare 200 Slim White Potions.
			if (!skill_produce_mix(sd, skillid, 547, 0, 0, 0, 200))
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;
	case AM_TWILIGHT3:
		if (sd) {
			int ebottle = pc_search_inventory(sd,713);
			if( ebottle >= 0 )
				ebottle = sd->status.inventory[ebottle].amount;
			//check if you can produce all three, if not, then fail:
			if (!skill_can_produce_mix(sd,970,-1, 100) //100 Alcohol
				|| !skill_can_produce_mix(sd,7136,-1, 50) //50 Acid Bottle
				|| !skill_can_produce_mix(sd,7135,-1, 50) //50 Flame Bottle
				|| ebottle < 200 //200 empty bottle are required at total.
			) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			skill_produce_mix(sd, skillid, 970, 0, 0, 0, 100);
			skill_produce_mix(sd, skillid, 7136, 0, 0, 0, 50);
			skill_produce_mix(sd, skillid, 7135, 0, 0, 0, 50);
		}
		break;
	case SA_DISPELL:
		if (flag&1 || (i = skill_get_splash(skillid, skilllv)) < 1)
		{
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if((dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
				|| (tsc && tsc->data[SC_SPIRIT] && tsc->data[SC_SPIRIT]->val2 == SL_ROGUE) //Rogue's spirit defends againt dispel.
				|| rnd()%100 >= 50+10*skilllv
				|| ( tsc && tsc->option&OPTION_MADOGEAR ) )//Mado Gear is immune to dispell according to bug report 49 [Ind]
			{
				if (sd)
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			if(status_isimmune(bl) || !tsc || !tsc->count)
				break;
			for(i=0;i<SC_MAX;i++)
			{
				if (!tsc->data[i])
					continue;
				switch (i) {
				case SC_WEIGHT50:		case SC_WEIGHT90:		case SC_HALLUCINATION:
				case SC_STRIPWEAPON:	case SC_STRIPSHIELD:	case SC_STRIPARMOR:
				case SC_STRIPHELM:		case SC_CP_WEAPON:		case SC_CP_SHIELD:
				case SC_CP_ARMOR:		case SC_CP_HELM:		case SC_COMBO:
				case SC_STRFOOD:		case SC_AGIFOOD:		case SC_VITFOOD:
				case SC_INTFOOD:		case SC_DEXFOOD:		case SC_LUKFOOD:
				case SC_HITFOOD:		case SC_FLEEFOOD:		case SC_BATKFOOD:
				case SC_WATKFOOD:		case SC_MATKFOOD:		case SC_DANCING:
				case SC_EDP:			case SC_AUTOBERSERK:
				case SC_CARTBOOST:		case SC_MELTDOWN:		case SC_SAFETYWALL:
				case SC_SMA:			case SC_SPEEDUP0:		case SC_NOCHAT:
				case SC_ANKLE:			case SC_SPIDERWEB:		case SC_JAILED:
				case SC_ITEMBOOST:		case SC_EXPBOOST:		case SC_LIFEINSURANCE:
				case SC_BOSSMAPINFO:	case SC_PNEUMA:			case SC_AUTOSPELL:
				case SC_INCHITRATE:		case SC_INCATKRATE:		case SC_NEN:
				case SC_READYSTORM:		case SC_READYDOWN:		case SC_READYTURN:
				case SC_READYCOUNTER:	case SC_DODGE:			case SC_WARM:
				case SC_SPEEDUP1:		case SC_AUTOTRADE:		case SC_CRITICALWOUND:
				case SC_JEXPBOOST:		case SC_INVINCIBLE:		case SC_INVINCIBLEOFF:
				case SC_HELLPOWER:		case SC_MANU_ATK:		case SC_MANU_DEF:
				case SC_SPL_ATK:		case SC_SPL_DEF:		case SC_MANU_MATK:
				case SC_SPL_MATK:		case SC_RICHMANKIM:		case SC_ETERNALCHAOS:
				case SC_DRUMBATTLE:		case SC_NIBELUNGEN:		case SC_ROKISWEIL:
				case SC_INTOABYSS:		case SC_SIEGFRIED:		case SC_FOOD_STR_CASH:
				case SC_FOOD_AGI_CASH:	case SC_FOOD_VIT_CASH:	case SC_FOOD_DEX_CASH:
				case SC_FOOD_INT_CASH:	case SC_FOOD_LUK_CASH:	case SC_SEVENWIND:
				case SC_MIRACLE:		case SC_S_LIFEPOTION:	case SC_L_LIFEPOTION:
				case SC_INCHEALRATE:	case SC_ELECTRICSHOCKER:		case SC__STRIPACCESSORY:
				//case SC_SAVAGE_STEAK:			case SC_COCKTAIL_WARG_BLOOD:		case SC_MINOR_BBQ:
				//case SC_SIROMA_ICE_TEA:			case SC_DROCERA_HERB_STEAMED:		case SC_PUTTI_TAILS_NOODLES:
				case SC_NEUTRALBARRIER_MASTER:		case SC_NEUTRALBARRIER:			case SC_STEALTHFIELD_MASTER:
				case SC_STEALTHFIELD:	case SC_GIANTGROWTH:	case SC_MILLENNIUMSHIELD:
				case SC_REFRESH:		case SC_STONEHARDSKIN:	case SC_VITALITYACTIVATION:
				case SC_FIGHTINGSPIRIT:	case SC_ABUNDANCE:		case SC__SHADOWFORM:
				case SC_LEADERSHIP:		case SC_GLORYWOUNDS:	case SC_SOULCOLD:
				case SC_HAWKEYES:		case SC_GUILDAURA:	case SC_PUSH_CART:
					continue;
				/**
				 * bugreport:4888 these songs may only be dispelled if you're not in their song area anymore
				 **/
				case SC_WHISTLE:
				case SC_ASSNCROS:
				case SC_POEMBRAGI:
				case SC_APPLEIDUN:
				case SC_HUMMING:
				case SC_DONTFORGETME:
				case SC_FORTUNE:
				case SC_SERVICE4U:
					if( tsc->data[i]->val4 ) //val4 = out-of-song-area
						continue;
					break;
				case SC_ASSUMPTIO:
					if( bl->type == BL_MOB )
						continue;
					break;
				}
				if(i==SC_BERSERK) tsc->data[i]->val2=0; //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
				status_change_end(bl, (sc_type)i, INVALID_TIMER);
			}
			break;
		}
		//Affect all targets on splash area.
		map_foreachinrange(skill_area_sub, bl, i, BL_CHAR,
			src, skillid, skilllv, tick, flag|1,
			skill_castend_damage_id);
		break;

	case TF_BACKSLIDING: //This is the correct implementation as per packet logging information. [Skotlex]
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		skill_blown(src,bl,skill_get_blewcount(skillid,skilllv),unit_getdir(bl),0);
		break;

	case TK_HIGHJUMP:
		{
			int x,y, dir = unit_getdir(src);

			//Fails on noteleport maps, except for GvG and BG maps [Skotlex]
			if( map[src->m].flag.noteleport &&
				!(map[src->m].flag.battleground || map_flag_gvg2(src->m) )
			) {
				x = src->x;
				y = src->y;
			} else {
				x = src->x + dirx[dir]*skilllv*2;
				y = src->y + diry[dir]*skilllv*2;
			}

			clif_skill_nodamage(src,bl,TK_HIGHJUMP,skilllv,1);
			if(!map_count_oncell(src->m,x,y,BL_PC|BL_NPC|BL_MOB) && map_getcell(src->m,x,y,CELL_CHKREACH)) {
				clif_slide(src,x,y);
				unit_movepos(src, x, y, 1, 0);
			}
		}
		break;

	case SA_CASTCANCEL:
	case SO_SPELLFIST:		
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		unit_skillcastcancel(src,1);
		if(sd) {
			int sp = skill_get_sp(sd->skillid_old,sd->skilllv_old);
			if( skillid == SO_SPELLFIST ){
				sc_start4(src,type,100,skilllv+1,skilllv,sd->skillid_old,sd->skilllv_old,skill_get_time(skillid,skilllv));
				sd->skillid_old = sd->skilllv_old = 0;
				break; 
			}
			sp = sp * (90 - (skilllv-1)*20) / 100;
			if(sp < 0) sp = 0;
			status_zap(src, 0, sp);
		}
		break;
	case SA_SPELLBREAKER:
		{
			int sp;
			if(tsc && tsc->data[SC_MAGICROD]) {
				sp = skill_get_sp(skillid,skilllv);
				sp = sp * tsc->data[SC_MAGICROD]->val2 / 100;
				if(sp < 1) sp = 1;
				status_heal(bl,0,sp,2);
				status_percent_damage(bl, src, 0, -20, false); //20% max SP damage.
			} else {
				struct unit_data *ud = unit_bl2ud(bl);
				int bl_skillid=0,bl_skilllv=0,hp = 0;
				if (!ud || ud->skilltimer == INVALID_TIMER)
					break; //Nothing to cancel.
				bl_skillid = ud->skillid;
				bl_skilllv = ud->skilllv;
				if (tstatus->mode & MD_BOSS)
				{	//Only 10% success chance against bosses. [Skotlex]
					if (rnd()%100 < 90)
					{
						if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
						break;
					}
				} else if (!dstsd || map_flag_vs(bl->m)) //HP damage only on pvp-maps when against players.
					hp = tstatus->max_hp/50; //Recover 2% HP [Skotlex]

				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				unit_skillcastcancel(bl,0);
				sp = skill_get_sp(bl_skillid,bl_skilllv);
				status_zap(bl, hp, sp);

				if (hp && skilllv >= 5)
					hp>>=1;	//Recover half damaged HP at level 5 [Skotlex]
				else
					hp = 0;

				if (sp) //Recover some of the SP used
					sp = sp*(25*(skilllv-1))/100;

				if(hp || sp)
					status_heal(src, hp, sp, 2);
			}
		}
		break;
	case SA_MAGICROD:
		clif_skill_nodamage(src,src,SA_MAGICROD,skilllv,1);
		sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		break;
	case SA_AUTOSPELL:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(sd)
			clif_autospell(sd,skilllv);
		else {
			int maxlv=1,spellid=0;
			static const int spellarray[3] = { MG_COLDBOLT,MG_FIREBOLT,MG_LIGHTNINGBOLT };
			if(skilllv >= 10) {
				spellid = MG_FROSTDIVER;
//				if (tsc && tsc->data[SC_SPIRIT] && tsc->data[SC_SPIRIT]->val2 == SA_SAGE)
//					maxlv = 10;
//				else
					maxlv = skilllv - 9;
			}
			else if(skilllv >=8) {
				spellid = MG_FIREBALL;
				maxlv = skilllv - 7;
			}
			else if(skilllv >=5) {
				spellid = MG_SOULSTRIKE;
				maxlv = skilllv - 4;
			}
			else if(skilllv >=2) {
				int i = rnd()%3;
				spellid = spellarray[i];
				maxlv = skilllv - 1;
			}
			else if(skilllv > 0) {
				spellid = MG_NAPALMBEAT;
				maxlv = 3;
			}
			if(spellid > 0)
				sc_start4(src,SC_AUTOSPELL,100,skilllv,spellid,maxlv,0,
					skill_get_time(SA_AUTOSPELL,skilllv));
		}
		break;

	case BS_GREED:
		if(sd){
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinrange(skill_greed,bl,
				skill_get_splash(skillid, skilllv),BL_ITEM,bl);
		}
		break;

	case SA_ELEMENTWATER:
	case SA_ELEMENTFIRE:
	case SA_ELEMENTGROUND:
	case SA_ELEMENTWIND:
		if(sd && !dstmd) //Only works on monsters.
			break;
		if(tstatus->mode&MD_BOSS)
			break;
	case NPC_ATTRICHANGE:
	case NPC_CHANGEWATER:
	case NPC_CHANGEGROUND:
	case NPC_CHANGEFIRE:
	case NPC_CHANGEWIND:
	case NPC_CHANGEPOISON:
	case NPC_CHANGEHOLY:
	case NPC_CHANGEDARKNESS:
	case NPC_CHANGETELEKINESIS:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start2(bl, type, 100, skilllv, skill_get_ele(skillid,skilllv),
				skill_get_time(skillid, skilllv)));
		break;
	case NPC_CHANGEUNDEAD:
		//This skill should fail if target is wearing bathory/evil druid card [Brainstorm]
		//TO-DO This is ugly, fix it
		if(tstatus->def_ele==ELE_UNDEAD || tstatus->def_ele==ELE_DARK) break;
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start2(bl, type, 100, skilllv, skill_get_ele(skillid,skilllv),
				skill_get_time(skillid, skilllv)));
		break;

	case NPC_PROVOCATION:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (md) mob_unlocktarget(md, tick);
		break;

	case NPC_KEEPING:
	case NPC_BARRIER:
		{
			int skill_time = skill_get_time(skillid,skilllv);
			struct unit_data *ud = unit_bl2ud(bl);
			if (clif_skill_nodamage(src,bl,skillid,skilllv,
					sc_start(bl,type,100,skilllv,skill_time))
			&& ud) {	//Disable attacking/acting/moving for skill's duration.
				ud->attackabletime =
				ud->canact_tick =
				ud->canmove_tick = tick + skill_time;
			}
		}
		break;

	case NPC_REBIRTH:
		if( md && md->state.rebirth )
			break; // only works once
		sc_start(bl,type,100,skilllv,-1);
		break;

	case NPC_DARKBLESSING:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start2(bl,type,(50+skilllv*5),skilllv,skilllv,skill_get_time2(skillid,skilllv)));
		break;

	case NPC_LICK:
		status_zap(bl, 0, 100);
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,(skilllv*5),skilllv,skill_get_time2(skillid,skilllv)));
		break;

	case NPC_SUICIDE:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_kill(src); //When suiciding, neither exp nor drops is given.
		break;

	case NPC_SUMMONSLAVE:
	case NPC_SUMMONMONSTER:
		if(md && md->skillidx >= 0)
			mob_summonslave(md,md->db->skill[md->skillidx].val,skilllv,skillid);
		break;

	case NPC_CALLSLAVE:
		mob_warpslave(src,MOB_SLAVEDISTANCE);
		break;

	case NPC_RANDOMMOVE:
		if (md) {
			md->next_walktime = tick - 1;
			mob_randomwalk(md,tick);
		}
		break;

	case NPC_SPEEDUP:
		{
			// or does it increase casting rate? just a guess xD
			int i = SC_ASPDPOTION0 + skilllv - 1;
			if (i > SC_ASPDPOTION3)
				i = SC_ASPDPOTION3;
			clif_skill_nodamage(src,bl,skillid,skilllv,
				sc_start(bl,(sc_type)i,100,skilllv,skilllv * 60000));
		}
		break;

	case NPC_REVENGE:
		// not really needed... but adding here anyway ^^
		if (md && md->master_id > 0) {
			struct block_list *mbl, *tbl;
			if ((mbl = map_id2bl(md->master_id)) == NULL ||
				(tbl = battle_gettargeted(mbl)) == NULL)
				break;
			md->state.provoke_flag = tbl->id;
			mob_target(md, tbl, sstatus->rhw.range);
		}
		break;

	case NPC_RUN:
		{
			const int mask[8][2] = {{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1}};
			int dir = (bl == src)?unit_getdir(src):map_calc_dir(src,bl->x,bl->y); //If cast on self, run forward, else run away.
			unit_stop_attack(src);
			//Run skillv tiles overriding the can-move check.
			if (unit_walktoxy(src, src->x + skilllv * mask[dir][0], src->y + skilllv * mask[dir][1], 2) && md)
				md->state.skillstate = MSS_WALK; //Otherwise it isn't updated in the ai.
		}
		break;

	case NPC_TRANSFORMATION:
	case NPC_METAMORPHOSIS:
		if(md && md->skillidx >= 0) {
			int class_ = mob_random_class (md->db->skill[md->skillidx].val,0);
			if (skilllv > 1) //Multiply the rest of mobs. [Skotlex]
				mob_summonslave(md,md->db->skill[md->skillidx].val,skilllv-1,skillid);
			if (class_) mob_class_change(md, class_);
		}
		break;

	case NPC_EMOTION_ON:
	case NPC_EMOTION:
		//va[0] is the emotion to use.
		//NPC_EMOTION & NPC_EMOTION_ON can change a mob's mode 'permanently' [Skotlex]
		//val[1] 'sets' the mode
		//val[2] adds to the current mode
		//val[3] removes from the current mode
		//val[4] if set, asks to delete the previous mode change.
		if(md && md->skillidx >= 0 && tsc)
		{
			clif_emotion(bl, md->db->skill[md->skillidx].val[0]);
			if(md->db->skill[md->skillidx].val[4] && tsce)
				status_change_end(bl, type, INVALID_TIMER);

			if(md->db->skill[md->skillidx].val[1] || md->db->skill[md->skillidx].val[2])
				sc_start4(src, type, 100, skilllv,
					md->db->skill[md->skillidx].val[1],
					md->db->skill[md->skillidx].val[2],
					md->db->skill[md->skillidx].val[3],
					skill_get_time(skillid, skilllv));
		}
		break;

	case NPC_POWERUP:
		sc_start(bl,SC_INCATKRATE,100,200,skill_get_time(skillid, skilllv));
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,100,skill_get_time(skillid, skilllv)));
		break;

	case NPC_AGIUP:
		sc_start(bl,SC_SPEEDUP1,100,skilllv,skill_get_time(skillid, skilllv));
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,100,skill_get_time(skillid, skilllv)));
		break;

	case NPC_INVISIBLE:
		//Have val4 passed as 6 is for "infinite cloak" (do not end on attack/skill use).
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start4(bl,type,100,skilllv,0,0,6,skill_get_time(skillid,skilllv)));
		break;

	case NPC_SIEGEMODE:
		// not sure what it does
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case WE_MALE:
		{
			int hp_rate=(skilllv <= 0)? 0:skill_db[skillid].hp_rate[skilllv-1];
			int gain_hp= tstatus->max_hp*abs(hp_rate)/100; // The earned is the same % of the target HP than it costed the caster. [Skotlex]
			clif_skill_nodamage(src,bl,skillid,status_heal(bl, gain_hp, 0, 0),1);
		}
		break;
	case WE_FEMALE:
		{
			int sp_rate=(skilllv <= 0)? 0:skill_db[skillid].sp_rate[skilllv-1];
			int gain_sp=tstatus->max_sp*abs(sp_rate)/100;// The earned is the same % of the target SP than it costed the caster. [Skotlex]
			clif_skill_nodamage(src,bl,skillid,status_heal(bl, 0, gain_sp, 0),1);
		}
		break;

	// parent-baby skills
	case WE_BABY:
		if(sd){
			struct map_session_data *f_sd = pc_get_father(sd);
			struct map_session_data *m_sd = pc_get_mother(sd);
			// if neither was found
			if(!f_sd && !m_sd){
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock();
				return 0;
			}
			status_change_start(bl,SC_STUN,10000,skilllv,0,0,0,skill_get_time2(skillid,skilllv),8);
			if (f_sd) sc_start(&f_sd->bl,type,100,skilllv,skill_get_time(skillid,skilllv));
			if (m_sd) sc_start(&m_sd->bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		}
		break;

	case PF_HPCONVERSION:
		{
			int hp, sp;
			hp = sstatus->max_hp/10;
			sp = hp * 10 * skilllv / 100;
			if (!status_charge(src,hp,0)) {
				if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			status_heal(bl,0,sp,2);
		}
		break;

	case MA_REMOVETRAP:
	case HT_REMOVETRAP:		
		{
			struct skill_unit* su;
			struct skill_unit_group* sg;
			su = BL_CAST(BL_SKILL, bl);

			// Mercenaries can remove any trap
			// Players can only remove their own traps or traps on Vs maps.
			if( su && (sg = su->group) && (src->type == BL_MER || sg->src_id == src->id || map_flag_vs(bl->m)) && (skill_get_inf2(sg->skill_id)&INF2_TRAP) )
			{
				clif_skill_nodamage(src, bl, skillid, skilllv, 1);
				if( sd && !(sg->unit_id == UNT_USED_TRAPS || (sg->unit_id == UNT_ANKLESNARE && sg->val2 != 0 )) )
				{ // prevent picking up expired traps
					if( battle_config.skill_removetrap_type )
					{ // get back all items used to deploy the trap
						for( i = 0; i < 10; i++ )
						{
							if( skill_db[su->group->skill_id].itemid[i] > 0 )
							{
								int flag;
								struct item item_tmp;
								memset(&item_tmp,0,sizeof(item_tmp));
								item_tmp.nameid = skill_db[su->group->skill_id].itemid[i];
								item_tmp.identify = 1;
								if( item_tmp.nameid && (flag=pc_additem(sd,&item_tmp,skill_db[su->group->skill_id].amount[i],LOG_TYPE_OTHER)) )
								{
									clif_additem(sd,0,0,flag);
									map_addflooritem(&item_tmp,skill_db[su->group->skill_id].amount[i],sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
								}
							}
						}
					}
					else
					{ // get back 1 trap
						struct item item_tmp;
						memset(&item_tmp,0,sizeof(item_tmp));
						item_tmp.nameid = su->group->item_id?su->group->item_id:ITEMID_TRAP;
						item_tmp.identify = 1;
						if( item_tmp.nameid && (flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_OTHER)) )
						{
							clif_additem(sd,0,0,flag);
							map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
						}
					}
				}
				skill_delunit(su);
			}else if(sd)
				clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);

		}
		break;
	case HT_SPRINGTRAP:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		{
			struct skill_unit *su=NULL;
			if((bl->type==BL_SKILL) && (su=(struct skill_unit *)bl) && (su->group) ){
				switch(su->group->unit_id){
					case UNT_ANKLESNARE:	// ankle snare
						if (su->group->val2 != 0)
							// if it is already trapping something don't spring it,
							// remove trap should be used instead
							break;
						// otherwise fallthrough to below
					case UNT_BLASTMINE:
					case UNT_SKIDTRAP:
					case UNT_LANDMINE:
					case UNT_SHOCKWAVE:
					case UNT_SANDMAN:
					case UNT_FLASHER:
					case UNT_FREEZINGTRAP:
					case UNT_CLAYMORETRAP:
					case UNT_TALKIEBOX:
						su->group->unit_id = UNT_USED_TRAPS;
						clif_changetraplook(bl, UNT_USED_TRAPS);
						su->group->limit=DIFF_TICK(tick+1500,su->group->tick);
						su->limit=DIFF_TICK(tick+1500,su->group->tick);
				}
			}
		}
		break;
	case BD_ENCORE:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(sd)
			unit_skilluse_id(src,src->id,sd->skillid_dance,sd->skilllv_dance);
		break;

	case AS_SPLASHER:
		if(tstatus->mode&MD_BOSS
	/**
	 * Renewal dropped the 3/4 hp requirement
	 **/
	#ifndef RENEWAL
			|| tstatus-> hp > tstatus->max_hp*3/4
	#endif
				) {
			if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			map_freeblock_unlock();
			return 1;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start4(bl,type,100,skilllv,skillid,src->id,skill_get_time(skillid,skilllv),1000));
		if (sd) skill_blockpc_start (sd, skillid, skill_get_time(skillid, skilllv)+3000);
		break;

	case PF_MINDBREAKER:
		{
			if(tstatus->mode&MD_BOSS || battle_check_undead(tstatus->race,tstatus->def_ele))
			{
				map_freeblock_unlock();
				return 1;
			}

			if (tsce)
			{	//HelloKitty2 (?) explained that this silently fails when target is
				//already inflicted. [Skotlex]
				map_freeblock_unlock();
				return 1;
			}

			//Has a 55% + skilllv*5% success chance.
			if (!clif_skill_nodamage(src,bl,skillid,skilllv,
				sc_start(bl,type,55+5*skilllv,skilllv,skill_get_time(skillid,skilllv))))
			{
				if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock();
				return 0;
			}

			unit_skillcastcancel(bl,0);

			if(tsc && tsc->count){
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				if(tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE)
					status_change_end(bl, SC_STONE, INVALID_TIMER);
				status_change_end(bl, SC_SLEEP, INVALID_TIMER);
			}

			if(dstmd)
				mob_target(dstmd,src,skill_get_range2(src,skillid,skilllv));
		}
		break;

	case PF_SOULCHANGE:
		{
			unsigned int sp1 = 0, sp2 = 0;
			if (dstmd) {
				if (dstmd->state.soul_change_flag) {
					if(sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
					break;
				}
				dstmd->state.soul_change_flag = 1;
				sp2 = sstatus->max_sp * 3 /100;
				status_heal(src, 0, sp2, 2);
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				break;
			}
			sp1 = sstatus->sp;
			sp2 = tstatus->sp;
			#ifdef	RENEWAL
				sp1 = sp1 / 2;
				sp2 = sp2 / 2;
			#endif
			status_set_sp(src, sp2, 3);
			status_set_sp(bl, sp1, 3);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	// Slim Pitcher
	case CR_SLIMPITCHER:
		// Updated to block Slim Pitcher from working on barricades and guardian stones.
		if( dstmd && (dstmd->class_ == MOBID_EMPERIUM || (dstmd->class_ >= MOBID_BARRICADE1 && dstmd->class_ <= MOBID_GUARIDAN_STONE2)) )
			break;
		if (potion_hp || potion_sp) {
			int hp = potion_hp, sp = potion_sp;
			hp = hp * (100 + (tstatus->vit<<1))/100;
			sp = sp * (100 + (tstatus->int_<<1))/100;
			if (dstsd) {
				if (hp)
					hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10 + pc_skillheal2_bonus(dstsd, skillid))/100;
				if (sp)
					sp = sp * (100 + pc_checkskill(dstsd,MG_SRECOVERY)*10 + pc_skillheal2_bonus(dstsd, skillid))/100;
			}
			if( tsc && tsc->count ) {
				if (tsc->data[SC_CRITICALWOUND]) {
					hp -= hp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
					sp -= sp * tsc->data[SC_CRITICALWOUND]->val2 / 100;
				}
				if (tsc->data[SC_DEATHHURT]) {
					hp -= hp * 20 / 100;
					sp -= sp * 20 / 100;
				}
				if( tsc->data[SC_WATER_INSIGNIA] && tsc->data[SC_WATER_INSIGNIA]->val1 == 2) {
					hp += hp / 10;
					sp += sp / 10;
				}
			}
			if(hp > 0)
				clif_skill_nodamage(NULL,bl,AL_HEAL,hp,1);
			if(sp > 0)
				clif_skill_nodamage(NULL,bl,MG_SRECOVERY,sp,1);
			status_heal(bl,hp,sp,0);
		}
		break;
	// Full Chemical Protection
	case CR_FULLPROTECTION:
		{
			unsigned int equip[] = {EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP};
			int i, s = 0, skilltime = skill_get_time(skillid,skilllv);

			for (i=0 ; i<4; i++) {
				if( bl->type != BL_PC || ( dstsd && pc_checkequip(dstsd,equip[i]) < 0 ) )
					continue;
				sc_start(bl,(sc_type)(SC_CP_WEAPON + i),100,skilllv,skilltime);
				s++;
			}
			if( sd && !s ){
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock(); // Don't consume item requirements
				return 0;
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case RG_CLEANER:	//AppleGirl
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case CG_LONGINGFREEDOM:
		{
			if (tsc && !tsce && (tsce=tsc->data[SC_DANCING]) && tsce->val4
				&& (tsce->val1&0xFFFF) != CG_MOONLIT) //Can't use Longing for Freedom while under Moonlight Petals. [Skotlex]
			{
				clif_skill_nodamage(src,bl,skillid,skilllv,
					sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
			}
		}
		break;

	case CG_TAROTCARD:
		{
			int eff, count = -1;
			if( rnd() % 100 > skilllv * 8 || (dstmd && ((dstmd->guardian_data && dstmd->class_ == MOBID_EMPERIUM) || mob_is_battleground(dstmd))) )
			{
				if( sd )
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);

				map_freeblock_unlock();
				return 0;
			}
			status_zap(src,0,skill_db[skill_get_index(skillid)].sp[skilllv]); // consume sp only if succeeded [Inkfish]
			do {
				eff = rnd() % 14;
				clif_specialeffect(bl, 523 + eff, AREA);
				switch (eff)
				{
				case 0:	// heals SP to 0
					status_percent_damage(src, bl, 0, 100, false);
					break;
				case 1:	// matk halved
					sc_start(bl,SC_INCMATKRATE,100,-50,skill_get_time2(skillid,skilllv));
					break;
				case 2:	// all buffs removed
					status_change_clear_buffs(bl,1);
					break;
				case 3:	// 1000 damage, random armor destroyed
					{
						int where[] = { EQP_ARMOR, EQP_SHIELD, EQP_HELM, EQP_SHOES, EQP_GARMENT };
						status_fix_damage(src, bl, 1000, 0);
						clif_damage(src,bl,tick,0,0,1000,0,0,0);
						if( !status_isdead(bl) )
							skill_break_equip(bl, where[rnd()%5], 10000, BCT_ENEMY);
					}
					break;
				case 4:	// atk halved
					sc_start(bl,SC_INCATKRATE,100,-50,skill_get_time2(skillid,skilllv));
					break;
				case 5:	// 2000HP heal, random teleported
					status_heal(src, 2000, 0, 0);
					if( !map_flag_vs(bl->m) )
						unit_warp(bl, -1,-1,-1, CLR_TELEPORT);
					break;
				case 6:	// random 2 other effects
					if (count == -1)
						count = 3;
					else
						count++; //Should not retrigger this one.
					break;
				case 7:	// stop freeze or stoned
					{
						enum sc_type sc[] = { SC_STOP, SC_FREEZE, SC_STONE };
						sc_start(bl,sc[rnd()%3],100,skilllv,skill_get_time2(skillid,skilllv));
					}
					break;
				case 8:	// curse coma and poison
					sc_start(bl,SC_COMA,100,skilllv,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_CURSE,100,skilllv,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_POISON,100,skilllv,skill_get_time2(skillid,skilllv));
					break;
				case 9:	// confusion
					sc_start(bl,SC_CONFUSION,100,skilllv,skill_get_time2(skillid,skilllv));
					break;
				case 10:	// 6666 damage, atk matk halved, cursed
					status_fix_damage(src, bl, 6666, 0);
					clif_damage(src,bl,tick,0,0,6666,0,0,0);
					sc_start(bl,SC_INCATKRATE,100,-50,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_INCMATKRATE,100,-50,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_CURSE,skilllv,100,skill_get_time2(skillid,skilllv));
					break;
				case 11:	// 4444 damage
					status_fix_damage(src, bl, 4444, 0);
					clif_damage(src,bl,tick,0,0,4444,0,0,0);
					break;
				case 12:	// stun
					sc_start(bl,SC_STUN,100,skilllv,5000);
					break;
				case 13:	// atk,matk,hit,flee,def reduced
					sc_start(bl,SC_INCATKRATE,100,-20,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_INCMATKRATE,100,-20,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_INCHITRATE,100,-20,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_INCFLEERATE,100,-20,skill_get_time2(skillid,skilllv));
					sc_start(bl,SC_INCDEFRATE,100,-20,skill_get_time2(skillid,skilllv));
					break;
				default:
					break;
				}
			} while ((--count) > 0);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case SL_ALCHEMIST:
	case SL_ASSASIN:
	case SL_BARDDANCER:
	case SL_BLACKSMITH:
	case SL_CRUSADER:
	case SL_HUNTER:
	case SL_KNIGHT:
	case SL_MONK:
	case SL_PRIEST:
	case SL_ROGUE:
	case SL_SAGE:
	case SL_SOULLINKER:
	case SL_STAR:
	case SL_SUPERNOVICE:
	case SL_WIZARD:
		//NOTE: here, 'type' has the value of the associated MAPID, not of the SC_SPIRIT constant.
		if (sd && !(dstsd && (dstsd->class_&MAPID_UPPERMASK) == type)) {
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		if (skillid == SL_SUPERNOVICE && dstsd && dstsd->die_counter && !(rnd()%100))
		{	//Erase death count 1% of the casts
			dstsd->die_counter = 0;
			pc_setglobalreg(dstsd,"PC_DIE_COUNTER", 0);
			clif_specialeffect(bl, 0x152, AREA);
			//SC_SPIRIT invokes status_calc_pc for us.
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start4(bl,SC_SPIRIT,100,skilllv,skillid,0,0,skill_get_time(skillid,skilllv)));
		sc_start(src,SC_SMA,100,skilllv,skill_get_time(SL_SMA,skilllv));
		break;
	case SL_HIGH:
		if (sd && !(dstsd && (dstsd->class_&JOBL_UPPER) && !(dstsd->class_&JOBL_2) && dstsd->status.base_level < 70)) {
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start4(bl,type,100,skilllv,skillid,0,0,skill_get_time(skillid,skilllv)));
		sc_start(src,SC_SMA,100,skilllv,skill_get_time(SL_SMA,skilllv));
		break;

	case SL_SWOO:
		if (tsce) {
			sc_start(src,SC_STUN,100,skilllv,10000);
			break;
		}
	case SL_SKA: // [marquis007]
	case SL_SKE:
		if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			status_change_start(src,SC_STUN,10000,skilllv,0,0,0,500,10);
			break;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		if (skillid == SL_SKE)
			sc_start(src,SC_SMA,100,skilllv,skill_get_time(SL_SMA,skilllv));
		break;

	// New guild skills [Celest]
	case GD_BATTLEORDER:
		if(flag&1) {
			if (status_get_guild_id(src) == status_get_guild_id(bl))
				sc_start(bl,type,100,skilllv,skill_get_time(skillid, skilllv));
		} else if (status_get_guild_id(src)) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinrange(skill_area_sub, src,
				skill_get_splash(skillid, skilllv), BL_PC,
				src,skillid,skilllv,tick, flag|BCT_GUILD|1,
				skill_castend_nodamage_id);
			if (sd)
				guild_block_skill(sd,skill_get_time2(skillid,skilllv));
		}
		break;
	case GD_REGENERATION:
		if(flag&1) {
			if (status_get_guild_id(src) == status_get_guild_id(bl))
				sc_start(bl,type,100,skilllv,skill_get_time(skillid, skilllv));
		} else if (status_get_guild_id(src)) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinrange(skill_area_sub, src,
				skill_get_splash(skillid, skilllv), BL_PC,
				src,skillid,skilllv,tick, flag|BCT_GUILD|1,
				skill_castend_nodamage_id);
			if (sd)
				guild_block_skill(sd,skill_get_time2(skillid,skilllv));
		}
		break;
	case GD_RESTORE:
		if(flag&1) {
			if (status_get_guild_id(src) == status_get_guild_id(bl))
				clif_skill_nodamage(src,bl,AL_HEAL,status_percent_heal(bl,90,90),1);
		} else if (status_get_guild_id(src)) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinrange(skill_area_sub, src,
				skill_get_splash(skillid, skilllv), BL_PC,
				src,skillid,skilllv,tick, flag|BCT_GUILD|1,
				skill_castend_nodamage_id);
			if (sd)
				guild_block_skill(sd,skill_get_time2(skillid,skilllv));
		}
		break;
	case GD_EMERGENCYCALL:
		{
			int dx[9]={-1, 1, 0, 0,-1, 1,-1, 1, 0};
			int dy[9]={ 0, 0, 1,-1, 1,-1,-1, 1, 0};
			int j = 0;
			struct guild *g = NULL;
			// i don't know if it actually summons in a circle, but oh well. ;P
			g = sd?sd->state.gmaster_flag:guild_search(status_get_guild_id(src));
			if (!g)
				break;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			for(i = 0; i < g->max_member; i++, j++) {
				if (j>8) j=0;
				if ((dstsd = g->member[i].sd) != NULL && sd != dstsd && !dstsd->state.autotrade && !pc_isdead(dstsd)) {
					if (map[dstsd->bl.m].flag.nowarp && !map_flag_gvg2(dstsd->bl.m))
						continue;
					if(map_getcell(src->m,src->x+dx[j],src->y+dy[j],CELL_CHKNOREACH))
						dx[j] = dy[j] = 0;
					pc_setpos(dstsd, map_id2index(src->m), src->x+dx[j], src->y+dy[j], CLR_RESPAWN);
				}
			}
			if (sd)
				guild_block_skill(sd,skill_get_time2(skillid,skilllv));
		}
		break;

	case SG_FEEL:
		//AuronX reported you CAN memorize the same map as all three. [Skotlex]
		if (sd) {
			if(!sd->feel_map[skilllv-1].index)
				clif_feel_req(sd->fd,sd, skilllv);
			else
				clif_feel_info(sd, skilllv-1, 1);
		}
		break;

	case SG_HATE:
		if (sd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if (!pc_set_hate_mob(sd, skilllv-1, bl))
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;

	case GS_GLITTERING:
		if(sd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if(rnd()%100 < (20+10*skilllv))
				pc_addspiritball(sd,skill_get_time(skillid,skilllv),10);
			else if(sd->spiritball > 0)
				pc_delspiritball(sd,1,0);
		}
		break;

	case GS_CRACKER:
		/* per official standards, this skill works on players and mobs. */
		if (sd && (dstsd || dstmd))
		{
			i =65 -5*distance_bl(src,bl); //Base rate
			if (i < 30) i = 30;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			sc_start(bl,SC_STUN, i,skilllv,skill_get_time2(skillid,skilllv));
		}
		break;

	case AM_CALLHOMUN:	//[orn]
		if (sd && !merc_call_homunculus(sd))
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		break;

	case AM_REST:
		if (sd) {
			if (merc_hom_vaporize(sd,1))
				clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			else
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;
	case MH_STAHL_HORN:
		if (sd) {
			if( skillid == MH_GOLDENE_FERSE )
				clif_skill_fail(sd,skillid,USESKILL_FAIL_CONDITION,0);
		}
		break;
	case HAMI_CASTLE:	//[orn]
		if(rnd()%100 < 20*skilllv && src != bl)
		{
			int x,y;
			x = src->x;
			y = src->y;
			if (hd)
				skill_blockhomun_start(hd, skillid, skill_get_time2(skillid,skilllv));

			if (unit_movepos(src,bl->x,bl->y,0,0)) {
				clif_skill_nodamage(src,src,skillid,skilllv,1); // Homunc
				clif_slide(src,bl->x,bl->y) ;
				if (unit_movepos(bl,x,y,0,0))
				{
					clif_skill_nodamage(bl,bl,skillid,skilllv,1); // Master
					clif_slide(bl,x,y) ;
				}

				//TODO: Shouldn't also players and the like switch targets?
				map_foreachinrange(skill_chastle_mob_changetarget,src,
					AREA_SIZE, BL_MOB, bl, src);
			}
		}
		// Failed
		else if (hd && hd->master)
			clif_skill_fail(hd->master, skillid, USESKILL_FAIL_LEVEL, 0);
		else if (sd)
			clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);
		break;
	case HVAN_CHAOTIC:	//[orn]
		{
			static const int per[5][2]={{20,50},{50,60},{25,75},{60,64},{34,67}};
			int r = rnd()%100;
			i = (skilllv-1)%5;
			if(r<per[i][0]) //Self
				bl = src;
			else if(r<per[i][1]) //Master
				bl = battle_get_master(src);
			else //Enemy
				bl = map_id2bl(battle_gettarget(src));

			if (!bl) bl = src;
			i = skill_calc_heal(src, bl, skillid, 1+rnd()%skilllv, true);
			//Eh? why double skill packet?
			clif_skill_nodamage(src,bl,AL_HEAL,i,1);
			clif_skill_nodamage(src,bl,skillid,i,1);
			status_heal(bl, i, 0, 0);
		}
		break;
	//Homun single-target support skills [orn]
	case HAMI_BLOODLUST:
	case HFLI_FLEET:
	case HFLI_SPEED:
	case HLIF_CHANGE:
	case MH_ANGRIFFS_MODUS:
	case MH_GOLDENE_FERSE:			
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		if (hd)
			skill_blockhomun_start(hd, skillid, skill_get_time2(skillid,skilllv));
		break;

	case NPC_DRAGONFEAR:
		if (flag&1) {
			const enum sc_type sc[] = { SC_STUN, SC_SILENCE, SC_CONFUSION, SC_BLEEDING };
			int j;
			j = i = rnd()%ARRAYLENGTH(sc);
			while ( !sc_start(bl,sc[i],100,skilllv,skill_get_time2(skillid,i+1)) ) {
				i++;
				if ( i == ARRAYLENGTH(sc) )
					i = 0;
				if (i == j)
					break;
			}
			break;
		}
	case NPC_WIDEBLEEDING:
	case NPC_WIDECONFUSE:
	case NPC_WIDECURSE:
	case NPC_WIDEFREEZE:
	case NPC_WIDESLEEP:
	case NPC_WIDESILENCE:
	case NPC_WIDESTONE:
	case NPC_WIDESTUN:
	case NPC_SLOWCAST:
	case NPC_WIDEHELLDIGNITY:
		if (flag&1)
			sc_start(bl,type,100,skilllv,skill_get_time2(skillid,skilllv));
		else {
			skill_area_temp[2] = 0; //For SD_PREAMBLE
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinrange(skill_area_sub, bl,
				skill_get_splash(skillid, skilllv),BL_CHAR,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
				skill_castend_nodamage_id);
		}
		break;
	case NPC_WIDESOULDRAIN:
		if (flag&1)
			status_percent_damage(src,bl,0,((skilllv-1)%5+1)*20,false);
		else {
			skill_area_temp[2] = 0; //For SD_PREAMBLE
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinrange(skill_area_sub, bl,
				skill_get_splash(skillid, skilllv),BL_CHAR,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
				skill_castend_nodamage_id);
		}
		break;
	case ALL_PARTYFLEE:
		if( sd  && !(flag&1) )
		{
			if( !sd->status.party_id ) 
			{
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		else 
			clif_skill_nodamage(src,bl,skillid,skilllv,sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		break;
	case NPC_TALK:
	case ALL_WEWISH:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case ALL_BUYING_STORE:
		if( sd )
		{// players only, skill allows 5 buying slots
			clif_skill_nodamage(src, bl, skillid, skilllv, buyingstore_setup(sd, MAX_BUYINGSTORE_SLOTS));
		}
		break;
	case RK_ENCHANTBLADE:
		clif_skill_nodamage(src,bl,skillid,skilllv,// formula not confirmed
			sc_start2(bl,type,100,skilllv,100+20*skilllv/*+sstatus->int_/2+status_get_lv(bl)/10*/,skill_get_time(skillid,skilllv)));
		break;
	case RK_DRAGONHOWLING:
		if( flag&1)
			sc_start(bl,type,50 + 6 * skilllv,skilllv,skill_get_time(skillid,skilllv));
		else
		{
			skill_area_temp[2] = 0;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinrange(skill_area_sub, src,
				skill_get_splash(skillid,skilllv),BL_CHAR,
				src,skillid,skilllv,tick,flag|BCT_ENEMY|SD_PREAMBLE|1,
				skill_castend_nodamage_id);
		}
		break;
	case RK_IGNITIONBREAK:
	case LG_EARTHDRIVE:
	case MH_LAVA_SLIDE:
			clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			i = skill_get_splash(skillid,skilllv);
			if( skillid == LG_EARTHDRIVE ) {
				int dummy = 1;
				map_foreachinarea(skill_cell_overlap, src->m, src->x-i, src->y-i, src->x+i, src->y+i, BL_SKILL, LG_EARTHDRIVE, &dummy, src);
			}
			map_foreachinrange(skill_area_sub, bl,i,BL_CHAR,
				src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;
	case RK_STONEHARDSKIN:
		if( sd && pc_checkskill(sd,RK_RUNEMASTERY) >= 4 )
		{
			int heal = sstatus->hp / 4; // 25% HP
			if( status_charge(bl,heal,0) )
				clif_skill_nodamage(src,bl,skillid,skilllv,sc_start2(bl,type,100,skilllv,heal,skill_get_time(skillid,skilllv)));
			else
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;
	case RK_REFRESH:
		if( sd && pc_checkskill(sd,RK_RUNEMASTERY) >= 8 )
		{
			int heal = status_get_max_hp(bl) * 25 / 100;
			clif_skill_nodamage(src,bl,skillid,skilllv,
				sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
			status_heal(bl,heal,0,1);
			status_change_clear_buffs(bl,2);
		}
		break;

	case RK_MILLENNIUMSHIELD:
		if( sd && pc_checkskill(sd,RK_RUNEMASTERY) >= 9 )
		{
			short shields = (rnd()%100<50) ? 4 : ((rnd()%100<80) ? 3 : 2);
			sc_start4(bl,type,100,skilllv,shields,1000,0,skill_get_time(skillid,skilllv));
			clif_millenniumshield(sd,shields);
			clif_skill_nodamage(src,bl,skillid,1,1);
		}
		break;

	case RK_GIANTGROWTH:
	case RK_VITALITYACTIVATION:
	case RK_ABUNDANCE:
		if( sd )
		{
			int lv = 1; // RK_GIANTGROWTH
			if( skillid == RK_VITALITYACTIVATION )
				lv = 2;
			else if( skillid == RK_ABUNDANCE )
				lv = 6;
			if( pc_checkskill(sd,RK_RUNEMASTERY) >= lv )
				clif_skill_nodamage(src,bl,skillid,skilllv,sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		}
		break;

	case RK_FIGHTINGSPIRIT:
		if( flag&1 ) {
			if( src == bl )
				sc_start2(bl,type,100,skill_area_temp[5],10*(sd?pc_checkskill(sd,RK_RUNEMASTERY):10),skill_get_time(skillid,skilllv));
			else
				sc_start(bl,type,100,skill_area_temp[5]/4,skill_get_time(skillid,skilllv));
		} else if( sd && pc_checkskill(sd,RK_RUNEMASTERY) >= 5 ) {
			if( sd->status.party_id ) {
				i = party_foreachsamemap(skill_area_sub,sd,skill_get_splash(skillid,skilllv),src,skillid,skilllv,tick,BCT_PARTY,skill_area_sub_count);
				skill_area_temp[5] = 7 * i; // ATK
				party_foreachsamemap(skill_area_sub,sd,skill_get_splash(skillid,skilllv),src,skillid,skilllv,tick,flag|BCT_PARTY|1,skill_castend_nodamage_id);
			} else
				sc_start2(bl,type,100,7,5,skill_get_time(skillid,skilllv));
		}
		clif_skill_nodamage(src,bl,skillid,1,1);
		break;
	/**
	 * Guilotine Cross
	 **/
	case GC_ROLLINGCUTTER:
		{
			short count = 1;
			skill_area_temp[2] = 0;
			map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid,skilllv),BL_CHAR,src,skillid,skilllv,tick,flag|BCT_ENEMY|SD_PREAMBLE|SD_SPLASH|1,skill_castend_damage_id);
			if( tsc && tsc->data[SC_ROLLINGCUTTER] )
			{ // Every time the skill is casted the status change is reseted adding a counter.
				count += (short)tsc->data[SC_ROLLINGCUTTER]->val1;
				if( count > 10 )
					count = 10; // Max coounter
				status_change_end(bl, SC_ROLLINGCUTTER, INVALID_TIMER);
			}
			sc_start(bl,SC_ROLLINGCUTTER,100,count,skill_get_time(skillid,skilllv));
			clif_skill_nodamage(src,src,skillid,skilllv,1);
		}
		break;

	case GC_WEAPONBLOCKING:
		if( tsc && tsc->data[SC_WEAPONBLOCKING] )
			status_change_end(bl, SC_WEAPONBLOCKING, INVALID_TIMER);
		else
			sc_start(bl,SC_WEAPONBLOCKING,100,skilllv,skill_get_time(skillid,skilllv));
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case GC_CREATENEWPOISON:
		if( sd )
		{
			clif_skill_produce_mix_list(sd,skillid,25);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;

	case GC_POISONINGWEAPON:
		if( sd ) {
			clif_poison_list(sd,skilllv);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case GC_ANTIDOTE:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( tsc )
		{
			status_change_end(bl, SC_PARALYSE, INVALID_TIMER);
			status_change_end(bl, SC_PYREXIA, INVALID_TIMER);
			status_change_end(bl, SC_DEATHHURT, INVALID_TIMER);
			status_change_end(bl, SC_LEECHESEND, INVALID_TIMER);
			status_change_end(bl, SC_VENOMBLEED, INVALID_TIMER);
			status_change_end(bl, SC_MAGICMUSHROOM, INVALID_TIMER);
			status_change_end(bl, SC_TOXIN, INVALID_TIMER);
			status_change_end(bl, SC_OBLIVIONCURSE, INVALID_TIMER);
		}
		break;

	case GC_PHANTOMMENACE:
		clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid,skilllv),BL_CHAR,
			src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;

	case GC_HALLUCINATIONWALK:
		{
			int heal = status_get_max_hp(bl) / 10;
			if( status_get_hp(bl) < heal ) { // if you haven't enough HP skill fails.
				if( sd ) clif_skill_fail(sd,skillid,USESKILL_FAIL_HP_INSUFFICIENT,0);
				break;
			}
			if( !status_charge(bl,heal,0) )
			{
				if( sd ) clif_skill_fail(sd,skillid,USESKILL_FAIL_HP_INSUFFICIENT,0);
				break;
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		}
		break;
	/**
	 * Arch Bishop
	 **/
	case AB_ANCILLA:
	 	if( sd ) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			skill_produce_mix(sd, skillid, ITEMID_ANCILLA, 0, 0, 0, 1);
		}
		break;

	case AB_CLEMENTIA:
	case AB_CANTO:
		{
			int bless_lv = pc_checkskill(sd,AL_BLESSING);
			int agi_lv = pc_checkskill(sd,AL_INCAGI);
			if( sd == NULL || sd->status.party_id == 0 || flag&1 )
				clif_skill_nodamage(bl, bl, skillid, skilllv, sc_start(bl,type,100,
					(skillid == AB_CLEMENTIA)? bless_lv : (skillid == AB_CANTO)? agi_lv : skilllv, skill_get_time(skillid,skilllv)));
			else if( sd )
				party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case AB_PRAEFATIO:
		if( sd == NULL || sd->status.party_id == 0 || flag&1 )
			clif_skill_nodamage(bl, bl, skillid, skilllv, sc_start4(bl, type, 100, skilllv, 0, 0, 1, skill_get_time(skillid, skilllv)));
		else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case AB_CHEAL:
		if( sd == NULL || sd->status.party_id == 0 || flag&1 )
		{
			if( sd && tstatus && !battle_check_undead(tstatus->race, tstatus->def_ele) )
			{
				i = skill_calc_heal(src, bl, AL_HEAL, pc_checkskill(sd, AL_HEAL), true);

				if( (dstsd && pc_ismadogear(dstsd)) || status_isimmune(bl) || (tsc && tsc->data[SC_BERSERK]))
						i = 0; // Should heal by 0 or won't do anything?? in iRO it breaks the healing to members.. [malufett]

				clif_skill_nodamage(bl, bl, skillid, i, 1);
				if( tsc && tsc->data[SC_AKAITSUKI] && i )
					i = ~i + 1;
				status_heal(bl, i, 0, 1);
			}
		}
		else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case AB_ORATIO:
		if( flag&1 )
			sc_start(bl, type, 40 + 5 * skilllv, skilllv, skill_get_time(skillid, skilllv));
		else
		{
			map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid, skilllv), BL_CHAR,
				src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;

	case AB_LAUDAAGNUS:
		if( flag&1 || sd == NULL ) {
			if( tsc && (tsc->data[SC_FREEZE] || tsc->data[SC_STONE] || tsc->data[SC_BLIND] ||
				tsc->data[SC_BURNING] || tsc->data[SC_FREEZING] || tsc->data[SC_CRYSTALIZE])) {
				// Success Chance: (40 + 10 * Skill Level) %
				if( rnd()%100 > 40+10*skilllv ) break; 
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				status_change_end(bl, SC_STONE, INVALID_TIMER);
				status_change_end(bl, SC_BLIND, INVALID_TIMER);
				status_change_end(bl, SC_BURNING, INVALID_TIMER);
				status_change_end(bl, SC_FREEZING, INVALID_TIMER);
				status_change_end(bl, SC_CRYSTALIZE, INVALID_TIMER);
			}else //Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
				clif_skill_nodamage(bl, bl, skillid, skilllv,
					sc_start(bl, type, 100, skilllv, skill_get_time(skillid, skilllv)));
		} else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv),
				src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case AB_LAUDARAMUS:
		if( flag&1 || sd == NULL ) {
			if( tsc && (tsc->data[SC_SLEEP] || tsc->data[SC_STUN] || tsc->data[SC_MANDRAGORA] || tsc->data[SC_SILENCE]) ){
				// Success Chance: (40 + 10 * Skill Level) %
				if( rnd()%100 > 40+10*skilllv )  break; 
				status_change_end(bl, SC_SLEEP, INVALID_TIMER);
				status_change_end(bl, SC_STUN, INVALID_TIMER);
				status_change_end(bl, SC_MANDRAGORA, INVALID_TIMER);
				status_change_end(bl, SC_SILENCE, INVALID_TIMER);
			}else // Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
				clif_skill_nodamage(bl, bl, skillid, skilllv,
					sc_start(bl, type, 100, skilllv, skill_get_time(skillid, skilllv)));
		} else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv),
				src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case AB_CLEARANCE:
		if( flag&1 || (i = skill_get_splash(skillid, skilllv)) < 1 )
		{ //As of the behavior in official server Clearance is just a super version of Dispell skill. [Jobbie]
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if((dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER) || rnd()%100 >= 30 + 10 * skilllv)
			{
				if (sd)
					clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
			if(status_isimmune(bl) || !tsc || !tsc->count)
				break;
			for(i=0;i<SC_MAX;i++)
			{
				if (!tsc->data[i])
					continue;
				switch (i) {
				case SC_WEIGHT50:    case SC_WEIGHT90:    case SC_HALLUCINATION:
				case SC_STRIPWEAPON: case SC_STRIPSHIELD: case SC_STRIPARMOR:
				case SC_STRIPHELM:   case SC_CP_WEAPON:   case SC_CP_SHIELD:
				case SC_CP_ARMOR:    case SC_CP_HELM:     case SC_COMBO:
				case SC_STRFOOD:     case SC_AGIFOOD:     case SC_VITFOOD:
				case SC_INTFOOD:     case SC_DEXFOOD:     case SC_LUKFOOD:
				case SC_HITFOOD:     case SC_FLEEFOOD:    case SC_BATKFOOD:
				case SC_WATKFOOD:    case SC_MATKFOOD:    case SC_DANCING:
				case SC_SPIRIT:      case SC_AUTOBERSERK:
				case SC_CARTBOOST:   case SC_MELTDOWN:    case SC_SAFETYWALL:
				case SC_SMA:         case SC_SPEEDUP0:    case SC_NOCHAT:
				case SC_ANKLE:       case SC_SPIDERWEB:   case SC_JAILED:
				case SC_ITEMBOOST:   case SC_EXPBOOST:    case SC_LIFEINSURANCE:
				case SC_BOSSMAPINFO: case SC_PNEUMA:      case SC_AUTOSPELL:
				case SC_INCHITRATE:  case SC_INCATKRATE:  case SC_NEN:
				case SC_READYSTORM:  case SC_READYDOWN:   case SC_READYTURN:
				case SC_READYCOUNTER:case SC_DODGE:       case SC_WARM:
				case SC_SPEEDUP1:    case SC_AUTOTRADE:   case SC_CRITICALWOUND:
				case SC_JEXPBOOST:	 case SC_INVINCIBLE:  case SC_INVINCIBLEOFF:
				case SC_HELLPOWER:	 case SC_MANU_ATK:    case SC_MANU_DEF:
				case SC_SPL_ATK:	 case SC_SPL_DEF:	  case SC_MANU_MATK:
				case SC_SPL_MATK:	 case SC_RICHMANKIM:  case SC_ETERNALCHAOS:
				case SC_DRUMBATTLE:	 case SC_NIBELUNGEN:  case SC_ROKISWEIL:
				case SC_INTOABYSS:	 case SC_SIEGFRIED:	  case SC_WHISTLE:
				case SC_ASSNCROS:	 case SC_POEMBRAGI:	  case SC_APPLEIDUN:
				case SC_HUMMING:	 case SC_DONTFORGETME: case SC_FORTUNE:
				case SC_SERVICE4U:	 case SC_FOOD_STR_CASH:	case SC_FOOD_AGI_CASH:
				case SC_FOOD_VIT_CASH:	case SC_FOOD_DEX_CASH:	case SC_FOOD_INT_CASH:
				case SC_FOOD_LUK_CASH:   case SC_ELECTRICSHOCKER: case SC_BITE:
				case SC__STRIPACCESSORY: case SC__ENERVATION: case SC__GROOMY:
				case SC__IGNORANCE:  case SC__LAZINESS:   case SC__UNLUCKY:
				case SC__WEAKNESS:   //case SC_SAVAGE_STEAK:  case SC_COCKTAIL_WARG_BLOOD:
				case SC_MAGNETICFIELD://case SC_MINOR_BBQ:   case SC_SIROMA_ICE_TEA:
				//case SC_DROCERA_HERB_STEAMED: case SC_PUTTI_TAILS_NOODLES:
				case SC_NEUTRALBARRIER_MASTER: case SC_NEUTRALBARRIER:
				case SC_STEALTHFIELD_MASTER: case SC_STEALTHFIELD:
				case SC_LEADERSHIP:		case SC_GLORYWOUNDS:	case SC_SOULCOLD:
				case SC_HAWKEYES:		case SC_GUILDAURA:	case SC_PUSH_CART:
				case SC_PARTYFLEE:
					continue;
				case SC_ASSUMPTIO:
					if( bl->type == BL_MOB )
						continue;
					break;
				}
				if(i==SC_BERSERK /*|| i==SC_SATURDAYNIGHTFEVER*/) tsc->data[i]->val2=0; //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
				status_change_end(bl,(sc_type)i,INVALID_TIMER);
			}
			break;
		}
		map_foreachinrange(skill_area_sub, bl, i, BL_CHAR, src, skillid, skilllv, tick, flag|1, skill_castend_damage_id);
		break;

	case AB_SILENTIUM:
		// Should the level of Lex Divina be equivalent to the level of Silentium or should the highest level learned be used? [LimitLine]
		map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid, skilllv), BL_CHAR,
			src, PR_LEXDIVINA, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		break;
	/**
	 * Warlock
	 **/
	case WL_STASIS:
		if( flag&1 )
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		else
		{
			map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid, skilllv),BL_CHAR,src,skillid,skilllv,tick,(map_flag_vs(src->m)?BCT_ALL:BCT_ENEMY|BCT_SELF)|flag|1,skill_castend_nodamage_id);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;

	case WL_WHITEIMPRISON:
		if( (src == bl || battle_check_target(src, bl, BCT_ENEMY)) && !is_boss(bl) )// Should not work with bosses.
		{
			int rate = ( sd? sd->status.job_level : 50 ) / 4;
		
			if(src == bl ) rate = 100; // Success Chance: On self, 100%
			else if(bl->type == BL_PC) rate += 20 + 10 * skilllv; // On Players, (20 + 10 * Skill Level) %
			else rate += 40 + 10 * skilllv; // On Monsters, (40 + 10 * Skill Level) %

			if( !(tsc && tsc->data[type]) ){
				i = sc_start2(bl,type,rate,skilllv,src->id,(src == bl)?5000:(bl->type == BL_PC)?skill_get_time(skillid,skilllv):skill_get_time2(skillid, skilllv));
				clif_skill_nodamage(src,bl,skillid,skilllv,i);
			}

			if( sd && i )
				skill_blockpc_start(sd,skillid,4000); // Reuse Delay only activated on success
			else if(sd)
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}else if( sd )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_TOTARGET,0);
		break;

	case WL_FROSTMISTY:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		map_foreachinrange(skill_area_sub,bl,skill_get_splash(skillid,skilllv),BL_CHAR|BL_SKILL,src,skillid,skilllv,tick,flag|BCT_ENEMY,skill_castend_damage_id);
		break;
		
	case WL_JACKFROST:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		map_foreachinshootrange(skill_area_sub,bl,skill_get_splash(skillid,skilllv),BL_CHAR|BL_SKILL,src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;

	case WL_MARSHOFABYSS:
		// Should marsh of abyss still apply half reduction to players after the 28/10 patch? [LimitLine]
		clif_skill_nodamage(src, bl, skillid, skilllv,
			sc_start4(bl, type, 100, skilllv, status_get_int(src), sd ? sd->status.job_level : 50, 0,
			skill_get_time(skillid, skilllv)));
		break;

	case WL_SIENNAEXECRATE:
		if( status_isimmune(bl) || !tsc )
			break;

		if( flag&1 ) {
			if( bl->id == skill_area_temp[1] )
				break; // Already work on this target

			if( tsc && tsc->data[SC_STONE] )
				status_change_end(bl,SC_STONE,INVALID_TIMER);
			else
				status_change_start(bl,SC_STONE,10000,skilllv,0,0,1000,skill_get_time(skillid, skilllv),2);
		} else {
			int rate = 40 + 8 * skilllv + ( sd? sd->status.job_level : 50 ) / 4;
			// IroWiki says Rate should be reduced by target stats, but currently unknown
			if( rnd()%100 < rate ) { // Success on First Target
				if( !tsc->data[SC_STONE] )
					rate = status_change_start(bl,SC_STONE,10000,skilllv,0,0,1000,skill_get_time(skillid, skilllv),2);
				else {
					rate = 1;
					status_change_end(bl,SC_STONE,INVALID_TIMER);
				}

				if( rate ) {
					skill_area_temp[1] = bl->id;
					map_foreachinrange(skill_area_sub,bl,skill_get_splash(skillid,skilllv),BL_CHAR,src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_nodamage_id);
				}
				// Doesn't send failure packet if it fails on defense.
			}
			else if( sd ) // Failure on Rate
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		}
		break;

	case WL_SUMMONFB:
	case WL_SUMMONBL:
	case WL_SUMMONWB:
	case WL_SUMMONSTONE:
		{
			short element = 0, sctype = 0, pos = -1;
			struct status_change *sc = status_get_sc(src);
			if( !sc ) break;

			for( i = SC_SPHERE_1; i <= SC_SPHERE_5; i++ )
			{
				if( !sctype && !sc->data[i] )
					sctype = i; // Take the free SC
				if( sc->data[i] )
					pos = max(sc->data[i]->val2,pos);
			}

			if( !sctype )
			{
				if( sd ) // No free slots to put SC
					clif_skill_fail(sd,skillid,USESKILL_FAIL_SUMMON,0);
				break;
			}

			pos++; // Used in val2 for SC. Indicates the order of this ball
			switch( skillid )
			{ // Set val1. The SC element for this ball
			case WL_SUMMONFB:    element = WLS_FIRE;  break;
			case WL_SUMMONBL:    element = WLS_WIND;  break;
			case WL_SUMMONWB:    element = WLS_WATER; break;
			case WL_SUMMONSTONE: element = WLS_STONE; break;
			}

			sc_start4(src,sctype,100,element,pos,skilllv,0,skill_get_time(skillid,skilllv));
			clif_skill_nodamage(src,bl,skillid,0,0);
		}
		break;

	case WL_READING_SB:
		if( sd ) {
			struct status_change *sc = status_get_sc(bl);

			for( i = SC_SPELLBOOK1; i <= SC_MAXSPELLBOOK; i++)
				if( sc && !sc->data[i] )
					break;
			if( i == SC_MAXSPELLBOOK ) {
				clif_skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_READING, 0);
				break;
			}

			sc_start(bl, SC_STOP, 100, skilllv, INVALID_TIMER); //Can't move while selecting a spellbook.
			clif_spellbook_list(sd);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;
	/**
	 * Ranger
	 **/
	case RA_FEARBREEZE:
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		clif_skill_nodamage(src, bl, skillid, skilllv, sc_start(bl, type, 100, skilllv, skill_get_time(skillid, skilllv)));
		break;

	case RA_WUGMASTERY:
		if( sd ) {
			if( !pc_iswug(sd) )
				pc_setoption(sd,sd->sc.option|OPTION_WUG);
			else
				pc_setoption(sd,sd->sc.option&~OPTION_WUG);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case RA_WUGRIDER:
		if( sd ) {
			if( !pc_isridingwug(sd) && pc_iswug(sd) ) {
				pc_setoption(sd,sd->sc.option&~OPTION_WUG);
				pc_setoption(sd,sd->sc.option|OPTION_WUGRIDER);
			} else if( pc_isridingwug(sd) ) {
				pc_setoption(sd,sd->sc.option&~OPTION_WUGRIDER);
				pc_setoption(sd,sd->sc.option|OPTION_WUG);
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case RA_WUGDASH:
		if( tsce ) {
			clif_skill_nodamage(src,bl,skillid,skilllv,status_change_end(bl, type, -1));
			map_freeblock_unlock();
			return 0;
		}
		if( sd && pc_isridingwug(sd) ) {
			clif_skill_nodamage(src,bl,skillid,skilllv,sc_start4(bl,type,100,skilllv,unit_getdir(bl),0,0,1));
			clif_walkok(sd);
		}
		break;

	case RA_SENSITIVEKEEN:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		clif_skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid,skilllv),BL_CHAR|BL_SKILL,src,skillid,skilllv,tick,flag|BCT_ENEMY,skill_castend_damage_id);
		break;
	/**
	 * Mechanic
	 **/
	case NC_F_SIDESLIDE:
	case NC_B_SIDESLIDE:
		{
			int dir = (skillid == NC_F_SIDESLIDE) ? (unit_getdir(src)+4)%8 : unit_getdir(src);
			skill_blown(src,bl,skill_get_blewcount(skillid,skilllv),dir,0x1);
			clif_slide(src,src->x,src->y);
			clif_fixpos(src); //Aegis sent this packet
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case NC_SELFDESTRUCTION:
		if( sd ) {
			if( pc_ismadogear(sd) )
				 pc_setmadogear(sd, 0);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			skill_castend_damage_id(src, src, skillid, skilllv, tick, flag);
			status_set_sp(src, 0, 0);
		}
		break;

	case NC_ANALYZE:
		clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		clif_skill_nodamage(src, bl, skillid, skilllv,
			sc_start(bl,type, 30 + 12 * skilllv,skilllv,skill_get_time(skillid,skilllv)));
		if( sd ) pc_overheat(sd,1);
		break;

	case NC_MAGNETICFIELD:
		if( (i = sc_start2(bl,type,100,skilllv,src->id,skill_get_time(skillid,skilllv))) )
		{
			map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid,skilllv),splash_target(src),src,skillid,skilllv,tick,flag|BCT_ENEMY|SD_SPLASH|1,skill_castend_damage_id);;
			clif_skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skillid,skilllv,6);
			if (sd) pc_overheat(sd,1);
		}
		clif_skill_nodamage(src,src,skillid,skilllv,i);
		break;

	case NC_REPAIR:
		if( sd )
		{
			int heal;
			if( dstsd && pc_ismadogear(dstsd) )
			{
				heal = dstsd->status.max_hp * (3+3*skilllv) / 100;
				status_heal(bl,heal,0,2);
			} else {
				heal = sd->status.max_hp * (3+3*skilllv) / 100;
				status_heal(src,heal,0,2);
			}

			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			clif_skill_nodamage(src, bl, skillid, skilllv, heal);
		}
		break;

	case NC_DISJOINT:
		{
			if( bl->type != BL_MOB ) break;
			md = map_id2md(bl->id);
			if( md && md->class_ >= 2042 && md->class_ <= 2046 )
				status_kill(bl);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;
	case SC_AUTOSHADOWSPELL:
		if( sd ) {
			if( sd->status.skill[sd->reproduceskill_id].id || sd->status.skill[sd->cloneskill_id].id ) {
				sc_start(src,SC_STOP,100,skilllv,-1);// The skilllv is stored in val1 used in skill_select_menu to determine the used skill lvl [Xazax]
				clif_autoshadowspell_list(sd);
				clif_skill_nodamage(src,bl,skillid,1,1);
			}
			else
				clif_skill_fail(sd,skillid,USESKILL_FAIL_IMITATION_SKILL_NONE,0);
		}
		break;

	case SC_SHADOWFORM:
		if( sd && dstsd && src != bl && !dstsd->shadowform_id ) {
			if( clif_skill_nodamage(src,bl,skillid,skilllv,sc_start4(src,type,100,skilllv,bl->id,4+skilllv,0,skill_get_time(skillid, skilllv))) )
				dstsd->shadowform_id = src->id;
		}
		else if( sd )
			clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);
		break;

	case SC_BODYPAINT:
		if( flag&1 ) {
			if( tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] ||
						tsc->data[SC_CHASEWALK] || tsc->data[SC_CLOAKINGEXCEED] ||
						tsc->data[SC__INVISIBILITY]) ) {
				status_change_end(bl, SC_HIDING, INVALID_TIMER);
				status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
				status_change_end(bl, SC_CHASEWALK, INVALID_TIMER);
				status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
				status_change_end(bl, SC__INVISIBILITY, INVALID_TIMER);

				sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
				sc_start(bl,SC_BLIND,53 + 2 * skilllv,skilllv,skill_get_time(skillid,skilllv));
			}
		} else {
			clif_skill_nodamage(src, bl, skillid, 0, 1);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), BL_CHAR,
				src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		}
		break;

	case SC_ENERVATION:
	case SC_GROOMY:
	case SC_LAZINESS:
	case SC_UNLUCKY:
	case SC_WEAKNESS:
		if( !(tsc && tsc->data[type]) ) {
			//((rand(myDEX / 12, myDEX / 4) + myJobLevel + 10 * skLevel) + myLevel / 10) - (targetLevel / 10 + targetLUK / 10 + (targetMaxWeight - targetWeight) / 1000 + rand(targetAGI / 6, targetAGI / 3))
			int rate = rnd_value(sstatus->dex/12,sstatus->dex/4) + 10*skilllv + (sd?sd->status.job_level:0) + status_get_lv(src)/10
				- status_get_lv(bl)/10 - tstatus->luk/10 - (dstsd?(dstsd->max_weight-dstsd->weight)/10000:0) - rnd_value(tstatus->agi/6,tstatus->agi/3);
			rate = cap_value(rate, skilllv+sstatus->dex/20, 100);
			clif_skill_nodamage(src,bl,skillid,0,sc_start(bl,type,rate,skilllv,skill_get_time(skillid,skilllv)));
		} else if( sd )
			 clif_skill_fail(sd,skillid,0,0);
		break;

	case SC_IGNORANCE:
		if( !(tsc && tsc->data[type]) ) {
			int rate = rnd_value(sstatus->dex/12,sstatus->dex/4) + 10*skilllv + (sd?sd->status.job_level:0) + status_get_lv(src)/10
				- status_get_lv(bl)/10 - tstatus->luk/10 - (dstsd?(dstsd->max_weight-dstsd->weight)/10000:0) - rnd_value(tstatus->agi/6,tstatus->agi/3);
			rate = cap_value(rate, skilllv+sstatus->dex/20, 100);
			if (clif_skill_nodamage(src,bl,skillid,0,sc_start(bl,type,rate,skilllv,skill_get_time(skillid,skilllv)))) {
				int sp = 200 * skilllv;
				if( dstmd ) sp = dstmd->level * 2;
				if( status_zap(bl,0,sp) )
					status_heal(src,0,sp/2,3);
			}
			else if( sd ) clif_skill_fail(sd,skillid,0,0);
		} else if( sd )
			clif_skill_fail(sd,skillid,0,0);
		break;

	case LG_TRAMPLE:
		clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		map_foreachinrange(skill_destroy_trap,bl,skill_get_splash(skillid,skilllv),BL_SKILL,tick);
		break;

	case LG_REFLECTDAMAGE:
		if( tsc && tsc->data[type] )
			status_change_end(bl,type,INVALID_TIMER);
		else
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case LG_SHIELDSPELL:
		if( flag&1 ) {
			int duration = (sd) ? sd->bonus.shieldmdef * 2000 : 10000;
			sc_start(bl,SC_SILENCE,100,skilllv,duration);
		} else if( sd ) {
			int opt = skilllv;
			int rate = rnd()%100;
			int val, brate;
			switch( skilllv ) {
				case 1:
					{
						struct item_data *shield_data = sd->inventory_data[sd->equip_index[EQI_HAND_L]];
						if( !shield_data || shield_data->type != IT_ARMOR ) {	// No shield?
							clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);
							break;
						}
						brate = shield_data->def * 10;
						if( rate < 50 )
							opt = 1;
						else if( rate < 75 )
							opt = 2;
						else
							opt = 3;

						switch( opt ) {
							case 1:
								sc_start(bl,SC_SHIELDSPELL_DEF,100,opt,-1);
								clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
								if( rate < brate )
									map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid,skilllv),BL_CHAR,src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
								status_change_end(bl,SC_SHIELDSPELL_DEF,INVALID_TIMER);
								break;
							case 2:
								val = shield_data->def / 10; // % Reflected damage.
								sc_start2(bl,SC_SHIELDSPELL_DEF,brate,opt,val,shield_data->def * 1000);
								break;
							case 3:
								val = shield_data->def; // Attack increase.
								sc_start2(bl,SC_SHIELDSPELL_DEF,brate,opt,val,shield_data->def * 3000);
								break;
						}
					}
					break;

				case 2:
					brate = sd->bonus.shieldmdef * 20;
					if( rate < 30 )
						opt = 1;
					else if( rate < 60 )
						opt = 2;
					else
						opt = 3;
					switch( opt ) {
						case 1:
							sc_start(bl,SC_SHIELDSPELL_MDEF,100,opt,-1);
							clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
							if( rate < brate )
								map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid,skilllv),BL_CHAR,src,skillid,skilllv,tick,flag|BCT_ENEMY|2,skill_castend_damage_id);
							status_change_end(bl,SC_SHIELDSPELL_MDEF,INVALID_TIMER);
							break;
						case 2:
							sc_start(bl,SC_SHIELDSPELL_MDEF,100,opt,-1);
							clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
							if( rate < brate )
								map_foreachinrange(skill_area_sub,src,skill_get_splash(skillid,skilllv),BL_CHAR,src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_nodamage_id);
							break;
						case 3:
							if( sc_start(bl,SC_SHIELDSPELL_MDEF,brate,opt,sd->bonus.shieldmdef * 30000) )
								clif_skill_nodamage(src,bl,PR_MAGNIFICAT,skilllv,
								sc_start(bl,SC_MAGNIFICAT,100,1,sd->bonus.shieldmdef * 30000));
							break;
					}
					break;

				case 3:
				{
					struct item *it = &sd->status.inventory[sd->equip_index[EQI_HAND_L]];
					if( !it ) {	// No shield?
						clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
						break;
					}
					brate = it->refine * 5;
					if( rate < 25 )
						opt = 1;
					else if( rate < 50 )
						opt = 2;
					else
						opt = 3;
					switch( opt ) {
						case 1:
							val = 105 * it->refine / 10;
							sc_start2(bl,SC_SHIELDSPELL_REF,brate,opt,val,skill_get_time(skillid,skilllv));
							break;
						case 2: case 3:
							if( rate < brate )
							{
								val = sstatus->max_hp * (11 + it->refine) / 100;
								status_heal(bl, val, 0, 3);
							}
							break;
						/*case 3:
							// Full protection. I need confirm what effect should be here. Moved to case 2 to until we got it.
							break;*/
					}
				}
				break;
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case LG_PIETY:
		if( flag&1 )
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		else {
			skill_area_temp[2] = 0;
			map_foreachinrange(skill_area_sub,bl,skill_get_splash(skillid,skilllv),BL_PC,src,skillid,skilllv,tick,flag|SD_PREAMBLE|BCT_PARTY|BCT_SELF|1,skill_castend_nodamage_id);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
	
	case LG_INSPIRATION:
		if( sd && !map[sd->bl.m].flag.noexppenalty && sd->status.base_level != MAX_LEVEL ) {
				sd->status.base_exp -= min(sd->status.base_exp, pc_nextbaseexp(sd) * 1 / 100); // 1% penalty.
				sd->status.job_exp -= min(sd->status.job_exp, pc_nextjobexp(sd) * 1 / 100);
				clif_updatestatus(sd,SP_BASEEXP);
				clif_updatestatus(sd,SP_JOBEXP);
		}
			clif_skill_nodamage(bl,src,skillid,skilllv,
				sc_start(bl, type, 100, skilllv, skill_get_time(skillid, skilllv)));
		break;
	case SR_CURSEDCIRCLE:
		if( flag&1 ) {
			if( is_boss(bl) ) break;
			if( sc_start2(bl, type, 100, skilllv, src->id, skill_get_time(skillid, skilllv))) {
				if( bl->type == BL_MOB )
					mob_unlocktarget((TBL_MOB*)bl,gettick());
				unit_stop_attack(bl);
				clif_bladestop(src, bl->id, 1);
				map_freeblock_unlock();
				return 1;
			}
		} else {
			int count = 0;
			clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			count = map_forcountinrange(skill_area_sub, src, skill_get_splash(skillid,skilllv), (sd)?sd->spiritball_old:15, // Assume 15 spiritballs in non-charactors
				BL_CHAR, src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			if( sd ) pc_delspiritball(sd, count, 0);
			clif_skill_nodamage(src, src, skillid, skilllv,
				sc_start2(src, SC_CURSEDCIRCLE_ATKER, 100, skilllv, count, skill_get_time(skillid,skilllv)));
		}
		break;

	case SR_RAISINGDRAGON:
		if( sd ) {
			short max = 5 + skilllv;
			sc_start(bl, SC_EXPLOSIONSPIRITS, 100, skilllv, skill_get_time(skillid, skilllv));				
			for( i = 0; i < max; i++ ) // Don't call more than max available spheres.
				pc_addspiritball(sd, skill_get_time(skillid, skilllv), max);
			clif_skill_nodamage(src, bl, skillid, skilllv, sc_start(bl, type, 100, skilllv,skill_get_time(skillid, skilllv)));
		}
		break;

	case SR_ASSIMILATEPOWER:
		if( flag&1 ) {
			i = 0;
			if( dstsd && dstsd->spiritball && (sd == dstsd || map_flag_vs(src->m)) && (dstsd->class_&MAPID_BASEMASK)!=MAPID_GUNSLINGER )
			{
				i = dstsd->spiritball; //1%sp per spiritball.
				pc_delspiritball(dstsd, dstsd->spiritball, 0);
			}
			if( i ) status_percent_heal(src, 0, i); 
			clif_skill_nodamage(src, bl, skillid, skilllv, i ? 1:0);
		} else {
			clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY|BCT_SELF|SD_SPLASH|1, skill_castend_nodamage_id);
		}
		break;

	case SR_POWERVELOCITY:
		if( !dstsd )
			break;
		if( sd && dstsd->spiritball <= 5 ) {
			for(i = 0; i <= 5; i++) {
				pc_addspiritball(dstsd, skill_get_time(MO_CALLSPIRITS, pc_checkskill(sd,MO_CALLSPIRITS)), i);
				pc_delspiritball(sd, sd->spiritball, 0);
			}
		}
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		break;

	case SR_GENTLETOUCH_CURE:
		{
			int heal;

			if( status_isimmune(bl) ) 
			{
				clif_skill_nodamage(src,bl,skillid,skilllv,0);
				break;
			}
		
			heal = 120 * skilllv + status_get_max_hp(bl) * (2 + skilllv) / 100;
			status_heal(bl, heal, 0, 0);
			clif_skill_nodamage(src, bl, AL_HEAL, heal, 1);

			if( (tsc && tsc->opt1) && (rnd()%100 < ((skilllv * 5) + (status_get_dex(src) + status_get_lv(src)) / 4) - (1 + (rnd() % 10))) )
			{
				status_change_end(bl, SC_STONE, INVALID_TIMER);
				status_change_end(bl, SC_FREEZE, INVALID_TIMER);
				status_change_end(bl, SC_STUN, INVALID_TIMER);
				status_change_end(bl, SC_POISON, INVALID_TIMER);
				status_change_end(bl, SC_SILENCE, INVALID_TIMER);
				status_change_end(bl, SC_BLIND, INVALID_TIMER);
				status_change_end(bl, SC_HALLUCINATION, INVALID_TIMER);
				status_change_end(bl, SC_BURNING, INVALID_TIMER);
				status_change_end(bl, SC_FREEZING, INVALID_TIMER);
			}

			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
	case SR_GENTLETOUCH_CHANGE:
	case SR_GENTLETOUCH_REVITALIZE:		
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start2(bl,type,100,skilllv,src->id,skill_get_time(skillid,skilllv)));
		break;
	case WA_SWING_DANCE:
	case WA_MOONLIT_SERENADE:
		if( sd == NULL || sd->status.party_id == 0 || (flag & 1) )
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		else if( sd ) {	// Only shows effects on caster.
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case WA_SYMPHONY_OF_LOVER:
	case MI_RUSH_WINDMILL:
	case MI_ECHOSONG:
		if( sd == NULL || sd->status.party_id == 0 || (flag & 1) )
			sc_start4(bl,type,100,skilllv,6*skilllv,(sd?pc_checkskill(sd,WM_LESSON):0),(sd?sd->status.job_level:0),skill_get_time(skillid,skilllv));
		else if( sd ) {	// Only shows effects on caster.
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skillid, skilllv), src, skillid, skilllv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case MI_HARMONIZE:
		if( src != bl )
			clif_skill_nodamage(src, src, skillid, skilllv, sc_start(src, type, 100, skilllv, skill_get_time(skillid,skilllv)));
		clif_skill_nodamage(src, bl, skillid, skilllv, sc_start(bl, type, 100, skilllv, skill_get_time(skillid,skilllv)));
		break;

	case WM_DEADHILLHERE:
		if( bl->type == BL_PC ) {
			if( !status_isdead(bl) )
				break;

			if( rnd()%100 < 88 + 2 * skilllv ) {
				int heal = tstatus->sp;
				if( heal <= 0 )
					heal = 1;
				tstatus->hp = heal;
				tstatus->sp -= tstatus->sp * ( 120 - 20 * skilllv ) / 100;
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				pc_revive((TBL_PC*)bl,heal,0);
				clif_resurrection(bl,1);
			}
		}
		break;
		
	case WM_SIRCLEOFNATURE:
		flag |= BCT_SELF|BCT_PARTY|BCT_GUILD;
	case WM_VOICEOFSIREN:
		if( skillid != WM_SIRCLEOFNATURE )
			flag &= ~BCT_SELF;
		if( flag&1 ) {
			sc_start2(bl,type,(skillid==WM_VOICEOFSIREN)?20+10*skilllv:100,skilllv,(skillid==WM_VOICEOFSIREN)?src->id:0,skill_get_time(skillid,skilllv));
		} else {
			map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid,skilllv),(skillid==WM_VOICEOFSIREN)?BL_CHAR|BL_SKILL:BL_PC, src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case WM_GLOOMYDAY:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( dstsd && ( pc_checkskill(dstsd,KN_BRANDISHSPEAR) || pc_checkskill(dstsd,LK_SPIRALPIERCE) ||
				pc_checkskill(dstsd,CR_SHIELDCHARGE) || pc_checkskill(dstsd,CR_SHIELDBOOMERANG) ||
				pc_checkskill(dstsd,PA_SHIELDCHAIN) || pc_checkskill(dstsd,LG_SHIELDPRESS) ) )
			{
				sc_start(bl,SC_GLOOMYDAY_SK,100,skilllv,skill_get_time(skillid,skilllv));
				break;
			}
		sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
		break;

	case WM_SATURDAY_NIGHT_FEVER:
		if( flag&1 ) {	// Affect to all targets arround the caster and caster too.
			if( !(tsc && tsc->data[type]) )
				sc_start(bl, type, 100, skilllv,skill_get_time(skillid, skilllv));
		} else if( flag&2 ) {
			if( src->id != bl->id && battle_check_target(src,bl,BCT_ENEMY) > 0 )
				status_fix_damage(src,bl,9999,clif_damage(src,bl,tick,0,0,9999,0,0,0));
		} else if( sd ) {
			if( !sd->status.party_id ) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_NEED_HELPER,0);
				break;
			}
			if( map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid,skilllv),
					BL_PC, src, skillid, skilllv, tick, BCT_ENEMY, skill_area_sub_count) > 7 )
				flag |= 2;
			else
				flag |= 1;
			map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid,skilllv),BL_PC, src, skillid, skilllv, tick, flag|BCT_ENEMY|BCT_SELF, skill_castend_nodamage_id);
			clif_skill_nodamage(src, bl, skillid, skilllv,
				sc_start(src,SC_STOP,100,skilllv,skill_get_time2(skillid,skilllv)));
			if( flag&2 ) // Dealed here to prevent conflicts
				status_fix_damage(src,bl,9999,clif_damage(src,bl,tick,0,0,9999,0,0,0));
		}
		break;

	case WM_SONG_OF_MANA:
	case WM_DANCE_WITH_WUG:
	case WM_LERADS_DEW:
		if( flag&1 ) {	// These affect to to all party members near the caster.
			struct status_change *sc = status_get_sc(src);
			if( sc && sc->data[type] ) {
				sc_start2(bl,type,100,skilllv,sc->data[type]->val2,skill_get_time(skillid,skilllv));
			}
		} else if( sd ) {
			short lv = (short)skilllv;
			int count = skill_check_pc_partner(sd,skillid,&lv,skill_get_splash(skillid,skilllv),1);
			if( sc_start2(bl,type,100,skilllv,count,skill_get_time(skillid,skilllv)) )
				party_foreachsamemap(skill_area_sub,sd,skill_get_splash(skillid,skilllv),src,skillid,skilllv,tick,flag|BCT_PARTY|1,skill_castend_nodamage_id);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);

		}
		break;

	case WM_MELODYOFSINK:
	case WM_BEYOND_OF_WARCRY:
	case WM_UNLIMITED_HUMMING_VOICE:
		if( flag&1 ) {
			sc_start2(bl,type,100,skilllv,skill_area_temp[0],skill_get_time(skillid,skilllv));
		} else {	// These affect to all targets arround the caster.
			short lv = (short)skilllv;
			skill_area_temp[0] = (sd) ? skill_check_pc_partner(sd,skillid,&lv,skill_get_splash(skillid,skilllv),1) : 50; // 50% chance in non BL_PC (clones).
			map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid,skilllv),BL_PC, src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case WM_RANDOMIZESPELL: {
			int improv_skillid = 0, improv_skilllv;
			do {
				i = rnd() % MAX_SKILL_IMPROVISE_DB;
				improv_skillid = skill_improvise_db[i].skillid;
			} while( improv_skillid == 0 || rnd()%10000 >= skill_improvise_db[i].per );
			improv_skilllv = 4 + skilllv;
			clif_skill_nodamage (src, bl, skillid, skilllv, 1);

			if( sd ) {
				sd->state.abra_flag = 2;
				sd->skillitem = improv_skillid;
				sd->skillitemlv = improv_skilllv;
				clif_item_skill(sd, improv_skillid, improv_skilllv);
			} else {
				struct unit_data *ud = unit_bl2ud(src);
				int inf = skill_get_inf(improv_skillid);
				int target_id = 0;
				if (!ud) break;
				if (inf&INF_SELF_SKILL || inf&INF_SUPPORT_SKILL) {
					if (src->type == BL_PET)
						bl = (struct block_list*)((TBL_PET*)src)->msd;
					if (!bl) bl = src;
					unit_skilluse_id(src, bl->id, improv_skillid, improv_skilllv);
				} else {
					if (ud->target)
						target_id = ud->target;
					else switch (src->type) {
						case BL_MOB: target_id = ((TBL_MOB*)src)->target_id; break;
						case BL_PET: target_id = ((TBL_PET*)src)->target_id; break;
					}
					if (!target_id)
						break;
					if (skill_get_casttype(improv_skillid) == CAST_GROUND) {
						bl = map_id2bl(target_id);
						if (!bl) bl = src;
						unit_skilluse_pos(src, bl->x, bl->y, improv_skillid, improv_skilllv);
					} else
						unit_skilluse_id(src, target_id, improv_skillid, improv_skilllv);
				}
			}
		}
		break;


	case RETURN_TO_ELDICASTES:
	case ALL_GUARDIAN_RECALL:
		if( sd )
		{
			short x, y; // Destiny position.
			unsigned short mapindex;

			if( skillid == RETURN_TO_ELDICASTES)
			{
				x = 198;
				y = 187;
				mapindex  = mapindex_name2id(MAP_DICASTES);
			}
			else
			{
				x = 44;
				y = 151;
				mapindex  = mapindex_name2id(MAP_MORA);
			}

			if(!mapindex)
			{ //Given map not found?
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				map_freeblock_unlock();
				return 0;
			}
			pc_setpos(sd, mapindex, x, y, CLR_TELEPORT);
		}
		break;

	case GM_SANDMAN:
		if( tsc ) {
			if( tsc->opt1 == OPT1_SLEEP )
				tsc->opt1 = 0;
			else
				tsc->opt1 = OPT1_SLEEP;
			clif_changeoption(bl);
			clif_skill_nodamage (src, bl, skillid, skilllv, 1);
		}
		break;

	case SO_ARRULLO:
		if( flag&1 )
			sc_start2(bl, type, 88 + 2 * skilllv, skilllv, 1, skill_get_time(skillid, skilllv));
		else {
			clif_skill_nodamage(src, bl, skillid, 0, 1);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), BL_CHAR,
							   src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		}
		break;
		
	case SO_SUMMON_AGNI:
	case SO_SUMMON_AQUA:
	case SO_SUMMON_VENTUS:
	case SO_SUMMON_TERA:
		if( sd ) {
			int elemental_class = skill_get_elemental_type(skillid,skilllv);
			
			// Remove previous elemental fisrt.
			if( sd->ed && elemental_delete(sd->ed,0) ) {
				clif_skill_fail(sd,skillid,0,0);
				break;
			}
			// Summoning the new one.
			if( !elemental_create(sd,elemental_class,skill_get_time(skillid,skilllv)) ) {
				clif_skill_fail(sd,skillid,0,0);
				break;
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
		
	case SO_EL_CONTROL:
		if( sd ) {
			int mode = EL_MODE_PASSIVE;	// Standard mode.
			if( !sd->ed ) {
				clif_skill_fail(sd,skillid,0,0);
				break;
			}
			if( skilllv == 4 ) {// At level 4 delete elementals.
				if( elemental_delete(sd->ed, 0) )
					clif_skill_fail(sd,skillid,0,0);
				break;
			}
			switch( skilllv ) {// Select mode bassed on skill level used.
				case 1: mode = EL_MODE_PASSIVE; break;
				case 2: mode = EL_MODE_ASSIST; break;
				case 3: mode = EL_MODE_AGGRESSIVE; break;
			}
			if( !elemental_change_mode(sd->ed,mode) ) {
				clif_skill_fail(sd,skillid,0,0);
				break;
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
		
	case SO_EL_ACTION:
		if( sd ) {
			if( !sd->ed )
				break;
			elemental_action(sd->ed, bl, tick);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
		
	case SO_EL_CURE:
		if( sd ) {
			struct elemental_data *ed = sd->ed;
			int s_hp = sd->battle_status.hp * 10 / 100, s_sp = sd->battle_status.sp * 10 / 100;
			int e_hp, e_sp;
			if( !ed ) {
				clif_skill_fail(sd,skillid,0,0);
				break;
			}
			if( !status_charge(&sd->bl,s_hp,s_sp) ) {
				clif_skill_fail(sd,skillid,0,0);
				break;
			}
			e_hp = ed->battle_status.max_hp * 10 / 100;
			e_sp = ed->battle_status.max_sp * 10 / 100;
			status_heal(&ed->bl,e_hp,e_sp,3);
			clif_skill_nodamage(src,&ed->bl,skillid,skilllv,1);
		}
		break;
		
	case GN_CHANGEMATERIAL:
	case SO_EL_ANALYSIS:
		if( sd ) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			clif_skill_itemlistwindow(sd,skillid,skilllv);
		}
		break;
		
	case GN_BLOOD_SUCKER:
		{
			struct status_change *sc = status_get_sc(src);
					
			if( sc && sc->bs_counter < skill_get_maxcount( skillid , skilllv) ) {
				if( tsc && tsc->data[type] ){
					(sc->bs_counter)--;
					status_change_end(src, type, INVALID_TIMER); // the first one cancels and the last one will take effect resetting the timer
				}
				clif_skill_nodamage(src, bl, skillid, skilllv, 1);
				sc_start2(bl, type, 100, skilllv, src->id, skill_get_time(skillid,skilllv));
				(sc->bs_counter)++;
			} else if( sd ) {
				clif_skill_fail(sd, skillid, USESKILL_FAIL_LEVEL, 0);
				break;
			}
		}
		break;
		
	case GN_MANDRAGORA:
		if( flag&1 ) {
			if ( clif_skill_nodamage(bl, src, skillid, skilllv,
									 sc_start(bl, type, 25 + 10 * skilllv, skilllv, skill_get_time(skillid, skilllv))) )
				status_zap(bl, 0, status_get_max_sp(bl) * (25 + 5 * skilllv) / 100);
		} else
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), BL_CHAR,
							   src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		break;
		
	case GN_SLINGITEM:
		if( sd ) {
			short ammo_id;
			i = sd->equip_index[EQI_AMMO];
			if( i <= 0 )
				break; // No ammo.
			ammo_id = sd->inventory_data[i]->nameid;
			if( ammo_id <= 0 )
				break;
			sd->itemid = ammo_id;
			if( itemdb_is_GNbomb(ammo_id) ) {
				if(battle_check_target(src,bl,BCT_ENEMY) > 0) {// Only attack if the target is an enemy.
					if( ammo_id == 13263 )
						map_foreachincell(skill_area_sub,bl->m,bl->x,bl->y,BL_CHAR,src,GN_SLINGITEM_RANGEMELEEATK,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
					else
						skill_attack(BF_WEAPON,src,src,bl,GN_SLINGITEM_RANGEMELEEATK,skilllv,tick,flag);
				} else //Otherwise, it fails, shows animation and removes items.
					clif_skill_fail(sd,GN_SLINGITEM_RANGEMELEEATK,0xa,0);
			} else if( itemdb_is_GNthrowable(ammo_id) ){
				struct script_code *script = sd->inventory_data[i]->script;
				if( !script )
					break;
				if( dstsd )
					run_script(script,0,dstsd->bl.id,fake_nd->bl.id);
				else
					run_script(script,0,src->id,0);
			}
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);// This packet is received twice actually, I think it is to show the animation.
		break;
		
	case GN_MIX_COOKING:
	case GN_MAKEBOMB:
	case GN_S_PHARMACY:
		if( sd ) {
			int qty = 1;
			sd->skillid_old = skillid;
			sd->skilllv_old = skilllv;
			if( skillid != GN_S_PHARMACY && skilllv > 1 )
				qty = 10;
			clif_cooking_list(sd,(skillid - GN_MIX_COOKING) + 27,skillid,qty,skillid==GN_MAKEBOMB?5:6);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
	case EL_CIRCLE_OF_FIRE:
	case EL_PYROTECHNIC:
	case EL_HEATER:
	case EL_TROPIC:
	case EL_AQUAPLAY:
	case EL_COOLER:
	case EL_CHILLY_AIR:
	case EL_GUST:
	case EL_BLAST:
	case EL_WILD_STORM:
	case EL_PETROLOGY:
	case EL_CURSED_SOIL:
	case EL_UPHEAVAL:
	case EL_FIRE_CLOAK:
	case EL_WATER_DROP:
	case EL_WIND_CURTAIN:
	case EL_SOLID_SKIN:
	case EL_STONE_SHIELD:
	case EL_WIND_STEP: {
			struct elemental_data *ele = BL_CAST(BL_ELEM, src);
			if( ele ) {
				sc_type type2 = type-1;
				struct status_change *sc = status_get_sc(&ele->bl);
				
				if( (sc && sc->data[type2]) || (tsc && tsc->data[type]) ) {
					elemental_clean_single_effect(ele, skillid);
				} else {
					clif_skill_nodamage(src,bl,skillid,skilllv,1);
					clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
					if( skillid == EL_WIND_STEP )	// There aren't telemport, just push to the master.
						skill_blown(src,bl,skill_get_blewcount(skillid,skilllv),(map_calc_dir(src,bl->x,bl->y)+4)%8,0);
					sc_start(src,type2,100,skilllv,skill_get_time(skillid,skilllv));
					sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv));
				}
			}
		}
		break;
		
	case EL_FIRE_MANTLE:
	case EL_WATER_BARRIER:
	case EL_ZEPHYR:
	case EL_POWER_OF_GAIA:
		clif_skill_nodamage(src,src,skillid,skilllv,1);
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		skill_unitsetting(src,skillid,skilllv,bl->x,bl->y,0);
		break;
		
	case EL_WATER_SCREEN: {
			struct elemental_data *ele = BL_CAST(BL_ELEM, src);
			if( ele ) {
				struct status_change *sc = status_get_sc(&ele->bl);
				sc_type type2 = type-1;
				
				clif_skill_nodamage(src,src,skillid,skilllv,1);
				if( (sc && sc->data[type2]) || (tsc && tsc->data[type]) ) {
					elemental_clean_single_effect(ele, skillid);
				} else {
					// This not heals at the end.
					clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
					sc_start(src,type2,100,skilllv,skill_get_time(skillid,skilllv));
					sc_start(bl,type,100,src->id,skill_get_time(skillid,skilllv));
				}
			}
		}
		break;

	case KO_KAHU_ENTEN:
	case KO_HYOUHU_HUBUKI:
	case KO_KAZEHU_SEIRAN:
	case KO_DOHU_KOUKAI:
		if(sd) {
			int ttype = skill_get_ele(skillid, skilllv);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			pc_add_talisman(sd, skill_get_time(skillid, skilllv), 10, ttype);
		}
		break;

	case KO_ZANZOU:
		if(sd){
			struct mob_data *md;

			md = mob_once_spawn_sub(src, src->m, src->x, src->y, status_get_name(src), 2308, "");
			if( md )
			{
				md->master_id = src->id;
				md->special_state.ai = 4;
				if( md->deletetimer != INVALID_TIMER )
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer (gettick() + skill_get_time(skillid, skilllv), mob_timer_delete, md->bl.id, 0);
				mob_spawn( md );
				pc_setinvincibletimer(sd,500);// unlock target lock
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				skill_blown(src,bl,skill_get_blewcount(skillid,skilllv),unit_getdir(bl),0);
			}
		}
		break;	

	case KO_KYOUGAKU:
		if( dstsd && tsc && !tsc->data[type] && rand()%100 < tstatus->int_/2 ){
			clif_skill_nodamage(src,bl,skillid,skilllv,
				sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		}else if( sd )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		break;

	case KO_JYUSATSU:
		if( dstsd && tsc && !tsc->data[type] && rand()%100 < tstatus->int_/2 ){
			clif_skill_nodamage(src,bl,skillid,skilllv,
				status_change_start(bl,type,10000,skilllv,0,0,0,skill_get_time(skillid,skilllv),1));
			status_zap(bl, tstatus->max_hp*skilllv*5/100 , 0);
			if( status_get_lv(bl) <= status_get_lv(src) )
				status_change_start(bl,SC_COMA,10,skilllv,0,src->id,0,0,0);
		}else if( sd )
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
		break;

	case KO_GENWAKU:
		{
			int x = src->x, y = src->y;
			if (unit_movepos(src,bl->x,bl->y,0,0)) {
				clif_skill_nodamage(src,src,skillid,skilllv,1);
				clif_slide(src,bl->x,bl->y) ;
				sc_start(src,SC_CONFUSION,80,skilllv,skill_get_time(skillid,skilllv));
				if (unit_movepos(bl,x,y,0,0))
				{
					clif_skill_damage(bl,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, -1, 6);
					if( bl->type == BL_PC && pc_issit((TBL_PC*)bl))
						clif_sitting(bl); //Avoid sitting sync problem
					clif_slide(bl,x,y) ;
					sc_start(bl,SC_CONFUSION,80,skilllv,skill_get_time(skillid,skilllv));
				}
			}
		}
		break;

	case OB_AKAITSUKI:
	case OB_OBOROGENSOU:
		if( sd && ( (skillid == OB_OBOROGENSOU && bl->type == BL_MOB) // This skill does not work on monsters.
			|| is_boss(bl) ) ){ // Does not work on Boss monsters.
			clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
	case KO_IZAYOI:
	case OB_ZANGETSU:
	case KG_KYOMU:
	case KG_KAGEMUSYA:
		clif_skill_nodamage(src,bl,skillid,skilllv,
			sc_start(bl,type,100,skilllv,skill_get_time(skillid,skilllv)));
		clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		break;

	case KG_KAGEHUMI:
		if( flag&1 ){
			if(tsc && ( tsc->option&(OPTION_CLOAK|OPTION_HIDE) || 
				tsc->data[SC_CAMOUFLAGE] || tsc->data[SC__SHADOWFORM] ||
				tsc->data[SC_MARIONETTE] || tsc->data[SC_HARMONIZE])){ 
					sc_start(src, type, 100, skilllv, skill_get_time(skillid, skilllv));
					sc_start(bl, type, 100, skilllv, skill_get_time(skillid, skilllv));
					status_change_end(bl, SC_HIDING, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
					status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
					status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
					status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
					status_change_end(bl, SC_MARIONETTE, INVALID_TIMER);
					status_change_end(bl, SC_HARMONIZE, INVALID_TIMER);
			}
			if( skill_area_temp[2] == 1 ){
				clif_skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
				sc_start(src, SC_STOP, 100, skilllv, skill_get_time(skillid, skilllv));
			}
		}else{
			skill_area_temp[2] = 0;
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skillid, skilllv), splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_nodamage_id);	
		}
		break;
	default:
		if( skillid >= HM_SKILLBASE && skillid <= HM_SKILLBASE + MAX_HOMUNSKILL ) {
			if( src->type == BL_HOM && ((TBL_HOM*)src)->master->fd )
				clif_colormes(((TBL_HOM*)src)->master, COLOR_RED, "This skill is not yet supported");
		} else /* temporary until all the homun-s skills are supported otherwise console would fill up with pointless warnings */
			ShowWarning("skill_castend_nodamage_id: Unknown skill used:%d\n",skillid);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		map_freeblock_unlock();
		return 1;
	}

	if(skillid != SR_CURSEDCIRCLE){
		struct status_change *sc = status_get_sc(src);
		if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] )//Should only remove after the skill had been casted.
			status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);
	}

	if (dstmd) { //Mob skill event for no damage skills (damage ones are handled in battle_calc_damage) [Skotlex]
		mob_log_damage(dstmd, src, 0); //Log interaction (counts as 'attacker' for the exp bonus)
		mobskill_event(dstmd, src, tick, MSC_SKILLUSED|(skillid<<16));
	}

	if( sd && !(flag&1) )
	{// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = gettick();

		if( sd->state.arrow_atk )
		{// consume arrow on last invocation to this skill.
			battle_consume_ammo(sd, skillid, skilllv);
		}
		skill_onskillusage(sd, bl, skillid, tick);
		// perform skill requirement consumption
		skill_consume_requirement(sd,skillid,skilllv,2);
	}

	map_freeblock_unlock();
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_id(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list *target, *src;
	struct map_session_data *sd;
	struct mob_data *md;
	struct unit_data *ud;
	struct status_change *sc = NULL;
	int inf,inf2,flag = 0;

	src = map_id2bl(id);
	if( src == NULL )
	{
		ShowDebug("skill_castend_id: src == NULL (tid=%d, id=%d)\n", tid, id);
		return 0;// not found
	}

	ud = unit_bl2ud(src);
	if( ud == NULL )
	{
		ShowDebug("skill_castend_id: ud == NULL (tid=%d, id=%d)\n", tid, id);
		return 0;// ???
	}

	sd = BL_CAST(BL_PC,  src);
	md = BL_CAST(BL_MOB, src);

	if( src->prev == NULL ) {
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if(ud->skillid != SA_CASTCANCEL && ud->skillid != SO_SPELLFIST) {// otherwise handled in unit_skillcastcancel()
		if( ud->skilltimer != tid ) {
			ShowError("skill_castend_id: Timer mismatch %d!=%d!\n", ud->skilltimer, tid);
			ud->skilltimer = INVALID_TIMER;
			return 0;
		}

		if( sd && ud->skilltimer != INVALID_TIMER && (pc_checkskill(sd,SA_FREECAST) > 0 || ud->skillid == LG_EXEEDBREAK) )
		{// restore original walk speed
			ud->skilltimer = INVALID_TIMER;
			status_calc_bl(&sd->bl, SCB_SPEED);
		}

		ud->skilltimer = INVALID_TIMER;
	}

	if (ud->skilltarget == id)
		target = src;
	else
		target = map_id2bl(ud->skilltarget);

	// Use a do so that you can break out of it when the skill fails.
	do {
		if(!target || target->prev==NULL) break;

		if(src->m != target->m || status_isdead(src)) break;

		switch (ud->skillid) {
			//These should become skill_castend_pos
			case WE_CALLPARTNER:
				if(sd) clif_callpartner(sd);
			case WE_CALLPARENT:
			case WE_CALLBABY:
			case AM_RESURRECTHOMUN:
			case PF_SPIDERWEB:
				//Find a random spot to place the skill. [Skotlex]
				inf2 = skill_get_splash(ud->skillid, ud->skilllv);
				ud->skillx = target->x + inf2;
				ud->skilly = target->y + inf2;
				if (inf2 && !map_random_dir(target, &ud->skillx, &ud->skilly)) {
					ud->skillx = target->x;
					ud->skilly = target->y;
				}
				ud->skilltimer=tid;
				return skill_castend_pos(tid,tick,id,data);
			case GN_WALLOFTHORN:
				ud->skillx = target->x;
				ud->skilly = target->y;
				ud->skilltimer = tid;
				return skill_castend_pos(tid,tick,id,data);				
		}

		if(ud->skillid == RG_BACKSTAP) {
			int dir = map_calc_dir(src,target->x,target->y),t_dir = unit_getdir(target);
			if(check_distance_bl(src, target, 0) || map_check_dir(dir,t_dir)) {
				break;
			}
		}

		if( ud->skillid == PR_TURNUNDEAD )
		{
			struct status_data *tstatus = status_get_status_data(target);
			if( !battle_check_undead(tstatus->race, tstatus->def_ele) )
				break;
		}

		if( ud->skillid == RA_WUGSTRIKE ){
			if( !path_search(NULL,src->m,src->x,src->y,target->x,target->y,1,CELL_CHKNOREACH))
				break;
		}

		if( ud->skillid == PR_LEXDIVINA || ud->skillid == MER_LEXDIVINA )
		{
			sc = status_get_sc(target);
			if( battle_check_target(src,target, BCT_ENEMY) <= 0 && (!sc || !sc->data[SC_SILENCE]) )
			{ //If it's not an enemy, and not silenced, you can't use the skill on them. [Skotlex]
				clif_skill_nodamage (src, target, ud->skillid, ud->skilllv, 0);
				break;
			}
		}
		else
		{ // Check target validity.
			inf = skill_get_inf(ud->skillid);
			inf2 = skill_get_inf2(ud->skillid);

			if(inf&INF_ATTACK_SKILL ||
				(inf&INF_SELF_SKILL && inf2&INF2_NO_TARGET_SELF) //Combo skills
					) // Casted through combo.
				inf = BCT_ENEMY; //Offensive skill.
			else if(inf2&INF2_NO_ENEMY)
				inf = BCT_NOENEMY;
			else
				inf = 0;

			if(inf2 & (INF2_PARTY_ONLY|INF2_GUILD_ONLY) && src != target)
			{
				inf |=
					(inf2&INF2_PARTY_ONLY?BCT_PARTY:0)|
					(inf2&INF2_GUILD_ONLY?BCT_GUILD:0);
				//Remove neutral targets (but allow enemy if skill is designed to be so)
				inf &= ~BCT_NEUTRAL;
			}

			if( ud->skillid >= SL_SKE && ud->skillid <= SL_SKA && target->type == BL_MOB )
			{
				if( ((TBL_MOB*)target)->class_ == MOBID_EMPERIUM )
					break;
			}
			else
			if (inf && battle_check_target(src, target, inf) <= 0){
				if (sd) clif_skill_fail(sd,ud->skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}

			if(inf&BCT_ENEMY && (sc = status_get_sc(target)) &&
				sc->data[SC_FOGWALL] &&
				rnd()%100 < 75)
		  	{	//Fogwall makes all offensive-type targetted skills fail at 75%
				if (sd) clif_skill_fail(sd,ud->skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
		}

		//Avoid doing double checks for instant-cast skills.
		if (tid != INVALID_TIMER && !status_check_skilluse(src, target, ud->skillid, 1))
			break;

		if(md) {
			md->last_thinktime=tick +MIN_MOBTHINKTIME;
			if(md->skillidx >= 0 && md->db->skill[md->skillidx].emotion >= 0)
				clif_emotion(src, md->db->skill[md->skillidx].emotion);
		}

		if(src != target && battle_config.skill_add_range &&
			!check_distance_bl(src, target, skill_get_range2(src,ud->skillid,ud->skilllv)+battle_config.skill_add_range))
		{
			if (sd) {
				clif_skill_fail(sd,ud->skillid,USESKILL_FAIL_LEVEL,0);
				if(battle_config.skill_out_range_consume) //Consume items anyway. [Skotlex]
					skill_consume_requirement(sd,ud->skillid,ud->skilllv,3);
			}
			break;
		}

		if( sd )
		{
			if( !skill_check_condition_castend(sd, ud->skillid, ud->skilllv) )
				break;
			else
				skill_consume_requirement(sd,ud->skillid,ud->skilllv,1);
		}
#ifdef OFFICIAL_WALKPATH
		if( !path_search_long(NULL, src->m, src->x, src->y, target->x, target->y, CELL_CHKWALL) )
			break;
#endif
		if( (src->type == BL_MER || src->type == BL_HOM) && !skill_check_condition_mercenary(src, ud->skillid, ud->skilllv, 1) )
			break;

		if (ud->state.running && ud->skillid == TK_JUMPKICK)
        {
            ud->state.running = 0;
            status_change_end(src, SC_RUN, INVALID_TIMER);
			flag = 1;
		}
		
		if (ud->walktimer != INVALID_TIMER && ud->skillid != TK_RUN && ud->skillid != RA_WUGDASH)
			unit_stop_walking(src,1);

		if( !sd || sd->skillitem != ud->skillid || skill_get_delay(ud->skillid,ud->skilllv) )
			ud->canact_tick = tick + skill_delayfix(src, ud->skillid, ud->skilllv); //Tests show wings don't overwrite the delay but skill scrolls do. [Inkfish]
		if( sd && skill_get_cooldown(ud->skillid,ud->skilllv) > 0 ){
			int i, cooldown = skill_get_cooldown(ud->skillid, ud->skilllv);
			for (i = 0; i < ARRAYLENGTH(sd->skillcooldown) && sd->skillcooldown[i].id; i++) { // Increases/Decreases cooldown of a skill by item/card bonuses.
				if (sd->skillcooldown[i].id == ud->skillid){
					cooldown += sd->skillcooldown[i].val;
					break;
				}
			}
			skill_blockpc_start(sd, ud->skillid, cooldown);
		}
		if( battle_config.display_status_timers && sd )
			clif_status_change(src, SI_ACTIONDELAY, 1, skill_delayfix(src, ud->skillid, ud->skilllv), 0, 0, 0);
		if( sd )
		{
			switch( ud->skillid )
			{
			case GS_DESPERADO:
				sd->canequip_tick = tick + skill_get_time(ud->skillid, ud->skilllv);
				break;
			case CR_GRANDCROSS:
			case NPC_GRANDDARKNESS:
				if( (sc = status_get_sc(src)) && sc->data[SC_STRIPSHIELD] )
				{
					const struct TimerData *timer = get_timer(sc->data[SC_STRIPSHIELD]->timer);
					if( timer && timer->func == status_change_timer && DIFF_TICK(timer->tick,gettick()+skill_get_time(ud->skillid, ud->skilllv)) > 0 )
						break;
				}
				sc_start2(src, SC_STRIPSHIELD, 100, 0, 1, skill_get_time(ud->skillid, ud->skilllv));
				break;
			}
		}
		if (skill_get_state(ud->skillid) != ST_MOVE_ENABLE)
			unit_set_walkdelay(src, tick, battle_config.default_walk_delay+skill_get_walkdelay(ud->skillid, ud->skilllv), 1);

		if(battle_config.skill_log && battle_config.skill_log&src->type)
			ShowInfo("Type %d, ID %d skill castend id [id =%d, lv=%d, target ID %d]\n",
				src->type, src->id, ud->skillid, ud->skilllv, target->id);

		map_freeblock_lock();

		// SC_MAGICPOWER needs to switch states before any damage is actually dealt
		skill_toggle_magicpower(src, ud->skillid);
		if( ud->skillid != RA_CAMOUFLAGE ) // only normal attack and auto cast skills benefit from its bonuses
			status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);

		if (skill_get_casttype(ud->skillid) == CAST_NODAMAGE)
			skill_castend_nodamage_id(src,target,ud->skillid,ud->skilllv,tick,flag);
		else
			skill_castend_damage_id(src,target,ud->skillid,ud->skilllv,tick,flag);

		sc = status_get_sc(src);
		if(sc && sc->count) {
			if(sc->data[SC_SPIRIT] &&
				sc->data[SC_SPIRIT]->val2 == SL_WIZARD &&
				sc->data[SC_SPIRIT]->val3 == ud->skillid &&
			  	ud->skillid != WZ_WATERBALL)
				sc->data[SC_SPIRIT]->val3 = 0; //Clear bounced spell check.

			if( sc->data[SC_DANCING] && skill_get_inf2(ud->skillid)&INF2_SONG_DANCE && sd )
				skill_blockpc_start(sd,BD_ADAPTATION,3000);
		}

		if( sd && ud->skillid != SA_ABRACADABRA && ud->skillid != WM_RANDOMIZESPELL ) // they just set the data so leave it as it is.[Inkfish]
			sd->skillitem = sd->skillitemlv = 0;

		if (ud->skilltimer == INVALID_TIMER) {
			if(md) md->skillidx = -1;
			else ud->skillid = 0; //mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skilllv = ud->skilltarget = 0;
		}
		map_freeblock_unlock();
		return 1;
	} while(0);

	//Skill failed.
	if (ud->skillid == MO_EXTREMITYFIST && sd && !(sc && sc->data[SC_FOGWALL]))
  	{	//When Asura fails... (except when it fails from Fog of Wall)
		//Consume SP/spheres
		skill_consume_requirement(sd,ud->skillid, ud->skilllv,1);
		status_set_sp(src, 0, 0);
		sc = &sd->sc;
		if (sc->count)
		{	//End states
			status_change_end(src, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
			status_change_end(src, SC_BLADESTOP, INVALID_TIMER);
		}
		if (target && target->m == src->m)
		{	//Move character to target anyway.
			int dx,dy;
			dx = target->x - src->x;
			dy = target->y - src->y;
			if(dx > 0) dx++;
			else if(dx < 0) dx--;
			if (dy > 0) dy++;
			else if(dy < 0) dy--;

			if (unit_movepos(src, src->x+dx, src->y+dy, 1, 1))
			{	//Display movement + animation.
				clif_slide(src,src->x,src->y);
				clif_skill_damage(src,target,tick,sd->battle_status.amotion,0,0,1,ud->skillid, ud->skilllv, 5);
			}
			clif_skill_fail(sd,ud->skillid,USESKILL_FAIL_LEVEL,0);
		}
	}

	ud->skillid = ud->skilllv = ud->skilltarget = 0;
	if( !sd || sd->skillitem != ud->skillid || skill_get_delay(ud->skillid,ud->skilllv) )
		ud->canact_tick = tick;
	//You can't place a skill failed packet here because it would be
	//sent in ALL cases, even cases where skill_check_condition fails
	//which would lead to double 'skill failed' messages u.u [Skotlex]
	if(sd)
		sd->skillitem = sd->skillitemlv = 0;
	else if(md)
		md->skillidx = -1;
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_pos(int tid, unsigned int tick, int id, intptr_t data)
{
	struct block_list* src = map_id2bl(id);
	int maxcount;
	struct map_session_data *sd;
	struct unit_data *ud = unit_bl2ud(src);
	struct mob_data *md;

	nullpo_ret(ud);

	sd = BL_CAST(BL_PC , src);
	md = BL_CAST(BL_MOB, src);

	if( src->prev == NULL ) {
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if( ud->skilltimer != tid )
	{
		ShowError("skill_castend_pos: Timer mismatch %d!=%d\n", ud->skilltimer, tid);
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if( sd && ud->skilltimer != INVALID_TIMER && ( pc_checkskill(sd,SA_FREECAST) > 0 || ud->skillid == LG_EXEEDBREAK ) )
	{// restore original walk speed
		ud->skilltimer = INVALID_TIMER;
		status_calc_bl(&sd->bl, SCB_SPEED);
	}
	ud->skilltimer = INVALID_TIMER;

	do {
		if( status_isdead(src) )
			break;

		if( !(src->type&battle_config.skill_reiteration) &&
			skill_get_unit_flag(ud->skillid)&UF_NOREITERATION &&
			skill_check_unit_range(src,ud->skillx,ud->skilly,ud->skillid,ud->skilllv)
		  )
		{
			if (sd) clif_skill_fail(sd,ud->skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		if( src->type&battle_config.skill_nofootset &&
			skill_get_unit_flag(ud->skillid)&UF_NOFOOTSET &&
			skill_check_unit_range2(src,ud->skillx,ud->skilly,ud->skillid,ud->skilllv)
		  )
		{
			if (sd) clif_skill_fail(sd,ud->skillid,USESKILL_FAIL_LEVEL,0);
			break;
		}
		if( src->type&battle_config.land_skill_limit &&
			(maxcount = skill_get_maxcount(ud->skillid, ud->skilllv)) > 0
		  ) {
			int i;
			for(i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i] && maxcount;i++) {
				if(ud->skillunit[i]->skill_id == ud->skillid)
					maxcount--;
			}
			if( maxcount == 0 )
			{
				if (sd) clif_skill_fail(sd,ud->skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
		}

		if(tid != INVALID_TIMER)
		{	//Avoid double checks on instant cast skills. [Skotlex]
			if (!status_check_skilluse(src, NULL, ud->skillid, 1))
				break;
			if(battle_config.skill_add_range &&
				!check_distance_blxy(src, ud->skillx, ud->skilly, skill_get_range2(src,ud->skillid,ud->skilllv)+battle_config.skill_add_range)) {
				if (sd && battle_config.skill_out_range_consume) //Consume items anyway.
					skill_consume_requirement(sd,ud->skillid,ud->skilllv,3);
				break;
			}
		}

		if( sd )
		{
			if( !skill_check_condition_castend(sd, ud->skillid, ud->skilllv) )
				break;
			else
				skill_consume_requirement(sd,ud->skillid,ud->skilllv,1);
		}

		if( (src->type == BL_MER || src->type == BL_HOM) && !skill_check_condition_mercenary(src, ud->skillid, ud->skilllv, 1) )
			break;

		if(md) {
			md->last_thinktime=tick +MIN_MOBTHINKTIME;
			if(md->skillidx >= 0 && md->db->skill[md->skillidx].emotion >= 0)
				clif_emotion(src, md->db->skill[md->skillidx].emotion);
		}

		if(battle_config.skill_log && battle_config.skill_log&src->type)
			ShowInfo("Type %d, ID %d skill castend pos [id =%d, lv=%d, (%d,%d)]\n",
				src->type, src->id, ud->skillid, ud->skilllv, ud->skillx, ud->skilly);

		if (ud->walktimer != INVALID_TIMER)
			unit_stop_walking(src,1);

		if( !sd || sd->skillitem != ud->skillid || skill_get_delay(ud->skillid,ud->skilllv) )
			ud->canact_tick = tick + skill_delayfix(src, ud->skillid, ud->skilllv);
		if( sd && skill_get_cooldown(ud->skillid,ud->skilllv) > 0 ){
			int i, cooldown = skill_get_cooldown(ud->skillid, ud->skilllv);
			for (i = 0; i < ARRAYLENGTH(sd->skillcooldown) && sd->skillcooldown[i].id; i++) { // Increases/Decreases cooldown of a skill by item/card bonuses.
				if (sd->skillcooldown[i].id == ud->skillid){
					cooldown += sd->skillcooldown[i].val;
					break;
				}
			}
			skill_blockpc_start(sd, ud->skillid, cooldown);
		}
		if( battle_config.display_status_timers && sd )
			clif_status_change(src, SI_ACTIONDELAY, 1, skill_delayfix(src, ud->skillid, ud->skilllv), 0, 0, 0);
//		if( sd )
//		{
//			switch( ud->skillid )
//			{
//			case ????:
//				sd->canequip_tick = tick + ????;
//				break;
//			}
//		}
		unit_set_walkdelay(src, tick, battle_config.default_walk_delay+skill_get_walkdelay(ud->skillid, ud->skilllv), 1);
		status_change_end(src,SC_CAMOUFLAGE, INVALID_TIMER);// only normal attack and auto cast skills benefit from its bonuses
		map_freeblock_lock();
		skill_castend_pos2(src,ud->skillx,ud->skilly,ud->skillid,ud->skilllv,tick,0);

		if( sd && sd->skillitem != AL_WARP ) // Warp-Portal thru items will clear data in skill_castend_map. [Inkfish]
			sd->skillitem = sd->skillitemlv = 0;

		if (ud->skilltimer == INVALID_TIMER) {
			if (md) md->skillidx = -1;
			else ud->skillid = 0; //Non mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skilllv = ud->skillx = ud->skilly = 0;
		}

		map_freeblock_unlock();
		return 1;
	} while(0);

	if( !sd || sd->skillitem != ud->skillid || skill_get_delay(ud->skillid,ud->skilllv) )
		ud->canact_tick = tick;
	ud->skillid = ud->skilllv = 0;
	if(sd)
		sd->skillitem = sd->skillitemlv = 0;
	else if(md)
		md->skillidx  = -1;
	return 0;

}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_pos2(struct block_list* src, int x, int y, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct map_session_data* sd;
	struct status_change* sc;
	struct status_change_entry *sce;
	struct skill_unit_group* sg;
	enum sc_type type;
	int i;

	//if(skilllv <= 0) return 0;
	if(skillid > 0 && skilllv <= 0) return 0;	// celest

	nullpo_ret(src);

	if(status_isdead(src))
		return 0;

	sd = BL_CAST(BL_PC, src);

	sc = status_get_sc(src);
	type = status_skill2sc(skillid);
	sce = (sc && type != -1)?sc->data[type]:NULL;

	switch (skillid) { //Skill effect.
		case WZ_METEOR:
		case MO_BODYRELOCATION:
		case CR_CULTIVATION:
		case HW_GANBANTEIN:
		case LG_EARTHDRIVE:
			break; //Effect is displayed on respective switch case.
		default:
			if(skill_get_inf(skillid)&INF_SELF_SKILL)
				clif_skill_nodamage(src,src,skillid,skilllv,1);
			else
				clif_skill_poseffect(src,skillid,skilllv,x,y,tick);
	}

	// SC_MAGICPOWER needs to switch states before any damage is actually dealt
	skill_toggle_magicpower(src, skillid);

	switch(skillid)
	{
	case PR_BENEDICTIO:
		skill_area_temp[1] = src->id;
		i = skill_get_splash(skillid, skilllv);
		map_foreachinarea(skill_area_sub,
			src->m, x-i, y-i, x+i, y+i, BL_PC,
			src, skillid, skilllv, tick, flag|BCT_ALL|1,
			skill_castend_nodamage_id);
		map_foreachinarea(skill_area_sub,
			src->m, x-i, y-i, x+i, y+i, BL_CHAR,
			src, skillid, skilllv, tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		break;

	case BS_HAMMERFALL:
		i = skill_get_splash(skillid, skilllv);
		map_foreachinarea (skill_area_sub,
			src->m, x-i, y-i, x+i, y+i, BL_CHAR,
			src, skillid, skilllv, tick, flag|BCT_ENEMY|2,
			skill_castend_nodamage_id);
		break;

	case HT_DETECTING:
		i = skill_get_splash(skillid, skilllv);
		map_foreachinarea( status_change_timer_sub,
			src->m, x-i, y-i, x+i,y+i,BL_CHAR,
			src,NULL,SC_SIGHT,tick);
		if(battle_config.traps_setting&1)
		map_foreachinarea( skill_reveal_trap,
			src->m, x-i, y-i, x+i,y+i,BL_SKILL);
		break;

	case SR_RIDEINLIGHTNING:
		i = skill_get_splash(skillid, skilllv);
		map_foreachinarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR, 
			src, skillid, skilllv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
		break;

	case SA_VOLCANO:
	case SA_DELUGE:
	case SA_VIOLENTGALE:
	{	//Does not consumes if the skill is already active. [Skotlex]
		struct skill_unit_group *sg;
		if ((sg= skill_locate_element_field(src)) != NULL && ( sg->skill_id == SA_VOLCANO || sg->skill_id == SA_DELUGE || sg->skill_id == SA_VIOLENTGALE ))
		{
			if (sg->limit - DIFF_TICK(gettick(), sg->tick) > 0)
			{
				skill_unitsetting(src,skillid,skilllv,x,y,0);
				return 0; // not to consume items
			}
			else
				sg->limit = 0; //Disable it.
		}
		skill_unitsetting(src,skillid,skilllv,x,y,0);
		break;
	}
	case MG_SAFETYWALL:
	case MG_FIREWALL:
	case MG_THUNDERSTORM:

	case AL_PNEUMA:
	case WZ_ICEWALL:
	case WZ_FIREPILLAR:
	case WZ_QUAGMIRE:
	case WZ_VERMILION:
	case WZ_STORMGUST:
	case WZ_HEAVENDRIVE:
	case PR_SANCTUARY:
	case PR_MAGNUS:
	case CR_GRANDCROSS:
	case NPC_GRANDDARKNESS:
	case HT_SKIDTRAP:
	case MA_SKIDTRAP:
	case HT_LANDMINE:
	case MA_LANDMINE:
	case HT_ANKLESNARE:
	case HT_SHOCKWAVE:
	case HT_SANDMAN:
	case MA_SANDMAN:
	case HT_FLASHER:
	case HT_FREEZINGTRAP:
	case MA_FREEZINGTRAP:
	case HT_BLASTMINE:
	case HT_CLAYMORETRAP:
	case AS_VENOMDUST:
	case AM_DEMONSTRATION:
	case PF_FOGWALL:
	case PF_SPIDERWEB:
	case HT_TALKIEBOX:
	case WE_CALLPARTNER:
	case WE_CALLPARENT:
	case WE_CALLBABY:
	case AC_SHOWER:	//Ground-placed skill implementation.
	case MA_SHOWER:
	case SA_LANDPROTECTOR:
	case BD_LULLABY:
	case BD_RICHMANKIM:
	case BD_ETERNALCHAOS:
	case BD_DRUMBATTLEFIELD:
	case BD_RINGNIBELUNGEN:
	case BD_ROKISWEIL:
	case BD_INTOABYSS:
	case BD_SIEGFRIED:
	case BA_DISSONANCE:
	case BA_POEMBRAGI:
	case BA_WHISTLE:
	case BA_ASSASSINCROSS:
	case BA_APPLEIDUN:
	case DC_UGLYDANCE:
	case DC_HUMMING:
	case DC_DONTFORGETME:
	case DC_FORTUNEKISS:
	case DC_SERVICEFORYOU:
	case CG_MOONLIT:
	case GS_DESPERADO:
	case NJ_KAENSIN:
	case NJ_BAKUENRYU:
	case NJ_SUITON:
	case NJ_HYOUSYOURAKU:
	case NJ_RAIGEKISAI:
	case NJ_KAMAITACHI:
#ifdef RENEWAL
	case NJ_HUUMA:
#endif
	case NPC_EVILLAND:
	case RA_ELECTRICSHOCKER:
	case RA_CLUSTERBOMB:
	case RA_MAGENTATRAP:
	case RA_COBALTTRAP:
	case RA_MAIZETRAP:
	case RA_VERDURETRAP:
	case RA_FIRINGTRAP:
	case RA_ICEBOUNDTRAP:
	case SC_MANHOLE:
	case SC_DIMENSIONDOOR:
	case SC_CHAOSPANIC:
	case SC_BLOODYLUST:
	case SC_MAELSTROM:
	case WM_REVERBERATION:
	case WM_SEVERE_RAINSTORM:
	case WM_POEMOFNETHERWORLD:
	case SO_PSYCHIC_WAVE:
	case SO_VACUUM_EXTREME:
	case GN_WALLOFTHORN:
	case GN_THORNS_TRAP:
	case GN_DEMONIC_FIRE:
	case GN_HELLS_PLANT:
	case SO_EARTHGRAVE:
	case SO_DIAMONDDUST:
	case SO_FIRE_INSIGNIA:
	case SO_WATER_INSIGNIA:
	case SO_WIND_INSIGNIA:
	case SO_EARTH_INSIGNIA:
	case MH_POISON_MIST:
	case KO_HUUMARANKA:
	case KO_MUCHANAGE:
	case KO_BAKURETSU:
	case KO_ZENKAI:
	//case KO_MAKIBISHI:
		flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
	case GS_GROUNDDRIFT: //Ammo should be deleted right away.
		skill_unitsetting(src,skillid,skilllv,x,y,0);
		break;
	case RG_GRAFFITI:			/* Graffiti [Valaris] */
		skill_clear_unitgroup(src);
		skill_unitsetting(src,skillid,skilllv,x,y,0);
		flag|=1;
		break;
	case HP_BASILICA:
		if( sc->data[SC_BASILICA] )
			status_change_end(src, SC_BASILICA, INVALID_TIMER); // Cancel Basilica
		else
		{ // Create Basilica. Start SC on caster. Unit timer start SC on others.
			skill_clear_unitgroup(src);
			if( skill_unitsetting(src,skillid,skilllv,x,y,0) )
				sc_start4(src,type,100,skilllv,0,0,src->id,skill_get_time(skillid,skilllv));
			flag|=1;
		}
		break;
	case CG_HERMODE:
		skill_clear_unitgroup(src);
		if ((sg = skill_unitsetting(src,skillid,skilllv,x,y,0)))
			sc_start4(src,SC_DANCING,100,
				skillid,0,skilllv,sg->group_id,skill_get_time(skillid,skilllv));
		flag|=1;
		break;
	case RG_CLEANER: // [Valaris]
		i = skill_get_splash(skillid, skilllv);
		map_foreachinarea(skill_graffitiremover,src->m,x-i,y-i,x+i,y+i,BL_SKILL);
		break;

	case SO_CLOUD_KILL:
	case SO_WARMER:
		flag|=(skillid == SO_WARMER)?8:4;
		skill_unitsetting(src,skillid,skilllv,x,y,0);
		break;
			
	case WZ_METEOR: {
			int area = skill_get_splash(skillid, skilllv);
			short tmpx = 0, tmpy = 0, x1 = 0, y1 = 0;

			for( i = 0; i < 2 + (skilllv>>1); i++ ) {
				// Creates a random Cell in the Splash Area
				tmpx = x - area + rnd()%(area * 2 + 1);
				tmpy = y - area + rnd()%(area * 2 + 1);

				if( i == 0 && path_search_long(NULL, src->m, src->x, src->y, tmpx, tmpy, CELL_CHKWALL) )
					clif_skill_poseffect(src,skillid,skilllv,tmpx,tmpy,tick);

				if( i > 0 )
					skill_addtimerskill(src,tick+i*1000,0,tmpx,tmpy,skillid,skilllv,(x1<<16)|y1,0);

				x1 = tmpx;
				y1 = tmpy;
			}

			skill_addtimerskill(src,tick+i*1000,0,tmpx,tmpy,skillid,skilllv,-1,0);
		}
		break;

	case AL_WARP:
		if(sd)
		{
			clif_skill_warppoint(sd, skillid, skilllv, sd->status.save_point.map,
				(skilllv >= 2) ? sd->status.memo_point[0].map : 0,
				(skilllv >= 3) ? sd->status.memo_point[1].map : 0,
				(skilllv >= 4) ? sd->status.memo_point[2].map : 0
			);
		}
		return 0; // not to consume item.

	case MO_BODYRELOCATION:
		if (unit_movepos(src, x, y, 1, 1)) {
#if PACKETVER >= 20111005
			clif_snap(src, src->x, src->y);
#else
			clif_skill_poseffect(src,skillid,skilllv,src->x,src->y,tick);
#endif
			if (sd)
				skill_blockpc_start (sd, MO_EXTREMITYFIST, 2000);
		}
		break;
	case NJ_SHADOWJUMP:
		if( !map_flag_gvg(src->m) && !map[src->m].flag.battleground ) { //You don't move on GVG grounds.
			unit_movepos(src, x, y, 1, 0);
			clif_slide(src,x,y);
		}
		status_change_end(src, SC_HIDING, INVALID_TIMER);
		break;
	case AM_SPHEREMINE:
	case AM_CANNIBALIZE:
		{
			int summons[5] = { 1589, 1579, 1575, 1555, 1590 };
			//int summons[5] = { 1020, 1068, 1118, 1500, 1368 };
			int class_ = skillid==AM_SPHEREMINE?1142:summons[skilllv-1];
			struct mob_data *md;

			// Correct info, don't change any of this! [celest]
			md = mob_once_spawn_sub(src, src->m, x, y, status_get_name(src),class_,"");
			if (md) {
				md->master_id = src->id;
				md->special_state.ai = skillid==AM_SPHEREMINE?2:3;
				if( md->deletetimer != INVALID_TIMER )
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer (gettick() + skill_get_time(skillid,skilllv), mob_timer_delete, md->bl.id, 0);
				mob_spawn (md); //Now it is ready for spawning.
			}
		}
		break;

	// Slim Pitcher [Celest]
	case CR_SLIMPITCHER:
		if (sd) {
			int i = skilllv%11 - 1;
			int j = pc_search_inventory(sd,skill_db[skillid].itemid[i]);
			if( j < 0 || skill_db[skillid].itemid[i] <= 0 || sd->inventory_data[j] == NULL || sd->status.inventory[j].amount < skill_db[skillid].amount[i] )
			{
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			potion_flag = 1;
			potion_hp = 0;
			potion_sp = 0;
			run_script(sd->inventory_data[j]->script,0,sd->bl.id,0);
			potion_flag = 0;
			//Apply skill bonuses
			i = pc_checkskill(sd,CR_SLIMPITCHER)*10
				+ pc_checkskill(sd,AM_POTIONPITCHER)*10
				+ pc_checkskill(sd,AM_LEARNINGPOTION)*5
				+ pc_skillheal_bonus(sd, skillid);

			potion_hp = potion_hp * (100+i)/100;
			potion_sp = potion_sp * (100+i)/100;

			if(potion_hp > 0 || potion_sp > 0) {
				i = skill_get_splash(skillid, skilllv);
				map_foreachinarea(skill_area_sub,
					src->m,x-i,y-i,x+i,y+i,BL_CHAR,
					src,skillid,skilllv,tick,flag|BCT_PARTY|BCT_GUILD|1,
					skill_castend_nodamage_id);
			}
		} else {
			int i = skilllv%11 - 1;
			struct item_data *item;
			i = skill_db[skillid].itemid[i];
			item = itemdb_search(i);
			potion_flag = 1;
			potion_hp = 0;
			potion_sp = 0;
			run_script(item->script,0,src->id,0);
			potion_flag = 0;
			i = skill_get_max(CR_SLIMPITCHER)*10;

			potion_hp = potion_hp * (100+i)/100;
			potion_sp = potion_sp * (100+i)/100;

			if(potion_hp > 0 || potion_sp > 0) {
				i = skill_get_splash(skillid, skilllv);
				map_foreachinarea(skill_area_sub,
					src->m,x-i,y-i,x+i,y+i,BL_CHAR,
					src,skillid,skilllv,tick,flag|BCT_PARTY|BCT_GUILD|1,
						skill_castend_nodamage_id);
			}
		}
		break;

	case HW_GANBANTEIN:
		if (rnd()%100 < 80) {
			int dummy = 1;
			clif_skill_poseffect(src,skillid,skilllv,x,y,tick);
			i = skill_get_splash(skillid, skilllv);
			map_foreachinarea(skill_cell_overlap, src->m, x-i, y-i, x+i, y+i, BL_SKILL, HW_GANBANTEIN, &dummy, src);
		} else {
			if (sd) clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			return 1;
		}
		break;

	case HW_GRAVITATION:
		if ((sg = skill_unitsetting(src,skillid,skilllv,x,y,0)))
			sc_start4(src,type,100,skilllv,0,BCT_SELF,sg->group_id,skill_get_time(skillid,skilllv));
		flag|=1;
		break;

	// Plant Cultivation [Celest]
	case CR_CULTIVATION:
		if (sd) {
			if( map_count_oncell(src->m,x,y,BL_CHAR) > 0 )
			{
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				return 1;
			}
			clif_skill_poseffect(src,skillid,skilllv,x,y,tick);
			if (rnd()%100 < 50) {
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
			} else {
				TBL_MOB* md = mob_once_spawn_sub(src, src->m, x, y, "--ja--",(skilllv < 2 ? 1084+rnd()%2 : 1078+rnd()%6),"");
				int i;
				if (!md) break;
				if ((i = skill_get_time(skillid, skilllv)) > 0)
				{
					if( md->deletetimer != INVALID_TIMER )
						delete_timer(md->deletetimer, mob_timer_delete);
					md->deletetimer = add_timer (tick + i, mob_timer_delete, md->bl.id, 0);
				}
				mob_spawn (md);
			}
		}
		break;

	case SG_SUN_WARM:
	case SG_MOON_WARM:
	case SG_STAR_WARM:
		skill_clear_unitgroup(src);
		if ((sg = skill_unitsetting(src,skillid,skilllv,src->x,src->y,0)))
			sc_start4(src,type,100,skilllv,0,0,sg->group_id,skill_get_time(skillid,skilllv));
		flag|=1;
		break;

	case PA_GOSPEL:
		if (sce && sce->val4 == BCT_SELF)
		{
			status_change_end(src, SC_GOSPEL, INVALID_TIMER);
			return 0;
		}
		else
	  	{
			sg = skill_unitsetting(src,skillid,skilllv,src->x,src->y,0);
			if (!sg) break;
			if (sce)
				status_change_end(src, type, INVALID_TIMER); //Was under someone else's Gospel. [Skotlex]
			sc_start4(src,type,100,skilllv,0,sg->group_id,BCT_SELF,skill_get_time(skillid,skilllv));
			clif_skill_poseffect(src, skillid, skilllv, 0, 0, tick); // PA_GOSPEL music packet
		}
		break;
	case NJ_TATAMIGAESHI:
		if (skill_unitsetting(src,skillid,skilllv,src->x,src->y,0))
			sc_start(src,type,100,skilllv,skill_get_time2(skillid,skilllv));
		break;

	case AM_RESURRECTHOMUN:	//[orn]
		if (sd)
		{
			if (!merc_resurrect_homunculus(sd, 20*skilllv, x, y))
			{
				clif_skill_fail(sd,skillid,USESKILL_FAIL_LEVEL,0);
				break;
			}
		}
		break;

	case NC_COLDSLOWER:
	case NC_ARMSCANNON:
	case RK_DRAGONBREATH:
	case RK_WINDCUTTER:
	case WM_LULLABY_DEEPSLEEP:
		i = skill_get_splash(skillid,skilllv);
		map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,splash_target(src),
			src,skillid,skilllv,tick,flag|(skillid==WM_LULLABY_DEEPSLEEP?BCT_ALL:BCT_ENEMY)|1,skill_castend_damage_id);
		break;
	/**
	 * Guilotine Cross
	 **/
	case GC_POISONSMOKE:
		if( !(sc && sc->data[SC_POISONINGWEAPON]) ) {
			if( sd )
				clif_skill_fail(sd,skillid,USESKILL_FAIL_GC_POISONINGWEAPON,0);
			return 0;
		}
		clif_skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skillid,skilllv,6);
		skill_unitsetting(src, skillid, skilllv, x, y, flag);
		//status_change_end(src,SC_POISONINGWEAPON,INVALID_TIMER); // 08/31/2011 - When using poison smoke, you no longer lose the poisoning weapon effect.
		break;
	/**
	 * Arch Bishop
	 **/
	case AB_EPICLESIS:
		if( (sg = skill_unitsetting(src, skillid, skilllv, x, y, 0)) ) {
			i = sg->unit->range;
			map_foreachinarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR, src, ALL_RESURRECTION, 1, tick, flag|BCT_NOENEMY|1,skill_castend_nodamage_id);
		}
		break;
	/**
	 * Warlock
	 **/
	case WL_COMET:
		if( sc ) {
			sc->comet_x = x;
			sc->comet_y = y;
		}
		i = skill_get_splash(skillid,skilllv);
		map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,splash_target(src),src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;

	case WL_EARTHSTRAIN:
		{
			int i, wave = skilllv + 4, dir = map_calc_dir(src,x,y);
			int sx = x = src->x, sy = y = src->y; // Store first caster's location to avoid glitch on unit setting

			for( i = 1; i <= wave; i++ )
			{
				switch( dir ){
					case 0: case 1: case 7: sy = y + i; break;
					case 3: case 4: case 5: sy = y - i; break;
					case 2: sx = x - i; break;
					case 6: sx = x + i; break;
				}
				skill_addtimerskill(src,gettick() + (150 * i),0,sx,sy,skillid,skilllv,dir,flag&2);
			}
		}
		break;
	/**
	 * Ranger
	 **/
	case RA_DETONATOR:
		i = skill_get_splash(skillid, skilllv);
		map_foreachinarea(skill_detonator, src->m, x-i, y-i, x+i, y+i, BL_SKILL, src);
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skillid, skilllv, 6);
		break;
	/**
	 * Mechanic
	 **/
	case NC_NEUTRALBARRIER:
	case NC_STEALTHFIELD:
		skill_clear_unitgroup(src); // To remove previous skills - cannot used combined
		if( (sg = skill_unitsetting(src,skillid,skilllv,src->x,src->y,0)) != NULL ) {
			sc_start2(src,skillid == NC_NEUTRALBARRIER ? SC_NEUTRALBARRIER_MASTER : SC_STEALTHFIELD_MASTER,100,skilllv,sg->group_id,skill_get_time(skillid,skilllv));
			if( sd ) pc_overheat(sd,1);
		}
		break;

	case NC_SILVERSNIPER:
		{
			int class_ = 2042;
			struct mob_data *md;

			md = mob_once_spawn_sub(src, src->m, x, y, status_get_name(src), class_, "");
			if( md )
			{
				md->master_id = src->id;
				md->special_state.ai = 3;
				if( md->deletetimer != INVALID_TIMER )
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer (gettick() + skill_get_time(skillid, skilllv), mob_timer_delete, md->bl.id, 0);
				mob_spawn( md );
			}
		}
		break;

	case NC_MAGICDECOY:
		if( sd ) clif_magicdecoy_list(sd,skilllv,x,y);
		break;

	case SC_FEINTBOMB:
		clif_skill_nodamage(src,src,skillid,skilllv,1);
		skill_unitsetting(src,skillid,skilllv,x,y,0); // Set bomb on current Position
		skill_blown(src,src,6,unit_getdir(src),0);
		break;

	case LG_OVERBRAND:
		{
			int width;//according to data from irowiki it actually is a square
			for( width = 0; width < 7; width++ )
				for( i = 0; i < 7; i++ )
					map_foreachincell(skill_area_sub, src->m, x-2+i, y-2+width, splash_target(src), src, LG_OVERBRAND_BRANDISH, skilllv, tick, flag|BCT_ENEMY,skill_castend_damage_id);
			for( width = 0; width < 7; width++ )
				for( i = 0; i < 7; i++ )
					map_foreachincell(skill_area_sub, src->m, x-2+i, y-2+width, splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY,skill_castend_damage_id);
		}
		break;

	case LG_BANDING:
		if( sc && sc->data[SC_BANDING] )
			status_change_end(src,SC_BANDING,INVALID_TIMER);
		else if( (sg = skill_unitsetting(src,skillid,skilllv,src->x,src->y,0)) != NULL ) {
			sc_start4(src,SC_BANDING,100,skilllv,0,0,sg->group_id,skill_get_time(skillid,skilllv));
			if( sd ) pc_banding(sd,skilllv);
		}
		clif_skill_nodamage(src,src,skillid,skilllv,1);
		break;

	case LG_RAYOFGENESIS:
		if( status_charge(src,status_get_max_hp(src)*3*skilllv / 100,0) ) {
			i = skill_get_splash(skillid,skilllv);
			map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,splash_target(src),
				src,skillid,skilllv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		} else if( sd )
			clif_skill_fail(sd,skillid,USESKILL_FAIL,0);
		break;

	case WM_DOMINION_IMPULSE:
		i = skill_get_splash(skillid, skilllv);
		map_foreachinarea( skill_ative_reverberation,
			src->m, x-i, y-i, x+i,y+i,BL_SKILL);
		break;
		
	case WM_GREAT_ECHO:
		flag|=1; // Should counsume 1 item per skill usage.
		map_foreachinrange(skill_area_sub, src, skill_get_splash(skillid,skilllv),splash_target(src), src, skillid, skilllv, tick, flag|BCT_ENEMY, skill_castend_damage_id);
		break;
	case GN_CRAZYWEED: {
			int area = skill_get_splash(GN_CRAZYWEED_ATK, skilllv);
			short x1 = 0, y1 = 0;

			for( i = 0; i < 3 + (skilllv/2); i++ ) {
				x1 = x - area + rnd()%(area * 2 + 1);
				y1 = y - area + rnd()%(area * 2 + 1);
				skill_addtimerskill(src,tick+i*150,0,x1,y1,GN_CRAZYWEED_ATK,skilllv,-1,0);
			}
		}
		break;		
	case GN_FIRE_EXPANSION: {
		int i;
		struct unit_data *ud = unit_bl2ud(src);
		
		if( !ud ) break;
		
		for( i = 0; i < MAX_SKILLUNITGROUP && ud->skillunit[i]; i ++ ) {
			if( ud->skillunit[i]->skill_id == GN_DEMONIC_FIRE &&
			   distance_xy(x, y, ud->skillunit[i]->unit->bl.x, ud->skillunit[i]->unit->bl.y) < 4 ) {
				switch( skilllv ) {							
					case 3:
						ud->skillunit[i]->unit_id = UNT_FIRE_EXPANSION_SMOKE_POWDER;
						clif_changetraplook(&ud->skillunit[i]->unit->bl, UNT_FIRE_EXPANSION_SMOKE_POWDER);
						break;
					case 4:
						ud->skillunit[i]->unit_id = UNT_FIRE_EXPANSION_TEAR_GAS;
						clif_changetraplook(&ud->skillunit[i]->unit->bl, UNT_FIRE_EXPANSION_TEAR_GAS);
						break;
					case 5:
						map_foreachinarea(skill_area_sub, src->m,
										  ud->skillunit[i]->unit->bl.x - 3, ud->skillunit[i]->unit->bl.y - 3,
										  ud->skillunit[i]->unit->bl.x + 3, ud->skillunit[i]->unit->bl.y + 3, BL_CHAR,
										  src, CR_ACIDDEMONSTRATION, sd ? pc_checkskill(sd, CR_ACIDDEMONSTRATION) : skilllv, tick, flag|BCT_ENEMY|1|SD_LEVEL, skill_castend_damage_id);
						skill_delunit(ud->skillunit[i]->unit);
						break;
					default:
						ud->skillunit[i]->unit->val2 = skilllv;
						ud->skillunit[i]->unit->group->val2 = skilllv;
						break;
					}
				}
			}
		}
		break;
		
	case SO_FIREWALK:
	case SO_ELECTRICWALK:
		if( sc && sc->data[type] )
			status_change_end(src,type,INVALID_TIMER);
		clif_skill_nodamage(src, src ,skillid, skilllv,
							sc_start2(src, type, 100, skillid, skilllv, skill_get_time(skillid, skilllv)));
		break;

	default:
		if( skillid >= HM_SKILLBASE && skillid <= HM_SKILLBASE + MAX_HOMUNSKILL ) {
			if( src->type == BL_HOM && ((TBL_HOM*)src)->master->fd )
				clif_colormes(((TBL_HOM*)src)->master, COLOR_RED, "This skill is not yet supported");
		} else /* temporary until all the homun-s skills are supported otherwise console would fill up with pointless warnings */
			ShowWarning("skill_castend_pos2: Unknown skill used:%d\n",skillid);
		return 1;
	}

	if( sc && sc->data[SC_CURSEDCIRCLE_ATKER] ) //Should only remove after the skill has been casted.
		status_change_end(src,SC_CURSEDCIRCLE_ATKER,INVALID_TIMER);

	if( sd )
	{// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = gettick();

		if( sd->state.arrow_atk && !(flag&1) )
		{// consume arrow if this is a ground skill
			battle_consume_ammo(sd, skillid, skilllv);
		}

		// perform skill requirement consumption
		skill_consume_requirement(sd,skillid,skilllv,2);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_map (struct map_session_data *sd, short skill_num, const char *map)
{
	nullpo_ret(sd);

//Simplify skill_failed code.
#define skill_failed(sd) { sd->menuskill_id = sd->menuskill_val = 0; }
	if(skill_num != sd->menuskill_id)
		return 0;

	if( sd->bl.prev == NULL || pc_isdead(sd) ) {
		skill_failed(sd);
		return 0;
	}

	if( ( sd->sc.opt1 && sd->sc.opt1 != OPT1_BURNING ) || sd->sc.option&OPTION_HIDE ) {
		skill_failed(sd);
		return 0;
	}
	if(sd->sc.count && (
		sd->sc.data[SC_SILENCE] ||
		sd->sc.data[SC_ROKISWEIL] ||
		sd->sc.data[SC_AUTOCOUNTER] ||
		sd->sc.data[SC_STEELBODY] ||
		(sd->sc.data[SC_DANCING] && skill_num < RK_ENCHANTBLADE && !pc_checkskill(sd, WM_LESSON)) ||
		sd->sc.data[SC_BERSERK] ||
		sd->sc.data[SC_BASILICA] ||
		sd->sc.data[SC_MARIONETTE] ||
		sd->sc.data[SC_WHITEIMPRISON] ||
		(sd->sc.data[SC_STASIS] && skill_block_check(&sd->bl, SC_STASIS, skill_num)) ||
		(sd->sc.data[SC_KAGEHUMI] && skill_block_check(&sd->bl, SC_KAGEHUMI, skill_num)) ||
		sd->sc.data[SC_OBLIVIONCURSE] ||
		sd->sc.data[SC__MANHOLE]
	 )) {
		skill_failed(sd);
		return 0;
	}

	pc_stop_attack(sd);
	pc_stop_walking(sd,0);

	if(battle_config.skill_log && battle_config.skill_log&BL_PC)
		ShowInfo("PC %d skill castend skill =%d map=%s\n",sd->bl.id,skill_num,map);

	if(strcmp(map,"cancel")==0) {
		skill_failed(sd);
		return 0;
	}

	switch(skill_num)
	{
	case AL_TELEPORT:
		if(strcmp(map,"Random")==0)
			pc_randomwarp(sd,CLR_TELEPORT);
		else if (sd->menuskill_val > 1) //Need lv2 to be able to warp here.
			pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
		break;

	case AL_WARP:
		{
			const struct point *p[4];
			struct skill_unit_group *group;
			int i, lv, wx, wy;
			int maxcount=0;
			int x,y;
			unsigned short mapindex;

			mapindex  = mapindex_name2id((char*)map);
			if(!mapindex) { //Given map not found?
				clif_skill_fail(sd,skill_num,USESKILL_FAIL_LEVEL,0);
				skill_failed(sd);
				return 0;
			}
			p[0] = &sd->status.save_point;
			p[1] = &sd->status.memo_point[0];
			p[2] = &sd->status.memo_point[1];
			p[3] = &sd->status.memo_point[2];

			if((maxcount = skill_get_maxcount(skill_num, sd->menuskill_val)) > 0) {
				for(i=0;i<MAX_SKILLUNITGROUP && sd->ud.skillunit[i] && maxcount;i++) {
					if(sd->ud.skillunit[i]->skill_id == skill_num)
						maxcount--;
				}
				if(!maxcount) {
					clif_skill_fail(sd,skill_num,USESKILL_FAIL_LEVEL,0);
					skill_failed(sd);
					return 0;
				}
			}

			lv = sd->skillitem==skill_num?sd->skillitemlv:pc_checkskill(sd,skill_num);
			wx = sd->menuskill_val>>16;
			wy = sd->menuskill_val&0xffff;

			if( lv <= 0 ) return 0;
			if( lv > 4 ) lv = 4; // crash prevention

			// check if the chosen map exists in the memo list
			ARR_FIND( 0, lv, i, mapindex == p[i]->map );
			if( i < lv ) {
				x=p[i]->x;
				y=p[i]->y;
			} else {
				skill_failed(sd);
				return 0;
			}

			if(!skill_check_condition_castend(sd, sd->menuskill_id, lv))
			{  // This checks versus skillid/skilllv...
				skill_failed(sd);
				return 0;
			}

			skill_consume_requirement(sd,sd->menuskill_id,lv,2);
			sd->skillitem = sd->skillitemlv = 0; // Clear data that's skipped in 'skill_castend_pos' [Inkfish]

			if((group=skill_unitsetting(&sd->bl,skill_num,lv,wx,wy,0))==NULL) {
				skill_failed(sd);
				return 0;
			}
			
			group->val1 = (group->val1<<16)|(short)0;
			// record the destination coordinates
			group->val2 = (x<<16)|y;
			group->val3 = mapindex;
		}
		break;
	}

	sd->menuskill_id = sd->menuskill_val = 0;
	return 0;
#undef skill_failed
}

/// transforms 'target' skill unit into dissonance (if conditions are met)
static int skill_dance_overlap_sub(struct block_list* bl, va_list ap)
{
	struct skill_unit* target = (struct skill_unit*)bl;
	struct skill_unit* src = va_arg(ap, struct skill_unit*);
	int flag = va_arg(ap, int);

	if (src == target)
		return 0;
	if (!target->group || !(target->group->state.song_dance&0x1))
		return 0;
	if (!(target->val2 & src->val2 & ~UF_ENSEMBLE)) //They don't match (song + dance) is valid.
		return 0;

	if (flag) //Set dissonance
		target->val2 |= UF_ENSEMBLE; //Add ensemble to signal this unit is overlapping.
	else //Remove dissonance
		target->val2 &= ~UF_ENSEMBLE;

	clif_skill_setunit(target); //Update look of affected cell.

	return 1;
}

//Does the song/dance overlapping -> dissonance check. [Skotlex]
//When flag is 0, this unit is about to be removed, cancel the dissonance effect
//When 1, this unit has been positioned, so start the cancel effect.
int skill_dance_overlap(struct skill_unit* unit, int flag)
{
	if (!unit || !unit->group || !(unit->group->state.song_dance&0x1))
		return 0;
	if (!flag && !(unit->val2&UF_ENSEMBLE))
		return 0; //Nothing to remove, this unit is not overlapped.

	if (unit->val1 != unit->group->skill_id)
	{	//Reset state
		unit->val1 = unit->group->skill_id;
		unit->val2 &= ~UF_ENSEMBLE;
	}

	return map_foreachincell(skill_dance_overlap_sub, unit->bl.m,unit->bl.x,unit->bl.y,BL_SKILL, unit,flag);
}

/*==========================================
 * Converts this group information so that it is handled as a Dissonance or Ugly Dance cell.
 * Flag: 0 - Convert, 1 - Revert.
 *------------------------------------------*/
static bool skill_dance_switch(struct skill_unit* unit, int flag)
{
	static int prevflag = 1;  // by default the backup is empty
	static struct skill_unit_group backup;
	struct skill_unit_group* group = unit->group;

	// val2&UF_ENSEMBLE is a hack to indicate dissonance
	if ( !(group->state.song_dance&0x1 && unit->val2&UF_ENSEMBLE) )
		return false;

	if( flag == prevflag )
	{// protection against attempts to read an empty backup / write to a full backup
		ShowError("skill_dance_switch: Attempted to %s (skill_id=%d, skill_lv=%d, src_id=%d).\n",
			flag ? "read an empty backup" : "write to a full backup",
			group->skill_id, group->skill_lv, group->src_id);
		return false;
	}
	prevflag = flag;

	if( !flag )
	{	//Transform
		int skillid = unit->val2&UF_SONG ? BA_DISSONANCE : DC_UGLYDANCE;

		// backup
		backup.skill_id    = group->skill_id;
		backup.skill_lv    = group->skill_lv;
		backup.unit_id     = group->unit_id;
		backup.target_flag = group->target_flag;
		backup.bl_flag     = group->bl_flag;
		backup.interval    = group->interval;

		// replace
		group->skill_id    = skillid;
		group->skill_lv    = 1;
		group->unit_id     = skill_get_unit_id(skillid,0);
		group->target_flag = skill_get_unit_target(skillid);
		group->bl_flag     = skill_get_unit_bl_target(skillid);
		group->interval    = skill_get_unit_interval(skillid);
	}
	else
	{	//Restore
		group->skill_id    = backup.skill_id;
		group->skill_lv    = backup.skill_lv;
		group->unit_id     = backup.unit_id;
		group->target_flag = backup.target_flag;
		group->bl_flag     = backup.bl_flag;
		group->interval    = backup.interval;
	}

	return true;
}
/**
 * Upon Ice Wall cast it checks all nearby mobs to find any who may be blocked by the IW
 **/
static int skill_icewall_block(struct block_list *bl,va_list ap) {
	struct block_list *target = NULL;
	struct mob_data *md = ((TBL_MOB*)bl);

	nullpo_ret(bl);
	nullpo_ret(md);
	if( !md->target_id || ( target = map_id2bl(md->target_id) ) == NULL )
		return 0;
	
	if( path_search_long(NULL,bl->m,bl->x,bl->y,target->x,target->y,CELL_CHKICEWALL) )
		return 0;

	if( !check_distance_bl(bl, target, status_get_range(bl) ) ) {
		mob_unlocktarget(md,gettick());
		mob_stop_walking(md,1);
	}

	return 0;
}
/*==========================================
 * Initializes and sets a ground skill.
 * flag&1 is used to determine when the skill 'morphs' (Warp portal becomes active, or Fire Pillar becomes active)
 *------------------------------------------*/
struct skill_unit_group* skill_unitsetting (struct block_list *src, short skillid, short skilllv, short x, short y, int flag)
{
	struct skill_unit_group *group;
	int i,limit,val1=0,val2=0,val3=0;
	int target,interval,range,unit_flag,req_item=0;
	struct s_skill_unit_layout *layout;
	struct map_session_data *sd;
	struct status_data *status;
	struct status_change *sc;
	int active_flag=1;
	int subunt=0;

	nullpo_retr(NULL, src);

	limit = skill_get_time(skillid,skilllv);
	range = skill_get_unit_range(skillid,skilllv);
	interval = skill_get_unit_interval(skillid);
	target = skill_get_unit_target(skillid);
	unit_flag = skill_get_unit_flag(skillid);
	layout = skill_get_unit_layout(skillid,skilllv,src,x,y);

	sd = BL_CAST(BL_PC, src);
	status = status_get_status_data(src);
	sc = status_get_sc(src);	// for traps, firewall and fogwall - celest

	switch( skillid ) {
	case MG_SAFETYWALL:
	#ifdef RENEWAL
		/**
		 * According to data provided in RE, SW life is equal to 3 times caster's health
		 **/
		val2 = status_get_max_hp(src) * 3;
	#else
		val2 = skilllv+1;
	#endif
		break;
	case MG_FIREWALL:
		if(sc && sc->data[SC_VIOLENTGALE])
			limit = limit*3/2;
		val2=4+skilllv;
		break;

	case AL_WARP:
		val1=skilllv+6;
		if(!(flag&1))
			limit=2000;
		else // previous implementation (not used anymore)
		{	//Warp Portal morphing to active mode, extract relevant data from src. [Skotlex]
			if( src->type != BL_SKILL ) return NULL;
			group = ((TBL_SKILL*)src)->group;
			src = map_id2bl(group->src_id);
			if( !src ) return NULL;
			val2 = group->val2; //Copy the (x,y) position you warp to
			val3 = group->val3; //as well as the mapindex to warp to.
		}
		break;
	case HP_BASILICA:
		val1 = src->id; // Store caster id.
		break;

	case PR_SANCTUARY:
	case NPC_EVILLAND:
		val1=(skilllv+3)*2;
		break;

	case WZ_FIREPILLAR:
		if((flag&1)!=0)
			limit=1000;
		val1=skilllv+2;
		break;
	case WZ_QUAGMIRE:	//The target changes to "all" if used in a gvg map. [Skotlex]
	case AM_DEMONSTRATION:
	case GN_HELLS_PLANT:
		if (map_flag_vs(src->m) && battle_config.vs_traps_bctall
			&& (src->type&battle_config.vs_traps_bctall))
			target = BCT_ALL;
		break;
	case HT_SHOCKWAVE:
		val1=skilllv*15+10;
	case HT_SANDMAN:
	case MA_SANDMAN:
	case HT_CLAYMORETRAP:
	case HT_SKIDTRAP:
	case MA_SKIDTRAP:
	case HT_LANDMINE:
	case MA_LANDMINE:
	case HT_ANKLESNARE:
	case HT_FLASHER:
	case HT_FREEZINGTRAP:
	case MA_FREEZINGTRAP:
	case HT_BLASTMINE:
	/**
	 * Ranger
	 **/
	case RA_ELECTRICSHOCKER:
	case RA_CLUSTERBOMB:
	case RA_MAGENTATRAP:
	case RA_COBALTTRAP:
	case RA_MAIZETRAP:
	case RA_VERDURETRAP:
	case RA_FIRINGTRAP:
	case RA_ICEBOUNDTRAP:
		{
			struct skill_condition req = skill_get_requirement(sd,skillid,skilllv);
			ARR_FIND(0, MAX_SKILL_ITEM_REQUIRE, i, req.itemid[i] && (req.itemid[i] == ITEMID_TRAP || req.itemid[i] == ITEMID_TRAP_ALLOY));
			if( req.itemid[i] )
				req_item = req.itemid[i];
			if( map_flag_gvg(src->m) || map[src->m].flag.battleground )
				limit *= 4; // longer trap times in WOE [celest]
			if( battle_config.vs_traps_bctall && map_flag_vs(src->m) && (src->type&battle_config.vs_traps_bctall) )
				target = BCT_ALL;
		}
		break;

	case SA_LANDPROTECTOR:
	case SA_VOLCANO:
	case SA_DELUGE:
	case SA_VIOLENTGALE:
	{
		struct skill_unit_group *old_sg;
		if ((old_sg = skill_locate_element_field(src)) != NULL)
		{	//HelloKitty confirmed that these are interchangeable,
			//so you can change element and not consume gemstones.
			if ((
				old_sg->skill_id == SA_VOLCANO ||
				old_sg->skill_id == SA_DELUGE ||
				old_sg->skill_id == SA_VIOLENTGALE
			) && old_sg->limit > 0)
			{	//Use the previous limit (minus the elapsed time) [Skotlex]
				limit = old_sg->limit - DIFF_TICK(gettick(), old_sg->tick);
				if (limit < 0)	//This can happen...
					limit = skill_get_time(skillid,skilllv);
			}
			skill_clear_group(src,1);
		}
		break;
	}

	case BA_DISSONANCE:
	case DC_UGLYDANCE:
		val1 = 10;	//FIXME: This value is not used anywhere, what is it for? [Skotlex]
		break;
	case BA_WHISTLE:
		val1 = skilllv +status->agi/10; // Flee increase
		val2 = ((skilllv+1)/2)+status->luk/10; // Perfect dodge increase
		if(sd){
			val1 += pc_checkskill(sd,BA_MUSICALLESSON);
			val2 += pc_checkskill(sd,BA_MUSICALLESSON);
		}
		break;
	case DC_HUMMING:
		val1 = 2*skilllv+status->dex/10; // Hit increase
		#ifdef RENEWAL
			val1 *= 2;
		#endif
		if(sd)
			val1 += pc_checkskill(sd,DC_DANCINGLESSON);
		break;
	case BA_POEMBRAGI:
		val1 = 3*skilllv+status->dex/10; // Casting time reduction
		//For some reason at level 10 the base delay reduction is 50%.
		val2 = (skilllv<10?3*skilllv:50)+status->int_/5; // After-cast delay reduction
		if(sd){
			val1 += 2*pc_checkskill(sd,BA_MUSICALLESSON);
			val2 += 2*pc_checkskill(sd,BA_MUSICALLESSON);
		}
		break;
	case DC_DONTFORGETME:
		val1 = status->dex/10 + 3*skilllv + 5; // ASPD decrease
		val2 = status->agi/10 + 3*skilllv + 5; // Movement speed adjustment.
		if(sd){
			val1 += pc_checkskill(sd,DC_DANCINGLESSON);
			val2 += pc_checkskill(sd,DC_DANCINGLESSON);
		}
		break;
	case BA_APPLEIDUN:
		val1 = 5+2*skilllv+status->vit/10; // MaxHP percent increase
		if(sd)
			val1 += pc_checkskill(sd,BA_MUSICALLESSON);
		break;
	case DC_SERVICEFORYOU:
		val1 = 15+skilllv+(status->int_/10); // MaxSP percent increase TO-DO: this INT bonus value is guessed
		val2 = 20+3*skilllv+(status->int_/10); // SP cost reduction
		if(sd){
			val1 += pc_checkskill(sd,DC_DANCINGLESSON); //TO-DO This bonus value is guessed
			val2 += pc_checkskill(sd,DC_DANCINGLESSON); //TO-DO Should be half this value
		}
		break;
	case BA_ASSASSINCROSS:
		val1 = 100+(10*skilllv)+(status->agi/10); // ASPD increase
		if(sd)
			val1 += 5*pc_checkskill(sd,BA_MUSICALLESSON);
		break;
	case DC_FORTUNEKISS:
		val1 = 10+skilllv+(status->luk/10); // Critical increase
		if(sd)
			val1 += pc_checkskill(sd,DC_DANCINGLESSON);
		val1*=10; //Because every 10 crit is an actual cri point.
		break;
	case BD_DRUMBATTLEFIELD:
	#ifdef RENEWAL
		val1 = (skilllv+5)*25;	//Watk increase
		val2 = skilllv*10;		//Def increase
	#else
		val1 = (skilllv+1)*25;	//Watk increase
		val2 = (skilllv+1)*2;	//Def increase
	#endif
		break;
	case BD_RINGNIBELUNGEN:
		val1 = (skilllv+2)*25;	//Watk increase
		break;
	case BD_RICHMANKIM:
		val1 = 25 + 11*skilllv; //Exp increase bonus.
		break;
	case BD_SIEGFRIED:
		val1 = 55 + skilllv*5;	//Elemental Resistance
		val2 = skilllv*10;	//Status ailment resistance
		break;
	case WE_CALLPARTNER:
		if (sd) val1 = sd->status.partner_id;
		break;
	case WE_CALLPARENT:
		if (sd) {
			val1 = sd->status.father;
		 	val2 = sd->status.mother;
		}
		break;
	case WE_CALLBABY:
		if (sd) val1 = sd->status.child;
		break;
	case NJ_KAENSIN:
		skill_clear_group(src, 1); //Delete previous Kaensins/Suitons
		val2 = (skilllv+1)/2 + 4;
		break;
	case NJ_SUITON:
		skill_clear_group(src, 1);
		break;

	case GS_GROUNDDRIFT:
		{
		int element[5]={ELE_WIND,ELE_DARK,ELE_POISON,ELE_WATER,ELE_FIRE};

		val1 = status->rhw.ele;
		if (!val1)
			val1=element[rnd()%5];

		switch (val1)
		{
			case ELE_FIRE:
				subunt++;
			case ELE_WATER:
				subunt++;
			case ELE_POISON:
				subunt++;
			case ELE_DARK:
				subunt++;
			case ELE_WIND:
				break;
			default:
				subunt=rnd()%5;
				break;
		}

		break;
		}
	case GC_POISONSMOKE:
		if( !(sc && sc->data[SC_POISONINGWEAPON]) )
			return NULL;
		val2 = sc->data[SC_POISONINGWEAPON]->val2; // Type of Poison
		val3 = sc->data[SC_POISONINGWEAPON]->val1;
		limit = 4000 + 2000 * skilllv;
		break;
	case GD_LEADERSHIP:
	case GD_GLORYWOUNDS:
	case GD_SOULCOLD:
	case GD_HAWKEYES:
		limit = 1000000;//it doesn't matter
		break;
	case LG_BANDING:
		limit = -1;
		break;
	case WM_REVERBERATION:
		interval = limit;
		val2 = 1;
	case WM_POEMOFNETHERWORLD:	// Can't be placed on top of Land Protector.
		if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) )
			return NULL;
		break;
	case SO_CLOUD_KILL:
		skill_clear_group(src, 4);
		break;
	case SO_WARMER:
		skill_clear_group(src, 8);
		break;
	case SO_VACUUM_EXTREME:
		range++;
		break;
	case GN_WALLOFTHORN:
		if( flag&1 )
			limit = 3000;
		val3 = (x<<16)|y;
		break;	
	case KO_ZENKAI:
		if( sd ){
			ARR_FIND(1, 6, i, sd->talisman[i] > 0);
			if( i < 5 ){
				val1 = sd->talisman[i]; // no. of aura
				val2 = i; // aura type
				limit += val1 * 1000;
				subunt = i - 1;
				pc_del_talisman(sd, sd->talisman[i], i);
			}
		}
		break;
	}

	nullpo_retr(NULL, group=skill_initunitgroup(src,layout->count,skillid,skilllv,skill_get_unit_id(skillid,flag&1)+subunt, limit, interval));
	group->val1=val1;
	group->val2=val2;
	group->val3=val3;
	group->target_flag=target;
	group->bl_flag= skill_get_unit_bl_target(skillid);
	group->state.ammo_consume = (sd && sd->state.arrow_atk && skillid != GS_GROUNDDRIFT); //Store if this skill needs to consume ammo.
	group->state.song_dance = (unit_flag&(UF_DANCE|UF_SONG)?1:0)|(unit_flag&UF_ENSEMBLE?2:0); //Signals if this is a song/dance/duet
	group->state.guildaura = ( skillid >= GD_LEADERSHIP && skillid <= GD_HAWKEYES )?1:0;
	group->item_id = req_item;
  	//if tick is greater than current, do not invoke onplace function just yet. [Skotlex]
	if (DIFF_TICK(group->tick, gettick()) > SKILLUNITTIMER_INTERVAL)
		active_flag = 0;

	if(skillid==HT_TALKIEBOX || skillid==RG_GRAFFITI){
		group->valstr=(char *) aMalloc(MESSAGE_SIZE*sizeof(char));
		if (sd)
			safestrncpy(group->valstr, sd->message, MESSAGE_SIZE);
		else //Eh... we have to write something here... even though mobs shouldn't use this. [Skotlex]
			safestrncpy(group->valstr, "Boo!", MESSAGE_SIZE);
	}

	if (group->state.song_dance) {
		if(sd){
			sd->skillid_dance = skillid;
			sd->skilllv_dance = skilllv;
		}
		if (
			sc_start4(src, SC_DANCING, 100, skillid, group->group_id, skilllv,
				(group->state.song_dance&2?BCT_SELF:0), limit+1000) &&
			sd && group->state.song_dance&2 && skillid != CG_HERMODE //Hermod is a encore with a warp!
		)
			skill_check_pc_partner(sd, skillid, &skilllv, 1, 1);
	}

	limit = group->limit;
	for( i = 0; i < layout->count; i++ )
	{
		struct skill_unit *unit;
		int ux = x + layout->dx[i];
		int uy = y + layout->dy[i];
		int val1 = skilllv;
		int val2 = 0;
		int alive = 1;

		if( !group->state.song_dance && !map_getcell(src->m,ux,uy,CELL_CHKREACH) )
			continue; // don't place skill units on walls (except for songs/dances/encores)
		if( battle_config.skill_wall_check && skill_get_unit_flag(skillid)&UF_PATHCHECK && !path_search_long(NULL,src->m,ux,uy,x,y,CELL_CHKWALL) )
			continue; // no path between cell and center of casting.

		switch( skillid )
		{
		case MG_FIREWALL:
		case NJ_KAENSIN:
			val2=group->val2;
			break;
		case WZ_ICEWALL:
			val1 = (skilllv <= 1) ? 500 : 200 + 200*skilllv;
			val2 = map_getcell(src->m, ux, uy, CELL_GETTYPE);
			break;
		case HT_LANDMINE:
		case MA_LANDMINE:
		case HT_ANKLESNARE:
		case HT_SHOCKWAVE:
		case HT_SANDMAN:
		case MA_SANDMAN:
		case HT_FLASHER:
		case HT_FREEZINGTRAP:
		case MA_FREEZINGTRAP:
		case HT_TALKIEBOX:
		case HT_SKIDTRAP:
		case MA_SKIDTRAP:
		case HT_CLAYMORETRAP:
		case HT_BLASTMINE:
		/**
		 * Ranger
		 **/
		case RA_ELECTRICSHOCKER:
		case RA_CLUSTERBOMB:
		case RA_MAGENTATRAP:
		case RA_COBALTTRAP:
		case RA_MAIZETRAP:
		case RA_VERDURETRAP:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			val1 = 3500;
			break;
		case GS_DESPERADO:
			val1 = abs(layout->dx[i]);
			val2 = abs(layout->dy[i]);
			if (val1 < 2 || val2 < 2) { //Nearby cross, linear decrease with no diagonals
				if (val2 > val1) val1 = val2;
				if (val1) val1--;
				val1 = 36 -12*val1;
			} else //Diagonal edges
				val1 = 28 -4*val1 -4*val2;
			if (val1 < 1) val1 = 1;
			val2 = 0;
			break;
		case WM_REVERBERATION:
			val1 = 1 + skilllv;
			break;
		case GN_WALLOFTHORN:
			val1 = 1000 * skilllv;	// Need official value. [LimitLine]
			break;				
		default:
			if (group->state.song_dance&0x1)
				val2 = unit_flag&(UF_DANCE|UF_SONG); //Store whether this is a song/dance
			break;
		}

		if( range <= 0 )
			map_foreachincell(skill_cell_overlap,src->m,ux,uy,BL_SKILL,skillid, &alive, src);
		if( !alive )
			continue;

		nullpo_retr(NULL, unit=skill_initunit(group,i,ux,uy,val1,val2));
		unit->limit=limit;
		unit->range=range;

		if (skillid == PF_FOGWALL && alive == 2)
		{	//Double duration of cells on top of Deluge/Suiton
			unit->limit *= 2;
			group->limit = unit->limit;
		}

		// execute on all targets standing on this cell
		if (range==0 && active_flag)
			map_foreachincell(skill_unit_effect,unit->bl.m,unit->bl.x,unit->bl.y,group->bl_flag,&unit->bl,gettick(),1);
	}

	if (!group->alive_count)
	{	//No cells? Something that was blocked completely by Land Protector?
		skill_delunitgroup(group);
		return NULL;
	}

	//success, unit created.
	switch( skillid ) {
		case WZ_ICEWALL:
			map_foreachinrange(skill_icewall_block, src, AREA_SIZE, BL_MOB);
			break;
		case NJ_TATAMIGAESHI: //Store number of tiles.
			group->val1 = group->alive_count;
			break;
	}

	return group;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_unit_onplace (struct skill_unit *src, struct block_list *bl, unsigned int tick)
{
	struct skill_unit_group *sg;
	struct block_list *ss;
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;
	int skillid;

	nullpo_ret(src);
	nullpo_ret(bl);

	if(bl->prev==NULL || !src->alive || status_isdead(bl))
		return 0;

	nullpo_ret(sg=src->group);
	nullpo_ret(ss=map_id2bl(sg->src_id));

	if( skill_get_type(sg->skill_id) == BF_MAGIC && map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR) && sg->skill_id != SA_LANDPROTECTOR )
		return 0; //AoE skills are ineffective. [Skotlex]

	sc = status_get_sc(bl);

	if (sc && sc->option&OPTION_HIDE && sg->skill_id != WZ_HEAVENDRIVE && sg->skill_id != WL_EARTHSTRAIN )
		return 0; //Hidden characters are immune to AoE skills except to these. [Skotlex]

	type = status_skill2sc(sg->skill_id);
	sce = (sc && type != -1)?sc->data[type]:NULL;
	skillid = sg->skill_id; //In case the group is deleted, we need to return the correct skill id, still.
	switch (sg->unit_id)
	{
	case UNT_SPIDERWEB:
		if( sc && sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1 > 0 )
		{ // If you are fiberlocked and can't move, it will only increase your fireweakness level. [Inkfish]
			sc->data[SC_SPIDERWEB]->val2++;
			break;
		}
		else if( sc )
		{
			int sec = skill_get_time2(sg->skill_id,sg->skill_lv);
			if( status_change_start(bl,type,10000,sg->skill_lv,1,sg->group_id,0,sec,8) )
			{
				const struct TimerData* td = sc->data[type]?get_timer(sc->data[type]->timer):NULL; 
				if( td )
					sec = DIFF_TICK(td->tick, tick);
				map_moveblock(bl, src->bl.x, src->bl.y, tick);
				clif_fixpos(bl);
				sg->val2 = bl->id;
			}
			else
				sec = 3000; //Couldn't trap it?
			sg->limit = DIFF_TICK(tick,sg->tick)+sec;
		}
		break;
	case UNT_SAFETYWALL:
		if (!sce)
			sc_start4(bl,type,100,sg->skill_lv,sg->group_id,sg->group_id,0,sg->limit);
		break;

	case UNT_PNEUMA:
	case UNT_CHAOSPANIC:
	case UNT_MAELSTROM:
		if (!sce)
			sc_start4(bl,type,100,sg->skill_lv,sg->group_id,0,0,sg->limit);
		break;

	case UNT_WARP_WAITING: {
		int working = sg->val1&0xffff;

		if(bl->type==BL_PC && !working){
			struct map_session_data *sd = (struct map_session_data *)bl;
			if((!sd->chatID || battle_config.chat_warpportal)
				&& sd->ud.to_x == src->bl.x && sd->ud.to_y == src->bl.y)
			{
				int x = sg->val2>>16;
				int y = sg->val2&0xffff;
				int count = sg->val1>>16;
				unsigned short m = sg->val3;

				if( --count <= 0 )
					skill_delunitgroup(sg);
				
				if ( map_mapindex2mapid(sg->val3) == sd->bl.m && x == sd->bl.x && y == sd->bl.y )
					working = 1;/* we break it because officials break it, lovely stuff. */

				sg->val1 = (count<<16)|working;
				
				pc_setpos(sd,m,x,y,CLR_TELEPORT);
			}
		} else if(bl->type == BL_MOB && battle_config.mob_warp&2) {
			int m = map_mapindex2mapid(sg->val3);
			if (m < 0) break; //Map not available on this map-server.
			unit_warp(bl,m,sg->val2>>16,sg->val2&0xffff,CLR_TELEPORT);
		}
	}
		break;

	case UNT_QUAGMIRE:
		if( !sce && battle_check_target(&sg->unit->bl,bl,sg->target_flag) > 0 )
			sc_start4(bl,type,100,sg->skill_lv,sg->group_id,0,0,sg->limit);
		break;

	case UNT_VOLCANO:
	case UNT_DELUGE:
	case UNT_VIOLENTGALE:
		if(!sce)
			sc_start(bl,type,100,sg->skill_lv,sg->limit);
		break;

	case UNT_SUITON:
		if(!sce)
			sc_start4(bl,type,100,sg->skill_lv,
			map_flag_vs(bl->m) || battle_check_target(&src->bl,bl,BCT_ENEMY)>0?1:0, //Send val3 =1 to reduce agi.
			0,0,sg->limit);
		break;

	case UNT_HERMODE:
		if (sg->src_id!=bl->id && battle_check_target(&src->bl,bl,BCT_PARTY|BCT_GUILD) > 0)
			status_change_clear_buffs(bl,1); //Should dispell only allies.
	case UNT_RICHMANKIM:
	case UNT_ETERNALCHAOS:
	case UNT_DRUMBATTLEFIELD:
	case UNT_RINGNIBELUNGEN:
	case UNT_ROKISWEIL:
	case UNT_INTOABYSS:
	case UNT_SIEGFRIED:
		 //Needed to check when a dancer/bard leaves their ensemble area.
		if (sg->src_id==bl->id && !(sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_BARDDANCER))
			return skillid;
		if (!sce)
			sc_start4(bl,type,100,sg->skill_lv,sg->val1,sg->val2,0,sg->limit);
		break;
	case UNT_WHISTLE:
	case UNT_ASSASSINCROSS:
	case UNT_POEMBRAGI:
	case UNT_APPLEIDUN:
	case UNT_HUMMING:
	case UNT_DONTFORGETME:
	case UNT_FORTUNEKISS:
	case UNT_SERVICEFORYOU:
		if (sg->src_id==bl->id && !(sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_BARDDANCER)) {
			if( sce )/* We have the status but we're not elegible for it, so we take it away. (bugreport:4591) */
				sce->val4 = 2;
			return 0;
		}
		if (!sc) return 0;
		if (!sce)
			sc_start4(bl,type,100,sg->skill_lv,sg->val1,sg->val2,0,sg->limit);
		else if (sce->val4 == 1) {
			//Readjust timers since the effect will not last long.
			sce->val4 = 0;
			delete_timer(sce->timer, status_change_timer);
			sce->timer = add_timer(tick+sg->limit, status_change_timer, bl->id, type);
		}
		break;

	case UNT_FOGWALL:
		if (!sce)
		{
			sc_start4(bl, type, 100, sg->skill_lv, sg->val1, sg->val2, sg->group_id, sg->limit);
			if (battle_check_target(&src->bl,bl,BCT_ENEMY)>0)
				skill_additional_effect (ss, bl, sg->skill_id, sg->skill_lv, BF_MISC, ATK_DEF, tick);
		}
		break;

	case UNT_GRAVITATION:
		if (!sce)
			sc_start4(bl,type,100,sg->skill_lv,0,BCT_ENEMY,sg->group_id,sg->limit);
		break;

// officially, icewall has no problems existing on occupied cells [ultramage]
//	case UNT_ICEWALL: //Destroy the cell. [Skotlex]
//		src->val1 = 0;
//		if(src->limit + sg->tick > tick + 700)
//			src->limit = DIFF_TICK(tick+700,sg->tick);
//		break;

	case UNT_MOONLIT:
		//Knockback out of area if affected char isn't in Moonlit effect
		if (sc && sc->data[SC_DANCING] && (sc->data[SC_DANCING]->val1&0xFFFF) == CG_MOONLIT)
			break;
		if (ss == bl) //Also needed to prevent infinite loop crash.
			break;
		skill_blown(ss,bl,skill_get_blewcount(sg->skill_id,sg->skill_lv),unit_getdir(bl),0);
		break;

	case UNT_WALLOFTHORN:
		if( status_get_mode(bl)&MD_BOSS )
			break;	// iRO Wiki says that this skill don't affect to Boss monsters.
		if( map_flag_vs(bl->m) || bl->id == src->bl.id || battle_check_target(&src->bl,bl, BCT_ENEMY) == 1 )
			skill_attack(skill_get_type(sg->skill_id), ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
		break;
			
	case UNT_GD_LEADERSHIP:
	case UNT_GD_GLORYWOUNDS:
	case UNT_GD_SOULCOLD:
	case UNT_GD_HAWKEYES:
		if ( !sce )
			sc_start4(bl,type,100,sg->skill_lv,0,0,0,1000);
		break;
	}
	return skillid;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_onplace_timer (struct skill_unit *src, struct block_list *bl, unsigned int tick)
{
	struct skill_unit_group *sg;
	struct block_list *ss;
	TBL_PC* tsd;
	struct status_data *tstatus;
	struct status_change *tsc;
	struct skill_unit_group_tickset *ts;
	enum sc_type type;
	int skillid;
	int diff=0;

	nullpo_ret(src);
	nullpo_ret(bl);

	if (bl->prev==NULL || !src->alive || status_isdead(bl))
		return 0;

	nullpo_ret(sg=src->group);
	nullpo_ret(ss=map_id2bl(sg->src_id));
	tsd = BL_CAST(BL_PC, bl);
	tsc = status_get_sc(bl);
	
	if ( tsc && tsc->data[SC_HOVERING] )
		return 0; //Under hovering characters are immune to trap and ground target skills.

	tstatus = status_get_status_data(bl);
	type = status_skill2sc(sg->skill_id);
	skillid = sg->skill_id;

	if (sg->interval == -1) {
		switch (sg->unit_id) {
			case UNT_ANKLESNARE: //These happen when a trap is splash-triggered by multiple targets on the same cell.
			case UNT_FIREPILLAR_ACTIVE:
			case UNT_ELECTRICSHOCKER:
			case UNT_MANHOLE:
				return 0;
			default:
				ShowError("skill_unit_onplace_timer: interval error (unit id %x)\n", sg->unit_id);
				return 0;
		}
	}

	if ((ts = skill_unitgrouptickset_search(bl,sg,tick)))
	{	//Not all have it, eg: Traps don't have it even though they can be hit by Heaven's Drive [Skotlex]
		diff = DIFF_TICK(tick,ts->tick);
		if (diff < 0)
			return 0;
		ts->tick = tick+sg->interval;

		if ((skillid==CR_GRANDCROSS || skillid==NPC_GRANDDARKNESS) && !battle_config.gx_allhit)
			ts->tick += sg->interval*(map_count_oncell(bl->m,bl->x,bl->y,BL_CHAR)-1);
	}

	switch (sg->unit_id)
	{
		case UNT_FIREWALL:
		case UNT_KAEN:
		{
			int count=0;
			const int x = bl->x, y = bl->y;
			
			if( sg->skill_id == GN_WALLOFTHORN && !map_flag_vs(bl->m) )
				break;
			
			//Take into account these hit more times than the timer interval can handle.
			do
				skill_attack(BF_MAGIC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick+count*sg->interval,0);
			while(--src->val2 && x == bl->x && y == bl->y &&
				++count < SKILLUNITTIMER_INTERVAL/sg->interval && !status_isdead(bl));

			if (src->val2<=0)
				skill_delunit(src);
		}
		break;

		case UNT_SANCTUARY:
			if( battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race==RC_DEMON )
			{ //Only damage enemies with offensive Sanctuary. [Skotlex]
				if( battle_check_target(&src->bl,bl,BCT_ENEMY) > 0 && skill_attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0) )
					sg->val1 -= 2; // reduce healing count if this was meant for damaging [hekate]
			}
			else
			{
				int heal = skill_calc_heal(ss,bl,sg->skill_id,sg->skill_lv,true);
				struct mob_data *md = BL_CAST(BL_MOB, bl);
#ifdef RENEWAL
				if( md && md->class_ == MOBID_EMPERIUM )
					break;
#endif
				if( md && mob_is_battleground(md) )
					break;
				if( tstatus->hp >= tstatus->max_hp )
					break;
				if( status_isimmune(bl) )
					heal = 0;
				clif_skill_nodamage(&src->bl, bl, AL_HEAL, heal, 1);
				if( tsc && tsc->data[SC_AKAITSUKI] && heal )
					heal = ~heal + 1;
				status_heal(bl, heal, 0, 0);
				if( diff >= 500 )
					sg->val1--;
			}
			if( sg->val1 <= 0 )
				skill_delunitgroup(sg);
			break;

		case UNT_EVILLAND:
			//Will heal demon and undead element monsters, but not players.
			if ((bl->type == BL_PC) || (!battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race!=RC_DEMON))
			{	//Damage enemies
				if(battle_check_target(&src->bl,bl,BCT_ENEMY)>0)
					skill_attack(BF_MISC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			} else {
				int heal = skill_calc_heal(ss,bl,sg->skill_id,sg->skill_lv,true);
				if (tstatus->hp >= tstatus->max_hp)
					break;
				if (status_isimmune(bl))
					heal = 0;
				clif_skill_nodamage(&src->bl, bl, AL_HEAL, heal, 1);
				status_heal(bl, heal, 0, 0);
			}
			break;

		case UNT_MAGNUS:
			if (!battle_check_undead(tstatus->race,tstatus->def_ele) && tstatus->race!=RC_DEMON)
				break;
			skill_attack(BF_MAGIC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_DUMMYSKILL:
			switch (sg->skill_id)
			{
				case SG_SUN_WARM: //SG skills [Komurka]
				case SG_MOON_WARM:
				case SG_STAR_WARM:
				{
					int count = 0;
					const int x = bl->x, y = bl->y;

					//If target isn't knocked back it should hit every "interval" ms [Playtester]
					do
					{
						if( bl->type == BL_PC )
							status_zap(bl, 0, 15); // sp damage to players
						else // mobs
						if( status_charge(ss, 0, 2) ) // costs 2 SP per hit
						{
							if( !skill_attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick+count*sg->interval,0) )
								status_charge(ss, 0, 8); //costs additional 8 SP if miss
						}
						else
						{ //should end when out of sp.
							sg->limit = DIFF_TICK(tick,sg->tick);
							break;
						}
					} while( x == bl->x && y == bl->y &&
						++count < SKILLUNITTIMER_INTERVAL/sg->interval && !status_isdead(bl) );
				}
				break;
		/**
		 * The storm gust counter was dropped in renewal
		 **/
		#ifndef RENEWAL
				case WZ_STORMGUST: //SG counter does not reset per stormgust. IE: One hit from a SG and two hits from another will freeze you.
					if (tsc) 
						tsc->sg_counter++; //SG hit counter.
					if (skill_attack(skill_get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0) <= 0 && tsc)
						tsc->sg_counter=0; //Attack absorbed.
				break;
		#endif
				case GS_DESPERADO:
					if (rnd()%100 < src->val1)
						skill_attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
				break;
				case GN_CRAZYWEED_ATK:
					if( bl->type == BL_SKILL ){
						struct skill_unit *su = (struct skill_unit *)bl;
						if( su && !(skill_get_inf2(su->group->skill_id)&INF2_TRAP) )
							break;
					} 
				default:
					skill_attack(skill_get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			}
			break;

		case UNT_FIREPILLAR_WAITING:
			skill_unitsetting(ss,sg->skill_id,sg->skill_lv,src->bl.x,src->bl.y,1);
			skill_delunit(src);
			break;

		case UNT_SKIDTRAP:
			{
				skill_blown(&src->bl,bl,skill_get_blewcount(sg->skill_id,sg->skill_lv),unit_getdir(bl),0);
				sg->unit_id = UNT_USED_TRAPS;
				clif_changetraplook(&src->bl, UNT_USED_TRAPS);
				sg->limit=DIFF_TICK(tick,sg->tick)+1500;
			}
			break;

		case UNT_ANKLESNARE:
		case UNT_MANHOLE:
			if( sg->val2 == 0 && tsc && (sg->unit_id == UNT_ANKLESNARE || bl->id != sg->src_id) ) {
				int sec = skill_get_time2(sg->skill_id,sg->skill_lv);
				if( status_change_start(bl,type,10000,sg->skill_lv,sg->group_id,0,0,sec, 8) ) {
					const struct TimerData* td = tsc->data[type]?get_timer(tsc->data[type]->timer):NULL; 
					if( td )
						sec = DIFF_TICK(td->tick, tick);
					unit_movepos(bl, src->bl.x, src->bl.y, 0, 0);
					clif_fixpos(bl);
					sg->val2 = bl->id;
				} else
					sec = 3000; //Couldn't trap it?
				if( sg->unit_id == UNT_ANKLESNARE ) {
					clif_skillunit_update(&src->bl);
					/**
					 * If you're snared from a trap that was invisible this makes the trap be
					 * visible again -- being you stepped on it (w/o this the trap remains invisible and you go "WTF WHY I CANT MOVE")
					 * bugreport:3961
					 **/
					clif_changetraplook(&src->bl, UNT_ANKLESNARE);
				}
				sg->limit = DIFF_TICK(tick,sg->tick)+sec;
				sg->interval = -1;
				src->range = 0;
			}
			break;

		case UNT_ELECTRICSHOCKER:
			if( bl->id != ss->id ) {
				if( status_get_mode(bl)&MD_BOSS )
					break;
				if( status_change_start(bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill_get_time2(sg->skill_id, sg->skill_lv), 8) ) {

					map_moveblock(bl, src->bl.x, src->bl.y, tick);
					clif_fixpos(bl);
				
				}
				
				map_foreachinrange(skill_trap_splash, &src->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &src->bl, tick);
				sg->unit_id = UNT_USED_TRAPS; //Changed ID so it does not invoke a for each in area again.
			}			
			break;
			
		case UNT_VENOMDUST:
			if(tsc && !tsc->data[type])
				status_change_start(bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill_get_time2(sg->skill_id,sg->skill_lv),0);
			break;

			
		case UNT_MAGENTATRAP:
		case UNT_COBALTTRAP:
		case UNT_MAIZETRAP:
		case UNT_VERDURETRAP:
			if( bl->type == BL_PC )// it won't work on players
				break;
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
		case UNT_CLUSTERBOMB:
			if( bl->id == ss->id )// it won't trigger on caster
				break;
		case UNT_LANDMINE:
		case UNT_CLAYMORETRAP:
		case UNT_BLASTMINE:
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
		case UNT_FREEZINGTRAP:
		case UNT_FIREPILLAR_ACTIVE:
		case UNT_MAKIBISHI:
			map_foreachinrange(skill_trap_splash,&src->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &src->bl,tick);
			if (sg->unit_id != UNT_FIREPILLAR_ACTIVE)
				clif_changetraplook(&src->bl, sg->unit_id==UNT_LANDMINE?UNT_FIREPILLAR_ACTIVE:UNT_USED_TRAPS);
			sg->limit=DIFF_TICK(tick,sg->tick)+1500 +
				(sg->unit_id== UNT_CLUSTERBOMB || sg->unit_id== UNT_ICEBOUNDTRAP?1000:0);// Cluster Bomb/Icebound has 1s to disappear once activated.
			sg->unit_id = UNT_USED_TRAPS; //Changed ID so it does not invoke a for each in area again.
			break;

		case UNT_TALKIEBOX:
			if (sg->src_id == bl->id)
				break;
			if (sg->val2 == 0){
				clif_talkiebox(&src->bl, sg->valstr);
				sg->unit_id = UNT_USED_TRAPS;
				clif_changetraplook(&src->bl, UNT_USED_TRAPS);
				sg->limit = DIFF_TICK(tick, sg->tick) + 5000;
				sg->val2 = -1;
			}
			break;

		case UNT_LULLABY:
			if (ss->id == bl->id)
				break;
			skill_additional_effect(ss, bl, sg->skill_id, sg->skill_lv, BF_LONG|BF_SKILL|BF_MISC, ATK_DEF, tick);
			break;

		case UNT_UGLYDANCE:	//Ugly Dance [Skotlex]
			if (ss->id != bl->id)
				skill_additional_effect(ss, bl, sg->skill_id, sg->skill_lv, BF_LONG|BF_SKILL|BF_MISC, ATK_DEF, tick);
			break;

		case UNT_DISSONANCE:
			skill_attack(BF_MISC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			break;

		case UNT_APPLEIDUN: //Apple of Idun [Skotlex]
		{
			int heal;
#ifdef RENEWAL
			struct mob_data *md = BL_CAST(BL_MOB, bl);
			if( md && md->class_ == MOBID_EMPERIUM )
				break;
#endif
			if( sg->src_id == bl->id && !(tsc && tsc->data[SC_SPIRIT] && tsc->data[SC_SPIRIT]->val2 == SL_BARDDANCER) )
				break; // affects self only when soullinked
			heal = skill_calc_heal(ss,bl,sg->skill_id, sg->skill_lv, true);
			if( tsc->data[SC_AKAITSUKI] && heal )
				heal = ~heal + 1;
			clif_skill_nodamage(&src->bl, bl, AL_HEAL, heal, 1);
			status_heal(bl, heal, 0, 0);
			break;
		}

 		case UNT_TATAMIGAESHI:
		case UNT_DEMONSTRATION:
			skill_attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_GOSPEL:
			if (rnd()%100 > sg->skill_lv*10 || ss == bl)
				break;
			if (battle_check_target(ss,bl,BCT_PARTY)>0)
			{ // Support Effect only on party, not guild
				int heal;
				int i = rnd()%13; // Positive buff count
				int time = skill_get_time2(sg->skill_id, sg->skill_lv); //Duration
				switch (i)
				{
					case 0: // Heal 1~9999 HP
						heal = rnd() %9999+1;
						clif_skill_nodamage(ss,bl,AL_HEAL,heal,1);
						status_heal(bl,heal,0,0);
						break;
					case 1: // End all negative status
						status_change_clear_buffs(bl,2);
						if (tsd) clif_gospel_info(tsd, 0x15);
						break;
					case 2: // Immunity to all status
						sc_start(bl,SC_SCRESIST,100,100,time);
						if (tsd) clif_gospel_info(tsd, 0x16);
						break;
					case 3: // MaxHP +100%
						sc_start(bl,SC_INCMHPRATE,100,100,time);
						if (tsd) clif_gospel_info(tsd, 0x17);
						break;
					case 4: // MaxSP +100%
						sc_start(bl,SC_INCMSPRATE,100,100,time);
						if (tsd) clif_gospel_info(tsd, 0x18);
						break;
					case 5: // All stats +20
						sc_start(bl,SC_INCALLSTATUS,100,20,time);
						if (tsd) clif_gospel_info(tsd, 0x19);
						break;
					case 6: // Level 10 Blessing
						sc_start(bl,SC_BLESSING,100,10,time);
						break;
					case 7: // Level 10 Increase AGI
						sc_start(bl,SC_INCREASEAGI,100,10,time);
						break;
					case 8: // Enchant weapon with Holy element
						sc_start(bl,SC_ASPERSIO,100,1,time);
						if (tsd) clif_gospel_info(tsd, 0x1c);
						break;
					case 9: // Enchant armor with Holy element
						sc_start(bl,SC_BENEDICTIO,100,1,time);
						if (tsd) clif_gospel_info(tsd, 0x1d);
						break;
					case 10: // DEF +25%
						sc_start(bl,SC_INCDEFRATE,100,25,time);
						if (tsd) clif_gospel_info(tsd, 0x1e);
						break;
					case 11: // ATK +100%
						sc_start(bl,SC_INCATKRATE,100,100,time);
						if (tsd) clif_gospel_info(tsd, 0x1f);
						break;
					case 12: // HIT/Flee +50
						sc_start(bl,SC_INCHIT,100,50,time);
						sc_start(bl,SC_INCFLEE,100,50,time);
						if (tsd) clif_gospel_info(tsd, 0x20);
						break;
				}
			}
			else if (battle_check_target(&src->bl,bl,BCT_ENEMY)>0)
			{ // Offensive Effect
				int i = rnd()%9; // Negative buff count
				int time = skill_get_time2(sg->skill_id, sg->skill_lv);
				switch (i)
				{
					case 0: // Deal 1~9999 damage
						skill_attack(BF_MISC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
						break;
					case 1: // Curse
						sc_start(bl,SC_CURSE,100,1,time);
						break;
					case 2: // Blind
						sc_start(bl,SC_BLIND,100,1,time);
						break;
					case 3: // Poison
						sc_start(bl,SC_POISON,100,1,time);
						break;
					case 4: // Level 10 Provoke
						sc_start(bl,SC_PROVOKE,100,10,time);
						break;
					case 5: // DEF -100%
						sc_start(bl,SC_INCDEFRATE,100,-100,time);
						break;
					case 6: // ATK -100%
						sc_start(bl,SC_INCATKRATE,100,-100,time);
						break;
					case 7: // Flee -100%
						sc_start(bl,SC_INCFLEERATE,100,-100,time);
						break;
					case 8: // Speed/ASPD -25%
						sc_start4(bl,SC_GOSPEL,100,1,0,0,BCT_ENEMY,time);
						break;
				}
			}
			break;

		case UNT_BASILICA:
			{
				int i = battle_check_target(&src->bl, bl, BCT_ENEMY);
				if( i > 0 && !(status_get_mode(bl)&MD_BOSS) )
				{ // knock-back any enemy except Boss
					skill_blown(&src->bl, bl, 2, unit_getdir(bl), 0);
					clif_fixpos(bl);
				}

				if( sg->src_id != bl->id && i <= 0 )
					sc_start4(bl, type, 100, 0, 0, 0, src->bl.id, sg->interval + 100);
			}
			break;

		case UNT_GRAVITATION:
		case UNT_EARTHSTRAIN:
		case UNT_FIREWALK:
		case UNT_ELECTRICWALK:
		case UNT_PSYCHIC_WAVE:
			skill_attack(skill_get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_GROUNDDRIFT_WIND:
		case UNT_GROUNDDRIFT_DARK:
		case UNT_GROUNDDRIFT_POISON:
		case UNT_GROUNDDRIFT_WATER:
		case UNT_GROUNDDRIFT_FIRE:
			map_foreachinrange(skill_trap_splash,&src->bl,
				skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag,
				&src->bl,tick);
			sg->unit_id = UNT_USED_TRAPS;
			//clif_changetraplook(&src->bl, UNT_FIREPILLAR_ACTIVE);
			sg->limit=DIFF_TICK(tick,sg->tick)+1500;
			break;
		/**
		 * 3rd stuff
		 **/
		case UNT_POISONSMOKE:
			if( battle_check_target(ss,bl,BCT_ENEMY) > 0 && !(tsc && tsc->data[sg->val2]) && rnd()%100 < 20 )
				sc_start(bl,sg->val2,100,sg->val3,skill_get_time2(GC_POISONINGWEAPON, 1));
			break;

		case UNT_EPICLESIS:
			if( bl->type == BL_PC && !battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race != RC_DEMON )
			{
				if( ++sg->val2 % 3 == 0 ) {
					int hp, sp;
					switch( sg->skill_lv )
					{
						case 1: case 2: hp = 3; sp = 2; break;
						case 3: case 4: hp = 4; sp = 3; break;
						case 5: default: hp = 5; sp = 4; break;
					}
					hp = tstatus->max_hp * hp / 100;
					sp = tstatus->max_sp * sp / 100;
					status_heal(bl, hp, sp, 2);
					sc_start(bl, type, 100, sg->skill_lv, (sg->interval * 3) + 100);
				}
				// Reveal hidden players every 5 seconds.
				if( sg->val2 % 5 == 0 ) {
					// TODO: check if other hidden status can be removed.
					status_change_end(bl,SC_HIDING,INVALID_TIMER);
					status_change_end(bl,SC_CLOAKING,INVALID_TIMER);
				}
			}
			/* Enable this if kRO fix the current skill. Currently no damage on undead and demon monster. [Jobbie]
			else if( battle_check_target(ss, bl, BCT_ENEMY) > 0 && battle_check_undead(tstatus->race, tstatus->def_ele) )
				skill_castend_damage_id(&src->bl, bl, sg->skill_id, sg->skill_lv, 0, 0);*/
			break;

		case UNT_STEALTHFIELD:
			if( bl->id == sg->src_id )
				break; // Dont work on Self (video shows that)
		case UNT_NEUTRALBARRIER:
			sc_start(bl,type,100,sg->skill_lv,sg->interval + 100);
			break;

		case UNT_DIMENSIONDOOR:
			if( tsd && !map[bl->m].flag.noteleport )
				pc_randomwarp(tsd,3);
			else if( bl->type == BL_MOB && battle_config.mob_warp&8 )
				unit_warp(bl,-1,-1,-1,3);
			break;

		case UNT_REVERBERATION:
			clif_changetraplook(&src->bl,UNT_USED_TRAPS);
			map_foreachinrange(skill_trap_splash,&src->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &src->bl,tick);
			sg->limit = DIFF_TICK(tick,sg->tick)+1000;
			sg->unit_id = UNT_USED_TRAPS;
			break;

		case UNT_SEVERE_RAINSTORM:
			if( battle_check_target(&src->bl, bl, BCT_ENEMY) )
				skill_attack(BF_WEAPON,ss,&src->bl,bl,WM_SEVERE_RAINSTORM_MELEE,sg->skill_lv,tick,0);
			break;
		case UNT_NETHERWORLD:
			if( !(status_get_mode(bl)&MD_BOSS) && ss != bl && battle_check_target(&src->bl, bl, BCT_PARTY) ) {
				if( !(tsc && tsc->data[type]) ){
					sc_start(bl, type, 100, sg->skill_lv, skill_get_time2(sg->skill_id,sg->skill_lv));
					sg->limit = DIFF_TICK(tick,sg->tick);
					sg->unit_id = UNT_USED_TRAPS;
				}
			}
			break;
		case UNT_THORNS_TRAP:
			if( tsc ) {
				if( !sg->val2 ) {
					int sec = skill_get_time2(sg->skill_id, sg->skill_lv);
					if( sc_start(bl, type, 100, sg->skill_lv, sec) ) {
						const struct TimerData* td = tsc->data[type]?get_timer(tsc->data[type]->timer):NULL; 
						if( td )
							sec = DIFF_TICK(td->tick, tick);
						///map_moveblock(bl, src->bl.x, src->bl.y, tick); // in official server it doesn't behave like this. [malufett]
						clif_fixpos(bl);
						sg->val2 = bl->id;
					} else
						sec = 3000;	// Couldn't trap it?
					sg->limit = DIFF_TICK(tick, sg->tick) + sec;
				} else if( tsc->data[SC_THORNSTRAP] && bl->id == sg->val2 )
					skill_attack(skill_get_type(GN_THORNS_TRAP), ss, ss, bl, sg->skill_id, sg->skill_lv, tick, SD_LEVEL|SD_ANIMATION);
			}
			break;
			
		case UNT_DEMONIC_FIRE: {
				TBL_PC* sd =  BL_CAST(BL_PC, ss);
				switch( sg->val2 ) {
					case 1:
					case 2:
					default:
						sc_start(bl, SC_BURNING, 4 + 4 * sg->skill_lv, sg->skill_lv,
								 skill_get_time2(sg->skill_id, sg->skill_lv));
						skill_attack(skill_get_type(sg->skill_id), ss, &src->bl, bl,
									 sg->skill_id, sg->skill_lv + 10 * sg->val2, tick, 0);
						break;
					case 3:
						skill_attack(skill_get_type(CR_ACIDDEMONSTRATION), ss, &src->bl, bl,
									 CR_ACIDDEMONSTRATION, sd ? pc_checkskill(sd, CR_ACIDDEMONSTRATION) : sg->skill_lv, tick, 0);
						break;
						
				}
			}
			break;
			
		case UNT_FIRE_EXPANSION_SMOKE_POWDER:
			sc_start(bl, status_skill2sc(GN_FIRE_EXPANSION_SMOKE_POWDER), 100, sg->skill_lv, 1000);
			break;
			
		case UNT_FIRE_EXPANSION_TEAR_GAS:
			sc_start(bl, status_skill2sc(GN_FIRE_EXPANSION_TEAR_GAS), 100, sg->skill_lv, 1000);
			break;
			
		case UNT_HELLS_PLANT:
			if( battle_check_target(&src->bl,bl,BCT_ENEMY) > 0 )
				skill_attack(skill_get_type(GN_HELLS_PLANT_ATK), ss, &src->bl, bl, GN_HELLS_PLANT_ATK, sg->skill_lv, tick, 0);
			if( ss != bl) //The caster is the only one who can step on the Plants, without destroying them
				sg->limit = DIFF_TICK(tick, sg->tick) + 100;
			break;
			
		case UNT_CLOUD_KILL:
			if(tsc && !tsc->data[type])
				status_change_start(bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill_get_time2(sg->skill_id,sg->skill_lv),8);
			skill_attack(skill_get_type(sg->skill_id),ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;
			
		case UNT_WARMER:
			if( bl->type == BL_PC && !battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race != RC_DEMON ) {
				int hp = 125 * sg->skill_lv; // Officially is 125 * skill_lv.
				struct status_change *ssc = status_get_sc(ss);
				if( ssc && ssc->data[SC_HEATER_OPTION] )
					hp += hp * ssc->data[SC_HEATER_OPTION]->val3 / 100;
				if( tstatus->hp != tstatus->max_hp )
					clif_skill_nodamage(&src->bl, bl, AL_HEAL, hp, 0);
				if( tsc && tsc->data[SC_AKAITSUKI] && hp )
					hp = ~hp + 1;
				status_heal(bl, hp, 0, 0);
				sc_start(bl, SC_WARMER, 100, sg->skill_lv, skill_get_time2(sg->skill_id,sg->skill_lv));
			}
			break;
			
		case UNT_FIRE_INSIGNIA:
		case UNT_WATER_INSIGNIA:
		case UNT_WIND_INSIGNIA:
		case UNT_EARTH_INSIGNIA:
		case UNT_ZEPHYR:
			sc_start(bl,type, 100, sg->skill_lv, sg->interval);
			break;			
			
		case UNT_VACUUM_EXTREME:
			{// TODO: official behavior in gvg area. [malufett]
				int sec = sg->limit - DIFF_TICK(tick, sg->tick); 
				int range = skill_get_unit_range(sg->skill_id, sg->skill_lv);

				if( tsc && !tsc->data[type] &&
					distance_xy(src->bl.x, src->bl.y, bl->x, bl->y) <= range)// don't consider outer bounderies
					sc_start(bl, type, 100, sg->skill_lv, sec);

				if( unit_is_walking(bl) && // wait until target stop walking
					( tsc && tsc->data[type] && tsc->data[type]->val4 >= tsc->data[type]->val3-range ))
						break; 

				if( tsc && ( !tsc->data[type] || (tsc->data[type] && tsc->data[type]->val4 < 1 ) ) )
					break;

				if( unit_is_walking(bl) && 
					distance_xy(src->bl.x, src->bl.y, bl->x, bl->y) > range )// going outside of boundaries? then force it to stop
						unit_stop_walking(bl,1);

				if(	!unit_is_walking(bl) && 
					distance_xy(src->bl.x, src->bl.y, bl->x, bl->y) <= range &&  // only snap if the target is inside the range or
					src->bl.x != bl->x && src->bl.y != bl->y){// diagonal position parallel to VE's center
					unit_movepos(bl, src->bl.x, src->bl.y, 0, 0);
					clif_fixpos(bl);
				}		
			}
			break;
			
		case UNT_FIRE_MANTLE:
			if( battle_check_target(&src->bl, bl, BCT_ENEMY) )
				skill_attack(BF_MAGIC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;	

		case UNT_ZENKAI_WATER:
		case UNT_ZENKAI_LAND:
		case UNT_ZENKAI_FIRE:
		case UNT_ZENKAI_WIND:
			if( battle_check_target(&src->bl,bl,BCT_ENEMY) > 0 ){
				switch( sg->unit_id ){
					case UNT_ZENKAI_WATER:
						sc_start(bl, SC_CRYSTALIZE, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						sc_start(bl, SC_FREEZE, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						sc_start(bl, SC_FREEZING, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						break;
					case UNT_ZENKAI_LAND:
						sc_start(bl, SC_STONE, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						sc_start(bl, SC_POISON, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						break;
					case UNT_ZENKAI_FIRE:
						sc_start(bl, SC_BURNING, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						break;
					case UNT_ZENKAI_WIND:
						sc_start(bl, SC_SILENCE, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						sc_start(bl, SC_SLEEP, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						sc_start(bl, SC_DEEPSLEEP, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv)); 
						break;
				}
			}else
				sc_start2(bl,type,100,sg->val1,sg->val2,skill_get_time2(sg->skill_id, sg->skill_lv));
			break;
	
	}

	if (bl->type == BL_MOB && ss != bl)
		mobskill_event((TBL_MOB*)bl, ss, tick, MSC_SKILLUSED|(skillid<<16));

	return skillid;
}
/*==========================================
 * Triggered when a char steps out of a skill cell
 *------------------------------------------*/
int skill_unit_onout (struct skill_unit *src, struct block_list *bl, unsigned int tick)
{
	struct skill_unit_group *sg;
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	nullpo_ret(src);
	nullpo_ret(bl);
	nullpo_ret(sg=src->group);
	sc = status_get_sc(bl);
	type = status_skill2sc(sg->skill_id);
	sce = (sc && type != -1)?sc->data[type]:NULL;
	
	if( bl->prev==NULL ||
		(status_isdead(bl) && sg->unit_id != UNT_ANKLESNARE && sg->unit_id != UNT_SPIDERWEB) ) //Need to delete the trap if the source died.
		return 0;

	switch(sg->unit_id){
	case UNT_SAFETYWALL:
	case UNT_PNEUMA:
	case UNT_EPICLESIS://Arch Bishop
	case UNT_NEUTRALBARRIER:
	case UNT_STEALTHFIELD:
		if (sce)
			status_change_end(bl, type, INVALID_TIMER);
		break;

	case UNT_BASILICA:
		if( sce && sce->val4 == src->bl.id )
			status_change_end(bl, type, INVALID_TIMER);
		break;
	case UNT_HERMODE:	//Clear Hermode if the owner moved.
		if (sce && sce->val3 == BCT_SELF && sce->val4 == sg->group_id)
			status_change_end(bl, type, INVALID_TIMER);
		break;

	case UNT_SPIDERWEB:
		{
			struct block_list *target = map_id2bl(sg->val2);
			if (target && target==bl)
			{
				if (sce && sce->val3 == sg->group_id)
					status_change_end(bl, type, INVALID_TIMER);
				sg->limit = DIFF_TICK(tick,sg->tick)+1000;
			}
			break;
		}
	}
	return sg->skill_id;
}

/*==========================================
 * Triggered when a char steps out of a skill group (entirely) [Skotlex]
 *------------------------------------------*/
static int skill_unit_onleft (int skill_id, struct block_list *bl, unsigned int tick)
{
	struct status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;

	type = status_skill2sc(skill_id);
	sce = (sc && type != -1)?sc->data[type]:NULL;

	switch (skill_id)
	{
		case WZ_QUAGMIRE:
			if (bl->type==BL_MOB)
				break;
			if (sce)
				status_change_end(bl, type, INVALID_TIMER);
			break;

		case BD_LULLABY:
		case BD_RICHMANKIM:
		case BD_ETERNALCHAOS:
		case BD_DRUMBATTLEFIELD:
		case BD_RINGNIBELUNGEN:
		case BD_ROKISWEIL:
		case BD_INTOABYSS:
		case BD_SIEGFRIED:
			if(sc && sc->data[SC_DANCING] && (sc->data[SC_DANCING]->val1&0xFFFF) == skill_id)
			{	//Check if you just stepped out of your ensemble skill to cancel dancing. [Skotlex]
				//We don't check for SC_LONGING because someone could always have knocked you back and out of the song/dance.
				//FIXME: This code is not perfect, it doesn't checks for the real ensemble's owner,
				//it only checks if you are doing the same ensemble. So if there's two chars doing an ensemble
				//which overlaps, by stepping outside of the other parther's ensemble will cause you to cancel
				//your own. Let's pray that scenario is pretty unlikely and noone will complain too much about it.
				status_change_end(bl, SC_DANCING, INVALID_TIMER);
			}
		case MG_SAFETYWALL:
		case AL_PNEUMA:
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
		case CG_HERMODE:
		case HW_GRAVITATION:
		case NJ_SUITON:
		case SC_MAELSTROM:
		case SC_BLOODYLUST:
		case EL_WATER_BARRIER:
		case EL_ZEPHYR:
		case EL_POWER_OF_GAIA:		
		case SO_FIRE_INSIGNIA:
		case SO_WATER_INSIGNIA:
		case SO_WIND_INSIGNIA:
		case SO_EARTH_INSIGNIA:			
			if (sce)
				status_change_end(bl, type, INVALID_TIMER);
			break;

		case BA_POEMBRAGI:
		case BA_WHISTLE:
		case BA_ASSASSINCROSS:
		case BA_APPLEIDUN:
		case DC_HUMMING:
		case DC_DONTFORGETME:
		case DC_FORTUNEKISS:
		case DC_SERVICEFORYOU:
			if (sce) {
				if( sce->val4 == 2 ) {/* We have the status but we're not elegible for it, so we take it away. (bugreport:4591) */
					if( !( sc && sc->data[SC_DANCING] && sc->data[SC_DANCING]->val2 ) )/* check if we're still in the skill */
						status_change_end(bl,type,INVALID_TIMER);
				} else {
					delete_timer(sce->timer, status_change_timer);
					//NOTE: It'd be nice if we could get the skill_lv for a more accurate extra time, but alas...
					//not possible on our current implementation.
					sce->val4 = 1; //Store the fact that this is a "reduced" duration effect.
					sce->timer = add_timer(tick+skill_get_time2(skill_id,1), status_change_timer, bl->id, type);
				}
			}
			break;
		case PF_FOGWALL:
			if (sce)
			{
				status_change_end(bl, type, INVALID_TIMER);
				if ((sce=sc->data[SC_BLIND]))
				{
					if (bl->type == BL_PC) //Players get blind ended inmediately, others have it still for 30 secs. [Skotlex]
						status_change_end(bl, SC_BLIND, INVALID_TIMER);
					else {
						delete_timer(sce->timer, status_change_timer);
						sce->timer = add_timer(30000+tick, status_change_timer, bl->id, SC_BLIND);
					}
				}
			}
			break;
		case GD_LEADERSHIP:
		case GD_GLORYWOUNDS:
		case GD_SOULCOLD:
		case GD_HAWKEYES:
			if( !(sce && sce->val4) )
				status_change_end(bl, type, INVALID_TIMER);
			break;
	}

	return skill_id;
}

/*==========================================
 * Invoked when a unit cell has been placed/removed/deleted.
 * flag values:
 * flag&1: Invoke onplace function (otherwise invoke onout)
 * flag&4: Invoke a onleft call (the unit might be scheduled for deletion)
 *------------------------------------------*/
static int skill_unit_effect (struct block_list* bl, va_list ap)
{
	struct skill_unit* unit = va_arg(ap,struct skill_unit*);
	struct skill_unit_group* group = unit->group;
	unsigned int tick = va_arg(ap,unsigned int);
	unsigned int flag = va_arg(ap,unsigned int);
	int skill_id;
	bool dissonance;

	if( (!unit->alive && !(flag&4)) || bl->prev == NULL )
		return 0;

	nullpo_ret(group);

	dissonance = skill_dance_switch(unit, 0);

	//Necessary in case the group is deleted after calling on_place/on_out [Skotlex]
	skill_id = group->skill_id;

	//Target-type check.
	if( !(group->bl_flag&bl->type && battle_check_target(&unit->bl,bl,group->target_flag)>0) && (flag&4) ) {
		if( group->state.song_dance&0x1 || (group->src_id == bl->id && group->state.song_dance&0x2) )
			skill_unit_onleft(skill_id, bl, tick);//Ensemble check to terminate it.
	} else {
		if( flag&1 )
			skill_unit_onplace(unit,bl,tick);
		else
			skill_unit_onout(unit,bl,tick);

		if( flag&4 )
	  		skill_unit_onleft(skill_id, bl, tick);
	}

	if( dissonance ) skill_dance_switch(unit, 1);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_ondamaged (struct skill_unit *src, struct block_list *bl, int damage, unsigned int tick)
{
	struct skill_unit_group *sg;

	nullpo_ret(src);
	nullpo_ret(sg=src->group);

	switch( sg->unit_id ) {
	case UNT_BLASTMINE:
	case UNT_SKIDTRAP:
	case UNT_LANDMINE:
	case UNT_SHOCKWAVE:
	case UNT_SANDMAN:
	case UNT_FLASHER:
	case UNT_CLAYMORETRAP:
	case UNT_FREEZINGTRAP:
	case UNT_TALKIEBOX:
	case UNT_ANKLESNARE:
	case UNT_ICEWALL:
	case UNT_REVERBERATION:
	case UNT_WALLOFTHORN:
		src->val1-=damage;
		break;
	default:
		damage = 0;
		break;
	}
	return damage;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_check_condition_char_sub (struct block_list *bl, va_list ap)
{
	int *c, skillid;
	struct block_list *src;
	struct map_session_data *sd;
	struct map_session_data *tsd;
	int *p_sd;	//Contains the list of characters found.

	nullpo_ret(bl);
	nullpo_ret(tsd=(struct map_session_data*)bl);
	nullpo_ret(src=va_arg(ap,struct block_list *));
	nullpo_ret(sd=(struct map_session_data*)src);

	c=va_arg(ap,int *);
	p_sd = va_arg(ap, int *);
	skillid = va_arg(ap,int);

	if ( ((skillid != PR_BENEDICTIO && *c >=1) || *c >=2) && !(skill_get_inf2(skillid)&INF2_CHORUS_SKILL) )
		return 0; //Partner found for ensembles, or the two companions for Benedictio. [Skotlex]

	if (bl == src)
		return 0;

	if(pc_isdead(tsd))
		return 0;

	if (tsd->sc.data[SC_SILENCE] || ( tsd->sc.opt1 && tsd->sc.opt1 != OPT1_BURNING ))
		return 0;

	if( skill_get_inf2(skillid)&INF2_CHORUS_SKILL ) {
		if( tsd->status.party_id == sd->status.party_id && (tsd->class_&MAPID_THIRDMASK) == MAPID_MINSTRELWANDERER )
			p_sd[(*c)++] = tsd->bl.id;
		return 1;
	} else {

		switch(skillid) {
			case PR_BENEDICTIO: {
				int dir = map_calc_dir(&sd->bl,tsd->bl.x,tsd->bl.y);
				dir = (unit_getdir(&sd->bl) + dir)%8; //This adjusts dir to account for the direction the sd is facing.
				if ((tsd->class_&MAPID_BASEMASK) == MAPID_ACOLYTE && (dir == 2 || dir == 6) //Must be standing to the left/right of Priest.
					&& sd->status.sp >= 10)
					p_sd[(*c)++]=tsd->bl.id;
				return 1;
			}
			case AB_ADORAMUS:
			// Adoramus does not consume Blue Gemstone when there is at least 1 Priest class next to the caster
				if( (tsd->class_&MAPID_UPPERMASK) == MAPID_PRIEST )
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			case WL_COMET:
			// Comet does not consume Red Gemstones when there is at least 1 Warlock class next to the caster
				if( ( sd->class_&MAPID_THIRDMASK ) == MAPID_WARLOCK )
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			case LG_RAYOFGENESIS:
				if( tsd->status.party_id == sd->status.party_id && (tsd->class_&MAPID_THIRDMASK) == MAPID_ROYAL_GUARD &&
					tsd->sc.data[SC_BANDING] )
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			default: //Warning: Assuming Ensemble Dance/Songs for code speed. [Skotlex]
				{
					int skilllv;
					if(pc_issit(tsd) || !unit_can_move(&tsd->bl))
						return 0;
					if (sd->status.sex != tsd->status.sex &&
							(tsd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER &&
							(skilllv = pc_checkskill(tsd, skillid)) > 0 &&
							(tsd->weapontype1==W_MUSICAL || tsd->weapontype1==W_WHIP) &&
							sd->status.party_id && tsd->status.party_id &&
							sd->status.party_id == tsd->status.party_id &&
							!tsd->sc.data[SC_DANCING])
					{
						p_sd[(*c)++]=tsd->bl.id;
						return skilllv;
					} else {
						return 0;
					}
				}
				break;
		}
		
	}
	return 0;
}

/*==========================================
 * Checks and stores partners for ensemble skills [Skotlex]
 *------------------------------------------*/
int skill_check_pc_partner (struct map_session_data *sd, short skill_id, short* skill_lv, int range, int cast_flag)
{
	static int c=0;
	static int p_sd[2] = { 0, 0 };
	int i;
	bool is_chorus = ( skill_get_inf2(skill_id)&INF2_CHORUS_SKILL );

	if (!battle_config.player_skill_partner_check || pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL))
		return is_chorus ? MAX_PARTY : 99; //As if there were infinite partners.

	if (cast_flag) {	//Execute the skill on the partners.
		struct map_session_data* tsd;
		switch (skill_id) {
			case PR_BENEDICTIO:
				for (i = 0; i < c; i++) {
					if ((tsd = map_id2sd(p_sd[i])) != NULL)
						status_charge(&tsd->bl, 0, 10);
				}
				return c;
			case AB_ADORAMUS:
				if( c > 0 && (tsd = map_id2sd(p_sd[0])) != NULL ) {
					i = 2 * (*skill_lv);
					status_charge(&tsd->bl, 0, i);
				}
				break;
			case WM_GREAT_ECHO:
				for( i = 0; i < c; i++ ) {
					if( (tsd = map_id2sd(p_sd[i])) != NULL )
						status_zap(&tsd->bl,0,skill_get_sp(skill_id,*skill_lv)/c);
				}
			break;
			default: //Warning: Assuming Ensemble skills here (for speed)
				if( is_chorus )
					break;//Chorus skills are not to be parsed as ensambles
				if (c > 0 && sd->sc.data[SC_DANCING] && (tsd = map_id2sd(p_sd[0])) != NULL) {
					sd->sc.data[SC_DANCING]->val4 = tsd->bl.id;
					sc_start4(&tsd->bl,SC_DANCING,100,skill_id,sd->sc.data[SC_DANCING]->val2,*skill_lv,sd->bl.id,skill_get_time(skill_id,*skill_lv)+1000);
					clif_skill_nodamage(&tsd->bl, &sd->bl, skill_id, *skill_lv, 1);
					tsd->skillid_dance = skill_id;
					tsd->skilllv_dance = *skill_lv;
				}
				return c;
		}
	}

	//Else: new search for partners.
	c = 0;
	memset (p_sd, 0, sizeof(p_sd));
	if( is_chorus )
		i = party_foreachsamemap(skill_check_condition_char_sub,sd,AREA_SIZE,&sd->bl, &c, &p_sd, skill_id, *skill_lv);
	else
		i = map_foreachinrange(skill_check_condition_char_sub, &sd->bl, range, BL_PC, &sd->bl, &c, &p_sd, skill_id);

	if ( skill_id != PR_BENEDICTIO && skill_id != AB_ADORAMUS && skill_id != WL_COMET ) //Apply the average lv to encore skills.
		*skill_lv = (i+(*skill_lv))/(c+1); //I know c should be one, but this shows how it could be used for the average of n partners.
	return c;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_check_condition_mob_master_sub (struct block_list *bl, va_list ap)
{
	int *c,src_id,mob_class,skill;
	struct mob_data *md;

	md=(struct mob_data*)bl;
	src_id=va_arg(ap,int);
	mob_class=va_arg(ap,int);
	skill=va_arg(ap,int);
	c=va_arg(ap,int *);

	if( md->master_id != src_id || md->special_state.ai != (unsigned)(skill == AM_SPHEREMINE?2:skill == KO_ZANZOU?4:3) )
		return 0; //Non alchemist summoned mobs have nothing to do here.

	if(md->class_==mob_class)
		(*c)++;

	return 1;
}

/*==========================================
 * Determines if a given skill should be made to consume ammo
 * when used by the player. [Skotlex]
 *------------------------------------------*/
int skill_isammotype (struct map_session_data *sd, int skill)
{
	return (
		battle_config.arrow_decrement==2 &&
		(sd->status.weapon == W_BOW || (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE)) &&
		skill != HT_PHANTASMIC &&
		skill_get_type(skill) == BF_WEAPON &&
	  	!(skill_get_nk(skill)&NK_NO_DAMAGE) &&
		!skill_get_spiritball(skill,1) //Assume spirit spheres are used as ammo instead.
	);
}

int skill_check_condition_castbegin(struct map_session_data* sd, short skill, short lv) {
	struct status_data *status;
	struct status_change *sc;
	struct skill_condition require;
	int i;

	nullpo_ret(sd);
	
	if (lv <= 0 || sd->chatID) return 0;

	if( pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL) && sd->skillitem != skill )
	{	//GMs don't override the skillItem check, otherwise they can use items without them being consumed! [Skotlex]
		sd->state.arrow_atk = skill_get_ammotype(skill)?1:0; //Need to do arrow state check.
		sd->spiritball_old = sd->spiritball; //Need to do Spiritball check.
		return 1;
	}

	switch( sd->menuskill_id ) {
		case AM_PHARMACY:
			switch( skill ) {
				case AM_PHARMACY:
				case AC_MAKINGARROW:
				case BS_REPAIRWEAPON:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
					return 0;
			}
			break;
		case GN_MIX_COOKING:
		case GN_MAKEBOMB:
		case GN_S_PHARMACY:
		case GN_CHANGEMATERIAL:
			if( sd->menuskill_id != skill )
				return 0;
			break;
	}
	status = &sd->battle_status;
	sc = &sd->sc;
	if( !sc->count )
		sc = NULL;

	if( sd->skillitem == skill )
	{
		if( sd->state.abra_flag ) // Hocus-Pocus was used. [Inkfish]
			sd->state.abra_flag = 0;
		else
		{ // When a target was selected, consume items that were skipped in pc_use_item [Skotlex]
			if( (i = sd->itemindex) == -1 ||
				sd->status.inventory[i].nameid != sd->itemid ||
				sd->inventory_data[i] == NULL ||
				!sd->inventory_data[i]->flag.delay_consume ||
				sd->status.inventory[i].amount < 1
				)
			{	//Something went wrong, item exploit?
				sd->itemid = sd->itemindex = -1;
				return 0;
			}
			//Consume
			sd->itemid = sd->itemindex = -1;
			if( skill == WZ_EARTHSPIKE && sc && sc->data[SC_EARTHSCROLL] && rnd()%100 > sc->data[SC_EARTHSCROLL]->val2 ) // [marquis007]
				; //Do not consume item.
			else if( sd->status.inventory[i].expire_time == 0 )
				pc_delitem(sd,i,1,0,0,LOG_TYPE_CONSUME); // Rental usable items are not consumed until expiration
		}
		return 1;
	}

	if( pc_is90overweight(sd) )
	{
		clif_skill_fail(sd,skill,USESKILL_FAIL_WEIGHTOVER,0);
		return 0;
	}

	if( sc && ( sc->data[SC__SHADOWFORM] || sc->data[SC__IGNORANCE] ) )
		return 0;

	switch( skill ) { // Turn off check.
		case BS_MAXIMIZE:		case NV_TRICKDEAD:	case TF_HIDING:			case AS_CLOAKING:		case CR_AUTOGUARD:
		case ML_AUTOGUARD:		case CR_DEFENDER:	case ML_DEFENDER:		case ST_CHASEWALK:		case PA_GOSPEL:
		case CR_SHRINK:			case TK_RUN:		case GS_GATLINGFEVER:	case TK_READYCOUNTER:	case TK_READYDOWN:
		case TK_READYSTORM:		case TK_READYTURN:	case SG_FUSION:			case RA_WUGDASH:		case KO_YAMIKUMO:
			if( sc && sc->data[status_skill2sc(skill)] )
				return 1;
	}

	// Check the skills that can be used while mounted on a warg
	if( pc_isridingwug(sd) ) {
		switch( skill ) {
			case HT_SKIDTRAP:     case HT_LANDMINE:     case HT_ANKLESNARE:     case HT_SHOCKWAVE:
			case HT_SANDMAN:      case HT_FLASHER:      case HT_FREEZINGTRAP:   case HT_BLASTMINE:
			case HT_CLAYMORETRAP: case HT_SPRINGTRAP:   case RA_DETONATOR:      case RA_CLUSTERBOMB:
			case HT_TALKIEBOX:	  case RA_FIRINGTRAP:	case RA_ICEBOUNDTRAP:
			case RA_WUGDASH:      case RA_WUGRIDER:		case RA_WUGSTRIKE:
				break;
			default: // in official there is no message.
				return 0;
		}

	}
	if( pc_ismadogear(sd) ) {
		switch( skill ) { //None Mado skills are unusable when Mado is equipped. [Jobbie]
			case BS_REPAIRWEAPON:  case WS_MELTDOWN:
			case BS_HAMMERFALL:    case WS_CARTBOOST:
			case BS_ADRENALINE:    case WS_WEAPONREFINE:
			case BS_WEAPONPERFECT: case WS_CARTTERMINATION:
			case BS_OVERTHRUST:    case WS_OVERTHRUSTMAX:
			case BS_MAXIMIZE:	   case NC_AXEBOOMERANG:
			case BS_ADRENALINE2:   case NC_POWERSWING:
			case BS_UNFAIRLYTRICK: case NC_AXETORNADO:
			case BS_GREED:
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			default: //Only Mechanic exlcusive skill can be used.
				break;
		}
	}
	if( lv < 1 || lv > MAX_SKILL_LEVEL )
		return 0;

	require = skill_get_requirement(sd,skill,lv);

	//Can only update state when weapon/arrow info is checked.
	sd->state.arrow_atk = require.ammo?1:0;
	
	// perform skill-specific checks (and actions)
	switch( skill ) {
		case SO_SPELLFIST:	
			if(sd->skillid_old != MG_FIREBOLT && sd->skillid_old != MG_COLDBOLT && sd->skillid_old != MG_LIGHTNINGBOLT){
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case SA_CASTCANCEL:		
			if(sd->ud.skilltimer == INVALID_TIMER) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case AL_WARP:
			if(!battle_config.duel_allow_teleport && sd->duel_group) { // duel restriction [LuzZza]
				clif_displaymessage(sd->fd, "Duel: Can't use warp in duel.");
				return 0;
			}
			break;
		case MO_CALLSPIRITS:
			if(sc && sc->data[SC_RAISINGDRAGON])
				lv += sc->data[SC_RAISINGDRAGON]->val1;
			if(sd->spiritball >= lv) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case MO_FINGEROFFENSIVE:
		case GS_FLING:
		case SR_RAMPAGEBLASTER:
		case SR_RIDEINLIGHTNING:
			if( sd->spiritball > 0 && sd->spiritball < require.spiritball )
				sd->spiritball_old = require.spiritball = sd->spiritball;
			else
				sd->spiritball_old = require.spiritball;
			break;
		case MO_CHAINCOMBO:
			if(!sc)
				return 0;
			if(sc->data[SC_BLADESTOP])
				break;
			if(sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == MO_TRIPLEATTACK)
				break;
			return 0;
		case MO_COMBOFINISH:
			if(!(sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == MO_CHAINCOMBO))
				return 0;
			break;
		case CH_TIGERFIST:
			if(!(sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == MO_COMBOFINISH))
				return 0;
			break;
		case CH_CHAINCRUSH:
			if(!(sc && sc->data[SC_COMBO]))
				return 0;
			if(sc->data[SC_COMBO]->val1 != MO_COMBOFINISH && sc->data[SC_COMBO]->val1 != CH_TIGERFIST)
				return 0;
			break;
		case MO_EXTREMITYFIST:
	//		if(sc && sc->data[SC_EXTREMITYFIST]) //To disable Asura during the 5 min skill block uncomment this...
	//			return 0;
			if( sc && (sc->data[SC_BLADESTOP] || sc->data[SC_CURSEDCIRCLE_ATKER]) )
				break;
			if( sc && sc->data[SC_COMBO] )
			{
				switch(sc->data[SC_COMBO]->val1) {
					case MO_COMBOFINISH:
					case CH_TIGERFIST:
					case CH_CHAINCRUSH:
						break;
					default:
						return 0;
				}
			}
			else if( !unit_can_move(&sd->bl) )
			{	//Placed here as ST_MOVE_ENABLE should not apply if rooted or on a combo. [Skotlex]
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case TK_MISSION:
			if( (sd->class_&MAPID_UPPERMASK) != MAPID_TAEKWON )
			{// Cannot be used by Non-Taekwon classes
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case TK_READYCOUNTER:
		case TK_READYDOWN:
		case TK_READYSTORM:
		case TK_READYTURN:
		case TK_JUMPKICK:
			if( (sd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER )
			{// Soul Linkers cannot use this skill
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case TK_TURNKICK:
		case TK_STORMKICK:
		case TK_DOWNKICK:
		case TK_COUNTER:
			if ((sd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
				return 0; //Anti-Soul Linker check in case you job-changed with Stances active.
			if(!(sc && sc->data[SC_COMBO]) || sc->data[SC_COMBO]->val1 == TK_JUMPKICK)
				return 0; //Combo needs to be ready

			if (sc->data[SC_COMBO]->val3) {	//Kick chain
				//Do not repeat a kick.
				if (sc->data[SC_COMBO]->val3 != skill)
					break;
				status_change_end(&sd->bl, SC_COMBO, INVALID_TIMER);
				return 0;
			}
			if(sc->data[SC_COMBO]->val1 != skill && !( sd && sd->status.base_level >= 90 && pc_famerank(sd->status.char_id, MAPID_TAEKWON) )) {	//Cancel combo wait.
				unit_cancel_combo(&sd->bl);
				return 0;
			}
			break; //Combo ready.
		case BD_ADAPTATION:
			{
				int time;
				if(!(sc && sc->data[SC_DANCING]))
				{
					clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
				time = 1000*(sc->data[SC_DANCING]->val3>>16);
				if (skill_get_time(
					(sc->data[SC_DANCING]->val1&0xFFFF), //Dance Skill ID
					(sc->data[SC_DANCING]->val1>>16)) //Dance Skill LV
					- time < skill_get_time2(skill,lv))
				{
					clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
					return 0;
				}
			}
			break;

		case PR_BENEDICTIO:
			if (skill_check_pc_partner(sd, skill, &lv, 1, 0) < 2)
			{
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case SL_SMA:
			if(!(sc && sc->data[SC_SMA]))
				return 0;
			break;

		case HT_POWER:
			if(!(sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == skill))
				return 0;
			break;

		case CG_HERMODE:
			if(!npc_check_areanpc(1,sd->bl.m,sd->bl.x,sd->bl.y,skill_get_splash(skill, lv)))
			{
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case CG_MOONLIT: //Check there's no wall in the range+1 area around the caster. [Skotlex]
			{
				int i,x,y,range = skill_get_splash(skill, lv)+1;
				int size = range*2+1;
				for (i=0;i<size*size;i++) {
					x = sd->bl.x+(i%size-range);
					y = sd->bl.y+(i/size-range);
					if (map_getcell(sd->bl.m,x,y,CELL_CHKWALL)) {
						clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
						return 0;
					}
				}
			}
			break;
		case PR_REDEMPTIO:
			{
				int exp;
				if( ((exp = pc_nextbaseexp(sd)) > 0 && get_percentage(sd->status.base_exp, exp) < 1) ||
					((exp = pc_nextjobexp(sd)) > 0 && get_percentage(sd->status.job_exp, exp) < 1)) {
					clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0); //Not enough exp.
					return 0;
				}
				break;
			}
		case AM_TWILIGHT2:
		case AM_TWILIGHT3:
			if (!party_skill_check(sd, sd->status.party_id, skill, lv))
			{
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case SG_SUN_WARM:
		case SG_MOON_WARM:
		case SG_STAR_WARM:
			if (sc && sc->data[SC_MIRACLE])
				break;
			i = skill-SG_SUN_WARM;
			if (sd->bl.m == sd->feel_map[i].m)
				break;
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
			break;
		case SG_SUN_COMFORT:
		case SG_MOON_COMFORT:
		case SG_STAR_COMFORT:
			if (sc && sc->data[SC_MIRACLE])
				break;
			i = skill-SG_SUN_COMFORT;
			if (sd->bl.m == sd->feel_map[i].m &&
				(battle_config.allow_skill_without_day || sg_info[i].day_func()))
				break;
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		case SG_FUSION:
			if (sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_STAR)
				break;
			//Auron insists we should implement SP consumption when you are not Soul Linked. [Skotlex]
			//Only invoke on skill begin cast (instant cast skill). [Kevin]
			if( require.sp > 0 )
			{
				if (status->sp < (unsigned int)require.sp)
					clif_skill_fail(sd,skill,USESKILL_FAIL_SP_INSUFFICIENT,0);
				else
					status_zap(&sd->bl, 0, require.sp);
			}
			return 0;
		case GD_BATTLEORDER:
		case GD_REGENERATION:
		case GD_RESTORE:
			if (!map_flag_gvg2(sd->bl.m)) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case GD_EMERGENCYCALL:
			// other checks were already done in skillnotok()
			if (!sd->status.guild_id || !sd->state.gmaster_flag)
				return 0;
			break;

		case GS_GLITTERING:
			if(sd->spiritball >= 10) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case NJ_ISSEN:
			if (status->hp < 2) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		case NJ_BUNSINJYUTSU:
			if (!(sc && sc->data[SC_NEN])) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;

		case NJ_ZENYNAGE:
		case KO_MUCHANAGE:
			if(sd->status.zeny < require.zeny) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_MONEY,0);
				return 0;
			}
			break;
		case PF_HPCONVERSION:
			if (status->sp == status->max_sp)
				return 0; //Unusable when at full SP.
			break;
		case AM_CALLHOMUN: //Can't summon if a hom is already out
			if (sd->status.hom_id && sd->hd && !sd->hd->homunculus.vaporize) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case AM_REST: //Can't vapo homun if you don't have an active homunc or it's hp is < 80%
			if (!merc_is_hom_active(sd->hd) || sd->hd->battle_status.hp < (sd->hd->battle_status.max_hp*80/100))
			{
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		/**
		 * Arch Bishop
		 **/
		case AB_ANCILLA:
			{
				int count = 0;
				for( i = 0; i < MAX_INVENTORY; i ++ )
					if( sd->status.inventory[i].nameid == ITEMID_ANCILLA )
						count += sd->status.inventory[i].amount;
				if( count >= 3 ) {
					clif_skill_fail(sd, skill, USESKILL_FAIL_ANCILLA_NUMOVER, 0);
					return 0;
				}
			}
			break;
		/**
		 * Keeping as a note:
		 * Bug Report #17 provides a link to a sep-2011 changelog that shows this requirement was removed
		 **/
		//case AB_LAUDAAGNUS:
		//case AB_LAUDARAMUS:
		//	if( !sd->status.party_id ) {
		//		clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
		//		return 0;
		//	}
		//	break;

		case AB_ADORAMUS:
		/**
		 * Warlock
		 **/
		case WL_COMET:
			if( skill_check_pc_partner(sd,skill,&lv,1,0) <= 0 && ((i = pc_search_inventory(sd,require.itemid[0])) < 0 || sd->status.inventory[i].amount < require.amount[0]) )
			{
				//clif_skill_fail(sd,skill,USESKILL_FAIL_NEED_ITEM,require.amount[0],require.itemid[0]);
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case WL_SUMMONFB:
		case WL_SUMMONBL:
		case WL_SUMMONWB:
		case WL_SUMMONSTONE:
			if( sc )
			{
				ARR_FIND(SC_SPHERE_1,SC_SPHERE_5+1,i,!sc->data[i]);
				if( i == SC_SPHERE_5+1 )
				{ // No more free slots
					clif_skill_fail(sd,skill,USESKILL_FAIL_SUMMON,0);
					return 0;
				}
			}
			break;
		/**
		 * Guilotine Cross
		 **/
		case GC_HALLUCINATIONWALK:
			if( sc && (sc->data[SC_HALLUCINATIONWALK] || sc->data[SC_HALLUCINATIONWALK_POSTDELAY]) ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case GC_COUNTERSLASH:
		case GC_WEAPONCRUSH:
			if( !(sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == GC_WEAPONBLOCKING) ) {
				clif_skill_fail(sd, skill, USESKILL_FAIL_GC_WEAPONBLOCKING, 0);
				return 0;
			}
			break;
		case GC_CROSSRIPPERSLASHER:
			if( !(sc && sc->data[SC_ROLLINGCUTTER]) ) {
				clif_skill_fail(sd, skill, USESKILL_FAIL_CONDITION, 0);
				return 0;
			}
			break;
		case GC_POISONSMOKE:
		case GC_VENOMPRESSURE:
			if( !(sc && sc->data[SC_POISONINGWEAPON]) ) {
				clif_skill_fail(sd, skill, USESKILL_FAIL_GC_POISONINGWEAPON, 0);
				return 0;
			}
			break;
		/**
		 * Ranger
		 **/
		case RA_WUGMASTERY:
			if( pc_isfalcon(sd) || pc_isridingwug(sd) || sd->sc.data[SC__GROOMY]) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case RA_WUGSTRIKE:
			if( !pc_iswug(sd) && !pc_isridingwug(sd) ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case RA_WUGRIDER:
			if( pc_isfalcon(sd) || ( !pc_isridingwug(sd) && !pc_iswug(sd) ) ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case RA_WUGDASH:
			if(!pc_isridingwug(sd)) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		/**
		 * Royal Guard
		 **/
		case LG_BANDING:
			if( sc && sc->data[SC_INSPIRATION] ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case LG_PRESTIGE:
			if( sc && (sc->data[SC_BANDING] || sc->data[SC_INSPIRATION]) ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case LG_RAGEBURST:
			if( sd->spiritball == 0 ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_SKILLINTERVAL,0);
				return 0;
			}
			sd->spiritball_old = require.spiritball = sd->spiritball;
			break;
		case LG_RAYOFGENESIS:
			if( sc && sc->data[SC_INSPIRATION]  )
				return 1;	// Don't check for partner.
			if( !(sc && sc->data[SC_BANDING]) ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL,0);
				return 0;
			} else if( skill_check_pc_partner(sd,skill,&lv,skill_get_range(skill,lv),0) < 1 )
				return 0; // Just fails, no msg here.
			break;
		case LG_HESPERUSLIT:
			if( !sc || !sc->data[SC_BANDING] ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case SR_FALLENEMPIRE:
			if( !(sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == SR_DRAGONCOMBO) )
				return 0;
			break;

		case SR_CRESCENTELBOW:
			if( sc && sc->data[SC_CRESCENTELBOW] ) {
				clif_skill_fail(sd, skill, USESKILL_FAIL_DUPLICATE, 0);
				return 0;
			}
			break;
		case SR_CURSEDCIRCLE:
			if( sd->spiritball > 0 )
				sd->spiritball_old = require.spiritball = sd->spiritball;
			else {
				clif_skill_fail(sd,skill,0,0);
				return 0;
			}
			break;
		case SR_GATEOFHELL:
			if( sd->spiritball > 0 )
				sd->spiritball_old = require.spiritball;
			break;
		case SC_MANHOLE:
		case SC_DIMENSIONDOOR:
			if( sc && sc->data[SC_MAGNETICFIELD] ) {
				clif_skill_fail(sd,skill,0,0);
				return 0;
			}
			break;
		case WM_GREAT_ECHO: {
				int count;
				count = skill_check_pc_partner(sd, skill, &lv, skill_get_splash(skill,lv), 0);
				if( count < 1 ) {
					clif_skill_fail(sd,skill,USESKILL_FAIL_NEED_HELPER,0);
					return 0;
				} else
					require.sp -= require.sp * 20 * count / 100; //  -20% each W/M in the party.
			}
			break;
		case SO_FIREWALK:
		case SO_ELECTRICWALK:	// Can't be casted until you've walked all cells.
			if( sc && sc->data[SC_PROPERTYWALK] &&
			   sc->data[SC_PROPERTYWALK]->val3 < skill_get_maxcount(sc->data[SC_PROPERTYWALK]->val1,sc->data[SC_PROPERTYWALK]->val2) ) {
				clif_skill_fail(sd,skill,0x0,0);
				return 0;
			}
			break;
		case SO_EL_CONTROL:
			if( !sd->status.ele_id || !sd->ed ) {
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case RETURN_TO_ELDICASTES:
			if( pc_ismadogear(sd) ) { //Cannot be used if Mado is equipped.
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;	
		case LG_REFLECTDAMAGE:
		case CR_REFLECTSHIELD:
			if( sc && sc->data[SC_KYOMU] && rand()%100 < 30){
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
		case KO_KAHU_ENTEN:
		case KO_HYOUHU_HUBUKI:
		case KO_KAZEHU_SEIRAN:
		case KO_DOHU_KOUKAI:
			{ 
				int ttype = skill_get_ele(skill, lv);
				ARR_FIND(1, 5, i, sd->talisman[i] > 0 && i != ttype);
				if( (i < 5 && i != ttype) || sd->talisman[ttype] >= 10 ){
					clif_skill_fail(sd, skill, USESKILL_FAIL_LEVEL, 0);
					return 0;
				}
			}
			break;
		case KO_KAIHOU:
		case KO_ZENKAI:
			ARR_FIND(1, 6, i, sd->talisman[i] > 0);
			if( i > 4 ) { 
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
			break;
	}

	switch(require.state) {
	case ST_HIDING:
		if(!(sc && sc->option&OPTION_HIDE)) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_CLOAKING:
		if(!pc_iscloaking(sd)) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_HIDDEN:
		if(!pc_ishiding(sd)) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_RIDING:
		if(!pc_isriding(sd)) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_FALCON:
		if(!pc_isfalcon(sd)) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_CARTBOOST:
		if(!(sc && sc->data[SC_CARTBOOST])) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
	case ST_CART:
		if(!pc_iscarton(sd)) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_SHIELD:
		if(sd->status.shield <= 0) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_SIGHT:
		if(!(sc && sc->data[SC_SIGHT])) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_EXPLOSIONSPIRITS:
		if(!(sc && sc->data[SC_EXPLOSIONSPIRITS])) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_RECOV_WEIGHT_RATE:
		if(battle_config.natural_heal_weight_rate <= 100 && sd->weight*100/sd->max_weight >= (unsigned int)battle_config.natural_heal_weight_rate) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_MOVE_ENABLE:
		if (sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == skill)
			sd->ud.canmove_tick = gettick(); //When using a combo, cancel the can't move delay to enable the skill. [Skotlex]

		if (!unit_can_move(&sd->bl)) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	case ST_WATER:
		if (sc && (sc->data[SC_DELUGE] || sc->data[SC_SUITON]))
			break;
		if (map_getcell(sd->bl.m,sd->bl.x,sd->bl.y,CELL_CHKWATER))
			break;
		clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
		return 0;
	/**
	 * Rune Knight
	 **/
	case ST_RIDINGDRAGON:
		if( !pc_isridingdragon(sd) ) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	/**
	 * Wug
	 **/
	case ST_WUG:
		if( !pc_iswug(sd) ) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	/**
	 * Riding Wug
	 **/
	case ST_RIDINGWUG:
		if( !pc_isridingwug(sd) ){
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	/**
	 * Mechanic
	 **/
	case ST_MADO:
		if( !pc_ismadogear(sd) ) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
		break;
	/**
	 * Sorcerer
	 **/
	case ST_ELEMENTALSPIRIT:
		if(!sd->ed) {
			clif_skill_fail(sd,skill,USESKILL_FAIL_EL_SUMMON,0);
			return 0;
		}
		break;
	}

	if(require.mhp > 0 && get_percentage(status->hp, status->max_hp) > require.mhp) {
		//mhp is the max-hp-requirement, that is,
		//you must have this % or less of HP to cast it.
		clif_skill_fail(sd,skill,USESKILL_FAIL_HP_INSUFFICIENT,0);
		return 0;
	}

	if( require.weapon && !pc_check_weapontype(sd,require.weapon) ) {
		clif_skill_fail(sd,skill,USESKILL_FAIL_THIS_WEAPON,0);
		return 0;
	}

	if( require.sp > 0 && status->sp < (unsigned int)require.sp) {
		clif_skill_fail(sd,skill,USESKILL_FAIL_SP_INSUFFICIENT,0);
		return 0;
	}

	if( require.zeny > 0 && sd->status.zeny < require.zeny ) {
		clif_skill_fail(sd,skill,USESKILL_FAIL_MONEY,0);
		return 0;
	}

	if( require.spiritball > 0 && sd->spiritball < require.spiritball) {
		clif_skill_fail(sd,skill,USESKILL_FAIL_SPIRITS,require.spiritball);
		return 0;
	}

	return 1;
}

int skill_check_condition_castend(struct map_session_data* sd, short skill, short lv)
{
	struct skill_condition require;
	struct status_data *status;
	int i;
	int index[MAX_SKILL_ITEM_REQUIRE];

	nullpo_ret(sd);

	if( lv <= 0 || sd->chatID )
		return 0;

	if( pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL) && sd->skillitem != skill )
	{	//GMs don't override the skillItem check, otherwise they can use items without them being consumed! [Skotlex]
		sd->state.arrow_atk = skill_get_ammotype(skill)?1:0; //Need to do arrow state check.
		sd->spiritball_old = sd->spiritball; //Need to do Spiritball check.
		return 1;
	}

	switch( sd->menuskill_id ) { // Cast start or cast end??
		case AM_PHARMACY:
			switch( skill ) {
				case AM_PHARMACY:
				case AC_MAKINGARROW:
				case BS_REPAIRWEAPON:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
					return 0;
			}
			break;
		case GN_MIX_COOKING:
		case GN_MAKEBOMB:
		case GN_S_PHARMACY:
		case GN_CHANGEMATERIAL:
			if( sd->menuskill_id != skill )
				return 0;
			break;
	}
	
	if( sd->skillitem == skill ) // Casting finished (Item skill or Hocus-Pocus)
		return 1;

	if( pc_is90overweight(sd) )
	{
		clif_skill_fail(sd,skill,USESKILL_FAIL_WEIGHTOVER,0);
		return 0;
	}

	// perform skill-specific checks (and actions)
	switch( skill )
	{
	case PR_BENEDICTIO:
		skill_check_pc_partner(sd, skill, &lv, 1, 1);
		break;
	case AM_CANNIBALIZE:
	case AM_SPHEREMINE:
	{
		int c=0;
		int summons[5] = { 1589, 1579, 1575, 1555, 1590 };
		//int summons[5] = { 1020, 1068, 1118, 1500, 1368 };
		int maxcount = (skill==AM_CANNIBALIZE)? 6-lv : skill_get_maxcount(skill,lv);
		int mob_class = (skill==AM_CANNIBALIZE)? summons[lv-1] :1142;
		if(battle_config.land_skill_limit && maxcount>0 && (battle_config.land_skill_limit&BL_PC)) {
			i = map_foreachinmap(skill_check_condition_mob_master_sub ,sd->bl.m, BL_MOB, sd->bl.id, mob_class, skill, &c);
			if(c >= maxcount ||
				(skill==AM_CANNIBALIZE && c != i && battle_config.summon_flora&2))
			{	//Fails when: exceed max limit. There are other plant types already out.
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
				return 0;
			}
		}
		break;
	}
	case NC_SILVERSNIPER:
	case NC_MAGICDECOY:
		{
			int c = 0, j;
			int maxcount = skill_get_maxcount(skill,lv);
			int mob_class = 2042;
			if( skill == NC_MAGICDECOY )
				mob_class = 2043;

			if( battle_config.land_skill_limit && maxcount > 0 && ( battle_config.land_skill_limit&BL_PC ) ) {
				if( skill == NC_MAGICDECOY ) {
					for( j = mob_class; j <= 2046; j++ )
						map_foreachinmap(skill_check_condition_mob_master_sub, sd->bl.m, BL_MOB, sd->bl.id, j, skill, &c);
				} else
					map_foreachinmap(skill_check_condition_mob_master_sub, sd->bl.m, BL_MOB, sd->bl.id, mob_class, skill, &c);
				if( c >= maxcount ) {
					clif_skill_fail(sd , skill, USESKILL_FAIL_LEVEL, 0);
					return 0;
				}
			}
		}
		break;
	case KO_ZANZOU:
		{
			int c = 0;
			i = map_foreachinmap(skill_check_condition_mob_master_sub, sd->bl.m, BL_MOB, sd->bl.id, 2308, skill, &c);
			if( c >= skill_get_maxcount(skill,lv) || c != i)
			{
				clif_skill_fail(sd , skill, USESKILL_FAIL_LEVEL, 0);
				return 0;
			}
		}
		break;
	}

	status = &sd->battle_status;

	require = skill_get_requirement(sd,skill,lv);

	if( require.hp > 0 && status->hp <= (unsigned int)require.hp) {
		clif_skill_fail(sd,skill,USESKILL_FAIL_HP_INSUFFICIENT,0);
		return 0;
	}

	if( require.ammo ) { //Skill requires stuff equipped in the arrow slot.
		if((i=sd->equip_index[EQI_AMMO]) < 0 || !sd->inventory_data[i] ) {
			clif_arrow_fail(sd,0);
			return 0;
		} else if( sd->status.inventory[i].amount < require.ammo_qty ) {
			char e_msg[100];
			sprintf(e_msg,"Skill Failed. [%s] requires %dx %s.",
						skill_get_desc(skill),
						require.ammo_qty,
						itemdb_jname(sd->status.inventory[i].nameid));
			clif_colormes(sd,COLOR_RED,e_msg);
			return 0;
		}
		if (!(require.ammo&1<<sd->inventory_data[i]->look))
		{	//Ammo type check. Send the "wrong weapon type" message
			//which is the closest we have to wrong ammo type. [Skotlex]
			clif_arrow_fail(sd,0); //Haplo suggested we just send the equip-arrows message instead. [Skotlex]
			//clif_skill_fail(sd,skill,USESKILL_FAIL_THIS_WEAPON,0);
			return 0;
		}
	}

	for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; ++i )
	{
		if( !require.itemid[i] )
			continue;
		index[i] = pc_search_inventory(sd,require.itemid[i]);
		if( index[i] < 0 || sd->status.inventory[index[i]].amount < require.amount[i] ) {
			if( require.itemid[i] == ITEMID_RED_GEMSTONE )
				clif_skill_fail(sd,skill,USESKILL_FAIL_REDJAMSTONE,0);// red gemstone required
			else if( require.itemid[i] == ITEMID_BLUE_GEMSTONE )
				clif_skill_fail(sd,skill,USESKILL_FAIL_BLUEJAMSTONE,0);// blue gemstone required
			else
				clif_skill_fail(sd,skill,USESKILL_FAIL_LEVEL,0);
			return 0;
		}
	}

	return 1;
}

// type&2: consume items (after skill was used)
// type&1: consume the others (before skill was used)
int skill_consume_requirement( struct map_session_data *sd, short skill, short lv, short type)
{
	struct skill_condition req;

	nullpo_ret(sd);

	req = skill_get_requirement(sd,skill,lv);

	if( type&1 )
	{
		if( skill == CG_TAROTCARD || sd->state.autocast )
			req.sp = 0; // TarotCard will consume sp in skill_cast_nodamage_id [Inkfish]
		if(req.hp || req.sp)
			status_zap(&sd->bl, req.hp, req.sp);

		if(req.spiritball > 0)
			pc_delspiritball(sd,req.spiritball,0);

		if(req.zeny > 0)
		{
			if( skill == NJ_ZENYNAGE )
				req.zeny = 0; //Zeny is reduced on skill_attack.
			if( sd->status.zeny < req.zeny )
				req.zeny = sd->status.zeny;
			pc_payzeny(sd,req.zeny);
		}
	}

	if( type&2 )
	{
		struct status_change *sc = &sd->sc;
		int n,i;

		if( !sc->count )
			sc = NULL;

		for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; ++i )
		{
			if( !req.itemid[i] )
				continue;

			if( itemid_isgemstone(req.itemid[i]) && skill != HW_GANBANTEIN && sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_WIZARD )
				continue; //Gemstones are checked, but not substracted from inventory.

			if( (n = pc_search_inventory(sd,req.itemid[i])) >= 0 )
				pc_delitem(sd,n,req.amount[i],0,1,LOG_TYPE_CONSUME);
		}
	}

	return 1;
}

struct skill_condition skill_get_requirement(struct map_session_data* sd, short skill, short lv)
{
	struct skill_condition req;
	struct status_data *status;
	struct status_change *sc;
	int i,j,hp_rate,sp_rate, sp_skill_rate_bonus = 100;

	memset(&req,0,sizeof(req));

	if( !sd )
		return req;

	if( sd->skillitem == skill )
		return req; // Item skills and Hocus-Pocus don't have requirements.[Inkfish]

	sc = &sd->sc;
	if( !sc->count )
		sc = NULL;

	switch( skill )
	{ // Turn off check.
	case BS_MAXIMIZE:		case NV_TRICKDEAD:	case TF_HIDING:			case AS_CLOAKING:		case CR_AUTOGUARD:
	case ML_AUTOGUARD:		case CR_DEFENDER:	case ML_DEFENDER:		case ST_CHASEWALK:		case PA_GOSPEL:
	case CR_SHRINK:			case TK_RUN:		case GS_GATLINGFEVER:	case TK_READYCOUNTER:	case TK_READYDOWN:
	case TK_READYSTORM:		case TK_READYTURN:	case SG_FUSION:			case KO_YAMIKUMO:
		if( sc && sc->data[status_skill2sc(skill)] )
			return req;
	}

	j = skill_get_index(skill);
	if( j == 0 ) // invalid skill id
  		return req;
	if( lv < 1 || lv > MAX_SKILL_LEVEL )
		return req;

	status = &sd->battle_status;

	req.hp = skill_db[j].hp[lv-1];
	hp_rate = skill_db[j].hp_rate[lv-1];
	if(hp_rate > 0)
		req.hp += (status->hp * hp_rate)/100;
	else
		req.hp += (status->max_hp * (-hp_rate))/100;

	req.sp = skill_db[j].sp[lv-1];
	if((sd->skillid_old == BD_ENCORE) && skill == sd->skillid_dance)
		req.sp /= 2;
	sp_rate = skill_db[j].sp_rate[lv-1];
	if(sp_rate > 0)
		req.sp += (status->sp * sp_rate)/100;
	else
		req.sp += (status->max_sp * (-sp_rate))/100;
	if( sd->dsprate != 100 )
		req.sp = req.sp * sd->dsprate / 100;
	
	ARR_FIND(0, ARRAYLENGTH(sd->skillusesprate), i, sd->skillusesprate[i].id == skill);
	if( i < ARRAYLENGTH(sd->skillusesprate) )
		sp_skill_rate_bonus += sd->skillusesprate[i].val;
	ARR_FIND(0, ARRAYLENGTH(sd->skillusesp), i, sd->skillusesp[i].id == skill);
	if( i < ARRAYLENGTH(sd->skillusesp) )
		req.sp -= sd->skillusesp[i].val;
	
	req.sp = cap_value(req.sp * sp_skill_rate_bonus / 100, 0, SHRT_MAX);
	
	if( sc ) {
		if( sc->data[SC__LAZINESS] )
			req.sp += req.sp + sc->data[SC__LAZINESS]->val1 * 10;
		if( sc->data[SC_RECOGNIZEDSPELL] )
			req.sp += req.sp / 4;
	}

	req.zeny = skill_db[j].zeny[lv-1];

	if( sc && sc->data[SC__UNLUCKY] )
		req.zeny += sc->data[SC__UNLUCKY]->val1 * 500;

	req.spiritball = skill_db[j].spiritball[lv-1];

	req.state = skill_db[j].state;

	req.mhp = skill_db[j].mhp[lv-1];

	req.weapon = skill_db[j].weapon;

	req.ammo_qty = skill_db[j].ammo_qty[lv-1];
	if (req.ammo_qty)
		req.ammo = skill_db[j].ammo;

	if (!req.ammo && skill && skill_isammotype(sd, skill))
	{	//Assume this skill is using the weapon, therefore it requires arrows.
		req.ammo = 0xFFFFFFFF; //Enable use on all ammo types.
		req.ammo_qty = 1;
	}

	for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++ ) {
		if( (skill == AM_POTIONPITCHER || skill == CR_SLIMPITCHER || skill == CR_CULTIVATION) && i != lv%11 - 1 )
			continue;

		switch( skill ) {
			case AM_CALLHOMUN:
				if (sd->status.hom_id) //Don't delete items when hom is already out.
					continue;
				break;
			case NC_SHAPESHIFT:
				if( i < 4 )
					continue;
				break;
			case WZ_FIREPILLAR: // celest
				if (lv <= 5)	// no gems required at level 1-5
					continue;
				break;
			case AB_ADORAMUS:
				if( itemid_isgemstone(skill_db[j].itemid[i]) && skill_check_pc_partner(sd,skill,&lv, 1, 2) )
					continue;
				break;
			case WL_COMET:
				if( itemid_isgemstone(skill_db[j].itemid[i]) && skill_check_pc_partner(sd,skill,&lv, 1, 0) )
					continue;
				break;
			case GN_FIRE_EXPANSION:
				if( i < 5 )
					continue;
				break;
			case SO_SUMMON_AGNI:
			case SO_SUMMON_AQUA:
			case SO_SUMMON_VENTUS:
			case SO_SUMMON_TERA:
				if( i < 3 )
					continue;
				break;
		}

		req.itemid[i] = skill_db[j].itemid[i];
		req.amount[i] = skill_db[j].amount[i];

		if( itemid_isgemstone(req.itemid[i]) && skill != HW_GANBANTEIN )
		{
			if( sd->special_state.no_gemstone )
			{	//Make it substract 1 gem rather than skipping the cost.
				if( --req.amount[i] < 1 )
					req.itemid[i] = 0;
			}
			if(sc && sc->data[SC_INTOABYSS])
			{
				if( skill != SA_ABRACADABRA )
					req.itemid[i] = req.amount[i] = 0;
				else if( --req.amount[i] < 1 )
					req.amount[i] = 1; // Hocus Pocus allways use at least 1 gem
			}
		}
		if( skill >= HT_SKIDTRAP && skill <= HT_TALKIEBOX && pc_checkskill(sd, RA_RESEARCHTRAP) > 0){
			if( (j=pc_search_inventory(sd,req.itemid[i])) < 0  || ( j >= 0 && sd->status.inventory[j].amount < req.amount[i] ) ){
				req.itemid[i] = ITEMID_TRAP_ALLOY; 
				req.amount[i] = 1;
			}
			break;
		}
	}

	/* requirements are level-dependent */
	switch( skill ) {
		case NC_SHAPESHIFT:
		case GN_FIRE_EXPANSION:
		case SO_SUMMON_AGNI:
		case SO_SUMMON_AQUA:
		case SO_SUMMON_VENTUS:
		case SO_SUMMON_TERA:
			req.itemid[lv-1] = skill_db[j].itemid[lv-1];
			req.amount[lv-1] = skill_db[j].amount[lv-1];
			break;
	}
	
	// Check for cost reductions due to skills & SCs
	switch(skill) {
		case MC_MAMMONITE:
			if(pc_checkskill(sd,BS_UNFAIRLYTRICK)>0)
				req.zeny -= req.zeny*10/100;
			break;
		case AL_HOLYLIGHT:
			if(sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_PRIEST)
				req.sp *= 5;
			break;
		case SL_SMA:
		case SL_STUN:
		case SL_STIN:
		{
			int kaina_lv = pc_checkskill(sd,SL_KAINA);

			if(kaina_lv==0 || sd->status.base_level<70)
				break;
			if(sd->status.base_level>=90)
				req.sp -= req.sp*7*kaina_lv/100;
			else if(sd->status.base_level>=80)
				req.sp -= req.sp*5*kaina_lv/100;
			else if(sd->status.base_level>=70)
				req.sp -= req.sp*3*kaina_lv/100;
		}
			break;
		case MO_TRIPLEATTACK:
		case MO_CHAINCOMBO:
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
			if(sc && sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_MONK)
				req.sp -= req.sp*25/100; //FIXME: Need real data. this is a custom value.
			break;
		case MO_BODYRELOCATION:
			if( sc && sc->data[SC_EXPLOSIONSPIRITS] )
				req.spiritball = 0;
			break;
		case MO_EXTREMITYFIST:
			if( sc )
			{
				if( sc->data[SC_BLADESTOP] )
					req.spiritball--;
				else if( sc->data[SC_COMBO] )
				{
					switch( sc->data[SC_COMBO]->val1 )
					{
						case MO_COMBOFINISH:
							req.spiritball = 4;
							break;
						case CH_TIGERFIST:
							req.spiritball = 3;
							break;
						case CH_CHAINCRUSH:	//It should consume whatever is left as long as it's at least 1.
							req.spiritball = sd->spiritball?sd->spiritball:1;
							break;
					}
				}else if( sc->data[SC_RAISINGDRAGON] && sd->spiritball > 5)
					req.spiritball = sd->spiritball; // must consume all regardless of the amount required
			}
			break;
		case SR_RAMPAGEBLASTER:
			req.spiritball = sd->spiritball?sd->spiritball:15;
			break;
		case SR_GATEOFHELL:
			if( sc && sc->data[SC_COMBO] && sc->data[SC_COMBO]->val1 == SR_FALLENEMPIRE )
				req.sp -= req.sp * 10 / 100;
			break;
		case SO_SUMMON_AGNI:
		case SO_SUMMON_AQUA:
		case SO_SUMMON_VENTUS:
		case SO_SUMMON_TERA:
			req.sp -= req.sp * (5 + 5 * pc_checkskill(sd,SO_EL_SYMPATHY)) / 100;
			break;
	}
	
	return req;
}

/*==========================================
 * Does cast-time reductions based on dex, item bonuses and config setting
 *------------------------------------------*/
int skill_castfix (struct block_list *bl, int skill_id, int skill_lv) {
	int time = skill_get_cast(skill_id, skill_lv);

	nullpo_ret(bl);
#ifndef RENEWAL_CAST
	{
		struct map_session_data *sd;

		sd = BL_CAST(BL_PC, bl);

		// calculate base cast time (reduced by dex)
		if( !(skill_get_castnodex(skill_id, skill_lv)&1) ) {
			int scale = battle_config.castrate_dex_scale - status_get_dex(bl);
			if( scale > 0 )	// not instant cast
				time = time * scale / battle_config.castrate_dex_scale;
			else
				return 0;	// instant cast
		}

		// calculate cast time reduced by item/card bonuses
		if( !(skill_get_castnodex(skill_id, skill_lv)&4) && sd )
		{
			int i;
			if( sd->castrate != 100 )
				time = time * sd->castrate / 100;
			for( i = 0; i < ARRAYLENGTH(sd->skillcast) && sd->skillcast[i].id; i++ )
			{
				if( sd->skillcast[i].id == skill_id )
				{
					time+= time * sd->skillcast[i].val / 100;
					break;
				}
			}
		}
		
	}
#endif
	// config cast time multiplier
	if (battle_config.cast_rate != 100)
		time = time * battle_config.cast_rate / 100;
	// return final cast time
	return (time > 0) ? time : 0;
}

/*==========================================
 * Does cast-time reductions based on sc data.
 *------------------------------------------*/
int skill_castfix_sc (struct block_list *bl, int time)
{
	struct status_change *sc = status_get_sc(bl);

	if( time < 0 )
		return 0;

	if (sc && sc->count) {
		if (sc->data[SC_SLOWCAST])
			time += time * sc->data[SC_SLOWCAST]->val2 / 100;
		if (sc->data[SC_SUFFRAGIUM]) {
			time -= time * sc->data[SC_SUFFRAGIUM]->val2 / 100;
			status_change_end(bl, SC_SUFFRAGIUM, -1);
		}
		if (sc->data[SC_MEMORIZE]) {
			time>>=1;
			if ((--sc->data[SC_MEMORIZE]->val2) <= 0)
				status_change_end(bl, SC_MEMORIZE, -1);
		}
		if (sc->data[SC_POEMBRAGI])
			time -= time * sc->data[SC_POEMBRAGI]->val2 / 100;		
		if (sc->data[SC_IZAYOI])
			time -= time * 50 / 100;
	}

	return (time > 0) ? time : 0;
}
#ifdef RENEWAL_CAST
int skill_vfcastfix (struct block_list *bl, double time, int skill_id, int skill_lv)
{
	struct status_change *sc = status_get_sc(bl);
	struct map_session_data *sd = BL_CAST(BL_PC,bl);
	int fixed = skill_get_fixed_cast(skill_id, skill_lv), fixcast_r = 0, varcast_r = 0, i = 0;

	if( time < 0 )
		return 0;

	if( fixed == 0 ){
		fixed = (int)time * 20 / 100; // fixed time
		time = time * 80 / 100; // variable time
	}else if( fixed < 0 ) // no fixed cast time
		fixed = 0;

	if(sd  && !(skill_get_castnodex(skill_id, skill_lv)&4) ){ // Increases/Decreases fixed/variable cast time of a skill by item/card bonuses.
		if( sd->bonus.varcastrate < 0 )
			VARCAST_REDUCTION(sd->bonus.varcastrate);
		for (i = 0; i < ARRAYLENGTH(sd->skillfixcast) && sd->skillfixcast[i].id; i++) 
			if (sd->skillfixcast[i].id == skill_id){ // bonus2 bSkillFixedCast
				fixed += sd->skillfixcast[i].val;
				break;
			}
		for( i = 0; i < ARRAYLENGTH(sd->skillvarcast) && sd->skillvarcast[i].id; i++ )
			if( sd->skillvarcast[i].id == skill_id ){ // bonus2 bSkillVariableCast
				time += sd->skillvarcast[i].val;
				break;
			}
		for( i = 0; i < ARRAYLENGTH(sd->skillcast) && sd->skillcast[i].id; i++ )
			if( sd->skillcast[i].id == skill_id ){ // bonus2 bVariableCastrate
				if( (i=sd->skillcast[i].val) < 0)
					VARCAST_REDUCTION(i);
				break;
			}
	}

	if (sc && sc->count && !(skill_get_castnodex(skill_id, skill_lv)&2) ) {
		// All variable cast additive bonuses must come first
		if (sc->data[SC_SLOWCAST])
			VARCAST_REDUCTION(-sc->data[SC_SLOWCAST]->val2);

		// Variable cast reduction bonuses
		if (sc->data[SC_SUFFRAGIUM]) {
			VARCAST_REDUCTION(sc->data[SC_SUFFRAGIUM]->val2);
			status_change_end(bl, SC_SUFFRAGIUM, INVALID_TIMER);
		}
		if (sc->data[SC_MEMORIZE]) {
			VARCAST_REDUCTION(50);
			if ((--sc->data[SC_MEMORIZE]->val2) <= 0)
				status_change_end(bl, SC_MEMORIZE, INVALID_TIMER);
		}
		if (sc->data[SC_POEMBRAGI])
			VARCAST_REDUCTION(sc->data[SC_POEMBRAGI]->val2);
		if (sc->data[SC_IZAYOI])
			VARCAST_REDUCTION(50);
		// Fixed cast reduction bonuses
		if( sc->data[SC__LAZINESS] )
			fixcast_r = max(fixcast_r, sc->data[SC__LAZINESS]->val2);
		if( sc->data[SC_SECRAMENT] )
			fixcast_r = max(fixcast_r, sc->data[SC_SECRAMENT]->val2);
		if( sd && ( skill_lv = pc_checkskill(sd, WL_RADIUS) ) && skill_id >= WL_WHITEIMPRISON && skill_id <= WL_FREEZE_SP  )
			fixcast_r = max(fixcast_r, 5 + skill_lv * 5);
		// Fixed cast non percentage bonuses
		if( sc->data[SC_MANDRAGORA] && (skill_id >= SM_BASH && skill_id <= RETURN_TO_ELDICASTES) )
			fixed += sc->data[SC_MANDRAGORA]->val1 * 1000 / 2;
		if (sc->data[SC_IZAYOI]  && (skill_id >= NJ_TOBIDOUGU && skill_id <= NJ_ISSEN))
			fixed = 0;
	}

	if( sd && !(skill_get_castnodex(skill_id, skill_lv)&4) ){
		VARCAST_REDUCTION( max(sd->bonus.varcastrate, 0) + max(i, 0) );
		fixcast_r = max(fixcast_r, sd->bonus.fixcastrate) + min(sd->bonus.fixcastrate,0);
	}

	if( varcast_r < 0 ) // now compute overall factors
		time = time * (1 - (float)varcast_r / 100);
	if( !(skill_get_castnodex(skill_id, skill_lv)&1) )// reduction from status point
		time = (1 - sqrt( ((float)(status_get_dex(bl)*2 + status_get_int(bl)) / battle_config.vcast_stat_scale) )) * time;
	// underflow checking/capping
	time = max(time, 0) + (1 - (float)min(fixcast_r, 100) / 100) * fixed;

	return (int)time;
}
#endif

/*==========================================
 * Does delay reductions based on dex/agi, sc data, item bonuses, ...
 *------------------------------------------*/
int skill_delayfix (struct block_list *bl, int skill_id, int skill_lv)
{
	int delaynodex = skill_get_delaynodex(skill_id, skill_lv);
	int time = skill_get_delay(skill_id, skill_lv);
	struct map_session_data *sd;
	struct status_change *sc = status_get_sc(bl);

	nullpo_ret(bl);
	sd = BL_CAST(BL_PC, bl);

	if (skill_id == SA_ABRACADABRA || skill_id == WM_RANDOMIZESPELL)
		return 0; //Will use picked skill's delay.

	if (bl->type&battle_config.no_skill_delay)
		return battle_config.min_skill_delay_limit;

	if (time < 0)
		time = -time + status_get_amotion(bl);	// If set to <0, add to attack motion.

	// Delay reductions
	switch (skill_id) {	//Monk combo skills have their delay reduced by agi/dex.
	case MO_TRIPLEATTACK:
	case MO_CHAINCOMBO:
	case MO_COMBOFINISH:
	case CH_TIGERFIST:
	case CH_CHAINCRUSH:
	case SR_DRAGONCOMBO:
	case SR_FALLENEMPIRE:
		time -= 4*status_get_agi(bl) - 2*status_get_dex(bl);
		break;
	case HP_BASILICA:
		if( sc && !sc->data[SC_BASILICA] )
			time = 0; // There is no Delay on Basilica creation, only on cancel
		break;
	default:
		if (battle_config.delay_dependon_dex && !(delaynodex&1))
		{	// if skill delay is allowed to be reduced by dex
			int scale = battle_config.castrate_dex_scale - status_get_dex(bl);
			if (scale > 0)
				time = time * scale / battle_config.castrate_dex_scale;
			else //To be capped later to minimum.
				time = 0;
		}
		if (battle_config.delay_dependon_agi && !(delaynodex&1))
		{	// if skill delay is allowed to be reduced by agi
			int scale = battle_config.castrate_dex_scale - status_get_agi(bl);
			if (scale > 0)
				time = time * scale / battle_config.castrate_dex_scale;
			else //To be capped later to minimum.
				time = 0;
		}
	}

	if ( sc && sc->data[SC_SPIRIT] )
	{
		switch (skill_id) {
			case CR_SHIELDBOOMERANG:
				if (sc->data[SC_SPIRIT]->val2 == SL_CRUSADER)
					time /= 2;
				break;
			case AS_SONICBLOW:
				if (!map_flag_gvg(bl->m) && !map[bl->m].flag.battleground && sc->data[SC_SPIRIT]->val2 == SL_ASSASIN)
					time /= 2;
				break;
		}
	}

	if (!(delaynodex&2))
	{
		if (sc && sc->count) {
			if (sc->data[SC_POEMBRAGI])
				time -= time * sc->data[SC_POEMBRAGI]->val3 / 100;
		}
	}

	if( !(delaynodex&4) && sd && sd->delayrate != 100 )
		time = time * sd->delayrate / 100;

	if (battle_config.delay_rate != 100)
		time = time * battle_config.delay_rate / 100;

	if (time < status_get_amotion(bl))
		time = status_get_amotion(bl); // Delay can never be below amotion [Playtester]

	return max(time, battle_config.min_skill_delay_limit);
}

/*=========================================
 *
 *-----------------------------------------*/
struct square {
	int val1[5];
	int val2[5];
};

static void skill_brandishspear_first (struct square *tc, int dir, int x, int y)
{
	nullpo_retv(tc);

	if(dir == 0){
		tc->val1[0]=x-2;
		tc->val1[1]=x-1;
		tc->val1[2]=x;
		tc->val1[3]=x+1;
		tc->val1[4]=x+2;
		tc->val2[0]=
		tc->val2[1]=
		tc->val2[2]=
		tc->val2[3]=
		tc->val2[4]=y-1;
	}
	else if(dir==2){
		tc->val1[0]=
		tc->val1[1]=
		tc->val1[2]=
		tc->val1[3]=
		tc->val1[4]=x+1;
		tc->val2[0]=y+2;
		tc->val2[1]=y+1;
		tc->val2[2]=y;
		tc->val2[3]=y-1;
		tc->val2[4]=y-2;
	}
	else if(dir==4){
		tc->val1[0]=x-2;
		tc->val1[1]=x-1;
		tc->val1[2]=x;
		tc->val1[3]=x+1;
		tc->val1[4]=x+2;
		tc->val2[0]=
		tc->val2[1]=
		tc->val2[2]=
		tc->val2[3]=
		tc->val2[4]=y+1;
	}
	else if(dir==6){
		tc->val1[0]=
		tc->val1[1]=
		tc->val1[2]=
		tc->val1[3]=
		tc->val1[4]=x-1;
		tc->val2[0]=y+2;
		tc->val2[1]=y+1;
		tc->val2[2]=y;
		tc->val2[3]=y-1;
		tc->val2[4]=y-2;
	}
	else if(dir==1){
		tc->val1[0]=x-1;
		tc->val1[1]=x;
		tc->val1[2]=x+1;
		tc->val1[3]=x+2;
		tc->val1[4]=x+3;
		tc->val2[0]=y-4;
		tc->val2[1]=y-3;
		tc->val2[2]=y-1;
		tc->val2[3]=y;
		tc->val2[4]=y+1;
	}
	else if(dir==3){
		tc->val1[0]=x+3;
		tc->val1[1]=x+2;
		tc->val1[2]=x+1;
		tc->val1[3]=x;
		tc->val1[4]=x-1;
		tc->val2[0]=y-1;
		tc->val2[1]=y;
		tc->val2[2]=y+1;
		tc->val2[3]=y+2;
		tc->val2[4]=y+3;
	}
	else if(dir==5){
		tc->val1[0]=x+1;
		tc->val1[1]=x;
		tc->val1[2]=x-1;
		tc->val1[3]=x-2;
		tc->val1[4]=x-3;
		tc->val2[0]=y+3;
		tc->val2[1]=y+2;
		tc->val2[2]=y+1;
		tc->val2[3]=y;
		tc->val2[4]=y-1;
	}
	else if(dir==7){
		tc->val1[0]=x-3;
		tc->val1[1]=x-2;
		tc->val1[2]=x-1;
		tc->val1[3]=x;
		tc->val1[4]=x+1;
		tc->val2[1]=y;
		tc->val2[0]=y+1;
		tc->val2[2]=y-1;
		tc->val2[3]=y-2;
		tc->val2[4]=y-3;
	}

}

static void skill_brandishspear_dir (struct square* tc, int dir, int are)
{
	int c;
	nullpo_retv(tc);

	for( c = 0; c < 5; c++ )
	{
		switch( dir )
		{
			case 0:                   tc->val2[c]+=are; break;
			case 1: tc->val1[c]-=are; tc->val2[c]+=are; break;
			case 2: tc->val1[c]-=are;                   break;
			case 3: tc->val1[c]-=are; tc->val2[c]-=are; break;
			case 4:                   tc->val2[c]-=are; break;
			case 5: tc->val1[c]+=are; tc->val2[c]-=are; break;
			case 6: tc->val1[c]+=are;                   break;
			case 7: tc->val1[c]+=are; tc->val2[c]+=are; break;
		}
	}
}

void skill_brandishspear(struct block_list* src, struct block_list* bl, int skillid, int skilllv, unsigned int tick, int flag)
{
	int c,n=4;
	int dir = map_calc_dir(src,bl->x,bl->y);
	struct square tc;
	int x=bl->x,y=bl->y;
	skill_brandishspear_first(&tc,dir,x,y);
	skill_brandishspear_dir(&tc,dir,4);
	skill_area_temp[1] = bl->id;

	if(skilllv > 9){
		for(c=1;c<4;c++){
			map_foreachincell(skill_area_sub,
				bl->m,tc.val1[c],tc.val2[c],BL_CHAR,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|n,
				skill_castend_damage_id);
		}
	}
	if(skilllv > 6){
		skill_brandishspear_dir(&tc,dir,-1);
		n--;
	}else{
		skill_brandishspear_dir(&tc,dir,-2);
		n-=2;
	}

	if(skilllv > 3){
		for(c=0;c<5;c++){
			map_foreachincell(skill_area_sub,
				bl->m,tc.val1[c],tc.val2[c],BL_CHAR,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|n,
				skill_castend_damage_id);
			if(skilllv > 6 && n==3 && c==4){
				skill_brandishspear_dir(&tc,dir,-1);
				n--;c=-1;
			}
		}
	}
	for(c=0;c<10;c++){
		if(c==0||c==5) skill_brandishspear_dir(&tc,dir,-1);
		map_foreachincell(skill_area_sub,
			bl->m,tc.val1[c%5],tc.val2[c%5],BL_CHAR,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
	}
}

/*==========================================
 * Weapon Repair [Celest/DracoRPG]
 *------------------------------------------*/
void skill_repairweapon (struct map_session_data *sd, int idx) {
	int material;
	int materials[4] = { 1002, 998, 999, 756 };
	struct item *item;
	struct map_session_data *target_sd;

	nullpo_retv(sd);

	if ( !( target_sd = map_id2sd(sd->menuskill_val) ) ) //Failed....
		return;
	
	if( idx == 0xFFFF ) // No item selected ('Cancel' clicked)
		return;
	if( idx < 0 || idx >= MAX_INVENTORY )
		return; //Invalid index??

	item = &target_sd->status.inventory[idx];
	if( item->nameid <= 0 || item->attribute == 0 )
		return; //Again invalid item....

	if( sd != target_sd && !battle_check_range(&sd->bl,&target_sd->bl, skill_get_range2(&sd->bl, sd->menuskill_id,sd->menuskill_val2) ) ){
		clif_item_repaireffect(sd,idx,1);
		return;
	}

	if ( target_sd->inventory_data[idx]->type == IT_WEAPON )
		material = materials [ target_sd->inventory_data[idx]->wlv - 1 ]; // Lv1/2/3/4 weapons consume 1 Iron Ore/Iron/Steel/Rough Oridecon
	else
		material = materials [2]; // Armors consume 1 Steel
	if ( pc_search_inventory(sd,material) < 0 ) {
		clif_skill_fail(sd,sd->menuskill_id,USESKILL_FAIL_LEVEL,0);
		return;
	}
	
	clif_skill_nodamage(&sd->bl,&target_sd->bl,sd->menuskill_id,1,1);
	
	item->attribute = 0;/* clear broken state */
	
	clif_equiplist(target_sd);
	
	pc_delitem(sd,pc_search_inventory(sd,material),1,0,0,LOG_TYPE_CONSUME);
	
	clif_item_repaireffect(sd,idx,0);
	
	if( sd != target_sd )
		clif_item_repaireffect(target_sd,idx,0);
}

/*==========================================
 * Item Appraisal
 *------------------------------------------*/
void skill_identify (struct map_session_data *sd, int idx)
{
	int flag=1;

	nullpo_retv(sd);

	if(idx >= 0 && idx < MAX_INVENTORY) {
		if(sd->status.inventory[idx].nameid > 0 && sd->status.inventory[idx].identify == 0 ){
			flag=0;
			sd->status.inventory[idx].identify=1;
		}
	}
	clif_item_identified(sd,idx,flag);
}

/*==========================================
 * Weapon Refine [Celest]
 *------------------------------------------*/
void skill_weaponrefine (struct map_session_data *sd, int idx)
{
	nullpo_retv(sd);

	if (idx >= 0 && idx < MAX_INVENTORY)
	{
		int i = 0, ep = 0, per;
		int material[5] = { 0, 1010, 1011, 984, 984 };
		struct item *item;
		struct item_data *ditem = sd->inventory_data[idx];
		item = &sd->status.inventory[idx];

		if(item->nameid > 0 && ditem->type == IT_WEAPON)
		{
			if( item->refine >= sd->menuskill_val
			||  item->refine >= 10		// if it's no longer refineable
			||  ditem->flag.no_refine 	// if the item isn't refinable
			||  (i = pc_search_inventory(sd, material [ditem->wlv])) < 0 )
			{
				clif_skill_fail(sd,sd->menuskill_id,USESKILL_FAIL_LEVEL,0);
				return;
			}

			per = status_get_refine_chance(ditem->wlv, (int)item->refine);
			per += (((signed int)sd->status.job_level)-50)/2; //Updated per the new kro descriptions. [Skotlex]

			pc_delitem(sd, i, 1, 0, 0, LOG_TYPE_OTHER);
			if (per > rnd() % 100) {
				log_pick_pc(sd, LOG_TYPE_OTHER, -1, item); 
				item->refine++;
				log_pick_pc(sd, LOG_TYPE_OTHER,  1, item);
				if(item->equip) {
					ep = item->equip;
					pc_unequipitem(sd,idx,3);
				}
				clif_refine(sd->fd,0,idx,item->refine);
				clif_delitem(sd,idx,1,3);
				clif_additem(sd,idx,1,0);
				if (ep)
					pc_equipitem(sd,idx,ep);
				clif_misceffect(&sd->bl,3);
				if(item->refine == 10 &&
					item->card[0] == CARD0_FORGE &&
					(int)MakeDWord(item->card[2],item->card[3]) == sd->status.char_id)
				{ // Fame point system [DracoRPG]
					switch(ditem->wlv){
						case 1:
							pc_addfame(sd,1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
							break;
						case 2:
							pc_addfame(sd,25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
							break;
						case 3:
							pc_addfame(sd,1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
							break;
					}
				}
			} else {
				item->refine = 0;
				if(item->equip)
					pc_unequipitem(sd,idx,3);
				clif_refine(sd->fd,1,idx,item->refine);
				pc_delitem(sd,idx,1,0,2, LOG_TYPE_OTHER);
				clif_misceffect(&sd->bl,2);
				clif_emotion(&sd->bl, E_OMG);
			}
		}
	}
}

/*==========================================
 *
 *------------------------------------------*/
int skill_autospell (struct map_session_data *sd, int skillid)
{
	int skilllv;
	int maxlv=1,lv;

	nullpo_ret(sd);

	skilllv = sd->menuskill_val;
	lv=pc_checkskill(sd,skillid);

	if(skilllv <= 0 || !lv) return 0; // Player must learn the skill before doing auto-spell [Lance]

	if(skillid==MG_NAPALMBEAT)	maxlv=3;
	else if(skillid==MG_COLDBOLT || skillid==MG_FIREBOLT || skillid==MG_LIGHTNINGBOLT){
		if (sd->sc.data[SC_SPIRIT] && sd->sc.data[SC_SPIRIT]->val2 == SL_SAGE)
			maxlv =10; //Soul Linker bonus. [Skotlex]
		else if(skilllv==2) maxlv=1;
		else if(skilllv==3) maxlv=2;
		else if(skilllv>=4) maxlv=3;
	}
	else if(skillid==MG_SOULSTRIKE){
		if(skilllv==5) maxlv=1;
		else if(skilllv==6) maxlv=2;
		else if(skilllv>=7) maxlv=3;
	}
	else if(skillid==MG_FIREBALL){
		if(skilllv==8) maxlv=1;
		else if(skilllv>=9) maxlv=2;
	}
	else if(skillid==MG_FROSTDIVER) maxlv=1;
	else return 0;

	if(maxlv > lv)
		maxlv = lv;

	sc_start4(&sd->bl,SC_AUTOSPELL,100,skilllv,skillid,maxlv,0,
		skill_get_time(SA_AUTOSPELL,skilllv));
	return 0;
}

/*==========================================
 * Sitting skills functions.
 *------------------------------------------*/
static int skill_sit_count (struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;
	int type =va_arg(ap,int);
	sd=(struct map_session_data*)bl;

	if(!pc_issit(sd))
		return 0;

	if(type&1 && pc_checkskill(sd,RG_GANGSTER) > 0)
		return 1;

	if(type&2 && (pc_checkskill(sd,TK_HPTIME) > 0 || pc_checkskill(sd,TK_SPTIME) > 0))
		return 1;

	return 0;
}

static int skill_sit_in (struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;
	int type =va_arg(ap,int);

	sd=(struct map_session_data*)bl;

	if(!pc_issit(sd))
		return 0;

	if(type&1 && pc_checkskill(sd,RG_GANGSTER) > 0)
		sd->state.gangsterparadise=1;

	if(type&2 && (pc_checkskill(sd,TK_HPTIME) > 0 || pc_checkskill(sd,TK_SPTIME) > 0 ))
	{
		sd->state.rest=1;
		status_calc_regen(bl, &sd->battle_status, &sd->regen);
		status_calc_regen_rate(bl, &sd->regen, &sd->sc);
	}

	return 0;
}

static int skill_sit_out (struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;
	int type =va_arg(ap,int);
	sd=(struct map_session_data*)bl;
	if(sd->state.gangsterparadise && type&1)
		sd->state.gangsterparadise=0;
	if(sd->state.rest && type&2) {
		sd->state.rest=0;
		status_calc_regen(bl, &sd->battle_status, &sd->regen);
		status_calc_regen_rate(bl, &sd->regen, &sd->sc);
	}
	return 0;
}

int skill_sit (struct map_session_data *sd, int type)
{
	int flag = 0;
	int range = 0, lv;
	nullpo_ret(sd);


	if((lv = pc_checkskill(sd,RG_GANGSTER)) > 0) {
		flag|=1;
		range = skill_get_splash(RG_GANGSTER, lv);
	}
	if((lv = pc_checkskill(sd,TK_HPTIME)) > 0) {
		flag|=2;
		range = skill_get_splash(TK_HPTIME, lv);
	}
	else if ((lv = pc_checkskill(sd,TK_SPTIME)) > 0) {
		flag|=2;
		range = skill_get_splash(TK_SPTIME, lv);
	}

	if( type ) {
		clif_status_load(&sd->bl,SI_SIT,1);
	} else {
		clif_status_load(&sd->bl,SI_SIT,0);
	}

	if (!flag) return 0;

	if(type) {
		if (map_foreachinrange(skill_sit_count,&sd->bl, range, BL_PC, flag) > 1)
			map_foreachinrange(skill_sit_in,&sd->bl, range, BL_PC, flag);
	} else {
		if (map_foreachinrange(skill_sit_count,&sd->bl, range, BL_PC, flag) < 2)
			map_foreachinrange(skill_sit_out,&sd->bl, range, BL_PC, flag);
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_frostjoke_scream (struct block_list *bl, va_list ap)
{
	struct block_list *src;
	int skillnum,skilllv;
	unsigned int tick;

	nullpo_ret(bl);
	nullpo_ret(src=va_arg(ap,struct block_list*));

	skillnum=va_arg(ap,int);
	skilllv=va_arg(ap,int);
	if(skilllv <= 0) return 0;
	tick=va_arg(ap,unsigned int);

	if (src == bl || status_isdead(bl))
		return 0;
	if (bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		if ( sd && sd->sc.option&(OPTION_INVISIBLE|OPTION_MADOGEAR) )
			return 0;//Frost Joke / Scream cannot target invisible or MADO Gear characters [Ind]
	}
	//It has been reported that Scream/Joke works the same regardless of woe-setting. [Skotlex]
	if(battle_check_target(src,bl,BCT_ENEMY) > 0)
		skill_additional_effect(src,bl,skillnum,skilllv,BF_MISC,ATK_DEF,tick);
	else if(battle_check_target(src,bl,BCT_PARTY) > 0 && rnd()%100 < 10)
		skill_additional_effect(src,bl,skillnum,skilllv,BF_MISC,ATK_DEF,tick);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static void skill_unitsetmapcell (struct skill_unit *src, int skill_num, int skill_lv, cell_t cell, bool flag)
{
	int range = skill_get_unit_range(skill_num,skill_lv);
	int x,y;

	for( y = src->bl.y - range; y <= src->bl.y + range; ++y )
		for( x = src->bl.x - range; x <= src->bl.x + range; ++x )
			map_setcell(src->bl.m, x, y, cell, flag);
}

/*==========================================
 *
 *------------------------------------------*/
int skill_attack_area (struct block_list *bl, va_list ap)
{
	struct block_list *src,*dsrc;
	int atk_type,skillid,skilllv,flag,type;
	unsigned int tick;

	if(status_isdead(bl))
		return 0;

	atk_type = va_arg(ap,int);
	src=va_arg(ap,struct block_list*);
	dsrc=va_arg(ap,struct block_list*);
	skillid=va_arg(ap,int);
	skilllv=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);
	flag=va_arg(ap,int);
	type=va_arg(ap,int);


	if (skill_area_temp[1] == bl->id) //This is the target of the skill, do a full attack and skip target checks.
		return skill_attack(atk_type,src,dsrc,bl,skillid,skilllv,tick,flag);

	if(battle_check_target(dsrc,bl,type) <= 0 ||
		!status_check_skilluse(NULL, bl, skillid, 2))
		return 0;


	switch (skillid) {
	case WZ_FROSTNOVA: //Skills that don't require the animation to be removed
	case NPC_ACIDBREATH:
	case NPC_DARKNESSBREATH:
	case NPC_FIREBREATH:
	case NPC_ICEBREATH:
	case NPC_THUNDERBREATH:
		return skill_attack(atk_type,src,dsrc,bl,skillid,skilllv,tick,flag);
	default:
		//Area-splash, disable skill animation.
		return skill_attack(atk_type,src,dsrc,bl,skillid,skilllv,tick,flag|SD_ANIMATION);
	}
}
/*==========================================
 *
 *------------------------------------------*/
int skill_clear_group (struct block_list *bl, int flag)
{
	struct unit_data *ud = unit_bl2ud(bl);
	struct skill_unit_group *group[MAX_SKILLUNITGROUP];
	int i, count=0;

	nullpo_ret(bl);
	if (!ud) return 0;

	//All groups to be deleted are first stored on an array since the array elements shift around when you delete them. [Skotlex]
	for (i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i];i++)
	{
		switch (ud->skillunit[i]->skill_id) {
			case SA_DELUGE:
			case SA_VOLCANO:
			case SA_VIOLENTGALE:
			case SA_LANDPROTECTOR:
			case NJ_SUITON:
			case NJ_KAENSIN:
				if (flag&1)
					group[count++]= ud->skillunit[i];
				break;
			case SO_CLOUD_KILL:
				if( flag&4 )
					group[count++]= ud->skillunit[i];
				break;
			case SO_WARMER:
				if( flag&8 )
					group[count++]= ud->skillunit[i];
				break;				
			default:
				if (flag&2 && skill_get_inf2(ud->skillunit[i]->skill_id)&INF2_TRAP)
					group[count++]= ud->skillunit[i];
				break;
		}

	}
	for (i=0;i<count;i++)
		skill_delunitgroup(group[i]);
	return count;
}

/*==========================================
 * Returns the first element field found [Skotlex]
 *------------------------------------------*/
struct skill_unit_group *skill_locate_element_field(struct block_list *bl)
{
	struct unit_data *ud = unit_bl2ud(bl);
	int i;
	nullpo_ret(bl);
	if (!ud) return NULL;

	for (i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i];i++) {
		switch (ud->skillunit[i]->skill_id) {
			case SA_DELUGE:
			case SA_VOLCANO:
			case SA_VIOLENTGALE:
			case SA_LANDPROTECTOR:
			case NJ_SUITON:
			case SO_WARMER:
			case SO_CLOUD_KILL:				
				return ud->skillunit[i];
		}
	}
	return NULL;
}

// for graffiti cleaner [Valaris]
int skill_graffitiremover (struct block_list *bl, va_list ap)
{
	struct skill_unit *unit=NULL;

	nullpo_ret(bl);
	nullpo_ret(ap);

	if(bl->type!=BL_SKILL || (unit=(struct skill_unit *)bl) == NULL)
		return 0;

	if((unit->group) && (unit->group->unit_id == UNT_GRAFFITI))
		skill_delunit(unit);

	return 0;
}

int skill_greed (struct block_list *bl, va_list ap)
{
	struct block_list *src;
	struct map_session_data *sd=NULL;
	struct flooritem_data *fitem=NULL;

	nullpo_ret(bl);
	nullpo_ret(src = va_arg(ap, struct block_list *));

	if(src->type == BL_PC && (sd=(struct map_session_data *)src) && bl->type==BL_ITEM && (fitem=(struct flooritem_data *)bl))
		pc_takeitem(sd, fitem);

	return 0;
}
//For Ranger's Detonator [Jobbie/3CeAM]
int skill_detonator(struct block_list *bl, va_list ap)
{
	struct skill_unit *unit=NULL;
	struct block_list *src;
	int unit_id;

	nullpo_ret(bl);
	nullpo_ret(ap);
	src = va_arg(ap,struct block_list *);

	if( bl->type != BL_SKILL || (unit = (struct skill_unit *)bl) == NULL || !unit->group )
		return 0;
	if( unit->group->src_id != src->id )
		return 0;

	unit_id = unit->group->unit_id;
	switch( unit_id )
	{ //List of Hunter and Ranger Traps that can be detonate.
		case UNT_BLASTMINE:
		case UNT_SANDMAN:
		case UNT_CLAYMORETRAP:
		case UNT_TALKIEBOX:
		case UNT_CLUSTERBOMB:
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
			if( unit_id == UNT_TALKIEBOX )
			{
				clif_talkiebox(bl,unit->group->valstr);
				unit->group->val2 = -1;
			}
			else
				map_foreachinrange(skill_trap_splash,bl,skill_get_splash(unit->group->skill_id,unit->group->skill_lv),unit->group->bl_flag,bl,unit->group->tick);

			clif_changetraplook(bl,unit_id == UNT_FIRINGTRAP ? UNT_DUMMYSKILL : UNT_USED_TRAPS);
			unit->group->unit_id = UNT_USED_TRAPS;
			unit->group->limit = DIFF_TICK(gettick(),unit->group->tick) + 
				(unit_id == UNT_TALKIEBOX ? 5000 : (unit_id == UNT_CLUSTERBOMB || unit_id == UNT_ICEBOUNDTRAP? 2500 : 1500) );
			break;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_cell_overlap(struct block_list *bl, va_list ap)
{
	int skillid;
	int *alive;
	struct skill_unit *unit;

	skillid = va_arg(ap,int);
	alive = va_arg(ap,int *);
	unit = (struct skill_unit *)bl;

	if (unit == NULL || unit->group == NULL || (*alive) == 0)
		return 0;

	switch (skillid) {
		case SA_LANDPROTECTOR:
			if( unit->group->skill_id == SA_LANDPROTECTOR ) {//Check for offensive Land Protector to delete both. [Skotlex]
				(*alive) = 0;
				skill_delunit(unit);
				return 1;
			}
			if( !(skill_get_inf2(unit->group->skill_id)&(INF2_SONG_DANCE|INF2_TRAP)) ) { //It deletes everything except songs/dances and traps
				skill_delunit(unit);
				return 1;
			}
			break;
		case HW_GANBANTEIN:
		case LG_EARTHDRIVE:
			if( !(unit->group->state.song_dance&0x1) ) {// Don't touch song/dance.
				skill_delunit(unit);
				return 1;
			}
			break;
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
// The official implementation makes them fail to appear when casted on top of ANYTHING
// but I wonder if they didn't actually meant to fail when casted on top of each other?
// hence, I leave the alternate implementation here, commented. [Skotlex]
			if (unit->range <= 0)
			{
				(*alive) = 0;
				return 1;
			}
/*
			switch (unit->group->skill_id)
			{	//These cannot override each other.
				case SA_VOLCANO:
				case SA_DELUGE:
				case SA_VIOLENTGALE:
					(*alive) = 0;
					return 1;
			}
*/
			break;
		case PF_FOGWALL:
			switch(unit->group->skill_id) {
				case SA_VOLCANO: //Can't be placed on top of these
				case SA_VIOLENTGALE:
					(*alive) = 0;
					return 1;
				case SA_DELUGE:
				case NJ_SUITON:
				//Cheap 'hack' to notify the calling function that duration should be doubled [Skotlex]
					(*alive) = 2;
					break;
			}
			break;
		case HP_BASILICA:
			if (unit->group->skill_id == HP_BASILICA)
			{	//Basilica can't be placed on top of itself to avoid map-cell stacking problems. [Skotlex]
				(*alive) = 0;
				return 1;
			}
			break;
		case GN_CRAZYWEED_ATK:
			switch(unit->group->unit_id){ //TODO: look for other ground skills that are affected.
				case UNT_WALLOFTHORN:
				case UNT_THORNS_TRAP:
				case UNT_BLOODYLUST:
				case UNT_CHAOSPANIC:
				case UNT_MAELSTROM:
				case UNT_FIREPILLAR_ACTIVE:
				case UNT_LANDPROTECTOR:
				case UNT_VOLCANO:
				case UNT_DELUGE:
				case UNT_VIOLENTGALE:
				case UNT_SAFETYWALL:
				case UNT_PNEUMA:
					skill_delunit(unit);
					return 1;
			}
			break;
	}

	if (unit->group->skill_id == SA_LANDPROTECTOR && !(skill_get_inf2(skillid)&(INF2_SONG_DANCE|INF2_TRAP))) {	//It deletes everything except songs/dances/traps
		(*alive) = 0;
		return 1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_chastle_mob_changetarget(struct block_list *bl,va_list ap)
{
	struct mob_data* md;
	struct unit_data*ud = unit_bl2ud(bl);
	struct block_list *from_bl;
	struct block_list *to_bl;
	md = (struct mob_data*)bl;
	from_bl = va_arg(ap,struct block_list *);
	to_bl = va_arg(ap,struct block_list *);

	if(ud && ud->target == from_bl->id)
		ud->target = to_bl->id;

	if(md->bl.type == BL_MOB && md->target_id == from_bl->id)
		md->target_id = to_bl->id;
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
static int skill_trap_splash (struct block_list *bl, va_list ap)
{
	struct block_list *src;
	int tick;
	struct skill_unit *unit;
	struct skill_unit_group *sg;
	struct block_list *ss;
	src = va_arg(ap,struct block_list *);
	unit = (struct skill_unit *)src;
	tick = va_arg(ap,int);

	if( !unit->alive || bl->prev == NULL )
		return 0;

	nullpo_ret(sg = unit->group);
	nullpo_ret(ss = map_id2bl(sg->src_id));

	if(battle_check_target(src,bl,sg->target_flag) <= 0)
		return 0;

	switch(sg->unit_id){
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
			skill_additional_effect(ss,bl,sg->skill_id,sg->skill_lv,BF_MISC,ATK_DEF,tick);
			break;
		case UNT_GROUNDDRIFT_WIND:
			if(skill_attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(bl,SC_STUN,5,sg->skill_lv,skill_get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_DARK:
			if(skill_attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(bl,SC_BLIND,5,sg->skill_lv,skill_get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_POISON:
			if(skill_attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(bl,SC_POISON,5,sg->skill_lv,skill_get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_WATER:
			if(skill_attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(bl,SC_FREEZE,5,sg->skill_lv,skill_get_time2(sg->skill_id, sg->skill_lv));
			break;
		case UNT_GROUNDDRIFT_FIRE:
			if(skill_attack(BF_WEAPON,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				skill_blown(src,bl,skill_get_blewcount(sg->skill_id,sg->skill_lv),-1,0);
			break;
		case UNT_ELECTRICSHOCKER:
			clif_skill_damage(src,bl,tick,0,0,-30000,1,sg->skill_id,sg->skill_lv,5);
			break;
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
		case UNT_CLUSTERBOMB:
			if( ss != bl )
				skill_attack(BF_MISC,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1|SD_LEVEL);
			break;
		case UNT_MAGENTATRAP:
		case UNT_COBALTTRAP:
		case UNT_MAIZETRAP:
		case UNT_VERDURETRAP:
			if( bl->type != BL_PC && !is_boss(bl) )
				sc_start2(bl,SC_ELEMENTALCHANGE,100,sg->skill_lv,skill_get_ele(sg->skill_id,sg->skill_lv),skill_get_time2(sg->skill_id,sg->skill_lv));
			break;
		case UNT_REVERBERATION:
			skill_addtimerskill(ss,tick+50,bl->id,0,0,WM_REVERBERATION_MELEE,sg->skill_lv,BF_WEAPON,0); // for proper skill delay animation when use with Dominion Impulse
			skill_addtimerskill(ss,tick+250,bl->id,0,0,WM_REVERBERATION_MAGIC,sg->skill_lv,BF_MAGIC,0);
			break;
		default:
			skill_attack(skill_get_type(sg->skill_id),ss,src,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;
	}
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_enchant_elemental_end (struct block_list *bl, int type)
{
	struct status_change *sc;
	const enum sc_type scs[] = { SC_ENCPOISON, SC_ASPERSIO, SC_FIREWEAPON, SC_WATERWEAPON, SC_WINDWEAPON, SC_EARTHWEAPON, SC_SHADOWWEAPON, SC_GHOSTWEAPON, SC_ENCHANTARMS, SC_EXEEDBREAK };
	int i;
	nullpo_ret(bl);
	nullpo_ret(sc= status_get_sc(bl));

	if (!sc->count) return 0;

	for (i = 0; i < ARRAYLENGTH(scs); i++)
		if (type != scs[i] && sc->data[scs[i]])
			status_change_end(bl, scs[i], INVALID_TIMER);

	return 0;
}

bool skill_check_cloaking(struct block_list *bl, struct status_change_entry *sce)
{
	static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
	static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
	bool wall = true;

	if( (bl->type == BL_PC && battle_config.pc_cloak_check_type&1)
	||	(bl->type != BL_PC && battle_config.monster_cloak_check_type&1) )
	{	//Check for walls.
		int i;
		ARR_FIND( 0, 8, i, map_getcell(bl->m, bl->x+dx[i], bl->y+dy[i], CELL_CHKNOPASS) != 0 );
		if( i == 8 )
			wall = false;
	}

	if( sce )
	{
		if( !wall )
		{
			if( sce->val1 < 3 ) //End cloaking.
				status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
			else
			if( sce->val4&1 )
			{	//Remove wall bonus
				sce->val4&=~1;
				status_calc_bl(bl,SCB_SPEED);
			}
		}
		else
		{
			if( !(sce->val4&1) )
			{	//Add wall speed bonus
				sce->val4|=1;
				status_calc_bl(bl,SCB_SPEED);
			}
		}
	}

	return wall;
}
bool skill_check_camouflage(struct block_list *bl, struct status_change_entry *sce)
{
	static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
	static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
	bool wall = true;

	if( bl->type == BL_PC )
	{	//Check for walls.
		int i;
		ARR_FIND( 0, 8, i, map_getcell(bl->m, bl->x+dx[i], bl->y+dy[i], CELL_CHKNOPASS) != 0 );
		if( i == 8 )
			wall = false;
	}
		
	if( sce )
	{
		if( !wall )
		{
			if( sce->val1 < 3 ) //End camouflage.
				status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
			else
			if( sce->val3&1 )
			{	//Remove wall bonus
				sce->val3&=~1;
				status_calc_bl(bl,SCB_SPEED);
			}
		}
	}

	return wall;
}

/*==========================================
 *
 *------------------------------------------*/
struct skill_unit *skill_initunit (struct skill_unit_group *group, int idx, int x, int y, int val1, int val2)
{
	struct skill_unit *unit;

	nullpo_retr(NULL, group);
	nullpo_retr(NULL, group->unit); // crash-protection against poor coding
	nullpo_retr(NULL, unit=&group->unit[idx]);

	if(!unit->alive)
		group->alive_count++;

	unit->bl.id=map_get_new_object_id();
	unit->bl.type=BL_SKILL;
	unit->bl.m=group->map;
	unit->bl.x=x;
	unit->bl.y=y;
	unit->group=group;
	unit->alive=1;
	unit->val1=val1;
	unit->val2=val2;

	idb_put(skillunit_db, unit->bl.id, unit);
	map_addiddb(&unit->bl);
	map_addblock(&unit->bl);

	// perform oninit actions
	switch (group->skill_id) {
		case WZ_ICEWALL:
			map_setgatcell(unit->bl.m,unit->bl.x,unit->bl.y,5);
			clif_changemapcell(0,unit->bl.m,unit->bl.x,unit->bl.y,5,AREA);
			skill_unitsetmapcell(unit,WZ_ICEWALL,group->skill_lv,CELL_ICEWALL,true);
			map[unit->bl.m].icewall_num++;
			break;
		case SA_LANDPROTECTOR:
			skill_unitsetmapcell(unit,SA_LANDPROTECTOR,group->skill_lv,CELL_LANDPROTECTOR,true);
			break;
		case HP_BASILICA:
			skill_unitsetmapcell(unit,HP_BASILICA,group->skill_lv,CELL_BASILICA,true);
			break;
		case SC_MAELSTROM:
			skill_unitsetmapcell(unit,SC_MAELSTROM,group->skill_lv,CELL_MAELSTROM,true);
			break;
		default:
			if (group->state.song_dance&0x1) //Check for dissonance.
				skill_dance_overlap(unit, 1);
			break;
	}

	clif_skill_setunit(unit);

	return unit;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_delunit (struct skill_unit* unit)
{
	struct skill_unit_group *group;

	nullpo_ret(unit);
	if( !unit->alive )
		return 0;
	unit->alive=0;

	nullpo_ret(group=unit->group);

	if( group->state.song_dance&0x1 ) //Cancel dissonance effect.
		skill_dance_overlap(unit, 0);

	// invoke onout event
	if( !unit->range )
		map_foreachincell(skill_unit_effect,unit->bl.m,unit->bl.x,unit->bl.y,group->bl_flag,&unit->bl,gettick(),4);

	// perform ondelete actions
	switch (group->skill_id) {
		case HT_ANKLESNARE: {
				struct block_list* target = map_id2bl(group->val2);
				if( target )
					status_change_end(target, SC_ANKLE, INVALID_TIMER);
			}
			break;
		case WZ_ICEWALL:
			map_setgatcell(unit->bl.m,unit->bl.x,unit->bl.y,unit->val2);
			clif_changemapcell(0,unit->bl.m,unit->bl.x,unit->bl.y,unit->val2,ALL_SAMEMAP); // hack to avoid clientside cell bug
			skill_unitsetmapcell(unit,WZ_ICEWALL,group->skill_lv,CELL_ICEWALL,false);
			map[unit->bl.m].icewall_num--;
			break;
		case SA_LANDPROTECTOR:
			skill_unitsetmapcell(unit,SA_LANDPROTECTOR,group->skill_lv,CELL_LANDPROTECTOR,false);
			break;
		case HP_BASILICA:
			skill_unitsetmapcell(unit,HP_BASILICA,group->skill_lv,CELL_BASILICA,false);
			break;
		case RA_ELECTRICSHOCKER: {
				struct block_list* target = map_id2bl(group->val2);
				if( target )
					status_change_end(target, SC_ELECTRICSHOCKER, INVALID_TIMER);	
			}
			break;
		case SC_MAELSTROM:
			skill_unitsetmapcell(unit,SC_MAELSTROM,group->skill_lv,CELL_MAELSTROM,false);
			break;
		case SC_MANHOLE: // Note : Removing the unit don't remove the status (official info)
			if( group->val2 ) { // Someone Traped
				struct status_change *tsc = status_get_sc( map_id2bl(group->val2));
				if( tsc && tsc->data[SC__MANHOLE] )
					tsc->data[SC__MANHOLE]->val4 = 0; // Remove the Unit ID
			}
			break;
	}

	clif_skill_delunit(unit);

	unit->group=NULL;
	map_delblock(&unit->bl); // don't free yet
	map_deliddb(&unit->bl);
	idb_remove(skillunit_db, unit->bl.id);
	if(--group->alive_count==0)
		skill_delunitgroup(group);

	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
static DBMap* group_db = NULL;// int group_id -> struct skill_unit_group*

/// Returns the target skill_unit_group or NULL if not found.
struct skill_unit_group* skill_id2group(int group_id)
{
	return (struct skill_unit_group*)idb_get(group_db, group_id);
}


static int skill_unit_group_newid = MAX_SKILL_DB;

/// Returns a new group_id that isn't being used in group_db.
/// Fatal error if nothing is available.
static int skill_get_new_group_id(void)
{
	if( skill_unit_group_newid >= MAX_SKILL_DB && skill_id2group(skill_unit_group_newid) == NULL )
		return skill_unit_group_newid++;// available
	{// find next id
		int base_id = skill_unit_group_newid;
		while( base_id != ++skill_unit_group_newid )
		{
			if( skill_unit_group_newid < MAX_SKILL_DB )
				skill_unit_group_newid = MAX_SKILL_DB;
			if( skill_id2group(skill_unit_group_newid) == NULL )
				return skill_unit_group_newid++;// available
		}
		// full loop, nothing available
		ShowFatalError("skill_get_new_group_id: All ids are taken. Exiting...");
		exit(1);
	}
}

struct skill_unit_group* skill_initunitgroup (struct block_list* src, int count, short skillid, short skilllv, int unit_id, int limit, int interval)
{
	struct unit_data* ud = unit_bl2ud( src );
	struct skill_unit_group* group;
	int i;

	if(skillid <= 0 || skilllv <= 0) return 0;

	nullpo_retr(NULL, src);
	nullpo_retr(NULL, ud);

	// find a free spot to store the new unit group
	ARR_FIND( 0, MAX_SKILLUNITGROUP, i, ud->skillunit[i] == NULL );
	if(i == MAX_SKILLUNITGROUP)
	{
		// array is full, make room by discarding oldest group
		int j=0;
		unsigned maxdiff=0,x,tick=gettick();
		for(i=0;i<MAX_SKILLUNITGROUP && ud->skillunit[i];i++)
			if((x=DIFF_TICK(tick,ud->skillunit[i]->tick))>maxdiff){
				maxdiff=x;
				j=i;
			}
		skill_delunitgroup(ud->skillunit[j]);
		//Since elements must have shifted, we use the last slot.
		i = MAX_SKILLUNITGROUP-1;
	}

	group             = ers_alloc(skill_unit_ers, struct skill_unit_group);
	group->src_id     = src->id;
	group->party_id   = status_get_party_id(src);
	group->guild_id   = status_get_guild_id(src);
	group->bg_id      = bg_team_get_id(src);
	group->group_id   = skill_get_new_group_id();
	group->unit       = (struct skill_unit *)aCalloc(count,sizeof(struct skill_unit));
	group->unit_count = count;
	group->alive_count = 0;
	group->val1       = 0;
	group->val2       = 0;
	group->val3       = 0;
	group->skill_id   = skillid;
	group->skill_lv   = skilllv;
	group->unit_id    = unit_id;
	group->map        = src->m;
	group->limit      = limit;
	group->interval   = interval;
	group->tick       = gettick();
	group->valstr     = NULL;

	ud->skillunit[i] = group;

	if (skillid == PR_SANCTUARY) //Sanctuary starts healing +1500ms after casted. [Skotlex]
		group->tick += 1500;

	idb_put(group_db, group->group_id, group);
	return group;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_delunitgroup_(struct skill_unit_group *group, const char* file, int line, const char* func)
{
	struct block_list* src;
	struct unit_data *ud;
	int i,j;
	
	if( group == NULL )
	{
		ShowDebug("skill_delunitgroup: group is NULL (source=%s:%d, %s)! Please report this! (#3504)\n", file, line, func);
		return 0;
	}

	src=map_id2bl(group->src_id);
	ud = unit_bl2ud(src);
	if(!src || !ud) {
		ShowError("skill_delunitgroup: Group's source not found! (src_id: %d skill_id: %d)\n", group->src_id, group->skill_id);
		return 0;
	}

	if( !status_isdead(src) && ((TBL_PC*)src)->state.warping && !((TBL_PC*)src)->state.changemap ) {
		switch( group->skill_id ) {
			case BA_DISSONANCE:
			case BA_POEMBRAGI:
			case BA_WHISTLE:
			case BA_ASSASSINCROSS:
			case BA_APPLEIDUN:
			case DC_UGLYDANCE:
			case DC_HUMMING:
			case DC_DONTFORGETME:
			case DC_FORTUNEKISS:
			case DC_SERVICEFORYOU:
				skill_usave_add(((TBL_PC*)src), group->skill_id, group->skill_lv);
				break;
		}
	}

	if (skill_get_unit_flag(group->skill_id)&(UF_DANCE|UF_SONG|UF_ENSEMBLE))
	{
		struct status_change* sc = status_get_sc(src);
		if (sc && sc->data[SC_DANCING])
		{
			sc->data[SC_DANCING]->val2 = 0 ; //This prevents status_change_end attempting to redelete the group. [Skotlex]
			status_change_end(src, SC_DANCING, INVALID_TIMER);
		}
	}

	// end Gospel's status change on 'src'
	// (needs to be done when the group is deleted by other means than skill deactivation)
	if (group->unit_id == UNT_GOSPEL) {
		struct status_change *sc = status_get_sc(src);
		if(sc && sc->data[SC_GOSPEL]) {
			sc->data[SC_GOSPEL]->val3 = 0; //Remove reference to this group. [Skotlex]
			status_change_end(src, SC_GOSPEL, INVALID_TIMER);
		}
	}

	switch( group->skill_id ) {
		case SG_SUN_WARM:
		case SG_MOON_WARM:
		case SG_STAR_WARM:
			{
				struct status_change *sc = NULL;
				if( (sc = status_get_sc(src)) != NULL  && sc->data[SC_WARM] ) {
					sc->data[SC_WARM]->val4 = 0;
					status_change_end(src, SC_WARM, INVALID_TIMER);
				}
			}
			break;
		case NC_NEUTRALBARRIER:
			{
				struct status_change *sc = NULL;
				if( (sc = status_get_sc(src)) != NULL && sc->data[SC_NEUTRALBARRIER_MASTER] ) {
					sc->data[SC_NEUTRALBARRIER_MASTER]->val2 = 0;
					status_change_end(src,SC_NEUTRALBARRIER_MASTER,INVALID_TIMER);
				}
			}
			break;
		case NC_STEALTHFIELD:
			{
				struct status_change *sc = NULL;
				if( (sc = status_get_sc(src)) != NULL && sc->data[SC_STEALTHFIELD_MASTER] ) {
					sc->data[SC_STEALTHFIELD_MASTER]->val2 = 0;
					status_change_end(src,SC_STEALTHFIELD_MASTER,INVALID_TIMER);
				}
			}
			break;
		case LG_BANDING:
			{
				struct status_change *sc = NULL;
				if( (sc = status_get_sc(src)) && sc->data[SC_BANDING] ) {
					sc->data[SC_BANDING]->val4 = 0;
					status_change_end(src,SC_BANDING,INVALID_TIMER);
				}
			}
			break;
	}

	if (src->type==BL_PC && group->state.ammo_consume)
		battle_consume_ammo((TBL_PC*)src, group->skill_id, group->skill_lv);

	group->alive_count=0;

	// remove all unit cells
	if(group->unit != NULL)
		for( i = 0; i < group->unit_count; i++ )
			skill_delunit(&group->unit[i]);

	// clear Talkie-box string
	if( group->valstr != NULL )
	{
		aFree(group->valstr);
		group->valstr = NULL;
	}

	idb_remove(group_db, group->group_id);
	map_freeblock(&group->unit->bl); // schedules deallocation of whole array (HACK)
	group->unit=NULL;
	group->group_id=0;
	group->unit_count=0;

	// locate this group, swap with the last entry and delete it
	ARR_FIND( 0, MAX_SKILLUNITGROUP, i, ud->skillunit[i] == group );
	ARR_FIND( i, MAX_SKILLUNITGROUP, j, ud->skillunit[j] == NULL ); j--;
	if( i < MAX_SKILLUNITGROUP )
	{
		ud->skillunit[i] = ud->skillunit[j];
		ud->skillunit[j] = NULL;
		ers_free(skill_unit_ers, group);
	}
	else
		ShowError("skill_delunitgroup: Group not found! (src_id: %d skill_id: %d)\n", group->src_id, group->skill_id);

	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_clear_unitgroup (struct block_list *src)
{
	struct unit_data *ud = unit_bl2ud(src);

	nullpo_ret(ud);

	while (ud->skillunit[0])
		skill_delunitgroup(ud->skillunit[0]);

	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
struct skill_unit_group_tickset *skill_unitgrouptickset_search (struct block_list *bl, struct skill_unit_group *group, int tick)
{
	int i,j=-1,k,s,id;
	struct unit_data *ud;
	struct skill_unit_group_tickset *set;

	nullpo_ret(bl);
	if (group->interval==-1)
		return NULL;

	ud = unit_bl2ud(bl);
	if (!ud) return NULL;

	set = ud->skillunittick;

	if (skill_get_unit_flag(group->skill_id)&UF_NOOVERLAP)
		id = s = group->skill_id;
	else
		id = s = group->group_id;

	for (i=0; i<MAX_SKILLUNITGROUPTICKSET; i++) {
		k = (i+s) % MAX_SKILLUNITGROUPTICKSET;
		if (set[k].id == id)
			return &set[k];
		else if (j==-1 && (DIFF_TICK(tick,set[k].tick)>0 || set[k].id==0))
			j=k;
	}

	if (j == -1) {
		ShowWarning ("skill_unitgrouptickset_search: tickset is full\n");
		j = id % MAX_SKILLUNITGROUPTICKSET;
	}

	set[j].id = id;
	set[j].tick = tick;
	return &set[j];
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_timer_sub_onplace (struct block_list* bl, va_list ap)
{
	struct skill_unit* unit = va_arg(ap,struct skill_unit *);
	struct skill_unit_group* group = unit->group;
	unsigned int tick = va_arg(ap,unsigned int);

	if( !unit->alive || bl->prev == NULL )
		return 0;

	nullpo_ret(group);

	if( !(skill_get_inf2(group->skill_id)&(INF2_SONG_DANCE|INF2_TRAP|INF2_NOLP)) && map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR) )
		return 0; //AoE skills are ineffective. [Skotlex]

	if( battle_check_target(&unit->bl,bl,group->target_flag) <= 0 )
		return 0;

	skill_unit_onplace_timer(unit,bl,tick);

	return 1;
}

/**
 * @see DBApply
 */
static int skill_unit_timer_sub(DBKey key, DBData *data, va_list ap)
{
	struct skill_unit* unit = db_data2ptr(data);
	struct skill_unit_group* group = unit->group;
	unsigned int tick = va_arg(ap,unsigned int);
  	bool dissonance;
	struct block_list* bl = &unit->bl;

	if( !unit->alive )
		return 0;

	nullpo_ret(group);

	// check for expiration
	if( !group->state.guildaura && (DIFF_TICK(tick,group->tick) >= group->limit || DIFF_TICK(tick,group->tick) >= unit->limit) )
	{// skill unit expired (inlined from skill_unit_onlimit())
		switch( group->unit_id )
		{
			case UNT_BLASTMINE:
#ifdef RENEWAL
			case UNT_CLAYMORETRAP:
#endif
			case UNT_GROUNDDRIFT_WIND:
			case UNT_GROUNDDRIFT_DARK:
			case UNT_GROUNDDRIFT_POISON:
			case UNT_GROUNDDRIFT_WATER:
			case UNT_GROUNDDRIFT_FIRE:
				group->unit_id = UNT_USED_TRAPS;
				//clif_changetraplook(bl, UNT_FIREPILLAR_ACTIVE);
				group->limit=DIFF_TICK(tick+1500,group->tick);
				unit->limit=DIFF_TICK(tick+1500,group->tick);
			break;

			case UNT_ANKLESNARE:
			case UNT_ELECTRICSHOCKER:
				if( group->val2 > 0 ) {
					// Used Trap don't returns back to item
					skill_delunit(unit);
					break;
				}
			case UNT_SKIDTRAP:
			case UNT_LANDMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
#ifndef RENEWAL
			case UNT_CLAYMORETRAP:
#endif
			case UNT_TALKIEBOX:
			case UNT_CLUSTERBOMB:
			case UNT_MAGENTATRAP:
			case UNT_COBALTTRAP:
			case UNT_MAIZETRAP:
			case UNT_VERDURETRAP:
			case UNT_FIRINGTRAP:
			case UNT_ICEBOUNDTRAP:

			{
				struct block_list* src;
				if( unit->val1 > 0 && (src = map_id2bl(group->src_id)) != NULL && src->type == BL_PC )
				{ // revert unit back into a trap
					struct item item_tmp;
					memset(&item_tmp,0,sizeof(item_tmp));
					item_tmp.nameid = group->item_id?group->item_id:ITEMID_TRAP;
					item_tmp.identify = 1;
					map_addflooritem(&item_tmp,1,bl->m,bl->x,bl->y,0,0,0,0);
				}
				skill_delunit(unit);
			}
			break;

			case UNT_WARP_ACTIVE:
				// warp portal opens (morph to a UNT_WARP_WAITING cell)
				group->unit_id = skill_get_unit_id(group->skill_id, 1); // UNT_WARP_WAITING
				clif_changelook(&unit->bl, LOOK_BASE, group->unit_id);
				// restart timers
				group->limit = skill_get_time(group->skill_id,group->skill_lv);
				unit->limit = skill_get_time(group->skill_id,group->skill_lv);
				// apply effect to all units standing on it
				map_foreachincell(skill_unit_effect,unit->bl.m,unit->bl.x,unit->bl.y,group->bl_flag,&unit->bl,gettick(),1);
			break;

			case UNT_CALLFAMILY:
			{
				struct map_session_data *sd = NULL;
				if(group->val1) {
		  			sd = map_charid2sd(group->val1);
					group->val1 = 0;
					if (sd && !map[sd->bl.m].flag.nowarp)
						pc_setpos(sd,map_id2index(unit->bl.m),unit->bl.x,unit->bl.y,CLR_TELEPORT);
				}
				if(group->val2) {
					sd = map_charid2sd(group->val2);
					group->val2 = 0;
					if (sd && !map[sd->bl.m].flag.nowarp)
						pc_setpos(sd,map_id2index(unit->bl.m),unit->bl.x,unit->bl.y,CLR_TELEPORT);
				}
				skill_delunit(unit);
			}
			break;

			case UNT_REVERBERATION:
				if( unit->val1 <= 0 ) { // If it was deactivated.
					skill_delunit(unit);
					break;
				}
				clif_changetraplook(bl,UNT_USED_TRAPS);
				map_foreachinrange(skill_trap_splash, bl, skill_get_splash(group->skill_id, group->skill_lv), group->bl_flag, bl, tick);
				group->limit = DIFF_TICK(tick,group->tick)+1000;
				unit->limit = DIFF_TICK(tick,group->tick)+1000;
				group->unit_id = UNT_USED_TRAPS;
			break;

			case UNT_FEINTBOMB: {
				struct block_list *src =  map_id2bl(group->src_id);
				if( src )
					map_foreachinrange(skill_area_sub, &group->unit->bl, unit->range, splash_target(src), src, SC_FEINTBOMB, group->skill_lv, tick, BCT_ENEMY|1, skill_castend_damage_id);
				skill_delunit(unit);
				break;
			}
			
			case UNT_BANDING:
			{
				struct block_list *src = map_id2bl(group->src_id);
				struct status_change *sc;
				if( !src || (sc = status_get_sc(src)) == NULL || !sc->data[SC_BANDING] )
				{
					skill_delunit(unit);
					break;
				}
				// This unit isn't removed while SC_BANDING is active.
				group->limit = DIFF_TICK(tick+group->interval,group->tick);
				unit->limit = DIFF_TICK(tick+group->interval,group->tick);
			}
			break;

			default:
				skill_delunit(unit);
		}
	}
	else
	{// skill unit is still active
		switch( group->unit_id )
		{
			case UNT_ICEWALL:
				// icewall loses 50 hp every second
				unit->val1 -= SKILLUNITTIMER_INTERVAL/20; // trap's hp
				if( unit->val1 <= 0 && unit->limit + group->tick > tick + 700 )
					unit->limit = DIFF_TICK(tick+700,group->tick);
				break;
			case UNT_BLASTMINE:
			case UNT_SKIDTRAP:
			case UNT_LANDMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_CLAYMORETRAP:
			case UNT_FREEZINGTRAP:
			case UNT_TALKIEBOX:
			case UNT_ANKLESNARE:
				if( unit->val1 <= 0 ) {
					if( group->unit_id == UNT_ANKLESNARE && group->val2 > 0 )
						skill_delunit(unit);
					else {
						clif_changetraplook(bl, group->unit_id==UNT_LANDMINE?UNT_FIREPILLAR_ACTIVE:UNT_USED_TRAPS);
						group->limit = DIFF_TICK(tick, group->tick) + 1500;
						group->unit_id = UNT_USED_TRAPS;
					}
				}
				break;
			case UNT_REVERBERATION:
				if( unit->val1 <= 0 ){
					clif_changetraplook(bl,UNT_USED_TRAPS);
					map_foreachinrange(skill_trap_splash, bl, skill_get_splash(group->skill_id, group->skill_lv), group->bl_flag, bl, tick);
					group->limit = DIFF_TICK(tick,group->tick)+1000;
					unit->limit = DIFF_TICK(tick,group->tick)+1000;
					group->unit_id = UNT_USED_TRAPS;
				}
				break;
			case UNT_WALLOFTHORN:
				if( unit->val1 <= 0 ) {
					group->unit_id = UNT_USED_TRAPS;
					group->limit = DIFF_TICK(tick, group->tick) + 1500;
				}
				break;
		}
	}

	//Don't continue if unit or even group is expired and has been deleted.
	if( !group || !unit->alive )
		return 0;

	dissonance = skill_dance_switch(unit, 0);

	if( unit->range >= 0 && group->interval != -1 )
	{
		if( battle_config.skill_wall_check )
			map_foreachinshootrange(skill_unit_timer_sub_onplace, bl, unit->range, group->bl_flag, bl,tick);
		else
			map_foreachinrange(skill_unit_timer_sub_onplace, bl, unit->range, group->bl_flag, bl,tick);

		if(unit->range == -1) //Unit disabled, but it should not be deleted yet.
			group->unit_id = UNT_USED_TRAPS;

		if( group->unit_id == UNT_TATAMIGAESHI )
		{
			unit->range = -1; //Disable processed cell.
			if (--group->val1 <= 0) // number of live cells
	  		{	//All tiles were processed, disable skill.
				group->target_flag=BCT_NOONE;
				group->bl_flag= BL_NUL;
			}
		}
	}

  	if( dissonance ) skill_dance_switch(unit, 1);

	return 0;
}
/*==========================================
 * Executes on all skill units every SKILLUNITTIMER_INTERVAL miliseconds.
 *------------------------------------------*/
int skill_unit_timer(int tid, unsigned int tick, int id, intptr_t data)
{
	map_freeblock_lock();

	skillunit_db->foreach(skillunit_db, skill_unit_timer_sub, tick);

	map_freeblock_unlock();

	return 0;
}

static int skill_unit_temp[20];  // temporary storage for tracking skill unit skill ids as players move in/out of them
/*==========================================
 *
 *------------------------------------------*/
int skill_unit_move_sub (struct block_list* bl, va_list ap)
{
	struct skill_unit* unit = (struct skill_unit *)bl;
	struct skill_unit_group* group = unit->group;

	struct block_list* target = va_arg(ap,struct block_list*);
	unsigned int tick = va_arg(ap,unsigned int);
	int flag = va_arg(ap,int);

	bool dissonance;
	int skill_id;
	int i;

	nullpo_ret(group);

	if( !unit->alive || target->prev == NULL )
		return 0;

	if( flag&1 && ( unit->group->skill_id == PF_SPIDERWEB || unit->group->skill_id == GN_THORNS_TRAP ) )
		return 0; // Fiberlock is never supposed to trigger on skill_unit_move. [Inkfish]

	dissonance = skill_dance_switch(unit, 0);

	//Necessary in case the group is deleted after calling on_place/on_out [Skotlex]
	skill_id = unit->group->skill_id;

	if( unit->group->interval != -1 && !(skill_get_unit_flag(skill_id)&UF_DUALMODE) && skill_id != BD_LULLABY ) //Lullaby is the exception, bugreport:411
	{	//Non-dualmode unit skills with a timer don't trigger when walking, so just return
		if( dissonance ) skill_dance_switch(unit, 1);
		return 0;
	}

	//Target-type check.
	if( !(group->bl_flag&target->type && battle_check_target(&unit->bl,target,group->target_flag) > 0) )
	{
		if( group->src_id == target->id && group->state.song_dance&0x2 )
		{	//Ensemble check to see if they went out/in of the area [Skotlex]
			if( flag&1 )
			{
				if( flag&2 )
				{	//Clear this skill id.
					ARR_FIND( 0, ARRAYLENGTH(skill_unit_temp), i, skill_unit_temp[i] == skill_id );
					if( i < ARRAYLENGTH(skill_unit_temp) )
						skill_unit_temp[i] = 0;
				}
			}
			else
			{
				if( flag&2 )
				{	//Store this skill id.
					ARR_FIND( 0, ARRAYLENGTH(skill_unit_temp), i, skill_unit_temp[i] == 0 );
					if( i < ARRAYLENGTH(skill_unit_temp) )
						skill_unit_temp[i] = skill_id;
					else
						ShowError("skill_unit_move_sub: Reached limit of unit objects per cell!\n");
				}

			}

			if( flag&4 )
				skill_unit_onleft(skill_id,target,tick);
		}

		if( dissonance ) skill_dance_switch(unit, 1);

		return 0;
	}
	else
	{
		if( flag&1 )
		{
			int result = skill_unit_onplace(unit,target,tick);
			if( flag&2 && result )
			{	//Clear skill ids we have stored in onout.
				ARR_FIND( 0, ARRAYLENGTH(skill_unit_temp), i, skill_unit_temp[i] == result );
				if( i < ARRAYLENGTH(skill_unit_temp) )
					skill_unit_temp[i] = 0;
			}
		}
		else
		{
			int result = skill_unit_onout(unit,target,tick);
			if( flag&2 && result )
			{	//Store this unit id.
				ARR_FIND( 0, ARRAYLENGTH(skill_unit_temp), i, skill_unit_temp[i] == 0 );
				if( i < ARRAYLENGTH(skill_unit_temp) )
					skill_unit_temp[i] = skill_id;
				else
					ShowError("skill_unit_move_sub: Reached limit of unit objects per cell!\n");
			}
		}

		//TODO: Normally, this is dangerous since the unit and group could be freed
		//inside the onout/onplace functions. Currently it is safe because we know song/dance
		//cells do not get deleted within them. [Skotlex]
		if( dissonance ) skill_dance_switch(unit, 1);

		if( flag&4 )
			skill_unit_onleft(skill_id,target,tick);

		return 1;
	}
}

/*==========================================
 * Invoked when a char has moved and unit cells must be invoked (onplace, onout, onleft)
 * Flag values:
 * flag&1: invoke skill_unit_onplace (otherwise invoke skill_unit_onout)
 * flag&2: this function is being invoked twice as a bl moves, store in memory the affected
 * units to figure out when they have left a group.
 * flag&4: Force a onleft event (triggered when the bl is killed, for example)
 *------------------------------------------*/
int skill_unit_move (struct block_list *bl, unsigned int tick, int flag)
{
	nullpo_ret(bl);

	if( bl->prev == NULL )
		return 0;

	if( flag&2 && !(flag&1) )
	{	//Onout, clear data
		memset(skill_unit_temp, 0, sizeof(skill_unit_temp));
	}

	map_foreachincell(skill_unit_move_sub,bl->m,bl->x,bl->y,BL_SKILL,bl,tick,flag);

	if( flag&2 && flag&1 )
	{	//Onplace, check any skill units you have left.
		int i;
		for( i = 0; i < ARRAYLENGTH(skill_unit_temp); i++ )
			if( skill_unit_temp[i] )
				skill_unit_onleft(skill_unit_temp[i], bl, tick);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_unit_move_unit_group (struct skill_unit_group *group, int m, int dx, int dy)
{
	int i,j;
	unsigned int tick = gettick();
	int *m_flag;
	struct skill_unit *unit1;
	struct skill_unit *unit2;

	if (group == NULL)
		return 0;
	if (group->unit_count<=0)
		return 0;
	if (group->unit==NULL)
		return 0;

	if (skill_get_unit_flag(group->skill_id)&UF_ENSEMBLE)
		return 0; //Ensembles may not be moved around.

	if( group->unit_id == UNT_ICEWALL || group->unit_id == UNT_WALLOFTHORN )
		return 0; //Icewalls and Wall of Thorns don't get knocked back
	
	m_flag = (int *) aCalloc(group->unit_count, sizeof(int));
	//    m_flag
	//		0: Neither of the following (skill_unit_onplace & skill_unit_onout are needed)
	//		1: Unit will move to a slot that had another unit of the same group (skill_unit_onplace not needed)
	//		2: Another unit from same group will end up positioned on this unit (skill_unit_onout not needed)
	//		3: Both 1+2.
	for(i=0;i<group->unit_count;i++){
		unit1=&group->unit[i];
		if (!unit1->alive || unit1->bl.m!=m)
			continue;
		for(j=0;j<group->unit_count;j++){
			unit2=&group->unit[j];
			if (!unit2->alive)
				continue;
			if (unit1->bl.x+dx==unit2->bl.x && unit1->bl.y+dy==unit2->bl.y){
				m_flag[i] |= 0x1;
			}
			if (unit1->bl.x-dx==unit2->bl.x && unit1->bl.y-dy==unit2->bl.y){
				m_flag[i] |= 0x2;
			}
		}
	}
	j = 0;
	for (i=0;i<group->unit_count;i++) {
		unit1=&group->unit[i];
		if (!unit1->alive)
			continue;
		if (!(m_flag[i]&0x2)) {
			if (group->state.song_dance&0x1) //Cancel dissonance effect.
				skill_dance_overlap(unit1, 0);
			map_foreachincell(skill_unit_effect,unit1->bl.m,unit1->bl.x,unit1->bl.y,group->bl_flag,&unit1->bl,tick,4);
		}
		//Move Cell using "smart" criteria (avoid useless moving around)
		switch(m_flag[i])
		{
			case 0:
			//Cell moves independently, safely move it.
				map_moveblock(&unit1->bl, unit1->bl.x+dx, unit1->bl.y+dy, tick);
				break;
			case 1:
			//Cell moves unto another cell, look for a replacement cell that won't collide
			//and has no cell moving into it (flag == 2)
				for(;j<group->unit_count;j++)
				{
					if(m_flag[j]!=2 || !group->unit[j].alive)
						continue;
					//Move to where this cell would had moved.
					unit2 = &group->unit[j];
					map_moveblock(&unit1->bl, unit2->bl.x+dx, unit2->bl.y+dy, tick);
					j++; //Skip this cell as we have used it.
					break;
				}
				break;
			case 2:
			case 3:
				break; //Don't move the cell as a cell will end on this tile anyway.
		}
		if (!(m_flag[i]&0x2)) { //We only moved the cell in 0-1
			if (group->state.song_dance&0x1) //Check for dissonance effect.
				skill_dance_overlap(unit1, 1);
			clif_skill_setunit(unit1);
			map_foreachincell(skill_unit_effect,unit1->bl.m,unit1->bl.x,unit1->bl.y,group->bl_flag,&unit1->bl,tick,1);
		}
	}
	aFree(m_flag);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_can_produce_mix (struct map_session_data *sd, int nameid, int trigger, int qty)
{
	int i,j;

	nullpo_ret(sd);

	if(nameid<=0)
		return 0;

	for(i=0;i<MAX_SKILL_PRODUCE_DB;i++){
		if(skill_produce_db[i].nameid == nameid ){
			if((j=skill_produce_db[i].req_skill)>0 &&
				pc_checkskill(sd,j) < skill_produce_db[i].req_skill_lv)
					continue; // must iterate again to check other skills that produce it. [malufett]
			if( j > 0 && sd->menuskill_id > 0 && sd->menuskill_id != j )
				continue; // special case
			break;
		}
	}
	
	if( i >= MAX_SKILL_PRODUCE_DB )
		return 0;

	if( pc_checkadditem(sd, nameid, qty) == ADDITEM_OVERAMOUNT )
	{// cannot carry the produced stuff
		return 0;
	}

	if(trigger>=0){
		if(trigger>20) { // Non-weapon, non-food item (itemlv must match)
			if(skill_produce_db[i].itemlv!=trigger)
				return 0;
		} else if(trigger>10) { // Food (any item level between 10 and 20 will do)
			if(skill_produce_db[i].itemlv<=10 || skill_produce_db[i].itemlv>20)
				return 0;
		} else { // Weapon (itemlv must be higher or equal)
			if(skill_produce_db[i].itemlv>trigger)
				return 0;
		}
	}

	for(j=0;j<MAX_PRODUCE_RESOURCE;j++){
		int id,x,y;
		if( (id=skill_produce_db[i].mat_id[j]) <= 0 )
			continue;
		if(skill_produce_db[i].mat_amount[j] <= 0) {
			if(pc_search_inventory(sd,id) < 0)
				return 0;
		}
		else {
			for(y=0,x=0;y<MAX_INVENTORY;y++)
				if( sd->status.inventory[y].nameid == id )
					x+=sd->status.inventory[y].amount;
			if(x<qty*skill_produce_db[i].mat_amount[j])
				return 0;
		}
	}
	return i+1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_produce_mix (struct map_session_data *sd, int skill_id, int nameid, int slot1, int slot2, int slot3, int qty)
{
	int slot[3];
	int i,sc,ele,idx,equip,wlv,make_per = 0,flag = 0,skilllv = 0;
	int num = -1; // exclude the recipe
	struct status_data *status;
	struct item_data* data;

	nullpo_ret(sd);
	status = status_get_status_data(&sd->bl);
	
	if( sd->skillid_old == skill_id )
		skilllv = sd->skilllv_old;

	if( !(idx=skill_can_produce_mix(sd,nameid,-1, qty)) )
		return 0;
	idx--;

	if (qty < 1)
		qty = 1;

	if (!skill_id) //A skill can be specified for some override cases.
		skill_id = skill_produce_db[idx].req_skill;

	if( skill_id == GC_RESEARCHNEWPOISON )
		skill_id = GC_CREATENEWPOISON;

	slot[0]=slot1;
	slot[1]=slot2;
	slot[2]=slot3;

	for(i=0,sc=0,ele=0;i<3;i++){ //Note that qty should always be one if you are using these!
		int j;
		if( slot[i]<=0 )
			continue;
		j = pc_search_inventory(sd,slot[i]);
		if(j < 0)
			continue;
		if(slot[i]==1000){	/* Star Crumb */
			pc_delitem(sd,j,1,1,0,LOG_TYPE_PRODUCE);
			sc++;
		}
		if(slot[i]>=994 && slot[i]<=997 && ele==0){	/* Flame Heart . . . Great Nature */
			static const int ele_table[4]={3,1,4,2};
			pc_delitem(sd,j,1,1,0,LOG_TYPE_PRODUCE);
			ele=ele_table[slot[i]-994];
		}
	}

	if( skill_id == RK_RUNEMASTERY ) {
		int temp_qty, skill_lv = pc_checkskill(sd,skill_id);
		data = itemdb_search(nameid);
		
		if( skill_lv == 10 ) temp_qty = 1 + rnd()%3;
		else if( skill_lv > 5 ) temp_qty = 1 + rnd()%2;
		else temp_qty = 1;
		
		if (data->stack.inventory) {
			for( i = 0; i < MAX_INVENTORY; i++ ) {
				if( sd->status.inventory[i].nameid == nameid ) {
					if( sd->status.inventory[i].amount >= data->stack.amount ) {
						clif_msgtable(sd->fd,0x61b);
						return 0;
					} else {
						/**
						 * the amount fits, say we got temp_qty 4 and 19 runes, we trim temp_qty to 1.
						 **/
						if( temp_qty + sd->status.inventory[i].amount >= data->stack.amount )
							temp_qty = data->stack.amount - sd->status.inventory[i].amount;
					}
					break;
				}
			}
		}
		qty = temp_qty;
	}

	for(i=0;i<MAX_PRODUCE_RESOURCE;i++){
		int j,id,x;
		if( (id=skill_produce_db[idx].mat_id[i]) <= 0 )
			continue;
		num++;
		x=( skill_id == RK_RUNEMASTERY ? 1 : qty)*skill_produce_db[idx].mat_amount[i];
		do{
			int y=0;
			j = pc_search_inventory(sd,id);

			if(j >= 0){
				y = sd->status.inventory[j].amount;
				if(y>x)y=x;
				pc_delitem(sd,j,y,0,0,LOG_TYPE_PRODUCE);
			} else
				ShowError("skill_produce_mix: material item error\n");

			x-=y;
		}while( j>=0 && x>0 );
	}

	if( (equip = (itemdb_isequip(nameid) && skill_id != GN_CHANGEMATERIAL && skill_id != GN_MAKEBOMB )) )
		wlv = itemdb_wlv(nameid);
	if(!equip) {
		switch(skill_id){
			case BS_IRON:
			case BS_STEEL:
			case BS_ENCHANTEDSTONE:
				// Ores & Metals Refining - skill bonuses are straight from kRO website [DracoRPG]
				i = pc_checkskill(sd,skill_id);
				make_per = sd->status.job_level*20 + status->dex*10 + status->luk*10; //Base chance
				switch(nameid){
					case 998: // Iron
						make_per += 4000+i*500; // Temper Iron bonus: +26/+32/+38/+44/+50
						break;
					case 999: // Steel
						make_per += 3000+i*500; // Temper Steel bonus: +35/+40/+45/+50/+55
						break;
					case 1000: //Star Crumb
						make_per = 100000; // Star Crumbs are 100% success crafting rate? (made 1000% so it succeeds even after penalties) [Skotlex]
						break;
					default: // Enchanted Stones
						make_per += 1000+i*500; // Enchantedstone Craft bonus: +15/+20/+25/+30/+35
					break;
				}
				break;
			case ASC_CDP:
				make_per = (2000 + 40*status->dex + 20*status->luk);
				break;
			case AL_HOLYWATER:
			/**
			 * Arch Bishop
			 **/
			case AB_ANCILLA:
				make_per = 100000; //100% success
				break;
			case AM_PHARMACY: // Potion Preparation - reviewed with the help of various Ragnainfo sources [DracoRPG]
			case AM_TWILIGHT1:
			case AM_TWILIGHT2:
			case AM_TWILIGHT3:
				make_per = pc_checkskill(sd,AM_LEARNINGPOTION)*50
					+ pc_checkskill(sd,AM_PHARMACY)*300 + sd->status.job_level*20
					+ (status->int_/2)*10 + status->dex*10+status->luk*10;
				if(merc_is_hom_active(sd->hd)) {//Player got a homun
					int skill;
					if((skill=merc_hom_checkskill(sd->hd,HVAN_INSTRUCT)) > 0) //His homun is a vanil with instruction change
						make_per += skill*100; //+1% bonus per level
				}
				switch(nameid){
					case 501: // Red Potion
					case 503: // Yellow Potion
					case 504: // White Potion
						make_per += (1+rnd()%100)*10 + 2000;
						break;
					case 970: // Alcohol
						make_per += (1+rnd()%100)*10 + 1000;
						break;
					case 7135: // Bottle Grenade
					case 7136: // Acid Bottle
					case 7137: // Plant Bottle
					case 7138: // Marine Sphere Bottle
						make_per += (1+rnd()%100)*10;
						break;
					case 546: // Condensed Yellow Potion
						make_per -= (1+rnd()%50)*10;
						break;
					case 547: // Condensed White Potion
					case 7139: // Glistening Coat
						make_per -= (1+rnd()%100)*10;
					    break;
					//Common items, recieve no bonus or penalty, listed just because they are commonly produced
					case 505: // Blue Potion
					case 545: // Condensed Red Potion
					case 605: // Anodyne
					case 606: // Aloevera
					default:
						break;
				}
				if(battle_config.pp_rate != 100)
					make_per = make_per * battle_config.pp_rate / 100;
				break;
			case SA_CREATECON: // Elemental Converter Creation
				make_per = 100000; // should be 100% success rate
				break;
			/**
			 * Rune Knight
			 **/
			case RK_RUNEMASTERY:
				make_per = 5 * (sd->itemid + pc_checkskill(sd,skill_id)) * 100;
				break;
			/**
			 * Guilotine Cross
			 **/
			case GC_CREATENEWPOISON:
				make_per = 3000 + 500 * pc_checkskill(sd,GC_RESEARCHNEWPOISON);
				qty = 1+rnd()%pc_checkskill(sd,GC_RESEARCHNEWPOISON);
				break;
			case GN_CHANGEMATERIAL: 
				for(i=0; i<MAX_SKILL_PRODUCE_DB; i++)
					if( skill_changematerial_db[i].itemid == nameid ){
						make_per = skill_changematerial_db[i].rate * 10;
						break;
					}
				break;	
			case GN_S_PHARMACY:
				{
					int difficulty = 0;
					
					difficulty = (620 - 20 * skilllv);// (620 - 20 * Skill Level)

					make_per = status->int_ + status->dex/2 + status->luk + sd->status.job_level + (30+rnd()%120) + // (Caster痴 INT) + (Caster痴 DEX / 2) + (Caster痴 LUK) + (Caster痴 Job Level) + Random number between (30 ~ 150) +
								(sd->status.base_level-100) + pc_checkskill(sd, AM_LEARNINGPOTION) + pc_checkskill(sd, CR_FULLPROTECTION)*(4+rnd()%6); // (Caster痴 Base Level - 100) + (Potion Research x 5) + (Full Chemical Protection Skill Level) x (Random number between 4 ~ 10)
					
					switch(nameid){// difficulty factor
						case 12422:	case 12425:	
						case 12428:
							difficulty += 10;
							break;
						case 6212:	case 12426:
							difficulty += 15;
							break;
						case 13264:	case 12423:	
						case 12427:	case 12436:
							difficulty += 20;
							break;
						case 6210:	case 6211:	
						case 12437:
							difficulty += 30;
							break;
						case 12424:	case 12475:
							difficulty += 40;
							break;
					}

					if( make_per >= 400 && make_per > difficulty)
						qty = 10;
					else if( make_per >= 300 && make_per > difficulty)
						qty = 7;
					else if( make_per >= 100 && make_per > difficulty)
						qty = 6;
					else if( make_per >= 1 && make_per > difficulty)
						qty = 5;
					else
						qty = 4;
					make_per = 10000;
				}
				break;
			case GN_MAKEBOMB:
			case GN_MIX_COOKING:
				{
					int difficulty = 30 + rnd()%120; // Random number between (30 ~ 150)
				
					make_per = sd->status.job_level / 4 + status->luk / 2 + status->dex / 3; // (Caster痴 Job Level / 4) + (Caster痴 LUK / 2) + (Caster痴 DEX / 3)
					qty = ~(5 + rnd()%5) + 1;

					switch(nameid){// difficulty factor
						case 13260:
							difficulty += 5;
							break;
						case 13261:	case 13262:
							difficulty += 10;
							break;
						case 12429:	case 12430:	case 12431:
						case 12432:	case 12433:	case 12434:
						case 13263:
							difficulty += 15;
							break;
						case 13264:	
							difficulty += 20;
							break;
					}

					if( make_per >= 30 && make_per > difficulty)
						qty = 10 + rnd()%2;
					else if( make_per >= 10 && make_per > difficulty)
						qty = 10;
					else if( make_per == 10 && make_per > difficulty)
						qty = 8;
					else if( (make_per >= 50 || make_per < 30) && make_per < difficulty)
						;// Food/Bomb creation fails.
					else if( make_per >= 30 && make_per < difficulty)
						qty = 5;
		
					if( qty < 0 || (skilllv == 1 && make_per < difficulty)){
						qty = ~qty + 1;
						make_per = 0;
					}else
						make_per = 10000;
					qty = (skilllv > 1 ? qty : 1);
				}
				break;
			default:
				if (sd->menuskill_id ==	AM_PHARMACY &&
					sd->menuskill_val > 10 && sd->menuskill_val <= 20)
				{	//Assume Cooking Dish
					if (sd->menuskill_val >= 15) //Legendary Cooking Set.
						make_per = 10000; //100% Success
					else
						make_per = 1200 * (sd->menuskill_val - 10)
							+ 20  * (sd->status.base_level + 1)
							+ 20  * (status->dex + 1)
							+ 100 * (rnd()%(30+5*(sd->cook_mastery/400) - (6+sd->cook_mastery/80)) + (6+sd->cook_mastery/80))
							- 400 * (skill_produce_db[idx].itemlv - 11 + 1)
							- 10  * (100 - status->luk + 1)
							- 500 * (num - 1)
							- 100 * (rnd()%4 + 1);
					break;
				}
				make_per = 5000;
				break;
		}
	} else { // Weapon Forging - skill bonuses are straight from kRO website, other things from a jRO calculator [DracoRPG]
		make_per = 5000 + sd->status.job_level*20 + status->dex*10 + status->luk*10; // Base
		make_per += pc_checkskill(sd,skill_id)*500; // Smithing skills bonus: +5/+10/+15
		make_per += pc_checkskill(sd,BS_WEAPONRESEARCH)*100 +((wlv >= 3)? pc_checkskill(sd,BS_ORIDEOCON)*100:0); // Weaponry Research bonus: +1/+2/+3/+4/+5/+6/+7/+8/+9/+10, Oridecon Research bonus (custom): +1/+2/+3/+4/+5
		make_per -= (ele?2000:0) + sc*1500 + (wlv>1?wlv*1000:0); // Element Stone: -20%, Star Crumb: -15% each, Weapon level malus: -0/-20/-30
		if(pc_search_inventory(sd,989) > 0) make_per+= 1000; // Emperium Anvil: +10
		else if(pc_search_inventory(sd,988) > 0) make_per+= 500; // Golden Anvil: +5
		else if(pc_search_inventory(sd,987) > 0) make_per+= 300; // Oridecon Anvil: +3
		else if(pc_search_inventory(sd,986) > 0) make_per+= 0; // Anvil: +0?
		if(battle_config.wp_rate != 100)
			make_per = make_per * battle_config.wp_rate / 100;
	}

	if (sd->class_&JOBL_BABY) //if it's a Baby Class
		make_per = (make_per * 50) / 100; //Baby penalty is 50% (bugreport:4847)

	if(make_per < 1) make_per = 1;


	if(rnd()%10000 < make_per || qty > 1){ //Success, or crafting multiple items.
		struct item tmp_item;
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid=nameid;
		tmp_item.amount=1;
		tmp_item.identify=1;
		if(equip){
			tmp_item.card[0]=CARD0_FORGE;
			tmp_item.card[1]=((sc*5)<<8)+ele;
			tmp_item.card[2]=GetWord(sd->status.char_id,0); // CharId
			tmp_item.card[3]=GetWord(sd->status.char_id,1);
		} else {
			//Flag is only used on the end, so it can be used here. [Skotlex]
			switch (skill_id) {
				case BS_DAGGER:
				case BS_SWORD:
				case BS_TWOHANDSWORD:
				case BS_AXE:
				case BS_MACE:
				case BS_KNUCKLE:
				case BS_SPEAR:
					flag = battle_config.produce_item_name_input&0x1;
					break;
				case AM_PHARMACY:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:			
					flag = battle_config.produce_item_name_input&0x2;
					break;
				case AL_HOLYWATER:
				/**
				 * Arch Bishop
				 **/
				case AB_ANCILLA:
					flag = battle_config.produce_item_name_input&0x8;
					break;
				case ASC_CDP:
					flag = battle_config.produce_item_name_input&0x10;
					break;
				default:
					flag = battle_config.produce_item_name_input&0x80;
					break;
			}
			if (flag) {
				tmp_item.card[0]=CARD0_CREATE;
				tmp_item.card[1]=0;
				tmp_item.card[2]=GetWord(sd->status.char_id,0); // CharId
				tmp_item.card[3]=GetWord(sd->status.char_id,1);
			}
		}

//		if(log_config.produce > 0)
//			log_produce(sd,nameid,slot1,slot2,slot3,1);
//TODO update PICKLOG

		if(equip){
			clif_produceeffect(sd,0,nameid);
			clif_misceffect(&sd->bl,3);
			if(itemdb_wlv(nameid) >= 3 && ((ele? 1 : 0) + sc) >= 3) // Fame point system [DracoRPG]
				pc_addfame(sd,10); // Success to forge a lv3 weapon with 3 additional ingredients = +10 fame point
		} else {
			int fame = 0;
			tmp_item.amount = 0;

			for (i=0; i< qty; i++) {	//Apply quantity modifiers.
				if( (skill_id == GN_MIX_COOKING || skill_id == GN_MAKEBOMB || skill_id == GN_S_PHARMACY) && make_per > 1){
					tmp_item.amount = qty;
					break;
				}
				if (rnd()%10000 < make_per || qty == 1) { //Success
					tmp_item.amount++;
					if(nameid < 545 || nameid > 547)
						continue;
					if( skill_id != AM_PHARMACY &&
						skill_id != AM_TWILIGHT1 &&
						skill_id != AM_TWILIGHT2 &&
						skill_id != AM_TWILIGHT3 )
						continue;
					//Add fame as needed.
					switch(++sd->potion_success_counter) {
						case 3:
							fame+=1; // Success to prepare 3 Condensed Potions in a row
							break;
						case 5:
							fame+=3; // Success to prepare 5 Condensed Potions in a row
							break;
						case 7:
							fame+=10; // Success to prepare 7 Condensed Potions in a row
							break;
						case 10:
							fame+=50; // Success to prepare 10 Condensed Potions in a row
							sd->potion_success_counter = 0;
							break;
					}
				} else //Failure
					sd->potion_success_counter = 0;
			}

			if (fame)
				pc_addfame(sd,fame);
			//Visual effects and the like.
			switch (skill_id) {
				case AM_PHARMACY:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
				case ASC_CDP:
					clif_produceeffect(sd,2,nameid);
					clif_misceffect(&sd->bl,5);
					break;
				case BS_IRON:
				case BS_STEEL:
				case BS_ENCHANTEDSTONE:
					clif_produceeffect(sd,0,nameid);
					clif_misceffect(&sd->bl,3);
					break;
				case RK_RUNEMASTERY:
				case GC_CREATENEWPOISON:
					clif_produceeffect(sd,2,nameid);
					clif_misceffect(&sd->bl,5);
					break;	
				default: //Those that don't require a skill?
					if( skill_produce_db[idx].itemlv > 10 && skill_produce_db[idx].itemlv <= 20)
					{ //Cooking items.
						clif_specialeffect(&sd->bl, 608, AREA);
						if( sd->cook_mastery < 1999 )
							pc_setglobalreg(sd, "COOK_MASTERY",sd->cook_mastery + ( 1 << ( (skill_produce_db[idx].itemlv - 11) / 2 ) ) * 5);
					}
					break;
			}
		}
		if ( skill_id == GN_CHANGEMATERIAL && tmp_item.amount) { //Success
			int j, k = 0;
			for(i=0; i<MAX_SKILL_PRODUCE_DB; i++)
				if( skill_changematerial_db[i].itemid == nameid ){
					for(j=0; j<5; j++){
						if( rnd()%1000 < skill_changematerial_db[i].qty_rate[j] ){
							tmp_item.amount = qty * skill_changematerial_db[i].qty[j];
							if((flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
								clif_additem(sd,0,0,flag);
								map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
							}
							k++;
						}
					}
					break;
				}
			if( k ){
				clif_msg_skill(sd,skill_id,0x627);	
				return 1;
			}
		} else if (tmp_item.amount) { //Success
			if((flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
				clif_additem(sd,0,0,flag);
				map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
			if( skill_id == GN_MIX_COOKING || skill_id == GN_MAKEBOMB || skill_id ==  GN_S_PHARMACY )
				clif_msg_skill(sd,skill_id,0x627);	
			return 1;
		}
	}
	//Failure
//	if(log_config.produce)
//		log_produce(sd,nameid,slot1,slot2,slot3,0);
//TODO update PICKLOG

	if(equip){
		clif_produceeffect(sd,1,nameid);
		clif_misceffect(&sd->bl,2);
	} else {
		switch (skill_id) {
			case ASC_CDP: //25% Damage yourself, and display same effect as failed potion.
				status_percent_damage(NULL, &sd->bl, -25, 0, true);
			case AM_PHARMACY:
			case AM_TWILIGHT1:
			case AM_TWILIGHT2:
			case AM_TWILIGHT3:
				clif_produceeffect(sd,3,nameid);
				clif_misceffect(&sd->bl,6);
				sd->potion_success_counter = 0; // Fame point system [DracoRPG]
				break;
			case BS_IRON:
			case BS_STEEL:
			case BS_ENCHANTEDSTONE:
				clif_produceeffect(sd,1,nameid);
				clif_misceffect(&sd->bl,2);
				break;
			case RK_RUNEMASTERY:
			case GC_CREATENEWPOISON:
				clif_produceeffect(sd,3,nameid);
				clif_misceffect(&sd->bl,6);
				break;
			case GN_MIX_COOKING: {
					struct item tmp_item;
					const int compensation[5] = {13265, 13266, 13267, 12435, 13268};
					int rate = rnd()%500;
					memset(&tmp_item,0,sizeof(tmp_item));
					if( rate < 50) i = 4;
					else if( rate < 100) i = 2+rnd()%1;
					else if( rate < 250 ) i = 1;
					else if( rate < 500 ) i = 0;
					tmp_item.nameid = compensation[i];
					tmp_item.amount = qty;
					tmp_item.identify = 1;
					if( pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE) ) {
						clif_additem(sd,0,0,flag);
						map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
					}
					clif_msg_skill(sd,skill_id,0x628);
				}
				break;
			case GN_MAKEBOMB:
			case GN_S_PHARMACY:
			case GN_CHANGEMATERIAL:
				clif_msg_skill(sd,skill_id,0x628);
				break;				
			default:
				if( skill_produce_db[idx].itemlv > 10 && skill_produce_db[idx].itemlv <= 20 )
				{ //Cooking items.
					clif_specialeffect(&sd->bl, 609, AREA);
					if( sd->cook_mastery > 0 )
						pc_setglobalreg(sd, "COOK_MASTERY", sd->cook_mastery - ( 1 << ((skill_produce_db[idx].itemlv - 11) / 2) ) - ( ( ( 1 << ((skill_produce_db[idx].itemlv - 11) / 2) ) >> 1 ) * 3 ));
				}
		}
	}
	return 0;
}

int skill_arrow_create (struct map_session_data *sd, int nameid)
{
	int i,j,flag,index=-1;
	struct item tmp_item;

	nullpo_ret(sd);

	if(nameid <= 0)
		return 1;

	for(i=0;i<MAX_SKILL_ARROW_DB;i++)
		if(nameid == skill_arrow_db[i].nameid) {
			index = i;
			break;
		}

	if(index < 0 || (j = pc_search_inventory(sd,nameid)) < 0)
		return 1;

	pc_delitem(sd,j,1,0,0,LOG_TYPE_PRODUCE);
	for(i=0;i<MAX_ARROW_RESOURCE;i++) {
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.identify = 1;
		tmp_item.nameid = skill_arrow_db[index].cre_id[i];
		tmp_item.amount = skill_arrow_db[index].cre_amount[i];
		if(battle_config.produce_item_name_input&0x4) {
			tmp_item.card[0]=CARD0_CREATE;
			tmp_item.card[1]=0;
			tmp_item.card[2]=GetWord(sd->status.char_id,0); // CharId
			tmp_item.card[3]=GetWord(sd->status.char_id,1);
		}
		if(tmp_item.nameid <= 0 || tmp_item.amount <= 0)
			continue;
		if((flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
			clif_additem(sd,0,0,flag);
			map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
		}
	}

	return 0;
}
int skill_poisoningweapon( struct map_session_data *sd, int nameid) {
	sc_type type;
	int chance, i;
	nullpo_ret(sd);
	if( nameid <= 0 || (i = pc_search_inventory(sd,nameid)) < 0 || pc_delitem(sd,i,1,0,0,LOG_TYPE_CONSUME) ) {
		clif_skill_fail(sd,GC_POISONINGWEAPON,USESKILL_FAIL_LEVEL,0);
		return 0;
	}
	switch( nameid )
	{ // t_lv used to take duration from skill_get_time2
		case PO_PARALYSE:      type = SC_PARALYSE;      break;
		case PO_PYREXIA:       type = SC_PYREXIA;		break;
		case PO_DEATHHURT:     type = SC_DEATHHURT;     break;
		case PO_LEECHESEND:    type = SC_LEECHESEND;    break;
		case PO_VENOMBLEED:    type = SC_VENOMBLEED;    break;
		case PO_TOXIN:         type = SC_TOXIN;         break;
		case PO_MAGICMUSHROOM: type = SC_MAGICMUSHROOM; break;
		case PO_OBLIVIONCURSE: type = SC_OBLIVIONCURSE; break;
		default:
			clif_skill_fail(sd,GC_POISONINGWEAPON,USESKILL_FAIL_LEVEL,0);
			return 0;
	}

	chance = 2 + 2 * sd->menuskill_val; // 2 + 2 * skill_lv
	sc_start4(&sd->bl, SC_POISONINGWEAPON, 100, pc_checkskill(sd, GC_RESEARCHNEWPOISON), //in Aegis it store the level of GC_RESEARCHNEWPOISON in val1
		type, chance, 0, skill_get_time(GC_POISONINGWEAPON, sd->menuskill_val));

	return 0;
}

static void skill_toggle_magicpower(struct block_list *bl, short skillid)
{
	struct status_change *sc = status_get_sc(bl);

	// non-offensive and non-magic skills do not affect the status
	if (skill_get_nk(skillid)&NK_NO_DAMAGE || !(skill_get_type(skillid)&BF_MAGIC))
		return;

	if (sc && sc->count && sc->data[SC_MAGICPOWER])
	{
		if (sc->data[SC_MAGICPOWER]->val4)
		{
			status_change_end(bl, SC_MAGICPOWER, INVALID_TIMER);
		}
		else
		{
			sc->data[SC_MAGICPOWER]->val4 = 1;
			status_calc_bl(bl, status_sc2scb_flag(SC_MAGICPOWER));
		}
	}
}


int skill_magicdecoy(struct map_session_data *sd, int nameid) {
	int x, y, i, class_, skill;
	struct mob_data *md;
	nullpo_ret(sd);
	skill = sd->menuskill_val;

	if( nameid <= 0 || !itemdb_is_element(nameid) || (i = pc_search_inventory(sd,nameid)) < 0 || !skill || pc_delitem(sd,i,1,0,0,LOG_TYPE_CONSUME) )
	{
		clif_skill_fail(sd,NC_MAGICDECOY,USESKILL_FAIL_LEVEL,0);
		return 0;
	}

	// Spawn Position
	pc_delitem(sd,i,1,0,0,LOG_TYPE_CONSUME);
	x = sd->sc.comet_x;
	y = sd->sc.comet_y;
	sd->sc.comet_x = sd->sc.comet_y = 0;
	sd->menuskill_val = 0;

	class_ = (nameid == 990 || nameid == 991) ? 2043 + nameid - 990 : (nameid == 992) ? 2046 : 2045;


	md =  mob_once_spawn_sub(&sd->bl, sd->bl.m, x, y, sd->status.name, class_, "");
	if( md ) {
		md->master_id = sd->bl.id;
		md->special_state.ai = 3;
		if( md->deletetimer != INVALID_TIMER )
			delete_timer(md->deletetimer, mob_timer_delete);
		md->deletetimer = add_timer (gettick() + skill_get_time(NC_MAGICDECOY,skill), mob_timer_delete, md->bl.id, 0);
		mob_spawn(md);
		md->status.matk_min = md->status.matk_max = 250 + (50 * skill);
	}

	return 0;
}

// Warlock Spellbooks. [LimitLine/3CeAM]
int skill_spellbook (struct map_session_data *sd, int nameid) {
	int i, max_preserve, skill_id, point;
	struct status_change *sc;
	
	nullpo_ret(sd);

	sc = status_get_sc(&sd->bl);
	status_change_end(&sd->bl, SC_STOP, INVALID_TIMER);

	for(i=SC_SPELLBOOK1; i <= SC_MAXSPELLBOOK; i++) if( sc && !sc->data[i] ) break;
	if( i > SC_MAXSPELLBOOK ) 
	{ 
		clif_skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_READING, 0);
		return 0;
	}

	ARR_FIND(0,MAX_SKILL_SPELLBOOK_DB,i,skill_spellbook_db[i].nameid == nameid); // Search for information of this item
	if( i == MAX_SKILL_SPELLBOOK_DB ) return 0; 

	if( !pc_checkskill(sd, (skill_id = skill_spellbook_db[i].skillid)) )
	{ // User don't know the skill
		sc_start(&sd->bl, SC_SLEEP, 100, 1, skill_get_time(WL_READING_SB, pc_checkskill(sd,WL_READING_SB)));
		clif_skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_DIFFICULT_SLEEP, 0);
		return 0;
	}

	max_preserve = 4 * pc_checkskill(sd, WL_FREEZE_SP) + status_get_int(&sd->bl) / 10 + sd->status.base_level / 10;
	point = skill_spellbook_db[i].point;

	if( sc && sc->data[SC_READING_SB] ){
		if( (sc->data[SC_READING_SB]->val2 + point) > max_preserve )
		{ 
			clif_skill_fail(sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_PRESERVATION_POINT, 0);
			return 0;
		}
		for(i = SC_MAXSPELLBOOK; i >= SC_SPELLBOOK1; i--){ // This is how official saves spellbook. [malufett]
			if( !sc->data[i] ){
				sc->data[SC_READING_SB]->val2 += point; // increase points
				sc_start4(&sd->bl, (sc_type)i, 100, skill_id, pc_checkskill(sd,skill_id), point, 0, INVALID_TIMER);
				break;
			}
		}
	}else{
		sc_start2(&sd->bl, SC_READING_SB, 100, 0, point, INVALID_TIMER);
		sc_start4(&sd->bl, SC_MAXSPELLBOOK, 100, skill_id, pc_checkskill(sd,skill_id), point, 0, INVALID_TIMER);
	}

	return 1;
}
int skill_select_menu(struct map_session_data *sd,int flag,int skill_id) {
	int id, lv, prob, aslvl = 0;
	nullpo_ret(sd);
	
	if (sd->sc.data[SC_STOP]) {
		aslvl = sd->sc.data[SC_STOP]->val1;
		status_change_end(&sd->bl,SC_STOP,INVALID_TIMER);
	}

	if( (id = sd->status.skill[skill_id].id) == 0 || sd->status.skill[skill_id].flag != SKILL_FLAG_PLAGIARIZED ||
				skill_id >= GS_GLITTERING || skill_get_type(skill_id) != BF_MAGIC ) {
		clif_skill_fail(sd,SC_AUTOSHADOWSPELL,0,0);
		return 0;
	}

	lv = (aslvl + 1) / 2; // The level the skill will be autocasted
	lv = min(lv,sd->status.skill[skill_id].lv);
	prob = (aslvl == 10) ? 15 : (32 - 2 * aslvl); // Probability at level 10 was increased to 15.
	sc_start4(&sd->bl,SC__AUTOSHADOWSPELL,100,id,lv,prob,0,skill_get_time(SC_AUTOSHADOWSPELL,aslvl));
	return 0;
}
int skill_elementalanalysis(struct map_session_data* sd, int n, int skill_lv, unsigned short* item_list) {
	int i;
	
	nullpo_ret(sd);
	nullpo_ret(item_list);
	
	if( n <= 0 )
		return 1;
	
	for( i = 0; i < n; i++ ) {
		int nameid, add_amount, del_amount, idx, product, flag;
		struct item tmp_item;
		
		idx = item_list[i*2+0]-2;
		del_amount = item_list[i*2+1];
		
		if( skill_lv == 2 )
			del_amount -= (del_amount % 10);
		add_amount = (skill_lv == 1) ? del_amount * (5 + rnd()%5) : del_amount / 10 ;
		
		if( (nameid = sd->status.inventory[idx].nameid) <= 0 || del_amount > sd->status.inventory[idx].amount ) {
			clif_skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
			return 1;
		}
		
		switch( nameid ) {
				// Level 1
			case 994: product = 990; break;	// Flame Heart -> Red Blood.
			case 995: product = 991; break;	// Mystic Frozen -> Crystal Blue.
			case 996: product = 992; break; // Rough Wind -> Wind of Verdure.
			case 997: product = 993; break; // Great Nature -> Green Live.
				// Level 2
			case 990: product = 994; break;	// Red Blood -> Flame Heart.
			case 991: product = 995; break;	// Crystal Blue -> Mystic Frozen.
			case 992: product = 996; break; // Wind of Verdure -> Rough Wind.
			case 993: product = 997; break; // Green Live -> Great Nature.
			default:
				clif_skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
				return 1;
		}

		if( pc_delitem(sd,idx,del_amount,0,1,LOG_TYPE_CONSUME) ) {
			clif_skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
			return 1;
		}

		if( skill_lv == 2 && rnd()%100 < 25 ) {	// At level 2 have a fail chance. You loose your items if it fails.
			clif_skill_fail(sd,SO_EL_ANALYSIS,USESKILL_FAIL_LEVEL,0);
			return 1;
		}
		
		
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid = product;
		tmp_item.amount = add_amount;
		tmp_item.identify = 1;

		if( tmp_item.amount ) {
			if( (flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_CONSUME)) ) {
				clif_additem(sd,0,0,flag);
				map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
		}
		
	}
	
	return 0;
}

int skill_changematerial(struct map_session_data *sd, int n, unsigned short *item_list) {
	int i, j, k, c, p = 0, nameid, amount;
	
	nullpo_ret(sd);
	nullpo_ret(item_list);

	// Search for objects that can be created.
	for( i = 0; i < MAX_SKILL_PRODUCE_DB; i++ ) {
		if( skill_produce_db[i].itemlv == 26 ) {
			p = 0;
			do {
				c = 0;
				// Verification of overlap between the objects required and the list submitted.
				for( j = 0; j < MAX_PRODUCE_RESOURCE; j++ ) {
					if( skill_produce_db[i].mat_id[j] > 0 ) {
						for( k = 0; k < n; k++ ) {
							int idx = item_list[k*2+0]-2;
							nameid = sd->status.inventory[idx].nameid;
							amount = item_list[k*2+1];
							if( nameid > 0 && sd->status.inventory[idx].identify == 0 ){
								clif_msg_skill(sd,GN_CHANGEMATERIAL,0x62D);
								return 0;
							}
							if( nameid == skill_produce_db[i].mat_id[j] && (amount-p*skill_produce_db[i].mat_amount[j]) >= skill_produce_db[i].mat_amount[j]
								&& (amount-p*skill_produce_db[i].mat_amount[j])%skill_produce_db[i].mat_amount[j] == 0 ) // must be in exact amount
								c++; // match
						}
					}
					else
						break;	// No more items required
				}
				p++;
			} while(n == j && c == n);
			p--;
			if ( p > 0 ) {
				skill_produce_mix(sd,GN_CHANGEMATERIAL,skill_produce_db[i].nameid,0,0,0,p);
				return 1;
			}
		}
	}

	if( p == 0)
		clif_msg_skill(sd,GN_CHANGEMATERIAL,0x623);

	return 0;
}
/**
 * for Royal Guard's LG_TRAMPLE
 **/
static int skill_destroy_trap( struct block_list *bl, va_list ap ) {
	struct skill_unit *su = (struct skill_unit *)bl;
	struct skill_unit_group *sg;
	unsigned int tick;
	
	nullpo_ret(su);
	tick = va_arg(ap, unsigned int);

	if (su->alive && (sg = su->group) && skill_get_inf2(sg->skill_id)&INF2_TRAP) {
		switch( sg->unit_id ) {
			case UNT_LANDMINE:
			case UNT_CLAYMORETRAP:
			case UNT_BLASTMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
			case UNT_CLUSTERBOMB:
			case UNT_FIRINGTRAP:
			case UNT_ICEBOUNDTRAP:
				map_foreachinrange(skill_trap_splash,&su->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &su->bl,tick);
				break;
		}
		// Traps aren't recovered.
		skill_delunit(su);
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
int skill_blockpc_end(int tid, unsigned int tick, int id, intptr_t data)
{
	struct map_session_data *sd = map_id2sd(id);
	struct skill_cd * cd = NULL;

	if (data <= 0 || data >= MAX_SKILL)
		return 0;
	if (!sd) return 0;
	if (sd->blockskill[data] != (0x1|(tid&0xFE))) return 0;

	if( ( cd = idb_get(skillcd_db,sd->status.char_id) ) ) {
		int i,cursor;
		ARR_FIND( 0, cd->cursor+1, cursor, cd->skidx[cursor] == data );
		cd->duration[cursor] = 0;
		cd->skidx[cursor] = 0;
		cd->nameid[cursor] = 0;
		// compact the cool down list
		for( i = 0, cursor = 0; i < cd->cursor; i++ ) {
			if( cd->duration[i] == 0 )
				continue;
			if( cursor != i ) {
				cd->duration[cursor] = cd->duration[i];
				cd->skidx[cursor] = cd->skidx[i];
				cd->nameid[cursor] = cd->nameid[i];
			}
			cursor++;
		}
		if( cursor == 0 )
			idb_remove(skillcd_db,sd->status.char_id);
		else
			cd->cursor = cursor;
	}

	sd->blockskill[data] = 0;
	return 1;
}

/**
 * flags a singular skill as being blocked from persistent usage.
 * @param   sd        the player the skill delay affects
 * @param   skillid   the skill which should be delayed
 * @param   tick      the length of time the delay should last
 * @param   load      whether this assignment is being loaded upon player login
 * @return  0 if successful, -1 otherwise
 */
int skill_blockpc_start_(struct map_session_data *sd, int skillid, int tick, bool load)
{
	int oskillid = skillid;
	struct skill_cd* cd = NULL;

	nullpo_retr (-1, sd);

	skillid = skill_get_index(skillid);
	if (skillid == 0)
		return -1;

	if (tick < 1) {
		sd->blockskill[skillid] = 0;
		return -1;
	}

	if( battle_config.display_status_timers )
		clif_skill_cooldown(sd, skillid, tick);

	if( !load )
	{// not being loaded initially so ensure the skill delay is recorded
		if( !(cd = idb_get(skillcd_db,sd->status.char_id)) )
		{// create a new skill cooldown object for map storage
			CREATE( cd, struct skill_cd, 1 );
			idb_put( skillcd_db, sd->status.char_id, cd );
		}

		// record the skill duration in the database map
		cd->duration[cd->cursor] = tick;
		cd->skidx[cd->cursor] = skillid;
		cd->nameid[cd->cursor] = oskillid;
		cd->cursor++;
	}

	sd->blockskill[skillid] = 0x1|(0xFE&add_timer(gettick()+tick,skill_blockpc_end,sd->bl.id,skillid));
	return 0;
}

int skill_blockhomun_end(int tid, unsigned int tick, int id, intptr_t data)	//[orn]
{
	struct homun_data *hd = (TBL_HOM*) map_id2bl(id);
	if (data <= 0 || data >= MAX_SKILL)
		return 0;
	if (hd) hd->blockskill[data] = 0;

	return 1;
}

int skill_blockhomun_start(struct homun_data *hd, int skillid, int tick)	//[orn]
{
	nullpo_retr (-1, hd);

	skillid = skill_get_index(skillid);
	if (skillid == 0)
		return -1;

	if (tick < 1) {
		hd->blockskill[skillid] = 0;
		return -1;
	}
	hd->blockskill[skillid] = 1;
	return add_timer(gettick() + tick, skill_blockhomun_end, hd->bl.id, skillid);
}

int skill_blockmerc_end(int tid, unsigned int tick, int id, intptr_t data)	//[orn]
{
	struct mercenary_data *md = (TBL_MER*)map_id2bl(id);
	if( data <= 0 || data >= MAX_SKILL )
		return 0;
	if( md ) md->blockskill[data] = 0;

	return 1;
}

int skill_blockmerc_start(struct mercenary_data *md, int skillid, int tick)
{
	nullpo_retr (-1, md);

	if( (skillid = skill_get_index(skillid)) == 0 )
		return -1;
	if( tick < 1 )
	{
		md->blockskill[skillid] = 0;
		return -1;
	}
	md->blockskill[skillid] = 1;
	return add_timer(gettick() + tick, skill_blockmerc_end, md->bl.id, skillid);
}
/**
 * Adds a new skill unit entry for this player to recast after map load
 **/
void skill_usave_add(struct map_session_data * sd, int skill_num, int skill_lv) {
	struct skill_usave * sus = NULL;

	if( idb_exists(skillusave_db,sd->status.char_id) ) {
		idb_remove(skillusave_db,sd->status.char_id);
	}
	
	CREATE( sus, struct skill_usave, 1 );
	idb_put( skillusave_db, sd->status.char_id, sus );

	sus->skill_num = skill_num;
	sus->skill_lv = skill_lv;
	
	return;
}
void skill_usave_trigger(struct map_session_data *sd) {
	struct skill_usave * sus = NULL;

	if( ! (sus = idb_get(skillusave_db,sd->status.char_id)) ) {
		return;
	}
	
	skill_unitsetting(&sd->bl,sus->skill_num,sus->skill_lv,sd->bl.x,sd->bl.y,0);

	idb_remove(skillusave_db,sd->status.char_id);
	
	return;
}
/*
 *
 */
int skill_split_str (char *str, char **val, int num)
{
	int i;

	for( i = 0; i < num && str; i++ )
	{
		val[i] = str;
		str = strchr(str,',');
		if( str )
			*str++=0;
	}

	return i;
}
/*
 *
 */
int skill_split_atoi (char *str, int *val)
{
	int i, j, diff, step = 1;

	for (i=0; i<MAX_SKILL_LEVEL; i++) {
		if (!str) break;
		val[i] = atoi(str);
		str = strchr(str,':');
		if (str)
			*str++=0;
	}
	if(i==0) //No data found.
		return 0;
	if(i==1)
	{	//Single value, have the whole range have the same value.
		for (; i < MAX_SKILL_LEVEL; i++)
			val[i] = val[i-1];
		return i;
	}
	//Check for linear change with increasing steps until we reach half of the data acquired.
	for (step = 1; step <= i/2; step++)
	{
		diff = val[i-1] - val[i-step-1];
		for(j = i-1; j >= step; j--)
			if ((val[j]-val[j-step]) != diff)
				break;

		if (j>=step) //No match, try next step.
			continue;

		for(; i < MAX_SKILL_LEVEL; i++)
		{	//Apply linear increase
			val[i] = val[i-step]+diff;
			if (val[i] < 1 && val[i-1] >=0) //Check if we have switched from + to -, cap the decrease to 0 in said cases.
			{ val[i] = 1; diff = 0; step = 1; }
		}
		return i;
	}
	//Okay.. we can't figure this one out, just fill out the stuff with the previous value.
	for (;i<MAX_SKILL_LEVEL; i++)
		val[i] = val[i-1];
	return i;
}

/*
 *
 */
void skill_init_unit_layout (void)
{
	int i,j,size,pos = 0;

	memset(skill_unit_layout,0,sizeof(skill_unit_layout));

	// standard square layouts go first
	for (i=0; i<=MAX_SQUARE_LAYOUT; i++) {
		size = i*2+1;
		skill_unit_layout[i].count = size*size;
		for (j=0; j<size*size; j++) {
			skill_unit_layout[i].dx[j] = (j%size-i);
			skill_unit_layout[i].dy[j] = (j/size-i);
		}
	}

	// afterwards add special ones
	pos = i;
	for (i=0;i<MAX_SKILL_DB;i++) {
		if (!skill_db[i].unit_id[0] || skill_db[i].unit_layout_type[0] != -1)
			continue;
		if( i >= HM_SKILLRANGEMIN && i <= EL_SKILLRANGEMAX ) {
			int skill = i;

			if( i >= EL_SKILLRANGEMIN && i <= EL_SKILLRANGEMAX ) {
				skill -= EL_SKILLRANGEMIN;
				skill += EL_SKILLBASE;
			}
			if( skill == EL_FIRE_MANTLE ) {
				static const int dx[] = {-1, 0, 1, 1, 1, 0,-1,-1};
				static const int dy[] = { 1, 1, 1, 0,-1,-1,-1, 0};
				skill_unit_layout[pos].count = 8;
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			}
		} else {		
			switch (i) {
				case MG_FIREWALL:
				case WZ_ICEWALL:
				case WL_EARTHSTRAIN://Warlock
					// these will be handled later
					break;
				case PR_SANCTUARY:
				case NPC_EVILLAND: {
						static const int dx[] = {
							-1, 0, 1,-2,-1, 0, 1, 2,-2,-1,
							 0, 1, 2,-2,-1, 0, 1, 2,-1, 0, 1};
						static const int dy[]={
							-2,-2,-2,-1,-1,-1,-1,-1, 0, 0,
							 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2};
						skill_unit_layout[pos].count = 21;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				case PR_MAGNUS: {
						static const int dx[] = {
							-1, 0, 1,-1, 0, 1,-3,-2,-1, 0,
							 1, 2, 3,-3,-2,-1, 0, 1, 2, 3,
							-3,-2,-1, 0, 1, 2, 3,-1, 0, 1,-1, 0, 1};
						static const int dy[] = {
							-3,-3,-3,-2,-2,-2,-1,-1,-1,-1,
							-1,-1,-1, 0, 0, 0, 0, 0, 0, 0,
							 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3};
						skill_unit_layout[pos].count = 33;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				case MH_POISON_MIST:
				case AS_VENOMDUST: {
						static const int dx[] = {-1, 0, 0, 0, 1};
						static const int dy[] = { 0,-1, 0, 1, 0};
						skill_unit_layout[pos].count = 5;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				case CR_GRANDCROSS:
				case NPC_GRANDDARKNESS: {
						static const int dx[] = {
							 0, 0,-1, 0, 1,-2,-1, 0, 1, 2,
							-4,-3,-2,-1, 0, 1, 2, 3, 4,-2,
							-1, 0, 1, 2,-1, 0, 1, 0, 0};
						static const int dy[] = {
							-4,-3,-2,-2,-2,-1,-1,-1,-1,-1,
							 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
							 1, 1, 1, 1, 2, 2, 2, 3, 4};
						skill_unit_layout[pos].count = 29;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				case PF_FOGWALL: {
						static const int dx[] = {
							-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
						static const int dy[] = {
							-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1};
						skill_unit_layout[pos].count = 15;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				case PA_GOSPEL: {
						static const int dx[] = {
							-1, 0, 1,-1, 0, 1,-3,-2,-1, 0,
							 1, 2, 3,-3,-2,-1, 0, 1, 2, 3,
							-3,-2,-1, 0, 1, 2, 3,-1, 0, 1,
							-1, 0, 1};
						static const int dy[] = {
							-3,-3,-3,-2,-2,-2,-1,-1,-1,-1,
							-1,-1,-1, 0, 0, 0, 0, 0, 0, 0,
							 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
							 3, 3, 3};
						skill_unit_layout[pos].count = 33;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				case NJ_KAENSIN: {
						static const int dx[] = {-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
						static const int dy[] = { 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2};
						skill_unit_layout[pos].count = 24;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				case NJ_TATAMIGAESHI: {
						//Level 1 (count 4, cross of 3x3)
						static const int dx1[] = {-1, 1, 0, 0};
						static const int dy1[] = { 0, 0,-1, 1};
						//Level 2-3 (count 8, cross of 5x5)
						static const int dx2[] = {-2,-1, 1, 2, 0, 0, 0, 0};
						static const int dy2[] = { 0, 0, 0, 0,-2,-1, 1, 2};
						//Level 4-5 (count 12, cross of 7x7
						static const int dx3[] = {-3,-2,-1, 1, 2, 3, 0, 0, 0, 0, 0, 0};
						static const int dy3[] = { 0, 0, 0, 0, 0, 0,-3,-2,-1, 1, 2, 3};
						//lv1
						j = 0;
						skill_unit_layout[pos].count = 4;
						memcpy(skill_unit_layout[pos].dx,dx1,sizeof(dx1));
						memcpy(skill_unit_layout[pos].dy,dy1,sizeof(dy1));
						skill_db[i].unit_layout_type[j] = pos;
						//lv2/3
						j++;
						pos++;
						skill_unit_layout[pos].count = 8;
						memcpy(skill_unit_layout[pos].dx,dx2,sizeof(dx2));
						memcpy(skill_unit_layout[pos].dy,dy2,sizeof(dy2));
						skill_db[i].unit_layout_type[j] = pos;
						skill_db[i].unit_layout_type[++j] = pos;
						//lv4/5
						j++;
						pos++;
						skill_unit_layout[pos].count = 12;
						memcpy(skill_unit_layout[pos].dx,dx3,sizeof(dx3));
						memcpy(skill_unit_layout[pos].dy,dy3,sizeof(dy3));
						skill_db[i].unit_layout_type[j] = pos;
						skill_db[i].unit_layout_type[++j] = pos;
						//Fill in the rest using lv 5.
						for (;j<MAX_SKILL_LEVEL;j++)
							skill_db[i].unit_layout_type[j] = pos;
						//Skip, this way the check below will fail and continue to the next skill.
						pos++;
					}
					break;
				case GN_WALLOFTHORN: {
						static const int dx[] = {-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 2, 2, 2, 2, 1, 0};
						static const int dy[] = { 2, 2, 1, 0,-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 2, 2};
						skill_unit_layout[pos].count = 16;
						memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
						memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
					}
					break;
				default:
					ShowError("unknown unit layout at skill %d\n",i);
					break;
			}
		}
		if (!skill_unit_layout[pos].count)
			continue;
		for (j=0;j<MAX_SKILL_LEVEL;j++)
			skill_db[i].unit_layout_type[j] = pos;
		pos++;
	}

	// firewall and icewall have 8 layouts (direction-dependent)
	firewall_unit_pos = pos;
	for (i=0;i<8;i++) {
		if (i&1) {
			skill_unit_layout[pos].count = 5;
			if (i&0x2) {
				int dx[] = {-1,-1, 0, 0, 1};
				int dy[] = { 1, 0, 0,-1,-1};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 1, 1 ,0, 0,-1};
				int dy[] = { 1, 0, 0,-1,-1};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			}
		} else {
			skill_unit_layout[pos].count = 3;
			if (i%4==0) {
				int dx[] = {-1, 0, 1};
				int dy[] = { 0, 0, 0};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 0, 0, 0};
				int dy[] = {-1, 0, 1};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			}
		}
		pos++;
	}
	icewall_unit_pos = pos;
	for (i=0;i<8;i++) {
		skill_unit_layout[pos].count = 5;
		if (i&1) {
			if (i&0x2) {
				int dx[] = {-2,-1, 0, 1, 2};
				int dy[] = { 2, 1, 0,-1,-2};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 2, 1 ,0,-1,-2};
				int dy[] = { 2, 1, 0,-1,-2};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			}
		} else {
			if (i%4==0) {
				int dx[] = {-2,-1, 0, 1, 2};
				int dy[] = { 0, 0, 0, 0, 0};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			} else {
				int dx[] = { 0, 0, 0, 0, 0};
				int dy[] = {-2,-1, 0, 1, 2};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			}
		}
		pos++;
	}
	earthstrain_unit_pos = pos;
	for( i = 0; i < 8; i++ )
	{ // For each Direction
		skill_unit_layout[pos].count = 15;
		switch( i )
		{
		case 0: case 1: case 3: case 4: case 5: case 7:
			{
				int dx[] = {-7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7};
				int dy[] = { 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			}
			break;
		case 2:
		case 6:
			{
				int dx[] = { 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0};
				int dy[] = {-7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7};
				memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
				memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
			}
			break;
		}
		pos++;
	}

}

int skill_block_check(struct block_list *bl, sc_type type , int skillid) {
	int inf = 0;
	struct status_change *sc = status_get_sc(bl); 

	if( !sc || !bl || skillid < 1 )
		return 0; // Can do it

	switch(type){
		case SC_STASIS:
			inf = skill_get_inf2(skillid);
			if( inf == INF2_SONG_DANCE || /*skill_get_inf2(skillid) == INF2_CHORUS_SKILL ||*/ inf == INF2_SPIRIT_SKILL )
				return 1; // Can't do it.
			switch( skillid )
			{
				case NV_FIRSTAID:		case TF_HIDING:			case AS_CLOAKING:		case WZ_SIGHTRASHER:
				case RG_STRIPWEAPON:		case RG_STRIPSHIELD:		case RG_STRIPARMOR:		case WZ_METEOR:
				case RG_STRIPHELM:		case SC_STRIPACCESSARY:		case ST_FULLSTRIP:		case WZ_SIGHTBLASTER:
				case ST_CHASEWALK:		case SC_ENERVATION:		case SC_GROOMY:			case WZ_ICEWALL:
				case SC_IGNORANCE:		case SC_LAZINESS:		case SC_UNLUCKY:		case WZ_STORMGUST:
				case SC_WEAKNESS:		case AL_RUWACH:			case AL_PNEUMA:			case WZ_JUPITEL:
				case AL_HEAL:			case AL_BLESSING:		case AL_INCAGI:			case WZ_VERMILION:
				case AL_TELEPORT:		case AL_WARP:			case AL_HOLYWATER:		case WZ_EARTHSPIKE:
				case AL_HOLYLIGHT:		case PR_IMPOSITIO:		case PR_ASPERSIO:		case WZ_HEAVENDRIVE:
				case PR_SANCTUARY:		case PR_STRECOVERY:		case PR_MAGNIFICAT:		case WZ_QUAGMIRE:
				case ALL_RESURRECTION:		case PR_LEXDIVINA:		case PR_LEXAETERNA:		case HW_GRAVITATION:
				case PR_MAGNUS:			case PR_TURNUNDEAD:		case MG_SRECOVERY:		case HW_MAGICPOWER:
				case MG_SIGHT:			case MG_NAPALMBEAT:		case MG_SAFETYWALL:		case HW_GANBANTEIN:
				case MG_SOULSTRIKE:		case MG_COLDBOLT:		case MG_FROSTDIVER:		case WL_DRAINLIFE:
				case MG_STONECURSE:		case MG_FIREBALL:		case MG_FIREWALL:		case WL_SOULEXPANSION:
				case MG_FIREBOLT:		case MG_LIGHTNINGBOLT:		case MG_THUNDERSTORM:		case MG_ENERGYCOAT:
				case WL_WHITEIMPRISON:		case WL_SUMMONFB:		case WL_SUMMONBL:		case WL_SUMMONWB:
				case WL_SUMMONSTONE:		case WL_SIENNAEXECRATE:		case WL_RELEASE:		case WL_EARTHSTRAIN:
				case WL_RECOGNIZEDSPELL: 	case WL_READING_SB:		case SA_MAGICROD:		case SA_SPELLBREAKER:
				case SA_DISPELL:		case SA_FLAMELAUNCHER:		case SA_FROSTWEAPON:		case SA_LIGHTNINGLOADER:
				case SA_SEISMICWEAPON:		case SA_VOLCANO:		case SA_DELUGE:			case SA_VIOLENTGALE:
				case SA_LANDPROTECTOR:		case PF_HPCONVERSION:		case PF_SOULCHANGE:		case PF_SPIDERWEB:
				case PF_FOGWALL:		case TK_RUN:			case TK_HIGHJUMP:		case TK_SEVENWIND:
				case SL_KAAHI:			case SL_KAUPE:			case SL_KAITE:

				// Skills that need to be confirmed.
				case SO_FIREWALK:		case SO_ELECTRICWALK:		case SO_SPELLFIST:		case SO_EARTHGRAVE:
				case SO_DIAMONDDUST:		case SO_POISON_BUSTER:		case SO_PSYCHIC_WAVE:		case SO_CLOUD_KILL:
				case SO_STRIKING:		case SO_WARMER:			case SO_VACUUM_EXTREME:		case SO_VARETYR_SPEAR:
				case SO_ARRULLO:
					return 1;	// Can't do it.
			}
			break;
		case SC_KAGEHUMI:
			switch(skillid){
				case TF_HIDING:		case AS_CLOAKING:	case GC_CLOAKINGEXCEED:	case SC_SHADOWFORM:
				case MI_HARMONIZE:	case CG_MARIONETTE:	case AL_TELEPORT:		case TF_BACKSLIDING:
				case RA_CAMOUFLAGE: case ST_CHASEWALK:	case GD_EMERGENCYCALL:
					return 1; // needs more info
			}
			break;
	}

	return 0;
}

int skill_get_elemental_type( int skill_id , int skill_lv ) {
	int type = 0;
	
	switch( skill_id ) {
		case SO_SUMMON_AGNI:	type = 2114; break;
		case SO_SUMMON_AQUA:	type = 2117; break;
		case SO_SUMMON_VENTUS:	type = 2120; break;
		case SO_SUMMON_TERA:	type = 2123; break;
	}
	
	type += skill_lv - 1;
	
	return type;
}

/**
 * reload stored skill cooldowns when a player logs in.
 * @param   sd     the affected player structure
 */
void skill_cooldown_load(struct map_session_data * sd)
{
	int i;
	struct skill_cd* cd = NULL;

	// always check to make sure the session properly exists
	nullpo_retv(sd);

	if( !(cd = idb_get(skillcd_db, sd->status.char_id)) )
	{// no skill cooldown is associated with this character
		return;
	}
	
	// process each individual cooldown associated with the character
	for( i = 0; i < cd->cursor; i++ )
	{
		// block the skill from usage but ensure it is not recorded (load = true)
		skill_blockpc_start_( sd, cd->nameid[i], cd->duration[i], true );
	}
}

/*==========================================
 * DB reading.
 * skill_db.txt
 * skill_require_db.txt
 * skill_cast_db.txt
 * skill_castnodex_db.txt
 * skill_nocast_db.txt
 * skill_unit_db.txt
 * produce_db.txt
 * create_arrow_db.txt
 * abra_db.txt
 *------------------------------------------*/

static bool skill_parse_row_skilldb(char* split[], int columns, int current)
{// id,range,hit,inf,element,nk,splash,max,list_num,castcancel,cast_defence_rate,inf2,maxcount,skill_type,blow_count,name,description
	int id = atoi(split[0]);
	int i;
	if( (id >= GD_SKILLRANGEMIN && id <= GD_SKILLRANGEMAX)
	||  (id >= HM_SKILLRANGEMIN && id <= HM_SKILLRANGEMAX)
	||  (id >= MC_SKILLRANGEMIN && id <= MC_SKILLRANGEMAX)
	||  (id >= EL_SKILLRANGEMIN && id <= EL_SKILLRANGEMAX) ) {
		ShowWarning("skill_parse_row_skilldb: Skill id %d is forbidden (interferes with guild/homun/mercenary skill mapping)!\n", id);
		return false;
	}

	i = skill_get_index(id);
	if( !i ) // invalid skill id
		return false;

	skill_split_atoi(split[1],skill_db[i].range);
	skill_db[i].hit = atoi(split[2]);
	skill_db[i].inf = atoi(split[3]);
	skill_split_atoi(split[4],skill_db[i].element);
	skill_db[i].nk = (int)strtol(split[5], NULL, 0);
	skill_split_atoi(split[6],skill_db[i].splash);
	skill_db[i].max = atoi(split[7]);
	skill_split_atoi(split[8],skill_db[i].num);

	if( strcmpi(split[9],"yes") == 0 )
		skill_db[i].castcancel = 1;
	else
		skill_db[i].castcancel = 0;
	skill_db[i].cast_def_rate = atoi(split[10]);
	skill_db[i].inf2 = (int)strtol(split[11], NULL, 0);
	skill_split_atoi(split[12],skill_db[i].maxcount);
	if( strcmpi(split[13],"weapon") == 0 )
		skill_db[i].skill_type = BF_WEAPON;
	else if( strcmpi(split[13],"magic") == 0 )
		skill_db[i].skill_type = BF_MAGIC;
	else if( strcmpi(split[13],"misc") == 0 )
		skill_db[i].skill_type = BF_MISC;
	else
		skill_db[i].skill_type = 0;
	skill_split_atoi(split[14],skill_db[i].blewcount);
	safestrncpy(skill_db[i].name, trim(split[15]), sizeof(skill_db[i].name));
	safestrncpy(skill_db[i].desc, trim(split[16]), sizeof(skill_db[i].desc));
	strdb_iput(skilldb_name2id, skill_db[i].name, id);

	return true;
}

static bool skill_parse_row_requiredb(char* split[], int columns, int current)
{// SkillID,HPCost,MaxHPTrigger,SPCost,HPRateCost,SPRateCost,ZenyCost,RequiredWeapons,RequiredAmmoTypes,RequiredAmmoAmount,RequiredState,SpiritSphereCost,RequiredItemID1,RequiredItemAmount1,RequiredItemID2,RequiredItemAmount2,RequiredItemID3,RequiredItemAmount3,RequiredItemID4,RequiredItemAmount4,RequiredItemID5,RequiredItemAmount5,RequiredItemID6,RequiredItemAmount6,RequiredItemID7,RequiredItemAmount7,RequiredItemID8,RequiredItemAmount8,RequiredItemID9,RequiredItemAmount9,RequiredItemID10,RequiredItemAmount10
	char* p;
	int j;

	int i = atoi(split[0]);
	i = skill_get_index(i);
	if( !i ) // invalid skill id
		return false;

	skill_split_atoi(split[1],skill_db[i].hp);
	skill_split_atoi(split[2],skill_db[i].mhp);
	skill_split_atoi(split[3],skill_db[i].sp);
	skill_split_atoi(split[4],skill_db[i].hp_rate);
	skill_split_atoi(split[5],skill_db[i].sp_rate);
	skill_split_atoi(split[6],skill_db[i].zeny);

	//FIXME: document this
	p = split[7];
	for( j = 0; j < 32; j++ )
	{
		int l = atoi(p);
		if( l == 99 ) // Any weapon
		{
			skill_db[i].weapon = 0;
			break;
		}
		else
			skill_db[i].weapon |= 1<<l;
		p = strchr(p,':');
		if(!p)
			break;
		p++;
	}

	//FIXME: document this
	p = split[8];
	for( j = 0; j < 32; j++ )
	{
		int l = atoi(p);
		if( l == 99 ) // Any ammo type
		{
			skill_db[i].ammo = 0xFFFFFFFF;
			break;
		}
		else if( l ) // 0 stands for no requirement
			skill_db[i].ammo |= 1<<l;
		p = strchr(p,':');
		if( !p )
			break;
		p++;
	}
	skill_split_atoi(split[9],skill_db[i].ammo_qty);

	if(      strcmpi(split[10],"hiding")==0 ) skill_db[i].state = ST_HIDING;
	else if( strcmpi(split[10],"cloaking")==0 ) skill_db[i].state = ST_CLOAKING;
	else if( strcmpi(split[10],"hidden")==0 ) skill_db[i].state = ST_HIDDEN;
	else if( strcmpi(split[10],"riding")==0 ) skill_db[i].state = ST_RIDING;
	else if( strcmpi(split[10],"falcon")==0 ) skill_db[i].state = ST_FALCON;
	else if( strcmpi(split[10],"cart")==0 ) skill_db[i].state = ST_CART;
	else if( strcmpi(split[10],"shield")==0 ) skill_db[i].state = ST_SHIELD;
	else if( strcmpi(split[10],"sight")==0 ) skill_db[i].state = ST_SIGHT;
	else if( strcmpi(split[10],"explosionspirits")==0 ) skill_db[i].state = ST_EXPLOSIONSPIRITS;
	else if( strcmpi(split[10],"cartboost")==0 ) skill_db[i].state = ST_CARTBOOST;
	else if( strcmpi(split[10],"recover_weight_rate")==0 ) skill_db[i].state = ST_RECOV_WEIGHT_RATE;
	else if( strcmpi(split[10],"move_enable")==0 ) skill_db[i].state = ST_MOVE_ENABLE;
	else if( strcmpi(split[10],"water")==0 ) skill_db[i].state = ST_WATER;
	/**
	 * New States
	 **/
	else if( strcmpi(split[10],"dragon")==0 ) skill_db[i].state = ST_RIDINGDRAGON;
	else if( strcmpi(split[10],"warg")==0 ) skill_db[i].state = ST_WUG;
	else if( strcmpi(split[10],"ridingwarg")==0 ) skill_db[i].state = ST_RIDINGWUG;
	else if( strcmpi(split[10],"mado")==0 ) skill_db[i].state = ST_MADO;
	else if( strcmpi(split[10],"elementalspirit")==0 ) skill_db[i].state = ST_ELEMENTALSPIRIT;
	/**
	 * Unknown or no state
	 **/
	else skill_db[i].state = ST_NONE;

	skill_split_atoi(split[11],skill_db[i].spiritball);
	for( j = 0; j < MAX_SKILL_ITEM_REQUIRE; j++ ) {
		skill_db[i].itemid[j] = atoi(split[12+ 2*j]);
		skill_db[i].amount[j] = atoi(split[13+ 2*j]);
	}

	return true;
}

static bool skill_parse_row_castdb(char* split[], int columns, int current)
{// SkillID,CastingTime,AfterCastActDelay,AfterCastWalkDelay,Duration1,Duration2
	int i = atoi(split[0]);
	i = skill_get_index(i);
	if( !i ) // invalid skill id
		return false;

	skill_split_atoi(split[1],skill_db[i].cast);
	skill_split_atoi(split[2],skill_db[i].delay);
	skill_split_atoi(split[3],skill_db[i].walkdelay);
	skill_split_atoi(split[4],skill_db[i].upkeep_time);
	skill_split_atoi(split[5],skill_db[i].upkeep_time2);
	skill_split_atoi(split[6],skill_db[i].cooldown);
#ifdef RENEWAL_CAST
	skill_split_atoi(split[7],skill_db[i].fixed_cast);
#endif
	return true;
}

static bool skill_parse_row_castnodexdb(char* split[], int columns, int current)
{// Skill id,Cast,Delay (optional)
	int i = atoi(split[0]);
	i = skill_get_index(i);
	if( !i ) // invalid skill id
		return false;

	skill_split_atoi(split[1],skill_db[i].castnodex);
	if( split[2] ) // optional column
		skill_split_atoi(split[2],skill_db[i].delaynodex);

	return true;
}

static bool skill_parse_row_nocastdb(char* split[], int columns, int current)
{// SkillID,Flag
	int i = atoi(split[0]);
	i = skill_get_index(i);
	if( !i ) // invalid skill id
		return false;

	skill_db[i].nocast |= atoi(split[1]);

	return true;
}

static bool skill_parse_row_unitdb(char* split[], int columns, int current)
{// ID,unit ID,unit ID 2,layout,range,interval,target,flag
	int i = atoi(split[0]);
	i = skill_get_index(i);
	if( !i ) // invalid skill id
		return false;

	skill_db[i].unit_id[0] = strtol(split[1],NULL,16);
	skill_db[i].unit_id[1] = strtol(split[2],NULL,16);
	skill_split_atoi(split[3],skill_db[i].unit_layout_type);
	skill_split_atoi(split[4],skill_db[i].unit_range);
	skill_db[i].unit_interval = atoi(split[5]);

	if( strcmpi(split[6],"noenemy")==0 ) skill_db[i].unit_target = BCT_NOENEMY;
	else if( strcmpi(split[6],"friend")==0 ) skill_db[i].unit_target = BCT_NOENEMY;
	else if( strcmpi(split[6],"party")==0 ) skill_db[i].unit_target = BCT_PARTY;
	else if( strcmpi(split[6],"ally")==0 ) skill_db[i].unit_target = BCT_PARTY|BCT_GUILD;
	else if( strcmpi(split[6],"guild")==0 ) skill_db[i].unit_target = BCT_GUILD;
	else if( strcmpi(split[6],"all")==0 ) skill_db[i].unit_target = BCT_ALL;
	else if( strcmpi(split[6],"enemy")==0 ) skill_db[i].unit_target = BCT_ENEMY;
	else if( strcmpi(split[6],"self")==0 ) skill_db[i].unit_target = BCT_SELF;
	else if( strcmpi(split[6],"noone")==0 ) skill_db[i].unit_target = BCT_NOONE;
	else skill_db[i].unit_target = strtol(split[6],NULL,16);

	skill_db[i].unit_flag = strtol(split[7],NULL,16);

	if (skill_db[i].unit_flag&UF_DEFNOTENEMY && battle_config.defnotenemy)
		skill_db[i].unit_target = BCT_NOENEMY;

	//By default, target just characters.
	skill_db[i].unit_target |= BL_CHAR;
	if (skill_db[i].unit_flag&UF_NOPC)
		skill_db[i].unit_target &= ~BL_PC;
	if (skill_db[i].unit_flag&UF_NOMOB)
		skill_db[i].unit_target &= ~BL_MOB;
	if (skill_db[i].unit_flag&UF_SKILL)
		skill_db[i].unit_target |= BL_SKILL;

	return true;
}

static bool skill_parse_row_producedb(char* split[], int columns, int current)
{// ProduceItemID,ItemLV,RequireSkill,RequireSkillLv,MaterialID1,MaterialAmount1,......
	int x,y;

	int i = atoi(split[0]);
	if( !i )
		return false;

	skill_produce_db[current].nameid = i;
	skill_produce_db[current].itemlv = atoi(split[1]);
	skill_produce_db[current].req_skill = atoi(split[2]);
	skill_produce_db[current].req_skill_lv = atoi(split[3]);

	for( x = 4, y = 0; x+1 < columns && split[x] && split[x+1] && y < MAX_PRODUCE_RESOURCE; x += 2, y++ )
	{
		skill_produce_db[current].mat_id[y] = atoi(split[x]);
		skill_produce_db[current].mat_amount[y] = atoi(split[x+1]);
	}

	return true;
}

static bool skill_parse_row_createarrowdb(char* split[], int columns, int current)
{// SourceID,MakeID1,MakeAmount1,...,MakeID5,MakeAmount5
	int x,y;

	int i = atoi(split[0]);
	if( !i )
		return false;

	skill_arrow_db[current].nameid = i;

	for( x = 1, y = 0; x+1 < columns && split[x] && split[x+1] && y < MAX_ARROW_RESOURCE; x += 2, y++ )
	{
		skill_arrow_db[current].cre_id[y] = atoi(split[x]);
		skill_arrow_db[current].cre_amount[y] = atoi(split[x+1]);
	}

	return true;
}
static bool skill_parse_row_spellbookdb(char* split[], int columns, int current)
{// SkillID,PreservePoints

	int skillid = atoi(split[0]),
		points = atoi(split[1]),
		nameid = atoi(split[2]);

	if( !skill_get_index(skillid) || !skill_get_max(skillid) )
		ShowError("spellbook_db: Invalid skill ID %d\n", skillid);
	if ( !skill_get_inf(skillid) )
		ShowError("spellbook_db: Passive skills cannot be memorized (%d/%s)\n", skillid, skill_get_name(skillid));
	if( points < 1 )
		ShowError("spellbook_db: PreservePoints have to be 1 or above! (%d/%s)\n", skillid, skill_get_name(skillid));
	else
	{
		skill_spellbook_db[current].skillid = skillid;
		skill_spellbook_db[current].point = points;
		skill_spellbook_db[current].nameid = nameid;

		return true;
	}

	return false;
}
static bool skill_parse_row_improvisedb(char* split[], int columns, int current)
{// SkillID
	int i = atoi(split[0]);
	short j = atoi(split[1]);

	if( !skill_get_index(i) || !skill_get_max(i) ) {
		ShowError("skill_improvise_db: Invalid skill ID %d\n", i);
		return false;
	}
	if ( !skill_get_inf(i) ) {
		ShowError("skill_improvise_db: Passive skills cannot be casted (%d/%s)\n", i, skill_get_name(i));
		return false;
	}
	if( j < 1 ) {
		ShowError("skill_improvise_db: Chances have to be 1 or above! (%d/%s)\n", i, skill_get_name(i));
		return false;
	}
	if( current >= MAX_SKILL_IMPROVISE_DB ) {
		ShowError("skill_improvise_db: Maximum amount of entries reached (%d), increase MAX_SKILL_IMPROVISE_DB\n",MAX_SKILL_IMPROVISE_DB);
	}
	skill_improvise_db[current].skillid = i;
	skill_improvise_db[current].per = j; // Still need confirm it.

	return true;
}
static bool skill_parse_row_magicmushroomdb(char* split[], int column, int current)
{
	int i = atoi(split[0]);

	if( !skill_get_index(i) || !skill_get_max(i) )
	{
		ShowError("magicmushroom_db: Invalid skill ID %d\n", i);
		return false;
	}
	if ( !skill_get_inf(i) )
	{
		ShowError("magicmushroom_db: Passive skills cannot be casted (%d/%s)\n", i, skill_get_name(i));
		return false;
	}

	skill_magicmushroom_db[current].skillid = i;

	return true;
}

static bool skill_parse_row_reproducedb(char* split[], int column, int current) {
	int skillid = atoi(split[0]);
	
	skillid = skill_get_index(skillid);
	if( !skillid )
		return false;

	skill_reproduce_db[skillid] = true;

	return true;
}


static bool skill_parse_row_abradb(char* split[], int columns, int current)
{// SkillID,DummyName,RequiredHocusPocusLevel,Rate
	int i = atoi(split[0]);
	if( !skill_get_index(i) || !skill_get_max(i) )
	{
		ShowError("abra_db: Invalid skill ID %d\n", i);
		return false;
	}
	if ( !skill_get_inf(i) )
	{
		ShowError("abra_db: Passive skills cannot be casted (%d/%s)\n", i, skill_get_name(i));
		return false;
	}

	skill_abra_db[current].skillid = i;
	skill_abra_db[current].req_lv = atoi(split[2]);
	skill_abra_db[current].per = atoi(split[3]);

	return true;
}

static bool skill_parse_row_changematerialdb(char* split[], int columns, int current)
{// SkillID
	int i = atoi(split[0]);
	short j = atoi(split[1]);
	int x,y;

	for(x=0; x<MAX_SKILL_PRODUCE_DB; x++){
		if( skill_produce_db[x].nameid == i )
			if( skill_produce_db[x].req_skill == GN_CHANGEMATERIAL )
				break;
	}

	if( x >= MAX_SKILL_PRODUCE_DB ){
		ShowError("changematerial_db: Not supported item ID(%d) for Change Material. \n", i);
		return false;
	}

	if( current >= MAX_SKILL_PRODUCE_DB ) {
		ShowError("skill_changematerial_db: Maximum amount of entries reached (%d), increase MAX_SKILL_PRODUCE_DB\n",MAX_SKILL_PRODUCE_DB);
	}

	skill_changematerial_db[current].itemid = i;
	skill_changematerial_db[current].rate = j;

	for( x = 2, y = 0; x+1 < columns && split[x] && split[x+1] && y < 5; x += 2, y++ )
	{
		skill_changematerial_db[current].qty[y] = atoi(split[x]);
		skill_changematerial_db[current].qty_rate[y] = atoi(split[x+1]);
	}

	return true;
}

static void skill_readdb(void)
{
	// init skill db structures
	db_clear(skilldb_name2id);
	memset(skill_db,0,sizeof(skill_db));
	memset(skill_produce_db,0,sizeof(skill_produce_db));
	memset(skill_arrow_db,0,sizeof(skill_arrow_db));
	memset(skill_abra_db,0,sizeof(skill_abra_db));
	memset(skill_spellbook_db,0,sizeof(skill_spellbook_db));
	memset(skill_magicmushroom_db,0,sizeof(skill_magicmushroom_db));
	memset(skill_reproduce_db,0,sizeof(skill_reproduce_db));
	memset(skill_changematerial_db,0,sizeof(skill_changematerial_db));

	// load skill databases
	safestrncpy(skill_db[0].name, "UNKNOWN_SKILL", sizeof(skill_db[0].name));
	safestrncpy(skill_db[0].desc, "Unknown Skill", sizeof(skill_db[0].desc));

	sv_readdb(db_path, DBPATH"skill_db.txt"          , ',',  17, 17, MAX_SKILL_DB, skill_parse_row_skilldb);
	sv_readdb(db_path, DBPATH"skill_require_db.txt"  , ',',  32, 32, MAX_SKILL_DB, skill_parse_row_requiredb);
#ifdef RENEWAL_CAST
	sv_readdb(db_path, "re/skill_cast_db.txt"     , ',',   8,  8, MAX_SKILL_DB, skill_parse_row_castdb);
#else
	sv_readdb(db_path, "pre-re/skill_cast_db.txt"     , ',',   7,  7, MAX_SKILL_DB, skill_parse_row_castdb);
#endif
	sv_readdb(db_path, DBPATH"skill_castnodex_db.txt", ',',   2,  3, MAX_SKILL_DB, skill_parse_row_castnodexdb);
	sv_readdb(db_path, DBPATH"skill_unit_db.txt"     , ',',   8,  8, MAX_SKILL_DB, skill_parse_row_unitdb);

	sv_readdb(db_path, DBPATH"skill_nocast_db.txt"   , ',',   2,  2, MAX_SKILL_DB, skill_parse_row_nocastdb);

	skill_init_unit_layout();
	sv_readdb(db_path, "produce_db.txt"        , ',',   4,  4+2*MAX_PRODUCE_RESOURCE, MAX_SKILL_PRODUCE_DB, skill_parse_row_producedb);
	sv_readdb(db_path, "create_arrow_db.txt"   , ',', 1+2,  1+2*MAX_ARROW_RESOURCE, MAX_SKILL_ARROW_DB, skill_parse_row_createarrowdb);
	sv_readdb(db_path, "abra_db.txt"           , ',',   4,  4, MAX_SKILL_ABRA_DB, skill_parse_row_abradb);
	//Warlock
	sv_readdb(db_path, "spellbook_db.txt"      , ',',   3,  3, MAX_SKILL_SPELLBOOK_DB, skill_parse_row_spellbookdb);
	//Guillotine Cross
	sv_readdb(db_path, "magicmushroom_db.txt"  , ',',   1,  1, MAX_SKILL_MAGICMUSHROOM_DB, skill_parse_row_magicmushroomdb);
	sv_readdb(db_path, "skill_reproduce_db.txt", ',',   1,  1, MAX_SKILL_DB, skill_parse_row_reproducedb);
	sv_readdb(db_path, "skill_improvise_db.txt"      , ',',   2,  2, MAX_SKILL_IMPROVISE_DB, skill_parse_row_improvisedb);
	sv_readdb(db_path, "skill_changematerial_db.txt"      , ',',   4,  4+2*5, MAX_SKILL_PRODUCE_DB, skill_parse_row_changematerialdb);

}

void skill_reload (void)
{
	skill_readdb();
}

/*==========================================
 *
 *------------------------------------------*/
int do_init_skill (void)
{
	skilldb_name2id = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_RELEASE_DATA, 0);
	skill_readdb();

	group_db = idb_alloc(DB_OPT_BASE);
	skillunit_db = idb_alloc(DB_OPT_BASE);
	skillcd_db = idb_alloc(DB_OPT_RELEASE_DATA);
	skillusave_db = idb_alloc(DB_OPT_RELEASE_DATA);
	skill_unit_ers = ers_new(sizeof(struct skill_unit_group),"skill.c::skill_unit_ers",ERS_OPT_NONE);
	skill_timer_ers  = ers_new(sizeof(struct skill_timerskill),"skill.c::skill_timer_ers",ERS_OPT_NONE);

	add_timer_func_list(skill_unit_timer,"skill_unit_timer");
	add_timer_func_list(skill_castend_id,"skill_castend_id");
	add_timer_func_list(skill_castend_pos,"skill_castend_pos");
	add_timer_func_list(skill_timerskill,"skill_timerskill");
	add_timer_func_list(skill_blockpc_end, "skill_blockpc_end");

	add_timer_interval(gettick()+SKILLUNITTIMER_INTERVAL,skill_unit_timer,0,0,SKILLUNITTIMER_INTERVAL);

	return 0;
}

int do_final_skill(void)
{
	db_destroy(skilldb_name2id);
	db_destroy(group_db);
	db_destroy(skillunit_db);
	db_destroy(skillcd_db);
	db_destroy(skillusave_db);
	ers_destroy(skill_unit_ers);
	ers_destroy(skill_timer_ers);
	return 0;
}
