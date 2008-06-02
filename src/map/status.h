// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _STATUS_H_
#define _STATUS_H_

struct block_list;
struct mob_data;
struct pet_data;
struct homun_data;
struct status_change;

//Use this to refer the max refinery level [Skotlex]
#define MAX_REFINE 10
#define MAX_REFINE_BONUS 5

extern unsigned long StatusChangeFlagTable[];


// Status changes listing. These code are for use by the server. 
typedef enum sc_type {
	SC_NONE = -1,

	//First we enumerate common status ailments which are often used around.
	SC_STONE = 0,
	SC_COMMON_MIN = 0, // begin
	SC_FREEZE,
	SC_STUN,
	SC_SLEEP,
	SC_POISON,
	SC_CURSE,
	SC_SILENCE,
	SC_CONFUSION,
	SC_BLIND,
	SC_BLEEDING,
	SC_DPOISON, //10
	SC_COMMON_MAX = 10, // end
	
	//Next up, we continue on 20, to leave enough room for additional "common" ailments in the future.
	SC_PROVOKE = 20,
	SC_ENDURE,
	SC_TWOHANDQUICKEN,
	SC_CONCENTRATE,
	SC_HIDING,
	SC_CLOAKING,
	SC_ENCPOISON,
	SC_POISONREACT,
	SC_QUAGMIRE,
	SC_ANGELUS,
	SC_BLESSING, //30
	SC_SIGNUMCRUCIS,
	SC_INCREASEAGI,
	SC_DECREASEAGI,
	SC_SLOWPOISON,
	SC_IMPOSITIO  ,
	SC_SUFFRAGIUM,
	SC_ASPERSIO,
	SC_BENEDICTIO,
	SC_KYRIE,
	SC_MAGNIFICAT, //40
	SC_GLORIA,
	SC_AETERNA,
	SC_ADRENALINE,
	SC_WEAPONPERFECTION,
	SC_OVERTHRUST,
	SC_MAXIMIZEPOWER,
	SC_TRICKDEAD,
	SC_LOUD,
	SC_ENERGYCOAT,
	SC_BROKENARMOR, //50 - NOTE: These two aren't used anywhere, and they have an icon...
	SC_BROKENWEAPON,
	SC_HALLUCINATION,
	SC_WEIGHT50,
	SC_WEIGHT90,
	SC_ASPDPOTION0,
	SC_ASPDPOTION1,
	SC_ASPDPOTION2,
	SC_ASPDPOTION3,
	SC_SPEEDUP0,
	SC_SPEEDUP1, //60
	SC_ATKPOTION,
	SC_MATKPOTION,
	SC_WEDDING,
	SC_SLOWDOWN,
	SC_ANKLE,
	SC_KEEPING,
	SC_BARRIER,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR, //70
	SC_STRIPHELM,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	SC_AUTOGUARD,
	SC_REFLECTSHIELD,
	SC_SPLASHER,
	SC_PROVIDENCE,
	SC_DEFENDER, //80
	SC_MAGICROD,
	SC_SPELLBREAKER,
	SC_AUTOSPELL,
	SC_SIGHTTRASHER,
	SC_AUTOBERSERK,
	SC_SPEARQUICKEN,
	SC_AUTOCOUNTER,
	SC_SIGHT,
	SC_SAFETYWALL,
	SC_RUWACH, //90
	SC_EXTREMITYFIST,
	SC_EXPLOSIONSPIRITS,
	SC_COMBO,
	SC_BLADESTOP_WAIT,
	SC_BLADESTOP,
	SC_FIREWEAPON,
	SC_WATERWEAPON,
	SC_WINDWEAPON,
	SC_EARTHWEAPON,
	SC_VOLCANO, //100,
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_WATK_ELEMENT,
	SC_ARMOR,
	SC_ARMOR_ELEMENT,
	SC_NOCHAT,
	SC_BABY,
	SC_AURABLADE,
	SC_PARRYING,
	SC_CONCENTRATION, //110
	SC_TENSIONRELAX,
	SC_BERSERK,
	SC_FURY,
	SC_GOSPEL,
	SC_ASSUMPTIO,
	SC_BASILICA,
	SC_GUILDAURA,
	SC_MAGICPOWER,
	SC_EDP,
	SC_TRUESIGHT, //120
	SC_WINDWALK,
	SC_MELTDOWN,
	SC_CARTBOOST,
	SC_CHASEWALK,
	SC_REJECTSWORD,
	SC_MARIONETTE,
	SC_MARIONETTE2,
	SC_CHANGEUNDEAD,
	SC_JOINTBEAT,
	SC_MINDBREAKER, //130
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	SC_DEVOTION,
	SC_SACRIFICE,
	SC_STEELBODY,
	SC_ORCISH,
	SC_READYSTORM,
	SC_READYDOWN,
	SC_READYTURN, //140
	SC_READYCOUNTER,
	SC_DODGE,
	SC_RUN,
	SC_SHADOWWEAPON,
	SC_ADRENALINE2,
	SC_GHOSTWEAPON,
	SC_KAIZEL,
	SC_KAAHI,
	SC_KAUPE,
	SC_ONEHAND, //150
	SC_PRESERVE,
	SC_BATTLEORDERS,
	SC_REGENERATION,
	SC_DOUBLECAST,
	SC_GRAVITATION,
	SC_MAXOVERTHRUST,
	SC_LONGING,
	SC_HERMODE,
	SC_SHRINK,
	SC_SIGHTBLASTER, //160
	SC_WINKCHARM,
	SC_CLOSECONFINE,
	SC_CLOSECONFINE2,
	SC_DANCING,
	SC_ELEMENTALCHANGE,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
	SC_NIBELUNGEN,
	SC_ROKISWEIL, //170
	SC_INTOABYSS,
	SC_SIEGFRIED,
	SC_WHISTLE,
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	SC_MODECHANGE,
	SC_HUMMING,
	SC_DONTFORGETME,
	SC_FORTUNE, //180
	SC_SERVICE4U,
	SC_STOP,	//Prevents inflicted chars from walking. [Skotlex]
	SC_SPURT,
	SC_SPIRIT,
	SC_COMA, //Not a real SC_, it makes a char's HP/SP hit 1.
	SC_INTRAVISION,
	SC_INCALLSTATUS,
	SC_INCSTR,
	SC_INCAGI,
	SC_INCVIT, //190
	SC_INCINT,
	SC_INCDEX,
	SC_INCLUK,
	SC_INCHIT,
	SC_INCHITRATE,
	SC_INCFLEE,
	SC_INCFLEERATE,
	SC_INCMHPRATE,
	SC_INCMSPRATE,
	SC_INCATKRATE, //200
	SC_INCMATKRATE,
	SC_INCDEFRATE,
	SC_STRFOOD,
	SC_AGIFOOD,
	SC_VITFOOD,
	SC_INTFOOD,
	SC_DEXFOOD,
	SC_LUKFOOD,
	SC_HITFOOD,
	SC_FLEEFOOD, //210
	SC_BATKFOOD,
	SC_WATKFOOD,
	SC_MATKFOOD,
	SC_SCRESIST, //Increases resistance to status changes.
	SC_XMAS, // Xmas Suit [Valaris]
	SC_WARM, //SG skills [Komurka]
	SC_SUN_COMFORT,
	SC_MOON_COMFORT,
	SC_STAR_COMFORT,
	SC_FUSION, //220
	SC_SKILLRATE_UP,
	SC_SKE,
	SC_KAITE,
	SC_SWOO, // [marquis007]
	SC_SKA, // [marquis007]
	SC_TKREST, // [marquis007]
	SC_MIRACLE, //SG 'hidden' skill [Komurka]
	SC_MADNESSCANCEL,
	SC_ADJUSTMENT,
	SC_INCREASING,  //230
	SC_GATLINGFEVER,
	SC_TATAMIGAESHI,
	SC_UTSUSEMI,
	SC_BUNSINJYUTSU,
	SC_KAENSIN,
	SC_SUITON,
	SC_NEN,
	SC_KNOWLEDGE,
	SC_SMA,
	SC_FLING,	//240
	SC_AVOID,
	SC_CHANGE,
	SC_BLOODLUST,
	SC_FLEET,
	SC_SPEED,
	SC_DEFENCE,
	SC_INCASPDRATE,
	SC_INCFLEE2,
	SC_JAILED,
	SC_ENCHANTARMS,	//250
	SC_MAGICALATTACK,
	SC_ARMORCHANGE,
	SC_CRITICALWOUND,
	SC_MAGICMIRROR,
	SC_SLOWCAST,
	SC_SUMMER,
	SC_EXPBOOST,
	SC_ITEMBOOST,
	SC_BOSSMAPINFO, 
	SC_LIFEINSURANCE, //260
	SC_INCCRI,
	SC_INCDEF,
	SC_INCBASEATK,
	SC_FASTCAST,
	SC_MDEF_RATE,
	SC_HPREGEN,
	SC_INCHEALRATE,
	SC_PNEUMA,
	SC_AUTOTRADE,
	SC_KSPROTECTED,
	SC_ARMOR_RESIST,
	SC_SPCOST_RATE,
	SC_COMMONSC_RESIST,
	SC_SEVENWIND,
	SC_DEF_RATE,
	SC_SPREGEN,
	SC_WALKSPEED,
	SC_MAX, //Automatically updated max, used in for's to check we are within bounds.
} sc_type;

