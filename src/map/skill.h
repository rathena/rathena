// $Id: skill.h,v 1.5 2004/11/26 5:47:12 PM Celestia Exp $
#ifndef _SKILL_H_
#define _SKILL_H_

#include "map.h"

#define MAX_SKILL_DB			515
#define MAX_SKILL_PRODUCE_DB	150
#define MAX_PRODUCE_RESOURCE	7
#define MAX_SKILL_ARROW_DB		150
#define MAX_SKILL_ABRA_DB		350

// スキルデ?タベ?ス
struct skill_db {
	int range[MAX_SKILL_LEVEL],hit,inf,pl,nk,max;
	int num[MAX_SKILL_LEVEL];
	int cast[MAX_SKILL_LEVEL],delay[MAX_SKILL_LEVEL];
	int upkeep_time[MAX_SKILL_LEVEL],upkeep_time2[MAX_SKILL_LEVEL];
	int castcancel,cast_def_rate;
	int inf2,maxcount,skill_type;
	int blewcount[MAX_SKILL_LEVEL];
	int hp[MAX_SKILL_LEVEL],sp[MAX_SKILL_LEVEL],mhp[MAX_SKILL_LEVEL],hp_rate[MAX_SKILL_LEVEL],sp_rate[MAX_SKILL_LEVEL],zeny[MAX_SKILL_LEVEL];
	int weapon,state,spiritball[MAX_SKILL_LEVEL];
	int itemid[10],amount[10];
	int castnodex[MAX_SKILL_LEVEL];
	int nocast;
};
extern struct skill_db skill_db[MAX_SKILL_DB];

struct skill_name_db { 
        int id;     // skill id
        char *name; // search strings
        char *desc; // description that shows up for search's
};
extern const struct skill_name_db skill_names[];

// アイテム作成デ?タベ?ス
struct skill_produce_db {
	int nameid, trigger;
	int req_skill,itemlv;
	int mat_id[MAX_PRODUCE_RESOURCE],mat_amount[MAX_PRODUCE_RESOURCE];
};
extern struct skill_produce_db skill_produce_db[MAX_SKILL_PRODUCE_DB];

// 矢作成デ?タベ?ス
struct skill_arrow_db {
	int nameid, trigger;
	int cre_id[5],cre_amount[5];
};
extern struct skill_arrow_db skill_arrow_db[MAX_SKILL_ARROW_DB];

// アブラカダブラデ?タベ?ス
struct skill_abra_db {
	int nameid;
	int req_lv;
	int per;
};
extern struct skill_abra_db skill_abra_db[MAX_SKILL_ABRA_DB];

struct block_list;
struct map_session_data;
struct skill_unit;
struct skill_unit_group;

int do_init_skill(void);

// スキルデ?タベ?スへのアクセサ
int	skill_get_hit( int id );
int	skill_get_inf( int id );
int	skill_get_pl( int id );
int	skill_get_nk( int id );
int	skill_get_max( int id );
int skill_get_range( int id , int lv );
int	skill_get_hp( int id ,int lv );
int	skill_get_mhp( int id ,int lv );
int	skill_get_sp( int id ,int lv );
int	skill_get_zeny( int id ,int lv );
int	skill_get_num( int id ,int lv );
int	skill_get_cast( int id ,int lv );
int	skill_get_delay( int id ,int lv );
int	skill_get_time( int id ,int lv );
int	skill_get_time2( int id ,int lv );
int	skill_get_castdef( int id );
int	skill_get_weapontype( int id );
int skill_get_unit_id(int id,int flag);
int	skill_get_inf2( int id );
int	skill_get_maxcount( int id );
int	skill_get_blewcount( int id ,int lv );
int	skill_tree_get_max( int id, int b_class );	// Celest

// スキルの使用
int skill_use_id( struct map_session_data *sd, int target_id,
	int skill_num,int skill_lv);
int skill_use_pos( struct map_session_data *sd,
	int skill_x, int skill_y, int skill_num, int skill_lv);

int skill_castend_map( struct map_session_data *sd,int skill_num, const char *map);

int skill_cleartimerskill(struct block_list *src);
int skill_addtimerskill(struct block_list *src,unsigned int tick,int target,int x,int y,int skill_id,int skill_lv,int type,int flag);

