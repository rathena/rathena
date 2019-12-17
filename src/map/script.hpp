// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include "../common/cbasetypes.hpp"
#include "../common/db.hpp"
#include "../common/mmo.hpp"
#include "../common/timer.hpp"

#define NUM_WHISPER_VAR 10

///////////////////////////////////////////////////////////////////////////////
//## TODO possible enhancements: [FlavioJS]
// - 'callfunc' supporting labels in the current npc "::LabelName"
// - 'callfunc' supporting labels in other npcs "NpcName::LabelName"
// - 'function FuncName;' function declarations reverting to global functions
//   if local label isn't found
// - join callfunc and callsub's functionality
// - remove dynamic allocation in add_word()
// - remove GETVALUE / SETVALUE
// - clean up the set_reg / set_val / setd_sub mess
// - detect invalid label references at parse-time

//
// struct script_state* st;
//

/// Returns the script_data at the target index
#define script_getdata(st,i) ( &((st)->stack->stack_data[(st)->start + (i)]) )
/// Returns if the stack contains data at the target index
#define script_hasdata(st,i) ( (st)->end > (st)->start + (i) )
/// Returns the index of the last data in the stack
#define script_lastdata(st) ( (st)->end - (st)->start - 1 )
/// Pushes an int into the stack
#define script_pushint(st,val) push_val((st)->stack, C_INT, (val))
/// Pushes a string into the stack (script engine frees it automatically)
#define script_pushstr(st,val) push_str((st)->stack, C_STR, (val))
/// Pushes a copy of a string into the stack
#define script_pushstrcopy(st,val) push_str((st)->stack, C_STR, aStrdup(val))
/// Pushes a constant string into the stack (must never change or be freed)
#define script_pushconststr(st,val) push_str((st)->stack, C_CONSTSTR, const_cast<char *>(val))
/// Pushes a nil into the stack
#define script_pushnil(st) push_val((st)->stack, C_NOP, 0)
/// Pushes a copy of the data in the target index
#define script_pushcopy(st,i) push_copy((st)->stack, (st)->start + (i))

#define script_isstring(st,i) data_isstring(get_val(st, script_getdata(st,i)))
#define script_isint(st,i) data_isint(get_val(st, script_getdata(st,i)))

#define script_getnum(st,val) conv_num(st, script_getdata(st,val))
#define script_getstr(st,val) conv_str(st, script_getdata(st,val))
#define script_getref(st,val) ( script_getdata(st,val)->ref )
// Returns name of currently running function
#define script_getfuncname(st) ( st->funcname )

// Note: "top" functions/defines use indexes relative to the top of the stack
//       -1 is the index of the data at the top

/// Returns the script_data at the target index relative to the top of the stack
#define script_getdatatop(st,i) ( &((st)->stack->stack_data[(st)->stack->sp + (i)]) )
/// Pushes a copy of the data in the target index relative to the top of the stack
#define script_pushcopytop(st,i) push_copy((st)->stack, (st)->stack->sp + (i))
/// Removes the range of values [start,end[ relative to the top of the stack
#define script_removetop(st,start,end) ( pop_stack((st), ((st)->stack->sp + (start)), (st)->stack->sp + (end)) )

//
// struct script_data* data;
//

/// Returns if the script data is a string
#define data_isstring(data) ( (data)->type == C_STR || (data)->type == C_CONSTSTR )
/// Returns if the script data is an int
#define data_isint(data) ( (data)->type == C_INT )
/// Returns if the script data is a reference
#define data_isreference(data) ( (data)->type == C_NAME )
/// Returns if the script data is a label
#define data_islabel(data) ( (data)->type == C_POS )
/// Returns if the script data is an internal script function label
#define data_isfunclabel(data) ( (data)->type == C_USERFUNC_POS )

/// Returns if this is a reference to a constant
#define reference_toconstant(data) ( str_data[reference_getid(data)].type == C_INT )
/// Returns if this a reference to a param
#define reference_toparam(data) ( str_data[reference_getid(data)].type == C_PARAM )
/// Returns if this a reference to a variable
//##TODO confirm it's C_NAME [FlavioJS]
#define reference_tovariable(data) ( str_data[reference_getid(data)].type == C_NAME )
/// Returns the unique id of the reference (id and index)
#define reference_getuid(data) ( (data)->u.num )
/// Returns the id of the reference
#define reference_getid(data) ( (int32)(int64)(reference_getuid(data) & 0xffffffff) )
/// Returns the array index of the reference
#define reference_getindex(data) ( (uint32)(int64)((reference_getuid(data) >> 32) & 0xffffffff) )
/// Returns the name of the reference
#define reference_getname(data) ( str_buf + str_data[reference_getid(data)].str )
/// Returns the linked list of uid-value pairs of the reference (can be NULL)
#define reference_getref(data) ( (data)->ref )
/// Returns the value of the constant
#define reference_getconstant(data) ( str_data[reference_getid(data)].val )
/// Returns the type of param
#define reference_getparamtype(data) ( str_data[reference_getid(data)].val )

/// Composes the uid of a reference from the id and the index
#define reference_uid(id,idx) ( (int64) ((uint64)(id) & 0xFFFFFFFF) | ((uint64)(idx) << 32) )

/// Checks whether two references point to the same variable (or array)
#define is_same_reference(data1, data2) \
	(  reference_getid(data1) == reference_getid(data2) \
	&& ( (data1->ref == data2->ref && data1->ref == NULL) \
	  || (data1->ref != NULL && data2->ref != NULL && data1->ref->vars == data2->ref->vars \
	     ) ) )

#define script_getvarid(var) ( (int32)(int64)(var & 0xFFFFFFFF) )
#define script_getvaridx(var) ( (uint32)(int64)((var >> 32) & 0xFFFFFFFF) )

#define not_server_variable(prefix) ( (prefix) != '$' && (prefix) != '.' && (prefix) != '\'')
#define is_string_variable(name) ( (name)[strlen(name) - 1] == '$' )

#define FETCH(n, t) \
		if( script_hasdata(st,n) ) \
			(t)=script_getnum(st,n);

/// Maximum amount of elements in script arrays
#define SCRIPT_MAX_ARRAYSIZE (UINT_MAX - 1)

enum script_cmd_result {
	SCRIPT_CMD_SUCCESS = 0, ///when a buildin cmd was correctly done
	SCRIPT_CMD_FAILURE = 1, ///when an errors appear in cmd, show_debug will follow
};

#define SCRIPT_BLOCK_SIZE 512
enum e_labelType { LABEL_NEXTLINE = 1, LABEL_START };

struct map_session_data;
struct eri;

extern int potion_flag; //For use on Alchemist improved potions/Potion Pitcher. [Skotlex]
extern int potion_hp, potion_per_hp, potion_sp, potion_per_sp;
extern int potion_target;
extern unsigned int *generic_ui_array;
extern unsigned int generic_ui_array_size;

struct Script_Config {
	unsigned warn_func_mismatch_argtypes : 1;
	unsigned warn_func_mismatch_paramnum : 1;
	int check_cmdcount;
	int check_gotocount;
	int input_min_value;
	int input_max_value;

	// PC related
	const char *die_event_name;
	const char *kill_pc_event_name;
	const char *kill_mob_event_name;
	const char *login_event_name;
	const char *logout_event_name;
	const char *loadmap_event_name;
	const char *baselvup_event_name;
	const char *joblvup_event_name;
	const char *stat_calc_event_name;

	// NPC related
	const char* ontouch_event_name;
	const char* ontouch2_event_name;
	const char* ontouchnpc_event_name;
	const char* onwhisper_event_name;
	const char* oncommand_event_name;
	const char* onbuy_event_name;
	const char* onsell_event_name;

	// Init related
	const char* init_event_name;
	const char* inter_init_event_name;
	const char* inter_init_once_event_name;

	// Guild related
	const char* guild_break_event_name;
	const char* agit_start_event_name;
	const char* agit_init_event_name;
	const char* agit_end_event_name;
	const char* agit_start2_event_name;
	const char* agit_init2_event_name;
	const char* agit_end2_event_name;
	const char* agit_start3_event_name;
	const char* agit_init3_event_name;
	const char* agit_end3_event_name;

	// Timer related
	const char* timer_event_name;
	const char* timer_quit_event_name;
	const char* timer_minute_event_name;
	const char* timer_hour_event_name;
	const char* timer_clock_event_name;
	const char* timer_day_event_name;
	const char* timer_sunday_event_name;
	const char* timer_monday_event_name;
	const char* timer_tuesday_event_name;
	const char* timer_wednesday_event_name;
	const char* timer_thursday_event_name;
	const char* timer_friday_event_name;
	const char* timer_saturday_event_name;

	// Instance related
	const char* instance_init_event_name;
	const char* instance_destroy_event_name;
};
extern struct Script_Config script_config;

