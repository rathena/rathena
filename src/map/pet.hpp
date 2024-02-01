// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PET_HPP
#define PET_HPP

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/mmo.hpp>
#include <common/timer.hpp>

#include "battle.hpp"
#include "mob.hpp"
#include "pc.hpp"
#include "script.hpp"
#include "status.hpp"
#include "unit.hpp"

#include <unordered_map>

#define MAX_PETLOOT_SIZE	30

struct s_pet_evo_data {
	uint16 target_mob_id;
	std::unordered_map<t_itemid, uint16> requirements;
};

/// Pet DB
struct s_pet_db {
	uint16 class_; ///< Monster ID
	t_itemid itemID; ///< Lure ID
	t_itemid EggID; ///< Egg ID
	t_itemid AcceID; ///< Accessory ID
	t_itemid FoodID; ///< Food ID
	uint16 fullness; ///< Amount of hunger decresed each hungry_delay interval
	uint32 hungry_delay; ///< Hunger value decrease each x seconds
	int32 hunger_increase; ///< Hunger increased every time the pet is fed.
	int32 r_hungry; ///< Intimacy increased after feeding
	int32 r_full; ///< Intimacy increased when over-fed
	uint32 intimate; ///< Initial intimacy value
	int32 die; ///< Intimacy increased when die
	int32 hungry_intimacy_dec; ///< Intimacy increased when hungry
	uint16 capture; ///< Capture success rate 10000 = 100%
	bool s_perfor; ///< Special performance
	uint16 attack_rate; ///< Rate of which the pet will attack (requires at least pet_support_min_friendly intimacy).
	uint16 defence_attack_rate; ///< Rate of which the pet will retaliate when master is being attacked (requires at least pet_support_min_friendly intimacy).
	uint16 change_target_rate; ///< Rate of which the pet will change its attack target.
	bool allow_autofeed; ///< Can this pet use auto feeding mechanic.
	std::unordered_map<uint16, std::shared_ptr<s_pet_evo_data>> evolution_data; ///< Data for evolving the pet.
	struct script_code
		*pet_support_script, ///< Script since pet hatched. For pet* script commands only.
		*pet_bonus_script; ///< Bonus script for this pet.

	~s_pet_db()
	{
		if( this->pet_support_script ){
			script_free_code(this->pet_support_script);
		}

		if( this->pet_bonus_script ){
			script_free_code(this->pet_bonus_script);
		}
	}
};

enum e_pet_itemtype : uint8 { PET_CATCH,PET_EGG,PET_EQUIP,PET_FOOD };

enum e_pet_catch : uint16 {
	PET_CATCH_FAIL = 0, ///< A catch attempt failed
	PET_CATCH_UNIVERSAL = 1, ///< The catch attempt is universal (ignoring MD_STATUS_IMMUNE/Boss)
	PET_CATCH_UNIVERSAL_ITEM = 2,
};

enum e_pet_intimate_level : uint16 {
	PET_INTIMATE_NONE = 0,
	PET_INTIMATE_AWKWARD = 1,
	PET_INTIMATE_SHY = 100,
	PET_INTIMATE_NEUTRAL = 250,
	PET_INTIMATE_CORDIAL = 750,
	PET_INTIMATE_LOYAL = 910,
	PET_INTIMATE_MAX = 1000
};

enum e_pet_hungry : uint16 {
	PET_HUNGRY_NONE = 0,
	PET_HUNGRY_VERY_HUNGRY = 10,
	PET_HUNGRY_HUNGRY = 25,
	PET_HUNGRY_NEUTRAL = 75,
	PET_HUNGRY_SATISFIED = 90,
	PET_HUNGRY_STUFFED = 100
};

struct pet_recovery { //Stat recovery
	enum sc_type type;	//Status Change id
	unsigned short delay; //How long before curing (secs).
	int timer;
};

struct pet_bonus {
	unsigned short type; //bStr, bVit?
	unsigned short val;	//value
	unsigned short duration; //in seconds
	unsigned short delay;	//Time before re-effect the bonus in seconds
	int timer;
};

struct pet_skill_attack { //Attack Skill
	unsigned short id;
	unsigned short lv; // Skill level
	unsigned short damage; // Fixed damage value of petskillattack2
	unsigned short div_; //0 = Normal skill. >0 = Fixed damage (lv), fixed div_.
	unsigned short rate; //Base chance of skill ocurrance (10 = 10% of attacks)
	unsigned short bonusrate; //How being 100% loyal affects cast rate (10 = At 1000 intimacy->rate+10%
};

struct pet_skill_support { //Support Skill
	unsigned short id;
	unsigned short lv;
	unsigned short hp; //Max HP% for skill to trigger (50 -> 50% for Magnificat)
	unsigned short sp; //Max SP% for skill to trigger (100 = no check)
	unsigned short delay; //Time (secs) between being able to recast.
	int timer;
};

struct pet_loot {
	struct item *item;
	unsigned short count;
	unsigned short weight;
	unsigned short max;
};