//Numerates the Number for the status changes (client-dependent), imported from jA
enum si_type {
	SI_BLANK		= -1,
	SI_PROVOKE		= 0,
	SI_ENDURE		= 1,
	SI_TWOHANDQUICKEN	= 2,
	SI_CONCENTRATE		= 3,
	SI_HIDING		= 4,
	SI_CLOAKING		= 5,
	SI_ENCPOISON		= 6,
	SI_POISONREACT		= 7,
	SI_QUAGMIRE		= 8,
	SI_ANGELUS		= 9,
	SI_BLESSING		= 10,
	SI_SIGNUMCRUCIS		= 11,
	SI_INCREASEAGI		= 12,
	SI_DECREASEAGI		= 13,
	SI_SLOWPOISON		= 14,
	SI_IMPOSITIO  		= 15,
	SI_SUFFRAGIUM		= 16,
	SI_ASPERSIO		= 17,
	SI_BENEDICTIO		= 18,
	SI_KYRIE		= 19,
	SI_MAGNIFICAT		= 20,
	SI_GLORIA		= 21,
	SI_AETERNA		= 22,
	SI_ADRENALINE		= 23,
	SI_WEAPONPERFECTION	= 24,
	SI_OVERTHRUST		= 25,
	SI_MAXIMIZEPOWER	= 26,
	SI_RIDING		= 27,
	SI_FALCON		= 28,
	SI_TRICKDEAD		= 29,
	SI_LOUD			= 30,
	SI_ENERGYCOAT		= 31,
	SI_BROKENARMOR		= 32,
	SI_BROKENWEAPON		= 33,
	SI_HALLUCINATION	= 34,
	SI_WEIGHT50 		= 35,
	SI_WEIGHT90		= 36,
	SI_ASPDPOTION		= 37,
	//38: Again Aspd Potion
	//39: Again Aspd Potion
	//40: Again Aspd Potion
	SI_SPEEDPOTION1		= 41,
	SI_SPEEDPOTION2		= 42,
	SI_STRIPWEAPON		= 50,
	SI_STRIPSHIELD		= 51,
	SI_STRIPARMOR		= 52,
	SI_STRIPHELM		= 53,
	SI_CP_WEAPON		= 54,
	SI_CP_SHIELD		= 55,
	SI_CP_ARMOR		= 56,
	SI_CP_HELM		= 57,
	SI_AUTOGUARD		= 58,
	SI_REFLECTSHIELD	= 59,
	SI_PROVIDENCE		= 61,
	SI_DEFENDER		= 62,
	SI_AUTOSPELL		= 65,
	SI_SPEARQUICKEN		= 68,
	SI_EXPLOSIONSPIRITS	= 86,
	SI_STEELBODY		= 87,
	SI_FIREWEAPON		= 90,
	SI_WATERWEAPON		= 91,
	SI_WINDWEAPON		= 92,
	SI_EARTHWEAPON		= 93,
	SI_STOP			= 95,
	SI_UNDEAD		= 97,
// 102 = again gloria - from what I saw on screenshots, I wonder if it isn't gospel... [DracoRPG]
	SI_AURABLADE		= 103,
	SI_PARRYING		= 104,
	SI_CONCENTRATION	= 105,
	SI_TENSIONRELAX		= 106,
	SI_BERSERK		= 107,
	SI_ASSUMPTIO		= 110,
	SI_LANDENDOW		= 112,
	SI_MAGICPOWER		= 113,
	SI_EDP			= 114,
	SI_TRUESIGHT		= 115,
	SI_WINDWALK		= 116,
	SI_MELTDOWN		= 117,
	SI_CARTBOOST		= 118,
	//119, blank
	SI_REJECTSWORD		= 120,
	SI_MARIONETTE		= 121,
	SI_MARIONETTE2		= 122,
	SI_MOONLIT		= 123,
	SI_BLEEDING		= 124,
	SI_JOINTBEAT		= 125,
	SI_BABY			= 130,
	SI_AUTOBERSERK		= 132,
	SI_RUN			= 133,
	SI_BUMP			= 134,
	SI_READYSTORM		= 135,
	SI_READYDOWN		= 137,
	SI_READYTURN		= 139,
	SI_READYCOUNTER		= 141,
	SI_DODGE		= 143,
	//SI_RUN		= 144,  //is not RUN. need info on what this is.
	SI_SPURT		= 145,
	SI_SHADOWWEAPON		= 146,
	SI_ADRENALINE2		= 147,
	SI_GHOSTWEAPON		= 148,
	SI_SPIRIT		= 149,
	SI_DEVIL		= 152,
	SI_KAITE		= 153,
	SI_KAIZEL		= 156,
	SI_KAAHI		= 157,
	SI_KAUPE		= 158,
	SI_SMA			= 159,
	SI_NIGHT		= 160,
	SI_ONEHAND		= 161,
	SI_WARM			= 165,	
//	166 | The three show the exact same display: ultra red character (165, 166, 167)	
//	167 |	
	SI_SUN_COMFORT		= 169,
	SI_MOON_COMFORT		= 170,	
	SI_STAR_COMFORT		= 171,	
	SI_PRESERVE		= 181,
	SI_INCSTR		= 182,
	SI_INTRAVISION		= 184,
	SI_DOUBLECAST		= 186,
	SI_MAXOVERTHRUST	= 188,
	SI_TAROT		= 191, // the icon allows no doubt... but what is it really used for ?? [DracoRPG]
	SI_SHRINK		= 197,
	SI_SIGHTBLASTER		= 198,
	SI_WINKCHARM		= 199,
	SI_CLOSECONFINE		= 200,
	SI_CLOSECONFINE2	= 201,
	SI_MADNESSCANCEL	= 203,	//[blackhole89]
	SI_GATLINGFEVER		= 204,
	SI_TKREST = 205, // 205 = Gloria again (but TK- Happy State looks like it)
	SI_UTSUSEMI		= 206,
	SI_BUNSINJYUTSU		= 207,
	SI_NEN			= 208,
	SI_ADJUSTMENT		= 209,
	SI_ACCURACY		= 210,
	SI_FOODSTR		= 241,
	SI_FOODAGI		= 242,
	SI_FOODVIT		= 243,
	SI_FOODDEX		= 244,
	SI_FOODINT		= 245,
	SI_FOODLUK		= 246,
	SI_FOODFLEE		= 247,
	SI_FOODHIT		= 248,
	SI_FOODCRI		= 249,
	SI_EXPBOOST		= 250,
	SI_LIFEINSURANCE	= 251,
	SI_ITEMBOOST		= 252,
	SI_BOSSMAPINFO		= 253,
	//SI_TURTLEGENERAL	= 260, //All mobs display as Turtle General
	//SI_BIOMOBTRICKDEAD	= 263, //Bio Mob effect on you and SI_TRICKDEAD
	//SI_BLURRY		= 264, //For short time blurry screen and get Gloria icon
	//SI_FOODSTR		= 271, //Same as 241
	//SI_FOODAGI		= 272, //Same as 242
	//SI_FOODVIT		= 273, //Same as 243
	//SI_FOODDEX		= 274, //Same as 244
	//SI_FOODINT		= 275, //Same as 245
	//SI_FOODLUK		= 276, //Same as 246
	SI_SLOWCAST		= 282,
	SI_CRITICALWOUND	= 286,
	SI_DEF_RATE		= 290,
	SI_MDEF_RATE	= 291,
	SI_INCCRI		= 292,
	SI_INCHEALRATE	= 293,
	SI_HPREGEN		= 294,
	// 295 Sword ?
	SI_SPCOST_RATE	= 300,
	SI_COMMONSC_RESIST	= 301,
	SI_ARMOR_RESIST	= 302,
};

