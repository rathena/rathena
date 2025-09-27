// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef BATTLE_HPP
#define BATTLE_HPP

#include <bitset>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <config/core.hpp>

#include "map.hpp" //ELE_MAX
#include "skill.hpp"

//fwd declaration
class map_session_data;
struct mob_data;
struct block_list;
enum e_damage_type : uint8;


/// State of a single attack attempt; used in flee/def penalty calculations when mobbed
enum damage_lv : uint8 {
	ATK_NONE,    /// Not an attack
	ATK_LUCKY,   /// Attack was lucky-dodged
	ATK_FLEE,    /// Attack was dodged
	ATK_MISS,    /// Attack missed because of element/race modifier.
	ATK_BLOCK,   /// Attack was blocked by some skills.
	ATK_DEF      /// Attack connected
};

/// Flag for base damage calculation
enum e_base_damage_flag : uint16 {
	BDMG_NONE	= 0x0000, /// None
	BDMG_CRIT	= 0x0001, /// Critical hit damage
	BDMG_ARROW  = 0x0002, /// Add arrow attack and use ranged damage formula
	BDMG_MAGIC  = 0x0004, /// Use MATK for base damage (e.g. Magic Crasher)
	BDMG_NOSIZE = 0x0008, /// Skip target size adjustment (e.g. Weapon Perfection)
};

/// Flag of the final calculation
enum e_battle_flag : uint16 {
	BF_NONE		= 0x0000, /// None
	BF_WEAPON	= 0x0001, /// Weapon attack
	BF_MAGIC	= 0x0002, /// Magic attack
	BF_MISC		= 0x0004, /// Misc attack

	BF_SHORT	= 0x0010, /// Short attack
	BF_LONG		= 0x0040, /// Long attack

	BF_SKILL	= 0x0100, /// Skill attack
	BF_NORMAL	= 0x0200, /// Normal attack

	BF_WEAPONMASK	= BF_WEAPON|BF_MAGIC|BF_MISC, /// Weapon attack mask
	BF_RANGEMASK	= BF_SHORT|BF_LONG, /// Range attack mask
	BF_SKILLMASK	= BF_SKILL|BF_NORMAL, /// Skill attack mask
};

/// Battle check target [Skotlex]
enum e_battle_check_target : uint32 {
	BCT_NOONE		= 0x000000, ///< No one
	BCT_SELF		= 0x010000, ///< Self
	BCT_ENEMY		= 0x020000, ///< Enemy
	BCT_PARTY		= 0x040000, ///< Party members
	BCT_GUILDALLY	= 0x080000, ///< Only allies, NOT guildmates
	BCT_NEUTRAL		= 0x100000, ///< Neutral target
	BCT_SAMEGUILD	= 0x200000, ///< Guildmates, No Guild Allies

	BCT_ALL			= 0x3F0000, ///< All targets

	BCT_WOS			= 0x400000, ///< Except self and your master
	BCT_SLAVE		= BCT_SELF|BCT_WOS,				///< Does not hit yourself/master, but hits your/master's slaves
	BCT_GUILD		= BCT_SAMEGUILD|BCT_GUILDALLY,	///< Guild AND Allies (BCT_SAMEGUILD|BCT_GUILDALLY)
	BCT_NOGUILD		= BCT_ALL&~BCT_GUILD,			///< Except guildmates
	BCT_NOPARTY		= BCT_ALL&~BCT_PARTY,			///< Except party members
	BCT_NOENEMY		= BCT_ALL&~BCT_ENEMY,			///< Except enemy
	BCT_ALLY		= BCT_PARTY|BCT_GUILD,
	BCT_FRIEND		= BCT_NOENEMY,
};

/// Check flag for common damage bonuses such as: ATKpercent, Refine, Passive Mastery, Spirit Spheres and Star Crumbs
enum e_bonus_chk_flag : uint8 {
	BCHK_ALL,    /// Check if all of the common damage bonuses apply to this skill
	BCHK_REFINE, /// Check if refine bonus is applied (pre-renewal only currently)
	BCHK_STAR,   /// Check if Star Crumb bonus is applied (pre-renewal only currently)
};

