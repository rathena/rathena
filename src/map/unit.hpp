// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef UNIT_HPP
#define UNIT_HPP

#include <common/cbasetypes.hpp>
#include <common/timer.hpp>

#include "path.hpp" // struct walkpath_data
#include "skill.hpp" // struct skill_timerskill, struct skill_unit_group, struct skill_unit_group_tickset

enum sc_type : int16;
struct block_list;
struct unit_data;
class map_session_data;
enum clr_type : uint8;

extern const short dirx[DIR_MAX]; ///lookup to know where will move to x according dir
extern const short diry[DIR_MAX]; ///lookup to know where will move to y according dir

struct unit_data {
	struct block_list *bl; ///link to owner object BL_PC|BL_MOB|BL_PET|BL_NPC|BL_HOM|BL_MER|BL_ELEM
	struct walkpath_data walkpath;
	struct skill_timerskill *skilltimerskill[MAX_SKILLTIMERSKILL];
	std::vector<std::shared_ptr<s_skill_unit_group>> skillunits;
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	short attacktarget_lv;
	short to_x, to_y;
	uint8 sx, sy; // Subtile position (0-15, with 8 being center of cell)
	short skillx, skilly;
	uint16 skill_id, skill_lv;
	int skilltarget;
	int skilltimer;
	int target;
	int target_to;
	int attacktimer;
	int walktimer;
	int chaserange;
	bool stepaction; //Action should be executed on step [Playtester]
	int steptimer; //Timer that triggers the action [Playtester]
	uint16 stepskill_id, stepskill_lv; //Remembers skill that should be casted on step [Playtester]
	t_tick attackabletime;
	t_tick canact_tick;
	t_tick canmove_tick;
	bool immune_attack; ///< Whether the unit is immune to attacks
	uint8 dir;
	unsigned char target_count;
	struct s_udState {
		unsigned change_walk_target : 1 ;
		unsigned skillcastcancel : 1 ;
		unsigned attack_continue : 1 ;
		unsigned step_attack : 1;
		unsigned walk_easy : 1 ;
		unsigned running : 1;
		unsigned walk_script : 1;
		unsigned blockedmove : 1;
		unsigned blockedskill : 1;
		unsigned ignore_cell_stack_limit : 1;
		bool force_walk; ///< Used with script commands unitwalk/unitwalkto. Disables monster idle and random walk.
	} state;
	char walk_done_event[EVENT_NAME_LENGTH];
	char title[NAME_LENGTH];
	int32 group_id;

	std::vector<int> shadow_scar_timer;
};

struct view_data {
	uint16 class_;
	t_itemid
		weapon,
		shield, //Or left-hand weapon.
		robe,
		head_top,
		head_mid,
		head_bottom;
	uint16
		hair_style,
		hair_color,
		cloth_color,
		body_style;
	char sex;
	unsigned dead_sit : 2; // 0: Standing, 1: Dead, 2: Sitting
};

/// Enum for unit_blown_immune
enum e_unit_blown {
	UB_KNOCKABLE = 0, // Can be knocked back / stopped
	UB_NO_KNOCKBACK_MAP, // On a WoE/BG map
	UB_MD_KNOCKBACK_IMMUNE, // Target is MD_KNOCKBACK_IMMUNE
	UB_TARGET_BASILICA, // Target is in Basilica area
	UB_TARGET_NO_KNOCKBACK, // Target has 'special_state.no_knockback'
	UB_TARGET_TRAP, // Target is a trap that cannot be knocked back
};

/// Enum for unit_stop_walking
enum e_unit_stop_walking {
	USW_NONE = 0x0, /// Unit will keep walking to their original destination
	USW_FIXPOS = 0x1, /// Issue a fixpos packet afterwards
	USW_MOVE_ONCE = 0x2, /// Force the unit to move one cell if it hasn't yet
	USW_MOVE_FULL_CELL = 0x4, /// Enable moving to the next cell when unit was already half-way there (may cause on-touch/place side-effects, such as a scripted map change)
	USW_FORCE_STOP = 0x8, /// Force stop moving, even if walktimer is currently INVALID_TIMER
	USW_ALL = 0xf,
};

