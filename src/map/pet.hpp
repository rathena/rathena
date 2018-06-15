// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PET_HPP_
#define _PET_HPP_

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp"

#include "script.hpp"
#include "status.hpp"
#include "unit.hpp"

#define MAX_PETLOOT_SIZE	30

/// Pet DB
struct s_pet_db {
	short class_; ///< Monster ID
	char name[NAME_LENGTH], ///< AEGIS name
		jname[NAME_LENGTH]; ///< English name
	unsigned short itemID; ///< Lure ID
	unsigned short EggID; ///< Egg ID
	unsigned short AcceID; ///< Accessory ID
	unsigned short FoodID; ///< Food ID
	int fullness; ///< Amount of hunger decresed each hungry_delay interval
	int hungry_delay; ///< Hunger value decrease each x seconds
	int r_hungry; ///< Intimacy increased after feeding
	int r_full; ///< Intimacy reduced when over-fed
	int intimate; ///< Initial intimacy value
	int die; ///< Intimacy decreased when die
	int capture; ///< Capture success rate 1000 = 100%
	int speed; ///< Walk speed
	char s_perfor; ///< Special performance
	int talk_convert_class; ///< Disables pet talk (instead of talking they emote  with /!.) (?)
	int attack_rate; ///< Rate of which the pet will attack (requires at least pet_support_min_friendly intimacy).
	int defence_attack_rate; ///< Rate of which the pet will retaliate when master is being attacked (requires at least pet_support_min_friendly intimacy).
	int change_target_rate; ///< Rate of which the pet will change its attack target.
	struct script_code
		*pet_script, ///< Script since pet hatched
		*pet_loyal_script; ///< Script when pet is loyal

	~s_pet_db()
	{
		if( this->pet_script ){
			script_free_code(this->pet_script);
		}

		if( this->pet_loyal_script ){
			script_free_code(this->pet_loyal_script);
		}
	}
};

enum e_pet_itemtype : uint8 { PET_CATCH,PET_EGG,PET_EQUIP,PET_FOOD };

enum e_pet_catch : uint16 {
	PET_CATCH_FAIL = 0, ///< A catch attempt failed
	PET_CATCH_UNIVERSAL = 1, ///< The catch attempt is universal (ignoring MD_STATUS_IMMUNE/Boss)
	PET_CATCH_UNIVERSAL_ITEM = 2,
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

struct s_pet_db *pet_db(uint16 pet_id);

struct pet_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data vd;
	struct s_pet pet;
	struct status_data status;
	struct mob_db *db;
	int pet_hungry_timer;
	int target_id;
	struct {
		unsigned skillbonus : 1;
	} state;
	int move_fail_count;
	unsigned int next_walktime,last_thinktime;
	unsigned short rate_fix;	//Support rate as modified by intimacy (1000 = 100%) [Skotlex]

	struct pet_recovery* recovery;
	struct pet_bonus* bonus;
	struct pet_skill_attack* a_skill;
	struct pet_skill_support* s_skill;
	struct pet_loot* loot;

	int masterteleport_timer;
	struct map_session_data *master;

	s_pet_db* get_pet_db() {
		return pet_db(this->pet.class_);
	}
};

bool pet_create_egg(struct map_session_data *sd, unsigned short item_id);
int pet_hungry_val(struct pet_data *pd);
void pet_set_intimate(struct pet_data *pd, int value);
int pet_target_check(struct pet_data *pd,struct block_list *bl,int type);
void pet_unlocktarget(struct pet_data *pd);
int pet_sc_check(struct map_session_data *sd, int type); //Skotlex
struct s_pet_db* pet_db_search(int key, enum e_pet_itemtype type);
int pet_hungry_timer_delete(struct pet_data *pd);
bool pet_data_init(struct map_session_data *sd, struct s_pet *pet);
int pet_birth_process(struct map_session_data *sd, struct s_pet *pet);
int pet_recv_petdata(uint32 account_id,struct s_pet *p,int flag);
int pet_select_egg(struct map_session_data *sd,short egg_index);
int pet_catch_process1(struct map_session_data *sd,int target_class);
int pet_catch_process2(struct map_session_data *sd,int target_id);
bool pet_get_egg(uint32 account_id, short pet_class, int pet_id);
int pet_menu(struct map_session_data *sd,int menunum);
int pet_change_name(struct map_session_data *sd,char *name);
int pet_change_name_ack(struct map_session_data *sd, char* name, int flag);
int pet_equipitem(struct map_session_data *sd,int index);
int pet_lootitem_drop(struct pet_data *pd,struct map_session_data *sd);
int pet_attackskill(struct pet_data *pd, int target_id);
int pet_skill_support_timer(int tid, unsigned int tick, int id, intptr_t data); // [Skotlex]
int pet_skill_bonus_timer(int tid, unsigned int tick, int id, intptr_t data); // [Valaris]
int pet_recovery_timer(int tid, unsigned int tick, int id, intptr_t data); // [Valaris]
int pet_heal_timer(int tid, unsigned int tick, int id, intptr_t data); // [Valaris]

#define pet_stop_walking(pd, type) unit_stop_walking(&(pd)->bl, type)
#define pet_stop_attack(pd) unit_stop_attack(&(pd)->bl)

void read_petdb(void);
void do_init_pet(void);
void do_final_pet(void);

#endif /* _PET_HPP_ */