typedef enum c_op {
	C_NOP, // end of script/no value (nil)
	C_POS,
	C_INT, // number
	C_PARAM, // parameter variable (see pc_readparam/pc_setparam)
	C_FUNC, // buildin function call
	C_STR, // string (free'd automatically)
	C_CONSTSTR, // string (not free'd)
	C_ARG, // start of argument list
	C_NAME,
	C_EOL, // end of line (extra stack values are cleared)
	C_RETINFO,
	C_USERFUNC, // internal script function
	C_USERFUNC_POS, // internal script function label
	C_REF, // the next call to c_op2 should push back a ref to the left operand

	// operators
	C_OP3, // a ? b : c
	C_LOR, // a || b
	C_LAND, // a && b
	C_LE, // a <= b
	C_LT, // a < b
	C_GE, // a >= b
	C_GT, // a > b
	C_EQ, // a == b
	C_NE, // a != b
	C_XOR, // a ^ b
	C_OR, // a | b
	C_AND, // a & b
	C_ADD, // a + b
	C_SUB, // a - b
	C_MUL, // a * b
	C_DIV, // a / b
	C_MOD, // a % b
	C_NEG, // - a
	C_LNOT, // ! a
	C_NOT, // ~ a
	C_R_SHIFT, // a >> b
	C_L_SHIFT, // a << b
	C_ADD_POST, // a++
	C_SUB_POST, // a--
	C_ADD_PRE, // ++a
	C_SUB_PRE, // --a
} c_op;

/**
 * Generic reg database abstraction to be used with various types of regs/script variables.
 */
struct reg_db {
	struct DBMap *vars;
	struct DBMap *arrays;
};

struct script_retinfo {
	struct reg_db scope;        ///< scope variables
	struct script_code* script; ///< script code
	int pos;                    ///< script location
	int nargs;                  ///< argument count
	int defsp;                  ///< default stack pointer
};

struct script_data {
	enum c_op type;
	union script_data_val {
		int64 num;
		char *str;
		struct script_retinfo* ri;
	} u;
	struct reg_db *ref;
};

// Moved defsp from script_state to script_stack since
// it must be saved when script state is RERUNLINE. [Eoe / jA 1094]
struct script_code {
	int script_size;
	unsigned char* script_buf;
	struct reg_db local;
	unsigned short instances;
};

struct script_stack {
	int sp;                         ///< number of entries in the stack
	int sp_max;                     ///< capacity of the stack
	int defsp;
	struct script_data *stack_data; ///< stack
	struct reg_db scope;            ///< scope variables
};


//
// Script state
//
enum e_script_state { RUN,STOP,END,RERUNLINE,GOTO,RETFUNC,CLOSE };

struct script_state {
	struct script_stack* stack;
	int start,end;
	int pos;
	enum e_script_state state;
	int rid,oid;
	struct script_code *script;
	struct sleep_data {
		int tick,timer,charid;
	} sleep;
	//For backing up purposes
	struct script_state *bk_st;
	int bk_npcid;
	unsigned freeloop : 1;// used by buildin_freeloop
	unsigned op2ref : 1;// used by op_2
	unsigned npc_item_flag : 1;
	unsigned mes_active : 1;  // Store if invoking character has a NPC dialog box open.
	char* funcname; // Stores the current running function name
	unsigned int id;
};

struct script_reg {
	int64 index;
	int data;
};

struct script_regstr {
	int64 index;
	char* data;
};

struct script_array {
	unsigned int id;       ///< the first 32b of the 64b uid, aka the id
	unsigned int size;     ///< how many members
	unsigned int *members; ///< member list
};

enum script_parse_options {
	SCRIPT_USE_LABEL_DB = 0x1,// records labels in scriptlabel_db
	SCRIPT_IGNORE_EXTERNAL_BRACKETS = 0x2,// ignores the check for {} brackets around the script
	SCRIPT_RETURN_EMPTY_SCRIPT = 0x4// returns the script object instead of NULL for empty scripts
};

enum monsterinfo_types {
	MOB_NAME = 0,
	MOB_LV,
	MOB_MAXHP,
	MOB_BASEEXP,
	MOB_JOBEXP,
	MOB_ATK1,
	MOB_ATK2,
	MOB_DEF,
	MOB_MDEF,
	MOB_STR,
	MOB_AGI,
	MOB_VIT,
	MOB_INT,
	MOB_DEX,
	MOB_LUK,
	MOB_RANGE,
	MOB_RANGE2,
	MOB_RANGE3,
	MOB_SIZE,
	MOB_RACE,
	MOB_ELEMENT,
	MOB_MODE,
	MOB_MVPEXP
};

enum petinfo_types {
	PETINFO_ID = 0,
	PETINFO_CLASS,
	PETINFO_NAME,
	PETINFO_INTIMATE,
	PETINFO_HUNGRY,
	PETINFO_RENAMED,
	PETINFO_LEVEL,
	PETINFO_BLOCKID,
	PETINFO_EGGID,
	PETINFO_FOODID
};

enum e_questinfo_types {
	QTYPE_QUEST = 0,
	QTYPE_QUEST2,
	QTYPE_JOB,
	QTYPE_JOB2,
	QTYPE_EVENT,
	QTYPE_EVENT2,
	QTYPE_WARG,
	QTYPE_CLICKME = QTYPE_WARG,
	QTYPE_DAILYQUEST,
	QTYPE_WARG2,
	QTYPE_EVENT3 = QTYPE_WARG2,
	QTYPE_JOBQUEST,
	QTYPE_JUMPING_PORING,
	// 11 - 9998 = free
	QTYPE_NONE = 9999
};

enum e_questinfo_markcolor : uint8 {
	QMARK_NONE = 0,
	QMARK_YELLOW,
	QMARK_GREEN,
	QMARK_PURPLE,
	QMARK_MAX
};

#ifndef WIN32
	// These are declared in wingdi.h
	/* Font Weights */
	#define FW_DONTCARE         0
	#define FW_THIN             100
	#define FW_EXTRALIGHT       200
	#define FW_LIGHT            300
	#define FW_NORMAL           400
	#define FW_MEDIUM           500
	#define FW_SEMIBOLD         600
	#define FW_BOLD             700
	#define FW_EXTRABOLD        800
	#define FW_HEAVY            900
#endif

enum unitdata_mobtypes {
	UMOB_SIZE = 0,
	UMOB_LEVEL,
	UMOB_HP,
	UMOB_MAXHP,
	UMOB_MASTERAID,
	UMOB_MAPID,
	UMOB_X,
	UMOB_Y,
	UMOB_SPEED,
	UMOB_MODE,
	UMOB_AI,
	UMOB_SCOPTION,
	UMOB_SEX,
	UMOB_CLASS,
	UMOB_HAIRSTYLE,
	UMOB_HAIRCOLOR,
	UMOB_HEADBOTTOM,
	UMOB_HEADMIDDLE,
	UMOB_HEADTOP,
	UMOB_CLOTHCOLOR,
	UMOB_SHIELD,
	UMOB_WEAPON,
	UMOB_LOOKDIR,
	UMOB_CANMOVETICK,
	UMOB_STR,
	UMOB_AGI,
	UMOB_VIT,
	UMOB_INT,
	UMOB_DEX,
	UMOB_LUK,
	UMOB_SLAVECPYMSTRMD,
	UMOB_DMGIMMUNE,
	UMOB_ATKRANGE,
	UMOB_ATKMIN,
	UMOB_ATKMAX,
	UMOB_MATKMIN,
	UMOB_MATKMAX,
	UMOB_DEF,
	UMOB_MDEF,
	UMOB_HIT,
	UMOB_FLEE,
	UMOB_PDODGE,
	UMOB_CRIT,
	UMOB_RACE,
	UMOB_ELETYPE,
	UMOB_ELELEVEL,
	UMOB_AMOTION,
	UMOB_ADELAY,
	UMOB_DMOTION,
	UMOB_TARGETID,
};

enum unitdata_homuntypes {
	UHOM_SIZE = 0,
	UHOM_LEVEL,
	UHOM_HP,
	UHOM_MAXHP,
	UHOM_SP,
	UHOM_MAXSP,
	UHOM_MASTERCID,
	UHOM_MAPID,
	UHOM_X,
	UHOM_Y,
	UHOM_HUNGER,
	UHOM_INTIMACY,
	UHOM_SPEED,
	UHOM_LOOKDIR,
	UHOM_CANMOVETICK,
	UHOM_STR,
	UHOM_AGI,
	UHOM_VIT,
	UHOM_INT,
	UHOM_DEX,
	UHOM_LUK,
	UHOM_DMGIMMUNE,
	UHOM_ATKRANGE,
	UHOM_ATKMIN,
	UHOM_ATKMAX,
	UHOM_MATKMIN,
	UHOM_MATKMAX,
	UHOM_DEF,
	UHOM_MDEF,
	UHOM_HIT,
	UHOM_FLEE,
	UHOM_PDODGE,
	UHOM_CRIT,
	UHOM_RACE,
	UHOM_ELETYPE,
	UHOM_ELELEVEL,
	UHOM_AMOTION,
	UHOM_ADELAY,
	UHOM_DMOTION,
	UHOM_TARGETID,
};