// 追加?果
int skill_additional_effect( struct block_list* src, struct block_list *bl,int skillid,int skilllv,int attack_type,unsigned int tick);

// ユニットスキル
struct skill_unit_group *skill_unitsetting( struct block_list *src, int skillid,int skilllv,int x,int y,int flag);
struct skill_unit *skill_initunit(struct skill_unit_group *group,int idx,int x,int y);
int skill_delunit(struct skill_unit *unit);
struct skill_unit_group *skill_initunitgroup(struct block_list *src,
	int count,int skillid,int skilllv,int unit_id);
int skill_delunitgroup(struct skill_unit_group *group);
struct skill_unit_group_tickset *skill_unitgrouptickset_search(
	struct block_list *bl,int group_id);
int skill_unitgrouptickset_delete(struct block_list *bl,int group_id);
int skill_clear_unitgroup(struct block_list *src);

int skill_unit_ondamaged(struct skill_unit *src,struct block_list *bl,
	int damage,unsigned int tick);

int skill_castfix( struct block_list *bl, int time );
int skill_delayfix( struct block_list *bl, int time );
int skill_check_unit_range(int m,int x,int y,int range,int skillid);
int skill_check_unit_range2(int m,int x,int y,int range);
// -- moonsoul	(added skill_check_unit_cell)
int skill_check_unit_cell(int skillid,int m,int x,int y,int unit_id);
int skill_unit_out_all( struct block_list *bl,unsigned int tick,int range);
int skill_unit_move( struct block_list *bl,unsigned int tick,int range);
int skill_unit_move_unit_group( struct skill_unit_group *group, int m,int dx,int dy);

struct skill_unit_group *skill_check_dancing( struct block_list *src );
void skill_stop_dancing(struct block_list *src, int flag);

// 詠唱キャンセル
int skill_castcancel(struct block_list *bl,int type);

int skill_gangsterparadise(struct map_session_data *sd ,int type);
void skill_brandishspear_first(struct square *tc,int dir,int x,int y);
void skill_brandishspear_dir(struct square *tc,int dir,int are);
int skill_autospell(struct map_session_data *md,int skillid);
void skill_devotion(struct map_session_data *md,int target);
void skill_devotion2(struct block_list *bl,int crusader);
int skill_devotion3(struct block_list *bl,int target);
void skill_devotion_end(struct map_session_data *md,struct map_session_data *sd,int target);

#define skill_calc_heal(bl,skill_lv) (( battle_get_lv(bl)+battle_get_int(bl) )/8 *(4+ skill_lv*8))

// その他
int skill_check_cloaking(struct block_list *bl);
int skill_type_cloaking(struct block_list *bl);
int skill_is_danceskill(int id);

// ステ?タス異常
int skill_status_change_start(struct block_list *bl,int type,int val1,int val2,int val3,int val4,int tick,int flag);
int skill_status_change_timer(int tid, unsigned int tick, int id, int data);
int skill_encchant_eremental_end(struct block_list *bl, int type);
int skill_status_change_end( struct block_list* bl , int type,int tid );
int skill_status_change_clear(struct block_list *bl,int type);
int skillnotok(int skillid, struct map_session_data *sd);

// アイテム作成
int skill_can_produce_mix( struct map_session_data *sd, int nameid, int trigger );
int skill_produce_mix( struct map_session_data *sd,
	int nameid, int slot1, int slot2, int slot3 );

int skill_arrow_create( struct map_session_data *sd,int nameid);

// mobスキルのため
int skill_castend_nodamage_id( struct block_list *src, struct block_list *bl,int skillid,int skilllv,unsigned int tick,int flag );
int skill_castend_damage_id( struct block_list* src, struct block_list *bl,int skillid,int skilllv,unsigned int tick,int flag );
int skill_castend_pos2( struct block_list *src, int x,int y,int skillid,int skilllv,unsigned int tick,int flag);

// スキル攻?一括?理
int skill_attack( int attack_type, struct block_list* src, struct block_list *dsrc,
	 struct block_list *bl,int skillid,int skilllv,unsigned int tick,int flag );

void skill_reload(void);