/// Damage structure
struct Damage {
#ifdef RENEWAL
	int64 statusAtk, statusAtk2, weaponAtk, weaponAtk2, equipAtk, equipAtk2, masteryAtk, masteryAtk2, percentAtk, percentAtk2;
#else
	int64 basedamage; /// Right hand damage that a normal attack would deal
#endif
	int64 damage, /// Right hand damage
		damage2; /// Left hand damage
	enum e_damage_type type; /// Check clif_damage for type
	int16 div_; /// Number of hit
	int32 amotion,
		dmotion;
	int32 blewcount; /// Number of knockback
	int32 flag; /// chk e_battle_flag
	int32 miscflag;
	enum damage_lv dmg_lv; /// ATK_LUCKY,ATK_FLEE,ATK_DEF
	bool isspdamage; /// Display blue damage numbers in clif_damage
};

// Damage Calculation

struct Damage battle_calc_attack(int32 attack_type,block_list *bl,block_list *target,uint16 skill_id,uint16 skill_lv,int32 flag);

int64 battle_calc_return_damage(block_list *bl, block_list *src, int64 *, int32 flag, uint16 skill_id, bool status_reflect);

void battle_drain(map_session_data *sd, block_list *tbl, int64 rdamage, int64 ldamage, int32 race, int32 class_);

int64 battle_attr_fix(block_list* src, block_list* target, int64 damage, int32 atk_elem, int32 def_type, int32 def_lv, int32 flag = 0);
int32 battle_calc_cardfix(int32 attack_type, block_list *src, block_list *target, std::bitset<NK_MAX> nk, int32 s_ele, int32 s_ele_, int64 damage, int32 left, int32 flag);

// Final calculation Damage
int64 battle_calc_damage(block_list *src,block_list *bl,struct Damage *d,int64 damage,uint16 skill_id,uint16 skill_lv);
int64 battle_calc_gvg_damage(block_list *src,block_list *bl,int64 damage,uint16 skill_id,int32 flag);
int64 battle_calc_bg_damage(block_list *src,block_list *bl,int64 damage,uint16 skill_id,int32 flag);
int64 battle_calc_pk_damage(block_list &src, block_list &bl, int64 damage, uint16 skill_id, int32 flag);

int32 battle_damage(block_list *src, block_list *target, int64 damage, int16 div_, uint16 skill_lv, uint16 skill_id, enum damage_lv dmg_lv, uint16 attack_type, bool additional_effects, t_tick tick, bool isspdamage, bool is_norm_attacked = false);
int32 battle_delay_damage (t_tick tick, int32 amotion, block_list *src, block_list *target, int32 attack_type, uint16 skill_id, uint16 skill_lv, int64 damage, enum damage_lv dmg_lv, int16 div_, bool additional_effects, bool isspdamage, bool is_norm_attacked = false);
int32 battle_fix_damage(block_list* src, block_list* target, int64 damage, int16 div_, uint16 skill_id);

int32 battle_calc_chorusbonus(map_session_data *sd);

// Summary normal attack treatment (basic attack)
enum damage_lv battle_weapon_attack( block_list *bl,block_list *target,t_tick tick,int32 flag);

// Accessors
block_list* battle_get_master(block_list *src);
block_list* battle_gettargeted(block_list *target);
block_list* battle_getenemy(block_list *target, int32 type, int32 range);
int32 battle_gettarget(block_list *bl);
uint16 battle_getcurrentskill(block_list *bl);

int32 battle_check_undead(int32 race,int32 element);
int32 battle_check_target(block_list *src, block_list *target,int32 flag);
bool battle_check_range(block_list *src,block_list *bl,int32 range);
bool battle_check_coma(map_session_data& sd, block_list& target, e_battle_flag attack_type);

void battle_consume_ammo(map_session_data* sd, int32 skill, int32 lv);

bool is_infinite_defense(block_list *target, int32 flag);

// Settings

#define MIN_HAIR_STYLE battle_config.min_hair_style
#define MAX_HAIR_STYLE battle_config.max_hair_style
#define MIN_HAIR_COLOR battle_config.min_hair_color
#define MAX_HAIR_COLOR battle_config.max_hair_color
#define MIN_CLOTH_COLOR battle_config.min_cloth_color
#define MAX_CLOTH_COLOR battle_config.max_cloth_color
#define MIN_BODY_STYLE battle_config.min_body_style
#define MAX_BODY_STYLE battle_config.max_body_style