// JOINTBEAT stackable ailments
#define BREAK_ANKLE    0x01 // MoveSpeed reduced by 50%
#define BREAK_WRIST    0x02 // ASPD reduced by 25%
#define BREAK_KNEE     0x04 // MoveSpeed reduced by 30%, ASPD reduced by 10%
#define BREAK_SHOULDER 0x08 // DEF reduced by 50%
#define BREAK_WAIST    0x10 // DEF reduced by 25%, ATK reduced by 25%
#define BREAK_NECK     0x20 // current attack does 2x damage, inflicts 'bleeding' for 30 seconds
#define BREAK_FLAGS    ( BREAK_ANKLE | BREAK_WRIST | BREAK_KNEE | BREAK_SHOULDER | BREAK_WAIST | BREAK_NECK )

extern int current_equip_item_index;
extern int current_equip_card_id;

extern int percentrefinery[5][MAX_REFINE+1]; //The last slot always has a 0% success chance [Skotlex]

//Mode definitions to clear up code reading. [Skotlex]
#define MD_CANMOVE 0x0001
#define MD_LOOTER 0x0002
#define MD_AGGRESSIVE 0x0004
#define MD_ASSIST 0x0008
#define MD_CASTSENSOR_IDLE 0x0010
#define MD_BOSS 0x0020
#define MD_PLANT 0x0040
#define MD_CANATTACK 0x0080
#define MD_DETECTOR 0x0100
#define MD_CASTSENSOR_CHASE 0x0200
#define MD_CHANGECHASE 0x0400
#define MD_ANGRY 0x0800
#define MD_CHANGETARGET_MELEE 0x1000
#define MD_CHANGETARGET_CHASE 0x2000
#define MD_MASK 0xFFFF