class PetDatabase : public TypesafeYamlDatabase<uint16,s_pet_db>{
public:
	PetDatabase() : TypesafeYamlDatabase( "PET_DB", 1 ){

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode( const ryml::NodeRef& node ) override;

	// Additional
	bool reload();
};

extern PetDatabase pet_db;

TIMER_FUNC(pet_endautobonus);

/// Pet AutoBonus bonus struct
struct s_petautobonus {
	int16 rate;
	uint16 atk_type;
	std::string bonus_script, other_script;
	t_tick duration;
	int32 timer;

	~s_petautobonus() {
		if (this->timer != INVALID_TIMER) {
			delete_timer(this->timer, pet_endautobonus);
			this->timer = INVALID_TIMER;
		}

		this->bonus_script.clear();
		this->other_script.clear();
	}

};

/// Pet Autobonus database wrapper
struct s_pet_autobonus_wrapper {
	script_code *script;

	~s_pet_autobonus_wrapper() {
		if (this->script != nullptr) {
			script_free_code(this->script);
			this->script = nullptr;
		}
	}
};

extern std::unordered_map<std::string, std::shared_ptr<s_pet_autobonus_wrapper>> pet_autobonuses;

struct pet_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data vd;
	struct s_pet pet;
	struct status_data status;
	std::shared_ptr<s_mob_db> db;
	int pet_hungry_timer;
	int target_id;
	struct {
		unsigned skillbonus : 1;
	} state;
	int move_fail_count;
	t_tick next_walktime,last_thinktime;
	unsigned short rate_fix;	//Support rate as modified by intimacy (1000 = 100%) [Skotlex]

	struct pet_recovery* recovery;
	struct pet_bonus* bonus;
	struct pet_skill_attack* a_skill;
	struct pet_skill_support* s_skill;
	struct pet_loot* loot;
	std::vector<std::shared_ptr<s_petautobonus>> autobonus, autobonus2, autobonus3;

	int masterteleport_timer;
	map_session_data *master;

	std::shared_ptr<s_pet_db> get_pet_db() {
		return pet_db.find(this->pet.class_);
	}

	int get_pet_walk_speed() {
		switch (battle_config.pet_walk_speed) {
			default:
			case 1: // Master
				return this->master->battle_status.speed;
			case 2: // DEFAULT_WALK_SPEED
				return DEFAULT_WALK_SPEED;
			case 3: // Mob database
				return this->db->status.speed;
		}
	}
};

bool pet_create_egg(map_session_data *sd, t_itemid item_id);
int pet_hungry_val(struct pet_data *pd);
void pet_set_intimate(struct pet_data *pd, int value);
int pet_target_check(struct pet_data *pd,struct block_list *bl,int type);
void pet_unlocktarget(struct pet_data *pd);
int pet_sc_check(map_session_data *sd, int type); //Skotlex
std::shared_ptr<s_pet_db> pet_db_search(int key, enum e_pet_itemtype type);
int pet_hungry_timer_delete(struct pet_data *pd);
bool pet_data_init(map_session_data *sd, struct s_pet *pet);
bool pet_return_egg( map_session_data *sd, struct pet_data *pd );
int pet_birth_process(map_session_data *sd, struct s_pet *pet);
int pet_recv_petdata(uint32 account_id,struct s_pet *p,int flag);
int pet_select_egg(map_session_data *sd,short egg_index);
int pet_catch_process1(map_session_data *sd,int target_class);
int pet_catch_process2(map_session_data *sd,int target_id);
bool pet_get_egg(uint32 account_id, short pet_class, int pet_id);
int pet_menu(map_session_data *sd,int menunum);
int pet_change_name(map_session_data *sd,char *name);
int pet_change_name_ack(map_session_data *sd, char* name, int flag);
int pet_equipitem(map_session_data *sd,int index);
int pet_lootitem_drop(struct pet_data *pd,map_session_data *sd);
int pet_attackskill(struct pet_data *pd, int target_id);
TIMER_FUNC(pet_skill_support_timer); // [Skotlex]
TIMER_FUNC(pet_skill_bonus_timer); // [Valaris]
TIMER_FUNC(pet_recovery_timer); // [Valaris]
TIMER_FUNC(pet_heal_timer); // [Valaris]
int pet_egg_search(map_session_data *sd, int pet_id);
void pet_evolution(map_session_data *sd, int16 pet_id);
int pet_food(map_session_data *sd, struct pet_data *pd);
void pet_clear_support_bonuses(map_session_data *sd);

#define pet_stop_walking(pd, type) unit_stop_walking(&(pd)->bl, type)
#define pet_stop_attack(pd) unit_stop_attack(&(pd)->bl)

bool pet_addautobonus(std::vector<std::shared_ptr<s_petautobonus>> &bonus, const std::string &script, int16 rate, uint32 dur, uint16 atk_type, const std::string &other_script, bool onskill);
void pet_exeautobonus(map_session_data &sd, std::vector<std::shared_ptr<s_petautobonus>> *bonus, std::shared_ptr<s_petautobonus> &autobonus);
void pet_delautobonus(map_session_data &sd, std::vector<std::shared_ptr<s_petautobonus>> &bonus, bool restore);

void do_init_pet(void);
void do_final_pet(void);

#endif /* PET_HPP */