enum unitdata_pettypes {
	UPET_SIZE = 0,
	UPET_LEVEL,
	UPET_HP,
	UPET_MAXHP,
	UPET_MASTERAID,
	UPET_MAPID,
	UPET_X,
	UPET_Y,
	UPET_HUNGER,
	UPET_INTIMACY,
	UPET_SPEED,
	UPET_LOOKDIR,
	UPET_CANMOVETICK,
	UPET_STR,
	UPET_AGI,
	UPET_VIT,
	UPET_INT,
	UPET_DEX,
	UPET_LUK,
	UPET_DMGIMMUNE,
	UPET_ATKRANGE,
	UPET_ATKMIN,
	UPET_ATKMAX,
	UPET_MATKMIN,
	UPET_MATKMAX,
	UPET_DEF,
	UPET_MDEF,
	UPET_HIT,
	UPET_FLEE,
	UPET_PDODGE,
	UPET_CRIT,
	UPET_RACE,
	UPET_ELETYPE,
	UPET_ELELEVEL,
	UPET_AMOTION,
	UPET_ADELAY,
	UPET_DMOTION,
};

enum unitdata_merctypes {
	UMER_SIZE = 0,
	UMER_HP,
	UMER_MAXHP,
	UMER_MASTERCID,
	UMER_MAPID,
	UMER_X,
	UMER_Y,
	UMER_KILLCOUNT,
	UMER_LIFETIME,
	UMER_SPEED,
	UMER_LOOKDIR,
	UMER_CANMOVETICK,
	UMER_STR,
	UMER_AGI,
	UMER_VIT,
	UMER_INT,
	UMER_DEX,
	UMER_LUK,
	UMER_DMGIMMUNE,
	UMER_ATKRANGE,
	UMER_ATKMIN,
	UMER_ATKMAX,
	UMER_MATKMIN,
	UMER_MATKMAX,
	UMER_DEF,
	UMER_MDEF,
	UMER_HIT,
	UMER_FLEE,
	UMER_PDODGE,
	UMER_CRIT,
	UMER_RACE,
	UMER_ELETYPE,
	UMER_ELELEVEL,
	UMER_AMOTION,
	UMER_ADELAY,
	UMER_DMOTION,
	UMER_TARGETID,
};

enum unitdata_elemtypes {
	UELE_SIZE = 0,
	UELE_HP,
	UELE_MAXHP,
	UELE_SP,
	UELE_MAXSP,
	UELE_MASTERCID,
	UELE_MAPID,
	UELE_X,
	UELE_Y,
	UELE_LIFETIME,
	UELE_MODE,
	UELE_SPEED,
	UELE_LOOKDIR,
	UELE_CANMOVETICK,
	UELE_STR,
	UELE_AGI,
	UELE_VIT,
	UELE_INT,
	UELE_DEX,
	UELE_LUK,
	UELE_DMGIMMUNE,
	UELE_ATKRANGE,
	UELE_ATKMIN,
	UELE_ATKMAX,
	UELE_MATKMIN,
	UELE_MATKMAX,
	UELE_DEF,
	UELE_MDEF,
	UELE_HIT,
	UELE_FLEE,
	UELE_PDODGE,
	UELE_CRIT,
	UELE_RACE,
	UELE_ELETYPE,
	UELE_ELELEVEL,
	UELE_AMOTION,
	UELE_ADELAY,
	UELE_DMOTION,
	UELE_TARGETID,
};

enum unitdata_npctypes {
	UNPC_DISPLAY = 0,
	UNPC_LEVEL,
	UNPC_HP,
	UNPC_MAXHP,
	UNPC_MAPID,
	UNPC_X,
	UNPC_Y,
	UNPC_LOOKDIR,
	UNPC_STR,
	UNPC_AGI,
	UNPC_VIT,
	UNPC_INT,
	UNPC_DEX,
	UNPC_LUK,
	UNPC_PLUSALLSTAT,
	UNPC_DMGIMMUNE,
	UNPC_ATKRANGE,
	UNPC_ATKMIN,
	UNPC_ATKMAX,
	UNPC_MATKMIN,
	UNPC_MATKMAX,
	UNPC_DEF,
	UNPC_MDEF,
	UNPC_HIT,
	UNPC_FLEE,
	UNPC_PDODGE,
	UNPC_CRIT,
	UNPC_RACE,
	UNPC_ELETYPE,
	UNPC_ELELEVEL,
	UNPC_AMOTION,
	UNPC_ADELAY,
	UNPC_DMOTION,
};

enum navigation_service {
	NAV_NONE = 0, ///< 0
	NAV_AIRSHIP_ONLY = 1, ///< 1 (actually 1-9)
	NAV_SCROLL_ONLY = 10, ///< 10
	NAV_AIRSHIP_AND_SCROLL = NAV_AIRSHIP_ONLY + NAV_SCROLL_ONLY, ///< 11 (actually 11-99)
	NAV_KAFRA_ONLY = 100, ///< 100
	NAV_KAFRA_AND_AIRSHIP = NAV_KAFRA_ONLY + NAV_AIRSHIP_ONLY, ///< 101 (actually 101-109)
	NAV_KAFRA_AND_SCROLL = NAV_KAFRA_ONLY + NAV_SCROLL_ONLY, ///< 110
	NAV_ALL = NAV_AIRSHIP_ONLY + NAV_SCROLL_ONLY + NAV_KAFRA_ONLY ///< 111 (actually 111-255)
};

enum random_option_attribute {
	ROA_ID = 0,
	ROA_VALUE,
	ROA_PARAM,
};

enum instance_info_type {
	IIT_ID,
	IIT_TIME_LIMIT,
	IIT_IDLE_TIMEOUT,
	IIT_ENTER_MAP,
	IIT_ENTER_X,
	IIT_ENTER_Y,
	IIT_MAPCOUNT,
	IIT_MAP
};

enum e_instance_live_info_type : uint8 {
	ILI_NAME,
	ILI_MODE,
	ILI_OWNER
};

enum vip_status_type {
	VIP_STATUS_ACTIVE = 1,
	VIP_STATUS_EXPIRE,
	VIP_STATUS_REMAINING
};