struct Battle_Config
{
	int32 warp_point_debug;
	int32 enable_critical;
	int32 mob_critical_rate;
	int32 critical_rate;
	int32 enable_baseatk, enable_baseatk_renewal;
	int32 enable_perfect_flee;
	int32 cast_rate, delay_rate;
	int32 delay_dependon_dex, delay_dependon_agi;
	int32 sdelay_attack_enable;
	int32 left_cardfix_to_right;
	int32 cardfix_monster_physical;
	int32 skill_add_range;
	int32 skill_out_range_consume;
	int32 skill_amotion_leniency;
	int32 skillrange_by_distance; //[Skotlex]
	int32 use_weapon_skill_range; //[Skotlex]
	int32 pc_damage_delay_rate;
	int32 defnotenemy;
	int32 vs_traps_bctall;
	int32 traps_setting;
	int32 clear_unit_ondeath; //[Skotlex]
	int32 clear_unit_onwarp; //[Skotlex]
	int32 random_monster_checklv;
	int32 attr_recover;
	int32 item_auto_get;
	int32 flooritem_lifetime;
	int32 first_attack_loot_bonus;
	int32 item_first_get_time;
	int32 item_second_get_time;
	int32 item_third_get_time;
	int32 mvp_item_first_get_time;
	int32 mvp_item_second_get_time;
	int32 mvp_item_third_get_time;
	int32 base_exp_rate,job_exp_rate;
	int32 drop_rate0item;
	int32 death_penalty_type;
	int32 death_penalty_base,death_penalty_job;
	int32 pvp_exp;  // [MouseJstr]
	int32 gtb_sc_immunity;
	int32 zeny_penalty;
	int32 restart_hp_rate;
	int32 restart_sp_rate;
	int32 mvp_exp_rate;
	int32 mvp_hp_rate;
	int32 monster_hp_rate;
	int32 view_range_rate;
	int32 chase_range_rate;
	int32 atc_spawn_quantity_limit;
	int32 atc_slave_clone_limit;
	int32 partial_name_scan;
	int32 skillfree;
	int32 skillup_limit;
	int32 wp_rate;
	int32 pp_rate;
	int32 monster_active_enable;
	int32 monster_damage_delay_rate;
	int32 monster_loot_type;
	int32 mob_skill_rate;	//[Skotlex]
	int32 mob_skill_delay;	//[Skotlex]
	int32 mob_count_rate;
	int32 no_spawn_on_player; //[Skotlex]
	int32 force_random_spawn; //[Skotlex]
	int32 mob_spawn_delay, plant_spawn_delay, boss_spawn_delay;	// [Skotlex]
	int32 slaves_inherit_mode;
	int32 slaves_inherit_speed;
	int32 summons_trigger_autospells;
	int32 pc_walk_delay_rate; //Adjusts can't walk delay after being hit for players. [Skotlex]
	int32 walk_delay_rate; //Adjusts can't walk delay after being hit. [Skotlex]
	int32 multihit_delay;  //Adjusts can't walk delay per hit on multi-hitting skills. [Skotlex]
	int32 infinite_endure; // Always have endure
	int32 quest_skill_learn;
	int32 quest_skill_reset;
	int32 basic_skill_check;
	int32 guild_emperium_check;
	int32 guild_exp_limit;
	int32 guild_max_castles;
	int32 guild_skill_relog_delay;
	int32 guild_skill_relog_type;
	int32 emergency_call;
	int32 guild_aura;
	int32 pc_invincible_time;

	int32 pet_catch_rate;
	int32 pet_rename;
	int32 pet_friendly_rate;
	int32 pet_hungry_delay_rate;
	int32 pet_hungry_friendly_decrease;
	int32 pet_status_support;
	int32 pet_attack_support;
	int32 pet_damage_support;
	int32 pet_support_min_friendly;	//[Skotlex]
	int32 pet_support_rate;
	int32 pet_attack_exp_to_master;
	int32 pet_attack_exp_rate;
	int32 pet_lv_rate; //[Skotlex]
	int32 pet_max_stats; //[Skotlex]
	int32 pet_max_atk1; //[Skotlex]
	int32 pet_max_atk2; //[Skotlex]
	int32 pet_no_gvg; //Disables pets in gvg. [Skotlex]
	int32 pet_equip_required;
	int32 pet_unequip_destroy;
	int32 pet_master_dead;

