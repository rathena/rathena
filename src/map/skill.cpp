// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "skill.hpp"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <common/cbasetypes.hpp>
#include <common/ers.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>
#include <common/utils.hpp>

#include "achievement.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "chrif.hpp"
#include "clif.hpp"
#include "date.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"
#include "script.hpp"
#include "status.hpp"
#include "unit.hpp"

using namespace rathena;

#define SKILLUNITTIMER_INTERVAL	100
#define TIMERSKILL_INTERVAL	150

static struct eri *skill_timer_ers = nullptr; //For handling skill_timerskills [Skotlex]
static DBMap* bowling_db = nullptr; // int mob_id -> struct mob_data*

DBMap* skillunit_db = nullptr; // int id -> struct skill_unit*

/**
 * Skill Unit Persistency during endack routes (mostly for songs see bugreport:4574)
 */
DBMap* skillusave_db = nullptr; // char_id -> struct skill_usave
struct skill_usave {
	uint16 skill_id, skill_lv;
};

struct s_skill_produce_db skill_produce_db[MAX_SKILL_PRODUCE_DB];
static unsigned short skill_produce_count;

AbraDatabase abra_db;
ReadingSpellbookDatabase reading_spellbook_db;
SkillArrowDatabase skill_arrow_db;

#define MAX_SKILL_CHANGEMATERIAL_DB 75
#define MAX_SKILL_CHANGEMATERIAL_SET 3
struct s_skill_changematerial_db {
	t_itemid nameid;
	unsigned short rate;
	unsigned short qty[MAX_SKILL_CHANGEMATERIAL_SET];
	unsigned short qty_rate[MAX_SKILL_CHANGEMATERIAL_SET];
};
struct s_skill_changematerial_db skill_changematerial_db[MAX_SKILL_CHANGEMATERIAL_DB];
static unsigned short skill_changematerial_count;


MagicMushroomDatabase magic_mushroom_db;

struct s_skill_unit_layout skill_unit_layout[MAX_SKILL_UNIT_LAYOUT];
int firewall_unit_pos;
int icewall_unit_pos;
int earthstrain_unit_pos;
int firerain_unit_pos;
int wallofthorn_unit_pos;

struct s_skill_nounit_layout skill_nounit_layout[MAX_SKILL_UNIT_LAYOUT2];

static char dir_ka = -1; // Holds temporary direction to the target for SR_KNUCKLEARROW

//Early declaration
bool skill_strip_equip(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv);
static int skill_check_unit_range (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv);
static int skill_check_unit_range2 (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv, bool isNearNPC);
static int skill_destroy_trap( struct block_list *bl, va_list ap );
static int skill_check_condition_mob_master_sub (struct block_list *bl, va_list ap);
static bool skill_check_condition_sc_required( map_session_data& sd, uint16 skill_id, s_skill_condition& require );
static bool skill_check_unit_movepos(uint8 check_flag, struct block_list *bl, short dst_x, short dst_y, int easy, bool checkpath);

// Use this function for splash skills that can't hit icewall when cast by players
static inline int splash_target(struct block_list* bl) {
	return ( bl->type == BL_MOB ) ? BL_SKILL|BL_CHAR : BL_CHAR;
}

/**
 * Get skill id from name
 * @param name
 * @return Skill ID of the skill, or 0 if not found.
 **/
uint16 skill_name2id(const char* name) {
	if (name == nullptr)
		return 0;

	for (const auto &it : skill_db) {
		if (strcmpi(it.second->name, name) == 0)
			return it.first;
	}

	return 0;
}

/**
 * Get Skill name
 * @param skill_id
 * @return AEGIS Skill name
 **/
const char* skill_get_name( uint16 skill_id ) {
	return skill_db.find(skill_id)->name;
}

/**
 * Get Skill name
 * @param skill_id
 * @return English Skill name
 **/
const char* skill_get_desc( uint16 skill_id ) {
	return skill_db.find(skill_id)->desc;
}

static bool skill_check(uint16 id) {
	if (id == 0 || skill_get_index(id) == 0)
		return false;
	return true;
}

#define skill_get(id, var) do {\
	if (!skill_check(id))\
		return 0;\
	return var;\
} while(0)

#define skill_get_lv(id, lv, arrvar) do {\
	if (!skill_check(id))\
		return 0;\
	int lv_idx = min(lv, MAX_SKILL_LEVEL) - 1;\
	if (lv > MAX_SKILL_LEVEL && arrvar[lv_idx] > 1 && lv_idx > 1) {\
		int a__ = arrvar[lv_idx - 2];\
		int b__ = arrvar[lv_idx - 1];\
		int c__ = arrvar[lv_idx];\
		return (c__ + ((lv - MAX_SKILL_LEVEL + 1) * (b__ - a__) / 2) + ((lv - MAX_SKILL_LEVEL) * (c__ - b__) / 2));\
	}\
	return arrvar[lv_idx];\
} while(0)

// Skill DB
e_damage_type skill_get_hit( uint16 skill_id )                     { if (!skill_check(skill_id)) return DMG_NORMAL; return skill_db.find(skill_id)->hit; }
int skill_get_inf( uint16 skill_id )                               { skill_get(skill_id, skill_db.find(skill_id)->inf); }
int skill_get_ele( uint16 skill_id , uint16 skill_lv )             { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->element); }
int skill_get_max( uint16 skill_id )                               { skill_get(skill_id, skill_db.find(skill_id)->max); }
int skill_get_range( uint16 skill_id , uint16 skill_lv )           { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->range); }
int skill_get_splash_( uint16 skill_id , uint16 skill_lv )         { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->splash);  }
int skill_get_num( uint16 skill_id ,uint16 skill_lv )              { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->num); }
int skill_get_cast( uint16 skill_id ,uint16 skill_lv )             { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->cast); }
int skill_get_delay( uint16 skill_id ,uint16 skill_lv )            { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->delay); }
int skill_get_walkdelay( uint16 skill_id ,uint16 skill_lv )        { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->walkdelay); }
int skill_get_time( uint16 skill_id ,uint16 skill_lv )             { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->upkeep_time); }
int skill_get_time2( uint16 skill_id ,uint16 skill_lv )            { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->upkeep_time2); }
int skill_get_castdef( uint16 skill_id )                           { skill_get(skill_id, skill_db.find(skill_id)->cast_def_rate); }
int skill_get_castcancel( uint16 skill_id )                        { skill_get(skill_id, skill_db.find(skill_id)->castcancel); }
int skill_get_maxcount( uint16 skill_id ,uint16 skill_lv )         { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->maxcount); }
int skill_get_blewcount( uint16 skill_id ,uint16 skill_lv )        { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->blewcount); }
int skill_get_castnodex( uint16 skill_id )                         { skill_get(skill_id, skill_db.find(skill_id)->castnodex); }
int skill_get_delaynodex( uint16 skill_id )                        { skill_get(skill_id, skill_db.find(skill_id)->delaynodex); }
int skill_get_nocast ( uint16 skill_id )                           { skill_get(skill_id, skill_db.find(skill_id)->nocast); }
int skill_get_type( uint16 skill_id )                              { skill_get(skill_id, skill_db.find(skill_id)->skill_type); }
int skill_get_unit_id ( uint16 skill_id )                          { skill_get(skill_id, skill_db.find(skill_id)->unit_id); }
int skill_get_unit_id2 ( uint16 skill_id )                         { skill_get(skill_id, skill_db.find(skill_id)->unit_id2); }
int skill_get_unit_interval( uint16 skill_id )                     { skill_get(skill_id, skill_db.find(skill_id)->unit_interval); }
int skill_get_unit_range( uint16 skill_id, uint16 skill_lv )       { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->unit_range); }
int skill_get_unit_target( uint16 skill_id )                       { skill_get(skill_id, skill_db.find(skill_id)->unit_target&BCT_ALL); }
int skill_get_unit_bl_target( uint16 skill_id )                    { skill_get(skill_id, skill_db.find(skill_id)->unit_target&BL_ALL); }
int skill_get_unit_layout_type( uint16 skill_id ,uint16 skill_lv ) { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->unit_layout_type); }
int skill_get_cooldown( uint16 skill_id, uint16 skill_lv )         { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->cooldown); }
int skill_get_giveap( uint16 skill_id, uint16 skill_lv )           { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->giveap); }
#ifdef RENEWAL_CAST
int skill_get_fixed_cast( uint16 skill_id ,uint16 skill_lv )       { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->fixed_cast); }
#endif
// Skill requirements
int skill_get_hp( uint16 skill_id ,uint16 skill_lv )               { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.hp); }
int skill_get_mhp( uint16 skill_id ,uint16 skill_lv )              { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.mhp); }
int skill_get_sp( uint16 skill_id ,uint16 skill_lv )               { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.sp); }
int skill_get_ap( uint16 skill_id, uint16 skill_lv )               { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.ap); }
int skill_get_hp_rate( uint16 skill_id, uint16 skill_lv )          { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.hp_rate); }
int skill_get_sp_rate( uint16 skill_id, uint16 skill_lv )          { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.sp_rate); }
int skill_get_ap_rate(uint16 skill_id, uint16 skill_lv)            { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.ap_rate); }
int skill_get_zeny( uint16 skill_id ,uint16 skill_lv )             { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.zeny); }
int skill_get_weapontype( uint16 skill_id )                        { skill_get(skill_id, skill_db.find(skill_id)->require.weapon); }
int skill_get_ammotype( uint16 skill_id )                          { skill_get(skill_id, skill_db.find(skill_id)->require.ammo); }
int skill_get_ammo_qty( uint16 skill_id, uint16 skill_lv )         { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.ammo_qty); }
int skill_get_state( uint16 skill_id )                             { skill_get(skill_id, skill_db.find(skill_id)->require.state); }
size_t skill_get_status_count( uint16 skill_id )                   { skill_get(skill_id, skill_db.find(skill_id)->require.status.size()); }
int skill_get_spiritball( uint16 skill_id, uint16 skill_lv )       { skill_get_lv(skill_id, skill_lv, skill_db.find(skill_id)->require.spiritball); }
sc_type skill_get_sc(int16 skill_id)                               { if (!skill_check(skill_id)) return SC_NONE; return skill_db.find(skill_id)->sc; }

int skill_get_splash( uint16 skill_id , uint16 skill_lv ) {
	int splash = skill_get_splash_(skill_id, skill_lv);
	if (splash < 0)
		return AREA_SIZE;
	return splash;
}

bool skill_get_nk_(uint16 skill_id, std::vector<e_skill_nk> nk) {
	if( skill_id == 0 ){
		return false;
	}

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (!skill)
		return false;

	for (const auto &nkit : nk) {
		if (skill->nk[nkit])
			return true;
	}

	return false;
}

bool skill_get_inf2_(uint16 skill_id, std::vector<e_skill_inf2> inf2) {
	if( skill_id == 0 ){
		return false;
	}

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (!skill)
		return false;

	for (const auto &inf2it : inf2) {
		if (skill->inf2[inf2it])
			return true;
	}

	return false;
}

bool skill_get_unit_flag_(uint16 skill_id, std::vector<e_skill_unit_flag> unit) {
	if( skill_id == 0 ){
		return false;
	}

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (!skill)
		return false;

	for (const auto &unitit : unit) {
		if (skill->unit_flag[unitit])
			return true;
	}

	return false;
}

int skill_tree_get_max(uint16 skill_id, int b_class) {
	auto skill = skill_tree_db.get_skill_data(b_class, skill_id);
	if (skill != nullptr)
		return skill->max_lv;
	else
		return skill_get_max(skill_id);
}

int skill_frostjoke_scream(struct block_list *bl,va_list ap);
int skill_attack_area(struct block_list *bl,va_list ap);
std::shared_ptr<s_skill_unit_group> skill_locate_element_field(struct block_list *bl); // [Skotlex]
int skill_graffitiremover(struct block_list *bl, va_list ap); // [Valaris]
int skill_greed(struct block_list *bl, va_list ap);
static int skill_cell_overlap(struct block_list *bl, va_list ap);
static int skill_trap_splash(struct block_list *bl, va_list ap);
struct skill_unit_group_tickset *skill_unitgrouptickset_search(struct block_list *bl,std::shared_ptr<s_skill_unit_group> sg,t_tick tick);
static int skill_unit_onplace(struct skill_unit *src,struct block_list *bl,t_tick tick);
int skill_unit_onleft(uint16 skill_id, struct block_list *bl,t_tick tick);
static int skill_unit_effect(struct block_list *bl,va_list ap);
static int skill_bind_trap(struct block_list *bl, va_list ap);

e_cast_type skill_get_casttype (uint16 skill_id) {
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if( skill == nullptr ){
		return CAST_DAMAGE;
	}

	if (skill->inf&(INF_GROUND_SKILL))
		return CAST_GROUND;
	if (skill->inf&INF_SUPPORT_SKILL)
		return CAST_NODAMAGE;

	if (skill->inf&INF_SELF_SKILL) {
		if(skill->inf2[INF2_NOTARGETSELF])
			return CAST_DAMAGE; //Combo skill.
		return CAST_NODAMAGE;
	}
	if (skill->nk[NK_NODAMAGE])
		return CAST_NODAMAGE;
	return CAST_DAMAGE;
}

//Returns actual skill range taking into account attack range and AC_OWL [Skotlex]
int skill_get_range2(struct block_list *bl, uint16 skill_id, uint16 skill_lv, bool isServer) {
	if( bl->type == BL_MOB && battle_config.mob_ai&0x400 )
		return 9; //Mobs have a range of 9 regardless of skill used.

	int32 range = skill_get_range(skill_id, skill_lv);

	if( range < 0 ) {
		if( battle_config.use_weapon_skill_range&bl->type )
			return status_get_range(bl);
		range *=-1;
	}

	if (isServer && range > 14) {
		range = 14; // Server-sided base range can't be above 14
	}

	std::bitset<INF2_MAX> inf2 = skill_db.find(skill_id)->inf2;

	if(inf2[INF2_ALTERRANGEVULTURE] || inf2[INF2_ALTERRANGESNAKEEYE] ){
		if( bl->type == BL_PC ) {
			if(inf2[INF2_ALTERRANGEVULTURE]) range += pc_checkskill((TBL_PC*)bl, AC_VULTURE);
			// added to allow GS skills to be effected by the range of Snake Eyes [Reddozen]
			if(inf2[INF2_ALTERRANGESNAKEEYE]) range += pc_checkskill((TBL_PC*)bl, GS_SNAKEEYE);
		} else
			range += battle_config.mob_eye_range_bonus;
	}
	if(inf2[INF2_ALTERRANGESHADOWJUMP] || inf2[INF2_ALTERRANGERADIUS] || inf2[INF2_ALTERRANGERESEARCHTRAP] ){
		if( bl->type == BL_PC ) {
			if(inf2[INF2_ALTERRANGESHADOWJUMP]) range = skill_get_range(NJ_SHADOWJUMP,pc_checkskill((TBL_PC*)bl,NJ_SHADOWJUMP));
			if(inf2[INF2_ALTERRANGERADIUS]) range += pc_checkskill((TBL_PC*)bl, WL_RADIUS);
			if(inf2[INF2_ALTERRANGERESEARCHTRAP]) {
				int rt_range[11] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5 };

				range += rt_range[pc_checkskill((TBL_PC*)bl, RA_RESEARCHTRAP)];
			}
		}
	}

	if( !range && bl->type != BL_PC )
		return 9; // Enable non players to use self skills on others. [Skotlex]
	return range;
}

/** Copy Referral: dummy skills should point to their source.
 * @param skill_id Dummy skill ID
 * @return Real skill id if found
 **/
unsigned short skill_dummy2skill_id(unsigned short skill_id) {
	switch (skill_id) {
		case AB_DUPLELIGHT_MELEE:
		case AB_DUPLELIGHT_MAGIC:
			return AB_DUPLELIGHT;
		case WL_CHAINLIGHTNING_ATK:
			return WL_CHAINLIGHTNING;
		case WL_TETRAVORTEX_FIRE:
		case WL_TETRAVORTEX_WATER:
		case WL_TETRAVORTEX_WIND:
		case WL_TETRAVORTEX_GROUND:
			return WL_TETRAVORTEX;
		case WL_SUMMON_ATK_FIRE:
			return WL_SUMMONFB;
		case WL_SUMMON_ATK_WIND:
			return WL_SUMMONBL;
		case WL_SUMMON_ATK_WATER:
			return WL_SUMMONWB;
		case WL_SUMMON_ATK_GROUND:
			return WL_SUMMONSTONE;
		case LG_OVERBRAND_BRANDISH:
		case LG_OVERBRAND_PLUSATK:
			return LG_OVERBRAND;
		case WM_REVERBERATION_MELEE:
		case WM_REVERBERATION_MAGIC:
			return WM_REVERBERATION;
		case WM_SEVERE_RAINSTORM_MELEE:
			return WM_SEVERE_RAINSTORM;
		case GN_CRAZYWEED_ATK:
			return GN_CRAZYWEED;
		case GN_HELLS_PLANT_ATK:
			return GN_HELLS_PLANT;
		case GN_SLINGITEM_RANGEMELEEATK:
			return GN_SLINGITEM;
		case RL_R_TRIP_PLUSATK:
			return RL_R_TRIP;
		case NPC_MAXPAIN_ATK:
			return NPC_MAXPAIN;
		case SU_CN_METEOR2:
			return SU_CN_METEOR;
		case SU_SV_ROOTTWIST_ATK:
			return SU_SV_ROOTTWIST;
		case SU_LUNATICCARROTBEAT2:
			return SU_LUNATICCARROTBEAT;
		case NPC_REVERBERATION_ATK:
			return NPC_REVERBERATION;
		case NPC_MAGMA_ERUPTION_DOTDAMAGE:
			return NPC_MAGMA_ERUPTION;
		case NPC_DANCINGBLADE_ATK:
			return NPC_DANCINGBLADE;
		case DK_SERVANTWEAPON_ATK:
			return DK_SERVANTWEAPON;
		case DK_HACKANDSLASHER_ATK:
			return DK_HACKANDSLASHER;
		case AG_DESTRUCTIVE_HURRICANE_CLIMAX:
			return AG_DESTRUCTIVE_HURRICANE;
		case AG_VIOLENT_QUAKE_ATK:
			return AG_VIOLENT_QUAKE;
		case AG_ALL_BLOOM_ATK:
		case AG_ALL_BLOOM_ATK2:
			return AG_ALL_BLOOM;
		case AG_CRYSTAL_IMPACT_ATK:
			return AG_CRYSTAL_IMPACT;
		case AG_ASTRAL_STRIKE_ATK:
			return AG_ASTRAL_STRIKE;
		case AG_CRIMSON_ARROW_ATK:
			return AG_CRIMSON_ARROW;
		case CD_ARBITRIUM_ATK:
			return CD_ARBITRIUM;
		case ABC_CHAIN_REACTION_SHOT_ATK:
			return ABC_CHAIN_REACTION_SHOT;
		case ABC_FROM_THE_ABYSS_ATK:
			return ABC_FROM_THE_ABYSS;
		case BO_ACIDIFIED_ZONE_WATER_ATK:
			return BO_ACIDIFIED_ZONE_WATER;
		case BO_ACIDIFIED_ZONE_GROUND_ATK:
			return BO_ACIDIFIED_ZONE_GROUND;
		case BO_ACIDIFIED_ZONE_WIND_ATK:
			return BO_ACIDIFIED_ZONE_WIND;
		case BO_ACIDIFIED_ZONE_FIRE_ATK:
			return BO_ACIDIFIED_ZONE_FIRE;
		case TR_ROSEBLOSSOM_ATK:
			return TR_ROSEBLOSSOM;
		case EM_ELEMENTAL_BUSTER_FIRE:
		case EM_ELEMENTAL_BUSTER_WATER:
		case EM_ELEMENTAL_BUSTER_WIND:
		case EM_ELEMENTAL_BUSTER_GROUND:
		case EM_ELEMENTAL_BUSTER_POISON:
			return EM_ELEMENTAL_BUSTER;
	}
	return skill_id;
}

/**
 * Check skill unit maxcount
 * @param src: Caster to check against
 * @param x: X location of skill
 * @param y: Y location of skill
 * @param skill_id: Skill used
 * @param skill_lv: Skill level used
 * @param type: Type of unit to check against for battle_config checks
 * @param display_failure: Display skill failure message
 * @return True on skill cast success or false on failure
 */
bool skill_pos_maxcount_check(struct block_list *src, int16 x, int16 y, uint16 skill_id, uint16 skill_lv, enum bl_type type, bool display_failure) {
	if (!src)
		return false;

	struct unit_data *ud = unit_bl2ud(src);
	map_session_data *sd = map_id2sd(src->id);
	int maxcount = 0;
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (!(type&battle_config.skill_reiteration) && skill->unit_flag[UF_NOREITERATION] && skill_check_unit_range(src, x, y, skill_id, skill_lv)) {
		if (sd && display_failure)
			clif_skill_fail( *sd, skill_id );
		return false;
	}
	if (type&battle_config.skill_nofootset && skill->unit_flag[UF_NOFOOTSET] && skill_check_unit_range2(src, x, y, skill_id, skill_lv, false)) {
		if (sd && display_failure)
			clif_skill_fail( *sd, skill_id );
		return false;
	}
	if (type&battle_config.land_skill_limit && (maxcount = skill_get_maxcount(skill_id, skill_lv)) > 0) {
		unit_skillunit_maxcount(*ud, skill_id, maxcount);

		if (maxcount == 0) {
			if (sd && display_failure)
				clif_skill_fail( *sd, skill_id );
			return false;
		}
	}

	return true;
}

/**
 * Calculates heal value of skill's effect
 * @param src: Unit casting heal
 * @param target: Target of src
 * @param skill_id: Skill ID used
 * @param skill_lv: Skill Level used
 * @param heal: True if it's the heal part or false if it's the damage part of the skill
 * @return modified heal value
 */
int skill_calc_heal(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, bool heal) {
	int skill, hp = 0;
#ifdef RENEWAL
	int hp_bonus = 0;
	double global_bonus = 1;
#endif
	map_session_data *sd = BL_CAST(BL_PC, src);
	map_session_data *tsd = BL_CAST(BL_PC, target);
	status_change *sc, *tsc;

	sc = status_get_sc(src);
	tsc = status_get_sc(target);

	switch( skill_id ) {
#ifndef RENEWAL
		case BA_APPLEIDUN:
			hp = 30 + 5 * skill_lv + (status_get_vit(src) / 2); // HP recovery
			if (sd)
				hp += 5 * pc_checkskill(sd, BA_MUSICALLESSON);
			break;
#endif
		case PR_SANCTUARY:
			hp = (skill_lv > 6) ? 777 : skill_lv * 100;
			break;
		case NPC_EVILLAND:
			hp = (skill_lv > 6) ? 666 : skill_lv * 100;
			break;
		case AB_HIGHNESSHEAL:
#ifdef RENEWAL
			hp = ((status_get_int(src) + status_get_lv(src)) / 5) * 30;

			if (sd && ((skill = pc_checkskill(sd, HP_MEDITATIO)) > 0))
				hp_bonus += skill * 2;
#else
			hp = ((status_get_lv(src) + status_get_int(src)) / 8) * (4 + ((sd ? pc_checkskill(sd, AL_HEAL) : 1) * 8));
			hp = (hp * (17 + 3 * skill_lv)) / 10;
#endif
			break;
		case SU_FRESHSHRIMP:
			hp = (status_get_lv(src) + status_get_int(src)) / 5 * 6;
			break;
		case SU_BUNCHOFSHRIMP:
			hp = (status_get_lv(src) + status_get_int(src)) / 5 * 15;
			break;
		case CD_MEDIALE_VOTUM:// Does the learned level of heal affect this skill?
		case CD_DILECTIO_HEAL:// Same question for this one too. [Rytech]
			//hp = (status_get_lv(src) + status_get_int(src)) / 5 * 30 * pc_checkskill(sd, AL_HEAL) / 10;
			hp = (status_get_lv(src) + status_get_int(src)) / 5 * 30;
#ifdef RENEWAL
			if (sd && ((skill = pc_checkskill(sd, HP_MEDITATIO)) > 0))
				hp_bonus += skill * 2;
#endif
			break;
		default:
			if (skill_lv >= battle_config.max_heal_lv)
				return battle_config.max_heal;
#ifdef RENEWAL
			/**
			 * Renewal Heal Formula
			 * Formula: ( [(Base Level + INT) / 5] x 30 ) x (Heal Level / 10) x (Modifiers) + MATK
			 */
			hp = (status_get_lv(src) + status_get_int(src)) / 5 * 30 * skill_lv / 10;
#else
			hp = (status_get_lv(src) + status_get_int(src)) / 8 * (4 + (skill_lv * 8));
#endif

			if (sd && ((skill = pc_checkskill(sd, HP_MEDITATIO)) > 0))
#ifdef RENEWAL
				hp_bonus += skill * 2;
#else
				hp += hp * skill * 2 / 100;
#endif
			else if (src->type == BL_HOM && (skill = hom_checkskill(((TBL_HOM*)src), HLIF_BRAIN)) > 0)
#ifdef RENEWAL
				hp_bonus += skill * 2;
#else
				hp += hp * skill * 2 / 100;
#endif
			if (sd && tsd && sd->status.partner_id == tsd->status.char_id && (sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.sex == 0)
				hp *= 2;
			break;
	}

	if( (!heal || (target && target->type == BL_MER)) && skill_id != NPC_EVILLAND )
		hp /= 2;

	if (sd) {
		if (pc_checkskill(sd, SU_POWEROFSEA) > 0) {
#ifdef RENEWAL
			hp_bonus += 10;
#else
			hp += hp * 10 / 100;
#endif

			if (pc_checkskill_summoner(sd, SUMMONER_POWER_SEA) >= 20)
#ifdef RENEWAL
				hp_bonus += 20;
#else
				hp += hp * 20 / 100;
#endif
		}

		if ((skill = pc_checkskill(sd, NV_BREAKTHROUGH)) > 0)
#ifdef RENEWAL
			hp_bonus += 2 * skill;
#else
			hp += hp * skill * 2 / 100;
#endif

		if ((skill = pc_checkskill(sd, NV_TRANSCENDENCE)) > 0)
#ifdef RENEWAL
			hp_bonus += 3 * skill;
#else
			hp += hp * skill * 3 / 100;
#endif

		if (skill = pc_skillheal_bonus(sd, skill_id))
#ifdef RENEWAL
			hp_bonus += skill;
#else
			hp += hp * skill / 100;
#endif
	}

	if (tsd && (skill = pc_skillheal2_bonus(tsd, skill_id)))
#ifdef RENEWAL
		hp_bonus += skill;
#else
		hp += hp * skill / 100;
#endif

	if (sc && sc->count) {
		if (sc->getSCE(SC_OFFERTORIUM) && (skill_id == AB_HIGHNESSHEAL || skill_id == AB_CHEAL || skill_id == NPC_CHEAL || skill_id == PR_SANCTUARY || skill_id == AL_HEAL ||
			skill_id == CD_DILECTIO_HEAL || skill_id == CD_MEDIALE_VOTUM ))
#ifdef RENEWAL
			hp_bonus += sc->getSCE(SC_OFFERTORIUM)->val2;
#else
			hp += hp * sc->getSCE(SC_OFFERTORIUM)->val2 / 100;
#endif
		if (sc->getSCE(SC_GLASTHEIM_HEAL) && skill_id != NPC_EVILLAND && skill_id != BA_APPLEIDUN)
#ifdef RENEWAL
			hp_bonus += sc->getSCE(SC_GLASTHEIM_HEAL)->val1;
#else
			hp += hp * sc->getSCE(SC_GLASTHEIM_HEAL)->val1 / 100;
#endif
#ifdef RENEWAL
		if (sc->getSCE(SC_MEDIALE) && skill_id == CD_MEDIALE_VOTUM)
			hp_bonus += sc->getSCE(SC_MEDIALE)->val2;
#endif
	}

	if (tsc && tsc->count) {
		if (skill_id != NPC_EVILLAND && skill_id != BA_APPLEIDUN) {
			if (tsc->getSCE(SC_INCHEALRATE))
#ifdef RENEWAL
				hp_bonus += tsc->getSCE(SC_INCHEALRATE)->val1; //Only affects Heal, Sanctuary and PotionPitcher.(like bHealPower) [Inkfish]
#else
				hp += hp * tsc->getSCE(SC_INCHEALRATE)->val1 / 100;
#endif
			if (tsc->getSCE(SC_GLASTHEIM_HEAL))
#ifdef RENEWAL
				hp_bonus += tsc->getSCE(SC_GLASTHEIM_HEAL)->val2;
#else
				hp += hp * tsc->getSCE(SC_GLASTHEIM_HEAL)->val2 / 100;
#endif
			if (tsc->getSCE(SC_ANCILLA))
#ifdef RENEWAL
				hp_bonus += tsc->getSCE(SC_ANCILLA)->val1;
#else
				hp += hp * tsc->getSCE(SC_ANCILLA)->val1 / 100;
			if (tsc->getSCE(SC_WATER_INSIGNIA) && tsc->getSCE(SC_WATER_INSIGNIA)->val1 == 2)
				hp += hp / 10;
#endif
#ifdef RENEWAL
			if (tsc->getSCE(SC_ASSUMPTIO))
				hp_bonus += tsc->getSCE(SC_ASSUMPTIO)->val1 * 2;
#endif
			if (tsc->getSCE(SC_VITALIZE_POTION))
#ifdef RENEWAL
				hp_bonus += 10;
#else
				hp += hp * 10 / 100;
#endif
		}
	}

#ifdef RENEWAL
	if (hp_bonus)
		hp += hp * hp_bonus / 100;

	// MATK part of the RE heal formula [malufett]
	// Note: in this part matk bonuses from items or skills are not applied
	switch( skill_id ) {
		case BA_APPLEIDUN:
		case PR_SANCTUARY:
		case NPC_EVILLAND:
			break;
		default:
			{
				status_data* status = status_get_status_data(*src);
				int min, max;

				min = status_base_matk_min(src, status, status_get_lv(src));
				max = status_base_matk_max(src, status, status_get_lv(src));
				if( status->rhw.matk > 0 ){
					int wMatk, variance;
					wMatk = status->rhw.matk;
					variance = wMatk * status->rhw.wlv / 10;
					min += wMatk - variance;
					max += wMatk + variance;
				}

				if( sc && sc->getSCE(SC_RECOGNIZEDSPELL) )
					min = max;

				if( sd && sd->right_weapon.overrefine > 0 ){
					min++;
					max += sd->right_weapon.overrefine - 1;
				}

				if(max > min)
					hp += min+rnd()%(max-min);
				else
					hp += min;
			}
	}

	// Global multipliers are applied after the MATK is applied
	if (tsc && tsc->count) {
		if (skill_id != NPC_EVILLAND && skill_id != BA_APPLEIDUN) {
			if (tsc->getSCE(SC_WATER_INSIGNIA) && tsc->getSCE(SC_WATER_INSIGNIA)->val1 == 2)
				global_bonus *= 1.1f;
		}
	}

	if (skill_id == AB_HIGHNESSHEAL)
		global_bonus *= 2 + 0.3f * (skill_lv - 1);
	else if (skill_id == CD_DILECTIO_HEAL)// Description says its 10% on Lv 1 but thats clearly a typo. [Rytech]
		global_bonus *= 1 + 0.15f + 0.05f * skill_lv;
#endif

	if (heal && tsc && tsc->count) {
		uint8 penalty = 0;

		if (tsc->getSCE(SC_CRITICALWOUND))
			penalty += tsc->getSCE(SC_CRITICALWOUND)->val2;
		if (tsc->getSCE(SC_DEATHHURT) && tsc->getSCE(SC_DEATHHURT)->val3 == 1)
			penalty += 20;
		if (tsc->getSCE(SC_NORECOVER_STATE))
			penalty = 100;
		if (penalty > 0) {
#ifdef RENEWAL
			penalty = cap_value(penalty, 1, 100);
			global_bonus *= (100 - penalty) / 100.f;
#else
			hp -= hp * penalty / 100;
#endif
		}
	}

#ifdef RENEWAL
	hp = (int)(hp * global_bonus);

	// Final heal increased by HPlus.
	// Is this the right place for this??? [Rytech]
	if ( sd && status_get_hplus(src) > 0 )
		hp += hp * status_get_hplus(src) / 100;

	return (heal) ? max(1, hp) : hp;
#else
	return hp;
#endif
}

/**
 * Making Plagiarism and Reproduce check their own function
 * Previous prevention for NPC skills, Wedding skills, and INF3_DIS_PLAGIA are removed since we use skill_db.yml [Cydh]
 * @param sd: Player who will copy the skill
 * @param skill_id: Target skill
 * @return 0 - Cannot be copied; 1 - Can be copied by Plagiarism 2 - Can be copied by Reproduce
 * @author Aru - for previous check; Jobbie for class restriction idea; Cydh expands the copyable skill
 */
int8 skill_isCopyable(map_session_data *sd, uint16 skill_id) {
	uint16 skill_idx = skill_get_index(skill_id);

	if (!skill_idx)
		return 0;

	// Only copy skill that player doesn't have or the skill is old clone
	if (sd->status.skill[skill_idx].id != 0 && sd->status.skill[skill_idx].flag != SKILL_FLAG_PLAGIARIZED)
		return 0;

	s_skill_copyable copyable = skill_db.find(skill_id)->copyable;

	//Plagiarism only able to copy skill while SC_PRESERVE is not active and skill is copyable by Plagiarism
	if (copyable.option & SKILL_COPY_PLAGIARISM && pc_checkskill(sd,RG_PLAGIARISM) && !sd->sc.getSCE(SC_PRESERVE))
		return 1;

	//Reproduce can copy skill if SC__REPRODUCE is active and the skill is copyable by Reproduce
	if (copyable.option & SKILL_COPY_REPRODUCE && pc_checkskill(sd,SC_REPRODUCE) && sd->sc.getSCE(SC__REPRODUCE) && sd->sc.getSCE(SC__REPRODUCE)->val1)
		return 2;

	return 0;
}

/**
 * Check if the skill is ok to cast and when.
 * Done before skill_check_condition_castbegin, requirement
 * @param skill_id: Skill ID that casted
 * @param sd: Player who casted
 * @return true: Skill cannot be used, false: otherwise
 * @author [MouseJstr]
 */
bool skill_isNotOk( uint16 skill_id, map_session_data& sd ){
	if (pc_has_permission(&sd,PC_PERM_SKILL_UNCONDITIONAL))
		return false; // can do any damn thing they want

	if (skill_id == AL_TELEPORT && sd.skillitem == skill_id && sd.skillitemlv > 2)
		return false; // Teleport lv 3 bypasses this check.[Inkfish]

	struct map_data *mapdata = map_getmapdata(sd.bl.m);

	if (mapdata->getMapFlag(MF_NOSKILL) && skill_id != ALL_EQSWITCH && !sd.skillitem) //Item skills bypass noskill
		return true;

	// Epoque:
	// This code will compare the player's attack motion value which is influenced by ASPD before
	// allowing a skill to be cast. This is to prevent no-delay ACT files from spamming skills such as
	// AC_DOUBLE which do not have a skill delay and are not regarded in terms of attack motion.
	if (!sd.state.autocast && sd.skillitem != skill_id && sd.canskill_tick &&
		DIFF_TICK(gettick(),sd.canskill_tick) < (sd.battle_status.amotion * (battle_config.skill_amotion_leniency) / 100))
	{// attempted to cast a skill before the attack motion has finished
		return true;
	}

	if (skill_blockpc_get(&sd, skill_id) != -1){
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_SKILLINTERVAL );
		return true;
	}

	/**
	 * It has been confirmed on a official server (thanks to Yommy) that item-cast skills bypass all mapflag restrictions
	 * Also, without this check, an exploit where an item casting + healing (or any other kind buff) isn't deleted after used on a restricted map
	 */
	if( sd.skillitem == skill_id && !sd.skillitem_keep_requirement && !sd.state.abra_flag)
		return false;

	uint32 skill_nocast = skill_get_nocast(skill_id);
	// Check skill restrictions [Celest]
	if( (skill_nocast&1 && !mapdata_flag_vs2(mapdata)) ||
		(skill_nocast&2 && mapdata->getMapFlag(MF_PVP)) ||
		(skill_nocast&4 && mapdata_flag_gvg2_no_te(mapdata)) ||
		(skill_nocast&8 && mapdata->getMapFlag(MF_BATTLEGROUND)) ||
		(skill_nocast&16 && mapdata_flag_gvg2_te(mapdata)) || // WOE:TE
		(mapdata->zone && skill_nocast&(mapdata->zone) && mapdata->getMapFlag(MF_RESTRICTED)) ){
			clif_msg_color(&sd, MSI_IMPOSSIBLE_SKILL_AREA, color_table[COLOR_CYAN]); // This skill cannot be used within this area.
			return true;
	}

	if( sd.sc.getSCE(SC_ALL_RIDING) )
		return true; //You can't use skills while in the new mounts (The client doesn't let you, this is to make cheat-safe)

	switch (skill_id) {
		case AL_WARP:
		case RETURN_TO_ELDICASTES:
		case ALL_GUARDIAN_RECALL:
		case ECLAGE_RECALL:
		case ALL_PRONTERA_RECALL:
		case ALL_GLASTHEIM_RECALL:
		case ALL_THANATOS_RECALL:
		case ALL_LIGHTHALZEN_RECALL:
			if(mapdata->getMapFlag(MF_NOWARP)) {
				clif_skill_teleportmessage( sd, NOTIFY_MAPINFO_CANT_TP );
				return true;
			}
			return false;
		case AL_TELEPORT:
		case NPC_FATALMENACE:
		case SC_DIMENSIONDOOR:
		case ALL_ODINS_RECALL:
		case WE_CALLALLFAMILY:
			if(mapdata->getMapFlag(MF_NOTELEPORT)) {
				clif_skill_teleportmessage( sd, NOTIFY_MAPINFO_CANT_TP );
				return true;
			}
			return false; // gonna be checked in 'skill_castend_nodamage_id'
		case WE_CALLPARTNER:
		case WE_CALLPARENT:
		case WE_CALLBABY:
			if (mapdata->getMapFlag(MF_NOMEMO)) {
				clif_skill_teleportmessage( sd, NOTIFY_MAPINFO_CANT_MEMO );
				return true;
			}
			break;
		case MC_VENDING:
			if (map_getmapflag(sd.bl.m, MF_NOVENDING)) {
				clif_displaymessage(sd.fd, msg_txt(&sd, 276)); // "You can't open a shop on this map"
				clif_skill_fail( sd, skill_id );
				return true;
			}
			if (map_getcell(sd.bl.m, sd.bl.x, sd.bl.y, CELL_CHKNOVENDING)) {
				clif_displaymessage(sd.fd, msg_txt(&sd, 204)); // "You can't open a shop on this cell."
				clif_skill_fail( sd, skill_id );
				return true;
			}
			if (npc_isnear(&sd.bl)) {
				// uncomment to send msg_txt.
				//char output[150];
				//sprintf(output, msg_txt(662), battle_config.min_npc_vendchat_distance);
				//clif_displaymessage(sd.fd, output);
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_THERE_ARE_NPC_AROUND );
				return true;
			}
			break;
		case ALL_BUYING_STORE:
			if( map_getmapflag(sd.bl.m, MF_NOBUYINGSTORE) ) {
				clif_displaymessage (sd.fd, msg_txt(&sd,276)); // "You can't open a shop on this map"
				clif_skill_fail( sd, skill_id );
				return true;
			}
			if( map_getcell(sd.bl.m,sd.bl.x,sd.bl.y,CELL_CHKNOBUYINGSTORE) ) {
				clif_displaymessage (sd.fd, msg_txt(&sd,204)); // "You can't open a shop on this cell."
				clif_skill_fail( sd, skill_id );
				return true;
			}
			if( npc_isnear(&sd.bl) ) {
				// uncomment to send msg_txt.
				//char output[150];
				//sprintf(output, msg_txt(662), battle_config.min_npc_vendchat_distance);
				//clif_displaymessage(sd.fd, output);
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_THERE_ARE_NPC_AROUND );
				return true;
			}
			break;
		case MC_IDENTIFY:
			return false; // always allowed
		case WZ_ICEWALL:
			// noicewall flag [Valaris]
			if (mapdata->getMapFlag(MF_NOICEWALL)) {
				clif_skill_fail( sd, skill_id );
				return true;
			}
			break;
		case GC_DARKILLUSION:
			if( mapdata_flag_gvg2(mapdata) ) {
				clif_skill_fail( sd, skill_id );
				return true;
			}
			break;
		case GD_EMERGENCYCALL:
		case GD_ITEMEMERGENCYCALL:
			if (
				!(battle_config.emergency_call&((is_agit_start())?2:1)) ||
				!(battle_config.emergency_call&(mapdata_flag_gvg2(mapdata)?8:4)) ||
				(battle_config.emergency_call&16 && mapdata->getMapFlag(MF_NOWARPTO) && !(mapdata->getMapFlag(MF_GVG_CASTLE) || mapdata->getMapFlag(MF_GVG_TE_CASTLE)))
			)	{
				clif_skill_fail( sd, skill_id );
				return true;
			}
			break;
		case WM_SIRCLEOFNATURE:
		case WM_SOUND_OF_DESTRUCTION:
		case WM_LULLABY_DEEPSLEEP:
		case WM_GLOOMYDAY:
		case WM_SATURDAY_NIGHT_FEVER:
			if( !mapdata_flag_vs(mapdata) ) {
				clif_skill_teleportmessage( sd, NOTIFY_MAPINFO_CANT_USE_SKILL );	// This skill cannot be used in this area
				return true;
			}
			break;

	}
	return false;
}

/**
 * Check if the homunculus skill is ok to be processed
 * After checking from Homunculus side, also check the master condition
 * @param hd: Homunculus who casted
 * @param skill_id: Skill ID casted
 * @param skill_lv: Skill level casted
 * @return true: Skill cannot be used, false: otherwise
 */
bool skill_isNotOk_hom(struct homun_data *hd, uint16 skill_id, uint16 skill_lv)
{
	nullpo_retr(true, hd);

	int8 spiritball = skill_get_spiritball(skill_id, skill_lv);
	map_session_data* sd = hd->master;
	status_change* sc = status_get_sc(&hd->bl);

	if (!sd)
		return true;

	if (sc && !sc->count)
		sc = nullptr;

	if (util::vector_exists(hd->blockskill, skill_id)) {
		clif_skill_fail(*sd, skill_id, USESKILL_FAIL_SKILLINTERVAL);
		return true;
	}

	switch(skill_id) {
		case HFLI_SBR44:
			if (hom_get_intimacy_grade(hd) <= HOMGRADE_HATE_WITH_PASSION) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_RELATIONGRADE );
				return true;
			}
			break;
		case HVAN_EXPLOSION:
			if (hd->homunculus.intimacy < (unsigned int)battle_config.hvan_explosion_intimate) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_RELATIONGRADE );
				return true;
			}
			break;
		case MH_LIGHT_OF_REGENE: // Must be cordial
			if (hom_get_intimacy_grade(hd) < HOMGRADE_CORDIAL) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_RELATIONGRADE );
				return true;
			}
			break;
		case MH_GOLDENE_FERSE: // Can't be used with Angriff's Modus
			if (sc && sc->getSCE(SC_ANGRIFFS_MODUS))
				return true;
			break;
		case MH_ANGRIFFS_MODUS:
			if (sc && sc->getSCE(SC_GOLDENE_FERSE))
				return true;
			break;
		case MH_TINDER_BREAKER: // Must be in grappling mode
			if (!(sc && sc->getSCE(SC_STYLE_CHANGE) && sc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_GRAPPLING)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_STYLE_CHANGE_GRAPPLER, 1 );
				return true;
			}
			break;
		case MH_SONIC_CRAW: // Must be in fighting mode
			if (!(sc && sc->getSCE(SC_STYLE_CHANGE) && sc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_FIGHTING)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_STYLE_CHANGE_FIGHTER );
				return true;
			}
			break;
		case MH_SILVERVEIN_RUSH:
			if (!(sc && sc->getSCE(SC_STYLE_CHANGE) && sc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_FIGHTING)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_STYLE_CHANGE_FIGHTER );
				return true;
			}
			if (!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == MH_SONIC_CRAW)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_COMBOSKILL, MH_SONIC_CRAW );
				return true;
			}
			break;
		case MH_MIDNIGHT_FRENZY:
			if (!(sc && sc->getSCE(SC_STYLE_CHANGE) && sc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_FIGHTING)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_STYLE_CHANGE_FIGHTER );
				return true;
			}
			if (!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == MH_SILVERVEIN_RUSH)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_COMBOSKILL, MH_SILVERVEIN_RUSH );
				return true;
			}
			break;
		case MH_CBC:
			if (!(sc && sc->getSCE(SC_STYLE_CHANGE) && sc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_GRAPPLING)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_STYLE_CHANGE_GRAPPLER );
				return true;
			}
			if (!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == MH_TINDER_BREAKER)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_COMBOSKILL, MH_TINDER_BREAKER );
				return true;
			}
			break;
		case MH_EQC:
			if (!(sc && sc->getSCE(SC_STYLE_CHANGE) && sc->getSCE(SC_STYLE_CHANGE)->val1 == MH_MD_GRAPPLING)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_STYLE_CHANGE_GRAPPLER );
				return true;
			}
			if (!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == MH_CBC)) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_COMBOSKILL, MH_CBC );
				return true;
			}
			break;
	}
	if (spiritball) {
		if (hd->homunculus.spiritball < spiritball) {
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_SPIRITS, spiritball );
			return true;
		}

		// Blazing And Furious removes all spirit balls after calculating the amount of hits
		if (skill_id != MH_BLAZING_AND_FURIOUS)
			hom_delspiritball(hd, spiritball, 1);
	}

	//Use master's criteria.
	return skill_isNotOk( skill_id, *sd );
}

/**
 * Check if the mercenary skill is ok to be processed
 * After checking from Homunculus side, also check the master condition
 * @param skill_id: Skill ID that casted
 * @param md: Mercenary who casted
 * @return true: Skill cannot be used, false: otherwise
 */
bool skill_isNotOk_mercenary( uint16 skill_id, s_mercenary_data& md ){
	map_session_data* sd = md.master;

	if (sd == nullptr)
		return true;

	if (util::vector_exists(md.blockskill, skill_id)) {
		clif_skill_fail(*sd, skill_id, USESKILL_FAIL_SKILLINTERVAL);
		return true;
	}

	if( md.master != nullptr ){
		return skill_isNotOk( skill_id, *md.master );
	}else{
		return true;
	}
}

/**
 * Check if the skill can be casted near NPC or not
 * @param src Object who casted
 * @param skill_id Skill ID that casted
 * @param skill_lv Skill Lv
 * @param pos_x Position x of the target
 * @param pos_y Position y of the target
 * @return true: Skill cannot be used, false: otherwise
 * @author [Cydh]
 */
bool skill_isNotOk_npcRange(struct block_list *src, uint16 skill_id, uint16 skill_lv, int pos_x, int pos_y) {
	if (!src)
		return false;

	if (src->type == BL_PC && pc_has_permission(BL_CAST(BL_PC,src),PC_PERM_SKILL_UNCONDITIONAL))
		return false;

	//if self skill
	if (skill_get_inf(skill_id)&INF_SELF_SKILL) {
		pos_x = src->x;
		pos_y = src->y;
	}

	if (pos_x <= 0) pos_x = src->x;
	if (pos_y <= 0) pos_y = src->y;

	return skill_check_unit_range2(src,pos_x,pos_y,skill_id,skill_lv,true) != 0;
}

struct s_skill_unit_layout *skill_get_unit_layout(uint16 skill_id, uint16 skill_lv, struct block_list* src, int x, int y)
{
	int pos = skill_get_unit_layout_type(skill_id,skill_lv);
	uint8 dir;

	if (pos < -1 || pos >= MAX_SKILL_UNIT_LAYOUT) {
		ShowError("skill_get_unit_layout: unsupported layout type %d for skill %d (level %d)\n", pos, skill_id, skill_lv);
		pos = cap_value(pos, 0, MAX_SQUARE_LAYOUT); // cap to nearest square layout
	}

	nullpo_retr(nullptr, src);

	//Monsters sometimes deploy more units on level 10
	if (src->type == BL_MOB && skill_lv >= 10) {
		if (skill_id == WZ_WATERBALL)
			pos = 4; //9x9 Area
	}

	if (pos != -1) // simple single-definition layout
		return &skill_unit_layout[pos];

	dir = (src->x == x && src->y == y) ? 6 : map_calc_dir(src,x,y); // 6 - default aegis direction

	if (skill_id == MG_FIREWALL)
		return &skill_unit_layout [firewall_unit_pos + dir];
	else if (skill_id == WZ_ICEWALL)
		return &skill_unit_layout [icewall_unit_pos + dir];
	else if( skill_id == WL_EARTHSTRAIN )
		return &skill_unit_layout [earthstrain_unit_pos + dir];
	else if( skill_id == RL_FIRE_RAIN )
		return &skill_unit_layout[firerain_unit_pos + dir];
	else if( skill_id == GN_WALLOFTHORN )
		return &skill_unit_layout[wallofthorn_unit_pos + dir];

	ShowError("skill_get_unit_layout: unknown unit layout for skill %d (level %d)\n", skill_id, skill_lv);
	return &skill_unit_layout[0]; // default 1x1 layout
}

/** Stores temporary values.
 * Common usages:
 * [0] holds number of targets in area
 * [1] holds the id of the original target
 * [2] counts how many targets have been processed. counter is added in skill_area_sub if the foreach function flag is: flag&(SD_SPLASH|SD_PREAMBLE)
 */
static int skill_area_temp[8];

/*==========================================
 * Add effect to skill when hit succesfully target
 *------------------------------------------*/
int skill_additional_effect( struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, enum damage_lv dmg_lv, t_tick tick ){
	nullpo_ret(src);
	nullpo_ret(bl);

	if(skill_id > 0 && !skill_lv) return 0;	// don't forget auto attacks! - celest

	if( dmg_lv < ATK_BLOCK ) // Don't apply effect if miss.
		return 0;

	map_session_data* sd = BL_CAST( BL_PC, src );
	mob_data* md = BL_CAST( BL_MOB, src );
	map_session_data* dstsd = BL_CAST( BL_PC, bl );
	mob_data* dstmd = BL_CAST( BL_MOB, bl );

	status_change* sc = status_get_sc( src );
	status_change* tsc = status_get_sc( bl );
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*bl);

	// Taekwon combos activate on traps, so we need to check them even for targets that don't have status
	if (sd && skill_id == 0 && !(attack_type&BF_SKILL) && sc) {
		// Chance to trigger Taekwon kicks
		if (sc->getSCE(SC_READYSTORM) &&
			sc_start4(src, src, SC_COMBO, 15, TK_STORMKICK,
				0, 2, 0,
				(2000 - 4 * sstatus->agi - 2 * sstatus->dex)))
			; //Stance triggered
		else if (sc->getSCE(SC_READYDOWN) &&
			sc_start4(src, src, SC_COMBO, 15, TK_DOWNKICK,
				0, 2, 0,
				(2000 - 4 * sstatus->agi - 2 * sstatus->dex)))
			; //Stance triggered
		else if (sc->getSCE(SC_READYTURN) &&
			sc_start4(src, src, SC_COMBO, 15, TK_TURNKICK,
				0, 2, 0,
				(2000 - 4 * sstatus->agi - 2 * sstatus->dex)))
			; //Stance triggered
		else if (sc->getSCE(SC_READYCOUNTER)) { //additional chance from SG_FRIEND [Komurka]
			int rate = 20;
			if (sc->getSCE(SC_SKILLRATE_UP) && sc->getSCE(SC_SKILLRATE_UP)->val1 == TK_COUNTER) {
				rate += rate*sc->getSCE(SC_SKILLRATE_UP)->val2 / 100;
				status_change_end(src, SC_SKILLRATE_UP);
			}
			sc_start4(src, src, SC_COMBO, rate, TK_COUNTER,
				0, 2, 0,
				(2000 - 4 * sstatus->agi - 2 * sstatus->dex))
				; //Stance triggered
		}
	}

	if (!tsc) //skill additional effect is about adding effects to the target...
		//So if the target can't be inflicted with statuses, this is pointless.
		return 0;

	if( sd )
	{ // These statuses would be applied anyway even if the damage was blocked by some skills. [Inkfish]
		if( skill_id != WS_CARTTERMINATION && skill_id != AM_DEMONSTRATION && skill_id != CR_REFLECTSHIELD && skill_id != MS_REFLECTSHIELD && skill_id != GN_HELLS_PLANT_ATK
#ifndef RENEWAL
		&& skill_id != ASC_BREAKER
#endif
		) {
			// Trigger status effects
			for (const auto &it : sd->addeff) {
				int rate = it.rate;
				if( attack_type&BF_LONG ) // Any ranged physical attack takes status arrows into account (Grimtooth...) [DracoRPG]
					rate += it.arrow_rate;
				if( !rate )
					continue;

				if ((it.flag&(ATF_WEAPON|ATF_MAGIC|ATF_MISC)) != (ATF_WEAPON|ATF_MAGIC|ATF_MISC)) {
					// Trigger has attack type consideration.
					if ((it.flag&ATF_WEAPON && attack_type&BF_WEAPON) ||
						(it.flag&ATF_MAGIC && attack_type&BF_MAGIC) ||
						(it.flag&ATF_MISC && attack_type&BF_MISC))
						;
					else
						continue;
				}

				if ((it.flag&(ATF_LONG|ATF_SHORT)) != (ATF_LONG|ATF_SHORT)) {
					// Trigger has range consideration.
					if ((it.flag&ATF_LONG && !(attack_type&BF_LONG)) ||
						(it.flag&ATF_SHORT && !(attack_type&BF_SHORT)))
						continue; //Range Failed.
				}

				if (it.flag&ATF_TARGET)
					status_change_start(src, bl, it.sc, rate, 7, 0, 0, 0, it.duration, SCSTART_NONE, 100);
				if (it.flag&ATF_SELF)
					status_change_start(src, src, it.sc, rate, 7, 0, 0, 0, it.duration, SCSTART_NONE, 100);
			}
		}

		// Enchanting Shadow gives a chance to inflict Shadow Wounds to the enemy.
		if (sc != nullptr) {
			status_change_entry *sce = sc->getSCE(SC_SHADOW_WEAPON);
			unit_data *ud = unit_bl2ud(bl);

			if (sce != nullptr && ud != nullptr && rnd_chance(sce->val1, 100)) {
				unit_addshadowscar(*ud, skill_get_time2(SHC_ENCHANTING_SHADOW, sce->val1));
			}
		}

		if( skill_id ) {
			// Trigger status effects on skills
			for (const auto &it : sd->addeff_onskill) {
				if (skill_id != it.skill_id || !it.rate)
					continue;

				if (it.target&ATF_TARGET)
					status_change_start(src, bl, it.sc, it.rate, 7, 0, 0, 0, it.duration, SCSTART_NONE, 100);
				if (it.target&ATF_SELF)
					status_change_start(src, src, it.sc, it.rate, 7, 0, 0, 0, it.duration, SCSTART_NONE, 100);
			}
			//"While the damage can be blocked by Pneuma, the chance to break armor remains", irowiki. [Cydh]
			if (dmg_lv == ATK_BLOCK && skill_id == AM_ACIDTERROR) {
				sc_start2(src,bl,SC_BLEEDING,(skill_lv*3),skill_lv,src->id,skill_get_time2(skill_id,skill_lv));
#ifdef RENEWAL
				if (skill_break_equip(src,bl, EQP_ARMOR, (1000 * skill_lv + 500) - 1000, BCT_ENEMY))
#else
				if (skill_break_equip(src,bl, EQP_ARMOR, 100*skill_get_time(skill_id,skill_lv), BCT_ENEMY))
#endif
					clif_emotion(bl,ET_HUK);
			}
		}
	}

	if( dmg_lv < ATK_DEF ) // no damage, return;
		return 0;

	switch(skill_id) {
		case 0:
			{ // Normal attacks (no skill used)
				if( attack_type&BF_SKILL )
					break; // If a normal attack is a skill, it's splash damage. [Inkfish]
				if(sd) {
					int skill;

					// Automatic trigger of Blitz Beat
					if (pc_isfalcon(sd) && sd->status.weapon == W_BOW && (skill = pc_checkskill(sd, HT_BLITZBEAT)) > 0 && rnd() % 1000 <= sstatus->luk * 10 / 3 + 1) {
						int rate;

						if ((sd->class_ & MAPID_THIRDMASK) == MAPID_RANGER)
							rate = 5;
						else
							rate = (sd->status.job_level + 9) / 10;

						skill_castend_damage_id(src, bl, HT_BLITZBEAT, (skill < rate) ? skill : rate, tick, SD_LEVEL);
					}
					// Automatic trigger of Warg Strike
					if (pc_iswug(sd) && (skill = pc_checkskill(sd, RA_WUGSTRIKE)) > 0) {
						int rate = sstatus->luk * 10 / 3 + 1;

						if (pc_isfalcon(sd))
							rate = rate / 3;

						if (rnd() % 1000 <= rate)
							skill_castend_damage_id(src, bl, RA_WUGSTRIKE, skill, tick, 0);
					}
					// Automatic trigger of Hawk Rush
					if (pc_isfalcon(sd) && sd->status.weapon == W_BOW && (skill = pc_checkskill(sd, WH_HAWKRUSH)) > 0) {
						int rate = sstatus->con * 10 / 3 + 1;

						rate += rate * (20 * pc_checkskill(sd, WH_NATUREFRIENDLY)) / 100;

						if (rnd() % 1000 <= rate)
							skill_castend_damage_id(src, bl, WH_HAWKRUSH, skill, tick, 0);
					}
					// Gank
					if(dstmd && sd->status.weapon != W_BOW &&
						(skill=pc_checkskill(sd,RG_SNATCHER)) > 0 &&
						(skill*15 + 55) + pc_checkskill(sd,TF_STEAL)*10 > rnd()%1000) {
						if(pc_steal_item(sd,bl,pc_checkskill(sd,TF_STEAL)))
							clif_skill_nodamage(src,*bl,TF_STEAL,skill);
						else
							clif_skill_fail( *sd, RG_SNATCHER );
					}
				}

				if (sc) {
					struct status_change_entry *sce;
					// Enchant Poison gives a chance to poison attacked enemies
					if((sce=sc->getSCE(SC_ENCPOISON))) //Don't use sc_start since chance comes in 1/10000 rate.
						status_change_start(src,bl,SC_POISON,sce->val2, sce->val1,src->id,0,0,
							skill_get_time2(AS_ENCHANTPOISON,sce->val1),SCSTART_NONE);
					// Enchant Deadly Poison gives a chance to deadly poison attacked enemies
					if((sce=sc->getSCE(SC_EDP)))
						sc_start4(src,bl,SC_DPOISON,sce->val2, sce->val1,src->id,0,0,
							skill_get_time2(ASC_EDP,sce->val1));
					if ((sce = sc->getSCE(SC_LUXANIMA)) && rnd() % 100 < sce->val2)
						skill_castend_nodamage_id(src, bl, RK_STORMBLAST, 1, tick, 0);
				}
			}
			break;

	case SM_BASH:
		if( sd && skill_lv > 5 && pc_checkskill(sd,SM_FATALBLOW)>0 ){
			//BaseChance gets multiplied with BaseLevel/50.0; 500/50 simplifies to 10 [Playtester]
			status_change_start(src,bl,SC_STUN,(skill_lv-5)*sd->status.base_level*10,
				skill_lv,0,0,0,skill_get_time2(skill_id,skill_lv),SCSTART_NONE);
		}
		break;

	case MER_CRASH:
		sc_start(src,bl,SC_STUN,(6*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case AS_VENOMKNIFE:
	case AS_SPLASHER:
		sc_start2(src, bl, SC_POISON, 100, skill_lv, src->id, skill_get_time2(skill_id, skill_lv));
		break;

	case TF_POISON:
		if (!sc_start2(src, bl, SC_POISON, (4 * skill_lv + 10), skill_lv, src->id, skill_get_time2(skill_id, skill_lv)) && sd)
			clif_skill_fail( *sd, skill_id );
		break;

	case AS_SONICBLOW:
	case HN_MEGA_SONIC_BLOW:
		if (!map_flag_gvg2(bl->m) && !map_getmapflag(bl->m, MF_BATTLEGROUND) && sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_ASSASIN)
			sc_start(src, bl, SC_STUN, (4 * skill_lv + 20), skill_lv, skill_get_time2(skill_id, skill_lv)); //Link gives double stun chance outside GVG/BG
		else
			sc_start(src, bl, SC_STUN, (2 * skill_lv + 10), skill_lv, skill_get_time2(skill_id, skill_lv));
		break;

	case AS_GRIMTOOTH:
		if (dstmd && !status_has_mode(tstatus,MD_STATUSIMMUNE))
			sc_start(src,bl,SC_QUAGMIRE,100,0,skill_get_time2(skill_id,skill_lv));
		break;

	case WZ_FIREPILLAR:
		unit_set_walkdelay(bl, tick, skill_get_time2(skill_id, skill_lv), 1);
		break;

	case MG_FROSTDIVER:
		if(!sc_start(src,bl,SC_FREEZE,min(skill_lv*3+35,skill_lv+60),skill_lv,skill_get_time2(skill_id,skill_lv)) && sd)
			clif_skill_fail( *sd, skill_id );
		break;

	case WZ_FROSTNOVA:
		sc_start(src,bl,SC_FREEZE,skill_lv*5+33,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case WZ_STORMGUST:
		// Storm Gust counter was dropped in renewal
#ifdef RENEWAL
		sc_start(src,bl,SC_FREEZE,65-(5*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
#else
		//On third hit, there is a 150% to freeze the target
		if(tsc->sg_counter >= 3 &&
			sc_start(src,bl,SC_FREEZE,150,skill_lv,skill_get_time2(skill_id,skill_lv)))
			tsc->sg_counter = 0;
		// Being it only resets on success it'd keep stacking and eventually overflowing on mvps, so we reset at a high value
		else if( tsc->sg_counter > 250 )
			tsc->sg_counter = 0;
#endif
		break;

	case NPC_STORMGUST2:
		if (skill_lv == 1)
			sc_start(src,bl,SC_FREEZE,10,skill_lv,skill_get_time2(skill_id,skill_lv));
		else if (skill_lv == 2)
			sc_start(src,bl,SC_FREEZE,7,skill_lv,skill_get_time2(skill_id,skill_lv));
		else
			sc_start(src,bl,SC_FREEZE,3,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case WZ_METEOR:
	case HN_METEOR_STORM_BUSTER:
		sc_start(src,bl,SC_STUN,3*skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case WZ_VERMILION:
#ifdef RENEWAL
		sc_start(src,bl,SC_BLIND,10 + 5 * skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
#else
		sc_start(src,bl,SC_BLIND,min(4*skill_lv,40),skill_lv,skill_get_time2(skill_id,skill_lv));
#endif
		break;

	case WZ_HEAVENDRIVE:
		status_change_end(bl, SC_SV_ROOTTWIST);
		break;

	case HT_FREEZINGTRAP:
	case MA_FREEZINGTRAP:
		sc_start(src,bl,SC_FREEZE,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case HT_FLASHER:
		sc_start(src,bl,SC_BLIND,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case HT_LANDMINE:
	case MA_LANDMINE:
		sc_start(src,bl,SC_STUN,10,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case HT_SHOCKWAVE:
		status_percent_damage(src, bl, 0, -(15*skill_lv+5), false);
		break;

	case HT_SANDMAN:
	case MA_SANDMAN:
		sc_start(src,bl,SC_SLEEP,(10*skill_lv+40),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case TF_SPRINKLESAND:
		sc_start(src,bl,SC_BLIND,20,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case TF_THROWSTONE:
		if (!sc_start(src,bl,SC_STUN,3,skill_lv,skill_get_time(skill_id,skill_lv))) //only blind if success
			sc_start(src,bl,SC_BLIND,3,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case NPC_DARKCROSS:
	case CR_HOLYCROSS:
		sc_start(src,bl,SC_BLIND,3*skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case NPC_GRANDDARKNESS:
		sc_start(src, bl, SC_BLIND, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;

	case CR_GRANDCROSS:
		//Chance to cause blind status vs demon and undead element, but not against players
		if(!dstsd && (battle_check_undead(tstatus->race,tstatus->def_ele) || tstatus->race == RC_DEMON))
			sc_start(src,bl,SC_BLIND,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case AM_ACIDTERROR:
		sc_start2(src,bl,SC_BLEEDING,(skill_lv*3),skill_lv,src->id,skill_get_time2(skill_id,skill_lv));
#ifdef RENEWAL
		if (skill_break_equip(src,bl, EQP_ARMOR, (1000 * skill_lv + 500) - 1000, BCT_ENEMY))
#else
		if (skill_break_equip(src,bl, EQP_ARMOR, 100*skill_get_time(skill_id,skill_lv), BCT_ENEMY))
#endif
			clif_emotion(bl,ET_HUK);
		break;

	case AM_DEMONSTRATION:
#ifdef RENEWAL
		skill_break_equip(src,bl, EQP_WEAPON, 300 * skill_lv, BCT_ENEMY);
#else
		skill_break_equip(src,bl, EQP_WEAPON, 100*skill_lv, BCT_ENEMY);
#endif
		break;

	case CR_SHIELDCHARGE:
		sc_start(src,bl,SC_STUN,(15+skill_lv*5),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

#ifndef RENEWAL
	case PA_PRESSURE:
		status_percent_damage(src, bl, 0, 15+5*skill_lv, false);
		[[fallthrough]];
	case HW_GRAVITATION:
		//Pressure and Gravitation can trigger physical autospells
		attack_type |= BF_NORMAL;
		attack_type |= BF_WEAPON;
		break;
#endif

	case RG_RAID:
		sc_start(src,bl,SC_STUN,(10+3*skill_lv),skill_lv,skill_get_time(skill_id,skill_lv));
		sc_start(src,bl,SC_BLIND,(10+3*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
#ifdef RENEWAL
		sc_start(src, bl, SC_RAID, 100, skill_lv, 10000); // Hardcoded to 10 seconds since Duration1 and Duration2 are used
		break;

	case RG_BACKSTAP:
		sc_start(src,bl,SC_STUN,(5+2*skill_lv),skill_lv,skill_get_time(skill_id,skill_lv));
#endif
		break;

	case BA_FROSTJOKER:
	case DC_SCREAM:
	{
		int rate = 150 + 50 * skill_lv; // Aegis accuracy (1000 = 100%)
		int duration = skill_get_time2(skill_id, skill_lv);
		if (skill_id == DC_SCREAM) rate += 100; // DC_SCREAM has a 10% higher base chance
		if (battle_check_target(src, bl, BCT_PARTY) > 0) {
			// On party members: Chance is divided by 4 and BA_FROSTJOKER duration is fixed to 15000ms
			rate /= 4;
			duration = skill_get_time(skill_id, skill_lv);
		}
		status_change_start(src, bl, skill_get_sc(skill_id), rate*10, skill_lv, 0, 0, 0, duration, SCSTART_NONE);
	}
		break;

	case BD_LULLABY:
		status_change_start(src, bl, SC_SLEEP, (sstatus->int_ * 2 + rnd_value(100, 300)) * 10, skill_lv, 0, 0, 0, skill_get_time2(skill_id, skill_lv), SCSTART_NONE);
		break;

#ifdef RENEWAL
	case DC_UGLYDANCE:
		// !TODO: How does caster's DEX/AGI play a role?
		status_zap( bl, 0, 2 * skill_lv + 10 );
		break;
#else
	case DC_UGLYDANCE: {
		int rate = 5 + 5 * skill_lv;
		int skill = pc_checkskill( sd, DC_DANCINGLESSON );

		if( skill > 0 ){
			rate += 5 + skill;
		}

		status_zap( bl, 0, rate );
		} break;
#endif
	case SL_STUN:
		if (tstatus->size==SZ_MEDIUM) //Only stuns mid-sized mobs.
			sc_start(src,bl,SC_STUN,(30+10*skill_lv),skill_lv,skill_get_time(skill_id,skill_lv));
		break;

	case NPC_PETRIFYATTACK:
		sc_start2(src,bl,SC_STONEWAIT,(20*skill_lv),skill_lv,src->id,skill_get_time2(skill_id,skill_lv),skill_get_time(skill_id, skill_lv));
		break;
	case NPC_CURSEATTACK:
		sc_start(src,bl,SC_CURSE,(20*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_SLEEPATTACK:
		sc_start(src,bl,SC_SLEEP,(20*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_BLINDATTACK:
		sc_start(src,bl,SC_BLIND,(20*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_POISON:
		sc_start(src,bl,SC_POISON,(20*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_SILENCEATTACK:
		sc_start(src,bl,SC_SILENCE,(20*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_STUNATTACK:
		sc_start(src,bl,SC_STUN,(20*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_BLEEDING:
		sc_start(src,bl,SC_BLEEDING,(20*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_BLEEDING2:
		sc_start(src,bl,SC_BLEEDING,(50+10*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_CHANGEUNDEAD:
		sc_start(src, bl, SC_CHANGEUNDEAD, (10 * skill_lv), skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case NPC_ACIDBREATH:
		sc_start(src,bl,SC_POISON,70,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_ICEBREATH:
		sc_start(src,bl,SC_FREEZE,70,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_ICEBREATH2:
		sc_start(src,bl,SC_FREEZE,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_MENTALBREAKER:
		{	//Based on observations by Tharis, Mental Breaker should do SP damage
			//equal to Matk*skLevel.
			int rate = sstatus->matk_min;
			if (rate < sstatus->matk_max)
				rate += rnd()%(sstatus->matk_max - sstatus->matk_min);
			rate*=skill_lv;
			status_zap(bl, 0, rate);
			break;
		}
	// Equipment breaking monster skills [Celest]
	case NPC_ARMORBRAKE:
		skill_break_equip(src,bl, EQP_ARMOR, 150*skill_lv, BCT_ENEMY);
		break;
	case NPC_HELMBRAKE:
		skill_break_equip(src,bl, EQP_HELM, 150*skill_lv, BCT_ENEMY);
		break;
	case NPC_SHIELDBRAKE:
		skill_break_equip(src,bl, EQP_SHIELD, 150*skill_lv, BCT_ENEMY);
		break;

	case CH_TIGERFIST: {
		t_tick basetime = skill_get_time(skill_id, skill_lv);
		t_tick mintime = 15 * (status_get_lv(src) + 100);

		if (status_bl_has_mode(bl, MD_STATUSIMMUNE))
			basetime /= 5;
		basetime = std::max((basetime * status_get_agi(bl)) / -200 + basetime, mintime);
		sc_start(src, bl, SC_ANKLE, (1 + skill_lv) * 10, 0, basetime);
	}
		break;

	case LK_SPIRALPIERCE:
	case ML_SPIRALPIERCE:
	case HN_SPIRAL_PIERCE_MAX:
		if( dstsd || ( dstmd && !status_bl_has_mode(bl,MD_STATUSIMMUNE) ) ) //Does not work on status immune
			sc_start(src,bl,SC_ANKLE,100,0,skill_get_time2(skill_id,skill_lv));
		break;

	case ST_REJECTSWORD:
		sc_start(src,bl,SC_AUTOCOUNTER,(skill_lv*15),skill_lv,skill_get_time(skill_id,skill_lv));
		break;

	case PF_FOGWALL:
		if (src != bl && !tsc->getSCE(SC_DELUGE))
			sc_start(src,bl,SC_BLIND,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case LK_HEADCRUSH: //Headcrush has chance of causing Bleeding status, except on demon and undead element
		if (!(battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON))
			sc_start2(src,bl, SC_BLEEDING,50, skill_lv, src->id, skill_get_time2(skill_id,skill_lv));
		break;

	case ASC_METEORASSAULT:
		//Any enemies hit by this skill will receive Stun, Darkness, or external bleeding status ailment with a 5%+5*skill_lv% chance.
		switch(rnd()%3) {
			case 0:
				sc_start(src,bl,SC_BLIND,(5+skill_lv*5),skill_lv,skill_get_time2(skill_id,1));
				break;
			case 1:
				sc_start(src,bl,SC_STUN,(5+skill_lv*5),skill_lv,skill_get_time2(skill_id,2));
				break;
			default:
				sc_start2(src,bl,SC_BLEEDING,(5+skill_lv*5),skill_lv,src->id,skill_get_time2(skill_id,3));
		}
		break;

	case HW_NAPALMVULCAN:
	case HN_NAPALM_VULCAN_STRIKE:
		sc_start(src,bl,SC_CURSE,5*skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case WS_CARTTERMINATION:	// Cart termination
		sc_start(src,bl,SC_STUN,5*skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case CR_ACIDDEMONSTRATION:
	case GN_FIRE_EXPANSION_ACID:
		skill_break_equip(src,bl, EQP_WEAPON|EQP_ARMOR, 100*skill_lv, BCT_ENEMY);
		break;

	case TK_DOWNKICK:
		sc_start(src,bl,SC_STUN,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case TK_JUMPKICK:
		// debuff the following statuses
		if( dstsd && dstsd->class_ != MAPID_SOUL_LINKER && !tsc->getSCE(SC_PRESERVE) ) {
			status_change_end(bl, SC_SPIRIT);
			status_change_end(bl, SC_ADRENALINE2);
			status_change_end(bl, SC_KAITE);
			status_change_end(bl, SC_KAAHI);
			status_change_end(bl, SC_ONEHAND);
			status_change_end(bl, SC_ASPDPOTION2);
			// New soul links confirmed to not dispell with this skill
			// but thats likely a bug since soul links can't stack and
			// soul cutter skill works on them. So ill add this here for now. [Rytech]
			status_change_end(bl, SC_SOULGOLEM);
			status_change_end(bl, SC_SOULSHADOW);
			status_change_end(bl, SC_SOULFALCON);
			status_change_end(bl, SC_SOULFAIRY);
		}
		break;
	case TK_TURNKICK:
	case MO_BALKYOUNG: //Note: attack_type is passed as BF_WEAPON for the actual target, BF_MISC for the splash-affected mobs.
		if(attack_type&BF_MISC) //70% base stun chance...
			sc_start(src,bl,SC_STUN,70,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case GS_BULLSEYE: //0.1% coma rate.
		if(tstatus->race == RC_BRUTE || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_PLAYER_HUMAN || tstatus->race == RC_PLAYER_DORAM)
			status_change_start(src,bl,SC_COMA,10,skill_lv,0,src->id,0,0,SCSTART_NONE);
		break;
	case GS_PIERCINGSHOT:
		sc_start2(src,bl,SC_BLEEDING,(skill_lv*3),skill_lv,src->id,skill_get_time2(skill_id,skill_lv));
		break;
	case NJ_HYOUSYOURAKU:
		sc_start(src,bl,SC_FREEZE,(10+10*skill_lv),skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case GS_FLING:
		sc_start(src,bl,SC_FLING,100, sd?sd->spiritball_old:5,skill_get_time(skill_id,skill_lv));
		break;
	case GS_DISARM:
		skill_strip_equip(src, bl, skill_id, skill_lv);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case NPC_EVILLAND:
		sc_start(src,bl,SC_BLIND,5*skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_HELLJUDGEMENT:
		sc_start(src,bl,SC_CURSE,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_HELLJUDGEMENT2:
		switch(rnd()%6) {
		case 0:
			sc_start(src,bl,SC_SLEEP,100,skill_lv,skill_get_time2(skill_id,skill_lv));
			break;
		case 1:
			sc_start(src,bl,SC_CONFUSION,100,skill_lv,skill_get_time2(skill_id,skill_lv));
			break;
		case 2:
			sc_start(src,bl,SC_HALLUCINATION,100,skill_lv,skill_get_time2(skill_id,skill_lv));
			break;
		case 3:
			sc_start(src,bl,SC_STUN,100,skill_lv,skill_get_time2(skill_id,skill_lv));
			break;
		case 4:
			sc_start(src,bl,SC_FEAR,100,skill_lv,skill_get_time2(skill_id,skill_lv));
			break;
		default:
			sc_start(src,bl,SC_CURSE,100,skill_lv,skill_get_time2(skill_id,skill_lv));
			break;
		}
		break;
	case NPC_CRITICALWOUND:
	case NPC_WIDECRITICALWOUND:
		sc_start(src,bl,SC_CRITICALWOUND,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_FIRESTORM:
		sc_start(src,bl,SC_BURNT,100,skill_lv,skill_get_time(skill_id,skill_lv));
		break;
	case RK_DRAGONBREATH:
		sc_start4(src,bl,SC_BURNING,15,skill_lv,1000,src->id,0,skill_get_time(skill_id,skill_lv));
		break;
	case RK_DRAGONBREATH_WATER:
		sc_start(src,bl,SC_FREEZING,15,skill_lv,skill_get_time(skill_id,skill_lv));
		break;
	case NPC_DRAGONBREATH:
		if (skill_lv > 5)
			sc_start4(src,bl,SC_FREEZING,50,skill_lv,1000,src->id,0,skill_get_time(skill_id,skill_lv));
		else
			sc_start4(src,bl,SC_BURNING,50,skill_lv,1000,src->id,0,skill_get_time(skill_id,skill_lv));
		break;
	case AB_ADORAMUS:
		sc_start(src,bl, SC_ADORAMUS, skill_lv * 4 + (sd ? sd->status.job_level : 50) / 2, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case WL_COMET:
		sc_start(src, bl, SC_MAGIC_POISON, 100, skill_lv, 20000);
		break;
	case NPC_COMET:
		sc_start4(src,bl,SC_BURNING,100,skill_lv,1000,src->id,0,skill_get_time2(skill_id,skill_lv));
		break;
	case NPC_JACKFROST:
		sc_start(src,bl,SC_FREEZE,200,skill_lv,skill_get_time(skill_id,skill_lv));
		break;
	case RA_WUGBITE: {
			int wug_rate = (50 + 10 * skill_lv) + 2 * ((sd) ? pc_checkskill(sd,RA_TOOTHOFWUG)*2 : skill_get_max(RA_TOOTHOFWUG)) - (status_get_agi(bl) / 4);
			if (wug_rate < 50)
				wug_rate = 50;
			sc_start(src,bl, SC_BITE, wug_rate, skill_lv, (skill_get_time(skill_id,skill_lv) + ((sd) ? pc_checkskill(sd,RA_TOOTHOFWUG)*500 : skill_get_max(RA_TOOTHOFWUG))) );
		}
		break;
	case RA_SENSITIVEKEEN:
		if( rnd()%100 < 8 * skill_lv )
			skill_castend_damage_id(src, bl, RA_WUGBITE, ((sd) ? pc_checkskill(sd, RA_WUGBITE) : skill_get_max(RA_WUGBITE)), tick, SD_ANIMATION);
		break;
	case RA_FIRINGTRAP:
		sc_start4(src, bl, SC_BURNING, 50 + skill_lv * 10, skill_lv, 1000, src->id, 0, skill_get_time2(skill_id, skill_lv));
		break;
	case RA_ICEBOUNDTRAP:
		sc_start(src, bl, SC_FREEZING, 50 + skill_lv * 10, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case NC_PILEBUNKER:
		if( rnd()%100 < 25 + 15*skill_lv ) {
			status_change_end(bl, SC_KYRIE);
			status_change_end(bl, SC_ASSUMPTIO);
			status_change_end(bl, SC_STEELBODY);
			status_change_end(bl, SC_GT_CHANGE);
			status_change_end(bl, SC_GT_REVITALIZE);
			status_change_end(bl, SC_AUTOGUARD);
			status_change_end(bl, SC_REFLECTDAMAGE);
			status_change_end(bl, SC_DEFENDER);
			status_change_end(bl, SC_PRESTIGE);
			status_change_end(bl, SC_BANDING);
			status_change_end(bl, SC_MILLENNIUMSHIELD);
		}
		break;
	case NC_FLAMELAUNCHER:
		sc_start(src, bl, SC_BURNING, 20 + 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case NC_COLDSLOWER:
		// Status chances are applied officially through a check
		// The skill first trys to give the frozen status to targets that are hit
		sc_start(src, bl, SC_FREEZE, 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		if (!tsc->getSCE(SC_FREEZE)) // If it fails to give the frozen status, it will attempt to give the freezing status
			sc_start(src, bl, SC_FREEZING, 20 + skill_lv * 10, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case NC_POWERSWING:
		sc_start(src,bl, SC_STUN, 10, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case GC_WEAPONCRUSH:
		skill_castend_nodamage_id(src,bl,skill_id,skill_lv,tick,BCT_ENEMY);
		break;
	case LG_PINPOINTATTACK: {
		int rate = 30 + 5 * ((sd) ? pc_checkskill(sd,LG_PINPOINTATTACK) : skill_lv) + (status_get_agi(src) + status_get_lv(src)) / 10;
		switch( skill_lv ) {
			case 1:
				sc_start2(src,bl,SC_BLEEDING,rate,skill_lv,src->id,skill_get_time(skill_id,skill_lv));
				break;
			case 2:
				skill_break_equip(src, bl, EQP_HELM, rate * 100, BCT_ENEMY);
				break;
			case 3:
				skill_break_equip(src, bl, EQP_SHIELD, rate * 100, BCT_ENEMY);
				break;
			case 4:
				skill_break_equip(src, bl, EQP_ARMOR, rate * 100, BCT_ENEMY);
				break;
			case 5:
				skill_break_equip(src, bl, EQP_WEAPON, rate * 100, BCT_ENEMY);
				break;
		}
		} break;

	case LG_MOONSLASHER:
		sc_start(src,src,SC_OVERBRANDREADY,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case LG_RAYOFGENESIS:	// 50% chance to cause Blind on Undead and Demon monsters.
		if ( battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON )
			sc_start(src,bl, SC_BLIND, 50, skill_lv, skill_get_time(skill_id,skill_lv));
		break;
	case NPC_RAYOFGENESIS:
		if (skill_lv < 8)
			sc_start(src,bl, SC_BLIND, 50, skill_lv, skill_get_time(skill_id,skill_lv));
		else
			sc_start(src,bl, SC_BLIND, 100, skill_lv, skill_get_time(skill_id,skill_lv));
		break;
	case LG_HESPERUSLIT:
		if( pc_checkskill(sd,LG_PINPOINTATTACK) > 0 && sc && sc->getSCE(SC_BANDING) && sc->getSCE(SC_BANDING)->val2 > 5 )
			skill_castend_damage_id(src,bl,LG_PINPOINTATTACK, rnd_value<uint16>(1, pc_checkskill(sd,LG_PINPOINTATTACK)),tick,0);
		break;
	case SR_DRAGONCOMBO:
		sc_start(src,bl, SC_STUN, 1 + skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case SR_WINDMILL:
		if( dstsd )
			skill_addtimerskill(src,tick+status_get_amotion(src),bl->id,0,0,skill_id,skill_lv,BF_WEAPON,0);
		else if( dstmd )
			sc_start(src,bl, SC_STUN, 100, skill_lv, 1000 + 1000 * (rnd() %3));
		break;
	case SR_GENTLETOUCH_QUIET:  //  [(Skill Level x 5) + (Caster?s DEX + Caster?s Base Level) / 10]
		sc_start(src,bl, SC_SILENCE, 5 * skill_lv + (status_get_dex(src) + status_get_lv(src)) / 10, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case SR_EARTHSHAKER:
		if (dstmd != nullptr && dstmd->guardian_data == nullptr)    // Target is a mob (boss included) and not a guardian type. [Atemo]
			sc_start(src, bl, SC_EARTHSHAKER, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		sc_start(src,bl,SC_STUN, 25 + 5 * skill_lv,skill_lv,skill_get_time(skill_id,skill_lv));
		status_change_end(bl, SC_SV_ROOTTWIST);
		break;
	case SO_EARTHGRAVE:
		sc_start2(src,bl, SC_BLEEDING, 5 * skill_lv, skill_lv, src->id, skill_get_time2(skill_id, skill_lv));	// Need official rate. [LimitLine]
		break;
	case SO_DIAMONDDUST: {
		int rate = 5 + 5 * skill_lv;
		if( sc && sc->getSCE(SC_COOLER_OPTION) )
			rate += (sd ? sd->status.job_level / 5 : 0);
		sc_start(src,bl, SC_CRYSTALIZE, rate, skill_lv, skill_get_time2(skill_id, skill_lv));
		} break;
	case SO_VARETYR_SPEAR:
		sc_start(src,bl, SC_STUN, 5 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case SO_POISON_BUSTER:
		sc_start(src,bl, SC_POISON, 5 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case GN_SPORE_EXPLOSION:
		sc_start(src, bl, SC_SPORE_EXPLOSION, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case GN_SLINGITEM_RANGEMELEEATK:
		if( sd ) {
			switch( sd->itemid ) {	// Starting SCs here instead of do it in skill_additional_effect to simplify the code.
				case ITEMID_COCONUT_BOMB:
					sc_start(src,bl, SC_STUN, 5 + sd->status.job_level / 2, skill_lv, 1000 * sd->status.job_level / 3);
					sc_start2(src,bl, SC_BLEEDING, 3 + sd->status.job_level / 2, skill_lv, src->id, 1000 * status_get_lv(src) / 4 + sd->status.job_level / 3);
					break;
				case ITEMID_MELON_BOMB:
					sc_start4(src, bl, SC_MELON_BOMB, 100, skill_lv, 20 + sd->status.job_level, 10 + sd->status.job_level / 2, 0, 1000 * status_get_lv(src) / 4);
					break;
				case ITEMID_BANANA_BOMB:
					{
						uint16 duration = (battle_config.banana_bomb_duration ? battle_config.banana_bomb_duration : 1000 * sd->status.job_level / 4);

						sc_start(src,bl, SC_BANANA_BOMB_SITDOWN, status_get_lv(src) + sd->status.job_level + sstatus->dex / 6 - status_get_lv(bl) - tstatus->agi / 4 - tstatus->luk / 5, skill_lv, duration);
						sc_start(src,bl, SC_BANANA_BOMB, 100, skill_lv, 30000);
						break;
					}
			}
			sd->itemid = 0;
		}
		break;
	case GN_HELLS_PLANT_ATK:
		sc_start(src,bl, SC_STUN,  20 + 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		sc_start2(src,bl, SC_BLEEDING, 5 + 5 * skill_lv, skill_lv, src->id,skill_get_time(skill_id, skill_lv));
		break;
	case EL_WIND_SLASH:	// Non confirmed rate.
		sc_start2(src,bl, SC_BLEEDING, 25, skill_lv, src->id, skill_get_time(skill_id,skill_lv));
		break;
	case EL_STONE_HAMMER:
		sc_start(src, bl, SC_STUN, 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case EL_ROCK_CRUSHER:
		sc_start(src,bl, SC_ROCK_CRUSHER,50,skill_lv,skill_get_time(EL_ROCK_CRUSHER,skill_lv));
		break;
	case EL_ROCK_CRUSHER_ATK:
		sc_start(src,bl,SC_ROCK_CRUSHER_ATK,50,skill_lv,skill_get_time(EL_ROCK_CRUSHER,skill_lv));
		break;
	case EL_TYPOON_MIS:
		sc_start(src,bl,SC_SILENCE,10*skill_lv,skill_lv,skill_get_time(skill_id,skill_lv));
		break;
	case KO_JYUMONJIKIRI:
		sc_start(src,bl,SC_JYUMONJIKIRI,100,skill_lv,skill_get_time(skill_id,skill_lv));
		break;
	case SP_SOULEXPLOSION:
	case KO_SETSUDAN: // Remove soul link when hit.
		status_change_end(bl, SC_SPIRIT);
		status_change_end(bl, SC_SOULGOLEM);
		status_change_end(bl, SC_SOULSHADOW);
		status_change_end(bl, SC_SOULFALCON);
		status_change_end(bl, SC_SOULFAIRY);
		break;
	case KO_MAKIBISHI:
		sc_start(src,bl, SC_STUN, 10 * skill_lv, skill_lv, skill_get_time2(skill_id,skill_lv));
		break;
	case MH_EQC:
		{
			struct homun_data *hd = BL_CAST(BL_HOM, src);

			if (hd) {
				sc_start2(src, bl, SC_STUN, 100, skill_lv, bl->id, 1000 * hd->homunculus.level / 50 + 500 * skill_lv);
				status_change_end(bl, SC_TINDER_BREAKER2);
			}
		}
		break;
	case MH_STAHL_HORN:
		sc_start(src,bl, SC_STUN, 20 + 2 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case MH_NEEDLE_OF_PARALYZE:
		sc_start(src,bl, SC_PARALYSIS, 30 + 5 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case MH_TOXIN_OF_MANDARA:
		sc_start(src,bl, SC_TOXIN_OF_MANDARA, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case MH_XENO_SLASHER:
		sc_start4(src, bl, SC_BLEEDING, skill_lv, skill_lv, src->id, 0, 0, skill_get_time2(skill_id, skill_lv));
		break;
	case NPC_MAGMA_ERUPTION:
	case NC_MAGMA_ERUPTION: // Stun effect from 'slam'
		sc_start(src, bl, SC_STUN, 90, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case NC_MAGMA_ERUPTION_DOTDAMAGE: // Burning effect from 'eruption'
		sc_start4(src, bl, SC_BURNING, 10 * skill_lv, skill_lv, 1000, src->id, 0, skill_get_time2(skill_id, skill_lv));
		break;
	case GN_ILLUSIONDOPING:
		if( sc_start(src,bl,SC_ILLUSIONDOPING,100 - skill_lv * 10,skill_lv,skill_get_time(skill_id,skill_lv)) )
			sc_start(src,bl,SC_HALLUCINATION,100,skill_lv,skill_get_time(skill_id,skill_lv));
		break;

	case RL_MASS_SPIRAL:
		sc_start2(src,bl,SC_BLEEDING,30 + 10 * skill_lv,skill_lv,src->id,skill_get_time(skill_id,skill_lv));
		break;
	case RL_SLUGSHOT:
		sc_start(src,bl,SC_STUN,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case RL_BANISHING_BUSTER: {
			if (!tsc || !tsc->count)
				break;

			if (status_isimmune(bl))
				break;

			if ((dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER) || rnd()%100 >= 50 + 5 * skill_lv) {
				if (sd)
					clif_skill_fail( *sd, skill_id );
				break;
			}

			uint16 n = skill_lv;

			for (const auto &it : status_db) {
				sc_type status = static_cast<sc_type>(it.first);

				if (n <= 0)
					break;
				if (!tsc->getSCE(status))
					continue;
				
				if (it.second->flag[SCF_NOBANISHINGBUSTER])
					continue;

				switch (status) {
					case SC_WHISTLE:		case SC_ASSNCROS:		case SC_POEMBRAGI:
					case SC_APPLEIDUN:		case SC_HUMMING:		case SC_DONTFORGETME:
					case SC_FORTUNE:		case SC_SERVICE4U:
						if (!battle_config.dispel_song || tsc->getSCE(status)->val4 == 0)
							continue; //If in song area don't end it, even if config enabled
						break;
					case SC_ASSUMPTIO:
						if( bl->type == BL_MOB )
							continue;
						break;
				}
				if( status == SC_BERSERK || status == SC_SATURDAYNIGHTFEVER )
					tsc->getSCE(status)->val2 = 0;
				status_change_end(bl,status);
				n--;
			}
			//Remove bonus_script by Banishing Buster
			if (dstsd)
				pc_bonus_script_clear(dstsd,BSF_REM_ON_BANISHING_BUSTER);
		}
		break;
	case RL_S_STORM:
		//kRO update 2014-02-12. Break a headgear by minimum chance 5%/10%/15%/20%/25%
		skill_break_equip(src, bl, EQP_HEAD_TOP, max(skill_lv * 500, (sstatus->dex * skill_lv * 10) - (tstatus->agi * 20)), BCT_ENEMY); //! TODO: Figure out break chance formula
		break;
	case RL_AM_BLAST:
		sc_start(src,bl,SC_ANTI_M_BLAST,20 + 10 * skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case SU_SCRATCH:
		sc_start2(src, bl, SC_BLEEDING, skill_lv * 10 + 70, skill_lv, src->id, skill_get_time(skill_id, skill_lv));
		break;
	case SU_SV_STEMSPEAR:
		sc_start2(src, bl, SC_BLEEDING, 10, skill_lv, src->id, skill_get_time2(skill_id, skill_lv));
		break;
	case SU_CN_METEOR2:
		sc_start(src, bl, SC_CURSE, 20, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case SU_SCAROFTAROU:
		sc_start(src, bl, SC_STUN, 10, skill_lv, skill_get_time2(skill_id, skill_lv)); //! TODO: What's the chance/time?
		break;
	case SU_LUNATICCARROTBEAT2:
		sc_start(src, bl, SC_STUN, 20, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case SJ_FULLMOONKICK:
		sc_start(src, bl, SC_BLIND, 15 + 5 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case SJ_STAREMPEROR:
		sc_start(src, bl, SC_SILENCE, 50 + 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case SP_CURSEEXPLOSION:
		status_change_end(bl, SC_SOULCURSE);
		break;
	case SP_SHA:
		sc_start(src, bl, SC_SP_SHA, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case DK_SERVANT_W_PHANTOM:
		sc_start(src, bl, SC_HANDICAPSTATE_DEEPBLIND, 30 + 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case AG_DESTRUCTIVE_HURRICANE:// Targets hit are dealt a additional hit through Climax.
		if (sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 1)
			skill_castend_damage_id(src, bl, AG_DESTRUCTIVE_HURRICANE_CLIMAX, skill_lv, tick, SD_LEVEL|SD_ANIMATION);
		break;
	case AG_CRYSTAL_IMPACT:// Targets hit are dealt aftershock damage.
		skill_castend_damage_id(src, bl, AG_CRYSTAL_IMPACT_ATK, skill_lv, tick, SD_LEVEL);
		break;
	case IQ_OLEUM_SANCTUM:
		sc_start(src, bl, SC_HOLY_OIL, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case IQ_FIRST_BRAND:
		sc_start(src, bl, SC_FIRST_BRAND, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case IQ_SECOND_FLAME:
	case IQ_SECOND_FAITH:
	case IQ_SECOND_JUDGEMENT:
		sc_start(src, bl, SC_SECOND_BRAND, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case IQ_THIRD_PUNISH:
	case IQ_THIRD_FLAME_BOMB:
	case IQ_THIRD_CONSECRATION:
		status_change_end(bl, SC_SECOND_BRAND);
		break;
	case CD_ARBITRIUM:// Target is Deep Silenced by chance and is then dealt a 2nd splash hit.
		sc_start(src, bl, SC_HANDICAPSTATE_DEEPSILENCE, 20 + 5 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		skill_castend_damage_id(src, bl, CD_ARBITRIUM_ATK, skill_lv, tick, SD_LEVEL);
		break;
	case SHC_FATAL_SHADOW_CROW:
		sc_start( src, bl, SC_DARKCROW, 100, max( 1, pc_checkskill( sd, GC_DARKCROW ) ), skill_get_time( skill_id, skill_lv ) );
		break;
	case ABC_UNLUCKY_RUSH:
		sc_start(src, bl, SC_HANDICAPSTATE_MISFORTUNE, 30 + 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case TR_ROSEBLOSSOM:// Rose blossom seed can only bloom if the target is hit.
		sc_start4(src, bl, SC_ROSEBLOSSOM, 100, skill_lv, TR_ROSEBLOSSOM_ATK, src->id, 0, skill_get_time(skill_id, skill_lv));
		[[fallthrough]];
	case WM_METALICSOUND:
	case WM_REVERBERATION:
		status_change_end(bl, SC_SOUNDBLEND);
		break;
	case EM_DIAMOND_STORM:
		sc_start(src, bl, SC_HANDICAPSTATE_FROSTBITE, 5, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case EM_LIGHTNING_LAND:
		sc_start(src, bl, SC_HANDICAPSTATE_LIGHTNINGSTRIKE, 3, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case EM_VENOM_SWAMP:
		sc_start(src, bl, SC_HANDICAPSTATE_DEADLYPOISON, 3, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case EM_CONFLAGRATION:
		sc_start(src, bl, SC_HANDICAPSTATE_CONFLAGRATION, 3, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case EM_TERRA_DRIVE:
		sc_start(src, bl, SC_HANDICAPSTATE_CRYSTALLIZATION, 5, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case MT_RUSH_QUAKE:
		sc_start( src, bl, SC_RUSH_QUAKE1, 100, skill_lv, skill_get_time( skill_id, skill_lv ) );
		break;
	case HN_SHIELD_CHAIN_RUSH:
	case HN_JACK_FROST_NOVA:
	case HN_GROUND_GRAVITATION:
		sc_start(src, bl, skill_get_sc(skill_id), 100, 0, skill_get_time2(skill_id, skill_lv));
		break;
	} //end switch skill_id

	if (md && battle_config.summons_trigger_autospells && md->master_id && md->special_state.ai && md->special_state.ai != AI_ABR && md->special_state.ai != AI_BIONIC)
	{	//Pass heritage to Master for status causing effects. [Skotlex]
		sd = map_id2sd(md->master_id);
		src = sd?&sd->bl:src;
	}

	if( attack_type&BF_WEAPON )
	{ // Breaking Equipment
		if( sd && battle_config.equip_self_break_rate )
		{	// Self weapon breaking
			int rate = battle_config.equip_natural_break_rate;
#ifndef RENEWAL
			if( sc )
			{
				if(sc->getSCE(SC_OVERTHRUST))
					rate += 10;
				if(sc->getSCE(SC_MAXOVERTHRUST))
					rate += 10;
			}
#endif
			if( rate )
				skill_break_equip(src,src, EQP_WEAPON, rate, BCT_SELF);
		}
		if( battle_config.equip_skill_break_rate && skill_id != WS_CARTTERMINATION && skill_id != ITM_TOMAHAWK )
		{	// Cart Termination/Tomahawk won't trigger breaking data. Why? No idea, go ask Gravity.
			// Target weapon breaking
			int rate = 0;
			if( sd )
				rate += sd->bonus.break_weapon_rate;
			if (sc) {
				if (sc->getSCE(SC_MELTDOWN))
					rate += sc->getSCE(SC_MELTDOWN)->val2;
				if (sc->getSCE(SC_WEAPONBREAKER))
					rate += sc->getSCE(SC_WEAPONBREAKER)->val2;
			}
			if( rate )
				skill_break_equip(src,bl, EQP_WEAPON, rate, BCT_ENEMY);

			// Target armor breaking
			rate = 0;
			if( sd )
				rate += sd->bonus.break_armor_rate;
			if( sc && sc->getSCE(SC_MELTDOWN) )
				rate += sc->getSCE(SC_MELTDOWN)->val3;
			if( rate )
				skill_break_equip(src,bl, EQP_ARMOR, rate, BCT_ENEMY);
		}
		if (sd && !skill_id && bl->type == BL_PC) { // This effect does not work with skills.
			if (sd->def_set_race[tstatus->race].rate)
				status_change_start(src,bl, SC_DEFSET, sd->def_set_race[tstatus->race].rate, sd->def_set_race[tstatus->race].value,
					0, 0, 0, sd->def_set_race[tstatus->race].tick, SCSTART_NOTICKDEF);
			if (sd->mdef_set_race[tstatus->race].rate)
				status_change_start(src,bl, SC_MDEFSET, sd->mdef_set_race[tstatus->race].rate, sd->mdef_set_race[tstatus->race].value,
					0, 0, 0, sd->mdef_set_race[tstatus->race].tick, SCSTART_NOTICKDEF);
			if (sd->norecover_state_race[tstatus->race].rate)
				status_change_start(src, bl, SC_NORECOVER_STATE, sd->norecover_state_race[tstatus->race].rate,
					0, 0, 0, 0, sd->norecover_state_race[tstatus->race].tick, SCSTART_NONE);
		}
	}

	if( sd && sd->ed && sc && !status_isdead(*bl) && !skill_id ) {
		struct unit_data *ud = unit_bl2ud(src);
		int skill;

		if( sc->getSCE(SC_WILD_STORM_OPTION) )
			skill = sc->getSCE(SC_WILD_STORM_OPTION)->val2;
		else if( sc->getSCE(SC_UPHEAVAL_OPTION) )
			skill = sc->getSCE(SC_UPHEAVAL_OPTION)->val3;
		else if( sc->getSCE(SC_TROPIC_OPTION) )
			skill = sc->getSCE(SC_TROPIC_OPTION)->val3;
		else if( sc->getSCE(SC_CHILLY_AIR_OPTION) )
			skill = sc->getSCE(SC_CHILLY_AIR_OPTION)->val3;
		else
			skill = 0;

		if ( rnd()%100 < 25 && skill ){
			skill_castend_damage_id(src, bl, skill, 5, tick, 0);

			if (ud) {
				int delay = skill_delayfix(src, skill, skill_lv);
				if (DIFF_TICK(ud->canact_tick, tick + delay) < 0){
					ud->canact_tick = i64max(tick + delay, ud->canact_tick);
					if ( battle_config.display_status_timers )
						clif_status_change(src, EFST_POSTDELAY, 1, delay, 0, 0, 0);
				}
			}
		}
	}

	// Autospell when attacking
	if( sd && !status_isdead(*bl) && !sd->autospell.empty() )
	{
		for (const auto &it : sd->autospell) {
			if (!(((it.battle_flag)&attack_type)&BF_WEAPONMASK &&
				  ((it.battle_flag)&attack_type)&BF_RANGEMASK &&
				  ((it.battle_flag)&attack_type)&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled

			int skill = it.id;

			sd->state.autocast = 1;
			if ( skill_isNotOk(skill, *sd) ) {
				sd->state.autocast = 0;
				continue;
			}
			sd->state.autocast = 0;

			uint16 autospl_skill_lv = it.lv ? it.lv : 1;

			if (it.flag & AUTOSPELL_FORCE_RANDOM_LEVEL)
				autospl_skill_lv = rnd_value<uint16>( 1, autospl_skill_lv );

			int rate = (!sd->state.arrow_atk) ? it.rate : it.rate / 2;

			if (rnd()%1000 >= rate)
				continue;

			block_list *tbl = (it.flag & AUTOSPELL_FORCE_TARGET) ? bl : src;
			e_cast_type type = skill_get_casttype(skill);

			if (type == CAST_GROUND) {
				if (!skill_pos_maxcount_check(src, tbl->x, tbl->y, skill, autospl_skill_lv, BL_PC, false))
					continue;
			}
			if (battle_config.autospell_check_range &&
				!battle_check_range(bl, tbl, skill_get_range2(src, skill, autospl_skill_lv, true)))
				continue;

			if (skill == PF_SPIDERWEB) //Special case, due to its nature of coding.
				type = CAST_GROUND;
#ifndef RENEWAL
			else if (skill == AS_SONICBLOW)
				pc_stop_attack(sd); //Special case, Sonic Blow autospell should stop the player attacking.
#endif

			sd->state.autocast = 1;
			skill_consume_requirement(sd,skill,autospl_skill_lv,1);
#ifndef RENEWAL
			skill_toggle_magicpower(src, skill);
#endif
			switch (type) {
				case CAST_GROUND:
					skill_castend_pos2(src, tbl->x, tbl->y, skill, autospl_skill_lv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill_castend_nodamage_id(src, tbl, skill, autospl_skill_lv, tick, 0);
					break;
				case CAST_DAMAGE:
					skill_castend_damage_id(src, tbl, skill, autospl_skill_lv, tick, 0);
					break;
			}
			sd->state.autocast = 0;

			//Set canact delay. [Skotlex]
			unit_data *ud = unit_bl2ud(src);

			if (ud) {
				int delay = skill_delayfix(src, skill, autospl_skill_lv);

				if (DIFF_TICK(ud->canact_tick, tick + delay) < 0){
					ud->canact_tick = i64max(tick + delay, ud->canact_tick);
					if ( battle_config.display_status_timers && sd )
						clif_status_change(src, EFST_POSTDELAY, 1, delay, 0, 0, 0);
				}
			}
		}
	}

	// Check for player and pet autobonuses when attacking
	if (sd != nullptr) {
		// Player
		if (!sd->autobonus.empty()) {
			for (auto &it : sd->autobonus) {
				if (it == nullptr)
					continue;
				if (rnd_value(0, 1000) >= it->rate)
					continue;
				if (!(((it->atk_type) & attack_type) & BF_WEAPONMASK &&
					  ((it->atk_type) & attack_type) & BF_RANGEMASK &&
					  ((it->atk_type) & attack_type) & BF_SKILLMASK))
					continue; // one or more trigger conditions were not fulfilled

				pc_exeautobonus(*sd, &sd->autobonus, it);
			}
		}

		// Pet
		if (sd->pd != nullptr && !sd->pd->autobonus.empty()) {
			for (auto &it : sd->pd->autobonus) {
				if (it == nullptr)
					continue;
				if (rnd_value(0, 1000) >= it->rate)
					continue;
				if (!(((it->atk_type) & attack_type) & BF_WEAPONMASK &&
					  ((it->atk_type) & attack_type) & BF_RANGEMASK &&
					  ((it->atk_type) & attack_type) & BF_SKILLMASK))
					continue; // one or more trigger conditions were not fulfilled

				pet_exeautobonus(*sd, &sd->pd->autobonus, it);
			}
		}
	}

	//Polymorph
	if(sd && sd->bonus.classchange && attack_type&BF_WEAPON &&
		dstmd && !status_has_mode(tstatus,MD_STATUSIMMUNE) &&
		(rnd()%10000 < sd->bonus.classchange))
	{
		int class_ = mob_get_random_id(MOBG_BRANCH_OF_DEAD_TREE, RMF_DB_RATE, 0);
		if (class_ != 0 && mobdb_checkid(class_))
			mob_class_change(dstmd,class_);
	}

	if (sd && sc) {
		struct status_change_entry *sce;
		if ((sce = sc->getSCE(SC_2011RWC_SCROLL)) && rnd() % 1000 <= 10)
			skill_castend_nodamage_id(src, src, AC_CONCENTRATION, max(3, pc_checkskill(sd,AC_CONCENTRATION)), tick, 0);
	}

	return 0;
}

int skill_onskillusage(map_session_data *sd, struct block_list *bl, uint16 skill_id, t_tick tick) {
	if( sd == nullptr || !skill_id )
		return 0;

	for (auto &it : sd->autospell3) {
		if (it.trigger_skill != skill_id)
			continue;

		if (it.lock)
			continue;  // autospell already being executed

		uint16 skill = it.id;

		sd->state.autocast = 1; //set this to bypass sd->canskill_tick check

		if( skill_isNotOk(skill, *sd) ) {
			sd->state.autocast = 0;
			continue;
		}

		sd->state.autocast = 0;

		// DANGER DANGER: here force target actually means use yourself as target!
		block_list *tbl = (it.flag & AUTOSPELL_FORCE_TARGET) ? &sd->bl : bl;

		if( tbl == nullptr ){
			continue; // No target
		}

		if( rnd()%1000 >= it.rate )
			continue;

		uint16 skill_lv = it.lv ? it.lv : 1;

		if (it.flag & AUTOSPELL_FORCE_RANDOM_LEVEL)
			skill_lv = rnd_value<uint16>( 1, skill_lv ); //random skill_lv

		e_cast_type type = skill_get_casttype(skill);

		if (type == CAST_GROUND && !skill_pos_maxcount_check(&sd->bl, tbl->x, tbl->y, skill_id, skill_lv, BL_PC, false))
			continue;

		if (battle_config.autospell_check_range &&
			!battle_check_range(bl, tbl, skill_get_range2(&sd->bl, skill, skill_lv, true)))
			continue;

		sd->state.autocast = 1;
		it.lock = true;
		skill_consume_requirement(sd,skill,skill_lv,1);
		switch( type ) {
			case CAST_GROUND:
				skill_castend_pos2(&sd->bl, tbl->x, tbl->y, skill, skill_lv, tick, 0);
				break;
			case CAST_NODAMAGE:
				skill_castend_nodamage_id(&sd->bl, tbl, skill, skill_lv, tick, 0);
				break;
			case CAST_DAMAGE:
				skill_castend_damage_id(&sd->bl, tbl, skill, skill_lv, tick, 0);
				break;
		}
		it.lock = false;
		sd->state.autocast = 0;
	}

	// Check for player and pet autobonuses when being attacked by skill_id
	if (sd != nullptr) {
		// Player
		if (!sd->autobonus3.empty()) {
			for (auto &it : sd->autobonus3) {
				if (it == nullptr)
					continue;
				if (rnd_value(0, 1000) >= it->rate)
					continue;
				if (it->atk_type != skill_id)
					continue;

				pc_exeautobonus(*sd, &sd->autobonus3, it);
			}
		}

		// Pet
		if (sd->pd != nullptr && !sd->pd->autobonus3.empty()) {
			for (auto &it : sd->pd->autobonus3) {
				if (it == nullptr)
					continue;
				if (rnd_value(0, 1000) >= it->rate)
					continue;
				if (it->atk_type != skill_id)
					continue;

				pet_exeautobonus(*sd, &sd->pd->autobonus3, it);
			}
		}
	}

	return 1;
}

/* Splitted off from skill_additional_effect, which is never called when the
 * attack skill kills the enemy. Place in this function counter status effects
 * when using skills that will take effect on the source, not the target. [Skotlex]
 */
int skill_counter_additional_effect (struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, int attack_type, t_tick tick)
{
	int rate;
	map_session_data *sd=nullptr;
	map_session_data *dstsd=nullptr;

	nullpo_ret(src);
	nullpo_ret(bl);

	if(skill_id > 0 && !skill_lv) return 0;	// don't forget auto attacks! - celest

	sd = BL_CAST(BL_PC, src);
	dstsd = BL_CAST(BL_PC, bl);

	if(dstsd && attack_type & BF_WEAPONMASK) {	//Counter effects.
		for (const auto &it : dstsd->addeff_atked) {
			rate = it.rate;
			if (attack_type&BF_LONG)
				rate += it.arrow_rate;
			if (rate == 0)
				continue;

			if ((it.flag&(ATF_WEAPON|ATF_MAGIC|ATF_MISC)) != (ATF_WEAPON|ATF_MAGIC|ATF_MISC)) {
				// Trigger has attack type consideration.
				if ((it.flag&ATF_WEAPON && attack_type&BF_WEAPON) ||
					(it.flag&ATF_MAGIC && attack_type&BF_MAGIC) ||
					(it.flag&ATF_MISC && attack_type&BF_MISC))
					;
				else
					continue;
			}

			if ((it.flag&(ATF_LONG|ATF_SHORT)) != (ATF_LONG|ATF_SHORT)) {	//Trigger has range consideration.
				if((it.flag&ATF_LONG && !(attack_type&BF_LONG)) ||
					(it.flag&ATF_SHORT && !(attack_type&BF_SHORT)))
					continue; //Range Failed.
			}

			if (it.flag&ATF_TARGET && src != bl)
				status_change_start(src, src, it.sc, rate, 7, 0, 0, 0, it.duration, SCSTART_NONE, 100);
			if (it.flag&ATF_SELF && !status_isdead(*bl))
				status_change_start(src, bl, it.sc, rate, 7, 0, 0, 0, it.duration, SCSTART_NONE, 100);
		}
	}

	switch(skill_id) {
	case GS_FULLBUSTER:
		sc_start(src,src,SC_BLIND,2*skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;
	case HFLI_SBR44:	//[orn]
		if(src->type == BL_HOM){
			homun_data& hd = reinterpret_cast<homun_data&>( *src );

			hd.homunculus.intimacy = hom_intimacy_grade2intimacy(HOMGRADE_HATE_WITH_PASSION);

			clif_send_homdata( hd, SP_INTIMATE );
		}
		break;
	case CR_GRANDCROSS:
		if (src == bl) {
			// Grand Cross on self specifically only triggers "When hit by physical attack" autospells and ignores everything else
			attack_type |= BF_WEAPON;
			attack_type &= ~BF_MAGIC;
		}
		break;
	case LG_HESPERUSLIT:
		{
			status_change *sc = status_get_sc(src);

			if( sc && sc->getSCE(SC_FORCEOFVANGUARD)) {
				for(int i = 0; i < sc->getSCE(SC_FORCEOFVANGUARD)->val3; i++ )
					pc_addspiritball(sd, skill_get_time(LG_FORCEOFVANGUARD,1),sc->getSCE(SC_FORCEOFVANGUARD)->val3);
			}
		}
		break;
	case SP_SPA:
		sc_start(src, src, SC_USE_SKILL_SP_SPA, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case SP_SHA:
		sc_start(src, src, SC_USE_SKILL_SP_SHA, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	case SP_SWHOO:
		sc_start(src, src, SC_USE_SKILL_SP_SHA, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	}

	if(sd && (sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR &&
		map_getmapflag(sd->bl.m, MF_NOSUNMOONSTARMIRACLE) == 0) {	//SG_MIRACLE [Komurka]
		// 0.005% chance per sg_miracle_skill_ratio
		// Chance is further reduced if agi is above 46
		if (rnd_chance(battle_config.sg_miracle_skill_ratio, 20000) && rnd_chance(46, (int)sd->battle_status.agi))
			sc_start(src, src, SC_MIRACLE, 100, 1, battle_config.sg_miracle_skill_duration);
	}

	if(sd && skill_id && attack_type&BF_MAGIC && status_isdead(*bl) &&
	 	!(skill_get_inf(skill_id)&(INF_GROUND_SKILL|INF_SELF_SKILL)) &&
		(rate=pc_checkskill(sd,HW_SOULDRAIN))>0
	){	//Soul Drain should only work on targetted spells [Skotlex]
		if (pc_issit(sd)) pc_setstand(sd, true); //Character stuck in attacking animation while 'sitting' fix. [Skotlex]
		clif_skill_nodamage(src,*bl,HW_SOULDRAIN,rate);
		status_heal(src, 0, status_get_lv(bl)*(95+15*rate)/100, 2);
	}

	if( sd && status_isdead(*bl) ) {
		int sp = 0, hp = 0;
		if( (attack_type&(BF_WEAPON|BF_SHORT)) == (BF_WEAPON|BF_SHORT) ) {
			sp += sd->bonus.sp_gain_value;
			sp += sd->indexed_bonus.sp_gain_race[status_get_race(bl)] + sd->indexed_bonus.sp_gain_race[RC_ALL];
			hp += sd->bonus.hp_gain_value;
		}
		if( (attack_type&(BF_WEAPON|BF_LONG)) == (BF_WEAPON|BF_LONG) ) {
			sp += sd->bonus.long_sp_gain_value;
			hp += sd->bonus.long_hp_gain_value;
		}
		if( attack_type&BF_MAGIC ) {
			sp += sd->bonus.magic_sp_gain_value;
			hp += sd->bonus.magic_hp_gain_value;
			if( skill_id == WZ_WATERBALL ) {//(bugreport:5303)
				status_change *sc = nullptr;
				if( ( sc = status_get_sc(src) ) ) {
					if(sc->getSCE(SC_SPIRIT) &&
								sc->getSCE(SC_SPIRIT)->val2 == SL_WIZARD &&
								sc->getSCE(SC_SPIRIT)->val3 == WZ_WATERBALL)
								sc->getSCE(SC_SPIRIT)->val3 = 0; //Clear bounced spell check.
				}
			}
		}
		if( hp || sp ) { // updated to force healing to allow healing through berserk
			status_heal(src, hp, sp, battle_config.show_hp_sp_gain ? 3 : 1);
		}
	}

	if (dstsd && !status_isdead(*bl) && !(skill_id && skill_get_nk(skill_id, NK_NODAMAGE))) {
		status_change *sc = status_get_sc(bl);

		if (sc && sc->getSCE(SC_DORAM_SVSP) && attack_type&(BF_MAGIC|BF_LONG))
			skill_castend_damage_id(bl, src, SU_SV_STEMSPEAR, (pc_checkskill(dstsd, SU_SV_STEMSPEAR) ? pc_checkskill(dstsd, SU_SV_STEMSPEAR) : 1), tick, 0);
	}

	// Trigger counter-spells to retaliate against damage causing skills.
	if(dstsd && !status_isdead(*bl) && !dstsd->autospell2.empty() &&
		!(skill_id && skill_get_nk(skill_id, NK_NODAMAGE)))
	{
		for (const auto &it : dstsd->autospell2) {
			if (!(((it.battle_flag)&attack_type)&BF_WEAPONMASK &&
				  ((it.battle_flag)&attack_type)&BF_RANGEMASK &&
				  ((it.battle_flag)&attack_type)&BF_SKILLMASK))
				continue; // one or more trigger conditions were not fulfilled

			uint16 autospl_skill_id = it.id, autospl_skill_lv = it.lv ? it.lv : 1;

			if (it.flag & AUTOSPELL_FORCE_RANDOM_LEVEL)
				autospl_skill_lv = rnd_value<uint16>( 1, autospl_skill_lv );

			int autospl_rate = it.rate;

			//Physical range attacks only trigger autospells half of the time
			if ((attack_type&(BF_WEAPON|BF_LONG)) == (BF_WEAPON|BF_LONG))
				 autospl_rate /= 2;

			dstsd->state.autocast = 1;
			if ( skill_isNotOk(autospl_skill_id, *dstsd) ) {
				dstsd->state.autocast = 0;
				continue;
			}
			dstsd->state.autocast = 0;

			if (rnd()%1000 >= autospl_rate)
				continue;

			block_list *tbl = (it.flag & AUTOSPELL_FORCE_TARGET) ? src : bl;
			e_cast_type type = skill_get_casttype(autospl_skill_id);

			if (type == CAST_GROUND && !skill_pos_maxcount_check(bl, tbl->x, tbl->y, autospl_skill_id, autospl_skill_lv, BL_PC, false))
					continue;

			if (!battle_check_range(bl, tbl, skill_get_range2(src, autospl_skill_id, autospl_skill_lv, true)) && battle_config.autospell_check_range)
				continue;

			dstsd->state.autocast = 1;
			skill_consume_requirement(dstsd,autospl_skill_id,autospl_skill_lv,1);
			switch (type) {
				case CAST_GROUND:
					skill_castend_pos2(bl, tbl->x, tbl->y, autospl_skill_id, autospl_skill_lv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill_castend_nodamage_id(bl, tbl, autospl_skill_id, autospl_skill_lv, tick, 0);
					break;
				case CAST_DAMAGE:
					skill_castend_damage_id(bl, tbl, autospl_skill_id, autospl_skill_lv, tick, 0);
					break;
			}
			dstsd->state.autocast = 0;

			//Set canact delay. [Skotlex]
			unit_data *ud = unit_bl2ud(bl);

			if (ud) {
				int delay = skill_delayfix(bl, autospl_skill_id, autospl_skill_lv);

				if (DIFF_TICK(ud->canact_tick, tick + delay) < 0){
					ud->canact_tick = i64max(tick + delay, ud->canact_tick);
					if ( battle_config.display_status_timers && dstsd )
						clif_status_change(bl, EFST_POSTDELAY, 1, delay, 0, 0, 0);
				}
			}
		}
	}

	// Check for player and pet autobonuses when attacked
	if (dstsd != nullptr && !status_isdead(*bl) && !(skill_id && skill_get_nk(skill_id, NK_NODAMAGE))) {
		// Player
		if (!dstsd->autobonus2.empty()) {
			for (auto &it : dstsd->autobonus2) {
				if (it == nullptr)
					continue;
				if (rnd_value(0, 1000) >= it->rate)
					continue;
				if (!(((it->atk_type) & attack_type) & BF_WEAPONMASK &&
					  ((it->atk_type) & attack_type) & BF_RANGEMASK &&
					  ((it->atk_type) & attack_type) & BF_SKILLMASK))
					continue; // one or more trigger conditions were not fulfilled

				pc_exeautobonus(*dstsd, &dstsd->autobonus2, it);
			}
		}

		// Pet
		if (dstsd->pd != nullptr && !dstsd->pd->autobonus2.empty()) {
			for (auto &it : dstsd->pd->autobonus2) {
				if (it == nullptr)
					continue;
				if (rnd_value(0, 1000) >= it->rate)
					continue;
				if (!(((it->atk_type) & attack_type) & BF_WEAPONMASK &&
					  ((it->atk_type) & attack_type) & BF_RANGEMASK &&
					  ((it->atk_type) & attack_type) & BF_SKILLMASK))
					continue; // one or more trigger conditions were not fulfilled

				pet_exeautobonus(*dstsd, &dstsd->pd->autobonus2, it);
			}
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
int skill_break_equip(struct block_list *src, struct block_list *bl, unsigned short where, int rate, int flag)
{
	status_change *src_sc = status_get_sc(src);

	// Grant player skills/items the ability to "break" non-player equipment.
	// WS_MELTDOWN is exempt from this check.
	if (!battle_config.break_mob_equip && bl->type != BL_PC && !(src_sc && src_sc->getSCE(SC_MELTDOWN)))
		return 0;

	const int where_list[6]     = { EQP_WEAPON, EQP_ARMOR, EQP_SHIELD, EQP_HELM, EQP_ACC, EQP_SHADOW_GEAR };
	const enum sc_type scatk[6] = { SC_STRIPWEAPON, SC_STRIPARMOR, SC_STRIPSHIELD, SC_STRIPHELM, SC__STRIPACCESSORY, SC_SHADOW_STRIP };
	const enum sc_type scdef[6] = { SC_CP_WEAPON, SC_CP_ARMOR, SC_CP_SHIELD, SC_CP_HELM, SC_NONE, SC_PROTECTSHADOWEQUIP };
	status_change *sc = status_get_sc(bl);
	int i;
	TBL_PC *sd;
	sd = BL_CAST(BL_PC, bl);
	if (sc && !sc->count)
		sc = nullptr;

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
				case W_DOUBLE_AA:	// Axe usage during dual wield should also prevent breaking [Neutral]
				case W_DOUBLE_DA:
				case W_DOUBLE_SA:
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

	for (i = 0; i < 6; i++) {
		if (where&where_list[i]) {
			if (sc && sc->count && sc->getSCE(scdef[i]))
				where&=~where_list[i];
			else if (rnd()%10000 >= rate)
				where&=~where_list[i];
			else if (!sd) //Cause Strip effect.
				sc_start(src,bl,scatk[i],100,0,skill_get_time(status_db.getSkill(scatk[i]),1));
		}
	}
	if (!where) //Nothing to break.
		return 0;
	if (sd) {
		for (i = 0; i < EQI_MAX; i++) {
			short j = sd->equip_index[i];
			if (j < 0 || sd->inventory.u.items_inventory[j].attribute == 1 || !sd->inventory_data[j])
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
				case EQI_ACC_L:
					flag = (where&EQP_ACC_L);
					break;
				case EQI_ACC_R:
					flag = (where&EQP_ACC_R);
					break;
				case EQI_SHADOW_ARMOR:
					flag = (where&EQP_SHADOW_ARMOR);
					break;
				case EQI_SHADOW_WEAPON:
					flag = (where&EQP_SHADOW_WEAPON);
					break;
				case EQI_SHADOW_SHIELD:
					flag = (where&EQP_SHADOW_SHIELD);
					break;
				case EQI_SHADOW_SHOES:
					flag = (where&EQP_SHADOW_SHOES);
					break;
				case EQI_SHADOW_ACC_R:
					flag = (where&EQP_SHADOW_ACC_R);
					break;
				case EQI_SHADOW_ACC_L:
					flag = (where&EQP_SHADOW_ACC_L);
					break;
				default:
					continue;
			}
			if (flag) {
				sd->inventory.u.items_inventory[j].attribute = 1;
				pc_unequipitem(sd, j, 3);
			}
		}
		clif_equiplist(sd);
	}

	return where; //Return list of pieces broken.
}

/**
 * Strip equipment from a target
 * @param src: Source of call
 * @param target: Target to strip
 * @param skill_id: Skill used
 * @param skill_lv: Skill level used
 * @return True on successful strip or false otherwise
 */
bool skill_strip_equip(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv)
{
	nullpo_retr(false, src);
	nullpo_retr(false, target);

	status_change *tsc = status_get_sc(target);

	if (!tsc || tsc->option&OPTION_MADOGEAR) // Mado Gear cannot be divested [Ind]
		return false;

	const int pos[6]             = { EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HELM, EQP_ACC, EQP_SHADOW_GEAR };
	const enum sc_type sc_atk[6] = { SC_STRIPWEAPON, SC_STRIPSHIELD, SC_STRIPARMOR, SC_STRIPHELM, SC__STRIPACCESSORY, SC_SHADOW_STRIP };
	const enum sc_type sc_def[6] = { SC_CP_WEAPON, SC_CP_SHIELD, SC_CP_ARMOR, SC_CP_HELM, SC_NONE, SC_PROTECTSHADOWEQUIP };
	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*target);
	int rate, time, location, mod = 100;

	switch (skill_id) { // Rate
		case RG_STRIPWEAPON:
		case RG_STRIPARMOR:
		case RG_STRIPSHIELD:
		case RG_STRIPHELM:
		case GC_WEAPONCRUSH:
			rate = 50 * (skill_lv + 1) + 2 * (sstatus->dex - tstatus->dex);
			mod = 1000;
			break;
		case ST_FULLSTRIP: {
			int min_rate = 50 + 20 * skill_lv;

			rate = min_rate + 2 * (sstatus->dex - tstatus->dex);
			rate = max(min_rate, rate);
			mod = 1000;
			break;
		}
		case GS_DISARM:
			rate = sstatus->dex / (4 * (7 - skill_lv)) + sstatus->luk / (4 * (6 - skill_lv));
			rate = rate + status_get_lv(src) - (tstatus->agi * rate / 100) - tstatus->luk - status_get_lv(target);
			break;
		case WL_EARTHSTRAIN: {
			int job_lv = 0;

			if (src->type == BL_PC)
				job_lv = ((TBL_PC*)src)->status.job_level;
			rate = 6 * skill_lv + job_lv / 4 + sstatus->dex / 10;
			break;
		}
		case SC_STRIPACCESSARY:
			rate = 12 + 2 * skill_lv;
			break;
		case ABC_STRIP_SHADOW:
			rate = 50 * (skill_lv + 3) + 2 * (sstatus->dex - tstatus->dex);
			mod = 1000;
			break;
		default:
			return false;
	}

	if (rnd()%mod >= rate)
		return false;

	switch (skill_id) { // Duration
		case SC_STRIPACCESSARY:
		case GS_DISARM:
			time = skill_get_time(skill_id, skill_lv);
			break;
		case WL_EARTHSTRAIN:
		case RG_STRIPWEAPON:
		case RG_STRIPARMOR:
		case RG_STRIPSHIELD:
		case RG_STRIPHELM:
		case GC_WEAPONCRUSH:
		case ST_FULLSTRIP:
		case ABC_STRIP_SHADOW:
			if (skill_id == WL_EARTHSTRAIN)
				time = skill_get_time2(skill_id, skill_lv);
			else
				time = skill_get_time(skill_id, skill_lv);

			if (target->type == BL_PC)
				time += max(1, skill_lv + 500 * (sstatus->dex - tstatus->dex));
			else {
				time += 15000;
				time += max(1, skill_lv + 500 * (sstatus->dex - tstatus->dex));
			}
			break;
	}

	switch (skill_id) { // Location
		case GC_WEAPONCRUSH:
		case RG_STRIPWEAPON:
		case GS_DISARM:
			location = EQP_WEAPON;
			break;
		case RG_STRIPARMOR:
			location = EQP_ARMOR;
			break;
		case RG_STRIPSHIELD:
			location = EQP_SHIELD;
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
		case ABC_STRIP_SHADOW:
			location = EQP_SHADOW_GEAR;
			break;
	}

	for (uint8 i = 0; i < ARRAYLENGTH(pos); i++) {
		if (location&pos[i] && sc_def[i] > SC_NONE && tsc->getSCE(sc_def[i]))
			location &=~ pos[i];
	}
	if (!location)
		return false;

	for (uint8 i = 0; i < ARRAYLENGTH(pos); i++) {
		if (location&pos[i] && !sc_start(src, target, sc_atk[i], 100, skill_lv, time))
			location &=~ pos[i];
	}
	return location ? true : false;
}

/**
 * Used to knock back players, monsters, traps, etc
 * @param src Object that give knock back
 * @param target Object that receive knock back
 * @param count Number of knock back cell requested
 * @param dir Direction indicates the way OPPOSITE to the knockback direction (or -1 for default behavior)
 * @param flag
		BLOWN_DONT_SEND_PACKET - position update packets must not be sent
		BLOWN_IGNORE_NO_KNOCKBACK - ignores players' special_state.no_knockback
			These flags "return 'count' instead of 0 if target is cannot be knocked back":
		BLOWN_NO_KNOCKBACK_MAP - at WOE/BG map
		BLOWN_MD_KNOCKBACK_IMMUNE - if target is MD_KNOCKBACK_IMMUNE
		BLOWN_TARGET_NO_KNOCKBACK - if target has 'special_state.no_knockback'
		BLOWN_TARGET_BASILICA - if target is in Basilica area (Pre-Renewal)
 * @return Number of knocked back cells done
 */
short skill_blown(struct block_list* src, struct block_list* target, char count, int8 dir, enum e_skill_blown flag)
{
	int dx = 0, dy = 0;
	uint8 checkflag = 0;
	status_change *tsc = status_get_sc(target);
	enum e_unit_blown reason = UB_KNOCKABLE;

	nullpo_ret(src);
	nullpo_ret(target);

	if (!count)
		return count; // Actual knockback distance is 0.

	// Create flag needed in unit_blown_immune
	if(src != target)
		checkflag |= 0x1; // Offensive
	if(!(flag&BLOWN_IGNORE_NO_KNOCKBACK))
		checkflag |= 0x2; // Knockback type
	if(status_get_class_(src) == CLASS_BOSS)
		checkflag |= 0x4; // Boss attack

	// Get reason and check for flags
	reason = unit_blown_immune(target, checkflag);
	switch(reason) {
		case UB_NO_KNOCKBACK_MAP: return ((flag&BLOWN_NO_KNOCKBACK_MAP) ? count : 0); // No knocking back in WoE / BG
		case UB_MD_KNOCKBACK_IMMUNE: return ((flag&BLOWN_MD_KNOCKBACK_IMMUNE) ? count : 0); // Immune can't be knocked back
		case UB_TARGET_BASILICA: return ((flag&BLOWN_TARGET_BASILICA) ? count : 0); // Basilica caster can't be knocked-back by normal monsters.
		case UB_TARGET_NO_KNOCKBACK: return ((flag&BLOWN_TARGET_NO_KNOCKBACK) ? count : 0); // Target has special_state.no_knockback (equip)
		case UB_TARGET_TRAP: return count; // Trap cannot be knocked back
	}

	if (dir == -1) // <optimized>: do the computation here instead of outside
		dir = map_calc_dir(target, src->x, src->y); // Direction from src to target, reversed

	if (dir >= 0 && dir < 8) { // Take the reversed 'direction' and reverse it
		dx = -dirx[dir];
		dy = -diry[dir];
	}

	if (tsc) {
		if (tsc->getSCE(SC_SU_STOOP)) // Any knockback will cancel it.
			status_change_end(target, SC_SU_STOOP);
		if (tsc->getSCE(SC_ROLLINGCUTTER))
			status_change_end(target, SC_ROLLINGCUTTER);
		if (tsc->getSCE(SC_CRESCIVEBOLT))
			status_change_end(target, SC_CRESCIVEBOLT);
		if (tsc->getSCE(SC_SV_ROOTTWIST)) // Shouldn't move.
			return 0;
	}

	return unit_blown(target, dx, dy, count, flag);	// Send over the proper flag
}

// Checks if 'bl' should reflect back a spell cast by 'src'.
// type is the type of magic attack: 0: indirect (aoe), 1: direct (targetted)
// In case of success returns type of reflection, otherwise 0
//		1 - Regular reflection (Maya)
//		2 - SL_KAITE reflection
//		3 - NPC_MAGICMIRROR reflection
static int skill_magic_reflect(struct block_list* src, struct block_list* bl, int type)
{
	status_change *sc = status_get_sc(bl);
	map_session_data* sd = BL_CAST(BL_PC, bl);

	// Deadly Projection null's all magic reflection.
	if (sc && sc->getSCE(SC_DEADLY_DEFEASANCE))
		return 0;

	if (!sc || !sc->getSCE(SC_KYOMU)) { // Kyomu doesn't reflect
		// Item-based reflection - Bypasses Boss check
		if (sd && sd->bonus.magic_damage_return && type && rnd()%100 < sd->bonus.magic_damage_return)
			return 1;

		// Magic Mirror reflection - Bypasses Boss check
		if (sc && sc->getSCE(SC_MAGICMIRROR) && rnd()%100 < sc->getSCE(SC_MAGICMIRROR)->val2)
			return 3;
	}

	if( status_get_class_(src) == CLASS_BOSS )
		return 0;

	// status-based reflection
	if( !sc || sc->count == 0 )
		return 0;

	// Kaite reflection - Does not bypass Boss check
	if( sc->getSCE(SC_KAITE) && (src->type == BL_PC || status_get_lv(src) <= 80)
#ifdef RENEWAL
		&& type // Does not reflect AoE
#endif
		) {
		// Kaite only works against non-players if they are low-level.
		// Kyomu doesn't disable Kaite, but the "skill fail chance" part of Kyomu applies to it.
		clif_specialeffect(bl, EF_ATTACKENERGY2, AREA);
		if( --sc->getSCE(SC_KAITE)->val2 <= 0 )
			status_change_end(bl, SC_KAITE);
		return 2;
	}

	return 0;
}

/**
 * Checks whether a skill can be used in combos or not
 * @param skill_id: Target skill
 * @return	0: Skill is not a combo
 *			1: Skill is a normal combo
 *			2: Skill is combo that prioritizes auto-target even if val2 is set 
 * @author Panikon
 */
int skill_is_combo(uint16 skill_id) {
	switch(skill_id) {
		case MO_CHAINCOMBO:
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
		case MO_EXTREMITYFIST:
		case TK_TURNKICK:
		case TK_STORMKICK:
		case TK_DOWNKICK:
		case TK_COUNTER:
		case TK_JUMPKICK:
		case HT_POWER:
		case SR_DRAGONCOMBO:
			return 1;
		case SR_FALLENEMPIRE:
		case SR_TIGERCANNON:
		case SR_GATEOFHELL:
			return 2;
	}
	return 0;
}

/*
 * Combo handler, start stop combo status
 */
void skill_combo_toggle_inf(struct block_list* bl, uint16 skill_id, int inf){
	TBL_PC *sd = BL_CAST(BL_PC, bl);
	switch (skill_id) {
		case MH_MIDNIGHT_FRENZY:
		case MH_EQC:
			{
				int skill_id2 = ((skill_id==MH_EQC)?MH_TINDER_BREAKER:MH_SONIC_CRAW);
				short idx = hom_skill_get_index(skill_id2);
				int flag = (inf?SKILL_FLAG_TMP_COMBO:SKILL_FLAG_PERMANENT);
				TBL_HOM *hd = BL_CAST(BL_HOM, bl);
				if (idx == -1)
					break;
				hd->homunculus.hskill[idx].flag= flag;
				clif_homskillinfoblock( *hd ); //refresh info //@FIXME we only want to refresh one skill
			}
			break;
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
			if (sd) clif_skillinfo(sd,MO_EXTREMITYFIST, inf);
			break;
		case TK_JUMPKICK:
			if (sd) clif_skillinfo(sd,TK_JUMPKICK, inf);
			break;
		case MO_TRIPLEATTACK:
			if (sd && pc_checkskill(sd, SR_DRAGONCOMBO) > 0)
				clif_skillinfo(sd,SR_DRAGONCOMBO, inf);
			break;
		case SR_FALLENEMPIRE:
			if (sd){
				clif_skillinfo(sd,SR_GATEOFHELL, inf);
				clif_skillinfo(sd,SR_TIGERCANNON, inf);
			}
			break;
	}
}

void skill_combo(struct block_list* src,struct block_list *dsrc, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick){
	t_tick duration = 0; //Set to duration the user can use a combo skill or 1 for aftercast delay of pre-skill
	int nodelay = 0; //Set to 1 for no walk/attack delay, set to 2 for no walk delay
	int target_id = bl->id; //Set to 0 if combo skill should not autotarget
	struct status_change_entry *sce;
	TBL_PC *sd = BL_CAST(BL_PC,src);
	TBL_HOM *hd = BL_CAST(BL_HOM,src);
	status_change *sc = status_get_sc(src);

	if(sc == nullptr) return;

	//End previous combo state after skill is invoked
	if ((sce = sc->getSCE(SC_COMBO)) != nullptr) {
		switch (skill_id) {
		case TK_TURNKICK:
		case TK_STORMKICK:
		case TK_DOWNKICK:
		case TK_COUNTER:
			if (sd && pc_famerank(sd->status.char_id,MAPID_TAEKWON)) {//Extend combo time.
				sce->val1 = skill_id; //Update combo-skill
				sce->val3 = skill_id;
				if( sce->timer != INVALID_TIMER )
					delete_timer(sce->timer, status_change_timer);
				sce->timer = add_timer(tick+sce->val4, status_change_timer, src->id, SC_COMBO);
				break;
			}
			unit_cancel_combo(src); // Cancel combo wait
			break;
		default:
			if( src == dsrc ) // Ground skills are exceptions. [Inkfish]
				status_change_end(src, SC_COMBO);
		}
	}

	//start new combo
	if (sd) { //player only
		switch (skill_id) {
		case MO_TRIPLEATTACK:
			if (pc_checkskill(sd, MO_CHAINCOMBO) > 0 || pc_checkskill(sd, SR_DRAGONCOMBO) > 0) {
				duration = 1;
				target_id = 0; // Will target current auto-target instead
			}
			break;
		case MO_CHAINCOMBO:
			if (pc_checkskill(sd, MO_COMBOFINISH) > 0 && sd->spiritball > 0) {
				duration = 1;
				target_id = 0; // Will target current auto-target instead
			}
			break;
		case MO_COMBOFINISH:
			if (sd->status.party_id > 0) //bonus from SG_FRIEND [Komurka]
				party_skill_check(sd, sd->status.party_id, MO_COMBOFINISH, skill_lv);
			if (pc_checkskill(sd, CH_TIGERFIST) > 0 && sd->spiritball > 0) {
				duration = 1;
				target_id = 0; // Will target current auto-target instead
			}
			[[fallthrough]]; // so we can possibly cast TigerFist or straight to ExtremityFist
		case CH_TIGERFIST:
			if (!duration && pc_checkskill(sd, CH_CHAINCRUSH) > 0 && sd->spiritball > 1) {
				duration = 1;
				target_id = 0; // Will target current auto-target instead
			}
			[[fallthrough]]; // so we can possibly cast ChainCrush or straight to ExtremityFist
		case CH_CHAINCRUSH:
			if (!duration && pc_checkskill(sd, MO_EXTREMITYFIST) > 0 && sd->spiritball > 0 && sd->sc.getSCE(SC_EXPLOSIONSPIRITS)) {
				duration = 1;
				target_id = 0; // Will target current auto-target instead
			}
			break;
		case AC_DOUBLE:
			if (pc_checkskill(sd, HT_POWER)) {
				duration = 2000;
				nodelay = 1; //Neither gives walk nor attack delay
				target_id = 0; //Does not need to be used on previous target
			}
			break;
		case SR_DRAGONCOMBO:
			if (pc_checkskill(sd, SR_FALLENEMPIRE) > 0)
				duration = 1;
			break;
		case SR_FALLENEMPIRE:
			if (pc_checkskill(sd, SR_TIGERCANNON) > 0 || pc_checkskill(sd, SR_GATEOFHELL) > 0)
				duration = 1;
			break;
		case SJ_PROMINENCEKICK:
			if (pc_checkskill(sd, SJ_SOLARBURST) > 0)
				duration = 1;
			break;
		}
	}
	else { //other
		switch(skill_id) {
		case MH_TINDER_BREAKER:
		case MH_CBC:
		case MH_SONIC_CRAW:
		case MH_SILVERVEIN_RUSH:
			if(hd->homunculus.spiritball > 0) duration = 2000;
				nodelay = 1;
			break;
		case MH_EQC:
		case MH_MIDNIGHT_FRENZY:
			if(hd->homunculus.spiritball >= 2) duration = 2000;
				nodelay = 1;
			break;
		}
	}

	if (duration) { //Possible to chain
		if(sd && duration==1) duration = DIFF_TICK(sd->ud.canact_tick, tick); //Auto calc duration
		duration = i64max(status_get_amotion(src),duration); //Never less than aMotion
		sc_start4(src,src,SC_COMBO,100,skill_id,target_id,nodelay,0,duration);
		clif_combo_delay(src, duration);
	}
}

/**
 * Copy skill by Plagiarism or Reproduce
 * @param src: The caster
 * @param bl: The target
 * @param skill_id: Skill that casted
 * @param skill_lv: Skill level of the casted skill
 */
static void skill_do_copy(struct block_list* src,struct block_list *bl, uint16 skill_id, uint16 skill_lv)
{
	TBL_PC *tsd = BL_CAST(BL_PC, bl);

	if (!tsd || (!pc_checkskill(tsd,RG_PLAGIARISM) && !pc_checkskill(tsd,SC_REPRODUCE)))
		return;
	//If SC_PRESERVE is active and SC__REPRODUCE is not active, nothing to do
	else if (tsd->sc.getSCE(SC_PRESERVE) && !tsd->sc.getSCE(SC__REPRODUCE))
		return;
	else {
		uint16 idx;
		uint8 lv;

		skill_id = skill_dummy2skill_id(skill_id);

		//Use skill index, avoiding out-of-bound array [Cydh]
		if (!(idx = skill_get_index(skill_id)))
			return;

		switch (skill_isCopyable(tsd,skill_id)) {
			case 1: //Copied by Plagiarism
				{
					if (tsd->cloneskill_idx > 0 && tsd->status.skill[tsd->cloneskill_idx].flag == SKILL_FLAG_PLAGIARIZED) {
						clif_deleteskill(*tsd,tsd->status.skill[tsd->cloneskill_idx].id, true);
						tsd->status.skill[tsd->cloneskill_idx].id = 0;
						tsd->status.skill[tsd->cloneskill_idx].lv = 0;
						tsd->status.skill[tsd->cloneskill_idx].flag = SKILL_FLAG_PERMANENT;
					}

					lv = min(skill_lv,pc_checkskill(tsd,RG_PLAGIARISM)); //Copied level never be > player's RG_PLAGIARISM level

					tsd->cloneskill_idx = idx;
					pc_setglobalreg(tsd, add_str(SKILL_VAR_PLAGIARISM), skill_id);
					pc_setglobalreg(tsd, add_str(SKILL_VAR_PLAGIARISM_LV), lv);
				}
				break;
			case 2: //Copied by Reproduce
				{
					status_change *tsc = status_get_sc(bl);
					//Already did SC check
					//Skill level copied depends on Reproduce skill that used
					lv = (tsc) ? tsc->getSCE(SC__REPRODUCE)->val1 : 1;
					if( tsd->reproduceskill_idx > 0 && tsd->status.skill[tsd->reproduceskill_idx].flag == SKILL_FLAG_PLAGIARIZED ) {
						clif_deleteskill(*tsd,tsd->status.skill[tsd->reproduceskill_idx].id, true);
						tsd->status.skill[tsd->reproduceskill_idx].id = 0;
						tsd->status.skill[tsd->reproduceskill_idx].lv = 0;
						tsd->status.skill[tsd->reproduceskill_idx].flag = SKILL_FLAG_PERMANENT;
					}

					//Level dependent and limitation.
					if (src->type == BL_PC) //If player, max skill level is skill_get_max(skill_id)
						lv = min(lv,skill_get_max(skill_id));
					else //Monster might used skill level > allowed player max skill lv. Ex. Drake with Waterball lv. 10
						lv = min(lv,skill_lv);

					tsd->reproduceskill_idx = idx;
					pc_setglobalreg(tsd, add_str(SKILL_VAR_REPRODUCE), skill_id);
					pc_setglobalreg(tsd, add_str(SKILL_VAR_REPRODUCE_LV), lv);
				}
				break;
			default: return;
		}
		tsd->status.skill[idx].id = skill_id;
		tsd->status.skill[idx].lv = lv;
		tsd->status.skill[idx].flag = SKILL_FLAG_PLAGIARIZED;
		clif_addskill(tsd,skill_id);
	}
}

/**
 * Knockback the target on skill_attack
 * @param src is the master behind the attack
 * @param dsrc is the actual originator of the damage, can be the same as src, or a BL_SKILL
 * @param target is the target to be attacked.
 * @param blewcount
 * @param skill_id
 * @param skill_lv
 * @param damage
 * @param tick
 * @param flag can hold a bunch of information:
 */
void skill_attack_blow(struct block_list *src, struct block_list *dsrc, struct block_list *target, uint8 blewcount, uint16 skill_id, uint16 skill_lv, int64 damage, t_tick tick, int flag) {
	int8 dir = -1; // Default direction
	//Only knockback if it's still alive, otherwise a "ghost" is left behind. [Skotlex]
	//Reflected spells do not bounce back (src == dsrc since it only happens for direct skills)
	if (!blewcount || target == dsrc || status_isdead(*target))
		return;

	// Skill specific direction
	switch (skill_id) {
		case MG_FIREWALL:
		case EL_FIRE_MANTLE:
			dir = unit_getdir(target); // Backwards
			break;
		// This ensures the storm randomly pushes instead of exactly a cell backwards per official mechanics.
		case NPC_STORMGUST2:
		case WZ_STORMGUST:
			if(!battle_config.stormgust_knockback)
				dir = rnd()%8;
			break;
		case MC_CARTREVOLUTION:
			if (battle_config.cart_revo_knockback)
				dir = 6; // Official servers push target to the West
			break;
		case AC_SHOWER:
		case WL_CRIMSONROCK:
			if (!battle_config.arrow_shower_knockback && skill_id == AC_SHOWER)
				dir = map_calc_dir(target, src->x, src->y);
			else
				dir = map_calc_dir(target, skill_area_temp[4], skill_area_temp[5]);
			break;
		case HT_PHANTASMIC: // issue #1378
			if (status_get_hp(target) - damage <= 0) return;
			break;
	}

	// Blown-specific handling
	switch( skill_id ) {
		case SR_KNUCKLEARROW:
			// Ignore knockback damage bonus if in WOE (player cannot be knocked in WOE)
			// Boss & Immune Knockback stay in place and don't get bonus damage
			// Give knockback damage bonus only hits the wall. (bugreport:9096)
			if (skill_blown(dsrc, target, blewcount, dir_ka, (enum e_skill_blown)(BLOWN_IGNORE_NO_KNOCKBACK|BLOWN_NO_KNOCKBACK_MAP|BLOWN_MD_KNOCKBACK_IMMUNE|BLOWN_TARGET_NO_KNOCKBACK|BLOWN_TARGET_BASILICA)) < blewcount)
				skill_addtimerskill(src, tick + 300 * ((flag&2) ? 1 : 2), target->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag|4);
			dir_ka = -1;
			break;
		case RL_R_TRIP:
			if (skill_blown(dsrc,target,blewcount,dir,BLOWN_NONE) < blewcount)
				skill_addtimerskill(src, tick + status_get_amotion(src), target->id, 0, 0, RL_R_TRIP_PLUSATK, skill_lv, BF_WEAPON, flag|SD_ANIMATION);
			break;
		default:
			skill_blown(dsrc,target,blewcount,dir, BLOWN_NONE);
			if (!blewcount && target->type == BL_SKILL && damage > 0) {
				TBL_SKILL *su = (TBL_SKILL*)target;
				if (su->group && su->group->skill_id == HT_BLASTMINE)
					skill_blown(src, target, 3, -1, BLOWN_NONE);
			}
			break;
	}

	clif_fixpos( *target );
}

/*
 * =========================================================================
 * Does a skill attack with the given properties.
 * @param src is the master behind the attack (player/mob/pet)
 * @param dsrc is the actual originator of the damage, can be the same as src, or a BL_SKILL
 * @param bl is the target to be attacked.
 * @param flag can hold a bunch of information:
 *        flag&1
 *        flag&2 - Disable re-triggered by double casting
 *        flag&4 - Skip to blow target (because already knocked back before skill_attack somewhere)
 *        flag&8 - SC_COMBO state used to deal bonus damage
 *
 *        flag&0xFFF is passed to the underlying battle_calc_attack for processing.
 *             (usually holds number of targets, or just 1 for simple splash attacks)
 *
 *        flag&0xF000 - Values from enum e_skill_display
 *        flag&0x3F0000 - Values from enum e_battle_check_target
 * 
 *        flag&0x1000000 - Return 0 if damage was reflected
 *-------------------------------------------------------------------------*/
int64 skill_attack (int attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int flag)
{
	struct Damage dmg;
	status_change *sc, *tsc;
	map_session_data *sd, *tsd;
	int64 damage;
	bool rmdamage = false;//magic reflected
	int type;
	enum e_damage_type dmg_type;
	bool shadow_flag = false;
	bool additional_effects = true;

	if(skill_id > 0 && !skill_lv)
		return 0;

	nullpo_ret(src);	//Source is the master behind the attack (player/mob/pet)
	nullpo_ret(dsrc);	//dsrc is the actual originator of the damage, can be the same as src, or a skill casted by src.
	nullpo_ret(bl);		//Target to be attacked.

	if (status_bl_has_mode(bl,MD_SKILLIMMUNE) || (status_get_class(bl) == MOBID_EMPERIUM && !skill_get_inf2(skill_id, INF2_TARGETEMPERIUM)))
		return 0;

	if (src != dsrc) {
		//When caster is not the src of attack, this is a ground skill, and as such, do the relevant target checking. [Skotlex]
		if (!status_check_skilluse(battle_config.skill_caster_check?src:nullptr, bl, skill_id, 2))
			return 0;
	} else if ((flag&SD_ANIMATION) && skill_get_nk(skill_id, NK_SPLASH)) {
		//Note that splash attacks often only check versus the targetted mob, those around the splash area normally don't get checked for being hidden/cloaked/etc. [Skotlex]
		if (!status_check_skilluse(src, bl, skill_id, 2))
			return 0;
	}

	sd = BL_CAST(BL_PC, src);
	tsd = BL_CAST(BL_PC, bl);

	status_data* sstatus = status_get_status_data(*src);
	status_data* tstatus = status_get_status_data(*bl);
	sc= status_get_sc(src);
	tsc= status_get_sc(bl);
	if (tsc && !tsc->count)
		tsc = nullptr; //Don't need it.

	 //Trick Dead protects you from damage, but not from buffs and the like, hence it's placed here.
	if (tsc && tsc->getSCE(SC_TRICKDEAD))
		return 0;

#ifndef RENEWAL
	//When Gravitational Field is active, damage can only be dealt by Gravitational Field and Autospells
	if(sd && sc && sc->getSCE(SC_GRAVITATION) && sc->getSCE(SC_GRAVITATION)->val3 == BCT_SELF && skill_id != HW_GRAVITATION && !sd->state.autocast)
		return 0;
#endif

	dmg = battle_calc_attack(attack_type,src,bl,skill_id,skill_lv,flag&0xFFF);

	//If the damage source is a unit, the damage is not delayed
	if (src != dsrc)
		dmg.amotion = 0;

	//! CHECKME: This check maybe breaks the battle_calc_attack, and maybe need better calculation.
	// Adjusted to the new system [Skotlex]
	if( src->type == BL_PET ) { // [Valaris]
		struct pet_data *pd = (TBL_PET*)src;
		if (pd->a_skill && pd->a_skill->div_ && pd->a_skill->id == skill_id) { //petskillattack2
			if (battle_config.pet_ignore_infinite_def || !is_infinite_defense(bl,dmg.flag)) {
				int element = skill_get_ele(skill_id, skill_lv);
				/*if (skill_id == -1) Does it ever worked?
					element = sstatus->rhw.ele;*/
				if (element != ELE_NEUTRAL || !(battle_config.attack_attr_none&BL_PET))
					dmg.damage = battle_attr_fix(src, bl, pd->a_skill->damage, element, tstatus->def_ele, tstatus->ele_lv);
				else
					dmg.damage = pd->a_skill->damage; // Fixed damage
				
			}
			else
				dmg.damage = 1*pd->a_skill->div_;
			dmg.damage2 = 0;
			dmg.div_= pd->a_skill->div_;
		}
	}

	if( dmg.flag&BF_MAGIC && ( skill_id != NPC_EARTHQUAKE || (battle_config.eq_single_target_reflectable && (flag&0xFFF) == 1) ) )
	{ // Earthquake on multiple targets is not counted as a target skill. [Inkfish]
		if( (dmg.damage || dmg.damage2) && (type = skill_magic_reflect(src, bl, src==dsrc)) )
		{	//Magic reflection, switch caster/target
			struct block_list *tbl = bl;
			rmdamage = true;
			bl = src;
			src = tbl;
			dsrc = tbl;
			sd = BL_CAST(BL_PC, src);
			tsd = BL_CAST(BL_PC, bl);
			tsc = status_get_sc(bl);
			if (tsc && !tsc->count)
				tsc = nullptr; //Don't need it.
			/* bugreport:2564 flag&2 disables double casting trigger */
			flag |= 2;
			//Reflected magic damage will not cause the caster to be knocked back [Playtester]
			flag |= 4;
			//Spirit of Wizard blocks Kaite's reflection
			if( type == 2 && tsc && tsc->getSCE(SC_SPIRIT) && tsc->getSCE(SC_SPIRIT)->val2 == SL_WIZARD )
			{	//Consume one Fragment per hit of the casted skill? [Skotlex]
				type = tsd?pc_search_inventory (tsd, ITEMID_FRAGMENT_OF_CRYSTAL):0;
				if (type >= 0) {
					if ( tsd )
						pc_delitem(tsd, type, 1, 0, 1, LOG_TYPE_CONSUME);
					dmg.damage = dmg.damage2 = 0;
					dmg.dmg_lv = ATK_MISS;
					tsc->getSCE(SC_SPIRIT)->val3 = skill_id;
					tsc->getSCE(SC_SPIRIT)->val4 = dsrc->id;
				}
			}

			// Official Magic Reflection Behavior : damage reflected depends on gears caster wears, not target
#if MAGIC_REFLECTION_TYPE
#ifdef RENEWAL
			if( dmg.dmg_lv != ATK_MISS ) { //Wiz SL cancelled and consumed fragment
#else
			// issue:6415 in pre-renewal Kaite reflected the entire damage received
			// regardless of caster's equipment (Aegis 11.1)
			if( dmg.dmg_lv != ATK_MISS && type == 1 ) { //Wiz SL cancelled and consumed fragment
#endif
				short s_ele = skill_get_ele(skill_id, skill_lv);

				if (s_ele == ELE_WEAPON) // the skill takes the weapon's element
					s_ele = sstatus->rhw.ele;
				else if (s_ele == ELE_ENDOWED) //Use status element
					s_ele = status_get_attack_sc_element(src,status_get_sc(src));
				else if( s_ele == ELE_RANDOM) //Use random element
					s_ele = rnd()%ELE_ALL;

				if(type == 3)
					dmg.flag = BF_WEAPON|BF_NORMAL|BF_SHORT;
				dmg.damage = battle_attr_fix(bl, bl, dmg.damage, s_ele, status_get_element(bl), status_get_element_level(bl));

				if( tsc && tsc->getSCE(SC_ENERGYCOAT) ) {
					status_data* status = status_get_status_data(*bl);
					int per = 100*status->sp / status->max_sp -1; //100% should be counted as the 80~99% interval
					per /=20; //Uses 20% SP intervals.
					//SP Cost: 1% + 0.5% per every 20% SP
					if (!status_charge(bl, 0, (10+5*per)*status->max_sp/1000))
						status_change_end(bl, SC_ENERGYCOAT);
					//Reduction: 6% + 6% every 20%
					dmg.damage -= dmg.damage * (6 * (1+per)) / 100;
				}

				int64 reduce = 0;

				if (tsd && tsd->bonus.reduce_damage_return != 0) {
					reduce += tsd->bonus.reduce_damage_return;
				}
				if (tsc && tsc->getSCE(SC_REFLECTDAMAGE)) {
					reduce += (tsc->getSCE(SC_REFLECTDAMAGE)->val2);
				}
				if (tsc && tsc->getSCE(SC_REF_T_POTION))
					reduce += 100;
				if (dmg.damage > 0) {
					dmg.damage -= dmg.damage * i64min(100, reduce) / 100;
					dmg.damage = i64max(dmg.damage, dmg.div_);
				}
			}
#endif
		}

		if(tsc && tsc->getSCE(SC_MAGICROD) && src == dsrc) {
			int sp = skill_get_sp(skill_id,skill_lv);
#ifndef RENEWAL 
			clif_skill_nodamage(bl,*bl,SA_MAGICROD,skill_lv);
#endif
			dmg.damage = dmg.damage2 = 0;
			dmg.dmg_lv = ATK_MISS; //This will prevent skill additional effect from taking effect. [Skotlex]
			sp = sp * tsc->getSCE(SC_MAGICROD)->val2 / 100;
			if(skill_id == WZ_WATERBALL && skill_lv > 1)
				sp = sp/((skill_lv|1)*(skill_lv|1)); //Estimate SP cost of a single water-ball
			status_heal(bl, 0, sp, 2);
		}
		if( (dmg.damage || dmg.damage2) && tsc && (tsc->getSCE(SC_HALLUCINATIONWALK) && rnd()%100 < tsc->getSCE(SC_HALLUCINATIONWALK)->val3 || tsc->getSCE(SC_NPC_HALLUCINATIONWALK) && rnd()%100 < tsc->getSCE(SC_NPC_HALLUCINATIONWALK)->val3) ) {
			dmg.damage = dmg.damage2 = 0;
			dmg.dmg_lv = ATK_MISS;
		}
	}

	damage = dmg.damage + dmg.damage2;

	if ((dmg.flag & BF_MAGIC) && tsc && tsc->getSCE(SC_MAXPAIN)) {
		auto * sce = tsc->getSCE(SC_MAXPAIN);
		sce->val3 = (int)damage;
		sce->val2 = 0;
		if (!tsc->getSCE(SC_KYOMU)) //SC_KYOMU invalidates reflecting ability.
			skill_castend_damage_id(bl, src, NPC_MAXPAIN_ATK, sce->val1, tick, flag);
	}

	if( (skill_id == AL_INCAGI || skill_id == AL_BLESSING ||
		skill_id == CASH_BLESSING || skill_id == CASH_INCAGI ||
		skill_id == MER_INCAGI || skill_id == MER_BLESSING) && tsc && tsc->getSCE(SC_CHANGEUNDEAD) )
		damage = 1;

	if( damage && tsc && tsc->getSCE(SC_GENSOU) && dmg.flag&BF_MAGIC ){
		struct block_list *nbl;
		nbl = battle_getenemyarea(bl,bl->x,bl->y,2,BL_CHAR,bl->id);
		if( nbl ){ // Only one target is chosen.
			damage = damage / 2; // Deflect half of the damage to a target nearby
			clif_skill_damage(bl, nbl, tick, status_get_amotion(src), 0, battle_fix_damage(bl,nbl,damage,0,0), dmg.div_, OB_OBOROGENSOU_TRANSITION_ATK, -1, DMG_SINGLE);
		}
	}

	//Skill hit type
	dmg_type = (skill_id == 0) ? DMG_SPLASH : skill_get_hit(skill_id);

	switch( skill_id ) {
		case SC_TRIANGLESHOT:
			if( rnd()%100 > (1 + skill_lv) )
				dmg.blewcount = 0;
			break;
		default:
			if (damage < dmg.div_ && skill_id != CH_PALMSTRIKE)
				dmg.blewcount = 0; //only pushback when it hit for other
			break;
	}

	switch( skill_id ) {
		case CR_GRANDCROSS:
		case NPC_GRANDDARKNESS:
			if( battle_config.gx_disptype)
				dsrc = src;
			if( src == bl)
				dmg_type = DMG_ENDURE;
			else
				flag|= SD_ANIMATION;
			break;
		case NJ_TATAMIGAESHI: //For correct knockback.
			dsrc = src;
			flag|= SD_ANIMATION;
			break;
		case TK_COUNTER: {	//bonus from SG_FRIEND [Komurka]
			int level;
			if( sd && sd->status.party_id > 0 && (level = pc_checkskill(sd,SG_FRIEND)) )
				party_skill_check(sd, sd->status.party_id, TK_COUNTER,level);
			}
			break;
		case SL_STIN:
		case SL_STUN:
			if (skill_lv >= 7) {
				status_change *sc_cur = status_get_sc(src);
				if (sc_cur && !sc_cur->getSCE(SC_SMA))
					sc_start(src,src,SC_SMA,100,skill_lv,skill_get_time(SL_SMA, skill_lv));
			}
			break;
		case DK_SERVANT_W_DEMOL:// Only give servant's per target after damage calculation.
			pc_addservantball( *sd );
			break;
		case KN_PIERCE:
		case LK_SPIRALPIERCE:
		case RK_HUNDREDSPEAR:
		case DK_MADNESS_CRUSHER:
			if (sc && sc->getSCE(SC_CHARGINGPIERCE)) {
				if (sc->getSCE(SC_CHARGINGPIERCE_COUNT)) {
					if (sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 < 10) // If the charge count is below 10, add 1.
						sc_start(src, src, SC_CHARGINGPIERCE_COUNT, 100, sc->getSCE(SC_CHARGINGPIERCE_COUNT)->val1 + 1, skill_get_time2(DK_CHARGINGPIERCE, 1));
					else { // If charge count is 10, bonus damage is applied for 1 attack and then the count status ends.
						clif_specialeffect(bl, 1767, AREA);
						status_change_end(src, SC_CHARGINGPIERCE_COUNT);
					}
				} else // No count status detected? Start charge count at 1.
					sc_start(src, src, SC_CHARGINGPIERCE_COUNT, 100, 1, skill_get_time2(DK_CHARGINGPIERCE, 1));
			}
			break;
	}

	//combo handling
	skill_combo(src,dsrc,bl,skill_id,skill_lv,tick);

	//Display damage.
	switch( skill_id ) {
		case PA_GOSPEL: //Should look like Holy Cross [Skotlex]
			dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion, damage, dmg.div_, CR_HOLYCROSS, -1, DMG_SPLASH);
			break;
		//Skills that need be passed as a normal attack for the client to display correctly.
		case NPC_SELFDESTRUCTION:
			if(src->type == BL_PC)
				dmg.blewcount = 10;
			dmg.amotion = 0; //Disable delay or attack will do no damage since source is dead by the time it takes effect. [Skotlex]
			[[fallthrough]];
		case HVAN_EXPLOSION:
		case KN_AUTOCOUNTER:
		case NPC_CRITICALSLASH:
		case TF_DOUBLE:
		case GS_CHAINACTION:
			dmg.dmotion = clif_damage(*src,*bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,dmg.type,dmg.damage2,false);
			break;

		case AS_SPLASHER:
			if( flag&SD_ANIMATION ) // the surrounding targets
				dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, DMG_SPLASH); // needs -1 as skill level
			else // the central target doesn't display an animation
				dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -2, DMG_SPLASH); // needs -2(!) as skill level
			break;
		case SR_EARTHSHAKER:
			dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,skill_id,-2,DMG_SINGLE);
			break;
		case WL_SOULEXPANSION:
		case WL_COMET:
		case NPC_COMET:
		case KO_MUCHANAGE:
#ifndef RENEWAL
		case NJ_HUUMA:
#endif
			dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skill_id,skill_lv,DMG_MULTI_HIT);
			break;
		case WL_CHAINLIGHTNING_ATK:
			dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,1,WL_CHAINLIGHTNING_ATK,-2,DMG_SINGLE);
			break;
		case WL_TETRAVORTEX_FIRE:
			dmg.dmotion = clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, WL_TETRAVORTEX_WIND, -1, DMG_SPLASH);
			break;
		case LG_SHIELDPRESS:
			dmg.dmotion = clif_skill_damage(dsrc, bl, tick, status_get_amotion(src), dmg.dmotion, damage, dmg.div_, skill_id, -1, DMG_SINGLE);
			break;
		case NPC_EARTHQUAKE:
			dmg.dmotion = clif_skill_damage(src, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, DMG_ENDURE);
			break;
		case NPC_DARKPIERCING:
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
		case KO_BAKURETSU:
		case GN_HELLS_PLANT_ATK:
		case SU_SV_ROOTTWIST_ATK:
			dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skill_id,-1,DMG_SPLASH);
			break;
		case GN_FIRE_EXPANSION_ACID:
			dmg.dmotion = clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, CR_ACIDDEMONSTRATION, skill_lv, DMG_MULTI_HIT);
			break;
		case GN_SLINGITEM_RANGEMELEEATK:
			dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,GN_SLINGITEM,-2,DMG_SINGLE);
			break;
		case EL_STONE_RAIN:
			dmg.dmotion = clif_skill_damage(dsrc,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,skill_id,-1,(flag&1)?DMG_MULTI_HIT:DMG_SPLASH);
			break;
		case WM_SEVERE_RAINSTORM_MELEE:
			dmg.dmotion = clif_skill_damage(src,bl,tick,dmg.amotion,dmg.dmotion,damage,dmg.div_,WM_SEVERE_RAINSTORM,-2,DMG_SPLASH);
			break;
		case HT_CLAYMORETRAP:
		case HT_BLASTMINE:
		case HT_FLASHER:
		case HT_FREEZINGTRAP:
		case RA_CLUSTERBOMB:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
			dmg.dmotion = clif_skill_damage(src, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, (flag&SD_LEVEL) ? -1 : skill_lv, DMG_SPLASH);
			if( dsrc != src ) // avoid damage display redundancy
				break;
			[[fallthrough]];
		case HT_LANDMINE:
			dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, dmg_type);
			break;
		case WZ_SIGHTBLASTER:
			//Sightblaster should never call clif_skill_damage twice
			dmg.dmotion = clif_skill_damage(src, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, (flag&SD_LEVEL) ? -1 : skill_lv, DMG_SPLASH);
			break;
		case RL_R_TRIP_PLUSATK:
		case RL_S_STORM:
			dmg.dmotion = clif_skill_damage(dsrc,bl,tick,status_get_amotion(src),dmg.dmotion,damage,dmg.div_,skill_id,-1,DMG_SPLASH);
			break;
		case SU_LUNATICCARROTBEAT:
		case SU_LUNATICCARROTBEAT2:
		case SP_CURSEEXPLOSION:
		case SP_SPA:
		case SP_SHA:
			if (dmg.div_ < 2)
				type = DMG_SPLASH;
			if (!(flag&SD_ANIMATION))
				clif_skill_nodamage(dsrc, *bl, skill_id, skill_lv);
			[[fallthrough]];
		case WM_REVERBERATION:
			dmg.dmotion = clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -2, dmg_type);
			break;
		case SJ_FALLINGSTAR_ATK:
		case SJ_FALLINGSTAR_ATK2:
			dmg.dmotion = clif_skill_damage(src,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -2, DMG_MULTI_HIT);
			break;
		case SJ_NOVAEXPLOSING:
			dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -2, DMG_SINGLE);
			break;
		case DK_HACKANDSLASHER_ATK:
			dmg.dmotion = clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, dmg_type);
			break;
		case AG_STORM_CANNON:
		case AG_CRIMSON_ARROW:
			dmg.dmotion = clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, skill_lv, DMG_SPLASH);
			break;
		case TR_ROSEBLOSSOM_ATK:
		case ABC_FROM_THE_ABYSS_ATK:
			dmg.dmotion = clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, -1, DMG_SPLASH);
			break;
		case TR_SOUNDBLEND:
			if (flag&SD_ANIMATION)// For some reason the caster reacts on the splash flag. Best reduce amotion to minimize it for now. [Rytech]
				dmg.dmotion = clif_skill_damage(dsrc, bl, tick, 10, dmg.dmotion, damage, dmg.div_, skill_id, -1, DMG_SPLASH);
			else
				dmg.dmotion = clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, skill_lv, dmg_type);
			break;
		case AB_DUPLELIGHT_MELEE:
		case AB_DUPLELIGHT_MAGIC:
			dmg.amotion = 300;/* makes the damage value not overlap with previous damage (when displayed by the client) */
			[[fallthrough]];
		default:
			if( flag&SD_ANIMATION && dmg.div_ < 2 ) //Disabling skill animation doesn't works on multi-hit.
				dmg_type = DMG_SPLASH;
			if (src->type == BL_SKILL) {
				TBL_SKILL *su = (TBL_SKILL*)src;
				if (su->group && skill_get_inf2(su->group->skill_id, INF2_ISTRAP)) { // show damage on trap targets
					clif_skill_damage(src, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, flag&SD_LEVEL ? -1 : skill_lv, DMG_SPLASH);
					break;
				}
			}
			dmg.dmotion = clif_skill_damage(dsrc,bl,tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skill_id, flag&SD_LEVEL?-1:skill_lv, dmg_type);
			break;
	}

	map_freeblock_lock();

	if (bl->type == BL_PC && skill_id && skill_db.find(skill_id)->copyable.option && //Only copy skill that copyable [Cydh]
		dmg.flag&BF_SKILL && dmg.damage+dmg.damage2 > 0 && damage < status_get_hp(bl)) //Cannot copy skills if the blow will kill you. [Skotlex]
		skill_do_copy(src,bl,skill_id,skill_lv);

	if (dmg.dmg_lv >= ATK_MISS && (type = skill_get_walkdelay(skill_id, skill_lv)) > 0)
	{	//Skills with can't walk delay also stop normal attacking for that
		//duration when the attack connects. [Skotlex]
		struct unit_data *ud = unit_bl2ud(src);
		if (ud && DIFF_TICK(ud->attackabletime, tick + type) < 0)
			ud->attackabletime = tick + type;
	}

	shadow_flag = skill_check_shadowform(bl, damage, dmg.div_);

	// Instant damage
	if( !dmg.amotion ) {
		//Deal damage before knockback to allow stuff like firewall+storm gust combo.
		if( (!tsc || (!tsc->getSCE(SC_DEVOTION) && skill_id != CR_REFLECTSHIELD && !tsc->getSCE(SC_WATER_SCREEN_OPTION))
#ifndef RENEWAL
			|| skill_id == HW_GRAVITATION
#endif
			|| skill_id == NPC_EVILLAND) && !shadow_flag )
			battle_damage(src, bl, damage, dmg.dmotion, skill_lv, skill_id, dmg.dmg_lv, dmg.flag, false, tick, false);
		if( !status_isdead(*bl) && additional_effects )
			skill_additional_effect(src,bl,skill_id,skill_lv,dmg.flag,dmg.dmg_lv,tick);
		if( damage > 0 ) //Counter status effects [Skotlex]
			skill_counter_additional_effect(src,bl,skill_id,skill_lv,dmg.flag,tick);
	}

	// Blow!
	if (!(flag&4))
		skill_attack_blow(src, dsrc, bl, (uint8)dmg.blewcount, skill_id, skill_lv, damage, tick, flag);

	// Delayed damage must be dealt after the knockback (it needs to know actual position of target)
	if( dmg.amotion ) {
		if( shadow_flag ) {
			if( !status_isdead(*bl) && additional_effects )
				skill_additional_effect(src, bl, skill_id, skill_lv, dmg.flag, dmg.dmg_lv, tick);
			if( dmg.flag > ATK_BLOCK )
				skill_counter_additional_effect(src, bl, skill_id, skill_lv, dmg.flag, tick);
		} else
			battle_delay_damage(tick, dmg.amotion,src,bl,dmg.flag,skill_id,skill_lv,damage,dmg.dmg_lv,dmg.dmotion, additional_effects, false);
	} else {
		// Trigger monster skill condition for damage skills with no amotion.
		if (bl->type == BL_MOB && src != bl && !status_isdead(*bl)) {
			if (damage > 0)
				mobskill_event(BL_CAST(BL_MOB, bl), src, tick, dmg.flag);
			if (skill_id > 0)
				mobskill_event(BL_CAST(BL_MOB, bl), src, tick, MSC_SKILLUSED | (skill_id << 16));
		}
	}

	// Trigger monster skill condition for damage skills.
	if (bl->type == BL_MOB && src != bl && !status_isdead(*bl)) {
		if (damage > 0)
			mobskill_event(BL_CAST(BL_MOB, bl), src, tick, dmg.flag, damage);
		if (skill_id > 0)
			mobskill_event(BL_CAST(BL_MOB, bl), src, tick, MSC_SKILLUSED | (skill_id << 16), damage);
	}

	if (tsc  && skill_id != NPC_EVILLAND && skill_id != SP_SOULEXPLOSION && skill_id != SJ_NOVAEXPLOSING
#ifndef RENEWAL
		&& skill_id != PA_PRESSURE && skill_id != HW_GRAVITATION
#endif
		) {
		if (tsc->getSCE(SC_DEVOTION)) {
			struct status_change_entry *sce = tsc->getSCE(SC_DEVOTION);
			struct block_list *d_bl = map_id2bl(sce->val1);

			if (d_bl && (
				(d_bl->type == BL_MER && ((TBL_MER*)d_bl)->master && ((TBL_MER*)d_bl)->master->bl.id == bl->id) ||
				(d_bl->type == BL_PC && ((TBL_PC*)d_bl)->devotion[sce->val2] == bl->id)
				) && check_distance_bl(bl, d_bl, sce->val3) )
			{
				int64 devotion_damage = damage;

				// Needed to check the devotion master for Rebound Shield status.
				status_change *d_sc = status_get_sc(d_bl);

				if (d_sc && d_sc->getSCE(SC_REBOUND_S))
					devotion_damage -= devotion_damage * d_sc->getSCE(SC_REBOUND_S)->val2 / 100;

				if (!rmdamage) {
					clif_damage(*d_bl, *d_bl, gettick(), 0, 0, devotion_damage, 0, DMG_NORMAL, 0, false);
					battle_fix_damage(src, d_bl, devotion_damage, 0, 0);
				} else {
					bool isDevotRdamage = false;

					if (battle_config.devotion_rdamage && battle_config.devotion_rdamage > rnd()%100)
						isDevotRdamage = true;
					// If !isDevotRdamage, reflected magics are done directly on the target not on paladin
					// This check is only for magical skill.
					// For BF_WEAPON skills types track var rdamage and function battle_calc_return_damage
					clif_damage(*bl, (!isDevotRdamage) ? *bl : *d_bl, gettick(), 0, 0, devotion_damage, 0, DMG_NORMAL, 0, false);
					battle_fix_damage(bl, (!isDevotRdamage) ? bl : d_bl, devotion_damage, 0, 0);
				}
			} else {
				status_change_end(bl, SC_DEVOTION);
				if (!dmg.amotion)
					battle_fix_damage(src, bl, damage, dmg.dmotion, 0);
			}
		}
		if (tsc->getSCE(SC_WATER_SCREEN_OPTION)) {
			struct status_change_entry *sce = tsc->getSCE(SC_WATER_SCREEN_OPTION);
			struct block_list *e_bl = map_id2bl(sce->val1);

			if (e_bl) {
				if (!rmdamage) {
					clif_skill_damage(e_bl, e_bl, gettick(), 0, 0, damage, dmg.div_, skill_id, -1, skill_get_hit(skill_id));
					battle_fix_damage(src, e_bl, damage, 0, 0);
				} else {
					clif_skill_damage(bl, bl, gettick(), 0, 0, damage, dmg.div_, skill_id, -1, skill_get_hit(skill_id));
					battle_fix_damage(bl, bl, damage, 0, 0);
				}
			}
		}
	}

	if(damage > 0 && !status_has_mode(tstatus,MD_STATUSIMMUNE)) {
		if( skill_id == RG_INTIMIDATE ) {
			int rate = 50 + skill_lv * 5;
			rate = rate + (status_get_lv(src) - status_get_lv(bl));
			if(rnd()%100 < rate)
				skill_addtimerskill(src,tick + 800,bl->id,0,0,skill_id,skill_lv,0,flag);
		} else if( skill_id == NPC_FATALMENACE ) {
			int16 x = skill_area_temp[4], y = skill_area_temp[5];

			map_search_freecell(nullptr, bl->m, &x, &y, 2, 2, 1);
			skill_addtimerskill(bl,tick + 800,bl->id,x,y,skill_id,skill_lv,0,flag);
		}
	}

	if( sd && src != bl && damage > 0 && ( dmg.flag&BF_WEAPON ||
		(dmg.flag&BF_MISC && (skill_id == RA_CLUSTERBOMB || skill_id == RA_FIRINGTRAP || skill_id == RA_ICEBOUNDTRAP)) ) )
	{
		if (battle_config.left_cardfix_to_right)
			battle_drain(sd, bl, dmg.damage, dmg.damage, tstatus->race, tstatus->class_);
		else
			battle_drain(sd, bl, dmg.damage, dmg.damage2, tstatus->race, tstatus->class_);
	}

	if (sd && tsc && dmg.flag&BF_LONG && tsc->getSCE(SC_WINDSIGN) && rand()%100 < tsc->getSCE(SC_WINDSIGN)->val2)
		status_heal(src, 0, 0, 1, 0);

	if( damage > 0 ) { // Post-damage effects
		switch( skill_id ) {
			case GC_VENOMPRESSURE: {
					status_change *ssc = status_get_sc(src);
					if( ssc && ssc->getSCE(SC_POISONINGWEAPON) && rnd()%100 < 70 + 5*skill_lv ) {
						sc_start4(src, bl, (sc_type)ssc->getSCE(SC_POISONINGWEAPON)->val2, 100, ssc->getSCE(SC_POISONINGWEAPON)->val1, 0, 1, 0, skill_get_time2(GC_POISONINGWEAPON, 1));
						status_change_end(src,SC_POISONINGWEAPON);
						clif_skill_nodamage(src,*bl,skill_id,skill_lv);
					}
				}
				break;
		}
		if( sd )
			skill_onskillusage(sd, bl, skill_id, tick);
	}

	if (!(flag&2)) {
		switch (skill_id) {
			case MG_COLDBOLT:
			case MG_FIREBOLT:
			case MG_LIGHTNINGBOLT:
				if (sc && sc->getSCE(SC_DOUBLECAST) && rnd() % 100 < sc->getSCE(SC_DOUBLECAST)->val2)
					//skill_addtimerskill(src, tick + dmg.div_*dmg.amotion, bl->id, 0, 0, skill_id, skill_lv, BF_MAGIC, flag|2);
					skill_addtimerskill(src, tick + dmg.amotion, bl->id, 0, 0, skill_id, skill_lv, BF_MAGIC, flag|2);
				break;
			case SU_BITE:
			case SU_SCRATCH:
			case SU_SV_STEMSPEAR:
			case SU_SCAROFTAROU:
			case SU_PICKYPECK:
				if (status_get_lv(src) > 29 && rnd() % 100 < 10 * status_get_lv(src) / 30)
					skill_addtimerskill(src, tick + dmg.amotion + skill_get_delay(skill_id, skill_lv), bl->id, 0, 0, skill_id, skill_lv, attack_type, flag|2);
				break;
		}
	}

	map_freeblock_unlock();

	if ((flag&0x1000000) && rmdamage)
		return 0; //Should return 0 when damage was reflected

	return damage;
}

/*==========================================
 * Sub function for recursive skill call.
 * Checking bl battle flag and display damage
 * then call func with source,target,skill_id,skill_lv,tick,flag
 *------------------------------------------*/
typedef int (*SkillFunc)(struct block_list *, struct block_list *, int, int, t_tick, int);
int skill_area_sub(struct block_list *bl, va_list ap)
{
	struct block_list *src;
	uint16 skill_id,skill_lv;
	int flag;
	t_tick tick;
	SkillFunc func;

	nullpo_ret(bl);

	src = va_arg(ap,struct block_list *);
	skill_id = va_arg(ap,int);
	skill_lv = va_arg(ap,int);
	tick = va_arg(ap,t_tick);
	flag = va_arg(ap,int);
	func = va_arg(ap,SkillFunc);

	if (flag&BCT_WOS && src == bl)
		return 0;

	if(battle_check_target(src,bl,flag) > 0) {
		// several splash skills need this initial dummy packet to display correctly
		if (flag&SD_PREAMBLE && skill_area_temp[2] == 0)
			clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);

		if (flag&(SD_SPLASH|SD_PREAMBLE))
			skill_area_temp[2]++;

		return func(src,bl,skill_id,skill_lv,tick,flag);
	}
	return 0;
}

static int skill_check_unit_range_sub(struct block_list *bl, va_list ap)
{
	struct skill_unit *unit;
	uint16 skill_id,g_skill_id;

	unit = (struct skill_unit *)bl;

	if(bl->prev == nullptr || bl->type != BL_SKILL)
		return 0;

	if(!unit->alive)
		return 0;

	skill_id = va_arg(ap,int);
	g_skill_id = unit->group->skill_id;

	switch (skill_id) {
		case AL_PNEUMA: //Pneuma doesn't work even if just one cell overlaps with Land Protector
			if(g_skill_id == SA_LANDPROTECTOR)
				break;
			[[fallthrough]];
		case MH_STEINWAND:
		case MG_SAFETYWALL:
		case SC_MAELSTROM:
			if(g_skill_id != MH_STEINWAND && g_skill_id != MG_SAFETYWALL && g_skill_id != AL_PNEUMA && g_skill_id != SC_MAELSTROM)
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
#ifndef RENEWAL
		case HP_BASILICA:
#endif
		case RA_ELECTRICSHOCKER:
		case RA_CLUSTERBOMB:
		case RA_MAGENTATRAP:
		case RA_COBALTTRAP:
		case RA_MAIZETRAP:
		case RA_VERDURETRAP:
		case RA_FIRINGTRAP:
		case RA_ICEBOUNDTRAP:
		case SC_DIMENSIONDOOR:
		case SC_BLOODYLUST:
		case NPC_REVERBERATION:
		case GN_THORNS_TRAP:
		case RL_B_TRAP:
		case SC_ESCAPE:
			//Non stackable on themselves and traps (including venom dust which does not has the trap inf2 set)
			if (skill_id != g_skill_id && !skill_get_inf2(g_skill_id, INF2_ISTRAP) && g_skill_id != AS_VENOMDUST && g_skill_id != MH_POISON_MIST)
				return 0;
			break;
		default: //Avoid stacking with same kind of trap. [Skotlex]
			if (g_skill_id != skill_id)
				return 0;
			break;
	}

	return 1;
}

static int skill_check_unit_range (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv)
{
	//Non players do not check for the skill's splash-trigger area.
	int range = bl->type==BL_PC?skill_get_unit_range(skill_id, skill_lv):0;
	int layout_type = skill_get_unit_layout_type(skill_id,skill_lv);
	if (layout_type==-1 || layout_type>MAX_SQUARE_LAYOUT) {
		ShowError("skill_check_unit_range: unsupported layout type %d for skill %d\n",layout_type,skill_id);
		return 0;
	}

	range += layout_type;
	return map_foreachinallarea(skill_check_unit_range_sub,bl->m,x-range,y-range,x+range,y+range,BL_SKILL,skill_id);
}

static int skill_check_unit_range2_sub (struct block_list *bl, va_list ap)
{
	uint16 skill_id;

	if(bl->prev == nullptr)
		return 0;

	skill_id = va_arg(ap,int);

	if( status_isdead(*bl) && skill_id != AL_WARP )
		return 0;

#ifndef RENEWAL
	if( skill_id == HP_BASILICA && bl->type == BL_PC )
		return 0;
#endif

	if( skill_id == AM_DEMONSTRATION && bl->type == BL_MOB && ((TBL_MOB*)bl)->mob_id == MOBID_EMPERIUM )
		return 0; //Allow casting Bomb/Demonstration Right under emperium [Skotlex]
	return 1;
}

/**
 * Used to check range condition of the casted skill. Used if the skill has UF_NOFOOTSET or INF2_DISABLENEARNPC
 * @param bl Object that casted skill
 * @param x Position x of the target
 * @param y Position y of the target
 * @param skill_id The casted skill
 * @param skill_lv The skill Lv
 * @param isNearNPC 'true' means, check the range between target and nearer NPC by using npc_isnear and range calculation [Cydh]
 * @return 0: No object (BL_CHAR or BL_PC) within the range. If 'isNearNPC' the target oject is BL_NPC
 */
static int skill_check_unit_range2 (struct block_list *bl, int x, int y, uint16 skill_id, uint16 skill_lv, bool isNearNPC)
{
	int range = 0, type;

	//Range for INF2_DISABLENEARNPC is using skill splash value [Cydh]
	if (isNearNPC)
		range = skill_get_splash(skill_id,skill_lv);

	//While checking INF2_DISABLENEARNPC and the range from splash is 0, get the range from skill_unit range and layout. [Cydh]
	if (!isNearNPC || !range) {
		switch (skill_id) {	// to be expanded later
			case WZ_ICEWALL:
				range = 2;
				break;
			case SC_MANHOLE:
				range = 0;
				break;
			default: {
					int layout_type = skill_get_unit_layout_type(skill_id,skill_lv);

					if (layout_type == -1 || layout_type > MAX_SQUARE_LAYOUT) {
						ShowError("skill_check_unit_range2: unsupported layout type %d for skill %d\n",layout_type,skill_id);
						return 0;
					}
					range = skill_get_unit_range(skill_id,skill_lv) + layout_type;
				}
				break;
		}
	}

	uint16 skill_npc_range = skill_db.find(skill_id)->unit_nonearnpc_range;

	//Check the additional range [Cydh]
	if (isNearNPC && skill_npc_range > 0)
		range += skill_npc_range;

	if (!isNearNPC) { //Doesn't check the NPC range
		//If the caster is a monster/NPC, only check for players. Otherwise just check characters
		if (bl->type&battle_config.skill_nofootset)
			type = BL_CHAR;
		else if(bl->type == BL_MOB)
			type = BL_MOB; //Monsters can never place traps on top of each other regardless of setting
		else
			return 0; //Don't check
	} else
		type = BL_NPC;

	return (!isNearNPC) ?
		//!isNearNPC is used for UF_NOFOOTSET, regardless the NPC position, only check the BL_CHAR or BL_PC
		map_foreachinallarea(skill_check_unit_range2_sub,bl->m,x - range,y - range,x + range,y + range,type,skill_id):
		//isNearNPC is used to check range from NPC
		map_foreachinallarea(npc_isnear_sub,bl->m,x - range,y - range,x + range,y + range,type,skill_id);
}

/** Apply special cases where skills require HP/SP/AP but do not consume them, then continue with consuming HP/SP/AP
* @param bl Object from which HP/SP/AP are consumed
* @param skill_id ID of used skill
* @param hp Original HP requirement to use skill
* @param sp Original SP requirement to use skill
* @param ap Original AP requirement to use skill
*/
void skill_consume_hpspap(block_list* bl, uint16 skill_id, int hp, int sp, int ap)
{
	nullpo_retv(bl);

	switch (skill_id) {
		//Skills that require HP but do not consume them
	case SM_MAGNUM:
	case MS_MAGNUM:
		hp = 0;
		break;
	}

	status_zap(bl, hp, sp, ap);
}

/*==========================================
 * Checks that you have the requirements for casting a skill for homunculus/mercenary.
 * Flag:
 * &1: finished casting the skill (invoke hp/sp/item consumption)
 * &2: picked menu entry (Warp Portal, Teleport and other menu based skills)
 *------------------------------------------*/
static int skill_check_condition_mercenary(struct block_list *bl, uint16 skill_id, uint16 skill_lv, int type)
{
	map_session_data *sd = nullptr;
	int i, hp, sp, hp_rate, sp_rate, state, mhp;
	t_itemid itemid[MAX_SKILL_ITEM_REQUIRE];
	int amount[ARRAYLENGTH(itemid)], index[ARRAYLENGTH(itemid)];

	nullpo_retr(0, bl);

	switch( bl->type )
	{
		case BL_HOM: sd = ((TBL_HOM*)bl)->master; break;
		case BL_MER: sd = ((TBL_MER*)bl)->master; break;
	}

	if( sd == nullptr ){
		return 0;
	}

	status_data* status = status_get_status_data(*bl);
	skill_lv = cap_value(skill_lv, 1, MAX_SKILL_LEVEL);

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (skill == nullptr)
		return 0;

	// Requirements
	for( i = 0; i < ARRAYLENGTH(itemid); i++ )
	{
		itemid[i] = skill->require.itemid[i];
		amount[i] = skill->require.amount[i];
	}
	hp = skill->require.hp[skill_lv - 1];
	sp = skill->require.sp[skill_lv - 1];
	hp_rate = skill->require.hp_rate[skill_lv - 1];
	sp_rate = skill->require.sp_rate[skill_lv - 1];
	state = skill->require.state;
	if ((mhp = skill->require.mhp[skill_lv - 1]) > 0)
		hp += (status->max_hp * mhp) / 100;
	if( hp_rate > 0 )
		hp += (status->hp * hp_rate) / 100;
	else
		hp += (status->max_hp * (-hp_rate)) / 100;
	if( sp_rate > 0 )
		sp += (status->sp * sp_rate) / 100;
	else
		sp += (status->max_sp * (-sp_rate)) / 100;

	if( !(type&2) )
	{
		if( hp > 0 && status->hp <= (unsigned int)hp )
		{
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT );
			return 0;
		}
		if( sp > 0 && status->sp <= (unsigned int)sp )
		{
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT );
			return 0;
		}
	}

	if( !type )
		switch( state )
		{
			case ST_MOVE_ENABLE:
				if( !unit_can_move(bl) )
				{
					clif_skill_fail( *sd, skill_id );
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
		if( itemid[i] == 0 ) continue; // No item
		index[i] = pc_search_inventory(sd, itemid[i]);
		if( index[i] < 0 || sd->inventory.u.items_inventory[index[i]].amount < amount[i] )
		{
			clif_skill_fail( *sd, skill_id );
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
		skill_consume_hpspap(bl, skill_id, hp, sp, 0);

	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_area_sub_count (struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv, t_tick tick, int flag)
{
	switch (skill_id) {
		case RL_QD_SHOT:
			{
				if (src->type == BL_PC && BL_CAST(BL_PC,src)) {
					struct unit_data *ud = unit_bl2ud(src);
					if (ud && ud->target == target->id)
						return 1;
				}
			}
	}
	return 1;
}

/*==========================================
 *
 *------------------------------------------*/
static TIMER_FUNC(skill_timerskill){
	struct block_list *src = map_id2bl(id),*target;
	struct unit_data *ud = unit_bl2ud(src);
	struct skill_timerskill *skl;
	struct skill_unit *unit = nullptr;
	int range;

	nullpo_ret(src);
	nullpo_ret(ud);
	skl = ud->skilltimerskill[data];
	nullpo_ret(skl);
	ud->skilltimerskill[data] = nullptr;

	do {
		if(src->prev == nullptr)
			break; // Source not on Map
		if(skl->target_id) {
			target = map_id2bl(skl->target_id);
			if( ( skl->skill_id == RG_INTIMIDATE || skl->skill_id == NPC_FATALMENACE ) && (!target || target->prev == nullptr || !check_distance_bl(src,target,AREA_SIZE)) )
				target = src; //Required since it has to warp.

			if (skl->skill_id == SR_SKYNETBLOW) {
				skill_area_temp[1] = 0;
				clif_skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skl->skill_id,skl->skill_lv,DMG_SINGLE);
				map_foreachinallrange(skill_area_sub,src,skill_get_splash(skl->skill_id,skl->skill_lv),BL_CHAR|BL_SKILL,src,
					skl->skill_id,skl->skill_lv,tick,skl->flag|BCT_ENEMY|SD_SPLASH|1,skill_castend_damage_id);
				break;
			}

			if(target == nullptr)
				break; // Target offline?
			if(target->prev == nullptr)
				break; // Target not on Map
			if(src->m != target->m)
				break; // Different Maps

			if(status_isdead(*src)) {
				switch(skl->skill_id) {
					case WL_CHAINLIGHTNING_ATK:
					case WL_TETRAVORTEX_FIRE:
					case WL_TETRAVORTEX_WATER:
					case WL_TETRAVORTEX_WIND:
					case WL_TETRAVORTEX_GROUND:
					// For SR_FLASHCOMBO
					case SR_DRAGONCOMBO:
					case SR_FALLENEMPIRE:
					case SR_TIGERCANNON:
					case NPC_DANCINGBLADE_ATK:
						if (src->type != BL_PC)
							continue;
						break; // Exceptions
					default:
						continue; // Caster is Dead
				}
			}
			if(status_isdead(*target) && skl->skill_id != RG_INTIMIDATE && skl->skill_id != WZ_WATERBALL)
				break;

			switch(skl->skill_id) {
				case KN_AUTOCOUNTER:
					clif_skill_nodamage(src,*target,skl->skill_id,skl->skill_lv);
					break;
				case RG_INTIMIDATE:
					if (unit_warp(src,-1,-1,-1,CLR_TELEPORT) == 0) {
						short x,y;
						map_search_freecell(src, 0, &x, &y, 1, 1, 0);
						if (target != src && !status_isdead(*target))
							unit_warp(target, -1, x, y, CLR_TELEPORT);
					}
					break;
				case BA_FROSTJOKER:
				case DC_SCREAM:
					range= skill_get_splash(skl->skill_id, skl->skill_lv);
					map_foreachinallarea(skill_frostjoke_scream,skl->map,skl->x-range,skl->y-range,
						skl->x+range,skl->y+range,BL_CHAR,src,skl->skill_id,skl->skill_lv,tick);
					break;
				case PR_LEXDIVINA:
					if (src->type == BL_MOB) {
						// Monsters use the default duration when casting Lex Divina
						sc_start(src, target, SC_SILENCE, skl->type, skl->skill_lv, skill_get_time2(status_db.getSkill(SC_SILENCE), 1));
						break;
					}
					sc_start(src, target, SC_SILENCE, skl->type, skl->skill_lv, skill_get_time2(skl->skill_id, skl->skill_lv));
					break;
				case PR_STRECOVERY:
					sc_start(src, target, SC_BLIND, skl->type, skl->skill_lv, skill_get_time2(skl->skill_id, skl->skill_lv));
					break;
				case BS_HAMMERFALL:
					sc_start(src, target, SC_STUN, skl->type, skl->skill_lv, skill_get_time2(skl->skill_id, skl->skill_lv));
					break;
				case MER_LEXDIVINA:
					sc_start(src, target, SC_SILENCE, skl->type, skl->skill_lv, skill_get_time2(skl->skill_id, skl->skill_lv));
					break;
				case WZ_WATERBALL:
				{
					//Get the next waterball cell to consume
					struct s_skill_unit_layout *layout;
					int i;
					layout = skill_get_unit_layout(skl->skill_id, skl->skill_lv, src, skl->x, skl->y);
					for (i = skl->type; i >= 0 && i < layout->count; i++) {
						int ux = skl->x + layout->dx[i];
						int uy = skl->y + layout->dy[i];
						unit = map_find_skill_unit_oncell(src, ux, uy, WZ_WATERBALL, nullptr, 0);
						if (unit)
							break;
					}
				}
					[[fallthrough]];
				case WZ_JUPITEL:
					// Official behaviour is to hit as long as there is a line of sight, regardless of distance
					if (skl->type > 0 && !status_isdead(*target) && path_search_long(nullptr,src->m,src->x,src->y,target->x,target->y,CELL_CHKWALL)) {
						// Apply canact delay here to prevent hacks (unlimited casting)
						ud->canact_tick = i64max(tick + status_get_amotion(src), ud->canact_tick);
						skill_attack(BF_MAGIC, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag);
					}
					if (unit && !status_isdead(*target) && !status_isdead(*src)) {
						skill_delunit(unit); // Consume unit for next waterball
						//Timer will continue and walkdelay set until target is dead, even if there is currently no line of sight
						unit_set_walkdelay(src, tick, TIMERSKILL_INTERVAL, 1);
						skill_addtimerskill(src,tick+TIMERSKILL_INTERVAL,target->id,skl->x,skl->y,skl->skill_id,skl->skill_lv,skl->type+1,skl->flag);
					} else {
						status_change *sc = status_get_sc(src);
						if(sc) {
							if(sc->getSCE(SC_SPIRIT) &&
								sc->getSCE(SC_SPIRIT)->val2 == SL_WIZARD &&
								sc->getSCE(SC_SPIRIT)->val3 == skl->skill_id)
								sc->getSCE(SC_SPIRIT)->val3 = 0; //Clear bounced spell check.
						}
					}
					break;
				case NPC_DANCINGBLADE_ATK:
					skill_attack(BF_WEAPON,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
					if (skl->type < 4) {
						struct block_list *nbl = nullptr;

						nbl = battle_getenemyarea(src, target->x, target->y, 5, splash_target(src), src->id);
						skill_addtimerskill(src, tick + 650, (nbl?nbl:target)->id, skl->x, 0, NPC_DANCINGBLADE_ATK, skl->skill_lv, skl->type + 1, 0);
					}
					break;
				case WL_CHAINLIGHTNING_ATK: {
#ifndef RENEWAL
						skill_toggle_magicpower(src, skl->skill_id); // Only the first hit will be amplified
#endif
						skill_attack(BF_MAGIC,src,src,target,skl->skill_id,skl->skill_lv,tick,9 - skl->type); // Hit a Lightning on the current Target
						if( skl->type < (4 + skl->skill_lv - 1) && skl->x < 3  )
						{ // Remaining Chains Hit
							struct block_list *nbl = nullptr; // Next Target of Chain
							nbl = battle_getenemyarea(src, target->x, target->y, (skl->type>2)?2:3, // After 2 bounces, it will bounce to other targets in 7x7 range.
									splash_target(src), target->id); // Search for a new Target around current one...
							if( nbl == nullptr )
								skl->x++;
							else
								skl->x = 0;
							skill_addtimerskill(src, tick + 650, (nbl?nbl:target)->id, skl->x, 0, WL_CHAINLIGHTNING_ATK, skl->skill_lv, skl->type + 1, 0);
						}
					}
					break;
				case WL_TETRAVORTEX_FIRE:
				case WL_TETRAVORTEX_WATER:
				case WL_TETRAVORTEX_WIND:
				case WL_TETRAVORTEX_GROUND:
					clif_skill_nodamage(src,*target,skl->skill_id,skl->skill_lv);
					skill_attack(BF_MAGIC,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag|SD_LEVEL|SD_ANIMATION);
					if (skl->type >= 3) { // Final Hit
						if (!status_isdead(*target)) { // Final Status Effect
							int effects[4] = { SC_BURNING, SC_FREEZING, SC_BLEEDING, SC_STUN },
								applyeffects[4] = { 0, 0, 0, 0 },
								i, j = 0, k = 0;
							for(i = 1; i <= 8; i = i + i) {
								if (skl->x&i) {
									applyeffects[j] = effects[k];
									j++;
								}
								k++;
							}
							if (j) {
								i = applyeffects[rnd()%j];
								sc_start(src, target, static_cast<sc_type>(i), 100, skl->skill_lv, (i == SC_BURNING ? 18000 : (i == SC_FREEZING ? 27000 : (i == SC_BLEEDING ? 108000 : 4500))));
							}
						}
					}
					break;
				case NPC_REVERBERATION_ATK:
					skill_castend_damage_id(src,target,skl->skill_id,skl->skill_lv,tick,skl->flag|SD_LEVEL|SD_ANIMATION);
					break;
				case NPC_FATALMENACE:
					unit_warp(src, -1, skl->x, skl->y, CLR_TELEPORT);
					break;
				case SR_WINDMILL:
					if( target->type == BL_PC ) {
						map_session_data *tsd = nullptr;
						if( (tsd = ((TBL_PC*)target)) && !pc_issit(tsd) ) {
							pc_setsit(tsd);
							skill_sit(tsd, true);
							clif_sitting(tsd->bl);
						}
					}
					break;
				case SR_KNUCKLEARROW:
					skill_attack(BF_WEAPON, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag|SD_LEVEL);
					break;
				case CH_PALMSTRIKE:
					{
						status_change* tsc = status_get_sc(target);
						status_change* sc = status_get_sc(src);
						if( ( tsc && tsc->option&OPTION_HIDE ) ||
							( sc && sc->option&OPTION_HIDE ) ){
							skill_blown(src,target,skill_get_blewcount(skl->skill_id, skl->skill_lv), -1, BLOWN_NONE);
							break;
						}
						skill_attack(skl->type,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
						break;
					}
				// For SR_FLASHCOMBO
				case SR_DRAGONCOMBO:
				case SR_FALLENEMPIRE:
				case SR_TIGERCANNON:
					if( src->type == BL_PC ) {
						if( distance_xy(src->x, src->y, target->x, target->y) >= 3 )
							break;
						skill_castend_damage_id(src, target, skl->skill_id, pc_checkskill(((TBL_PC *)src), skl->skill_id), tick, 0);
					}
					break;

				case SU_SV_ROOTTWIST_ATK: {
						status_change *tsc = status_get_sc(target);

						if (tsc && tsc->getSCE(SC_SV_ROOTTWIST)) {
							if (check_distance_bl(src, target, 32)) // Only damage if caster is within 32x32 area
								skill_attack(skl->type, src, target, target, skl->skill_id, skl->skill_lv, tick, skl->flag);
							skill_addtimerskill(src, tick + 1000, target->id, 0, 0, skl->skill_id, skl->skill_lv, skl->type, skl->flag);
						}
					}
					break;
				case NPC_PULSESTRIKE2:
					skill_castend_damage_id(src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
					break;
				case ABC_DEFT_STAB:
				case ABC_FRENZY_SHOT:
					skill_castend_damage_id(src, target, skl->skill_id, skl->skill_lv, tick, skl->flag);
					break;
				case HVAN_EXPLOSION:
					status_kill(src);
					break;
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
				case GN_CRAZYWEED_ATK:
					{
						int dummy = 1, i = skill_get_unit_range(skl->skill_id,skl->skill_lv);
						map_foreachinarea(skill_cell_overlap, src->m, skl->x-i, skl->y-i, skl->x+i, skl->y+i, BL_SKILL, skl->skill_id, &dummy, src);
					}
					[[fallthrough]];
				case WL_EARTHSTRAIN:
					skill_unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,(skl->type<<16)|skl->flag);
					break;
				case RL_FIRE_RAIN: {
						int dummy = 1, i = skill_get_splash(skl->skill_id,skl->skill_lv);

						if (rnd() % 100 < (15 + 5 * skl->skill_lv))
							map_foreachinallarea(skill_cell_overlap,src->m,skl->x-i,skl->y-i,skl->x+i,skl->y+i,BL_SKILL,skl->skill_id,&dummy,src);
						skill_unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,0);
					}
					break;
				case NPC_MAGMA_ERUPTION:
				case NC_MAGMA_ERUPTION:
					skill_unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,0);
					break;
				case HN_METEOR_STORM_BUSTER: {
						int16 area = 4;
						int16 tmpx = rnd_value( skl->x - area, skl->x + area );
						int16 tmpy = rnd_value( skl->y - area, skl->y + area );

						if( map_getcell(src->m, tmpx, tmpy, CELL_CHKLANDPROTECTOR) ) {
							return 0;
						}

						int splash = skill_get_splash(skl->skill_id, skl->skill_lv);

						clif_skill_poseffect(src, skl->skill_id, skl->skill_lv, tmpx, tmpy, tick);
						map_foreachinarea(skill_area_sub, src->m, tmpx - splash, tmpy - splash, tmpx + splash, tmpy + splash, BL_CHAR, src, skl->skill_id, skl->skill_lv, tick, skl->flag | BCT_ENEMY | SD_SPLASH | SKILL_ALTDMG_FLAG | 1, skill_castend_damage_id);
						skill_unitsetting(src, skl->skill_id, skl->skill_lv, tmpx, tmpy, skill_get_unit_interval(skl->skill_id));
					}
					break;
				case NW_HASTY_FIRE_IN_THE_HOLE:
					skill_castend_pos2(src, skl->x, skl->y, skl->skill_id, skl->skill_lv, tick, skl->flag);
					break;
				case NW_GRENADES_DROPPING: {
						int area = skill_get_splash(skl->skill_id, skl->skill_lv);
						short tmpx = 0, tmpy = 0;

						tmpx = skl->x - area + rnd() % (area * 2 + 1);
						tmpy = skl->y - area + rnd() % (area * 2 + 1);
						skill_unitsetting(src, skl->skill_id, skl->skill_lv, tmpx, tmpy, skl->flag);
					}
					break;
				case NW_MISSION_BOMBARD:  {
						int area = skill_get_unit_range(skl->skill_id, skl->skill_lv);
						int range = skill_get_splash(skl->skill_id, skl->skill_lv);
						short tmpx = 0, tmpy = 0;

						tmpx = skl->x - range + rnd() % (range * 2 + 1);
						tmpy = skl->y - range + rnd() % (range * 2 + 1);
						map_foreachinarea(skill_area_sub, src->m, tmpx - range, tmpy - range, tmpx + range, tmpy + range, BL_CHAR,
							src, skl->skill_id, skl->skill_lv, tick, skl->flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
					}
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
int skill_addtimerskill (struct block_list *src, t_tick tick, int target, int x,int y, uint16 skill_id, uint16 skill_lv, int type, int flag)
{
	int i;
	struct unit_data *ud;
	nullpo_retr(1, src);
	if (src->prev == nullptr)
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
			switch(ud->skilltimerskill[i]->skill_id) {
				case WL_TETRAVORTEX_FIRE:
				case WL_TETRAVORTEX_WATER:
				case WL_TETRAVORTEX_WIND:
				case WL_TETRAVORTEX_GROUND:
				// For SR_FLASHCOMBO
				case SR_DRAGONCOMBO:
				case SR_FALLENEMPIRE:
				case SR_TIGERCANNON:
					if (src->type != BL_PC)
						break;
					continue;
			}
			delete_timer(ud->skilltimerskill[i]->timer, skill_timerskill);
			ers_free(skill_timer_ers, ud->skilltimerskill[i]);
			ud->skilltimerskill[i]=nullptr;
		}
	}
	return 1;
}
static int skill_active_reverberation(struct block_list *bl, va_list ap) {
	skill_unit *su = (skill_unit*)bl;

	nullpo_ret(su);

	if (bl->type != BL_SKILL)
		return 0;

	std::shared_ptr<s_skill_unit_group> sg = su->group;

	if (su->alive && sg && sg->skill_id == NPC_REVERBERATION) {
		map_foreachinallrange(skill_trap_splash, bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, bl, gettick());
		su->limit = DIFF_TICK(gettick(), sg->tick);
		sg->unit_id = UNT_USED_TRAPS;
	}
	return 1;
}

/**
 * Reveal hidden trap
 **/
static int skill_reveal_trap(struct block_list *bl, va_list ap)
{
	TBL_SKILL *su = (TBL_SKILL*)bl;

	if (su->alive && su->group && su->hidden && skill_get_inf2(su->group->skill_id, INF2_ISTRAP)) {
		//Change look is not good enough, the client ignores it as an actual trap still. [Skotlex]
		//clif_changetraplook(bl, su->group->unit_id);

		su->hidden = false;
		skill_getareachar_skillunit_visibilty(su, AREA);
		return 1;
	}
	return 0;
}

/**
 * Attempt to reveal trap in area
 * @param src Skill caster
 * @param range Affected range
 * @param x
 * @param y
 * TODO: Remove hardcode usages for this function
 **/
void skill_reveal_trap_inarea(struct block_list *src, int range, int x, int y) {
	if (!battle_config.traps_setting)
		return;
	nullpo_retv(src);
	map_foreachinallarea(skill_reveal_trap, src->m, x-range, y-range, x+range, y+range, BL_SKILL);
}

/*========================================== [Playtester]
* Process tarot card's effects
* @param src: Source of the tarot card effect
* @param target: Target of the tartor card effect
* @param skill_id: ID of the skill used
* @param skill_lv: Level of the skill used
* @param tick: Processing tick time
* @return Card number
*------------------------------------------*/
static int skill_tarotcard(struct block_list* src, struct block_list *target, uint16 skill_id, uint16 skill_lv, t_tick tick)
{
	int card = 0;

	if (battle_config.tarotcard_equal_chance) {
		//eAthena equal chances
		card = rnd() % 14 + 1;
	}
	else {
		//Official chances
		int rate = rnd() % 100;
		if (rate < 10) card = 1; // THE FOOL
		else if (rate < 20) card = 2; // THE MAGICIAN
		else if (rate < 30) card = 3; // THE HIGH PRIESTESS
		else if (rate < 37) card = 4; // THE CHARIOT
		else if (rate < 47) card = 5; // STRENGTH
		else if (rate < 62) card = 6; // THE LOVERS
		else if (rate < 63) card = 7; // WHEEL OF FORTUNE
		else if (rate < 69) card = 8; // THE HANGED MAN
		else if (rate < 74) card = 9; // DEATH
		else if (rate < 82) card = 10; // TEMPERANCE
		else if (rate < 83) card = 11; // THE DEVIL
		else if (rate < 85) card = 12; // THE TOWER
		else if (rate < 90) card = 13; // THE STAR
		else card = 14; // THE SUN
	}

	switch (card) {
	case 1: // THE FOOL - heals SP to 0
	{
		status_percent_damage(src, target, 0, 100, false);
		break;
	}
	case 2: // THE MAGICIAN - matk halved
	{
		sc_start(src, target, SC_INCMATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		break;
	}
	case 3: // THE HIGH PRIESTESS - all buffs removed
	{
		status_change_clear_buffs(target, SCCB_BUFFS | SCCB_CHEM_PROTECT);
		break;
	}
	case 4: // THE CHARIOT - 1000 damage, random armor destroyed
	{
		battle_fix_damage(src, target, 1000, 0, skill_id);
		clif_damage(*src, *target, tick, 0, 0, 1000, 0, DMG_NORMAL, 0, false);
		if (!status_isdead(*target))
		{
			unsigned short where[] = { EQP_ARMOR, EQP_SHIELD, EQP_HELM };
			skill_break_equip(src, target, where[rnd() % 3], 10000, BCT_ENEMY);
		}
		break;
	}
	case 5: // STRENGTH - atk halved
	{
		sc_start(src, target, SC_INCATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		break;
	}
	case 6: // THE LOVERS - 2000HP heal, random teleported
	{
		status_heal(target, 2000, 0, 0);
		if (!map_flag_vs(target->m))
			unit_warp(target, -1, -1, -1, CLR_TELEPORT);
		break;
	}
	case 7: // WHEEL OF FORTUNE - random 2 other effects
	{
		// Recursive call
		skill_tarotcard(src, target, skill_id, skill_lv, tick);
		skill_tarotcard(src, target, skill_id, skill_lv, tick);
		break;
	}
	case 8: // THE HANGED MAN - ankle, freeze or stoned
	{
		enum sc_type sc[] = { SC_ANKLE, SC_FREEZE, SC_STONEWAIT };
		uint8 rand_eff = rnd() % 3;
		int time = ((rand_eff == 0) ? skill_get_time2(skill_id, skill_lv) : skill_get_time2(status_db.getSkill(sc[rand_eff]), 1));

		if (sc[rand_eff] == SC_STONEWAIT)
			sc_start2(src, target, SC_STONEWAIT, 100, skill_lv, src->id, time, skill_get_time(status_db.getSkill(SC_STONEWAIT), 1));
		else
			sc_start(src, target, sc[rand_eff], 100, skill_lv, time);
		break;
	}
	case 9: // DEATH - curse, coma and poison
	{
		status_change_start(src, target, SC_COMA, 10000, skill_lv, 0, src->id, 0, 0, SCSTART_NONE);
		sc_start(src, target, SC_CURSE, 100, skill_lv, skill_get_time2(status_db.getSkill(SC_CURSE), 1));
		sc_start2(src, target, SC_POISON, 100, skill_lv, src->id, skill_get_time2(status_db.getSkill(SC_POISON), 1));
		break;
	}
	case 10: // TEMPERANCE - confusion
	{
		sc_start(src, target, SC_CONFUSION, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;
	}
	case 11: // THE DEVIL - 6666 damage, atk and matk halved, cursed
	{
		battle_fix_damage(src, target, 6666, 0, skill_id);
		clif_damage(*src, *target, tick, 0, 0, 6666, 0, DMG_NORMAL, 0, false);
		sc_start(src, target, SC_INCATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCMATKRATE, 100, -50, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_CURSE, skill_lv, 100, skill_get_time2(status_db.getSkill(SC_CURSE), 1));
		break;
	}
	case 12: // THE TOWER - 4444 damage
	{
		battle_fix_damage(src, target, 4444, 0, skill_id);
		clif_damage(*src, *target, tick, 0, 0, 4444, 0, DMG_NORMAL, 0, false);
		break;
	}
	case 13: // THE STAR - stun
	{
		sc_start(src, target, SC_STUN, 100, skill_lv, skill_get_time2(status_db.getSkill(SC_STUN), 1));
		break;
	}
	default: // THE SUN - atk, matk, hit, flee and def reduced, immune to more tarot card effects
	{
#ifdef RENEWAL
		//In renewal, this card gives the SC_TAROTCARD status change which makes you immune to other cards
		sc_start(src, target, SC_TAROTCARD, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
#endif
		sc_start(src, target, SC_INCATKRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCMATKRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCHITRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCFLEERATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		sc_start(src, target, SC_INCDEFRATE, 100, -20, skill_get_time2(skill_id, skill_lv));
		return 14; //To make sure a valid number is returned
	}
	}

	return card;
}

/*==========================================
 *
 *
 *------------------------------------------*/
int skill_castend_damage_id (struct block_list* src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int flag)
{
	map_session_data *sd = nullptr;
	status_change *sc, *tsc;

	if (skill_id > 0 && !skill_lv) return 0;

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->m != bl->m)
		return 1;

	if (bl->prev == nullptr)
		return 1;

	sd = BL_CAST(BL_PC, src);

	if (status_isdead(*bl))
		return 1;

	if (skill_id && skill_id != AG_DEADLY_PROJECTION && skill_get_type(skill_id) == BF_MAGIC && status_isimmune(bl) == 100)
	{	//GTB makes all targetted magic display miss with a single bolt.
		sc_type sct = skill_get_sc(skill_id);
		if(sct != SC_NONE)
			status_change_end(bl, sct);
		clif_skill_damage(src, bl, tick, status_get_amotion(src), status_get_dmotion(bl), 0, 1, skill_id, skill_lv, skill_get_hit(skill_id));
		return 1;
	}

	sc = status_get_sc(src);
	tsc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = nullptr; //Unneeded
	if (tsc && !tsc->count)
		tsc = nullptr;

	status_data* tstatus = status_get_status_data(*bl);

	map_freeblock_lock();

	switch(skill_id) {
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
	case NPC_CHANGEUNDEAD:
	case NPC_ARMORBRAKE:
	case NPC_HELMBRAKE:
	case NPC_SHIELDBRAKE:
	case NPC_BLINDATTACK:
	case NPC_SILENCEATTACK:
	case NPC_STUNATTACK:
	case NPC_PETRIFYATTACK:
	case NPC_CURSEATTACK:
	case NPC_SLEEPATTACK:
#ifdef RENEWAL
	case CR_ACIDDEMONSTRATION:
#endif
	case LK_AURABLADE:
	case LK_SPIRALPIERCE:
	case ML_SPIRALPIERCE:
	case CG_ARROWVULCAN:
	case HW_MAGICCRASHER:
	case ITM_TOMAHAWK:
	case CH_CHAINCRUSH:
	case CH_TIGERFIST:
	case PA_SHIELDCHAIN:	// Shield Chain
	case PA_SACRIFICE:
	case WS_CARTTERMINATION:	// Cart Termination
	case AS_VENOMKNIFE:
	case HT_PHANTASMIC:
	case TK_DOWNKICK:
	case TK_COUNTER:
	case GS_CHAINACTION:
	case GS_TRIPLEACTION:
#ifndef RENEWAL
	case GS_MAGICALBULLET:
#endif
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
	case NPC_BLEEDING2:
	case NPC_CRITICALWOUND:
	case NPC_HELLPOWER:
	case RK_SONICWAVE:
	case AB_DUPLELIGHT_MELEE:
	case RA_AIMEDBOLT:
	case NC_BOOSTKNUCKLE:
	case NC_PILEBUNKER:
	case NC_AXEBOOMERANG:
	case NC_POWERSWING:
	case NPC_MAGMA_ERUPTION:
	case NC_MAGMA_ERUPTION:
	case GC_WEAPONCRUSH:
	case GC_VENOMPRESSURE:
	case SC_TRIANGLESHOT:
	case SC_FEINTBOMB:
	case LG_BANISHINGPOINT:
	case LG_SHIELDPRESS:
	case LG_RAGEBURST:
	case LG_HESPERUSLIT:
	case SR_DRAGONCOMBO:
	case SR_FALLENEMPIRE:
	case SR_CRESCENTELBOW_AUTOSPELL:
	case SR_GATEOFHELL:
	case SR_GENTLETOUCH_QUIET:
	case WM_SEVERE_RAINSTORM_MELEE:
	case WM_GREAT_ECHO:
	case GN_SLINGITEM_RANGEMELEEATK:
	case KO_SETSUDAN:
	case RL_MASS_SPIRAL:
	case RL_BANISHING_BUSTER:
	case RL_SLUGSHOT:
	case RL_AM_BLAST:
	case BO_ACIDIFIED_ZONE_WATER_ATK:
	case BO_ACIDIFIED_ZONE_GROUND_ATK:
	case BO_ACIDIFIED_ZONE_WIND_ATK:
	case BO_ACIDIFIED_ZONE_FIRE_ATK:
	case ABC_CHAIN_REACTION_SHOT_ATK:
	case ABR_BATTLE_BUSTER:
	case ABR_DUAL_CANNON_FIRE:
	case ABR_INFINITY_BUSTER:
	case MT_TRIPLE_LASER:
	case NW_MISSION_BOMBARD:
	case NW_HASTY_FIRE_IN_THE_HOLE:
	case NW_BASIC_GRENADE:
	case NW_WILD_FIRE:
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;
	case DK_DRAGONIC_AURA:
	case DK_STORMSLASH:
	case CD_EFFLIGO:
	case ABC_FRENZY_SHOT:
	case WH_HAWKRUSH:
	case WH_HAWKBOOMERANG:
	case TR_ROSEBLOSSOM:
	case TR_RHYTHMSHOOTING:
	case HN_MEGA_SONIC_BLOW:
	case HN_SPIRAL_PIERCE_MAX:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		if (skill_id == DK_DRAGONIC_AURA)
			sc_start(src, src, SC_DRAGONIC_AURA, 100, skill_lv, skill_get_time(skill_id,skill_lv));
		break;

	case SHC_ETERNAL_SLASH:
		if( sc && sc->getSCE(SC_E_SLASH_COUNT) )
			sc_start(src, src, SC_E_SLASH_COUNT, 100, min( 5, 1 + sc->getSCE(SC_E_SLASH_COUNT)->val1 ), skill_get_time(skill_id, skill_lv));
		else
			sc_start(src, src, SC_E_SLASH_COUNT, 100, 1, skill_get_time(skill_id, skill_lv));
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case SHC_SHADOW_STAB:
		if (sc && sc->getSCE(SC_CLOAKINGEXCEED))
			flag |= SKILL_ALTDMG_FLAG;

		status_change_end(src, SC_CLOAKING);
		status_change_end(src, SC_CLOAKINGEXCEED);

		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case WH_CRESCIVE_BOLT:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		if( sc && sc->getSCE(SC_CRESCIVEBOLT) )
			sc_start(src, src, SC_CRESCIVEBOLT, 100, min( 3, 1 + sc->getSCE(SC_CRESCIVEBOLT)->val1 ), skill_get_time(skill_id, skill_lv));
		else
			sc_start(src, src, SC_CRESCIVEBOLT, 100, 1, skill_get_time(skill_id, skill_lv));
		break;

	case ABC_UNLUCKY_RUSH:
		// Jump to the target before attacking.
		if (skill_check_unit_movepos(5, src, bl->x, bl->y, 0, 1))
			skill_blown(src, src, 1, (map_calc_dir(bl, src->x, src->y) + 4) % 8, BLOWN_NONE);

		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case MO_TRIPLEATTACK:
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag|SD_ANIMATION);
		break;

	case LK_HEADCRUSH:
		if (status_get_class_(bl) == CLASS_BOSS) {
			if (sd)
				clif_skill_fail( *sd, skill_id );
			break;
		}
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case LK_JOINTBEAT:
		flag = 1 << rnd() % 6;
		if (flag != BREAK_NECK && tsc && tsc->getSCE(SC_JOINTBEAT) && tsc->getSCE(SC_JOINTBEAT)->val2 & BREAK_NECK)
			flag = BREAK_NECK; // Target should always receive double damage if neck is already broken
		if (skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag))
			status_change_start(src, bl, SC_JOINTBEAT, (50 * (skill_lv + 1) - (270 * tstatus->str) / 100) * 10, skill_lv, flag & BREAK_FLAGS, src->id, 0, skill_get_time2(skill_id, skill_lv), SCSTART_NONE);
		break;

	case MO_COMBOFINISH:
		if (!(flag&1) && sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_MONK)
		{	//Becomes a splash attack when Soul Linked.
			map_foreachinshootrange(skill_area_sub, bl,
				skill_get_splash(skill_id, skill_lv),BL_CHAR|BL_SKILL,
				src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,
				skill_castend_damage_id);
		} else
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case TK_STORMKICK: // Taekwon kicks [Dralnu]
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		skill_area_temp[1] = 0;
		map_foreachinshootrange(skill_attack_area, src,
			skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL,
			BF_WEAPON, src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
		break;

	case KN_CHARGEATK:
		{
		bool path = path_search_long(nullptr, src->m, src->x, src->y, bl->x, bl->y,CELL_CHKWALL);
#ifdef RENEWAL
		int dist = skill_get_blewcount(skill_id, skill_lv);
#else
		unsigned int dist = distance_bl(src, bl);
#endif
		uint8 dir = map_calc_dir(bl, src->x, src->y);

		// teleport to target (if not on WoE grounds)
		if (skill_check_unit_movepos(5, src, bl->x, bl->y, 0, 1))
			skill_blown(src, src, 1, (dir+4)%8, BLOWN_NONE); //Target position is actually one cell next to the target

		// cause damage and knockback if the path to target was a straight one
		if (path) {
			if(skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, dist)) {
#ifdef RENEWAL
				if (map_getmapdata(src->m)->getMapFlag(MF_PVP))
					dist += 2; // Knockback is 4 on PvP maps
#endif
				skill_blown(src, bl, dist, dir, BLOWN_NONE);
			}
			//HACK: since knockback officially defaults to the left, the client also turns to the left... therefore,
			// make the caster look in the direction of the target
			unit_setdir(src, (dir+4)%8);
		}

		}
		break;

	case NC_FLAMELAUNCHER:
		skill_area_temp[1] = bl->id;
		if (battle_config.skill_eightpath_algorithm) {
			//Use official AoE algorithm
			map_foreachindir(skill_attack_area, src->m, src->x, src->y, bl->x, bl->y,
				skill_get_splash(skill_id, skill_lv), skill_get_maxcount(skill_id, skill_lv), 0, splash_target(src),
				skill_get_type(skill_id), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
		} else {
			map_foreachinpath(skill_attack_area, src->m, src->x, src->y, bl->x, bl->y,
				skill_get_splash(skill_id, skill_lv), skill_get_maxcount(skill_id, skill_lv), splash_target(src),
				skill_get_type(skill_id), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
		}
		break;

#ifndef RENEWAL
	case SN_SHARPSHOOTING:
		flag |= 2; // Flag for specific mob damage formula
		[[fallthrough]];
#endif
	case MA_SHARPSHOOTING:
	case NJ_KAMAITACHI:
	case NPC_DARKPIERCING:
	case NPC_ACIDBREATH:
	case NPC_DARKNESSBREATH:
	case NPC_FIREBREATH:
	case NPC_ICEBREATH:
	case NPC_ICEBREATH2:
	case NPC_THUNDERBREATH:
	case AG_STORM_CANNON:
	case AG_CRIMSON_ARROW:
		skill_area_temp[1] = bl->id;
		if (skill_id == AG_STORM_CANNON || skill_id == AG_CRIMSON_ARROW)
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		if (battle_config.skill_eightpath_algorithm) {
			//Use official AoE algorithm
			if (!(map_foreachindir(skill_attack_area, src->m, src->x, src->y, bl->x, bl->y,
			   skill_get_splash(skill_id, skill_lv), skill_get_maxcount(skill_id, skill_lv), 0, splash_target(src),
			   skill_get_type(skill_id), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY))) {
#ifndef RENEWAL
			   	if (skill_id == SN_SHARPSHOOTING)
			   		flag &= ~2; // Only targets in the splash area are affected
#endif

				//These skills hit at least the target if the AoE doesn't hit
				skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
			}
		} else {
			map_foreachinpath(skill_attack_area, src->m, src->x, src->y, bl->x, bl->y,
				skill_get_splash(skill_id, skill_lv), skill_get_maxcount(skill_id, skill_lv), splash_target(src),
				skill_get_type(skill_id), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
		}
		if (skill_id == AG_CRIMSON_ARROW)
			skill_castend_damage_id(src, bl, AG_CRIMSON_ARROW_ATK, skill_lv, tick, flag|SD_LEVEL|SD_ANIMATION);
		break;

	case MO_INVESTIGATE:
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		status_change_end(src, SC_BLADESTOP);
		break;

	case RG_BACKSTAP:
		{
			if (!check_distance_bl(src, bl, 0)) {
#ifdef RENEWAL
				uint8 dir = map_calc_dir(src, bl->x, bl->y);
				short x, y;

				if (dir > 0 && dir < 4)
					x = -1;
				else if (dir > 4)
					x = 1;
				else
					x = 0;

				if (dir > 2 && dir < 6)
					y = -1;
				else if (dir == 7 || dir < 2)
					y = 1;
				else
					y = 0;

				if (battle_check_target(src, bl, BCT_ENEMY) > 0 && unit_movepos(src, bl->x + x, bl->y + y, 2, true)) { // Display movement + animation.
#else
				uint8 dir = map_calc_dir(src, bl->x, bl->y), t_dir = unit_getdir(bl);

				if (!map_check_dir(dir, t_dir) || bl->type == BL_SKILL) {
#endif
					status_change_end(src, SC_HIDING);
					dir = dir < 4 ? dir+4 : dir-4; // change direction [Celest]
					unit_setdir(bl,dir);
#ifdef RENEWAL
					clif_blown(src);
#endif
					skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
				}
				else if (sd)
					clif_skill_fail( *sd, skill_id );
			}
		}
		break;

	case MO_FINGEROFFENSIVE:
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		if (battle_config.finger_offensive_type && sd) {
			for (int i = 1; i < sd->spiritball_old; i++)
				skill_addtimerskill(src, tick + i * 200, bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag);
		}
		status_change_end(src, SC_BLADESTOP);
		break;

	case MO_CHAINCOMBO:
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		status_change_end(src, SC_BLADESTOP);
		break;

#ifndef RENEWAL
	case NJ_ISSEN:
#endif
	case MO_EXTREMITYFIST:
		{
			struct block_list *mbl = bl; // For NJ_ISSEN
			short x, y, i = 2; // Move 2 cells (From target)
			short dir = map_calc_dir(src,bl->x,bl->y);

#ifdef RENEWAL
			if (skill_id == MO_EXTREMITYFIST && sd && sd->spiritball_old > 5)
				flag |= 1; // Give +100% damage increase
#endif
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			if (skill_id == MO_EXTREMITYFIST) {
				status_set_sp(src, 0, 0);
				sc_start(src, src, SC_EXTREMITYFIST, 100, skill_lv, skill_get_time(skill_id, skill_lv));
				status_change_end(src, SC_EXPLOSIONSPIRITS);
				status_change_end(src, SC_BLADESTOP);
			} else {
				status_set_hp(src, 1, 0);
				status_change_end(src, SC_NEN);
				status_change_end(src, SC_HIDING);
			}
			if (skill_id == MO_EXTREMITYFIST) {
				mbl = src; // For MO_EXTREMITYFIST
				i = 3; // Move 3 cells (From caster)
			}
			if (dir > 0 && dir < 4)
				x = -i;
			else if (dir > 4)
				x = i;
			else
				x = 0;
			if (dir > 2 && dir < 6)
				y = -i;
			else if (dir == 7 || dir < 2)
				y = i;
			else
				y = 0;
			// Ashura Strike still has slide effect in GVG
			if ((mbl == src || (!map_flag_gvg2(src->m) && !map_getmapflag(src->m, MF_BATTLEGROUND))) &&
				unit_movepos(src, mbl->x + x, mbl->y + y, 1, 1)) {
				clif_blown(src);
				clif_spiritball(src);
			}
		}
		break;

	case HT_POWER:
		if( tstatus->race == RC_BRUTE || tstatus->race == RC_PLAYER_DORAM || tstatus->race == RC_INSECT )
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case SU_PICKYPECK:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		[[fallthrough]];
	case SU_BITE:
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		break;
	case SU_SVG_SPIRIT:
		skill_area_temp[1] = bl->id;
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, bl->x, bl->y,
			skill_get_splash(skill_id, skill_lv), skill_get_maxcount(skill_id, skill_lv), splash_target(src),
			skill_get_type(skill_id), src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
		break;

	//Splash attack skills.
	case AS_GRIMTOOTH:
	case MC_CARTREVOLUTION:
	case NPC_SPLASHATTACK:
		flag |= SD_PREAMBLE; // a fake packet will be sent for the first target to be hit
		[[fallthrough]];
	case AS_SPLASHER:
	case HT_BLITZBEAT:
	case AC_SHOWER:
	case MA_SHOWER:
	case MG_NAPALMBEAT:
	case MG_FIREBALL:
	case RG_RAID:
#ifdef RENEWAL
	case SN_SHARPSHOOTING:
#endif
	case HW_NAPALMVULCAN:
	case NJ_HUUMA:
	case ASC_METEORASSAULT:
	case GS_SPREADATTACK:
	case NPC_PULSESTRIKE:
	case NPC_PULSESTRIKE2:
	case NPC_HELLJUDGEMENT:
	case NPC_HELLJUDGEMENT2:
	case NPC_VAMPIRE_GIFT:
	case NPC_MAXPAIN_ATK:
	case NPC_JACKFROST:
	case NPC_REVERBERATION_ATK:
	case NPC_ARROWSTORM:
	case NPC_KILLING_AURA:
	case NPC_IGNITIONBREAK:
	case RK_IGNITIONBREAK:
	case RK_HUNDREDSPEAR:
	case AB_JUDEX:
	case AB_ADORAMUS:
	case WL_SOULEXPANSION:
	case WL_CRIMSONROCK:
	case WL_JACKFROST:
	case RA_ARROWSTORM:
	case RA_WUGDASH:
	case NC_VULCANARM:
	case NC_COLDSLOWER:
	case NC_SELFDESTRUCTION:
	case NC_AXETORNADO:
	case GC_ROLLINGCUTTER:
	case GC_COUNTERSLASH:
	case LG_CANNONSPEAR:
	case LG_OVERBRAND:
	case LG_MOONSLASHER:
	case LG_RAYOFGENESIS:
	case NPC_RAYOFGENESIS:
	case LG_EARTHDRIVE:
	case SR_RAMPAGEBLASTER:
	case SR_SKYNETBLOW:
	case SR_WINDMILL:
	case SR_RIDEINLIGHTNING:
	case SO_VARETYR_SPEAR:
	case SO_POISON_BUSTER:
	case GN_CART_TORNADO:
	case GN_CARTCANNON:
	case GN_SPORE_EXPLOSION:
	case GN_DEMONIC_FIRE:
	case GN_FIRE_EXPANSION_ACID:
	case GN_HELLS_PLANT_ATK:
	case KO_HAPPOKUNAI:
	case KO_HUUMARANKA:
	case KO_MUCHANAGE:
	case KO_BAKURETSU:
	case GN_ILLUSIONDOPING:
	case RL_FIREDANCE:
	case RL_S_STORM:
	case RL_R_TRIP:
	case MH_XENO_SLASHER:
	case MH_HEILIGE_PFERD:
	case MH_THE_ONE_FIGHTER_RISES:
	case NC_ARMSCANNON:
	case SU_SCRATCH:
	case SU_LUNATICCARROTBEAT:
	case SU_LUNATICCARROTBEAT2:
	case SJ_FULLMOONKICK:
	case SJ_NEWMOONKICK:
	case SJ_SOLARBURST:
	case SJ_PROMINENCEKICK:
	case SJ_STAREMPEROR:
	case SJ_FALLINGSTAR_ATK2:
	case SP_CURSEEXPLOSION:
	case SP_SHA:
	case SP_SWHOO:
	case DK_SERVANTWEAPON_ATK:
	case DK_SERVANT_W_PHANTOM:
	case DK_SERVANT_W_DEMOL:
	case DK_MADNESS_CRUSHER:
	case DK_HACKANDSLASHER:
	case DK_DRAGONIC_BREATH:
	case AG_CRIMSON_ARROW_ATK:
	case AG_DESTRUCTIVE_HURRICANE:
	case AG_SOUL_VC_STRIKE:
	case AG_CRYSTAL_IMPACT:
	case AG_CRYSTAL_IMPACT_ATK:
	case AG_ROCK_DOWN:
	case AG_FROZEN_SLASH:
	case IQ_OLEUM_SANCTUM:
	case IQ_MASSIVE_F_BLASTER:
	case IQ_EXPOSION_BLASTER:
	case IQ_FIRST_BRAND:
	case IQ_SECOND_FLAME:
	case IQ_SECOND_FAITH:
	case IQ_SECOND_JUDGEMENT:
	case IQ_THIRD_PUNISH:
	case IQ_THIRD_FLAME_BOMB:
	case IQ_THIRD_CONSECRATION:
	case IG_OVERSLASH:
	case CD_ARBITRIUM_ATK:
	case CD_PETITIO:
	case CD_FRAMEN:
	case SHC_DANCING_KNIFE:
	case SHC_SAVAGE_IMPACT:
	case SHC_IMPACT_CRATER:
	case SHC_FATAL_SHADOW_CROW:
	case MT_AXE_STOMP:
	case MT_MIGHTY_SMASH:
	case MT_RUSH_QUAKE:
	case MT_A_MACHINE:
	case MT_SPARK_BLASTER:
	case ABC_ABYSS_DAGGER:
	case ABC_CHAIN_REACTION_SHOT:
	case ABC_DEFT_STAB:
	case WH_GALESTORM:
	case BO_ACIDIFIED_ZONE_WATER:
	case BO_ACIDIFIED_ZONE_GROUND:
	case BO_ACIDIFIED_ZONE_WIND:
	case BO_ACIDIFIED_ZONE_FIRE:
	case TR_ROSEBLOSSOM_ATK:
	case ABC_FROM_THE_ABYSS_ATK:
	case EM_ELEMENTAL_BUSTER_FIRE:
	case EM_ELEMENTAL_BUSTER_WATER:
	case EM_ELEMENTAL_BUSTER_WIND:
	case EM_ELEMENTAL_BUSTER_GROUND:
	case EM_ELEMENTAL_BUSTER_POISON:
	case EM_EL_FLAMEROCK:
	case EM_EL_AGE_OF_ICE:
	case EM_EL_STORM_WIND:
	case EM_EL_AVALANCHE:
	case EM_EL_DEADLY_POISON:
	case BO_EXPLOSIVE_POWDER:
	case BO_MAYHEMIC_THORNS:
	case NPC_WIDECRITICALWOUND:
	case IG_SHIELD_SHOOTING:
	case TR_METALIC_FURY:
	case IG_GRAND_JUDGEMENT:
	case HN_JUPITEL_THUNDER_STORM:
		if( flag&1 ) {//Recursive invocation
			int sflag = skill_area_temp[0] & 0xFFF;
			int heal = 0;
			std::bitset<INF2_MAX> inf2 = skill_db.find(skill_id)->inf2;

			if (tsc && tsc->getSCE(SC_HOVERING) && inf2[INF2_IGNOREHOVERING])
				break; // Under Hovering characters are immune to select trap and ground target skills.

			if (skill_id == AB_ADORAMUS && map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR))
				break; // No damage should happen if the target is on Land Protector

			// Servant Weapon - Demol only hits if the target is marked with a sign by the attacking caster.
			if (skill_id == DK_SERVANT_W_DEMOL && !(tsc && tsc->getSCE(SC_SERVANT_SIGN) && tsc->getSCE(SC_SERVANT_SIGN)->val1 == src->id))
				break;

			// Deft Stab - Make sure the flag of 2 is passed on when the skill is double casted.
			if (skill_id == ABC_DEFT_STAB && flag&2)
				sflag |= 2;

			if( flag&SD_LEVEL )
				sflag |= SD_LEVEL; // -1 will be used in packets instead of the skill level
			if( skill_area_temp[1] != bl->id && !inf2[INF2_ISNPC] )
				sflag |= SD_ANIMATION; // original target gets no animation (as well as all NPC skills)

			// If a enemy player is standing next to a mob when splash Es- skill is casted, the player won't get hurt.
			if ((skill_id == SP_SHA || skill_id == SP_SWHOO) && !battle_config.allow_es_magic_pc && bl->type != BL_MOB)
				break;

			heal = (int)skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, sflag);

			switch (skill_id) {
				case NPC_VAMPIRE_GIFT:
					if (heal > 0) {
						clif_skill_nodamage(nullptr, *src, AL_HEAL, heal);
						status_heal(src, heal, 0, 0);
					}
					break;
#ifdef RENEWAL
				case SN_SHARPSHOOTING:
					status_change_end(src, SC_CAMOUFLAGE);
					break;
#endif
				case SJ_PROMINENCEKICK: // Trigger the 2nd hit. (100% fire damage.)
					skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, sflag|8|SD_ANIMATION);
					break;
			}
		} else {
			int starget = BL_CHAR|BL_SKILL, splash_size = skill_get_splash(skill_id, skill_lv);

			skill_area_temp[0] = 0;
			skill_area_temp[1] = bl->id;
			skill_area_temp[2] = 0;

			if (sd && (skill_id == SP_SHA || skill_id == SP_SWHOO) && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
				status_change_start(src, bl, SC_STUN, 10000, skill_lv, 0, 0, 0, 500, 10);
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
				break;
			}

			if (skill_id == SP_SWHOO)
				status_change_end(src, SC_USE_SKILL_SP_SPA);

			switch ( skill_id ) {
				case LG_EARTHDRIVE:
				case GN_CARTCANNON:
				case SU_SCRATCH:
				case BO_MAYHEMIC_THORNS:
				case DK_DRAGONIC_BREATH:
				case DK_HACKANDSLASHER:
				case MT_SPARK_BLASTER:
				case HN_JUPITEL_THUNDER_STORM:
					clif_skill_nodamage(src,*bl,skill_id,skill_lv);
					break;
#ifdef RENEWAL
				case NJ_HUUMA:
#endif
				case LG_MOONSLASHER:
				case MH_XENO_SLASHER:
					clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
					break;
				case NPC_REVERBERATION_ATK:
				case NC_ARMSCANNON:
					skill_area_temp[1] = 0;
					starget = splash_target(src);
					break;
				case WL_CRIMSONROCK:
					skill_area_temp[4] = bl->x;
					skill_area_temp[5] = bl->y;
					break;
				case SU_LUNATICCARROTBEAT:
					if (sd && pc_search_inventory(sd, skill_db.find(SU_LUNATICCARROTBEAT)->require.itemid[0]) >= 0)
						skill_id = SU_LUNATICCARROTBEAT2;
					break;
				case DK_SERVANT_W_PHANTOM:
					// Jump to the target before attacking.
					if (skill_check_unit_movepos(5, src, bl->x, bl->y, 0, 1))
						skill_blown(src, src, 1, (map_calc_dir(bl, src->x, src->y) + 4) % 8, BLOWN_NONE);
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);// Trigger animation on servants.
					break;
				case SHC_SAVAGE_IMPACT: {
					if( sc && sc->getSCE( SC_CLOAKINGEXCEED ) ){
						skill_area_temp[0] = 2;
						status_change_end( src, SC_CLOAKINGEXCEED );
					}

					uint8 dir = DIR_NORTHEAST;	// up-right when src is on the same cell of target

					if (bl->x != src->x || bl->y != src->y)
						dir = map_calc_dir(bl, src->x, src->y);	// dir based on target as we move player based on target location

					// Move the player 1 cell near the target, between the target and the player
					if (skill_check_unit_movepos(5, src, bl->x + dirx[dir], bl->y + diry[dir], 0, 1))
						clif_blown(src);
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					break;
				}
				case SHC_FATAL_SHADOW_CROW: {
					uint8 dir = DIR_NORTHEAST;

					if (bl->x != src->x || bl->y != src->y)
						dir = map_calc_dir(bl, src->x, src->y);	// dir based on target as we move player based on target location

					// Move the player 1 cell near the target, between the target and the player
					if (skill_check_unit_movepos(5, src, bl->x + dirx[dir], bl->y + diry[dir], 0, 1))
						clif_blown(src);
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);// Trigger animation
					break;
				}
				case AG_CRYSTAL_IMPACT_ATK:
					if (sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 5)
						splash_size = 2;// Gives the aftershock hit a 5x5 splash AoE.
					break;
				case AG_ROCK_DOWN:
				case IQ_FIRST_BRAND:
				case IQ_SECOND_FLAME:
				case IQ_SECOND_FAITH:
				case IQ_SECOND_JUDGEMENT:
				case CD_PETITIO:
				case CD_FRAMEN:
				case ABC_DEFT_STAB:
				case EM_EL_FLAMEROCK:
				case EM_EL_AGE_OF_ICE:
				case EM_EL_STORM_WIND:
				case EM_EL_AVALANCHE:
				case EM_EL_DEADLY_POISON:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					break;
				case ABC_CHAIN_REACTION_SHOT:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					map_foreachinrange(skill_area_sub, bl, skill_get_splash(ABC_CHAIN_REACTION_SHOT_ATK, skill_lv), BL_CHAR|BL_SKILL, src, ABC_CHAIN_REACTION_SHOT_ATK, skill_lv, tick + (200 + status_get_amotion(src)), flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
					break;
				case IQ_THIRD_PUNISH:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					if (sd) {
						uint8 limit = 5;

						if (sc && sc->getSCE(SC_RAISINGDRAGON))
							limit += sc->getSCE(SC_RAISINGDRAGON)->val1;
						for (uint8 i = 0; i < limit; i++)
							pc_addspiritball(sd, skill_get_time(skill_id, skill_lv), limit);
					}
					break;
				case IQ_THIRD_FLAME_BOMB:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					if (sd && sd->spiritball / 5 > 1)
						skill_area_temp[0] = sd->spiritball / 5 - 1;
					break;
				case IQ_THIRD_CONSECRATION:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					status_heal(src, status_get_max_hp(src) * skill_lv / 100, status_get_max_sp(src) * skill_lv / 100, 0);
					break;
				case IG_OVERSLASH:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					skill_area_temp[0] = map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
					break;
				case WH_GALESTORM:// Give AP if 3 or more targets are hit.
					if (sd && map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill_area_sub_count) >= 3)
						status_heal(src, 0, 0, 10, 0);
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					break;
				case BO_ACIDIFIED_ZONE_WATER:
				case BO_ACIDIFIED_ZONE_GROUND:
				case BO_ACIDIFIED_ZONE_WIND:
				case BO_ACIDIFIED_ZONE_FIRE:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					if (bl->type == BL_PC)// Place single cell AoE if hitting a player.
						skill_castend_pos2(src, bl->x, bl->y, skill_id, skill_lv, tick, 0);
					break;
				case MT_RUSH_QUAKE:
					// Jump to the target before attacking.
					if( skill_check_unit_movepos( 5, src, bl->x, bl->y, 0, 1 ) ){
						skill_blown( src, src, 1, direction_opposite( static_cast<enum directions>( map_calc_dir( bl, src->x, src->y ) ) ), BLOWN_NONE);
					}
					clif_skill_nodamage( src, *bl, skill_id, skill_lv); // Trigger animation
					clif_blown( src );
					// TODO: does this buff start before or after dealing damage? [Muh]
					sc_start( src, src, SC_RUSH_QUAKE2, 100, skill_lv, skill_get_time2( skill_id, skill_lv ) );
					break;
				case IG_SHIELD_SHOOTING:
				case IG_GRAND_JUDGEMENT:
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
					sc_start(src, src, skill_get_sc(skill_id), 100, skill_lv, skill_get_time(skill_id, skill_lv));
					break;
			}

			// if skill damage should be split among targets, count them
			//SD_LEVEL -> Forced splash damage for Auto Blitz-Beat -> count targets
			//special case: Venom Splasher uses a different range for searching than for splashing
			if( flag&SD_LEVEL || skill_get_nk(skill_id, NK_SPLASHSPLIT) )
				skill_area_temp[0] = map_foreachinallrange(skill_area_sub, bl, (skill_id == AS_SPLASHER)?1:splash_size, BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill_area_sub_count);

			// recursive invocation of skill_castend_damage_id() with flag|1
			map_foreachinrange(skill_area_sub, bl, splash_size, starget, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);

			if (skill_id == RA_ARROWSTORM)
				status_change_end(src, SC_CAMOUFLAGE);
			if( skill_id == AS_SPLASHER ) {
				map_freeblock_unlock(); // Don't consume a second gemstone.
				return 0;
			}
		}
		break;
	case NW_THE_VIGILANTE_AT_NIGHT:
		if (flag & 1)
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		break;
	case NW_SPIRAL_SHOOTING:
		if (flag & 1) {
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		} else {
			int splash = skill_get_splash(skill_id, skill_lv);

			if (sd && sd->weapontype1 == W_GRENADE)
				splash += 2;
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinrange(skill_area_sub, bl, splash, BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
			if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
				status_change_end(src, SC_INTENSIVE_AIM_COUNT);
		}
		break;

	//Place units around target
	case NJ_BAKUENRYU:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		skill_unitsetting(src, skill_id, skill_lv, bl->x, bl->y, 0);
		break;

	case SM_MAGNUM:
	case MS_MAGNUM:
		if( flag&1 ) {
			// For players, damage depends on distance, so add it to flag if it is > 1
			// Cannot hit hidden targets
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION|(sd?distance_bl(src, bl):0));
		}
		break;

#ifdef RENEWAL
	case KN_BRANDISHSPEAR:
		skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		break;
#else
	case KN_BRANDISHSPEAR:
#endif
	case ML_BRANDISH:
		//Coded apart for it needs the flag passed to the damage calculation.
		if (skill_area_temp[1] != bl->id)
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION);
		else
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		break;

#ifdef RENEWAL
	case KN_BOWLINGBASH:
		if (flag & 1) {
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, (skill_area_temp[0]) > 0 ? SD_ANIMATION | skill_area_temp[0] : skill_area_temp[0]);
		} else {
			skill_area_temp[0] = map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		}
		break;
#else
	case KN_BOWLINGBASH:
#endif
	case MS_BOWLINGBASH:
		{
			int min_x,max_x,min_y,max_y,i,c,dir,tx,ty;
			// Chain effect and check range gets reduction by recursive depth, as this can reach 0, we don't use blowcount
			c = (skill_lv-(flag&0xFFF)+1)/2;
			// Determine the Bowling Bash area depending on configuration
			if (battle_config.bowling_bash_area == 0) {
				// Gutter line system
				min_x = ((src->x)-c) - ((src->x)-c)%40;
				if(min_x < 0) min_x = 0;
				max_x = min_x + 39;
				min_y = ((src->y)-c) - ((src->y)-c)%40;
				if(min_y < 0) min_y = 0;
				max_y = min_y + 39;
			} else if (battle_config.bowling_bash_area == 1) {
				// Gutter line system without demi gutter bug
				min_x = src->x - (src->x)%40;
				max_x = min_x + 39;
				min_y = src->y - (src->y)%40;
				max_y = min_y + 39;
			} else {
				// Area around caster
				min_x = src->x - battle_config.bowling_bash_area;
				max_x = src->x + battle_config.bowling_bash_area;
				min_y = src->y - battle_config.bowling_bash_area;
				max_y = src->y + battle_config.bowling_bash_area;
			}
			// Initialization, break checks, direction
			if((flag&0xFFF) > 0) {
				// Ignore monsters outside area
				if(bl->x < min_x || bl->x > max_x || bl->y < min_y || bl->y > max_y)
					break;
				// Ignore monsters already in list
				if(idb_exists(bowling_db, bl->id))
					break;
				// Random direction
				dir = rnd()%8;
			} else {
				// Create an empty list of already hit targets
				db_clear(bowling_db);
				// Direction is walkpath
				dir = (unit_getdir(src)+4)%8;
			}
			// Add current target to the list of already hit targets
			idb_put(bowling_db, bl->id, bl);
			// Keep moving target in direction square by square
			tx = bl->x;
			ty = bl->y;
			for(i=0;i<c;i++) {
				// Target coordinates (get changed even if knockback fails)
				tx -= dirx[dir];
				ty -= diry[dir];
				// If target cell is a wall then break
				if(map_getcell(bl->m,tx,ty,CELL_CHKWALL))
					break;
				skill_blown(src,bl,1,dir,BLOWN_NONE);

				int count;

				// Splash around target cell, but only cells inside area; we first have to check the area is not negative
				if((max(min_x,tx-1) <= min(max_x,tx+1)) &&
					(max(min_y,ty-1) <= min(max_y,ty+1)) &&
					(count = map_foreachinallarea(skill_area_sub, bl->m, max(min_x,tx-1), max(min_y,ty-1), min(max_x,tx+1), min(max_y,ty+1), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY, skill_area_sub_count))) {
					// Recursive call
					map_foreachinallarea(skill_area_sub, bl->m, max(min_x,tx-1), max(min_y,ty-1), min(max_x,tx+1), min(max_y,ty+1), splash_target(src), src, skill_id, skill_lv, tick, (flag|BCT_ENEMY)+1, skill_castend_damage_id);
					// Self-collision
					if(bl->x >= min_x && bl->x <= max_x && bl->y >= min_y && bl->y <= max_y)
						skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,(flag&0xFFF)>0?SD_ANIMATION|count:count);
					break;
				}
			}
			// Original hit or chain hit depending on flag
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,(flag&0xFFF)>0?SD_ANIMATION:0);
		}
		break;

	case KN_SPEARSTAB:
		if(flag&1) {
			if (bl->id==skill_area_temp[1])
				break;
			if (skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,SD_ANIMATION))
				skill_blown(src,bl,skill_area_temp[2],-1,BLOWN_NONE);
		} else {
			int x=bl->x,y=bl->y,i,dir;
			dir = map_calc_dir(bl,src->x,src->y);
			skill_area_temp[1] = bl->id;
			skill_area_temp[2] = skill_get_blewcount(skill_id,skill_lv);
			// all the enemies between the caster and the target are hit, as well as the target
			if (skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,0))
				skill_blown(src,bl,skill_area_temp[2],-1,BLOWN_NONE);
			for (i=0;i<4;i++) {
				map_foreachincell(skill_area_sub,bl->m,x,y,BL_CHAR,
					src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
				x += dirx[dir];
				y += diry[dir];
			}
		}
		break;

	case TK_TURNKICK:
	case MO_BALKYOUNG: //Active part of the attack. Skill-attack [Skotlex]
	{
		skill_area_temp[1] = bl->id; //NOTE: This is used in skill_castend_nodamage_id to avoid affecting the target.
		if (skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag))
			map_foreachinallrange(skill_area_sub,bl,
				skill_get_splash(skill_id, skill_lv),BL_CHAR,
				src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,
				skill_castend_nodamage_id);
	}
		break;
	case CH_PALMSTRIKE: //	Palm Strike takes effect 1sec after casting. [Skotlex]
	//	clif_skill_nodamage(src,*bl,skill_id,skill_lv,false); //Can't make this one display the correct attack animation delay :/
		clif_damage(*src,*bl,tick,status_get_amotion(src),0,-1,1,DMG_ENDURE,0,false); //Display an absorbed damage attack.
		skill_addtimerskill(src, tick + (1000+status_get_amotion(src)), bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag);
		break;

	case PR_TURNUNDEAD:
	case ALL_RESURRECTION:
		if (!battle_check_undead(tstatus->race, tstatus->def_ele))
			break;
		skill_attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case AL_HOLYLIGHT:
		status_change_end(bl, SC_P_ALTER);
		[[fallthrough]];
	case MG_SOULSTRIKE:
	case NPC_DARKSTRIKE:
	case MG_COLDBOLT:
	case MG_FIREBOLT:
	case MG_LIGHTNINGBOLT:
	case WZ_EARTHSPIKE:
	case AL_HEAL:
	case NPC_DARKTHUNDER:
	case NPC_FIRESTORM:
	case PR_ASPERSIO:
	case MG_FROSTDIVER:
	case WZ_SIGHTBLASTER:
	case WZ_SIGHTRASHER:
#ifdef RENEWAL
	case PA_PRESSURE:
#endif
	case NJ_KOUENKA:
	case NJ_HYOUSENSOU:
	case NJ_HUUJIN:
	case AB_HIGHNESSHEAL:
	case AB_DUPLELIGHT_MAGIC:
	case WM_METALICSOUND:
	case KO_KAIHOU:
	case MH_ERASER_CUTTER:
	case AG_ASTRAL_STRIKE:
	case AG_ASTRAL_STRIKE_ATK:
	case AG_DESTRUCTIVE_HURRICANE_CLIMAX:
	case CD_ARBITRIUM:
	case HN_METEOR_STORM_BUSTER:
		skill_attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case IG_JUDGEMENT_CROSS:
	case TR_SOUNDBLEND:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		skill_attack(BF_MAGIC, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case AG_DEADLY_PROJECTION:
		sc_start(src, bl, SC_DEADLY_DEFEASANCE, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		skill_attack(BF_MAGIC, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case NPC_MAGICALATTACK:
		skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		sc_start(src,src,SC_MAGICALATTACK,100,skill_lv,skill_get_time(skill_id,skill_lv));
		break;

	case HVAN_CAPRICE:
		{
			static const std::array<e_skill, 4> subskills = { MG_COLDBOLT, MG_FIREBOLT, MG_LIGHTNINGBOLT, WZ_EARTHSPIKE };
			e_skill subskill_id = subskills.at(rnd() % subskills.size());
			skill_attack(skill_get_type(subskill_id), src, src, bl, subskill_id, skill_lv, tick, flag);
		}
		break;

	case WZ_WATERBALL:
		//Deploy waterball cells, these are used and turned into waterballs via the timerskill
		skill_unitsetting(src, skill_id, skill_lv, src->x, src->y, 0);
		skill_addtimerskill(src, tick, bl->id, src->x, src->y, skill_id, skill_lv, 0, flag);
		break;
	case WZ_JUPITEL:
		//Jupitel Thunder is delayed by 150ms, you can cast another spell before the knockback
		skill_addtimerskill(src, tick+TIMERSKILL_INTERVAL, bl->id, 0, 0, skill_id, skill_lv, 1, flag);
		break;

	case PR_BENEDICTIO:
		//Should attack undead and demons. [Skotlex]
		if (battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race == RC_DEMON)
			skill_attack(BF_MAGIC, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case SJ_NOVAEXPLOSING:
		skill_attack(BF_MISC, src, src, bl, skill_id, skill_lv, tick, flag);

		// We can end Dimension here since the cooldown code is processed before this point.
		if (sc && sc->getSCE(SC_DIMENSION))
			status_change_end(src, SC_DIMENSION);
		else // Dimension not active? Activate the 2 second skill block penalty.
			sc_start(src, &sd->bl, SC_NOVAEXPLOSING, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
		
	case SP_SOULEXPLOSION:
		if (!(tsc && (tsc->getSCE(SC_SPIRIT) || tsc->getSCE(SC_SOULGOLEM) || tsc->getSCE(SC_SOULSHADOW) || tsc->getSCE(SC_SOULFALCON) || tsc->getSCE(SC_SOULFAIRY))) || tstatus->hp < 10 * tstatus->max_hp / 100) { // Requires target to have a soul link and more then 10% of MaxHP.
			// With this skill requiring a soul link, and the target to have more then 10% if MaxHP, I wonder
			// if the cooldown still happens after it fails. Need a confirm. [Rytech] 
			if (sd)
				clif_skill_fail( *sd, skill_id );
			break;
		}

		skill_attack(BF_MISC, src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case SL_SMA:
		status_change_end(src, SC_SMA);
		[[fallthrough]];
	case SL_STIN:
	case SL_STUN:
	case SP_SPA:
		if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
			status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			clif_skill_fail( *sd, skill_id );
			break;
		}
		skill_attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case NPC_DARKBREATH:
		clif_emotion(src,ET_ANGER);
		if (rnd() % 2 == 0)
			break; // 50% chance
		[[fallthrough]];
	case SN_FALCONASSAULT:
#ifndef RENEWAL
	case PA_PRESSURE:
	case CR_ACIDDEMONSTRATION:
#endif
	case TF_THROWSTONE:
	case NPC_SMOKING:
	case GS_FLING:
	case NJ_ZENYNAGE:
	case GN_THORNS_TRAP:
	case RL_B_TRAP:
		skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		break;
#ifdef RENEWAL
	case NJ_ISSEN: {
		short x, y;
		short dir = map_calc_dir(src, bl->x, bl->y);

		// Move 2 cells (From target)
		if (dir > 0 && dir < 4)
			x = -2;
		else if (dir > 4)
			x = 2;
		else
			x = 0;
		if (dir > 2 && dir < 6)
			y = -2;
		else if (dir == 7 || dir < 2)
			y = 2;
		else
			y = 0;
		// Doesn't have slide effect in GVG
		if (skill_check_unit_movepos(5, src, bl->x + x, bl->y + y, 1, 1)) {
			clif_blown(src);
			clif_spiritball(src);
		}
		skill_attack(BF_MISC, src, src, bl, skill_id, skill_lv, tick, flag);
		status_set_hp(src, umax(status_get_max_hp(src) / 100, 1), 0);
		status_change_end(src, SC_NEN);
		status_change_end(src, SC_HIDING);
	}
	break;
#endif
	case RK_DRAGONBREATH_WATER:
	case RK_DRAGONBREATH:
	case NPC_DRAGONBREATH:
		if( tsc && tsc->getSCE(SC_HIDING) )
			clif_skill_nodamage(src,*src,skill_id,skill_lv);
		else {
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		}
		break;

	case NPC_SELFDESTRUCTION:
		if( tsc && tsc->getSCE(SC_HIDING) )
			break;
		[[fallthrough]];
	case HVAN_EXPLOSION:
		if (src != bl)
			skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	// Celest
	case PF_SOULBURN:
		if (rnd()%100 < (skill_lv < 5 ? 30 + skill_lv * 10 : 70)) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			if (skill_lv == 5)
				skill_attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
			status_percent_damage(src, bl, 0, 100, false);
		} else {
			clif_skill_nodamage(src,*src,skill_id,skill_lv);
			if (skill_lv == 5)
				skill_attack(BF_MAGIC,src,src,src,skill_id,skill_lv,tick,flag);
			status_percent_damage(src, src, 0, 100, false);
		}
		break;

	case NPC_BLOODDRAIN:
	case NPC_ENERGYDRAIN:
		{
			int heal = (int)skill_attack( (skill_id == NPC_BLOODDRAIN) ? BF_WEAPON : BF_MAGIC,
					src, src, bl, skill_id, skill_lv, tick, flag);
			if (heal > 0){
				clif_skill_nodamage(nullptr, *src, AL_HEAL, heal);
				status_heal(src, heal, 0, 0);
			}
		}
		break;

	case GS_BULLSEYE:
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case NJ_KASUMIKIRI:
		if (skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag) > 0)
			sc_start(src,src,SC_HIDING,100,skill_lv,skill_get_time(skill_id,skill_lv));
		break;
	case NJ_KIRIKAGE:
		if( !map_flag_gvg2(src->m) && !map_getmapflag(src->m, MF_BATTLEGROUND) )
		{	//You don't move on GVG grounds.
			short x, y;
			map_search_freecell(bl, 0, &x, &y, 1, 1, 0);
			if (unit_movepos(src, x, y, 0, 0)) {
				clif_blown(src);
			}
		}
		status_change_end(src, SC_HIDING);
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;
	case RK_PHANTOMTHRUST:
	case NPC_PHANTOMTHRUST:
		unit_setdir(src,map_calc_dir(src, bl->x, bl->y));
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);

		skill_blown(src,bl,distance_bl(src,bl)-1,unit_getdir(src),BLOWN_NONE);
		if( battle_check_target(src,bl,BCT_ENEMY) > 0 )
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;
	case RK_WINDCUTTER:
	case RK_STORMBLAST:
		if( flag&1 )
			skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		else {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			map_foreachinallrange(skill_area_sub, bl,skill_get_splash(skill_id, skill_lv),BL_CHAR,src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,skill_castend_nodamage_id);
		}
		break;
	case GC_DARKILLUSION:
		{
			short x, y;
			short dir = map_calc_dir(src,bl->x,bl->y);

			if( dir > 0 && dir < 4) x = 2;
			else if( dir > 4 ) x = -2;
			else x = 0;
			if( dir > 2 && dir < 6 ) y = 2;
			else if( dir == 7 || dir < 2 ) y = -2;
			else y = 0;

			if( unit_movepos(src, bl->x+x, bl->y+y, 1, 1) ) {
				clif_blown(src);
				skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
				if( rnd()%100 < 4 * skill_lv )
					skill_castend_damage_id(src,bl,GC_CROSSIMPACT,skill_lv,tick,flag);
			}

		}
		break;

	case GC_CROSSRIPPERSLASHER:
		if( sd && !(sc && sc->getSCE(SC_ROLLINGCUTTER)) )
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_CONDITION );
		else
		{
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		}
		break;
	case GC_CROSSIMPACT: {
		uint8 dir = DIR_NORTHEAST;

		if (bl->x != src->x || bl->y != src->y)
			dir = map_calc_dir(bl, src->x, src->y);	// dir based on target as we move player based on target location

		if (skill_check_unit_movepos(0, src, bl->x + dirx[dir], bl->y + diry[dir], 1, 1)) {
			clif_blown(src);
			skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		} else {
			if (sd)
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
		}
		break;
	}
	case GC_PHANTOMMENACE:
		if (flag&1) { // Only Hits Invisible Targets
			if(tsc && (tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD))) {
				status_change_end(bl, SC_CLOAKINGEXCEED);
				skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			}
			if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM); // Should only end, no damage dealt.
		}
		break;

	case GC_DARKCROW:
		skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
		sc_start(src, bl, SC_DARKCROW, 100, skill_lv, skill_get_time(skill_id, skill_lv)); // Should be applied even on miss
		break;

	case WL_DRAINLIFE:
		{
			int heal = (int)skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
			int rate = 70 + 5 * skill_lv;

			heal = heal * (5 + 5 * skill_lv) / 100;

			if( bl->type == BL_SKILL )
				heal = 0; // Don't absorb heal from Ice Walls or other skill units.

			if( heal && rnd()%100 < rate )
			{
				status_heal(src, heal, 0, 0);
				clif_skill_nodamage(nullptr, *src, AL_HEAL, heal);
			}
		}
		break;

	case WL_TETRAVORTEX_FIRE:
	case WL_TETRAVORTEX_WATER:
	case WL_TETRAVORTEX_WIND:
	case WL_TETRAVORTEX_GROUND:
		skill_addtimerskill(src, tick + skill_area_temp[0] * 200, bl->id, skill_area_temp[1], 0, skill_id, skill_lv, 0, flag);
		break;

	case WL_TETRAVORTEX:
		if (sd == nullptr) { // Monster usage
			uint8 i = 0;
			const static std::vector<std::vector<uint16>> tetra_skills = { { WL_TETRAVORTEX_FIRE, 1 },
																		   { WL_TETRAVORTEX_WIND, 4 },
																		   { WL_TETRAVORTEX_WATER, 2 },
																		   { WL_TETRAVORTEX_GROUND, 8 } };

			for (const auto &skill : tetra_skills) {
				if (skill_lv > 5) {
					skill_area_temp[0] = i;
					skill_area_temp[1] = skill[1];
					map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill[0], skill_lv, tick, flag | BCT_ENEMY, skill_castend_damage_id);
				} else
					skill_addtimerskill(src, tick + i * 200, bl->id, skill[1], 0, skill[0], skill_lv, i, flag);
				i++;
			}
		} else if (sc) { // No SC? No spheres
			int i, k = 0;

			if (sc->getSCE(SC_SPHERE_5)) // If 5 spheres, remove last one (based on reverse order) and only do 4 actions (Official behavior)
				status_change_end(src, SC_SPHERE_1);

			for (i = SC_SPHERE_5; i >= SC_SPHERE_1; i--) { // Loop should always be 4 for regular players, but unconditional_skill could be less
				if (sc->getSCE(static_cast<sc_type>(i)) == nullptr)
					continue;

				uint16 subskill = 0;

				switch (sc->getSCE(static_cast<sc_type>(i))->val1) {
					case WLS_FIRE:
						subskill = WL_TETRAVORTEX_FIRE;
						k |= 1;
						break;
					case WLS_WIND:
						subskill = WL_TETRAVORTEX_WIND;
						k |= 4;
						break;
					case WLS_WATER:
						subskill = WL_TETRAVORTEX_WATER;
						k |= 2;
						break;
					case WLS_STONE:
						subskill = WL_TETRAVORTEX_GROUND;
						k |= 8;
						break;
				}

				if (skill_lv > 5) {
					skill_area_temp[0] = abs(i - SC_SPHERE_5);
					skill_area_temp[1] = k;
					map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, subskill, skill_lv, tick, flag | BCT_ENEMY, skill_castend_damage_id);
				} else
					skill_addtimerskill(src, tick + abs(i - SC_SPHERE_5) * 200, bl->id, k, 0, subskill, skill_lv, abs(i - SC_SPHERE_5), flag);
				status_change_end(src, static_cast<sc_type>(i));
			}
		}
		break;

	case WL_RELEASE:
		if (sc == nullptr)
			break;
		if (sd) {
			int i;

#ifndef RENEWAL
			skill_toggle_magicpower(src, skill_id); // No hit will be amplified
#endif
			if (skill_lv == 1) { // SpellBook
				if (sc->getSCE(SC_FREEZE_SP) == nullptr)
					break;

				bool found_spell = false;

				for (i = SC_MAXSPELLBOOK; i >= SC_SPELLBOOK1; i--) { // List all available spell to be released
					if (sc->getSCE(i) != nullptr) {
						found_spell = true;
						break;
					}
				}

				if (!found_spell)
					break;

				// Now extract the data from the preserved spell
				uint16 pres_skill_id = sc->getSCE(i)->val1;
				uint16 pres_skill_lv = sc->getSCE(i)->val2;
				uint16 point = sc->getSCE(i)->val3;

				status_change_end(src, static_cast<sc_type>(i));

				if( sc->getSCE(SC_FREEZE_SP)->val2 > point )
					sc->getSCE(SC_FREEZE_SP)->val2 -= point;
				else // Last spell to be released
					status_change_end(src, SC_FREEZE_SP);

				if( !skill_check_condition_castbegin(*sd, pres_skill_id, pres_skill_lv) )
					break;

				// Get the requirement for the preserved skill
				skill_consume_requirement(sd, pres_skill_id, pres_skill_lv, 1);

				switch( skill_get_casttype(pres_skill_id) )
				{
					case CAST_GROUND:
						skill_castend_pos2(src, bl->x, bl->y, pres_skill_id, pres_skill_lv, tick, 0);
						break;
					case CAST_NODAMAGE:
						skill_castend_nodamage_id(src, bl, pres_skill_id, pres_skill_lv, tick, 0);
						break;
					case CAST_DAMAGE:
						skill_castend_damage_id(src, bl, pres_skill_id, pres_skill_lv, tick, 0);
						break;
				}

				sd->ud.canact_tick = i64max(tick + skill_delayfix(src, pres_skill_id, pres_skill_lv), sd->ud.canact_tick);
				clif_status_change(src, EFST_POSTDELAY, 1, skill_delayfix(src, pres_skill_id, pres_skill_lv), 0, 0, 0);

				int cooldown = pc_get_skillcooldown(sd,pres_skill_id, pres_skill_lv);

				if( cooldown > 0 )
					skill_blockpc_start(sd, pres_skill_id, cooldown);
			} else { // Summoned Balls
				for (i = SC_SPHERE_5; i >= SC_SPHERE_1; i--) {
					if (sc->getSCE(static_cast<sc_type>(i)) == nullptr)
						continue;

					int skele = WL_RELEASE - 5 + sc->getSCE(static_cast<sc_type>(i))->val1 - WLS_FIRE; // Convert Ball Element into Skill ATK for balls

					// WL_SUMMON_ATK_FIRE, WL_SUMMON_ATK_WIND, WL_SUMMON_ATK_WATER, WL_SUMMON_ATK_GROUND
					skill_addtimerskill(src, tick + (t_tick)status_get_adelay(src) * abs(i - SC_SPHERE_1), bl->id, 0, 0, skele, sc->getSCE(static_cast<sc_type>(i))->val2, BF_MAGIC, flag | SD_LEVEL);
					status_change_end(src, static_cast<sc_type>(i)); // Eliminate ball
				}
				clif_skill_nodamage(src, *bl, skill_id, 0);
			}
		}
		break;
	case WL_FROSTMISTY:
		// Causes Freezing status through walls.
		sc_start(src, bl, SC_FREEZING, 25 + 5 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		sc_start(src, bl, SC_MISTY_FROST, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		// Doesn't deal damage through non-shootable walls.
		if( !battle_config.skill_wall_check || (battle_config.skill_wall_check && path_search(nullptr,src->m,src->x,src->y,bl->x,bl->y,1,CELL_CHKWALL)) )
			skill_attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag|SD_ANIMATION);
		break;
	case WL_HELLINFERNO:
		if (flag & 1) {
			skill_attack(BF_MAGIC, src, src, bl, skill_id, skill_lv, tick, flag);
			skill_addtimerskill(src, tick + 300, bl->id, 0, 0, skill_id, skill_lv, BF_MAGIC, flag | 2);
		} else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		}
		break;
	case RA_WUGSTRIKE:
		if( sd && pc_isridingwug(sd) ){
			uint8 dir = map_calc_dir(bl, src->x, src->y);

			if( unit_movepos(src, bl->x+dirx[dir], bl->y+diry[dir], 1, 1) ) {
				clif_blown(src);
				skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			}
			break;
		}
		[[fallthrough]];
	case RA_WUGBITE:
		if( path_search(nullptr,src->m,src->x,src->y,bl->x,bl->y,1,CELL_CHKNOREACH) ) {
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		}else if( sd && skill_id == RA_WUGBITE ) // Only RA_WUGBITE has the skill fail message.
			clif_skill_fail( *sd, skill_id );

		break;

	case RA_SENSITIVEKEEN:
		if( bl->type != BL_SKILL ) { // Only Hits Invisible Targets
			if (tsc && ((tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD))) {
				status_change_end(bl, SC_CLOAKINGEXCEED);
				skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			}
			if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM); // Should only end, no damage dealt.
		} else {
			skill_unit *su = BL_CAST(BL_SKILL, bl);
			std::shared_ptr<s_skill_unit_group> sg;

			if (su && (sg = su->group) && skill_get_inf2(sg->skill_id, INF2_ISTRAP)) {
				if( !(sg->unit_id == UNT_USED_TRAPS || (sg->unit_id == UNT_ANKLESNARE && sg->val2 != 0 )) )
				{
					struct item item_tmp;
					memset(&item_tmp,0,sizeof(item_tmp));
					item_tmp.nameid = sg->item_id?sg->item_id:ITEMID_TRAP;
					item_tmp.identify = 1;
					if( item_tmp.nameid )
						map_addflooritem(&item_tmp,1,bl->m,bl->x,bl->y,0,0,0,4,0);
				}
				skill_delunit(su);
			}
		}
		break;
	case NC_INFRAREDSCAN:
		if( flag&1 ) {
			status_change_end(bl, SC_HIDING);
			status_change_end(bl, SC_CLOAKING);
			status_change_end(bl, SC_CLOAKINGEXCEED);
			status_change_end(bl, SC_CAMOUFLAGE);
			status_change_end(bl, SC_NEWMOON);
			if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM);
			sc_start(src,bl, SC_INFRAREDSCAN, 10000, skill_lv, skill_get_time(skill_id, skill_lv));
		} else {
			clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		}
		break;
	case SC_FATALMENACE:
		if( flag&1 )
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		else {
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
			clif_skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skill_id,skill_lv,DMG_SINGLE);
		}
		break;
	
	case NPC_FATALMENACE:	// todo should it teleport the target ?
		if( flag&1 )
			skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		else {
			short x, y;
			map_search_freecell(src, 0, &x, &y, -1, -1, 0);
			// Destination area
			skill_area_temp[4] = x;
			skill_area_temp[5] = y;
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
			skill_addtimerskill(src,tick + 800,src->id,x,y,skill_id,skill_lv,0,flag); // To teleport Self
			clif_skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skill_id,skill_lv,DMG_SINGLE);
		}
		break;
	
	case LG_PINPOINTATTACK:
		if (skill_check_unit_movepos(5, src, bl->x, bl->y, 1, 1))
			clif_blown(src);
		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case SR_KNUCKLEARROW:
		// Holds current direction of bl/target to src/attacker before the src is moved to bl location
		dir_ka = map_calc_dir(bl, src->x, src->y);
		// Has slide effect
		if (skill_check_unit_movepos(5, src, bl->x, bl->y, 1, 1))
			skill_blown(src, src, 1, (dir_ka + 4) % 8, BLOWN_NONE); // Target position is actually one cell next to the target
		skill_addtimerskill(src, tick + 300, bl->id, 0, 0, skill_id, skill_lv, BF_WEAPON, flag|SD_LEVEL|2);
		break;

	case SR_HOWLINGOFLION:
			status_change_end(bl, SC_SWINGDANCE);
			status_change_end(bl, SC_SYMPHONYOFLOVER);
			status_change_end(bl, SC_MOONLITSERENADE);
			status_change_end(bl, SC_RUSHWINDMILL);
			status_change_end(bl, SC_ECHOSONG);
			status_change_end(bl, SC_HARMONIZE);
			status_change_end(bl, SC_NETHERWORLD);
			status_change_end(bl, SC_VOICEOFSIREN);
			status_change_end(bl, SC_DEEPSLEEP);
			status_change_end(bl, SC_SIRCLEOFNATURE);
			status_change_end(bl, SC_GLOOMYDAY);
			status_change_end(bl, SC_GLOOMYDAY_SK);
			status_change_end(bl, SC_SONGOFMANA);
			status_change_end(bl, SC_DANCEWITHWUG);
			status_change_end(bl, SC_SATURDAYNIGHTFEVER);
			status_change_end(bl, SC_LERADSDEW);
			status_change_end(bl, SC_MELODYOFSINK);
			status_change_end(bl, SC_BEYONDOFWARCRY);
			status_change_end(bl, SC_UNLIMITEDHUMMINGVOICE);
			skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION);
		break;

	case SR_EARTHSHAKER:
		if( flag&1 ) { //by default cloaking skills are remove by aoe skills so no more checking/removing except hiding and cloaking exceed.
			skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			status_change_end(bl, SC_CLOAKINGEXCEED);
			if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM);
		} else {
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		}
		break;

	case SR_TIGERCANNON:
		if (flag & 1) {
			skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag | SD_ANIMATION);
		} else if (sd) {
			if (sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SR_FALLENEMPIRE && !sc->getSCE(SC_FLASHCOMBO))
				flag |= 8; // Only apply Combo bonus when Tiger Cannon is not used through Flash Combo
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR | BL_SKILL, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		}
		break;

	case WM_REVERBERATION:
		if (flag & 1)
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
			battle_consume_ammo(sd, skill_id, skill_lv); // Consume here since Magic/Misc attacks reset arrow_atk
		}
		break;

	case NPC_POISON_BUSTER:
		if( tsc && tsc->getSCE(SC_POISON) ) {
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
			status_change_end(bl, SC_POISON);
		}
		else if( sd )
			clif_skill_fail( *sd, skill_id );
		break;

	case KO_JYUMONJIKIRI: {
			short x, y;
			short dir = map_calc_dir(src,bl->x,bl->y);

			if (dir > 0 && dir < 4)
				x = 2;
			else if (dir > 4)
				x = -2;
			else
				x = 0;
			if (dir > 2 && dir < 6)
				y = 2;
			else if (dir == 7 || dir < 2)
				y = -2;
			else
				y = 0;
			if (unit_movepos(src,bl->x + x,bl->y + y,1,1)) {
				clif_blown(src);
				skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
			}
		}
		break;

	case EL_FIRE_BOMB:
	case EL_FIRE_WAVE:
	case EL_WATER_SCREW:
	case EL_HURRICANE:
	case EL_TYPOON_MIS:
		if( flag&1 )
			skill_attack(skill_get_type(skill_id+1),src,src,bl,skill_id+1,skill_lv,tick,flag);
		else {
			int i = skill_get_splash(skill_id,skill_lv);
			clif_skill_nodamage(src,*battle_get_master(src),skill_id,skill_lv);
			clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			if( rnd()%100 < 30 )
				map_foreachinrange(skill_area_sub,bl,i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
			else
				skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		}
		break;

	case EL_ROCK_CRUSHER:
		clif_skill_nodamage(src,*battle_get_master(src),skill_id,skill_lv);
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		if( rnd()%100 < 50 )
			skill_attack(BF_MAGIC,src,src,bl,skill_id,skill_lv,tick,flag);
		else
			skill_attack(BF_WEAPON,src,src,bl,EL_ROCK_CRUSHER_ATK,skill_lv,tick,flag);
		break;

	case EL_STONE_RAIN:
		if( flag&1 )
			skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		else {
			int i = skill_get_splash(skill_id,skill_lv);
			clif_skill_nodamage(src,*battle_get_master(src),skill_id,skill_lv);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			if( rnd()%100 < 30 )
				map_foreachinrange(skill_area_sub,bl,i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
			else
				skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		}
		break;

	case EL_FIRE_ARROW:
	case EL_ICE_NEEDLE:
	case EL_WIND_SLASH:
	case EL_STONE_HAMMER:
		clif_skill_nodamage(src,*battle_get_master(src),skill_id,skill_lv);
		clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		break;

	case EL_TIDAL_WEAPON:
		if( src->type == BL_ELEM ) {
			s_elemental_data *ele = BL_CAST(BL_ELEM,src);
			status_change *tsc_ele = status_get_sc(&ele->bl);
			sc_type type = SC_TIDAL_WEAPON_OPTION, type2 = SC_TIDAL_WEAPON;

			clif_skill_nodamage(src,*battle_get_master(src),skill_id,skill_lv);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			if( (tsc_ele && tsc_ele->getSCE(type2)) || (tsc && tsc->getSCE(type)) ) {
				status_change_end(battle_get_master(src),type);
				status_change_end(src,type2);
			}
			if( rnd()%100 < 50 )
				skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
			else {
				sc_start(src,src,type2,100,skill_lv,skill_get_time(skill_id,skill_lv));
				sc_start(src,battle_get_master(src),type,100,ele->bl.id,skill_get_time(skill_id,skill_lv));
			}
			clif_skill_nodamage(src,*src,skill_id,skill_lv);
		}
		break;

	//recursive homon skill
	case MH_ABSOLUTE_ZEPHYR:
	case MH_TOXIN_OF_MANDARA:
	case MH_BLAZING_AND_FURIOUS:
	case MH_MAGMA_FLOW:
	case MH_HEILIGE_STANGE:
		if(flag&1){
			if((skill_id == MH_MAGMA_FLOW) && ((rnd()%100)>(3*skill_lv)) )
				break;//chance to not trigger atk for magma
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		}
		else
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		break;

	case MH_TWISTER_CUTTER:
	case MH_GLANZEN_SPIES:
	case MH_STAHL_HORN:
	case MH_NEEDLE_OF_PARALYZE:
	case MH_NEEDLE_STINGER:
	case MH_SONIC_CRAW:
	case MH_MIDNIGHT_FRENZY:
	case MH_SILVERVEIN_RUSH:
		skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		break;
	case MH_TINDER_BREAKER:
	case MH_CBC:
	case MH_EQC:
		{
			TBL_HOM *hd = BL_CAST(BL_HOM,src);
			int duration = max(skill_lv, (status_get_str(src) / 7 - status_get_str(bl) / 10)) * 1000; //Yommy formula
			sc_type type;

			if( skill_id == MH_TINDER_BREAKER ){
				type = SC_TINDER_BREAKER2;
				if( unit_movepos(src, bl->x, bl->y, 1, 1) ){
					clif_blown(src);
					clif_skill_poseffect(src,skill_id,skill_lv,bl->x,bl->y,tick);
				}
			}else if( skill_id == MH_CBC ){
				type = SC_CBC;
			}else if( skill_id == MH_EQC ){
				type = SC_EQC;
			}

			clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start4(src,bl,type,100,skill_lv,src->id,0,0,duration));
			skill_attack(skill_get_type(skill_id),src,src,bl,skill_id,skill_lv,tick,flag);
		}
		break;

	case RL_H_MINE:
		if (!(flag&1)) {
			// Direct attack
			if (!sd || !sd->flicker) {
				if (skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag))
					status_change_start(src, bl, SC_H_MINE, 10000, skill_id, 0, 0, 0, skill_get_time(skill_id,skill_lv), SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
				break;
			}
			// Triggered by RL_FLICKER
			if (sd && sd->flicker) {
				// Splash damage around it!
				map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL,
					src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
				flag |= 1; // Don't consume requirement
				if (tsc &&tsc->getSCE(SC_H_MINE) && tsc->getSCE(SC_H_MINE)->val2 == src->id) {
					status_change_end(bl, SC_H_MINE);
					sc_start4(src, bl, SC_BURNING, 10 * skill_lv, skill_lv, 1000, src->id, 0, skill_get_time2(skill_id,skill_lv));
				}
			}
		}
		else
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		if (sd && sd->flicker)
			flag |= 1; // Don't consume requirement
		break;

	case RL_QD_SHOT:
		if (skill_area_temp[1] == bl->id)
			break;
		if (flag&1 && tsc && tsc->getSCE(SC_C_MARKER))
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION);
		break;
	case RL_D_TAIL:
	case RL_HAMMER_OF_GOD:
		if (flag&1)
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag|SD_ANIMATION);
		else {
			if (sd && tsc && tsc->getSCE(SC_C_MARKER)) {
				int i;

				ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == bl->id);

				if (i < MAX_SKILL_CRIMSON_MARKER)
					flag |= 8;
			}

			if (skill_id == RL_HAMMER_OF_GOD)
				clif_skill_poseffect(src, skill_id, 1, bl->x, bl->y, gettick());
			else
				clif_skill_nodamage(src, *bl, skill_id, skill_lv);

			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		}
		break;

	case SU_SCAROFTAROU:
		sc_start(src, bl, SC_BITESCAR, 10, skill_lv, skill_get_time(skill_id, skill_lv)); //! TODO: What's the activation chance for the Bite effect?
		skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		break;
	case SU_SV_STEMSPEAR:
		if (sd && pc_checkskill(sd, SU_SPIRITOFLAND))
			sc_start(src, src, SC_DORAM_WALKSPEED, 100, 50, skill_get_time(SU_SPIRITOFLAND, 1));
		skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case 0:/* no skill - basic/normal attack */
		if(sd) {
			if (flag & 3){
				if (bl->id != skill_area_temp[1])
					skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, SD_LEVEL|flag);
			} else {
				skill_area_temp[1] = bl->id;
				map_foreachinallrange(skill_area_sub, bl,
					sd->bonus.splash_range, BL_CHAR,
					src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1,
					skill_castend_damage_id);
				flag|=1; //Set flag to 1 so ammo is not double-consumed. [Skotlex]
			}
		}
		break;

	case SJ_FALLINGSTAR_ATK:
		if (sd) { // If a player used the skill it will search for targets marked by that player. 
			if (tsc && tsc->getSCE(SC_FLASHKICK) && tsc->getSCE(SC_FLASHKICK)->val4 == 1) { // Mark placed by a player.
				int8 i = 0;

				ARR_FIND(0, MAX_STELLAR_MARKS, i, sd->stellar_mark[i] == bl->id);
				if (i < MAX_STELLAR_MARKS) {
					skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
					skill_castend_damage_id(src, bl, SJ_FALLINGSTAR_ATK2, skill_lv, tick, 0);
				}
			}
		} else if ( tsc && tsc->getSCE(SC_FLASHKICK) && tsc->getSCE(SC_FLASHKICK)->val4 == 2 ) { // Mark placed by a monster.
			// If a monster used the skill it will search for targets marked by any monster since they can't track their own targets.
			skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag);
			skill_castend_damage_id(src, bl, SJ_FALLINGSTAR_ATK2, skill_lv, tick, 0);
		}
		break;
	case SJ_FLASHKICK: {
			map_session_data *tsd = BL_CAST(BL_PC, bl);
			struct mob_data *md = BL_CAST(BL_MOB, src), *tmd = BL_CAST(BL_MOB, bl);

			// Only players and monsters can be tagged....I think??? [Rytech]
			// Lets only allow players and monsters to use this skill for safety reasons.
			if ((!tsd && !tmd) || !sd && !md) {
				if (sd)
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
				break;
			}

			// Check if the target is already tagged by another source.
			if ((tsd && tsd->sc.getSCE(SC_FLASHKICK) && tsd->sc.getSCE(SC_FLASHKICK)->val1 != src->id) || (tmd && tmd->sc.getSCE(SC_FLASHKICK) && tmd->sc.getSCE(SC_FLASHKICK)->val1 != src->id)) { // Same as the above check, but for monsters.
				// Can't tag a player that was already tagged from another source.
				if (sd)
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
				map_freeblock_unlock();
				return 1;
			}

			if (sd) { // Tagging the target.
				int i;

				ARR_FIND(0, MAX_STELLAR_MARKS, i, sd->stellar_mark[i] == bl->id);
				if (i == MAX_STELLAR_MARKS) {
					ARR_FIND(0, MAX_STELLAR_MARKS, i, sd->stellar_mark[i] == 0);
					if (i == MAX_STELLAR_MARKS) { // Max number of targets tagged. Fail the skill.
						clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
						map_freeblock_unlock();
						return 1;
					}
				}

				// Tag the target only if damage was done. If it deals no damage, it counts as a miss and won't tag.
				// Note: Not sure if it works like this in official but you can't mark on something you can't
				// hit, right? For now well just use this logic until we can get a confirm on if it does this or not. [Rytech]
				if (skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag) > 0) { // Add the ID of the tagged target to the player's tag list and start the status on the target.
					sd->stellar_mark[i] = bl->id;

					// Val4 flags if the status was applied by a player or a monster.
					// This will be important for other skills that work together with this one.
					// 1 = Player, 2 = Monster.
					// Note: Because the attacker's ID and the slot number is handled here, we have to
					// apply the status here. We can't pass this data to skill_additional_effect.
					sc_start4(src, bl, SC_FLASHKICK, 100, src->id, i, skill_lv, 1, skill_get_time(skill_id, skill_lv));
				}
			} else if (md) { // Monsters can't track with this skill. Just give the status.
				if (skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag) > 0)
					sc_start4(src, bl, SC_FLASHKICK, 100, 0, 0, skill_lv, 2, skill_get_time(skill_id, skill_lv));
			}
		}
		break;

	case NPC_VENOMIMPRESS:
		if (skill_attack(BF_WEAPON, src, src, bl, skill_id, skill_lv, tick, flag))
			sc_start(src, bl, SC_VENOMIMPRESS, 100, skill_lv, skill_get_time(skill_id,skill_lv));
		break;

	case HN_DOUBLEBOWLINGBASH:
		if (flag & 1) {
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, skill_area_temp[0] & 0xFFF);
		} else {
			int splash = skill_get_splash(skill_id, skill_lv);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			skill_area_temp[0] = map_foreachinallrange(skill_area_sub, bl, splash, BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
			map_foreachinrange(skill_area_sub, bl, splash, BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
			sc_start(src, src, SC_HNNOWEAPON, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		}
		break;

	case HN_SHIELD_CHAIN_RUSH:
		if (flag & 1) {
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		} else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
			sc_start(src, src, SC_HNNOWEAPON, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		}
		break;

	case HN_JACK_FROST_NOVA:
	case HN_HELLS_DRIVE:
	case HN_GROUND_GRAVITATION:
		if (flag & 1)
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		break;

	case HN_NAPALM_VULCAN_STRIKE:
		if (flag & 1) {
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag);
		} else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		}
		break;

	case NW_MAGAZINE_FOR_ONE:
	case NW_ONLY_ONE_BULLET:
 		skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
		if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
			status_change_end(src, SC_INTENSIVE_AIM_COUNT);
 		break;

	default:
		ShowWarning("skill_castend_damage_id: Unknown skill used:%d\n",skill_id);
		clif_skill_damage(src, bl, tick, status_get_amotion(src), tstatus->dmotion,
			0, abs(skill_get_num(skill_id, skill_lv)),
			skill_id, skill_lv, skill_get_hit(skill_id));
		map_freeblock_unlock();
		return 1;
	}

	if( sc && sc->getSCE(SC_CURSEDCIRCLE_ATKER) ) //Should only remove after the skill has been casted.
		status_change_end(src,SC_CURSEDCIRCLE_ATKER);

	map_freeblock_unlock();

	if( sd && !(flag&1) )
	{// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = gettick();

		if( sd->state.arrow_atk )
		{// consume arrow on last invocation to this skill.
			battle_consume_ammo(sd, skill_id, skill_lv);
		}

		// perform skill requirement consumption
		if (!(flag&SKILL_NOCONSUME_REQ))
			skill_consume_requirement(sd,skill_id,skill_lv,2);
	}

	return 0;
}

/**
 * Give a song's buff/debuff or damage to all targets around
 * @param target: Target
 * @param ap: Argument list
 * @return 1 on success or 0 otherwise
 */
static int skill_apply_songs(struct block_list* target, va_list ap)
{
	int flag = va_arg(ap, int);
	struct block_list* src = va_arg(ap, struct block_list*);
	uint16 skill_id = static_cast<uint16>(va_arg(ap, int));
	uint16 skill_lv = static_cast<uint16>(va_arg(ap, int));
	t_tick tick = va_arg(ap, t_tick);

	if (flag & BCT_WOS && src == target)
		return 0;

	if (battle_check_target(src, target, flag) > 0) {
		switch (skill_id) {
			// Attack type songs
		case BA_DISSONANCE:
			skill_attack(BF_MAGIC, src, src, target, skill_id, skill_lv, tick, 0);
			return 1;
		case DC_UGLYDANCE:
		case BD_LULLABY:
			return skill_additional_effect(src, target, skill_id, skill_lv, BF_LONG | BF_SKILL | BF_MISC, ATK_DEF, tick);
		default: // Buff/Debuff type songs
			if (skill_id == CG_HERMODE && src->id != target->id)
				status_change_clear_buffs(target, SCCB_HERMODE); // Should dispell only allies.
			return sc_start(src, target, skill_get_sc(skill_id), 100, skill_lv, skill_get_time(skill_id, skill_lv));
		}
	}

	return 0;
}

/**
 * Calculate a song's bonus values
 * @param src: Caster
 * @param skill_id: Song skill ID
 * @param skill_lv: Song skill level
 * @param tick: Timer tick
 * @return Number of targets or 0 otherwise
 */
static int skill_castend_song(struct block_list* src, uint16 skill_id, uint16 skill_lv, t_tick tick)
{
	nullpo_ret(src);

	if (src->type != BL_PC) {
		ShowWarning("skill_castend_song: Expected player type for src!\n");
		return 0;
	}

	if (!(skill_get_inf2_(skill_id, { INF2_ISSONG, INF2_ISENSEMBLE }))) {
		ShowWarning("skill_castend_song: Unknown song skill ID: %u\n", skill_id);
		return 0;
	}

	map_session_data* sd = BL_CAST(BL_PC, src);
	int flag = BCT_PARTY;

	switch (skill_id) {
		case BD_ROKISWEIL:
			flag = BCT_ENEMY | BCT_WOS;
			break;
		case BD_LULLABY:
		case BD_ETERNALCHAOS:
		case BA_DISSONANCE:
		case DC_UGLYDANCE:
		case DC_DONTFORGETME:
			flag = BCT_ENEMY;
			break;
		case CG_HERMODE:
			flag |= BCT_GUILD;
			break;
	}

	clif_skill_nodamage(src, *src, skill_id, skill_lv);
	sd->skill_id_dance = skill_id;
	sd->skill_lv_dance = skill_lv;

	if (skill_get_inf2(skill_id, INF2_ISENSEMBLE))
		skill_check_pc_partner(sd, skill_id, &skill_lv, 3, 1);

	return map_foreachinrange(skill_apply_songs, src, skill_get_splash(skill_id, skill_lv), splash_target(src), flag, src, skill_id, skill_lv, tick);
}

/**
 * Use no-damage skill from 'src' to 'bl
 * @param src Caster
 * @param bl Target of the skill, bl maybe same with src for self skill
 * @param skill_id
 * @param skill_lv
 * @param tick
 * @param flag Various value, &1: Recursive effect
 **/
int skill_castend_nodamage_id (struct block_list *src, struct block_list *bl, uint16 skill_id, uint16 skill_lv, t_tick tick, int flag)
{
	map_session_data *sd, *dstsd;
	struct mob_data *md, *dstmd;
	struct homun_data *hd;
	s_mercenary_data *mer;
	status_change *tsc;
	struct status_change_entry *tsce;

	int i = 0;
	enum sc_type type;

	if(skill_id > 0 && !skill_lv) return 0;	// celest

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

	if(bl->prev == nullptr)
		return 1;
	if(status_isdead(*src))
		return 1;

	if( src != bl && status_isdead(*bl) ) {
		switch( skill_id ) { // Skills that may be cast on dead targets
			case NPC_WIDESOULDRAIN:
			case PR_REDEMPTIO:
			case ALL_RESURRECTION:
			case WM_DEADHILLHERE:
			case WE_ONEFOREVER:
				break;
			default:
				return 1;
		}
	}

	status_data* tstatus = status_get_status_data(*bl);
	status_data* sstatus = status_get_status_data(*src);

	//Check for undead skills that convert a no-damage skill into a damage one. [Skotlex]
	switch (skill_id) {
 		case AL_HEAL:
		case ALL_RESURRECTION:
		case PR_ASPERSIO:
		case AB_HIGHNESSHEAL:
			//Apparently only player casted skills can be offensive like this.
			if (sd && battle_check_undead(tstatus->race,tstatus->def_ele)) {
				if (battle_check_target(src, bl, BCT_ENEMY) < 1) {
					//Offensive heal does not works on non-enemies. [Skotlex]
					clif_skill_fail( *sd, skill_id );
					return 0;
				}
				return skill_castend_damage_id (src, bl, skill_id, skill_lv, tick, flag);
			}
			break;
		case NPC_SMOKING: //Since it is a self skill, this one ends here rather than in damage_id. [Skotlex]
			return skill_castend_damage_id (src, bl, skill_id, skill_lv, tick, flag);
		case MH_STEINWAND: {
			struct block_list *s_src = battle_get_master(src);
			short ret = 0;
			if(!skill_check_unit_range(src, src->x, src->y, skill_id, skill_lv))  //prevent reiteration
			    ret = skill_castend_pos2(src,src->x,src->y,skill_id,skill_lv,tick,flag); //cast on homon
			if(s_src && !skill_check_unit_range(s_src, s_src->x, s_src->y, skill_id, skill_lv))
			    ret |= skill_castend_pos2(s_src,s_src->x,s_src->y,skill_id,skill_lv,tick,flag); //cast on master
			return ret;
		    }
		    break;
		default:
			//Skill is actually ground placed.
			if (src == bl && skill_get_unit_id(skill_id))
				return skill_castend_pos2(src,bl->x,bl->y,skill_id,skill_lv,tick,0);
	}

	type = skill_get_sc(skill_id);
	tsc = status_get_sc(bl);
	status_change* sc = status_get_sc(src);
	tsce = (tsc && type != SC_NONE)?tsc->getSCE(type):nullptr;

	if (src!=bl && type > SC_NONE &&
		CHK_ELEMENT((i = skill_get_ele(skill_id, skill_lv))) && i > ELE_NEUTRAL &&
		skill_get_inf(skill_id) != INF_SUPPORT_SKILL &&
		battle_attr_fix(nullptr, nullptr, 100, i, tstatus->def_ele, tstatus->ele_lv) <= 0)
		return 1; //Skills that cause an status should be blocked if the target element blocks its element.

	map_freeblock_lock();
	switch(skill_id)
	{
	case HLIF_HEAL:	//[orn]
	case AL_HEAL:
	case AB_HIGHNESSHEAL:
		{
			int heal = skill_calc_heal(src, bl, skill_id, skill_lv, true);

			if (status_isimmune(bl) || (dstmd && (status_get_class(bl) == MOBID_EMPERIUM || status_get_class_(bl) == CLASS_BATTLEFIELD)))
				heal = 0;

			if( tsc && tsc->count ) {
				if( tsc->getSCE(SC_KAITE) && !status_has_mode(sstatus,MD_STATUSIMMUNE) ) { //Bounce back heal
					if (--tsc->getSCE(SC_KAITE)->val2 <= 0)
						status_change_end(bl, SC_KAITE);
					if (src == bl)
						heal=0; //When you try to heal yourself under Kaite, the heal is voided.
					else {
						bl = src;
						dstsd = sd;
					}
				}
				else if (tsc->getSCE(SC_BERSERK) || tsc->getSCE(SC_SATURDAYNIGHTFEVER))
					heal = 0; //Needed so that it actually displays 0 when healing.
			}
			if (skill_id == AL_HEAL)
				status_change_end(bl, SC_BITESCAR);
			clif_skill_nodamage(src, *bl, skill_id, heal);
			if( tsc && tsc->getSCE(SC_AKAITSUKI) && heal && skill_id != HLIF_HEAL )
				heal = ~heal + 1;
			t_exp heal_get_jobexp = status_heal(bl,heal,0,0);

			if(sd && dstsd && heal > 0 && sd != dstsd && battle_config.heal_exp > 0){
				heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
				if (heal_get_jobexp <= 0)
					heal_get_jobexp = 1;
				pc_gainexp (sd, bl, 0, heal_get_jobexp, 0);
			}
		}
		break;

	case CD_REPARATIO: {
		if (bl->type != BL_PC) { // Only works on players.
			if (sd)
				clif_skill_fail( *sd, skill_id );
			break;
		}

		int heal_amount = 0;

		if (!status_isimmune(bl))
			heal_amount = tstatus->max_hp;

		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		clif_skill_nodamage(nullptr, *bl, AL_HEAL, heal_amount);
		status_heal(bl, heal_amount, 0, 0);
	}
	break;

	case PR_REDEMPTIO:
		if (sd && !(flag&1)) {
			if (sd->status.party_id == 0) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
			skill_area_temp[0] = 0;
			party_foreachsamemap(skill_area_sub,
				sd,skill_get_splash(skill_id, skill_lv),
				src,skill_id,skill_lv,tick, flag|BCT_PARTY|1,
				skill_castend_nodamage_id);
			if (skill_area_temp[0] == 0) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
#ifndef RENEWAL
			skill_area_temp[0] = battle_config.exp_cost_redemptio_limit - skill_area_temp[0]; // The actual penalty...
			if (skill_area_temp[0] > 0 && !map_getmapflag(src->m, MF_NOEXPPENALTY) && battle_config.exp_cost_redemptio) { //Apply penalty
				//If total penalty is 1% => reduced 0.2% penalty per each revived player
				pc_lostexp(sd, u64min(sd->status.base_exp, (pc_nextbaseexp(sd) * skill_area_temp[0] * battle_config.exp_cost_redemptio / battle_config.exp_cost_redemptio_limit) / 100), 0);
			}
			status_set_sp(src, 0, 0);
#endif
			status_set_hp(src, 1, 0);
			break;
		} else if (!(status_isdead(*bl) && flag&1)) { 
			//Invalid target, skip resurrection.
			break;
		}
		//Revive
		skill_area_temp[0]++; //Count it in, then fall-through to the Resurrection code.
		skill_lv = 3; //Resurrection level 3 is used
		[[fallthrough]];

	case ALL_RESURRECTION:
		if(sd && (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND)))
		{	//No reviving in WoE grounds!
			clif_skill_fail( *sd, skill_id );
			break;
		}
		if (!status_isdead(*bl))
			break;
		{
			int per = 0, sper = 0;
			if (tsc && tsc->getSCE(SC_HELLPOWER)) {
				clif_skill_nodamage(src, *bl, ALL_RESURRECTION, skill_lv);
				break;
			}

			if (map_getmapflag(bl->m, MF_PVP) && dstsd && dstsd->pvp_point < 0)
				break;

			switch(skill_lv){
			case 1: per=10; break;
			case 2: per=30; break;
			case 3: per=50; break;
			case 4: per=80; break;
			}
			if(dstsd && dstsd->special_state.restart_full_recover)
				per = sper = 100;
			if (status_revive(bl, per, sper))
			{
				clif_skill_nodamage(src,*bl,ALL_RESURRECTION,skill_lv); //Both Redemptio and Res show this skill-animation.
				if(sd && dstsd && battle_config.resurrection_exp > 0)
				{
					t_exp exp = 0,jexp = 0;
					int lv = dstsd->status.base_level - sd->status.base_level, jlv = dstsd->status.job_level - sd->status.job_level;
					if(lv > 0 && pc_nextbaseexp(dstsd)) {
						exp = (t_exp)(dstsd->status.base_exp * lv * battle_config.resurrection_exp / 1000000.);
						if (exp < 1) exp = 1;
					}
					if(jlv > 0 && pc_nextjobexp(dstsd)) {
						jexp = (t_exp)(dstsd->status.job_exp * lv * battle_config.resurrection_exp / 1000000.);
						if (jexp < 1) jexp = 1;
					}
					if(exp > 0 || jexp > 0)
						pc_gainexp (sd, bl, exp, jexp, 0);
				}
			}
		}
		break;

	case AL_DECAGI:
	case MER_DECAGI:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv,
			sc_start(src,bl, type, (50 + skill_lv * 3 + (status_get_lv(src) + sstatus->int_)/5), skill_lv, skill_get_time(skill_id,skill_lv)));
		break;

	case AL_CRUCIS:
		if (flag&1)
			sc_start(src,bl,type, 23+skill_lv*4 +status_get_lv(src) -status_get_lv(bl), skill_lv,skill_get_time(skill_id,skill_lv));
		else {
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR,
				src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case SP_SOULCURSE:
		if (flag&1)
			sc_start(src, bl, type, 30 + 10 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		else {
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case PR_LEXDIVINA:
	case MER_LEXDIVINA:
		if (tsce)
			status_change_end(bl, type);
		else
			skill_addtimerskill(src, tick+1000, bl->id, 0, 0, skill_id, skill_lv, 100, flag);
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		break;

	case SA_ABRACADABRA:
		if (abra_db.empty()) {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			break;
		}
		else {
			int abra_skill_id = 0, abra_skill_lv;
			size_t checked = 0, checked_max = abra_db.size() * 3;

			do {
				auto abra_spell = abra_db.random();

				abra_skill_id = abra_spell->skill_id;
				abra_skill_lv = min(skill_lv, skill_get_max(abra_skill_id));

				if( rnd() % 10000 < abra_spell->per[max(skill_lv - 1, 0)] ){
					break;
				}
			} while (checked++ < checked_max);

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);

			if( sd )
			{// player-casted
				sd->state.abra_flag = 1;
				sd->skillitem = abra_skill_id;
				sd->skillitemlv = abra_skill_lv;
				sd->skillitem_keep_requirement = false;
				clif_item_skill(sd, abra_skill_id, abra_skill_lv);
			}
			else
			{// mob-casted
				struct unit_data *ud = unit_bl2ud(src);
				int inf = skill_get_inf(abra_skill_id);
				if (!ud) break;
				if (inf&INF_SELF_SKILL || inf&INF_SUPPORT_SKILL) {
					if (src->type == BL_PET)
						bl = (struct block_list*)((TBL_PET*)src)->master;
					if (!bl) bl = src;
					unit_skilluse_id(src, bl->id, abra_skill_id, abra_skill_lv);
				} else {	//Assume offensive skills
					int target_id = 0;
					if (ud->target)
						target_id = ud->target;
					else switch (src->type) {
						case BL_MOB: target_id = ((TBL_MOB*)src)->target_id; break;
						case BL_PET: target_id = ((TBL_PET*)src)->target_id; break;
					}
					if (!target_id)
						break;
					if (skill_get_casttype(abra_skill_id) == CAST_GROUND) {
						bl = map_id2bl(target_id);
						if (!bl) bl = src;
						unit_skilluse_pos(src, bl->x, bl->y, abra_skill_id, abra_skill_lv);
					} else
						unit_skilluse_id(src, target_id, abra_skill_id, abra_skill_lv);
				}
			}
		}
		break;

	case SA_COMA:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time2(skill_id,skill_lv)));
		break;
	case SA_FULLRECOVERY:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
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
			clif_skill_nodamage(nullptr, *bl, AL_HEAL, heal);
			if( dstmd )
			{ // Reset Damage Logs
				memset(dstmd->dmglog, 0, sizeof(dstmd->dmglog));
				dstmd->tdmg = 0;
			}
		}
		break;
	case SA_SUMMONMONSTER:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if (sd)
			mob_once_spawn(sd, src->m, src->x, src->y,"--ja--", -1, 1, "", SZ_SMALL, AI_NONE);
		break;
	case SA_LEVELUP:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if (sd && pc_nextbaseexp(sd))
			pc_gainexp(sd, nullptr, pc_nextbaseexp(sd) * 10 / 100, 0, 0);
		break;
	case SA_INSTANTDEATH:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		status_kill(src);
		break;
	case SA_QUESTION:
		clif_emotion(src,ET_QUESTION);
		[[fallthrough]];
	case SA_GRAVITY:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case SA_CLASSCHANGE:
	case SA_MONOCELL:
		if (dstmd)
		{
			int class_;

			if ( sd && status_has_mode(&dstmd->status,MD_STATUSIMMUNE) ) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
			class_ = (skill_id == SA_MONOCELL ? MOBID_PORING : mob_get_random_id(MOBG_CLASSCHANGE, RMF_DB_RATE, 0));
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			mob_class_change(dstmd,class_);
			if( tsc && status_has_mode(&dstmd->status,MD_STATUSIMMUNE) ) {
				const enum sc_type scs[] = { SC_QUAGMIRE, SC_PROVOKE, SC_ROKISWEIL, SC_GRAVITATION, SC_SUITON, SC_STRIPWEAPON, SC_STRIPSHIELD, SC_STRIPARMOR, SC_STRIPHELM, SC_BLADESTOP };
				for (i = SC_COMMON_MIN; i <= SC_COMMON_MAX; i++)
					if (tsc->getSCE(i)) status_change_end(bl, (sc_type)i);
				for (i = 0; i < ARRAYLENGTH(scs); i++)
					if (tsc->getSCE(scs[i])) status_change_end(bl, scs[i]);
			}
		}
		break;
	case SA_DEATH:
		if ( sd && dstmd && status_has_mode(&dstmd->status,MD_STATUSIMMUNE) ) {
			clif_skill_fail( *sd, skill_id );
			break;
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		status_kill(bl);
		break;
	case SA_REVERSEORCISH:
	case ALL_REVERSEORCISH:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id, skill_lv)));
		break;
	case SA_FORTUNE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if(sd) pc_getzeny(sd,status_get_lv(bl)*100,LOG_TYPE_STEAL);
		break;
	case SA_TAMINGMONSTER:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if (sd && dstmd && pet_db.find(dstmd->mob_id)) {
			pet_catch_process1(sd, dstmd->mob_id);
		}
		break;

	case CR_PROVIDENCE:
		if(sd && dstsd){ //Check they are not another crusader [Skotlex]
			if ((dstsd->class_&MAPID_UPPERMASK) == MAPID_CRUSADER) {
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 1;
			}
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;

	case CG_MARIONETTE:
		{
			if( (sd && dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER && dstsd->status.sex == sd->status.sex) || (tsc && (tsc->getSCE(SC_CURSE) || tsc->getSCE(SC_QUAGMIRE))) )
			{// Cannot cast on another bard/dancer-type class of the same gender as caster, or targets under Curse/Quagmire
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 1;
			}

			if( sc && tsc )
			{
				if( !sc->getSCE(SC_MARIONETTE) && !tsc->getSCE(SC_MARIONETTE2) )
				{
					sc_start(src,src,SC_MARIONETTE,100,bl->id,skill_get_time(skill_id,skill_lv));
					sc_start(src,bl,SC_MARIONETTE2,100,src->id,skill_get_time(skill_id,skill_lv));
					clif_skill_nodamage(src,*bl,skill_id,skill_lv);
				}
				else
				if(  sc->getSCE(SC_MARIONETTE ) &&  sc->getSCE(SC_MARIONETTE )->val1 == bl->id &&
					tsc->getSCE(SC_MARIONETTE2) && tsc->getSCE(SC_MARIONETTE2)->val1 == src->id )
				{
					status_change_end(src, SC_MARIONETTE);
					status_change_end(bl, SC_MARIONETTE2);
				}
				else
				{
					if( sd )
						clif_skill_fail( *sd, skill_id );

					map_freeblock_unlock();
					return 1;
				}
			}
		}
		break;

	case RG_CLOSECONFINE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start4(src,bl,type,100,skill_lv,src->id,0,0,skill_get_time(skill_id,skill_lv)));
		break;
	case SA_FLAMELAUNCHER:	// added failure chance and chance to break weapon if turned on [Valaris]
	case SA_FROSTWEAPON:
	case SA_LIGHTNINGLOADER:
	case SA_SEISMICWEAPON:
		if (dstsd && dstsd->status.weapon == W_FIST) {
			if (sd)
				clif_skill_fail( *sd, skill_id );
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,false);
			break;
		}
#ifdef RENEWAL
		clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
#else
		// 100% success rate at lv4 & 5, but lasts longer at lv5
		if(!clif_skill_nodamage(src,*bl,skill_id,skill_lv, sc_start(src,bl,type,(60+skill_lv*10),skill_lv, skill_get_time(skill_id,skill_lv)))) {
			if (dstsd){
				short index = dstsd->equip_index[EQI_HAND_R];
				if (index != -1 && dstsd->inventory_data[index] && dstsd->inventory_data[index]->type == IT_WEAPON)
					pc_unequipitem(dstsd, index, 3); //Must unequip the weapon instead of breaking it [Daegaladh]
			}
			if (sd)
				clif_skill_fail( *sd, skill_id );
		}
#endif
		break;

	case PR_ASPERSIO:
		if (sd && dstmd) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,false);
			break;
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;

	case ITEM_ENCHANTARMS:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_get_ele(skill_id, skill_lv), skill_get_time(skill_id, skill_lv)));
		break;

	case TK_SEVENWIND:
		switch(skill_get_ele(skill_id,skill_lv)) {
			case ELE_EARTH : type = SC_EARTHWEAPON;  break;
			case ELE_WIND  : type = SC_WINDWEAPON;   break;
			case ELE_WATER : type = SC_WATERWEAPON;  break;
			case ELE_FIRE  : type = SC_FIREWEAPON;   break;
			case ELE_GHOST : type = SC_GHOSTWEAPON;  break;
			case ELE_DARK  : type = SC_SHADOWWEAPON; break;
			case ELE_HOLY  : type = SC_ASPERSIO;     break;
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));

		sc_start(src,bl,SC_SEVENWIND,100,skill_lv,skill_get_time(skill_id,skill_lv));

		break;

	case NPC_MOVE_COORDINATE:
		{
			int16 px = bl->x, py = bl->y;
			if (!skill_check_unit_movepos(0, bl, src->x, src->y, 1, 1)) {
				return 0;
			}

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			clif_blown(bl);

			// If caster is not a boss, switch coordinates with the target
			if (status_get_class_(src) != CLASS_BOSS) {
				if (!skill_check_unit_movepos(0, src, px, py, 1, 1)) {
					return 0;
				}

				clif_blown(src);
			}
		}
		break;

	case NPC_IMMUNE_PROPERTY:
		switch (skill_lv) {
			case 1: type = SC_IMMUNE_PROPERTY_NOTHING; break;
			case 2: type = SC_IMMUNE_PROPERTY_WATER; break;
			case 3: type = SC_IMMUNE_PROPERTY_GROUND; break;
			case 4: type = SC_IMMUNE_PROPERTY_FIRE; break;
			case 5: type = SC_IMMUNE_PROPERTY_WIND; break;
			case 6: type = SC_IMMUNE_PROPERTY_DARKNESS; break;
			case 7: type = SC_IMMUNE_PROPERTY_SAINT; break;
			case 8: type = SC_IMMUNE_PROPERTY_POISON; break;
			case 9: type = SC_IMMUNE_PROPERTY_TELEKINESIS; break;
			case 10: type = SC_IMMUNE_PROPERTY_UNDEAD; break;
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;

	case PR_KYRIE:
	case MER_KYRIE:
	case SU_TUNAPARTY:
	case SU_GROOMING:
	case SU_CHATTERING:
	case ALL_RAY_OF_PROTECTION:
		clif_skill_nodamage(bl,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;
	//Passive Magnum, should had been casted on yourself.
	case SM_MAGNUM:
	case MS_MAGNUM:
		skill_area_temp[1] = 0;
		map_foreachinshootrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_SKILL|BL_CHAR,
			src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
		clif_skill_nodamage(src, *src,skill_id,skill_lv);
		// Initiate 20% of your damage becomes fire element.
#ifdef RENEWAL
		sc_start4(src,src,SC_SUB_WEAPONPROPERTY,100,ELE_FIRE,20,skill_id,0,skill_get_time2(skill_id, skill_lv));
#else
		sc_start4(src,src,SC_WATK_ELEMENT,100,ELE_FIRE,20,0,0,skill_get_time2(skill_id, skill_lv));
#endif
		break;

	case MH_BLAZING_AND_FURIOUS:
	case TK_JUMPKICK:
		/* Check if the target is an enemy; if not, skill should fail so the character doesn't unit_movepos (exploitable) */
		if( battle_check_target(src, bl, BCT_ENEMY) > 0 ) {
			if( unit_movepos(src, bl->x, bl->y, 2, 1) ) {
				skill_attack(BF_WEAPON,src,src,bl,skill_id,skill_lv,tick,flag);
				clif_blown(src);
			}
		}else if( sd ){
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
		}
		break;

	case PR_BENEDICTIO:
		if (!battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race != RC_DEMON)
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		break;
	case AL_INCAGI:
	case AL_BLESSING:
	case MER_INCAGI:
	case MER_BLESSING:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		if (dstsd != nullptr && tsc && tsc->getSCE(SC_CHANGEUNDEAD)) {
			if (tstatus->hp > 1)
				skill_attack(BF_MISC,src,src,bl,skill_id,skill_lv,tick,flag);
			break;
		}
		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case CR_REFLECTSHIELD:
	case MS_REFLECTSHIELD:
		if (tsc && tsc->getSCE(SC_DARKCROW)) { // SC_DARKCROW prevents using reflecting skills
			if (sd)
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
			break;
		}
		[[fallthrough]];
	case PR_SLOWPOISON:
	case PR_LEXAETERNA:
#ifndef RENEWAL
	case PR_IMPOSITIO:
	case PR_SUFFRAGIUM:
#endif
	case LK_BERSERK:
	case MS_BERSERK:
	case KN_TWOHANDQUICKEN:
	case KN_ONEHAND:
	case MER_QUICKEN:
	case CR_SPEARQUICKEN:
	case AS_POISONREACT:
#ifndef RENEWAL
	case MC_LOUD:
#endif
	case MG_ENERGYCOAT:
	case MO_EXPLOSIONSPIRITS:
	case MO_STEELBODY:
	case MO_BLADESTOP:
	case LK_AURABLADE:
	case LK_PARRYING:
	case MS_PARRYING:
	case LK_CONCENTRATION:
#ifdef RENEWAL
	case HP_BASILICA:
#endif
	case WS_CARTBOOST:
	case SN_SIGHT:
	case WS_MELTDOWN:
	case WS_OVERTHRUSTMAX:
	case ST_REJECTSWORD:
	case HW_MAGICPOWER:
	case PF_MEMORIZE:
	case PA_SACRIFICE:
	case PF_DOUBLECASTING:
	case SG_SUN_COMFORT:
	case SG_MOON_COMFORT:
	case SG_STAR_COMFORT:
	case GS_MADNESSCANCEL:
	case GS_ADJUSTMENT:
	case GS_INCREASING:
#ifdef RENEWAL
	case GS_MAGICALBULLET:
#endif
	case NJ_KASUMIKIRI:
	case NJ_UTSUSEMI:
	case NJ_NEN:
	case NPC_DEFENDER:
	case NPC_MAGICMIRROR:
	case ST_PRESERVE:
	case NPC_KEEPING:
	case NPC_WEAPONBRAKER:
	case NPC_BARRIER:
	case NPC_INVINCIBLE:
	case RK_DEATHBOUND:
	case AB_EXPIATIO:
	case AB_DUPLELIGHT:
	case AB_SECRAMENT:
	case AB_OFFERTORIUM:
	case NC_ACCELERATION:
	case NC_HOVERING:
	case NC_SHAPESHIFT:
	case WL_MARSHOFABYSS:
	case WL_RECOGNIZEDSPELL:
	case GC_VENOMIMPRESS:
	case SC_DEADLYINFECT:
	case LG_EXEEDBREAK:
	case LG_PRESTIGE:
	case LG_INSPIRATION:
	case SR_CRESCENTELBOW:
	case SR_LIGHTNINGWALK:
	case GN_CARTBOOST:
	case GN_BLOOD_SUCKER:
	case GN_HELLS_PLANT:
	case KO_MEIKYOUSISUI:
	case ALL_ODINS_POWER:
	case ALL_FULL_THROTTLE:
	case RA_UNLIMIT:
	case WL_TELEKINESIS_INTENSE:
	case RL_HEAT_BARREL:
	case RL_P_ALTER:
	case RL_E_CHAIN:
	case SU_FRESHSHRIMP:
	case SU_ARCLOUSEDASH:
	case NPC_MAXPAIN:
	case NPC_KILLING_AURA:
	case SP_SOULREAPER:
	case SJ_LIGHTOFMOON:
	case SJ_LIGHTOFSTAR:
	case SJ_FALLINGSTAR:
	case SJ_LIGHTOFSUN:
	case SJ_BOOKOFDIMENSION:
	case NPC_HALLUCINATIONWALK:
	case DK_CHARGINGPIERCE:
	case DK_VIGOR:
	case AG_CLIMAX:
	case IQ_POWERFUL_FAITH:
	case IQ_FIRM_FAITH:
	case IQ_SINCERE_FAITH:
	case IQ_FIRST_FAITH_POWER:
	case IQ_JUDGE:
	case IQ_THIRD_EXOR_FLAME:
	case IG_REBOUND_SHIELD:
	case IG_HOLY_SHIELD:
	case CD_ARGUTUS_VITA:
	case CD_ARGUTUS_TELUM:
	case CD_PRESENS_ACIES:
	case CD_RELIGIO:
	case CD_BENEDICTUM:
	case SHC_SHADOW_EXCEED:
	case SHC_POTENT_VENOM:
	case SHC_ENCHANTING_SHADOW:
	case MT_D_MACHINE:
	case ABC_ABYSS_SLAYER:
	case WH_WIND_SIGN:
	case WH_CALAMITYGALE:
	case BO_RESEARCHREPORT:
	case TR_MYSTIC_SYMPHONY:
	case TR_KVASIR_SONATA:
	case EM_SPELL_ENCHANTING:
	case NPC_DAMAGE_HEAL:
	case NPC_RELIEVE_ON:
	case NPC_RELIEVE_OFF:
	case HN_BREAKINGLIMIT:
	case HN_RULEBREAK:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;

	case NPC_GRADUAL_GRAVITY:
	case NPC_DEADLYCURSE:
		status_change_start(src, bl, type, 10000, skill_lv, 0, 0, 0, skill_get_time(skill_id, skill_lv), SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		break;

	case NPC_ALL_STAT_DOWN: 
		status_change_start(src, bl, type, 10000, skill_lv, 0, 0, 0, skill_get_time(skill_id, skill_lv), SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		break;

	// EDP also give +25% WATK poison pseudo element to user.
	case ASC_EDP:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
#ifdef RENEWAL
		sc_start4(src, src, SC_SUB_WEAPONPROPERTY, 100, ELE_POISON, 25, skill_id, 0, skill_get_time(skill_id, skill_lv));
#else
		sc_start4(src, src, SC_WATK_ELEMENT, 100, ELE_POISON, 25, 0, 0, skill_get_time(skill_id, skill_lv));
#endif
		break;

	case LG_SHIELDSPELL:
		if (skill_lv == 1)
			type = SC_SHIELDSPELL_HP;
		else if (skill_lv == 2)
			type = SC_SHIELDSPELL_SP;
		else
			type = SC_SHIELDSPELL_ATK;

		clif_skill_nodamage(src, *bl, skill_id, skill_lv,
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		break;

	case DK_SERVANTWEAPON:
	case ABC_FROM_THE_ABYSS:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start2(src, bl, type, 100, skill_lv, src->id, skill_get_time(skill_id, skill_lv)));
		break;

	case TR_SOUNDBLEND:
		skill_castend_damage_id(src, bl, skill_id, skill_lv, tick, 0);
		clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start2(src, bl, type, 100, skill_lv, src->id, skill_get_time(skill_id, skill_lv)));
		break;

	case AG_VIOLENT_QUAKE:
	case AG_ALL_BLOOM:
		sc_start(src, bl, type, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		break;

	case AG_DESTRUCTIVE_HURRICANE:
	case AG_CRYSTAL_IMPACT:
		if (flag&1) { // Buff from Crystal Impact with level 1 Climax.
			sc_start(src, bl, type, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
		} else {
			uint16 climax_lv = 0, splash_size = skill_get_splash(skill_id, skill_lv);

			if (sc && sc->getSCE(SC_CLIMAX))
				climax_lv = sc->getSCE(SC_CLIMAX)->val1;

			if (climax_lv == 5) { // Adjusts splash AoE size depending on skill.
				if (skill_id == AG_DESTRUCTIVE_HURRICANE)
					splash_size = 9; // 19x19
				else if(skill_id == AG_CRYSTAL_IMPACT)
					splash_size = 7; // 15x15
			}

			skill_area_temp[1] = 0;
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);

			if (skill_id == AG_DESTRUCTIVE_HURRICANE && climax_lv == 4) // Buff for caster instead of damage AoE.
				sc_start(src, bl, type, 100, skill_lv, skill_get_time2(skill_id, skill_lv));
			else if (skill_id == AG_CRYSTAL_IMPACT && climax_lv == 1) // Buffs the caster and allies instead of doing damage AoE.
					map_foreachinrange(skill_area_sub, bl, splash_size, BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ALLY|SD_SPLASH|1, skill_castend_nodamage_id);
			else {
				if (skill_id == AG_DESTRUCTIVE_HURRICANE && climax_lv == 1) // Display extra animation for the additional hit cast.
					clif_skill_nodamage(src, *bl, AG_DESTRUCTIVE_HURRICANE_CLIMAX, skill_lv);

				map_foreachinrange(skill_area_sub, bl, splash_size, BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
			}
		}
		break;

	case CD_MEDIALE_VOTUM:
	case CD_DILECTIO_HEAL:
		if (flag & 1) {
			if (sd == nullptr || sd->status.party_id == 0 || (flag & 2)) {
				int heal_amount = skill_calc_heal(src, bl, skill_id, skill_lv, 1);

				clif_skill_nodamage(nullptr, *bl, AL_HEAL, heal_amount);
				status_heal(bl, heal_amount, 0, 0);
			} else if (sd)
				party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag | BCT_PARTY | 3, skill_castend_nodamage_id);
		} else {
			if (skill_id == CD_MEDIALE_VOTUM)
				clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
			else { // Dilectio Heal
				clif_skill_nodamage(src, *bl, skill_id, skill_lv); // Placed here to display animation on target only.
				skill_castend_nodamage_id(bl, bl, skill_id, skill_lv, tick, 1);
			}
		}
		break;

	case CD_COMPETENTIA:
		if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
			int hp_amount = tstatus->max_hp * (20 * skill_lv) / 100;
			int sp_amount = tstatus->max_sp * (20 * skill_lv) / 100;

			clif_skill_nodamage(nullptr, *bl, AL_HEAL, hp_amount);
			status_heal(bl, hp_amount, 0, 0);

			clif_skill_nodamage(nullptr, *bl, MG_SRECOVERY, sp_amount);
			status_heal(bl, 0, sp_amount, 0);

			clif_skill_nodamage(bl, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		} else if (sd)
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case BO_ADVANCE_PROTECTION:
		if( sd && ( !dstsd || pc_checkequip( dstsd, EQP_SHADOW_GEAR ) < 0 ) ){
			clif_skill_fail( *sd, skill_id );
			map_freeblock_unlock(); // Don't consume item requirements
			return 0;
		}
		clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start( src, bl, type, 100, skill_lv, skill_get_time( skill_id, skill_lv ) ) );
		break;

	case EM_ACTIVITY_BURN:
		if (bl->type == BL_PC && rnd() % 100 < 20 + 10 * skill_lv) {
			uint8 ap_burn[5] = { 20, 30, 50, 60, 70 };

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			status_fix_apdamage(src, bl, ap_burn[skill_lv - 1], 0, skill_id);
		} else if (sd)
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
		break;

	case EM_INCREASING_ACTIVITY:
		if (bl->type == BL_PC) {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			status_heal(bl, 0, 0, 10 * skill_lv, 0);
		} else if (sd)
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
		break;

	case SJ_GRAVITYCONTROL: {
			int fall_damage = sstatus->batk + sstatus->rhw.atk - tstatus->def2;

			if (bl->type == BL_PC)
				fall_damage += dstsd->weight / 10 - tstatus->def;
			else // Monster's don't have weight. Put something in its place.
				fall_damage += 50 * status_get_lv(src) - tstatus->def;

			fall_damage = max(1, fall_damage);

			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start2(src, bl, type, 100, skill_lv, fall_damage, skill_get_time(skill_id, skill_lv)));
		}
		break;
	case NPC_HALLUCINATION:
	case NPC_HELLPOWER:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv,
			sc_start(src, bl, type, skill_lv*20, skill_lv, skill_get_time2(skill_id, skill_lv)));
		break;

	case KN_AUTOCOUNTER:
		sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		skill_addtimerskill(src,tick + 100,bl->id,0,0,skill_id,skill_lv,BF_WEAPON,flag);
		break;

	case SO_STRIKING:
		if (battle_check_target(src, bl, BCT_SELF|BCT_PARTY) > 0) {
			int bonus = 0;

			if (dstsd) {
				short index = dstsd->equip_index[EQI_HAND_R];

				if (index >= 0 && dstsd->inventory_data[index] && dstsd->inventory_data[index]->type == IT_WEAPON)
					bonus = (20 * skill_lv) * dstsd->inventory_data[index]->weapon_level;
			}

			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start2(src,bl, type, 100, skill_lv, bonus, skill_get_time(skill_id, skill_lv)));
		} else if (sd)
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_TOTARGET );
		break;

	case NPC_STOP:
		if( clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl,type,100,skill_lv,src->id,skill_get_time(skill_id,skill_lv)) ) )
			sc_start2(src,src,type,100,skill_lv,bl->id,skill_get_time(skill_id,skill_lv));
		break;
	case HP_ASSUMPTIO:
		if( sd && dstmd )
			clif_skill_fail( *sd, skill_id );
		else
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;
	case MG_SIGHT:
	case MER_SIGHT:
	case AL_RUWACH:
	case WZ_SIGHTBLASTER:
	case NPC_WIDESIGHT:
	case NPC_STONESKIN:
	case NPC_ANTIMAGIC:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl,type,100,skill_lv,skill_id,skill_get_time(skill_id,skill_lv)));
		break;
	case HLIF_AVOID:
	case HAMI_DEFENCE:
		// Master
		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		// Homunculus
		clif_skill_nodamage(src, *src, skill_id, skill_lv, sc_start(src, src, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		break;
	case NJ_BUNSINJYUTSU:
		status_change_end(bl, SC_BUNSINJYUTSU); // on official recasting cancels existing mirror image [helvetica]
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		status_change_end(bl, SC_NEN);
		break;
/* Was modified to only affect targetted char.	[Skotlex]
	case HP_ASSUMPTIO:
		if (flag&1)
			sc_start(bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		else
		{
			map_foreachinallrange(skill_area_sub, bl,
				skill_get_splash(skill_id, skill_lv), BL_PC,
				src, skill_id, skill_lv, tick, flag|BCT_ALL|1,
				skill_castend_nodamage_id);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;
*/
	case SM_ENDURE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;

	case AS_ENCHANTPOISON:
		if( sc_start( src, bl, type, 100, skill_lv, skill_get_time( skill_id, skill_lv ) ) ){
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}else{
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, false );

			if( sd != nullptr ){
				clif_skill_fail( *sd, skill_id );
			}
		}
		break;

	case LK_TENSIONRELAX:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start4(src,bl,type,100,skill_lv,0,0,skill_get_time2(skill_id,skill_lv),
				skill_get_time(skill_id,skill_lv)));
		break;

	case MC_CHANGECART:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

	case MC_CARTDECORATE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if( sd ) {
			clif_SelectCart(sd);
		}
		break;

	case TK_MISSION:
		if (sd) {
			if (sd->mission_mobid && (sd->mission_count || rnd()%100)) { //Cannot change target when already have one
				clif_mission_info(sd, sd->mission_mobid, sd->mission_count);
				clif_skill_fail( *sd, skill_id );
				break;
			}

			int id = mob_get_random_id(MOBG_TAEKWON_MISSION, RMF_NONE, 0);

			if (!id) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
			sd->mission_mobid = id;
			sd->mission_count = 0;
			pc_setglobalreg(sd, add_str(TKMISSIONID_VAR), id);
			clif_mission_info(sd, id, 0);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case AC_CONCENTRATION:
		{
			int splash = skill_get_splash(skill_id, skill_lv);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
			skill_reveal_trap_inarea(src, splash, src->x, src->y);
			map_foreachinallrange( status_change_timer_sub, src,
				splash, BL_CHAR, src, nullptr, type, tick);
		}
		break;

	case SM_PROVOKE:
	case SM_SELFPROVOKE:
	case MER_PROVOKE:
		if( status_has_mode(tstatus,MD_STATUSIMMUNE) || battle_check_undead(tstatus->race,tstatus->def_ele) ) {
			map_freeblock_unlock();
			return 1;
		}
		// Official chance is 70% + 3%*skill_lv + srcBaseLevel% - tarBaseLevel%
		if(!(i = sc_start(src, bl, type, skill_id == SM_SELFPROVOKE ? 100 : (70 + 3 * skill_lv + status_get_lv(src) - status_get_lv(bl)), skill_lv, skill_get_time(skill_id, skill_lv))))
		{
			if( sd )
				clif_skill_fail( *sd, skill_id );
			map_freeblock_unlock();
			return 0;
		}
		clif_skill_nodamage(src, *bl, skill_id == SM_SELFPROVOKE ? SM_PROVOKE : skill_id, skill_lv, i != 0);
		unit_skillcastcancel(bl, 2);

		if( dstmd )
		{
			dstmd->state.provoke_flag = src->id;
			mob_target(dstmd, src, skill_get_range2(src, skill_id, skill_lv, true));
		}
		// Provoke can cause Coma even though it's a nodamage skill
		if (sd && battle_check_coma(*sd, *bl, BF_MISC))
			status_change_start(src, bl, SC_COMA, 10000, skill_lv, 0, src->id, 0, 0, SCSTART_NONE);
		break;

	case ML_DEVOTION:
	case CR_DEVOTION:
		{
			int count, lv;
			if( !dstsd || (!sd && !mer) )
			{ // Only players can be devoted
				if( sd )
					clif_skill_fail( *sd, skill_id );
				break;
			}

			if( (lv = status_get_lv(src) - dstsd->status.base_level) < 0 )
				lv = -lv;
			if( lv > battle_config.devotion_level_difference || // Level difference requeriments
				(dstsd->sc.getSCE(type) && dstsd->sc.getSCE(type)->val1 != src->id) || // Cannot Devote a player devoted from another source
				(skill_id == ML_DEVOTION && (!mer || mer != dstsd->md)) || // Mercenary only can devote owner
				(dstsd->class_&MAPID_UPPERMASK) == MAPID_CRUSADER || // Crusader Cannot be devoted
				(dstsd->sc.getSCE(SC_HELLPOWER))) // Players affected by SC_HELLPOWER cannot be devoted.
			{
				if( sd )
					clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 1;
			}

			i = 0;
			count = (sd)? min(skill_lv,MAX_DEVOTION) : 1; // Mercenary only can Devote owner
			if( sd )
			{ // Player Devoting Player
				ARR_FIND(0, count, i, sd->devotion[i] == bl->id );
				if( i == count )
				{
					ARR_FIND(0, count, i, sd->devotion[i] == 0 );
					if( i == count )
					{ // No free slots, skill Fail
						clif_skill_fail( *sd, skill_id );
						map_freeblock_unlock();
						return 1;
					}
				}

				sd->devotion[i] = bl->id;
			}
			else
				mer->devotion_flag = 1; // Mercenary Devoting Owner

			clif_skill_nodamage(src, *bl, skill_id, skill_lv,
				sc_start4(src, bl, type, 10000, src->id, i, skill_get_range2(src, skill_id, skill_lv, true), 0, skill_get_time2(skill_id, skill_lv)));
			clif_devotion(src, nullptr);
		}
		break;

	case SP_SOULUNITY: {
			int8 count = min(5 + skill_lv, MAX_UNITED_SOULS);

			if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
				if (!dstsd || !sd) { // Only put player's souls in unity.
					if (sd)
						clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
					break;
				}

				if (dstsd->sc.getSCE(type) && dstsd->sc.getSCE(type)->val2 != src->id) { // Fail if a player is in unity with another source.
					if (sd)
						clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
					map_freeblock_unlock();
					return 1;
				}

				if (sd) { // Unite player's soul with caster's soul.
					i = 0;

					ARR_FIND(0, count, i, sd->united_soul[i] == bl->id);
					if (i == count) {
						ARR_FIND(0, count, i, sd->united_soul[i] == 0);
						if(i == count) { // No more free slots? Fail the skill.
							clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
							map_freeblock_unlock();
							return 1;
						}
					}

					sd->united_soul[i] = bl->id;
				}

				clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start4(src, bl, type, 100, skill_lv, src->id, i, 0, skill_get_time(skill_id, skill_lv)));
			} else if (sd)
				party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case DK_SERVANT_W_SIGN: // Max allowed targets to be marked.
		// Only players and monsters can be marked....I think??? [Rytech]
		// Lets only allow players and monsters to use this skill for safety reasons.
		if ((!dstsd && !dstmd) || !sd && !md) {
			if (sd)
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
			break;
		}

		// Check if the target is already marked by another source.
		if (tsc && tsc->getSCE(type) && tsc->getSCE(type)->val1 != src->id) {
			if (sd)
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
			map_freeblock_unlock();
			return 1;
		}

		
		// Mark the target.
		if( sd ){
			int8 count = MAX_SERVANT_SIGN;

			ARR_FIND(0, count, i, sd->servant_sign[i] == bl->id);
			if (i == count) {
				ARR_FIND(0, count, i, sd->servant_sign[i] == 0);
				if (i == count) { // Max number of targets marked. Fail the skill.
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
					map_freeblock_unlock();
					return 1;
				}

				// Add the ID of the marked target to the player's sign list.
				sd->servant_sign[i] = bl->id;
			}

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			sc_start4(src, bl, type, 100, src->id, i, skill_lv, 0, skill_get_time(skill_id, skill_lv));
		} else if (md) // Monster's cant track with this skill. Just give the status.
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start4(src, bl, type, 100, 0, 0, skill_lv, 0, skill_get_time(skill_id, skill_lv)));
		break;

	case MO_CALLSPIRITS:
		if(sd) {
			int limit = skill_lv;
			if( sd->sc.getSCE(SC_RAISINGDRAGON) )
				limit += sd->sc.getSCE(SC_RAISINGDRAGON)->val1;
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			pc_addspiritball(sd,skill_get_time(skill_id,skill_lv),limit);
		}
		break;

	case CH_SOULCOLLECT:
		if(sd) {
			int limit = 5;
			if( sd->sc.getSCE(SC_RAISINGDRAGON) )
				limit += sd->sc.getSCE(SC_RAISINGDRAGON)->val1;
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			for (i = 0; i < limit; i++)
				pc_addspiritball(sd,skill_get_time(skill_id,skill_lv),limit);
		}
		break;

	case MO_KITRANSLATION:
		if(dstsd && ((dstsd->class_&MAPID_BASEMASK) != MAPID_GUNSLINGER && (dstsd->class_&MAPID_UPPERMASK) != MAPID_REBELLION) && dstsd->spiritball < 5) {
			//Require will define how many spiritballs will be transferred
			struct s_skill_condition require;
			require = skill_get_requirement(sd,skill_id,skill_lv);
			pc_delspiritball(sd,require.spiritball,0);
			for (i = 0; i < require.spiritball; i++)
				pc_addspiritball(dstsd,skill_get_time(skill_id,skill_lv),5);
		} else {
			if(sd)
				clif_skill_fail( *sd, skill_id );
			map_freeblock_unlock();
			return 0;
		}
		break;

	case TK_TURNKICK:
	case MO_BALKYOUNG: //Passive part of the attack. Splash knock-back+stun. [Skotlex]
		if (skill_area_temp[1] != bl->id) {
			skill_blown(src,bl,skill_get_blewcount(skill_id,skill_lv),-1,BLOWN_NONE);
			skill_additional_effect(src,bl,skill_id,skill_lv,BF_MISC,ATK_DEF,tick); //Use Misc rather than weapon to signal passive pushback
		}
		break;

	case MO_ABSORBSPIRITS:
		i = 0;
		if (dstsd && (battle_check_target(src, bl, BCT_SELF) > 0 || (battle_check_target(src, bl, BCT_ENEMY) > 0 && (map_flag_vs(src->m) || (sd && sd->duel_group && sd->duel_group == dstsd->duel_group)))) && // Only works on self and enemies
			((dstsd->class_&MAPID_BASEMASK) != MAPID_GUNSLINGER || (dstsd->class_&MAPID_UPPERMASK) != MAPID_REBELLION)) { // split the if for readability, and included gunslingers in the check so that their coins cannot be removed [Reddozen]
			if (dstsd->spiritball > 0) {
				i = dstsd->spiritball * 7;
				pc_delspiritball(dstsd,dstsd->spiritball,0);
			}
			if (dstsd->spiritcharm_type != CHARM_TYPE_NONE && dstsd->spiritcharm > 0) {
				i += dstsd->spiritcharm * 7;
				pc_delspiritcharm(dstsd,dstsd->spiritcharm,dstsd->spiritcharm_type);
			}
		} else if (dstmd && !status_has_mode(tstatus,MD_STATUSIMMUNE) && rnd() % 100 < 20) { // check if target is a monster and not status immune, for the 20% chance to absorb 2 SP per monster's level [Reddozen]
			i = 2 * dstmd->level;
			mob_target(dstmd,src,0);
		} else {
			if (sd)
				clif_skill_fail( *sd, skill_id );
			break;
		}
		if (i) status_heal(src, 0, i, 3);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,i != 0);
		break;

	case AC_MAKINGARROW:
		if( sd != nullptr ){
			clif_arrow_create_list( *sd );
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case AM_PHARMACY:
		if(sd) {
			clif_skill_produce_mix_list( *sd, skill_id, 22 );
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case SA_CREATECON:
		if( sd != nullptr ){
			clif_elementalconverter_list( *sd );
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case BS_HAMMERFALL:
		skill_addtimerskill(src, tick+1000, bl->id, 0, 0, skill_id, skill_lv, min(20+10*skill_lv, 50+5*skill_lv), flag);
		break;

	case RG_RAID:
		skill_area_temp[1] = 0;
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		map_foreachinrange(skill_area_sub, bl,
			skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL,
			src,skill_id,skill_lv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		status_change_end(src, SC_HIDING);
		break;

	//List of self skills that give damage around caster
	case ASC_METEORASSAULT:
	case GS_SPREADATTACK:
	case RK_WINDCUTTER:
	case RK_STORMBLAST:
	case NC_AXETORNADO:
	case GC_COUNTERSLASH:
	case SR_SKYNETBLOW:
	case SR_RAMPAGEBLASTER:
	case SR_HOWLINGOFLION:
	case LG_CANNONSPEAR:
	case LG_OVERBRAND:
	case NPC_RAYOFGENESIS:
	case LG_RAYOFGENESIS:
	case MH_THE_ONE_FIGHTER_RISES:
	case MH_HEILIGE_PFERD:
	case KO_HAPPOKUNAI:
	case RL_FIREDANCE:
	case RL_R_TRIP:
	case SJ_FULLMOONKICK:
	case SJ_NEWMOONKICK:
	case SJ_SOLARBURST:
	case SJ_STAREMPEROR:
	case SJ_FALLINGSTAR_ATK:
	case DK_SERVANT_W_DEMOL:
	case AG_FROZEN_SLASH:
	case IQ_OLEUM_SANCTUM:
	case IQ_MASSIVE_F_BLASTER:
	case IQ_EXPOSION_BLASTER:
	case SHC_IMPACT_CRATER:
	case MT_AXE_STOMP:
	case MT_MIGHTY_SMASH:
	case ABC_ABYSS_DAGGER:
	case BO_EXPLOSIVE_POWDER:
	{
		int starget = BL_CHAR|BL_SKILL;

		if (skill_id == SR_HOWLINGOFLION)
			starget = splash_target(src);
		if (skill_id == SJ_NEWMOONKICK) {
			if (tsce) {
				status_change_end(bl, type);
				clif_skill_nodamage(src, *bl, skill_id, skill_lv);
				break;
			} else
				sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		}
		if (skill_id == SJ_STAREMPEROR && sc && sc->getSCE(SC_DIMENSION)) {
			if (sd) {
				// Remove old shields if any exist.
				pc_delspiritball(sd, sd->spiritball, 0);
				sc_start2(src, bl, SC_DIMENSION1, 100, skill_lv, status_get_max_sp(src), skill_get_time2(SJ_BOOKOFDIMENSION, 1));
				sc_start2(src, bl, SC_DIMENSION2, 100, skill_lv, status_get_max_sp(src), skill_get_time2(SJ_BOOKOFDIMENSION, 1));
			}
			status_change_end(src, SC_DIMENSION);
		}
		if (skill_id == IQ_MASSIVE_F_BLASTER || skill_id == SHC_IMPACT_CRATER || skill_id == MT_AXE_STOMP || skill_id == ABC_ABYSS_DAGGER)
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		if (skill_id == MH_THE_ONE_FIGHTER_RISES) {
			hom_addspiritball(hd, MAX_SPIRITBALL);
		}

		skill_area_temp[1] = 0;
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		i = map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), starget,
				src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		if( !i && ( skill_id == RK_WINDCUTTER || skill_id == NC_AXETORNADO || skill_id == LG_CANNONSPEAR || skill_id == SR_SKYNETBLOW || skill_id == KO_HAPPOKUNAI ) )
			clif_skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
	}
		break;

	case SHC_DANCING_KNIFE:
		if (flag & 1) {
			skill_area_temp[1] = 0;

			// Note: doesn't force player to stand before attacking
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR | BL_SKILL, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_LEVEL | SD_SPLASH, skill_castend_damage_id);
		} else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		}
		break;

	case MT_A_MACHINE:
		if (flag & 1) {
			skill_area_temp[1] = 0;

			if (sd && pc_issit(sd)) { // Force player to stand before attacking
				pc_setstand(sd, true);
				skill_sit(sd, false);
			}

			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR | BL_SKILL, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_LEVEL | SD_SPLASH, skill_castend_damage_id);
		} else {
			if (dstsd) {
				int lv = abs( status_get_lv( src ) - status_get_lv( bl ) );

				if (lv > battle_config.attack_machine_level_difference) {
					if (sd)
						clif_skill_fail( *sd, skill_id );
					map_freeblock_unlock();
					return 0;
				}
			}

			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		}
		break;

	case EM_ELEMENTAL_BUSTER: {
		if (sd == nullptr)
			break;

		if (!sd->ed || !(sd->ed->elemental.class_ >= ELEMENTALID_DILUVIO && sd->ed->elemental.class_ <= ELEMENTALID_SERPENS)) {
			clif_skill_fail( *sd, skill_id );
			map_freeblock_unlock();
			return 0;
		}

		uint16 buster_element;

		switch (sd->ed->elemental.class_) {
			case ELEMENTALID_ARDOR:
				buster_element = EM_ELEMENTAL_BUSTER_FIRE;
				break;
			case ELEMENTALID_DILUVIO:
				buster_element = EM_ELEMENTAL_BUSTER_WATER;
				break;
			case ELEMENTALID_PROCELLA:
				buster_element = EM_ELEMENTAL_BUSTER_WIND;
				break;
			case ELEMENTALID_TERREMOTUS:
				buster_element = EM_ELEMENTAL_BUSTER_GROUND;
				break;
			case ELEMENTALID_SERPENS:
				buster_element = EM_ELEMENTAL_BUSTER_POISON;
				break;
		}

		skill_area_temp[1] = 0;
		clif_skill_nodamage(src, *bl, buster_element, skill_lv);// Animation for the triggered blaster element.
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);// Triggered after blaster animation to make correct skill name scream appear.
		map_foreachinrange(skill_area_sub, bl, 6, BL_CHAR | BL_SKILL, src, buster_element, skill_lv, tick, flag | BCT_ENEMY | SD_LEVEL | SD_SPLASH | 1, skill_castend_damage_id);
	}
	break;

	case NPC_IGNITIONBREAK:
	case RK_IGNITIONBREAK:
		skill_area_temp[1] = 0;
#if PACKETVER >= 20180207
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
#else
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
#endif
		map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		break;

	case SR_TIGERCANNON:
	case SR_WINDMILL:
	case GN_CART_TORNADO:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		[[fallthrough]];
	case SR_EARTHSHAKER:
	case NC_INFRAREDSCAN:
	case NPC_VAMPIRE_GIFT:
	case NPC_HELLJUDGEMENT:
	case NPC_HELLJUDGEMENT2:
	case NPC_PULSESTRIKE:
	case LG_MOONSLASHER:
	case NPC_WIDECRITICALWOUND:
		skill_castend_damage_id(src, src, skill_id, skill_lv, tick, flag);
		break;

#ifdef RENEWAL
	case KN_BRANDISHSPEAR:
		map_foreachindir(skill_area_sub, src->m, src->x, src->y, bl->x, bl->y,
			skill_get_splash(skill_id, skill_lv), skill_get_maxcount(skill_id, skill_lv), 0, splash_target(src),
			src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 0,
			skill_castend_damage_id);
		break;
#else
	case KN_BRANDISHSPEAR:
#endif
	case ML_BRANDISH:
		skill_area_temp[1] = bl->id;

		if(skill_lv >= 10)
			map_foreachindir(skill_area_sub, src->m, src->x, src->y, bl->x, bl->y,
				skill_get_splash(skill_id, skill_lv), 1, skill_get_maxcount(skill_id, skill_lv)-1, splash_target(src),
				src, skill_id, skill_lv, tick, flag | BCT_ENEMY | (sd?3:0),
				skill_castend_damage_id);
		if(skill_lv >= 7)
			map_foreachindir(skill_area_sub, src->m, src->x, src->y, bl->x, bl->y,
				skill_get_splash(skill_id, skill_lv), 1, skill_get_maxcount(skill_id, skill_lv)-2, splash_target(src),
				src, skill_id, skill_lv, tick, flag | BCT_ENEMY | (sd?2:0),
				skill_castend_damage_id);
		if(skill_lv >= 4)
			map_foreachindir(skill_area_sub, src->m, src->x, src->y, bl->x, bl->y,
				skill_get_splash(skill_id, skill_lv), 1, skill_get_maxcount(skill_id, skill_lv)-3, splash_target(src),
				src, skill_id, skill_lv, tick, flag | BCT_ENEMY | (sd?1:0),
				skill_castend_damage_id);
		map_foreachindir(skill_area_sub, src->m, src->x, src->y, bl->x, bl->y,
			skill_get_splash(skill_id, skill_lv), skill_get_maxcount(skill_id, skill_lv)-3, 0, splash_target(src),
			src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 0,
			skill_castend_damage_id);
		break;

	case WZ_SIGHTRASHER:
		//Passive side of the attack.
		status_change_end(src, SC_SIGHT);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		map_foreachinshootrange(skill_area_sub,src,
			skill_get_splash(skill_id, skill_lv),BL_CHAR|BL_SKILL,
			src,skill_id,skill_lv,tick, flag|BCT_ENEMY|SD_ANIMATION|1,
			skill_castend_damage_id);
		break;

	case WZ_FROSTNOVA:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		skill_area_temp[1] = 0;
		map_foreachinshootrange(skill_attack_area, src,
			skill_get_splash(skill_id, skill_lv), splash_target(src),
			BF_MAGIC, src, src, skill_id, skill_lv, tick, flag, BCT_ENEMY);
		break;

	case NPC_SELFDESTRUCTION:
		//Self Destruction hits everyone in range (allies+enemies)
		//Except for Summoned Marine spheres on non-versus maps, where it's just enemy.
		i = ((!md || md->special_state.ai == AI_SPHERE) && !map_flag_vs(src->m))?
			BCT_ENEMY:BCT_ALL;
		map_delblock(src); //Required to prevent chain-self-destructions hitting back.
		map_foreachinshootrange(skill_area_sub, bl,
			skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL,
			src, skill_id, skill_lv, tick, flag|i,
			skill_castend_damage_id);
		if(map_addblock(src)) {
			map_freeblock_unlock();
			return 1;
		}
		status_kill(src);
		break;
	case AL_ANGELUS:
#ifdef RENEWAL
	case PR_SUFFRAGIUM:
	case PR_IMPOSITIO:
#endif
	case PR_MAGNIFICAT:
	case PR_GLORIA:
		if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {

			// Animations don't play when outside visible range
			if (check_distance_bl(src, bl, AREA_SIZE))
				clif_skill_nodamage(bl, *bl, skill_id, skill_lv);

			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		}
		else if (sd)
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
		break;
#ifdef RENEWAL
	case MC_LOUD:
#endif
	case SN_WINDWALK:
	case CASH_BLESSING:
	case CASH_INCAGI:
	case CASH_ASSUMPTIO:
	case WM_FRIGG_SONG:
	case NV_HELPANGEL:
	case IG_GUARDIAN_SHIELD:
	case IG_ULTIMATE_SACRIFICE:// Is the animation on this skill correct? Check if its on caster only or all affected. [Rytech]
		if( sd == nullptr || sd->status.party_id == 0 || (flag & 1) )
			clif_skill_nodamage(bl, *bl, skill_id, skill_lv, sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		else if (sd)
		{
			if (skill_id == IG_ULTIMATE_SACRIFICE)
				status_set_hp(src, 1, 0);
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case MER_MAGNIFICAT:
		if( mer != nullptr )
		{
			clif_skill_nodamage(bl, *bl, skill_id, skill_lv, sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
			if( mer->master && mer->master->status.party_id != 0 && !(flag&1) )
				party_foreachsamemap(skill_area_sub, mer->master, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
			else if( mer->master && !(flag&1) )
				clif_skill_nodamage(src, mer->master->bl, skill_id, skill_lv, sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		}
		break;

	case BS_ADRENALINE:
	case BS_ADRENALINE2:
	case BS_WEAPONPERFECT:
	case BS_OVERTHRUST:
		if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
			int weapontype = skill_get_weapontype(skill_id);
			if (!weapontype || !dstsd || pc_check_weapontype(dstsd, weapontype)) {
				clif_skill_nodamage(bl, *bl, skill_id, skill_lv,
					sc_start2(src, bl, type, 100, skill_lv, (src == bl) ? 1 : 0, skill_get_time(skill_id, skill_lv)));
			}
		} else if (sd) {
			party_foreachsamemap(skill_area_sub,
				sd,skill_get_splash(skill_id, skill_lv),
				src,skill_id,skill_lv,tick, flag|BCT_PARTY|1,
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
	case SJ_LUNARSTANCE:
	case SJ_STARSTANCE:
	case SJ_UNIVERSESTANCE:
	case SJ_SUNSTANCE:
	case SP_SOULCOLLECT:
	case IG_GUARD_STANCE:
	case IG_ATTACK_STANCE:
		if( tsce )
		{
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,status_change_end(bl, type));
			map_freeblock_unlock();
			return 0;
		}

		if( skill_id == SP_SOULCOLLECT ){
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start2(src, bl, type, 100, skill_lv, pc_checkskill(sd, SP_SOULENERGY), skill_get_time(skill_id, skill_lv)));
		}else{
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		}
		break;
	case SL_KAITE:
	case SL_KAAHI:
	case SL_KAIZEL:
	case SL_KAUPE:
	case SP_KAUTE:
		if (sd) {
			if (!dstsd || !(
				(sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_SOULLINKER) ||
				(dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER ||
				dstsd->status.char_id == sd->status.char_id ||
				dstsd->status.char_id == sd->status.partner_id ||
				dstsd->status.char_id == sd->status.child ||
				(skill_id == SP_KAUTE && dstsd->sc.getSCE(SC_SOULUNITY))
			)) {
				status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCSTART_NORATEDEF);
				clif_skill_fail( *sd, skill_id );
				break;
			}
		}
		if (skill_id == SP_KAUTE) {
			if (!status_charge(src, sstatus->max_hp * (10 + 2 * skill_lv) / 100, 0)) {
				if (sd)
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
				break;
			}
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			status_heal(bl, 0, tstatus->max_sp * (10 + 2 * skill_lv) / 100, 2);
		} else
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		break;
	case SM_AUTOBERSERK:
	case MER_AUTOBERSERK:
		if( tsce )
			i = status_change_end(bl, type);
		else
			i = sc_start(src,bl,type,100,skill_lv,60000);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,i);
		break;
	case TF_HIDING:
	case ST_CHASEWALK:
	case KO_YAMIKUMO:
		if (tsce)
		{
			clif_skill_nodamage(src,*bl,skill_id,-1,status_change_end(bl, type)); //Hide skill-scream animation.
			map_freeblock_unlock();
			return 0;
		}
		clif_skill_nodamage(src,*bl,skill_id,-1,sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;
	case TK_RUN:
		if (tsce)
		{
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,status_change_end(bl, type));
			map_freeblock_unlock();
			return 0;
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start4(src,bl,type,100,skill_lv,unit_getdir(bl),0,0,0));
		if (sd) // If the client receives a skill-use packet inmediately before a walkok packet, it will discard the walk packet! [Skotlex]
			clif_walkok(*sd); // So aegis has to resend the walk ok.
		break;

	case AS_CLOAKING:
	case GC_CLOAKINGEXCEED:
	case LG_FORCEOFVANGUARD:
	case SC_REPRODUCE:
	case SC_INVISIBILITY:
	case RA_CAMOUFLAGE:
		if (tsce) {
			i = status_change_end(bl, type);
			if( i )
				clif_skill_nodamage(src,*bl,skill_id,( skill_id == LG_FORCEOFVANGUARD || skill_id == RA_CAMOUFLAGE ) ? skill_lv : -1,i);
			else if( sd )
				clif_skill_fail( *sd, skill_id );
			map_freeblock_unlock();
			return 0;
		}
		i = sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		if( i )
			clif_skill_nodamage(src,*bl,skill_id,( skill_id == LG_FORCEOFVANGUARD || skill_id == RA_CAMOUFLAGE ) ? skill_lv : -1,i);
		else if( sd )
			clif_skill_fail( *sd, skill_id,  USESKILL_FAIL_LEVEL );
		break;

	case CG_SPECIALSINGER:
		if (tsc && tsc->getSCE(SC_ENSEMBLEFATIGUE)) {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			status_change_end(bl, SC_ENSEMBLEFATIGUE);
		}
		break;

	case BD_ADAPTATION:
#ifdef RENEWAL
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
#else
		if(tsc && tsc->getSCE(SC_DANCING)){
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			status_change_end(bl, SC_DANCING);
		}
#endif
		break;

	case BA_FROSTJOKER:
	case DC_SCREAM:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		skill_addtimerskill(src,tick+3000,bl->id,src->x,src->y,skill_id,skill_lv,0,flag);

		if (md) {
			// custom hack to make the mob display the skill, because these skills don't show the skill use text themselves
			//NOTE: mobs don't have the sprite animation that is used when performing this skill (will cause glitches)
			char temp[70];
			snprintf(temp, sizeof(temp), "%s : %s !!",md->name,skill_get_desc(skill_id));
			clif_disp_overhead(&md->bl,temp);
		}
		break;

	case BA_PANGVOICE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv, sc_start(src,bl,SC_CONFUSION,70,7,skill_get_time(skill_id,skill_lv)));
#ifdef RENEWAL
		sc_start(src, bl, SC_BLEEDING, 30, skill_lv, skill_get_time2(skill_id, skill_lv)); // TODO: Confirm success rate
#endif
		break;

	case DC_WINKCHARM:
		if( dstsd ) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv, sc_start(src,bl,SC_CONFUSION,10,7,skill_get_time2(skill_id,skill_lv)));
#ifdef RENEWAL
			sc_start(src, bl, SC_HALLUCINATION, 30, skill_lv, skill_get_time(skill_id, skill_lv)); // TODO: Confirm success rate and duration
#endif
		} else
		if( dstmd )
		{
			if( status_get_lv(src) > status_get_lv(bl)
			&&  (tstatus->race == RC_DEMON || tstatus->race == RC_DEMIHUMAN || tstatus->race == RC_PLAYER_HUMAN || tstatus->race == RC_PLAYER_DORAM || tstatus->race == RC_ANGEL)
			&&  !status_has_mode(tstatus,MD_STATUSIMMUNE) )
				clif_skill_nodamage(src,*bl,skill_id,skill_lv, sc_start2(src,bl,type,(status_get_lv(src) - status_get_lv(bl)) + 40, skill_lv, src->id, skill_get_time(skill_id, skill_lv)));
			else
			{
				clif_skill_nodamage(src,*bl,skill_id,skill_lv,false);
				if(sd) clif_skill_fail( *sd, skill_id );
			}
		}
		break;

#ifdef RENEWAL
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
		skill_castend_song(src, skill_id, skill_lv, tick);
		break;
#endif

	case TF_STEAL:
		if(sd) {
			if(pc_steal_item(sd,bl,skill_lv))
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			else
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
		}
		break;

	case RG_STEALCOIN:
		if(sd) {
			if(pc_steal_coin(sd,bl))
			{
				dstmd->state.provoke_flag = src->id;
				mob_target(dstmd, src, skill_get_range2(src, skill_id, skill_lv, true));
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);

			}
			else
				clif_skill_fail( *sd, skill_id );
		}
		break;

	case MG_STONECURSE:
		{
			if (status_has_mode(tstatus,MD_STATUSIMMUNE)) {
				if (sd)
					clif_skill_fail( *sd, skill_id );
				break;
			}
			if(status_isimmune(bl) || !tsc)
				break;

			int32 brate = 0;

			if (sd && sd->sc.getSCE(SC_PETROLOGY_OPTION))
				brate = sd->sc.getSCE(SC_PETROLOGY_OPTION)->val3;

			if (sc_start2(src, bl, type, (skill_lv * 4 + 20) + brate, skill_lv, src->id, skill_get_time2(skill_id, skill_lv), skill_get_time(skill_id, skill_lv)))
				clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			else if(sd) {
				clif_skill_fail( *sd, skill_id );
				// Level 6-10 doesn't consume a red gem if it fails [celest]
				if (skill_lv > 5)
				{ // not to consume items
					map_freeblock_unlock();
					return 0;
				}
			}
		}
		break;

	case NV_FIRSTAID:
		clif_skill_nodamage(src,*bl,skill_id,5);
		status_heal(bl,5,0,0);
		break;

	case AL_CURE:
		if(status_isimmune(bl)) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,false);
			break;
		}
		status_change_end(bl, SC_SILENCE);
		status_change_end(bl, SC_BLIND);
		status_change_end(bl, SC_CONFUSION);
		status_change_end(bl, SC_BITESCAR);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

	case TF_DETOXIFY:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		status_change_end(bl, SC_POISON);
		status_change_end(bl, SC_DPOISON);
		break;

	case PR_STRECOVERY:
		if(status_isimmune(bl)) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,false);
			break;
		}
		if (tsc) {
			status_change_end(bl, SC_FREEZE);
			status_change_end(bl, SC_STONEWAIT);
			status_change_end(bl, SC_STONE);
			status_change_end(bl, SC_SLEEP);
			status_change_end(bl, SC_STUN);
			status_change_end(bl, SC_WHITEIMPRISON);
			status_change_end(bl, SC_STASIS);
			status_change_end(bl, SC_NETHERWORLD);
		}
		if(battle_check_undead(tstatus->race,tstatus->def_ele))
			skill_addtimerskill(src, tick+1000, bl->id, 0, 0, skill_id, skill_lv, 100, flag);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if(dstmd)
			mob_unlocktarget(dstmd,tick);
		break;

	// Mercenary Supportive Skills
	case MER_BENEDICTION:
		status_change_end(bl, SC_CURSE);
		status_change_end(bl, SC_BLIND);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case MER_COMPRESS:
		status_change_end(bl, SC_BLEEDING);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case MER_MENTALCURE:
		status_change_end(bl, SC_CONFUSION);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case MER_RECUPERATE:
		status_change_end(bl, SC_POISON);
		status_change_end(bl, SC_DPOISON);
		status_change_end(bl, SC_SILENCE);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case MER_REGAIN:
		status_change_end(bl, SC_SLEEP);
		status_change_end(bl, SC_STUN);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case MER_TENDER:
		status_change_end(bl, SC_FREEZE);
		status_change_end(bl, SC_STONE);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

	case MER_SCAPEGOAT:
		if( mer && mer->master )
		{
			status_heal(&mer->master->bl, mer->battle_status.hp, 0, 2);
			status_damage(src, src, mer->battle_status.max_hp, 0, 0, 1, skill_id);
		}
		break;

	case MER_ESTIMATION:
		if( !mer )
			break;
		sd = mer->master;
		[[fallthrough]];
	case WZ_ESTIMATION:
		if( sd == nullptr )
			break;
		if( dstsd )
		{ // Fail on Players
			clif_skill_fail( *sd, skill_id );
			break;
		}

		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		clif_skill_estimation(sd, bl);
		if( skill_id == MER_ESTIMATION )
			sd = nullptr;
		break;

	case BS_REPAIRWEAPON:
		if(sd && dstsd)
			clif_item_repair_list( *sd, *dstsd, skill_lv );
		break;

	case MC_IDENTIFY:
		if(sd) {
			clif_item_identify_list(sd);
			if( sd->menuskill_id != MC_IDENTIFY ) {// failed, dont consume anything
				map_freeblock_unlock();
				return 1;
			}
			else { // consume sp only if succeeded
				struct s_skill_condition req = skill_get_requirement(sd,skill_id,skill_lv);
				status_zap(src,0,req.sp);
			}
		}
		break;

	// Weapon Refining [Celest]
	case WS_WEAPONREFINE:
		if( sd != nullptr ){
			clif_item_refine_list( *sd );
		}
		break;

	case MC_VENDING:
		if(sd)
		{	//Prevent vending of GMs with unnecessary Level to trade/drop. [Skotlex]
			if ( !pc_can_give_items(sd) )
				clif_skill_fail( *sd, skill_id );
			else {
				sd->state.prevend = 1;
				sd->state.workinprogress = WIP_DISABLE_ALL;
				sd->vend_skill_lv = skill_lv;
				ARR_FIND(0, MAX_CART, i, sd->cart.u.items_cart[i].nameid && sd->cart.u.items_cart[i].id == 0);
				if (i < MAX_CART) {
					// Save the cart before opening the vending UI
					sd->state.pending_vending_ui = true;
					intif_storage_save(sd, &sd->cart);
				}
				else{
					// Instantly open the vending UI
					sd->state.pending_vending_ui = false;
					clif_openvendingreq( *sd, 2+skill_lv );
				}
			}
		}
		break;

	case AL_TELEPORT:
	case ALL_ODINS_RECALL:
		if(sd != nullptr)
		{
			if (map_getmapflag(bl->m, MF_NOTELEPORT) && skill_lv <= 2) {
				clif_skill_teleportmessage( *sd, NOTIFY_MAPINFO_CANT_TP );
				break;
			}
			if(!battle_config.duel_allow_teleport && sd->duel_group && skill_lv <= 2) { // duel restriction [LuzZza]
				char output[128]; sprintf(output, msg_txt(sd,365), skill_get_name(AL_TELEPORT));
				clif_displaymessage(sd->fd, output); //"Duel: Can't use %s in duel."
				break;
			}


			if( sd->state.autocast || ( (sd->skillitem == AL_TELEPORT || battle_config.skip_teleport_lv1_menu) && skill_lv == 1 ) || skill_lv == 3 )
			{
				if( skill_lv == 1 )
					pc_randomwarp(sd,CLR_TELEPORT);
				else
					pc_setpos( sd, mapindex_name2id( sd->status.save_point.map ), sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT );
				break;
			}

			clif_skill_nodamage(src,*bl,skill_id,skill_lv);

			std::vector<std::string> maps = {
				"Random"
			};

			if( skill_lv == 1 && skill_id != ALL_ODINS_RECALL ){
				clif_skill_warppoint( *sd, skill_id, skill_lv, maps );
			}else{
				maps.push_back( sd->status.save_point.map );

				clif_skill_warppoint( *sd, skill_id, skill_lv, maps );
			}
		} else
			unit_warp(bl,-1,-1,-1,CLR_TELEPORT);
		break;

	case NPC_EXPULSION:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		unit_warp(bl,-1,-1,-1,CLR_TELEPORT);
		break;

	case AL_HOLYWATER:
		if(sd) {
			if (skill_produce_mix(sd, skill_id, ITEMID_HOLY_WATER, 0, 0, 0, 1, -1)) {
				struct skill_unit* su;
				if ((su = map_find_skill_unit_oncell(bl, bl->x, bl->y, NJ_SUITON, nullptr, 0)) != nullptr)
					skill_delunit(su);
				clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			}
			else
				clif_skill_fail( *sd, skill_id );
		}
		break;

	case TF_PICKSTONE:
		if(sd) {
			unsigned char eflag;
			struct item item_tmp;
			struct block_list tbl;
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			memset(&item_tmp,0,sizeof(item_tmp));
			memset(&tbl,0,sizeof(tbl)); // [MouseJstr]
			item_tmp.nameid = ITEMID_STONE;
			item_tmp.identify = 1;
			tbl.id = 0;
			// Commented because of duplicate animation [Lemongrass]
			// At the moment this displays the pickup animation a second time
			// If this is required in older clients, we need to add a version check here
			//clif_takeitem(sd->bl,tbl);
			eflag = pc_additem(sd,&item_tmp,1,LOG_TYPE_PRODUCE);
			if(eflag) {
				clif_additem(sd,0,0,eflag);
				if (battle_config.skill_drop_items_full)
					map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,4,0);
			}
		}
		break;
	case ASC_CDP:
		if(sd) {
			if(skill_produce_mix(sd, skill_id, ITEMID_POISON_BOTTLE, 0, 0, 0, 1, -1)) //Produce a Poison Bottle.
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			else
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_STUFF_INSUFFICIENT );
		}
		break;

	case RG_STRIPWEAPON:
	case RG_STRIPSHIELD:
	case RG_STRIPARMOR:
	case RG_STRIPHELM:
	case ST_FULLSTRIP:
	case GC_WEAPONCRUSH:
	case SC_STRIPACCESSARY:
	case ABC_STRIP_SHADOW: {
		bool i;

		//Special message when trying to use strip on FCP [Jobbie]
		if( sd && skill_id == ST_FULLSTRIP && tsc && tsc->getSCE(SC_CP_WEAPON) && tsc->getSCE(SC_CP_HELM) && tsc->getSCE(SC_CP_ARMOR) && tsc->getSCE(SC_CP_SHIELD))
		{
			clif_gospel_info(sd, 0x28);
			break;
		}

		if( (i = skill_strip_equip(src, bl, skill_id, skill_lv)) || (skill_id != ST_FULLSTRIP && skill_id != GC_WEAPONCRUSH ) )
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,i);

		//Nothing stripped.
		if( sd && !i )
			clif_skill_fail( *sd, skill_id );
		break;
	}

	case AM_BERSERKPITCHER:
	case AM_POTIONPITCHER:
		{
			int j,hp = 0,sp = 0;
			if( dstmd && dstmd->mob_id == MOBID_EMPERIUM ) {
				map_freeblock_unlock();
				return 1;
			}
			if( sd ) {
				int x,bonus=100;
				struct s_skill_condition require = skill_get_requirement(sd, skill_id, skill_lv);
				x = skill_lv%11 - 1;
				j = pc_search_inventory(sd, require.itemid[x]);
				if (j < 0 || require.itemid[x] <= 0) {
					clif_skill_fail( *sd, skill_id );
					map_freeblock_unlock();
					return 1;
				}
				if (sd->inventory_data[j] == nullptr || sd->inventory.u.items_inventory[j].amount < require.amount[x]) {
					clif_skill_fail( *sd, skill_id );
					map_freeblock_unlock();
					return 1;
				}
				if( skill_id == AM_BERSERKPITCHER ) {
					if( dstsd && dstsd->status.base_level < (unsigned int)sd->inventory_data[j]->elv ) {
						clif_skill_fail( *sd, skill_id );
						map_freeblock_unlock();
						return 1;
					}
				}
				potion_flag = 1;
				potion_hp = potion_sp = potion_per_hp = potion_per_sp = 0;
				potion_target = bl->id;
				run_script(sd->inventory_data[j]->script,0,sd->bl.id,0);
				potion_flag = potion_target = 0;
				if( sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_ALCHEMIST )
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
						hp = hp * (100 + (tstatus->vit * 2)) / 100;
						if( dstsd )
							hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10) / 100;
					}
					if( potion_sp > 0 ) {
						sp = potion_sp * (100 + pc_checkskill(sd,AM_POTIONPITCHER)*10 + pc_checkskill(sd,AM_LEARNINGPOTION)*5)*bonus/10000;
						sp = sp * (100 + (tstatus->int_ * 2)) / 100;
						if( dstsd )
							sp = sp * (100 + pc_checkskill(dstsd,MG_SRECOVERY)*10) / 100;
					}
				}

				if ((bonus = pc_get_itemgroup_bonus_group(sd, IG_POTION, sd->itemgrouphealrate))) {
					hp += hp * bonus / 100;
				}

				if( ( bonus = pc_get_itemgroup_bonus_group( sd, IG_POTION, sd->itemgroupsphealrate ) ) ){
					sp += sp * bonus / 100;
				}

				if( (j = pc_skillheal_bonus(sd, skill_id)) ) {
					hp += hp * j / 100;
					sp += sp * j / 100;
				}
			} else {
				//Maybe replace with potion_hp, but I'm unsure how that works [Playtester]
				switch (skill_lv) {
					case 1: hp = 45; break;
					case 2: hp = 105; break;
					case 3: hp = 175; break;
					default: hp = 325; break;
				}
				hp = (hp + rnd()%(skill_lv*20+1)) * (150 + skill_lv*10) / 100;
				hp = hp * (100 + (tstatus->vit * 2)) / 100;
				if( dstsd )
					hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10) / 100;
			}
			if( dstsd && (j = pc_skillheal2_bonus(dstsd, skill_id)) ) {
				hp += hp * j / 100;
				sp += sp * j / 100;
			}
			// Final heal increased by HPlus.
			// Is this the right place for this??? [Rytech]
			// Can HPlus also affect SP recovery???
			if (sd && sstatus->hplus > 0) {
				hp += hp * sstatus->hplus / 100;
				sp += sp * sstatus->hplus / 100;
			}
			if (tsc && tsc->count) {
				uint8 penalty = 0;

				if (tsc->getSCE(SC_WATER_INSIGNIA) && tsc->getSCE(SC_WATER_INSIGNIA)->val1 == 2) {
					hp += hp / 10;
					sp += sp / 10;
				}
				if (tsc->getSCE(SC_CRITICALWOUND))
					penalty += tsc->getSCE(SC_CRITICALWOUND)->val2;
				if (tsc->getSCE(SC_DEATHHURT) && tsc->getSCE(SC_DEATHHURT)->val3)
					penalty += 20;
				if (tsc->getSCE(SC_NORECOVER_STATE))
					penalty = 100;
				if (penalty > 0) {
					hp -= hp * penalty / 100;
					sp -= sp * penalty / 100;
				}
			}

#ifdef RENEWAL
			if (bl->type == BL_HOM)
				hp *= 3; // Heal effectiveness is 3x for Homunculus
#endif

			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			if( hp > 0 || (skill_id == AM_POTIONPITCHER && sp <= 0) )
				clif_skill_nodamage(nullptr,*bl,AL_HEAL,hp,1);
			if( sp > 0 )
				clif_skill_nodamage(nullptr,*bl,MG_SRECOVERY,sp);
			if (tsc) {
#ifdef RENEWAL
				if (tsc->getSCE(SC_EXTREMITYFIST))
					sp = 0;
#endif
				if (tsc->getSCE(SC_NORECOVER_STATE)) {
					hp = 0;
					sp = 0;
				}
			}
			status_heal(bl,hp,sp,0);
		}
		break;
	case AM_CP_WEAPON:
	case AM_CP_SHIELD:
	case AM_CP_ARMOR:
	case AM_CP_HELM:
		{
			unsigned int equip[] = {EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP};

			if( sd && ( bl->type != BL_PC || ( dstsd && pc_checkequip(dstsd,equip[skill_id - AM_CP_WEAPON]) < 0 ) ) ){
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock(); // Don't consume item requirements
				return 0;
			}
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		}
		break;
	case AM_TWILIGHT1:
		if (sd) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			//Prepare 200 White Potions.
			if (!skill_produce_mix(sd, skill_id, ITEMID_WHITE_POTION, 0, 0, 0, 200, -1))
				clif_skill_fail( *sd, skill_id );
		}
		break;
	case AM_TWILIGHT2:
		if (sd) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			//Prepare 200 Slim White Potions.
			if (!skill_produce_mix(sd, skill_id, ITEMID_WHITE_SLIM_POTION, 0, 0, 0, 200, -1))
				clif_skill_fail( *sd, skill_id );
		}
		break;
	case AM_TWILIGHT3:
		if (sd) {
			int ebottle = pc_search_inventory(sd,ITEMID_EMPTY_BOTTLE);
			short alcohol_idx = -1, acid_idx = -1, fire_idx = -1;
			if( ebottle >= 0 )
				ebottle = sd->inventory.u.items_inventory[ebottle].amount;
			//check if you can produce all three, if not, then fail:
			if (!(alcohol_idx = skill_can_produce_mix(sd,ITEMID_ALCOHOL,-1, 100)) //100 Alcohol
				|| !(acid_idx = skill_can_produce_mix(sd,ITEMID_ACID_BOTTLE,-1, 50)) //50 Acid Bottle
				|| !(fire_idx = skill_can_produce_mix(sd,ITEMID_FIRE_BOTTLE,-1, 50)) //50 Flame Bottle
				|| ebottle < 200 //200 empty bottle are required at total.
			) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			skill_produce_mix(sd, skill_id, ITEMID_ALCOHOL, 0, 0, 0, 100, alcohol_idx-1);
			skill_produce_mix(sd, skill_id, ITEMID_ACID_BOTTLE, 0, 0, 0, 50, acid_idx-1);
			skill_produce_mix(sd, skill_id, ITEMID_FIRE_BOTTLE, 0, 0, 0, 50, fire_idx-1);
		}
		break;
	case SA_DISPELL:
		if (flag&1 || (i = skill_get_splash(skill_id, skill_lv)) < 1) {
			if (sd && dstsd && !map_flag_vs(sd->bl.m) && (!sd->duel_group || sd->duel_group != dstsd->duel_group) && (!sd->status.party_id || sd->status.party_id != dstsd->status.party_id))
				break; // Outside PvP it should only affect party members and no skill fail message
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			if((dstsd && (dstsd->class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
				|| (tsc && tsc->getSCE(SC_SPIRIT) && tsc->getSCE(SC_SPIRIT)->val2 == SL_ROGUE) //Rogue's spirit defends againt dispel.
				|| rnd()%100 >= 50+10*skill_lv)
			{
				if (sd)
					clif_skill_fail( *sd, skill_id );
				break;
			}
			if(status_isimmune(bl))
				break;

			//Remove bonus_script by Dispell
			if (dstsd)
				pc_bonus_script_clear(dstsd,BSF_REM_ON_DISPELL);
			// Monsters will unlock their target instead
			else if (dstmd)
				mob_unlocktarget(dstmd, tick);

			if(!tsc || !tsc->count)
				break;

			//Statuses that can't be Dispelled
			for (const auto &it : status_db) {
				sc_type status = static_cast<sc_type>(it.first);

				if (!tsc->getSCE(status))
					continue;

				if (it.second->flag[SCF_NODISPELL])
					continue;
				switch (status) {
					// bugreport:4888 these songs may only be dispelled if you're not in their song area anymore
					case SC_WHISTLE:		case SC_ASSNCROS:		case SC_POEMBRAGI:
					case SC_APPLEIDUN:		case SC_HUMMING:		case SC_DONTFORGETME:
					case SC_FORTUNE:		case SC_SERVICE4U:
						if (!battle_config.dispel_song || tsc->getSCE(status)->val4 == 0)
							continue; //If in song area don't end it, even if config enabled
						break;
					case SC_ASSUMPTIO:
						if( bl->type == BL_MOB )
							continue;
						break;
				}
				if (i == SC_BERSERK || i == SC_SATURDAYNIGHTFEVER)
					tsc->getSCE(status)->val2 = 0; //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
				status_change_end(bl, status);
			}
			break;
		}

		//Affect all targets on splash area.
		map_foreachinallrange(skill_area_sub, bl, i, BL_CHAR,
			src, skill_id, skill_lv, tick, flag|1,
			skill_castend_damage_id);
		break;

	case TF_BACKSLIDING: //This is the correct implementation as per packet logging information. [Skotlex]
		{
			short blew_count = skill_blown(src,bl,skill_get_blewcount(skill_id,skill_lv),unit_getdir(bl),(enum e_skill_blown)(BLOWN_IGNORE_NO_KNOCKBACK
#ifdef RENEWAL
			|BLOWN_DONT_SEND_PACKET
#endif
			));
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
#ifdef RENEWAL
			if(blew_count > 0)
				clif_blown(src); // Always blow, otherwise it shows a casting animation. [Lemongrass]
#else
			clif_slide(*bl, bl->x, bl->y); //Show the casting animation on pre-re
#endif
		}
		break;

	case TK_HIGHJUMP:
		{
			int x,y, dir = unit_getdir(src);
			struct map_data *mapdata = &map[src->m];

			//Fails on noteleport maps, except for GvG and BG maps [Skotlex]
			if( mapdata->getMapFlag(MF_NOTELEPORT) &&
				!(mapdata->getMapFlag(MF_BATTLEGROUND) || mapdata_flag_gvg2(mapdata) )
			) {
				clif_skill_nodamage(src, *bl, TK_HIGHJUMP, skill_lv);
				break;
			} else if(dir%2) {
				//Diagonal
				x = src->x + dirx[dir]*(skill_lv*4)/3;
				y = src->y + diry[dir]*(skill_lv*4)/3;
			} else {
				x = src->x + dirx[dir]*skill_lv*2;
				y = src->y + diry[dir]*skill_lv*2;
			}

			int x1 = x + dirx[dir];
			int y1 = y + diry[dir];

			clif_skill_nodamage(src,*bl,TK_HIGHJUMP,skill_lv);
			if( !map_count_oncell(src->m,x,y,BL_PC|BL_NPC|BL_MOB,0) && map_getcell(src->m,x,y,CELL_CHKREACH) &&
				!map_count_oncell(src->m,x1,y1,BL_PC|BL_NPC|BL_MOB,0) && map_getcell(src->m,x1,y1,CELL_CHKREACH) &&
				unit_movepos(src, x, y, 1, 0))
				clif_blown(src);
		}
		break;

	case SA_CASTCANCEL:
	case SO_SPELLFIST:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		unit_skillcastcancel(src,1);
		if(sd) {
			int sp = skill_get_sp(sd->skill_id_old,sd->skill_lv_old);
			if( skill_id == SO_SPELLFIST ){
				sc_start4(src,src,type,100,skill_lv,sd->skill_id_old,sd->skill_lv_old,0,skill_get_time(skill_id,skill_lv));
				sd->skill_id_old = sd->skill_lv_old = 0;
				break;
			}
			sp = sp * (90 - (skill_lv-1)*20) / 100;
			if(sp < 0) sp = 0;
			status_zap(src, 0, sp);
		}
		break;
	case SA_SPELLBREAKER:
		{
			int sp;
			if (dstsd && tsc && tsc->getSCE(SC_MAGICROD)) {
				// If target enemy player has Magic Rod, then 20% of your SP is transferred to that player
				sp = status_percent_damage(bl, src, 0, -20, false);
				status_heal(bl, 0, sp, 2);
			}
			else {
				struct unit_data* ud = unit_bl2ud(bl);
				if (!ud || ud->skilltimer == INVALID_TIMER)
					break; //Nothing to cancel.
				int hp = 0;
				if (status_has_mode(tstatus, MD_STATUSIMMUNE)) { //Only 10% success chance against status immune. [Skotlex]
					if (rnd_chance(90, 100))
					{
						if (sd) clif_skill_fail( *sd, skill_id );
						break;
					}
				}
#ifdef RENEWAL
				else // HP damage does not work on bosses in renewal
#endif
					if (skill_lv >= 5 && (!dstsd || map_flag_vs(bl->m))) //HP damage only on pvp-maps when against players.
						hp = tstatus->max_hp / 50; //Siphon 2% HP at level 5

				clif_skill_nodamage(src, *bl, skill_id, skill_lv);
				unit_skillcastcancel(bl, 0);
				sp = skill_get_sp(ud->skill_id, ud->skill_lv);
				status_zap(bl, 0, sp);
				// Recover some of the SP used
				status_heal(src, 0, sp * (25 * (skill_lv - 1)) / 100, 2);

				// If damage would be lethal, it does not deal damage
				if (hp && hp < tstatus->hp) {
					clif_damage(*src, *bl, tick, 0, 0, hp, 0, DMG_NORMAL, 0, false);
					status_zap(bl, hp, 0);
					// Recover 50% of damage dealt
					status_heal(src, hp / 2, 0, 2);
				}
			}
		}
		break;
	case SA_MAGICROD:
#ifdef RENEWAL
		clif_skill_nodamage(src,*src,SA_MAGICROD,skill_lv);
#endif
		sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		break;
	case SA_AUTOSPELL:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if (sd) {
			sd->state.workinprogress = WIP_DISABLE_ALL;
			clif_autospell( *sd, skill_lv );
		} else {
			int maxlv=1,spellid=0;
			static const int spellarray[3] = { MG_COLDBOLT,MG_FIREBOLT,MG_LIGHTNINGBOLT };

			if(skill_lv >= 10) {
				spellid = MG_FROSTDIVER;
//				if (tsc && tsc->getSCE(SC_SPIRIT) && tsc->getSCE(SC_SPIRIT)->val2 == SA_SAGE)
//					maxlv = 10;
//				else
					maxlv = skill_lv - 9;
			}
			else if(skill_lv >=8) {
				spellid = MG_FIREBALL;
				maxlv = skill_lv - 7;
			}
			else if(skill_lv >=5) {
				spellid = MG_SOULSTRIKE;
				maxlv = skill_lv - 4;
			}
			else if(skill_lv >=2) {
				int i_rnd = rnd()%3;
				spellid = spellarray[i_rnd];
				maxlv = skill_lv - 1;
			}
			else if(skill_lv > 0) {
				spellid = MG_NAPALMBEAT;
				maxlv = 3;
			}

			if(spellid > 0)
				sc_start4(src,src,SC_AUTOSPELL,100,skill_lv,spellid,maxlv,0,
					skill_get_time(SA_AUTOSPELL,skill_lv));
		}
		break;

	case BS_GREED:
		if(sd){
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			map_foreachinallrange(skill_greed,bl,
				skill_get_splash(skill_id, skill_lv),BL_ITEM,bl);
		}
		break;

	case SA_ELEMENTWATER:
	case SA_ELEMENTFIRE:
	case SA_ELEMENTGROUND:
	case SA_ELEMENTWIND:
		if (sd && (!dstmd || status_has_mode(tstatus,MD_STATUSIMMUNE))) // Only works on monsters (Except status immune monsters).
			break;
		[[fallthrough]];
	case NPC_ATTRICHANGE:
	case NPC_CHANGEWATER:
	case NPC_CHANGEGROUND:
	case NPC_CHANGEFIRE:
	case NPC_CHANGEWIND:
	case NPC_CHANGEPOISON:
	case NPC_CHANGEHOLY:
	case NPC_CHANGEDARKNESS:
	case NPC_CHANGETELEKINESIS:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl, type, 100, skill_lv, skill_get_ele(skill_id,skill_lv),
				skill_get_time(skill_id, skill_lv)));
		break;

	case NPC_PROVOCATION:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if (md) mob_unlocktarget(md, tick);
		break;

	case NPC_REBIRTH:
		if( md && md->state.rebirth )
			break; // only works once
		sc_start(src,bl,type,100,skill_lv,INFINITE_TICK);
		break;

	case NPC_DARKBLESSING:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl,type,(50+skill_lv*5),skill_lv,skill_lv,skill_get_time2(skill_id,skill_lv)));
		break;

	case NPC_LICK:
		status_zap(bl, 0, 100);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,(skill_lv*20),skill_lv,skill_get_time2(skill_id,skill_lv)));
		break;

	case NPC_SUICIDE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		status_kill(src); //When suiciding, neither exp nor drops is given.
		break;

	case NPC_SUMMONSLAVE:
	case NPC_SUMMONMONSTER:
	case NPC_DEATHSUMMON:
		if(md && md->skill_idx >= 0)
			mob_summonslave(md,md->db->skill[md->skill_idx]->val,skill_lv,skill_id);
		break;

	case NPC_CALLSLAVE:
		mob_warpslave(src,MOB_SLAVEDISTANCE);
		break;

	case NPC_RANDOMMOVE:
		if (md) {
			md->next_walktime = tick - 1;
			if (md->special_state.ai == AI_SPHERE)
				unit_escape(&md->bl, bl, 7, 2);
			else
				mob_randomwalk(md,tick);
		}
		break;

	case NPC_SPEEDUP:
		{
			// or does it increase casting rate? just a guess xD
			int i_type = SC_ASPDPOTION0 + skill_lv - 1;
			if (i_type > SC_ASPDPOTION3)
				i_type = SC_ASPDPOTION3;
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,
				sc_start(src,bl,(sc_type)i_type,100,skill_lv,skill_lv * 60000));
		}
		break;

	case NPC_REVENGE:
		// not really needed... but adding here anyway ^^
		if (md && md->master_id > 0) {
			struct block_list *mbl, *tbl;
			if ((mbl = map_id2bl(md->master_id)) == nullptr ||
				(tbl = battle_gettargeted(mbl)) == nullptr)
				break;
			md->state.provoke_flag = tbl->id;
			mob_target(md, tbl, sstatus->rhw.range);
		}
		break;

	case NPC_RUN:
		if (md) {
			block_list* tbl = map_id2bl(md->target_id);

			if (tbl) {
				md->state.can_escape = 1;
				mob_unlocktarget(md, tick);
				// Official distance is 7, if level > 1, distance = level
				t_tick time = unit_escape(src, tbl, skill_lv > 1 ? skill_lv : 7, 2);

				if (time) {
					// Need to set state here as it's not set otherwise
					md->state.skillstate = MSS_WALK;
					// Set AI to inactive for the duration of this movement
					md->last_thinktime = tick + time;
				}
			}
		}
		break;

	case NPC_TRANSFORMATION:
	case NPC_METAMORPHOSIS:
		if(md && md->skill_idx >= 0) {
			int class_ = mob_random_class (md->db->skill[md->skill_idx]->val,0);
			if (skill_lv > 1) //Multiply the rest of mobs. [Skotlex]
				mob_summonslave(md,md->db->skill[md->skill_idx]->val,skill_lv-1,skill_id);
			if (class_) mob_class_change(md, class_);
		}
		break;

	case NPC_EMOTION_ON:
	case NPC_EMOTION:
		//val[0] is the emotion to use.
		//NPC_EMOTION & NPC_EMOTION_ON can change a mob's mode 'permanently' [Skotlex]
		//val[1] 'sets' the mode
		//val[2] adds to the current mode
		//val[3] removes from the current mode
		//val[4] if set, asks to delete the previous mode change.
		if(md && md->skill_idx >= 0 && tsc)
		{
			clif_emotion(bl, md->db->skill[md->skill_idx]->val[0]);
			if(md->db->skill[md->skill_idx]->val[4] && tsce)
				status_change_end(bl, type);

			//If mode gets set by NPC_EMOTION then the target should be reset [Playtester]
			if(!battle_config.npc_emotion_behavior && skill_id == NPC_EMOTION && md->db->skill[md->skill_idx]->val[1])
				mob_unlocktarget(md,tick);

			if(md->db->skill[md->skill_idx]->val[1] || md->db->skill[md->skill_idx]->val[2])
				sc_start4(src,src, type, 100, skill_lv,
					md->db->skill[md->skill_idx]->val[1],
					md->db->skill[md->skill_idx]->val[2],
					md->db->skill[md->skill_idx]->val[3],
					skill_get_time(skill_id, skill_lv));

			//Reset aggressive state depending on resulting mode
			if (!battle_config.npc_emotion_behavior)
				md->state.aggressive = status_has_mode(&md->status,MD_ANGRY)?1:0;
		}
		break;

	case NPC_POWERUP:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl,type,100,200,100,skill_get_time(skill_id, skill_lv)));
		break;

	case NPC_AGIUP:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl,type,100,50,100,skill_get_time(skill_id, skill_lv)));
		break;

	case NPC_INVISIBLE:
		//Have val4 passed as 6 is for "infinite cloak" (do not end on attack/skill use).
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start4(src,bl,type,100,skill_lv,0,0,6,skill_get_time(skill_id,skill_lv)));
		break;

	case NPC_SIEGEMODE:
		// Not implemented/used: Gives EFST_SIEGEMODE which reduces speed to 1000.
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

	case NPC_INVINCIBLEOFF:
	case MER_INVINCIBLEOFF2:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		status_change_end(bl, SC_INVINCIBLE);
		break;

	case WE_MALE: {
			uint8 hp_rate = abs(skill_get_hp_rate(skill_id, skill_lv));

			if (hp_rate && status_get_hp(src) > status_get_max_hp(src) / hp_rate) {
				int gain_hp = tstatus->max_hp * hp_rate / 100; // The earned is the same % of the target HP than it costed the caster. [Skotlex]

				clif_skill_nodamage(src,*bl,skill_id,status_heal(bl, gain_hp, 0, 0));
			}
		}
		break;
	case WE_FEMALE: {
			uint8 sp_rate = abs(skill_get_sp_rate(skill_id, skill_lv));

			if (sp_rate && status_get_sp(src) > status_get_max_sp(src) / sp_rate) {
				int gain_sp = tstatus->max_sp * sp_rate / 100; // The earned is the same % of the target SP than it costed the caster. [Skotlex]

				clif_skill_nodamage(src,*bl,skill_id,status_heal(bl, 0, gain_sp, 0));
			}
		}
		break;

	// parent-baby skills
	case WE_BABY:
		if(sd){
			map_session_data *f_sd = pc_get_father(sd);
			map_session_data *m_sd = pc_get_mother(sd);

			if( (!f_sd && !m_sd) // if neither was found
				|| (sd->status.party_id != 0 && //not in same party
					((!f_sd || sd->status.party_id != f_sd->status.party_id) 
					&& (!m_sd || sd->status.party_id != m_sd->status.party_id) //if both are online they should all be in same team
					))
				|| ((!f_sd || !check_distance_bl(&sd->bl, &f_sd->bl, AREA_SIZE)) //not in same screen
					&& (!m_sd || !check_distance_bl(&sd->bl, &m_sd->bl, AREA_SIZE)))
			) {
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 0;
			}
			status_change_start(src,bl,SC_STUN,10000,skill_lv,0,0,0,skill_get_time2(skill_id,skill_lv),SCSTART_NORATEDEF);
			if (f_sd) sc_start(src,&f_sd->bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
			if (m_sd) sc_start(src,&m_sd->bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		}
		break;

	case WE_CALLALLFAMILY:
		if (sd) {
			map_session_data *p_sd = pc_get_partner(sd);
			map_session_data *c_sd = pc_get_child(sd);

			if (!p_sd && !c_sd) { // Fail if no family members are found
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 1;
			}

			// Partner must be on the same map and in same party
			if (p_sd && !status_isdead(p_sd->bl) && p_sd->bl.m == sd->bl.m && p_sd->status.party_id == sd->status.party_id)
				pc_setpos(p_sd, map_id2index(sd->bl.m), sd->bl.x, sd->bl.y, CLR_TELEPORT);
			// Child must be on the same map and in same party as the parent casting
			if (c_sd && !status_isdead(c_sd->bl) && c_sd->bl.m == sd->bl.m && c_sd->status.party_id == sd->status.party_id)
				pc_setpos(c_sd, map_id2index(sd->bl.m), sd->bl.x, sd->bl.y, CLR_TELEPORT);
		}
		break;

	case WE_ONEFOREVER:
		if (sd) {
			map_session_data *p_sd = pc_get_partner(sd);
			map_session_data *c_sd = pc_get_child(sd);

			if (!p_sd && !c_sd && !dstsd) { // Fail if no family members are found
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 1;
			}
			if (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND)) { // No reviving in WoE grounds!
				clif_skill_fail( *sd, skill_id );
				break;
			}
			if (status_isdead(*bl)) {
				int per = 30, sper = 0;

				if (battle_check_undead(tstatus->race, tstatus->def_ele))
					break;
				if (tsc && tsc->getSCE(SC_HELLPOWER))
					break;
				if (map_getmapflag(bl->m, MF_PVP) && dstsd->pvp_point < 0)
					break;
				if (dstsd->special_state.restart_full_recover)
					per = sper = 100;
				if ((dstsd == p_sd || dstsd == c_sd) && status_revive(bl, per, sper)) // Only family members can be revived
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			}
		}
		break;

	case WE_CHEERUP:
		if (sd) {
			map_session_data *f_sd = pc_get_father(sd);
			map_session_data *m_sd = pc_get_mother(sd);

			if (!f_sd && !m_sd && !dstsd) { // Fail if no family members are found
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 1;
			}
			if (flag&1) { // Buff can only be given to parents in 7x7 AoE around baby
				if (dstsd == f_sd || dstsd == m_sd)
					clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
			} else
				map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ALL|1, skill_castend_nodamage_id);
		}
		break;

	case PF_HPCONVERSION:
		{
			int hp, sp;
			hp = sstatus->max_hp/10;
			sp = hp * 10 * skill_lv / 100;
			if (!status_charge(src,hp,0)) {
				if (sd) clif_skill_fail( *sd, skill_id );
				break;
			}
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			status_heal(bl,0,sp,2);
		}
		break;

	case MA_REMOVETRAP:
	case HT_REMOVETRAP:
		{
			skill_unit* su = BL_CAST(BL_SKILL, bl);
			std::shared_ptr<s_skill_unit_group> sg;
			std::shared_ptr<s_skill_db> skill_group;

			// Mercenaries can remove any trap
			// Players can only remove their own traps or traps on Vs maps.
			if( su && (sg = su->group) && (src->type == BL_MER || sg->src_id == src->id || map_flag_vs(bl->m)) && ( skill_group = skill_db.find(sg->skill_id) ) && skill_group->inf2[INF2_ISTRAP] )
			{
				clif_skill_nodamage(src, *bl, skill_id, skill_lv);
				if( sd && !(sg->unit_id == UNT_USED_TRAPS || (sg->unit_id == UNT_ANKLESNARE && sg->val2 != 0 )) )
				{ // prevent picking up expired traps
					if( battle_config.skill_removetrap_type )
					{ // get back all items used to deploy the trap
						for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; i++ )
						{
							if( skill_group->require.itemid[i] > 0 )
							{
								int flag2;
								struct item item_tmp;
								memset(&item_tmp,0,sizeof(item_tmp));
								item_tmp.nameid = skill_group->require.itemid[i];
								item_tmp.identify = 1;
								item_tmp.amount = skill_group->require.amount[i];
								if( item_tmp.nameid && (flag2=pc_additem(sd,&item_tmp,item_tmp.amount,LOG_TYPE_OTHER)) ){
									clif_additem(sd,0,0,flag2);
									if (battle_config.skill_drop_items_full)
										map_addflooritem(&item_tmp,item_tmp.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,4,0);
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
							if (battle_config.skill_drop_items_full)
								map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,4,0);
						}
					}
				}
				skill_delunit(su);
			}else if(sd)
				clif_skill_fail( *sd, skill_id );

		}
		break;
	case HT_SPRINGTRAP:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		{
			struct skill_unit *su=nullptr;
			if((bl->type==BL_SKILL) && (su=(struct skill_unit *)bl) && (su->group) ){
				switch(su->group->unit_id){
					case UNT_ANKLESNARE:	// ankle snare
						if (su->group->val2 != 0)
							// if it is already trapping something don't spring it,
							// remove trap should be used instead
							break;
						[[fallthrough]];
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
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if(sd)
			unit_skilluse_id(src,src->id,sd->skill_id_dance,sd->skill_lv_dance);
		break;

	case TR_RETROSPECTION:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		if (sd)
			unit_skilluse_id(src, src->id, sd->skill_id_song, sd->skill_lv_song);
		break;

	case AS_SPLASHER:
		if( status_has_mode(tstatus,MD_STATUSIMMUNE)
		// Renewal dropped the 3/4 hp requirement
#ifndef RENEWAL
			|| tstatus-> hp > tstatus->max_hp*3/4
#endif
				) {
			if (sd) clif_skill_fail( *sd, skill_id );
			map_freeblock_unlock();
			return 1;
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start4(src,bl,type,100,skill_lv,skill_id,src->id,skill_get_time(skill_id,skill_lv),1000));
		break;

	case PF_MINDBREAKER:
		{
			if(status_has_mode(tstatus,MD_STATUSIMMUNE) || battle_check_undead(tstatus->race,tstatus->def_ele)) {
				map_freeblock_unlock();
				return 1;
			}

			if (tsce)
			{	//HelloKitty2 (?) explained that this silently fails when target is
				//already inflicted. [Skotlex]
				map_freeblock_unlock();
				return 1;
			}

			//Has a 55% + skill_lv*5% success chance.
			if (!clif_skill_nodamage(src,*bl,skill_id,skill_lv,
				sc_start(src,bl,type,55+5*skill_lv,skill_lv,skill_get_time(skill_id,skill_lv))))
			{
				if (sd) clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 0;
			}

			unit_skillcastcancel(bl,0);

			if (dstmd)
				mob_target(dstmd, src, skill_get_range2(src, skill_id, skill_lv, true));
		}
		break;

	case PF_SOULCHANGE:
		{
			unsigned int sp1 = 0, sp2 = 0;
			if (dstmd) {
				if (dstmd->state.soul_change_flag) {
					if(sd) clif_skill_fail( *sd, skill_id );
					break;
				}
				dstmd->state.soul_change_flag = 1;
				sp2 = sstatus->max_sp * 3 /100;
				status_heal(src, 0, sp2, 2);
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
				break;
			}
			sp1 = sstatus->sp;
			sp2 = tstatus->sp;
#ifdef	RENEWAL
			sp1 = sp1 / 2;
			sp2 = sp2 / 2;
			if (tsc && tsc->getSCE(SC_EXTREMITYFIST))
				sp1 = tstatus->sp;
#endif
			if (tsc && tsc->getSCE(SC_NORECOVER_STATE))
				sp1 = tstatus->sp;
			status_set_sp(src, sp2, 3);
			status_set_sp(bl, sp1, 3);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	// Slim Pitcher
	case CR_SLIMPITCHER:
		// Updated to block Slim Pitcher from working on barricades and guardian stones.
		if (dstmd && (dstmd->mob_id == MOBID_EMPERIUM || status_get_class_(bl) == CLASS_BATTLEFIELD))
			break;
		if (potion_hp || potion_sp) {
			int hp = potion_hp, sp = potion_sp;
			hp = hp * (100 + (tstatus->vit * 2))/100;
			sp = sp * (100 + (tstatus->int_ * 2))/100;
			if (dstsd) {
				if (hp)
					hp = hp * (100 + pc_checkskill(dstsd,SM_RECOVERY)*10 + pc_skillheal2_bonus(dstsd, skill_id))/100;
				if (sp)
					sp = sp * (100 + pc_checkskill(dstsd,MG_SRECOVERY)*10 + pc_skillheal2_bonus(dstsd, skill_id))/100;
			}
			if (tsc && tsc->count) {
				uint8 penalty = 0;

				if (tsc->getSCE(SC_WATER_INSIGNIA) && tsc->getSCE(SC_WATER_INSIGNIA)->val1 == 2) {
					hp += hp / 10;
					sp += sp / 10;
				}
				if (tsc->getSCE(SC_CRITICALWOUND))
					penalty += tsc->getSCE(SC_CRITICALWOUND)->val2;
				if (tsc->getSCE(SC_DEATHHURT) && tsc->getSCE(SC_DEATHHURT)->val3 == 1)
					penalty += 20;
				if (tsc->getSCE(SC_NORECOVER_STATE))
					penalty = 100;
				if (penalty > 0) {
					hp -= hp * penalty / 100;
					sp -= sp * penalty / 100;
				}
			}
			if(hp > 0)
				clif_skill_nodamage(nullptr,*bl,AL_HEAL,hp);
			if(sp > 0)
				clif_skill_nodamage(nullptr,*bl,MG_SRECOVERY,sp);
			status_heal(bl,hp,sp,0);
		}
		break;
	// Full Chemical Protection
	case CR_FULLPROTECTION:
		{
			unsigned int equip[] = {EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP};
			int i_eqp, s = 0, skilltime = skill_get_time(skill_id,skill_lv);

			for (i_eqp = 0; i_eqp < 4; i_eqp++) {
				if( bl->type != BL_PC || ( dstsd && pc_checkequip(dstsd,equip[i_eqp]) < 0 ) )
					continue;
				sc_start(src,bl,(sc_type)(SC_CP_WEAPON + i_eqp),100,skill_lv,skilltime);
				s++;
			}
			if( sd && !s ){
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock(); // Don't consume item requirements
				return 0;
			}
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case RG_CLEANER:	//AppleGirl
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

#ifndef RENEWAL
	case CG_LONGINGFREEDOM:
		{
			if (tsc && !tsce && (tsce=tsc->getSCE(SC_DANCING)) && tsce->val4
				&& (tsce->val1&0xFFFF) != CG_MOONLIT) //Can't use Longing for Freedom while under Moonlight Petals. [Skotlex]
			{
				clif_skill_nodamage(src,*bl,skill_id,skill_lv,
					sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
			}
		}
		break;
#endif

	case CG_TAROTCARD:
		{
			int card = -1;
			if (tsc && tsc->getSCE(SC_TAROTCARD)) {
				//Target currently has the SUN tarot card effect and is immune to any other effect
				map_freeblock_unlock();
				return 0;
			}
			if( rnd() % 100 > skill_lv * 8 ||
#ifndef RENEWAL
			(tsc && tsc->getSCE(SC_BASILICA)) ||
#endif
			(dstmd && ((dstmd->guardian_data && dstmd->mob_id == MOBID_EMPERIUM) || status_get_class_(bl) == CLASS_BATTLEFIELD)) ) {
				if( sd )
					clif_skill_fail( *sd, skill_id );

				map_freeblock_unlock();
				return 0;
			}
			status_zap(src,0,skill_get_sp(skill_id,skill_lv)); // consume sp only if succeeded [Inkfish]
			card = skill_tarotcard(src, bl, skill_id, skill_lv, tick); // actual effect is executed here
			clif_specialeffect((card == 6) ? src : bl, EF_TAROTCARD1 + card - 1, AREA);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
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
	case SL_HIGH:
		if( sc_start2( src, bl, type, 100, skill_lv, skill_id, skill_get_time( skill_id, skill_lv ) ) ){
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);

			// 1% chance to erase death count on successful cast
			if( skill_id == SL_SUPERNOVICE && dstsd && dstsd->die_counter && rnd_chance( 1, 100 )  ){
				pc_setparam( dstsd, SP_PCDIECOUNTER, 0 );
				clif_specialeffect( bl, EF_ANGEL2, AREA );
				status_calc_pc( dstsd, SCO_NONE );
			}

			sc_start( src, src, SC_SMA, 100, skill_lv, skill_get_time( SL_SMA, skill_lv ) );
		}else{
			if( sd ){
				clif_skill_fail( *sd, skill_id );
			}
		}
		break;
	case SP_SOULGOLEM:
	case SP_SOULSHADOW:
	case SP_SOULFALCON:
	case SP_SOULFAIRY:
		if( sc_start( src, bl, type, 100, skill_lv, skill_get_time( skill_id, skill_lv ) ) ){
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}else{
			if( sd ){
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
			}
		}
		break;

	case SP_SOULREVOLVE:
		if (!(tsc && (tsc->getSCE(SC_SPIRIT) || tsc->getSCE(SC_SOULGOLEM) || tsc->getSCE(SC_SOULSHADOW) || tsc->getSCE(SC_SOULFALCON) || tsc->getSCE(SC_SOULFAIRY)))) {
			if (sd)
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
			break;
		}
		status_heal(bl, 0, 50*skill_lv, 2);
		status_change_end(bl, SC_SPIRIT);
		status_change_end(bl, SC_SOULGOLEM);
		status_change_end(bl, SC_SOULSHADOW);
		status_change_end(bl, SC_SOULFALCON);
		status_change_end(bl, SC_SOULFAIRY);
		break;

	case SL_SWOO:
		if (tsce) {
			if(sd)
				clif_skill_fail( *sd, skill_id );
			status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,10000,SCSTART_NORATEDEF);
			status_change_end(bl, SC_SWOO);
			break;
		}
		[[fallthrough]];
	case SL_SKA: // [marquis007]
	case SL_SKE:
		if (sd && !battle_config.allow_es_magic_pc && bl->type != BL_MOB) {
			clif_skill_fail( *sd, skill_id );
			status_change_start(src,src,SC_STUN,10000,skill_lv,0,0,0,500,SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			break;
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		if (skill_id == SL_SKE)
			sc_start(src,src,SC_SMA,100,skill_lv,skill_get_time(SL_SMA,skill_lv));
		break;

	// New guild skills [Celest]
	case GD_BATTLEORDER:
	case GD_REGENERATION:
	case GD_RESTORE:
	case GD_EMERGENCY_MOVE:
		if(flag&1) {
			if (status_get_guild_id(src) == status_get_guild_id(bl)) {				
				if( skill_id == GD_RESTORE )
					clif_skill_nodamage(src,*bl,AL_HEAL,status_percent_heal(bl,90,90));
				else
					sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id, skill_lv));
			}
		} else if (status_get_guild_id(src)) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			map_foreachinallrange(skill_area_sub, src,
				skill_get_splash(skill_id, skill_lv), BL_PC,
				src,skill_id,skill_lv,tick, flag|BCT_GUILD|1,
				skill_castend_nodamage_id);
			if (sd)
#ifdef RENEWAL
				skill_blockpc_start(sd, skill_id, skill_get_cooldown(skill_id, skill_lv));
#else
				guild_block_skill(sd, skill_get_time2(skill_id, skill_lv));
#endif
		}
		break;
	case GD_EMERGENCYCALL:
	case GD_ITEMEMERGENCYCALL:
		{
			int8 dx[9] = {-1, 1, 0, 0,-1, 1,-1, 1, 0};
			int8 dy[9] = { 0, 0, 1,-1, 1,-1,-1, 1, 0};
			uint8 j = 0, calls = 0, called = 0;
			// i don't know if it actually summons in a circle, but oh well. ;P
			auto g = sd?sd->guild:guild_search(status_get_guild_id(src));
			if (!g)
				break;

			if (skill_id == GD_ITEMEMERGENCYCALL)
				switch (skill_lv) {
					case 1:	calls = 7; break;
					case 2:	calls = 12; break;
					case 3:	calls = 20; break;
					default: calls = 0;	break;
				}

			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			for (i = 0; i < g->guild.max_member && (!calls || (calls && called < calls)); i++, j++) {
				if (j > 8)
					j = 0;
				if ((dstsd = g->guild.member[i].sd) != nullptr && sd != dstsd && !dstsd->state.autotrade && !pc_isdead(dstsd)) {
					if (dstsd->status.disable_call)
						continue;
					if (map_getmapflag(dstsd->bl.m, MF_NOWARP) && !map_flag_gvg2(dstsd->bl.m))
						continue;
					if (!pc_job_can_entermap((enum e_job)dstsd->status.class_, src->m, pc_get_group_level(dstsd)))
						continue;
					if(map_getcell(src->m,src->x+dx[j],src->y+dy[j],CELL_CHKNOREACH))
						dx[j] = dy[j] = 0;
					if (!pc_setpos(dstsd, map_id2index(src->m), src->x+dx[j], src->y+dy[j], CLR_RESPAWN))
						called++;
				}
			}
			if (sd)
#ifdef RENEWAL
				skill_blockpc_start(sd, skill_id, skill_get_cooldown(skill_id, skill_lv));
#else
				guild_block_skill(sd, skill_get_time2(skill_id, skill_lv));
#endif
		}
		break;
	case GD_CHARGESHOUT_FLAG:
		if (sd && sd->guild && sd->state.gmaster_flag == 1) {
			mob_data *md = mob_once_spawn_sub(src, src->m, src->x, src->y, sd->guild->guild.name, MOBID_GUILD_SKILL_FLAG, nullptr, SZ_SMALL, AI_GUILD);

			if (md) {
				sd->guild->chargeshout_flag_id = md->bl.id;
				md->master_id = src->id;

				if (md->deletetimer != INVALID_TIMER)
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer(gettick() + skill_get_time(GD_CHARGESHOUT_FLAG, skill_lv), mob_timer_delete, md->bl.id, 0);
				mob_spawn(md);
			}
		}
		break;
	case GD_CHARGESHOUT_BEATING:
		if (sd && sd->guild && map_blid_exists(sd->guild->chargeshout_flag_id)) {
			block_list *mob_bl = map_id2bl(sd->guild->chargeshout_flag_id);

			if (pc_setpos(sd, map_id2index(mob_bl->m), mob_bl->x, mob_bl->y, CLR_RESPAWN) != SETPOS_OK)
				clif_skill_fail( *sd, skill_id );
			else
				clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		} else if (sd)
			clif_skill_fail( *sd, skill_id );
		break;

	case SG_FEEL:
		//AuronX reported you CAN memorize the same map as all three. [Skotlex]
		if (sd) {
			if(!sd->feel_map[skill_lv-1].index)
				clif_feel_req(sd->fd,sd, skill_lv);
			else
				clif_feel_info(sd, skill_lv-1, 1);
		}
		break;

	case SG_HATE:
		if (sd) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			if (!pc_set_hate_mob(sd, skill_lv-1, bl))
				clif_skill_fail( *sd, skill_id );
		}
		break;

	case SJ_DOCUMENT:
		if (sd) {
			switch (skill_lv) {
				case 1:
					pc_resetfeel(sd);
					break;
				case 2:
					pc_resethate(sd);
					break;
				case 3:
					pc_resetfeel(sd);
					pc_resethate(sd);
					break;
			}
		}
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		break;

	case GS_GLITTERING:
		if(sd) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			if(rnd()%100 < (20+10*skill_lv))
				pc_addspiritball(sd,skill_get_time(skill_id,skill_lv),10);
			else if(sd->spiritball > 0 && !pc_checkskill(sd,RL_RICHS_COIN))
				pc_delspiritball(sd,1,0);
		}
		break;

	case GS_CRACKER:
		/* per official standards, this skill works on players and mobs. */
		if (sd && (dstsd || dstmd))
		{
			i =65 -5*distance_bl(src,bl); //Base rate
			if (i < 30) i = 30;
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			sc_start(src,bl,SC_STUN, i,skill_lv,skill_get_time2(skill_id,skill_lv));
		}
		break;

	case AM_CALLHOMUN:	//[orn]
		if (sd && !hom_call(sd))
			clif_skill_fail( *sd, skill_id );
#ifdef RENEWAL
		else if (sd && hom_is_active(sd->hd))
			skill_area_temp[0] = 1; // Already passed pre-cast checks
#endif
		break;

	case AM_REST:
		if (sd) {
			if (hom_vaporize(sd,HOM_ST_REST))
				clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			else
				clif_skill_fail( *sd, skill_id );
		}
		break;

	case HAMI_CASTLE:	//[orn]
		if (src != bl && rnd_chance(20 * skill_lv, 100)) {
			// Get one of the monsters targeting the player and set the homunculus as its new target
			if (block_list* tbl = battle_gettargeted(bl); tbl != nullptr && tbl->type == BL_MOB) {
				if (unit_data* ud = unit_bl2ud(tbl); ud != nullptr)
					unit_changetarget_sub(*ud, *src);
			}

			int16 x = src->x, y = src->y;
			// Move homunculus
			if (unit_movepos(src, bl->x, bl->y, 0, false)) {
				clif_blown(src);
				// Move player
				if (unit_movepos(bl, x, y, 0, false))
					clif_blown(bl);
				// Show the animation on the homunculus only
				clif_skill_nodamage(src, *src, skill_id, skill_lv);
			}
		}
		else if (hd != nullptr && hd->master != nullptr)
			clif_skill_fail( *hd->master, skill_id );
		else if (sd != nullptr)
			clif_skill_fail( *sd, skill_id );
		break;
	case HVAN_CHAOTIC:
		{
			// Chance per skill level
			static const std::array<uint8, 5> chance_homunculus = {
				20,
				50,
				25,
				50,
				34
			};
			static const std::array<uint8, 5> chance_master = {
				static_cast<uint8>(chance_homunculus[0] + 30),
				static_cast<uint8>(chance_homunculus[1] + 10),
				static_cast<uint8>(chance_homunculus[2] + 50),
				static_cast<uint8>(chance_homunculus[3] + 4),
				static_cast<uint8>(chance_homunculus[4] + 33)
			};

			uint8 chance = rnd_value(1, 100);

			// Homunculus
			if (chance <= chance_homunculus[skill_lv - 1])
				bl = src;
			// Master
			else if (chance <= chance_master[skill_lv - 1])
				bl = battle_get_master(src);
			// Enemy (A random enemy targeting the master)
			else
				bl = battle_gettargeted(battle_get_master(src));

			// If there's no enemy the chance reverts to the homunculus
			if (bl == nullptr)
				bl = src;

			int32 heal = skill_calc_heal(src, bl, skill_id, rnd_value<uint16>(1, skill_lv), true);

			// Official servers send the Heal skill packet with the healed amount, and then the skill packet with 1 as healed amount
			clif_skill_nodamage(src, *bl, AL_HEAL, heal);
			clif_skill_nodamage(src, *bl, skill_id, 1);
			status_heal(bl, heal, 0, 0);
		} break;
	case HVAN_EXPLOSION:
		if( hd != nullptr ){
			clif_skill_nodamage(src, *src, skill_id, skill_lv, 1);
			map_foreachinshootrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR | BL_SKILL, src, skill_id, skill_lv, tick, flag | BCT_ENEMY, skill_castend_damage_id);

			hd->homunculus.intimacy = hom_intimacy_grade2intimacy(HOMGRADE_HATE_WITH_PASSION);
			clif_send_homdata(*hd, SP_INTIMATE);

			// There's a delay between the explosion and the homunculus death
			skill_addtimerskill(src, tick + skill_get_time(skill_id, skill_lv), src->id, 0, 0, skill_id, skill_lv, 0, flag);
		} break;
	// Homun single-target support skills [orn]
	case HLIF_CHANGE:
#ifndef RENEWAL
		status_percent_heal(bl, 100, 100);
		[[fallthrough]];
#endif
	case HAMI_BLOODLUST:
	case HFLI_FLEET:
	case HFLI_SPEED:
	case MH_ANGRIFFS_MODUS:
	case MH_GOLDENE_FERSE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;

	case NPC_DRAGONFEAR:
		if (flag&1) {
			const enum sc_type sc[] = { SC_STUN, SC_SILENCE, SC_CONFUSION, SC_BLEEDING };
			int j;
			j = i = rnd()%ARRAYLENGTH(sc);
			while ( !sc_start2(src,bl,sc[i],100,skill_lv,src->id,skill_get_time2(skill_id,i+1)) ) {
				i++;
				if ( i == ARRAYLENGTH(sc) )
					i = 0;
				if (i == j)
					break;
			}
			break;
		}
		[[fallthrough]];
	case NPC_WIDEBLEEDING:
	case NPC_WIDEBLEEDING2:
	case NPC_WIDECONFUSE:
	case NPC_WIDECONFUSE2:
	case NPC_WIDECURSE:
	case NPC_WIDECURSE2:
	case NPC_WIDEFREEZE:
	case NPC_WIDEFREEZE2:
	case NPC_WIDESLEEP:
	case NPC_WIDESLEEP2:
	case NPC_WIDESILENCE:
	case NPC_WIDESILENCE2:
	case NPC_WIDESTONE:
	case NPC_WIDESTONE2:
	case NPC_WIDESTUN:
	case NPC_WIDESTUN2:
	case NPC_SLOWCAST:
	case NPC_WIDEHELLDIGNITY:
	case NPC_WIDEHEALTHFEAR:
	case NPC_WIDEBODYBURNNING:
	case NPC_WIDEFROSTMISTY:
	case NPC_WIDECOLD:
	case NPC_WIDE_DEEP_SLEEP:
	case NPC_WIDESIREN:
	case NPC_WIDEWEB:
	case NPC_DEADLYCURSE2:
		if (flag&1){
			switch ( type ) {
			case SC_BURNING:
				sc_start4(src,bl,type,100,skill_lv,1000,src->id,0,skill_get_time2(skill_id,skill_lv));
				break;
			case SC_STONEWAIT:
				sc_start2(src,bl,type,100,skill_lv,src->id,skill_get_time2(skill_id,skill_lv),skill_get_time(skill_id, skill_lv));
				break;
			default:
				sc_start2(src,bl,type,100,skill_lv,src->id,skill_get_time2(skill_id,skill_lv));
			}
		}
		else {
			skill_area_temp[2] = 0; //For SD_PREAMBLE
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			map_foreachinallrange(skill_area_sub, bl,
				skill_get_splash(skill_id, skill_lv),BL_CHAR,
				src,skill_id,skill_lv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
				skill_castend_nodamage_id);
		}
		break;
	case NPC_WIDESOULDRAIN:
		if (flag&1)
			status_percent_damage(src,bl,0,((skill_lv-1)%5+1)*20,false);
		else {
			skill_area_temp[2] = 0; //For SD_PREAMBLE
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			map_foreachinallrange(skill_area_sub, bl,
				skill_get_splash(skill_id, skill_lv),BL_CHAR,
				src,skill_id,skill_lv,tick, flag|BCT_ENEMY|SD_PREAMBLE|1,
				skill_castend_nodamage_id);
		}
		break;
	case NPC_FIRESTORM: {
		int sflag = flag;

		if( skill_lv > 1 )
			sflag |= 4;
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		map_foreachinshootrange(skill_area_sub,src,skill_get_splash(skill_id,skill_lv),splash_target(src),src,
			skill_id,skill_lv,tick,sflag|BCT_ENEMY|SD_ANIMATION|1,skill_castend_damage_id);
		}
		break;
	case ALL_PARTYFLEE:
		if( sd  && !(flag&1) ) {
			if( !sd->status.party_id ) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		} else
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;
	case NPC_TALK:
	case ALL_WEWISH:
	case ALL_CATCRY:
	case ALL_DREAM_SUMMERNIGHT:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;
	case ALL_BUYING_STORE:
		if( sd )
		{// players only, skill allows 5 buying slots
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, buyingstore_setup(sd, MAX_BUYINGSTORE_SLOTS) == 0);
		}
		break;
	case RK_ENCHANTBLADE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start2(src,bl,type,100,skill_lv,((100+20*skill_lv)*status_get_lv(src))/100+sstatus->int_,skill_get_time(skill_id,skill_lv)));
		break;
	case RK_DRAGONHOWLING:
		if( flag&1)
			sc_start(src,bl,type,50 + 6 * skill_lv,skill_lv,skill_get_time(skill_id,skill_lv));
		else
		{
			skill_area_temp[2] = 0;
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			map_foreachinallrange(skill_area_sub, src,
				skill_get_splash(skill_id,skill_lv),BL_CHAR,
				src,skill_id,skill_lv,tick,flag|BCT_ENEMY|SD_PREAMBLE|1,
				skill_castend_nodamage_id);
		}
		break;

	case LG_EARTHDRIVE: {
			int dummy = 1;

			clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			i = skill_get_splash(skill_id,skill_lv);
			map_foreachinallarea(skill_cell_overlap, src->m, src->x-i, src->y-i, src->x+i, src->y+i, BL_SKILL, LG_EARTHDRIVE, &dummy, src);
			map_foreachinrange(skill_area_sub, bl,i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		}
		break;
	case RK_LUXANIMA:
		status_change_clear_buffs(bl, SCCB_LUXANIMA); // For bonus_script
		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		break;
	case RK_GIANTGROWTH:
	case RK_STONEHARDSKIN:
	case RK_VITALITYACTIVATION:
	case RK_ABUNDANCE:
	case RK_CRUSHSTRIKE:
	case RK_REFRESH:
	case RK_MILLENNIUMSHIELD:
		if (sd) {
			uint8 rune_level = 1; // RK_GIANTGROWTH

			if (skill_id == RK_VITALITYACTIVATION)
 				rune_level = 2;
			else if (skill_id == RK_STONEHARDSKIN)
				rune_level = 4;
			else if (skill_id == RK_ABUNDANCE)
 				rune_level = 6;
			else if (skill_id == RK_CRUSHSTRIKE)
 				rune_level = 7;
			else if (skill_id == RK_REFRESH)
				rune_level = 8;
			else if (skill_id == RK_MILLENNIUMSHIELD)
				rune_level = 9;
			if (pc_checkskill(sd, RK_RUNEMASTERY) >= rune_level) {
				if (sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)))
					clif_skill_nodamage(src, *bl, skill_id, skill_lv);
				else if (skill_id == RK_STONEHARDSKIN)
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT );
			} else
				clif_skill_fail( *sd, skill_id );
 		}
 		break;

	case NPC_MILLENNIUMSHIELD:
		if (sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)))
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
 		break;

	case RK_FIGHTINGSPIRIT: {
			uint8 runemastery_skill_lv = (sd ? pc_checkskill(sd, RK_RUNEMASTERY) : skill_get_max(RK_RUNEMASTERY));

			// val1: ATKBonus: ? // !TODO: Confirm new ATK formula
			// val2: ASPD boost: [RK_RUNEMASTERYlevel * 4 / 10] * 10 ==> RK_RUNEMASTERYlevel * 4
			sc_start2(src,bl,type,100,70 + 7 * runemastery_skill_lv,4 * runemastery_skill_lv,skill_get_time(skill_id,skill_lv));
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			break;
		}

	case GC_ROLLINGCUTTER:
		{
			short count = 1;
			skill_area_temp[2] = 0;
			map_foreachinrange(skill_area_sub,src,skill_get_splash(skill_id,skill_lv),BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|SD_PREAMBLE|SD_SPLASH|1,skill_castend_damage_id);
			if( tsc && tsc->getSCE(SC_ROLLINGCUTTER) )
			{ // Every time the skill is casted the status change is reseted adding a counter.
				count += (short)tsc->getSCE(SC_ROLLINGCUTTER)->val1;
				if( count > 10 )
					count = 10; // Max coounter
				status_change_end(bl, SC_ROLLINGCUTTER);
			}
			sc_start(src,bl,SC_ROLLINGCUTTER,100,count,skill_get_time(skill_id,skill_lv));
			clif_skill_nodamage(src,*src,skill_id,skill_lv);
		}
		break;

	case GC_WEAPONBLOCKING:
		if( tsc && tsc->getSCE(SC_WEAPONBLOCKING) )
			status_change_end(bl, SC_WEAPONBLOCKING);
		else
			sc_start(src,bl,SC_WEAPONBLOCKING,100,skill_lv,skill_get_time(skill_id,skill_lv));
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

	case GC_CREATENEWPOISON:
		if( sd )
		{
			clif_skill_produce_mix_list( *sd, skill_id, 25 );
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case GC_POISONINGWEAPON:
		if( sd ) {
			clif_poison_list( *sd, skill_lv );
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case GC_ANTIDOTE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if( tsc )
		{
			status_change_end(bl, SC_PARALYSE);
			status_change_end(bl, SC_PYREXIA);
			status_change_end(bl, SC_DEATHHURT);
			status_change_end(bl, SC_LEECHESEND);
			status_change_end(bl, SC_VENOMBLEED);
			status_change_end(bl, SC_MAGICMUSHROOM);
			status_change_end(bl, SC_TOXIN);
			status_change_end(bl, SC_OBLIVIONCURSE);
		}
		break;

	case GC_PHANTOMMENACE:
		clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		map_foreachinrange(skill_area_sub,src,skill_get_splash(skill_id,skill_lv),BL_CHAR,
			src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;

	case GC_HALLUCINATIONWALK:
		{
			int heal = status_get_max_hp(bl) / 10;
			if( status_get_hp(bl) < heal ) { // if you haven't enough HP skill fails.
				if( sd ) clif_skill_fail( *sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT );
				break;
			}
			if( !status_charge(bl,heal,0) )
			{
				if( sd ) clif_skill_fail( *sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT );
				break;
			}
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		}
		break;

	case AB_ANCILLA:
		if( sd ) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			skill_produce_mix(sd, skill_id, ITEMID_ANCILLA, 0, 0, 0, 1, -1);
		}
		break;

	case AB_CLEMENTIA:
	case AB_CANTO:
		{
			int bless_lv = ((sd) ? pc_checkskill(sd,AL_BLESSING) : skill_get_max(AL_BLESSING)) + (((sd) ? sd->status.job_level : 50) / 10);
			int agi_lv = ((sd) ? pc_checkskill(sd,AL_INCAGI) : skill_get_max(AL_INCAGI)) + (((sd) ? sd->status.job_level : 50) / 10);
			if( sd == nullptr || sd->status.party_id == 0 || flag&1 )
				clif_skill_nodamage(bl, *bl, skill_id, skill_lv, sc_start(src,bl,type,100,
					(skill_id == AB_CLEMENTIA)? bless_lv : (skill_id == AB_CANTO)? agi_lv : skill_lv, skill_get_time(skill_id,skill_lv)));
			else if( sd )
				party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case AB_PRAEFATIO:
	case AB_RENOVATIO:
		if( !sd || sd->status.party_id == 0 || flag&1 ) {
			if (skill_id == AB_PRAEFATIO)
				clif_skill_nodamage(bl, *bl, skill_id, skill_lv, sc_start4(src, bl, type, 100, skill_lv, 0, 0, (sd && sd->status.party_id ? party_foreachsamemap(party_sub_count, sd, 0) : 1 ), skill_get_time(skill_id, skill_lv)));
			else
				clif_skill_nodamage(bl, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		} else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case AB_CHEAL:
		if( !sd || sd->status.party_id == 0 || flag&1 ) {
			if( sd && tstatus && !battle_check_undead(tstatus->race, tstatus->def_ele) && !tsc->getSCE(SC_BERSERK) ) {
				int partycount = (sd->status.party_id ? party_foreachsamemap(party_sub_count, sd, 0) : 0);

				i = skill_calc_heal(src, bl, AL_HEAL, pc_checkskill(sd, AL_HEAL), true);

				if( partycount > 1 )
					i += (i / 100) * (partycount * 10) / 4;
				if( (dstsd && pc_ismadogear(dstsd)) || status_isimmune(bl))
					i = 0; // Should heal by 0 or won't do anything?? in iRO it breaks the healing to members.. [malufett]

				clif_skill_nodamage(src, *bl, skill_id, i);
				if( tsc && tsc->getSCE(SC_AKAITSUKI) && i )
					i = ~i + 1;
				status_heal(bl, i, 0, 0);
			}
		} else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case NPC_CHEAL:
		if( flag&1 ) {
			if( tstatus && !battle_check_undead(tstatus->race, tstatus->def_ele) && !tsc->getSCE(SC_BERSERK) ) {
				i = skill_calc_heal(src, bl, AL_HEAL, 10, true);
				if (status_isimmune(bl))
					i = 0;
				clif_skill_nodamage(src, *bl, skill_id, i);
				if( tsc && tsc->getSCE(SC_AKAITSUKI) && i )
					i = ~i + 1;
				status_heal(bl, i, 0, 0);
			}
		}
		else {
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_MOB,
				src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case AB_ORATIO:
		if( flag&1 )
			sc_start(src,bl, type, 40 + 5 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
		else {
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR,
				src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case AB_LAUDAAGNUS:
		if( flag&1 || !sd || !sd->status.party_id ) {
			if( tsc && (tsc->getSCE(SC_FREEZE) || tsc->getSCE(SC_STONE) || tsc->getSCE(SC_BLIND) ||
				tsc->getSCE(SC_BURNING) || tsc->getSCE(SC_FREEZING) || tsc->getSCE(SC_CRYSTALIZE))) {
				// Success Chance: (60 + 10 * Skill Level) %
				if( rnd()%100 > 60+10*skill_lv ) break;
				status_change_end(bl, SC_FREEZE);
				status_change_end(bl, SC_STONE);
				status_change_end(bl, SC_BLIND);
				status_change_end(bl, SC_BURNING);
				status_change_end(bl, SC_FREEZING);
				status_change_end(bl, SC_CRYSTALIZE);
			} else //Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
				clif_skill_nodamage(bl, *bl, skill_id, skill_lv,
					sc_start(src,bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		} else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv),
				src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case AB_LAUDARAMUS:
		if( flag&1 || !sd || !sd->status.party_id ) {
			if( tsc && (tsc->getSCE(SC_SLEEP) || tsc->getSCE(SC_STUN) || tsc->getSCE(SC_MANDRAGORA) || tsc->getSCE(SC_SILENCE) || tsc->getSCE(SC_DEEPSLEEP)) ){
				// Success Chance: (60 + 10 * Skill Level) %
				if( rnd()%100 > 60+10*skill_lv )  break;
				status_change_end(bl, SC_SLEEP);
				status_change_end(bl, SC_STUN);
				status_change_end(bl, SC_MANDRAGORA);
				status_change_end(bl, SC_SILENCE);
				status_change_end(bl, SC_DEEPSLEEP);
			} else // Success rate only applies to the curing effect and not stat bonus. Bonus status only applies to non infected targets
				clif_skill_nodamage(bl, *bl, skill_id, skill_lv,
					sc_start(src,bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		} else if( sd )
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv),
				src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case AB_CLEARANCE:
		if( flag&1 || (i = skill_get_splash(skill_id, skill_lv)) < 1 ) { // As of the behavior in official server Clearance is just a super version of Dispell skill. [Jobbie]

			if( bl->type != BL_MOB && battle_check_target(src,bl,BCT_PARTY) <= 0 ) // Only affect mob or party.
				break;

			clif_skill_nodamage(src,*bl,skill_id,skill_lv);

			if(rnd()%100 >= 60 + 8 * skill_lv) {
				if (sd)
					clif_skill_fail( *sd, skill_id );
				break;
			}
			
			if(status_isimmune(bl))
				break;

			//Remove bonus_script by Clearance
			if (dstsd)
				pc_bonus_script_clear(dstsd,BSF_REM_ON_CLEARANCE);

			if(!tsc || !tsc->count)
				break;

			//Statuses change that can't be removed by Cleareance
			for (const auto &it : status_db) {
				sc_type status = static_cast<sc_type>(it.first);

				if (!tsc->getSCE(status))
					continue;

				if (it.second->flag[SCF_NOCLEARANCE])
					continue;

				switch (status) {
					case SC_WHISTLE:		case SC_ASSNCROS:		case SC_POEMBRAGI:
					case SC_APPLEIDUN:		case SC_HUMMING:		case SC_DONTFORGETME:
					case SC_FORTUNE:		case SC_SERVICE4U:
						if (!battle_config.dispel_song || tsc->getSCE(status)->val4 == 0)
							continue; //If in song area don't end it, even if config enabled
						break;
					case SC_ASSUMPTIO:
						if (bl->type == BL_MOB)
							continue;
						break;
				}
				if (i == SC_BERSERK || i == SC_SATURDAYNIGHTFEVER)
					tsc->getSCE(status)->val2 = 0; //Mark a dispelled berserk to avoid setting hp to 100 by setting hp penalty to 0.
				status_change_end(bl,status);
			}
			break;
		}

		map_foreachinallrange(skill_area_sub, bl, i, BL_CHAR, src, skill_id, skill_lv, tick, flag|1, skill_castend_damage_id);
		break;

	case AB_SILENTIUM:
		// Should the level of Lex Divina be equivalent to the level of Silentium or should the highest level learned be used? [LimitLine]
		map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR,
			src, PR_LEXDIVINA, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		break;

	case WL_STASIS:
		if (flag&1)
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		else {
			struct map_data *mapdata = map_getmapdata(src->m);

			map_foreachinallrange(skill_area_sub,src,skill_get_splash(skill_id, skill_lv),BL_CHAR,src,skill_id,skill_lv,tick,(mapdata_flag_vs(mapdata)?BCT_ALL:BCT_ENEMY|BCT_SELF)|flag|1,skill_castend_nodamage_id);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case NPC_DANCINGBLADE:
		skill_addtimerskill(src, tick + status_get_amotion(src), bl->id, 0, 0, NPC_DANCINGBLADE_ATK, skill_lv, 0, 0);
		break;

	case WL_CHAINLIGHTNING:
		skill_addtimerskill(src, tick + status_get_amotion(src), bl->id, 0, 0, WL_CHAINLIGHTNING_ATK, skill_lv, 0, 0);
		break;

	case WL_WHITEIMPRISON:
		if( (src == bl || battle_check_target(src, bl, BCT_ENEMY)>0) && status_get_class_(bl) != CLASS_BOSS && !status_isimmune(bl) ) // Should not work with Bosses.
		{
			int rate = ( sd? sd->status.job_level : 50 ) / 4;

			if( src == bl ) rate = 100; // Success Chance: On self, 100%
			else if(bl->type == BL_PC) rate += 20 + 10 * skill_lv; // On Players, (20 + 10 * Skill Level) %
			else rate += 40 + 10 * skill_lv; // On Monsters, (40 + 10 * Skill Level) %

			if( sd )
				skill_blockpc_start(sd,skill_id,4000);

			if( !(tsc && tsc->getSCE(type)) ){
				i = sc_start2(src,bl,type,rate,skill_lv,src->id,(src == bl)?5000:(bl->type == BL_PC)?skill_get_time(skill_id,skill_lv):skill_get_time2(skill_id, skill_lv));
				clif_skill_nodamage(src,*bl,skill_id,skill_lv,i);
				if( sd && !i )
					clif_skill_fail( *sd, skill_id );
			}
		}else
		if( sd )
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_TOTARGET );
		break;

	case NPC_JACKFROST:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		map_foreachinrange(skill_area_sub,bl,skill_get_splash(skill_id,skill_lv),BL_CHAR|BL_SKILL,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;

	case WL_SIENNAEXECRATE:
		if( status_isimmune(bl) || !tsc )
			break;

		if( flag&1 ) {
			if( bl->id == skill_area_temp[1] )
				break; // Already work on this target

			status_change_start(src,bl,type,10000,skill_lv,src->id,0,0,skill_get_time2(skill_id,skill_lv), SCSTART_NOTICKDEF, skill_get_time(skill_id, skill_lv));
		} else {
			int rate = 45 + 5 * skill_lv + ( sd? sd->status.job_level : 50 ) / 4;
			// IroWiki says Rate should be reduced by target stats, but currently unknown
			if( rnd()%100 < rate ) { // Success on First Target
				if( status_change_start(src,bl,type,10000,skill_lv,src->id,0,0,skill_get_time2(skill_id,skill_lv), SCSTART_NOTICKDEF, skill_get_time(skill_id, skill_lv)) ) {
					clif_skill_nodamage(src,*bl,skill_id,skill_lv);
					skill_area_temp[1] = bl->id;
					map_foreachinallrange(skill_area_sub,bl,skill_get_splash(skill_id,skill_lv),BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_nodamage_id);
				}
				// Doesn't send failure packet if it fails on defense.
			}
			else if( sd ) // Failure on Rate
				clif_skill_fail( *sd, skill_id );
		}
		break;

	case WL_SUMMONFB:
	case WL_SUMMONBL:
	case WL_SUMMONWB:
	case WL_SUMMONSTONE:
		{
			if (sc == nullptr)
				break;

			e_wl_spheres element;

			switch (skill_id) { // Set val2. The SC element for this ball
				case WL_SUMMONFB:
					element = WLS_FIRE;
					break;
				case WL_SUMMONBL:
					element = WLS_WIND;
					break;
				case WL_SUMMONWB:
					element = WLS_WATER;
					break;
				case WL_SUMMONSTONE:
					element = WLS_STONE;
					break;
			}

			if (skill_lv == 1) {
				sc_type sphere = SC_NONE;

				for (i = SC_SPHERE_1; i <= SC_SPHERE_5; i++) {
					if (sc->getSCE(i) == nullptr) {
						sphere = static_cast<sc_type>(i); // Take the free SC
						break;
					}
				}

				if (sphere == SC_NONE) {
					if (sd) // No free slots to put SC
						clif_skill_fail( *sd, skill_id, USESKILL_FAIL_SUMMON );
					break;
				}

				sc_start2(src, src, sphere, 100, element, skill_lv, skill_get_time(skill_id, skill_lv));
			} else {
				for (i = SC_SPHERE_1; i <= SC_SPHERE_5; i++) {
					status_change_end(src, static_cast<sc_type>(i)); // Removes previous type
					sc_start2(src, src, static_cast<sc_type>(i), 100, element, skill_lv, skill_get_time(skill_id, skill_lv));
				}
			}

			clif_skill_nodamage(src, *bl, skill_id, 0, false);
		}
		break;

	case WL_READING_SB_READING:
		if (sd) {
			if (pc_checkskill(sd, WL_READING_SB) == 0 || skill_lv < 1 || skill_lv > 10) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_SPELLBOOK_READING );
				break;
			}

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			skill_spellbook(*sd, ITEMID_WL_MB_SG + skill_lv - 1);
		}
		break;

	case RA_FEARBREEZE:
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src,bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		break;

	case RA_WUGMASTERY:
		if( sd ) {
			if( !pc_iswug(sd) )
				pc_setoption(sd,sd->sc.option|OPTION_WUG);
			else
				pc_setoption(sd,sd->sc.option&~OPTION_WUG);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
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
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case RA_WUGDASH:
		if( tsce ) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,status_change_end(bl, type));
			map_freeblock_unlock();
			return 0;
		}
		if( sd && pc_isridingwug(sd) ) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start4(src,bl,type,100,skill_lv,unit_getdir(bl),0,0,0));
			clif_walkok(*sd);
		}
		break;

	case RA_SENSITIVEKEEN:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		clif_skill_damage(src,src,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		map_foreachinrange(skill_area_sub,src,skill_get_splash(skill_id,skill_lv),BL_CHAR|BL_SKILL,src,skill_id,skill_lv,tick,flag|BCT_ENEMY,skill_castend_damage_id);
		break;

	case NC_F_SIDESLIDE:
	case NC_B_SIDESLIDE:
		{
			uint8 dir = (skill_id == NC_F_SIDESLIDE) ? (unit_getdir(src)+4)%8 : unit_getdir(src);
			skill_blown(src,bl,skill_get_blewcount(skill_id,skill_lv),dir,BLOWN_IGNORE_NO_KNOCKBACK);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case NC_SELFDESTRUCTION:
		if( sd ) {
			if( pc_ismadogear(sd) )
				pc_setmadogear(sd, false);
			skill_area_temp[1] = 0;
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
			status_set_sp(src, 0, 0);
			skill_clear_unitgroup(src);
		}
		break;

	case NC_EMERGENCYCOOL:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		if (sd) {
			struct s_skill_condition req = skill_get_requirement(sd, skill_id, skill_lv);
			int16 limit[] = { -45, -75, -105 };

			i = 0;
			for (const auto &reqItem : req.eqItem) {
				if (pc_search_inventory(sd, reqItem) != -1)
					break;
				i++;
			}
			pc_overheat(*sd, limit[min(i, 2)]);
		}
		break;

	case NC_ANALYZE:
		clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		sc_start(src,bl,type, 30 + 12 * skill_lv,skill_lv,skill_get_time(skill_id,skill_lv));
		break;

	case NC_MAGNETICFIELD:
		if (flag & 1) {
			sc_start2(src, bl, SC_MAGNETICFIELD, 100, skill_lv, src->id, skill_get_time(skill_id, skill_lv));
		} else {
			if (map_flag_vs(src->m)) // Doesn't affect the caster in non-PVP maps [exneval]
				sc_start2(src, bl, type, 100, skill_lv, src->id, skill_get_time(skill_id, skill_lv));
			map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_nodamage_id);
			clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		}
		break;

	case NC_REPAIR:
		if( sd ) {
			int heal, hp = 0;
			if( !dstsd || !pc_ismadogear(dstsd) ) {
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_TOTARGET );
				break;
			}
			switch(skill_lv) {
				case 1: hp = 4; break;
				case 2: hp = 7; break;
				case 3: hp = 13; break;
				case 4: hp = 17; break;
				case 5: default: hp = 23; break;
			}
			heal = dstsd->status.max_hp * hp / 100;
			status_heal(bl,heal,0,2);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, heal != 0);
		}
		break;

	case NC_DISJOINT:
		{
			if( bl->type != BL_MOB ) break;
			md = map_id2md(bl->id);
			if( md && md->mob_id >= MOBID_SILVERSNIPER && md->mob_id <= MOBID_MAGICDECOY_WIND )
				status_kill(bl);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;
	case SC_AUTOSHADOWSPELL:
		if( sd ) {
			if( (sd->reproduceskill_idx > 0 && sd->status.skill[sd->reproduceskill_idx].id) ||
				(sd->cloneskill_idx > 0 && sd->status.skill[sd->cloneskill_idx].id) )
			{
				sc_start(src,src,SC_STOP,100,skill_lv,INFINITE_TICK);// The skill_lv is stored in val1 used in skill_select_menu to determine the used skill lvl [Xazax]
				clif_autoshadowspell_list( *sd );
				clif_skill_nodamage(src,*bl,skill_id,1);
			}
			else
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_IMITATION_SKILL_NONE );
		}
		break;

	case SC_SHADOWFORM:
		if( sd && dstsd && src != bl && !dstsd->shadowform_id ) {
			if( clif_skill_nodamage(src,*bl,skill_id,skill_lv,sc_start4(src,src,type,100,skill_lv,bl->id,4+skill_lv,0,skill_get_time(skill_id, skill_lv))) )
				dstsd->shadowform_id = src->id;
		}
		else if( sd )
			clif_skill_fail( *sd, skill_id );
		break;

	case SC_BODYPAINT:
		if( flag&1 ) {
			if (tsc && ((tsc->option&(OPTION_HIDE|OPTION_CLOAK)) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC_STEALTHFIELD))) {
				status_change_end(bl,SC_HIDING);
				status_change_end(bl,SC_CLOAKING);
				status_change_end(bl,SC_CLOAKINGEXCEED);
				status_change_end(bl,SC_CAMOUFLAGE);
				status_change_end(bl,SC_NEWMOON);
				if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
					status_change_end(bl, SC__SHADOWFORM);
			}
			// Attack Speed decrease and Blind happen to everyone around caster, not just hidden targets.
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			sc_start(src, bl, SC_BLIND, 53 + 2 * skill_lv, skill_lv, skill_get_time2(skill_id, skill_lv));
		} else {
			clif_skill_nodamage(src, *bl, skill_id, 0);
			map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR,
				src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		}
		break;

	case SC_ENERVATION:
	case SC_GROOMY:
	case SC_LAZINESS:
	case SC_UNLUCKY:
	case SC_WEAKNESS:
		if( !(tsc && tsc->getSCE(type)) ) {
			int rate;

			if (status_get_class_(bl) == CLASS_BOSS)
				break;
			rate = status_get_lv(src) / 10 + rnd_value(sstatus->dex / 12, sstatus->dex / 4) + ( sd ? sd->status.job_level : 50 ) + 10 * skill_lv
					   - (status_get_lv(bl) / 10 + rnd_value(tstatus->agi / 6, tstatus->agi / 3) + tstatus->luk / 10 + ( dstsd ? (dstsd->max_weight / 10 - dstsd->weight / 10 ) / 100 : 0));
			rate = cap_value(rate, skill_lv + sstatus->dex / 20, 100);
			clif_skill_nodamage(src,*bl,skill_id,0,sc_start(src,bl,type,rate,skill_lv,skill_get_time(skill_id,skill_lv)));
		} else if( sd )
			 clif_skill_fail( *sd, skill_id );
		break;

	case SC_IGNORANCE:
		if( !(tsc && tsc->getSCE(type)) ) {
			int rate;

			if (status_get_class_(bl) == CLASS_BOSS)
				break;
			rate = status_get_lv(src) / 10 + rnd_value(sstatus->dex / 12, sstatus->dex / 4) + ( sd ? sd->status.job_level : 50 ) + 10 * skill_lv
					   - (status_get_lv(bl) / 10 + rnd_value(tstatus->agi / 6, tstatus->agi / 3) + tstatus->luk / 10 + ( dstsd ? (dstsd->max_weight / 10 - dstsd->weight / 10 ) / 100 : 0));
			rate = cap_value(rate, skill_lv + sstatus->dex / 20, 100);
			if (clif_skill_nodamage(src,*bl,skill_id,0,sc_start(src,bl,type,rate,skill_lv,skill_get_time(skill_id,skill_lv)))) {
				int sp = 100 * skill_lv;

				if( dstmd )
					sp = dstmd->level;
				if( !dstmd )
					status_zap(bl, 0, sp);

				status_heal(src, 0, sp / 2, 3);
			} else if( sd )
				clif_skill_fail( *sd, skill_id );
		} else if( sd )
			clif_skill_fail( *sd, skill_id );
		break;

	case LG_TRAMPLE:
		clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		if (rnd()%100 < (25 + 25 * skill_lv))
			map_foreachinallrange(skill_destroy_trap,bl,skill_get_splash(skill_id,skill_lv),BL_SKILL,tick);
		status_change_end(bl, SC_SV_ROOTTWIST);
		break;

	case LG_REFLECTDAMAGE:
		if( tsc && tsc->getSCE(type) )
			status_change_end(bl,type);
		else
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

	case LG_PIETY:
		if( flag&1 )
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		else {
			skill_area_temp[2] = 0;
			map_foreachinallrange(skill_area_sub,bl,skill_get_splash(skill_id,skill_lv),BL_PC,src,skill_id,skill_lv,tick,flag|SD_PREAMBLE|BCT_PARTY|BCT_SELF|1,skill_castend_nodamage_id);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case SR_CURSEDCIRCLE:
		if( flag&1 ) {
			if( status_get_class_(bl) == CLASS_BOSS )
				break;
			if( sc_start2(src,bl, type, 100, skill_lv, src->id, skill_get_time(skill_id, skill_lv))) {
				if( bl->type == BL_MOB )
					mob_unlocktarget((TBL_MOB*)bl,gettick());
				clif_bladestop(src, bl->id, 1);
				map_freeblock_unlock();
				return 1;
			}
		} else {
			int count = 0;
			clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			count = map_forcountinrange(skill_area_sub, src, skill_get_splash(skill_id,skill_lv), (sd)?sd->spiritball_old:15, // Assume 15 spiritballs in non-charactors
				BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			if( sd ) pc_delspiritball(sd, count, 0);
			clif_skill_nodamage(src, *src, skill_id, skill_lv,
				sc_start2(src,src, SC_CURSEDCIRCLE_ATKER, 100, skill_lv, count, skill_get_time(skill_id,skill_lv)));
		}
		break;

	case NPC_SR_CURSEDCIRCLE:
		if( flag&1 ) {
			if( status_get_class_(bl) == CLASS_BOSS )
				break;
			if( sc_start2(src,bl, type, 50, skill_lv, src->id, skill_get_time(skill_id, skill_lv))) {
				if( bl->type == BL_MOB )
					mob_unlocktarget((TBL_MOB*)bl,gettick());
				clif_bladestop(src, bl->id, 1);
				map_freeblock_unlock();
				return 1;
			}
		} else {
			int count = 0;
			clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			count = map_forcountinrange(skill_area_sub, src, skill_get_splash(skill_id,skill_lv), (sd)?sd->spiritball_old:15, // Assume 15 spiritballs in non-charactors
				BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			if( sd ) pc_delspiritball(sd, count, 0);
			clif_skill_nodamage(src, *src, skill_id, skill_lv,
				sc_start2(src,src, SC_CURSEDCIRCLE_ATKER, 50, skill_lv, count, skill_get_time(skill_id,skill_lv)));
		}
		break;

	case SR_RAISINGDRAGON:
		if( sd ) {
			short max = 5 + skill_lv;
			sc_start(src,bl, SC_EXPLOSIONSPIRITS, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			for( i = 0; i < max; i++ ) // Don't call more than max available spheres.
				pc_addspiritball(sd, skill_get_time(skill_id, skill_lv), max);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src,bl, type, 100, skill_lv,skill_get_time(skill_id, skill_lv)));
		}
		break;

	case SR_ASSIMILATEPOWER:
		if (flag&1) {
			i = 0;
			if (dstsd && (sd == dstsd || map_flag_vs(src->m)) && (dstsd->class_&MAPID_BASEMASK)!=MAPID_GUNSLINGER) {
				if (dstsd->spiritball > 0) {
					i = dstsd->spiritball;
					pc_delspiritball(dstsd,dstsd->spiritball,0);
				}
				if (dstsd->spiritcharm_type != CHARM_TYPE_NONE && dstsd->spiritcharm > 0) {
					i += dstsd->spiritcharm;
					pc_delspiritcharm(dstsd,dstsd->spiritcharm,dstsd->spiritcharm_type);
				}
			}
			if (i)
				status_percent_heal(src, 0, i);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, i != 0);
		} else {
			clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), splash_target(src), src, skill_id, skill_lv, tick, flag|BCT_ENEMY|BCT_SELF|SD_SPLASH|1, skill_castend_nodamage_id);
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
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		break;

	case SR_GENTLETOUCH_CURE:
		{
			unsigned int heal;

			if (dstmd && (dstmd->mob_id == MOBID_EMPERIUM || status_get_class_(bl) == CLASS_BATTLEFIELD))
				heal = 0;
			else {
				heal = (120 * skill_lv) + (status_get_max_hp(bl) * skill_lv / 100);
				status_heal(bl, heal, 0, 0);
			}

			if( tsc && tsc->count && rnd_chance( ( skill_lv * 5 + ( status_get_dex( src ) + status_get_lv( src ) ) / 4 ) - rnd_value( 1, 10 ), 100 ) ){
				status_change_end(bl, SC_STONE);
				status_change_end(bl, SC_FREEZE);
				status_change_end(bl, SC_STUN);
				status_change_end(bl, SC_POISON);
				status_change_end(bl, SC_SILENCE);
				status_change_end(bl, SC_BLIND);
				status_change_end(bl, SC_HALLUCINATION);
			}

			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;
	case SR_GENTLETOUCH_ENERGYGAIN:
	case SR_GENTLETOUCH_CHANGE:
	case SR_GENTLETOUCH_REVITALIZE:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		break;
	case SR_FLASHCOMBO: {
		const int combo[] = { SR_DRAGONCOMBO, SR_FALLENEMPIRE, SR_TIGERCANNON };
		const int delay[] = { 0, 750, 1250 };

		if (sd) // Disable attacking/acting/moving for skill's duration.
			sd->ud.attackabletime = sd->canuseitem_tick = sd->ud.canact_tick = tick + delay[2];
		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,src,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		for (i = 0; i < ARRAYLENGTH(combo); i++)
			skill_addtimerskill(src,tick + delay[i],bl->id,0,0,combo[i],skill_lv,BF_WEAPON,flag|SD_LEVEL);
	}
	break;

	case WA_SWING_DANCE:
	case WA_MOONLIT_SERENADE:
	case WA_SYMPHONY_OF_LOVER:
	case MI_RUSH_WINDMILL:
	case MI_ECHOSONG:
		if( !sd || !sd->status.party_id || (flag & 1) ) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			sc_start2(src,bl,type,100,skill_lv,((sd) ? pc_checkskill(sd,WM_LESSON) : skill_get_max(WM_LESSON)),skill_get_time(skill_id,skill_lv));
		} else if( sd ) {
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
			sc_start2(src,bl,type,100,skill_lv,((sd) ? pc_checkskill(sd,WM_LESSON) : skill_get_max(WM_LESSON)),skill_get_time(skill_id,skill_lv));
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case MI_HARMONIZE:
		if( src != bl )
			clif_skill_nodamage(src, *src, skill_id, skill_lv, sc_start(src,src, type, 100, skill_lv, skill_get_time(skill_id,skill_lv)));
		clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src,bl, type, 100, skill_lv, skill_get_time(skill_id,skill_lv)));
		break;

	case WM_DEADHILLHERE:
		if( bl->type == BL_PC ) {
			if( !status_isdead(*bl) )
				break;

			tstatus->hp = max(tstatus->sp, 1);
			tstatus->sp -= tstatus->sp * ( 60 - 10 * skill_lv ) / 100;
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			pc_revive(reinterpret_cast<map_session_data*>(bl),true,true);
			clif_resurrection( *bl );
		}
		break;

	case WM_VOICEOFSIREN:
		if (flag&1)
			sc_start2(src,bl,type,skill_area_temp[5],skill_lv,src->id,skill_area_temp[6]);
		else {
			// Success chance: (Skill Level x 6) + (Voice Lesson Skill Level x 2) + (Caster's Job Level / 2) %
			skill_area_temp[5] = skill_lv * 6 + ((sd) ? pc_checkskill(sd, WM_LESSON) : 1) * 2 + (sd ? sd->status.job_level : 50) / 2;
			skill_area_temp[6] = skill_get_time(skill_id,skill_lv);
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id,skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag|BCT_ALL|BCT_WOS|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case WM_GLOOMYDAY:
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		if( dstsd && ( pc_checkskill(dstsd,KN_BRANDISHSPEAR) || pc_checkskill(dstsd,LK_SPIRALPIERCE) ||
				pc_checkskill(dstsd,CR_SHIELDCHARGE) || pc_checkskill(dstsd,CR_SHIELDBOOMERANG) ||
				pc_checkskill(dstsd,PA_SHIELDCHAIN) || pc_checkskill(dstsd,LG_SHIELDPRESS) ) )
			{ // !TODO: Which skills aren't boosted anymore?
				sc_start(src,bl,SC_GLOOMYDAY_SK,100,skill_lv,skill_get_time(skill_id,skill_lv));
				break;
			}
		sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		break;

	case WM_SATURDAY_NIGHT_FEVER:
		if( flag&1 ) {
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		} else if (sd) {
			if( rnd()%100 < sstatus->int_ / 6 + sd->status.job_level / 5 + skill_lv * 4 + pc_checkskill(sd, WM_LESSON) ) { // !TODO: What's the Lesson bonus?
				map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id,skill_lv),BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			}
		}
		break;

	case WM_SIRCLEOFNATURE:
	case WM_SONG_OF_MANA:
	case WM_DANCE_WITH_WUG:
	case WM_LERADS_DEW:
	case WM_UNLIMITED_HUMMING_VOICE:
		if( flag&1 ) {	// These affect to to all party members near the caster.
			if( sc && sc->getSCE(type) ) {
				sc_start2(src,bl,type,100,skill_lv,pc_checkskill(sd, WM_LESSON),skill_get_time(skill_id,skill_lv));
			}
		} else if( sd ) {
			if( sc_start2(src,bl,type,100,skill_lv,pc_checkskill(sd, WM_LESSON),skill_get_time(skill_id,skill_lv)) )
				party_foreachsamemap(skill_area_sub,sd,skill_get_splash(skill_id,skill_lv),src,skill_id,skill_lv,tick,flag|BCT_PARTY|1,skill_castend_nodamage_id);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case WM_MELODYOFSINK:
		if( flag&1 ) {
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		} else {	// These affect to all targets around the caster.
			if( rnd()%100 < 5 + 5 * skill_lv + pc_checkskill(sd, WM_LESSON) ) { // !TODO: What's the Lesson bonus?
				map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id,skill_lv),BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			}
		}
		break;

	case WM_BEYOND_OF_WARCRY:
		if( flag&1 ) {
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
		} else {	// These affect to all targets around the caster.
			if( rnd()%100 < 12 + 3 * skill_lv + (sd ? pc_checkskill(sd, WM_LESSON) : 0) ) { // !TODO: What's the Lesson bonus?
				map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id,skill_lv),BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			}
		}
		break;

	case WM_SOUND_OF_DESTRUCTION:
		if (flag&1) {
			sc_start(src, bl, type, 100, skill_lv, (sd ? pc_checkskill(sd, WM_LESSON) * 500 : 0) + skill_get_time(skill_id, skill_lv)); // !TODO: Confirm Lesson increase
		} else {
			map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv),BL_PC, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case WM_RANDOMIZESPELL:
		if (rnd() % 100 < 30 + (10 * skill_lv)) {
			status_change_end(bl, SC_SONGOFMANA);
			status_change_end(bl, SC_DANCEWITHWUG);
			status_change_end(bl, SC_LERADSDEW);
			status_change_end(bl, SC_SATURDAYNIGHTFEVER);
			status_change_end(bl, SC_BEYONDOFWARCRY);
			status_change_end(bl, SC_MELODYOFSINK);
			status_change_end(bl, SC_BEYONDOFWARCRY);
			status_change_end(bl, SC_UNLIMITEDHUMMINGVOICE);

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case RETURN_TO_ELDICASTES:
	case ALL_GUARDIAN_RECALL:
	case ECLAGE_RECALL:
	case ALL_PRONTERA_RECALL:
	case ALL_GLASTHEIM_RECALL:
	case ALL_THANATOS_RECALL:
	case ALL_LIGHTHALZEN_RECALL:
		if( sd != nullptr ){
			short x=0, y=0; // Destination position.
			unsigned short mapindex=0;

			switch(skill_id){
			default:
			case RETURN_TO_ELDICASTES:
				x = 198;
				y = 187;
				mapindex  = mapindex_name2id(MAP_DICASTES);
				break;
			case ALL_GUARDIAN_RECALL:
				x = 44;
				y = 151;
				mapindex  = mapindex_name2id(MAP_MORA);
				break;
			case ECLAGE_RECALL:
				x = 47;
				y = 31;
				mapindex  = mapindex_name2id(MAP_ECLAGE_IN);
				break;
			case ALL_PRONTERA_RECALL:
				if(skill_lv == 1) {
					x = 115;
					y = 72;
				}
				else if(skill_lv == 2) {
					x = 159;
					y = 192;
				}
				mapindex  = mapindex_name2id(MAP_PRONTERA);
				break;
			case ALL_GLASTHEIM_RECALL:
				x = 200;
				y = 268;
				mapindex  = mapindex_name2id(MAP_GLASTHEIM);
				break;
			case ALL_THANATOS_RECALL:
				x = 139;
				y = 156;
				mapindex  = mapindex_name2id(MAP_THANATOS);
				break;
			case ALL_LIGHTHALZEN_RECALL:
				x = 307;
				y = 307;
				mapindex  = mapindex_name2id(MAP_LIGHTHALZEN);
				break;
			}

			if(!mapindex)
			{ //Given map not found?
				clif_skill_fail( *sd, skill_id );
				map_freeblock_unlock();
				return 0;
			}
			pc_setpos(sd, mapindex, x, y, CLR_TELEPORT);
		}
		break;

	case ECL_SNOWFLIP:
	case ECL_PEONYMAMY:
	case ECL_SADAGUI:
	case ECL_SEQUOIADUST:
		switch(skill_id){
		case ECL_SNOWFLIP:
			status_change_end(bl, SC_SLEEP);
			status_change_end(bl, SC_BLEEDING);
			status_change_end(bl, SC_BURNING);
			status_change_end(bl, SC_DEEPSLEEP);
			break;
		case ECL_PEONYMAMY:
			status_change_end(bl, SC_FREEZE);
			status_change_end(bl, SC_FREEZING);
			status_change_end(bl, SC_CRYSTALIZE);
			break;
		case ECL_SADAGUI:
			status_change_end(bl, SC_STUN);
			status_change_end(bl, SC_CONFUSION);
			status_change_end(bl, SC_HALLUCINATION);
			status_change_end(bl, SC_FEAR);
			break;
		case ECL_SEQUOIADUST:
			status_change_end(bl, SC_STONE);
			status_change_end(bl, SC_POISON);
			status_change_end(bl, SC_CURSE);
			status_change_end(bl, SC_BLIND);
			status_change_end(bl, SC_ORCISH);
			status_change_end(bl, SC_DECREASEAGI);
			break;
		}
		clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, 1, DMG_SINGLE);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		break;

	case GM_SANDMAN:
		if( tsc ) {
			if( tsc->opt1 == OPT1_SLEEP )
				tsc->opt1 = 0;
			else
				tsc->opt1 = OPT1_SLEEP;
			clif_changeoption(bl);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case SO_ARRULLO:
		{
			int rate = (15 + 5 * skill_lv) + status_get_int(src) / 5 + (sd ? sd->status.job_level / 5 : 0) - status_get_int(bl) / 6 - status_get_luk(bl) / 10;

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			sc_start(src, bl, type, rate, skill_lv, skill_get_time(skill_id, skill_lv));
		}
		break;

	case WM_LULLABY_DEEPSLEEP:
		if (flag&1) {
			int rate = 4 * skill_lv + (sd ? pc_checkskill(sd, WM_LESSON) * 2 : 0) + status_get_lv(src) / 15 + (sd ? sd->status.job_level / 5 : 0);
			int duration = skill_get_time(skill_id, skill_lv) - (status_get_base_status(bl)->int_ * 50 + status_get_lv(bl) * 50); // Duration reduction for Deep Sleep Lullaby is doubled

			sc_start(src, bl, type, rate, skill_lv, duration);
		} else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinallrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		}
		break;

	case SO_SUMMON_AGNI:
	case SO_SUMMON_AQUA:
	case SO_SUMMON_VENTUS:
	case SO_SUMMON_TERA:
		if( sd ) {
			int elemental_class = skill_get_elemental_type(skill_id,skill_lv);

			// Remove previous elemental first.
			if( sd->ed )
				elemental_delete(sd->ed);

			// Summoning the new one.
			if( !elemental_create(sd,elemental_class,skill_get_time(skill_id,skill_lv)) ) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case SO_EL_CONTROL:
		if( sd ) {
			int mode;

			if( !sd->ed )	break;

			if( skill_lv == 4 ) {// At level 4 delete elementals.
				elemental_delete(sd->ed);
				break;
			}
			switch( skill_lv ) {// Select mode bassed on skill level used.
				case 1: mode = EL_MODE_PASSIVE; break; // Standard mode.
				case 2: mode = EL_MODE_ASSIST; break;
				case 3: mode = EL_MODE_AGGRESSIVE; break;
			}
			if( !elemental_change_mode(sd->ed,mode) ) {
				clif_skill_fail( *sd, skill_id );
				break;
			}
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		}
		break;

	case SO_EL_ACTION:
		if( sd ) {
				int duration = 3000;
			if( !sd->ed )
				break;
			switch(sd->ed->db->class_) {
				case ELEMENTALID_AGNI_M: case ELEMENTALID_AQUA_M:
				case ELEMENTALID_VENTUS_M: case ELEMENTALID_TERA_M:
					duration = 6000;
					break;
				case ELEMENTALID_AGNI_L: case ELEMENTALID_AQUA_L:
				case ELEMENTALID_VENTUS_L: case ELEMENTALID_TERA_L:
					duration = 9000;
					break;
			}
			sd->skill_id_old = skill_id;
			elemental_action(sd->ed, bl, tick);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			skill_blockpc_start(sd, skill_id, duration);
		}
		break;

	case SO_EL_CURE:
		if( sd ) {
			s_elemental_data *ed = sd->ed;
			int s_hp, s_sp;

			if( !ed )
				break;

			s_hp = sd->battle_status.hp * 10 / 100;
			s_sp = sd->battle_status.sp * 10 / 100;

			if( !status_charge(&sd->bl,s_hp,s_sp) ) {
				clif_skill_fail( *sd, skill_id );
				break;
			}

			status_heal(&ed->bl,s_hp,s_sp,3);
			clif_skill_nodamage(src,ed->bl,skill_id,skill_lv);
		}
		break;

	case GN_CHANGEMATERIAL:
	case SO_EL_ANALYSIS:
		if( sd ) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			clif_skill_itemlistwindow(sd,skill_id,skill_lv);
		}
		break;

	case NPC_MANDRAGORA:
	case GN_MANDRAGORA:
		if( flag&1 ) {
			int rate;
			if (skill_id == NPC_MANDRAGORA)
				rate = (20 * skill_lv) - (tstatus->vit + tstatus->luk) / 5;
			else
				rate = 25 + (10 * skill_lv) - (tstatus->vit + tstatus->luk) / 5;

			if (rate < 10)
				rate = 10;
			if (bl->type == BL_MOB || (tsc && tsc->getSCE(type)))
				break; // Don't activate if target is a monster or zap SP if target already has Mandragora active.
			if (rnd()%100 < rate) {
				sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
				status_zap(bl,0,status_get_max_sp(bl) * (25 + 5 * skill_lv) / 100);
			}
		} else {
			map_foreachinallrange(skill_area_sub,bl,skill_get_splash(skill_id,skill_lv),BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_nodamage_id);
			clif_skill_nodamage(src,*src,skill_id,skill_lv);
		}
		break;
	case GN_SLINGITEM:
		if( sd ) {
			i = sd->equip_index[EQI_AMMO];
			if( i < 0 )
				break; // No ammo.
			t_itemid ammo_id = sd->inventory_data[i]->nameid;
			if( ammo_id == 0 )
				break;
			sd->itemid = ammo_id;
			if( itemdb_group.item_exists(IG_BOMB, ammo_id) ) {
				if(battle_check_target(src,bl,BCT_ENEMY) > 0) {// Only attack if the target is an enemy.
					if( ammo_id == ITEMID_PINEAPPLE_BOMB )
						map_foreachincell(skill_area_sub,bl->m,bl->x,bl->y,BL_CHAR,src,GN_SLINGITEM_RANGEMELEEATK,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
					else
						skill_attack(BF_WEAPON,src,src,bl,GN_SLINGITEM_RANGEMELEEATK,skill_lv,tick,flag);
				} else //Otherwise, it fails, shows animation and removes items.
					clif_skill_fail( *sd, GN_SLINGITEM_RANGEMELEEATK, USESKILL_FAIL );
			} else if (itemdb_group.item_exists(IG_THROWABLE, ammo_id)) {
				switch (ammo_id) {
					case ITEMID_HP_INC_POTS_TO_THROW: // MaxHP +(500 + Thrower BaseLv * 10 / 3) and heals 1% MaxHP
						sc_start2(src, bl, SC_PROMOTE_HEALTH_RESERCH, 100, 2, 1, 500000);
						status_percent_heal(bl, 1, 0);
						break;
					case ITEMID_HP_INC_POTM_TO_THROW: // MaxHP +(1500 + Thrower BaseLv * 10 / 3) and heals 2% MaxHP
						sc_start2(src, bl, SC_PROMOTE_HEALTH_RESERCH, 100, 2, 2, 500000);
						status_percent_heal(bl, 2, 0);
						break;
					case ITEMID_HP_INC_POTL_TO_THROW: // MaxHP +(2500 + Thrower BaseLv * 10 / 3) and heals 5% MaxHP
						sc_start2(src, bl, SC_PROMOTE_HEALTH_RESERCH, 100, 2, 3, 500000);
						status_percent_heal(bl, 5, 0);
						break;
					case ITEMID_SP_INC_POTS_TO_THROW: // MaxSP +(Thrower BaseLv / 10 - 5)% and recovers 2% MaxSP
						sc_start2(src, bl, SC_ENERGY_DRINK_RESERCH, 100, 2, 1, 500000);
						status_percent_heal(bl, 0, 2);
						break;
					case ITEMID_SP_INC_POTM_TO_THROW: // MaxSP +(Thrower BaseLv / 10)% and recovers 4% MaxSP
						sc_start2(src, bl, SC_ENERGY_DRINK_RESERCH, 100, 2, 2, 500000);
						status_percent_heal(bl, 0, 4);
						break;
					case ITEMID_SP_INC_POTL_TO_THROW: // MaxSP +(Thrower BaseLv / 10 + 5)% and recovers 8% MaxSP
						sc_start2(src, bl, SC_ENERGY_DRINK_RESERCH, 100, 2, 3, 500000);
						status_percent_heal(bl, 0, 8);
						break;
					default:
						if (dstsd)
							run_script(sd->inventory_data[i]->script, 0, dstsd->bl.id, fake_nd->bl.id);
						break;
				}
			}
		}
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);// This packet is received twice actually, I think it is to show the animation.
		break;
	case GN_MIX_COOKING:
	case GN_MAKEBOMB:
	case GN_S_PHARMACY:
		if( sd ) {
			int qty = 1;
			sd->skill_id_old = skill_id;
			sd->skill_lv_old = skill_lv;
			if( skill_id != GN_S_PHARMACY && skill_lv > 1 )
				qty = 10;
			clif_cooking_list( *sd, ( skill_id - GN_MIX_COOKING ) + 27, skill_id, qty, skill_id == GN_MAKEBOMB ? 5 : 6 );
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
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
	case EL_WIND_STEP:
	case EM_EL_FLAMETECHNIC:
	case EM_EL_FLAMEARMOR:
	case EM_EL_COLD_FORCE:
	case EM_EL_CRYSTAL_ARMOR:
	case EM_EL_GRACE_BREEZE:
	case EM_EL_EYES_OF_STORM:
	case EM_EL_EARTH_CARE:
	case EM_EL_STRONG_PROTECTION:
	case EM_EL_DEEP_POISONING:
	case EM_EL_POISON_SHIELD:
	{
			s_elemental_data *ele = BL_CAST(BL_ELEM, src);
			if( ele ) {
				sc_type type2 = (sc_type)(type-1);
				status_change *esc = status_get_sc(&ele->bl);

				if( (esc && esc->getSCE(type2)) || (tsc && tsc->getSCE(type)) ) {
					status_change_end(src,type);
					status_change_end(bl,type2);
				} else {
					clif_skill_nodamage(src,*src,skill_id,skill_lv);
					if (!(skill_id >= EM_EL_FLAMETECHNIC && skill_id <= EM_EL_DEADLY_POISON))
						clif_skill_damage(src, ( skill_id == EL_GUST || skill_id == EL_BLAST || skill_id == EL_WILD_STORM )?src:bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
					if( skill_id == EL_WIND_STEP )	// There aren't teleport, just push the master away.
						skill_blown(src,bl,(rnd()%skill_get_blewcount(skill_id,skill_lv))+1,rnd()%8,BLOWN_NONE);
					sc_start(src,src,type2,100,skill_lv,skill_get_time(skill_id,skill_lv));
					sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv));
				}
			}
		}
		break;
	case EL_FIRE_MANTLE:
	case EL_WATER_BARRIER:
	case EL_ZEPHYR:
	case EL_POWER_OF_GAIA:
		clif_skill_damage(src, bl, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		skill_unitsetting(src,skill_id,skill_lv,bl->x,bl->y,0);
		break;
	case EL_WATER_SCREEN: {
			s_elemental_data *ele = BL_CAST(BL_ELEM, src);
			if( ele ) {
				status_change *esc = status_get_sc(&ele->bl);
				sc_type type2 = (sc_type)(type-1);

				clif_skill_nodamage(src,*src,skill_id,skill_lv);
				if( (esc && esc->getSCE(type2)) || (tsc && tsc->getSCE(type)) ) {
					status_change_end(bl,type);
					status_change_end(src,type2);
				} else {
					// This not heals at the end.
					clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
					sc_start(src,src,type2,100,skill_lv,skill_get_time(skill_id,skill_lv));
					sc_start(src,bl,type,100,src->id,skill_get_time(skill_id,skill_lv));
				}
			}
		}
		break;	

	case KO_KAHU_ENTEN:
	case KO_HYOUHU_HUBUKI:
	case KO_KAZEHU_SEIRAN:
	case KO_DOHU_KOUKAI:
		if (sd) {
			int ele_type = skill_get_ele(skill_id,skill_lv);
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			pc_addspiritcharm(sd,skill_get_time(skill_id,skill_lv),MAX_SPIRITCHARM,ele_type);
		}
		break;
	case KO_ZANZOU:
		if(sd){
			struct mob_data *md2;

			md2 = mob_once_spawn_sub(src, src->m, src->x, src->y, status_get_name(src), MOBID_ZANZOU, "", SZ_SMALL, AI_NONE);
			if( md2 )
			{
				md2->master_id = src->id;
				md2->special_state.ai = AI_ZANZOU;
				if( md2->deletetimer != INVALID_TIMER )
					delete_timer(md2->deletetimer, mob_timer_delete);
				md2->deletetimer = add_timer (gettick() + skill_get_time(skill_id, skill_lv), mob_timer_delete, md2->bl.id, 0);
				mob_spawn( md2 );
				map_foreachinallrange(unit_changetarget, src, AREA_SIZE, BL_MOB, src, &md2->bl);
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
				skill_blown(src,bl,skill_get_blewcount(skill_id,skill_lv),unit_getdir(bl),BLOWN_NONE);
			}
		}
		break;

	case KO_KYOUGAKU:
		if( dstsd && tsc && !tsc->getSCE(type) && rnd()%100 < tstatus->int_/2 ){
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,
				sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		}else if( sd )
			clif_skill_fail( *sd, skill_id );
		break;
	case KO_JYUSATSU:
		if( dstsd && tsc && !tsc->getSCE(type) &&
			rnd()%100 < ((45+5*skill_lv) + skill_lv*5 - status_get_int(bl)/2) ){//[(Base chance of success) + (Skill Level x 5) - (int / 2)]%.
			clif_skill_nodamage(src,*bl,skill_id,skill_lv,
				status_change_start(src,bl,type,10000,skill_lv,0,0,0,skill_get_time(skill_id,skill_lv),SCSTART_NOAVOID|SCSTART_NOTICKDEF));
			status_percent_damage(src, bl, tstatus->hp * skill_lv * 5, 0, false); // Does not kill the target.
			if( status_get_lv(bl) <= status_get_lv(src) )
				status_change_start(src,bl,SC_COMA,10,skill_lv,0,src->id,0,0,SCSTART_NONE);
		}else if( sd )
			clif_skill_fail( *sd, skill_id );
		break;
	case KO_GENWAKU:
		if ((dstsd || dstmd) && !status_has_mode(tstatus,MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC) && battle_check_target(src,bl,BCT_ENEMY) > 0) {
			int x = src->x, y = src->y;

			if (sd && rnd()%100 > ((45+5*skill_lv) - status_get_int(bl)/10)) { //[(Base chance of success) - (Intelligence Objectives / 10)]%.
				clif_skill_fail( *sd, skill_id );
				break;
			}

			// Confusion is still inflicted (but rate isn't reduced), no matter map type.
			status_change_start(src, src, SC_CONFUSION, 2500, skill_lv, 0, 0, 0, skill_get_time(skill_id, skill_lv), SCSTART_NORATEDEF);
			status_change_start(src, bl, SC_CONFUSION, 7500, skill_lv, 0, 0, 0, skill_get_time(skill_id, skill_lv), SCSTART_NORATEDEF);

			if (skill_check_unit_movepos(5,src,bl->x,bl->y,0,0)) {
				clif_skill_nodamage(src, *src, skill_id, skill_lv);
				clif_blown(src);
				if (!unit_blown_immune(bl, 0x1)) {
					unit_movepos(bl,x,y,0,0);
					if (bl->type == BL_PC && pc_issit((TBL_PC*)bl))
						clif_sitting(*bl); //Avoid sitting sync problem
					clif_blown(bl);
					map_foreachinallrange(unit_changetarget, src, AREA_SIZE, BL_CHAR, src, bl);
				}
			}
		}
		break;
	case OB_AKAITSUKI:
	case OB_OBOROGENSOU:
		if( sd && ( (skill_id == OB_OBOROGENSOU && bl->type == BL_MOB) // This skill does not work on monsters.
			|| status_bl_has_mode(bl,MD_STATUSIMMUNE) ) ){ // Does not work on status immune monsters.
			clif_skill_fail( *sd, skill_id );
			break;
		}
		[[fallthrough]];
	case KO_IZAYOI:
	case OB_ZANGETSU:
	case KG_KYOMU:
	case KG_KAGEMUSYA:
	case SP_SOULDIVISION:
		if (skill_id == SP_SOULDIVISION) { // Usable only on other players.
			if (bl->type != BL_PC) {
				if (sd)
					clif_skill_fail( *sd, skill_id );
				break;
			}
		}

		clif_skill_nodamage(src,*bl,skill_id,skill_lv,
			sc_start(src,bl,type,100,skill_lv,skill_get_time(skill_id,skill_lv)));
		clif_skill_damage(src,bl,tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		break;
	case KG_KAGEHUMI:
		if( flag&1 ){
			if (bl->type != BL_PC)
				break;
			if (tsc && (tsc->option & (OPTION_CLOAK | OPTION_HIDE) || tsc->getSCE(SC_CAMOUFLAGE) || tsc->getSCE(SC__SHADOWFORM) || tsc->getSCE(SC_MARIONETTE) || tsc->getSCE(SC_HARMONIZE))) {
					status_change_end(bl, SC_HIDING);
					status_change_end(bl, SC_CLOAKING);
					status_change_end(bl, SC_CLOAKINGEXCEED);
					status_change_end(bl, SC_CAMOUFLAGE);
					status_change_end(bl, SC_NEWMOON);
					if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
						status_change_end(bl, SC__SHADOWFORM);
					status_change_end(bl, SC_MARIONETTE);
					status_change_end(bl, SC_HARMONIZE);
					sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			}
		}else{
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR|BL_SKILL, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case MH_SILENT_BREEZE:
		{
			int heal = 5 * status_get_lv(&hd->bl) +
#ifdef RENEWAL
						status_base_matk_min(bl, &hd->battle_status, status_get_lv(&hd->bl));
#else
						status_base_matk_min(&hd->battle_status);
#endif
			//Silences the homunculus and target
			status_change_start(src,src,SC_SILENCE,10000,skill_lv,0,0,0,skill_get_time(skill_id,skill_lv),SCSTART_NONE);
			status_change_start(src,bl,SC_SILENCE,10000,skill_lv,0,0,0,skill_get_time(skill_id,skill_lv),SCSTART_NONE);

			//Recover the target's HP
			status_heal(bl,heal,0,3);

			//Removes these SC from target
			if (tsc) {
				const enum sc_type scs[] = {
					SC_MANDRAGORA, SC_HARMONIZE, SC_DEEPSLEEP, SC_VOICEOFSIREN, SC_SLEEP, SC_CONFUSION, SC_HALLUCINATION
				};
				for (i = 0; i < ARRAYLENGTH(scs); i++)
					if (tsc->getSCE(scs[i])) status_change_end(bl, scs[i]);
			}
		}
		break;
	case MH_OVERED_BOOST:
		if (hd && battle_get_master(src)) {
			sc_start(src, battle_get_master(src), type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		}
		break;
	case MH_GRANITIC_ARMOR:
	case MH_PYROCLASTIC:
		if(hd) {
			struct block_list *s_bl = battle_get_master(src);
			if(s_bl) sc_start2(src, s_bl, type, 100, skill_lv, hd->homunculus.level, skill_get_time(skill_id, skill_lv)); //start on master
			sc_start2(src, bl, type, 100, skill_lv, hd->homunculus.level, skill_get_time(skill_id, skill_lv));
	     }
	     break;
	case MH_LIGHT_OF_REGENE: //self
		if(hd) {
			struct block_list *s_bl = battle_get_master(src);
			if(s_bl) sc_start(src, s_bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			sc_start2(src, src, type, 100, skill_lv, hd->homunculus.level, skill_get_time(skill_id, skill_lv));
		}
		break;
	case MH_STYLE_CHANGE:
		if(hd){
			struct status_change_entry *sce;
			if((sce=hd->sc.getSCE(SC_STYLE_CHANGE))!=nullptr){ //in preparation for other bl usage
				if(sce->val1 == MH_MD_FIGHTING) sce->val1 = MH_MD_GRAPPLING;
				else sce->val1 = MH_MD_FIGHTING;
				//if(hd->master && hd->sc.getSCE(SC_STYLE_CHANGE)) { // Aegis does not show any message when switching fighting style
				//	char output[128];
				//	safesnprintf(output,sizeof(output),msg_txt(sd,378),(sce->val1==MH_MD_FIGHTING?"fighthing":"grappling"));
				//	clif_messagecolor(&hd->master->bl, color_table[COLOR_RED], output, false, SELF);
				//}
			}
			else sc_start(&hd->bl,&hd->bl, SC_STYLE_CHANGE, 100, MH_MD_FIGHTING, INFINITE_TICK);
		}
		break;
	case MH_GOLDENE_TONE:
	case MH_TEMPERING: {
		block_list* master_bl = battle_get_master(src);
		
		if (master_bl != nullptr){
			clif_skill_nodamage(src,*master_bl,skill_id,skill_lv);
			sc_start(src, master_bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		}
		} break;
	case MH_PAIN_KILLER:
		bl = battle_get_master(src);
		if (bl != nullptr)
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		break;
	case MH_MAGMA_FLOW:
	   sc_start(src,bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
	   break;
	case MH_SUMMON_LEGION: {
		int summons[5] = {MOBID_S_HORNET, MOBID_S_GIANT_HORNET, MOBID_S_GIANT_HORNET, MOBID_S_LUCIOLA_VESPA, MOBID_S_LUCIOLA_VESPA};
		int qty[5] =     {3   , 3   , 4   , 4   , 5};
		struct mob_data *sum_md;
		int i_slave,c=0;

		int maxcount = qty[skill_lv-1];
		i_slave = map_foreachinmap(skill_check_condition_mob_master_sub ,hd->bl.m, BL_MOB, hd->bl.id, summons[skill_lv-1], skill_id, &c);
		if(c >= maxcount) {
			map_freeblock_unlock();
			return 0; //max qty already spawned
		}

		for(i_slave=0; i_slave<qty[skill_lv - 1]; i_slave++){ //easy way
			sum_md = mob_once_spawn_sub(src, src->m, src->x, src->y, status_get_name(src), summons[skill_lv - 1], "", SZ_SMALL, AI_ATTACK);
			if (sum_md) {
				sum_md->master_id =  src->id;
				sum_md->special_state.ai = AI_LEGION;
				if (sum_md->deletetimer != INVALID_TIMER)
					delete_timer(sum_md->deletetimer, mob_timer_delete);
				sum_md->deletetimer = add_timer(gettick() + skill_get_time(skill_id, skill_lv), mob_timer_delete, sum_md->bl.id, 0);
				mob_spawn(sum_md); //Now it is ready for spawning.
				sc_start4(&sum_md->bl,&sum_md->bl, SC_MODECHANGE, 100, 1, 0, MD_CANATTACK|MD_AGGRESSIVE, 0, 60000);
			}
		}
		}
		break;

	case RL_RICHS_COIN:
		if (sd) {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			for (i = 0; i < 10; i++)
				pc_addspiritball(sd,skill_get_time(skill_id,skill_lv),10);
		}
		break;
	case RL_C_MARKER:
		if (sd) {
			// If marked by someone else remove it
			if (tsce && tsce->val2 != src->id)
				status_change_end(bl, type);

			// Check if marked before
			ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, sd->c_marker[i] == bl->id);
			if (i == MAX_SKILL_CRIMSON_MARKER) {
				// Find empty slot
				ARR_FIND(0, MAX_SKILL_CRIMSON_MARKER, i, !sd->c_marker[i]);
				if (i == MAX_SKILL_CRIMSON_MARKER) {
					clif_skill_fail( *sd, skill_id );
					break;
				}
			}

			sd->c_marker[i] = bl->id;
			status_change_start(src, bl, type, 10000, skill_lv, src->id, 0, 0, skill_get_time(skill_id,skill_lv), SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		// If mob casts this, at least SC_C_MARKER as debuff
		else {
			status_change_start(src, bl, type, 10000, skill_lv, src->id, 0, 0, skill_get_time(skill_id,skill_lv), SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;
	case RL_QD_SHOT:
		if (sd) {
			skill_area_temp[1] = bl->id;
			// Check surrounding
			skill_area_temp[0] = map_foreachinrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, BCT_ENEMY, skill_area_sub_count);
			if (skill_area_temp[0])
				map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);

			// Main target always receives damage
			clif_skill_nodamage(src, *src, skill_id, skill_lv);
			skill_attack(skill_get_type(skill_id), src, src, bl, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_LEVEL);
		} else {
			clif_skill_nodamage(src, *src, skill_id, skill_lv);
			map_foreachinrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_SPLASH|1, skill_castend_damage_id);
		}
		status_change_end(src, SC_QD_SHOT_READY); // End here to prevent spamming of the skill onto the target.
		skill_area_temp[0] = 0;
		skill_area_temp[1] = 0;
		break;
	case RL_FLICKER:
		if (sd) {
			sd->flicker = true;
			skill_area_temp[1] = 0;
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			// Detonate RL_B_TRAP
			if (pc_checkskill(sd, RL_B_TRAP))
				map_foreachinallrange(skill_bind_trap, src, AREA_SIZE, BL_SKILL, src);
			// Detonate RL_H_MINE
			if ((i = pc_checkskill(sd, RL_H_MINE)))
				map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, RL_H_MINE, i, tick, flag|BCT_ENEMY|SD_SPLASH, skill_castend_damage_id);
			sd->flicker = false;
		}
		break;

	case SO_ELEMENTAL_SHIELD:
		if (!sd || sd->status.party_id == 0 || flag&1) {
			if (sd && sd->status.party_id == 0) {
				clif_skill_nodamage(src,*bl,skill_id,skill_lv);
				if (sd->ed && skill_get_state(skill_id) == ST_ELEMENTALSPIRIT2)
					elemental_delete(sd->ed);
			}
			skill_unitsetting(bl, MG_SAFETYWALL, skill_lv + 5, bl->x, bl->y, 0);
			skill_unitsetting(bl, AL_PNEUMA, 1, bl->x, bl->y, 0);
		}
		else {
			clif_skill_nodamage(src,*bl,skill_id,skill_lv);
			if (sd->ed && skill_get_state(skill_id) == ST_ELEMENTALSPIRIT2)
				elemental_delete(sd->ed);
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id,skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case SU_HIDE:
		if (tsce) {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			status_change_end(bl, type);
			map_freeblock_unlock();
			return 0;
		}
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
 		break;

	case SU_STOOP:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
 		break;

	case SU_SV_ROOTTWIST:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		if (sd && status_get_class_(bl) == CLASS_BOSS) {
			clif_skill_fail( *sd, skill_id, USESKILL_FAIL_TOTARGET );
			break;
		}
		if (tsc && tsc->count && tsc->getSCE(type)) // Refresh the status only if it's already active.
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		else {
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			if (sd && pc_checkskill(sd, SU_SPIRITOFLAND))
				sc_start(src, src, SC_DORAM_MATK, 100, sd->status.base_level, skill_get_time(SU_SPIRITOFLAND, 1));
			skill_addtimerskill(src, tick + 1000, bl->id, 0, 0, SU_SV_ROOTTWIST_ATK, skill_lv, skill_get_type(SU_SV_ROOTTWIST_ATK), flag);
		}
		break;

	case SU_TUNABELLY:
	{
		unsigned int heal = 0;

		if (dstmd && (dstmd->mob_id == MOBID_EMPERIUM || status_get_class_(bl) == CLASS_BATTLEFIELD))
			heal = 0;
		else if (status_get_hp(bl) != status_get_max_hp(bl))
			heal = ((2 * skill_lv - 1) * 10) * status_get_max_hp(bl) / 100;
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		status_heal(bl, heal, 0, 0);
	}
		break;

	case SU_BUNCHOFSHRIMP:
	case SU_HISS:
	case SU_PURRING:
	case SU_MEOWMEOW:
		if (sd == nullptr || sd->status.party_id == 0 || flag&1) {
			int duration = skill_get_time(skill_id, skill_lv);

			if (skill_id == SU_BUNCHOFSHRIMP && pc_checkskill(sd, SU_SPIRITOFSEA))
				duration += skill_get_time2(SU_BUNCHOFSHRIMP, skill_lv);
			clif_skill_nodamage(bl, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, duration));
		} else if (sd) {
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		}
		break;

	case SU_SHRIMPARTY:
		if (sd == nullptr || sd->status.party_id == 0 || flag&1) {
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			if (sd && (i = pc_checkskill(sd, SU_FRESHSHRIMP)) > 0) {
				clif_skill_nodamage(bl, *bl, SU_FRESHSHRIMP, i, 1);
				sc_start(src, bl, SC_FRESHSHRIMP, 100, i, skill_get_time(SU_FRESHSHRIMP, i));
			}
		} else if (sd)
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag|BCT_PARTY|1, skill_castend_nodamage_id);
		break;

	case SU_POWEROFFLOCK:
		if (flag&1) {
			sc_start(src, bl, SC_FEAR, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			sc_start(src, bl, SC_FREEZE, 100, skill_lv, skill_get_time2(skill_id, skill_lv)); //! TODO: What's the duration?
		} else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			if (battle_config.skill_wall_check)
				map_foreachinshootrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			else
				map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		}
		break;

	case ALL_EQSWITCH:
		if( sd ){
			clif_equipswitch_reply( sd, false );

			for( int i = 0, position = 0; i < EQI_MAX; i++ ){
				if( sd->equip_switch_index[i] >= 0 && !( position & equip_bitmask[i] ) ){
					position |= pc_equipswitch( sd, sd->equip_switch_index[i] );
				}
			}
		}
		break;

	case AB_VITUPERATUM:
		if (flag&1)
			clif_skill_nodamage(src, *bl, skill_id, skill_lv, sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv)));
		else {
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case AB_CONVENIO:
		if (sd) {
			party_data *p = party_search(sd->status.party_id);
			int i = 0, count = 0;

			// Only usable in party
			if (p == nullptr) {
				clif_skill_fail( *sd, skill_id );
				break;
			}

			// Only usable as party leader.
			ARR_FIND(0, MAX_PARTY, i, p->data[i].sd == sd);
			if (i == MAX_PARTY || !p->party.member[i].leader) {
				clif_skill_fail( *sd, skill_id );
				break;
			}

			// Do the teleport part
			for (i = 0; i < MAX_PARTY; ++i) {
				map_session_data *pl_sd = p->data[i].sd;

				if (pl_sd == nullptr || pl_sd == sd || pl_sd->status.party_id != p->party.party_id || pc_isdead(pl_sd) ||
					sd->bl.m != pl_sd->bl.m)
					continue;

				// Respect /call configuration
				if( pl_sd->status.disable_call ){
					continue;
				}

				if (!(map_getmapflag(sd->bl.m, MF_NOTELEPORT) || map_getmapflag(sd->bl.m, MF_PVP) || map_getmapflag(sd->bl.m, MF_BATTLEGROUND) || map_flag_gvg2(sd->bl.m))) {
					pc_setpos(pl_sd, map_id2index(sd->bl.m), sd->bl.x, sd->bl.y, CLR_TELEPORT);
					count++;
				}
			}
			if (!count)
				clif_skill_fail( *sd, skill_id );
		}
		break;

	case NPC_PULSESTRIKE2:
		for (int i = 0; i < 3; i++)
			skill_addtimerskill(src, tick + (t_tick)skill_get_time(skill_id, skill_lv) * i, bl->id, 0, 0, skill_id, skill_lv, skill_get_type(skill_id), flag);
		break;

	case BO_THE_WHOLE_PROTECTION:
		if (sd == nullptr || sd->status.party_id == 0 || (flag & 1)) {
			unsigned int equip[] = { EQP_WEAPON, EQP_SHIELD, EQP_ARMOR, EQP_HEAD_TOP };

			for (uint8 i_eqp = 0; i_eqp < 4; i_eqp++) {
				if (bl->type != BL_PC || (dstsd && pc_checkequip(dstsd, equip[i_eqp]) < 0))
					continue;
				sc_start(src, bl, (sc_type)(SC_CP_WEAPON + i_eqp), 100, skill_lv, skill_get_time(skill_id, skill_lv));
			}
		} else if (sd) {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
		}
		break;

	case TR_MUSICAL_INTERLUDE:
	case TR_JAWAII_SERENADE:
	case TR_PRON_MARCH:
		if (sd == nullptr || sd->status.party_id == 0 || (flag & 1))
			sc_start4(src, bl, type, 100, skill_lv, 0, flag, 0, skill_get_time(skill_id, skill_lv));
		else if (sd) {
			clif_skill_nodamage(bl, *bl, skill_id, skill_lv);

			sd->skill_id_song = skill_id;
			sd->skill_lv_song = skill_lv;

			if (skill_check_pc_partner(sd, skill_id, &skill_lv, AREA_SIZE, 0) > 0)
				flag |= 2;

			party_foreachsamemap(skill_area_sub, sd, skill_get_splash(skill_id, skill_lv), src, skill_id, skill_lv, tick, flag | BCT_PARTY | 1, skill_castend_nodamage_id);
		}
		break;

	case TR_GEF_NOCTURN:
	case TR_AIN_RHAPSODY:
		if (flag & 1)
			sc_start4(src, bl, type, 100, skill_lv, 0, flag, 0, skill_get_time(skill_id, skill_lv));
		else if (sd) {
			clif_skill_nodamage(bl, *bl, skill_id, skill_lv);

			sd->skill_id_song = skill_id;
			sd->skill_lv_song = skill_lv;

			if (skill_check_pc_partner(sd, skill_id, &skill_lv, AREA_SIZE, 0) > 0)
				flag |= 2;

			map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_nodamage_id);
		}
		break;

	case TR_ROKI_CAPRICCIO:
	case TR_NIPELHEIM_REQUIEM:
		if (flag & 1) { // Need official success chances.
			uint16 success_chance = 5 * skill_lv;

			if (flag & 2)
				success_chance *= 2;

			// Is it a chance to inflect so and so, or seprate chances for inflicting each status? [Rytech]
			if (skill_id == TR_ROKI_CAPRICCIO) {
				sc_start(src, bl, SC_CONFUSION, 4 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
				sc_start(src, bl, SC_HANDICAPSTATE_MISFORTUNE, success_chance, skill_lv, skill_get_time2(skill_id, skill_lv));
			} else { // TR_NIPELHEIM_REQUIEM
				sc_start(src, bl, SC_CURSE, 4 * skill_lv, skill_lv, skill_get_time(skill_id, skill_lv));
				sc_start(src, bl, SC_HANDICAPSTATE_DEPRESSION, success_chance, skill_lv, skill_get_time2(skill_id, skill_lv));
			}
		} else if (sd) {
			clif_skill_nodamage(bl, *bl, skill_id, skill_lv);

			sd->skill_id_song = skill_id;
			sd->skill_lv_song = skill_lv;

			if (skill_check_pc_partner(sd, skill_id, &skill_lv, AREA_SIZE, 0) > 0)
				flag |= 2;

			map_foreachinallrange(skill_area_sub, src, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_nodamage_id);
		}
		break;

	case ABR_NET_REPAIR:
	case ABR_NET_SUPPORT:
		if (flag & 1) {
			int heal_amount;

			if (skill_id == ABR_NET_REPAIR) {
				heal_amount = tstatus->max_hp * 10 / 100;
				clif_skill_nodamage(nullptr, *bl, AL_HEAL, heal_amount);
				status_heal(bl, heal_amount, 0, 0);
			} else { // ABR_NET_SUPPORT
				heal_amount = tstatus->max_sp * 3 / 100;
				clif_skill_nodamage(nullptr, *bl, MG_SRECOVERY, heal_amount);
				status_heal(bl, 0, heal_amount, 0);
			}
		} else {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ALLY | SD_SPLASH | 1, skill_castend_nodamage_id);
		}
		break;

	case WH_HAWK_M:
		if (sd) {
			if (!pc_isfalcon(sd))
				pc_setoption(sd, sd->sc.option | OPTION_FALCON);
			else
				pc_setoption(sd, sd->sc.option&~OPTION_FALCON);
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case EM_SUMMON_ELEMENTAL_ARDOR:
	case EM_SUMMON_ELEMENTAL_DILUVIO:
	case EM_SUMMON_ELEMENTAL_PROCELLA:
	case EM_SUMMON_ELEMENTAL_TERREMOTUS:
	case EM_SUMMON_ELEMENTAL_SERPENS: {
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);

			if (sd == nullptr)
				break;

			uint16 em_elem[5] = { ELEMENTALID_ARDOR, ELEMENTALID_DILUVIO, ELEMENTALID_PROCELLA, ELEMENTALID_TERREMOTUS, ELEMENTALID_SERPENS };
			uint16 so_elem[5] = { ELEMENTALID_AGNI_L, ELEMENTALID_AQUA_L, ELEMENTALID_VENTUS_L, ELEMENTALID_TERA_L, 0 };
			uint8 elem_value = 4 - (EM_SUMMON_ELEMENTAL_SERPENS - skill_id);

			if (sd->ed && ((skill_id >= EM_SUMMON_ELEMENTAL_ARDOR && skill_id <= EM_SUMMON_ELEMENTAL_TERREMOTUS && sd->ed->elemental.class_ == so_elem[elem_value]) ||
						   (skill_id == EM_SUMMON_ELEMENTAL_SERPENS &&
							(sd->ed->elemental.class_ == ELEMENTALID_AGNI_L || sd->ed->elemental.class_ == ELEMENTALID_AQUA_L ||
							 sd->ed->elemental.class_ == ELEMENTALID_VENTUS_L || sd->ed->elemental.class_ == ELEMENTALID_TERA_L)))) {
				// Remove the old elemental before summoning the super one.
				elemental_delete(sd->ed);

				if (!elemental_create(sd, em_elem[elem_value], skill_get_time(skill_id, skill_lv))) {
					clif_skill_fail( *sd, skill_id );
					break;
				} else // Elemental summoned. Buff the player with the bonus.
					sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
			} else {
				clif_skill_fail( *sd, skill_id );
				break;
			}
		}
		break;

	case EM_ELEMENTAL_VEIL:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);

		if (sd == nullptr)
			break;

		if (sd->ed && sd->ed->elemental.class_ >= ELEMENTALID_DILUVIO && sd->ed->elemental.class_ <= ELEMENTALID_SERPENS)
			sc_start(src, &sd->ed->bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		else
			clif_skill_fail( *sd, skill_id );
		break;

	case MT_M_MACHINE:
	case BO_BIONIC_PHARMACY:
		if (sd) {
			sd->skill_id_old = skill_id;
			sd->skill_lv_old = skill_lv;

			if (skill_id == MT_M_MACHINE)
				clif_cooking_list( *sd, 31, skill_id, 1, 7 );
			else // BO_BIONIC_PHARMACY
				clif_cooking_list( *sd, 32, skill_id, 1, 8 );
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		}
		break;

	case MT_SUMMON_ABR_BATTLE_WARIOR:
	case MT_SUMMON_ABR_DUAL_CANNON:
	case MT_SUMMON_ABR_MOTHER_NET:
	case MT_SUMMON_ABR_INFINITY: {
			uint32 abrs[4] = { MOBID_ABR_BATTLE_WARIOR, MOBID_ABR_DUAL_CANNON, MOBID_ABR_MOTHER_NET, MOBID_ABR_INFINITY };

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));

			mob_data *md = mob_once_spawn_sub(src, src->m, src->x, src->y, "--ja--", abrs[3 - (MT_SUMMON_ABR_INFINITY - skill_id)], "", SZ_SMALL, AI_ABR);

			if (md) {
				md->master_id = src->id;
				md->special_state.ai = AI_ABR;

				if (md->deletetimer != INVALID_TIMER)
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer(gettick() + skill_get_time(skill_id, skill_lv), mob_timer_delete, md->bl.id, 0);
				mob_spawn(md);
			}
		}
		break;

	case BO_WOODENWARRIOR:
	case BO_WOODEN_FAIRY:
	case BO_CREEPER:
	case BO_HELLTREE: { // A poring is used in the 4th slot as a dummy since the Research Report skill is in between the Creeper and Hell Tree skills.
			uint32 bionics[5] = { MOBID_BIONIC_WOODENWARRIOR, MOBID_BIONIC_WOODEN_FAIRY, MOBID_BIONIC_CREEPER, MOBID_PORING, MOBID_BIONIC_HELLTREE };

			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));

			mob_data *md = mob_once_spawn_sub(src, src->m, src->x, src->y, "--ja--", bionics[4 - (BO_HELLTREE - skill_id)], "", SZ_SMALL, AI_BIONIC);

			if (md) {
				md->master_id = src->id;
				md->special_state.ai = AI_BIONIC;

				if (md->deletetimer != INVALID_TIMER)
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer(gettick() + skill_get_time(skill_id, skill_lv), mob_timer_delete, md->bl.id, 0);
				mob_spawn(md);
			}
		}
		break;

#ifdef RENEWAL
	case CG_HERMODE:
		skill_castend_song(src, skill_id, skill_lv, tick);
		break;
#endif

	case NPC_LEASH:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);

		if( !skill_check_unit_movepos( 0, bl, src->x, src->y, 1, 1 ) ){
			return 0;
		}

		clif_blown( bl );
		break;

	case NPC_WIDELEASH:
		if( flag & 1 ){
			if( !skill_check_unit_movepos( 0, bl, src->x, src->y, 1, 1 ) ){
				return 0;
			}

			clif_blown( bl );
		}else{
			skill_area_temp[2] = 0; // For SD_PREAMBLE
			clif_skill_nodamage(src, *bl, skill_id, skill_lv);
			map_foreachinallrange( skill_area_sub, bl, skill_get_splash( skill_id, skill_lv ), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_PREAMBLE | 1, skill_castend_nodamage_id );
		}
		break;

	case HN_HELLS_DRIVE:
		clif_skill_nodamage(src, *bl, skill_id, skill_lv);
		map_foreachinrange(skill_area_sub, bl, skill_get_splash(skill_id, skill_lv), BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
		break;

	case NW_THE_VIGILANTE_AT_NIGHT:
		i = skill_get_splash(skill_id, skill_lv);
		skill_area_temp[0] = 0;
		skill_area_temp[1] = bl->id;
		skill_area_temp[2] = 0;

		if (sd && sd->weapontype1 == W_GATLING) {
			i = 5; // 11x11
			clif_skill_nodamage(src, *bl, NW_THE_VIGILANTE_AT_NIGHT_GUN_GATLING, skill_lv);
		} else
			clif_skill_nodamage(src, *bl, NW_THE_VIGILANTE_AT_NIGHT_GUN_SHOTGUN, skill_lv);
		map_foreachinrange(skill_area_sub, bl, i, BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | 1, skill_castend_damage_id);
		if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
			status_change_end(src, SC_INTENSIVE_AIM_COUNT);
		break;

	case NW_INTENSIVE_AIM:
		if (tsc && tsc->getSCE(type)) {
			status_change_end(src, SC_INTENSIVE_AIM_COUNT);
			status_change_end(bl, type);
		} else {
			status_change_end(src, SC_INTENSIVE_AIM_COUNT);
			sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		}
		clif_skill_nodamage(src, *src, skill_id, skill_lv);
		break;

	case NW_HIDDEN_CARD:
	case NW_AUTO_FIRING_LAUNCHER:
		sc_start(src, bl, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		clif_skill_nodamage(src, *src, skill_id, skill_lv);
		break;

	case NW_GRENADE_FRAGMENT:
		status_change_end(src, type);
		if (skill_lv < 7)
			sc_start(src, bl, (sc_type)(SC_GRENADE_FRAGMENT_1 -1 + skill_lv), 100, skill_lv, skill_get_time(skill_id, skill_lv));
		else if (skill_lv == 7) {
			status_change_end(src, SC_GRENADE_FRAGMENT_1);
			status_change_end(src, SC_GRENADE_FRAGMENT_2);
			status_change_end(src, SC_GRENADE_FRAGMENT_3);
			status_change_end(src, SC_GRENADE_FRAGMENT_4);
			status_change_end(src, SC_GRENADE_FRAGMENT_5);
			status_change_end(src, SC_GRENADE_FRAGMENT_6);
		}
		clif_skill_nodamage(src, *src, skill_id, skill_lv);
		break;

	default: {
		std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);
		ShowWarning("skill_castend_nodamage_id: missing code case for skill %s(%d)\n", skill ? skill->name : "UNKNOWN", skill_id);
		clif_skill_nodamage(src,*bl,skill_id,skill_lv);
		map_freeblock_unlock();
		return 1;
		}
	}

	if (skill_id != SR_CURSEDCIRCLE && skill_id != NPC_SR_CURSEDCIRCLE) {
		if (sc && sc->getSCE(SC_CURSEDCIRCLE_ATKER)) // Should only remove after the skill had been casted.
			status_change_end(src,SC_CURSEDCIRCLE_ATKER);
	}

	if (dstmd) { //Mob skill event for no damage skills (damage ones are handled in battle_damage/skill_attack) [Skotlex]
		mob_log_damage(dstmd, src, 0); //Log interaction (counts as 'attacker' for the exp bonus)
		mobskill_event(dstmd, src, tick, MSC_SKILLUSED|(skill_id<<16));
	}

	if( sd && !(flag&1) )
	{// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = gettick();

		if( sd->state.arrow_atk )
		{// consume arrow on last invocation to this skill.
			battle_consume_ammo(sd, skill_id, skill_lv);
		}
		skill_onskillusage(sd, bl, skill_id, tick);
		// perform skill requirement consumption
		if (!(flag&SKILL_NOCONSUME_REQ))
			skill_consume_requirement(sd,skill_id,skill_lv,2);
	}

	map_freeblock_unlock();
	return 0;
}

/**
 * Checking that causing skill failed
 * @param src Caster
 * @param target Target
 * @param skill_id
 * @param skill_lv
 * @return -1 success, others are failed @see enum useskill_fail_cause.
 **/
static int8 skill_castend_id_check(struct block_list *src, struct block_list *target, uint16 skill_id, uint16 skill_lv) {
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);
	int inf = skill->inf;
	status_change *tsc = status_get_sc(target);

	if (src != target && (status_bl_has_mode(target,MD_SKILLIMMUNE) || (status_get_class(target) == MOBID_EMPERIUM && !skill->inf2[INF2_TARGETEMPERIUM])) && skill_get_casttype(skill_id) == CAST_NODAMAGE)
		return USESKILL_FAIL_MAX; // Don't show a skill fail message (NoDamage type doesn't consume requirements)

	switch (skill_id) {
		case AL_HEAL:
		case AL_INCAGI:
		case AL_DECAGI:
		case SA_DISPELL: // Mado Gear is immune to Dispell according to bugreport:49 [Ind]
		case AB_RENOVATIO:
		case AB_HIGHNESSHEAL:
			if (tsc && tsc->option&OPTION_MADOGEAR)
				return USESKILL_FAIL_TOTARGET;
			break;
		case RG_BACKSTAP:
			{
#ifndef RENEWAL
				uint8 dir = map_calc_dir(src,target->x,target->y), t_dir = unit_getdir(target);

				if (map_check_dir(dir, t_dir))
					return USESKILL_FAIL_MAX;
#endif

				if (check_distance_bl(src, target, 0))
					return USESKILL_FAIL_MAX;
			}
			break;
		case PR_TURNUNDEAD:
			{
				status_data* tstatus = status_get_status_data(*target);
				if (!battle_check_undead(tstatus->race, tstatus->def_ele))
					return USESKILL_FAIL_MAX;
			}
			break;
		case PR_LEXDIVINA:
		case MER_LEXDIVINA:
			{
				//If it's not an enemy, and not silenced, you can't use the skill on them. [Skotlex]
				if (battle_check_target(src,target, BCT_ENEMY) <= 0 && (!tsc || !tsc->getSCE(SC_SILENCE)))
					return USESKILL_FAIL_LEVEL;
				else
					return -1; //Works on silenced allies
			}
			break;
		case RA_WUGSTRIKE:
			// Check if path can be reached
			if (!path_search(nullptr,src->m,src->x,src->y,target->x,target->y,1,CELL_CHKNOREACH))
				return USESKILL_FAIL_MAX;
			break;
		case MG_NAPALMBEAT:
		case MG_FIREBALL:
		case HT_BLITZBEAT:
		case AS_GRIMTOOTH:
		case MO_COMBOFINISH:
		case NC_VULCANARM:
		case SR_TIGERCANNON:
			// These can damage traps, but can't target traps directly
			if (target->type == BL_SKILL) {
				TBL_SKILL *su = (TBL_SKILL*)target;
				if (!su || !su->group)
					return USESKILL_FAIL_MAX;				
				if (skill_get_inf2(su->group->skill_id, INF2_ISTRAP))
					return USESKILL_FAIL_MAX;
			}
			break;
		case IQ_SECOND_FLAME:
		case IQ_SECOND_FAITH:
		case IQ_SECOND_JUDGEMENT:
			if (!tsc || !(tsc->getSCE(SC_FIRST_BRAND) || tsc->getSCE(SC_SECOND_BRAND)))
				return USESKILL_FAIL_LEVEL;
			break;
		case IQ_THIRD_PUNISH:
		case IQ_THIRD_FLAME_BOMB:
		case IQ_THIRD_CONSECRATION:
			if (!tsc || !tsc->getSCE(SC_SECOND_BRAND))
				return USESKILL_FAIL_LEVEL;
			break;
	}

	if (inf&INF_ATTACK_SKILL ||
		(inf&INF_SELF_SKILL && skill->inf2[INF2_NOTARGETSELF]) //Combo skills
		) // Casted through combo.
		inf = BCT_ENEMY; //Offensive skill.
	else if (skill->inf2[INF2_NOTARGETENEMY])
		inf = BCT_NOENEMY;
	else
		inf = 0;

	if ((skill->inf2[INF2_PARTYONLY] || skill->inf2[INF2_GUILDONLY]) && src != target) {
		inf |=
			(skill->inf2[INF2_PARTYONLY]?BCT_PARTY:0)|
			(skill->inf2[INF2_GUILDONLY]?BCT_GUILD:0);
		//Remove neutral targets (but allow enemy if skill is designed to be so)
		inf &= ~BCT_NEUTRAL;
	}

	switch (skill_id) {
		// Cannot be casted to Emperium
		case WZ_ESTIMATION:
		case SL_SKE:
		case SL_SKA:
		case RK_PHANTOMTHRUST:
		case NPC_PHANTOMTHRUST:
			if (target->type == BL_MOB && ((TBL_MOB*)target)->mob_id == MOBID_EMPERIUM)
				return USESKILL_FAIL_MAX;
			break;
	}

	if (inf && battle_check_target(src, target, inf) <= 0) {
		switch(skill_id) {
			case RK_PHANTOMTHRUST:
			case NPC_PHANTOMTHRUST:
			case AB_CLEARANCE:
				return USESKILL_FAIL_TOTARGET;
			default:
				return USESKILL_FAIL_LEVEL;
		}
	}

	// Fogwall makes all offensive-type targetted skills fail at 75%
	// Jump Kick can still fail even though you can jump to friendly targets.
	if ((inf&BCT_ENEMY || skill_id == TK_JUMPKICK) && tsc && tsc->getSCE(SC_FOGWALL) && rnd() % 100 < 75)
		return USESKILL_FAIL_LEVEL;

	return -1;
}

TIMER_FUNC( skill_keep_using ){
	map_session_data* sd = map_id2sd( id );

	if( sd && sd->skill_keep_using.skill_id ){
		clif_parse_skill_toid( sd, sd->skill_keep_using.skill_id, sd->skill_keep_using.level, sd->skill_keep_using.target );
		sd->skill_keep_using.tid = INVALID_TIMER;
	}

	return 0;
}

/**
 * Check & process skill to target on castend. Determines if skill is 'damage' or 'nodamage'
 * @param tid
 * @param tick
 * @param data
 **/
TIMER_FUNC(skill_castend_id){
	struct block_list *target, *src;
	map_session_data *sd;
	struct mob_data *md;
	struct unit_data *ud;
	int flag = 0;

	src = map_id2bl(id);
	if( src == nullptr )
	{
		ShowDebug("skill_castend_id: src == nullptr (tid=%d, id=%d)\n", tid, id);
		return 0;// not found
	}

	ud = unit_bl2ud(src);
	if( ud == nullptr )
	{
		ShowDebug("skill_castend_id: ud == nullptr (tid=%d, id=%d)\n", tid, id);
		return 0;// ???
	}

	sd = BL_CAST(BL_PC,  src);
	md = BL_CAST(BL_MOB, src);
	status_change *sc = status_get_sc(src);

	if( src->prev == nullptr ) {
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if(ud->skill_id != SA_CASTCANCEL && ud->skill_id != SO_SPELLFIST) {// otherwise handled in unit_skillcastcancel()
		if( ud->skilltimer != tid ) {
			ShowError("skill_castend_id: Timer mismatch %d!=%d!\n", ud->skilltimer, tid);
			ud->skilltimer = INVALID_TIMER;
			return 0;
		}

		if( sd && ud->skilltimer != INVALID_TIMER && (pc_checkskill(sd,SA_FREECAST) > 0 || ud->skill_id == LG_EXEEDBREAK) )
		{// restore original walk speed
			ud->skilltimer = INVALID_TIMER;
			status_calc_bl(&sd->bl, { SCB_SPEED, SCB_ASPD });
		} else
			ud->skilltimer = INVALID_TIMER;
	}

	if (ud->skilltarget == id)
		target = src;
	else
		target = map_id2bl(ud->skilltarget);

	// Use a do so that you can break out of it when the skill fails.
	do {
		bool fail = false;
		int8 res = USESKILL_FAIL_LEVEL;

		if (!target || target->prev == nullptr)
			break;

		if (src->m != target->m || status_isdead(*src))
			break;

		//These should become skill_castend_pos
		switch (ud->skill_id) {
			case WE_CALLPARTNER:
				if (sd) {
					map_session_data *p_sd = pc_get_partner(sd);

					if (p_sd != nullptr && (p_sd->state.autotrade || p_sd->status.disable_call)) {
						fail = true;
						break;
					} else
						clif_callpartner(*sd);
				}
				break;
			case WE_CALLPARENT:
				if (sd) {
					map_session_data *f_sd = pc_get_father(sd);
					map_session_data *m_sd = pc_get_mother(sd);

					if ((f_sd != nullptr && (f_sd->state.autotrade || f_sd->status.disable_call)) || (m_sd != nullptr && (m_sd->state.autotrade || m_sd->status.disable_call))) {
						fail = true;
						break;
					}
				}
				break;
			case WE_CALLBABY:
				if (sd) {
					map_session_data *c_sd = pc_get_child(sd);

					if (c_sd != nullptr && (c_sd->state.autotrade || c_sd->status.disable_call)) {
						fail = true;
						break;
					}
				}
				break;
			case AM_RESURRECTHOMUN:
			case PF_SPIDERWEB:
				{
					//Find a random spot to place the skill. [Skotlex]
					int splash = skill_get_splash(ud->skill_id, ud->skill_lv);
					ud->skillx = target->x + splash;
					ud->skilly = target->y + splash;
					if (splash && !map_random_dir(target, &ud->skillx, &ud->skilly)) {
						ud->skillx = target->x;
						ud->skilly = target->y;
					}
					ud->skilltimer = tid;
					return skill_castend_pos(tid,tick,id,data);
				}
			case GN_WALLOFTHORN:
			case SC_ESCAPE:
			case WL_FROSTMISTY:
			case SU_CN_POWDERING:
			case AG_RAIN_OF_CRYSTAL:
				ud->skillx = target->x;
				ud->skilly = target->y;
				ud->skilltimer = tid;
				return skill_castend_pos(tid,tick,id,data);
		}

		// Failing
		if (fail || (res = skill_castend_id_check(src, target, ud->skill_id, ud->skill_lv)) >= 0) {
			if (sd && res != USESKILL_FAIL_MAX)
				clif_skill_fail( *sd, ud->skill_id, (enum useskill_fail_cause)res );
			break;
		}

		//Avoid doing double checks for instant-cast skills.
		if (tid != INVALID_TIMER && !status_check_skilluse(src, target, ud->skill_id, 1))
			break;

		if(md) {
			md->last_thinktime=tick +MIN_MOBTHINKTIME;
			if(md->skill_idx >= 0 && md->db->skill[md->skill_idx]->emotion >= 0)
				clif_emotion(src, md->db->skill[md->skill_idx]->emotion);
		}

		if (src != target && battle_config.skill_add_range &&
			!check_distance_bl(src, target, skill_get_range2(src, ud->skill_id, ud->skill_lv, true) + battle_config.skill_add_range))
		{
			if (sd) {
				clif_skill_fail( *sd, ud->skill_id);
				if (battle_config.skill_out_range_consume) //Consume items anyway. [Skotlex]
					skill_consume_requirement(sd, ud->skill_id, ud->skill_lv, 3);
			}
			break;
		}
#ifdef OFFICIAL_WALKPATH
		if(skill_get_casttype(ud->skill_id) != CAST_NODAMAGE && !path_search_long(nullptr, src->m, src->x, src->y, target->x, target->y, CELL_CHKWALL))
		{
			if (sd) {
				clif_skill_fail( *sd, ud->skill_id );
				skill_consume_requirement(sd,ud->skill_id,ud->skill_lv,3); //Consume items anyway.
			}
			break;
		}
#endif
		if( sd )
		{
			if( !skill_check_condition_castend(*sd, ud->skill_id, ud->skill_lv) )
				break;
			else {
				skill_consume_requirement(sd,ud->skill_id,ud->skill_lv,1);

				int add_ap = skill_get_giveap( ud->skill_id, ud->skill_lv );

				// Give AP
				if (add_ap > 0) {
					switch (ud->skill_id) {
						case TR_ROSEBLOSSOM:
						case TR_RHYTHMSHOOTING:
						case TR_METALIC_FURY: // Don't know the official increase. For now lets do up to 50% increase.
						case TR_SOUNDBLEND: // Retrospection does the same for song skills. [Rytech]
							add_ap += add_ap * (10 * pc_checkskill(sd, TR_STAGE_MANNER)) / 100;
							break;
						case TR_GEF_NOCTURN:
						case TR_ROKI_CAPRICCIO:
						case TR_AIN_RHAPSODY:
						case TR_MUSICAL_INTERLUDE:
						case TR_JAWAII_SERENADE:
						case TR_NIPELHEIM_REQUIEM:
						case TR_PRON_MARCH:
							if (sd->skill_id_old == TR_RETROSPECTION) {
								add_ap += add_ap * 50 / 100;
								sd->skill_id_old = ud->skill_id; // Prevents AP bonus on non Retro Spection use.
							}
							break;
						case WH_CRESCIVE_BOLT:
							if (sc && sc->getSCE(SC_CRESCIVEBOLT) && sc->getSCE(SC_CRESCIVEBOLT)->val1 >= 3) {
								add_ap += 2;
							}
							break;
					}

					status_heal(&sd->bl, 0, 0, add_ap, 0);
				}

				if (src != target && (status_bl_has_mode(target,MD_SKILLIMMUNE) || (status_get_class(target) == MOBID_EMPERIUM && !skill_get_inf2(ud->skill_id, INF2_TARGETEMPERIUM))) && skill_get_casttype(ud->skill_id) == CAST_DAMAGE) {
					clif_skill_fail( *sd, ud->skill_id );
					break; // Show a skill fail message (Damage type consumes requirements)
				}
			}
		}

		if( (src->type == BL_MER || src->type == BL_HOM) && !skill_check_condition_mercenary(src, ud->skill_id, ud->skill_lv, 1) )
			break;

		if (ud->state.running && ud->skill_id == TK_JUMPKICK){
			ud->state.running = 0;
			status_change_end(src, SC_RUN);
			flag = 1;
		}

		if (ud->walktimer != INVALID_TIMER && ud->skill_id != TK_RUN && ud->skill_id != RA_WUGDASH)
			unit_stop_walking(src,1);

		if (!sd || sd->skillitem != ud->skill_id || skill_get_delay(ud->skill_id, ud->skill_lv))
			ud->canact_tick = i64max(tick + skill_delayfix(src, ud->skill_id, ud->skill_lv), ud->canact_tick - SECURITY_CASTTIME);

		// Cooldown application
		switch (src->type) {
		case BL_PC:{
			// Increases/Decreases cooldown of a skill by item/card bonuses.
			int cooldown = pc_get_skillcooldown(sd, ud->skill_id, ud->skill_lv);
			if (cooldown > 0)
				skill_blockpc_start(sd, ud->skill_id, cooldown);
		} break;
		case BL_HOM:{
			homun_data& hd = reinterpret_cast<homun_data&>(*src);
#ifdef RENEWAL
			skill_blockhomun_start(&hd, ud->skill_id, skill_get_cooldown(ud->skill_id, ud->skill_lv));
#else
			skill_blockhomun_start(&hd, ud->skill_id, skill_get_delay(ud->skill_id, ud->skill_lv));
#endif
		} break;
		}

		if( battle_config.display_status_timers && sd )
			clif_status_change(src, EFST_POSTDELAY, 1, skill_delayfix(src, ud->skill_id, ud->skill_lv), 0, 0, 0);
		if( sd )
		{
			switch( ud->skill_id )
			{
			case GS_DESPERADO:
			case RL_FIREDANCE:
				sd->canequip_tick = tick + skill_get_time(ud->skill_id, ud->skill_lv);
				break;
			case KN_BRANDISHSPEAR:
			case CR_GRANDCROSS: {
				sc_type type;

				if (ud->skill_id == KN_BRANDISHSPEAR)
					type = SC_STRIPWEAPON;
				else
					type = SC_STRIPSHIELD;

				if (sc && sc->getSCE(type)) {
					const struct TimerData* timer = get_timer(sc->getSCE(type)->timer);

					if (timer && timer->func == status_change_timer && DIFF_TICK(timer->tick, gettick() + skill_get_time(ud->skill_id, ud->skill_lv)) > 0)
						break;
				}
				sc_start2(src, src, type, 100, 0, 1, skill_get_time(ud->skill_id, ud->skill_lv));
				// During Grand Cross you are in ankle state (cannot move or teleport)
				if (ud->skill_id == CR_GRANDCROSS)
					sc_start(src, src, SC_ANKLE, 100, 0, skill_get_time(ud->skill_id, ud->skill_lv));
				break;
			}
			}
		}
		if (skill_get_state(ud->skill_id) != ST_MOVE_ENABLE) {
			// When monsters used a skill they won't walk for amotion, this does not apply to players
			// This is also important for monster skill usage behavior
			if (src->type == BL_MOB)
				unit_set_walkdelay(src, tick, max((int)status_get_amotion(src), skill_get_walkdelay(ud->skill_id, ud->skill_lv)), 1);
			else
				unit_set_walkdelay(src, tick, battle_config.default_walk_delay + skill_get_walkdelay(ud->skill_id, ud->skill_lv), 1);
		}

		if(battle_config.skill_log && battle_config.skill_log&src->type)
			ShowInfo("Type %d, ID %d skill castend id [id =%d, lv=%d, target ID %d]\n",
				src->type, src->id, ud->skill_id, ud->skill_lv, target->id);

		map_freeblock_lock();

		if (skill_get_casttype(ud->skill_id) == CAST_NODAMAGE)
			skill_castend_nodamage_id(src,target,ud->skill_id,ud->skill_lv,tick,flag);
		else
			skill_castend_damage_id(src,target,ud->skill_id,ud->skill_lv,tick,flag);

		if( sd && sd->skill_keep_using.tid == INVALID_TIMER && sd->skill_keep_using.skill_id > 0 && sd->skill_keep_using.skill_id == ud->skill_id && !skill_isNotOk(ud->skill_id, *sd) && skill_check_condition_castbegin(*sd, ud->skill_id, ud->skill_lv) ){
			sd->skill_keep_using.tid = add_timer( sd->ud.canact_tick + 100, skill_keep_using, sd->bl.id, 0 );
		}

		if(sc && sc->count) {
			if (ud->skill_id != RA_CAMOUFLAGE)
				status_change_end(src, SC_CAMOUFLAGE); // Applies to the first skill if active

			if(sc->getSCE(SC_SPIRIT) &&
				sc->getSCE(SC_SPIRIT)->val2 == SL_WIZARD &&
				sc->getSCE(SC_SPIRIT)->val3 == ud->skill_id &&
				ud->skill_id != WZ_WATERBALL)
				sc->getSCE(SC_SPIRIT)->val3 = 0; //Clear bounced spell check.
#ifndef RENEWAL
			if( sc->getSCE(SC_DANCING) && sd && skill_get_inf2(ud->skill_id, INF2_ISSONG) )
				skill_blockpc_start(sd,BD_ADAPTATION,3000);
#endif
		}

		if (sd && ud->skill_id != SA_ABRACADABRA) // they just set the data so leave it as it is.[Inkfish]
			sd->skillitem = sd->skillitemlv = sd->skillitem_keep_requirement = 0;

		if (ud->skilltimer == INVALID_TIMER) {
			if(md) md->skill_idx = -1;
			else ud->skill_id = 0; //mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skill_lv = ud->skilltarget = 0;
		}
		map_freeblock_unlock();
		return 1;
	} while(0);

	//Skill failed.
	if (ud->skill_id == MO_EXTREMITYFIST && sd && !(sc && sc->getSCE(SC_FOGWALL)))
	{	//When Asura fails... (except when it fails from Wall of Fog)
		//Consume SP/spheres
		skill_consume_requirement(sd,ud->skill_id, ud->skill_lv,1);
		status_set_sp(src, 0, 0);
		sc_start(src, src, SC_EXTREMITYFIST, 100, ud->skill_lv, skill_get_time(ud->skill_id, ud->skill_lv));
		status_change_end(src, SC_EXPLOSIONSPIRITS);
		status_change_end(src, SC_BLADESTOP);
		if( target && target->m == src->m ) { //Move character to target anyway.
			short x, y;
			short dir = map_calc_dir(src,target->x,target->y);

			//Move 3 cells (From Caster)
			if( dir > 0 && dir < 4 )
				x = -3;
			else if( dir > 4 )
				x = 3;
			else
				x = 0;
			if( dir > 2 && dir < 6 )
				y = -3;
			else if( dir == 7 || dir < 2 )
				y = 3;
			else
				y = 0;
			if( unit_movepos(src, src->x + x, src->y + y, 1, 1) ) { //Display movement + animation.
				clif_blown(src);
				clif_spiritball(src);
			}
			clif_skill_damage(src,target,tick,sd->battle_status.amotion,0,0,1,ud->skill_id,ud->skill_lv,DMG_SPLASH);
		}
	}

	ud->skill_id = ud->skilltarget = 0;
	if( !sd || sd->skillitem != ud->skill_id || skill_get_delay(ud->skill_id,ud->skill_lv) )
		ud->canact_tick = tick;
	//You can't place a skill failed packet here because it would be
	//sent in ALL cases, even cases where skill_check_condition fails
	//which would lead to double 'skill failed' messages u.u [Skotlex]
	if (sd) {
		sd->skillitem = sd->skillitemlv = sd->skillitem_keep_requirement = 0;
		if (sd->skill_keep_using.skill_id > 0) {
			sd->skill_keep_using.skill_id = 0;
			if (sd->skill_keep_using.tid != INVALID_TIMER) {
				delete_timer(sd->skill_keep_using.tid, skill_keep_using);
				sd->skill_keep_using.tid = INVALID_TIMER;
			}
		}
	} else if (md)
		md->skill_idx = -1;
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
TIMER_FUNC(skill_castend_pos){
	struct block_list* src = map_id2bl(id);
	map_session_data *sd;
	struct unit_data *ud = unit_bl2ud(src);
	struct mob_data *md;

	nullpo_ret(ud);

	sd = BL_CAST(BL_PC , src);
	md = BL_CAST(BL_MOB, src);

	if( src->prev == nullptr ) {
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if( ud->skilltimer != tid )
	{
		ShowError("skill_castend_pos: Timer mismatch %d!=%d\n", ud->skilltimer, tid);
		ud->skilltimer = INVALID_TIMER;
		return 0;
	}

	if( sd && ud->skilltimer != INVALID_TIMER && ( pc_checkskill(sd,SA_FREECAST) > 0 || ud->skill_id == LG_EXEEDBREAK ) )
	{// restore original walk speed
		ud->skilltimer = INVALID_TIMER;
		status_calc_bl(&sd->bl, { SCB_SPEED, SCB_ASPD });
	} else
		ud->skilltimer = INVALID_TIMER;

	do {
		if( status_isdead(*src) )
			break;

		if (!skill_pos_maxcount_check(src, ud->skillx, ud->skilly, ud->skill_id, ud->skill_lv, src->type, true))
			break;

		if(tid != INVALID_TIMER)
		{	//Avoid double checks on instant cast skills. [Skotlex]
			if (!status_check_skilluse(src, nullptr, ud->skill_id, 1))
				break;
			if (battle_config.skill_add_range &&
				!check_distance_blxy(src, ud->skillx, ud->skilly, skill_get_range2(src, ud->skill_id, ud->skill_lv, true) + battle_config.skill_add_range)) {
				if (sd && battle_config.skill_out_range_consume) //Consume items anyway.
					skill_consume_requirement(sd, ud->skill_id, ud->skill_lv, 3);
				break;
			}
		}

		if( sd )
		{
			if( ud->skill_id != AL_WARP && !skill_check_condition_castend(*sd, ud->skill_id, ud->skill_lv) )
				break;
			else {
				skill_consume_requirement(sd, ud->skill_id, ud->skill_lv, 1);

				int add_ap = skill_get_giveap(ud->skill_id, ud->skill_lv);

				// Give AP
				if (add_ap > 0) {
					switch (ud->skill_id) {
						case WH_DEEPBLINDTRAP:
						case WH_SOLIDTRAP:
						case WH_SWIFTTRAP:
						case WH_FLAMETRAP:
							if (pc_checkskill(sd, WH_ADVANCED_TRAP) >= 3)
								add_ap += 1;
							break;
					}
					status_heal(&sd->bl, 0, 0, add_ap, 0);
				}
			}
		}

		if( (src->type == BL_MER || src->type == BL_HOM) && !skill_check_condition_mercenary(src, ud->skill_id, ud->skill_lv, 1) )
			break;

		if(md) {
			md->last_thinktime=tick +MIN_MOBTHINKTIME;
			if(md->skill_idx >= 0 && md->db->skill[md->skill_idx]->emotion >= 0)
				clif_emotion(src, md->db->skill[md->skill_idx]->emotion);
		}

		if(battle_config.skill_log && battle_config.skill_log&src->type)
			ShowInfo("Type %d, ID %d skill castend pos [id =%d, lv=%d, (%d,%d)]\n",
				src->type, src->id, ud->skill_id, ud->skill_lv, ud->skillx, ud->skilly);

		if (ud->walktimer != INVALID_TIMER)
			unit_stop_walking(src,1);

		if (!sd || sd->skillitem != ud->skill_id || skill_get_delay(ud->skill_id, ud->skill_lv))
			ud->canact_tick = i64max(tick + skill_delayfix(src, ud->skill_id, ud->skill_lv), ud->canact_tick - SECURITY_CASTTIME);
		if (sd) { //Cooldown application
			int cooldown = pc_get_skillcooldown(sd,ud->skill_id, ud->skill_lv);
			if(cooldown) skill_blockpc_start(sd, ud->skill_id, cooldown);
		}
		if( battle_config.display_status_timers && sd )
			clif_status_change(src, EFST_POSTDELAY, 1, skill_delayfix(src, ud->skill_id, ud->skill_lv), 0, 0, 0);
//		if( sd )
//		{
//			switch( ud->skill_id )
//			{
//			case ????:
//				sd->canequip_tick = tick + ????;
//				break;
//			}
//		}
		// When monsters used a skill they won't walk for amotion, this does not apply to players
		// This is also important for monster skill usage behavior
		if (src->type == BL_MOB)
			unit_set_walkdelay(src, tick, max((int)status_get_amotion(src), skill_get_walkdelay(ud->skill_id, ud->skill_lv)), 1);
		else
			unit_set_walkdelay(src, tick, battle_config.default_walk_delay + skill_get_walkdelay(ud->skill_id, ud->skill_lv), 1);
		map_freeblock_lock();
		skill_castend_pos2(src,ud->skillx,ud->skilly,ud->skill_id,ud->skill_lv,tick,0);

		if (ud->skill_id != RA_CAMOUFLAGE)
			status_change_end(src, SC_CAMOUFLAGE); // Applies to the first skill if active

		if( sd && sd->skillitem != AL_WARP ) // Warp-Portal thru items will clear data in skill_castend_map. [Inkfish]
			sd->skillitem = sd->skillitemlv = sd->skillitem_keep_requirement = 0;

		if (ud->skilltimer == INVALID_TIMER) {
			if (md) md->skill_idx = -1;
			else ud->skill_id = 0; //Non mobs can't clear this one as it is used for skill condition 'afterskill'
			ud->skill_lv = ud->skillx = ud->skilly = 0;
		}

		map_freeblock_unlock();
		return 1;
	} while(0);

	if( !sd || sd->skillitem != ud->skill_id || skill_get_delay(ud->skill_id,ud->skill_lv) )
		ud->canact_tick = tick;
	ud->skill_id = ud->skill_lv = 0;
	if(sd)
		sd->skillitem = sd->skillitemlv = sd->skillitem_keep_requirement = 0;
	else if(md)
		md->skill_idx  = -1;
	return 0;

}

/* skill count without self */
static int skill_count_wos(struct block_list *bl,va_list ap) {
	struct block_list* src = va_arg(ap, struct block_list*);
	if( src->id != bl->id ) {
		return 1;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_pos2(struct block_list* src, int x, int y, uint16 skill_id, uint16 skill_lv, t_tick tick, int flag)
{
	map_session_data* sd;
	status_change* sc;
	struct status_change_entry *sce;
	std::shared_ptr<s_skill_unit_group> sg;
	enum sc_type type;
	int i;

	//if(skill_lv <= 0) return 0;
	if(skill_id > 0 && !skill_lv) return 0;	// celest

	nullpo_ret(src);

	if(status_isdead(*src))
		return 0;

	sd = BL_CAST(BL_PC, src);

	sc = status_get_sc(src);
	type = skill_get_sc(skill_id);
	sce = (sc && type != SC_NONE)?sc->getSCE(type):nullptr;

	switch (skill_id) { //Skill effect.
		case WZ_METEOR:
		case WZ_ICEWALL:
		case MO_BODYRELOCATION:
		case CR_CULTIVATION:
		case HW_GANBANTEIN:
		case LG_EARTHDRIVE:
		case SC_ESCAPE:
		case SU_CN_METEOR:
		case NPC_RAINOFMETEOR:
		case HN_METEOR_STORM_BUSTER:
		case NW_GRENADES_DROPPING:
			break; //Effect is displayed on respective switch case.
		default:
			if(skill_get_inf(skill_id)&INF_SELF_SKILL)
				clif_skill_nodamage(src,*src,skill_id,skill_lv);
			else
				clif_skill_poseffect(src,skill_id,skill_lv,x,y,tick);
	}

	switch(skill_id)
	{
	case PR_BENEDICTIO:
		skill_area_temp[1] = src->id;
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_area_sub,
			src->m, x-i, y-i, x+i, y+i, BL_PC,
			src, skill_id, skill_lv, tick, flag|BCT_ALL|1,
			skill_castend_nodamage_id);
		map_foreachinallarea(skill_area_sub,
			src->m, x-i, y-i, x+i, y+i, BL_CHAR,
			src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		break;

	case BS_HAMMERFALL:
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_area_sub,
			src->m, x-i, y-i, x+i, y+i, BL_CHAR,
			src, skill_id, skill_lv, tick, flag|BCT_ENEMY|2,
			skill_castend_nodamage_id);
		break;

	case HT_DETECTING:
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea( status_change_timer_sub,
			src->m, x-i, y-i, x+i,y+i,BL_CHAR,
			src,nullptr,SC_SIGHT,tick);
		skill_reveal_trap_inarea(src, i, x, y);
		break;

	case SR_RIDEINLIGHTNING:
	case NW_BASIC_GRENADE:
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR,
			src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
		break;

	case NPC_LEX_AETERNA:
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR, src,
			PR_LEXAETERNA, 1, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		break;

	case SA_VOLCANO:
	case SA_DELUGE:
	case SA_VIOLENTGALE:
	{	//Does not consumes if the skill is already active. [Skotlex]
		std::shared_ptr<s_skill_unit_group> sg2;
		if ((sg2= skill_locate_element_field(src)) != nullptr && ( sg2->skill_id == SA_VOLCANO || sg2->skill_id == SA_DELUGE || sg2->skill_id == SA_VIOLENTGALE ))
		{
			if (sg2->limit - DIFF_TICK(gettick(), sg2->tick) > 0)
			{
				skill_unitsetting(src,skill_id,skill_lv,x,y,0);
				return 0; // not to consume items
			}
			else
				sg2->limit = 0; //Disable it.
		}
		skill_unitsetting(src,skill_id,skill_lv,x,y,0);
		break;
	}

	// Skill Unit Setting
	case MG_SAFETYWALL: {
		int dummy = 1;

		if (map_foreachincell(skill_cell_overlap, src->m, x, y, BL_SKILL, skill_id, &dummy, src)) {
			skill_unitsetting(src, skill_id, skill_lv, x, y, 0);
			return 0; // Don't consume gems if cast on Land Protector
		}
	}
	[[fallthrough]];
	case MG_FIREWALL:
	case MG_THUNDERSTORM:
	case AL_PNEUMA:
	case WZ_FIREPILLAR:
	case WZ_QUAGMIRE:
	case WZ_VERMILION:
	case WZ_STORMGUST:
	case WZ_HEAVENDRIVE:
	case NPC_GROUNDDRIVE:
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
	case SA_LANDPROTECTOR:
#ifndef RENEWAL
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
#endif
	case CG_MOONLIT:
	case GS_DESPERADO:
	case NJ_KAENSIN:
	case NJ_BAKUENRYU:
	case NJ_SUITON:
	case NJ_HYOUSYOURAKU:
	case NJ_RAIGEKISAI:
	case NJ_KAMAITACHI:
#ifdef RENEWAL
	case HW_GRAVITATION:
#endif
	case NPC_EVILLAND:
	case NPC_VENOMFOG:
	case NPC_COMET:
	case NPC_WIDESUCK:
	case NPC_ICEMINE:
	case NPC_FLAMECROSS:
	case NPC_HELLBURNING:
	case NPC_REVERBERATION:
	case WL_COMET:
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
	case SC_MAELSTROM:
	case SC_BLOODYLUST:
	case WM_POEMOFNETHERWORLD:
	case SO_PSYCHIC_WAVE:
	case NPC_PSYCHIC_WAVE:
	case SO_VACUUM_EXTREME:
	case GN_THORNS_TRAP:
	case SO_EARTHGRAVE:
	case SO_DIAMONDDUST:
	case SO_FIRE_INSIGNIA:
	case SO_WATER_INSIGNIA:
	case SO_WIND_INSIGNIA:
	case SO_EARTH_INSIGNIA:
	case KO_ZENKAI:
	case MH_LAVA_SLIDE:
	case MH_VOLCANIC_ASH:
	case MH_BLAST_FORGE:
	case MH_POISON_MIST:
	case MH_STEINWAND:
	case MH_XENO_SLASHER:
	case LG_KINGS_GRACE:
	case SJ_BOOKOFCREATINGSTAR:
	case RL_B_TRAP:
	case NPC_STORMGUST2:
	case AG_RAIN_OF_CRYSTAL:
	case AG_MYSTERY_ILLUSION:
	case AG_STRANTUM_TREMOR:
	case AG_TORNADO_STORM:
	case AG_FLORAL_FLARE_ROAD:
	case IG_CROSS_RAIN:
	case CD_PNEUMATICUS_PROCELLA:
	case ABC_ABYSS_STRIKE:
	case ABC_ABYSS_SQUARE:
	case WH_DEEPBLINDTRAP:
	case WH_SOLIDTRAP:
	case WH_SWIFTTRAP:
	case WH_FLAMETRAP:
	case BO_ACIDIFIED_ZONE_WATER:
	case BO_ACIDIFIED_ZONE_GROUND:
	case BO_ACIDIFIED_ZONE_WIND:
	case BO_ACIDIFIED_ZONE_FIRE:
	case EM_DIAMOND_STORM:
	case EM_LIGHTNING_LAND:
	case EM_VENOM_SWAMP:
	case EM_CONFLAGRATION:
	case EM_TERRA_DRIVE:
		flag|=1;//Set flag to 1 to prevent deleting ammo (it will be deleted on group-delete).
		[[fallthrough]];
	case GS_GROUNDDRIFT: //Ammo should be deleted right away.
	case GN_WALLOFTHORN:
	case GN_DEMONIC_FIRE:
		skill_unitsetting(src,skill_id,skill_lv,x,y,0);
		break;

	case WZ_ICEWALL:
	case NPC_CANE_OF_EVIL_EYE:
		flag|=1;
		if(skill_unitsetting(src,skill_id,skill_lv,x,y,0))
			clif_skill_poseffect(src,skill_id,skill_lv,x,y,tick);
		break;
	case RG_GRAFFITI:			/* Graffiti [Valaris] */
		skill_unitsetting(src,skill_id,skill_lv,x,y,0);
		flag|=1;
		break;
	case NPC_EARTHQUAKE:
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		skill_unitsetting(src, skill_id, skill_lv, x, y, 0);
		break;
#ifndef RENEWAL
	case HP_BASILICA:
		if( sc->getSCE(SC_BASILICA) ) {
			status_change_end(src, SC_BASILICA); // Cancel Basilica and return so requirement isn't consumed again
			return 0;
		} else { // Create Basilica. Start SC on caster. Unit timer start SC on others.
			if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) ) {
				if (sd)
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
				return 0;
			}
			skill_clear_unitgroup(src);
			skill_unitsetting(src,skill_id,skill_lv,x,y,0);
			flag|=1;
		}
		break;
#endif
#ifndef RENEWAL
	case CG_HERMODE:
		skill_clear_unitgroup(src);
		if ((sg = skill_unitsetting(src,skill_id,skill_lv,x,y,0)))
			sc_start4(src,src,SC_DANCING,100,
				skill_id,0,skill_lv,sg->group_id,skill_get_time(skill_id,skill_lv));
		flag|=1;
#endif
		break;
	case RG_CLEANER: // [Valaris]
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_graffitiremover,src->m,x-i,y-i,x+i,y+i,BL_SKILL,1);
		break;

	case SO_WARMER:
	case SO_CLOUD_KILL:
	case NPC_CLOUD_KILL:
		flag |= (skill_id == SO_WARMER) ? 8 : 4;
		skill_unitsetting(src,skill_id,skill_lv,x,y,0);
		break;

	case SU_CN_POWDERING:
	case SU_NYANGGRASS:
		if (sd && pc_checkskill(sd, SU_SPIRITOFLAND)) {
			if (skill_id == SU_CN_POWDERING)
				sc_start(src, src, SC_DORAM_FLEE2, 100, sd->status.base_level * 10 / 12, skill_get_time(SU_SPIRITOFLAND, 1));
			else
				sc_start(src, src, SC_DORAM_MATK, 100, sd->status.base_level, skill_get_time(SU_SPIRITOFLAND, 1));
		}
		flag |= 1;
		skill_unitsetting(src, skill_id, skill_lv, x, y, 0);
		break;

	case SU_CN_METEOR:
		if (sd) {
			if (pc_search_inventory(sd, skill_db.find(SU_CN_METEOR)->require.itemid[0]) >= 0)
				skill_id = SU_CN_METEOR2;
			if (pc_checkskill(sd, SU_SPIRITOFLAND))
				sc_start(src, src, SC_DORAM_SVSP, 100, 100, skill_get_time(SU_SPIRITOFLAND, 1));
		}
		[[fallthrough]];
	case WZ_METEOR:
	{
		int area = skill_get_splash(skill_id, skill_lv);
		short tmpx = 0, tmpy = 0;

		for (i = 1; i <= skill_get_time(skill_id, skill_lv) / skill_get_unit_interval(skill_id); i++) {
			// Creates a random Cell in the Splash Area
			tmpx = x - area + rnd() % (area * 2 + 1);
			tmpy = y - area + rnd() % (area * 2 + 1);
			skill_unitsetting(src, skill_id, skill_lv, tmpx, tmpy, flag + i * skill_get_unit_interval(skill_id));
		}
	}
	break;

	case AL_WARP:
		if(sd != nullptr) {
			std::vector<std::string> maps( MAX_MEMOPOINTS + 1 );

			maps.push_back( sd->status.save_point.map );

			if( skill_lv >= 2 ){
				maps.push_back( sd->status.memo_point[0].map );

				if( skill_lv >= 3 ){
					maps.push_back( sd->status.memo_point[1].map );

					if( skill_lv >= 4 ){
						maps.push_back( sd->status.memo_point[2].map );
					}
				}
			}

			clif_skill_warppoint( *sd, skill_id, skill_lv, maps );
		}
		if( sc && sc->getSCE(SC_CURSEDCIRCLE_ATKER) ) //Should only remove after the skill has been casted.
			status_change_end(src,SC_CURSEDCIRCLE_ATKER);
		return 0; // not to consume item.

	case MO_BODYRELOCATION:
		if (unit_movepos(src, x, y, 2, 1)) {
#if PACKETVER >= 20111005
			clif_snap(src, src->x, src->y);
#else
			clif_skill_poseffect(src,skill_id,skill_lv,src->x,src->y,tick);
#endif
			if (sd)
				skill_blockpc_start (sd, MO_EXTREMITYFIST, 2000);
		}
		break;
	case NJ_SHADOWJUMP:
		if( map_getcell(src->m,x,y,CELL_CHKREACH) && skill_check_unit_movepos(5, src, x, y, 1, 0) ) //You don't move on GVG grounds.
			clif_blown(src);
		status_change_end(src, SC_HIDING);
		break;
	case AM_SPHEREMINE:
	case AM_CANNIBALIZE:
		{
			int summons[5] = { MOBID_G_MANDRAGORA, MOBID_G_HYDRA, MOBID_G_FLORA, MOBID_G_PARASITE, MOBID_G_GEOGRAPHER };
			int class_ = skill_id==AM_SPHEREMINE?MOBID_MARINE_SPHERE:summons[skill_lv-1];
			enum mob_ai ai = (skill_id == AM_SPHEREMINE) ? AI_SPHERE : AI_FLORA;
			struct mob_data *md;

			// Correct info, don't change any of this! [celest]
			md = mob_once_spawn_sub(src, src->m, x, y, status_get_name(src), class_, "", SZ_SMALL, ai);
			if (md) {
				md->master_id = src->id;
				md->special_state.ai = ai;
				if( md->deletetimer != INVALID_TIMER )
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer (gettick() + skill_get_time(skill_id,skill_lv), mob_timer_delete, md->bl.id, 0);
				mob_spawn (md); //Now it is ready for spawning.
			}
		}
		break;

	// Slim Pitcher [Celest]
	case CR_SLIMPITCHER:
		if (sd) {
			int i_lv = 0, j = 0;
			struct s_skill_condition require = skill_get_requirement(sd, skill_id, skill_lv);
			i_lv = skill_lv%11 - 1;
			j = pc_search_inventory(sd, require.itemid[i_lv]);
			if (j < 0 || require.itemid[i_lv] <= 0 || sd->inventory_data[j] == nullptr || sd->inventory.u.items_inventory[j].amount < require.amount[i_lv])
			{
				clif_skill_fail( *sd, skill_id );
				return 1;
			}
			potion_flag = 1;
			potion_hp = 0;
			potion_sp = 0;
			run_script(sd->inventory_data[j]->script,0,sd->bl.id,0);
			potion_flag = 0;
			//Apply skill bonuses
			i_lv = pc_checkskill(sd,CR_SLIMPITCHER)*10
				+ pc_checkskill(sd,AM_POTIONPITCHER)*10
				+ pc_checkskill(sd,AM_LEARNINGPOTION)*5
				+ pc_skillheal_bonus(sd, skill_id);

			potion_hp = potion_hp * (100+i_lv)/100;
			potion_sp = potion_sp * (100+i_lv)/100;

			// Final heal increased by HPlus.
			// Is this the right place for this??? [Rytech]
			// Can HPlus also affect SP recovery???
			status_data* sstatus = status_get_status_data(*src);

			if (sstatus && sstatus->hplus > 0) {
				potion_hp += potion_hp * sstatus->hplus / 100;
				potion_sp += potion_sp * sstatus->hplus / 100;
			}

			if(potion_hp > 0 || potion_sp > 0) {
				i_lv = skill_get_splash(skill_id, skill_lv);
				map_foreachinallarea(skill_area_sub,
					src->m,x-i_lv,y-i_lv,x+i_lv,y+i_lv,BL_CHAR,
					src,skill_id,skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
					skill_castend_nodamage_id);
			}
		} else {
			struct item_data *item = itemdb_search(skill_db.find(skill_id)->require.itemid[skill_lv - 1]);
			int id = skill_get_max(CR_SLIMPITCHER) * 10;

			potion_flag = 1;
			potion_hp = 0;
			potion_sp = 0;
			run_script(item->script,0,src->id,0);
			potion_flag = 0;
			potion_hp = potion_hp * (100+id)/100;
			potion_sp = potion_sp * (100+id)/100;

			if(potion_hp > 0 || potion_sp > 0) {
				id = skill_get_splash(skill_id, skill_lv);
				map_foreachinallarea(skill_area_sub,
					src->m,x-id,y-id,x+id,y+id,BL_CHAR,
					src,skill_id,skill_lv,tick,flag|BCT_PARTY|BCT_GUILD|1,
						skill_castend_nodamage_id);
			}
		}
		break;

	case HW_GANBANTEIN:
		if (rnd()%100 < 80) {
			int dummy = 1;
			clif_skill_poseffect(src,skill_id,skill_lv,x,y,tick);
			i = skill_get_splash(skill_id, skill_lv);
			map_foreachinallarea(skill_cell_overlap, src->m, x-i, y-i, x+i, y+i, BL_SKILL, HW_GANBANTEIN, &dummy, src);
		} else {
			if (sd) clif_skill_fail( *sd, skill_id );
			return 1;
		}
		break;

#ifndef RENEWAL
	case HW_GRAVITATION:
		if ((sg = skill_unitsetting(src,skill_id,skill_lv,x,y,0)))
			sc_start4(src,src,type,100,skill_lv,0,BCT_SELF,sg->group_id,skill_get_time(skill_id,skill_lv));
		flag|=1;
		break;
#endif

	// Plant Cultivation [Celest]
	case CR_CULTIVATION:
		if (sd) {
			if( map_count_oncell(src->m,x,y,BL_CHAR,0) > 0 )
			{
				clif_skill_fail( *sd, skill_id );
				return 1;
			}
			clif_skill_poseffect(src,skill_id,skill_lv,x,y,tick);
			if (rnd()%100 < 50) {
				clif_skill_fail( *sd, skill_id );
			} else {
				TBL_MOB* md = nullptr;
				int t, mob_id;

				if (skill_lv == 1)
					mob_id = MOBID_BLACK_MUSHROOM + rnd() % 2;
				else {
					int rand_val = rnd() % 100;

					if (rand_val < 30)
						mob_id = MOBID_GREEN_PLANT;
					else if (rand_val < 55)
						mob_id = MOBID_RED_PLANT;
					else if (rand_val < 80)
						mob_id = MOBID_YELLOW_PLANT;
					else if (rand_val < 90)
						mob_id = MOBID_WHITE_PLANT;
					else if (rand_val < 98)
						mob_id = MOBID_BLUE_PLANT;
					else
						mob_id = MOBID_SHINING_PLANT;
				}

				md = mob_once_spawn_sub(src, src->m, x, y, "--ja--", mob_id, "", SZ_SMALL, AI_NONE);
				if (!md)
					break;
				if ((t = skill_get_time(skill_id, skill_lv)) > 0)
				{
					if( md->deletetimer != INVALID_TIMER )
						delete_timer(md->deletetimer, mob_timer_delete);
					md->deletetimer = add_timer (tick + t, mob_timer_delete, md->bl.id, 0);
				}
				mob_spawn(md);
			}
		}
		break;

	case SG_SUN_WARM:
	case SG_MOON_WARM:
	case SG_STAR_WARM:
		skill_clear_unitgroup(src);
		if ((sg = skill_unitsetting(src,skill_id,skill_lv,src->x,src->y,0)))
			sc_start4(src,src,type,100,skill_lv,0,0,sg->group_id,skill_get_time(skill_id,skill_lv));
		flag|=1;
		break;

	case PA_GOSPEL:
		if (sce && sce->val4 == BCT_SELF)
		{
			status_change_end(src, SC_GOSPEL);
			return 0;
		}
		else
		{
			sg = skill_unitsetting(src,skill_id,skill_lv,src->x,src->y,0);
			if (!sg) break;
			if (sce)
				status_change_end(src, type); //Was under someone else's Gospel. [Skotlex]
			sc_start4(src,src,type,100,skill_lv,0,sg->group_id,BCT_SELF,skill_get_time(skill_id,skill_lv));
			clif_skill_poseffect(src, skill_id, skill_lv, 0, 0, tick); // PA_GOSPEL music packet
		}
		break;
	case NJ_TATAMIGAESHI:
		if (skill_unitsetting(src,skill_id,skill_lv,src->x,src->y,0))
			sc_start(src,src,type,100,skill_lv,skill_get_time2(skill_id,skill_lv));
		break;

	case AM_RESURRECTHOMUN:	//[orn]
		if (sd)
		{
			if (!hom_ressurect(sd, 20*skill_lv, x, y))
			{
				clif_skill_fail( *sd, skill_id );
				break;
			}
		}
		break;

	case AC_SHOWER:
		status_change_end(src, SC_CAMOUFLAGE);
		[[fallthrough]];
	case MA_SHOWER:
	case NC_COLDSLOWER:
	case RK_DRAGONBREATH:
	case RK_DRAGONBREATH_WATER:
	case NPC_DRAGONBREATH:
	case WL_FROSTMISTY:
	case RL_HAMMER_OF_GOD:
		// Cast center might be relevant later (e.g. for knockback direction)
		skill_area_temp[4] = x;
		skill_area_temp[5] = y;
		i = skill_get_splash(skill_id,skill_lv);
		map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR|BL_SKILL,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;

	case SO_ARRULLO:
		i = skill_get_splash(skill_id,skill_lv);
		map_foreachinallarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,
			src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_nodamage_id);
		break;

	case GC_POISONSMOKE:
		if( !(sc && sc->getSCE(SC_POISONINGWEAPON)) ) {
			if( sd )
				clif_skill_fail( *sd, skill_id, USESKILL_FAIL_GC_POISONINGWEAPON );
			return 0;
		}
		clif_skill_damage(src,src,tick,status_get_amotion(src),0,-30000,1,skill_id,skill_lv,DMG_SINGLE);
		skill_unitsetting(src, skill_id, skill_lv, x, y, flag);
		break;

	case AB_EPICLESIS:
		if( (sg = skill_unitsetting(src, skill_id, skill_lv, x, y, 0)) ) {
			i = skill_get_splash(skill_id, skill_lv);
			map_foreachinallarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR, src, ALL_RESURRECTION, 1, tick, flag|BCT_NOENEMY|1,skill_castend_nodamage_id);
		}
		break;

	case WL_EARTHSTRAIN:
		{
			int w, wave = skill_lv + 4, dir = map_calc_dir(src,x,y);
			int sx = x = src->x, sy = y = src->y; // Store first caster's location to avoid glitch on unit setting

			for( w = 1; w <= wave; w++ )
			{
				switch( dir ){
					case 0: case 1: case 7: sy = y + w; break;
					case 3: case 4: case 5: sy = y - w; break;
					case 2: sx = x - w; break;
					case 6: sx = x + w; break;
				}
				skill_addtimerskill(src,gettick() + (140 * w),0,sx,sy,skill_id,skill_lv,dir,flag&2);
			}
		}
		break;

	case RA_DETONATOR:
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_detonator, src->m, x-i, y-i, x+i, y+i, BL_SKILL, src);
		clif_skill_damage(src, src, tick, status_get_amotion(src), 0, -30000, 1, skill_id, skill_lv, DMG_SINGLE);
		break;

	case NC_NEUTRALBARRIER:
	case NC_STEALTHFIELD:
		if( (sc->getSCE(SC_NEUTRALBARRIER_MASTER) && skill_id == NC_NEUTRALBARRIER) || (sc->getSCE(SC_STEALTHFIELD_MASTER) && skill_id == NC_STEALTHFIELD) ) {
			skill_clear_unitgroup(src);
			return 0;
		}
		skill_clear_unitgroup(src); // To remove previous skills - cannot used combined
		if( (sg = skill_unitsetting(src,skill_id,skill_lv,src->x,src->y,0)) != nullptr ) {
			sc_start2(src,src,skill_id == NC_NEUTRALBARRIER ? SC_NEUTRALBARRIER_MASTER : SC_STEALTHFIELD_MASTER,100,skill_lv,sg->group_id,skill_get_time(skill_id,skill_lv));
		}
		break;

	case NC_SILVERSNIPER:
		{
			struct mob_data *md;

			md = mob_once_spawn_sub(src, src->m, x, y, status_get_name(src), MOBID_SILVERSNIPER, "", SZ_SMALL, AI_NONE);
			if( md ) {
				md->master_id = src->id;
				md->special_state.ai = AI_FAW;
				if( md->deletetimer != INVALID_TIMER )
					delete_timer(md->deletetimer, mob_timer_delete);
				md->deletetimer = add_timer (gettick() + skill_get_time(skill_id, skill_lv), mob_timer_delete, md->bl.id, 0);
				mob_spawn(md);
			}
		}
		break;

	case NC_MAGICDECOY:
		if( sd ) clif_magicdecoy_list( *sd, skill_lv, x, y );
		break;

	case SC_FEINTBOMB: {
			std::shared_ptr<s_skill_unit_group> group = skill_unitsetting(src,skill_id,skill_lv,x,y,0); // Set bomb on current Position

			if( group == nullptr || group->unit == nullptr ) {
				if (sd)
					clif_skill_fail( *sd, skill_id );
				return 1;
			}
			map_foreachinallrange(unit_changetarget, src, AREA_SIZE, BL_MOB, src, &group->unit->bl); // Release all targets against the caster
			skill_blown(src, src, skill_get_blewcount(skill_id, skill_lv), unit_getdir(src), BLOWN_IGNORE_NO_KNOCKBACK); // Don't stop the caster from backsliding if special_state.no_knockback is active
			clif_skill_nodamage(src, *src, skill_id, skill_lv, false);
			sc_start(src, src, type, 100, skill_lv, skill_get_time(skill_id, skill_lv));
		}
		break;

	case SC_ESCAPE:
		skill_unitsetting(src, skill_id, skill_lv, x, y, 0);
		skill_blown(src, src, skill_get_blewcount(skill_id, skill_lv), unit_getdir(src), BLOWN_IGNORE_NO_KNOCKBACK); // Don't stop the caster from backsliding if special_state.no_knockback is active
		clif_skill_nodamage(src,*src,skill_id,skill_lv);
		flag |= 1;
		break;

	case LG_BANDING:
		if( sc && sc->getSCE(SC_BANDING) )
			status_change_end(src,SC_BANDING);
		else if( (sg = skill_unitsetting(src,skill_id,skill_lv,src->x,src->y,0)) != nullptr )
			sc_start4(src,src,SC_BANDING,100,skill_lv,0,0,sg->group_id,skill_get_time(skill_id,skill_lv));
		clif_skill_nodamage(src,*src,skill_id,skill_lv);
		break;

	case WM_DOMINION_IMPULSE:
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_active_reverberation, src->m, x-i, y-i, x+i,y+i,BL_SKILL);
		break;

	case WM_GREAT_ECHO:
		i = skill_get_splash(skill_id,skill_lv);
		map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		break;

	case WM_SEVERE_RAINSTORM:
		flag |= 1;
		if (sd)
			sd->canequip_tick = tick + skill_get_time(skill_id, skill_lv); // Can't switch equips for the duration of the skill.
		skill_unitsetting(src,skill_id,skill_lv,x,y,0);
		break;

	case GN_CRAZYWEED: {
			int area = skill_get_splash(GN_CRAZYWEED_ATK, skill_lv);
			for( i = 0; i < 3 + (skill_lv/2); i++ ) {
				int x1 = x - area + rnd()%(area * 2 + 1);
				int y1 = y - area + rnd()%(area * 2 + 1);
				skill_addtimerskill(src,tick+i*150,0,x1,y1,GN_CRAZYWEED_ATK,skill_lv,-1,0);
			}
		}
		break;
	case GN_FIRE_EXPANSION: {
			struct unit_data* ud = unit_bl2ud(src);

			if (!ud) break;

			auto predicate = [x, y](std::shared_ptr<s_skill_unit_group> sg) { auto* su = sg->unit; return sg->skill_id == GN_DEMONIC_FIRE && distance_xy(x, y, su->bl.x, su->bl.y) < 4; };
			auto it = std::find_if(ud->skillunits.begin(), ud->skillunits.end(), predicate);
			if (it != ud->skillunits.end()) {
				auto* unit_group = it->get();
				skill_unit* su = unit_group->unit;

				switch (skill_lv) {
				case 1: {
					// TODO:
					int duration = (int)(unit_group->limit - DIFF_TICK(tick, unit_group->tick));

					skill_delunit(su);
					skill_unitsetting(src, GN_DEMONIC_FIRE, 1, x, y, duration);
					flag |= 1;
				}
						break;
				case 2:
					map_foreachinallarea(skill_area_sub, src->m, su->bl.x - 2, su->bl.y - 2, su->bl.x + 2, su->bl.y + 2, BL_CHAR, src, GN_DEMONIC_FIRE, skill_lv + 20, tick, flag | BCT_ENEMY | SD_LEVEL | 1, skill_castend_damage_id);
					if (su != nullptr)
						skill_delunit(su);
					break;
				case 3:
					skill_delunit(su);
					skill_unitsetting(src, GN_FIRE_EXPANSION_SMOKE_POWDER, 1, x, y, 0);
					flag |= 1;
					break;
				case 4:
					skill_delunit(su);
					skill_unitsetting(src, GN_FIRE_EXPANSION_TEAR_GAS, 1, x, y, 0);
					flag |= 1;
					break;
				case 5: {
					uint16 acid_lv = 5; // Cast at Acid Demonstration at level 5 unless the user has a higher level learned.

					if (sd && pc_checkskill(sd, CR_ACIDDEMONSTRATION) > 5)
						acid_lv = pc_checkskill(sd, CR_ACIDDEMONSTRATION);
					map_foreachinallarea(skill_area_sub, src->m, su->bl.x - 2, su->bl.y - 2, su->bl.x + 2, su->bl.y + 2, BL_CHAR, src, GN_FIRE_EXPANSION_ACID, acid_lv, tick, flag | BCT_ENEMY | SD_LEVEL | 1, skill_castend_damage_id);
					if (su != nullptr)
						skill_delunit(su);
				}
					break;
				}
			}
		}
		break;

	case SO_FIREWALK:
	case SO_ELECTRICWALK:
	case NPC_FIREWALK:
	case NPC_ELECTRICWALK:
		if( sc && sc->getSCE(type) )
			status_change_end(src,type);
		sc_start2(src, src, type, 100, skill_id, skill_lv, skill_get_time(skill_id, skill_lv));
		break;

	case KO_MAKIBISHI:
		for( i = 0; i < (skill_lv+2); i++ ) {
			x = src->x - 1 + rnd()%3;
			y = src->y - 1 + rnd()%3;
			skill_unitsetting(src,skill_id,skill_lv,x,y,0);
		}
		break;
	case KO_MUCHANAGE: {
			int rate = 0;

			status_data* sstatus = status_get_status_data(*src);
			i = skill_get_splash(skill_id,skill_lv);
			rate = (100 - (1000 / (sstatus->dex + sstatus->luk) * 5)) * (skill_lv / 2 + 5) / 10;
			if( rate < 0 )
				rate = 0;
			skill_area_temp[0] = map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,src,skill_id,skill_lv,tick,BCT_ENEMY,skill_area_sub_count);
			if( rnd()%100 < rate )
				map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|1,skill_castend_damage_id);
		}
		break;

	case RL_FALLEN_ANGEL:
		if (unit_movepos(src,x,y,1,1)) {
			clif_snap(src, src->x, src->y);
			sc_start(src, src, type, 100, skill_id, skill_get_time(skill_id, skill_lv));
		} else {
			if (sd)
				clif_skill_fail( *sd, skill_id );
		}
		break;
	case RL_FIRE_RAIN: {
			int w, wave = skill_lv + 5, dir = map_calc_dir(src,x,y);
			int sx = x = src->x, sy = y = src->y;

			for (w = 0; w <= wave; w++) {
				switch (dir) {
					case DIR_NORTH:
					case DIR_NORTHWEST:
					case DIR_NORTHEAST:
						sy = y + w;
						break;
					case DIR_WEST:
						sx = x - w;
						break;
					case DIR_SOUTHWEST:
					case DIR_SOUTH:
					case DIR_SOUTHEAST:
						sy = y - w;
						break;
					case DIR_EAST:
						sx = x + w;
						break;
				}
				skill_addtimerskill(src,gettick() + (80 * w),0,sx,sy,skill_id,skill_lv,dir,flag);
			}
		}
		break;

	case NPC_MAGMA_ERUPTION:
	case NC_MAGMA_ERUPTION:
		// 1st, AoE 'slam' damage
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR,
			src, skill_id, skill_lv, tick, flag|BCT_ENEMY|SD_ANIMATION|1, skill_castend_damage_id);
		// 2nd, AoE 'eruption' unit
		skill_addtimerskill(src,tick + status_get_amotion(src) * 2,0,x,y,skill_id,skill_lv,0,flag);
		break;
	case SU_LOPE:
		{
			uint8 dir = map_calc_dir(src, x, y);

			// Fails on noteleport maps, except for GvG and BG maps
			if (map_getmapflag(src->m, MF_NOTELEPORT) && !(map_getmapflag(src->m, MF_BATTLEGROUND) || map_flag_gvg2(src->m))) {
				x = src->x;
				y = src->y;
			}

			clif_skill_nodamage(src, *src, skill_id, skill_lv);
			if (!map_count_oncell(src->m, x, y, BL_PC|BL_NPC|BL_MOB, 0) && map_getcell(src->m, x, y, CELL_CHKREACH) && unit_movepos(src, x, y, 1, 0))
				clif_blown(src);
		}
		break;

	case AG_ASTRAL_STRIKE:
		i = skill_get_splash(skill_id, skill_lv);
		map_foreachinallarea(skill_area_sub, src->m, x-i, y-i, x+i, y+i, BL_CHAR, src, skill_id, skill_lv, tick, flag|BCT_ENEMY|1, skill_castend_damage_id);
		flag |= 1;
		skill_unitsetting(src, skill_id, skill_lv, x, y, 0);
		break;

	case AG_VIOLENT_QUAKE:
	case AG_ALL_BLOOM: {
			int area = skill_get_splash(skill_id, skill_lv);
			int unit_time = skill_get_time(skill_id, skill_lv);
			int unit_interval = skill_get_unit_interval(skill_id);
			uint16 tmpx = 0, tmpy = 0, sub_skill = 0, climax_lv = 0;

			// Grab Climax's effect level if active.
			// This affects the behavior of certain skills in certain ways.
			if (sc && sc->getSCE(SC_CLIMAX))
				climax_lv = sc->getSCE(SC_CLIMAX)->val1;

			if (skill_id == AG_VIOLENT_QUAKE) {
				sub_skill = AG_VIOLENT_QUAKE_ATK;

				// Fixes rising rocks spawn area to 7x7.
				if (climax_lv == 5)
					area = 3;
			} else { // AG_ALL_BLOOM
				sub_skill = AG_ALL_BLOOM_ATK;

				if (climax_lv == 1) { // Rose buds spawn at double the speed.
					unit_time /= 2;
					unit_interval /= 2;
				}
			}

			// Displays the earthquake / flower garden.
			skill_unitsetting(src, skill_id, skill_lv, x, y, 0);

			if (climax_lv == 4) { // Deals no damage and instead inflicts a status on the enemys in range.
				i = skill_get_splash(skill_id, skill_lv);
				map_foreachinallarea(skill_area_sub, src->m, x - i, y - i, x + i, y + i, BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1, skill_castend_nodamage_id);
			} else for (i = 1; i <= unit_time / unit_interval; i++) { // Spawn the rising rocks / rose buds on random spots at seperate intervals
				tmpx = x - area + rnd() % (area * 2 + 1);
				tmpy = y - area + rnd() % (area * 2 + 1);
				skill_unitsetting(src, sub_skill, skill_lv, tmpx, tmpy, flag + i * unit_interval);

				if ((skill_id == AG_VIOLENT_QUAKE && climax_lv == 1) || (skill_id == AG_ALL_BLOOM && climax_lv == 2)) { // Spwan a 2nd rising rock / rose bud along with the 1st one.
					tmpx = x - area + rnd() % (area * 2 + 1);
					tmpy = y - area + rnd() % (area * 2 + 1);
					skill_unitsetting(src, sub_skill, skill_lv, tmpx, tmpy, flag + i * unit_interval);
				}
			}

			// One final attack the size of the flower garden is dealt after
			// all rose buds explode if Climax level 5 is active.
			if (skill_id == AG_ALL_BLOOM && climax_lv == 5)
				skill_unitsetting(src, AG_ALL_BLOOM_ATK2, skill_lv, x, y, flag + i * unit_interval);
		}
		break;

	case NPC_RAINOFMETEOR:
		{
			int area = skill_get_splash(skill_id, skill_lv);
			short tmpx = 0, tmpy = 0;

			for (i = 1; i <= (skill_get_time(skill_id, skill_lv)/skill_get_unit_interval(skill_id)); i++) {
				// Casts a double meteor in the first interval.
				if (i == 1) {
					// The first meteor is at the center
					skill_unitsetting(src, skill_id, skill_lv, x, y, flag+skill_get_unit_interval(skill_id));

					// The second meteor is near the first
					tmpx = x - 1 + rnd()%3;
					tmpy = y - 1 + rnd()%3;
					skill_unitsetting(src, skill_id, skill_lv, tmpx, tmpy, flag+skill_get_unit_interval(skill_id));
				}
				else {	// Casts 1 meteor per interval in the splash area
					tmpx = x - area + rnd()%(area * 2 + 1);
					tmpy = y - area + rnd()%(area * 2 + 1);
					skill_unitsetting(src, skill_id, skill_lv, tmpx, tmpy, flag+i*skill_get_unit_interval(skill_id));
				}
			}
		}
		break;

	case HN_JACK_FROST_NOVA:
	case HN_GROUND_GRAVITATION: {
			if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) ) {
				if( sd != nullptr ){
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
				}

				return 0;
			}

			int splash = skill_get_splash(skill_id, skill_lv);

			map_foreachinarea(skill_area_sub, src->m, x - splash, y - splash, x + splash, y + splash, BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | SKILL_ALTDMG_FLAG | 1, skill_castend_damage_id);
			skill_unitsetting(src, skill_id, skill_lv, x, y, flag);

			for (i = 1; i <= (skill_get_time(skill_id, skill_lv) / skill_get_unit_interval(skill_id)); i++) {
				skill_addtimerskill(src, tick + (t_tick)i*skill_get_unit_interval(skill_id), 0, x, y, skill_id, skill_lv, 0, flag);
			}
		}
		break;

	case HN_METEOR_STORM_BUSTER: {
			if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) ) {
				if( sd != nullptr ){
					clif_skill_fail( *sd, skill_id, USESKILL_FAIL );
				}

				return 0;
			}

			int splash = skill_get_splash(skill_id, skill_lv);

			map_foreachinarea(skill_area_sub, src->m, x - splash, y - splash, x + splash, y + splash, BL_CHAR, src, skill_id, skill_lv, tick, flag | BCT_ENEMY | SD_SPLASH | SKILL_ALTDMG_FLAG | 1, skill_castend_damage_id);
			skill_unitsetting(src, skill_id, skill_lv, x, y, skill_get_unit_interval(skill_id));

			for (i = 1; i <= (skill_get_time(skill_id, skill_lv) / skill_get_time2(skill_id, skill_lv)); i++) {
				skill_addtimerskill(src, tick + (t_tick)i*skill_get_time2(skill_id, skill_lv), 0, x, y, skill_id, skill_lv, 0, flag);
			}
		}
		break;

	case NW_WILD_FIRE:
		i = skill_get_splash(skill_id, skill_lv);
		if (sd && sd->status.weapon == W_GRENADE)
			i += 2;
		map_foreachinallarea(skill_area_sub,
			src->m, x - i, y - i, x + i, y + i, BL_CHAR,
			src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1,
			skill_castend_damage_id);
		if (sc && sc->getSCE(SC_INTENSIVE_AIM_COUNT))
			status_change_end(src, SC_INTENSIVE_AIM_COUNT);
		break;
	case NW_HASTY_FIRE_IN_THE_HOLE:
		i = skill_get_splash(skill_id, skill_lv);
		if (flag & 1){
			i++;
		}
		if (flag & 2){
			i++;
		}
		map_foreachinallarea(skill_area_sub,
			src->m, x - i, y - i, x + i, y + i, BL_CHAR,
			src, skill_id, skill_lv, tick, flag | BCT_ENEMY | 1,
			skill_castend_damage_id);
		if (!(flag & 1)) {
			skill_addtimerskill(src, tick + 300, 0, x, y, skill_id, skill_lv, 0, flag | 1 | SKILL_NOCONSUME_REQ);
			skill_addtimerskill(src, tick + 600, 0, x, y, skill_id, skill_lv, 0, flag | 3 | SKILL_NOCONSUME_REQ);
		}
		break;
	case NW_GRENADES_DROPPING: {
			uint16 splash = skill_get_splash(skill_id, skill_lv);
			uint16 tmpx = rnd_value( x - splash, x + splash );
			uint16 tmpy = rnd_value( y - splash, y + splash );
			skill_unitsetting(src, skill_id, skill_lv, tmpx, tmpy, flag);
			for (i = 0; i <= (skill_get_time(skill_id, skill_lv) / skill_get_unit_interval(skill_id)); i++) {
				skill_addtimerskill(src, tick + (t_tick)i*skill_get_unit_interval(skill_id), 0, x, y, skill_id, skill_lv, 0, flag);
			}
		} break;
	case NW_MISSION_BOMBARD:
		i = skill_get_splash(skill_id,skill_lv);
		map_foreachinarea(skill_area_sub,src->m,x-i,y-i,x+i,y+i,BL_CHAR|BL_SKILL,src,skill_id,skill_lv,tick,flag|BCT_ENEMY|SKILL_ALTDMG_FLAG|1,skill_castend_damage_id);
		skill_unitsetting(src, skill_id, skill_lv, x, y, flag);

		for (i = 1; i <= (skill_get_time(skill_id, skill_lv) / skill_get_unit_interval(skill_id)); i++) {
			skill_addtimerskill(src, tick + (t_tick)i*skill_get_unit_interval(skill_id), 0, x, y, skill_id, skill_lv, 0, flag);
		}
		break;

	default:
		ShowWarning("skill_castend_pos2: Unknown skill used:%d\n",skill_id);
		return 1;
	}

	if( sc && sc->getSCE(SC_CURSEDCIRCLE_ATKER) ) //Should only remove after the skill has been casted.
		status_change_end(src,SC_CURSEDCIRCLE_ATKER);

	if( sd )
	{// ensure that the skill last-cast tick is recorded
		sd->canskill_tick = gettick();

		if( sd->state.arrow_atk && !(flag&1) )
		{// consume arrow if this is a ground skill
			battle_consume_ammo(sd, skill_id, skill_lv);
		}
		skill_onskillusage(sd, nullptr, skill_id, tick);
		// perform skill requirement consumption
		if (!(flag&SKILL_NOCONSUME_REQ))
			skill_consume_requirement(sd,skill_id,skill_lv,2);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int skill_castend_map (map_session_data *sd, uint16 skill_id, const char *mapname)
{
	nullpo_ret(sd);

//Simplify skill_failed code.
#define skill_failed(sd) { sd->menuskill_id = sd->menuskill_val = 0; }
	if(skill_id != sd->menuskill_id)
		return 0;

	if( sd->bl.prev == nullptr || pc_isdead(sd) ) {
		skill_failed(sd);
		return 0;
	}

	if( sd->sc.cant.cast ) {
		skill_failed(sd);
		return 0;
	}

	pc_stop_attack(sd);

	if(battle_config.skill_log && battle_config.skill_log&BL_PC)
		ShowInfo("PC %d skill castend skill =%d map=%s\n",sd->bl.id,skill_id,mapname);

	if(strcmp(mapname,"cancel")==0) {
		skill_failed(sd);
		return 0;
	}

	switch(skill_id)
	{
	case AL_TELEPORT:
	case ALL_ODINS_RECALL:
		//The storage window is closed automatically by the client when there's
		//any kind of map change, so we need to restore it automatically
		//bugreport:8027
		if(strcmp(mapname,"Random") == 0)
			pc_randomwarp(sd,CLR_TELEPORT);
		else if (sd->menuskill_val > 1 || skill_id == ALL_ODINS_RECALL) //Need lv2 to be able to warp here.
			pc_setpos( sd, mapindex_name2id( sd->status.save_point.map ),sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT );

		clif_refresh_storagewindow(sd);
		break;

	case AL_WARP:
		if( sd != nullptr ){
			const struct s_point_str *p[4];
			std::shared_ptr<s_skill_unit_group> group;
			int i, lv, wx, wy;
			int maxcount=0;
			int x,y;
			unsigned short mapindex;

			mapindex  = mapindex_name2id((char*)mapname);
			if(!mapindex) { //Given map not found?
				clif_skill_fail( *sd, skill_id );
				skill_failed(sd);
				return 0;
			}
			p[0] = &sd->status.save_point;
			p[1] = &sd->status.memo_point[0];
			p[2] = &sd->status.memo_point[1];
			p[3] = &sd->status.memo_point[2];

			if((maxcount = skill_get_maxcount(skill_id, sd->menuskill_val)) > 0) {
				unit_skillunit_maxcount(sd->ud, skill_id, maxcount);

				if (maxcount == 0) {
					clif_skill_fail( *sd, skill_id );
					skill_failed(sd);
					return 0;
				}
			}

			lv = sd->skillitem==skill_id?sd->skillitemlv:pc_checkskill(sd,skill_id);
			wx = sd->menuskill_val>>16;
			wy = sd->menuskill_val&0xffff;

			if( lv <= 0 ) return 0;
			if( lv > 4 ) lv = 4; // crash prevention

			// check if the chosen map exists in the memo list
			ARR_FIND( 0, lv, i, strncmp( p[i]->map, mapname, sizeof( p[i]->map ) ) == 0 );
			if( i < lv ) {
				x=p[i]->x;
				y=p[i]->y;
			} else {
				skill_failed(sd);
				return 0;
			}

			if(!skill_check_condition_castend(*sd, sd->menuskill_id, lv))
			{  // This checks versus skill_id/skill_lv...
				skill_failed(sd);
				return 0;
			}

			skill_consume_requirement(sd,sd->menuskill_id,lv,2);
			sd->skillitem = sd->skillitemlv = sd->skillitem_keep_requirement = 0; // Clear data that's skipped in 'skill_castend_pos' [Inkfish]

			if((group=skill_unitsetting(&sd->bl,skill_id,lv,wx,wy,0))==nullptr) {
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
	if (!(target->val2 & src->val2 & ~(1 << UF_ENSEMBLE))) //They don't match (song + dance) is valid.
		return 0;

	if (flag) //Set dissonance
		target->val2 |= (1 << UF_ENSEMBLE); //Add ensemble to signal this unit is overlapping.
	else //Remove dissonance
		target->val2 &= ~(1 << UF_ENSEMBLE);

	skill_getareachar_skillunit_visibilty(target, AREA);

	return 1;
}

//Does the song/dance overlapping -> dissonance check. [Skotlex]
//When flag is 0, this unit is about to be removed, cancel the dissonance effect
//When 1, this unit has been positioned, so start the cancel effect.
int skill_dance_overlap(struct skill_unit* unit, int flag)
{
	if (!unit || !unit->group || !(unit->group->state.song_dance&0x1))
		return 0;
	if (!flag && !(unit->val2&(1 << UF_ENSEMBLE)))
		return 0; //Nothing to remove, this unit is not overlapped.

	if (unit->val1 != unit->group->skill_id)
	{	//Reset state
		unit->val1 = unit->group->skill_id;
		unit->val2 &= ~(1 << UF_ENSEMBLE);
	}

	return map_foreachincell(skill_dance_overlap_sub, unit->bl.m,unit->bl.x,unit->bl.y,BL_SKILL, unit,flag);
}

/**
 * Converts this group information so that it is handled as a Dissonance or Ugly Dance cell.
 * @param unit Skill unit data (from BA_DISSONANCE or DC_UGLYDANCE)
 * @param flag 0 Convert
 * @param flag 1 Revert
 * @return true success
 * @TODO: This should be completely removed later and rewritten
 *	The entire execution of the overlapping songs instances is dirty and hacked together
 *	Overlapping cells should be checked on unit entry, not infinitely loop checked causing 1000's of executions a song/dance
 */
static bool skill_dance_switch(struct skill_unit* unit, int flag)
{
	static int prevflag = 1;  // by default the backup is empty
	static s_skill_unit_group backup;
	std::shared_ptr<s_skill_unit_group> group;

	if( unit == nullptr || (group = unit->group) == nullptr )
		return false;

	//val2&(1 << UF_ENSEMBLE) is a hack to indicate dissonance
	if ( !((group->state.song_dance&0x1) && (unit->val2&(1 << UF_ENSEMBLE))) )
		return false;

	if( flag == prevflag ) { //Protection against attempts to read an empty backup/write to a full backup
		ShowError("skill_dance_switch: Attempted to %s (skill_id=%d, skill_lv=%d, src_id=%d).\n",
			flag ? "read an empty backup" : "write to a full backup",
			group->skill_id, group->skill_lv, group->src_id);
		return false;
	}

	prevflag = flag;

	if (!flag) { //Transform
		uint16 skill_id = unit->val2&(1 << UF_SONG) ? BA_DISSONANCE : DC_UGLYDANCE;

		// backup
		backup.skill_id    = group->skill_id;
		backup.skill_lv    = group->skill_lv;
		backup.unit_id     = group->unit_id;
		backup.target_flag = group->target_flag;
		backup.bl_flag     = group->bl_flag;
		backup.interval    = group->interval;

		// replace
		group->skill_id    = skill_id;
		group->skill_lv    = 1;
		group->unit_id     = skill_get_unit_id(skill_id);
		group->target_flag = skill_get_unit_target(skill_id);
		group->bl_flag     = skill_get_unit_bl_target(skill_id);
		group->interval    = skill_get_unit_interval(skill_id);
	} else { //Restore
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
 * Initializes and sets a ground skill / skill unit. Usually called after skill_casted_pos() or skill_castend_map()
 * @param src Object that triggers the skill
 * @param skill_id Skill ID
 * @param skill_lv Skill level of used skill
 * @param x Position x
 * @param y Position y
 * @param flag &1: Used to determine when the skill 'morphs' (Warp portal becomes active, or Fire Pillar becomes active)
 *		xx_METEOR: flag &1 contains if the unit can cause curse, flag is also the duration of the unit in milliseconds
 * @return s_skill_unit_group
 */
std::shared_ptr<s_skill_unit_group> skill_unitsetting(struct block_list *src, uint16 skill_id, uint16 skill_lv, int16 x, int16 y, int flag)
{
	std::shared_ptr<s_skill_unit_group> group;
	int i, val1 = 0, val2 = 0, val3 = 0;
	t_tick limit;
	int link_group_id = 0;
	int target, interval, range;
	t_itemid req_item = 0;
	struct s_skill_unit_layout *layout;
	map_session_data *sd;
	status_change *sc;
	int active_flag = 1;
	int subunt = 0;
	bool hidden = false;
	struct map_data *mapdata;

	nullpo_retr(nullptr, src);

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	mapdata = map_getmapdata(src->m);
	limit = skill_get_time3(mapdata, skill_id,skill_lv);
	range = skill_get_unit_range(skill_id,skill_lv);
	interval = skill->unit_interval;
	target = skill_get_unit_target(skill_id);
	layout = skill_get_unit_layout(skill_id,skill_lv,src,x,y);

	sd = BL_CAST(BL_PC, src);
	status_data* status = status_get_status_data(*src);
	sc = status_get_sc(src);	// for traps, firewall and fogwall - celest
	hidden = (skill->unit_flag[UF_HIDDENTRAP] && (battle_config.traps_setting == 2 || (battle_config.traps_setting == 1 && map_flag_vs(src->m))));

	switch( skill_id ) {
	case MH_STEINWAND:
		val2 = 4 + skill_lv;
		val3 = 300 * skill_lv + 65 * ( status->int_ +  status_get_lv(src) ) + status->max_sp; //nb hp
		break;
	case MG_SAFETYWALL:
		val2 = skill_lv + 1;
#ifdef RENEWAL
		val3 = 300 * skill_lv + 65 * (status->int_ + status_get_lv(src)) + status->max_sp;
#endif
		break;
	case MG_FIREWALL:
		if(sc && sc->getSCE(SC_VIOLENTGALE))
			limit = limit*3/2;
		val2 = 4+skill_lv;
		break;

	case AL_WARP:
		val1=skill_lv+6;
		if(!(flag&1))
			limit=2000;
		else // previous implementation (not used anymore)
		{	//Warp Portal morphing to active mode, extract relevant data from src. [Skotlex]
			if( src->type != BL_SKILL ) return nullptr;
			group = ((TBL_SKILL*)src)->group;
			src = map_id2bl(group->src_id);
			if( !src ) return nullptr;
			val2 = group->val2; //Copy the (x,y) position you warp to
			val3 = group->val3; //as well as the mapindex to warp to.
		}
		break;
#ifndef RENEWAL
	case HP_BASILICA:
		val1 = src->id; // Store caster id.
		break;
#endif

	case PR_SANCTUARY:
	case NPC_EVILLAND:
		val1=skill_lv+3;
		break;
	case WZ_METEOR:
	case SU_CN_METEOR:
	case SU_CN_METEOR2:
	case NPC_RAINOFMETEOR:
	case HN_METEOR_STORM_BUSTER:
		limit = flag;
		flag = 0; // Flag should not influence anything else for these skills
		break;
	case WZ_FIREPILLAR:
		if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) )
			return nullptr;
		if((flag&1)!=0)
			limit=1000;
		val1=skill_lv+2;
		break;
	case WZ_QUAGMIRE:	//The target changes to "all" if used in a gvg map. [Skotlex]
	case AM_DEMONSTRATION:
		if (battle_config.vs_traps_bctall && (src->type&battle_config.vs_traps_bctall) && map_flag_vs(src->m))
			target = BCT_ALL;
		break;
	case HT_SKIDTRAP:
	case MA_SKIDTRAP:
		//Save position of caster
		val1 = ((src->x)<<16)|(src->y);
		[[fallthrough]];
	case HT_ANKLESNARE:
	case HT_SHOCKWAVE:
	case HT_SANDMAN:
	case MA_SANDMAN:
	case HT_CLAYMORETRAP:
	case HT_LANDMINE:
	case MA_LANDMINE:
	case HT_FLASHER:
	case HT_FREEZINGTRAP:
	case MA_FREEZINGTRAP:
	case HT_BLASTMINE:
	case RA_ELECTRICSHOCKER:
	case RA_CLUSTERBOMB:
	case RA_MAGENTATRAP:
	case RA_COBALTTRAP:
	case RA_MAIZETRAP:
	case RA_VERDURETRAP:
	case RA_FIRINGTRAP:
	case RA_ICEBOUNDTRAP:
	case RL_B_TRAP:
	case SC_ESCAPE:
		{
			struct s_skill_condition req = skill_get_requirement(sd,skill_id,skill_lv);
			ARR_FIND(0, MAX_SKILL_ITEM_REQUIRE, i, req.itemid[i] && (req.itemid[i] == ITEMID_TRAP || req.itemid[i] == ITEMID_TRAP_ALLOY));
			if( i != MAX_SKILL_ITEM_REQUIRE && req.itemid[i] )
				req_item = req.itemid[i];
			if (skill_id == RL_B_TRAP) // Target type should not change on GvG maps.
				break;
			if( battle_config.vs_traps_bctall && (src->type&battle_config.vs_traps_bctall) && map_flag_vs(src->m) )
				target = BCT_ALL;
		}
		break;

	case SA_LANDPROTECTOR:
	case SA_VOLCANO:
	case SA_DELUGE:
	case SA_VIOLENTGALE:
	case SC_CHAOSPANIC:
	{
		std::shared_ptr<s_skill_unit_group> old_sg = skill_locate_element_field(src);

		if (old_sg != nullptr)
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
					limit = skill_get_time(skill_id,skill_lv);
			}
			skill_clear_group(src,1);
		}
		break;
	}

	case BA_WHISTLE:
		val1 = skill_lv + status->agi / 10; // Flee increase
		val2 = (skill_lv + 1) / 2 + status->luk / 30; // Perfect dodge increase
		if (sd) {
			val1 += pc_checkskill(sd, BA_MUSICALLESSON) / 2;
			val2 += pc_checkskill(sd, BA_MUSICALLESSON) / 5;
		}
		break;
	case DC_HUMMING:
		val1 = 1 + 2 * skill_lv + status->dex / 10; // Hit increase
		if (sd)
			val1 += pc_checkskill(sd, DC_DANCINGLESSON);
		break;
	case BA_POEMBRAGI:
		val1 = 3 * skill_lv + status->dex / 10; // Casting time reduction
		//For some reason at level 10 the base delay reduction is 50%.
		val2 = (skill_lv < 10 ? 3 * skill_lv : 50) + status->int_ / 5; // After-cast delay reduction
		if (sd) {
			val1 += pc_checkskill(sd, BA_MUSICALLESSON);
			val2 += 2 * pc_checkskill(sd, BA_MUSICALLESSON);
		}
		break;
	case DC_DONTFORGETME:
#ifdef RENEWAL
		val1 = 3 * skill_lv + status->dex / 15; // ASPD decrease
		val2 = 2 * skill_lv + status->agi / 20; // Movement speed adjustment.
#else
		val1 = 5 + 3 * skill_lv + status->dex / 10; // ASPD decrease
		val2 = 5 + 3 * skill_lv + status->agi / 10; // Movement speed adjustment.
#endif		
		if (sd) {
			val1 += pc_checkskill(sd, DC_DANCINGLESSON);
#ifdef RENEWAL
			val2 += pc_checkskill(sd, DC_DANCINGLESSON) / 2;
#else
			val2 += pc_checkskill(sd, DC_DANCINGLESSON);
#endif
		}
		val1 *= 10; //Because 10 is actually 1% aspd
		break;
	case DC_SERVICEFORYOU:
		val1 = 15 + skill_lv + (status->int_ / 10); // MaxSP percent increase
		val2 = 20 + 3 * skill_lv + (status->int_ / 10); // SP cost reduction
		if (sd) {
			val1 += pc_checkskill(sd, DC_DANCINGLESSON) / 2;
			val2 += pc_checkskill(sd, DC_DANCINGLESSON) / 2;
		}
		break;
	case BA_ASSASSINCROSS:
		if (sd)
			val1 = pc_checkskill(sd, BA_MUSICALLESSON) / 2;
		val1 += 5 + skill_lv + (status->agi / 20);
		val1 *= 10; // ASPD works with 1000 as 100%
		break;
	case DC_FORTUNEKISS:
		val1 = 10 + skill_lv + (status->luk / 10); // Critical increase
		val1 *= 10; //Because every 10 crit is an actual cri point.
		if (sd)
			val1 += 5 * pc_checkskill(sd, DC_DANCINGLESSON);
		break;
	case BD_DRUMBATTLEFIELD:
		val1 = (skill_lv+1)*25;	//Atk increase
		val2 = (skill_lv+1)*2;	//Def increase
		break;
	case BD_RINGNIBELUNGEN:
		val1 = (skill_lv+2)*25;	//Atk increase
		break;
	case BD_RICHMANKIM:
		val1 = 25 + 11*skill_lv; //Exp increase bonus.
		break;
	case BD_SIEGFRIED:
		val1 = 55 + skill_lv*5;	//Elemental Resistance
		val2 = skill_lv*10;	//Status ailment resistance
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
		val2 = (skill_lv+1)/2 + 4;
		break;
	case NJ_SUITON:
		skill_clear_group(src, 1);
		break;

	case GS_GROUNDDRIFT:
		{
			// Ground Drift Element is decided when it's placed.
			int ele = skill_get_ele(skill_id, skill_lv);
			int element[5] = { ELE_WIND, ELE_DARK, ELE_POISON, ELE_WATER, ELE_FIRE };

			if (ele == ELE_RANDOM)
				val1 = element[rnd()%5]; // Use random from available unit visual?
			else if (ele == ELE_ENDOWED)
				val1 = status_get_attack_sc_element(src,sc);
			else if (ele == ELE_WEAPON) {
				val1 = status->rhw.ele;
				if (sc && sc->getSCE(SC_ENCHANTARMS))
					val1 = sc->getSCE(SC_ENCHANTARMS)->val1;
			}

			switch (val1) {
				case ELE_FIRE:
					subunt++;
					[[fallthrough]];
				case ELE_WATER:
					subunt++;
					[[fallthrough]];
				case ELE_POISON:
					subunt++;
					[[fallthrough]];
				case ELE_DARK:
					subunt++;
					[[fallthrough]];
				case ELE_WIND:
					break;
				default:
					subunt = rnd()%5;
					break;
			}

			break;
		}
	case GC_POISONSMOKE:
		if( !(sc && sc->getSCE(SC_POISONINGWEAPON)) )
			return nullptr;
		val2 = sc->getSCE(SC_POISONINGWEAPON)->val2; // Type of Poison
		val3 = sc->getSCE(SC_POISONINGWEAPON)->val1;
		limit = skill_get_time(skill_id, skill_lv);
		break;
	case NPC_COMET:
	case WL_COMET:
		if (sc) {
			sc->comet_x = x;
			sc->comet_y = y;
		}
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
	case WM_POEMOFNETHERWORLD:	// Can't be placed on top of Land Protector.
		if( skill_id == WM_POEMOFNETHERWORLD && map_flag_gvg2(src->m) )
			target = BCT_ALL;
		[[fallthrough]];
	case WM_SEVERE_RAINSTORM:
	case SO_WATER_INSIGNIA:
	case SO_FIRE_INSIGNIA:
	case SO_WIND_INSIGNIA:
	case SO_EARTH_INSIGNIA:
		if( map_getcell(src->m, x, y, CELL_CHKLANDPROTECTOR) )
			return nullptr;
		break;
	case SO_CLOUD_KILL:
	case NPC_CLOUD_KILL:
		skill_clear_group(src, 4);
		break;
	case SO_WARMER:
		skill_clear_group(src, 8);
		break;
	case SO_FIREWALK:
	case SO_ELECTRICWALK:
		limit = skill_get_time2(skill_id, skill_lv);
		break;
	case GN_WALLOFTHORN:
		// Turns to Firewall
		if( flag&1 )
			limit = 3000;
		val3 = (x<<16)|y;
		break;
	case GN_DEMONIC_FIRE:
		if (flag) { // Fire Expansion level 1
			limit = flag + 10000;
			flag = 0;
		}
		break;
	case GN_FIRE_EXPANSION_SMOKE_POWDER:
	case GN_FIRE_EXPANSION_TEAR_GAS:
		limit = ((sd ? pc_checkskill(sd,GN_DEMONIC_FIRE) : 1) + 1) * limit;
		break;
	case KO_ZENKAI:
		if (sd && sd->spiritcharm_type != CHARM_TYPE_NONE && sd->spiritcharm > 0) {
			val1 = sd->spiritcharm;
			val2 = sd->spiritcharm_type;
			limit = 6000 * val1;
			subunt = sd->spiritcharm_type - 1;
			pc_delspiritcharm(sd,sd->spiritcharm,sd->spiritcharm_type);
		}
		break;
#ifndef RENEWAL
	case HW_GRAVITATION:
		if(sc && sc->getSCE(SC_GRAVITATION) && sc->getSCE(SC_GRAVITATION)->val3 == BCT_SELF)
			link_group_id = sc->getSCE(SC_GRAVITATION)->val4;
		break;
#endif
	case SO_VACUUM_EXTREME:
		// Coordinates
		val1 = x;
		val2 = y;
		val3 = 0; // Suck target at n seconds.
		break;
	case MH_POISON_MIST:
	case MH_BLAST_FORGE:
	case MH_LAVA_SLIDE:
		skill_clear_group(src, 1);
		break;
	case MH_VOLCANIC_ASH:
		if (!map_flag_vs(src->m))
			target = BCT_ENEMY;
		break;
	case AG_VIOLENT_QUAKE:
	case AG_ALL_BLOOM:
		if (sc && sc->getSCE(SC_CLIMAX)) {
			if (skill_id == AG_ALL_BLOOM && sc->getSCE(SC_CLIMAX)->val1 == 1)
				limit /= 2; // Rose buds spawn at double the speed, so rose garden duration must be halved.
			else if (sc->getSCE(SC_CLIMAX)->val1 == 4)
				limit = 3000; // Show main AoE for fixed duration on status giving effect.
		}
		break;
	case AG_VIOLENT_QUAKE_ATK:
	case AG_ALL_BLOOM_ATK:
	case AG_ALL_BLOOM_ATK2:
		limit = flag;
		flag = 0;
		if (skill_id == AG_VIOLENT_QUAKE_ATK && sc && sc->getSCE(SC_CLIMAX) && sc->getSCE(SC_CLIMAX)->val1 == 2)
			range = 4; // Rising rocks splash is increased to 9x9.
		break;
	case WH_DEEPBLINDTRAP:
	case WH_SOLIDTRAP:
	case WH_SWIFTTRAP:
	case WH_FLAMETRAP:
		limit += 3000 * (sd ? pc_checkskill(sd, WH_ADVANCED_TRAP) : 5);
		break;
		
	case NW_GRENADES_DROPPING:
		limit = skill_get_time2(skill_id,skill_lv);
		break;
	}

	// Init skill unit group
	group = skill_initunitgroup(src, layout->count, skill_id, skill_lv, (flag & 1 ? skill->unit_id2 : skill->unit_id) + subunt, limit, interval);

	if (group == nullptr)
		return nullptr;

	group->val1 = val1;
	group->val2 = val2;
	group->val3 = val3;
	group->link_group_id = link_group_id;
	group->target_flag = target;
	group->bl_flag = skill_get_unit_bl_target(skill_id);
	group->state.ammo_consume = (sd && sd->state.arrow_atk && skill_id != GS_GROUNDDRIFT); //Store if this skill needs to consume ammo.
	group->state.song_dance = (((skill->unit_flag[UF_DANCE] || skill->unit_flag[UF_SONG])?1:0)|(skill->unit_flag[UF_ENSEMBLE]?2:0)); //Signals if this is a song/dance/duet
	group->state.guildaura = ( skill_id >= GD_LEADERSHIP && skill_id <= GD_HAWKEYES )?1:0;
	group->item_id = req_item;

	// If tick is greater than current, do not invoke onplace function just yet. [Skotlex]
	if (DIFF_TICK(group->tick, gettick()) > SKILLUNITTIMER_INTERVAL)
		active_flag = 0;

	// Put message for Talkie Box & Graffiti
	if (skill_id == HT_TALKIEBOX || skill_id == RG_GRAFFITI) {
		group->valstr=(char *) aMalloc(MESSAGE_SIZE*sizeof(char));
		if (sd)
			safestrncpy(group->valstr, sd->message, MESSAGE_SIZE);
		else //Eh... we have to write something here... even though mobs shouldn't use this. [Skotlex]
			safestrncpy(group->valstr, "Boo!", MESSAGE_SIZE);
	}

	// Dance skill
	if (group->state.song_dance) {
		if(sd) {
			sd->skill_id_dance = skill_id;
			sd->skill_lv_dance = skill_lv;
		}
		if (
			sc_start4(src, src, SC_DANCING, 100, skill_id, group->group_id, skill_lv, (group->state.song_dance&2?BCT_SELF:0), limit+1000) &&
			sd && group->state.song_dance&2 && skill_id != CG_HERMODE //Hermod is a encore with a warp!
		)
			skill_check_pc_partner(sd, skill_id, &skill_lv, 1, 1);
	}

	// Set skill unit
	limit = group->limit;
	for( i = 0; i < layout->count; i++ ) {
		struct skill_unit *unit;
		int ux = x + layout->dx[i];
		int uy = y + layout->dy[i];
		int unit_val1 = skill_lv;
		int unit_val2 = 0;
		int alive = 1;

		// are the coordinates out of range?
		if( ux <= 0 || uy <= 0 || ux >= mapdata->xs || uy >= mapdata->ys ){
			continue;
		}

		if( !group->state.song_dance && !map_getcell(src->m,ux,uy,CELL_CHKREACH) )
			continue; // don't place skill units on walls (except for songs/dances/encores)
		if( battle_config.skill_wall_check && skill->unit_flag[UF_PATHCHECK] && !path_search_long(nullptr,src->m,ux,uy,src->x,src->y,CELL_CHKWALL) )
			continue; // no path between cell and caster

		switch( skill_id ) {
			// HP for Skill unit that can be damaged, see also skill_unit_ondamaged
			case HT_LANDMINE:
			case MA_LANDMINE:
			case HT_ANKLESNARE:
			case HT_SHOCKWAVE:
			case HT_SANDMAN:
			case MA_SANDMAN:
			case HT_FLASHER:
			case HT_FREEZINGTRAP:
			case MA_FREEZINGTRAP:
			case HT_SKIDTRAP:
			case MA_SKIDTRAP:
			case HT_CLAYMORETRAP:
			case HT_BLASTMINE:
			case SC_ESCAPE:
				unit_val1 = 3500;
				break;

			case MG_FIREWALL:
			case NJ_KAENSIN:
				unit_val2 = group->val2;
				break;
			case CR_GRANDCROSS:
			case NPC_GRANDDARKNESS:
				unit_val1 = (skill_get_time(skill_id, skill_lv) / interval); //Default: 950/300 = 3 hits
				break;
			case WZ_ICEWALL:
				unit_val1 = 200 + 200*skill_lv;
				unit_val2 = map_getcell(src->m, ux, uy, CELL_GETTYPE);
				break;
			case WZ_WATERBALL:
				//Check if there are cells that can be turned into waterball units
				if (!sd || map_getcell(src->m, ux, uy, CELL_CHKWATER) 
					|| (map_find_skill_unit_oncell(src, ux, uy, SA_DELUGE, nullptr, 1)) != nullptr || (map_find_skill_unit_oncell(src, ux, uy, NJ_SUITON, nullptr, 1)) != nullptr)
					break; //Turn water, deluge or suiton into waterball cell
				continue;
			case GS_DESPERADO:
				unit_val1 = abs(layout->dx[i]);
				unit_val2 = abs(layout->dy[i]);
				if (unit_val1 < 2 || unit_val2 < 2) { //Nearby cross, linear decrease with no diagonals
					if (unit_val2 > unit_val1) unit_val1 = unit_val2;
					if (unit_val1) unit_val1--;
					unit_val1 = 36 -12*unit_val1;
				} else //Diagonal edges
					unit_val1 = 28 -4*unit_val1 -4*unit_val2;
				if (unit_val1 < 1) unit_val1 = 1;
				unit_val2 = 0;
				break;
			case NPC_REVERBERATION:
				unit_val1 = 1 + skill_lv;
				break;
			case WM_POEMOFNETHERWORLD:
				unit_val1 = 1 + skill_lv;
				break;
			case GN_WALLOFTHORN:
				if (flag&1) // Turned become Firewall
					break;
				unit_val1 = 2000 + 2000 * skill_lv; // HP
				unit_val2 = 20; // Max hits
				break;
			case RL_B_TRAP:
				unit_val1 = 3500;
				unit_val2 = 0;
				break;
			default:
				if (group->state.song_dance&0x1)
					unit_val2 = (skill->unit_flag[UF_DANCE] ? (1 << UF_DANCE) : skill->unit_flag[UF_SONG] ? (1 << UF_SONG) : 0); //Store whether this is a song/dance
				break;
		}

		if (skill->unit_flag[UF_RANGEDSINGLEUNIT] && i == (layout->count / 2))
			unit_val2 |= (1 << UF_RANGEDSINGLEUNIT); // center.

		if( sd && map_getcell(src->m, ux, uy, CELL_CHKMAELSTROM) ) //Does not recover SP from monster skills
			map_foreachincell(skill_maelstrom_suction,src->m,ux,uy,BL_SKILL,skill_id,skill_lv);

		// Check active cell to failing or remove current unit
		map_foreachincell(skill_cell_overlap,src->m,ux,uy,BL_SKILL,skill_id, &alive, src);
		if( !alive )
			continue;

		nullpo_retr(nullptr, (unit = skill_initunit(group,i,ux,uy,unit_val1,unit_val2,hidden)));
		unit->limit = limit;
		unit->range = range;

		if (skill_id == PF_FOGWALL && alive == 2)
		{	//Double duration of cells on top of Deluge/Suiton
			unit->limit *= 2;
			group->limit = unit->limit;
		}

		// Execute on all targets standing on this cell
		if (range == 0 && active_flag)
			map_foreachincell(skill_unit_effect,unit->bl.m,unit->bl.x,unit->bl.y,group->bl_flag,&unit->bl,gettick(),1);
	}

	if (!group->alive_count)
	{	//No cells? Something that was blocked completely by Land Protector?
		skill_delunitgroup(group);
		return nullptr;
	}

	//success, unit created.
	switch( skill_id ) {
		case NJ_TATAMIGAESHI: //Store number of tiles.
			group->val1 = group->alive_count;
			break;
	}

	return group;
}

/*==========================================
 *
 *------------------------------------------*/
void ext_skill_unit_onplace(struct skill_unit *unit, struct block_list *bl, t_tick tick)
{
	skill_unit_onplace(unit, bl, tick);
}

/**
 * Triggeres when 'target' (based on skill unit target) is stand at unit area
 * while skill unit initialized or moved (such by knock back).
 * As a follow of skill_unit_effect flag &1
 * @param unit
 * @param bl Target
 * @param tick
 */
static int skill_unit_onplace(struct skill_unit *unit, struct block_list *bl, t_tick tick)
{
	
	struct block_list *ss; // Actual source that cast the skill unit
	status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;
	uint16 skill_id;

	nullpo_ret(unit);
	nullpo_ret(bl);

	if(bl->prev == nullptr || !unit->alive || status_isdead(*bl))
		return 0;

	std::shared_ptr<s_skill_unit_group> sg = unit->group;

	if (sg == nullptr)
		return 0;

	nullpo_ret(ss = map_id2bl(sg->src_id));

	status_data* tstatus = status_get_status_data(*bl);

	if( (skill_get_type(sg->skill_id) == BF_MAGIC && ((battle_config.land_protector_behavior) ? map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR) : map_getcell(unit->bl.m, unit->bl.x, unit->bl.y, CELL_CHKLANDPROTECTOR)) && sg->skill_id != SA_LANDPROTECTOR) ||
		map_getcell(bl->m, bl->x, bl->y, CELL_CHKMAELSTROM) )
		return 0; //AoE skills are ineffective. [Skotlex]

	std::shared_ptr<s_skill_db> skill = skill_db.find(sg->skill_id);

	if( (skill->inf2[INF2_ISSONG] || skill->inf2[INF2_ISENSEMBLE]) && map_getcell(bl->m, bl->x, bl->y, CELL_CHKBASILICA) )
		return 0; //Songs don't work in Basilica

	sc = status_get_sc(bl);

	if (sc && sc->option&OPTION_HIDE && !skill->inf2[INF2_TARGETHIDDEN])
		return 0; //Hidden characters are immune to AoE skills except to these. [Skotlex]

	if (sc && sc->getSCE(SC_VACUUM_EXTREME) && map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR))
		status_change_end(bl, SC_VACUUM_EXTREME);

	if (sc && sc->getSCE(SC_HOVERING) && skill->inf2[INF2_IGNOREHOVERING])
		return 0; // Under Hovering characters are immune to select trap and ground target skills.

	type = skill_get_sc(sg->skill_id);
	sce = (sc && type != SC_NONE) ? sc->getSCE(type) : nullptr;
	skill_id = sg->skill_id; //In case the group is deleted, we need to return the correct skill id, still.
	switch (sg->unit_id) {
		case UNT_SPIDERWEB:
			if (sc) {
				//Duration in PVM is: 1st - 8s, 2nd - 16s, 3rd - 8s
				//Duration in PVP is: 1st - 4s, 2nd - 8s, 3rd - 12s
				t_tick sec = skill_get_time2(sg->skill_id, sg->skill_lv);
				const struct TimerData* td;
				struct map_data *mapdata = map_getmapdata(bl->m);

				if (mapdata_flag_vs(mapdata))
					sec /= 2;
				if (sc->getSCE(type)) {
					if (sc->getSCE(type)->val2 && sc->getSCE(type)->val3 && sc->getSCE(type)->val4) {
						//Already triple affected, immune
						sg->limit = DIFF_TICK(tick, sg->tick);
						break;
					}
					//Don't increase val1 here, we need a higher val in status_change_start so it overwrites the old one
					if (mapdata_flag_vs(mapdata) && sc->getSCE(type)->val1 < 3)
						sec *= (sc->getSCE(type)->val1 + 1);
					else if(!mapdata_flag_vs(mapdata) && sc->getSCE(type)->val1 < 2)
						sec *= (sc->getSCE(type)->val1 + 1);
					//Add group id to status change
					if (sc->getSCE(type)->val2 == 0)
						sc->getSCE(type)->val2 = sg->group_id;
					else if (sc->getSCE(type)->val3 == 0)
						sc->getSCE(type)->val3 = sg->group_id;
					else if (sc->getSCE(type)->val4 == 0)
						sc->getSCE(type)->val4 = sg->group_id;
					//Overwrite status change with new duration
					if ((td = get_timer(sc->getSCE(type)->timer))!=nullptr)
						status_change_start(ss, bl, type, 10000, sc->getSCE(type)->val1 + 1, sc->getSCE(type)->val2, sc->getSCE(type)->val3, sc->getSCE(type)->val4,
							i64max(DIFF_TICK(td->tick, tick), sec), SCSTART_NORATEDEF);
				}
				else {
					if (status_change_start(ss, bl, type, 10000, 1, sg->group_id, 0, 0, sec, SCSTART_NORATEDEF)) {
						td = sc->getSCE(type) ? get_timer(sc->getSCE(type)->timer) : nullptr;
						if (td)
							sec = DIFF_TICK(td->tick, tick);
						map_moveblock(bl, unit->bl.x, unit->bl.y, tick);
						clif_fixpos( *bl );
					}
					else
						sec = 3000; //Couldn't trap it?
				}
				sg->val2 = bl->id;
				sg->limit = DIFF_TICK(tick, sg->tick) + sec;
			}
			break;
		case UNT_SAFETYWALL:
			if (!sce)
				sc_start4(ss, bl,type,100,sg->skill_lv,sg->skill_id,sg->group_id,0,sg->limit);
			break;

		case UNT_BLOODYLUST:
			if (sg->src_id == bl->id)
				break; //Does not affect the caster.
			if( !sce && sc_start4(ss, bl,type,100,sg->skill_lv,0,SC__BLOODYLUST,0,sg->limit) )
				// Dirty fix to add extra time to Bloody Lust so it doesn't end before
				// Berserk, causing HP to drop to 100 when we don't want it to [Akinari]
				sc_start(ss, bl,SC__BLOODYLUST,100,sg->skill_lv,sg->limit+100);
			break;

		case UNT_PNEUMA:
			if (!sce)
				sc_start4(ss, bl,type,100,sg->skill_lv,sg->group_id,0,0,sg->limit);
			break;

		case UNT_CHAOSPANIC:
			status_change_start(ss, bl, type, 3500 + (sg->skill_lv * 1500), sg->skill_lv, 0, 0, 1, skill_get_time2(sg->skill_id, sg->skill_lv), SCSTART_NOAVOID|SCSTART_NORATEDEF|SCSTART_NOTICKDEF);
			break;

		case UNT_WARP_WAITING: {
			int working = sg->val1&0xffff;

			if(bl->type==BL_PC && !working){
				map_session_data *sd = (map_session_data *)bl;
				if((!sd->chatID || battle_config.chat_warpportal)
					&& sd->ud.to_x == unit->bl.x && sd->ud.to_y == unit->bl.y)
				{
					int x = sg->val2>>16;
					int y = sg->val2&0xffff;
					int count = sg->val1>>16;
					unsigned short m = sg->val3;

					if( --count <= 0 )
						skill_delunitgroup(sg);

					if ( map_mapindex2mapid(m) == sd->bl.m && x == sd->bl.x && y == sd->bl.y )
						working = 1;/* we break it because officials break it, lovely stuff. */

					sg->val1 = (count<<16)|working;

					if (pc_job_can_entermap((enum e_job)sd->status.class_, map_mapindex2mapid(m), pc_get_group_level(sd)))
						pc_setpos(sd,m,x,y,CLR_TELEPORT);
				}
			} else if(bl->type == BL_MOB && battle_config.mob_warp&2) {
				int16 m = map_mapindex2mapid(sg->val3);
				if (m < 0) break; //Map not available on this map-server.
				unit_warp(bl,m,sg->val2>>16,sg->val2&0xffff,CLR_TELEPORT);
			}
		}
			break;

		case UNT_QUAGMIRE:
			if( !sce && battle_check_target(&unit->bl,bl,sg->target_flag) > 0 )
				sc_start4(ss, bl,type,100,sg->skill_lv,sg->group_id,0,0,sg->limit);
			break;

		case UNT_VOLCANO:
		case UNT_DELUGE:
		case UNT_VIOLENTGALE:
		case UNT_FIRE_INSIGNIA:
		case UNT_WATER_INSIGNIA:
		case UNT_WIND_INSIGNIA:
		case UNT_EARTH_INSIGNIA:
			if(!sce)
				sc_start(ss, bl,type,100,sg->skill_lv,sg->limit);
			break;

		case UNT_WATER_BARRIER:
		case UNT_ZEPHYR:
		case UNT_POWER_OF_GAIA:
			if (bl->id == ss->id)
				break; // Doesn't affect the Elemental
			if (!sce)
				sc_start(ss, bl, type, 100, sg->skill_lv, sg->limit);
			break;

		case UNT_SUITON:
			if(!sce)
				sc_start4(ss, bl,type,100,sg->skill_lv,
				map_flag_vs(bl->m) || battle_check_target(&unit->bl,bl,BCT_ENEMY)>0?1:0, //Send val3 =1 to reduce agi.
				0,0,sg->limit);
			break;

		case UNT_HERMODE:
			if (sg->src_id!=bl->id && battle_check_target(&unit->bl,bl,BCT_PARTY|BCT_GUILD) > 0)
				status_change_clear_buffs(bl, SCCB_HERMODE); //Should dispell only allies.
			[[fallthrough]];
		case UNT_RICHMANKIM:
		case UNT_ETERNALCHAOS:
		case UNT_DRUMBATTLEFIELD:
		case UNT_RINGNIBELUNGEN:
		case UNT_ROKISWEIL:
		case UNT_INTOABYSS:
		case UNT_SIEGFRIED:
			 //Needed to check when a dancer/bard leaves their ensemble area.
			if (sg->src_id==bl->id && !(sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_BARDDANCER))
				return skill_id;
			if (!sce)
				sc_start4(ss, bl,type,100,sg->skill_lv,sg->val1,sg->val2,0,sg->limit);
			break;
		case UNT_WHISTLE:
		case UNT_ASSASSINCROSS:
		case UNT_POEMBRAGI:
		case UNT_APPLEIDUN:
		case UNT_HUMMING:
		case UNT_DONTFORGETME:
		case UNT_FORTUNEKISS:
		case UNT_SERVICEFORYOU:
			if (sg->src_id==bl->id && !(sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_BARDDANCER))
				return 0;

			if (!sc) return 0;
			if (!sce)
				sc_start4(ss, bl,type,100,sg->skill_lv,sg->val1,sg->val2,0,sg->limit);
			else if (sce->val4 == 1) { //Readjust timers since the effect will not last long.
				sce->val4 = 0; //remove the mark that we stepped out
				delete_timer(sce->timer, status_change_timer);
				sce->timer = add_timer(tick+sg->limit, status_change_timer, bl->id, type); //put duration back to 3min
			}
			break;

		case UNT_FOGWALL:
			if (!sce)
			{
				sc_start4(ss, bl, type, 100, sg->skill_lv, sg->val1, sg->val2, sg->group_id, sg->limit);
				if (battle_check_target(&unit->bl,bl,BCT_ENEMY)>0)
					skill_additional_effect (ss, bl, sg->skill_id, sg->skill_lv, BF_MISC, ATK_DEF, tick);
			}
			break;

#ifndef RENEWAL
		case UNT_GRAVITATION:
			if (!sce)
				sc_start4(ss, bl,type,100,sg->skill_lv,0,BCT_ENEMY,sg->group_id,sg->limit);
			break;

		case UNT_BASILICA:
			{
				int i = battle_check_target(bl, bl, BCT_ENEMY);

				if (i > 0) {
					skill_blown(ss, bl, skill_get_blewcount(skill_id, sg->skill_lv), unit_getdir(bl), BLOWN_NONE);
					break;
				}
				if (!sce && i <= 0)
					sc_start4(ss, bl, type, 100, 0, 0, sg->group_id, ss->id, sg->limit);
			}
			break;
#endif

		case UNT_MOONLIT:
			//Knockback out of area if affected char isn't in Moonlit effect
			if (sc && sc->getSCE(SC_DANCING) && (sc->getSCE(SC_DANCING)->val1&0xFFFF) == CG_MOONLIT)
				break;
			if (ss == bl) //Also needed to prevent infinite loop crash.
				break;
			skill_blown(ss,bl,skill_get_blewcount(sg->skill_id,sg->skill_lv),unit_getdir(bl),BLOWN_NONE);
			break;

		case UNT_REVERBERATION:
			if (sg->src_id == bl->id)
				break; //Does not affect the caster.
			clif_changetraplook(&unit->bl,UNT_USED_TRAPS);
			map_foreachinrange(skill_trap_splash,&unit->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &unit->bl,tick);
			sg->limit = DIFF_TICK(tick,sg->tick) + 1500;
			sg->unit_id = UNT_USED_TRAPS;
			break;

		case UNT_FIRE_EXPANSION_SMOKE_POWDER:
			if (!sce && battle_check_target(&unit->bl, bl, sg->target_flag) > 0)
				sc_start(ss, bl, type, 100, sg->skill_lv, sg->limit);
			break;

		case UNT_FIRE_EXPANSION_TEAR_GAS:
			if (!sce && battle_check_target(&unit->bl, bl, sg->target_flag) > 0 && map_flag_vs(bl->m))
				if( sc_start4(ss, bl, type, 100, sg->skill_lv, 0, ss->id,0, sg->limit) )
					sc_start(ss, bl, SC_TEARGAS_SOB, 100, sg->skill_lv, sg->limit);
			break;

		case UNT_VOLCANIC_ASH:
			if (!sce)
				sc_start(ss, bl, SC_ASH, 100, sg->skill_lv, skill_get_time(MH_VOLCANIC_ASH, sg->skill_lv));
			break;

		case UNT_KINGS_GRACE:
			if (!sce) {
				int state = 0;

				if (!map_flag_vs(ss->m) && !map_flag_gvg2(ss->m))
					state |= BCT_GUILD;
				if (battle_check_target(&unit->bl, bl, BCT_SELF|BCT_PARTY|state) > 0)
					sc_start4(ss, bl, type, 100, sg->skill_lv, 0, ss->id, 0, sg->limit);
			}
			break;

		case UNT_STEALTHFIELD:
			if( bl->id == sg->src_id )
				break; // Doesn't work on self (video shows that)
			if (!sce)
				sc_start(ss, bl,type,100,sg->skill_lv,sg->limit);
			break;

		case UNT_NEUTRALBARRIER:
			if (!sce)
				status_change_start(ss, bl, type, 10000, sg->skill_lv, 0, 0, 0, sg->limit, SCSTART_NOICON);
			break;

		case UNT_WARMER:
			if (!sce && bl->type == BL_PC && !battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race != RC_DEMON)
				sc_start2(ss, bl, type, 100, sg->skill_lv, ss->id, skill_get_time(sg->skill_id, sg->skill_lv));
			break;

		case UNT_CATNIPPOWDER:
			if (sg->src_id == bl->id)
				break; // Does not affect the caster or Boss.
			if (!sce && battle_check_target(&unit->bl, bl, BCT_ENEMY) > 0)
				sc_start(ss, bl, type, 100, sg->skill_lv, skill_get_time(sg->skill_id, sg->skill_lv));
			break;

		case UNT_NYANGGRASS:
			if (!sce)
				sc_start(ss, bl, type, 100, sg->skill_lv, skill_get_time(sg->skill_id, sg->skill_lv));
			break;

		case UNT_CREATINGSTAR:
			if (!sce)
				sc_start4(ss, bl, type, 100, sg->skill_lv, ss->id, unit->bl.id, 0, sg->limit);
			break;

		case UNT_GD_LEADERSHIP:
		case UNT_GD_GLORYWOUNDS:
		case UNT_GD_SOULCOLD:
		case UNT_GD_HAWKEYES:
			if ( !sce && battle_check_target(&unit->bl, bl, sg->target_flag) > 0 )
				sc_start4(ss, bl,type,100,sg->skill_lv,0,0,0,1000);
			break;
	}
	return skill_id;
}

/**
 * Process skill unit each interval (sg->interval, see interval field of skill_db.yml)
 * @param unit Skill unit
 * @param bl Valid 'target' above the unit, that has been check in skill_unit_timer_sub_onplace
 * @param tick
 */
int skill_unit_onplace_timer(struct skill_unit *unit, struct block_list *bl, t_tick tick)
{
	struct block_list *ss;
	TBL_PC* tsd;
	status_change *sc, *tsc;
	struct skill_unit_group_tickset *ts;
	enum sc_type type;
	uint16 skill_id;
	t_tick diff = 0;

	nullpo_ret(unit);
	nullpo_ret(bl);

	if (bl->prev == nullptr || !unit->alive || status_isdead(*bl))
		return 0;

	std::shared_ptr<s_skill_unit_group> sg = unit->group;

	if (sg == nullptr)
		return 0;

	nullpo_ret(ss = map_id2bl(sg->src_id));

	tsd = BL_CAST(BL_PC, bl);
	tsc = status_get_sc(bl);
	sc = status_get_sc(ss);
	status_data* tstatus = status_get_status_data(*bl);
	type = skill_get_sc(sg->skill_id);
	skill_id = sg->skill_id;

	std::bitset<INF2_MAX> inf2 = skill_db.find(skill_id)->inf2;

	if (sc && sc->getSCE(SC_VOICEOFSIREN) && sc->getSCE(SC_VOICEOFSIREN)->val2 == bl->id && inf2[INF2_ISTRAP])
		return 0; // Traps cannot be activated by the Maestro or Wanderer that enticed the trapper with this skill.

	if (tsc && tsc->getSCE(SC_HOVERING) && inf2[INF2_IGNOREHOVERING])
		return 0; // Under Hovering characters are immune to trap and ground target skills.

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
	}

	// Wall of Thorn damaged by Fire element unit [Cydh]
	//! TODO: This check doesn't matter the location, as long as one of units touched, this check will be executed.
	if (bl->type == BL_SKILL && skill_get_ele(sg->skill_id, sg->skill_lv) == ELE_FIRE) {
		struct skill_unit *su = (struct skill_unit *)bl;
		if (su && su->group && su->group->unit_id == UNT_WALLOFTHORN) {
			skill_unitsetting(map_id2bl(su->group->src_id), su->group->skill_id, su->group->skill_lv, su->group->val3>>16, su->group->val3&0xffff, 1);
			su->group->limit = sg->limit = 0;
			su->group->unit_id = sg->unit_id = UNT_USED_TRAPS;
			return skill_id;
		}
	}

	switch (sg->unit_id) {
		// Units that deals simple attack
		case UNT_GRAVITATION:
		case UNT_EARTHSTRAIN:
		case UNT_FIREWALK:
		case UNT_ELECTRICWALK:
		case UNT_PSYCHIC_WAVE:
		case UNT_LAVA_SLIDE:
		case UNT_MAKIBISHI:
		case UNT_VENOMFOG:
		case UNT_ICEMINE:
		case UNT_FLAMECROSS:
		case UNT_HELLBURNING:
		case UNT_CANE_OF_EVIL_EYE:
		case UNT_RAIN_OF_CRYSTAL:
		case UNT_MYSTERY_ILLUSION:
		case UNT_STRANTUM_TREMOR:
		case UNT_TORNADO_STORM:
		case UNT_FLORAL_FLARE_ROAD:
		case UNT_CROSS_RAIN:
		case UNT_PNEUMATICUS_PROCELLA:
		case UNT_LIGHTNING_LAND:
		case UNT_VENOM_SWAMP:
		case UNT_CONFLAGRATION:
		case UNT_DEEPBLINDTRAP:
		case UNT_SOLIDTRAP:
		case UNT_SWIFTTRAP:
		case UNT_FLAMETRAP:
		case UNT_GROUND_GRAVITATION:
		case UNT_JACK_FROST_NOVA:
			skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_DUMMYSKILL:
			switch (sg->skill_id) {
				case SG_SUN_WARM: //SG skills [Komurka]
				case SG_MOON_WARM:
				case SG_STAR_WARM: {
					int count = 0;
					const int x = bl->x, y = bl->y;

					//If target isn't knocked back it should hit every "interval" ms [Playtester]
					do {
						if( bl->type == BL_PC )
							status_zap(bl, 0, 15); // sp damage to players
						else // mobs
						if( status_charge(ss, 0, 2) ) { // costs 2 SP per hit
							if( !skill_attack(BF_WEAPON,ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick+(t_tick)count*sg->interval,0) )
								status_charge(ss, 0, 8); //costs additional 8 SP if miss
						} else { //should end when out of sp.
							sg->limit = DIFF_TICK(tick,sg->tick);
							break;
						}
					} while(sg->interval > 0 && x == bl->x && y == bl->y &&
						++count < SKILLUNITTIMER_INTERVAL/sg->interval && !status_isdead(*bl) );
				}
					break;
#ifndef RENEWAL // The storm gust counter was dropped in renewal
				case WZ_STORMGUST: //SG counter does not reset per stormgust. IE: One hit from a SG and two hits from another will freeze you.
					if (tsc)
						tsc->sg_counter++; //SG hit counter.
					if (skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0) <= 0 && tsc)
						tsc->sg_counter=0; //Attack absorbed.
					break;
#endif
				case GS_DESPERADO:
					if (rnd()%100 < unit->val1)
						skill_attack(BF_WEAPON,ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
					break;
				case NPC_COMET:
				case WL_COMET:
					if (map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR) == 0)// Nothing should happen if the target is on Land Protector
						skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
					break;
				case NPC_WIDESUCK: {
						int heal = (int)skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);

						if (heal > 0) {
							clif_skill_nodamage(ss,*bl,sg->skill_id,sg->skill_lv);
							clif_skill_nodamage(nullptr,*ss,AL_HEAL,heal);
							status_heal(ss,heal,0,0);
						}
					}
					break;
				case CR_GRANDCROSS:
				case NPC_GRANDDARKNESS:
					if(!battle_config.gx_allhit)
						unit->val1--;
					skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
					break;
				default:
					skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			}
			break;

		case UNT_FIREWALL:
		case UNT_KAEN: {
			int count = 0;
			const int x = bl->x, y = bl->y;

			if (skill_id == GN_WALLOFTHORN && battle_check_target(ss, bl, sg->target_flag) <= 0)
				break;

			//Take into account these hit more times than the timer interval can handle.
			do
				skill_attack(BF_MAGIC,ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick+(t_tick)count*sg->interval,0);
			while(sg->interval > 0 && --unit->val2 && x == bl->x && y == bl->y &&
				++count < SKILLUNITTIMER_INTERVAL/sg->interval && !status_isdead(*bl));

			if (unit->val2 <= 0)
				skill_delunit(unit);
		}
		break;
		case UNT_SANCTUARY:
			if( battle_check_undead(tstatus->race, tstatus->def_ele) || tstatus->race==RC_DEMON )
			{ //Only damage enemies with offensive Sanctuary. [Skotlex]
				if( battle_check_target(&unit->bl,bl,BCT_ENEMY) > 0 && skill_attack(BF_MAGIC, ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, 0) )
					sg->val1 -= 1; // Reduce the number of targets that can still be hit
			} else {
				int heal = skill_calc_heal(ss,bl,sg->skill_id,sg->skill_lv,true);
				struct mob_data *md = BL_CAST(BL_MOB, bl);

#ifdef RENEWAL
				if (md && md->mob_id == MOBID_EMPERIUM)
					break;
#endif
				if (md && status_get_class_(bl) == CLASS_BATTLEFIELD)
					break;
				if( tstatus->hp >= tstatus->max_hp )
					break;
				if( status_isimmune(bl) )
					heal = 0;
				clif_skill_nodamage(&unit->bl, *bl, AL_HEAL, heal);
				if( tsc && tsc->getSCE(SC_AKAITSUKI) && heal )
					heal = ~heal + 1;
				status_heal(bl, heal, 0, 0);
			}
			break;

		case UNT_EVILLAND:
			//Will heal demon and undead element monsters, but not players.
			if ((bl->type == BL_PC) || (!battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race!=RC_DEMON))
			{	//Damage enemies
				if(battle_check_target(&unit->bl,bl,BCT_ENEMY)>0)
					skill_attack(BF_MISC, ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			} else {
				int heal = skill_calc_heal(ss,bl,sg->skill_id,sg->skill_lv,true);

				if (tstatus->hp >= tstatus->max_hp)
					break;
				if (status_isimmune(bl))
					heal = 0;
				clif_skill_nodamage(&unit->bl, *bl, AL_HEAL, heal);
				status_heal(bl, heal, 0, 0);
			}
			break;

		case UNT_MAGNUS:
#ifndef RENEWAL
			if (!battle_check_undead(tstatus->race,tstatus->def_ele) && tstatus->race!=RC_DEMON)
				break;
#endif
			skill_attack(BF_MAGIC,ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_FIREPILLAR_WAITING:
			skill_unitsetting(ss,sg->skill_id,sg->skill_lv,unit->bl.x,unit->bl.y,1);
			skill_delunit(unit);
			break;

		case UNT_SKIDTRAP: {
				//Knockback away from position of user during placement [Playtester]
				skill_blown(&unit->bl,bl,skill_get_blewcount(sg->skill_id,sg->skill_lv),
					(map_calc_dir_xy(sg->val1>>16,sg->val1&0xFFFF,bl->x,bl->y,6)+4)%8,BLOWN_NONE);
				sg->unit_id = UNT_USED_TRAPS;
				clif_changetraplook(&unit->bl, UNT_USED_TRAPS);
				sg->limit=DIFF_TICK(tick,sg->tick)+1500;
#ifdef RENEWAL
				// In renewal, target will be stopped for 3 seconds
				sc_start(ss,bl,SC_STOP,100,0,skill_get_time2(sg->skill_id,sg->skill_lv));
#else
				// In pre-renewal, if target was a monster, it will unlock target and become idle
				struct mob_data* md = BL_CAST(BL_MOB, bl);
				if (md)
					mob_unlocktarget(md, tick);
#endif
			}
			break;

		case UNT_ANKLESNARE:
		case UNT_MANHOLE:
			if( sg->val2 == 0 && tsc && ((sg->unit_id == UNT_ANKLESNARE && skill_id != SC_ESCAPE) || bl->id != sg->src_id) ) {
				t_tick sec = skill_get_time2(sg->skill_id,sg->skill_lv);
				if (sg->unit_id == UNT_ANKLESNARE) {
					t_tick mintime = 30 * (status_get_lv(ss) + 100);
#ifndef RENEWAL
					// Bosses cannot activate Ankle Snare in renewal so we don't need this code
					if (status_bl_has_mode(bl, MD_STATUSIMMUNE))
						sec /= 5;
#endif
					sec = std::max((sec * status_get_agi(bl)) / -200 + sec, mintime);
				}

				if( status_change_start(ss, bl,type,10000,sg->skill_lv,sg->group_id,0,0,sec, SCSTART_NORATEDEF) ) {
					const struct TimerData* td = tsc->getSCE(type)?get_timer(tsc->getSCE(type)->timer):nullptr;

					if( td )
						sec = DIFF_TICK(td->tick, tick);
					if( (sg->unit_id == UNT_MANHOLE && bl->type == BL_PC)
						|| !unit_blown_immune(bl,0x1) )
					{
						unit_movepos(bl, unit->bl.x, unit->bl.y, 0, 0);
						clif_fixpos( *bl );
					}
					sg->val2 = bl->id;
				} else
					sec = 3000; //Couldn't trap it?
				if (sg->unit_id == UNT_ANKLESNARE) {
					clif_skillunit_update( unit->bl );
					/**
					 * If you're snared from a trap that was invisible this makes the trap be
					 * visible again -- being you stepped on it (w/o this the trap remains invisible and you go "WTF WHY I CANT MOVE")
					 * bugreport:3961
					 **/
					clif_changetraplook(&unit->bl, UNT_ANKLESNARE);
				}
				sg->limit = DIFF_TICK(tick,sg->tick)+sec;
				sg->interval = -1;
				unit->range = 0;
			}
			break;

		case UNT_EARTHQUAKE:
			sg->val1++; // Hit count
			skill_attack(skill_get_type(sg->skill_id), ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, map_foreachinallrange(skill_area_sub, &unit->bl, skill_get_splash(sg->skill_id, sg->skill_lv), BL_CHAR, &unit->bl, sg->skill_id, sg->skill_lv, tick, BCT_ENEMY, skill_area_sub_count) | (sg->val1 == 1 ? NPC_EARTHQUAKE_FLAG : 0));
			break;

		case UNT_ELECTRICSHOCKER:
			if( bl->id != ss->id ) {
				if( status_change_start(ss, bl,type,10000,sg->skill_lv,sg->group_id,0,0,skill_get_time2(sg->skill_id, sg->skill_lv), SCSTART_NORATEDEF) ) {
					map_moveblock(bl, unit->bl.x, unit->bl.y, tick);
					clif_fixpos( *bl );
				}
				map_foreachinallrange(skill_trap_splash, &unit->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &unit->bl, tick);
				sg->unit_id = UNT_USED_TRAPS; //Changed ID so it does not invoke a for each in area again.
			}
			break;

		case UNT_VENOMDUST:
			if(tsc && !tsc->getSCE(type))
				status_change_start(ss,bl,type,10000,sg->skill_lv,sg->src_id,0,0,skill_get_time2(sg->skill_id,sg->skill_lv),SCSTART_NONE);
			break;

		case UNT_LANDMINE:
			//Land Mine only hits single target
			skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			sg->unit_id = UNT_USED_TRAPS; //Changed ID so it does not invoke a for each in area again.
			sg->limit = 1500;
			break;
		case UNT_MAGENTATRAP:
		case UNT_COBALTTRAP:
		case UNT_MAIZETRAP:
		case UNT_VERDURETRAP:
			if( bl->type == BL_PC )// it won't work on players
				break;
			[[fallthrough]];
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
		case UNT_CLUSTERBOMB:
			if( bl->id == ss->id )// it won't trigger on caster
				break;
			[[fallthrough]];
		case UNT_BLASTMINE:
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
		case UNT_FREEZINGTRAP:
		case UNT_FIREPILLAR_ACTIVE:
		case UNT_CLAYMORETRAP:
		{
			int bl_flag = sg->bl_flag;
			if (tsc && tsc->getSCE(SC__MANHOLE))
				break;
			if (sg->unit_id == UNT_FIRINGTRAP || sg->unit_id == UNT_ICEBOUNDTRAP || sg->unit_id == UNT_CLAYMORETRAP)
				bl_flag = bl_flag|BL_SKILL|~BCT_SELF;
			map_foreachinrange(skill_trap_splash, &unit->bl, skill_get_splash(sg->skill_id, sg->skill_lv), bl_flag, &unit->bl, tick);
			if (sg->unit_id != UNT_FIREPILLAR_ACTIVE)
				clif_changetraplook(&unit->bl,(sg->unit_id == UNT_LANDMINE ? UNT_FIREPILLAR_ACTIVE : UNT_USED_TRAPS));
			sg->limit = DIFF_TICK(tick, sg->tick) +
				(sg->unit_id == UNT_CLUSTERBOMB || sg->unit_id == UNT_ICEBOUNDTRAP ? 1000 : 0) + // Cluster Bomb/Icebound has 1s to disappear once activated.
				(sg->unit_id == UNT_FIRINGTRAP ? 0 : 1500); // Firing Trap gets removed immediately once activated.
			sg->unit_id = UNT_USED_TRAPS; // Change ID so it does not invoke a for each in area again.
		}
			break;

		case UNT_TALKIEBOX:
			if (sg->src_id == bl->id)
				break;
			if (sg->val2 == 0) {
				clif_talkiebox(&unit->bl, sg->valstr);
				sg->unit_id = UNT_USED_TRAPS;
				clif_changetraplook(&unit->bl, UNT_USED_TRAPS);
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
			skill_attack(BF_MISC, ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			break;

		case UNT_APPLEIDUN: { //Apple of Idun [Skotlex]
				int heal;
#ifdef RENEWAL
				struct mob_data *md = BL_CAST(BL_MOB, bl);

				if (md && md->mob_id == MOBID_EMPERIUM)
					break;
#endif
				if (sg->src_id == bl->id && !(tsc && tsc->getSCE(SC_SPIRIT) && tsc->getSCE(SC_SPIRIT)->val2 == SL_BARDDANCER))
					break; // affects self only when soullinked
				heal = skill_calc_heal(ss,bl,sg->skill_id, sg->skill_lv, true);
				if (tsc->getSCE(SC_AKAITSUKI) && heal)
					heal = ~heal + 1;
				clif_skill_nodamage(&unit->bl, *bl, AL_HEAL, heal);
				status_heal(bl, heal, 0, 0);
			}
			break;

		case UNT_TATAMIGAESHI:
		case UNT_DEMONSTRATION:
			skill_attack(BF_WEAPON,ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_GOSPEL:
			if (rnd() % 100 >= 50 + sg->skill_lv * 5 || ss == bl)
				break;
			if (battle_check_target(ss, bl, BCT_PARTY) > 0)
			{ // Support Effect only on party, not guild
				int heal;
				int i = rnd() % 13; // Positive buff count
				int time = skill_get_time2(sg->skill_id, sg->skill_lv); //Duration
				switch (i)
				{
					case 0: // Heal 1000~9999 HP
						heal = rnd() % 9000 + 1000;
						clif_skill_nodamage(ss, *bl, AL_HEAL, heal);
						status_heal(bl, heal, 0, 0);
						break;
					case 1: // End all negative status
						status_change_clear_buffs(bl, SCCB_DEBUFFS | SCCB_REFRESH);
						if (tsd) clif_gospel_info(tsd, 0x15);
						break;
					case 2: // Immunity to all status
						sc_start(ss, bl, SC_SCRESIST, 100, 100, time);
						if (tsd) clif_gospel_info(tsd, 0x16);
						break;
					case 3: // MaxHP +100%
						sc_start(ss, bl, SC_INCMHPRATE, 100, 100, time);
						if (tsd) clif_gospel_info(tsd, 0x17);
						break;
					case 4: // MaxSP +100%
						sc_start(ss, bl, SC_INCMSPRATE, 100, 100, time);
						if (tsd) clif_gospel_info(tsd, 0x18);
						break;
					case 5: // All stats +20
						sc_start(ss, bl, SC_INCALLSTATUS, 100, 20, time);
						if (tsd) clif_gospel_info(tsd, 0x19);
						break;
					case 6: // Level 10 Blessing
						sc_start(ss, bl, SC_BLESSING, 100, 10, skill_get_time(AL_BLESSING, 10));
						break;
					case 7: // Level 10 Increase AGI
						sc_start(ss, bl, SC_INCREASEAGI, 100, 10, skill_get_time(AL_INCAGI, 10));
						break;
					case 8: // Enchant weapon with Holy element
						sc_start(ss, bl, SC_ASPERSIO, 100, 1, time);
						if (tsd) clif_gospel_info(tsd, 0x1c);
						break;
					case 9: // Enchant armor with Holy element
						sc_start(ss, bl, SC_BENEDICTIO, 100, 1, time);
						if (tsd) clif_gospel_info(tsd, 0x1d);
						break;
					case 10: // DEF +25%
						sc_start(ss, bl, SC_INCDEFRATE, 100, 25, 10000); //10 seconds
						if (tsd) clif_gospel_info(tsd, 0x1e);
						break;
					case 11: // ATK +100%
						sc_start(ss, bl, SC_INCATKRATE, 100, 100, time);
						if (tsd) clif_gospel_info(tsd, 0x1f);
						break;
					case 12: // HIT/Flee +50
						sc_start(ss, bl, SC_INCHIT, 100, 50, time);
						sc_start(ss, bl, SC_INCFLEE, 100, 50, time);
						if (tsd) clif_gospel_info(tsd, 0x20);
						break;
				}
			}
			else if (battle_check_target(&unit->bl, bl, BCT_ENEMY) > 0)
			{ // Offensive Effect
				int i = rnd() % 10; // Negative buff count
				switch (i)
				{
					case 0: // Deal 3000~7999 damage reduced by DEF
					case 1: // Deal 1500~5499 damage unreducable
						skill_attack(BF_MISC, ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, i);
						break;
					case 2: // Curse
						sc_start(ss, bl, SC_CURSE, 100, 1, 1800000); //30 minutes
						break;
					case 3: // Blind
						sc_start(ss, bl, SC_BLIND, 100, 1, 1800000); //30 minutes
						break;
					case 4: // Poison
						sc_start2(ss, bl, SC_POISON, 100, 1, ss->id, 1800000); //30 minutes
						break;
					case 5: // Level 10 Provoke
						clif_skill_nodamage(nullptr, *bl, SM_PROVOKE, 10, sc_start(ss, bl, SC_PROVOKE, 100, 10, INFINITE_TICK)); //Infinite
						break;
					case 6: // DEF -100%
						sc_start(ss, bl, SC_INCDEFRATE, 100, -100, 20000); //20 seconds
						break;
					case 7: // ATK -100%
						sc_start(ss, bl, SC_INCATKRATE, 100, -100, 20000); //20 seconds
						break;
					case 8: // Flee -100%
						sc_start(ss, bl, SC_INCFLEERATE, 100, -100, 20000); //20 seconds
						break;
					case 9: // Speed/ASPD -25%
						sc_start4(ss, bl, SC_GOSPEL, 100, 1, 0, 0, BCT_ENEMY, 20000); //20 seconds
						break;
				}
			}
			break;

#ifndef RENEWAL
		case UNT_BASILICA:
			{
				int i = battle_check_target(&unit->bl, bl, BCT_ENEMY);

				if (i > 0) {
					skill_blown(&unit->bl, bl, skill_get_blewcount(skill_id, sg->skill_lv), unit_getdir(bl), BLOWN_NONE);
					break;
				}
				if (i <= 0 && (!tsc || !tsc->getSCE(SC_BASILICA)))
					sc_start4(ss, bl, type, 100, 0, 0, sg->group_id, ss->id, sg->limit);
			}
			break;
#endif

		case UNT_GROUNDDRIFT_WIND:
		case UNT_GROUNDDRIFT_DARK:
		case UNT_GROUNDDRIFT_POISON:
		case UNT_GROUNDDRIFT_WATER:
		case UNT_GROUNDDRIFT_FIRE:
			map_foreachinrange(skill_trap_splash,&unit->bl,
				skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag,
				&unit->bl,tick);
			sg->unit_id = UNT_USED_TRAPS;
			//clif_changetraplook(&src->bl, UNT_FIREPILLAR_ACTIVE);
			sg->limit=DIFF_TICK(tick,sg->tick);
			break;

		case UNT_POISONSMOKE:
			if( battle_check_target(ss,bl,BCT_ENEMY) > 0 && !(tsc && tsc->getSCE(sg->val2)) && rnd()%100 < 50 )
				sc_start4(ss,bl,(sc_type)sg->val2,100,sg->val3,0,1,0,skill_get_time2(GC_POISONINGWEAPON, 1));
			break;

		case UNT_EPICLESIS:
			++sg->val1; // Increment outside of the check to get the exact interval of the skill unit
			if( bl->type == BL_PC && !battle_check_undead(tstatus->race, tstatus->def_ele) && tstatus->race != RC_DEMON ) {
				if (sg->val1 % 3 == 0) { // Recover players every 3 seconds
					int hp, sp;

					switch( sg->skill_lv ) {
						case 1: case 2: hp = 3; sp = 2; break;
						case 3: case 4: hp = 4; sp = 3; break;
						case 5: default: hp = 5; sp = 4; break;
					}
					hp = tstatus->max_hp * hp / 100;
					sp = tstatus->max_sp * sp / 100;
					if (tstatus->hp < tstatus->max_hp)
						clif_skill_nodamage(&unit->bl, *bl, AL_HEAL, hp);
					if (tstatus->sp < tstatus->max_sp)
						clif_skill_nodamage(&unit->bl, *bl, MG_SRECOVERY, sp);
					if (tsc && tsc->getSCE(SC_AKAITSUKI) && hp)
						hp = ~hp + 1;
					status_heal(bl, hp, sp, 3);
				}
				if (sg->val1 % 5 == 0) { // Reveal hidden players every 5 seconds
					// Doesn't remove Invisibility or Chase Walk.
					status_change_end(bl,SC_HIDING);
					status_change_end(bl,SC_CLOAKING);
					status_change_end(bl,SC_CLOAKINGEXCEED);
					status_change_end(bl,SC_CAMOUFLAGE);
					status_change_end(bl,SC_NEWMOON);
					if (tsc && tsc->getSCE(SC__SHADOWFORM) && rnd() % 100 < 100 - tsc->getSCE(SC__SHADOWFORM)->val1 * 10) // [100 - (Skill Level x 10)] %
						status_change_end(bl, SC__SHADOWFORM);
				}
				sc_start(ss, bl, type, 100, sg->skill_lv, sg->interval + 100);
			}
			/* Enable this if kRO fix the current skill. Currently no damage on undead and demon monster. [Jobbie]
			else if( battle_check_target(ss, bl, BCT_ENEMY) > 0 && battle_check_undead(tstatus->race, tstatus->def_ele) )
				skill_castend_damage_id(&src->bl, bl, sg->skill_id, sg->skill_lv, 0, 0);*/
			break;

		case UNT_DIMENSIONDOOR:
			if( tsd && !map_getmapflag(bl->m, MF_NOTELEPORT) )
				pc_randomwarp(tsd,CLR_TELEPORT);
			else if( bl->type == BL_MOB && battle_config.mob_warp&8 )
				unit_warp(bl,-1,-1,-1,CLR_TELEPORT);
			break;

		case UNT_REVERBERATION:
			clif_changetraplook(&unit->bl,UNT_USED_TRAPS);
			map_foreachinrange(skill_trap_splash, &unit->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &unit->bl, tick);
			sg->limit = DIFF_TICK(tick,sg->tick) + 1000;
			sg->unit_id = UNT_USED_TRAPS;
			break;

		case UNT_SEVERE_RAINSTORM:
			if( battle_check_target(&unit->bl, bl, BCT_ENEMY) > 0 )
				skill_attack(BF_WEAPON,ss,&unit->bl,bl,WM_SEVERE_RAINSTORM_MELEE,sg->skill_lv,tick,0);
			break;
		case UNT_NETHERWORLD:
			if (!status_bl_has_mode(bl,MD_STATUSIMMUNE) || (!map_flag_gvg2(ss->m) && battle_check_target(&unit->bl,bl,BCT_PARTY) < 0)) {
				if (!(tsc && tsc->getSCE(type))) {
					sc_start(ss, bl, type, 100, sg->skill_lv, skill_get_time2(sg->skill_id,sg->skill_lv));
					sg->limit = DIFF_TICK(tick,sg->tick);
					sg->unit_id = UNT_USED_TRAPS;
				}
			}
			break;
		case UNT_THORNS_TRAP:
			if( tsc ) {
				if( !sg->val2 ) {
					t_tick sec = skill_get_time2(sg->skill_id, sg->skill_lv);
					if( sc_start(ss, bl, type, 100, sg->skill_lv, sec) ) {
						const struct TimerData* td = tsc->getSCE(type)?get_timer(tsc->getSCE(type)->timer):nullptr;
						if( td )
							sec = DIFF_TICK(td->tick, tick);
						///map_moveblock(bl, src->bl.x, src->bl.y, tick); // in official server it doesn't behave like this. [malufett]
						clif_fixpos( *bl );
						sg->val2 = bl->id;
					} else
						sec = 3000;	// Couldn't trap it?
					sg->limit = DIFF_TICK(tick, sg->tick) + sec;
				} else if( tsc->getSCE(SC_THORNSTRAP) && bl->id == sg->val2 )
					skill_attack(skill_get_type(GN_THORNS_TRAP), ss, ss, bl, sg->skill_id, sg->skill_lv, tick, SD_LEVEL|SD_ANIMATION);
			}
			break;

		case UNT_WALLOFTHORN:
			if (unit->val2-- <= 0) // Max hit reached
				break;
			if (status_bl_has_mode(bl,MD_STATUSIMMUNE))
				break; // This skill doesn't affect to Boss monsters. [iRO Wiki]
			skill_blown(&unit->bl, bl, skill_get_blewcount(sg->skill_id, sg->skill_lv), unit_getdir(bl), BLOWN_IGNORE_NO_KNOCKBACK);
			skill_addtimerskill(ss, tick + 100, bl->id, unit->bl.x, unit->bl.y, sg->skill_id, sg->skill_lv, skill_get_type(sg->skill_id), 4|SD_LEVEL);
			break;

		case UNT_DEMONIC_FIRE:
			switch( sg->val2 ) {
				case 1:
				default:
					sc_start4(ss, bl, SC_BURNING, 4 + 4 * sg->skill_lv, sg->skill_lv, 1000, ss->id, 0, skill_get_time2(sg->skill_id, sg->skill_lv));
					skill_attack(skill_get_type(skill_id), ss, &unit->bl, bl, sg->skill_id, sg->skill_lv + 10 * sg->val2, tick, 0);
					break;
			}
			break;

		case UNT_ZEPHYR:
			if (ss == bl)
				break; // Doesn't affect the Elemental
			sc_start(ss, bl, type, 100, sg->skill_lv, sg->interval);
			break;

		case UNT_CLOUD_KILL:
			if (tsc && !tsc->getSCE(type))
				status_change_start(ss, bl, type, 10000, sg->skill_lv, ss->id, 0, 0, skill_get_time(sg->skill_id, sg->skill_lv), SCSTART_NOTICKDEF);
			skill_attack(skill_get_type(sg->skill_id), ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			break;

		case UNT_VACUUM_EXTREME:
			if (tsc && (tsc->getSCE(SC_HALLUCINATIONWALK) || tsc->getSCE(SC_NPC_HALLUCINATIONWALK) || tsc->getSCE(SC_HOVERING) || tsc->getSCE(SC_VACUUM_EXTREME) ||
				(tsc->getSCE(SC_VACUUM_EXTREME_POSTDELAY) && tsc->getSCE(SC_VACUUM_EXTREME_POSTDELAY)->val2 == sg->group_id))) // Ignore post delay from other vacuum (this will make stack effect enabled)
				return 0;

			// Apply effect and suck targets one-by-one each n seconds
			sc_start4(ss, bl, type, 100, sg->skill_lv, sg->group_id, (sg->val1 << 16) | (sg->val2), ++sg->val3 * 500, (sg->limit - DIFF_TICK(tick, sg->tick)));
			break;

		case UNT_BANDING:
			if( battle_check_target(&unit->bl, bl, BCT_ENEMY) > 0 && !(tsc && tsc->getSCE(SC_BANDING_DEFENCE)) )
				sc_start(ss, bl, SC_BANDING_DEFENCE, (status_get_lv(&unit->bl) / 5) + (sg->skill_lv * 5) - (status_get_agi(bl) / 10), 90, skill_get_time2(sg->skill_id, sg->skill_lv));
			break;

		case UNT_FIRE_MANTLE:
			if( battle_check_target(&unit->bl, bl, BCT_ENEMY) > 0 )
				skill_attack(BF_MAGIC,ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
			break;

		case UNT_ZENKAI_WATER:
		case UNT_ZENKAI_LAND:
		case UNT_ZENKAI_FIRE:
		case UNT_ZENKAI_WIND:
			if( battle_check_target(&unit->bl,bl,BCT_ENEMY) > 0 ) {
				switch( sg->unit_id ) {
					case UNT_ZENKAI_WATER:
						switch (rnd()%2 + 1) {
							case 1:
								sc_start(ss, bl, SC_FREEZE, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv));
								break;
							case 2:
								sc_start(ss, bl, SC_FREEZING, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv));
								break;
						}
						break;
					case UNT_ZENKAI_LAND:
						switch (rnd()%2 + 1) {
							case 1:
								sc_start2(ss, bl, SC_STONEWAIT, sg->val1*5, sg->skill_lv, ss->id, skill_get_time2(sg->skill_id, sg->skill_lv), skill_get_time(sg->skill_id, sg->skill_lv));
								break;
							case 2:
								sc_start2(ss, bl, SC_POISON, sg->val1*5, sg->skill_lv, ss->id, skill_get_time2(sg->skill_id, sg->skill_lv));
								break;
						}
						break;
					case UNT_ZENKAI_FIRE:
						sc_start4(ss, bl, SC_BURNING, sg->val1*5, sg->skill_lv, 1000, ss->id, 0, skill_get_time2(sg->skill_id, sg->skill_lv));
						break;
					case UNT_ZENKAI_WIND:
						switch (rnd()%3 + 1) {
							case 1:
								sc_start(ss, bl, SC_SLEEP, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv));
								break;
							case 2:
								sc_start(ss, bl, SC_SILENCE, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv));
								break;
							case 3:
								sc_start(ss, bl, SC_DEEPSLEEP, sg->val1*5, sg->skill_lv, skill_get_time2(sg->skill_id, sg->skill_lv));
								break;
						}
						break;
				}
			} else
				sc_start2(ss, bl,type,100,sg->val1,sg->val2,skill_get_time2(sg->skill_id, sg->skill_lv));
			break;

		case UNT_POISON_MIST:
			skill_attack(BF_MAGIC, ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			status_change_start(ss, bl, SC_POISON_MIST, (10 + 10 * sg->skill_lv)*100, sg->skill_lv, sg->skill_id, 0, 0, skill_get_time2(sg->skill_id, sg->skill_lv), SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			break;

		case UNT_CHAOSPANIC:
			if (tsc && tsc->getSCE(type))
				break;
			status_change_start(ss, bl, type, 3500 + (sg->skill_lv * 1500), sg->skill_lv, 0, 0, 1, skill_get_time2(sg->skill_id, sg->skill_lv), SCSTART_NOAVOID|SCSTART_NORATEDEF|SCSTART_NOTICKDEF);
			break;

		case UNT_B_TRAP:
			if (tsc && tsc->getSCE(type))
				break;
			sc_start(ss, bl, type, 100, sg->skill_lv, skill_get_time2(sg->skill_id,sg->skill_lv));
			unit->val2++; // Mark as ever been used
			break;

		case UNT_FIRE_RAIN:
			clif_skill_damage(ss,bl,tick,status_get_amotion(ss),0,
				skill_attack(skill_get_type(sg->skill_id),ss,&unit->bl,bl,sg->skill_id,sg->skill_lv,tick,SD_ANIMATION|SD_SPLASH),
				1,sg->skill_id,sg->skill_lv,DMG_SINGLE);
			break;

		case UNT_MAGMA_ERUPTION:
			skill_attack(skill_get_type(NC_MAGMA_ERUPTION_DOTDAMAGE), ss, &unit->bl, bl, NC_MAGMA_ERUPTION_DOTDAMAGE, sg->skill_lv, tick, 0);
			if (sg->skill_id == NC_MAGMA_ERUPTION)
				skill_attack(skill_get_type(NC_MAGMA_ERUPTION_DOTDAMAGE), ss, &unit->bl, bl, NC_MAGMA_ERUPTION_DOTDAMAGE, sg->skill_lv, tick, 0);
			else
				skill_attack(skill_get_type(NPC_MAGMA_ERUPTION_DOTDAMAGE), ss, &unit->bl, bl, NPC_MAGMA_ERUPTION_DOTDAMAGE, sg->skill_lv, tick, 0);
			break;

		case UNT_ACIDIFIED_ZONE_WATER:
			skill_attack(skill_get_type(BO_ACIDIFIED_ZONE_WATER_ATK), ss, &unit->bl, bl, BO_ACIDIFIED_ZONE_WATER_ATK, sg->skill_lv, tick, 0);
			break;

		case UNT_ACIDIFIED_ZONE_GROUND:
			skill_attack(skill_get_type(BO_ACIDIFIED_ZONE_GROUND_ATK), ss, &unit->bl, bl, BO_ACIDIFIED_ZONE_GROUND_ATK, sg->skill_lv, tick, 0);
			break;

		case UNT_ACIDIFIED_ZONE_WIND:
			skill_attack(skill_get_type(BO_ACIDIFIED_ZONE_WIND_ATK), ss, &unit->bl, bl, BO_ACIDIFIED_ZONE_WIND_ATK, sg->skill_lv, tick, 0);
			break;

		case UNT_ACIDIFIED_ZONE_FIRE:
			skill_attack(skill_get_type(BO_ACIDIFIED_ZONE_FIRE_ATK), ss, &unit->bl, bl, BO_ACIDIFIED_ZONE_FIRE_ATK, sg->skill_lv, tick, 0);
			break;

		case UNT_ASTRAL_STRIKE:
			skill_attack(skill_get_type(AG_ASTRAL_STRIKE_ATK), ss, &unit->bl, bl, AG_ASTRAL_STRIKE_ATK, sg->skill_lv, tick, 0);
			break;

		case UNT_ABYSS_SQUARE: {
				short flag = 0;

				// Check to see if the caster is in the AoE.
				if (distance_bl(ss, &unit->bl) <= unit->range)
					flag |= 2; // If yes, skill hits twice.

				skill_attack(skill_get_type(sg->skill_id), ss, &unit->bl, bl, sg->skill_id, sg->skill_lv, tick, flag);
			}
			break;
	}

	if (bl->type == BL_MOB && ss != bl)
		mobskill_event((TBL_MOB*)bl, ss, tick, MSC_SKILLUSED|(skill_id<<16));

	return skill_id;
}

/**
 * Triggered when a char steps out of a skill unit
 * @param src Skill unit from char moved out
 * @param bl Char
 * @param tick
 */
int skill_unit_onout(struct skill_unit *src, struct block_list *bl, t_tick tick)
{
	status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	nullpo_ret(src);
	nullpo_ret(bl);

	std::shared_ptr<s_skill_unit_group> sg = src->group;

	if (sg == nullptr)
		return 0;

	sc = status_get_sc(bl);
	type = skill_get_sc(sg->skill_id);
	sce = (sc && type != SC_NONE)?sc->getSCE(type):nullptr;

	if (bl->prev == nullptr || (status_isdead(*bl) && sg->unit_id != UNT_ANKLESNARE)) //Need to delete the trap if the source died.
		return 0;

	switch(sg->unit_id){
		case UNT_SAFETYWALL:
		case UNT_PNEUMA:
		case UNT_EPICLESIS://Arch Bishop
			if (sce)
				status_change_end(bl, type);
			break;

#ifndef RENEWAL
		case UNT_BASILICA:
			if (sce && sce->val4 != bl->id)
				status_change_end(bl, type);
			break;
#endif

		case UNT_HERMODE:	//Clear Hermode if the owner moved.
			if (sce && sce->val3 == BCT_SELF && sce->val4 == sg->group_id)
				status_change_end(bl, type);
			break;

		case UNT_DISSONANCE:
		case UNT_UGLYDANCE: //Used for updating timers in song overlap instances
			{
				for(uint16 i = BA_WHISTLE; i <= DC_SERVICEFORYOU; i++) {
					if(skill_get_inf2(i, INF2_ISSONG)) {
						type = skill_get_sc(i);
						sce = (sc && type != SC_NONE)?sc->getSCE(type):nullptr;
						if(sce)
							return i;
					}
				}
			}
			[[fallthrough]];
		case UNT_WHISTLE:
		case UNT_ASSASSINCROSS:
		case UNT_POEMBRAGI:
		case UNT_APPLEIDUN:
		case UNT_HUMMING:
		case UNT_DONTFORGETME:
		case UNT_FORTUNEKISS:
		case UNT_SERVICEFORYOU:
			if (sg->src_id==bl->id && !(sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_BARDDANCER))
				return -1;
	}
	return sg->skill_id;
}

/**
 * Triggered when a char steps out of a skill group (entirely) [Skotlex]
 * @param skill_id Skill ID
 * @param bl A char
 * @param tick
 */
int skill_unit_onleft(uint16 skill_id, struct block_list *bl, t_tick tick)
{
	status_change *sc;
	struct status_change_entry *sce;
	enum sc_type type;

	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = nullptr;

	type = skill_get_sc(skill_id);
	sce = (sc && type != SC_NONE)?sc->getSCE(type):nullptr;

	switch (skill_id)
	{
		case WZ_QUAGMIRE:
			if (bl->type==BL_MOB)
				break;
			if (sce)
				status_change_end(bl, type);
			break;

		case BD_LULLABY:
		case BD_RICHMANKIM:
		case BD_ETERNALCHAOS:
		case BD_DRUMBATTLEFIELD:
		case BD_RINGNIBELUNGEN:
		case BD_ROKISWEIL:
		case BD_INTOABYSS:
		case BD_SIEGFRIED:
			if(sc && sc->getSCE(SC_DANCING) && (sc->getSCE(SC_DANCING)->val1&0xFFFF) == skill_id)
			{	//Check if you just stepped out of your ensemble skill to cancel dancing. [Skotlex]
				//We don't check for SC_LONGING because someone could always have knocked you back and out of the song/dance.
				//FIXME: This code is not perfect, it doesn't checks for the real ensemble's owner,
				//it only checks if you are doing the same ensemble. So if there's two chars doing an ensemble
				//which overlaps, by stepping outside of the other parther's ensemble will cause you to cancel
				//your own. Let's pray that scenario is pretty unlikely and noone will complain too much about it.
				status_change_end(bl, SC_DANCING);
			}
			[[fallthrough]];
		case MH_STEINWAND:
		case MG_SAFETYWALL:
		case AL_PNEUMA:
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
		case CG_HERMODE:
#ifndef RENEWAL
		case HW_GRAVITATION:
		case HP_BASILICA:
#endif
		case NJ_SUITON:
		case SC_MAELSTROM:
		case EL_WATER_BARRIER:
		case EL_ZEPHYR:
		case EL_POWER_OF_GAIA:
		case SO_WARMER:
		case SO_FIRE_INSIGNIA:
		case SO_WATER_INSIGNIA:
		case SO_WIND_INSIGNIA:
		case SO_EARTH_INSIGNIA:
		case SJ_BOOKOFCREATINGSTAR:
		case SC_BLOODYLUST:
		case GN_FIRE_EXPANSION_SMOKE_POWDER:
		case GN_FIRE_EXPANSION_TEAR_GAS:
		case LG_KINGS_GRACE:
		case NC_STEALTHFIELD:
		case NC_NEUTRALBARRIER:
		case SU_NYANGGRASS:
			if (sce)
				status_change_end(bl, type);
			break;
		case BA_DISSONANCE:
		case DC_UGLYDANCE: //Used for updating song timers in overlap instances
			{
				for(uint16 i = BA_WHISTLE; i <= DC_SERVICEFORYOU; i++){
					if(skill_get_inf2(i, INF2_ISSONG)){
						type = skill_get_sc(i);
						sce = (sc && type != SC_NONE)?sc->getSCE(type):nullptr;
						if(sce && !sce->val4){ //We don't want dissonance updating this anymore
							delete_timer(sce->timer, status_change_timer);
							sce->val4 = 1; //Store the fact that this is a "reduced" duration effect.
							sce->timer = add_timer(tick+skill_get_time2(i,1), status_change_timer, bl->id, type);
						}
					}
				}
			}
			break;
		case BA_POEMBRAGI:
		case BA_WHISTLE:
		case BA_ASSASSINCROSS:
		case BA_APPLEIDUN:
		case DC_HUMMING:
		case DC_DONTFORGETME:
		case DC_FORTUNEKISS:
		case DC_SERVICEFORYOU:
			if (sce)
			{
				delete_timer(sce->timer, status_change_timer);
				//NOTE: It'd be nice if we could get the skill_lv for a more accurate extra time, but alas...
				//not possible on our current implementation.
				sce->val4 = 1; //Store the fact that this is a "reduced" duration effect.
				sce->timer = add_timer(tick+skill_get_time2(skill_id,1), status_change_timer, bl->id, type);
			}
			break;
		case PF_FOGWALL:
			if (sce)
			{
				status_change_end(bl, type);
				if ((sce=sc->getSCE(SC_BLIND)))
				{
					if (bl->type == BL_PC) //Players get blind ended inmediately, others have it still for 30 secs. [Skotlex]
						status_change_end(bl, SC_BLIND);
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
				status_change_end(bl, type);
			break;
	}

	return skill_id;
}

/*==========================================
 * Invoked when a unit cell has been placed/removed/deleted.
 * flag values:
 * flag&1: Invoke onplace function (otherwise invoke onout)
 * flag&4: Invoke a onleft call (the unit might be scheduled for deletion)
 * flag&8: Recursive
 *------------------------------------------*/
static int skill_unit_effect(struct block_list* bl, va_list ap)
{
	struct skill_unit* unit = va_arg(ap,struct skill_unit*);
	t_tick tick = va_arg(ap,t_tick);
	unsigned int flag = va_arg(ap,unsigned int);
	uint16 skill_id;
	bool dissonance = false;
	bool isTarget = false;

	if( (!unit->alive && !(flag&4)) || bl->prev == nullptr )
		return 0;

	std::shared_ptr<s_skill_unit_group> group = unit->group;

	if (group == nullptr)
		return 0;

	if( !(flag&8) ) {
		dissonance = skill_dance_switch(unit, 0);
		//Target-type check.
		isTarget = group->bl_flag & bl->type && battle_check_target( &unit->bl, bl, group->target_flag ) > 0;
	}

	//Necessary in case the group is deleted after calling on_place/on_out [Skotlex]
	skill_id = group->skill_id;
	if( isTarget ){
		if( flag&1 )
			skill_unit_onplace(unit,bl,tick);
		else {
			if( skill_unit_onout(unit,bl,tick) == -1 )
				return 0; // Don't let a Bard/Dancer update their own song timer
		}

		if( flag&4 )
			skill_unit_onleft(skill_id, bl, tick);
	} else if( !isTarget && flag&4 && ( group->state.song_dance&0x1 || ( group->src_id == bl->id && group->state.song_dance&0x2 ) ) )
		skill_unit_onleft(skill_id, bl, tick);//Ensemble check to terminate it.

	if( dissonance ) {
		skill_dance_switch(unit, 1);
		//we placed a dissonance, let's update
		map_foreachincell(skill_unit_effect,unit->bl.m,unit->bl.x,unit->bl.y,group->bl_flag,&unit->bl,gettick(),4|8);
	}

	return 0;
}

/**
 * Check skill unit while receiving damage
 * @param unit Skill unit
 * @param damage Received damage
 * @return Damage
 */
int64 skill_unit_ondamaged(struct skill_unit *unit, int64 damage)
{
	nullpo_ret(unit);

	std::shared_ptr<s_skill_unit_group> sg = unit->group;

	if (sg == nullptr)
		return 0;

	switch( sg->unit_id ) {
		case UNT_BLASTMINE:
		case UNT_SKIDTRAP:
		case UNT_LANDMINE:
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
		case UNT_CLAYMORETRAP:
		case UNT_FREEZINGTRAP:
		case UNT_ANKLESNARE:
		case UNT_ICEWALL:
		case UNT_WALLOFTHORN:
		case UNT_REVERBERATION:
		case UNT_NETHERWORLD:
			unit->val1 -= (int)cap_value(damage,INT_MIN,INT_MAX);
			break;
		default:
			damage = 0;
			break;
	}

	return damage;
}

/**
 * Check char condition around the skill caster
 * @param bl Char around area
 * @param *c Counter for 'valid' condition found
 * @param *p_sd Stores 'rid' of char found
 * @param skill_id Skill ID
 * @param skill_lv Level of used skill
 */
int skill_check_condition_char_sub (struct block_list *bl, va_list ap)
{
	int *c, skill_id;
	struct block_list *src;
	map_session_data *sd;
	map_session_data *tsd;
	int *p_sd;	//Contains the list of characters found.

	nullpo_ret(bl);
	nullpo_ret(tsd=(map_session_data*)bl);
	nullpo_ret(src=va_arg(ap,struct block_list *));
	nullpo_ret(sd=(map_session_data*)src);

	c=va_arg(ap,int *);
	p_sd = va_arg(ap, int *);
	skill_id = va_arg(ap,int);

	bool is_chorus = skill_get_inf2(skill_id, INF2_ISCHORUS);

	if (skill_id == PR_BENEDICTIO) {
		if (*c >= 2) // Check for two companions for Benedictio. [Skotlex]
			return 0;
	}
	else if (is_chorus) {
		if (*c == MAX_PARTY) // Check for partners for Chorus; Cap if the entire party is accounted for.
			return 0;
	}
	else if (*c >= 1) // Check for one companion for all other cases.
		return 0;

	if (bl == src)
		return 0;

	if(pc_isdead(tsd))
		return 0;

	if (tsd->sc.cant.cast)
		return 0;

	if( is_chorus ) {
		if( tsd->status.party_id && sd->status.party_id &&
				tsd->status.party_id == sd->status.party_id &&
				(tsd->class_&MAPID_THIRDMASK) == MAPID_MINSTRELWANDERER )
			p_sd[(*c)++] = tsd->bl.id;
		return 1;
	} else {

		switch(skill_id) {
			case PR_BENEDICTIO: {
				uint8 dir = map_calc_dir(&sd->bl,tsd->bl.x,tsd->bl.y);
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
			case TR_GEF_NOCTURN:
			case TR_ROKI_CAPRICCIO:
			case TR_AIN_RHAPSODY:
			case TR_MUSICAL_INTERLUDE:
			case TR_JAWAII_SERENADE:
			case TR_NIPELHEIM_REQUIEM:
			case TR_PRON_MARCH:// Does the partner's learned skill level affects anything? [Rytech]
				if (sd->status.sex != tsd->status.sex && (tsd->class_&MAPID_FOURTHMASK) == MAPID_TROUBADOURTROUVERE &&
					sd->status.party_id && tsd->status.party_id && sd->status.party_id == tsd->status.party_id)
					p_sd[(*c)++] = tsd->bl.id;
				return 1;
			default: //Warning: Assuming Ensemble Dance/Songs for code speed. [Skotlex]
				{
					uint16 skill_lv;
					if(pc_issit(tsd) || !unit_can_move(&tsd->bl))
						return 0;
					if (sd->status.sex != tsd->status.sex &&
							(tsd->class_&MAPID_UPPERMASK) == MAPID_BARDDANCER &&
							(skill_lv = pc_checkskill(tsd, skill_id)) > 0 &&
							(tsd->weapontype1==W_MUSICAL || tsd->weapontype1==W_WHIP) &&
							sd->status.party_id && tsd->status.party_id &&
							sd->status.party_id == tsd->status.party_id &&
							!tsd->sc.getSCE(SC_DANCING))
					{
						p_sd[(*c)++]=tsd->bl.id;
						return skill_lv;
					}
				}
				break;
		}

	}
	return 0;
}

/**
 * Checks and stores partners for ensemble skills [Skotlex]
 * Max partners is 2.
 * @param sd Caster
 * @param skill_id
 * @param skill_lv
 * @param range Area range to check
 * @param cast_flag Special handle
 */
int skill_check_pc_partner(map_session_data *sd, uint16 skill_id, uint16 *skill_lv, int range, int cast_flag)
{
	static int c=0;
	static int p_sd[MAX_PARTY];
	int i;
	bool is_chorus = skill_get_inf2(skill_id, INF2_ISCHORUS);

	if (!sd)
		return 0;

	if (!battle_config.player_skill_partner_check || pc_has_permission(sd, PC_PERM_SKILL_UNCONDITIONAL))
		return is_chorus ? MAX_PARTY : 99; //As if there were infinite partners.

	if (cast_flag) {	//Execute the skill on the partners.
		map_session_data* tsd;
		switch (skill_id) {
			case PR_BENEDICTIO:
			case WM_GREAT_ECHO:
				for (i = 0; i < c; i++) {
					if ((tsd = map_id2sd(p_sd[i])) != nullptr)
						status_charge(&tsd->bl, 0, (skill_id == PR_BENEDICTIO) ? 10 : skill_get_sp(skill_id, *skill_lv) / 2);
				}
				return c;
			case AB_ADORAMUS:
				if( c > 0 && (tsd = map_id2sd(p_sd[0])) != nullptr ) {
					i = 2 * (*skill_lv);
					status_charge(&tsd->bl, 0, i);
				}
				break;
			default:
				if( is_chorus )
					break;//Chorus skills are not to be parsed as ensembles
				if (skill_get_inf2(skill_id, INF2_ISENSEMBLE)) {
					if (c > 0 && sd->sc.getSCE(SC_DANCING) && (tsd = map_id2sd(p_sd[0])) != nullptr) {
						sd->sc.getSCE(SC_DANCING)->val4 = tsd->bl.id;
						sc_start4(&sd->bl,&tsd->bl,SC_DANCING,100,skill_id,sd->sc.getSCE(SC_DANCING)->val2,*skill_lv,sd->bl.id,skill_get_time(skill_id,*skill_lv)+1000);
						clif_skill_nodamage(&tsd->bl, sd->bl, skill_id, *skill_lv);
						tsd->skill_id_dance = skill_id;
						tsd->skill_lv_dance = *skill_lv;
#ifdef RENEWAL
						sc_start(&sd->bl, &sd->bl, SC_ENSEMBLEFATIGUE, 100, 1, skill_get_time(CG_SPECIALSINGER, *skill_lv));
						sc_start(&sd->bl, &tsd->bl, SC_ENSEMBLEFATIGUE, 100, 1, skill_get_time(CG_SPECIALSINGER, *skill_lv));
#endif
					}
				}
				return c;
		}
	}

	//Else: new search for partners.
	c = 0;
	memset (p_sd, 0, sizeof(p_sd));
	i = map_foreachinallrange(skill_check_condition_char_sub, &sd->bl, range, BL_PC, &sd->bl, &c, &p_sd, skill_id);

	if ( skill_id != PR_BENEDICTIO && skill_id != AB_ADORAMUS && skill_id != WM_GREAT_ECHO && 
		!(skill_id >= TR_GEF_NOCTURN && skill_id <= TR_PRON_MARCH)) //Apply the average lv to encore skills.
		*skill_lv = (i+(*skill_lv))/(c+1); //I know c should be one, but this shows how it could be used for the average of n partners.
	return c;
}

/**
 * Sub function to count how many spawned mob is around.
 * Some skills check with matched AI.
 * @param rid Source ID
 * @param mob_class Monster ID
 * @param skill_id Used skill
 * @param *c Counter for found monster
 */
static int skill_check_condition_mob_master_sub(struct block_list *bl, va_list ap)
{
	int *c,src_id,mob_class,skill;
	uint16 ai;
	struct mob_data *md;

	md=(struct mob_data*)bl;
	src_id=va_arg(ap,int);
	mob_class=va_arg(ap,int);
	skill=va_arg(ap,int);
	c=va_arg(ap,int *);

	switch (skill) {
		case AM_SPHEREMINE:
			ai = AI_SPHERE;
			break;
		case AM_CANNIBALIZE:
			ai = AI_FLORA;
			break;
		case KO_ZANZOU:
			ai = AI_ZANZOU;
			break;
		case MH_SUMMON_LEGION:
			ai = AI_LEGION;
			break;
		case NC_SILVERSNIPER:
		case NC_MAGICDECOY:
			ai = AI_FAW;
			break;
		case MT_SUMMON_ABR_BATTLE_WARIOR:
		case MT_SUMMON_ABR_DUAL_CANNON:
		case MT_SUMMON_ABR_MOTHER_NET:
		case MT_SUMMON_ABR_INFINITY:
			ai = AI_ABR;
			break;
		case BO_WOODENWARRIOR:
		case BO_WOODEN_FAIRY:
		case BO_CREEPER:
		case BO_HELLTREE:
			ai = AI_BIONIC;
			break;
		default:
			ai = AI_FLORA;
			break;
	}

	if( md->master_id != src_id || md->special_state.ai != ai)
		return 0; //Non alchemist summoned mobs have nothing to do here.

	if(md->mob_id==mob_class)
		(*c)++;

	return 1;
}

/**
 * Determines if a given skill should be made to consume ammo
 * when used by the player. [Skotlex]
 * @param sd Player
 * @param skill_id Skill ID
 * @return True if skill is need ammo; False otherwise.
 */
int skill_isammotype(map_session_data *sd, unsigned short skill_id)
{
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	return (
		battle_config.arrow_decrement == 2 &&
		(sd->status.weapon == W_BOW || (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE)) &&
		skill_id != HT_PHANTASMIC &&
		skill->skill_type == BF_WEAPON &&
		!skill->nk[NK_NODAMAGE] &&
		!skill_get_spiritball(skill_id,1) //Assume spirit spheres are used as ammo instead.
	);
}

/**
* Check SC required to cast a skill
* @param sc
* @param skill_id
* @return True if condition is met, False otherwise
**/
static bool skill_check_condition_sc_required( map_session_data& sd, uint16 skill_id, s_skill_condition& require ){
	if (require.status.empty())
		return true;

	status_change *sc = &sd.sc;

	if (sc == nullptr) {
		clif_skill_fail( sd, skill_id );
		return false;
	}

	// May have multiple requirements
	for (const auto &reqStatus : require.status) {
		if (reqStatus == SC_NONE)
			continue;

		useskill_fail_cause cause;

		switch (reqStatus) {
			// Official fail message
			case SC_PUSH_CART:
				cause = USESKILL_FAIL_CART;
				break;
			case SC_POISONINGWEAPON:
				cause = USESKILL_FAIL_GC_POISONINGWEAPON;
				break;
			case SC_WEAPONBLOCK_ON:
				cause = USESKILL_FAIL_GC_WEAPONBLOCKING;
				break;
			default:
				cause = USESKILL_FAIL_LEVEL;
				break;
		}

		if (!sc->getSCE(reqStatus)) {
			clif_skill_fail( sd, skill_id, cause );
			return false;
		}
	}

	return true;
}

/**
* Check SC of BL
* @param sc
* @param skill_id
* @return 1 if condition is met, 0 otherwise
**/

int skill_check_bl_sc(struct block_list *target, va_list ap) {

	nullpo_ret(target);

	int sc_id = va_arg(ap,int);

	status_change *sc = status_get_sc(target);

	if (sc && sc->getSCE(sc_id))
		return 1;

	return 0;

}

/** 
 * Check skill condition when cast begin
 * For ammo, only check if the skill need ammo
 * For checking ammo requirement (type and amount) will be skill_check_condition_castend
 * @param sd Player who uses skill
 * @param skill_id ID of used skill
 * @param skill_lv Level of used skill
 * @return true: All condition passed, false: Failed
 */
bool skill_check_condition_castbegin( map_session_data& sd, uint16 skill_id, uint16 skill_lv ){
	struct s_skill_condition require;
	int i;

	if (sd.chatID)
		return false;

	if( pc_has_permission(&sd, PC_PERM_SKILL_UNCONDITIONAL) && sd.skillitem != skill_id )
	{	//GMs don't override the skillItem check, otherwise they can use items without them being consumed! [Skotlex]
		sd.state.arrow_atk = skill_get_ammotype(skill_id)?1:0; //Need to do arrow state check.
		sd.spiritball_old = sd.spiritball; //Need to do Spiritball check.
		sd.soulball_old = sd.soulball; //Need to do Soulball check.
		sd.servantball_old = sd.servantball; //Need to do Servantball check.
		sd.abyssball_old = sd.abyssball; //Need to do Abyssball check.
		return true;
	}

	switch( sd.menuskill_id ) {
		case AM_PHARMACY:
			switch( skill_id ) {
				case AM_PHARMACY:
				case AC_MAKINGARROW:
				case BS_REPAIRWEAPON:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
					return false;
			}
			break;
		case GN_MIX_COOKING:
		case GN_MAKEBOMB:
		case GN_S_PHARMACY:
		case GN_CHANGEMATERIAL:
		case MT_M_MACHINE:
		case BO_BIONIC_PHARMACY:
			if( sd.menuskill_id != skill_id )
				return false;
			break;
	}

	struct status_data* status = &sd.battle_status;
	status_change* sc = &sd.sc;

	if( !sc->count )
		sc = nullptr;

	if( sd.skillitem == skill_id )
	{
		if( sd.state.abra_flag ) // Hocus-Pocus was used. [Inkfish]
			sd.state.abra_flag = 0;
		else
		{ // When a target was selected, consume items that were skipped in pc_use_item [Skotlex]
			if( (i = sd.itemindex) == -1 ||
				sd.inventory.u.items_inventory[i].nameid != sd.itemid ||
				sd.inventory_data[i] == nullptr ||
				sd.inventory_data[i]->flag.delay_consume == DELAYCONSUME_NONE ||
				sd.inventory.u.items_inventory[i].amount < 1
				)
			{	//Something went wrong, item exploit?
				sd.itemid = 0;
				sd.itemindex = -1;
				return false;
			}
			//Consume
			sd.itemid = 0;
			sd.itemindex = -1;
			if( (skill_id == WZ_EARTHSPIKE && sc && sc->getSCE(SC_EARTHSCROLL) && rnd()%100 > sc->getSCE(SC_EARTHSCROLL)->val2) || sd.inventory_data[i]->flag.delay_consume & DELAYCONSUME_NOCONSUME ) // [marquis007]
				; //Do not consume item.
			else if( sd.inventory.u.items_inventory[i].expire_time == 0 )
				pc_delitem(&sd,i,1,0,0,LOG_TYPE_CONSUME); // Rental usable items are not consumed until expiration
		}
		if(!sd.skillitem_keep_requirement)
			return true;
	}

	if( pc_is90overweight(&sd) ) {
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_WEIGHTOVER );
		return false;
	}

	if( sc && ( sc->getSCE(SC__SHADOWFORM) || sc->getSCE(SC__IGNORANCE) ) )
		return false;

	//Checks if disabling skill - in which case no SP requirements are necessary
	if( sc && skill_disable_check(*sc,skill_id))
		return true;

	std::bitset<INF2_MAX> inf2 = skill_db.find(skill_id)->inf2;

	// Check the skills that can be used while mounted on a warg
	if( pc_isridingwug(&sd) ) {
		if(!inf2[INF2_ALLOWONWARG])
			return false; // in official there is no message.
	}
	if( pc_ismadogear(&sd) ) {
		// Skills that are unusable when Mado is equipped. [Jobbie]
		if(!inf2[INF2_ALLOWONMADO]){
			clif_skill_fail( sd, skill_id, USESKILL_FAIL_MADOGEAR_RIDE );
			return false;
		}
	}

	//if (skill_lv < 1 || skill_lv > MAX_SKILL_LEVEL)
	//	return false;

	require = skill_get_requirement(&sd,skill_id,skill_lv);

	//Can only update state when weapon/arrow info is checked.
	sd.state.arrow_atk = require.ammo?1:0;

	// perform skill-group checks
	if(skill_id != WM_GREAT_ECHO && inf2[INF2_ISCHORUS]) {
		if (skill_check_pc_partner(&sd, skill_id, &skill_lv, AREA_SIZE, 0) < 1) {
			clif_skill_fail( sd, skill_id );
			return false;
		}
	}
	else if(inf2[INF2_ISENSEMBLE]) {
		if (skill_check_pc_partner(&sd, skill_id, &skill_lv, 1, 0) < 1 && !(sc && sc->getSCE(SC_KVASIR_SONATA))) {
			clif_skill_fail( sd, skill_id );
			return false;
		}
	}
	// perform skill-specific checks (and actions)
	switch( skill_id ) {
		case RG_GRAFFITI:
			if (map_foreachinmap(skill_graffitiremover,sd.bl.m,BL_SKILL,0)) { // If a previous Graffiti exists skill fails to cast.
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SO_SPELLFIST:
			if(sd.skill_id_old != MG_FIREBOLT && sd.skill_id_old != MG_COLDBOLT && sd.skill_id_old != MG_LIGHTNINGBOLT) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			[[fallthrough]];
		case SA_CASTCANCEL:
			if(sd.ud.skilltimer == INVALID_TIMER) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case AS_CLOAKING:
		{
			if( skill_lv < 3 && ((sd.bl.type == BL_PC && battle_config.pc_cloak_check_type&1)
			||	(sd.bl.type != BL_PC && battle_config.monster_cloak_check_type&1) )) { //Check for walls.
				int dir;
				ARR_FIND( 0, DIR_MAX, dir, map_getcell(sd.bl.m, sd.bl.x+dirx[dir], sd.bl.y+diry[dir], CELL_CHKNOPASS) != 0 );
				if( dir == DIR_MAX ) {
					clif_skill_fail( sd, skill_id );
					return false;
				}
			}
			break;
		}
		case AL_WARP:
			if(!battle_config.duel_allow_teleport && sd.duel_group) { // duel restriction [LuzZza]
				char output[128];
				sprintf(output, msg_txt(&sd,365), skill_get_name(AL_WARP));
				clif_displaymessage(sd.fd, output); //"Duel: Can't use %s in duel."
				return false;
			}
			break;
		case AL_HOLYWATER:
			if(pc_search_inventory(&sd,ITEMID_EMPTY_BOTTLE) < 0) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case MO_CALLSPIRITS:
			if(sc && sc->getSCE(SC_RAISINGDRAGON))
				skill_lv += sc->getSCE(SC_RAISINGDRAGON)->val1;
			if(sd.spiritball >= skill_lv) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case MO_FINGEROFFENSIVE:
		case GS_FLING:
		case SR_RIDEINLIGHTNING:
			if( sd.spiritball > 0 && sd.spiritball < require.spiritball )
				sd.spiritball_old = require.spiritball = sd.spiritball;
			else
				sd.spiritball_old = require.spiritball;
			break;
		case MO_CHAINCOMBO:
			if(!sc)
				return false;
			if(sc->getSCE(SC_BLADESTOP))
				break;
			if(sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == MO_TRIPLEATTACK)
				break;
			return false;
		case MO_COMBOFINISH:
			if(!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == MO_CHAINCOMBO))
				return false;
			break;
		case CH_TIGERFIST:
			if(!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == MO_COMBOFINISH))
				return false;
			break;
		case CH_CHAINCRUSH:
			if(!(sc && sc->getSCE(SC_COMBO)))
				return false;
			if(sc->getSCE(SC_COMBO)->val1 != MO_COMBOFINISH && sc->getSCE(SC_COMBO)->val1 != CH_TIGERFIST)
				return false;
			break;
		case SJ_SOLARBURST:
			if (!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SJ_PROMINENCEKICK))
				return 0;
			break;
		case MO_EXTREMITYFIST:
	//		if(sc && sc->getSCE(SC_EXTREMITYFIST)) //To disable Asura during the 5 min skill block uncomment this...
	//			return false;
			if( sc && (sc->getSCE(SC_BLADESTOP) || sc->getSCE(SC_CURSEDCIRCLE_ATKER)) )
				break;
			if( sc && sc->getSCE(SC_COMBO) ) {
				switch(sc->getSCE(SC_COMBO)->val1) {
					case MO_COMBOFINISH:
					case CH_TIGERFIST:
					case CH_CHAINCRUSH:
						break;
					default:
						return false;
				}
			}
			else if( !unit_can_move(&sd.bl) ) { //Placed here as ST_MOVE_ENABLE should not apply if rooted or on a combo. [Skotlex]
				clif_skill_fail( sd, skill_id );
				return false;
			}
			sd.spiritball_old = sd.spiritball;
			break;
		case TK_MISSION:
			if( (sd.class_&MAPID_UPPERMASK) != MAPID_TAEKWON ) { // Cannot be used by Non-Taekwon classes
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ASC_EDP:
#ifdef RENEWAL
			if (sd.weapontype1 == W_FIST && battle_config.switch_remove_edp&2) {
#else
			if (sd.weapontype1 == W_FIST && battle_config.switch_remove_edp&1) {
#endif
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_THIS_WEAPON );
				return false;
			}
			break;
		case TK_READYCOUNTER:
		case TK_READYDOWN:
		case TK_READYSTORM:
		case TK_READYTURN:
		case TK_JUMPKICK:
			if( (sd.class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER ) { // Soul Linkers cannot use this skill
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case TK_TURNKICK:
		case TK_STORMKICK:
		case TK_DOWNKICK:
		case TK_COUNTER:
			if ((sd.class_&MAPID_UPPERMASK) == MAPID_SOUL_LINKER)
				return false; //Anti-Soul Linker check in case you job-changed with Stances active.
			if(!(sc && sc->getSCE(SC_COMBO)) || sc->getSCE(SC_COMBO)->val1 == TK_JUMPKICK)
				return false; //Combo needs to be ready

			if (sc->getSCE(SC_COMBO)->val3) {	//Kick chain
				//Do not repeat a kick.
				if (sc->getSCE(SC_COMBO)->val3 != skill_id)
					break;
				status_change_end(&sd.bl, SC_COMBO);
				return false;
			}
			if(sc->getSCE(SC_COMBO)->val1 != skill_id && !pc_is_taekwon_ranker(&sd)) {	//Cancel combo wait.
				unit_cancel_combo(&sd.bl);
				return false;
			}
			break; //Combo ready.
#ifndef RENEWAL
		case BD_ADAPTATION:
			{
				int time;
				if(!(sc && sc->getSCE(SC_DANCING))) {
					clif_skill_fail( sd, skill_id );
					return false;
				}
				time = 1000*(sc->getSCE(SC_DANCING)->val3>>16);
				if (skill_get_time(
					(sc->getSCE(SC_DANCING)->val1&0xFFFF), //Dance Skill ID
					(sc->getSCE(SC_DANCING)->val1>>16)) //Dance Skill LV
					- time < skill_get_time2(skill_id,skill_lv))
				{
					clif_skill_fail( sd, skill_id );
					return false;
				}
			}
			break;
#endif
		case PR_BENEDICTIO:
			if (skill_check_pc_partner(&sd, skill_id, &skill_lv, 1, 0) < 2) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SL_SMA:
			if(!sc || !(sc->getSCE(SC_SMA) || sc->getSCE(SC_USE_SKILL_SP_SHA)))
				return false;
			break;
		case HT_POWER:
			if(!(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == AC_DOUBLE))
				return false;
			break;
#ifndef RENEWAL
		case CG_HERMODE:
			if(!npc_check_areanpc(1,sd.bl.m,sd.bl.x,sd.bl.y,skill_get_splash(skill_id, skill_lv))) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
#endif
		case CG_MOONLIT: //Check there's no wall in the range+1 area around the caster. [Skotlex]
			{
				int s,range = skill_get_splash(skill_id, skill_lv)+1;
				int size = range*2+1;
				for (s=0;s<size*size;s++) {
					int x = sd.bl.x+(s%size-range);
					int y = sd.bl.y+(s/size-range);
					if (map_getcell(sd.bl.m,x,y,CELL_CHKWALL)) {
						clif_skill_fail( sd, skill_id );
						return false;
					}
				}
			}
			break;
#ifndef RENEWAL
		case PR_REDEMPTIO:
			{
				t_exp exp = pc_nextbaseexp(&sd);
				uint32 exp_needp = battle_config.exp_cost_redemptio;

				if (exp_needp && (exp > 0 && get_percentage_exp(sd.status.base_exp, exp) < exp_needp)) {
					clif_skill_fail( sd, skill_id ); //Not enough exp.
					return false;
				}
				break;
			}
		case HP_BASILICA:
			if( !sc || (sc && !sc->getSCE(SC_BASILICA))) {
				// When castbegin, needs 7x7 clear area
				int s,range = skill_get_unit_layout_type(skill_id,skill_lv)+1;
				int size = range*2+1;
				for( s=0;s<size*size;s++ ) {
					int x = sd.bl.x+(s%size-range);
					int y = sd.bl.y+(s/size-range);
					if( map_getcell(sd.bl.m,x,y,CELL_CHKWALL) ) {
						clif_skill_fail( sd, skill_id, USESKILL_FAIL );
						return false;
					}
				}
				if( map_foreachinallrange(skill_count_wos, &sd.bl, range, BL_MOB|BL_PC, &sd.bl) ) {
					clif_skill_fail( sd, skill_id, USESKILL_FAIL );
					return false;
				}
			}
			break;
#endif
		case AM_TWILIGHT2:
		case AM_TWILIGHT3:
			if (!party_skill_check(&sd, sd.status.party_id, skill_id, skill_lv)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SG_SUN_WARM:
		case SG_MOON_WARM:
		case SG_STAR_WARM:
			if (sc && sc->getSCE(SC_MIRACLE))
				break;
			i = skill_id-SG_SUN_WARM;
			if (sd.bl.m == sd.feel_map[i].m)
				break;
			clif_skill_fail( sd, skill_id );
			return false;
			break;
		case SG_SUN_COMFORT:
		case SG_MOON_COMFORT:
		case SG_STAR_COMFORT:
			if (sc && sc->getSCE(SC_MIRACLE))
				break;
			i = skill_id-SG_SUN_COMFORT;
			if (sd.bl.m == sd.feel_map[i].m &&
				(battle_config.allow_skill_without_day || sg_info[i].day_func()))
				break;
			clif_skill_fail( sd, skill_id );
			return false;
		case SG_FUSION:
			if (sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_STAR)
				break;
			//Auron insists we should implement SP consumption when you are not Soul Linked. [Skotlex]
			//Only invoke on skill begin cast (instant cast skill). [Kevin]
			if( require.sp > 0 ) {
				if (status->sp < (unsigned int)require.sp)
					clif_skill_fail( sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT );
				else
					status_zap(&sd.bl, 0, require.sp);
			}
			return false;
		case GD_BATTLEORDER:
		case GD_REGENERATION:
		case GD_RESTORE:
		case GD_CHARGESHOUT_FLAG:
		case GD_CHARGESHOUT_BEATING:
		case GD_EMERGENCY_MOVE:
			if (!map_flag_gvg2(sd.bl.m)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			[[fallthrough]];
		case GD_EMERGENCYCALL:
		case GD_ITEMEMERGENCYCALL:
			// other checks were already done in skill_isNotOk()
			if (!sd.status.guild_id || (sd.state.gmaster_flag == 0 && skill_id != GD_CHARGESHOUT_BEATING))
				return false;
			break;

		case GS_GLITTERING:
		case RL_RICHS_COIN:
			if(sd.spiritball >= 10) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case NJ_ISSEN:
#ifdef RENEWAL
			if (status->hp < (status->hp/100)) {
#else
			if (status->hp < 2) {
#endif
				clif_skill_fail( sd, skill_id );
				return false;
			}
			[[fallthrough]];
		case NJ_BUNSINJYUTSU:
			if (!(sc && sc->getSCE(SC_NEN))) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case NJ_ZENYNAGE:
		case KO_MUCHANAGE:
			if(sd.status.zeny < require.zeny) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_MONEY );
				return false;
			}
			break;
		case PF_HPCONVERSION:
			if (status->sp == status->max_sp)
				return false; //Unusable when at full SP.
			break;
		case SP_KAUTE: // Fail if below 30% MaxHP.
			if (status->hp < 30 * status->max_hp / 100) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL );
				return false;
			}
			break;
		case AM_CALLHOMUN: //Can't summon if a hom is already out
			if (sd.status.hom_id && sd.hd && !sd.hd->homunculus.vaporize) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case AM_REST: //Can't vapo homun if you don't have an active homunc or it's hp is < 80%
			if (!hom_is_active(sd.hd) || sd.hd->battle_status.hp < (sd.hd->battle_status.max_hp*80/100)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case AB_ANCILLA: {
				int count = 0;

				for( i = 0; i < MAX_INVENTORY; i++ )
					if( sd.inventory.u.items_inventory[i].nameid == ITEMID_ANCILLA )
						count += sd.inventory.u.items_inventory[i].amount;
				if( count >= 3 ) {
					clif_skill_fail( sd, skill_id, USESKILL_FAIL_ANCILLA_NUMOVER );
					return false;
				}
			}
			break;
		case AB_ADORAMUS: // bugreport:7647 mistress card DOES remove requirements for gemstones from Adoramus -helvetica
			if( skill_check_pc_partner(&sd,skill_id,&skill_lv,1,0) <= 0 && require.itemid[0]
				&& sd.special_state.no_gemstone == 0
				&& ((i = pc_search_inventory(&sd,require.itemid[0])) < 0 || sd.inventory.u.items_inventory[i].amount < require.amount[0]) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case WL_SUMMONFB:
		case WL_SUMMONBL:
		case WL_SUMMONWB:
		case WL_SUMMONSTONE:
			if (skill_lv == 1 && sc) { // Failure only happens on level 1
				ARR_FIND(SC_SPHERE_1, SC_SPHERE_5 + 1, i, !sc->getSCE(i));

				if (i == SC_SPHERE_5 + 1) { // No more free slots
					clif_skill_fail( sd, skill_id, USESKILL_FAIL_SUMMON );
					return false;
				}
			}
			break;
		case WL_TETRAVORTEX: // bugreport:7598 moved sphere check to precast to avoid triggering cooldown per official behavior -helvetica
		case WL_RELEASE: {
				int active_spheres = 0, req_spheres = 0;

				for (i = SC_SPHERE_1; i <= SC_SPHERE_5; i++) {
					if (sc && sc->getSCE(i))
						active_spheres++;
				}

				// Cast requirement
				if (skill_id == WL_TETRAVORTEX)
					req_spheres = 4;
				else if (skill_id == WL_RELEASE && skill_lv == 2) // Only level 2 uses Spheres
					req_spheres = 1;

				if (active_spheres < req_spheres) { // Need minimum amount of spheres
					clif_skill_fail( sd, skill_id, (skill_id == WL_RELEASE) ? USESKILL_FAIL_SUMMON_NONE : USESKILL_FAIL_LEVEL );
					return false;
				}
			}
			break;
		case GC_HALLUCINATIONWALK:
			if( sc && (sc->getSCE(SC_HALLUCINATIONWALK) || sc->getSCE(SC_HALLUCINATIONWALK_POSTDELAY)) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case NPC_HALLUCINATIONWALK:
			if( sc && sc->getSCE(SC_NPC_HALLUCINATIONWALK) ) {
				return false;
			}
			break;
		case RA_WUGMASTERY:
			if( (pc_isfalcon(&sd) && (!pc_checkskill(&sd, WH_HAWK_M) && !battle_config.warg_can_falcon)) || pc_isridingwug(&sd) || sd.sc.getSCE(SC__GROOMY)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case RA_WUGSTRIKE:
			if( !pc_iswug(&sd) && !pc_isridingwug(&sd) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case RA_WUGRIDER:
			if( (pc_isfalcon(&sd) && (!pc_checkskill(&sd, WH_HAWK_M) && !battle_config.warg_can_falcon)) || ( !pc_isridingwug(&sd) && !pc_iswug(&sd) ) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case RA_WUGDASH:
			if(!pc_isridingwug(&sd)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			else {
				int16 sx = sd.bl.x;
				int16 sy = sd.bl.y;
				uint8 dir = unit_getdir(&sd.bl) % DIR_MAX;
				
				switch (dir) {
					case DIR_NORTH: sy++; break;
					case DIR_NORTHWEST: sx--; sy++; break;
					case DIR_WEST: sx--; break;
					case DIR_SOUTHWEST: sx--; sy--; break;
					case DIR_SOUTH: sy--; break;
					case DIR_SOUTHEAST: sx++; sy--; break;
					case DIR_EAST: sx++; break;
					case DIR_NORTHEAST: sx++; sy++; break;
				}
				if (map_count_oncell(sd.bl.m, sx, sy, BL_CHAR, 1) > 0) {
					return false;
				}
			}
			break;
		case LG_RAYOFGENESIS:
		case LG_BANDING:
			if( sc && sc->getSCE(SC_INSPIRATION) )
				return true; // Don't check for partner.
			break;
		case LG_PRESTIGE:
			if( sc && sc->getSCE(SC_INSPIRATION) )
				return true; // Don't check for partner.
			if( sc && sc->getSCE(SC_BANDING) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case LG_RAGEBURST:
			if( sd.spiritball == 0 ) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_SKILLINTERVAL );
				return false;
			}
			sd.spiritball_old = require.spiritball = sd.spiritball;
			break;
		case SR_FALLENEMPIRE:
			if( !(sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SR_DRAGONCOMBO) )
				return false;
			break;

		case SR_CRESCENTELBOW:
			if( sc && sc->getSCE(SC_CRESCENTELBOW) ) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_DUPLICATE );
				return false;
			}
			break;
		case SR_CURSEDCIRCLE:
			if (map_flag_gvg2(sd.bl.m)) {
				if (map_foreachinallrange(mob_count_sub, &sd.bl, skill_get_splash(skill_id, skill_lv), BL_MOB,
					MOBID_EMPERIUM, MOBID_GUARDIAN_STONE1, MOBID_GUARDIAN_STONE2)) {
					char output[128];

					sprintf(output,"%s",msg_txt(&sd,382)); // You're too close to a stone or emperium to use this skill.
					clif_messagecolor(&sd.bl,color_table[COLOR_RED], output, false, SELF);
					return false;
				}
			}
			if( sd.spiritball > 0 )
				sd.spiritball_old = require.spiritball = sd.spiritball;
			else {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SR_GATEOFHELL:
			if( sd.spiritball > 0 )
				sd.spiritball_old = require.spiritball;
			break;
		case SC_MANHOLE:
		case SC_DIMENSIONDOOR:
			if( sc && sc->getSCE(SC_MAGNETICFIELD) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SC_FEINTBOMB:
			if( map_getcell(sd.bl.m,sd.bl.x,sd.bl.y,CELL_CHKLANDPROTECTOR) || map_getcell(sd.bl.m,sd.bl.x,sd.bl.y,CELL_CHKMAELSTROM) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case WM_GREAT_ECHO: {
				int count = skill_check_pc_partner(&sd, skill_id, &skill_lv, AREA_SIZE, 1);

				if (count > 0)
					require.sp -= require.sp * 20 * count / 100; // -20% each W/M in the party.
			}
			break;
		case SO_FIREWALK:
		case SO_ELECTRICWALK:
		case NPC_FIREWALK:
		case NPC_ELECTRICWALK:	// Can't be casted until you've walked all cells.
			if( sc && sc->getSCE(SC_PROPERTYWALK) &&
			   sc->getSCE(SC_PROPERTYWALK)->val3 < skill_get_maxcount(sc->getSCE(SC_PROPERTYWALK)->val1,sc->getSCE(SC_PROPERTYWALK)->val2) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SO_EL_CONTROL:
			if( !sd.status.ele_id || !sd.ed ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case KO_JYUMONJIKIRI:
			if (sd.weapontype1 != W_FIST && (sd.weapontype2 != W_FIST || sd.status.shield != W_FIST))
				return true;
			else {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case KO_KAHU_ENTEN:
		case KO_HYOUHU_HUBUKI:
		case KO_KAZEHU_SEIRAN:
		case KO_DOHU_KOUKAI:
			if (sd.spiritcharm_type == skill_get_ele(skill_id,skill_lv) && sd.spiritcharm >= MAX_SPIRITCHARM) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_SUMMON );
				return false;
			}
			break;
		case KO_KAIHOU:
		case KO_ZENKAI:
			if (sd.spiritcharm_type == CHARM_TYPE_NONE || sd.spiritcharm <= 0) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_SUMMON_NONE );
				return false;
			}
			break;
		case SJ_FULLMOONKICK:
			if (!(sc && sc->getSCE(SC_NEWMOON))) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SJ_STAREMPEROR:
		case SJ_NOVAEXPLOSING:
		case SJ_GRAVITYCONTROL:
		case SJ_BOOKOFDIMENSION:
		case SJ_BOOKOFCREATINGSTAR:
		case SP_SOULDIVISION:
		case SP_SOULEXPLOSION:
			if (!map_flag_vs(sd.bl.m)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case SP_SWHOO:
			if (!(sc && sc->getSCE(SC_USE_SKILL_SP_SPA)))
				return false;
			break;
		case DK_SERVANT_W_PHANTOM:
		case DK_SERVANT_W_DEMOL:
			if (sd.servantball > 0 && sd.servantball < require.spiritball)
				sd.servantball_old = require.spiritball = sd.servantball;
			else
				sd.servantball_old = require.spiritball;
			break;
		case IQ_SECOND_FAITH:
		case IQ_THIRD_PUNISH:
			if (!(sc && (sc->getSCE(SC_FIRST_FAITH_POWER) || sc->getSCE(SC_SECOND_JUDGE) || sc->getSCE(SC_THIRD_EXOR_FLAME))))
				return false;
			break;
		case IQ_SECOND_JUDGEMENT:
		case IQ_THIRD_CONSECRATION:
			if (!(sc && (sc->getSCE(SC_SECOND_JUDGE) || sc->getSCE(SC_THIRD_EXOR_FLAME))))
				return false;
			break;
		case IQ_SECOND_FLAME:
		case IQ_THIRD_FLAME_BOMB:
			if (!(sc && sc->getSCE(SC_THIRD_EXOR_FLAME)))
				return false;
			break;
	}

	/* check state required */
	switch (require.state) {
		case ST_HIDDEN:
			if(!pc_ishiding(&sd)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_RIDING:
			if(!pc_isriding(&sd) && !pc_isridingdragon(&sd)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_FALCON:
			if(!pc_isfalcon(&sd)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_CART:
			if(!pc_iscarton(&sd)) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_CART );
				return false;
			}
			break;
		case ST_SHIELD:
			if(sd.status.shield <= 0) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_RECOVER_WEIGHT_RATE:
#ifdef RENEWAL
			if(pc_is70overweight(&sd)) {
#else
			if(pc_is50overweight(&sd)) {
#endif
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_MOVE_ENABLE:
			if (sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == skill_id)
				sd.ud.canmove_tick = gettick(); //When using a combo, cancel the can't move delay to enable the skill. [Skotlex]

			if (!unit_can_move(&sd.bl)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_WATER:
			if (sc && (sc->getSCE(SC_DELUGE) || sc->getSCE(SC_SUITON)))
				break;
			if (map_getcell(sd.bl.m,sd.bl.x,sd.bl.y,CELL_CHKWATER) && !map_getcell(sd.bl.m,sd.bl.x,sd.bl.y,CELL_CHKLANDPROTECTOR))
				break;
			clif_skill_fail( sd, skill_id );
			return false;
		case ST_RIDINGDRAGON:
			if( !pc_isridingdragon(&sd) ) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_DRAGON );
				return false;
			}
			break;
		case ST_WUG:
			if( !pc_iswug(&sd) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_RIDINGWUG:
			if( !pc_isridingwug(&sd) ) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_MADO:
			if( !pc_ismadogear(&sd) ) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_MADOGEAR );
				return false;
			}
			break;
		case ST_ELEMENTALSPIRIT:
		case ST_ELEMENTALSPIRIT2:
			if(!sd.ed) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_EL_SUMMON );
				return false;
			}
			break;
		case ST_PECO:
			if(!pc_isriding(&sd)) {
				clif_skill_fail( sd, skill_id );
				return false;
			}
			break;
		case ST_SUNSTANCE:
			if (!(sc && (sc->getSCE(SC_SUNSTANCE) || sc->getSCE(SC_UNIVERSESTANCE)))) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL );
				return false;
			}
			break;
		case ST_MOONSTANCE:
			if (!(sc && (sc->getSCE(SC_LUNARSTANCE) || sc->getSCE(SC_UNIVERSESTANCE)))) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL );
				return false;
			}
			break;
		case ST_STARSTANCE:
			if (!(sc && (sc->getSCE(SC_STARSTANCE) || sc->getSCE(SC_UNIVERSESTANCE)))) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL );
				return false;
			}
			break;
		case ST_UNIVERSESTANCE:
			if (!(sc && sc->getSCE(SC_UNIVERSESTANCE))) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL );
				return false;
			}
			break;
	}

	/* check the status required */
	if (!require.status.empty()) {
		switch (skill_id) {
			// Being checked later in skill_check_condition_castend()
			case WZ_SIGHTRASHER:
				break;
			default:
				if (!skill_check_condition_sc_required(sd, skill_id, require))
					return false;
				break;
		}
	}

	// Check for equipped item(s)
	if (!require.eqItem.empty()) {
		size_t count = require.eqItem.size();

		for (const auto &it : require.eqItem) {
			t_itemid reqeqit = it;

			if (!reqeqit)
				break; // Skill has no required item(s); get out of here
			switch(skill_id) { // Specific skills require multiple items while default will handle singular cases
				case NC_PILEBUNKER:
				case RL_P_ALTER:
					if (!pc_checkequip2(&sd,reqeqit,EQI_ACC_L,EQI_MAX)) {
						count--;
						if (!count) {
							if( skill_id == RL_P_ALTER ){
								clif_msg( &sd, MSI_FAIL_NEED_EQUIPPED_PROPERTY_SAINT_BULLET );
							}else{
								clif_skill_fail( sd, skill_id, USESKILL_FAIL_THIS_WEAPON );
							}
							return false;
						} else
							continue;
					}
					break;
				case NC_ACCELERATION:
				case NC_SELFDESTRUCTION:
				case NC_SHAPESHIFT:
				case NC_EMERGENCYCOOL:
				case NC_MAGNETICFIELD:
				case NC_NEUTRALBARRIER:
				case NC_STEALTHFIELD:
					if (pc_search_inventory(&sd, reqeqit) == -1) {
						count--;
						if (!count) {
							clif_skill_fail( sd, skill_id, USESKILL_FAIL_NEED_EQUIPMENT, 0, require.eqItem[0] );
							return false;
						} else
							continue;
					}
					break;
				default:
					if (!pc_checkequip2(&sd,reqeqit,EQI_ACC_L,EQI_MAX)) {
						clif_skill_fail( sd, skill_id, USESKILL_FAIL_NEED_EQUIPMENT, 0, reqeqit );
						return false;
					}
					break;
			}
		}
	}

	if(require.mhp > 0 && get_percentage(status->hp, status->max_hp) > require.mhp) {
		//mhp is the max-hp-requirement, that is,
		//you must have this % or less of HP to cast it.
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT );
		return false;
	}

	if( require.weapon && !pc_check_weapontype(&sd,require.weapon) ) {
		switch(skill_id) {
			case RA_AIMEDBOLT:
				break;
			default:
				switch((unsigned int)log2(require.weapon)) {
					case W_REVOLVER:
						clif_msg(&sd, MSI_FAIL_NEED_EQUIPPED_GUN_HANDGUN);
						break;
					case W_RIFLE:
						clif_msg(&sd, MSI_FAIL_NEED_EQUIPPED_GUN_RIFLE);
						break;
					case W_GATLING:
						clif_msg(&sd, MSI_FAIL_NEED_EQUIPPED_GUN_GATLING);
						break;
					case W_SHOTGUN:
						clif_msg(&sd, MSI_FAIL_NEED_EQUIPPED_GUN_SHOTGUN);
						break;
					case W_GRENADE:
						clif_msg(&sd, MSI_FAIL_NEED_EQUIPPED_GUN_GRANADE);
						break;
					default:
						clif_skill_fail( sd, skill_id, USESKILL_FAIL_THIS_WEAPON );
						break;
				}
				return false;
		}
	}

	if( require.sp > 0 && status->sp < (unsigned int)require.sp) {
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_SP_INSUFFICIENT );
		return false;
	}

	if (require.ap > 0 && status->ap < (unsigned int)require.ap) {
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_AP_INSUFFICIENT );
		return false;
	}

	if( require.zeny > 0 && sd.status.zeny < require.zeny ) {
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_MONEY );
		return false;
	}

	if (require.spiritball > 0) { // Skills that require certain types of spheres to use.
		switch (skill_id) { // Skills that require soul spheres.
			case SP_SOULGOLEM:
			case SP_SOULSHADOW:
			case SP_SOULFALCON:
			case SP_SOULFAIRY:
			case SP_SOULCURSE:
			case SP_SPA:
			case SP_SHA:
			case SP_SWHOO:
			case SP_SOULUNITY:
			case SP_SOULDIVISION:
			case SP_SOULREAPER:
			case SP_SOULEXPLOSION:
			case SP_KAUTE:
				if (sd.soulball < require.spiritball) {
					clif_skill_fail( sd, skill_id, USESKILL_FAIL_SPIRITS );
					return false;
				}
				break;

			// Skills that requires servants.
			case DK_SERVANT_W_SIGN:
			case DK_SERVANT_W_PHANTOM:
			case DK_SERVANT_W_DEMOL:
				if (sd.servantball < require.spiritball) {
					clif_skill_fail( sd, skill_id, USESKILL_FAIL_SPIRITS );
					return false;
				}
				break;

			default: // Skills that require spirit/coin spheres.
				if (sd.spiritball < require.spiritball) {
					if ((sd.class_&MAPID_BASEMASK) == MAPID_GUNSLINGER || (sd.class_&MAPID_UPPERMASK) == MAPID_REBELLION)
						clif_skill_fail( sd, skill_id, USESKILL_FAIL_COINS, (require.spiritball == -1) ? 1 : require.spiritball );
					else
						clif_skill_fail( sd, skill_id, USESKILL_FAIL_SPIRITS, (require.spiritball == -1) ? 1 : require.spiritball );
					return false;
				}
				break;
		}
	}

	return true;
}

/**
 * Check skill condition when cast end.
 * Checking ammo requirement (type and amount) will be here, not at skill_check_condition_castbegin
 * @param sd Player who uses skill
 * @param skill_id ID of used skill
 * @param skill_lv Level of used skill
 * @return true: All condition passed, false: Failed
 */
bool skill_check_condition_castend( map_session_data& sd, uint16 skill_id, uint16 skill_lv ){
	struct s_skill_condition require;
	struct status_data *status;
	int i;
	short index[MAX_SKILL_ITEM_REQUIRE];

	if( sd.chatID )
		return false;

	if( pc_has_permission(&sd, PC_PERM_SKILL_UNCONDITIONAL) && sd.skillitem != skill_id ) {
		//GMs don't override the skillItem check, otherwise they can use items without them being consumed! [Skotlex]
		sd.state.arrow_atk = skill_get_ammotype(skill_id)?1:0; //Need to do arrow state check.
		sd.spiritball_old = sd.spiritball; //Need to do Spiritball check.
		sd.soulball_old = sd.soulball; //Need to do Soulball check.
		sd.servantball_old = sd.servantball; //Need to do Servantball check.
		sd.abyssball_old = sd.abyssball; //Need to do Abyssball check.
		return true;
	}

	switch( sd.menuskill_id ) { // Cast start or cast end??
		case AM_PHARMACY:
			switch( skill_id ) {
				case AM_PHARMACY:
				case AC_MAKINGARROW:
				case BS_REPAIRWEAPON:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
					return false;
			}
			break;
		case GN_MIX_COOKING:
		case GN_MAKEBOMB:
		case GN_S_PHARMACY:
		case GN_CHANGEMATERIAL:
		case MT_M_MACHINE:
		case BO_BIONIC_PHARMACY:
			if( sd.menuskill_id != skill_id )
				return false;
			break;
	}

	if( sd.skillitem == skill_id && !sd.skillitem_keep_requirement ) // Casting finished (Item skill or Hocus-Pocus)
		return true;

	if( pc_is90overweight(&sd) ) {
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_WEIGHTOVER );
		return false;
	}

	// perform skill-specific checks (and actions)
	switch( skill_id ) {
		case PR_BENEDICTIO:
			skill_check_pc_partner(&sd, skill_id, &skill_lv, 1, 1);
			break;
		case AM_CANNIBALIZE:
		case AM_SPHEREMINE: {
			int c=0;
			int summons[5] = { MOBID_G_MANDRAGORA, MOBID_G_HYDRA, MOBID_G_FLORA, MOBID_G_PARASITE, MOBID_G_GEOGRAPHER };
			int maxcount = (skill_id==AM_CANNIBALIZE)? 6-skill_lv : skill_get_maxcount(skill_id,skill_lv);
			int mob_class = (skill_id==AM_CANNIBALIZE)? summons[skill_lv-1] :MOBID_MARINE_SPHERE;
			if(battle_config.land_skill_limit && maxcount>0 && (battle_config.land_skill_limit&BL_PC)) {
				i = map_foreachinmap(skill_check_condition_mob_master_sub, sd.bl.m, BL_MOB, sd.bl.id, mob_class, skill_id, &c);
				if(c >= maxcount ||
					(skill_id==AM_CANNIBALIZE && c != i && battle_config.summon_flora&2))
				{	//Fails when: exceed max limit. There are other plant types already out.
					clif_skill_fail( sd, skill_id );
					return false;
				}
			}
			break;
		}
		case NC_SILVERSNIPER:
		case NC_MAGICDECOY: {
				int c = 0;
				int maxcount = skill_get_maxcount(skill_id,skill_lv);
				int mob_class = (skill_id == NC_MAGICDECOY)? MOBID_MAGICDECOY_FIRE : MOBID_SILVERSNIPER;

				if( battle_config.land_skill_limit && maxcount > 0 && ( battle_config.land_skill_limit&BL_PC ) ) {
					if( skill_id == NC_MAGICDECOY ) {
						int j;
						for( j = mob_class; j <= MOBID_MAGICDECOY_WIND; j++ )
							map_foreachinmap(skill_check_condition_mob_master_sub, sd.bl.m, BL_MOB, sd.bl.id, j, skill_id, &c);
					} else
						map_foreachinmap(skill_check_condition_mob_master_sub, sd.bl.m, BL_MOB, sd.bl.id, mob_class, skill_id, &c);
					if( c >= maxcount ) {
						clif_skill_fail( sd, skill_id );
						return false;
					}
				}
			}
			break;
		case KO_ZANZOU: {
				int c = 0;

				i = map_foreachinmap(skill_check_condition_mob_master_sub, sd.bl.m, BL_MOB, sd.bl.id, MOBID_ZANZOU, skill_id, &c);
				if( c >= skill_get_maxcount(skill_id,skill_lv) || c != i) {
					clif_skill_fail( sd , skill_id );
					return false;
				}
			}
			break;
		case MT_SUMMON_ABR_BATTLE_WARIOR:
		case MT_SUMMON_ABR_DUAL_CANNON:
		case MT_SUMMON_ABR_MOTHER_NET:
		case MT_SUMMON_ABR_INFINITY: {
			uint32 abrs[4] = { MOBID_ABR_BATTLE_WARIOR, MOBID_ABR_DUAL_CANNON, MOBID_ABR_MOTHER_NET, MOBID_ABR_INFINITY };
			int maxcount = skill_get_maxcount(skill_id, skill_lv), c = 0;

			if (battle_config.land_skill_limit && maxcount > 0 && (battle_config.land_skill_limit & BL_PC)) {
				map_foreachinmap(skill_check_condition_mob_master_sub, sd.bl.m, BL_MOB, sd.bl.id, abrs[3 - (MT_SUMMON_ABR_INFINITY - skill_id)], skill_id, &c);
				if (c >= maxcount) {
					clif_skill_fail( sd, skill_id );
					return false;
				}
			}
			break;
		}
		case BO_WOODENWARRIOR:
		case BO_WOODEN_FAIRY:
		case BO_CREEPER:
		case BO_HELLTREE: {
			uint32 bionics[5] = { MOBID_BIONIC_WOODENWARRIOR, MOBID_BIONIC_WOODEN_FAIRY, MOBID_BIONIC_CREEPER, MOBID_PORING, MOBID_BIONIC_HELLTREE };
			int maxcount = skill_get_maxcount(skill_id, skill_lv), c = 0;

			if (battle_config.land_skill_limit && maxcount > 0 && (battle_config.land_skill_limit & BL_PC)) {
				map_foreachinmap(skill_check_condition_mob_master_sub, sd.bl.m, BL_MOB, sd.bl.id, bionics[4 - (BO_HELLTREE - skill_id)], skill_id, &c);
				if (c >= maxcount) {
					clif_skill_fail( sd, skill_id );
					return false;
				}
			}
			break;
		}
#ifdef RENEWAL
		case ASC_EDP:
			int16 item_edp = itemdb_group.item_exists_pc(&sd, IG_EDP);
			if (item_edp < 0) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_NEED_ITEM, 1, ITEMID_POISON_BOTTLE ); // [%s] required '%d' amount.
				return false;
			} else
				pc_delitem(&sd, item_edp, 1, 0, 1, LOG_TYPE_CONSUME);
			break;
#endif
	}

	status = &sd.battle_status;

	require = skill_get_requirement(&sd,skill_id,skill_lv);

	if( require.hp > 0 && status->hp <= (unsigned int)require.hp) {
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_HP_INSUFFICIENT );
		return false;
	}

	if( require.weapon && !pc_check_weapontype(&sd,require.weapon) ) {
		clif_skill_fail( sd, skill_id, USESKILL_FAIL_THIS_WEAPON );
		return false;
	}

	if( require.ammo ) { //Skill requires stuff equipped in the ammo slot.
		uint8 extra_ammo = 0;

#ifdef RENEWAL
		switch(skill_id) { // 2016-10-26 kRO update made these skills require an extra ammo to cast
			case WM_SEVERE_RAINSTORM:
			case RL_FIREDANCE:
			case RL_R_TRIP:
			case RL_FIRE_RAIN:
				extra_ammo = 1;
				break;
			default:
				break;
		}
#endif
		if((i=sd.equip_index[EQI_AMMO]) < 0 || !sd.inventory_data[i] ) {
			clif_arrow_fail( sd, ARROWFAIL_NO_AMMO );
			return false;
		} else if( sd.inventory.u.items_inventory[i].amount < require.ammo_qty + extra_ammo ) {
			char e_msg[100];
			if (require.ammo&(1<<AMMO_BULLET|1<<AMMO_GRENADE|1<<AMMO_SHELL)) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_NEED_MORE_BULLET );
				return false;
			}
			else if (require.ammo&(1<<AMMO_KUNAI)) {
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_NEED_EQUIPMENT_KUNAI );
				return false;
			}
			sprintf(e_msg,msg_txt(&sd,381), //Skill Failed. [%s] requires %dx %s.
						skill_get_desc(skill_id),
						require.ammo_qty,
						itemdb_ename(sd.inventory.u.items_inventory[i].nameid));
			clif_messagecolor(&sd.bl,color_table[COLOR_RED],e_msg,false,SELF);
			return false;
		}
		if (!(require.ammo&1<<sd.inventory_data[i]->subtype)) { //Ammo type check. Send the "wrong weapon type" message
			//which is the closest we have to wrong ammo type. [Skotlex]
			clif_arrow_fail( sd, ARROWFAIL_NO_AMMO ); //Haplo suggested we just send the equip-arrows message instead. [Skotlex]
			//clif_skill_fail( sd, skill_id, USESKILL_FAIL_THIS_WEAPON );
			return false;
		}
	}

	for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; ++i ) {
		if( !require.itemid[i] )
			continue;
		index[i] = pc_search_inventory(&sd,require.itemid[i]);
		if( index[i] < 0 || sd.inventory.u.items_inventory[index[i]].amount < require.amount[i] ) {
			if( require.itemid[i] == ITEMID_HOLY_WATER )
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_HOLYWATER ); //Holy water is required.
			else if( require.itemid[i] == ITEMID_RED_GEMSTONE )
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_REDJAMSTONE ); //Red gemstone is required.
			else if( require.itemid[i] == ITEMID_BLUE_GEMSTONE )
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_BLUEJAMSTONE ); //Blue gemstone is required.
			else if( require.itemid[i] == ITEMID_PAINT_BRUSH )
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_PAINTBRUSH ); //Paint brush is required.
			else if( require.itemid[i] == ITEMID_ANCILLA )
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_ANCILLA ); //Ancilla is required.
			else
				clif_skill_fail( sd, skill_id, USESKILL_FAIL_NEED_ITEM, require.amount[i], require.itemid[i] ); // [%s] required '%d' amount.
			return false;
		}
	}

	/* check the status required */
	if (!require.status.empty()) {
		switch (skill_id) {
			case WZ_SIGHTRASHER:
				if (!skill_check_condition_sc_required(sd, skill_id, require))
					return false;
				break;
			default:
				break;
		}
	}

	return true;
}

/** Consume skill requirement
 * @param sd Player who uses the skill
 * @param skill_id ID of used skill
 * @param skill_lv Level of used skill
 * @param type Consume type
 *  type&1: consume the others (before skill was used);
 *  type&2: consume items (after skill was used)
 */
void skill_consume_requirement(map_session_data *sd, uint16 skill_id, uint16 skill_lv, short type)
{
	struct s_skill_condition require;

	nullpo_retv(sd);

	require = skill_get_requirement(sd,skill_id,skill_lv);

	if( type&1 ) {
		switch( skill_id ) {
			case CG_TAROTCARD: // TarotCard will consume sp in skill_cast_nodamage_id [Inkfish]
			case MC_IDENTIFY:
				require.sp = 0;
				break;
			case MO_KITRANSLATION:
				//Spiritual Bestowment only uses spirit sphere when giving it to someone
				require.spiritball = 0;
				[[fallthrough]];
			default:
				if(sd->state.autocast)
					require.sp = 0;
			break;
		}
		if(require.hp || require.sp || require.ap)
			skill_consume_hpspap(&sd->bl, skill_id, require.hp, require.sp, require.ap);

		if(require.spiritball > 0) { // Skills that require certain types of spheres to use
			switch (skill_id) { // Skills that require soul spheres.
				case SP_SOULGOLEM:
				case SP_SOULSHADOW:
				case SP_SOULFALCON:
				case SP_SOULFAIRY:
				case SP_SOULCURSE:
				case SP_SPA:
				case SP_SHA:
				case SP_SWHOO:
				case SP_SOULUNITY:
				case SP_SOULDIVISION:
				case SP_SOULREAPER:
				case SP_SOULEXPLOSION:
				case SP_KAUTE:
					pc_delsoulball(sd, require.spiritball, false);
					break;

				// Skills that require servants.
				// Note: We don't update the servants display here
				// since using these skills auto trigger an animation
				// with them in unique ways that makes them vanish.
				case DK_SERVANT_W_SIGN:
				case DK_SERVANT_W_PHANTOM:
				case DK_SERVANT_W_DEMOL:
					pc_delservantball( *sd, require.spiritball );
					break;

				default: // Skills that require spirit/coin spheres.
					pc_delspiritball(sd, require.spiritball, 0);
					break;
			}
		}
		else if(require.spiritball == -1) {
			sd->spiritball_old = sd->spiritball;
			pc_delspiritball(sd,sd->spiritball,0);
		}

		if(require.zeny > 0)
		{
			if( skill_id == NJ_ZENYNAGE )
				require.zeny = 0; //Zeny is reduced on skill_attack.
			if( sd->status.zeny < require.zeny )
				require.zeny = sd->status.zeny;
			pc_payzeny(sd,require.zeny,LOG_TYPE_CONSUME);
		}
	}

	if( type&2 ) {
		status_change *sc = &sd->sc;
		int n,i;

		if( !sc->count )
			sc = nullptr;

		for( i = 0; i < MAX_SKILL_ITEM_REQUIRE; ++i )
		{
			if( !require.itemid[i] )
				continue;

			if( itemdb_group.item_exists(IG_GEMSTONE, require.itemid[i]) && skill_id != HW_GANBANTEIN && sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_WIZARD )
				continue; //Gemstones are checked, but not substracted from inventory.

			switch( skill_id ){
				case SA_SEISMICWEAPON:
					if( sc && sc->getSCE(SC_UPHEAVAL_OPTION) && rnd()%100 < 50 )
						continue;
					break;
				case SA_FLAMELAUNCHER:
				case SA_VOLCANO:
					if( sc && sc->getSCE(SC_TROPIC_OPTION) && rnd()%100 < 50 )
						continue;
					break;
				case SA_FROSTWEAPON:
				case SA_DELUGE:
					if( sc && sc->getSCE(SC_CHILLY_AIR_OPTION) && rnd()%100 < 50 )
						continue;
					break;
				case SA_LIGHTNINGLOADER:
				case SA_VIOLENTGALE:
					if( sc && sc->getSCE(SC_WILD_STORM_OPTION) && rnd()%100 < 50 )
						continue;
					break;
			}

			if( (n = pc_search_inventory(sd,require.itemid[i])) >= 0 )
				pc_delitem(sd,n,require.amount[i],0,1,LOG_TYPE_CONSUME);
		}
	}
}

/**
 * Get skill requirements and return the value after some additional/reduction condition (such item bonus and status change)
 * @param sd Player's that will be checked
 * @param skill_id Skill that's being used
 * @param skill_lv Skill level of used skill
 * @return s_skill_condition Struct 's_skill_condition' that store the modified skill requirements
 */
struct s_skill_condition skill_get_requirement(map_session_data* sd, uint16 skill_id, uint16 skill_lv)
{
	struct s_skill_condition req;
	struct status_data *status;
	status_change *sc;
	int i,hp_rate,sp_rate, ap_rate, sp_skill_rate_bonus = 100;

	memset(&req,0,sizeof(req));

	if( !sd )
		return req;

	if( sd->skillitem == skill_id && !sd->skillitem_keep_requirement )
		return req; // Item skills and Hocus-Pocus don't have requirements.[Inkfish]

	sc = &sd->sc;
	if( !sc->count )
		sc = nullptr;

	//Checks if disabling skill - in which case no SP requirements are necessary
	if( sc && skill_disable_check(*sc,skill_id) )
		return req;

	skill_lv = cap_value(skill_lv, 1, MAX_SKILL_LEVEL);

	status = &sd->battle_status;

	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	req.hp = skill->require.hp[skill_lv - 1];
	hp_rate = skill->require.hp_rate[skill_lv - 1];
	if(hp_rate > 0)
		req.hp += (status->hp * hp_rate)/100;
	else
		req.hp += (status->max_hp * (-hp_rate))/100;

	req.sp = skill->require.sp[skill_lv-1];
	if((sd->skill_id_old == BD_ENCORE) && skill_id == sd->skill_id_dance)
		req.sp /= 2;
	if ((sd->skill_id_old == TR_RETROSPECTION) && skill_id == sd->skill_id_song)
		req.sp -= req.sp * 30 / 100;
	sp_rate = skill->require.sp_rate[skill_lv-1];
	if(sp_rate > 0)
		req.sp += (status->sp * sp_rate)/100;
	else
		req.sp += (status->max_sp * (-sp_rate))/100;
	if( sd->dsprate != 100 )
		req.sp = req.sp * sd->dsprate / 100;

	for (auto &it : sd->skillusesprate) {
		if (it.id == skill_id) {
			sp_skill_rate_bonus -= it.val;
			break;
		}
	}

	for (auto &it : sd->skillusesp) {
		if (it.id == skill_id) {
			req.sp -= it.val;
			break;
		}
	}

	if (skill_id == sd->status.skill[sd->reproduceskill_idx].id)
		req.sp += req.sp * 30 / 100;

	req.sp = cap_value(req.sp * sp_skill_rate_bonus / 100, 0, SHRT_MAX);

	if( sc ) {
		if( sc->getSCE(SC__LAZINESS) )
			req.sp += req.sp + sc->getSCE(SC__LAZINESS)->val1 * 10;
		if( sc->getSCE(SC_RECOGNIZEDSPELL) )
			req.sp += req.sp / 4;
		if( sc->getSCE(SC_OFFERTORIUM))
			req.sp += req.sp * sc->getSCE(SC_OFFERTORIUM)->val3 / 100;
		if( sc->getSCE(SC_TELEKINESIS_INTENSE) && skill_get_ele(skill_id, skill_lv) == ELE_GHOST)
			req.sp -= req.sp * sc->getSCE(SC_TELEKINESIS_INTENSE)->val2 / 100;
#ifdef RENEWAL
		if (sc->getSCE(SC_ADAPTATION) && (skill_get_inf2(skill_id, INF2_ISSONG)))
			req.sp -= req.sp * 20 / 100;
		if (sc->getSCE(SC_NIBELUNGEN) && sc->getSCE(SC_NIBELUNGEN)->val2 == RINGNBL_SPCONSUM)
			req.sp -= req.sp * 30 / 100;
#endif
		if (sc->getSCE(SC_GLOOMYDAY))
			req.sp += req.sp * (skill_lv * 10) / 100;
		if (sc->getSCE(SC_CRESCIVEBOLT))
			req.sp += req.sp * (20 * sc->getSCE(SC_CRESCIVEBOLT)->val1) / 100;
	}

	req.ap = skill->require.ap[skill_lv - 1];
	ap_rate = skill->require.ap_rate[skill_lv - 1];
	if (ap_rate > 0)
		req.ap += (status->ap * ap_rate) / 100;
	else
		req.ap += (status->max_ap * (-ap_rate)) / 100;

	req.zeny = skill->require.zeny[skill_lv-1];

	if( sc && sc->getSCE(SC__UNLUCKY) ) {
		if(sc->getSCE(SC__UNLUCKY)->val1 < 3)
			req.zeny += sc->getSCE(SC__UNLUCKY)->val1 * 250;
		else
			req.zeny += 1000;
	}

	req.spiritball = skill->require.spiritball[skill_lv-1];
	req.state = skill->require.state;

	req.mhp = skill->require.mhp[skill_lv-1];
	req.weapon = skill->require.weapon;
	req.ammo_qty = skill->require.ammo_qty[skill_lv-1];
	if (skill_id == NW_MAGAZINE_FOR_ONE && sd->weapontype1 == W_GATLING)
		req.ammo_qty += 4;
	if (req.ammo_qty)
		req.ammo = skill->require.ammo;

	if (!req.ammo && skill_id && skill_isammotype(sd, skill_id))
	{	//Assume this skill is using the weapon, therefore it requires arrows.
		req.ammo = AMMO_TYPE_ALL; //Enable use on all ammo types.
		req.ammo_qty = 1;
	}

	req.status = skill->require.status;
	req.eqItem = skill->require.eqItem;

	// Level dependence flag is determined based on the ItemCost Level label
	bool level_dependent = skill->require.itemid_level_dependent;

	switch( skill_id ) {
		/* Skill level-dependent checks */
		case NC_SHAPESHIFT: // NOTE: Magic_Gear_Fuel must be last in the ItemCost list depending on the skill's max level
		case NC_REPAIR: // NOTE: Repair_Kit must be last in the ItemCost list depending on the skill's max level
			req.itemid[1] = skill->require.itemid[skill->max];
			req.amount[1] = skill->require.amount[skill->max];
			[[fallthrough]];

		/* Normal skill requirements and gemstone checks */
		default:
			for( i = 0; i < ((!level_dependent) ? MAX_SKILL_ITEM_REQUIRE : 2); i++ ) {
				// Skip this for level_dependent requirement, just looking forward for gemstone removal. Assumed if there is gemstone there.
				if (!level_dependent) {
					switch( skill_id ) {
						case AM_POTIONPITCHER:
						case CR_SLIMPITCHER:
						case CR_CULTIVATION:
							if (i != skill_lv%11 - 1)
								continue;
							break;
#ifdef RENEWAL
						case AM_CALLHOMUN:
							// Player has no homunculus, only requires first item
							if (i > 0 && sd->hd == nullptr) {
								i = MAX_SKILL_ITEM_REQUIRE;
								continue;
							}

							// Recalling from Rest state has a different consume item (stored as second item)
							if (skill_area_temp[0] == 1 || (sd->hd != nullptr && sd->hd->homunculus.vaporize == HOM_ST_REST)) {
								req.itemid[0] = skill->require.itemid[1];
								req.amount[0] = skill->require.amount[1];
								if (skill_area_temp[0] == 1 && hom_is_active(sd->hd))
									skill_area_temp[0] = 0;
								i = MAX_SKILL_ITEM_REQUIRE;
								continue;
							}
							break;
#else
						case AM_CALLHOMUN:
							if (sd->status.hom_id) //Don't delete items when hom is already out.
								continue;
							break;
#endif
						case AB_ADORAMUS:
							if( itemdb_group.item_exists(IG_GEMSTONE, skill->require.itemid[i]) && (sd->special_state.no_gemstone == 2 || skill_check_pc_partner(sd,skill_id,&skill_lv, 1, 2)) )
								continue;
							break;
					}

					req.itemid[i] = skill->require.itemid[i];
					req.amount[i] = skill->require.amount[i];

					if ((skill_id >= HT_SKIDTRAP && skill_id <= HT_TALKIEBOX && pc_checkskill(sd, RA_RESEARCHTRAP) > 0) || skill_id == SC_ESCAPE) {
						int16 itIndex;

						if ((itIndex = pc_search_inventory(sd,req.itemid[i])) < 0 || ( itIndex >= 0 && sd->inventory.u.items_inventory[itIndex].amount < req.amount[i])) {
							if (skill_id == SC_ESCAPE) // Alloy Trap has priority over normal Trap
								req.itemid[i] = ITEMID_TRAP;
							else
								req.itemid[i] = ITEMID_TRAP_ALLOY;
							req.amount[i] = 1;
						}
						break;
					}
				}
				else {
					// Process level_dependent requirement
					if (level_dependent && skill_lv <= MAX_SKILL_ITEM_REQUIRE && i == 0) {
						req.itemid[0] = skill->require.itemid[skill_lv - 1];
						req.amount[0] = skill->require.amount[skill_lv - 1];
					}
				}

				// Check requirement for gemstone.
				if (itemdb_group.item_exists(IG_GEMSTONE, req.itemid[i])) {
					if( sd->special_state.no_gemstone == 2 ) // Remove all Magic Stone required for all skills for VIP.
						req.itemid[i] = req.amount[i] = 0;
					else {
						if( sd->special_state.no_gemstone || (sc && sc->getSCE(SC_INTOABYSS)) )
						{	// All gem skills except Hocus Pocus and Ganbantein can cast for free with Mistress card -helvetica
							if (skill_id != SA_ABRACADABRA && skill_id != HW_GANBANTEIN)
		 						req.itemid[i] = req.amount[i] = 0;
							else if( --req.amount[i] < 1 )
								req.amount[i] = 1; // Hocus Pocus always use at least 1 gem
						}
					}
				}
				// Check requirement for Magic Gear Fuel
				if (req.itemid[i] == ITEMID_MAGIC_GEAR_FUEL && sd->special_state.no_mado_fuel)
					req.itemid[i] = req.amount[i] = 0;
			}
			break;
	}

	// Check for cost reductions due to skills & SCs
	switch(skill_id) {
		case MC_MAMMONITE:
#ifdef RENEWAL
		case WS_CARTTERMINATION:
#endif
			if(pc_checkskill(sd,BS_UNFAIRLYTRICK)>0)
#ifdef RENEWAL
				req.zeny -= req.zeny*20/100;
#else
				req.zeny -= req.zeny*10/100;
#endif
			break;
		case AL_HOLYLIGHT:
			if(sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_PRIEST)
				req.sp *= 5;
			break;
		case SL_SMA:
		case SL_STUN:
		case SL_STIN:
		{
			int kaina_lv = sd?pc_checkskill(sd,SL_KAINA):skill_get_max(SL_KAINA);

			if(kaina_lv==0 || !sd || sd->status.base_level<70)
				break;
			if(sd->status.base_level>=90)
				req.sp -= req.sp*7*kaina_lv/100;
			else if(sd->status.base_level>=80)
				req.sp -= req.sp*5*kaina_lv/100;
			else if(sd->status.base_level>=70)
				req.sp -= req.sp*3*kaina_lv/100;
		}
			break;
		case MO_CHAINCOMBO:
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
			if(sc && sc->getSCE(SC_SPIRIT) && sc->getSCE(SC_SPIRIT)->val2 == SL_MONK)
				req.sp = 2; //Monk Spirit makes monk/champion combo skills cost 2 SP regardless of original cost
			break;
		case MO_BODYRELOCATION:
			if( sc && sc->getSCE(SC_EXPLOSIONSPIRITS) )
				req.spiritball = 0;
			break;
		case MO_EXTREMITYFIST:
			if( sc ) {
				if( sc->getSCE(SC_BLADESTOP) )
					req.spiritball--;
				else if( sc->getSCE(SC_COMBO) ) {
#ifndef RENEWAL
					switch( sc->getSCE(SC_COMBO)->val1 ) {
						case MO_COMBOFINISH:
							req.spiritball = 4;
							break;
						case CH_TIGERFIST:
							req.spiritball = 3;
							break;
						case CH_CHAINCRUSH: //It should consume whatever is left as long as it's at least 1.
							req.spiritball = sd->spiritball?sd->spiritball:1;
							break;
					}
#else
					req.spiritball = sd->spiritball ? sd->spiritball : 1;
#endif
				} else if( sc->getSCE(SC_RAISINGDRAGON) && sd->spiritball > 5)
					req.spiritball = sd->spiritball; // must consume all regardless of the amount required
			}
			break;
		case LG_RAGEBURST:
			req.spiritball = sd->spiritball?sd->spiritball:1;
			break;
		case SR_FALLENEMPIRE:
			if (sc && (sc->getSCE(SC_FIRST_FAITH_POWER) || sc->getSCE(SC_SECOND_JUDGE) || sc->getSCE(SC_THIRD_EXOR_FLAME)))
				req.spiritball = 0;
			break;
		case SR_TIGERCANNON:
			if (sc && sc->getSCE(SC_THIRD_EXOR_FLAME))
				req.spiritball = 0;
			break;
		case SR_RAMPAGEBLASTER:
		case SR_RIDEINLIGHTNING:
			if (sc && sc->getSCE(SC_MASSIVE_F_BLASTER))
				req.spiritball = 0;
			break;
		case SR_GATEOFHELL:
			if( sc && sc->getSCE(SC_COMBO) && sc->getSCE(SC_COMBO)->val1 == SR_FALLENEMPIRE )
				req.sp -= req.sp * 10 / 100;
			break;
		case SR_FLASHCOMBO:
			if (sc && (sc->getSCE(SC_SECOND_JUDGE) || sc->getSCE(SC_THIRD_EXOR_FLAME)))
				req.spiritball = 0;
			break;
		case SO_SUMMON_AGNI:
		case SO_SUMMON_AQUA:
		case SO_SUMMON_VENTUS:
		case SO_SUMMON_TERA: {
				int spirit_sympathy = pc_checkskill(sd,SO_EL_SYMPATHY);

				if( spirit_sympathy )
					req.sp -= req.sp * (5 + 5 * spirit_sympathy) / 100;
			}
			break;
		case NPC_PSYCHIC_WAVE:
		case SO_PSYCHIC_WAVE:
			if( sc && (sc->getSCE(SC_HEATER_OPTION) || sc->getSCE(SC_COOLER_OPTION) || sc->getSCE(SC_CURSED_SOIL_OPTION) || sc->getSCE(SC_BLAST_OPTION)) )
				req.sp += req.sp / 2; // 1.5x SP cost
			break;
	}

	//Check if player is using the copied skill [Cydh]
	uint16 idx = skill_get_index(skill_id);

	if (sd->status.skill[idx].flag == SKILL_FLAG_PLAGIARIZED) {
		uint16 req_opt = skill->copyable.req_opt;

		if (req_opt & SKILL_REQ_HPCOST)
			req.hp = 0;
		if (req_opt & SKILL_REQ_MAXHPTRIGGER)
			req.mhp = 0;
		if (req_opt & SKILL_REQ_SPCOST)
			req.sp = 0;
		if (req_opt & SKILL_REQ_HPRATECOST)
			req.hp_rate = 0;
		if (req_opt & SKILL_REQ_SPRATECOST)
			req.sp_rate = 0;
		if (req_opt & SKILL_REQ_ZENYCOST)
			req.zeny = 0;
		if (req_opt & SKILL_REQ_WEAPON)
			req.weapon = 0;
		if (req_opt & SKILL_REQ_AMMO) {
			req.ammo = 0;
			req.ammo_qty = 0;
		}
		if (req_opt & SKILL_REQ_STATE)
			req.state = ST_NONE;
		if (req_opt & SKILL_REQ_STATUS) {
			req.status.clear();
			req.status.shrink_to_fit();
		}
		if (req_opt & SKILL_REQ_SPIRITSPHERECOST)
			req.spiritball = 0;
		if (req_opt & SKILL_REQ_ITEMCOST) {
			memset(req.itemid, 0, sizeof(req.itemid));
			memset(req.amount, 0, sizeof(req.amount));
		}
		if (req_opt & SKILL_REQ_EQUIPMENT) {
			req.eqItem.clear();
			req.eqItem.shrink_to_fit();
		}
		if (req_opt & SKILL_REQ_APCOST)
			req.ap = 0;
		if (req_opt & SKILL_REQ_APRATECOST)
			req.ap_rate = 0;
	}

	return req;
}

/*==========================================
 * Does cast-time reductions based on dex, item bonuses and config setting
 *------------------------------------------*/
int skill_castfix(struct block_list *bl, uint16 skill_id, uint16 skill_lv) {
	nullpo_ret(bl);

	double time = skill_get_cast(skill_id, skill_lv);

#ifndef RENEWAL_CAST
	{
		map_session_data *sd = BL_CAST(BL_PC, bl);
		status_change *sc = status_get_sc(bl);
		int reduce_cast_rate = 0;
		uint8 flag = skill_get_castnodex(skill_id);

		// Calculate base cast time (reduced by dex)
		if (!(flag&1)) {
			int scale = battle_config.castrate_dex_scale - status_get_dex(bl);

			if (scale > 0)	// not instant cast
				time = time * (float)scale / battle_config.castrate_dex_scale;
			else
				return 0; // instant cast
		}

		// Calculate cast time reduced by item/card bonuses
		if (sd) {
			if (!(flag&4)) {
				if (sd->castrate != 100)
					reduce_cast_rate += 100 - sd->castrate;
				if (sd->bonus.add_varcast != 0)
					time += sd->bonus.add_varcast; // bonus bVariableCast
			}
			// Skill-specific reductions work regardless of flag
			for (const auto &it : sd->skillcastrate) {
				if (it.id == skill_id) {
					time += time * it.val / 100;
					break;
				}
			}
			for (const auto &it : sd->skillvarcast) {
				if (it.id == skill_id) { // bonus2 bSkillVariableCast
					time += it.val;
					break;
				}
			}
		}

		// These cast time reductions are processed even if the skill fails
		if (sc && sc->count) {
			// Magic Strings stacks additively with item bonuses
			if (!(flag&2) && sc->getSCE(SC_POEMBRAGI))
				reduce_cast_rate += sc->getSCE(SC_POEMBRAGI)->val2;
			// Foresight halves the cast time, it does not stack additively
			if (sc->getSCE(SC_MEMORIZE)) {
				if (!sd || pc_checkskill(sd, skill_id) > 0) { // Foresight only decreases cast times from learned skills, not skills granted by items
					if(!(flag&2))
						time -= time * 50 / 100;
					// Foresight counter gets reduced even if the skill is not affected by it
					if ((--sc->getSCE(SC_MEMORIZE)->val2) <= 0)
						status_change_end(bl, SC_MEMORIZE);
				}
			}
		}

		time = time * (1 - (float)reduce_cast_rate / 100);
	}
#endif

	// config cast time multiplier
	if (battle_config.cast_rate != 100)
		time = time * battle_config.cast_rate / 100;
	// return final cast time
	time = max((int)time, 0);
	//ShowInfo("Castime castfix = %f\n",time);

	return (int)time;
}

#ifndef RENEWAL_CAST
/**
 * Get the skill cast time for Pre-Re cast
 * @param bl: The caster
 * @param time: Cast time before Status Change addition or reduction
 * @return time: Modified castime after status change addition or reduction
 */
int skill_castfix_sc(struct block_list *bl, double time, uint8 flag)
{
	if (time < 0)
		return 0;

	if (bl->type == BL_MOB || bl->type == BL_NPC)
		return (int)time;

	status_change *sc = status_get_sc(bl);

	if (sc && sc->count) {
		if (!(flag&2)) {
			if (sc->getSCE(SC_SLOWCAST))
				time += time * sc->getSCE(SC_SLOWCAST)->val2 / 100;
			if (sc->getSCE(SC_PARALYSIS))
				time += sc->getSCE(SC_PARALYSIS)->val3;
			if (sc->getSCE(SC_IZAYOI))
				time -= time * 50 / 100;
			if (sc->getSCE(SC_2011RWC_SCROLL))
				time -= time * 5 / 100;
		}
		if (sc->getSCE(SC_SUFFRAGIUM)) {
			if(!(flag&2))
				time -= time * sc->getSCE(SC_SUFFRAGIUM)->val2 / 100;
			//Suffragium ends even if the skill is not affected by it
			status_change_end(bl, SC_SUFFRAGIUM);
		}
	}

	time = std::max(time, 0.0);
	//ShowInfo("Castime castfix_sc = %f\n",time);

	return (int)time;
}
#else
/**
 * Get the skill cast time for RENEWAL_CAST.
 * FixedRate reduction never be stacked, always get the HIGHEST VALUE TO REDUCE (-20% vs 10%, -20% wins!)
 * Additive value:
 *    Variable CastTime : time  += value
 *    Fixed CastTime    : fixed += value
 * Multipicative value
 *    Variable CastTime : VARCAST_REDUCTION(value)
 *    Fixed CastTime    : FIXEDCASTRATE2(value)
 * @param bl: The caster
 * @param time: Cast time without reduction
 * @param skill_id: Skill ID of the casted skill
 * @param skill_lv: Skill level of the casted skill
 * @return time: Modified castime after status and bonus addition or reduction
 */
int skill_vfcastfix(struct block_list *bl, double time, uint16 skill_id, uint16 skill_lv)
{
	nullpo_ret(bl);

	if (time < 0)
		return 0;

	if (bl->type == BL_MOB || bl->type == BL_NPC)
		return (int)time;

	status_change *sc = status_get_sc(bl);
	map_session_data *sd = BL_CAST(BL_PC, bl);
	int fixed = skill_get_fixed_cast(skill_id, skill_lv), fixcast_r = 0, varcast_r = 0, reduce_cast_rate = 0;
	uint8 flag = skill_get_castnodex(skill_id);

	if (fixed < 0) {
		if (battle_config.default_fixed_castrate > 0) {
			fixed = (int)time * battle_config.default_fixed_castrate / 100; // fixed time
			time = time * (100 - battle_config.default_fixed_castrate) / 100; // variable time
		} else
			fixed = 0;
	}

	// Additive Variable Cast bonus adjustments by items
	if (sd && !(flag&4)) {
		if (sd->bonus.varcastrate != 0)
			reduce_cast_rate += sd->bonus.varcastrate; // bonus bVariableCastrate
		if (sd->bonus.fixcastrate != 0)
			fixcast_r -= sd->bonus.fixcastrate; // bonus bFixedCastrate
		if (sd->bonus.add_varcast != 0)
			time += sd->bonus.add_varcast; // bonus bVariableCast
		if (sd->bonus.add_fixcast != 0)
			fixed += sd->bonus.add_fixcast; // bonus bFixedCast
		for (const auto &it : sd->skillfixcast) {
			if (it.id == skill_id) { // bonus2 bSkillFixedCast
				fixed += it.val;
				break;
			}
		}
		for (const auto &it : sd->skillvarcast) {
			if (it.id == skill_id) { // bonus2 bSkillVariableCast
				time += it.val;
				break;
			}
		}
		for (const auto &it : sd->skillcastrate) {
			if (it.id == skill_id) { // bonus2 bVariableCastrate
				reduce_cast_rate += it.val;
				break;
			}
		}
		for (const auto &it : sd->skillfixcastrate) {
			if (it.id == skill_id) { // bonus2 bFixedCastrate
				fixcast_r = max(fixcast_r, it.val);
				break;
			}
		}
	}

	// Adjusted by active statuses
	if (sc && sc->count && !(flag&2)) {
		// Multiplicative Variable CastTime values
		if (sc->getSCE(SC_SLOWCAST))
			VARCAST_REDUCTION(-sc->getSCE(SC_SLOWCAST)->val2);
		if (sc->getSCE(SC__LAZINESS))
			VARCAST_REDUCTION(-sc->getSCE(SC__LAZINESS)->val2);
		if (sc->getSCE(SC_SUFFRAGIUM)) {
			VARCAST_REDUCTION(sc->getSCE(SC_SUFFRAGIUM)->val2);
#ifndef RENEWAL
			status_change_end(bl, SC_SUFFRAGIUM, INVALID_TIMER);
#endif
		}
		if (sc->getSCE(SC_MEMORIZE)) {
			if (!sd || pc_checkskill(sd, skill_id) > 0) { // Foresight only decreases cast times from learned skills, not skills granted by items
				reduce_cast_rate += 50;
				if ((--sc->getSCE(SC_MEMORIZE)->val2) <= 0)
					status_change_end(bl, SC_MEMORIZE);
			}
		}
		if (sc->getSCE(SC_POEMBRAGI))
			reduce_cast_rate += sc->getSCE(SC_POEMBRAGI)->val2;
		if (sc->getSCE(SC_IZAYOI))
			VARCAST_REDUCTION(50);
		if (sc->getSCE(SC_WATER_INSIGNIA) && sc->getSCE(SC_WATER_INSIGNIA)->val1 == 3 && skill_get_type(skill_id) == BF_MAGIC && skill_get_ele(skill_id, skill_lv) == ELE_WATER)
			VARCAST_REDUCTION(30); //Reduces 30% Variable Cast Time of magic Water spells.
		if (sc->getSCE(SC_TELEKINESIS_INTENSE))
			VARCAST_REDUCTION(sc->getSCE(SC_TELEKINESIS_INTENSE)->val2);
		if (sc->getSCE(SC_SOULFAIRY))
			VARCAST_REDUCTION(sc->getSCE(SC_SOULFAIRY)->val3);
		if (sc->getSCE(SC_EP16_2_BUFF_AC))
			VARCAST_REDUCTION(80);
		// Multiplicative Fixed CastTime values
		if (sc->getSCE(SC_SECRAMENT))
			fixcast_r = max(fixcast_r, sc->getSCE(SC_SECRAMENT)->val2);
		if (sd && (skill_lv = pc_checkskill(sd, WL_RADIUS) ) && skill_id >= WL_WHITEIMPRISON && skill_id <= WL_FREEZE_SP)
			fixcast_r = max(fixcast_r, ((status_get_int(bl) + status_get_lv(bl)) / 15 + skill_lv * 5));
		if (sc->getSCE(SC_DANCEWITHWUG))
			fixcast_r = max(fixcast_r, sc->getSCE(SC_DANCEWITHWUG)->val4);
		if (sc->getSCE(SC_HEAT_BARREL))
			fixcast_r = max(fixcast_r, sc->getSCE(SC_HEAT_BARREL)->val2);
		if (sc->getSCE(SC_FREEZING))
			fixcast_r -= 50;
		if (sc->getSCE(SC_SWINGDANCE))
			fixcast_r = max(fixcast_r, skill_lv * 6);
		// Additive Fixed CastTime values
		if (sc->getSCE(SC_MANDRAGORA))
			fixed += sc->getSCE(SC_MANDRAGORA)->val1 * 500;
		if (sc->getSCE(SC_GUST_OPTION) || sc->getSCE(SC_BLAST_OPTION) || sc->getSCE(SC_WILD_STORM_OPTION))
			fixed -= 1000;
		if (sc->getSCE(SC_IZAYOI))
			fixed = 0;
		if (sc->getSCE(SC_GLOOMYDAY))
			fixed += skill_lv * 500;
		if (sc->getSCE(SC_2011RWC_SCROLL))
			VARCAST_REDUCTION(5);
	}
	if (sc && sc->getSCE(SC_SECRAMENT) && skill_id == HW_MAGICPOWER && (flag&2)) // Sacrament lowers Mystical Amplification cast time
		fixcast_r = max(fixcast_r, sc->getSCE(SC_SECRAMENT)->val2);

	if (varcast_r < 0)
		time = time * (1 - (float)min(varcast_r, 100) / 100);

	// Apply Variable CastTime calculation by INT & DEX
	if (!(flag&1))
		time = time * (1 - sqrt(((float)(status_get_dex(bl) * 2 + status_get_int(bl)) / battle_config.vcast_stat_scale)));

	time = time * (1 - (float)min(reduce_cast_rate, 100) / 100);
	time = max((int)time, 0) + (1 - (float)min(fixcast_r, 100) / 100) * max(fixed, 0); //Underflow checking/capping

	return (int)time;
}
#endif

/*==========================================
 * Does delay reductions based on dex/agi, sc data, item bonuses, ...
 *------------------------------------------*/
int skill_delayfix(struct block_list *bl, uint16 skill_id, uint16 skill_lv)
{
	nullpo_ret(bl);

	if (skill_id == SA_ABRACADABRA)
		return 0; //Will use picked skill's delay.

	if (bl->type&battle_config.no_skill_delay)
		return battle_config.min_skill_delay_limit;

	int delaynodex = skill_get_delaynodex(skill_id);
	double time = skill_get_delay(skill_id, skill_lv);

	if (time < 0)
		time = -time + status_get_amotion(bl);	// If set to <0, add to attack motion.

	status_change* sc = status_get_sc(bl);

	// Delay reductions
	switch (skill_id) {	//Monk combo skills have their delay reduced by agi/dex.
		case MO_TRIPLEATTACK:
		case MO_CHAINCOMBO:
		case MO_COMBOFINISH:
		case CH_TIGERFIST:
		case CH_CHAINCRUSH:
		case SR_DRAGONCOMBO:
		case SR_FALLENEMPIRE:
		case SJ_PROMINENCEKICK:
			//If delay not specified, it will be 1000 - 4*agi - 2*dex
			if (time == 0)
				time = 1000;
			time -= (4 * status_get_agi(bl) + 2 * status_get_dex(bl));
			break;
#ifndef RENEWAL
		case HP_BASILICA:
			if (sc && !sc->getSCE(SC_BASILICA))
				time = 0; // There is no Delay on Basilica creation, only on cancel
			break;
#endif
		default:
			if (battle_config.delay_dependon_dex && !(delaynodex&1)) { // if skill delay is allowed to be reduced by dex
				int scale = battle_config.castrate_dex_scale - status_get_dex(bl);

				if (scale > 0)
					time = time * scale / battle_config.castrate_dex_scale;
				else //To be capped later to minimum.
					time = 0;
			}
			if (battle_config.delay_dependon_agi && !(delaynodex&1)) { // if skill delay is allowed to be reduced by agi
				int scale = battle_config.castrate_dex_scale - status_get_agi(bl);

				if (scale > 0)
					time = time * scale / battle_config.castrate_dex_scale;
				else //To be capped later to minimum.
					time = 0;
			}
	}

	if (sc && sc->count) {
		if (sc->getSCE(SC_SPIRIT)) {
			switch (skill_id) {
				case CR_SHIELDBOOMERANG:
					if (sc->getSCE(SC_SPIRIT)->val2 == SL_CRUSADER)
						time /= 2;
					break;
				case AS_SONICBLOW:
					if (!map_flag_gvg2(bl->m) && !map_getmapflag(bl->m, MF_BATTLEGROUND) && sc->getSCE(SC_SPIRIT)->val2 == SL_ASSASIN)
						time /= 2;
					break;
			}
		}
	}

	int delay = 0;

	if (!(delaynodex&2)) {
		if (sc && sc->count) {
			if (sc->getSCE(SC_POEMBRAGI))
				delay += sc->getSCE(SC_POEMBRAGI)->val3;
			if (sc->getSCE(SC_WIND_INSIGNIA) && sc->getSCE(SC_WIND_INSIGNIA)->val1 == 3 && skill_get_type(skill_id) == BF_MAGIC && skill_get_ele(skill_id, skill_lv) == ELE_WIND)
				delay += 50; // After Delay of Wind element spells reduced by 50%.
			if (sc->getSCE(SC_MAGICMUSHROOM) && sc->getSCE(SC_MAGICMUSHROOM)->val3 == 0)
				delay += sc->getSCE(SC_MAGICMUSHROOM)->val2;
		}
	}

	if (!(delaynodex&4) && bl->type == BL_PC) {
		map_session_data* sd = (map_session_data*)bl;

		if (sd->bonus.delayrate != 0) // bonus bDelayRate
			delay += sd->bonus.delayrate;

		for (auto &it : sd->skilldelay) { // bonus2 bSkillDelay
			if (it.id == skill_id) {
				time += it.val;
				break;
			}
		}
	}

	if (delay != 0)
		time = time * (1 - (float)min(delay, 100) / 100);

	if (battle_config.delay_rate != 100)
		time = time * battle_config.delay_rate / 100;

	//ShowInfo("Delay delayfix = %f\n",time);

	return max((int)time,0);
}


/*==========================================
 * Weapon Repair [Celest/DracoRPG]
 *------------------------------------------*/
void skill_repairweapon( map_session_data& sd, int idx ){
	static const t_itemid weapon_materials[MAX_WEAPON_LEVEL] = {
		ITEMID_IRON_ORE,
		ITEMID_IRON,
		ITEMID_STEEL,
		ITEMID_ORIDECON_STONE,
#ifdef RENEWAL
		0
#endif
	};
	static const t_itemid armor_materials[MAX_ARMOR_LEVEL] = {
		ITEMID_STEEL,
#ifdef RENEWAL
		0
#endif
	};
	t_itemid material = 0;
	struct item *item;
	map_session_data *target_sd;

	if ( !( target_sd = map_id2sd(sd.menuskill_val) ) ) //Failed....
		return;

	if( idx == 0xFFFF ) // No item selected ('Cancel' clicked)
		return;
	if( idx < 0 || idx >= MAX_INVENTORY )
		return; //Invalid index??

	item = &target_sd->inventory.u.items_inventory[idx];
	if( !item->nameid || !item->attribute )
		return; //Again invalid item....

	if (itemdb_ishatched_egg(item))
		return;

	if (sd.status.char_id != target_sd->status.char_id && !battle_check_range(&sd.bl, &target_sd->bl, skill_get_range2(&sd.bl, sd.menuskill_id, sd.menuskill_val2, true))) {
		clif_item_repaireffect( sd, idx, true );
		return;
	}

	if( target_sd->inventory_data[idx]->type == IT_WEAPON ){
		material = weapon_materials[target_sd->inventory_data[idx]->weapon_level - 1];
	}else if( target_sd->inventory_data[idx]->type == IT_ARMOR ){
		material = armor_materials[target_sd->inventory_data[idx]->armor_level - 1];
	}

	if( material == 0 || pc_search_inventory( &sd, material ) < 0 ){
		clif_skill_fail( sd, sd.menuskill_id );
		return;
	}

	clif_skill_nodamage(&sd.bl,target_sd->bl,sd.menuskill_id,1);

	item->attribute = 0;/* clear broken state */

	clif_equiplist(target_sd);

	pc_delitem(&sd,pc_search_inventory(&sd,material),1,0,0,LOG_TYPE_CONSUME);

	clif_item_repaireffect( sd, idx, false );

	if( sd.status.char_id != target_sd->status.char_id )
		clif_item_repaireffect( *target_sd, idx, false );
}

/*==========================================
 * Item Appraisal
 *------------------------------------------*/
void skill_identify(map_session_data *sd, int idx)
{
	bool failure = true;

	nullpo_retv(sd);

	sd->state.workinprogress = WIP_DISABLE_NONE;

	if(idx >= 0 && idx < MAX_INVENTORY) {
		if(sd->inventory.u.items_inventory[idx].nameid > 0 && sd->inventory.u.items_inventory[idx].identify == 0 ){
			failure = false;
			sd->inventory.u.items_inventory[idx].identify = 1;
		}
	}
	clif_item_identified( *sd, idx, failure );
}

/*==========================================
 * Weapon Refine [Celest]
 *------------------------------------------*/
void skill_weaponrefine( map_session_data& sd, int idx ){
	static const t_itemid material[MAX_WEAPON_LEVEL] = {
		ITEMID_PHRACON,
		ITEMID_EMVERETARCON,
		ITEMID_ORIDECON,
		ITEMID_ORIDECON,
#ifdef RENEWAL
		0
#endif
	};

	if (idx >= 0 && idx < MAX_INVENTORY)
	{
		struct item_data *ditem = sd.inventory_data[idx];
		struct item* item = &sd.inventory.u.items_inventory[idx];

		if(item->nameid > 0 && ditem->type == IT_WEAPON) {
			if( ditem->flag.no_refine || ditem->weapon_level < 1 ) { 	// if the item isn't refinable
				clif_skill_fail( sd, sd.menuskill_id );
				return;
			}
			if( item->refine >= sd.menuskill_val || item->refine >= 10 ) {
				clif_upgrademessage(&sd, 2, item->nameid);
				return;
			}

			int i = pc_search_inventory( &sd, material[ditem->weapon_level - 1] );

			if( i < 0 ) {
				clif_upgrademessage( &sd, 3, material[ditem->weapon_level - 1] );
				return;
			}

			std::shared_ptr<s_refine_level_info> info = refine_db.findLevelInfo( *ditem, *item );

			if( info == nullptr ){
				clif_skill_fail( sd, sd.menuskill_id );
				return;
			}

			std::shared_ptr<s_refine_cost> cost = util::umap_find( info->costs, (uint16)REFINE_COST_NORMAL );

			if( cost == nullptr ){
				clif_skill_fail( sd, sd.menuskill_id );
				return;
			}

			if( cost->nameid != material[ditem->weapon_level - 1] ){
				ShowDebug( "skill_weaponrefine: The hardcoded refine requirement %d for weapon level %d does not match %d from the refine database.\n", material[ditem->weapon_level - 1], ditem->weapon_level, cost->nameid );
				clif_skill_fail( sd, sd.menuskill_id );
				return;
			}

			int per = ( cost->chance / 100 );
			if( sd.class_&JOBL_THIRD )
				per += 10;
			else
				per += (((signed int)sd.status.job_level)-50)/2; //Updated per the new kro descriptions. [Skotlex]

			pc_delitem(&sd, i, 1, 0, 0, LOG_TYPE_OTHER);
			if (per > rnd() % 100) {
				int ep=0;
				log_pick_pc(&sd, LOG_TYPE_OTHER, -1, item);
				item->refine++;
				log_pick_pc(&sd, LOG_TYPE_OTHER,  1, item);
				if(item->equip) {
					ep = item->equip;
					pc_unequipitem(&sd,idx,3);
				}
				clif_delitem(sd,idx,1,3);
				clif_upgrademessage(&sd, 0, item->nameid);
				clif_inventorylist(&sd);
				clif_refine( sd, idx, ITEMREFINING_SUCCESS );
				if( ditem->type == IT_WEAPON ){
					achievement_update_objective(&sd, AG_ENCHANT_SUCCESS, 2, ditem->weapon_level, item->refine);
				}
				if (ep)
					pc_equipitem(&sd,idx,ep);
				clif_misceffect( sd.bl, NOTIFYEFFECT_REFINE_SUCCESS );
				if(item->refine == 10 &&
					item->card[0] == CARD0_FORGE &&
					(int)MakeDWord(item->card[2],item->card[3]) == sd.status.char_id)
				{ // Fame point system [DracoRPG]
					switch(ditem->weapon_level){
						case 1:
							pc_addfame(sd, battle_config.fame_refine_lv1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
							break;
						case 2:
							pc_addfame(sd, battle_config.fame_refine_lv2); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
							break;
						case 3:
							pc_addfame(sd, battle_config.fame_refine_lv3); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
							break;
					}
				}
			} else {
				item->refine = 0;
				if(item->equip)
					pc_unequipitem(&sd,idx,3);
				clif_upgrademessage(&sd, 1, item->nameid);
				clif_refine( sd, idx, ITEMREFINING_FAILURE );
				achievement_update_objective(&sd, AG_ENCHANT_FAIL, 1, 1);
				pc_delitem(&sd,idx,1,0,2, LOG_TYPE_OTHER);
				clif_misceffect( sd.bl, NOTIFYEFFECT_REFINE_FAILURE );
				clif_emotion(&sd.bl, ET_HUK);
			}
		}
	}
}

/*==========================================
 *
 *------------------------------------------*/
int skill_autospell(map_session_data *sd, uint16 skill_id)
{
	nullpo_ret(sd);

	if (skill_id == 0 || skill_db.get_index(skill_id, true, __FUNCTION__, __FILE__, __LINE__) == 0 || SKILL_CHK_GUILD(skill_id))
		return 0;

	uint16 lv = pc_checkskill(sd, skill_id), skill_lv = sd->menuskill_val;
	uint16 maxlv = 1;

	if (skill_lv == 0 || lv == 0)
		return 0; // Player must learn the skill before doing auto-spell [Lance]

#ifdef RENEWAL
	if ((skill_id == MG_COLDBOLT || skill_id == MG_FIREBOLT || skill_id == MG_LIGHTNINGBOLT) && sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_SAGE)
		maxlv = 10; //Soul Linker bonus. [Skotlex]
	else
		maxlv = skill_lv / 2; // Half of Autospell's level unless player learned a lower level (capped below)
#else
	if(skill_id==MG_NAPALMBEAT)	maxlv=3;
	else if(skill_id==MG_COLDBOLT || skill_id==MG_FIREBOLT || skill_id==MG_LIGHTNINGBOLT){
		if (sd->sc.getSCE(SC_SPIRIT) && sd->sc.getSCE(SC_SPIRIT)->val2 == SL_SAGE)
			maxlv = 10; //Soul Linker bonus. [Skotlex]
		else if(skill_lv==2) maxlv=1;
		else if(skill_lv==3) maxlv=2;
		else if(skill_lv>=4) maxlv=3;
	}
	else if(skill_id==MG_SOULSTRIKE){
		if(skill_lv==5) maxlv=1;
		else if(skill_lv==6) maxlv=2;
		else if(skill_lv>=7) maxlv=3;
	}
	else if(skill_id==MG_FIREBALL){
		if(skill_lv==8) maxlv=1;
		else if(skill_lv>=9) maxlv=2;
	}
	else if(skill_id==MG_FROSTDIVER) maxlv=1;
	else return 0;
#endif

	maxlv = min(lv, maxlv);

	sc_start4(&sd->bl,&sd->bl,SC_AUTOSPELL,100,skill_lv,skill_id,maxlv,0,
		skill_get_time(SA_AUTOSPELL,skill_lv));
	return 0;
}

/**
 * Count the number of players with Gangster Paradise, Peaceful Break, or Happy Break.
 * @param bl: Player object
 * @param ap: va_arg list
 * @return 1 if the player has learned Gangster Paradise, Peaceful Break, or Happy Break otherwise 0
 */
static int skill_sit_count(struct block_list *bl, va_list ap)
{
	map_session_data *sd = (map_session_data*)bl;
	int flag = va_arg(ap, int);

	if (!pc_issit(sd))
		return 0;

	if (flag&1 && pc_checkskill(sd, RG_GANGSTER) > 0)
		return 1;

	if (flag&2 && (pc_checkskill(sd, TK_HPTIME) > 0 || pc_checkskill(sd, TK_SPTIME) > 0))
		return 1;

	return 0;
}

/**
 * Triggered when a player sits down to activate bonus states.
 * @param bl: Player object
 * @param ap: va_arg list
 * @return 0
 */
static int skill_sit_in(struct block_list *bl, va_list ap)
{
	map_session_data *sd = (map_session_data*)bl;
	int flag = va_arg(ap, int);

	if (!pc_issit(sd))
		return 0;

	if (flag&1 && pc_checkskill(sd, RG_GANGSTER) > 0)
		sd->state.gangsterparadise = 1;

	if (flag&2 && (pc_checkskill(sd, TK_HPTIME) > 0 || pc_checkskill(sd, TK_SPTIME) > 0 )) {
		sd->state.rest = 1;
		status_calc_regen(bl, &sd->battle_status, &sd->regen);
		status_calc_regen_rate(bl, &sd->regen, &sd->sc);
	}

	return 0;
}

/**
 * Triggered when a player stands up to deactivate bonus states.
 * @param bl: Player object
 * @param ap: va_arg list
 * @return 0
 */
static int skill_sit_out(struct block_list *bl, va_list ap)
{
	map_session_data *sd = (map_session_data*)bl;
	int flag = va_arg(ap, int), range = va_arg(ap, int);

	if (map_foreachinallrange(skill_sit_count, &sd->bl, range, BL_PC, flag) > 1)
		return 0;

	if (flag&1 && sd->state.gangsterparadise)
		sd->state.gangsterparadise = 0;
	if (flag&2 && sd->state.rest) {
		sd->state.rest = 0;
		status_calc_regen(bl, &sd->battle_status, &sd->regen);
		status_calc_regen_rate(bl, &sd->regen, &sd->sc);
	}

	return 0;
}

/**
 * Toggle Sit icon and player bonuses when sitting/standing.
 * @param sd: Player data
 * @param sitting: True when sitting or false when standing
 * @return 0
 */
int skill_sit(map_session_data *sd, bool sitting)
{
	int flag = 0, range = 0, lv;

	nullpo_ret(sd);

	if ((lv = pc_checkskill(sd, RG_GANGSTER)) > 0) {
		flag |= 1;
		range = skill_get_splash(RG_GANGSTER, lv);
	}
	if ((lv = pc_checkskill(sd, TK_HPTIME)) > 0) {
		flag |= 2;
		range = skill_get_splash(TK_HPTIME, lv);
	} else if ((lv = pc_checkskill(sd, TK_SPTIME)) > 0) {
		flag |= 2;
		range = skill_get_splash(TK_SPTIME, lv);
	}

	if (sitting)
		clif_status_load(&sd->bl, EFST_SIT, 1);
	else
		clif_status_load(&sd->bl, EFST_SIT, 0);

	if (!flag) // No need to count area if no skills are learned.
		return 0;

	if (sitting) {
		if (map_foreachinallrange(skill_sit_count, &sd->bl, range, BL_PC, flag) > 1)
			map_foreachinallrange(skill_sit_in, &sd->bl, range, BL_PC, flag);
	} else
		map_foreachinallrange(skill_sit_out, &sd->bl, range, BL_PC, flag, range);

	return 0;
}

/*==========================================
 * Do Forstjoke/Scream effect
 *------------------------------------------*/
int skill_frostjoke_scream(struct block_list *bl, va_list ap)
{
	struct block_list *src;
	uint16 skill_id,skill_lv;
	t_tick tick;

	nullpo_ret(bl);
	nullpo_ret(src = va_arg(ap,struct block_list*));

	skill_id = va_arg(ap,int);
	skill_lv = va_arg(ap,int);
	if(!skill_lv)
		return 0;
	tick = va_arg(ap,t_tick);

	if (src == bl || status_isdead(*bl))
		return 0;
	if (bl->type == BL_PC) {
		map_session_data *sd = (map_session_data *)bl;
		if ( sd && sd->sc.option&(OPTION_INVISIBLE|OPTION_MADOGEAR) )
			return 0;//Frost Joke / Scream cannot target invisible or MADO Gear characters [Ind]
	}
	//It has been reported that Scream/Joke works the same regardless of woe-setting. [Skotlex]
	if(battle_check_target(src,bl,BCT_ENEMY|BCT_PARTY) > 0)
		skill_additional_effect(src,bl,skill_id,skill_lv,BF_MISC,ATK_DEF,tick);

	return 0;
}

/**
 * Set map cell flag as skill unit effect
 * @param src Skill unit
 * @param skill_id
 * @param skill_lv
 * @param cell Cell type cell_t
 * @param flag 0/1
 */
static void skill_unitsetmapcell(struct skill_unit *src, uint16 skill_id, uint16 skill_lv, cell_t cell, bool flag)
{
	int range = skill_get_unit_range(skill_id,skill_lv);
	int x, y;

	for( y = src->bl.y - range; y <= src->bl.y + range; ++y )
		for( x = src->bl.x - range; x <= src->bl.x + range; ++x )
			map_setcell(src->bl.m, x, y, cell, flag);
}

/**
 * Do skill attack area (such splash effect) around the 'first' target.
 * First target will skip skill condition, always receive damage. But,
 * around it, still need target/condition validation by
 * battle_check_target and status_check_skilluse
 * @param bl
 * @param ap { atk_type, src, dsrc, skill_id, skill_lv, tick, flag, type }
 */
int skill_attack_area(struct block_list *bl, va_list ap)
{
	struct block_list *src,*dsrc;
	int atk_type,skill_id,skill_lv,flag,type;
	t_tick tick;

	if(status_isdead(*bl))
		return 0;

	atk_type = va_arg(ap,int);
	src = va_arg(ap,struct block_list*);
	dsrc = va_arg(ap,struct block_list*);
	skill_id = va_arg(ap,int);
	skill_lv = va_arg(ap,int);
	tick = va_arg(ap,t_tick);
	flag = va_arg(ap,int);
	type = va_arg(ap,int);

	if (skill_area_temp[1] == bl->id) { //This is the target of the skill, do a full attack and skip target checks.
		return (int)skill_attack(atk_type,src,dsrc,bl,skill_id,skill_lv,tick,flag);
	}

	if(battle_check_target(dsrc,bl,type) <= 0 ||
		!status_check_skilluse(nullptr, bl, skill_id, 2))
		return 0;

	switch (skill_id) {
		case WZ_FROSTNOVA: //Skills that don't require the animation to be removed
			if (src->x == bl->x && src->y == bl->y)
				return 0; //Does not hit current cell
			if (map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR)) // Attack should not happen if the target is on Land Protector
				return 0;
			[[fallthrough]];
		case NPC_ACIDBREATH:
		case NPC_DARKNESSBREATH:
		case NPC_FIREBREATH:
		case NPC_ICEBREATH:
		case NPC_ICEBREATH2:
		case NPC_THUNDERBREATH:
			return (int)skill_attack(atk_type,src,dsrc,bl,skill_id,skill_lv,tick,flag);
		default:
			//Area-splash, disable skill animation.
			return (int)skill_attack(atk_type,src,dsrc,bl,skill_id,skill_lv,tick,flag|SD_ANIMATION);
	}
}

/**
 * Clear skill unit group
 * @param bl: Unit to check
 * @param flag: Skill group to clear
 */
int skill_clear_group(block_list *bl, uint8 flag)
{
	nullpo_ret(bl);

	unit_data *ud = unit_bl2ud(bl);

	if (ud == nullptr)
		return 0;

	size_t count = 0;
	bool deleted = false;

	// The after loop statement might look stupid, but this prevents iteration problems, if an entry was deleted
	for (auto it = ud->skillunits.begin(); it != ud->skillunits.end(); (deleted ? it = ud->skillunits.begin() : it++), deleted = false) {
		switch ((*it)->skill_id) {
			case SA_DELUGE:
			case SA_VOLCANO:
			case SA_VIOLENTGALE:
			case SA_LANDPROTECTOR:
			case NJ_SUITON:
			case NJ_KAENSIN:
			case SC_CHAOSPANIC:
			case MH_POISON_MIST:
			case MH_LAVA_SLIDE:
				if (flag & 1) {
					skill_delunitgroup(*it);
					count++;
					deleted = true;
				}
				break;
			case SO_CLOUD_KILL:
			case NPC_CLOUD_KILL:
				if (flag & 4) {
					skill_delunitgroup(*it);
					count++;
					deleted = true;
				}
				break;
			case SO_WARMER:
				if (flag & 8) {
					skill_delunitgroup(*it);
					count++;
					deleted = true;
				}
				break;
			default:
				if (flag & 2 && skill_get_inf2((*it)->skill_id, INF2_ISTRAP)) {
					skill_delunitgroup(*it);
					count++;
					deleted = true;
				}
				break;
		}
	}

	return static_cast<int>(count);
}

/**
 * Returns the first element field found [Skotlex]
 * @param bl
 * @return s_skill_unit_group
 */
std::shared_ptr<s_skill_unit_group> skill_locate_element_field(struct block_list *bl)
{
	nullpo_ret(bl);

	unit_data *ud = unit_bl2ud(bl);

	if (ud == nullptr)
		return nullptr;

	for (const auto su : ud->skillunits) {
		switch (su->skill_id) {
			case SA_DELUGE:
			case SA_VOLCANO:
			case SA_VIOLENTGALE:
			case SA_LANDPROTECTOR:
			case NJ_SUITON:
			case SO_CLOUD_KILL:
			case NPC_CLOUD_KILL:
			case SO_WARMER:
			case SC_CHAOSPANIC:
			case MH_POISON_MIST:
			case MH_LAVA_SLIDE:
				return su;
		}
	}
	return nullptr;
}

/// Graffiti cleaner [Valaris]
int skill_graffitiremover(struct block_list *bl, va_list ap)
{
	struct skill_unit *unit = nullptr;
	int remove = va_arg(ap, int);

	nullpo_retr(0, bl);

	if (bl->type != BL_SKILL || (unit = (struct skill_unit *)bl) == nullptr)
		return 0;

	if ((unit->group) && (unit->group->unit_id == UNT_GRAFFITI)) {
		if (remove == 1)
			skill_delunit(unit);
		return 1;
	}

	return 0;
}

/// Greed effect
int skill_greed(struct block_list *bl, va_list ap)
{
	struct block_list *src;
	map_session_data *sd = nullptr;
	struct flooritem_data *fitem = nullptr;

	nullpo_ret(bl);
	nullpo_ret(src = va_arg(ap, struct block_list *));

	if(src->type == BL_PC && (sd = (map_session_data *)src) && bl->type == BL_ITEM && (fitem = (struct flooritem_data *)bl))
		pc_takeitem(sd, fitem);

	return 0;
}

/// Ranger's Detonator [Jobbie/3CeAM]
int skill_detonator(struct block_list *bl, va_list ap)
{
	nullpo_ret(bl);

	if (bl->type != BL_SKILL)
		return 0;

	block_list *src = va_arg(ap, block_list *);
	skill_unit *unit = (skill_unit *)bl;

	if (unit == nullptr)
		return 0;

	std::shared_ptr<s_skill_unit_group> group = unit->group;

	if (group == nullptr || group->src_id != src->id)
		return 0;

	int unit_id = group->unit_id;

	switch( unit_id )
	{ //List of Hunter and Ranger Traps that can be detonate.
		case UNT_BLASTMINE:
		case UNT_SANDMAN:
		case UNT_CLAYMORETRAP:
		case UNT_TALKIEBOX:
		case UNT_CLUSTERBOMB:
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
			switch(unit_id) {
				case UNT_TALKIEBOX:
					clif_talkiebox(bl,group->valstr);
					group->val2 = -1;
					break;
				case UNT_CLAYMORETRAP:
				case UNT_FIRINGTRAP:
				case UNT_ICEBOUNDTRAP:
					map_foreachinrange(skill_trap_splash,bl,skill_get_splash(group->skill_id,group->skill_lv),group->bl_flag|BL_SKILL|~BCT_SELF,bl,group->tick);
					break;
				default:
					map_foreachinrange(skill_trap_splash,bl,skill_get_splash(group->skill_id,group->skill_lv),group->bl_flag,bl,group->tick);
					break;
			}
			if (unit->group == nullptr)
				return 0;
			clif_changetraplook(bl, UNT_USED_TRAPS);
			group->unit_id = UNT_USED_TRAPS;
			group->limit = DIFF_TICK(gettick(),group->tick) +
				(unit_id == UNT_TALKIEBOX ? 5000 : (unit_id == UNT_CLUSTERBOMB || unit_id == UNT_ICEBOUNDTRAP? 2500 : (unit_id == UNT_FIRINGTRAP ? 0 : 1500)) );
			break;
	}
	return 0;
}

/**
 * Calculate Royal Guard's Banding bonus
 * @param sd: Player data
 * @return Number of Royal Guard
 */
int skill_banding_count(map_session_data *sd)
{
 	nullpo_ret(sd);

 	return cap_value(party_foreachsamemap(party_sub_count, sd, skill_get_splash(LG_BANDING, 1)), 0, MAX_PARTY);
}

/**
 * Rebellion's Bind Trap explosion
 * @author [Cydh]
 */
static int skill_bind_trap(struct block_list *bl, va_list ap) {
	struct skill_unit *su = nullptr;
	struct block_list *src = nullptr;

	nullpo_ret(bl);

	src = va_arg(ap,struct block_list *);

	if (bl->type != BL_SKILL || !(su = (struct skill_unit *)bl) || !(su->group))
		return 0;
	if (su->group->unit_id != UNT_B_TRAP || su->group->src_id != src->id)
		return 0;

	map_foreachinallrange(skill_trap_splash, bl, su->range, BL_CHAR, bl,su->group->tick);
	clif_changetraplook(bl, UNT_USED_TRAPS);
	su->group->unit_id = UNT_USED_TRAPS;
	su->group->limit = DIFF_TICK(gettick(), su->group->tick) + 500;
	return 1;
}

/*==========================================
 * Check new skill unit cell when overlapping in other skill unit cell.
 * Catched skill in cell value pushed to *unit pointer.
 * Set (*alive) to 0 will ends 'new unit' check
 *------------------------------------------*/
static int skill_cell_overlap(struct block_list *bl, va_list ap)
{
	uint16 skill_id;
	int *alive;
	struct skill_unit *unit;

	skill_id = va_arg(ap,int);
	alive = va_arg(ap,int *);
	unit = (struct skill_unit *)bl;

	if (unit == nullptr || unit->group == nullptr || (*alive) == 0)
		return 0;

	if (unit->group->state.guildaura) /* guild auras are not cancelled! */
		return 0;

	switch (skill_id) {
		case SA_LANDPROTECTOR: {
				if( unit->group->skill_id == SA_LANDPROTECTOR ) {//Check for offensive Land Protector to delete both. [Skotlex]
					(*alive) = 0;
					skill_delunit(unit);
					return 1;
				}

				std::shared_ptr<s_skill_db> skill = skill_db.find(unit->group->skill_id);

				//It deletes everything except traps and barriers
				if ((!skill->inf2[INF2_ISTRAP] && !skill->inf2[INF2_IGNORELANDPROTECTOR]) || unit->group->skill_id == WZ_FIREPILLAR) {
					if (skill->unit_flag[UF_RANGEDSINGLEUNIT]) {
						if (unit->val2&(1 << UF_RANGEDSINGLEUNIT))
							skill_delunitgroup(unit->group);
					} else
						skill_delunit(unit);
					return 1;
				}
			}
			break;
		case GN_CRAZYWEED_ATK:
			if (skill_get_unit_flag(unit->group->skill_id, UF_CRAZYWEEDIMMUNE))
				break;
			[[fallthrough]];
		case HW_GANBANTEIN:
		case LG_EARTHDRIVE:
			// Officially songs/dances are removed
			if (skill_get_unit_flag(unit->group->skill_id, UF_RANGEDSINGLEUNIT)) {
				if (unit->val2&(1 << UF_RANGEDSINGLEUNIT))
					skill_delunitgroup(unit->group);
			} else
				skill_delunit(unit);
			return 1;
		case SA_VOLCANO:
		case SA_DELUGE:
		case SA_VIOLENTGALE:
// The official implementation makes them fail to appear when casted on top of ANYTHING
// but I wonder if they didn't actually meant to fail when casted on top of each other?
// hence, I leave the alternate implementation here, commented. [Skotlex]
			if (unit->range <= 0 && skill_get_unit_id(unit->group->skill_id) != UNT_DUMMYSKILL)
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
		case WZ_WATERBALL:
			switch (unit->group->skill_id) {
				case SA_DELUGE:
				case NJ_SUITON:
					//Consumes deluge/suiton
					skill_delunit(unit);
					return 1;
			}
			break;
		case WZ_ICEWALL:
#ifndef RENEWAL
		case HP_BASILICA:
		case HW_GRAVITATION:
#endif
			//These can't be placed on top of themselves (duration can't be refreshed)
			if (unit->group->skill_id == skill_id)
			{
				(*alive) = 0;
				return 1;
			}
			break;
		case RL_FIRE_RAIN: {
				std::bitset<UF_MAX> uf = skill_db.find(unit->group->skill_id)->unit_flag;

				if (uf[UF_REMOVEDBYFIRERAIN]) {
					if (uf[UF_RANGEDSINGLEUNIT]) {
						if (unit->val2&(1 << UF_RANGEDSINGLEUNIT))
							skill_delunitgroup(unit->group);
					} else
						skill_delunit(unit);
					return 1;
				}
			}
			break;
	}

	std::bitset<INF2_MAX> inf2 = skill_db.find(skill_id)->inf2;

	if (unit->group->skill_id == SA_LANDPROTECTOR && !inf2[INF2_ISTRAP] && !inf2[INF2_IGNORELANDPROTECTOR] ) { //It deletes everything except traps and barriers
		(*alive) = 0;
		return 1;
	}

	return 0;
}

/*==========================================
 * Splash effect for skill unit 'trap type'.
 * Chance triggered when damaged, timeout, or char step on it.
 *------------------------------------------*/
static int skill_trap_splash(struct block_list *bl, va_list ap)
{
	struct block_list *src = va_arg(ap,struct block_list *);
	struct skill_unit *unit = nullptr;
	t_tick tick = va_arg(ap,t_tick);
	struct block_list *ss; //Skill src bl

	nullpo_ret(src);

	unit = (struct skill_unit *)src;

	if (!unit || !unit->alive || bl->prev == nullptr)
		return 0;

	std::shared_ptr<s_skill_unit_group> sg = unit->group;

	if (sg == nullptr)
		return 0;

	nullpo_ret(ss = map_id2bl(sg->src_id));

	if (battle_check_target(src,bl,sg->target_flag) <= 0)
		return 0;

	switch (sg->unit_id) {
		case UNT_B_TRAP:
			if (battle_check_target(ss, bl, sg->target_flag&~BCT_SELF) > 0)
				skill_castend_damage_id(ss, bl, sg->skill_id, sg->skill_lv, tick, SD_ANIMATION|SD_LEVEL|SD_SPLASH|1);
			break;
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
			skill_additional_effect(ss,bl,sg->skill_id,sg->skill_lv,BF_MISC,ATK_DEF,tick);
			break;
		case UNT_GROUNDDRIFT_WIND:
			if(skill_attack(skill_get_type(sg->skill_id),ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(ss,bl,SC_STUN,50,sg->skill_lv,skill_get_time2(sg->skill_id, 1));
			break;
		case UNT_GROUNDDRIFT_DARK:
			if(skill_attack(skill_get_type(sg->skill_id),ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(ss,bl,SC_BLIND,50,sg->skill_lv,skill_get_time2(sg->skill_id, 2));
			break;
		case UNT_GROUNDDRIFT_POISON:
			if(skill_attack(skill_get_type(sg->skill_id),ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start2(ss,bl,SC_POISON,50,sg->skill_lv,ss->id,skill_get_time2(sg->skill_id, 3));
			break;
		case UNT_GROUNDDRIFT_WATER:
			if(skill_attack(skill_get_type(sg->skill_id),ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				sc_start(ss,bl,SC_FREEZE,50,sg->skill_lv,skill_get_time2(sg->skill_id, 4));
			break;
		case UNT_GROUNDDRIFT_FIRE:
			if(skill_attack(skill_get_type(sg->skill_id),ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1))
				skill_blown(src,bl,skill_get_blewcount(sg->skill_id,sg->skill_lv),-1,BLOWN_NONE);
			break;
		case UNT_ELECTRICSHOCKER:
			if (bl->id != ss->id) {
				if (status_bl_has_mode(bl,MD_STATUSIMMUNE))
					break;
				if (status_change_start(ss, bl, SC_ELECTRICSHOCKER, 10000, sg->skill_lv, sg->group_id, 0, 0, skill_get_time2(sg->skill_id, sg->skill_lv), SCSTART_NORATEDEF)) {
					map_moveblock(bl, unit->bl.x, unit->bl.y, tick);
					clif_fixpos( *bl );
					clif_skill_damage(src, bl, tick, 0, 0, -30000, 1, sg->skill_id, sg->skill_lv, DMG_SPLASH);
				}
			}
			break;
		case UNT_MAGENTATRAP:
		case UNT_COBALTTRAP:
		case UNT_MAIZETRAP:
		case UNT_VERDURETRAP:
			if( bl->type == BL_MOB && status_get_class_(bl) != CLASS_BOSS ) {
				status_data* status = status_get_status_data(*bl);

				status->def_ele = skill_get_ele(sg->skill_id, sg->skill_lv);
				status->ele_lv = (unsigned char)sg->skill_lv;
			}
			break;
		case UNT_REVERBERATION: // For proper skill delay animation when used with Dominion Impulse
			skill_addtimerskill(ss, tick + 50, bl->id, 0, 0, NPC_REVERBERATION_ATK, sg->skill_lv, BF_WEAPON, 0);
			break;
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
			if( src->id == bl->id ) break;
			if( bl->type == BL_SKILL ) {
				struct skill_unit *su = (struct skill_unit *)bl;

				if (su && su->group->unit_id == UNT_USED_TRAPS)
					break;
			}
			[[fallthrough]];
		case UNT_CLUSTERBOMB:
			if( ss != bl )
				skill_attack(BF_MISC,ss,src,bl,sg->skill_id,sg->skill_lv,tick,sg->val1|SD_LEVEL);
			break;
		case UNT_CLAYMORETRAP:
			if( src->id == bl->id ) break;
			if( bl->type == BL_SKILL ) {
				struct skill_unit *su = (struct skill_unit *)bl;

				if (!su)
					return 0;

				switch(su->group->unit_id) {
					case UNT_CLAYMORETRAP:
					case UNT_LANDMINE:
					case UNT_BLASTMINE:
					case UNT_SHOCKWAVE:
					case UNT_SANDMAN:
					case UNT_FLASHER:
					case UNT_FREEZINGTRAP:
					case UNT_FIRINGTRAP:
					case UNT_ICEBOUNDTRAP:
						clif_changetraplook(bl, UNT_USED_TRAPS);
						su->group->limit = DIFF_TICK(gettick(),su->group->tick) + 1500;
						su->group->unit_id = UNT_USED_TRAPS;
						break;
				}
			}
			[[fallthrough]];
		default: {
				int split_count = 0;

				if (skill_get_nk(sg->skill_id, NK_SPLASHSPLIT))
					split_count = max(1, map_foreachinallrange(skill_area_sub, src, skill_get_splash(sg->skill_id, sg->skill_lv), BL_CHAR, src, sg->skill_id, sg->skill_lv, tick, BCT_ENEMY, skill_area_sub_count));
				skill_attack(skill_get_type(sg->skill_id), ss, src, bl, sg->skill_id, sg->skill_lv, tick, split_count);
			}
			break;
	}
	return 1;
}

int skill_maelstrom_suction(struct block_list *bl, va_list ap)
{
	uint16 skill_id, skill_lv;
	struct skill_unit *unit;

	nullpo_ret(bl);

	skill_id = va_arg(ap,int);
	skill_lv = va_arg(ap,int);
	unit = (struct skill_unit *)bl;

	if( unit == nullptr || unit->group == nullptr )
		return 0;

	if( skill_get_inf2(skill_id, INF2_ISTRAP) )
		return 0;

	if( unit->group->skill_id == SC_MAELSTROM ) {
		struct block_list *src;

		if( (src = map_id2bl(unit->group->src_id)) ){
			int sp = unit->group->skill_lv * skill_lv;

			if( src->type == BL_PC )
				sp += ((TBL_PC*)src)->status.job_level / 5;
			status_heal(src, 0, sp/2, 1);
		}
	}

	return 0;
}

/**
 * Check cloaking condition
 * @param bl
 * @param sce
 * @return True if near wall; False otherwise
 */
bool skill_check_cloaking(struct block_list *bl, struct status_change_entry *sce)
{
	bool wall = true;

	if( (bl->type == BL_PC && battle_config.pc_cloak_check_type&1)
	||	(bl->type != BL_PC && battle_config.monster_cloak_check_type&1) )
	{	//Check for walls.
		static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
		static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
		int i;
		ARR_FIND( 0, 8, i, map_getcell(bl->m, bl->x+dx[i], bl->y+dy[i], CELL_CHKNOPASS) != 0 );
		if( i == 8 )
			wall = false;
	}

	if( sce ) {
		if( !wall ) {
			if( sce->val1 < 3 ) //End cloaking.
				status_change_end(bl, SC_CLOAKING);
			else if( sce->val4&1 ) { //Remove wall bonus
				sce->val4&=~1;
				status_calc_bl(bl, { SCB_SPEED });
			}
		} else {
			if( !(sce->val4&1) ) { //Add wall speed bonus
				sce->val4|=1;
				status_calc_bl(bl, { SCB_SPEED });
			}
		}
	}

	return wall;
}

/** Check Shadow Form on the target
 * @param bl: Target
 * @param damage: Damage amount
 * @param hit
 * @return true - in Shadow Form state; false - otherwise
 */
bool skill_check_shadowform(struct block_list *bl, int64 damage, int hit)
{
	status_change *sc;

	nullpo_retr(false,bl);

	if (!damage)
		return false;

	sc = status_get_sc(bl);

	if( sc && sc->getSCE(SC__SHADOWFORM) ) {
		struct block_list *src = map_id2bl(sc->getSCE(SC__SHADOWFORM)->val2);

		if( !src || src->m != bl->m ) { 
			status_change_end(bl, SC__SHADOWFORM);
			return false;
		}

		if( src && (status_isdead(*src) || !battle_check_target(bl,src,BCT_ENEMY)) ) {
			if( src->type == BL_PC )
				((TBL_PC*)src)->shadowform_id = 0;
			status_change_end(bl, SC__SHADOWFORM);
			return false;
		}

		status_damage(bl, src, damage, 0, clif_damage(*src, *src, gettick(), 500, 500, damage, hit, (hit > 1 ? DMG_MULTI_HIT : DMG_NORMAL), 0, false), 0, SC__SHADOWFORM);
		if( sc && sc->getSCE(SC__SHADOWFORM) && (--sc->getSCE(SC__SHADOWFORM)->val3) <= 0 ) {
			status_change_end(bl, SC__SHADOWFORM);
			if( src->type == BL_PC )
				((TBL_PC*)src)->shadowform_id = 0;
		}
		return true;
	}
	return false;
}

/**
 * Check camouflage condition
 * @param bl
 * @param sce
 * @return True if near wall; False otherwise
 * @TODO: Seems wrong
 */
bool skill_check_camouflage(struct block_list *bl, struct status_change_entry *sce)
{
	bool wall = true;

	if( bl->type == BL_PC ) { //Check for walls.
		static int dx[] = { 0, 1, 0, -1, -1,  1, 1, -1};
		static int dy[] = {-1, 0, 1,  0, -1, -1, 1,  1};
		int i;
		ARR_FIND( 0, 8, i, map_getcell(bl->m, bl->x+dx[i], bl->y+dy[i], CELL_CHKNOPASS) != 0 );
		if( i == 8 )
			wall = false;
	}

	if( sce ) {
		if( !wall && sce->val1 < 3 ) //End camouflage.
			status_change_end(bl, SC_CAMOUFLAGE);
		status_calc_bl(bl, { SCB_SPEED });
	}

	return wall;
}

/**
 * Process skill unit visibilty for single BL in area
 * @param bl
 * @param ap
 * @author [Cydh]
 **/
int skill_getareachar_skillunit_visibilty_sub(struct block_list *bl, va_list ap) {
	struct skill_unit *su = nullptr;
	struct block_list *src = nullptr;
	unsigned int party1 = 0;
	bool visible = true;

	nullpo_ret(bl);
	nullpo_ret((su = va_arg(ap, struct skill_unit*)));
	nullpo_ret((src = va_arg(ap, struct block_list*)));
	party1 = va_arg(ap, unsigned int);

	if (src != bl) {
		unsigned int party2 = status_get_party_id(bl);
		if (!party1 || !party2 || party1 != party2)
			visible = false;
	}

	clif_getareachar_skillunit(bl, su, SELF, visible);
	return 1;
}

/**
 * Check for skill unit visibilty in area on
 * - skill first placement
 * - skill moved (knocked back, moved dance)
 * @param su Skill unit
 * @param target Affected target for this visibility @see enum send_target
 * @author [Cydh]
 **/
void skill_getareachar_skillunit_visibilty(struct skill_unit *su, enum send_target target) {
	nullpo_retv(su);

	if (!su->hidden) // It's not hidden, just do this!
		clif_getareachar_skillunit(&su->bl, su, target, true);
	else {
		struct block_list *src = battle_get_master(&su->bl);
		map_foreachinallarea(skill_getareachar_skillunit_visibilty_sub, su->bl.m, su->bl.x-AREA_SIZE, su->bl.y-AREA_SIZE,
			su->bl.x+AREA_SIZE, su->bl.y+AREA_SIZE, BL_PC, su, src, status_get_party_id(src));
	}
}

/**
 * Check for skill unit visibilty on single BL on insight/spawn action
 * @param su Skill unit
 * @param bl Block list
 * @author [Cydh]
 **/
void skill_getareachar_skillunit_visibilty_single(struct skill_unit *su, struct block_list *bl) {
	bool visible = true;
	struct block_list *src = nullptr;

	nullpo_retv(bl);
	nullpo_retv(su);
	nullpo_retv((src = battle_get_master(&su->bl)));

	if (su->hidden && src != bl) {
		unsigned int party1 = status_get_party_id(src);
		unsigned int party2 = status_get_party_id(bl);
		if (!party1 || !party2 || party1 != party2)
			visible = false;
	}

	clif_getareachar_skillunit(bl, su, SELF, visible);
}

/**
 * Initialize new skill unit for skill unit group.
 * Overall, Skill Unit makes skill unit group which each group holds their cell datas (skill unit)
 * @param group Skill unit group
 * @param idx
 * @param x
 * @param y
 * @param val1
 * @param val2
 */
struct skill_unit *skill_initunit(std::shared_ptr<s_skill_unit_group> group, int idx, int x, int y, int val1, int val2, bool hidden)
{
	if (group == nullptr || group->unit == nullptr)
		return nullptr;

	skill_unit *unit = &group->unit[idx];

	if (unit == nullptr)
		return nullptr;

	if( map_getcell(map_id2bl(group->src_id)->m, x, y, CELL_CHKMAELSTROM) )
		return unit;

	if(!unit->alive)
		group->alive_count++;

	unit->bl.id = map_get_new_object_id();
	unit->bl.type = BL_SKILL;
	unit->bl.m = group->map;
	unit->bl.x = x;
	unit->bl.y = y;
	unit->group = group;
	unit->alive = 1;
	unit->val1 = val1;
	unit->val2 = val2;
	unit->hidden = hidden;

	// Stores new skill unit
	idb_put(skillunit_db, unit->bl.id, unit);
	map_addiddb(&unit->bl);
	if(map_addblock(&unit->bl))
		return nullptr;

	// Perform oninit actions
	switch (group->skill_id) {
		case WZ_ICEWALL:
			map_setgatcell(unit->bl.m,unit->bl.x,unit->bl.y,5);
			clif_changemapcell(0,unit->bl.m,unit->bl.x,unit->bl.y,5,AREA);
			skill_unitsetmapcell(unit,WZ_ICEWALL,group->skill_lv,CELL_ICEWALL,true);
			break;
		case SA_LANDPROTECTOR:
			skill_unitsetmapcell(unit,SA_LANDPROTECTOR,group->skill_lv,CELL_LANDPROTECTOR,true);
			break;
#ifndef RENEWAL
		case HP_BASILICA:
			skill_unitsetmapcell(unit,HP_BASILICA,group->skill_lv,CELL_BASILICA,true);
			break;
#endif
		case SC_MAELSTROM:
			skill_unitsetmapcell(unit,SC_MAELSTROM,group->skill_lv,CELL_MAELSTROM,true);
			break;
		default:
			if (group->state.song_dance&0x1) //Check for dissonance.
				skill_dance_overlap(unit, 1);
			break;
	}

	skill_getareachar_skillunit_visibilty(unit, AREA);
	return unit;
}

/**
 * Remove unit
 * @param unit
 */
int skill_delunit(struct skill_unit* unit)
{
	nullpo_ret(unit);

	if( !unit->alive )
		return 0;

	unit->alive = 0;

	std::shared_ptr<s_skill_unit_group> group = unit->group;

	if (group == nullptr)
		return 0;

	if( group->state.song_dance&0x1 ) //Cancel dissonance effect.
		skill_dance_overlap(unit, 0);

	// invoke onout event
	if( !unit->range )
		map_foreachincell(skill_unit_effect,unit->bl.m,unit->bl.x,unit->bl.y,group->bl_flag,&unit->bl,gettick(),4);

	// perform ondelete actions
	switch (group->skill_id) {
		case HT_ANKLESNARE:
		case SC_ESCAPE:
			{
				struct block_list* target = map_id2bl(group->val2);

				if( target )
					status_change_end(target, SC_ANKLE);
			}
			break;
		case WZ_ICEWALL:
			map_setgatcell(unit->bl.m,unit->bl.x,unit->bl.y,unit->val2);
			clif_changemapcell(0,unit->bl.m,unit->bl.x,unit->bl.y,unit->val2,ALL_SAMEMAP); // hack to avoid clientside cell bug
			skill_unitsetmapcell(unit,WZ_ICEWALL,group->skill_lv,CELL_ICEWALL,false);
			break;
		case SA_LANDPROTECTOR:
			skill_unitsetmapcell(unit,SA_LANDPROTECTOR,group->skill_lv,CELL_LANDPROTECTOR,false);
			break;
#ifndef RENEWAL
		case HP_BASILICA:
			skill_unitsetmapcell(unit,HP_BASILICA,group->skill_lv,CELL_BASILICA,false);
			break;
#endif
		case RA_ELECTRICSHOCKER: {
				struct block_list* target = map_id2bl(group->val2);

				if( target )
					status_change_end(target, SC_ELECTRICSHOCKER);
			}
			break;
		case SC_MAELSTROM:
			skill_unitsetmapcell(unit,SC_MAELSTROM,group->skill_lv,CELL_MAELSTROM,false);
			break;
		case SC_MANHOLE: // Note : Removing the unit don't remove the status (official info)
			if( group->val2 ) { // Someone Traped
				status_change *tsc = status_get_sc( map_id2bl(group->val2));
				if( tsc && tsc->getSCE(SC__MANHOLE) )
					tsc->getSCE(SC__MANHOLE)->val4 = 0; // Remove the Unit ID
			}
			break;
	}

	clif_skill_delunit( *unit );

	unit->group=nullptr;
	map_delblock(&unit->bl); // don't free yet
	map_deliddb(&unit->bl);
	idb_remove(skillunit_db, unit->bl.id);
	if(--group->alive_count==0)
		skill_delunitgroup(group);

	return 0;
}


static std::unordered_map<int, std::shared_ptr<s_skill_unit_group>> skillunit_group_db; /// Skill unit group DB. Key int group_id -> struct s_skill_unit_group*

/// Returns the target s_skill_unit_group or nullptr if not found.
std::shared_ptr<s_skill_unit_group> skill_id2group(int group_id) {
	return util::umap_find(skillunit_group_db, group_id);
}

static int skill_unit_group_newid = MAX_SKILL; /// Skill Unit Group ID

/**
 * Returns a new group_id that isn't being used in skillunit_group_db.
 * Fatal error if nothing is available.
 */
static int skill_get_new_group_id(void)
{
	if( skill_unit_group_newid >= MAX_SKILL && skill_id2group(skill_unit_group_newid) == nullptr )
		return skill_unit_group_newid++;// available
	{// find next id
		int base_id = skill_unit_group_newid;
		while( base_id != ++skill_unit_group_newid )
		{
			if( skill_unit_group_newid < MAX_SKILL )
				skill_unit_group_newid = MAX_SKILL;
			if( skill_id2group(skill_unit_group_newid) == nullptr )
				return skill_unit_group_newid++;// available
		}
		// full loop, nothing available
		ShowFatalError("skill_get_new_group_id: All ids are taken. Exiting...");
		exit(1);
	}
}

/**
 * Initialize skill unit group called while setting new unit (skill unit/ground skill) in skill_unitsetting()
 * @param src Object that cast the skill
 * @param count How many 'cells' used that needed. Related with skill layout
 * @param skill_id ID of used skill
 * @param skill_lv Skill level of used skill
 * @param unit_id Unit ID (see skill.hpp::e_skill_unit_id)
 * @param limit Lifetime for skill unit, uses skill_get_time(skill_id, skill_lv)
 * @param interval Time interval
 * @return s_skill_unit_group
 */
std::shared_ptr<s_skill_unit_group> skill_initunitgroup(struct block_list* src, int count, uint16 skill_id, uint16 skill_lv, int unit_id, t_tick limit, int interval)
{
	nullpo_retr(nullptr, src);

	unit_data *ud = unit_bl2ud(src);

	nullpo_retr(nullptr, ud);

	if (skill_id == 0 || skill_lv == 0)
		return 0;

	auto group = std::make_shared<s_skill_unit_group>();

	group->src_id     = src->id;
	group->party_id   = status_get_party_id(src);
	group->guild_id   = status_get_guild_id(src);
	group->bg_id      = bg_team_get_id(src);
	group->group_id   = skill_get_new_group_id();
	group->link_group_id = 0;
	group->unit       = (skill_unit *)aCalloc(count, sizeof(skill_unit));
	group->unit_count = count;
	group->alive_count = 0;
	group->val1       = 0;
	group->val2       = 0;
	group->val3       = 0;
	group->skill_id   = skill_id;
	group->skill_lv   = skill_lv;
	group->unit_id    = unit_id;
	group->map        = src->m;
	group->limit      = limit;
	group->interval   = interval;
	group->tick       = gettick();
	group->valstr     = nullptr;

	ud->skillunits.push_back(group);

	// Stores this new group
	skillunit_group_db.insert({ group->group_id, group });

	return group;
}

/**
 * Remove skill unit group
 * @param group
 * @param file
 * @param line
 * @param *func
 */
int skill_delunitgroup_(std::shared_ptr<s_skill_unit_group> group, const char* file, int line, const char* func)
{
	struct block_list* src;
	struct unit_data *ud;
	short i;
	int link_group_id;

	if( group == nullptr ) {
		ShowDebug("skill_delunitgroup: group is nullptr (source=%s:%d, %s)! Please report this! (#3504)\n", file, line, func);
		return 0;
	}

	src = map_id2bl(group->src_id);
	ud = unit_bl2ud(src);
	if (!src || !ud) {
		ShowError("skill_delunitgroup: Group's source not found! (src_id: %d skill_id: %d)\n", group->src_id, group->skill_id);
		return 0;
	}

	if( !status_isdead(*src) && ((TBL_PC*)src)->state.warping && !((TBL_PC*)src)->state.changemap ) {
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
			case NC_NEUTRALBARRIER:
			case NC_STEALTHFIELD:
				skill_usave_add(((TBL_PC*)src), group->skill_id, group->skill_lv);
				break;
		}
	}

	if (skill_get_unit_flag_(group->skill_id, { UF_DANCE, UF_SONG, UF_ENSEMBLE })) {
		status_change* sc = status_get_sc(src);
		if (sc && sc->getSCE(SC_DANCING)) {
			sc->getSCE(SC_DANCING)->val2 = 0 ; //This prevents status_change_end attempting to redelete the group. [Skotlex]
			status_change_end(src, SC_DANCING);
		}
	}

	// End SC from the master when the skill group is deleted
	i = SC_NONE;
	switch (group->unit_id) {
		case UNT_GOSPEL:	i = SC_GOSPEL;		break;
#ifndef RENEWAL
		case UNT_BASILICA:	i = SC_BASILICA;	break;
#endif
	}
	if (i != SC_NONE) {
		status_change *sc = status_get_sc(src);
		if (sc && sc->getSCE(i)) {
			sc->getSCE(i)->val3 = 0; //Remove reference to this group. [Skotlex]
			status_change_end(src, (sc_type)i);
		}
	}

	switch( group->skill_id ) {
		case PF_SPIDERWEB:
		{
			struct block_list* target = map_id2bl(group->val2);
			status_change *sc;
			bool removed = true;
			//Clear group id from status change
			if (target && (sc = status_get_sc(target)) != nullptr && sc->getSCE(SC_SPIDERWEB)) {
				if (sc->getSCE(SC_SPIDERWEB)->val2 == group->group_id)
					sc->getSCE(SC_SPIDERWEB)->val2 = 0;
				else if (sc->getSCE(SC_SPIDERWEB)->val3 == group->group_id)
					sc->getSCE(SC_SPIDERWEB)->val3 = 0;
				else if (sc->getSCE(SC_SPIDERWEB)->val4 == group->group_id)
					sc->getSCE(SC_SPIDERWEB)->val4 = 0;
				else //Group was already removed in status_change_end, don't call it again!
					removed = false;

				//The last group was cleared, end status change
				if(removed && sc->getSCE(SC_SPIDERWEB)->val2 == 0 && sc->getSCE(SC_SPIDERWEB)->val3 == 0 && sc->getSCE(SC_SPIDERWEB)->val4 == 0)
					status_change_end(target, SC_SPIDERWEB);
			}
		}
			break;
		case SG_SUN_WARM:
		case SG_MOON_WARM:
		case SG_STAR_WARM: {
				status_change *sc = status_get_sc(src);

				if (sc && sc->getSCE(SC_WARM)) {
					sc->getSCE(SC_WARM)->val4 = 0;
					status_change_end(src, SC_WARM);
				}
			}
			break;
		case LG_BANDING: {
				status_change *sc = status_get_sc(src);

				if (sc && sc->getSCE(SC_BANDING)) {
					sc->getSCE(SC_BANDING)->val4 = 0;
					status_change_end(src, SC_BANDING);
				}
			}
			break;
		case NC_NEUTRALBARRIER:
			{
				status_change *sc = nullptr;
				if( (sc = status_get_sc(src)) != nullptr ) {
					if ( sc->getSCE(SC_NEUTRALBARRIER_MASTER) )
					{
						sc->getSCE(SC_NEUTRALBARRIER_MASTER)->val2 = 0;
						status_change_end(src,SC_NEUTRALBARRIER_MASTER);
					}
					status_change_end(src,SC_NEUTRALBARRIER);
				}
			}
			break;
		case NC_STEALTHFIELD:
			{
				status_change *sc = nullptr;
				if( (sc = status_get_sc(src)) != nullptr && sc->getSCE(SC_STEALTHFIELD_MASTER) ) {
					sc->getSCE(SC_STEALTHFIELD_MASTER)->val2 = 0;
					status_change_end(src,SC_STEALTHFIELD_MASTER);
				}
			}
			break;
	}

	if (src->type==BL_PC && group->state.ammo_consume)
		battle_consume_ammo((TBL_PC*)src, group->skill_id, group->skill_lv);

	group->alive_count=0;

	// remove all unit cells
	if(group->unit != nullptr)
		for( int j = 0; j < group->unit_count; j++ )
			skill_delunit(&group->unit[j]);

	// clear Talkie-box string
	if( group->valstr != nullptr ) {
		aFree(group->valstr);
		group->valstr = nullptr;
	}

	link_group_id = group->link_group_id;

	if (skillunit_group_db.erase(group->group_id) != 1)
		ShowError("skill_delunitgroup: Group not found! (src_id: %d skill_id: %d)\n", group->src_id, group->skill_id);

	util::vector_erase_if_exists(ud->skillunits, group);

	if(link_group_id) {
		std::shared_ptr<s_skill_unit_group> group_cur = skill_id2group(link_group_id);

		if(group_cur)
			skill_delunitgroup(group_cur);
	}

	return 1;
}

/**
 * Clear all Skill Unit Group from an Object, example usage when player logged off or dead
 * @param src
 */
void skill_clear_unitgroup(struct block_list *src)
{
	nullpo_retv(src);

	unit_data *ud = unit_bl2ud(src);

	nullpo_retv(ud);

	// The after loop statement might look stupid, but this prevents iteration problems, if an entry was deleted
	for (auto it = ud->skillunits.begin(); it != ud->skillunits.end(); it = ud->skillunits.begin()) {
		skill_delunitgroup(*it);
	}
}

/**
 * Search tickset for skill unit in skill unit group
 * @param bl Block List for skill_unit
 * @param group Skill unit group
 * @param tick
 * @return skill_unit_group_tickset if found
 */
struct skill_unit_group_tickset *skill_unitgrouptickset_search(struct block_list *bl, std::shared_ptr<s_skill_unit_group> group, t_tick tick)
{
	int i, j = -1, s, id;
	struct unit_data *ud;
	struct skill_unit_group_tickset *set;

	nullpo_ret(bl);
	if (group->interval == -1)
		return nullptr;

	ud = unit_bl2ud(bl);
	if (!ud)
		return nullptr;

	set = ud->skillunittick;

	if (skill_get_unit_flag(group->skill_id, UF_NOOVERLAP))
		id = s = group->skill_id;
	else
		id = s = group->group_id;

	for (i=0; i<MAX_SKILLUNITGROUPTICKSET; i++) {
		int k = (i+s) % MAX_SKILLUNITGROUPTICKSET;
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
 * Check for validity skill unit that triggered by skill_unit_timer_sub
 * And trigger skill_unit_onplace_timer for object that maybe stands there (catched object is *bl)
 *------------------------------------------*/
int skill_unit_timer_sub_onplace(struct block_list* bl, va_list ap)
{
	struct skill_unit* unit = va_arg(ap,struct skill_unit *);
	t_tick tick = va_arg(ap,t_tick);

	nullpo_ret(unit);

	if( !unit->alive || bl->prev == nullptr )
		return 0;

	std::shared_ptr<s_skill_unit_group> group = unit->group;

	if (group == nullptr)
		return 0;

	std::shared_ptr<s_skill_db> skill = skill_db.find(group->skill_id);

	if( !(skill->inf2[INF2_ISSONG] || skill->inf2[INF2_ISTRAP]) && !skill->inf2[INF2_IGNORELANDPROTECTOR] && group->skill_id != NC_NEUTRALBARRIER && (battle_config.land_protector_behavior ? map_getcell(bl->m, bl->x, bl->y, CELL_CHKLANDPROTECTOR) : map_getcell(unit->bl.m, unit->bl.x, unit->bl.y, CELL_CHKLANDPROTECTOR)) )
		return 0; //AoE skills are ineffective. [Skotlex]

#ifdef RENEWAL
	// Ankle Snare and Skid Trap can no longer trap bosses in renewal
	if ((group->unit_id == UNT_ANKLESNARE || group->unit_id == UNT_SKIDTRAP) && status_bl_has_mode(bl, MD_STATUSIMMUNE))
		return 0;
#endif

	if( battle_check_target(&unit->bl,bl,group->target_flag) <= 0 )
		return 0;

	skill_unit_onplace_timer(unit,bl,tick);
	return 1;
}

/**
 * @see DBApply
 * Sub function of skill_unit_timer for executing each skill unit from skillunit_db
 */
static int skill_unit_timer_sub(DBKey key, DBData *data, va_list ap)
{
	struct skill_unit* unit = (struct skill_unit*)db_data2ptr(data);
	t_tick tick = va_arg(ap,t_tick);
	bool dissonance;
	struct block_list* bl = &unit->bl;

	nullpo_ret(unit);

	if( !unit->alive )
		return 0;

	std::shared_ptr<s_skill_unit_group> group = unit->group;

	if (group == nullptr)
		return 0;

	// Check for expiration
	if( !group->state.guildaura && (DIFF_TICK(tick,group->tick) >= group->limit || DIFF_TICK(tick,group->tick) >= unit->limit) )
	{// skill unit expired (inlined from skill_unit_onlimit())
		switch( group->unit_id ) {
			case UNT_ICEWALL:
				unit->val1 -= 50; // icewall loses 50 hp every second
				group->limit = DIFF_TICK(tick + group->interval,group->tick);
				unit->limit = DIFF_TICK(tick + group->interval,group->tick);
				if( unit->val1 <= 0 )
					skill_delunit(unit);
				break;
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
				if (group->val2 > 0) { //Used Trap doesn't return back to item
					skill_delunit(unit);
					break;
				}
				[[fallthrough]];
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
				if( unit->val1 > 0 && (src = map_id2bl(group->src_id)) != nullptr && src->type == BL_PC )
				{ // revert unit back into a trap
					struct item item_tmp;
					memset(&item_tmp,0,sizeof(item_tmp));
					item_tmp.nameid = group->item_id?group->item_id:ITEMID_TRAP;
					item_tmp.identify = 1;
					map_addflooritem(&item_tmp,1,bl->m,bl->x,bl->y,0,0,0,4,0);
				}
				skill_delunit(unit);
			}
				break;

			case UNT_WARP_ACTIVE:
				// warp portal opens (morph to a UNT_WARP_WAITING cell)
				group->unit_id = skill_get_unit_id2(group->skill_id); // UNT_WARP_WAITING
				clif_changelook(&unit->bl, LOOK_BASE, group->unit_id);
				// restart timers
				group->limit = skill_get_time(group->skill_id,group->skill_lv);
				unit->limit = skill_get_time(group->skill_id,group->skill_lv);
				// apply effect to all units standing on it
				map_foreachincell(skill_unit_effect,unit->bl.m,unit->bl.x,unit->bl.y,group->bl_flag,&unit->bl,gettick(),1);
				break;

			case UNT_CALLFAMILY:
			{
				map_session_data *sd = nullptr;
				if(group->val1) {
					sd = map_charid2sd(group->val1);
					group->val1 = 0;
					if (sd && !map_getmapflag(sd->bl.m, MF_NOWARP) && pc_job_can_entermap((enum e_job)sd->status.class_, unit->bl.m, pc_get_group_level(sd)))
						pc_setpos(sd,map_id2index(unit->bl.m),unit->bl.x,unit->bl.y,CLR_TELEPORT);
				}
				if(group->val2) {
					sd = map_charid2sd(group->val2);
					group->val2 = 0;
					if (sd && !map_getmapflag(sd->bl.m, MF_NOWARP) && pc_job_can_entermap((enum e_job)sd->status.class_, unit->bl.m, pc_get_group_level(sd)))
						pc_setpos(sd,map_id2index(unit->bl.m),unit->bl.x,unit->bl.y,CLR_TELEPORT);
				}
				skill_delunit(unit);
			}
				break;

			case UNT_REVERBERATION:
			case UNT_NETHERWORLD:
				if( unit->val1 <= 0 ) { // If it was deactivated.
					skill_delunit(unit);
					break;
				}
				clif_changetraplook(bl,UNT_USED_TRAPS);
				if (group->unit_id == UNT_REVERBERATION)
					map_foreachinrange(skill_trap_splash, bl, skill_get_splash(group->skill_id, group->skill_lv), group->bl_flag, bl, tick);
				group->limit = DIFF_TICK(tick,group->tick) + 1000;
				unit->limit = DIFF_TICK(tick,group->tick) + 1000;
				group->unit_id = UNT_USED_TRAPS;
				break;

			case UNT_FEINTBOMB: {
				struct block_list *src = map_id2bl(group->src_id);

				if (src)
					map_foreachinrange(skill_area_sub, &unit->bl, unit->range, BL_CHAR|BL_SKILL, src, group->skill_id, group->skill_lv, tick, BCT_ENEMY|SD_ANIMATION|5, skill_castend_damage_id);
				skill_delunit(unit);
			}
				break;

			case UNT_BANDING:
			{
				struct block_list *src = map_id2bl(group->src_id);
				status_change *sc;
				if( !src || (sc = status_get_sc(src)) == nullptr || !sc->getSCE(SC_BANDING) ) {
					skill_delunit(unit);
					break;
				}
				// This unit isn't removed while SC_BANDING is active.
				group->limit = DIFF_TICK(tick+group->interval,group->tick);
				unit->limit = DIFF_TICK(tick+group->interval,group->tick);
			}
				break;

			case UNT_B_TRAP:
				{
					struct block_list* src;
					if (group->item_id && unit->val2 <= 0 && (src = map_id2bl(group->src_id)) && src->type == BL_PC) {
						struct item item_tmp;
						memset(&item_tmp, 0, sizeof(item_tmp));
						item_tmp.nameid = group->item_id;
						item_tmp.identify = 1;
						map_addflooritem(&item_tmp, 1, bl->m, bl->x, bl->y, 0, 0, 0, 4, 0);
					}
					skill_delunit(unit);
				}
				break;

			default:
				if (group->val2 == 1 && (group->skill_id == WZ_METEOR || group->skill_id == SU_CN_METEOR || group->skill_id == SU_CN_METEOR2 || 
					group->skill_id == AG_VIOLENT_QUAKE_ATK || group->skill_id == AG_ALL_BLOOM_ATK || group->skill_id == AG_ALL_BLOOM_ATK2 || group->skill_id == NPC_RAINOFMETEOR ||
					group->skill_id == HN_METEOR_STORM_BUSTER)) {
					// Deal damage before expiration
					break;
				}
				skill_delunit(unit);
				break;
		}
	} else {// skill unit is still active
		switch( group->unit_id ) {
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
			case UNT_B_TRAP:
				if( unit->val1 <= 0 ) {
					if( group->unit_id == UNT_ANKLESNARE && group->val2 > 0 )
						skill_delunit(unit);
					else {
						clif_changetraplook(bl, group->unit_id == UNT_LANDMINE ? UNT_FIREPILLAR_ACTIVE : UNT_USED_TRAPS);
						group->limit = DIFF_TICK(tick, group->tick) + 1500;
						group->unit_id = UNT_USED_TRAPS;
					}
				}
				break;
			case UNT_REVERBERATION:
			case UNT_NETHERWORLD:
				if (unit->val1 <= 0) {
					clif_changetraplook(bl,UNT_USED_TRAPS);
					if (group->unit_id == UNT_REVERBERATION)
						map_foreachinrange(skill_trap_splash, bl, skill_get_splash(group->skill_id, group->skill_lv), group->bl_flag, bl, tick);
					group->limit = DIFF_TICK(tick,group->tick) + 1000;
					unit->limit = DIFF_TICK(tick,group->tick) + 1000;
					group->unit_id = UNT_USED_TRAPS;
				}
				break;
			case UNT_WALLOFTHORN:
				if (group->val3 < 0) { // Remove if attacked by fire element, turned to Fire Wall
					skill_delunitgroup(group);
					break;
				}
				if (unit->val1 <= 0 || unit->val2 <= 0) // Remove the unit only if no HP or hit limit is reached
					skill_delunit(unit);
				break;
			case UNT_SANCTUARY:
				if (group->val1 <= 0) {
					skill_delunitgroup(group);
				}
				break;
			default:
				if (group->skill_id == WZ_METEOR || group->skill_id == SU_CN_METEOR || group->skill_id == SU_CN_METEOR2 || 
					group->skill_id == AG_VIOLENT_QUAKE_ATK || group->skill_id == AG_ALL_BLOOM_ATK || group->skill_id == AG_ALL_BLOOM_ATK2 || group->skill_id == NPC_RAINOFMETEOR ||
					group->skill_id == HN_METEOR_STORM_BUSTER) {
					if (group->val2 == 0 && (DIFF_TICK(tick, group->tick) >= group->limit - group->interval || DIFF_TICK(tick, group->tick) >= unit->limit - group->interval)) {
						// Unit will expire the next interval, start dropping Meteor
						block_list *src = map_id2bl(group->src_id);

						if (src != nullptr) {
							if (group->skill_id == AG_VIOLENT_QUAKE_ATK || group->skill_id == AG_ALL_BLOOM_ATK || group->skill_id == AG_ALL_BLOOM_ATK2)
								clif_skill_poseffect(src, group->skill_id, -1, bl->x, bl->y, tick); // Don't yell a blank skill name.
							else
								clif_skill_poseffect(src, group->skill_id, group->skill_lv, bl->x, bl->y, tick);
							group->val2 = 1;
						}
					}
					// No damage until expiration
					return 0;
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
		map_foreachinrange(skill_unit_timer_sub_onplace, bl, unit->range, group->bl_flag, bl,tick);

		if(unit->range == -1) //Unit disabled, but it should not be deleted yet.
			group->unit_id = UNT_USED_TRAPS;
		else if( group->unit_id == UNT_TATAMIGAESHI ) {
			unit->range = -1; //Disable processed cell.
			if (--group->val1 <= 0) { // number of live cells
				//All tiles were processed, disable skill.
				group->target_flag=BCT_NOONE;
				group->bl_flag= BL_NUL;
			}
		}
		else if (group->skill_id == WZ_METEOR || group->skill_id == SU_CN_METEOR || group->skill_id == SU_CN_METEOR2 || 
			group->skill_id == AG_VIOLENT_QUAKE_ATK || group->skill_id == AG_ALL_BLOOM_ATK || group->skill_id == AG_ALL_BLOOM_ATK2 || group->skill_id == NPC_RAINOFMETEOR ||
			group->skill_id == HN_METEOR_STORM_BUSTER ||
			((group->skill_id == CR_GRANDCROSS || group->skill_id == NPC_GRANDDARKNESS) && unit->val1 <= 0)) {
			skill_delunit(unit);
			return 0;
		}
	}

	if( dissonance )
		skill_dance_switch(unit, 1);

	return 0;
}

/*==========================================
 * Executes on all skill units every SKILLUNITTIMER_INTERVAL miliseconds.
 *------------------------------------------*/
TIMER_FUNC(skill_unit_timer){
	map_freeblock_lock();

	skillunit_db->foreach(skillunit_db, skill_unit_timer_sub, tick);

	map_freeblock_unlock();
	return 0;
}

static std::vector<int16> skill_unit_cell; // Temporary storage for tracking skill unit skill IDs as players move in/out of them

/*==========================================
 * flag :
 *	1 : store that skill_unit in array
 *	2 : clear that skill_unit
 *	4 : call_on_left
 *------------------------------------------*/
int skill_unit_move_sub(struct block_list* bl, va_list ap)
{
	struct skill_unit* unit = (struct skill_unit *)bl;
	struct block_list* target = va_arg(ap,struct block_list*);
	t_tick tick = va_arg(ap,t_tick);
	int flag = va_arg(ap,int);
	bool dissonance;
	uint16 skill_id;

	nullpo_ret(unit);
	nullpo_ret(target);

	if( !unit->alive || target->prev == nullptr )
		return 0;

	std::shared_ptr<s_skill_unit_group> group = unit->group;

	if (group == nullptr)
		return 0;

	if( flag&1 && ( group->skill_id == PF_SPIDERWEB || group->skill_id == GN_THORNS_TRAP ) )
		return 0; // Fiberlock is never supposed to trigger on skill_unit_move. [Inkfish]

	dissonance = skill_dance_switch(unit, 0);

	//Necessary in case the group is deleted after calling on_place/on_out [Skotlex]
	skill_id = group->skill_id;

	if( group->interval != -1 && !skill_get_unit_flag(skill_id, UF_DUALMODE) && skill_id != BD_LULLABY ) //Lullaby is the exception, bugreport:411
	{	//Non-dualmode unit skills with a timer don't trigger when walking, so just return
		if( dissonance ) {
			skill_dance_switch(unit, 1);
			skill_unit_onleft(skill_unit_onout(unit,target,tick),target,tick); //we placed a dissonance, let's update
		}
		return 0;
	}

	//Target-type check.
	if( !(group->bl_flag&target->type && battle_check_target(&unit->bl,target,group->target_flag) > 0) ) {
		if( group->src_id == target->id && group->state.song_dance&0x2 ) { //Ensemble check to see if they went out/in of the area [Skotlex]
			if( flag&1 ) {
				if( flag&2 ) { //Clear this skill id.
					util::vector_erase_if_exists(skill_unit_cell, skill_id);
				}
			} else {
				if( flag&2 ) { //Store this skill id.
					skill_unit_cell.push_back(skill_id);
				}
			}

			if( flag&4 )
				skill_unit_onleft(skill_id,target,tick);
		}

		if( dissonance )
			skill_dance_switch(unit, 1);

		return 0;
	} else {
		if( flag&1 ) {
			int result = skill_unit_onplace(unit,target,tick);

			if( flag&2 && result > 0 ) { //Clear skill ids we have stored in onout.
				util::vector_erase_if_exists(skill_unit_cell, result);
			}
		} else {
			int result = skill_unit_onout(unit,target,tick);

			if( flag&2 && result > 0 ) { //Store this unit id.
				skill_unit_cell.push_back(skill_id);
			}
		}

		//TODO: Normally, this is dangerous since the unit and group could be freed
		//inside the onout/onplace functions. Currently it is safe because we know song/dance
		//cells do not get deleted within them. [Skotlex]
		if( dissonance )
			skill_dance_switch(unit, 1);

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
int skill_unit_move(struct block_list *bl, t_tick tick, int flag)
{
	nullpo_ret(bl);

	if( bl->prev == nullptr )
		return 0;

	if( flag&2 && !(flag&1) ) //Onout, clear data
		skill_unit_cell.clear();

	map_foreachincell(skill_unit_move_sub,bl->m,bl->x,bl->y,BL_SKILL,bl,tick,flag);

	if( flag&2 && flag&1 ) { //Onplace, check any skill units you have left.
		for (const auto &it : skill_unit_cell) {
			skill_unit_onleft(it, bl, tick);
		}
	}

	return 0;
}

/*==========================================
 * Moves skill unit to map m with coordinates x & y (example when knocked back)
 * @param bl Skill unit
 * @param m Map
 * @param dx
 * @param dy
 *------------------------------------------*/
void skill_unit_move_unit(struct block_list *bl, int dx, int dy) {
	t_tick tick = gettick();
	struct skill_unit *su;

	if (bl->type != BL_SKILL)
		return;
	if (!(su = (struct skill_unit *)bl))
		return;
	if (!su->alive)
		return;

	if (su->group && skill_get_unit_flag(su->group->skill_id, UF_ENSEMBLE))
		return; //Ensembles may not be moved around.

	if (!bl->prev) {
		bl->x = dx;
		bl->y = dy;
		return;
	}

	map_moveblock(bl, dx, dy, tick);
	map_foreachincell(skill_unit_effect,bl->m,bl->x,bl->y,su->group->bl_flag,bl,tick,1);
	skill_getareachar_skillunit_visibilty(su, AREA);
	return;
}

/**
 * Moves skill unit group to map m with coordinates x & y (example when knocked back)
 * @param group Skill Group
 * @param m Map
 * @param dx
 * @param dy
 */
void skill_unit_move_unit_group(std::shared_ptr<s_skill_unit_group> group, int16 m, int16 dx, int16 dy)
{
	int i, j;
	t_tick tick = gettick();
	int *m_flag;
	struct skill_unit *unit1;
	struct skill_unit *unit2;

	if (group == nullptr)
		return;

	if (group->unit_count <= 0)
		return;

	if (group->unit == nullptr)
		return;

	if (skill_get_unit_flag(group->skill_id, UF_ENSEMBLE))
		return; //Ensembles may not be moved around.

	m_flag = (int *) aCalloc(group->unit_count, sizeof(int));
	//    m_flag
	//		0: Neither of the following (skill_unit_onplace & skill_unit_onout are needed)
	//		1: Unit will move to a slot that had another unit of the same group (skill_unit_onplace not needed)
	//		2: Another unit from same group will end up positioned on this unit (skill_unit_onout not needed)
	//		3: Both 1+2.
	for(i = 0; i < group->unit_count; i++) {
		unit1 =& group->unit[i];
		if (!unit1->alive || unit1->bl.m != m)
			continue;
		for(j = 0; j < group->unit_count; j++) {
			unit2 = &group->unit[j];
			if (!unit2->alive)
				continue;
			if (unit1->bl.x+dx == unit2->bl.x && unit1->bl.y+dy == unit2->bl.y)
				m_flag[i] |= 0x1;
			if (unit1->bl.x-dx == unit2->bl.x && unit1->bl.y-dy == unit2->bl.y)
				m_flag[i] |= 0x2;
		}
	}

	j = 0;

	for (i = 0; i < group->unit_count; i++) {
		unit1 = &group->unit[i];
		if (!unit1->alive)
			continue;
		if (!(m_flag[i]&0x2)) {
			if (group->state.song_dance&0x1) //Cancel dissonance effect.
				skill_dance_overlap(unit1, 0);
			map_foreachincell(skill_unit_effect,unit1->bl.m,unit1->bl.x,unit1->bl.y,group->bl_flag,&unit1->bl,tick,4);
		}
		//Move Cell using "smart" criteria (avoid useless moving around)
		switch(m_flag[i]) {
			case 0:
			//Cell moves independently, safely move it.
				map_foreachinmovearea(clif_outsight, &unit1->bl, AREA_SIZE, dx, dy, BL_PC, &unit1->bl);
				map_moveblock(&unit1->bl, unit1->bl.x+dx, unit1->bl.y+dy, tick);
				break;
			case 1:
			//Cell moves unto another cell, look for a replacement cell that won't collide
			//and has no cell moving into it (flag == 2)
				for(; j < group->unit_count; j++) {
					int dx2, dy2;
					if(m_flag[j] != 2 || !group->unit[j].alive)
						continue;
					//Move to where this cell would had moved.
					unit2 = &group->unit[j];
					dx2 = unit2->bl.x + dx - unit1->bl.x;
					dy2 = unit2->bl.y + dy - unit1->bl.y;
					map_foreachinmovearea(clif_outsight, &unit1->bl, AREA_SIZE, dx2, dy2, BL_PC, &unit1->bl);
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
			skill_getareachar_skillunit_visibilty(unit1, AREA);
			map_foreachincell(skill_unit_effect,unit1->bl.m,unit1->bl.x,unit1->bl.y,group->bl_flag,&unit1->bl,tick,1);
		}
	}

	aFree(m_flag);
}

/**
 * Checking product requirement in player's inventory.
 * Checking if player has the item or not, the amount, and the weight limit.
 * @param sd Player
 * @param nameid Product requested
 * @param trigger Trigger criteria to match will 'ItemLv'
 * @param qty Amount of item will be created
 * @return 0 If failed or Index+1 of item found on skill_produce_db[]
 */
short skill_can_produce_mix(map_session_data *sd, t_itemid nameid, int trigger, int qty)
{
	nullpo_ret(sd);

	if (!item_db.exists(nameid))
		return 0;

	short i, j;

	for (i = 0; i < MAX_SKILL_PRODUCE_DB; i++) {
		if (skill_produce_db[i].nameid == nameid) {
			if ((j = skill_produce_db[i].req_skill) > 0 &&
				pc_checkskill(sd,j) < skill_produce_db[i].req_skill_lv)
				continue; // must iterate again to check other skills that produce it. [malufett]
			if (j > 0 && sd->menuskill_id > 0 && sd->menuskill_id != j)
				continue; // special case
			break;
		}
	}

	if (nameid == ITEMID_HOMUNCULUS_SUPPLEMENT) { // Temporary check since the produce_db specifically wants the Pharmacy skill to use
		if (pc_checkskill(sd, AM_BIOETHICS) == 0)
			return 0;
	}

	if (i >= MAX_SKILL_PRODUCE_DB)
		return 0;

	// Cannot carry the produced stuff
	if (pc_checkadditem(sd, nameid, qty) == CHKADDITEM_OVERAMOUNT)
		return 0;

	// Matching the requested produce list
	if (trigger >= 0) {
		if (trigger > 20) { // Non-weapon, non-food item (itemlv must match)
			if (skill_produce_db[i].itemlv != trigger)
				return 0;
		} else if (trigger > 10) { // Food (any item level between 10 and 20 will do)
			if (skill_produce_db[i].itemlv <= 10 || skill_produce_db[i].itemlv > 20)
				return 0;
		} else { // Weapon (itemlv must be higher or equal)
			if (skill_produce_db[i].itemlv > trigger)
				return 0;
		}
	}

	// Check on player's inventory
	for (j = 0; j < MAX_PRODUCE_RESOURCE; j++) {
		t_itemid nameid_produce;

		if (!(nameid_produce = skill_produce_db[i].mat_id[j]))
			continue;
		if (skill_produce_db[i].mat_amount[j] == 0) {
			if (pc_search_inventory(sd,nameid_produce) < 0)
				return 0;
		} else {
			unsigned short idx, amt;

			for (idx = 0, amt = 0; idx < MAX_INVENTORY; idx++)
				if (sd->inventory.u.items_inventory[idx].nameid == nameid_produce)
					amt += sd->inventory.u.items_inventory[idx].amount;
			if (amt < qty * skill_produce_db[i].mat_amount[j])
				return 0;
		}
	}
	return i + 1;
}

/**
 * Attempt to produce an item
 * @param sd Player
 * @param skill_id Skill used
 * @param nameid Requested product
 * @param slot1
 * @param slot2
 * @param slot3
 * @param qty Amount of requested item
 * @param produce_idx Index of produce entry in skill_produce_db[]. (Optional. Assumed the requirements are complete, checked somewhere)
 * @return True is success, False if failed
 */
bool skill_produce_mix(map_session_data *sd, uint16 skill_id, t_itemid nameid, int slot1, int slot2, int slot3, int qty, short produce_idx)
{
	int slot[3];
	int i, sc, ele, idx, equip, wlv, make_per = 0, flag = 0, skill_lv = 0;
	int num = -1; // exclude the recipe

	nullpo_ret(sd);

	status_data* status = status_get_status_data(sd->bl);

	if( sd->skill_id_old == skill_id )
		skill_lv = sd->skill_lv_old;

	if (produce_idx == -1) {
		if( !(idx = skill_can_produce_mix(sd,nameid,-1, qty)) )
			return false;

		idx--;
	}
	else
		idx = produce_idx;

	if (qty < 1)
		qty = 1;

	if (!skill_id) //A skill can be specified for some override cases.
		skill_id = skill_produce_db[idx].req_skill;

	if( skill_id == GC_RESEARCHNEWPOISON )
		skill_id = GC_CREATENEWPOISON;

	slot[0] = slot1;
	slot[1] = slot2;
	slot[2] = slot3;

	for (i = 0, sc = 0, ele = 0; i < 3; i++) { //Note that qty should always be one if you are using these!
		short j;
		if (slot[i] <= 0)
			continue;
		j = pc_search_inventory(sd,slot[i]);
		if (j < 0)
			continue;
		if (slot[i] == ITEMID_STAR_CRUMB) {
			pc_delitem(sd,j,1,1,0,LOG_TYPE_PRODUCE);
			sc++;
		}
		if (slot[i] >= ITEMID_FLAME_HEART && slot[i] <= ITEMID_GREAT_NATURE && ele == 0) {
			static const int ele_table[4] = { ELE_FIRE, ELE_WATER, ELE_WIND, ELE_EARTH };
			pc_delitem(sd,j,1,1,0,LOG_TYPE_PRODUCE);
			ele = ele_table[slot[i]-ITEMID_FLAME_HEART];
		}
	}

	for (i = 0; i < MAX_PRODUCE_RESOURCE; i++) {
		short x, j;
		t_itemid id = skill_produce_db[idx].mat_id[i];

		if (!item_db.exists(id))
			continue;
		num++;
		x = (skill_id == RK_RUNEMASTERY ? 1 : qty) * skill_produce_db[idx].mat_amount[i];
		do {
			int y = 0;

			j = pc_search_inventory(sd,id);

			if (j >= 0) {
				y = sd->inventory.u.items_inventory[j].amount;
				if (y > x)
					y = x;
				pc_delitem(sd,j,y,0,0,LOG_TYPE_PRODUCE);
			} else {
				ShowError("skill_produce_mix: material item error\n");
				return false;
			}
			x -= y;
		} while( j >= 0 && x > 0 );
	}

	if ((equip = (itemdb_isequip(nameid) && skill_id != GN_CHANGEMATERIAL && skill_id != GN_MAKEBOMB)) && itemdb_type(nameid) == IT_WEAPON )
		wlv = itemdb_wlv(nameid);
	else
		wlv = 0;

	if (!equip) {
		switch (skill_id) {
			case BS_IRON:
			case BS_STEEL:
			case BS_ENCHANTEDSTONE:
				// Ores & Metals Refining - skill bonuses are straight from kRO website [DracoRPG]
				i = pc_checkskill(sd,skill_id);
				make_per = sd->status.job_level*20 + status->dex*10 + status->luk*10; //Base chance
				switch (nameid) {
					case ITEMID_IRON:
						make_per += 4000+i*500; // Temper Iron bonus: +26/+32/+38/+44/+50
						break;
					case ITEMID_STEEL:
						make_per += 3000+i*500; // Temper Steel bonus: +35/+40/+45/+50/+55
						break;
					case ITEMID_STAR_CRUMB:
						make_per = 100000; // Star Crumbs are 100% success crafting rate? (made 1000% so it succeeds even after penalties) [Skotlex]
						break;
					default: // Enchanted Stones
						make_per += 1000+i*500; // Enchanted stone Craft bonus: +15/+20/+25/+30/+35
						break;
				}
				break;
			case ASC_CDP:
				make_per = (2000 + 40*status->dex + 20*status->luk);
				break;
			case AL_HOLYWATER:
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
				if (hom_is_active(sd->hd)) {//Player got a homun
					int skill;
					if ((skill = hom_checkskill(sd->hd,HVAN_INSTRUCT)) > 0) //His homun is a vanil with instruction change
						make_per += skill*100; //+1% bonus per level
				}
				switch(nameid){
					case ITEMID_RED_POTION:
					case ITEMID_YELLOW_POTION:
					case ITEMID_WHITE_POTION:
						make_per += (1+rnd()%100)*10 + 2000;
						break;
					case ITEMID_ALCOHOL:
						make_per += (1+rnd()%100)*10 + 1000;
						break;
					case ITEMID_FIRE_BOTTLE:
					case ITEMID_ACID_BOTTLE:
					case ITEMID_MAN_EATER_BOTTLE:
					case ITEMID_MINI_BOTTLE:
						make_per += (1+rnd()%100)*10;
						break;
					case ITEMID_YELLOW_SLIM_POTION:
						make_per -= (1+rnd()%50)*10;
						break;
					case ITEMID_WHITE_SLIM_POTION:
					case ITEMID_COATING_BOTTLE:
						make_per -= (1+rnd()%100)*10;
						break;
					//Common items, receive no bonus or penalty, listed just because they are commonly produced
					case ITEMID_BLUE_POTION:
					case ITEMID_RED_SLIM_POTION:
					case ITEMID_ANODYNE:
					case ITEMID_ALOEBERA:
					default:
						break;
				}
				if (battle_config.pp_rate != 100)
					make_per = make_per * battle_config.pp_rate / 100;
				break;
			case SA_CREATECON: // Elemental Converter Creation
				make_per = 100000; // should be 100% success rate
				break;

			case RK_RUNEMASTERY: {
					int A = 100 * (30 + 2 * pc_checkskill(sd, skill_id));
					int B = 100 * status->dex / 30 + 10 * (status->luk + sd->status.job_level);
					int C = 100 * cap_value(sd->itemid,0,100); //itemid depend on makerune()
					int D = 0;

					switch (nameid) { //rune rank it_diff 9 craftable rune
						case ITEMID_BERKANA:
							D = -2000;
							break; //Rank S
						case ITEMID_NAUTHIZ:
						case ITEMID_URUZ:
							D = -1500;
							break; //Rank A
						case ITEMID_ISA:
						case ITEMID_WYRD:
							D = -1000;
						break; //Rank B
						case ITEMID_RAIDO:
						case ITEMID_THURISAZ:
						case ITEMID_HAGALAZ:
						case ITEMID_OTHILA:
							D = -500;
							break; //Rank C
						default:
							D = -1500;
							break; //not specified =-15%
					}
					make_per = A + B + C + D;

					uint8 runemastery_skill_lv = pc_checkskill(sd,skill_id);

					if (runemastery_skill_lv > 9)
						qty = 2 + rnd() % 5; // 2~6
					else if (runemastery_skill_lv > 4)
						qty = 2 + rnd() % 3; // 2~4
					else
						qty = 2;
				}
				break;

			case GC_CREATENEWPOISON:
				make_per = 3000 + 500 * pc_checkskill(sd,GC_RESEARCHNEWPOISON);
				qty = 1+rnd()%pc_checkskill(sd,GC_RESEARCHNEWPOISON);
				break;
			case GN_CHANGEMATERIAL:
				for (i = 0; i < MAX_SKILL_CHANGEMATERIAL_DB; i++) {
					if (skill_changematerial_db[i].nameid == nameid) {
						make_per = skill_changematerial_db[i].rate * 10;
						break;
					}
				}
				break;
			case GN_S_PHARMACY:
				{
					int difficulty = (620 - 20 * skill_lv); // (620 - 20 * Skill Level)
					const int production_count[] = { 7, 8, 8, 9, 9, 10, 10, 11, 11, 12 };

					switch (nameid) { // Item difficulty factor
						case ITEMID_HP_INCREASE_POTION_SMALL:
						case ITEMID_SP_INCREASE_POTION_SMALL:
						case ITEMID_CONCENTRATED_WHITE_POTION_Z:
							difficulty += 10;
							break;
						case ITEMID_BOMB_MUSHROOM_SPORE:
						case ITEMID_SP_INCREASE_POTION_MEDIUM:
							difficulty += 15;
							break;
						case ITEMID_HP_INCREASE_POTION_MEDIUM:
						case ITEMID_SP_INCREASE_POTION_LARGE:
						case ITEMID_VITATA500:
							difficulty += 20;
							break;
						case ITEMID_SEED_OF_HORNY_PLANT:
						case ITEMID_BLOODSUCK_PLANT_SEED:
						case ITEMID_CONCENTRATED_CEROMAIN_SOUP:
							difficulty += 30;
							break;
						case ITEMID_HP_INCREASE_POTION_LARGE:
						case ITEMID_CURE_FREE:
							difficulty += 40;
							break;
					}

					make_per = status->int_ + status->dex / 2 + status->luk + sd->status.job_level + (30 + rnd() % 120 + 1) + // Caster's INT + (Caster's DEX / 2) + Caster's LUK + Caster's Job Level + Random number between (30 ~ 150) +
						sd->status.base_level + 5 * (pc_checkskill(sd, AM_LEARNINGPOTION) - 20) + pc_checkskill(sd, CR_FULLPROTECTION) * (6 + rnd() % 4 + 1); // Caster's Base Level + (5 x (Potion Research Skill Level - 20)) + (Full Chemical Protection Skill Level x Random number between (6 ~ 10))
					make_per -= difficulty;
					qty = production_count[skill_lv - 1];

					// Determine quantity from difficulty
					if (make_per < 1)
						qty -= 6;
					else if (make_per < 100)
						qty -= 5;
					else if (make_per < 300)
						qty -= 4;
					else if (make_per < 400)
						qty -= 3;

					make_per = 100000; // Adjust success back to 100% for crafting
				}
				break;
			case GN_MAKEBOMB:
			case GN_MIX_COOKING:
				{
					int difficulty = 30 + rnd() % 120 + 1; // Random number between (30 ~ 150)

					switch (nameid) { // Item difficulty factor
						// GN_MAKEBOMB
						case ITEMID_APPLE_BOMB:
							difficulty += 5;
							break;
						case ITEMID_COCONUT_BOMB:
						case ITEMID_MELON_BOMB:
							difficulty += 10;
							break;
						case ITEMID_PINEAPPLE_BOMB:
							difficulty += 15;
							break;
						case ITEMID_BANANA_BOMB:
							difficulty += 20;
							break;
						// GN_MIX_COOKING
						case ITEMID_SAVAGE_FULL_ROAST:
						case ITEMID_COCKTAIL_WARG_BLOOD:
						case ITEMID_MINOR_STEW:
						case ITEMID_SIROMA_ICED_TEA:
						case ITEMID_DROSERA_HERB_SALAD:
						case ITEMID_PETITE_TAIL_NOODLES:
							difficulty += 15;
							break;
					}

					make_per = sd->status.job_level / 4 + status->luk / 2 + status->dex / 3; // (Caster's Job Level / 4) + (Caster's LUK / 2) + (Caster's DEX / 3)

					if (skill_lv > 1) {
						make_per -= difficulty;

						// Determine quantity from difficulty
						if (make_per >= 30)
							qty = 10 + rnd() % 2;
						else if (make_per >= 10)
							qty = 10;
						else if (make_per >= -10)
							qty = 8;
						else if (make_per >= -30)
							qty = 5;
						else
							qty = 0;
					} else {
						if (make_per < difficulty)
							qty = 0;
					}

					make_per = 100000; // Adjust success back to 100% for crafting
				}
				break;
			case MT_M_MACHINE:
			case BO_BIONIC_PHARMACY: // Difficulty formula unknown. Making it 100% success for now. [Rytech]
				if (skill_id == MT_M_MACHINE)
					qty = 7 + skill_lv;
				else // BO_BIONIC_PHARMACY
					qty = 10 + skill_lv;
				make_per = 100000;
				break;
			default:
				if (sd->menuskill_id == AM_PHARMACY &&
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
		make_per = 5000 + ((sd->class_&JOBL_THIRD)?1400:sd->status.job_level*20) + status->dex*10 + status->luk*10; // Base
		make_per += pc_checkskill(sd,skill_id)*500; // Smithing skills bonus: +5/+10/+15
		// Weaponry Research bonus: +1/+2/+3/+4/+5/+6/+7/+8/+9/+10
		make_per += pc_checkskill(sd,BS_WEAPONRESEARCH)*100;
		//  Oridecon Research bonus (custom): +1/+2/+3/+4/+5
		if( wlv >= 3 ){
			make_per += pc_checkskill(sd, BS_ORIDEOCON) * 100;
		}
		// Element Stone: -20%
		if( ele ){
			make_per -= 2000;
		}
		// Star Crumb: -15% each
		make_per -= sc * 1500;
		//  Weapon level malus: -0/-10/-20/-30
		if( wlv > 1 ){
			make_per -= ( wlv * 1000 );
		}
		if      (pc_search_inventory(sd,ITEMID_EMPERIUM_ANVIL) > -1) make_per+= 1000; // Emperium Anvil: +10
		else if (pc_search_inventory(sd,ITEMID_GOLDEN_ANVIL) > -1)   make_per+= 500; // Golden Anvil: +5
		else if (pc_search_inventory(sd,ITEMID_ORIDECON_ANVIL) > -1) make_per+= 300; // Oridecon Anvil: +3
		else if (pc_search_inventory(sd,ITEMID_ANVIL) > -1)          make_per+= 0; // Anvil: +0?
		if (battle_config.wp_rate != 100)
			make_per = make_per * battle_config.wp_rate / 100;
	}

	if (sd->class_&JOBL_BABY) //if it's a Baby Class
		make_per = (make_per * 50) / 100; //Baby penalty is 50% (bugreport:4847)

	if (make_per < 1) make_per = 1;

	if (qty > 1 || rnd()%10000 < make_per){ //Success, or crafting multiple items.
		struct item tmp_item;
		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid = nameid;
		tmp_item.amount = 1;
		tmp_item.identify = 1;
		if (equip) {
			tmp_item.card[0] = CARD0_FORGE;
			tmp_item.card[1] = ((sc*5)<<8)+ele;
			tmp_item.card[2] = GetWord(sd->status.char_id,0); // CharId
			tmp_item.card[3] = GetWord(sd->status.char_id,1);
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
				tmp_item.card[0] = CARD0_CREATE;
				tmp_item.card[1] = 0;
				tmp_item.card[2] = GetWord(sd->status.char_id,0); // CharId
				tmp_item.card[3] = GetWord(sd->status.char_id,1);
			}
		}

//		if(log_config.produce > 0)
//			log_produce(sd,nameid,slot1,slot2,slot3,1);
//TODO update PICKLOG

		if (equip) {
			clif_produceeffect(sd,0,nameid);
			clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_SUCCESS );
			if (wlv >= 3 && ((ele? 1 : 0) + sc) >= 3) // Fame point system [DracoRPG]
				pc_addfame(*sd, battle_config.fame_forge); // Success to forge a lv3 weapon with 3 additional ingredients = +10 fame point
		} else {
			int fame = 0;
			tmp_item.amount = 0;

			for (i = 0; i < qty; i++) {	//Apply quantity modifiers.
				if ((skill_id == GN_MIX_COOKING || skill_id == GN_MAKEBOMB || skill_id == GN_S_PHARMACY || skill_id == MT_M_MACHINE || skill_id == BO_BIONIC_PHARMACY) && make_per > 1) {
					tmp_item.amount = qty;
					break;
				}
				if (qty == 1 || rnd()%10000 < make_per) { //Success
					tmp_item.amount++;
					if (nameid < ITEMID_RED_SLIM_POTION || nameid > ITEMID_WHITE_SLIM_POTION)
						continue;
					if (skill_id != AM_PHARMACY &&
						skill_id != AM_TWILIGHT1 &&
						skill_id != AM_TWILIGHT2 &&
						skill_id != AM_TWILIGHT3)
						continue;
					//Add fame as needed.
					switch(++sd->potion_success_counter) {
						case 3:
							fame += battle_config.fame_pharmacy_3; // Success to prepare 3 Condensed Potions in a row
							break;
						case 5:
							fame += battle_config.fame_pharmacy_5; // Success to prepare 5 Condensed Potions in a row
							break;
						case 7:
							fame += battle_config.fame_pharmacy_7; // Success to prepare 7 Condensed Potions in a row
							break;
						case 10:
							fame += battle_config.fame_pharmacy_10; // Success to prepare 10 Condensed Potions in a row
							sd->potion_success_counter = 0;
							break;
					}
				} else //Failure
					sd->potion_success_counter = 0;
			}

			if (fame)
				pc_addfame(*sd, fame);
			//Visual effects and the like.
			switch (skill_id) {
				case AM_PHARMACY:
				case AM_TWILIGHT1:
				case AM_TWILIGHT2:
				case AM_TWILIGHT3:
				case ASC_CDP:
				case GC_CREATENEWPOISON:
					clif_produceeffect(sd,2,nameid);
					clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_SUCCESS );
					break;
				case BS_IRON:
				case BS_STEEL:
				case BS_ENCHANTEDSTONE:
					clif_produceeffect(sd,0,nameid);
					clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_SUCCESS );
					break;
				default: //Those that don't require a skill?
					if (skill_produce_db[idx].itemlv > 10 && skill_produce_db[idx].itemlv <= 20) { //Cooking items.
						clif_specialeffect(&sd->bl, EF_COOKING_OK, AREA);
						pc_setparam(sd, SP_COOKMASTERY, sd->cook_mastery + ( 1 << ( (skill_produce_db[idx].itemlv - 11) / 2 ) ) * 5);
					}
					break;
			}
		}

		if (skill_id == GN_CHANGEMATERIAL && tmp_item.amount) { //Success
			int j, k = 0, l;
			bool isStackable = itemdb_isstackable(tmp_item.nameid);

			for (i = 0; i < MAX_SKILL_CHANGEMATERIAL_DB; i++) {
				if (skill_changematerial_db[i].nameid == nameid){
					for (j = 0; j < MAX_SKILL_CHANGEMATERIAL_SET; j++){
						if (rnd()%1000 < skill_changematerial_db[i].qty_rate[j]){
							uint16 total_qty = qty * skill_changematerial_db[i].qty[j];
							tmp_item.amount = (isStackable ? total_qty : 1);
							for (l = 0; l < total_qty; l += tmp_item.amount) {
								if ((flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
									clif_additem(sd,0,0,flag);
									if( battle_config.skill_drop_items_full ){
										map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,4,0);
									}
								}
							}
							k++;
						}
					}
					break;
				}
			}
			if (k) {
				clif_produceeffect(sd,6,nameid);
				clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_SUCCESS );
				clif_msg_skill(sd,skill_id, MSI_SKILL_SUCCESS);
				return true;
			}
		} else if (tmp_item.amount) { //Success
			if ((flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
				clif_additem(sd,0,0,flag);
				if( battle_config.skill_drop_items_full ){
					map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,4,0);
				}
			}
			switch (skill_id) {
				case RK_RUNEMASTERY:
					clif_produceeffect(sd, 4, nameid);
					clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_SUCCESS );
					break;
				case GN_MIX_COOKING:
				case GN_MAKEBOMB:
				case GN_S_PHARMACY:
					clif_produceeffect(sd, 6, nameid);
					clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_SUCCESS );
					clif_msg_skill(sd, skill_id, MSI_SKILL_SUCCESS);
					break;
				case MT_M_MACHINE:
					clif_produceeffect(sd, 0, nameid);
					clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_SUCCESS );
					clif_msg_skill(sd, skill_id, MSI_SKILL_SUCCESS);
					break;
				case BO_BIONIC_PHARMACY:
					clif_produceeffect(sd, 2, nameid);
					clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_SUCCESS );
					clif_msg_skill(sd, skill_id, MSI_SKILL_SUCCESS);
					break;
			}
			return true;
		}
	}

	//Failure
//	if(log_config.produce)
//		log_produce(sd,nameid,slot1,slot2,slot3,0);
//TODO update PICKLOG

	if (equip) {
		clif_produceeffect(sd,1,nameid);
		clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_FAILURE );
	} else {
		switch (skill_id) {
			case ASC_CDP: //25% Damage yourself, and display same effect as failed potion.
				status_percent_damage(nullptr, &sd->bl, -25, 0, true);
				[[fallthrough]];
			case AM_PHARMACY:
			case AM_TWILIGHT1:
			case AM_TWILIGHT2:
			case AM_TWILIGHT3:
			case GC_CREATENEWPOISON:
				clif_produceeffect(sd,3,nameid);
				clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_FAILURE );
				sd->potion_success_counter = 0; // Fame point system [DracoRPG]
				break;
			case BS_IRON:
			case BS_STEEL:
			case BS_ENCHANTEDSTONE:
				clif_produceeffect(sd,1,nameid);
				clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_FAILURE );
				break;
			case RK_RUNEMASTERY:
				clif_produceeffect(sd,5,nameid);
				clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_FAILURE );
				break;
			case GN_MIX_COOKING:
				if (qty == 0) {
					item tmp_item;
					const t_itemid compensation[5] = { ITEMID_BLACK_LUMP, ITEMID_BLACK_HARD_LUMP, ITEMID_VERY_HARD_LUMP, ITEMID_BLACK_MASS, ITEMID_MYSTERIOUS_POWDER };
					int rate = rnd() % 1000 + 1;

					memset(&tmp_item, 0, sizeof(tmp_item));

					if (rate < 500)
						i = 0;
					else if (rate < 750)
						i = 1;
					else if (rate < 850)
						i = 2;
					else if (rate < 950)
						i = 3;
					else
						i = 4;

					tmp_item.nameid = compensation[i];
					tmp_item.amount = (skill_lv == 1) ? 1 : 5 + rand() % 5;
					tmp_item.identify = 1;
					if ((flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
						clif_additem(sd,0,0,flag);
						if( battle_config.skill_drop_items_full ){
							map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,4,0);
						}
					}
					clif_produceeffect(sd,7,nameid);
					clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_FAILURE );
					clif_msg_skill(sd, skill_id, MSI_SKILL_FAIL);
				}
				break;
			case GN_MAKEBOMB:
			case GN_S_PHARMACY:
			case GN_CHANGEMATERIAL:
				clif_produceeffect(sd,7,nameid);
				clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_FAILURE );
				clif_msg_skill(sd, skill_id, MSI_SKILL_FAIL);
				break;
			case MT_M_MACHINE:
				clif_produceeffect(sd, 1, nameid);
				clif_misceffect( sd->bl, NOTIFYEFFECT_REFINE_FAILURE );
				clif_msg_skill(sd, skill_id, MSI_SKILL_FAIL);
				break;
			case BO_BIONIC_PHARMACY:
				clif_produceeffect(sd, 3, nameid);
				clif_misceffect( sd->bl, NOTIFYEFFECT_PHARMACY_FAILURE );
				clif_msg_skill(sd, skill_id, MSI_SKILL_FAIL);
				break;
			default:
				if (skill_produce_db[idx].itemlv > 10 && skill_produce_db[idx].itemlv <= 20 ) { //Cooking items.
					clif_specialeffect(&sd->bl, EF_COOKING_FAIL, AREA);
					// todo: What in the world is this calculation
					pc_setparam(sd, SP_COOKMASTERY, sd->cook_mastery - ( 1 << ((skill_produce_db[idx].itemlv - 11) / 2) ) - ( ( ( 1 << ((skill_produce_db[idx].itemlv - 11) / 2) ) >> 1 ) * 3 ));
				}
				break;
		}
	}
	return false;
}

/**
 * Attempt to create arrow by specified material
 * @param sd Player
 * @param nameid Item ID of material
 * @return True if created, False is failed
 */
bool skill_arrow_create(map_session_data *sd, t_itemid nameid)
{
	nullpo_ret(sd);

	if (!nameid || !item_db.exists(nameid) || skill_arrow_db.empty())
		return false;

	std::shared_ptr<s_skill_arrow_db> arrow = nullptr;

	for (const auto &it : skill_arrow_db) {
		if (nameid == it.second->nameid) {
			arrow = it.second;
			break;
		}
	}
	short j;

	if (arrow == nullptr || (j = pc_search_inventory(sd,nameid)) < 0 || arrow->created.empty())
		return false;

	pc_delitem(sd,j,1,0,0,LOG_TYPE_PRODUCE);

	for (const auto &it : arrow->created) {
		char flag = 0;

		if (it.first == 0 || !item_db.exists(it.first) || it.second == 0)
			continue;

		struct item tmp_item = { 0 };
		tmp_item.identify = 1;
		tmp_item.nameid = it.first;
		tmp_item.amount = it.second;
		if (battle_config.produce_item_name_input&0x4) {
			tmp_item.card[0] = CARD0_CREATE;
			tmp_item.card[1] = 0;
			tmp_item.card[2] = GetWord(sd->status.char_id,0); // CharId
			tmp_item.card[3] = GetWord(sd->status.char_id,1);
		}
		if ((flag = pc_additem(sd,&tmp_item,tmp_item.amount,LOG_TYPE_PRODUCE))) {
			clif_additem(sd,0,0,flag);
			if( battle_config.skill_drop_items_full )
				map_addflooritem(&tmp_item,tmp_item.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,4,0);
		}
	}
	return true;
}

/**
 * Enchant weapon with poison
 * @param sd Player
 * @nameid Item ID of poison type
 */
void skill_poisoningweapon( map_session_data& sd, t_itemid nameid ){
	if( !nameid || pc_delitem(&sd,pc_search_inventory(&sd,nameid),1,0,0,LOG_TYPE_CONSUME) ) {
		clif_skill_fail( sd, GC_POISONINGWEAPON );
		return;
	}

	sc_type type;
	int chance;
	//uint16 msg = 1443; //Official is using msgstringtable.txt
	char output[CHAT_SIZE_MAX];
	const char *msg;

	switch( nameid ) {
		case ITEMID_PARALYSE:      type = SC_PARALYSE;      /*msg = 1444;*/ msg = "Paralyze"; break;
		case ITEMID_PYREXIA:       type = SC_PYREXIA;		/*msg = 1448;*/ msg = "Pyrexia"; break;
		case ITEMID_DEATHHURT:     type = SC_DEATHHURT;     /*msg = 1447;*/ msg = "Deathhurt"; break;
		case ITEMID_LEECHESEND:    type = SC_LEECHESEND;    /*msg = 1450;*/ msg = "Leech End"; break;
		case ITEMID_VENOMBLEED:    type = SC_VENOMBLEED;    /*msg = 1445;*/ msg = "Venom Bleed"; break;
		case ITEMID_TOXIN:         type = SC_TOXIN;         /*msg = 1443;*/ msg = "Toxin"; break;
		case ITEMID_MAGICMUSHROOM: type = SC_MAGICMUSHROOM; /*msg = 1446;*/ msg = "Magic Mushroom"; break;
		case ITEMID_OBLIVIONCURSE: type = SC_OBLIVIONCURSE; /*msg = 1449;*/ msg = "Oblivion Curse"; break;
		default:
			clif_skill_fail( sd, GC_POISONINGWEAPON );
			return;
	}

	status_change_end(&sd.bl, SC_POISONINGWEAPON); // End the status so a new poison can be applied (if changed)
	chance = 2 + 2 * sd.menuskill_val; // 2 + 2 * skill_lv
	sc_start4(&sd.bl,&sd.bl, SC_POISONINGWEAPON, 100, pc_checkskill(&sd, GC_RESEARCHNEWPOISON), //in Aegis it store the level of GC_RESEARCHNEWPOISON in val1
		type, chance, 0, skill_get_time(GC_POISONINGWEAPON, sd.menuskill_val));
	status_change_start(&sd.bl, &sd.bl, type, 10000, sd.menuskill_val, 0, 0, 0, skill_get_time(GC_POISONINGWEAPON, sd.menuskill_val), SCSTART_NOAVOID | SCSTART_NOICON); // Apply bonus to caster

	sprintf(output, msg_txt(&sd,721), msg);
	clif_messagecolor(&sd.bl,color_table[COLOR_WHITE],output,false,SELF);

/*#if PACKETVER >= 20110208 //! TODO: Check the correct PACKVETVER
	clif_msg(&sd,msg);
#endif*/
}

/**
 * Toggles the effect of Mystical Amplifcation (Pre-renewal).
 * @param bl: Object
 * @param skill_id: Skill invoking to determine if Magic type
 */
void skill_toggle_magicpower(struct block_list *bl, uint16 skill_id)
{
	status_change *sc = status_get_sc(bl);

	// non-offensive and non-magic skills do not affect the status
	if (skill_get_nk(skill_id, NK_NODAMAGE) || !(skill_get_type(skill_id)&BF_MAGIC))
		return;

	if (sc && sc->count && sc->getSCE(SC_MAGICPOWER)) {
		if (sc->getSCE(SC_MAGICPOWER)->val4) {
			status_change_end(bl, SC_MAGICPOWER);
		} else {
			sc->getSCE(SC_MAGICPOWER)->val4 = 1;
			status_calc_bl_(bl, status_db.getCalcFlag(SC_MAGICPOWER));
			if(bl->type == BL_PC){// update current display.
				clif_updatestatus(*((map_session_data *)bl),SP_MATK1);
				clif_updatestatus(*((map_session_data *)bl),SP_MATK2);
			}
		}
	}
}

void skill_magicdecoy( map_session_data& sd, t_itemid nameid ){
	int x, y, i, class_, skill;
	struct mob_data *md;

	skill = sd.menuskill_val;

	if( !nameid || !itemdb_group.item_exists(IG_ELEMENT, nameid) || (i = pc_search_inventory(&sd,nameid)) < 0 || !skill || pc_delitem(&sd,i,1,0,0,LOG_TYPE_CONSUME) ) {
		clif_skill_fail( sd, NC_MAGICDECOY );
		return;
	}

	// Spawn Position
	pc_delitem(&sd,i,1,0,0,LOG_TYPE_CONSUME);
	x = sd.sc.comet_x;
	y = sd.sc.comet_y;
	sd.sc.comet_x = 0;
	sd.sc.comet_y = 0;
	sd.menuskill_val = 0;

	// Item picked decides the mob class
	switch(nameid) {
		case ITEMID_SCARLET_PTS:		class_ = MOBID_MAGICDECOY_FIRE;		break;
		case ITEMID_INDIGO_PTS:			class_ = MOBID_MAGICDECOY_WATER;	break;
		case ITEMID_YELLOW_WISH_PTS:	class_ = MOBID_MAGICDECOY_WIND;		break;
		case ITEMID_LIME_GREEN_PTS:		class_ = MOBID_MAGICDECOY_EARTH;	break;
		default:
			clif_skill_fail( sd, NC_MAGICDECOY );
			return;
	}

	md = mob_once_spawn_sub(&sd.bl, sd.bl.m, x, y, sd.status.name, class_, "", SZ_SMALL, AI_NONE);
	if( md ) {
		struct unit_data *ud = unit_bl2ud(&md->bl);
		md->master_id = sd.bl.id;
		md->special_state.ai = AI_FAW;
		if(ud) {
			ud->skill_id = NC_MAGICDECOY;
			ud->skill_lv = skill;
		}
		if( md->deletetimer != INVALID_TIMER )
			delete_timer(md->deletetimer, mob_timer_delete);
		md->deletetimer = add_timer (gettick() + skill_get_time(NC_MAGICDECOY,skill), mob_timer_delete, md->bl.id, 0);
		mob_spawn(md);
	}
}

/**
 * Process Warlock Spellbooks
 * @param sd: Player data
 * @param nameid: Spellbook item used
 */
void skill_spellbook(map_session_data &sd, t_itemid nameid) {

	if (reading_spellbook_db.empty())
		return;

	status_change *sc = status_get_sc(&sd.bl);

	for (int i = SC_SPELLBOOK1; i <= SC_MAXSPELLBOOK; i++) {
		if (sc == nullptr || sc->getSCE(i) == nullptr)
			break;
		if (i == SC_MAXSPELLBOOK) {
			clif_skill_fail( sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_READING );
			return;
		}
	}

	std::shared_ptr<s_skill_spellbook_db> spell = reading_spellbook_db.findBook(nameid);

	if (spell == nullptr)
		return;

	uint16 skill_id = spell->skill_id, skill_lv = pc_checkskill(&sd, skill_id);

	if (skill_lv == 0) { // Caster hasn't learned the skill
		sc_start(&sd.bl, &sd.bl, SC_SLEEP, 100, 1, skill_get_time(WL_READING_SB, pc_checkskill(&sd, WL_READING_SB)));
		clif_skill_fail( sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_DIFFICULT_SLEEP );
		return;
	}

	uint16 points = spell->points;

	if (sc && sc->getSCE(SC_FREEZE_SP)) {
		if ((sc->getSCE(SC_FREEZE_SP)->val2 + points) > 8 * pc_checkskill(&sd, WL_FREEZE_SP) + status_get_int(&sd.bl) / 10 + sd.status.base_level / 10) {
			clif_skill_fail( sd, WL_READING_SB, USESKILL_FAIL_SPELLBOOK_PRESERVATION_POINT );
			return;
		}
		for (int i = SC_MAXSPELLBOOK; i >= SC_SPELLBOOK1; i--) { // This is how official saves spellbook. [malufett]
			if (!sc->getSCE(i)) {
				sc->getSCE(SC_FREEZE_SP)->val2 += points; // increase points
				sc_start4(&sd.bl, &sd.bl, (sc_type)i, 100, skill_id, skill_lv, points, 0, INFINITE_TICK);
				break;
			}
		}
	} else {
		sc_start2(&sd.bl, &sd.bl, SC_FREEZE_SP, 100, 0, points, INFINITE_TICK);
		sc_start4(&sd.bl, &sd.bl, SC_MAXSPELLBOOK, 100, skill_id, skill_lv, points, 0, INFINITE_TICK);
	}

	// Reading Spell Book SP cost same as the sealed spell.
	status_zap(&sd.bl, 0, skill_get_sp(skill_id, skill_lv));
}

void skill_select_menu( map_session_data& sd, uint16 skill_id ){
	int lv, prob, aslvl = 0;
	uint16 id, sk_idx = 0;

	if (sd.sc.getSCE(SC_STOP)) {
		aslvl = sd.sc.getSCE(SC_STOP)->val1;
		status_change_end(&sd.bl,SC_STOP);
	}

	if (!skill_id || !(sk_idx = skill_get_index(skill_id)))
		return;

	if( !skill_get_inf2(skill_id, INF2_ISAUTOSHADOWSPELL) || (id = sd.status.skill[sk_idx].id) == 0 || sd.status.skill[sk_idx].flag != SKILL_FLAG_PLAGIARIZED ) {
		clif_skill_fail( sd, SC_AUTOSHADOWSPELL );
		return;
	}

	lv = (aslvl + 5) / 2; // The level the skill will be autocasted
	lv = min(lv,sd.status.skill[sk_idx].lv);
	prob = (aslvl >= 10) ? 15 : (30 - 2 * aslvl); // Probability at level 10 was increased to 15.
	sc_start4(&sd.bl,&sd.bl,SC__AUTOSHADOWSPELL,100,id,lv,prob,aslvl,skill_get_time(SC_AUTOSHADOWSPELL,aslvl));
}

int skill_elementalanalysis( map_session_data& sd, int n, uint16 skill_lv, unsigned short* item_list ){
	nullpo_ret(item_list);

	if( n <= 0 )
		return 1;

	for( int i = 0; i < n; i++ ) {
		t_itemid nameid, product;
		int add_amount, del_amount, idx;
		struct item tmp_item;

		idx = item_list[i*2+0]-2;

		if( idx < 0 || idx >= MAX_INVENTORY ){
			return 1;
		}

		del_amount = item_list[i*2+1];

		if( skill_lv == 2 )
			del_amount -= (del_amount % 10);
		add_amount = (skill_lv == 1) ? del_amount * (5 + rnd()%5) : del_amount / 10 ;

		if( (nameid = sd.inventory.u.items_inventory[idx].nameid) <= 0 || del_amount > sd.inventory.u.items_inventory[idx].amount ) {
			clif_skill_fail( sd, SO_EL_ANALYSIS );
			return 1;
		}

		switch( nameid ) {
			// Level 1
			case ITEMID_FLAME_HEART:		product = ITEMID_BLOODY_RED;		break;
			case ITEMID_MISTIC_FROZEN:		product = ITEMID_CRYSTAL_BLUE;		break;
			case ITEMID_ROUGH_WIND:			product = ITEMID_WIND_OF_VERDURE;	break;
			case ITEMID_GREAT_NATURE:		product = ITEMID_YELLOW_LIVE;		break;
			// Level 2
			case ITEMID_BLOODY_RED:			product = ITEMID_FLAME_HEART;		break;
			case ITEMID_CRYSTAL_BLUE:		product = ITEMID_MISTIC_FROZEN;		break;
			case ITEMID_WIND_OF_VERDURE:	product = ITEMID_ROUGH_WIND;		break;
			case ITEMID_YELLOW_LIVE:		product = ITEMID_GREAT_NATURE;		break;
			default:
				clif_skill_fail( sd, SO_EL_ANALYSIS );
				return 1;
		}

		if( pc_delitem(&sd,idx,del_amount,0,1,LOG_TYPE_CONSUME) ) {
			clif_skill_fail( sd, SO_EL_ANALYSIS );
			return 1;
		}

		if( skill_lv == 2 && rnd()%100 < 25 ) {	// At level 2 have a fail chance. You loose your items if it fails.
			clif_skill_fail( sd, SO_EL_ANALYSIS );
			return 1;
		}

		memset(&tmp_item,0,sizeof(tmp_item));
		tmp_item.nameid = product;
		tmp_item.amount = add_amount;
		tmp_item.identify = 1;

		if( tmp_item.amount ) {
			unsigned char flag = pc_additem(&sd,&tmp_item,tmp_item.amount,LOG_TYPE_CONSUME);
			if( flag != 0 ) {
				clif_additem(&sd,0,0,flag);
				if( battle_config.skill_drop_items_full )
					map_addflooritem(&tmp_item,tmp_item.amount,sd.bl.m,sd.bl.x,sd.bl.y,0,0,0,4,0);
			}
		}

	}

	return 0;
}

int skill_changematerial(map_session_data *sd, int n, unsigned short *item_list) {
	int i, j, k, c, p = 0, amount;
	t_itemid nameid;

	nullpo_ret(sd);
	nullpo_ret(item_list);

	// Search for objects that can be created.
	for( i = 0; i < MAX_SKILL_PRODUCE_DB; i++ ) {
		if( skill_produce_db[i].itemlv == 26 && skill_produce_db[i].nameid > 0 ) {
			p = 0;
			do {
				c = 0;
				// Verification of overlap between the objects required and the list submitted.
				for( j = 0; j < MAX_PRODUCE_RESOURCE; j++ ) {
					if( skill_produce_db[i].mat_id[j] > 0 ) {
						for( k = 0; k < n; k++ ) {
							int idx = item_list[k*2+0]-2;

							if( idx < 0 || idx >= MAX_INVENTORY ){
								return 0;
							}

							nameid = sd->inventory.u.items_inventory[idx].nameid;
							amount = item_list[k*2+1];
							if( nameid > 0 && sd->inventory.u.items_inventory[idx].identify == 0 ){
								clif_msg_skill(sd, GN_CHANGEMATERIAL, MSI_SKILL_FAIL_MATERIAL_IDENTITY);
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
				skill_produce_mix(sd,GN_CHANGEMATERIAL,skill_produce_db[i].nameid,0,0,0,p,i);
				return 1;
			}
		}
	}

	if( p == 0)
		clif_msg_skill(sd, GN_CHANGEMATERIAL, MSI_SKILL_RECIPE_NOTEXIST);

	return 0;
}

/**
 * For Royal Guard's LG_TRAMPLE
 */
static int skill_destroy_trap(struct block_list *bl, va_list ap)
{
	skill_unit *su = (struct skill_unit *)bl;

	nullpo_ret(su);

	std::shared_ptr<s_skill_unit_group> sg;
	t_tick tick = va_arg(ap, t_tick);

	if (su->alive && (sg = su->group) && skill_get_inf2(sg->skill_id, INF2_ISTRAP)) {
		switch( sg->unit_id ) {
			case UNT_CLAYMORETRAP:
			case UNT_FIRINGTRAP:
			case UNT_ICEBOUNDTRAP:
				map_foreachinrange(skill_trap_splash,&su->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag|BL_SKILL|~BCT_SELF, &su->bl,tick);
				break;
			case UNT_LANDMINE:
			case UNT_BLASTMINE:
			case UNT_SHOCKWAVE:
			case UNT_SANDMAN:
			case UNT_FLASHER:
			case UNT_FREEZINGTRAP:
			case UNT_CLUSTERBOMB:
				if (battle_config.skill_wall_check && !skill_get_nk(sg->skill_id, NK_NODAMAGE))
					map_foreachinshootrange(skill_trap_splash,&su->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &su->bl,tick);
				else
					map_foreachinallrange(skill_trap_splash,&su->bl, skill_get_splash(sg->skill_id, sg->skill_lv), sg->bl_flag, &su->bl,tick);
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
int skill_blockpc_get(map_session_data *sd, int skillid) {
	int i;

	nullpo_retr(-1, sd);

	ARR_FIND(0, MAX_SKILLCOOLDOWN, i, sd->scd[i] && sd->scd[i]->skill_id == skillid);
	return (i >= MAX_SKILLCOOLDOWN) ? -1 : i;
}

TIMER_FUNC(skill_blockpc_end){
	map_session_data *sd = map_id2sd(id);
	int i = (int)data;

	if (!sd || data < 0 || data >= MAX_SKILLCOOLDOWN)
		return 0;

	if (!sd->scd[i] || sd->scd[i]->timer != tid) {
		ShowWarning("skill_blockpc_end: Invalid Timer or not Skill Cooldown.\n");
		return 0;
	}

	aFree(sd->scd[i]);
	sd->scd[i] = nullptr;
		return 1;
}

/**
 * Flags a singular skill as being blocked from persistent usage.
 * @param   sd        the player the skill delay affects
 * @param   skill_id   the skill which should be delayed
 * @param   tick      the length of time the delay should last
 * @param   load      whether this assignment is being loaded upon player login
 * @return  0 if successful, -1 otherwise
 */
int skill_blockpc_start(map_session_data *sd, int skill_id, t_tick tick) {
	int i;

	nullpo_retr(-1, sd);

	if (!skill_id || tick < 1)
		return -1;

	ARR_FIND(0, MAX_SKILLCOOLDOWN, i, sd->scd[i] && sd->scd[i]->skill_id == skill_id);
	if (i < MAX_SKILLCOOLDOWN) { // Skill already with cooldown
		delete_timer(sd->scd[i]->timer, skill_blockpc_end);
		aFree(sd->scd[i]);
		sd->scd[i] = nullptr;
	}

	ARR_FIND(0, MAX_SKILLCOOLDOWN, i, !sd->scd[i]);
	if (i < MAX_SKILLCOOLDOWN) { // Free Slot found
		CREATE(sd->scd[i], struct skill_cooldown_entry, 1);
		sd->scd[i]->skill_id = skill_id;
		sd->scd[i]->timer = add_timer(gettick() + tick, skill_blockpc_end, sd->bl.id, i);

		if (battle_config.display_status_timers)
			clif_skill_cooldown( *sd, skill_id, tick );

		return 1;
	} else {
		ShowWarning("skill_blockpc_start: Too many skillcooldowns, increase MAX_SKILLCOOLDOWN.\n");
		return 0;
	}
}

int skill_blockpc_clear(map_session_data *sd) {
	int i;

	nullpo_ret(sd);

	for (i = 0; i < MAX_SKILLCOOLDOWN; i++) {
		if (!sd->scd[i])
			continue;
		delete_timer(sd->scd[i]->timer, skill_blockpc_end);
		aFree(sd->scd[i]);
		sd->scd[i] = nullptr;
	}
	return 1;
}

TIMER_FUNC(skill_blockhomun_end){
	struct homun_data *hd = (TBL_HOM*) map_id2bl(id);

	if (hd) {
		auto skill = util::vector_get(hd->blockskill, (uint16)data);

		if (skill != hd->blockskill.end())
			hd->blockskill.erase(skill);
	}

	return 1;
}

int skill_blockhomun_start(struct homun_data *hd, uint16 skill_id, int tick)	//[orn]
{
	nullpo_retr(-1, hd);

	if (!skill_db.exists(skill_id))
		return -1;

	auto skill = util::vector_get(hd->blockskill, skill_id);

	if (tick < 1 && skill != hd->blockskill.end()) {
		hd->blockskill.erase(skill);
		return -1;
	}

	hd->blockskill.push_back(skill_id);

	if (battle_config.display_status_timers)
		clif_skill_cooldown(*hd->master, skill_id, tick);

	return add_timer(gettick() + tick, skill_blockhomun_end, hd->bl.id, skill_id);
}

TIMER_FUNC(skill_blockmerc_end){
	s_mercenary_data *md = (TBL_MER*)map_id2bl(id);

	if (md) {
		auto skill = util::vector_get(md->blockskill, (uint16)data);

		if (skill != md->blockskill.end())
			md->blockskill.erase(skill);
	}

	return 1;
}

int skill_blockmerc_start(s_mercenary_data *md, uint16 skill_id, int tick)
{
	nullpo_retr(-1, md);

	if (!skill_db.exists(skill_id))
		return -1;

	auto skill = util::vector_get(md->blockskill, skill_id);

	if (tick < 1 && skill != md->blockskill.end()) {
		md->blockskill.erase(skill);
		return -1;
	}

	md->blockskill.push_back(skill_id);

	if (battle_config.display_status_timers)
		clif_skill_cooldown(*md->master, skill_id, tick);

	return add_timer(gettick() + tick, skill_blockmerc_end, md->bl.id, skill_id);
}
/**
 * Adds a new skill unit entry for this player to recast after map load
 * @param sd: Player
 * @param skill_id: Skill ID to save
 * @param skill_lv: Skill level to save
 */
void skill_usave_add(map_session_data *sd, uint16 skill_id, uint16 skill_lv)
{
	struct skill_usave *sus = nullptr;

	if (idb_exists(skillusave_db,sd->status.char_id))
		idb_remove(skillusave_db,sd->status.char_id);

	CREATE(sus, struct skill_usave, 1);
	idb_put(skillusave_db, sd->status.char_id, sus);

	sus->skill_id = skill_id;
	sus->skill_lv = skill_lv;
}

/**
 * Loads saved skill unit entries for this player after map load
 * @param sd: Player
 */
void skill_usave_trigger(map_session_data *sd)
{
	skill_usave *sus;
	std::shared_ptr<s_skill_unit_group> group;

	if (!(sus = static_cast<skill_usave *>(idb_get(skillusave_db,sd->status.char_id))))
		return;

	if ((group = skill_unitsetting(&sd->bl, sus->skill_id, sus->skill_lv, sd->bl.x, sd->bl.y, 0)))
		if (sus->skill_id == NC_NEUTRALBARRIER || sus->skill_id == NC_STEALTHFIELD )
			sc_start2(&sd->bl, &sd->bl, (sus->skill_id == NC_NEUTRALBARRIER ? SC_NEUTRALBARRIER_MASTER : SC_STEALTHFIELD_MASTER), 100, sus->skill_lv, group->group_id, skill_get_time(sus->skill_id, sus->skill_lv));
	idb_remove(skillusave_db, sd->status.char_id);
}

/*
 *
 */
int skill_split_str (char *str, char **val, int num)
{
	int i;

	for( i = 0; i < num && str; i++ ) {
		val[i] = str;
		str = strchr(str,',');
		if( str )
			*str++ = 0;
	}

	return i;
}

/*
 *
 */
void skill_init_unit_layout (void) {
	int i,j,pos = 0;

	memset(skill_unit_layout,0,sizeof(skill_unit_layout));

	// standard square layouts go first
	for (i=0; i<=MAX_SQUARE_LAYOUT; i++) {
		int size = i*2+1;
		skill_unit_layout[i].count = size*size;
		for (j=0; j<size*size; j++) {
			skill_unit_layout[i].dx[j] = (j%size-i);
			skill_unit_layout[i].dy[j] = (j/size-i);
		}
	}

	// afterwards add special ones
	pos = i;
	for (const auto &it : skill_db) {
		std::shared_ptr<s_skill_db> skill = it.second;
		uint16 skill_id = skill->nameid;

		if (!skill->unit_id || skill->unit_layout_type[0] != -1)
			continue;

		if( skill_id == EL_FIRE_MANTLE ) {
			static const int dx[] = {-1, 0, 1, 1, 1, 0,-1,-1};
			static const int dy[] = { 1, 1, 1, 0,-1,-1,-1, 0};
			skill_unit_layout[pos].count = 8;
			memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
			memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
		} else {
			switch (skill_id) {
				case MG_FIREWALL:
				case WZ_ICEWALL:
				case WL_EARTHSTRAIN:
				case RL_FIRE_RAIN:
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
						skill->unit_layout_type[j] = pos;
						//lv2/3
						j++;
						pos++;
						skill_unit_layout[pos].count = 8;
						memcpy(skill_unit_layout[pos].dx,dx2,sizeof(dx2));
						memcpy(skill_unit_layout[pos].dy,dy2,sizeof(dy2));
						skill->unit_layout_type[j] = pos;
						skill->unit_layout_type[++j] = pos;
						//lv4/5
						j++;
						pos++;
						skill_unit_layout[pos].count = 12;
						memcpy(skill_unit_layout[pos].dx,dx3,sizeof(dx3));
						memcpy(skill_unit_layout[pos].dy,dy3,sizeof(dy3));
						skill->unit_layout_type[j] = pos;
						skill->unit_layout_type[++j] = pos;
						//Fill in the rest using lv 5.
						for (;j<MAX_SKILL_LEVEL;j++)
							skill->unit_layout_type[j] = pos;
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
				case NPC_FLAMECROSS: {
						static const int dx[] = {-2,-1, 1, 2, 0, 0, 0, 0};
						static const int dy[] = { 0, 0, 0, 0,-2,-1, 1, 2};

						skill_unit_layout[pos].count = 8;
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
			skill->unit_layout_type[j] = pos;
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
	firerain_unit_pos = pos;
	for( i = 0; i < 8; i++ ) {
		skill_unit_layout[pos].count = 3;
		switch( i ) {
			case 0: case 1: case 3: case 4: case 5: case 7:
				{
					static const int dx[] = {-1, 0, 1};
					static const int dy[] = { 0, 0, 0};

					memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
			case 2:
			case 6:
				{
					static const int dx[] = { 0, 0, 0};
					static const int dy[] = {-1, 0, 1};

					memcpy(skill_unit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_unit_layout[pos].dy,dy,sizeof(dy));
				}
				break;
		}
		pos++;
	}

	if( pos >= MAX_SKILL_UNIT_LAYOUT )
		ShowError("skill_init_unit_layout: The skill_unit_layout has met the limit or overflowed (pos=%d)\n", pos);
}

void skill_init_nounit_layout (void) {
	int i, pos = 0;

	memset(skill_nounit_layout,0,sizeof(skill_nounit_layout));

	for( i = 0; i < 8; i++ ) {
		if( i&1 ) {
			skill_nounit_layout[pos].count = 33;
			if( i&2 ) {
				if( i&4 ) { // 7
					int dx[] = { 5, 6, 7, 5, 6, 4, 5, 6, 4, 5, 3, 4, 5, 3, 4, 2, 3, 4, 2, 3, 1, 2, 3, 1, 2, 0, 1, 2, 0, 1,-1, 0, 1};
					int dy[] = { 7, 6, 5, 6, 5, 6, 5, 4, 5, 4, 5, 4, 3, 4, 3, 4, 3, 2, 3, 2, 3, 2, 1, 2, 1, 2, 1, 0, 1, 0, 1, 0,-1};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 3
					int dx[] = {-5,-6,-7,-5,-6,-4,-5,-6,-4,-5,-3,-4,-5,-3,-4,-2,-3,-4,-2,-3,-1,-2,-3,-1,-2, 0,-1,-2, 0,-1, 1, 0,-1};
					int dy[] = {-7,-6,-5,-6,-5,-6,-5,-4,-5,-4,-5,-4,-3,-4,-3,-4,-3,-2,-3,-2,-3,-2,-1,-2,-1,-2,-1, 0,-1, 0,-1, 0, 1};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			} else {
				if( i&4 ) { // 5
					int dx[] = { 7, 6, 5, 6, 5, 6, 5, 4, 5, 4, 5, 4, 3, 4, 3, 4, 3, 2, 3, 2, 3, 2, 1, 2, 1, 2, 1, 0, 1, 0, 1, 0,-1};
					int dy[] = {-5,-6,-7,-5,-6,-4,-5,-6,-4,-5,-3,-4,-5,-3,-4,-2,-3,-4,-2,-3,-1,-2,-3,-1,-2, 0,-1,-2, 0,-1, 1, 0,-1};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 1
					int dx[] = {-7,-6,-5,-6,-5,-6,-5,-4,-5,-4,-5,-4,-3,-4,-3,-4,-3,-2,-3,-2,-3,-2,-1,-2,-1,-2,-1, 0,-1, 0,-1, 0, 1};
					int dy[] = { 5, 6, 7, 5, 6, 4, 5, 6, 4, 5, 3, 4, 5, 3, 4, 2, 3, 4, 2, 3, 1, 2, 3, 1, 2, 0, 1, 2, 0, 1,-1, 0, 1};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			}
		} else {
			skill_nounit_layout[pos].count = 21;
			if( i&2 ) {
				if( i&4 ) { // 6
					int dx[] = { 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6};
					int dy[] = { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 2
					int dx[] = {-6,-5,-4,-3,-2,-1, 0,-6,-5,-4,-3,-2,-1, 0,-6,-5,-4,-3,-2,-1, 0};
					int dy[] = { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			} else {
				if( i&4 ) { // 4
					int dx[] = {-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1};
					int dy[] = { 0, 0, 0,-1,-1,-1,-2,-2,-2,-3,-3,-3,-4,-4,-4,-5,-5,-5,-6,-6,-6};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 0
					int dx[] = {-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1,-1, 0, 1};
					int dy[] = { 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			}
		}
		pos++;
	}

	for( i = 0; i < 8; i++ ) {
		if( i&1 ) {
			skill_nounit_layout[pos].count = 74;
			if( i&2 ) {
				if( i&4 ) { // 7
					int dx[] = {-2,-1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-2,-1, 0, 1, 2, 3, 4, 5, 6, 7,
								-3,-2,-1, 0, 1, 2, 3, 4, 5, 6, 7,-3,-2,-1,-0, 1, 2, 3, 4, 5, 6,
								-4,-3,-2,-1, 0, 1, 2, 3, 4, 5, 6,-4,-3,-2,-1,-0, 1, 2, 3, 4, 5,
								-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5};
					int dy[] = { 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,
								 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,
								 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,
								 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 3
					int dx[] = { 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,
								 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,
								 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,
								 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5};
					int dy[] = {-8,-7,-6,-5,-4,-3,-2,-1, 0, 1, 2,-7,-6,-5,-4,-3,-2,-1, 0, 1, 2,
								-7,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3,
								-6,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4,
								-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			} else {
				if( i&4 ) { // 5
					int dx[] = { 8, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2, 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,
								 7, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3, 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,
								 6, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,
								 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5};
					int dy[] = { 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,-8, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7,
								 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,-7, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6,
								 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,-6, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5,
								 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 1
					int dx[] = {-8,-7,-6,-5,-4,-3,-2,-1, 0, 1, 2,-7,-6,-5,-4,-3,-2,-1, 0, 1, 2,
								-7,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3,-6,-5,-4,-3,-2,-1, 0, 1, 2, 3,
								-6,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4,
								-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5};
					int dy[] = {-2,-1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-2,-1, 0, 1, 2, 3, 4, 5, 6, 7,
								-3,-2,-1, 0, 1, 2, 3, 4, 5, 6, 7,-3,-2,-1, 0, 1, 2, 3, 4, 5, 6,
								-4,-3,-2,-1, 0, 1, 2, 3, 4, 5, 6,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5,
								-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			}
		} else {
			skill_nounit_layout[pos].count = 44;
			if( i&2 ) {
				if( i&4 ) { // 6
					int dx[] = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3};
					int dy[] = { 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3,-4,-4,-4,-4,-5,-5,-5,-5};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 2
					int dx[] = {-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0,-3,-2,-1, 0};
					int dy[] = { 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0,-1,-1,-1,-1,-2,-2,-2,-2,-3,-3,-3,-3,-4,-4,-4,-4,-5,-5,-5,-5};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			} else {
				if( i&4 ) { // 4
					int dx[] = { 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5, 5, 4, 3, 2, 1, 0,-1,-2,-3,-4,-5};
					int dy[] = {-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				} else { // 0
					int dx[] = {-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5,-5,-4,-3,-2,-1, 0, 1, 2, 3, 4, 5};
					int dy[] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

					memcpy(skill_nounit_layout[pos].dx,dx,sizeof(dx));
					memcpy(skill_nounit_layout[pos].dy,dy,sizeof(dy));
				}
			}
		}
		pos++;
	}

	if( pos >= MAX_SKILL_UNIT_LAYOUT2 )
		ShowError("skill_init_nounit_layout: The skill_nounit_layout has met the limit or overflowed (pos=%d)\n", pos);
}

int skill_block_check(struct block_list *bl, sc_type type , uint16 skill_id) {
	status_change *sc = status_get_sc(bl);

	if( !sc || !bl || !skill_id )
		return 0; // Can do it

	switch (type) {
		case SC_ANKLE:
			if (skill_id == AL_TELEPORT)
				return 1;
			break;
		case SC_STASIS:
			if (skill_get_inf2_(skill_id, { INF2_ISSONG, INF2_ISENSEMBLE, INF2_ISCHORUS }) || skill_get_type(skill_id) == BF_MAGIC)
				return 1; // Can't do it.
			break;
		case SC_KAGEHUMI:
			if (skill_get_inf2(skill_id, INF2_IGNOREKAGEHUMI))
				return 1;
			break;
		case SC_BITE:
			if (skill_get_inf2(skill_id, INF2_IGNOREWUGBITE))
				return 1;
			break;
	}

	return 0;
}

/**
 * Determines whether a skill is currently active or not. Used for purposes of cancelling HP/SP usage when disabling a skill.
 * @param sc: Status changes active on target
 * @param skill_id: Skill to toggle
 * @return True on success or false otherwise
 */
bool skill_disable_check(status_change &sc, uint16 skill_id) {
	std::shared_ptr<s_skill_db> skill = skill_db.find(skill_id);

	if (skill == nullptr || skill->sc <= SC_NONE || skill->sc >= SC_MAX)
		return false;

	if (skill->inf2[INF2_TOGGLEABLE]) {
		if (sc.getSCE(skill->sc))
			return true;
		// These 2 skills contain a master and are not correctly pulled using skill_get_sc
		if (skill->nameid == NC_NEUTRALBARRIER && sc.getSCE(SC_NEUTRALBARRIER_MASTER))
			return true;
		if (skill->nameid == NC_STEALTHFIELD && sc.getSCE(SC_STEALTHFIELD_MASTER))
			return true;
	}

	return false;
}

int skill_get_elemental_type( uint16 skill_id , uint16 skill_lv ) {
	int type = 0;

	switch( skill_id ) {
		case SO_SUMMON_AGNI:	type = ELEMENTALID_AGNI_S;		break;
		case SO_SUMMON_AQUA:	type = ELEMENTALID_AQUA_S;		break;
		case SO_SUMMON_VENTUS:	type = ELEMENTALID_VENTUS_S;	break;
		case SO_SUMMON_TERA:	type = ELEMENTALID_TERA_S;		break;
	}

	type += skill_lv - 1;

	return type;
}

/**
 * Check before do `unit_movepos` call
 * @param check_flag Flags: 1:Check for BG maps, 2:Check for GVG maps on WOE times, 4:Check for GVG maps regardless Agit flags
 * @return True:If unit can be moved, False:If check on flags are met or unit cannot be moved.
 **/
static bool skill_check_unit_movepos(uint8 check_flag, struct block_list *bl, short dst_x, short dst_y, int easy, bool checkpath) {
	status_change *sc;

	nullpo_retr(false, bl);

	struct map_data *mapdata = map_getmapdata(bl->m);

	if (check_flag&1 && mapdata->getMapFlag(MF_BATTLEGROUND))
		return false;
	if (check_flag&2 && mapdata_flag_gvg(mapdata))
		return false;
	if (check_flag&4 && mapdata_flag_gvg2(mapdata))
		return false;

	sc = status_get_sc(bl);
	if (sc && sc->getSCE(SC_SV_ROOTTWIST))
		return false;

	return unit_movepos(bl, dst_x, dst_y, easy, checkpath);
}

/**
 * Get skill duration after adjustments by skill_duration mapflag
 * @param mapdata: Source map data
 * @param skill_id: Skill ID
 * @param skill_lv: Skill level
 * @return Adjusted skill duration
 */
int skill_get_time3(struct map_data *mapdata, uint16 skill_id, uint16 skill_lv) {
	int time = 0;

	if (!(time = skill_get_time(skill_id, skill_lv)))
		return 0;

	if (mapdata && !mapdata->skill_duration.empty() && mapdata->skill_duration.find(skill_id) != mapdata->skill_duration.end())
		return time / 100 * mapdata->skill_duration[skill_id];
	return time;
}

const std::string SkillDatabase::getDefaultLocation() {
	return std::string(db_path) + "/skill_db.yml";
}

template<typename T, size_t S> bool SkillDatabase::parseNode(const std::string& nodeName, const std::string& subNodeName, const ryml::NodeRef& node, T (&arr)[S]) {
	int32 value;
	const auto& skNode = node[c4::to_csubstr(nodeName)];
	if (!skNode.is_seq()) {
		if (!this->asInt32(node, nodeName, value))
			return false;

		for (size_t i = 0; i < S; i++)
			arr[i] = value;
	} else {
		uint16 max_level = 0;

		for (const auto& it : skNode) {
			uint16 skill_lv;

			if (!this->asUInt16(it, "Level", skill_lv))
				continue;

			if (skill_lv > MAX_SKILL_LEVEL) {
				this->invalidWarning(it["Level"], "%s Level exceeds the maximum skill level of %d, skipping.\n", nodeName.c_str(), MAX_SKILL_LEVEL);
				return false;
			}

			if (!this->asInt32(it, subNodeName, value))
				continue;

			arr[skill_lv - 1] = value;
			max_level = max(max_level, skill_lv);
		}

		size_t i = max_level, j;

		// Check for linear change with increasing steps until we reach half of the data acquired.
		for (size_t step = 1; step <= i / 2; step++) {
			int diff = arr[i - 1] - arr[i - step - 1];

			for (j = i - 1; j >= step; j--) {
				if ((arr[j] - arr[j - step]) != diff)
					break;
			}

			if (j >= step) // No match, try next step.
				continue;

			for (; i < MAX_SKILL_LEVEL; i++) { // Apply linear increase
				arr[i] = arr[i - step] + diff;

				if (arr[i] < 1 && arr[i - 1] >= 0) { // Check if we have switched from + to -, cap the decrease to 0 in said cases.
					arr[i] = 1;
					diff = 0;
					step = 1;
				}
			}

			return true;
		}

		// Unable to determine linear trend, fill remaining array values with last value
		for (; i < S; i++)
			arr[i] = arr[max_level - 1];
	}

	return true;
}

/**
 * Reads and parses an entry from the skill_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 SkillDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint16 skill_id;

	if (!this->asUInt16(node, "Id", skill_id))
		return 0;

	std::shared_ptr<s_skill_db> skill = this->find(skill_id);
	bool exists = skill != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Name", "Description", "MaxLevel" }))
			return 0;

		skill = std::make_shared<s_skill_db>();
		skill->nameid = skill_id;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		name.resize(SKILL_NAME_LENGTH);
		memcpy(skill->name, name.c_str(), sizeof(skill->name));
	}

	if (this->nodeExists(node, "Description")) {
		std::string name;

		if (!this->asString(node, "Description", name))
			return 0;

		name.resize(SKILL_DESC_LENGTH);
		memcpy(skill->desc, name.c_str(), sizeof(skill->desc));
	}

	if (this->nodeExists(node, "MaxLevel")) {
		uint16 skill_lv;

		if (!this->asUInt16(node, "MaxLevel", skill_lv))
			return 0;

		if (skill_lv == 0 || skill_lv > MAX_SKILL_LEVEL) {
			this->invalidWarning(node["MaxLevel"], "MaxLevel %hu does not meet the bounds of 1~%d.\n", skill_lv, MAX_SKILL_LEVEL);
			return 0;
		}

		skill->max = skill_lv;
	}

	if (this->nodeExists(node, "Type")) {
		std::string type;

		if (!this->asString(node, "Type", type))
			return 0;

		std::string type_constant = "BF_" + type;

		int64 constant;

		if (!script_get_constant(type_constant.c_str(), &constant)) {
			this->invalidWarning(node["Type"], "Type %s is invalid.\n", type.c_str());
			return 0;
		}

		if (constant < BF_NONE || constant > BF_MISC) {
			this->invalidWarning(node["Type"], "Constant Type %s is not a supported skill type.\n", type_constant.c_str());
			return 0;
		}

		skill->skill_type = static_cast<e_battle_flag>(constant);
	} else {
		if (!exists)
			skill->skill_type = BF_NONE;
	}

	if (this->nodeExists(node, "TargetType")) {
		std::string inf;

		if (!this->asString(node, "TargetType", inf))
			return 0;

		std::string inf_constant = "INF_" + inf + "_SKILL";
		int64 constant;

		if (!script_get_constant(inf_constant.c_str(), &constant)) {
			this->invalidWarning(node["TargetType"], "TargetType %s is invalid.\n", inf.c_str());
			return 0;
		}

		skill->inf = static_cast<uint16>(constant);
	}

	if (this->nodeExists(node, "DamageFlags")) {
		const auto& damageNode = node["DamageFlags"];

		for (const auto& it : damageNode) {
			std::string nk;
			c4::from_chars(it.key(), &nk);
			std::string nk_constant = "NK_" + nk;
			int64 constant;

			if (!script_get_constant(nk_constant.c_str(), &constant)) {
				this->invalidWarning(damageNode, "DamageFlags %s is invalid.\n", nk.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(damageNode, nk, active))
				return 0;

			if (active)
				skill->nk.set(static_cast<uint8>(constant));
			else
				skill->nk.reset(static_cast<uint8>(constant));
		}
	}

	if (this->nodeExists(node, "Flags")) {
		const auto& infoNode = node["Flags"];

		for (const auto& it : infoNode) {
			std::string inf2;
			c4::from_chars(it.key(), &inf2);
			std::string inf2_constant = "INF2_" + inf2;
			int64 constant;

			if (!script_get_constant(inf2_constant.c_str(), &constant)) {
				this->invalidWarning(infoNode, "Flag %s is invalid, skipping.\n", inf2.c_str());
				continue;
			}

			bool active;

			if (!this->asBool(infoNode, inf2, active))
				return 0;

			if (active)
				skill->inf2.set(static_cast<uint8>(constant));
			else
				skill->inf2.reset(static_cast<uint8>(constant));
		}
	}

	if (this->nodeExists(node, "Range")) {
		if (!this->parseNode("Range", "Size", node, skill->range))
			return 0;
	} else {
		if (!exists)
			memset(skill->range, 0, sizeof(skill->range));
	}

	if (this->nodeExists(node, "Hit")) {
		std::string hit;

		if (!this->asString(node, "Hit", hit))
			return 0;

		std::string hit_constant = "DMG_" + hit;
		int64 constant;

		if (!script_get_constant(hit_constant.c_str(), &constant)) {
			this->invalidWarning(node["Hit"], "Hit %s is invalid.\n", hit.c_str());
			return 0;
		}

		skill->hit = static_cast<e_damage_type>(constant);
	} else {
		if (!exists)
			skill->hit = DMG_NORMAL;
	}

	if (this->nodeExists(node, "HitCount")) {
		if (!this->parseNode("HitCount", "Count", node, skill->num))
			return 0;
	} else {
		if (!exists)
			memset(skill->num, 0, sizeof(skill->num));
	}

	if (this->nodeExists(node, "Element")) {
		const auto elementNode = node["Element"];
		std::string element;

		if (!elementNode.is_seq()) {
			if (!this->asString(node, "Element", element))
				return 0;

			std::string element_constant = "ELE_" + element;
			int64 constant;

			if (!script_get_constant(element_constant.c_str(), &constant)) {
				this->invalidWarning(elementNode["Element"], "Element %s is invalid.\n", element.c_str());
				return 0;
			}

			if (constant == ELE_NONE) {
				this->invalidWarning(elementNode["Element"], "%s is not a valid element.\n", element.c_str());
				return 0;
			}

			memset(skill->element, static_cast<e_element>(constant), sizeof(skill->element));
		} else {
			for (const auto& it : elementNode) {
				uint16 skill_lv;

				if (!this->asUInt16(it, "Level", skill_lv))
					continue;

				if (skill_lv > MAX_SKILL_LEVEL) {
					this->invalidWarning(it["Level"], "Element Level exceeds the maximum skill level of %d, skipping.\n", MAX_SKILL_LEVEL);
					return false;
				}

				if (!this->asString(it, "Element", element))
					continue;

				std::string element_constant = "ELE_" + element;
				int64 constant;

				if (!script_get_constant(element_constant.c_str(), &constant)) {
					this->invalidWarning(elementNode["Element"], "Element %s is invalid.\n", element.c_str());
					return 0;
				}

				if (constant == ELE_NONE) {
					this->invalidWarning(elementNode["Element"], "%s is not a valid element.\n", element.c_str());
					return 0;
				}

				skill->element[skill_lv - 1] = static_cast<e_element>(constant);
			}
		}
	} else {
		if (!exists)
			memset(skill->element, ELE_NEUTRAL, sizeof(skill->element));
	}

	if (this->nodeExists(node, "SplashArea")) {
		if (!this->parseNode("SplashArea", "Area", node, skill->splash))
			return 0;
	} else {
		if (!exists)
			memset(skill->splash, 0, sizeof(skill->splash));
	}

	if (this->nodeExists(node, "ActiveInstance")) {
		if (!this->parseNode("ActiveInstance", "Max", node, skill->maxcount))
			return 0;
	} else {
		if (!exists)
			memset(skill->maxcount, 0, sizeof(skill->maxcount));
	}

	if (this->nodeExists(node, "Knockback")) {
		if (!this->parseNode("Knockback", "Amount", node, skill->blewcount))
			return 0;
	} else {
		if (!exists)
			memset(skill->blewcount, 0, sizeof(skill->blewcount));
	}

	if (this->nodeExists(node, "CopyFlags")) {
		const auto& copyNode = node["CopyFlags"];

		if (this->nodeExists(copyNode, "Skill")) {
			const auto& copyskillNode = copyNode["Skill"];

			if (this->nodeExists(copyskillNode, "Plagiarism")) {
				bool active;

				if (!this->asBool(copyskillNode, "Plagiarism", active))
					return 0;

				if (active)
					skill->copyable.option |= SKILL_COPY_PLAGIARISM;
				else
					skill->copyable.option &= SKILL_COPY_PLAGIARISM;
			}

			if (this->nodeExists(copyskillNode, "Reproduce")) {
				bool active;

				if (!this->asBool(copyskillNode, "Reproduce", active))
					return 0;

				if (active)
					skill->copyable.option |= SKILL_COPY_REPRODUCE;
				else
					skill->copyable.option &= SKILL_COPY_REPRODUCE;
			}
		} else {
			this->invalidWarning(copyNode, "CopyFlags requires a Skill copy type.\n");
			return 0;
		}

		if (this->nodeExists(copyNode, "RemoveRequirement")) {
			const auto& copyreqNode = copyNode["RemoveRequirement"];

			for (const auto& it : copyreqNode) {
				std::string req;
				c4::from_chars(it.key(), &req);
				std::string req_constant = "SKILL_REQ_" + req;
				int64 constant;

				if (!script_get_constant(req_constant.c_str(), &constant)) {
					this->invalidWarning(copyreqNode, "CopyFlags RemoveRequirement %s is invalid.\n", req.c_str());
					return 0;
				}

				skill->copyable.req_opt |= constant;
			}
		} else {
			if (!exists)
				skill->copyable.req_opt = 0;
		}
	}

	if (this->nodeExists(node, "NoNearNPC")) {
		const auto& npcNode = node["NoNearNPC"];

		if (this->nodeExists(npcNode, "AdditionalRange")) {
			uint16 range;

			if (!this->asUInt16(npcNode, "AdditionalRange", range))
				return 0;

			skill->unit_nonearnpc_range = range;
		} else {
			if (!exists)
				skill->unit_nonearnpc_range = 0;
		}

		if (this->nodeExists(npcNode, "Type")) {
			const auto& npctypeNode = npcNode["Type"];

			for (const auto& it : npctypeNode) {
				std::string type;
				c4::from_chars(it.key(), &type);
				std::string type_constant = "SKILL_NONEAR_" + type;
				int64 constant;

				if (!script_get_constant(type_constant.c_str(), &constant)) {
					this->invalidWarning(npctypeNode, "NoNearNPC Type %s is invalid.\n", type.c_str());
					return 0;
				}

				bool active;

				if (!this->asBool(npctypeNode, type, active))
					return 0;

				if (active)
					skill->unit_nonearnpc_type |= constant;
				else
					skill->unit_nonearnpc_type &= ~constant;
			}
		} else {
			if (!exists)
				skill->unit_nonearnpc_type = 0;
		}
	}

	if (this->nodeExists(node, "CastCancel")) {
		bool active;

		if (!this->asBool(node, "CastCancel", active))
			return 0;

		skill->castcancel = active;
	} else {
		if (!exists)
			skill->castcancel = false;
	}

	if (this->nodeExists(node, "CastDefenseReduction")) {
		uint16 reduction;

		if (!this->asUInt16(node, "CastDefenseReduction", reduction))
			return 0;

		skill->cast_def_rate = reduction;
	} else {
		if (!exists)
			skill->cast_def_rate = 0;
	}

	if (this->nodeExists(node, "CastTime")) {
		if (!this->parseNode("CastTime", "Time", node, skill->cast))
			return 0;
	} else {
		if (!exists)
			memset(skill->cast, 0, sizeof(skill->cast));
	}

	if (this->nodeExists(node, "AfterCastActDelay")) {
		if (!this->parseNode("AfterCastActDelay", "Time", node, skill->delay))
			return 0;
	} else {
		if (!exists)
			memset(skill->delay, 0, sizeof(skill->delay));
	}

	if (this->nodeExists(node, "AfterCastWalkDelay")) {
		if (!this->parseNode("AfterCastWalkDelay", "Time", node, skill->walkdelay))
			return 0;
	} else {
		if (!exists)
			memset(skill->walkdelay, 0, sizeof(skill->walkdelay));
	}

	if (this->nodeExists(node, "Duration1")) {
		if (!this->parseNode("Duration1", "Time", node, skill->upkeep_time))
			return 0;
	} else {
		if (!exists)
			memset(skill->upkeep_time, 0, sizeof(skill->upkeep_time));
	}

	if (this->nodeExists(node, "Duration2")) {
		if (!this->parseNode("Duration2", "Time", node, skill->upkeep_time2))
			return 0;
	} else {
		if (!exists)
			memset(skill->upkeep_time2, 0, sizeof(skill->upkeep_time2));
	}

	if (this->nodeExists(node, "Cooldown")) {
		if (!this->parseNode("Cooldown", "Time", node, skill->cooldown))
			return 0;
	} else {
		if (!exists)
			memset(skill->cooldown, 0, sizeof(skill->cooldown));
	}

#ifdef RENEWAL_CAST
	if (this->nodeExists(node, "FixedCastTime")) {
		if (!this->parseNode("FixedCastTime", "Time", node, skill->fixed_cast))
			return 0;
	} else {
		if (!exists)
			memset(skill->fixed_cast, 0, sizeof(skill->fixed_cast));
	}
#endif

	if (this->nodeExists(node, "CastTimeFlags")) {
		const auto& castNode = node["CastTimeFlags"];

		for (const auto& it : castNode) {
			std::string flag;
			c4::from_chars(it.key(), &flag);
			std::string flag_constant = "SKILL_CAST_" + flag;
			int64 constant;

			if (!script_get_constant(flag_constant.c_str(), &constant)) {
				this->invalidWarning(castNode, "CastTimeFlags %s option is invalid.\n", flag.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(castNode, flag, active))
				return 0;

			if (active)
				skill->castnodex |= constant;
			else
				skill->castnodex &= ~constant;
		}
	}

	if (this->nodeExists(node, "CastDelayFlags")) {
		const auto& castNode = node["CastDelayFlags"];

		for (const auto& it : castNode) {
			std::string flag;
			c4::from_chars(it.key(), &flag);
			std::string flag_constant = "SKILL_CAST_" + flag;
			int64 constant;

			if (!script_get_constant(flag_constant.c_str(), &constant)) {
				this->invalidWarning(castNode, "CastDelayFlags %s option is invalid.\n", flag.c_str());
				return 0;
			}

			bool active;

			if (!this->asBool(castNode, flag, active))
				return 0;

			if (active)
				skill->delaynodex |= constant;
			else
				skill->delaynodex &= ~constant;
		}
	}

	if (this->nodeExists(node, "Requires")) {
		const auto& requireNode = node["Requires"];

		if (this->nodeExists(requireNode, "HpCost")) {
			if (!this->parseNode("HpCost", "Amount", requireNode, skill->require.hp))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.hp, 0, sizeof(skill->require.hp));
		}

		if (this->nodeExists(requireNode, "SpCost")) {
			if (!this->parseNode("SpCost", "Amount", requireNode, skill->require.sp))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.sp, 0, sizeof(skill->require.sp));
		}

		if (this->nodeExists(requireNode, "ApCost")) {
			if (!this->parseNode("ApCost", "Amount", requireNode, skill->require.ap))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.ap, 0, sizeof(skill->require.ap));
		}

		if (this->nodeExists(requireNode, "HpRateCost")) {
			if (!this->parseNode("HpRateCost", "Amount", requireNode, skill->require.hp_rate))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.hp_rate, 0, sizeof(skill->require.hp_rate));
		}

		if (this->nodeExists(requireNode, "SpRateCost")) {
			if (!this->parseNode("SpRateCost", "Amount", requireNode, skill->require.sp_rate))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.sp_rate, 0, sizeof(skill->require.sp_rate));
		}

		if (this->nodeExists(requireNode, "ApRateCost")) {
			if (!this->parseNode("ApRateCost", "Amount", requireNode, skill->require.ap_rate))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.ap_rate, 0, sizeof(skill->require.ap_rate));
		}

		if (this->nodeExists(requireNode, "MaxHpTrigger")) {
			if (!this->parseNode("MaxHpTrigger", "Amount", requireNode, skill->require.mhp))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.mhp, 0, sizeof(skill->require.mhp));
		}

		if (this->nodeExists(requireNode, "ZenyCost")) {
			if (!this->parseNode("ZenyCost", "Amount", requireNode, skill->require.zeny))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.zeny, 0, sizeof(skill->require.zeny));
		}

		if (this->nodeExists(requireNode, "Weapon")) {
			const auto& weaponNode = requireNode["Weapon"];

			if (this->nodeExists(weaponNode, "All")) {
				bool active;

				if (!this->asBool(weaponNode, "All", active))
					return 0;

				if (active)
					skill->require.weapon = 0;
			} else {
				for (const auto& it : weaponNode) {
					std::string weapon;
					c4::from_chars(it.key(), &weapon);
					std::string weapon_constant = "W_" + weapon;
					int64 constant;

					if (!script_get_constant(weapon_constant.c_str(), &constant)) {
						this->invalidWarning(weaponNode, "Requires Weapon %s is invalid.\n", weapon.c_str());
						return 0;
					}

					bool active;

					if (!this->asBool(weaponNode, weapon, active))
						return 0;

					if (active)
						skill->require.weapon |= 1 << constant;
					else
						skill->require.weapon &= ~(1 << constant);
				}
			}
		} else {
			if (!exists)
				skill->require.weapon = 0;
		}

		if (this->nodeExists(requireNode, "Ammo")) {
			const auto& ammoNode = requireNode["Ammo"];

			if (this->nodeExists(ammoNode, "None")) {
				bool active;

				if (!this->asBool(ammoNode, "None", active))
					return 0;

				if (active)
					skill->require.ammo = 0;
			} else {
				for (const auto& it : ammoNode) {
					std::string ammo;
					c4::from_chars(it.key(), &ammo);
					std::string ammo_constant = "AMMO_" + ammo;
					int64 constant;

					if (!script_get_constant(ammo_constant.c_str(), &constant)) {
						this->invalidWarning(ammoNode, "Requires Ammo %s is invalid.\n", ammo.c_str());
						return 0;
					}

					bool active;

					if (!this->asBool(ammoNode, ammo, active))
						return 0;

					if (active)
						skill->require.ammo |= 1 << constant;
					else
						skill->require.ammo &= ~(1 << constant);
				}
			}
		} else {
			if (!exists)
				skill->require.ammo = 0;
		}

		if (this->nodeExists(requireNode, "AmmoAmount")) {
			if (skill->require.ammo == 0) {
				this->invalidWarning(requireNode["AmmoAmount"], "An ammo type is required before specifying ammo amount.\n");
				return 0;
			}

			if (!this->parseNode("AmmoAmount", "Amount", requireNode, skill->require.ammo_qty))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.ammo_qty, 0, sizeof(skill->require.ammo_qty));
		}

		if (this->nodeExists(requireNode, "State")) {
			std::string state;

			if (!this->asString(requireNode, "State", state))
				return 0;

			std::string state_constant = "ST_" + state;
			int64 constant;

			if (!script_get_constant(state_constant.c_str(), &constant)) {
				this->invalidWarning(requireNode["State"], "Requires State %s is invalid.\n", state.c_str());
				return 0;
			}

			skill->require.state = static_cast<int32>(constant);
		}

		if (this->nodeExists(requireNode, "Status")) {
			const auto& statusNode = requireNode["Status"];

			for (const auto& it : statusNode) {
				std::string status;
				c4::from_chars(it.key(), &status);
				std::string status_constant = "SC_" + status;
				int64 constant;

				if (!script_get_constant(status_constant.c_str(), &constant)) {
					this->invalidWarning(statusNode, "Requires Status %s is invalid.\n", status.c_str());
					return 0;
				}

				bool active;

				if (!this->asBool(statusNode, status, active))
					return 0;

				auto status_exists = util::vector_get(skill->require.status, constant);

				if (active && status_exists == skill->require.status.end())
					skill->require.status.push_back(static_cast<sc_type>(constant));
				else if (!active && status_exists != skill->require.status.end())
					skill->require.status.erase(status_exists);
			}
		}

		if (this->nodeExists(requireNode, "SpiritSphereCost")) {
			if (!this->parseNode("SpiritSphereCost", "Amount", requireNode, skill->require.spiritball))
				return 0;
		} else {
			if (!exists)
				memset(skill->require.spiritball, 0, sizeof(skill->require.spiritball));
		}

		if (this->nodeExists(requireNode, "ItemCost")) {
			const auto itemNode = requireNode["ItemCost"];
			int32 count = 0;

			for (const auto& it : itemNode) {
				std::string item_name;

				if (!this->asString(it, "Item", item_name))
					continue;

				std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

				if (item == nullptr) {
					this->invalidWarning(it["Item"], "Requires ItemCost Item %s does not exist.\n", item_name.c_str());
					return 0;
				}

				int32 amount;

				if (!this->asInt32(it, "Amount", amount))
					continue;

				if (this->nodeExists(it, "Level")) {
					uint16 cost_level;

					if (!this->asUInt16(it, "Level", cost_level))
						continue;

					if (cost_level < 1 || cost_level > skill->max) {
						this->invalidWarning(it["Level"], "Requires ItemCost Level %d is not within %s's level range of 1~%d.\n", cost_level, skill->name, skill->max);
						return 0;
					}

					count = cost_level - 1;

					if (!skill->require.itemid_level_dependent)
						skill->require.itemid_level_dependent = true;
				}

				skill->require.itemid[count] = item->nameid;
				skill->require.amount[count] = amount;
				count++;
			}
		}

		if (this->nodeExists(requireNode, "Equipment")) {
			const auto& equipNode = requireNode["Equipment"];

			for (const auto& it : equipNode) {
				std::string item_name;
				c4::from_chars(it.key(), &item_name);

				std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

				if (item == nullptr) {
					this->invalidWarning(it, "Requires Equipment %s does not exist.\n", item_name.c_str());
					return 0;
				}

				bool active;

				if (!this->asBool(equipNode, item_name, active))
					return 0;

				auto equip_exists = util::vector_get(skill->require.eqItem, item->nameid);

				if (active && equip_exists == skill->require.eqItem.end())
					skill->require.eqItem.push_back(item->nameid);
				else if (!active && equip_exists != skill->require.eqItem.end())
					skill->require.eqItem.erase(equip_exists);
			}
		}
	}

	if (this->nodeExists(node, "GiveAp")) {
		if (!this->parseNode("GiveAp", "Amount", node, skill->giveap))
			return 0;
	} else {
		if (!exists)
			memset(skill->giveap, 0, sizeof(skill->giveap));
	}

	if (this->nodeExists(node, "Unit")) {
		const auto& unitNode = node["Unit"];

		if (this->nodeExists(unitNode, "Id")) {
			std::string unit;

			if (!this->asString(unitNode, "Id", unit))
				return 0;

			std::string unit_constant = "UNT_" + unit;
			int64 constant;

			if (!script_get_constant(unit_constant.c_str(), &constant)) {
				this->invalidWarning(unitNode["Id"], "Unit Id %s is invalid.\n", unit.c_str());
				return 0;
			}

			skill->unit_id = static_cast<uint16>(constant);
		} else {
			this->invalidWarning(unitNode["Id"], "Unit requires an Id.\n");
			return 0;
		}

		if (this->nodeExists(unitNode, "AlternateId")) {
			std::string unit;

			if (!this->asString(unitNode, "AlternateId", unit))
				return 0;

			std::string unit_constant = "UNT_" + unit;
			int64 constant;

			if (!script_get_constant(unit_constant.c_str(), &constant)) {
				this->invalidWarning(unitNode["AlternateId"], "Alternate Unit Id %s is invalid.\n", unit.c_str());
				return 0;
			}


			skill->unit_id2 = static_cast<uint16>(constant);
		} else {
			if (!exists)
				skill->unit_id2 = 0;
		}

		if (this->nodeExists(unitNode, "Layout")) {
			if (!this->parseNode("Layout", "Size", unitNode, skill->unit_layout_type))
				return 0;
		} else {
			if (!exists)
				memset(skill->unit_layout_type, 0, sizeof(skill->unit_layout_type));
		}

		if (this->nodeExists(unitNode, "Range")) {
			if (!this->parseNode("Range", "Size", unitNode, skill->unit_range))
				return 0;
		} else {
			if (!exists)
				memset(skill->unit_range, 0, sizeof(skill->unit_range));
		}

		if (this->nodeExists(unitNode, "Interval")) {
			int16 interval;

			if (!this->asInt16(unitNode, "Interval", interval))
				return 0;

			skill->unit_interval = interval;
		} else {
			if (!exists)
				skill->unit_interval = 0;
		}

		if (this->nodeExists(unitNode, "Target")) {
			std::string target;

			if (!this->asString(unitNode, "Target", target))
				return 0;

			std::string target_constant = "BCT_" + target;
			int64 constant;

			if (!script_get_constant(target_constant.c_str(), &constant)) {
				this->invalidWarning(unitNode["Target"], "Unit Target %s is invalid.\n", target.c_str());
				return 0;
			}

			skill->unit_target = static_cast<int32>(constant);
		} else {
			if (!exists)
				skill->unit_target = BCT_ALL;
		}

		if (this->nodeExists(unitNode, "Flag")) {
			const auto& flagNode = unitNode["Flag"];

			for (const auto& it : flagNode) {
				std::string flag;
				c4::from_chars(it.key(), &flag);
				std::string flag_constant = "UF_" + flag;
				int64 constant;

				if (!script_get_constant(flag_constant.c_str(), &constant)) {
					this->invalidWarning(it, "Skill Unit Flag %s is invalid.\n", flag.c_str());
					return 0;
				}

				bool active;

				if (!this->asBool(flagNode, flag, active))
					return 0;

				if (active)
					skill->unit_flag.set(static_cast<uint8>(constant));
				else
					skill->unit_flag.reset(static_cast<uint8>(constant));
			}

			if (skill->unit_flag[UF_NOENEMY] && battle_config.defnotenemy)
				skill->unit_target = BCT_NOENEMY;

			// By default, target just characters.
			skill->unit_target |= BL_CHAR;
			if (skill->unit_flag[UF_NOPC])
				skill->unit_target &= ~BL_PC;
			if (skill->unit_flag[UF_NOMOB])
				skill->unit_target &= ~BL_MOB;
			if (skill->unit_flag[UF_SKILL])
				skill->unit_target |= BL_SKILL;
		} else {
			if (!exists){
				skill->unit_flag = UF_NONE;
				// By default, target just characters.
				skill->unit_target |= BL_CHAR;
			}
		}
	}

	if (this->nodeExists(node, "Status")) {
		std::string status;

		if (!this->asString(node, "Status", status))
			return 0;

		std::string status_constant = "SC_" + status;
		int64 constant;

		if (!script_get_constant(status_constant.c_str(), &constant)) {
			this->invalidWarning(node["Status"], "Status %s is invalid.\n", status.c_str());
			return 0;
		}

		if (constant < SC_NONE || constant >= SC_MAX) {
			this->invalidWarning(node["Status"], "Status %s is unknown. Defaulting to SC_NONE.\n", status.c_str());
			constant = SC_NONE;
		}

		skill->sc = static_cast<sc_type>(constant);
	} else {
		if (!exists)
			skill->sc = SC_NONE;
	}

	if (!exists) {
		this->put(skill_id, skill);
		this->skilldb_id2idx[skill_id] = this->skill_num;
		this->skill_num++;
	}

	return 1;
}

void SkillDatabase::clear() {
	TypesafeCachedYamlDatabase::clear();
	memset( this->skilldb_id2idx, 0, sizeof( this->skilldb_id2idx ) );
	this->skill_num = 1;
}

void SkillDatabase::loadingFinished(){
	if( this->skill_num > MAX_SKILL ){
		ShowError( "There are more skills defined in the skill database (%d) than the MAX_SKILL (%d) define. Please increase it and recompile.\n", this->skill_num, MAX_SKILL );
	}

	TypesafeCachedYamlDatabase::loadingFinished();
}

/**
 * Get skill index from skill_db array. The index is also being used for skill lookup in mmo_charstatus::skill[]
 * @param skill_id
 * @param silent If Skill is undefined, show error message!
 * @return Skill Index or 0 if not found/unset
 **/
uint16 SkillDatabase::get_index( uint16 skill_id, bool silent, const char *func, const char *file, int line ){
	uint16 idx = this->skilldb_id2idx[skill_id];

	if( idx == 0 && skill_id != 0 && !silent ){
		ShowError( "Skill '%d' is undefined! %s:%d::%s\n", skill_id, file, line, func );
	}

	return idx;
}

SkillDatabase skill_db;

const std::string ReadingSpellbookDatabase::getDefaultLocation() {
	return std::string(db_path) + "/spellbook_db.yml";
}
/**
 * Reads and parses an entry from the spellbook_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 ReadingSpellbookDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string skill_name;

	if (!this->asString(node, "Skill", skill_name))
		return 0;

	uint16 skill_id = skill_name2id(skill_name.c_str());

	if (skill_id == 0) {
		this->invalidWarning(node["Skill"], "Invalid skill name \"%s\", skipping.\n", skill_name.c_str());
		return 0;
	}

	if (skill_get_inf(skill_id) ==	INF_PASSIVE_SKILL) {
		this->invalidWarning(node["Skill"], "Passive skill %s cannot be memorized in a Spell Book.\n", skill_name.c_str());
		return 0;
	}

	std::shared_ptr<s_skill_spellbook_db> spell = this->find(skill_id);
	bool exists = spell != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Book", "PreservePoints" }))
			return 0;

		spell = std::make_shared<s_skill_spellbook_db>();
		spell->skill_id = skill_id;
	}

	if (this->nodeExists(node, "Book")) {
		std::string book_name;

		if (!this->asString(node, "Book", book_name))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( book_name.c_str() );

		if (item == nullptr) {
			this->invalidWarning(node["Book"], "Book item %s does not exist.\n", book_name.c_str());
			return 0;
		}

		spell->nameid = item->nameid;
	}

	if (this->nodeExists(node, "PreservePoints")) {
		uint16 points;

		if (!this->asUInt16(node, "PreservePoints", points))
			return 0;

		spell->points = points;
	}

	if (!exists)
		this->put(skill_id, spell);

	return 1;
}

/**
 * Check if the specified item is available in the spellbook_db or not
 * @param nameid: Book Item ID
 * @return Spell data or nullptr otherwise
 */
std::shared_ptr<s_skill_spellbook_db> ReadingSpellbookDatabase::findBook(t_itemid nameid) {
	if (!item_db.exists(nameid) || reading_spellbook_db.empty())
		return nullptr;

	for (const auto &spell : reading_spellbook_db) {
		if (spell.second->nameid == nameid)
			return spell.second;
	}

	return nullptr;
}

const std::string MagicMushroomDatabase::getDefaultLocation() {
	return std::string(db_path) + "/magicmushroom_db.yml";
}

/**
 * Reads and parses an entry from the magicmushroom_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 MagicMushroomDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string skill_name;

	if (!this->asString(node, "Skill", skill_name))
		return 0;

	uint16 skill_id = skill_name2id(skill_name.c_str());

	if (!skill_id) {
		this->invalidWarning(node["Skill"], "Invalid Magic Mushroom skill name \"%s\", skipping.\n", skill_name.c_str());
		return 0;
	}

	if (!skill_get_inf(skill_id)) {
		this->invalidWarning(node["Skill"], "Passive skill %s cannot be casted by Magic Mushroom.\n", skill_name.c_str());
		return 0;
	}

	std::shared_ptr<s_skill_magicmushroom_db> mushroom = this->find(skill_id);
	bool exists = mushroom != nullptr;

	if (!exists) {
		mushroom = std::make_shared<s_skill_magicmushroom_db>();
		mushroom->skill_id = skill_id;

		this->put(skill_id, mushroom);
	}

	return 1;
}

/** Reads skill no cast db
 * Structure: SkillID,Flag
 */
static bool skill_parse_row_nocastdb( char* split[], size_t columns, size_t current ){
	std::shared_ptr<s_skill_db> skill = skill_db.find(atoi(split[0]));

	if (!skill)
		return false;

	skill->nocast |= atoi(split[1]);

	return true;
}

/** Reads Produce db
 * Structure: ProduceItemID,ItemLV,RequireSkill,Requireskill_lv,MaterialID1,MaterialAmount1,...
 */
static bool skill_parse_row_producedb( char* split[], size_t columns, size_t current ){
	unsigned short x, y;
	unsigned short id = atoi(split[0]);
	t_itemid nameid = 0;
	bool found = false;

	if (id >= ARRAYLENGTH(skill_produce_db)) {
		ShowError("skill_parse_row_producedb: Maximum db entries reached.\n");
		return false;
	}

	// Clear previous data, for importing support
	memset(&skill_produce_db[id], 0, sizeof(skill_produce_db[id]));
	// Import just for clearing/disabling from original data
	if (!(nameid = strtoul(split[1], nullptr, 10))) {
		//ShowInfo("skill_parse_row_producedb: Product list with ID %d removed from list.\n", id);
		return true;
	}

	if (!item_db.exists(nameid)) {
		ShowError("skill_parse_row_producedb: Invalid item %u.\n", nameid);
		return false;
	}

	skill_produce_db[id].nameid = nameid;
	skill_produce_db[id].itemlv = atoi(split[2]);
	skill_produce_db[id].req_skill = atoi(split[3]);
	skill_produce_db[id].req_skill_lv = atoi(split[4]);

	for (x = 5, y = 0; x+1 < columns && split[x] && split[x+1] && y < MAX_PRODUCE_RESOURCE; x += 2, y++) {
		skill_produce_db[id].mat_id[y] = strtoul(split[x], nullptr, 10);
		skill_produce_db[id].mat_amount[y] = atoi(split[x+1]);
	}

	if (!found)
		skill_produce_count++;

	return true;
}

const std::string SkillArrowDatabase::getDefaultLocation() {
	return std::string(db_path) + "/create_arrow_db.yml";
}

/**
 * Reads and parses an entry from the create_arrow_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 SkillArrowDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string source_name;

	if (!this->asString(node, "Source", source_name))
		return 0;

	std::shared_ptr<item_data> item = item_db.search_aegisname( source_name.c_str() );

	if (item == nullptr) {
		this->invalidWarning(node["Source"], "Item %s does not exist.\n", source_name.c_str());
		return 0;
	}

	t_itemid nameid = item->nameid;

	std::shared_ptr<s_skill_arrow_db> arrow = this->find(nameid);
	bool exists = arrow != nullptr;

	if (!exists) {
		arrow = std::make_shared<s_skill_arrow_db>();
		arrow->nameid = nameid;
	}

	const auto& MakeNode = node["Make"];

	for (const auto &it : MakeNode) {
		std::string item_name;

		if (!this->asString(it, "Item", item_name))
			return 0;

		std::shared_ptr<item_data> item = item_db.search_aegisname( item_name.c_str() );

		if (item == nullptr) {
			this->invalidWarning(it["Item"], "Item %s does not exist.\n", item_name.c_str());
			return 0;
		}

		uint16 amount;

		if (!this->asUInt16(it, "Amount", amount))
			return 0;

		if (amount == 0) {
			if (arrow->created.erase(item->nameid) == 0)
				this->invalidWarning(it["Amount"], "Failed to remove %s, the entry doesn't exist in Source %s.\n", item_name.c_str(), source_name.c_str());
			continue;
		}
		if (amount > MAX_AMOUNT) {
			this->invalidWarning(it["Amount"], "Amount %hu exceeds %hu, skipping.\n", amount, MAX_AMOUNT);
			continue;
		}

		arrow->created[item->nameid] = amount;
	}

	if (!exists)
		this->put(nameid, arrow);

	return 1;
}

const std::string AbraDatabase::getDefaultLocation() {
	return std::string(db_path) + "/abra_db.yml";
}

/**
 * Reads and parses an entry from the abra_db.
 * @param node: YAML node containing the entry.
 * @return count of successfully parsed rows
 */
uint64 AbraDatabase::parseBodyNode(const ryml::NodeRef& node) {
	std::string skill_name;

	if (!this->asString(node, "Skill", skill_name))
		return 0;

	uint16 skill_id = skill_name2id(skill_name.c_str());

	if (!skill_id) {
		this->invalidWarning(node["Skill"], "Invalid Abra skill name \"%s\", skipping.\n", skill_name.c_str());
		return 0;
	}

	if (!skill_get_inf(skill_id)) {
		this->invalidWarning(node["Skill"], "Passive skill %s cannot be casted by Abra.\n", skill_name.c_str());
		return 0;
	}

	std::shared_ptr<s_skill_abra_db> abra = this->find(skill_id);
	bool exists = abra != nullptr;

	if (!exists) {
		abra = std::make_shared<s_skill_abra_db>();
		abra->skill_id = skill_id;
	}

	if (this->nodeExists(node, "Probability")) {
		const auto& probNode = node["Probability"];
		uint16 probability;

		if (!probNode.is_seq()) {
			if (!this->asUInt16Rate(probNode, "Probability", probability))
				return 0;

			abra->per.fill(probability);
		} else {
			abra->per.fill(0);

			for (const auto& it : probNode) {
				uint16 skill_lv;

				if (!this->asUInt16(it, "Level", skill_lv))
					continue;

				if (skill_lv > MAX_SKILL_LEVEL) {
					this->invalidWarning(probNode["Level"], "Probability Level exceeds the maximum skill level of %d, skipping.\n", MAX_SKILL_LEVEL);
					return 0;
				}

				if (!this->asUInt16Rate(it, "Probability", probability))
					continue;

				abra->per[skill_lv - 1] = probability;
			}
		}
	} else {
		if (!exists)
			abra->per.fill(500);
	}

	if (!exists)
		this->put(skill_id, abra);

	return 1;
}

/** Reads change material db
 * Structure: ProductID,BaseRate,MakeAmount1,MakeAmountRate1...,MakeAmount5,MakeAmountRate5
 */
static bool skill_parse_row_changematerialdb( char* split[], size_t columns, size_t current ){
	uint16 id = atoi(split[0]);
	t_itemid nameid = strtoul(split[1], nullptr, 10);
	short rate = atoi(split[2]);
	bool found = false;
	int x, y;

	if (id >= MAX_SKILL_CHANGEMATERIAL_DB) {
		ShowError("skill_parse_row_changematerialdb: Maximum amount of entries reached (%d), increase MAX_SKILL_CHANGEMATERIAL_DB\n",MAX_SKILL_CHANGEMATERIAL_DB);
		return false;
	}

	// Clear previous data, for importing support
	if (skill_changematerial_db[id].nameid > 0) {
		found = true;
		memset(&skill_changematerial_db[id], 0, sizeof(skill_changematerial_db[id]));
	}

	// Import just for clearing/disabling from original data
	// NOTE: If import for disabling, better disable list from produce_db instead of here, or creation just failed with deleting requirements.
	if (nameid == 0) {
		memset(&skill_changematerial_db[id], 0, sizeof(skill_changematerial_db[id]));
		//ShowInfo("skill_parse_row_changematerialdb: Change Material list with ID %d removed from list.\n", id);
		return true;
	}

	// Entry must be exists in skill_produce_db and with required skill GN_CHANGEMATERIAL
	for (x = 0; x < MAX_SKILL_PRODUCE_DB; x++) {
		if (skill_produce_db[x].nameid == nameid)
			if( skill_produce_db[x].req_skill == GN_CHANGEMATERIAL )
				break;
	}

	if (x >= MAX_SKILL_PRODUCE_DB) {
		ShowError("skill_parse_row_changematerialdb: Not supported item ID (%u) for Change Material. \n", nameid);
		return false;
	}

	skill_changematerial_db[id].nameid = nameid;
	skill_changematerial_db[id].rate = rate;

	for (x = 3, y = 0; x+1 < columns && split[x] && split[x+1] && y < MAX_SKILL_CHANGEMATERIAL_SET; x += 2, y++) {
		skill_changematerial_db[id].qty[y] = atoi(split[x]);
		skill_changematerial_db[id].qty_rate[y] = atoi(split[x+1]);
	}

	if (!found)
		skill_changematerial_count++;

	return true;
}

/**
 * Reads skill damage adjustment
 * @author [Lilith]
 */
static bool skill_parse_row_skilldamage( char* split[], size_t columns, size_t current ){
	int64 caster_tmp;
	uint16 id;
	int caster, value;
	char *result;

	trim(split[0]);
	if (ISDIGIT(split[0][0])) {
		value = strtol(split[0], &result, 10);

		if (*result) {
			ShowError("skill_parse_row_skilldamage: Invalid skill %s given, skipping.\n", result);
			return false;
		}

		id = value;
	} else
		id = skill_name2id(split[0]);

	std::shared_ptr<s_skill_db> skill = skill_db.find(id);

	if (!skill)
		return false;

	skill->damage = {};
	trim(split[1]);
	if (ISDIGIT(split[1][0])) {
		value = strtol(split[1], &result, 10);

		if (*result) {
			ShowError("skill_parse_row_skilldamage: Invalid caster %s given for skill %d, skipping.\n", result, id);
			return false;
		}

		caster = value;
	} else { // Try to parse caster as constant
		if (!script_get_constant(split[1], &caster_tmp)) {
			ShowError("skill_parse_row_skilldamage: Invalid caster constant given for skill %d, skipping.\n", id);
			return false;
		}
		caster = static_cast<uint16>(caster_tmp);
	}
	skill->damage.caster |= caster;

	value = strtol(split[2], &result, 10);

	if (*result) {
		ShowError("skill_parse_row_skilldamage: Invalid map %s given for skill %d, skipping.\n", result, id);
		return false;
	}

	skill->damage.map |= value;

	for(int offset = 3, i = SKILLDMG_PC; i < SKILLDMG_MAX && offset < columns; i++, offset++ ){
		value = strtol(split[offset], &result, 10);

		if (*result && *result != ' ') {
			ShowError("skill_parse_row_skilldamage: Invalid damage %s given for skill %d, defaulting to 0.\n", result, id);
			value = 0;
		}
		skill->damage.rate[i] = cap_value(value, -100, 100000);
	}

	return true;
}

/** Reads skill database files */
static void skill_readdb(void) {
	int i;
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
		//add other path here
	};

	memset(skill_produce_db,0,sizeof(skill_produce_db));
	memset(skill_changematerial_db,0,sizeof(skill_changematerial_db));
	skill_produce_count = skill_changematerial_count = 0;

	skill_db.load();

	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		size_t n1 = strlen(db_path)+strlen(dbsubpath[i])+1;
		size_t n2 = strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1;
		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);

		if (i == 0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		} else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}

		sv_readdb(dbsubpath2, "skill_nocast_db.txt"   , ',',   2,  2, -1, skill_parse_row_nocastdb, i > 0);

		sv_readdb(dbsubpath2, "produce_db.txt"        , ',',   5,  5+2*MAX_PRODUCE_RESOURCE, MAX_SKILL_PRODUCE_DB, skill_parse_row_producedb, i > 0);
		sv_readdb(dbsubpath1, "skill_changematerial_db.txt" , ',',   5,  5+2*MAX_SKILL_CHANGEMATERIAL_SET, MAX_SKILL_CHANGEMATERIAL_DB, skill_parse_row_changematerialdb, i > 0);
		sv_readdb(dbsubpath1, "skill_damage_db.txt"         , ',',   4,  3+SKILLDMG_MAX, -1, skill_parse_row_skilldamage, i > 0);

		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}

	abra_db.load();
	magic_mushroom_db.load();
	reading_spellbook_db.load();
	skill_arrow_db.load();

	skill_init_unit_layout();
	skill_init_nounit_layout();
}

void skill_reload (void) {
	skill_db.clear();
	abra_db.clear();
	magic_mushroom_db.clear();
	reading_spellbook_db.clear();
	skill_arrow_db.clear();

	skill_readdb();

	/* lets update all players skill tree : so that if any skill modes were changed they're properly updated */
	s_mapiterator *iter = mapit_getallusers();

	for( map_session_data *sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) ) {
		pc_validate_skill(sd);
		clif_skillinfoblock(sd);
	}
	mapit_free(iter);
}

/*==========================================
 *
 *------------------------------------------*/
void do_init_skill(void)
{
	skill_readdb();

	skillunit_db = idb_alloc(DB_OPT_BASE);
	skillusave_db = idb_alloc(DB_OPT_RELEASE_DATA);
	bowling_db = idb_alloc(DB_OPT_BASE);
	skill_timer_ers  = ers_new(sizeof(struct skill_timerskill),"skill.cpp::skill_timer_ers",ERS_CACHE_OPTIONS);

	ers_chunk_size(skill_timer_ers, 150);

	add_timer_func_list(skill_unit_timer,"skill_unit_timer");
	add_timer_func_list(skill_castend_id,"skill_castend_id");
	add_timer_func_list(skill_castend_pos,"skill_castend_pos");
	add_timer_func_list(skill_timerskill,"skill_timerskill");
	add_timer_func_list(skill_blockpc_end, "skill_blockpc_end");
	add_timer_func_list(skill_keep_using, "skill_keep_using");

	add_timer_interval(gettick()+SKILLUNITTIMER_INTERVAL,skill_unit_timer,0,0,SKILLUNITTIMER_INTERVAL);
}

void do_final_skill(void)
{
	skill_db.clear();
	abra_db.clear();
	magic_mushroom_db.clear();
	reading_spellbook_db.clear();
	skill_arrow_db.clear();

	db_destroy(skillunit_db);
	db_destroy(skillusave_db);
	db_destroy(bowling_db);
	ers_destroy(skill_timer_ers);
}