enum e_special_effects {
	EF_NONE = -1,
	EF_HIT1,
	EF_HIT2,
	EF_HIT3,
	EF_HIT4,
	EF_HIT5,
	EF_HIT6,
	EF_ENTRY,
	EF_EXIT,
	EF_WARP,
	EF_ENHANCE,
	EF_COIN,
	EF_ENDURE,
	EF_BEGINSPELL,
	EF_GLASSWALL,
	EF_HEALSP,
	EF_SOULSTRIKE,
	EF_BASH,
	EF_MAGNUMBREAK,
	EF_STEAL,
	EF_HIDING,
	EF_PATTACK,
	EF_DETOXICATION,
	EF_SIGHT,
	EF_STONECURSE,
	EF_FIREBALL,
	EF_FIREWALL,
	EF_ICEARROW,
	EF_FROSTDIVER,
	EF_FROSTDIVER2,
	EF_LIGHTBOLT,
	EF_THUNDERSTORM,
	EF_FIREARROW,
	EF_NAPALMBEAT,
	EF_RUWACH,
	EF_TELEPORTATION,
	EF_READYPORTAL,
	EF_PORTAL,
	EF_INCAGILITY,
	EF_DECAGILITY,
	EF_AQUA,
	EF_SIGNUM,
	EF_ANGELUS,
	EF_BLESSING,
	EF_INCAGIDEX,
	EF_SMOKE,
	EF_FIREFLY,
	EF_SANDWIND,
	EF_TORCH,
	EF_SPRAYPOND,
	EF_FIREHIT,
	EF_FIRESPLASHHIT,
	EF_COLDHIT,
	EF_WINDHIT,
	EF_POISONHIT,
	EF_BEGINSPELL2,
	EF_BEGINSPELL3,
	EF_BEGINSPELL4,
	EF_BEGINSPELL5,
	EF_BEGINSPELL6,
	EF_BEGINSPELL7,
	EF_LOCKON,
	EF_WARPZONE,
	EF_SIGHTRASHER,
	EF_BARRIER,
	EF_ARROWSHOT,
	EF_INVENOM,
	EF_CURE,
	EF_PROVOKE,
	EF_MVP,
	EF_SKIDTRAP,
	EF_BRANDISHSPEAR,
	EF_CONE,
	EF_SPHERE,
	EF_BOWLINGBASH,
	EF_ICEWALL,
	EF_GLORIA,
	EF_MAGNIFICAT,
	EF_RESURRECTION,
	EF_RECOVERY,
	EF_EARTHSPIKE,
	EF_SPEARBMR,
	EF_PIERCE,
	EF_TURNUNDEAD,
	EF_SANCTUARY,
	EF_IMPOSITIO,
	EF_LEXAETERNA,
	EF_ASPERSIO,
	EF_LEXDIVINA,
	EF_SUFFRAGIUM,
	EF_STORMGUST,
	EF_LORD,
	EF_BENEDICTIO,
	EF_METEORSTORM,
	EF_YUFITEL,
	EF_YUFITELHIT,
	EF_QUAGMIRE,
	EF_FIREPILLAR,
	EF_FIREPILLARBOMB,
	EF_HASTEUP,
	EF_FLASHER,
	EF_REMOVETRAP,
	EF_REPAIRWEAPON,
	EF_CRASHEARTH,
	EF_PERFECTION,
	EF_MAXPOWER,
	EF_BLASTMINE,
	EF_BLASTMINEBOMB,
	EF_CLAYMORE,
	EF_FREEZING,
	EF_BUBBLE,
	EF_GASPUSH,
	EF_SPRINGTRAP,
	EF_KYRIE,
	EF_MAGNUS,
	EF_BOTTOM,
	EF_BLITZBEAT,
	EF_WATERBALL,
	EF_WATERBALL2,
	EF_FIREIVY,
	EF_DETECTING,
	EF_CLOAKING,
	EF_SONICBLOW,
	EF_SONICBLOWHIT,
	EF_GRIMTOOTH,
	EF_VENOMDUST,
	EF_ENCHANTPOISON,
	EF_POISONREACT,
	EF_POISONREACT2,
	EF_OVERTHRUST,
	EF_SPLASHER,
	EF_TWOHANDQUICKEN,
	EF_AUTOCOUNTER,
	EF_GRIMTOOTHATK,
	EF_FREEZE,
	EF_FREEZED,
	EF_ICECRASH,
	EF_SLOWPOISON,
	EF_BOTTOM2,
	EF_FIREPILLARON,
	EF_SANDMAN,
	EF_REVIVE,
	EF_PNEUMA,
	EF_HEAVENSDRIVE,
	EF_SONICBLOW2,
	EF_BRANDISH2,
	EF_SHOCKWAVE,
	EF_SHOCKWAVEHIT,
	EF_EARTHHIT,
	EF_PIERCESELF,
	EF_BOWLINGSELF,
	EF_SPEARSTABSELF,
	EF_SPEARBMRSELF,
	EF_HOLYHIT,
	EF_CONCENTRATION,
	EF_REFINEOK,
	EF_REFINEFAIL,
	EF_JOBCHANGE,
	EF_LVUP,
	EF_JOBLVUP,
	EF_TOPRANK,
	EF_PARTY,
	EF_RAIN,
	EF_SNOW,
	EF_SAKURA,
	EF_STATUS_STATE,
	EF_BANJJAKII,
	EF_MAKEBLUR,
	EF_TAMINGSUCCESS,
	EF_TAMINGFAILED,
	EF_ENERGYCOAT,
	EF_CARTREVOLUTION,
	EF_VENOMDUST2,
	EF_CHANGEDARK,
	EF_CHANGEFIRE,
	EF_CHANGECOLD,
	EF_CHANGEWIND,
	EF_CHANGEFLAME,
	EF_CHANGEEARTH,
	EF_CHAINGEHOLY,
	EF_CHANGEPOISON,
	EF_HITDARK,
	EF_MENTALBREAK,
	EF_MAGICALATTHIT,
	EF_SUI_EXPLOSION,
	EF_DARKATTACK,
	EF_SUICIDE,
	EF_COMBOATTACK1,
	EF_COMBOATTACK2,
	EF_COMBOATTACK3,
	EF_COMBOATTACK4,
	EF_COMBOATTACK5,
	EF_GUIDEDATTACK,
	EF_POISONATTACK,
	EF_SILENCEATTACK,
	EF_STUNATTACK,
	EF_PETRIFYATTACK,
	EF_CURSEATTACK,
	EF_SLEEPATTACK,
	EF_TELEKHIT,
	EF_PONG,
	EF_LEVEL99,
	EF_LEVEL99_2,
	EF_LEVEL99_3,
	EF_GUMGANG,
	EF_POTION1,
	EF_POTION2,
	EF_POTION3,
	EF_POTION4,
	EF_POTION5,
	EF_POTION6,
	EF_POTION7,
	EF_POTION8,
	EF_DARKBREATH,
	EF_DEFFENDER,
	EF_KEEPING,
	EF_SUMMONSLAVE,
	EF_BLOODDRAIN,
	EF_ENERGYDRAIN,
	EF_POTION_CON,
	EF_POTION_,
	EF_POTION_BERSERK,
	EF_POTIONPILLAR,
	EF_DEFENDER,
	EF_GANBANTEIN,
	EF_WIND,
	EF_VOLCANO,
	EF_GRANDCROSS,
	EF_INTIMIDATE,
	EF_CHOOKGI,
	EF_CLOUD,
	EF_CLOUD2,
	EF_MAPPILLAR,
	EF_LINELINK,
	EF_CLOUD3,
	EF_SPELLBREAKER,
	EF_DISPELL,
	EF_DELUGE,
	EF_VIOLENTGALE,
	EF_LANDPROTECTOR,
	EF_BOTTOM_VO,
	EF_BOTTOM_DE,
	EF_BOTTOM_VI,
	EF_BOTTOM_LA,
	EF_FASTMOVE,
	EF_MAGICROD,
	EF_HOLYCROSS,
	EF_SHIELDCHARGE,
	EF_MAPPILLAR2,
	EF_PROVIDENCE,
	EF_SHIELDBOOMERANG,
	EF_SPEARQUICKEN,
	EF_DEVOTION,
	EF_REFLECTSHIELD,
	EF_ABSORBSPIRITS,
	EF_STEELBODY,
	EF_FLAMELAUNCHER,
	EF_FROSTWEAPON,
	EF_LIGHTNINGLOADER,
	EF_SEISMICWEAPON,
	EF_MAPPILLAR3,
	EF_MAPPILLAR4,
	EF_GUMGANG2,
	EF_TEIHIT1,
	EF_GUMGANG3,
	EF_TEIHIT2,
	EF_TANJI,
	EF_TEIHIT1X,
	EF_CHIMTO,
	EF_STEALCOIN,
	EF_STRIPWEAPON,
	EF_STRIPSHIELD,
	EF_STRIPARMOR,
	EF_STRIPHELM,
	EF_CHAINCOMBO,
	EF_RG_COIN,
	EF_BACKSTAP,
	EF_TEIHIT3,
	EF_BOTTOM_DISSONANCE,
	EF_BOTTOM_LULLABY,
	EF_BOTTOM_RICHMANKIM,
	EF_BOTTOM_ETERNALCHAOS,
	EF_BOTTOM_DRUMBATTLEFIELD,
	EF_BOTTOM_RINGNIBELUNGEN,
	EF_BOTTOM_ROKISWEIL,
	EF_BOTTOM_INTOABYSS,
	EF_BOTTOM_SIEGFRIED,
	EF_BOTTOM_WHISTLE,
	EF_BOTTOM_ASSASSINCROSS,
	EF_BOTTOM_POEMBRAGI,
	EF_BOTTOM_APPLEIDUN,
	EF_BOTTOM_UGLYDANCE,
	EF_BOTTOM_HUMMING,
	EF_BOTTOM_DONTFORGETME,
	EF_BOTTOM_FORTUNEKISS,
	EF_BOTTOM_SERVICEFORYOU,
	EF_TALK_FROSTJOKE,
	EF_TALK_SCREAM,
	EF_POKJUK,
	EF_THROWITEM,
	EF_THROWITEM2,
	EF_CHEMICALPROTECTION,
	EF_POKJUK_SOUND,
	EF_DEMONSTRATION,
	EF_CHEMICAL2,
	EF_TELEPORTATION2,
	EF_PHARMACY_OK,
	EF_PHARMACY_FAIL,
	EF_FORESTLIGHT,
	EF_THROWITEM3,
	EF_FIRSTAID,
	EF_SPRINKLESAND,
	EF_LOUD,
	EF_HEAL,
	EF_HEAL2,
	EF_EXIT2,
	EF_GLASSWALL2,
	EF_READYPORTAL2,
	EF_PORTAL2,
	EF_BOTTOM_MAG,
	EF_BOTTOM_SANC,
	EF_HEAL3,
	EF_WARPZONE2,
	EF_FORESTLIGHT2,
	EF_FORESTLIGHT3,
	EF_FORESTLIGHT4,
	EF_HEAL4,
	EF_FOOT,
	EF_FOOT2,
	EF_BEGINASURA,
	EF_TRIPLEATTACK,
	EF_HITLINE,
	EF_HPTIME,
	EF_SPTIME,
	EF_MAPLE,
	EF_BLIND,
	EF_POISON,
	EF_GUARD,
	EF_JOBLVUP50,
	EF_ANGEL2,
	EF_MAGNUM2,
	EF_CALLZONE,
	EF_PORTAL3,
	EF_COUPLECASTING,
	EF_HEARTCASTING,
	EF_ENTRY2,
	EF_SAINTWING,
	EF_SPHEREWIND,
	EF_COLORPAPER,
	EF_LIGHTSPHERE,
	EF_WATERFALL,
	EF_WATERFALL_90,
	EF_WATERFALL_SMALL,
	EF_WATERFALL_SMALL_90,
	EF_WATERFALL_T2,
	EF_WATERFALL_T2_90,
	EF_WATERFALL_SMALL_T2,
	EF_WATERFALL_SMALL_T2_90,
	EF_MINI_TETRIS,
	EF_GHOST,
	EF_BAT,
	EF_BAT2,
	EF_SOULBREAKER,
	EF_LEVEL99_4,
	EF_VALLENTINE,
	EF_VALLENTINE2,
	EF_PRESSURE,
	EF_BASH3D,
	EF_AURABLADE,
	EF_REDBODY,
	EF_LKCONCENTRATION,
	EF_BOTTOM_GOSPEL,
	EF_ANGEL,
	EF_DEVIL,
	EF_DRAGONSMOKE,
	EF_BOTTOM_BASILICA,
	EF_ASSUMPTIO,
	EF_HITLINE2,
	EF_BASH3D2,
	EF_ENERGYDRAIN2,
	EF_TRANSBLUEBODY,
	EF_MAGICCRASHER,
	EF_LIGHTSPHERE2,
	EF_LIGHTBLADE,
	EF_ENERGYDRAIN3,
	EF_LINELINK2,
	EF_LINKLIGHT,
	EF_TRUESIGHT,
	EF_FALCONASSAULT,
	EF_TRIPLEATTACK2,
	EF_PORTAL4,
	EF_MELTDOWN,
	EF_CARTBOOST,
	EF_REJECTSWORD,
	EF_TRIPLEATTACK3,
	EF_SPHEREWIND2,
	EF_LINELINK3,
	EF_PINKBODY,
	EF_LEVEL99_5,
	EF_LEVEL99_6,
	EF_BASH3D3,
	EF_BASH3D4,
	EF_NAPALMVALCAN,
	EF_PORTAL5,
	EF_MAGICCRASHER2,
	EF_BOTTOM_SPIDER,
	EF_BOTTOM_FOGWALL,
	EF_SOULBURN,
	EF_SOULCHANGE,
	EF_BABY,
	EF_SOULBREAKER2,
	EF_RAINBOW,
	EF_PEONG,
	EF_TANJI2,
	EF_PRESSEDBODY,
	EF_SPINEDBODY,
	EF_KICKEDBODY,
	EF_AIRTEXTURE,
	EF_HITBODY,
	EF_DOUBLEGUMGANG,
	EF_REFLECTBODY,
	EF_BABYBODY,
	EF_BABYBODY2,
	EF_GIANTBODY,
	EF_GIANTBODY2,
	EF_ASURABODY,
	EF_4WAYBODY,
	EF_QUAKEBODY,
	EF_ASURABODY_MONSTER,
	EF_HITLINE3,
	EF_HITLINE4,
	EF_HITLINE5,
	EF_HITLINE6,
	EF_ELECTRIC,
	EF_ELECTRIC2,
	EF_HITLINE7,
	EF_STORMKICK,
	EF_HALFSPHERE,
	EF_ATTACKENERGY,
	EF_ATTACKENERGY2,
	EF_CHEMICAL3,
	EF_ASSUMPTIO2,
	EF_BLUECASTING,
	EF_RUN,
	EF_STOPRUN,
	EF_STOPEFFECT,
	EF_JUMPBODY,
	EF_LANDBODY,
	EF_FOOT3,
	EF_FOOT4,
	EF_TAE_READY,
	EF_GRANDCROSS2,
	EF_SOULSTRIKE2,
	EF_YUFITEL2,
	EF_NPC_STOP,
	EF_DARKCASTING,
	EF_GUMGANGNPC,
	EF_AGIUP,
	EF_JUMPKICK,
	EF_QUAKEBODY2,
	EF_STORMKICK1,
	EF_STORMKICK2,
	EF_STORMKICK3,
	EF_STORMKICK4,
	EF_STORMKICK5,
	EF_STORMKICK6,
	EF_STORMKICK7,
	EF_SPINEDBODY2,
	EF_BEGINASURA1,
	EF_BEGINASURA2,
	EF_BEGINASURA3,
	EF_BEGINASURA4,
	EF_BEGINASURA5,
	EF_BEGINASURA6,
	EF_BEGINASURA7,
	EF_AURABLADE2,
	EF_DEVIL1,
	EF_DEVIL2,
	EF_DEVIL3,
	EF_DEVIL4,
	EF_DEVIL5,
	EF_DEVIL6,
	EF_DEVIL7,
	EF_DEVIL8,
	EF_DEVIL9,
	EF_DEVIL10,
	EF_DOUBLEGUMGANG2,
	EF_DOUBLEGUMGANG3,
	EF_BLACKDEVIL,
	EF_FLOWERCAST,
	EF_FLOWERCAST2,
	EF_FLOWERCAST3,
	EF_MOCHI,
	EF_LAMADAN,
	EF_EDP,
	EF_SHIELDBOOMERANG2,
	EF_RG_COIN2,
	EF_GUARD2,
	EF_SLIM,
	EF_SLIM2,
	EF_SLIM3,
	EF_CHEMICALBODY,
	EF_CASTSPIN,
	EF_PIERCEBODY,
	EF_SOULLINK,
	EF_CHOOKGI2,
	EF_MEMORIZE,
	EF_SOULLIGHT,
	EF_MAPAE,
	EF_ITEMPOKJUK,
	EF_05VAL,
	EF_BEGINASURA11,
	EF_NIGHT,
	EF_CHEMICAL2DASH,
	EF_GROUNDSAMPLE,
	EF_GI_EXPLOSION,
	EF_CLOUD4,
	EF_CLOUD5,
	EF_BOTTOM_HERMODE,
	EF_CARTTER,
	EF_ITEMFAST,
	EF_SHIELDBOOMERANG3,
	EF_DOUBLECASTBODY,
	EF_GRAVITATION,
	EF_TAROTCARD1,
	EF_TAROTCARD2,
	EF_TAROTCARD3,
	EF_TAROTCARD4,
	EF_TAROTCARD5,
	EF_TAROTCARD6,
	EF_TAROTCARD7,
	EF_TAROTCARD8,
	EF_TAROTCARD9,
	EF_TAROTCARD10,
	EF_TAROTCARD11,
	EF_TAROTCARD12,
	EF_TAROTCARD13,
	EF_TAROTCARD14,
	EF_ACIDDEMON,
	EF_GREENBODY,
	EF_THROWITEM4,
	EF_BABYBODY_BACK,
	EF_THROWITEM5,
	EF_BLUEBODY,
	EF_HATED,
	EF_REDLIGHTBODY,
	EF_RO2YEAR,
	EF_SMA_READY,
	EF_STIN,
	EF_RED_HIT,
	EF_BLUE_HIT,
	EF_QUAKEBODY3,
	EF_SMA,
	EF_SMA2,
	EF_STIN2,
	EF_HITTEXTURE,
	EF_STIN3,
	EF_SMA3,
	EF_BLUEFALL,
	EF_BLUEFALL_90,
	EF_FASTBLUEFALL,
	EF_FASTBLUEFALL_90,
	EF_BIG_PORTAL,
	EF_BIG_PORTAL2,
	EF_SCREEN_QUAKE,
	EF_HOMUNCASTING,
	EF_HFLIMOON1,
	EF_HFLIMOON2,
	EF_HFLIMOON3,
	EF_HO_UP,
	EF_HAMIDEFENCE,
	EF_HAMICASTLE,
	EF_HAMIBLOOD,
	EF_HATED2,
	EF_TWILIGHT1,
	EF_TWILIGHT2,
	EF_TWILIGHT3,
	EF_ITEM_THUNDER,
	EF_ITEM_CLOUD,
	EF_ITEM_CURSE,
	EF_ITEM_ZZZ,
	EF_ITEM_RAIN,
	EF_ITEM_LIGHT,
	EF_ANGEL3,
	EF_M01,
	EF_M02,
	EF_M03,
	EF_M04,
	EF_M05,
	EF_M06,
	EF_M07,
	EF_KAIZEL,
	EF_KAAHI,
	EF_CLOUD6,
	EF_FOOD01,
	EF_FOOD02,
	EF_FOOD03,
	EF_FOOD04,
	EF_FOOD05,
	EF_FOOD06,
	EF_SHRINK,
	EF_THROWITEM6,
	EF_SIGHT2,
	EF_QUAKEBODY4,
	EF_FIREHIT2,
	EF_NPC_STOP2,
	EF_NPC_STOP2_DEL,
	EF_FVOICE,
	EF_WINK,
	EF_COOKING_OK,
	EF_COOKING_FAIL,
	EF_TEMP_OK,
	EF_TEMP_FAIL,
	EF_HAPGYEOK,
	EF_THROWITEM7,
	EF_THROWITEM8,
	EF_THROWITEM9,
	EF_THROWITEM10,
	EF_BUNSINJYUTSU,
	EF_KOUENKA,
	EF_HYOUSENSOU,
	EF_BOTTOM_SUITON,
	EF_STIN4,
	EF_THUNDERSTORM2,
	EF_CHEMICAL4,
	EF_STIN5,
	EF_MADNESS_BLUE,
	EF_MADNESS_RED,
	EF_RG_COIN3,
	EF_BASH3D5,
	EF_CHOOKGI3,
	EF_KIRIKAGE,
	EF_TATAMI,
	EF_KASUMIKIRI,
	EF_ISSEN,
	EF_KAEN,
	EF_BAKU,
	EF_HYOUSYOURAKU,
	EF_DESPERADO,
	EF_LIGHTNING_S,
	EF_BLIND_S,
	EF_POISON_S,
	EF_FREEZING_S,
	EF_FLARE_S,
	EF_RAPIDSHOWER,
	EF_MAGICALBULLET,
	EF_SPREADATTACK,
	EF_TRACKCASTING,
	EF_TRACKING,
	EF_TRIPLEACTION,
	EF_BULLSEYE,
	EF_MAP_MAGICZONE,
	EF_MAP_MAGICZONE2,
	EF_DAMAGE1,
	EF_DAMAGE1_2,
	EF_DAMAGE1_3,
	EF_UNDEADBODY,
	EF_UNDEADBODY_DEL,
	EF_GREEN_NUMBER,
	EF_BLUE_NUMBER,
	EF_RED_NUMBER,
	EF_PURPLE_NUMBER,
	EF_BLACK_NUMBER,
	EF_WHITE_NUMBER,
	EF_YELLOW_NUMBER,
	EF_PINK_NUMBER,
	EF_BUBBLE_DROP,
	EF_NPC_EARTHQUAKE,
	EF_DA_SPACE,
	EF_DRAGONFEAR,
	EF_BLEEDING,
	EF_WIDECONFUSE,
	EF_BOTTOM_RUNNER,
	EF_BOTTOM_TRANSFER,
	EF_CRYSTAL_BLUE,
	EF_BOTTOM_EVILLAND,
	EF_GUARD3,
	EF_NPC_SLOWCAST,
	EF_CRITICALWOUND,
	EF_GREEN99_3,
	EF_GREEN99_5,
	EF_GREEN99_6,
	EF_MAPSPHERE,
	EF_POK_LOVE,
	EF_POK_WHITE,
	EF_POK_VALEN,
	EF_POK_BIRTH,
	EF_POK_CHRISTMAS,
	EF_MAP_MAGICZONE3,
	EF_MAP_MAGICZONE4,
	EF_DUST,
	EF_TORCH_RED,
	EF_TORCH_GREEN,
	EF_MAP_GHOST,
	EF_GLOW1,
	EF_GLOW2,
	EF_GLOW4,
	EF_TORCH_PURPLE,
	EF_CLOUD7,
	EF_CLOUD8,
	EF_FLOWERLEAF,
	EF_MAPSPHERE2,
	EF_GLOW11,
	EF_GLOW12,
	EF_CIRCLELIGHT,
	EF_ITEM315,
	EF_ITEM316,
	EF_ITEM317,
	EF_ITEM318,
	EF_STORM_MIN,
	EF_POK_JAP,
	EF_MAP_GREENLIGHT,
	EF_MAP_MAGICWALL,
	EF_MAP_GREENLIGHT2,
	EF_YELLOWFLY1,
	EF_YELLOWFLY2,
	EF_BOTTOM_BLUE,
	EF_BOTTOM_BLUE2,
	EF_WEWISH,
	EF_FIREPILLARON2,
	EF_FORESTLIGHT5,
	EF_SOULBREAKER3,
	EF_ADO_STR,
	EF_IGN_STR,
	EF_CHIMTO2,
	EF_WINDCUTTER,
	EF_DETECT2,
	EF_FROSTMYSTY,
	EF_CRIMSON_STR,
	EF_HELL_STR,
	EF_SPR_MASH,
	EF_SPR_SOULE,
	EF_DHOWL_STR,
	EF_EARTHWALL,
	EF_SOULBREAKER4,
	EF_CHAINL_STR,
	EF_CHOOKGI_FIRE,
	EF_CHOOKGI_WIND,
	EF_CHOOKGI_WATER,
	EF_CHOOKGI_GROUND,
	EF_MAGENTA_TRAP,
	EF_COBALT_TRAP,
	EF_MAIZE_TRAP,
	EF_VERDURE_TRAP,
	EF_NORMAL_TRAP,
	EF_CLOAKING2,
	EF_AIMED_STR,
	EF_ARROWSTORM_STR,
	EF_LAULAMUS_STR,
	EF_LAUAGNUS_STR,
	EF_MILSHIELD_STR,
	EF_CONCENTRATION2,
	EF_FIREBALL2,
	EF_BUNSINJYUTSU2,
	EF_CLEARTIME,
	EF_GLASSWALL3,
	EF_ORATIO,
	EF_POTION_BERSERK2,
	EF_CIRCLEPOWER,
	EF_ROLLING1,
	EF_ROLLING2,
	EF_ROLLING3,
	EF_ROLLING4,
	EF_ROLLING5,
	EF_ROLLING6,
	EF_ROLLING7,
	EF_ROLLING8,
	EF_ROLLING9,
	EF_ROLLING10,
	EF_PURPLEBODY,
	EF_STIN6,
	EF_RG_COIN4,
	EF_POISONWAV,
	EF_POISONSMOKE,
	EF_GUMGANG4,
	EF_SHIELDBOOMERANG4,
	EF_CASTSPIN2,
	EF_VULCANWAV,
	EF_AGIUP2,
	EF_DETECT3,
	EF_AGIUP3,
	EF_DETECT4,
	EF_ELECTRIC3,
	EF_GUARD4,
	EF_BOTTOM_BARRIER,
	EF_BOTTOM_STEALTH,
	EF_REPAIRTIME,
	EF_NC_ANAL,
	EF_FIRETHROW,
	EF_VENOMIMPRESS,
	EF_FROSTMISTY,
	EF_BURNING,
	EF_COLDTHROW,
	EF_MAKEHALLU,
	EF_HALLUTIME,
	EF_INFRAREDSCAN,
	EF_CRASHAXE,
	EF_GTHUNDER,
	EF_STONERING,
	EF_INTIMIDATE2,
	EF_STASIS,
	EF_REDLINE,
	EF_FROSTDIVER3,
	EF_BOTTOM_BASILICA2,
	EF_RECOGNIZED,
	EF_TETRA,
	EF_TETRACASTING,
	EF_FIREBALL3,
	EF_INTIMIDATE3,
	EF_RECOGNIZED2,
	EF_CLOAKING3,
	EF_INTIMIDATE4,
	EF_STRETCH,
	EF_BLACKBODY,
	EF_ENERVATION,
	EF_ENERVATION2,
	EF_ENERVATION3,
	EF_ENERVATION4,
	EF_ENERVATION5,
	EF_ENERVATION6,
	EF_LINELINK4,
	EF_RG_COIN5,
	EF_WATERFALL_ANI,
	EF_BOTTOM_MANHOLE,
	EF_MANHOLE,
	EF_MAKEFEINT,
	EF_FORESTLIGHT6,
	EF_DARKCASTING2,
	EF_BOTTOM_ANI,
	EF_BOTTOM_MAELSTROM,
	EF_BOTTOM_BLOODYLUST,
	EF_BEGINSPELL_N1,
	EF_BEGINSPELL_N2,
	EF_HEAL_N,
	EF_CHOOKGI_N,
	EF_JOBLVUP50_2,
	EF_CHEMICAL2DASH2,
	EF_CHEMICAL2DASH3,
	EF_ROLLINGCAST,
	EF_WATER_BELOW,
	EF_WATER_FADE,
	EF_BEGINSPELL_N3,
	EF_BEGINSPELL_N4,
	EF_BEGINSPELL_N5,
	EF_BEGINSPELL_N6,
	EF_BEGINSPELL_N7,
	EF_BEGINSPELL_N8,
	EF_WATER_SMOKE,
	EF_DANCE1,
	EF_DANCE2,
	EF_LINKPARTICLE,
	EF_SOULLIGHT2,
	EF_SPR_PARTICLE,
	EF_SPR_PARTICLE2,
	EF_SPR_PLANT,
	EF_CHEMICAL_V,
	EF_SHOOTPARTICLE,
	EF_BOT_REVERB,
	EF_RAIN_PARTICLE,
	EF_CHEMICAL_V2,
	EF_SECRA,
	EF_BOT_REVERB2,
	EF_CIRCLEPOWER2,
	EF_SECRA2,
	EF_CHEMICAL_V3,
	EF_ENERVATION7,
	EF_CIRCLEPOWER3,
	EF_SPR_PLANT2,
	EF_CIRCLEPOWER4,
	EF_SPR_PLANT3,
	EF_RG_COIN6,
	EF_SPR_PLANT4,
	EF_CIRCLEPOWER5,
	EF_SPR_PLANT5,
	EF_CIRCLEPOWER6,
	EF_SPR_PLANT6,
	EF_CIRCLEPOWER7,
	EF_SPR_PLANT7,
	EF_CIRCLEPOWER8,
	EF_SPR_PLANT8,
	EF_HEARTASURA,
	EF_BEGINSPELL_150,
	EF_LEVEL99_150,
	EF_PRIMECHARGE,
	EF_GLASSWALL4,
	EF_GRADIUS_LASER,
	EF_BASH3D6,
	EF_GUMGANG5,
	EF_HITLINE8,
	EF_ELECTRIC4,
	EF_TEIHIT1T,
	EF_SPINMOVE,
	EF_FIREBALL4,
	EF_TRIPLEATTACK4,
	EF_CHEMICAL3S,
	EF_GROUNDSHAKE,
	EF_DQ9_CHARGE,
	EF_DQ9_CHARGE2,
	EF_DQ9_CHARGE3,
	EF_DQ9_CHARGE4,
	EF_BLUELINE,
	EF_SELFSCROLL,
	EF_SPR_LIGHTPRINT,
	EF_PNG_TEST,
	EF_BEGINSPELL_YB,
	EF_CHEMICAL2DASH4,
	EF_GROUNDSHAKE2,
	EF_PRESSURE2,
	EF_RG_COIN7,
	EF_PRIMECHARGE2,
	EF_PRIMECHARGE3,
	EF_PRIMECHARGE4,
	EF_GREENCASTING,
	EF_WALLOFTHORN,
	EF_FIREBALL5,
	EF_THROWITEM11,
	EF_SPR_PLANT9,
	EF_DEMONICFIRE,
	EF_DEMONICFIRE2,
	EF_DEMONICFIRE3,
	EF_HELLSPLANT,
	EF_FIREWALL2,
	EF_VACUUM,
	EF_SPR_PLANT10,
	EF_SPR_LIGHTPRINT2,
	EF_POISONSMOKE2,
	EF_MAKEHALLU2,
	EF_SHOCKWAVE2,
	EF_SPR_PLANT11,
	EF_COLDTHROW2,
	EF_DEMONICFIRE4,
	EF_PRESSURE3,
	EF_LINKPARTICLE2,
	EF_SOULLIGHT3,
	EF_CHAREFFECT,
	EF_GUMGANG6,
	EF_FIREBALL6,
	EF_GUMGANG7,
	EF_GUMGANG8,
	EF_GUMGANG9,
	EF_BOTTOM_DE2,
	EF_COLDSTATUS,
	EF_SPR_LIGHTPRINT3,
	EF_WATERBALL3,
	EF_HEAL_N2,
	EF_RAIN_PARTICLE2,
	EF_CLOUD9,
	EF_YELLOWFLY3,
	EF_EL_GUST,
	EF_EL_BLAST,
	EF_EL_AQUAPLAY,
	EF_EL_UPHEAVAL,
	EF_EL_WILD_STORM,
	EF_EL_CHILLY_AIR,
	EF_EL_CURSED_SOIL,
	EF_EL_COOLER,
	EF_EL_TROPIC,
	EF_EL_PYROTECHNIC,
	EF_EL_PETROLOGY,
	EF_EL_HEATER,
	EF_POISON_MIST,
	EF_ERASER_CUTTER,
	EF_SILENT_BREEZE,
	EF_MAGMA_FLOW,
	EF_GRAYBODY,
	EF_LAVA_SLIDE,
	EF_SONIC_CLAW,
	EF_TINDER_BREAKER,
	EF_MIDNIGHT_FRENZY,
	EF_MACRO,
	EF_CHEMICAL_ALLRANGE,
	EF_TETRA_FIRE,
	EF_TETRA_WATER,
	EF_TETRA_WIND,
	EF_TETRA_GROUND,
	EF_EMITTER,
	EF_VOLCANIC_ASH,
	EF_LEVEL99_ORB1,
	EF_LEVEL99_ORB2,
	EF_LEVEL150,
	EF_LEVEL150_SUB,
	EF_THROWITEM4_1,
	EF_THROW_HAPPOKUNAI,
	EF_THROW_MULTIPLE_COIN,
	EF_THROW_BAKURETSU,
	EF_ROTATE_HUUMARANKA,
	EF_ROTATE_BG,
	EF_ROTATE_LINE_GRAY,
	EF_2011RWC,
	EF_2011RWC2,
	EF_KAIHOU,
	EF_GROUND_EXPLOSION,
	EF_KG_KAGEHUMI,
	EF_KO_ZENKAI_WATER,
	EF_KO_ZENKAI_LAND,
	EF_KO_ZENKAI_FIRE,
	EF_KO_ZENKAI_WIND,
	EF_KO_JYUMONJIKIRI,
	EF_KO_SETSUDAN,
	EF_RED_CROSS,
	EF_KO_IZAYOI,
	EF_ROTATE_LINE_BLUE,
	EF_KG_KYOMU,
	EF_KO_HUUMARANKA,
	EF_BLUELIGHTBODY,
	EF_KAGEMUSYA,
	EF_OB_GENSOU,
	EF_NO100_FIRECRACKER,
	EF_KO_MAKIBISHI,
	EF_KAIHOU1,
	EF_AKAITSUKI,
	EF_ZANGETSU,
	EF_GENSOU,
	EF_HAT_EFFECT,
	EF_CHERRYBLOSSOM,
	EF_EVENT_CLOUD,
	EF_RUN_MAKE_OK,
	EF_RUN_MAKE_FAILURE,
	EF_MIRESULT_MAKE_OK,
	EF_MIRESULT_MAKE_FAIL,
	EF_ALL_RAY_OF_PROTECTION,
	EF_VENOMFOG,
	EF_DUSTSTORM,
	EF_LEVEL160,
	EF_LEVEL160_SUB,
	EF_MAPCHAIN,
	EF_MAGIC_FLOOR,
	EF_ICEMINE,
	EF_FLAMECORSS,
	EF_ICEMINE_1,
	EF_DANCE_BLADE_ATK,
	EF_DARKPIERCING,
	EF_INVINCIBLEOFF2,
	EF_MAXPAIN,
	EF_DEATHSUMMON,
	EF_MOONSTAR,
	EF_STRANGELIGHTS,
	EF_SUPER_STAR,
	EF_YELLOBODY,
	EF_COLORPAPER2,
	EF_EVILS_PAW,
	EF_GC_DARKCROW,
	EF_RK_DRAGONBREATH_WATER,
	EF_ALL_FULL_THROTTLE,
	EF_SR_FLASHCOMBO,
	EF_RK_LUXANIMA,
	EF_CLOUD10,
	EF_SO_ELEMENTAL_SHIELD,
	EF_AB_OFFERTORIUM,
	EF_WL_TELEKINESIS_INTENSE,
	EF_GN_ILLUSIONDOPING,
	EF_NC_MAGMA_ERUPTION,
	EF_LG_KINGS_GRACE,
	EF_BLOODDRAIN2,
	EF_NPC_WIDEWEB,
	EF_NPC_BURNT,
	EF_NPC_CHILL,
	EF_RA_UNLIMIT,
	EF_AB_OFFERTORIUM_RING,
	EF_SC_ESCAPE,
	EF_WM_FRIGG_SONG,
	EF_FLICKER,
	EF_C_MAKER,
	EF_HAMMER_OF_GOD,
	EF_MASS_SPIRAL,
	EF_FIRE_RAIN,
	EF_WHITEBODY,
	EF_BANISHING_BUSTER,
	EF_SLUGSHOT,
	EF_D_TAIL,
	EF_BIND_TRAP1,
	EF_BIND_TRAP2,
	EF_BIND_TRAP3,
	EF_JUMPBODY1,
	EF_ANIMATED_EMITTER,
	EF_RL_EXPLOSION,
	EF_C_MAKER_1,
	EF_QD_SHOT,
	EF_P_ALTER,
	EF_S_STORM,
	EF_MUSIC_HAT,
	EF_CLOUD_KILL,
	EF_ESCAPE,
	EF_XENO_SLASHER,
	EF_FLOWERSMOKE,
	EF_FSTONE,
	EF_QSCARABA,
	EF_LJOSALFAR,
	EF_HAPPINESSSTAR,
	EF_POWER_OF_GAIA,
	EF_MAPLE_FALLS,
	EF_MARKING_USE_CHANGEMONSTER,
	EF_MAGICAL_FEATHER,
	EF_MERMAID_LONGING,
	EF_GIFT_OF_SNOW,
	EF_ACH_COMPLETE,
	EF_TIME_ACCESSORY,
	EF_SPRITEMABLE,
	EF_TUNAPARTY,
	EF_MAX
};

