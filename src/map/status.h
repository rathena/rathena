#ifndef _STATUS_H_
#define _STATUS_H_

enum {	// struct map_session_data の status_changeの番?テ?ブル
// SC_SENDMAX未?はクライアントへの通知あり。
// 2-2次職の値はなんかめちゃくちゃっぽいので暫定。たぶん?更されます。
	SC_SENDMAX			= 128,	// note: max is now 182, but we'll need to do alot of moving around
	SC_PROVOKE			= 0,
	SC_ENDURE			= 1,
	SC_TWOHANDQUICKEN	= 2,
	SC_CONCENTRATE		= 3,
	SC_HIDING			= 4,
	SC_CLOAKING			= 5,
	SC_ENCPOISON		= 6,
	SC_POISONREACT		= 7,
	SC_QUAGMIRE			= 8,
	SC_ANGELUS			= 9,
	SC_BLESSING			= 10,
	SC_SIGNUMCRUCIS		= 11,
	SC_INCREASEAGI		= 12,
	SC_DECREASEAGI		= 13,
	SC_SLOWPOISON		= 14,
	SC_IMPOSITIO		= 15,
	SC_SUFFRAGIUM		= 16,
	SC_ASPERSIO			= 17,
	SC_BENEDICTIO		= 18,
	SC_KYRIE			= 19,
	SC_MAGNIFICAT		= 20,
	SC_GLORIA			= 21,
	SC_AETERNA			= 22,
	SC_ADRENALINE		= 23,
	SC_WEAPONPERFECTION	= 24,
	SC_OVERTHRUST		= 25,
	SC_MAXIMIZEPOWER	= 26,
	SC_RIDING			= 27,
	SC_FALCON			= 28,
	SC_TRICKDEAD		= 29,
	SC_LOUD				= 30,
	SC_ENERGYCOAT		= 31,
	SC_BROKNARMOR		= 32,
	SC_BROKNWEAPON		= 33,
	SC_HALLUCINATION	= 34,
	SC_WEIGHT50			= 35,
	SC_WEIGHT90			= 36,
	SC_SPEEDPOTION0		= 37,
	SC_SPEEDPOTION1		= 38,
	SC_SPEEDPOTION2		= 39,
	SC_SPEEDPOTION3		= 40,
	SC_SPEEDUP0			= 41, // for skill speedup
	SC_SPEEDUP1			= 42, // for skill speedup
//-- 43-50
	SC_STRIPWEAPON		= 50,
	SC_STRIPSHIELD		= 51,
	SC_STRIPARMOR		= 52,
	SC_STRIPHELM		= 53,
	SC_CP_WEAPON		= 54,
	SC_CP_SHIELD		= 55,
	SC_CP_ARMOR			= 56,
	SC_CP_HELM			= 57,
	SC_AUTOGUARD		= 58,
	SC_REFLECTSHIELD	= 59,
	SC_DEVOTION			= 60,
	SC_PROVIDENCE		= 61,
	SC_DEFENDER			= 62,
	SC_AUTOSPELL		= 65,
	SC_SPEARSQUICKEN	= 68,
//-- 69-85
	SC_EXPLOSIONSPIRITS	= 86,
	SC_STEELBODY		= 87,
	SC_COMBO			= 89,
	SC_FLAMELAUNCHER	= 90,
	SC_FROSTWEAPON		= 91,
	SC_LIGHTNINGLOADER	= 92,
	SC_SEISMICWEAPON	= 93,
//-- 94-102
	SC_AURABLADE		= 103, /* オ?ラブレ?ド */
	SC_PARRYING			= 104, /* パリイング */
	SC_CONCENTRATION	= 105, /* コンセントレ?ション */
	SC_TENSIONRELAX		= 106, /* テンションリラックス */
	SC_BERSERK			= 107, /* バ?サ?ク */
//-- 108, 109
	SC_ASSUMPTIO		= 110, /* アシャンプティオ */
//-- 111, 112
	SC_MAGICPOWER		= 113, /* 魔法力?幅 */
	SC_EDP				= 114, /* エフェクトが判明したら移動 */
	SC_TRUESIGHT		= 115, /* トゥル?サイト */
	SC_WINDWALK			= 116, /* ウインドウォ?ク */
	SC_MELTDOWN			= 117, /* メルトダウン */
	SC_CARTBOOST		= 118, /* カ?トブ?スト */
//-- 119
	SC_REJECTSWORD		= 120, /* リジェクトソ?ド */
	SC_MARIONETTE		= 121, /* マリオネットコントロ?ル */
	SC_MARIONETTE2		= 122, // Marionette target
//-- 123
	SC_BLEEDING			= 124, /* ヘッドクラッシュ */
	SC_JOINTBEAT		= 125, /* ジョイントビ?ト */
//-- 126, 127

