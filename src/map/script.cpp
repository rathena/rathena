// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

//#define DEBUG_DISP
//#define DEBUG_DISASM
//#define DEBUG_RUN
//#define DEBUG_HASH
//#define DEBUG_DUMP_STACK

#include "script.hpp"

#include <errno.h>
#include <math.h>
#include <setjmp.h>
#include <stdlib.h> // atoi, strtol, strtoll, exit

#ifdef PCRE_SUPPORT
#include "../../3rdparty/pcre/include/pcre.h" // preg_match
#endif

#include "../common/cbasetypes.hpp"
#include "../common/ers.hpp"  // ers_destroy
#include "../common/malloc.hpp"
#include "../common/md5calc.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utils.hpp"

#include "achievement.hpp"
#include "atcommand.hpp"
#include "battle.hpp"
#include "battleground.hpp"
#include "channel.hpp"
#include "chat.hpp"
#include "chrif.hpp"
#include "clan.hpp"
#include "clif.hpp"
#include "date.hpp" // date type enum, date_get()
#include "elemental.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "instance.hpp"
#include "intif.hpp"
#include "itemdb.hpp"
#include "log.hpp"
#include "mail.hpp"
#include "map.hpp"
#include "mapreg.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "path.hpp"
#include "pc.hpp"
#include "pc_groups.hpp"
#include "pet.hpp"
#include "quest.hpp"
#include "storage.hpp"

struct eri *array_ers;
DBMap *st_db;
unsigned int active_scripts;
unsigned int next_id;
struct eri *st_ers;
struct eri *stack_ers;

static bool script_rid2sd_( struct script_state *st, struct map_session_data** sd, const char *func );

/**
 * Get `sd` from a account id in `loc` param instead of attached rid
 * @param st Script
 * @param loc Location to look account id in script parameter
 * @param sd Variable that will be assigned
 * @return True if `sd` is assigned, false otherwise
 **/
static bool script_accid2sd_(struct script_state *st, uint8 loc, struct map_session_data **sd, const char *func) {
	if (script_hasdata(st, loc)) {
		int id_ = script_getnum(st, loc);
		if (!(*sd = map_id2sd(id_))){
			ShowError("%s: Player with account id '%d' is not found.\n", func, id_);
			return false;
		}else{
			return true;
		}
	}
	else
		return script_rid2sd_(st,sd,func);
}

/**
 * Get `sd` from a char id in `loc` param instead of attached rid
 * @param st Script
 * @param loc Location to look char id in script parameter
 * @param sd Variable that will be assigned
 * @return True if `sd` is assigned, false otherwise
 **/
static bool script_charid2sd_(struct script_state *st, uint8 loc, struct map_session_data **sd, const char *func) {
	if (script_hasdata(st, loc)) {
		int id_ = script_getnum(st, loc);
		if (!(*sd = map_charid2sd(id_))){
			ShowError("%s: Player with char id '%d' is not found.\n", func, id_);
			return false;
		}else{
			return true;
		}
	}
	else
		return script_rid2sd_(st,sd,func);
}

/**
 * Get `sd` from a nick in `loc` param instead of attached rid
 * @param st Script
 * @param loc Location to look nick in script parameter
 * @param sd Variable that will be assigned
 * @return True if `sd` is assigned, false otherwise
 **/
static bool script_nick2sd_(struct script_state *st, uint8 loc, struct map_session_data **sd, const char *func) {
	if (script_hasdata(st, loc)) {
		const char *name_ = script_getstr(st, loc);
		if (!(*sd = map_nick2sd(name_,false))){
			ShowError("%s: Player with nick '%s' is not found.\n", func, name_);
			return false;
		}else{
			return true;
		}
	}
	else
		return script_rid2sd_(st,sd,func);
}

/**
 * Get `sd` from a mapid in `loc` param instead of attached rid
 * @param st Script
 * @param loc Location to look mapid in script parameter
 * @param sd Variable that will be assigned
 * @return True if `sd` is assigned, false otherwise
 **/
static bool script_mapid2sd_(struct script_state *st, uint8 loc, struct map_session_data **sd, const char *func) {
	if (script_hasdata(st, loc)) {
		int id_ = script_getnum(st, loc);
		if (!(*sd = map_id2sd(id_))){
			ShowError("%s: Player with map id '%d' is not found.\n", func, id_);
			return false;
		}else{
			return true;
		}
	}
	else
		return script_rid2sd_(st,sd,func);
}

/**
 * Get `bl` from an ID in `loc` param, if ID is 0 will return the `bl` of the script's activator.
 * @param st Script
 * @param loc Location to look for ID in script parameter
 * @param bl Variable that will be assigned
 * @return True if `bl` is assigned, false otherwise
 **/
static bool script_rid2bl_(struct script_state *st, uint8 loc, struct block_list **bl, const char *func) {
	int unit_id;

	if ( !script_hasdata(st, loc) || ( unit_id = script_getnum(st, loc) ) == 0)
		unit_id = st->rid;
		
	*bl =  map_id2bl(unit_id);

	if ( *bl )
		return true;
	else {
		ShowError("%s: Unit with ID '%d' is not found.\n", func, unit_id);
		return false;
	}
}

#define script_accid2sd(loc,sd) script_accid2sd_(st,(loc),&(sd),__FUNCTION__)
#define script_charid2sd(loc,sd) script_charid2sd_(st,(loc),&(sd),__FUNCTION__)
#define script_nick2sd(loc,sd) script_nick2sd_(st,(loc),&(sd),__FUNCTION__)
#define script_mapid2sd(loc,sd) script_mapid2sd_(st,(loc),&(sd),__FUNCTION__)
#define script_rid2sd(sd) script_rid2sd_(st,&(sd),__FUNCTION__)
#define script_rid2bl(loc,bl) script_rid2bl_(st,(loc),&(bl),__FUNCTION__)

/// temporary buffer for passing around compiled bytecode
/// @see add_scriptb, set_label, parse_script
static unsigned char* script_buf = NULL;
static int script_pos = 0, script_size = 0;

static inline int GETVALUE(const unsigned char* buf, int i)
{
	return (int)MakeDWord(MakeWord(buf[i], buf[i+1]), MakeWord(buf[i+2], 0));
}
static inline void SETVALUE(unsigned char* buf, int i, int n)
{
	buf[i]   = GetByte(n, 0);
	buf[i+1] = GetByte(n, 1);
	buf[i+2] = GetByte(n, 2);
}

// String buffer structures.
// str_data stores string information
static struct str_data_struct {
	enum c_op type;
	int str;
	int backpatch;
	int label;
	int (*func)(struct script_state *st);
	int val;
	int next;
	bool deprecated;
} *str_data = NULL;
static int str_data_size = 0; // size of the data
static int str_num = LABEL_START; // next id to be assigned

// str_buf holds the strings themselves
static char *str_buf;
static int str_size = 0; // size of the buffer
static int str_pos = 0; // next position to be assigned


// Using a prime number for SCRIPT_HASH_SIZE should give better distributions
#define SCRIPT_HASH_SIZE 1021
int str_hash[SCRIPT_HASH_SIZE];
// Specifies which string hashing method to use
//#define SCRIPT_HASH_DJB2
//#define SCRIPT_HASH_SDBM
#define SCRIPT_HASH_ELF

static DBMap* scriptlabel_db = NULL; // const char* label_name -> int script_pos
static DBMap* userfunc_db = NULL; // const char* func_name -> struct script_code*
static int parse_options = 0;
DBMap* script_get_label_db(void) { return scriptlabel_db; }
DBMap* script_get_userfunc_db(void) { return userfunc_db; }

// important buildin function references for usage in scripts
static int buildin_set_ref = 0;
static int buildin_callsub_ref = 0;
static int buildin_callfunc_ref = 0;
static int buildin_getelementofarray_ref = 0;

// Caches compiled autoscript item code.
// Note: This is not cleared when reloading itemdb.
static DBMap* autobonus_db = NULL; // char* script -> char* bytecode

struct Script_Config script_config = {
	1, // warn_func_mismatch_argtypes
	1, 65535, 2048, //warn_func_mismatch_paramnum/check_cmdcount/check_gotocount
	0, INT_MAX, // input_min_value/input_max_value
	// NOTE: None of these event labels should be longer than <EVENT_NAME_LENGTH> characters
	// PC related
	"OnPCDieEvent", //die_event_name
	"OnPCKillEvent", //kill_pc_event_name
	"OnNPCKillEvent", //kill_mob_event_name
	"OnPCLoginEvent", //login_event_name
	"OnPCLogoutEvent", //logout_event_name
	"OnPCLoadMapEvent", //loadmap_event_name
	"OnPCBaseLvUpEvent", //baselvup_event_name
	"OnPCJobLvUpEvent", //joblvup_event_name
	"OnPCStatCalcEvent", //stat_calc_event_name
	// NPC related
	"OnTouch_",	//ontouch_event_name (runs on first visible char to enter area, picks another char if the first char leaves)
	"OnTouch",	//ontouch2_event_name (run whenever a char walks into the OnTouch area)
	"OnTouchNPC", //ontouchnpc_event_name (run whenever a monster walks into the OnTouch area)
	"OnWhisperGlobal",	//onwhisper_event_name (is executed when a player sends a whisper message to the NPC)
	"OnCommand", //oncommand_event_name (is executed by script command cmdothernpc)
	"OnBuyItem", //onbuy_event_name (is executed when items are bought)
	"OnSellItem", //onsell_event_name (is executed when items are sold)
	// Init related
	"OnInit", //init_event_name (is executed on all npcs when all npcs were loaded)
	"OnInterIfInit", //inter_init_event_name (is executed on inter server connection)
	"OnInterIfInitOnce", //inter_init_once_event_name (is only executed on the first inter server connection)
	// Guild related
	"OnGuildBreak", //guild_break_event_name (is executed on all castles of the guild that is broken)
	"OnAgitStart", //agit_start_event_name (is executed when WoE FE is started)
	"OnAgitInit", //agit_init_event_name (is executed after all castle owning guilds have been loaded)
	"OnAgitEnd", //agit_end_event_name (is executed when WoE FE has ended)
	"OnAgitStart2", //agit_start2_event_name (is executed when WoE SE is started)
	"OnAgitInit2", //agit_init2_event_name (is executed after all castle owning guilds have been loaded)
	"OnAgitEnd2", //agit_end2_event_name (is executed when WoE SE has ended)
	"OnAgitStart3", //agit_start3_event_name (is executed when WoE TE is started)
	"OnAgitInit3", //agit_init3_event_name (is executed after all castle owning guilds have been loaded)
	"OnAgitEnd3", //agit_end3_event_name (is executed when WoE TE has ended)
	// Timer related
	"OnTimer", //timer_event_name (is executed by a timer at the specific second)
	"OnTimerQuit", //timer_quit_event_name (is executed when a timer is aborted)
	"OnMinute", //timer_minute_event_name (is executed by a timer at the specific minute)
	"OnHour", //timer_hour_event_name (is executed by a timer at the specific hour)
	"OnClock", //timer_clock_event_name (is executed by a timer at the specific hour and minute)
	"OnDay", //timer_day_event_name (is executed by a timer at the specific month and day)
	"OnSun", //timer_sunday_event_name (is executed by a timer on sunday at the specific hour and minute)
	"OnMon", //timer_monday_event_name (is executed by a timer on monday at the specific hour and minute)
	"OnTue", //timer_tuesday_event_name (is executed by a timer on tuesday at the specific hour and minute)
	"OnWed", //timer_wednesday_event_name (is executed by a timer on wednesday at the specific hour and minute)
	"OnThu", //timer_thursday_event_name (is executed by a timer on thursday at the specific hour and minute)
	"OnFri", //timer_friday_event_name (is executed by a timer on friday at the specific hour and minute)
	"OnSat", //timer_saturday_event_name (is executed by a timer on saturday at the specific hour and minute)
	// Instance related
	"OnInstanceInit", //instance_init_event_name (is executed right after instance creation)
	"OnInstanceDestroy", //instance_destroy_event_name (is executed right before instance destruction)
};

static jmp_buf     error_jump;
static char*       error_msg;
static const char* error_pos;
static int         error_report; // if the error should produce output
// Used by disp_warning_message
static const char* parser_current_src;
static const char* parser_current_file;
static int         parser_current_line;

// for advanced scripting support ( nested if, switch, while, for, do-while, function, etc )
// [Eoe / jA 1080, 1081, 1094, 1164]
enum curly_type {
	TYPE_NULL = 0,
	TYPE_IF,
	TYPE_SWITCH,
	TYPE_WHILE,
	TYPE_FOR,
	TYPE_DO,
	TYPE_USERFUNC,
	TYPE_ARGLIST // function argument list
};

enum e_arglist
{
	ARGLIST_UNDEFINED = 0,
	ARGLIST_NO_PAREN  = 1,
	ARGLIST_PAREN     = 2,
};

static struct {
	struct {
		enum curly_type type;
		int index;
		int count;
		int flag;
		struct linkdb_node *case_label;
	} curly[256];		// Information right parenthesis
	int curly_count;	// The number of right brackets
	int index;			// Number of the syntax used in the script
} syntax;

const char* parse_curly_close(const char* p);
const char* parse_syntax_close(const char* p);
const char* parse_syntax_close_sub(const char* p,int* flag);
const char* parse_syntax(const char* p);
static int parse_syntax_for_flag = 0;

extern short current_equip_item_index; //for New CARDS Scripts. It contains Inventory Index of the EQUIP_SCRIPT caller item. [Lupus]
extern unsigned int current_equip_combo_pos;

int potion_flag=0; //For use on Alchemist improved potions/Potion Pitcher. [Skotlex]
int potion_hp=0, potion_per_hp=0, potion_sp=0, potion_per_sp=0;
int potion_target = 0;
unsigned int *generic_ui_array = NULL;
unsigned int generic_ui_array_size = 0;


c_op get_com(unsigned char *script,int *pos);
int get_num(unsigned char *script,int *pos);

typedef struct script_function {
	int (*func)(struct script_state *st);
	const char *name;
	const char *arg;
	const char *deprecated;
} script_function;

extern script_function buildin_func[];

static struct linkdb_node *sleep_db; // int oid -> struct script_state *

/*==========================================
 * (Only those needed) local declaration prototype
 *------------------------------------------*/
const char* parse_subexpr(const char* p,int limit);
int run_func(struct script_state *st);
unsigned short script_instancegetid(struct script_state *st);

const char* script_op2name(int op)
{
#define RETURN_OP_NAME(type) case type: return #type
	switch( op )
	{
	RETURN_OP_NAME(C_NOP);
	RETURN_OP_NAME(C_POS);
	RETURN_OP_NAME(C_INT);
	RETURN_OP_NAME(C_PARAM);
	RETURN_OP_NAME(C_FUNC);
	RETURN_OP_NAME(C_STR);
	RETURN_OP_NAME(C_CONSTSTR);
	RETURN_OP_NAME(C_ARG);
	RETURN_OP_NAME(C_NAME);
	RETURN_OP_NAME(C_EOL);
	RETURN_OP_NAME(C_RETINFO);
	RETURN_OP_NAME(C_USERFUNC);
	RETURN_OP_NAME(C_USERFUNC_POS);

	RETURN_OP_NAME(C_REF);

	// operators
	RETURN_OP_NAME(C_OP3);
	RETURN_OP_NAME(C_LOR);
	RETURN_OP_NAME(C_LAND);
	RETURN_OP_NAME(C_LE);
	RETURN_OP_NAME(C_LT);
	RETURN_OP_NAME(C_GE);
	RETURN_OP_NAME(C_GT);
	RETURN_OP_NAME(C_EQ);
	RETURN_OP_NAME(C_NE);
	RETURN_OP_NAME(C_XOR);
	RETURN_OP_NAME(C_OR);
	RETURN_OP_NAME(C_AND);
	RETURN_OP_NAME(C_ADD);
	RETURN_OP_NAME(C_SUB);
	RETURN_OP_NAME(C_MUL);
	RETURN_OP_NAME(C_DIV);
	RETURN_OP_NAME(C_MOD);
	RETURN_OP_NAME(C_NEG);
	RETURN_OP_NAME(C_LNOT);
	RETURN_OP_NAME(C_NOT);
	RETURN_OP_NAME(C_R_SHIFT);
	RETURN_OP_NAME(C_L_SHIFT);
	RETURN_OP_NAME(C_ADD_POST);
	RETURN_OP_NAME(C_SUB_POST);
	RETURN_OP_NAME(C_ADD_PRE);
	RETURN_OP_NAME(C_SUB_PRE);

	default:
		ShowDebug("script_op2name: unexpected op=%d\n", op);
		return "???";
	}
#undef RETURN_OP_NAME
}

#ifdef DEBUG_DUMP_STACK
static void script_dump_stack(struct script_state* st)
{
	int i;
	ShowMessage("\tstart = %d\n", st->start);
	ShowMessage("\tend   = %d\n", st->end);
	ShowMessage("\tdefsp = %d\n", st->stack->defsp);
	ShowMessage("\tsp    = %d\n", st->stack->sp);
	for( i = 0; i < st->stack->sp; ++i )
	{
		struct script_data* data = &st->stack->stack_data[i];
		ShowMessage("\t[%d] %s", i, script_op2name(data->type));
		switch( data->type )
		{
		case C_INT:
		case C_POS:
			ShowMessage(" %d\n", data->u.num);
			break;

		case C_STR:
		case C_CONSTSTR:
			ShowMessage(" \"%s\"\n", data->u.str);
			break;

		case C_NAME:
			ShowMessage(" \"%s\" (id=%d ref=%p subtype=%s)\n", reference_getname(data), data->u.num, data->ref, script_op2name(str_data[data->u.num].type));
			break;

		case C_RETINFO:
			{
				struct script_retinfo* ri = data->u.ri;
				ShowMessage(" %p {scope.vars=%p, scope.arrays=%p, script=%p, pos=%d, nargs=%d, defsp=%d}\n", ri, ri->scope.vars, ri->scope.arrays, ri->script, ri->pos, ri->nargs, ri->defsp);
			}
			break;
		default:
			ShowMessage("\n");
			break;
		}
	}
}
#endif

/// Reports on the console the src of a script error.
static void script_reportsrc(struct script_state *st)
{
	struct block_list* bl;

	if( st->oid == 0 )
		return; //Can't report source.

	bl = map_id2bl(st->oid);
	if( bl == NULL )
		return;

	switch( bl->type ) {
		case BL_NPC:
			if( bl->m >= 0 )
				ShowDebug("Source (NPC): %s at %s (%d,%d)\n", ((struct npc_data *)bl)->name, map_mapid2mapname(bl->m), bl->x, bl->y);
			else
				ShowDebug("Source (NPC): %s (invisible/not on a map)\n", ((struct npc_data *)bl)->name);
			break;
		default:
			if( bl->m >= 0 )
				ShowDebug("Source (Non-NPC type %d): name %s at %s (%d,%d)\n", bl->type, status_get_name(bl), map_mapid2mapname(bl->m), bl->x, bl->y);
			else
				ShowDebug("Source (Non-NPC type %d): name %s (invisible/not on a map)\n", bl->type, status_get_name(bl));
			break;
	}
}

/// Reports on the console information about the script data.
static void script_reportdata(struct script_data* data)
{
	if( data == NULL )
		return;
	switch( data->type ) {
		case C_NOP:// no value
			ShowDebug("Data: nothing (nil)\n");
			break;
		case C_INT:// number
			ShowDebug("Data: number value=%" PRId64 "\n", data->u.num);
			break;
		case C_STR:
		case C_CONSTSTR:// string
			if( data->u.str ) {
				ShowDebug("Data: string value=\"%s\"\n", data->u.str);
			} else {
				ShowDebug("Data: string value=NULL\n");
			}
			break;
		case C_NAME:// reference
			if( reference_tovariable(data) ) {// variable
				const char* name = reference_getname(data);
				ShowDebug("Data: variable name='%s' index=%d\n", name, reference_getindex(data));
			} else if( reference_toconstant(data) ) {// constant
				ShowDebug("Data: constant name='%s' value=%d\n", reference_getname(data), reference_getconstant(data));
			} else if( reference_toparam(data) ) {// param
				ShowDebug("Data: param name='%s' type=%d\n", reference_getname(data), reference_getparamtype(data));
			} else {// ???
				ShowDebug("Data: reference name='%s' type=%s\n", reference_getname(data), script_op2name(data->type));
				ShowDebug("Please report this!!! - script->str_data.type=%s\n", script_op2name(str_data[reference_getid(data)].type));
			}
			break;
		case C_POS:// label
			ShowDebug("Data: label pos=%" PRId64 "\n", data->u.num);
			break;
		default:
			ShowDebug("Data: %s\n", script_op2name(data->type));
			break;
	}
}


/// Reports on the console information about the current built-in function.
static void script_reportfunc(struct script_state* st)
{
	int params, id;
	struct script_data* data;

	if( !script_hasdata(st,0) )
	{// no stack
		return;
	}

	data = script_getdata(st,0);

	if( !data_isreference(data) || str_data[reference_getid(data)].type != C_FUNC )
	{// script currently not executing a built-in function or corrupt stack
		return;
	}

	id     = reference_getid(data);
	params = script_lastdata(st)-1;

	if( params > 0 )
	{
		int i;
		ShowDebug("Function: %s (%d parameter%s):\n", get_str(id), params, ( params == 1 ) ? "" : "s");

		for( i = 2; i <= script_lastdata(st); i++ )
		{
			script_reportdata(script_getdata(st,i));
		}
	}
	else
	{
		ShowDebug("Function: %s (no parameters)\n", get_str(id));
	}
}
/*==========================================
 * Output error message
 *------------------------------------------*/
static void disp_error_message2(const char *mes,const char *pos,int report)
{
	error_msg = aStrdup(mes);
	error_pos = pos;
	error_report = report;
	longjmp( error_jump, 1 );
}
#define disp_error_message(mes,pos) disp_error_message2(mes,pos,1)

static void disp_warning_message(const char *mes, const char *pos) {
	script_warning(parser_current_src,parser_current_file,parser_current_line,mes,pos);
}

/// Checks event parameter validity
static void check_event(struct script_state *st, const char *evt)
{
	if( evt && evt[0] && !stristr(evt, "::On") )
	{
		ShowWarning("NPC event parameter deprecated! Please use 'NPCNAME::OnEVENT' instead of '%s'.\n", evt);
		script_reportsrc(st);
	}
}

/*==========================================
 * Hashes the input string
 *------------------------------------------*/
static unsigned int calc_hash(const char* p)
{
	unsigned int h;

#if defined(SCRIPT_HASH_DJB2)
	h = 5381;
	while( *p ) // hash*33 + c
		h = ( h << 5 ) + h + ((unsigned char)TOLOWER(*p++));
#elif defined(SCRIPT_HASH_SDBM)
	h = 0;
	while( *p ) // hash*65599 + c
		h = ( h << 6 ) + ( h << 16 ) - h + ((unsigned char)TOLOWER(*p++));
#elif defined(SCRIPT_HASH_ELF) // UNIX ELF hash
	h = 0;
	while( *p ){
		unsigned int g;
		h = ( h << 4 ) + ((unsigned char)TOLOWER(*p++));
		g = h & 0xF0000000;
		if( g )
		{
			h ^= g >> 24;
			h &= ~g;
		}
	}
#else // athena hash
	h = 0;
	while( *p )
		h = ( h << 1 ) + ( h >> 3 ) + ( h >> 5 ) + ( h >> 8 ) + (unsigned char)TOLOWER(*p++);
#endif

	return h % SCRIPT_HASH_SIZE;
}

bool script_check_RegistryVariableLength(int pType, const char *val, size_t* vlen) 
{
	size_t len = strlen(val);

	if (vlen)
		*vlen = len;
	switch (pType) {
		case 0:
			return (len < 33); // key check
		case 1:
			return (len < 255); // value check
		default:
			return false;
	}
}

/*==========================================
 * str_data manipulation functions
 *------------------------------------------*/

/// Looks up string using the provided id.
const char* get_str(int id)
{
	Assert( id >= LABEL_START && id < str_size );
	return str_buf+str_data[id].str;
}

/// Returns the uid of the string, or -1.
static int search_str(const char* p)
{
	int i;

	for( i = str_hash[calc_hash(p)]; i != 0; i = str_data[i].next )
		if( strcasecmp(get_str(i),p) == 0 )
			return i;

	return -1;
}

/// Stores a copy of the string and returns its id.
/// If an identical string is already present, returns its id instead.
int add_str(const char* p)
{
	int h;
	int len;

	h = calc_hash(p);

	if( str_hash[h] == 0 )
	{// empty bucket, add new node here
		str_hash[h] = str_num;
	}
	else
	{// scan for end of list, or occurence of identical string
		int i;
		for( i = str_hash[h]; ; i = str_data[i].next )
		{
			if( strcasecmp(get_str(i),p) == 0 )
				return i; // string already in list
			if( str_data[i].next == 0 )
				break; // reached the end
		}

		// append node to end of list
		str_data[i].next = str_num;
	}

	// grow list if neccessary
	if( str_num >= str_data_size )
	{
		str_data_size += 128;
		RECREATE(str_data,struct str_data_struct,str_data_size);
		memset(str_data + (str_data_size - 128), '\0', 128);
	}

	len=(int)strlen(p);

	// grow string buffer if neccessary
	while( str_pos+len+1 >= str_size )
	{
		str_size += 256;
		RECREATE(str_buf,char,str_size);
		memset(str_buf + (str_size - 256), '\0', 256);
	}

	safestrncpy(str_buf+str_pos, p, len+1);
	str_data[str_num].type = C_NOP;
	str_data[str_num].str = str_pos;
	str_data[str_num].next = 0;
	str_data[str_num].func = NULL;
	str_data[str_num].backpatch = -1;
	str_data[str_num].label = -1;
	str_pos += len+1;

	return str_num++;
}


/// Appends 1 byte to the script buffer.
static void add_scriptb(int a)
{
	if( script_pos+1 >= script_size )
	{
		script_size += SCRIPT_BLOCK_SIZE;
		RECREATE(script_buf,unsigned char,script_size);
	}
	script_buf[script_pos++] = (uint8)(a);
}

/// Appends a c_op value to the script buffer.
/// The value is variable-length encoded into 8-bit blocks.
/// The encoding scheme is ( 01?????? )* 00??????, LSB first.
/// All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
static void add_scriptc(int a)
{
	while( a >= 0x40 )
	{
		add_scriptb((a&0x3f)|0x40);
		a = (a - 0x40) >> 6;
	}

	add_scriptb(a);
}

/// Appends an integer value to the script buffer.
/// The value is variable-length encoded into 8-bit blocks.
/// The encoding scheme is ( 11?????? )* 10??????, LSB first.
/// All blocks but the last hold 7 bits of data, topmost bit is always 1 (carries).
static void add_scripti(int a)
{
	while( a >= 0x40 )
	{
		add_scriptb((a&0x3f)|0xc0);
		a = (a - 0x40) >> 6;
	}
	add_scriptb(a|0x80);
}

/// Appends a str_data object (label/function/variable/integer) to the script buffer.

///
/// @param l The id of the str_data entry
// Maximum up to 16M
static void add_scriptl(int l)
{
	int backpatch = str_data[l].backpatch;

	switch(str_data[l].type){
	case C_POS:
	case C_USERFUNC_POS:
		add_scriptc(C_POS);
		add_scriptb(str_data[l].label);
		add_scriptb(str_data[l].label>>8);
		add_scriptb(str_data[l].label>>16);
		break;
	case C_NOP:
	case C_USERFUNC:
		// Embedded data backpatch there is a possibility of label
		add_scriptc(C_NAME);
		str_data[l].backpatch = script_pos;
		add_scriptb(backpatch);
		add_scriptb(backpatch>>8);
		add_scriptb(backpatch>>16);
		break;
	case C_INT:
		add_scripti(abs(str_data[l].val));
		if( str_data[l].val < 0 ) //Notice that this is negative, from jA (Rayce)
			add_scriptc(C_NEG);
		break;
	default: // assume C_NAME
		add_scriptc(C_NAME);
		add_scriptb(l);
		add_scriptb(l>>8);
		add_scriptb(l>>16);
		break;
	}
}

/*==========================================
 * Resolve the label
 *------------------------------------------*/
void set_label(int l,int pos, const char* script_pos_cur)
{
	int i;

	if(str_data[l].type==C_INT || str_data[l].type==C_PARAM || str_data[l].type==C_FUNC)
	{	//Prevent overwriting constants values, parameters and built-in functions [Skotlex]
		disp_error_message("set_label: invalid label name",script_pos_cur);
		return;
	}
	if(str_data[l].label!=-1){
		disp_error_message("set_label: dup label ",script_pos_cur);
		return;
	}
	str_data[l].type=(str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
	str_data[l].label=pos;
	for(i=str_data[l].backpatch;i>=0 && i!=0x00ffffff;){
		int next=GETVALUE(script_buf,i);
		script_buf[i-1]=(str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
		SETVALUE(script_buf,i,pos);
		i=next;
	}
}

/// Skips spaces and/or comments.
const char* skip_space(const char* p)
{
	if( p == NULL )
		return NULL;
	for(;;)
	{
		while( ISSPACE(*p) )
			++p;
		if( *p == '/' && p[1] == '/' )
		{// line comment
			while(*p && *p!='\n')
				++p;
		}
		else if( *p == '/' && p[1] == '*' )
		{// block comment
			p += 2;
			for(;;)
			{
				if( *p == '\0' ) {
					disp_warning_message("script:script->skip_space: end of file while parsing block comment. expected " CL_BOLD "*/" CL_NORM, p);
					return p;
				}
				if( *p == '*' && p[1] == '/' )
				{// end of block comment
					p += 2;
					break;
				}
				++p;
			}
		}
		else
			break;
	}
	return p;
}

/// Skips a word.
/// A word consists of undercores and/or alphanumeric characters,
/// and valid variable prefixes/postfixes.
static const char* skip_word(const char* p)
{
	// prefix
	switch( *p ) {
		case '@':// temporary char variable
			++p; break;
		case '#':// account variable
			p += ( p[1] == '#' ? 2 : 1 ); break;
		case '\'':// instance variable
			++p; break;
		case '.':// npc variable
			p += ( p[1] == '@' ? 2 : 1 ); break;
		case '$':// global variable
			p += ( p[1] == '@' ? 2 : 1 ); break;
	}

	while( ISALNUM(*p) || *p == '_' )
		++p;

	// postfix
	if( *p == '$' )// string
		p++;

	return p;
}

/// Adds a word to str_data.
/// @see skip_word
/// @see add_str
static int add_word(const char* p)
{
	char* word;
	int len;
	int i;

	// Check for a word
	len = skip_word(p) - p;
	if( len == 0 )
		disp_error_message("script:add_word: invalid word. A word consists of undercores and/or alphanumeric characters, and valid variable prefixes/postfixes.", p);

	// Duplicate the word
	word = (char*)aMalloc(len+1);
	memcpy(word, p, len);
	word[len] = 0;

	// add the word
	i = add_str(word);
	aFree(word);
	return i;
}

/// Parses a function call.
/// The argument list can have parenthesis or not.
/// The number of arguments is checked.
static
const char* parse_callfunc(const char* p, int require_paren, int is_custom)
{
	const char* p2;
	const char* arg=NULL;
	int func;

	func = add_word(p);
	if( str_data[func].type == C_FUNC ){
		// buildin function
		add_scriptl(func);
		add_scriptc(C_ARG);
		arg = buildin_func[str_data[func].val].arg;
#if defined(SCRIPT_COMMAND_DEPRECATION)
		if( str_data[func].deprecated ){
			ShowWarning( "Usage of deprecated script function '%s'.\n", get_str(func) );
			ShowWarning( "This function was deprecated on '%s' and could become unavailable anytime soon.\n", buildin_func[str_data[func].val].deprecated );
		}
#endif
	} else if( str_data[func].type == C_USERFUNC || str_data[func].type == C_USERFUNC_POS ){
		// script defined function
		add_scriptl(buildin_callsub_ref);
		add_scriptc(C_ARG);
		add_scriptl(func);
		arg = buildin_func[str_data[buildin_callsub_ref].val].arg;
		if( *arg == 0 )
			disp_error_message("parse_callfunc: callsub has no arguments, please review its definition",p);
		if( *arg != '*' )
			++arg; // count func as argument
	} else {
		const char* name = get_str(func);
		if( !is_custom && strdb_get(userfunc_db, name) == NULL ) {
			disp_error_message("parse_line: expect command, missing function name or calling undeclared function",p);
		} else {;
			add_scriptl(buildin_callfunc_ref);
			add_scriptc(C_ARG);
			add_scriptc(C_STR);
			while( *name ) add_scriptb(*name ++);
			add_scriptb(0);
			arg = buildin_func[str_data[buildin_callfunc_ref].val].arg;
			if( *arg != '*' ) ++ arg;
		}
	}

	p = skip_word(p);
	p = skip_space(p);
	syntax.curly[syntax.curly_count].type = TYPE_ARGLIST;
	syntax.curly[syntax.curly_count].count = 0;
	if( *p == ';' )
	{// <func name> ';'
		syntax.curly[syntax.curly_count].flag = ARGLIST_NO_PAREN;
	} else if( *p == '(' && *(p2=skip_space(p+1)) == ')' )
	{// <func name> '(' ')'
		syntax.curly[syntax.curly_count].flag = ARGLIST_PAREN;
		p = p2;
	/*
	} else if( 0 && require_paren && *p != '(' )
	{// <func name>
		syntax.curly[syntax.curly_count].flag = ARGLIST_NO_PAREN;
	*/
	} else
	{// <func name> <arg list>
		if( require_paren ){
			if( *p != '(' )
				disp_error_message("need '('",p);
			++p; // skip '('
			syntax.curly[syntax.curly_count].flag = ARGLIST_PAREN;
		} else if( *p == '(' ){
			syntax.curly[syntax.curly_count].flag = ARGLIST_UNDEFINED;
		} else {
			syntax.curly[syntax.curly_count].flag = ARGLIST_NO_PAREN;
		}
		++syntax.curly_count;
		while( *arg ) {
			p2=parse_subexpr(p,-1);
			if( p == p2 )
				break; // not an argument
			if( *arg != '*' )
				++arg; // next argument

			p=skip_space(p2);
			if( *arg == 0 || *p != ',' )
				break; // no more arguments
			++p; // skip comma
		}
		--syntax.curly_count;
	}
	if( *arg && *arg != '?' && *arg != '*' )
		disp_error_message2("parse_callfunc: not enough arguments, expected ','", p, script_config.warn_func_mismatch_paramnum);
	if( syntax.curly[syntax.curly_count].type != TYPE_ARGLIST )
		disp_error_message("parse_callfunc: DEBUG last curly is not an argument list",p);
	if( syntax.curly[syntax.curly_count].flag == ARGLIST_PAREN ){
		if( *p != ')' )
			disp_error_message("parse_callfunc: expected ')' to close argument list",p);
		++p;
	}
	add_scriptc(C_FUNC);
	return p;
}

/// Processes end of logical script line.
/// @param first When true, only fix up scheduling data is initialized
/// @param p Script position for error reporting in set_label
static void parse_nextline(bool first, const char* p)
{
	if( !first )
	{
		add_scriptc(C_EOL);  // mark end of line for stack cleanup
		set_label(LABEL_NEXTLINE, script_pos, p);  // fix up '-' labels
	}

	// initialize data for new '-' label fix up scheduling
	str_data[LABEL_NEXTLINE].type      = C_NOP;
	str_data[LABEL_NEXTLINE].backpatch = -1;
	str_data[LABEL_NEXTLINE].label     = -1;
}

/**
 * Pushes a variable into stack, processing its array index if needed.
 * @see parse_variable
 */
void parse_variable_sub_push(int word, const char *p2)
{
	if( p2 ) { // Process the variable index
		const char *p3 = NULL;

		// Push the getelementofarray method into the stack
		add_scriptl(buildin_getelementofarray_ref);
		add_scriptc(C_ARG);
		add_scriptl(word);

		// Process the sub-expression for this assignment
		p3 = parse_subexpr(p2 + 1, 1);
		p3 = skip_space(p3);

		if( *p3 != ']' ) // Closing parenthesis is required for this script
			disp_error_message("Missing closing ']' parenthesis for the variable assignment.", p3);

		// Push the closing function stack operator onto the stack
		add_scriptc(C_FUNC);
		p3++;
	} else // No array index, simply push the variable or value onto the stack
		add_scriptl(word);
}

/// Parse a variable assignment using the direct equals operator
/// @param p script position where the function should run from
/// @return NULL if not a variable assignment, the new position otherwise
const char* parse_variable(const char* p) {
	int word;
	c_op type = C_NOP;
	const char *p2 = NULL;
	const char *var = p;

	if ((p[0] == '+' && p[1] == '+' && (type = C_ADD_PRE)) // pre ++
	 || (p[0] == '-' && p[1] == '-' && (type = C_SUB_PRE))) // pre --
		var = p = skip_space(&p[2]);

	// skip the variable where applicable
	p = skip_word(p);
	p = skip_space(p);

	if( p == NULL ) {// end of the line or invalid buffer
		return NULL;
	}

	if( *p == '[' ) {// array variable so process the array as appropriate
		int i,j;
		for( p2 = p, i = 0, j = 1; p; ++ i ) {
			if( *p ++ == ']' && --(j) == 0 ) break;
			if( *p == '[' ) ++ j;
		}

		if( !(p = skip_space(p)) ) {// end of line or invalid characters remaining
			disp_error_message("Missing right expression or closing bracket for variable.", p);
		}
	}

	if( type == C_NOP &&
	!( ( p[0] == '=' && p[1] != '=' && (type = C_EQ) ) // =
	|| ( p[0] == '+' && p[1] == '=' && (type = C_ADD) ) // +=
	|| ( p[0] == '-' && p[1] == '=' && (type = C_SUB) ) // -=
	|| ( p[0] == '^' && p[1] == '=' && (type = C_XOR) ) // ^=
	|| ( p[0] == '|' && p[1] == '=' && (type = C_OR ) ) // |=
	|| ( p[0] == '&' && p[1] == '=' && (type = C_AND) ) // &=
	|| ( p[0] == '*' && p[1] == '=' && (type = C_MUL) ) // *=
	|| ( p[0] == '/' && p[1] == '=' && (type = C_DIV) ) // /=
	|| ( p[0] == '%' && p[1] == '=' && (type = C_MOD) ) // %=
	|| ( p[0] == '~' && p[1] == '=' && (type = C_NOT) ) // ~=
	|| ( p[0] == '+' && p[1] == '+' && (type = C_ADD_POST) ) // post ++
	|| ( p[0] == '-' && p[1] == '-' && (type = C_SUB_POST) ) // post --
	|| ( p[0] == '<' && p[1] == '<' && p[2] == '=' && (type = C_L_SHIFT) ) // <<=
	|| ( p[0] == '>' && p[1] == '>' && p[2] == '=' && (type = C_R_SHIFT) ) // >>=
	) )
	{// failed to find a matching operator combination so invalid
		return NULL;
	}

	switch( type ) {
		case C_ADD_PRE: // pre ++
		case C_SUB_PRE: // pre --
			// Nothing more to skip
		break;

		case C_EQ: {// incremental modifier
			p = skip_space( &p[1] );
		}
		break;

		case C_L_SHIFT:
		case C_R_SHIFT: {// left or right shift modifier
			p = skip_space( &p[3] );
		}
		break;

		default: {// normal incremental command
			p = skip_space( &p[2] );
		}
	}

	if( p == NULL ) {// end of line or invalid buffer
		return NULL;
	}

	// push the set function onto the stack
	add_scriptl(buildin_set_ref);
	add_scriptc(C_ARG);

	// always append parenthesis to avoid errors
	syntax.curly[syntax.curly_count].type = TYPE_ARGLIST;
	syntax.curly[syntax.curly_count].count = 0;
	syntax.curly[syntax.curly_count].flag = ARGLIST_PAREN;

	// increment the total curly count for the position in the script
	++ syntax.curly_count;

	// parse the variable currently being modified
	word = add_word(var);

	if( str_data[word].type == C_FUNC || str_data[word].type == C_USERFUNC || str_data[word].type == C_USERFUNC_POS ) // cannot assign a variable which exists as a function or label
		disp_error_message("Cannot modify a variable which has the same name as a function or label.", p);

	parse_variable_sub_push(word, p2); // Push variable onto the stack

	if( type != C_EQ )
		parse_variable_sub_push(word, p2); // Push variable onto the stack once again (first argument of setr)

	if( type == C_ADD_POST || type == C_SUB_POST ) { // post ++ / --
		add_scripti(1);
		add_scriptc(type == C_ADD_POST ? C_ADD : C_SUB);
		parse_variable_sub_push(word, p2); // Push variable onto the stack (third argument of setr)
	} else if( type == C_ADD_PRE || type == C_SUB_PRE ) { // pre ++ / --
		add_scripti(1);
		add_scriptc(type == C_ADD_PRE ? C_ADD : C_SUB);
	} else { // Process the value as an expression
		p = parse_subexpr(p, -1);

		if( type != C_EQ )
		{// push the type of modifier onto the stack
			add_scriptc(type);
		}
	}

	// decrement the curly count for the position within the script
	-- syntax.curly_count;

	// close the script by appending the function operator
	add_scriptc(C_FUNC);

	// push the buffer from the method
	return p;
}

/*
 * Checks whether the gives string is a number literal
 *
 * Mainly necessary to differentiate between number literals and NPC name
 * constants, since several of those start with a digit.
 *
 * All this does is to check if the string begins with an optional + or - sign,
 * followed by a hexadecimal or decimal number literal literal and is NOT
 * followed by a underscore or letter.
 *
 * @author : Hercules.ws
 * @param p Pointer to the string to check
 * @return Whether the string is a number literal
 */
bool is_number(const char *p) {
	const char *np;
	if (!p)
		return false;
	if (*p == '-' || *p == '+')
		p++;
	np = p;
	if (*p == '0' && p[1] == 'x') {
		p+=2;
		np = p;
		// Hexadecimal
		while (ISXDIGIT(*np))
			np++;
	} else {
		// Decimal
		while (ISDIGIT(*np))
			np++;
	}
	if (p != np && *np != '_' && !ISALPHA(*np)) // At least one digit, and next isn't a letter or _
		return true;
	return false;
}

/*==========================================
 * Analysis section
 *------------------------------------------*/
const char* parse_simpleexpr(const char *p)
{
	long long i;
	p=skip_space(p);

	if(*p==';' || *p==',')
		disp_error_message("parse_simpleexpr: unexpected end of expression",p);
	if(*p=='('){
		if( (i=syntax.curly_count-1) >= 0 && syntax.curly[i].type == TYPE_ARGLIST )
			++syntax.curly[i].count;
		p=parse_subexpr(p+1,-1);
		p=skip_space(p);
		if( (i=syntax.curly_count-1) >= 0 && syntax.curly[i].type == TYPE_ARGLIST &&
				syntax.curly[i].flag == ARGLIST_UNDEFINED && --syntax.curly[i].count == 0
		){
			if( *p == ',' ){
				syntax.curly[i].flag = ARGLIST_PAREN;
				return p;
			} else
				syntax.curly[i].flag = ARGLIST_NO_PAREN;
		}
		if( *p != ')' )
			disp_error_message("parse_simpleexpr: unmatched ')'",p);
		++p;
	} else if(is_number(p)) {
		char *np;
		while(*p == '0' && ISDIGIT(p[1])) p++;
		i=strtoll(p,&np,0);
		if( i < INT_MIN ) {
			i = INT_MIN;
			disp_warning_message("parse_simpleexpr: underflow detected, capping value to INT_MIN",p);
		} else if( i > INT_MAX ) {
			i = INT_MAX;
			disp_warning_message("parse_simpleexpr: overflow detected, capping value to INT_MAX",p);
		}
		add_scripti((int)i);
		p=np;
	} else if(*p=='"'){
		add_scriptc(C_STR);
		p++;
		while( *p && *p != '"' ){
			if( (unsigned char)p[-1] <= 0x7e && *p == '\\' )
			{
				char buf[8];
				size_t len = skip_escaped_c(p) - p;
				size_t n = sv_unescape_c(buf, p, len);
				if( n != 1 )
					ShowDebug("parse_simpleexpr: unexpected length %d after unescape (\"%.*s\" -> %.*s)\n", (int)n, (int)len, p, (int)n, buf);
				p += len;
				add_scriptb(*buf);
				continue;
			}
			else if( *p == '\n' )
				disp_error_message("parse_simpleexpr: unexpected newline @ string",p);
			add_scriptb(*p++);
		}
		if(!*p)
			disp_error_message("parse_simpleexpr: unexpected eof @ string",p);
		add_scriptb(0);
		p++;	//'"'
	} else {
		int l;
		const char* pv;

		// label , register , function etc
		if(skip_word(p)==p)
			disp_error_message("parse_simpleexpr: unexpected character",p);

		l=add_word(p);
		if( str_data[l].type == C_FUNC || str_data[l].type == C_USERFUNC || str_data[l].type == C_USERFUNC_POS)
			return parse_callfunc(p,1,0);
		else {
			const char* name = get_str(l);
			if( strdb_get(userfunc_db,name) != NULL ) {
				return parse_callfunc(p,1,1);
			}
		}

		if( (pv = parse_variable(p)) )
		{// successfully processed a variable assignment
			return pv;
		}

#if defined(SCRIPT_CONSTANT_DEPRECATION)
		if( str_data[l].type == C_INT && str_data[l].deprecated ){
			ShowWarning( "Usage of deprecated constant '%s'.\n", get_str(l) );
			ShowWarning( "This constant was deprecated and could become unavailable anytime soon.\n" );
		}
#endif

		p=skip_word(p);
		if( *p == '[' ){
			// array(name[i] => getelementofarray(name,i) )
			add_scriptl(buildin_getelementofarray_ref);
			add_scriptc(C_ARG);
			add_scriptl(l);

			p=parse_subexpr(p+1,-1);
			p=skip_space(p);
			if( *p != ']' )
				disp_error_message("parse_simpleexpr: unmatched ']'",p);
			++p;
			add_scriptc(C_FUNC);
		}else
			add_scriptl(l);

	}

	return p;
}

/*==========================================
 * Analysis of the expression
 *------------------------------------------*/
const char* parse_subexpr(const char* p,int limit)
{
	int op,opl,len;

	p=skip_space(p);

	if( *p == '-' ){
		const char* tmpp = skip_space(p+1);
		if( *tmpp == ';' || *tmpp == ',' ){
			add_scriptl(LABEL_NEXTLINE);
			p++;
			return p;
		}
	}

	if( (op = C_ADD_PRE, p[0] == '+' && p[1] == '+') || (op = C_SUB_PRE, p[0] == '-' && p[1] == '-') ) // Pre ++ -- operators
		p = parse_variable(p);
	else if( (op = C_NEG, *p == '-') || (op = C_LNOT, *p == '!') || (op = C_NOT, *p == '~') ) { // Unary - ! ~ operators
		p = parse_subexpr(p + 1, 11);
		add_scriptc(op);
	} else
		p = parse_simpleexpr(p);
	p = skip_space(p);
	while((
			(op=C_OP3,opl=0,len=1,*p=='?') ||
			((op=C_ADD,opl=9,len=1,*p=='+') && p[1]!='+') ||
			((op=C_SUB,opl=9,len=1,*p=='-') && p[1]!='-') ||
			(op=C_MUL,opl=10,len=1,*p=='*') ||
			(op=C_DIV,opl=10,len=1,*p=='/') ||
			(op=C_MOD,opl=10,len=1,*p=='%') ||
			(op=C_LAND,opl=2,len=2,*p=='&' && p[1]=='&') ||
			(op=C_AND,opl=5,len=1,*p=='&') ||
			(op=C_LOR,opl=1,len=2,*p=='|' && p[1]=='|') ||
			(op=C_OR,opl=3,len=1,*p=='|') ||
			(op=C_XOR,opl=4,len=1,*p=='^') ||
			(op=C_EQ,opl=6,len=2,*p=='=' && p[1]=='=') ||
			(op=C_NE,opl=6,len=2,*p=='!' && p[1]=='=') ||
			(op=C_R_SHIFT,opl=8,len=2,*p=='>' && p[1]=='>') ||
			(op=C_GE,opl=7,len=2,*p=='>' && p[1]=='=') ||
			(op=C_GT,opl=7,len=1,*p=='>') ||
			(op=C_L_SHIFT,opl=8,len=2,*p=='<' && p[1]=='<') ||
			(op=C_LE,opl=7,len=2,*p=='<' && p[1]=='=') ||
			(op=C_LT,opl=7,len=1,*p=='<')) && opl>limit){
		p+=len;
		if(op == C_OP3) {
			p=parse_subexpr(p,-1);
			p=skip_space(p);
			if( *(p++) != ':')
				disp_error_message("parse_subexpr: expected ':'", p-1);
			p=parse_subexpr(p,-1);
		} else {
			p=parse_subexpr(p,opl);
		}
		add_scriptc(op);
		p=skip_space(p);
	}

	return p;  /* return first untreated operator */
}

/*==========================================
 * Evaluation of the expression
 *------------------------------------------*/
const char* parse_expr(const char *p)
{
	switch(*p){
	case ')': case ';': case ':': case '[': case ']':
	case '}':
		disp_error_message("parse_expr: unexpected character",p);
	}
	p=parse_subexpr(p,-1);
	return p;
}

/*==========================================
 * Analysis of the line
 *------------------------------------------*/
const char* parse_line(const char* p)
{
	const char* p2;

	p=skip_space(p);
	if(*p==';') {
		//Close decision for if(); for(); while();
		p = parse_syntax_close(p + 1);
		return p;
	}
	if(*p==')' && parse_syntax_for_flag)
		return p+1;

	p = skip_space(p);
	if(p[0] == '{') {
		syntax.curly[syntax.curly_count].type  = TYPE_NULL;
		syntax.curly[syntax.curly_count].count = -1;
		syntax.curly[syntax.curly_count].index = -1;
		syntax.curly_count++;
		return p + 1;
	} else if(p[0] == '}') {
		return parse_curly_close(p);
	}

	// Syntax-related processing
	p2 = parse_syntax(p);
	if(p2 != NULL)
		return p2;

	// attempt to process a variable assignment
	p2 = parse_variable(p);

	if( p2 != NULL )
	{// variable assignment processed so leave the method
		return parse_syntax_close(p2 + 1);
	}

	p = parse_callfunc(p,0,0);
	p = skip_space(p);

	if(parse_syntax_for_flag) {
		if( *p != ')' )
			disp_error_message("parse_line: expected ')'",p);
	} else {
		if( *p != ';' )
			disp_error_message("parse_line: expected ';'",p);
	}

	//Binding decision for if(), for(), while()
	p = parse_syntax_close(p+1);

	return p;
}

// { ... } Closing process
const char* parse_curly_close(const char* p)
{
	if(syntax.curly_count <= 0) {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_NULL) {
		syntax.curly_count--;
		//Close decision  if, for , while
		p = parse_syntax_close(p + 1);
		return p;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_SWITCH) {
		//Closing switch()
		int pos = syntax.curly_count-1;
		char label[256];
		int l;
		// Remove temporary variables
		sprintf(label,"set $@__SW%x_VAL,0;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// Go to the end pointer unconditionally
		sprintf(label,"goto __SW%x_FIN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// You are here labeled
		sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
		l=add_str(label);
		set_label(l,script_pos, p);

		if(syntax.curly[pos].flag) {
			//Exists default
			sprintf(label,"goto __SW%x_DEF;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;
		}

		// Label end
		sprintf(label,"__SW%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos, p);
		linkdb_final(&syntax.curly[pos].case_label);	// free the list of case label
		syntax.curly_count--;
		//Closing decision if, for , while
		p = parse_syntax_close(p + 1);
		return p;
	} else {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	}
}

// Syntax-related processing
//	 break, case, continue, default, do, for, function,
//	 if, switch, while ? will handle this internally.
const char* parse_syntax(const char* p)
{
	const char *p2 = skip_word(p);

	switch(*p) {
	case 'B':
	case 'b':
		if(p2 - p == 5 && !strncasecmp(p,"break",5)) {
			// break Processing
			char label[256];
			int pos = syntax.curly_count - 1;
			while(pos >= 0) {
				if(syntax.curly[pos].type == TYPE_DO) {
					sprintf(label,"goto __DO%x_FIN;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_FOR) {
					sprintf(label,"goto __FR%x_FIN;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_WHILE) {
					sprintf(label,"goto __WL%x_FIN;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_SWITCH) {
					sprintf(label,"goto __SW%x_FIN;",syntax.curly[pos].index);
					break;
				}
				pos--;
			}
			if(pos < 0) {
				disp_error_message("parse_syntax: unexpected 'break'",p);
			} else {
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				parse_line(label);
				syntax.curly_count--;
			}
			p = skip_space(p2);
			if(*p != ';')
				disp_error_message("parse_syntax: expected ';'",p);
			// Closing decision if, for , while
			p = parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'c':
	case 'C':
		if(p2 - p == 4 && !strncasecmp(p,"case",4)) {
			//Processing case
			int pos = syntax.curly_count-1;
			if(pos < 0 || syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'case' ",p);
				return p+1;
			} else {
				char label[256];
				int  l,v;
				char *np;
				if(syntax.curly[pos].count != 1) {
					//Jump for FALLTHRU
					sprintf(label,"goto __SW%x_%xJ;",syntax.curly[pos].index,syntax.curly[pos].count);
					syntax.curly[syntax.curly_count++].type = TYPE_NULL;
					parse_line(label);
					syntax.curly_count--;

					// You are here labeled
					sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
					l=add_str(label);
					set_label(l,script_pos, p);
				}
				//Decision statement switch
				p = skip_space(p2);
				if(p == p2) {
					disp_error_message("parse_syntax: expected a space ' '",p);
				}
				// check whether case label is integer or not
				if(is_number(p)) {
					//Numeric value
					v = (int)strtol(p,&np,0);
					if((*p == '-' || *p == '+') && ISDIGIT(p[1])) // pre-skip because '-' can not skip_word
						p++;
					p = skip_word(p);
					if(np != p)
						disp_error_message("parse_syntax: 'case' label is not an integer",np);
				} else {
					//Check for constants
					p2 = skip_word(p);
					v = (int)(size_t) (p2-p); // length of word at p2
					memcpy(label,p,v);
					label[v]='\0';
					if( !script_get_constant(label, &v) )
						disp_error_message("parse_syntax: 'case' label is not an integer",p);
					p = skip_word(p);
				}
				p = skip_space(p);
				if(*p != ':')
					disp_error_message("parse_syntax: expect ':'",p);
				sprintf(label,"if(%d != $@__SW%x_VAL) goto __SW%x_%x;",
					v,syntax.curly[pos].index,syntax.curly[pos].index,syntax.curly[pos].count+1);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				// Bad I do not parse twice
				p2 = parse_line(label);
				parse_line(p2);
				syntax.curly_count--;
				if(syntax.curly[pos].count != 1) {
					// Label after the completion of FALLTHRU
					sprintf(label,"__SW%x_%xJ",syntax.curly[pos].index,syntax.curly[pos].count);
					l=add_str(label);
					set_label(l,script_pos,p);
				}
				// check duplication of case label [Rayce]
				if(linkdb_search(&syntax.curly[pos].case_label, (void*)__64BPRTSIZE(v)) != NULL)
					disp_error_message("parse_syntax: dup 'case'",p);
				linkdb_insert(&syntax.curly[pos].case_label, (void*)__64BPRTSIZE(v), (void*)1);

				sprintf(label,"set $@__SW%x_VAL,0;",syntax.curly[pos].index);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;

				parse_line(label);
				syntax.curly_count--;
				syntax.curly[pos].count++;
			}
			return p + 1;
		} else if(p2 - p == 8 && !strncasecmp(p,"continue",8)) {
			// Processing continue
			char label[256];
			int pos = syntax.curly_count - 1;
			while(pos >= 0) {
				if(syntax.curly[pos].type == TYPE_DO) {
					sprintf(label,"goto __DO%x_NXT;",syntax.curly[pos].index);
					syntax.curly[pos].flag = 1; //Flag put the link for continue
					break;
				} else if(syntax.curly[pos].type == TYPE_FOR) {
					sprintf(label,"goto __FR%x_NXT;",syntax.curly[pos].index);
					break;
				} else if(syntax.curly[pos].type == TYPE_WHILE) {
					sprintf(label,"goto __WL%x_NXT;",syntax.curly[pos].index);
					break;
				}
				pos--;
			}
			if(pos < 0) {
				disp_error_message("parse_syntax: unexpected 'continue'",p);
			} else {
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				parse_line(label);
				syntax.curly_count--;
			}
			p = skip_space(p2);
			if(*p != ';')
				disp_error_message("parse_syntax: expected ';'",p);
			//Closing decision if, for , while
			p = parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'd':
	case 'D':
		if(p2 - p == 7 && !strncasecmp(p,"default",7)) {
			// Switch - default processing
			int pos = syntax.curly_count-1;
			if(pos < 0 || syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'default'",p);
			} else if(syntax.curly[pos].flag) {
				disp_error_message("parse_syntax: dup 'default'",p);
			} else {
				char label[256];
				int l;
				// Put the label location
				p = skip_space(p2);
				if(*p != ':') {
					disp_error_message("parse_syntax: expected ':'",p);
				}
				sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
				l=add_str(label);
				set_label(l,script_pos,p);

				// Skip to the next link w/o condition
				sprintf(label,"goto __SW%x_%x;",syntax.curly[pos].index,syntax.curly[pos].count+1);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				parse_line(label);
				syntax.curly_count--;

				// The default label
				sprintf(label,"__SW%x_DEF",syntax.curly[pos].index);
				l=add_str(label);
				set_label(l,script_pos,p);

				syntax.curly[syntax.curly_count - 1].flag = 1;
				syntax.curly[pos].count++;
			}
			return p + 1;
		} else if(p2 - p == 2 && !strncasecmp(p,"do",2)) {
			int l;
			char label[256];
			p=skip_space(p2);

			syntax.curly[syntax.curly_count].type  = TYPE_DO;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			// Label of the (do) form here
			sprintf(label,"__DO%x_BGN",syntax.curly[syntax.curly_count].index);
			l=add_str(label);
			set_label(l,script_pos,p);
			syntax.curly_count++;
			return p;
		}
		break;
	case 'f':
	case 'F':
		if(p2 - p == 3 && !strncasecmp(p,"for",3)) {
			int l;
			char label[256];
			int  pos = syntax.curly_count;
			syntax.curly[syntax.curly_count].type  = TYPE_FOR;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			syntax.curly_count++;

			p=skip_space(p2);

			if(*p != '(')
				disp_error_message("parse_syntax: expected '('",p);
			p++;

			// Execute the initialization statement
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			p=parse_line(p);
			syntax.curly_count--;

			// Form the start of label decision
			sprintf(label,"__FR%x_J",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			p=skip_space(p);
			if(*p == ';') {
				// For (; Because the pattern of always true ;)
				;
			} else {
				// Skip to the end point if the condition is false
				sprintf(label,"__FR%x_FIN",syntax.curly[pos].index);
				add_scriptl(add_str("jump_zero"));
				add_scriptc(C_ARG);
				p=parse_expr(p);
				p=skip_space(p);
				add_scriptl(add_str(label));
				add_scriptc(C_FUNC);
			}
			if(*p != ';')
				disp_error_message("parse_syntax: expected ';'",p);
			p++;

			// Skip to the beginning of the loop
			sprintf(label,"goto __FR%x_BGN;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;

			// Labels to form the next loop
			sprintf(label,"__FR%x_NXT",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			// Process the next time you enter the loop
			// A ')' last for; flag to be treated as'
			parse_syntax_for_flag = 1;
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			p=parse_line(p);
			syntax.curly_count--;
			parse_syntax_for_flag = 0;

			// Skip to the determination process conditions
			sprintf(label,"goto __FR%x_J;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;

			// Loop start labeling
			sprintf(label,"__FR%x_BGN",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);
			return p;
		}
		else if( p2 - p == 8 && strncasecmp(p,"function",8) == 0 )
		{// internal script function
			const char *func_name;

			func_name = skip_space(p2);
			p = skip_word(func_name);
			if( p == func_name )
				disp_error_message("parse_syntax:function: function name is missing or invalid", p);
			p2 = skip_space(p);
			if( *p2 == ';' )
			{// function <name> ;
				// function declaration - just register the name
				int l;
				l = add_word(func_name);
				if( str_data[l].type == C_NOP )// register only, if the name was not used by something else
					str_data[l].type = C_USERFUNC;
				else if( str_data[l].type == C_USERFUNC )
					;  // already registered
				else
					disp_error_message("parse_syntax:function: function name is invalid", func_name);

				// Close condition of if, for, while
				p = parse_syntax_close(p2 + 1);
				return p;
			}
			else if(*p2 == '{')
			{// function <name> <line/block of code>
				char label[256];
				int l;

				syntax.curly[syntax.curly_count].type  = TYPE_USERFUNC;
				syntax.curly[syntax.curly_count].count = 1;
				syntax.curly[syntax.curly_count].index = syntax.index++;
				syntax.curly[syntax.curly_count].flag  = 0;
				++syntax.curly_count;

				// Jump over the function code
				sprintf(label, "goto __FN%x_FIN;", syntax.curly[syntax.curly_count-1].index);
				syntax.curly[syntax.curly_count].type = TYPE_NULL;
				++syntax.curly_count;
				parse_line(label);
				--syntax.curly_count;

				// Set the position of the function (label)
				l=add_word(func_name);
				if( str_data[l].type == C_NOP || str_data[l].type == C_USERFUNC )// register only, if the name was not used by something else
				{
					str_data[l].type = C_USERFUNC;
					set_label(l, script_pos, p);
					if( parse_options&SCRIPT_USE_LABEL_DB )
						strdb_iput(scriptlabel_db, get_str(l), script_pos);
				}
				else
					disp_error_message("parse_syntax:function: function name is invalid", func_name);

				return skip_space(p);
			}
			else
			{
				disp_error_message("expect ';' or '{' at function syntax",p);
			}
		}
		break;
	case 'i':
	case 'I':
		if(p2 - p == 2 && !strncasecmp(p,"if",2)) {
			// If process
			char label[256];
			p=skip_space(p2);
			if(*p != '(') { //Prevent if this {} non-c syntax. from Rayce (jA)
				disp_error_message("need '('",p);
			}
			syntax.curly[syntax.curly_count].type  = TYPE_IF;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			sprintf(label,"__IF%x_%x",syntax.curly[syntax.curly_count].index,syntax.curly[syntax.curly_count].count);
			syntax.curly_count++;
			add_scriptl(add_str("jump_zero"));
			add_scriptc(C_ARG);
			p=parse_expr(p);
			p=skip_space(p);
			add_scriptl(add_str(label));
			add_scriptc(C_FUNC);
			return p;
		}
		break;
	case 's':
	case 'S':
		if(p2 - p == 6 && !strncasecmp(p,"switch",6)) {
			// Processing of switch ()
			char label[256];
			p=skip_space(p2);
			if(*p != '(') {
				disp_error_message("need '('",p);
			}
			syntax.curly[syntax.curly_count].type  = TYPE_SWITCH;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			sprintf(label,"$@__SW%x_VAL",syntax.curly[syntax.curly_count].index);
			syntax.curly_count++;
			add_scriptl(add_str("set"));
			add_scriptc(C_ARG);
			add_scriptl(add_str(label));
			p=parse_expr(p);
			p=skip_space(p);
			if(*p != '{') {
				disp_error_message("parse_syntax: expected '{'",p);
			}
			add_scriptc(C_FUNC);
			return p + 1;
		}
		break;
	case 'w':
	case 'W':
		if(p2 - p == 5 && !strncasecmp(p,"while",5)) {
			int l;
			char label[256];
			p=skip_space(p2);
			if(*p != '(') {
				disp_error_message("need '('",p);
			}
			syntax.curly[syntax.curly_count].type  = TYPE_WHILE;
			syntax.curly[syntax.curly_count].count = 1;
			syntax.curly[syntax.curly_count].index = syntax.index++;
			syntax.curly[syntax.curly_count].flag  = 0;
			// Form the start of label decision
			sprintf(label,"__WL%x_NXT",syntax.curly[syntax.curly_count].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			// Skip to the end point if the condition is false
			sprintf(label,"__WL%x_FIN",syntax.curly[syntax.curly_count].index);
			syntax.curly_count++;
			add_scriptl(add_str("jump_zero"));
			add_scriptc(C_ARG);
			p=parse_expr(p);
			p=skip_space(p);
			add_scriptl(add_str(label));
			add_scriptc(C_FUNC);
			return p;
		}
		break;
	}
	return NULL;
}

const char* parse_syntax_close(const char *p) {
	// If (...) for (...) hoge (); as to make sure closed closed once again
	int flag;

	do {
		p = parse_syntax_close_sub(p,&flag);
	} while(flag);
	return p;
}

// Close judgment if, for, while, of do
//	 flag == 1 : closed
//	 flag == 0 : not closed
const char* parse_syntax_close_sub(const char* p,int* flag)
{
	char label[256];
	int pos = syntax.curly_count - 1;
	int l;
	*flag = 1;

	if(syntax.curly_count <= 0) {
		*flag = 0;
		return p;
	} else if(syntax.curly[pos].type == TYPE_IF) {
		const char *bp = p;
		const char *p2;

		// if-block and else-block end is a new line
		parse_nextline(false, p);

		// Skip to the last location if
		sprintf(label,"goto __IF%x_FIN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// Put the label of the location
		sprintf(label,"__IF%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
		l=add_str(label);
		set_label(l,script_pos,p);

		syntax.curly[pos].count++;
		p = skip_space(p);
		p2 = skip_word(p);
		if(!syntax.curly[pos].flag && p2 - p == 4 && !strncasecmp(p,"else",4)) {
			// else  or else - if
			p = skip_space(p2);
			p2 = skip_word(p);
			if(p2 - p == 2 && !strncasecmp(p,"if",2)) {
				// else - if
				p=skip_space(p2);
				if(*p != '(') {
					disp_error_message("need '('",p);
				}
				sprintf(label,"__IF%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
				add_scriptl(add_str("jump_zero"));
				add_scriptc(C_ARG);
				p=parse_expr(p);
				p=skip_space(p);
				add_scriptl(add_str(label));
				add_scriptc(C_FUNC);
				*flag = 0;
				return p;
			} else {
				// else
				if(!syntax.curly[pos].flag) {
					syntax.curly[pos].flag = 1;
					*flag = 0;
					return p;
				}
			}
		}
		// Close if
		syntax.curly_count--;
		// Put the label of the final location
		sprintf(label,"__IF%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		if(syntax.curly[pos].flag == 1) {
			// Because the position of the pointer is the same if not else for this
			return bp;
		}
		return p;
	} else if(syntax.curly[pos].type == TYPE_DO) {
		int l2;
		char label2[256];
		const char *p2;

		if(syntax.curly[pos].flag) {
			// (Come here continue) to form the label here
			sprintf(label2,"__DO%x_NXT",syntax.curly[pos].index);
			l2=add_str(label2);
			set_label(l2,script_pos,p);
		}

		// Skip to the end point if the condition is false
		p = skip_space(p);
		p2 = skip_word(p);
		if(p2 - p != 5 || strncasecmp(p,"while",5))
			disp_error_message("parse_syntax: expected 'while'",p);

		p = skip_space(p2);
		if(*p != '(') {
			disp_error_message("need '('",p);
		}

		// do-block end is a new line
		parse_nextline(false, p);

		sprintf(label2,"__DO%x_FIN",syntax.curly[pos].index);
		add_scriptl(add_str("jump_zero"));
		add_scriptc(C_ARG);
		p=parse_expr(p);
		p=skip_space(p);
		add_scriptl(add_str(label2));
		add_scriptc(C_FUNC);

		// Skip to the starting point
		sprintf(label2,"goto __DO%x_BGN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label2);
		syntax.curly_count--;

		// Form label of the end point conditions
		sprintf(label2,"__DO%x_FIN",syntax.curly[pos].index);
		l2=add_str(label2);
		set_label(l2,script_pos,p);
		p = skip_space(p);
		if(*p != ';') {
			disp_error_message("parse_syntax: expected ';'",p);
			return p+1;
		}
		p++;
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[pos].type == TYPE_FOR) {
		// for-block end is a new line
		parse_nextline(false, p);

		// Skip to the next loop
		sprintf(label,"goto __FR%x_NXT;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// End for labeling
		sprintf(label,"__FR%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[pos].type == TYPE_WHILE) {
		// while-block end is a new line
		parse_nextline(false, p);

		// Skip to the decision while
		sprintf(label,"goto __WL%x_NXT;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// End while labeling
		sprintf(label,"__WL%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_USERFUNC) {
		int pos2 = syntax.curly_count-1;
		char label2[256];
		int l2;
		// Back
		sprintf(label2,"return;");
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label2);
		syntax.curly_count--;

		// Put the label of the location
		sprintf(label2,"__FN%x_FIN",syntax.curly[pos2].index);
		l2=add_str(label2);
		set_label(l2,script_pos,p);
		syntax.curly_count--;
		return p;
	} else {
		*flag = 0;
		return p;
	}
}

/*==========================================
 * Added built-in functions
 *------------------------------------------*/
static void add_buildin_func(void)
{
	int i;
	for( i = 0; buildin_func[i].func; i++ )
	{
		// arg must follow the pattern: (v|s|i|r|l)*\?*\*?
		// 'v' - value (either string or int or reference)
		// 's' - string
		// 'i' - int
		// 'r' - reference (of a variable)
		// 'l' - label
		// '?' - one optional parameter
		// '*' - unknown number of optional parameters
		const char* p = buildin_func[i].arg;
		while( *p == 'v' || *p == 's' || *p == 'i' || *p == 'r' || *p == 'l' ) ++p;
		while( *p == '?' ) ++p;
		if( *p == '*' ) ++p;
		if( *p != 0){
			ShowWarning("add_buildin_func: ignoring function \"%s\" with invalid arg \"%s\".\n", buildin_func[i].name, buildin_func[i].arg);
		} else if( *skip_word(buildin_func[i].name) != 0 ){
			ShowWarning("add_buildin_func: ignoring function with invalid name \"%s\" (must be a word).\n", buildin_func[i].name);
		} else {
			int n = add_str(buildin_func[i].name);
			str_data[n].type = C_FUNC;
			str_data[n].val = i;
			str_data[n].func = buildin_func[i].func;
			str_data[n].deprecated = (buildin_func[i].deprecated != NULL);

			if (!strcmp(buildin_func[i].name, "setr")) buildin_set_ref = n;
			else if (!strcmp(buildin_func[i].name, "callsub")) buildin_callsub_ref = n;
			else if (!strcmp(buildin_func[i].name, "callfunc")) buildin_callfunc_ref = n;
			else if( !strcmp(buildin_func[i].name, "getelementofarray") ) buildin_getelementofarray_ref = n;
		}
	}
}

/**
 * String comparison with a char array to a script constant
 * @param prefix: Char array to compare
 * @param value: Script constant
 * @return Constant name as char array or NULL otherwise
 */
const char* script_get_constant_str( const char* prefix, int64 value ){
	const char* name;

	for(int i = 0; i < str_data_size; i++ ){
		// Check if it is a constant
		if( str_data[i].type != C_INT ){
			continue;
		}

		// Check if the value matches
		if( str_data[i].val != value ){
			continue;
		}

		// Look up the actual string
		name = get_str(i);

		// Compare the prefix
		if( !strncasecmp( name, prefix, strlen(prefix) ) ){
			// We found a match
			return name;
		}
	}

	return NULL;
}

/// Retrieves the value of a constant parameter.
bool script_get_parameter(const char* name, int* value)
{
	int n = search_str(name);

	if (n == -1 || str_data[n].type != C_PARAM)
	{// not found or not a parameter
		return false;
	}
	value[0] = str_data[n].val;

	return true;
}

/// Retrieves the value of a constant.
bool script_get_constant(const char* name, int* value)
{
	int n = search_str(name);

	if( n == -1 || str_data[n].type != C_INT )
	{// not found or not a constant
		return false;
	}
	value[0] = str_data[n].val;

#if defined(SCRIPT_CONSTANT_DEPRECATION)
	if( str_data[n].deprecated ){
		ShowWarning( "Usage of deprecated constant '%s'.\n", name );
		ShowWarning( "This constant was deprecated and could become unavailable anytime soon.\n" );
	}
#endif

	return true;
}

/// Creates new constant or parameter with given value.
void script_set_constant(const char* name, int value, bool isparameter, bool deprecated)
{
	int n = add_str(name);

	if( str_data[n].type == C_NOP )
	{// new
		str_data[n].type = isparameter ? C_PARAM : C_INT;
		str_data[n].val  = value;
		str_data[n].deprecated = deprecated;
	}
	else if( str_data[n].type == C_PARAM || str_data[n].type == C_INT )
	{// existing parameter or constant
		ShowError("script_set_constant: Attempted to overwrite existing %s '%s' (old value=%d, new value=%d).\n", ( str_data[n].type == C_PARAM ) ? "parameter" : "constant", name, str_data[n].val, value);
	}
	else
	{// existing name
		ShowError("script_set_constant: Invalid name for %s '%s' (already defined as %s).\n", isparameter ? "parameter" : "constant", name, script_op2name(str_data[n].type));
	}
}

static bool read_constdb_sub( char* fields[], int columns, int current ){
	char name[1024], val[1024];
	int type = 0;

	if( columns > 1 ){
		if( sscanf(fields[0], "%1023[A-Za-z0-9/_]", name) != 1 ||
			sscanf(fields[1], "%1023[A-Za-z0-9/_]", val) != 1 || 
			( columns >= 2 && sscanf(fields[2], "%11d", &type) != 1 ) ){
			ShowWarning("Skipping line '" CL_WHITE "%d" CL_RESET "', invalid constant definition\n", current);
			return false;
		}
	}else{
		if( sscanf(fields[0], "%1023[A-Za-z0-9/_] %1023[A-Za-z0-9/_-] %11d", name, val, &type) < 2 ){
			ShowWarning( "Skipping line '" CL_WHITE "%d" CL_RESET "', invalid constant definition\n", current );
			return false;
		}
	}

	script_set_constant(name, (int)strtol(val, NULL, 0), (type != 0), false);

	return true;
}

/*==========================================
 * Reading constant databases
 * const.txt
 *------------------------------------------*/
static void read_constdb(void){
	const char* dbsubpath[] = {
		"",
		"/" DBIMPORT,
	};

	for( int i = 0; i < ARRAYLENGTH(dbsubpath); i++ ){
		int n2 = strlen(db_path) + strlen(dbsubpath[i]) + 1;
		char* dbsubpath2 = (char*)aMalloc(n2 + 1);
		bool silent = i > 0;

		safesnprintf(dbsubpath2, n2, "%s%s", db_path, dbsubpath[i]);

		sv_readdb(dbsubpath2, "const.txt", ',', 1, 3, -1, &read_constdb_sub, silent);

		aFree(dbsubpath2);
	}
}

/**
 * Sets source-end constants for NPC scripts to access.
 **/
void script_hardcoded_constants(void) {
	#include "script_constants.hpp"
}

/*==========================================
 * Display emplacement line of script
 *------------------------------------------*/
static const char* script_print_line(StringBuf* buf, const char* p, const char* mark, int line)
{
	int i;
	if( p == NULL || !p[0] ) return NULL;
	if( line < 0 )
		StringBuf_Printf(buf, "*% 5d : ", -line);
	else
		StringBuf_Printf(buf, " % 5d : ", line);
	for(i=0;p[i] && p[i] != '\n';i++){
		if(p + i != mark)
			StringBuf_Printf(buf, "%c", p[i]);
		else
			StringBuf_Printf(buf, "\'%c\'", p[i]);
	}
	StringBuf_AppendStr(buf, "\n");
	return p+i+(p[i] == '\n' ? 1 : 0);
}

void script_errorwarning_sub(StringBuf *buf, const char* src, const char* file, int start_line, const char* error_msg_cur, const char* error_pos_cur) {
	// Find the line where the error occurred
	int j;
	int line = start_line;
	const char *p;
	const char *linestart[5] = { NULL, NULL, NULL, NULL, NULL };

	for(p=src;p && *p;line++){
		const char *lineend=strchr(p,'\n');
		if(lineend==NULL || error_pos_cur<lineend){
			break;
		}
		for( j = 0; j < 4; j++ ) {
			linestart[j] = linestart[j+1];
		}
		linestart[4] = p;
		p=lineend+1;
	}

	StringBuf_Printf(buf, "script error on %s line %d\n", file, line);
	StringBuf_Printf(buf, "    %s\n", error_msg_cur);
	for(j = 0; j < 5; j++ ) {
		script_print_line(buf, linestart[j], NULL, line + j - 5);
	}
	p = script_print_line(buf, p, error_pos_cur, -line);
	for(j = 0; j < 5; j++) {
		p = script_print_line(buf, p, NULL, line + j + 1 );
	}
}

void script_error(const char* src, const char* file, int start_line, const char* error_msg_cur, const char* error_pos_cur) {
	StringBuf buf;

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "\a\n");

	script_errorwarning_sub(&buf, src, file, start_line, error_msg_cur, error_pos_cur);

	ShowError("%s", StringBuf_Value(&buf));
	StringBuf_Destroy(&buf);
}

void script_warning(const char* src, const char* file, int start_line, const char* error_msg_cur, const char* error_pos_cur) {
	StringBuf buf;

	StringBuf_Init(&buf);

	script_errorwarning_sub(&buf, src, file, start_line, error_msg_cur, error_pos_cur);

	ShowWarning("%s", StringBuf_Value(&buf));
	StringBuf_Destroy(&buf);
}

/*==========================================
 * Analysis of the script
 *------------------------------------------*/
struct script_code* parse_script(const char *src,const char *file,int line,int options)
{
	const char *p,*tmpp;
	int i;
	struct script_code* code = NULL;
	static bool first=true;
	char end;
	bool unresolved_names = false;

	parser_current_src = src;
	parser_current_file = file;
	parser_current_line = line;

	if( src == NULL )
		return NULL;// empty script

	memset(&syntax,0,sizeof(syntax));
	if(first){
		add_buildin_func();
		read_constdb();
		script_hardcoded_constants();
		first=false;
	}

	script_buf=(unsigned char *)aMalloc(SCRIPT_BLOCK_SIZE*sizeof(unsigned char));
	script_pos=0;
	script_size=SCRIPT_BLOCK_SIZE;
	parse_nextline(true, NULL);

	// who called parse_script is responsible for clearing the database after using it, but just in case... lets clear it here
	if( options&SCRIPT_USE_LABEL_DB )
		db_clear(scriptlabel_db);
	parse_options = options;

	if( setjmp( error_jump ) != 0 ) {
		//Restore program state when script has problems. [from jA]
		int j;
		const int size = ARRAYLENGTH(syntax.curly);
		if( error_report )
			script_error(src,file,line,error_msg,error_pos);
		aFree( error_msg );
		aFree( script_buf );
		script_pos  = 0;
		script_size = 0;
		script_buf  = NULL;
		for(j=LABEL_START;j<str_num;j++)
			if(str_data[j].type == C_NOP) str_data[j].type = C_NAME;
		for(j=0; j<size; j++)
			linkdb_final(&syntax.curly[j].case_label);
		return NULL;
	}

	parse_syntax_for_flag=0;
	p=src;
	p=skip_space(p);
	if( options&SCRIPT_IGNORE_EXTERNAL_BRACKETS )
	{// does not require brackets around the script
		if( *p == '\0' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT) )
		{// empty script and can return NULL
			aFree( script_buf );
			script_pos  = 0;
			script_size = 0;
			script_buf  = NULL;
			return NULL;
		}
		end = '\0';
	}
	else
	{// requires brackets around the script
		if( *p != '{' )
			disp_error_message("not found '{'",p);
		p = skip_space(p+1);
		if( *p == '}' && !(options&SCRIPT_RETURN_EMPTY_SCRIPT) )
		{// empty script and can return NULL
			aFree( script_buf );
			script_pos  = 0;
			script_size = 0;
			script_buf  = NULL;
			return NULL;
		}
		end = '}';
	}

	// clear references of labels, variables and internal functions
	for(i=LABEL_START;i<str_num;i++){
		if(
			str_data[i].type==C_POS || str_data[i].type==C_NAME ||
			str_data[i].type==C_USERFUNC || str_data[i].type == C_USERFUNC_POS
		){
			str_data[i].type=C_NOP;
			str_data[i].backpatch=-1;
			str_data[i].label=-1;
		}
	}

	while( syntax.curly_count != 0 || *p != end )
	{
		if( *p == '\0' )
			disp_error_message("unexpected end of script",p);
		// Special handling only label
		tmpp=skip_space(skip_word(p));
		if(*tmpp==':' && !(!strncasecmp(p,"default:",8) && p + 7 == tmpp)){
			i=add_word(p);
			set_label(i,script_pos,p);
			if( parse_options&SCRIPT_USE_LABEL_DB )
				strdb_iput(scriptlabel_db, get_str(i), script_pos);
			p=tmpp+1;
			p=skip_space(p);
			continue;
		}

		// All other lumped
		p=parse_line(p);
		p=skip_space(p);

		parse_nextline(false, p);
	}

	add_scriptc(C_NOP);

	// trim code to size
	script_size = script_pos;
	RECREATE(script_buf,unsigned char,script_pos);

	// default unknown references to variables
	for(i=LABEL_START;i<str_num;i++){
		if(str_data[i].type==C_NOP){
			int j;
			str_data[i].type=C_NAME;
			str_data[i].label=i;
			for(j=str_data[i].backpatch;j>=0 && j!=0x00ffffff;){
				int next=GETVALUE(script_buf,j);
				SETVALUE(script_buf,j,i);
				j=next;
			}
		}
		else if( str_data[i].type == C_USERFUNC )
		{// 'function name;' without follow-up code
			ShowError("parse_script: function '%s' declared but not defined.\n", str_buf+str_data[i].str);
			unresolved_names = true;
		}
	}

	if( unresolved_names )
	{
		disp_error_message("parse_script: unresolved function references", p);
	}

#ifdef DEBUG_DISP
	for(i=0;i<script_pos;i++){
		if((i&15)==0) ShowMessage("%04x : ",i);
		ShowMessage("%02x ",script_buf[i]);
		if((i&15)==15) ShowMessage("\n");
	}
	ShowMessage("\n");
#endif
#ifdef DEBUG_DISASM
	{
		int i = 0,j;
		while(i < script_pos) {
			c_op op = get_com(script_buf,&i);

			ShowMessage("%06x %s", i, script_op2name(op));
			j = i;
			switch(op) {
			case C_INT:
				ShowMessage(" %d", get_num(script_buf,&i));
				break;
			case C_POS:
				ShowMessage(" 0x%06x", *(int*)(script_buf+i)&0xffffff);
				i += 3;
				break;
			case C_NAME:
				j = (*(int*)(script_buf+i)&0xffffff);
				ShowMessage(" %s", ( j == 0xffffff ) ? "?? unknown ??" : get_str(j));
				i += 3;
				break;
			case C_STR:
				j = strlen(script_buf + i);
				ShowMessage(" %s", script_buf + i);
				i += j+1;
				break;
			}
			ShowMessage(CL_CLL"\n");
		}
	}
#endif

	CREATE(code,struct script_code,1);
	code->script_buf  = script_buf;
	code->script_size = script_size;
	code->local.vars = NULL;
	code->local.arrays = NULL;
	return code;
}

/// Returns the player attached to this script, identified by the rid.
/// If there is no player attached, the script is terminated.
static bool script_rid2sd_( struct script_state *st, struct map_session_data** sd, const char *func ){
	*sd = map_id2sd( st->rid );

	if( *sd ){
		return true;
	}else{
		ShowError("%s: fatal error ! player not attached!\n",func);
		script_reportfunc(st);
		script_reportsrc(st);
		st->state = END;
		return false;
	}
}

/**
 * Dereferences a variable/constant, replacing it with a copy of the value.
 * @param st Script state
 * @param data Variable/constant
 * @param sd If NULL, will try to use sd from st->rid (for player's variables)
 */
void get_val_(struct script_state* st, struct script_data* data, struct map_session_data *sd)
{
	const char* name;
	char prefix;
	char postfix;

	if( !data_isreference(data) )
		return;// not a variable/constant

	name = reference_getname(data);
	prefix = name[0];
	postfix = name[strlen(name) - 1];

	//##TODO use reference_tovariable(data) when it's confirmed that it works [FlavioJS]
	if( !reference_toconstant(data) && not_server_variable(prefix) ) {
		if( sd == NULL && !script_rid2sd(sd) ) {// needs player attached
			if( postfix == '$' ) {// string variable
				ShowWarning("script:get_val: cannot access player variable '%s', defaulting to \"\"\n", name);
				data->type = C_CONSTSTR;
				data->u.str = const_cast<char *>("");
			} else {// integer variable
				ShowWarning("script:get_val: cannot access player variable '%s', defaulting to 0\n", name);
				data->type = C_INT;
				data->u.num = 0;
			}
			return;
		}
	}

	if( postfix == '$' ) {// string variable

		switch( prefix ) {
			case '@':
				data->u.str = pc_readregstr(sd, data->u.num);
				break;
			case '$':
				data->u.str = mapreg_readregstr(data->u.num);
				break;
			case '#':
				if( name[1] == '#' )
					data->u.str = pc_readaccountreg2str(sd, data->u.num);// global
				else
					data->u.str = pc_readaccountregstr(sd, data->u.num);// local
				break;
			case '.':
				{
					struct DBMap* n = data->ref ?
							data->ref->vars : name[1] == '@' ?
							st->stack->scope.vars : // instance/scope variable
							st->script->local.vars; // npc variable
					if( n )
						data->u.str = (char*)i64db_get(n,reference_getuid(data));
					else
						data->u.str = NULL;
				}
				break;
			case '\'':
				{
					unsigned short instance_id = script_instancegetid(st);
					if( instance_id )
						data->u.str = (char*)i64db_get(instance_data[instance_id].regs.vars,reference_getuid(data));
					else {
						ShowWarning("script:get_val: cannot access instance variable '%s', defaulting to \"\"\n", name);
						data->u.str = NULL;
					}
				}
				break;
			default:
				data->u.str = pc_readglobalreg_str(sd, data->u.num);
				break;
		}

		if( data->u.str == NULL || data->u.str[0] == '\0' ) {// empty string
			data->type = C_CONSTSTR;
			data->u.str = const_cast<char *>("");
		} else {// duplicate string
			data->type = C_STR;
			data->u.str = aStrdup(data->u.str);
		}

	} else {// integer variable

		data->type = C_INT;

		if( reference_toconstant(data) ) {
			data->u.num = reference_getconstant(data);
		} else if( reference_toparam(data) ) {
			data->u.num = pc_readparam(sd, reference_getparamtype(data));
		} else
			switch( prefix ) {
				case '@':
					data->u.num = pc_readreg(sd, data->u.num);
					break;
				case '$':
					data->u.num = mapreg_readreg(data->u.num);
					break;
				case '#':
					if( name[1] == '#' )
						data->u.num = pc_readaccountreg2(sd, data->u.num);// global
					else
						data->u.num = pc_readaccountreg(sd, data->u.num);// local
					break;
				case '.':
					{
						struct DBMap* n = data->ref ?
								data->ref->vars : name[1] == '@' ?
								st->stack->scope.vars : // instance/scope variable
								st->script->local.vars; // npc variable
						if( n )
							data->u.num = (int)i64db_iget(n,reference_getuid(data));
						else
							data->u.num = 0;
					}
					break;
				case '\'':
					{
						unsigned short instance_id = script_instancegetid(st);
						if( instance_id )
							data->u.num = (int)i64db_iget(instance_data[instance_id].regs.vars,reference_getuid(data));
						else {
							ShowWarning("script:get_val: cannot access instance variable '%s', defaulting to 0\n", name);
							data->u.num = 0;
						}
					}
					break;
				default:
					data->u.num = pc_readglobalreg(sd, data->u.num);
					break;
			}

	}
	data->ref = NULL;

	return;
}

void get_val(struct script_state* st, struct script_data* data)
{
	get_val_(st,data,NULL);
}

struct script_data* push_val2(struct script_stack* stack, enum c_op type, int64 val, struct reg_db* ref);

/// Retrieves the value of a reference identified by uid (variable, constant, param)
/// The value is left in the top of the stack and needs to be removed manually.
/// @param st[in]: script state.
/// @param uid[in]: reference identifier.
/// @param ref[in]: the container to look up the reference into.
/// @return: the retrieved value of the reference.
void* get_val2(struct script_state* st, int64 uid, struct reg_db *ref)
{
	struct script_data* data;
	push_val2(st->stack, C_NAME, uid, ref);
	data = script_getdatatop(st, -1);
	get_val(st, data);
	//! TODO: Support data->u.num as int64 instead cast it to int? (in get_val, it was casted to int).
	return (data->type == C_INT ? (void*)__64BPRTSIZE((int)data->u.num) : (void*)__64BPRTSIZE(data->u.str));
}

/**
 * Because, currently, array members with key 0 are indifferenciable from normal variables, we should ensure its actually in
 * Will be gone as soon as undefined var feature is implemented
 **/
void script_array_ensure_zero(struct script_state *st, struct map_session_data *sd, int64 uid, struct reg_db *ref)
{
	const char *name = get_str(script_getvarid(uid));
	// is here st can be null pointer and st->rid is wrong?
	struct reg_db *src = script_array_src(st, sd ? sd : st->rid ? map_id2sd(st->rid) : NULL, name, ref);
	bool insert = false;

	if (sd && !st) {
		// when sd comes, st isn't available
		insert = true;
	} else {
		if( is_string_variable(name) ) {
			char* str = (char*)get_val2(st, uid, ref);
			if( str && *str )
				insert = true;
			script_removetop(st, -1, 0);
		} else {
			int32 num = (int32)__64BPRTSIZE(get_val2(st, uid, ref));
			if( num )
				insert = true;
			script_removetop(st, -1, 0);
		}
	}

	if (src && src->arrays) {
		struct script_array *sa = static_cast<script_array *>(idb_get(src->arrays, script_getvarid(uid)));
		if (sa) {
			unsigned int i;

			ARR_FIND(0, sa->size, i, sa->members[i] == 0);
			if( i != sa->size ) {
				if( !insert )
					script_array_remove_member(src,sa,i);
				return;
			}

			script_array_add_member(sa,0);
		} else if (insert) {
			script_array_update(src,reference_uid(script_getvarid(uid), 0),false);
		}
	}
}

/**
 * Returns array size by ID
 **/
unsigned int script_array_size(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref)
{
	struct script_array *sa = NULL;
	struct reg_db *src = script_array_src(st, sd, name, ref);

	if (src && src->arrays)
		sa = static_cast<script_array *>(idb_get(src->arrays, search_str(name)));

	return sa ? sa->size : 0;
}

/**
 * Returns array's highest key (for that awful getarraysize implementation that doesn't really gets the array size)
 **/
unsigned int script_array_highest_key(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref)
{
	struct script_array *sa = NULL;
	struct reg_db *src = script_array_src(st, sd, name, ref);

	if (src && src->arrays) {
		int key = add_word(name);

		script_array_ensure_zero(st,sd,reference_uid(key, 0), ref);

		if( ( sa = static_cast<script_array *>(idb_get(src->arrays, key)) ) ) {
			unsigned int i, highest_key = 0;

			for(i = 0; i < sa->size; i++) {
				if( sa->members[i] > highest_key )
					highest_key = sa->members[i];
			}

			return sa->size ? highest_key + 1 : 0;
		}
	}
	
	return SCRIPT_CMD_SUCCESS;
}

int script_free_array_db(DBKey key, DBData *data, va_list ap)
{
	struct script_array *sa = static_cast<script_array *>(db_data2ptr(data));
	aFree(sa->members);
	ers_free(array_ers, sa);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Clears script_array and removes it from script->array_db
 **/
void script_array_delete(struct reg_db *src, struct script_array *sa)
{
	aFree(sa->members);
	idb_remove(src->arrays, sa->id);
	ers_free(array_ers, sa);
}

/**
 * Removes a member from a script_array list
 *
 * @param idx the index of the member in script_array struct list, not of the actual array member
 **/
void script_array_remove_member(struct reg_db *src, struct script_array *sa, unsigned int idx)
{
	unsigned int i, cursor;

	// it's the only member left, no need to do anything other than delete the array data
	if( sa->size == 1 ) {
		script_array_delete(src,sa);
		return;
	}

	sa->members[idx] = UINT_MAX;

	for(i = 0, cursor = 0; i < sa->size; i++) {
		if( sa->members[i] == UINT_MAX )
			continue;
		if( i != cursor )
			sa->members[cursor] = sa->members[i];
		cursor++;
	}

	sa->size = cursor;
}

/**
 * Appends a new array index to the list in script_array
 *
 * @param idx the index of the array member being inserted
 **/
void script_array_add_member(struct script_array *sa, unsigned int idx)
{
	RECREATE(sa->members, unsigned int, ++sa->size);

	sa->members[sa->size - 1] = idx;
}

/**
 * Obtains the source of the array database for this type and scenario
 * Initializes such database when not yet initialized.
 **/
struct reg_db *script_array_src(struct script_state *st, struct map_session_data *sd, const char *name, struct reg_db *ref)
{
	struct reg_db *src = NULL;

	switch( name[0] ) {
		// from player
		default:  // char reg
		case '@': // temp char reg
		case '#': // account reg
			src = &sd->regs;
			break;
		case '$': // map reg
			src = &regs;
			break;
		case '.': // npc/script
			if (ref)
				src = ref;
			else
				src = (name[1] == '@') ? &st->stack->scope : &st->script->local;
			break;
		case '\'': // instance
			{
				unsigned short instance_id = script_instancegetid(st);

				if( instance_id ) {
					src = &instance_data[instance_id].regs;
				}
				break;
			}
	}

	if( src ) {
		if( !src->arrays )
			src->arrays = idb_alloc(DB_OPT_BASE);
		return src;
	}

	return NULL;
}

/**
 * Processes a array member modification, and update data accordingly
 *
 * @param src[in,out] Variable source database. If the array database doesn't exist, it is created.
 * @param num[in]     Variable ID
 * @param empty[in]   Whether the modified member is empty (needs to be removed)
 **/
void script_array_update(struct reg_db *src, int64 num, bool empty)
{
	struct script_array *sa = NULL;
	int id = script_getvarid(num);
	unsigned int index = script_getvaridx(num);

	if (!src->arrays) {
		src->arrays = idb_alloc(DB_OPT_BASE);
	} else {
		sa = static_cast<script_array *>(idb_get(src->arrays, id));
	}

	if( sa ) {
		unsigned int i;

		// search
		for(i = 0; i < sa->size; i++) {
			if( sa->members[i] == index )
				break;
		}

		// if existent
		if( i != sa->size ) {
			// if empty, we gotta remove it
			if( empty ) {
				script_array_remove_member(src, sa, i);
			}
		} else if( !empty ) { /* new entry */
			script_array_add_member(sa,index);
			// we do nothing if its empty, no point in modifying array data for a new empty member
		}
	} else if ( !empty ) { // we only move to create if not empty
		sa = ers_alloc(array_ers, struct script_array);
		sa->id = id;
		sa->members = NULL;
		sa->size = 0;
		script_array_add_member(sa,index);
		idb_put(src->arrays, id, sa);
	}
}

/**
 * Stores the value of a script variable
 *
 * @param st:    current script state.
 * @param sd:    current character data.
 * @param num:   variable identifier.
 * @param name:  variable name.
 * @param value: new value.
 * @param ref:   variable container, in case of a npc/scope variable reference outside the current scope.
 * @return: 0 failure, 1 success.
 *
 * TODO: return values are screwed up, have been for some time (reaad: years), e.g. some functions return 1 failure and success.
 *------------------------------------------*/
int set_reg(struct script_state* st, struct map_session_data* sd, int64 num, const char* name, const void* value, struct reg_db *ref)
{
	char prefix = name[0];
	size_t vlen = 0;
	if ( !script_check_RegistryVariableLength(0,name,&vlen) )
	{
		ShowError("set_reg: Variable name length is too long (aid: %d, cid: %d): '%s' sz=%d\n", sd?sd->status.account_id:-1, sd?sd->status.char_id:-1, name, vlen);
		return 0;
	}

	if( is_string_variable(name) ) {// string variable
		const char *str = (const char*)value;

		switch (prefix) {
			case '@':
				pc_setregstr(sd, num, str);
				return 1;
			case '$':
				return mapreg_setregstr(num, str);
			case '#':
				return (name[1] == '#') ?
					pc_setaccountreg2str(sd, num, str) :
					pc_setaccountregstr(sd, num, str);
			case '.':
				{
					struct reg_db *n = (ref) ? ref : (name[1] == '@') ? &st->stack->scope : &st->script->local;
					if( n ) {
						if (str[0])  {
							i64db_put(n->vars, num, aStrdup(str));
							if( script_getvaridx(num) )
								script_array_update(n, num, false);
						} else {
							i64db_remove(n->vars, num);
							if( script_getvaridx(num) )
								script_array_update(n, num, true);
						}
					}
				}
				return 1;
			case '\'':
				{
					unsigned short instance_id = script_instancegetid(st);
					if( instance_id ) {
						if( str[0] ) {
							i64db_put(instance_data[instance_id].regs.vars, num, aStrdup(str));
							if( script_getvaridx(num) )
								script_array_update(&instance_data[instance_id].regs, num, false);
						} else {
							i64db_remove(instance_data[instance_id].regs.vars, num);
							if (script_getvaridx(num))
								script_array_update(&instance_data[instance_id].regs, num, true);
						}
					} else {
						ShowError("script_set_reg: cannot write instance variable '%s', NPC not in a instance!\n", name);
						script_reportsrc(st);
					}
				return 1;
				}
			default:
				return pc_setglobalreg_str(sd, num, str);
		}
	} else {// integer variable
		int val = (int)__64BPRTSIZE(value);

		if(str_data[script_getvarid(num)].type == C_PARAM) {
			if( pc_setparam(sd, str_data[script_getvarid(num)].val, val) == 0 ) {
				if( st != NULL ) {
					ShowError("script_set_reg: failed to set param '%s' to %d.\n", name, val);
					script_reportsrc(st);
					st->state = END;
				}
				return 0;
			}
			return 1;
		}

		switch (prefix) {
			case '@':
				pc_setreg(sd, num, val);
				return 1;
			case '$':
				return mapreg_setreg(num, val);
			case '#':
				return (name[1] == '#') ?
					pc_setaccountreg2(sd, num, val) :
					pc_setaccountreg(sd, num, val);
			case '.':
				{
					struct reg_db *n = (ref) ? ref : (name[1] == '@') ? &st->stack->scope : &st->script->local;
					if( n ) {
						if( val != 0 ) {
							i64db_iput(n->vars, num, val);
							if( script_getvaridx(num) )
								script_array_update(n, num, false);
						} else {
							i64db_remove(n->vars, num);
							if( script_getvaridx(num) )
								script_array_update(n, num, true);
						}
					}
				}
				return 1;
			case '\'':
				{
					unsigned short instance_id = script_instancegetid(st);
					if( instance_id ) {
						if( val != 0 ) {
							i64db_iput(instance_data[instance_id].regs.vars, num, val);
							if( script_getvaridx(num) )
								script_array_update(&instance_data[instance_id].regs, num, false);
						} else {
							i64db_remove(instance_data[instance_id].regs.vars, num);
							if (script_getvaridx(num))
								script_array_update(&instance_data[instance_id].regs, num, true);
						}
					} else {
						ShowError("script_set_reg: cannot write instance variable '%s', NPC not in a instance!\n", name);
						script_reportsrc(st);
					}
				return 1;
				}
			default:
				return pc_setglobalreg(sd, num, val);
		}
	}
}

int set_var(struct map_session_data* sd, char* name, void* val)
{
	return set_reg(NULL, sd, reference_uid(add_str(name),0), name, val, NULL);
}

void setd_sub(struct script_state *st, struct map_session_data *sd, const char *varname, int elem, void *value, struct reg_db *ref)
{
	set_reg(st, sd, reference_uid(add_str(varname),elem), varname, value, ref);
}

/**
 * Converts the data to a string
 * @param st
 * @param data
 * @param sd
 */
const char* conv_str_(struct script_state* st, struct script_data* data, struct map_session_data *sd)
{
	char* p;

	get_val_(st, data, sd);
	if( data_isstring(data) )
	{// nothing to convert
	}
	else if( data_isint(data) )
	{// int -> string
		CREATE(p, char, ITEM_NAME_LENGTH);
		snprintf(p, ITEM_NAME_LENGTH, "%" PRId64 "", data->u.num);
		p[ITEM_NAME_LENGTH-1] = '\0';
		data->type = C_STR;
		data->u.str = p;
	}
	else if( data_isreference(data) )
	{// reference -> string
		//##TODO when does this happen (check get_val) [FlavioJS]
		data->type = C_CONSTSTR;
		data->u.str = reference_getname(data);
	}
	else
	{// unsupported data type
		ShowError("script:conv_str: cannot convert to string, defaulting to \"\"\n");
		script_reportdata(data);
		script_reportsrc(st);
		data->type = C_CONSTSTR;
		data->u.str = const_cast<char *>("");
	}
	return data->u.str;
}

const char* conv_str(struct script_state* st, struct script_data* data)
{
	return conv_str_(st, data, NULL);
}

/**
 * Converts the data to an int
 * @param st
 * @param data
 * @param sd
 */
int conv_num_(struct script_state* st, struct script_data* data, struct map_session_data *sd)
{
	get_val_(st, data, sd);
	if( data_isint(data) )
	{// nothing to convert
	}
	else if( data_isstring(data) )
	{// string -> int
		// the result does not overflow or underflow, it is capped instead
		// ex: 999999999999 is capped to INT_MAX (2147483647)
		char* p = data->u.str;
		long num;

		errno = 0;
		num = strtol(data->u.str, NULL, 10);// change radix to 0 to support octal numbers "o377" and hex numbers "0xFF"
		if( errno == ERANGE
#if LONG_MAX > INT_MAX
			|| num < INT_MIN || num > INT_MAX
#endif
			)
		{
			if( num <= INT_MIN )
			{
				num = INT_MIN;
				ShowError("script:conv_num: underflow detected, capping to %ld\n", num);
			}
			else//if( num >= INT_MAX )
			{
				num = INT_MAX;
				ShowError("script:conv_num: overflow detected, capping to %ld\n", num);
			}
			script_reportdata(data);
			script_reportsrc(st);
		}
		if( data->type == C_STR )
			aFree(p);
		data->type = C_INT;
		data->u.num = (int)num;
	}
#if 0
	// FIXME this function is being used to retrieve the position of labels and
	// probably other stuff [FlavioJS]
	else
	{// unsupported data type
		ShowError("script:conv_num: cannot convert to number, defaulting to 0\n");
		script_reportdata(data);
		script_reportsrc(st);
		data->type = C_INT;
		data->u.num = 0;
	}
#endif
	return (int)data->u.num;
}

int conv_num(struct script_state* st, struct script_data* data)
{
	return conv_num_(st, data, NULL);
}

//
// Stack operations
//

/// Increases the size of the stack
void stack_expand(struct script_stack* stack)
{
	stack->sp_max += 64;
	stack->stack_data = (struct script_data*)aRealloc(stack->stack_data,
			stack->sp_max * sizeof(stack->stack_data[0]) );
	memset(stack->stack_data + (stack->sp_max - 64), 0,
			64 * sizeof(stack->stack_data[0]) );
}

/// Pushes a value into the stack
#define push_val(stack,type,val) push_val2(stack, type, val, NULL)

/// Pushes a value into the stack (with reference)
struct script_data* push_val2(struct script_stack* stack, enum c_op type, int64 val, struct reg_db *ref)
{
	if( stack->sp >= stack->sp_max )
		stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.num = val;
	stack->stack_data[stack->sp].ref   = ref;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a string into the stack
struct script_data* push_str(struct script_stack* stack, enum c_op type, char* str)
{
	if( stack->sp >= stack->sp_max )
		stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.str = str;
	stack->stack_data[stack->sp].ref   = NULL;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a retinfo into the stack
struct script_data* push_retinfo(struct script_stack* stack, struct script_retinfo* ri, struct reg_db *ref)
{
	if( stack->sp >= stack->sp_max )
		stack_expand(stack);
	stack->stack_data[stack->sp].type = C_RETINFO;
	stack->stack_data[stack->sp].u.ri = ri;
	stack->stack_data[stack->sp].ref  = ref;
	stack->sp++;
	return &stack->stack_data[stack->sp-1];
}

/// Pushes a copy of the target position into the stack
struct script_data* push_copy(struct script_stack* stack, int pos)
{
	switch( stack->stack_data[pos].type ) {
		case C_CONSTSTR:
			return push_str(stack, C_CONSTSTR, stack->stack_data[pos].u.str);
			break;
		case C_STR:
			return push_str(stack, C_STR, aStrdup(stack->stack_data[pos].u.str));
			break;
		case C_RETINFO:
			ShowFatalError("script:push_copy: can't create copies of C_RETINFO. Exiting...\n");
			exit(1);
			break;
		default:
			return push_val2(
				stack,stack->stack_data[pos].type,
				stack->stack_data[pos].u.num,
				stack->stack_data[pos].ref
			);
			break;
	}
}

/// Removes the values in indexes [start,end] from the stack.
/// Adjusts all stack pointers.
void pop_stack(struct script_state* st, int start, int end)
{
	struct script_stack* stack = st->stack;
	struct script_data* data;
	int i;

	if( start < 0 )
		start = 0;
	if( end > stack->sp )
		end = stack->sp;
	if( start >= end )
		return;// nothing to pop

	// free stack elements
	for( i = start; i < end; i++ )
	{
		data = &stack->stack_data[i];
		if( data->type == C_STR )
			aFree(data->u.str);
		if( data->type == C_RETINFO ) {
			struct script_retinfo* ri = data->u.ri;

			if (ri->scope.vars) {
				script_free_vars(ri->scope.vars);
				ri->scope.vars = NULL;
			}
			if (ri->scope.arrays) {
				ri->scope.arrays->destroy(ri->scope.arrays, script_free_array_db);
				ri->scope.arrays = NULL;
			}
			if( data->ref )
				aFree(data->ref);
			aFree(ri);
		}
		data->type = C_NOP;
	}

	// move the rest of the elements
	if( stack->sp > end )
	{
		memmove(&stack->stack_data[start], &stack->stack_data[end], sizeof(stack->stack_data[0])*(stack->sp - end));
		for( i = start + stack->sp - end; i < stack->sp; ++i )
			stack->stack_data[i].type = C_NOP;
	}

	// adjust stack pointers
	if( st->start > end ){
		st->start -= end - start;
	}else if( st->start > start ){
		st->start = start;
	}
	
	if( st->end > end ){
		st->end -= end - start;
	}else if( st->end > start ){
		st->end = start;
	}
	
	if( stack->defsp > end ){
		stack->defsp -= end - start;
	}else if( stack->defsp > start ){
		stack->defsp = start;
	}
	
	stack->sp -= end - start;
}

///
///
///

/*==========================================
 * Release script dependent variable, dependent variable of function
 *------------------------------------------*/
void script_free_vars(struct DBMap* storage)
{
	if( storage ) {
		// destroy the storage construct containing the variables
		db_destroy(storage);
	}
}

void script_free_code(struct script_code* code)
{
	nullpo_retv(code);

	if (code->instances)
		script_stop_scriptinstances(code);
	script_free_vars(code->local.vars);
	if (code->local.arrays)
		code->local.arrays->destroy(code->local.arrays, script_free_array_db);
	aFree(code->script_buf);
	aFree(code);
}

/// Creates a new script state.
///
/// @param script Script code
/// @param pos Position in the code
/// @param rid Who is running the script (attached player)
/// @param oid Where the code is being run (npc 'object')
/// @return Script state
struct script_state* script_alloc_state(struct script_code* rootscript, int pos, int rid, int oid)
{
	struct script_state* st;

	st = ers_alloc(st_ers, struct script_state);
	st->stack = ers_alloc(stack_ers, struct script_stack);
	st->stack->sp = 0;
	st->stack->sp_max = 64;
	CREATE(st->stack->stack_data, struct script_data, st->stack->sp_max);
	st->stack->defsp = st->stack->sp;
	st->stack->scope.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	st->stack->scope.arrays = NULL;
	st->state = RUN;
	st->script = rootscript;
	st->pos = pos;
	st->rid = rid;
	st->oid = oid;
	st->sleep.timer = INVALID_TIMER;
	st->npc_item_flag = battle_config.item_enabled_npc;
	
	if( st->script->instances != USHRT_MAX )
		st->script->instances++;
	else {
		struct npc_data *nd = map_id2nd(oid);

		ShowError("Over 65k instances of '%s' script are being run!\n",nd ? nd->name : "unknown");
	}

	if (!st->script->local.vars)
		st->script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);

	st->id = next_id++;
	active_scripts++;

	idb_put(st_db, st->id, st);

	return st;
}

/// Frees a script state.
///
/// @param st Script state
void script_free_state(struct script_state* st)
{
	if (idb_exists(st_db, st->id)) {
		struct map_session_data *sd = st->rid ? map_id2sd(st->rid) : NULL;

		if (st->bk_st) // backup was not restored
			ShowDebug("script_free_state: Previous script state lost (rid=%d, oid=%d, state=%d, bk_npcid=%d).\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);

		if (sd && sd->st == st) { // Current script is aborted.
			if(sd->state.using_fake_npc) {
				clif_clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
				sd->state.using_fake_npc = 0;
			}
			sd->st = NULL;
			sd->npc_id = 0;
		}

		if (st->sleep.timer != INVALID_TIMER)
			delete_timer(st->sleep.timer, run_script_timer);
		if (st->stack) {
			script_free_vars(st->stack->scope.vars);
			if (st->stack->scope.arrays)
				st->stack->scope.arrays->destroy(st->stack->scope.arrays, script_free_array_db);
			pop_stack(st, 0, st->stack->sp);
			aFree(st->stack->stack_data);
			ers_free(stack_ers, st->stack);
			st->stack = NULL;
		}
		if (st->script && st->script->instances != USHRT_MAX && --st->script->instances == 0) {
			if (st->script->local.vars && !db_size(st->script->local.vars)) {
				script_free_vars(st->script->local.vars);
				st->script->local.vars = NULL;
			}
			if (st->script->local.arrays && !db_size(st->script->local.arrays)) {
				st->script->local.arrays->destroy(st->script->local.arrays, script_free_array_db);
				st->script->local.arrays = NULL;
			}
		}
		st->pos = -1;

		idb_remove(st_db, st->id);
		ers_free(st_ers, st);
		if (--active_scripts == 0)
			next_id = 0;
	}
}

//
// Main execution unit
//
/*==========================================
 * Read command
 *------------------------------------------*/
c_op get_com(unsigned char *script,int *pos)
{
	int i = 0, j = 0;

	if(script[*pos]>=0x80){
		return C_INT;
	}
	while(script[*pos]>=0x40){
		i=script[(*pos)++]<<j;
		j+=6;
	}
	return (c_op)(i+(script[(*pos)++]<<j));
}

/*==========================================
 *  Income figures
 *------------------------------------------*/
int get_num(unsigned char *script,int *pos)
{
	int i,j;
	i=0; j=0;
	while(script[*pos]>=0xc0){
		i+=(script[(*pos)++]&0x7f)<<j;
		j+=6;
	}
	return i+((script[(*pos)++]&0x7f)<<j);
}

/// Ternary operators
/// test ? if_true : if_false
void op_3(struct script_state* st, int op)
{
	struct script_data* data;
	int flag = 0;

	data = script_getdatatop(st, -3);
	get_val(st, data);

	if( data_isstring(data) )
		flag = data->u.str[0];// "" -> false
	else if( data_isint(data) )
		flag = data->u.num == 0 ? 0 : 1;// 0 -> false
	else
	{
		ShowError("script:op_3: invalid data for the ternary operator test\n");
		script_reportdata(data);
		script_reportsrc(st);
		script_removetop(st, -3, 0);
		script_pushnil(st);
		return;
	}
	if( flag )
		script_pushcopytop(st, -2);
	else
		script_pushcopytop(st, -1);
	script_removetop(st, -4, -1);
}

/// Binary string operators
/// s1 EQ s2 -> i
/// s1 NE s2 -> i
/// s1 GT s2 -> i
/// s1 GE s2 -> i
/// s1 LT s2 -> i
/// s1 LE s2 -> i
/// s1 ADD s2 -> s
void op_2str(struct script_state* st, int op, const char* s1, const char* s2)
{
	int a = 0;

	switch(op){
		case C_EQ: a = (strcmp(s1,s2) == 0); break;
		case C_NE: a = (strcmp(s1,s2) != 0); break;
		case C_GT: a = (strcmp(s1,s2) >  0); break;
		case C_GE: a = (strcmp(s1,s2) >= 0); break;
		case C_LT: a = (strcmp(s1,s2) <  0); break;
		case C_LE: a = (strcmp(s1,s2) <= 0); break;
		case C_ADD:
			{
				char* buf = (char *)aMalloc((strlen(s1)+strlen(s2)+1)*sizeof(char));
				strcpy(buf, s1);
				strcat(buf, s2);
				script_pushstr(st, buf);
				return;
			}
		default:
			ShowError("script:op2_str: unexpected string operator %s\n", script_op2name(op));
			script_reportsrc(st);
			script_pushnil(st);
			st->state = END;
			return;
	}

	script_pushint(st,a);
}

/// Binary number operators
/// i OP i -> i
void op_2num(struct script_state* st, int op, int i1, int i2)
{
	int ret;
	double ret_double;

	switch( op ) {
		case C_AND:  ret = i1 & i2;		break;
		case C_OR:   ret = i1 | i2;		break;
		case C_XOR:  ret = i1 ^ i2;		break;
		case C_LAND: ret = (i1 && i2);	break;
		case C_LOR:  ret = (i1 || i2);	break;
		case C_EQ:   ret = (i1 == i2);	break;
		case C_NE:   ret = (i1 != i2);	break;
		case C_GT:   ret = (i1 >  i2);	break;
		case C_GE:   ret = (i1 >= i2);	break;
		case C_LT:   ret = (i1 <  i2);	break;
		case C_LE:   ret = (i1 <= i2);	break;
		case C_R_SHIFT: ret = i1>>i2;	break;
		case C_L_SHIFT: ret = i1<<i2;	break;
		case C_DIV:
		case C_MOD:
			if( i2 == 0 )
			{
				ShowError("script:op_2num: division by zero detected op=%s i1=%d i2=%d\n", script_op2name(op), i1, i2);
				script_reportsrc(st);
				script_pushnil(st);
				st->state = END;
				return;
			}
			else if( op == C_DIV )
				ret = i1 / i2;
			else//if( op == C_MOD )
				ret = i1 % i2;
			break;
		default:
			switch( op ) {// operators that can overflow/underflow
				case C_ADD: ret = i1 + i2; ret_double = (double)i1 + (double)i2; break;
				case C_SUB: ret = i1 - i2; ret_double = (double)i1 - (double)i2; break;
				case C_MUL: ret = i1 * i2; ret_double = (double)i1 * (double)i2; break;
				default:
					ShowError("script:op_2num: unexpected number operator %s i1=%d i2=%d\n", script_op2name(op), i1, i2);
					script_reportsrc(st);
					script_pushnil(st);
					return;
			}
			if( ret_double < (double)INT_MIN ) {
				ShowWarning("script:op_2num: underflow detected op=%s i1=%d i2=%d\n", script_op2name(op), i1, i2);
				script_reportsrc(st);
				ret = INT_MIN;
			}
			else if( ret_double > (double)INT_MAX ) {
				ShowWarning("script:op_2num: overflow detected op=%s i1=%d i2=%d\n", script_op2name(op), i1, i2);
				script_reportsrc(st);
				ret = INT_MAX;
			}
	}
	script_pushint(st, ret);
}

/// Binary operators
void op_2(struct script_state *st, int op)
{
	struct script_data* left, leftref;
	struct script_data* right;

	leftref.type = C_NOP;

	left = script_getdatatop(st, -2);
	right = script_getdatatop(st, -1);

	if (st->op2ref) {
		if (data_isreference(left)) {
			leftref = *left;
		}

		st->op2ref = 0;
	}

	get_val(st, left);
	get_val(st, right);

	// automatic conversions
	switch( op )
	{
		case C_ADD:
			if( data_isint(left) && data_isstring(right) )
			{// convert int-string to string-string
				conv_str(st, left);
			}
			else if( data_isstring(left) && data_isint(right) )
			{// convert string-int to string-string
				conv_str(st, right);
			}
			break;
	}

	if( data_isstring(left) && data_isstring(right) )
	{// ss => op_2str
		op_2str(st, op, left->u.str, right->u.str);
		script_removetop(st, leftref.type == C_NOP ? -3 : -2, -1);// pop the two values before the top one

		if (leftref.type != C_NOP)
		{
			if (left->type == C_STR) // don't free C_CONSTSTR
				aFree(left->u.str);
			*left = leftref;
		}
	}
	else if( data_isint(left) && data_isint(right) )
	{// ii => op_2num
		int i1 = (int)left->u.num;
		int i2 = (int)right->u.num;

		script_removetop(st, leftref.type == C_NOP ? -2 : -1, 0);
		op_2num(st, op, i1, i2);

		if (leftref.type != C_NOP)
			*left = leftref;
	}
	else
	{// invalid argument
		ShowError("script:op_2: invalid data for operator %s\n", script_op2name(op));
		script_reportdata(left);
		script_reportdata(right);
		script_reportsrc(st);
		script_removetop(st, -2, 0);
		script_pushnil(st);
		st->state = END;
	}
}

/// Unary operators
/// NEG i -> i
/// NOT i -> i
/// LNOT i -> i
void op_1(struct script_state* st, int op)
{
	struct script_data* data;
	int i1;

	data = script_getdatatop(st, -1);
	get_val(st, data);

	if( !data_isint(data) )
	{// not a number
		ShowError("script:op_1: argument is not a number (op=%s)\n", script_op2name(op));
		script_reportdata(data);
		script_reportsrc(st);
		script_pushnil(st);
		st->state = END;
		return;
	}

	i1 = (int)data->u.num;
	script_removetop(st, -1, 0);
	switch( op )
	{
		case C_NEG: i1 = -i1; break;
		case C_NOT: i1 = ~i1; break;
		case C_LNOT: i1 = !i1; break;
		default:
			ShowError("script:op_1: unexpected operator %s i1=%d\n", script_op2name(op), i1);
			script_reportsrc(st);
			script_pushnil(st);
			st->state = END;
			return;
	}
	script_pushint(st, i1);
}


/// Checks the type of all arguments passed to a built-in function.
///
/// @param st Script state whose stack arguments should be inspected.
/// @param func Built-in function for which the arguments are intended.
static void script_check_buildin_argtype(struct script_state* st, int func)
{
	int idx, invalid = 0;

	for( idx = 2; script_hasdata(st, idx); idx++ )
	{
		struct script_data* data = script_getdata(st, idx);
		script_function* sf = &buildin_func[str_data[func].val];
		char type = sf->arg[idx-2];

		if( type == '?' || type == '*' )
		{// optional argument or unknown number of optional parameters ( no types are after this )
			break;
		}
		else if( type == 0 )
		{// more arguments than necessary ( should not happen, as it is checked before )
			ShowWarning("Found more arguments than necessary. unexpected arg type %s\n",script_op2name(data->type));
			invalid++;
			break;
		}
		else
		{
			const char* name = NULL;

			if( data_isreference(data) )
			{// get name for variables to determine the type they refer to
				name = reference_getname(data);
			}

			switch( type )
			{
				case 'v':
					if( !data_isstring(data) && !data_isint(data) && !data_isreference(data) )
					{// variant
						ShowWarning("Unexpected type for argument %d. Expected string, number or variable.\n", idx-1);
						script_reportdata(data);
						invalid++;
					}
					break;
				case 's':
					if( !data_isstring(data) && !( data_isreference(data) && is_string_variable(name) ) )
					{// string
						ShowWarning("Unexpected type for argument %d. Expected string.\n", idx-1);
						script_reportdata(data);
						invalid++;
					}
					break;
				case 'i':
					if( !data_isint(data) && !( data_isreference(data) && ( reference_toparam(data) || reference_toconstant(data) || !is_string_variable(name) ) ) )
					{// int ( params and constants are always int )
						ShowWarning("Unexpected type for argument %d. Expected number.\n", idx-1);
						script_reportdata(data);
						invalid++;
					}
					break;
				case 'r':
					if( !data_isreference(data) )
					{// variables
						ShowWarning("Unexpected type for argument %d. Expected variable, got %s.\n", idx-1,script_op2name(data->type));
						script_reportdata(data);
						invalid++;
					}
					break;
				case 'l':
					if( !data_islabel(data) && !data_isfunclabel(data) )
					{// label
						ShowWarning("Unexpected type for argument %d. Expected label, got %s\n", idx-1,script_op2name(data->type));
						script_reportdata(data);
						invalid++;
					}
					break;
			}
		}
	}

	if(invalid)
	{
		ShowDebug("Function: %s\n", get_str(func));
		script_reportsrc(st);
	}
}


/// Executes a buildin command.
/// Stack: C_NAME(<command>) C_ARG <arg0> <arg1> ... <argN>
int run_func(struct script_state *st)
{
	struct script_data* data;
	int i,start_sp,end_sp,func;

	end_sp = st->stack->sp;// position after the last argument
	for( i = end_sp-1; i > 0 ; --i )
		if( st->stack->stack_data[i].type == C_ARG )
			break;
	if( i == 0 ) {
		ShowError("script:run_func: C_ARG not found. please report this!!!\n");
		st->state = END;
		script_reportsrc(st);
		return 1;
	}
	start_sp = i-1;// C_NAME of the command
	st->start = start_sp;
	st->end = end_sp;

	data = &st->stack->stack_data[st->start];
	if( data->type == C_NAME && str_data[data->u.num].type == C_FUNC ) {
		func = (int)data->u.num;
		st->funcname = reference_getname(data);
	} else {
		ShowError("script:run_func: not a buildin command.\n");
		script_reportdata(data);
		script_reportsrc(st);
		st->state = END;
		return 1;
	}

	if( script_config.warn_func_mismatch_argtypes ) {
		script_check_buildin_argtype(st, func);
	}

	if(str_data[func].func) {
#if defined(SCRIPT_COMMAND_DEPRECATION)
		if( buildin_func[str_data[func].val].deprecated ){
			ShowWarning( "Usage of deprecated script function '%s'.\n", get_str(func) );
			ShowWarning( "This function was deprecated on '%s' and could become unavailable anytime soon.\n", buildin_func[str_data[func].val].deprecated );
			script_reportsrc(st);
		}
#endif

		if (str_data[func].func(st) == SCRIPT_CMD_FAILURE) //Report error
			script_reportsrc(st);
	} else {
		ShowError("script:run_func: '%s' (id=%d type=%s) has no C function. please report this!!!\n", get_str(func), func, script_op2name(str_data[func].type));
		script_reportsrc(st);
		st->state = END;
	}

	// Stack's datum are used when re-running functions [Eoe]
	if( st->state == RERUNLINE )
		return 0;

	pop_stack(st, st->start, st->end);
	if( st->state == RETFUNC ) {// return from a user-defined function
		struct script_retinfo* ri;
		int olddefsp = st->stack->defsp;
		int nargs;

		pop_stack(st, st->stack->defsp, st->start);// pop distractions from the stack
		if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp-1].type != C_RETINFO )
		{
			ShowWarning("script:run_func: return without callfunc or callsub!\n");
			script_reportsrc(st);
			st->state = END;
			return 1;
		}
		script_free_vars(st->stack->scope.vars);
		st->stack->scope.arrays->destroy(st->stack->scope.arrays, script_free_array_db);

		ri = st->stack->stack_data[st->stack->defsp-1].u.ri;
		nargs = ri->nargs;
		st->pos = ri->pos;
		st->script = ri->script;
		st->stack->scope.vars = ri->scope.vars;
		st->stack->scope.arrays = ri->scope.arrays;
		st->stack->defsp = ri->defsp;
		memset(ri, 0, sizeof(struct script_retinfo));

		pop_stack(st, olddefsp-nargs-1, olddefsp);// pop arguments and retinfo

		st->state = GOTO;
	}

	return 0;
}

/*==========================================
 * script execution
 *------------------------------------------*/
void run_script(struct script_code *rootscript, int pos, int rid, int oid)
{
	struct script_state *st;

	if( rootscript == NULL || pos < 0 )
		return;

	// TODO In jAthena, this function can take over the pending script in the player. [FlavioJS]
	//      It is unclear how that can be triggered, so it needs the be traced/checked in more detail.
	// NOTE At the time of this change, this function wasn't capable of taking over the script state because st->scriptroot was never set.
	st = script_alloc_state(rootscript, pos, rid, oid);
	run_script_main(st);
}

/**
 * Free all related script code
 * @param code: Script code to free
 */
void script_stop_scriptinstances(struct script_code *code) {
	DBIterator *iter;
	struct script_state* st;

	if( !active_scripts )
		return; // Don't even bother.

	iter = db_iterator(st_db);

	for( st = static_cast<script_state *>(dbi_first(iter)); dbi_exists(iter); st = static_cast<script_state *>(dbi_next(iter)) ) {
		if( st->script == code )
			script_free_state(st);
	}

	dbi_destroy(iter);
}

/*==========================================
 * Timer function for sleep
 *------------------------------------------*/
TIMER_FUNC(run_script_timer){
	struct script_state *st = (struct script_state *)data;
	struct linkdb_node *node = (struct linkdb_node *)sleep_db;

	// If it was a player before going to sleep and there is still a unit attached to the script
	if( id != 0 && st->rid ){
		struct map_session_data *sd = map_id2sd(st->rid);

		// Attached player is offline(logout) or another unit type(should not happen)
		if( !sd ){
			st->rid = 0;
			st->state = END;
		// Character mismatch. Cancel execution.
		}else if( sd->status.char_id != id ){
			ShowWarning( "Script sleep timer detected a character mismatch CID %d != %d\n", sd->status.char_id, id );
			script_reportsrc(st);
			st->rid = 0;
			st->state = END;
		}
	}

	while (node && st->sleep.timer != INVALID_TIMER) {
		if ((int)__64BPRTSIZE(node->key) == st->oid && ((struct script_state *)node->data)->sleep.timer == st->sleep.timer) {
			script_erase_sleepdb(node);
			st->sleep.timer = INVALID_TIMER;
			break;
		}
		node = node->next;
	}
	if(st->state != RERUNLINE)
		st->sleep.tick = 0;
	run_script_main(st);
	return 0;
}

/**
 * Remove sleep timers from the NPC
 * @param id: NPC ID
 */
void script_stop_sleeptimers(int id) {
	for (;;) {
		struct script_state *st = (struct script_state *)linkdb_erase(&sleep_db, (void *)__64BPRTSIZE(id));

		if (!st)
			break; // No more sleep timers

		if (st->oid == id)
			script_free_state(st);
	}
}

/**
 * Delete the specified node from sleep_db
 * @param n: Linked list of sleep timers
 */
struct linkdb_node *script_erase_sleepdb(struct linkdb_node *n) {
	struct linkdb_node *retnode;

	if (!n)
		return NULL;

	if (!n->prev)
		sleep_db = n->next;
	else
		n->prev->next = n->next;

	if (n->next)
		n->next->prev = n->prev;

	retnode = n->next;
	aFree(n);

	return retnode;
}

/// Detaches script state from possibly attached character and restores it's previous script if any.
///
/// @param st Script state to detach.
/// @param dequeue_event Whether to schedule any queued events, when there was no previous script.
static void script_detach_state(struct script_state* st, bool dequeue_event)
{
	struct map_session_data* sd;

	if(st->rid && (sd = map_id2sd(st->rid))!=NULL) {
		sd->st = st->bk_st;
		sd->npc_id = st->bk_npcid;
		sd->state.disable_atcommand_on_npc = 0;
		if(st->bk_st) {
			//Remove tag for removal.
			st->bk_st = NULL;
			st->bk_npcid = 0;
		} else if(dequeue_event) {

#ifdef SECURE_NPCTIMEOUT
			/**
			 * We're done with this NPC session, so we cancel the timer (if existent) and move on
			 **/
			if( sd->npc_idle_timer != INVALID_TIMER ) {
				delete_timer(sd->npc_idle_timer,npc_secure_timeout_timer);
				sd->npc_idle_timer = INVALID_TIMER;
			}
#endif
			npc_event_dequeue(sd);
		}
	}
	else if(st->bk_st)
	{// rid was set to 0, before detaching the script state
		ShowError("script_detach_state: Found previous script state without attached player (rid=%d, oid=%d, state=%d, bk_npcid=%d)\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
		script_reportsrc(st->bk_st);

		script_free_state(st->bk_st);
		st->bk_st = NULL;
	}
}

/// Attaches script state to possibly attached character and backups it's previous script, if any.
///
/// @param st Script state to attach.
void script_attach_state(struct script_state* st){
	struct map_session_data* sd;

	if(st->rid && (sd = map_id2sd(st->rid))!=NULL)
	{
		if(st!=sd->st)
		{
			if(st->bk_st)
			{// there is already a backup
				ShowDebug("script_attach_state: Previous script state lost (rid=%d, oid=%d, state=%d, bk_npcid=%d).\n", st->bk_st->rid, st->bk_st->oid, st->bk_st->state, st->bk_npcid);
			}
			st->bk_st = sd->st;
			st->bk_npcid = sd->npc_id;
		}
		sd->st = st;
		sd->npc_id = st->oid;
		sd->npc_item_flag = st->npc_item_flag; // load default.
		sd->state.disable_atcommand_on_npc = (!pc_has_permission(sd, PC_PERM_ENABLE_COMMAND));
#ifdef SECURE_NPCTIMEOUT
		if( sd->npc_idle_timer == INVALID_TIMER )
			sd->npc_idle_timer = add_timer(gettick() + (SECURE_NPCTIMEOUT_INTERVAL*1000),npc_secure_timeout_timer,sd->bl.id,0);
		sd->npc_idle_tick = gettick();
#endif
	}
}

/*==========================================
 * The main part of the script execution
 *------------------------------------------*/
void run_script_main(struct script_state *st)
{
	int cmdcount = script_config.check_cmdcount;
	int gotocount = script_config.check_gotocount;
	TBL_PC *sd;
	struct script_stack *stack = st->stack;

	script_attach_state(st);

	if(st->state == RERUNLINE) {
		run_func(st);
		if(st->state == GOTO)
			st->state = RUN;
	} else if(st->state != END)
		st->state = RUN;

	while(st->state == RUN) {
		enum c_op c = get_com(st->script->script_buf,&st->pos);
		switch(c){
		case C_EOL:
			if( stack->defsp > stack->sp )
				ShowError("script:run_script_main: unexpected stack position (defsp=%d sp=%d). please report this!!!\n", stack->defsp, stack->sp);
			else
				pop_stack(st, stack->defsp, stack->sp);// pop unused stack data. (unused return value)
			break;
		case C_INT:
			push_val(stack,C_INT,get_num(st->script->script_buf,&st->pos));
			break;
		case C_POS:
		case C_NAME:
			push_val(stack,c,GETVALUE(st->script->script_buf,st->pos));
			st->pos+=3;
			break;
		case C_ARG:
			push_val(stack,c,0);
			break;
		case C_STR:
			push_str(stack,C_CONSTSTR,(char*)(st->script->script_buf+st->pos));
			while(st->script->script_buf[st->pos++]);
			break;
		case C_FUNC:
			run_func(st);
			if(st->state==GOTO){
				st->state = RUN;
				if( !st->freeloop && gotocount>0 && (--gotocount)<=0 ){
					ShowError("script:run_script_main: infinity loop !\n");
					script_reportsrc(st);
					st->state=END;
				}
			}
			break;

		case C_REF:
			st->op2ref = 1;
			break;

		case C_NEG:
		case C_NOT:
		case C_LNOT:
			op_1(st ,c);
			break;

		case C_ADD:
		case C_SUB:
		case C_MUL:
		case C_DIV:
		case C_MOD:
		case C_EQ:
		case C_NE:
		case C_GT:
		case C_GE:
		case C_LT:
		case C_LE:
		case C_AND:
		case C_OR:
		case C_XOR:
		case C_LAND:
		case C_LOR:
		case C_R_SHIFT:
		case C_L_SHIFT:
			op_2(st, c);
			break;

		case C_OP3:
			op_3(st, c);
			break;

		case C_NOP:
			st->state=END;
			break;

		default:
			ShowError("script:run_script_main:unknown command : %d @ %d\n",c,st->pos);
			st->state=END;
			break;
		}
		if( !st->freeloop && cmdcount>0 && (--cmdcount)<=0 ){
			ShowError("script:run_script_main: infinity loop !\n");
			script_reportsrc(st);
			st->state=END;
		}
	}

	if(st->sleep.tick > 0) {
		//Restore previous script
		script_detach_state(st, false);
		//Delay execution
		sd = map_id2sd(st->rid); // Get sd since script might have attached someone while running. [Inkfish]
		st->sleep.charid = sd?sd->status.char_id:0;
		st->sleep.timer = add_timer(gettick() + st->sleep.tick, run_script_timer, st->sleep.charid, (intptr_t)st);
		linkdb_insert(&sleep_db, (void *)__64BPRTSIZE(st->oid), st);
	} else if(st->state != END && st->rid) {
		//Resume later (st is already attached to player).
		if(st->bk_st) {
			ShowWarning("Unable to restore stack! Double continuation!\n");
			//Report BOTH scripts to see if that can help somehow.
			ShowDebug("Previous script (lost):\n");
			script_reportsrc(st->bk_st);
			ShowDebug("Current script:\n");
			script_reportsrc(st);

			script_free_state(st->bk_st);
			st->bk_st = NULL;
		}
	} else {
		//Dispose of script.
		if ((sd = map_id2sd(st->rid))!=NULL)
		{	//Restore previous stack and save char.
			if(sd->state.using_fake_npc){
				clif_clearunit_single(sd->npc_id, CLR_OUTSIGHT, sd->fd);
				sd->state.using_fake_npc = 0;
			}
			//Restore previous script if any.
			script_detach_state(st, true);
			if (sd->vars_dirty)
				intif_saveregistry(sd);
		}
		script_free_state(st);
	}
}

int script_config_read(const char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;


	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%1023[^:]: %1023[^\r\n]",w1,w2);
		if(i!=2)
			continue;

		if(strcmpi(w1,"warn_func_mismatch_paramnum")==0) {
			script_config.warn_func_mismatch_paramnum = config_switch(w2);
		}
		else if(strcmpi(w1,"check_cmdcount")==0) {
			script_config.check_cmdcount = config_switch(w2);
		}
		else if(strcmpi(w1,"check_gotocount")==0) {
			script_config.check_gotocount = config_switch(w2);
		}
		else if(strcmpi(w1,"input_min_value")==0) {
			script_config.input_min_value = config_switch(w2);
		}
		else if(strcmpi(w1,"input_max_value")==0) {
			script_config.input_max_value = config_switch(w2);
		}
		else if(strcmpi(w1,"warn_func_mismatch_argtypes")==0) {
			script_config.warn_func_mismatch_argtypes = config_switch(w2);
		}
		else if(strcmpi(w1,"import")==0){
			script_config_read(w2);
		}
		else {
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
		}
	}
	fclose(fp);

	return 0;
}

/**
 * @see DBApply
 */
static int db_script_free_code_sub(DBKey key, DBData *data, va_list ap)
{
	struct script_code *code = static_cast<script_code *>(db_data2ptr(data));
	if (code)
		script_free_code(code);
	return 0;
}

void script_run_autobonus(const char *autobonus, struct map_session_data *sd, unsigned int pos)
{
	struct script_code *script = (struct script_code *)strdb_get(autobonus_db, autobonus);

	if( script )
	{
		int j;
		ARR_FIND( 0, EQI_MAX, j, sd->equip_index[j] >= 0 && sd->inventory.u.items_inventory[sd->equip_index[j]].equip == pos );
		if( j < EQI_MAX ) {
			//Single item autobonus
			current_equip_item_index = sd->equip_index[j];
			current_equip_combo_pos = 0;
		} else {
			//Combo autobonus
			current_equip_item_index = -1;
			current_equip_combo_pos = pos;
		}
		run_script(script,0,sd->bl.id,0);
	}
}

void script_add_autobonus(const char *autobonus)
{
	if( strdb_get(autobonus_db, autobonus) == NULL )
	{
		struct script_code *script = parse_script(autobonus, "autobonus", 0, 0);

		if( script )
			strdb_put(autobonus_db, autobonus, script);
	}
}


/// resets a temporary character array variable to given value
void script_cleararray_pc(struct map_session_data* sd, const char* varname, void* value)
{
	struct script_array *sa = NULL;
	struct reg_db *src = NULL;
	unsigned int i, *list = NULL, size = 0;
	int key;

	key = add_str(varname);

	if( !(src = script_array_src(NULL,sd,varname,NULL) ) )
		return;

	if( value )
		script_array_ensure_zero(NULL,sd,reference_uid(key,0), NULL);

	if( !(sa = static_cast<script_array *>(idb_get(src->arrays, key))) ) /* non-existent array, nothing to empty */
		return;

	size = sa->size;
	list = script_array_cpy_list(sa);

	for(i = 0; i < size; i++) {
		set_reg(NULL,sd,reference_uid(key, list[i]),varname,value,NULL);
	}
}


/// sets a temporary character array variable element idx to given value
/// @param refcache Pointer to an int variable, which keeps a copy of the reference to varname and must be initialized to 0. Can be NULL if only one element is set.
void script_setarray_pc(struct map_session_data* sd, const char* varname, uint32 idx, void* value, int* refcache)
{
	int key;

	if( idx >= SCRIPT_MAX_ARRAYSIZE ) {
		ShowError("script_setarray_pc: Variable '%s' has invalid index '%u' (char_id=%d).\n", varname, idx, sd->status.char_id);
		return;
	}

	key = ( refcache && refcache[0] ) ? refcache[0] : add_str(varname);

	set_reg(NULL,sd,reference_uid(key, idx),varname,value,NULL);

	if( refcache ) {
		// save to avoid repeated script->add_str calls
		refcache[0] = key;
	}
}

/**
 * Clears persistent variables from memory
 **/
int script_reg_destroy(DBKey key, DBData *data, va_list ap)
{
	struct script_reg_state *src;

	if( data->type != DB_DATA_PTR ) // got no need for those!
		return 0;

	src = static_cast<script_reg_state *>(db_data2ptr(data));

	if( src->type ) {
		struct script_reg_str *p = (struct script_reg_str *)src;

		if( p->value )
			aFree(p->value);

		ers_free(str_reg_ers,p);
	} else {
		ers_free(num_reg_ers,(struct script_reg_num*)src);
	}

	return 0;
}

/**
 * Clears a single persistent variable
 **/
void script_reg_destroy_single(struct map_session_data *sd, int64 reg, struct script_reg_state *data)
{
	i64db_remove(sd->regs.vars, reg);

	if( data->type ) {
		struct script_reg_str *p = (struct script_reg_str*)data;

		if( p->value )
			aFree(p->value);

		ers_free(str_reg_ers,p);
	} else {
		ers_free(num_reg_ers,(struct script_reg_num*)data);
	}
}

unsigned int *script_array_cpy_list(struct script_array *sa)
{
	if( sa->size > generic_ui_array_size )
		script_generic_ui_array_expand(sa->size);
	memcpy(generic_ui_array, sa->members, sizeof(unsigned int)*sa->size);
	return generic_ui_array;
}

void script_generic_ui_array_expand (unsigned int plus)
{
	generic_ui_array_size += plus + 100;
	RECREATE(generic_ui_array, unsigned int, generic_ui_array_size);
}

/*==========================================
 * Destructor
 *------------------------------------------*/
void do_final_script() {
	int i;
	DBIterator *iter;
	struct script_state *st;

#ifdef DEBUG_HASH
	if (battle_config.etc_log)
	{
		FILE *fp = fopen("hash_dump.txt","wt");
		if(fp) {
			int count[SCRIPT_HASH_SIZE];
			int count2[SCRIPT_HASH_SIZE]; // number of buckets with a certain number of items
			int n=0;
			int min=INT_MAX,max=0,zero=0;
			double mean=0.0f;
			double median=0.0f;

			ShowNotice("Dumping script str hash information to hash_dump.txt\n");
			memset(count, 0, sizeof(count));
			fprintf(fp,"num : hash : data_name\n");
			fprintf(fp,"---------------------------------------------------------------\n");
			for(i=LABEL_START; i<str_num; i++) {
				unsigned int h = calc_hash(get_str(i));
				fprintf(fp,"%04d : %4u : %s\n",i,h, get_str(i));
				++count[h];
			}
			fprintf(fp,"--------------------\n\n");
			memset(count2, 0, sizeof(count2));
			for(i=0; i<SCRIPT_HASH_SIZE; i++) {
				fprintf(fp,"  hash %3d = %d\n",i,count[i]);
				if(min > count[i])
					min = count[i];		// minimun count of collision
				if(max < count[i])
					max = count[i];		// maximun count of collision
				if(count[i] == 0)
					zero++;
				++count2[count[i]];
			}
			fprintf(fp,"\n--------------------\n  items : buckets\n--------------------\n");
			for( i=min; i <= max; ++i ){
				fprintf(fp,"  %5d : %7d\n",i,count2[i]);
				mean += 1.0f*i*count2[i]/SCRIPT_HASH_SIZE; // Note: this will always result in <nr labels>/<nr buckets>
			}
			for( i=min; i <= max; ++i ){
				n += count2[i];
				if( n*2 >= SCRIPT_HASH_SIZE )
				{
					if( SCRIPT_HASH_SIZE%2 == 0 && SCRIPT_HASH_SIZE/2 == n )
						median = (i+i+1)/2.0f;
					else
						median = i;
					break;
				}
			}
			fprintf(fp,"--------------------\n    min = %d, max = %d, zero = %d\n    mean = %lf, median = %lf\n",min,max,zero,mean,median);
			fclose(fp);
		}
	}
#endif

	mapreg_final();

	db_destroy(scriptlabel_db);
	userfunc_db->destroy(userfunc_db, db_script_free_code_sub);
	autobonus_db->destroy(autobonus_db, db_script_free_code_sub);

	ers_destroy(array_ers);
	if (generic_ui_array)
		aFree(generic_ui_array);

	iter = db_iterator(st_db);
	for( st = static_cast<script_state *>(dbi_first(iter)); dbi_exists(iter); st = static_cast<script_state *>(dbi_next(iter)) )
		script_free_state(st);
	dbi_destroy(iter);

	if (str_data)
		aFree(str_data);
	if (str_buf)
		aFree(str_buf);

	for( i = 0; i < atcmd_binding_count; i++ ) {
		aFree(atcmd_binding[i]);
	}

	if( atcmd_binding_count != 0 )
		aFree(atcmd_binding);

	ers_destroy(st_ers);
	ers_destroy(stack_ers);
	db_destroy(st_db);
}
/*==========================================
 * Initialization
 *------------------------------------------*/
void do_init_script(void) {
	st_db = idb_alloc(DB_OPT_BASE);
	userfunc_db = strdb_alloc(DB_OPT_DUP_KEY,0);
	scriptlabel_db = strdb_alloc(DB_OPT_DUP_KEY,50);
	autobonus_db = strdb_alloc(DB_OPT_DUP_KEY,0);

	st_ers = ers_new(sizeof(struct script_state), "script.cpp::st_ers", ERS_CACHE_OPTIONS);
	stack_ers = ers_new(sizeof(struct script_stack), "script.cpp::script_stack", ERS_OPT_FLEX_CHUNK);
	array_ers = ers_new(sizeof(struct script_array), "script.cpp:array_ers", ERS_CLEAN_OPTIONS);

	ers_chunk_size(st_ers, 10);
	ers_chunk_size(stack_ers, 10);

	active_scripts = 0;
	next_id = 0;

	mapreg_init();
}

void script_reload(void) {
	int i;
	DBIterator *iter;
	struct script_state *st;

	userfunc_db->clear(userfunc_db, db_script_free_code_sub);
	db_clear(scriptlabel_db);

	// @commands (script based)
	// Clear bindings
	for( i = 0; i < atcmd_binding_count; i++ ) {
		aFree(atcmd_binding[i]);
	}

	if( atcmd_binding_count != 0 )
		aFree(atcmd_binding);

	atcmd_binding_count = 0;

	iter = db_iterator(st_db);
	for( st = static_cast<script_state *>(dbi_first(iter)); dbi_exists(iter); st = static_cast<script_state *>(dbi_next(iter)) )
		script_free_state(st);
	dbi_destroy(iter);
	db_clear(st_db);

	mapreg_reload();
}

//-----------------------------------------------------------------------------
// buildin functions
//

#define BUILDIN_DEF(x,args) { buildin_ ## x , #x , args, NULL }
#define BUILDIN_DEF2(x,x2,args) { buildin_ ## x , x2 , args, NULL }
#define BUILDIN_DEF_DEPRECATED(x,args,deprecationdate) { buildin_ ## x , #x , args, deprecationdate }
#define BUILDIN_DEF2_DEPRECATED(x,x2,args,deprecationdate) { buildin_ ## x , x2 , args, deprecationdate }
#define BUILDIN_FUNC(x) int buildin_ ## x (struct script_state* st)

/////////////////////////////////////////////////////////////////////
// NPC interaction
//

/// Appends a message to the npc dialog.
/// If a dialog doesn't exist yet, one is created.
///
/// mes "<message>";
BUILDIN_FUNC(mes)
{
	TBL_PC* sd;
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	if( !script_hasdata(st, 3) )
	{// only a single line detected in the script
		clif_scriptmes(sd, st->oid, script_getstr(st, 2));
	}
	else
	{// parse multiple lines as they exist
		int i;

		for( i = 2; script_hasdata(st, i); i++ )
		{
			// send the message to the client
			clif_scriptmes(sd, st->oid, script_getstr(st, i));
		}
	}

	st->mes_active = 1; // Invoking character has a NPC dialog box open.
	return SCRIPT_CMD_SUCCESS;
}

/// Displays the button 'next' in the npc dialog.
/// The dialog text is cleared and the script continues when the button is pressed.
///
/// next;
BUILDIN_FUNC(next)
{
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;
#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_WAIT;
#endif
	st->state = STOP;
	clif_scriptnext(sd, st->oid);
	return SCRIPT_CMD_SUCCESS;
}

/// Clears the dialog and continues the script without a next button.
///
/// clear;
BUILDIN_FUNC(clear)
{
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	clif_scriptclear(sd, st->oid);
	return SCRIPT_CMD_SUCCESS;
}

/// Ends the script and displays the button 'close' on the npc dialog.
/// The dialog is closed when the button is pressed.
///
/// close;
BUILDIN_FUNC(close)
{
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	if( !st->mes_active ) {
		TBL_NPC* nd = map_id2nd(st->oid);
		st->state = END; // Keep backwards compatibility.
		ShowWarning("Incorrect use of 'close' command! (source:%s / path:%s)\n",nd?nd->name:"Unknown",nd?nd->path:"Unknown");
	} else {
		st->state = CLOSE;
		st->mes_active = 0;
	}

	clif_scriptclose(sd, st->oid);
	return SCRIPT_CMD_SUCCESS;
}

/// Displays the button 'close' on the npc dialog.
/// The dialog is closed and the script continues when the button is pressed.
///
/// close2;
BUILDIN_FUNC(close2)
{
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	st->state = STOP;

	if( st->mes_active )
		st->mes_active = 0;

	clif_scriptclose(sd, st->oid);
	return SCRIPT_CMD_SUCCESS;
}

/// Counts the number of valid and total number of options in 'str'
/// If max_count > 0 the counting stops when that valid option is reached
/// total is incremented for each option (NULL is supported)
static int menu_countoptions(const char* str, int max_count, int* total)
{
	int count = 0;
	int bogus_total;

	if( total == NULL )
		total = &bogus_total;
	++(*total);

	// initial empty options
	while( *str == ':' )
	{
		++str;
		++(*total);
	}
	// count menu options
	while( *str != '\0' )
	{
		++count;
		--max_count;
		if( max_count == 0 )
			break;
		while( *str != ':' && *str != '\0' )
			++str;
		while( *str == ':' )
		{
			++str;
			++(*total);
		}
	}
	return count;
}

/// Displays a menu with options and goes to the target label.
/// The script is stopped if cancel is pressed.
/// Options with no text are not displayed in the client.
///
/// Options can be grouped together, separated by the character ':' in the text:
///   ex: menu "A:B:C",L_target;
/// All these options go to the specified target label.
///
/// The index of the selected option is put in the variable @menu.
/// Indexes start with 1 and are consistent with grouped and empty options.
///   ex: menu "A::B",-,"",L_Impossible,"C",-;
///       // displays "A", "B" and "C", corresponding to indexes 1, 3 and 5
///
/// NOTE: the client closes the npc dialog when cancel is pressed
///
/// menu "<option_text>",<target_label>{,"<option_text>",<target_label>,...};
BUILDIN_FUNC(menu)
{
	int i;
	const char* text;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_MENU;
#endif

	// TODO detect multiple scripts waiting for input at the same time, and what to do when that happens
	if( sd->state.menu_or_input == 0 )
	{
		struct StringBuf buf;

		if( script_lastdata(st) % 2 == 0 )
		{// argument count is not even (1st argument is at index 2)
			ShowError("buildin_menu: Illegal number of arguments (%d).\n", (script_lastdata(st) - 1));
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}

		StringBuf_Init(&buf);
		sd->npc_menu = 0;
		for( i = 2; i < script_lastdata(st); i += 2 )
		{
			struct script_data* data;
			// menu options
			text = script_getstr(st, i);

			// target label
			data = script_getdata(st, i+1);
			if( !data_islabel(data) )
			{// not a label
				StringBuf_Destroy(&buf);
				ShowError("buildin_menu: Argument #%d (from 1) is not a label or label not found.\n", i);
				script_reportdata(data);
				st->state = END;
				return SCRIPT_CMD_FAILURE;
			}

			// append option(s)
			if( text[0] == '\0' )
				continue;// empty string, ignore
			if( sd->npc_menu > 0 )
				StringBuf_AppendStr(&buf, ":");
			StringBuf_AppendStr(&buf, text);
			sd->npc_menu += menu_countoptions(text, 0, NULL);
		}
		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;

		/**
		 * menus beyond this length crash the client (see bugreport:6402)
		 **/
		if( StringBuf_Length(&buf) >= 2047 ) {
			struct npc_data * nd = map_id2nd(st->oid);
			char* menu;
			CREATE(menu, char, 2048);
			safestrncpy(menu, StringBuf_Value(&buf), 2047);
			ShowWarning("buildin_menu: NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StringBuf_Length(&buf));
			clif_scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif_scriptmenu(sd, st->oid, StringBuf_Value(&buf));

		StringBuf_Destroy(&buf);

		if( sd->npc_menu >= 0xff )
		{// client supports only up to 254 entries; 0 is not used and 255 is reserved for cancel; excess entries are displayed but cause 'uint8' overflow
			ShowWarning("buildin_menu: Too many options specified (current=%d, max=254).\n", sd->npc_menu);
			script_reportsrc(st);
		}
	}
	else if( sd->npc_menu == 0xff )
	{// Cancel was pressed
		sd->state.menu_or_input = 0;
		st->state = END;
	}
	else
	{// goto target label
		int menu = 0;

		sd->state.menu_or_input = 0;
		if( sd->npc_menu <= 0 )
		{
			ShowDebug("buildin_menu: Unexpected selection (%d)\n", sd->npc_menu);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}

		// get target label
		for( i = 2; i < script_lastdata(st); i += 2 )
		{
			text = script_getstr(st, i);
			sd->npc_menu -= menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		if( sd->npc_menu > 0 )
		{// Invalid selection
			ShowDebug("buildin_menu: Selection is out of range (%d pairs are missing?) - please report this\n", sd->npc_menu);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
		if( !data_islabel(script_getdata(st, i + 1)) )
		{// TODO remove this temporary crash-prevention code (fallback for multiple scripts requesting user input)
			ShowError("buildin_menu: Unexpected data in label argument\n");
			script_reportdata(script_getdata(st, i + 1));
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
		pc_setreg(sd, add_str("@menu"), menu);
		st->pos = script_getnum(st, i + 1);
		st->state = GOTO;
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Displays a menu with options and returns the selected option.
/// Behaves like 'menu' without the target labels.
///
/// select(<option_text>{,<option_text>,...}) -> <selected_option>
///
/// @see menu
BUILDIN_FUNC(select)
{
	int i;
	const char* text;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_MENU;
#endif

	if( sd->state.menu_or_input == 0 ) {
		struct StringBuf buf;

		StringBuf_Init(&buf);
		sd->npc_menu = 0;
		for( i = 2; i <= script_lastdata(st); ++i ) {
			text = script_getstr(st, i);

			if( sd->npc_menu > 0 )
				StringBuf_AppendStr(&buf, ":");

			StringBuf_AppendStr(&buf, text);
			sd->npc_menu += menu_countoptions(text, 0, NULL);
		}

		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;

		/**
		 * menus beyond this length crash the client (see bugreport:6402)
		 **/
		if( StringBuf_Length(&buf) >= 2047 ) {
			struct npc_data * nd = map_id2nd(st->oid);
			char* menu;
			CREATE(menu, char, 2048);
			safestrncpy(menu, StringBuf_Value(&buf), 2047);
			ShowWarning("buildin_select: NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StringBuf_Length(&buf));
			clif_scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif_scriptmenu(sd, st->oid, StringBuf_Value(&buf));
		StringBuf_Destroy(&buf);

		if( sd->npc_menu >= 0xff ) {
			ShowWarning("buildin_select: Too many options specified (current=%d, max=254).\n", sd->npc_menu);
			script_reportsrc(st);
		}
	} else if( sd->npc_menu == 0xff ) {// Cancel was pressed
		sd->state.menu_or_input = 0;
		st->state = END;
	} else {// return selected option
		int menu = 0;

		sd->state.menu_or_input = 0;
		for( i = 2; i <= script_lastdata(st); ++i ) {
			text = script_getstr(st, i);
			sd->npc_menu -= menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		pc_setreg(sd, add_str("@menu"), menu);
		script_pushint(st, menu);
		st->state = RUN;
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Displays a menu with options and returns the selected option.
/// Behaves like 'menu' without the target labels, except when cancel is
/// pressed.
/// When cancel is pressed, the script continues and 255 is returned.
///
/// prompt(<option_text>{,<option_text>,...}) -> <selected_option>
///
/// @see menu
BUILDIN_FUNC(prompt)
{
	int i;
	const char *text;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_MENU;
#endif

	if( sd->state.menu_or_input == 0 )
	{
		struct StringBuf buf;

		StringBuf_Init(&buf);
		sd->npc_menu = 0;
		for( i = 2; i <= script_lastdata(st); ++i )
		{
			text = script_getstr(st, i);
			if( sd->npc_menu > 0 )
				StringBuf_AppendStr(&buf, ":");
			StringBuf_AppendStr(&buf, text);
			sd->npc_menu += menu_countoptions(text, 0, NULL);
		}

		st->state = RERUNLINE;
		sd->state.menu_or_input = 1;

		/**
		 * menus beyond this length crash the client (see bugreport:6402)
		 **/
		if( StringBuf_Length(&buf) >= 2047 ) {
			struct npc_data * nd = map_id2nd(st->oid);
			char* menu;
			CREATE(menu, char, 2048);
			safestrncpy(menu, StringBuf_Value(&buf), 2047);
			ShowWarning("buildin_prompt: NPC Menu too long! (source:%s / length:%d)\n",nd?nd->name:"Unknown",StringBuf_Length(&buf));
			clif_scriptmenu(sd, st->oid, menu);
			aFree(menu);
		} else
			clif_scriptmenu(sd, st->oid, StringBuf_Value(&buf));
		StringBuf_Destroy(&buf);

		if( sd->npc_menu >= 0xff )
		{
			ShowWarning("buildin_prompt: Too many options specified (current=%d, max=254).\n", sd->npc_menu);
			script_reportsrc(st);
		}
	}
	else if( sd->npc_menu == 0xff )
	{// Cancel was pressed
		sd->state.menu_or_input = 0;
		pc_setreg(sd, add_str("@menu"), 0xff);
		script_pushint(st, 0xff);
		st->state = RUN;
	}
	else
	{// return selected option
		int menu = 0;

		sd->state.menu_or_input = 0;
		for( i = 2; i <= script_lastdata(st); ++i )
		{
			text = script_getstr(st, i);
			sd->npc_menu -= menu_countoptions(text, sd->npc_menu, &menu);
			if( sd->npc_menu <= 0 )
				break;// entry found
		}
		pc_setreg(sd, add_str("@menu"), menu);
		script_pushint(st, menu);
		st->state = RUN;
	}
	return SCRIPT_CMD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// ...
//

/// Jumps to the target script label.
///
/// goto <label>;
BUILDIN_FUNC(goto)
{
	if( !data_islabel(script_getdata(st,2)) )
	{
		ShowError("buildin_goto: Not a label\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	st->pos = script_getnum(st,2);
	st->state = GOTO;
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * user-defined function call
 *------------------------------------------*/
BUILDIN_FUNC(callfunc)
{
	int i, j;
	struct script_retinfo* ri;
	struct script_code* scr;
	const char* str = script_getstr(st,2);
	struct reg_db *ref = NULL;

	scr = (struct script_code*)strdb_get(userfunc_db, str);
	if(!scr) {
		ShowError("buildin_callfunc: Function not found! [%s]\n", str);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	ref = (struct reg_db *)aCalloc(sizeof(struct reg_db), 2);
	ref[0].vars = st->stack->scope.vars;
	if (!st->stack->scope.arrays)
		st->stack->scope.arrays = idb_alloc(DB_OPT_BASE); // TODO: Can this happen? when?
	ref[0].arrays = st->stack->scope.arrays;
	ref[1].vars = st->script->local.vars;
	if (!st->script->local.arrays)
		st->script->local.arrays = idb_alloc(DB_OPT_BASE); // TODO: Can this happen? when?
	ref[1].arrays = st->script->local.arrays;

	for(i = st->start+3, j = 0; i < st->end; i++, j++) {
		struct script_data* data = push_copy(st->stack,i);

		if (data_isreference(data) && !data->ref) {
			const char* name = reference_getname(data);

			if (name[0] == '.')
				data->ref = (name[1] == '@' ? &ref[0] : &ref[1]);
		}
	}

	CREATE(ri, struct script_retinfo, 1);
	ri->script       = st->script;              // script code
	ri->scope.vars   = st->stack->scope.vars;   // scope variables
	ri->scope.arrays = st->stack->scope.arrays; // scope arrays
	ri->pos          = st->pos;                 // script location
	ri->nargs        = j;                       // argument count
	ri->defsp        = st->stack->defsp;        // default stack pointer
	push_retinfo(st->stack, ri, ref);

	st->pos = 0;
	st->script = scr;
	st->stack->defsp = st->stack->sp;
	st->state = GOTO;
	st->stack->scope.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	st->stack->scope.arrays = idb_alloc(DB_OPT_BASE);

	if (!st->script->local.vars)
		st->script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * subroutine call
 *------------------------------------------*/
BUILDIN_FUNC(callsub)
{
	int i,j;
	struct script_retinfo* ri;
	int pos = script_getnum(st,2);
	struct reg_db *ref = NULL;

	if( !data_islabel(script_getdata(st,2)) && !data_isfunclabel(script_getdata(st,2)) ) {
		ShowError("buildin_callsub: Argument is not a label\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	ref = (struct reg_db *)aCalloc(sizeof(struct reg_db), 1);
	ref[0].vars = st->stack->scope.vars;
	if (!st->stack->scope.arrays)
		st->stack->scope.arrays = idb_alloc(DB_OPT_BASE); // TODO: Can this happen? when?
	ref[0].arrays = st->stack->scope.arrays;

	for(i = st->start+3, j = 0; i < st->end; i++, j++) {
		struct script_data* data = push_copy(st->stack,i);

		if (data_isreference(data) && !data->ref) {
			const char* name = reference_getname(data);

			if (name[0] == '.' && name[1] == '@')
				data->ref = &ref[0];
		}
	}

	CREATE(ri, struct script_retinfo, 1);
	ri->script       = st->script;              // script code
	ri->scope.vars   = st->stack->scope.vars;   // scope variables
	ri->scope.arrays = st->stack->scope.arrays; // scope arrays
	ri->pos          = st->pos;                 // script location
	ri->nargs        = j;                       // argument count
	ri->defsp        = st->stack->defsp;        // default stack pointer
	push_retinfo(st->stack, ri, ref);

	st->pos = pos;
	st->stack->defsp = st->stack->sp;
	st->state = GOTO;
	st->stack->scope.vars = i64db_alloc(DB_OPT_RELEASE_DATA);
	st->stack->scope.arrays = idb_alloc(DB_OPT_BASE);

	return SCRIPT_CMD_SUCCESS;
}

/// Retrieves an argument provided to callfunc/callsub.
/// If the argument doesn't exist
///
/// getarg(<index>{,<default_value>}) -> <value>
BUILDIN_FUNC(getarg)
{
	struct script_retinfo* ri;
	int idx;

	if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp - 1].type != C_RETINFO )
	{
		ShowError("buildin_getarg: No callfunc or callsub!\n");
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}
	ri = st->stack->stack_data[st->stack->defsp - 1].u.ri;

	idx = script_getnum(st,2);

	if( idx >= 0 && idx < ri->nargs )
		push_copy(st->stack, st->stack->defsp - 1 - ri->nargs + idx);
	else if( script_hasdata(st,3) )
		script_pushcopy(st, 3);
	else
	{
		ShowError("buildin_getarg: Index (idx=%d) out of range (nargs=%d) and no default value found\n", idx, ri->nargs);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Returns from the current function, optionaly returning a value from the functions.
/// Don't use outside script functions.
///
/// return;
/// return <value>;
BUILDIN_FUNC(return)
{
	if( script_hasdata(st,2) )
	{// return value
		struct script_data* data;
		script_pushcopy(st, 2);
		data = script_getdatatop(st, -1);
		if( data_isreference(data) ) {
			const char* name = reference_getname(data);
			if( name[0] == '.' && name[1] == '@' ) { // scope variable
				if( !data->ref || data->ref->vars == st->stack->scope.vars )
					get_val(st, data); // current scope, convert to value
				if( data->ref && data->ref->vars == st->stack->stack_data[st->stack->defsp-1].u.ri->scope.vars )
					data->ref = NULL; // Reference to the parent scope, remove reference pointer
			}
		}
	}
	else
	{// no return value
		script_pushnil(st);
	}
	st->state = RETFUNC;
	return SCRIPT_CMD_SUCCESS;
}

/// Returns a random number from 0 to <range>-1.
/// Or returns a random number from <min> to <max>.
/// If <min> is greater than <max>, their numbers are switched.
/// rand(<range>) -> <int>
/// rand(<min>,<max>) -> <int>
BUILDIN_FUNC(rand)
{
	int range;
	int min;

	if( script_hasdata(st,3) )
	{// min,max
		int max = script_getnum(st,3);
		min = script_getnum(st,2);
		if( max < min )
			SWAP(min, max);
		range = max - min + 1;
	}
	else
	{// range
		min = 0;
		range = script_getnum(st,2);
	}
	if( range <= 1 )
		script_pushint(st, min);
	else
		script_pushint(st, rnd()%range + min);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Warp sd to str,x,y or Random or SavePoint/Save
 *------------------------------------------*/
BUILDIN_FUNC(warp)
{
	int ret;
	int x,y;
	const char* str;
	struct map_session_data* sd;

	if(!script_charid2sd(5, sd))
		return SCRIPT_CMD_SUCCESS;

	str = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);

	if(strcmp(str,"Random")==0)
		ret = pc_randomwarp(sd,CLR_TELEPORT);
	else if(strcmp(str,"SavePoint")==0 || strcmp(str,"Save")==0)
		ret = pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
	else
		ret = pc_setpos(sd,mapindex_name2id(str),x,y,CLR_OUTSIGHT);

	if( ret ) {
		ShowError("buildin_warp: moving player '%s' to \"%s\",%d,%d failed.\n", sd->status.name, str, x, y);
		return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Warp a specified area
 *------------------------------------------*/
static int buildin_areawarp_sub(struct block_list *bl,va_list ap)
{
	int x2,y2,x3,y3;
	unsigned int index;

	index = va_arg(ap,unsigned int);
	x2 = va_arg(ap,int);
	y2 = va_arg(ap,int);
	x3 = va_arg(ap,int);
	y3 = va_arg(ap,int);

	if(index == 0)
		pc_randomwarp((TBL_PC *)bl,CLR_TELEPORT);
	else if(x3 && y3) {
		int max, tx, ty, j = 0;
		int16 m;

		m = map_mapindex2mapid(index);

		// choose a suitable max number of attempts
		if( (max = (y3-y2+1)*(x3-x2+1)*3) > 1000 )
			max = 1000;

		// find a suitable map cell
		do {
			tx = rnd()%(x3-x2+1)+x2;
			ty = rnd()%(y3-y2+1)+y2;
			j++;
		} while( map_getcell(m,tx,ty,CELL_CHKNOPASS) && j < max );

		pc_setpos((TBL_PC *)bl,index,tx,ty,CLR_OUTSIGHT);
	}
	else
		pc_setpos((TBL_PC *)bl,index,x2,y2,CLR_OUTSIGHT);
	return 0;
}

BUILDIN_FUNC(areawarp)
{
	int16 m, x0,y0,x1,y1, x2,y2,x3=0,y3=0;
	unsigned int index;
	const char *str;
	const char *mapname;

	mapname = script_getstr(st,2);
	x0  = script_getnum(st,3);
	y0  = script_getnum(st,4);
	x1  = script_getnum(st,5);
	y1  = script_getnum(st,6);
	str = script_getstr(st,7);
	x2  = script_getnum(st,8);
	y2  = script_getnum(st,9);

	if( script_hasdata(st,10) && script_hasdata(st,11) ) { // Warp area to area
		if( (x3 = script_getnum(st,10)) < 0 || (y3 = script_getnum(st,11)) < 0 ){
			x3 = 0;
			y3 = 0;
		} else if( x3 && y3 ) {
			// normalize x3/y3 coordinates
			if( x3 < x2 ) SWAP(x3,x2);
			if( y3 < y2 ) SWAP(y3,y2);
		}
	}

	if( (m = map_mapname2mapid(mapname)) < 0 )
		return SCRIPT_CMD_FAILURE;

	if( strcmp(str,"Random") == 0 )
		index = 0;
	else if( !(index=mapindex_name2id(str)) )
		return SCRIPT_CMD_FAILURE;

	map_foreachinallarea(buildin_areawarp_sub, m,x0,y0,x1,y1, BL_PC, index,x2,y2,x3,y3);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * areapercentheal <map>,<x1>,<y1>,<x2>,<y2>,<hp>,<sp>
 *------------------------------------------*/
static int buildin_areapercentheal_sub(struct block_list *bl,va_list ap)
{
	int hp, sp;
	hp = va_arg(ap, int);
	sp = va_arg(ap, int);
	pc_percentheal((TBL_PC *)bl,hp,sp);
	return 0;
}

BUILDIN_FUNC(areapercentheal)
{
	int hp,sp,m;
	const char *mapname;
	int x0,y0,x1,y1;

	mapname=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	hp=script_getnum(st,7);
	sp=script_getnum(st,8);

	if( (m=map_mapname2mapid(mapname))< 0)
		return SCRIPT_CMD_FAILURE;

	map_foreachinallarea(buildin_areapercentheal_sub,m,x0,y0,x1,y1,BL_PC,hp,sp);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Warpparty - [Fredzilla] [Paradox924X]
 * Syntax: warpparty "to_mapname",x,y,Party_ID,{<"from_mapname">,<range x>,<range y>};
 * If 'from_mapname' is specified, only the party members on that map will be warped
 *------------------------------------------*/
BUILDIN_FUNC(warpparty)
{
	TBL_PC *sd = NULL;
	TBL_PC *pl_sd;
	struct party_data* p;
	int type, mapindex = 0, m = -1, i, rx = 0, ry = 0;

	const char* str = script_getstr(st,2);
	int x = script_getnum(st,3);
	int y = script_getnum(st,4);
	int p_id = script_getnum(st,5);
	const char* str2 = NULL;
	if ( script_hasdata(st,6) )
		str2 = script_getstr(st,6);
	if (script_hasdata(st, 7))
		rx = script_getnum(st, 7);
	if (script_hasdata(st, 8))
		ry = script_getnum(st, 8);

	p = party_search(p_id);
	if(!p)
		return SCRIPT_CMD_SUCCESS;

	type = ( strcmp(str,"Random")==0 ) ? 0
	     : ( strcmp(str,"SavePointAll")==0 ) ? 1
		 : ( strcmp(str,"SavePoint")==0 ) ? 2
		 : ( strcmp(str,"Leader")==0 ) ? 3
		 : 4;

	switch (type)
	{
		case 3:
			for(i = 0; i < MAX_PARTY && !p->party.member[i].leader; i++);
			if (i == MAX_PARTY || !p->data[i].sd) //Leader not found / not online
				return SCRIPT_CMD_FAILURE;
			pl_sd = p->data[i].sd;
			mapindex = pl_sd->mapindex;
			m = map_mapindex2mapid(mapindex);
			x = pl_sd->bl.x;
			y = pl_sd->bl.y;
			break;
		case 4:
			mapindex = mapindex_name2id(str);
			if (!mapindex) {// Invalid map
				return SCRIPT_CMD_FAILURE;
			}
			m = map_mapindex2mapid(mapindex);
			break;
		case 2:
			//"SavePoint" uses save point of the currently attached player
			if ( !script_rid2sd(sd) )
				return SCRIPT_CMD_SUCCESS;
			break;
	}

	for (i = 0; i < MAX_PARTY; i++)
	{
		if( !(pl_sd = p->data[i].sd) || pl_sd->status.party_id != p_id )
			continue;

		if( str2 && strcmp(str2, map_getmapdata(pl_sd->bl.m)->name) != 0 )
			continue;

		if( pc_isdead(pl_sd) )
			continue;

		switch( type )
		{
		case 0: // Random
			if(!map_getmapflag(pl_sd->bl.m, MF_NOWARP))
				pc_randomwarp(pl_sd,CLR_TELEPORT);
		break;
		case 1: // SavePointAll
			if(!map_getmapflag(pl_sd->bl.m, MF_NORETURN))
				pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);
		break;
		case 2: // SavePoint
			if(!map_getmapflag(pl_sd->bl.m, MF_NORETURN))
				pc_setpos(pl_sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
		break;
		case 3: // Leader
			if (p->party.member[i].leader)
				continue;
		case 4: // m,x,y
			if (rx || ry) {
				int x1 = x + rx, y1 = y + ry,
					x0 = x - rx, y0 = y - ry;
				uint8 attempts = 10;

				do {
					x = x0 + rnd()%(x1 - x0 + 1);
					y = y0 + rnd()%(y1 - y0 + 1);
				} while ((--attempts) > 0 && !map_getcell(m, x, y, CELL_CHKPASS));
			}

			if(!map_getmapflag(pl_sd->bl.m, MF_NORETURN) && !map_getmapflag(pl_sd->bl.m, MF_NOWARP) && pc_job_can_entermap((enum e_job)pl_sd->status.class_, m, pl_sd->group_level))
				pc_setpos(pl_sd,mapindex,x,y,CLR_TELEPORT);
		break;
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Warpguild - [Fredzilla]
 * Syntax: warpguild "mapname",x,y,Guild_ID;
 *------------------------------------------*/
BUILDIN_FUNC(warpguild)
{
	TBL_PC *sd = NULL;
	TBL_PC *pl_sd;
	struct guild* g;
	struct s_mapiterator* iter;
	int type, mapindex = 0, m = -1;

	const char* str = script_getstr(st,2);
	int x           = script_getnum(st,3);
	int y           = script_getnum(st,4);
	int gid         = script_getnum(st,5);

	g = guild_search(gid);
	if( g == NULL )
		return SCRIPT_CMD_SUCCESS;

	type = ( strcmp(str,"Random")==0 ) ? 0
	     : ( strcmp(str,"SavePointAll")==0 ) ? 1
		 : ( strcmp(str,"SavePoint")==0 ) ? 2
		 : 3;

	if( type == 2 && !script_rid2sd(sd) )
	{// "SavePoint" uses save point of the currently attached player
		return SCRIPT_CMD_SUCCESS;
	}

	switch (type) {
		case 3:
			mapindex = mapindex_name2id(str);
			if (!mapindex)
				return SCRIPT_CMD_FAILURE;
			m = map_mapindex2mapid(mapindex);
			break;
	}

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if( pl_sd->status.guild_id != gid )
			continue;

		switch( type )
		{
		case 0: // Random
			if(!map_getmapflag(pl_sd->bl.m, MF_NOWARP))
				pc_randomwarp(pl_sd,CLR_TELEPORT);
		break;
		case 1: // SavePointAll
			if(!map_getmapflag(pl_sd->bl.m, MF_NORETURN))
				pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);
		break;
		case 2: // SavePoint
			if(!map_getmapflag(pl_sd->bl.m, MF_NORETURN))
				pc_setpos(pl_sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
		break;
		case 3: // m,x,y
			if(!map_getmapflag(pl_sd->bl.m, MF_NORETURN) && !map_getmapflag(pl_sd->bl.m, MF_NOWARP) && pc_job_can_entermap((enum e_job)pl_sd->status.class_, m, pl_sd->group_level))
				pc_setpos(pl_sd,mapindex,x,y,CLR_TELEPORT);
		break;
		}
	}
	mapit_free(iter);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Force Heal a player (hp and sp)
 *------------------------------------------*/
BUILDIN_FUNC(heal)
{
	TBL_PC *sd;
	int hp,sp;

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_SUCCESS;

	hp=script_getnum(st,2);
	sp=script_getnum(st,3);
	status_heal(&sd->bl, hp, sp, 1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Heal a player by item (get vit bonus etc)
 *------------------------------------------*/
BUILDIN_FUNC(itemheal)
{
	TBL_PC *sd;
	int hp,sp;

	hp=script_getnum(st,2);
	sp=script_getnum(st,3);

	if(potion_flag==1) {
		potion_hp = hp;
		potion_sp = sp;
		return SCRIPT_CMD_SUCCESS;
	}

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_SUCCESS;

	pc_itemheal(sd,sd->itemid,hp,sp);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(percentheal)
{
	int hp,sp;
	TBL_PC* sd;

	hp=script_getnum(st,2);
	sp=script_getnum(st,3);

	if(potion_flag==1) {
		potion_per_hp = hp;
		potion_per_sp = sp;
		return SCRIPT_CMD_SUCCESS;
	}

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_SUCCESS;

#ifdef RENEWAL
	if( sd->sc.data[SC_EXTREMITYFIST2] )
		sp = 0;
#endif

	if (sd->sc.data[SC_NORECOVER_STATE]) {
		hp = 0;
		sp = 0;
	}

	if (sd->sc.data[SC_BITESCAR])
		hp = 0;

	pc_percentheal(sd,hp,sp);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(jobchange)
{
	int job, upper=-1;

	job=script_getnum(st,2);
	if( script_hasdata(st,3) )
		upper=script_getnum(st,3);

	if (pcdb_checkid(job))
	{
		TBL_PC* sd;

		if (!script_charid2sd(4,sd))
			return SCRIPT_CMD_SUCCESS;

		pc_jobchange(sd, job, upper);
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(jobname)
{
	int class_=script_getnum(st,2);
	script_pushconststr(st, (char*)job_name(class_));
	return SCRIPT_CMD_SUCCESS;
}

/// Get input from the player.
/// For numeric inputs the value is capped to the range [min,max]. Returns 1 if
/// the value was higher than 'max', -1 if lower than 'min' and 0 otherwise.
/// For string inputs it returns 1 if the string was longer than 'max', -1 is
/// shorter than 'min' and 0 otherwise.
///
/// input(<var>{,<min>{,<max>}}) -> <int>
BUILDIN_FUNC(input)
{
	TBL_PC* sd;
	struct script_data* data;
	int64 uid;
	const char* name;
	int min;
	int max;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	data = script_getdata(st,2);
	if( !data_isreference(data) ){
		ShowError("script:input: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}
	uid = reference_getuid(data);
	name = reference_getname(data);
	min = (script_hasdata(st,3) ? script_getnum(st,3) : script_config.input_min_value);
	max = (script_hasdata(st,4) ? script_getnum(st,4) : script_config.input_max_value);

#ifdef SECURE_NPCTIMEOUT
	sd->npc_idle_type = NPCT_WAIT;
#endif

	if( !sd->state.menu_or_input )
	{	// first invocation, display npc input box
		sd->state.menu_or_input = 1;
		st->state = RERUNLINE;
		if( is_string_variable(name) )
			clif_scriptinputstr(sd,st->oid);
		else
			clif_scriptinput(sd,st->oid);
	}
	else
	{	// take received text/value and store it in the designated variable
		sd->state.menu_or_input = 0;
		if( is_string_variable(name) )
		{
			int len = (int)strlen(sd->npc_str);
			set_reg(st, sd, uid, name, (void*)sd->npc_str, script_getref(st,2));
			script_pushint(st, (len > max ? 1 : len < min ? -1 : 0));
		}
		else
		{
			int amount = sd->npc_amount;
			set_reg(st, sd, uid, name, (void*)__64BPRTSIZE(cap_value(amount,min,max)), script_getref(st,2));
			script_pushint(st, (amount > max ? 1 : amount < min ? -1 : 0));
		}
		st->state = RUN;
	}
	return SCRIPT_CMD_SUCCESS;
}

// declare the copyarray method here for future reference
BUILDIN_FUNC(copyarray);

/// Sets the value of a variable.
/// The value is converted to the type of the variable.
///
/// set(<variable>,<value>{,<charid>})
BUILDIN_FUNC(setr)
{
	TBL_PC* sd = NULL;
	struct script_data* data;
	//struct script_data* datavalue;
	int64 num;
	uint8 pos = 4;
	const char* name, *command = script_getfuncname(st);
	char prefix;

	data = script_getdata(st,2);
	//datavalue = script_getdata(st,3);
	if( !data_isreference(data) )
	{
		ShowError("script:set: not a variable\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	num = reference_getuid(data);
	name = reference_getname(data);
	prefix = *name;

	if (!strcmp(command, "setr"))
		pos = 5;

	if (not_server_variable(prefix) && !script_charid2sd(pos,sd)) {
		ShowError("buildin_set: No player attached for player variable '%s'\n", name);
		return SCRIPT_CMD_FAILURE;
	}

#if 0
	if( data_isreference(datavalue) )
	{// the value being referenced is a variable
		const char* namevalue = reference_getname(datavalue);

		if( !not_array_variable(*namevalue) )
		{// array variable being copied into another array variable
			if( sd == NULL && not_server_variable(*namevalue) && !(sd = script_rid2sd(st)) )
			{// player must be attached in order to copy a player variable
				ShowError("script:set: no player attached for player variable '%s'\n", namevalue);
				return SCRIPT_CMD_SUCCESS;
			}

			if( is_string_variable(namevalue) != is_string_variable(name) )
			{// non-matching array value types
				ShowWarning("script:set: two array variables do not match in type.\n");
				return SCRIPT_CMD_SUCCESS;
			}

			// push the maximum number of array values to the stack
			push_val(st->stack, C_INT, SCRIPT_MAX_ARRAYSIZE);

			// call the copy array method directly
			return buildin_copyarray(st);
		}
	}
#endif

	if( !strcmp(command, "setr") && script_hasdata(st, 4) ) { // Optional argument used by post-increment/post-decrement constructs to return the previous value
		if( is_string_variable(name) )
			script_pushstrcopy(st,script_getstr(st, 4));
		else
			script_pushint(st,script_getnum(st, 4));
	} else // Return a copy of the variable reference
		script_pushcopy(st, 2);

	if( is_string_variable(name) )
		set_reg(st,sd,num,name,(void*)script_getstr(st,3),script_getref(st,2));
	else
		set_reg(st,sd,num,name,(void*)__64BPRTSIZE(script_getnum(st,3)),script_getref(st,2));

	return SCRIPT_CMD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
/// Array variables
///

/// Sets values of an array, from the starting index.
/// ex: setarray arr[1],1,2,3;
///
/// setarray <array variable>,<value1>{,<value2>...};
BUILDIN_FUNC(setarray)
{
	struct script_data* data;
	const char* name;
	uint32 start;
	uint32 end;
	int32 id;
	int32 i;
	TBL_PC* sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:setarray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) )
	{
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;// no player attached
	}

	end = start + script_lastdata(st) - 2;
	if( end > SCRIPT_MAX_ARRAYSIZE )
		end = SCRIPT_MAX_ARRAYSIZE;

	if( is_string_variable(name) )
	{// string array
		for( i = 3; start < end; ++start, ++i )
			set_reg(st, sd, reference_uid(id, start), name, (void*)script_getstr(st,i), reference_getref(data));
	}
	else
	{// int array
		for( i = 3; start < end; ++start, ++i )
			set_reg(st, sd, reference_uid(id, start), name, (void*)__64BPRTSIZE(script_getnum(st,i)), reference_getref(data));
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Sets count values of an array, from the starting index.
/// ex: cleararray arr[0],0,1;
///
/// cleararray <array variable>,<value>,<count>;
BUILDIN_FUNC(cleararray)
{
	struct script_data* data;
	const char* name;
	uint32 start;
	uint32 end;
	int32 id;
	void* v;
	TBL_PC* sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:cleararray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) )
	{
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;// no player attached
	}

	if( is_string_variable(name) )
		v = (void*)script_getstr(st, 3);
	else
		v = (void*)__64BPRTSIZE(script_getnum(st, 3));

	end = start + script_getnum(st, 4);
	if( end > SCRIPT_MAX_ARRAYSIZE )
		end = SCRIPT_MAX_ARRAYSIZE;

	for( ; start < end; ++start )
		set_reg(st, sd, reference_uid(id, start), name, v, script_getref(st,2));
	return SCRIPT_CMD_SUCCESS;
}

/// Copies data from one array to another.
/// ex: copyarray arr[0],arr[2],2;
///
/// copyarray <destination array variable>,<source array variable>,<count>;
BUILDIN_FUNC(copyarray)
{
	struct script_data* data1;
	struct script_data* data2;
	const char* name1;
	const char* name2;
	int32 idx1;
	int32 idx2;
	int32 id1;
	int32 id2;
	void* v;
	int32 i;
	uint32 count;
	TBL_PC* sd = NULL;

	data1 = script_getdata(st, 2);
	data2 = script_getdata(st, 3);
	if( !data_isreference(data1) || !data_isreference(data2) )
	{
		ShowError("script:copyarray: not a variable\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id1 = reference_getid(data1);
	id2 = reference_getid(data2);
	idx1 = reference_getindex(data1);
	idx2 = reference_getindex(data2);
	name1 = reference_getname(data1);
	name2 = reference_getname(data2);

	if( is_string_variable(name1) != is_string_variable(name2) )
	{
		ShowError("script:copyarray: type mismatch\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// data type mismatch
	}

	if( not_server_variable(*name1) || not_server_variable(*name2) )
	{
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;// no player attached
	}

	count = script_getnum(st, 4);
	if( count > SCRIPT_MAX_ARRAYSIZE - idx1 )
		count = SCRIPT_MAX_ARRAYSIZE - idx1;
	if( count <= 0 || (id1 == id2 && idx1 == idx2) )
		return SCRIPT_CMD_SUCCESS;// nothing to copy

	if( id1 == id2 && idx1 > idx2 )
	{// destination might be overlapping the source - copy in reverse order
		for( i = count - 1; i >= 0; --i )
		{
			v = get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
			set_reg(st, sd, reference_uid(id1, idx1 + i), name1, v, reference_getref(data1));
			script_removetop(st, -1, 0);
		}
	}
	else
	{// normal copy
		for( i = 0; i < count; ++i )
		{
			if( idx2 + i < SCRIPT_MAX_ARRAYSIZE )
			{
				v = get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
				set_reg(st, sd, reference_uid(id1, idx1 + i), name1, v, reference_getref(data1));
				script_removetop(st, -1, 0);
			}
			else// out of range - assume ""/0
				set_reg(st, sd, reference_uid(id1, idx1 + i), name1, (is_string_variable(name1)?(void*)"":(void*)0), reference_getref(data1));
		}
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Returns the size of the array.
/// Assumes that everything before the starting index exists.
/// ex: getarraysize(arr[3])
///
/// getarraysize(<array variable>) -> <int>
BUILDIN_FUNC(getarraysize)
{
	struct script_data* data;
	const char* name;
	struct map_session_data* sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getarraysize: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	name = reference_getname(data);

	if( not_server_variable(*name) ){
		if (!script_rid2sd(sd))
			return SCRIPT_CMD_SUCCESS;// no player attached
	}

	script_pushint(st, script_array_highest_key(st, sd, reference_getname(data), reference_getref(data)));
	return SCRIPT_CMD_SUCCESS;
}

int script_array_index_cmp(const void *a, const void *b) {
	return ( *(unsigned int*)a - *(unsigned int*)b );
}

/// Deletes count or all the elements in an array, from the starting index.
/// ex: deletearray arr[4],2;
///
/// deletearray <array variable>;
/// deletearray <array variable>,<count>;
BUILDIN_FUNC(deletearray)
{
	struct script_data* data;
	const char* name;
	unsigned int start, end, i;
	int id;
	TBL_PC *sd = NULL;
	struct script_array *sa = NULL;
	struct reg_db *src = NULL;
	void *value;

	data = script_getdata(st, 2);
	if( !data_isreference(data) ) {
		ShowError("script:deletearray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) ) {
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;// no player attached
	}

	if (!(src = script_array_src(st, sd, name, reference_getref(data)))) {
		ShowError("script:deletearray: not a array\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	script_array_ensure_zero(st, NULL, data->u.num, reference_getref(data));

	if ( !(sa = static_cast<script_array *>(idb_get(src->arrays, id))) ) { // non-existent array, nothing to empty
		return SCRIPT_CMD_SUCCESS;// not a variable
	}

	end = script_array_highest_key(st, sd, name, reference_getref(data));

	if( start >= end )
		return SCRIPT_CMD_SUCCESS;// nothing to free

	if( is_string_variable(name) )
		value = (void *)"";
	else
		value = (void *)0;

	if( script_hasdata(st,3) ) {
		unsigned int count = script_getnum(st, 3);
		if( count > end - start )
			count = end - start;
		if( count <= 0 )
			return SCRIPT_CMD_SUCCESS;// nothing to free

		if( end - start < sa->size ) {
			// Better to iterate directly on the array, no speed-up from using sa
			for( ; start + count < end; ++start ) {
				// Compact and overwrite
				void* v = get_val2(st, reference_uid(id, start + count), reference_getref(data));
				set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
				script_removetop(st, -1, 0);
			}
			for( ; start < end; start++ ) {
				// Clean up any leftovers that weren't overwritten
				set_reg(st, sd, reference_uid(id, start), name, value, reference_getref(data));
			}
		} else {
			// using sa to speed up
			unsigned int *list = NULL, size = 0;
			list = script_array_cpy_list(sa);
			size = sa->size;
			qsort(list, size, sizeof(unsigned int), script_array_index_cmp);
			
			ARR_FIND(0, size, i, list[i] >= start);
			
			for( ; i < size && list[i] < start + count; i++ ) {
				// Clear any entries between start and start+count, if they exist
				set_reg(st, sd, reference_uid(id, list[i]), name, value, reference_getref(data));
			}
			
			for( ; i < size && list[i] < end; i++ ) {
				// Move back count positions any entries between start+count to fill the gaps
				void* v = get_val2(st, reference_uid(id, list[i]), reference_getref(data));
				set_reg(st, sd, reference_uid(id, list[i]-count), name, v, reference_getref(data));
				script_removetop(st, -1, 0);
				// Clear their originals
				set_reg(st, sd, reference_uid(id, list[i]), name, value, reference_getref(data));
			}
		}
	} else {
		unsigned int *list = NULL, size = 0;
		list = script_array_cpy_list(sa);
		size = sa->size;
		
		for(i = 0; i < size; i++) {
			if( list[i] >= start ) // Less expensive than sorting it, most likely
				set_reg(st, sd, reference_uid(id, list[i]), name, value, reference_getref(data));
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Returns a reference to the target index of the array variable.
/// Equivalent to var[index].
///
/// getelementofarray(<array variable>,<index>) -> <variable reference>
BUILDIN_FUNC(getelementofarray)
{
	struct script_data* data;
	int32 id;
	int64 i;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getelementofarray: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_SUCCESS;// not a variable
	}

	id = reference_getid(data);

	i = script_getnum(st, 3);
	if (i < 0 || i >= SCRIPT_MAX_ARRAYSIZE) {
		ShowWarning("script:getelementofarray: index out of range (%" PRId64 ")\n", i);
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// out of range
	}

	push_val2(st->stack, C_NAME, reference_uid(id, i), reference_getref(data));
	return SCRIPT_CMD_SUCCESS;
}

/// Return the index number of the first matching value in an array.
/// ex: inarray arr,1;
///
/// inarray <array variable>,<value>;
BUILDIN_FUNC(inarray)
{
	struct script_data *data;
	const char* name;
	int id, i, array_size;
	struct map_session_data* sd = NULL;
	struct reg_db *ref = NULL;
	data = script_getdata(st, 2);

	if (!data_isreference(data))
	{
		ShowError("buildin_inarray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	name = reference_getname(data);
	ref = reference_getref(data);

	if (not_server_variable(*name) && !script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	array_size = script_array_highest_key(st, sd, name, ref) - 1;

	if (array_size < 0)
	{
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	if (array_size > SCRIPT_MAX_ARRAYSIZE)
	{
		ShowError("buildin_inarray: The array is too large.\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	id = reference_getid(data);
	if (is_string_variable(name))
	{
		const char* temp;
		const char* value;
		value = script_getstr(st, 3);
		for (i = 0; i <= array_size; ++i)
		{
			temp = (char*)get_val2(st, reference_uid(id, i), ref);
			if (!strcmp(temp, value))
			{
				script_removetop(st, -1, 0);
				script_pushint(st, i);
				return SCRIPT_CMD_SUCCESS;
			}
			script_removetop(st, -1, 0);
		}
	}
	else
	{
		int temp, value;
		value = script_getnum(st, 3);
		for (i = 0; i <= array_size; ++i)
		{
			temp = (int32)__64BPRTSIZE(get_val2(st, reference_uid(id, i), ref));
			script_removetop(st, -1, 0);
			if (temp == value)
			{
				script_pushint(st, i);
				return SCRIPT_CMD_SUCCESS;
			}
			
		}
	}

	script_pushint(st, -1);
	return SCRIPT_CMD_SUCCESS;
}

/// Return the number of matches in two arrays.
/// ex: countinarray arr[0],arr1[0];
///
/// countinarray <array variable>,<array variable>;
BUILDIN_FUNC(countinarray)
{
	struct script_data *data1 , *data2;
	const char* name1;
	const char* name2;
	int id1, id2, i, j, array_size1, array_size2, case_count = 0;
	struct map_session_data* sd = NULL;
	struct reg_db *ref1 = NULL, *ref2 = NULL;
	data1 = script_getdata(st, 2);
	data2 = script_getdata(st, 3);

	if (!data_isreference(data1) || !data_isreference(data2))
	{
		ShowError("buildin_countinarray: not a variable\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	name1 = reference_getname(data1);
	name2 = reference_getname(data2);
	ref1 = reference_getref(data1);
	ref2 = reference_getref(data2);

	if (not_server_variable(*name1) && not_server_variable(*name2) && !script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	array_size1 = script_array_highest_key(st, sd, name1, ref1) - 1;
	array_size2 = script_array_highest_key(st, sd, name2, ref2) - 1;

	if (array_size1 < 0 || array_size2 < 0)
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (array_size1 > SCRIPT_MAX_ARRAYSIZE || array_size2 > SCRIPT_MAX_ARRAYSIZE)
	{
		ShowError("buildin_countinarray: The array is too large.\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	i = reference_getindex(data1);
	j = reference_getindex(data2);
	if (array_size1 < i || array_size2 < j)
	{	//To prevent unintended behavior
		ShowError("buildin_countinarray: The given index of the array is higher than the array size.\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	id1 = reference_getid(data1);
	id2 = reference_getid(data2);
	if (is_string_variable(name1) && is_string_variable(name2))
	{
		const char* temp1;
		const char* temp2;
		for (; i <= array_size1; ++i)
		{
			temp1 = (char*)get_val2(st, reference_uid(id1, i), ref1);
			for (j = reference_getindex(data2); j <= array_size2; j++)
			{
				temp2 = (char*)get_val2(st, reference_uid(id2, j), ref2);
				if (!strcmp(temp1, temp2))
				{
					case_count++;
				}
				script_removetop(st, -1, 0);
			}
			script_removetop(st, -1, 0);
		}
	}
	else if (!is_string_variable(name1) && !is_string_variable(name2))
	{
		int temp1, temp2;
		for (; i <= array_size1; ++i)
		{
			temp1 = (int32)__64BPRTSIZE(get_val2(st, reference_uid(id1, i), ref1));
			for (j = reference_getindex(data2); j <= array_size2; j++)
			{
				temp2 = (int32)__64BPRTSIZE(get_val2(st, reference_uid(id2, j), ref2));
				if (temp1 == temp2)
				{
					case_count++;
				}
				script_removetop(st, -1, 0);
			}
			script_removetop(st, -1, 0);
		}
	}
	else
	{
		ShowError("buildin_countinarray: Arrays does not match , You can't compare int array to string array.\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, case_count);
	return SCRIPT_CMD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
/// ...
///

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(setlook)
{
	int type,val;
	TBL_PC* sd;

	type = script_getnum(st,2);
	val = script_getnum(st,3);

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_SUCCESS;

	pc_changelook(sd,type,val);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(changelook)
{ // As setlook but only client side
	int type,val;
	TBL_PC* sd;

	type = script_getnum(st,2);
	val = script_getnum(st,3);

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_SUCCESS;

	clif_changelook(&sd->bl,type,val);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(cutin)
{
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	clif_cutin(sd,script_getstr(st,2),script_getnum(st,3));
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(viewpoint)
{
	int type,x,y,id,color;
	TBL_PC* sd;

	type=script_getnum(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	id=script_getnum(st,5);
	color=script_getnum(st,6);

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	clif_viewpoint(sd,st->oid,type,x,y,id,color);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set random options for new item
 * @param st Script state
 * @param it Temporary item data
 * @param funcname Function name
 * @param x First position of random option id array from the script
 **/
static int script_getitem_randomoption(struct script_state *st, struct map_session_data* sd, struct item *it, const char *funcname, int x) {
	int i, opt_id_n;
	struct script_data *opt_id = script_getdata(st,x);
	struct script_data *opt_val = script_getdata(st,x+1);
	struct script_data *opt_param = script_getdata(st,x+2);
	const char *opt_id_var = reference_getname(opt_id);
	const char *opt_val_var = reference_getname(opt_val);
	const char *opt_param_var = reference_getname(opt_param);
	int32 opt_id_id, opt_id_idx;
	int32 opt_val_id, opt_val_idx;
	int32 opt_param_id, opt_param_idx;
	struct reg_db *opt_id_ref = NULL, *opt_val_ref = NULL, *opt_param_ref = NULL;

	// Check if the variable requires a player
	if( not_server_variable(opt_id_var[0]) && sd == nullptr ){
		// If no player is attached
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_%s: variable \"%s\" was not a server variable, but no player was attached.\n", funcname, opt_id_var );
			return false;
		}
	}

	if( !data_isreference(opt_id) || !script_array_src( st, sd, opt_id_var, reference_getref(opt_id) ) ){
		ShowError( "buildin_%s: The option id parameter is not an array.\n", funcname );
		return SCRIPT_CMD_FAILURE;
	}

	if (is_string_variable(opt_id_var)) {
		ShowError("buildin_%s: The array %s is not numeric type.\n", funcname, opt_id_var);
		return SCRIPT_CMD_FAILURE;
	}

	// Check if the variable requires a player
	if( not_server_variable(opt_val_var[0]) && sd == nullptr ){
		// If no player is attached
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_%s: variable \"%s\" was not a server variable, but no player was attached.\n", funcname, opt_val_var );
			return false;
		}
	}

	if( !data_isreference(opt_val) || !script_array_src( st, sd, opt_val_var, reference_getref(opt_val) ) ){
		ShowError( "buildin_%s: The option value parameter is not an array.\n", funcname );
		return SCRIPT_CMD_FAILURE;
	}

	if (is_string_variable(opt_val_var)) {
		ShowError("buildin_%s: The array %s is not numeric type.\n", funcname, opt_val_var);
		return SCRIPT_CMD_FAILURE;
	}

	// Check if the variable requires a player
	if( not_server_variable(opt_param_var[0]) && sd == nullptr ){
		// If no player is attached
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_%s: variable \"%s\" was not a server variable, but no player was attached.\n", funcname, opt_param_var );
			return false;
		}
	}

	if( !data_isreference(opt_param) || !script_array_src( st, sd, opt_param_var, reference_getref(opt_param) ) ){
		ShowError( "buildin_%s: The option param parameter is not an array.\n", funcname );
		return SCRIPT_CMD_FAILURE;
	}

	if (is_string_variable(opt_param_var)) {
		ShowError("buildin_%s: The array %s is not numeric type.\n", funcname, opt_param_var);
		return SCRIPT_CMD_FAILURE;
	}

	opt_id_ref = reference_getref(opt_id);
	opt_id_n = script_array_highest_key(st, sd, opt_id_var, opt_id_ref);

	if (opt_id_n < 1) {
		ShowError("buildin_%s: No option id listed.\n", funcname);
		return SCRIPT_CMD_FAILURE;
	}

	opt_val_ref = reference_getref(opt_val);
	opt_param_ref = reference_getref(opt_param);

	opt_id_id = reference_getid(opt_id);
	opt_val_id = reference_getid(opt_val);
	opt_param_id = reference_getid(opt_param);

	opt_id_idx = reference_getindex(opt_id);
	opt_val_idx = reference_getindex(opt_val);
	opt_param_idx = reference_getindex(opt_param);
	
	for (i = 0; i < opt_id_n && i < MAX_ITEM_RDM_OPT; i++) {
		it->option[i].id = static_cast<short>((int32)__64BPRTSIZE(get_val2(st,reference_uid(opt_id_id,opt_id_idx+i),opt_id_ref)));
		script_removetop(st, -1, 0);
		it->option[i].value = static_cast<short>((int32)__64BPRTSIZE(get_val2(st,reference_uid(opt_val_id,opt_val_idx+i),opt_val_ref)));
		script_removetop(st, -1, 0);
		it->option[i].param = static_cast<char>((int32)__64BPRTSIZE(get_val2(st,reference_uid(opt_param_id,opt_param_idx+i),opt_param_ref)));
		script_removetop(st, -1, 0);
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Returns number of items in inventory/cart/storage
/// countitem <nameID>{,<accountID>});
/// countitem2 <nameID>,<Identified>,<Refine>,<Attribute>,<Card0>,<Card1>,<Card2>,<Card3>{,<accountID>}) [Lupus]
/// cartcountitem <nameID>{,<accountID>});
/// cartcountitem2 <nameID>,<Identified>,<Refine>,<Attribute>,<Card0>,<Card1>,<Card2>,<Card3>{,<accountID>})
/// storagecountitem <nameID>{,<accountID>});
/// storagecountitem2 <nameID>,<Identified>,<Refine>,<Attribute>,<Card0>,<Card1>,<Card2>,<Card3>{,<accountID>})
/// guildstoragecountitem <nameID>{,<accountID>});
/// guildstoragecountitem2 <nameID>,<Identified>,<Refine>,<Attribute>,<Card0>,<Card1>,<Card2>,<Card3>{,<accountID>})
/// countitem3(<item id>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>)
/// countitem3("<item name>",<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>)
BUILDIN_FUNC(countitem)
{
	int i = 0, aid = 3;
	struct item_data* id = NULL;
	struct script_data* data;
	char *command = (char *)script_getfuncname(st);
	uint8 loc = 0;
	uint16 size, count = 0;
	struct item *items;
	TBL_PC *sd = NULL;
	struct s_storage *gstor = NULL;

	if( command[strlen(command)-1] == '2' ) {
		i = 1;
		aid = 10;
	}
	else if (command[strlen(command)-1] == '3') {
		i = 1;
		aid = 13;
	}

	if( script_hasdata(st,aid) ) {
		if( !(sd = map_id2sd( (aid = script_getnum(st, aid)) )) ) {
			ShowError("buildin_%s: player not found (AID=%d).\n", command, aid);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;
	}

	if( !strncmp(command, "cart", 4) ) {
		loc = TABLE_CART;
		size = MAX_CART;
		items = sd->cart.u.items_cart;
	}
	else if( !strncmp(command, "storage", 7) ) {
		loc = TABLE_STORAGE;
		size = MAX_STORAGE;
		items = sd->storage.u.items_storage;
	}
	else if( !strncmp(command, "guildstorage", 12) ) {
		gstor = guild2storage2(sd->status.guild_id);

		if (gstor && !sd->state.storage_flag) {
			loc = TABLE_GUILD_STORAGE;
			size = MAX_GUILD_STORAGE;
			items = gstor->u.items_guild;
		} else {
			script_pushint(st, -1);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	else {
		size = MAX_INVENTORY;
		items = sd->inventory.u.items_inventory;
	}

	if( loc == TABLE_CART && !pc_iscarton(sd) ) {
		ShowError("buildin_%s: Player doesn't have cart (CID:%d).\n", command, sd->status.char_id);
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	
	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable

	if( data_isstring(data) ) // item name
		id = itemdb_searchname(conv_str(st, data));
	else // item id
		id = itemdb_exists(conv_num(st, data));

	if( id == NULL ) {
		ShowError("buildin_%s: Invalid item '%s'.\n", command, script_getstr(st,2));  // returns string, regardless of what it was
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (loc == TABLE_GUILD_STORAGE)
		gstor->lock = true;

	if( !i ) { // For count/cart/storagecountitem function
		unsigned short nameid = id->nameid;
		for( i = 0; i < size; i++ )
			if( &items[i] && items[i].nameid == nameid )
				count += items[i].amount;
	}
	else { // For count/cart/storagecountitem2 function
		struct item it;
		bool check_randomopt = false;
		memset(&it, 0, sizeof(it));

		it.nameid = id->nameid;
		it.identify = script_getnum(st,3);
		it.refine  = script_getnum(st,4);
		it.attribute = script_getnum(st,5);
		it.card[0] = script_getnum(st,6);
		it.card[1] = script_getnum(st,7);
		it.card[2] = script_getnum(st,8);
		it.card[3] = script_getnum(st,9);

		if (command[strlen(command)-1] == '3') {
			int res = script_getitem_randomoption(st, sd, &it, command, 10);
			if (res != SCRIPT_CMD_SUCCESS)
				return SCRIPT_CMD_FAILURE;
			check_randomopt = true;
		}

		for( i = 0; i < size; i++ ) {
			struct item *itm = &items[i];
			if (!itm || !itm->nameid || itm->amount < 1)
				continue;
			if (itm->nameid != it.nameid || itm->identify != it.identify || itm->refine != it.refine || itm->attribute != it.attribute)
				continue;
			if (memcmp(it.card, itm->card, sizeof(it.card)))
				continue;
			if (check_randomopt) {
				uint8 j;
				for (j = 0; j < MAX_ITEM_RDM_OPT; j++) {
					if (itm->option[j].id != it.option[j].id || itm->option[j].value != it.option[j].value || itm->option[j].param != it.option[j].param)
						break;
				}
				if (j != MAX_ITEM_RDM_OPT)
					continue;
			}
			count += items[i].amount;
		}
	}

	if (loc == TABLE_GUILD_STORAGE) {
		storage_guild_storageclose(sd);
		gstor->lock = false;
	}

	script_pushint(st, count);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Check if item with this amount can fit in inventory
 * Checking : weight, stack amount >(MAX_AMOUNT), slots amount >(MAX_INVENTORY)
 * Return
 *	0 : fail
 *	1 : success (npc side only)
 *------------------------------------------*/
BUILDIN_FUNC(checkweight)
{
	int slots = 0;
	unsigned short amount2 = 0;
	unsigned int weight = 0, i, nbargs;
	struct item_data* id = NULL;
	struct map_session_data* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	nbargs = script_lastdata(st)+1;
	if(nbargs%2) {
		ShowError("buildin_checkweight: Invalid nb of args should be a multiple of 2.\n");
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}
	slots = pc_inventoryblank(sd); //nb of empty slot

	for (i = 2; i < nbargs; i += 2) {
		unsigned short nameid, amount;
		struct script_data* data = script_getdata(st,i);
		get_val(st, data); // Convert into value in case of a variable
		if( data_isstring(data) ) // item name
			id = itemdb_searchname(conv_str(st, data));
		else // item id
			id = itemdb_exists(conv_num(st, data));
		if( id == NULL ) {
			ShowError("buildin_checkweight: Invalid item '%s'.\n", script_getstr(st,i));  // returns string, regardless of what it was
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}
		nameid = id->nameid;

		amount = script_getnum(st,i+1);
		if( amount < 1 ) {
			ShowError("buildin_checkweight: Invalid amount '%d'.\n", amount);
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}

		weight += itemdb_weight(nameid)*amount; //total weight for all chk
		if( weight + sd->weight > sd->max_weight )
		{// too heavy
			script_pushint(st,0);
			return SCRIPT_CMD_SUCCESS;
		}

		switch( pc_checkadditem(sd, nameid, amount) ) {
			case CHKADDITEM_EXIST:
				// item is already in inventory, but there is still space for the requested amount
				break;
			case CHKADDITEM_NEW:
				if( itemdb_isstackable(nameid) ) {
					// stackable
					amount2++;
					if( slots < amount2 ) {
						script_pushint(st,0);
						return SCRIPT_CMD_SUCCESS;
					}
				} else {
					// non-stackable
					amount2 += amount;
					if( slots < amount2) {
						script_pushint(st,0);
						return SCRIPT_CMD_SUCCESS;
					}
				}
				break;
			case CHKADDITEM_OVERAMOUNT:
				script_pushint(st,0);
				return SCRIPT_CMD_SUCCESS;
		}
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(checkweight2)
{
	//variable sub checkweight
	int i = 0, slots = 0, weight = 0;
	short fail = 0;
	unsigned short amount2 = 0;

	//variable for array parsing
	struct script_data* data_it;
	struct script_data* data_nb;
	const char* name_it;
	const char* name_nb;
	int32 id_it, id_nb;
	int32 idx_it, idx_nb;
	int nb_it, nb_nb; //array size

	TBL_PC *sd;
	
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	data_it = script_getdata(st, 2);
	data_nb = script_getdata(st, 3);

	if( !data_isreference(data_it) || !data_isreference(data_nb)) {
		ShowError("buildin_checkweight2: parameter not a variable\n");
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id_it = reference_getid(data_it);
	id_nb = reference_getid(data_nb);
	idx_it = reference_getindex(data_it);
	idx_nb = reference_getindex(data_nb);
	name_it = reference_getname(data_it);
	name_nb = reference_getname(data_nb);

	if(is_string_variable(name_it) || is_string_variable(name_nb)) {
		ShowError("buildin_checkweight2: illegal type, need int\n");
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;// not supported
	}
	nb_it = script_array_highest_key(st, sd, reference_getname(data_it), reference_getref(data_it));
	nb_nb = script_array_highest_key(st, sd, reference_getname(data_nb), reference_getref(data_nb));
	if(nb_it != nb_nb) {
		ShowError("buildin_checkweight2: Size mistmatch: nb_it=%d, nb_nb=%d\n",nb_it,nb_nb);
		fail = 1;
	}

	slots = pc_inventoryblank(sd);
	for(i=0; i<nb_it; i++) {
		unsigned short amount, nameid = (int32)__64BPRTSIZE(get_val2(st,reference_uid(id_it,idx_it+i),reference_getref(data_it)));
		script_removetop(st, -1, 0);
		amount = (int32)__64BPRTSIZE(get_val2(st,reference_uid(id_nb,idx_nb+i),reference_getref(data_nb)));
		script_removetop(st, -1, 0);

		if(fail)
			continue; //cpntonie to depop rest

		if(itemdb_exists(nameid) == NULL ) {
			ShowError("buildin_checkweight2: Invalid item '%d'.\n", nameid);
			fail=1;
			continue;
		}
		if(amount < 0 ) {
			ShowError("buildin_checkweight2: Invalid amount '%d'.\n", amount);
			fail = 1;
			continue;
		}

		weight += itemdb_weight(nameid)*amount;
		if( weight + sd->weight > sd->max_weight ) {
			fail = 1;
			continue;
		}
		switch( pc_checkadditem(sd, nameid, amount) ) {
			case CHKADDITEM_EXIST:
				// item is already in inventory, but there is still space for the requested amount
				break;
			case CHKADDITEM_NEW:
				if( itemdb_isstackable(nameid) ) {// stackable
					amount2++;
					if( slots < amount2 )
						fail = 1;
				} else {// non-stackable
					amount2 += amount;
					if( slots < amount2 ) {
						fail = 1;
					}
				}
				break;
			case CHKADDITEM_OVERAMOUNT:
				fail = 1;
		} //end switch
	} //end loop DO NOT break it prematurly we need to depop all stack

	fail ? script_pushint(st,0) : script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * getitem <item id>,<amount>{,<account ID>};
 * getitem "<item name>",<amount>{,<account ID>};
 *
 * getitembound <item id>,<amount>,<type>{,<account ID>};
 * getitembound "<item id>",<amount>,<type>{,<account ID>};
 * Type:
 *	0 - No bound
 *	1 - Account Bound
 *	2 - Guild Bound
 *	3 - Party Bound
 *	4 - Character Bound
 *------------------------------------------*/
BUILDIN_FUNC(getitem)
{
	int get_count, i;
	unsigned short nameid, amount;
	struct item it;
	TBL_PC *sd;
	struct script_data *data;
	unsigned char flag = 0;
	const char* command = script_getfuncname(st);
	struct item_data *id = NULL;

	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ) {// "<item name>"
		const char *name = conv_str(st,data);
		id = itemdb_searchname(name);
		if( id == NULL ){
			ShowError("buildin_getitem: Nonexistant item %s requested.\n", name);
			return SCRIPT_CMD_FAILURE; //No item created.
		}
		nameid = id->nameid;
	} else if( data_isint(data) ) {// <item id>
		nameid = conv_num(st,data);
		if( !(id = itemdb_exists(nameid)) ){
			ShowError("buildin_getitem: Nonexistant item %d requested.\n", nameid);
			return SCRIPT_CMD_FAILURE; //No item created.
		}
	} else {
		ShowError("buildin_getitem: invalid data type for argument #1 (%d).", data->type);
		return SCRIPT_CMD_FAILURE;
	}

	// <amount>
	if( (amount = script_getnum(st,3)) <= 0)
		return SCRIPT_CMD_SUCCESS; //return if amount <=0, skip the useles iteration

	memset(&it,0,sizeof(it));
	it.nameid = nameid;
	it.identify = 1;
	it.bound = BOUND_NONE;

	if( !strcmp(command,"getitembound") ) {
		char bound = script_getnum(st,4);
		if( bound < BOUND_NONE || bound >= BOUND_MAX ) {
			ShowError("script_getitembound: Not a correct bound type! Type=%d\n",bound);
			return SCRIPT_CMD_FAILURE;
		}
		script_mapid2sd(5,sd);
		it.bound = bound;
	} else {
		script_mapid2sd(4,sd);
	}

	if( sd == NULL ) // no target
		return SCRIPT_CMD_SUCCESS;

	//Check if it's stackable.
	if (!itemdb_isstackable2(id))
		get_count = 1;
	else
		get_count = amount;

	for (i = 0; i < amount; i += get_count)
	{
		// if not pet egg
		if (!pet_create_egg(sd, nameid))
		{
			if ((flag = pc_additem(sd, &it, get_count, LOG_TYPE_SCRIPT)))
			{
				clif_additem(sd, 0, 0, flag);
				if( pc_candrop(sd,&it) )
					map_addflooritem(&it,get_count,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
			}
		}
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * getitem2 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>};
 * getitem2 "<item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>};
 *
 * getitembound2 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<bound type>{,<account ID>};
 * getitembound2 "<item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<bound type>{,<account ID>};
 *
 * getitem3 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account ID>};
 * getitem3 "<item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account ID>};
 *
 * getitembound3 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<bound type>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account ID>};
 * getitembound3 "<item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<bound type>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account ID>};
 * Type:
 *	0 - No bound
 *	1 - Account Bound
 *	2 - Guild Bound
 *	3 - Party Bound
 *	4 - Character Bound
 *------------------------------------------*/
BUILDIN_FUNC(getitem2)
{
	unsigned short nameid, amount;
	int iden, ref, attr;
	unsigned short c1, c2, c3, c4;
	char bound = BOUND_NONE;
	struct item_data *item_data = NULL;
	struct item item_tmp;
	TBL_PC *sd;
	struct script_data *data;
	const char* command = script_getfuncname(st);
	int offset = 0;

	if( !strncmp(command,"getitembound",12) ) {
		int aid_pos = 12;
		bound = script_getnum(st,11);
		if( bound < BOUND_NONE || bound >= BOUND_MAX ) {
			ShowError("script_getitembound2: Not a correct bound type! Type=%d\n",bound);
			return SCRIPT_CMD_FAILURE;
		}
		if (command[strlen(command)-1] == '3') {
			offset = 12;
			aid_pos = 15;
		}
		script_mapid2sd(aid_pos,sd);
	} else {
		int aid_pos = 11;
		if (strcmpi(command,"getitem3") == 0) {
			offset = 11;
			aid_pos = 14;
		} 
		script_mapid2sd(aid_pos,sd);
	}

	if( sd == NULL ) // no target
		return SCRIPT_CMD_SUCCESS;

	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ) {
		const char *name = conv_str(st,data);
		if( (item_data = itemdb_searchname(name)) == NULL ){
			ShowError("buildin_getitem2: Nonexistant item %s requested (by conv_str).\n", name);
			return SCRIPT_CMD_FAILURE; //No item created.
		}
		nameid = item_data->nameid;
	} else {
		nameid = conv_num(st,data);
		if( (item_data = itemdb_exists(nameid)) == NULL ){
			ShowError("buildin_getitem2: Nonexistant item %d requested (by conv_num).\n", nameid);
			return SCRIPT_CMD_FAILURE; //No item created.
		}
	}

	amount = script_getnum(st,3);
	iden = script_getnum(st,4);
	ref = script_getnum(st,5);
	attr = script_getnum(st,6);
	c1 = (unsigned short)script_getnum(st,7);
	c2 = (unsigned short)script_getnum(st,8);
	c3 = (unsigned short)script_getnum(st,9);
	c4 = (unsigned short)script_getnum(st,10);

	if( item_data ) {
		int get_count = 0, i;
		memset(&item_tmp,0,sizeof(item_tmp));
		if( item_data->type == IT_WEAPON || item_data->type == IT_ARMOR || item_data->type == IT_SHADOWGEAR ) {
			if(ref > MAX_REFINE)
				ref = MAX_REFINE;
		}
		else if( item_data->type == IT_PETEGG ) {
			iden = 1;
			ref = 0;
		}
		else {
			iden = 1;
			ref = attr = 0;
		}

		item_tmp.nameid = nameid;
		item_tmp.identify = iden;
		item_tmp.refine = ref;
		item_tmp.attribute = attr;
		item_tmp.card[0] = c1;
		item_tmp.card[1] = c2;
		item_tmp.card[2] = c3;
		item_tmp.card[3] = c4;
		item_tmp.bound = bound;

		if (offset != 0) {
			int res = script_getitem_randomoption(st, sd, &item_tmp, command, offset);
			if (res == SCRIPT_CMD_FAILURE)
				return SCRIPT_CMD_FAILURE;
		}

		//Check if it's stackable.
		if (!itemdb_isstackable2(item_data))
			get_count = 1;
		else
			get_count = amount;

		for (i = 0; i < amount; i += get_count)
		{
			// if not pet egg
			if (!pet_create_egg(sd, nameid))
			{
				unsigned char flag = 0;
				if ((flag = pc_additem(sd, &item_tmp, get_count, LOG_TYPE_SCRIPT)))
				{
					clif_additem(sd, 0, 0, flag);
					if( pc_candrop(sd,&item_tmp) )
						map_addflooritem(&item_tmp,get_count,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
				}
			}
		}
	}
	return SCRIPT_CMD_SUCCESS;
}

/** Gives rental item to player
 * rentitem <item id>,<seconds>{,<account_id>}
 * rentitem "<item name>",<seconds>{,<account_id>}
 */
BUILDIN_FUNC(rentitem) {
	struct map_session_data *sd;
	struct script_data *data;
	struct item it;
	int seconds;
	unsigned short nameid = 0;
	unsigned char flag = 0;

	data = script_getdata(st,2);
	get_val(st,data);

	if (!script_accid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	if( data_isstring(data) )
	{
		const char *name = conv_str(st,data);
		struct item_data *itd = itemdb_searchname(name);
		if( itd == NULL )
		{
			ShowError("buildin_rentitem: Nonexistant item %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
		nameid = itd->nameid;
	}
	else if( data_isint(data) )
	{
		nameid = conv_num(st,data);
		if( nameid == 0 || !itemdb_exists(nameid) )
		{
			ShowError("buildin_rentitem: Nonexistant item %hu requested.\n", nameid);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else
	{
		ShowError("buildin_rentitem: invalid data type for argument #1 (%d).\n", data->type);
		return SCRIPT_CMD_FAILURE;
	}

	seconds = script_getnum(st,3);
	memset(&it, 0, sizeof(it));
	it.nameid = nameid;
	it.identify = 1;
	it.expire_time = (unsigned int)(time(NULL) + seconds);
	it.bound = BOUND_NONE;

	if( (flag = pc_additem(sd, &it, 1, LOG_TYPE_SCRIPT)) )
	{
		clif_additem(sd, 0, 0, flag);
		return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Gives rental item to player with advanced option
 * rentitem2 <item id>,<time>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account_id>};
 * rentitem2 "<item name>",<time>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account_id>};
 *
 * rentitem3 <item id>,<time>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account_id>};
 * rentitem3 "<item name>",<time>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account_id>};
 */
BUILDIN_FUNC(rentitem2) {
	struct map_session_data *sd;
	struct script_data *data;
	struct item it;
	struct item_data *id;
	int seconds;
	unsigned short nameid = 0;
	unsigned char flag = 0;
	int iden,ref,attr,c1,c2,c3,c4;
	const char *funcname = script_getfuncname(st);

	data = script_getdata(st,2);
	get_val(st,data);

	if (funcname[strlen(funcname)-1] == '3') {
		if (!script_accid2sd(14,sd))
			return SCRIPT_CMD_FAILURE;
	} else if (!script_accid2sd(11,sd))
		return SCRIPT_CMD_FAILURE;

	if( data_isstring(data) ) {
		const char *name = conv_str(st,data);
		id = itemdb_searchname(name);
		if( id == NULL ) {
			ShowError("buildin_rentitem2: Nonexistant item %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
		nameid = id->nameid;
	}
	else if( data_isint(data) ) {
		nameid = conv_num(st,data);
		if( !(id = itemdb_search(nameid))) {
			ShowError("buildin_rentitem2: Nonexistant item %hu requested.\n", nameid);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		ShowError("buildin_rentitem2: invalid data type for argument #1 (%d).\n", data->type);
		return SCRIPT_CMD_FAILURE;
	}
	
	seconds = script_getnum(st,3);
	iden = script_getnum(st,4);
	ref = script_getnum(st,5);
	attr = script_getnum(st,6);

	if (id->type==IT_WEAPON || id->type==IT_ARMOR || id->type==IT_SHADOWGEAR) {
		if(ref > MAX_REFINE) ref = MAX_REFINE;
	}
	else if (id->type==IT_PETEGG) {
		iden = 1;
		ref = 0;
	}
	else {
		iden = 1;
		ref = attr = 0;
	}

	c1 = (short)script_getnum(st,7);
	c2 = (short)script_getnum(st,8);
	c3 = (short)script_getnum(st,9);
	c4 = (short)script_getnum(st,10);

	memset(&it, 0, sizeof(it));
	it.nameid = nameid;
	it.identify = iden;
	it.refine = ref;
	it.attribute = attr;
	it.card[0] = (short)c1;
	it.card[1] = (short)c2;
	it.card[2] = (short)c3;
	it.card[3] = (short)c4;
	it.expire_time = (unsigned int)(time(NULL) + seconds);

	if (funcname[strlen(funcname)-1] == '3') {
		int res = script_getitem_randomoption(st, sd, &it, funcname, 11);
		if (res != SCRIPT_CMD_SUCCESS)
			return res;
	}

	if( (flag = pc_additem(sd, &it, 1, LOG_TYPE_SCRIPT)) ) {
		clif_additem(sd, 0, 0, flag);
		return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * gets an item with someone's name inscribed [Skotlex]
 * getinscribeditem item_num, character_name
 * Returned Qty is always 1, only works on equip-able
 * equipment
 *------------------------------------------*/
BUILDIN_FUNC(getnameditem)
{
	unsigned short nameid;
	struct item item_tmp;
	TBL_PC *sd, *tsd;
	struct script_data *data;

	if (!script_rid2sd(sd))
	{	//Player not attached!
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data == NULL)
		{	//Failed
			script_pushint(st,0);
			return SCRIPT_CMD_SUCCESS;
		}
		nameid = item_data->nameid;
	}else
		nameid = conv_num(st,data);

	if(!itemdb_exists(nameid)/* || itemdb_isstackable(nameid)*/)
	{	//Even though named stackable items "could" be risky, they are required for certain quests.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	data=script_getdata(st,3);
	get_val(st,data);
	if( data_isstring(data) )	//Char Name
		tsd=map_nick2sd(conv_str(st,data),false);
	else	//Char Id was given
		tsd=map_charid2sd(conv_num(st,data));

	if( tsd == NULL )
	{	//Failed
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid=nameid;
	item_tmp.amount=1;
	item_tmp.identify=1;
	item_tmp.card[0]=CARD0_CREATE; //we don't use 255! because for example SIGNED WEAPON shouldn't get TOP10 BS Fame bonus [Lupus]
	item_tmp.card[2]=tsd->status.char_id;
	item_tmp.card[3]=tsd->status.char_id >> 16;
	if(pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT)) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;	//Failed to add item, we will not drop if they don't fit
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * gets a random item ID from an item group [Skotlex]
 * groupranditem <group_num>{,<sub_group>};
 *------------------------------------------*/
BUILDIN_FUNC(grouprandomitem) {
	struct s_item_group_entry *entry = NULL;
	int sub_group = 1;

	FETCH(3, sub_group);
	entry = itemdb_get_randgroupitem(script_getnum(st,2),sub_group);
	if (!entry) {
		ShowError("buildin_grouprandomitem: Invalid item group with group_id '%d', sub_group '%d'.\n", script_getnum(st,2), sub_group);
		script_pushint(st,UNKNOWN_ITEM_ID);
		return SCRIPT_CMD_FAILURE;
	}
	script_pushint(st,entry->nameid);
	return SCRIPT_CMD_SUCCESS;
}

/**
* makeitem <item id>,<amount>,"<map name>",<X>,<Y>;
* makeitem "<item name>",<amount>,"<map name>",<X>,<Y>;
*/
BUILDIN_FUNC(makeitem) {
	uint16 nameid, amount, flag = 0, x, y;
	const char *mapname;
	int m;
	struct item item_tmp;
	struct script_data *data;

	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name = conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid = item_data->nameid;
		else
			nameid = UNKNOWN_ITEM_ID;
	}
	else
		nameid = conv_num(st,data);

	amount = script_getnum(st,3);
	mapname	= script_getstr(st,4);
	x = script_getnum(st,5);
	y = script_getnum(st,6);

	if(strcmp(mapname,"this")==0) {
		TBL_PC *sd;
		if (!script_rid2sd(sd))
			return SCRIPT_CMD_SUCCESS; //Failed...
		m = sd->bl.m;
	} else
		m = map_mapname2mapid(mapname);

	if(nameid<0) {
		nameid = -nameid;
		flag = 1;
	}

	if(nameid > 0) {
		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid = nameid;
		if(!flag)
			item_tmp.identify = 1;
		else
			item_tmp.identify = itemdb_isidentified(nameid);

		map_addflooritem(&item_tmp,amount,m,x,y,0,0,0,4,0);
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * makeitem2 <item id>,<amount>,"<map name>",<X>,<Y>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>;
 * makeitem2 "<item name>",<amount>,"<map name>",<X>,<Y>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>;
 *
 * makeitem3 <item id>,<amount>,"<map name>",<X>,<Y>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>;
 * makeitem3 "<item name>",<amount>,"<map name>",<X>,<Y>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>;
 */
BUILDIN_FUNC(makeitem2) {
	uint16 nameid, amount, x, y;
	const char *mapname;
	int m;
	struct item item_tmp;
	struct script_data *data;
	struct item_data *id;
	const char *funcname = script_getfuncname(st);

	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name = conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid = item_data->nameid;
		else
			nameid = UNKNOWN_ITEM_ID;
	}
	else
		nameid = conv_num(st,data);

	amount = script_getnum(st,3);
	mapname	= script_getstr(st,4);
	x = script_getnum(st,5);
	y = script_getnum(st,6);

	if (strcmp(mapname,"this")==0) {
		TBL_PC *sd;
		if (!script_rid2sd(sd))
			return SCRIPT_CMD_SUCCESS; //Failed...
		m = sd->bl.m;
	}
	else
		m = map_mapname2mapid(mapname);
	
	if ((id = itemdb_search(nameid))) {
		char iden, ref, attr;
		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid = nameid;

		iden = (char)script_getnum(st,7);
		ref = (char)script_getnum(st,8);
		attr = (char)script_getnum(st,9);		

		if (id->type==IT_WEAPON || id->type==IT_ARMOR || id->type==IT_SHADOWGEAR) {
			if(ref > MAX_REFINE) ref = MAX_REFINE;
		}
		else if (id->type==IT_PETEGG) {
			iden = 1;
			ref = 0;
		}
		else {
			iden = 1;
			ref = attr = 0;
		}
		
		item_tmp.identify = iden;
		item_tmp.refine = ref;
		item_tmp.attribute = attr;
		item_tmp.card[0] = script_getnum(st,10);
		item_tmp.card[1] = script_getnum(st,11);
		item_tmp.card[2] = script_getnum(st,12);
		item_tmp.card[3] = script_getnum(st,13);

		if (funcname[strlen(funcname)-1] == '3') {
			int res = script_getitem_randomoption(st, nullptr, &item_tmp, funcname, 14);
			if (res != SCRIPT_CMD_SUCCESS)
				return res;
		}

		map_addflooritem(&item_tmp,amount,m,x,y,0,0,0,4,0);
	}
	else
		return SCRIPT_CMD_FAILURE;
	return SCRIPT_CMD_SUCCESS;
}

/// Counts / deletes the current item given by idx.
/// Used by buildin_delitem_search
/// Relies on all input data being already fully valid.
static void buildin_delitem_delete(struct map_session_data* sd, int idx, int* amount, uint8 loc, bool delete_items)
{
	int delamount;
	struct item *itm = NULL;
	struct s_storage *gstor = NULL;

	switch(loc) {
		case TABLE_CART:
			itm = &sd->cart.u.items_cart[idx];
			break;
		case TABLE_STORAGE:
			itm = &sd->storage.u.items_storage[idx];
			break;
		case TABLE_GUILD_STORAGE:
		{
			gstor = guild2storage2(sd->status.guild_id);

			itm = &gstor->u.items_guild[idx];
		}
			break;
		default: // TABLE_INVENTORY
			itm = &sd->inventory.u.items_inventory[idx];
			break;
	}

	delamount = ( amount[0] < itm->amount ) ? amount[0] : itm->amount;

	if( delete_items )
	{
		if( itemdb_type(itm->nameid) == IT_PETEGG && itm->card[0] == CARD0_PET )
		{// delete associated pet
			intif_delete_petdata(MakeDWord(itm->card[1], itm->card[2]));
		}
		switch(loc) {
			case TABLE_CART:
				pc_cart_delitem(sd,idx,delamount,0,LOG_TYPE_SCRIPT);
				break;
			case TABLE_STORAGE:
				storage_delitem(sd,&sd->storage,idx,delamount);
				log_pick_pc(sd,LOG_TYPE_SCRIPT,-delamount,itm);
				break;
			case TABLE_GUILD_STORAGE:
				gstor->lock = true;
				storage_guild_delitem(sd, gstor, idx, delamount);
				log_pick_pc(sd, LOG_TYPE_SCRIPT, -delamount, itm);
				storage_guild_storageclose(sd);
				gstor->lock = false;
				break;
			default: // TABLE_INVENTORY
				pc_delitem(sd, idx, delamount, 0, 0, LOG_TYPE_SCRIPT);
				break;
		}
	}

	amount[0]-= delamount;
}


/// Searches for item(s) and checks, if there is enough of them.
/// Used by delitem and delitem2
/// Relies on all input data being already fully valid.
/// @param exact_match will also match item by specified attributes
///   0x0: Only item id
///   0x1: identify, attributes, cards
///   0x2: random option
/// @return true when all items could be deleted, false when there were not enough items to delete
static bool buildin_delitem_search(struct map_session_data* sd, struct item* it, uint8 exact_match, uint8 loc)
{
	bool delete_items = false;
	int i, amount, size;
	struct item *items;

	// prefer always non-equipped items
	it->equip = 0;

	// when searching for nameid only, prefer additionally
	if( !exact_match )
	{
		// non-refined items
		it->refine = 0;
		// card-less items
		memset(it->card, 0, sizeof(it->card));
	}

	switch(loc) {
		case TABLE_CART:
			size = MAX_CART;
			items = sd->cart.u.items_cart;
			break;
		case TABLE_STORAGE:
			size = MAX_STORAGE;
			items = sd->storage.u.items_storage;
			break;
		case TABLE_GUILD_STORAGE:
		{
			struct s_storage *gstor = guild2storage2(sd->status.guild_id);

			size = MAX_GUILD_STORAGE;
			items = gstor->u.items_guild;
		}
			break;
		default: // TABLE_INVENTORY
			size = MAX_INVENTORY;
			items = sd->inventory.u.items_inventory;
			break;
	}

	for(;;)
	{
		unsigned short important = 0;
		amount = it->amount;

		// 1st pass -- less important items / exact match
		for( i = 0; amount && i < size; i++ )
		{
			struct item *itm = NULL;

			if( !&items[i] || !(itm = &items[i])->nameid || itm->nameid != it->nameid )
			{// wrong/invalid item
				continue;
			}

			if( itm->equip != it->equip || itm->refine != it->refine )
			{// not matching attributes
				important++;
				continue;
			}

			if( exact_match )
			{
				if( (exact_match&0x1) && ( itm->identify != it->identify || itm->attribute != it->attribute || memcmp(itm->card, it->card, sizeof(itm->card)) ) )
				{// not matching exact attributes
					continue;
				}
				if (exact_match&0x2) {
					uint8 j;
					for (j = 0; j < MAX_ITEM_RDM_OPT; j++) {
						if (itm->option[j].id != it->option[j].id || itm->option[j].value != it->option[j].value || itm->option[j].param != it->option[j].param)
							break;
					}
					if (j != MAX_ITEM_RDM_OPT)
						continue;
				}
			}
			else
			{
				if( itemdb_type(itm->nameid) == IT_PETEGG )
				{
					if( itm->card[0] == CARD0_PET && CheckForCharServer() )
					{// pet which cannot be deleted
						continue;
					}
				}
				else if( memcmp(itm->card, it->card, sizeof(itm->card)) )
				{// named/carded item
					important++;
					continue;
				}
			}

			// count / delete item
			buildin_delitem_delete(sd, i, &amount, loc, delete_items);
		}

		// 2nd pass -- any matching item
		if( amount == 0 || important == 0 )
		{// either everything was already consumed or no items were skipped
			;
		}
		else for( i = 0; amount && i < size; i++ )
		{
			struct item *itm = NULL;

			if( !&items[i] || !(itm = &items[i])->nameid || itm->nameid != it->nameid )
			{// wrong/invalid item
				continue;
			}

			if( itemdb_type(itm->nameid) == IT_PETEGG && itm->card[0] == CARD0_PET && CheckForCharServer() )
			{// pet which cannot be deleted
				continue;
			}

			if( exact_match )
			{
				if( (exact_match&0x1) && ( itm->refine != it->refine || itm->identify != it->identify || itm->attribute != it->attribute || memcmp(itm->card, it->card, sizeof(itm->card)) ) )
				{// not matching attributes
					continue;
				}
				if (exact_match&0x2) {
					uint8 j;
					for (j = 0; j < MAX_ITEM_RDM_OPT; j++) {
						if (itm->option[j].id != it->option[j].id || itm->option[j].value != it->option[j].value || itm->option[j].param != it->option[j].param)
							break;
					}
					if (j != MAX_ITEM_RDM_OPT)
						continue;
				}
			}

			// count / delete item
			buildin_delitem_delete(sd, i, &amount, loc, delete_items);
		}

		if( amount )
		{// not enough items
			return false;
		}
		else if( delete_items )
		{// we are done with the work
			return true;
		}
		else
		{// get rid of the items now
			delete_items = true;
		}
	}
}


/// Deletes items from the target/attached player.
/// Prioritizes ordinary items.
///
/// delitem <item id>,<amount>{,<account id>}
/// delitem "<item name>",<amount>{,<account id>}
/// cartdelitem <item id>,<amount>{,<account id>}
/// cartdelitem "<item name>",<amount>{,<account id>}
/// storagedelitem <item id>,<amount>{,<account id>}
/// storagedelitem "<item name>",<amount>{,<account id>}
/// guildstoragedelitem <item id>,<amount>{,<account id>}
/// guildstoragedelitem "<item name>",<amount>{,<account id>}
BUILDIN_FUNC(delitem)
{
	TBL_PC *sd;
	struct item it;
	struct script_data *data;
	uint8 loc = 0;
	char* command = (char*)script_getfuncname(st);

	if(!strncmp(command, "cart", 4))
		loc = TABLE_CART;
	else if(!strncmp(command, "storage", 7))
		loc = TABLE_STORAGE;
	else if(!strncmp(command, "guildstorage", 12))
		loc = TABLE_GUILD_STORAGE;

	if( !script_accid2sd(4,sd) ){
		// In any case cancel script execution
		st->state = END;
		return SCRIPT_CMD_SUCCESS;
	}

	if (loc == TABLE_CART && !pc_iscarton(sd)) {
		ShowError("buildin_cartdelitem: player doesn't have cart (CID=%d).\n", sd->status.char_id);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}
	if (loc == TABLE_GUILD_STORAGE) {
		struct s_storage *gstor = guild2storage2(sd->status.guild_id);

		if (gstor == NULL || sd->state.storage_flag) {
			script_pushint(st, -1);
			return SCRIPT_CMD_FAILURE;
		}
	}

	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) )
	{
		const char* item_name = conv_str(st,data);
		struct item_data* id = itemdb_searchname(item_name);
		if( id == NULL )
		{
			ShowError("buildin_%s: unknown item \"%s\".\n", command, item_name);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
		it.nameid = id->nameid;// "<item name>"
	}
	else
	{
		it.nameid = conv_num(st,data);// <item id>
		if( !itemdb_exists( it.nameid ) )
		{
			ShowError("buildin_%s: unknown item \"%hu\".\n", command, it.nameid);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}

	it.amount=script_getnum(st,3);

	if( it.amount <= 0 )
		return SCRIPT_CMD_SUCCESS;// nothing to do

	if( buildin_delitem_search(sd, &it, 0, loc) )
	{// success
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_%s: failed to delete %d items (AID=%d item_id=%hu).\n", command, it.amount, sd->status.account_id, it.nameid);
	st->state = END;
	st->mes_active = 0;
	clif_scriptclose(sd, st->oid);
	return SCRIPT_CMD_FAILURE;
}

/// Deletes items from the target/attached player.
///
/// delitem2 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// delitem2 "<Item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// cartdelitem2 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// cartdelitem2 "<Item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// storagedelitem2 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// storagedelitem2 "<Item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// guildstoragedelitem2 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// guildstoragedelitem2 "<Item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>{,<account ID>}
/// delitem3 <item id>,<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account ID>};
/// delitem3 "<item name>",<amount>,<identify>,<refine>,<attribute>,<card1>,<card2>,<card3>,<card4>,<RandomIDArray>,<RandomValueArray>,<RandomParamArray>{,<account ID>};
BUILDIN_FUNC(delitem2)
{
	TBL_PC *sd;
	struct item it;
	struct script_data *data;
	uint8 loc = 0;
	char* command = (char*)script_getfuncname(st);
	int aid_pos = 11;
	uint8 flag = 0x1;

	if(!strncmp(command, "cart", 4))
		loc = TABLE_CART;
	else if(!strncmp(command, "storage", 7))
		loc = TABLE_STORAGE;
	else if(!strncmp(command, "guildstorage", 12))
		loc = TABLE_GUILD_STORAGE;

	if (command[strlen(command)-1] == '3')
		aid_pos = 14;

	if( !script_accid2sd(aid_pos,sd) ){
		// In any case cancel script execution
		st->state = END;
		return SCRIPT_CMD_SUCCESS;
	}

	if (loc == TABLE_CART && !pc_iscarton(sd)) {
		ShowError("buildin_cartdelitem: player doesn't have cart (CID=%d).\n", sd->status.char_id);
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}
	if (loc == TABLE_GUILD_STORAGE) {
		struct s_storage *gstor = guild2storage2(sd->status.guild_id);

		if (gstor == NULL || sd->state.storage_flag) {
			script_pushint(st, -1);
			return SCRIPT_CMD_FAILURE;
		}
	}

	memset(&it, 0, sizeof(it));

	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) )
	{
		const char* item_name = conv_str(st,data);
		struct item_data* id = itemdb_searchname(item_name);
		if( id == NULL )
		{
			ShowError("buildin_%s: unknown item \"%s\".\n", command, item_name);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
		it.nameid = id->nameid;// "<item name>"
	}
	else
	{
		it.nameid = conv_num(st,data);// <item id>
		if( !itemdb_exists( it.nameid ) )
		{
			ShowError("buildin_%s: unknown item \"%hu\".\n", command, it.nameid);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}

	it.amount=script_getnum(st,3);
	it.identify=script_getnum(st,4);
	it.refine=script_getnum(st,5);
	it.attribute=script_getnum(st,6);
	it.card[0]=(short)script_getnum(st,7);
	it.card[1]=(short)script_getnum(st,8);
	it.card[2]=(short)script_getnum(st,9);
	it.card[3]=(short)script_getnum(st,10);

	if (command[strlen(command)-1] == '3') {
		int res = script_getitem_randomoption(st, sd, &it, command, 11);
		if (res != SCRIPT_CMD_SUCCESS)
			return SCRIPT_CMD_FAILURE;
		flag |= 0x2;
	}

	if( it.amount <= 0 )
		return SCRIPT_CMD_SUCCESS;// nothing to do

	if( buildin_delitem_search(sd, &it, flag, loc) )
	{// success
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_%s: failed to delete %d items (AID=%d item_id=%hu).\n", command, it.amount, sd->status.account_id, it.nameid);
	st->state = END;
	st->mes_active = 0;
	clif_scriptclose(sd, st->oid);
	return SCRIPT_CMD_FAILURE;
}

/*==========================================
 * Enables/Disables use of items while in an NPC [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(enableitemuse)
{
	TBL_PC *sd;
	if (script_rid2sd(sd))
		st->npc_item_flag = sd->npc_item_flag = 1;
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(disableitemuse)
{
	TBL_PC *sd;
	if (script_rid2sd(sd))
		st->npc_item_flag = sd->npc_item_flag = 0;
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Returns a character's specified stat.
 * Check pc_readparam for available options.
 * readparam <param>{,"<nick>"}
 * readparam <param>{,<char_id>}
 *------------------------------------------*/
BUILDIN_FUNC(readparam)
{
	int value;
	struct script_data *data = script_getdata(st, 2);
	TBL_PC *sd = NULL;

	if( script_hasdata(st, 3) ){
		struct script_data *data2 = script_getdata(st, 3);

		get_val(st, data2);
		if (data_isint(data2) || script_getnum(st, 3)) {
			script_charid2sd(3, sd);
		} else if (data_isstring(data2)) {
			script_nick2sd(3, sd);
		}
	}else{
		script_rid2sd(sd);
	}
	
	if( !sd ){
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	// If you use a parameter, return the value behind it
	if( reference_toparam(data) ){
		get_val_(st, data, sd);
		value = (int)data->u.num;
	}else{
		value = pc_readparam(sd,script_getnum(st, 2));
	}

	script_pushint(st,value);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Return charid identification
 * return by @num :
 *	0 : char_id
 *	1 : party_id
 *	2 : guild_id
 *	3 : account_id
 *	4 : bg_id
 *	5 : clan_id
 *------------------------------------------*/
BUILDIN_FUNC(getcharid)
{
	int num;
	TBL_PC *sd;

	num = script_getnum(st,2);

	if( !script_nick2sd(3,sd) ){
		script_pushint(st,0); //return 0, according docs
		return SCRIPT_CMD_SUCCESS;
	}

	switch( num ) {
	case 0: script_pushint(st,sd->status.char_id); break;
	case 1: script_pushint(st,sd->status.party_id); break;
	case 2: script_pushint(st,sd->status.guild_id); break;
	case 3: script_pushint(st,sd->status.account_id); break;
	case 4: script_pushint(st,sd->bg_id); break;
	case 5: script_pushint(st,sd->status.clan_id); break;
	default:
		ShowError("buildin_getcharid: invalid parameter (%d).\n", num);
		script_pushint(st,0);
		break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * returns the GID of an NPC
 *------------------------------------------*/
BUILDIN_FUNC(getnpcid)
{
	int num = script_getnum(st,2);
	struct npc_data* nd = NULL;

	if( script_hasdata(st,3) )
	{// unique npc name
		if( ( nd = npc_name2id(script_getstr(st,3)) ) == NULL )
		{
			ShowError("buildin_getnpcid: No such NPC '%s'.\n", script_getstr(st,3));
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}
	}

	switch (num) {
		case 0:
			script_pushint(st,nd ? nd->bl.id : st->oid);
			break;
		default:
			ShowError("buildin_getnpcid: invalid parameter (%d).\n", num);
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Return the name of the party_id
 * null if not found
 *------------------------------------------*/
BUILDIN_FUNC(getpartyname)
{
	int party_id;
	struct party_data* p;

	party_id = script_getnum(st,2);

	if( ( p = party_search(party_id) ) != NULL )
	{
		script_pushstrcopy(st,p->party.name);
	}
	else
	{
		script_pushconststr(st,"null");
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get the information of the members of a party by type
 * @party_id, @type
 * return by @type :
 *	- : nom des membres
 *	1 : char_id des membres
 *	2 : account_id des membres
 *------------------------------------------*/
BUILDIN_FUNC(getpartymember)
{
	struct party_data *p;
	uint8 j = 0;

	p = party_search(script_getnum(st,2));

	if (p != NULL) {
		uint8 i, type = 0;
		struct script_data *data = NULL;
		char *varname = NULL;

		if (script_hasdata(st,3))
 			type = script_getnum(st,3);

		if (script_hasdata(st,4)) {
			data = script_getdata(st, 4);
			if (!data_isreference(data)) {
				ShowError("buildin_getpartymember: Error in argument! Please give a variable to store values in.\n");
				return SCRIPT_CMD_FAILURE;
			}
			varname = reference_getname(data);
			if (type <= 0 && varname[strlen(varname)-1] != '$') {
				ShowError("buildin_getpartymember: The array %s is not string type.\n", varname);
				return SCRIPT_CMD_FAILURE;
			}
		}

		for (i = 0; i < MAX_PARTY; i++) {
			if (p->party.member[i].account_id) {
				switch (type) {
					case 2:
						if (data)
							setd_sub(st, NULL, varname, j, (void *)__64BPRTSIZE(p->party.member[i].account_id), data->ref);
						else
							mapreg_setreg(reference_uid(add_str("$@partymemberaid"), j),p->party.member[i].account_id);
						break;
					case 1:
						if (data)
							setd_sub(st, NULL, varname, j, (void *)__64BPRTSIZE(p->party.member[i].char_id), data->ref);
						else
							mapreg_setreg(reference_uid(add_str("$@partymembercid"), j),p->party.member[i].char_id);
						break;
					default:
						if (data)
							setd_sub(st, NULL, varname, j, (void *)__64BPRTSIZE(p->party.member[i].name), data->ref);
						else
							mapreg_setregstr(reference_uid(add_str("$@partymembername$"), j),p->party.member[i].name);
						break;
				}

				j++;
			}
		}
	}

	mapreg_setreg(add_str("$@partymembercount"),j);
	script_pushint(st, j);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Retrieves party leader. if flag is specified,
 * return some of the leader data. Otherwise, return name.
 *------------------------------------------*/
BUILDIN_FUNC(getpartyleader)
{
	int party_id, type = 0, i=0;
	struct party_data *p;

	party_id=script_getnum(st,2);
	if( script_hasdata(st,3) )
		type=script_getnum(st,3);

	p=party_search(party_id);

	if (p) //Search leader
	for(i = 0; i < MAX_PARTY && !p->party.member[i].leader; i++);

	if (!p || i == MAX_PARTY) { //leader not found
		if (type)
			script_pushint(st,-1);
		else
			script_pushconststr(st,"null");
		return SCRIPT_CMD_SUCCESS;
	}

	switch (type) {
		case 1: script_pushint(st,p->party.member[i].account_id); break;
		case 2: script_pushint(st,p->party.member[i].char_id); break;
		case 3: script_pushint(st,p->party.member[i].class_); break;
		case 4: script_pushstrcopy(st,mapindex_id2name(p->party.member[i].map)); break;
		case 5: script_pushint(st,p->party.member[i].lv); break;
		default: script_pushstrcopy(st,p->party.member[i].name); break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Return the name of the @guild_id
 * null if not found
 *------------------------------------------*/
BUILDIN_FUNC(getguildname)
{
	int guild_id;
	struct guild* g;

	guild_id = script_getnum(st,2);
	if( ( g = guild_search(guild_id) ) != NULL )
		script_pushstrcopy(st,g->name);
	else 
		script_pushconststr(st,"null");
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Return the name of the guild master of @guild_id
 * null if not found
 *------------------------------------------*/
BUILDIN_FUNC(getguildmaster)
{
	int guild_id;
	struct guild* g;

	guild_id = script_getnum(st,2);
	if( ( g = guild_search(guild_id) ) != NULL )
		script_pushstrcopy(st,g->member[0].name);
	else 
		script_pushconststr(st,"null");
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getguildmasterid)
{
	int guild_id;
	struct guild* g;

	guild_id = script_getnum(st,2);
	if( ( g = guild_search(guild_id) ) != NULL )
		script_pushint(st,g->member[0].char_id);
	else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get char string information by type :
 * Return by @type :
 *	0 : char_name
 *	1 : party_name or ""
 *	2 : guild_name or ""
 *	3 : map_name
 *	- : ""
 * strcharinfo(<type>{,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(strcharinfo)
{
	TBL_PC *sd;
	int num;
	struct guild* g;
	struct party_data* p;

	if (!script_charid2sd(3,sd)) {
		script_pushconststr(st,"");
		return SCRIPT_CMD_FAILURE;
	}

	num=script_getnum(st,2);
	switch(num){
		case 0:
			script_pushstrcopy(st,sd->status.name);
			break;
		case 1:
			if( ( p = party_search(sd->status.party_id) ) != NULL ) {
				script_pushstrcopy(st,p->party.name);
			} else {
				script_pushconststr(st,"");
			}
			break;
		case 2:
			if( ( g = sd->guild ) != NULL ) {
				script_pushstrcopy(st,g->name);
			} else {
				script_pushconststr(st,"");
			}
			break;
		case 3:
			script_pushconststr(st,map_getmapdata(sd->bl.m)->name);
			break;
		default:
			ShowWarning("buildin_strcharinfo: unknown parameter.\n");
			script_pushconststr(st,"");
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get npc string information by type
 * return by @type:
 *	0 : name
 *	1 : str#
 *	2 : #str
 *	3 : ::str
 *	4 : map name
 *------------------------------------------*/
BUILDIN_FUNC(strnpcinfo)
{
	TBL_NPC* nd;
	int num;
	char *buf,*name=NULL;

	nd = map_id2nd(st->oid);
	if (!nd) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_SUCCESS;
	}

	num = script_getnum(st,2);
	switch(num){
		case 0: // display name
			name = aStrdup(nd->name);
			break;
		case 1: // visible part of display name
			if((buf = strchr(nd->name,'#')) != NULL)
			{
				name = aStrdup(nd->name);
				name[buf - nd->name] = 0;
			} else // Return the name, there is no '#' present
				name = aStrdup(nd->name);
			break;
		case 2: // # fragment
			if((buf = strchr(nd->name,'#')) != NULL)
				name = aStrdup(buf+1);
			break;
		case 3: // unique name
			name = aStrdup(nd->exname);
			break;
		case 4: // map name
			if (nd->bl.m >= 0)
				name = aStrdup(map_getmapdata(nd->bl.m)->name);
			break;
	}

	if(name)
		script_pushstr(st, name);
	else
		script_pushconststr(st, "");

	return SCRIPT_CMD_SUCCESS;
}

/**
 * getequipid({<equipment slot>,<char_id>})
 **/
BUILDIN_FUNC(getequipid)
{
	int i, num;
	TBL_PC* sd;

	if (!script_charid2sd(3, sd)) {
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 2))
		num = script_getnum(st, 2);
	else
		num = EQI_COMPOUND_ON;

	if (num == EQI_COMPOUND_ON)
		i = current_equip_item_index;
	else if (equip_index_check(num)) // get inventory position of item
		i = pc_checkequip(sd, equip_bitmask[num]);
	else {
		ShowError( "buildin_getequipid: Unknown equip index '%d'\n", num );
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if (i >= 0 && i < MAX_INVENTORY && sd->inventory_data[i])
		script_pushint(st, sd->inventory_data[i]->nameid);
	else
		script_pushint(st, -1);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * getequipuniqueid(<equipment slot>{,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(getequipuniqueid)
{
	int i, num;
	TBL_PC* sd;
	struct item* item;

	if (!script_charid2sd(3, sd)) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	num = script_getnum(st,2);
	if ( !equip_index_check(num) ) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	// get inventory position of item
	i = pc_checkequip(sd,equip_bitmask[num]);
	if (i < 0) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	item = &sd->inventory.u.items_inventory[i];
	if (item != 0) {
		int maxlen = 256;
		char *buf = (char *)aMalloc(maxlen*sizeof(char));

		memset(buf, 0, maxlen);
		snprintf(buf, maxlen-1, "%llu", (unsigned long long)item->unique_id);

		script_pushstr(st, buf);
	} else
		script_pushconststr(st, "");

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get the equipement name at pos
 * return item jname or ""
 * getequipname(<equipment slot>{,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(getequipname)
{
	int i, num;
	TBL_PC* sd;
	struct item_data* item;

	if (!script_charid2sd(3, sd)) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	num = script_getnum(st,2);
	if( !equip_index_check(num) )
	{
		script_pushconststr(st,"");
		return SCRIPT_CMD_SUCCESS;
	}

	// get inventory position of item
	i = pc_checkequip(sd,equip_bitmask[num]);
	if( i < 0 )
	{
		script_pushconststr(st,"");
		return SCRIPT_CMD_SUCCESS;
	}

	item = sd->inventory_data[i];
	if( item != 0 )
		script_pushstrcopy(st,item->jname);
	else
		script_pushconststr(st,"");

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * getbrokenid [Valaris]
 * getbrokenid(<number>{,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(getbrokenid)
{
	int i,num,id = 0,brokencounter = 0;
	TBL_PC *sd;

	if (!script_charid2sd(3, sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	num = script_getnum(st,2);
	for(i = 0; i < MAX_INVENTORY; i++) {
		if(sd->inventory.u.items_inventory[i].attribute) {
				brokencounter++;
				if(num == brokencounter){
					id = sd->inventory.u.items_inventory[i].nameid;
					break;
				}
		}
	}

	script_pushint(st,id);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * repair [Valaris]
 * repair <num>{,<char_id>};
 *------------------------------------------*/
BUILDIN_FUNC(repair)
{
	int i,num;
	int repaircounter = 0;
	TBL_PC *sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	num = script_getnum(st,2);
	for(i = 0; i < MAX_INVENTORY; i++) {
		if(sd->inventory.u.items_inventory[i].attribute) {
				repaircounter++;
				if(num == repaircounter) {
					sd->inventory.u.items_inventory[i].attribute = 0;
					clif_equiplist(sd);
					clif_produceeffect(sd, 0, sd->inventory.u.items_inventory[i].nameid);
					clif_misceffect(&sd->bl, 3);
					break;
				}
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * repairall {<char_id>}
 *------------------------------------------*/
BUILDIN_FUNC(repairall)
{
	int i, repaircounter = 0;
	TBL_PC *sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i < MAX_INVENTORY; i++)
	{
		if(sd->inventory.u.items_inventory[i].nameid && sd->inventory.u.items_inventory[i].attribute)
		{
			sd->inventory.u.items_inventory[i].attribute = 0;
			clif_produceeffect(sd,0,sd->inventory.u.items_inventory[i].nameid);
			repaircounter++;
		}
	}

	if(repaircounter)
	{
		clif_misceffect(&sd->bl, 3);
		clif_equiplist(sd);
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Chk if player have something equiped at pos
 * getequipisequiped <pos>{,<char_id>}
 *------------------------------------------*/
BUILDIN_FUNC(getequipisequiped)
{
	int i = -1,num;
	TBL_PC *sd;

	num = script_getnum(st,2);

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if ( equip_index_check(num) )
		i=pc_checkequip(sd,equip_bitmask[num]);

	if(i >= 0)
		script_pushint(st,1);
	else
		 script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Chk if the player have something equiped at pos
 * if so chk if this item ain't marked not refinable or rental
 * return (npc)
 *	1 : true
 *	0 : false
 * getequipisenableref(<equipment slot>{,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(getequipisenableref)
{
	int i = -1,num;
	TBL_PC *sd;

	num = script_getnum(st,2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if( equip_index_check(num) )
		i = pc_checkequip(sd,equip_bitmask[num]);
	if( i >= 0 && sd->inventory_data[i] && !sd->inventory_data[i]->flag.no_refine && !sd->inventory.u.items_inventory[i].expire_time )
		script_pushint(st,1);
	else
		script_pushint(st,0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get the item refined value at pos
 * return (npc)
 *	x : refine amount
 *	0 : false (not refined)
 * getequiprefinerycnt(<equipment slot>{,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(getequiprefinerycnt)
{
	int i = -1,num;
	TBL_PC *sd;

	num = script_getnum(st,2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (equip_index_check(num))
		i=pc_checkequip(sd,equip_bitmask[num]);
	if(i >= 0)
		script_pushint(st,sd->inventory.u.items_inventory[i].refine);
	else
		script_pushint(st,0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get the weapon level value at pos
 * (pos should normally only be EQI_HAND_L or EQI_HAND_R)
 * return (npc)
 *	x : weapon level
 *	0 : false
 * getequipweaponlv(<equipment slot>{,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(getequipweaponlv)
{
	int i = -1, num;
	TBL_PC *sd;

	num = script_getnum(st, 2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	if (num == -1){
		if( current_equip_item_index == -1 ){
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}

		i = current_equip_item_index;
	}else if (equip_index_check(num))
		i = pc_checkequip(sd, equip_bitmask[num]);
	if (i >= 0 && sd->inventory_data[i])
		script_pushint(st, sd->inventory_data[i]->wlv);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get the item refine chance (from refine.txt) for item at pos
 * return (npc)
 *	x : refine chance
 *	0 : false (max refine level or unequip..)
 * getequippercentrefinery(<equipment slot>{,<enriched>,<char_id>})
 *------------------------------------------*/
BUILDIN_FUNC(getequippercentrefinery)
{
	int i = -1,num;
	bool enriched = false;
	TBL_PC *sd;

	num = script_getnum(st,2);
	if (script_hasdata(st, 3))
		enriched = script_getnum(st, 3) != 0;

	if (!script_charid2sd(4, sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (equip_index_check(num))
		i = pc_checkequip(sd,equip_bitmask[num]);
	if (i >= 0 && sd->inventory.u.items_inventory[i].nameid && sd->inventory.u.items_inventory[i].refine < MAX_REFINE) {
		enum refine_type type = REFINE_TYPE_SHADOW;
		if (sd->inventory_data[i]->type != IT_SHADOWGEAR)
			type = (enum refine_type)sd->inventory_data[i]->wlv;
		script_pushint(st, status_get_refine_chance(type, (int)sd->inventory.u.items_inventory[i].refine, enriched));
	}
	else
		script_pushint(st,0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Refine +1 item at pos and log and display refine
 * successrefitem <equipment slot>{,<count>{,<char_id>}}
 *------------------------------------------*/
BUILDIN_FUNC(successrefitem) {
	short i = -1, up = 1;
	int pos;
	TBL_PC *sd;

	pos = script_getnum(st,2);

	if (!script_charid2sd(4, sd)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 3))
		up = script_getnum(st, 3);

	if (equip_index_check(pos))
		i = pc_checkequip(sd,equip_bitmask[pos]);
	if (i >= 0) {
		unsigned int ep = sd->inventory.u.items_inventory[i].equip;

		//Logs items, got from (N)PC scripts [Lupus]
		log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->inventory.u.items_inventory[i]);

		if (sd->inventory.u.items_inventory[i].refine >= MAX_REFINE) {
			script_pushint(st, MAX_REFINE);
			return SCRIPT_CMD_SUCCESS;
		}

		sd->inventory.u.items_inventory[i].refine += up;
		sd->inventory.u.items_inventory[i].refine = cap_value( sd->inventory.u.items_inventory[i].refine, 0, MAX_REFINE);
		pc_unequipitem(sd,i,2); // status calc will happen in pc_equipitem() below

		clif_refine(sd->fd,0,i,sd->inventory.u.items_inventory[i].refine);
		clif_delitem(sd,i,1,3);

		//Logs items, got from (N)PC scripts [Lupus]
		log_pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->inventory.u.items_inventory[i]);

		clif_additem(sd,i,1,0);
		pc_equipitem(sd,i,ep);
		clif_misceffect(&sd->bl,3);
		achievement_update_objective(sd, AG_REFINE_SUCCESS, 2, sd->inventory_data[i]->wlv, sd->inventory.u.items_inventory[i].refine);
		if (sd->inventory.u.items_inventory[i].refine == MAX_REFINE &&
			sd->inventory.u.items_inventory[i].card[0] == CARD0_FORGE &&
			sd->status.char_id == (int)MakeDWord(sd->inventory.u.items_inventory[i].card[2],sd->inventory.u.items_inventory[i].card[3]))
		{ // Fame point system [DracoRPG]
			switch (sd->inventory_data[i]->wlv){
				case 1:
					pc_addfame(sd, battle_config.fame_refine_lv1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
					break;
				case 2:
					pc_addfame(sd, battle_config.fame_refine_lv2); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
					break;
				case 3:
					pc_addfame(sd, battle_config.fame_refine_lv3); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
					break;
			 }
		}
		script_pushint(st, sd->inventory.u.items_inventory[i].refine);
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_successrefitem: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
	script_pushint(st, -1);
	return SCRIPT_CMD_FAILURE;
}

/*==========================================
 * Show a failed Refine +1 attempt
 * failedrefitem <equipment slot>{,<char_id>}
 *------------------------------------------*/
BUILDIN_FUNC(failedrefitem) {
	short i = -1;
	int pos;
	TBL_PC *sd;

	pos = script_getnum(st,2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	if (equip_index_check(pos))
		i = pc_checkequip(sd,equip_bitmask[pos]);
	if (i >= 0) {
		sd->inventory.u.items_inventory[i].refine = 0;
		pc_unequipitem(sd,i,3); //recalculate bonus
		clif_refine(sd->fd,1,i,sd->inventory.u.items_inventory[i].refine); //notify client of failure
		pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT);
		clif_misceffect(&sd->bl,2); 	// display failure effect
		achievement_update_objective(sd, AG_REFINE_FAIL, 1, 1);
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_failedrefitem: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
	script_pushint(st, 0);
	return SCRIPT_CMD_FAILURE;
}

/*==========================================
 * Downgrades an Equipment Part by -1 . [Masao]
 * downrefitem <equipment slot>{,<count>{,<char_id>}}
 *------------------------------------------*/
BUILDIN_FUNC(downrefitem) {
	short i = -1, down = 1;
	int pos;
	TBL_PC *sd;

	if (!script_charid2sd(4, sd)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	pos = script_getnum(st,2);
	if (script_hasdata(st, 3))
		down = script_getnum(st, 3);

	if (equip_index_check(pos))
		i = pc_checkequip(sd,equip_bitmask[pos]);
	if (i >= 0) {
		unsigned int ep = sd->inventory.u.items_inventory[i].equip;

		//Logs items, got from (N)PC scripts [Lupus]
		log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->inventory.u.items_inventory[i]);

		pc_unequipitem(sd,i,2); // status calc will happen in pc_equipitem() below
		sd->inventory.u.items_inventory[i].refine -= down;
		sd->inventory.u.items_inventory[i].refine = cap_value( sd->inventory.u.items_inventory[i].refine, 0, MAX_REFINE);

		clif_refine(sd->fd,2,i,sd->inventory.u.items_inventory[i].refine);
		clif_delitem(sd,i,1,3);

		//Logs items, got from (N)PC scripts [Lupus]
		log_pick_pc(sd, LOG_TYPE_SCRIPT, 1, &sd->inventory.u.items_inventory[i]);

		clif_additem(sd,i,1,0);
		pc_equipitem(sd,i,ep);
		clif_misceffect(&sd->bl,2);
		achievement_update_objective(sd, AG_REFINE_FAIL, 1, sd->inventory.u.items_inventory[i].refine);
		script_pushint(st, sd->inventory.u.items_inventory[i].refine);
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_downrefitem: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
	script_pushint(st, -1);
	return SCRIPT_CMD_FAILURE;
}

/**
 * Delete the item equipped at pos.
 * delequip <equipment slot>{,<char_id>};
 **/
BUILDIN_FUNC(delequip) {
	short i = -1;
	int pos;
	int8 ret;
	TBL_PC *sd;

	pos = script_getnum(st,2);
	if (!script_charid2sd(3,sd)) {
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	if (equip_index_check(pos))
		i = pc_checkequip(sd,equip_bitmask[pos]);
	if (i >= 0) {
		pc_unequipitem(sd,i,3); //recalculate bonus
		ret = !(pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT));
	}
	else {
		ShowError("buildin_delequip: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,ret);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Break the item equipped at pos.
 * breakequip <equipment slot>{,<char_id>};
 **/
BUILDIN_FUNC(breakequip) {
	short i = -1;
	int pos;
	TBL_PC *sd;

	pos = script_getnum(st,2);
	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if (equip_index_check(pos))
		i = pc_checkequip(sd,equip_bitmask[pos]);
	if (i >= 0) {
		sd->inventory.u.items_inventory[i].attribute = 1;
		pc_unequipitem(sd,i,3);
		clif_equiplist(sd);
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_breakequip: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
	script_pushint(st,0);
	return SCRIPT_CMD_FAILURE;
}

/**
 * statusup <stat>{,<char_id>};
 **/
BUILDIN_FUNC(statusup)
{
	int type;
	TBL_PC *sd;

	type = script_getnum(st,2);

	if (!script_charid2sd(3, sd))
		return SCRIPT_CMD_FAILURE;

	pc_statusup(sd, type, 1);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * statusup2 <stat>,<amount>{,<char_id>};
 **/
BUILDIN_FUNC(statusup2)
{
	int type,val;
	TBL_PC *sd;

	type=script_getnum(st,2);
	val=script_getnum(st,3);

	if (!script_charid2sd(4, sd))
		return SCRIPT_CMD_FAILURE;

	pc_statusup2(sd,type,val);

	return SCRIPT_CMD_SUCCESS;
}

/// See 'doc/item_bonus.txt'
///
/// bonus <bonus type>,<val1>;
/// bonus2 <bonus type>,<val1>,<val2>;
/// bonus3 <bonus type>,<val1>,<val2>,<val3>;
/// bonus4 <bonus type>,<val1>,<val2>,<val3>,<val4>;
/// bonus5 <bonus type>,<val1>,<val2>,<val3>,<val4>,<val5>;
BUILDIN_FUNC(bonus)
{
	int type;
	int val1 = 0;
	int val2 = 0;
	int val3 = 0;
	int val4 = 0;
	int val5 = 0;
	TBL_PC* sd;
	struct script_data *data;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS; // no player attached

	type = script_getnum(st,2);
	switch( type ) {
		case SP_AUTOSPELL:
		case SP_AUTOSPELL_WHENHIT:
		case SP_AUTOSPELL_ONSKILL:
		case SP_SKILL_ATK:
		case SP_SKILL_HEAL:
		case SP_SKILL_HEAL2:
		case SP_ADD_SKILL_BLOW:
		case SP_CASTRATE:
		case SP_ADDEFF_ONSKILL:
		case SP_SKILL_USE_SP_RATE:
		case SP_SKILL_COOLDOWN:
		case SP_SKILL_FIXEDCAST:
		case SP_SKILL_VARIABLECAST:
		case SP_VARCASTRATE:
		case SP_FIXCASTRATE:
		case SP_SKILL_DELAY:
		case SP_SKILL_USE_SP:
		case SP_SUB_SKILL:
			// these bonuses support skill names
			data = script_getdata(st, 3);
			get_val(st, data); // Convert into value in case of a variable
			val1 = ( data_isstring(data) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
			break;
		default:
			if (script_hasdata(st, 3))
				val1 = script_getnum(st, 3);
			break;
	}

	switch( script_lastdata(st)-2 ) {
		case 0:
		case 1:
			pc_bonus(sd, type, val1);
			break;
		case 2:
			val2 = script_getnum(st,4);
			pc_bonus2(sd, type, val1, val2);
			break;
		case 3:
			val2 = script_getnum(st,4);
			val3 = script_getnum(st,5);
			pc_bonus3(sd, type, val1, val2, val3);
			break;
		case 4:
			data = script_getdata(st, 4);
			get_val(st, data); // Convert into value in case of a variable
			if( type == SP_AUTOSPELL_ONSKILL && data_isstring(data) )
				val2 = skill_name2id(script_getstr(st,4)); // 2nd value can be skill name
			else
				val2 = script_getnum(st,4);

			val3 = script_getnum(st,5);
			val4 = script_getnum(st,6);
			pc_bonus4(sd, type, val1, val2, val3, val4);
			break;
		case 5:
			data = script_getdata(st, 4);
			get_val(st, data); // Convert into value in case of a variable
			if( type == SP_AUTOSPELL_ONSKILL && data_isstring(data) )
				val2 = skill_name2id(script_getstr(st,4)); // 2nd value can be skill name
			else
				val2 = script_getnum(st,4);

			val3 = script_getnum(st,5);
			val4 = script_getnum(st,6);
			val5 = script_getnum(st,7);
			pc_bonus5(sd, type, val1, val2, val3, val4, val5);
			break;
		default:
			ShowDebug("buildin_bonus: unexpected number of arguments (%d)\n", (script_lastdata(st) - 1));
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(autobonus)
{
	unsigned int dur, pos;
	short rate;
	short atk_type = 0;
	TBL_PC* sd;
	const char *bonus_script, *other_script = NULL;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS; // no player attached

	if (current_equip_combo_pos)
		pos = current_equip_combo_pos;
	else
		pos = sd->inventory.u.items_inventory[current_equip_item_index].equip;

	if((sd->state.autobonus&pos) == pos)
		return SCRIPT_CMD_SUCCESS;

	rate = script_getnum(st,3);
	dur = script_getnum(st,4);
	bonus_script = script_getstr(st,2);
	if( !rate || !dur || !pos || !bonus_script )
		return SCRIPT_CMD_SUCCESS;

	if( script_hasdata(st,5) )
		atk_type = script_getnum(st,5);
	if( script_hasdata(st,6) )
		other_script = script_getstr(st,6);

	if( pc_addautobonus(sd->autobonus,ARRAYLENGTH(sd->autobonus),
		bonus_script,rate,dur,atk_type,other_script,pos,false) )
	{
		script_add_autobonus(bonus_script);
		if( other_script )
			script_add_autobonus(other_script);
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(autobonus2)
{
	unsigned int dur, pos;
	short rate;
	short atk_type = 0;
	TBL_PC* sd;
	const char *bonus_script, *other_script = NULL;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS; // no player attached

	if (current_equip_combo_pos)
		pos = current_equip_combo_pos;
	else
		pos = sd->inventory.u.items_inventory[current_equip_item_index].equip;

	if((sd->state.autobonus&pos) == pos)
		return SCRIPT_CMD_SUCCESS;

	rate = script_getnum(st,3);
	dur = script_getnum(st,4);
	bonus_script = script_getstr(st,2);
	if( !rate || !dur || !pos || !bonus_script )
		return SCRIPT_CMD_SUCCESS;

	if( script_hasdata(st,5) )
		atk_type = script_getnum(st,5);
	if( script_hasdata(st,6) )
		other_script = script_getstr(st,6);

	if( pc_addautobonus(sd->autobonus2,ARRAYLENGTH(sd->autobonus2),
		bonus_script,rate,dur,atk_type,other_script,pos,false) )
	{
		script_add_autobonus(bonus_script);
		if( other_script )
			script_add_autobonus(other_script);
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(autobonus3)
{
	unsigned int dur, pos;
	short rate,atk_type;
	TBL_PC* sd;
	const char *bonus_script, *other_script = NULL;
	struct script_data *data;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS; // no player attached

	if (current_equip_combo_pos)
		pos = current_equip_combo_pos;
	else
		pos = sd->inventory.u.items_inventory[current_equip_item_index].equip;

	if((sd->state.autobonus&pos) == pos)
		return SCRIPT_CMD_SUCCESS;

	rate = script_getnum(st,3);
	dur = script_getnum(st,4);
	data = script_getdata(st, 5);
	get_val(st, data); // Convert into value in case of a variable
	atk_type = ( data_isstring(data) ? skill_name2id(script_getstr(st,5)) : script_getnum(st,5) );
	bonus_script = script_getstr(st,2);
	if( !rate || !dur || !pos || !atk_type || !bonus_script )
		return SCRIPT_CMD_SUCCESS;

	if( script_hasdata(st,6) )
		other_script = script_getstr(st,6);

	if( pc_addautobonus(sd->autobonus3,ARRAYLENGTH(sd->autobonus3),
		bonus_script,rate,dur,atk_type,other_script,pos,true) )
	{
		script_add_autobonus(bonus_script);
		if( other_script )
			script_add_autobonus(other_script);
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Changes the level of a player skill.
/// <flag> defaults to 1
/// <flag>=0 : set the level of the skill
/// <flag>=1 : set the temporary level of the skill
/// <flag>=2 : add to the level of the skill
///
/// skill <skill id>,<level>,<flag>
/// skill <skill id>,<level>
/// skill "<skill name>",<level>,<flag>
/// skill "<skill name>",<level>
BUILDIN_FUNC(skill)
{
	int id;
	int level;
	int flag = ADDSKILL_TEMP;
	TBL_PC* sd;
	struct script_data *data;
	const char* command = script_getfuncname(st);

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;// no player attached, report source

	if (strcmpi(command, "addtoskill") == 0)
		flag = ADDSKILL_TEMP_ADDLEVEL;

	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	id = ( data_isstring(data) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	pc_skill(sd, id, level, (enum e_addskill_type)flag);

	return SCRIPT_CMD_SUCCESS;
}

/// Increases the level of a guild skill.
///
/// guildskill <skill id>,<amount>;
/// guildskill "<skill name>",<amount>;
BUILDIN_FUNC(guildskill)
{
	int id;
	int level;
	TBL_PC* sd;
	int i;
	struct script_data *data;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;// no player attached, report source

	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	id = ( data_isstring(data) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	for( i=0; i < level; i++ )
		guild_skillup(sd, id);

	return SCRIPT_CMD_SUCCESS;
}

/// Returns the level of the player skill.
///
/// getskilllv(<skill id>) -> <level>
/// getskilllv("<skill name>") -> <level>
BUILDIN_FUNC(getskilllv)
{
	int id;
	TBL_PC* sd;
	struct script_data *data;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;// no player attached, report source

	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	id = ( data_isstring(data) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	script_pushint(st, pc_checkskill(sd,id));

	return SCRIPT_CMD_SUCCESS;
}

/// Returns the level of the guild skill.
///
/// getgdskilllv(<guild id>,<skill id>) -> <level>
/// getgdskilllv(<guild id>,"<skill name>") -> <level>
BUILDIN_FUNC(getgdskilllv)
{
	int guild_id;
	uint16 skill_id;
	struct guild* g;
	struct script_data *data;

	guild_id = script_getnum(st,2);
	data = script_getdata(st, 3);
	get_val(st, data); // Convert into value in case of a variable
	skill_id = ( data_isstring(data) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	g = guild_search(guild_id);
	if( g == NULL )
		script_pushint(st, -1);
	else
		script_pushint(st, guild_checkskill(g,skill_id));

	return SCRIPT_CMD_SUCCESS;
}

/// Returns the 'basic_skill_check' setting.
/// This config determines if the server checks the skill level of NV_BASIC
/// before allowing the basic actions.
///
/// basicskillcheck() -> <bool>
BUILDIN_FUNC(basicskillcheck)
{
	script_pushint(st, battle_config.basic_skill_check);
	return SCRIPT_CMD_SUCCESS;
}

/// Returns the GM level of the player.
///
/// getgmlevel({<char_id>}) -> <level>
BUILDIN_FUNC(getgmlevel)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	script_pushint(st, pc_get_group_level(sd));
	return SCRIPT_CMD_SUCCESS;
}

/// Returns the group ID of the player.
///
/// getgroupid({<char_id>}) -> <int>
BUILDIN_FUNC(getgroupid)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	script_pushint(st, pc_get_group_id(sd));
	return SCRIPT_CMD_SUCCESS;
}

/// Terminates the execution of this script instance.
///
/// end
BUILDIN_FUNC(end)
{
	TBL_PC* sd;

	sd = map_id2sd(st->rid);

	st->state = END;

	if (st->stack->defsp >= 1 && st->stack->stack_data[st->stack->defsp-1].type == C_RETINFO) {
		int i;

		for(i = 0; i < st->stack->sp; i++) {
			if (st->stack->stack_data[i].type == C_RETINFO) { // Grab the first, aka the original
				struct script_retinfo *ri = st->stack->stack_data[i].u.ri;
				st->script = ri->script;
				break;
			}
		}
	}

	if( st->mes_active )
		st->mes_active = 0;

	if (sd){
		if (sd->state.callshop == 0)
			clif_scriptclose(sd, st->oid); // If a menu/select/prompt is active, close it.
		else 
			sd->state.callshop = 0;
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Checks if the player has that effect state (option).
///
/// checkoption(<option>{,<char_id>}) -> <bool>
BUILDIN_FUNC(checkoption)
{
	int option;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	option = script_getnum(st,2);
	if( sd->sc.option&option )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Checks if the player is in that body state (opt1).
///
/// checkoption1(<opt1>{,<char_id>}) -> <bool>
BUILDIN_FUNC(checkoption1)
{
	int opt1;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	opt1 = script_getnum(st,2);
	if( sd->sc.opt1 == opt1 )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Checks if the player has that health state (opt2).
///
/// checkoption2(<opt2>{,<char_id>}) -> <bool>
BUILDIN_FUNC(checkoption2)
{
	int opt2;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	opt2 = script_getnum(st,2);
	if( sd->sc.opt2&opt2 )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Changes the effect state (option) of the player.
/// <flag> defaults to 1
/// <flag>=0 : removes the option
/// <flag>=other : adds the option
///
/// setoption <option>{,<flag>{,<char_id>}};
BUILDIN_FUNC(setoption)
{
	int option;
	int flag = 1;
	TBL_PC* sd;

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	option = script_getnum(st,2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	else if( !option ){// Request to remove everything.
		flag = 0;
		option = OPTION_FALCON|OPTION_RIDING;
#ifndef NEW_CARTS
		option |= OPTION_CART;
#endif
	}
	if( flag ){// Add option
		if( option&OPTION_WEDDING && !battle_config.wedding_modifydisplay )
			option &= ~OPTION_WEDDING;// Do not show the wedding sprites
		pc_setoption(sd, sd->sc.option|option);
	} else// Remove option
		pc_setoption(sd, sd->sc.option&~option);

	return SCRIPT_CMD_SUCCESS;
}

/// Returns if the player has a cart.
///
/// checkcart({char_id}) -> <bool>
///
/// @author Valaris
BUILDIN_FUNC(checkcart)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if( pc_iscarton(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Sets the cart of the player.
/// <type> defaults to 1
/// <type>=0 : removes the cart
/// <type>=1 : Normal cart
/// <type>=2 : Wooden cart
/// <type>=3 : Covered cart with flowers and ferns
/// <type>=4 : Wooden cart with a Panda doll on the back
/// <type>=5 : Normal cart with bigger wheels, a roof and a banner on the back
///
/// setcart {<type>{,<char_id>}};
BUILDIN_FUNC(setcart)
{
	int type = 1;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if( script_hasdata(st,2) )
		type = script_getnum(st,2);
	pc_setcart(sd, type);

	return SCRIPT_CMD_SUCCESS;
}

/// Returns if the player has a falcon.
///
/// checkfalcon({char_id}) -> <bool>
///
/// @author Valaris
BUILDIN_FUNC(checkfalcon)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if( pc_isfalcon(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Sets if the player has a falcon or not.
/// <flag> defaults to 1
///
/// setfalcon {<flag>{,<char_id>}};
BUILDIN_FUNC(setfalcon)
{
	int flag = 1;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);

	pc_setfalcon(sd, flag);

	return SCRIPT_CMD_SUCCESS;
}

/// Returns if the player is riding.
///
/// checkriding({char_id}) -> <bool>
///
/// @author Valaris
BUILDIN_FUNC(checkriding)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if( pc_isriding(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Sets if the player is riding.
/// <flag> defaults to 1
///
/// setriding {<flag>{,<char_id>}};
BUILDIN_FUNC(setriding)
{
	int flag = 1;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);
	pc_setriding(sd, flag);

	return SCRIPT_CMD_SUCCESS;
}

/// Returns if the player has a warg.
///
/// checkwug({char_id}) -> <bool>
///
BUILDIN_FUNC(checkwug)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if( pc_iswug(sd) || pc_isridingwug(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Returns if the player is wearing MADO Gear.
///
/// checkmadogear({<char_id>}) -> <bool>
///
BUILDIN_FUNC(checkmadogear)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if( pc_ismadogear(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Sets if the player is riding MADO Gear.
/// <flag> defaults to 1
///
/// setmadogear {<flag>{,<char_id>}};
BUILDIN_FUNC(setmadogear)
{
	int flag = 1;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);
	pc_setmadogear(sd, flag);

	return SCRIPT_CMD_SUCCESS;
}

/// Sets the save point of the player.
///
/// save "<map name>",<x>,<y>{,{<range x>,<range y>,}<char_id>}
/// savepoint "<map name>",<x>,<y>{,{<range x>,<range y>,}<char_id>}
BUILDIN_FUNC(savepoint)
{
	int x, y, m, cid_pos = 5;
	unsigned short map_idx;
	const char* str;
	TBL_PC* sd;

	if (script_lastdata(st) > 5)
		cid_pos = 7;

	if (!script_charid2sd(cid_pos,sd))
		return SCRIPT_CMD_FAILURE;// no player attached, report source

	str = script_getstr(st, 2);

	map_idx = mapindex_name2id(str);
	if( !map_idx )
		return SCRIPT_CMD_FAILURE;

	x = script_getnum(st,3);
	y = script_getnum(st,4);
	m = map_mapindex2mapid(map_idx);

	if (cid_pos == 7) {
		int dx = script_getnum(st,5), dy = script_getnum(st,6),
			x1 = x + dx, y1 = y + dy,
			x0 = x - dx, y0 = y - dy;
		uint8 n = 10;
		do {
			x = x0 + rnd()%(x1-x0+1);
			y = y0 + rnd()%(y1-y0+1);
		} while (m != -1 && (--n) > 0 && !map_getcell(m, x, y, CELL_CHKPASS));
	}

	// Check for valid coordinates if map in local map-server
	if (m != -1 && !map_getcell(m, x, y, CELL_CHKPASS)) {
		ShowError("buildin_savepoint: Invalid coordinates %d,%d at map %s.\n", x, y, str);
		return SCRIPT_CMD_FAILURE;
	}

	pc_setsavepoint(sd, map_idx, x, y);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * GetTimeTick(0: System Tick, 1: Time Second Tick, 2: Unix epoch)
 *------------------------------------------*/
BUILDIN_FUNC(gettimetick)	/* Asgard Version */
{
	int type;
	time_t timer;
	struct tm *t;

	type=script_getnum(st,2);

	switch(type){
	case 2:
		//type 2:(Get the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC
		//        from the system clock.)
		script_pushint(st,(int)time(NULL));
		break;
	case 1:
		//type 1:(Second Ticks: 0-86399, 00:00:00-23:59:59)
		time(&timer);
		t=localtime(&timer);
		script_pushint(st,((t->tm_hour)*3600+(t->tm_min)*60+t->tm_sec));
		break;
	case 0:
	default:
		//type 0:(System Ticks)
		script_pushint(st,gettick());
		break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * GetTime(Type)
 *
 * Returns the current value of a certain date type.
 * Possible types are:
 * - DT_SECOND Current seconds
 * - DT_MINUTE Current minute
 * - DT_HOUR Current hour
 * - DT_DAYOFWEEK Day of current week
 * - DT_DAYOFMONTH Day of current month
 * - DT_MONTH Current month
 * - DT_YEAR Current year
 * - DT_DAYOFYEAR Day of current year
 *
 * If none of the above types is supplied -1 will be returned to the script
 * and the script source will be reported into the mapserver console.
 *------------------------------------------*/
BUILDIN_FUNC(gettime)
{
	int type;

	type = script_getnum(st,2);

	if( type <= DT_MIN || type >= DT_MAX ){
		ShowError( "buildin_gettime: Invalid date type %d\n", type );
		script_reportsrc(st);
		script_pushint(st,-1);
	}else{
		script_pushint(st,date_get((enum e_date_type)type));
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Returns the current server time or the provided time in a readable format.
 * gettimestr(<"time_format">,<max_length>{,<time_tick>});
 */
BUILDIN_FUNC(gettimestr)
{
	char *tmpstr;
	const char *fmtstr;
	int maxlen;
	time_t now;

	fmtstr = script_getstr(st,2);
	maxlen = script_getnum(st,3);

	if (script_hasdata(st, 4)) {
		if (script_getnum(st, 4) < 0) {
			ShowWarning("buildin_gettimestr: a positive value must be supplied to be valid.\n");
			return SCRIPT_CMD_FAILURE;
		} else
			now = (time_t)script_getnum(st, 4);
	} else
		now = time(NULL);

	tmpstr = (char *)aMalloc((maxlen+1)*sizeof(char));
	strftime(tmpstr,maxlen,fmtstr,localtime(&now));
	tmpstr[maxlen] = '\0';

	script_pushstr(st,tmpstr);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Open player storage
 *------------------------------------------*/
BUILDIN_FUNC(openstorage)
{
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	storage_storageopen(sd);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(guildopenstorage)
{
	TBL_PC* sd;
	int ret;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	ret = storage_guild_storageopen(sd);
	script_pushint(st,ret);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Make player use a skill trought item usage
 *------------------------------------------*/
/// itemskill <skill id>,<level>
/// itemskill "<skill name>",<level>
BUILDIN_FUNC(itemskill)
{
	int id;
	int lv;
	bool keep_requirement;
	TBL_PC* sd;
	struct script_data *data;

	if( !script_rid2sd(sd) || sd->ud.skilltimer != INVALID_TIMER )
		return SCRIPT_CMD_SUCCESS;

	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	id = ( data_isstring(data) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	lv = script_getnum(st,3);
	if (script_hasdata(st, 4)) {
		keep_requirement = (script_getnum(st, 4) != 0);
	} else {
		keep_requirement = false;
	}

	sd->skillitem=id;
	sd->skillitemlv=lv;
	sd->skillitem_keep_requirement = keep_requirement;
	clif_item_skill(sd,id,lv);
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Attempt to create an item
 *------------------------------------------*/
BUILDIN_FUNC(produce)
{
	int trigger;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	trigger=script_getnum(st,2);
	clif_skill_produce_mix_list(sd, -1, trigger);
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(cooking)
{
	int trigger;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	trigger=script_getnum(st,2);
	clif_cooking_list(sd, trigger, AM_PHARMACY, 1, 1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Create a pet
 *------------------------------------------*/
BUILDIN_FUNC(makepet)
{
	struct map_session_data* sd;
	uint16 mob_id;
	struct s_pet_db* pet;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_FAILURE;

	mob_id = script_getnum(st,2);
	pet = pet_db(mob_id);

	if( !pet ){
		ShowError( "buildin_makepet: failed to create a pet with mob id %hu\n", mob_id);
		return SCRIPT_CMD_FAILURE;
	}

	sd->catch_target_class = mob_id;
	intif_create_pet( sd->status.account_id, sd->status.char_id, pet->class_, mob_db(pet->class_)->lv, pet->EggID, 0, pet->intimate, 100, 0, 1, pet->jname );

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Give player exp base,job * quest_exp_rate/100
 * getexp <base xp>,<job xp>{,<char_id>};
 **/
BUILDIN_FUNC(getexp)
{
	TBL_PC* sd;
	int base=0,job=0;
	double bonus;

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	base=script_getnum(st,2);
	job =script_getnum(st,3);
	if(base<0 || job<0)
		return SCRIPT_CMD_SUCCESS;

	// bonus for npc-given exp
	bonus = battle_config.quest_exp_rate / 100.;
	if (base)
		base = (int) cap_value(base * bonus, 0, INT_MAX);
	if (job)
		job = (int) cap_value(job * bonus, 0, INT_MAX);

	pc_gainexp(sd, NULL, base, job, 1);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Gain guild exp [Celest]
 *------------------------------------------*/
BUILDIN_FUNC(guildgetexp)
{
	TBL_PC* sd;
	int exp;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	exp = script_getnum(st,2);
	if(exp < 0)
		return SCRIPT_CMD_SUCCESS;
	if(sd && sd->status.guild_id > 0)
		guild_getexp (sd, exp);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Changes the guild master of a guild [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(guildchangegm)
{
	TBL_PC *sd;
	int guild_id;
	const char *name;

	guild_id = script_getnum(st,2);
	name = script_getstr(st,3);
	sd=map_nick2sd(name,false);

	if (!sd)
		script_pushint(st,0);
	else
		script_pushint(st,guild_gm_change(guild_id, sd->status.char_id));

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Spawn a monster:
 * *monster "<map name>",<x>,<y>,"<name to show>",<mob id>,<amount>{,"<event label>",<size>,<ai>};
 *------------------------------------------*/
BUILDIN_FUNC(monster)
{
	const char* mapn	= script_getstr(st,2);
	int x				= script_getnum(st,3);
	int y				= script_getnum(st,4);
	const char* str		= script_getstr(st,5);
	int class_			= script_getnum(st,6);
	int amount			= script_getnum(st,7);
	const char* event	= "";
	unsigned int size	= SZ_SMALL;
	enum mob_ai ai		= AI_NONE;

	struct map_session_data* sd;
	int16 m;
	int i;

	if (script_hasdata(st, 8)) {
		event = script_getstr(st, 8);
		check_event(st, event);
	}

	if (script_hasdata(st, 9)) {
		size = script_getnum(st, 9);
		if (size > SZ_BIG) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing size %d for monster class %d\n", size, class_);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (script_hasdata(st, 10)) {
		ai = static_cast<enum mob_ai>(script_getnum(st, 10));
		if (ai >= AI_MAX) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing ai %d for monster class %d\n", ai, class_);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (class_ >= 0 && !mobdb_checkid(class_)) {
		ShowWarning("buildin_monster: Attempted to spawn non-existing monster class %d\n", class_);
		return SCRIPT_CMD_FAILURE;
	}

	sd = map_id2sd(st->rid);

	if (sd && strcmp(mapn, "this") == 0)
		m = sd->bl.m;
	else
		m = map_mapname2mapid(mapn);

	for(i = 0; i < amount; i++) { //not optimised
		int mobid = mob_once_spawn(sd, m, x, y, str, class_, 1, event, size, ai);

		if (mobid)
			mapreg_setreg(reference_uid(add_str("$@mobid"), i), mobid);
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Request List of Monster Drops
 *------------------------------------------*/
BUILDIN_FUNC(getmobdrops)
{
	int class_ = script_getnum(st,2);
	int i, j = 0;
	struct mob_db *mob;

	if( !mobdb_checkid(class_) )
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	mob = mob_db(class_);

	for( i = 0; i < MAX_MOB_DROP_TOTAL; i++ )
	{
		if( mob->dropitem[i].nameid < 1 )
			continue;
		if( itemdb_exists(mob->dropitem[i].nameid) == NULL )
			continue;

		mapreg_setreg(reference_uid(add_str("$@MobDrop_item"), j), mob->dropitem[i].nameid);
		mapreg_setreg(reference_uid(add_str("$@MobDrop_rate"), j), mob->dropitem[i].p);

		j++;
	}

	mapreg_setreg(add_str("$@MobDrop_count"), j);
	script_pushint(st, 1);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Spawn a monster in a random location
 * in x0,x1,y0,y1 area.
 *------------------------------------------*/
BUILDIN_FUNC(areamonster)
{
	const char* mapn	= script_getstr(st,2);
	int x0				= script_getnum(st,3);
	int y0				= script_getnum(st,4);
	int x1				= script_getnum(st,5);
	int y1				= script_getnum(st,6);
	const char* str		= script_getstr(st,7);
	int class_			= script_getnum(st,8);
	int amount			= script_getnum(st,9);
	const char* event	= "";
	unsigned int size	= SZ_SMALL;
	enum mob_ai ai		= AI_NONE;

	struct map_session_data* sd;
	int16 m;
	int i;

	if (script_hasdata(st,10)) {
		event = script_getstr(st, 10);
		check_event(st, event);
	}

	if (script_hasdata(st, 11)) {
		size = script_getnum(st, 11);
		if (size > 3) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing size %d for monster class %d\n", size, class_);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (script_hasdata(st, 12)) {
		ai = static_cast<enum mob_ai>(script_getnum(st, 12));
		if (ai >= AI_MAX) {
			ShowWarning("buildin_monster: Attempted to spawn non-existing ai %d for monster class %d\n", ai, class_);
			return SCRIPT_CMD_FAILURE;
		}
	}

	sd = map_id2sd(st->rid);

	if (sd && strcmp(mapn, "this") == 0)
		m = sd->bl.m;
	else
		m = map_mapname2mapid(mapn);

	for(i = 0; i < amount; i++) { //not optimised
		int mobid = mob_once_spawn_area(sd, m, x0, y0, x1, y1, str, class_, 1, event, size, ai);

		if (mobid)
			mapreg_setreg(reference_uid(add_str("$@mobid"), i), mobid);
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * KillMonster subcheck, verify if mob to kill ain't got an even to handle, could be force kill by allflag
 *------------------------------------------*/
 static int buildin_killmonster_sub_strip(struct block_list *bl,va_list ap)
{ //same fix but with killmonster instead - stripping events from mobs.
	TBL_MOB* md = (TBL_MOB*)bl;
	char *event=va_arg(ap,char *);
	int allflag=va_arg(ap,int);

	md->state.npc_killmonster = 1;

	if(!allflag){
		if(strcmp(event,md->npc_event)==0)
			status_kill(bl);
	}else{
		if(!md->spawn)
			status_kill(bl);
	}
	md->state.npc_killmonster = 0;
	return SCRIPT_CMD_SUCCESS;
}

static int buildin_killmonster_sub(struct block_list *bl,va_list ap)
{
	TBL_MOB* md = (TBL_MOB*)bl;
	char *event=va_arg(ap,char *);
	int allflag=va_arg(ap,int);

	if(!allflag){
		if(strcmp(event,md->npc_event)==0)
			status_kill(bl);
	}else{
		if(!md->spawn)
			status_kill(bl);
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(killmonster)
{
	const char *mapname,*event;
	int16 m,allflag=0;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);
	if(strcmp(event,"All")==0)
		allflag = 1;
	else
		check_event(st, event);

	if( (m=map_mapname2mapid(mapname))<0 )
		return SCRIPT_CMD_SUCCESS;

	if( script_hasdata(st,4) ) {
		if ( script_getnum(st,4) == 1 ) {
			map_foreachinmap(buildin_killmonster_sub, m, BL_MOB, event ,allflag);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	map_freeblock_lock();
	map_foreachinmap(buildin_killmonster_sub_strip, m, BL_MOB, event ,allflag);
	map_freeblock_unlock();
	return SCRIPT_CMD_SUCCESS;
}

static int buildin_killmonsterall_sub_strip(struct block_list *bl,va_list ap)
{ //Strips the event from the mob if it's killed the old method.
	struct mob_data *md;

	md = BL_CAST(BL_MOB, bl);
	if (md->npc_event[0])
		md->npc_event[0] = 0;

	status_kill(bl);
	return 0;
}
static int buildin_killmonsterall_sub(struct block_list *bl,va_list ap)
{
	status_kill(bl);
	return 0;
}
BUILDIN_FUNC(killmonsterall)
{
	const char *mapname;
	int16 m;
	mapname=script_getstr(st,2);

	if( (m = map_mapname2mapid(mapname))<0 )
		return SCRIPT_CMD_SUCCESS;

	if( script_hasdata(st,3) ) {
		if ( script_getnum(st,3) == 1 ) {
			map_foreachinmap(buildin_killmonsterall_sub,m,BL_MOB);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	map_foreachinmap(buildin_killmonsterall_sub_strip,m,BL_MOB);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Creates a clone of a player.
 * clone map, x, y, event, char_id, master_id, mode, flag, duration
 *------------------------------------------*/
BUILDIN_FUNC(clone)
{
	TBL_PC *sd, *msd=NULL;
	uint32 char_id, master_id = 0, x, y, flag = 0;
	int16 m;
	enum e_mode mode = MD_NONE;

	unsigned int duration = 0;
	const char *mapname,*event;

	mapname=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	event=script_getstr(st,5);
	char_id=script_getnum(st,6);

	if( script_hasdata(st,7) )
		master_id=script_getnum(st,7);

	if( script_hasdata(st,8) )
		mode=static_cast<e_mode>(script_getnum(st,8));

	if( script_hasdata(st,9) )
		flag=script_getnum(st,9);

	if( script_hasdata(st,10) )
		duration=script_getnum(st,10);

	check_event(st, event);

	m = map_mapname2mapid(mapname);
	if (m < 0)
		return SCRIPT_CMD_SUCCESS;

	sd = map_charid2sd(char_id);

	if (master_id) {
		msd = map_charid2sd(master_id);
		if (msd)
			master_id = msd->bl.id;
		else
			master_id = 0;
	}
	if (sd) //Return ID of newly crafted clone.
		script_pushint(st,mob_clone_spawn(sd, m, x, y, event, master_id, mode, flag, 1000*duration));
	else //Failed to create clone.
		script_pushint(st,0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(doevent)
{
	const char* event;
	struct map_session_data* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	event = script_getstr(st,2);

	check_event(st, event);
	npc_event(sd, event, 0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(donpcevent)
{
	const char* event = script_getstr(st,2);
	check_event(st, event);
	if( !npc_event_do(event) ) {
		struct npc_data * nd = map_id2nd(st->oid);
		ShowDebug("NPCEvent '%s' not found! (source: %s)\n",event,nd?nd->name:"Unknown");
		script_pushint(st, 0);
	} else
		script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/// for Aegis compatibility
/// basically a specialized 'donpcevent', with the event specified as two arguments instead of one
BUILDIN_FUNC(cmdothernpc)	// Added by RoVeRT
{
	const char* npc = script_getstr(st,2);
	const char* command = script_getstr(st,3);
	char event[EVENT_NAME_LENGTH];

	safesnprintf(event,EVENT_NAME_LENGTH, "%s::%s%s",npc,script_config.oncommand_event_name,command);
	check_event(st, event);

	if( npc_event_do(event) ){
		script_pushint(st, true);
	}else{
		struct npc_data * nd = map_id2nd(st->oid);
		ShowDebug("NPCEvent '%s' not found! (source: %s)\n", event, nd ? nd->name : "Unknown");
		script_pushint(st, false);
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(addtimer)
{
	int tick;
	const char* event;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	tick = script_getnum(st,2);
	event = script_getstr(st, 3);

	check_event(st, event);

	if (!pc_addeventtimer(sd,tick,event)) {
		ShowWarning("buildin_addtimer: Event timer is full, can't add new event timer. (cid:%d timer:%s)\n",sd->status.char_id,event);
		return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(deltimer)
{
	const char *event;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	event=script_getstr(st, 2);

	check_event(st, event);
	pc_deleventtimer(sd,event);
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(addtimercount)
{
	const char *event;
	int tick;
	TBL_PC* sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	tick=script_getnum(st,2);
	event=script_getstr(st,3);

	check_event(st, event);
	pc_addeventtimercount(sd,event,tick);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(initnpctimer)
{
	struct npc_data *nd;
	int flag = 0;

	if( script_hasdata(st,3) )
	{	//Two arguments: NPC name and attach flag.
		nd = npc_name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	}
	else if( script_hasdata(st,2) )
	{	//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		get_val(st,data);
		if( data_isstring(data) ) //NPC name
			nd = npc_name2id(conv_str(st, data));
		else if( data_isint(data) ) //Flag
		{
			nd = (struct npc_data *)map_id2bl(st->oid);
			flag = conv_num(st,data);
		}
		else
		{
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return SCRIPT_CMD_FAILURE;
		}
	}
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( !nd )
		return SCRIPT_CMD_SUCCESS;
	if( flag ) //Attach
	{
		TBL_PC* sd;
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;
		nd->u.scr.rid = sd->bl.id;
	}

	nd->u.scr.timertick = 0;
	npc_settimerevent_tick(nd,0);
	npc_timerevent_start(nd, st->rid);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(startnpctimer)
{
	struct npc_data *nd;
	int flag = 0;

	if( script_hasdata(st,3) )
	{	//Two arguments: NPC name and attach flag.
		nd = npc_name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	}
	else if( script_hasdata(st,2) )
	{	//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		get_val(st,data);
		if( data_isstring(data) ) //NPC name
			nd = npc_name2id(conv_str(st, data));
		else if( data_isint(data) ) //Flag
		{
			nd = (struct npc_data *)map_id2bl(st->oid);
			flag = conv_num(st,data);
		}
		else
		{
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return SCRIPT_CMD_FAILURE;
		}
	}
	else
		nd=(struct npc_data *)map_id2bl(st->oid);

	if( !nd )
		return SCRIPT_CMD_SUCCESS;
	if( flag ) //Attach
	{
		TBL_PC* sd;
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;
		nd->u.scr.rid = sd->bl.id;
	}

	npc_timerevent_start(nd, st->rid);
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(stopnpctimer)
{
	struct npc_data *nd;
	int flag = 0;

	if( script_hasdata(st,3) )
	{	//Two arguments: NPC name and attach flag.
		nd = npc_name2id(script_getstr(st, 2));
		flag = script_getnum(st,3);
	}
	else if( script_hasdata(st,2) )
	{	//Check if argument is numeric (flag) or string (npc name)
		struct script_data *data;
		data = script_getdata(st,2);
		get_val(st,data);
		if( data_isstring(data) ) //NPC name
			nd = npc_name2id(conv_str(st, data));
		else if( data_isint(data) ) //Flag
		{
			nd = (struct npc_data *)map_id2bl(st->oid);
			flag = conv_num(st,data);
		}
		else
		{
			ShowError("initnpctimer: invalid argument type #1 (needs be int or string)).\n");
			return SCRIPT_CMD_FAILURE;
		}
	}
	else
		nd=(struct npc_data *)map_id2bl(st->oid);

	if( !nd )
		return SCRIPT_CMD_SUCCESS;
	if( flag ) //Detach
		nd->u.scr.rid = 0;

	npc_timerevent_stop(nd);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(getnpctimer)
{
	struct npc_data *nd;
	TBL_PC *sd;
	int type = script_getnum(st,2);
	int val = 0;

	if( script_hasdata(st,3) )
		nd = npc_name2id(script_getstr(st,3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( !nd || nd->bl.type != BL_NPC )
	{
		script_pushint(st,0);
		ShowError("getnpctimer: Invalid NPC.\n");
		return SCRIPT_CMD_FAILURE;
	}

	switch( type )
	{
	case 0: val = npc_gettimerevent_tick(nd); break;
	case 1:
		if( nd->u.scr.rid )
		{
			sd = map_id2sd(nd->u.scr.rid);
			if( !sd )
			{
				ShowError("buildin_getnpctimer: Attached player not found!\n");
				break;
			}
			val = (sd->npc_timer_id != INVALID_TIMER);
		}
		else
			val = (nd->u.scr.timerid != INVALID_TIMER);
		break;
	case 2: val = nd->u.scr.timeramount; break;
	}

	script_pushint(st,val);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(setnpctimer)
{
	int tick;
	struct npc_data *nd;

	tick = script_getnum(st,2);
	if( script_hasdata(st,3) )
		nd = npc_name2id(script_getstr(st,3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( !nd || nd->bl.type != BL_NPC )
	{
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return SCRIPT_CMD_FAILURE;
	}

	npc_settimerevent_tick(nd,tick);
	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * attaches the player rid to the timer [Celest]
 *------------------------------------------*/
BUILDIN_FUNC(attachnpctimer)
{
	TBL_PC *sd;
	struct npc_data *nd = (struct npc_data *)map_id2bl(st->oid);

	if( !nd || nd->bl.type != BL_NPC )
	{
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return SCRIPT_CMD_FAILURE;
	}

	if( !script_nick2sd(2,sd) ){
		script_pushint(st,1);
		ShowWarning("attachnpctimer: Invalid player.\n");
		return SCRIPT_CMD_FAILURE;
	}

	nd->u.scr.rid = sd->bl.id;
	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * detaches a player rid from the timer [Celest]
 *------------------------------------------*/
BUILDIN_FUNC(detachnpctimer)
{
	struct npc_data *nd;

	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st,2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( !nd || nd->bl.type != BL_NPC )
	{
		script_pushint(st,1);
		ShowError("detachnpctimer: Invalid NPC.\n");
		return SCRIPT_CMD_FAILURE;
	}

	nd->u.scr.rid = 0;
	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * To avoid "player not attached" script errors, this function is provided,
 * it checks if there is a player attached to the current script. [Skotlex]
 * If no, returns 0, if yes, returns the account_id of the attached player.
 *------------------------------------------*/
BUILDIN_FUNC(playerattached)
{
	if(st->rid == 0 || map_id2sd(st->rid) == NULL)
		script_pushint(st,0);
	else
		script_pushint(st,st->rid);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(announce)
{
	const char *mes       = script_getstr(st,2);
	int         flag      = script_getnum(st,3);
	const char *fontColor = script_hasdata(st,4) ? script_getstr(st,4) : NULL;
	int         fontType  = script_hasdata(st,5) ? script_getnum(st,5) : FW_NORMAL; // default fontType
	int         fontSize  = script_hasdata(st,6) ? script_getnum(st,6) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,7) ? script_getnum(st,7) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontY

	if (flag&(BC_TARGET_MASK|BC_SOURCE_MASK)) // Broadcast source or broadcast region defined
	{
		send_target target;
		struct block_list *bl;

		// If bc_npc flag is set, use NPC as broadcast source
		if(flag&BC_NPC){
			bl = map_id2bl(st->oid);
		}else{
			struct map_session_data* sd;

			if( script_rid2sd(sd) )
				bl = &sd->bl;
			else
				bl = NULL;
		}
		
		if (bl == NULL)
			return SCRIPT_CMD_SUCCESS;

		switch (flag&BC_TARGET_MASK) {
			case BC_MAP:	target = ALL_SAMEMAP;	break;
			case BC_AREA:	target = AREA;			break;
			case BC_SELF:	target = SELF;			break;
			default:		target = ALL_CLIENT;	break; // BC_ALL
		}

		if (fontColor)
			clif_broadcast2(bl, mes, (int)strlen(mes)+1, strtol(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY, target);
		else
			clif_broadcast(bl, mes, (int)strlen(mes)+1, flag&BC_COLOR_MASK, target);
	}
	else
	{
		if (fontColor)
			intif_broadcast2(mes, (int)strlen(mes)+1, strtol(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY);
		else
			intif_broadcast(mes, (int)strlen(mes)+1, flag&BC_COLOR_MASK);
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
static int buildin_announce_sub(struct block_list *bl, va_list ap)
{
	char *mes       = va_arg(ap, char *);
	int   len       = va_arg(ap, int);
	int   type      = va_arg(ap, int);
	char *fontColor = va_arg(ap, char *);
	short fontType  = (short)va_arg(ap, int);
	short fontSize  = (short)va_arg(ap, int);
	short fontAlign = (short)va_arg(ap, int);
	short fontY     = (short)va_arg(ap, int);
	if (fontColor)
		clif_broadcast2(bl, mes, len, strtol(fontColor, (char **)NULL, 0), fontType, fontSize, fontAlign, fontY, SELF);
	else
		clif_broadcast(bl, mes, len, type, SELF);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mapannounce)
{
	const char *mapname   = script_getstr(st,2);
	const char *mes       = script_getstr(st,3);
	int         flag      = script_getnum(st,4);
	const char *fontColor = script_hasdata(st,5) ? script_getstr(st,5) : NULL;
	int         fontType  = script_hasdata(st,6) ? script_getnum(st,6) : FW_NORMAL; // default fontType
	int         fontSize  = script_hasdata(st,7) ? script_getnum(st,7) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,9) ? script_getnum(st,9) : 0;     // default fontY
	int16 m;

	if ((m = map_mapname2mapid(mapname)) < 0)
		return SCRIPT_CMD_SUCCESS;

	map_foreachinmap(buildin_announce_sub, m, BL_PC,
			mes, strlen(mes)+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(areaannounce)
{
	const char *mapname   = script_getstr(st,2);
	int         x0        = script_getnum(st,3);
	int         y0        = script_getnum(st,4);
	int         x1        = script_getnum(st,5);
	int         y1        = script_getnum(st,6);
	const char *mes       = script_getstr(st,7);
	int         flag      = script_getnum(st,8);
	const char *fontColor = script_hasdata(st,9) ? script_getstr(st,9) : NULL;
	int         fontType  = script_hasdata(st,10) ? script_getnum(st,10) : FW_NORMAL; // default fontType
	int         fontSize  = script_hasdata(st,11) ? script_getnum(st,11) : 12;    // default fontSize
	int         fontAlign = script_hasdata(st,12) ? script_getnum(st,12) : 0;     // default fontAlign
	int         fontY     = script_hasdata(st,13) ? script_getnum(st,13) : 0;     // default fontY
	int16 m;

	if ((m = map_mapname2mapid(mapname)) < 0)
		return SCRIPT_CMD_SUCCESS;

	map_foreachinallarea(buildin_announce_sub, m, x0, y0, x1, y1, BL_PC,
		mes, strlen(mes)+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(getusers)
{
	int flag, val = 0;
	struct map_session_data* sd;
	struct block_list* bl = NULL;

	flag = script_getnum(st,2);

	switch(flag&0x07)
	{
		case 0:
			if(flag&0x8)
			{// npc
				bl = map_id2bl(st->oid);
			}
			else if(script_rid2sd(sd))
			{// pc
				bl = &sd->bl;
			}

			if(bl)
			{
				val = map_getmapdata(bl->m)->users;
			}
			break;
		case 1:
			val = map_getusers();
			break;
		default:
			ShowWarning("buildin_getusers: Unknown type %d.\n", flag);
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,val);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * getmapguildusers("mapname",guild ID) Returns the number guild members present on a map [Reddozen]
 *------------------------------------------*/
BUILDIN_FUNC(getmapguildusers)
{
	const char *str;
	int16 m;
	int gid;
	int c=0;
	struct guild *g = NULL;
	str=script_getstr(st,2);
	gid=script_getnum(st,3);
	if ((m = map_mapname2mapid(str)) < 0) { // map id on this server (m == -1 if not in actual map-server)
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	g = guild_search(gid);

	if (g){
		unsigned short i;
		for(i = 0; i < g->max_member; i++)
		{
			if (g->member[i].sd && g->member[i].sd->bl.m == m)
				c++;
		}
	}

	script_pushint(st,c);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(getmapusers)
{
	const char *str;
	int16 m;
	str=script_getstr(st,2);
	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	script_pushint(st,map_getmapdata(m)->users);
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 *------------------------------------------*/
static int buildin_getareausers_sub(struct block_list *bl,va_list ap)
{
	int *users=va_arg(ap,int *);
	(*users)++;
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getareausers)
{
	const char *str;
	int16 m,x0,y0,x1,y1,users=0; //doubt we can have more then 32k users on
	str=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	map_foreachinallarea(buildin_getareausers_sub,
		m,x0,y0,x1,y1,BL_PC,&users);
	script_pushint(st,users);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
static int buildin_getareadropitem_sub(struct block_list *bl,va_list ap)
{
	unsigned short nameid = (unsigned short)va_arg(ap, int);
	unsigned short *amount = (unsigned short *)va_arg(ap, int *);
	struct flooritem_data *drop=(struct flooritem_data *)bl;

	if(drop->item.nameid==nameid)
		(*amount)+=drop->item.amount;

	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(getareadropitem)
{
	const char *str;
	int16 m,x0,y0,x1,y1;
	unsigned short nameid, amount = 0;
	struct script_data *data;

	str=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);

	data=script_getdata(st,7);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		nameid=UNKNOWN_ITEM_ID;
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,data);

	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	map_foreachinallarea(buildin_getareadropitem_sub,
		m,x0,y0,x1,y1,BL_ITEM,nameid,&amount);
	script_pushint(st,amount);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(enablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(disablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(hideoffnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,2);
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(hideonnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,4);
	return SCRIPT_CMD_SUCCESS;
}

/* Starts a status effect on the target unit or on the attached player.
 *
 * sc_start  <effect_id>,<duration>,<val1>{,<rate>,<flag>,{<unit_id>}};
 * sc_start2 <effect_id>,<duration>,<val1>,<val2>{,<rate,<flag>,{<unit_id>}};
 * sc_start4 <effect_id>,<duration>,<val1>,<val2>,<val3>,<val4>{,<rate,<flag>,{<unit_id>}};
 * <flag>: enum e_status_change_start_flags
 */
BUILDIN_FUNC(sc_start)
{
	TBL_NPC * nd = map_id2nd(st->oid);
	struct block_list* bl;
	enum sc_type type;
	int tick, val1, val2, val3, val4=0, rate, flag;
	char start_type;
	const char* command = script_getfuncname(st);

	if(strstr(command, "4"))
		start_type = 4;
	else if(strstr(command, "2"))
		start_type = 2;
	else
		start_type = 1;

	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);

	//If from NPC we make default flag 1 to be unavoidable
	if(nd && nd->bl.id == fake_nd->bl.id)
		flag = script_hasdata(st,5+start_type)?script_getnum(st,5+start_type):SCSTART_NOTICKDEF;
	else
		flag = script_hasdata(st,5+start_type)?script_getnum(st,5+start_type):SCSTART_NOAVOID;

	rate = script_hasdata(st,4+start_type)?min(script_getnum(st,4+start_type),10000):10000;

	if(script_hasdata(st,(6+start_type)))
		bl = map_id2bl(script_getnum(st,(6+start_type)));
	else
		bl = map_id2bl(st->rid);

	if(tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_sc2skill(type) != 0)
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill_get_time(status_sc2skill(type), val1);
	}

	if(potion_flag == 1 && potion_target) { //skill.c set the flags before running the script, this is a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
		val4 = 1;// Mark that this was a thrown sc_effect
	}

	if(!bl)
		return SCRIPT_CMD_SUCCESS;

	switch(start_type) {
		case 1:
			status_change_start(bl, bl, type, rate, val1, 0, 0, val4, tick, flag);
			break;
		case 2:
			val2 = script_getnum(st,5);
			status_change_start(bl, bl, type, rate, val1, val2, 0, val4, tick, flag);
			break;
		case 4:
			val2 = script_getnum(st,5);
			val3 = script_getnum(st,6);
			val4 = script_getnum(st,7);
			status_change_start(bl, bl, type, rate, val1, val2, val3, val4, tick, flag);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Ends one or all status effects on the target unit or on the attached player.
///
/// sc_end <effect_id>{,<unit_id>};
BUILDIN_FUNC(sc_end)
{
	struct block_list* bl;
	int type;

	type = script_getnum(st, 2);
	if (script_hasdata(st, 3))
		bl = map_id2bl(script_getnum(st, 3));
	else
		bl = map_id2bl(st->rid);

	if (potion_flag == 1 && potion_target) //##TODO how does this work [FlavioJS]
		bl = map_id2bl(potion_target);

	if (!bl)
		return SCRIPT_CMD_SUCCESS;

	if (type >= 0 && type < SC_MAX) {
		struct status_change *sc = status_get_sc(bl);
		struct status_change_entry *sce = sc ? sc->data[type] : NULL;

		if (!sce)
			return SCRIPT_CMD_SUCCESS;

		switch (type) {
			case SC_WEIGHT50:
			case SC_WEIGHT90:
			case SC_NOCHAT:
			case SC_PUSH_CART:
			case SC_ALL_RIDING:
			case SC_STYLE_CHANGE:
			case SC_MONSTER_TRANSFORM:
			case SC_ACTIVE_MONSTER_TRANSFORM:
			case SC_MTF_ASPD:
			case SC_MTF_RANGEATK:
			case SC_MTF_MATK:
			case SC_MTF_MLEATKED:
			case SC_MTF_CRIDAMAGE:
			case SC_MTF_ASPD2:
			case SC_MTF_RANGEATK2:
			case SC_MTF_MATK2:
			case SC_MTF_MHP:
			case SC_MTF_MSP:
			case SC_MTF_PUMPKIN:
			case SC_MTF_HITFLEE:
			case SC_ATTHASTE_CASH:
			case SC_REUSE_LIMIT_A:				case SC_REUSE_LIMIT_B:			case SC_REUSE_LIMIT_C:
			case SC_REUSE_LIMIT_D:				case SC_REUSE_LIMIT_E:			case SC_REUSE_LIMIT_F:
			case SC_REUSE_LIMIT_G:				case SC_REUSE_LIMIT_H:			case SC_REUSE_LIMIT_MTF:
			case SC_REUSE_LIMIT_ASPD_POTION:	case SC_REUSE_MILLENNIUMSHIELD:	case SC_REUSE_CRUSHSTRIKE:
			case SC_REUSE_STORMBLAST:			case SC_ALL_RIDING_REUSE_LIMIT:	case SC_REUSE_REFRESH:
			case SC_REUSE_LIMIT_ECL:			case SC_REUSE_LIMIT_RECALL:
				return SCRIPT_CMD_SUCCESS;
			default:
				break;
		}

		//This should help status_change_end force disabling the SC in case it has no limit.
		sce->val1 = sce->val2 = sce->val3 = sce->val4 = 0;
		status_change_end(bl, (sc_type)type, INVALID_TIMER);
	} else
		status_change_clear(bl, 3); // remove all effects

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Ends all status effects from any learned skill on the attached player.
 * if <job_id> was given it will end the effect of that class for the attached player
 * sc_end_class {<char_id>{,<job_id>}};
 */
BUILDIN_FUNC(sc_end_class)
{
	struct map_session_data *sd;
	uint16 skill_id;
	int class_;

	if (!script_charid2sd(2, sd))
		return SCRIPT_CMD_FAILURE;

	if (script_hasdata(st, 3))
		class_ = script_getnum(st, 3);
	else
		class_ = sd->status.class_;

	if (!pcdb_checkid(class_)) {
		ShowError("buildin_sc_end_class: Invalid job ID '%d' given.\n", script_getnum(st, 3));
		return SCRIPT_CMD_FAILURE;
	}

	for (int i = 0; i < MAX_SKILL_TREE && (skill_id = skill_tree[pc_class2idx(class_)][i].skill_id) > 0; i++) {
		enum sc_type sc = status_skill2sc(skill_id);

		if (sc > SC_COMMON_MAX && sd->sc.data[sc])
			status_change_end(&sd->bl, sc, INVALID_TIMER);
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * @FIXME atm will return reduced tick, 0 immune, 1 no tick
 *------------------------------------------*/
BUILDIN_FUNC(getscrate)
{
	struct block_list *bl;
	int type,rate;

	type=script_getnum(st,2);
	rate=script_getnum(st,3);
	if( script_hasdata(st,4) ) //get for the bl assigned
		bl = map_id2bl(script_getnum(st,4));
	else
		bl = map_id2bl(st->rid);

	if (bl)
		rate = status_get_sc_def(NULL,bl, (sc_type)type, 10000, 10000, SCSTART_NONE);

	script_pushint(st,rate);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getstatus(<effect type>{,<type>{,<char_id>}});
 **/
BUILDIN_FUNC(getstatus)
{
	int id, type;
	struct map_session_data* sd;

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	id = script_getnum(st, 2);
	type = script_hasdata(st, 3) ? script_getnum(st, 3) : 0;

	if( id <= SC_NONE || id >= SC_MAX )
	{// invalid status type given
		ShowWarning("script.cpp:getstatus: Invalid status type given (%d).\n", id);
		return SCRIPT_CMD_SUCCESS;
	}

	if( sd->sc.count == 0 || !sd->sc.data[id] )
	{// no status is active
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch( type )
	{
		case 1:	 script_pushint(st, sd->sc.data[id]->val1);	break;
		case 2:  script_pushint(st, sd->sc.data[id]->val2);	break;
		case 3:  script_pushint(st, sd->sc.data[id]->val3);	break;
		case 4:  script_pushint(st, sd->sc.data[id]->val4);	break;
		case 5:
			{
				struct TimerData* timer = (struct TimerData*)get_timer(sd->sc.data[id]->timer);

				if( timer )
				{// return the amount of time remaining
					script_pushint(st, timer->tick - gettick());
				} else {
					script_pushint(st, -1);
				}
			}
			break;
		default: script_pushint(st, 1); break;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(debugmes)
{
	const char *str;
	str=script_getstr(st,2);
	ShowDebug("script debug : %d %d : %s\n",st->rid,st->oid,str);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *------------------------------------------*/
BUILDIN_FUNC(catchpet)
{
	int pet_id;
	TBL_PC *sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	pet_id= script_getnum(st,2);

	pet_catch_process1(sd,pet_id);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * [orn]
 *------------------------------------------*/
BUILDIN_FUNC(homunculus_evolution)
{
	TBL_PC *sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	if(hom_is_active(sd->hd))
	{
		if (sd->hd->homunculus.intimacy >= battle_config.homunculus_evo_intimacy_need)
			hom_evolution(sd->hd);
		else
			clif_emotion(&sd->hd->bl, ET_SWEAT);
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Checks for vaporized morph state
 * and deletes ITEMID_STRANGE_EMBRYO.
 *------------------------------------------*/
BUILDIN_FUNC(homunculus_mutate)
{
	int homun_id;
	TBL_PC *sd;

	if( !script_rid2sd(sd) || sd->hd == NULL )
		return SCRIPT_CMD_SUCCESS;

	if(script_hasdata(st,2))
		homun_id = script_getnum(st,2);
	else
		homun_id = 6048 + (rnd() % 4);

	if( sd->hd->homunculus.vaporize == HOM_ST_MORPH ) {
		int m_class = hom_class2mapid(sd->hd->homunculus.class_);
		int m_id = hom_class2mapid(homun_id);
		short i = pc_search_inventory(sd, ITEMID_STRANGE_EMBRYO);

		if ( m_class != -1 && m_id != -1 && m_class&HOM_EVO && m_id&HOM_S && sd->hd->homunculus.level >= 99 && i >= 0 ) {
			sd->hd->homunculus.vaporize = HOM_ST_REST; // Remove morph state.
			hom_call(sd); // Respawn homunculus.
			hom_mutate(sd->hd, homun_id);
			pc_delitem(sd, i, 1, 0, 0, LOG_TYPE_SCRIPT);
			script_pushint(st, 1);
			return SCRIPT_CMD_SUCCESS;
		} else
			clif_emotion(&sd->bl, ET_SWEAT);
	} else
		clif_emotion(&sd->bl, ET_SWEAT);

	script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Puts homunculus into morph state
 * and gives ITEMID_STRANGE_EMBRYO.
 *------------------------------------------*/
BUILDIN_FUNC(morphembryo)
{
	struct item item_tmp;
	TBL_PC *sd;

	if( !script_rid2sd(sd) || sd->hd == NULL )
		return SCRIPT_CMD_SUCCESS;

	if( hom_is_active(sd->hd) ) {
		int m_class = hom_class2mapid(sd->hd->homunculus.class_);

		if ( m_class != -1 && m_class&HOM_EVO && sd->hd->homunculus.level >= 99 ) {
			char i;
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = ITEMID_STRANGE_EMBRYO;
			item_tmp.identify = 1;

			if( (i = pc_additem(sd, &item_tmp, 1, LOG_TYPE_SCRIPT)) ) {
				clif_additem(sd, 0, 0, i);
				clif_emotion(&sd->bl, ET_SWEAT); // Fail to avoid item drop exploit.
			} else {
				hom_vaporize(sd, HOM_ST_MORPH);
				script_pushint(st, 1);
				return SCRIPT_CMD_SUCCESS;
			}
		} else
			clif_emotion(&sd->hd->bl, ET_SWEAT);
	} else
		clif_emotion(&sd->bl, ET_SWEAT);

	script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

// [Zephyrus]
BUILDIN_FUNC(homunculus_shuffle)
{
	TBL_PC *sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	if(hom_is_active(sd->hd))
		hom_shuffle(sd->hd);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Check for homunculus state.
 * Return: -1 = No homunculus
 *          0 = Homunculus is active
 *          1 = Homunculus is vaporized (rest)
 *          2 = Homunculus is in morph state
 *------------------------------------------*/
BUILDIN_FUNC(checkhomcall)
{
	TBL_PC *sd;
	TBL_HOM *hd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	hd = sd->hd;

	if( !hd )
		script_pushint(st, -1);
	else
		script_pushint(st, hd->homunculus.vaporize);

	return SCRIPT_CMD_SUCCESS;
}

//These two functions bring the eA MAPID_* class functionality to scripts.
BUILDIN_FUNC(eaclass)
{
	int class_;
	if( script_hasdata(st,2) )
		class_ = script_getnum(st,2);
	else {
		TBL_PC *sd;

		if (!script_charid2sd(3,sd)) {
			script_pushint(st,-1);
			return SCRIPT_CMD_SUCCESS;
		}
		class_ = sd->status.class_;
	}
	script_pushint(st,pc_jobid2mapid(class_));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(roclass)
{
	int class_ =script_getnum(st,2);
	int sex;
	if( script_hasdata(st,3) )
		sex = script_getnum(st,3);
	else {
		TBL_PC *sd;
		if (st->rid && script_rid2sd(sd))
			sex = sd->status.sex;
		else
			sex = SEX_MALE; //Just use male when not found.
	}
	script_pushint(st,pc_mapid2jobid(class_, sex));
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Tells client to open a hatching window, used for pet incubator
 *------------------------------------------*/
BUILDIN_FUNC(birthpet)
{
	TBL_PC *sd;
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	if( sd->status.pet_id )
	{// do not send egg list, when you already have a pet
		return SCRIPT_CMD_SUCCESS;
	}

	clif_sendegg(sd);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * resetlvl <action type>{,<char_id>};
 * @param action_type:
 *	1 : make like after rebirth
 *	2 : blvl,jlvl=1, skillpoint=0
 * 	3 : don't reset skill, blvl=1
 *	4 : jlvl=0
 * @author AppleGirl
 **/
BUILDIN_FUNC(resetlvl)
{
	TBL_PC *sd;

	int type=script_getnum(st,2);

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	pc_resetlvl(sd,type);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Reset a player status point
 * resetstatus({<char_id>});
 **/
BUILDIN_FUNC(resetstatus)
{
	TBL_PC *sd;
	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	pc_resetstate(sd);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Reset player's skill
 * resetskill({<char_id>});
 **/
BUILDIN_FUNC(resetskill)
{
	TBL_PC *sd;
	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	pc_resetskill(sd,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Counts total amount of skill points.
 * skillpointcount({<char_id>})
 **/
BUILDIN_FUNC(skillpointcount)
{
	TBL_PC *sd;
	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	script_pushint(st,sd->status.skill_point + pc_resetskill(sd,2));
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(changebase)
{
	TBL_PC *sd=NULL;
	int vclass;

	if( !script_mapid2sd(3,sd) )
		return SCRIPT_CMD_SUCCESS;

	vclass = script_getnum(st,2);
	if(vclass == JOB_WEDDING)
	{
		if (!battle_config.wedding_modifydisplay || //Do not show the wedding sprites
			sd->class_&JOBL_BABY //Baby classes screw up when showing wedding sprites. [Skotlex] They don't seem to anymore.
			)
		return SCRIPT_CMD_SUCCESS;
	}

	if(!sd->disguise && vclass != sd->vd.class_) {
		status_set_viewdata(&sd->bl, vclass);
		//Updated client view. Base, Weapon and Cloth Colors.
		clif_changelook(&sd->bl,LOOK_BASE,sd->vd.class_);
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		if (sd->vd.cloth_color)
			clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->vd.cloth_color);
		if (sd->vd.body_style)
			clif_changelook(&sd->bl,LOOK_BODY2,sd->vd.body_style);
		clif_skillinfoblock(sd);
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Change account sex and unequip all item and request for a changesex to char-serv
 * changesex({<char_id>});
 */
BUILDIN_FUNC(changesex)
{
	int i;
	TBL_PC *sd = NULL;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	pc_resetskill(sd,4);
	// to avoid any problem with equipment and invalid sex, equipment is unequiped.
	for(i = 0; i < EQI_MAX; i++) {
		if (sd->equip_index[i] >= 0)
			pc_unequipitem(sd, sd->equip_index[i], 3);
	}

	chrif_changesex(sd, true);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Change character's sex and unequip all item and request for a changesex to char-serv
 * changecharsex({<char_id>});
 */
BUILDIN_FUNC(changecharsex)
{
#if PACKETVER >= 20141016
	int i;
	TBL_PC *sd = NULL;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	pc_resetskill(sd,4);
	// to avoid any problem with equipment and invalid sex, equipment is unequiped.
	for (i = 0; i < EQI_MAX; i++) {
		if (sd->equip_index[i] >= 0)
			pc_unequipitem(sd, sd->equip_index[i], 3);
	}

	chrif_changesex(sd, false);
	return SCRIPT_CMD_SUCCESS;
#else
	return SCRIPT_CMD_FAILURE;
#endif
}

/*==========================================
 * Works like 'announce' but outputs in the common chat window
 *------------------------------------------*/
BUILDIN_FUNC(globalmes)
{
	struct block_list *bl = map_id2bl(st->oid);
	struct npc_data *nd = (struct npc_data *)bl;
	const char *name=NULL,*mes;

	mes=script_getstr(st,2);
	if(mes==NULL)
		return SCRIPT_CMD_SUCCESS;

	if(script_hasdata(st,3)){	//  npc name to display
		name=script_getstr(st,3);
	} else {
		name=nd->name; //use current npc name
	}

	npc_globalmessage(name,mes);	// broadcast  to all players connected
	return SCRIPT_CMD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// NPC waiting room (chat room)
//

/// Creates a waiting room (chat room) for this npc.
///
/// waitingroom "<title>",<limit>{,"<event>"{,<trigger>{,<zeny>{,<minlvl>{,<maxlvl>}}}}};
BUILDIN_FUNC(waitingroom)
{
	struct npc_data* nd;
	const char* title = script_getstr(st, 2);
	int limit = script_getnum(st, 3);
	const char* ev = script_hasdata(st,4) ? script_getstr(st,4) : "";
	int trigger =  script_hasdata(st,5) ? script_getnum(st,5) : limit;
	int zeny =  script_hasdata(st,6) ? script_getnum(st,6) : 0;
	int minLvl =  script_hasdata(st,7) ? script_getnum(st,7) : 1;
	int maxLvl =  script_hasdata(st,8) ? script_getnum(st,8) : MAX_LEVEL;

	nd = (struct npc_data *)map_id2bl(st->oid);
	if( nd != NULL )
		chat_createnpcchat(nd, title, limit, 1, trigger, ev, zeny, minLvl, maxLvl);

	return SCRIPT_CMD_SUCCESS;
}

/// Removes the waiting room of the current or target npc.
///
/// delwaitingroom "<npc_name>";
/// delwaitingroom;
BUILDIN_FUNC(delwaitingroom)
{
	struct npc_data* nd;
	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st, 2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	if( nd != NULL )
		chat_deletenpcchat(nd);
	return SCRIPT_CMD_SUCCESS;
}

/// Kick the specified player from the waiting room of the target npc.
///
/// waitingroomkick "<npc_name>", <kickusername>;
BUILDIN_FUNC(waitingroomkick)
{
	struct npc_data* nd;
	struct chat_data* cd;
	const char* kickusername;
	
	nd = npc_name2id(script_getstr(st,2));
	kickusername = script_getstr(st,3);

	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL )
		chat_npckickchat(cd, kickusername);
	return SCRIPT_CMD_SUCCESS;
}

/// Get Users in waiting room and stores gids in .@waitingroom_users[]
/// Num users stored in .@waitingroom_usercount
///
/// getwaitingroomusers "<npc_name>";
BUILDIN_FUNC(getwaitingroomusers)
{
	struct npc_data* nd;
	struct chat_data* cd;

	int i, j=0;

	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st, 2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL ) {
		for(i = 0; i < cd->users; ++i) {
			setd_sub(st, NULL, ".@waitingroom_users", j, (void *)__64BPRTSIZE(cd->usersd[i]->status.account_id), NULL);
			j++;
		}
		setd_sub(st, NULL, ".@waitingroom_usercount", 0, (void *)__64BPRTSIZE(j), NULL);
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Kicks all the players from the waiting room of the current or target npc.
///
/// kickwaitingroomall "<npc_name>";
/// kickwaitingroomall;
BUILDIN_FUNC(waitingroomkickall)
{
	struct npc_data* nd;
	struct chat_data* cd;

	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st,2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL )
		chat_npckickall(cd);
	return SCRIPT_CMD_SUCCESS;
}

/// Enables the waiting room event of the current or target npc.
///
/// enablewaitingroomevent "<npc_name>";
/// enablewaitingroomevent;
BUILDIN_FUNC(enablewaitingroomevent)
{
	struct npc_data* nd;
	struct chat_data* cd;

	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st, 2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL )
		chat_enableevent(cd);
	return SCRIPT_CMD_SUCCESS;
}

/// Disables the waiting room event of the current or target npc.
///
/// disablewaitingroomevent "<npc_name>";
/// disablewaitingroomevent;
BUILDIN_FUNC(disablewaitingroomevent)
{
	struct npc_data *nd;
	struct chat_data *cd;

	if( script_hasdata(st,2) )
		nd = npc_name2id(script_getstr(st, 2));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( nd != NULL && (cd=(struct chat_data *)map_id2bl(nd->chat_id)) != NULL )
		chat_disableevent(cd);
	return SCRIPT_CMD_SUCCESS;
}

/// Returns info on the waiting room of the current or target npc.
/// Returns -1 if the type unknown
/// <type>=0 : current number of users
/// <type>=1 : maximum number of users allowed
/// <type>=2 : the number of users that trigger the event
/// <type>=3 : if the trigger is disabled
/// <type>=4 : the title of the waiting room
/// <type>=5 : the password of the waiting room
/// <type>=16 : the name of the waiting room event
/// <type>=32 : if the waiting room is full
/// <type>=33 : if there are enough users to trigger the event
///
/// getwaitingroomstate(<type>,"<npc_name>") -> <info>
/// getwaitingroomstate(<type>) -> <info>
BUILDIN_FUNC(getwaitingroomstate)
{
	struct npc_data *nd;
	struct chat_data *cd;
	int type;

	type = script_getnum(st,2);
	if( script_hasdata(st,3) )
		nd = npc_name2id(script_getstr(st, 3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( nd == NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id)) == NULL )
	{
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type)
	{
	case 0:  script_pushint(st, cd->users); break;
	case 1:  script_pushint(st, cd->limit); break;
	case 2:  script_pushint(st, cd->trigger&0x7f); break;
	case 3:  script_pushint(st, ((cd->trigger&0x80)!=0)); break;
	case 4:  script_pushstrcopy(st, cd->title); break;
	case 5:  script_pushstrcopy(st, cd->pass); break;
	case 16: script_pushstrcopy(st, cd->npc_event);break;
	case 32: script_pushint(st, (cd->users >= cd->limit)); break;
	case 33: script_pushint(st, (cd->users >= cd->trigger)); break;
	default: script_pushint(st, -1); break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Warps the trigger or target amount of players to the target map and position.
/// Players are automatically removed from the waiting room.
/// Those waiting the longest will get warped first.
/// The target map can be "Random" for a random position in the current map,
/// and "SavePoint" for the savepoint map+position.
/// The map flag noteleport of the current map is only considered when teleporting to the savepoint.
///
/// The id's of the teleported players are put into the array $@warpwaitingpc[]
/// The total number of teleported players is put into $@warpwaitingpcnum
///
/// warpwaitingpc "<map name>",<x>,<y>,<number of players>;
/// warpwaitingpc "<map name>",<x>,<y>;
BUILDIN_FUNC(warpwaitingpc)
{
	int x;
	int y;
	int i;
	int n;
	const char* map_name;
	struct npc_data* nd;
	struct chat_data* cd;

	nd = (struct npc_data *)map_id2bl(st->oid);
	if( nd == NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id)) == NULL )
		return SCRIPT_CMD_SUCCESS;

	map_name = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	n = cd->trigger&0x7f;

	if( script_hasdata(st,5) )
		n = script_getnum(st,5);

	for( i = 0; i < n && cd->users > 0; i++ )
	{
		TBL_PC* sd = cd->usersd[0];

		if( strcmp(map_name,"SavePoint") == 0 && map_getmapflag(sd->bl.m, MF_NOTELEPORT) )
		{// can't teleport on this map
			break;
		}

		if( cd->zeny )
		{// fee set
			if( (uint32)sd->status.zeny < cd->zeny )
			{// no zeny to cover set fee
				break;
			}
			pc_payzeny(sd, cd->zeny, LOG_TYPE_NPC, NULL);
		}

		mapreg_setreg(reference_uid(add_str("$@warpwaitingpc"), i), sd->bl.id);

		if( strcmp(map_name,"Random") == 0 )
			pc_randomwarp(sd,CLR_TELEPORT);
		else if( strcmp(map_name,"SavePoint") == 0 )
			pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT);
		else
			pc_setpos(sd, mapindex_name2id(map_name), x, y, CLR_OUTSIGHT);
	}
	mapreg_setreg(add_str("$@warpwaitingpcnum"), i);
	return SCRIPT_CMD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////
// ...
//

/// Detaches a character from a script.
///
/// @param st Script state to detach the character from.
static void script_detach_rid(struct script_state* st)
{
	if(st->rid)
	{
		script_detach_state(st, false);
		st->rid = 0;
	}
}

/*=========================================================================
 * Attaches a set of RIDs to the current script. [digitalhamster]
 * addrid(<type>{,<flag>{,<parameters>}});
 * <type>:
 *	0 : All players in the server.
 *	1 : All players in the map of the invoking player, or the invoking NPC if no player is attached.
 *	2 : Party members of a specified party ID.
 *	    [ Parameters: <party id> ]
 *	3 : Guild members of a specified guild ID.
 *	    [ Parameters: <guild id> ]
 *	4 : All players in a specified area of the map of the invoking player (or NPC).
 *	    [ Parameters: <x0>,<y0>,<x1>,<y1> ]
 *	5 : All players in the map.
 *	    [ Parameters: "<map name>" ]
 *	Account ID: The specified account ID.
 * <flag>:
 *	0 : Players are always attached. (default)
 *	1 : Players currently running another script will not be attached.
 *-------------------------------------------------------------------------*/
static int buildin_addrid_sub(struct block_list *bl,va_list ap)
{
	int forceflag;
	struct map_session_data *sd = (TBL_PC *)bl;
	struct script_state* st;

	st = va_arg(ap,struct script_state*);
	forceflag = va_arg(ap,int);

	if(!forceflag || !sd->st)
		if(sd->status.account_id != st->rid)
			run_script(st->script,st->pos,sd->status.account_id,st->oid);
	return 0;
}

BUILDIN_FUNC(addrid)
{
	struct s_mapiterator* iter;
	struct block_list *bl;
	TBL_PC *sd;

	if(st->rid < 1) {
		st->state = END;
		bl = map_id2bl(st->oid);
	} else
		bl = map_id2bl(st->rid); //if run without rid it'd error,also oid if npc, else rid for map
	iter = mapit_getallusers();

	switch(script_getnum(st,2)) {
		case 0:
			for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
				if(!script_getnum(st,3) || !sd->st)
					if(sd->status.account_id != st->rid) //attached player already runs.
						run_script(st->script,st->pos,sd->status.account_id,st->oid);
			}
			break;
		case 1:
			for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
				if(!script_getnum(st,3) || !sd->st)
					if((sd->bl.m == bl->m) && (sd->status.account_id != st->rid))
						run_script(st->script,st->pos,sd->status.account_id,st->oid);
			}
			break;
		case 2:
			if(script_getnum(st,4) == 0) {
				script_pushint(st,0);
				return SCRIPT_CMD_SUCCESS;
			}
			for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
				if(!script_getnum(st,3) || !sd->st)
					if((sd->status.account_id != st->rid) && (sd->status.party_id == script_getnum(st,4))) //attached player already runs.
						run_script(st->script,st->pos,sd->status.account_id,st->oid);
			}
			break;
		case 3:
			if(script_getnum(st,4) == 0) {
				script_pushint(st,0);
				return SCRIPT_CMD_SUCCESS;
			}
			for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
				if(!script_getnum(st,3) || !sd->st)
					if((sd->status.account_id != st->rid) && (sd->status.guild_id == script_getnum(st,4))) //attached player already runs.
						run_script(st->script,st->pos,sd->status.account_id,st->oid);
			}
			break;
		case 4:
			map_foreachinallarea(buildin_addrid_sub,
			bl->m,script_getnum(st,4),script_getnum(st,5),script_getnum(st,6),script_getnum(st,7),BL_PC,
			st,script_getnum(st,3));//4-x0 , 5-y0 , 6-x1, 7-y1
			break;
		case 5:
			if (script_getstr(st, 4) == NULL) {
				script_pushint(st, 0);
				return SCRIPT_CMD_FAILURE;
			}
			if (map_mapname2mapid(script_getstr(st, 4)) < 0) {
				script_pushint(st, 0);
				return SCRIPT_CMD_FAILURE;
			}
			map_foreachinmap(buildin_addrid_sub, map_mapname2mapid(script_getstr(st, 4)), BL_PC, st, script_getnum(st, 3));
			break;
		default:
			if((map_id2sd(script_getnum(st,2))) == NULL) { // Player not found.
				script_pushint(st,0);
				return SCRIPT_CMD_SUCCESS;
			}
			if(!script_getnum(st,3) || !map_id2sd(script_getnum(st,2))->st) {
				run_script(st->script,st->pos,script_getnum(st,2),st->oid);
				script_pushint(st,1);
			}
			return SCRIPT_CMD_SUCCESS;
	}
	mapit_free(iter);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Attach sd char id to script and detach current one if any
 *------------------------------------------*/
BUILDIN_FUNC(attachrid)
{
	int rid = script_getnum(st,2);
	bool force;

	if( script_hasdata(st,3) ){
		force = script_getnum(st,3) != 0;
	}else{
		force = true;
	}

	struct map_session_data* sd = map_id2sd(rid);

	if( sd != NULL && ( !sd->npc_id || force ) ){
		script_detach_rid(st);

		st->rid = rid;
		script_attach_state(st);
		script_pushint(st,true);
	}else{
		script_pushint(st,false);
	}

	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Detach script to rid
 *------------------------------------------*/
BUILDIN_FUNC(detachrid)
{
	script_detach_rid(st);
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Chk if account connected, (and charid from account if specified)
 *------------------------------------------*/
BUILDIN_FUNC(isloggedin)
{
	TBL_PC* sd = map_id2sd(script_getnum(st,2));
	if (script_hasdata(st,3) && sd &&
		sd->status.char_id != script_getnum(st,3))
		sd = NULL;
	push_val(st->stack,C_INT,sd!=NULL);
	return SCRIPT_CMD_SUCCESS;
}


/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(setmapflagnosave)
{
	int16 m,x,y;
	unsigned short mapindex;
	const char *str,*str2;
	union u_mapflag_args args = {};

	str=script_getstr(st,2);
	str2=script_getstr(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);
	m = map_mapname2mapid(str);
	mapindex = mapindex_name2id(str2);

	if(m < 0) {
		ShowWarning("buildin_setmapflagnosave: Invalid map name %s.\n", str);
		return SCRIPT_CMD_FAILURE;
	}

	args.nosave.map = mapindex;
	args.nosave.x = x;
	args.nosave.y = y;

	map_setmapflag_sub(m, MF_NOSAVE, true, &args);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getmapflag)
{
	int16 m;
	int mf;
	const char *str;

	str=script_getstr(st,2);

	m = map_mapname2mapid(str);
	if (m < 0) {
		ShowWarning("buildin_getmapflag: Invalid map name %s.\n", str);
		return SCRIPT_CMD_FAILURE;
	}

	mf = script_getnum(st, 3);

	if( mf < MF_MIN || mf > MF_MAX ){
		ShowError( "buildin_getmapflag: Unsupported mapflag '%d'.\n", mf );
		return SCRIPT_CMD_FAILURE;
	}

	union u_mapflag_args args = {};

	if (mf == MF_SKILL_DAMAGE && !script_hasdata(st, 4))
		args.flag_val = SKILLDMG_MAX;
	else
		FETCH(4, args.flag_val);

	script_pushint(st, map_getmapflag_sub(m, static_cast<e_mapflag>(mf), &args));

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setmapflag)
{
	int16 m;
	int mf;
	const char *str;

	str = script_getstr(st,2);

	m = map_mapname2mapid(str);
	if (m < 0) {
		ShowWarning("buildin_setmapflag: Invalid map name %s.\n", str);
		return SCRIPT_CMD_FAILURE;
	}

	mf = script_getnum(st, 3);

	if( mf < MF_MIN || mf > MF_MAX ){
		ShowError( "buildin_setmapflag: Unsupported mapflag '%d'.\n", mf );
		return SCRIPT_CMD_FAILURE;
	}

	union u_mapflag_args args = {};

	switch(mf) {
		case MF_SKILL_DAMAGE:
			if (script_hasdata(st, 4) && script_hasdata(st, 5))
				args.skill_damage.rate[script_getnum(st, 5)] = script_getnum(st, 4);
			else {
				ShowWarning("buildin_setmapflag: Unable to set skill_damage mapflag as flag data is missing.\n");
				return SCRIPT_CMD_FAILURE;
			}
			break;
		case MF_NOSAVE: // Assume setting "SavePoint"
			args.nosave.map = 0;
			args.nosave.x = -1;
			args.nosave.y = -1;
			break;
		case MF_PVP_NIGHTMAREDROP: // Assume setting standard drops
			args.nightmaredrop.drop_id = -1;
			args.nightmaredrop.drop_per = 300;
			args.nightmaredrop.drop_type = NMDT_EQUIP;
			break;
		default:
			FETCH(4, args.flag_val);
			break;
	}

	map_setmapflag_sub(m, static_cast<e_mapflag>(mf), true, &args);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(removemapflag)
{
	int16 m;
	int mf;
	const char *str;
	union u_mapflag_args args = {};

	str = script_getstr(st, 2);
	m = map_mapname2mapid(str);

	if (m < 0) {
		ShowWarning("buildin_removemapflag: Invalid map name %s.\n", str);
		return SCRIPT_CMD_FAILURE;
	}

	mf = script_getnum(st, 3);

	if( mf < MF_MIN || mf > MF_MAX ){
		ShowError( "buildin_removemapflag: Unsupported mapflag '%d'.\n", mf );
		return SCRIPT_CMD_FAILURE;
	}

	FETCH(4, args.flag_val);

	map_setmapflag_sub(m, static_cast<e_mapflag>(mf), false, &args);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(pvpon)
{
	int16 m;
	const char *str = script_getstr(st,2);

	m = map_mapname2mapid(str);

	if (m < 0 || map_getmapflag(m, MF_PVP))
		return SCRIPT_CMD_FAILURE;

	map_setmapflag(m, MF_PVP, true);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(pvpoff)
{
	int16 m;
	const char *str = script_getstr(st,2);

	m = map_mapname2mapid(str);

	if(m < 0 || !map_getmapflag(m, MF_PVP))
		return SCRIPT_CMD_FAILURE;

	map_setmapflag(m, MF_PVP, false);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(gvgon)
{
	int16 m;
	const char *str = script_getstr(st,2);

	m = map_mapname2mapid(str);

	if (m < 0 || map_getmapflag(m, MF_GVG))
		return SCRIPT_CMD_FAILURE;

	map_setmapflag(m, MF_GVG, true);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(gvgoff)
{
	int16 m;
	const char *str = script_getstr(st,2);

	m = map_mapname2mapid(str);

	if (m < 0 || !map_getmapflag(m, MF_GVG))
		return SCRIPT_CMD_FAILURE;
		map_setmapflag(m, MF_GVG, false);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(gvgon3)
{
	int16 m;
	const char *str = script_getstr(st,2);

	m = map_mapname2mapid(str);

	if (m < 0 || map_getmapflag(m, MF_GVG_TE))
		return SCRIPT_CMD_FAILURE;

	map_setmapflag(m, MF_GVG_TE, true);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(gvgoff3)
{
	int16 m;
	const char *str = script_getstr(st,2);

	m = map_mapname2mapid(str);

	if (m < 0 || !map_getmapflag(m, MF_GVG_TE))
		return SCRIPT_CMD_FAILURE;

	map_setmapflag(m, MF_GVG_TE, false);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Shows an emotion on top of a NPC by default or the given GID
 * emotion <emotion ID>{,<target ID>};
 */
BUILDIN_FUNC(emotion)
{
	struct block_list *bl = NULL;
	int type = script_getnum(st,2);

	if (type < ET_SURPRISE || type >= ET_MAX) {
		ShowWarning("buildin_emotion: Unknown emotion %d (min=%d, max=%d).\n", type, ET_SURPRISE, (ET_MAX-1));
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 3) && !script_rid2bl(3, bl)) {
		ShowWarning("buildin_emotion: Unknown game ID supplied %d.\n", script_getnum(st, 3));
		return SCRIPT_CMD_FAILURE;
	}
	if (!bl)
		bl = map_id2bl(st->oid);

	clif_emotion(bl, type);
	return SCRIPT_CMD_SUCCESS;
}


static int buildin_maprespawnguildid_sub_pc(struct map_session_data* sd, va_list ap)
{
	int16 m=va_arg(ap,int);
	int g_id=va_arg(ap,int);
	int flag=va_arg(ap,int);

	if(!sd || sd->bl.m != m)
		return 0;
	if(
		(sd->status.guild_id == g_id && flag&1) || //Warp out owners
		(sd->status.guild_id != g_id && flag&2) || //Warp out outsiders
		(sd->status.guild_id == 0 && flag&2)	// Warp out players not in guild
	)
		pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,CLR_TELEPORT);
	return 1;
}

static int buildin_maprespawnguildid_sub_mob(struct block_list *bl,va_list ap)
{
	struct mob_data *md=(struct mob_data *)bl;

	if(!md->guardian_data && md->mob_id != MOBID_EMPERIUM && ( !mob_is_clone(md->mob_id) || battle_config.guild_maprespawn_clones ))
		status_kill(bl);

	return 1;
}

/*
 * Function to kick guild members out of a map and to their save points.
 * m : mapid
 * g_id : owner guild id
 * flag & 1 : Warp out owners
 * flag & 2 : Warp out outsiders
 * flag & 4 : reserved for mobs
 * */
BUILDIN_FUNC(maprespawnguildid)
{
	const char *mapname=script_getstr(st,2);
	int g_id=script_getnum(st,3);
	int flag=script_getnum(st,4);

	int16 m=map_mapname2mapid(mapname);

	if(m == -1)
		return SCRIPT_CMD_SUCCESS;

	//Catch ALL players (in case some are 'between maps' on execution time)
	map_foreachpc(buildin_maprespawnguildid_sub_pc,m,g_id,flag);
	if (flag&4) //Remove script mobs.
		map_foreachinmap(buildin_maprespawnguildid_sub_mob,m,BL_MOB);
	return SCRIPT_CMD_SUCCESS;
}

/// Siege commands

/**
 * Start WoE:FE
 * agitstart();
 */
BUILDIN_FUNC(agitstart)
{
	guild_agit_start();

	return SCRIPT_CMD_SUCCESS;
}

/**
 * End WoE:FE
 * agitend();
 */
BUILDIN_FUNC(agitend)
{
	guild_agit_end();

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Start WoE:SE
 * agitstart2();
 */
BUILDIN_FUNC(agitstart2)
{
	guild_agit2_start();

	return SCRIPT_CMD_SUCCESS;
}

/**
 * End WoE:SE
 * agitend();
 */
BUILDIN_FUNC(agitend2)
{
	guild_agit2_end();

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Start WoE:TE
 * agitstart3();
 */
BUILDIN_FUNC(agitstart3)
{
	guild_agit3_start();

	return SCRIPT_CMD_SUCCESS;
}

/**
 * End WoE:TE
 * agitend3();
 */
BUILDIN_FUNC(agitend3)
{
	guild_agit3_end();

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Returns whether WoE:FE is on or off.
 * agitcheck();
 */
BUILDIN_FUNC(agitcheck)
{
	script_pushint(st, agit_flag);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Returns whether WoE:SE is on or off.
 * agitcheck2();
 */
BUILDIN_FUNC(agitcheck2)
{
	script_pushint(st, agit2_flag);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Returns whether WoE:TE is on or off.
 * agitcheck3();
 */
BUILDIN_FUNC(agitcheck3)
{
	script_pushint(st, agit3_flag);
	return SCRIPT_CMD_SUCCESS;
}

/// Sets the guild_id of this npc.
///
/// flagemblem <guild_id>;
BUILDIN_FUNC(flagemblem)
{
	TBL_NPC* nd;
	int g_id = script_getnum(st,2);

	if(g_id < 0)
		return SCRIPT_CMD_SUCCESS;

	nd = (TBL_NPC*)map_id2nd(st->oid);
	if( nd == NULL ) {
		ShowError("script:flagemblem: npc %d not found\n", st->oid);
	} else if( nd->subtype != NPCTYPE_SCRIPT ) {
		ShowError("script:flagemblem: unexpected subtype %d for npc %d '%s'\n", nd->subtype, st->oid, nd->exname);
	} else {
		bool changed = ( nd->u.scr.guild_id != g_id )?true:false;
		nd->u.scr.guild_id = g_id;
		clif_guild_emblem_area(&nd->bl);
		/* guild flag caching */
		if( g_id ) /* adding a id */
			guild_flag_add(nd);
		else if( changed ) /* removing a flag */
			guild_flag_remove(nd);
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getcastlename)
{
	const char* mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	struct guild_castle* gc = guild_mapname2gc(mapname);
	const char* name = (gc) ? gc->castle_name : "";
	script_pushstrcopy(st,name);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getcastledata)
{
	const char *mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	int index = script_getnum(st,3);
	struct guild_castle *gc = guild_mapname2gc(mapname);

	if (gc == NULL) {
		script_pushint(st,0);
		ShowWarning("buildin_getcastledata: guild castle for map '%s' not found\n", mapname);
		return SCRIPT_CMD_FAILURE;
	}

	switch (index) {
		case 1:
			script_pushint(st,gc->guild_id); break;
		case 2:
			script_pushint(st,gc->economy); break;
		case 3:
			script_pushint(st,gc->defense); break;
		case 4:
			script_pushint(st,gc->triggerE); break;
		case 5:
			script_pushint(st,gc->triggerD); break;
		case 6:
			script_pushint(st,gc->nextTime); break;
		case 7:
			script_pushint(st,gc->payTime); break;
		case 8:
			script_pushint(st,gc->createTime); break;
		case 9:
			script_pushint(st,gc->visibleC); break;
		default:
			if (index > 9 && index <= 9+MAX_GUARDIANS) {
				script_pushint(st,gc->guardian[index-10].visible);
				break;
			}
			script_pushint(st,0);
			ShowWarning("buildin_getcastledata: index = '%d' is out of allowed range\n", index);
			return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(setcastledata)
{
	const char *mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	int index = script_getnum(st,3);
	int value = script_getnum(st,4);
	struct guild_castle *gc = guild_mapname2gc(mapname);

	if (gc == NULL) {
		ShowWarning("buildin_setcastledata: guild castle for map '%s' not found\n", mapname);
		return SCRIPT_CMD_FAILURE;
	}

	if (index <= 0 || index > 9+MAX_GUARDIANS) {
		ShowWarning("buildin_setcastledata: index = '%d' is out of allowed range\n", index);
		return SCRIPT_CMD_FAILURE;
	}

	guild_castledatasave(gc->castle_id, index, value);
	return SCRIPT_CMD_SUCCESS;
}

/* =====================================================================
 * ---------------------------------------------------------------------*/
BUILDIN_FUNC(requestguildinfo)
{
	int guild_id=script_getnum(st,2);
	const char *event=NULL;

	if( script_hasdata(st,3) ){
		event=script_getstr(st,3);
		check_event(st, event);
	}

	if(guild_id>0)
		guild_npc_request_info(guild_id,event);
	return SCRIPT_CMD_SUCCESS;
}

/// Returns the number of cards that have been compounded onto the specified equipped item.
/// getequipcardcnt(<equipment slot>);
BUILDIN_FUNC(getequipcardcnt)
{
	int i=-1,j,num;
	TBL_PC *sd;
	int count;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	num=script_getnum(st,2);
	if (equip_index_check(num))
		i=pc_checkequip(sd,equip_bitmask[num]);

	if (i < 0 || !sd->inventory_data[i]) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if(itemdb_isspecial(sd->inventory.u.items_inventory[i].card[0]))
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	count = 0;
	for( j = 0; j < sd->inventory_data[i]->slot; j++ )
		if( sd->inventory.u.items_inventory[i].card[j] && itemdb_type(sd->inventory.u.items_inventory[i].card[j]) == IT_CARD )
			count++;

	script_pushint(st,count);
	return SCRIPT_CMD_SUCCESS;
}

/// Removes all cards from the item found in the specified equipment slot of the invoking character,
/// and give them to the character. If any cards were removed in this manner, it will also show a success effect.
/// successremovecards <slot>;
BUILDIN_FUNC(successremovecards) {
	int i=-1,c,cardflag=0;

	TBL_PC* sd;
	int num;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	num = script_getnum(st,2);

	if (equip_index_check(num))
		i=pc_checkequip(sd,equip_bitmask[num]);

	if (i < 0 || !sd->inventory_data[i]) {
		return SCRIPT_CMD_SUCCESS;
	}

	if(itemdb_isspecial(sd->inventory.u.items_inventory[i].card[0]))
		return SCRIPT_CMD_SUCCESS;

	for( c = sd->inventory_data[i]->slot - 1; c >= 0; --c ) {
		if( sd->inventory.u.items_inventory[i].card[c] && itemdb_type(sd->inventory.u.items_inventory[i].card[c]) == IT_CARD ) {// extract this card from the item
			unsigned char flag = 0;
			struct item item_tmp;
			memset(&item_tmp,0,sizeof(item_tmp));
			cardflag = 1;
			item_tmp.nameid   = sd->inventory.u.items_inventory[i].card[c];
			item_tmp.identify = 1;

			if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){	// get back the cart in inventory
				clif_additem(sd,0,0,flag);
				map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
			}
		}
	}

	if(cardflag == 1) {//if card was remove remplace item with no card
		unsigned char flag = 0, j;
		struct item item_tmp;
		memset(&item_tmp,0,sizeof(item_tmp));

		item_tmp.nameid      = sd->inventory.u.items_inventory[i].nameid;
		item_tmp.identify    = 1;
		item_tmp.refine      = sd->inventory.u.items_inventory[i].refine;
		item_tmp.attribute   = sd->inventory.u.items_inventory[i].attribute;
		item_tmp.expire_time = sd->inventory.u.items_inventory[i].expire_time;
		item_tmp.bound       = sd->inventory.u.items_inventory[i].bound;

		for (j = sd->inventory_data[i]->slot; j < MAX_SLOTS; j++)
			item_tmp.card[j]=sd->inventory.u.items_inventory[i].card[j];
		
		for (j = 0; j < MAX_ITEM_RDM_OPT; j++){
			item_tmp.option[j].id=sd->inventory.u.items_inventory[i].option[j].id;
			item_tmp.option[j].value=sd->inventory.u.items_inventory[i].option[j].value;
			item_tmp.option[j].param=sd->inventory.u.items_inventory[i].option[j].param;
		}

		pc_delitem(sd,i,1,0,3,LOG_TYPE_SCRIPT);
		if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){	//chk if can be spawn in inventory otherwise put on floor
			clif_additem(sd,0,0,flag);
			map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
		}

		clif_misceffect(&sd->bl,3);
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Removes all cards from the item found in the specified equipment slot of the invoking character.
/// failedremovecards <slot>, <type>;
/// <type>=0 : will destroy both the item and the cards.
/// <type>=1 : will keep the item, but destroy the cards.
/// <type>=2 : will keep the cards, but destroy the item.
/// <type>=? : will just display the failure effect.
BUILDIN_FUNC(failedremovecards) {
	int i=-1,c,cardflag=0;

	TBL_PC* sd;
	int num;
	int typefail;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	num = script_getnum(st,2);
	typefail = script_getnum(st,3);

	if (equip_index_check(num))
		i=pc_checkequip(sd,equip_bitmask[num]);

	if (i < 0 || !sd->inventory_data[i])
		return SCRIPT_CMD_SUCCESS;

	if(itemdb_isspecial(sd->inventory.u.items_inventory[i].card[0]))
		return SCRIPT_CMD_SUCCESS;

	for( c = sd->inventory_data[i]->slot - 1; c >= 0; --c ) {
		if( sd->inventory.u.items_inventory[i].card[c] && itemdb_type(sd->inventory.u.items_inventory[i].card[c]) == IT_CARD ) {
			cardflag = 1;

			if(typefail == 2) {// add cards to inventory, clear
				unsigned char flag = 0;
				struct item item_tmp;

				memset(&item_tmp,0,sizeof(item_tmp));

				item_tmp.nameid   = sd->inventory.u.items_inventory[i].card[c];
				item_tmp.identify = 1;

				if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){
					clif_additem(sd,0,0,flag);
					map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
				}
			}
		}
	}

	if(cardflag == 1) {
		if(typefail == 0 || typefail == 2){	// destroy the item
			pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT);
		}else if(typefail == 1){ // destroy the card
			unsigned char flag = 0, j;
			struct item item_tmp;

			memset(&item_tmp,0,sizeof(item_tmp));

			item_tmp.nameid      = sd->inventory.u.items_inventory[i].nameid;
			item_tmp.identify    = 1;
			item_tmp.refine      = sd->inventory.u.items_inventory[i].refine;
			item_tmp.attribute   = sd->inventory.u.items_inventory[i].attribute;
			item_tmp.expire_time = sd->inventory.u.items_inventory[i].expire_time;
			item_tmp.bound       = sd->inventory.u.items_inventory[i].bound;

			for (j = sd->inventory_data[i]->slot; j < MAX_SLOTS; j++)
				item_tmp.card[j]=sd->inventory.u.items_inventory[i].card[j];
			
			for (j = 0; j < MAX_ITEM_RDM_OPT; j++){
				item_tmp.option[j].id=sd->inventory.u.items_inventory[i].option[j].id;
				item_tmp.option[j].value=sd->inventory.u.items_inventory[i].option[j].value;
				item_tmp.option[j].param=sd->inventory.u.items_inventory[i].option[j].param;
			}

			pc_delitem(sd,i,1,0,2,LOG_TYPE_SCRIPT);

			if((flag=pc_additem(sd,&item_tmp,1,LOG_TYPE_SCRIPT))){
				clif_additem(sd,0,0,flag);
				map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
			}
		}
		clif_misceffect(&sd->bl,2);
	}
	return SCRIPT_CMD_SUCCESS;
}

/* ================================================================
 * mapwarp "<from map>","<to map>",<x>,<y>,<type>,<ID for Type>;
 * type: 0=everyone, 1=guild, 2=party;	[Reddozen]
 * improved by [Lance]
 * ================================================================*/
BUILDIN_FUNC(mapwarp)	// Added by RoVeRT
{
	int x,y,m,check_val=0,check_ID=0,i=0;
	struct guild *g = NULL;
	struct party_data *p = NULL;
	const char *str;
	const char *mapname;
	unsigned int index;
	mapname=script_getstr(st,2);
	str=script_getstr(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);
	if(script_hasdata(st,7)){
		check_val=script_getnum(st,6);
		check_ID=script_getnum(st,7);
	}

	if((m=map_mapname2mapid(mapname))< 0)
		return SCRIPT_CMD_SUCCESS;

	if(!(index=mapindex_name2id(str)))
		return SCRIPT_CMD_SUCCESS;

	switch(check_val){
		case 1:
			g = guild_search(check_ID);
			if (g){
				for( i=0; i < g->max_member; i++)
				{
					if(g->member[i].sd && g->member[i].sd->bl.m==m){
						pc_setpos(g->member[i].sd,index,x,y,CLR_TELEPORT);
					}
				}
			}
			break;
		case 2:
			p = party_search(check_ID);
			if(p){
				for(i=0;i<MAX_PARTY; i++){
					if(p->data[i].sd && p->data[i].sd->bl.m == m){
						pc_setpos(p->data[i].sd,index,x,y,CLR_TELEPORT);
					}
				}
			}
			break;
		default:
			map_foreachinmap(buildin_areawarp_sub,m,BL_PC,index,x,y,0,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

static int buildin_mobcount_sub(struct block_list *bl,va_list ap)	// Added by RoVeRT
{
	char *event=va_arg(ap,char *);
	struct mob_data *md = ((struct mob_data *)bl);
	if( md->status.hp > 0 && (!event || strcmp(event,md->npc_event) == 0) )
		return 1;
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mobcount)	// Added by RoVeRT
{
	const char *mapname,*event;
	int16 m;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);

	if( strcmp(event, "all") == 0 )
		event = NULL;
	else
		check_event(st, event);

	if( strcmp(mapname, "this") == 0 ) {
		struct map_session_data *sd;
		if( script_rid2sd(sd) )
			m = sd->bl.m;
		else {
			script_pushint(st,-1);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	else if( (m = map_mapname2mapid(mapname)) < 0 ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st,map_foreachinmap(buildin_mobcount_sub, m, BL_MOB, event));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(marriage)
{
	TBL_PC *sd;
	TBL_PC *p_sd;

	if(!script_rid2sd(sd) || !(p_sd=map_nick2sd(script_getstr(st,2),false)) || !pc_marriage(sd,p_sd)){
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(wedding_effect)
{
	TBL_PC *sd;
	struct block_list *bl;

	if(!script_rid2sd(sd)) {
		bl=map_id2bl(st->oid);
	} else
		bl=&sd->bl;
	clif_wedding_effect(bl);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * divorce({<char_id>})
 **/
BUILDIN_FUNC(divorce)
{
	TBL_PC *sd;

	if (!script_charid2sd(2,sd) || !pc_divorce(sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * ispartneron({<char_id>})
 **/
BUILDIN_FUNC(ispartneron)
{
	TBL_PC *sd;

	if (!script_charid2sd(2,sd) || !pc_ismarried(sd) ||
		map_charid2sd(sd->status.partner_id) == NULL)
	{
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getpartnerid({<char_id>})
 **/
BUILDIN_FUNC(getpartnerid)
{
	TBL_PC *sd;

	if (!script_charid2sd(2,sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,sd->status.partner_id);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getchildid({<char_id>})
 **/
BUILDIN_FUNC(getchildid)
{
	TBL_PC *sd;

	if (!script_charid2sd(2,sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,sd->status.child);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getmotherid({<char_id>})
 **/
BUILDIN_FUNC(getmotherid)
{
	TBL_PC *sd;

	if (!script_charid2sd(2,sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,sd->status.mother);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getfatherid({<char_id>})
 **/
BUILDIN_FUNC(getfatherid)
{
	TBL_PC *sd;

	if (!script_charid2sd(2,sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,sd->status.father);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(warppartner)
{
	int x,y;
	unsigned short mapindex;
	const char *str;
	TBL_PC *sd;
	TBL_PC *p_sd;

	if(!script_rid2sd(sd) || !pc_ismarried(sd) ||
	(p_sd=map_charid2sd(sd->status.partner_id)) == NULL) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);

	mapindex = mapindex_name2id(str);
	if (mapindex) {
		pc_setpos(p_sd,mapindex,x,y,CLR_OUTSIGHT);
		script_pushint(st,1);
	} else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*================================================
 * Script for Displaying MOB Information [Valaris]
 *------------------------------------------------*/
BUILDIN_FUNC(strmobinfo)
{

	int num=script_getnum(st,2);
	int class_=script_getnum(st,3);

	if(!mobdb_checkid(class_))
	{
		if (num < 3) //requested a string
			script_pushconststr(st,"");
		else
			script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch (num) {
	case 1: script_pushstrcopy(st,mob_db(class_)->name); break;
	case 2: script_pushstrcopy(st,mob_db(class_)->jname); break;
	case 3: script_pushint(st,mob_db(class_)->lv); break;
	case 4: script_pushint(st,mob_db(class_)->status.max_hp); break;
	case 5: script_pushint(st,mob_db(class_)->status.max_sp); break;
	case 6: script_pushint(st,mob_db(class_)->base_exp); break;
	case 7: script_pushint(st,mob_db(class_)->job_exp); break;
	default:
		script_pushint(st,0);
		break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Summon guardians [Valaris]
 * guardian("<map name>",<x>,<y>,"<name to show>",<mob id>{,"<event label>"}{,<guardian index>}) -> <id>
 *------------------------------------------*/
BUILDIN_FUNC(guardian)
{
	int class_=0,x=0,y=0,guardian=0;
	const char *str,*mapname,*evt="";
	bool has_index = false;

	mapname	  =script_getstr(st,2);
	x	  =script_getnum(st,3);
	y	  =script_getnum(st,4);
	str	  =script_getstr(st,5);
	class_=script_getnum(st,6);

	if( script_hasdata(st,8) )
	{// "<event label>",<guardian index>
		evt=script_getstr(st,7);
		guardian=script_getnum(st,8);
		has_index = true;
	} else if( script_hasdata(st,7) ){
		struct script_data *data = script_getdata(st,7);
		get_val(st,data);
		if( data_isstring(data) )
		{// "<event label>"
			evt=script_getstr(st,7);
		} else if( data_isint(data) )
		{// <guardian index>
			guardian=script_getnum(st,7);
			has_index = true;
		} else {
			ShowError("script:guardian: invalid data type for argument #6 (from 1)\n");
			script_reportdata(data);
			return SCRIPT_CMD_FAILURE;
		}
	}

	check_event(st, evt);
	script_pushint(st, mob_spawn_guardian(mapname,x,y,str,class_,evt,guardian,has_index));
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Invisible Walls [Zephyrus]
 *------------------------------------------*/
BUILDIN_FUNC(setwall)
{
	const char *mapname, *name;
	int x, y, m, size, dir;
	bool shootable;

	mapname = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	size = script_getnum(st,5);
	dir = script_getnum(st,6);
	shootable = script_getnum(st,7) != 0;
	name = script_getstr(st,8);

	if( (m = map_mapname2mapid(mapname)) < 0 )
		return SCRIPT_CMD_SUCCESS; // Invalid Map

	map_iwall_set(m, x, y, size, dir, shootable, name);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(delwall)
{
	const char *name = script_getstr(st,2);

	if( !map_iwall_remove(name) ){
		ShowError( "buildin_delwall: wall \"%s\" does not exist.\n", name );
		return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(checkwall)
{
	const char *wall_name = script_getstr(st, 2);

	script_pushint(st, map_iwall_exist(wall_name));
	return SCRIPT_CMD_SUCCESS;
}

/// Retrieves various information about the specified guardian.
///
/// guardianinfo("<map_name>", <index>, <type>) -> <value>
/// type: 0 - whether it is deployed or not
///       1 - maximum hp
///       2 - current hp
///
BUILDIN_FUNC(guardianinfo)
{
	const char* mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	int id = script_getnum(st,3);
	int type = script_getnum(st,4);

	struct guild_castle* gc = guild_mapname2gc(mapname);
	struct mob_data* gd;

	if( gc == NULL || id < 0 || id >= MAX_GUARDIANS )
	{
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}

	if( type == 0 )
		script_pushint(st, gc->guardian[id].visible);
	else
	if( !gc->guardian[id].visible )
		script_pushint(st,-1);
	else
	if( (gd = map_id2md(gc->guardian[id].id)) == NULL )
		script_pushint(st,-1);
	else
	{
		if     ( type == 1 ) script_pushint(st,gd->status.max_hp);
		else if( type == 2 ) script_pushint(st,gd->status.hp);
		else
			script_pushint(st,-1);
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get the item name by item_id or null
 *------------------------------------------*/
BUILDIN_FUNC(getitemname)
{
	unsigned short item_id = 0;
	struct item_data *i_data;
	char *item_name;
	struct script_data *data;

	data=script_getdata(st,2);
	get_val(st,data);

	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			item_id=item_data->nameid;
	}else
		item_id=conv_num(st,data);

	i_data = itemdb_exists(item_id);
	if (i_data == NULL)
	{
		script_pushconststr(st,"null");
		return SCRIPT_CMD_SUCCESS;
	}
	item_name=(char *)aMalloc(ITEM_NAME_LENGTH*sizeof(char));

	memcpy(item_name, i_data->jname, ITEM_NAME_LENGTH);
	script_pushstr(st,item_name);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Returns number of slots an item has. [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(getitemslots)
{
	unsigned short item_id;
	struct item_data *i_data;

	item_id=script_getnum(st,2);

	i_data = itemdb_exists(item_id);

	if (i_data)
		script_pushint(st,i_data->slot);
	else
		script_pushint(st,-1);
	return SCRIPT_CMD_SUCCESS;
}

// TODO: add matk here if needed/once we get rid of RENEWAL

/*==========================================
 * Returns some values of an item [Lupus]
 * Price, Weight, etc...
	getiteminfo(itemID,n), where n
		0 value_buy;
		1 value_sell;
		2 type;
		3 maxchance = Max drop chance of this item e.g. 1 = 0.01% , etc..
				if = 0, then monsters don't drop it at all (rare or a quest item)
				if = -1, then this item is sold in NPC shops only
		4 sex;
		5 equip;
		6 weight;
		7 atk;
		8 def;
		9 range;
		10 slot;
		11 look;
		12 elv;
		13 wlv;
		14 view id
		15 eLvmax
		16 matk (renewal)
 *------------------------------------------*/
BUILDIN_FUNC(getiteminfo)
{
	unsigned short item_id,n;
	struct item_data *i_data;

	item_id	= script_getnum(st,2);
	n	= script_getnum(st,3);
	i_data = itemdb_exists(item_id);

	if (i_data && n <= 16) {
		int *item_arr = (int*)&i_data->value_buy;
#ifndef RENEWAL
		if (n == 16)
			script_pushint(st,0);
		else
#endif
		script_pushint(st,item_arr[n]);
	} else
		script_pushint(st,-1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Set some values of an item [Lupus]
 * Price, Weight, etc...
	setiteminfo(itemID,n,Value), where n
		0 value_buy;
		1 value_sell;
		2 type;
		3 maxchance = Max drop chance of this item e.g. 1 = 0.01% , etc..
				if = 0, then monsters don't drop it at all (rare or a quest item)
				if = -1, then this item is sold in NPC shops only
		4 sex;
		5 equip;
		6 weight;
		7 atk;
		8 def;
		9 range;
		10 slot;
		11 look;
		12 elv;
		13 wlv;
		14 view id
  * Returns Value or -1 if the wrong field's been set
 *------------------------------------------*/
BUILDIN_FUNC(setiteminfo)
{
	unsigned short item_id;
	int n,value;
	struct item_data *i_data;

	item_id	= script_getnum(st,2);
	n	= script_getnum(st,3);
	value	= script_getnum(st,4);
	i_data = itemdb_exists(item_id);

	if (i_data && n>=0 && n<=14) {
		int *item_arr = (int*)&i_data->value_buy;
		item_arr[n] = value;
		script_pushint(st,value);
	} else
		script_pushint(st,-1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Returns value from equipped item slot n [Lupus]
	getequipcardid(num,slot)
	where
		num = eqip position slot
		slot = 0,1,2,3 (Card Slot N)

	This func returns CARD ID, 255,254,-255 (for card 0, if the item is produced)
		it's useful when you want to check item cards or if it's signed
	Useful for such quests as "Sign this refined item with players name" etc
		Hat[0] +4 -> Player's Hat[0] +4
 *------------------------------------------*/
BUILDIN_FUNC(getequipcardid)
{
	int i=-1,num,slot;
	TBL_PC *sd;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	num=script_getnum(st,2);
	slot=script_getnum(st,3);

	if (equip_index_check(num))
		i=pc_checkequip(sd,equip_bitmask[num]);
	if(i >= 0 && slot>=0 && slot<4)
		script_pushint(st,sd->inventory.u.items_inventory[i].card[slot]);
	else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * petskillbonus [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(petskillbonus)
{
	struct pet_data *pd;
	TBL_PC *sd;

	if(!script_rid2sd(sd) || sd->pd == NULL)
		return SCRIPT_CMD_FAILURE;

	pd = sd->pd;
	if (pd->bonus)
	{ //Clear previous bonus
		if (pd->bonus->timer != INVALID_TIMER)
			delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
	} else //init
		pd->bonus = (struct pet_bonus *) aMalloc(sizeof(struct pet_bonus));

	pd->bonus->type = script_getnum(st,2);
	pd->bonus->val = script_getnum(st,3);
	pd->bonus->duration = script_getnum(st,4);
	pd->bonus->delay = script_getnum(st,5);

	if (pd->state.skillbonus == 1)
		pd->state.skillbonus = 0;	// waiting state

	// wait for timer to start
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->bonus->timer = INVALID_TIMER;
	else
		pd->bonus->timer = add_timer(gettick()+pd->bonus->delay*1000, pet_skill_bonus_timer, sd->bl.id, 0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * pet looting [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(petloot)
{
	int max;
	struct pet_data *pd;
	TBL_PC *sd;

	if(!script_rid2sd(sd) || sd->pd==NULL)
		return SCRIPT_CMD_SUCCESS;

	max=script_getnum(st,2);

	if(max < 1)
		max = 1;	//Let'em loot at least 1 item.
	else if (max > MAX_PETLOOT_SIZE)
		max = MAX_PETLOOT_SIZE;

	pd = sd->pd;
	if (pd->loot != NULL)
	{	//Release whatever was there already and reallocate memory
		pet_lootitem_drop(pd, pd->master);
		aFree(pd->loot->item);
	}
	else
		pd->loot = (struct pet_loot *)aMalloc(sizeof(struct pet_loot));

	pd->loot->item = (struct item *)aCalloc(max,sizeof(struct item));

	pd->loot->max=max;
	pd->loot->count = 0;
	pd->loot->weight = 0;
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Set arrays with info of all sd inventory :
 * @inventorylist_id, @inventorylist_amount, @inventorylist_equip,
 * @inventorylist_refine, @inventorylist_identify, @inventorylist_attribute,
 * @inventorylist_card(0..3), @inventorylist_expire
 * @inventorylist_count = scalar
 *------------------------------------------*/
BUILDIN_FUNC(getinventorylist)
{
	TBL_PC *sd;
	char card_var[NAME_LENGTH], randopt_var[50];
	int i,j=0,k;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	for(i=0;i<MAX_INVENTORY;i++){
		if(sd->inventory.u.items_inventory[i].nameid > 0 && sd->inventory.u.items_inventory[i].amount > 0){
			pc_setreg(sd,reference_uid(add_str("@inventorylist_id"), j),sd->inventory.u.items_inventory[i].nameid);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_amount"), j),sd->inventory.u.items_inventory[i].amount);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_equip"), j),sd->inventory.u.items_inventory[i].equip);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_refine"), j),sd->inventory.u.items_inventory[i].refine);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_identify"), j),sd->inventory.u.items_inventory[i].identify);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_attribute"), j),sd->inventory.u.items_inventory[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++)
			{
				sprintf(card_var, "@inventorylist_card%d",k+1);
				pc_setreg(sd,reference_uid(add_str(card_var), j),sd->inventory.u.items_inventory[i].card[k]);
			}
			pc_setreg(sd,reference_uid(add_str("@inventorylist_expire"), j),sd->inventory.u.items_inventory[i].expire_time);
			pc_setreg(sd,reference_uid(add_str("@inventorylist_bound"), j),sd->inventory.u.items_inventory[i].bound);
			for (k = 0; k < MAX_ITEM_RDM_OPT; k++)
			{
				sprintf(randopt_var, "@inventorylist_option_id%d",k+1);
				pc_setreg(sd,reference_uid(add_str(randopt_var), j),sd->inventory.u.items_inventory[i].option[k].id);
				sprintf(randopt_var, "@inventorylist_option_value%d",k+1);
				pc_setreg(sd,reference_uid(add_str(randopt_var), j),sd->inventory.u.items_inventory[i].option[k].value);
				sprintf(randopt_var, "@inventorylist_option_parameter%d",k+1);
				pc_setreg(sd,reference_uid(add_str(randopt_var), j),sd->inventory.u.items_inventory[i].option[k].param);
			}
			j++;
		}
	}
	pc_setreg(sd,add_str("@inventorylist_count"),j);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getskilllist ({<char_id>});
 **/
BUILDIN_FUNC(getskilllist)
{
	TBL_PC *sd;
	int i,j=0;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	for(i=0;i<MAX_SKILL;i++){
		if(sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0){
			pc_setreg(sd,reference_uid(add_str("@skilllist_id"), j),sd->status.skill[i].id);
			pc_setreg(sd,reference_uid(add_str("@skilllist_lv"), j),sd->status.skill[i].lv);
			pc_setreg(sd,reference_uid(add_str("@skilllist_flag"), j),sd->status.skill[i].flag);
			j++;
		}
	}
	pc_setreg(sd,add_str("@skilllist_count"),j);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * clearitem ({<char_id>});
 **/
BUILDIN_FUNC(clearitem)
{
	TBL_PC *sd;
	int i;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	for (i=0; i<MAX_INVENTORY; i++) {
		if (sd->inventory.u.items_inventory[i].amount) {
			pc_delitem(sd, i, sd->inventory.u.items_inventory[i].amount, 0, 0, LOG_TYPE_SCRIPT);
		}
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Disguise Player (returns Mob/NPC ID if success, 0 on fail)
 * disguise <Monster ID>{,<char_id>};
 **/
BUILDIN_FUNC(disguise)
{
	int id;
	TBL_PC* sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	id = script_getnum(st,2);

	if (mobdb_checkid(id) || npcdb_checkid(id)) {
		pc_disguise(sd, id);
		script_pushint(st,id);
	} else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Undisguise Player (returns 1 if success, 0 on fail)
 * undisguise {<char_id>};
 **/
BUILDIN_FUNC(undisguise)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if (sd->disguise) {
		pc_disguise(sd, 0);
		script_pushint(st,0);
	} else {
		script_pushint(st,1);
	}
	return SCRIPT_CMD_SUCCESS;
}

 /**
 * Transform an NPC to another _class
 *
 * classchange(<view id>{,"<NPC name>","<flag>"});
 * @param flag: Specify target
 *   BC_AREA - Sprite is sent to players in the vicinity of the source (default).
 *   BC_SELF - Sprite is sent only to player attached.
 */
BUILDIN_FUNC(classchange)
{
	int _class, type = 1;
	struct npc_data* nd = NULL;
	TBL_PC *sd = map_id2sd(st->rid);
	send_target target = AREA;

	_class = script_getnum(st,2);

	if (script_hasdata(st, 3) && strlen(script_getstr(st,3)) > 0)
		nd = npc_name2id(script_getstr(st, 3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if (nd == NULL)
		return SCRIPT_CMD_FAILURE;

	if (script_hasdata(st, 4)) {
		switch(script_getnum(st, 4)) {
			case BC_SELF:	target = SELF;			break;
			case BC_AREA:
			default:		target = AREA;			break;
		}
	}
	if (target != SELF)
		clif_class_change(&nd->bl,_class,type);
	else if (sd == NULL)
		return SCRIPT_CMD_FAILURE;
	else
		clif_class_change_target(&nd->bl,_class,type,target,sd);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Display an effect
 *------------------------------------------*/
BUILDIN_FUNC(misceffect)
{
	int type;

	type=script_getnum(st,2);

	if( type <= EF_NONE || type >= EF_MAX ){
		ShowError( "buildin_misceffect: unsupported effect id %d\n", type );
		return SCRIPT_CMD_FAILURE;
	}


	if(st->oid && st->oid != fake_nd->bl.id) {
		struct block_list *bl = map_id2bl(st->oid);
		if (bl)
			clif_specialeffect(bl,type,AREA);
	} else{
		TBL_PC *sd;
		if(script_rid2sd(sd))
			clif_specialeffect(&sd->bl,type,AREA);
	}
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * Play a BGM on a single client [Rikter/Yommy]
 *------------------------------------------*/
BUILDIN_FUNC(playBGM)
{
	struct map_session_data* sd;

	if( script_rid2sd(sd) ) {
		clif_playBGM(sd, script_getstr(st,2));
	}
	return SCRIPT_CMD_SUCCESS;
}

static int playBGM_sub(struct block_list* bl,va_list ap)
{
	const char* name = va_arg(ap,const char*);
	clif_playBGM(BL_CAST(BL_PC, bl), name);
	return 0;
}

static int playBGM_foreachpc_sub(struct map_session_data* sd, va_list args)
{
	const char* name = va_arg(args, const char*);
	clif_playBGM(sd, name);
	return 0;
}

/*==========================================
 * Play a BGM on multiple client [Rikter/Yommy]
 *------------------------------------------*/
BUILDIN_FUNC(playBGMall)
{
	const char* name;
	name = script_getstr(st,2);

	if( script_hasdata(st,7) ) {// specified part of map
		const char* mapname = script_getstr(st,3);
		int x0 = script_getnum(st,4);
		int y0 = script_getnum(st,5);
		int x1 = script_getnum(st,6);
		int y1 = script_getnum(st,7);

		map_foreachinallarea(playBGM_sub, map_mapname2mapid(mapname), x0, y0, x1, y1, BL_PC, name);
	}
	else if( script_hasdata(st,3) ) {// entire map
		const char* mapname = script_getstr(st,3);

		map_foreachinmap(playBGM_sub, map_mapname2mapid(mapname), BL_PC, name);
	}
	else {// entire server
		map_foreachpc(&playBGM_foreachpc_sub, name);
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Play a .wav sound for sd
 *------------------------------------------*/
BUILDIN_FUNC(soundeffect)
{
	TBL_PC* sd;

	if(script_rid2sd(sd)){
		const char* name = script_getstr(st,2);
		int type = script_getnum(st,3);

		clif_soundeffect(sd,&sd->bl,name,type);
	}
	return SCRIPT_CMD_SUCCESS;
}

int soundeffect_sub(struct block_list* bl,va_list ap)
{
	char* name = va_arg(ap,char*);
	int type = va_arg(ap,int);

	clif_soundeffect((TBL_PC *)bl, bl, name, type);

	return 0;
}

/*==========================================
 * Play a sound effect (.wav) on multiple clients
 * soundeffectall "<filepath>",<type>{,"<map name>"}{,<x0>,<y0>,<x1>,<y1>};
 *------------------------------------------*/
BUILDIN_FUNC(soundeffectall)
{
	struct block_list* bl;
	struct map_session_data* sd;
	const char* name;
	int type;

	if( st->rid && script_rid2sd(sd) )
		bl = &sd->bl;
	else
		bl = map_id2bl(st->oid);

	if (!bl)
		return SCRIPT_CMD_SUCCESS;

	name = script_getstr(st,2);
	type = script_getnum(st,3);

	//FIXME: enumerating map squares (map_foreach) is slower than enumerating the list of online players (map_foreachpc?) [ultramage]

	if(!script_hasdata(st,4))
	{	// area around
		clif_soundeffectall(bl, name, type, AREA);
	}
	else
	if(!script_hasdata(st,5))
	{	// entire map
		const char* mapname = script_getstr(st,4);
		map_foreachinmap(soundeffect_sub, map_mapname2mapid(mapname), BL_PC, name, type);
	}
	else
	if(script_hasdata(st,8))
	{	// specified part of map
		const char* mapname = script_getstr(st,4);
		int x0 = script_getnum(st,5);
		int y0 = script_getnum(st,6);
		int x1 = script_getnum(st,7);
		int y1 = script_getnum(st,8);
		map_foreachinallarea(soundeffect_sub, map_mapname2mapid(mapname), x0, y0, x1, y1, BL_PC, name, type);
	}
	else
	{
		ShowError("buildin_soundeffectall: insufficient arguments for specific area broadcast.\n");
	}
	return SCRIPT_CMD_SUCCESS;
}
/*==========================================
 * pet status recovery [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(petrecovery)
{
	struct pet_data *pd;
	TBL_PC *sd;
	int sc;

	if(!script_rid2sd(sd) || sd->pd == NULL)
		return SCRIPT_CMD_FAILURE;

	sc = script_getnum(st,2);
	if (sc <= SC_NONE || sc >= SC_MAX) {
		ShowError("buildin_petrecovery: Invalid SC type: %d\n", sc);
		return SCRIPT_CMD_FAILURE;
	}

	pd = sd->pd;

	if (pd->recovery)
	{ //Halt previous bonus
		if (pd->recovery->timer != INVALID_TIMER)
			delete_timer(pd->recovery->timer, pet_recovery_timer);
	} else //Init
		pd->recovery = (struct pet_recovery *)aMalloc(sizeof(struct pet_recovery));

	pd->recovery->type = (sc_type)sc;
	pd->recovery->delay = script_getnum(st,3);
	pd->recovery->timer = INVALID_TIMER;
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * pet attack skills [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
/// petskillattack <skill id>,<level>,<rate>,<bonusrate>
/// petskillattack "<skill name>",<level>,<rate>,<bonusrate>
BUILDIN_FUNC(petskillattack)
{
	struct pet_data *pd;
	struct script_data *data;
	TBL_PC *sd;
	int id = 0;

	if(!script_rid2sd(sd) || sd->pd == NULL)
		return SCRIPT_CMD_FAILURE;

	data = script_getdata(st, 2);
	get_val(st, data);
	id = (data_isstring(data) ? skill_name2id(script_getstr(st,2)) : skill_get_index(script_getnum(st,2)));
	if (!id) {
		ShowError("buildin_petskillattack: Invalid skill defined!\n");
		return SCRIPT_CMD_FAILURE;
	}

	pd = sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_skill_attack *)aMalloc(sizeof(struct pet_skill_attack));

	pd->a_skill->id = id;
	pd->a_skill->damage = 0;
	pd->a_skill->lv = (unsigned short)min(script_getnum(st,3), skill_get_max(pd->a_skill->id));
	pd->a_skill->div_ = 0;
	pd->a_skill->rate = script_getnum(st,4);
	pd->a_skill->bonusrate = script_getnum(st,5);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * pet attack skills [Valaris]
 *------------------------------------------*/
/// petskillattack2 <skill id>,<damage>,<div>,<rate>,<bonusrate>
/// petskillattack2 "<skill name>",<damage>,<div>,<rate>,<bonusrate>
BUILDIN_FUNC(petskillattack2)
{
	struct pet_data *pd;
	struct script_data *data;
	TBL_PC *sd;
	int id = 0;

	if(!script_rid2sd(sd) || sd->pd == NULL)
		return SCRIPT_CMD_FAILURE;

	data = script_getdata(st, 2);
	get_val(st, data);
	id = (data_isstring(data) ? skill_name2id(script_getstr(st,2)) : skill_get_index(script_getnum(st,2)));
	if (!id) {
		ShowError("buildin_petskillattack2: Invalid skill defined!\n");
		return SCRIPT_CMD_FAILURE;
	}

	pd = sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_skill_attack *)aMalloc(sizeof(struct pet_skill_attack));

	pd->a_skill->id = id;
	pd->a_skill->damage = script_getnum(st,3); // Fixed damage
	pd->a_skill->lv = (unsigned short)skill_get_max(pd->a_skill->id); // Adjust to max skill level
	pd->a_skill->div_ = script_getnum(st,4);
	pd->a_skill->rate = script_getnum(st,5);
	pd->a_skill->bonusrate = script_getnum(st,6);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------*/
/// petskillsupport <skill id>,<level>,<delay>,<hp>,<sp>
/// petskillsupport "<skill name>",<level>,<delay>,<hp>,<sp>
BUILDIN_FUNC(petskillsupport)
{
	struct pet_data *pd;
	struct script_data *data;
	TBL_PC *sd;
	int id = 0;

	if(!script_rid2sd(sd) || sd->pd == NULL)
		return SCRIPT_CMD_FAILURE;

	data = script_getdata(st, 2);
	get_val(st, data);
	id = (data_isstring(data) ? skill_name2id(script_getstr(st,2)) : skill_get_index(script_getnum(st,2)));
	if (!id) {
		ShowError("buildin_petskillsupport: Invalid skill defined!\n");
		return SCRIPT_CMD_FAILURE;
	}

	pd = sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != INVALID_TIMER)
		{
			if (pd->s_skill->id)
				delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				delete_timer(pd->s_skill->timer, pet_heal_timer);
		}
	} else //init memory
		pd->s_skill = (struct pet_skill_support *) aMalloc(sizeof(struct pet_skill_support));

	pd->s_skill->id = id;
	pd->s_skill->lv = script_getnum(st,3);
	pd->s_skill->delay = script_getnum(st,4);
	pd->s_skill->hp = script_getnum(st,5);
	pd->s_skill->sp = script_getnum(st,6);

	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->s_skill->timer = INVALID_TIMER;
	else
		pd->s_skill->timer = add_timer(gettick()+pd->s_skill->delay*1000,pet_skill_support_timer,sd->bl.id,0);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Scripted skill effects [Celest]
 *------------------------------------------*/
/// skilleffect <skill id>,<level>
/// skilleffect "<skill name>",<level>
BUILDIN_FUNC(skilleffect)
{
	TBL_PC *sd;
	uint16 skill_id, skill_lv;
	struct script_data *data;
	
	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;
	
	data = script_getdata(st, 2);

	get_val(st, data); // Convert into value in case of a variable
	skill_id = ( data_isstring(data) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	skill_lv = script_getnum(st,3);

	/* Ensure we're standing because the following packet causes the client to virtually set the char to stand,
	 * which leaves the server thinking it still is sitting. */
	if( pc_issit(sd) && pc_setstand(sd, false) ) {
		skill_sit(sd, 0);
		clif_standing(&sd->bl);
	}

	clif_skill_nodamage(&sd->bl,&sd->bl,skill_id,skill_lv,1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * NPC skill effects [Valaris]
 *------------------------------------------*/
/// npcskilleffect <skill id>,<level>,<x>,<y>
/// npcskilleffect "<skill name>",<level>,<x>,<y>
BUILDIN_FUNC(npcskilleffect)
{
	struct block_list *bl= map_id2bl(st->oid);
	uint16 skill_id, skill_lv;
	int x, y;
	struct script_data *data = script_getdata(st, 2);

	get_val(st, data); // Convert into value in case of a variable
	skill_id=( data_isstring(data) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	skill_lv=script_getnum(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);

	if (bl)
		clif_skill_poseffect(bl,skill_id,skill_lv,x,y,gettick());
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Special effects [Valaris]
 *------------------------------------------*/
BUILDIN_FUNC(specialeffect)
{
	struct block_list *bl=map_id2bl(st->oid);
	int type = script_getnum(st,2);
	enum send_target target = script_hasdata(st,3) ? (send_target)script_getnum(st,3) : AREA;

	if(bl==NULL)
		return SCRIPT_CMD_SUCCESS;

	if( type <= EF_NONE || type >= EF_MAX ){
		ShowError( "buildin_specialeffect: unsupported effect id %d\n", type );
		return SCRIPT_CMD_FAILURE;
	}

	if( script_hasdata(st,4) )
	{
		TBL_NPC *nd = npc_name2id(script_getstr(st,4));
		if(nd)
			clif_specialeffect(&nd->bl, type, target);
	}
	else
	{
		if (target == SELF) {
			TBL_PC *sd;
			if (script_rid2sd(sd))
				clif_specialeffect_single(bl,type,sd->fd);
		} else {
			clif_specialeffect(bl, type, target);
		}
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(specialeffect2)
{
	TBL_PC *sd;

	if( script_nick2sd(4,sd) ){
		int type = script_getnum(st,2);
		enum send_target target = script_hasdata(st,3) ? (send_target)script_getnum(st,3) : AREA;

		if( type <= EF_NONE || type >= EF_MAX ){
			ShowError( "buildin_specialeffect2: unsupported effect id %d\n", type );
			return SCRIPT_CMD_FAILURE;
		}

		clif_specialeffect(&sd->bl, type, target);
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * nude({<char_id>});
 * @author [Valaris]
 **/
BUILDIN_FUNC(nude)
{
	TBL_PC *sd;
	int i, calcflag = 0;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	for( i = 0 ; i < EQI_MAX; i++ ) {
		if( sd->equip_index[ i ] >= 0 ) {
			if( !calcflag )
				calcflag = 1;
			pc_unequipitem( sd , sd->equip_index[ i ] , 2);
		}
	}

	if( calcflag )
		status_calc_pc(sd,SCO_NONE);
	return SCRIPT_CMD_SUCCESS;
}

int atcommand_sub(struct script_state* st,int type) {
	TBL_PC *sd, dummy_sd;
	int fd;
	const char *cmd;

	cmd = script_getstr(st,2);

	if (st->rid) {
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;

		fd = sd->fd;
	} else { //Use a dummy character.
		sd = &dummy_sd;
		fd = 0;

		memset(&dummy_sd, 0, sizeof(TBL_PC));
		if (st->oid) {
			struct block_list* bl = map_id2bl(st->oid);
			memcpy(&dummy_sd.bl, bl, sizeof(struct block_list));
			if (bl->type == BL_NPC)
				safestrncpy(dummy_sd.status.name, ((TBL_NPC*)bl)->name, NAME_LENGTH);
			sd->mapindex = (bl->m > 0) ? map_id2index(bl->m) : mapindex_name2id(map_default.mapname);
		}

		// Init Group ID, Level, & permissions
		sd->group_id = sd->group_level = 99;
		sd->permissions |= PC_PERM_ALLPERMISSION;
	}

	if (!is_atcommand(fd, sd, cmd, type)) {
		ShowWarning("buildin_atcommand: failed to execute command '%s'\n", cmd);
		script_reportsrc(st);
		return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * gmcommand [MouseJstr]
 *------------------------------------------*/
BUILDIN_FUNC(atcommand) {
	return atcommand_sub(st,0);
}

/** Displays a message for the player only (like system messages like "you got an apple" )
* dispbottom("<message>"{,<color>{,<char_id>}})
* @param message 
* @param color Hex color default (Green)
*/
BUILDIN_FUNC(dispbottom)
{
	TBL_PC *sd;
	int color = 0;
	const char *message;

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	message = script_getstr(st,2);

	if (script_hasdata(st,3))
		color = script_getnum(st,3); // <color>

	if(sd) {
		if (script_hasdata(st,3))
			clif_messagecolor(&sd->bl, color, message, true, SELF);		// [Napster]
		else
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], message, false, SELF);
	}
	return SCRIPT_CMD_SUCCESS;
}

/*===================================
 * Heal portion of recovery command
 *-----------------------------------*/
int recovery_sub(struct map_session_data* sd, int revive)
{
	if(revive&(1|4) && pc_isdead(sd)) {
		status_revive(&sd->bl, 100, 100);
		clif_displaymessage(sd->fd,msg_txt(sd,16)); // You've been revived!
		clif_specialeffect(&sd->bl, EF_RESURRECTION, AREA);
	} else if(revive&(1|2) && !pc_isdead(sd)) {
		status_percent_heal(&sd->bl, 100, 100);
		clif_displaymessage(sd->fd,msg_txt(sd,680)); // You have been recovered!
	}
	return SCRIPT_CMD_SUCCESS;
}

/*=========================================================================
 * Fully Recover a Character's HP/SP - [Capuche] & [Akinari]
 * recovery <type>{,<option>,<revive_flag>{,<map name>}};
 * <type> determines <option>:
 *	0 : char_id
 *	1 : party_id
 *	2 : guild_id
 *	3 : map_name
 *	4 : all characters
 * <revive_flag>:
 *	1 : Revive and heal all players (default)
 *	2 : Heal living players only
 *	4 : Revive dead players only
 * <map name>:
 *	for types 1-2 : map_name (null = all maps)
 *-------------------------------------------------------------------------*/
BUILDIN_FUNC(recovery)
{
	TBL_PC *sd;
	int map_idx = 0, type = 0, revive = 1;

	type = script_getnum(st,2);

	if(script_hasdata(st,4))
		revive = script_getnum(st,4);

	switch(type) {
		case 0:
			if(!script_charid2sd(3,sd))
				return SCRIPT_CMD_SUCCESS; //If we don't have sd by now, bail out
			recovery_sub(sd, revive);
			break;
		case 1:
		{
			struct party_data* p;
			//When no party given, we use invoker party
			int p_id = 0, i;
			if(script_hasdata(st,5)) {//Bad maps shouldn't cause issues
				map_idx = map_mapname2mapid(script_getstr(st,5));
				if(map_idx < 1) { //But we'll check anyways
					ShowDebug("recovery: bad map name given (%s)\n", script_getstr(st,5));
					return SCRIPT_CMD_FAILURE;
				}
			}
			if(script_hasdata(st,3))
				p_id = script_getnum(st,3);
			else if(script_rid2sd(sd))
				p_id = sd->status.party_id;
			p = party_search(p_id);
			if(p == NULL)
				return SCRIPT_CMD_SUCCESS;
			for (i = 0; i < MAX_PARTY; i++) {
				struct map_session_data* pl_sd;
				if((!(pl_sd = p->data[i].sd) || pl_sd->status.party_id != p_id)
					|| (map_idx && pl_sd->bl.m != map_idx))
					continue;
				recovery_sub(pl_sd, revive);
			}
			break;
		}
		case 2:
		{
			struct guild* g;
			//When no guild given, we use invoker guild
			int g_id = 0, i;
			if(script_hasdata(st,5)) {//Bad maps shouldn't cause issues
				map_idx = map_mapname2mapid(script_getstr(st,5));
				if(map_idx < 1) { //But we'll check anyways
					ShowDebug("recovery: bad map name given (%s)\n", script_getstr(st,5));
					return SCRIPT_CMD_FAILURE;
				}
			}
			if(script_hasdata(st,3))
				g_id = script_getnum(st,3);
			else if(script_rid2sd(sd))
				g_id = sd->status.guild_id;
			g = guild_search(g_id);
			if(g == NULL)
				return SCRIPT_CMD_SUCCESS;
			for (i = 0; i < MAX_GUILD; i++) {
				struct map_session_data* pl_sd;
				if((!(pl_sd = g->member[i].sd) || pl_sd->status.guild_id != g_id)
					|| (map_idx && pl_sd->bl.m != map_idx))
					continue;
				recovery_sub(pl_sd, revive);
			}
			break;
		}
		case 3:
			if(script_hasdata(st,3))
				map_idx = map_mapname2mapid(script_getstr(st,3));
			else if(script_rid2sd(sd))
				map_idx = sd->bl.m;
			if(map_idx < 1)
				return SCRIPT_CMD_FAILURE; //No sd and no map given - return
		case 4:
		{
			struct s_mapiterator *iter;
			struct script_data *data = script_getdata(st, 3);

			get_val(st, data); // Convert into value in case of a variable
			if(script_hasdata(st,3) && !data_isstring(data))
				revive = script_getnum(st,3); // recovery 4,<revive_flag>;
			iter = mapit_getallusers();
			for (sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter)) {
				if(type == 3 && sd->bl.m != map_idx)
					continue;
				recovery_sub(sd, revive);
			}
			mapit_free(iter);
			break;
		}
		default:
			ShowWarning("script: buildin_recovery: Invalid type %d\n", type);
			script_pushint(st,-1);
			return SCRIPT_CMD_FAILURE;
	}
	script_pushint(st,1); //Successfully executed without errors
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get your pet info: getpetinfo <type>{,<char_id>}
 * n -> 0:pet_id 1:pet_class 2:pet_name
 * 3:friendly 4:hungry, 5: rename flag.6:level,
 * 7:game id
 *------------------------------------------*/
BUILDIN_FUNC(getpetinfo)
{
	TBL_PC *sd;
	TBL_PET *pd;
	int type = script_getnum(st,2);

	if (!script_charid2sd(3, sd) || !(pd = sd->pd)) {
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type){
		case PETINFO_ID:		script_pushint(st,pd->pet.pet_id); break;
		case PETINFO_CLASS:		script_pushint(st,pd->pet.class_); break;
		case PETINFO_NAME:		script_pushstrcopy(st,pd->pet.name); break;
		case PETINFO_INTIMATE:	script_pushint(st,pd->pet.intimate); break;
		case PETINFO_HUNGRY:	script_pushint(st,pd->pet.hungry); break;
		case PETINFO_RENAMED:	script_pushint(st,pd->pet.rename_flag); break;
		case PETINFO_LEVEL:		script_pushint(st,(int)pd->pet.level); break;
		case PETINFO_BLOCKID:	script_pushint(st,pd->bl.id); break;
		case PETINFO_EGGID:		script_pushint(st,pd->pet.egg_id); break;
		case PETINFO_FOODID:	script_pushint(st,pd->get_pet_db()->FoodID); break;
		default:
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Get your homunculus info: gethominfo <type>{,<char_id>};
 * type -> 0:hom_id 1:class 2:name
 * 3:friendly 4:hungry, 5: rename flag.
 * 6: level, 7: game id
 *------------------------------------------*/
BUILDIN_FUNC(gethominfo)
{
	TBL_PC *sd;
	TBL_HOM *hd;
	int type=script_getnum(st,2);

	if (!script_charid2sd(3, sd) || !(hd = sd->hd)) {
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(type){
		case 0: script_pushint(st,hd->homunculus.hom_id); break;
		case 1: script_pushint(st,hd->homunculus.class_); break;
		case 2: script_pushstrcopy(st,hd->homunculus.name); break;
		case 3: script_pushint(st,hd->homunculus.intimacy); break;
		case 4: script_pushint(st,hd->homunculus.hunger); break;
		case 5: script_pushint(st,hd->homunculus.rename_flag); break;
		case 6: script_pushint(st,hd->homunculus.level); break;
		case 7: script_pushint(st,hd->bl.id); break;
		default:
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/// Retrieves information about character's mercenary
/// getmercinfo <type>[,<char id>];
BUILDIN_FUNC(getmercinfo)
{
	int type;
	struct map_session_data* sd;
	struct mercenary_data* md;

	if( !script_charid2sd(3,sd) ){
		script_pushnil(st);
		return SCRIPT_CMD_FAILURE;
	}

	type = script_getnum(st,2);
	md = sd->md;

	if( md == NULL ){
		if( type == 2 )
			script_pushconststr(st,"");
		else
			script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch( type )
	{
		case 0: script_pushint(st,md->mercenary.mercenary_id); break;
		case 1: script_pushint(st,md->mercenary.class_); break;
		case 2: script_pushstrcopy(st,md->db->name); break;
		case 3: script_pushint(st,mercenary_get_faith(md)); break;
		case 4: script_pushint(st,mercenary_get_calls(md)); break;
		case 5: script_pushint(st,md->mercenary.kill_count); break;
		case 6: script_pushint(st,mercenary_get_lifetime(md)); break;
		case 7: script_pushint(st,md->db->lv); break;
		case 8: script_pushint(st,md->bl.id); break;
		default:
			ShowError("buildin_getmercinfo: Invalid type %d (char_id=%d).\n", type, sd->status.char_id);
			script_pushnil(st);
			return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Shows wether your inventory(and equips) contain
   selected card or not.
	checkequipedcard(4001);
 *------------------------------------------*/
BUILDIN_FUNC(checkequipedcard)
{
	TBL_PC *sd;

	if(script_rid2sd(sd)){
		int n,i,c=0;
		c=script_getnum(st,2);

		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->inventory.u.items_inventory[i].nameid > 0 && sd->inventory.u.items_inventory[i].amount && sd->inventory_data[i]){
				if (itemdb_isspecial(sd->inventory.u.items_inventory[i].card[0]))
					continue;
				for(n=0;n<sd->inventory_data[i]->slot;n++){
					if(sd->inventory.u.items_inventory[i].card[n] == c) {
						script_pushint(st,1);
						return SCRIPT_CMD_SUCCESS;
					}
				}
			}
		}
	}
	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(jump_zero)
{
	int sel;
	sel=script_getnum(st,2);
	if(!sel) {
		int pos;
		if( !data_islabel(script_getdata(st,3)) ){
			ShowError("script: jump_zero: not label !\n");
			st->state=END;
			return SCRIPT_CMD_FAILURE;
		}

		pos=script_getnum(st,3);
		st->pos=pos;
		st->state=GOTO;
	}
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * movenpc [MouseJstr]
 *------------------------------------------*/
BUILDIN_FUNC(movenpc)
{
	TBL_NPC *nd = NULL;
	const char *npc;
	int x,y;

	npc = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);

	if ((nd = npc_name2id(npc)) == NULL){
		ShowError("script: movenpc: NPC with ID '%s' was not found!\n", npc );
		return -1;
	}

	if (script_hasdata(st,5))
		nd->ud.dir = script_getnum(st,5) % 8;
	npc_movenpc(nd, x, y);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * message [MouseJstr]
 *------------------------------------------*/
BUILDIN_FUNC(message)
{
	const char *msg,*player;
	TBL_PC *pl_sd = NULL;

	player = script_getstr(st,2);
	msg = script_getstr(st,3);

	if((pl_sd=map_nick2sd((char *) player,false)) == NULL)
		return SCRIPT_CMD_SUCCESS;
	clif_displaymessage(pl_sd->fd, msg);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * npctalk("<message>"{,"<NPC name>","<flag>"});
 * @param flag: Specify target
 *   BC_ALL  - Broadcast message is sent server-wide.
 *   BC_MAP  - Message is sent to everyone in the same map as the source of the npc.
 *   BC_AREA - Message is sent to players in the vicinity of the source (default).
 *   BC_SELF - Message is sent only to player attached.
 */
BUILDIN_FUNC(npctalk)
{
	struct npc_data* nd = NULL;
	const char* str = script_getstr(st,2);

	if (script_hasdata(st, 3) && strlen(script_getstr(st,3)) > 0)
		nd = npc_name2id(script_getstr(st, 3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if (nd != NULL) {
		send_target target = AREA;
		char message[CHAT_SIZE_MAX];

		if (script_hasdata(st, 4)) {
			switch(script_getnum(st, 4)) {
				case BC_ALL:	target = ALL_CLIENT;	break;
				case BC_MAP:	target = ALL_SAMEMAP;	break;
				case BC_SELF:	target = SELF;			break;
				case BC_AREA:
				default:		target = AREA;			break;
			}
		}
		safesnprintf(message, sizeof(message), "%s", str);
		if (target != SELF)
			clif_messagecolor(&nd->bl, color_table[COLOR_WHITE], message, false, target);
		else {
			TBL_PC *sd = map_id2sd(st->rid);
			if (sd == NULL)
				return SCRIPT_CMD_FAILURE;
			clif_messagecolor_target(&nd->bl, color_table[COLOR_WHITE], message, false, target, sd);
		}
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Sends a message to the waitingroom of the invoking NPC.
 * chatmes "<message>"{,"<NPC name>"};
 * @author Jey
 */
BUILDIN_FUNC(chatmes)
{
	struct npc_data* nd = NULL;
	const char* str = script_getstr(st,2);

	if (script_hasdata(st, 3))
		nd = npc_name2id(script_getstr(st, 3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if (nd != NULL && nd->chat_id) {
		char message[256];
		safesnprintf(message, sizeof(message), "%s", str);
		clif_GlobalMessage(map_id2bl(nd->chat_id), message, CHAT_WOS);
	}
	return SCRIPT_CMD_SUCCESS;
}

// change npc walkspeed [Valaris]
BUILDIN_FUNC(npcspeed)
{
	struct npc_data* nd;
	int speed;

	speed = script_getnum(st,2);
	nd =(struct npc_data *)map_id2bl(st->oid);

	if( nd ) {
		nd->speed = speed;
		nd->ud.state.speed_changed = 1;
	}

	return SCRIPT_CMD_SUCCESS;
}
// make an npc walk to a position [Valaris]
BUILDIN_FUNC(npcwalkto)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	int x=0,y=0;

	x=script_getnum(st,2);
	y=script_getnum(st,3);

	if(nd) {
		if (!nd->status.hp)
			status_calc_npc(nd, SCO_FIRST);
		else
			status_calc_npc(nd, SCO_NONE);
		unit_walktoxy(&nd->bl,x,y,0);
	}
	return SCRIPT_CMD_SUCCESS;
}

// stop an npc's movement [Valaris]
BUILDIN_FUNC(npcstop)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);

	if(nd) {
		unit_stop_walking(&nd->bl,1|4);
	}
	return SCRIPT_CMD_SUCCESS;
}


/**
 * getlook(<type>{,<char_id>})
 **/
BUILDIN_FUNC(getlook)
{
	int type,val;
	TBL_PC *sd;

	if (!script_charid2sd(3,sd)) {
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	type=script_getnum(st,2);
	val=-1;
	switch(type) {
		// TODO: implement LOOK_BASE as stated in script doc
		case LOOK_HAIR:     	val=sd->status.hair; break; //1
		case LOOK_WEAPON:   	val=sd->status.weapon; break; //2
		case LOOK_HEAD_BOTTOM:	val=sd->status.head_bottom; break; //3
		case LOOK_HEAD_TOP: 	val=sd->status.head_top; break; //4
		case LOOK_HEAD_MID: 	val=sd->status.head_mid; break; //5
		case LOOK_HAIR_COLOR:	val=sd->status.hair_color; break; //6
		case LOOK_CLOTHES_COLOR:	val=sd->status.clothes_color; break; //7
		case LOOK_SHIELD:   	val=sd->status.shield; break; //8
		case LOOK_SHOES:    	break; //9
		case LOOK_ROBE:     	val=sd->status.robe; break; //12
		case LOOK_BODY2:		val=sd->status.body; break; //13
	}

	script_pushint(st,val);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * getsavepoint(<information type>{,<char_id>})
 * @param type 0- map name, 1- x, 2- y
 **/
BUILDIN_FUNC(getsavepoint)
{
	TBL_PC* sd;
	int type;

	if (!script_charid2sd(3,sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	type = script_getnum(st,2);

	switch(type) {
		case 0: script_pushstrcopy(st,mapindex_id2name(sd->status.save_point.map)); break;
		case 1: script_pushint(st,sd->status.save_point.x); break;
		case 2: script_pushint(st,sd->status.save_point.y); break;
		default:
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Get position for BL objects.
 * getmapxy(<map name>,<x>,<y>,<type>{,<char name>});
 * @param mapname: String variable for output map name
 * @param x: Integer variable for output coord X
 * @param y: Integer variable for output coord Y
 * @param type: Type of object
 *   UNITTYPE_PC - Character coord
 *   UNITTYPE_NPC - NPC coord
 *   UNITTYPE_PET - Pet coord
 *   UNITTYPE_HOM - Homun coord
 *   UNITTYPE_MER - Mercenary coord
 *   UNITTYPE_ELEM - Elemental coord
 * @param charname: Name object. If empty or "this" use the current object
 * @return 0 - success; -1 - some error, MapName$,MapX,MapY contains unknown value.
 */
BUILDIN_FUNC(getmapxy)
{
	struct block_list *bl = NULL;
	TBL_PC *sd=NULL;

	int64 num;
	const char *name;
	char prefix;

	int x,y,type;
	char mapname[MAP_NAME_LENGTH];

	if( !data_isreference(script_getdata(st,2)) ) {
		ShowWarning("script: buildin_getmapxy: mapname value is not a variable.\n");
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}
	if( !data_isreference(script_getdata(st,3)) ) {
		ShowWarning("script: buildin_getmapxy: mapx value is not a variable.\n");
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}
	if( !data_isreference(script_getdata(st,4)) ) {
		ShowWarning("script: buildin_getmapxy: mapy value is not a variable.\n");
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if( !is_string_variable(reference_getname(script_getdata(st, 2))) ) {
		ShowWarning("script: buildin_getmapxy: %s is not a string variable.\n",reference_getname(script_getdata(st, 2)));
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if( is_string_variable(reference_getname(script_getdata(st, 3))) ) {
		ShowWarning("script: buildin_getmapxy: %s is a string variable, should be an INT.\n",reference_getname(script_getdata(st, 3)));
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if( is_string_variable(reference_getname(script_getdata(st, 4))) ) {
		ShowWarning("script: buildin_getmapxy: %s is a string variable, should be an INT.\n",reference_getname(script_getdata(st, 4)));
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	// Possible needly check function parameters on C_STR,C_INT,C_INT
	type=script_getnum(st,5);

	switch (type) {
		case UNITTYPE_PC:	//Get Character Position
			if( script_nick2sd(6,sd) )
				bl = &sd->bl;
			break;
		case UNITTYPE_NPC:	//Get NPC Position
			if( script_hasdata(st,6) )
			{
				struct npc_data *nd;
				nd=npc_name2id(script_getstr(st,6));
				if (nd)
					bl = &nd->bl;
			} else //In case the origin is not an npc?
				bl=map_id2bl(st->oid);
			break;
		case UNITTYPE_PET:	//Get Pet Position
			if( script_nick2sd(6, sd) && sd->pd )
				bl = &sd->pd->bl;
			break;
		case UNITTYPE_HOM:	//Get Homun Position
			if( script_nick2sd(6, sd) && sd->hd )
				bl = &sd->hd->bl;
			break;
		case UNITTYPE_MER: //Get Mercenary Position
			if( script_nick2sd(6, sd) && sd->md )
				bl = &sd->md->bl;
			break;
		case UNITTYPE_ELEM: //Get Elemental Position
			if( script_nick2sd(6, sd) && sd->ed )
				bl = &sd->ed->bl;
			break;
		default:
			ShowWarning("script: buildin_getmapxy: Invalid type %d.\n", type);
			script_pushint(st,-1);
			return SCRIPT_CMD_FAILURE;
	}
	if (!bl) { //No object found.
		script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}

	x= bl->x;
	y= bl->y;
	safestrncpy(mapname, map_getmapdata(bl->m)->name, MAP_NAME_LENGTH);

	//Set MapName$
	num=st->stack->stack_data[st->start+2].u.num;
	name=get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix)){
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_getmapxy: variable '%s' for mapname is not a server variable, but no player is attached!", name );
			return SCRIPT_CMD_FAILURE;
		}
	}else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)mapname,script_getref(st,2));

	//Set MapX
	num=st->stack->stack_data[st->start+3].u.num;
	name=get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix)){
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_getmapxy: variable '%s' for mapX is not a server variable, but no player is attached!", name );
			return SCRIPT_CMD_FAILURE;
		}
	}else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)__64BPRTSIZE(x),script_getref(st,3));

	//Set MapY
	num=st->stack->stack_data[st->start+4].u.num;
	name=get_str(script_getvarid(num));
	prefix=*name;

	if(not_server_variable(prefix)){
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_getmapxy: variable '%s' for mapY is not a server variable, but no player is attached!", name );
			return SCRIPT_CMD_FAILURE;
		}
	}else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)__64BPRTSIZE(y),script_getref(st,4));

	//Return Success value
	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/// Returns the map name of given map ID.
///
/// mapid2name <map ID>;
BUILDIN_FUNC(mapid2name)
{
	uint16 m = script_getnum(st, 2);

	if (m < 0) {
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}

	script_pushstrcopy(st, map_mapid2mapname(m));

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Allows player to write NPC logs (i.e. Bank NPC, etc) [Lupus]
 *------------------------------------------*/
BUILDIN_FUNC(logmes)
{
	const char *str;
	struct map_session_data* sd = map_id2sd(st->rid);

	str = script_getstr(st,2);
	if( sd ){
		log_npc(sd,str);
	}else{
		struct npc_data* nd = map_id2nd(st->oid);

		if( !nd ){
			ShowError( "buildin_logmes: Invalid usage without player or npc.\n" );
			return SCRIPT_CMD_FAILURE;
		}

		log_npc(nd,str);
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(summon)
{
	int _class, timeout=0;
	const char *str,*event="";
	TBL_PC *sd;
	struct mob_data *md;
	int tick = gettick();

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;

	str	=script_getstr(st,2);
	_class=script_getnum(st,3);
	if( script_hasdata(st,4) )
		timeout=script_getnum(st,4);
	if( script_hasdata(st,5) ){
		event=script_getstr(st,5);
		check_event(st, event);
	}

	clif_skill_poseffect(&sd->bl,AM_CALLHOMUN,1,sd->bl.x,sd->bl.y,tick);

	md = mob_once_spawn_sub(&sd->bl, sd->bl.m, sd->bl.x, sd->bl.y, str, _class, event, SZ_SMALL, AI_NONE);
	if (md) {
		md->master_id=sd->bl.id;
		md->special_state.ai = AI_ATTACK;
		if( md->deletetimer != INVALID_TIMER )
			delete_timer(md->deletetimer, mob_timer_delete);
		md->deletetimer = add_timer(tick+(timeout>0?timeout:60000),mob_timer_delete,md->bl.id,0);
		mob_spawn (md); //Now it is ready for spawning.
		clif_specialeffect(&md->bl,EF_ENTRY2,AREA);
		sc_start4(NULL,&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
	}
	script_pushint(st, md->bl.id);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Checks whether it is daytime/nighttime
 *------------------------------------------*/
BUILDIN_FUNC(isnight)
{
	script_pushint(st,(night_flag == 1));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(isday)
{
	script_pushint(st,(night_flag == 0));
	return SCRIPT_CMD_SUCCESS;
}

/*================================================
 * Check how many items/cards in the list are
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------*/
BUILDIN_FUNC(isequippedcnt)
{
	TBL_PC *sd;
	int i, id = 1;
	int ret = 0;

	if (!script_rid2sd(sd)) { //If the player is not attached it is a script error anyway... but better prevent the map server from crashing...
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	for (i=0; id!=0; i++) {
		short j;
		FETCH (i+2, id) else id = 0;
		if (id <= 0)
			continue;

		for (j=0; j<EQI_MAX; j++) {
			short index = sd->equip_index[j];
			if(index < 0)
				continue;
			if (pc_is_same_equip_index((enum equip_index)j, sd->equip_index, index))
				continue;

			if(!sd->inventory_data[index])
				continue;

			if (itemdb_type(id) != IT_CARD) { //No card. Count amount in inventory.
				if (sd->inventory_data[index]->nameid == id)
					ret+= sd->inventory.u.items_inventory[index].amount;
			} else { //Count cards.
				short k;
				if (itemdb_isspecial(sd->inventory.u.items_inventory[index].card[0]))
					continue; //No cards
				for(k=0; k<sd->inventory_data[index]->slot; k++) {
					if (sd->inventory.u.items_inventory[index].card[k] == id)
						ret++; //[Lupus]
				}
			}
		}
	}

	script_pushint(st,ret);
	return SCRIPT_CMD_SUCCESS;
}

/*================================================
 * Check whether another card has been
 * equipped - used for 2/15's cards patch [celest]
 * -- Items checked cannot be reused in another
 * card set to prevent exploits
 *------------------------------------------------*/
BUILDIN_FUNC(isequipped)
{
	TBL_PC *sd;
	int i, id = 1;
	int ret = -1;
	//Original hash to reverse it when full check fails.
	unsigned int setitem_hash = 0, setitem_hash2 = 0;

	if (!script_rid2sd(sd)) { //If the player is not attached it is a script error anyway... but better prevent the map server from crashing...
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	setitem_hash = sd->bonus.setitem_hash;
	setitem_hash2 = sd->bonus.setitem_hash2;
	for (i=0; id!=0; i++) {
		int flag = 0;
		short j;
		FETCH (i+2, id) else id = 0;
		if (id <= 0)
			continue;
		for (j=0; j<EQI_MAX; j++) {
			short index = sd->equip_index[j];
			if(index < 0)
				continue;
			if (pc_is_same_equip_index((enum equip_index)i, sd->equip_index, index))
				continue;

			if(!sd->inventory_data[index])
				continue;

			if (itemdb_type(id) != IT_CARD) {
				if (sd->inventory_data[index]->nameid != id)
					continue;
				flag = 1;
				break;
			} else { //Cards
				short k;
				if (sd->inventory_data[index]->slot == 0 ||
					itemdb_isspecial(sd->inventory.u.items_inventory[index].card[0]))
					continue;

				for (k = 0; k < sd->inventory_data[index]->slot; k++)
				{	//New hash system which should support up to 4 slots on any equipment. [Skotlex]
					unsigned int hash = 0;
					if (sd->inventory.u.items_inventory[index].card[k] != id)
						continue;

					hash = 1<<((j<5?j:j-5)*4 + k);
					// check if card is already used by another set
					if ( ( j < 5 ? sd->bonus.setitem_hash : sd->bonus.setitem_hash2 ) & hash)
						continue;

					// We have found a match
					flag = 1;
					// Set hash so this card cannot be used by another
					if (j<5)
						sd->bonus.setitem_hash |= hash;
					else
						sd->bonus.setitem_hash2 |= hash;
					break;
				}
			}
			if (flag) break; //Card found
		}
		if (ret == -1)
			ret = flag;
		else
			ret &= flag;
		if (!ret) break;
	}
	if (!ret) {//When check fails, restore original hash values. [Skotlex]
		sd->bonus.setitem_hash = setitem_hash;
		sd->bonus.setitem_hash2 = setitem_hash2;
	}
	script_pushint(st,ret);
	return SCRIPT_CMD_SUCCESS;
}

/*================================================
 * Check how many given inserted cards in the CURRENT
 * weapon - used for 2/15's cards patch [Lupus]
 *------------------------------------------------*/
BUILDIN_FUNC(cardscnt)
{
	TBL_PC *sd;
	int i, k, id = 1;
	int ret = 0;
	int index;

	if( !script_rid2sd(sd) ){
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	for (i=0; id!=0; i++) {
		FETCH (i+2, id) else id = 0;
		if (id <= 0)
			continue;

		index = current_equip_item_index; //we get CURRENT WEAPON inventory index from status.c [Lupus]
		if(index < 0) continue;

		if(!sd->inventory_data[index])
			continue;

		if(itemdb_type(id) != IT_CARD) {
			if (sd->inventory_data[index]->nameid == id)
				ret+= sd->inventory.u.items_inventory[index].amount;
		} else {
			if (itemdb_isspecial(sd->inventory.u.items_inventory[index].card[0]))
				continue;
			for(k=0; k<sd->inventory_data[index]->slot; k++) {
				if (sd->inventory.u.items_inventory[index].card[k] == id)
					ret++;
			}
		}
	}
	script_pushint(st,ret);
//	script_pushint(st,current_equip_item_index);
	return SCRIPT_CMD_SUCCESS;
}

/*=======================================================
 * Returns the refined number of the current item, or an
 * item with inventory index specified
 *-------------------------------------------------------*/
BUILDIN_FUNC(getrefine)
{
	TBL_PC *sd;
	if (script_rid2sd(sd)){
		if( current_equip_item_index == -1 ){
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		}

		script_pushint(st,sd->inventory.u.items_inventory[current_equip_item_index].refine);
	}else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/*=======================================================
 * Day/Night controls
 *-------------------------------------------------------*/
BUILDIN_FUNC(night)
{
	if (night_flag != 1) map_night_timer(night_timer_tid, 0, 0, 1);
	return SCRIPT_CMD_SUCCESS;
}
BUILDIN_FUNC(day)
{
	if (night_flag != 0) map_day_timer(day_timer_tid, 0, 0, 1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * unequip <equipment slot>{,<char_id>};
 * @author [Spectre]
 **/
BUILDIN_FUNC(unequip) {
	int pos;
	TBL_PC *sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	pos = script_getnum(st,2);
	if (equip_index_check(pos)) {
		short i = pc_checkequip(sd,equip_bitmask[pos]);
		if (i >= 0) {
			pc_unequipitem(sd,i,1|2);
			script_pushint(st, 1);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	ShowError("buildin_unequip: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
	script_pushint(st, 0);
	return SCRIPT_CMD_FAILURE;
}

/**
 * equip <item id>{,<char_id>};
 **/
BUILDIN_FUNC(equip) {
	unsigned short nameid = 0;
	TBL_PC *sd;
	struct item_data *item_data;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	nameid = script_getnum(st,2);
	if ((item_data = itemdb_exists(nameid))) {
		int i;

		ARR_FIND( 0, MAX_INVENTORY, i, sd->inventory.u.items_inventory[i].nameid == nameid );
		if (i < MAX_INVENTORY) {
			pc_equipitem(sd,i,item_data->equip);
			script_pushint(st,1);
			return SCRIPT_CMD_SUCCESS;
		}
	}

	ShowError("buildin_equip: Item %hu cannot be equipped\n",nameid);
	script_pushint(st,0);
	return SCRIPT_CMD_FAILURE;
}

BUILDIN_FUNC(autoequip)
{
	unsigned short nameid;
	int flag;
	struct item_data *item_data;
	nameid=script_getnum(st,2);
	flag=script_getnum(st,3);

	if( ( item_data = itemdb_exists(nameid) ) == NULL )
	{
		ShowError("buildin_autoequip: Invalid item '%hu'.\n", nameid);
		return SCRIPT_CMD_FAILURE;
	}

	if( !itemdb_isequip2(item_data) )
	{
		ShowError("buildin_autoequip: Item '%hu' cannot be equipped.\n", nameid);
		return SCRIPT_CMD_FAILURE;
	}

	item_data->flag.autoequip = flag>0?1:0;
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set the value of a given battle config
 * setbattleflag "<battle flag>",<value>{,<reload>};
 */
BUILDIN_FUNC(setbattleflag)
{
	const char *flag, *value;

	flag = script_getstr(st,2);
	value = script_getstr(st,3);  // HACK: Retrieve number as string (auto-converted) for battle_set_value

	if (battle_set_value(flag, value) == 0)
		ShowWarning("buildin_setbattleflag: unknown battle_config flag '%s'\n",flag);
	else {
		ShowInfo("buildin_setbattleflag: battle_config flag '%s' is now set to '%s'.\n",flag,value);

		if (script_hasdata(st, 4) && script_getnum(st, 4)) { // Only attempt to reload monster data if told to
			const char *expdrop_flags[] = { // Only certain flags require a reload, check for those types
				"base_exp_rate", "job_exp_rate", "mvp_exp_rate", "quest_exp_rate", "heal_exp", "resurrection_exp",
				"item_rate_common", "item_rate_common_boss", "item_rate_common_mvp", "item_drop_common_min", "item_drop_common_max",
				"item_rate_heal", "item_rate_heal_boss", "item_rate_heal_mvp", "item_rate_heal_min", "item_rate_heal_max",
				"item_rate_use", "item_rate_use_boss", "item_rate_use_mvp", "item_rate_use_min", "item_rate_use_max",
				"item_rate_equip", "item_rate_equip_boss", "item_rate_equip_mvp", "item_rate_equip_min", "item_rate_equip_max",
				"item_rate_card", "item_rate_card_boss", "item_rate_card_mvp", "item_rate_card_min", "item_rate_card_max",
				"item_rate_mvp", "item_rate_mvp_min", "item_rate_mvp_max", "item_rate_mvp_mode",
				"item_rate_treasure", "item_rate_treasure_min", "item_rate_treasure_max",
				"item_logarithmic_drops", "drop_rate0item", "drop_rateincrease",
			};
			uint8 i;

			for (i = 0; i < ARRAYLENGTH(expdrop_flags); i++) {
				if (!strcmpi(flag, expdrop_flags[i])) {
					mob_reload();
					break;
				}
			}
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Get the value of a given battle config
 * getbattleflag("<battle flag>")
 */
BUILDIN_FUNC(getbattleflag)
{
	const char *flag = script_getstr(st,2);

	script_pushint(st,battle_get_value(flag));
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// strlen [Valaris]
//-------------------------------------------------------
BUILDIN_FUNC(getstrlen)
{

	const char *str = script_getstr(st,2);
	int len = (str) ? (int)strlen(str) : 0;

	script_pushint(st,len);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// isalpha [Valaris]
//-------------------------------------------------------
BUILDIN_FUNC(charisalpha)
{
	const char *str=script_getstr(st,2);
	int pos=script_getnum(st,3);

	int val = ( str && pos >= 0 && (unsigned int)pos < strlen(str) ) ? ISALPHA( str[pos] ) != 0 : 0;

	script_pushint(st,val);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// charisupper <str>, <index>
//-------------------------------------------------------
BUILDIN_FUNC(charisupper)
{
	const char *str = script_getstr(st,2);
	int pos = script_getnum(st,3);

	int val = ( str && pos >= 0 && (unsigned int)pos < strlen(str) ) ? ISUPPER( str[pos] ) : 0;

	script_pushint(st,val);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// charislower <str>, <index>
//-------------------------------------------------------
BUILDIN_FUNC(charislower)
{
	const char *str = script_getstr(st,2);
	int pos = script_getnum(st,3);

	int val = ( str && pos >= 0 && (unsigned int)pos < strlen(str) ) ? ISLOWER( str[pos] ) : 0;

	script_pushint(st,val);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// charat <str>, <index>
//-------------------------------------------------------
BUILDIN_FUNC(charat) {
	const char *str = script_getstr(st,2);
	int pos = script_getnum(st,3);

	if( pos >= 0 && (unsigned int)pos < strlen(str) ) {
		char output[2];
		output[0] = str[pos];
		output[1] = '\0';
		script_pushstrcopy(st, output);
	} else
		script_pushconststr(st, "");
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// setchar <string>, <char>, <index>
//-------------------------------------------------------
BUILDIN_FUNC(setchar)
{
	const char *str = script_getstr(st,2);
	const char *c = script_getstr(st,3);
	int index = script_getnum(st,4);
	char *output = aStrdup(str);

	if(index >= 0 && index < strlen(output))
		output[index] = *c;

	script_pushstr(st, output);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// insertchar <string>, <char>, <index>
//-------------------------------------------------------
BUILDIN_FUNC(insertchar)
{
	const char *str = script_getstr(st,2);
	const char *c = script_getstr(st,3);
	int index = script_getnum(st,4);
	char *output;
	size_t len = strlen(str);

	if(index < 0)
		index = 0;
	else if(index > len)
		index = len;

	output = (char*)aMalloc(len + 2);

	memcpy(output, str, index);
	output[index] = c[0];
	memcpy(&output[index+1], &str[index], len - index);
	output[len+1] = '\0';

	script_pushstr(st, output);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// delchar <string>, <index>
//-------------------------------------------------------
BUILDIN_FUNC(delchar)
{
	const char *str = script_getstr(st,2);
	int index = script_getnum(st,3);
	char *output;
	size_t len = strlen(str);

	if(index < 0 || index > len) {
		//return original
		output = aStrdup(str);
		script_pushstr(st, output);
		return SCRIPT_CMD_SUCCESS;
	}

	output = (char*)aMalloc(len);

	memcpy(output, str, index);
	memcpy(&output[index], &str[index+1], len - index);

	script_pushstr(st, output);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// strtoupper <str>
//-------------------------------------------------------
BUILDIN_FUNC(strtoupper)
{
	const char *str = script_getstr(st,2);
	char *output = aStrdup(str);
	char *cursor = output;

	while (*cursor != '\0') {
		*cursor = TOUPPER(*cursor);
		cursor++;
	}

	script_pushstr(st, output);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// strtolower <str>
//-------------------------------------------------------
BUILDIN_FUNC(strtolower)
{
	const char *str = script_getstr(st,2);
	char *output = aStrdup(str);
	char *cursor = output;

	while (*cursor != '\0') {
		*cursor = TOLOWER(*cursor);
		cursor++;
	}

	script_pushstr(st, output);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// substr <str>, <start>, <end>
//-------------------------------------------------------
BUILDIN_FUNC(substr)
{
	const char *str = script_getstr(st,2);
	char *output;
	int start = script_getnum(st,3);
	int end = script_getnum(st,4);
	int len = 0;

	if(start >= 0 && end < strlen(str) && start <= end) {
		len = end - start + 1;
		output = (char*)aMalloc(len + 1);
		memcpy(output, &str[start], len);
	} else
		output = (char*)aMalloc(1);

	output[len] = '\0';

	script_pushstr(st, output);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// explode <dest_string_array>, <str>, <delimiter>
// Note: delimiter is limited to 1 char
//-------------------------------------------------------
BUILDIN_FUNC(explode)
{
	struct script_data* data = script_getdata(st, 2);
	const char *str = script_getstr(st,3);
	const char delimiter = script_getstr(st, 4)[0];
	int32 id;
	size_t len = strlen(str);
	int i = 0, j = 0;
	int start;
	char *temp;
	const char* name;
	TBL_PC* sd = NULL;

	temp = (char*)aMalloc(len + 1);

	if( !data_isreference(data) ) {
		ShowError("script:explode: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( !is_string_variable(name) ) {
		ShowError("script:explode: not string array\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// data type mismatch
	}

	if( not_server_variable(*name) ) {
		if( !script_rid2sd(sd) )
			return SCRIPT_CMD_SUCCESS;// no player attached
	}

	while(str[i] != '\0') {
		if(str[i] == delimiter && start < SCRIPT_MAX_ARRAYSIZE-1) { //break at delimiter but ignore after reaching last array index
			temp[j] = '\0';
			set_reg(st, sd, reference_uid(id, start++), name, (void*)temp, reference_getref(data));
			j = 0;
			++i;
		} else {
			temp[j++] = str[i++];
		}
	}

	//set last string
	temp[j] = '\0';
	set_reg(st, sd, reference_uid(id, start), name, (void*)temp, reference_getref(data));

	aFree(temp);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// implode <string_array>
// implode <string_array>, <glue>
//-------------------------------------------------------
BUILDIN_FUNC(implode)
{
	struct script_data* data = script_getdata(st, 2);
	const char *name;
	uint32 glue_len = 0, array_size, id;
	char *output;
	TBL_PC* sd = NULL;

	if( !data_isreference(data) ) {
		ShowError("script:implode: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id = reference_getid(data);
	name = reference_getname(data);

	if( !is_string_variable(name) ) {
		ShowError("script:implode: not string array\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// data type mismatch
	}

	if( not_server_variable(*name) && !script_rid2sd(sd) ) {
		return SCRIPT_CMD_SUCCESS;// no player attached
	}

	//count chars
	array_size = script_array_highest_key(st, sd, name, reference_getref(data)) - 1;

	if(array_size == -1) { //empty array check (AmsTaff)
		ShowWarning("script:implode: array length = 0\n");
		output = (char*)aMalloc(sizeof(char)*5);
		sprintf(output,"%s","NULL");
	} else {
		const char *glue = NULL, *temp;
		size_t len = 0;
		int i, k = 0;

		for(i = 0; i <= array_size; ++i) {
			temp = (char*) get_val2(st, reference_uid(id, i), reference_getref(data));
			len += strlen(temp);
			script_removetop(st, -1, 0);
		}

		//allocate mem
		if( script_hasdata(st,3) ) {
			glue = script_getstr(st,3);
			glue_len = strlen(glue);
			len += glue_len * (array_size);
		}
		output = (char*)aMalloc(len + 1);

		//build output
		for(i = 0; i < array_size; ++i) {
			temp = (char*) get_val2(st, reference_uid(id, i), reference_getref(data));
			len = strlen(temp);
			memcpy(&output[k], temp, len);
			k += len;

			if(glue_len != 0) {
				memcpy(&output[k], glue, glue_len);
				k += glue_len;
			}

			script_removetop(st, -1, 0);
		}

		temp = (char*) get_val2(st, reference_uid(id, array_size), reference_getref(data));
		len = strlen(temp);
		memcpy(&output[k], temp, len);
		k += len;
		script_removetop(st, -1, 0);
		output[k] = '\0';
	}

	script_pushstr(st, output);
	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// sprintf(<format>, ...);
// Implements C sprintf, except format %n. The resulting string is
// returned, instead of being saved in variable by reference.
//-------------------------------------------------------
BUILDIN_FUNC(sprintf)
{
	unsigned int len, argc = 0, arg = 0, buf2_len = 0;
	const char* format;
	char* p;
	char* q;
	char* buf  = NULL;
	char* buf2 = NULL;
	struct script_data* data;
	StringBuf final_buf;

	// Fetch init data
	format = script_getstr(st, 2);
	argc = script_lastdata(st)-2;
	len = strlen(format);

	// Skip parsing, where no parsing is required.
	if(len == 0) {
		script_pushconststr(st,"");
		return SCRIPT_CMD_SUCCESS;
	}

	// Pessimistic alloc
	CREATE(buf, char, len+1);

	// Need not be parsed, just solve stuff like %%.
	if(argc == 0) {
		memcpy(buf,format,len+1);
		script_pushstrcopy(st, buf);
		aFree(buf);
		return SCRIPT_CMD_SUCCESS;
	}

	safestrncpy(buf, format, len+1);

	// Issue sprintf for each parameter
	StringBuf_Init(&final_buf);
	q = buf;
	while((p = strchr(q, '%')) != NULL) {
		if(p != q) {
			len = p - q + 1;

			if(buf2_len < len) {
				RECREATE(buf2, char, len);
				buf2_len = len;
			}

			safestrncpy(buf2, q, len);
			StringBuf_AppendStr(&final_buf, buf2);
			q = p;
		}

		p = q + 1;

		if(*p == '%') {  // %%
			StringBuf_AppendStr(&final_buf, "%");
			q+=2;
			continue;
		}

		if(*p == 'n') {  // %n
			ShowWarning("buildin_sprintf: Format %%n not supported! Skipping...\n");
			script_reportsrc(st);
			q+=2;
			continue;
		}

		if(arg >= argc) {
			ShowError("buildin_sprintf: Not enough arguments passed!\n");
			if(buf) aFree(buf);
			if(buf2) aFree(buf2);
			StringBuf_Destroy(&final_buf);
			script_pushconststr(st,"");
			return SCRIPT_CMD_FAILURE;
		}

		if((p = strchr(q+1, '%')) == NULL)
			p = strchr(q, 0);  // EOS

		len = p - q + 1;

		if(buf2_len < len) {
			RECREATE(buf2, char, len);
			buf2_len = len;
		}

		safestrncpy(buf2, q, len);
		q = p;

		// Note: This assumes the passed value being the correct
		// type to the current format specifier. If not, the server
		// probably crashes or returns anything else, than expected,
		// but it would behave in normal code the same way so it's
		// the scripter's responsibility.
		data = script_getdata(st, arg+3);

		if(data_isstring(data))  // String
			StringBuf_Printf(&final_buf, buf2, script_getstr(st, arg+3));
		else if(data_isint(data))  // Number
			StringBuf_Printf(&final_buf, buf2, script_getnum(st, arg+3));
		else if(data_isreference(data)) {  // Variable
			char* name = reference_getname(data);

			if(name[strlen(name)-1]=='$')  // var Str
				StringBuf_Printf(&final_buf, buf2, script_getstr(st, arg+3));
			else  // var Int
				StringBuf_Printf(&final_buf, buf2, script_getnum(st, arg+3));
		} else {  // Unsupported type
			ShowError("buildin_sprintf: Unknown argument type!\n");
			if(buf) aFree(buf);
			if(buf2) aFree(buf2);
			StringBuf_Destroy(&final_buf);
			script_pushconststr(st,"");
			return SCRIPT_CMD_FAILURE;
		}

		arg++;
	}

	// Append anything left
	if(*q)
		StringBuf_AppendStr(&final_buf, q);

	// Passed more, than needed
	if(arg < argc) {
		ShowWarning("buildin_sprintf: Unused arguments passed.\n");
		script_reportsrc(st);
	}

	script_pushstrcopy(st, StringBuf_Value(&final_buf));

	if(buf) aFree(buf);
	if(buf2) aFree(buf2);
	StringBuf_Destroy(&final_buf);

	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// sscanf(<str>, <format>, ...);
// Implements C sscanf.
//-------------------------------------------------------
BUILDIN_FUNC(sscanf){
	unsigned int argc, arg = 0, len;
	struct script_data* data;
	struct map_session_data* sd = NULL;
	const char* str;
	const char* format;
	const char* p;
	const char* q;
	char* buf = NULL;
	char* buf_p;
	char* ref_str = NULL;
	int ref_int;

	// Get data
	str = script_getstr(st, 2);
	format = script_getstr(st, 3);
	argc = script_lastdata(st)-3;

	len = strlen(format);


	if (len != 0 && strlen(str) == 0) {
		// If the source string is empty but the format string is not, we return -1
		// according to the C specs. (if the format string is also empty, we shall
		// continue and return 0: 0 conversions took place out of the 0 attempted.)
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	CREATE(buf, char, len*2+1);

	// Issue sscanf for each parameter
	*buf = 0;
	q = format;
	while((p = strchr(q, '%'))){
		if(p!=q){
			strncat(buf, q, (size_t)(p-q));
			q = p;
		}
		p = q+1;
		if(*p=='*' || *p=='%'){  // Skip
			strncat(buf, q, 2);
			q+=2;
			continue;
		}
		if(arg>=argc){
			ShowError("buildin_sscanf: Not enough arguments passed!\n");
			script_pushint(st, -1);
			if(buf) aFree(buf);
			if(ref_str) aFree(ref_str);
			return SCRIPT_CMD_FAILURE;
		}
		if((p = strchr(q+1, '%'))==NULL){
			p = strchr(q, 0);  // EOS
		}
		len = p-q;
		strncat(buf, q, len);
		q = p;

		// Validate output
		data = script_getdata(st, arg+4);
		if(!data_isreference(data) || !reference_tovariable(data)){
			ShowError("buildin_sscanf: Target argument is not a variable!\n");
			script_pushint(st, -1);
			if(buf) aFree(buf);
			if(ref_str) aFree(ref_str);
			return SCRIPT_CMD_FAILURE;
		}
		buf_p = reference_getname(data);
		if(not_server_variable(*buf_p) && !script_rid2sd(sd)){
			script_pushint(st, -1);
			if(buf) aFree(buf);
			if(ref_str) aFree(ref_str);
			return SCRIPT_CMD_SUCCESS;
		}

		// Save value if any
		if(buf_p[strlen(buf_p)-1]=='$'){  // String
			if(ref_str==NULL){
				CREATE(ref_str, char, strlen(str)+1);
			}
			if(sscanf(str, buf, ref_str)==0){
				break;
			}
			set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, (void *)(ref_str), reference_getref(data));
		} else {  // Number
			if(sscanf(str, buf, &ref_int)==0){
				break;
			}
			set_reg(st, sd, reference_uid( reference_getid(data), reference_getindex(data) ), buf_p, (void *)__64BPRTSIZE(ref_int), reference_getref(data));
		}
		arg++;

		// Disable used format (%... -> %*...)
		buf_p = strchr(buf, 0);
		memmove(buf_p-len+2, buf_p-len+1, len);
		*(buf_p-len+1) = '*';
	}

	script_pushint(st, arg);
	if(buf) aFree(buf);
	if(ref_str) aFree(ref_str);

	return SCRIPT_CMD_SUCCESS;
}

//=======================================================
// strpos(<haystack>, <needle>)
// strpos(<haystack>, <needle>, <offset>)
//
// Implements PHP style strpos. Adapted from code from
// http://www.daniweb.com/code/snippet313.html, Dave Sinkula
//-------------------------------------------------------
BUILDIN_FUNC(strpos) {
	const char *haystack = script_getstr(st,2);
	const char *needle = script_getstr(st,3);
	int i;
	size_t len;

	if( script_hasdata(st,4) )
		i = script_getnum(st,4);
	else
		i = 0;

	if (needle[0] == '\0') {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	len = strlen(haystack);
	for ( ; i < len; ++i ) {
		if ( haystack[i] == *needle ) {
			// matched starting char -- loop through remaining chars
			const char *h, *n;
			for ( h = &haystack[i], n = needle; *h && *n; ++h, ++n ) {
				if ( *h != *n ) {
					break;
				}
			}
			if ( !*n ) { // matched all of 'needle' to null termination
				script_pushint(st, i);
				return SCRIPT_CMD_SUCCESS;
			}
		}
	}
	script_pushint(st, -1);
	return SCRIPT_CMD_SUCCESS;
}

//===============================================================
// replacestr <input>, <search>, <replace>{, <usecase>{, <count>}}
//
// Note: Finds all instances of <search> in <input> and replaces
// with <replace>. If specified will only replace as many
// instances as specified in <count>. By default will be case
// sensitive.
//---------------------------------------------------------------
BUILDIN_FUNC(replacestr)
{
	const char *input = script_getstr(st, 2);
	const char *find = script_getstr(st, 3);
	const char *replace = script_getstr(st, 4);
	size_t inputlen = strlen(input);
	size_t findlen = strlen(find);
	struct StringBuf output;
	bool usecase = true;

	int count = 0;
	int numFinds = 0;
	int i = 0, f = 0;

	if(findlen == 0) {
		ShowError("script:replacestr: Invalid search length.\n");
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	if(script_hasdata(st, 5)) {
		struct script_data *data = script_getdata(st, 5);

		get_val(st, data); // Convert into value in case of a variable
		if( !data_isstring(data) )
			usecase = script_getnum(st, 5) != 0;
		else {
			ShowError("script:replacestr: Invalid usecase value. Expected int got string\n");
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}

	if(script_hasdata(st, 6)) {
		count = script_getnum(st, 6);
		if(count == 0) {
			ShowError("script:replacestr: Invalid count value. Expected int got string\n");
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}

	StringBuf_Init(&output);

	for(; i < inputlen; i++) {
		if(count && count == numFinds) {	//found enough, stop looking
			break;
		}

		for(f = 0; f <= findlen; f++) {
			if(f == findlen) { //complete match
				numFinds++;
				StringBuf_AppendStr(&output, replace);

				i += findlen - 1;
				break;
			} else {
				if(usecase) {
					if((i + f) > inputlen || input[i + f] != find[f]) {
						StringBuf_Printf(&output, "%c", input[i]);
						break;
					}
				} else {
					if(((i + f) > inputlen || input[i + f] != find[f]) && TOUPPER(input[i+f]) != TOUPPER(find[f])) {
						StringBuf_Printf(&output, "%c", input[i]);
						break;
					}
				}
			}
		}
	}

	//append excess after enough found
	if(i < inputlen)
		StringBuf_AppendStr(&output, &(input[i]));

	script_pushstrcopy(st, StringBuf_Value(&output));
	StringBuf_Destroy(&output);
	return SCRIPT_CMD_SUCCESS;
}

//========================================================
// countstr <input>, <search>{, <usecase>}
//
// Note: Counts the number of times <search> occurs in
// <input>. By default will be case sensitive.
//--------------------------------------------------------
BUILDIN_FUNC(countstr)
{
	const char *input = script_getstr(st, 2);
	const char *find = script_getstr(st, 3);
	size_t inputlen = strlen(input);
	size_t findlen = strlen(find);
	bool usecase = true;

	int numFinds = 0;
	int i = 0, f = 0;

	if(findlen == 0) {
		ShowError("script:countstr: Invalid search length.\n");
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	if(script_hasdata(st, 4)) {
		struct script_data *data;

		data = script_getdata(st, 4);
		get_val(st, data); // Convert into value in case of a variable
		if( !data_isstring(data) )
			usecase = script_getnum(st, 4) != 0;
		else {
			ShowError("script:countstr: Invalid usecase value. Expected int got string\n");
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}

	for(; i < inputlen; i++) {
		for(f = 0; f <= findlen; f++) {
			if(f == findlen) { //complete match
				numFinds++;
				i += findlen - 1;
				break;
			} else {
				if(usecase) {
					if((i + f) > inputlen || input[i + f] != find[f]) {
						break;
					}
				} else {
					if(((i + f) > inputlen || input[i + f] != find[f]) && TOUPPER(input[i+f]) != TOUPPER(find[f])) {
						break;
					}
				}
			}
		}
	}
	script_pushint(st, numFinds);
	return SCRIPT_CMD_SUCCESS;
}


/// Changes the display name and/or display class of the npc.
/// Returns 0 is successful, 1 if the npc does not exist.
///
/// setnpcdisplay("<npc name>", "<new display name>", <new class id>, <new size>) -> <int>
/// setnpcdisplay("<npc name>", "<new display name>", <new class id>) -> <int>
/// setnpcdisplay("<npc name>", "<new display name>") -> <int>
/// setnpcdisplay("<npc name>", <new class id>) -> <int>
BUILDIN_FUNC(setnpcdisplay)
{
	const char* name;
	const char* newname = NULL;
	int class_ = JT_FAKENPC, size = -1;
	struct script_data* data;
	struct npc_data* nd;

	name = script_getstr(st,2);
	data = script_getdata(st,3);

	if( script_hasdata(st,4) )
		class_ = script_getnum(st,4);
	if( script_hasdata(st,5) )
		size = script_getnum(st,5);

	get_val(st, data);
	if( data_isstring(data) )
 		newname = conv_str(st,data);
	else if( data_isint(data) )
 		class_ = conv_num(st,data);
	else
	{
		ShowError("script:setnpcdisplay: expected string or number\n");
		script_reportdata(data);
		return SCRIPT_CMD_FAILURE;
	}

	nd = npc_name2id(name);
	if( nd == NULL )
	{// not found
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}

	// update npc
	if( newname )
		npc_setdisplayname(nd, newname);

	if( size != -1 && size != (int)nd->size )
		nd->size = size;
	else
		size = -1;

	if( class_ != JT_FAKENPC && nd->class_ != class_ )
		npc_setclass(nd, class_);
	else if( size != -1 )
	{ // Required to update the visual size
		clif_clearunit_area(&nd->bl, CLR_OUTSIGHT);
		clif_spawn(&nd->bl);
	}

	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(atoi)
{
	const char *value;
	value = script_getstr(st,2);
	script_pushint(st,atoi(value));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(axtoi)
{
	const char *hex = script_getstr(st,2);
	long value = strtol(hex, NULL, 16);
#if LONG_MAX > INT_MAX || LONG_MIN < INT_MIN
	value = cap_value(value, INT_MIN, INT_MAX);
#endif
	script_pushint(st, (int)value);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(strtol)
{
	const char *string = script_getstr(st, 2);
	int base = script_getnum(st, 3);
	long value = strtol(string, NULL, base);
#if LONG_MAX > INT_MAX || LONG_MIN < INT_MIN
	value = cap_value(value, INT_MIN, INT_MAX);
#endif
	script_pushint(st, (int)value);
	return SCRIPT_CMD_SUCCESS;
}

// case-insensitive substring search [lordalfa]
BUILDIN_FUNC(compare)
{
	const char *message;
	const char *cmpstring;
	message = script_getstr(st,2);
	cmpstring = script_getstr(st,3);
	script_pushint(st,(stristr(message,cmpstring) != NULL));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(strcmp)
{
	const char *str1;
	const char *str2;
	str1 = script_getstr(st,2);
	str2 = script_getstr(st,3);
	script_pushint(st,strcmp(str1, str2));
	return SCRIPT_CMD_SUCCESS;
}

// [zBuffer] List of mathematics commands --->
BUILDIN_FUNC(sqrt)
{
	double i, a;
	i = script_getnum(st,2);
	a = sqrt(i);
	script_pushint(st,(int)a);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(pow)
{
	double i, a, b;
	a = script_getnum(st,2);
	b = script_getnum(st,3);
	i = pow(a,b);
	script_pushint(st,(int)i);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(distance)
{
	int x0, y0, x1, y1;

	x0 = script_getnum(st,2);
	y0 = script_getnum(st,3);
	x1 = script_getnum(st,4);
	y1 = script_getnum(st,5);

	script_pushint(st,distance_xy(x0,y0,x1,y1));
	return SCRIPT_CMD_SUCCESS;
}

// <--- [zBuffer] List of mathematics commands

BUILDIN_FUNC(md5)
{
	const char *tmpstr;
	char *md5str;

	tmpstr = script_getstr(st,2);
	md5str = (char *)aMalloc((32+1)*sizeof(char));
	MD5_String(tmpstr, md5str);
	script_pushstr(st, md5str);
	return SCRIPT_CMD_SUCCESS;
}

// [zBuffer] List of dynamic var commands --->
/**
 * setd "<variable name>",<value>{,<char_id>};
 **/
BUILDIN_FUNC(setd)
{
	TBL_PC *sd = NULL;
	char varname[100];
	const char *buffer;
	int elem;
	buffer = script_getstr(st, 2);

	if(sscanf(buffer, "%99[^[][%11d]", varname, &elem) < 2)
		elem = 0;

	if( not_server_variable(*varname) ) {
		if (!script_charid2sd(4,sd))
			return SCRIPT_CMD_FAILURE;
	}

	if( is_string_variable(varname) ) {
		setd_sub(st, sd, varname, elem, (void *)script_getstr(st, 3), NULL);
	} else {
		setd_sub(st, sd, varname, elem, (void *)__64BPRTSIZE(script_getnum(st, 3)), NULL);
	}

	return SCRIPT_CMD_SUCCESS;
}

int buildin_query_sql_sub(struct script_state* st, Sql* handle)
{
	int i, j;
	TBL_PC* sd = NULL;
	const char* query;
	struct script_data* data;
	const char* name;
	unsigned int max_rows = SCRIPT_MAX_ARRAYSIZE; // maximum number of rows
	int num_vars;
	int num_cols;

	// check target variables
	for( i = 3; script_hasdata(st,i); ++i ) {
		data = script_getdata(st, i);
		if( data_isreference(data) ) { // it's a variable
			name = reference_getname(data);
			if( not_server_variable(*name) && sd == NULL ) { // requires a player
				if( !script_rid2sd(sd) ) { // no player attached
					script_reportdata(data);
					st->state = END;
					return SCRIPT_CMD_FAILURE;
				}
			}
		} else {
			ShowError("script:query_sql: not a variable\n");
			script_reportdata(data);
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}
	}
	num_vars = i - 3;

	// Execute the query
	query = script_getstr(st,2);

	if( SQL_ERROR == Sql_QueryStr(handle, query) ) {
		Sql_ShowDebug(handle);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	if( Sql_NumRows(handle) == 0 ) { // No data received
		Sql_FreeResult(handle);
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	// Count the number of columns to store
	num_cols = Sql_NumColumns(handle);
	if( num_vars < num_cols ) {
		ShowWarning("script:query_sql: Too many columns, discarding last %u columns.\n", (unsigned int)(num_cols-num_vars));
		script_reportsrc(st);
	} else if( num_vars > num_cols ) {
		ShowWarning("script:query_sql: Too many variables (%u extra).\n", (unsigned int)(num_vars-num_cols));
		script_reportsrc(st);
	}

	// Store data
	for( i = 0; i < max_rows && SQL_SUCCESS == Sql_NextRow(handle); ++i ) {
		for( j = 0; j < num_vars; ++j ) {
			char* str = NULL;

			if( j < num_cols )
				Sql_GetData(handle, j, &str, NULL);

			data = script_getdata(st, j+3);
			name = reference_getname(data);
			if( is_string_variable(name) )
				setd_sub(st, sd, name, i, (void *)(str?str:""), reference_getref(data));
			else
				setd_sub(st, sd, name, i, (void *)__64BPRTSIZE((str?atoi(str):0)), reference_getref(data));
		}
	}
	if( i == max_rows && max_rows < Sql_NumRows(handle) ) {
		ShowWarning("script:query_sql: Only %d/%u rows have been stored.\n", max_rows, (unsigned int)Sql_NumRows(handle));
		script_reportsrc(st);
	}

	// Free data
	Sql_FreeResult(handle);
	script_pushint(st, i);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(query_sql) {
	return buildin_query_sql_sub(st, qsmysql_handle);
}

BUILDIN_FUNC(query_logsql) {
	if( !log_config.sql_logs ) {// logmysql_handle == NULL
		ShowWarning("buildin_query_logsql: SQL logs are disabled, query '%s' will not be executed.\n", script_getstr(st,2));
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	return buildin_query_sql_sub(st, logmysql_handle);
}

//Allows escaping of a given string.
BUILDIN_FUNC(escape_sql)
{
	const char *str;
	char *esc_str;
	size_t len;

	str = script_getstr(st,2);
	len = strlen(str);
	esc_str = (char*)aMalloc(len*2+1);
	Sql_EscapeStringLen(mmysql_handle, esc_str, str, len);
	script_pushstr(st, esc_str);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getd)
{
	char varname[100];
	const char *buffer;
	int elem;

	buffer = script_getstr(st, 2);

	if(sscanf(buffer, "%99[^[][%11d]", varname, &elem) < 2)
		elem = 0;

	// Push the 'pointer' so it's more flexible [Lance]
	push_val(st->stack, C_NAME, reference_uid(add_str(varname), elem));

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(callshop)
{
	TBL_PC *sd = NULL;
	struct npc_data *nd;
	const char *shopname;
	int flag = 0;
	if (!script_rid2sd(sd)) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	shopname = script_getstr(st, 2);
	if (script_hasdata(st,3))
		flag = script_getnum(st,3);
	nd = npc_name2id(shopname);
	if( !nd || nd->bl.type != BL_NPC || (nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP) ) {
		ShowError("buildin_callshop: Shop [%s] not found (or NPC is not shop type)\n", shopname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (nd->subtype == NPCTYPE_SHOP) {
		// flag the user as using a valid script call for opening the shop (for floating NPCs)
		sd->state.callshop = 1;

		switch (flag) {
			case 1: npc_buysellsel(sd,nd->bl.id,0); break; //Buy window
			case 2: npc_buysellsel(sd,nd->bl.id,1); break; //Sell window
			default: clif_npcbuysell(sd,nd->bl.id); break; //Show menu
		}
	}
#if PACKETVER >= 20131223
	else if (nd->subtype == NPCTYPE_MARKETSHOP) {
		unsigned short i;

		for (i = 0; i < nd->u.shop.count; i++) {
			if (nd->u.shop.shop_item[i].qty)
				break;
		}

		if (i == nd->u.shop.count) {
			clif_messagecolor(&sd->bl, color_table[COLOR_RED], msg_txt(sd, 534), false, SELF);
			return SCRIPT_CMD_FAILURE;
		}

		sd->npc_shopid = nd->bl.id;
		clif_npc_market_open(sd, nd);
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
#endif
	else
		clif_cashshop_show(sd, nd);

	sd->npc_shopid = nd->bl.id;
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(npcshopitem)
{
	const char* npcname = script_getstr(st, 2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	int amount;
	uint16 offs = 2;

	if( !nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP ) ) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

#if PACKETVER >= 20131223
	if (nd->subtype == NPCTYPE_MARKETSHOP) {
		offs = 3;
		npc_market_delfromsql_(nd->exname, 0, true);
	}
#endif

	// get the count of new entries
	amount = (script_lastdata(st)-2)/offs;

	// generate new shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, amount);
	for (n = 0, i = 3; n < amount; n++, i+=offs) {
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
#if PACKETVER >= 20131223
		if (nd->subtype == NPCTYPE_MARKETSHOP) {
			nd->u.shop.shop_item[n].qty = script_getnum(st,i+2);
			nd->u.shop.shop_item[n].flag = 1;
			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[n]);
		}
#endif
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(npcshopadditem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	uint16 offs = 2, amount;

	if (!nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP)) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (nd->subtype == NPCTYPE_MARKETSHOP)
		offs = 3;

	// get the count of new entries
	amount = (script_lastdata(st)-2)/offs;

#if PACKETVER >= 20131223
	if (nd->subtype == NPCTYPE_MARKETSHOP) {
		for (n = 0, i = 3; n < amount; n++, i += offs) {
			uint16 nameid = script_getnum(st,i), j;

			// Check existing entries
			ARR_FIND(0, nd->u.shop.count, j, nd->u.shop.shop_item[j].nameid == nameid);
			if (j == nd->u.shop.count) {
				RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+1);
				j = nd->u.shop.count;
				nd->u.shop.shop_item[j].flag = 1;
				nd->u.shop.count++;
			}

			nd->u.shop.shop_item[j].nameid = nameid;
			nd->u.shop.shop_item[j].value = script_getnum(st,i+1);
			nd->u.shop.shop_item[j].qty = script_getnum(st,i+2);

			npc_market_tosql(nd->exname, &nd->u.shop.shop_item[j]);
		}
		script_pushint(st,1);
		return SCRIPT_CMD_SUCCESS;
	}
#endif

	// append new items to existing shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+amount);
	for (n = nd->u.shop.count, i = 3; n < nd->u.shop.count+amount; n++, i+=offs)
	{
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(npcshopdelitem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i, size;
	unsigned short amount;

	if (!nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP)) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	amount = script_lastdata(st)-2;
	size = nd->u.shop.count;

	// remove specified items from the shop item list
	for( i = 3; i < 3 + amount; i++ ) {
		unsigned short nameid = script_getnum(st,i);

		ARR_FIND( 0, size, n, nd->u.shop.shop_item[n].nameid == nameid );
		if( n < size ) {
			if (n+1 != size)
				memmove(&nd->u.shop.shop_item[n], &nd->u.shop.shop_item[n+1], sizeof(nd->u.shop.shop_item[0])*(size-n));
#if PACKETVER >= 20131223
			if (nd->subtype == NPCTYPE_MARKETSHOP)
				npc_market_delfromsql_(nd->exname, nameid, false);
#endif
			size--;
		}
	}

	RECREATE(nd->u.shop.shop_item, struct npc_item_list, size);
	nd->u.shop.count = size;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Sets a script to attach to a shop npc.
 * npcshopattach "<npc_name>";
 **/
BUILDIN_FUNC(npcshopattach)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int flag = 1;

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);

	if (!nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP)) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (flag)
		nd->master_nd = ((struct npc_data *)map_id2bl(st->oid));
	else
		nd->master_nd = NULL;

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Returns some values of an item [Lupus]
 * Price, Weight, etc...
	setitemscript(itemID,"{new item bonus script}",[n]);
   Where n:
	0 - script
	1 - Equip script
	2 - Unequip script
 *------------------------------------------*/
BUILDIN_FUNC(setitemscript)
{
	unsigned short item_id;
	int n = 0;
	const char *script;
	struct item_data *i_data;
	struct script_code **dstscript;

	item_id	= script_getnum(st,2);
	script = script_getstr(st,3);
	if( script_hasdata(st,4) )
		n=script_getnum(st,4);
	i_data = itemdb_exists(item_id);

	if (!i_data || script==NULL || ( script[0] && script[0]!='{' )) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	switch (n) {
	case 2:
		dstscript = &i_data->unequip_script;
		break;
	case 1:
		dstscript = &i_data->equip_script;
		break;
	default:
		dstscript = &i_data->script;
		break;
	}
	if(*dstscript)
		script_free_code(*dstscript);

	*dstscript = script[0] ? parse_script(script, "script_setitemscript", 0, 0) : NULL;
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/*=======================================================
 * Add or Update a mob drop temporarily [Akinari]
 * Original Idea By: [Lupus]
 *
 * addmonsterdrop <mob_id or name>,<item_id>,<rate>;
 *
 * If given an item the mob already drops, the rate
 * is updated to the new rate.  Rate cannot exceed 10000
 * Returns 1 if succeeded (added/updated a mob drop)
 *-------------------------------------------------------*/
BUILDIN_FUNC(addmonsterdrop)
{
	struct mob_db *mob;
	struct script_data *data;
	unsigned short item_id;
	int rate;

	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	if(data_isstring(data))
		mob = mob_db(mobdb_searchname(script_getstr(st,2)));
	else
		mob = mob_db(script_getnum(st,2));

	item_id=script_getnum(st,3);
	rate=script_getnum(st,4);

	if(!itemdb_exists(item_id)){
		ShowError("addmonsterdrop: Nonexistant item %hu requested.\n", item_id );
		return SCRIPT_CMD_FAILURE;
	}

	if(mob) { //We got a valid monster, check for available drop slot
		unsigned char i, c = 0;
		for(i = 0; i < MAX_MOB_DROP_TOTAL; i++) {
			if(mob->dropitem[i].nameid) {
				if(mob->dropitem[i].nameid == item_id) { //If it equals item_id we update that drop
					c = i;
					break;
				}
				continue;
			}
			if(!c) //Accept first available slot only
				c = i;
		}
		if(c) { //Fill in the slot with the item and rate
			mob->dropitem[c].nameid = item_id;
			mob->dropitem[c].p = (rate > 10000)?10000:rate;
			mob_reload_itemmob_data(); // Reload the mob search data stored in the item_data
			script_pushint(st,1);
		} else //No place to put the new drop
			script_pushint(st,0);
	} else {
		ShowWarning("addmonsterdrop: bad mob id given %d\n",script_getnum(st,2));
		return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;

}

/*=======================================================
 * Delete a mob drop temporarily [Akinari]
 * Original Idea By: [Lupus]
 *
 * delmonsterdrop <mob_id or name>,<item_id>;
 *
 * Returns 1 if succeeded (deleted a mob drop)
 *-------------------------------------------------------*/
BUILDIN_FUNC(delmonsterdrop)
{
	struct mob_db *mob;
	struct script_data *data;
	unsigned short item_id;

	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	if(data_isstring(data))
		mob = mob_db(mobdb_searchname(script_getstr(st,2)));
	else
		mob = mob_db(script_getnum(st,2));

	item_id=script_getnum(st,3);

	if(!itemdb_exists(item_id)){
		ShowError("delmonsterdrop: Nonexistant item %hu requested.\n", item_id );
		return SCRIPT_CMD_FAILURE;
	}

	if(mob) { //We got a valid monster, check for item drop on monster
		unsigned char i;
		for(i = 0; i < MAX_MOB_DROP_TOTAL; i++) {
			if(mob->dropitem[i].nameid == item_id) {
				mob->dropitem[i].nameid = 0;
				mob->dropitem[i].p = 0;
				mob_reload_itemmob_data(); // Reload the mob search data stored in the item_data
				script_pushint(st,1);
				return SCRIPT_CMD_SUCCESS;
			}
		}
		//No drop on that monster
		script_pushint(st,0);
	} else {
		ShowWarning("delmonsterdrop: bad mob id given %d\n",script_getnum(st,2));
		return SCRIPT_CMD_FAILURE;
	}
	return SCRIPT_CMD_SUCCESS;
}


/*==========================================
 * Returns some values of a monster [Lupus]
 * Name, Level, race, size, etc...
	getmonsterinfo(monsterID,queryIndex);
 *------------------------------------------*/
BUILDIN_FUNC(getmonsterinfo)
{
	struct mob_db *mob;
	int mob_id;

	mob_id	= script_getnum(st,2);
	if (!mobdb_checkid(mob_id)) {
		//ShowError("buildin_getmonsterinfo: Wrong Monster ID: %i\n", mob_id);
		if ( script_getnum(st,3) == MOB_NAME ) // requested the name
			script_pushconststr(st,"null");
		else
			script_pushint(st,-1);
		return SCRIPT_CMD_SUCCESS;
	}
	mob = mob_db(mob_id);
	switch ( script_getnum(st,3) ) {
		case MOB_NAME:		script_pushstrcopy(st,mob->jname); break;
		case MOB_LV:		script_pushint(st,mob->lv); break;
		case MOB_MAXHP:		script_pushint(st,mob->status.max_hp); break;
		case MOB_BASEEXP:	script_pushint(st,mob->base_exp); break;
		case MOB_JOBEXP:	script_pushint(st,mob->job_exp); break;
		case MOB_ATK1:		script_pushint(st,mob->status.rhw.atk); break;
		case MOB_ATK2:		script_pushint(st,mob->status.rhw.atk2); break;
		case MOB_DEF:		script_pushint(st,mob->status.def); break;
		case MOB_MDEF:		script_pushint(st,mob->status.mdef); break;
		case MOB_STR:		script_pushint(st,mob->status.str); break;
		case MOB_AGI:		script_pushint(st,mob->status.agi); break;
		case MOB_VIT:		script_pushint(st,mob->status.vit); break;
		case MOB_INT:		script_pushint(st,mob->status.int_); break;
		case MOB_DEX:		script_pushint(st,mob->status.dex); break;
		case MOB_LUK:		script_pushint(st,mob->status.luk); break;
		case MOB_RANGE:		script_pushint(st,mob->status.rhw.range); break;
		case MOB_RANGE2:	script_pushint(st,mob->range2); break;
		case MOB_RANGE3:	script_pushint(st,mob->range3); break;
		case MOB_SIZE:		script_pushint(st,mob->status.size); break;
		case MOB_RACE:		script_pushint(st,mob->status.race); break;
		case MOB_ELEMENT:	script_pushint(st,mob->status.def_ele); break;
		case MOB_MODE:		script_pushint(st,mob->status.mode); break;
		case MOB_MVPEXP:	script_pushint(st,mob->mexp); break;
		default: script_pushint(st,-1); //wrong Index
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Check player's vending/buyingstore/autotrade state
 * checkvending({"<Player Name>"})
 * @author [Nab4]
 */
BUILDIN_FUNC(checkvending) {
	TBL_PC *sd = NULL;

	if (!script_nick2sd(2,sd) ) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}
	else {
		int8 ret = 0;
		if (sd->state.vending)
			ret = 1;
		else if (sd->state.buyingstore)
			ret = 4;

		if (sd->state.autotrade)
			ret |= 2;
		script_pushint(st, ret);
	}
	return SCRIPT_CMD_SUCCESS;
}


BUILDIN_FUNC(checkchatting) // check chatting [Marka]
{
	TBL_PC *sd = NULL;

	if( script_nick2sd(2,sd) )
		script_pushint(st,(sd->chatID != 0));
	else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(checkidle)
{
	TBL_PC *sd = NULL;

	if( script_nick2sd(2,sd) )
		script_pushint(st, DIFF_TICK(last_tick, sd->idletime));
	else
		script_pushint(st, 0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(searchitem)
{
	struct script_data* data = script_getdata(st, 2);
	const char *itemname = script_getstr(st,3);
	struct item_data *items[MAX_SEARCH];
	int count;

	char* name;
	int32 start;
	int32 id;
	int32 i;
	TBL_PC* sd = NULL;

	if ((items[0] = itemdb_exists(atoi(itemname))))
		count = 1;
	else
		count = itemdb_searchname_array(items, ARRAYLENGTH(items), itemname);

	if (!count) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	if( !data_isreference(data) )
	{
		ShowError("script:searchitem: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);

	if( not_server_variable(*name) && !script_rid2sd(sd) )
	{
		return SCRIPT_CMD_SUCCESS;// no player attached
	}

	if( is_string_variable(name) )
	{// string array
		ShowError("script:searchitem: not an integer array reference\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;// not supported
	}

	for( i = 0; i < count; ++start, ++i )
	{// Set array
		void* v = (void*)__64BPRTSIZE((int)items[i]->nameid);
		set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
	}

	script_pushint(st, count);
	return SCRIPT_CMD_SUCCESS;
}

// [zBuffer] List of player cont commands --->
BUILDIN_FUNC(rid2name)
{
	struct block_list *bl = NULL;
	int rid = script_getnum(st,2);
	if((bl = map_id2bl(rid)))
	{
		switch(bl->type) {
			case BL_MOB: script_pushstrcopy(st,((TBL_MOB*)bl)->name); break;
			case BL_PC:  script_pushstrcopy(st,((TBL_PC*)bl)->status.name); break;
			case BL_NPC: script_pushstrcopy(st,((TBL_NPC*)bl)->exname); break;
			case BL_PET: script_pushstrcopy(st,((TBL_PET*)bl)->pet.name); break;
			case BL_HOM: script_pushstrcopy(st,((TBL_HOM*)bl)->homunculus.name); break;
			case BL_MER: script_pushstrcopy(st,((TBL_MER*)bl)->db->name); break;
			default:
				ShowError("buildin_rid2name: BL type unknown.\n");
				script_pushconststr(st,"");
				break;
		}
	} else {
		ShowError("buildin_rid2name: invalid RID\n");
		script_pushconststr(st,"(null)");
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Toggle a unit from moving.
 * pcblockmove(<unit_id>,<option>);
 */
BUILDIN_FUNC(pcblockmove)
{
	struct block_list *bl = NULL;

	if (script_getnum(st, 2))
		bl = map_id2bl(script_getnum(st,2));
	else
		bl = map_id2bl(st->rid);

	if (bl) {
		struct unit_data *ud = unit_bl2ud(bl);

		if (ud)
			ud->state.blockedmove = script_getnum(st,3) > 0;
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Toggle a unit from casting skills.
 * pcblockskill(<unit_id>,<option>);
 */
BUILDIN_FUNC(pcblockskill)
{
	struct block_list *bl = NULL;

	if (script_getnum(st, 2))
		bl = map_id2bl(script_getnum(st,2));
	else
		bl = map_id2bl(st->rid);

	if (bl) {
		struct unit_data *ud = unit_bl2ud(bl);

		if (ud)
			ud->state.blockedskill = script_getnum(st, 3) > 0;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(pcfollow)
{
	TBL_PC *sd;

	if( script_mapid2sd(2,sd) )
		pc_follow(sd, script_getnum(st,3));

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(pcstopfollow)
{
	TBL_PC *sd;

	if(script_mapid2sd(2,sd))
		pc_stop_following(sd);

	return SCRIPT_CMD_SUCCESS;
}
// <--- [zBuffer] List of player cont commands
// [zBuffer] List of unit control commands --->

/// Checks to see if the unit exists.
///
/// unitexists <unit id>;
BUILDIN_FUNC(unitexists)
{
	struct block_list* bl;

	bl = map_id2bl(script_getnum(st, 2));

	if (!bl)
		script_pushint(st, false);
	else
		script_pushint(st, true);

	return SCRIPT_CMD_SUCCESS;
}

/// Gets the type of the given Game ID.
///
/// getunittype <unit id>;
BUILDIN_FUNC(getunittype)
{
	struct block_list* bl;
	uint8 value = 0;

	if(!script_rid2bl(2,bl))
	{
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	switch (bl->type) {
		case BL_PC:   value = UNITTYPE_PC; break;
		case BL_NPC:  value = UNITTYPE_NPC; break;
		case BL_PET:  value = UNITTYPE_PET; break;
		case BL_MOB:  value = UNITTYPE_MOB; break;
		case BL_HOM:  value = UNITTYPE_HOM; break;
		case BL_MER:  value = UNITTYPE_MER; break;
		case BL_ELEM: value = UNITTYPE_ELEM; break;
		default:      value = -1; break;
	}

	script_pushint(st, value);
	return SCRIPT_CMD_SUCCESS;
}

/// Gets specific live information of a bl.
///
/// getunitdata <unit id>,<arrayname>;
BUILDIN_FUNC(getunitdata)
{
	TBL_PC *sd = st->rid ? map_id2sd(st->rid) : NULL;
	struct block_list* bl;
	TBL_MOB* md = NULL;
	TBL_HOM* hd = NULL;
	TBL_MER* mc = NULL;
	TBL_PET* pd = NULL;
	TBL_ELEM* ed = NULL;
	TBL_NPC* nd = NULL;
	char* name;
	struct script_data *data = script_getdata(st, 3);

	if (!data_isreference(data)) {
		ShowWarning("buildin_getunitdata: Error in argument! Please give a variable to store values in.\n");
		return SCRIPT_CMD_FAILURE;
	}

	if(!script_rid2bl(2,bl))
	{
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	switch (bl->type) {
		case BL_MOB:  md = map_id2md(bl->id); break;
		case BL_HOM:  hd = map_id2hd(bl->id); break;
		case BL_PET:  pd = map_id2pd(bl->id); break;
		case BL_MER:  mc = map_id2mc(bl->id); break;
		case BL_ELEM: ed = map_id2ed(bl->id); break;
		case BL_NPC:  nd = map_id2nd(bl->id); break;
		default:
			ShowWarning("buildin_getunitdata: Invalid object type!\n");
			return SCRIPT_CMD_FAILURE;
	}

	name = reference_getname(data);

#define getunitdata_sub(idx__,var__) setd_sub(st,sd,name,(idx__),(void *)__64BPRTSIZE((int)(var__)),data->ref)

	switch(bl->type) {
		case BL_MOB:
			if (!md) {
				ShowWarning("buildin_getunitdata: Error in finding object BL_MOB!\n");
				return SCRIPT_CMD_FAILURE;
			}
			getunitdata_sub(UMOB_SIZE, md->status.size);
			getunitdata_sub(UMOB_LEVEL, md->level);
			getunitdata_sub(UMOB_HP, md->status.hp);
			getunitdata_sub(UMOB_MAXHP, md->status.max_hp);
			getunitdata_sub(UMOB_MASTERAID, md->master_id);
			getunitdata_sub(UMOB_MAPID, md->bl.m);
			getunitdata_sub(UMOB_X, md->bl.x);
			getunitdata_sub(UMOB_Y, md->bl.y);
			getunitdata_sub(UMOB_SPEED, md->status.speed);
			getunitdata_sub(UMOB_MODE, md->status.mode);
			getunitdata_sub(UMOB_AI, md->special_state.ai);
			getunitdata_sub(UMOB_SCOPTION, md->sc.option);
			getunitdata_sub(UMOB_SEX, md->vd->sex);
			getunitdata_sub(UMOB_CLASS, md->vd->class_);
			getunitdata_sub(UMOB_HAIRSTYLE, md->vd->hair_style);
			getunitdata_sub(UMOB_HAIRCOLOR, md->vd->hair_color);
			getunitdata_sub(UMOB_HEADBOTTOM, md->vd->head_bottom);
			getunitdata_sub(UMOB_HEADMIDDLE, md->vd->head_mid);
			getunitdata_sub(UMOB_HEADTOP, md->vd->head_top);
			getunitdata_sub(UMOB_CLOTHCOLOR, md->vd->cloth_color);
			getunitdata_sub(UMOB_SHIELD, md->vd->shield);
			getunitdata_sub(UMOB_WEAPON, md->vd->weapon);
			getunitdata_sub(UMOB_LOOKDIR, md->ud.dir);
			getunitdata_sub(UMOB_CANMOVETICK, md->ud.canmove_tick);
			getunitdata_sub(UMOB_STR, md->status.str);
			getunitdata_sub(UMOB_AGI, md->status.agi);
			getunitdata_sub(UMOB_VIT, md->status.vit);
			getunitdata_sub(UMOB_INT, md->status.int_);
			getunitdata_sub(UMOB_DEX, md->status.dex);
			getunitdata_sub(UMOB_LUK, md->status.luk);
			getunitdata_sub(UMOB_SLAVECPYMSTRMD, md->state.copy_master_mode);
			getunitdata_sub(UMOB_DMGIMMUNE, md->ud.immune_attack);
			getunitdata_sub(UMOB_ATKRANGE, md->status.rhw.range);
			getunitdata_sub(UMOB_ATKMIN, md->status.rhw.atk);
			getunitdata_sub(UMOB_ATKMAX, md->status.rhw.atk2);
			getunitdata_sub(UMOB_MATKMIN, md->status.matk_min);
			getunitdata_sub(UMOB_MATKMAX, md->status.matk_max);
			getunitdata_sub(UMOB_DEF, md->status.def);
			getunitdata_sub(UMOB_MDEF, md->status.mdef);
			getunitdata_sub(UMOB_HIT, md->status.hit);
			getunitdata_sub(UMOB_FLEE, md->status.flee);
			getunitdata_sub(UMOB_PDODGE, md->status.flee2);
			getunitdata_sub(UMOB_CRIT, md->status.cri);
			getunitdata_sub(UMOB_RACE, md->status.race);
			getunitdata_sub(UMOB_ELETYPE, md->status.def_ele);
			getunitdata_sub(UMOB_ELELEVEL, md->status.ele_lv);
			getunitdata_sub(UMOB_AMOTION, md->status.amotion);
			getunitdata_sub(UMOB_ADELAY, md->status.adelay);
			getunitdata_sub(UMOB_DMOTION, md->status.dmotion);
			getunitdata_sub(UMOB_TARGETID, md->target_id);
			break;

		case BL_HOM:
			if (!hd) {
				ShowWarning("buildin_getunitdata: Error in finding object BL_HOM!\n");
				return SCRIPT_CMD_FAILURE;
			}
			getunitdata_sub(UHOM_SIZE, hd->base_status.size);
			getunitdata_sub(UHOM_LEVEL, hd->homunculus.level);
			getunitdata_sub(UHOM_HP, hd->homunculus.hp);
			getunitdata_sub(UHOM_MAXHP, hd->homunculus.max_hp);
			getunitdata_sub(UHOM_SP, hd->homunculus.sp);
			getunitdata_sub(UHOM_MAXSP, hd->homunculus.max_sp);
			getunitdata_sub(UHOM_MASTERCID, hd->homunculus.char_id);
			getunitdata_sub(UHOM_MAPID, hd->bl.m);
			getunitdata_sub(UHOM_X, hd->bl.x);
			getunitdata_sub(UHOM_Y, hd->bl.y);
			getunitdata_sub(UHOM_HUNGER, hd->homunculus.hunger);
			getunitdata_sub(UHOM_INTIMACY, hd->homunculus.intimacy);
			getunitdata_sub(UHOM_SPEED, hd->base_status.speed);
			getunitdata_sub(UHOM_LOOKDIR, hd->ud.dir);
			getunitdata_sub(UHOM_CANMOVETICK, hd->ud.canmove_tick);
			getunitdata_sub(UHOM_STR, hd->base_status.str);
			getunitdata_sub(UHOM_AGI, hd->base_status.agi);
			getunitdata_sub(UHOM_VIT, hd->base_status.vit);
			getunitdata_sub(UHOM_INT, hd->base_status.int_);
			getunitdata_sub(UHOM_DEX, hd->base_status.dex);
			getunitdata_sub(UHOM_LUK, hd->base_status.luk);
			getunitdata_sub(UHOM_DMGIMMUNE, hd->ud.immune_attack);
			getunitdata_sub(UHOM_ATKRANGE, hd->battle_status.rhw.range);
			getunitdata_sub(UHOM_ATKMIN, hd->base_status.rhw.atk);
			getunitdata_sub(UHOM_ATKMAX, hd->base_status.rhw.atk2);
			getunitdata_sub(UHOM_MATKMIN, hd->base_status.matk_min);
			getunitdata_sub(UHOM_MATKMAX, hd->base_status.matk_max);
			getunitdata_sub(UHOM_DEF, hd->battle_status.def);
			getunitdata_sub(UHOM_MDEF, hd->battle_status.mdef);
			getunitdata_sub(UHOM_HIT, hd->battle_status.hit);
			getunitdata_sub(UHOM_FLEE, hd->battle_status.flee);
			getunitdata_sub(UHOM_PDODGE, hd->battle_status.flee2);
			getunitdata_sub(UHOM_CRIT, hd->battle_status.cri);
			getunitdata_sub(UHOM_RACE, hd->battle_status.race);
			getunitdata_sub(UHOM_ELETYPE, hd->battle_status.def_ele);
			getunitdata_sub(UHOM_ELELEVEL, hd->battle_status.ele_lv);
			getunitdata_sub(UHOM_AMOTION, hd->battle_status.amotion);
			getunitdata_sub(UHOM_ADELAY, hd->battle_status.adelay);
			getunitdata_sub(UHOM_DMOTION, hd->battle_status.dmotion);
			getunitdata_sub(UHOM_TARGETID, hd->ud.target);
			break;

		case BL_PET:
			if (!pd) {
				ShowWarning("buildin_getunitdata: Error in finding object BL_PET!\n");
				return SCRIPT_CMD_FAILURE;
			}
			getunitdata_sub(UPET_SIZE, pd->status.size);
			getunitdata_sub(UPET_LEVEL, pd->pet.level);
			getunitdata_sub(UPET_HP, pd->status.hp);
			getunitdata_sub(UPET_MAXHP, pd->status.max_hp);
			getunitdata_sub(UPET_MASTERAID, pd->pet.account_id);
			getunitdata_sub(UPET_MAPID, pd->bl.m);
			getunitdata_sub(UPET_X, pd->bl.x);
			getunitdata_sub(UPET_Y, pd->bl.y);
			getunitdata_sub(UPET_HUNGER, pd->pet.hungry);
			getunitdata_sub(UPET_INTIMACY, pd->pet.intimate);
			getunitdata_sub(UPET_SPEED, pd->status.speed);
			getunitdata_sub(UPET_LOOKDIR, pd->ud.dir);
			getunitdata_sub(UPET_CANMOVETICK, pd->ud.canmove_tick);
			getunitdata_sub(UPET_STR, pd->status.str);
			getunitdata_sub(UPET_AGI, pd->status.agi);
			getunitdata_sub(UPET_VIT, pd->status.vit);
			getunitdata_sub(UPET_INT, pd->status.int_);
			getunitdata_sub(UPET_DEX, pd->status.dex);
			getunitdata_sub(UPET_LUK, pd->status.luk);
			getunitdata_sub(UPET_DMGIMMUNE, pd->ud.immune_attack);
			getunitdata_sub(UPET_ATKRANGE, pd->status.rhw.range);
			getunitdata_sub(UPET_ATKMIN, pd->status.rhw.atk);
			getunitdata_sub(UPET_ATKMAX, pd->status.rhw.atk2);
			getunitdata_sub(UPET_MATKMIN, pd->status.matk_min);
			getunitdata_sub(UPET_MATKMAX, pd->status.matk_max);
			getunitdata_sub(UPET_DEF, pd->status.def);
			getunitdata_sub(UPET_MDEF, pd->status.mdef);
			getunitdata_sub(UPET_HIT, pd->status.hit);
			getunitdata_sub(UPET_FLEE, pd->status.flee);
			getunitdata_sub(UPET_PDODGE, pd->status.flee2);
			getunitdata_sub(UPET_CRIT, pd->status.cri);
			getunitdata_sub(UPET_RACE, pd->status.race);
			getunitdata_sub(UPET_ELETYPE, pd->status.def_ele);
			getunitdata_sub(UPET_ELELEVEL, pd->status.ele_lv);
			getunitdata_sub(UPET_AMOTION, pd->status.amotion);
			getunitdata_sub(UPET_ADELAY, pd->status.adelay);
			getunitdata_sub(UPET_DMOTION, pd->status.dmotion);
			break;

		case BL_MER:
			if (!mc) {
				ShowWarning("buildin_getunitdata: Error in finding object BL_MER!\n");
				return SCRIPT_CMD_FAILURE;
			}
			getunitdata_sub(UMER_SIZE, mc->base_status.size);
			getunitdata_sub(UMER_HP, mc->base_status.hp);
			getunitdata_sub(UMER_MAXHP, mc->base_status.max_hp);
			getunitdata_sub(UMER_MASTERCID, mc->mercenary.char_id);
			getunitdata_sub(UMER_MAPID, mc->bl.m);
			getunitdata_sub(UMER_X, mc->bl.x);
			getunitdata_sub(UMER_Y, mc->bl.y);
			getunitdata_sub(UMER_KILLCOUNT, mc->mercenary.kill_count);
			getunitdata_sub(UMER_LIFETIME, mc->mercenary.life_time);
			getunitdata_sub(UMER_SPEED, mc->base_status.speed);
			getunitdata_sub(UMER_LOOKDIR, mc->ud.dir);
			getunitdata_sub(UMER_CANMOVETICK, mc->ud.canmove_tick);
			getunitdata_sub(UMER_STR, mc->base_status.str);
			getunitdata_sub(UMER_AGI, mc->base_status.agi);
			getunitdata_sub(UMER_VIT, mc->base_status.vit);
			getunitdata_sub(UMER_INT, mc->base_status.int_);
			getunitdata_sub(UMER_DEX, mc->base_status.dex);
			getunitdata_sub(UMER_LUK, mc->base_status.luk);
			getunitdata_sub(UMER_DMGIMMUNE, mc->ud.immune_attack);
			getunitdata_sub(UMER_ATKRANGE, mc->base_status.rhw.range);
			getunitdata_sub(UMER_ATKMIN, mc->base_status.rhw.atk);
			getunitdata_sub(UMER_ATKMAX, mc->base_status.rhw.atk2);
			getunitdata_sub(UMER_MATKMIN, mc->base_status.matk_min);
			getunitdata_sub(UMER_MATKMAX, mc->base_status.matk_max);
			getunitdata_sub(UMER_DEF, mc->base_status.def);
			getunitdata_sub(UMER_MDEF, mc->base_status.mdef);
			getunitdata_sub(UMER_HIT, mc->base_status.hit);
			getunitdata_sub(UMER_FLEE, mc->base_status.flee);
			getunitdata_sub(UMER_PDODGE, mc->base_status.flee2);
			getunitdata_sub(UMER_CRIT, mc->base_status.cri);
			getunitdata_sub(UMER_RACE, mc->base_status.race);
			getunitdata_sub(UMER_ELETYPE, mc->base_status.def_ele);
			getunitdata_sub(UMER_ELELEVEL, mc->base_status.ele_lv);
			getunitdata_sub(UMER_AMOTION, mc->base_status.amotion);
			getunitdata_sub(UMER_ADELAY, mc->base_status.adelay);
			getunitdata_sub(UMER_DMOTION, mc->base_status.dmotion);
			getunitdata_sub(UMER_TARGETID, mc->ud.target);
			break;

		case BL_ELEM:
			if (!ed) {
				ShowWarning("buildin_getunitdata: Error in finding object BL_ELEM!\n");
				return SCRIPT_CMD_FAILURE;
			}
			getunitdata_sub(UELE_SIZE, ed->base_status.size);
			getunitdata_sub(UELE_HP, ed->elemental.hp);
			getunitdata_sub(UELE_MAXHP, ed->elemental.max_hp);
			getunitdata_sub(UELE_SP, ed->elemental.sp);
			getunitdata_sub(UELE_MAXSP, ed->elemental.max_sp);
			getunitdata_sub(UELE_MASTERCID, ed->elemental.char_id);
			getunitdata_sub(UELE_MAPID, ed->bl.m);
			getunitdata_sub(UELE_X, ed->bl.x);
			getunitdata_sub(UELE_Y, ed->bl.y);
			getunitdata_sub(UELE_LIFETIME, ed->elemental.life_time);
			getunitdata_sub(UELE_MODE, ed->elemental.mode);
			getunitdata_sub(UELE_SP, ed->base_status.speed);
			getunitdata_sub(UELE_LOOKDIR, ed->ud.dir);
			getunitdata_sub(UELE_CANMOVETICK, ed->ud.canmove_tick);
			getunitdata_sub(UELE_STR, ed->base_status.str);
			getunitdata_sub(UELE_AGI, ed->base_status.agi);
			getunitdata_sub(UELE_VIT, ed->base_status.vit);
			getunitdata_sub(UELE_INT, ed->base_status.int_);
			getunitdata_sub(UELE_DEX, ed->base_status.dex);
			getunitdata_sub(UELE_LUK, ed->base_status.luk);
			getunitdata_sub(UELE_DMGIMMUNE, ed->ud.immune_attack);
			getunitdata_sub(UELE_ATKRANGE, ed->base_status.rhw.range);
			getunitdata_sub(UELE_ATKMIN, ed->base_status.rhw.atk);
			getunitdata_sub(UELE_ATKMAX, ed->base_status.rhw.atk2);
			getunitdata_sub(UELE_MATKMIN, ed->base_status.matk_min);
			getunitdata_sub(UELE_MATKMAX, ed->base_status.matk_max);
			getunitdata_sub(UELE_DEF, ed->base_status.def);
			getunitdata_sub(UELE_MDEF, ed->base_status.mdef);
			getunitdata_sub(UELE_HIT, ed->base_status.hit);
			getunitdata_sub(UELE_FLEE, ed->base_status.flee);
			getunitdata_sub(UELE_PDODGE, ed->base_status.flee2);
			getunitdata_sub(UELE_CRIT, ed->base_status.cri);
			getunitdata_sub(UELE_RACE, ed->base_status.race);
			getunitdata_sub(UELE_ELETYPE, ed->base_status.def_ele);
			getunitdata_sub(UELE_ELELEVEL, ed->base_status.ele_lv);
			getunitdata_sub(UELE_AMOTION, ed->base_status.amotion);
			getunitdata_sub(UELE_ADELAY, ed->base_status.adelay);
			getunitdata_sub(UELE_DMOTION, ed->base_status.dmotion);
			getunitdata_sub(UELE_TARGETID, ed->ud.target);
			break;

		case BL_NPC:
			if (!nd) {
				ShowWarning("buildin_getunitdata: Error in finding object BL_NPC!\n");
				return SCRIPT_CMD_FAILURE;
			}
			getunitdata_sub(UNPC_DISPLAY, nd->class_);
			getunitdata_sub(UNPC_LEVEL, nd->level);
			getunitdata_sub(UNPC_HP, nd->status.hp);
			getunitdata_sub(UNPC_MAXHP, nd->status.max_hp);
			getunitdata_sub(UNPC_MAPID, nd->bl.m);
			getunitdata_sub(UNPC_X, nd->bl.x);
			getunitdata_sub(UNPC_Y, nd->bl.y);
			getunitdata_sub(UNPC_LOOKDIR, nd->ud.dir);
			getunitdata_sub(UNPC_STR, nd->status.str);
			getunitdata_sub(UNPC_AGI, nd->status.agi);
			getunitdata_sub(UNPC_VIT, nd->status.vit);
			getunitdata_sub(UNPC_INT, nd->status.int_);
			getunitdata_sub(UNPC_DEX, nd->status.dex);
			getunitdata_sub(UNPC_LUK, nd->status.luk);
			getunitdata_sub(UNPC_PLUSALLSTAT, nd->stat_point);
			getunitdata_sub(UNPC_DMGIMMUNE, nd->ud.immune_attack);
			getunitdata_sub(UNPC_ATKRANGE, nd->status.rhw.range);
			getunitdata_sub(UNPC_ATKMIN, nd->status.rhw.atk);
			getunitdata_sub(UNPC_ATKMAX, nd->status.rhw.atk2);
			getunitdata_sub(UNPC_MATKMIN, nd->status.matk_min);
			getunitdata_sub(UNPC_MATKMAX, nd->status.matk_max);
			getunitdata_sub(UNPC_DEF, nd->status.def);
			getunitdata_sub(UNPC_MDEF, nd->status.mdef);
			getunitdata_sub(UNPC_HIT, nd->status.hit);
			getunitdata_sub(UNPC_FLEE, nd->status.flee);
			getunitdata_sub(UNPC_PDODGE, nd->status.flee2);
			getunitdata_sub(UNPC_CRIT, nd->status.cri);
			getunitdata_sub(UNPC_RACE, nd->status.race);
			getunitdata_sub(UNPC_ELETYPE, nd->status.def_ele);
			getunitdata_sub(UNPC_ELELEVEL, nd->status.ele_lv);
			getunitdata_sub(UNPC_AMOTION,  nd->status.amotion);
			getunitdata_sub(UNPC_ADELAY, nd->status.adelay);
			getunitdata_sub(UNPC_DMOTION, nd->status.dmotion);
			break;

		default:
			ShowWarning("buildin_getunitdata: Unknown object type!\n");
			return SCRIPT_CMD_FAILURE;
	}

#undef getunitdata_sub

	return SCRIPT_CMD_SUCCESS;
}

/// Changes the live data of a bl.
///
/// setunitdata <unit id>,<type>,<value>;
BUILDIN_FUNC(setunitdata)
{
	struct block_list* bl = NULL;
	struct script_data* data;
	const char *mapname = NULL;
	TBL_MOB* md = NULL;
	TBL_HOM* hd = NULL;
	TBL_MER* mc = NULL;
	TBL_PET* pd = NULL;
	TBL_ELEM* ed = NULL;
	TBL_NPC* nd = NULL;
	int type, value = 0;
	bool calc_status = false;

	if(!script_rid2bl(2,bl))
	{
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	switch (bl->type) {
		case BL_MOB:  md = map_id2md(bl->id); break;
		case BL_HOM:  hd = map_id2hd(bl->id); break;
		case BL_PET:  pd = map_id2pd(bl->id); break;
		case BL_MER:  mc = map_id2mc(bl->id); break;
		case BL_ELEM: ed = map_id2ed(bl->id); break;
		case BL_NPC:
			nd = map_id2nd(bl->id);
			if (!nd->status.hp)
				status_calc_npc(nd, SCO_FIRST);
			else
				status_calc_npc(nd, SCO_NONE);
			break;
		default:
			ShowError("buildin_setunitdata: Invalid object!\n");
			return SCRIPT_CMD_FAILURE;
	}

	type = script_getnum(st, 3);
	data = script_getdata(st, 4);
	get_val(st, data);

	if (type == 5 && data_isstring(data))
		mapname = conv_str(st, data);
	else if (data_isint(data))
		value = conv_num(st, data);
	else {
		ShowError("buildin_setunitdata: Invalid data type for argument #3 (%d).\n", data->type);
		return SCRIPT_CMD_FAILURE;
	}

	switch (bl->type) {
	case BL_MOB:
		if (!md) {
			ShowWarning("buildin_setunitdata: Error in finding object BL_MOB!\n");
			return SCRIPT_CMD_FAILURE;
		}
		if (!md->base_status) {
			md->base_status = (struct status_data*)aCalloc(1, sizeof(struct status_data));
			memcpy(md->base_status, &md->db->status, sizeof(struct status_data));
		}

		// Check if the view data will be modified
		switch( type ){
			case UMOB_SEX:
			//case UMOB_CLASS: // Called by status_set_viewdata
			case UMOB_HAIRSTYLE:
			case UMOB_HAIRCOLOR:
			case UMOB_HEADBOTTOM:
			case UMOB_HEADMIDDLE:
			case UMOB_HEADTOP:
			case UMOB_CLOTHCOLOR:
			case UMOB_SHIELD:
			case UMOB_WEAPON:
				mob_set_dynamic_viewdata( md );
				break;
		}

		switch (type) {
			case UMOB_SIZE: md->base_status->size = (unsigned char)value; calc_status = true; break;
			case UMOB_LEVEL: md->level = (unsigned short)value; break;
			case UMOB_HP: md->base_status->hp = (unsigned int)value; status_set_hp(bl, (unsigned int)value, 0); clif_name_area(&md->bl); break;
			case UMOB_MAXHP: md->base_status->max_hp = (unsigned int)value; status_set_maxhp(bl, (unsigned int)value, 0); clif_name_area(&md->bl); break;
			case UMOB_MASTERAID: md->master_id = value; break;
			case UMOB_MAPID: if (mapname) value = map_mapname2mapid(mapname); unit_warp(bl, (short)value, 0, 0, CLR_TELEPORT); break;
			case UMOB_X: if (!unit_walktoxy(bl, (short)value, md->bl.y, 2)) unit_movepos(bl, (short)value, md->bl.y, 0, 0); break;
			case UMOB_Y: if (!unit_walktoxy(bl, md->bl.x, (short)value, 2)) unit_movepos(bl, md->bl.x, (short)value, 0, 0); break;
			case UMOB_SPEED: md->base_status->speed = (unsigned short)value; status_calc_misc(bl, &md->status, md->level); calc_status = true; break;
			case UMOB_MODE: md->base_status->mode = (enum e_mode)value; calc_status = true; break;
			case UMOB_AI: md->special_state.ai = (enum mob_ai)value; break;
			case UMOB_SCOPTION: md->sc.option = (unsigned short)value; break;
			case UMOB_SEX: md->vd->sex = (char)value; clif_clearunit_area(bl, CLR_OUTSIGHT); clif_spawn(bl); break;
			case UMOB_CLASS: status_set_viewdata(bl, (unsigned short)value); clif_clearunit_area(bl, CLR_OUTSIGHT); clif_spawn(bl); break;
			case UMOB_HAIRSTYLE: clif_changelook(bl, LOOK_HAIR, (unsigned short)value); break;
			case UMOB_HAIRCOLOR: clif_changelook(bl, LOOK_HAIR_COLOR, (unsigned short)value); break;
			case UMOB_HEADBOTTOM: clif_changelook(bl, LOOK_HEAD_BOTTOM, (unsigned short)value); break;
			case UMOB_HEADMIDDLE: clif_changelook(bl, LOOK_HEAD_MID, (unsigned short)value); break;
			case UMOB_HEADTOP: clif_changelook(bl, LOOK_HEAD_TOP, (unsigned short)value); break;
			case UMOB_CLOTHCOLOR: clif_changelook(bl, LOOK_CLOTHES_COLOR, (unsigned short)value); break;
			case UMOB_SHIELD: clif_changelook(bl, LOOK_SHIELD, (unsigned short)value); break;
			case UMOB_WEAPON: clif_changelook(bl, LOOK_WEAPON, (unsigned short)value); break;
			case UMOB_LOOKDIR: unit_setdir(bl, (uint8)value); break;
			case UMOB_CANMOVETICK: md->ud.canmove_tick = value > 0 ? (unsigned int)value : 0; break;
			case UMOB_STR: md->base_status->str = (unsigned short)value; status_calc_misc(bl, &md->status, md->level); calc_status = true; break;
			case UMOB_AGI: md->base_status->agi = (unsigned short)value; status_calc_misc(bl, &md->status, md->level); calc_status = true; break;
			case UMOB_VIT: md->base_status->vit = (unsigned short)value; status_calc_misc(bl, &md->status, md->level); calc_status = true; break;
			case UMOB_INT: md->base_status->int_ = (unsigned short)value; status_calc_misc(bl, &md->status, md->level); calc_status = true; break;
			case UMOB_DEX: md->base_status->dex = (unsigned short)value; status_calc_misc(bl, &md->status, md->level); calc_status = true; break;
			case UMOB_LUK: md->base_status->luk = (unsigned short)value; status_calc_misc(bl, &md->status, md->level); calc_status = true; break;
			case UMOB_SLAVECPYMSTRMD:
				if (value > 0) {
					TBL_MOB *md2 = NULL;
					if (!md->master_id || !(md2 = map_id2md(md->master_id))) {
						ShowWarning("buildin_setunitdata: Trying to set UMOB_SLAVECPYMSTRMD on mob without master!\n");
						break;
					}
					md->base_status->mode = md2->status.mode;
					md->state.copy_master_mode = 1;
				} else
					md->state.copy_master_mode = 0;
				calc_status = true;
				break;
			case UMOB_DMGIMMUNE: md->ud.immune_attack = value > 0; break;
			case UMOB_ATKRANGE: md->base_status->rhw.range = (unsigned short)value; calc_status = true; break;
			case UMOB_ATKMIN: md->base_status->rhw.atk = (unsigned short)value; calc_status = true; break;
			case UMOB_ATKMAX: md->base_status->rhw.atk2 = (unsigned short)value; calc_status = true; break;
			case UMOB_MATKMIN: md->base_status->matk_min = (unsigned short)value; calc_status = true; break;
			case UMOB_MATKMAX: md->base_status->matk_max = (unsigned short)value; calc_status = true; break;
			case UMOB_DEF: md->base_status->def = (defType)value; calc_status = true; break;
			case UMOB_MDEF: md->base_status->mdef = (defType)value; calc_status = true; break;
			case UMOB_HIT: md->base_status->hit = (short)value; calc_status = true; break;
			case UMOB_FLEE: md->base_status->flee = (short)value; calc_status = true; break;
			case UMOB_PDODGE: md->base_status->flee2 = (short)value; calc_status = true; break;
			case UMOB_CRIT: md->base_status->cri = (short)value; calc_status = true; break;
			case UMOB_RACE: md->base_status->race = (unsigned char)value; calc_status = true; break;
			case UMOB_ELETYPE: md->base_status->def_ele = (unsigned char)value; calc_status = true; break;
			case UMOB_ELELEVEL: md->base_status->ele_lv = (unsigned char)value; calc_status = true; break;
			case UMOB_AMOTION: md->base_status->amotion = (short)value; calc_status = true; break;
			case UMOB_ADELAY: md->base_status->adelay = (short)value; calc_status = true; break;
			case UMOB_DMOTION: md->base_status->dmotion = (short)value; calc_status = true; break;
			case UMOB_TARGETID: {
				struct block_list* target = map_id2bl(value);
				if (!target) {
					ShowWarning("buildin_setunitdata: Error in finding target for BL_MOB!\n");
					return SCRIPT_CMD_FAILURE;
				}
				mob_target(md,target,0);
				break;
			}
			default:
				ShowError("buildin_setunitdata: Unknown data identifier %d for BL_MOB.\n", type);
				return SCRIPT_CMD_FAILURE;
			}
			if (calc_status)
				status_calc_bl(&md->bl, SCB_BATTLE);
		break;

	case BL_HOM:
		if (!hd) {
			ShowWarning("buildin_setunitdata: Error in finding object BL_HOM!\n");
			return SCRIPT_CMD_FAILURE;
		}
		switch (type) {
			case UHOM_SIZE: hd->base_status.size = (unsigned char)value; calc_status = true; break;
			case UHOM_LEVEL: hd->homunculus.level = (unsigned short)value; break;
			case UHOM_HP: hd->base_status.hp = (unsigned int)value; status_set_hp(bl, (unsigned int)value, 0); break;
			case UHOM_MAXHP: hd->base_status.max_hp = (unsigned int)value; status_set_maxhp(bl, (unsigned int)value, 0); break;
			case UHOM_SP: hd->base_status.sp = (unsigned int)value; status_set_sp(bl, (unsigned int)value, 0); break;
			case UHOM_MAXSP: hd->base_status.max_sp = (unsigned int)value; status_set_maxsp(bl, (unsigned int)value, 0); break;
			case UHOM_MASTERCID: hd->homunculus.char_id = (uint32)value; break;
			case UHOM_MAPID: if (mapname) value = map_mapname2mapid(mapname); unit_warp(bl, (short)value, 0, 0, CLR_TELEPORT); break;
			case UHOM_X: if (!unit_walktoxy(bl, (short)value, hd->bl.y, 2)) unit_movepos(bl, (short)value, hd->bl.y, 0, 0); break;
			case UHOM_Y: if (!unit_walktoxy(bl, hd->bl.x, (short)value, 2)) unit_movepos(bl, hd->bl.x, (short)value, 0, 0); break;
			case UHOM_HUNGER: hd->homunculus.hunger = (short)value; clif_send_homdata(map_charid2sd(hd->homunculus.char_id), SP_HUNGRY, hd->homunculus.hunger); break;
			case UHOM_INTIMACY: hom_increase_intimacy(hd, (unsigned int)value); clif_send_homdata(map_charid2sd(hd->homunculus.char_id), SP_INTIMATE, hd->homunculus.intimacy / 100); break;
			case UHOM_SPEED: hd->base_status.speed = (unsigned short)value; status_calc_misc(bl, &hd->base_status, hd->homunculus.level); calc_status = true; break;
			case UHOM_LOOKDIR: unit_setdir(bl, (uint8)value); break;
			case UHOM_CANMOVETICK: hd->ud.canmove_tick = value > 0 ? (unsigned int)value : 0; break;
			case UHOM_STR: hd->base_status.str = (unsigned short)value; status_calc_misc(bl, &hd->base_status, hd->homunculus.level); calc_status = true; break;
			case UHOM_AGI: hd->base_status.agi = (unsigned short)value; status_calc_misc(bl, &hd->base_status, hd->homunculus.level); calc_status = true; break;
			case UHOM_VIT: hd->base_status.vit = (unsigned short)value; status_calc_misc(bl, &hd->base_status, hd->homunculus.level); calc_status = true; break;
			case UHOM_INT: hd->base_status.int_ = (unsigned short)value; status_calc_misc(bl, &hd->base_status, hd->homunculus.level); calc_status = true; break;
			case UHOM_DEX: hd->base_status.dex = (unsigned short)value; status_calc_misc(bl, &hd->base_status, hd->homunculus.level); calc_status = true; break;
			case UHOM_LUK: hd->base_status.luk = (unsigned short)value; status_calc_misc(bl, &hd->base_status, hd->homunculus.level); calc_status = true; break;
			case UHOM_DMGIMMUNE: hd->ud.immune_attack = value > 0; break;
			case UHOM_ATKRANGE: hd->base_status.rhw.range = (unsigned short)value; calc_status = true; break;
			case UHOM_ATKMIN: hd->base_status.rhw.atk = (unsigned short)value; calc_status = true; break;
			case UHOM_ATKMAX: hd->base_status.rhw.atk2 = (unsigned short)value; calc_status = true; break;
			case UHOM_MATKMIN: hd->base_status.matk_min = (unsigned short)value; calc_status = true; break;
			case UHOM_MATKMAX: hd->base_status.matk_max = (unsigned short)value; calc_status = true; break;
			case UHOM_DEF: hd->base_status.def = (defType)value; calc_status = true; break;
			case UHOM_MDEF: hd->base_status.mdef = (defType)value; calc_status = true; break;
			case UHOM_HIT: hd->base_status.hit = (short)value; calc_status = true; break;
			case UHOM_FLEE: hd->base_status.flee = (short)value; calc_status = true; break;
			case UHOM_PDODGE: hd->base_status.flee2 = (short)value; calc_status = true; break;
			case UHOM_CRIT: hd->base_status.cri = (short)value; calc_status = true; break;
			case UHOM_RACE: hd->base_status.race = (unsigned char)value; calc_status = true; break;
			case UHOM_ELETYPE: hd->base_status.def_ele = (unsigned char)value; calc_status = true; break;
			case UHOM_ELELEVEL: hd->base_status.ele_lv = (unsigned char)value; calc_status = true; break;
			case UHOM_AMOTION: hd->base_status.amotion = (short)value; calc_status = true; break;
			case UHOM_ADELAY: hd->base_status.adelay = (short)value; calc_status = true; break;
			case UHOM_DMOTION: hd->base_status.dmotion = (short)value; calc_status = true; break;
			case UHOM_TARGETID: {
				struct block_list* target = map_id2bl(value);
				if (!target) {
					ShowWarning("buildin_setunitdata: Error in finding target for BL_HOM!\n");
					return SCRIPT_CMD_FAILURE;
				}
				unit_attack(&hd->bl, target->id, 1);
				break;
			}
			default:
				ShowError("buildin_setunitdata: Unknown data identifier %d for BL_HOM.\n", type);
				return SCRIPT_CMD_FAILURE;
			}
			if (calc_status)
				status_calc_bl(&hd->bl, SCB_BATTLE);
		break;

	case BL_PET:
		if (!pd) {
			ShowWarning("buildin_setunitdata: Error in finding object BL_PET!\n");
			return SCRIPT_CMD_FAILURE;
		}
		switch (type) {
			case UPET_SIZE: pd->status.size = (unsigned char)value; break;
			case UPET_LEVEL: pd->pet.level = (unsigned short)value; break;
			case UPET_HP: status_set_hp(bl, (unsigned int)value, 0); break;
			case UPET_MAXHP: status_set_maxhp(bl, (unsigned int)value, 0); break;
			case UPET_MASTERAID: pd->pet.account_id = (unsigned int)value; break;
			case UPET_MAPID: if (mapname) value = map_mapname2mapid(mapname); unit_warp(bl, (short)value, 0, 0, CLR_TELEPORT); break;
			case UPET_X: if (!unit_walktoxy(bl, (short)value, pd->bl.y, 2)) unit_movepos(bl, (short)value, md->bl.y, 0, 0); break;
			case UPET_Y: if (!unit_walktoxy(bl, pd->bl.x, (short)value, 2)) unit_movepos(bl, pd->bl.x, (short)value, 0, 0); break;
			case UPET_HUNGER: pd->pet.hungry = (short)value; clif_send_petdata(map_id2sd(pd->pet.account_id), pd, 2, pd->pet.hungry); break;
			case UPET_INTIMACY: pet_set_intimate(pd, (unsigned int)value); clif_send_petdata(map_id2sd(pd->pet.account_id), pd, 1, pd->pet.intimate); break;
			case UPET_SPEED: pd->status.speed = (unsigned short)value; status_calc_misc(bl, &pd->status, pd->pet.level); break;
			case UPET_LOOKDIR: unit_setdir(bl, (uint8)value); break;
			case UPET_CANMOVETICK: pd->ud.canmove_tick = value > 0 ? (unsigned int)value : 0; break;
			case UPET_STR: pd->status.str = (unsigned short)value; status_calc_misc(bl, &pd->status, pd->pet.level); break;
			case UPET_AGI: pd->status.agi = (unsigned short)value; status_calc_misc(bl, &pd->status, pd->pet.level); break;
			case UPET_VIT: pd->status.vit = (unsigned short)value; status_calc_misc(bl, &pd->status, pd->pet.level); break;
			case UPET_INT: pd->status.int_ = (unsigned short)value; status_calc_misc(bl, &pd->status, pd->pet.level); break;
			case UPET_DEX: pd->status.dex = (unsigned short)value; status_calc_misc(bl, &pd->status, pd->pet.level); break;
			case UPET_LUK: pd->status.luk = (unsigned short)value; status_calc_misc(bl, &pd->status, pd->pet.level); break;
			case UPET_DMGIMMUNE: pd->ud.immune_attack = value > 0; break;
			case UPET_ATKRANGE: pd->status.rhw.range = (unsigned short)value; break;
			case UPET_ATKMIN: pd->status.rhw.atk = (unsigned short)value; break;
			case UPET_ATKMAX: pd->status.rhw.atk2 = (unsigned short)value; break;
			case UPET_MATKMIN: pd->status.matk_min = (unsigned short)value; break;
			case UPET_MATKMAX: pd->status.matk_max = (unsigned short)value; break;
			case UPET_DEF: pd->status.def = (defType)value; break;
			case UPET_MDEF: pd->status.mdef = (defType)value; break;
			case UPET_HIT: pd->status.hit = (short)value; break;
			case UPET_FLEE: pd->status.flee = (short)value; break;
			case UPET_PDODGE: pd->status.flee2 = (short)value; break;
			case UPET_CRIT: pd->status.cri = (short)value; break;
			case UPET_RACE: pd->status.race = (unsigned char)value; break;
			case UPET_ELETYPE: pd->status.def_ele = (unsigned char)value; break;
			case UPET_ELELEVEL: pd->status.ele_lv = (unsigned char)value; break;
			case UPET_AMOTION: pd->status.amotion = (short)value; break;
			case UPET_ADELAY: pd->status.adelay = (short)value; break;
			case UPET_DMOTION: pd->status.dmotion = (short)value; break;
			default:
				ShowError("buildin_setunitdata: Unknown data identifier %d for BL_PET.\n", type);
				return SCRIPT_CMD_FAILURE;
			}
		break;

	case BL_MER:
		if (!mc) {
			ShowWarning("buildin_setunitdata: Error in finding object BL_MER!\n");
			return SCRIPT_CMD_FAILURE;
		}
		switch (type) {
			case UMER_SIZE: mc->base_status.size = (unsigned char)value; calc_status = true; break;
			case UMER_HP: mc->base_status.hp = (unsigned int)value; status_set_hp(bl, (unsigned int)value, 0); break;
			case UMER_MAXHP: mc->base_status.max_hp = (unsigned int)value; status_set_maxhp(bl, (unsigned int)value, 0); break;
			case UMER_MASTERCID: mc->mercenary.char_id = (uint32)value; break;
			case UMER_MAPID: if (mapname) value = map_mapname2mapid(mapname); unit_warp(bl, (short)value, 0, 0, CLR_TELEPORT); break;
			case UMER_X: if (!unit_walktoxy(bl, (short)value, mc->bl.y, 2)) unit_movepos(bl, (short)value, mc->bl.y, 0, 0); break;
			case UMER_Y: if (!unit_walktoxy(bl, mc->bl.x, (short)value, 2)) unit_movepos(bl, mc->bl.x, (short)value, 0, 0); break;
			case UMER_KILLCOUNT: mc->mercenary.kill_count = (unsigned int)value; break;
			case UMER_LIFETIME: mc->mercenary.life_time = (unsigned int)value; break;
			case UMER_SPEED: mc->base_status.speed = (unsigned short)value; status_calc_misc(bl, &mc->base_status, mc->db->lv); calc_status = true; break;
			case UMER_LOOKDIR: unit_setdir(bl, (uint8)value); break;
			case UMER_CANMOVETICK: mc->ud.canmove_tick = value > 0 ? (unsigned int)value : 0; break;
			case UMER_STR: mc->base_status.str = (unsigned short)value; status_calc_misc(bl, &mc->base_status, mc->db->lv); calc_status = true; break;
			case UMER_AGI: mc->base_status.agi = (unsigned short)value; status_calc_misc(bl, &mc->base_status, mc->db->lv); calc_status = true; break;
			case UMER_VIT: mc->base_status.vit = (unsigned short)value; status_calc_misc(bl, &mc->base_status, mc->db->lv); calc_status = true; break;
			case UMER_INT: mc->base_status.int_ = (unsigned short)value; status_calc_misc(bl, &mc->base_status, mc->db->lv); calc_status = true; break;
			case UMER_DEX: mc->base_status.dex = (unsigned short)value; status_calc_misc(bl, &mc->base_status, mc->db->lv); calc_status = true; break;
			case UMER_LUK: mc->base_status.luk = (unsigned short)value; status_calc_misc(bl, &mc->base_status, mc->db->lv); calc_status = true; break;
			case UMER_DMGIMMUNE: mc->ud.immune_attack = value > 0; break;
			case UMER_ATKRANGE: mc->base_status.rhw.range = (unsigned short)value; calc_status = true; break;
			case UMER_ATKMIN: mc->base_status.rhw.atk = (unsigned short)value; calc_status = true; break;
			case UMER_ATKMAX: mc->base_status.rhw.atk2 = (unsigned short)value; calc_status = true; break;
			case UMER_MATKMIN: mc->base_status.matk_min = (unsigned short)value; calc_status = true; break;
			case UMER_MATKMAX: mc->base_status.matk_max = (unsigned short)value; calc_status = true; break;
			case UMER_DEF: mc->base_status.def = (defType)value; calc_status = true; break;
			case UMER_MDEF: mc->base_status.mdef = (defType)value; calc_status = true; break;
			case UMER_HIT: mc->base_status.hit = (short)value; calc_status = true; break;
			case UMER_FLEE: mc->base_status.flee = (short)value; calc_status = true; break;
			case UMER_PDODGE: mc->base_status.flee2 = (short)value; calc_status = true; break;
			case UMER_CRIT: mc->base_status.cri = (short)value; calc_status = true; break;
			case UMER_RACE: mc->base_status.race = (unsigned char)value; calc_status = true; break;
			case UMER_ELETYPE: mc->base_status.def_ele = (unsigned char)value; calc_status = true; break;
			case UMER_ELELEVEL: mc->base_status.ele_lv = (unsigned char)value; calc_status = true; break;
			case UMER_AMOTION: mc->base_status.amotion = (short)value; calc_status = true; break;
			case UMER_ADELAY: mc->base_status.adelay = (short)value; calc_status = true; break;
			case UMER_DMOTION: mc->base_status.dmotion = (short)value; calc_status = true; break;
			case UMER_TARGETID: {
				struct block_list* target = map_id2bl(value);
				if (!target) {
					ShowWarning("buildin_setunitdata: Error in finding target for BL_MER!\n");
					return SCRIPT_CMD_FAILURE;
				}
				unit_attack(&mc->bl, target->id, 1);
				break;
			}
			default:
				ShowError("buildin_setunitdata: Unknown data identifier %d for BL_MER.\n", type);
				return SCRIPT_CMD_FAILURE;
			}
			if (calc_status)
				status_calc_bl(&mc->bl, SCB_BATTLE);
		break;

	case BL_ELEM:
		if (!ed) {
			ShowWarning("buildin_setunitdata: Error in finding object BL_ELEM!\n");
			return SCRIPT_CMD_FAILURE;
		}
		switch (type) {
			case UELE_SIZE: ed->base_status.size = (unsigned char)value; calc_status = true; break;
			case UELE_HP: ed->base_status.hp = (unsigned int)value; status_set_hp(bl, (unsigned int)value, 0); break;
			case UELE_MAXHP: ed->base_status.max_hp = (unsigned int)value; status_set_maxhp(bl, (unsigned int)value, 0); break;
			case UELE_SP: ed->base_status.sp = (unsigned int)value; status_set_sp(bl, (unsigned int)value, 0); break;
			case UELE_MAXSP: ed->base_status.max_sp = (unsigned int)value; status_set_maxsp(bl, (unsigned int)value, 0); break;
			case UELE_MASTERCID: ed->elemental.char_id = (uint32)value; break;
			case UELE_MAPID: if (mapname) value = map_mapname2mapid(mapname); unit_warp(bl, (short)value, 0, 0, CLR_TELEPORT); break;
			case UELE_X: if (!unit_walktoxy(bl, (short)value, ed->bl.y, 2)) unit_movepos(bl, (short)value, ed->bl.y, 0, 0); break;
			case UELE_Y: if (!unit_walktoxy(bl, ed->bl.x, (short)value, 2)) unit_movepos(bl, ed->bl.x, (short)value, 0, 0); break;
			case UELE_LIFETIME: ed->elemental.life_time = (unsigned int)value; break;
			case UELE_MODE: ed->elemental.mode = (enum e_mode)value; calc_status = true; break;
			case UELE_SPEED: ed->base_status.speed = (unsigned short)value; status_calc_misc(bl, &ed->base_status, ed->db->lv); calc_status = true; break;
			case UELE_LOOKDIR: unit_setdir(bl, (uint8)value); break;
			case UELE_CANMOVETICK: ed->ud.canmove_tick = value > 0 ? (unsigned int)value : 0; break;
			case UELE_STR: ed->base_status.str = (unsigned short)value; status_calc_misc(bl, &ed->base_status, ed->db->lv); calc_status = true; break;
			case UELE_AGI: ed->base_status.agi = (unsigned short)value; status_calc_misc(bl, &ed->base_status, ed->db->lv); calc_status = true; break;
			case UELE_VIT: ed->base_status.vit = (unsigned short)value; status_calc_misc(bl, &ed->base_status, ed->db->lv); calc_status = true; break;
			case UELE_INT: ed->base_status.int_ = (unsigned short)value; status_calc_misc(bl, &ed->base_status, ed->db->lv); calc_status = true; break;
			case UELE_DEX: ed->base_status.dex = (unsigned short)value; status_calc_misc(bl, &ed->base_status, ed->db->lv); calc_status = true; break;
			case UELE_LUK: ed->base_status.luk = (unsigned short)value; status_calc_misc(bl, &ed->base_status, ed->db->lv); calc_status = true; break;
			case UELE_DMGIMMUNE: ed->ud.immune_attack = value > 0; break;
			case UELE_ATKRANGE: ed->base_status.rhw.range = (unsigned short)value; calc_status = true; break;
			case UELE_ATKMIN: ed->base_status.rhw.atk = (unsigned short)value; calc_status = true; break;
			case UELE_ATKMAX: ed->base_status.rhw.atk2 = (unsigned short)value; calc_status = true; break;
			case UELE_MATKMIN: ed->base_status.matk_min = (unsigned short)value; calc_status = true; break;
			case UELE_MATKMAX: ed->base_status.matk_max = (unsigned short)value; calc_status = true; break;
			case UELE_DEF: ed->base_status.def = (defType)value; calc_status = true; break;
			case UELE_MDEF: ed->base_status.mdef = (defType)value; calc_status = true; break;
			case UELE_HIT: ed->base_status.hit = (short)value; calc_status = true; break;
			case UELE_FLEE: ed->base_status.flee = (short)value; calc_status = true; break;
			case UELE_PDODGE: ed->base_status.flee2 = (short)value; calc_status = true; break;
			case UELE_CRIT: ed->base_status.cri = (short)value; calc_status = true; break;
			case UELE_RACE: ed->base_status.race = (unsigned char)value; calc_status = true; break;
			case UELE_ELETYPE: ed->base_status.def_ele = (unsigned char)value; calc_status = true; break;
			case UELE_ELELEVEL: ed->base_status.ele_lv = (unsigned char)value; calc_status = true; break;
			case UELE_AMOTION: ed->base_status.amotion = (short)value; calc_status = true; break;
			case UELE_ADELAY: ed->base_status.adelay = (short)value; calc_status = true; break;
			case UELE_DMOTION: ed->base_status.dmotion = (short)value; calc_status = true; break;
			case UELE_TARGETID: {
				struct block_list* target = map_id2bl(value);
				if (!target) {
					ShowWarning("buildin_setunitdata: Error in finding target for BL_ELEM!\n");
					return SCRIPT_CMD_FAILURE;
				}
				elemental_change_mode(ed, static_cast<e_mode>(EL_MODE_AGGRESSIVE));
				unit_attack(&ed->bl, target->id, 1);
				break;
			}
			default:
				ShowError("buildin_setunitdata: Unknown data identifier %d for BL_ELEM.\n", type);
				return SCRIPT_CMD_FAILURE;
			}
			if (calc_status)
				status_calc_bl(&ed->bl, SCB_BATTLE);
		break;

	case BL_NPC:
		if (!nd) {
			ShowWarning("buildin_setunitdata: Error in finding object BL_NPC!\n");
			return SCRIPT_CMD_FAILURE;
		}
		switch (type) {
			case UNPC_DISPLAY: status_set_viewdata(bl, (unsigned short)value); break;
			case UNPC_LEVEL: nd->level = (unsigned int)value; break;
			case UNPC_HP: status_set_hp(bl, (unsigned int)value, 0); break;
			case UNPC_MAXHP: status_set_maxhp(bl, (unsigned int)value, 0); break;
			case UNPC_MAPID: if (mapname) value = map_mapname2mapid(mapname); unit_warp(bl, (short)value, 0, 0, CLR_TELEPORT); break;
			case UNPC_X: if (!unit_walktoxy(bl, (short)value, nd->bl.y, 2)) unit_movepos(bl, (short)value, nd->bl.x, 0, 0); break;
			case UNPC_Y: if (!unit_walktoxy(bl, nd->bl.x, (short)value, 2)) unit_movepos(bl, nd->bl.x, (short)value, 0, 0); break;
			case UNPC_LOOKDIR: unit_setdir(bl, (uint8)value); break;
			case UNPC_STR: nd->params.str = (unsigned short)value; status_calc_misc(bl, &nd->status, nd->level); break;
			case UNPC_AGI: nd->params.agi = (unsigned short)value; status_calc_misc(bl, &nd->status, nd->level); break;
			case UNPC_VIT: nd->params.vit = (unsigned short)value; status_calc_misc(bl, &nd->status, nd->level); break;
			case UNPC_INT: nd->params.int_ = (unsigned short)value; status_calc_misc(bl, &nd->status, nd->level); break;
			case UNPC_DEX: nd->params.dex = (unsigned short)value; status_calc_misc(bl, &nd->status, nd->level); break;
			case UNPC_LUK: nd->params.luk = (unsigned short)value; status_calc_misc(bl, &nd->status, nd->level); break;
			case UNPC_PLUSALLSTAT: nd->stat_point = (unsigned int)value; break;
			case UNPC_ATKRANGE: nd->status.rhw.range = (unsigned short)value; break;
			case UNPC_ATKMIN: nd->status.rhw.atk = (unsigned short)value; break;
			case UNPC_ATKMAX: nd->status.rhw.atk2 = (unsigned short)value; break;
			case UNPC_MATKMIN: nd->status.matk_min = (unsigned short)value; break;
			case UNPC_MATKMAX: nd->status.matk_max = (unsigned short)value; break;
			case UNPC_DEF: nd->status.def = (defType)value; break;
			case UNPC_MDEF: nd->status.mdef = (defType)value; break;
			case UNPC_HIT: nd->status.hit = (short)value; break;
			case UNPC_FLEE: nd->status.flee = (short)value; break;
			case UNPC_PDODGE: nd->status.flee2 = (short)value; break;
			case UNPC_CRIT: nd->status.cri = (short)value; break;
			case UNPC_RACE: nd->status.race = (unsigned char)value; break;
			case UNPC_ELETYPE: nd->status.def_ele = (unsigned char)value; break;
			case UNPC_ELELEVEL: nd->status.ele_lv = (unsigned char)value; break;
			case UNPC_AMOTION: nd->status.amotion = (short)value; break;
			case UNPC_ADELAY: nd->status.adelay = (short)value; break;
			case UNPC_DMOTION: nd->status.dmotion = (short)value; break;
			default:
				ShowError("buildin_setunitdata: Unknown data identifier %d for BL_NPC.\n", type);
				return SCRIPT_CMD_FAILURE;
			}
		break;

	default:
		ShowWarning("buildin_setunitdata: Unknown object type!\n");
		return SCRIPT_CMD_FAILURE;
	}

	// Client information updates
	switch (bl->type) {
		case BL_HOM:
			clif_send_homdata(hd->master, SP_ACK, 0);
			break;
		case BL_PET:
			clif_send_petstatus(pd->master);
			break;
		case BL_MER:
			clif_mercenary_info(map_charid2sd(mc->mercenary.char_id));
			clif_mercenary_skillblock(map_charid2sd(mc->mercenary.char_id));
			break;
		case BL_ELEM:
			clif_elemental_info(ed->master);
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Gets the name of a bl.
/// Supported types are [MOB|HOM|PET|NPC].
/// MER and ELEM don't support custom names.
///
/// getunitname <unit id>;
BUILDIN_FUNC(getunitname)
{
	struct block_list* bl = NULL;

	if(!script_rid2bl(2,bl)){
		script_pushconststr(st, "Unknown");
		return SCRIPT_CMD_FAILURE;
	}

	script_pushstrcopy(st, status_get_name(bl));

	return SCRIPT_CMD_SUCCESS;
}

/// Changes the name of a bl.
/// Supported types are [MOB|HOM|PET].
/// For NPC see 'setnpcdisplay', MER and ELEM don't support custom names.
///
/// setunitname <unit id>,<name>;
BUILDIN_FUNC(setunitname)
{
	struct block_list* bl = NULL;
	TBL_MOB* md = NULL;
	TBL_HOM* hd = NULL;
	TBL_PET* pd = NULL;

	if(!script_rid2bl(2,bl))
	{
		script_pushconststr(st, "Unknown");
		return SCRIPT_CMD_FAILURE;
	}

	switch (bl->type) {
		case BL_MOB:  md = map_id2md(bl->id); break;
		case BL_HOM:  hd = map_id2hd(bl->id); break;
		case BL_PET:  pd = map_id2pd(bl->id); break;
		default:
			ShowWarning("buildin_setunitname: Invalid object type!\n");
			return SCRIPT_CMD_FAILURE;
	}

	switch (bl->type) {
		case BL_MOB:
			if (!md) {
				ShowWarning("buildin_setunitname: Error in finding object BL_MOB!\n");
				return SCRIPT_CMD_FAILURE;
			}
			safestrncpy(md->name, script_getstr(st, 3), NAME_LENGTH);
			break;
		case BL_HOM:
			if (!hd) {
				ShowWarning("buildin_setunitname: Error in finding object BL_HOM!\n");
				return SCRIPT_CMD_FAILURE;
			}
			safestrncpy(hd->homunculus.name, script_getstr(st, 3), NAME_LENGTH);
			break;
		case BL_PET:
			if (!pd) {
				ShowWarning("buildin_setunitname: Error in finding object BL_PET!\n");
				return SCRIPT_CMD_FAILURE;
			}
			safestrncpy(pd->pet.name, script_getstr(st, 3), NAME_LENGTH);
			break;
		default:
			ShowWarning("buildin_setunitname: Unknown object type!\n");
			return SCRIPT_CMD_FAILURE;
	}
	clif_name_area(bl); // Send update to client.

	return SCRIPT_CMD_SUCCESS;
}

/// Makes the unit walk to target position or map.
/// Returns if it was successful.
///
/// unitwalk(<unit_id>,<x>,<y>{,<event_label>}) -> <bool>
/// unitwalkto(<unit_id>,<target_id>{,<event_label>}) -> <bool>
BUILDIN_FUNC(unitwalk)
{
	struct block_list* bl;
	struct unit_data *ud = NULL;
	const char *cmd = script_getfuncname(st), *done_label = "";
	uint8 off = 5;
	
	if(!script_rid2bl(2,bl))
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	ud = unit_bl2ud(bl);

	if (!strcmp(cmd,"unitwalk")) {
		int x = script_getnum(st,3);
		int y = script_getnum(st,4);

		if (script_pushint(st, unit_can_reach_pos(bl,x,y,0)))
			add_timer(gettick()+50, unit_delay_walktoxy_timer, bl->id, (x<<16)|(y&0xFFFF)); // Need timer to avoid mismatches
	} else {
		struct block_list* tbl = map_id2bl(script_getnum(st,3));

		if (!tbl) {
			ShowError("buildin_unitwalk: Bad target destination.\n");
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
		} else if (script_pushint(st, unit_can_reach_bl(bl, tbl, distance_bl(bl, tbl)+1, 0, NULL, NULL)))
			add_timer(gettick()+50, unit_delay_walktobl_timer, bl->id, tbl->id); // Need timer to avoid mismatches
		off = 4;
	}

	if (ud && script_hasdata(st, off)) {
		done_label = script_getstr(st, off);
		check_event(st, done_label);
		safestrncpy(ud->walk_done_event, done_label, sizeof(ud->walk_done_event));
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Kills the unit.
///
/// unitkill <unit_id>;
BUILDIN_FUNC(unitkill)
{
	struct block_list* bl;

	if(script_rid2bl(2,bl))
		status_kill(bl);

	return SCRIPT_CMD_SUCCESS;
}

/// Warps the unit to the target position in the target map.
/// Returns if it was successful.
///
/// unitwarp(<unit_id>,"<map name>",<x>,<y>) -> <bool>
BUILDIN_FUNC(unitwarp)
{
	int map_idx;
	short x;
	short y;
	struct block_list* bl;
	const char *mapname;

	mapname = script_getstr(st, 3);
	x = (short)script_getnum(st,4);
	y = (short)script_getnum(st,5);

	if(!script_rid2bl(2,bl))
	{
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (!strcmp(mapname,"this"))
		map_idx = bl?bl->m:-1;
	else
		map_idx = map_mapname2mapid(mapname);

	if (map_idx >= 0 && bl != NULL)
		script_pushint(st, unit_warp(bl,map_idx,x,y,CLR_OUTSIGHT));
	else
		script_pushint(st, 0);

	return SCRIPT_CMD_SUCCESS;
}

/// Makes the unit attack the target.
/// If the unit is a player and <action type> is not 0, it does a continuous
/// attack instead of a single attack.
/// Returns if the request was successful.
///
/// unitattack(<unit_id>,"<target name>"{,<action type>}) -> <bool>
/// unitattack(<unit_id>,<target_id>{,<action type>}) -> <bool>
BUILDIN_FUNC(unitattack)
{
	struct block_list* unit_bl;
	struct block_list* target_bl = NULL;
	struct script_data* data;
	int actiontype = 0;

	if (!script_rid2bl(2,unit_bl)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	data = script_getdata(st, 3);
	get_val(st, data);

	if (data_isstring(data)) {
		TBL_PC* sd = map_nick2sd(conv_str(st, data),false);
		if( sd != NULL )
			target_bl = &sd->bl;
	} else
		target_bl = map_id2bl(conv_num(st, data));

	if (!target_bl) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st,4))
		actiontype = script_getnum(st,4);

	switch(unit_bl->type) {
		case BL_PC: {
			struct map_session_data* sd = (struct map_session_data*)unit_bl;

			clif_parse_ActionRequest_sub(sd, actiontype > 0 ? 0x07 : 0x00, target_bl->id, gettick());
			script_pushint(st, sd->ud.target == target_bl->id);
			return SCRIPT_CMD_SUCCESS;
		}
		case BL_MOB:
			((TBL_MOB *)unit_bl)->target_id = target_bl->id;
			break;
		case BL_PET:
			((TBL_PET *)unit_bl)->target_id = target_bl->id;
			break;
		default:
			ShowError("buildin_unitattack: Unsupported source unit type %d.\n", unit_bl->type);
			script_pushint(st, false);
			return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, unit_walktobl(unit_bl, target_bl, 65025, 2));
	return SCRIPT_CMD_SUCCESS;
}

/// Makes the unit stop attacking.
///
/// unitstopattack <unit_id>;
BUILDIN_FUNC(unitstopattack)
{
	struct block_list* bl;

	if(script_rid2bl(2,bl))
	{
		unit_stop_attack(bl);
		if (bl->type == BL_MOB)
			((TBL_MOB*)bl)->target_id = 0;
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Makes the unit stop walking.
///
/// unitstopwalk <unit_id>{,<flag>};
BUILDIN_FUNC(unitstopwalk)
{
	struct block_list* bl;
	int flag = USW_NONE;

	if (script_hasdata(st, 3))
		flag = script_getnum(st, 3);

	if(script_rid2bl(2,bl))
		unit_stop_walking(bl, flag);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Makes the unit say the given message.
 *
 * unittalk <unit_id>,"<message>"{,"<flag>"};
 * @param flag: Specify target
 *   bc_area - Message is sent to players in the vicinity of the source (default).
 *   bc_self - Message is sent only to player attached.
 */
BUILDIN_FUNC(unittalk)
{
	const char* message;
	struct block_list* bl;

	message = script_getstr(st, 3);

	if(script_rid2bl(2,bl))
	{
		send_target target = AREA;
		struct StringBuf sbuf;

		if (script_hasdata(st, 4)) {
			if (script_getnum(st, 4) == BC_SELF) {
				if (map_id2sd(bl->id) == NULL) {
					ShowWarning("script: unittalk: bc_self can't be used for non-players objects.\n");
					return SCRIPT_CMD_FAILURE;
				}
				target = SELF;
			}
		}

		StringBuf_Init(&sbuf);
		StringBuf_Printf(&sbuf, "%s", message);
		clif_disp_overhead_(bl, StringBuf_Value(&sbuf), target);
		StringBuf_Destroy(&sbuf);
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Makes the unit do an emotion.
///
/// unitemote <unit_id>,<emotion>;
///
/// @see ET_* in script_constants.h
BUILDIN_FUNC(unitemote)
{
	int emotion;
	struct block_list* bl;

	emotion = script_getnum(st,3);

	if (emotion < ET_SURPRISE || emotion >= ET_MAX) {
		ShowWarning("buildin_emotion: Unknown emotion %d (min=%d, max=%d).\n", emotion, ET_SURPRISE, (ET_MAX-1));
		return SCRIPT_CMD_FAILURE;
	}

	if (script_rid2bl(2,bl))
		clif_emotion(bl, emotion);

	return SCRIPT_CMD_SUCCESS;
}

/// Makes the unit cast the skill on the target or self if no target is specified.
///
/// unitskilluseid <unit_id>,<skill_id>,<skill_lv>{,<target_id>,<casttime>};
/// unitskilluseid <unit_id>,"<skill name>",<skill_lv>{,<target_id>,<casttime>};
BUILDIN_FUNC(unitskilluseid)
{
	int unit_id, target_id, casttime;
	uint16 skill_id, skill_lv;
	struct block_list* bl;
	struct script_data *data;

	unit_id  = script_getnum(st,2);
	data = script_getdata(st, 3);
	get_val(st, data); // Convert into value in case of a variable
	skill_id = ( data_isstring(data) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	skill_lv = script_getnum(st,4);
	target_id = ( script_hasdata(st,5) ? script_getnum(st,5) : unit_id );
	casttime = ( script_hasdata(st,6) ? script_getnum(st,6) : 0 );
	
	if(script_rid2bl(2,bl)){
		if (bl->type == BL_NPC) {
			if (!((TBL_NPC*)bl)->status.hp)
				status_calc_npc(((TBL_NPC*)bl), SCO_FIRST);
			else
				status_calc_npc(((TBL_NPC*)bl), SCO_NONE);
		}
		unit_skilluse_id2(bl, target_id, skill_id, skill_lv, (casttime * 1000) + skill_castfix(bl, skill_id, skill_lv), skill_get_castcancel(skill_id));
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Makes the unit cast the skill on the target position.
///
/// unitskillusepos <unit_id>,<skill_id>,<skill_lv>,<target_x>,<target_y>{,<casttime>};
/// unitskillusepos <unit_id>,"<skill name>",<skill_lv>,<target_x>,<target_y>{,<casttime>};
BUILDIN_FUNC(unitskillusepos)
{
	int skill_x, skill_y, casttime;
	uint16 skill_id, skill_lv;
	struct block_list* bl;
	struct script_data *data;

	data = script_getdata(st, 3);
	get_val(st, data); // Convert into value in case of a variable
	skill_id = ( data_isstring(data) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	skill_lv = script_getnum(st,4);
	skill_x  = script_getnum(st,5);
	skill_y  = script_getnum(st,6);
	casttime = ( script_hasdata(st,7) ? script_getnum(st,7) : 0 );

	if(script_rid2bl(2,bl)){
		if (bl->type == BL_NPC) {
			if (!((TBL_NPC*)bl)->status.hp)
				status_calc_npc(((TBL_NPC*)bl), SCO_FIRST);
			else
				status_calc_npc(((TBL_NPC*)bl), SCO_NONE);
		}
		unit_skilluse_pos2(bl, skill_x, skill_y, skill_id, skill_lv, (casttime * 1000) + skill_castfix(bl, skill_id, skill_lv), skill_get_castcancel(skill_id));
	}

	return SCRIPT_CMD_SUCCESS;
}

// <--- [zBuffer] List of unit control commands

/// Pauses the execution of the script, detaching the player
///
/// sleep <milli seconds>;
BUILDIN_FUNC(sleep)
{
	// First call(by function call)
	if (st->sleep.tick == 0) {
		int ticks;

		ticks = script_getnum(st, 2);

		if (ticks <= 0) {
			ShowError("buildin_sleep: negative or zero amount('%d') of milli seconds is not supported\n", ticks);
			return SCRIPT_CMD_FAILURE;
		}

		// detach the player
		script_detach_rid(st);

		// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = ticks;
	// Second call(by timer after sleeping time is over)
	} else {		
		// Continue the script
		st->state = RUN;
		st->sleep.tick = 0;
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Pauses the execution of the script, keeping the unit attached
/// Stops the script if no unit is attached
///
/// sleep2(<milli seconds>)
BUILDIN_FUNC(sleep2)
{
	// First call(by function call)
	if (st->sleep.tick == 0) {
		int ticks;

		ticks = script_getnum(st, 2);

		if (ticks <= 0) {
			ShowError( "buildin_sleep2: negative or zero amount('%d') of milli seconds is not supported\n", ticks );
			return SCRIPT_CMD_FAILURE;
		}

		if (map_id2bl(st->rid) == NULL) {
			ShowError( "buildin_sleep2: no unit is attached\n" );
			return SCRIPT_CMD_FAILURE;
		}
		
		// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = ticks;
	// Second call(by timer after sleeping time is over)
	} else {		
		// Check if the unit is still attached
		// NOTE: This should never happen, since run_script_timer already checks this
		if (map_id2bl(st->rid) == NULL) {
			// The unit is not attached anymore - terminate the script
			st->rid = 0;
			st->state = END;
		} else {
			// The unit is still attached - continue the script
			st->state = RUN;
			st->sleep.tick = 0;
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/// Awakes all the sleep timers of the target npc
///
/// awake "<npc name>";
BUILDIN_FUNC(awake)
{
	DBIterator *iter;
	struct script_state *tst;
	struct npc_data* nd;

	if ((nd = npc_name2id(script_getstr(st, 2))) == NULL) {
		ShowError("buildin_awake: NPC \"%s\" not found\n", script_getstr(st, 2));
		return SCRIPT_CMD_FAILURE;
	}

	iter = db_iterator(st_db);

	for (tst = static_cast<script_state *>(dbi_first(iter)); dbi_exists(iter); tst = static_cast<script_state *>(dbi_next(iter))) {
		if (tst->oid == nd->bl.id) {
			if (tst->sleep.timer == INVALID_TIMER) { // already awake ???
				continue;
			}

			delete_timer(tst->sleep.timer, run_script_timer);

			// Trigger the timer function
			run_script_timer(INVALID_TIMER, gettick(), tst->sleep.charid, (intptr_t)tst);
		}
	}
	dbi_destroy(iter);

	return SCRIPT_CMD_SUCCESS;
}

/// Returns a reference to a variable of the target NPC.
/// Returns 0 if an error occurs.
///
/// getvariableofnpc(<variable>, "<npc name>") -> <reference>
BUILDIN_FUNC(getvariableofnpc)
{
	struct script_data* data;
	const char* name;
	struct npc_data* nd;

	data = script_getdata(st,2);
	if( !data_isreference(data) )
	{// Not a reference (aka varaible name)
		ShowError("buildin_getvariableofnpc: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	name = reference_getname(data);
	if( *name != '.' || name[1] == '@' )
	{// not a npc variable
		ShowError("buildin_getvariableofnpc: invalid scope (not npc variable)\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	nd = npc_name2id(script_getstr(st,3));
	if( nd == NULL || nd->subtype != NPCTYPE_SCRIPT || nd->u.scr.script == NULL )
	{// NPC not found or has no script
		ShowError("buildin_getvariableofnpc: can't find npc %s\n", script_getstr(st,3));
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	if (!nd->u.scr.script->local.vars)
		nd->u.scr.script->local.vars = i64db_alloc(DB_OPT_RELEASE_DATA);

	push_val2(st->stack, C_NAME, reference_getuid(data), &nd->u.scr.script->local);
	return SCRIPT_CMD_SUCCESS;
}

/// Opens a warp portal.
/// Has no "portal opening" effect/sound, it opens the portal immediately.
///
/// warpportal <source x>,<source y>,"<target map>",<target x>,<target y>;
///
/// @author blackhole89
BUILDIN_FUNC(warpportal)
{
	int spx;
	int spy;
	unsigned short mapindex;
	int tpx;
	int tpy;
	struct skill_unit_group* group;
	struct block_list* bl;

	bl = map_id2bl(st->oid);
	if( bl == NULL ) {
		ShowError("buildin_warpportal: NPC is needed\n");
		return SCRIPT_CMD_FAILURE;
	}

	spx = script_getnum(st,2);
	spy = script_getnum(st,3);
	mapindex = mapindex_name2id(script_getstr(st, 4));
	tpx = script_getnum(st,5);
	tpy = script_getnum(st,6);

	if( mapindex == 0 ) {
		ShowError("buildin_warpportal: Target map not found %s.\n", script_getstr(st, 4));
		return SCRIPT_CMD_FAILURE;
	}

	group = skill_unitsetting(bl, AL_WARP, 4, spx, spy, 0);
	if( group == NULL )
		return SCRIPT_CMD_FAILURE;// failed
	group->val1 = (group->val1<<16)|(short)0;
	group->val2 = (tpx<<16) | tpy;
	group->val3 = mapindex;

	return SCRIPT_CMD_SUCCESS;
}

/**
 * openmail({<char_id>});
 **/
BUILDIN_FUNC(openmail)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	mail_openmail(sd);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * openauction({<char_id>});
 **/
BUILDIN_FUNC(openauction)
{
	TBL_PC* sd;

	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;

	if( !battle_config.feature_auction ) {
		clif_messagecolor(&sd->bl, color_table[COLOR_RED], msg_txt(sd, 517), false, SELF);
		return SCRIPT_CMD_SUCCESS;
	}

	clif_Auction_openwindow(sd);

	return SCRIPT_CMD_SUCCESS;
}

/// Retrieves the value of the specified flag of the specified cell.
///
/// checkcell("<map name>",<x>,<y>,<type>) -> <bool>
///
/// @see cell_chk* constants in const.txt for the types
BUILDIN_FUNC(checkcell)
{
	int16 m = map_mapname2mapid(script_getstr(st,2));
	int16 x = script_getnum(st,3);
	int16 y = script_getnum(st,4);
	cell_chk type = (cell_chk)script_getnum(st,5);

	script_pushint(st, map_getcell(m, x, y, type));

	return SCRIPT_CMD_SUCCESS;
}

/// Modifies flags of cells in the specified area.
///
/// setcell "<map name>",<x1>,<y1>,<x2>,<y2>,<type>,<flag>;
///
/// @see cell_* constants in const.txt for the types
BUILDIN_FUNC(setcell)
{
	int16 m = map_mapname2mapid(script_getstr(st,2));
	int16 x1 = script_getnum(st,3);
	int16 y1 = script_getnum(st,4);
	int16 x2 = script_getnum(st,5);
	int16 y2 = script_getnum(st,6);
	cell_t type = (cell_t)script_getnum(st,7);
	bool flag = script_getnum(st,8) != 0;

	int x,y;

	if( x1 > x2 ) SWAP(x1,x2);
	if( y1 > y2 ) SWAP(y1,y2);

	for( y = y1; y <= y2; ++y )
		for( x = x1; x <= x2; ++x )
			map_setcell(m, x, y, type, flag);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Gets a free cell in the specified area.
 * getfreecell "<map name>",<rX>,<rY>{,<x>,<y>,<rangeX>,<rangeY>,<flag>};
 */
BUILDIN_FUNC(getfreecell)
{
	const char *mapn = script_getstr(st, 2), *name;
	char prefix;
	struct map_session_data *sd;
	int64 num;
	int16 m, x = 0, y = 0;
	int rx = -1, ry = -1, flag = 1;

	sd = map_id2sd(st->rid);

	if (!data_isreference(script_getdata(st, 3))) {
		ShowWarning("script: buildin_getfreecell: rX is not a variable.\n");
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	if (!data_isreference(script_getdata(st, 4))) {
		ShowWarning("script: buildin_getfreecell: rY is not a variable.\n");
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	if (is_string_variable(reference_getname(script_getdata(st, 3)))) {
		ShowWarning("script: buildin_getfreecell: rX is a string, must be an INT.\n");
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	if (is_string_variable(reference_getname(script_getdata(st, 4)))) {
		ShowWarning("script: buildin_getfreecell: rY is a string, must be an INT.\n");
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 5))
		x = script_getnum(st, 5);

	if (script_hasdata(st, 6))
		y = script_getnum(st, 6);

	if (script_hasdata(st, 7))
		rx = script_getnum(st, 7);

	if (script_hasdata(st, 8))
		ry = script_getnum(st, 8);

	if (script_hasdata(st, 9))
		flag = script_getnum(st, 9);

	if (sd && strcmp(mapn, "this") == 0)
		m = sd->bl.m;
	else
		m = map_mapname2mapid(mapn);

	map_search_freecell(NULL, m, &x, &y, rx, ry, flag);

	// Set MapX
	num = st->stack->stack_data[st->start + 3].u.num;
	name = get_str(num&0x00ffffff);
	prefix = *name;

	if (not_server_variable(prefix)){
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_getfreecell: variable '%s' for mapX is not a server variable, but no player is attached!", name );
			return SCRIPT_CMD_FAILURE;
		}
	}else
		sd = NULL;

	set_reg(st, sd, num, name, (void*)__64BPRTSIZE((int)x), script_getref(st, 3));

	// Set MapY
	num = st->stack->stack_data[st->start + 4].u.num;
	name = get_str(num&0x00ffffff);
	prefix = *name;

	if (not_server_variable(prefix)){
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_getfreecell: variable '%s' for mapY is not a server variable, but no player is attached!", name );
			return SCRIPT_CMD_FAILURE;
		}
	}else
		sd = NULL;

	set_reg(st, sd, num, name, (void*)__64BPRTSIZE((int)y), script_getref(st, 4));

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Mercenary Commands
 *------------------------------------------*/
BUILDIN_FUNC(mercenary_create)
{
	struct map_session_data *sd;
	int class_, contract_time;

	if( !script_rid2sd(sd) || sd->md || sd->status.mer_id != 0 )
		return SCRIPT_CMD_SUCCESS;

	class_ = script_getnum(st,2);

	if( !mercenary_db(class_) )
		return SCRIPT_CMD_SUCCESS;

	contract_time = script_getnum(st,3);
	mercenary_create(sd, class_, contract_time);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mercenary_heal)
{
	struct map_session_data *sd;
	int hp, sp;

	if( !script_rid2sd(sd) || sd->md == NULL )
		return SCRIPT_CMD_SUCCESS;
	hp = script_getnum(st,2);
	sp = script_getnum(st,3);

	status_heal(&sd->md->bl, hp, sp, 0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mercenary_sc_start)
{
	struct map_session_data *sd;
	enum sc_type type;
	int tick, val1;

	if( !script_rid2sd(sd) || sd->md == NULL )
		return SCRIPT_CMD_SUCCESS;

	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);

	status_change_start(NULL, &sd->md->bl, type, 10000, val1, 0, 0, 0, tick, SCSTART_NOTICKDEF);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mercenary_get_calls)
{
	struct map_session_data *sd;
	int guild;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	guild = script_getnum(st,2);
	switch( guild )
	{
		case ARCH_MERC_GUILD:
			script_pushint(st,sd->status.arch_calls);
			break;
		case SPEAR_MERC_GUILD:
			script_pushint(st,sd->status.spear_calls);
			break;
		case SWORD_MERC_GUILD:
			script_pushint(st,sd->status.sword_calls);
			break;
		default:
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mercenary_set_calls)
{
	struct map_session_data *sd;
	int guild, value, *calls;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	guild = script_getnum(st,2);
	value = script_getnum(st,3);

	switch( guild )
	{
		case ARCH_MERC_GUILD:
			calls = &sd->status.arch_calls;
			break;
		case SPEAR_MERC_GUILD:
			calls = &sd->status.spear_calls;
			break;
		case SWORD_MERC_GUILD:
			calls = &sd->status.sword_calls;
			break;
		default:
			ShowError("buildin_mercenary_set_calls: Invalid guild '%d'.\n", guild);
			return SCRIPT_CMD_SUCCESS; // Invalid Guild
	}

	*calls += value;
	*calls = cap_value(*calls, 0, INT_MAX);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mercenary_get_faith)
{
	struct map_session_data *sd;
	int guild;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	guild = script_getnum(st,2);
	switch( guild )
	{
		case ARCH_MERC_GUILD:
			script_pushint(st,sd->status.arch_faith);
			break;
		case SPEAR_MERC_GUILD:
			script_pushint(st,sd->status.spear_faith);
			break;
		case SWORD_MERC_GUILD:
			script_pushint(st,sd->status.sword_faith);
			break;
		default:
			script_pushint(st,0);
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(mercenary_set_faith)
{
	struct map_session_data *sd;
	int guild, value, *calls;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	guild = script_getnum(st,2);
	value = script_getnum(st,3);

	switch( guild )
	{
		case ARCH_MERC_GUILD:
			calls = &sd->status.arch_faith;
			break;
		case SPEAR_MERC_GUILD:
			calls = &sd->status.spear_faith;
			break;
		case SWORD_MERC_GUILD:
			calls = &sd->status.sword_faith;
			break;
		default:
			ShowError("buildin_mercenary_set_faith: Invalid guild '%d'.\n", guild);
			return SCRIPT_CMD_SUCCESS; // Invalid Guild
	}

	*calls += value;
	*calls = cap_value(*calls, 0, INT_MAX);
	if( mercenary_get_guild(sd->md) == guild )
		clif_mercenary_updatestatus(sd,SP_MERCFAITH);

	return SCRIPT_CMD_SUCCESS;
}

/*------------------------------------------
 * Book Reading
 *------------------------------------------*/
BUILDIN_FUNC(readbook)
{
	struct map_session_data *sd;
	int book_id, page;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	book_id = script_getnum(st,2);
	page = script_getnum(st,3);

	clif_readbook(sd->fd, book_id, page);
	return SCRIPT_CMD_SUCCESS;
}

/******************
Questlog script commands
*******************/
 /**
 * Add job criteria to questinfo
 * @param qi Quest Info
 * @param job
 * @author [Cydh]
 **/
static void buildin_questinfo_setjob(struct questinfo *qi, int job) {
	RECREATE(qi->jobid, unsigned short, qi->jobid_count+1);
	qi->jobid[qi->jobid_count++] = job;
}
/**
 * questinfo <Quest ID>,<Icon>{,<Map Mark Color>{,<Job Class>}};
 **/
BUILDIN_FUNC(questinfo)
{
	TBL_NPC* nd = map_id2nd(st->oid);
	int quest_id, icon, color = 0;
	struct questinfo qi, *q2;

	if( nd == NULL || nd->bl.m == -1 ) {
		ShowError("buildin_questinfo: No NPC attached.\n");
		return SCRIPT_CMD_FAILURE;
	}

	quest_id = script_getnum(st, 2);
	icon = script_getnum(st, 3);

#if PACKETVER >= 20120410
	switch(icon){
		case QTYPE_QUEST:
		case QTYPE_QUEST2:
		case QTYPE_JOB:
		case QTYPE_JOB2:
		case QTYPE_EVENT:
		case QTYPE_EVENT2:
		// Warg icons were replaced in this client
#if PACKETVER < 20170315
		case QTYPE_WARG:
		case QTYPE_WARG2:
#else
		case QTYPE_CLICKME:
		case QTYPE_DAILYQUEST:
		case QTYPE_EVENT3:
		case QTYPE_JOBQUEST:
		case QTYPE_JUMPING_PORING:
#endif
			// Leave everything as it is
			break;
		case QTYPE_NONE:
		default:
			// Default to nothing if icon id is invalid.
			icon = QTYPE_NONE;
			break;
	}
#else
	if(icon < QTYPE_QUEST || icon > 7) // TODO: check why 7 and not QTYPE_WARG, might be related to icon + 1 below
		icon = QTYPE_QUEST;
	else
		icon = icon + 1;
#endif

	qi.quest_id = quest_id;
	qi.icon = (unsigned char)icon;
	qi.nd = nd;

	if( script_hasdata(st, 4) ) {
		color = script_getnum(st, 4);
		if( color < 0 || color > 3 ) {
			ShowWarning("buildin_questinfo: invalid color '%d', changing to 0\n",color);
			script_reportfunc(st);
			color = 0;
		}
	}
	qi.color = (unsigned char)color;

	qi.min_level = 1;
	qi.max_level = MAX_LEVEL;

	q2 = map_add_questinfo(nd->bl.m, &qi);
	q2->req = NULL;
	q2->req_count = 0;
	q2->jobid = NULL;
	q2->jobid_count = 0;

	if(script_hasdata(st, 5)) {
		int job = script_getnum(st, 5);

		if (!pcdb_checkid(job))
			ShowError("buildin_questinfo: Nonexistant Job Class.\n");
		else {
			buildin_questinfo_setjob(q2, job);
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * setquest <ID>{,<char_id>};
 **/
BUILDIN_FUNC(setquest)
{
	struct map_session_data *sd;
	int quest_id;

	quest_id = script_getnum(st, 2);

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if( quest_add(sd, quest_id)  == -1 ){
		script_reportsrc(st);
		script_reportfunc(st);
	}

	//20120410 or 20090218 ? no reason that shouldn't work for 2009
	pc_show_questinfo(sd); 
	return SCRIPT_CMD_SUCCESS;
}

/**
 * erasequest <ID>{,<char_id>};
 **/
BUILDIN_FUNC(erasequest)
{
	struct map_session_data *sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if( quest_delete(sd, script_getnum(st, 2))  == -1 ){
		script_reportsrc(st);
		script_reportfunc(st);
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * completequest <ID>{,<char_id>};
 **/
BUILDIN_FUNC(completequest)
{
	struct map_session_data *sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	quest_update_status(sd, script_getnum(st, 2), Q_COMPLETE);
	//20120410 or 20090218
	pc_show_questinfo(sd);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * changequest <ID>,<ID2>{,<char_id>};
 **/
BUILDIN_FUNC(changequest)
{
	struct map_session_data *sd;
	
	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	if( quest_change(sd, script_getnum(st, 2),script_getnum(st, 3)) == -1 ){
		script_reportsrc(st);
		script_reportfunc(st);
	}

	//20120410 or 20090218
	pc_show_questinfo(sd);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * checkquest(<ID>{,PLAYTIME|HUNTING{,<char_id>}})
 **/
BUILDIN_FUNC(checkquest)
{
	struct map_session_data *sd;
	enum quest_check_type type = HAVEQUEST;

	if( script_hasdata(st, 3) )
		type = (enum quest_check_type)script_getnum(st, 3);

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st, quest_check(sd, script_getnum(st, 2), type));

	return SCRIPT_CMD_SUCCESS;
}

/**
 * isbegin_quest(<ID>{,<char_id>})
 **/
BUILDIN_FUNC(isbegin_quest)
{
	struct map_session_data *sd;
	int i;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	i = quest_check(sd, script_getnum(st, 2), (enum quest_check_type) HAVEQUEST);
	script_pushint(st, i + (i < 1));

	return SCRIPT_CMD_SUCCESS;
}

/**
 * showevent <icon>{,<mark color>{,<char_id>}}
 **/
BUILDIN_FUNC(showevent)
{
	TBL_PC *sd;
	struct npc_data *nd = map_id2nd(st->oid);
	int icon, color = 0;

	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_FAILURE;

	if( sd == NULL || nd == NULL )
		return SCRIPT_CMD_SUCCESS;

	icon = script_getnum(st, 2);
	if( script_hasdata(st, 3) ) {
		color = script_getnum(st, 3);
		if( color < 0 || color > 3 ) {
			ShowWarning("buildin_showevent: invalid color '%d', changing to 0\n",color);
			script_reportfunc(st);
			color = 0;
		}
	}

#if PACKETVER >= 20120410
	if(icon < 0 || (icon > 8 && icon != 9999) || icon == 7)
		icon = 9999; // Default to nothing if icon id is invalid.
#else
	if(icon < 0 || icon > 7)
		icon = 0;
	else
		icon = icon + 1;
#endif

	clif_quest_show_event(sd, &nd->bl, icon, color);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * BattleGround System
 *------------------------------------------*/
BUILDIN_FUNC(waitingroom2bg)
{
	struct npc_data *nd;
	struct chat_data *cd;
	const char *map_name, *ev = "", *dev = "";
	int x, y, mapindex = 0, bg_id;
	unsigned char i,c=0;

	if( script_hasdata(st,7) )
		nd = npc_name2id(script_getstr(st,7));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);

	if( nd == NULL || (cd = (struct chat_data *)map_id2bl(nd->chat_id)) == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	map_name = script_getstr(st,2);
	if (strcmp(map_name, "-") != 0 && (mapindex = mapindex_name2id(map_name)) == 0)
	{ // Invalid Map
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	x = script_getnum(st,3);
	y = script_getnum(st,4);
	if(script_hasdata(st,5))
		ev = script_getstr(st,5); // Logout Event
	if(script_hasdata(st,6))
		dev = script_getstr(st,6); // Die Event

	check_event(st, ev);
	check_event(st, dev);

	if( (bg_id = bg_create(mapindex, x, y, ev, dev)) == 0 )
	{ // Creation failed
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	for (i = 0; i < cd->users; i++) { // Only add those who are in the chat room
		struct map_session_data *sd;
		if( (sd = cd->usersd[i]) != NULL && bg_team_join(bg_id, sd) ){
			mapreg_setreg(reference_uid(add_str("$@arenamembers"), c), sd->bl.id);
			++c;
		}
	}

	mapreg_setreg(add_str("$@arenamembersnum"), c);
	script_pushint(st,bg_id);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(waitingroom2bg_single)
{
	const char* map_name;
	struct npc_data *nd;
	struct chat_data *cd;
	struct map_session_data *sd;
	struct battleground_data *bg;
	int x, y, mapindex, bg_id;

	bg_id = script_getnum(st,2);
	if ((bg = bg_team_search(bg_id)) == NULL) {
		script_pushint(st, false);
		return SCRIPT_CMD_SUCCESS;
	}
	if (script_hasdata(st, 3)) {
		map_name = script_getstr(st, 3);
		if ((mapindex = mapindex_name2id(map_name)) == 0) {
			script_pushint(st, false);
			return SCRIPT_CMD_SUCCESS; // Invalid Map
		}
		x = script_getnum(st, 4);
		y = script_getnum(st, 5);
	}
	else {
		mapindex = bg->mapindex;
		x = bg->x;
		y = bg->y;
	}

	nd = npc_name2id(script_getstr(st,6));

	if( nd == NULL || (cd = (struct chat_data *)map_id2bl(nd->chat_id)) == NULL || cd->users <= 0 )
		return SCRIPT_CMD_SUCCESS;

	if( (sd = cd->usersd[0]) == NULL )
		return SCRIPT_CMD_SUCCESS;

	if( bg_team_join(bg_id, sd) && pc_setpos(sd, mapindex, x, y, CLR_TELEPORT) == SETPOS_OK)
	{
		script_pushint(st, true);
	}
	else
		script_pushint(st, false);

	return SCRIPT_CMD_SUCCESS;
}


/// Creates an instance of battleground battle group.
/// *bg_create("<map name>",<x>,<y>{,"<On Quit Event>","<On Death Event>"});
/// @author [secretdataz]
BUILDIN_FUNC(bg_create) {
	const char *map_name, *ev = "", *dev = "";
	int x, y, mapindex = 0;

	map_name = script_getstr(st, 2);
	if (strcmp(map_name, "-") != 0 && (mapindex = mapindex_name2id(map_name)) == 0)
	{ // Invalid Map
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	x = script_getnum(st, 3);
	y = script_getnum(st, 4);
	if(script_hasdata(st, 5))
		ev = script_getstr(st, 5); // Logout Event
	if(script_hasdata(st, 6))
		dev = script_getstr(st, 6); // Die Event

	check_event(st, ev);
	check_event(st, dev);

	script_pushint(st, bg_create(mapindex, x, y, ev, dev));
	return SCRIPT_CMD_SUCCESS;
}

/// Adds attached player or <char id> (if specified) to an existing 
/// battleground group and warps it to the specified coordinates on
/// the given map.
/// bg_join(<battle group>,{"<map name>",<x>,<y>{,<char id>}});
/// @author [secretdataz]
BUILDIN_FUNC(bg_join) {
	const char* map_name;
	struct map_session_data *sd;
	struct battleground_data *bg;
	int x, y, bg_id, mapindex;

	bg_id = script_getnum(st, 2);
	if ((bg = bg_team_search(bg_id)) == NULL) {
		script_pushint(st, false);
		return SCRIPT_CMD_SUCCESS;
	}
	if (script_hasdata(st, 3)) {
		map_name = script_getstr(st, 3);
		if ((mapindex = mapindex_name2id(map_name)) == 0) {
			script_pushint(st, false);
			return SCRIPT_CMD_SUCCESS; // Invalid Map
		}
		x = script_getnum(st, 4);
		y = script_getnum(st, 5);
	} else {
		mapindex = bg->mapindex;
		x = bg->x;
		y = bg->y;
	}

	if (!script_charid2sd(6, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (bg_team_join(bg_id, sd) && pc_setpos(sd, mapindex, x, y, CLR_TELEPORT) == SETPOS_OK)
	{
		script_pushint(st, true);
	}
	else
		script_pushint(st, false);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_team_setxy)
{
	struct battleground_data *bg;
	int bg_id;

	bg_id = script_getnum(st,2);
	if( (bg = bg_team_search(bg_id)) == NULL )
		return SCRIPT_CMD_SUCCESS;

	bg->x = script_getnum(st,3);
	bg->y = script_getnum(st,4);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_warp)
{
	int x, y, mapindex, bg_id;
	const char* map_name;

	bg_id = script_getnum(st,2);
	map_name = script_getstr(st,3);
	if( (mapindex = mapindex_name2id(map_name)) == 0 )
		return SCRIPT_CMD_SUCCESS; // Invalid Map
	x = script_getnum(st,4);
	y = script_getnum(st,5);
	bg_team_warp(bg_id, mapindex, x, y);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_monster)
{
	int class_ = 0, x = 0, y = 0, bg_id = 0;
	const char *str,*mapname, *evt="";

	bg_id  = script_getnum(st,2);
	mapname    = script_getstr(st,3);
	x      = script_getnum(st,4);
	y      = script_getnum(st,5);
	str    = script_getstr(st,6);
	class_ = script_getnum(st,7);
	if( script_hasdata(st,8) ) evt = script_getstr(st,8);
	check_event(st, evt);
	script_pushint(st, mob_spawn_bg(mapname,x,y,str,class_,evt,bg_id));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_monster_set_team)
{
	struct mob_data *md;
	struct block_list *mbl;
	int id = script_getnum(st,2),
		bg_id = script_getnum(st,3);

	if( id == 0 || (mbl = map_id2bl(id)) == NULL || mbl->type != BL_MOB )
		return SCRIPT_CMD_SUCCESS;
	md = (TBL_MOB *)mbl;
	md->bg_id = bg_id;

	mob_stop_attack(md);
	mob_stop_walking(md, 0);
	md->target_id = md->attacked_id = 0;
	clif_name_area(&md->bl);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_leave)
{
	struct map_session_data *sd = NULL;
	if( !script_charid2sd(2,sd) || !sd->bg_id )
		return SCRIPT_CMD_SUCCESS;

	bg_team_leave(sd,0);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_destroy)
{
	int bg_id = script_getnum(st,2);
	bg_team_delete(bg_id);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_getareausers)
{
	const char *str;
	int16 m, x0, y0, x1, y1;
	int bg_id;
	int i = 0, c = 0;
	struct battleground_data *bg = NULL;

	bg_id = script_getnum(st,2);
	str = script_getstr(st,3);

	if( (bg = bg_team_search(bg_id)) == NULL || (m = map_mapname2mapid(str)) < 0 )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	x0 = script_getnum(st,4);
	y0 = script_getnum(st,5);
	x1 = script_getnum(st,6);
	y1 = script_getnum(st,7);

	for( i = 0; i < MAX_BG_MEMBERS; i++ )
	{
		struct map_session_data *sd;
		if( (sd = bg->members[i].sd) == NULL )
			continue;
		if( sd->bl.m != m || sd->bl.x < x0 || sd->bl.y < y0 || sd->bl.x > x1 || sd->bl.y > y1 )
			continue;
		c++;
	}

	script_pushint(st,c);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_updatescore)
{
	const char *str;
	int16 m;

	str = script_getstr(st,2);
	if( (m = map_mapname2mapid(str)) < 0 )
		return SCRIPT_CMD_SUCCESS;

	struct map_data *mapdata = map_getmapdata(m);

	mapdata->bgscore_lion = script_getnum(st,3);
	mapdata->bgscore_eagle = script_getnum(st,4);

	clif_bg_updatescore(m);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(bg_get_data)
{
	struct battleground_data *bg;
	int bg_id = script_getnum(st,2),
		type = script_getnum(st,3), i;

	if( (bg = bg_team_search(bg_id)) == NULL )
	{
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch( type )
	{
		case 0: script_pushint(st, bg->count); break;
		case 1:
			for (i = 0; bg->members[i].sd != NULL; i++)
				mapreg_setreg(reference_uid(add_str("$@arenamembers"), i), bg->members[i].sd->bl.id);
			mapreg_setreg(add_str("$@arenamemberscount"), i);
			script_pushint(st, i);
			break;
		default:
			ShowError("script:bg_get_data: unknown data identifier %d\n", type);
			break;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Instancing System
 *------------------------------------------*/
//Returns an Instance ID
//Checks NPC first, then if player is attached we check
unsigned short script_instancegetid(struct script_state* st)
{
	unsigned short instance_id = 0;
	struct npc_data *nd;

	if( (nd = map_id2nd(st->oid)) && nd->instance_id > 0 )
		instance_id = nd->instance_id;
	else {
		struct map_session_data *sd = NULL;
		struct party_data *pd = NULL;
		struct guild *gd = NULL;
		struct clan *cd = NULL;

		if ((sd = map_id2sd(st->rid))) {
			if (sd->instance_id)
				instance_id = sd->instance_id;
			if (instance_id == 0 && sd->status.party_id && (pd = party_search(sd->status.party_id)) != NULL && pd->instance_id)
				instance_id = pd->instance_id;
			if (instance_id == 0 && sd->status.guild_id && (gd = guild_search(sd->status.guild_id)) != NULL && gd->instance_id)
				instance_id = gd->instance_id;
			if (instance_id == 0 && sd->status.clan_id && (cd = clan_search(sd->status.clan_id)) != NULL && cd->instance_id)
				instance_id = cd->instance_id;
		}
	}

	return instance_id;
}

/*==========================================
 * Creates the instance
 * Returns Instance ID if created successfully
 *------------------------------------------*/
BUILDIN_FUNC(instance_create)
{
	enum instance_mode mode = IM_PARTY;
	int owner_id = 0;

	if (script_hasdata(st, 3)) {
		mode = static_cast<instance_mode>(script_getnum(st, 3));

		if (mode < IM_NONE || mode >= IM_MAX) {
			ShowError("buildin_instance_create: Unknown instance mode %d for '%s'\n", mode, script_getstr(st, 2));
			return SCRIPT_CMD_FAILURE;
		}
	}
	if (script_hasdata(st, 4))
		owner_id = script_getnum(st, 4);
	else {
		// If sd is NULL, instance_create will return -2.
		struct map_session_data *sd = NULL;

		switch(mode) {
			case IM_NONE:
				owner_id = st->oid;
				break;
			case IM_CHAR:
				if (script_rid2sd(sd))
					owner_id = sd->status.char_id;
				break;
			case IM_PARTY:
				if (script_rid2sd(sd))
					owner_id = sd->status.party_id;
				break;
			case IM_GUILD:
				if (script_rid2sd(sd))
					owner_id = sd->status.guild_id;
				break;
			case IM_CLAN:
				if (script_rid2sd(sd))
					owner_id = sd->status.clan_id;
				break;
			default:
				ShowError("buildin_instance_create: Invalid instance mode (instance name: %s)\n", script_getstr(st, 2));
				return SCRIPT_CMD_FAILURE;
		}
	}

	script_pushint(st, instance_create(owner_id, script_getstr(st, 2), mode));
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Destroys an instance (unofficial)
 * Officially instances are only destroyed by timeout
 *
 * instance_destroy {<instance_id>};
 *------------------------------------------*/
BUILDIN_FUNC(instance_destroy)
{
	unsigned short instance_id;

	if( script_hasdata(st,2) )
		instance_id = script_getnum(st,2);
	else
		instance_id = script_instancegetid(st);

	if( instance_id == 0 ) {
		ShowError("buildin_instance_destroy: Trying to destroy invalid instance %hu.\n", instance_id);
		return SCRIPT_CMD_FAILURE;
	}

	instance_destroy(instance_id);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Warps player to instance
 * Results:
 *	IE_OK: Success
 *	IE_NOMEMBER: Character not in party/guild (for party/guild type instances)
 *	IE_NOINSTANCE: Character/Party/Guild doesn't have instance
 *	IE_OTHER: Other errors (instance not in DB, instance doesn't match with character/party/guild, etc.)
 *------------------------------------------*/
BUILDIN_FUNC(instance_enter)
{
	struct map_session_data *sd = NULL;
	int x = script_hasdata(st,3) ? script_getnum(st, 3) : -1;
	int y = script_hasdata(st,4) ? script_getnum(st, 4) : -1;
	unsigned short instance_id;

	if (script_hasdata(st, 6))
		instance_id = script_getnum(st, 6);
	else
		instance_id = script_instancegetid(st);

	if (!script_charid2sd(5,sd))
		return SCRIPT_CMD_FAILURE;

	script_pushint(st, instance_enter(sd, instance_id, script_getstr(st, 2), x, y));

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Returns the name of a duplicated NPC
 *
 * instance_npcname <npc_name>{,<instance_id>};
 * <npc_name> is the full name of an NPC.
 *------------------------------------------*/
BUILDIN_FUNC(instance_npcname)
{
	const char *str;
	unsigned short instance_id = 0;
	struct npc_data *nd;

	str = script_getstr(st,2);
	if( script_hasdata(st,3) )
		instance_id = script_getnum(st,3);
	else
		instance_id = script_instancegetid(st);

	if( instance_id && (nd = npc_name2id(str)) != NULL ) {
		static char npcname[NAME_LENGTH];
		snprintf(npcname, sizeof(npcname), "dup_%hu_%d", instance_id, nd->bl.id);
		script_pushconststr(st,npcname);
	} else {
		ShowError("buildin_instance_npcname: Invalid instance NPC (instance_id: %hu, NPC name: \"%s\".)\n", instance_id, str);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Returns the name of a duplicated map
 *
 * instance_mapname <map_name>{,<instance_id>};
 *------------------------------------------*/
BUILDIN_FUNC(instance_mapname)
{
 	const char *str;
	int16 m;
	unsigned short instance_id = 0;

	str = script_getstr(st,2);

	if( script_hasdata(st,3) )
		instance_id = script_getnum(st,3);
	else
		instance_id = script_instancegetid(st);

	// Check that instance mapname is a valid map
	if(!instance_id || (m = instance_mapname2mapid(str,instance_id)) < 0)
		script_pushconststr(st, "");
	else
		script_pushconststr(st, map_getmapdata(m)->name);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Returns an Instance ID
 *------------------------------------------*/
BUILDIN_FUNC(instance_id)
{
	script_pushint(st, script_instancegetid(st));
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Warps all players inside an instance
 *
 * instance_warpall <map_name>,<x>,<y>{,<instance_id>};
 *------------------------------------------*/
static int buildin_instance_warpall_sub(struct block_list *bl, va_list ap)
{
	unsigned int m = va_arg(ap,unsigned int);
	int x = va_arg(ap,int);
	int y = va_arg(ap,int);
	unsigned short instance_id = va_arg(ap,unsigned int);
	struct map_session_data *sd = NULL;
	int owner_id = 0;

	nullpo_retr(0, bl);

	if (bl->type != BL_PC)
		return 0;

	sd = (TBL_PC *)bl;
	owner_id = instance_data[instance_id].owner_id;
	switch(instance_data[instance_id].mode) {
		case IM_NONE:
			break;
		case IM_CHAR:
			if (sd->status.char_id != owner_id)
				return 0;
			break;
		case IM_PARTY:
			if (sd->status.party_id != owner_id)
				return 0;
			break;
		case IM_GUILD:
			if (sd->status.guild_id != owner_id)
				return 0;
		case IM_CLAN:
			if (sd->status.clan_id != owner_id)
				return 0;
	}

	pc_setpos(sd, m, x, y, CLR_TELEPORT);

	return 1;
}

BUILDIN_FUNC(instance_warpall)
{
	int16 m, i;
	unsigned short instance_id;
	const char *mapn;
	int x, y;

	mapn = script_getstr(st,2);
	x    = script_getnum(st,3);
	y    = script_getnum(st,4);
	if( script_hasdata(st,5) )
		instance_id = script_getnum(st,5);
	else
		instance_id = script_instancegetid(st);

	if( !instance_id || (m = map_mapname2mapid(mapn)) < 0 || (m = instance_mapname2mapid(map_getmapdata(m)->name,instance_id)) < 0)
		return SCRIPT_CMD_FAILURE;

	for(i = 0; i < instance_data[instance_id].cnt_map; i++)
		map_foreachinmap(buildin_instance_warpall_sub, instance_data[instance_id].map[i]->m, BL_PC, map_id2index(m), x, y, instance_id);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Broadcasts to all maps inside an instance
 *
 * instance_announce <instance id>,"<text>",<flag>{,<fontColor>{,<fontType>{,<fontSize>{,<fontAlign>{,<fontY>}}}}};
 * Using 0 for <instance id> will auto-detect the id.
 *------------------------------------------*/
BUILDIN_FUNC(instance_announce) {
	unsigned short instance_id = script_getnum(st,2);
	const char     *mes        = script_getstr(st,3);
	int            flag        = script_getnum(st,4);
	const char     *fontColor  = script_hasdata(st,5) ? script_getstr(st,5) : NULL;
	int            fontType    = script_hasdata(st,6) ? script_getnum(st,6) : FW_NORMAL; // default fontType
	int            fontSize    = script_hasdata(st,7) ? script_getnum(st,7) : 12;    // default fontSize
	int            fontAlign   = script_hasdata(st,8) ? script_getnum(st,8) : 0;     // default fontAlign
	int            fontY       = script_hasdata(st,9) ? script_getnum(st,9) : 0;     // default fontY
	int i;

	if( instance_id == 0 ) {
		instance_id = script_instancegetid(st);
	}

	if( !instance_id && &instance_data[instance_id] != NULL) {
		ShowError("buildin_instance_announce: Intance is not found.\n");
		return SCRIPT_CMD_FAILURE;
	}

	for( i = 0; i < instance_data[instance_id].cnt_map; i++ )
		map_foreachinmap(buildin_announce_sub, instance_data[instance_id].map[i]->m, BL_PC,
						 mes, strlen(mes)+1, flag&BC_COLOR_MASK, fontColor, fontType, fontSize, fontAlign, fontY);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * instance_check_party [malufett]
 * Values:
 * party_id : Party ID of the invoking character. [Required Parameter]
 * amount : Amount of needed Partymembers for the Instance. [Optional Parameter]
 * min : Minimum Level needed to join the Instance. [Optional Parameter]
 * max : Maxium Level allowed to join the Instance. [Optional Parameter]
 * Example: instance_check_party (getcharid(1){,amount}{,min}{,max});
 * Example 2: instance_check_party (getcharid(1),1,1,99);
 *------------------------------------------*/
BUILDIN_FUNC(instance_check_party)
{
	int amount, min, max, i, party_id, c = 0;
	struct party_data *p = NULL;

	amount = script_hasdata(st,3) ? script_getnum(st,3) : 1; // Amount of needed Partymembers for the Instance.
	min = script_hasdata(st,4) ? script_getnum(st,4) : 1; // Minimum Level needed to join the Instance.
	max  = script_hasdata(st,5) ? script_getnum(st,5) : MAX_LEVEL; // Maxium Level allowed to join the Instance.

	if( min < 1 || min > MAX_LEVEL) {
		ShowError("buildin_instance_check_party: Invalid min level, %d\n", min);
		return SCRIPT_CMD_FAILURE;
	} else if(  max < 1 || max > MAX_LEVEL) {
		ShowError("buildin_instance_check_party: Invalid max level, %d\n", max);
		return SCRIPT_CMD_FAILURE;
	}

	if( script_hasdata(st,2) )
		party_id = script_getnum(st,2);
	else return SCRIPT_CMD_FAILURE;

	if( !(p = party_search(party_id)) ) {
		script_pushint(st, 0); // Returns false if party does not exist.
		return SCRIPT_CMD_FAILURE;
	}

	for( i = 0; i < MAX_PARTY; i++ ) {
		struct map_session_data *pl_sd;
		if( (pl_sd = p->data[i].sd) )
			if(map_id2bl(pl_sd->bl.id)) {
				if(pl_sd->status.base_level < min) {
					script_pushint(st, 0);
					return SCRIPT_CMD_SUCCESS;
				} else if(pl_sd->status.base_level > max) {
					script_pushint(st, 0);
					return SCRIPT_CMD_SUCCESS;
				}
					c++;
			}
	}

	if(c < amount)
		script_pushint(st, 0); // Not enough Members in the Party to join Instance.
	else
		script_pushint(st, 1);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * instance_check_guild
 * Values:
 * guild_id : Guild ID of the invoking character. [Required Parameter]
 * amount : Amount of needed Guild members for the Instance. [Optional Parameter]
 * min : Minimum Level needed to join the Instance. [Optional Parameter]
 * max : Maxium Level allowed to join the Instance. [Optional Parameter]
 * Example: instance_check_guild (getcharid(2){,amount}{,min}{,max});
 * Example 2: instance_check_guild (getcharid(2),1,1,99);
 *------------------------------------------*/
BUILDIN_FUNC(instance_check_guild)
{
	int amount, min, max, i, guild_id = 0, c = 0;
	struct guild *g = NULL;

	amount = script_hasdata(st,3) ? script_getnum(st,3) : 1; // Amount of needed Guild members for the Instance.
	min = script_hasdata(st,4) ? script_getnum(st,4) : 1; // Minimum Level needed to join the Instance.
	max  = script_hasdata(st,5) ? script_getnum(st,5) : MAX_LEVEL; // Maxium Level allowed to join the Instance.

	if (min < 1 || min > MAX_LEVEL) {
		ShowError("buildin_instance_check_guild: Invalid min level, %d\n", min);
		return SCRIPT_CMD_FAILURE;
	} else if (max < 1 || max > MAX_LEVEL) {
		ShowError("buildin_instance_check_guild: Invalid max level, %d\n", max);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st,2))
		guild_id = script_getnum(st,2);
	else
		return SCRIPT_CMD_FAILURE;

	if (!(g = guild_search(guild_id))) {
		script_pushint(st, 0); // Returns false if guild does not exist.
		return SCRIPT_CMD_FAILURE;
	}

	for(i = 0; i < MAX_GUILD; i++) {
		struct map_session_data *pl_sd;

		if ((pl_sd = g->member[i].sd)) {
			if (map_id2bl(pl_sd->bl.id)) {
				if (pl_sd->status.base_level < min) {
					script_pushint(st, 0);
					return SCRIPT_CMD_SUCCESS;
				} else if (pl_sd->status.base_level > max) {
					script_pushint(st, 0);
					return SCRIPT_CMD_SUCCESS;
				}
				c++;
			}
		}
	}

	if (c < amount)
		script_pushint(st, 0); // Not enough Members in the Guild to join Instance.
	else
		script_pushint(st, 1);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * instance_check_clan
 * Values:
 * clan_id : Clan ID of the invoking character. [Required Parameter]
 * amount : Amount of needed Clan members for the Instance. [Optional Parameter]
 * min : Minimum Level needed to join the Instance. [Optional Parameter]
 * max : Maxium Level allowed to join the Instance. [Optional Parameter]
 * Example: instance_check_clan (getcharid(5){,amount}{,min}{,max});
 * Example 2: instance_check_clan (getcharid(5),1,1,99);
 *------------------------------------------*/
BUILDIN_FUNC(instance_check_clan)
{
	int amount, min, max, i, clan_id = 0, c = 0;
	struct clan *cd = NULL;

	amount = script_hasdata(st,3) ? script_getnum(st,3) : 1; // Amount of needed Clan members for the Instance.
	min = script_hasdata(st,4) ? script_getnum(st,4) : 1; // Minimum Level needed to join the Instance.
	max  = script_hasdata(st,5) ? script_getnum(st,5) : MAX_LEVEL; // Maxium Level allowed to join the Instance.

	if (min < 1 || min > MAX_LEVEL) {
		ShowError("buildin_instance_check_clan: Invalid min level, %d\n", min);
		return SCRIPT_CMD_FAILURE;
	} else if (max < 1 || max > MAX_LEVEL) {
		ShowError("buildin_instance_check_clan: Invalid max level, %d\n", max);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st,2))
		clan_id = script_getnum(st,2);
	else
		return SCRIPT_CMD_FAILURE;

	if (!(cd = clan_search(clan_id))) {
		script_pushint(st, 0); // Returns false if clan does not exist.
		return SCRIPT_CMD_FAILURE;
	}

	for(i = 0; i < MAX_CLAN; i++) {
		struct map_session_data *pl_sd;

		if ((pl_sd = cd->members[i])) {
			if (map_id2bl(pl_sd->bl.id)) {
				if (pl_sd->status.base_level < min) {
					script_pushint(st, 0);
					return SCRIPT_CMD_SUCCESS;
				} else if (pl_sd->status.base_level > max) {
					script_pushint(st, 0);
					return SCRIPT_CMD_SUCCESS;
				}
				c++;
			}
		}
	}

	if (c < amount)
		script_pushint(st, 0); // Not enough Members in the Clan to join Instance.
	else
		script_pushint(st, 1);

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
* instance_info
* Values:
* name : name of the instance you want to look up. [Required Parameter]
* type : type of information you want to look up for the specified instance. [Required Parameter]
* index : index of the map in the instance. [Optional Parameter]
*------------------------------------------*/
BUILDIN_FUNC(instance_info)
{
	const char* name = script_getstr(st, 2);
	int type = script_getnum(st, 3);
	int index = 0;
	struct instance_db *db = instance_searchname_db(name);

	if( !db ){
		ShowError( "buildin_instance_info: Unknown instance name \"%s\".\n", name );
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	switch( type ){
		case IIT_ID:
			script_pushint(st, db->id);
			break;
		case IIT_TIME_LIMIT:
			script_pushint(st, db->limit);
			break;
		case IIT_IDLE_TIMEOUT:
			script_pushint(st, db->timeout);
			break;
		case IIT_ENTER_MAP:
			script_pushstrcopy(st, StringBuf_Value(db->enter.mapname));
			break;
		case IIT_ENTER_X:
			script_pushint(st, db->enter.x);
			break;
		case IIT_ENTER_Y:
			script_pushint(st, db->enter.y);
			break;
		case IIT_MAPCOUNT:
			script_pushint(st, db->maplist_count);
			break;
		case IIT_MAP:
			if( !script_hasdata(st, 4) || script_isstring(st, 4) ){
				ShowError( "buildin_instance_info: Type IIT_MAP requires a numeric index argument.\n" );
				script_pushconststr(st, "");
				return SCRIPT_CMD_FAILURE;
			}
			
			index = script_getnum(st, 4);

			if( index < 0 ){
				ShowError( "buildin_instance_info: Type IIT_MAP does not support a negative index argument.\n" );
				script_pushconststr(st, "");
				return SCRIPT_CMD_FAILURE;
			}

			if( index > UINT8_MAX ){
				ShowError( "buildin_instance_info: Type IIT_MAP does only support up to index %hu.\n", UINT8_MAX );
				script_pushconststr(st, "");
				return SCRIPT_CMD_FAILURE;
			}

			script_pushstrcopy(st, StringBuf_Value(db->maplist[index]));
			break;

		default:
			ShowError("buildin_instance_info: Unknown instance information type \"%d\".\n", type );
			script_pushint(st, -1);
			return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * Custom Fonts
 *------------------------------------------*/
BUILDIN_FUNC(setfont)
{
	struct map_session_data *sd;
	int font;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	font = script_getnum(st,2);

	if( sd->status.font != font )
		sd->status.font = font;
	else
		sd->status.font = 0;

	clif_font(sd);
	return SCRIPT_CMD_SUCCESS;
}

static int buildin_mobuseskill_sub(struct block_list *bl,va_list ap)
{
	TBL_MOB* md		= (TBL_MOB*)bl;
	struct block_list *tbl;
	int mobid		= va_arg(ap,int);
	uint16 skill_id	= va_arg(ap,int);
	uint16 skill_lv	= va_arg(ap,int);
	int casttime	= va_arg(ap,int);
	int cancel		= va_arg(ap,int);
	int emotion		= va_arg(ap,int);
	int target		= va_arg(ap,int);

	if( md->mob_id != mobid )
		return 0;

	// 0:self, 1:target, 2:master, default:random
	switch( target ) {
		case 0: tbl = map_id2bl(md->bl.id); break;
		case 1: tbl = map_id2bl(md->target_id); break;
		case 2: tbl = map_id2bl(md->master_id); break;
		default:tbl = battle_getenemy(&md->bl, DEFAULT_ENEMY_TYPE(md), skill_get_range2(&md->bl, skill_id, skill_lv, true)); break;
	}

	if( !tbl )
		return 0;

	if( md->ud.skilltimer != INVALID_TIMER ) // Cancel the casting skill.
		unit_skillcastcancel(bl,0);

	if( skill_get_casttype(skill_id) == CAST_GROUND )
		unit_skilluse_pos2(&md->bl, tbl->x, tbl->y, skill_id, skill_lv, casttime, cancel);
	else
		unit_skilluse_id2(&md->bl, tbl->id, skill_id, skill_lv, casttime, cancel);

	clif_emotion(&md->bl, emotion);

	return 1;
}

/*==========================================
 * areamobuseskill "Map Name",<x>,<y>,<range>,<Mob ID>,"Skill Name"/<Skill ID>,<Skill Lv>,<Cast Time>,<Cancelable>,<Emotion>,<Target Type>;
 *------------------------------------------*/
BUILDIN_FUNC(areamobuseskill)
{
	struct block_list center;
	struct script_data *data;
	int16 m;
	int range,mobid,skill_id,skill_lv,casttime,emotion,target,cancel;

	if( (m = map_mapname2mapid(script_getstr(st,2))) < 0 ) {
		ShowError("areamobuseskill: invalid map name.\n");
		return SCRIPT_CMD_SUCCESS;
	}

	center.m = m;
	center.x = script_getnum(st,3);
	center.y = script_getnum(st,4);
	range = script_getnum(st,5);
	mobid = script_getnum(st,6);
	data = script_getdata(st, 7);
	get_val(st, data); // Convert into value in case of a variable
	skill_id = ( data_isstring(data) ? skill_name2id(script_getstr(st,7)) : script_getnum(st,7) );
	if( (skill_lv = script_getnum(st,8)) > battle_config.mob_max_skilllvl )
		skill_lv = battle_config.mob_max_skilllvl;

	casttime = script_getnum(st,9);
	cancel = script_getnum(st,10);
	emotion = script_getnum(st,11);
	target = script_getnum(st,12);

	map_foreachinallrange(buildin_mobuseskill_sub, &center, range, BL_MOB, mobid, skill_id, skill_lv, casttime, cancel, emotion, target);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Display a progress bar above a character
 * progressbar "<color>",<seconds>;
 */
BUILDIN_FUNC(progressbar)
{
	struct map_session_data * sd;
	const char * color;
	unsigned int second;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;

	st->state = STOP;

	color = script_getstr(st,2);
	second = script_getnum(st,3);

	sd->progressbar.npc_id = st->oid;
	sd->progressbar.timeout = gettick() + second*1000;
	sd->state.workinprogress = WIP_DISABLE_ALL;

	clif_progressbar(sd, strtol(color, (char **)NULL, 0), second);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Display a progress bar above an NPC
 * progressbar_npc "<color>",<seconds>{,<"NPC Name">};
 */
BUILDIN_FUNC(progressbar_npc){
	struct npc_data* nd = NULL;

	if( script_hasdata(st, 4) ){
		const char* name = script_getstr(st, 4);

		nd = npc_name2id(name);

		if( !nd ){
			ShowError( "buildin_progressbar_npc: NPC \"%s\" was not found.\n", name );
			return SCRIPT_CMD_FAILURE;
		}
	}else{
		nd = map_id2nd(st->oid);
	}

	// First call(by function call)
	if( !nd->progressbar.timeout ){
		const char *color;
		int second;

		color = script_getstr(st, 2);
		second = script_getnum(st, 3);

		if( second < 0 ){
			ShowError( "buildin_progressbar_npc: negative amount('%d') of seconds is not supported\n", second );
			return SCRIPT_CMD_FAILURE;
		}

		// detach the player
		script_detach_rid(st);

		// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = second * 1000;
		nd->progressbar.timeout = gettick() + second * 1000;
		nd->progressbar.color = strtol(color, (char **)NULL, 0);

		clif_progressbar_npc_area(nd);
	// Second call(by timer after sleeping time is over)
	} else {
		// Continue the script
		st->state = RUN;
		st->sleep.tick = 0;
		nd->progressbar.timeout = nd->progressbar.color = 0;
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(pushpc)
{
	uint8 dir;
	int cells, dx, dy;
	struct map_session_data* sd;

	if(!script_rid2sd(sd))
	{
		return SCRIPT_CMD_SUCCESS;
	}

	dir = script_getnum(st,2);
	cells     = script_getnum(st,3);

	if(dir >= DIR_MAX)
	{
		ShowWarning("buildin_pushpc: Invalid direction %d specified.\n", dir);
		script_reportsrc(st);

		dir%= DIR_MAX;  // trim spin-over
	}

	if(!cells)
	{// zero distance
		return SCRIPT_CMD_SUCCESS;
	}
	else if(cells<0)
	{// pushing backwards
		dir = (dir+DIR_MAX/2)%DIR_MAX;  // turn around
		cells     = -cells;
	}

	dx = dirx[dir];
	dy = diry[dir];

	unit_blown(&sd->bl, dx, dy, cells, BLOWN_NONE);
	return SCRIPT_CMD_SUCCESS;
}


/// Invokes buying store preparation window
/// buyingstore <slots>;
BUILDIN_FUNC(buyingstore)
{
	struct map_session_data* sd;

	if( !script_rid2sd(sd) )
	{
		return SCRIPT_CMD_SUCCESS;
	}

	if( npc_isnear(&sd->bl) ) {
		char output[150];
		sprintf(output, msg_txt(sd,662), battle_config.min_npc_vendchat_distance);
		clif_displaymessage(sd->fd, output);
		return SCRIPT_CMD_SUCCESS;
	}

	buyingstore_setup(sd, script_getnum(st,2));
	return SCRIPT_CMD_SUCCESS;
}


/// Invokes search store info window
/// searchstores <uses>,<effect>;
BUILDIN_FUNC(searchstores)
{
	unsigned short effect;
	unsigned int uses;
	struct map_session_data* sd;

	if( !script_rid2sd(sd) )
	{
		return SCRIPT_CMD_SUCCESS;
	}

	uses   = script_getnum(st,2);
	effect = script_getnum(st,3);

	if( !uses )
	{
		ShowError("buildin_searchstores: Amount of uses cannot be zero.\n");
		return SCRIPT_CMD_FAILURE;
	}

	if( effect > 1 )
	{
		ShowError("buildin_searchstores: Invalid effect id %hu, specified.\n", effect);
		return SCRIPT_CMD_FAILURE;
	}

	searchstore_open(sd, uses, effect);
	return SCRIPT_CMD_SUCCESS;
}
/// Displays a number as large digital clock.
/// showdigit <value>[,<type>];
BUILDIN_FUNC(showdigit)
{
	unsigned int type = 0;
	int value;
	struct map_session_data* sd;

	if( !script_rid2sd(sd) )
	{
		return SCRIPT_CMD_SUCCESS;
	}

	value = script_getnum(st,2);

	if( script_hasdata(st,3) ){
		type = script_getnum(st,3);
	}

	switch( type ){
		case 0:
			break;
		case 1:
		case 2:
			// Use absolute value and then the negative value of it as starting value
			// This is what gravity's client does for these counters
			value = -abs(value);
			break;
		case 3:
			value = abs(value);
			if( value > 99 ){
				ShowWarning("buildin_showdigit: type 3 can display 2 digits at max. Capping value %d to 99...\n", value);
				script_reportsrc(st);
				value = 99;
			}
			break;
		default:
			ShowError("buildin_showdigit: Invalid type %u.\n", type);
			return SCRIPT_CMD_FAILURE;
	}

	clif_showdigit(sd, (unsigned char)type, value);
	return SCRIPT_CMD_SUCCESS;
}
/**
 * Rune Knight
 * makerune <% success bonus>{,<char_id>};
 **/
BUILDIN_FUNC(makerune) {
	TBL_PC* sd;
	
	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;
	clif_skill_produce_mix_list(sd,RK_RUNEMASTERY,24);
	sd->itemid = script_getnum(st,2);
	return SCRIPT_CMD_SUCCESS;
}
/**
 * checkdragon({<char_id>}) returns 1 if mounting a dragon or 0 otherwise.
 **/
BUILDIN_FUNC(checkdragon) {
	TBL_PC* sd;
	
	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	if( pc_isridingdragon(sd) )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}
/**
 * setdragon({optional Color{,<char_id>}}) returns 1 on success or 0 otherwise
 * - Toggles the dragon on a RK if he can mount;
 * @param Color - when not provided uses the green dragon;
 * - 1 : Green Dragon
 * - 2 : Brown Dragon
 * - 3 : Gray Dragon
 * - 4 : Blue Dragon
 * - 5 : Red Dragon
 **/
BUILDIN_FUNC(setdragon) {
	TBL_PC* sd;
	int color = script_hasdata(st,2) ? script_getnum(st,2) : 0;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;
	if( !pc_checkskill(sd,RK_DRAGONTRAINING) || (sd->class_&MAPID_THIRDMASK) != MAPID_RUNE_KNIGHT )
		script_pushint(st,0);//Doesn't have the skill or it's not a Rune Knight
	else if ( pc_isridingdragon(sd) ) {//Is mounted; release
		pc_setoption(sd, sd->sc.option&~OPTION_DRAGON);
		script_pushint(st,1);
	} else {//Not mounted; Mount now.
		unsigned int option = OPTION_DRAGON1;
		if( color ) {
			option = ( color == 1 ? OPTION_DRAGON1 :
					   color == 2 ? OPTION_DRAGON2 :
					   color == 3 ? OPTION_DRAGON3 :
					   color == 4 ? OPTION_DRAGON4 :
					   color == 5 ? OPTION_DRAGON5 : 0);
			if( !option ) {
				ShowWarning("script_setdragon: Unknown Color %d used; changing to green (1)\n",color);
				option = OPTION_DRAGON1;
			}
		}
		pc_setoption(sd, sd->sc.option|option);
		script_pushint(st,1);
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
 * ismounting({<char_id>}) returns 1 if mounting a new mount or 0 otherwise
 **/
BUILDIN_FUNC(ismounting) {
	TBL_PC* sd;
	
	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	if( sd->sc.data[SC_ALL_RIDING] )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * setmounting({<char_id>}) returns 1 on success or 0 otherwise
 * - Toggles new mounts on a player when he can mount
 * - Will fail if the player is mounting a non-new mount, e.g. dragon, peco, wug, etc.
 * - Will unmount the player is he is already mounting
 **/
BUILDIN_FUNC(setmounting) {
	TBL_PC* sd;
	
	if (!script_charid2sd(2,sd))
		return SCRIPT_CMD_FAILURE;
	if( sd->sc.option&(OPTION_WUGRIDER|OPTION_RIDING|OPTION_DRAGON|OPTION_MADOGEAR) ) {
		clif_msg(sd, NEED_REINS_OF_MOUNT);
		script_pushint(st,0); //can't mount with one of these
	} else {
		if( sd->sc.data[SC_ALL_RIDING] )
			status_change_end(&sd->bl, SC_ALL_RIDING, INVALID_TIMER); //release mount
		else
			sc_start(NULL, &sd->bl, SC_ALL_RIDING, 10000, 1, INVALID_TIMER); //mount
		script_pushint(st,1);//in both cases, return 1.
	}
	return SCRIPT_CMD_SUCCESS;
}
/**
 * Retrieves quantity of arguments provided to callfunc/callsub.
 * getargcount() -> amount of arguments received in a function
 **/
BUILDIN_FUNC(getargcount) {
	struct script_retinfo* ri;

	if( st->stack->defsp < 1 || st->stack->stack_data[st->stack->defsp - 1].type != C_RETINFO ) {
		ShowError("script:getargcount: used out of function or callsub label!\n");
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}
	ri = st->stack->stack_data[st->stack->defsp - 1].u.ri;

	script_pushint(st, ri->nargs);
	return SCRIPT_CMD_SUCCESS;
}
/**
 * getcharip(<account ID>/<character ID>/<character name>)
 **/
BUILDIN_FUNC(getcharip)
{
	struct map_session_data* sd = NULL;

	/* check if a character name is specified */
	if( script_hasdata(st, 2) )
	{
		struct script_data *data;

		data = script_getdata(st, 2);
		get_val(st, data); // Convert into value in case of a variable
		if (data_isstring(data))
			sd = map_nick2sd(script_getstr(st, 2),false);
		else if (data_isint(data) || script_getnum(st, 2))
		{
			int id = 0;
			id = script_getnum(st, 2);
			sd = (map_id2sd(id) ? map_id2sd(id) : map_charid2sd(id));
		}
	}
	else
		script_rid2sd(sd);

	/* check for sd and IP */
	if (!sd || !session[sd->fd]->client_addr)
	{
		script_pushconststr(st, "");
		return SCRIPT_CMD_SUCCESS;
	}

	/* return the client ip_addr converted for output */
	if (sd && sd->fd && session[sd->fd])
	{
		/* initiliaze */
		const char *ip_addr = NULL;
		uint32 ip;

		/* set ip, ip_addr and convert to ip and push str */
		ip = session[sd->fd]->client_addr;
		ip_addr = ip2str(ip, NULL);
		script_pushstrcopy(st, ip_addr);
	}
	return SCRIPT_CMD_SUCCESS;
}
/**
 * is_function(<function name>) -> 1 if function exists, 0 otherwise
 **/
BUILDIN_FUNC(is_function) {
	const char* str = script_getstr(st,2);

	if( strdb_exists(userfunc_db, str) )
		script_pushint(st,1);
	else
		script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
}
/**
 * get_revision() -> retrieves the current svn revision (if available)
 **/
BUILDIN_FUNC(get_revision) {
	const char *svn = get_svn_revision();

	if ( svn[0] != UNKNOWN_VERSION )
		script_pushint(st,atoi(svn));
	else
		script_pushint(st,-1); //unknown
	return SCRIPT_CMD_SUCCESS;
}
/* get_hash() -> retrieves the current git hash (if available)*/
BUILDIN_FUNC(get_githash) {
	const char* git = get_git_hash();
	char buf[CHAT_SIZE_MAX];
	safestrncpy(buf,git,strlen(git)+1);

	if ( git[0] != UNKNOWN_VERSION )
		script_pushstr(st,buf);
	else
		script_pushconststr(st,"Unknown"); //unknown
	return SCRIPT_CMD_SUCCESS;
}
/**
 * freeloop(<toggle>) -> toggles this script instance's looping-check ability
 **/
BUILDIN_FUNC(freeloop) {

	if( script_hasdata(st,2) ) {
		if( script_getnum(st,2) )
			st->freeloop = 1;
		else
			st->freeloop = 0;
	}

	script_pushint(st, st->freeloop);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * @commands (script based)
 **/
BUILDIN_FUNC(bindatcmd) {
	const char* atcmd;
	const char* eventName;
	int i, level = 0, level2 = 100;
	bool create = false;

	atcmd = script_getstr(st,2);
	eventName = script_getstr(st,3);

	if( *atcmd == atcommand_symbol || *atcmd == charcommand_symbol )
		atcmd++;

	if( script_hasdata(st,4) ) level = script_getnum(st,4);
	if( script_hasdata(st,5) ) level2 = script_getnum(st,5);

	if( atcmd_binding_count == 0 ) {
		CREATE(atcmd_binding,struct atcmd_binding_data*,1);

		create = true;
	} else {
		ARR_FIND(0, atcmd_binding_count, i, strcmp(atcmd_binding[i]->command,atcmd) == 0);
		if( i < atcmd_binding_count ) {/* update existent entry */
			safestrncpy(atcmd_binding[i]->npc_event, eventName, EVENT_NAME_LENGTH);
			atcmd_binding[i]->level = level;
			atcmd_binding[i]->level2 = level2;
		} else
			create = true;
	}

	if( create ) {
		i = atcmd_binding_count;

		if( atcmd_binding_count++ != 0 )
			RECREATE(atcmd_binding,struct atcmd_binding_data*,atcmd_binding_count);

		CREATE(atcmd_binding[i],struct atcmd_binding_data,1);

		safestrncpy(atcmd_binding[i]->command, atcmd, 50);
		safestrncpy(atcmd_binding[i]->npc_event, eventName, EVENT_NAME_LENGTH);
		atcmd_binding[i]->level = level;
		atcmd_binding[i]->level2 = level2;
	}
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(unbindatcmd) {
	const char* atcmd;
	int i =  0;

	atcmd = script_getstr(st, 2);

	if( *atcmd == atcommand_symbol || *atcmd == charcommand_symbol )
		atcmd++;

	if( atcmd_binding_count == 0 ) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	ARR_FIND(0, atcmd_binding_count, i, strcmp(atcmd_binding[i]->command, atcmd) == 0);
	if( i < atcmd_binding_count ) {
		int cursor = 0;
		aFree(atcmd_binding[i]);
		atcmd_binding[i] = NULL;
		/* compact the list now that we freed a slot somewhere */
		for( i = 0, cursor = 0; i < atcmd_binding_count; i++ ) {
			if( atcmd_binding[i] == NULL )
				continue;

			if( cursor != i ) {
				memmove(&atcmd_binding[cursor], &atcmd_binding[i], sizeof(struct atcmd_binding_data*));
			}

			cursor++;
		}

		if( (atcmd_binding_count = cursor) == 0 )
			aFree(atcmd_binding);

		script_pushint(st, 1);
	} else
		script_pushint(st, 0);/* not found */

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(useatcmd) {
	return atcommand_sub(st,3);
}

BUILDIN_FUNC(checkre)
{
	int num;

	num=script_getnum(st,2);
	switch(num){
		case 0:
			#ifdef RENEWAL
				script_pushint(st, 1);
			#else
				script_pushint(st, 0);
			#endif
			break;
		case 1:
			#ifdef RENEWAL_CAST
				script_pushint(st, 1);
			#else
				script_pushint(st, 0);
			#endif
			break;
		case 2:
			#ifdef RENEWAL_DROP
				script_pushint(st, 1);
			#else
				script_pushint(st, 0);
			#endif
			break;
		case 3:
			#ifdef RENEWAL_EXP
				script_pushint(st, 1);
			#else
				script_pushint(st, 0);
			#endif
			break;
		case 4:
			#ifdef RENEWAL_LVDMG
				script_pushint(st, 1);
			#else
				script_pushint(st, 0);
			#endif
			break;
		case 5:
			#ifdef RENEWAL_ASPD
				script_pushint(st, 1);
			#else
				script_pushint(st, 0);
			#endif
			break;
		default:
			ShowWarning("buildin_checkre: unknown parameter.\n");
			break;
	}
	return SCRIPT_CMD_SUCCESS;
}

/* getrandgroupitem <group_id>{,<quantity>{,<sub_group>{,<identify>{,<char_id>}}}} */
BUILDIN_FUNC(getrandgroupitem) {
	TBL_PC* sd;
	int i, get_count = 0, identify = 0;
	uint16 group, qty = 0;
	uint8 sub_group = 1;
	struct item item_tmp;
	struct s_item_group_entry *entry = NULL;

	if (!script_charid2sd(6, sd))
		return SCRIPT_CMD_SUCCESS;

	group = script_getnum(st,2);

	if (!group) {
		ShowError("buildin_getrandgroupitem: Invalid group id (%d)!\n",script_getnum(st,2));
		return SCRIPT_CMD_FAILURE;
	}

	FETCH(3, qty);
	FETCH(4, sub_group);
	FETCH(5, identify);

	entry = itemdb_get_randgroupitem(group,sub_group);
	if (!entry)
		return SCRIPT_CMD_FAILURE; //ensure valid itemid

	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid   = entry->nameid;
	item_tmp.identify = identify ? 1 : itemdb_isidentified(entry->nameid);

	if (!qty)
		qty = entry->amount;

	//Check if it's stackable.
	if (!itemdb_isstackable(entry->nameid)) {
		item_tmp.amount = 1;
		get_count = qty;
	}
	else {
		item_tmp.amount = qty;
		get_count = 1;
	}

	for (i = 0; i < get_count; i++) {
		// if not pet egg
		if (!pet_create_egg(sd, entry->nameid)) {
			unsigned char flag = 0;
			if ((flag = pc_additem(sd,&item_tmp,item_tmp.amount,LOG_TYPE_SCRIPT))) {
				clif_additem(sd,0,0,flag);
				if (pc_candrop(sd,&item_tmp))
					map_addflooritem(&item_tmp,item_tmp.amount,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0,0);
			}
		}
	}

	return SCRIPT_CMD_SUCCESS;
}

/* getgroupitem <group_id>{,<identify>{,<char_id>}};
 * Gives item(s) to the attached player based on item group contents
 */
BUILDIN_FUNC(getgroupitem) {
	TBL_PC *sd;
	int group_id = script_getnum(st,2);
	
	if (!script_charid2sd(4,sd))
		return SCRIPT_CMD_SUCCESS;
	
	if (itemdb_pc_get_itemgroup(group_id, (script_hasdata(st, 3) ? script_getnum(st, 3) != 0 : false), sd)) {
		ShowError("buildin_getgroupitem: Invalid group id '%d' specified.\n",group_id);
		return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}

/* cleanmap <map_name>;
 * cleanarea <map_name>, <x0>, <y0>, <x1>, <y1>; */
static int atcommand_cleanfloor_sub(struct block_list *bl, va_list ap)
{
	nullpo_ret(bl);
	map_clearflooritem(bl);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(cleanmap)
{
	const char *mapname;
	int16 m;

	mapname = script_getstr(st, 2);
	m = map_mapname2mapid(mapname);
	if (!m)
		return SCRIPT_CMD_FAILURE;

	if ((script_lastdata(st) - 2) < 4) {
		map_foreachinmap(atcommand_cleanfloor_sub, m, BL_ITEM);
	} else {
		int16 x0 = script_getnum(st, 3);
		int16 y0 = script_getnum(st, 4);
		int16 x1 = script_getnum(st, 5);
		int16 y1 = script_getnum(st, 6);
		if (x0 > 0 && y0 > 0 && x1 > 0 && y1 > 0) {
			map_foreachinallarea(atcommand_cleanfloor_sub, m, x0, y0, x1, y1, BL_ITEM);
		} else {
			ShowError("cleanarea: invalid coordinate defined!\n");
			return SCRIPT_CMD_FAILURE;
		}
	}
	return SCRIPT_CMD_SUCCESS;
}

/* Cast a skill on the attached player.
 * npcskill <skill id>, <skill lvl>, <stat point>, <NPC level>;
 * npcskill "<skill name>", <skill lvl>, <stat point>, <NPC level>; */
BUILDIN_FUNC(npcskill)
{
	uint16 skill_id;
	unsigned short skill_level;
	unsigned int stat_point;
	unsigned int npc_level;
	struct npc_data *nd;
	struct map_session_data *sd;
	struct script_data *data;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_SUCCESS;
	
	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	skill_id	= data_isstring(data) ? skill_name2id(script_getstr(st, 2)) : script_getnum(st, 2);
	skill_level	= script_getnum(st, 3);
	stat_point	= script_getnum(st, 4);
	npc_level	= script_getnum(st, 5);
	nd			= (struct npc_data *)map_id2bl(sd->npc_id);

	if (stat_point > battle_config.max_third_parameter) {
		ShowError("npcskill: stat point exceeded maximum of %d.\n",battle_config.max_third_parameter );
		return SCRIPT_CMD_FAILURE;
	}
	if (npc_level > MAX_LEVEL) {
		ShowError("npcskill: level exceeded maximum of %d.\n", MAX_LEVEL);
		return SCRIPT_CMD_FAILURE;
	}
	if (nd == NULL) { //ain't possible, but I don't trust people.
		return SCRIPT_CMD_FAILURE;
	}

	nd->level = npc_level;
	nd->stat_point = stat_point;

	if (!nd->status.hp)
		status_calc_npc(nd, SCO_FIRST);
	else
		status_calc_npc(nd, SCO_NONE);

	if (skill_get_inf(skill_id)&INF_GROUND_SKILL)
		unit_skilluse_pos2(&nd->bl, sd->bl.x, sd->bl.y, skill_id, skill_level,0,0);
	else
		unit_skilluse_id2(&nd->bl, sd->bl.id, skill_id, skill_level,0,0);

	return SCRIPT_CMD_SUCCESS;
}

/* Consumes an item.
 * consumeitem <item id>{,<char_id>};
 * consumeitem "<item name>"{,<char_id>};
 * @param item: Item ID or name
 */
BUILDIN_FUNC(consumeitem)
{
	TBL_PC *sd;
	struct script_data *data;
	struct item_data *item_data;

	if (!script_charid2sd(3, sd))
		return SCRIPT_CMD_FAILURE;

	data = script_getdata( st, 2 );
	get_val( st, data );

	if( data_isstring( data ) ){
		const char *name = conv_str( st, data );

		if( ( item_data = itemdb_searchname( name ) ) == NULL ){
			ShowError( "buildin_consumeitem: Nonexistant item %s requested.\n", name );
			return SCRIPT_CMD_FAILURE;
		}
	}else if( data_isint( data ) ){
		unsigned short nameid = conv_num( st, data );

		if( ( item_data = itemdb_exists( nameid ) ) == NULL ){
			ShowError("buildin_consumeitem: Nonexistant item %hu requested.\n", nameid );
			return SCRIPT_CMD_FAILURE;
		}
	}else{
		ShowError("buildin_consumeitem: invalid data type for argument #1 (%d).\n", data->type );
		return SCRIPT_CMD_FAILURE;
	}

	run_script( item_data->script, 0, sd->bl.id, 0 );
	return SCRIPT_CMD_SUCCESS;
}

/** Makes a player sit/stand.
 * sit {"<character name>"};
 * stand {"<character name>"};
 * Note: Use readparam(Sitting) which returns 1 or 0 (sitting or standing).
 * @param name: Player name that will be invoked
 */
BUILDIN_FUNC(sit)
{
	TBL_PC *sd;

	if( !script_nick2sd(2,sd) )
		return SCRIPT_CMD_FAILURE;

	if( !pc_issit(sd) ) {
		pc_setsit(sd);
		skill_sit(sd, 1);
		clif_sitting(&sd->bl);
	}
	return SCRIPT_CMD_SUCCESS;
}

/** Makes player to stand
* @param name: Player name that will be set
*/
BUILDIN_FUNC(stand)
{
	TBL_PC *sd;

	if( !script_nick2sd(2,sd) )
		return SCRIPT_CMD_FAILURE;

	if( pc_issit(sd) && pc_setstand(sd, false)) {
		skill_sit(sd, 0);
		clif_standing(&sd->bl);
	}

	return SCRIPT_CMD_SUCCESS;
}

/** Creates an array of bounded item IDs
 * countbound {<type>{,<char_id>}};
 * @param type: 0 - All bound items; 1 - Account Bound; 2 - Guild Bound; 3 - Party Bound
 * @return amt: Amount of items found
 */
BUILDIN_FUNC(countbound)
{
	int i, type, j = 0, k = 0;
	TBL_PC *sd;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	type = script_getnum(st,2);

	for( i = 0; i < MAX_INVENTORY; i ++ ) {
		if( sd->inventory.u.items_inventory[i].nameid > 0 && (
			(!type && sd->inventory.u.items_inventory[i].bound) || (type && sd->inventory.u.items_inventory[i].bound == type)
			))
		{
			pc_setreg(sd,reference_uid(add_str("@bound_items"), k),sd->inventory.u.items_inventory[i].nameid);
			k++;
			j += sd->inventory.u.items_inventory[i].amount;
		}
	}

	script_pushint(st,j);
	return SCRIPT_CMD_SUCCESS;
}

/** Creates new party
 * party_create "<party name>"{,<char id>{,<item share>{,<item share type>}}};
 * @param party_name: String of party name that will be created
 * @param char_id: Chara that will be leader of this new party. If no char_id specified, the invoker will be party leader
 * @param item_share: 0-Each Take. 1-Party Share
 * @param item_share_type: 0-Each Take. 1-Even Share
 * @return val: Result value
 *	-3	- party name is exist
 *	-2	- player is in party already
 *	-1	- player is not found
 *	0	- unknown error
 *	1	- success, will return party id $@party_create_id
 */
BUILDIN_FUNC(party_create)
{
	char party_name[NAME_LENGTH];
	int item1 = 0, item2 = 0;
	TBL_PC *sd = NULL;

	if (!script_charid2sd(3, sd)) {
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if( sd->status.party_id ) {
		script_pushint(st,-2);
		return SCRIPT_CMD_FAILURE;
	}

	safestrncpy(party_name,script_getstr(st,2),NAME_LENGTH);
	trim(party_name);
	if( party_searchname(party_name) ) {
		script_pushint(st,-3);
		return SCRIPT_CMD_FAILURE;
	}
	if( script_getnum(st,4) )
		item1 = 1;
	if( script_getnum(st,5) )
		item2 = 1;

	party_create_byscript = 1;
	script_pushint(st,party_create(sd,party_name,item1,item2));
	return SCRIPT_CMD_SUCCESS;
}

/** Adds player to specified party
 * party_addmember <party id>,<char id>;
 * @param party_id: The party that will be entered by player
 * @param char_id: Char id of player that will be joined to the party
 * @return val: Result value
 *	-5	- another character of the same account is in the party
 *	-4	- party is full
 *	-3	- party is not found
 *	-2	- player is in party already
 *	-1	- player is not found
 *	0	- unknown error
 *	1	- success
 */
BUILDIN_FUNC(party_addmember)
{
	int party_id = script_getnum(st,2);
	TBL_PC *sd;
	struct party_data *party;

	if( !(sd = map_charid2sd(script_getnum(st,3))) ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if( sd->status.party_id ) {
		script_pushint(st,-2);
		return SCRIPT_CMD_FAILURE;
	}

	if( !(party = party_search(party_id)) ) {
		script_pushint(st,-3);
		return SCRIPT_CMD_FAILURE;
	}

	if (battle_config.block_account_in_same_party) {
		int i;
		ARR_FIND(0, MAX_PARTY, i, party->party.member[i].account_id == sd->status.account_id);
		if (i < MAX_PARTY) {
			script_pushint(st,-5);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if( party->party.count >= MAX_PARTY ) {
		script_pushint(st,-4);
		return SCRIPT_CMD_FAILURE;
	}
	sd->party_invite = party_id;
	script_pushint(st,party_add_member(party_id,sd));
	return SCRIPT_CMD_SUCCESS;
}

/** Removes player from his/her party. If party_id and char_id is empty remove the invoker from his/her party
 * party_delmember {<char id>,<party_id>};
 * @param: char_id
 * @param: party_id
 * @return val: Result value
 *	-3	- player is not in party
 *	-2	- party is not found
 *	-1	- player is not found
 *	0	- unknown error
 *	1	- success
 */
BUILDIN_FUNC(party_delmember)
{
	TBL_PC *sd = NULL;

	if( !script_hasdata(st,2) && !script_hasdata(st,3) && !script_rid2sd(sd) ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}
	if( sd || script_charid2sd(2,sd) )
		script_pushint(st,party_removemember2(sd,0,0));
	else
		script_pushint(st,party_removemember2(NULL,script_getnum(st,2),script_getnum(st,3)));
	return SCRIPT_CMD_SUCCESS;
}

/** Changes party leader of specified party (even the leader is offline)
 * party_changeleader <party id>,<char id>;
 * @param party_id: ID of party
 * @param char_id: Char ID of new leader
 * @return val: Result value
 *	-4	- player is party leader already
 *	-3	- player is not in this party
 *	-2	- player is not found
 *	-1	- party is not found
 *	0	- unknown error
 *	1	- success */
BUILDIN_FUNC(party_changeleader)
{
	int i, party_id = script_getnum(st,2);
	TBL_PC *sd = NULL;
	TBL_PC *tsd = NULL;
	struct party_data *party = NULL;

	if( !(party = party_search(party_id)) ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	if( !(tsd = map_charid2sd(script_getnum(st,3))) ) {
		script_pushint(st,-2);
		return SCRIPT_CMD_FAILURE;
	}

	if( tsd->status.party_id != party_id ) {
		script_pushint(st,-3);
		return SCRIPT_CMD_FAILURE;
	}

	ARR_FIND(0,MAX_PARTY,i,party->party.member[i].leader);
	if( i >= MAX_PARTY ) {	//this is should impossible!
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}
	if( party->data[i].sd == tsd ) {
		script_pushint(st,-4);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,party_changeleader(sd,tsd,party));
	return SCRIPT_CMD_SUCCESS;
}

/** Changes party option
 * party_changeoption <party id>,<option>,<flag>;
 * @param party_id: ID of party that will be changed
 * @param option: Type of option
 * @return val: -1 - party is not found, 0 - invalid option, 1 - success
 */
BUILDIN_FUNC(party_changeoption)
{
	struct party_data *party;

	if( !(party = party_search(script_getnum(st,2))) ) {
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}
	script_pushint(st,party_setoption(party,script_getnum(st,3),script_getnum(st,4)));
	return SCRIPT_CMD_SUCCESS;
}

/** Destroys party with party id.
 * party_destroy <party id>;
 * @param party_id: ID of party that will be destroyed
 */
BUILDIN_FUNC(party_destroy)
{
	int i;
	struct party_data *party;

	if( !(party = party_search(script_getnum(st,2))) ) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	ARR_FIND(0,MAX_PARTY,i,party->party.member[i].leader);
	if( i >= MAX_PARTY || !party->data[i].sd ) { //leader not online
		int j;
		for( j = 0; j < MAX_PARTY; j++ ) {
			TBL_PC *sd = party->data[j].sd;
			if(sd)
				party_member_withdraw(party->party.party_id,sd->status.account_id,sd->status.char_id,sd->status.name,PARTY_MEMBER_WITHDRAW_LEAVE);
			else if( party->party.member[j].char_id )
				intif_party_leave(party->party.party_id,party->party.member[j].account_id,party->party.member[j].char_id,party->party.member[j].name,PARTY_MEMBER_WITHDRAW_LEAVE);
		}
		party_broken(party->party.party_id);
		script_pushint(st,1);
	}
	else //leader leave = party broken
		script_pushint(st,party_leave(party->data[i].sd));
	return SCRIPT_CMD_SUCCESS;
}

/** Returns various information about a player's VIP status. Need to enable VIP system
 * vip_status <type>,{"<character name>"};
 * @param type: Info type, see enum vip_status_type
 * @param name: Character name (Optional)
 */
BUILDIN_FUNC(vip_status) {
#ifdef VIP_ENABLE
	TBL_PC *sd;
	int type;

	if( !script_nick2sd(3,sd) )
		return SCRIPT_CMD_FAILURE;

	type = script_getnum(st, 2);

	switch(type) {
		case VIP_STATUS_ACTIVE: // Get VIP status.
			script_pushint(st, pc_isvip(sd));
			break;
		case VIP_STATUS_EXPIRE: // Get VIP expire date.
			if (pc_isvip(sd)) {
				script_pushint(st, sd->vip.time);
			} else
				script_pushint(st, 0);
			break;
		case VIP_STATUS_REMAINING: // Get remaining time.
			if (pc_isvip(sd)) {
				script_pushint(st, sd->vip.time - time(NULL));
			} else
				script_pushint(st, 0);
			break;
		default:
			ShowError( "buildin_vip_status: Unsupported type %d.\n", type );
			return SCRIPT_CMD_FAILURE;
	}
#else
	script_pushint(st, 0);
#endif
	return SCRIPT_CMD_SUCCESS;
}


/** Adds or removes VIP time in minutes. Need to enable VIP system
 * vip_time <time in mn>,{"<character name>"};
 * @param time: VIP duration in minutes. If time < 0 remove time, else add time.
 * @param name: Character name (optional)
 */
BUILDIN_FUNC(vip_time) {
#ifdef VIP_ENABLE //would be a pain for scripting npc otherwise
	TBL_PC *sd;
	int viptime = script_getnum(st, 2) * 60; // Convert since it's given in minutes.

	if( !script_nick2sd(3,sd) )
		return SCRIPT_CMD_FAILURE;

	chrif_req_login_operation(sd->status.account_id, sd->status.name, CHRIF_OP_LOGIN_VIP, viptime, 7, 0); 
#endif
	return SCRIPT_CMD_SUCCESS;
}


/**
 * Turns a player into a monster and optionally can grant a SC attribute effect.
 * montransform <monster name/ID>, <duration>, <sc type>, <val1>, <val2>, <val3>, <val4>;
 * active_transform <monster name/ID>, <duration>, <sc type>, <val1>, <val2>, <val3>, <val4>;
 * @param monster: Monster ID or name
 * @param duration: Transform duration in millisecond (ms)
 * @param sc_type: Type of SC that will be affected during the transformation
 * @param val1: Value for SC
 * @param val2: Value for SC
 * @param val3: Value for SC
 * @param val4: Value for SC
 * @author: malufett
 */
BUILDIN_FUNC(montransform) {
	TBL_PC *sd;
	enum sc_type type;
	int tick, mob_id, val1, val2, val3, val4;
	struct script_data *data;
	val1 = val2 = val3 = val4 = 0;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_FAILURE;

	data = script_getdata(st, 2);
	get_val(st, data); // Convert into value in case of a variable
	if( data_isstring(data) )
		mob_id = mobdb_searchname(script_getstr(st, 2));
	else
		mob_id = mobdb_checkid(script_getnum(st, 2));

	tick = script_getnum(st, 3);

	if (script_hasdata(st, 4))
		type = (sc_type)script_getnum(st, 4);
	else
		type = SC_NONE;

	if (mob_id == 0) {
		if( data_isstring(data) )
			ShowWarning("buildin_montransform: Attempted to use non-existing monster '%s'.\n", script_getstr(st, 2));
		else
			ShowWarning("buildin_montransform: Attempted to use non-existing monster of ID '%d'.\n", script_getnum(st, 2));
		return SCRIPT_CMD_FAILURE;
	}

	if (mob_id == MOBID_EMPERIUM) {
		ShowWarning("buildin_montransform: Monster 'Emperium' cannot be used.\n");
		return SCRIPT_CMD_FAILURE;
	}

	if (!(type >= SC_NONE && type < SC_MAX)) {
		ShowWarning("buildin_montransform: Unsupported status change id %d\n", type);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 5))
		val1 = script_getnum(st, 5);

	if (script_hasdata(st, 6))
		val2 = script_getnum(st, 6);

	if (script_hasdata(st, 7))
		val3 = script_getnum(st, 7);

	if (script_hasdata(st, 8))
		val4 = script_getnum(st, 8);

	if (tick != 0) {
		if (battle_config.mon_trans_disable_in_gvg && map_flag_gvg2(sd->bl.m)) {
			clif_displaymessage(sd->fd, msg_txt(sd,731)); // Transforming into monster is not allowed in Guild Wars.
			return SCRIPT_CMD_FAILURE;
		}

		if (sd->disguise){
			clif_displaymessage(sd->fd, msg_txt(sd,729)); // Cannot transform into monster while in disguise.
			return SCRIPT_CMD_FAILURE;
		}

		if (!strcmp(script_getfuncname(st), "active_transform")) {
			status_change_end(&sd->bl, SC_ACTIVE_MONSTER_TRANSFORM, INVALID_TIMER); // Clear previous
			sc_start2(NULL, &sd->bl, SC_ACTIVE_MONSTER_TRANSFORM, 100, mob_id, type, tick);
		} else {
			status_change_end(&sd->bl, SC_MONSTER_TRANSFORM, INVALID_TIMER); // Clear previous
			sc_start2(NULL, &sd->bl, SC_MONSTER_TRANSFORM, 100, mob_id, type, tick);
		}
		if (type != SC_NONE)
			sc_start4(NULL, &sd->bl, type, 100, val1, val2, val3, val4, tick);
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Attach script to player for certain duration
 * bonus_script "<script code>",<duration>{,<flag>{,<type>{,<status_icon>{,<char_id>}}}};
 * @param "script code"
 * @param duration
 * @param flag
 * @param icon
 * @param char_id
* @author [Cydh]
 **/
BUILDIN_FUNC(bonus_script) {
	uint16 flag = 0;
	int16 icon = EFST_BLANK;
	uint32 dur;
	uint8 type = 0;
	TBL_PC* sd;
	const char *script_str = NULL;
	struct s_bonus_script_entry *entry = NULL;

	if ( !script_charid2sd(7,sd) )
		return SCRIPT_CMD_FAILURE;
	
	script_str = script_getstr(st,2);
	dur = 1000 * abs(script_getnum(st,3));
	FETCH(4, flag);
	FETCH(5, type);
	FETCH(6, icon);

	// No Script string, No Duration!
	if (script_str[0] == '\0' || !dur) {
		ShowError("buildin_bonus_script: Invalid! Script: \"%s\". Duration: %d\n", script_str, dur);
		return SCRIPT_CMD_FAILURE;
	}

	if (strlen(script_str) >= MAX_BONUS_SCRIPT_LENGTH) {
		ShowError("buildin_bonus_script: Script string to long: \"%s\".\n", script_str);
		return SCRIPT_CMD_FAILURE;
	}

	if (icon <= EFST_BLANK || icon >= EFST_MAX)
		icon = EFST_BLANK;

	if ((entry = pc_bonus_script_add(sd, script_str, dur, (enum efst_types)icon, flag, type))) {
		linkdb_insert(&sd->bonus_script.head, (void *)((intptr_t)entry), entry);
		status_calc_pc(sd,SCO_NONE);
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
* Removes all bonus script from player
* bonus_script_clear {<flag>,{<char_id>}};
* @param flag 0 - Except permanent bonus, 1 - With permanent bonus
* @param char_id Clear script from this character
* @author [Cydh]
*/
BUILDIN_FUNC(bonus_script_clear) {
	TBL_PC* sd;
	bool flag = false;

	if (!script_charid2sd(3,sd))
		return SCRIPT_CMD_FAILURE;

	if (script_hasdata(st,2))
		flag = script_getnum(st,2) != 0;

	pc_bonus_script_clear(sd,(flag ? BSF_PERMANENT : BSF_REM_ALL));
	return SCRIPT_CMD_SUCCESS;
}

/** Allows player to use atcommand while talking with NPC
* enable_command;
* @author [Cydh], [Kichi]
*/
BUILDIN_FUNC(enable_command) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	sd->state.disable_atcommand_on_npc = 0;
	return SCRIPT_CMD_SUCCESS;
}

/** Prevents player to use atcommand while talking with NPC
* disable_command;
* @author [Cydh], [Kichi]
*/
BUILDIN_FUNC(disable_command) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;
	sd->state.disable_atcommand_on_npc = 1;
	return SCRIPT_CMD_SUCCESS;
}

/** Get the information of the members of a guild by type.
 * getguildmember <guild_id>{,<type>};
 * @param guild_id: ID of guild
 * @param type: Type of option (optional)
 */
BUILDIN_FUNC(getguildmember)
{
	struct guild *g = NULL;
	uint8 j = 0;

	g = guild_search(script_getnum(st,2));

	if (g) {
		uint8 i, type = 0;
		struct script_data *data = NULL;
		char *varname = NULL;

		if (script_hasdata(st,3))
 			type = script_getnum(st,3);

		if (script_hasdata(st,4)) {
			data = script_getdata(st, 4);
			if (!data_isreference(data)) {
				ShowError("buildin_getguildmember: Error in argument! Please give a variable to store values in.\n");
				return SCRIPT_CMD_FAILURE;
			}
			varname = reference_getname(data);
			if (type <= 0 && varname[strlen(varname)-1] != '$') {
				ShowError("buildin_getguildmember: The array %s is not string type.\n", varname);
				return SCRIPT_CMD_FAILURE;
			}
		}

		for (i = 0; i < MAX_GUILD; i++) {
			if (g->member[i].account_id) {
				switch (type) {
					case 2:
						if (data)
							setd_sub(st, NULL, varname, j, (void *)__64BPRTSIZE(g->member[i].account_id), data->ref);
						else
							mapreg_setreg(reference_uid(add_str("$@guildmemberaid"), j),g->member[i].account_id);
						break;
					case 1:
						if (data)
							setd_sub(st, NULL, varname, j, (void *)__64BPRTSIZE(g->member[i].char_id), data->ref);
						else
							mapreg_setreg(reference_uid(add_str("$@guildmembercid"), j), g->member[i].char_id);
						break;
					default:
						if (data)
							setd_sub(st, NULL, varname, j, (void *)__64BPRTSIZE(g->member[i].name), data->ref);
						else
							mapreg_setregstr(reference_uid(add_str("$@guildmembername$"), j), g->member[i].name);
						break;
				}
				j++;
			}
		}
	}
	mapreg_setreg(add_str("$@guildmembercount"), j);
	script_pushint(st, j);
	return SCRIPT_CMD_SUCCESS;
}

/** Adds spirit ball to player for 'duration' in milisecond
* addspiritball <count>,<duration>{,<char_id>};
* @param count How many spirit ball will be added
* @param duration How long spiritball is active until it disappears
* @param char_id Target player (Optional)
* @author [Cydh]
*/
BUILDIN_FUNC(addspiritball) {
	uint8 i, count;
	uint16 duration;
	struct map_session_data *sd = NULL;

	if (script_hasdata(st,4)) {
		if (!script_isstring(st,4))
			sd = map_charid2sd(script_getnum(st,4));
		else
			sd = map_nick2sd(script_getstr(st,4),false);
	}
	else
		script_rid2sd(sd);
	if (!sd)
		return SCRIPT_CMD_FAILURE;

	count = script_getnum(st,2);

	if (count == 0)
		return SCRIPT_CMD_SUCCESS;

	duration = script_getnum(st,3);

	for (i = 0; i < count; i++)
		pc_addspiritball(sd,duration,10);
	return SCRIPT_CMD_SUCCESS;
}

/** Deletes the spirit ball(s) from player
* delspiritball <count>{,<char_id>};
* @param count How many spirit ball will be deleted
* @param char_id Target player (Optional)
* @author [Cydh]
*/
BUILDIN_FUNC(delspiritball) {
	uint8 count;
	struct map_session_data *sd = NULL;
	
	if (script_hasdata(st,3)) {
		if (!script_isstring(st,3))
			sd = map_charid2sd(script_getnum(st,3));
		else
			sd = map_nick2sd(script_getstr(st,3),false);
	}
	else
		script_rid2sd(sd);
	if (!sd)
		return SCRIPT_CMD_FAILURE;

	count = script_getnum(st,2);

	if (count == 0)
		count = 1;

	pc_delspiritball(sd,count,0);
	return SCRIPT_CMD_SUCCESS;
}

/** Counts the spirit ball that player has
* countspiritball {<char_id>};
* @param char_id Target player (Optional)
* @author [Cydh]
*/
BUILDIN_FUNC(countspiritball) {
	struct map_session_data *sd;

	if (script_hasdata(st,2)) {
		if (!script_isstring(st,2))
			sd = map_charid2sd(script_getnum(st,2));
		else
			sd = map_nick2sd(script_getstr(st,2),false);
	}
	else
		script_rid2sd(sd);
	if (!sd)
		return SCRIPT_CMD_FAILURE;
	script_pushint(st,sd->spiritball);
	return SCRIPT_CMD_SUCCESS;
}

/** Merges separated stackable items because of guid
* mergeitem {<char_id>};
* @param char_id Char ID (Optional)
* @author [Cydh]
*/
BUILDIN_FUNC(mergeitem) {
	struct map_session_data *sd;

	if (!script_charid2sd(2, sd))
		return SCRIPT_CMD_FAILURE;

	clif_merge_item_open(sd);
	return SCRIPT_CMD_SUCCESS;
}

/** Merges separated stackable items because of guid (Unofficial)
* mergeitem2 {<item_id>,{<char_id>}};
* mergeitem2 {"<item name>",{<char_id>}};
* @param item Item ID/Name for merging specific item (Optional)
* @author [Cydh]
*/
BUILDIN_FUNC(mergeitem2) {
	struct map_session_data *sd;
	struct item *items = NULL;
	uint16 i, count = 0, nameid = 0;

	if (!script_charid2sd(3, sd))
		return SCRIPT_CMD_FAILURE;

	if (script_hasdata(st, 2)) {
		struct script_data *data = script_getdata(st, 2);
		get_val(st, data);

		if (data_isstring(data)) {// "<item name>"
			const char *name = conv_str(st,data);
			struct item_data *id;
			if (!(id = itemdb_searchname(name))) {
				ShowError("buildin_mergeitem2: Nonexistant item %s requested.\n", name);
				script_pushint(st, count);
				return SCRIPT_CMD_FAILURE;
			}
			nameid = id->nameid;
		}
		else if (data_isint(data)) {// <item id>
			nameid = conv_num(st,data);
			if (!itemdb_exists(nameid)) {
				ShowError("buildin_mergeitem: Nonexistant item %d requested.\n", nameid);
				script_pushint(st, count);
				return SCRIPT_CMD_FAILURE;
			}
		}
	}

	for (i = 0; i < MAX_INVENTORY; i++) {
		struct item *it = &sd->inventory.u.items_inventory[i];

		if (!it || !it->unique_id || it->expire_time || !itemdb_isstackable(it->nameid))
			continue;
		if ((!nameid || (nameid == it->nameid))) {
			uint8 k;
			if (!count) {
				CREATE(items, struct item, 1);
				memcpy(&items[count++], it, sizeof(struct item));
				pc_delitem(sd, i, it->amount, 0, 0, LOG_TYPE_NPC);
				continue;
			}
			for (k = 0; k < count; k++) {
				// Find Match
				if (&items[k] && items[k].nameid == it->nameid && items[k].bound == it->bound && memcmp(items[k].card, it->card, sizeof(it->card)) == 0) {
					items[k].amount += it->amount;
					pc_delitem(sd, i, it->amount, 0, 0, LOG_TYPE_NPC);
					break;
				}
			}
			if (k >= count) {
				// New entry
				RECREATE(items, struct item, count+1);
				memcpy(&items[count++], it, sizeof(struct item));
				pc_delitem(sd, i, it->amount, 0, 0, LOG_TYPE_NPC);
			}
		}
	}

	if (!items) // Nothing todo here
		return SCRIPT_CMD_SUCCESS;

	// Retrieve the items
	for (i = 0; i < count; i++) {
		uint8 flag = 0;
		if (!&items[i])
			continue;
		items[i].id = 0;
		items[i].unique_id = 0;
		if ((flag = pc_additem(sd, &items[i], items[i].amount, LOG_TYPE_NPC)))
			clif_additem(sd, i, items[i].amount, flag);
	}
	aFree(items);
	script_pushint(st, count);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Update an entry from NPC shop.
 * npcshopupdate "<name>",<item_id>,<price>{,<stock>}
 **/
BUILDIN_FUNC(npcshopupdate) {
	const char* npcname = script_getstr(st, 2);
	struct npc_data* nd = npc_name2id(npcname);
	uint16 nameid = script_getnum(st, 3);
	int price = script_getnum(st, 4);
#if PACKETVER >= 20131223
	uint16 stock = script_hasdata(st,5) ? script_getnum(st,5) : 0;
#endif
	int i;

	if( !nd || ( nd->subtype != NPCTYPE_SHOP && nd->subtype != NPCTYPE_CASHSHOP && nd->subtype != NPCTYPE_ITEMSHOP && nd->subtype != NPCTYPE_POINTSHOP && nd->subtype != NPCTYPE_MARKETSHOP ) ) { // Not found.
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}
	
	if (!nd->u.shop.count) {
		ShowError("buildin_npcshopupdate: Attempt to update empty shop from '%s'.\n", nd->exname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	for (i = 0; i < nd->u.shop.count; i++) {
		if (nd->u.shop.shop_item[i].nameid == nameid) {

			if (price != 0)
				nd->u.shop.shop_item[i].value = price;
#if PACKETVER >= 20131223
			if (nd->subtype == NPCTYPE_MARKETSHOP) {
				nd->u.shop.shop_item[i].qty = stock;
				npc_market_tosql(nd->exname, &nd->u.shop.shop_item[i]);
			}
#endif
		}
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

// Clan System
BUILDIN_FUNC(clan_join){
	struct map_session_data *sd;
	int clan_id = script_getnum(st,2);

	if( !script_charid2sd( 3, sd ) ){
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if( clan_member_join( sd, clan_id, sd->status.account_id, sd->status.char_id ) )
		script_pushint(st, true);
	else
		script_pushint(st, false);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(clan_leave){
	struct map_session_data *sd;

	if( !script_charid2sd( 2, sd ) ){
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if( clan_member_leave( sd, sd->status.clan_id, sd->status.account_id, sd->status.char_id ) )
		script_pushint(st, true);
	else
		script_pushint(st, false);

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Get rid from running script.
 * getattachedrid();
 **/
BUILDIN_FUNC(getattachedrid) {
	script_pushint(st, st->rid);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Get variable from a Player
 * getvar <variable>,<char_id>;
 */
BUILDIN_FUNC(getvar) {
	int char_id = script_getnum(st, 3);
	struct map_session_data *sd = map_charid2sd(char_id);
	struct script_data *data = NULL;
	const char *name = NULL;

	if (!sd) {
		ShowError("buildin_getvar: No player found with char id '%d'.\n", char_id);
		return SCRIPT_CMD_FAILURE;
	}

	data = script_getdata(st, 2);
	if (!data_isreference(data)) {
		ShowError("buildin_getvar: Not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	name = reference_getname(data);

	if (reference_toparam(data)) {
		ShowError("buildin_getvar: '%s' is a parameter - please use readparam instead\n", name);
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	if (name[0] == '.' || name[0] == '$' || name[0] == '\'') { // Not a PC variable
		ShowError("buildin_getvar: Invalid scope (not PC variable)\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	get_val_(st, data, sd);
	if (data_isint(data))
		script_pushint(st, conv_num_(st, data, sd));
	else
		script_pushstrcopy(st, conv_str_(st, data, sd));

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Display script message
 * showscript "<message>"{,<GID>,<flag>};
 * @param flag: Specify target
 *   AREA - Message is sent to players in the vicinity of the source (default).
 *   SELF - Message is sent only to player attached.
 **/
BUILDIN_FUNC(showscript) {
	struct block_list *bl = NULL;
	const char *msg = script_getstr(st,2);
	int id = 0;
	send_target target = AREA;

	if (script_hasdata(st,3)) {
		id = script_getnum(st,3);
		bl = map_id2bl(id);
	}
	else {
		bl = st->rid ? map_id2bl(st->rid) : map_id2bl(st->oid);
	}

	if (!bl) {
		ShowError("buildin_showscript: Script not attached. (id=%d, rid=%d, oid=%d)\n", id, st->rid, st->oid);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 4)) {
		target = static_cast<send_target>(script_getnum(st, 4));
		if (target == SELF && map_id2sd(bl->id) == NULL) {
			ShowWarning("script: showscript: self can't be used for non-players objects.\n");
			return SCRIPT_CMD_FAILURE;
		}
	}

	clif_showscript(bl, msg, target);

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Ignore the SECURE_NPCTIMEOUT function.
 * ignoretimeout <flag>{,<char_id>};
 */
BUILDIN_FUNC(ignoretimeout)
{
#ifdef SECURE_NPCTIMEOUT
	struct map_session_data *sd = NULL;

	if (script_hasdata(st,3)) {
		if (!script_isstring(st,3))
			sd = map_charid2sd(script_getnum(st,3));
		else
			sd = map_nick2sd(script_getstr(st,3),false);
	} else
		script_rid2sd(sd);

	if (!sd)
		return SCRIPT_CMD_FAILURE;

	sd->state.ignoretimeout = script_getnum(st,2) > 0;
#endif
	return SCRIPT_CMD_SUCCESS;
}

/**
 * geteleminfo <type>{,<char_id>};
 **/
BUILDIN_FUNC(geteleminfo) {
	TBL_ELEM *ed = NULL;
	TBL_PC *sd = NULL;
	int type = script_getnum(st,2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (!(ed = sd->ed)) {
		//ShowDebug("buildin_geteleminfo: Player doesn't have Elemental.\n");
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch (type) {
		case 0: script_pushint(st, ed->elemental.elemental_id); break;
		case 1: script_pushint(st, ed->bl.id); break;
		default:
			ShowError("buildin_geteleminfo: Invalid type '%d'.\n", type);
			script_pushint(st, 0);
			return SCRIPT_CMD_FAILURE;
	}

	return SCRIPT_CMD_SUCCESS;
}


/**
 * Set the quest info of quest_id only showed on player in level range.
 * setquestinfo_level <quest_id>,<min_level>,<max_level>
 * @author [Cydh]
 **/
BUILDIN_FUNC(setquestinfo_level) {
	TBL_NPC* nd = map_id2nd(st->oid);
	int quest_id = script_getnum(st, 2);
	struct questinfo *qi = map_has_questinfo(nd->bl.m, nd, quest_id);

	if (!qi) {
		ShowError("buildin_setquestinfo_level: Quest with ID '%d' is not defined yet.\n", quest_id);
		return SCRIPT_CMD_FAILURE;
	}

	qi->min_level = script_getnum(st, 3);
	qi->max_level = script_getnum(st, 4);
	if (!qi->max_level)
		qi->max_level = MAX_LEVEL;

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set the quest info of quest_id only showed for player that has quest criteria
 * setquestinfo_req <quest_id>,<quest_req_id>,<state>{,<quest_req_id>,<state>,...};
 * @author [Cydh]
 **/
BUILDIN_FUNC(setquestinfo_req) {
	TBL_NPC* nd = map_id2nd(st->oid);
	int quest_id = script_getnum(st, 2);
	struct questinfo *qi = map_has_questinfo(nd->bl.m, nd, quest_id);
	uint8 i = 0;
	uint8 num = script_lastdata(st);

	if (!qi) {
		ShowError("buildin_setquestinfo_req: Quest with ID '%d' is not defined yet.\n", quest_id);
		return SCRIPT_CMD_FAILURE;
	}

	if (quest_search(quest_id) == &quest_dummy) {
		ShowError("buildin_setquestinfo_req: Quest with ID '%d' is not found in Quest DB.\n", quest_id);
		return SCRIPT_CMD_FAILURE;
	}

	if (num%2) {
		ShowError("buildin_setquestinfo_req: Odd number of parameters(%d) - pairs of requirements are expected.\n", num-2);
		return SCRIPT_CMD_FAILURE;
	}

	for (i = 3; i <= num; i += 2) {
		RECREATE(qi->req, struct questinfo_req, qi->req_count+1);
		qi->req[qi->req_count].quest_id = script_getnum(st, i);
		qi->req[qi->req_count].state = (script_getnum(st, i+1) >= 2) ? 2 : (script_getnum(st, i+1) <= 0) ? 0 : 1;
		qi->req_count++;
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set the quest info of quest_id only showed for player that has specified Job
 * setquestinfo_job <quest_id>,<job>{,<job>...};
 * @author [Cydh]
 **/
BUILDIN_FUNC(setquestinfo_job) {
	TBL_NPC* nd = map_id2nd(st->oid);
	int quest_id = script_getnum(st, 2);
	struct questinfo *qi = map_has_questinfo(nd->bl.m, nd, quest_id);
	int job_id = 0;
	uint8 i = 0;
	uint8 num = script_lastdata(st)+1;

	if (!qi) {
		ShowError("buildin_setquestinfo_job: Quest with ID '%d' is not defined yet.\n", quest_id);
		return SCRIPT_CMD_FAILURE;
	}

	for (i = 3; i < num; i++) {
		job_id = script_getnum(st, i);
		if (!pcdb_checkid(job_id)) {
			ShowError("buildin_setquestinfo_job: Invalid job id '%d' in Quest with ID %d.\n", job_id, quest_id);
			continue;
		}
		buildin_questinfo_setjob(qi, job_id);
	}

	return SCRIPT_CMD_SUCCESS;
}

/**
 * opendressroom(<flag>{,<char_id>});
 */
BUILDIN_FUNC(opendressroom)
{
#if PACKETVER >= 20150513
	int flag = 1;
    TBL_PC* sd;

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);

    if (!script_charid2sd(3, sd))
        return SCRIPT_CMD_FAILURE;

    clif_dressing_room(sd, flag);

    return SCRIPT_CMD_SUCCESS;
#else
    return SCRIPT_CMD_FAILURE;
#endif
}

/**
 * navigateto("<map>"{,<x>,<y>,<flag>,<hide_window>,<monster_id>,<char_id>});
 */
BUILDIN_FUNC(navigateto){
#if PACKETVER >= 20111010
	TBL_PC* sd;
	const char *mapname;
	uint16 x = 0, y = 0, monster_id = 0;
	uint8 flag = NAV_KAFRA_AND_AIRSHIP;
	bool hideWindow = true;

	mapname = script_getstr(st,2);

	if( script_hasdata(st,3) )
		x = script_getnum(st,3);
	if( script_hasdata(st,4) )
		y = script_getnum(st,4);
	if( script_hasdata(st,5) )
		flag = (uint8)script_getnum(st,5);
	if( script_hasdata(st,6) )
		hideWindow = script_getnum(st,6) ? true : false;
	if( script_hasdata(st,7) )
		monster_id = script_getnum(st,7);

	if (!script_charid2sd(8, sd))
        return SCRIPT_CMD_FAILURE;

	clif_navigateTo(sd,mapname,x,y,flag,hideWindow,monster_id);

	return SCRIPT_CMD_SUCCESS;
#else
	return SCRIPT_CMD_FAILURE;
#endif
}

/**
 * adopt("<parent_name>","<baby_name>");
 * adopt(<parent_id>,<baby_id>);
 * https://rathena.org/board/topic/104014-suggestion-add-adopt-or-etc/
 */
BUILDIN_FUNC(adopt)
{
	TBL_PC *sd, *b_sd;
	struct script_data *data;
	enum adopt_responses response;

	data = script_getdata(st, 2);
	get_val(st, data);

	if (data_isstring(data)) {
		const char *name = conv_str(st, data);

		sd = map_nick2sd(name,false);
		if (sd == NULL) {
			ShowError("buildin_adopt: Non-existant parent character %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
	} else if (data_isint(data)) {
		uint32 char_id = conv_num(st, data);

		sd = map_charid2sd(char_id);
		if (sd == NULL) {
			ShowError("buildin_adopt: Non-existant parent character %d requested.\n", char_id);
			return SCRIPT_CMD_FAILURE;
		}
	} else {
		ShowError("buildin_adopt: Invalid data type for argument #1 (%d).", data->type);
		return SCRIPT_CMD_FAILURE;
	}

	data = script_getdata(st, 3);
	get_val(st, data);

	if (data_isstring(data)) {
		const char *name = conv_str(st, data);

		b_sd = map_nick2sd(name,false);
		if (b_sd == NULL) {
			ShowError("buildin_adopt: Non-existant baby character %s requested.\n", name);
			return SCRIPT_CMD_FAILURE;
		}
	} else if (data_isint(data)) {
		uint32 char_id = conv_num(st, data);

		b_sd = map_charid2sd(char_id);
		if (b_sd == NULL) {
			ShowError("buildin_adopt: Non-existant baby character %d requested.\n", char_id);
			return SCRIPT_CMD_FAILURE;
		}
	} else {
		ShowError("buildin_adopt: Invalid data type for argument #2 (%d).", data->type);
		return SCRIPT_CMD_FAILURE;
	}

	response = pc_try_adopt(sd, map_charid2sd(sd->status.partner_id), b_sd);

	if (response == ADOPT_ALLOWED) {
		TBL_PC *p_sd = map_charid2sd(sd->status.partner_id);

		b_sd->adopt_invite = sd->status.account_id;
		clif_Adopt_request(b_sd, sd, p_sd->status.account_id);
		script_pushint(st, ADOPT_ALLOWED);
		return SCRIPT_CMD_SUCCESS;
	}

	script_pushint(st, response);
	return SCRIPT_CMD_FAILURE;
}

/**
 * Returns the minimum or maximum of all the given parameters for integer variables.
 *
 * min( <value or array>{,value or array 2,...} );
 * minimum( <value or array>{,value or array 2,...} );
 * max( <value or array>{,value or array 2,...} );
 * maximum( <value or array>{,value or array 2,...} );
*/
BUILDIN_FUNC(minmax){
	char *functionname;
	unsigned int i;
	int value;
	// Function pointer for our comparison function (either min or max at the moment)
	int32 (*func)(int32, int32);
	
	// Get the real function name
	functionname = script_getfuncname(st);
	
	// Our data should start at offset 2
	i = 2;

	if( !script_hasdata( st, i ) ){
		ShowError( "buildin_%s: no arguments given!\n", functionname );
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	if( strnicmp( functionname, "min", strlen( "min" ) ) == 0 ){
		value = INT_MAX;
		func = i32min;
	}else if( strnicmp( functionname, "max", strlen( "max" ) ) == 0 ){
		value = INT_MIN;
		func = i32max;
	}else{
		ShowError( "buildin_%s: Unknown call case for min/max!\n", functionname );
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	// As long as we have data on our script stack
	while( script_hasdata(st,i) ){
		struct script_data *data;
		
		// Get the next piece of data from the script stack
		data = script_getdata( st, i );

		// Is the current parameter a single integer?
		if( data_isint( data ) ){
			value = func( value, script_getnum( st, i ) );
		// Is the current parameter an array variable?
		}else if( data_isreference( data ) ){
			const char *name;
			struct map_session_data* sd;
			unsigned int start, end;

			// Get the name of the variable
			name = reference_getname(data);

			// Check if it's a string variable
			if( is_string_variable( name ) ){
				ShowError( "buildin_%s: illegal type, need integer!\n", functionname );
				script_reportdata( data );
				st->state = END;
				return SCRIPT_CMD_FAILURE;
			}

			// Get the session data, if a player is attached
			sd = st->rid ? map_id2sd(st->rid) : NULL;

			// Try to find the array's source pointer
			if( !script_array_src( st, sd, name, reference_getref( data ) ) ){
				ShowError( "buildin_%s: not a array!\n", functionname );
				script_reportdata( data );
				st->state = END;
				return SCRIPT_CMD_FAILURE;
			}

			// Get the start and end indices of the array
			start = reference_getindex( data );
			end = script_array_highest_key( st, sd, name, reference_getref( data ) );

			// Skip empty arrays
			if( start < end ){
				int id;
				
				// For getting the values we need the id of the array
				id = reference_getid( data );

				// Loop through each value stored in the array
				for( ; start < end; start++ ){
					value = func( value, (int32)__64BPRTSIZE( get_val2( st, reference_uid( id, start ), reference_getref( data ) ) ) );

					script_removetop( st, -1, 0 );
				}
			}
		}else{
			ShowError( "buildin_%s: not a supported data type!\n", functionname );
			script_reportdata( data );
			st->state = END;
			return SCRIPT_CMD_FAILURE;
		}

		// Continue with the next stack entry
		i++;
	}

	script_pushint( st, value );

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Safety Base/Job EXP addition than using `set BaseExp,n;` or `set JobExp,n;`
 * Unlike `getexp` that affected by some adjustments.
 * getexp2 <base_exp>,<job_exp>{,<char_id>};
 * @author [Cydh]
 **/
BUILDIN_FUNC(getexp2) {
	TBL_PC *sd = NULL;
	int base_exp = script_getnum(st, 2);
	int job_exp = script_getnum(st, 3);

	if (!script_charid2sd(4, sd))
		return SCRIPT_CMD_FAILURE;

	if (base_exp == 0 && job_exp == 0)
		return SCRIPT_CMD_SUCCESS;

	if (base_exp > 0)
		pc_gainexp(sd, NULL, base_exp, 0, 2);
	else if (base_exp < 0)
		pc_lostexp(sd, base_exp * -1, 0);

	if (job_exp > 0)
		pc_gainexp(sd, NULL, 0, job_exp, 2);
	else if (job_exp < 0)
		pc_lostexp(sd, 0, job_exp * -1);
	return SCRIPT_CMD_SUCCESS;
}

/**
* Force stat recalculation of sd
* recalculatestat;
* @author [secretdataz]
**/
BUILDIN_FUNC(recalculatestat) {
	TBL_PC* sd;

	if (!script_rid2sd(sd))
		return SCRIPT_CMD_FAILURE;

	status_calc_pc(sd, SCO_FORCE);
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(hateffect){
#if PACKETVER >= 20150513
	struct map_session_data* sd;
	bool enable;
	int i, effectID;

	if( !script_rid2sd(sd) )
		return SCRIPT_CMD_FAILURE;

	effectID = script_getnum(st,2);
	enable = script_getnum(st,3) ? true : false;

	if( effectID <= HAT_EF_MIN || effectID >= HAT_EF_MAX ){
		ShowError( "buildin_hateffect: unsupported hat effect id %d\n", effectID );
		return SCRIPT_CMD_FAILURE;
	}

	ARR_FIND( 0, sd->hatEffectCount, i, sd->hatEffectIDs[i] == effectID );

	if( enable ){
		if( i < sd->hatEffectCount ){
			return SCRIPT_CMD_SUCCESS;
		}

		RECREATE(sd->hatEffectIDs,uint32,sd->hatEffectCount+1);
		sd->hatEffectIDs[sd->hatEffectCount] = effectID;
		sd->hatEffectCount++;
	}else{
		if( i == sd->hatEffectCount ){
			return SCRIPT_CMD_SUCCESS;
		}

		for( ; i < sd->hatEffectCount - 1; i++ ){
			sd->hatEffectIDs[i] = sd->hatEffectIDs[i+1];
		}

		sd->hatEffectCount--;

		if( !sd->hatEffectCount ){
			aFree(sd->hatEffectIDs);
			sd->hatEffectIDs = NULL;
		}
	}

	if( !sd->state.connect_new ){
		clif_hat_effect_single( sd, effectID, enable );
	}

#endif
	return SCRIPT_CMD_SUCCESS;
}

/**
* Retrieves param of current random option. Intended for random option script only.
* getrandomoptinfo(<type>);
* @author [secretdataz]
**/
BUILDIN_FUNC(getrandomoptinfo) {
	struct map_session_data *sd;
	int val;
	if (script_rid2sd(sd) && current_equip_item_index != -1 && current_equip_opt_index != -1 && sd->inventory.u.items_inventory[current_equip_item_index].option[current_equip_opt_index].id) {
		int param = script_getnum(st, 2);

		switch (param) {
			case ROA_ID:
				val = sd->inventory.u.items_inventory[current_equip_item_index].option[current_equip_opt_index].id;
				break;
			case ROA_VALUE:
				val = sd->inventory.u.items_inventory[current_equip_item_index].option[current_equip_opt_index].value;
				break;
			case ROA_PARAM:
				val = sd->inventory.u.items_inventory[current_equip_item_index].option[current_equip_opt_index].param;
				break;
			default:
				ShowWarning("buildin_getrandomoptinfo: Invalid attribute type %d (Max %d).\n", param, MAX_ITEM_RDM_OPT);
				val = 0;
				break;
		}
		script_pushint(st, val);
	}
	else {
		script_pushint(st, 0);
	}
	return SCRIPT_CMD_SUCCESS;
}

/**
* Retrieves a random option on an equipped item.
* getequiprandomoption(<equipment slot>,<index>,<type>{,<char id>});
* @author [secretdataz]
*/
BUILDIN_FUNC(getequiprandomoption) {
	struct map_session_data *sd;
	int val;
	short i = -1;
	int pos = script_getnum(st, 2);
	int index = script_getnum(st, 3);
	int type = script_getnum(st, 4);
	if (!script_charid2sd(5, sd)) {
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}
	if (index < 0 || index >= MAX_ITEM_RDM_OPT) {
		ShowError("buildin_getequiprandomoption: Invalid random option index %d.\n", index);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}
	if (equip_index_check(pos))
		i = pc_checkequip(sd, equip_bitmask[pos]);
	if (i < 0) {
		ShowError("buildin_getequiprandomoption: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	switch (type) {
		case ROA_ID:
			val = sd->inventory.u.items_inventory[i].option[index].id;
			break;
		case ROA_VALUE:
			val = sd->inventory.u.items_inventory[i].option[index].value;
			break;
		case ROA_PARAM:
			val = sd->inventory.u.items_inventory[i].option[index].param;
			break;
		default:
			ShowWarning("buildin_getequiprandomoption: Invalid attribute type %d (Max %d).\n", type, MAX_ITEM_RDM_OPT);
			val = 0;
			break;
	}
	script_pushint(st, val);
	return SCRIPT_CMD_SUCCESS;
}

/**
* Adds a random option on a random option slot on an equipped item and overwrites
* existing random option in the process.
* setrandomoption(<equipment slot>,<index>,<id>,<value>,<param>{,<char id>});
* @author [secretdataz]
*/
BUILDIN_FUNC(setrandomoption) {
	struct map_session_data *sd;
	struct s_random_opt_data *opt;
	int pos, index, id, value, param, ep;
	int i = -1;
	if (!script_charid2sd(7, sd))
		return SCRIPT_CMD_FAILURE;
	pos = script_getnum(st, 2);
	index = script_getnum(st, 3);
	id = script_getnum(st, 4);
	value = script_getnum(st, 5);
	param = script_getnum(st, 6);

	if ((opt = itemdb_randomopt_exists((short)id)) == NULL) {
		ShowError("buildin_setrandomoption: Random option ID %d does not exists.\n", id);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	if (index < 0 || index >= MAX_ITEM_RDM_OPT) {
		ShowError("buildin_setrandomoption: Invalid random option index %d.\n", index);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	if (equip_index_check(pos))
		i = pc_checkequip(sd, equip_bitmask[pos]);
	if (i >= 0) {
		ep = sd->inventory.u.items_inventory[i].equip;
		log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->inventory.u.items_inventory[i]);
		pc_unequipitem(sd, i, 2);
		sd->inventory.u.items_inventory[i].option[index].id = id;
		sd->inventory.u.items_inventory[i].option[index].value = value;
		sd->inventory.u.items_inventory[i].option[index].param = param;
		clif_delitem(sd, i, 1, 3);
		log_pick_pc(sd, LOG_TYPE_SCRIPT, -1, &sd->inventory.u.items_inventory[i]);
		clif_additem(sd, i, 1, 0);
		pc_equipitem(sd, i, ep);
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	ShowError("buildin_setrandomoption: No item equipped at pos %d (CID=%d/AID=%d).\n", pos, sd->status.char_id, sd->status.account_id);
	script_pushint(st, 0);
	return SCRIPT_CMD_FAILURE;
}

/// Returns the number of stat points needed to change the specified stat by val.
/// If val is negative, returns the number of stat points that would be needed to
/// raise the specified stat from (current value - val) to current value.
/// *needed_status_point(<type>,<val>{,<char id>});
/// @author [secretdataz]
BUILDIN_FUNC(needed_status_point) {
	struct map_session_data *sd;
	int type, val;
	if (!script_charid2sd(4, sd))
		return SCRIPT_CMD_FAILURE;
	type = script_getnum(st, 2);
	val = script_getnum(st, 3);

	script_pushint(st, pc_need_status_point(sd, type, val));
	return SCRIPT_CMD_SUCCESS;
}

/**
 * jobcanentermap("<mapname>"{,<JobID>});
 * Check if (player with) JobID can enter the map.
 * @param mapname Map name
 * @param JobID Player's JobID (optional)
 **/
BUILDIN_FUNC(jobcanentermap) {
	const char *mapname = script_getstr(st, 2);
	int mapidx = mapindex_name2id(mapname), m = -1;
	int jobid = 0;
	TBL_PC *sd = NULL;

	if (!mapidx) {// Invalid map
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}
	m = map_mapindex2mapid(mapidx);
	if (m == -1) { // Map is on different map server
		ShowError("buildin_jobcanentermap: Map '%s' is not found in this server.\n", mapname);
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (script_hasdata(st, 3)) {
		jobid = script_getnum(st, 3);
	} else {
		if (!script_rid2sd(sd)) {
			script_pushint(st, false);
			return SCRIPT_CMD_FAILURE;
		}
		jobid = sd->status.class_;
	}

	script_pushint(st, pc_job_can_entermap((enum e_job)jobid, m, sd ? sd->group_level : 0));
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Return alliance information between the two guilds.
 * getguildalliance(<guild id1>,<guild id2>);
 * Return values:
 *	-2 - Guild ID1 does not exist
 *	-1 - Guild ID2 does not exist
 *	 0 - Both guilds have no relation OR guild ID aren't given
 *	 1 - Both guilds are allies
 *	 2 - Both guilds are antagonists
 */
BUILDIN_FUNC(getguildalliance)
{
	struct guild *guild_data1, *guild_data2;
	int guild_id1, guild_id2, i = 0;

	guild_id1 = script_getnum(st,2);
	guild_id2 = script_getnum(st,3);

	if (guild_id1 < 1 || guild_id2 < 1) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (guild_id1 == guild_id2) {
		script_pushint(st, 1);
		return SCRIPT_CMD_SUCCESS;
	}

	guild_data1 = guild_search(guild_id1);
	guild_data2 = guild_search(guild_id2);

	if (guild_data1 == NULL) {
		ShowWarning("buildin_getguildalliance: Requesting non-existent GuildID1 '%d'.\n", guild_id1);
		script_pushint(st, -2);
		return SCRIPT_CMD_FAILURE;
	}
	if (guild_data2 == NULL) {
		ShowWarning("buildin_getguildalliance: Requesting non-existent GuildID2 '%d'.\n", guild_id2);
		script_pushint(st, -1);
		return SCRIPT_CMD_FAILURE;
	}

	ARR_FIND(0, MAX_GUILDALLIANCE, i, guild_data1->alliance[i].guild_id == guild_id2);
	if (i == MAX_GUILDALLIANCE) {
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (guild_data1->alliance[i].opposition)
		script_pushint(st, 2);
	else
		script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/*
 * openstorage2 <storage_id>,<mode>{,<account_id>}
 * mode @see enum e_storage_mode
 **/
BUILDIN_FUNC(openstorage2) {
	int stor_id = script_getnum(st, 2);
	TBL_PC *sd = NULL;

	if (!script_accid2sd(4, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	if (!storage_exists(stor_id)) {
		ShowError("buildin_openstorage2: Invalid storage_id '%d'!\n", stor_id);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st, storage_premiumStorage_load(sd, stor_id, script_getnum(st, 3)));
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Create a new channel
 * channel_create "<chname>","<alias>"{,"<password>"{<option>{,<delay>{,<color>{,<char_id>}}}}};
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_create) {
	struct Channel tmp_chan, *ch = NULL;
	const char *chname = script_getstr(st,2), *pass = NULL;
	int i = channel_chk((char*)chname, NULL, 3);
	TBL_PC *sd = NULL;

	if (i != 0) {
		ShowError("buildin_channel_create: Channel name '%s' is invalid. Errno %d\n", chname, i);
		script_pushint(st,i);
		return SCRIPT_CMD_FAILURE;
	}

	memset(&tmp_chan, 0, sizeof(struct Channel));

	if (script_hasdata(st,8)) {
		tmp_chan.char_id = script_getnum(st,8);
		if (!(sd = map_charid2sd(tmp_chan.char_id))) {
			ShowError("buildin_channel_create: Player with char id '%d' is not found.\n", tmp_chan.char_id);
			script_pushint(st,-5);
			return SCRIPT_CMD_FAILURE;
		}
		tmp_chan.type = CHAN_TYPE_PRIVATE;
		i = 1;
	}
	else {
		tmp_chan.type = CHAN_TYPE_PUBLIC;
		i = 0;
	}

	safestrncpy(tmp_chan.name, chname+1, sizeof(tmp_chan.name));
	safestrncpy(tmp_chan.alias, script_getstr(st,3), sizeof(tmp_chan.alias));
	if (script_hasdata(st,4) && (pass = script_getstr(st,4)) && strcmpi(pass,"null") != 0)
		safestrncpy(tmp_chan.pass, pass, sizeof(tmp_chan.pass));
	if (script_hasdata(st,5))
		tmp_chan.opt = script_getnum(st,5);
	else
		tmp_chan.opt = i ? channel_config.private_channel.opt : CHAN_OPT_BASE;
	if (script_hasdata(st,6))
		tmp_chan.msg_delay = script_getnum(st,6);
	else
		tmp_chan.msg_delay = i ? channel_config.private_channel.delay : 1000;
	if (script_hasdata(st,7))
		tmp_chan.color = script_getnum(st,7);
	else
		tmp_chan.color = i ? channel_config.private_channel.color : channel_getColor("Default");

	if (!(ch = channel_create(&tmp_chan))) {
		ShowError("buildin_channel_create: Cannot create channel '%s'.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}
	if (tmp_chan.char_id)
		channel_join(ch, sd);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set channel option
 * channel_setopt "<chname>",<option>,<value>;
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_setopt) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2);
	int opt = script_getnum(st,3), value = script_getnum(st,4);

	if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("buildin_channel_setopt: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	switch (opt) {
		case CHAN_OPT_ANNOUNCE_SELF:
		case CHAN_OPT_ANNOUNCE_JOIN:
		case CHAN_OPT_ANNOUNCE_LEAVE:
		case CHAN_OPT_COLOR_OVERRIDE:
		case CHAN_OPT_CAN_CHAT:
		case CHAN_OPT_CAN_LEAVE:
		case CHAN_OPT_AUTOJOIN:
			if (value)
				ch->opt |= opt;
			else
				ch->opt &= ~opt;
			break;
		case CHAN_OPT_MSG_DELAY:
			ch->msg_delay = value;
			break;
		default:
			ShowError("buildin_channel_setopt: Invalid option %d!\n", opt);
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set channel color
 * channel_setcolor "<chname>",<color>;
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_setcolor) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2);
	int color = script_getnum(st,3);

	if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("buildin_channel_setcolor: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	ch->color = (color & 0x0000FF) << 16 | (color & 0x00FF00) | (color & 0xFF0000) >> 16;//RGB to BGR

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set channel password
 * channel_setpass "<chname>","<password>";
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_setpass) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2), *passwd = script_getstr(st,3);

	if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("buildin_channel_setpass: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (!passwd || !strcmpi(passwd,"null"))
		memset(ch->pass, '\0', sizeof(ch->pass));
	else
		safestrncpy(ch->pass, passwd, sizeof(ch->pass));
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Set authorized groups
 * channel_setgroup "<chname>",<group_id>{,...,<group_id>};
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_setgroup) {
	struct Channel *ch = NULL;
	const char *funcname = script_getfuncname(st), *chname = script_getstr(st,2);
	int i, n = 0, group = 0;

	if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("buildin_channel_setgroup: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (funcname[strlen(funcname)-1] == '2') {
		struct script_data *data = script_getdata(st,3);
		const char *varname = reference_getname(data);
		int32 id, idx;

		if (varname[strlen(varname)-1] == '$') {
			ShowError("buildin_channel_setgroup: The array %s is not numeric type.\n", varname);
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}

		n = script_array_highest_key(st, NULL, reference_getname(data), reference_getref(data));
		if (n < 1) {
			ShowError("buildin_channel_setgroup: No group id listed.\n");
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}

		if (ch->groups)
			aFree(ch->groups);
		ch->groups = NULL;
		ch->group_count = 0;

		id = reference_getid(data);
		idx = reference_getindex(data);
		for (i = 0; i < n; i++) {
			group = (int32)__64BPRTSIZE(get_val2(st,reference_uid(id,idx+i),reference_getref(data)));
			if (group == 0) {
				script_pushint(st,1);
				return SCRIPT_CMD_SUCCESS;
			}
			RECREATE(ch->groups, unsigned short, ++ch->group_count);
			ch->groups[ch->group_count-1] = group;
			ShowInfo("buildin_channel_setgroup: (2) Added group %d. Num: %d\n", ch->groups[ch->group_count-1], ch->group_count);
		}
	}
	else {
		group = script_getnum(st,3);
		n = script_lastdata(st)-1;

		if (n < 1) {
			ShowError("buildin_channel_setgroup: Please input at least 1 group_id.\n");
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}

		if (ch->groups)
			aFree(ch->groups);
		ch->groups = NULL;
		ch->group_count = 0;

		if (group == 0) { // Removed group list
			script_pushint(st,1);
			return SCRIPT_CMD_SUCCESS;
		}

		CREATE(ch->groups, unsigned short, n);
		for (i = 3; i < n+2; i++) {
			ch->groups[ch->group_count++] = script_getnum(st,i);
			ShowInfo("buildin_channel_setgroup: (1) Added group %d. Num: %d\n", ch->groups[ch->group_count-1], ch->group_count);
		}
	}
	script_pushint(st,n);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Send message on channel
 * channel_chat "<chname>","<message>"{,<color>};
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_chat) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2), *msg = script_getstr(st,3);
	char output[CHAT_SIZE_MAX+1];
	unsigned long color = 0;

	// Check also local channels
	if (chname[0] != '#') { // By Map
		int m = mapindex_name2id(chname);
		if (!m || (m = map_mapindex2mapid(m)) < 0) {
			ShowError("buildin_channel_chat: Invalid map '%s'.\n", chname);
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}
		if (!(ch = map_getmapdata(m)->channel)) {
			ShowDebug("buildin_channel_chat: Map '%s' doesn't have local channel yet.\n", chname);
			script_pushint(st,0);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	else if (strcmpi(chname+1,channel_config.map_tmpl.name) == 0) {
		TBL_NPC *nd = map_id2nd(st->oid);
		if (!nd || nd->bl.m == -1) {
			ShowError("buildin_channel_chat: Floating NPC needs map name instead '%s'.\n", chname);
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}
		if (!(ch = map_getmapdata(nd->bl.m)->channel)) {
			ShowDebug("buildin_channel_chat: Map '%s' doesn't have local channel yet.\n", chname);
			script_pushint(st,0);
			return SCRIPT_CMD_SUCCESS;
		}
	}
	else if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("buildin_channel_chat: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (!ch) {
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	color = ch->color;
	FETCH(4, color);

	safesnprintf(output, CHAT_SIZE_MAX, "%s %s", ch->alias, msg);
	clif_channel_msg(ch,output,color);
	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Ban player from a channel
 * channel_ban "<chname>",<char_id>;
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_ban) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2);
	unsigned int char_id = script_getnum(st,3);
	TBL_PC *tsd;

	if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("buildin_channel_ban: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (ch->char_id == char_id) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	tsd = map_charid2sd(char_id);
	if (tsd && pc_has_permission(tsd,PC_PERM_CHANNEL_ADMIN)) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (!idb_exists(ch->banned, char_id)) {
		struct chan_banentry *cbe;
		char output[CHAT_SIZE_MAX+1];

		CREATE(cbe, struct chan_banentry, 1);
		cbe->char_id = char_id;
		if (tsd) {
			strcpy(cbe->char_name,tsd->status.name);
			channel_clean(ch,tsd,0);
			safesnprintf(output, CHAT_SIZE_MAX, msg_txt(tsd,769), ch->alias, tsd->status.name); // %s %s has been banned.
		}
		else
			safesnprintf(output, CHAT_SIZE_MAX, msg_txt(tsd,769), ch->alias, "****"); // %s %s has been banned.
		idb_put(ch->banned, char_id, cbe);
		clif_channel_msg(ch,output,ch->color);
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Ban player from a channel
 * channel_unban "<chname>",<char_id>;
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_unban) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2);
	unsigned int char_id = script_getnum(st,3);

	if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("buildin_channel_unban: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (ch->char_id == char_id) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	if (idb_exists(ch->banned, char_id)) {
		char output[CHAT_SIZE_MAX+1];
		TBL_PC *tsd = map_charid2sd(char_id);
		if (tsd)
			safesnprintf(output, CHAT_SIZE_MAX, msg_txt(tsd,770), ch->alias, tsd->status.name); // %s %s has been unbanned
		else
			safesnprintf(output, CHAT_SIZE_MAX, msg_txt(tsd,770), ch->alias, "****"); // %s %s has been unbanned.
		idb_remove(ch->banned, char_id);
		clif_channel_msg(ch,output,ch->color);
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Kick player from a channel
 * channel_kick "<chname>",<char_id>;
 * channel_kick "<chname>","<char_name>";
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_kick) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2);
	struct script_data *data = script_getdata(st,3);
	TBL_PC *tsd = NULL;
	int res = 1;

	get_val(st, data);
	if (data_isstring(data)) {
		if (!(tsd = map_nick2sd(conv_str(st,data),false))) {
			ShowError("buildin_channel_kick: Player with nick '%s' is not online\n", conv_str(st,data));
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}
	}
	else {
		if (!(tsd = map_charid2sd(conv_num(st,data)))) {
			ShowError("buildin_channel_kick: Player with char_id '%d' is not online\n", conv_num(st,data));
			script_pushint(st,0);
			return SCRIPT_CMD_FAILURE;
		}
	}

	if (!(ch = channel_name2channel((char *)chname, tsd, 0))) {
		ShowError("buildin_channel_kick: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,0);
		return SCRIPT_CMD_FAILURE;
	}

	if (channel_pc_haschan(tsd, ch) < 0 || ch->char_id == tsd->status.char_id || pc_has_permission(tsd,PC_PERM_CHANNEL_ADMIN)) {
		script_pushint(st,0);
		return SCRIPT_CMD_SUCCESS;
	}

	switch(ch->type){
		case CHAN_TYPE_ALLY: res = channel_pcquit(tsd,3); break;
		case CHAN_TYPE_MAP: res = channel_pcquit(tsd,4); break;
		default: res = channel_clean(ch,tsd,0); break;
	}
	
	if (res == 0) {
		char output[CHAT_SIZE_MAX+1];
		safesnprintf(output, CHAT_SIZE_MAX, msg_txt(tsd,889), ch->alias, tsd->status.name); // "%s %s is kicked"
		clif_channel_msg(ch,output,ch->color);
	}

	script_pushint(st,1);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Delete a channel
 * channel_delete "<chname>";
 * @author [Cydh]
 **/
BUILDIN_FUNC(channel_delete) {
	struct Channel *ch = NULL;
	const char *chname = script_getstr(st,2);

	if (!(ch = channel_name2channel((char *)chname, NULL, 0))) {
		ShowError("channel_delete: Channel name '%s' is invalid.\n", chname);
		script_pushint(st,-1);
		return SCRIPT_CMD_FAILURE;
	}

	script_pushint(st,channel_delete(ch,true));
	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(unloadnpc) {
	const char *name;
	struct npc_data* nd;

	name = script_getstr(st, 2);
	nd = npc_name2id(name);

	if( nd == NULL ){
		ShowError( "buildin_unloadnpc: npc '%s' was not found.\n", name );
		return SCRIPT_CMD_FAILURE;
	} else if ( nd->bl.id == st->oid ) {
		// Supporting self-unload isn't worth the problem it may cause. [Secret]
		ShowError("buildin_unloadnpc: You cannot self-unload NPC '%s'.\n.", name);
		return SCRIPT_CMD_FAILURE;
	}

	npc_unload_duplicates(nd);
	npc_unload(nd, true);
	npc_read_event_script();

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Add an achievement to the player's log
 * achievementadd(<achievement ID>{,<char ID>});
 */
BUILDIN_FUNC(achievementadd) {
	struct map_session_data *sd;
	int achievement_id = script_getnum(st, 2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (achievement_exists(achievement_id) == false) {
		ShowWarning("buildin_achievementadd: Achievement '%d' doesn't exist.\n", achievement_id);
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if( !sd->state.pc_loaded ){
		if( !running_npc_stat_calc_event ){
			ShowError( "buildin_achievementadd: call was too early. Character %s(CID: '%u') was not loaded yet.\n", sd->status.name, sd->status.char_id );
			return SCRIPT_CMD_FAILURE;
		}else{
			// Simply ignore it on the first call, because the status will be recalculated after loading anyway
			return SCRIPT_CMD_SUCCESS;
		}
	}

	if (achievement_add(sd, achievement_id))
		script_pushint(st, true);
	else
		script_pushint(st, false);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Removes an achievement on a player.
 * achievementremove(<achievement ID>{,<char ID>});
 * Just for Atemo. ;)
 */
BUILDIN_FUNC(achievementremove) {
	struct map_session_data *sd;
	int achievement_id = script_getnum(st, 2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (achievement_exists(achievement_id) == false) {
		ShowWarning("buildin_achievementremove: Achievement '%d' doesn't exist.\n", achievement_id);
		script_pushint(st, false);
		return SCRIPT_CMD_SUCCESS;
	}

	if( !sd->state.pc_loaded ){
		if( !running_npc_stat_calc_event ){
			ShowError( "buildin_achievementremove: call was too early. Character %s(CID: '%u') was not loaded yet.\n", sd->status.name, sd->status.char_id );
			return SCRIPT_CMD_FAILURE;
		}else{
			// Simply ignore it on the first call, because the status will be recalculated after loading anyway
			return SCRIPT_CMD_SUCCESS;
		}
	}

	if (achievement_remove(sd, achievement_id))
		script_pushint(st, true);
	else
		script_pushint(st, false);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Returns achievement progress
 * achievementinfo(<achievement ID>,<type>{,<char ID>});
 */
BUILDIN_FUNC(achievementinfo) {
	struct map_session_data *sd;
	int achievement_id = script_getnum(st, 2);

	if (!script_charid2sd(4, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (achievement_exists(achievement_id) == false) {
		ShowWarning("buildin_achievementinfo: Achievement '%d' doesn't exist.\n", achievement_id);
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if( !sd->state.pc_loaded ){
		script_pushint(st, false);
		if( !running_npc_stat_calc_event ){
			ShowError( "buildin_achievementinfo: call was too early. Character %s(CID: '%u') was not loaded yet.\n", sd->status.name, sd->status.char_id );
			return SCRIPT_CMD_FAILURE;
		}else{
			// Simply ignore it on the first call, because the status will be recalculated after loading anyway
			return SCRIPT_CMD_SUCCESS;
		}
	}

	script_pushint(st, achievement_check_progress(sd, achievement_id, script_getnum(st, 3)));
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Award an achievement; Ignores requirements
 * achievementcomplete(<achievement ID>{,<char ID>});
 */
BUILDIN_FUNC(achievementcomplete) {
	struct map_session_data *sd;
	int i, achievement_id = script_getnum(st, 2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (achievement_exists(achievement_id) == false) {
		ShowWarning("buildin_achievementcomplete: Achievement '%d' doesn't exist.\n", achievement_id);
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}
	
	if( !sd->state.pc_loaded ){
		if( !running_npc_stat_calc_event ){
			ShowError( "buildin_achievementcomplete: call was too early. Character %s(CID: '%u') was not loaded yet.\n", sd->status.name, sd->status.char_id );
			return SCRIPT_CMD_FAILURE;
		}else{
			// Simply ignore it on the first call, because the status will be recalculated after loading anyway
			return SCRIPT_CMD_SUCCESS;
		}
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.count)
		achievement_add(sd, achievement_id);
	achievement_update_achievement(sd, achievement_id, true);
	script_pushint(st, true);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Checks if the achievement exists on player.
 * achievementexists(<achievement ID>{,<char ID>});
 */
BUILDIN_FUNC(achievementexists) {
	struct map_session_data *sd;
	int i, achievement_id = script_getnum(st, 2);

	if (!script_charid2sd(3, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (achievement_exists(achievement_id) == false) {
		ShowWarning("buildin_achievementexists: Achievement '%d' doesn't exist.\n", achievement_id);
		script_pushint(st, false);
		return SCRIPT_CMD_SUCCESS;
	}

	if( !sd->state.pc_loaded ){
		script_pushint(st, false);
		if( !running_npc_stat_calc_event ){
			ShowError( "buildin_achievementexists: call was too early. Character %s(CID: '%u') was not loaded yet.\n", sd->status.name, sd->status.char_id );
			return SCRIPT_CMD_FAILURE;
		}else{
			// Simply ignore it on the first call, because the status will be recalculated after loading anyway
			return SCRIPT_CMD_SUCCESS;
		}
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	script_pushint(st, i < sd->achievement_data.count ? true : false);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Updates an achievement's value.
 * achievementupdate(<achievement ID>,<type>,<value>{,<char ID>});
 */
BUILDIN_FUNC(achievementupdate) {
	struct map_session_data *sd;
	int i, achievement_id, type, value;

	achievement_id = script_getnum(st, 2);
	type = script_getnum(st, 3);
	value = script_getnum(st, 4);

	if (!script_charid2sd(5, sd)) {
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if (achievement_exists(achievement_id) == false) {
		ShowWarning("buildin_achievementupdate: Achievement '%d' doesn't exist.\n", achievement_id);
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	if( !sd->state.pc_loaded ){
		if( !running_npc_stat_calc_event ){
			ShowError( "buildin_achievementupdate: call was too early. Character %s(CID: '%u') was not loaded yet.\n", sd->status.name, sd->status.char_id );
			return SCRIPT_CMD_FAILURE;
		}else{
			// Simply ignore it on the first call, because the status will be recalculated after loading anyway
			return SCRIPT_CMD_SUCCESS;
		}
	}

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.count)
		achievement_add(sd, achievement_id);

	ARR_FIND(0, sd->achievement_data.count, i, sd->achievement_data.achievements[i].achievement_id == achievement_id);
	if (i == sd->achievement_data.count) {
		script_pushint(st, false);
		return SCRIPT_CMD_SUCCESS;
	}

	if (type >= ACHIEVEINFO_COUNT1 && type <= ACHIEVEINFO_COUNT10)
		sd->achievement_data.achievements[i].count[type - 1] = value;
	else if (type == ACHIEVEINFO_COMPLETE || type == ACHIEVEINFO_COMPLETEDATE)
		sd->achievement_data.achievements[i].completed = value;
	else if (type == ACHIEVEINFO_GOTREWARD)
		sd->achievement_data.achievements[i].rewarded = value;
	else {
		ShowWarning("buildin_achievementupdate: Unknown type '%d'.\n", type);
		script_pushint(st, false);
		return SCRIPT_CMD_FAILURE;
	}

	achievement_update_achievement(sd, achievement_id, false);
	script_pushint(st, true);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Get an equipment's refine cost
 * getequiprefinecost(<equipment slot>,<type>,<information>{,<char id>})
 * returns -1 on fail
 */
BUILDIN_FUNC(getequiprefinecost) {
	int i = -1, slot, type, info;
	map_session_data *sd;

	slot = script_getnum(st, 2);
	type = script_getnum(st, 3);
	info = script_getnum(st, 4);

	if (!script_charid2sd(5, sd)) {
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}

	if (type < 0 || type >= REFINE_COST_MAX) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	if (equip_index_check(slot))
		i = pc_checkequip(sd, equip_bitmask[slot]);

	if (i < 0) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}

	int weapon_lv = sd->inventory_data[i]->wlv;
	if (sd->inventory_data[i]->type == IT_SHADOWGEAR) {
		if (sd->inventory_data[i]->equip == EQP_SHADOW_WEAPON)
			weapon_lv = REFINE_TYPE_WEAPON4;
		else
			weapon_lv = REFINE_TYPE_SHADOW;
	}

	script_pushint(st, status_get_refine_cost(weapon_lv, type, info != 0));

	return SCRIPT_CMD_SUCCESS;
}

/**
 * Round, floor, ceiling a number to arbitrary integer precision.
 * round(<number>,<precision>);
 * ceil(<number>,<precision>);
 * floor(<number>,<precision>);
 */
BUILDIN_FUNC(round) {
	int num = script_getnum(st, 2);
	int precision = script_getnum(st, 3);
	char* func = script_getfuncname(st);

	if (precision <= 0) {
		ShowError("buildin_round: Attempted to use zero or negative number as arbitrary precision.\n");
		return SCRIPT_CMD_FAILURE;
	}

	if (strcasecmp(func, "floor") == 0) {
		script_pushint(st, num - (num % precision));
	}
	else if (strcasecmp(func, "ceil") == 0) {
		script_pushint(st, num + precision - (num % precision));
	}
	else {
		script_pushint(st, (int)(round(num / (precision * 1.))) * precision);
	}

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(getequiptradability) {
	int i, num;
	TBL_PC *sd;

	num = script_getnum(st, 2);

	if (!script_charid2sd(3, sd)) {
		return SCRIPT_CMD_FAILURE;
	}

	if (equip_index_check(num))
		i = pc_checkequip(sd, equip_bitmask[num]);
	else{
		ShowError("buildin_getequiptradability: Unknown equip index '%d'\n",num);
		return SCRIPT_CMD_FAILURE;
	}

	if (i >= 0) {
		bool tradable = (sd->inventory.u.items_inventory[i].expire_time == 0 &&
			(!sd->inventory.u.items_inventory[i].bound || pc_can_give_bounded_items(sd)) &&
			itemdb_cantrade(&sd->inventory.u.items_inventory[i], pc_get_group_level(sd), pc_get_group_level(sd))
			);
		script_pushint(st, tradable);
	}
	else
		script_pushint(st, false);

	return SCRIPT_CMD_SUCCESS;
}

static inline bool mail_sub( struct script_state *st, struct script_data *data, struct map_session_data *sd, int i, const char **out_name, unsigned int *start, unsigned int *end, int32 *id ){
	const char *name;

	// Check if it is a variable
	if( !data_isreference(data) ){
		ShowError("buildin_mail: argument %d is not a variable.\n", i );
		return false;
	}

	name = reference_getname(data);

	if( is_string_variable(name) ){
		ShowError( "buildin_mail: variable \"%s\" is a string variable.\n", name );
		return false;
	}

	// Check if the variable requires a player
	if( not_server_variable(*name) && sd == NULL ){
		// If no player is attached
		if( !script_rid2sd(sd) ){
			ShowError( "buildin_mail: variable \"%s\" was not a server variable, but no player was attached.\n", name );
			return false;
		}
	}

	// Try to find the array's source pointer
	if( !script_array_src( st, sd, name, reference_getref(data) ) ){
		ShowError( "buildin_mail: variable \"%s\" is not an array.\n" );
		return false;
	}

	// Store the name for later usage
	*out_name = name;

	// Get the start and end indices of the array
	*start = reference_getindex(data);
	*end = script_array_highest_key( st, sd, name, reference_getref(data) );

	// For getting the values we need the id of the array
	*id = reference_getid(data);

	return true;
}

BUILDIN_FUNC(mail){
	const char *sender, *title, *body, *name;
	struct mail_message msg;
	struct script_data *data;
	struct map_session_data *sd = NULL;
	unsigned int i, j, k, num_items, start, end;
	int32 id;

	memset(&msg, 0, sizeof(struct mail_message));

	msg.dest_id = script_getnum(st,2);

	sender = script_getstr(st, 3);

	if( strlen(sender) > NAME_LENGTH ){
		ShowError( "buildin_mail: sender name can not be longer than %d characters.\n", NAME_LENGTH );
		return SCRIPT_CMD_FAILURE;
	}

	safestrncpy(msg.send_name, sender, NAME_LENGTH);

	title = script_getstr(st, 4);

	if( strlen(title) > MAIL_TITLE_LENGTH ){
		ShowError( "buildin_mail: title can not be longer than %d characters.\n", MAIL_TITLE_LENGTH );
		return SCRIPT_CMD_FAILURE;
	}

	safestrncpy(msg.title, title, MAIL_TITLE_LENGTH);

	body = script_getstr(st, 5);

	if( strlen(body) > MAIL_BODY_LENGTH ){
		ShowError( "buildin_mail: body can not be longer than %d characters.\n", MAIL_BODY_LENGTH );
		return SCRIPT_CMD_FAILURE;
	}

	safestrncpy(msg.body, body, MAIL_BODY_LENGTH);

	if( script_hasdata(st,6) ){
		int zeny = script_getnum(st, 6);

		if( zeny < 0 ){
			ShowError( "buildin_mail: a negative amount of zeny can not be sent.\n" );
			return SCRIPT_CMD_FAILURE;
		}else if( zeny > MAX_ZENY ){
			ShowError( "buildin_mail: amount of zeny %u is exceeding maximum of %u. Capping...\n", zeny, MAX_ZENY );
			zeny = MAX_ZENY;
		}

		msg.zeny = zeny;
	}

	// Items
	num_items = 0;
	while( script_hasdata(st,7) ){
		data = script_getdata(st,7);

		if( !mail_sub( st, data, sd, 7, &name, &start, &end, &id ) ){
			return SCRIPT_CMD_FAILURE;
		}

		num_items = end - start;

		if( num_items == 0 ){
			ShowWarning( "buildin_mail: array \"%s\" contained no items.\n", name );
			break;
		}

		if( num_items > MAIL_MAX_ITEM ){
			ShowWarning( "buildin_mail: array \"%s\" contained %d items, capping to maximum of %d...\n", name, num_items, MAIL_MAX_ITEM );
			num_items = MAIL_MAX_ITEM;
		}

		for( i = 0; i < num_items && start < end; i++, start++ ){
			msg.item[i].nameid = (int32)__64BPRTSIZE( get_val2( st, reference_uid( id, start ), reference_getref( data ) ) );
			msg.item[i].identify = 1;

			script_removetop(st, -1, 0);

			if( !itemdb_exists(msg.item[i].nameid) ){
				ShowError( "buildin_mail: invalid item id %hu.\n", msg.item[i].nameid );
				return SCRIPT_CMD_FAILURE;
			}
		}

		// Amounts
		if( !script_hasdata(st,8) ){
			ShowError("buildin_mail: missing item count variable at position %d.\n", 8);
			return SCRIPT_CMD_FAILURE;
		}

		data = script_getdata(st,8);

		if( !mail_sub( st, data, sd, 8, &name, &start, &end, &id ) ){
			return SCRIPT_CMD_FAILURE;
		}

		for( i = 0; i < num_items && start < end; i++, start++ ){
			struct item_data *item = itemdb_exists(msg.item[i].nameid);

			msg.item[i].amount = (int32)__64BPRTSIZE( get_val2( st, reference_uid( id, start ), reference_getref( data ) ) );

			script_removetop(st, -1, 0);

			if( msg.item[i].amount <= 0 ){
				ShowError( "buildin_mail: amount %d for item %hu is invalid.\n", msg.item[i].amount, msg.item[i].nameid );
				return SCRIPT_CMD_FAILURE;
			}else if( itemdb_isstackable2(item) ){
				uint16 max = item->stack.amount > 0 ? item->stack.amount : MAX_AMOUNT;

				if( msg.item[i].amount > max ){
					ShowWarning( "buildin_mail: amount %d for item %hu is exceeding the maximum of %d. Capping...\n", msg.item[i].amount, msg.item[i].nameid, max );
					msg.item[i].amount = max;
				}
			}else{
				if( msg.item[i].amount > 1 ){
					ShowWarning( "buildin_mail: amount %d is invalid for non-stackable item %hu.\n", msg.item[i].amount, msg.item[i].nameid );
					msg.item[i].amount = 1;
				}
			}
		}

		// Cards
		if( !script_hasdata(st,9) ){
			break;
		}

		for( i = 0, j = 9; i < MAX_SLOTS && script_hasdata(st,j); i++, j++ ){
			data = script_getdata(st,j);

			if( !mail_sub( st, data, sd, j + 1, &name, &start, &end, &id ) ){
				return SCRIPT_CMD_FAILURE;
			}

			for( k = 0; k < num_items && start < end; k++, start++ ){
				msg.item[k].card[i] = (int32)__64BPRTSIZE( get_val2( st, reference_uid( id, start ), reference_getref( data ) ) );

				script_removetop(st, -1, 0);

				if( msg.item[k].card[i] != 0 && !itemdb_exists(msg.item[k].card[i]) ){
					ShowError( "buildin_mail: invalid card id %hu.\n", msg.item[k].card[i] );
					return SCRIPT_CMD_FAILURE;
				}
			}
		}
	
		// Random Options
		if( !script_hasdata(st,9 + MAX_SLOTS) ){
			break;
		}

		for( i = 0, j = 9 + MAX_SLOTS; i < MAX_ITEM_RDM_OPT && script_hasdata(st,j) && script_hasdata(st,j + 1) && script_hasdata(st,j + 2); i++, j++ ){
			// Option IDs
			data = script_getdata(st, j);

			if( !mail_sub( st, data, sd, j + 1, &name, &start, &end, &id ) ){
				return SCRIPT_CMD_FAILURE;
			}

			for( k = 0; k < num_items && start < end; k++, start++ ){
				msg.item[k].option[i].id = (int32)__64BPRTSIZE( get_val2( st, reference_uid( id, start ), reference_getref( data ) ) );

				script_removetop(st, -1, 0);
			}

			j++;

			// Option values
			data = script_getdata(st, j);

			if( !mail_sub( st, data, sd, j + 1, &name, &start, &end, &id ) ){
				return SCRIPT_CMD_FAILURE;
			}

			for( k = 0; k < num_items && start < end; k++, start++ ){
				msg.item[k].option[i].value = (int32)__64BPRTSIZE( get_val2( st, reference_uid( id, start ), reference_getref( data ) ) );

				script_removetop(st, -1, 0);
			}

			j++;

			// Option parameters
			data = script_getdata(st, j);

			if( !mail_sub( st, data, sd, j + 1, &name, &start, &end, &id ) ){
				return SCRIPT_CMD_FAILURE;
			}

			for( k = 0; k < num_items && start < end; k++, start++ ){
				msg.item[k].option[i].param = (int32)__64BPRTSIZE( get_val2( st, reference_uid( id, start ), reference_getref( data ) ) );

				script_removetop(st, -1, 0);
			}
		}

		// Break the pseudo scope
		break;
	}

	msg.status = MAIL_NEW;
	msg.type = MAIL_INBOX_NORMAL;
	msg.timestamp = time(NULL);

	intif_Mail_send(0, &msg);

	return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(open_roulette){
#if PACKETVER < 20141022
	ShowError( "buildin_open_roulette: This command requires PACKETVER 2014-10-22 or newer.\n" );
	return SCRIPT_CMD_FAILURE;
#else
	struct map_session_data* sd;

	if( !battle_config.feature_roulette ){
		ShowError( "buildin_open_roulette: Roulette system is disabled.\n" );
		return SCRIPT_CMD_FAILURE;
	}

	if( !script_charid2sd( 2, sd ) ){
		return SCRIPT_CMD_FAILURE;
	}

	clif_roulette_open(sd);

	return SCRIPT_CMD_SUCCESS;
#endif
}

/*==========================================
 * identifyall({<type>{,<account_id>}})
 * <type>:
 *	true: identify the items and returns the count of unidentified items (default)
 *	false: returns the count of unidentified items only
 *------------------------------------------*/
BUILDIN_FUNC(identifyall) {
	TBL_PC *sd;
	bool identify_item = true;

	if (script_hasdata(st, 2))
		identify_item = script_getnum(st, 2) != 0;

	if( !script_hasdata(st, 3) )
		script_rid2sd(sd);
	else
		sd = map_id2sd( script_getnum(st, 3) );

	if (sd == NULL) {
		script_pushint(st, -1);
		return SCRIPT_CMD_SUCCESS;
	}
	script_pushint(st, pc_identifyall(sd, identify_item));

	return SCRIPT_CMD_SUCCESS;
}

#include "../custom/script.inc"

// declarations that were supposed to be exported from npc_chat.c
#ifdef PCRE_SUPPORT
BUILDIN_FUNC(defpattern);
BUILDIN_FUNC(activatepset);
BUILDIN_FUNC(deactivatepset);
BUILDIN_FUNC(deletepset);
#endif

/** Regular expression matching
 * preg_match(<pattern>,<string>{,<offset>})
 */
BUILDIN_FUNC(preg_match) {
#ifdef PCRE_SUPPORT
	pcre *re;
	pcre_extra *pcreExtra;
	const char *error;
	int erroffset, r, offset = 0;
	int subStrVec[30];

	const char* pattern = script_getstr(st,2);
	const char* subject = script_getstr(st,3);
	if (script_hasdata(st,4))
		offset = script_getnum(st,4);

	re = pcre_compile(pattern, 0, &error, &erroffset, NULL);
	pcreExtra = pcre_study(re, 0, &error);

	r = pcre_exec(re, pcreExtra, subject, (int)strlen(subject), offset, 0, subStrVec, 30);

	pcre_free(re);
	if (pcreExtra != NULL)
		pcre_free(pcreExtra);

	if (r < 0)
		script_pushint(st,0);
	else
		script_pushint(st,(r > 0) ? r : 30 / 3);

	return SCRIPT_CMD_SUCCESS;
#else
	ShowDebug("script:preg_match: cannot run without PCRE library enabled.\n");
	script_pushint(st,0);
	return SCRIPT_CMD_SUCCESS;
#endif
}

/// script command definitions
/// for an explanation on args, see add_buildin_func
struct script_function buildin_func[] = {
	// NPC interaction
	BUILDIN_DEF(mes,"s*"),
	BUILDIN_DEF(next,""),
	BUILDIN_DEF(clear,""),
	BUILDIN_DEF(close,""),
	BUILDIN_DEF(close2,""),
	BUILDIN_DEF(menu,"sl*"),
	BUILDIN_DEF(select,"s*"), //for future jA script compatibility
	BUILDIN_DEF(prompt,"s*"),
	//
	BUILDIN_DEF(goto,"l"),
	BUILDIN_DEF(callsub,"l*"),
	BUILDIN_DEF(callfunc,"s*"),
	BUILDIN_DEF(return,"?"),
	BUILDIN_DEF(getarg,"i?"),
	BUILDIN_DEF(jobchange,"i??"),
	BUILDIN_DEF(jobname,"i"),
	BUILDIN_DEF(input,"r??"),
	BUILDIN_DEF(warp,"sii?"),
	BUILDIN_DEF2(warp, "warpchar", "sii?"),
	BUILDIN_DEF(areawarp,"siiiisii??"),
	BUILDIN_DEF(warpparty,"siii???"), // [Fredzilla] [Paradox924X]
	BUILDIN_DEF(warpguild,"siii"), // [Fredzilla]
	BUILDIN_DEF(setlook,"ii?"),
	BUILDIN_DEF(changelook,"ii?"), // Simulates but don't Store it
	BUILDIN_DEF2(setr,"set","rv?"),
	BUILDIN_DEF(setr,"rv??"), // Not meant to be used directly, required for var++/var--
	BUILDIN_DEF(setarray,"rv*"),
	BUILDIN_DEF(cleararray,"rvi"),
	BUILDIN_DEF(copyarray,"rri"),
	BUILDIN_DEF(getarraysize,"r"),
	BUILDIN_DEF(deletearray,"r?"),
	BUILDIN_DEF(getelementofarray,"ri"),
	BUILDIN_DEF(inarray,"rv"),
	BUILDIN_DEF(countinarray,"rr"),
	BUILDIN_DEF(getitem,"vi?"),
	BUILDIN_DEF(rentitem,"vi?"),
	BUILDIN_DEF(rentitem2,"viiiiiiii?"),
	BUILDIN_DEF(getitem2,"viiiiiiii?"),
	BUILDIN_DEF(getnameditem,"vv"),
	BUILDIN_DEF2(grouprandomitem,"groupranditem","i?"),
	BUILDIN_DEF(makeitem,"visii"),
	BUILDIN_DEF(makeitem2,"visiiiiiiiii"),
	BUILDIN_DEF(delitem,"vi?"),
	BUILDIN_DEF2(delitem,"storagedelitem","vi?"),
	BUILDIN_DEF2(delitem,"guildstoragedelitem","vi?"),
	BUILDIN_DEF2(delitem,"cartdelitem","vi?"),
	BUILDIN_DEF(delitem2,"viiiiiiii?"),
	BUILDIN_DEF2(delitem2,"storagedelitem2","viiiiiiii?"),
	BUILDIN_DEF2(delitem2,"guildstoragedelitem2","viiiiiiii?"),
	BUILDIN_DEF2(delitem2,"cartdelitem2","viiiiiiii?"),
	BUILDIN_DEF2(enableitemuse,"enable_items",""),
	BUILDIN_DEF2(disableitemuse,"disable_items",""),
	BUILDIN_DEF(cutin,"si"),
	BUILDIN_DEF(viewpoint,"iiiii"),
	BUILDIN_DEF(heal,"ii?"),
	BUILDIN_DEF(itemheal,"ii?"),
	BUILDIN_DEF(percentheal,"ii?"),
	BUILDIN_DEF(rand,"i?"),
	BUILDIN_DEF(countitem,"v?"),
	BUILDIN_DEF2(countitem,"storagecountitem","v?"),
	BUILDIN_DEF2(countitem,"guildstoragecountitem","v?"),
	BUILDIN_DEF2(countitem,"cartcountitem","v?"),
	BUILDIN_DEF2(countitem,"countitem2","viiiiiii?"),
	BUILDIN_DEF2(countitem,"storagecountitem2","viiiiiii?"),
	BUILDIN_DEF2(countitem,"guildstoragecountitem2","viiiiiii?"),
	BUILDIN_DEF2(countitem,"cartcountitem2","viiiiiii?"),
	BUILDIN_DEF(checkweight,"vi*"),
	BUILDIN_DEF(checkweight2,"rr"),
	BUILDIN_DEF(readparam,"i?"),
	BUILDIN_DEF(getcharid,"i?"),
	BUILDIN_DEF(getnpcid,"i?"),
	BUILDIN_DEF(getpartyname,"i"),
	BUILDIN_DEF(getpartymember,"i??"),
	BUILDIN_DEF(getpartyleader,"i?"),
	BUILDIN_DEF(getguildname,"i"),
	BUILDIN_DEF(getguildmaster,"i"),
	BUILDIN_DEF(getguildmasterid,"i"),
	BUILDIN_DEF(strcharinfo,"i?"),
	BUILDIN_DEF(strnpcinfo,"i"),
	BUILDIN_DEF(getequipid,"??"),
	BUILDIN_DEF(getequipuniqueid,"i?"),
	BUILDIN_DEF(getequipname,"i?"),
	BUILDIN_DEF(getbrokenid,"i?"), // [Valaris]
	BUILDIN_DEF(repair,"i?"), // [Valaris]
	BUILDIN_DEF(repairall,"?"),
	BUILDIN_DEF(getequipisequiped,"i?"),
	BUILDIN_DEF(getequipisenableref,"i?"),
	BUILDIN_DEF(getequiprefinerycnt,"i?"),
	BUILDIN_DEF(getequipweaponlv,"i?"),
	BUILDIN_DEF(getequippercentrefinery,"i?"),
	BUILDIN_DEF(successrefitem,"i??"),
	BUILDIN_DEF(failedrefitem,"i?"),
	BUILDIN_DEF(downrefitem,"i??"),
	BUILDIN_DEF(statusup,"i?"),
	BUILDIN_DEF(statusup2,"ii?"),
	BUILDIN_DEF(bonus,"i?"),
	BUILDIN_DEF2(bonus,"bonus2","ivi"),
	BUILDIN_DEF2(bonus,"bonus3","ivii"),
	BUILDIN_DEF2(bonus,"bonus4","ivvii"),
	BUILDIN_DEF2(bonus,"bonus5","ivviii"),
	BUILDIN_DEF(autobonus,"sii??"),
	BUILDIN_DEF(autobonus2,"sii??"),
	BUILDIN_DEF(autobonus3,"siiv?"),
	BUILDIN_DEF(skill,"vi?"),
	BUILDIN_DEF2(skill,"addtoskill","vi?"), // [Valaris]
	BUILDIN_DEF(guildskill,"vi"),
	BUILDIN_DEF(getskilllv,"v"),
	BUILDIN_DEF(getgdskilllv,"iv"),
	BUILDIN_DEF(basicskillcheck,""),
	BUILDIN_DEF(getgmlevel,"?"),
	BUILDIN_DEF(getgroupid,"?"),
	BUILDIN_DEF(end,""),
	BUILDIN_DEF(checkoption,"i?"),
	BUILDIN_DEF(setoption,"i??"),
	BUILDIN_DEF(setcart,"??"),
	BUILDIN_DEF(checkcart,"?"),
	BUILDIN_DEF(setfalcon,"??"),
	BUILDIN_DEF(checkfalcon,"?"),
	BUILDIN_DEF(setriding,"??"),
	BUILDIN_DEF(checkriding,"?"),
	BUILDIN_DEF(checkwug,"?"),
	BUILDIN_DEF(checkmadogear,"?"),
	BUILDIN_DEF(setmadogear,"??"),
	BUILDIN_DEF2(savepoint,"save","sii???"),
	BUILDIN_DEF(savepoint,"sii???"),
	BUILDIN_DEF(gettimetick,"i"),
	BUILDIN_DEF(gettime,"i"),
	BUILDIN_DEF(gettimestr,"si?"),
	BUILDIN_DEF(openstorage,""),
	BUILDIN_DEF(guildopenstorage,""),
	BUILDIN_DEF(itemskill,"vi?"),
	BUILDIN_DEF(produce,"i"),
	BUILDIN_DEF(cooking,"i"),
	BUILDIN_DEF(monster,"siisii???"),
	BUILDIN_DEF(getmobdrops,"i"),
	BUILDIN_DEF(areamonster,"siiiisii???"),
	BUILDIN_DEF(killmonster,"ss?"),
	BUILDIN_DEF(killmonsterall,"s?"),
	BUILDIN_DEF(clone,"siisi????"),
	BUILDIN_DEF(doevent,"s"),
	BUILDIN_DEF(donpcevent,"s"),
	BUILDIN_DEF(cmdothernpc,"ss"),
	BUILDIN_DEF(addtimer,"is"),
	BUILDIN_DEF(deltimer,"s"),
	BUILDIN_DEF(addtimercount,"is"),
	BUILDIN_DEF(initnpctimer,"??"),
	BUILDIN_DEF(stopnpctimer,"??"),
	BUILDIN_DEF(startnpctimer,"??"),
	BUILDIN_DEF(setnpctimer,"i?"),
	BUILDIN_DEF(getnpctimer,"i?"),
	BUILDIN_DEF(attachnpctimer,"?"), // attached the player id to the npc timer [Celest]
	BUILDIN_DEF(detachnpctimer,"?"), // detached the player id from the npc timer [Celest]
	BUILDIN_DEF(playerattached,""), // returns id of the current attached player. [Skotlex]
	BUILDIN_DEF(announce,"si?????"),
	BUILDIN_DEF(mapannounce,"ssi?????"),
	BUILDIN_DEF(areaannounce,"siiiisi?????"),
	BUILDIN_DEF(getusers,"i"),
	BUILDIN_DEF(getmapguildusers,"si"),
	BUILDIN_DEF(getmapusers,"s"),
	BUILDIN_DEF(getareausers,"siiii"),
	BUILDIN_DEF(getareadropitem,"siiiiv"),
	BUILDIN_DEF(enablenpc,"s"),
	BUILDIN_DEF(disablenpc,"s"),
	BUILDIN_DEF(hideoffnpc,"s"),
	BUILDIN_DEF(hideonnpc,"s"),
	BUILDIN_DEF(sc_start,"iii???"),
	BUILDIN_DEF2(sc_start,"sc_start2","iiii???"),
	BUILDIN_DEF2(sc_start,"sc_start4","iiiiii???"),
	BUILDIN_DEF(sc_end,"i?"),
	BUILDIN_DEF(sc_end_class,"??"),
	BUILDIN_DEF(getstatus, "i??"),
	BUILDIN_DEF(getscrate,"ii?"),
	BUILDIN_DEF(debugmes,"s"),
	BUILDIN_DEF2(catchpet,"pet","i"),
	BUILDIN_DEF2(birthpet,"bpet",""),
	BUILDIN_DEF(catchpet,"i"),
	BUILDIN_DEF(birthpet,""),
	BUILDIN_DEF(resetlvl,"i?"),
	BUILDIN_DEF(resetstatus,"?"),
	BUILDIN_DEF(resetskill,"?"),
	BUILDIN_DEF(skillpointcount,"?"),
	BUILDIN_DEF(changebase,"i?"),
	BUILDIN_DEF(changesex,"?"),
	BUILDIN_DEF(changecharsex,"?"),
	BUILDIN_DEF(waitingroom,"si?????"),
	BUILDIN_DEF(delwaitingroom,"?"),
	BUILDIN_DEF(waitingroomkick,"ss"),
	BUILDIN_DEF(getwaitingroomusers, "?"),
	BUILDIN_DEF2(waitingroomkickall,"kickwaitingroomall","?"),
	BUILDIN_DEF(enablewaitingroomevent,"?"),
	BUILDIN_DEF(disablewaitingroomevent,"?"),
	BUILDIN_DEF2(enablewaitingroomevent,"enablearena",""),		// Added by RoVeRT
	BUILDIN_DEF2(disablewaitingroomevent,"disablearena",""),	// Added by RoVeRT
	BUILDIN_DEF(getwaitingroomstate,"i?"),
	BUILDIN_DEF(warpwaitingpc,"sii?"),
	BUILDIN_DEF(attachrid,"i?"),
	BUILDIN_DEF(addrid,"i?????"),
	BUILDIN_DEF(detachrid,""),
	BUILDIN_DEF(isloggedin,"i?"),
	BUILDIN_DEF(setmapflagnosave,"ssii"),
	BUILDIN_DEF(getmapflag,"si?"),
	BUILDIN_DEF(setmapflag,"si??"),
	BUILDIN_DEF(removemapflag,"si?"),
	BUILDIN_DEF(pvpon,"s"),
	BUILDIN_DEF(pvpoff,"s"),
	BUILDIN_DEF(gvgon,"s"),
	BUILDIN_DEF(gvgoff,"s"),
	BUILDIN_DEF(emotion,"i?"),
	BUILDIN_DEF(maprespawnguildid,"sii"),
	BUILDIN_DEF(agitstart,""),	// <Agit>
	BUILDIN_DEF(agitend,""),
	BUILDIN_DEF(agitcheck,""),   // <Agitcheck>
	BUILDIN_DEF(flagemblem,"i"),	// Flag Emblem
	BUILDIN_DEF(getcastlename,"s"),
	BUILDIN_DEF(getcastledata,"si"),
	BUILDIN_DEF(setcastledata,"sii"),
	BUILDIN_DEF(requestguildinfo,"i?"),
	BUILDIN_DEF(getequipcardcnt,"i"),
	BUILDIN_DEF(successremovecards,"i"),
	BUILDIN_DEF(failedremovecards,"ii"),
	BUILDIN_DEF(marriage,"s"),
	BUILDIN_DEF2(wedding_effect,"wedding",""),
	BUILDIN_DEF(divorce,"?"),
	BUILDIN_DEF(ispartneron,"?"),
	BUILDIN_DEF(getpartnerid,"?"),
	BUILDIN_DEF(getchildid,"?"),
	BUILDIN_DEF(getmotherid,"?"),
	BUILDIN_DEF(getfatherid,"?"),
	BUILDIN_DEF(warppartner,"sii"),
	BUILDIN_DEF(getitemname,"v"),
	BUILDIN_DEF(getitemslots,"i"),
	BUILDIN_DEF(makepet,"i"),
	BUILDIN_DEF(getexp,"ii?"),
	BUILDIN_DEF(getinventorylist,"?"),
	BUILDIN_DEF(getskilllist,"?"),
	BUILDIN_DEF(clearitem,"?"),
	BUILDIN_DEF(classchange,"i??"),
	BUILDIN_DEF(misceffect,"i"),
	BUILDIN_DEF(playBGM,"s"),
	BUILDIN_DEF(playBGMall,"s?????"),
	BUILDIN_DEF(soundeffect,"si"),
	BUILDIN_DEF(soundeffectall,"si?????"),	// SoundEffectAll [Codemaster]
	BUILDIN_DEF(strmobinfo,"ii"),	// display mob data [Valaris]
	BUILDIN_DEF(guardian,"siisi??"),	// summon guardians
	BUILDIN_DEF(guardianinfo,"sii"),	// display guardian data [Valaris]
	BUILDIN_DEF(petskillbonus,"iiii"), // [Valaris]
	BUILDIN_DEF(petrecovery,"ii"), // [Valaris]
	BUILDIN_DEF(petloot,"i"), // [Valaris]
	BUILDIN_DEF(petskillattack,"viii"), // [Skotlex]
	BUILDIN_DEF(petskillattack2,"viiii"), // [Valaris]
	BUILDIN_DEF(petskillsupport,"viiii"), // [Skotlex]
	BUILDIN_DEF(skilleffect,"vi"), // skill effect [Celest]
	BUILDIN_DEF(npcskilleffect,"viii"), // npc skill effect [Valaris]
	BUILDIN_DEF(specialeffect,"i??"), // npc skill effect [Valaris]
	BUILDIN_DEF(specialeffect2,"i??"), // skill effect on players[Valaris]
	BUILDIN_DEF(nude,"?"), // nude command [Valaris]
	BUILDIN_DEF(mapwarp,"ssii??"),		// Added by RoVeRT
	BUILDIN_DEF(atcommand,"s"), // [MouseJstr]
	BUILDIN_DEF2(atcommand,"charcommand","s"), // [MouseJstr]
	BUILDIN_DEF(movenpc,"sii?"), // [MouseJstr]
	BUILDIN_DEF(message,"ss"), // [MouseJstr]
	BUILDIN_DEF(npctalk,"s??"), // [Valaris]
	BUILDIN_DEF(chatmes,"s?"), // [Jey]
	BUILDIN_DEF(mobcount,"ss"),
	BUILDIN_DEF(getlook,"i?"),
	BUILDIN_DEF(getsavepoint,"i?"),
	BUILDIN_DEF(npcspeed,"i"), // [Valaris]
	BUILDIN_DEF(npcwalkto,"ii"), // [Valaris]
	BUILDIN_DEF(npcstop,""), // [Valaris]
	BUILDIN_DEF(getmapxy,"rrri?"),	//by Lorky [Lupus]
	BUILDIN_DEF(mapid2name,"i"),
	BUILDIN_DEF(checkoption1,"i?"),
	BUILDIN_DEF(checkoption2,"i?"),
	BUILDIN_DEF(guildgetexp,"i"),
	BUILDIN_DEF(guildchangegm,"is"),
	BUILDIN_DEF(logmes,"s"), //this command actls as MES but rints info into LOG file either SQL/TXT [Lupus]
	BUILDIN_DEF(summon,"si??"), // summons a slave monster [Celest]
	BUILDIN_DEF(isnight,""), // check whether it is night time [Celest]
	BUILDIN_DEF(isday,""), // check whether it is day time [Celest]
	BUILDIN_DEF(isequipped,"i*"), // check whether another item/card has been equipped [Celest]
	BUILDIN_DEF(isequippedcnt,"i*"), // check how many items/cards are being equipped [Celest]
	BUILDIN_DEF(cardscnt,"i*"), // check how many items/cards are being equipped in the same arm [Lupus]
	BUILDIN_DEF(getrefine,""), // returns the refined number of the current item, or an item with index specified [celest]
	BUILDIN_DEF(night,""), // sets the server to night time
	BUILDIN_DEF(day,""), // sets the server to day time
#ifdef PCRE_SUPPORT
	BUILDIN_DEF(defpattern,"iss"), // Define pattern to listen for [MouseJstr]
	BUILDIN_DEF(activatepset,"i"), // Activate a pattern set [MouseJstr]
	BUILDIN_DEF(deactivatepset,"i"), // Deactive a pattern set [MouseJstr]
	BUILDIN_DEF(deletepset,"i"), // Delete a pattern set [MouseJstr]
#endif
	BUILDIN_DEF(preg_match,"ss?"),
	BUILDIN_DEF(dispbottom,"s??"), //added from jA [Lupus]
	BUILDIN_DEF(recovery,"i???"),
	BUILDIN_DEF(getpetinfo,"i?"),
	BUILDIN_DEF(gethominfo,"i?"),
	BUILDIN_DEF(getmercinfo,"i?"),
	BUILDIN_DEF(checkequipedcard,"i"),
	BUILDIN_DEF(jump_zero,"il"), //for future jA script compatibility
	BUILDIN_DEF(globalmes,"s?"), //end jA addition
	BUILDIN_DEF(unequip,"i?"), // unequip command [Spectre]
	BUILDIN_DEF(getstrlen,"s"), //strlen [Valaris]
	BUILDIN_DEF(charisalpha,"si"), //isalpha [Valaris]
	BUILDIN_DEF(charat,"si"),
	BUILDIN_DEF(setchar,"ssi"),
	BUILDIN_DEF(insertchar,"ssi"),
	BUILDIN_DEF(delchar,"si"),
	BUILDIN_DEF(strtoupper,"s"),
	BUILDIN_DEF(strtolower,"s"),
	BUILDIN_DEF(charisupper, "si"),
	BUILDIN_DEF(charislower, "si"),
	BUILDIN_DEF(substr,"sii"),
	BUILDIN_DEF(explode, "rss"),
	BUILDIN_DEF(implode, "r?"),
	BUILDIN_DEF(sprintf,"s*"),  // [Mirei]
	BUILDIN_DEF(sscanf,"ss*"),  // [Mirei]
	BUILDIN_DEF(strpos,"ss?"),
	BUILDIN_DEF(replacestr,"sss??"),
	BUILDIN_DEF(countstr,"ss?"),
	BUILDIN_DEF(setnpcdisplay,"sv??"),
	BUILDIN_DEF(compare,"ss"), // Lordalfa - To bring strstr to scripting Engine.
	BUILDIN_DEF(strcmp,"ss"),
	BUILDIN_DEF(getiteminfo,"ii"), //[Lupus] returns Items Buy / sell Price, etc info
	BUILDIN_DEF(setiteminfo,"iii"), //[Lupus] set Items Buy / sell Price, etc info
	BUILDIN_DEF(getequipcardid,"ii"), //[Lupus] returns CARD ID or other info from CARD slot N of equipped item
	// [zBuffer] List of mathematics commands --->
	BUILDIN_DEF(sqrt,"i"),
	BUILDIN_DEF(pow,"ii"),
	BUILDIN_DEF(distance,"iiii"),
	BUILDIN_DEF2(minmax,"min", "*"),
	BUILDIN_DEF2(minmax,"minimum", "*"),
	BUILDIN_DEF2(minmax,"max", "*"),
	BUILDIN_DEF2(minmax,"maximum", "*"),
	// <--- [zBuffer] List of mathematics commands
	BUILDIN_DEF(md5,"s"),
	// [zBuffer] List of dynamic var commands --->
	BUILDIN_DEF(getd,"s"),
	BUILDIN_DEF(setd,"sv?"),
	BUILDIN_DEF(callshop,"s?"), // [Skotlex]
	BUILDIN_DEF(npcshopitem,"sii*"), // [Lance]
	BUILDIN_DEF(npcshopadditem,"sii*"),
	BUILDIN_DEF(npcshopdelitem,"si*"),
	BUILDIN_DEF(npcshopattach,"s?"),
	BUILDIN_DEF(equip,"i?"),
	BUILDIN_DEF(autoequip,"ii"),
	BUILDIN_DEF(setbattleflag,"si?"),
	BUILDIN_DEF(getbattleflag,"s"),
	BUILDIN_DEF(setitemscript,"is?"), //Set NEW item bonus script. Lupus
	BUILDIN_DEF(disguise,"i?"), //disguise player. Lupus
	BUILDIN_DEF(undisguise,"?"), //undisguise player. Lupus
	BUILDIN_DEF(getmonsterinfo,"ii"), //Lupus
	BUILDIN_DEF(addmonsterdrop,"vii"), //Akinari [Lupus]
	BUILDIN_DEF(delmonsterdrop,"vi"), //Akinari [Lupus]
	BUILDIN_DEF(axtoi,"s"),
	BUILDIN_DEF(query_sql,"s*"),
	BUILDIN_DEF(query_logsql,"s*"),
	BUILDIN_DEF(escape_sql,"v"),
	BUILDIN_DEF(atoi,"s"),
	BUILDIN_DEF(strtol,"si"),
	// [zBuffer] List of player cont commands --->
	BUILDIN_DEF(rid2name,"i"),
	BUILDIN_DEF(pcfollow,"ii"),
	BUILDIN_DEF(pcstopfollow,"i"),
	BUILDIN_DEF(pcblockmove,"ii"),
	BUILDIN_DEF2(pcblockmove,"unitblockmove","ii"),
	BUILDIN_DEF(pcblockskill,"ii"),
	BUILDIN_DEF2(pcblockskill,"unitblockskill","ii"),
	// <--- [zBuffer] List of player cont commands
	// [zBuffer] List of unit control commands --->
	BUILDIN_DEF(unitexists,"i"),
	BUILDIN_DEF(getunittype,"i"),
	BUILDIN_DEF(getunitname,"i"),
	BUILDIN_DEF(setunitname,"is"),
	BUILDIN_DEF(getunitdata,"i*"),
	BUILDIN_DEF(setunitdata,"iii"),
	BUILDIN_DEF(unitwalk,"iii?"),
	BUILDIN_DEF2(unitwalk,"unitwalkto","ii?"),
	BUILDIN_DEF(unitkill,"i"),
	BUILDIN_DEF(unitwarp,"isii"),
	BUILDIN_DEF(unitattack,"iv?"),
	BUILDIN_DEF(unitstopattack,"i"),
	BUILDIN_DEF(unitstopwalk,"i?"),
	BUILDIN_DEF(unittalk,"is?"),
	BUILDIN_DEF_DEPRECATED(unitemote,"ii","20170811"),
	BUILDIN_DEF(unitskilluseid,"ivi??"), // originally by Qamera [Celest]
	BUILDIN_DEF(unitskillusepos,"iviii?"), // [Celest]
// <--- [zBuffer] List of unit control commands
	BUILDIN_DEF(sleep,"i"),
	BUILDIN_DEF(sleep2,"i"),
	BUILDIN_DEF(awake,"s"),
	BUILDIN_DEF(getvariableofnpc,"rs"),
	BUILDIN_DEF(warpportal,"iisii"),
	BUILDIN_DEF2(homunculus_evolution,"homevolution",""),	//[orn]
	BUILDIN_DEF2(homunculus_mutate,"hommutate","?"),
	BUILDIN_DEF(morphembryo,""),
	BUILDIN_DEF2(homunculus_shuffle,"homshuffle",""),	//[Zephyrus]
	BUILDIN_DEF(checkhomcall,""),
	BUILDIN_DEF(eaclass,"??"),	//[Skotlex]
	BUILDIN_DEF(roclass,"i?"),	//[Skotlex]
	BUILDIN_DEF(checkvending,"?"),
	BUILDIN_DEF(checkchatting,"?"),
	BUILDIN_DEF(checkidle,"?"),
	BUILDIN_DEF(openmail,"?"),
	BUILDIN_DEF(openauction,"?"),
	BUILDIN_DEF(checkcell,"siii"),
	BUILDIN_DEF(setcell,"siiiiii"),
	BUILDIN_DEF(getfreecell,"srr?????"),
	BUILDIN_DEF(setwall,"siiiiis"),
	BUILDIN_DEF(delwall,"s"),
	BUILDIN_DEF(checkwall,"s"),
	BUILDIN_DEF(searchitem,"rs"),
	BUILDIN_DEF(mercenary_create,"ii"),
	BUILDIN_DEF(mercenary_heal,"ii"),
	BUILDIN_DEF(mercenary_sc_start,"iii"),
	BUILDIN_DEF(mercenary_get_calls,"i"),
	BUILDIN_DEF(mercenary_get_faith,"i"),
	BUILDIN_DEF(mercenary_set_calls,"ii"),
	BUILDIN_DEF(mercenary_set_faith,"ii"),
	BUILDIN_DEF(readbook,"ii"),
	BUILDIN_DEF(setfont,"i"),
	BUILDIN_DEF(areamobuseskill,"siiiiviiiii"),
	BUILDIN_DEF(progressbar,"si"),
	BUILDIN_DEF(progressbar_npc, "si?"),
	BUILDIN_DEF(pushpc,"ii"),
	BUILDIN_DEF(buyingstore,"i"),
	BUILDIN_DEF(searchstores,"ii"),
	BUILDIN_DEF(showdigit,"i?"),
	// WoE SE
	BUILDIN_DEF(agitstart2,""),
	BUILDIN_DEF(agitend2,""),
	BUILDIN_DEF(agitcheck2,""),
	// BattleGround
	BUILDIN_DEF(waitingroom2bg,"sii???"),
	BUILDIN_DEF(waitingroom2bg_single,"i????"),
	BUILDIN_DEF(bg_team_setxy,"iii"),
	BUILDIN_DEF(bg_warp,"isii"),
	BUILDIN_DEF(bg_monster,"isiisi?"),
	BUILDIN_DEF(bg_monster_set_team,"ii"),
	BUILDIN_DEF(bg_leave,"?"),
	BUILDIN_DEF(bg_destroy,"i"),
	BUILDIN_DEF(areapercentheal,"siiiiii"),
	BUILDIN_DEF(bg_get_data,"ii"),
	BUILDIN_DEF(bg_getareausers,"isiiii"),
	BUILDIN_DEF(bg_updatescore,"sii"),
	BUILDIN_DEF(bg_join,"i????"),
	BUILDIN_DEF(bg_create,"sii??"),

	// Instancing
	BUILDIN_DEF(instance_create,"s??"),
	BUILDIN_DEF(instance_destroy,"?"),
	BUILDIN_DEF(instance_id,""),
	BUILDIN_DEF(instance_enter,"s????"),
	BUILDIN_DEF(instance_npcname,"s?"),
	BUILDIN_DEF(instance_mapname,"s?"),
	BUILDIN_DEF(instance_warpall,"sii?"),
	BUILDIN_DEF(instance_announce,"isi?????"),
	BUILDIN_DEF(instance_check_party,"i???"),
	BUILDIN_DEF(instance_check_guild,"i???"),
	BUILDIN_DEF(instance_check_clan,"i???"),
	BUILDIN_DEF(instance_info,"si?"),
	/**
	 * 3rd-related
	 **/
	BUILDIN_DEF(makerune,"i?"),
	BUILDIN_DEF(checkdragon,"?"),//[Ind]
	BUILDIN_DEF(setdragon,"??"),//[Ind]
	BUILDIN_DEF(ismounting,"?"),//[Ind]
	BUILDIN_DEF(setmounting,"?"),//[Ind]
	BUILDIN_DEF(checkre,"i"),
	/**
	 * rAthena and beyond!
	 **/
	BUILDIN_DEF(getargcount,""),
	BUILDIN_DEF(getcharip,"?"),
	BUILDIN_DEF(is_function,"s"),
	BUILDIN_DEF(get_revision,""),
	BUILDIN_DEF(get_githash,""),
	BUILDIN_DEF(freeloop,"?"),
	BUILDIN_DEF(getrandgroupitem,"i????"),
	BUILDIN_DEF(cleanmap,"s"),
	BUILDIN_DEF2(cleanmap,"cleanarea","siiii"),
	BUILDIN_DEF(npcskill,"viii"),
	BUILDIN_DEF(consumeitem,"v?"),
	BUILDIN_DEF(delequip,"i?"),
	BUILDIN_DEF(breakequip,"i?"),
	BUILDIN_DEF(sit,"?"),
	BUILDIN_DEF(stand,"?"),
	//@commands (script based)
	BUILDIN_DEF(bindatcmd, "ss??"),
	BUILDIN_DEF(unbindatcmd, "s"),
	BUILDIN_DEF(useatcmd, "s"),

	//Quest Log System [Inkfish]
	BUILDIN_DEF(questinfo, "ii??"),
	BUILDIN_DEF(setquest, "i?"),
	BUILDIN_DEF(erasequest, "i?"),
	BUILDIN_DEF(completequest, "i?"),
	BUILDIN_DEF(checkquest, "i??"),
	BUILDIN_DEF(isbegin_quest,"i?"),
	BUILDIN_DEF(changequest, "ii?"),
	BUILDIN_DEF(showevent, "i??"),

	//Bound items [Xantara] & [Akinari]
	BUILDIN_DEF2(getitem,"getitembound","vii?"),
	BUILDIN_DEF2(getitem2,"getitembound2","viiiiiiiii?"),
	BUILDIN_DEF(countbound, "??"),

	// Party related
	BUILDIN_DEF(party_create,"s???"),
	BUILDIN_DEF(party_addmember,"ii"),
	BUILDIN_DEF(party_delmember,"??"),
	BUILDIN_DEF(party_changeleader,"ii"),
	BUILDIN_DEF(party_changeoption,"iii"),
	BUILDIN_DEF(party_destroy,"i"),

	// Clan system
	BUILDIN_DEF(clan_join,"i?"),
	BUILDIN_DEF(clan_leave,"?"),

	BUILDIN_DEF2(montransform, "transform", "vi?????"), // Monster Transform [malufett/Hercules]
	BUILDIN_DEF2(montransform, "active_transform", "vi?????"),
	BUILDIN_DEF(vip_status,"i?"),
	BUILDIN_DEF(vip_time,"i?"),
	BUILDIN_DEF(bonus_script,"si????"),
	BUILDIN_DEF(bonus_script_clear,"??"),
	BUILDIN_DEF(getgroupitem,"i??"),
	BUILDIN_DEF(enable_command,""),
	BUILDIN_DEF(disable_command,""),
	BUILDIN_DEF(getguildmember,"i??"),
	BUILDIN_DEF(addspiritball,"ii?"),
	BUILDIN_DEF(delspiritball,"i?"),
	BUILDIN_DEF(countspiritball,"?"),
	BUILDIN_DEF(mergeitem,"?"),
	BUILDIN_DEF(mergeitem2,"??"),
	BUILDIN_DEF(npcshopupdate,"sii?"),
	BUILDIN_DEF(getattachedrid,""),
	BUILDIN_DEF(getvar,"vi"),
	BUILDIN_DEF(showscript,"s??"),
	BUILDIN_DEF(ignoretimeout,"i?"),
	BUILDIN_DEF(geteleminfo,"i?"),
	BUILDIN_DEF(setquestinfo_level,"iii"),
	BUILDIN_DEF(setquestinfo_req,"iii*"),
	BUILDIN_DEF(setquestinfo_job,"ii*"),
	BUILDIN_DEF(opendressroom,"i?"),
	BUILDIN_DEF(navigateto,"s???????"),
	BUILDIN_DEF(getguildalliance,"ii"),
	BUILDIN_DEF(adopt,"vv"),
	BUILDIN_DEF(getexp2,"ii?"),
	BUILDIN_DEF(recalculatestat,""),
	BUILDIN_DEF(hateffect,"ii"),
	BUILDIN_DEF(getrandomoptinfo, "i"),
	BUILDIN_DEF(getequiprandomoption, "iii?"),
	BUILDIN_DEF(setrandomoption,"iiiii?"),
	BUILDIN_DEF(needed_status_point,"ii?"),
	BUILDIN_DEF(jobcanentermap,"s?"),
	BUILDIN_DEF(openstorage2,"ii?"),
	BUILDIN_DEF(unloadnpc, "s"),

	// WoE TE
	BUILDIN_DEF(agitstart3,""),
	BUILDIN_DEF(agitend3,""),
	BUILDIN_DEF(agitcheck3,""),
	BUILDIN_DEF(gvgon3,"s"),
	BUILDIN_DEF(gvgoff3,"s"),

	// Channel System
	BUILDIN_DEF(channel_create,"ss?????"),
	BUILDIN_DEF(channel_setopt,"sii"),
	BUILDIN_DEF(channel_setcolor,"si"),
	BUILDIN_DEF(channel_setpass,"ss"),
	BUILDIN_DEF(channel_setgroup,"si*"),
	BUILDIN_DEF2(channel_setgroup,"channel_setgroup2","sr"),
	BUILDIN_DEF(channel_chat,"ss?"),
	BUILDIN_DEF(channel_ban,"si"),
	BUILDIN_DEF(channel_unban,"si"),
	BUILDIN_DEF(channel_kick,"sv"),
	BUILDIN_DEF(channel_delete,"s"),

	// Item Random Option Extension [Cydh]
	BUILDIN_DEF2(getitem2,"getitem3","viiiiiiiirrr?"),
	BUILDIN_DEF2(getitem2,"getitembound3","viiiiiiiiirrr?"),
	BUILDIN_DEF2(rentitem2,"rentitem3","viiiiiiiirrr?"),
	BUILDIN_DEF2(makeitem2,"makeitem3","visiiiiiiiiirrr"),
	BUILDIN_DEF2(delitem2,"delitem3","viiiiiiiirrr?"),
	BUILDIN_DEF2(countitem,"countitem3","viiiiiiirrr?"),

	// Achievement System
	BUILDIN_DEF(achievementinfo,"ii?"),
	BUILDIN_DEF(achievementadd,"i?"),
	BUILDIN_DEF(achievementremove,"i?"),
	BUILDIN_DEF(achievementcomplete,"i?"),
	BUILDIN_DEF(achievementexists,"i?"),
	BUILDIN_DEF(achievementupdate,"iii?"),


	BUILDIN_DEF(getequiprefinecost,"iii?"),
	BUILDIN_DEF2(round, "round", "i"),
	BUILDIN_DEF2(round, "ceil", "i"),
	BUILDIN_DEF2(round, "floor", "i"),
	BUILDIN_DEF(getequiptradability, "i?"),
	BUILDIN_DEF(mail, "isss*"),
	BUILDIN_DEF(open_roulette,"?"),
	BUILDIN_DEF(identifyall,"??"),
#include "../custom/script_def.inc"

	{NULL,NULL,NULL},
};