	int32 skill_min_damage;
	int32 finger_offensive_type;
	int32 heal_exp;
	int32 max_heal_lv;
	int32 max_heal; //Mitternacht
	int32 resurrection_exp;
	int32 shop_exp;
	int32 combo_delay_rate;
	int32 item_check;
	int32 item_use_interval;	//[Skotlex]
	int32 cashfood_use_interval;
	int32 wedding_modifydisplay;
	int32 wedding_ignorepalette;	//[Skotlex]
	int32 xmas_ignorepalette;	// [Valaris]
	int32 summer_ignorepalette; // [Zephyrus]
	int32 hanbok_ignorepalette;
	int32 oktoberfest_ignorepalette;
	int32 natural_healhp_interval;
	int32 natural_healsp_interval;
	int32 natural_heal_skill_interval;
	int32 natural_heal_weight_rate;
	int32 arrow_decrement;
	int32 ammo_unequip;
	int32 ammo_check_weapon;
	int32 max_aspd;
	int32 max_walk_speed;	//Maximum walking speed after buffs [Skotlex]
	int32 max_hp_lv99;
	int32 max_hp_lv150;
	int32 max_hp;
	int32 max_sp;
	int32 max_lv, aura_lv;
	int32 max_parameter, max_baby_parameter;
	int32 max_cart_weight;
	int32 skill_log;
	int32 battle_log;
	int32 etc_log;
	int32 save_clothcolor;
	int32 undead_detect_type;
	int32 auto_counter_type;
	int32 min_hitrate;	//[Skotlex]
	int32 max_hitrate;	//[Skotlex]
	int32 agi_penalty_target;
	int32 agi_penalty_type;
	int32 agi_penalty_count;
	int32 agi_penalty_num;
	int32 vit_penalty_target;
	int32 vit_penalty_type;
	int32 vit_penalty_count;
	int32 vit_penalty_num;
	int32 weapon_defense_type;
	int32 magic_defense_type;
	int32 skill_reiteration;
	int32 skill_nofootset;
	int32 pc_cloak_check_type;
	int32 monster_cloak_check_type;
	int32 estimation_type;
	int32 gvg_short_damage_rate;
	int32 gvg_long_damage_rate;
	int32 gvg_weapon_damage_rate;
	int32 gvg_magic_damage_rate;
	int32 gvg_misc_damage_rate;
	int32 gvg_flee_penalty;
	int32 pk_short_damage_rate;
	int32 pk_long_damage_rate;
	int32 pk_weapon_damage_rate;
	int32 pk_magic_damage_rate;
	int32 pk_misc_damage_rate;
	int32 mob_changetarget_byskill;
	int32 attack_direction_change;
	int32 land_skill_limit;
	int32 monster_class_change_recover;
	int32 produce_item_name_input;
	int32 display_skill_fail;
	int32 chat_warpportal;
	int32 mob_warp;
	int32 dead_branch_active;
	int32 vending_max_value;
	int32 vending_over_max;
	int32 vending_tax;
	int32 vending_tax_min;
	int32 show_steal_in_same_party;
	int32 party_share_type;
	int32 party_hp_mode;
	int32 party_show_share_picker;
	int32 show_picker_item_type;
	int32 attack_attr_none;
	int32 item_rate_mvp, item_rate_common, item_rate_common_boss, item_rate_card, item_rate_card_boss,
		item_rate_equip, item_rate_equip_boss, item_rate_heal, item_rate_heal_boss, item_rate_use,
		item_rate_use_boss, item_rate_treasure, item_rate_adddrop, item_group_rate;
	int32 item_rate_common_mvp, item_rate_heal_mvp, item_rate_use_mvp, item_rate_equip_mvp, item_rate_card_mvp;

	int32 logarithmic_drops;
	int32 item_drop_common_min,item_drop_common_max;	// Added by TyrNemesis^
	int32 item_drop_card_min,item_drop_card_max;
	int32 item_drop_equip_min,item_drop_equip_max;
	int32 item_drop_mvp_min,item_drop_mvp_max;	// End Addition
	int32 item_drop_mvp_mode; //rAthena addition [Playtester]
	int32 item_drop_heal_min,item_drop_heal_max;	// Added by Valatris
	int32 item_drop_use_min,item_drop_use_max;	//End
	int32 item_drop_treasure_min,item_drop_treasure_max; //by [Skotlex]
	int32 item_drop_adddrop_min,item_drop_adddrop_max; //[Skotlex]
	int32 item_group_drop_min,item_group_drop_max;

	int32 prevent_logout;	// Added by RoVeRT
	int32 prevent_logout_trigger;
	int32 land_protector_behavior;
	int32 npc_emotion_behavior;