//Status change option definitions (options are what makes status changes visible to chars
//who were not on your field of sight when it happened)
//opt1: Non stackable status changes.
enum {
	OPT1_STONE = 1, //Petrified
	OPT1_FREEZE,
	OPT1_STUN,
	OPT1_SLEEP,
	//Aegis uses OPT1 = 5 to identify undead enemies (which also grants them immunity to the other opt1 changes)
	OPT1_STONEWAIT=6 //Petrifying
};

//opt2: Stackable status changes.
#define OPT2_POISON       0x0001
#define OPT2_CURSE        0x0002
#define OPT2_SILENCE      0x0004
#define OPT2_SIGNUMCRUCIS 0x0008
#define OPT2_BLIND        0x0010
#define OPT2_ANGELUS      0x0020
#define OPT2_BLEEDING     0x0040
#define OPT2_DPOISON      0x0080

#define OPTION_SIGHT 0x00000001
#define OPTION_HIDE 0x00000002
#define OPTION_CLOAK 0x00000004
#define OPTION_CART1 0x00000008
#define OPTION_FALCON 0x00000010
#define OPTION_RIDING 0x00000020
#define OPTION_INVISIBLE 0x00000040
#define OPTION_CART2 0x00000080
#define OPTION_CART3 0x00000100
#define OPTION_CART4 0x00000200
#define OPTION_CART5 0x00000400
#define OPTION_ORCISH 0x00000800
#define OPTION_WEDDING 0x00001000
#define OPTION_RUWACH 0x00002000
#define OPTION_CHASEWALK 0x00004000
//Note that clientside Flying and Xmas are 0x8000 for clients prior to 2007.
#define OPTION_FLYING 0x0008000
#define OPTION_XMAS 0x00010000
#define OPTION_SUMMER 0x00040000

