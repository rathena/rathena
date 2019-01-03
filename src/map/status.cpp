// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "status.hpp"

#include <functional>
#include <math.h>
#include <stdlib.h>
#include <string>
#include <yaml-cpp/yaml.h>

#include "../common/cbasetypes.hpp"
#include "../common/ers.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utils.hpp"

#include "battle.hpp"
#include "battleground.hpp"
#include "clif.hpp"
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "itemdb.hpp"
#include "map.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"
#include "script.hpp"

// Regen related flags.
enum e_regen {
	RGN_NONE = 0x00,
	RGN_HP   = 0x01,
	RGN_SP   = 0x02,
	RGN_SHP  = 0x04,
	RGN_SSP  = 0x08,
};

// Bonus values and upgrade chances for refining equipment
static struct {
	int chance[REFINE_CHANCE_TYPE_MAX][MAX_REFINE]; /// Success chance
	int bonus[MAX_REFINE]; /// Cumulative fixed bonus damage
	int randombonus_max[MAX_REFINE]; /// Cumulative maximum random bonus damage
	struct refine_cost cost[REFINE_COST_MAX];
} refine_info[REFINE_TYPE_MAX];

static int atkmods[SZ_ALL][MAX_WEAPON_TYPE];	/// ATK weapon modification for size (size_fix.txt)

static struct eri *sc_data_ers; /// For sc_data entries
static struct status_data dummy_status;

short current_equip_item_index; /// Contains inventory index of an equipped item. To pass it into the EQUP_SCRIPT [Lupus]
unsigned int current_equip_combo_pos; /// For combo items we need to save the position of all involved items here
int current_equip_card_id; /// To prevent card-stacking (from jA) [Skotlex]
bool running_npc_stat_calc_event; /// Indicate if OnPCStatCalcEvent is running.
// We need it for new cards 15 Feb 2005, to check if the combo cards are insrerted into the CURRENT weapon only to avoid cards exploits
short current_equip_opt_index; /// Contains random option index of an equipped item. [Secret]

/// Struct of SC configs [Cydh]
struct s_status_change_db {
	enum sc_type type;			///< SC_
	enum efst_type icon;		///< EFST_
	uint64 state;				///< SCS_
	uint64 calc_flag;			///< SCB_ flags
	uint16 opt1;				///< OPT1_
	uint16 opt2;				///< OPT2_
	uint64 opt3;				///< OPT1_
	uint64 look;				///, OPTION_ Changelook
	uint64 flag;				///< SCF_ Flags, enum e_status_change_flag
	bool display;				///< Display status effect/icon (for certain state)
	uint16 skill_id;			///< Associated skill for (addeff) duration lookups
	std::vector<sc_type> end;	///< List of SC that will be ended when this SC is activated
	std::vector<sc_type> fail;	///< List of SC that causing this SC cannot be activated
	bool end_return;			///< After SC ends the SC from end list, it does nothing
	struct script_code *script;	///< Executes this script when SC is active
	t_tick min_duration;		///< Minimum duration effect (after all status reduction)
	uint16 min_rate;			///< Minimum rate to be applied (after all status reduction)
	uint32 disabledon;			///< SC disabled on map zones

	~s_status_change_db()
	{
		if (script)
			script_free_code(script);
	}
};

/// Status Change DB
std::unordered_map<int, std::shared_ptr<s_status_change_db>> statuses;

/// Determine who will receive a clif_status_change packet for effects that require one to display correctly
static uint16 StatusRelevantBLTypes[EFST_MAX];

/**
 * Validates if type is in SC ranges
 * @param type SC type
 * @return True if type is in range, false otherwise
 **/
#define CHK_SC(type) ( ((type) > SC_NONE && (type) < SC_MAX) )

/**
 * Check if SC has entry in statuses
 * @param type SC Type
 * @return Entry of sc data in statuses
 **/
bool status_sc_exists(enum sc_type type)
{
	return statuses.find(type) != statuses.end();
}

/**
 * Return a status by type
 */
std::shared_ptr<s_status_change_db> &status_get(enum sc_type type)
{
	return statuses[type];
}

/**
 * Get SC name (Debug Only)
 * @param type
 * @return SC name or UNKNOWN_SC if not found
 **/
//static const char *status_sc_get_name(enum sc_type type) {
//	struct s_status_change_db *sc = status_sc_exists(type);
//	if (!sc || !sc->name || !StringBuf_Length(sc->name))
//		return "UNKNOWN_SC";
//	return StringBuf_Value(sc->name);
//}

static unsigned short status_calc_str(struct block_list *,struct status_change *,int);
static unsigned short status_calc_agi(struct block_list *,struct status_change *,int);
static unsigned short status_calc_vit(struct block_list *,struct status_change *,int);
static unsigned short status_calc_int(struct block_list *,struct status_change *,int);
static unsigned short status_calc_dex(struct block_list *,struct status_change *,int);
static unsigned short status_calc_luk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_batk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_watk(struct block_list *,struct status_change *,int);
static unsigned short status_calc_matk(struct block_list *,struct status_change *,int);
static signed short status_calc_hit(struct block_list *,struct status_change *,int);
static signed short status_calc_critical(struct block_list *,struct status_change *,int);
static signed short status_calc_flee(struct block_list *,struct status_change *,int);
static signed short status_calc_flee2(struct block_list *,struct status_change *,int);
static defType status_calc_def(struct block_list *bl, struct status_change *sc, int);
static signed short status_calc_def2(struct block_list *,struct status_change *,int);
static defType status_calc_mdef(struct block_list *bl, struct status_change *sc, int);
static signed short status_calc_mdef2(struct block_list *,struct status_change *,int);
static unsigned short status_calc_speed(struct block_list *,struct status_change *,int);
static short status_calc_aspd_rate(struct block_list *,struct status_change *,int);
static unsigned short status_calc_dmotion(struct block_list *bl, struct status_change *sc, int dmotion);
#ifdef RENEWAL_ASPD
static short status_calc_aspd(struct block_list *bl, struct status_change *sc, bool fixed);
#endif
static short status_calc_fix_aspd(struct block_list *bl, struct status_change *sc, int);
static unsigned int status_calc_maxhp(struct block_list *bl, uint64 maxhp);
static unsigned int status_calc_maxsp(struct block_list *bl, uint64 maxsp);
static unsigned char status_calc_element(struct block_list *bl, struct status_change *sc, int element);
static unsigned char status_calc_element_lv(struct block_list *bl, struct status_change *sc, int lv);
static enum e_mode status_calc_mode(struct block_list *bl, struct status_change *sc, enum e_mode mode);
#ifdef RENEWAL
static unsigned short status_calc_ematk(struct block_list *,struct status_change *,int);
#endif
static int status_get_hpbonus(struct block_list *bl, enum e_status_bonus type);
static int status_get_spbonus(struct block_list *bl, enum e_status_bonus type);
static unsigned int status_calc_maxhpsp_pc(struct map_session_data* sd, unsigned int stat, bool isHP);
static int status_get_sc_interval(enum sc_type type);

static bool status_change_isDisabledOnMap_(std::shared_ptr<s_status_change_db> scdb, bool mapIsVS, bool mapIsPVP, bool mapIsGVG, bool mapIsBG, unsigned int mapZone, bool mapIsTE);
#define status_change_isDisabledOnMap(type, m) ( status_change_isDisabledOnMap_(status_get(type), mapdata_flag_vs2((m)), m->flag[MF_PVP] != 0, mapdata_flag_gvg2_no_te((m)), m->flag[MF_BATTLEGROUND] != 0, (m->zone << 3) != 0, mapdata_flag_gvg2_te((m))) )

/**
 * Validates the variable of sc data by 'type'
 * @param type SC type
 * @param var Variable of s_status_change_db
 * @param ret Returns this value if the data is not found
 * @return True value of 'var'
 **/
#define CHK_SC2(type, var, ret) {\
	auto scdb = status_get((type));\
	if (!scdb)\
		return (ret);\
	return (scdb)->var;\
}

/**
 * Get icon id of SC
 * @param type: SC type
 * @return EFST ID
 **/
enum efst_type status_sc_get_icon(enum sc_type type) { CHK_SC2(type, icon, EFST_BLANK); }

/**
 * Get state flags of SC (SCS value)
 * @param type: SC type
 * @return state: State value
 **/
uint64 status_sc_get_state(enum sc_type type) { CHK_SC2(type, state, SCS_NONE); }

/**
 * Get flag of SC (SCB value) for status_calc_ flag
 * @param type: SC type
 * @return cal_flag: Calc value 
 **/
uint64 status_sc_get_calc_flag(enum sc_type type) { CHK_SC2(type, calc_flag, SCB_NONE); }

/**
 * Get Opt1 of SC
 * @param type: SC type
 * @return opt1: OPT1 value
 **/
uint16 status_sc_get_opt1(enum sc_type type) { CHK_SC2(type, opt1, OPT1_NONE); }

/**
 * Get Opt2 of SC
 * @param type: SC type
 * @return opt2: OPT2 value
 **/
uint16 status_sc_get_opt2(enum sc_type type) { CHK_SC2(type, opt2, OPT2_NONE); }

/**
 * Get Opt3 of SC
 * @param type: SC type
 * @return opt3: OPT3 value
 **/
uint64 status_sc_get_opt3(enum sc_type type) { CHK_SC2(type, opt3, OPT3_NORMAL); }

/**
 * Get Option look of SC
 * @param type: SC type
 * @return look: OPTION_ value
 **/
uint64 status_sc_get_look(enum sc_type type) { CHK_SC2(type, look, OPTION_NOTHING); }

/**
 * Get SC's option flags (SCO value)
 * @param type: SC type
 * @return flags: Option flags for SC
 **/
uint64 status_sc_get_flag(enum sc_type type) { CHK_SC2(type, flag, SCB_NONE); }

/**
 * Get SC's minimum duration after sc_def calculation
 * @param type: SC type
 * @return minimum duration in ms
 **/
t_tick status_sc_get_min_duration(enum sc_type type) { CHK_SC2(type, min_duration, 0); }

/**
 * Get SC's minimum rate after sc_def calculation
 * @param type: SC type
 * @return minimum rate 0 ~ 10000
 **/
uint32 status_sc_get_min_rate(enum sc_type type) { CHK_SC2(type, min_rate, 0); }

/**
 * Get SC's attached script
 * @param type: SC type
 * @return script code
 **/
struct script_code *status_sc_get_script(enum sc_type type) { CHK_SC2(type, script, NULL); }

/**
 * Will the SC stop its init after ends other SC in end_list?
 * @param sc: SC type
 * @return True: Stop, False: Continue
 **/
bool status_sc_get_end_return(enum sc_type type) { CHK_SC2(type, end_return, false); }

/**
 * Get SC's END list
 * @param sc: SC type
 * @return End list
 **/
std::vector<sc_type> status_sc_get_end_list(enum sc_type type) {
	auto sc = status_get(type);

	if (!sc || sc->end.empty())
		return {};

	return sc->end;
}

/**
 * Get SC's FAIL list
 * @param sc: SC type
 * @return Fail list
 **/
std::vector<sc_type> status_sc_get_fail_list(enum sc_type type) {
	auto sc = status_get(type);

	if (!sc || sc->fail.empty())
		return {};

	return sc->fail;
}

/**
 * Get BL type to display proper effect
 * @param efst: EFST type
 * @return BL types
 **/
uint16 status_efst_get_bl_type(enum efst_type efst) {
	if (efst <= EFST_BLANK || efst >= EFST_MAX)
		return BL_PC;
	return StatusRelevantBLTypes[efst];
}

/**
 * Returns the FIRST skill (in order of definition in initChangeTables) to use a given status change.
 * Utilized for various duration lookups. Use with caution!
 * @param sc The status to look up
 * @return A skill associated with the status
 **/
uint16 status_sc_get_skill(enum sc_type sc_type) {
	if (sc_type <= SC_NONE || sc_type >= SC_MAX) {
		ShowError("status_sc_get_skill: Unsupported status change id %d\n", sc_type);
		return 0;
	}

	CHK_SC2(sc_type, skill_id, 0);
}

/** Creates dummy status */
static void initDummyData(void) {
	memset(&dummy_status, 0, sizeof(dummy_status));
	dummy_status.hp =
	dummy_status.max_hp =
	dummy_status.max_sp =
	dummy_status.str =
	dummy_status.agi =
	dummy_status.vit =
	dummy_status.int_ =
	dummy_status.dex =
	dummy_status.luk =
	dummy_status.hit = 1;
	dummy_status.speed = 2000;
	dummy_status.adelay = 4000;
	dummy_status.amotion = 2000;
	dummy_status.dmotion = 2000;
	dummy_status.ele_lv = 1; // Min elemental level.
	dummy_status.mode = MD_CANMOVE;
}

/**
 * For copying a status_data structure from b to a, without overwriting current Hp and Sp
 * @param a: Status data structure to copy from
 * @param b: Status data structure to copy to
 */
static inline void status_cpy(struct status_data* a, const struct status_data* b)
{
	memcpy((void*)&a->max_hp, (const void*)&b->max_hp, sizeof(struct status_data)-(sizeof(a->hp)+sizeof(a->sp)));
}

/**
 * Sets HP to a given value
 * Will always succeed (overrides heal impediment statuses) but can't kill an object
 * @param bl: Object whose HP will be set [PC|MOB|HOM|MER|ELEM|NPC]
 * @param hp: What the HP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect
 * @return heal or zapped HP if valid
 */
int status_set_hp(struct block_list *bl, unsigned int hp, int flag)
{
	struct status_data *status;
	if (hp < 1)
		return 0;
	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (hp > status->max_hp)
		hp = status->max_hp;
	if (hp == status->hp)
		return 0;
	if (hp > status->hp)
		return status_heal(bl, hp - status->hp, 0, 1|flag);
	return status_zap(bl, status->hp - hp, 0);
}

/**
 * Sets Max HP to a given value
 * @param bl: Object whose Max HP will be set [PC|MOB|HOM|MER|ELEM|NPC]
 * @param maxhp: What the Max HP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect
 * @return heal or zapped HP if valid
 */
int status_set_maxhp(struct block_list *bl, unsigned int maxhp, int flag)
{
	struct status_data *status;
	int64 heal;

	if (maxhp < 1)
		return 0;
	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (maxhp == status->max_hp)
		return 0;

	heal = maxhp - status->max_hp;
	status->max_hp = maxhp;

	if (heal > 0)
		status_heal(bl, heal, 0, 1|flag);
	else
		status_zap(bl, -heal, 0);

	return maxhp;
}

/**
 * Sets SP to a given value
 * @param bl: Object whose SP will be set [PC|HOM|MER|ELEM]
 * @param sp: What the SP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect		
 * @return heal or zapped SP if valid
 */
int status_set_sp(struct block_list *bl, unsigned int sp, int flag)
{
	struct status_data *status;

	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (sp > status->max_sp)
		sp = status->max_sp;
	if (sp == status->sp)
		return 0;
	if (sp > status->sp)
		return status_heal(bl, 0, sp - status->sp, 1|flag);
	return status_zap(bl, 0, status->sp - sp);
}

/**
 * Sets Max SP to a given value
 * @param bl: Object whose Max SP will be set [PC|HOM|MER|ELEM]
 * @param maxsp: What the Max SP is to be set as
 * @param flag: Used in case final value is higher than current
 *		Use 2 to display healing effect
 * @return heal or zapped HP if valid
 */
int status_set_maxsp(struct block_list *bl, unsigned int maxsp, int flag)
{
	struct status_data *status;
	if (maxsp < 1)
		return 0;
	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0;

	if (maxsp == status->max_sp)
		return 0;
	if (maxsp > status->max_sp)
		status_heal(bl, maxsp - status->max_sp, 0, 1|flag);
	else
		status_zap(bl, status->max_sp - maxsp, 0);

	status->max_sp = maxsp;
	return maxsp;
}

/**
 * Takes HP/SP from an Object
 * @param bl: Object who will have HP/SP taken [PC|MOB|HOM|MER|ELEM]
 * @param hp: How much HP to charge
 * @param sp: How much SP to charge	
 * @return hp+sp through status_damage()
 * Note: HP/SP are integer values, not percentages. Values should be
 *	 calculated either within function call or before
 */
int64 status_charge(struct block_list* bl, int64 hp, int64 sp)
{
	if(!(bl->type&BL_CONSUME))
		return (int)hp+sp; // Assume all was charged so there are no 'not enough' fails.
	return status_damage(NULL, bl, hp, sp, 0, 3);
}

/**
 * Inflicts damage on the target with the according walkdelay.
 * @param src: Source object giving damage [PC|MOB|PET|HOM|MER|ELEM]
 * @param target: Target of the damage
 * @param dhp: How much damage to HP
 * @param dsp: How much damage to SP
 * @param walkdelay: Amount of time before object can walk again
 * @param flag: Damage flag decides various options
 *		flag&1: Passive damage - Does not trigger cancelling status changes
 *		flag&2: Fail if there is not enough to subtract
 *		flag&4: Mob does not give EXP/Loot if killed
 *		flag&8: Used to damage SP of a dead character
 * @return hp+sp
 * Note: HP/SP are integer values, not percentages. Values should be
 *	 calculated either within function call or before
 */
int status_damage(struct block_list *src,struct block_list *target,int64 dhp, int64 dsp, t_tick walkdelay, int flag)
{
	struct status_data *status;
	struct status_change *sc;
	int hp = (int)cap_value(dhp,INT_MIN,INT_MAX);
	int sp = (int)cap_value(dsp,INT_MIN,INT_MAX);

	nullpo_ret(target);

	if(sp && !(target->type&BL_CONSUME))
		sp = 0; // Not a valid SP target.

	if (hp < 0) { // Assume absorbed damage.
		status_heal(target, -hp, 0, 1);
		hp = 0;
	}

	if (sp < 0) {
		status_heal(target, 0, -sp, 1);
		sp = 0;
	}

	if (target->type == BL_SKILL) {
		if (!src || src->type&battle_config.can_damage_skill)
			return (int)skill_unit_ondamaged((struct skill_unit *)target, hp);
		return 0;
	}

	status = status_get_status_data(target);
	if(!status || status == &dummy_status )
		return 0;

	if ((unsigned int)hp >= status->hp) {
		if (flag&2) return 0;
		hp = status->hp;
	}

	if ((unsigned int)sp > status->sp) {
		if (flag&2) return 0;
		sp = status->sp;
	}

	if (!hp && !sp)
		return 0;

	if( !status->hp )
		flag |= 8;

	sc = status_get_sc(target);
	if( hp && battle_config.invincible_nodamage && src && sc && sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
		hp = 1;

	if( hp && !(flag&1) ) {
		if( sc ) {
			struct status_change_entry *sce;

			for (int i = 0; i < SC_MAX; i++) {
				if (status_sc_get_flag((sc_type)i)&SCF_REM_ON_DAMAGED)
					if (i != SC_STONE || (i == SC_STONE && sc->data[SC_STONE] && sc->opt1 == OPT1_STONE))
						status_change_end(target, (sc_type)i, INVALID_TIMER);
			}
			if ((sce=sc->data[SC_ENDURE]) && !sce->val4) {
				/** [Skotlex]
				* Endure count is only reduced by non-players on non-gvg maps.
				* val4 signals infinite endure.
				**/
				if (src && src->type != BL_PC && !map_flag_gvg2(target->m) && !map_getmapflag(target->m, MF_BATTLEGROUND) && --(sce->val2) <= 0)
					status_change_end(target, SC_ENDURE, INVALID_TIMER);
			}
			if ((sce=sc->data[SC_GRAVITATION]) && sce->val3 == BCT_SELF) {
				struct skill_unit_group* sg = skill_id2group(sce->val4);
				if (sg) {
					skill_delunitgroup(sg);
					sce->val4 = 0;
					status_change_end(target, SC_GRAVITATION, INVALID_TIMER);
				}
			}
			if(sc->data[SC_DANCING] && (unsigned int)hp > status->max_hp>>2)
				status_change_end(target, SC_DANCING, INVALID_TIMER);
			if(sc->data[SC_CLOAKINGEXCEED] && --(sc->data[SC_CLOAKINGEXCEED]->val2) <= 0)
				status_change_end(target, SC_CLOAKINGEXCEED, INVALID_TIMER);
			if(sc->data[SC_KAGEMUSYA] && --(sc->data[SC_KAGEMUSYA]->val3) <= 0)
				status_change_end(target, SC_KAGEMUSYA, INVALID_TIMER);
		}

		if (target->type == BL_PC)
			pc_bonus_script_clear(BL_CAST(BL_PC,target),BSF_REM_ON_DAMAGED);
		unit_skillcastcancel(target, 2);
	}

	status->hp-= hp;
	status->sp-= sp;

	if (sc && hp && status->hp) {
		if (sc->data[SC_AUTOBERSERK] &&
			(!sc->data[SC_PROVOKE] || !sc->data[SC_PROVOKE]->val2) &&
			status->hp < status->max_hp>>2)
			sc_start4(src,target,SC_PROVOKE,100,10,1,0,0,0);
		if (sc->data[SC_BERSERK] && status->hp <= 100)
			status_change_end(target, SC_BERSERK, INVALID_TIMER);
		if( sc->data[SC_RAISINGDRAGON] && status->hp <= 1000 )
			status_change_end(target, SC_RAISINGDRAGON, INVALID_TIMER);
		if (sc->data[SC_SATURDAYNIGHTFEVER] && status->hp <= 100)
			status_change_end(target, SC_SATURDAYNIGHTFEVER, INVALID_TIMER);
	}

	switch (target->type) {
		case BL_PC:  pc_damage((TBL_PC*)target,src,hp,sp); break;
		case BL_MOB: mob_damage((TBL_MOB*)target, src, hp); break;
		case BL_HOM: hom_damage((TBL_HOM*)target); break;
		case BL_MER: mercenary_heal((TBL_MER*)target,hp,sp); break;
		case BL_ELEM: elemental_heal((TBL_ELEM*)target,hp,sp); break;
	}

	if( src && target->type == BL_PC && ((TBL_PC*)target)->disguise ) { // Stop walking when attacked in disguise to prevent walk-delay bug
		unit_stop_walking( target, 1 );
	}

	if( status->hp || (flag&8) ) { // Still lives or has been dead before this damage.
		if (walkdelay)
			unit_set_walkdelay(target, gettick(), walkdelay, 0);
		return (int)(hp+sp);
	}

	status->hp = 0;
	/** [Skotlex]
	* NOTE: These dead functions should return:
	* 0: Death cancelled, auto-revived.
	* Non-zero: Standard death. Clear status, cancel move/attack, etc
	* &2: Remove object from map.
	* &4: Delete object from memory. (One time spawn mobs)
	**/
	switch (target->type) {
		case BL_PC:  flag = pc_dead((TBL_PC*)target,src); break;
		case BL_MOB: flag = mob_dead((TBL_MOB*)target, src, flag&4?3:0); break;
		case BL_HOM: flag = hom_dead((TBL_HOM*)target); break;
		case BL_MER: flag = mercenary_dead((TBL_MER*)target); break;
		case BL_ELEM: flag = elemental_dead((TBL_ELEM*)target); break;
		default:	// Unhandled case, do nothing to object.
			flag = 0;
			break;
	}

	if(!flag) // Death cancelled.
		return (int)(hp+sp);

	// Normal death
	if (battle_config.clear_unit_ondeath &&
		battle_config.clear_unit_ondeath&target->type)
		skill_clear_unitgroup(target);

	if(target->type&BL_REGEN) { // Reset regen ticks.
		struct regen_data *regen = status_get_regen_data(target);
		if (regen) {
			memset(&regen->tick, 0, sizeof(regen->tick));
			if (regen->sregen)
				memset(&regen->sregen->tick, 0, sizeof(regen->sregen->tick));
			if (regen->ssregen)
				memset(&regen->ssregen->tick, 0, sizeof(regen->ssregen->tick));
		}
	}

	if( sc && sc->data[SC_KAIZEL] && !map_flag_gvg2(target->m) ) { // flag&8 = disable Kaizel
		int time = skill_get_time2(SL_KAIZEL,sc->data[SC_KAIZEL]->val1);
		// Look for Osiris Card's bonus effect on the character and revive 100% or revive normally
		if ( target->type == BL_PC && BL_CAST(BL_PC,target)->special_state.restart_full_recover )
			status_revive(target, 100, 100);
		else
			status_revive(target, sc->data[SC_KAIZEL]->val2, 0);
		status_change_clear(target,0);
		clif_skill_nodamage(target,target,ALL_RESURRECTION,1,1);
		sc_start(src,target,skill_get_sc(PR_KYRIE),100,10,time);

		if( target->type == BL_MOB )
			((TBL_MOB*)target)->state.rebirth = 1;

		return (int)(hp+sp);
	}
	if (target->type == BL_MOB && sc && sc->data[SC_REBIRTH] && !((TBL_MOB*) target)->state.rebirth) { // Ensure the monster has not already rebirthed before doing so.
		status_revive(target, sc->data[SC_REBIRTH]->val2, 0);
		status_change_clear(target,0);
		((TBL_MOB*)target)->state.rebirth = 1;

		return (int)(hp+sp);
	}

	status_change_clear(target,0);

	if(flag&4) // Delete from memory. (also invokes map removal code)
		unit_free(target,CLR_DEAD);
	else if(flag&2) // remove from map
		unit_remove_map(target,CLR_DEAD);
	else { // Some death states that would normally be handled by unit_remove_map
		unit_stop_attack(target);
		unit_stop_walking(target,1);
		unit_skillcastcancel(target,0);
		clif_clearunit_area(target,CLR_DEAD);
		skill_unit_move(target,gettick(),4);
		skill_cleartimerskill(target);
	}

	// Always run NPC scripts for players last
	//FIXME those ain't always run if a player die if he was resurect meanwhile
	//cf SC_REBIRTH, SC_KAIZEL, pc_dead...
	if(target->type == BL_PC) {
		TBL_PC *sd = BL_CAST(BL_PC,target);
		if( sd->bg_id ) {
			struct battleground_data *bg;
			if( (bg = bg_team_search(sd->bg_id)) != NULL && bg->die_event[0] )
				npc_event(sd, bg->die_event, 0);
		}

		npc_script_event(sd,NPCE_DIE);
	}

	return (int)(hp+sp);
}

/**
 * Heals an object
 * @param bl: Object to heal [PC|MOB|HOM|MER|ELEM]
 * @param hhp: How much HP to heal
 * @param hsp: How much SP to heal
 * @param flag:	Whether it's Forced(&1), gives HP/SP(&2) heal effect,
 *      or gives HP(&4) heal effect with 0 heal
 *		Forced healing overrides heal impedement statuses (Berserk)
 * @return hp+sp
 */
int status_heal(struct block_list *bl,int64 hhp,int64 hsp, int flag)
{
	struct status_data *status;
	struct status_change *sc;
	int hp = (int)cap_value(hhp,INT_MIN,INT_MAX);
	int sp = (int)cap_value(hsp,INT_MIN,INT_MAX);

	status = status_get_status_data(bl);

	if (status == &dummy_status || !status->hp)
		return 0;

	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;

	if (hp < 0) {
		if (hp == INT_MIN) // -INT_MIN == INT_MIN in some architectures!
			hp++;
		status_damage(NULL, bl, -hp, 0, 0, 1);
		hp = 0;
	}

	if(hp) {
		if( sc && (sc->data[SC_BERSERK]) ) {
			if( flag&1 )
				flag &= ~2;
			else
				hp = 0;
		}

		if((unsigned int)hp > status->max_hp - status->hp)
			hp = status->max_hp - status->hp;
	}

	if(sp < 0) {
		if (sp == INT_MIN)
			sp++;
		status_damage(NULL, bl, 0, -sp, 0, 1);
		sp = 0;
	}

	if(sp) {
		if((unsigned int)sp > status->max_sp - status->sp)
			sp = status->max_sp - status->sp;
	}

	if(!sp && !hp && !(flag&4))
		return 0;

	status->hp += hp;
	status->sp += sp;

	if(hp && sc &&
		sc->data[SC_AUTOBERSERK] &&
		sc->data[SC_PROVOKE] &&
		sc->data[SC_PROVOKE]->val2==1 &&
		status->hp>=status->max_hp>>2
	)	// End auto berserk.
		status_change_end(bl, SC_PROVOKE, INVALID_TIMER);

	// Send HP update to client
	switch(bl->type) {
		case BL_PC:  pc_heal((TBL_PC*)bl,hp,sp,flag); break;
		case BL_MOB: mob_heal((TBL_MOB*)bl,hp); break;
		case BL_HOM: hom_heal((TBL_HOM*)bl); break;
		case BL_MER: mercenary_heal((TBL_MER*)bl,hp,sp); break;
		case BL_ELEM: elemental_heal((TBL_ELEM*)bl,hp,sp); break;
	}

	return (int)hp+sp;
}

/**
 * Applies percentage based damage to a unit.
 * If a mob is killed this way and there is no src, no EXP/Drops will be awarded.
 * @param src: Object initiating HP/SP modification [PC|MOB|PET|HOM|MER|ELEM]
 * @param target: Object to modify HP/SP
 * @param hp_rate: Percentage of HP to modify. If > 0:percent is of current HP, if < 0:percent is of max HP
 * @param sp_rate: Percentage of SP to modify. If > 0:percent is of current SP, if < 0:percent is of max SP
 * @param flag: \n
 *		0: Heal target \n 
 *		1: Use status_damage \n 
 *		2: Use status_damage and make sure target must not die from subtraction
 * @return hp+sp through status_heal()
 */
int status_percent_change(struct block_list *src, struct block_list *target, int8 hp_rate, int8 sp_rate, uint8 flag)
{
	struct status_data *status;
	unsigned int hp = 0, sp = 0;

	status = status_get_status_data(target);


	// It's safe now [MarkZD]
	if (hp_rate > 99)
		hp = status->hp;
	else if (hp_rate > 0)
		hp = apply_rate(status->hp, hp_rate);
	else if (hp_rate < -99)
		hp = status->max_hp;
	else if (hp_rate < 0)
		hp = (apply_rate(status->max_hp, -hp_rate));
	if (hp_rate && !hp)
		hp = 1;

	if (flag == 2 && hp >= status->hp)
		hp = status->hp-1; // Must not kill target.

	if (sp_rate > 99)
		sp = status->sp;
	else if (sp_rate > 0)
		sp = apply_rate(status->sp, sp_rate);
	else if (sp_rate < -99)
		sp = status->max_sp;
	else if (sp_rate < 0)
		sp = (apply_rate(status->max_sp, -sp_rate));
	if (sp_rate && !sp)
		sp = 1;

	// Ugly check in case damage dealt is too much for the received args of
	// status_heal / status_damage. [Skotlex]
	if (hp > INT_MAX) {
		hp -= INT_MAX;
		if (flag)
			status_damage(src, target, INT_MAX, 0, 0, (!src||src==target?5:1));
		else
			status_heal(target, INT_MAX, 0, 0);
	}
	if (sp > INT_MAX) {
		sp -= INT_MAX;
		if (flag)
			status_damage(src, target, 0, INT_MAX, 0, (!src||src==target?5:1));
		else
			status_heal(target, 0, INT_MAX, 0);
	}
	if (flag)
		return status_damage(src, target, hp, sp, 0, (!src||src==target?5:1));
	return status_heal(target, hp, sp, 0);
}

/**
 * Revives a unit
 * @param bl: Object to revive [PC|MOB|HOM]
 * @param per_hp: Percentage of HP to revive with
 * @param per_sp: Percentage of SP to revive with
 * @return Successful (1) or Invalid target (0)
 */
int status_revive(struct block_list *bl, unsigned char per_hp, unsigned char per_sp)
{
	struct status_data *status;
	unsigned int hp, sp;
	if (!status_isdead(bl)) return 0;

	status = status_get_status_data(bl);
	if (status == &dummy_status)
		return 0; // Invalid target.

	hp = (int64)status->max_hp * per_hp/100;
	sp = (int64)status->max_sp * per_sp/100;

	if(hp > status->max_hp - status->hp)
		hp = status->max_hp - status->hp;
	else if (per_hp && !hp)
		hp = 1;

	if(sp > status->max_sp - status->sp)
		sp = status->max_sp - status->sp;
	else if (per_sp && !sp)
		sp = 1;

	status->hp += hp;
	status->sp += sp;

	if (bl->prev) // Animation only if character is already on a map.
		clif_resurrection(bl, 1);
	switch (bl->type) {
		case BL_PC:  pc_revive((TBL_PC*)bl, hp, sp); break;
		case BL_MOB: mob_revive((TBL_MOB*)bl, hp); break;
		case BL_HOM: hom_revive((TBL_HOM*)bl, hp, sp); break;
	}
	return 1;
}

/**
 * Checks whether the src can use the skill on the target,
 * taking into account status/option of both source/target
 * @param src:	Object using skill on target [PC|MOB|PET|HOM|MER|ELEM]
		src MAY be NULL to indicate we shouldn't check it, this is a ground-based skill attack
 * @param target: Object being targeted by src [PC|MOB|HOM|MER|ELEM]
		 target MAY be NULL, which checks if src can cast skill_id on the ground
 * @param skill_id: Skill ID being used on target
 * @param flag:	0 - Trying to use skill on target
 *		1 - Cast bar is done
 *		2 - Skill already pulled off, check is due to ground-based skills or splash-damage ones
 * @return src can use skill (1) or cannot use skill (0)
 * @author [Skotlex]
 */
bool status_check_skilluse(struct block_list *src, struct block_list *target, uint16 skill_id, int flag) {
	struct status_data *status;
	struct status_change *sc = NULL, *tsc;
	int hide_flag;

	status = src ? status_get_status_data(src) : &dummy_status;

	if (src && src->type != BL_PC && status_isdead(src))
		return false;

	if (!skill_id) { // Normal attack checks.
		// This mode is only needed for melee attacking.
		if (!status_has_mode(status,MD_CANATTACK))
			return false;
		// Dead state is not checked for skills as some skills can be used
		// on dead characters, said checks are left to skill.c [Skotlex]
		if (target && status_isdead(target))
			return false;
	}

	switch( skill_id ) {
		case PA_PRESSURE:
			if( flag && target ) {
				// Gloria Avoids pretty much everything....
				tsc = status_get_sc(target);
				if(tsc && tsc->option&OPTION_HIDE)
					return false;
			}
			break;
		case GN_WALLOFTHORN:
			if( target && status_isdead(target) )
				return false;
			break;
		case AL_TELEPORT:
		case ALL_ODINS_POWER:
			// Should fail when used on top of Land Protector [Skotlex]
			if (src && map_getcell(src->m, src->x, src->y, CELL_CHKLANDPROTECTOR)
				&& !status_has_mode(status,MD_STATUS_IMMUNE)
				&& (src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_id))
				return false;
			break;
		case SC_MANHOLE:
			// Skill is disabled against special racial grouped monsters(GvG and Battleground)
			if (target && ( status_get_race2(target) == RC2_GVG || status_get_race2(target) == RC2_BATTLEFIELD ) )
				return false;
		default:
			break;
	}

	if ( src )
		sc = status_get_sc(src);

	if( sc && sc->count ) {
		if (sc->data[SC_ALL_RIDING])
			return false; //You can't use skills while in the new mounts (The client doesn't let you, this is to make cheat-safe)

		if (flag == 1 && !status_has_mode(status,MD_STATUS_IMMUNE) && ( // Applies to after cast completion only and doesn't apply to Boss monsters.
			(sc->data[SC_ASH] && rnd()%2) || // Volcanic Ash has a 50% chance of causing skills to fail.
			(sc->data[SC_KYOMU] && rnd()%100 < 25) // Kyomu has a 25% chance of causing skills fail.
		)) {
			if (src->type == BL_PC)
				clif_skill_fail((TBL_PC*)src,skill_id,USESKILL_FAIL_LEVEL,0);
			return false;
		}

		if (skill_id != RK_REFRESH && skill_id != SU_GROOMING && skill_id != SR_GENTLETOUCH_CURE) { // Stuned/Frozen/etc
			if (flag != 1) // Can't cast, casted stuff can't damage.
				return false;
			if (skill_get_casttype(skill_id) == CAST_DAMAGE)
				return false; // Damage spells stop casting.
		}

		if (
			(sc->data[SC_TRICKDEAD] && skill_id != NV_TRICKDEAD)
			|| (sc->data[SC_AUTOCOUNTER] && !flag && skill_id)
			|| (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF && skill_id != PA_GOSPEL)
			|| (sc->data[SC_SUHIDE] && skill_id != SU_HIDE)
		)
			return false;

		if (sc->data[SC_WINKCHARM] && target && !flag) { // Prevents skill usage
			if (unit_bl2ud(src) && (unit_bl2ud(src))->walktimer == INVALID_TIMER)
				unit_walktobl(src, map_id2bl(sc->data[SC_WINKCHARM]->val2), 3, 1);
			clif_emotion(src, ET_THROB);
			return false;
		}

		if (sc->data[SC_BLADESTOP]) {
			switch (sc->data[SC_BLADESTOP]->val1) {
				case 5: if (skill_id == MO_EXTREMITYFIST) break;
				case 4: if (skill_id == MO_CHAINCOMBO) break;
				case 3: if (skill_id == MO_INVESTIGATE) break;
				case 2: if (skill_id == MO_FINGEROFFENSIVE) break;
				default: return false;
			}
		}

		if (sc->data[SC_DANCING] && flag!=2) {
			if (src->type == BL_PC && ((skill_id >= WA_SWING_DANCE && skill_id <= WM_UNLIMITED_HUMMING_VOICE ) ||
				skill_id == WM_FRIGG_SONG))
			{ // Lvl 5 Lesson or higher allow you use 3rd job skills while dancing.
				if( pc_checkskill((TBL_PC*)src,WM_LESSON) < 5 )
					return false;
			} else if(sc->data[SC_LONGING]) { // Allow everything except dancing/re-dancing. [Skotlex]
				if (skill_id == BD_ENCORE ||
					skill_get_inf2(skill_id)&(INF2_SONG_DANCE|INF2_ENSEMBLE_SKILL)
					)
					return false;
			} else if(!(skill_get_inf3(skill_id)&INF3_USABLE_DANCE)) // Skills that can be used in dancing state
				return false;
			if ((sc->data[SC_DANCING]->val1&0xFFFF) == CG_HERMODE && skill_id == BD_ADAPTATION)
				return false; // Can't amp out of Wand of Hermode :/ [Skotlex]
		}

		if (skill_id && // Do not block item-casted skills.
			(src->type != BL_PC || ((TBL_PC*)src)->skillitem != skill_id)
		) {	// Skills blocked through status changes...
			if (!flag && ( // Blocked only from using the skill (stuff like autospell may still go through
				sc->cant.cast ||
				(sc->data[SC_BASILICA] && (sc->data[SC_BASILICA]->val4 != src->id || skill_id != HP_BASILICA)) || // Only Basilica caster that can cast, and only Basilica to cancel it
				(sc->data[SC_MARIONETTE] && skill_id != CG_MARIONETTE) || // Only skill you can use is marionette again to cancel it
				(sc->data[SC_MARIONETTE2] && skill_id == CG_MARIONETTE) || // Cannot use marionette if you are being buffed by another
				(sc->data[SC_ANKLE] && skill_block_check(src, SC_ANKLE, skill_id)) ||
				(sc->data[SC_STASIS] && skill_block_check(src, SC_STASIS, skill_id)) ||
				(sc->data[SC_BITE] && skill_block_check(src, SC_BITE, skill_id)) ||
				(sc->data[SC_KAGEHUMI] && skill_block_check(src, SC_KAGEHUMI, skill_id))
			))
				return false;

			// Skill blocking.
			if (
				(sc->data[SC_VOLCANO] && skill_id == WZ_ICEWALL) ||
				(sc->data[SC_ROKISWEIL] && skill_id != BD_ADAPTATION) ||
				(sc->data[SC_HERMODE] && skill_get_inf(skill_id) & INF_SUPPORT_SKILL) ||
				(sc->data[SC_NOCHAT] && sc->data[SC_NOCHAT]->val1&MANNER_NOSKILL)
			)
				return false;
		}

		if (sc->option) {
			if ((sc->option&OPTION_HIDE) && src->type == BL_PC && (!skill_id || !(skill_get_inf3(skill_id)&INF3_USABLE_HIDING))) {
				// Non players can use all skills while hidden.
				return false;
			}
			if (sc->option&OPTION_CHASEWALK && skill_id != ST_CHASEWALK)
				return false;
		}
	}

	if (target == NULL || target == src) // No further checking needed.
		return true;

	tsc = status_get_sc(target);

	if (tsc && tsc->count) {
		/**
		* Attacks in invincible are capped to 1 damage and handled in batte.c.
		* Allow spell break and eske for sealed shrine GDB when in INVINCIBLE state.
		**/
		if( tsc->data[SC_INVINCIBLE] && !tsc->data[SC_INVINCIBLEOFF] && skill_id && !(skill_id&(SA_SPELLBREAKER|SL_SKE)) )
			return false;
		if(!skill_id && tsc->data[SC_TRICKDEAD])
			return false;
		if((skill_id == WZ_STORMGUST || skill_id == WZ_FROSTNOVA || skill_id == NJ_HYOUSYOURAKU)
			&& tsc->data[SC_FREEZE])
			return false;
		if(skill_id == PR_LEXAETERNA && (tsc->data[SC_FREEZE] || (tsc->data[SC_STONE] && tsc->opt1 == OPT1_STONE)))
			return false;
		if (tsc->data[SC__MANHOLE] && !(skill_get_inf3(skill_id)&INF3_USABLE_MANHOLE))
			return false;
	}

	// If targetting, cloak+hide protect you, otherwise only hiding does.
	hide_flag = flag?OPTION_HIDE:(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK);

 	// Skill that can hit hidden target
	if( skill_get_inf3(skill_id)&INF3_HIT_HIDING )
		hide_flag &= ~OPTION_HIDE;

	switch( target->type ) {
		case BL_PC: {
				struct map_session_data *tsd = (TBL_PC*)target;
				bool is_boss = (src && status_get_class_(src) == CLASS_BOSS);
				bool is_detect = status_has_mode(status,MD_DETECTOR);

				if (pc_isinvisible(tsd))
					return false;
				if (tsc) {
					if ((tsc->option&hide_flag) && !is_boss && (tsd->special_state.perfect_hiding || !is_detect))
						return false;
					if (tsc->data[SC_CLOAKINGEXCEED] && !is_boss && (tsd->special_state.perfect_hiding || is_detect))
						return false; // Works against insect and demon but not against bosses
					if (tsc->data[SC__FEINTBOMB] && (is_boss || is_detect))
						return false; // Works against all
					if ((tsc->data[SC_CAMOUFLAGE] || tsc->data[SC_STEALTHFIELD] || tsc->data[SC_SUHIDE]) && !(is_boss || is_detect) && (!skill_id || (!flag && src)))
						return false; // Insect, demon, and boss can detect
				}
			}
			break;
		case BL_ITEM: // Allow targetting of items to pick'em up (or in the case of mobs, to loot them).
			// !TODO: Would be nice if this could be used to judge whether the player can or not pick up the item it targets. [Skotlex]
			if (status_has_mode(status,MD_LOOTER))
				return true;
			return false;
		case BL_HOM:
		case BL_MER:
		case BL_ELEM:
			if( target->type == BL_HOM && skill_id && battle_config.hom_setting&HOMSET_NO_SUPPORT_SKILL && skill_get_inf(skill_id)&INF_SUPPORT_SKILL && battle_get_master(target) != src )
				return false; // Can't use support skills on Homunculus (only Master/Self)
			if( target->type == BL_MER && (skill_id == PR_ASPERSIO || (skill_id >= SA_FLAMELAUNCHER && skill_id <= SA_SEISMICWEAPON)) && battle_get_master(target) != src )
				return false; // Can't use Weapon endow skills on Mercenary (only Master)
			if( skill_id == AM_POTIONPITCHER && ( target->type == BL_MER || target->type == BL_ELEM) )
				return false; // Can't use Potion Pitcher on Mercenaries
		default:
			// Check for chase-walk/hiding/cloaking opponents.
			if( tsc ) {
				if( tsc->option&hide_flag && !status_has_mode(status,MD_DETECTOR))
					return false;
			}
	}
	return true;
}

/**
 * Checks whether the src can see the target
 * @param src:	Object using skill on target [PC|MOB|PET|HOM|MER|ELEM]
 * @param target: Object being targeted by src [PC|MOB|HOM|MER|ELEM]
 * @return src can see (1) or target is invisible (0)
 * @author [Skotlex]
 */
int status_check_visibility(struct block_list *src, struct block_list *target)
{
	int view_range;
	struct status_data* status = status_get_status_data(src);
	struct status_change* tsc = status_get_sc(target);
	switch (src->type) {
		case BL_MOB:
			view_range = ((TBL_MOB*)src)->min_chase;
			break;
		case BL_PET:
			view_range = ((TBL_PET*)src)->db->range2;
			break;
		default:
			view_range = AREA_SIZE;
	}

	if (src->m != target->m || !check_distance_bl(src, target, view_range))
		return 0;

	if ( src->type == BL_NPC) // NPCs don't care for the rest
		return 1;

	if (tsc) {
		bool is_boss = (status_get_class_(src) == CLASS_BOSS);
		bool is_detector = status_has_mode(status,MD_DETECTOR);

		switch (target->type) {	// Check for chase-walk/hiding/cloaking opponents.
			case BL_PC: {
					struct map_session_data *tsd = (TBL_PC*)target;

					if (((tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)) || tsc->data[SC_CAMOUFLAGE] || tsc->data[SC_STEALTHFIELD] || tsc->data[SC_SUHIDE]) && !is_boss && (tsd->special_state.perfect_hiding || !is_detector))
						return 0;
					if (tsc->data[SC_CLOAKINGEXCEED] && !is_boss && ((tsd && tsd->special_state.perfect_hiding) || is_detector))
						return 0;
					if (tsc->data[SC__FEINTBOMB] && !is_boss && !is_detector)
						return 0;
				}
				break;
			default:
				if (((tsc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK)) || tsc->data[SC_CAMOUFLAGE] || tsc->data[SC_STEALTHFIELD] || tsc->data[SC_SUHIDE]) && !is_boss && !is_detector)
					return 0;
		}
	}

	return 1;
}

/**
 * Base ASPD value taken from the job tables
 * @param sd: Player object
 * @param status: Player status
 * @return base amotion after single/dual weapon and shield adjustments [RENEWAL]
 *	  base amotion after single/dual weapon and stats adjustments [PRE-RENEWAL]
 */
int status_base_amotion_pc(struct map_session_data* sd, struct status_data* status)
{
	int amotion;
	int classidx = pc_class2idx(sd->status.class_);
#ifdef RENEWAL_ASPD
	int16 skill_lv, val = 0;
	float temp_aspd = 0;

	amotion = job_info[classidx].aspd_base[sd->weapontype1]; // Single weapon
	if (sd->status.shield)
		amotion += job_info[classidx].aspd_base[MAX_WEAPON_TYPE];
	else if (sd->weapontype2 && sd->equip_index[EQI_HAND_R] != sd->equip_index[EQI_HAND_L])
		amotion += job_info[classidx].aspd_base[sd->weapontype2] / 4; // Dual-wield

	switch(sd->status.weapon) {
		case W_BOW:
		case W_MUSICAL:
		case W_WHIP:
		case W_REVOLVER:
		case W_RIFLE:
		case W_GATLING:
		case W_SHOTGUN:
		case W_GRENADE:
			temp_aspd = status->dex * status->dex / 7.0f + status->agi * status->agi * 0.5f;
			break;
		default:
			temp_aspd = status->dex * status->dex / 5.0f + status->agi * status->agi * 0.5f;
			break;
	}
	temp_aspd = (float)(sqrt(temp_aspd) * 0.25f) + 0xc4;
	if ((skill_lv = pc_checkskill(sd,SA_ADVANCEDBOOK)) > 0 && sd->status.weapon == W_BOOK)
		val += (skill_lv - 1) / 2 + 1;
	if ((skill_lv = pc_checkskill(sd,GS_SINGLEACTION)) > 0 && (sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		val += ((skill_lv + 1) / 2);
	amotion = ((int)(temp_aspd + ((float)(status_calc_aspd(&sd->bl, &sd->sc, true) + val) * status->agi / 200)) - min(amotion, 200));
#else
	// Angra Manyu disregards aspd_base and similar
	if (pc_checkequip2(sd, ITEMID_ANGRA_MANYU, EQI_ACC_L, EQI_MAX))
		return 0;

	// Base weapon delay
	amotion = (sd->status.weapon < MAX_WEAPON_TYPE)
	 ? (job_info[classidx].aspd_base[sd->status.weapon]) // Single weapon
	 : (job_info[classidx].aspd_base[sd->weapontype1] + job_info[classidx].aspd_base[sd->weapontype2]) * 7 / 10; // Dual-wield

	// Percentual delay reduction from stats
	amotion -= amotion * (4 * status->agi + status->dex) / 1000;

	// Raw delay adjustment from bAspd bonus
	amotion += sd->bonus.aspd_add;
#endif

 	return amotion;
}

/**
 * Base attack value calculated for units
 * @param bl: Object to get attack for [BL_CHAR|BL_NPC]
 * @param status: Object status
 * @return base attack
 */
unsigned short status_base_atk(const struct block_list *bl, const struct status_data *status, int level)
{
	int flag = 0, str, dex, dstr;

	if(!(bl->type&battle_config.enable_baseatk))
		return 0;

	if (bl->type == BL_PC)
	switch(((TBL_PC*)bl)->status.weapon) {
		case W_BOW:
		case W_MUSICAL:
		case W_WHIP:
		case W_REVOLVER:
		case W_RIFLE:
		case W_GATLING:
		case W_SHOTGUN:
		case W_GRENADE:
			flag = 1;
	}
	if (flag) {
#ifdef RENEWAL
		dstr =
#endif
		str = status->dex;
		dex = status->str;
	} else {
#ifdef RENEWAL
		dstr =
#endif
		str = status->str;
		dex = status->dex;
	}

	/** [Skotlex]
	* Normally only players have base-atk, but homunc have a different batk
	* equation, hinting that perhaps non-players should use this for batk.
	**/
	switch (bl->type) {
		case BL_HOM:
#ifdef RENEWAL
			str = 2 * level + status_get_homstr(bl);
#else
			dstr = str / 10;
			str += dstr*dstr;
#endif
			break;
		case BL_PC:
#ifdef RENEWAL
			str = (dstr * 10 + dex * 10 / 5 + status->luk * 10 / 3 + level * 10 / 4) / 10;
#else
			dstr = str / 10;
			str += dstr*dstr;
			str += dex / 5 + status->luk / 5;
#endif
			break;
		default:// Others
#ifdef RENEWAL
			str = dstr + level;
#else
			dstr = str / 10;
			str += dstr*dstr;
			str += dex / 5 + status->luk / 5;
#endif
			break;
	}

	return cap_value(str, 0, USHRT_MAX);
}

#ifdef RENEWAL
/**
 * Weapon attack value calculated for Players
 * @param wa: Weapon attack
 * @param status: Player status
 * @return weapon attack
 */
unsigned int status_weapon_atk(struct weapon_atk wa, struct map_session_data *sd)
{
	float str = sd->base_status.str;
	int weapon_atk_bonus = 0;

	if (wa.range > 3 && !pc_checkskill(sd, SU_SOULATTACK))
		str = sd->base_status.dex;
	if (sd->bonus.weapon_atk_rate)
		weapon_atk_bonus = wa.atk * sd->bonus.weapon_atk_rate / 100;
	// wa.atk2 = refinement, wa.atk = base equip atk, wa.atk*str/200 = bonus str
	return wa.atk + wa.atk2 + (int)(wa.atk * (str/200) + weapon_atk_bonus);
}
#endif

#ifndef RENEWAL
unsigned short status_base_matk_min(const struct status_data* status) { return status->int_ + (status->int_ / 7) * (status->int_ / 7); }
unsigned short status_base_matk_max(const struct status_data* status) { return status->int_ + (status->int_ / 5) * (status->int_ / 5); }
#else
/*
* Calculates minimum attack variance 80% from db's ATK1 for non BL_PC
* status->batk (base attack) will be added in battle_calc_base_damage
*/
unsigned short status_base_atk_min(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->rhw.atk * 80 / 100;
		case BL_HOM:
			return (status_get_homstr(bl) + status_get_homdex(bl)) / 5;
		default:
			return status->rhw.atk;
	}
}

/*
* Calculates maximum attack variance 120% from db's ATK1 for non BL_PC
* status->batk (base attack) will be added in battle_calc_base_damage
*/
unsigned short status_base_atk_max(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->rhw.atk * 120 / 100;
		case BL_HOM:
			return (status_get_homluk(bl) + status_get_homstr(bl) + status_get_homdex(bl)) / 3;
		default:
			return status->rhw.atk2;
	}
}

/*
* Calculates minimum magic attack
*/
unsigned short status_base_matk_min(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->int_ + level + status->rhw.matk * 70 / 100;
		case BL_HOM:
			return status_get_homint(bl) + level + (status_get_homint(bl) + status_get_homdex(bl)) / 5;
		case BL_PC:
		default:
			return status->int_ + (status->int_ / 2) + (status->dex / 5) + (status->luk / 3) + (level / 4);
	}
}

/*
* Calculates maximum magic attack
*/
unsigned short status_base_matk_max(struct block_list *bl, const struct status_data* status, int level)
{
	switch (bl->type) {
		case BL_PET:
		case BL_MOB:
		case BL_MER:
		case BL_ELEM:
			return status->int_ + level + status->rhw.matk * 130 / 100;
		case BL_HOM:
			return status_get_homint(bl) + level + (status_get_homluk(bl) + status_get_homint(bl) + status_get_homdex(bl)) / 3;
		case BL_PC:
		default:
			return status->int_ + (status->int_ / 2) + (status->dex / 5) + (status->luk / 3) + (level / 4);
	}
}
#endif

/**
 * Fills in the misc data that can be calculated from the other status info (except for level)
 * @param bl: Object to calculate status on [PC|MOB|PET|HOM|MERC|ELEM]
 * @param status: Player status
 */
void status_calc_misc(struct block_list *bl, struct status_data *status, int level)
{
	int stat;

	// Non players get the value set, players need to stack with previous bonuses.
	if( bl->type != BL_PC )
		status->batk =
		status->hit = status->flee =
		status->def2 = status->mdef2 =
		status->cri = status->flee2 = 0;

#ifdef RENEWAL // Renewal formulas
	if (bl->type == BL_HOM) {
		// Def2
		stat = status_get_homvit(bl) + status_get_homagi(bl) / 2;
		status->def2 = cap_value(stat, 0, SHRT_MAX);
		// Mdef2
		stat = (status_get_homvit(bl) + status_get_homint(bl)) / 2;
		status->mdef2 = cap_value(stat, 0, SHRT_MAX);
		// Def
		stat = status->def;
		stat += status_get_homvit(bl) + level / 2;
		status->def = cap_value(stat, 0, SHRT_MAX);
		// Mdef
		stat = (int)(((float)status_get_homvit(bl) + level) / 4 + (float)status_get_homint(bl) / 2);
		status->mdef = cap_value(stat, 0, SHRT_MAX);
		// Hit
		stat = level + status->dex + 150;
		status->hit = cap_value(stat, 1, SHRT_MAX);
		// Flee
		stat = level + status_get_homagi(bl);
		status->flee = cap_value(stat, 1, SHRT_MAX);
	} else {
		// Hit
		stat = status->hit;
		stat += level + status->dex + (bl->type == BL_PC ? status->luk / 3 + 175 : 150); //base level + ( every 1 dex = +1 hit ) + (every 3 luk = +1 hit) + 175
		status->hit = cap_value(stat, 1, SHRT_MAX);
		// Flee
		stat = status->flee;
		stat += level + status->agi + (bl->type == BL_MER ? 0 : bl->type == BL_PC ? status->luk / 5 : 0) + 100; //base level + ( every 1 agi = +1 flee ) + (every 5 luk = +1 flee) + 100
		status->flee = cap_value(stat, 1, SHRT_MAX);
		// Def2
		if (bl->type == BL_MER)
			stat = (int)(status->vit + ((float)level / 10) + ((float)status->vit / 5));
		else {
			stat = status->def2;
			stat += (int)(((float)level + status->vit) / 2 + (bl->type == BL_PC ? ((float)status->agi / 5) : 0)); //base level + (every 2 vit = +1 def) + (every 5 agi = +1 def)
		}
		status->def2 = cap_value(stat, 0, SHRT_MAX);
		// Mdef2
		if (bl->type == BL_MER)
			stat = (int)(((float)level / 10) + ((float)status->int_ / 5));
		else {
			stat = status->mdef2;
			stat += (int)(bl->type == BL_PC ? (status->int_ + ((float)level / 4) + ((float)(status->dex + status->vit) / 5)) : ((float)(status->int_ + level) / 4)); //(every 4 base level = +1 mdef) + (every 1 int = +1 mdef) + (every 5 dex = +1 mdef) + (every 5 vit = +1 mdef)
		}
		status->mdef2 = cap_value(stat, 0, SHRT_MAX);
	}

	// ATK
	if (bl->type != BL_PC) {
		status->rhw.atk2 = status_base_atk_max(bl, status, level);
		status->rhw.atk = status_base_atk_min(bl, status, level);
	}

	// MAtk
	status->matk_min = status_base_matk_min(bl, status, level);
	status->matk_max = status_base_matk_max(bl, status, level);
#else
	// Matk
	status->matk_min = status_base_matk_min(status);
	status->matk_max = status_base_matk_max(status);
	// Hit
	stat = status->hit;
	stat += level + status->dex;
	status->hit = cap_value(stat, 1, SHRT_MAX);
	// Flee
	stat = status->flee;
	stat += level + status->agi;
	status->flee = cap_value(stat, 1, SHRT_MAX);
	// Def2
	stat = status->def2;
	stat += status->vit;
	status->def2 = cap_value(stat, 0, SHRT_MAX);
	// Mdef2
	stat = status->mdef2;
	stat += status->int_ + (status->vit>>1);
	status->mdef2 = cap_value(stat, 0, SHRT_MAX);
#endif

	//Critical
	if( bl->type&battle_config.enable_critical ) {
		stat = status->cri;
		stat += 10 + (status->luk*10/3); // (every 1 luk = +0.3 critical)
		status->cri = cap_value(stat, 1, SHRT_MAX);
	} else
		status->cri = 0;

	if (bl->type&battle_config.enable_perfect_flee) {
		stat = status->flee2;
		stat += status->luk + 10; // (every 10 luk = +1 perfect flee)
		status->flee2 = cap_value(stat, 0, SHRT_MAX);
	} else
		status->flee2 = 0;

	if (status->batk) {
		int temp = status->batk + status_base_atk(bl, status, level);
		status->batk = cap_value(temp, 0, USHRT_MAX);
	} else
		status->batk = status_base_atk(bl, status, level);

	if (status->cri) {
		switch (bl->type) {
			case BL_MOB:
				if(battle_config.mob_critical_rate != 100)
					status->cri = cap_value(status->cri*battle_config.mob_critical_rate/100,1,SHRT_MAX);
				if(!status->cri && battle_config.mob_critical_rate)
					status->cri = 10;
				break;
			case BL_PC:
				// Players don't have a critical adjustment setting as of yet.
				break;
			default:
				if(battle_config.critical_rate != 100)
					status->cri = cap_value(status->cri*battle_config.critical_rate/100,1,SHRT_MAX);
				if (!status->cri && battle_config.critical_rate)
					status->cri = 10;
		}
	}

	if(bl->type&BL_REGEN)
		status_calc_regen(bl, status, status_get_regen_data(bl));
}

/**
 * Calculates the initial status for the given mob
 * @param md: Mob object
 * @param opt: Whether or not it is the first calculation
		This will only be false when a mob levels up (Regular and WoE Guardians)
 * @return 1 for calculated special statuses or 0 for none
 * @author [Skotlex]
 */
int status_calc_mob_(struct mob_data* md, enum e_status_calc_opt opt)
{
	struct status_data *status;
	struct block_list *mbl = NULL;
	int flag=0;

	if (opt&SCO_FIRST) { // Set basic level on respawn.
		if (md->level > 0 && md->level <= MAX_LEVEL && md->level != md->db->lv)
			;
		else
			md->level = md->db->lv;
	}

	// Check if we need custom base-status
	if (battle_config.mobs_level_up && md->level > md->db->lv)
		flag|=1;

	if (md->special_state.size)
		flag|=2;

	if (md->guardian_data && md->guardian_data->guardup_lv)
		flag|=4;
	if (md->mob_id == MOBID_EMPERIUM)
		flag|=4;

	if (battle_config.slaves_inherit_speed && md->master_id)
		flag|=8;

	if (md->master_id && md->special_state.ai>AI_ATTACK)
		flag|=16;

	if (md->master_id && battle_config.slaves_inherit_mode)
		flag |= 32;

	if (!flag) { // No special status required.
		if (md->base_status) {
			aFree(md->base_status);
			md->base_status = NULL;
		}
		if (opt&SCO_FIRST)
			memcpy(&md->status, &md->db->status, sizeof(struct status_data));
		return 0;
	}
	if (!md->base_status)
		md->base_status = (struct status_data*)aCalloc(1, sizeof(struct status_data));

	status = md->base_status;
	memcpy(status, &md->db->status, sizeof(struct status_data));

	if (flag&(8|16))
		mbl = map_id2bl(md->master_id);

	if (flag&8 && mbl) {
		struct status_data *mstatus = status_get_base_status(mbl);

		if (mstatus &&
			battle_config.slaves_inherit_speed&(status_has_mode(mstatus,MD_CANMOVE)?1:2))
			status->speed = mstatus->speed;
		if( status->speed < 2 ) // Minimum for the unit to function properly
			status->speed = 2;
	}

	if (flag&32)
		status_calc_slave_mode(md, map_id2md(md->master_id));

	if (flag&1) { // Increase from mobs leveling up [Valaris]
		int diff = md->level - md->db->lv;

		status->str += diff;
		status->agi += diff;
		status->vit += diff;
		status->int_ += diff;
		status->dex += diff;
		status->luk += diff;
		status->max_hp += diff * status->vit;
		status->max_sp += diff * status->int_;
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		status->speed -= cap_value(diff, 0, status->speed - 10);
	}

	if (flag&2 && battle_config.mob_size_influence) { // Change for sized monsters [Valaris]
		if (md->special_state.size == SZ_MEDIUM) {
			status->max_hp >>= 1;
			status->max_sp >>= 1;
			if (!status->max_hp) status->max_hp = 1;
			if (!status->max_sp) status->max_sp = 1;
			status->hp = status->max_hp;
			status->sp = status->max_sp;
			status->str >>= 1;
			status->agi >>= 1;
			status->vit >>= 1;
			status->int_ >>= 1;
			status->dex >>= 1;
			status->luk >>= 1;
			if (!status->str) status->str = 1;
			if (!status->agi) status->agi = 1;
			if (!status->vit) status->vit = 1;
			if (!status->int_) status->int_ = 1;
			if (!status->dex) status->dex = 1;
			if (!status->luk) status->luk = 1;
		} else if (md->special_state.size == SZ_BIG) {
			status->max_hp <<= 1;
			status->max_sp <<= 1;
			status->hp = status->max_hp;
			status->sp = status->max_sp;
			status->str <<= 1;
			status->agi <<= 1;
			status->vit <<= 1;
			status->int_ <<= 1;
			status->dex <<= 1;
			status->luk <<= 1;
		}
	}

	status_calc_misc(&md->bl, status, md->level);

	if(flag&4) { // Strengthen Guardians - custom value +10% / lv
		struct guild_castle *gc;
		struct map_data *mapdata = map_getmapdata(md->bl.m);

		gc=guild_mapname2gc(mapdata->name);
		if (!gc)
			ShowError("status_calc_mob: No castle set at map %s\n", mapdata->name);
		else if(gc->castle_id < 24 || md->mob_id == MOBID_EMPERIUM) {
#ifdef RENEWAL
			status->max_hp += 50 * (gc->defense / 5);
#else
			status->max_hp += 1000 * gc->defense;
#endif
			status->hp = status->max_hp;
			status->def += (gc->defense+2)/3;
			status->mdef += (gc->defense+2)/3;
		}
		if(md->mob_id != MOBID_EMPERIUM) {
			status->max_hp += 1000 * gc->defense;
			status->hp = status->max_hp;
			status->batk += 2 * md->guardian_data->guardup_lv + 8;
			status->rhw.atk += 2 * md->guardian_data->guardup_lv + 8;
			status->rhw.atk2 += 2 * md->guardian_data->guardup_lv + 8;
			status->aspd_rate -= 2 * md->guardian_data->guardup_lv + 3;
		}
	}

	if (flag&16 && mbl) { // Max HP setting from Summon Flora/marine Sphere
		struct unit_data *ud = unit_bl2ud(mbl);
		// Remove special AI when this is used by regular mobs.
		if (mbl->type == BL_MOB && !((TBL_MOB*)mbl)->special_state.ai)
			md->special_state.ai = AI_NONE;
		if (ud) { 
			// Different levels of HP according to skill level
			if(!ud->skill_id) // !FIXME: We lost the unit data for magic decoy in somewhere before this
				ud->skill_id = ((TBL_PC*)mbl)->menuskill_id;
			switch(ud->skill_id) {
				case AM_SPHEREMINE:
					status->max_hp = 2000 + 400*ud->skill_lv;
					break;
				case KO_ZANZOU:
					status->max_hp = 3000 + 3000 * ud->skill_lv;
					break;
				case AM_CANNIBALIZE:
					status->max_hp = 1500 + 200*ud->skill_lv + 10*status_get_lv(mbl);
					status->mode = static_cast<e_mode>(status->mode|MD_CANATTACK|MD_AGGRESSIVE);
					break;
				case MH_SUMMON_LEGION:
				{
					int homblvl = status_get_lv(mbl);

					status->max_hp = 10 * (100 * (ud->skill_lv + 2) + homblvl);
					status->batk = 100 * (ud->skill_lv+5) / 2;
					status->def = 10 * (100 * (ud->skill_lv+2) + homblvl);
					// status->aspd_rate = 10 * (2 * (20 - ud->skill_lv) - homblvl/10);
					// status->aspd_rate = max(100,status->aspd_rate);
					break;
				}
				case NC_SILVERSNIPER:
				{
					struct status_data *mstatus = status_get_status_data(mbl);
					if(!mstatus)
						break;
					status->max_hp = (1000 * ud->skill_lv) + (mstatus->hp / 3) + (status_get_lv(mbl) * 12);
					status->batk = 200 * ud->skill_lv;
					break;
				}
				case NC_MAGICDECOY:
				{
					struct status_data *mstatus = status_get_status_data(mbl);
					if(!mstatus)
						break;
					status->max_hp = (1000 * ((TBL_PC*)mbl)->menuskill_val) + (mstatus->sp * 4) + (status_get_lv(mbl) * 12);
					status->matk_min = status->matk_max = 250 + 50*((TBL_PC*)mbl)->menuskill_val;
					break;
				}
			}
			status->hp = status->max_hp;
		}
	}

	if (opt&SCO_FIRST) // Initial battle status
		memcpy(&md->status, status, sizeof(struct status_data));

	return 1;
}

/**
 * Calculates the stats of the given pet
 * @param pd: Pet object
 * @param opt: Whether or not it is the first calculation
		This will only be false when a pet levels up
 * @return 1
 * @author [Skotlex]
 */
void status_calc_pet_(struct pet_data *pd, enum e_status_calc_opt opt)
{
	nullpo_retv(pd);

	if (opt&SCO_FIRST) {
		memcpy(&pd->status, &pd->db->status, sizeof(struct status_data));
		pd->status.mode = MD_CANMOVE; // Pets discard all modes, except walking
		pd->status.class_ = CLASS_NORMAL;
		pd->status.speed = pd->get_pet_db()->speed;

		if(battle_config.pet_attack_support || battle_config.pet_damage_support) {
			// Attack support requires the pet to be able to attack
			pd->status.mode = static_cast<e_mode>(pd->status.mode|MD_CANATTACK);
		}
	}

	if (battle_config.pet_lv_rate && pd->master) {
		struct map_session_data *sd = pd->master;
		int lv;

		lv =sd->status.base_level*battle_config.pet_lv_rate/100;
		if (lv < 0)
			lv = 1;
		if (lv != pd->pet.level || opt&SCO_FIRST) {
			struct status_data *bstat = &pd->db->status, *status = &pd->status;

			pd->pet.level = lv;
			if (!(opt&SCO_FIRST)) // Lv Up animation
				clif_misceffect(&pd->bl, 0);
			status->rhw.atk = (bstat->rhw.atk*lv)/pd->db->lv;
			status->rhw.atk2 = (bstat->rhw.atk2*lv)/pd->db->lv;
			status->str = (bstat->str*lv)/pd->db->lv;
			status->agi = (bstat->agi*lv)/pd->db->lv;
			status->vit = (bstat->vit*lv)/pd->db->lv;
			status->int_ = (bstat->int_*lv)/pd->db->lv;
			status->dex = (bstat->dex*lv)/pd->db->lv;
			status->luk = (bstat->luk*lv)/pd->db->lv;

			status->rhw.atk = cap_value(status->rhw.atk, 1, battle_config.pet_max_atk1);
			status->rhw.atk2 = cap_value(status->rhw.atk2, 2, battle_config.pet_max_atk2);
			status->str = cap_value(status->str,1,battle_config.pet_max_stats);
			status->agi = cap_value(status->agi,1,battle_config.pet_max_stats);
			status->vit = cap_value(status->vit,1,battle_config.pet_max_stats);
			status->int_= cap_value(status->int_,1,battle_config.pet_max_stats);
			status->dex = cap_value(status->dex,1,battle_config.pet_max_stats);
			status->luk = cap_value(status->luk,1,battle_config.pet_max_stats);

			status_calc_misc(&pd->bl, &pd->status, lv);

			if (!(opt&SCO_FIRST)) // Not done the first time because the pet is not visible yet
				clif_send_petstatus(sd);
		}
	} else if (opt&SCO_FIRST) {
		status_calc_misc(&pd->bl, &pd->status, pd->db->lv);
		if (!battle_config.pet_lv_rate && pd->pet.level != pd->db->lv)
			pd->pet.level = pd->db->lv;
	}

	// Support rate modifier (1000 = 100%)
	pd->rate_fix = min(1000 * (pd->pet.intimate - battle_config.pet_support_min_friendly) / (1000 - battle_config.pet_support_min_friendly) + 500, USHRT_MAX);
	pd->rate_fix = min(apply_rate(pd->rate_fix, battle_config.pet_support_rate), USHRT_MAX);
}

/**
 * Get HP bonus modifiers
 * @param bl: block_list that will be checked
 * @param type: type of e_status_bonus (STATUS_BONUS_FIX or STATUS_BONUS_RATE)
 * @return bonus: total bonus for HP
 * @author [Cydh]
 */
static int status_get_hpbonus(struct block_list *bl, enum e_status_bonus type) {
	int bonus = 0;

	if (type == STATUS_BONUS_FIX) {
		struct status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			struct map_session_data *sd = map_id2sd(bl->id);
			uint8 i;

			bonus += sd->bonus.hp;
			if ((i = pc_checkskill(sd,CR_TRUST)) > 0)
				bonus += i * 200;
			if (pc_checkskill(sd,SU_SPRITEMABLE) > 0)
				bonus += 1000;
			if (pc_checkskill(sd, SU_POWEROFSEA) > 0) {
				bonus += 1000;
				if ((pc_checkskill(sd, SU_TUNABELLY) + pc_checkskill(sd, SU_TUNAPARTY) + pc_checkskill(sd, SU_BUNCHOFSHRIMP) + pc_checkskill(sd, SU_FRESHSHRIMP) +
					pc_checkskill(sd, SU_GROOMING) + pc_checkskill(sd, SU_PURRING) + pc_checkskill(sd, SU_SHRIMPARTY)) > 19)
						bonus += 2000;
			}
#ifndef HP_SP_TABLES
			if ((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 99)
				bonus += 2000; // Supernovice lvl99 hp bonus.
			if ((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && sd->status.base_level >= 150)
				bonus += 2000; // Supernovice lvl150 hp bonus.
#endif
		}

		//Bonus by SC
		if (sc) {
			if(sc->data[SC_INCMHP])
				bonus += sc->data[SC_INCMHP]->val1;
			if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2)
				bonus += 500;
			if(sc->data[SC_LERADSDEW])
				bonus += sc->data[SC_LERADSDEW]->val3;
			if (sc->data[SC_PROMOTE_HEALTH_RESERCH])
				bonus += sc->data[SC_PROMOTE_HEALTH_RESERCH]->val4;
			if(sc->data[SC_INSPIRATION])
				bonus += (600 * sc->data[SC_INSPIRATION]->val1);
			if(sc->data[SC_SOLID_SKIN_OPTION])
				bonus += 2000;
			if(sc->data[SC_MTF_MHP])
				bonus += sc->data[SC_MTF_MHP]->val1;
			if(sc->data[SC_MARIONETTE])
				bonus -= 1000;
			if(sc->data[SC_SWORDCLAN])
				bonus += 30;
			if(sc->data[SC_ARCWANDCLAN])
				bonus += 30;
			if(sc->data[SC_GOLDENMACECLAN])
				bonus += 30;
			if(sc->data[SC_CROSSBOWCLAN])
				bonus += 30;
			if(sc->data[SC_GLASTHEIM_HPSP])
				bonus += sc->data[SC_GLASTHEIM_HPSP]->val1;
		}
	} else if (type == STATUS_BONUS_RATE) {
		struct status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			struct map_session_data *sd = map_id2sd(bl->id);
			bonus += sd->hprate;
			bonus -= 100; //Default hprate is 100, so it should be add 0%
		}

		//Bonus by SC
		if (sc) {
			//Increasing
			if(sc->data[SC_INCMHPRATE])
				bonus += sc->data[SC_INCMHPRATE]->val1;
			if(sc->data[SC_APPLEIDUN])
				bonus += sc->data[SC_APPLEIDUN]->val2;
			if(sc->data[SC_DELUGE])
				bonus += sc->data[SC_DELUGE]->val2;
			if(sc->data[SC_BERSERK])
				bonus += 200; //+200%
			if(sc->data[SC_MERC_HPUP])
				bonus += sc->data[SC_MERC_HPUP]->val2;
			if(sc->data[SC_EPICLESIS])
				bonus += sc->data[SC_EPICLESIS]->val2;
			if(sc->data[SC_FRIGG_SONG])
				bonus += sc->data[SC_FRIGG_SONG]->val2;
			if(sc->data[SC_FORCEOFVANGUARD])
				bonus += (3 * sc->data[SC_FORCEOFVANGUARD]->val1);
			if(sc->data[SC_INSPIRATION])
				bonus += (5 * sc->data[SC_INSPIRATION]->val1);
			if(sc->data[SC_RAISINGDRAGON])
				bonus += (2 + sc->data[SC_RAISINGDRAGON]->val1);
			if(sc->data[SC_GT_REVITALIZE])
				bonus += sc->data[SC_GT_REVITALIZE]->val2;
			if(sc->data[SC_MUSTLE_M])
				bonus += sc->data[SC_MUSTLE_M]->val1;
			if(sc->data[SC_ANGRIFFS_MODUS])
				bonus += (5 * sc->data[SC_ANGRIFFS_MODUS]->val1);
			if(sc->data[SC_PETROLOGY_OPTION])
				bonus += sc->data[SC_PETROLOGY_OPTION]->val2;
			if(sc->data[SC_POWER_OF_GAIA])
				bonus += sc->data[SC_POWER_OF_GAIA]->val3;
			if(sc->data[SC_CURSED_SOIL_OPTION])
				bonus += sc->data[SC_CURSED_SOIL_OPTION]->val2;
			if(sc->data[SC_UPHEAVAL_OPTION])
				bonus += sc->data[SC_UPHEAVAL_OPTION]->val2;
			if(sc->data[SC_LAUDAAGNUS])
				bonus += 2 + (sc->data[SC_LAUDAAGNUS]->val1 * 2);

			//Decreasing
			if(sc->data[SC_VENOMBLEED])
				bonus -= 15;
			if(sc->data[SC_BEYONDOFWARCRY])
				bonus -= sc->data[SC_BEYONDOFWARCRY]->val4;
			if(sc->data[SC__WEAKNESS])
				bonus -= sc->data[SC__WEAKNESS]->val2;
			if(sc->data[SC_MYSTERIOUS_POWDER])
				bonus -= sc->data[SC_MYSTERIOUS_POWDER]->val1;
			if(sc->data[SC_GT_CHANGE]) // Max HP decrease: [Skill Level x 4] %
				bonus -= (4 * sc->data[SC_GT_CHANGE]->val1);
			if(sc->data[SC_EQC])
				bonus -= sc->data[SC_EQC]->val3;
		}
		// Max rate reduce is -100%
		bonus = cap_value(bonus,-100,INT_MAX);
	}

	return min(bonus,INT_MAX);
}

/**
 * Get SP bonus modifiers
 * @param bl: block_list that will be checked
 * @param type: type of e_status_bonus (STATUS_BONUS_FIX or STATUS_BONUS_RATE)
 * @return bonus: total bonus for SP
 * @author [Cydh]
 */
static int status_get_spbonus(struct block_list *bl, enum e_status_bonus type) {
	int bonus = 0;

	if (type == STATUS_BONUS_FIX) {
		struct status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			struct map_session_data *sd = map_id2sd(bl->id);
			uint8 i;

			bonus += sd->bonus.sp;
			if ((i = pc_checkskill(sd,SL_KAINA)) > 0)
				bonus += 30 * i;
			if ((i = pc_checkskill(sd,RA_RESEARCHTRAP)) > 0)
				bonus += 200 + 20 * i;
			if ((i = pc_checkskill(sd,WM_LESSON)) > 0)
				bonus += 30 * i;
			if (pc_checkskill(sd,SU_SPRITEMABLE) > 0)
				bonus += 100;
			if (pc_checkskill(sd, SU_POWEROFSEA) > 0) {
				bonus += 100;
				if ((pc_checkskill(sd, SU_TUNABELLY) + pc_checkskill(sd, SU_TUNAPARTY) + pc_checkskill(sd, SU_BUNCHOFSHRIMP) + pc_checkskill(sd, SU_FRESHSHRIMP) +
					pc_checkskill(sd, SU_GROOMING) + pc_checkskill(sd, SU_PURRING) + pc_checkskill(sd, SU_SHRIMPARTY)) > 19)
						bonus += 200;
			}
		}

		//Bonus by SC
		if (sc) {
			if(sc->data[SC_INCMSP])
				bonus += sc->data[SC_INCMSP]->val1;
			if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3)
				bonus += 50;
			if(sc->data[SC_MTF_MSP])
				bonus += sc->data[SC_MTF_MSP]->val1;
			if(sc->data[SC_SWORDCLAN])
				bonus += 10;
			if(sc->data[SC_ARCWANDCLAN])
				bonus += 10;
			if(sc->data[SC_GOLDENMACECLAN])
				bonus += 10;
			if(sc->data[SC_CROSSBOWCLAN])
				bonus += 10;
			if(sc->data[SC_GLASTHEIM_HPSP])
				bonus += sc->data[SC_GLASTHEIM_HPSP]->val2;
		}
	} else if (type == STATUS_BONUS_RATE) {
		struct status_change *sc = status_get_sc(bl);

		//Only for BL_PC
		if (bl->type == BL_PC) {
			struct map_session_data *sd = map_id2sd(bl->id);
			uint8 i;

			bonus += sd->sprate;
			bonus -= 100; //Default sprate is 100, so it should be add 0%

			if((i = pc_checkskill(sd,HP_MEDITATIO)) > 0)
				bonus += i;
			if((i = pc_checkskill(sd,HW_SOULDRAIN)) > 0)
				bonus += 2 * i;
		}

		//Bonus by SC
		if (sc) {
			if(sc->data[SC_INCMSPRATE])
				bonus += sc->data[SC_INCMSPRATE]->val1;
			if(sc->data[SC_RAISINGDRAGON])
				bonus += (2 + sc->data[SC_RAISINGDRAGON]->val1);
			if(sc->data[SC_SERVICE4U])
				bonus += sc->data[SC_SERVICE4U]->val2;
			if(sc->data[SC_MERC_SPUP])
				bonus += sc->data[SC_MERC_SPUP]->val2;
			if(sc->data[SC_LIFE_FORCE_F])
				bonus += sc->data[SC_LIFE_FORCE_F]->val1;
			if(sc->data[SC_VITATA_500])
				bonus += sc->data[SC_VITATA_500]->val2;
			if (sc->data[SC_ENERGY_DRINK_RESERCH])
				bonus += sc->data[SC_ENERGY_DRINK_RESERCH]->val4;
		}
		// Max rate reduce is -100%
		bonus = cap_value(bonus,-100,INT_MAX);
	}

	return min(bonus,INT_MAX);
}

/**
 * Get final MaxHP or MaxSP for player. References: http://irowiki.org/wiki/Max_HP and http://irowiki.org/wiki/Max_SP
 * The calculation needs base_level, base_status/battle_status (vit or int), additive modifier, and multiplicative modifier
 * @param sd Player
 * @param stat Vit/Int of player as param modifier
 * @param isHP true - calculates Max HP, false - calculated Max SP
 * @return max The max value of HP or SP
 */
static unsigned int status_calc_maxhpsp_pc(struct map_session_data* sd, unsigned int stat, bool isHP) {
	double dmax = 0;
	uint16 idx, level, job_id;

	nullpo_ret(sd);

	job_id = pc_mapid2jobid(sd->class_,sd->status.sex);
	idx = pc_class2idx(job_id);
	level = umax(sd->status.base_level,1);

	if (isHP) { //Calculates MaxHP
		dmax = job_info[idx].base_hp[level-1] * (1 + (umax(stat,1) * 0.01)) * ((sd->class_&JOBL_UPPER)?1.25:(pc_is_taekwon_ranker(sd))?3:1);
		dmax += status_get_hpbonus(&sd->bl,STATUS_BONUS_FIX);
		dmax += (int64)(dmax * status_get_hpbonus(&sd->bl,STATUS_BONUS_RATE) / 100); //Aegis accuracy
	}
	else { //Calculates MaxSP
		dmax = job_info[idx].base_sp[level-1] * (1 + (umax(stat,1) * 0.01)) * ((sd->class_&JOBL_UPPER)?1.25:(pc_is_taekwon_ranker(sd))?3:1);
		dmax += status_get_spbonus(&sd->bl,STATUS_BONUS_FIX);
		dmax += (int64)(dmax * status_get_spbonus(&sd->bl,STATUS_BONUS_RATE) / 100); //Aegis accuracy
	}

	//Make sure it's not negative before casting to unsigned int
	if(dmax < 1) dmax = 1;

	return cap_value((unsigned int)dmax,1,UINT_MAX);
}

/**
 * Calculates player's weight
 * @param sd: Player object
 * @param flag: Calculation type
 *   CALCWT_ITEM - Item weight
 *   CALCWT_MAXBONUS - Skill/Status/Configuration max weight bonus
 * @return false - failed, true - success
 */
bool status_calc_weight(struct map_session_data *sd, enum e_status_calc_weight_opt flag)
{
	int b_weight, b_max_weight, skill, i;
	struct status_change *sc;

	nullpo_retr(false, sd);

	sc = &sd->sc;
	b_max_weight = sd->max_weight; // Store max weight for later comparison
	b_weight = sd->weight; // Store current weight for later comparison
	sd->max_weight = job_info[pc_class2idx(sd->status.class_)].max_weight_base + sd->status.str * 300; // Recalculate max weight

	if (flag&CALCWT_ITEM) {
		sd->weight = 0; // Reset current weight

		for(i = 0; i < MAX_INVENTORY; i++) {
			if (!sd->inventory.u.items_inventory[i].nameid || sd->inventory_data[i] == NULL)
				continue;
			sd->weight += sd->inventory_data[i]->weight * sd->inventory.u.items_inventory[i].amount;
		}
	}

	if (flag&CALCWT_MAXBONUS) {
		// Skill/Status bonus weight increases
		sd->max_weight += sd->add_max_weight; // From bAddMaxWeight
		if ((skill = pc_checkskill(sd, MC_INCCARRY)) > 0)
			sd->max_weight += 2000 * skill;
		if (pc_isriding(sd) && pc_checkskill(sd, KN_RIDING) > 0)
			sd->max_weight += 10000;
		else if (pc_isridingdragon(sd))
			sd->max_weight += 5000 + 2000 * pc_checkskill(sd, RK_DRAGONTRAINING);
		if (sc->data[SC_KNOWLEDGE])
			sd->max_weight += sd->max_weight * sc->data[SC_KNOWLEDGE]->val1 / 10;
		if ((skill = pc_checkskill(sd, ALL_INCCARRY)) > 0)
			sd->max_weight += 2000 * skill;
		if (pc_ismadogear(sd))
			sd->max_weight += 15000;
	}

	// Update the client if the new weight calculations don't match
	if (b_weight != sd->weight)
		clif_updatestatus(sd, SP_WEIGHT);
	if (b_max_weight != sd->max_weight) {
		clif_updatestatus(sd, SP_MAXWEIGHT);
		pc_updateweightstatus(sd);
	}

	return true;
}

/**
 * Calculates player's cart weight
 * @param sd: Player object
 * @param flag: Calculation type
 *   CALCWT_ITEM - Cart item weight
 *   CALCWT_MAXBONUS - Skill/Status/Configuration max weight bonus
 *   CALCWT_CARTSTATE - Whether to check for cart state
 * @return false - failed, true - success
 */
bool status_calc_cart_weight(struct map_session_data *sd, enum e_status_calc_weight_opt flag)
{
	int b_cart_weight_max, i;

	nullpo_retr(false, sd);

	if (!pc_iscarton(sd) && !(flag&CALCWT_CARTSTATE))
		return false;

	b_cart_weight_max = sd->cart_weight_max; // Store cart max weight for later comparison
	sd->cart_weight_max = battle_config.max_cart_weight; // Recalculate max weight

	if (flag&CALCWT_ITEM) {
		sd->cart_weight = 0; // Reset current cart weight
		sd->cart_num = 0; // Reset current cart item count

		for(i = 0; i < MAX_CART; i++) {
			if (!sd->cart.u.items_cart[i].nameid)
				continue;
			sd->cart_weight += itemdb_weight(sd->cart.u.items_cart[i].nameid) * sd->cart.u.items_cart[i].amount; // Recalculate current cart weight
			sd->cart_num++; // Recalculate current cart item count
		}
	}

	// Skill bonus max weight increases
	if (flag&CALCWT_MAXBONUS)
		sd->cart_weight_max += (pc_checkskill(sd, GN_REMODELING_CART) * 5000);

	// Update the client if the new weight calculations don't match
	if (b_cart_weight_max != sd->cart_weight_max)
		clif_updatestatus(sd, SP_CARTINFO);

	return true;
}

/**
 * Calculates player data from scratch without counting SC adjustments
 * Should be invoked whenever players raise stats, learn passive skills or change equipment
 * @param sd: Player object
 * @param opt: Whether it is first calc (login) or not
 * @return (-1) for too many recursive calls, (1) recursive call, (0) success
 */
int status_calc_pc_sub(struct map_session_data* sd, enum e_status_calc_opt opt)
{
	static int calculating = 0; ///< Check for recursive call preemption. [Skotlex]
	struct status_data *base_status; ///< Pointer to the player's base status
	const struct status_change *sc = &sd->sc;
	struct s_skill b_skill[MAX_SKILL]; ///< Previous skill tree
	int i, skill, refinedef = 0;
	short index = -1;

	if (++calculating > 10) // Too many recursive calls!
		return -1;

	// Remember player-specific values that are currently being shown to the client (for refresh purposes)
	memcpy(b_skill, &sd->status.skill, sizeof(b_skill));

	pc_calc_skilltree(sd);	// SkillTree calculation

	if (opt&SCO_FIRST) {
		// Load Hp/SP from char-received data.
		sd->battle_status.hp = sd->status.hp;
		sd->battle_status.sp = sd->status.sp;
		sd->regen.sregen = &sd->sregen;
		sd->regen.ssregen = &sd->ssregen;
	}

	base_status = &sd->base_status;
	// These are not zeroed. [zzo]
	sd->hprate = 100;
	sd->sprate = 100;
	sd->castrate = 100;
	sd->delayrate = 100;
	sd->dsprate = 100;
	sd->hprecov_rate = 100;
	sd->sprecov_rate = 100;
	sd->matk_rate = 100;
	sd->critical_rate = sd->hit_rate = sd->flee_rate = sd->flee2_rate = 100;
	sd->def_rate = sd->def2_rate = sd->mdef_rate = sd->mdef2_rate = 100;
	sd->regen.state.block = 0;
	sd->add_max_weight = 0;

	// Zeroed arrays, order follows the order in pc.hpp.
	// Add new arrays to the end of zeroed area in pc.hpp (see comments) and size here. [zzo]
	memset (sd->param_bonus, 0, sizeof(sd->param_bonus)
		+ sizeof(sd->param_equip)
		+ sizeof(sd->subele)
		+ sizeof(sd->subele_script)
		+ sizeof(sd->subdefele)
		+ sizeof(sd->subrace)
		+ sizeof(sd->subclass)
		+ sizeof(sd->subrace2)
		+ sizeof(sd->subsize)
		+ sizeof(sd->coma_class)
		+ sizeof(sd->coma_race)
		+ sizeof(sd->weapon_coma_ele)
		+ sizeof(sd->weapon_coma_race)
		+ sizeof(sd->weapon_coma_class)
		+ sizeof(sd->weapon_atk)
		+ sizeof(sd->weapon_damage_rate)
		+ sizeof(sd->arrow_addele)
		+ sizeof(sd->arrow_addrace)
		+ sizeof(sd->arrow_addclass)
		+ sizeof(sd->arrow_addsize)
		+ sizeof(sd->magic_addele)
		+ sizeof(sd->magic_addele_script)
		+ sizeof(sd->magic_addrace)
		+ sizeof(sd->magic_addclass)
		+ sizeof(sd->magic_addsize)
		+ sizeof(sd->magic_atk_ele)
		+ sizeof(sd->critaddrace)
		+ sizeof(sd->expaddrace)
		+ sizeof(sd->expaddclass)
		+ sizeof(sd->ignore_mdef_by_race)
		+ sizeof(sd->ignore_mdef_by_class)
		+ sizeof(sd->ignore_def_by_race)
		+ sizeof(sd->ignore_def_by_class)
		+ sizeof(sd->sp_gain_race)
		+ sizeof(sd->magic_addrace2)
		+ sizeof(sd->ignore_mdef_by_race2)
		+ sizeof(sd->dropaddrace)
		+ sizeof(sd->dropaddclass)
		);

	memset (&sd->right_weapon.overrefine, 0, sizeof(sd->right_weapon) - sizeof(sd->right_weapon.atkmods));
	memset (&sd->left_weapon.overrefine, 0, sizeof(sd->left_weapon) - sizeof(sd->left_weapon.atkmods));

	if (sd->special_state.intravision && !sd->sc.data[SC_INTRAVISION]) // Clear intravision as long as nothing else is using it
		clif_status_load(&sd->bl, EFST_CLAIRVOYANCE, 0);

	memset(&sd->special_state,0,sizeof(sd->special_state));

	if (pc_isvip(sd)) // Magic Stone requirement avoidance for VIP.
		sd->special_state.no_gemstone = battle_config.vip_gemstone;

	if (!sd->state.permanent_speed) {
		memset(&base_status->max_hp, 0, sizeof(struct status_data)-(sizeof(base_status->hp)+sizeof(base_status->sp)));
		base_status->speed = DEFAULT_WALK_SPEED;
	} else {
		int pSpeed = base_status->speed;

		memset(&base_status->max_hp, 0, sizeof(struct status_data)-(sizeof(base_status->hp)+sizeof(base_status->sp)));
		base_status->speed = pSpeed;
	}

	// !FIXME: Most of these stuff should be calculated once, but how do I fix the memset above to do that? [Skotlex]
	// Give them all modes except these (useful for clones)
	base_status->mode = static_cast<e_mode>(MD_MASK&~(MD_STATUS_IMMUNE|MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC|MD_DETECTOR|MD_ANGRY|MD_TARGETWEAK));

	base_status->size = (sd->class_&JOBL_BABY || ((battle_config.summoner_trait&2) && (sd->class_&MAPID_BASEMASK) == MAPID_SUMMONER)) ? SZ_SMALL : SZ_MEDIUM;
	if (battle_config.character_size && pc_isriding(sd)) { // [Lupus]
		if (sd->class_&JOBL_BABY) {
			if (battle_config.character_size&SZ_BIG)
				base_status->size++;
		} else
		if(battle_config.character_size&SZ_MEDIUM)
			base_status->size++;
	}
	base_status->aspd_rate = 1000;
	base_status->ele_lv = 1;
	base_status->race = ((battle_config.summoner_trait&1) && (sd->class_&MAPID_BASEMASK) == MAPID_SUMMONER) ? RC_BRUTE : RC_PLAYER;
	base_status->class_ = CLASS_NORMAL;

	sd->autospell.clear();
	sd->autospell2.clear();
	sd->autospell3.clear();
	sd->addeff.clear();
	sd->addeff_atked.clear();
	sd->addeff_onskill.clear();
	sd->skillatk.clear();
	sd->skillusesprate.clear();
	sd->skillusesp.clear();
	sd->skillheal.clear();
	sd->skillheal2.clear();
	sd->skillblown.clear();
	sd->skillcastrate.clear();
	sd->skillfixcastrate.clear();
	sd->subskill.clear();
	sd->skillcooldown.clear();
	sd->skillfixcast.clear();
	sd->skillvarcast.clear();
	sd->add_def.clear();
	sd->add_mdef.clear();
	sd->add_mdmg.clear();
	sd->reseff.clear();
	sd->itemgrouphealrate.clear();
	sd->add_drop.clear();
	sd->itemhealrate.clear();
	sd->subele2.clear();
	sd->skilldelay.clear();

	// Zero up structures...
	memset(&sd->hp_loss, 0, sizeof(sd->hp_loss)
		+ sizeof(sd->sp_loss)
		+ sizeof(sd->hp_regen)
		+ sizeof(sd->sp_regen)
		+ sizeof(sd->percent_hp_regen)
		+ sizeof(sd->percent_sp_regen)
		+ sizeof(sd->def_set_race)
		+ sizeof(sd->mdef_set_race)
		+ sizeof(sd->norecover_state_race)
		+ sizeof(sd->hp_vanish_race)
		+ sizeof(sd->sp_vanish_race)
	);

	memset(&sd->bonus, 0, sizeof(sd->bonus));

	// Autobonus
	pc_delautobonus(sd, sd->autobonus, true);
	pc_delautobonus(sd, sd->autobonus2, true);
	pc_delautobonus(sd, sd->autobonus3, true);

	running_npc_stat_calc_event = true;
	npc_script_event(sd, NPCE_STATCALC);
	running_npc_stat_calc_event = false;

	// Parse equipment
	for (i = 0; i < EQI_MAX; i++) {
		current_equip_item_index = index = sd->equip_index[i]; // We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		current_equip_combo_pos = 0;
		if (index < 0)
			continue;
		if (i == EQI_AMMO)
			continue;
		if (pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index))
			continue;
		if (!sd->inventory_data[index])
			continue;

		base_status->def += sd->inventory_data[index]->def;

		// Items may be equipped, their effects however are nullified.
		if (opt&SCO_FIRST && sd->inventory_data[index]->equip_script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT)
			|| !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) { // Execute equip-script on login
			run_script(sd->inventory_data[index]->equip_script,0,sd->bl.id,0);
			if (!calculating)
				return 1;
		}

		// Sanitize the refine level in case someone decreased the value inbetween
		if (sd->inventory.u.items_inventory[index].refine > MAX_REFINE)
			sd->inventory.u.items_inventory[index].refine = MAX_REFINE;

		if (sd->inventory_data[index]->type == IT_WEAPON) {
			int r = sd->inventory.u.items_inventory[index].refine, wlv = sd->inventory_data[index]->wlv;
			struct weapon_data *wd;
			struct weapon_atk *wa;

			if(wlv >= REFINE_TYPE_MAX)
				wlv = REFINE_TYPE_MAX - 1;
			if(i == EQI_HAND_L && sd->inventory.u.items_inventory[index].equip == EQP_HAND_L) {
				wd = &sd->left_weapon; // Left-hand weapon
				wa = &base_status->lhw;
			} else {
				wd = &sd->right_weapon;
				wa = &base_status->rhw;
			}
			wa->atk += sd->inventory_data[index]->atk;
			if(r)
				wa->atk2 = refine_info[wlv].bonus[r-1] / 100;
#ifdef RENEWAL
			wa->matk += sd->inventory_data[index]->matk;
			wa->wlv = wlv;
			if(r && sd->weapontype1 != W_BOW) // Renewal magic attack refine bonus
				wa->matk += refine_info[wlv].bonus[r-1] / 100;
#endif
			if(r) // Overrefine bonus.
				wd->overrefine = refine_info[wlv].randombonus_max[r-1] / 100;
			wa->range += sd->inventory_data[index]->range;
			if(sd->inventory_data[index]->script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) {
				if (wd == &sd->left_weapon) {
					sd->state.lr_flag = 1;
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if (!calculating) // Abort, run_script retriggered this. [Skotlex]
					return 1;
			}
#ifdef RENEWAL
			if (sd->bonus.weapon_matk_rate)
				wa->matk += wa->matk * sd->bonus.weapon_matk_rate / 100;
#endif
			if(sd->inventory.u.items_inventory[index].card[0] == CARD0_FORGE) { // Forged weapon
				wd->star += (sd->inventory.u.items_inventory[index].card[1]>>8);
				if(wd->star >= 15) wd->star = 40; // 3 Star Crumbs now give +40 dmg
				if(pc_famerank(MakeDWord(sd->inventory.u.items_inventory[index].card[2],sd->inventory.u.items_inventory[index].card[3]) ,MAPID_BLACKSMITH))
					wd->star += 10;
				if (!wa->ele) // Do not overwrite element from previous bonuses.
					wa->ele = (sd->inventory.u.items_inventory[index].card[1]&0x0f);
			}
		} else if(sd->inventory_data[index]->type == IT_ARMOR) {
			int r;

			if ( (r = sd->inventory.u.items_inventory[index].refine) )
				refinedef += refine_info[REFINE_TYPE_ARMOR].bonus[r-1];
			if(sd->inventory_data[index]->script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) {
				if( i == EQI_HAND_L ) // Shield
					sd->state.lr_flag = 3;
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if( i == EQI_HAND_L ) // Shield
					sd->state.lr_flag = 0;
				if (!calculating) // Abort, run_script retriggered this. [Skotlex]
					return 1;
			}
		} else if( sd->inventory_data[index]->type == IT_SHADOWGEAR ) { // Shadow System
			if (sd->inventory_data[index]->script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(sd->inventory_data[index],sd->bl.m))) {
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
				if( !calculating )
					return 1;
			}
		}
	}

	if(sd->equip_index[EQI_AMMO] >= 0) {
		index = sd->equip_index[EQI_AMMO];
		if(sd->inventory_data[index]) { // Arrows
			sd->bonus.arrow_atk += sd->inventory_data[index]->atk;
			sd->state.lr_flag = 2;
			if( !itemdb_group_item_exists(IG_THROWABLE, sd->inventory_data[index]->nameid) ) // Don't run scripts on throwable items
				run_script(sd->inventory_data[index]->script,0,sd->bl.id,0);
			sd->state.lr_flag = 0;
			if (!calculating) // Abort, run_script retriggered status_calc_pc. [Skotlex]
				return 1;
		}
	}

	// We've got combos to process and check
	if( sd->combos.count ) {
		for (i = 0; i < sd->combos.count; i++) {
			uint8 j = 0;
			bool no_run = false;
			struct item_combo *combo = NULL;

			current_equip_item_index = -1;
			current_equip_combo_pos = sd->combos.pos[i];

			if (!sd->combos.bonus[i] || !(combo = itemdb_combo_exists(sd->combos.id[i])))
				continue;
			// Check combo items
			while (j < combo->count) {
				struct item_data *id = itemdb_exists(combo->nameid[j]);
				// Don't run the script if at least one of combo's pair has restriction
				if (id && !pc_has_permission(sd, PC_PERM_USE_ALL_EQUIPMENT) && itemdb_isNoEquip(id, sd->bl.m)) {
					no_run = true;
					break;
				}
				j++;
			}
			if (no_run)
				continue;
			run_script(sd->combos.bonus[i],0,sd->bl.id,0);
			if (!calculating) // Abort, run_script retriggered this
				return 1;
		}
	}

	// Store equipment script bonuses
	memcpy(sd->param_equip,sd->param_bonus,sizeof(sd->param_equip));
	memset(sd->param_bonus, 0, sizeof(sd->param_bonus));

	base_status->def += (refinedef+50)/100;

	// Parse Cards
	for (i = 0; i < EQI_MAX; i++) {
		current_equip_item_index = index = sd->equip_index[i]; // We pass INDEX to current_equip_item_index - for EQUIP_SCRIPT (new cards solution) [Lupus]
		current_equip_combo_pos = 0;
		if (index < 0)
			continue;
		if (i == EQI_AMMO)
			continue;
		if (pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index))
			continue;

		if (sd->inventory_data[index]) {
			int j;
			struct item_data *data;

			// Card script execution.
			if (itemdb_isspecial(sd->inventory.u.items_inventory[index].card[0]))
				continue;
			for (j = 0; j < MAX_SLOTS; j++) { // Uses MAX_SLOTS to support Soul Bound system [Inkfish]
				int c = sd->inventory.u.items_inventory[index].card[j];
				current_equip_card_id= c;
				if(!c)
					continue;
				data = itemdb_exists(c);
				if(!data)
					continue;
				if (opt&SCO_FIRST && data->equip_script && (pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) || !itemdb_isNoEquip(data,sd->bl.m))) {// Execute equip-script on login
					run_script(data->equip_script,0,sd->bl.id,0);
					if (!calculating)
						return 1;
				}
				if(!data->script)
					continue;
				if(!pc_has_permission(sd,PC_PERM_USE_ALL_EQUIPMENT) && itemdb_isNoEquip(data,sd->bl.m)) // Card restriction checks.
					continue;
				if(i == EQI_HAND_L && sd->inventory.u.items_inventory[index].equip == EQP_HAND_L) { // Left hand status.
					sd->state.lr_flag = 1;
					run_script(data->script,0,sd->bl.id,0);
					sd->state.lr_flag = 0;
				} else
					run_script(data->script,0,sd->bl.id,0);
				if (!calculating) // Abort, run_script his function. [Skotlex]
					return 1;
			}
		}
	}
	current_equip_card_id = 0; // Clear stored card ID [Secret]

	// Parse random options
	for (i = 0; i < EQI_MAX; i++) {
		current_equip_item_index = index = sd->equip_index[i];
		current_equip_combo_pos = 0;
		current_equip_opt_index = -1;

		if (index < 0)
			continue;
		if (i == EQI_AMMO)
			continue;
		if (pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index))
			continue;
		
		if (sd->inventory_data[index]) {
			int j;
			struct s_random_opt_data *data;
			for (j = 0; j < MAX_ITEM_RDM_OPT; j++) {
				short opt_id = sd->inventory.u.items_inventory[index].option[j].id;

				if (!opt_id)
					continue;
				current_equip_opt_index = j;
				data = itemdb_randomopt_exists(opt_id);
				if (!data || !data->script)
					continue;
				if (!pc_has_permission(sd, PC_PERM_USE_ALL_EQUIPMENT) && itemdb_isNoEquip(sd->inventory_data[index], sd->bl.m))
					continue;
				if (i == EQI_HAND_L && sd->inventory.u.items_inventory[index].equip == EQP_HAND_L) { // Left hand status.
					sd->state.lr_flag = 1;
					run_script(data->script, 0, sd->bl.id, 0);
					sd->state.lr_flag = 0;
				}
				else
					run_script(data->script, 0, sd->bl.id, 0);
				if (!calculating)
					return 1;
			}
		}
		current_equip_opt_index = -1;
	}

	if (sc->count && sc->data[SC_ITEMSCRIPT]) {
		struct item_data *data = itemdb_exists(sc->data[SC_ITEMSCRIPT]->val1);
		if (data && data->script)
			run_script(data->script, 0, sd->bl.id, 0);
	}

	pc_bonus_script(sd);

	if (sd->sc.count) { // Script bonus from SC [Cydh]
		struct script_code *script = NULL;
		for (i = 0; i < SC_MAX; i++) {
			if (!sc->data[i] || !(script = status_sc_get_script((sc_type)i)))
				continue;
			run_script(script, 0, sd->bl.id, 0);
			if (!calculating)
				return 1;
		}
	}

	if( sd->pd ) { // Pet Bonus
		struct pet_data *pd = sd->pd;
		s_pet_db *pet_db_ptr = pd->get_pet_db();

		if( pet_db_ptr != nullptr && pet_db_ptr->pet_loyal_script && pd->pet.intimate >= battle_config.pet_equip_min_friendly )
			run_script(pd->get_pet_db()->pet_loyal_script,0,sd->bl.id,0);
		if( pd->pet.intimate > 0 && (!battle_config.pet_equip_required || pd->pet.equip > 0) && pd->state.skillbonus == 1 && pd->bonus )
			pc_bonus(sd,pd->bonus->type, pd->bonus->val);
	}

	// param_bonus now holds card bonuses.
	if(base_status->rhw.range < 1) base_status->rhw.range = 1;
	if(base_status->lhw.range < 1) base_status->lhw.range = 1;
	if(base_status->rhw.range < base_status->lhw.range)
		base_status->rhw.range = base_status->lhw.range;

	sd->bonus.double_rate += sd->bonus.double_add_rate;
	sd->bonus.perfect_hit += sd->bonus.perfect_hit_add;
	sd->bonus.splash_range += sd->bonus.splash_add_range;

	// Damage modifiers from weapon type
	sd->right_weapon.atkmods[SZ_SMALL] = atkmods[SZ_SMALL][sd->weapontype1];
	sd->right_weapon.atkmods[SZ_MEDIUM] = atkmods[SZ_MEDIUM][sd->weapontype1];
	sd->right_weapon.atkmods[SZ_BIG] = atkmods[SZ_BIG][sd->weapontype1];
	sd->left_weapon.atkmods[SZ_SMALL] = atkmods[SZ_SMALL][sd->weapontype2];
	sd->left_weapon.atkmods[SZ_MEDIUM] = atkmods[SZ_MEDIUM][sd->weapontype2];
	sd->left_weapon.atkmods[SZ_BIG] = atkmods[SZ_BIG][sd->weapontype2];

	if((pc_isriding(sd) || pc_isridingdragon(sd)) &&
		(sd->status.weapon==W_1HSPEAR || sd->status.weapon==W_2HSPEAR))
	{	// When Riding with spear, damage modifier to mid-class becomes
		// same as versus large size.
		sd->right_weapon.atkmods[SZ_MEDIUM] = sd->right_weapon.atkmods[SZ_BIG];
		sd->left_weapon.atkmods[SZ_MEDIUM] = sd->left_weapon.atkmods[SZ_BIG];
	}

// ----- STATS CALCULATION -----

	// Job bonuses
	index = pc_class2idx(sd->status.class_);
	for(i=0;i<(int)sd->status.job_level && i<MAX_LEVEL;i++) {
		if(!job_info[index].job_bonus[i])
			continue;
		switch(job_info[index].job_bonus[i]) {
			case 1: base_status->str++; break;
			case 2: base_status->agi++; break;
			case 3: base_status->vit++; break;
			case 4: base_status->int_++; break;
			case 5: base_status->dex++; break;
			case 6: base_status->luk++; break;
		}
	}

	// If a Super Novice has never died and is at least joblv 70, he gets all stats +10
	if(((sd->class_&MAPID_UPPERMASK) == MAPID_SUPER_NOVICE && (sd->status.job_level >= 70  || sd->class_&JOBL_THIRD)) && sd->die_counter == 0) {
		base_status->str += 10;
		base_status->agi += 10;
		base_status->vit += 10;
		base_status->int_+= 10;
		base_status->dex += 10;
		base_status->luk += 10;
	}

	// Absolute modifiers from passive skills
	if(pc_checkskill(sd,BS_HILTBINDING)>0)
		base_status->str++;
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0)
		base_status->int_ += (skill+1)/2; // +1 INT / 2 lv
	if((skill=pc_checkskill(sd,AC_OWL))>0)
		base_status->dex += skill;
	if((skill = pc_checkskill(sd,RA_RESEARCHTRAP))>0)
		base_status->int_ += skill;
	if (pc_checkskill(sd, SU_POWEROFLAND) > 0)
		base_status->int_ += 20;

	// Bonuses from cards and equipment as well as base stat, remember to avoid overflows.
	i = base_status->str + sd->status.str + sd->param_bonus[0] + sd->param_equip[0];
	base_status->str = cap_value(i,0,USHRT_MAX);
	i = base_status->agi + sd->status.agi + sd->param_bonus[1] + sd->param_equip[1];
	base_status->agi = cap_value(i,0,USHRT_MAX);
	i = base_status->vit + sd->status.vit + sd->param_bonus[2] + sd->param_equip[2];
	base_status->vit = cap_value(i,0,USHRT_MAX);
	i = base_status->int_+ sd->status.int_+ sd->param_bonus[3] + sd->param_equip[3];
	base_status->int_ = cap_value(i,0,USHRT_MAX);
	i = base_status->dex + sd->status.dex + sd->param_bonus[4] + sd->param_equip[4];
	base_status->dex = cap_value(i,0,USHRT_MAX);
	i = base_status->luk + sd->status.luk + sd->param_bonus[5] + sd->param_equip[5];
	base_status->luk = cap_value(i,0,USHRT_MAX);

// ------ ATTACK CALCULATION ------

	// Base batk value is set in status_calc_misc
#ifndef RENEWAL
	// !FIXME: Weapon-type bonus (Why is the weapon_atk bonus applied to base attack?)
	if (sd->status.weapon < MAX_WEAPON_TYPE && sd->weapon_atk[sd->status.weapon])
		base_status->batk += sd->weapon_atk[sd->status.weapon];
	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,BS_HILTBINDING))>0)
		base_status->batk += 4;
#else
	base_status->watk = status_weapon_atk(base_status->rhw, sd);
	base_status->watk2 = status_weapon_atk(base_status->lhw, sd);
	base_status->eatk = sd->bonus.eatk;
#endif

// ----- HP MAX CALCULATION -----
	base_status->max_hp = sd->status.max_hp = status_calc_maxhpsp_pc(sd,base_status->vit,true);

	if(battle_config.hp_rate != 100)
		base_status->max_hp = (unsigned int)(battle_config.hp_rate * (base_status->max_hp/100.));

	if (sd->status.base_level < 100)
		base_status->max_hp = cap_value(base_status->max_hp,1,(unsigned int)battle_config.max_hp_lv99);
	else if (sd->status.base_level < 151)
		base_status->max_hp = cap_value(base_status->max_hp,1,(unsigned int)battle_config.max_hp_lv150);
	else
		base_status->max_hp = cap_value(base_status->max_hp,1,(unsigned int)battle_config.max_hp);

// ----- SP MAX CALCULATION -----
	base_status->max_sp = sd->status.max_sp = status_calc_maxhpsp_pc(sd,base_status->int_,false);

	if(battle_config.sp_rate != 100)
		base_status->max_sp = (unsigned int)(battle_config.sp_rate * (base_status->max_sp/100.));

	base_status->max_sp = cap_value(base_status->max_sp,1,(unsigned int)battle_config.max_sp);

// ----- RESPAWN HP/SP -----

	// Calc respawn hp and store it on base_status
	if (sd->special_state.restart_full_recover) {
		base_status->hp = base_status->max_hp;
		base_status->sp = base_status->max_sp;
	} else {
		if((sd->class_&MAPID_BASEMASK) == MAPID_NOVICE && !(sd->class_&JOBL_2)
			&& battle_config.restart_hp_rate < 50)
			base_status->hp = base_status->max_hp>>1;
		else
			base_status->hp = (int64)base_status->max_hp * battle_config.restart_hp_rate/100;
		if(!base_status->hp)
			base_status->hp = 1;

		base_status->sp = (int64)base_status->max_sp * battle_config.restart_sp_rate /100;

		if( !base_status->sp ) // The minimum for the respawn setting is SP:1
			base_status->sp = 1;
	}

// ----- MISC CALCULATION -----
	status_calc_misc(&sd->bl, base_status, sd->status.base_level);

	// Equipment modifiers for misc settings
	if(sd->matk_rate < 0)
		sd->matk_rate = 0;

	if(sd->matk_rate != 100) {
		base_status->matk_max = base_status->matk_max * sd->matk_rate/100;
		base_status->matk_min = base_status->matk_min * sd->matk_rate/100;
	}

	if(sd->hit_rate < 0)
		sd->hit_rate = 0;
	if(sd->hit_rate != 100)
		base_status->hit = base_status->hit * sd->hit_rate/100;

	if(sd->flee_rate < 0)
		sd->flee_rate = 0;
	if(sd->flee_rate != 100)
		base_status->flee = base_status->flee * sd->flee_rate/100;

	if(sd->def2_rate < 0)
		sd->def2_rate = 0;
	if(sd->def2_rate != 100)
		base_status->def2 = base_status->def2 * sd->def2_rate/100;

	if(sd->mdef2_rate < 0)
		sd->mdef2_rate = 0;
	if(sd->mdef2_rate != 100)
		base_status->mdef2 = base_status->mdef2 * sd->mdef2_rate/100;

	if(sd->critical_rate < 0)
		sd->critical_rate = 0;
	if(sd->critical_rate != 100)
		base_status->cri = cap_value(base_status->cri * sd->critical_rate/100,SHRT_MIN,SHRT_MAX);
	if (pc_checkskill(sd, SU_POWEROFLIFE) > 0)
		base_status->cri += 200;

	if(sd->flee2_rate < 0)
		sd->flee2_rate = 0;
	if(sd->flee2_rate != 100)
		base_status->flee2 = base_status->flee2 * sd->flee2_rate/100;

// ----- HIT CALCULATION -----

	// Absolute modifiers from passive skills
#ifndef RENEWAL
	if((skill=pc_checkskill(sd,BS_WEAPONRESEARCH))>0)
		base_status->hit += skill*2;
#endif
	if((skill=pc_checkskill(sd,AC_VULTURE))>0) {
#ifndef RENEWAL
		base_status->hit += skill;
#endif
		if(sd->status.weapon == W_BOW)
			base_status->rhw.range += skill;
	}
	if(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE) {
		if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0)
			base_status->hit += 2*skill;
		if((skill=pc_checkskill(sd,GS_SNAKEEYE))>0) {
			base_status->hit += skill;
			base_status->rhw.range += skill;
		}
	}
	if((sd->status.weapon == W_1HAXE || sd->status.weapon == W_2HAXE) && (skill = pc_checkskill(sd,NC_TRAININGAXE)) > 0)
		base_status->hit += skill * 3;
	if((sd->status.weapon == W_MACE || sd->status.weapon == W_2HMACE) && (skill = pc_checkskill(sd,NC_TRAININGAXE)) > 0)
		base_status->hit += skill * 2;
	if (pc_checkskill(sd, SU_POWEROFLIFE) > 0)
		base_status->hit += 20;

	if ((skill = pc_checkskill(sd, SU_SOULATTACK)) > 0)
		base_status->rhw.range += skill_get_range2(&sd->bl, SU_SOULATTACK, skill, true);

// ----- FLEE CALCULATION -----

	// Absolute modifiers from passive skills
	if((skill=pc_checkskill(sd,TF_MISS))>0)
		base_status->flee += skill*(sd->class_&JOBL_2 && (sd->class_&MAPID_BASEMASK) == MAPID_THIEF? 4 : 3);
	if((skill=pc_checkskill(sd,MO_DODGE))>0)
		base_status->flee += (skill*3)>>1;
	if (pc_checkskill(sd, SU_POWEROFLIFE) > 0)
		base_status->flee += 20;

// ----- EQUIPMENT-DEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->def_rate < 0)
		sd->def_rate = 0;
	if(sd->def_rate != 100) {
		i = base_status->def * sd->def_rate/100;
		base_status->def = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

	if(pc_ismadogear(sd) && pc_checkskill(sd, NC_MAINFRAME) > 0)
		base_status->def += 20 + (pc_checkskill(sd, NC_MAINFRAME) * 20);

#ifndef RENEWAL
	if (!battle_config.weapon_defense_type && base_status->def > battle_config.max_def) {
		base_status->def2 += battle_config.over_def_bonus*(base_status->def -battle_config.max_def);
		base_status->def = (unsigned char)battle_config.max_def;
	}
#endif

// ----- EQUIPMENT-MDEF CALCULATION -----

	// Apply relative modifiers from equipment
	if(sd->mdef_rate < 0)
		sd->mdef_rate = 0;
	if(sd->mdef_rate != 100) {
		i =  base_status->mdef * sd->mdef_rate/100;
		base_status->mdef = cap_value(i, DEFTYPE_MIN, DEFTYPE_MAX);
	}

#ifndef RENEWAL
	if (!battle_config.magic_defense_type && base_status->mdef > battle_config.max_def) {
		base_status->mdef2 += battle_config.over_def_bonus*(base_status->mdef -battle_config.max_def);
		base_status->mdef = (signed char)battle_config.max_def;
	}
#endif

// ----- ASPD CALCULATION -----

	/// Unlike other stats, ASPD rate modifiers from skills/SCs/items/etc are first all added together, then the final modifier is applied

	// Basic ASPD value
	i = status_base_amotion_pc(sd,base_status);
	base_status->amotion = cap_value(i,pc_maxaspd(sd),2000);

	// Relative modifiers from passive skills
#ifndef RENEWAL_ASPD
	if((skill=pc_checkskill(sd,SA_ADVANCEDBOOK))>0 && sd->status.weapon == W_BOOK)
		base_status->aspd_rate -= 5*skill;
	if((skill = pc_checkskill(sd,SG_DEVIL)) > 0 && pc_is_maxjoblv(sd))
		base_status->aspd_rate -= 30*skill;
	if((skill=pc_checkskill(sd,GS_SINGLEACTION))>0 &&
		(sd->status.weapon >= W_REVOLVER && sd->status.weapon <= W_GRENADE))
		base_status->aspd_rate -= ((skill+1)/2) * 10;
	if(pc_isriding(sd))
		base_status->aspd_rate += 500-100*pc_checkskill(sd,KN_CAVALIERMASTERY);
	else if(pc_isridingdragon(sd))
		base_status->aspd_rate += 250-50*pc_checkskill(sd,RK_DRAGONTRAINING);
#else // Needs more info
	if((skill = pc_checkskill(sd,SG_DEVIL)) > 0 && pc_is_maxjoblv(sd))
		base_status->aspd_rate += 30*skill;
	if(pc_isriding(sd))
		base_status->aspd_rate -= 500-100*pc_checkskill(sd,KN_CAVALIERMASTERY);
	else if(pc_isridingdragon(sd))
		base_status->aspd_rate -= 250-50*pc_checkskill(sd,RK_DRAGONTRAINING);
#endif
	base_status->adelay = 2*base_status->amotion;


// ----- DMOTION -----

	i =  800-base_status->agi*4;
	base_status->dmotion = cap_value(i, 400, 800);
	if(battle_config.pc_damage_delay_rate != 100)
		base_status->dmotion = base_status->dmotion*battle_config.pc_damage_delay_rate/100;

// ----- MISC CALCULATIONS -----

	// Weight
	status_calc_weight(sd, CALCWT_MAXBONUS);
	status_calc_cart_weight(sd, CALCWT_MAXBONUS);

	if (pc_checkskill(sd, SM_MOVINGRECOVERY) > 0 || pc_ismadogear(sd))
		sd->regen.state.walk = 1;
	else
		sd->regen.state.walk = 0;

	// Skill SP cost
	if((skill=pc_checkskill(sd,HP_MANARECHARGE))>0 )
		sd->dsprate -= 4*skill;

	if(sc->data[SC_SERVICE4U])
		sd->dsprate -= sc->data[SC_SERVICE4U]->val3;

	if(sc->data[SC_SPCOST_RATE])
		sd->dsprate -= sc->data[SC_SPCOST_RATE]->val1;

	// Underflow protections.
	if(sd->dsprate < 0)
		sd->dsprate = 0;
	if(sd->castrate < 0)
		sd->castrate = 0;
	if(sd->delayrate < 0)
		sd->delayrate = 0;
	if(sd->hprecov_rate < 0)
		sd->hprecov_rate = 0;
	if(sd->sprecov_rate < 0)
		sd->sprecov_rate = 0;

	// Anti-element and anti-race
	if((skill=pc_checkskill(sd,CR_TRUST))>0)
		sd->subele[ELE_HOLY] += skill*5;
	if((skill=pc_checkskill(sd,BS_SKINTEMPER))>0) {
		sd->subele[ELE_NEUTRAL] += skill;
		sd->subele[ELE_FIRE] += skill*5;
	}
	if((skill=pc_checkskill(sd,SA_DRAGONOLOGY))>0) {
#ifdef RENEWAL
		skill = skill * 2;
#else
		skill = skill * 4;
#endif
		sd->right_weapon.addrace[RC_DRAGON]+=skill;
		sd->left_weapon.addrace[RC_DRAGON]+=skill;
		sd->magic_addrace[RC_DRAGON]+=skill;
		sd->subrace[RC_DRAGON]+=skill;
	}

	if(sc->count) {
		if(sc->data[SC_CONCENTRATE]) { // Update the card-bonus data
			sc->data[SC_CONCENTRATE]->val3 = sd->param_bonus[1]; // Agi
			sc->data[SC_CONCENTRATE]->val4 = sd->param_bonus[4]; // Dex
		}
		if(sc->data[SC_SIEGFRIED]) {
			i = sc->data[SC_SIEGFRIED]->val2;
			sd->subele[ELE_WATER] += i;
			sd->subele[ELE_EARTH] += i;
			sd->subele[ELE_FIRE] += i;
			sd->subele[ELE_WIND] += i;
			sd->subele[ELE_POISON] += i;
			sd->subele[ELE_HOLY] += i;
			sd->subele[ELE_DARK] += i;
			sd->subele[ELE_GHOST] += i;
			sd->subele[ELE_UNDEAD] += i;
		}
		if(sc->data[SC_PROVIDENCE]) {
			sd->subele[ELE_HOLY] += sc->data[SC_PROVIDENCE]->val2;
			sd->subrace[RC_DEMON] += sc->data[SC_PROVIDENCE]->val2;
		}
        if (sc->data[SC_GEFFEN_MAGIC1]) {
            sd->right_weapon.addrace[RC_PLAYER] += sc->data[SC_GEFFEN_MAGIC1]->val1;
            sd->right_weapon.addrace[RC_DEMIHUMAN] += sc->data[SC_GEFFEN_MAGIC1]->val1;
            sd->left_weapon.addrace[RC_PLAYER] += sc->data[SC_GEFFEN_MAGIC1]->val1;
            sd->left_weapon.addrace[RC_DEMIHUMAN] += sc->data[SC_GEFFEN_MAGIC1]->val1;
        }
        if (sc->data[SC_GEFFEN_MAGIC2]) {
            sd->magic_addrace[RC_PLAYER] += sc->data[SC_GEFFEN_MAGIC2]->val1;
            sd->magic_addrace[RC_DEMIHUMAN] += sc->data[SC_GEFFEN_MAGIC2]->val1;
        }
        if(sc->data[SC_GEFFEN_MAGIC3]) {
            sd->subrace[RC_PLAYER] += sc->data[SC_GEFFEN_MAGIC3]->val1;
            sd->subrace[RC_DEMIHUMAN] += sc->data[SC_GEFFEN_MAGIC3]->val1;
        }
		if(sc->data[SC_ARMOR_ELEMENT_WATER]) {	// This status change should grant card-type elemental resist.
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_ELEMENT_WATER]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_ELEMENT_WATER]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_ELEMENT_WATER]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_ELEMENT_WATER]->val4;
		}
		if(sc->data[SC_ARMOR_ELEMENT_EARTH]) {	// This status change should grant card-type elemental resist.
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_ELEMENT_EARTH]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_ELEMENT_EARTH]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_ELEMENT_EARTH]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_ELEMENT_EARTH]->val4;
		}
		if(sc->data[SC_ARMOR_ELEMENT_FIRE]) {	// This status change should grant card-type elemental resist.
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_ELEMENT_FIRE]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_ELEMENT_FIRE]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_ELEMENT_FIRE]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_ELEMENT_FIRE]->val4;
		}
		if(sc->data[SC_ARMOR_ELEMENT_WIND]) {	// This status change should grant card-type elemental resist.
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_ELEMENT_WIND]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_ELEMENT_WIND]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_ELEMENT_WIND]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_ELEMENT_WIND]->val4;
		}
		if(sc->data[SC_ARMOR_RESIST]) { // Undead Scroll
			sd->subele[ELE_WATER] += sc->data[SC_ARMOR_RESIST]->val1;
			sd->subele[ELE_EARTH] += sc->data[SC_ARMOR_RESIST]->val2;
			sd->subele[ELE_FIRE] += sc->data[SC_ARMOR_RESIST]->val3;
			sd->subele[ELE_WIND] += sc->data[SC_ARMOR_RESIST]->val4;
		}
		if( sc->data[SC_FIRE_CLOAK_OPTION] ) {
			i = sc->data[SC_FIRE_CLOAK_OPTION]->val2;
			sd->subele[ELE_FIRE] += i;
			sd->subele[ELE_WATER] -= i;
		}
		if( sc->data[SC_WATER_DROP_OPTION] ) {
			i = sc->data[SC_WATER_DROP_OPTION]->val2;
			sd->subele[ELE_WATER] += i;
			sd->subele[ELE_WIND] -= i;
		}
		if( sc->data[SC_WIND_CURTAIN_OPTION] ) {
			i = sc->data[SC_WIND_CURTAIN_OPTION]->val2;
			sd->subele[ELE_WIND] += i;
			sd->subele[ELE_EARTH] -= i;
		}
		if( sc->data[SC_STONE_SHIELD_OPTION] ) {
			i = sc->data[SC_STONE_SHIELD_OPTION]->val2;
			sd->subele[ELE_EARTH] += i;
			sd->subele[ELE_FIRE] -= i;
		}
		if (sc->data[SC_MTF_MLEATKED] )
			sd->subele[ELE_NEUTRAL] += sc->data[SC_MTF_MLEATKED]->val3;
		if (sc->data[SC_MTF_CRIDAMAGE])
			sd->bonus.crit_atk_rate += sc->data[SC_MTF_CRIDAMAGE]->val1;
		if (sc->data[SC_GLASTHEIM_ATK]) {
			sd->ignore_mdef_by_race[RC_UNDEAD] += sc->data[SC_GLASTHEIM_ATK]->val1;
			sd->ignore_mdef_by_race[RC_DEMON] += sc->data[SC_GLASTHEIM_ATK]->val1;
		}
		if (sc->data[SC_LAUDARAMUS])
			sd->bonus.crit_atk_rate += 5 * sc->data[SC_LAUDARAMUS]->val1;
	}
	status_cpy(&sd->battle_status, base_status);

// ----- CLIENT-SIDE REFRESH -----
	if(!sd->bl.prev) {
		// Will update on LoadEndAck
		calculating = 0;
		return 0;
	}
	if(memcmp(b_skill,sd->status.skill,sizeof(sd->status.skill)))
		clif_skillinfoblock(sd);

	// If the skill is learned, the status is infinite.
	if( (skill = pc_checkskill(sd,SU_SPRITEMABLE)) > 0 && !sd->sc.data[SC_SPRITEMABLE] )
		sc_start(&sd->bl, &sd->bl, SC_SPRITEMABLE, 100, 1, INFINITE_TICK);

	calculating = 0;

	return 0;
}

/// Intermediate function since C++ does not have a try-finally syntax
int status_calc_pc_( struct map_session_data* sd, enum e_status_calc_opt opt ){
	// Save the old script the player was attached to
	struct script_state* previous_st = sd->st;

	// Store the return value of the original function
	int ret = status_calc_pc_sub( sd, opt );

	// If an old script is present
	if( previous_st ){
		// Reattach the player to it, so that the limitations of that script kick back in
		script_attach_state( previous_st );
	}

	// Return the original return value
	return ret;
}

/**
 * Calculates Mercenary data
 * @param md: Mercenary object
 * @param opt: Whether it is first calc or not (0 on level up or status)
 * @return 0
 */
int status_calc_mercenary_(struct mercenary_data *md, enum e_status_calc_opt opt)
{
	struct status_data *status = &md->base_status;
	struct s_mercenary *merc = &md->mercenary;

	if (opt&SCO_FIRST) {
		memcpy(status, &md->db->status, sizeof(struct status_data));
		status->class_ = CLASS_NORMAL;
		status->mode = static_cast<e_mode>(MD_CANMOVE|MD_CANATTACK);
		status->hp = status->max_hp;
		status->sp = status->max_sp;
		md->battle_status.hp = merc->hp;
		md->battle_status.sp = merc->sp;
		if (md->master)
			status->speed = status_get_speed(&md->master->bl);
	}

	status_calc_misc(&md->bl, status, md->db->lv);
	status_cpy(&md->battle_status, status);

	return 0;
}

/**
 * Calculates Homunculus data
 * @param hd: Homunculus object
 * @param opt: Whether it is first calc or not (0 on level up or status)
 * @return 1
 */
int status_calc_homunculus_(struct homun_data *hd, enum e_status_calc_opt opt)
{
	struct status_data *status = &hd->base_status;
	struct s_homunculus *hom = &hd->homunculus;
	int skill_lv;
	int amotion;

	status->str = hom->str / 10;
	status->agi = hom->agi / 10;
	status->vit = hom->vit / 10;
	status->dex = hom->dex / 10;
	status->int_ = hom->int_ / 10;
	status->luk = hom->luk / 10;

	APPLY_HOMUN_LEVEL_STATWEIGHT();

	if (opt&SCO_FIRST) {
		const struct s_homunculus_db *db = hd->homunculusDB;
		status->def_ele = db->element;
		status->ele_lv = 1;
		status->race = db->race;
		status->class_ = CLASS_NORMAL;
		status->size = (hom->class_ == db->evo_class) ? db->evo_size : db->base_size;
		status->rhw.range = 1 + status->size;
		status->mode = static_cast<e_mode>(MD_CANMOVE|MD_CANATTACK);
		status->speed = DEFAULT_WALK_SPEED;
		if (battle_config.hom_setting&HOMSET_COPY_SPEED && hd->master)
			status->speed = status_get_speed(&hd->master->bl);

		status->hp = 1;
		status->sp = 1;
	}

	status->aspd_rate = 1000;

#ifdef RENEWAL
	amotion = hd->homunculusDB->baseASPD;
	amotion = amotion - amotion * (status->dex + hom->dex_value) / 1000 - (status->agi + hom->agi_value) * amotion / 250;
	status->def = status->mdef = 0;
#else
	skill_lv = hom->level / 10 + status->vit / 5;
	status->def = cap_value(skill_lv, 0, 99);

	skill_lv = hom->level / 10 + status->int_ / 5;
	status->mdef = cap_value(skill_lv, 0, 99);

	amotion = (1000 - 4 * status->agi - status->dex) * hd->homunculusDB->baseASPD / 1000;
#endif

	status->amotion = cap_value(amotion, battle_config.max_aspd, 2000);
	status->adelay = status->amotion; //It seems adelay = amotion for Homunculus.

	status->max_hp = hom->max_hp;
	status->max_sp = hom->max_sp;

	hom_calc_skilltree(hd, 0);

	if((skill_lv = hom_checkskill(hd, HAMI_SKIN)) > 0)
		status->def += skill_lv * 4;

	if((skill_lv = hom_checkskill(hd, HVAN_INSTRUCT)) > 0) {
		status->int_ += 1 + skill_lv / 2 + skill_lv / 4 + skill_lv / 5;
		status->str += 1 + skill_lv / 3 + skill_lv / 3 + skill_lv / 4;
	}

	if((skill_lv = hom_checkskill(hd, HAMI_SKIN)) > 0)
		status->max_hp += skill_lv * 2 * status->max_hp / 100;

	if((skill_lv = hom_checkskill(hd, HLIF_BRAIN)) > 0)
		status->max_sp += (1 + skill_lv / 2 - skill_lv / 4 + skill_lv / 5) * status->max_sp / 100;

	if (opt&SCO_FIRST) {
		hd->battle_status.hp = hom->hp;
		hd->battle_status.sp = hom->sp;
		if(hom->class_ == 6052) // Eleanor
			sc_start(&hd->bl,&hd->bl, SC_STYLE_CHANGE, 100, MH_MD_FIGHTING, INFINITE_TICK);
	}

#ifndef RENEWAL
	status->rhw.atk = status->dex;
	status->rhw.atk2 = status->str + hom->level;
#endif

	status_calc_misc(&hd->bl, status, hom->level);

	status_cpy(&hd->battle_status, status);
	return 1;
}

/**
 * Calculates Elemental data
 * @param ed: Elemental object
 * @param opt: Whether it is first calc or not (0 on status change)
 * @return 0
 */
int status_calc_elemental_(struct elemental_data *ed, enum e_status_calc_opt opt)
{
	struct status_data *status = &ed->base_status;
	struct s_elemental *ele = &ed->elemental;
	struct map_session_data *sd = ed->master;

	if( !sd )
		return 0;

	if (opt&SCO_FIRST) {
		memcpy(status, &ed->db->status, sizeof(struct status_data));
		if( !ele->mode )
			status->mode = EL_MODE_PASSIVE;
		else
			status->mode = ele->mode;

		status->class_ = CLASS_NORMAL;
		status_calc_misc(&ed->bl, status, 0);

		status->max_hp = ele->max_hp;
		status->max_sp = ele->max_sp;
		status->hp = ele->hp;
		status->sp = ele->sp;
		status->rhw.atk = ele->atk;
		status->rhw.atk2 = ele->atk2;
		status->matk_min += ele->matk;
		status->def += ele->def;
		status->mdef += ele->mdef;
		status->flee = ele->flee;
		status->hit = ele->hit;

		if (ed->master)
			status->speed = status_get_speed(&ed->master->bl);

		memcpy(&ed->battle_status,status,sizeof(struct status_data));
	} else {
		status_calc_misc(&ed->bl, status, 0);
		status_cpy(&ed->battle_status, status);
	}

	return 0;
}

/**
 * Calculates NPC data
 * @param nd: NPC object
 * @param opt: Whether it is first calc or not (what?)
 * @return 0
 */
int status_calc_npc_(struct npc_data *nd, enum e_status_calc_opt opt)
{
	struct status_data *status = &nd->status;

	if (!nd)
		return 0;

	if (opt&SCO_FIRST) {
		status->hp = 1;
		status->sp = 1;
		status->max_hp = 1;
		status->max_sp = 1;

		status->def_ele = ELE_NEUTRAL;
		status->ele_lv = 1;
		status->race = RC_DEMIHUMAN;
		status->class_ = CLASS_NORMAL;
		status->size = nd->size;
		status->rhw.range = 1 + status->size;
		status->mode = static_cast<e_mode>(MD_CANMOVE|MD_CANATTACK);
		status->speed = nd->speed;
	}

	status->str = nd->stat_point + nd->params.str;
	status->agi = nd->stat_point + nd->params.agi;
	status->vit = nd->stat_point + nd->params.vit;
	status->int_= nd->stat_point + nd->params.int_;
	status->dex = nd->stat_point + nd->params.dex;
	status->luk = nd->stat_point + nd->params.luk;

	status_calc_misc(&nd->bl, status, nd->level);
	status_cpy(&nd->status, status);

	return 0;
}

/**
 * Calculates regeneration values
 * Applies passive skill regeneration additions
 * @param bl: Object to calculate regen for [PC|HOM|MER|ELEM]
 * @param status: Object's status
 * @param regen: Object's base regeneration data
 */
void status_calc_regen(struct block_list *bl, struct status_data *status, struct regen_data *regen)
{
	struct map_session_data *sd;
	struct status_change *sc;
	int val, skill, reg_flag;

	if( !(bl->type&BL_REGEN) || !regen )
		return;

	sd = BL_CAST(BL_PC,bl);
	sc = status_get_sc(bl);

	val = 1 + (status->vit/5) + (status->max_hp/200);

	if( sd && sd->hprecov_rate != 100 )
		val = val*sd->hprecov_rate/100;

	reg_flag = bl->type == BL_PC ? 0 : 1;

	regen->hp = cap_value(val, reg_flag, SHRT_MAX);

	val = 1 + (status->int_/6) + (status->max_sp/100);
	if( status->int_ >= 120 )
		val += ((status->int_-120)>>1) + 4;

	if( sd && sd->sprecov_rate != 100 )
		val = val*sd->sprecov_rate/100;

	regen->sp = cap_value(val, reg_flag, SHRT_MAX);

	if( sd ) {
		struct regen_data_sub *sregen;
		if( (skill=pc_checkskill(sd,HP_MEDITATIO)) > 0 ) {
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
		// Only players have skill/sitting skill regen for now.
		sregen = regen->sregen;

		val = 0;
		if( (skill=pc_checkskill(sd,SM_RECOVERY)) > 0 )
			val += skill*5 + skill*status->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=pc_checkskill(sd,MG_SRECOVERY)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		if( (skill=pc_checkskill(sd,NJ_NINPOU)) > 0 )
			val += skill*3 + skill*status->max_sp/500;
		if( (skill=pc_checkskill(sd,WM_LESSON)) > 0 )
			val += 3 + 3 * skill;

		if (sc && sc->count) {
			if (sc->data[SC_SHRIMPBLESSING])
				val *= 150 / 100;
			if (sc->data[SC_ANCILLA])
				val += sc->data[SC_ANCILLA]->val2 / 100;
		}

		sregen->sp = cap_value(val, 0, SHRT_MAX);

		// Skill-related recovery (only when sit)
		sregen = regen->ssregen;

		val = 0;
		if( (skill=pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*4 + skill*status->max_hp/500;

		if( (skill=pc_checkskill(sd,TK_HPTIME)) > 0 && sd->state.rest )
			val += skill*30 + skill*status->max_hp/500;
		sregen->hp = cap_value(val, 0, SHRT_MAX);

		val = 0;
		if( (skill=pc_checkskill(sd,TK_SPTIME)) > 0 && sd->state.rest ) {
			val += skill*3 + skill*status->max_sp/500;
			if ((skill=pc_checkskill(sd,SL_KAINA)) > 0) // Power up Enjoyable Rest
				val += (30+10*skill)*val/100;
		}
		if( (skill=pc_checkskill(sd,MO_SPIRITSRECOVERY)) > 0 )
			val += skill*2 + skill*status->max_sp/500;
		sregen->sp = cap_value(val, 0, SHRT_MAX);
	}

	if( bl->type == BL_HOM ) {
		struct homun_data *hd = (TBL_HOM*)bl;
		if( (skill = hom_checkskill(hd,HAMI_SKIN)) > 0 ) {
			val = regen->hp*(100+5*skill)/100;
			regen->hp = cap_value(val, 1, SHRT_MAX);
		}
		if( (skill = hom_checkskill(hd,HLIF_BRAIN)) > 0 ) {
			val = regen->sp*(100+3*skill)/100;
			regen->sp = cap_value(val, 1, SHRT_MAX);
		}
	} else if( bl->type == BL_MER ) {
		val = (status->max_hp * status->vit / 10000 + 1) * 6;
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = (status->max_sp * (status->int_ + 10) / 750) + 1;
		regen->sp = cap_value(val, 1, SHRT_MAX);
	} else if( bl->type == BL_ELEM ) {
		val = (status->max_hp * status->vit / 10000 + 1) * 6;
		regen->hp = cap_value(val, 1, SHRT_MAX);

		val = (status->max_sp * (status->int_ + 10) / 750) + 1;
		regen->sp = cap_value(val, 1, SHRT_MAX);
	}
}

/**
 * Calculates SC (Status Changes) regeneration values
 * @param bl: Object to calculate regen for [PC|HOM|MER|ELEM]
 * @param regen: Object's base regeneration data
 * @param sc: Object's status change data
 */
void status_calc_regen_rate(struct block_list *bl, struct regen_data *regen, struct status_change *sc)
{
	if (!(bl->type&BL_REGEN) || !regen)
		return;

	regen->flag = RGN_HP|RGN_SP;
	if(regen->sregen) {
		if (regen->sregen->hp)
			regen->flag |= RGN_SHP;

		if (regen->sregen->sp)
			regen->flag |= RGN_SSP;
		regen->sregen->rate.hp = regen->sregen->rate.sp = 100;
	}
	if (regen->ssregen) {
		if (regen->ssregen->hp)
			regen->flag |= RGN_SHP;

		if (regen->ssregen->sp)
			regen->flag |= RGN_SSP;
		regen->ssregen->rate.hp = regen->ssregen->rate.sp = 100;
	}
	regen->rate.hp = regen->rate.sp = 100;

	if (!sc || !sc->count)
		return;

	// No HP or SP regen
	if ((sc->data[SC_POISON] && !sc->data[SC_SLOWPOISON])
		|| (sc->data[SC_DPOISON] && !sc->data[SC_SLOWPOISON])
		|| sc->data[SC_BERSERK]
		|| sc->data[SC_TRICKDEAD]
		|| sc->data[SC_BLEEDING]
		|| sc->data[SC_MAGICMUSHROOM]
		|| sc->data[SC_SATURDAYNIGHTFEVER]
		|| sc->data[SC_REBOUND])
		regen->flag = RGN_NONE;

	// No natural SP regen
	if (sc->data[SC_DANCING] ||
#ifdef RENEWAL
		sc->data[SC_MAXIMIZEPOWER] ||
#endif
#ifndef RENEWAL
		(bl->type == BL_PC && (((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK &&
		(sc->data[SC_EXTREMITYFIST] || sc->data[SC_EXPLOSIONSPIRITS]) && (!sc->data[SC_SPIRIT] || sc->data[SC_SPIRIT]->val2 != SL_MONK)) ||
#else
		(bl->type == BL_PC && (((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK &&
		sc->data[SC_EXTREMITYFIST] && (!sc->data[SC_SPIRIT] || sc->data[SC_SPIRIT]->val2 != SL_MONK)) ||
#endif
		sc->data[SC_OBLIVIONCURSE] || sc->data[SC_VITALITYACTIVATION])
		regen->flag &= ~RGN_SP;

	if (sc->data[SC_TENSIONRELAX]) {
		if (sc->data[SC_WEIGHT50] || sc->data[SC_WEIGHT90])
			regen->state.overweight = 0; // 1x HP regen
		else {
			regen->rate.hp += 200;
			if (regen->sregen)
				regen->sregen->rate.hp += 200;
		}
	}

	if (sc->data[SC_MAGNIFICAT])
		regen->rate.sp += 100;

	if (sc->data[SC_REGENERATION]) {
		const struct status_change_entry *sce = sc->data[SC_REGENERATION];
		if (!sce->val4) {
			regen->rate.hp += (sce->val2*100);
			regen->rate.sp += (sce->val3*100);
		} else
			regen->flag &= ~sce->val4; // Remove regen as specified by val4
	}
	if (sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 1)
		regen->hp += cap_value(regen->hp * 50 / 100, 1, SHRT_MAX);
	if(sc->data[SC_GT_REVITALIZE]) {
		regen->hp += cap_value(regen->hp * sc->data[SC_GT_REVITALIZE]->val3/100, 1, SHRT_MAX);
		regen->state.walk = 1;
	}
	if (bl->type == BL_ELEM) { // Recovery bonus only applies to the Elementals.
		int ele_class = status_get_class(bl);

		switch (ele_class) {
		case ELEMENTALID_AGNI_S:
		case ELEMENTALID_AGNI_M:
		case ELEMENTALID_AGNI_L:
			if (sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 1)
				regen->rate.hp += 100;
			break;
		case ELEMENTALID_AQUA_S:
		case ELEMENTALID_AQUA_M:
		case ELEMENTALID_AQUA_L:
			if (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 1)
				regen->rate.hp += 100;
			break;
		case ELEMENTALID_VENTUS_S:
		case ELEMENTALID_VENTUS_M:
		case ELEMENTALID_VENTUS_L:
			if (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 1)
				regen->rate.hp += 100;
			break;
		case ELEMENTALID_TERA_S:
		case ELEMENTALID_TERA_M:
		case ELEMENTALID_TERA_L:
			if (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 1)
				regen->rate.hp += 100;
			break;
		}
	}
	if (sc->data[SC_EXTRACT_WHITE_POTION_Z])
		regen->rate.hp += (unsigned short)(regen->rate.hp * sc->data[SC_EXTRACT_WHITE_POTION_Z]->val1 / 100.);
	if (sc->data[SC_VITATA_500])
		regen->rate.sp += (unsigned short)(regen->rate.sp * sc->data[SC_VITATA_500]->val1 / 100.);
	if (sc->data[SC_CATNIPPOWDER]) {
		regen->rate.hp *= 2;
		regen->rate.sp *= 2;
	}
}

/**
 * Applies a state to a unit - See [StatusChangeStateTable]
 * @param bl: Object to change state on [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change data
 * @param flag: Which state to apply to bl
 * @param start: (1) start state, (0) remove state
 */
void status_calc_state( struct block_list *bl, struct status_change *sc, enum e_scs_flag flag, bool start )
{

	/// No sc at all, we can zero without any extra weight over our conciousness
	if( !sc->count ) {
		memset(&sc->cant, 0, sizeof (sc->cant));
		return;
	}

	// Can't move
	if( flag&SCS_NOMOVE ) {
		if( !(flag&SCS_NOMOVECOND) )
			sc->cant.move = ( start ? 1 : 0 );
		else if(
				     (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF)	// cannot move while gospel is in effect
				  || (sc->data[SC_BASILICA] && sc->data[SC_BASILICA]->val4 == bl->id) // Basilica caster cannot move
				  || (sc->data[SC_GRAVITATION] && sc->data[SC_GRAVITATION]->val3 == BCT_SELF)
				  || (sc->data[SC_CAMOUFLAGE] && sc->data[SC_CAMOUFLAGE]->val1 < 3)
				  || (sc->data[SC_MAGNETICFIELD] && sc->data[SC_MAGNETICFIELD]->val2 != bl->id)
				  || (sc->data[SC_FEAR] && sc->data[SC_FEAR]->val2 > 0)
				  || (sc->data[SC_SPIDERWEB] && sc->data[SC_SPIDERWEB]->val1)
				  || (sc->data[SC_HIDING] && (bl->type != BL_PC || (pc_checkskill(BL_CAST(BL_PC,bl),RG_TUNNELDRIVE) <= 0)))
				  || (sc->data[SC_DANCING] && sc->data[SC_DANCING]->val4 && (
						!sc->data[SC_LONGING] ||
						(sc->data[SC_DANCING]->val1&0xFFFF) == CG_MOONLIT ||
						(sc->data[SC_DANCING]->val1&0xFFFF) == CG_HERMODE
						))
				  || (sc->data[SC_CRYSTALIZE] && bl->type != BL_MOB)
				  || (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
 				 )
				 sc->cant.move = ( start ? 1 : 0 );
	}

	// Can't use skills
	if( flag&SCS_NOCAST ) {
		if( !(flag&SCS_NOCASTCOND) )
			sc->cant.cast = ( start ? 1 : 0 );
		else if( (sc->data[SC_CRYSTALIZE] && bl->type != BL_MOB) )
			sc->cant.cast = ( start ? 1 : 0 );
	}

	// Can't chat
	if( flag&SCS_NOCHAT ) {
		if( !(flag&SCS_NOCHATCOND) )
			sc->cant.chat = ( start ? 1 : 0 );
		else if(sc->data[SC_NOCHAT] && sc->data[SC_NOCHAT]->val1&MANNER_NOCHAT)
			sc->cant.chat = ( start ? 1 : 0 );
	}

	// Can't attack
	if( flag&SCS_NOATTACK ) {
		if( !(flag&SCS_NOATTACKCOND) )
			sc->cant.attack = ( start ? 1 : 0 );
		/*else if( )
			sc->cant.attack = ( start ? 1 : 0 );*/
	}

	// Player-only states
	if( bl->type == BL_PC ) {
		// Can't pick-up items
		if( flag&SCS_NOPICKITEM ) {
			if( !(flag&SCS_NOPICKITEMCOND) )
				sc->cant.pickup = ( start ? 1 : 0 );
			else if( (sc->data[SC_NOCHAT] && sc->data[SC_NOCHAT]->val1&MANNER_NOITEM) )
				sc->cant.pickup = ( start ? 1 : 0 );
		}

		// Can't drop items
		if( flag&SCS_NODROPITEM ) {
			if( !(flag&SCS_NODROPITEMCOND) )
				sc->cant.drop = ( start ? 1 : 0 );
			else if( (sc->data[SC_NOCHAT] && sc->data[SC_NOCHAT]->val1&MANNER_NOITEM) )
				sc->cant.drop = ( start ? 1 : 0 );
		}

		// Can't equip item
		if( flag&SCS_NOEQUIPITEM ) {
			if( !(flag&SCS_NOEQUIPITEMCOND) )
				sc->cant.equip = ( start ? 1 : 0 );
			/*else if(  )
				sc->cant.equip = ( start ? 1 : 0 );*/
		}

		// Can't unequip item
		if( flag&SCS_NOUNEQUIPITEM) {
			if( !(flag&SCS_NOUNEQUIPITEMCOND) )
				sc->cant.unequip = ( start ? 1 : 0 );
			/*else if(  )
				sc->cant.unequip = ( start ? 1 : 0 );*/
		}

		// Can't consume item
		if( flag&SCS_NOCONSUMEITEM) {
			if( !(flag&SCS_NOCONSUMEITEMCOND) )
				sc->cant.consume = ( start ? 1 : 0 );
			else if( (sc->data[SC_GRAVITATION] && sc->data[SC_GRAVITATION]->val3 == BCT_SELF) ||
				 (sc->data[SC_NOCHAT] && sc->data[SC_NOCHAT]->val1&MANNER_NOITEM) )
				sc->cant.consume = ( start ? 1 : 0 );
		}
	}

	return;
}

/**
 * Recalculates parts of an objects status according to specified flags
 * See [set_sc] [add_sc]
 * @param bl: Object whose status has changed [PC|MOB|HOM|MER|ELEM]
 * @param flag: Which status has changed on bl
 */
void status_calc_bl_main(struct block_list *bl, /*enum scb_flag*/int flag)
{
	const struct status_data *b_status = status_get_base_status(bl); // Base Status
	struct status_data *status = status_get_status_data(bl); // Battle Status
	struct status_change *sc = status_get_sc(bl);
	TBL_PC *sd = BL_CAST(BL_PC,bl);
	int temp;

	if (!b_status || !status)
		return;

	/** [Playtester]
	* This needs to be done even if there is currently no status change active, because
	* we need to update the speed on the client when the last status change ends.
	**/
	if(flag&SCB_SPEED) {
		struct unit_data *ud = unit_bl2ud(bl);
		/** [Skotlex]
		* Re-walk to adjust speed (we do not check if walktimer != INVALID_TIMER
		* because if you step on something while walking, the moment this
		* piece of code triggers the walk-timer is set on INVALID_TIMER)
		**/
		if (ud)
			ud->state.change_walk_target = ud->state.speed_changed = 1;
	}

	if((!(bl->type&BL_REGEN)) && (!sc || !sc->count)) { // No difference.
		status_cpy(status, b_status);
		return;
	}

	if(flag&SCB_STR) {
		status->str = status_calc_str(bl, sc, b_status->str);
		flag|=SCB_BATK;
		if( bl->type&BL_HOM )
			flag |= SCB_WATK;
	}

	if(flag&SCB_AGI) {
		status->agi = status_calc_agi(bl, sc, b_status->agi);
		flag|=SCB_FLEE
#ifdef RENEWAL
			|SCB_DEF2
#endif
			;
		if( bl->type&(BL_PC|BL_HOM) )
			flag |= SCB_ASPD|SCB_DSPD;
	}

	if(flag&SCB_VIT) {
		status->vit = status_calc_vit(bl, sc, b_status->vit);
		flag|=SCB_DEF2|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag |= SCB_MAXHP;
		if( bl->type&BL_HOM )
			flag |= SCB_DEF;
	}

	if(flag&SCB_INT) {
		status->int_ = status_calc_int(bl, sc, b_status->int_);
		flag|=SCB_MATK|SCB_MDEF2;
		if( bl->type&(BL_PC|BL_HOM|BL_MER|BL_ELEM) )
			flag |= SCB_MAXSP;
		if( bl->type&BL_HOM )
			flag |= SCB_MDEF;
	}

	if(flag&SCB_DEX) {
		status->dex = status_calc_dex(bl, sc, b_status->dex);
		flag|=SCB_BATK|SCB_HIT
#ifdef RENEWAL
			|SCB_MATK|SCB_MDEF2
#endif
			;
		if( bl->type&(BL_PC|BL_HOM) )
			flag |= SCB_ASPD;
		if( bl->type&BL_HOM )
			flag |= SCB_WATK;
	}

	if(flag&SCB_LUK) {
		status->luk = status_calc_luk(bl, sc, b_status->luk);
		flag|=SCB_BATK|SCB_CRI|SCB_FLEE2
#ifdef RENEWAL
			|SCB_MATK|SCB_HIT|SCB_FLEE
#endif
			;
	}

	if(flag&SCB_BATK && b_status->batk) {
		int lv = status_get_lv(bl);
		status->batk = status_base_atk(bl, status, lv);
		temp = b_status->batk - status_base_atk(bl, b_status, lv);
		if (temp) {
			temp += status->batk;
			status->batk = cap_value(temp, 0, USHRT_MAX);
		}
		status->batk = status_calc_batk(bl, sc, status->batk);
	}

	if(flag&SCB_WATK) {
#ifndef RENEWAL
		status->rhw.atk = status_calc_watk(bl, sc, b_status->rhw.atk);
		if (!sd) // Should not affect weapon refine bonus
			status->rhw.atk2 = status_calc_watk(bl, sc, b_status->rhw.atk2);

		if (sd && sd->bonus.weapon_atk_rate)
			status->rhw.atk += status->rhw.atk * sd->bonus.weapon_atk_rate / 100;
		if(b_status->lhw.atk) {
			if (sd) {
				sd->state.lr_flag = 1;
				status->lhw.atk = status_calc_watk(bl, sc, b_status->lhw.atk);
				sd->state.lr_flag = 0;
			} else {
				status->lhw.atk = status_calc_watk(bl, sc, b_status->lhw.atk);
				status->lhw.atk2= status_calc_watk(bl, sc, b_status->lhw.atk2);
			}
		}
#else
		if(!b_status->watk) { // We only have left-hand weapon
			status->watk = 0;
			status->watk2 = status_calc_watk(bl, sc, b_status->watk2);
		}
		else status->watk = status_calc_watk(bl, sc, b_status->watk);
#endif
	}

	if(flag&SCB_HIT) {
		if (status->dex == b_status->dex
#ifdef RENEWAL
			&& status->luk == b_status->luk
#endif
			)
			status->hit = status_calc_hit(bl, sc, b_status->hit);
		else
			status->hit = status_calc_hit(bl, sc, b_status->hit + (status->dex - b_status->dex)
#ifdef RENEWAL
			 + (status->luk/3 - b_status->luk/3)
#endif
			 );
	}

	if(flag&SCB_FLEE) {
		if (status->agi == b_status->agi
#ifdef RENEWAL
			&& status->luk == b_status->luk
#endif
			)
			status->flee = status_calc_flee(bl, sc, b_status->flee);
		else
			status->flee = status_calc_flee(bl, sc, b_status->flee +(status->agi - b_status->agi)
#ifdef RENEWAL
			+ (status->luk/5 - b_status->luk/5)
#endif
			);
	}

	if(flag&SCB_DEF) {
		status->def = status_calc_def(bl, sc, b_status->def);

		if( bl->type&BL_HOM )
			status->def += (status->vit/5 - b_status->vit/5);
	}

	if(flag&SCB_DEF2) {
		if (status->vit == b_status->vit
#ifdef RENEWAL
			&& status->agi == b_status->agi
#endif
			)
			status->def2 = status_calc_def2(bl, sc, b_status->def2);
		else
			status->def2 = status_calc_def2(bl, sc, b_status->def2
#ifdef RENEWAL
			+ (int)( ((float)status->vit/2 - (float)b_status->vit/2) + ((float)status->agi/5 - (float)b_status->agi/5) )
#else
			+ (status->vit - b_status->vit)
#endif
		);
	}

	if(flag&SCB_MDEF) {
		status->mdef = status_calc_mdef(bl, sc, b_status->mdef);

		if( bl->type&BL_HOM )
			status->mdef += (status->int_/5 - b_status->int_/5);
	}

	if(flag&SCB_MDEF2) {
		if (status->int_ == b_status->int_ && status->vit == b_status->vit
#ifdef RENEWAL
			&& status->dex == b_status->dex
#endif
			)
			status->mdef2 = status_calc_mdef2(bl, sc, b_status->mdef2);
		else
			status->mdef2 = status_calc_mdef2(bl, sc, b_status->mdef2 +(status->int_ - b_status->int_)
#ifdef RENEWAL
			+ (int)( ((float)status->dex/5 - (float)b_status->dex/5) + ((float)status->vit/5 - (float)b_status->vit/5) )
#else
			+ ((status->vit - b_status->vit)>>1)
#endif
			);
	}

	if(flag&SCB_SPEED) {
		status->speed = status_calc_speed(bl, sc, b_status->speed);

		if( bl->type&BL_PC && !(sd && sd->state.permanent_speed) && status->speed < battle_config.max_walk_speed )
			status->speed = battle_config.max_walk_speed;

		if( bl->type&BL_PET && ((TBL_PET*)bl)->master)
			status->speed = status_get_speed(&((TBL_PET*)bl)->master->bl);
		if( bl->type&BL_HOM && battle_config.hom_setting&HOMSET_COPY_SPEED && ((TBL_HOM*)bl)->master)
			status->speed = status_get_speed(&((TBL_HOM*)bl)->master->bl);
		if( bl->type&BL_MER && ((TBL_MER*)bl)->master)
			status->speed = status_get_speed(&((TBL_MER*)bl)->master->bl);
		if( bl->type&BL_ELEM && ((TBL_ELEM*)bl)->master)
			status->speed = status_get_speed(&((TBL_ELEM*)bl)->master->bl);
	}

	if(flag&SCB_CRI && b_status->cri) {
		if (status->luk == b_status->luk)
			status->cri = status_calc_critical(bl, sc, b_status->cri);
		else
			status->cri = status_calc_critical(bl, sc, b_status->cri + 3*(status->luk - b_status->luk));

		/// After status_calc_critical so the bonus is applied despite if you have or not a sc bugreport:5240
		if( bl->type == BL_PC && ((TBL_PC*)bl)->status.weapon == W_KATAR )
			status->cri <<= 1;
	}

	if(flag&SCB_FLEE2 && b_status->flee2) {
		if (status->luk == b_status->luk)
			status->flee2 = status_calc_flee2(bl, sc, b_status->flee2);
		else
			status->flee2 = status_calc_flee2(bl, sc, b_status->flee2 +(status->luk - b_status->luk));
	}

	if(flag&SCB_ATK_ELE) {
		status->rhw.ele = status_calc_attack_element(bl, sc, b_status->rhw.ele);
		if (sd) sd->state.lr_flag = 1;
		status->lhw.ele = status_calc_attack_element(bl, sc, b_status->lhw.ele);
		if (sd) sd->state.lr_flag = 0;
	}

	if(flag&SCB_DEF_ELE) {
		status->def_ele = status_calc_element(bl, sc, b_status->def_ele);
		status->ele_lv = status_calc_element_lv(bl, sc, b_status->ele_lv);
	}

	if(flag&SCB_MODE) {
		status->mode = status_calc_mode(bl, sc, b_status->mode);
		// Since mode changed, reset their state.
		if (!status_has_mode(status,MD_CANATTACK))
			unit_stop_attack(bl);
		if (!status_has_mode(status,MD_CANMOVE))
			unit_stop_walking(bl,1);
	}

	/**
	* No status changes alter these yet.
	* if(flag&SCB_SIZE)
	* if(flag&SCB_RACE)
	* if(flag&SCB_RANGE)
	**/

	if(flag&SCB_MAXHP) {
		if( bl->type&BL_PC ) {
			status->max_hp = status_calc_maxhpsp_pc(sd,status->vit,true);

			if(battle_config.hp_rate != 100)
				status->max_hp = (unsigned int)(battle_config.hp_rate * (status->max_hp/100.));

			if (sd->status.base_level < 100)
				status->max_hp = umin(status->max_hp,(unsigned int)battle_config.max_hp_lv99);
			else if (sd->status.base_level < 151)
				status->max_hp = umin(status->max_hp,(unsigned int)battle_config.max_hp_lv150);
			else
				status->max_hp = umin(status->max_hp,(unsigned int)battle_config.max_hp);
		}
		else
			status->max_hp = status_calc_maxhp(bl, b_status->max_hp);

		if( status->hp > status->max_hp ) { // !FIXME: Should perhaps a status_zap should be issued?
			status->hp = status->max_hp;
			if( sd ) clif_updatestatus(sd,SP_HP);
		}
	}

	if(flag&SCB_MAXSP) {
		if( bl->type&BL_PC ) {
			status->max_sp = status_calc_maxhpsp_pc(sd,status->int_,false);

			if(battle_config.sp_rate != 100)
				status->max_sp = (unsigned int)(battle_config.sp_rate * (status->max_sp/100.));

			status->max_sp = umin(status->max_sp,(unsigned int)battle_config.max_sp);
		}
		else
			status->max_sp = status_calc_maxsp(bl, b_status->max_sp);

		if( status->sp > status->max_sp ) {
			status->sp = status->max_sp;
			if( sd ) clif_updatestatus(sd,SP_SP);
		}
	}

	if(flag&SCB_MATK) {
#ifndef RENEWAL
		status->matk_min = status_base_matk_min(status) + (sd?sd->bonus.ematk:0);
		status->matk_max = status_base_matk_max(status) + (sd?sd->bonus.ematk:0);
#else
		/**
		 * RE MATK Formula (from irowiki:http:// irowiki.org/wiki/MATK)
		 * MATK = (sMATK + wMATK + eMATK) * Multiplicative Modifiers
		 **/
		int lv = status_get_lv(bl);
		status->matk_min = status_base_matk_min(bl, status, lv);
		status->matk_max = status_base_matk_max(bl, status, lv);

		switch( bl->type ) {
			case BL_PC: {
				int wMatk = 0;
				int variance = 0;

				// Any +MATK you get from skills and cards, including cards in weapon, is added here.
				if (sd) {
					if (sd->bonus.ematk > 0)
						status->matk_min += sd->bonus.ematk;
					if (pc_checkskill(sd, SU_POWEROFLAND) > 0) {
						if ((pc_checkskill(sd, SU_SV_STEMSPEAR) + pc_checkskill(sd, SU_CN_POWDERING) + pc_checkskill(sd, SU_CN_METEOR) + pc_checkskill(sd, SU_SV_ROOTTWIST) +
						pc_checkskill(sd, SU_CHATTERING) + pc_checkskill(sd, SU_MEOWMEOW) + pc_checkskill(sd, SU_NYANGGRASS)) > 19)
							status->matk_min += status->matk_min * 20 / 100;
					}
				}

				status->matk_min = status_calc_ematk(bl, sc, status->matk_min);
				status->matk_max = status->matk_min;

				// This is the only portion in MATK that varies depending on the weapon level and refinement rate.
				if (b_status->lhw.matk) {
					if (sd) {
						//sd->state.lr_flag = 1; //?? why was that set here
						status->lhw.matk = b_status->lhw.matk;
						sd->state.lr_flag = 0;
					} else {
						status->lhw.matk = b_status->lhw.matk;
					}
				}

				if (b_status->rhw.matk) {
					status->rhw.matk = b_status->rhw.matk;
				}

				if (status->rhw.matk) {
					wMatk += status->rhw.matk;
					variance += wMatk * status->rhw.wlv / 10;
				}

				if (status->lhw.matk) {
					wMatk += status->lhw.matk;
					variance += status->lhw.matk * status->lhw.wlv / 10;
				}

				status->matk_min += wMatk - variance;
				status->matk_max += wMatk + variance;
				}
				break;
		}
#endif

		if (bl->type&BL_PC && sd->matk_rate != 100) {
			status->matk_max = status->matk_max * sd->matk_rate/100;
			status->matk_min = status->matk_min * sd->matk_rate/100;
		}

		if ((bl->type&BL_HOM && battle_config.hom_setting&HOMSET_SAME_MATK)  /// Hom Min Matk is always the same as Max Matk
				|| (sc && sc->data[SC_RECOGNIZEDSPELL]))
			status->matk_min = status->matk_max;

#ifdef RENEWAL
		if( sd && sd->right_weapon.overrefine > 0) {
			status->matk_min++;
			status->matk_max += sd->right_weapon.overrefine - 1;
		}
#endif

		status->matk_max = status_calc_matk(bl, sc, status->matk_max);
		status->matk_min = status_calc_matk(bl, sc, status->matk_min);
	}

	if(flag&SCB_ASPD) {
		int amotion;

		if ( bl->type&BL_HOM ) {
#ifdef RENEWAL_ASPD
			amotion = ((TBL_HOM*)bl)->homunculusDB->baseASPD;
			amotion = amotion - amotion * status_get_homdex(bl) / 1000 - status_get_homagi(bl) * amotion / 250;
			amotion = (amotion * status_calc_aspd(bl, sc, true) + status_calc_aspd(bl, sc, false)) / - 100 + amotion;
#else
			amotion = (1000 - 4 * status->agi - status->dex) * ((TBL_HOM*)bl)->homunculusDB->baseASPD / 1000;

			amotion = status_calc_aspd_rate(bl, sc, amotion);
			amotion = amotion * status->aspd_rate / 1000;
#endif

			amotion = status_calc_fix_aspd(bl, sc, amotion);
			status->amotion = cap_value(amotion, battle_config.max_aspd, 2000);

			status->adelay = status->amotion;
		} else if ( bl->type&BL_PC ) {
			uint16 skill_lv;

			amotion = status_base_amotion_pc(sd,status);
#ifndef RENEWAL_ASPD
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);
#endif
			// Absolute ASPD % modifier
			amotion = amotion * status->aspd_rate / 1000;
			if (sd->ud.skilltimer != INVALID_TIMER && (skill_lv = pc_checkskill(sd, SA_FREECAST)) > 0)
				amotion = amotion * 5 * (skill_lv + 10) / 100;
#ifdef RENEWAL_ASPD
			// RE ASPD % modifier
			amotion += (max(0xc3 - amotion, 2) * (status->aspd_rate2 + status_calc_aspd(bl, sc, false))) / 100;
			amotion = 10 * (200 - amotion);

			amotion += sd->bonus.aspd_add;
#endif
			amotion = status_calc_fix_aspd(bl, sc, amotion);
			status->amotion = cap_value(amotion,pc_maxaspd(sd),2000);

			status->adelay = 2 * status->amotion;
		} else { // Mercenary and mobs
			amotion = b_status->amotion;
			status->aspd_rate = status_calc_aspd_rate(bl, sc, b_status->aspd_rate);
			amotion = amotion*status->aspd_rate/1000;

			amotion = status_calc_fix_aspd(bl, sc, amotion);
			status->amotion = cap_value(amotion, battle_config.monster_max_aspd, 2000);

			temp = b_status->adelay*status->aspd_rate/1000;
			status->adelay = cap_value(temp, battle_config.monster_max_aspd*2, 4000);
		}
	}

	if(flag&SCB_DSPD) {
		int dmotion;
		if( bl->type&BL_PC ) {
			if (b_status->agi == status->agi)
				status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
			else {
				dmotion = 800-status->agi*4;
				status->dmotion = cap_value(dmotion, 400, 800);
				if(battle_config.pc_damage_delay_rate != 100)
					status->dmotion = status->dmotion*battle_config.pc_damage_delay_rate/100;
				// It's safe to ignore b_status->dmotion since no bonus affects it.
				status->dmotion = status_calc_dmotion(bl, sc, status->dmotion);
			}
		} else if( bl->type&BL_HOM ) {
			dmotion = 800-status->agi*4;
			status->dmotion = cap_value(dmotion, 400, 800);
			status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
		} else { // Mercenary and mobs
			status->dmotion = status_calc_dmotion(bl, sc, b_status->dmotion);
		}
	}

	if(flag&(SCB_VIT|SCB_MAXHP|SCB_INT|SCB_MAXSP) && bl->type&BL_REGEN)
		status_calc_regen(bl, status, status_get_regen_data(bl));

	if(flag&SCB_REGEN && bl->type&BL_REGEN)
		status_calc_regen_rate(bl, status_get_regen_data(bl), sc);
}

/**
 * Recalculates parts of an objects status according to specified flags
 * Also sends updates to the client when necessary
 * See [set_sc] [add_sc]
 * @param bl: Object whose status has changed [PC|MOB|HOM|MER|ELEM]
 * @param flag: Which status has changed on bl
 * @param opt: If true, will cause status_calc_* functions to run their base status initialization code
 */
void status_calc_bl_(struct block_list* bl, enum e_scb_flag flag, enum e_status_calc_opt opt)
{
	struct status_data b_status; // Previous battle status
	struct status_data* status; // Pointer to current battle status

	if (bl->type == BL_PC && ((TBL_PC*)bl)->delayed_damage != 0) {
		if (opt&SCO_FORCE)
			((TBL_PC*)bl)->state.hold_recalc = 0; /* Clear and move on */
		else {
			((TBL_PC*)bl)->state.hold_recalc = 1; /* Flag and stop */
			return;
		}
	}

	// Remember previous values
	status = status_get_status_data(bl);
	memcpy(&b_status, status, sizeof(struct status_data));

	if( flag&SCB_BASE ) { // Calculate the object's base status too
		switch( bl->type ) {
		case BL_PC:  status_calc_pc_(BL_CAST(BL_PC,bl), opt);          break;
		case BL_MOB: status_calc_mob_(BL_CAST(BL_MOB,bl), opt);        break;
		case BL_PET: status_calc_pet_(BL_CAST(BL_PET,bl), opt);        break;
		case BL_HOM: status_calc_homunculus_(BL_CAST(BL_HOM,bl), opt); break;
		case BL_MER: status_calc_mercenary_(BL_CAST(BL_MER,bl), opt);  break;
		case BL_ELEM: status_calc_elemental_(BL_CAST(BL_ELEM,bl), opt);  break;
		case BL_NPC: status_calc_npc_(BL_CAST(BL_NPC,bl), opt); break;
		}
	}

	if( bl->type == BL_PET )
		return; // Pets are not affected by statuses

	if (opt&SCO_FIRST && bl->type == BL_MOB)
		return; // Assume there will be no statuses active

	status_calc_bl_main(bl, flag);

	if (opt&SCO_FIRST && bl->type == BL_HOM)
		return; // Client update handled by caller

	// Compare against new values and send client updates
	if( bl->type == BL_PC ) {
		TBL_PC* sd = BL_CAST(BL_PC, bl);

		if(b_status.str != status->str)
			clif_updatestatus(sd,SP_STR);
		if(b_status.agi != status->agi)
			clif_updatestatus(sd,SP_AGI);
		if(b_status.vit != status->vit)
			clif_updatestatus(sd,SP_VIT);
		if(b_status.int_ != status->int_)
			clif_updatestatus(sd,SP_INT);
		if(b_status.dex != status->dex)
			clif_updatestatus(sd,SP_DEX);
		if(b_status.luk != status->luk)
			clif_updatestatus(sd,SP_LUK);
		if(b_status.hit != status->hit)
			clif_updatestatus(sd,SP_HIT);
		if(b_status.flee != status->flee)
			clif_updatestatus(sd,SP_FLEE1);
		if(b_status.amotion != status->amotion)
			clif_updatestatus(sd,SP_ASPD);
		if(b_status.speed != status->speed)
			clif_updatestatus(sd,SP_SPEED);

		if(b_status.batk != status->batk
#ifndef RENEWAL
			|| b_status.rhw.atk != status->rhw.atk || b_status.lhw.atk != status->lhw.atk
#endif
			)
			clif_updatestatus(sd,SP_ATK1);

		if(b_status.def != status->def) {
			clif_updatestatus(sd,SP_DEF1);
#ifdef RENEWAL
			clif_updatestatus(sd,SP_DEF2);
#endif
		}

		if(
#ifdef RENEWAL
			b_status.watk != status->watk || b_status.watk2 != status->watk2 || b_status.eatk != status->eatk
#else
			b_status.rhw.atk2 != status->rhw.atk2 || b_status.lhw.atk2 != status->lhw.atk2
#endif
			)
			clif_updatestatus(sd,SP_ATK2);

		if(b_status.def2 != status->def2) {
			clif_updatestatus(sd,SP_DEF2);
#ifdef RENEWAL
			clif_updatestatus(sd,SP_DEF1);
#endif
		}
		if(b_status.flee2 != status->flee2)
			clif_updatestatus(sd,SP_FLEE2);
		if(b_status.cri != status->cri)
			clif_updatestatus(sd,SP_CRITICAL);
#ifndef RENEWAL
		if(b_status.matk_max != status->matk_max)
			clif_updatestatus(sd,SP_MATK1);
		if(b_status.matk_min != status->matk_min)
			clif_updatestatus(sd,SP_MATK2);
#else
		if(b_status.matk_max != status->matk_max || b_status.matk_min != status->matk_min) {
			clif_updatestatus(sd,SP_MATK2);
			clif_updatestatus(sd,SP_MATK1);
		}
#endif
		if(b_status.mdef != status->mdef) {
			clif_updatestatus(sd,SP_MDEF1);
#ifdef RENEWAL
			clif_updatestatus(sd,SP_MDEF2);
#endif
		}
		if(b_status.mdef2 != status->mdef2) {
			clif_updatestatus(sd,SP_MDEF2);
#ifdef RENEWAL
			clif_updatestatus(sd,SP_MDEF1);
#endif
		}
		if(b_status.rhw.range != status->rhw.range)
			clif_updatestatus(sd,SP_ATTACKRANGE);
		if(b_status.max_hp != status->max_hp)
			clif_updatestatus(sd,SP_MAXHP);
		if(b_status.max_sp != status->max_sp)
			clif_updatestatus(sd,SP_MAXSP);
		if(b_status.hp != status->hp)
			clif_updatestatus(sd,SP_HP);
		if(b_status.sp != status->sp)
			clif_updatestatus(sd,SP_SP);
	} else if( bl->type == BL_HOM ) {
		TBL_HOM* hd = BL_CAST(BL_HOM, bl);

		if( hd->master && memcmp(&b_status, status, sizeof(struct status_data)) != 0 )
			clif_hominfo(hd->master,hd,0);
	} else if( bl->type == BL_MER ) {
		TBL_MER* md = BL_CAST(BL_MER, bl);

		if (!md->master)
			return;

		if( b_status.rhw.atk != status->rhw.atk || b_status.rhw.atk2 != status->rhw.atk2 )
			clif_mercenary_updatestatus(md->master, SP_ATK1);
		if( b_status.matk_max != status->matk_max )
			clif_mercenary_updatestatus(md->master, SP_MATK1);
		if( b_status.hit != status->hit )
			clif_mercenary_updatestatus(md->master, SP_HIT);
		if( b_status.cri != status->cri )
			clif_mercenary_updatestatus(md->master, SP_CRITICAL);
		if( b_status.def != status->def )
			clif_mercenary_updatestatus(md->master, SP_DEF1);
		if( b_status.mdef != status->mdef )
			clif_mercenary_updatestatus(md->master, SP_MDEF1);
		if( b_status.flee != status->flee )
			clif_mercenary_updatestatus(md->master, SP_MERCFLEE);
		if( b_status.amotion != status->amotion )
			clif_mercenary_updatestatus(md->master, SP_ASPD);
		if( b_status.max_hp != status->max_hp )
			clif_mercenary_updatestatus(md->master, SP_MAXHP);
		if( b_status.max_sp != status->max_sp )
			clif_mercenary_updatestatus(md->master, SP_MAXSP);
		if( b_status.hp != status->hp )
			clif_mercenary_updatestatus(md->master, SP_HP);
		if( b_status.sp != status->sp )
			clif_mercenary_updatestatus(md->master, SP_SP);
	} else if( bl->type == BL_ELEM ) {
		TBL_ELEM* ed = BL_CAST(BL_ELEM, bl);

		if (!ed->master)
			return;

		if( b_status.max_hp != status->max_hp )
			clif_elemental_updatestatus(ed->master, SP_MAXHP);
		if( b_status.max_sp != status->max_sp )
			clif_elemental_updatestatus(ed->master, SP_MAXSP);
		if( b_status.hp != status->hp )
			clif_elemental_updatestatus(ed->master, SP_HP);
		if( b_status.sp != status->sp )
			clif_mercenary_updatestatus(ed->master, SP_SP);
	}
}

/**
 * Adds strength modifications based on status changes
 * @param bl: Object to change str [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param str: Initial str
 * @return modified str with cap_value(str,0,USHRT_MAX)
 */
static unsigned short status_calc_str(struct block_list *bl, struct status_change *sc, int str)
{
	if(!sc || !sc->count)
		return cap_value(str,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		str -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(str,0,USHRT_MAX);
	}
	if(sc->data[SC_INCALLSTATUS])
		str += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_CHASEWALK2])
		str += sc->data[SC_CHASEWALK2]->val1;
	if(sc->data[SC_INCSTR])
		str += sc->data[SC_INCSTR]->val1;
	if(sc->data[SC_STRFOOD])
		str += sc->data[SC_STRFOOD]->val1;
	if(sc->data[SC_FOOD_STR_CASH])
		str += sc->data[SC_FOOD_STR_CASH]->val1;
	if(sc->data[SC_BATTLEORDERS])
		str += 5;
	if(sc->data[SC_LEADERSHIP])
		str += sc->data[SC_LEADERSHIP]->val1;
	if(sc->data[SC_LOUD])
		str += 4;
	if(sc->data[SC_TRUESIGHT])
		str += 5;
	if(sc->data[SC_SPURT])
		str += 10;
	if(sc->data[SC_NEN])
		str += sc->data[SC_NEN]->val1;
	if(sc->data[SC_BLESSING]) {
		if(sc->data[SC_BLESSING]->val2)
			str += sc->data[SC_BLESSING]->val2;
		else
			str >>= 1;
	}
	if(sc->data[SC_MARIONETTE])
		str -= ((sc->data[SC_MARIONETTE]->val3)>>16)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		str += ((sc->data[SC_MARIONETTE2]->val3)>>16)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
		str += ((sc->data[SC_SPIRIT]->val3)>>16)&0xFF;
	if(sc->data[SC_GIANTGROWTH])
		str += 30;
	if(sc->data[SC_BEYONDOFWARCRY])
		str -= sc->data[SC_BEYONDOFWARCRY]->val3;
	if(sc->data[SC_SAVAGE_STEAK])
		str += sc->data[SC_SAVAGE_STEAK]->val1;
	if(sc->data[SC_INSPIRATION])
		str += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_2011RWC_SCROLL])
		str += sc->data[SC_2011RWC_SCROLL]->val1;
	if(sc->data[SC_STOMACHACHE])
		str -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		str -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_SWORDCLAN])
		str += 1;
	if(sc->data[SC_JUMPINGCLAN])
		str += 1;
	if(sc->data[SC_FULL_THROTTLE])
		str += str * sc->data[SC_FULL_THROTTLE]->val3 / 100;
	if(sc->data[SC_CHEERUP])
		str += 3;
	if(sc->data[SC_GLASTHEIM_STATE])
		str += sc->data[SC_GLASTHEIM_STATE]->val1;

	return (unsigned short)cap_value(str,0,USHRT_MAX);
}

/**
 * Adds agility modifications based on status changes
 * @param bl: Object to change agi [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param agi: Initial agi
 * @return modified agi with cap_value(agi,0,USHRT_MAX)
 */
static unsigned short status_calc_agi(struct block_list *bl, struct status_change *sc, int agi)
{
	if(!sc || !sc->count)
		return cap_value(agi,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		agi -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(agi,0,USHRT_MAX);
	}
	if(sc->data[SC_CONCENTRATE] && !sc->data[SC_QUAGMIRE])
		agi += (agi-sc->data[SC_CONCENTRATE]->val3)*sc->data[SC_CONCENTRATE]->val2/100;
	if(sc->data[SC_INCALLSTATUS])
		agi += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCAGI])
		agi += sc->data[SC_INCAGI]->val1;
	if(sc->data[SC_AGIFOOD])
		agi += sc->data[SC_AGIFOOD]->val1;
	if(sc->data[SC_FOOD_AGI_CASH])
		agi += sc->data[SC_FOOD_AGI_CASH]->val1;
	if(sc->data[SC_SOULCOLD])
		agi += sc->data[SC_SOULCOLD]->val1;
	if(sc->data[SC_TRUESIGHT])
		agi += 5;
	if(sc->data[SC_INCREASEAGI])
		agi += sc->data[SC_INCREASEAGI]->val2;
	if(sc->data[SC_INCREASING])
		agi += 4; // Added based on skill updates [Reddozen]
	if(sc->data[SC_2011RWC_SCROLL])
		agi += sc->data[SC_2011RWC_SCROLL]->val1;
	if(sc->data[SC_DECREASEAGI])
		agi -= sc->data[SC_DECREASEAGI]->val2;
	if(sc->data[SC_QUAGMIRE])
		agi -= sc->data[SC_QUAGMIRE]->val2;
	if(sc->data[SC_SUITON] && sc->data[SC_SUITON]->val3)
		agi -= sc->data[SC_SUITON]->val2;
	if(sc->data[SC_MARIONETTE])
		agi -= ((sc->data[SC_MARIONETTE]->val3)>>8)&0xFF;
	if(sc->data[SC_MARIONETTE2])
		agi += ((sc->data[SC_MARIONETTE2]->val3)>>8)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
		agi += ((sc->data[SC_SPIRIT]->val3)>>8)&0xFF;
	if(sc->data[SC_ADORAMUS])
		agi -= sc->data[SC_ADORAMUS]->val2;
	if(sc->data[SC_MARSHOFABYSS])
		agi -= agi * sc->data[SC_MARSHOFABYSS]->val2 / 100;
	if(sc->data[SC_DROCERA_HERB_STEAMED])
		agi += sc->data[SC_DROCERA_HERB_STEAMED]->val1;
	if(sc->data[SC_INSPIRATION])
		agi += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		agi -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		agi -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_CROSSBOWCLAN])
		agi += 1;
	if(sc->data[SC_JUMPINGCLAN])
		agi += 1;
	if(sc->data[SC_FULL_THROTTLE])
		agi += agi * sc->data[SC_FULL_THROTTLE]->val3 / 100;
	if (sc->data[SC_ARCLOUSEDASH])
		agi += sc->data[SC_ARCLOUSEDASH]->val2;
	if(sc->data[SC_CHEERUP])
		agi += 3;
	if(sc->data[SC_GLASTHEIM_STATE])
		agi += sc->data[SC_GLASTHEIM_STATE]->val1;

	return (unsigned short)cap_value(agi,0,USHRT_MAX);
}

/**
 * Adds vitality modifications based on status changes
 * @param bl: Object to change vit [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param vit: Initial vit
 * @return modified vit with cap_value(vit,0,USHRT_MAX)
 */
static unsigned short status_calc_vit(struct block_list *bl, struct status_change *sc, int vit)
{
	if(!sc || !sc->count)
		return cap_value(vit,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		vit -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(vit,0,USHRT_MAX);
	}
	if(sc->data[SC_INCALLSTATUS])
		vit += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCVIT])
		vit += sc->data[SC_INCVIT]->val1;
	if(sc->data[SC_VITFOOD])
		vit += sc->data[SC_VITFOOD]->val1;
	if(sc->data[SC_FOOD_VIT_CASH])
		vit += sc->data[SC_FOOD_VIT_CASH]->val1;
	if(sc->data[SC_CHANGE])
		vit += sc->data[SC_CHANGE]->val2;
	if(sc->data[SC_GLORYWOUNDS])
		vit += sc->data[SC_GLORYWOUNDS]->val1;
	if(sc->data[SC_TRUESIGHT])
		vit += 5;
	if(sc->data[SC_MARIONETTE])
		vit -= sc->data[SC_MARIONETTE]->val3&0xFF;
	if(sc->data[SC_MARIONETTE2])
		vit += sc->data[SC_MARIONETTE2]->val3&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
		vit += sc->data[SC_SPIRIT]->val3&0xFF;
	if(sc->data[SC_MINOR_BBQ])
		vit += sc->data[SC_MINOR_BBQ]->val1;
	if(sc->data[SC_INSPIRATION])
		vit += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_2011RWC_SCROLL])
		vit += sc->data[SC_2011RWC_SCROLL]->val1;
	if(sc->data[SC_STOMACHACHE])
		vit -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		vit -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_SWORDCLAN])
		vit += 1;
	if(sc->data[SC_JUMPINGCLAN])
		vit += 1;
	if(sc->data[SC_STRIPARMOR] && bl->type != BL_PC)
		vit -= vit * sc->data[SC_STRIPARMOR]->val2/100;
	if(sc->data[SC_FULL_THROTTLE])
		vit += vit * sc->data[SC_FULL_THROTTLE]->val3 / 100;
#ifdef RENEWAL
	if(sc->data[SC_DEFENCE])
		vit += sc->data[SC_DEFENCE]->val2;
#endif
	if(sc->data[SC_CHEERUP])
		vit += 3;
	if(sc->data[SC_GLASTHEIM_STATE])
		vit += sc->data[SC_GLASTHEIM_STATE]->val1;

	return (unsigned short)cap_value(vit,0,USHRT_MAX);
}

/**
 * Adds intelligence modifications based on status changes
 * @param bl: Object to change int [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param int_: Initial int
 * @return modified int with cap_value(int_,0,USHRT_MAX)
 */
static unsigned short status_calc_int(struct block_list *bl, struct status_change *sc, int int_)
{
	if(!sc || !sc->count)
		return cap_value(int_,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		int_ -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(int_,0,USHRT_MAX);
	}
	if(sc->data[SC_INCALLSTATUS])
		int_ += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCINT])
		int_ += sc->data[SC_INCINT]->val1;
	if(sc->data[SC_INTFOOD])
		int_ += sc->data[SC_INTFOOD]->val1;
	if(sc->data[SC_FOOD_INT_CASH])
		int_ += sc->data[SC_FOOD_INT_CASH]->val1;
	if(sc->data[SC_CHANGE])
		int_ += sc->data[SC_CHANGE]->val3;
	if(sc->data[SC_BATTLEORDERS])
		int_ += 5;
	if(sc->data[SC_TRUESIGHT])
		int_ += 5;
	if(sc->data[SC_BLESSING]) {
		if (sc->data[SC_BLESSING]->val2)
			int_ += sc->data[SC_BLESSING]->val2;
		else
			int_ >>= 1;
	}
	if(sc->data[SC_NEN])
		int_ += sc->data[SC_NEN]->val1;
	if(sc->data[SC_MARIONETTE])
		int_ -= ((sc->data[SC_MARIONETTE]->val4)>>16)&0xFF;
	if(sc->data[SC_2011RWC_SCROLL])
		int_ += sc->data[SC_2011RWC_SCROLL]->val1;
	if(sc->data[SC_MARIONETTE2])
		int_ += ((sc->data[SC_MARIONETTE2]->val4)>>16)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
		int_ += ((sc->data[SC_SPIRIT]->val4)>>16)&0xFF;
	if(sc->data[SC_INSPIRATION])
		int_ += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_MELODYOFSINK])
		int_ -= sc->data[SC_MELODYOFSINK]->val3;
	if(sc->data[SC_MANDRAGORA])
		int_ -= 4 * sc->data[SC_MANDRAGORA]->val1;
	if(sc->data[SC_COCKTAIL_WARG_BLOOD])
		int_ += sc->data[SC_COCKTAIL_WARG_BLOOD]->val1;
	if(sc->data[SC_STOMACHACHE])
		int_ -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		int_ -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_ARCWANDCLAN])
		int_ += 1;
	if(sc->data[SC_GOLDENMACECLAN])
		int_ += 1;
	if(sc->data[SC_JUMPINGCLAN])
		int_ += 1;
	if(sc->data[SC_FULL_THROTTLE])
		int_ += int_ * sc->data[SC_FULL_THROTTLE]->val3 / 100;
	if(sc->data[SC_CHEERUP])
		int_ += 3;
	if(sc->data[SC_GLASTHEIM_STATE])
		int_ += sc->data[SC_GLASTHEIM_STATE]->val1;

	if(bl->type != BL_PC) {
		if(sc->data[SC_STRIPHELM])
			int_ -= int_ * sc->data[SC_STRIPHELM]->val2/100;
		if(sc->data[SC__STRIPACCESSORY])
			int_ -= int_ * sc->data[SC__STRIPACCESSORY]->val2 / 100;
	}

	return (unsigned short)cap_value(int_,0,USHRT_MAX);
}

/**
 * Adds dexterity modifications based on status changes
 * @param bl: Object to change dex [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param dex: Initial dex
 * @return modified dex with cap_value(dex,0,USHRT_MAX)
 */
static unsigned short status_calc_dex(struct block_list *bl, struct status_change *sc, int dex)
{
	if(!sc || !sc->count)
		return cap_value(dex,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		dex -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(dex,0,USHRT_MAX);
	}
	if(sc->data[SC_CONCENTRATE] && !sc->data[SC_QUAGMIRE])
		dex += (dex-sc->data[SC_CONCENTRATE]->val4)*sc->data[SC_CONCENTRATE]->val2/100;
	if(sc->data[SC_INCALLSTATUS])
		dex += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCDEX])
		dex += sc->data[SC_INCDEX]->val1;
	if(sc->data[SC_DEXFOOD])
		dex += sc->data[SC_DEXFOOD]->val1;
	if(sc->data[SC_FOOD_DEX_CASH])
		dex += sc->data[SC_FOOD_DEX_CASH]->val1;
	if(sc->data[SC_BATTLEORDERS])
		dex += 5;
	if(sc->data[SC_HAWKEYES])
		dex += sc->data[SC_HAWKEYES]->val1;
	if(sc->data[SC_TRUESIGHT])
		dex += 5;
	if(sc->data[SC_QUAGMIRE])
		dex -= sc->data[SC_QUAGMIRE]->val2;
	if(sc->data[SC_BLESSING]) {
		if (sc->data[SC_BLESSING]->val2)
			dex += sc->data[SC_BLESSING]->val2;
		else
			dex >>= 1;
	}
	if(sc->data[SC_INCREASING])
		dex += 4; // Added based on skill updates [Reddozen]
	if(sc->data[SC_MARIONETTE])
		dex -= ((sc->data[SC_MARIONETTE]->val4)>>8)&0xFF;
	if(sc->data[SC_2011RWC_SCROLL])
		dex += sc->data[SC_2011RWC_SCROLL]->val1;
	if(sc->data[SC_MARIONETTE2])
		dex += ((sc->data[SC_MARIONETTE2]->val4)>>8)&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
		dex += ((sc->data[SC_SPIRIT]->val4)>>8)&0xFF;
	if(sc->data[SC_SIROMA_ICE_TEA])
		dex += sc->data[SC_SIROMA_ICE_TEA]->val1;
	if(sc->data[SC_INSPIRATION])
		dex += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		dex -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		dex -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_ARCWANDCLAN])
		dex += 1;
	if(sc->data[SC_CROSSBOWCLAN])
		dex += 1;
	if(sc->data[SC_JUMPINGCLAN])
		dex += 1;
	if(sc->data[SC__STRIPACCESSORY] && bl->type != BL_PC)
		dex -= dex * sc->data[SC__STRIPACCESSORY]->val2 / 100;
	if(sc->data[SC_MARSHOFABYSS])
		dex -= dex * sc->data[SC_MARSHOFABYSS]->val2 / 100;
	if(sc->data[SC_FULL_THROTTLE])
		dex += dex * sc->data[SC_FULL_THROTTLE]->val3 / 100;
	if(sc->data[SC_CHEERUP])
		dex += 3;
	if(sc->data[SC_GLASTHEIM_STATE])
		dex += sc->data[SC_GLASTHEIM_STATE]->val1;

	return (unsigned short)cap_value(dex,0,USHRT_MAX);
}

/**
 * Adds luck modifications based on status changes
 * @param bl: Object to change luk [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param luk: Initial luk
 * @return modified luk with cap_value(luk,0,USHRT_MAX)
 */
static unsigned short status_calc_luk(struct block_list *bl, struct status_change *sc, int luk)
{
	if(!sc || !sc->count)
		return cap_value(luk,0,USHRT_MAX);

	if(sc->data[SC_HARMONIZE]) {
		luk -= sc->data[SC_HARMONIZE]->val2;
		return (unsigned short)cap_value(luk,0,USHRT_MAX);
	}
	if(sc->data[SC_CURSE])
		return 0;
	if(sc->data[SC_INCALLSTATUS])
		luk += sc->data[SC_INCALLSTATUS]->val1;
	if(sc->data[SC_INCLUK])
		luk += sc->data[SC_INCLUK]->val1;
	if(sc->data[SC_LUKFOOD])
		luk += sc->data[SC_LUKFOOD]->val1;
	if(sc->data[SC_FOOD_LUK_CASH])
		luk += sc->data[SC_FOOD_LUK_CASH]->val1;
	if(sc->data[SC_TRUESIGHT])
		luk += 5;
	if(sc->data[SC_GLORIA])
		luk += 30;
	if(sc->data[SC_MARIONETTE])
		luk -= sc->data[SC_MARIONETTE]->val4&0xFF;
	if(sc->data[SC_MARIONETTE2])
		luk += sc->data[SC_MARIONETTE2]->val4&0xFF;
	if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
		luk += sc->data[SC_SPIRIT]->val4&0xFF;
	if(sc->data[SC_PUTTI_TAILS_NOODLES])
		luk += sc->data[SC_PUTTI_TAILS_NOODLES]->val1;
	if(sc->data[SC_INSPIRATION])
		luk += sc->data[SC_INSPIRATION]->val3;
	if(sc->data[SC_STOMACHACHE])
		luk -= sc->data[SC_STOMACHACHE]->val1;
	if(sc->data[SC_KYOUGAKU])
		luk -= sc->data[SC_KYOUGAKU]->val2;
	if(sc->data[SC_2011RWC_SCROLL])
		luk += sc->data[SC_2011RWC_SCROLL]->val1;
	if(sc->data[SC__STRIPACCESSORY] && bl->type != BL_PC)
		luk -= luk * sc->data[SC__STRIPACCESSORY]->val2 / 100;
	if(sc->data[SC_BANANA_BOMB])
		luk -= 75;
	if(sc->data[SC_GOLDENMACECLAN])
		luk += 1;
	if(sc->data[SC_JUMPINGCLAN])
		luk += 1;
	if(sc->data[SC_FULL_THROTTLE])
		luk += luk * sc->data[SC_FULL_THROTTLE]->val3 / 100;
	if(sc->data[SC_CHEERUP])
		luk += 3;
	if(sc->data[SC_GLASTHEIM_STATE])
		luk += sc->data[SC_GLASTHEIM_STATE]->val1;

	return (unsigned short)cap_value(luk,0,USHRT_MAX);
}

/**
 * Adds base attack modifications based on status changes
 * @param bl: Object to change batk [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param batk: Initial batk
 * @return modified batk with cap_value(batk,0,USHRT_MAX)
 */
static unsigned short status_calc_batk(struct block_list *bl, struct status_change *sc, int batk)
{
	if(!sc || !sc->count)
		return cap_value(batk,0,USHRT_MAX);

	if(sc->data[SC_ATKPOTION])
		batk += sc->data[SC_ATKPOTION]->val1;
	if(sc->data[SC_BATKFOOD])
		batk += sc->data[SC_BATKFOOD]->val1;
#ifndef RENEWAL
	if(sc->data[SC_GATLINGFEVER])
		batk += sc->data[SC_GATLINGFEVER]->val3;
	if(sc->data[SC_MADNESSCANCEL])
		batk += 100;
#endif
	if(sc->data[SC_FULL_SWING_K])
		batk += sc->data[SC_FULL_SWING_K]->val1;
	if(sc->data[SC_ASH])
		batk -= batk * sc->data[SC_ASH]->val4 / 100;
	if(bl->type == BL_HOM && sc->data[SC_PYROCLASTIC])
		batk += sc->data[SC_PYROCLASTIC]->val2;
	if (sc->data[SC_ANGRIFFS_MODUS])
		batk += sc->data[SC_ANGRIFFS_MODUS]->val2;
	if(sc->data[SC_2011RWC_SCROLL])
		batk += 30;
	if(sc->data[SC_INCATKRATE])
		batk += batk * sc->data[SC_INCATKRATE]->val1/100;
	if(sc->data[SC_PROVOKE])
		batk += batk * sc->data[SC_PROVOKE]->val3/100;
#ifndef RENEWAL
	if(sc->data[SC_CONCENTRATION])
		batk += batk * sc->data[SC_CONCENTRATION]->val2/100;
#endif
	if(sc->data[SC_SKE])
		batk += batk * 3;
	if(sc->data[SC_BLOODLUST])
		batk += batk * sc->data[SC_BLOODLUST]->val2/100;
	if(sc->data[SC_JOINTBEAT] && sc->data[SC_JOINTBEAT]->val2&BREAK_WAIST)
		batk -= batk * 25/100;
	if(sc->data[SC_CURSE])
		batk -= batk * 25/100;
	/* Curse shouldn't effect on this? <- Curse OR Bleeding??
	if(sc->data[SC_BLEEDING])
		batk -= batk * 25 / 100; */
	if(sc->data[SC_FLEET])
		batk += batk * sc->data[SC_FLEET]->val3/100;
	if(sc->data[SC__ENERVATION])
		batk -= batk * sc->data[SC__ENERVATION]->val2 / 100;
	if(sc->data[SC_SATURDAYNIGHTFEVER])
		batk += 100 * sc->data[SC_SATURDAYNIGHTFEVER]->val1;
	if( sc->data[SC_ZANGETSU] )
		batk += sc->data[SC_ZANGETSU]->val2;
	if(sc->data[SC_QUEST_BUFF1])
		batk += sc->data[SC_QUEST_BUFF1]->val1;
	if(sc->data[SC_QUEST_BUFF2])
		batk += sc->data[SC_QUEST_BUFF2]->val1;
	if(sc->data[SC_QUEST_BUFF3])
		batk += sc->data[SC_QUEST_BUFF3]->val1;
	if (sc->data[SC_SHRIMP])
		batk += batk * sc->data[SC_SHRIMP]->val2 / 100;

	return (unsigned short)cap_value(batk,0,USHRT_MAX);
}

/**
 * Adds weapon attack modifications based on status changes
 * @param bl: Object to change watk [PC]
 * @param sc: Object's status change information
 * @param watk: Initial watk
 * @return modified watk with cap_value(watk,0,USHRT_MAX)
 */
static unsigned short status_calc_watk(struct block_list *bl, struct status_change *sc, int watk)
{
	if(!sc || !sc->count)
		return cap_value(watk,0,USHRT_MAX);

#ifndef RENEWAL
	if(sc->data[SC_IMPOSITIO])
		watk += sc->data[SC_IMPOSITIO]->val2;
	if(sc->data[SC_DRUMBATTLE])
		watk += sc->data[SC_DRUMBATTLE]->val2;
#endif
	if(sc->data[SC_WATKFOOD])
		watk += sc->data[SC_WATKFOOD]->val1;
	if(sc->data[SC_VOLCANO])
		watk += sc->data[SC_VOLCANO]->val2;
	if(sc->data[SC_MERC_ATKUP])
		watk += sc->data[SC_MERC_ATKUP]->val2;
	if(sc->data[SC_WATER_BARRIER])
		watk -= sc->data[SC_WATER_BARRIER]->val2;
#ifndef RENEWAL
	if(sc->data[SC_NIBELUNGEN]) {
		if (bl->type != BL_PC)
			watk += sc->data[SC_NIBELUNGEN]->val2;
		else {
			TBL_PC *sd = (TBL_PC*)bl;
			short index = sd->equip_index[sd->state.lr_flag?EQI_HAND_L:EQI_HAND_R];

			if(index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->wlv == 4)
				watk += sc->data[SC_NIBELUNGEN]->val2;
		}
	}
	if(sc->data[SC_CONCENTRATION])
		watk += watk * sc->data[SC_CONCENTRATION]->val2 / 100;
#endif
	if(sc->data[SC_INCATKRATE])
		watk += watk * sc->data[SC_INCATKRATE]->val1/100;
	if(sc->data[SC_PROVOKE])
		watk += watk * sc->data[SC_PROVOKE]->val3/100;
	if(sc->data[SC_SKE])
		watk += watk * 3;
	if(sc->data[SC_FLEET])
		watk += watk * sc->data[SC_FLEET]->val3/100;
	if(sc->data[SC_CURSE])
		watk -= watk * 25/100;
	if(sc->data[SC_STRIPWEAPON] && bl->type != BL_PC)
		watk -= watk * sc->data[SC_STRIPWEAPON]->val2/100;
	if(sc->data[SC_FIGHTINGSPIRIT])
		watk += sc->data[SC_FIGHTINGSPIRIT]->val1;
	if(sc->data[SC_SHIELDSPELL_DEF] && sc->data[SC_SHIELDSPELL_DEF]->val1 == 3)
		watk += sc->data[SC_SHIELDSPELL_DEF]->val2;
	if(sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 1)
		watk += (10 + 10 * sc->data[SC_BANDING]->val1) * sc->data[SC_BANDING]->val2;
	if(sc->data[SC_INSPIRATION])
		watk += 40 * sc->data[SC_INSPIRATION]->val1 + 3 * sc->data[SC_INSPIRATION]->val2;
	if(sc->data[SC_GT_CHANGE])
		watk += sc->data[SC_GT_CHANGE]->val2;
	if(sc->data[SC__ENERVATION])
		watk -= watk * sc->data[SC__ENERVATION]->val2 / 100;
	if(sc->data[SC_STRIKING])
		watk += sc->data[SC_STRIKING]->val2;
	if(sc->data[SC_RUSHWINDMILL])
		watk += sc->data[SC_RUSHWINDMILL]->val3;
	if(sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 2)
		watk += 50;
	if((sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 2)
	   || (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2)
	   || (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 2)
	   || (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2))
		watk += watk * 10 / 100;
	if(sc->data[SC_PYROTECHNIC_OPTION])
		watk += sc->data[SC_PYROTECHNIC_OPTION]->val2;
	if(sc->data[SC_HEATER_OPTION])
		watk += sc->data[SC_HEATER_OPTION]->val2;
	if(sc->data[SC_TROPIC_OPTION])
		watk += sc->data[SC_TROPIC_OPTION]->val2;
	if( sc && sc->data[SC_TIDAL_WEAPON] )
		watk += watk * sc->data[SC_TIDAL_WEAPON]->val2 / 100;
	if(bl->type == BL_PC && sc->data[SC_PYROCLASTIC])
		watk += sc->data[SC_PYROCLASTIC]->val2;
	if(sc->data[SC_ANGRIFFS_MODUS])
		watk += watk * sc->data[SC_ANGRIFFS_MODUS]->val2/100;
	if(sc->data[SC_ODINS_POWER])
		watk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1;
	if (sc->data[SC_FLASHCOMBO])
		watk += sc->data[SC_FLASHCOMBO]->val2;
	if (sc->data[SC_CATNIPPOWDER])
		watk -= watk * sc->data[SC_CATNIPPOWDER]->val2 / 100;
	if (sc->data[SC_CHATTERING])
		watk += sc->data[SC_CHATTERING]->val2;

	return (unsigned short)cap_value(watk,0,USHRT_MAX);
}

#ifdef RENEWAL
/**
 * Adds equip magic attack modifications based on status changes [RENEWAL]
 * @param bl: Object to change matk [PC]
 * @param sc: Object's status change information
 * @param matk: Initial matk
 * @return modified matk with cap_value(matk,0,USHRT_MAX)
 */
static unsigned short status_calc_ematk(struct block_list *bl, struct status_change *sc, int matk)
{
	if (!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);

	if (sc->data[SC_MATKPOTION])
		matk += sc->data[SC_MATKPOTION]->val1;
	if (sc->data[SC_MATKFOOD])
		matk += sc->data[SC_MATKFOOD]->val1;
	if(sc->data[SC_MANA_PLUS])
		matk += sc->data[SC_MANA_PLUS]->val1;
	if(sc->data[SC_COOLER_OPTION])
		matk += sc->data[SC_COOLER_OPTION]->val2;
	if(sc->data[SC_AQUAPLAY_OPTION])
		matk += sc->data[SC_AQUAPLAY_OPTION]->val2;
	if(sc->data[SC_CHILLY_AIR_OPTION])
		matk += sc->data[SC_CHILLY_AIR_OPTION]->val2;
	if(sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3)
		matk += 50;
	if(sc->data[SC_ODINS_POWER])
		matk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1; // 70 lvl1, 100lvl2
	if(sc->data[SC_MOONLITSERENADE])
		matk += sc->data[SC_MOONLITSERENADE]->val3;
	if(sc->data[SC_IZAYOI])
		matk += 25 * sc->data[SC_IZAYOI]->val1;
	if(sc->data[SC_ZANGETSU])
		matk += sc->data[SC_ZANGETSU]->val3;
	if(sc->data[SC_QUEST_BUFF1])
		matk += sc->data[SC_QUEST_BUFF1]->val1;
	if(sc->data[SC_QUEST_BUFF2])
		matk += sc->data[SC_QUEST_BUFF2]->val1;
	if(sc->data[SC_QUEST_BUFF3])
		matk += sc->data[SC_QUEST_BUFF3]->val1;
	if(sc->data[SC_MTF_MATK2])
		matk += sc->data[SC_MTF_MATK2]->val1;
	if(sc->data[SC_2011RWC_SCROLL])
		matk += 30;
	if (sc->data[SC_CATNIPPOWDER])
		matk -= matk * sc->data[SC_CATNIPPOWDER]->val2 / 100;
	if (sc->data[SC_CHATTERING])
		matk += sc->data[SC_CHATTERING]->val2;
	if (sc->data[SC_DORAM_MATK])
		matk += sc->data[SC_DORAM_MATK]->val1;

	return (unsigned short)cap_value(matk,0,USHRT_MAX);
}
#endif

/**
 * Adds magic attack modifications based on status changes
 * @param bl: Object to change matk [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param matk: Initial matk
 * @return modified matk with cap_value(matk,0,USHRT_MAX)
 */
static unsigned short status_calc_matk(struct block_list *bl, struct status_change *sc, int matk)
{
	if(!sc || !sc->count)
		return cap_value(matk,0,USHRT_MAX);
#ifndef RENEWAL
	/// Take note fixed value first before % modifiers [PRE-RENEWAL]
	if (sc->data[SC_MATKPOTION])
		matk += sc->data[SC_MATKPOTION]->val1;
	if (sc->data[SC_MATKFOOD])
		matk += sc->data[SC_MATKFOOD]->val1;
	if (sc->data[SC_MANA_PLUS])
		matk += sc->data[SC_MANA_PLUS]->val1;
	if (sc->data[SC_AQUAPLAY_OPTION])
		matk += sc->data[SC_AQUAPLAY_OPTION]->val2;
	if (sc->data[SC_CHILLY_AIR_OPTION])
		matk += sc->data[SC_CHILLY_AIR_OPTION]->val2;
	if (sc->data[SC_COOLER_OPTION])
		matk += sc->data[SC_COOLER_OPTION]->val2;
	if (sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 3)
		matk += 50;
	if (sc->data[SC_ODINS_POWER])
		matk += 40 + 30 * sc->data[SC_ODINS_POWER]->val1; // 70 lvl1, 100lvl2
	if (sc->data[SC_IZAYOI])
		matk += 25 * sc->data[SC_IZAYOI]->val1;
	if (sc->data[SC_MTF_MATK2])
		matk += sc->data[SC_MTF_MATK2]->val1;
	if (sc->data[SC_2011RWC_SCROLL])
		matk += 30;
#endif
	if (sc->data[SC_ZANGETSU])
		matk += sc->data[SC_ZANGETSU]->val3;
	if (sc->data[SC_QUEST_BUFF1])
		matk += sc->data[SC_QUEST_BUFF1]->val1;
	if (sc->data[SC_QUEST_BUFF2])
		matk += sc->data[SC_QUEST_BUFF2]->val1;
	if (sc->data[SC_QUEST_BUFF3])
		matk += sc->data[SC_QUEST_BUFF3]->val1;
	if (sc->data[SC_MAGICPOWER] && sc->data[SC_MAGICPOWER]->val4)
		matk += matk * sc->data[SC_MAGICPOWER]->val3/100;
	if (sc->data[SC_MINDBREAKER])
		matk += matk * sc->data[SC_MINDBREAKER]->val2/100;
	if (sc->data[SC_INCMATKRATE])
		matk += matk * sc->data[SC_INCMATKRATE]->val1/100;
	if (sc->data[SC_MOONLITSERENADE])
		matk += sc->data[SC_MOONLITSERENADE]->val3/100;
	if (sc->data[SC_MTF_MATK])
		matk += matk * sc->data[SC_MTF_MATK]->val1 / 100;
	if(sc->data[SC_2011RWC_SCROLL])
		matk += 30;
	if (sc->data[SC_SHRIMP])
		matk += matk * sc->data[SC_SHRIMP]->val2 / 100;

	return (unsigned short)cap_value(matk,0,USHRT_MAX);
}

/**
 * Adds critical modifications based on status changes
 * @param bl: Object to change critical [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param critical: Initial critical
 * @return modified critical with cap_value(critical,10,USHRT_MAX)
 */
static signed short status_calc_critical(struct block_list *bl, struct status_change *sc, int critical)
{
	if(!sc || !sc->count)
		return cap_value(critical,10,SHRT_MAX);

	if (sc->data[SC_INCCRI])
		critical += sc->data[SC_INCCRI]->val2;
	if (sc->data[SC_CRIFOOD])
		critical += sc->data[SC_CRIFOOD]->val1;
	if (sc->data[SC_EXPLOSIONSPIRITS])
		critical += sc->data[SC_EXPLOSIONSPIRITS]->val2;
	if (sc->data[SC_FORTUNE])
		critical += sc->data[SC_FORTUNE]->val2;
	if (sc->data[SC_TRUESIGHT])
		critical += sc->data[SC_TRUESIGHT]->val2;
	if (sc->data[SC_CLOAKING])
		critical += critical;
	if (sc->data[SC_STRIKING])
		critical += critical * sc->data[SC_STRIKING]->val1 / 100;
#ifdef RENEWAL
	if (sc->data[SC_SPEARQUICKEN])
		critical += 3*sc->data[SC_SPEARQUICKEN]->val1*10;
#endif
	if (sc->data[SC__INVISIBILITY])
		critical += sc->data[SC__INVISIBILITY]->val3 * 10;
	if (sc->data[SC__UNLUCKY])
		critical -= sc->data[SC__UNLUCKY]->val2;
	if(sc->data[SC_BEYONDOFWARCRY])
		critical += sc->data[SC_BEYONDOFWARCRY]->val3;

	return (short)cap_value(critical,10,SHRT_MAX);
}

/**
 * Adds hit modifications based on status changes
 * @param bl: Object to change hit [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param hit: Initial hit
 * @return modified hit with cap_value(hit,1,USHRT_MAX)
 */
static signed short status_calc_hit(struct block_list *bl, struct status_change *sc, int hit)
{
	if(!sc || !sc->count)
		return cap_value(hit,1,SHRT_MAX);

	if(sc->data[SC_INCHIT])
		hit += sc->data[SC_INCHIT]->val1;
	if(sc->data[SC_HITFOOD])
		hit += sc->data[SC_HITFOOD]->val1;
	if(sc->data[SC_TRUESIGHT])
		hit += sc->data[SC_TRUESIGHT]->val3;
	if(sc->data[SC_HUMMING])
		hit += sc->data[SC_HUMMING]->val2;
	if(sc->data[SC_CONCENTRATION])
		hit += sc->data[SC_CONCENTRATION]->val3;
	if(sc->data[SC_INSPIRATION])
		hit += 5 * sc->data[SC_INSPIRATION]->val1 + sc->data[SC_INSPIRATION]->val2 / 2;
	if(sc->data[SC_ADJUSTMENT])
		hit -= 30;
	if(sc->data[SC_INCREASING])
		hit += 20; // RockmanEXE; changed based on updated [Reddozen]
	if(sc->data[SC_MERC_HITUP])
		hit += sc->data[SC_MERC_HITUP]->val2;
	if(sc->data[SC_MTF_HITFLEE])
		hit += sc->data[SC_MTF_HITFLEE]->val1;
	if(sc->data[SC_INCHITRATE])
		hit += hit * sc->data[SC_INCHITRATE]->val1/100;
	if(sc->data[SC_BLIND])
		hit -= hit * 25/100;
	if(sc->data[SC_HEAT_BARREL])
		hit -= sc->data[SC_HEAT_BARREL]->val4;
	if(sc->data[SC__GROOMY])
		hit -= hit * sc->data[SC__GROOMY]->val3 / 100;
	if(sc->data[SC_FEAR])
		hit -= hit * 20 / 100;
	if (sc->data[SC_ASH])
		hit -= hit * sc->data[SC_ASH]->val2 / 100;
	if (sc->data[SC_TEARGAS])
		hit -= hit * 50 / 100;
	if(sc->data[SC_ILLUSIONDOPING])
		hit -= sc->data[SC_ILLUSIONDOPING]->val2;
	if (sc->data[SC_MTF_ASPD])
		hit += sc->data[SC_MTF_ASPD]->val2;

	return (short)cap_value(hit,1,SHRT_MAX);
}

/**
 * Adds flee modifications based on status changes
 * @param bl: Object to change flee [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param flee: Initial flee
 * @return modified flee with cap_value(flee,1,USHRT_MAX)
 */
static signed short status_calc_flee(struct block_list *bl, struct status_change *sc, int flee)
{
	if( bl->type == BL_PC ) {
		struct map_data *mapdata = map_getmapdata(bl->m);

		if( mapdata_flag_gvg(mapdata) )
			flee -= flee * battle_config.gvg_flee_penalty/100;
		else if( mapdata->flag[MF_BATTLEGROUND] )
			flee -= flee * battle_config.bg_flee_penalty/100;
	}

	if(!sc || !sc->count)
		return cap_value(flee,1,SHRT_MAX);
	if(sc->data[SC_OVERED_BOOST]) //Should be final and unmodifiable by any means
		return sc->data[SC_OVERED_BOOST]->val2;
	if(sc->data[SC_TINDER_BREAKER] || sc->data[SC_TINDER_BREAKER2])
		return 1; // 1 = min flee

	// Fixed value
	if(sc->data[SC_INCFLEE])
		flee += sc->data[SC_INCFLEE]->val1;
	if(sc->data[SC_FLEEFOOD])
		flee += sc->data[SC_FLEEFOOD]->val1;
	if(sc->data[SC_WHISTLE])
		flee += sc->data[SC_WHISTLE]->val2;
	if(sc->data[SC_WINDWALK])
		flee += sc->data[SC_WINDWALK]->val2;
	if(sc->data[SC_VIOLENTGALE])
		flee += sc->data[SC_VIOLENTGALE]->val2;
	if(sc->data[SC_MOON_COMFORT]) // SG skill [Komurka]
		flee += sc->data[SC_MOON_COMFORT]->val2;
	if(sc->data[SC_CLOSECONFINE])
		flee += 10;
	if (sc->data[SC_ANGRIFFS_MODUS])
		flee -= sc->data[SC_ANGRIFFS_MODUS]->val3;
	if(sc->data[SC_ADJUSTMENT])
		flee += 30;
	if(sc->data[SC_SPEED])
		flee += 10 + sc->data[SC_SPEED]->val1 * 10;
	if(sc->data[SC_GATLINGFEVER])
		flee -= sc->data[SC_GATLINGFEVER]->val4;
	if(sc->data[SC_PARTYFLEE])
		flee += sc->data[SC_PARTYFLEE]->val1 * 10;
	if(sc->data[SC_MERC_FLEEUP])
		flee += sc->data[SC_MERC_FLEEUP]->val2;
	if( sc->data[SC_HALLUCINATIONWALK] )
		flee += sc->data[SC_HALLUCINATIONWALK]->val2;
	if(sc->data[SC_MTF_HITFLEE])
		flee += sc->data[SC_MTF_HITFLEE]->val2;
	if( sc->data[SC_WATER_BARRIER] )
		flee -= sc->data[SC_WATER_BARRIER]->val2;
	if( sc->data[SC_C_MARKER] )
		flee -= sc->data[SC_C_MARKER]->val3;
#ifdef RENEWAL
	if( sc->data[SC_SPEARQUICKEN] )
		flee += 2 * sc->data[SC_SPEARQUICKEN]->val1;
#endif

	// Rate value
	if(sc->data[SC_INCFLEERATE])
		flee += flee * sc->data[SC_INCFLEERATE]->val1/100;
	if(sc->data[SC_SPIDERWEB])
		flee -= flee * 50/100;
	if(sc->data[SC_BERSERK])
		flee -= flee * 50/100;
	if(sc->data[SC_BLIND])
		flee -= flee * 25/100;
	if(sc->data[SC_FEAR])
		flee -= flee * 20 / 100;
	if(sc->data[SC_PARALYSE])
		flee -= flee * 10 / 100;
	if(sc->data[SC_INFRAREDSCAN])
		flee -= flee * 30 / 100;
	if( sc->data[SC__LAZINESS] )
		flee -= flee * sc->data[SC__LAZINESS]->val3 / 100;
	if( sc->data[SC_GLOOMYDAY] )
		flee -= flee * sc->data[SC_GLOOMYDAY]->val2 / 100;
	if( sc->data[SC_SATURDAYNIGHTFEVER] )
		flee -= flee * (40 + 10 * sc->data[SC_SATURDAYNIGHTFEVER]->val1) / 100;
	if( sc->data[SC_WIND_STEP_OPTION] )
		flee += flee * sc->data[SC_WIND_STEP_OPTION]->val2 / 100;
	if( sc->data[SC_ZEPHYR] )
		flee += sc->data[SC_ZEPHYR]->val2;
	if(sc->data[SC_ASH])
		flee -= flee * sc->data[SC_ASH]->val4 / 100;
	if (sc->data[SC_GOLDENE_FERSE])
		flee += flee * sc->data[SC_GOLDENE_FERSE]->val2 / 100;
	if (sc->data[SC_SMOKEPOWDER])
		flee += flee * 20 / 100;
	if (sc->data[SC_TEARGAS])
		flee -= flee * 50 / 100;
	//if( sc->data[SC_C_MARKER] )
	//	flee -= (flee * sc->data[SC_C_MARKER]->val3) / 100;
	if (sc->data[SC_GROOMING])
		flee += sc->data[SC_GROOMING]->val2;

	return (short)cap_value(flee,1,SHRT_MAX);
}

/**
 * Adds perfect flee modifications based on status changes
 * @param bl: Object to change flee2 [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param flee2: Initial flee2
 * @return modified flee2 with cap_value(flee2,10,USHRT_MAX)
 */
static signed short status_calc_flee2(struct block_list *bl, struct status_change *sc, int flee2)
{
	if(!sc || !sc->count)
		return cap_value(flee2,10,SHRT_MAX);

	if(sc->data[SC_INCFLEE2])
		flee2 += sc->data[SC_INCFLEE2]->val2;
	if(sc->data[SC_WHISTLE])
		flee2 += sc->data[SC_WHISTLE]->val3*10;
	if(sc->data[SC__UNLUCKY])
		flee2 -= flee2 * sc->data[SC__UNLUCKY]->val2 / 100;
	if (sc->data[SC_HISS])
		flee2 += sc->data[SC_HISS]->val2;
	if (sc->data[SC_DORAM_FLEE2])
		flee2 += sc->data[SC_DORAM_FLEE2]->val1;

	return (short)cap_value(flee2,10,SHRT_MAX);
}

/**
 * Adds defense (left-side) modifications based on status changes
 * @param bl: Object to change def [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param def: Initial def
 * @return modified def with cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX)
 */
static defType status_calc_def(struct block_list *bl, struct status_change *sc, int def)
{
	if(!sc || !sc->count)
		return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);

	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_BARRIER])
		return 100;
	if(sc->data[SC_KEEPING])
		return 90;
#ifndef RENEWAL /// Steel Body does not provide 90 DEF in [RENEWAL]
	if(sc->data[SC_STEELBODY])
		return 90;
#endif
	if (sc->data[SC_NYANGGRASS]) {
		if (bl->type == BL_PC)
			return 0;
		else
			return def >>= 1;
	}
	if(sc->data[SC_DEFSET])
		return sc->data[SC_DEFSET]->val1;

	if(sc->data[SC_DRUMBATTLE])
		def += sc->data[SC_DRUMBATTLE]->val3;
#ifndef RENEWAL
	if(sc->data[SC_DEFENCE])
		def += sc->data[SC_DEFENCE]->val2;
#endif
	if(sc->data[SC_INCDEFRATE])
		def += def * sc->data[SC_INCDEFRATE]->val1/100;
	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2)
		def += 50;
	if(sc->data[SC_ODINS_POWER])
		def -= 20 * sc->data[SC_ODINS_POWER]->val1;
	if( sc->data[SC_ANGRIFFS_MODUS] )
		def -= 30 + 20 * sc->data[SC_ANGRIFFS_MODUS]->val1;
	if(sc->data[SC_STONEHARDSKIN])
		def += sc->data[SC_STONEHARDSKIN]->val1;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		def >>=1;
	if(sc->data[SC_FREEZE])
		def >>=1;
	if(sc->data[SC_SIGNUMCRUCIS])
		def -= def * sc->data[SC_SIGNUMCRUCIS]->val2/100;
	if(sc->data[SC_CONCENTRATION])
		def -= def * sc->data[SC_CONCENTRATION]->val4/100;
	if(sc->data[SC_SKE])
		def >>=1;
	if(sc->data[SC_PROVOKE] && bl->type != BL_PC) // Provoke doesn't alter player defense->
		def -= def * sc->data[SC_PROVOKE]->val4/100;
	if(sc->data[SC_STRIPSHIELD] && bl->type != BL_PC) // Player doesn't have def reduction only equip removed
		def -= def * sc->data[SC_STRIPSHIELD]->val2/100;
	if (sc->data[SC_FLING])
		def -= def * (sc->data[SC_FLING]->val2)/100;
	if( sc->data[SC_FREEZING] )
		def -= def * (bl->type == BL_PC ? 30 : 10) / 100;
	if( sc->data[SC_ANALYZE] )
		def -= def * (14 * sc->data[SC_ANALYZE]->val1) / 100;
	if( sc->data[SC_NEUTRALBARRIER] )
		def += def * sc->data[SC_NEUTRALBARRIER]->val2 / 100;
	if( sc->data[SC_SHIELDSPELL_REF] && sc->data[SC_SHIELDSPELL_REF]->val1 == 2 )
		def += sc->data[SC_SHIELDSPELL_REF]->val2;
	if( sc->data[SC_PRESTIGE] )
		def += sc->data[SC_PRESTIGE]->val3;
	if( sc->data[SC_BANDING] && sc->data[SC_BANDING]->val2 > 1 )
		def += (5 + sc->data[SC_BANDING]->val1) * sc->data[SC_BANDING]->val2 / 10;
	if( sc->data[SC_ECHOSONG] )
		def += def * sc->data[SC_ECHOSONG]->val3 / 100;
	if( sc->data[SC_SATURDAYNIGHTFEVER] )
		def -= def * (10 + 10 * sc->data[SC_SATURDAYNIGHTFEVER]->val1) / 100;
	if( sc->data[SC_EARTHDRIVE] )
		def -= def * 25 / 100;
	if( sc->data[SC_CAMOUFLAGE] )
		def -= def * 5 * sc->data[SC_CAMOUFLAGE]->val3 / 100;
	if( sc->data[SC_SOLID_SKIN_OPTION] )
		def += def * sc->data[SC_SOLID_SKIN_OPTION]->val2 / 100;
	if( sc->data[SC_ROCK_CRUSHER] )
		def -= def * sc->data[SC_ROCK_CRUSHER]->val2 / 100;
	if( sc->data[SC_POWER_OF_GAIA] )
		def += def * sc->data[SC_POWER_OF_GAIA]->val2 / 100;
	if(sc->data[SC_ASH])
		def -= def * sc->data[SC_ASH]->val3/100;
	if( sc->data[SC_OVERED_BOOST] && bl->type == BL_HOM )
		def -= def * sc->data[SC_OVERED_BOOST]->val4 / 100;
	if(sc->data[SC_GLASTHEIM_ITEMDEF])
		def += sc->data[SC_GLASTHEIM_ITEMDEF]->val1;

	return (defType)cap_value(def,DEFTYPE_MIN,DEFTYPE_MAX);
}

/**
 * Adds defense (right-side) modifications based on status changes
 * @param bl: Object to change def2 [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param def2: Initial def2
 * @return modified def2 with cap_value(def2,SHRT_MIN,SHRT_MAX)
 */
static signed short status_calc_def2(struct block_list *bl, struct status_change *sc, int def2)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(def2,1,SHRT_MAX);
#endif

	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_ETERNALCHAOS])
		return 0;
	if(sc->data[SC_DEFSET])
		return sc->data[SC_DEFSET]->val1;

	if(sc->data[SC_SUN_COMFORT])
		def2 += sc->data[SC_SUN_COMFORT]->val2;
#ifdef RENEWAL
	if (sc->data[SC_SKA])
		def2 += 80;
#endif
	if(sc->data[SC_ANGELUS])
#ifdef RENEWAL /// The VIT stat bonus is boosted by angelus [RENEWAL]
		def2 += status_get_vit(bl) / 2 * sc->data[SC_ANGELUS]->val2/100;
#else
		def2 += def2 * sc->data[SC_ANGELUS]->val2/100;
	if(sc->data[SC_CONCENTRATION])
		def2 -= def2 * sc->data[SC_CONCENTRATION]->val4/100;
#endif
	if(sc->data[SC_POISON])
		def2 -= def2 * 25/100;
	if(sc->data[SC_DPOISON])
		def2 -= def2 * 25/100;
	if(sc->data[SC_SKE])
		def2 -= def2 * 50/100;
	if(sc->data[SC_PROVOKE])
		def2 -= def2 * sc->data[SC_PROVOKE]->val4/100;
	if(sc->data[SC_JOINTBEAT])
		def2 -= def2 * ( sc->data[SC_JOINTBEAT]->val2&BREAK_SHOULDER ? 50 : 0 ) / 100
			  + def2 * ( sc->data[SC_JOINTBEAT]->val2&BREAK_WAIST ? 25 : 0 ) / 100;
	if(sc->data[SC_FLING])
		def2 -= def2 * (sc->data[SC_FLING]->val3)/100;
	if(sc->data[SC_ANALYZE])
		def2 -= def2 * (14 * sc->data[SC_ANALYZE]->val1) / 100;
	if(sc->data[SC_ASH])
		def2 -= def2 * sc->data[SC_ASH]->val3/100;
	if (sc->data[SC_PARALYSIS])
		def2 -= def2 * sc->data[SC_PARALYSIS]->val2 / 100;
	if(sc->data[SC_EQC])
		def2 -= def2 * sc->data[SC_EQC]->val2 / 100;
	if( sc->data[SC_CAMOUFLAGE] )
		def2 -= def2 * 5 * sc->data[SC_CAMOUFLAGE]->val3 / 100;

#ifdef RENEWAL
	return (short)cap_value(def2,SHRT_MIN,SHRT_MAX);
#else
	return (short)cap_value(def2,1,SHRT_MAX);
#endif
}

/**
 * Adds magic defense (left-side) modifications based on status changes
 * @param bl: Object to change mdef [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param mdef: Initial mdef
 * @return modified mdef with cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX)
 */
static defType status_calc_mdef(struct block_list *bl, struct status_change *sc, int mdef)
{
	if(!sc || !sc->count)
		return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);

	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_BARRIER])
		return 100;

#ifndef RENEWAL /// Steel Body does not provide 90 MDEF in [RENEWAL]
	if(sc->data[SC_STEELBODY])
		return 90;
#endif
	if (sc->data[SC_NYANGGRASS]) {
		if (bl->type == BL_PC)
			return 0;
		else
			return mdef >>= 1;
	}
	if(sc->data[SC_MDEFSET])
		return sc->data[SC_MDEFSET]->val1;

	if(sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 3)
		mdef += 50;
	if(sc->data[SC_ENDURE]) // It has been confirmed that Eddga card grants 1 MDEF, not 0, not 10, but 1.
		mdef += (sc->data[SC_ENDURE]->val4 == 0) ? sc->data[SC_ENDURE]->val1 : 1;
	if(sc->data[SC_STONEHARDSKIN])
		mdef += sc->data[SC_STONEHARDSKIN]->val1;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		mdef += 25 * mdef / 100;
	if(sc->data[SC_FREEZE])
		mdef += 25 * mdef / 100;
	if(sc->data[SC_BURNING])
		mdef -= 25 * mdef / 100;
	if( sc->data[SC_NEUTRALBARRIER] )
		mdef += mdef * sc->data[SC_NEUTRALBARRIER]->val2 / 100;
	if(sc->data[SC_ANALYZE])
		mdef -= mdef * ( 14 * sc->data[SC_ANALYZE]->val1 ) / 100;
	if(sc->data[SC_SYMPHONYOFLOVER])
		mdef += mdef * sc->data[SC_SYMPHONYOFLOVER]->val3 / 100;
	if(sc->data[SC_GT_CHANGE]) {
		mdef -= sc->data[SC_GT_CHANGE]->val4;
		if (mdef < 0)
			return 0;
	}
	if (sc->data[SC_ODINS_POWER])
		mdef -= 20 * sc->data[SC_ODINS_POWER]->val1;
	if(sc->data[SC_GLASTHEIM_ITEMDEF])
		mdef += sc->data[SC_GLASTHEIM_ITEMDEF]->val2;

	return (defType)cap_value(mdef,DEFTYPE_MIN,DEFTYPE_MAX);
}

/**
 * Adds magic defense (right-side) modifications based on status changes
 * @param bl: Object to change mdef2 [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param mdef2: Initial mdef2
 * @return modified mdef2 with cap_value(mdef2,SHRT_MIN,SHRT_MAX)
 */
static signed short status_calc_mdef2(struct block_list *bl, struct status_change *sc, int mdef2)
{
	if(!sc || !sc->count)
#ifdef RENEWAL
		return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
		return (short)cap_value(mdef2,1,SHRT_MAX);
#endif

	if(sc->data[SC_BERSERK])
		return 0;
	if(sc->data[SC_SKA])
		return 90;
	if(sc->data[SC_MDEFSET])
		return sc->data[SC_MDEFSET]->val1;

	if(sc->data[SC_MINDBREAKER])
		mdef2 -= mdef2 * sc->data[SC_MINDBREAKER]->val3/100;
	if(sc->data[SC_BURNING])
		mdef2 -= mdef2 * 25 / 100;
	if(sc->data[SC_ANALYZE])
		mdef2 -= mdef2 * (14 * sc->data[SC_ANALYZE]->val1) / 100;

#ifdef RENEWAL
	return (short)cap_value(mdef2,SHRT_MIN,SHRT_MAX);
#else
	return (short)cap_value(mdef2,1,SHRT_MAX);
#endif
}

/**
 * Adds speed modifications based on status changes
 * @param bl: Object to change speed [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param speed: Initial speed
 * @return modified speed with cap_value(speed,10,USHRT_MAX)
 */
static unsigned short status_calc_speed(struct block_list *bl, struct status_change *sc, int speed)
{
	TBL_PC* sd = BL_CAST(BL_PC, bl);
	int speed_rate = 100;

	if (sc == NULL || (sd && sd->state.permanent_speed))
		return (unsigned short)cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);

	if (sd && pc_ismadogear(sd)) { // Mado speed is not affected by other statuses
		int val = 0;

		if (pc_checkskill(sd, NC_MADOLICENCE) < 5)
			val = 50 - 10 * pc_checkskill(sd, NC_MADOLICENCE);
		else
			val -= 25;
		if (sc->data[SC_ACCELERATION])
			val -= 25;
		speed += speed * val / 100;

		return (unsigned short)cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
	}

	if( sd && sd->ud.skilltimer != INVALID_TIMER && (pc_checkskill(sd,SA_FREECAST) > 0 || sd->ud.skill_id == LG_EXEEDBREAK) ) {
		if( sd->ud.skill_id == LG_EXEEDBREAK )
			speed_rate = 160 - 10 * sd->ud.skill_lv;
		else
			speed_rate = 175 - 5 * pc_checkskill(sd,SA_FREECAST);
	} else {
		int val = 0;

		// GetMoveHasteValue2()
		if( sc->data[SC_FUSION] )
			val = 25;
		else if( sd ) {
			if( pc_isriding(sd) || sd->sc.option&OPTION_DRAGON )
				val = 25; // Same bonus
			else if( pc_isridingwug(sd) )
				val = 15 + 5 * pc_checkskill(sd, RA_WUGRIDER);
			else if( sc->data[SC_ALL_RIDING] )
				val = battle_config.rental_mount_speed_boost;
		}
		speed_rate -= val;

		// GetMoveSlowValue()
		if( sd && sc->data[SC_HIDING] && pc_checkskill(sd,RG_TUNNELDRIVE) > 0 )
			val = 120 - 6 * pc_checkskill(sd,RG_TUNNELDRIVE);
		else if( sd && sc->data[SC_CHASEWALK] && sc->data[SC_CHASEWALK]->val3 < 0 )
			val = sc->data[SC_CHASEWALK]->val3;
		else {
			val = 0;
			// Longing for Freedom cancels song/dance penalty
			if( sc->data[SC_LONGING] )
				val = max( val, 50 - 10 * sc->data[SC_LONGING]->val1 );
			else
			if( sd && sc->data[SC_DANCING] )
				val = max( val, 500 - (40 + 10 * (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_BARDDANCER)) * pc_checkskill(sd,(sd->status.sex?BA_MUSICALLESSON:DC_DANCINGLESSON)) );

			if( sc->data[SC_DECREASEAGI] )
				val = max( val, 25 );
			if( sc->data[SC_QUAGMIRE] || sc->data[SC_HALLUCINATIONWALK_POSTDELAY] || (sc->data[SC_GLOOMYDAY] && sc->data[SC_GLOOMYDAY]->val4) )
				val = max( val, 50 );
			if( sc->data[SC_DONTFORGETME] )
				val = max( val, sc->data[SC_DONTFORGETME]->val3 );
			if( sc->data[SC_CURSE] )
				val = max( val, 300 );
			if( sc->data[SC_CHASEWALK] )
				val = max( val, sc->data[SC_CHASEWALK]->val3 );
			if( sc->data[SC_WEDDING] )
				val = max( val, 100 );
			if( sc->data[SC_JOINTBEAT] && sc->data[SC_JOINTBEAT]->val2&(BREAK_ANKLE|BREAK_KNEE) )
				val = max( val, (sc->data[SC_JOINTBEAT]->val2&BREAK_ANKLE ? 50 : 0) + (sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE ? 30 : 0) );
			if( sc->data[SC_CLOAKING] && (sc->data[SC_CLOAKING]->val4&1) == 0 )
				val = max( val, sc->data[SC_CLOAKING]->val1 < 3 ? 300 : 30 - 3 * sc->data[SC_CLOAKING]->val1 );
			if( sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY )
				val = max( val, 75 );
			if( sc->data[SC_SLOWDOWN] ) // Slow Potion
				val = max( val, sc->data[SC_SLOWDOWN]->val1 );
			if( sc->data[SC_GATLINGFEVER] )
				val = max( val, 100 );
			if( sc->data[SC_SUITON] )
				val = max( val, sc->data[SC_SUITON]->val3 );
			if( sc->data[SC_SWOO] )
				val = max( val, 300 );
			if( sc->data[SC_SKA] )
				val = max( val, 25 );
			if( sc->data[SC_FREEZING] )
				val = max( val, 30 );
			if( sc->data[SC_MARSHOFABYSS] )
				val = max( val, sc->data[SC_MARSHOFABYSS]->val3 );
			if( sc->data[SC_CAMOUFLAGE] && sc->data[SC_CAMOUFLAGE]->val1 > 2 )
				val = max( val, 25 * (5 - sc->data[SC_CAMOUFLAGE]->val1) );
			if( sc->data[SC_STEALTHFIELD] )
				val = max( val, 20 );
			if( sc->data[SC__LAZINESS] )
				val = max( val, 25 );
			if( sc->data[SC_BANDING_DEFENCE] )
				val = max( val, sc->data[SC_BANDING_DEFENCE]->val1 ); // +90% walking speed.
			if( sc->data[SC_ROCK_CRUSHER_ATK] )
				val = max( val, sc->data[SC_ROCK_CRUSHER_ATK]->val2 );
			if( sc->data[SC_POWER_OF_GAIA] )
				val = max( val, sc->data[SC_POWER_OF_GAIA]->val2 );
			if( sc->data[SC_MELON_BOMB] )
				val = max( val, sc->data[SC_MELON_BOMB]->val2 );
			if( sc->data[SC_REBOUND] )
				val = max( val, 25 );
			if( sc->data[SC_B_TRAP] )
				val = max( val, sc->data[SC_B_TRAP]->val3 );
			if (sc->data[SC_CATNIPPOWDER])
				val = max(val, sc->data[SC_CATNIPPOWDER]->val3);

			if( sd && sd->bonus.speed_rate + sd->bonus.speed_add_rate > 0 ) // Permanent item-based speedup
				val = max( val, sd->bonus.speed_rate + sd->bonus.speed_add_rate );
		}
		speed_rate += val;
		val = 0;

		if( sc->data[SC_MARSHOFABYSS] && speed_rate > 150 )
			speed_rate = 150;

		// GetMoveHasteValue1()
		if( sc->data[SC_SPEEDUP1] ) // !FIXME: used both by NPC_AGIUP and Speed Potion script
			val = max( val, sc->data[SC_SPEEDUP1]->val1 );
		if( sc->data[SC_INCREASEAGI] )
			val = max( val, 25 );
		if( sc->data[SC_WINDWALK] )
			val = max( val, 2 * sc->data[SC_WINDWALK]->val1 );
		if( sc->data[SC_CARTBOOST] )
			val = max( val, 20 );
		if( sd && (sd->class_&MAPID_UPPERMASK) == MAPID_ASSASSIN && pc_checkskill(sd,TF_MISS) > 0 )
			val = max( val, 1 * pc_checkskill(sd,TF_MISS) );
		if( sc->data[SC_CLOAKING] && (sc->data[SC_CLOAKING]->val4&1) == 1 )
			val = max( val, sc->data[SC_CLOAKING]->val1 >= 10 ? 25 : 3 * sc->data[SC_CLOAKING]->val1 - 3 );
		if( sc->data[SC_BERSERK] )
			val = max( val, 25 );
		if( sc->data[SC_RUN] )
			val = max( val, 55 );
		if( sc->data[SC_AVOID] )
			val = max( val, 10 * sc->data[SC_AVOID]->val1 );
		if( sc->data[SC_INVINCIBLE] && !sc->data[SC_INVINCIBLEOFF] )
			val = max( val, 75 );
		if( sc->data[SC_CLOAKINGEXCEED] )
			val = max( val, sc->data[SC_CLOAKINGEXCEED]->val3);
		if( sc->data[SC_HOVERING] )
			val = max( val, 10 );
		if( sc->data[SC_GN_CARTBOOST] )
			val = max( val, sc->data[SC_GN_CARTBOOST]->val2 );
		if( sc->data[SC_SWINGDANCE] )
			val = max( val, sc->data[SC_SWINGDANCE]->val3 );
		if( sc->data[SC_WIND_STEP_OPTION] )
			val = max( val, sc->data[SC_WIND_STEP_OPTION]->val2 );
		if( sc->data[SC_FULL_THROTTLE] )
			val = max( val, 25 );
		if (sc->data[SC_ARCLOUSEDASH])
			val = max(val, sc->data[SC_ARCLOUSEDASH]->val3);
		if( sc->data[SC_DORAM_WALKSPEED] )
			val = max(val, sc->data[SC_DORAM_WALKSPEED]->val1);

		// !FIXME: official items use a single bonus for this [ultramage]
		if( sc->data[SC_SPEEDUP0] ) // Temporary item-based speedup
			val = max( val, sc->data[SC_SPEEDUP0]->val1 );
		if( sd && sd->bonus.speed_rate + sd->bonus.speed_add_rate < 0 ) // Permanent item-based speedup
			val = max( val, -(sd->bonus.speed_rate + sd->bonus.speed_add_rate) );

		speed_rate -= val;

		if( speed_rate < 40 )
			speed_rate = 40;
	}

	// GetSpeed()
	if( sd && pc_iscarton(sd) )
		speed += speed * (50 - 5 * pc_checkskill(sd,MC_PUSHCART)) / 100;
	if( sc->data[SC_PARALYSE] )
		speed += speed * 50 / 100;
	if( speed_rate != 100 )
		speed = speed * speed_rate / 100;
	if( sc->data[SC_STEELBODY] )
		speed = 200;
	if( sc->data[SC_DEFENDER] )
		speed = max(speed, 200);
	if( sc->data[SC_WALKSPEED] && sc->data[SC_WALKSPEED]->val1 > 0 ) // ChangeSpeed
		speed = speed * 100 / sc->data[SC_WALKSPEED]->val1;

	return (unsigned short)cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
}

#ifdef RENEWAL_ASPD
/**
 * Renewal attack speed modifiers based on status changes
 * This function only affects RENEWAL players and comes after base calculation
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param fixed: True - fixed value [malufett]
 *               False - percentage value
 * @return modified aspd
 */
static short status_calc_aspd(struct block_list *bl, struct status_change *sc, bool fixed)
{
	int bonus = 0;

	if (!sc || !sc->count)
		return 0;

	if (fixed) {
		enum sc_type sc_val;

		if (!sc->data[SC_QUAGMIRE]) {
			if (bonus < 7 && (sc->data[SC_TWOHANDQUICKEN] || sc->data[SC_ONEHAND] || sc->data[SC_MERC_QUICKEN] || sc->data[SC_ADRENALINE] || sc->data[SC_SPEARQUICKEN]))
				bonus = 7;
			else if (bonus < 6 && sc->data[SC_ADRENALINE2])
				bonus = 6;
			else if (bonus < 5 && sc->data[SC_FLEET])
				bonus = 5;
		}

		if (sc->data[SC_ASSNCROS] && bonus < sc->data[SC_ASSNCROS]->val2) {
			if (bl->type != BL_PC)
				bonus += sc->data[SC_ASSNCROS]->val2;
			else {
				switch(((TBL_PC*)bl)->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						break;
					default:
						bonus += sc->data[SC_ASSNCROS]->val2;
						break;
				}
			}
		}

		if (bonus < 20 && sc->data[SC_MADNESSCANCEL])
			bonus = 20;
		else if (bonus < 15 && sc->data[SC_BERSERK])
			bonus = 15;

		if (sc->data[sc_val = SC_ASPDPOTION3] || sc->data[sc_val = SC_ASPDPOTION2] || sc->data[sc_val = SC_ASPDPOTION1] || sc->data[sc_val = SC_ASPDPOTION0])
			bonus += sc->data[sc_val]->val1;
		if (sc->data[SC_ATTHASTE_CASH])
			bonus += sc->data[SC_ATTHASTE_CASH]->val1;
		if (sc->data[SC_HEAT_BARREL])
			bonus += sc->data[SC_HEAT_BARREL]->val1;
	} else {
		if (sc->data[SC_DONTFORGETME])
			bonus -= sc->data[SC_DONTFORGETME]->val2 / 10;
		if (sc->data[SC_LONGING])
			bonus -= sc->data[SC_LONGING]->val2 / 10;
		if (sc->data[SC_STEELBODY])
			bonus -= 25;
		if (sc->data[SC_SKA])
			bonus -= 25;
		if (sc->data[SC_DEFENDER])
			bonus -= sc->data[SC_DEFENDER]->val4 / 10;
		if (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY)
			bonus -= 75;
		if (sc->data[SC_GRAVITATION])
			bonus -= sc->data[SC_GRAVITATION]->val2 / 10; // Needs more info
		if (sc->data[SC_JOINTBEAT]) { // Needs more info
			if (sc->data[SC_JOINTBEAT]->val2&BREAK_WRIST)
				bonus -= 25;
			if (sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE)
				bonus -= 10;
		}
		if (sc->data[SC_FREEZING])
			bonus -= 30;
		if (sc->data[SC_HALLUCINATIONWALK_POSTDELAY])
			bonus -= 50;
		if (sc->data[SC_PARALYSE])
			bonus -= 10;
		if (sc->data[SC__BODYPAINT])
			bonus -= 5 * sc->data[SC__BODYPAINT]->val1;
		if (sc->data[SC__INVISIBILITY])
			bonus -= sc->data[SC__INVISIBILITY]->val2;
		if (sc->data[SC__GROOMY])
			bonus -= sc->data[SC__GROOMY]->val2;
		if (sc->data[SC_SWINGDANCE])
			bonus += sc->data[SC_SWINGDANCE]->val3;
		if (sc->data[SC_DANCEWITHWUG])
			bonus += sc->data[SC_DANCEWITHWUG]->val3;
		if (sc->data[SC_GLOOMYDAY])
			bonus -= sc->data[SC_GLOOMYDAY]->val3;
		if (sc->data[SC_EARTHDRIVE])
			bonus -= 25;
		if (sc->data[SC_GT_CHANGE])
			bonus += sc->data[SC_GT_CHANGE]->val3;
		if (sc->data[SC_MELON_BOMB])
			bonus -= sc->data[SC_MELON_BOMB]->val3;
		if (sc->data[SC_PAIN_KILLER])
			bonus -= sc->data[SC_PAIN_KILLER]->val2;
		if (sc->data[SC_BOOST500])
			bonus += sc->data[SC_BOOST500]->val1;
		if (sc->data[SC_EXTRACT_SALAMINE_JUICE])
			bonus += sc->data[SC_EXTRACT_SALAMINE_JUICE]->val1;
		if (sc->data[SC_GOLDENE_FERSE])
			bonus += sc->data[SC_GOLDENE_FERSE]->val3;
		if (sc->data[SC_INCASPDRATE])
			bonus += sc->data[SC_INCASPDRATE]->val1;
		if (sc->data[SC_GATLINGFEVER])
			bonus += sc->data[SC_GATLINGFEVER]->val1;
		if (sc->data[SC_STAR_COMFORT])
			bonus += 3 * sc->data[SC_STAR_COMFORT]->val1;
		if (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 2)
			bonus += 10;
	}

	return bonus;
}
#endif

/**
 * Modifies ASPD by a number, rather than a percentage (10 = 1 ASPD)
 * A subtraction reduces the delay, meaning an increase in ASPD
 * This comes after the percentage changes and is based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param aspd: Object's current ASPD
 * @return modified aspd
 */
static short status_calc_fix_aspd(struct block_list *bl, struct status_change *sc, int aspd)
{
	if (!sc || !sc->count)
		return cap_value(aspd, 0, 2000);
	if (sc->data[SC_OVERED_BOOST])
		return cap_value(2000 - sc->data[SC_OVERED_BOOST]->val3 * 10, 0, 2000);

	if ((sc->data[SC_GUST_OPTION] || sc->data[SC_BLAST_OPTION] || sc->data[SC_WILD_STORM_OPTION]))
		aspd -= 50; // +5 ASPD
	if (sc->data[SC_FIGHTINGSPIRIT] && sc->data[SC_FIGHTINGSPIRIT]->val2)
		aspd -= sc->data[SC_FIGHTINGSPIRIT]->val2;
	if (sc->data[SC_MTF_ASPD])
		aspd -= sc->data[SC_MTF_ASPD]->val1;
	if (sc->data[SC_MTF_ASPD2])
		aspd -= sc->data[SC_MTF_ASPD2]->val1;

	return cap_value(aspd, 0, 2000); // Will be recap for proper bl anyway
}

/**
 * Calculates an object's ASPD modifier based on status changes (alters amotion value)
 * Note: The scale of aspd_rate is 1000 = 100%
 * Note2: This only affects Homunculus, Mercenaries, and Pre-renewal Players
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param aspd_rate: Object's current ASPD
 * @return modified aspd_rate
 */
static short status_calc_aspd_rate(struct block_list *bl, struct status_change *sc, int aspd_rate)
{
	int i;

	if(!sc || !sc->count)
		return cap_value(aspd_rate,0,SHRT_MAX);

	if( !sc->data[SC_QUAGMIRE] ) {
		int max = 0;
		if(sc->data[SC_STAR_COMFORT])
			max = sc->data[SC_STAR_COMFORT]->val2;

		if(sc->data[SC_TWOHANDQUICKEN] &&
			max < sc->data[SC_TWOHANDQUICKEN]->val2)
			max = sc->data[SC_TWOHANDQUICKEN]->val2;

		if(sc->data[SC_ONEHAND] &&
			max < sc->data[SC_ONEHAND]->val2)
			max = sc->data[SC_ONEHAND]->val2;

		if(sc->data[SC_MERC_QUICKEN] &&
			max < sc->data[SC_MERC_QUICKEN]->val2)
			max = sc->data[SC_MERC_QUICKEN]->val2;

		if(sc->data[SC_ADRENALINE2] &&
			max < sc->data[SC_ADRENALINE2]->val3)
			max = sc->data[SC_ADRENALINE2]->val3;

		if(sc->data[SC_ADRENALINE] &&
			max < sc->data[SC_ADRENALINE]->val3)
			max = sc->data[SC_ADRENALINE]->val3;

		if(sc->data[SC_SPEARQUICKEN] &&
			max < sc->data[SC_SPEARQUICKEN]->val2)
			max = sc->data[SC_SPEARQUICKEN]->val2;

		if(sc->data[SC_GATLINGFEVER] &&
			max < sc->data[SC_GATLINGFEVER]->val2)
			max = sc->data[SC_GATLINGFEVER]->val2;

		if(sc->data[SC_FLEET] &&
			max < sc->data[SC_FLEET]->val2)
			max = sc->data[SC_FLEET]->val2;

		if(sc->data[SC_ASSNCROS] && max < sc->data[SC_ASSNCROS]->val2) {
			if (bl->type!=BL_PC)
				max = sc->data[SC_ASSNCROS]->val2;
			else
				switch(((TBL_PC*)bl)->status.weapon) {
					case W_BOW:
					case W_REVOLVER:
					case W_RIFLE:
					case W_GATLING:
					case W_SHOTGUN:
					case W_GRENADE:
						break;
					default:
						max = sc->data[SC_ASSNCROS]->val2;
			}
		}
		aspd_rate -= max;

		if(sc->data[SC_BERSERK])
			aspd_rate -= 300;
		else if(sc->data[SC_MADNESSCANCEL])
			aspd_rate -= 200;
	}

	if( sc->data[i=SC_ASPDPOTION3] ||
		sc->data[i=SC_ASPDPOTION2] ||
		sc->data[i=SC_ASPDPOTION1] ||
		sc->data[i=SC_ASPDPOTION0] )
		aspd_rate -= sc->data[i]->val2;

	if (sc->data[SC_ATTHASTE_CASH])
		aspd_rate -= sc->data[SC_ATTHASTE_CASH]->val2;

	if(sc->data[SC_DONTFORGETME])
		aspd_rate += sc->data[SC_DONTFORGETME]->val2;
	if(sc->data[SC_LONGING])
		aspd_rate += sc->data[SC_LONGING]->val2;
	if(sc->data[SC_STEELBODY])
		aspd_rate += 250;
	if(sc->data[SC_SKA])
		aspd_rate += 250;
	if(sc->data[SC_DEFENDER])
		aspd_rate += sc->data[SC_DEFENDER]->val4;
	if(sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_ENEMY)
		aspd_rate += 250;
	if(sc->data[SC_GRAVITATION])
		aspd_rate += sc->data[SC_GRAVITATION]->val2;
	if(sc->data[SC_JOINTBEAT]) {
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_WRIST )
			aspd_rate += 250;
		if( sc->data[SC_JOINTBEAT]->val2&BREAK_KNEE )
			aspd_rate += 100;
	}
	if( sc->data[SC_FREEZING] )
		aspd_rate += 300;
	if( sc->data[SC_HALLUCINATIONWALK_POSTDELAY] )
		aspd_rate += 500;
	if( sc->data[SC_PARALYSE] )
		aspd_rate += 100;
	if( sc->data[SC__BODYPAINT] )
		aspd_rate +=  50 * sc->data[SC__BODYPAINT]->val1;
	if( sc->data[SC__INVISIBILITY] )
		aspd_rate += sc->data[SC__INVISIBILITY]->val2 * 10;
	if( sc->data[SC__GROOMY] )
		aspd_rate += sc->data[SC__GROOMY]->val2 * 10;
	if( sc->data[SC_SWINGDANCE] )
		aspd_rate -= sc->data[SC_SWINGDANCE]->val3 * 10;
	if( sc->data[SC_DANCEWITHWUG] )
		aspd_rate -= sc->data[SC_DANCEWITHWUG]->val3 * 10;
	if( sc->data[SC_GLOOMYDAY] )
		aspd_rate += sc->data[SC_GLOOMYDAY]->val3 * 10;
	if( sc->data[SC_EARTHDRIVE] )
		aspd_rate += 250;
	if( sc->data[SC_GT_CHANGE] )
		aspd_rate -= sc->data[SC_GT_CHANGE]->val3 * 10;
	if( sc->data[SC_MELON_BOMB] )
		aspd_rate += sc->data[SC_MELON_BOMB]->val3 * 10;
	if( sc->data[SC_BOOST500] )
		aspd_rate -= sc->data[SC_BOOST500]->val1 *10;
	if( sc->data[SC_EXTRACT_SALAMINE_JUICE] )
		aspd_rate -= sc->data[SC_EXTRACT_SALAMINE_JUICE]->val1 * 10;
	if( sc->data[SC_INCASPDRATE] )
		aspd_rate -= sc->data[SC_INCASPDRATE]->val1 * 10;
	if( sc->data[SC_PAIN_KILLER])
		aspd_rate += sc->data[SC_PAIN_KILLER]->val2 * 10;
	if( sc->data[SC_GOLDENE_FERSE])
		aspd_rate -= sc->data[SC_GOLDENE_FERSE]->val3 * 10;
	if (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 2)
		aspd_rate -= 100;

	return (short)cap_value(aspd_rate,0,SHRT_MAX);
}

/**
 * Modifies the damage delay time based on status changes
 * The lower your delay, the quicker you can act after taking damage
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param dmotion: Object's current damage delay
 * @return modified delay rate
 */
static unsigned short status_calc_dmotion(struct block_list *bl, struct status_change *sc, int dmotion)
{
	if( !sc || !sc->count || map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND) )
		return cap_value(dmotion,0,USHRT_MAX);

	/// It has been confirmed on official servers that MvP mobs have no dmotion even without endure
	if( sc->data[SC_ENDURE] || ( bl->type == BL_MOB && status_get_class_(bl) == CLASS_BOSS ) )
		return 0;
	if( sc->data[SC_RUN] || sc->data[SC_WUGDASH] )
		return 0;

	return (unsigned short)cap_value(dmotion,0,USHRT_MAX);
}

/**
 * Calculates a max HP based on status changes
 * Values can either be percentages or fixed, based on how equations are formulated
 * @param bl: Object's block_list data
 * @param maxhp: Object's current max HP
 * @return modified maxhp
 */
static unsigned int status_calc_maxhp(struct block_list *bl, uint64 maxhp)
{
	int rate = 100;

	maxhp += status_get_hpbonus(bl,STATUS_BONUS_FIX);

	if ((rate += status_get_hpbonus(bl,STATUS_BONUS_RATE)) != 100)
		maxhp = maxhp * rate / 100;

	return (unsigned int)cap_value(maxhp,1,UINT_MAX);
}

/**
 * Calculates a max SP based on status changes
 * Values can either be percentages or fixed, bas ed on how equations are formulated
 * @param bl: Object's block_list data
 * @param maxsp: Object's current max SP
 * @return modified maxsp
 */
static unsigned int status_calc_maxsp(struct block_list *bl, uint64 maxsp)
{
	int rate = 100;

	maxsp += status_get_spbonus(bl,STATUS_BONUS_FIX);
	
	if ((rate += status_get_spbonus(bl,STATUS_BONUS_RATE)) != 100)
		maxsp = maxsp * rate / 100;
	
	return (unsigned int)cap_value(maxsp,1,UINT_MAX);
}

/**
 * Changes a player's element based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param element: Object's current element
 * @return new element
 */
static unsigned char status_calc_element(struct block_list *bl, struct status_change *sc, int element)
{
	if(!sc || !sc->count)
		return cap_value(element, 0, UCHAR_MAX);

	if(sc->data[SC_FREEZE])
		return ELE_WATER;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return ELE_EARTH;
	if(sc->data[SC_BENEDICTIO])
		return ELE_HOLY;
	if(sc->data[SC_CHANGEUNDEAD])
		return ELE_UNDEAD;
	if(sc->data[SC_ELEMENTALCHANGE])
		return sc->data[SC_ELEMENTALCHANGE]->val2;
	if(sc->data[SC_SHAPESHIFT])
		return sc->data[SC_SHAPESHIFT]->val2;

	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

/**
 * Changes a player's element level based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param lv: Object's current element level
 * @return new element level
 */
static unsigned char status_calc_element_lv(struct block_list *bl, struct status_change *sc, int lv)
{
	if(!sc || !sc->count)
		return cap_value(lv, 1, 4);

	if(sc->data[SC_FREEZE])
		return 1;
	if(sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
		return 1;
	if(sc->data[SC_BENEDICTIO])
		return 1;
	if(sc->data[SC_CHANGEUNDEAD])
		return 1;
	if(sc->data[SC_ELEMENTALCHANGE])
		return sc->data[SC_ELEMENTALCHANGE]->val1;
	if(sc->data[SC_SHAPESHIFT])
		return 1;
	if(sc->data[SC__INVISIBILITY])
		return 1;

	return (unsigned char)cap_value(lv,1,4);
}

/**
 * Changes a player's attack element based on status changes
 * @param bl: Object to change aspd [PC|MOB|HOM|MER|ELEM]
 * @param sc: Object's status change information
 * @param element: Object's current attack element
 * @return new attack element
 */
unsigned char status_calc_attack_element(struct block_list *bl, struct status_change *sc, int element)
{
	if(!sc || !sc->count)
		return cap_value(element, 0, UCHAR_MAX);
	if(sc->data[SC_ENCHANTARMS])
		return sc->data[SC_ENCHANTARMS]->val2;
	if(sc->data[SC_WATERWEAPON]
		|| (sc->data[SC_WATER_INSIGNIA] && sc->data[SC_WATER_INSIGNIA]->val1 == 2) )
		return ELE_WATER;
	if(sc->data[SC_EARTHWEAPON]
		|| (sc->data[SC_EARTH_INSIGNIA] && sc->data[SC_EARTH_INSIGNIA]->val1 == 2) )
		return ELE_EARTH;
	if(sc->data[SC_FIREWEAPON]
		|| (sc->data[SC_FIRE_INSIGNIA] && sc->data[SC_FIRE_INSIGNIA]->val1 == 2) )
		return ELE_FIRE;
	if(sc->data[SC_WINDWEAPON]
		|| (sc->data[SC_WIND_INSIGNIA] && sc->data[SC_WIND_INSIGNIA]->val1 == 2) )
		return ELE_WIND;
	if(sc->data[SC_ENCPOISON])
		return ELE_POISON;
	if(sc->data[SC_ASPERSIO])
		return ELE_HOLY;
	if(sc->data[SC_SHADOWWEAPON])
		return ELE_DARK;
	if(sc->data[SC_GHOSTWEAPON] || sc->data[SC__INVISIBILITY])
		return ELE_GHOST;
	if(sc->data[SC_TIDAL_WEAPON_OPTION] || sc->data[SC_TIDAL_WEAPON] )
		return ELE_WATER;
	if(sc->data[SC_PYROCLASTIC])
		return ELE_FIRE;
	return (unsigned char)cap_value(element,0,UCHAR_MAX);
}

/**
 * Changes the mode of an object
 * @param bl: Object whose mode to change [PC|MOB|PET|HOM|NPC]
 * @param sc: Object's status change data
 * @param mode: Original mode
 * @return mode with cap_value(mode, 0, INT_MAX)
 */
static enum e_mode status_calc_mode(struct block_list *bl, struct status_change *sc, enum e_mode mode)
{
	if(!sc || !sc->count)
		return cap_value(mode, MD_NONE, static_cast<e_mode>(INT_MAX));
	if(sc->data[SC_MODECHANGE]) {
		if (sc->data[SC_MODECHANGE]->val2)
			mode = static_cast<e_mode>((mode&~MD_MASK)|sc->data[SC_MODECHANGE]->val2); // Set mode
		if (sc->data[SC_MODECHANGE]->val3)
			mode = static_cast<e_mode>(mode|sc->data[SC_MODECHANGE]->val3); // Add mode
		if (sc->data[SC_MODECHANGE]->val4)
			mode = static_cast<e_mode>(mode&~sc->data[SC_MODECHANGE]->val4); // Del mode
	}
	return cap_value(mode, MD_NONE, static_cast<e_mode>(INT_MAX));
}

/**
 * Changes the mode of a slave mob
 * @param md: Slave mob whose mode to change
 * @param mmd: Master of slave mob
 */
void status_calc_slave_mode(struct mob_data *md, struct mob_data *mmd)
{
	switch (battle_config.slaves_inherit_mode) {
		case 1: //Always aggressive
			if (!status_has_mode(&md->status,MD_AGGRESSIVE))
				sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 0);
			break;
		case 2: //Always passive
			if (status_has_mode(&md->status,MD_AGGRESSIVE))
				sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, 0, 0, MD_AGGRESSIVE, 0);
			break;
		case 4: // Overwrite with slave mode
			sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, MD_CANMOVE|MD_NORANDOM_WALK|MD_CANATTACK, 0, 0, 0);
			break;
		default: //Copy master
			if (status_has_mode(&mmd->status,MD_AGGRESSIVE))
				sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 0);
			else
				sc_start4(NULL, &md->bl, SC_MODECHANGE, 100, 1, 0, 0, MD_AGGRESSIVE, 0);
			break;
	}
}

/**
 * Gets the name of the given bl
 * @param bl: Object whose name to get [PC|MOB|PET|HOM|NPC]
 * @return name or "Unknown" if any other bl->type than noted above
 */
const char* status_get_name(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:	return ((TBL_PC *)bl)->fakename[0] != '\0' ? ((TBL_PC*)bl)->fakename : ((TBL_PC*)bl)->status.name;
		case BL_MOB:	return ((TBL_MOB*)bl)->name;
		case BL_PET:	return ((TBL_PET*)bl)->pet.name;
		case BL_HOM:	return ((TBL_HOM*)bl)->homunculus.name;
		//case BL_MER: // They only have database names which are global, not specific to GID.
		case BL_NPC:	return ((TBL_NPC*)bl)->name;
		//case BL_ELEM: // They only have database names which are global, not specific to GID.
	}
	return "Unknown";
}

/**
 * Gets the class/sprite id of the given bl
 * @param bl: Object whose class to get [PC|MOB|PET|HOM|MER|NPC|ELEM]
 * @return class or 0 if any other bl->type than noted above
 */
int status_get_class(struct block_list *bl)
{
	nullpo_ret(bl);
	switch( bl->type ) {
		case BL_PC:	return ((TBL_PC*)bl)->status.class_;
		case BL_MOB:	return ((TBL_MOB*)bl)->vd->class_; // Class used on all code should be the view class of the mob.
		case BL_PET:	return ((TBL_PET*)bl)->pet.class_;
		case BL_HOM:	return ((TBL_HOM*)bl)->homunculus.class_;
		case BL_MER:	return ((TBL_MER*)bl)->mercenary.class_;
		case BL_NPC:	return ((TBL_NPC*)bl)->class_;
		case BL_ELEM:	return ((TBL_ELEM*)bl)->elemental.class_;
	}
	return 0;
}

/**
 * Gets the base level of the given bl
 * @param bl: Object whose base level to get [PC|MOB|PET|HOM|MER|NPC|ELEM]
 * @return base level or 1 if any other bl->type than noted above
 */
int status_get_lv(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:	return ((TBL_PC*)bl)->status.base_level;
		case BL_MOB:	return ((TBL_MOB*)bl)->level;
		case BL_PET:	return ((TBL_PET*)bl)->pet.level;
		case BL_HOM:	return ((TBL_HOM*)bl)->homunculus.level;
		case BL_MER:	return ((TBL_MER*)bl)->db->lv;
		case BL_ELEM:	return ((TBL_ELEM*)bl)->db->lv;
		case BL_NPC:	return ((TBL_NPC*)bl)->level;
	}
	return 1;
}

/**
 * Gets the regeneration info of the given bl
 * @param bl: Object whose regen info to get [PC|HOM|MER|ELEM]
 * @return regen data or NULL if any other bl->type than noted above
 */
struct regen_data *status_get_regen_data(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:	return &((TBL_PC*)bl)->regen;
		case BL_HOM:	return &((TBL_HOM*)bl)->regen;
		case BL_MER:	return &((TBL_MER*)bl)->regen;
		case BL_ELEM:	return &((TBL_ELEM*)bl)->regen;
		default:
			return NULL;
	}
}

/**
 * Gets the status data of the given bl
 * @param bl: Object whose status to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return status or "dummy_status" if any other bl->type than noted above
 */
struct status_data *status_get_status_data(struct block_list *bl)
{
	nullpo_retr(&dummy_status, bl);

	switch (bl->type) {
		case BL_PC: 	return &((TBL_PC*)bl)->battle_status;
		case BL_MOB:	return &((TBL_MOB*)bl)->status;
		case BL_PET:	return &((TBL_PET*)bl)->status;
		case BL_HOM:	return &((TBL_HOM*)bl)->battle_status;
		case BL_MER:	return &((TBL_MER*)bl)->battle_status;
		case BL_ELEM:	return &((TBL_ELEM*)bl)->battle_status;
		case BL_NPC:	return ((mobdb_checkid(((TBL_NPC*)bl)->class_) == 0) ? &((TBL_NPC*)bl)->status : &dummy_status);
		default:
			return &dummy_status;
	}
}

/**
 * Gets the base status data of the given bl
 * @param bl: Object whose status to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return base_status or NULL if any other bl->type than noted above
 */
struct status_data *status_get_base_status(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:	return &((TBL_PC*)bl)->base_status;
		case BL_MOB:	return ((TBL_MOB*)bl)->base_status ? ((TBL_MOB*)bl)->base_status : &((TBL_MOB*)bl)->db->status;
		case BL_PET:	return &((TBL_PET*)bl)->db->status;
		case BL_HOM:	return &((TBL_HOM*)bl)->base_status;
		case BL_MER:	return &((TBL_MER*)bl)->base_status;
		case BL_ELEM:	return &((TBL_ELEM*)bl)->base_status;
		case BL_NPC:	return ((mobdb_checkid(((TBL_NPC*)bl)->class_) == 0) ? &((TBL_NPC*)bl)->status : NULL);
		default:
			return NULL;
	}
}

/**
 * Gets the defense of the given bl
 * @param bl: Object whose defense to get [PC|MOB|HOM|MER|ELEM]
 * @return defense with cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX)
 */
defType status_get_def(struct block_list *bl)
{
	struct unit_data *ud;
	struct status_data *status = status_get_status_data(bl);
	int def = status?status->def:0;
	ud = unit_bl2ud(bl);
	if (ud && ud->skilltimer != INVALID_TIMER)
		def -= def * skill_get_castdef(ud->skill_id)/100;

	return cap_value(def, DEFTYPE_MIN, DEFTYPE_MAX);
}

/**
 * Gets the walking speed of the given bl
 * @param bl: Object whose speed to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return speed
 */
unsigned short status_get_speed(struct block_list *bl)
{
	if(bl->type==BL_NPC)// Only BL with speed data but no status_data [Skotlex]
		return ((struct npc_data *)bl)->speed;
	return status_get_status_data(bl)->speed;
}

/**
 * Gets the party ID of the given bl
 * @param bl: Object whose party ID to get [PC|MOB|PET|HOM|MER|SKILL|ELEM]
 * @return party ID
 */
int status_get_party_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:
			return ((TBL_PC*)bl)->status.party_id;
		case BL_PET:
			if (((TBL_PET*)bl)->master)
				return ((TBL_PET*)bl)->master->status.party_id;
			break;
		case BL_MOB: {
				struct mob_data *md=(TBL_MOB*)bl;
				if( md->master_id > 0 ) {
					struct map_session_data *msd;
					if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
						return msd->status.party_id;
					return -md->master_id;
				}
			}
			break;
		case BL_HOM:
			if (((TBL_HOM*)bl)->master)
				return ((TBL_HOM*)bl)->master->status.party_id;
			break;
		case BL_MER:
			if (((TBL_MER*)bl)->master)
				return ((TBL_MER*)bl)->master->status.party_id;
			break;
		case BL_SKILL:
			if (((TBL_SKILL*)bl)->group)
				return ((TBL_SKILL*)bl)->group->party_id;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->status.party_id;
			break;
	}
	return 0;
}

/**
 * Gets the guild ID of the given bl
 * @param bl: Object whose guild ID to get [PC|MOB|PET|HOM|MER|SKILL|ELEM|NPC]
 * @return guild ID
 */
int status_get_guild_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:
			return ((TBL_PC*)bl)->status.guild_id;
		case BL_PET:
			if (((TBL_PET*)bl)->master)
				return ((TBL_PET*)bl)->master->status.guild_id;
			break;
		case BL_MOB:
			{
				struct map_session_data *msd;
				struct mob_data *md = (struct mob_data *)bl;
				if (md->guardian_data)	// Guardian's guild [Skotlex]
					return md->guardian_data->guild_id;
				if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
					return msd->status.guild_id; // Alchemist's mobs [Skotlex]
			}
			break;
		case BL_HOM:
			if (((TBL_HOM*)bl)->master)
				return ((TBL_HOM*)bl)->master->status.guild_id;
			break;
		case BL_MER:
			if (((TBL_MER*)bl)->master)
				return ((TBL_MER*)bl)->master->status.guild_id;
			break;
		case BL_NPC:
			if (((TBL_NPC*)bl)->subtype == NPCTYPE_SCRIPT)
				return ((TBL_NPC*)bl)->u.scr.guild_id;
			break;
		case BL_SKILL:
			if (((TBL_SKILL*)bl)->group)
				return ((TBL_SKILL*)bl)->group->guild_id;
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->status.guild_id;
			break;
	}
	return 0;
}

/**
 * Gets the guild emblem ID of the given bl
 * @param bl: Object whose emblem ID to get [PC|MOB|PET|HOM|MER|SKILL|ELEM|NPC]
 * @return guild emblem ID
 */
int status_get_emblem_id(struct block_list *bl)
{
	nullpo_ret(bl);
	switch (bl->type) {
		case BL_PC:
			return ((TBL_PC*)bl)->guild_emblem_id;
		case BL_PET:
			if (((TBL_PET*)bl)->master)
				return ((TBL_PET*)bl)->master->guild_emblem_id;
			break;
		case BL_MOB:
			{
				struct map_session_data *msd;
				struct mob_data *md = (struct mob_data *)bl;
				if (md->guardian_data)	// Guardian's guild [Skotlex]
					return md->guardian_data->emblem_id;
				if (md->special_state.ai && (msd = map_id2sd(md->master_id)) != NULL)
					return msd->guild_emblem_id; // Alchemist's mobs [Skotlex]
			}
			break;
		case BL_HOM:
			if (((TBL_HOM*)bl)->master)
				return ((TBL_HOM*)bl)->master->guild_emblem_id;
			break;
		case BL_MER:
			if (((TBL_MER*)bl)->master)
				return ((TBL_MER*)bl)->master->guild_emblem_id;
			break;
		case BL_NPC:
			if (((TBL_NPC*)bl)->subtype == NPCTYPE_SCRIPT && ((TBL_NPC*)bl)->u.scr.guild_id > 0) {
				struct guild *g = guild_search(((TBL_NPC*)bl)->u.scr.guild_id);
				if (g)
					return g->emblem_id;
			}
			break;
		case BL_ELEM:
			if (((TBL_ELEM*)bl)->master)
				return ((TBL_ELEM*)bl)->master->guild_emblem_id;
			break;
	}
	return 0;
}

/**
 * Gets the race2 of a mob or pet
 * @param bl: Object whose race2 to get [MOB|PET]
 * @return race2
 */
enum e_race2 status_get_race2(struct block_list *bl)
{
	nullpo_retr(RC2_NONE,bl);

	if (bl->type == BL_MOB)
		return ((struct mob_data *)bl)->db->race2;
	if (bl->type == BL_PET)
		return ((struct pet_data *)bl)->db->race2;
	return RC2_NONE;
}

/**
 * Checks if an object is dead 
 * @param bl: Object to check [PC|MOB|HOM|MER|ELEM]
 * @return 1: Is dead or 0: Is alive
 */
int status_isdead(struct block_list *bl)
{
	nullpo_ret(bl);
	return status_get_status_data(bl)->hp == 0;
}

/**
 * Checks if an object is immune to magic 
 * @param bl: Object to check [PC|MOB|HOM|MER|ELEM]
 * @return value of magic damage to be blocked
 */
int status_isimmune(struct block_list *bl)
{
	struct status_change *sc =status_get_sc(bl);
	if (sc && sc->data[SC_HERMODE])
		return 100;

	if (bl->type == BL_PC &&
		((TBL_PC*)bl)->special_state.no_magic_damage >= battle_config.gtb_sc_immunity)
		return ((TBL_PC*)bl)->special_state.no_magic_damage;
	return 0;
}

/**
 * Get view data of an object 
 * @param bl: Object whose view data to get [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @return view data structure bl->vd
 */
struct view_data* status_get_viewdata(struct block_list *bl)
{
	nullpo_retr(NULL, bl);
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->vd;
		case BL_MOB: return ((TBL_MOB*)bl)->vd;
		case BL_PET: return &((TBL_PET*)bl)->vd;
		case BL_NPC: return ((TBL_NPC*)bl)->vd;
		case BL_HOM: return ((TBL_HOM*)bl)->vd;
		case BL_MER: return ((TBL_MER*)bl)->vd;
		case BL_ELEM: return ((TBL_ELEM*)bl)->vd;
	}
	return NULL;
}

/**
 * Set view data of an object
 * This function deals with class, mount, and item views
 * SC views are set in clif_getareachar_unit() 
 * @param bl: Object whose view data to set [PC|MOB|PET|HOM|MER|ELEM|NPC]
 * @param class_: class of the object
 */
void status_set_viewdata(struct block_list *bl, int class_)
{
	struct view_data* vd;
	nullpo_retv(bl);
	if (mobdb_checkid(class_) || mob_is_clone(class_))
		vd = mob_get_viewdata(class_);
	else if (npcdb_checkid(class_))
		vd = npc_get_viewdata(class_);
	else if (homdb_checkid(class_))
		vd = hom_get_viewdata(class_);
	else if (mercenary_db(class_))
		vd = mercenary_get_viewdata(class_);
	else if (elemental_class(class_))
		vd = elemental_get_viewdata(class_);
	else
		vd = NULL;

	switch (bl->type) {
	case BL_PC:
		{
			TBL_PC* sd = (TBL_PC*)bl;
			if (pcdb_checkid(class_)) {
				if (sd->sc.option&OPTION_RIDING) {
					switch (class_) { // Adapt class to a Mounted one.
						case JOB_KNIGHT:
							class_ = JOB_KNIGHT2;
							break;
						case JOB_CRUSADER:
							class_ = JOB_CRUSADER2;
							break;
						case JOB_LORD_KNIGHT:
							class_ = JOB_LORD_KNIGHT2;
							break;
						case JOB_PALADIN:
							class_ = JOB_PALADIN2;
							break;
						case JOB_BABY_KNIGHT:
							class_ = JOB_BABY_KNIGHT2;
							break;
						case JOB_BABY_CRUSADER:
							class_ = JOB_BABY_CRUSADER2;
							break;
					}
				}
				sd->vd.class_ = class_;
				clif_get_weapon_view(sd, &sd->vd.weapon, &sd->vd.shield);
				sd->vd.head_top = sd->status.head_top;
				sd->vd.head_mid = sd->status.head_mid;
				sd->vd.head_bottom = sd->status.head_bottom;
				sd->vd.hair_style = cap_value(sd->status.hair,0,battle_config.max_hair_style);
				sd->vd.hair_color = cap_value(sd->status.hair_color,0,battle_config.max_hair_color);
				sd->vd.cloth_color = cap_value(sd->status.clothes_color,0,battle_config.max_cloth_color);
				sd->vd.body_style = cap_value(sd->status.body,0,battle_config.max_body_style);
				sd->vd.sex = sd->status.sex;

				if (sd->vd.cloth_color) {
					if(sd->sc.option&OPTION_WEDDING && battle_config.wedding_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&OPTION_XMAS && battle_config.xmas_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&(OPTION_SUMMER|OPTION_SUMMER2) && battle_config.summer_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&OPTION_HANBOK && battle_config.hanbok_ignorepalette)
						sd->vd.cloth_color = 0;
					if(sd->sc.option&OPTION_OKTOBERFEST && battle_config.oktoberfest_ignorepalette)
						sd->vd.cloth_color = 0;
				}
				if ( sd->vd.body_style && sd->sc.option&OPTION_COSTUME)
 					sd->vd.body_style = 0;
			} else if (vd)
				memcpy(&sd->vd, vd, sizeof(struct view_data));
			else
				ShowError("status_set_viewdata (PC): No view data for class %d\n", class_);
		}
	break;
	case BL_MOB:
		{
			TBL_MOB* md = (TBL_MOB*)bl;
			if (vd){
				mob_free_dynamic_viewdata( md );

				md->vd = vd;
			}else if( pcdb_checkid( class_ ) ){
				mob_set_dynamic_viewdata( md );

				md->vd->class_ = class_;
			}else
				ShowError("status_set_viewdata (MOB): No view data for class %d\n", class_);
		}
	break;
	case BL_PET:
		{
			TBL_PET* pd = (TBL_PET*)bl;
			if (vd) {
				memcpy(&pd->vd, vd, sizeof(struct view_data));
				if (!pcdb_checkid(vd->class_)) {
					pd->vd.hair_style = battle_config.pet_hair_style;
					if(pd->pet.equip) {
						pd->vd.head_bottom = itemdb_viewid(pd->pet.equip);
						if (!pd->vd.head_bottom)
							pd->vd.head_bottom = pd->pet.equip;
					}
				}
			} else
				ShowError("status_set_viewdata (PET): No view data for class %d\n", class_);
		}
	break;
	case BL_NPC:
		{
			TBL_NPC* nd = (TBL_NPC*)bl;
			if (vd)
				nd->vd = vd;
			else
				ShowError("status_set_viewdata (NPC): No view data for class %d\n", class_);
		}
	break;
	case BL_HOM:
		{
			struct homun_data *hd = (struct homun_data*)bl;
			if (vd)
				hd->vd = vd;
			else
				ShowError("status_set_viewdata (HOMUNCULUS): No view data for class %d\n", class_);
		}
		break;
	case BL_MER:
		{
			struct mercenary_data *md = (struct mercenary_data*)bl;
			if (vd)
				md->vd = vd;
			else
				ShowError("status_set_viewdata (MERCENARY): No view data for class %d\n", class_);
		}
		break;
	case BL_ELEM:
		{
			struct elemental_data *ed = (struct elemental_data*)bl;
			if (vd)
				ed->vd = vd;
			else
				ShowError("status_set_viewdata (ELEMENTAL): No view data for class %d\n", class_);
		}
		break;
	}
}

/**
 * Get status change data of an object
 * @param bl: Object whose sc data to get [PC|MOB|HOM|MER|ELEM|NPC]
 * @return status change data structure bl->sc
 */
struct status_change *status_get_sc(struct block_list *bl)
{
	if( bl )
	switch (bl->type) {
		case BL_PC:  return &((TBL_PC*)bl)->sc;
		case BL_MOB: return &((TBL_MOB*)bl)->sc;
		case BL_NPC: return &((TBL_NPC*)bl)->sc;
		case BL_HOM: return &((TBL_HOM*)bl)->sc;
		case BL_MER: return &((TBL_MER*)bl)->sc;
		case BL_ELEM: return &((TBL_ELEM*)bl)->sc;
	}
	return NULL;
}

/**
 * Initiate (memset) the status change data of an object
 * @param bl: Object whose sc data to memset [PC|MOB|HOM|MER|ELEM|NPC]
 */
void status_change_init(struct block_list *bl)
{
	struct status_change *sc = status_get_sc(bl);
	nullpo_retv(sc);
	memset(sc, 0, sizeof (struct status_change));
}

/*========================================== [Playtester]
* Returns the interval for status changes that iterate multiple times
* through the timer (e.g. those that deal damage in regular intervals)
* @param type: Status change (SC_*)
*------------------------------------------*/
static int status_get_sc_interval(enum sc_type type)
{
	switch (type) {
		case SC_POISON:
		case SC_DPOISON:
		case SC_LEECHESEND:
			return 1000;
		case SC_BURNING:
		case SC_PYREXIA:
			return 3000;
		case SC_MAGICMUSHROOM:
			return 4000;
		case SC_STONE:
			return 5000;
		case SC_BLEEDING:
		case SC_TOXIN:
			return 10000;
		default:
			break;
	}
	return 0;
}

/**
 * Applies SC defense to a given status change
 * This function also determines whether or not the status change will be applied
 * @param src: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param bl: Target of the status change
 * @param type: Status change (SC_*)
 * @param rate: Initial percentage rate of affecting bl
 * @param tick: Initial duration that the status change affects bl
 * @param flag: Value which determines what parts to calculate. See e_status_change_start_flags
 * @return adjusted duration based on flag values
 */
t_tick status_get_sc_def(struct block_list *src, struct block_list *bl, enum sc_type type, int rate, t_tick tick, unsigned char flag)
{
	/// Resistance rate: 10000 = 100%
	/// Example:	50% (5000) -> sc_def = 5000 -> 25%;
	///				5000ms -> tick_def = 5000 -> 2500ms
	int sc_def = 0, tick_def = -1; // -1 = use sc_def
	/// Fixed resistance value (after rate calculation)
	/// Example:	25% (2500) -> sc_def2 = 2000 -> 5%;
	///				2500ms -> tick_def2=2000 -> 500ms
	int sc_def2 = 0, tick_def2 = 0;

	uint16 min_rate = 0;
	t_tick min_dur = 0;

	struct status_data *status, *status_src;
	struct status_change *sc;
	struct map_session_data *sd;

	nullpo_ret(bl);
	if (src == NULL)
		return tick?tick:1; // This should not happen in current implementation, but leave it anyway

	// Status that are blocked by Golden Thief Bug card or Wand of Hermode
	if (status_isimmune(bl) && status_sc_get_flag(type)&SCF_FAILED_IMMUNITY)
		return 0;

	sd = BL_CAST(BL_PC,bl);
	status = status_get_status_data(bl);
	status_src = status_get_status_data(src);
	sc = status_get_sc(bl);
	if( sc && !sc->count )
		sc = NULL;

	switch (type) {
		case SC_POISON:
		case SC_DPOISON:
			sc_def = status->vit*100;
#ifndef RENEWAL
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			if (sd) {
				// For players: 60000 - 450*vit - 100*luk
				tick_def = status->vit*75;
				tick_def2 = status->luk*100;
			} else {
				// For monsters: 30000 - 200*vit
				tick>>=1;
				tick_def = (status->vit*200)/3;
			}
#endif
			break;
		case SC_STUN:
			sc_def = status->vit*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status->luk*10;
			break;
		case SC_SILENCE:
#ifndef RENEWAL
			sc_def = status->vit*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
#else
			sc_def = status->int_*100;
			sc_def2 = (status->vit + status->luk) * 5 + status_get_lv(bl)*10 - status_get_lv(src)*10;
#endif
			tick_def2 = status->luk*10;
			break;
		case SC_BLEEDING:
#ifndef RENEWAL
			sc_def = status->vit*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
#else
			sc_def = status->agi*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
#endif
			tick_def2 = status->luk*10;
			break;
		case SC_SLEEP:
#ifndef RENEWAL
			sc_def = status->int_*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
#else
			sc_def = status->agi*100;
			sc_def2 = (status->int_ + status->luk) * 5 + status_get_lv(bl)*10 - status_get_lv(src)*10;
#endif
			tick_def2 = status->luk*10;
			break;
		case SC_STONE:
			sc_def = status->mdef*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def = 0; // No duration reduction
			break;
		case SC_FREEZE:
			sc_def = status->mdef*100;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status_src->luk*-10; // Caster can increase final duration with luk
			break;
		case SC_CURSE:
			// Special property: immunity when luk is zero
			if (status->luk == 0)
				return 0;
			sc_def = status->luk*100;
			sc_def2 = status->luk*10 - status_get_lv(src)*10; // Curse only has a level penalty and no resistance
			tick_def = status->vit*100;
			tick_def2 = status->luk*10;
			break;
		case SC_BLIND:
			sc_def = (status->vit + status->int_)*50;
			sc_def2 = status->luk*10 + status_get_lv(bl)*10 - status_get_lv(src)*10;
			tick_def2 = status->luk*10;
			break;
		case SC_CONFUSION:
			sc_def = (status->str + status->int_)*50;
			sc_def2 = status_get_lv(src)*10 - status_get_lv(bl)*10 - status->luk*10; // Reversed sc_def2
			tick_def2 = status->luk*10;
			break;
		case SC_DECREASEAGI:
			if (sd)
				tick >>= 1; // Half duration for players.
			sc_def2 = status->mdef*100;
			break;
		case SC_ANKLE:
			if(status_has_mode(status,MD_STATUS_IMMUNE)) // Lasts 5 times less on bosses
				tick /= 5;
			sc_def = status->agi*50;
			break;
		case SC_DEEPSLEEP:
			tick_def2 = status_get_base_status(bl)->int_ * 25 + status_get_lv(bl) * 50;
			break;
		case SC_NETHERWORLD:
			// Resistance: {(Target's Base Level / 50) + (Target's Job Level / 10)} seconds
			tick_def2 = status_get_lv(bl) * 20 + (sd ? sd->status.job_level : 1) * 100;
			break;
		case SC_MARSHOFABYSS:
			// 5 second (Fixed) + 25 second - {( INT + LUK ) / 20 second }
			tick_def2 = (status->int_ + status->luk)*50;
			break;
		case SC_STASIS:
			// 10 second (fixed) + { Stasis Skill level * 10 - (Target's VIT + DEX) / 20 }
			tick_def2 = (status->vit + status->dex) * 50;
			break;
		case SC_WHITEIMPRISON:
			if( tick == 5000 ) // 100% on caster
				break;
			if( bl->type == BL_PC )
				tick_def2 = status_get_lv(bl)*20 + status->vit*25 + status->agi*10;
			else
				tick_def2 = (status->vit + status->luk)*50;
			break;
		case SC_BURNING:
			tick_def2 = 75*status->luk + 125*status->agi;
			break;
		case SC_FREEZING:
			tick_def2 = (status->vit + status->dex)*50;
			break;
		case SC_OBLIVIONCURSE: // 100% - (100 - 0.8 x INT)
			sc_def = status->int_*80;
			sc_def = max(sc_def, 500); // minimum of 5% resist
			tick_def = 0;
			//Fall through
		case SC_TOXIN:
		case SC_PARALYSE:
		case SC_VENOMBLEED:
		case SC_MAGICMUSHROOM:
		case SC_DEATHHURT:
		case SC_PYREXIA:
		case SC_LEECHESEND:
			tick_def2 = (status->vit + status->luk) * 500;
			break;
		case SC_BITE: // {(Base Success chance) - (Target's AGI / 4)}
			sc_def2 = status->agi*25;
			break;
		case SC_ELECTRICSHOCKER:
			tick_def2 = (status->vit + status->agi) * 70;
			break;
		case SC_CRYSTALIZE:
			tick_def2 = (sd ? sd->status.vit : status_get_base_status(bl)->vit) * 100;
			break;
		case SC_VACUUM_EXTREME:
			tick_def2 = (sd ? sd->status.str : status_get_base_status(bl)->str) * 50;
			break;
		case SC_KYOUGAKU:
			tick_def2 = 30*status->int_;
			break;
		case SC_PARALYSIS:
			tick_def2 = (status->vit + status->luk)*50;
			break;
		case SC_VOICEOFSIREN:
			// Resistance: {(Target's Base Level / 10) + (Target's Job Level / 5)} seconds
			tick_def2 = status_get_lv(bl) * 100 + (sd ? sd->status.job_level : 1) * 200;
			break;
		case SC_B_TRAP:
			tick_def = (sd ? sd->status.str : status_get_base_status(bl)->str) * 50; //! TODO: Figure out reduction formula
			break;
		case SC_NORECOVER_STATE:
			tick_def2 = status->luk * 100;
			break;
		default:
			// Effect that cannot be reduced? Likely a buff.
			if (!(rnd()%10000 < rate))
				return 0;
			return tick ? tick : 1;
	}

	if (sd) {
		if (battle_config.pc_sc_def_rate != 100) {
			sc_def = sc_def*battle_config.pc_sc_def_rate/100;
			sc_def2 = sc_def2*battle_config.pc_sc_def_rate/100;
		}

		sc_def = min(sc_def, battle_config.pc_max_sc_def*100);
		sc_def2 = min(sc_def2, battle_config.pc_max_sc_def*100);

		if (battle_config.pc_sc_def_rate != 100) {
			tick_def = tick_def*battle_config.pc_sc_def_rate/100;
			tick_def2 = tick_def2*battle_config.pc_sc_def_rate/100;
		}
	} else {
		if (battle_config.mob_sc_def_rate != 100) {
			sc_def = sc_def*battle_config.mob_sc_def_rate/100;
			sc_def2 = sc_def2*battle_config.mob_sc_def_rate/100;
		}

		sc_def = min(sc_def, battle_config.mob_max_sc_def*100);
		sc_def2 = min(sc_def2, battle_config.mob_max_sc_def*100);

		if (battle_config.mob_sc_def_rate != 100) {
			tick_def = tick_def*battle_config.mob_sc_def_rate/100;
			tick_def2 = tick_def2*battle_config.mob_sc_def_rate/100;
		}
	}

	if (sc) {
		if (sc->data[SC_SCRESIST])
			sc_def += sc->data[SC_SCRESIST]->val1*100; // Status resist
		else if (sc->data[SC_SIEGFRIED])
			sc_def += sc->data[SC_SIEGFRIED]->val3*100; // Status resistance.
		else if (sc->data[SC_SHIELDSPELL_REF] && sc->data[SC_SHIELDSPELL_REF]->val1 == 2)
			sc_def += sc->data[SC_SHIELDSPELL_REF]->val3*100;
	}

	// When tick def not set, reduction is the same for both.
	if(tick_def == -1)
		tick_def = sc_def;

	// Natural resistance
	if (!(flag&SCSTART_NORATEDEF)) {
		rate -= rate*sc_def/10000;
		rate -= sc_def2;

		// Item resistance (only applies to rate%)
		if (sd) {
			for (const auto &it : sd->reseff) {
				if (it.id == type)
					rate -= rate * it.val / 10000;
			}
			if (sd->sc.data[SC_COMMONSC_RESIST] && SC_COMMON_MIN <= type && type <= SC_COMMON_MAX)
				rate -= rate*sd->sc.data[SC_COMMONSC_RESIST]->val1/100;
		}

		// Aegis accuracy
		if(rate > 0 && rate%10 != 0) rate += (10 - rate%10);
	}

	// Cap minimum rate
	if ((min_rate = status_sc_get_min_rate(type)))
		rate = max(rate, min_rate);
	// Cap minimum duration
	if ((min_dur = status_sc_get_min_duration(type)))
		tick = i64max(tick, min_dur);

	if (rate < 10000 && (rate <= 0 || !(rnd()%10000 < rate)))
		return 0;

	// Duration cannot be reduced
	if (flag&SCSTART_NOTICKDEF)
		return i64max(tick, 1);

	tick -= tick*tick_def/10000;
	tick -= tick_def2;

	// Minimum durations
	switch (type) {
		case SC_ANKLE:
		case SC_MARSHOFABYSS:
			tick = i64max(tick, 5000); // Minimum duration 5s
			break;
		case SC_FREEZING:
			tick = i64max(tick, 6000); // Minimum duration 6s
			// NEED AEGIS CHECK: might need to be 10s (http://ro.gnjoy.com/news/notice/View.asp?seq=5352)
			break;
		case SC_BURNING:
		case SC_STASIS:
		case SC_VOICEOFSIREN:
			tick = i64max(tick, 10000); // Minimum duration 10s
			break;
		default:
			// Skills need to trigger even if the duration is reduced below 1ms
			tick = i64max(tick, 1);
			break;
	}

	return tick;
}

/**
 * Applies SC effect
 * @param bl: Source to apply effect
 * @param type: Status change (SC_*)
 * @param dval1~3: Depends on type of status change
 * Author: Ind
 */
void status_display_add(struct block_list *bl, enum sc_type type, int dval1, int dval2, int dval3) {
	struct eri *eri;
	struct sc_display_entry **sc_display;
	struct sc_display_entry ***sc_display_ptr;
	struct sc_display_entry *entry;
	int i;
	unsigned char sc_display_count;
	unsigned char *sc_display_count_ptr;

	nullpo_retv(bl);

	switch( bl->type ){
		case BL_PC: {
			struct map_session_data* sd = (struct map_session_data*)bl;

			sc_display_ptr = &sd->sc_display;
			sc_display_count_ptr = &sd->sc_display_count;
			eri = pc_sc_display_ers;
			}
			break;
		case BL_NPC: {
			struct npc_data* nd = (struct npc_data*)bl;

			sc_display_ptr = &nd->sc_display;
			sc_display_count_ptr = &nd->sc_display_count;
			eri = npc_sc_display_ers;
			}
			break;
		default:
			return;
	}

	sc_display = *sc_display_ptr;
	sc_display_count = *sc_display_count_ptr;

	ARR_FIND(0, sc_display_count, i, sc_display[i]->type == type);

	if( i != sc_display_count ) {
		sc_display[i]->val1 = dval1;
		sc_display[i]->val2 = dval2;
		sc_display[i]->val3 = dval3;
		return;
	}

	entry = ers_alloc(eri, struct sc_display_entry);

	entry->type = type;
	entry->val1 = dval1;
	entry->val2 = dval2;
	entry->val3 = dval3;

	RECREATE(sc_display, struct sc_display_entry *, ++sc_display_count);
	sc_display[sc_display_count - 1] = entry;
	*sc_display_ptr = sc_display;
	*sc_display_count_ptr = sc_display_count;
}

/**
 * Removes SC effect
 * @param bl: Source to remove effect
 * @param type: Status change (SC_*)
 * Author: Ind
 */
void status_display_remove(struct block_list *bl, enum sc_type type) {
	struct eri *eri;
	struct sc_display_entry **sc_display;
	struct sc_display_entry ***sc_display_ptr;
	int i;
	unsigned char sc_display_count;
	unsigned char *sc_display_count_ptr;

	nullpo_retv(bl);

	switch( bl->type ){
		case BL_PC: {
			struct map_session_data* sd = (struct map_session_data*)bl;

			sc_display_ptr = &sd->sc_display;
			sc_display_count_ptr = &sd->sc_display_count;
			eri = pc_sc_display_ers;
			}
			break;
		case BL_NPC: {
			struct npc_data* nd = (struct npc_data*)bl;

			sc_display_ptr = &nd->sc_display;
			sc_display_count_ptr = &nd->sc_display_count;
			eri = npc_sc_display_ers;
			}
			break;
		default:
			return;
	}

	sc_display = *sc_display_ptr;
	sc_display_count = *sc_display_count_ptr;

	ARR_FIND(0, sc_display_count, i, sc_display[i]->type == type);

	if( i != sc_display_count ) {
		int cursor;

		ers_free(eri, sc_display[i]);
		sc_display[i] = NULL;

		/* The all-mighty compact-o-matic */
		for( i = 0, cursor = 0; i < sc_display_count; i++ ) {
			if( sc_display[i] == NULL )
				continue;

			if( i != cursor )
				sc_display[cursor] = sc_display[i];

			cursor++;
		}

		if( !(sc_display_count = cursor) ) {
			aFree(sc_display);
			sc_display = NULL;
		}

		*sc_display_ptr = sc_display;
		*sc_display_count_ptr = sc_display_count;
	}
}

/**
 * Applies SC defense to a given status change
 * This function also determines whether or not the status change will be applied
 * @param src: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param bl: Target of the status change (See: enum sc_type)
 * @param type: Status change (SC_*)
 * @param rate: Initial percentage rate of affecting bl (0~10000)
 * @param val1~4: Depends on type of status change
 * @param tick: Initial duration that the status change affects bl
 * @param flag: Value which determines what parts to calculate. See e_status_change_start_flags
 * @return adjusted duration based on flag values
 */
int status_change_start(struct block_list* src, struct block_list* bl,enum sc_type type,int rate,int val1,int val2,int val3,int val4,t_tick duration,unsigned char flag) {
	struct map_session_data *sd = NULL;
	struct status_change* sc;
	struct status_change_entry* sce;
	struct status_data *status;
	struct view_data *vd;
	int opt_flag, undead_flag, val_flag = 0, t_tickime = 0;
	bool sc_isnew = true;
	uint64 calc_flag;

	nullpo_ret(bl);
	sc = status_get_sc(bl);
	status = status_get_status_data(bl);
	auto scdb = status_get(type);

	if( type <= SC_NONE || type >= SC_MAX || !scdb ) {
		ShowError("status_change_start: Invalid status change (%d)!\n", type);
		return 0;
	}

	if( !sc )
		return 0; // Unable to receive status changes

	if( bl->type != BL_NPC && status_isdead(bl) && ( type != SC_NOCHAT && type != SC_JAILED ) ) // SC_NOCHAT and SC_JAILED should work even on dead characters
		return 0;

	if (status_change_isDisabledOnMap(type, map_getmapdata(bl->m)))
		return 0;

	// Uncomment to prevent status adding hp to gvg mob (like bloodylust=hp*3 etc...
//	if (bl->type == BL_MOB)
//		if (status_get_race2(bl) == RC2_GVG && status_sc2scb_flag(type)&SCB_MAXHP) return 0;

	// Fail if Madogear is active
	if (sc->option&OPTION_MADOGEAR && scdb->flag&SCF_FAILED_MADO)
		return 0;

	// Check for Boss resistances
	if(status->mode&MD_STATUS_IMMUNE && !(flag&SCSTART_NOAVOID) && scdb->flag&SCF_BOSS_RESIST)
		return 0;

	// Check for MVP resistance
	if(status->mode&MD_MVP && !(flag&SCSTART_NOAVOID) && scdb->flag&SCF_MVP_RESIST)
		return 0;

	// Check failing SCs from list
	if (!scdb->fail.empty()) {
		for (const auto &it : scdb->fail) {
			if (it && sc->data[it])
				return 0;
		}
	}

	// Adjust tick according to status resistances
	if( !(flag&(SCSTART_NOAVOID|SCSTART_LOADED)) ) {
		duration = status_get_sc_def(src, bl, type, rate, duration, flag);
		if( !duration )
			return 0;
	}

	int tick = (int)duration;

	sd = BL_CAST(BL_PC, bl);
	vd = status_get_viewdata(bl);

	undead_flag = battle_check_undead(status->race,status->def_ele);
	// Check for immunities / sc fails
	switch (type) {
		case SC_VACUUM_EXTREME:
			if (sc->data[SC_VACUUM_EXTREME_POSTDELAY] && sc->data[SC_VACUUM_EXTREME_POSTDELAY]->val2 == val2) // Ignore post delay from other vacuum (this will make stack effect enabled)
		break;
		case SC_STONE:
		case SC_FREEZE:
			// Undead are immune to Freeze/Stone
			if (undead_flag && !(flag&SCSTART_NOAVOID))
				return 0;
		case SC_ALL_RIDING:
			if( !sd || sc->option&(OPTION_RIDING|OPTION_DRAGON|OPTION_WUG|OPTION_MADOGEAR) )
				return 0;
			break;
		case SC_SIGNUMCRUCIS:
			// Only affects demons and undead element (but not players)
			if((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC)
				return 0;
			break;
		case SC_AETERNA:
			if( sc->data[SC_STONE] && sc->opt1 == OPT1_STONE )
				return 0;
			break;
		case SC_KYRIE:
		case SC_TUNAPARTY:
			if (bl->type == BL_MOB)
				return 0;
			break;
		case SC_ADRENALINE:
			if(sd && !pc_check_weapontype(sd,skill_get_weapontype(BS_ADRENALINE)))
				return 0;
			break;
		case SC_ADRENALINE2:
			if(sd && !pc_check_weapontype(sd,skill_get_weapontype(BS_ADRENALINE2)))
				return 0;
			break;
		case SC_CLOAKING:
			// Avoid cloaking with no wall and low skill level. [Skotlex]
			// Due to the cloaking card, we have to check the wall versus to known
			// skill level rather than the used one. [Skotlex]
			// if (sd && val1 < 3 && skill_check_cloaking(bl,NULL))
			if( sd && pc_checkskill(sd, AS_CLOAKING) < 3 && !skill_check_cloaking(bl,NULL) )
				return 0;
			break;
		case SC_MODECHANGE: {
				enum e_mode mode;
				struct status_data *bstatus = status_get_base_status(bl);
				if (!bstatus) return 0;
				if (sc->data[type]) { // Pile up with previous values.
					if (!val2) val2 = sc->data[type]->val2;
					val3 |= sc->data[type]->val3;
					val4 |= sc->data[type]->val4;
				}
				mode = val2 ? static_cast<e_mode>((val2&~MD_MASK) | val2) : bstatus->mode; // Base mode
				if (val4) mode = static_cast<e_mode>(mode&~val4); // Del mode
				if (val3) mode = static_cast<e_mode>(mode | val3); // Add mode
				if (mode == bstatus->mode) { // No change.
					if (sc->data[type]) // Abort previous status
						return status_change_end(bl, type, INVALID_TIMER);
					return 0;
				}
			}
			break;
		// Strip skills, need to divest something or it fails.
		case SC_STRIPWEAPON:
			if (sd && !(flag&SCSTART_LOADED)) { // Apply sc anyway if loading saved sc_data
				short i;
				opt_flag = 0; // Reuse to check success condition.
				if(sd->bonus.unstripable_equip&EQP_WEAPON)
					return 0;
				i = sd->equip_index[EQI_HAND_L];
				if (i>=0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON) {
					opt_flag|=1;
					pc_unequipitem(sd,i,3); // Left-hand weapon
				}

				i = sd->equip_index[EQI_HAND_R];
				if (i>=0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_WEAPON) {
					opt_flag|=2;
					pc_unequipitem(sd,i,3);
				}
				if (!opt_flag) return 0;
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_STRIPSHIELD:
			if( val2 == 1 ) val2 = 0; // GX effect. Do not take shield off..
			else
			if (sd && !(flag&SCSTART_LOADED)) {
				short i;
				if(sd->bonus.unstripable_equip&EQP_SHIELD)
					return 0;
				i = sd->equip_index[EQI_HAND_L];
				if ( i < 0 || !sd->inventory_data[i] || sd->inventory_data[i]->type != IT_ARMOR )
					return 0;
				pc_unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_STRIPARMOR:
			if (sd && !(flag&SCSTART_LOADED)) {
				short i;
				if(sd->bonus.unstripable_equip&EQP_ARMOR)
					return 0;
				i = sd->equip_index[EQI_ARMOR];
				if ( i < 0 || !sd->inventory_data[i] )
					return 0;
				pc_unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_STRIPHELM:
			if (sd && !(flag&SCSTART_LOADED)) {
				short i;
				if(sd->bonus.unstripable_equip&EQP_HELM)
					return 0;
				i = sd->equip_index[EQI_HEAD_TOP];
				if ( i < 0 || !sd->inventory_data[i] )
					return 0;
				pc_unequipitem(sd,i,3);
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_MERC_FLEEUP:
		case SC_MERC_ATKUP:
		case SC_MERC_HPUP:
		case SC_MERC_SPUP:
		case SC_MERC_HITUP:
			if( bl->type != BL_MER )
				return 0; // Stats only for Mercenaries
			break;
		case SC_STRFOOD:
			if (sc->data[SC_FOOD_STR_CASH] && sc->data[SC_FOOD_STR_CASH]->val1 > val1)
				return 0;
			break;
		case SC_AGIFOOD:
			if (sc->data[SC_FOOD_AGI_CASH] && sc->data[SC_FOOD_AGI_CASH]->val1 > val1)
				return 0;
			break;
		case SC_VITFOOD:
			if (sc->data[SC_FOOD_VIT_CASH] && sc->data[SC_FOOD_VIT_CASH]->val1 > val1)
				return 0;
			break;
		case SC_INTFOOD:
			if (sc->data[SC_FOOD_INT_CASH] && sc->data[SC_FOOD_INT_CASH]->val1 > val1)
				return 0;
			break;
		case SC_DEXFOOD:
			if (sc->data[SC_FOOD_DEX_CASH] && sc->data[SC_FOOD_DEX_CASH]->val1 > val1)
				return 0;
			break;
		case SC_LUKFOOD:
			if (sc->data[SC_FOOD_LUK_CASH] && sc->data[SC_FOOD_LUK_CASH]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_STR_CASH:
			if (sc->data[SC_STRFOOD] && sc->data[SC_STRFOOD]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_AGI_CASH:
			if (sc->data[SC_AGIFOOD] && sc->data[SC_AGIFOOD]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_VIT_CASH:
			if (sc->data[SC_VITFOOD] && sc->data[SC_VITFOOD]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_INT_CASH:
			if (sc->data[SC_INTFOOD] && sc->data[SC_INTFOOD]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_DEX_CASH:
			if (sc->data[SC_DEXFOOD] && sc->data[SC_DEXFOOD]->val1 > val1)
				return 0;
			break;
		case SC_FOOD_LUK_CASH:
			if (sc->data[SC_LUKFOOD] && sc->data[SC_LUKFOOD]->val1 > val1)
				return 0;
			break;
		case SC_CAMOUFLAGE:
			if( sd && pc_checkskill(sd, RA_CAMOUFLAGE) < 3 && !skill_check_camouflage(bl,NULL) )
				return 0;
			break;
		case SC__STRIPACCESSORY:
			if( sd ) {
				short i = -1;
				if( !(sd->bonus.unstripable_equip&EQP_ACC_L) ) {
					i = sd->equip_index[EQI_ACC_L];
					if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
						pc_unequipitem(sd,i,3); // Left-Accessory
				}
				if( !(sd->bonus.unstripable_equip&EQP_ACC_R) ) {
					i = sd->equip_index[EQI_ACC_R];
					if( i >= 0 && sd->inventory_data[i] && sd->inventory_data[i]->type == IT_ARMOR )
						pc_unequipitem(sd,i,3); // Right-Accessory
				}
				if( i < 0 )
					return 0;
			}
			if (tick == 1) return 1; // Minimal duration: Only strip without causing the SC
			break;
		case SC_C_MARKER:
			if (src == bl)
				return 0;
			else {
				struct status_change *tsc = status_get_sc(bl);
				// Failed if the target is already marked and the new marker that isn't same marker
				if (tsc && tsc->data[type] && tsc->data[type]->val2 != src->id)
					return 0;
			}
			break;
	}

	// Before overlapping fail, one must check for status cured.
	switch (type) {
		case SC_BLESSING:
			// !TODO: Blessing and Agi up should do 1 damage against players on Undead Status, even on PvM
			// !but cannot be plagiarized (this requires aegis investigation on packets and official behavior) [Brainstorm]
			if ((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC) {
				status_change_end(bl, SC_CURSE, INVALID_TIMER);
				if (sc->data[SC_STONE] && sc->opt1 == OPT1_STONE)
					status_change_end(bl, SC_STONE, INVALID_TIMER);
				if (sc->data[SC_CURSE]) {
						status_change_end(bl, SC_CURSE, INVALID_TIMER);
						return 1; // End Curse and do not give stat boost
				}
			}
			if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
				status_change_end(bl, SC_SPIRIT, INVALID_TIMER);
			break;
		case SC_INCREASEAGI:
			if(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_HIGH)
				status_change_end(bl, SC_SPIRIT, INVALID_TIMER);
			break;
		case SC_DELUGE:
			if (sc->data[SC_FOGWALL] && sc->data[SC_BLIND])
				status_change_end(bl, SC_BLIND, INVALID_TIMER);
			break;
		case SC_SILENCE:
			if (sc->data[SC_GOSPEL] && sc->data[SC_GOSPEL]->val4 == BCT_SELF)
				status_change_end(bl, SC_GOSPEL, INVALID_TIMER);
			break;
		case SC_IMPOSITIO:
			if (sc->data[SC_IMPOSITIO] && sc->data[SC_IMPOSITIO]->val1 > val1) //Replace higher level effect for lower.
				status_change_end(bl,SC_IMPOSITIO,INVALID_TIMER);
			break;
		default:
			// If new SC has OPT1 while unit has OPT1, fail it!
			if (sc->opt1 && scdb->opt1)
				return 0;
			break;
	}

	std::vector<sc_type> list;

	if (type == SC_BERSERK && val3 == SC__BLOODYLUST) //There is some reasons that using SC_BERSERK first before SC__BLOODYLUST itself on Akinari's fix
		list = status_sc_get_end_list(SC__BLOODYLUST);
	else
		list = scdb->end;

	// End the SCs from the list
	if (!list.empty()) {
		bool isRemoving = false;

		for (const auto &it : list) {
			sc_type rem_sc = it;

			if (sc->data[rem_sc]) {
				switch (rem_sc) {
					case SC_BERSERK:
					case SC_SATURDAYNIGHTFEVER:
						sc->data[rem_sc]->val2 = 0; // Mark to not lose hp
					default:
						status_change_end(bl, rem_sc, INVALID_TIMER);
						isRemoving = true;
						break;
				}
			}
		}
		if (isRemoving && status_sc_get_end_return(type))
			return 0;
	}

	// Check for overlapping fails
	if( (sce = sc->data[type]) ) {
		if (scdb->flag&SCF_OVERLAP_FAIL)
			return 0;
		switch( type ) {
			case SC_MERC_FLEEUP:
			case SC_MERC_ATKUP:
			case SC_MERC_HPUP:
			case SC_MERC_SPUP:
			case SC_MERC_HITUP:
				if( sce->val1 > val1 )
					val1 = sce->val1;
				break;
			case SC_ADRENALINE:
			case SC_ADRENALINE2:
			case SC_WEAPONPERFECTION:
			case SC_OVERTHRUST:
				if (sce->val2 > val2)
					return 0;
				break;
			case SC_COMBO:
			case SC_DANCING:
			case SC_DEVOTION:
			case SC_ASPDPOTION0:
			case SC_ASPDPOTION1:
			case SC_ASPDPOTION2:
			case SC_ASPDPOTION3:
			case SC_ATKPOTION:
			case SC_MATKPOTION:
			case SC_ENCHANTARMS:
			case SC_ARMOR_ELEMENT_WATER:
			case SC_ARMOR_ELEMENT_EARTH:
			case SC_ARMOR_ELEMENT_FIRE:
			case SC_ARMOR_ELEMENT_WIND:
			case SC_ARMOR_RESIST:
			case SC_ATTHASTE_CASH:
			case SC_LHZ_DUN_N1:
			case SC_LHZ_DUN_N2:
			case SC_LHZ_DUN_N3:
			case SC_LHZ_DUN_N4:
				break;
			case SC_GOSPEL:
				 // Must not override a casting gospel char.
				if(sce->val4 == BCT_SELF)
					return 0;
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_ENDURE:
				if(sce->val4 && !val4)
					return 1; // Don't let you override infinite endure.
				if(sce->val1 > val1)
					return 1;
				break;
			case SC_JAILED:
				// When a player is already jailed, do not edit the jail data.
				val2 = sce->val2;
				val3 = sce->val3;
				val4 = sce->val4;
				break;
			case SC_LERADSDEW:
				if (sc && sc->data[SC_BERSERK])
					return 0;
			case SC_SHAPESHIFT:
			case SC_PROPERTYWALK:
				break;
			case SC_LEADERSHIP:
			case SC_GLORYWOUNDS:
			case SC_SOULCOLD:
			case SC_HAWKEYES:
				if( sce->val4 && !val4 ) // You cannot override master guild aura
					return 0;
				break;
			case SC_JOINTBEAT:
				val2 |= sce->val2; // Stackable ailments
			default:
				if(sce->val1 > val1)
					return 1; // Return true to not mess up skill animations. [Skotlex]
		}
	}

	vd = status_get_viewdata(bl);
	calc_flag = scdb->calc_flag;
	if(!(flag&SCSTART_LOADED)) // &4 - Do not parse val settings when loading SCs
	switch(type)
	{
		/* Permanent effects */
		case SC_AETERNA:
		case SC_MODECHANGE:
		case SC_WEIGHT50:
		case SC_WEIGHT90:
		case SC_BROKENWEAPON:
		case SC_BROKENARMOR:
		case SC_READYSTORM:
		case SC_READYDOWN:
		case SC_READYCOUNTER:
		case SC_READYTURN:
		case SC_DODGE:
		case SC_PUSH_CART:
		case SC_SPRITEMABLE:
		case SC_CLAN_INFO:
		case SC_DAILYSENDMAILCNT:
			tick = INFINITE_TICK;
			break;

		case SC_DECREASEAGI:
		case SC_INCREASEAGI:
		case SC_ADORAMUS:
			if (type == SC_ADORAMUS) {
				// 1000% base chance to blind, but still can be resisted
				sc_start(src, bl, SC_BLIND, 1000, val1, skill_get_time(scdb->skill_id, val1));
				if (sc->data[SC_ADORAMUS])
					return 0; //Adoramus can't refresh itself, but it can cause blind again
			}
			val2 = 2 + val1; // Agi change
			break;
		case SC_ENDURE:
			val2 = 7; // Hit-count [Celest]
			if( !(flag&SCSTART_NOAVOID) && (bl->type&(BL_PC|BL_MER)) && !map_flag_gvg2(bl->m) && !map_getmapflag(bl->m, MF_BATTLEGROUND) && !val4 ) {
				struct map_session_data *tsd;
				if( sd ) {
					int i;
					for( i = 0; i < MAX_DEVOTION; i++ ) {
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
							status_change_start(src,&tsd->bl, type, 10000, val1, val2, val3, val4, tick, SCSTART_NOAVOID|SCSTART_NOICON);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
					status_change_start(src,&tsd->bl, type, 10000, val1, val2, val3, val4, tick, SCSTART_NOAVOID|SCSTART_NOICON);
			}
			// val4 signals infinite endure (if val4 == 2 it is infinite endure from Berserk)
			if( val4 )
				tick = INFINITE_TICK;
			break;
		case SC_AUTOBERSERK:
			if (status->hp < status->max_hp>>2 &&
				(!sc->data[SC_PROVOKE] || sc->data[SC_PROVOKE]->val2==0))
					sc_start4(src,bl,SC_PROVOKE,100,10,1,0,0,60000);
			tick = INFINITE_TICK;
			break;
		case SC_SIGNUMCRUCIS:
			val2 = 10 + 4*val1; // Def reduction
			tick = INFINITE_TICK;
			clif_emotion(bl, ET_SWEAT);
			break;
		case SC_MAXIMIZEPOWER:
			t_tickime = val2 = tick>0?tick:60000;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			break;
		case SC_EDP:
			val2 = val1 + 2; // Chance to Poison enemies.
#ifndef RENEWAL
			val3 = 50*(val1+1); // Damage increase (+50 +50*lv%)
#endif
			if( sd )// [Ind] - iROwiki says each level increases its duration by 3 seconds
				tick += pc_checkskill(sd,GC_RESEARCHNEWPOISON)*3000;
			break;
		case SC_POISONREACT:
			val2=(val1+1)/2 + val1/10; // Number of counters [Skotlex]
			val3=50; // + 5*val1; // Chance to counter. [Skotlex]
			break;
		case SC_MAGICROD:
			val2 = val1*20; // SP gained
			break;
		case SC_KYRIE:
			if( val4 ) { // Formulas for Praefatio
				val2 = (status->max_hp * (val1 * 2 + 10) / 100) + val4 * 2; //%Max HP to absorb
				val3 = 6 + val1; //Hits
			} else { // Formulas for Kyrie Eleison
				val2 = status->max_hp * (val1 * 2 + 10) / 100;
				val3 = (val1 / 2 + 5);
			}
			break;
		case SC_MAGICPOWER:
			// val1: Skill lv
			val2 = 1; // Lasts 1 invocation
			val3 = 5*val1; // Matk% increase
			val4 = 0; // 0 = ready to be used, 1 = activated and running
			break;
		case SC_SACRIFICE:
			val2 = 5; // Lasts 5 hits
			tick = INFINITE_TICK;
			break;
		case SC_ENCPOISON:
			val2= 250+50*val1; // Poisoning Chance (2.5+0.5%) in 1/10000 rate
		case SC_ASPERSIO:
		case SC_FIREWEAPON:
		case SC_WATERWEAPON:
		case SC_WINDWEAPON:
		case SC_EARTHWEAPON:
		case SC_SHADOWWEAPON:
		case SC_GHOSTWEAPON:
			skill_enchant_elemental_end(bl,type);
			break;
		case SC_ELEMENTALCHANGE:
			// val1 : Element Lvl (if called by skill lvl 1, takes random value between 1 and 4)
			// val2 : Element (When no element, random one is picked)
			// val3 : 0 = called by skill 1 = called by script (fixed level)
			if( !val2 ) val2 = rnd()%ELE_ALL;

			if( val1 == 1 && val3 == 0 )
				val1 = 1 + rnd()%4;
			else if( val1 > 4 )
				val1 = 4; // Max Level
			val3 = 0; // Not need to keep this info.
			break;
		case SC_PROVIDENCE:
			val2 = val1*5; // Race/Ele resist
			break;
		case SC_REFLECTSHIELD:
			val2 = 10+val1*3; // %Dmg reflected
			// val4 used to mark if reflect shield is an inheritance bonus from Devotion
			if( !(flag&SCSTART_NOAVOID) && (bl->type&(BL_PC|BL_MER)) ) {
				struct map_session_data *tsd;
				if( sd ) {
					int i;
					for( i = 0; i < MAX_DEVOTION; i++ ) {
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
							status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 1, tick, SCSTART_NOAVOID|SCSTART_NOICON);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
					status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 1, tick, SCSTART_NOAVOID|SCSTART_NOICON);
			}
			break;
		case SC_STRIPWEAPON:
			if (!sd) // Watk reduction
				val2 = 25;
			break;
		case SC_STRIPSHIELD:
			if (!sd) // Def reduction
				val2 = 15;
			break;
		case SC_STRIPARMOR:
			if (!sd) // Vit reduction
				val2 = 40;
			break;
		case SC_STRIPHELM:
			if (!sd) // Int reduction
				val2 = 40;
			break;
		case SC_AUTOSPELL:
			// Val1 Skill LV of Autospell
			// Val2 Skill ID to cast
			// Val3 Max Lv to cast
			val4 = 5 + val1*2; // Chance of casting
			break;
		case SC_VOLCANO:
			{
				int8 enchant_eff[] = { 10, 14, 17, 19, 20 }; // Enchant addition
				uint8 i = max((val1-1)%5, 0);

				val2 = val1*10; // Watk increase
#ifndef RENEWAL
				if (status->def_ele != ELE_FIRE)
					val2 = 0;
#endif
				val3 = enchant_eff[i];
			}
			break;
		case SC_VIOLENTGALE:
			{
				int8 enchant_eff[] = { 10, 14, 17, 19, 20 }; // Enchant addition
				uint8 i = max((val1-1)%5, 0);

				val2 = val1*3; // Flee increase
#ifndef RENEWAL
				if (status->def_ele != ELE_WIND)
					val2 = 0;
#endif
				val3 = enchant_eff[i];
			}
			break;
		case SC_DELUGE:
			{
				int8 deluge_eff[]  = {  5,  9, 12, 14, 15 }; // HP addition rate n/100
				int8 enchant_eff[] = { 10, 14, 17, 19, 20 }; // Enchant addition
				uint8 i = max((val1-1)%5, 0);

				val2 = deluge_eff[i]; // HP increase
#ifndef RENEWAL
				if (status->def_ele != ELE_WATER)
					val2 = 0;
#endif
				val3 = enchant_eff[i];
			}
			break;
		case SC_SUITON:
			if (!val2 || (sd && (sd->class_&MAPID_BASEMASK) == MAPID_NINJA)) {
				// No penalties.
				val2 = 0; // Agi penalty
				val3 = 0; // Walk speed penalty
				break;
			}
			val3 = 50;
			val2 = 3*((val1+1)/3);
			if (val1 > 4) val2--;
			//Suiton is a special case, stop effect is forced and only happens when target enters it
			if (!unit_blown_immune(bl, 0x1))
				unit_stop_walking(bl, 9);
			break;
		case SC_ONEHAND:
		case SC_TWOHANDQUICKEN:
			val2 = 300;
			if (val1 > 10) // For boss casted skills [Skotlex]
				val2 += 20*(val1-10);
			break;
		case SC_MERC_QUICKEN:
			val2 = 300;
			break;
#ifndef RENEWAL_ASPD
		case SC_SPEARQUICKEN:
			val2 = 200+10*val1;
			break;
#endif
		case SC_DANCING:
			// val1 : Skill ID + LV
			// val2 : Skill Group of the Dance.
			// val3 : Brings the skill_lv (merged into val1 here)
			// val4 : Partner
			if (val1 == CG_MOONLIT)
				clif_status_change(bl,EFST_MOON,1,tick,0, 0, 0);
			val1|= (val3<<16);
			val3 = tick/1000; // Tick duration
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_LONGING:
			val2 = 500-100*val1; // Aspd penalty.
			break;
		case SC_EXPLOSIONSPIRITS:
			val2 = 75 + 25*val1; // Cri bonus
			break;

		case SC_ASPDPOTION0:
		case SC_ASPDPOTION1:
		case SC_ASPDPOTION2:
		case SC_ASPDPOTION3:
			val2 = 50*(2+type-SC_ASPDPOTION0);
			break;

		case SC_ATTHASTE_CASH:
			val2 = 50*val1; // Just custom for pre-re
			break;

		case SC_NOCHAT:
			// A hardcoded interval of 60 seconds is expected, as the time that SC_NOCHAT uses is defined by
			// mmocharstatus.manner, each negative point results in 1 minute with this status activated.
			// This is done this way because the message that the client displays is hardcoded, and only
			// shows how many minutes are remaining. [Panikon]
			tick = 60000;
			val1 = battle_config.manner_system; // Mute filters.
			if (sd) {
				clif_changestatus(sd,SP_MANNER,sd->status.manner);
				clif_updatestatus(sd,SP_MANNER);
			}
			break;

		case SC_STONE:
			val3 = max(val3, 100); // Incubation time
			val4 = max(tick-val3, 100); // Petrify time
			tick = val3;
			calc_flag = SCB_NONE; // Actual status changes take effect on petrified state.
			break;

		case SC_DPOISON:
			// Lose 10/15% of your life as long as it doesn't brings life below 25%
			if (status->hp > status->max_hp>>2) {
				int diff = status->max_hp*(bl->type==BL_PC?10:15)/100;
				if (status->hp - diff < status->max_hp>>2)
					diff = status->hp - (status->max_hp>>2);
				status_zap(bl, diff, 0);
			}
			// Fall through
		case SC_POISON:
		case SC_BLEEDING:
		case SC_BURNING:
		case SC_TOXIN:
		case SC_MAGICMUSHROOM:
		case SC_LEECHESEND:
			t_tickime = status_get_sc_interval(type);
			val4 = tick-t_tickime; // Remaining time
			break;

		case SC_PYREXIA:
			//Causes blind for duration of pyrexia, unreducable and unavoidable, but can be healed with e.g. green potion
			status_change_start(src,bl,SC_BLIND,10000,val1,0,0,0,tick,SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			t_tickime = status_get_sc_interval(type);
			val4 = tick-t_tickime; // Remaining time
			break;

		case SC_CONFUSION:
			if (!val4)
				clif_emotion(bl,ET_QUESTION);
			break;
		case SC_S_LIFEPOTION:
		case SC_L_LIFEPOTION:
			if( val1 == 0 ) return 0;
			// val1 = heal percent/amout
			// val2 = seconds between heals
			// val4 = total of heals
			if( val2 < 1 ) val2 = 1;
			if( (val4 = tick/(val2 * 1000)) < 1 )
				val4 = 1;
			t_tickime = val2 * 1000; // [GodLesZ] tick time
			break;
		case SC_BOSSMAPINFO:
			if( sd != NULL ) {
				struct mob_data *boss_md = map_getmob_boss(bl->m); // Search for Boss on this Map

				if( boss_md == NULL ) { // No MVP on this map
					clif_bossmapinfo(sd, NULL, BOSS_INFO_NOT);
					return 0;
				}
				val1 = boss_md->bl.id;
				t_tickime = 1000; // [GodLesZ] tick time
				val4 = tick / t_tickime;
			}
			break;
		case SC_HIDING:
			val2 = tick/1000;
			t_tickime = 1000; // [GodLesZ] tick time
			val3 = 0; // Unused, previously speed adjustment
			val4 = val1+3; // Seconds before SP substraction happen.
			break;
		case SC_CHASEWALK:
			val2 = tick>0?tick:10000; // Interval at which SP is drained.
			val3 = 35 - 5 * val1; // Speed adjustment.
			if (sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_ROGUE)
				val3 -= 40;
			val4 = 10+val1*2; // SP cost.
			if (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND)) val4 *= 5;
			break;
		case SC_CLOAKING:
			if (!sd) // Monsters should be able to walk with no penalties. [Skotlex]
				val1 = 10;
			t_tickime = val2 = tick>0?tick:60000; // SP consumption rate.
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			val3 = 0; // Unused, previously walk speed adjustment
			// val4&1 signals the presence of a wall.
			// val4&2 makes cloak not end on normal attacks [Skotlex]
			// val4&4 makes cloak not end on using skills
			if (bl->type == BL_PC || (bl->type == BL_MOB && ((TBL_MOB*)bl)->special_state.clone) )	// Standard cloaking.
				val4 |= battle_config.pc_cloak_check_type&7;
			else
				val4 |= battle_config.monster_cloak_check_type&7;
			break;
		case SC_SIGHT:			/* splash status */
		case SC_RUWACH:
		case SC_SIGHTBLASTER:
			val3 = skill_get_splash(val2, val1); // Val2 should bring the skill-id.
			val2 = tick/20;
			t_tickime = 20; // [GodLesZ] tick time
			break;

		case SC_AUTOGUARD:
			if( !(flag&SCSTART_NOAVOID) ) {
				struct map_session_data *tsd;
				int i;
				for( i = val2 = 0; i < val1; i++) {
					int t = 5-(i>>1);
					val2 += (t < 0)? 1:t;
				}

				if( bl->type&(BL_PC|BL_MER) ) {
					if( sd ) {
						for( i = 0; i < MAX_DEVOTION; i++ ) {
							if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) )
								status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 0, tick, SCSTART_NOAVOID|SCSTART_NOICON);
						}
					}
					else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag && (tsd = ((TBL_MER*)bl)->master) )
						status_change_start(src,&tsd->bl, type, 10000, val1, val2, 0, 0, tick, SCSTART_NOAVOID|SCSTART_NOICON);
				}
			}
			break;

		case SC_DEFENDER:
			if (!(flag&SCSTART_NOAVOID)) {
				val2 = 5 + 15*val1; // Damage reduction
				val3 = 0; // Unused, previously speed adjustment
				val4 = 250 - 50*val1; // Aspd adjustment

				if (sd) {
					struct map_session_data *tsd;
					int i;
					for (i = 0; i < MAX_DEVOTION; i++) { // See if there are devoted characters, and pass the status to them. [Skotlex]
						if (sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])))
							status_change_start(src,&tsd->bl,type,10000,val1,val2,val3,val4,tick,SCSTART_NOAVOID);
					}
				}
			}
			break;

		case SC_TENSIONRELAX:
			if (sd) {
				pc_setsit(sd);
				skill_sit(sd, 1);
				clif_sitting(&sd->bl);
			}
			val2 = 12; // SP cost
			t_tickime = 10000; // Decrease at 10secs intervals.
			val3 = tick / t_tickime;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			break;
		case SC_PARRYING:
		    val2 = 20 + val1*3; // Block Chance
			break;

		case SC_WINDWALK:
			val2 = (val1+1)/2; // Flee bonus is 1/1/2/2/3/3/4/4/5/5
			break;

		case SC_JOINTBEAT:
			if( val2&BREAK_NECK )
				sc_start2(src,bl,SC_BLEEDING,100,val1,val3,skill_get_time2(scdb->skill_id,val1));
			break;

		case SC_BERSERK:
			if( val3 == SC__BLOODYLUST )
				sc_start(src,bl,(sc_type)val3,100,val1,tick);
			if (!val3 && !(sc->data[SC_ENDURE] && sc->data[SC_ENDURE]->val4))
				sc_start4(src,bl, SC_ENDURE, 100,10,0,0,2, tick);
			// HP healing is performing after the calc_status call.
			// Val2 holds HP penalty
			if (!val4) val4 = skill_get_time2(scdb->skill_id,val1);
			if (!val4) val4 = 10000; // Val4 holds damage interval
			val3 = tick/val4; // val3 holds skill duration
			t_tickime = val4; // [GodLesZ] tick time
			break;

		case SC_GOSPEL:
			if(val4 == BCT_SELF) {	// Self effect
				val2 = tick/10000;
				t_tickime = 10000; // [GodLesZ] tick time
				status_change_clear_buffs(bl, SCCB_BUFFS|SCCB_DEBUFFS|SCCB_CHEM_PROTECT); // Remove buffs/debuffs
			}
			break;

		case SC_MARIONETTE:
		{
			int stat;

			val3 = 0;
			val4 = 0;
			stat = ( sd ? sd->status.str : status_get_base_status(bl)->str ) / 2; val3 |= cap_value(stat,0,0xFF)<<16;
			stat = ( sd ? sd->status.agi : status_get_base_status(bl)->agi ) / 2; val3 |= cap_value(stat,0,0xFF)<<8;
			stat = ( sd ? sd->status.vit : status_get_base_status(bl)->vit ) / 2; val3 |= cap_value(stat,0,0xFF);
			stat = ( sd ? sd->status.int_: status_get_base_status(bl)->int_) / 2; val4 |= cap_value(stat,0,0xFF)<<16;
			stat = ( sd ? sd->status.dex : status_get_base_status(bl)->dex ) / 2; val4 |= cap_value(stat,0,0xFF)<<8;
			stat = ( sd ? sd->status.luk : status_get_base_status(bl)->luk ) / 2; val4 |= cap_value(stat,0,0xFF);
			break;
		}
		case SC_MARIONETTE2:
		{
			int stat,max_stat;
			// Fetch caster information
			struct block_list *pbl = map_id2bl(val1);
			struct status_change *psc = pbl?status_get_sc(pbl):NULL;
			struct status_change_entry *psce = psc?psc->data[SC_MARIONETTE]:NULL;
			// Fetch target's stats
			struct status_data* status2 = status_get_status_data(bl); // Battle status

			if (!psce)
				return 0;

			val3 = 0;
			val4 = 0;
			max_stat = battle_config.max_parameter; // Cap to 99 (default)
			stat = (psce->val3 >>16)&0xFF; stat = min(stat, max_stat - status2->str ); val3 |= cap_value(stat,0,0xFF)<<16;
			stat = (psce->val3 >> 8)&0xFF; stat = min(stat, max_stat - status2->agi ); val3 |= cap_value(stat,0,0xFF)<<8;
			stat = (psce->val3 >> 0)&0xFF; stat = min(stat, max_stat - status2->vit ); val3 |= cap_value(stat,0,0xFF);
			stat = (psce->val4 >>16)&0xFF; stat = min(stat, max_stat - status2->int_); val4 |= cap_value(stat,0,0xFF)<<16;
			stat = (psce->val4 >> 8)&0xFF; stat = min(stat, max_stat - status2->dex ); val4 |= cap_value(stat,0,0xFF)<<8;
			stat = (psce->val4 >> 0)&0xFF; stat = min(stat, max_stat - status2->luk ); val4 |= cap_value(stat,0,0xFF);
			break;
		}
		case SC_SPIRIT:
			//1st Transcendent Spirit works similar to Marionette Control
			if(sd && val2 == SL_HIGH) {
				int stat,max_stat;
				struct status_data *status2 = status_get_base_status(bl);

				val3 = 0;
				val4 = 0;
				max_stat = (status_get_lv(bl)-10<50)?status_get_lv(bl)-10:50;
				stat = max(0, max_stat - status2->str); val3 |= cap_value(stat,0,0xFF)<<16;
				stat = max(0, max_stat - status2->agi ); val3 |= cap_value(stat,0,0xFF)<<8;
				stat = max(0, max_stat - status2->vit ); val3 |= cap_value(stat,0,0xFF);
				stat = max(0, max_stat - status2->int_); val4 |= cap_value(stat,0,0xFF)<<16;
				stat = max(0, max_stat - status2->dex ); val4 |= cap_value(stat,0,0xFF)<<8;
				stat = max(0, max_stat - status2->luk ); val4 |= cap_value(stat,0,0xFF);
			}
			break;

		case SC_REJECTSWORD:
			val2 = 15*val1; // Reflect chance
			val3 = 3; // Reflections
			tick = INFINITE_TICK;
			break;

		case SC_MEMORIZE:
			val2 = 5; // Memorized casts.
			tick = INFINITE_TICK;
			break;

		case SC_GRAVITATION:
			val2 = 50*val1; // aspd reduction
			break;

		case SC_REGENERATION:
			if (val1 == 1)
				val2 = 2;
			else
				val2 = val1; // HP Regerenation rate: 200% 200% 300%
			val3 = val1; // SP Regeneration Rate: 100% 200% 300%
			// If val4 comes set, this blocks regen rather than increase it.
			break;

		case SC_DEVOTION:
		{
			struct block_list *d_bl;
			struct status_change *d_sc;

			if( (d_bl = map_id2bl(val1)) && (d_sc = status_get_sc(d_bl)) && d_sc->count ) { // Inherits Status From Source
				const enum sc_type types[] = { SC_AUTOGUARD, SC_DEFENDER, SC_REFLECTSHIELD, SC_ENDURE };
				int i = (map_flag_gvg2(bl->m) || map_getmapflag(bl->m, MF_BATTLEGROUND))?2:3;
				while( i >= 0 ) {
					enum sc_type type2 = types[i];
					if( d_sc->data[type2] )
						status_change_start(d_bl, bl, type2, 10000, d_sc->data[type2]->val1, 0, 0, (type2 == SC_REFLECTSHIELD ? 1 : 0), skill_get_time(status_sc_get_skill(type2),d_sc->data[type2]->val1), (type2 == SC_DEFENDER) ? SCSTART_NOAVOID : SCSTART_NOAVOID|SCSTART_NOICON);
					i--;
				}
			}
			break;
		}

		case SC_COMA: // Coma. Sends a char to 1HP. If val2, do not zap sp
			status_zap(bl, status->hp-1, val2?0:status->sp);
			return 1;
			break;
		case SC_TINDER_BREAKER2:
		case SC_CLOSECONFINE2:
		{
			struct block_list *src2 = val2?map_id2bl(val2):NULL;
			struct status_change *sc2 = src2?status_get_sc(src2):NULL;
			enum sc_type type2 = ((type == SC_TINDER_BREAKER2)?SC_TINDER_BREAKER:SC_CLOSECONFINE);
			struct status_change_entry *sce2 = sc2?sc2->data[type2]:NULL;

			if (src2 && sc2) {
				if (!sce2) // Start lock on caster.
					sc_start4(src2,src2,type2,100,val1,1,0,0,tick+1000);
				else { // Increase count of locked enemies and refresh time.
					(sce2->val2)++;
					delete_timer(sce2->timer, status_change_timer);
					sce2->timer = add_timer(gettick()+tick+1000, status_change_timer, src2->id, type2);
				}
			} else // Status failed.
				return 0;
		}
			break;
		case SC_KAITE:
			val2 = 1+val1/5; // Number of bounces: 1 + skill_lv/5
			break;
		case SC_KAUPE:
			switch (val1) {
				case 3: // 33*3 + 1 -> 100%
					val2++;
				case 1:
				case 2: // 33, 66%
					val2 += 33*val1;
					val3 = 1; // Dodge 1 attack total.
					break;
				default: // Custom. For high level mob usage, higher level means more blocks. [Skotlex]
					val2 = 100;
					val3 = val1-2;
					break;
			}
			break;

		case SC_COMBO:
		{
			// val1: Skill ID
			// val2: When given, target (for autotargetting skills)
			// val3: When set, this combo time should NOT delay attack/movement
			// val3: If set to 2 this combo will delay ONLY attack
			// val3: TK: Last used kick
			// val4: TK: Combo time
			struct unit_data *ud = unit_bl2ud(bl);
			if ( ud && (!val3 || val3 == 2) ) {
				tick += 300 * battle_config.combo_delay_rate/100;
				ud->attackabletime = gettick()+tick;
				if( !val3 )
					unit_set_walkdelay(bl, gettick(), tick, 1);
			}
			val3 = 0;
			val4 = tick;
			break;
		}
		case SC_EARTHSCROLL:
			val2 = 11-val1; // Chance to consume: 11-skill_lv%
			break;
		case SC_RUN:
		{
			//Store time at which you started running.
			t_tick currenttick = gettick();
			// Note: this int64 value is stored in two separate int32 variables (FIXME)
			val3 = (int)(currenttick & 0x00000000ffffffffLL);
			val4 = (int)((currenttick & 0xffffffff00000000LL) >> 32);
			tick = INFINITE_TICK;
			break;
		}
		case SC_KAAHI:
			val2 = 200*val1; // HP heal
			val3 = 5*val1; // SP cost
			break;
		case SC_BLESSING:
			if ((!undead_flag && status->race!=RC_DEMON) || bl->type == BL_PC)
				val2 = val1;
			else
				val2 = 0; // 0 -> Half stat.
			break;
		case SC_TRICKDEAD:
			if (vd) vd->dead_sit = 1;
			tick = INFINITE_TICK;
			break;
		case SC_CONCENTRATE:
			val2 = 2 + val1;
			if (sd) { // Store the card-bonus data that should not count in the %
				val3 = sd->param_bonus[1]; // Agi
				val4 = sd->param_bonus[4]; // Dex
			} else
				val3 = val4 = 0;
			break;
		case SC_MAXOVERTHRUST:
			val2 = 20*val1; // Power increase
			break;
		case SC_OVERTHRUST:
		case SC_ADRENALINE2:
		case SC_ADRENALINE:
		case SC_WEAPONPERFECTION:
			{
				struct map_session_data * s_sd = BL_CAST(BL_PC, src);
				if (type == SC_OVERTHRUST) {
					// val2 holds if it was casted on self, or is bonus received from others
					val3 = (val2) ? 5 * val1 : 5; // Power increase
				}
				else if (type == SC_ADRENALINE2 || type == SC_ADRENALINE) {
					val3 = (val2) ? 300 : 200; // Aspd increase
				}
				if (s_sd && pc_checkskill(s_sd, BS_HILTBINDING) > 0)
					tick += tick / 10; //If caster has Hilt Binding, duration increases by 10%
			}
			break;
		case SC_CONCENTRATION:
			val2 = 5*val1; // Batk/Watk Increase
			val3 = 10*val1; // Hit Increase
			val4 = 5*val1; // Def reduction
			sc_start(src, bl, SC_ENDURE, 100, 1, tick); // Level 1 Endure effect
			break;
		case SC_ANGELUS:
			val2 = 5*val1; // def increase
			break;
		case SC_IMPOSITIO:
			val2 = 5*val1; // Watk increase
			break;
		case SC_MELTDOWN:
			val2 = 100*val1; // Chance to break weapon
			val3 = 70*val1; // Change to break armor
			break;
		case SC_TRUESIGHT:
			val2 = 10*val1; // Critical increase
			val3 = 3*val1; // Hit increase
			break;
		case SC_SUN_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk)/2; // def increase
			break;
		case SC_MOON_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk)/10; // flee increase
			break;
		case SC_STAR_COMFORT:
			val2 = (status_get_lv(bl) + status->dex + status->luk); // Aspd increase
			break;
		case SC_QUAGMIRE:
			val2 = (sd?5:10)*val1; // Agi/Dex decrease.
			break;

		// gs_something1 [Vicious]
		case SC_GATLINGFEVER:
			val2 = 20*val1; // Aspd increase
#ifndef RENEWAL
			val3 = 20+10*val1; // Atk increase
#endif
			val4 = 5*val1; // Flee decrease
			break;

		case SC_FLING:
			if (bl->type == BL_PC)
				val2 = 0; // No armor reduction to players.
			else
				val2 = 5*val1; // Def reduction
			val3 = 5*val1; // Def2 reduction
			break;
		case SC_PROVOKE:
			// val2 signals autoprovoke.
			val3 = 2+3*val1; // Atk increase
			val4 = 5+5*val1; // Def reduction.
			break;
		case SC_AVOID:
			// val2 = 10*val1; // Speed change rate.
			break;
		case SC_DEFENCE:
#ifdef RENEWAL
			val2 = 5 + (val1 * 5); // Vit bonus
#else
			val2 = 2*val1; // Def bonus
#endif
			break;
		case SC_BLOODLUST:
			val2 = 20+10*val1; // Atk rate change.
			val3 = 3*val1; // Leech chance
			val4 = 20; // Leech percent
			break;
		case SC_FLEET:
			val2 = 30*val1; // Aspd change
			val3 = 5+5*val1; // bAtk/wAtk rate change
			break;
		case SC_MINDBREAKER:
			val2 = 20*val1; // matk increase.
			val3 = 12*val1; // mdef2 reduction.
			break;
		case SC_JAILED:
			// Val1 is duration in minutes. Use INT_MAX to specify 'unlimited' time.
			tick = val1>0?1000:250;
			if (sd) {
				if (sd->mapindex != val2) {
					int pos =  (bl->x&0xFFFF)|(bl->y<<16), // Current Coordinates
					map_idx =  sd->mapindex; // Current Map
					// 1. Place in Jail (val2 -> Jail Map, val3 -> x, val4 -> y
					pc_setpos(sd,(unsigned short)val2,val3,val4, CLR_TELEPORT);
					// 2. Set restore point (val3 -> return map, val4 return coords
					val3 = map_idx;
					val4 = pos;
				} else if (!val3 || val3 == sd->mapindex) { // Use save point.
					val3 = sd->status.save_point.map;
					val4 = (sd->status.save_point.x&0xFFFF)
						|(sd->status.save_point.y<<16);
				}
			}
			break;
		case SC_UTSUSEMI:
			val2=(val1+1)/2; // Number of hits blocked
			val3=skill_get_blewcount(NJ_UTSUSEMI, val1); // knockback value.
			break;
		case SC_BUNSINJYUTSU:
			val2=(val1+1)/2; // Number of hits blocked
			break;
		case SC_CHANGE:
			val2= 30*val1; // Vit increase
			val3= 20*val1; // Int increase
			break;
		case SC_SWOO:
			if(status_has_mode(status,MD_STATUS_IMMUNE))
				tick /= 5; // !TODO: Reduce skill's duration. But for how long?
			break;
		case SC_ARMOR:
			// NPC_DEFENDER:
			val2 = 80; // Damage reduction
			// Attack requirements to be blocked:
			val3 = BF_LONG; // Range
			val4 = BF_WEAPON|BF_MISC; // Type
			break;
		case SC_ENCHANTARMS:
			// end previous enchants
			skill_enchant_elemental_end(bl,type);
			// Make sure the received element is valid.
			if (val2 >= ELE_ALL)
				val2 = val2%ELE_ALL;
			else if (val2 < 0)
				val2 = rnd()%ELE_ALL;
			break;
		case SC_CRITICALWOUND:
			val2 = 20*val1; // Heal effectiveness decrease
			break;
		case SC_MAGICMIRROR:
			// Level 1 ~ 5 & 6 ~ 10 has different duration
			// Level 6 ~ 10 use effect of level 1 ~ 5
			val1 = 1 + ((val1-1)%5);
		case SC_SLOWCAST:
			val2 = 20*val1; // Magic reflection/cast rate
			break;

		case SC_ARMORCHANGE:
			if (val2 == NPC_ANTIMAGIC) { // Boost mdef
				val2 =-20;
				val3 = 20;
			} else { // Boost def
				val2 = 20;
				val3 =-20;
			}
			// Level 1 ~ 5 & 6 ~ 10 has different duration
			// Level 6 ~ 10 use effect of level 1 ~ 5
			val1 = 1 + ((val1-1)%5);
			val2 *= val1; // 20% per level
			val3 *= val1;
			break;
		case SC_EXPBOOST:
		case SC_JEXPBOOST:
		case SC_JP_EVENT04:
			if (val1 < 0)
				val1 = 0;
			break;
		case SC_INCFLEE2:
		case SC_INCCRI:
			val2 = val1*10; // Actual boost (since 100% = 1000)
			break;
		case SC_SUFFRAGIUM:
			val2 = 15 * val1; // Speed cast decrease
			break;
		case SC_INCHEALRATE:
			if (val1 < 1)
				val1 = 1;
			break;
		case SC_DOUBLECAST:
			val2 = 30+10*val1; // Trigger rate
			break;
		case SC_KAIZEL:
			val2 = 10*val1; // % of life to be revived with
			break;
		// case SC_ARMOR_ELEMENT_WATER:
		// case SC_ARMOR_ELEMENT_EARTH:
		// case SC_ARMOR_ELEMENT_FIRE:
		// case SC_ARMOR_ELEMENT_WIND:
		// case SC_ARMOR_RESIST:
			// Mod your resistance against elements:
			// val1 = water | val2 = earth | val3 = fire | val4 = wind
			// break;
		// case ????:
			// Place here SCs that have no SCB_* data, no skill associated, no ICON
			// associated, and yet are not wrong/unknown. [Skotlex]
			// break;

		case SC_MERC_FLEEUP:
		case SC_MERC_ATKUP:
		case SC_MERC_HITUP:
			val2 = 15 * val1;
			break;
		case SC_MERC_HPUP:
		case SC_MERC_SPUP:
			val2 = 5 * val1;
			break;
		case SC_REBIRTH:
			val2 = 20*val1; // % of life to be revived with
			break;

		case SC_MANU_DEF:
		case SC_MANU_ATK:
		case SC_MANU_MATK:
			val2 = 1; // Manuk group
			break;
		case SC_SPL_DEF:
		case SC_SPL_ATK:
		case SC_SPL_MATK:
			val2 = 2; // Splendide group
			break;

		/* General */
		case SC_FEAR:
			status_change_start(src,bl,SC_ANKLE,10000,val1,0,0,0,2000,SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			break;

		/* Rune Knight */
		case SC_DEATHBOUND:
			val2 = 500 + 100 * val1;
			break;
		case SC_STONEHARDSKIN:
			if (!status_charge(bl, status->hp / 5, 0)) // 20% of HP
				return 0;
			if (sd)
				val1 = sd->status.job_level * pc_checkskill(sd, RK_RUNEMASTERY) / 4; // DEF/MDEF Increase
			break;
		case SC_REFRESH:
			status_heal(bl, status_get_max_hp(bl) * 25 / 100, 0, 1);
			status_change_clear_buffs(bl, SCCB_REFRESH);
			break;
		case SC_MILLENNIUMSHIELD:
			{
				int8 chance = rnd()%100;

				val2 = ((chance < 20) ? 4 : (chance < 50) ? 3 : 2); // Shield count
				val3 = 1000; // Shield HP
				clif_millenniumshield(bl, val2);
			}
 			break;
		case SC_ABUNDANCE:
			val4 = tick / 10000;
			t_tickime = 10000; // [GodLesZ] tick time
			break;
		case SC_GIANTGROWTH:
			val2 = 15; // Double damage success rate.
			break;

		/* Arch Bishop */
		case SC_RENOVATIO:
			val4 = tick / 5000;
			t_tickime = 5000;
			break;
		case SC_SECRAMENT:
			val2 = 10 * val1;
			break;
		case SC_VENOMIMPRESS:
			val2 = 10 * val1;
			break;
		case SC_WEAPONBLOCKING:
			val2 = 10 + 2 * val1; // Chance
			val4 = tick / 5000;
			t_tickime = 5000; // [GodLesZ] tick time
			break;
		case SC_OBLIVIONCURSE:
			val4 = tick / 3000;
			t_tickime = 3000; // [GodLesZ] tick time
			break;
		case SC_CLOAKINGEXCEED:
			val2 = (val1 + 1) / 2; // Hits
			val3 = (val1 - 1) * 10; // Walk speed
			if (bl->type == BL_PC)
				val4 |= battle_config.pc_cloak_check_type&7;
			else
				val4 |= battle_config.monster_cloak_check_type&7;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_HALLUCINATIONWALK:
			val2 = 50 * val1; // Evasion rate of physical attacks. Flee
			val3 = 10 * val1; // Evasion rate of magical attacks.
			break;
		case SC_MARSHOFABYSS:
			if( bl->type == BL_PC )
				val2 = 3 * val1; // AGI and DEX Reduction
			else // BL_MOB
				val2 = 6 * val1; // AGI and DEX Reduction
			val3 = 10 * val1; // Movement Speed Reduction
			break;
		case SC_FREEZE_SP:
			// val2 = sp drain per 10 seconds
			t_tickime = 10000; // [GodLesZ] tick time
			break;
		case SC_SPHERE_1:
		case SC_SPHERE_2:
		case SC_SPHERE_3:
		case SC_SPHERE_4:
		case SC_SPHERE_5:
			if( !sd )
				return 0;	// Should only work on players.
			val4 = tick / 1000;
			if( val4 < 1 )
				val4 = 1;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_SHAPESHIFT:
			switch( val1 ) {
				case 1: val2 = ELE_FIRE; break;
				case 2: val2 = ELE_EARTH; break;
				case 3: val2 = ELE_WIND; break;
				case 4: val2 = ELE_WATER; break;
			}
			break;
		case SC_ELECTRICSHOCKER:
		case SC_CRYSTALIZE:
			val4 = tick / 1000;
			if( val4 < 1 )
				val4 = 1;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_MEIKYOUSISUI:
			val2 = val1 * 2; // % HP each sec
			val3 = val1; // % SP each sec
			val4 = tick / 1000;
			if( val4 < 1 )
				val4 = 1;
			t_tickime = 1000;
			break;
		case SC_CAMOUFLAGE:
			val4 = tick/1000;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_WUGDASH:
		{
			//Store time at which you started running.
			t_tick currenttick = gettick();
			// Note: this int64 value is stored in two separate int32 variables (FIXME)
			val3 = (int)(currenttick&0x00000000ffffffffLL);
			val4 = (int)((currenttick&0xffffffff00000000LL)>>32);
			tick = INFINITE_TICK;
			break;
		}
		case SC__SHADOWFORM:
			{
				struct map_session_data * s_sd = map_id2sd(val2);
				if( s_sd )
					s_sd->shadowform_id = bl->id;
				val4 = tick / 1000;
				t_tickime = 1000; // [GodLesZ] tick time
			}
			break;
		case SC__STRIPACCESSORY:
			if (!sd)
				val2 = 20;
			break;
		case SC__INVISIBILITY:
			val2 = 50 - 10 * val1; // ASPD
			val3 = 20 * val1; // CRITICAL
			val4 = tick / 1000;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC__ENERVATION:
			val2 = 20 + 10 * val1; // ATK Reduction
			if (sd) {
				pc_delspiritball(sd,sd->spiritball,0);
				pc_delspiritcharm(sd,sd->spiritcharm,sd->spiritcharm_type);
			}
			break;
		case SC__GROOMY:
			val2 = 20 + 10 * val1; // ASPD
			val3 = 20 * val1; // HIT
			if( sd ) { // Removes Animals
				if( pc_isriding(sd) ) pc_setriding(sd, 0);
				if( pc_isridingdragon(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_DRAGON);
				if( pc_iswug(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_WUG);
				if( pc_isridingwug(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_WUGRIDER);
				if( pc_isfalcon(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_FALCON);
				if( sd->status.pet_id > 0 ) pet_menu(sd, 3);
				if( hom_is_active(sd->hd) ) hom_vaporize(sd, HOM_ST_ACTIVE);
				//if( sd->md ) mercenary_delete(sd->md,3); // Are Mercenaries removed? [aleos]
			}
			break;
		case SC__LAZINESS:
			val2 = 10 + 10 * val1; // Cast Increase
			val3 = 10 * val1; // Flee Reduction
			break;
		case SC__UNLUCKY:
		{
			sc_type rand_eff; 
			switch(rnd() % 3) {
				case 1: rand_eff = SC_BLIND; break;
				case 2: rand_eff = SC_SILENCE; break;
				default: rand_eff = SC_POISON; break;
			}
			val2 = 10 * val1; // Crit and Flee2 Reduction
			status_change_start(src,bl,rand_eff,10000,val1,0,(rand_eff == SC_POISON ? src->id : 0),0,tick,SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			break;
		}
		case SC__WEAKNESS:
			val2 = 10 * val1;
			// Bypasses coating protection and MADO
			sc_start(src,bl,SC_STRIPWEAPON,100,val1,tick);
			sc_start(src,bl,SC_STRIPSHIELD,100,val1,tick);
			break;
		case SC_GN_CARTBOOST:
			if( val1 < 3 )
				val2 = 50;
			else if( val1 > 2 && val1 < 5 )
				val2 = 75;
			else
				val2 = 100;
			break;
		case SC_PROPERTYWALK:
			val3 = 0;
			break;
		case SC_STRIKING:
			// val2 = watk bonus already calc
			val3 = 6 - val1;// spcost = 6 - level (lvl1:5 ... lvl 5: 1)
			val4 = tick / 1000;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_WARMER:
			val4 = tick / 3000;
			tick = INFINITE_TICK; // Duration sent to the client should be infinite
			t_tickime = 3000;
			break;
		case SC_BLOODSUCKER:
			val4 = tick / 1000;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_SWINGDANCE:
			val3 = 5 * val1 + val2; // Walk speed and aspd reduction.
			break;
		case SC_SYMPHONYOFLOVER:
			val3 = 12 * val1 + val2 + (sd?sd->status.job_level:50) / 4; // MDEF Increase in %
			break;
		case SC_MOONLITSERENADE: // MATK Increase
		case SC_RUSHWINDMILL: // ATK Increase
			val3 = 6 * val1 + val2 + (sd?sd->status.job_level:50) / 5;
			break;
		case SC_ECHOSONG:
			val3 = 6 * val1 + val2 + (sd?sd->status.job_level:50) / 4; // DEF Increase in %
			break;
		case SC_HARMONIZE:
			val2 = 5 + 5 * val1;
			break;
		case SC_VOICEOFSIREN:
			val4 = tick / 2000;
			t_tickime = 2000; // [GodLesZ] tick time
			break;
		case SC_DEEPSLEEP:
			val4 = tick / 2000;
			t_tickime = 2000; // [GodLesZ] tick time
			break;
		case SC_SIRCLEOFNATURE:
			val2 = 40 * val1; // HP recovery
			val3 = 4 * val1;	// SP consume
			val4 = tick / 1000;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_SONGOFMANA:
			val3 = 10 + min(5 * val2, 35);
			val4 = tick/5000;
			t_tickime = 5000; // [GodLesZ] tick time
			break;
		case SC_SATURDAYNIGHTFEVER:
			if (!val4) val4 = skill_get_time2(scdb->skill_id,val1);
			if (!val4) val4 = 3000;
			val3 = tick/val4;
			t_tickime = val4; // [GodLesZ] tick time
			break;
		case SC_GLOOMYDAY:
			val2 = 20 + 5 * val1; // Flee reduction.
			val3 = 15 + 5 * val1; // ASPD reduction.
			if( sd && rnd()%100 < val1 ) { // (Skill Lv) %
				val4 = 1; // Reduce walk speed by half.
				if( pc_isriding(sd) ) pc_setriding(sd, 0);
				if( pc_isridingdragon(sd) ) pc_setoption(sd, sd->sc.option&~OPTION_DRAGON);
			}
			break;
		case SC_GLOOMYDAY_SK:
			// Random number between [15 ~ (Voice Lesson Skill Level x 5) + (Skill Level x 10)] %.
			val2 = 15 + rnd()%( (sd?pc_checkskill(sd, WM_LESSON)*5:0) + val1*10 );
			break;
		case SC_SITDOWN_FORCE:
		case SC_BANANA_BOMB_SITDOWN:
			if( sd && !pc_issit(sd) ) {
				pc_setsit(sd);
				skill_sit(sd, 1);
				clif_sitting(bl);
			}
			break;
		case SC_DANCEWITHWUG:
			val3 = 5 + 5 * val2; // ASPD Increase
			val4 = 20 + 10 * val2; // Fixed Cast Time Reduction
			break;
		case SC_LERADSDEW:
			val3 = 200 * val1 + min(300 * val2, 2500); // MaxHP Increase
			break;
		case SC_MELODYOFSINK:
			val3 = val1 * val2; // INT Reduction.
			val4 = tick/1000;
			t_tickime = 1000;
			break;
		case SC_BEYONDOFWARCRY:
			val3 = val1 * val2; // STR and CRIT Reduction
			val4 = 4 * val1 + min(4 * (val2 - 2), 40); // MaxHP Reduction
			break;
		case SC_UNLIMITEDHUMMINGVOICE:
			{
				struct unit_data *ud = unit_bl2ud(bl);
				if( ud == NULL ) return 0;
				ud->state.skillcastcancel = 0;
				val3 = 15 - min(3 * val2, 15);
			}
			break;
		case SC_REFLECTDAMAGE:
			val2 = 15 + 5 * val1; // Reflect amount
			val3 = val1*5 + 25; // Number of reflects
			val4 = tick/1000; // Number of SP cycles (duration)
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_FORCEOFVANGUARD:
			val2 = 8 + 12 * val1; // Chance
			val3 = 5 + 2 * val1; // Max rage counters
			tick = INFINITE_TICK; // Endless duration in the client
			t_tickime = 10000; // [GodLesZ] tick time
			break;
		case SC_EXEEDBREAK:
			val2 = 150 * val1;
			if (sd) { // Players
				short index = sd->equip_index[EQI_HAND_R];

				if (index >= 0 && sd->inventory_data[index] && sd->inventory_data[index]->type == IT_WEAPON)
					val2 += 15 * sd->status.job_level + sd->inventory_data[index]->weight / 10 * sd->inventory_data[index]->wlv * status_get_lv(bl) / 100;
			} else // Monster
				val2 += 750;
			break;
		case SC_PRESTIGE:
			val2 = (status->int_ + status->luk) * val1 / 20 * status_get_lv(bl) / 200 + val1;	// Chance to evade magic damage.
			val3 = ((val1 * 15) + (10 * (sd?pc_checkskill(sd,CR_DEFENDER):skill_get_max(CR_DEFENDER)))) * status_get_lv(bl) / 100; // Defence added
			break;
		case SC_BANDING:
			val2 = (sd ? skill_banding_count(sd) : 1);
			t_tickime = 5000; // [GodLesZ] tick time
			break;
		case SC_MAGNETICFIELD:
			val3 = tick / 1000;
			t_tickime = 1000; // [GodLesZ] tick time
			break;
		case SC_INSPIRATION:
			val2 = (sd?sd->status.job_level:50);
			val3 = status_get_lv(bl) / 10 + val2 / 5; //All stat bonus
			val4 = tick / 5000;
			t_tickime = 5000; // [GodLesZ] tick time
			status_change_clear_buffs(bl, SCCB_BUFFS|SCCB_DEBUFFS); // Remove buffs/debuffs
			break;
		case SC_CRESCENTELBOW:
			val2 = (sd?sd->status.job_level:50) / 2 + (50 + 5 * val1);
			break;
		case SC_LIGHTNINGWALK: // [(Job Level / 2) + (40 + 5 * Skill Level)] %
			val1 = (sd?sd->status.job_level:2)/2 + 40 + 5 * val1;
			break;
		case SC_RAISINGDRAGON:
			val3 = tick / 5000;
			t_tickime = 5000; // [GodLesZ] tick time
			break;
		case SC_GT_ENERGYGAIN:
			val2 = 10 + 5 * val1; // Sphere gain chance.
			break;
		case SC_GT_CHANGE:
			{ // Take note there is no def increase as skill desc says. [malufett]
				int stat = status_get_int(src);

				if (stat <= 0)
					stat = 1; // Prevent divide by zero.
				val2 = (status_get_dex(src) / 4 + status_get_str(src) / 2) * val1 / 5; // ATK increase: ATK [{(Caster DEX / 4) + (Caster STR / 2)} x Skill Level / 5]
				val3 = status->agi * val1 / 60; // ASPD increase: [(Target AGI x Skill Level) / 60] %
				val4 = 200 / stat * val1; // MDEF decrease: MDEF [(200 / Caster INT) x Skill Level]
			}
			break;
		case SC_GT_REVITALIZE:
			// Take note there is no vit, aspd, speed increase as skill desc says. [malufett]
			val2 = 2 * val1; // MaxHP: [(Skill Level * 2)]%
			val3 = val1 * 30 + 50; // Natural HP recovery increase: [(Skill Level x 30) + 50] %
			// The stat def is not shown in the status window and it is processed differently
			val4 = status_get_vit(src) / 4 * val1; // STAT DEF increase: [(Caster VIT / 4) x Skill Level]
			break;
		case SC_PYROTECHNIC_OPTION:
			val2 = 60; // Eatk Renewal (Atk2)
			break;
		case SC_HEATER_OPTION:
			val2 = 120; // Eatk Renewal (Atk2)
			val3 = ELE_FIRE; // Change into fire element.
			break;
		case SC_TROPIC_OPTION:
			val2 = 180; // Eatk Renewal (Atk2)
			val3 = MG_FIREBOLT;
			break;
		case SC_AQUAPLAY_OPTION:
			val2 = 40;
			break;
		case SC_COOLER_OPTION:
			val2 = 80;
			val3 = ELE_WATER; // Change into water element.
			break;
		case SC_CHILLY_AIR_OPTION:
			val2 = 120; // Matk. Renewal (Matk1)
			val3 = MG_COLDBOLT;
			break;
		case SC_WIND_STEP_OPTION:
			val2 = 50; // % Increase speed and flee.
			break;
		case SC_BLAST_OPTION:
			val2 = 20;
			val3 = ELE_WIND; // Change into wind element.
			break;
		case SC_WILD_STORM_OPTION:
			val2 = MG_LIGHTNINGBOLT;
			break;
		case SC_PETROLOGY_OPTION:
			val2 = 5; //HP Rate bonus
			val3 = 50;
			break;
		case SC_SOLID_SKIN_OPTION:
			val2 = 33; //% Increase DEF
			break;
		case SC_CURSED_SOIL_OPTION:
			val2 = 10; //HP rate bonus
			val3 = ELE_EARTH; // Change into earth element.
			break;
		case SC_UPHEAVAL_OPTION:
			val2 = 15; //HP rate bonus
			val3 = WZ_EARTHSPIKE;
			break;
		case SC_CIRCLE_OF_FIRE_OPTION:
			val2 = 300;
			break;
		case SC_WATER_SCREEN_OPTION:
			t_tickime = 10000;
			break;
		case SC_FIRE_CLOAK_OPTION:
		case SC_WATER_DROP_OPTION:
		case SC_WIND_CURTAIN_OPTION:
		case SC_STONE_SHIELD_OPTION:
			val2 = 100; // Elemental modifier.
			break;
		case SC_TROPIC:
		case SC_CHILLY_AIR:
		case SC_WILD_STORM:
		case SC_UPHEAVAL:
			val2 += 10;
		case SC_HEATER:
		case SC_COOLER:
		case SC_BLAST:
		case SC_CURSED_SOIL:
			val2 += 10;
		case SC_PYROTECHNIC:
		case SC_AQUAPLAY:
		case SC_GUST:
		case SC_PETROLOGY:
			val2 += 5;
			val3 += 9000;
		case SC_CIRCLE_OF_FIRE:
		case SC_FIRE_CLOAK:
		case SC_WATER_DROP:
		case SC_WATER_SCREEN:
		case SC_WIND_CURTAIN:
		case SC_WIND_STEP:
		case SC_STONE_SHIELD:
		case SC_SOLID_SKIN:
			val2 += 5;
			val3 += 1000;
			t_tickime = val3; // [GodLesZ] tick time
			break;
		case SC_WATER_BARRIER:
			val2 = 30; // Reductions. Atk2 and Flee1
			break;
		case SC_ZEPHYR:
			val2 = 25; // Flee.
			break;
		case SC_TIDAL_WEAPON:
			val2 = 20; // Increase Elemental's attack.
			break;
		case SC_ROCK_CRUSHER:
		case SC_ROCK_CRUSHER_ATK:
		case SC_POWER_OF_GAIA:
			val2 = 33; //Def rate bonus/Speed rate reduction
			val3 = 20; //HP rate bonus
			break;
		case SC_TEARGAS:
			val2 = status_get_max_hp(bl) * 5 / 100; // Drain 5% HP
			val4 = tick / 2000;
			t_tickime = 2000;
			break;
		case SC_TEARGAS_SOB:
			val4 = tick / 3000;
			t_tickime = 3000;
			break;
		case SC_STOMACHACHE:
			val2 = 8; // SP consume.
			val4 = tick / 10000;
			t_tickime = 10000; // [GodLesZ] tick time
			break;
		case SC_PROMOTE_HEALTH_RESERCH:
			//val1: 1 = Regular Potion, 2 = Thrown Potion
			//val2: 1 = Small Potion, 2 = Medium Potion, 3 = Large Potion
			//val3: BaseLV of Thrower For Thrown Potions
			//val4: MaxHP Increase By Fixed Amount
			if (val1 == 1) // If potion was normally used, take the user's BaseLv
				val4 = 1000 * val2 - 500 + status_get_lv(bl) * 10 / 3;
			else if (val1 == 2) // If potion was thrown at someone, take the thrower's BaseLv
				val4 = 1000 * val2 - 500 + val3 * 10 / 3;
			if (val4 <= 0) // Prevents a negeative value from happening
				val4 = 0;
			break;
		case SC_ENERGY_DRINK_RESERCH:
			//val1: 1 = Regular Potion, 2 = Thrown Potion
			//val2: 1 = Small Potion, 2 = Medium Potion, 3 = Large Potion
			//val3: BaseLV of Thrower For Thrown Potions
			//val4: MaxSP Increase By Percentage Amount
			if (val1 == 1) // If potion was normally used, take the user's BaseLv
				val4 = status_get_lv(bl) / 10 + 5 * val2 - 10;
			else if (val1 == 2) // If potion was thrown at someone, take the thrower's BaseLv
				val4 = val3 / 10 + 5 * val2 - 10;
			if (val4 <= 0) // Prevents a negeative value from happening
				val4 = 0;
			break;
		case SC_KYOUGAKU:
			val2 = 2*val1 + rnd()%val1;
			clif_status_change(bl,EFST_ACTIVE_MONSTER_TRANSFORM,1,0,1002,0,0);
			break;
		case SC_KAGEMUSYA:
			val2 = tick/1000;
			val3 = val1 * 2;
			t_tickime = 1000;
			break;
		case SC_ZANGETSU:
			if( status_get_hp(bl) % 2 == 0 )
				val2 = (status_get_lv(bl) / 3) + (20 * val1); //+Watk
			else
				val2 -= (status_get_lv(bl) / 3) + (30 * val1); //-Watk

			if( status_get_sp(bl) % 2 == 0 )
				val3 = (status_get_lv(bl) / 3) + (20 * val1); //+Matk
			else
				val3 -= (status_get_lv(bl) / 3) + (30 * val1); //-Matk
			break;
		case SC_GENSOU:
			{
				int hp = status_get_hp(bl), lv = 5;
				short per = 100 / (status_get_max_hp(bl) / hp);

				if( per <= 15 )
					lv = 1;
				else if( per <= 30 )
					lv = 2;
				else if( per <= 50 )
					lv = 3;
				else if( per <= 75 )
					lv = 4;
				if( hp % 2 == 0)
					status_heal(bl, hp * (6-lv) * 4 / 100, status_get_sp(bl) * (6-lv) * 3 / 100, 1);
				else
					status_zap(bl, hp * (lv*4) / 100, status_get_sp(bl) * (lv*3) / 100);
			}
			break;
		case SC_ANGRIFFS_MODUS:
			val2 = 50 + 20 * val1; // atk bonus
			val3 = 40 + 20 * val1; // Flee reduction.
			val4 = tick/1000; // hp/sp reduction timer
			t_tickime = 1000;
			break;
		case SC_GOLDENE_FERSE:
			val2 = 10 + 10*val1; // flee bonus
			val3 = 6 + 4 * val1; // Aspd Bonus
			val4 = 2 + 2 * val1; // Chance of holy attack
			break;
		case SC_OVERED_BOOST:
			val2 = 300 + 40*val1; // flee bonus
			val3 = 179 + 2*val1; // aspd bonus
			val4 = 50; // def reduc %
			break;
		case SC_GRANITIC_ARMOR:
			val2 = 2*val1; // dmg reduction
			val3 = 6*val1; // dmg taken on status end (6%:12%:18%:24%:30%)
			val4 = 5*val1; // unknow formula
			break;
		case SC_MAGMA_FLOW:
			val2 = 3*val1; // Activation chance
			break;
		case SC_PYROCLASTIC:
			val2 += 10*val1; // atk bonus
			val3 = 2*val1; // Chance To AutoCast Hammer Fall %
			break;
		case SC_PARALYSIS: // [Lighta] need real info
			val2 = 2*val1; // def reduction
			val3 = 500*val1; // varcast augmentation
			break;
		case SC_LIGHT_OF_REGENE: // Yommy leak need confirm
			val2 = 20 * val1; // hp reco on death %
			break;
		case SC_PAIN_KILLER: // Yommy leak need confirm
			val2 = 10 * val1; // aspd reduction %
			val3 = min((( 200 * val1 ) * status_get_lv(src)) / 150, 1000); // dmg reduction linear. upto a maximum of 1000 [iRO Wiki]
			if(sc->data[SC_PARALYSIS])
				sc_start(src,bl, SC_ENDURE, 100, val1, tick); // Start endure for same duration
			break;
		case SC_STYLE_CHANGE:
			tick = INFINITE_TICK; // Infinite duration
			break;
		case SC_CBC:
			val3 = 10; // Drain sp % dmg
			val4 = tick/1000; // dmg each sec
			tick = 1000;
			break;
		case SC_EQC:
			val2 = 5 * val1; // def % reduc
			val3 = 2 * val1; // HP drain %
			break;
		case SC_ASH:
			val2 = 0; // hit % reduc
			val3 = 0; // def % reduc
			val4 = 0; // atk flee & reduc
			if (!status_bl_has_mode(bl,MD_STATUS_IMMUNE)) {
				val2 = 50;
				if (status_get_race(bl) == RC_PLANT) // plant type
					val3 = 50;
				if (status_get_element(bl) == ELE_WATER) // defense water type
					val4 = 50;
			}
			break;
		case SC_FULL_THROTTLE:
			val2 = ( val1 == 1 ? 6 : 6 - val1 );
			val3 = 20; //+% AllStats
			t_tickime = 1000;
			val4 = tick / t_tickime;
			break;
		case SC_REBOUND:
			t_tickime = 2000;
			val4 = tick / t_tickime;
			clif_emotion(bl, ET_SWEAT);
			break;
		case SC_KINGS_GRACE:
			val2 = 3 + val1; //HP Recover rate
			t_tickime = 1000;
			val4 = tick / t_tickime;
			break;
		case SC_TELEKINESIS_INTENSE:
			val2 = 10 * val1; // sp consum / casttime reduc %
			val3 = 40 * val1; // magic dmg bonus
			break;
		case SC_OFFERTORIUM:
			val2 = 30 * val1; // heal power bonus
			val3 = 100 + 20 * val1; // sp cost inc
			break;
		case SC_FRIGG_SONG:
			val2 = 5 * val1; // maxhp bonus
			val3 = 80 + 20 * val1; // healing
			t_tickime = 1000;
			val4 = tick / t_tickime;
			break;
		case SC_FLASHCOMBO:
			val2 = 20 * val1 + 20; // atk bonus
			break;
		case SC_DARKCROW:
			val2 = 30 * val1;
			break;
		case SC_UNLIMIT:
			val2 = 50 * val1;
			status_change_start(bl, bl, SC_DEFSET, 10000, 1, 0, 0, 0, tick, SCSTART_NOTICKDEF);
			status_change_start(bl, bl, SC_MDEFSET, 10000, 1, 0, 0, 0, tick, SCSTART_NOTICKDEF);
			break;
		case SC_MONSTER_TRANSFORM:
		case SC_ACTIVE_MONSTER_TRANSFORM:
			if( !mobdb_checkid(val1) )
				val1 = MOBID_PORING; // Default poring
			break;
		case SC_APPLEIDUN:
		{
			struct map_session_data * s_sd = BL_CAST(BL_PC, src);
			val2 = (5 + 2 * val1) + (status_get_vit(src) / 10); //HP Rate: (5 + 2 * skill_lv) + (vit/10) + (BA_MUSICALLESSON level)
			if (s_sd)
				val2 += pc_checkskill(s_sd, BA_MUSICALLESSON) / 2;
			break;
		}
		case SC_EPICLESIS:
			val2 = 5 * val1; //HP rate bonus
			break;
		case SC_ILLUSIONDOPING:
			val2 = 50; // -Hit
			break;

		case SC_OVERHEAT:
		case SC_OVERHEAT_LIMITPOINT:
		case SC_STEALTHFIELD:
			t_tickime = tick;
			tick = INFINITE_TICK;
			break;
		case SC_STEALTHFIELD_MASTER:
			t_tickime = val3 = 2000 + 1000 * val1;
			val4 = tick / t_tickime;
			break;
		case SC_VACUUM_EXTREME:
			// Suck target at n second, only if the n second is lower than the duration
			// Doesn't apply to BL_PC
			if (bl->type != BL_PC && val4 < tick && !unit_blown_immune(bl,0x1) && status_has_mode(status,MD_CANMOVE)) {
				t_tickime = val4;
				val4 = tick - t_tickime;
			}
			else
				val4 = 0;
			break;
		case SC_FIRE_INSIGNIA:
		case SC_WATER_INSIGNIA:
		case SC_WIND_INSIGNIA:
		case SC_EARTH_INSIGNIA:
			t_tickime = 5000;
			val4 = tick / t_tickime;
			break;
		case SC_NEUTRALBARRIER:
			val2 = 10 + val1 * 5; // Def/Mdef
			tick = INFINITE_TICK;
			break;

		/* Rebellion */
		case SC_B_TRAP:
			val2 = src->id;
			val3 = val1 * 25; // -movespeed TODO: Figure out movespeed rate
			break;
		case SC_C_MARKER:
			// val1 = skill_lv
			// val2 = src_id
			val3 = 10; // -10 flee
			//Start timer to send mark on mini map
			val4 = tick/1000;
			t_tickime = 1000; // Sends every 1 seconds
			break;
		case SC_H_MINE:
			val2 = src->id;
			break;
		case SC_HEAT_BARREL:
			{
				uint8 n = 10;
				if (sd)
					n = (uint8)sd->spiritball_old;

				//kRO Update 2016-05-25
				val2 = n * 5; // -fixed casttime
				val3 = (6 + val1 * 2) * n; // ATK
				val4 = 25 + val1 * 5; // -hit
			}
			break;
		case SC_P_ALTER:
			{
				uint8 n = 10;
				if (sd)
					n = (uint8)sd->spiritball_old;
				val2 = 10 * n; // +atk
				val3 = (status->max_hp * (val1 * 5) / 100); // Barrier HP
			}
			break;
		case SC_E_CHAIN:
			val2 = 10;
			if (sd)
				val2 = sd->spiritball_old;
			break;
		case SC_ANTI_M_BLAST:
			val2 = val1 * 10;
			break;
		case SC_CATNIPPOWDER:
			val2 = 50; // WATK%, MATK%
			val3 = 25 * val1; // Move speed reduction
			if (bl->type == BL_PC && pc_checkskill(sd, SU_SPIRITOFLAND))
				val4 = status_get_lv(src) / 12;
			break;
		case SC_BITESCAR: {
				const struct status_data *b_status = status_get_base_status(src); // Base Status

				val2 = (status_get_max_hp(bl) * (val1 + (b_status->dex / 25))) / status_get_max_hp(bl); // MHP% damage
				t_tickime = 1000;
				val4 = tick / t_tickime;
			}
			break;
		case SC_ARCLOUSEDASH:
			val2 = 15 + 5 * val1; // AGI
			val3 = 25; // Move speed increase
			if (sd && (sd->class_&MAPID_BASEMASK) == MAPID_SUMMONER)
				val4 = 10; // Ranged ATK increase
			break;
		case SC_SHRIMP:
			val2 = 10; // BATK%, MATK%
			break;
		case SC_FRESHSHRIMP: {
				int min = 0, max = 0;

#ifdef RENEWAL
				min = status_base_matk_min(src, status, status_get_lv(src));
				max = status_base_matk_max(src, status, status_get_lv(src));
				if (status->rhw.matk > 0) {
					int wMatk, variance;

					wMatk = status->rhw.matk;
					variance = wMatk * status->rhw.wlv / 10;
					min += wMatk - variance;
					max += wMatk + variance;
				}
#endif

				if (sd && sd->right_weapon.overrefine > 0) {
					min++;
					max += sd->right_weapon.overrefine - 1;
				}

				val2 += min + 178; // Heal
				if (max > min)
					val2 += rnd() % (max - min); // Heal

				if (sd) {
					if (pc_checkskill(sd, SU_POWEROFSEA)) {
						val2 += val2 * 10 / 100;
						if ((pc_checkskill(sd, SU_TUNABELLY) + pc_checkskill(sd, SU_TUNAPARTY) + pc_checkskill(sd, SU_BUNCHOFSHRIMP) + pc_checkskill(sd, SU_FRESHSHRIMP) +
							pc_checkskill(sd, SU_GROOMING) + pc_checkskill(sd, SU_PURRING) + pc_checkskill(sd, SU_SHRIMPARTY)) > 19)
								val2 += val2 * 20 / 100;
					}
					if (pc_checkskill(sd, SU_SPIRITOFSEA))
						val2 *= 2; // Doubles HP
				}
				t_tickime = 10000 - ((val1 - 1) * 1000);
				val4 = tick / t_tickime;
			}
			break;
		case SC_TUNAPARTY:
			val2 = (status->max_hp * (val1 * 10) / 100); // Max HP% to absorb
			if (sd && pc_checkskill(sd, SU_SPIRITOFSEA))
				val2 <<= 1; // Double the shield life
			break;
		case SC_HISS:
			val2 = 50; // Perfect Dodge
			sc_start(src, bl, SC_DORAM_WALKSPEED, 100, 50, skill_get_time2(SU_HISS, val1));
			break;
		case SC_GROOMING:
			val2 = 100; // Flee
			break;
		case SC_CHATTERING:
			val2 = 100; // eATK, eMATK
			sc_start(src, bl, SC_DORAM_WALKSPEED, 100, 50, skill_get_time2(SU_CHATTERING, val1));
			break;
		case SC_SWORDCLAN:
		case SC_ARCWANDCLAN:
		case SC_GOLDENMACECLAN:
		case SC_CROSSBOWCLAN:
		case SC_JUMPINGCLAN:
			tick = INFINITE_TICK;
			status_change_start(src,bl,SC_CLAN_INFO,10000,0,val2,0,0,INFINITE_TICK,flag);
			break;
		case SC_DORAM_BUF_01:
		case SC_DORAM_BUF_02:
			t_tickime = 10000; // every 10 seconds
			if( (val4 = tick/t_tickime) < 1 )
				val4 = 1;
			break;

		case SC_GLASTHEIM_ATK:
			val1 = 100; // Undead/Demon MDEF ignore rate
			break;
		case SC_GLASTHEIM_HEAL:
			val1 = 100; // Heal Power rate bonus
			val2 = 50; // Received heal rate bonus
			break;
		case SC_GLASTHEIM_HIDDEN:
			val1 = 90; // Damage rate reduction bonus
			break;
		case SC_GLASTHEIM_STATE:
			val1 = 20; // All-stat bonus
			break;
		case SC_GLASTHEIM_ITEMDEF:
			val1 = 200; // DEF bonus
			val2 = 50; // MDEF bonus
			break;
		case SC_GLASTHEIM_HPSP:
			val1 = 10000; // HP bonus
			val2 = 1000; // SP bonus
			break;
		case SC_ANCILLA:
			val1 = 15; // Heal Power rate bonus
			val2 = 30; // SP Recovery rate bonus
			break;
		//default:
		//	if( calc_flag == SCB_NONE && scdb->skill_id <= 0 && scdb->icon == EFST_BLANK ) {
		//		// Status change with no calc, no icon, and no skill associated...?
		//		ShowError("status_change_start: Unknown SC %d\n", type);
		//		return 0;
		//	}
	} else // Special considerations when loading SC data.
		switch( type ) {
			case SC_WEDDING:
			case SC_XMAS:
			case SC_SUMMER:
			case SC_HANBOK:
			case SC_OKTOBERFEST:
			case SC_DRESSUP:
				if( !vd )
					break;
				clif_changelook(bl,LOOK_BASE,vd->class_);
				clif_changelook(bl,LOOK_WEAPON,0);
				clif_changelook(bl,LOOK_SHIELD,0);
				clif_changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
				clif_changelook(bl,LOOK_BODY2,0);
				break;
			case SC_STONE:
				if (val3 > 0)
					break; //Incubation time still active
				//Fall through
			case SC_POISON:
			case SC_DPOISON:
			case SC_BLEEDING:
			case SC_BURNING:
			case SC_TOXIN:
			case SC_MAGICMUSHROOM:
			case SC_PYREXIA:
			case SC_LEECHESEND:
				t_tickime = tick;
				tick = t_tickime + max(val4,0);
				break;
			case SC_SWORDCLAN:
			case SC_ARCWANDCLAN:
			case SC_GOLDENMACECLAN:
			case SC_CROSSBOWCLAN:
			case SC_JUMPINGCLAN:
			case SC_CLAN_INFO:
				// If the player still has a clan status, but was removed from his clan
				if( sd && sd->status.clan_id == 0 ){
					return 0;
				}
				break;
		}

	// Values that must be set regardless of flag&4 e.g. val_flag [Ind]
	switch(type) {
		// Start |1 val_flag setting
		case SC_ROLLINGCUTTER:
		case SC_BANDING:
		case SC_SPHERE_1:
		case SC_SPHERE_2:
		case SC_SPHERE_3:
		case SC_SPHERE_4:
		case SC_SPHERE_5:
		case SC_OVERHEAT:
		case SC_LIGHTNINGWALK:
		case SC_MONSTER_TRANSFORM:
		case SC_ACTIVE_MONSTER_TRANSFORM:
		case SC_EXPBOOST:
		case SC_JEXPBOOST:
		case SC_ITEMBOOST:
		case SC_JP_EVENT04:
		case SC_PUSH_CART:
		case SC_SWORDCLAN:
		case SC_ARCWANDCLAN:
		case SC_GOLDENMACECLAN:
		case SC_CROSSBOWCLAN:
		case SC_JUMPINGCLAN:
		case SC_DRESSUP:
			val_flag |= 1;
			break;
		// Start |1|2 val_flag setting
		case SC_FIGHTINGSPIRIT:
		case SC_VENOMIMPRESS:
		case SC_WEAPONBLOCKING:
		case SC__INVISIBILITY:
		case SC__ENERVATION:
		case SC__WEAKNESS:
		case SC_PROPERTYWALK:
		case SC_PRESTIGE:
		case SC_SHIELDSPELL_DEF:
		case SC_SHIELDSPELL_MDEF:
		case SC_SHIELDSPELL_REF:
		case SC_CRESCENTELBOW:
		case SC_CHILLY_AIR_OPTION:
		case SC_GUST_OPTION:
		case SC_WILD_STORM_OPTION:
		case SC_UPHEAVAL_OPTION:
		case SC_CIRCLE_OF_FIRE_OPTION:
		case SC_CLAN_INFO:
		case SC_DAILYSENDMAILCNT:
			val_flag |= 1|2;
			break;
		// Start |1|2|4 val_flag setting
		case SC_POISONINGWEAPON:
		case SC_CLOAKINGEXCEED:
		case SC_HALLUCINATIONWALK:
		case SC__SHADOWFORM:
		case SC__GROOMY:
		case SC__LAZINESS:
		case SC__UNLUCKY:
		case SC_FORCEOFVANGUARD:
		case SC_SPELLFIST:
		case SC_CURSEDCIRCLE_ATKER:
		case SC_PYROTECHNIC_OPTION:
		case SC_HEATER_OPTION:
		case SC_AQUAPLAY_OPTION:
		case SC_COOLER_OPTION:
		case SC_BLAST_OPTION:
		case SC_PETROLOGY_OPTION:
		case SC_CURSED_SOIL_OPTION:
		case SC_WATER_BARRIER:
			val_flag |= 1|2|4;
			break;
	}

	/* [Ind] */
	if (scdb->flag&(SCF_DISPLAY_PC|SCF_DISPLAY_NPC)) {
		int dval1 = 0, dval2 = 0, dval3 = 0;

		switch (type) {
			case SC_ALL_RIDING:
				dval1 = 1;
				break;
			case SC_CLAN_INFO:
				dval1 = val1;
				dval2 = val2;
				dval3 = val3;
				break;
			default: /* All others: just copy val1 */
				dval1 = val1;
				break;
		}
		status_display_add(bl,type,dval1,dval2,dval3);
	}

	//SC that force player to stand if is sitting
	if (scdb->flag&SCF_SET_STAND && sd && pc_issit(sd))
		pc_setstand(sd, true);

	//SC that make stop attacking [LuzZza]
	if (scdb->flag&SCF_STOP_ATTACKING)
		unit_stop_attack(bl);

	//SC that make stop walking
	if (scdb->flag&SCF_STOP_WALKING) {
		switch (type) {
			case SC_ANKLE:
				if (battle_config.skill_trap_type || !map_flag_gvg(bl->m))
					unit_stop_walking(bl, 1);
				break;
			case SC__MANHOLE:
				if (bl->type == BL_PC || !unit_blown_immune(bl,0x1))
					unit_stop_walking(bl,1);
				break;
			case SC_VACUUM_EXTREME:
				if (bl->type != BL_PC && !unit_blown_immune(bl,0x1)) {
					unit_stop_walking(bl,1);
					unit_stop_attack(bl);
				}
				break;
			case SC_WHITEIMPRISON:
			case SC_DEEPSLEEP:
			case SC_CRYSTALIZE:
			case SC_FREEZE:
			case SC_STUN:
				if (sc->data[SC_DANCING])
					unit_stop_walking(bl, 1);
				break;
			default:
				if (!unit_blown_immune(bl,0x1))
					unit_stop_walking(bl,1);
				break;
		}
	}

	//SC that make stop casting
	if (battle_config.sc_castcancel&bl->type && scdb->flag&SCF_STOP_CASTING)
		unit_skillcastcancel(bl,0);

	opt_flag = scdb->flag&SCF_OPT_FLAGS;
	sc->opt1 = scdb->opt1;
	sc->opt2 |= scdb->opt2;
	sc->opt3 |= scdb->opt3;
	sc->option |= scdb->look;

	switch(type) {
		case SC_STONE: 
			if (val3 > 0)
				sc->opt1 = OPT1_STONEWAIT;
			else
				sc->opt1 = OPT1_STONE;
			break;
		case SC_DANCING:
			if ((val1&0xFFFF) != CG_MOONLIT)
				sc->opt3 |= OPT3_MOONLIT;
			opt_flag = 0;
			break;
		case SC_INCATKRATE:
			// Simulate Explosion Spirits effect for NPC_POWERUP [Skotlex]
			if (bl->type != BL_MOB) {
				opt_flag = 0;
				break;
			}
			break;
	}

	// On Aegis, when turning on a status change, first goes the option packet, then the sc packet.
	if (scdb->flag) {
		clif_changeoption(bl);
		if(sd && (opt_flag&SCF_OPT_CHANGELOOK)) {
			clif_changelook(bl,LOOK_BASE,vd->class_);
			clif_changelook(bl,LOOK_WEAPON,0);
			clif_changelook(bl,LOOK_SHIELD,0);
			clif_changelook(bl,LOOK_CLOTHES_COLOR,vd->cloth_color);
		}
	}

	if (calc_flag&SCB_DYE) { // Reset DYE color
		if (vd && vd->cloth_color) {
			val4 = vd->cloth_color;
			clif_changelook(bl,LOOK_CLOTHES_COLOR,0);
		}
		calc_flag&=~SCB_DYE;
	}

	/*if (calc_flag&SCB_BODY)// Might be needed in the future. [Rytech]
	{	//Reset body style
		if (vd && vd->body_style)
		{
			val4 = vd->body_style;
			clif_changelook(bl,LOOK_BODY2,0);
		}
		calc_flag&=~SCB_BODY;
	}*/

	if(!(flag&SCSTART_NOICON) && !(flag&SCSTART_LOADED && scdb->flag&(SCF_DISPLAY_PC|SCF_DISPLAY_NPC)))
		clif_status_change(bl,scdb->icon,1,tick,(val_flag&1)?val1:1,(val_flag&2)?val2:0,(val_flag&4)?val3:0);

	// Used as temporary storage for scs with interval ticks, so that the actual duration is sent to the client first.
	if( t_tickime )
		tick = t_tickime;

	// Don't trust the previous sce assignment, in case the SC ended somewhere between there and here.
	if((sce=sc->data[type])) { // reuse old sc
		if( sce->timer != INVALID_TIMER )
			delete_timer(sce->timer, status_change_timer);
		sc_isnew = false;
	} else { // New sc
		++(sc->count);
		sce = sc->data[type] = ers_alloc(sc_data_ers, struct status_change_entry);
	}
	sce->val1 = val1;
	sce->val2 = val2;
	sce->val3 = val3;
	sce->val4 = val4;
	if (tick >= 0)
		sce->timer = add_timer(gettick() + tick, status_change_timer, bl->id, type);
	else
		sce->timer = INVALID_TIMER; // Infinite duration

	if (calc_flag)
		status_calc_bl(bl,calc_flag);

	// Non-zero
	// Calc state for SC_STONE when OPT1_STONE in the timer
	if (sc_isnew && scdb->state && type != SC_STONE)
		status_calc_state(bl,sc,static_cast<e_scs_flag>(scdb->state),true);

	if(sd) {
		if (sd->pd)
			pet_sc_check(sd, type); // Skotlex: Pet Status Effect Healing
		switch (type) {
			case SC_BERSERK:
			case SC_MERC_HPUP:
			case SC_MERC_SPUP:
				status_calc_pc(sd, SCO_FORCE);
				break;
			default:
				status_calc_pc(sd, SCO_NONE);
				break;
		}
	}

	// 1st thing to execute when loading status
	switch (type) {
		case SC_FULL_THROTTLE:
			status_percent_heal(bl,100,0);
			break;
		case SC_BERSERK:
			if (!(sce->val2)) { // Don't heal if already set
				status_heal(bl, status->max_hp, 0, 1); // Do not use percent_heal as this healing must override BERSERK's block.
				status_set_sp(bl, 0, 0); // Damage all SP
			}
			sce->val2 = 5 * status->max_hp / 100;
			break;
		case SC_RUN:
			{
				struct unit_data *ud = unit_bl2ud(bl);

				if( ud )
					ud->state.running = unit_run(bl, NULL, SC_RUN);
			}
			break;
		case SC_BOSSMAPINFO:
			if (sd)
				clif_bossmapinfo(sd, map_id2boss(sce->val1), BOSS_INFO_ALIVE_WITHMSG); // First Message
			break;
		case SC_MERC_HPUP:
			status_percent_heal(bl, 100, 0); // Recover Full HP
			break;
		case SC_MERC_SPUP:
			status_percent_heal(bl, 0, 100); // Recover Full SP
			break;
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit_bl2ud(bl);

				if( ud )
					ud->state.running = unit_run(bl, sd, SC_WUGDASH);
			}
			break;
		case SC_COMBO:
			switch(sce->val1) {
			case TK_STORMKICK:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,bl,TK_READYSTORM,1,1);
				break;
			case TK_DOWNKICK:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,bl,TK_READYDOWN,1,1);
				break;
			case TK_TURNKICK:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,bl,TK_READYTURN,1,1);
				break;
			case TK_COUNTER:
				skill_combo_toggle_inf(bl, TK_JUMPKICK, 0);
				clif_skill_nodamage(bl,bl,TK_READYCOUNTER,1,1);
				break;
			default: // Rest just toggle inf to enable autotarget
				skill_combo_toggle_inf(bl,sce->val1,INF_SELF_SKILL);
				break;
			}
			break;
		case SC_RAISINGDRAGON:
			sce->val2 = status->max_hp / 100; // Officially tested its 1%hp drain. [Jobbie]
			break;
		case SC_C_MARKER:
			//Send mini-map, don't wait for first timer triggered
			if (src->type == BL_PC && (sd = map_id2sd(src->id)))
				clif_crimson_marker(sd, bl, false);
			break;
		case SC_ITEMSCRIPT: // Shows Buff Icons
			if (sd && val2 != EFST_BLANK)
				clif_status_change(bl, (efst_type)val2, 1, tick, 0, 0, 0);
			break;
		case SC_GVG_GIANT:
		case SC_GVG_GOLEM:
		case SC_GVG_STUN:
		case SC_GVG_STONE:
		case SC_GVG_FREEZ:
		case SC_GVG_SLEEP:
		case SC_GVG_CURSE:
		case SC_GVG_SILENCE:
		case SC_GVG_BLIND:
			if (val1 || val2)
				status_zap(bl, val1 ? val1 : 0, val2 ? val2 : 0);
			break;
	}

	if( scdb->flag&SCF_TRIGGER_ONTOUCH_ && sd && sd->touching_id )
		npc_touchnext_areanpc(sd,false); // Run OnTouch_ on next char in range

	return 1;
}

/**
 * End all statuses except those listed
 * TODO: May be useful for dispel instead resetting a list there
 * @param src: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param type: Changes behaviour of the function
 * 	0: PC killed -> Place here statuses that do not dispel on death.
 * 	1: If for some reason status_change_end decides to still keep the status when quitting.
 * 	2: Do clif_changeoption()
 * 	3: Do not remove some permanent/time-independent effects
 * @return 1: Success 0: Fail
 */
int status_change_clear(struct block_list* bl, int type)
{
	struct status_change* sc;
	int i;

	sc = status_get_sc(bl);

	if (!sc)
		return 0;

	// Cleaning all extras vars
	sc->comet_x = 0;
	sc->comet_y = 0;
#ifndef RENEWAL
	sc->sg_counter = 0;
#endif
	sc->bs_counter = 0;

	if (!sc->count)
		return 0;

	for (i = 0; i < SC_MAX; i++) {
		if (!sc->data[i])
			continue;

		enum e_scb_flag sc_flag = static_cast<e_scb_flag>(status_sc_get_flag((sc_type)i));

		if (type == 0) { // Type 0: PC killed
			if (sc_flag&SCF_NO_REM_ONDEAD) {
				switch (i) {
					case SC_ELEMENTALCHANGE: // Only when its Holy or Dark that it doesn't dispell on death
						if (sc->data[i]->val2 != ELE_HOLY && sc->data[i]->val2 != ELE_DARK)
							break;
					default:
						continue;
				}
			}
		}

		if (type == 3 && sc_flag&SCF_PERMANENT)
			continue;

		status_change_end(bl, (sc_type)i, INVALID_TIMER);
		if( type == 1 && sc->data[i] ) { // If for some reason status_change_end decides to still keep the status when quitting. [Skotlex]
			(sc->count)--;
			if (sc->data[i]->timer != INVALID_TIMER)
				delete_timer(sc->data[i]->timer, status_change_timer);
			ers_free(sc_data_ers, sc->data[i]);
			sc->data[i] = NULL;
		}
	}

	sc->opt1 = 0;
	sc->opt2 = 0;
	sc->opt3 = 0;

	if( type == 0 || type == 2 )
		clif_changeoption(bl);

	return 1;
}

/**
 * End a specific status after checking
 * @param bl: Source of the status change [PC|MOB|HOM|MER|ELEM|NPC]
 * @param type: Status change (SC_*)
 * @param tid: Timer
 * @param file: Used for dancing save
 * @param line: Used for dancing save
 * @return 1: Success 0: Fail
 */
int status_change_end_(struct block_list* bl, enum sc_type type, int tid, const char* file, int line)
{
	struct map_session_data *sd;
	struct status_change *sc;
	struct status_change_entry *sce;
	struct status_data *status;
	struct view_data *vd;
	int opt_flag = 0;
	enum e_scb_flag calc_flag = SCB_NONE;

	nullpo_ret(bl);

	sc = status_get_sc(bl);
	status = status_get_status_data(bl);
	auto scdb = status_get(type);

	if(type < 0 || type >= SC_MAX || !sc || !(sce = sc->data[type]) || !scdb)
		return 0;

	sd = BL_CAST(BL_PC,bl);

	if (sce->timer != tid && tid != INVALID_TIMER)
		return 0;

	if (tid == INVALID_TIMER) {
		if (type == SC_ENDURE && sce->val4)
			// Do not end infinite endure.
			return 0;
		if (type == SC_SPIDERWEB) {
			//Delete the unit group first to expire found in the status change
			struct skill_unit_group *group = NULL, *group2 = NULL;
			t_tick tick = gettick();
			int pos = 1;
			if (sce->val2)
				if (!(group = skill_id2group(sce->val2)))
					sce->val2 = 0;
			if (sce->val3) {
				if (!(group2 = skill_id2group(sce->val3)))
					sce->val3 = 0;
				else if (!group || ((group->limit - DIFF_TICK(tick, group->tick)) > (group2->limit - DIFF_TICK(tick, group2->tick)))) {
					group = group2;
					pos = 2;
				}
			}
			if (sce->val4) {
				if (!(group2 = skill_id2group(sce->val4)))
					sce->val4 = 0;
				else if (!group || ((group->limit - DIFF_TICK(tick, group->tick)) > (group2->limit - DIFF_TICK(tick, group2->tick)))) {
					group = group2;
					pos = 3;
				}
			}
			if (pos == 1)
				sce->val2 = 0;
			else if (pos == 2)
				sce->val3 = 0;
			else if (pos == 3)
				sce->val4 = 0;
			if (group)
				skill_delunitgroup(group);
			if (!status_isdead(bl) && (sce->val2 || sce->val3 || sce->val4))
				return 0; //Don't end the status change yet as there are still unit groups associated with it
		}
		if (sce->timer != INVALID_TIMER) // Could be a SC with infinite duration
			delete_timer(sce->timer,status_change_timer);
		if (sc->opt1)
			switch (type) {
				// "Ugly workaround"  [Skotlex]
				// delays status change ending so that a skill that sets opt1 fails to
				// trigger when it also removed one
				case SC_STONE:
					sce->val4 = -1; // Petrify time
				case SC_FREEZE:
				case SC_STUN:
				case SC_SLEEP:
					if (sce->val1) {
						// Removing the 'level' shouldn't affect anything in the code
						// since these SC are not affected by it, and it lets us know
						// if we have already delayed this attack or not.
						sce->val1 = 0;
						sce->timer = add_timer(gettick()+10, status_change_timer, bl->id, type);
						return 1;
					}
			}
	}

	(sc->count)--;

	if (scdb->state)
		status_calc_state(bl,sc,static_cast<e_scs_flag>(scdb->state),false);

	sc->data[type] = NULL;

	if (scdb->flag&(SCF_DISPLAY_PC|SCF_DISPLAY_NPC))
		status_display_remove(bl,type);

	vd = status_get_viewdata(bl);
	calc_flag = static_cast<e_scb_flag>(scdb->calc_flag);

	switch(type) {
		case SC_GRANITIC_ARMOR:
			{
				int damage = status->max_hp*sce->val3/100;
				if(status->hp < damage) // to not kill him
					damage = status->hp-1;
				status_damage(NULL,bl,damage,0,0,1);
			}
			break;
		case SC_PYROCLASTIC:
			if(bl->type == BL_PC)
				skill_break_equip(bl,bl,EQP_WEAPON,10000,BCT_SELF);
			break;
		case SC_RUN:
		{
			struct unit_data *ud = unit_bl2ud(bl);
			bool begin_spurt = true;
			// Note: this int64 value is stored in two separate int32 variables (FIXME)
			t_tick starttick  = (t_tick)sce->val3&0x00000000ffffffffLL;
			starttick |= ((t_tick)sce->val4<<32)&0xffffffff00000000LL;

			if (ud) {
				if(!ud->state.running)
					begin_spurt = false;
				ud->state.running = 0;
				if (ud->walktimer != INVALID_TIMER)
					unit_stop_walking(bl,1);
			}
			if (begin_spurt && sce->val1 >= 7 &&
				DIFF_TICK(gettick(), starttick) <= 1000 &&
				(!sd || (sd->weapontype1 == 0 && sd->weapontype2 == 0))
			)
				sc_start(bl,bl,SC_SPURT,100,sce->val1,skill_get_time2(scdb->skill_id, sce->val1));
		}
		break;
		case SC_AUTOBERSERK:
			if (sc->data[SC_PROVOKE] && sc->data[SC_PROVOKE]->val2 == 1)
				status_change_end(bl, SC_PROVOKE, INVALID_TIMER);
			break;

		case SC_ENDURE:
		case SC_DEFENDER:
		case SC_REFLECTSHIELD:
		case SC_AUTOGUARD:
			{
				struct map_session_data *tsd;
				if( bl->type == BL_PC ) { // Clear Status from others
					int i;
					for( i = 0; i < MAX_DEVOTION; i++ ) {
						if( sd->devotion[i] && (tsd = map_id2sd(sd->devotion[i])) && tsd->sc.data[type] )
							status_change_end(&tsd->bl, type, INVALID_TIMER);
					}
				}
				else if( bl->type == BL_MER && ((TBL_MER*)bl)->devotion_flag ) { // Clear Status from Master
					tsd = ((TBL_MER*)bl)->master;
					if( tsd && tsd->sc.data[type] )
						status_change_end(&tsd->bl, type, INVALID_TIMER);
				}
			}
			break;
		case SC_DEVOTION:
			{
				struct block_list *d_bl = map_id2bl(sce->val1);
				if( d_bl ) {
					if( d_bl->type == BL_PC )
						((TBL_PC*)d_bl)->devotion[sce->val2] = 0;
					else if( d_bl->type == BL_MER )
						((TBL_MER*)d_bl)->devotion_flag = 0;
					clif_devotion(d_bl, NULL);
				}

				status_change_end(bl, SC_AUTOGUARD, INVALID_TIMER);
				status_change_end(bl, SC_DEFENDER, INVALID_TIMER);
				status_change_end(bl, SC_REFLECTSHIELD, INVALID_TIMER);
				status_change_end(bl, SC_ENDURE, INVALID_TIMER);
			}
			break;

		case SC_BLADESTOP:
			if(sce->val4) {
				int tid2 = sce->val4; //stop the status for the other guy of bladestop as well
				struct block_list *tbl = map_id2bl(tid2);
				struct status_change *tsc = status_get_sc(tbl);
				sce->val4 = 0;
				if(tbl && tsc && tsc->data[SC_BLADESTOP]) {
					tsc->data[SC_BLADESTOP]->val4 = 0;
					status_change_end(tbl, SC_BLADESTOP, INVALID_TIMER);
				}
				clif_bladestop(bl, tid2, 0);
			}
			break;
		case SC_DANCING:
			{
				struct map_session_data *dsd;
				struct status_change_entry *dsc;

				if(sce->val4 && sce->val4 != BCT_SELF && (dsd=map_id2sd(sce->val4))) { // End status on partner as well
					dsc = dsd->sc.data[SC_DANCING];
					if(dsc) {
						// This will prevent recursive loops.
						dsc->val2 = dsc->val4 = 0;
						status_change_end(&dsd->bl, SC_DANCING, INVALID_TIMER);
					}
				}

				if(sce->val2) { // Erase associated land skill
					struct skill_unit_group *group;
					group = skill_id2group(sce->val2);
					if( group == NULL ) {
						ShowDebug("status_change_end: SC_DANCING is missing skill unit group (val1=%d, val2=%d, val3=%d, val4=%d, timer=%d, tid=%d, char_id=%d, map=%s, x=%d, y=%d). Please report this!\n",
							sce->val1, sce->val2, sce->val3, sce->val4, sce->timer, tid,
							sd ? sd->status.char_id : 0,
							mapindex_id2name(map_id2index(bl->m)), bl->x, bl->y);
					}
					sce->val2 = 0;
					skill_delunitgroup(group);
				}

				if((sce->val1&0xFFFF) == CG_MOONLIT)
					clif_status_change(bl,EFST_MOON,0,0,0,0,0);

				status_change_end(bl, SC_LONGING, INVALID_TIMER);
			}
			break;
		case SC_NOCHAT:
			if (sd && sd->status.manner < 0 && tid != INVALID_TIMER)
				sd->status.manner = 0;
			if (sd && tid == INVALID_TIMER) {
				clif_changestatus(sd,SP_MANNER,sd->status.manner);
				clif_updatestatus(sd,SP_MANNER);
			}
			break;
		case SC_SPLASHER:
			{
				struct block_list *src=map_id2bl(sce->val3);
				if(src && tid != INVALID_TIMER)
					skill_castend_damage_id(src, bl, sce->val2, sce->val1, gettick(), SD_LEVEL );
			}
			break;
		case SC_TINDER_BREAKER2:
		case SC_CLOSECONFINE2:{
			struct block_list *src = sce->val2?map_id2bl(sce->val2):NULL;
			struct status_change *sc2 = src?status_get_sc(src):NULL;
			enum sc_type type2 = ((type==SC_CLOSECONFINE2)?SC_CLOSECONFINE:SC_TINDER_BREAKER);
			if (src && sc2 && sc2->data[type2]) {
				// If status was already ended, do nothing.
				// Decrease count
				if (type==SC_TINDER_BREAKER2 || (--(sc2->data[type2]->val1) <= 0)) // No more holds, free him up.
					status_change_end(src, type2, INVALID_TIMER);
			}
		}
		case SC_TINDER_BREAKER:
		case SC_CLOSECONFINE:
			if (sce->val2 > 0) {
				// Caster has been unlocked... nearby chars need to be unlocked.
				int range = 1
					+ skill_get_range2(bl, scdb->skill_id, sce->val1, true)
					+ skill_get_range2(bl, TF_BACKSLIDING, 1, true); // Since most people use this to escape the hold....
				map_foreachinallarea(status_change_timer_sub,
					bl->m, bl->x-range, bl->y-range, bl->x+range,bl->y+range,BL_CHAR,bl,sce,type,gettick());
			}
			break;
		case SC_COMBO:
			skill_combo_toggle_inf(bl,sce->val1,0);
			break;
		case SC_MARIONETTE:
		case SC_MARIONETTE2: // Marionette target
			if (sce->val1) { // Check for partner and end their marionette status as well
				enum sc_type type2 = (type == SC_MARIONETTE) ? SC_MARIONETTE2 : SC_MARIONETTE;
				struct block_list *pbl = map_id2bl(sce->val1);
				struct status_change* sc2 = pbl?status_get_sc(pbl):NULL;

				if (sc2 && sc2->data[type2]) {
					sc2->data[type2]->val1 = 0;
					status_change_end(pbl, type2, INVALID_TIMER);
				}
			}
			break;

		case SC_CONCENTRATION:
			status_change_end(bl, SC_ENDURE, INVALID_TIMER);
			break;
		case SC_BERSERK:
			if(status->hp > 200 && sc && sc->data[SC__BLOODYLUST]) {
				status_percent_heal(bl, 100, 0);
				status_change_end(bl, SC__BLOODYLUST, INVALID_TIMER);
			} else if (status->hp > 100 && sce->val2) // If val2 is removed, no HP penalty (dispelled?) [Skotlex]
				status_set_hp(bl, 100, 0);
			if(sc->data[SC_ENDURE] && sc->data[SC_ENDURE]->val4 == 2) {
				sc->data[SC_ENDURE]->val4 = 0;
				status_change_end(bl, SC_ENDURE, INVALID_TIMER);
			}
			sc_start4(bl, bl, SC_REGENERATION, 100, 10,0,0,(RGN_HP|RGN_SP), skill_get_time(LK_BERSERK, sce->val1));
			break;
		case SC_GOSPEL:
			if (sce->val3) { // Clear the group.
				struct skill_unit_group* group = skill_id2group(sce->val3);
				sce->val3 = 0;
				if (group)
					skill_delunitgroup(group);
			}
			break;
		case SC_HERMODE:
			if(sce->val3 == BCT_SELF)
				skill_clear_unitgroup(bl);
			break;
		case SC_BASILICA: // Clear the skill area. [Skotlex]
				if (sce->val3 && sce->val4 == bl->id) {
					struct skill_unit_group* group = skill_id2group(sce->val3);
					sce->val3 = 0;
					if (group)
						skill_delunitgroup(group);
				}
				break;
		case SC_TRICKDEAD:
			if (vd) vd->dead_sit = 0;
			break;
		case SC_WARM:
		case SC__MANHOLE:
			if (sce->val4) { // Clear the group.
				struct skill_unit_group* group = skill_id2group(sce->val4);
				sce->val4 = 0;
				if( group ) // Might have been cleared before status ended, e.g. land protector
					skill_delunitgroup(group);
			}
			break;
		case SC_JAILED:
			if(tid == INVALID_TIMER)
				break;
		  	// Natural expiration.
			if(sd && sd->mapindex == sce->val2)
				pc_setpos(sd,(unsigned short)sce->val3,sce->val4&0xFFFF, sce->val4>>16, CLR_TELEPORT);
			break; // Guess hes not in jail :P
		case SC_CHANGE:
			if (tid == INVALID_TIMER)
		 		break;
			// "lose almost all their HP and SP" on natural expiration.
			status_set_hp(bl, 10, 0);
			status_set_sp(bl, 10, 0);
			break;
		case SC_AUTOTRADE:
			if (tid == INVALID_TIMER)
				break;
			// Vending is not automatically closed for autovenders
			vending_closevending(sd);
			map_quit(sd);
			// Because map_quit calls status_change_end with tid -1
			// from here it's not neccesary to continue
			return 1;
			break;
		case SC_STOP:
			if( sce->val2 ) {
				struct block_list* tbl = map_id2bl(sce->val2);
				sce->val2 = 0;
				if( tbl && (sc = status_get_sc(tbl)) && sc->data[SC_STOP] && sc->data[SC_STOP]->val2 == bl->id )
					status_change_end(tbl, SC_STOP, INVALID_TIMER);
			}
			break;
		case SC_TENSIONRELAX:
			if (sc && (sc->data[SC_WEIGHT50] || sc->data[SC_WEIGHT90]))
				status_get_regen_data(bl)->state.overweight = 1; // Add the overweight flag back
			break;
		case SC_MONSTER_TRANSFORM:
		case SC_ACTIVE_MONSTER_TRANSFORM:
			if (sce->val2)
				status_change_end(bl, (sc_type)sce->val2, INVALID_TIMER);
			break;

		/* 3rd Stuff */
		case SC_MILLENNIUMSHIELD:
			clif_millenniumshield(bl, 0);
			break;
		case SC_HALLUCINATIONWALK:
			sc_start(bl,bl,SC_HALLUCINATIONWALK_POSTDELAY,100,sce->val1,skill_get_time2(GC_HALLUCINATIONWALK,sce->val1));
			break;
		case SC_WHITEIMPRISON:
			{
				struct block_list* src = map_id2bl(sce->val2);
				if( tid == -1 || !src)
					break; // Terminated by Damage
				status_fix_damage(src,bl,400*sce->val1,clif_damage(bl,bl,gettick(),0,0,400*sce->val1,0,DMG_NORMAL,0,false));
			}
			break;
		case SC_WUGDASH:
			{
				struct unit_data *ud = unit_bl2ud(bl);
				if (ud) {
					ud->state.running = 0;
					if (ud->walktimer != INVALID_TIMER)
						unit_stop_walking(bl,1);
				}
			}
			break;
		case SC__SHADOWFORM:
			{
				struct map_session_data *s_sd = map_id2sd(sce->val2);

				if (s_sd) s_sd->shadowform_id = 0;
			}
			break;
		case SC_SATURDAYNIGHTFEVER: // Sit down force of Saturday Night Fever has the duration of only 3 seconds.
			sc_start(bl, bl,SC_SITDOWN_FORCE,100,sce->val1,skill_get_time2(WM_SATURDAY_NIGHT_FEVER,sce->val1));
			break;
		case SC_NEUTRALBARRIER_MASTER:
		case SC_STEALTHFIELD_MASTER:
			if( sce->val2 ) {
				struct skill_unit_group* group = skill_id2group(sce->val2);
				sce->val2 = 0;
				if( group ) // Might have been cleared before status ended, e.g. land protector
					skill_delunitgroup(group);
			}
			break;
		case SC_BANDING:
			if(sce->val4) {
				struct skill_unit_group *group = skill_id2group(sce->val4);
				sce->val4 = 0;
				if( group ) // Might have been cleared before status ended, e.g. land protector
					skill_delunitgroup(group);
			}
			break;
		case SC_CURSEDCIRCLE_ATKER:
			if( sce->val2 ) // Used the default area size cause there is a chance the caster could knock back and can't clear the target.
				map_foreachinallrange(status_change_timer_sub, bl, AREA_SIZE + 3, BL_CHAR, bl, sce, SC_CURSEDCIRCLE_TARGET, gettick());
			break;
		case SC_RAISINGDRAGON:
			if( sd && sce->val2 && !pc_isdead(sd) ) {
				int i = min(sd->spiritball,5);
				pc_delspiritball(sd, sd->spiritball, 0);
				status_change_end(bl, SC_EXPLOSIONSPIRITS, INVALID_TIMER);
				while( i > 0 ) {
					pc_addspiritball(sd, skill_get_time(MO_CALLSPIRITS, pc_checkskill(sd,MO_CALLSPIRITS)), 5);
					--i;
				}
			}
			break;
		case SC_CURSEDCIRCLE_TARGET:
			{
				struct block_list *src = map_id2bl(sce->val2);
				struct status_change *sc2 = status_get_sc(src);

				if( sc2 && sc2->data[SC_CURSEDCIRCLE_ATKER] && --(sc2->data[SC_CURSEDCIRCLE_ATKER]->val2) == 0 ) {
					clif_bladestop(bl, sce->val2, 0);
					status_change_end(src, SC_CURSEDCIRCLE_ATKER, INVALID_TIMER);
				}
			}
			break;
		case SC_BLOODSUCKER:
			if( sce->val2 ) {
				struct block_list *src = map_id2bl(sce->val2);
				if(src) {
					struct status_change *sc2 = status_get_sc(src);
					sc2->bs_counter--;
				}
			}
			break;
		case SC_TEARGAS:
			status_change_end(bl,SC_TEARGAS_SOB,INVALID_TIMER);
			break;
		case SC_SITDOWN_FORCE:
		case SC_BANANA_BOMB_SITDOWN:
			if( sd && pc_issit(sd) && pc_setstand(sd, false) )
				skill_sit(sd,0);
			break;
		case SC_KYOUGAKU:
			clif_status_load(bl, EFST_KYOUGAKU, 0); // Avoid client crash
			clif_status_load(bl, EFST_ACTIVE_MONSTER_TRANSFORM, 0);
			break;
		case SC_INTRAVISION:
			calc_flag = SCB_ALL; // Required for overlapping
			break;
		case SC_OVERED_BOOST:
			switch (bl->type) {
				case BL_HOM: {
						struct homun_data *hd = BL_CAST(BL_HOM,bl);

						if( hd )
							hd->homunculus.hunger = max(1,hd->homunculus.hunger - 50);
					}
					break;
				case BL_PC:
					status_zap(bl,0,status_get_max_sp(bl) / 2);
					break;
			}
			break;
		case SC_FULL_THROTTLE: {
				int sec = skill_get_time2(scdb->skill_id, sce->val1);

				clif_status_change(bl, EFST_DEC_AGI, 1, sec, 0, 0, 0);
				sc_start(bl, bl, SC_REBOUND, 100, sce->val1, sec);
			}
			break;
		case SC_REBOUND:
			clif_status_load(bl, EFST_DEC_AGI, 0);
			break;
		case SC_ITEMSCRIPT: // Removes Buff Icons
			if (sd && sce->val2 != EFST_BLANK)
				clif_status_load(bl, (enum efst_type)sce->val2, 0);
			break;
		case SC_C_MARKER:
			{
				// Remove mark data from caster
				struct map_session_data *caster = map_id2sd(sce->val2);
				uint8 i = 0;

				if (!caster)
					break;
				ARR_FIND(0,MAX_SKILL_CRIMSON_MARKER,i,caster->c_marker[i] == bl->id);
				if (i < MAX_SKILL_CRIMSON_MARKER) {
					caster->c_marker[i] = 0;
					clif_crimson_marker(caster, bl, true);
				}
			}
			break;
		case SC_H_MINE:
			{
				// Drop the material from target if expired
				struct item it;
				struct map_session_data *caster = NULL;

				if (sce->val3 || status_isdead(bl) || !(caster = map_id2sd(sce->val2)))
					break;
				if (!itemdb_exists(skill_get_itemid(RL_H_MINE,0)))
					break;
				memset(&it, 0, sizeof(it));
				it.nameid = skill_get_itemid(RL_H_MINE,0);
				it.amount = max(skill_get_itemqty(RL_H_MINE,0),1);
				it.identify = 1;
				map_addflooritem(&it, it.amount, bl->m,bl->x, bl->y, caster->status.char_id, 0, 0, 4, 0);
			}
			break;
		case SC_VACUUM_EXTREME:
			///< !CHECKME: Seems on official, there's delay before same target can be vacuumed in same area again [Cydh]
			sc_start2(bl, bl, SC_VACUUM_EXTREME_POSTDELAY, 100, sce->val1, sce->val2, skill_get_time2(SO_VACUUM_EXTREME,sce->val1));
			break;
		case SC_SWORDCLAN:
		case SC_ARCWANDCLAN:
		case SC_GOLDENMACECLAN:
		case SC_CROSSBOWCLAN:
		case SC_JUMPINGCLAN:
			status_change_end(bl,SC_CLAN_INFO,INVALID_TIMER);
			break;
	}

	// Reset the options as needed
	opt_flag = scdb->flag&SCF_OPT_FLAGS;
	switch (type) {
		case SC_SWOO:
		case SC_SKA:
			opt_flag = SCF_OPT_CHANGEOPTION;
 			break;
		case SC_DANCING:
			if ((sce->val1&0xFFFF) == CG_MOONLIT)
				sc->opt3 &= ~OPT3_MOONLIT;
			opt_flag = 0;
			break;
		case SC_INCATKRATE: // Simulated Explosion spirits effect.
			if (bl->type != BL_MOB) {
				opt_flag = 0;
				break;
			}
			break;
	}

	if (scdb->opt1)
		sc->opt1 = OPT1_NONE;

	if (scdb->opt2)
		sc->opt2 &= ~scdb->opt2;

	if (scdb->opt3)
		sc->opt3 &= ~scdb->opt3;

	if (scdb->look)
		sc->option &= ~scdb->look;

	if (calc_flag&SCB_DYE) { // Restore DYE color
		if (vd && !vd->cloth_color && sce->val4)
			clif_changelook(bl,LOOK_CLOTHES_COLOR,sce->val4);
		calc_flag = static_cast<e_scb_flag>(calc_flag&~SCB_DYE);
	}

	/*if (calc_flag&SCB_BODY)// Might be needed in the future. [Rytech]
	{	//Restore body style
		if (vd && !vd->body_style && sce->val4)
			clif_changelook(bl,LOOK_BODY2,sce->val4);
		calc_flag = static_cast<scb_flag>(calc_flag&~SCB_BODY);
	}*/

	// On Aegis, when turning off a status change, first goes the sc packet, then the option packet.
	clif_status_change(bl,scdb->icon,0,0,0,0,0);

	if( opt_flag&SCF_OPT_CHANGEOPTION ) // bugreport:681
		clif_changeoption2(bl);
	else if(opt_flag) {
		clif_changeoption(bl);
		if (sd && (opt_flag&SCF_OPT_CHANGELOOK)) {
			clif_changelook(bl,LOOK_BASE,sd->vd.class_);
			clif_get_weapon_view(sd,&sd->vd.weapon,&sd->vd.shield);
			clif_changelook(bl,LOOK_WEAPON,sd->vd.weapon);
			clif_changelook(bl,LOOK_SHIELD,sd->vd.shield);
			clif_changelook(bl,LOOK_CLOTHES_COLOR,cap_value(sd->status.clothes_color,0,battle_config.max_cloth_color));
			clif_changelook(bl,LOOK_BODY2,cap_value(sd->status.body,0,battle_config.max_body_style));
		}
	}
	if (calc_flag) {
		switch (type) {
		case SC_MAGICPOWER:
			//If Mystical Amplification ends, MATK is immediately recalculated
			status_calc_bl_(bl, calc_flag, SCO_FORCE);
			break;
		default:
			status_calc_bl(bl, calc_flag);
			break;
		}
	}

	if (scdb->look&OPTION_HIDE) // Out of hiding, invoke on place.
		skill_unit_move(bl,gettick(),1);

	if (opt_flag&SCF_TRIGGER_ONTOUCH_ && sd && !sd->state.warping && map_getcell(bl->m,bl->x,bl->y,CELL_CHKNPC))
		npc_touch_areanpc(sd,bl->m,bl->x,bl->y); // Trigger on-touch event.

	ers_free(sc_data_ers, sce);
	return 1;
}

/**
 * Resets timers for statuses
 * Used with reoccurring status effects, such as dropping SP every 5 seconds
 * @param tid: Timer ID
 * @param tick: How long before next call
 * @param id: ID of character
 * @param data: Information passed through the timer call
 * @return 1: Success 0: Fail
 */
TIMER_FUNC(status_change_timer){
	enum sc_type type = (sc_type)data;
	struct block_list *bl;
	struct map_session_data *sd;
	int interval = status_get_sc_interval(type);
	bool dounlock = false;

	bl = map_id2bl(id);
	if(!bl) {
		ShowDebug("status_change_timer: Null pointer id: %d data: %d\n", id, data);
		return 0;
	}

	struct status_change * const sc = status_get_sc(bl);
	struct status_data * const status = status_get_status_data(bl);
	if(!sc) {
		ShowDebug("status_change_timer: Null pointer id: %d data: %d bl-type: %d\n", id, data, bl->type);
		return 0;
	}
	
	struct status_change_entry * const sce = sc->data[type];
	if(!sce) {
		ShowDebug("status_change_timer: Null pointer id: %d data: %d bl-type: %d\n", id, data, bl->type);
		return 0;
	}
	if( sce->timer != tid ) {
		ShowError("status_change_timer: Mismatch for type %d: %d != %d (bl id %d)\n",type,tid,sce->timer, bl->id);
		return 0;
	}

	sd = BL_CAST(BL_PC, bl);

	std::function<void (t_tick)> sc_timer_next = [&sce, &bl, &data](t_tick t) {
		sce->timer = add_timer(t, status_change_timer, bl->id, data);
	};
	
	switch(type) {
	case SC_MAXIMIZEPOWER:
	case SC_CLOAKING:
		if(!status_charge(bl, 0, 1))
			break; // Not enough SP to continue.
		sc_timer_next(sce->val2+tick);
		return 0;

	case SC_CHASEWALK:
		if(!status_charge(bl, 0, sce->val4))
			break; // Not enough SP to continue.

		if (!sc->data[SC_CHASEWALK2]) {
			sc_start(bl,bl, SC_CHASEWALK2,100,1<<(sce->val1-1),
				(t_tick)(sc->data[SC_SPIRIT] && sc->data[SC_SPIRIT]->val2 == SL_ROGUE?10:1) // SL bonus -> x10 duration
				*skill_get_time2(status_sc_get_skill(type),sce->val1));
		}
		sc_timer_next(sce->val2+tick);
		return 0;
	break;

	case SC_HIDING:
		if(--(sce->val2)>0) {

			if(sce->val2 % sce->val4 == 0 && !status_charge(bl, 0, 1))
				break; // Fail if it's time to substract SP and there isn't.

			sc_timer_next(1000+tick);
			return 0;
		}
	break;

	case SC_SIGHT:
	case SC_RUWACH:
	case SC_SIGHTBLASTER:
		if(type == SC_SIGHTBLASTER) {
			//Restore trap immunity
			if(sce->val4%2)
				sce->val4--;
			map_foreachinallrange( status_change_timer_sub, bl, sce->val3, BL_CHAR|BL_SKILL, bl, sce, type, tick);
		} else {
			map_foreachinallrange( status_change_timer_sub, bl, sce->val3, BL_CHAR, bl, sce, type, tick);
			skill_reveal_trap_inarea(bl, sce->val3, bl->x, bl->y);
		}

		if( --(sce->val2)>0 ) {
			sce->val4 += 20; // Use for Shadow Form 2 seconds checking.
			sc_timer_next(20+tick);
			return 0;
		}
		break;

	case SC_PROVOKE:
		if(sce->val2) { // Auto-provoke (it is ended in status_heal)
			sc_timer_next(1000*60+tick);
			return 0;
		}
		break;

	case SC_STONE:
		if (sc->opt1 == OPT1_STONEWAIT && sce->val4) {
			sce->val3 = 0; //Incubation time used up
			unit_stop_attack(bl);
			if (sc->data[SC_DANCING]) {
				unit_stop_walking(bl, 1);
				status_change_end(bl, SC_DANCING, INVALID_TIMER);
			}
			status_change_end(bl, SC_AETERNA, INVALID_TIMER);
			sc->opt1 = OPT1_STONE;
			status_calc_state(bl,sc,static_cast<e_scs_flag>(status_sc_get_state(SC_STONE)),1);
			clif_changeoption(bl);
			sc_timer_next(min(sce->val4, interval) + tick);
			sce->val4 -= interval; //Remaining time
			status_calc_bl(bl, status_sc_get_calc_flag(type));
			return 0;
		}
		if (sce->val4 >= 0 && !(sce->val3) && status->hp > status->max_hp / 4) {
			status_percent_damage(NULL, bl, 1, 0, false);
		}
		break;

	case SC_POISON:
	case SC_DPOISON:
		if (sce->val4 >= 0 && !sc->data[SC_SLOWPOISON]) {
			unsigned int damage = 0;
			if (sd)
				damage = (type == SC_DPOISON) ? 2 + status->max_hp / 50 : 2 + status->max_hp * 3 / 200;
			else
				damage = (type == SC_DPOISON) ? 2 + status->max_hp / 100 : 2 + status->max_hp / 200;
			if (status->hp > umax(status->max_hp / 4, damage)) // Stop damaging after 25% HP left.
				status_zap(bl, damage, 0);
		}
		break;

	case SC_BLEEDING:
		if (sce->val4 >= 0) {
			int64 damage = rnd() % 600 + 200;
			if (!sd && damage >= status->hp)
				damage = status->hp - 1; // No deadly damage for monsters
			map_freeblock_lock();
			dounlock = true;
			status_zap(bl, damage, 0);
		}
		break;

	case SC_BURNING:
		if (sce->val4 >= 0) {
			int64 damage = 1000 + (3 * status->max_hp) / 100; // Deals fixed (1000 + 3%*MaxHP)
			map_freeblock_lock();
			dounlock = true;
			status_fix_damage(bl, bl, damage, clif_damage(bl, bl, tick, 0, 1, damage, 1, DMG_NORMAL, 0, false));
		}
		break;

	case SC_TOXIN:
		if (sce->val4 >= 0) { // Damage is every 10 seconds including 3%sp drain.
			map_freeblock_lock();
			dounlock = true;
			status_damage(bl, bl, 1, status->max_sp * 3 / 100, clif_damage(bl, bl, tick, status->amotion, status->dmotion + 500, 1, 1, DMG_NORMAL, 0, false), 0);
		}
		break;

	case SC_MAGICMUSHROOM:
		if (sce->val4 >= 0) {
			bool flag = 0;
			int64 damage = status->max_hp * 3 / 100;
			if (status->hp <= damage)
				damage = status->hp - 1; // Cannot Kill

			if (damage > 0) { // 3% Damage each 4 seconds
				map_freeblock_lock();
				status_zap(bl, damage, 0);
				flag = !sc->data[type]; // Killed? Should not
				map_freeblock_unlock();
			}

			if (!flag) { // Random Skill Cast
				if (skill_magicmushroom_count && sd && !pc_issit(sd)) { // Can't cast if sit
					int mushroom_skill_id = 0, checked = 0, checked_max = MAX_SKILL_MAGICMUSHROOM_DB * 3;
					unit_stop_attack(bl);
					unit_skillcastcancel(bl, 1);
					do {
						int i = rnd() % MAX_SKILL_MAGICMUSHROOM_DB;
						mushroom_skill_id = skill_magicmushroom_db[i].skill_id;
					} while (checked++ < checked_max && mushroom_skill_id == 0);

					if (!skill_get_index(mushroom_skill_id))
						break;

					switch (skill_get_casttype(mushroom_skill_id)) { // Magic Mushroom skills are buffs or area damage
					case CAST_GROUND:
						skill_castend_pos2(bl, bl->x, bl->y, mushroom_skill_id, 1, tick, 0);
						break;
					case CAST_NODAMAGE:
						skill_castend_nodamage_id(bl, bl, mushroom_skill_id, 1, tick, 0);
						break;
					case CAST_DAMAGE:
						skill_castend_damage_id(bl, bl, mushroom_skill_id, 1, tick, 0);
						break;
					}
				}
				clif_emotion(bl, ET_SMILE);
			}
		}
		break;

	case SC_PYREXIA:
		if (sce->val4 >= 0) {
			map_freeblock_lock();
			dounlock = true;
			status_fix_damage(bl, bl, 100, clif_damage(bl, bl, tick, status->amotion, status->dmotion + 500, 100, 1, DMG_NORMAL, 0, false));
		}
		break;

	case SC_LEECHESEND:
		if (sce->val4 >= 0) {
			int64 damage = status->vit * (sce->val1 - 3) + (int)status->max_hp / 100; // {Target VIT x (New Poison Research Skill Level - 3)} + (Target HP/100)
			map_freeblock_lock();
			dounlock = true;
			status_fix_damage(bl, bl, damage, clif_damage(bl, bl, tick, status->amotion, status->dmotion + 500, damage, 1, DMG_NORMAL, 0, false));
			unit_skillcastcancel(bl, 2);
		}
		break;

	case SC_TENSIONRELAX:
		if(status->max_hp > status->hp && --(sce->val3) >= 0) {
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;

	case SC_KNOWLEDGE:
		if (!sd) break;
		if(bl->m == sd->feel_map[0].m ||
			bl->m == sd->feel_map[1].m ||
			bl->m == sd->feel_map[2].m)
		{	// Timeout will be handled by pc_setpos
			sce->timer = INVALID_TIMER;
			return 0;
		}
		break;

	case SC_S_LIFEPOTION:
	case SC_L_LIFEPOTION:
		if( --(sce->val4) >= 0 ) {
			// val1 < 0 = per max% | val1 > 0 = exact amount
			int hp = 0;
			if( status->hp < status->max_hp )
				hp = (sce->val1 < 0) ? (int)(status->max_hp * -1 * sce->val1 / 100.) : sce->val1;
			status_heal(bl, hp, 0, 2);
			sc_timer_next((sce->val2 * 1000) + tick);
			return 0;
		}
		break;

	case SC_BOSSMAPINFO:
		if( sd && --(sce->val4) >= 0 ) {
			struct mob_data *boss_md = map_id2boss(sce->val1);

			if (boss_md) {
				if (sd->bl.m != boss_md->bl.m) // Not on same map anymore
					return 0;
				else if (boss_md->bl.prev != NULL) { // Boss is alive - Update X, Y on minimap
					sce->val2 = 0;
					clif_bossmapinfo(sd, boss_md, BOSS_INFO_ALIVE);
				} else if (boss_md->spawn_timer != INVALID_TIMER && !sce->val2) { // Boss is dead
					sce->val2 = 1;
					clif_bossmapinfo(sd, boss_md, BOSS_INFO_DEAD);
				}
			}
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_DANCING: // SP consumption by time of dancing skills
		{
			int s = 0;
			int sp = 1;
			if (--sce->val3 <= 0)
				break;
			switch(sce->val1&0xFFFF) {
				case BD_RICHMANKIM:
				case BD_DRUMBATTLEFIELD:
				case BD_RINGNIBELUNGEN:
				case BD_SIEGFRIED:
				case BA_DISSONANCE:
				case BA_ASSASSINCROSS:
				case DC_UGLYDANCE:
					s=3;
					break;
				case BD_LULLABY:
				case BD_ETERNALCHAOS:
				case BD_ROKISWEIL:
				case DC_FORTUNEKISS:
					s=4;
					break;
				case CG_HERMODE:
				case BD_INTOABYSS:
				case BA_WHISTLE:
				case DC_HUMMING:
				case BA_POEMBRAGI:
				case DC_SERVICEFORYOU:
					s=5;
					break;
				case BA_APPLEIDUN:
					#ifdef RENEWAL
						s=5;
					#else
						s=6;
					#endif
					break;
				case CG_MOONLIT:
					// Moonlit's cost is 4sp*skill_lv [Skotlex]
					sp= 4*(sce->val1>>16);
					// Upkeep is also every 10 secs.
				case DC_DONTFORGETME:
					s=10;
					break;
			}
			if( s != 0 && sce->val3 % s == 0 ) {
				if (sc->data[SC_LONGING])
					sp*= 3;
				if (!status_charge(bl, 0, sp))
					break;
			}
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_BERSERK:
		// 5% every 10 seconds [DracoRPG]
		if( --( sce->val3 ) > 0 && status_charge(bl, sce->val2, 0) && status->hp > 100 ) {
			sc_timer_next(sce->val4+tick);
			return 0;
		}
		break;

	case SC_NOCHAT:
		if(sd) {
			sd->status.manner++;
			clif_changestatus(sd,SP_MANNER,sd->status.manner);
			clif_updatestatus(sd,SP_MANNER);
			if (sd->status.manner < 0) { // Every 60 seconds your manner goes up by 1 until it gets back to 0.
				sc_timer_next(60000+tick);
				return 0;
			}
		}
		break;

	case SC_SPLASHER:
		// Custom Venom Splasher countdown timer
		// if (sce->val4 % 1000 == 0) {
		// 	char timer[10];
		// 	snprintf (timer, 10, "%d", sce->val4/1000);
		// 	clif_message(bl, timer);
		// }
		if((sce->val4 -= 500) > 0) {
			sc_timer_next(500 + tick);
			return 0;
		}
		break;

	case SC_MARIONETTE:
	case SC_MARIONETTE2:
		{
			struct block_list *pbl = map_id2bl(sce->val1);
			if( pbl && check_distance_bl(bl, pbl, 7) ) {
				sc_timer_next(1000 + tick);
				return 0;
			}
		}
		break;

	case SC_GOSPEL:
		if(sce->val4 == BCT_SELF && --(sce->val2) > 0) {
			int hp, sp;
			hp = (sce->val1 > 5) ? 45 : 30;
			sp = (sce->val1 > 5) ? 35 : 20;
			if(!status_charge(bl, hp, sp))
				break;
			sc_timer_next(10000+tick);
			return 0;
		}
		break;

	case SC_JAILED:
		if(sce->val1 == INT_MAX || --(sce->val1) > 0) {
			sc_timer_next(60000+tick);
			return 0;
		}
		break;

	case SC_BLIND:
		if(sc->data[SC_FOGWALL]) { // Blind lasts forever while you are standing on the fog.
			sc_timer_next(5000+tick);
			return 0;
		}
		break;
	case SC_ABUNDANCE:
		if(--(sce->val4) > 0) {
			status_heal(bl,0,60,0);
			sc_timer_next(10000+tick);
		}
		break;

	case SC_OBLIVIONCURSE:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl,ET_QUESTION);
			sc_timer_next(3000 + tick);
			return 0;
		}
		break;

	case SC_WEAPONBLOCKING:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl,0,3) )
				break;
			sc_timer_next(5000+tick);
			return 0;
		}
		break;

	case SC_CLOAKINGEXCEED:
		if(!status_charge(bl,0,10-sce->val1))
			break;
		sc_timer_next(1000 + tick);
		return 0;

	case SC_RENOVATIO:
		if( --(sce->val4) >= 0 ) {
			int heal = status->max_hp * 5 / 100;
			if( sc && sc->data[SC_AKAITSUKI] && heal )
				heal = ~heal + 1;
			status_heal(bl, heal, 0, 3);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_SPHERE_1:
	case SC_SPHERE_2:
	case SC_SPHERE_3:
	case SC_SPHERE_4:
	case SC_SPHERE_5:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl, 0, 1) )
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_FREEZE_SP:
		if( !status_charge(bl, 0, sce->val2) ) {
			int i;
			for(i = SC_SPELLBOOK1; i <= SC_MAXSPELLBOOK; i++) // Also remove stored spell as well.
				status_change_end(bl, (sc_type)i, INVALID_TIMER);
			break;
		}
		sc_timer_next(10000 + tick);
		return 0;

	case SC_ELECTRICSHOCKER:
		if( --(sce->val4) >= 0 ) {
			status_charge(bl, 0, 5 * sce->val1 * status->max_sp / 100);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_CAMOUFLAGE:
		if (!status_charge(bl, 0, 7 - sce->val1))
			break;
		if (--sce->val4 >= 0)
			sce->val3++;
		sc_timer_next(1000 + tick);
		return 0;

	case SC__REPRODUCE:
		if(!status_charge(bl, 0, 1))
			break;
		sc_timer_next(1000+tick);
		return 0;

	case SC__SHADOWFORM:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl, 0, 11 - sce->val1) )
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC__INVISIBILITY:
		if( !status_charge(bl, 0, (12 - 2 * sce->val1) * status->max_sp / 100) ) // 6% - skill_lv.
			break;
		sc_timer_next(1000 + tick);
		return 0;

	case SC_STRIKING:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl,0, sce->val3 ) )
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_WARMER: {
			int hp = 0;
			struct status_change *ssc = status_get_sc(map_id2bl(sce->val2));

			if (ssc && ssc->data[SC_HEATER_OPTION])
				hp = status->max_hp * 3 * sce->val1 / 100;
			else
				hp = status->max_hp * sce->val1 / 100;
			if (sc && sc->data[SC_AKAITSUKI] && hp)
				hp = ~hp + 1;
			if (status->hp != status->max_hp)
				status_heal(bl, hp, 0, 0);
			sc_timer_next(3000 + tick);
			return 0;
		}

	case SC_BLOODSUCKER:
		if( --(sce->val4) >= 0 ) {
			struct block_list *src = map_id2bl(sce->val2);
			int damage;
			if( !src || (src && (status_isdead(src) || src->m != bl->m || distance_bl(src, bl) >= 12)) )
				break;
			map_freeblock_lock();
			if (is_infinite_defense(bl, BF_MISC)) // Only does 1 damage to infinte defense type.
				damage = 1;
			else
				damage =  200 + 100 * sce->val1 + status_get_int(src);
			status_damage(src, bl, damage, 0, clif_damage(bl,bl,tick,status->amotion,status->dmotion+200,damage,1,DMG_NORMAL,0,false), 0);
			unit_skillcastcancel(bl,1);
			if ( sc->data[type] ) {
				sc_timer_next(1000 + tick);
			}
			map_freeblock_unlock();
			status_heal(src, damage*(5 + 5 * sce->val1)/100, 0, 0); // 5 + 5% per level
			return 0;
		}
		break;

	case SC_VOICEOFSIREN:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl,ET_THROB);
			sc_timer_next(2000 + tick);
			return 0;
		}
		break;

	case SC_DEEPSLEEP:
		if( --(sce->val4) >= 0 ) { // Recovers 3% HP/SP every 2 seconds.
			status_heal(bl, status->max_hp * 3 / 100, status->max_sp * 3 / 100, 2);
			sc_timer_next(2000 + tick);
			return 0;
		}
		break;

	case SC_SIRCLEOFNATURE:
		if( --(sce->val4) >= 0 ) {
			if( !status_charge(bl,0,sce->val3) )
				break;
			status_heal(bl, sce->val2, 0, 1);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_SONGOFMANA:
		if( --(sce->val4) >= 0 ) {
			status_heal(bl,0,sce->val3,3);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;


	case SC_SATURDAYNIGHTFEVER:
		// 1% HP/SP drain every val4 seconds [Jobbie]
		if( --(sce->val3) >= 0 ) {
			if( !status_charge(bl, status->hp / 100, status->sp / 100) )
				break;
			sc_timer_next(sce->val4+tick);
			return 0;
		}
		break;

	case SC_MELODYOFSINK:
		if( --(sce->val4) >= 0 ) {
			status_charge(bl, 0, status->max_sp * ( 2 * sce->val1 + 2 * sce->val2 ) / 100);
			sc_timer_next(1000+tick);
			return 0;
		}
		break;

	case SC_CRYSTALIZE:
		if( --(sce->val4) >= 0 ) { // Drains 2% of HP and 1% of SP every seconds.
			if (!status_charge(bl, status->max_hp * 2 / 100, status->max_sp / 100))
				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_FORCEOFVANGUARD:
		if( !status_charge(bl,0,24 - 4 * sce->val1) )
			break;
		sc_timer_next(10000 + tick);
		return 0;

	case SC_BANDING:
		if( status_charge(bl, 0, 7 - sce->val1) ) {
			sce->val2 = (sd ? skill_banding_count(sd) : 1);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_REFLECTDAMAGE:
		if( --(sce->val4) > 0 ) {
			if( !status_charge(bl,0,10) )
 				break;
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_OVERHEAT_LIMITPOINT:
		if (--(sce->val1) >= 0) { // Cooling
			int16 limit[] = { 150, 200, 280, 360, 450 };
			uint16 skill_lv = (sd ? pc_checkskill(sd, NC_MAINFRAME) : 0);

			if (sc && sc->data[SC_OVERHEAT])
				status_change_end(bl,SC_OVERHEAT,INVALID_TIMER);
			if (sce->val1 > limit[skill_lv])
				sc_start(bl, bl, SC_OVERHEAT, 100, sce->val1, 1000);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_OVERHEAT: {
			int damage = status->max_hp / 100; // Suggestion 1% each second

			if (damage >= status->hp)
				damage = status->hp - 1; // Do not kill, just keep you with 1 hp minimum
			map_freeblock_lock();
			status_fix_damage(NULL, bl, damage, clif_damage(bl, bl, tick, 0, 0, damage, 0, DMG_NORMAL, 0, false));
			if (sc->data[type]) {
				sc_timer_next(1000 + tick);
			}
			map_freeblock_unlock();
			return 0;
		}
		break;

	case SC_MAGNETICFIELD:
		if (--(sce->val3) >= 0) {
			struct block_list *src = map_id2bl(sce->val2);

			if (!src || (src && (status_isdead(src) || src->m != bl->m)))
				break;
			if (!status_charge(bl, 0, 50))
				status_zap(bl, 0, status->sp);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;

	case SC_INSPIRATION:
		if(--(sce->val4) >= 0) {
			int hp = status->max_hp * (35 - 5 * sce->val1) / 1000;
			int sp = status->max_sp * (45 - 5 * sce->val1) / 1000;

			if( !status_charge(bl,hp,sp) ) break;

			sc_timer_next(5000+tick);
			return 0;
		}
		break;

	case SC_RAISINGDRAGON:
		// 1% every 5 seconds [Jobbie]
		if( --(sce->val3)>0 && status_charge(bl, sce->val2, 0) ) {
			if( !sc->data[type] ) return 0;
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_TROPIC:
	case SC_CHILLY_AIR:
	case SC_WILD_STORM:
	case SC_UPHEAVAL:
	case SC_HEATER:
	case SC_COOLER:
	case SC_BLAST:
	case SC_CURSED_SOIL:
	case SC_PYROTECHNIC:
	case SC_AQUAPLAY:
	case SC_GUST:
	case SC_PETROLOGY:
	case SC_CIRCLE_OF_FIRE:
	case SC_FIRE_CLOAK:
	case SC_WATER_DROP:
	case SC_WATER_SCREEN:
	case SC_WIND_CURTAIN:
	case SC_WIND_STEP:
	case SC_STONE_SHIELD:
	case SC_SOLID_SKIN:
		if( !status_charge(bl,0,sce->val2) ) {
			struct block_list *s_bl = battle_get_master(bl);
			if (bl->type == BL_ELEM)
				elemental_change_mode(BL_CAST(BL_ELEM, bl), static_cast<e_mode>(MAX_ELESKILLTREE));
			if( s_bl )
				status_change_end(s_bl,static_cast<sc_type>(type+1),INVALID_TIMER);
			status_change_end(bl,type,INVALID_TIMER);
			break;
		}
		sc_timer_next(sce->val3 + tick);
		return 0;

	case SC_WATER_SCREEN_OPTION:
		status_heal(bl,1000,0,2);
		sc_timer_next(10000 + tick);
		return 0;

	case SC_TEARGAS:
		if( --(sce->val4) >= 0 ) {
			struct block_list *src = map_id2bl(sce->val3);
			int damage = sce->val2;

			map_freeblock_lock();
			clif_damage(bl, bl, tick, 0, 0, damage, 1, DMG_MULTI_HIT_ENDURE, 0, false);
			status_damage(src, bl, damage,0, 0, 1);
			if( sc->data[type] ) {
				sc_timer_next(2000 + tick);
			}
			map_freeblock_unlock();
			return 0;
		}
		break;
	case SC_TEARGAS_SOB:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl, ET_CRY);
			sc_timer_next(3000 + tick);
			return 0;
		}
		break;
	case SC_STOMACHACHE:
		if( --(sce->val4) >= 0 ) {
			status_charge(bl,0,sce->val2);	// Reduce 8 every 10 seconds.
			if( sd && !pc_issit(sd) ) { // Force to sit every 10 seconds.
				pc_setsit(sd);
				skill_sit(sd, 1);
				clif_sitting(bl);
			}
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;
	case SC_LEADERSHIP:
	case SC_GLORYWOUNDS:
	case SC_SOULCOLD:
	case SC_HAWKEYES:
		// They only end by status_change_end
		sc_timer_next(600000 + tick);
		return 0;
	case SC_MEIKYOUSISUI:
		if( --(sce->val4) >= 0 ) {
			status_heal(bl, status->max_hp * sce->val2 / 100, status->max_sp * sce->val3 / 100, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_KAGEMUSYA:
		if( --(sce->val2) >= 0 ) {
			if(!status_charge(bl, 0, 1)) break;
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_ANGRIFFS_MODUS:
		if(--(sce->val4) >= 0) { // Drain hp/sp
			if( !status_charge(bl,100,20) ) break;
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_CBC:
		if(--(sce->val4) >= 0) { // Drain hp/sp
			int hp=0;
			int sp = (status->max_sp * sce->val3) / 100;
			if(bl->type == BL_MOB) hp = sp*10;
			if( !status_charge(bl,hp,sp) )break;
			sc_timer_next(1000+tick);
			return 0;
		}
		break;
	case SC_FULL_THROTTLE:
		if( --(sce->val4) >= 0 ) {
			status_percent_damage(bl, bl, 0, sce->val2, false);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_REBOUND:
		if( --(sce->val4) >= 0 ) {
			clif_emotion(bl, ET_SWEAT);
			sc_timer_next(2000 + tick);
			return 0;
		}
		break;
	case SC_KINGS_GRACE:
		if( --(sce->val4) >= 0 ) {
			status_percent_heal(bl, sce->val2, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_FRIGG_SONG:
		if( --(sce->val4) >= 0 ) {
			status_heal(bl, sce->val3, 0, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_C_MARKER:
		if( --(sce->val4) >= 0 ) {
			TBL_PC *caster = map_id2sd(sce->val2);
			if (!caster || caster->bl.m != bl->m) //End the SC if caster isn't in same map
				break;
			sc_timer_next(1000 + tick);
			clif_crimson_marker(caster, bl, false);
			return 0;
		}
		break;
	case SC_STEALTHFIELD_MASTER:
		if (--(sce->val4) >= 0) {
			if (!status_charge(bl, 0, status->max_sp * 3 / 100))
				break;
			sc_timer_next(sce->val3 + tick);
			return 0;
		}
		break;
	case SC_VACUUM_EXTREME:
		if (sce->val4) {
			if (unit_movepos(bl, sce->val3>>16, sce->val3&0xFFFF, 0, false)) {
				clif_slide(bl, sce->val3>>16, sce->val3&0xFFFF);
				clif_fixpos(bl);
			}
			sc_timer_next(tick+sce->val4);
			sce->val4 = 0;
		}
		break;
	case SC_FIRE_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_FIRE)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_EARTH)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_WATER_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_WATER)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_FIRE)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_WIND_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_WIND)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_WATER)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;

	case SC_EARTH_INSIGNIA:
		if (--(sce->val4) >= 0) {
			if (status->def_ele == ELE_EARTH)
				status_heal(bl, status->max_hp / 100, 0, 1);
			else if (status->def_ele == ELE_WIND)
				status_zap(bl, status->max_hp / 100, 0);
			sc_timer_next(5000 + tick);
			return 0;
		}
		break;
	case SC_BITESCAR:
		if (--(sce->val4) >= 0) {
			status_percent_damage(bl, bl, -(sce->val2), 0, 0);
			sc_timer_next(1000 + tick);
			return 0;
		}
		break;
	case SC_FRESHSHRIMP:
		if (--(sce->val4) >= 0) {
			status_heal(bl, sce->val2, 0, 3);
			sc_timer_next((10000 - ((sce->val1 - 1) * 1000)) + tick);
			return 0;
		}
		break;
	case SC_DORAM_BUF_01:
		if( sd && --(sce->val4) >= 0 ) {
			if( status->hp < status->max_hp )
				status_heal(bl, 10, 0, 2);
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;
	case SC_DORAM_BUF_02:
		if( sd && --(sce->val4) >= 0 ) {
			if( status->sp < status->max_sp )
				status_heal(bl, 0, 5, 2);
			sc_timer_next(10000 + tick);
			return 0;
		}
		break;
	}

	// If status has an interval and there is at least 100ms remaining time, wait for next interval
	if(interval > 0 && sc->data[type] && sce->val4 >= 100) {
		sc_timer_next(min(sce->val4,interval)+tick);
		sce->val4 -= interval;
		if (dounlock)
			map_freeblock_unlock();
		return 0;
	}

	if (dounlock)
		map_freeblock_unlock();

	// Default for all non-handled control paths is to end the status
	return status_change_end( bl,type,tid );
}

/**
 * For each iteration of repetitive status
 * @param bl: Object [PC|MOB|HOM|MER|ELEM]
 * @param ap: va_list arguments (src, sce, type, tick)
 */
int status_change_timer_sub(struct block_list* bl, va_list ap)
{
	struct status_change* tsc;

	struct block_list* src = va_arg(ap,struct block_list*);
	struct status_change_entry* sce = va_arg(ap,struct status_change_entry*);
	enum sc_type type = (sc_type)va_arg(ap,int); // gcc: enum args get promoted to int
	t_tick tick = va_arg(ap,unsigned int);

	if (status_isdead(bl))
		return 0;

	tsc = status_get_sc(bl);

	switch( type ) {
	case SC_SIGHT: // Reveal hidden ennemy on 3*3 range
	case SC_CONCENTRATE:
		status_change_end(bl, SC_HIDING, INVALID_TIMER);
		status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
		status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
		status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
		if (tsc && tsc->data[SC__SHADOWFORM] && (sce && sce->val4 > 0 && sce->val4%2000 == 0) && // For every 2 seconds do the checking
			rnd()%100 < 100 - tsc->data[SC__SHADOWFORM]->val1 * 10) // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
		break;
	case SC_RUWACH: // Reveal hidden target and deal little dammages if enemy
		if (tsc && (tsc->data[SC_HIDING] || tsc->data[SC_CLOAKING] ||
				tsc->data[SC_CAMOUFLAGE] || tsc->data[SC_CLOAKINGEXCEED])) {
			status_change_end(bl, SC_HIDING, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKING, INVALID_TIMER);
			status_change_end(bl, SC_CAMOUFLAGE, INVALID_TIMER);
			status_change_end(bl, SC_CLOAKINGEXCEED, INVALID_TIMER);
			if(battle_check_target( src, bl, BCT_ENEMY ) > 0)
				skill_attack(BF_MAGIC,src,src,bl,AL_RUWACH,1,tick,0);
		}
		if (tsc && tsc->data[SC__SHADOWFORM] && (sce && sce->val4 > 0 && sce->val4%2000 == 0) && // For every 2 seconds do the checking
			rnd()%100 < 100 - tsc->data[SC__SHADOWFORM]->val1 * 10 ) { // [100 - (Skill Level x 10)] %
				status_change_end(bl, SC__SHADOWFORM, INVALID_TIMER);
				if (battle_check_target(src, bl, BCT_ENEMY) > 0)
					skill_attack(BF_MAGIC, src, src, bl, status_sc_get_skill(type), 1, tick, 0);
		}
		break;
	case SC_SIGHTBLASTER:
		if (battle_check_target( src, bl, BCT_ENEMY ) > 0 &&
			status_check_skilluse(src, bl, WZ_SIGHTBLASTER, 2))
		{
			if (sce) {
				struct skill_unit *su = NULL; 
				if(bl->type == BL_SKILL)
					su = (struct skill_unit *)bl;
				if (skill_attack(BF_MAGIC,src,src,bl,WZ_SIGHTBLASTER,sce->val1,tick,0x1000000)
					&& (!su || !su->group || !(skill_get_inf2(su->group->skill_id)&INF2_TRAP))) { // The hit is not counted if it's against a trap
					sce->val2 = 0; // This signals it to end.
				} else if((bl->type&BL_SKILL) && sce->val4%2 == 0) {
					//Remove trap immunity temporarily so it triggers if you still stand on it
					sce->val4++;
				}
			}
		}
		break;
	case SC_TINDER_BREAKER:
	case SC_CLOSECONFINE:{
		enum sc_type type2 = ((type==SC_CLOSECONFINE)?SC_CLOSECONFINE2:SC_TINDER_BREAKER2);
		// Lock char has released the hold on everyone...
		if (tsc && tsc->data[type2] && tsc->data[type2]->val2 == src->id) {
			tsc->data[type2]->val2 = 0;
			status_change_end(bl, type2, INVALID_TIMER);
		}
		break;
	}
	case SC_CURSEDCIRCLE_TARGET:
		if( tsc && tsc->data[SC_CURSEDCIRCLE_TARGET] && tsc->data[SC_CURSEDCIRCLE_TARGET]->val2 == src->id ) {
			clif_bladestop(bl, tsc->data[SC_CURSEDCIRCLE_TARGET]->val2, 0);
			status_change_end(bl, type, INVALID_TIMER);
		}
		break;
	}

	return 0;
}

/**
 * Clears buffs/debuffs on an object
 * @param bl: Object to clear [PC|MOB|HOM|MER|ELEM]
 * @param type: Type to remove
 *  SCCB_BUFFS: Clear Buffs
 *  SCCB_DEBUFFS: Clear Debuffs
 *  SCCB_REFRESH: Clear specific debuffs through RK_REFRESH
 *  SCCB_CHEM_PROTECT: Clear AM_CP_ARMOR/HELM/SHIELD/WEAPON
 *  SCCB_LUXANIMA: Bonus Script removed through RK_LUXANIMA
 */
void status_change_clear_buffs(struct block_list* bl, uint8 type)
{
	int i;
	struct status_change *sc= status_get_sc(bl);

	if (!sc || !sc->count)
		return;

	//Clears buffs with specified flag and type
	for (i = 0; i < SC_MAX; i++) {
		enum e_scb_flag flag = static_cast<e_scb_flag>(status_sc_get_flag((sc_type)i));

		if (!sc->data[i] || flag&SCF_NO_CLEARBUFF) //Skip status with SCF_NO_CLEARBUFF, no matter what
			continue;
		// &SCCB_LUXANIMA : Cleared by RK_LUXANIMA
		if (!(type&SCCB_LUXANIMA) && flag&SCF_REM_ON_LUXANIMA)
			continue;
		// &SCCB_CHEM_PROTECT : Clears AM_CP_ARMOR/HELP/SHIELD/WEAPON
		if (!(type&SCCB_CHEM_PROTECT) && flag&SCF_REM_CHEM_PROTECT)
			continue;
		// &SCCB_REFRESH : Cleared by RK_REFRESH
		if (!(type&SCCB_REFRESH) && flag&SCF_REM_ON_REFRESH)
			continue;
		// &SCCB_DEBUFFS : Clears debuffs
		if (!(type&SCCB_DEBUFFS) && flag&SCF_DEBUFF)
			continue;
		// &SCCB_BUFFS : Clears buffs
		if (!(type&SCCB_BUFFS) && !(flag&SCF_DEBUFF))
			continue;		
		if (i == SC_SATURDAYNIGHTFEVER || i == SC_BERSERK) // Mark to not lose HP
			sc->data[i]->val2 = 0;
		status_change_end(bl,(sc_type)i, INVALID_TIMER);
	}

	//Removes bonus_script
	if (bl->type == BL_PC) {
		i = 0;
		if (type&SCCB_BUFFS)    i |= BSF_REM_BUFF;
		if (type&SCCB_DEBUFFS)  i |= BSF_REM_DEBUFF;
		if (type&SCCB_REFRESH)  i |= BSF_REM_ON_REFRESH;
		if (type&SCCB_LUXANIMA) i |= BSF_REM_ON_LUXANIMA;
		pc_bonus_script_clear(BL_CAST(BL_PC,bl),i);
	}

	// Cleaning all extras vars
	sc->comet_x = 0;
	sc->comet_y = 0;
#ifndef RENEWAL
	sc->sg_counter = 0;
#endif
	sc->bs_counter = 0;

	return;
}

/**
 * Infect a user with status effects (SC_DEADLYINFECT)
 * @param src: Object initiating change on bl [PC|MOB|HOM|MER|ELEM]
 * @param bl: Object to change
 * @param type: 0 - Shadow Chaser attacking, 1 - Shadow Chaser being attacked
 * @return 1: Success 0: Fail
 */
int status_change_spread(struct block_list *src, struct block_list *bl, bool type)
{
	int i, flag = 0;
	struct status_change *sc = status_get_sc(src);
	const struct TimerData *timer = NULL;
	t_tick tick;
	struct status_change_data data;

	if( !sc || !sc->count )
		return 0;

	tick = gettick();

	// Status Immunity resistance
	if (status_bl_has_mode(src,MD_STATUS_IMMUNE) || status_bl_has_mode(bl,MD_STATUS_IMMUNE))
		return 0;

	for( i = SC_COMMON_MIN; i < SC_MAX; i++ ) {
		if( !sc->data[i] || i == SC_COMMON_MAX )
			continue;
		if (sc->data[i]->timer != INVALID_TIMER) {
			timer = get_timer(sc->data[i]->timer);
			if (timer == NULL || timer->func != status_change_timer || DIFF_TICK(timer->tick, tick) < 0)
				continue;
		}

		switch( i ) {
			// Debuffs that can be spread.
			// NOTE: We'll add/delete SCs when we are able to confirm it.
			case SC_DEATHHURT:
			case SC_PARALYSE:
				if (type)
					continue;
			case SC_CURSE:
			case SC_SILENCE:
			case SC_CONFUSION:
			case SC_BLIND:
			case SC_HALLUCINATION:
			case SC_SIGNUMCRUCIS:
			case SC_DECREASEAGI:
			//case SC_SLOWDOWN:
			//case SC_MINDBREAKER:
			//case SC_WINKCHARM:
			//case SC_STOP:
			case SC_ORCISH:
			//case SC_STRIPWEAPON: // Omg I got infected and had the urge to strip myself physically.
			//case SC_STRIPSHIELD: // No this is stupid and shouldnt be spreadable at all.
			//case SC_STRIPARMOR: // Disabled until I can confirm if it does or not. [Rytech]
			//case SC_STRIPHELM:
			//case SC__STRIPACCESSORY:
			//case SC_BITE:
			case SC_FEAR:
			case SC_FREEZING:
			case SC_VENOMBLEED:
				if (sc->data[i]->timer != INVALID_TIMER)
					data.tick = DIFF_TICK(timer->tick, tick);
				else
					data.tick = INFINITE_TICK;
				break;
			// Special cases
			case SC_TOXIN:
			case SC_MAGICMUSHROOM:
			case SC_PYREXIA:
			case SC_LEECHESEND:
				if (type)
					continue;
			case SC_POISON:
			case SC_DPOISON:
			case SC_BLEEDING:
			case SC_BURNING:
				if (sc->data[i]->timer != INVALID_TIMER)
					data.tick = DIFF_TICK(timer->tick, tick) + sc->data[i]->val4;
				else
					data.tick = INFINITE_TICK;
				break;
			default:
				continue;
		}
		if( i ) {
			data.val1 = sc->data[i]->val1;
			data.val2 = sc->data[i]->val2;
			data.val3 = sc->data[i]->val3;
			data.val4 = sc->data[i]->val4;
			status_change_start(src,bl,(sc_type)i,10000,data.val1,data.val2,data.val3,data.val4,data.tick,SCSTART_NOAVOID|SCSTART_NOTICKDEF|SCSTART_NORATEDEF);
			flag = 1;
		}
	}

	return flag;
}

/**
 * Applying natural heal bonuses (sit, skill, homun, etc...)
 * TODO: the va_list doesn't seem to be used, safe to remove?
 * @param bl: Object applying bonuses to [PC|HOM|MER|ELEM]
 * @param args: va_list arguments
 * @return which regeneration bonuses have been applied (flag)
 */
static t_tick natural_heal_prev_tick,natural_heal_diff_tick;
static int status_natural_heal(struct block_list* bl, va_list args)
{
	struct regen_data *regen;
	struct status_data *status;
	struct status_change *sc;
	struct unit_data *ud;
	struct view_data *vd = NULL;
	struct regen_data_sub *sregen;
	struct map_session_data *sd;
	int rate, multi = 1, flag;

	regen = status_get_regen_data(bl);
	if (!regen)
		return 0;
	status = status_get_status_data(bl);
	sc = status_get_sc(bl);
	if (sc && !sc->count)
		sc = NULL;
	sd = BL_CAST(BL_PC,bl);

	flag = regen->flag;
	if (flag&RGN_HP && (status->hp >= status->max_hp || regen->state.block&1))
		flag &= ~(RGN_HP|RGN_SHP);
	if (flag&RGN_SP && (status->sp >= status->max_sp || regen->state.block&2))
		flag &= ~(RGN_SP|RGN_SSP);

	if (flag && (
		status_isdead(bl) ||
		(sc && (sc->option&(OPTION_HIDE|OPTION_CLOAK|OPTION_CHASEWALK) || sc->data[SC__INVISIBILITY]))
	))
		flag = RGN_NONE;

	if (sd) {
		if (sd->hp_loss.value || sd->sp_loss.value)
			pc_bleeding(sd, natural_heal_diff_tick);
		if (sd->hp_regen.value || sd->sp_regen.value)
			pc_regen(sd, natural_heal_diff_tick);
	}

	if(flag&(RGN_SHP|RGN_SSP) && regen->ssregen &&
		(vd = status_get_viewdata(bl)) && vd->dead_sit == 2)
	{ // Apply sitting regen bonus.
		sregen = regen->ssregen;
		if(flag&(RGN_SHP)) { // Sitting HP regen
			rate = (int)(natural_heal_diff_tick * (sregen->rate.hp / 100.));
			if (regen->state.overweight)
				rate >>= 1; // Half as fast when overweight.
			sregen->tick.hp += rate;
			while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval) {
				sregen->tick.hp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp) { // Full
					flag &= ~(RGN_HP|RGN_SHP);
					break;
				}
			}
		}
		if(flag&(RGN_SSP)) { // Sitting SP regen
			rate = (int)(natural_heal_diff_tick * (sregen->rate.sp / 100.));
			if (regen->state.overweight)
				rate >>= 1; // Half as fast when overweight.
			sregen->tick.sp += rate;
			while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval) {
				sregen->tick.sp -= battle_config.natural_heal_skill_interval;
				if(status_heal(bl, 0, sregen->sp, 3) < sregen->sp) { // Full
					flag &= ~(RGN_SP|RGN_SSP);
					break;
				}
			}
		}
	}

	if (flag && regen->state.overweight)
		flag = RGN_NONE;

	ud = unit_bl2ud(bl);

	if (flag&(RGN_HP|RGN_SHP|RGN_SSP) && ud && ud->walktimer != INVALID_TIMER) {
		flag &= ~(RGN_SHP|RGN_SSP);
		if(!regen->state.walk)
			flag &= ~RGN_HP;
	}

	if (!flag)
		return 0;

	if (flag&(RGN_HP|RGN_SP)) {
		if(!vd)
			vd = status_get_viewdata(bl);
		if(vd && vd->dead_sit == 2)
			multi += 1; //This causes the interval to be halved
		if(regen->state.gc)
			multi += 1; //This causes the interval to be halved
	}

	// Natural Hp regen
	if (flag&RGN_HP) {
		rate = (int)(natural_heal_diff_tick * (regen->rate.hp/100. * multi));
		if (ud && ud->walktimer != INVALID_TIMER)
			rate /= 2;
		// Homun HP regen fix (they should regen as if they were sitting (twice as fast)
		if(bl->type == BL_HOM)
			rate *= 2;

		regen->tick.hp += rate;

		if(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval) {
			int val = 0;
			do {
				val += regen->hp;
				regen->tick.hp -= battle_config.natural_healhp_interval;
			} while(regen->tick.hp >= (unsigned int)battle_config.natural_healhp_interval);
			if (status_heal(bl, val, 0, 1) < val)
				flag &= ~RGN_SHP; // Full.
		}
	}

	// Natural SP regen
	if(flag&RGN_SP) {
		rate = (int)(natural_heal_diff_tick * (regen->rate.sp/100. * multi));
		// Homun SP regen fix (they should regen as if they were sitting (twice as fast)
		if(bl->type==BL_HOM)
			rate *= 2;
#ifdef RENEWAL
		if (bl->type == BL_PC && (((TBL_PC*)bl)->class_&MAPID_UPPERMASK) == MAPID_MONK &&
			sc && sc->data[SC_EXPLOSIONSPIRITS] && (!sc->data[SC_SPIRIT] || sc->data[SC_SPIRIT]->val2 != SL_MONK))
			rate /= 2; // Tick is doubled in Fury state
#endif
		regen->tick.sp += rate;

		if(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval) {
			int val = 0;
			do {
				val += regen->sp;
				regen->tick.sp -= battle_config.natural_healsp_interval;
			} while(regen->tick.sp >= (unsigned int)battle_config.natural_healsp_interval);
			if (status_heal(bl, 0, val, 1) < val)
				flag &= ~RGN_SSP; // full.
		}
	}

	if (!regen->sregen)
		return flag;

	// Skill regen
	sregen = regen->sregen;

	if(flag&RGN_SHP) { // Skill HP regen
		sregen->tick.hp += (int)(natural_heal_diff_tick * (sregen->rate.hp / 100.));

		while(sregen->tick.hp >= (unsigned int)battle_config.natural_heal_skill_interval) {
			sregen->tick.hp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, sregen->hp, 0, 3) < sregen->hp)
				break; // Full
		}
	}
	if(flag&RGN_SSP) { // Skill SP regen
		sregen->tick.sp += (int)(natural_heal_diff_tick * (sregen->rate.sp /100.));
		while(sregen->tick.sp >= (unsigned int)battle_config.natural_heal_skill_interval) {
			int val = sregen->sp;
			if (sd && sd->state.doridori) {
				val *= 2;
				sd->state.doridori = 0;
				if ((rate = pc_checkskill(sd,TK_SPTIME)))
					sc_start(bl,bl,skill_get_sc(TK_SPTIME),
						100,rate,skill_get_time(TK_SPTIME, rate));
				if (
					(sd->class_&MAPID_UPPERMASK) == MAPID_STAR_GLADIATOR &&
					rnd()%10000 < battle_config.sg_angel_skill_ratio
				) { // Angel of the Sun/Moon/Star
					clif_feel_hate_reset(sd);
					pc_resethate(sd);
					pc_resetfeel(sd);
				}
			}
			sregen->tick.sp -= battle_config.natural_heal_skill_interval;
			if(status_heal(bl, 0, val, 3) < val)
				break; // Full
		}
	}
	return flag;
}

/**
 * Natural heal main timer
 * @param tid: Timer ID
 * @param tick: Current tick (time)
 * @param id: Object ID to heal
 * @param data: data pushed through timer function
 * @return 0
 */
static TIMER_FUNC(status_natural_heal_timer){
	natural_heal_diff_tick = DIFF_TICK(tick,natural_heal_prev_tick);
	map_foreachregen(status_natural_heal);
	natural_heal_prev_tick = tick;
	return 0;
}

/**
 * Get the chance to upgrade a piece of equipment
 * @param wlv: The weapon type of the item to refine (see see enum refine_type)
 * @param refine: The target's refine level
 * @return The chance to refine the item, in percent (0~100)
 */
int status_get_refine_chance(enum refine_type wlv, int refine, bool enriched)
{
	if ( refine < 0 || refine >= MAX_REFINE)
		return 0;
	
	int type = enriched ? 1 : 0;
	if (battle_config.event_refine_chance)
		type |= 2;

	return refine_info[wlv].chance[type][refine];
}

/**
 * Check if status is disabled on a map
 * @param scdb: Status Change data
 * @param mapIsVS: If the map is a map_flag_vs type
 * @param mapisPVP: If the map is a PvP type
 * @param mapIsGVG: If the map is a map_flag_gvg type
 * @param mapIsBG: If the map is a Battleground type
 * @param mapZone: Map Zone type
 * @param mapIsTE: If the map us WOE TE
 * @return True - SC disabled on map; False - SC not disabled on map/Invalid SC
 */
static bool status_change_isDisabledOnMap_(std::shared_ptr<s_status_change_db> scdb, bool mapIsVS, bool mapIsPVP, bool mapIsGVG, bool mapIsBG, unsigned int mapZone, bool mapIsTE)
{
	if (!scdb || !scdb->disabledon)
		return false;

	if ((!mapIsVS && scdb->disabledon&1) ||
		(mapIsPVP && scdb->disabledon&2) ||
		(mapIsGVG && scdb->disabledon&4) ||
		(mapIsBG && scdb->disabledon&8) ||
		(mapIsTE && scdb->disabledon&16) ||
		(scdb->disabledon&(mapZone)))
	{
		return true;
	}

	return false;
}

/**
 * Clear a status if it is disabled on a map
 * @param bl: Block list data
 * @param sc: Status Change data
 */
void status_change_clear_onChangeMap(struct block_list *bl, struct status_change *sc)
{
	nullpo_retv(bl);

	if (sc && sc->count) {
		struct map_data *mapdata = map_getmapdata(bl->m);
		unsigned short i;
		bool mapIsVS = mapdata_flag_vs2(mapdata);
		bool mapIsPVP = mapdata->flag[MF_PVP] != 0;
		bool mapIsGVG = mapdata_flag_gvg2_no_te(mapdata);
		bool mapIsBG = mapdata->flag[MF_BATTLEGROUND] != 0;
		bool mapIsTE = mapdata_flag_gvg2_te(mapdata);
		unsigned int mapZone = mapdata->zone << 3;

		for (i = 0; i < SC_MAX; i++) {
			auto scdb = status_get((sc_type)i);

			if (!sc->data[i] || !scdb || !scdb->disabledon)
				continue;

			if (status_change_isDisabledOnMap_(scdb, mapIsVS, mapIsPVP, mapIsGVG, mapIsBG, mapZone, mapIsTE))
				status_change_end(bl, (sc_type)i, INVALID_TIMER);
		}
	}
}

/**
 * Read sizefix database for attack calculations
 * @param fields: Fields passed from sv_readdb
 * @param columns: Columns passed from sv_readdb function call
 * @param current: Current row being read into atkmods array
 * @return True
 */
static bool status_readdb_sizefix(char* fields[], int columns, int current)
{
	unsigned int i;

	for(i = 0; i < MAX_WEAPON_TYPE; i++)
		atkmods[current][i] = atoi(fields[i]);

	return true;
}

/**
 * Reads and parses an entry from the refine_db
 * @param node: The YAML node containing the entry
 * @param refine_info_index: The sequential index of the current entry
 * @param file_name: File name for displaying only
 * @return True on success or false on failure
 */
static bool status_yaml_readdb_refine_sub(const YAML::Node &node, int refine_info_index, const std::string &file_name) {
	if (refine_info_index < 0 || refine_info_index >= REFINE_TYPE_MAX)
		return false;

	int bonus_per_level = node["StatsPerLevel"].as<int>();
	int random_bonus_start_level = node["RandomBonusStartLevel"].as<int>();
	int random_bonus = node["RandomBonusValue"].as<int>();

	if (file_name.find("import") != std::string::npos) { // Import file, reset refine bonus before calculation
		for (int refine_level = 0; refine_level < MAX_REFINE; ++refine_level)
			refine_info[refine_info_index].bonus[refine_level] = 0;
	}

	const YAML::Node &costs = node["Costs"];

	for (const auto costit : costs) {
		const YAML::Node &type = costit;
		int idx = 0, price;
		unsigned short material;
		const std::string keys[] = { "Type", "Price", "Material" };

		for (int i = 0; i < ARRAYLENGTH(keys); i++) {
			if (!type[keys[i]].IsDefined())
				ShowWarning("status_yaml_readdb_refine_sub: Invalid refine cost with undefined " CL_WHITE "%s" CL_RESET "in file" CL_WHITE "%s" CL_RESET ".\n", keys[i].c_str(), file_name.c_str());
		}

		std::string refine_cost_const = type["Type"].as<std::string>();
		if (ISDIGIT(refine_cost_const[0]))
			idx = atoi(refine_cost_const.c_str());
		else
			script_get_constant(refine_cost_const.c_str(), &idx);
		price = type["Price"].as<int>();
		material = type["Material"].as<uint16>();

		refine_info[refine_info_index].cost[idx].nameid = material;
		refine_info[refine_info_index].cost[idx].zeny = price;
	}

	const YAML::Node &rates = node["Rates"];

	for (const auto rateit : rates) {
		const YAML::Node &level = rateit;
		int refine_level = level["Level"].as<int>() - 1;

		if (refine_level >= MAX_REFINE)
			continue;

		if (level["NormalChance"].IsDefined())
			refine_info[refine_info_index].chance[REFINE_CHANCE_NORMAL][refine_level] = level["NormalChance"].as<int>();
		if (level["EnrichedChance"].IsDefined())
			refine_info[refine_info_index].chance[REFINE_CHANCE_ENRICHED][refine_level] = level["EnrichedChance"].as<int>();
		if (level["EventNormalChance"].IsDefined())
			refine_info[refine_info_index].chance[REFINE_CHANCE_EVENT_NORMAL][refine_level] = level["EventNormalChance"].as<int>();
		if (level["EventEnrichedChance"].IsDefined())
			refine_info[refine_info_index].chance[REFINE_CHANCE_EVENT_ENRICHED][refine_level] = level["EventEnrichedChance"].as<int>();
		if (level["Bonus"].IsDefined())
			refine_info[refine_info_index].bonus[refine_level] = level["Bonus"].as<int>();

		if (refine_level >= random_bonus_start_level - 1)
			refine_info[refine_info_index].randombonus_max[refine_level] = random_bonus * (refine_level - random_bonus_start_level + 2);
	}
	for (int refine_level = 0; refine_level < MAX_REFINE; ++refine_level)
		refine_info[refine_info_index].bonus[refine_level] += bonus_per_level + (refine_level > 0 ? refine_info[refine_info_index].bonus[refine_level - 1] : 0);

	return true;
}

/**
 * Loads refine values from the refine_db
 * @param directory: Location of refine_db file
 * @param file: File name
 */
static void status_yaml_readdb_refine(const std::string &directory, const std::string &file) {
	int count = 0;
	const std::string labels[] = { "Armor", "WeaponLv1", "WeaponLv2", "WeaponLv3", "WeaponLv4", "Shadow" };
	const std::string current_file = directory + "/" + file;
	YAML::Node config;

	try {
		config = YAML::LoadFile(current_file);
	}
	catch (...) {
		ShowError("Failed to read '" CL_WHITE "%s" CL_RESET "'.\n", current_file.c_str());
		return;
	}

	for (int i = 0; i < ARRAYLENGTH(labels); i++) {
		const YAML::Node &node = config[labels[i]];

		if (node.IsDefined() && status_yaml_readdb_refine_sub(node, i, current_file))
			count++;
	}
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, current_file.c_str());
}

/**
 * Returns refine cost (zeny or item) for a weapon level.
 * @param weapon_lv Weapon level
 * @param type Refine type (can be retrieved from refine_cost_type enum)
 * @param what true = returns zeny, false = returns item id
 * @return Refine cost for a weapon level
 */
int status_get_refine_cost(int weapon_lv, int type, bool what) {
	return what ? refine_info[weapon_lv].cost[type].zeny : refine_info[weapon_lv].cost[type].nameid;
}

/**
 * Read attribute fix database for attack calculations
 * Function stores information in the attr_fix_table
 * @return True
 */
static bool status_readdb_attrfix(const char *basedir,bool silent)
{
	FILE *fp;
	char line[512], path[512];
	int entries = 0;


	sprintf(path, "%s/attr_fix.txt", basedir);
	fp = fopen(path,"r");
	if (fp == NULL) {
		if (silent==0)
			ShowError("Can't read %s\n", path);
		return 1;
	}
	while (fgets(line, sizeof(line), fp)) {
		int lv, i, j;
		if (line[0] == '/' && line[1] == '/')
			continue;

		lv = atoi(line);
		if (!CHK_ELEMENT_LEVEL(lv))
			continue;

		for (i = 0; i < ELE_ALL;) {
			char *p;
			if (!fgets(line, sizeof(line), fp))
				break;
			if (line[0]=='/' && line[1]=='/')
				continue;

			for (j = 0, p = line; j < ELE_ALL && p; j++) {
				while (*p == 32) //skipping space (32=' ')
					p++;
                                //TODO seem unsafe to continue without check
				attr_fix_table[lv-1][i][j] = atoi(p);
				p = strchr(p,',');
				if(p)
					*p++=0;
			}

			i++;
		}
		entries++;
	}
	fclose(fp);
	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", entries, path);
	return true;
}

static void yaml_invalid_warning(const char* fmt, const YAML::Node &node, const std::string &file) {
	YAML::Emitter out;
	out << node;
	ShowWarning(fmt, file.c_str());
	ShowMessage("%s\n", out.c_str());
}

/**
* Reads and parses an entry from the status_db.
* @param node: YAML node containing the entry.
* @param n: The sequential index of the current entry.
* @param source: The source YAML file.
* @return True on successful parse or false otherwise
*/
bool status_read_status_db_sub(const YAML::Node &node, int n, const std::string &source)
{
	int sc_id = 0;
	std::string name;
	bool existing = false;

	try {
		name = node["Status"].as<std::string>();
	}
	catch (...) {
		yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid Name field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
		return false;
	}
	if (name.compare(0, 3, "SC_")) {
		ShowWarning("status_read_status_db_sub: Invalid status Name %s. Expected a constant with 'SC_' prefix in \"%s\", skipping.\n", name.c_str(), source.c_str());
		return false;
	}
	if (!script_get_constant(name.c_str(), &sc_id) || !CHK_SC(sc_id)) {
		ShowWarning("status_read_status_db_sub: Invalid status Name %s. Non-existent constant in \"%s\", skipping.\n", name.c_str(), source.c_str());
		return false;
	}

	if (status_sc_exists((sc_type)sc_id)) {
		if (source.find("import") != std::string::npos) // Import file read-in, free previous value and store new value
			existing = true;
		else { // Normal file read-in
			ShowWarning("status_read_status_db_sub: Duplicate status %s (%d).\n", name.c_str(), sc_id);
			return false;
		}
	}

	if (!existing)
		statuses[sc_id] = std::make_shared<s_status_change_db>();
	auto &entry = statuses[sc_id];

	entry->type = (sc_type)sc_id;
	entry->icon = EFST_BLANK;
	entry->script = nullptr;

	if (node["Icon"]) {
		try {
			std::string icon = node["Icon"].as<std::string>();
			int icon_id;

			if (!script_get_constant(icon.c_str(), &icon_id) || icon_id <= EFST_BLANK || icon_id >= EFST_MAX)
				ShowWarning("status_read_status_db_sub: Invalid status Icon %s. Non-existent constant in \"%s\", defaulting to EFST_BLANK.\n", icon.c_str(), source.c_str());
			else
				entry->icon = static_cast<efst_type>(icon_id);
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid Icon field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["DurationLookup"]) {
		try {
			std::string skill = node["DurationLookup"].as<std::string>();
			int skill_id, idx;

			if (!(skill_id = skill_name2id(skill.c_str())) || !(idx = skill_get_index(skill_id)))
				ShowWarning("status_read_status_db_sub: Invalid status DurationLookup %s. Non-existent constant in \"%s\", skipping.\n", skill.c_str(), source.c_str());
			else
				entry->skill_id = skill_id;
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid DurationLookup field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["SCS"]) {
		try {
			const YAML::Node scs_list = node["SCS"];

			if (scs_list.IsSequence()) {
				for (int i = 0; i < scs_list.size(); i++) {
					std::string scs = scs_list[i].as<std::string>();
					int scs_id;

					if (!script_get_constant(scs.c_str(), &scs_id) || scs_id < SCS_NONE || scs_id >= SCS_MAX) {
						ShowWarning("status_read_status_db_sub: Invalid status SCS %s. Non-existent constant in \"%s\", skipping.\n", scs.c_str(), source.c_str());
						continue;
					}

					entry->state |= scs_id;
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status SCS format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid SCS field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["SCB"]) {
		try {
			const YAML::Node scb_list = node["SCB"];

			if (scb_list.IsSequence()) {
				for (int i = 0; i < scb_list.size(); i++) {
					std::string scb = scb_list[i].as<std::string>();
					int scb_id;

					if (!script_get_constant(scb.c_str(), &scb_id) || scb_id < SCB_NONE || scb_id >= SCB_MAX) {
						ShowWarning("status_read_status_db_sub: Invalid status SCB %s. Non-existent constant in \"%s\", skipping.\n", scb.c_str(), source.c_str());
						continue;
					}

					entry->calc_flag |= scb_id;
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status SCB format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid SCB field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["OPT1"]) {
		try {
			std::string opt = node["OPT1"].as<std::string>();
			int opt_id;

			if (!script_get_constant(opt.c_str(), &opt_id) || opt_id < OPT1_NONE || opt_id >= OPT1_MAX) {
				ShowWarning("status_read_status_db_sub: Invalid status OPT1 %s. Non-existent constant in \"%s\", defaulting to OPT1_NONE.\n", opt.c_str(), source.c_str());
				entry->opt1 = OPT1_NONE;
			} else
				entry->opt1 = static_cast<e_sc_opt1>(opt_id);
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid OPT1 field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["OPT2"]) {
		try {
			const YAML::Node opt_list = node["OPT2"];

			if (opt_list.IsSequence()) {
				for (int i = 0; i < opt_list.size(); i++) {
					std::string opt = opt_list[i].as<std::string>();
					int opt_id;

					if (!script_get_constant(opt.c_str(), &opt_id) || opt_id < OPT2_NONE || opt_id >= OPT2_MAX) {
						ShowWarning("status_read_status_db_sub: Invalid status OPT2 %s. Non-existent constant in \"%s\", skipping.\n", opt.c_str(), source.c_str());
						continue;
					}

					entry->opt2 |= opt_id;
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status OPT2 format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid OPT2 field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["OPT3"]) {
		try {
			const YAML::Node opt_list = node["OPT3"];

			if (opt_list.IsSequence()) {
				for (int i = 0; i < opt_list.size(); i++) {
					std::string opt = opt_list[i].as<std::string>();
					int opt_id;

					if (!script_get_constant(opt.c_str(), &opt_id) || opt_id < OPT3_NORMAL || opt_id >= OPT3_MAX) {
						ShowWarning("status_read_status_db_sub: Invalid status OPT3 %s. Non-existent constant in \"%s\", skipping.\n", opt.c_str(), source.c_str());
						continue;
					}

					entry->opt3 |= opt_id;
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status OPT3 format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid OPT3 field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["Option"]) {
		try {
			const YAML::Node option_list = node["Option"];

			if (option_list.IsSequence()) {
				for (int i = 0; i < option_list.size(); i++) {
					std::string option = option_list[i].as<std::string>();
					int option_id;

					if (!script_get_constant(option.c_str(), &option_id) || option_id < OPTION_NOTHING || option_id >= OPTION_MAX) {
						ShowWarning("status_read_status_db_sub: Invalid status Option %s. Non-existent constant in \"%s\", skipping.\n", option.c_str(), source.c_str());
						continue;
					}

					entry->look |= option_id;
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status Option format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid Option field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["Flag"]) {
		try {
			const YAML::Node flag_list = node["Flag"];

			if (flag_list.IsSequence()) {
				for (int i = 0; i < flag_list.size(); i++) {
					std::string flag = flag_list[i].as<std::string>();
					int flag_id;

					if (!script_get_constant(flag.c_str(), &flag_id) || flag_id < SCF_NONE || flag_id >= SCF_MAX) {
						ShowWarning("status_read_status_db_sub: Invalid status Flag %s. Non-existent constant in \"%s\", skipping.\n", flag.c_str(), source.c_str());
						continue;
					}

					entry->flag |= flag_id;
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status Flag format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid Flag field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["MinRate"]) {
		try {
			entry->min_rate = node["MinRate"].as<uint16>();
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid MinRate field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["MinDuration"]) {
		try {
			entry->min_duration = node["MinDuration"].as<t_tick>();
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid MinDuration field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["Fail"]) {
		try {
			const YAML::Node fail_list = node["Fail"];

			if (fail_list.IsSequence()) {
				for (int i = 0; i < fail_list.size(); i++) {
					std::string fail_sc = fail_list[i].as<std::string>();
					int fail_sc_id;

					if (!script_get_constant(fail_sc.c_str(), &fail_sc_id) || !CHK_SC(fail_sc_id)) {
						ShowWarning("status_read_status_db_sub: Invalid status Fail SC %s. Non-existent constant in \"%s\", skipping.\n", fail_sc.c_str(), source.c_str());
						continue;
					}

					entry->fail.push_back((sc_type)fail_sc_id);
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status Fail format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid Fail field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["End"]) {
		try {
			const YAML::Node end_list = node["End"];

			if (end_list.IsSequence()) {
				for (int i = 0; i < end_list.size(); i++) {
					std::string end_sc = end_list[i].as<std::string>();
					int end_sc_id;

					if (!script_get_constant(end_sc.c_str(), &end_sc_id) || !CHK_SC(end_sc_id)) {
						ShowWarning("status_read_status_db_sub: Invalid status End SC %s. Non-existent constant in \"%s\", skipping.\n", end_sc.c_str(), source.c_str());
						continue;
					}

					entry->end.push_back((sc_type)end_sc_id);
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status End format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid End field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["EndReturn"]) {
		try {
			entry->end_return = node["EndReturn"].as<bool>();
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid EndReturn field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["DisabledOn"]) {
		try {
			const YAML::Node disabledon_list = node["DisabledOn"];

			if (disabledon_list.IsSequence()) {
				for (int i = 0; i < disabledon_list.size(); i++) {
					int disabledon_id = disabledon_list[i].as<int>();

					if (disabledon_id <= 0) {
						ShowWarning("status_read_status_db_sub: Invalid status DisabledOn %d. Non-existent constant in \"%s\", skipping.\n", disabledon_id, source.c_str());
						continue;
					}

					entry->disabledon |= disabledon_id;
				}
			} else
				ShowWarning("status_read_status_db_sub: Invalid status DisabledOn format for status %s in \"%s\", skipping.\n", name.c_str(), source.c_str());
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid DisabledOn field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	if (node["Script"]) {
		try {
			std::string script_str = node["Script"].as<std::string>();
			struct script_code *script = parse_script(script_str.c_str(), source.c_str(), 0, 0);

			if (script) {
				entry->script = script;
				entry->calc_flag |= SCB_BASE; // Ask to recalculate
			}
		}
		catch (...) {
			yaml_invalid_warning("status_read_status_db_sub: Status definition with invalid Script field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, source);
			return false;
		}
	}

	StatusRelevantBLTypes[entry->icon] = BL_PC;
	if (entry->flag&SCF_BLEFFECT && entry->icon != EFST_BLANK)
		StatusRelevantBLTypes[entry->icon] |= BL_SCEFFECT;

	return true;
}

/**
 * Loads statuses from the status db.
 **/
static void status_readdb_status(const std::string &directory, const std::string &file) {
	const std::string current_file = directory + "/" + file;
	YAML::Node config;
	int count = 0;

	try {
		config = YAML::LoadFile(current_file);
	}
	catch (...) {
		ShowError("Failed to read '" CL_WHITE "%s" CL_RESET "'.\n", current_file.c_str());
		return;
	}

	for (const auto &node : config["Body"]) {
		if (node.IsDefined() && status_read_status_db_sub(node, count, current_file))
			count++;
	}

	ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' entries in '" CL_WHITE "%s" CL_RESET "'.\n", count, current_file.c_str());
}

/**
 * Sets defaults in tables and starts read db functions
 * sv_readdb reads the file, outputting the information line-by-line to
 * previous functions above, separating information by delimiter
 * DBs being read:
 *	attr_fix.txt: Attribute adjustment table for attacks
 *	size_fix.txt: Size adjustment table for weapons
 *	refine_db.txt: Refining data table
 * @return 0
 */
void status_readdb(void)
{
	int i, j, k;
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
		//add other path here
	};

	// Initialize databases to default

	// size_fix.txt
	for(i=0;i<ARRAYLENGTH(atkmods);i++)
		for(j=0;j<MAX_WEAPON_TYPE;j++)
			atkmods[i][j]=100;

	// refine_db.yml
	for(i=0;i<ARRAYLENGTH(refine_info);i++)
	{
		memset(refine_info[i].cost, 0, sizeof(struct refine_cost));
		for(j = 0; j < REFINE_CHANCE_TYPE_MAX; j++) {
			for(k=0;k<MAX_REFINE; k++)
			{
				refine_info[i].chance[j][k] = 100;
				refine_info[i].bonus[k] = 0;
				refine_info[i].randombonus_max[k] = 0;
			}
		}
	}

	// attr_fix.txt
	for(i=0;i<MAX_ELE_LEVEL;i++) {
		for(j=0;j<ELE_ALL;j++)
			for(k=0;k<ELE_ALL;k++)
				attr_fix_table[i][j][k]=100;
	}

	// read databases
	// path,filename,separator,mincol,maxcol,maxrow,func_parsor
	for(i=0; i<ARRAYLENGTH(dbsubpath); i++){
		size_t n1 = strlen(db_path)+strlen(dbsubpath[i])+1;
		size_t n2 = strlen(db_path)+strlen(DBPATH)+strlen(dbsubpath[i])+1;
		char* dbsubpath1 = (char*)aMalloc(n1+1);
		char* dbsubpath2 = (char*)aMalloc(n2+1);

		if(i==0) {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n2,"%s/%s%s",db_path,DBPATH,dbsubpath[i]);
		}
		else {
			safesnprintf(dbsubpath1,n1,"%s%s",db_path,dbsubpath[i]);
			safesnprintf(dbsubpath2,n1,"%s%s",db_path,dbsubpath[i]);
		}

		status_readdb_attrfix(dbsubpath2,i > 0); // !TODO use sv_readdb ?
		status_readdb_status(dbsubpath2, "status_db.yml");
		sv_readdb(dbsubpath1, "size_fix.txt",',',MAX_WEAPON_TYPE,MAX_WEAPON_TYPE,ARRAYLENGTH(atkmods),&status_readdb_sizefix, i > 0);

		status_yaml_readdb_refine(dbsubpath2, "refine_db.yml");

		aFree(dbsubpath1);
		aFree(dbsubpath2);
	}

	if (!battle_config.display_hallucination) { // Disable Hallucination.
		auto sc = status_get(SC_HALLUCINATION);

		if (sc)
			sc->icon = EFST_BLANK;
	}
}

/**
 * Reload status dbs
 **/
void status_reloaddb(void) {
	statuses.clear();
	status_readdb();
}

/**
 * Status db init and destroy.
 */
void do_init_status(void) {
	add_timer_func_list(status_change_timer,"status_change_timer");
	add_timer_func_list(status_natural_heal_timer,"status_natural_heal_timer");
	initDummyData();
	status_readdb();
	natural_heal_prev_tick = gettick();
	sc_data_ers = ers_new(sizeof(struct status_change_entry),"status.cpp::sc_data_ers",ERS_OPT_NONE);
	add_timer_interval(natural_heal_prev_tick + NATURAL_HEAL_INTERVAL, status_natural_heal_timer, 0, 0, NATURAL_HEAL_INTERVAL);
}

/** Destroy status data */
void do_final_status(void) {
	statuses.clear();
	ers_destroy(sc_data_ers);
}