enum e_hat_effects {
	HAT_EF_MIN = 0,
	HAT_EF_BLOSSOM_FLUTTERING,
	HAT_EF_MERMAID_LONGING,
	HAT_EF_RL_BANISHING_BUSTER,
	HAT_EF_LJOSALFAR,
	HAT_EF_CLOCKING,
	HAT_EF_SNOW,
	HAT_EF_MAKEBLUR,
	HAT_EF_SLEEPATTACK,
	HAT_EF_GUMGANG,
	HAT_EF_TALK_FROSTJOKE,
	HAT_EF_DEMONSTRATION,
	HAT_EF_FLUTTER_BUTTERFLY,
	HAT_EF_ANGEL_FLUTTERING,
	HAT_EF_BLESSING_OF_ANGELS,
	HAT_EF_ELECTRIC,
	HAT_EF_GREEN_FLOOR,
	HAT_EF_SHRINK,
	HAT_EF_VALHALLA_IDOL,
	HAT_EF_ANGEL_STAIRS,
	HAT_EF_GLOW_OF_NEW_YEAR,
	HAT_EF_BOTTOM_FORTUNEKISS,
	HAT_EF_PINKBODY,
	HAT_EF_DOUBLEGUMGANG,
	HAT_EF_GIANTBODY,
	HAT_EF_GREEN99_6,
	HAT_EF_CIRCLEPOWER,
	HAT_EF_BOTTOM_BLOODYLUST,
	HAT_EF_WATER_BELOW,
	HAT_EF_LEVEL99_150,
	HAT_EF_YELLOWFLY3,
	HAT_EF_KAGEMUSYA,
	HAT_EF_CHERRYBLOSSOM,
	HAT_EF_STRANGELIGHTS,
	HAT_EF_WL_TELEKINESIS_INTENSE,
	HAT_EF_AB_OFFERTORIUM_RING,
	HAT_EF_WHITEBODY2,
	HAT_EF_SAKURA,
	HAT_EF_CLOUD2,
	HAT_EF_FEATHER_FLUTTERING,
	HAT_EF_CAMELLIA_HAIR_PIN,
	HAT_EF_JP_EV_EFFECT01,
	HAT_EF_JP_EV_EFFECT02,
	HAT_EF_JP_EV_EFFECT03,
	HAT_EF_FLORAL_WALTZ,
	HAT_EF_MAGICAL_FEATHER,
	HAT_EF_HAT_EFFECT,
	HAT_EF_BAKURETSU_HADOU,
	HAT_EF_GOLD_SHOWER,
	HAT_EF_WHITEBODY,
	HAT_EF_WATER_BELOW2,
	HAT_EF_FIREWORK,
	HAT_EF_RETURN_TW_1ST_HAT,
	HAT_EF_C_FLUTTERBUTTERFLY_BL,
	HAT_EF_QSCARABA,
	HAT_EF_FSTONE,
	HAT_EF_MAGICCIRCLE,
	HAT_EF_GODCLASS,
	HAT_EF_GODCLASS2,
	HAT_EF_LEVEL99_RED,
	HAT_EF_LEVEL99_ULTRAMARINE,
	HAT_EF_LEVEL99_CYAN,
	HAT_EF_LEVEL99_LIME,
	HAT_EF_LEVEL99_VIOLET,
	HAT_EF_LEVEL99_LILAC,
	HAT_EF_LEVEL99_SUN_ORANGE,
	HAT_EF_LEVEL99_DEEP_PINK,
	HAT_EF_LEVEL99_BLACK,
	HAT_EF_LEVEL99_WHITE,
	HAT_EF_LEVEL160_RED,
	HAT_EF_LEVEL160_ULTRAMARINE,
	HAT_EF_LEVEL160_CYAN,
	HAT_EF_LEVEL160_LIME,
	HAT_EF_LEVEL160_VIOLET,
	HAT_EF_LEVEL160_LILAC,
	HAT_EF_LEVEL160_SUN_ORANGE,
	HAT_EF_LEVEL160_DEEP_PINK,
	HAT_EF_LEVEL160_BLACK,
	HAT_EF_LEVEL160_WHITE,
	HAT_EF_FULL_BLOOMCHERRY_TREE,
	HAT_EF_C_BLESSINGS_OF_SOUL,
	HAT_EF_MANYSTARS,
	HAT_EF_SUBJECT_AURA_GOLD,
	HAT_EF_SUBJECT_AURA_WHITE,
	HAT_EF_SUBJECT_AURA_RED,
	HAT_EF_C_SHINING_ANGEL_WING,
	HAT_EF_MAGIC_STAR_TW,
	HAT_EF_DIGITAL_SPACE,
	HAT_EF_SLEIPNIR,
	HAT_EF_C_MAPLE_WHICH_FALLS_RD,
	HAT_EF_MAGICCIRCLERAINBOW,
	HAT_EF_SNOWFLAKE_TIARA,
	HAT_EF_MIDGARTS_GLORY,
	HAT_EF_LEVEL99_TIGER,
	HAT_EF_LEVEL160_TIGER,
	HAT_EF_FLUFFYWING,
	HAT_EF_C_GHOST_EFFECT,
	HAT_EF_C_POPPING_PORING_AURA,
	HAT_EF_RESONATETAEGO,
	HAT_EF_99LV_RUNE_RED,
	HAT_EF_99LV_ROYAL_GUARD_BLUE,
	HAT_EF_99LV_WARLOCK_VIOLET,
	HAT_EF_99LV_SORCERER_LBLUE,
	HAT_EF_99LV_RANGER_GREEN,
	HAT_EF_99LV_MINSTREL_PINK,
	HAT_EF_99LV_ARCHBISHOP_WHITE,
	HAT_EF_99LV_GUILL_SILVER,
	HAT_EF_99LV_SHADOWC_BLACK,
	HAT_EF_99LV_MECHANIC_GOLD,
	HAT_EF_99LV_GENETIC_YGREEN,
	HAT_EF_160LV_RUNE_RED,
	HAT_EF_160LV_ROYAL_G_BLUE,
	HAT_EF_160LV_WARLOCK_VIOLET,
	HAT_EF_160LV_SORCERER_LBLUE,
	HAT_EF_160LV_RANGER_GREEN,
	HAT_EF_160LV_MINSTREL_PINK,
	HAT_EF_160LV_ARCHB_WHITE,
	HAT_EF_160LV_GUILL_SILVER,
	HAT_EF_160LV_SHADOWC_BLACK,
	HAT_EF_160LV_MECHANIC_GOLD,
	HAT_EF_160LV_GENETIC_YGREEN,
	HAT_EF_WATER_BELOW3,
	HAT_EF_WATER_BELOW4,
	HAT_EF_C_VALKYRIE_WING,
	HAT_EF_2019RTC_CELEAURA_TW,
	HAT_EF_2019RTC1ST_TW,
	HAT_EF_2019RTC2ST_TW,
	HAT_EF_2019RTC3ST_TW,
	HAT_EF_CONS_OF_WIND,
	HAT_EF_MAX
};