enum {
	ST_NONE,ST_HIDING,ST_CLOAKING,ST_HIDDEN,ST_RIDING,ST_FALCON,ST_CART,ST_SHIELD,ST_SIGHT,ST_EXPLOSIONSPIRITS,
	ST_RECOV_WEIGHT_RATE,ST_MOVE_ENABLE,ST_WATER,
};

enum {	// struct map_session_data の status_changeの番?テ?ブル
// SC_SENDMAX未?はクライアントへの通知あり。
// 2-2次職の値はなんかめちゃくちゃっぽいので暫定。たぶん?更されます。
	SC_SENDMAX				=128,
	SC_PROVOKE			= 0,
	SC_ENDURE			= 1,
	SC_TWOHANDQUICKEN		= 2,
	SC_CONCENTRATE		= 3,
	SC_HIDING			= 4,
	SC_CLOAKING			= 5,
	SC_ENCPOISON			= 6,
	SC_POISONREACT		= 7,
	SC_QUAGMIRE			= 8,
	SC_ANGELUS			= 9,
	SC_BLESSING			=10,
	SC_SIGNUMCRUCIS		=11,
	SC_INCREASEAGI			=12,
	SC_DECREASEAGI		=13,
	SC_SLOWPOISON			=14,
	SC_IMPOSITIO			=15,
	SC_SUFFRAGIUM			=16,
	SC_ASPERSIO			=17,
	SC_BENEDICTIO			=18,
	SC_KYRIE			=19,
	SC_MAGNIFICAT			=20,
	SC_GLORIA			=21,
	SC_AETERNA			=22,
	SC_ADRENALINE			=23,
	SC_WEAPONPERFECTION		=24,
	SC_OVERTHRUST			=25,
	SC_MAXIMIZEPOWER		=26,
	SC_RIDING			=27,
	SC_FALCON			=28,
	SC_TRICKDEAD			=29,
	SC_LOUD			=30,
	SC_ENERGYCOAT		=31,
	SC_HALLUCINATION		=34,
	SC_WEIGHT50			=35,
	SC_WEIGHT90			=36,
	SC_SPEEDPOTION0		=37,
	SC_SPEEDPOTION1		=38,
	SC_SPEEDPOTION2		=39,
	SC_STRIPWEAPON		=50,
	SC_STRIPSHIELD			=51,
	SC_STRIPARMOR			=52,
	SC_STRIPHELM			=53,
	SC_CP_WEAPON			=54,
	SC_CP_SHIELD			=55,
	SC_CP_ARMOR			=56,
	SC_CP_HELM			=57,
	SC_AUTOGUARD			=58,
	SC_REFLECTSHIELD		=59,
	SC_DEVOTION			=60,
	SC_PROVIDENCE			=61,
	SC_DEFENDER			=62,
	SC_AUTOSPELL			=65,
	SC_SPEARSQUICKEN		=68,
	SC_EXPLOSIONSPIRITS		=86,
	SC_STEELBODY			=87,
	SC_COMBO			=89,
	SC_FLAMELAUNCHER		=90,
	SC_FROSTWEAPON		=91,
	SC_LIGHTNINGLOADER		=92,
	SC_SEISMICWEAPON		=93,
	SC_AURABLADE			=103, /* オ?ラブレ?ド */
	SC_PARRYING			=104, /* パリイング */
	SC_CONCENTRATION		=105, /* コンセントレ?ション */
	SC_TENSIONRELAX		=106, /* テンションリラックス */
	SC_BERSERK			=107, /* バ?サ?ク */
	SC_ASSUMPTIO			=110, /* アシャンプティオ */
	SC_MAGICPOWER		=113, /* 魔法力?幅 */
	SC_TRUESIGHT			=115, /* トゥル?サイト */
	SC_WINDWALK			=116, /* ウインドウォ?ク */
	SC_MELTDOWN			=117, /* メルトダウン */
	SC_CARTBOOST			=118, /* カ?トブ?スト */
	SC_REJECTSWORD		=120, /* リジェクトソ?ド */
	SC_MARIONETTE			=121, /* マリオネットコントロ?ル */
	SC_HEADCRUSH			=124, /* ヘッドクラッシュ */
	SC_JOINTBEAT			=125, /* ジョイントビ?ト */