	int32 alchemist_summon_reward;	// [Valaris]
	int32 drops_by_luk;
	int32 drops_by_luk2;
	int32 equip_natural_break_rate;	//Base Natural break rate for attacks.
	int32 equip_self_break_rate; //Natural & Penalty skills break rate
	int32 equip_skill_break_rate; //Offensive skills break rate
	int32 multi_level_up;
	int32 multi_level_up_base;
	int32 multi_level_up_job;
	int32 max_exp_gain_rate; //Max amount of exp bar % you can get in one go.
	int32 pk_mode;
	int32 pk_mode_mes;
	int32 pk_level_range;

	int32 manner_system; // end additions [Valaris]
	int32 show_mob_info;

	int32 gx_allhit;
	int32 gx_disptype;
	int32 devotion_level_difference;
	int32 player_skill_partner_check;
	int32 invite_request_check;
	int32 skill_removetrap_type;
	int32 disp_experience;
	int32 disp_zeny;
	int32 backstab_bow_penalty;
	int32 hp_rate;
	int32 sp_rate;
	int32 bone_drop;
	int32 buyer_name;
	int32 dancing_weaponswitch_fix;

// eAthena additions
	int32 night_at_start; // added by [Yor]
	int32 day_duration; // added by [Yor]
	int32 night_duration; // added by [Yor]
	int32 ban_hack_trade; // added by [Yor]

	int32 min_hair_style; // added by [MouseJstr]
	int32 max_hair_style; // added by [MouseJstr]
	int32 min_hair_color; // added by [MouseJstr]
	int32 max_hair_color; // added by [MouseJstr]
	int32 min_cloth_color; // added by [MouseJstr]
	int32 max_cloth_color; // added by [MouseJstr]
	int32 pet_hair_style; // added by [Skotlex]

	int32 castrate_dex_scale; // added by [MouseJstr]
	int32 area_size; // added by [MouseJstr]

	int32 max_def, over_def_bonus; //added by [Skotlex]

	int32 zeny_from_mobs; // [Valaris]
	int32 mobs_level_up; // [Valaris]
	int32 mobs_level_up_exp_rate; // [Valaris]
	int32 pk_min_level; // [celest]
	int32 skill_steal_max_tries; //max steal skill tries on a mob. if 0, then w/o limit [Lupus]
	int32 skill_steal_random_options;
	int32 motd_type; // [celest]
	int32 exp_calc_type;
	int32 exp_bonus_attacker;
	int32 exp_bonus_max_attacker;
	int32 exp_bonus_nodamage_attacker;
	int32 min_skill_delay_limit;
	int32 amotion_min_skill_delay;
	int32 default_walk_delay;
	int32 no_skill_delay;
	int32 attack_walk_delay;
	int32 damage_walk_delay;
	int32 require_glory_guild;
	int32 idle_no_share;
	int32 party_update_interval;
	int32 party_even_share_bonus;
	int32 delay_battle_damage;
	int32 hide_woe_damage;
	int32 display_version;

	int32 display_hallucination;	// [Skotlex]
	int32 use_statpoint_table;	// [Skotlex]

	int32 debuff_on_logout; // Removes a few "official" negative Scs on logout. [Skotlex]
	int32 mob_ai; //Configures various mob_ai settings to make them smarter or dumber(official). [Skotlex]
	int32 hom_setting; //Configures various homunc settings which make them behave unlike normal characters.. [Skotlex]
	int32 dynamic_mobs; // Dynamic Mobs [Wizputer] - battle_athena flag implemented by [random]
	int32 mob_remove_damaged; // Dynamic Mobs - Remove mobs even if damaged [Wizputer]
	int32 mob_remove_delay; // Dynamic Mobs - delay before removing mobs from a map [Skotlex]
	int32 mob_active_time; //Duration through which mobs execute their Hard AI after players leave their area of sight.
	int32 boss_active_time;

	int32 show_hp_sp_drain, show_hp_sp_gain;	//[Skotlex]

	int32 mob_npc_event_type; //Determines on who the npc_event is executed. [Skotlex]

	int32 character_size; // if riders have size=2, and baby class riders size=1 [Lupus]
	int32 mob_max_skilllvl; // Max possible skill level [Lupus]
	int32 rare_drop_announce; // chance <= to show rare drops global announces
	int32 drop_rate_cap;  // Drop rate can't be raised above this amount by drop bonus items
	int32 drop_rate_cap_vip;

	int32 retaliate_to_master;	//Whether when a mob is attacked by another mob, it will retaliate versus the mob or the mob's master. [Skotlex]