enum e_convertpcinfo_type : uint8 {
	CPC_NAME      = 0,
	CPC_CHAR      = 1,
	CPC_ACCOUNT   = 2
};

/**
 * Player blocking actions related flags.
 */
enum e_pcblock_action_flag : uint16 {
	PCBLOCK_MOVE     = 0x001,
	PCBLOCK_ATTACK   = 0x002,
	PCBLOCK_SKILL    = 0x004,
	PCBLOCK_USEITEM  = 0x008,
	PCBLOCK_CHAT     = 0x010,
	PCBLOCK_IMMUNE   = 0x020,
	PCBLOCK_SITSTAND = 0x040,
	PCBLOCK_COMMANDS = 0x080,
	PCBLOCK_NPCCLICK = 0x100,
	PCBLOCK_NPC      = 0x18D,
	PCBLOCK_EMOTION  = 0x200,
	PCBLOCK_ALL      = 0x3FF,
};

/**
 * used to generate quick script_array entries
 **/
extern struct eri *array_ers;
extern DBMap *st_db;
extern unsigned int active_scripts;
extern unsigned int next_id;
extern struct eri *st_ers;
extern struct eri *stack_ers;

const char* skip_space(const char* p);
void script_error(const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos);
void script_warning(const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos);

bool is_number(const char *p);
struct script_code* parse_script(const char* src,const char* file,int line,int options);
void run_script(struct script_code *rootscript,int pos,int rid,int oid);

