// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _UNIT_H_
#define _UNIT_H_

//#include "map.h"
struct block_list;
struct unit_data;
struct map_session_data;

#include "clif.h"  // clr_type
#include "map.h" // struct block_list
#include "path.h" // struct walkpath_data
#include "skill.h" // struct skill_timerskill, struct skill_unit_group, struct skill_unit_group_tickset

struct unit_data {
	struct block_list *bl;
	struct walkpath_data walkpath;
	struct skill_timerskill *skilltimerskill[MAX_SKILLTIMERSKILL];
	struct skill_unit_group *skillunit[MAX_SKILLUNITGROUP];
	struct skill_unit_group_tickset skillunittick[MAX_SKILLUNITGROUPTICKSET];
	short attacktarget_lv;
	short to_x,to_y;
	short skillx,skilly;
	short skillid,skilllv;
	int   skilltarget;
	int   skilltimer;
	int   target;
	int   attacktimer;
	int   walktimer;
	int	chaserange;
	unsigned int attackabletime;
	unsigned int canact_tick;
	unsigned int canmove_tick;
	uint8 dir;
	unsigned char walk_count;
	struct {
		unsigned change_walk_target : 1 ;
		unsigned skillcastcancel : 1 ;
		unsigned attack_continue : 1 ;
		unsigned walk_easy : 1 ;
		unsigned running : 1;
		unsigned speed_changed : 1;
	} state;
};

struct view_data {
#ifdef __64BIT__
	unsigned int class_;
#endif
	unsigned short
#ifndef __64BIT__
		class_,
#endif
		weapon,
		shield, //Or left-hand weapon.
		robe,
		head_top,
		head_mid,
		head_bottom,
		hair_style,
		hair_color,
		cloth_color;
	char sex;
	unsigned dead_sit : 2;
};

// PC, MOB, PET に共通する処理を１つにまとめる計画

// 歩行開始
//     戻り値は、0 ( 成功 ), 1 ( 失敗 )
int unit_walktoxy( struct block_list *bl, short x, short y, int easy);
int unit_walktobl( struct block_list *bl, struct block_list *target, int range, int easy);
int unit_run(struct block_list *bl);
int unit_calc_pos(struct block_list *bl, int tx, int ty, int dir);

// 歩行停止
// typeは以下の組み合わせ : 
//     1: 位置情報の送信( この関数の後に位置情報を送信する場合は不要 )
//     2: ダメージディレイ有り
//     4: 不明(MOBのみ？)
int unit_stop_walking(struct block_list *bl,int type);
int unit_can_move(struct block_list *bl);
int unit_is_walking(struct block_list *bl);
int unit_set_walkdelay(struct block_list *bl, unsigned int tick, int delay, int type);

int unit_escape(struct block_list *bl, struct block_list *target, short dist);
// 位置の強制移動(吹き飛ばしなど)
int unit_movepos(struct block_list *bl, short dst_x, short dst_y, int easy, bool checkpath);
int unit_warp(struct block_list *bl, short map, short x, short y, clr_type type);
int unit_setdir(struct block_list *bl,unsigned char dir);
uint8 unit_getdir(struct block_list *bl);
int unit_blown(struct block_list* bl, int dx, int dy, int count, int flag);

// そこまで歩行でたどり着けるかの判定
bool unit_can_reach_pos(struct block_list *bl,int x,int y,int easy);
bool unit_can_reach_bl(struct block_list *bl,struct block_list *tbl, int range, int easy, short *x, short *y);

// 攻撃関連
int unit_stop_attack(struct block_list *bl);
int unit_attack(struct block_list *src,int target_id,int continuous);
int unit_cancel_combo(struct block_list *bl);

// スキル使用
int unit_skilluse_id(struct block_list *src, int target_id, short skill_num, short skill_lv);
int unit_skilluse_pos(struct block_list *src, short skill_x, short skill_y, short skill_num, short skill_lv);

// スキル使用( 補正済みキャスト時間、キャンセル不可設定付き )
int unit_skilluse_id2(struct block_list *src, int target_id, short skill_num, short skill_lv, int casttime, int castcancel);
int unit_skilluse_pos2( struct block_list *src, short skill_x, short skill_y, short skill_num, short skill_lv, int casttime, int castcancel);

// 詠唱キャンセル
int unit_skillcastcancel(struct block_list *bl,int type);

int unit_counttargeted(struct block_list *bl,int target_lv);

// unit_data の初期化処理
void unit_dataset(struct block_list *bl);

int unit_fixdamage(struct block_list *src,struct block_list *target,unsigned int tick,int sdelay,int ddelay,int damage,int div,int type,int damage2);
// その他
struct unit_data* unit_bl2ud(struct block_list *bl);
void unit_remove_map_pc(struct map_session_data *sd, clr_type clrtype);
void unit_free_pc(struct map_session_data *sd);
#define unit_remove_map(bl,clrtype) unit_remove_map_(bl,clrtype,__FILE__,__LINE__,__func__)
int unit_remove_map_(struct block_list *bl, clr_type clrtype, const char* file, int line, const char* func);
int unit_free(struct block_list *bl, clr_type clrtype);
int unit_changeviewsize(struct block_list *bl,short size);

// 初期化ルーチン
int do_init_unit(void);
int do_final_unit(void);
/**
 * Ranger
 **/
int unit_wugdash(struct block_list *bl, struct map_session_data *sd);

extern const short dirx[8];
extern const short diry[8];

#endif /* _UNIT_H_ */