#define OPTION_CART (OPTION_CART1|OPTION_CART2|OPTION_CART3|OPTION_CART4|OPTION_CART5)

#define OPTION_MASK ~0x40

//Defines for the manner system [Skotlex]
#define MANNER_NOCHAT 0x01
#define MANNER_NOSKILL 0x02
#define MANNER_NOCOMMAND 0x04
#define MANNER_NOITEM 0x08
#define MANNER_NOROOM 0x10

//Define flags for the status_calc_bl function. [Skotlex]
enum scb_flag
{
	SCB_NONE    = 0x00000000,
	SCB_BASE    = 0x00000001,
	SCB_MAXHP   = 0x00000002,
	SCB_MAXSP   = 0x00000004,
	SCB_STR     = 0x00000008,
	SCB_AGI     = 0x00000010,
	SCB_VIT     = 0x00000020,
	SCB_INT     = 0x00000040,
	SCB_DEX     = 0x00000080,
	SCB_LUK     = 0x00000100,
	SCB_BATK    = 0x00000200,
	SCB_WATK    = 0x00000400,
	SCB_MATK    = 0x00000800,
	SCB_HIT     = 0x00001000,
	SCB_FLEE    = 0x00002000,
	SCB_DEF     = 0x00004000,
	SCB_DEF2    = 0x00008000,
	SCB_MDEF    = 0x00010000,
	SCB_MDEF2   = 0x00020000,
	SCB_SPEED   = 0x00040000,
	SCB_ASPD    = 0x00080000,
	SCB_DSPD    = 0x00100000,
	SCB_CRI     = 0x00200000,
	SCB_FLEE2   = 0x00400000,
	SCB_ATK_ELE = 0x00800000,
	SCB_DEF_ELE = 0x01000000,
	SCB_MODE    = 0x02000000,
	SCB_SIZE    = 0x04000000,
	SCB_RACE    = 0x08000000,
	SCB_RANGE   = 0x10000000,
	SCB_REGEN   = 0x20000000,
	SCB_DYE     = 0x40000000, // force cloth-dye change to 0 to avoid client crashes.
	SCB_PC      = 0x80000000,