int set_reg(struct script_state* st, struct map_session_data* sd, int64 num, const char* name, const void* value, struct reg_db *ref);
int set_var(struct map_session_data *sd, char *name, void *val);
int conv_num(struct script_state *st,struct script_data *data);
const char* conv_str(struct script_state *st,struct script_data *data);
void pop_stack(struct script_state* st, int start, int end);
TIMER_FUNC(run_script_timer);
void script_stop_sleeptimers(int id);
struct linkdb_node *script_erase_sleepdb(struct linkdb_node *n);
void script_attach_state(struct script_state* st);
void script_detach_rid(struct script_state* st);
void run_script_main(struct script_state *st);

void script_stop_scriptinstances(struct script_code *code);
void script_free_code(struct script_code* code);
void script_free_vars(struct DBMap *storage);
struct script_state* script_alloc_state(struct script_code* rootscript, int pos, int rid, int oid);
void script_free_state(struct script_state* st);

struct DBMap* script_get_label_db(void);
struct DBMap* script_get_userfunc_db(void);
void script_run_autobonus(const char *autobonus, struct map_session_data *sd, unsigned int pos);

const char* script_get_constant_str(const char* prefix, int64 value);
bool script_get_parameter(const char* name, int* value);
bool script_get_constant(const char* name, int* value);
void script_set_constant_(const char* name, int value, const char* constant_name, bool isparameter, bool deprecated);
#define script_set_constant(name, value, isparameter, deprecated) script_set_constant_(name, value, NULL, isparameter, deprecated)
void script_hardcoded_constants(void);