	int32 duel_allow_pvp; // [LuzZza]
	int32 duel_allow_gvg; // [LuzZza]
	int32 duel_allow_teleport; // [LuzZza]
	int32 duel_autoleave_when_die; // [LuzZza]
	int32 duel_time_interval; // [LuzZza]
	int32 duel_only_on_same_map; // [Toms]

	int32 skip_teleport_lv1_menu; // possibility to disable (skip) Teleport Lv1 menu, that have only two lines `Random` and `Cancel` [LuzZza]

	int32 allow_skill_without_day; // [Komurka]
	int32 allow_es_magic_pc; // [Skotlex]
	int32 skill_wall_check; // [Skotlex]
	int32 official_cell_stack_limit; // [Playtester]
	int32 custom_cell_stack_limit; // [Skotlex]
	int32 skill_caster_check; // [Skotlex]
	int32 sc_castcancel; // [Skotlex]
	int32 pc_sc_def_rate; // [Skotlex]
	int32 mob_sc_def_rate;
	int32 pc_max_sc_def;
	int32 mob_max_sc_def;

	int32 sg_angel_skill_ratio;
	int32 sg_miracle_skill_ratio;
	int32 sg_miracle_skill_duration;
	int32 autospell_stacking; //Enables autospell cards to stack. [Skotlex]
	int32 override_mob_names; //Enables overriding spawn mob names with the mob_db names. [Skotlex]
	int32 min_chat_delay; //Minimum time between client messages. [Skotlex]
	int32 friend_auto_add; //When accepting friends, both get friended. [Skotlex]
	int32 hvan_explosion_intimate;	// fix [albator]
	int32 hom_rename;
	int32 homunculus_show_growth ;	//[orn]
	int32 homunculus_friendly_rate;
	int32 quest_exp_rate;
	int32 autotrade_mapflag;
	int32 at_timeout;
	int32 homunculus_autoloot;
	int32 idle_no_autoloot;
	int32 max_guild_alliance;
	int32 ksprotection;
	int32 auction_feeperhour;
	int32 auction_maximumprice;
	int32 homunculus_auto_vapor;	//Keep Homunculus from Vaporizing when master dies. [L0ne_W0lf]
	int32 display_status_timers;	//Show or hide skill buff/delay timers in recent clients [Sara]
	int32 skill_add_heal_rate;	//skills that bHealPower has effect on [Inkfish]
	int32 eq_single_target_reflectable;
	int32 invincible_nodamage;
	int32 mob_slave_keep_target;
	int32 autospell_check_range;	//Enable range check for autospell bonus. [L0ne_W0lf]
	int32 knockback_left;
	int32 client_reshuffle_dice;  // Reshuffle /dice
	int32 client_sort_storage;
	int32 feature_buying_store;
	int32 feature_search_stores;
	int32 searchstore_querydelay;
	int32 searchstore_maxresults;
	int32 display_party_name;
	int32 cashshop_show_points;
	int32 mail_show_status;
	int32 client_limit_unit_lv;
	int32 hom_max_level;
	int32 hom_S_max_level;
	int32 hom_S_growth_level;

	// [BattleGround Settings]
	int32 bg_update_interval;
	int32 bg_short_damage_rate;
	int32 bg_long_damage_rate;
	int32 bg_weapon_damage_rate;
	int32 bg_magic_damage_rate;
	int32 bg_misc_damage_rate;
	int32 bg_flee_penalty;

	// rAthena
	int32 max_third_parameter;
	int32 max_baby_third_parameter;
	int32 max_trans_parameter;
	int32 max_third_trans_parameter;
	int32 max_extended_parameter;
	int32 max_summoner_parameter;
	int32 max_fourth_parameter;
	int32 max_third_aspd;
	int32 max_summoner_aspd;
	int32 vcast_stat_scale;

	int32 mvp_tomb_enabled;
	int32 mvp_tomb_delay;

	int32 atcommand_suggestions_enabled;
	int32 min_npc_vendchat_distance;
	int32 atcommand_mobinfo_type;