	SCB_ALL     = 0x3FFFFFFF
};

//Define to determine who gets HP/SP consumed on doing skills/etc. [Skotlex]
#define BL_CONSUME (BL_PC|BL_HOM)
//Define to determine who has regen
#define BL_REGEN (BL_PC|BL_HOM)


//Basic damage info of a weapon
//Required because players have two of these, one in status_data
//and another for their left hand weapon.
struct weapon_atk {
	unsigned short atk, atk2;
	unsigned short range;
	unsigned char ele;
};


//For holding basic status (which can be modified by status changes)
struct status_data {
	unsigned int
		hp, sp,
		max_hp, max_sp;
	unsigned short
		str, agi, vit, int_, dex, luk,
		batk,
		matk_min, matk_max,
		speed,
		amotion, adelay, dmotion,
		mode;
	short 
		hit, flee, cri, flee2,
		def2, mdef2,
		aspd_rate;
	unsigned char
		def_ele, ele_lv,
		size, race;
	signed char
		def, mdef;
	struct weapon_atk rhw, lhw; //Right Hand/Left Hand Weapon.
};

//Additional regen data that only players have.
struct regen_data_sub {
	unsigned short
		hp,sp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp;
	} tick;
	
	//Regen rates (where every 1 means +100% regen)
	struct {
		unsigned char hp,sp;
	} rate;
};

struct regen_data {

	unsigned short flag; //Marks what stuff you may heal or not.
	unsigned short
		hp,sp,shp,ssp;

	//tick accumulation before healing.
	struct {
		unsigned int hp,sp,shp,ssp;
	} tick;
	
	//Regen rates (where every 1 means +100% regen)
	struct {
		unsigned char
		hp,sp,shp,ssp;
	} rate;
	
	struct {
		unsigned walk:1; //Can you regen even when walking?
		unsigned gc:1;	//Tags when you should have double regen due to GVG castle
		unsigned overweight :2; //overweight state (1: 50%, 2: 90%)
		unsigned block :2; //Block regen flag (1: Hp, 2: Sp)
	} state;

	//skill-regen, sitting-skill-regen (since not all chars with regen need it)
	struct regen_data_sub *sregen, *ssregen;
};

struct status_change_entry {
	int timer;
	int val1,val2,val3,val4;
};

