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

extern const int16 dirx[DIR_MAX]; ///lookup to know where will move to x according dir
extern const int16 diry[DIR_MAX]; ///lookup to know where will move to y according dir

struct unit_data {
	block_list *bl; ///link to owner object BL_PC|BL_MOB|BL_PET|BL_NPC|BL_HOM|BL_MER|BL_ELEM
	struct walkpath_data walkpath;
	struct skill_timerskill *skilltimerskill[MAX_SKILLTIMERSKILL];
	std::vector<std::shared_ptr<s_skill_unit_group>> skillunits;
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	int16 attacktarget_lv;
	int16 to_x, to_y;
	uint8 sx, sy; // Subtile position (0-15, with 8 being center of cell)
	int16 skillx, skilly;
	uint16 skill_id, skill_lv;
	int32 skilltarget;
	int32 skilltimer;
	int32 target;
	int32 target_to;
	int32 attacktimer;
	int32 walktimer;
	int32 chaserange;
	bool stepaction; //Action should be executed on step [Playtester]
	int32 steptimer; //Timer that triggers the action [Playtester]
	uint16 stepskill_id, stepskill_lv; //Remembers skill that should be casted on step [Playtester]
	t_tick attackabletime;
	t_tick canact_tick;
	t_tick canmove_tick;
	t_tick endure_tick; // Time until which unit cannot be stopped
	t_tick dmg_tick; // Last time the unit was damaged by a source
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

	std::vector<int32> shadow_scar_timer;
	std::vector<int16> hatEffects;

	// Functions and struct to calculate and store exact position at a certain tick
	int16 getx(t_tick tick);
	int16 gety(t_tick tick);
	void getpos(int16 &x, int16 &y, uint8 &sx, uint8 &sy, t_tick tick);
private:
	void update_pos(t_tick tick);
	struct {
		int16 x;
		int16 y;
		uint8 sx;
		uint8 sy;
		t_tick tick;
	} pos;
};

struct view_data {
	int32 look[LOOK_MAX];
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
	USW_RELEASE_TARGET = 0x10, /// Release chase target
	USW_ALL = 0x1f,
};

/// Enum for delay events
enum e_delay_event {
	DELAY_EVENT_ATTACK = 1, /// Executed a normal attack
	DELAY_EVENT_CASTBEGIN_ID, /// Started casting a target skill
	DELAY_EVENT_CASTBEGIN_POS, /// Started casting a ground skill
	DELAY_EVENT_CASTEND, /// Finished casting a skill
	DELAY_EVENT_CASTCANCEL, /// Skill cast was cancelled
	DELAY_EVENT_DAMAGED, /// Got damaged
	DELAY_EVENT_PARRY, /// Parry activated
};

// PC, MOB, PET

// Does walk action for unit
int32 unit_walktoxy(block_list *bl, int16 x, int16 y, unsigned char flag);
int32 unit_walktobl(block_list *bl, block_list *target, int32 range, unsigned char flag);
void unit_run_hit(block_list *bl, status_change *sc, map_session_data *sd, enum sc_type type);
bool unit_run(block_list *bl, map_session_data *sd, enum sc_type type);
int32 unit_calc_pos(block_list *bl, int32 tx, int32 ty, uint8 dir);
TIMER_FUNC(unit_delay_walktoxy_timer);
TIMER_FUNC(unit_delay_walktobl_timer);

void unit_stop_walking_soon(block_list& bl, t_tick tick = gettick());
// Causes the target object to stop moving.
bool unit_stop_walking( block_list* bl, int32 type, t_tick canmove_delay = 0 );
bool unit_can_move(block_list *bl);
int32 unit_is_walking(block_list *bl);

// Delay functions
void unit_set_attackdelay(block_list& bl, t_tick tick, e_delay_event event);
int32 unit_set_walkdelay(block_list *bl, t_tick tick, t_tick delay, int32 type, uint16 skill_id = 0);

t_tick unit_get_walkpath_time(block_list& bl);
t_tick unit_escape(block_list *bl, block_list *target, int16 dist, uint8 flag = 0);

// Instant unit changes
bool unit_movepos(block_list *bl, int16 dst_x, int16 dst_y, int32 easy, bool checkpath);
int32 unit_warp(block_list *bl, int16 map, int16 x, int16 y, clr_type type);
bool unit_setdir(block_list *bl, uint8 dir, bool send_update = true);
uint8 unit_getdir(block_list *bl);
int32 unit_blown(block_list* bl, int32 dx, int32 dy, int32 count, enum e_skill_blown flag);
enum e_unit_blown unit_blown_immune(block_list* bl, uint8 flag);

// Can-reach checks
bool unit_can_reach_pos(block_list *bl,int32 x,int32 y,int32 easy);
bool unit_can_reach_bl(block_list *bl,block_list *tbl, int32 range, int32 easy, int16 *x, int16 *y);

// Unit attack functions
int32 unit_stopattack(block_list *bl, va_list ap);
void unit_stop_attack(block_list *bl);
int32 unit_attack(block_list *src,int32 target_id,int32 continuous);
int32 unit_cancel_combo(block_list *bl);
bool unit_can_attack(block_list *bl, int32 target_id);

// Cast on a unit
int32 unit_skilluse_id(block_list *src, int32 target_id, uint16 skill_id, uint16 skill_lv);
int32 unit_skilluse_pos(block_list *src, int16 skill_x, int16 skill_y, uint16 skill_id, uint16 skill_lv);
int32 unit_skilluse_id2(block_list *src, int32 target_id, uint16 skill_id, uint16 skill_lv, int32 casttime, int32 castcancel, bool ignore_range = false);
int32 unit_skilluse_pos2( block_list *src, int16 skill_x, int16 skill_y, uint16 skill_id, uint16 skill_lv, int32 casttime, int32 castcancel, bool ignore_range = false);

// Step timer used for delayed attack and skill use
TIMER_FUNC(unit_step_timer);
void unit_stop_stepaction(block_list *bl);

// Cancel unit cast
int32 unit_skillcastcancel(block_list *bl, char type);

int32 unit_counttargeted(block_list *bl);
int32 unit_set_target(struct unit_data* ud, int32 target_id);

// unit_data
void unit_dataset(block_list *bl);
void unit_skillunit_maxcount(unit_data& ud, uint16 skill_id, int& maxcount);

// Remove unit
struct unit_data* unit_bl2ud(block_list *bl);
void unit_remove_map_pc(map_session_data *sd, clr_type clrtype);
void unit_refresh(block_list *bl, bool walking = false);
void unit_free_pc(map_session_data *sd);
#define unit_remove_map(bl,clrtype) unit_remove_map_(bl,clrtype,__FILE__,__LINE__,__func__)
int32 unit_remove_map_(block_list *bl, clr_type clrtype, const char* file, int32 line, const char* func);
int32 unit_free(block_list *bl, clr_type clrtype);
int32 unit_changetarget(block_list *bl,va_list ap);
void unit_changetarget_sub(unit_data& ud, block_list& target);

// Shadow Scar
void unit_addshadowscar(unit_data &ud, int32 interval);

void do_init_unit(void);
void do_final_unit(void);

#endif /* UNIT_HPP */
