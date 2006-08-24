// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _BATTLE_H_
#define _BATTLE_H_

// ダメージ
struct Damage {
	int damage,damage2;
	int type,div_;
	int amotion,dmotion;
	int blewcount;
	int flag;
	int dmg_lv;	//ATK_LUCKY,ATK_FLEE,ATK_DEF
};

// 属性表（読み込みはpc.c、battle_attr_fixで使用）
extern int attr_fix_table[4][10][10];

struct map_session_data;
struct mob_data;
struct block_list;

// ダメージ計算

struct Damage battle_calc_attack(int attack_type,struct block_list *bl,struct block_list *target,int skill_num,int skill_lv,int flag);

int battle_calc_return_damage(struct block_list *bl, int *damage, int flag);

void battle_drain(struct map_session_data *sd, struct block_list *tbl, int rdamage, int ldamage, int race, int boss);

int battle_attr_fix(struct block_list *src, struct block_list *target, int damage,int atk_elem,int def_type, int def_lv);

// ダメージ最終計算
int battle_calc_damage(struct block_list *src,struct block_list *bl,int damage,int div_,int skill_num,int skill_lv,int flag);
int battle_calc_gvg_damage(struct block_list *src,struct block_list *bl,int damage,int div_,int skill_num,int skill_lv,int flag);

enum {	// 最終計算のフラグ
	BF_WEAPON	= 0x0001,
	BF_MAGIC	= 0x0002,
	BF_MISC		= 0x0004,
	BF_SHORT	= 0x0010,
	BF_LONG		= 0x0040,
	BF_SKILL	= 0x0100,
	BF_NORMAL	= 0x0200,
	BF_WEAPONMASK=0x000f,
	BF_RANGEMASK= 0x00f0,
	BF_SKILLMASK= 0x0f00,
};

int battle_delay_damage (unsigned int tick, struct block_list *src, struct block_list *target, int attack_type, int skill_id, int skill_lv, int damage, int dmg_lv, int ddelay);

// 通常攻撃処理まとめ
int battle_weapon_attack( struct block_list *bl,struct block_list *target,
	 unsigned int tick,int flag);

// 各種パラメータを得る
struct block_list* battle_get_master(struct block_list *src);
struct block_list* battle_gettargeted(struct block_list *target);
int battle_gettarget(struct block_list *bl);
int battle_getcurrentskill(struct block_list *bl);

//New definitions [Skotlex]
#define BCT_ENEMY 0x020000
//This should be (~BCT_ENEMY&BCT_ALL)
#define BCT_NOENEMY 0x1d0000
#define BCT_PARTY	0x040000
//This should be (~BCT_PARTY&BCT_ALL)
#define BCT_NOPARTY 0x1b0000
#define BCT_GUILD	0x080000
//This should be (~BCT_GUILD&BCT_ALL)
#define BCT_NOGUILD 0x170000
#define BCT_ALL 0x1f0000
#define BCT_NOONE 0x000000
#define BCT_SELF 0x010000
#define BCT_NEUTRAL 0x100000

#define	is_boss(bl)	status_get_mexp(bl)	// Can refine later [Aru]

int battle_check_undead(int race,int element);
int battle_check_target(struct block_list *src, struct block_list *target,int flag);
int battle_check_range(struct block_list *src,struct block_list *bl,int range);

void battle_consume_ammo(struct map_session_data* sd, int skill, int lv);
// 設定

int battle_config_switch(const char *str); // [Valaris]