	SC_STONE			=128,
	SC_FREEZE			=129,
	SC_STAN			=130,
	SC_SLEEP			=131,
	SC_POISON			=132,
	SC_CURSE			=133,
	SC_SILENCE			=134,
	SC_CONFUSION			=135,
	SC_BLIND			=136,
	SC_DIVINA			= SC_SILENCE,

	SC_SAFETYWALL			=140,
	SC_PNEUMA			=141,
	SC_WATERBALL			=142,
	SC_ANKLE			=143,
	SC_DANCING			=144,
	SC_KEEPING			=145,
	SC_BARRIER			=146,

	SC_MAGICROD			=149,
	SC_SIGHT			=150,
	SC_RUWACH			=151,
	SC_AUTOCOUNTER		=152,
	SC_VOLCANO			=153,
	SC_DELUGE			=154,
	SC_VIOLENTGALE		=155,
	SC_BLADESTOP_WAIT		=156,
	SC_BLADESTOP			=157,
	SC_EXTREMITYFIST		=158,
	SC_GRAFFITI				=159,

	SC_LULLABY			=160,
	SC_RICHMANKIM			=161,
	SC_ETERNALCHAOS		=162,
	SC_DRUMBATTLE		=163,
	SC_NIBELUNGEN			=164,
	SC_ROKISWEIL			=165,
	SC_INTOABYSS			=166,
	SC_SIEGFRIED			=167,
	SC_DISSONANCE			=168,
	SC_WHISTLE			=169,
	SC_ASSNCROS			=170,
	SC_POEMBRAGI			=171,
	SC_APPLEIDUN			=172,
	SC_UGLYDANCE			=173,
	SC_HUMMING			=174,
	SC_DONTFORGETME		=175,
	SC_FORTUNE			=176,
	SC_SERVICE4U			=177,

	SC_SPIDERWEB			=180,		/* スパイダ?ウェッブ */
	SC_MEMORIZE			=181,		/* メモライズ */
//	SC_DPOISON			=182,		/* 猛毒 */

//	SC_EDP				=183,		/* エフェクトが判明したら移動 */

	SC_WEDDING			=187,	//結婚用(結婚衣裳になって?くのが?いとか)
	SC_NOCHAT			=188,	//赤エモ?態
	SC_SPLASHER			=189,	/* ベナムスプラッシャ? */
	SC_SELFDESTRUCTION		=190,	/* 自爆 */


// Used by English Team
	SC_BROKNARMOR		=32,
	SC_BROKNWEAPON		=33,
	SC_SLOWDOWN			=45, // for skill slowdown
	SC_SIGHTTRASHER		=73,
//	SC_BASILICA			=125, // 125 is the same id as joint break
	SC_BASILICA			=102, // temporarily use this before an actual id is found [celest]
	SC_EDP				=114, // 
	SC_MARIONETTE2		=122, // Marionette target
	SC_ENSEMBLE			=159,
	SC_FOGWALL			=178,
	SC_GOSPEL			=179,
	SC_LANDPROTECTOR	=182,
	SC_ADAPTATION		=183,
	SC_CHASEWALK		=184,
	SC_ATKPOT			=185,	// [Valaris]
	SC_MATKPOT			=186,	// [Valaris]
	SC_MINDBREAKER		=191,
	SC_SPELLBREAKER		=192,
	SC_DPOISON			=193, /* 猛毒 */
	SC_BLOCKSKILL		=194, // for disallowing the use of a skill for a time period

// [Celest]
	SC_BLEEDING			= 124, // Temporarily same id as headcrush	
	SC_MOONLIT			= 195,
	SC_LEADERSHIP		= 196,
	SC_GLORYWOUNDS		= 197,
	SC_SOULCOLD			= 198,
	SC_HAWKEYES			= 199,
	SC_BATTLEORDERS		= 200,
	SC_REGENERATION		= 201,
	SC_PRESERVE         = 202,

// -- testing various SC effects
//	SC_AURABLADE			=81,
//	SC_CONCENTRATION		=83,
//	SC_TENSIONRELAX		=84,
//	SC_BERSERK			=85,
//	SC_CALLSPIRITS			=100,
//	SC_PARRYING			=100,
//	SC_FREECAST			=101,
//	SC_ABSORBSPIRIT		=102,
//	SC_ASSUMPTIO			=114,
//	SC_SHARPSHOOT			=127,
//	SC_GANGSTER			=184,
//	SC_CANNIBALIZE			=186,
//	SC_SPHEREMINE			=187,
//	SC_METEOSTORM		=189,
//	SC_CASTCANCEL			=190,
//	SC_SPIDERWEB			=191,
};
extern int SkillStatusChangeTable[];