struct status_change {
	unsigned int option;// effect state (bitfield)
	unsigned int opt3;// skill state (bitfield)
	unsigned short opt1;// body state
	unsigned short opt2;// health state (bitfield)
	unsigned char count;
	//TODO: See if it is possible to implement the following SC's without requiring extra parameters while the SC is inactive.
	unsigned char jb_flag; //Joint Beat type flag
	unsigned short mp_matk_min, mp_matk_max; //Previous matk min/max for ground spells (Amplify magic power)
	int sg_id; //ID of the previous Storm gust that hit you
	unsigned char sg_counter; //Storm gust counter (previous hits from storm gust)
	struct status_change_entry *data[SC_MAX];
};

// for looking up associated data
sc_type status_skill2sc(int skill);
int status_sc2skill(sc_type sc);

int status_damage(struct block_list *src,struct block_list *target,int hp,int sp, int walkdelay, int flag);
//Define for standard HP damage attacks.
#define status_fix_damage(src, target, hp, walkdelay) status_damage(src, target, hp, 0, walkdelay, 0)
//Define for standard HP/SP damage triggers.
#define status_zap(bl, hp, sp) status_damage(NULL, bl, hp, sp, 0, 1)
//Define for standard HP/SP skill-related cost triggers (mobs require no HP/SP to use skills)
int status_charge(struct block_list* bl, int hp, int sp);
int status_percent_change(struct block_list *src,struct block_list *target,signed char hp_rate, signed char sp_rate, int flag);
//Easier handling of status_percent_change
#define status_percent_heal(bl, hp_rate, sp_rate) status_percent_change(NULL, bl, -(hp_rate), -(sp_rate), 0)
#define status_percent_damage(src, target, hp_rate, sp_rate, kill) status_percent_change(src, target, hp_rate, sp_rate, (kill)?1:2)
//Instant kill with no drops/exp/etc
#define status_kill(bl) status_percent_damage(NULL, bl, 100, 0, true)
//Used to set the hp/sp of an object to an absolute value (can't kill)
int status_set_hp(struct block_list *bl, unsigned int hp, int flag);
int status_set_sp(struct block_list *bl, unsigned int sp, int flag);
int status_heal(struct block_list *bl,int hp,int sp, int flag);
int status_revive(struct block_list *bl, unsigned char per_hp, unsigned char per_sp);

//Define for copying a status_data structure from b to a, without overwriting current Hp and Sp
#define status_cpy(a, b) \
	memcpy(&((a)->max_hp), &((b)->max_hp), sizeof(struct status_data)-(sizeof((a)->hp)+sizeof((a)->sp)))