extern struct Battle_Config {
	unsigned short warp_point_debug;
	unsigned short enable_critical;
	unsigned short mob_critical_rate;
	unsigned short critical_rate;
	unsigned short enable_baseatk;
	unsigned short enable_perfect_flee;
	unsigned short cast_rate,delay_rate,delay_dependon_dex;
	unsigned short sdelay_attack_enable;
	unsigned short left_cardfix_to_right;
	unsigned short skill_add_range;
	unsigned short skill_out_range_consume;
	unsigned short skillrange_by_distance; //[Skotlex]
	unsigned short use_weapon_skill_range; //[Skotlex]
	unsigned short pc_damage_delay_rate;
	unsigned short defnotenemy;
	unsigned short vs_traps_bctall;	
	unsigned short traps_setting;	
	unsigned short clear_unit_ondeath; //[Skotlex]
	unsigned short clear_unit_onwarp; //[Skotlex]
	unsigned short random_monster_checklv;
	unsigned short attr_recover;
	unsigned short flooritem_lifetime;
	unsigned short item_auto_get;
	int item_first_get_time;
	int item_second_get_time;
	int item_third_get_time;
	int mvp_item_first_get_time;
	int mvp_item_second_get_time;
	int mvp_item_third_get_time;
	int base_exp_rate,job_exp_rate;
	unsigned short drop_rate0item;
	unsigned short death_penalty_type;
	unsigned short death_penalty_base,death_penalty_job;
	unsigned short pvp_exp;  // [MouseJstr]
	unsigned short gtb_sc_immunity;
	int zeny_penalty;
	unsigned short restart_hp_rate;
	unsigned short restart_sp_rate;
	int mvp_exp_rate;
	unsigned short mvp_hp_rate;
	unsigned short monster_hp_rate;
	unsigned short monster_max_aspd;
	unsigned short view_range_rate;
	unsigned short chase_range_rate;
	unsigned short atc_gmonly;
	unsigned short atc_spawn_quantity_limit;
	unsigned short atc_slave_clone_limit;
	unsigned short gm_allskill;
	unsigned short gm_allskill_addabra;
	unsigned short gm_allequip;
	unsigned short gm_skilluncond;
	unsigned short gm_join_chat;
	unsigned short gm_kick_chat;
	unsigned short skillfree;
	unsigned short skillup_limit;
	unsigned short wp_rate;
	unsigned short pp_rate;
	unsigned short monster_active_enable;
	unsigned short monster_damage_delay_rate;
	unsigned short monster_loot_type;
	unsigned short mob_skill_rate;	//[Skotlex]
	unsigned short mob_skill_delay;	//[Skotlex]
	unsigned short mob_count_rate;
	unsigned short no_spawn_on_player; //[Skotlex]
	unsigned short force_random_spawn; //[Skotlex]
	unsigned short mob_spawn_delay, plant_spawn_delay, boss_spawn_delay;	// [Skotlex]
	unsigned short slaves_inherit_mode;
	unsigned short slaves_inherit_speed;
	unsigned short summons_trigger_autospells;
	unsigned short pc_walk_delay_rate; //Adjusts can't walk delay after being hit for players. [Skotlex]
	unsigned short walk_delay_rate; //Adjusts can't walk delay after being hit. [Skotlex]
	unsigned short multihit_delay;  //Adjusts can't walk delay per hit on multi-hitting skills. [Skotlex]
	unsigned short quest_skill_learn;
	unsigned short quest_skill_reset;
	unsigned short basic_skill_check;
	unsigned short guild_emperium_check;
	unsigned short guild_exp_limit;
	unsigned short guild_max_castles;
	unsigned short emergency_call;
	unsigned short guild_aura;
	unsigned short pc_invincible_time;
	unsigned short pet_catch_rate;
	unsigned short pet_rename;
	unsigned short pet_friendly_rate;
	unsigned short pet_hungry_delay_rate;
	unsigned short pet_hungry_friendly_decrease;
	unsigned short pet_status_support;
	unsigned short pet_attack_support;
	unsigned short pet_damage_support;
	unsigned short pet_support_min_friendly;	//[Skotlex]
	unsigned short pet_support_rate;
	unsigned short pet_attack_exp_to_master;
	unsigned short pet_attack_exp_rate;
	unsigned short pet_lv_rate; //[Skotlex]
	unsigned short pet_max_stats; //[Skotlex]
	unsigned short pet_max_atk1; //[Skotlex]
	unsigned short pet_max_atk2; //[Skotlex]
	unsigned short pet_no_gvg; //Disables pets in gvg. [Skotlex]
	unsigned short skill_min_damage;
	unsigned short finger_offensive_type;
	unsigned short heal_exp;
	unsigned short max_heal_lv;
	int max_heal; //Mitternacht
	unsigned short resurrection_exp;
	unsigned short shop_exp;
	unsigned short combo_delay_rate;
	unsigned short item_check;
	unsigned short item_use_interval;	//[Skotlex]
	unsigned short wedding_modifydisplay;
	unsigned short wedding_ignorepalette;	//[Skotlex]
	unsigned short xmas_ignorepalette;	// [Valaris]
	int natural_healhp_interval;
	int natural_healsp_interval;
	int natural_heal_skill_interval;
	unsigned short natural_heal_weight_rate;
	unsigned short item_name_override_grffile;
	unsigned short indoors_override_grffile;	// [Celest]
	unsigned short skill_sp_override_grffile;	// [Celest]
	unsigned short cardillust_read_grffile;
	unsigned short item_equip_override_grffile;
	unsigned short item_slots_override_grffile;
	unsigned short arrow_decrement;
	unsigned short max_aspd;
	unsigned short max_walk_speed;	//Maximum walking speed after buffs [Skotlex]
	int max_hp;
	int max_sp;
	unsigned short max_lv, aura_lv;
	unsigned short max_parameter, max_baby_parameter;
	int max_cart_weight;
	unsigned short skill_log;
	unsigned short battle_log;
	unsigned short save_log;
	unsigned short error_log;
	unsigned short etc_log;
	unsigned short save_clothcolor;
	unsigned short undead_detect_type;
	unsigned short auto_counter_type;
	unsigned short min_hitrate;	//[Skotlex]
	unsigned short max_hitrate;	//[Skotlex]
	unsigned short agi_penalty_target;
	unsigned short agi_penalty_type;
	unsigned short agi_penalty_count;
	unsigned short agi_penalty_num;
	unsigned short vit_penalty_target;
	unsigned short vit_penalty_type;
	unsigned short vit_penalty_count;
	unsigned short vit_penalty_num;
	unsigned short weapon_defense_type;
	unsigned short magic_defense_type;
	unsigned short skill_reiteration;
	unsigned short skill_nofootset;
	unsigned short pc_cloak_check_type;
	unsigned short monster_cloak_check_type;
	unsigned short estimation_type;
	unsigned short gvg_short_damage_rate;
	unsigned short gvg_long_damage_rate;
	unsigned short gvg_weapon_damage_rate;
	unsigned short gvg_magic_damage_rate;
	unsigned short gvg_misc_damage_rate;
	unsigned short gvg_flee_penalty;
	int gvg_eliminate_time;
	unsigned short pk_short_damage_rate;
	unsigned short pk_long_damage_rate;
	unsigned short pk_weapon_damage_rate;
	unsigned short pk_magic_damage_rate;
	unsigned short pk_misc_damage_rate;
	unsigned short mob_changetarget_byskill;
	unsigned short attack_direction_change;
	unsigned short land_skill_limit;
	unsigned short party_skill_penalty;
	unsigned short monster_class_change_full_recover;
	unsigned short produce_item_name_input;
	unsigned short produce_potion_name_input;
	unsigned short making_arrow_name_input;
	unsigned short holywater_name_input;
	unsigned short cdp_name_input;
	unsigned short display_skill_fail;
	unsigned short chat_warpportal;
	unsigned short mob_warp;
	unsigned short dead_branch_active;
	unsigned int vending_max_value;
	unsigned short show_steal_in_same_party;
	unsigned short party_share_type;
	unsigned short party_hp_mode;
	unsigned short party_show_share_picker;
	unsigned short attack_attr_none;
	int item_rate_mvp, item_rate_common, item_rate_common_boss, item_rate_card, item_rate_card_boss,	// added support for MVP drops [Reddozen]
		item_rate_equip, item_rate_equip_boss, item_rate_heal, item_rate_heal_boss, item_rate_use,
		item_rate_use_boss, item_rate_treasure,	// Added by RoVeRT, Additional Heal and Usable item rate by Val
		item_rate_adddrop;
	
	unsigned short logarithmic_drops;
	unsigned short item_drop_common_min,item_drop_common_max;	// Added by TyrNemesis^
	unsigned short item_drop_card_min,item_drop_card_max;
	unsigned short item_drop_equip_min,item_drop_equip_max;
	unsigned short item_drop_mvp_min,item_drop_mvp_max;	// End Addition
	unsigned short item_drop_heal_min,item_drop_heal_max;	// Added by Valatris
	unsigned short item_drop_use_min,item_drop_use_max;	//End
	unsigned short item_drop_treasure_min,item_drop_treasure_max; //by [Skotlex]
	unsigned short item_drop_adddrop_min,item_drop_adddrop_max; //[Skotlex]
	unsigned short prevent_logout;	// Added by RoVeRT

	unsigned short alchemist_summon_reward;	// [Valaris]
	unsigned short drops_by_luk;
	unsigned short drops_by_luk2;
	unsigned short equip_natural_break_rate;	//Base Natural break rate for attacks.
	unsigned short equip_self_break_rate; //Natural & Penalty skills break rate
	unsigned short equip_skill_break_rate; //Offensive skills break rate
	unsigned short pet_equip_required;
	unsigned short multi_level_up;
	unsigned short max_exp_gain_rate; //Max amount of exp bar % you can get in one go.
	unsigned short pk_mode;
	unsigned short pk_level_range;

	unsigned short manner_system;
	unsigned short show_mob_hp;  // end additions [Valaris]

	unsigned short agi_penalty_count_lv;
	unsigned short vit_penalty_count_lv;

	unsigned short gx_allhit;
	unsigned short gx_disptype;
	unsigned short devotion_level_difference;
	unsigned short player_skill_partner_check;
	unsigned short hide_GM_session;
	unsigned short invite_request_check;
	unsigned short skill_removetrap_type;
	unsigned short disp_experience;
	unsigned short disp_zeny;
	unsigned short castle_defense_rate;
	unsigned short backstab_bow_penalty;
	unsigned short hp_rate;
	unsigned short sp_rate;
	unsigned short gm_cant_drop_min_lv;
	unsigned short gm_cant_drop_max_lv;
	unsigned short disp_hpmeter;
	unsigned short bone_drop;
	unsigned short buyer_name;

// eAthena additions
	unsigned short night_at_start; // added by [Yor]
	int day_duration; // added by [Yor]
	int night_duration; // added by [Yor]
	unsigned short ban_spoof_namer; // added by [Yor]
	short ban_hack_trade; // added by [Yor]
	unsigned short hack_info_GM_level; // added by [Yor]
	unsigned short any_warp_GM_min_level; // added by [Yor]
	unsigned short packet_ver_flag; // added by [Yor]
	
	unsigned short min_hair_style; // added by [MouseJstr]
	unsigned short max_hair_style; // added by [MouseJstr]
	unsigned short min_hair_color; // added by [MouseJstr]
	unsigned short max_hair_color; // added by [MouseJstr]
	unsigned short min_cloth_color; // added by [MouseJstr]
	unsigned short max_cloth_color; // added by [MouseJstr]
	unsigned short pet_hair_style; // added by [Skotlex]

	unsigned short castrate_dex_scale; // added by [MouseJstr]
	unsigned short area_size; // added by [MouseJstr]

	unsigned short max_def, over_def_bonus; //added by [Skotlex]
	
	unsigned short zeny_from_mobs; // [Valaris]
	unsigned short mobs_level_up; // [Valaris]
	unsigned short mobs_level_up_exp_rate; // [Valaris]
	unsigned short pk_min_level; // [celest]
	unsigned short skill_steal_type; // [celest]
	unsigned short skill_steal_rate; // [celest]
	unsigned short skill_steal_max_tries; //max steal skill tries on a mob. if=256, then w/o limit [Lupus]
//	unsigned short night_darkness_level; // [celest]
	unsigned short motd_type; // [celest]
	unsigned short allow_atcommand_when_mute; // [celest]
	unsigned short finding_ore_rate; // orn
	unsigned short exp_calc_type;
	unsigned short min_skill_delay_limit;
	unsigned short default_skill_delay;
	unsigned short no_skill_delay;
	unsigned short require_glory_guild;
	unsigned short idle_no_share;
	unsigned short party_update_interval;
	unsigned short party_even_share_bonus;
	unsigned short delay_battle_damage;
	unsigned short hide_woe_damage;
	unsigned short display_version;
	unsigned short who_display_aid;

	unsigned short display_hallucination;	// [Skotlex]
	unsigned short use_statpoint_table;	// [Skotlex]

	unsigned short ignore_items_gender; //[Lupus]

	unsigned short copyskill_restrict; // [Aru]
	unsigned short berserk_cancels_buffs; // [Aru]
	unsigned short debuff_on_logout; // Removes a few "official" negative Scs on logout. [Skotlex]
	unsigned short mob_ai; //Configures various mob_ai settings to make them smarter or dumber(official). [Skotlex]
	unsigned short dynamic_mobs; // Dynamic Mobs [Wizputer] - battle_athena flag implemented by [random]
	unsigned short mob_remove_damaged; // Dynamic Mobs - Remove mobs even if damaged [Wizputer]
	int mob_remove_delay; // Dynamic Mobs - delay before removing mobs from a map [Skotlex]

	unsigned short show_hp_sp_drain, show_hp_sp_gain;	//[Skotlex]

	unsigned short mob_npc_event_type; //Determines on who the npc_event is executed. [Skotlex]
	unsigned short mob_clear_delay; // [Valaris]

	unsigned short character_size; // if riders have size=2, and baby class riders size=1 [Lupus]
	unsigned short mob_max_skilllvl; // Max possible skill level [Lupus]
	unsigned short rare_drop_announce; // chance <= to show rare drops global announces

	unsigned short retaliate_to_master;	//Whether when a mob is attacked by another mob, it will retaliate versus the mob or the mob's master. [Skotlex]
	unsigned short firewall_hits_on_undead; //Number of hits firewall does at a time on undead. [Skotlex]

	unsigned short title_lvl1; // Players titles [Lupus]
	unsigned short title_lvl2; // Players titles [Lupus]
	unsigned short title_lvl3; // Players titles [Lupus]
	unsigned short title_lvl4; // Players titles [Lupus]
	unsigned short title_lvl5; // Players titles [Lupus]
	unsigned short title_lvl6; // Players titles [Lupus]
	unsigned short title_lvl7; // Players titles [Lupus]
	unsigned short title_lvl8; // Players titles [Lupus]
	
	unsigned short duel_enable; // [LuzZza]
	unsigned short duel_allow_pvp; // [LuzZza]
	unsigned short duel_allow_gvg; // [LuzZza]
	unsigned short duel_allow_teleport; // [LuzZza]
	unsigned short duel_autoleave_when_die; // [LuzZza]
	unsigned short duel_time_interval; // [LuzZza]
	unsigned short duel_only_on_same_map; // [Toms]
	
	unsigned short skip_teleport_lv1_menu; // possibility to disable (skip) Teleport Lv1 menu, that have only two lines `Random` and `Cancel` [LuzZza]

	unsigned short allow_skill_without_day; // [Komurka]
	unsigned short allow_es_magic_pc; // [Skotlex]
	unsigned short skill_wall_check; // [Skotlex]
	unsigned short cell_stack_limit; // [Skotlex]
	unsigned short skill_caster_check; // [Skotlex]
	unsigned short sc_castcancel; // [Skotlex]
	unsigned short pc_sc_def_rate; // [Skotlex]
	unsigned short mob_sc_def_rate;
	unsigned short pc_luk_sc_def;
	unsigned short mob_luk_sc_def;
	unsigned short pc_max_sc_def;
	unsigned short mob_max_sc_def;

	unsigned short sg_miracle_skill_ratio;
	int sg_miracle_skill_duration;
	unsigned short autospell_stacking; //Enables autospell cards to stack. [Skotlex]
	unsigned short override_mob_names; //Enables overriding spawn mob names with the mob_db names. [Skotlex]
	unsigned short min_chat_delay; //Minimum time between client messages. [Skotlex]
	unsigned int hvan_explosion_intimate ;	// fix [albator]
	unsigned short homunculus_show_growth ;	//[orn]
	unsigned short homunculus_friendly_rate;
} battle_config;

void do_init_battle(void);
void do_final_battle(void);
extern int battle_config_read(const char *cfgName);
extern void battle_validate_conf(void);
extern void battle_set_defaults(void);
extern int battle_set_value(char *, char *);
int battle_get_value(char *);

#endif