enum {
	NV_BASIC = 1,

	SM_SWORD,
	SM_TWOHAND,
	SM_RECOVERY,
	SM_BASH,
	SM_PROVOKE,
	SM_MAGNUM,
	SM_ENDURE,

	MG_SRECOVERY,
	MG_SIGHT,
	MG_NAPALMBEAT,
	MG_SAFETYWALL,
	MG_SOULSTRIKE,
	MG_COLDBOLT,
	MG_FROSTDIVER,
	MG_STONECURSE,
	MG_FIREBALL,
	MG_FIREWALL,
	MG_FIREBOLT,
	MG_LIGHTNINGBOLT,
	MG_THUNDERSTORM,

	AL_DP,
	AL_DEMONBANE,
	AL_RUWACH,
	AL_PNEUMA,
	AL_TELEPORT,
	AL_WARP,
	AL_HEAL,
	AL_INCAGI,
	AL_DECAGI,
	AL_HOLYWATER,
	AL_CRUCIS,
	AL_ANGELUS,
	AL_BLESSING,
	AL_CURE,

	MC_INCCARRY,
	MC_DISCOUNT,
	MC_OVERCHARGE,
	MC_PUSHCART,
	MC_IDENTIFY,
	MC_VENDING,
	MC_MAMMONITE,

	AC_OWL,
	AC_VULTURE,
	AC_CONCENTRATION,
	AC_DOUBLE,
	AC_SHOWER,

	TF_DOUBLE,
	TF_MISS,
	TF_STEAL,
	TF_HIDING,
	TF_POISON,
	TF_DETOXIFY,

	ALL_RESURRECTION,

	KN_SPEARMASTERY,
	KN_PIERCE,
	KN_BRANDISHSPEAR,
	KN_SPEARSTAB,
	KN_SPEARBOOMERANG,
	KN_TWOHANDQUICKEN,
	KN_AUTOCOUNTER,
	KN_BOWLINGBASH,
	KN_RIDING,
	KN_CAVALIERMASTERY,

	PR_MACEMASTERY,
	PR_IMPOSITIO,
	PR_SUFFRAGIUM,
	PR_ASPERSIO,
	PR_BENEDICTIO,
	PR_SANCTUARY,
	PR_SLOWPOISON,
	PR_STRECOVERY,
	PR_KYRIE,
	PR_MAGNIFICAT,
	PR_GLORIA,
	PR_LEXDIVINA,
	PR_TURNUNDEAD,
	PR_LEXAETERNA,
	PR_MAGNUS,

	WZ_FIREPILLAR,
	WZ_SIGHTRASHER,
	WZ_FIREIVY,
	WZ_METEOR,
	WZ_JUPITEL,
	WZ_VERMILION,
	WZ_WATERBALL,
	WZ_ICEWALL,
	WZ_FROSTNOVA,
	WZ_STORMGUST,
	WZ_EARTHSPIKE,
	WZ_HEAVENDRIVE,
	WZ_QUAGMIRE,
	WZ_ESTIMATION,

	BS_IRON,
	BS_STEEL,
	BS_ENCHANTEDSTONE,
	BS_ORIDEOCON,
	BS_DAGGER,
	BS_SWORD,
	BS_TWOHANDSWORD,
	BS_AXE,
	BS_MACE,
	BS_KNUCKLE,
	BS_SPEAR,
	BS_HILTBINDING,
	BS_FINDINGORE,
	BS_WEAPONRESEARCH,
	BS_REPAIRWEAPON,
	BS_SKINTEMPER,
	BS_HAMMERFALL,
	BS_ADRENALINE,
	BS_WEAPONPERFECT,
	BS_OVERTHRUST,
	BS_MAXIMIZE,