// PC, MOB, PET

// Does walk action for unit
int unit_walktoxy(struct block_list *bl, short x, short y, unsigned char flag);
int unit_walktobl(struct block_list *bl, struct block_list *target, int range, unsigned char flag);
void unit_run_hit(struct block_list *bl, status_change *sc, map_session_data *sd, enum sc_type type);
bool unit_run(struct block_list *bl, map_session_data *sd, enum sc_type type);
int unit_calc_pos(struct block_list *bl, int tx, int ty, uint8 dir);
TIMER_FUNC(unit_delay_walktoxy_timer);
TIMER_FUNC(unit_delay_walktobl_timer);

void unit_stop_walking_soon(struct block_list& bl);
// Causes the target object to stop moving.
int unit_stop_walking(struct block_list *bl,int type);
bool unit_can_move(struct block_list *bl);
int unit_is_walking(struct block_list *bl);
int unit_set_walkdelay(struct block_list *bl, t_tick tick, t_tick delay, int type);

t_tick unit_get_walkpath_time(struct block_list& bl);
t_tick unit_escape(struct block_list *bl, struct block_list *target, short dist, uint8 flag = 0);

// Instant unit changes
bool unit_movepos(struct block_list *bl, short dst_x, short dst_y, int easy, bool checkpath);
int unit_warp(struct block_list *bl, short map, short x, short y, clr_type type);
bool unit_setdir(block_list *bl, uint8 dir, bool send_update = true);
uint8 unit_getdir(struct block_list *bl);
int unit_blown(struct block_list* bl, int dx, int dy, int count, enum e_skill_blown flag);
enum e_unit_blown unit_blown_immune(struct block_list* bl, uint8 flag);

// Can-reach checks
bool unit_can_reach_pos(struct block_list *bl,int x,int y,int easy);
bool unit_can_reach_bl(struct block_list *bl,struct block_list *tbl, int range, int easy, short *x, short *y);

// Unit attack functions
int unit_stopattack(struct block_list *bl, va_list ap);
void unit_stop_attack(struct block_list *bl);
int unit_attack(struct block_list *src,int target_id,int continuous);
int unit_cancel_combo(struct block_list *bl);
bool unit_can_attack(struct block_list *bl, int target_id);

// Cast on a unit
int unit_skilluse_id(struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv);
int unit_skilluse_pos(struct block_list *src, short skill_x, short skill_y, uint16 skill_id, uint16 skill_lv);
int unit_skilluse_id2(struct block_list *src, int target_id, uint16 skill_id, uint16 skill_lv, int casttime, int castcancel, bool ignore_range = false);
int unit_skilluse_pos2( struct block_list *src, short skill_x, short skill_y, uint16 skill_id, uint16 skill_lv, int casttime, int castcancel, bool ignore_range = false);

// Step timer used for delayed attack and skill use
TIMER_FUNC(unit_step_timer);
void unit_stop_stepaction(struct block_list *bl);

// Cancel unit cast
int unit_skillcastcancel(struct block_list *bl, char type);

int unit_counttargeted(struct block_list *bl);
int unit_set_target(struct unit_data* ud, int target_id);

// unit_data
void unit_dataset(struct block_list *bl);
void unit_skillunit_maxcount(unit_data& ud, uint16 skill_id, int& maxcount);

// Remove unit
struct unit_data* unit_bl2ud(struct block_list *bl);
void unit_remove_map_pc(map_session_data *sd, clr_type clrtype);
void unit_refresh(struct block_list *bl, bool walking = false);
void unit_free_pc(map_session_data *sd);
#define unit_remove_map(bl,clrtype) unit_remove_map_(bl,clrtype,__FILE__,__LINE__,__func__)
int unit_remove_map_(struct block_list *bl, clr_type clrtype, const char* file, int line, const char* func);
int unit_free(struct block_list *bl, clr_type clrtype);
int unit_changetarget(block_list *bl,va_list ap);
void unit_changetarget_sub(unit_data& ud, block_list& target);

// Shadow Scar
void unit_addshadowscar(unit_data &ud, int interval);

void do_init_unit(void);
void do_final_unit(void);

#endif /* UNIT_HPP */