	int32 mob_size_influence; // Enable modifications on earned experience, drop rates and monster status depending on monster size. [mkbu95]
	int32 skill_trap_type;
	int32 multi_trigger_trap;
	int32 allow_consume_restricted_item;
	int32 allow_equip_restricted_item;
	int32 max_walk_path;
	int32 item_enabled_npc;
	int32 item_onfloor; // Whether to drop an undroppable item on the map or destroy it if inventory is full.
	int32 bowling_bash_area;
	int32 drop_rateincrease;
	int32 feature_auction;
	int32 feature_banking;
	int32 vip_storage_increase;
	int32 vip_base_exp_increase;
	int32 vip_job_exp_increase;
	int32 vip_zeny_penalty;
	int32 vip_bm_increase;
	int32 vip_drop_increase;
	int32 vip_gemstone;
	int32 vip_exp_penalty_base;
	int32 vip_exp_penalty_job;
	int32 vip_disp_rate;
	int32 mon_trans_disable_in_gvg;
	int32 discount_item_point_shop;
	int32 update_enemy_position;
	int32 devotion_rdamage;
	int32 feature_itemlink;
	int32 feature_mesitemlink;
	int32 feature_mesitemlink_brackets;
	int32 feature_mesitemlink_dbname;
	int32 feature_mesitemicon;
	int32 feature_mesitemicon_dbname;

	// autotrade persistency
	int32 feature_autotrade;
	int32 feature_autotrade_direction;
	int32 feature_autotrade_head_direction;
	int32 feature_autotrade_sit;
	int32 feature_autotrade_open_delay;

	// Fame points
	int32 fame_taekwon_mission;
	int32 fame_refine_lv1;
	int32 fame_refine_lv2;
	int32 fame_refine_lv3;
	int32 fame_forge;
	int32 fame_pharmacy_3;
	int32 fame_pharmacy_5;
	int32 fame_pharmacy_7;
	int32 fame_pharmacy_10;

	int32 disp_servervip_msg;
	int32 warg_can_falcon;
	int32 path_blown_halt;
	int32 rental_mount_speed_boost;
	int32 warp_suggestions_enabled;
	int32 taekwon_mission_mobname;
	int32 teleport_on_portal;
	int32 cart_revo_knockback;
	int32 guild_notice_changemap;
	int32 transcendent_status_points;
	int32 taekwon_ranker_min_lv;
	int32 revive_onwarp;
	int32 mail_delay;
	int32 autotrade_monsterignore;
	int32 idletime_option;
	int32 spawn_direction;
	int32 arrow_shower_knockback;
	int32 devotion_rdamage_skill_only;
	int32 max_extended_aspd;
	int32 mob_chase_refresh; //How often a monster should refresh its chase [Playtester]
	int32 mob_icewall_walk_block; //How a normal monster should be trapped in icewall [Playtester]
	int32 boss_icewall_walk_block; //How a boss monster should be trapped in icewall [Playtester]
	int32 snap_dodge; // Enable or disable dodging damage snapping away [csnv]
	int32 stormgust_knockback;
	int32 default_fixed_castrate;
	int32 default_bind_on_equip;
	int32 pet_ignore_infinite_def; // Makes fixed damage of petskillattack2 ignores infinite defense
	int32 homunculus_evo_intimacy_need;
	int32 homunculus_evo_intimacy_reset;
	int32 monster_loot_search_type;
	int32 feature_roulette;
	int32 feature_roulette_bonus_reward;
	int32 monster_hp_bars_info;
	int32 min_body_style;
	int32 max_body_style;
	int32 save_body_style;
	int32 mob_eye_range_bonus; //Vulture's Eye and Snake's Eye range bonus
	int32 mob_stuck_warning; //Show warning if a monster is stuck too long
	int32 skill_eightpath_algorithm; //Official path algorithm
	int32 skill_eightpath_same_cell;
	int32 death_penalty_maxlv;
	int32 exp_cost_redemptio;
	int32 exp_cost_redemptio_limit;
	int32 mvp_exp_reward_message;
	int32 can_damage_skill; //Which BL types can damage traps
	int32 atcommand_levelup_events;
	int32 atcommand_disable_npc;
	int32 block_account_in_same_party;
	int32 tarotcard_equal_chance; //Official or equal chance for each card
	int32 change_party_leader_samemap;
	int32 dispel_song; //Can songs be dispelled?
	int32 refresh_song; // Can song durations be refreshed?
	int32 refresh_song_icon; // Should the song icon duration be refreshed?
	int32 guild_maprespawn_clones; // Should clones be killed by maprespawnguildid?
	int32 hide_fav_sell;
	int32 mail_daily_count;
	int32 mail_zeny_fee;
	int32 mail_attachment_price;
	int32 mail_attachment_weight;
	int32 banana_bomb_duration;
	int32 guild_leaderchange_delay;
	int32 guild_leaderchange_woe;
	int32 guild_alliance_onlygm;
	int32 feature_achievement;
	int32 allow_bound_sell;
	int32 autoloot_adjust;
	int32 feature_petevolution;
	int32 feature_pet_autofeed;
	int32 feature_pet_autofeed_rate;
	int32 pet_autofeed_always;
	int32 broadcast_hide_name;
	int32 skill_drop_items_full;
	int32 switch_remove_edp;
	int32 feature_homunculus_autofeed;
	int32 feature_homunculus_autofeed_rate;
	int32 summoner_race;
	int32 summoner_size;
	int32 homunculus_autofeed_always;
	int32 feature_attendance;
	int32 feature_privateairship;
	int32 rental_transaction;
	int32 min_shop_buy;
	int32 min_shop_sell;
	int32 feature_equipswitch;
	int32 pet_walk_speed;
	int32 blacksmith_fame_refine_threshold;
	int32 mob_nopc_idleskill_rate;
	int32 mob_nopc_move_rate;
	int32 boss_nopc_idleskill_rate;
	int32 boss_nopc_move_rate;
	int32 hom_idle_no_share;
	int32 idletime_hom_option;
	int32 devotion_standup_fix;
	int32 feature_bgqueue;
	int32 bgqueue_nowarp_mapflag;
	int32 homunculus_exp_gain;
	int32 rental_item_novalue;
	int32 ping_timer_interval;
	int32 ping_time;
	int32 show_skill_scale;
	int32 achievement_mob_share;
	int32 slave_stick_with_master;
	int32 slave_active_with_master;
	int32 at_logout_event;
	int32 homunculus_starving_rate;
	int32 homunculus_starving_delay;
	int32 drop_connection_on_quit;
	int32 mob_spawn_variance;
	int32 mercenary_autoloot;
	int32 mer_idle_no_share;
	int32 idletime_mer_option;
	int32 feature_refineui;
	int32 rndopt_drop_pillar;
	int32 pet_legacy_formula;
	int32 pet_distance_check;
	int32 pet_hide_check;