	HT_SKIDTRAP,
	HT_LANDMINE,
	HT_ANKLESNARE,
	HT_SHOCKWAVE,
	HT_SANDMAN,
	HT_FLASHER,
	HT_FREEZINGTRAP,
	HT_BLASTMINE,
	HT_CLAYMORETRAP,
	HT_REMOVETRAP,
	HT_TALKIEBOX,
	HT_BEASTBANE,
	HT_FALCON,
	HT_STEELCROW,
	HT_BLITZBEAT,
	HT_DETECTING,
	HT_SPRINGTRAP,

	AS_RIGHT,
	AS_LEFT,
	AS_KATAR,
	AS_CLOAKING,
	AS_SONICBLOW,
	AS_GRIMTOOTH,
	AS_ENCHANTPOISON,
	AS_POISONREACT,
	AS_VENOMDUST,
	AS_SPLASHER,

	NV_FIRSTAID,
	NV_TRICKDEAD,
	SM_MOVINGRECOVERY,
	SM_FATALBLOW,
	SM_AUTOBERSERK,
	AC_MAKINGARROW,
	AC_CHARGEARROW,
	TF_SPRINKLESAND,
	TF_BACKSLIDING,
	TF_PICKSTONE,
	TF_THROWSTONE,
	MC_CARTREVOLUTION,
	MC_CHANGECART,
	MC_LOUD,
	AL_HOLYLIGHT,
	MG_ENERGYCOAT,

	NPC_PIERCINGATT,
	NPC_MENTALBREAKER,
	NPC_RANGEATTACK,
	NPC_ATTRICHANGE,
	NPC_CHANGEWATER,
	NPC_CHANGEGROUND,
	NPC_CHANGEFIRE,
	NPC_CHANGEWIND,
	NPC_CHANGEPOISON,
	NPC_CHANGEHOLY,
	NPC_CHANGEDARKNESS,
	NPC_CHANGETELEKINESIS,
	NPC_CRITICALSLASH,
	NPC_COMBOATTACK,
	NPC_GUIDEDATTACK,
	NPC_SELFDESTRUCTION,
	NPC_SPLASHATTACK,
	NPC_SUICIDE,
	NPC_POISON,
	NPC_BLINDATTACK,
	NPC_SILENCEATTACK,
	NPC_STUNATTACK,
	NPC_PETRIFYATTACK,
	NPC_CURSEATTACK,
	NPC_SLEEPATTACK,
	NPC_RANDOMATTACK,
	NPC_WATERATTACK,
	NPC_GROUNDATTACK,
	NPC_FIREATTACK,
	NPC_WINDATTACK,
	NPC_POISONATTACK,
	NPC_HOLYATTACK,
	NPC_DARKNESSATTACK,
	NPC_TELEKINESISATTACK,
	NPC_MAGICALATTACK,
	NPC_METAMORPHOSIS,
	NPC_PROVOCATION,
	NPC_SMOKING,
	NPC_SUMMONSLAVE,
	NPC_EMOTION,
	NPC_TRANSFORMATION,
	NPC_BLOODDRAIN,
	NPC_ENERGYDRAIN,
	NPC_KEEPING,
	NPC_DARKBREATH,
	NPC_DARKBLESSING,
	NPC_BARRIER,
	NPC_DEFENDER,
	NPC_LICK,
	NPC_HALLUCINATION,
	NPC_REBIRTH,
	NPC_SUMMONMONSTER,

	RG_SNATCHER,
	RG_STEALCOIN,
	RG_BACKSTAP,
	RG_TUNNELDRIVE,
	RG_RAID,
	RG_STRIPWEAPON,
	RG_STRIPSHIELD,
	RG_STRIPARMOR,
	RG_STRIPHELM,
	RG_INTIMIDATE,
	RG_GRAFFITI,
	RG_FLAGGRAFFITI,
	RG_CLEANER,
	RG_GANGSTER,
	RG_COMPULSION,
	RG_PLAGIARISM,