	SC_STONE			= 128,
	SC_FREEZE			= 129,
// <-- 130 = a baby skill status?
	SC_STAN				= 130,
	SC_SLEEP			= 131,
// <-- 132 = another baby skill?
	SC_POISON			= 132,
	SC_CURSE			= 133,
	SC_SILENCE			= 134,
	SC_CONFUSION		= 135,
	SC_BLIND			= 136,
	SC_DIVINA			= SC_SILENCE,
//-- 137-139
	SC_SAFETYWALL		= 140,
	SC_PNEUMA			= 141,
//-- 142
	SC_ANKLE			= 143,
	SC_DANCING			= 144,
	SC_KEEPING			= 145,
	SC_BARRIER			= 146,
//-- 147,148
	SC_MAGICROD			= 149,
	SC_SIGHT			= 150,
	SC_RUWACH			= 151,
	SC_AUTOCOUNTER		= 152,
	SC_VOLCANO			= 153,
	SC_DELUGE			= 154,
	SC_VIOLENTGALE		= 155,
	SC_BLADESTOP_WAIT	= 156,
	SC_BLADESTOP		= 157,
	SC_EXTREMITYFIST	= 158,
//-- 159	
	SC_LULLABY			=160,
	SC_RICHMANKIM		=161,
	SC_ETERNALCHAOS		=162,
	SC_DRUMBATTLE		=163,
	SC_NIBELUNGEN		=164,
	SC_ROKISWEIL		=165,
	SC_INTOABYSS		=166,
	SC_SIEGFRIED		=167,
	SC_DISSONANCE		=168,
	SC_WHISTLE			=169,
	SC_ASSNCROS			=170,
	SC_POEMBRAGI		=171,
	SC_APPLEIDUN		=172,
	SC_UGLYDANCE		=173,
	SC_HUMMING			=174,
	SC_DONTFORGETME		=175,
	SC_FORTUNE			=176,
	SC_SERVICE4U		=177,
	SC_SPIDERWEB		=180,		/* スパイダ?ウェッブ */
// <-- 181 = unknown status
// <-- 182 = unknown status
	SC_SACRIFICE		=184,		/* サクリファイス */
	SC_WEDDING			=187,	//結婚用(結婚衣裳になって?くのが?いとか)
	SC_NOCHAT			=188,	//赤エモ?態
	SC_SPLASHER			=189,	/* ベナムスプラッシャ? */
	SC_SELFDESTRUCTION	=190,	/* 自爆 */
	SC_MEMORIZE			=197,		/* メモライズ */ // changed from 181 to 192
	SC_DPOISON			=198,		/* 猛毒 */

// Used by English Team
	SC_SLOWDOWN			=45, // for skill slowdown
	SC_AUTOBERSERK		=46,
	SC_SIGHTTRASHER		=73,
	SC_BASILICA			=102, // temporarily use this before an actual id is found [celest]	
	
	SC_ENSEMBLE			=159,
	SC_FOGWALL			=178,
	SC_GOSPEL			=179,
	SC_PRESERVE         =181,
	SC_BATTLEORDERS		=182,
	SC_MOONLIT			=183,
	SC_ATKPOT			=185,	// [Valaris]
	SC_MATKPOT			=186,	// [Valaris]
	SC_MINDBREAKER		=191,
	SC_SPELLBREAKER		=192,
	SC_LANDPROTECTOR	=193,
	SC_ADAPTATION		=194,
	SC_CHASEWALK		=195,
	SC_REGENERATION		=196,
	SC_GUILDAURA		=199,
	SC_BABY				=200,

// Icons
	_SC_BABY			=200
};
extern int SkillStatusChangeTable[];