void script_cleararray_pc(struct map_session_data* sd, const char* varname, void* value);
void script_setarray_pc(struct map_session_data* sd, const char* varname, uint32 idx, void* value, int* refcache);

int script_config_read(const char *cfgName);
void do_init_script(void);
void do_final_script(void);
int add_str(const char* p);
const char* get_str(int id);
void script_reload(void);

// @commands (script based)
void setd_sub(struct script_state *st, struct map_session_data *sd, const char *varname, int elem, void *value, struct reg_db *ref);

/**
 * Array Handling
 **/
struct reg_db *script_array_src(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref);
void script_array_update(struct reg_db *src, int64 num, bool empty);
void script_array_delete(struct reg_db *src, struct script_array *sa);
void script_array_remove_member(struct reg_db *src, struct script_array *sa, unsigned int idx);
void script_array_add_member(struct script_array *sa, unsigned int idx);
unsigned int script_array_size(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref);
unsigned int script_array_highest_key(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref);
void script_array_ensure_zero(struct script_state *st, struct map_session_data *sd, int64 uid, struct reg_db *ref);
int script_free_array_db(DBKey key, DBData *data, va_list ap);
/* */
void script_reg_destroy_single(struct map_session_data *sd, int64 reg, struct script_reg_state *data);
int script_reg_destroy(DBKey key, DBData *data, va_list ap);
/* */
void script_generic_ui_array_expand(unsigned int plus);
unsigned int *script_array_cpy_list(struct script_array *sa);

bool script_check_RegistryVariableLength(int pType, const char *val, size_t* vlen);

#endif /* SCRIPT_HPP */