	AM_AXEMASTERY,
	AM_LEARNINGPOTION,
	AM_PHARMACY,
	AM_DEMONSTRATION,
	AM_ACIDTERROR,
	AM_POTIONPITCHER,
	AM_CANNIBALIZE,
	AM_SPHEREMINE,
	AM_CP_WEAPON,
	AM_CP_SHIELD,
	AM_CP_ARMOR,
	AM_CP_HELM,
	AM_BIOETHICS,
	AM_BIOTECHNOLOGY,
	AM_CREATECREATURE,
	AM_CULTIVATION,
	AM_FLAMECONTROL,
	AM_CALLHOMUN,
	AM_REST,
	AM_DRILLMASTER,
	AM_HEALHOMUN,
	AM_RESURRECTHOMUN,

	CR_TRUST,
	CR_AUTOGUARD,
	CR_SHIELDCHARGE,
	CR_SHIELDBOOMERANG,
	CR_REFLECTSHIELD,
	CR_HOLYCROSS,
	CR_GRANDCROSS,
	CR_DEVOTION,
	CR_PROVIDENCE,
	CR_DEFENDER,
	CR_SPEARQUICKEN,

	MO_IRONHAND,
	MO_SPIRITSRECOVERY,
	MO_CALLSPIRITS,
	MO_ABSORBSPIRITS,
	MO_TRIPLEATTACK,
	MO_BODYRELOCATION,
	MO_DODGE,
	MO_INVESTIGATE,
	MO_FINGEROFFENSIVE,
	MO_STEELBODY,
	MO_BLADESTOP,
	MO_EXPLOSIONSPIRITS,
	MO_EXTREMITYFIST,
	MO_CHAINCOMBO,
	MO_COMBOFINISH,

	SA_ADVANCEDBOOK,
	SA_CASTCANCEL,
	SA_MAGICROD,
	SA_SPELLBREAKER,
	SA_FREECAST,
	SA_AUTOSPELL,
	SA_FLAMELAUNCHER,
	SA_FROSTWEAPON,
	SA_LIGHTNINGLOADER,
	SA_SEISMICWEAPON,
	SA_DRAGONOLOGY,
	SA_VOLCANO,
	SA_DELUGE,
	SA_VIOLENTGALE,
	SA_LANDPROTECTOR,
	SA_DISPELL,
	SA_ABRACADABRA,
	SA_MONOCELL,
	SA_CLASSCHANGE,
	SA_SUMMONMONSTER,
	SA_REVERSEORCISH,
	SA_DEATH,
	SA_FORTUNE,
	SA_TAMINGMONSTER,
	SA_QUESTION,
	SA_GRAVITY,
	SA_LEVELUP,
	SA_INSTANTDEATH,
	SA_FULLRECOVERY,
	SA_COMA,

	BD_ADAPTATION,
	BD_ENCORE,
	BD_LULLABY,
	BD_RICHMANKIM,
	BD_ETERNALCHAOS,
	BD_DRUMBATTLEFIELD,
	BD_RINGNIBELUNGEN,
	BD_ROKISWEIL,
	BD_INTOABYSS,
	BD_SIEGFRIED,
	BD_RAGNAROK,

	BA_MUSICALLESSON,
	BA_MUSICALSTRIKE,
	BA_DISSONANCE,
	BA_FROSTJOKE,
	BA_WHISTLE,
	BA_ASSASSINCROSS,
	BA_POEMBRAGI,
	BA_APPLEIDUN,

	DC_DANCINGLESSON,
	DC_THROWARROW,
	DC_UGLYDANCE,
	DC_SCREAM,
	DC_HUMMING,
	DC_DONTFORGETME,
	DC_FORTUNEKISS,
	DC_SERVICEFORYOU,

	WE_MALE = 334,
	WE_FEMALE,
	WE_CALLPARTNER,

	NPC_SELFDESTRUCTION2 = 331,
	ITM_TOMAHAWK = 337,
	NPC_DARKCROSS = 338,