extern int current_equip_item_index;

// パラメータ所得系 battle.c より移動
int status_get_class(struct block_list *bl);
int status_get_dir(struct block_list *bl);
int status_get_lv(struct block_list *bl);
int status_get_range(struct block_list *bl);
int status_get_hp(struct block_list *bl);
int status_get_max_hp(struct block_list *bl);
int status_get_str(struct block_list *bl);
int status_get_agi(struct block_list *bl);
int status_get_vit(struct block_list *bl);
int status_get_int(struct block_list *bl);
int status_get_dex(struct block_list *bl);
int status_get_luk(struct block_list *bl);
int status_get_hit(struct block_list *bl);
int status_get_flee(struct block_list *bl);
int status_get_def(struct block_list *bl);
int status_get_mdef(struct block_list *bl);
int status_get_flee2(struct block_list *bl);
int status_get_def2(struct block_list *bl);
int status_get_mdef2(struct block_list *bl);
int status_get_baseatk(struct block_list *bl);
int status_get_atk(struct block_list *bl);
int status_get_atk2(struct block_list *bl);
int status_get_speed(struct block_list *bl);
int status_get_adelay(struct block_list *bl);
int status_get_amotion(struct block_list *bl);
int status_get_dmotion(struct block_list *bl);
int status_get_element(struct block_list *bl);
int status_get_attack_element(struct block_list *bl);
int status_get_attack_element2(struct block_list *bl);  //左手武器属性取得
#define status_get_elem_type(bl)	(status_get_element(bl)%10)
#define status_get_elem_level(bl)	(status_get_element(bl)/10/2)
int status_get_party_id(struct block_list *bl);
int status_get_guild_id(struct block_list *bl);
int status_get_race(struct block_list *bl);
int status_get_size(struct block_list *bl);
int status_get_mode(struct block_list *bl);
int status_get_mexp(struct block_list *bl);
int status_get_race2(struct block_list *bl);

struct status_change *status_get_sc_data(struct block_list *bl);
short *status_get_sc_count(struct block_list *bl);
short *status_get_opt1(struct block_list *bl);
short *status_get_opt2(struct block_list *bl);
short *status_get_opt3(struct block_list *bl);
short *status_get_option(struct block_list *bl);

int status_get_matk1(struct block_list *bl);
int status_get_matk2(struct block_list *bl);
int status_get_critical(struct block_list *bl);
int status_get_atk_(struct block_list *bl);
int status_get_atk_2(struct block_list *bl);
int status_get_atk2(struct block_list *bl);

int status_isdead(struct block_list *bl);

int status_get_sc_def(struct block_list *bl, int type);
#define status_get_sc_def_mdef(bl)	(status_get_sc_def(bl, SP_MDEF1))
#define status_get_sc_def_vit(bl)	(status_get_sc_def(bl, SP_DEF2))
#define status_get_sc_def_int(bl)	(status_get_sc_def(bl, SP_MDEF2))
#define status_get_sc_def_luk(bl)	(status_get_sc_def(bl, SP_LUK))

// 状態異常関連 skill.c より移動
int status_change_start(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag);
int status_change_end( struct block_list* bl , int type,int tid );
int status_change_timer(int tid, unsigned int tick, int id, int data);
int status_change_timer_sub(struct block_list *bl, va_list ap );
int status_change_clear(struct block_list *bl,int type);

// ステータス計算 pc.c から分離
// pc_calcstatus
int status_calc_pc(struct map_session_data* sd,int first);
int status_calc_speed(struct map_session_data*); // [Celest]
// int status_calc_skilltree(struct map_session_data *sd);
int status_getrefinebonus(int lv,int type);
int status_percentrefinery(struct map_session_data *sd,struct item *item);
extern int percentrefinery[5][10];

int status_readdb(void);
int do_init_status(void);

#endif