struct regen_data *status_get_regen_data(struct block_list *bl);
struct status_data *status_get_status_data(struct block_list *bl);
struct status_data *status_get_base_status(struct block_list *bl);
const char * status_get_name(struct block_list *bl);
int status_get_class(struct block_list *bl);
int status_get_lv(struct block_list *bl);
#define status_get_range(bl) status_get_status_data(bl)->rhw.range
#define status_get_hp(bl) status_get_status_data(bl)->hp
#define status_get_max_hp(bl) status_get_status_data(bl)->max_hp
#define status_get_sp(bl) status_get_status_data(bl)->sp
#define status_get_max_sp(bl) status_get_status_data(bl)->max_sp
#define status_get_str(bl) status_get_status_data(bl)->str
#define status_get_agi(bl) status_get_status_data(bl)->agi
#define status_get_vit(bl) status_get_status_data(bl)->vit
#define status_get_int(bl) status_get_status_data(bl)->int_
#define status_get_dex(bl) status_get_status_data(bl)->dex
#define status_get_luk(bl) status_get_status_data(bl)->luk
#define status_get_hit(bl) status_get_status_data(bl)->hit
#define status_get_flee(bl) status_get_status_data(bl)->flee
signed char status_get_def(struct block_list *bl);
#define status_get_mdef(bl) status_get_status_data(bl)->mdef
#define status_get_flee2(bl) status_get_status_data(bl)->flee2
#define status_get_def2(bl) status_get_status_data(bl)->def2
#define status_get_mdef2(bl) status_get_status_data(bl)->mdef2
#define status_get_critical(bl)  status_get_status_data(bl)->cri
#define status_get_batk(bl) status_get_status_data(bl)->batk
#define status_get_watk(bl) status_get_status_data(bl)->rhw.atk
#define status_get_watk2(bl) status_get_status_data(bl)->rhw.atk2
#define status_get_matk_max(bl) status_get_status_data(bl)->matk_max
#define status_get_matk_min(bl) status_get_status_data(bl)->matk_min
#define status_get_lwatk(bl) status_get_status_data(bl)->lhw.atk
#define status_get_lwatk2(bl) status_get_status_data(bl)->lhw.atk2
unsigned short status_get_speed(struct block_list *bl);
#define status_get_adelay(bl) status_get_status_data(bl)->adelay
#define status_get_amotion(bl) status_get_status_data(bl)->amotion
#define status_get_dmotion(bl) status_get_status_data(bl)->dmotion
#define status_get_element(bl) status_get_status_data(bl)->def_ele
#define status_get_element_level(bl) status_get_status_data(bl)->ele_lv
unsigned char status_calc_attack_element(struct block_list *bl, struct status_change *sc, int element);
#define status_get_attack_sc_element(bl, sc) status_calc_attack_element(bl, sc, 0)
#define status_get_attack_element(bl) status_get_status_data(bl)->rhw.ele
#define status_get_attack_lelement(bl) status_get_status_data(bl)->lhw.ele
#define status_get_race(bl) status_get_status_data(bl)->race
#define status_get_size(bl) status_get_status_data(bl)->size
#define status_get_mode(bl) status_get_status_data(bl)->mode
int status_get_party_id(struct block_list *bl);
int status_get_guild_id(struct block_list *bl);
int status_get_emblem_id(struct block_list *bl);
int status_get_mexp(struct block_list *bl);
int status_get_race2(struct block_list *bl);

struct view_data *status_get_viewdata(struct block_list *bl);
void status_set_viewdata(struct block_list *bl, int class_);
void status_change_init(struct block_list *bl);
struct status_change *status_get_sc(struct block_list *bl);

int status_isdead(struct block_list *bl);
int status_isimmune(struct block_list *bl);

int status_get_sc_def(struct block_list *bl, enum sc_type type, int rate, int tick, int flag);
//Short version, receives rate in 1->100 range, and does not uses a flag setting.
#define sc_start(bl, type, rate, val1, tick) status_change_start(bl,type,100*(rate),val1,0,0,0,tick,0)
#define sc_start2(bl, type, rate, val1, val2, tick) status_change_start(bl,type,100*(rate),val1,val2,0,0,tick,0)
#define sc_start4(bl, type, rate, val1, val2, val3, val4, tick) status_change_start(bl,type,100*(rate),val1,val2,val3,val4,tick,0)

int status_change_start(struct block_list* bl,enum sc_type type,int rate,int val1,int val2,int val3,int val4,int tick,int flag);
int status_change_end(struct block_list* bl, enum sc_type type, int tid);
int kaahi_heal_timer(int tid, unsigned int tick, int id, intptr data);
int status_change_timer(int tid, unsigned int tick, int id, intptr data);
int status_change_timer_sub(struct block_list* bl, va_list ap);
int status_change_clear(struct block_list* bl, int type);
int status_change_clear_buffs(struct block_list* bl, int type);

void status_calc_bl(struct block_list *bl, unsigned long flag);
int status_calc_pet(struct pet_data* pd, int first); // [Skotlex]
int status_calc_pc(struct map_session_data* sd,int first);
int status_calc_mob(struct mob_data* md, int first); //[Skotlex]
int status_calc_homunculus(struct homun_data *hd, int first);
void status_calc_misc(struct block_list *bl, struct status_data *status, int level);
void status_calc_regen(struct block_list *bl, struct status_data *status, struct regen_data *regen);
void status_calc_regen_rate(struct block_list *bl, struct regen_data *regen, struct status_change *sc);

void status_freecast_switch(struct map_session_data *sd);
int status_getrefinebonus(int lv,int type);
int status_check_skilluse(struct block_list *src, struct block_list *target, int skill_num, int flag); // [Skotlex]
int status_check_visibility(struct block_list *src, struct block_list *target); //[Skotlex]

int status_readdb(void);
int do_init_status(void);
void do_final_status(void);

#endif /* _STATUS_H_ */