	LK_AURABLADE = 355,
	LK_PARRYING,
	LK_CONCENTRATION,
	LK_TENSIONRELAX,
	LK_BERSERK,
	LK_FURY,
	HP_ASSUMPTIO,
	HP_BASILICA,
	HP_MEDITATIO,
	HW_SOULDRAIN,
	HW_MAGICCRASHER,
	HW_MAGICPOWER,
	PA_PRESSURE,
	PA_SACRIFICE,
	PA_GOSPEL,
	CH_PALMSTRIKE,
	CH_TIGERFIST,
	CH_CHAINCRUSH,
	PF_HPCONVERSION,
	PF_SOULCHANGE,
	PF_SOULBURN,
	ASC_KATAR,
	ASC_HALLUCINATION,
	ASC_EDP,
	ASC_BREAKER,
	SN_SIGHT,
	SN_FALCONASSAULT,
	SN_SHARPSHOOTING,
	SN_WINDWALK,
	WS_MELTDOWN,
	WS_CREATECOIN,
	WS_CREATENUGGET,
	WS_CARTBOOST,
	WS_SYSTEMCREATE,
	ST_CHASEWALK,
	ST_REJECTSWORD,
	ST_STEALBACKPACK,
	CR_ALCHEMY,
	CR_SYNTHESISPOTION,
	CG_ARROWVULCAN,
	CG_MOONLIT,
	CG_MARIONETTE,
	LK_SPIRALPIERCE,
	LK_HEADCRUSH,
	LK_JOINTBEAT,
	HW_NAPALMVULCAN,
	CH_SOULCOLLECT,
	PF_MINDBREAKER,
	PF_MEMORIZE,
	PF_FOGWALL,
	PF_SPIDERWEB,
	ASC_METEORASSAULT,
	ASC_CDP,
	WE_BABY,
	WE_CALLPARENT,
	WE_CALLBABY,
	TK_RUN,
	TK_READYSTORM,
	TK_STORMKICK,
	TK_READYDOWN,
	TK_DOWNKICK,
	TK_READYTURN,
	TK_TURNKICK,
	TK_READYCOUNTER,
	TK_COUNTER,
	TK_DODGE,
	TK_JUMPKICK,
	TK_HPTIME,
	TK_SPTIME,
	TK_POWER,
	TK_SEVENWIND,
	TK_HIGHJUMP,
	SG_FEEL,
	SG_SUN_WARM,
	SG_MOON_WARM,
	SG_STAR_WARM,
	SG_SUN_COMFORT,
	SG_MOON_COMFORT,
	SG_STAR_COMFORT,
	SG_HATE,
	SG_SUN_ANGER,
	SG_MOON_ANGER,
	SG_STAR_ANGER,
	SG_SUN_BLESS,
	SG_MOON_BLESS,
	SG_STAR_BLESS,
	SG_DEVIL,
	SG_FRIEND,
	SG_KNOWLEDGE,
	SG_FUSION,
	SL_ALCHEMIST,
	AM_BERSERKPITCHER,
	SL_MONK,
	SL_STAR,
	SL_SAGE,
	SL_CRUSADER,
	SL_SUPERNOVICE,
	SL_KNIGHT,
	SL_WIZARD,
	SL_PRIEST,
	SL_BARDDANCER,
	SL_ROGUE,
	SL_ASSASIN,
	SL_BLACKSMITH,
	BS_ADRENALINE2,
	SL_HUNTER,
	SL_SOULLINKER,
	SL_KAIZEL,
	SL_KAAHI,
	SL_KAUPE,
	SL_KAITE,
	SL_KAINA,
	SL_STIN,
	SL_STUN,
	SL_SMA,
	SL_SWOO,
	SL_SKE,
	SL_SKA,
	
	ST_PRESERVE = 475,
	ST_FULLSTRIP,
	WS_WEAPONREFINE,
	CR_SLIMPITCHER,
	CR_FULLPROTECTION,
	

//	moved to common/mmo.h
/*	GD_APPROVAL=10000,
	GD_KAFRACONTACT=10001,
	GD_GUARDIANRESEARCH=10002,
	GD_GUARDUP=10003,
	GD_EXTENSION=10004,
	GD_GLORYGUILD=10005,
	GD_LEADERSHIP=10006,
	GD_GLORYWOUNDS=10007,
	GD_SOULCOLD=10008,
	GD_HAWKEYES=10009,
	GD_BATTLEORDER=10010,
	GD_REGENERATION=10011,
	GD_RESTORE=10012,
	GD_EMERGENCYCALL=10013,
	GD_DEVELOPMENT=10014,*/
};

#endif