	int32 instance_block_leave;
	int32 instance_block_leaderchange;
	int32 instance_block_invite;
	int32 instance_block_expulsion;
	// 4th Jobs Stuff
	int32 trait_points_job_change;
	int32 use_traitpoint_table;
	int32 max_trait_parameter;
	int32 max_res_mres_ignored;
	int32 max_ap;
	int32 ap_rate;
	int32 restart_ap_rate;
	int32 loose_ap_on_death;
	int32 loose_ap_on_map;
	int32 keep_ap_on_logout;
	int32 attack_machine_level_difference;

	int32 feature_barter;
	int32 feature_barter_extended;
	int32 break_mob_equip;
	int32 macro_detection_retry;
	int32 macro_detection_timeout;
	int32 macro_detection_punishment;
	int32 macro_detection_punishment_time;
	int32 macrochecker_delay;

	int32 feature_dynamicnpc_timeout;
	int32 feature_dynamicnpc_rangex;
	int32 feature_dynamicnpc_rangey;
	int32 feature_dynamicnpc_direction;

	int32 mob_respawn_time;
	int32 mob_unlock_time;
	int32 map_edge_size;
	int32 randomize_center_cell;

	int32 feature_stylist;
	int32 feature_banking_state_enforce;
	int32 instance_allow_reconnect;
	int32 synchronize_damage;
	int32 item_stacking;
	int32 hom_delay_reset_vaporize;
	int32 hom_delay_reset_warp;
	int32 alchemist_summon_setting;
	int32 loot_range;
	int32 assist_range;
	int32 open_box_weight_rate;
	int32 major_overweight_rate;
	int32 trade_count_stackable;
	int32 enable_bonus_map_drops;

#include <custom/battle_config_struct.inc>
};

extern struct Battle_Config battle_config;

void do_init_battle(void);
void do_final_battle(void);
extern int32 battle_config_read(const char *cfgName);
extern void battle_set_defaults(void);
int32 battle_set_value(const char* w1, const char* w2);
int32 battle_get_value(const char* w1);

//
block_list* battle_getenemyarea(block_list *src, int32 x, int32 y, int32 range, int32 type, int32 ignore_id);
/**
 * Royal Guard
 **/
int32 battle_damage_area( block_list *bl, va_list ap);

#endif /* BATTLE_HPP */
