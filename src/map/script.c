// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

//#define DEBUG_DISP
//#define DEBUG_DISASM
//#define DEBUG_RUN
//#define DEBUG_HASH

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/lock.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/utils.h"

#include "map.h"
#include "path.h"
#include "clif.h"
#include "chrif.h"
#include "itemdb.h"
#include "pc.h"
#include "status.h"
#include "storage.h"
#include "mob.h"
#include "npc.h"
#include "pet.h"
#include "mapreg.h"
#include "mercenary.h"	//[orn]
#include "intif.h"
#include "skill.h"
#include "status.h"
#include "chat.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"
#include "charcommand.h"
#include "log.h"
#include "unit.h"
#include "pet.h"
#include "mail.h"
#include "script.h"
#include "quest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef WIN32
	#include <sys/time.h>
#endif
#include <time.h>
#include <setjmp.h>
#include <errno.h>


///////////////////////////////////////////////////////////////////////////////
//## TODO possible enhancements: [FlavioJS]
// - 'callfunc' supporting labels in the current npc "::LabelName"
// - 'callfunc' supporting labels in other npcs "NpcName::LabelName"
// - 'function FuncName;' function declarations reverting to global functions 
//   if local label isn't found
// - join callfunc and callsub's functionality
// - use script_op2name in the DEBUG_DISASM block
// - remove dynamic allocation in add_word()
// - remove GETVALUE / SETVALUE
// - clean up the set_reg / set_val / setd_sub mess



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
#define script_pushconststr(st,val) push_str((st)->stack, C_CONSTSTR, (val))
/// Pushes a nil into the stack
#define script_pushnil(st) push_val((st)->stack, C_NOP, 0)
/// Pushes a copy of the data in the target index
#define script_pushcopy(st,i) push_copy((st)->stack, (st)->start + (i))

#define script_isstring(st,i) data_isstring(script_getdata(st,i))
#define script_isint(st,i) data_isint(script_getdata(st,i))

#define script_getnum(st,val) conv_num(st, script_getdata(st,val))
#define script_getstr(st,val) conv_str(st, script_getdata(st,val))
#define script_getref(st,val) ( script_getdata(st,val)->ref )

// Note: "top" functions/defines use indexes relative to the top of the stack
//       -1 is the index of the data at the top

/// Returns the script_data at the target index relative to the top of the stack
#define script_getdatatop(st,i) ( &((st)->stack->stack_data[(st)->stack->sp + (i)]) )
/// Pushes a copy of the data in the target index relative to the top of the stack
#define script_pushcopytop(st,i) push_copy((st)->stack, (st)->stack->sp + (i))
/// Removes the range of values [start,end[ relative to the top of the stack
#define script_removetop(st,start,end) ( pop_stack((st)->stack, ((st)->stack->sp + (start)), (st)->stack->sp + (end)) )

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
#define reference_getid(data) ( (int32)(reference_getuid(data) & 0x00ffffff) )
/// Returns the array index of the reference
#define reference_getindex(data) ( (int32)(((uint32)(reference_getuid(data) & 0xff000000)) >> 24) )
/// Returns the name of the reference
#define reference_getname(data) ( str_buf + str_data[reference_getid(data)].str )
/// Returns the linked list of uid-value pairs of the reference (can be NULL)
#define reference_getref(data) ( (data)->ref )
/// Returns the value of the constant
#define reference_getconstant(data) ( str_data[reference_getid(data)].val )
/// Returns the type of param
#define reference_getparamtype(data) ( str_data[reference_getid(data)].val )

/// Composes the uid of a reference from the id and the index
#define reference_uid(id,idx) ( (int32)((((uint32)(id)) & 0x00ffffff) | (((uint32)(idx)) << 24)) )

#define not_server_variable(prefix) ( (prefix) != '$' && (prefix) != '.')
#define not_array_variable(prefix) ( (prefix) != '$' && (prefix) != '@' && (prefix) != '.' )
#define is_string_variable(name) ( (name)[strlen(name) - 1] == '$' )

#define FETCH(n, t) \
		if( script_hasdata(st,n) ) \
			(t)=script_getnum(st,n);

#define SCRIPT_BLOCK_SIZE 512
enum { LABEL_NEXTLINE=1,LABEL_START };

/// temporary buffer for passing around compiled bytecode
/// @see add_scriptb, set_label, parse_script
static unsigned char* script_buf = NULL;
static int script_pos = 0, script_size = 0;

#define GETVALUE(buf,i)		((int)MakeDWord(MakeWord((buf)[i],(buf)[i+1]),MakeWord((buf)[i+2],0)))
#define SETVALUE(buf,i,n)	((buf)[i]=GetByte(n,0),(buf)[i+1]=GetByte(n,1),(buf)[i+2]=GetByte(n,2))

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


static DBMap* scriptlabel_db=NULL; // const char* label_name -> int script_pos
static DBMap* userfunc_db=NULL; // const char* func_name -> struct script_code*
static int parse_options=0;
DBMap* script_get_label_db(){ return scriptlabel_db; }
DBMap* script_get_userfunc_db(){ return userfunc_db; }

struct Script_Config script_config = {
	1, 65535, 2048, //warn_func_mismatch_paramnum/check_cmdcount/check_gotocount
	0, INT_MAX, // input_min_value/input_max_value
	"OnPCDieEvent", //die_event_name
	"OnPCKillEvent", //kill_pc_event_name
	"OnNPCKillEvent", //kill_mob_event_name
	"OnPCLoginEvent", //login_event_name
	"OnPCLogoutEvent", //logout_event_name
	"OnPCLoadMapEvent", //loadmap_event_name
	"OnPCBaseLvUpEvent", //baselvup_event_name
	"OnPCJobLvUpEvent" //joblvup_event_name
};

static jmp_buf     error_jump;
static char*       error_msg;
static const char* error_pos;
static int         error_report; // if the error should produce output

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

#define ARGLIST_UNDEFINED 0
#define ARGLIST_NO_PAREN  1
#define ARGLIST_PAREN     2
static struct {
	struct {
		enum curly_type type;
		int index;
		int count;
		int flag;
		struct linkdb_node *case_label;
	} curly[256];		// 右カッコの情報
	int curly_count;	// 右カッコの数
	int index;			// スクリプト内で使用した構文の数
} syntax;

const char* parse_curly_close(const char* p);
const char* parse_syntax_close(const char* p);
const char* parse_syntax_close_sub(const char* p,int* flag);
const char* parse_syntax(const char* p);
static int parse_syntax_for_flag = 0;

extern int current_equip_item_index; //for New CARDS Scripts. It contains Inventory Index of the EQUIP_SCRIPT caller item. [Lupus]
int potion_flag=0; //For use on Alchemist improved potions/Potion Pitcher. [Skotlex]
int potion_hp=0, potion_per_hp=0, potion_sp=0, potion_per_sp=0;
int potion_target=0;


c_op get_com(unsigned char *script,int *pos);
int get_num(unsigned char *script,int *pos);

typedef struct script_function {
	int (*func)(struct script_state *st);
	const char *name;
	const char *arg;
} script_function;

extern script_function buildin_func[];

static struct linkdb_node *sleep_db;

/*==========================================
 * ローカルプロトタイプ宣言 (必要な物のみ)
 *------------------------------------------*/
const char* parse_subexpr(const char* p,int limit);
void push_val(struct script_stack *stack,int type,int val);
int run_func(struct script_state *st);

enum {
	MF_NOMEMO,	//0
	MF_NOTELEPORT,
	MF_NOSAVE,
	MF_NOBRANCH,
	MF_NOPENALTY,
	MF_NOZENYPENALTY,
	MF_PVP,
	MF_PVP_NOPARTY,
	MF_PVP_NOGUILD,
	MF_GVG,
	MF_GVG_NOPARTY,	//10
	MF_NOTRADE,
	MF_NOSKILL,
	MF_NOWARP,
	MF_PARTYLOCK,
	MF_NOICEWALL,
	MF_SNOW,
	MF_FOG,
	MF_SAKURA,
	MF_LEAVES,
	MF_RAIN,	//20
	MF_INDOORS,
	MF_NOGO,
	MF_CLOUDS,
	MF_CLOUDS2,
	MF_FIREWORKS,
	MF_GVG_CASTLE,
	MF_GVG_DUNGEON,
	MF_NIGHTENABLED,
	MF_NOBASEEXP,
	MF_NOJOBEXP,	//30
	MF_NOMOBLOOT,
	MF_NOMVPLOOT,
	MF_NORETURN,
	MF_NOWARPTO,
	MF_NIGHTMAREDROP,
	MF_RESTRICTED,
	MF_NOCOMMAND,
	MF_NODROP,
	MF_JEXP,
	MF_BEXP,	//40
	MF_NOVENDING,
	MF_LOADEVENT,
	MF_NOCHAT,
	MF_NOEXPPENALTY,
	MF_GUILDLOCK
};

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

	default:
		ShowDebug("script_op2name: unexpected op=%d\n", op);
		return "???";
	}
#undef RETURN_OP_NAME
}

/// Reports on the console the src of a script error.
static void script_reportsrc(struct script_state *st)
{
	struct block_list* bl;

	if( st->oid == 0 )
		return; //Can't report source.

	bl = map_id2bl(st->oid);
	if( bl == NULL )
		return;

	switch( bl->type )
	{
	case BL_NPC:
		if( bl->m >= 0 )
			ShowDebug("Source (NPC): %s at %s (%d,%d)\n", ((struct npc_data *)bl)->name, map[bl->m].name, bl->x, bl->y);
		else
			ShowDebug("Source (NPC): %s (invisible/not on a map)\n", ((struct npc_data *)bl)->name);
		break;
	default:
		if( bl->m >= 0 )
			ShowDebug("Source (Non-NPC type %d): name %s at %s (%d,%d)\n", bl->type, status_get_name(bl), map[bl->m].name, bl->x, bl->y);
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
	switch( data->type )
	{
	case C_NOP:// no value
		ShowDebug("Data: nothing (nil)\n");
		break;
	case C_INT:// number
		ShowDebug("Data: number value=%d\n", data->u.num);
		break;
	case C_STR:
	case C_CONSTSTR:// string
		ShowDebug("Data: string value=\"%s\"\n", data->u.str);
		break;
	case C_NAME:// reference
		if( reference_tovariable(data) )
		{// variable
			const char* name = reference_getname(data);
			if( not_array_variable(*name) )
				ShowDebug("Data: variable name='%s'\n", name);
			else
				ShowDebug("Data: variable name='%s' index=%d\n", name, reference_getindex(data));
		}
		else if( reference_toconstant(data) )
		{// constant
			ShowDebug("Data: constant name='%s' value=%d\n", reference_getname(data), reference_getconstant(data));
		}
		else if( reference_toparam(data) )
		{// param
			ShowDebug("Data: param name='%s' type=%d\n", reference_getname(data), reference_getparamtype(data));
		}
		else
		{// ???
			ShowDebug("Data: reference name='%s' type=%s\n", reference_getname(data), script_op2name(data->type));
			ShowDebug("Please report this!!! - str_data.type=%s\n", script_op2name(str_data[reference_getid(data)].type));
		}
		break;
	case C_POS:// label
		ShowDebug("Data: label pos=%d\n", data->u.num);
		break;
	default:
		ShowDebug("Data: %s\n", script_op2name(data->type));
		break;
	}
}

/*==========================================
 * エラーメッセージ出力
 *------------------------------------------*/
static void disp_error_message2(const char *mes,const char *pos,int report)
{
	error_msg = aStrdup(mes);
	error_pos = pos;
	error_report = report;
	longjmp( error_jump, 1 );
}
#define disp_error_message(mes,pos) disp_error_message2(mes,pos,1)

/// Checks event parameter validity
static void check_event(struct script_state *st, const char *evt)
{
	if( evt != NULL && *evt != '\0' && !stristr(evt,"::On") ){
		ShowError("NPC event parameter deprecated! Please use 'NPCNAME::OnEVENT' instead of '%s'.\n",evt);
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
	int i, h;
	int len;

	h = calc_hash(p);

	if( str_hash[h] == 0 )
	{// empty bucket, add new node here
		str_hash[h] = str_num;
	}
	else
	{// scan for end of list, or occurence of identical string
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
// 最大16Mまで
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
		// ラベルの可能性があるのでbackpatch用データ埋め込み
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
 * ラベルを解決する
 *------------------------------------------*/
void set_label(int l,int pos, const char* script_pos)
{
	int i,next;

	if(str_data[l].type==C_INT || str_data[l].type==C_PARAM)
	{	//Prevent overwriting constants values and parameters [Skotlex]
		disp_error_message("set_label: invalid label name",script_pos);
		return;
	}
	if(str_data[l].label!=-1){
		disp_error_message("set_label: dup label ",script_pos);
		return;
	}
	str_data[l].type=(str_data[l].type == C_USERFUNC ? C_USERFUNC_POS : C_POS);
	str_data[l].label=pos;
	for(i=str_data[l].backpatch;i>=0 && i!=0x00ffffff;){
		next=GETVALUE(script_buf,i);
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
				if( *p == '\0' )
					return p;//disp_error_message("script:skip_space: end of file while parsing block comment. expected "CL_BOLD"*/"CL_NORM, p);
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
/// A word consists of undercores and/or alfanumeric characters,
/// and valid variable prefixes/postfixes.
static
const char* skip_word(const char* p)
{
	// prefix
	switch( *p )
	{
	case '@':// temporary char variable
		++p; break;
	case '#':// account variable
		p += ( p[1] == '#' ? 2 : 1 ); break;
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
static
int add_word(const char* p)
{
	char* word;
	int len;
	int i;

	// Check for a word
	len = skip_word(p) - p;
	if( len == 0 )
		disp_error_message("script:add_word: invalid word. A word consists of undercores and/or alfanumeric characters, and valid variable prefixes/postfixes.", p);

	// Duplicate the word
	CREATE(word, char, len+1);
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
const char* parse_callfunc(const char* p, int require_paren)
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
	} else if( str_data[func].type == C_USERFUNC || str_data[func].type == C_USERFUNC_POS ){
		// script defined function
		int callsub = search_str("callsub");
		add_scriptl(callsub);
		add_scriptc(C_ARG);
		add_scriptl(func);
		arg = buildin_func[str_data[callsub].val].arg;
		if( *arg == 0 )
			disp_error_message("parse_callfunc: callsub has no arguments, please review it's definition",p);
		if( *arg != '*' )
			++arg; // count func as argument
	} else
		disp_error_message("parse_line: expect command, missing function name or calling undeclared function",p);

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

/*==========================================
 * 項の解析
 *------------------------------------------*/
const char* parse_simpleexpr(const char *p)
{
	int i;
	p=skip_space(p);

	if(*p==';' || *p==',')
		disp_error_message("parse_simpleexpr: unexpected expr end",p);
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
			disp_error_message("parse_simpleexpr: unmatch ')'",p);
		++p;
	} else if(ISDIGIT(*p) || ((*p=='-' || *p=='+') && ISDIGIT(p[1]))){
		char *np;
		i=strtoul(p,&np,0);
		add_scripti(i);
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
		// label , register , function etc
		if(skip_word(p)==p)
			disp_error_message("parse_simpleexpr: unexpected character",p);

		l=add_word(p);
		if( str_data[l].type == C_FUNC || str_data[l].type == C_USERFUNC || str_data[l].type == C_USERFUNC_POS)
			return parse_callfunc(p,1);

		p=skip_word(p);
		if( *p == '[' ){
			// array(name[i] => getelementofarray(name,i) )
			add_scriptl(search_str("getelementofarray"));
			add_scriptc(C_ARG);
			add_scriptl(l);
			
			p=parse_subexpr(p+1,-1);
			p=skip_space(p);
			if( *p != ']' )
				disp_error_message("parse_simpleexpr: unmatch ']'",p);
			++p;
			add_scriptc(C_FUNC);
		}else
			add_scriptl(l);

	}

	return p;
}

/*==========================================
 * 式の解析
 *------------------------------------------*/
const char* parse_subexpr(const char* p,int limit)
{
	int op,opl,len;
	const char* tmpp;

	p=skip_space(p);

	if(*p=='-'){
		tmpp=skip_space(p+1);
		if(*tmpp==';' || *tmpp==','){
			add_scriptl(LABEL_NEXTLINE);
			p++;
			return p;
		}
	}
	tmpp=p;
	if((op=C_NEG,*p=='-') || (op=C_LNOT,*p=='!') || (op=C_NOT,*p=='~')){
		p=parse_subexpr(p+1,10);
		add_scriptc(op);
	} else
		p=parse_simpleexpr(p);
	p=skip_space(p);
	while((
			(op=C_OP3,opl=0,len=1,*p=='?') ||
			(op=C_ADD,opl=8,len=1,*p=='+') ||
			(op=C_SUB,opl=8,len=1,*p=='-') ||
			(op=C_MUL,opl=9,len=1,*p=='*') ||
			(op=C_DIV,opl=9,len=1,*p=='/') ||
			(op=C_MOD,opl=9,len=1,*p=='%') ||
			(op=C_LAND,opl=2,len=2,*p=='&' && p[1]=='&') ||
			(op=C_AND,opl=6,len=1,*p=='&') ||
			(op=C_LOR,opl=1,len=2,*p=='|' && p[1]=='|') ||
			(op=C_OR,opl=5,len=1,*p=='|') ||
			(op=C_XOR,opl=4,len=1,*p=='^') ||
			(op=C_EQ,opl=3,len=2,*p=='=' && p[1]=='=') ||
			(op=C_NE,opl=3,len=2,*p=='!' && p[1]=='=') ||
			(op=C_R_SHIFT,opl=7,len=2,*p=='>' && p[1]=='>') ||
			(op=C_GE,opl=3,len=2,*p=='>' && p[1]=='=') ||
			(op=C_GT,opl=3,len=1,*p=='>') ||
			(op=C_L_SHIFT,opl=7,len=2,*p=='<' && p[1]=='<') ||
			(op=C_LE,opl=3,len=2,*p=='<' && p[1]=='=') ||
			(op=C_LT,opl=3,len=1,*p=='<')) && opl>limit){
		p+=len;
		if(op == C_OP3) {
			p=parse_subexpr(p,-1);
			p=skip_space(p);
			if( *(p++) != ':')
				disp_error_message("parse_subexpr: need ':'", p-1);
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
 * 式の評価
 *------------------------------------------*/
const char* parse_expr(const char *p)
{
	switch(*p){
	case ')': case ';': case ':': case '[': case ']':
	case '}':
		disp_error_message("parse_expr: unexpected char",p);
	}
	p=parse_subexpr(p,-1);
	return p;
}

/*==========================================
 * 行の解析
 *------------------------------------------*/
const char* parse_line(const char* p)
{
	const char* p2;

	p=skip_space(p);
	if(*p==';') {
		// if(); for(); while(); のために閉じ判定
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

	// 構文関連の処理
	p2 = parse_syntax(p);
	if(p2 != NULL)
		return p2;

	p = parse_callfunc(p,0);
	p = skip_space(p);

	if(parse_syntax_for_flag) {
		if( *p != ')' )
			disp_error_message("parse_line: need ')'",p);
	} else {
		if( *p != ';' )
			disp_error_message("parse_line: need ';'",p);
	}

	// if, for , while の閉じ判定
	p = parse_syntax_close(p+1);

	return p;
}

// { ... } の閉じ処理
const char* parse_curly_close(const char* p)
{
	if(syntax.curly_count <= 0) {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_NULL) {
		syntax.curly_count--;
		// if, for , while の閉じ判定
		p = parse_syntax_close(p + 1);
		return p;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_SWITCH) {
		// switch() 閉じ判定
		int pos = syntax.curly_count-1;
		char label[256];
		int l;
		// 一時変数を消す
		sprintf(label,"set $@__SW%x_VAL,0;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// 無条件で終了ポインタに移動
		sprintf(label,"goto __SW%x_FIN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// 現在地のラベルを付ける
		sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
		l=add_str(label);
		set_label(l,script_pos, p);

		if(syntax.curly[pos].flag) {
			// default が存在する
			sprintf(label,"goto __SW%x_DEF;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;
		}

		// 終了ラベルを付ける
		sprintf(label,"__SW%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos, p);
		linkdb_final(&syntax.curly[pos].case_label);	// free the list of case label
		syntax.curly_count--;
		return p+1;
	} else {
		disp_error_message("parse_curly_close: unexpected string",p);
		return p + 1;
	}
}

// 構文関連の処理
//	 break, case, continue, default, do, for, function,
//	 if, switch, while をこの内部で処理します。
const char* parse_syntax(const char* p)
{
	const char *p2 = skip_word(p);

	switch(*p) {
	case 'B':
	case 'b':
		if(p2 - p == 5 && !strncasecmp(p,"break",5)) {
			// break の処理
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
				disp_error_message("parse_syntax: need ';'",p);
			// if, for , while の閉じ判定
			p = parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'c':
	case 'C':
		if(p2 - p == 4 && !strncasecmp(p,"case",4)) {
			// case の処理
			int pos = syntax.curly_count-1;
			if(pos < 0 || syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'case' ",p);
				return p+1;
			} else {
				char label[256];
				int  l,v;
				char *np;
				if(syntax.curly[pos].count != 1) {
					// FALLTHRU 用のジャンプ
					sprintf(label,"goto __SW%x_%xJ;",syntax.curly[pos].index,syntax.curly[pos].count);
					syntax.curly[syntax.curly_count++].type = TYPE_NULL;
					parse_line(label);
					syntax.curly_count--;

					// 現在地のラベルを付ける
					sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
					l=add_str(label);
					set_label(l,script_pos, p);
				}
				// switch 判定文
				p = skip_space(p2);
				if(p == p2) {
					disp_error_message("parse_syntax: expect space ' '",p);
				}
				// check whether case label is integer or not
				v = strtol(p,&np,0);
				if(np == p) { //Check for constants
					p2 = skip_word(p);
					v = p2-p; // length of word at p2
					memcpy(label,p,v);
					label[v]='\0';
					v = search_str(label);
					if (v < 0 || str_data[v].type != C_INT)
						disp_error_message("parse_syntax: 'case' label not integer",p);
					v = str_data[v].val;
					p = skip_word(p);
				} else { //Numeric value
					if((*p == '-' || *p == '+') && ISDIGIT(p[1]))	// pre-skip because '-' can not skip_word
						p++;
					p = skip_word(p);
					if(np != p)
						disp_error_message("parse_syntax: 'case' label not integer",np);
				}
				p = skip_space(p);
				if(*p != ':')
					disp_error_message("parse_syntax: expect ':'",p);
				sprintf(label,"if(%d != $@__SW%x_VAL) goto __SW%x_%x;",
					v,syntax.curly[pos].index,syntax.curly[pos].index,syntax.curly[pos].count+1);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				// ２回parse しないとダメ
				p2 = parse_line(label);
				parse_line(p2);
				syntax.curly_count--;
				if(syntax.curly[pos].count != 1) {
					// FALLTHRU 終了後のラベル
					sprintf(label,"__SW%x_%xJ",syntax.curly[pos].index,syntax.curly[pos].count);
					l=add_str(label);
					set_label(l,script_pos,p);
				}
				// check duplication of case label [Rayce]
				if(linkdb_search(&syntax.curly[pos].case_label, (void*)v) != NULL)
					disp_error_message("parse_syntax: dup 'case'",p);
				linkdb_insert(&syntax.curly[pos].case_label, (void*)v, (void*)1);

				sprintf(label,"set $@__SW%x_VAL,0;",syntax.curly[pos].index);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			
				parse_line(label);
				syntax.curly_count--;
				syntax.curly[pos].count++;
			}
			return p + 1;
		} else if(p2 - p == 8 && !strncasecmp(p,"continue",8)) {
			// continue の処理
			char label[256];
			int pos = syntax.curly_count - 1;
			while(pos >= 0) {
				if(syntax.curly[pos].type == TYPE_DO) {
					sprintf(label,"goto __DO%x_NXT;",syntax.curly[pos].index);
					syntax.curly[pos].flag = 1; // continue 用のリンク張るフラグ
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
				disp_error_message("parse_syntax: need ';'",p);
			// if, for , while の閉じ判定
			p = parse_syntax_close(p + 1);
			return p;
		}
		break;
	case 'd':
	case 'D':
		if(p2 - p == 7 && !strncasecmp(p,"default",7)) {
			// switch - default の処理
			int pos = syntax.curly_count-1;
			if(pos < 0 || syntax.curly[pos].type != TYPE_SWITCH) {
				disp_error_message("parse_syntax: unexpected 'default'",p);
			} else if(syntax.curly[pos].flag) {
				disp_error_message("parse_syntax: dup 'default'",p);
			} else {
				char label[256];
				int l;
				// 現在地のラベルを付ける
				p = skip_space(p2);
				if(*p != ':') {
					disp_error_message("parse_syntax: need ':'",p);
				}
				sprintf(label,"__SW%x_%x",syntax.curly[pos].index,syntax.curly[pos].count);
				l=add_str(label);
				set_label(l,script_pos,p);

				// 無条件で次のリンクに飛ばす
				sprintf(label,"goto __SW%x_%x;",syntax.curly[pos].index,syntax.curly[pos].count+1);
				syntax.curly[syntax.curly_count++].type = TYPE_NULL;
				parse_line(label);
				syntax.curly_count--;

				// default のラベルを付ける
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
			// 現在地のラベル形成する
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
				disp_error_message("parse_syntax: need '('",p);
			p++;

			// 初期化文を実行する
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			p=parse_line(p);
			syntax.curly_count--;

			// 条件判断開始のラベル形成する
			sprintf(label,"__FR%x_J",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			p=skip_space(p);
			if(*p == ';') {
				// for(;;) のパターンなので必ず真
				;
			} else {
				// 条件が偽なら終了地点に飛ばす
				sprintf(label,"__FR%x_FIN",syntax.curly[pos].index);
				add_scriptl(add_str("jump_zero"));
				add_scriptc(C_ARG);
				p=parse_expr(p);
				p=skip_space(p);
				add_scriptl(add_str(label));
				add_scriptc(C_FUNC);
			}
			if(*p != ';')
				disp_error_message("parse_syntax: need ';'",p);
			p++;

			// ループ開始に飛ばす
			sprintf(label,"goto __FR%x_BGN;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;

			// 次のループへのラベル形成する
			sprintf(label,"__FR%x_NXT",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			// 次のループに入る時の処理
			// for 最後の ')' を ';' として扱うフラグ
			parse_syntax_for_flag = 1;
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			p=parse_line(p);
			syntax.curly_count--;
			parse_syntax_for_flag = 0;

			// 条件判定処理に飛ばす
			sprintf(label,"goto __FR%x_J;",syntax.curly[pos].index);
			syntax.curly[syntax.curly_count++].type = TYPE_NULL;
			parse_line(label);
			syntax.curly_count--;

			// ループ開始のラベル付け
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
				if( str_data[l].type == C_NOP )//## ??? [FlavioJS]
					str_data[l].type = C_USERFUNC;

				// if, for , while の閉じ判定
				p = parse_syntax_close(p2 + 1);
				return p;			}
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
				if( str_data[l].type == C_NOP )//## ??? [FlavioJS]
					str_data[l].type = C_USERFUNC;
				set_label(l, script_pos, p);
				if( parse_options&SCRIPT_USE_LABEL_DB )
					strdb_put(scriptlabel_db, get_str(l), (void*)script_pos);
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
			// if() の処理
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
			// switch() の処理
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
				disp_error_message("parse_syntax: need '{'",p);
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
			// 条件判断開始のラベル形成する
			sprintf(label,"__WL%x_NXT",syntax.curly[syntax.curly_count].index);
			l=add_str(label);
			set_label(l,script_pos,p);

			// 条件が偽なら終了地点に飛ばす
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
	// if(...) for(...) hoge(); のように、１度閉じられたら再度閉じられるか確認する
	int flag;

	do {
		p = parse_syntax_close_sub(p,&flag);
	} while(flag);
	return p;
}

// if, for , while , do の閉じ判定
//	 flag == 1 : 閉じられた
//	 flag == 0 : 閉じられない
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
		// if 最終場所へ飛ばす
		sprintf(label,"goto __IF%x_FIN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// 現在地のラベルを付ける
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
		// if 閉じ
		syntax.curly_count--;
		// 最終地のラベルを付ける
		sprintf(label,"__IF%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		if(syntax.curly[pos].flag == 1) {
			// このifに対するelseじゃないのでポインタの位置は同じ
			return bp;
		}
		return p;
	} else if(syntax.curly[pos].type == TYPE_DO) {
		int l;
		char label[256];
		const char *p2;

		if(syntax.curly[pos].flag) {
			// 現在地のラベル形成する(continue でここに来る)
			sprintf(label,"__DO%x_NXT",syntax.curly[pos].index);
			l=add_str(label);
			set_label(l,script_pos,p);
		}

		// 条件が偽なら終了地点に飛ばす
		p = skip_space(p);
		p2 = skip_word(p);
		if(p2 - p != 5 || strncasecmp(p,"while",5))
			disp_error_message("parse_syntax: need 'while'",p);

		p = skip_space(p2);
		if(*p != '(') {
			disp_error_message("need '('",p);
		}
		sprintf(label,"__DO%x_FIN",syntax.curly[pos].index);
		add_scriptl(add_str("jump_zero"));
		add_scriptc(C_ARG);
		p=parse_expr(p);
		p=skip_space(p);
		add_scriptl(add_str(label));
		add_scriptc(C_FUNC);

		// 開始地点に飛ばす
		sprintf(label,"goto __DO%x_BGN;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// 条件終了地点のラベル形成する
		sprintf(label,"__DO%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		p = skip_space(p);
		if(*p != ';') {
			disp_error_message("parse_syntax: need ';'",p);
			return p+1;
		}
		p++;
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[pos].type == TYPE_FOR) {
		// 次のループに飛ばす
		sprintf(label,"goto __FR%x_NXT;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// for 終了のラベル付け
		sprintf(label,"__FR%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[pos].type == TYPE_WHILE) {
		// while 条件判断へ飛ばす
		sprintf(label,"goto __WL%x_NXT;",syntax.curly[pos].index);
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// while 終了のラベル付け
		sprintf(label,"__WL%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else if(syntax.curly[syntax.curly_count-1].type == TYPE_USERFUNC) {
		int pos = syntax.curly_count-1;
		char label[256];
		int l;
		// 戻す
		sprintf(label,"return;");
		syntax.curly[syntax.curly_count++].type = TYPE_NULL;
		parse_line(label);
		syntax.curly_count--;

		// 現在地のラベルを付ける
		sprintf(label,"__FN%x_FIN",syntax.curly[pos].index);
		l=add_str(label);
		set_label(l,script_pos,p);
		syntax.curly_count--;
		return p;
	} else {
		*flag = 0;
		return p;
	}
}

/*==========================================
 * 組み込み関数の追加
 *------------------------------------------*/
static void add_buildin_func(void)
{
	int i,n;
	const char* p;
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
		p = buildin_func[i].arg;
		while( *p == 'v' || *p == 's' || *p == 'i' || *p == 'r' || *p == 'l' ) ++p;
		while( *p == '?' ) ++p;
		if( *p == '*' ) ++p;
		if( *p != 0){
			ShowWarning("add_buildin_func: ignoring function \"%s\" with invalid arg \"%s\".\n", buildin_func[i].name, buildin_func[i].arg);
		} else if( *skip_word(buildin_func[i].name) != 0 ){
			ShowWarning("add_buildin_func: ignoring function with invalid name \"%s\" (must be a word).\n", buildin_func[i].name);
		} else {
			n = add_str(buildin_func[i].name);
			str_data[n].type = C_FUNC;
			str_data[n].val = i;
			str_data[n].func = buildin_func[i].func;
		}
	}
}

/*==========================================
 * 定数データベースの読み込み
 *------------------------------------------*/
static void read_constdb(void)
{
	FILE *fp;
	char line[1024],name[1024],val[1024];
	int n,type;

	sprintf(line, "%s/const.txt", db_path);
	fp=fopen(line, "r");
	if(fp==NULL){
		ShowError("can't read %s\n", line);
		return ;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0]=='/' && line[1]=='/')
			continue;
		type=0;
		if(sscanf(line,"%[A-Za-z0-9_],%[-0-9xXA-Fa-f],%d",name,val,&type)>=2 ||
		   sscanf(line,"%[A-Za-z0-9_] %[-0-9xXA-Fa-f] %d",name,val,&type)>=2){
			n=add_str(name);
			if(type==0)
				str_data[n].type=C_INT;
			else
				str_data[n].type=C_PARAM;
			str_data[n].val= (int)strtol(val,NULL,0);
		}
	}
	fclose(fp);
}

/*==========================================
 * エラー表示
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

void script_error(const char* src, const char* file, int start_line, const char* error_msg, const char* error_pos)
{
	// エラーが発生した行を求める
	int j;
	int line = start_line;
	const char *p;
	const char *linestart[5] = { NULL, NULL, NULL, NULL, NULL };
	StringBuf buf;

	for(p=src;p && *p;line++){
		const char *lineend=strchr(p,'\n');
		if(lineend==NULL || error_pos<lineend){
			break;
		}
		for( j = 0; j < 4; j++ ) {
			linestart[j] = linestart[j+1];
		}
		linestart[4] = p;
		p=lineend+1;
	}

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "\a\n");
	StringBuf_Printf(&buf, "script error on %s line %d\n", file, line);
	StringBuf_Printf(&buf, "    %s\n", error_msg);
	for(j = 0; j < 5; j++ ) {
		script_print_line(&buf, linestart[j], NULL, line + j - 5);
	}
	p = script_print_line(&buf, p, error_pos, -line);
	for(j = 0; j < 5; j++) {
		p = script_print_line(&buf, p, NULL, line + j + 1 );
	}
	ShowError("%s", StringBuf_Value(&buf));
	StringBuf_Destroy(&buf);
}

/*==========================================
 * スクリプトの解析
 *------------------------------------------*/
struct script_code* parse_script(const char *src,const char *file,int line,int options)
{
	const char *p,*tmpp;
	int i;
	struct script_code* code = NULL;
	static int first=1;
	char end;

	if( src == NULL )
		return NULL;// empty script

	memset(&syntax,0,sizeof(syntax));
	if(first){
		add_buildin_func();
		read_constdb();
		first=0;
	}

	script_buf=(unsigned char *)aMalloc(SCRIPT_BLOCK_SIZE*sizeof(unsigned char));
	script_pos=0;
	script_size=SCRIPT_BLOCK_SIZE;
	str_data[LABEL_NEXTLINE].type=C_NOP;
	str_data[LABEL_NEXTLINE].backpatch=-1;
	str_data[LABEL_NEXTLINE].label=-1;

	// who called parse_script is responsible for clearing the database after using it, but just in case... lets clear it here
	if( options&SCRIPT_USE_LABEL_DB )
		scriptlabel_db->clear(scriptlabel_db, NULL);
	parse_options = options;

	if( setjmp( error_jump ) != 0 ) {
		//Restore program state when script has problems. [from jA]
		int i;
		const int size = ARRAYLENGTH(syntax.curly);
		if( error_report )
			script_error(src,file,line,error_msg,error_pos);
		aFree( error_msg );
		aFree( script_buf );
		script_pos  = 0;
		script_size = 0;
		script_buf  = NULL;
		for(i=LABEL_START;i<str_num;i++)
			if(str_data[i].type == C_NOP) str_data[i].type = C_NAME;
		for(i=0; i<size; i++)
			linkdb_final(&syntax.curly[i].case_label);
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
		// labelだけ特殊処理
		tmpp=skip_space(skip_word(p));
		if(*tmpp==':' && !(!strncasecmp(p,"default:",8) && p + 7 == tmpp)){
			i=add_word(p);
			set_label(i,script_pos,p);
			if( parse_options&SCRIPT_USE_LABEL_DB )
				strdb_put(scriptlabel_db, get_str(i), (void*)script_pos);
			p=tmpp+1;
			p=skip_space(p);
			continue;
		}

		// 他は全部一緒くた
		p=parse_line(p);
		p=skip_space(p);
		add_scriptc(C_EOL);

		set_label(LABEL_NEXTLINE,script_pos,p);
		str_data[LABEL_NEXTLINE].type=C_NOP;
		str_data[LABEL_NEXTLINE].backpatch=-1;
		str_data[LABEL_NEXTLINE].label=-1;
	}

	add_scriptc(C_NOP);

	// trim code to size
	script_size = script_pos;
	RECREATE(script_buf,unsigned char,script_pos);

	// default unknown references to variables
	for(i=LABEL_START;i<str_num;i++){
		if(str_data[i].type==C_NOP){
			int j,next;
			str_data[i].type=C_NAME;
			str_data[i].label=i;
			for(j=str_data[i].backpatch;j>=0 && j!=0x00ffffff;){
				next=GETVALUE(script_buf,j);
				SETVALUE(script_buf,j,i);
				j=next;
			}
		}
	}

#ifdef DEBUG_DISP
	for(i=0;i<script_pos;i++){
		if((i&15)==0) printf("%04x : ",i);
		ShowMessage("%02x ",script_buf[i]);
		if((i&15)==15) printf("\n");
	}
	ShowMessage("\n");
#endif
#ifdef DEBUG_DISASM
	{
		int i = 0,j;
		while(i < script_pos) {
			ShowMessage("%06x ",i);
			j = i;
			switch(get_com(script_buf,&i)) {
			case C_EOL:	 printf("C_EOL"); break;
			case C_INT:	 printf("C_INT %d",get_num(script_buf,&i)); break;
			case C_POS:
				ShowMessage("C_POS  0x%06x",*(int*)(script_buf+i)&0xffffff);
				i += 3;
				break;
			case C_NAME:
				j = (*(int*)(script_buf+i)&0xffffff);
				ShowMessage("C_NAME %s",j == 0xffffff ? "?? unknown ??" : get_str(j));
				i += 3;
				break;
			case C_ARG:		ShowMessage("C_ARG"); break;
			case C_FUNC:	ShowMessage("C_FUNC"); break;
			case C_ADD:	 	ShowMessage("C_ADD"); break;
			case C_SUB:		ShowMessage("C_SUB"); break;
			case C_MUL:		ShowMessage("C_MUL"); break;
			case C_DIV:		ShowMessage("C_DIV"); break;
			case C_MOD:		ShowMessage("C_MOD"); break;
			case C_EQ:		ShowMessage("C_EQ"); break;
			case C_NE:		ShowMessage("C_NE"); break;
			case C_GT:		ShowMessage("C_GT"); break;
			case C_GE:		ShowMessage("C_GE"); break;
			case C_LT:		ShowMessage("C_LT"); break;
			case C_LE:		ShowMessage("C_LE"); break;
			case C_AND:		ShowMessage("C_AND"); break;
			case C_OR:		ShowMessage("C_OR"); break;
			case C_XOR:		ShowMessage("C_XOR"); break;
			case C_LAND:	ShowMessage("C_LAND"); break;
			case C_LOR:		ShowMessage("C_LOR"); break;
			case C_R_SHIFT:	ShowMessage("C_R_SHIFT"); break;
			case C_L_SHIFT:	ShowMessage("C_L_SHIFT"); break;
			case C_NEG:		ShowMessage("C_NEG"); break;
			case C_NOT:		ShowMessage("C_NOT"); break;
			case C_LNOT:	ShowMessage("C_LNOT"); break;
			case C_NOP:		ShowMessage("C_NOP"); break;
			case C_OP3:		ShowMessage("C_OP3"); break;
			case C_STR:
				j = strlen(script_buf + i);
				ShowMessage("C_STR %s",script_buf + i);
				i+= j+1;
				break;
			default:
				ShowMessage("unknown");
			}
			ShowMessage("\n");
		}
	}
#endif

	CREATE(code,struct script_code,1);
	code->script_buf  = script_buf;
	code->script_size = script_size;
	code->script_vars = NULL;
	return code;
}

/// Returns the player attached to this script, identified by the rid.
/// If there is no player attached, the script is terminated.
TBL_PC *script_rid2sd(struct script_state *st)
{
	TBL_PC *sd=map_id2sd(st->rid);
	if(!sd){
		ShowError("script_rid2sd: fatal error ! player not attached!\n");
		script_reportsrc(st);
		st->state = END;
	}
	return sd;
}

/// Dereferences a variable/constant, replacing it with a copy of the value.
///
/// @param st Script state
/// @param data Variable/constant
void get_val(struct script_state* st, struct script_data* data)
{
	const char* name;
	char prefix;
	char postfix;
	TBL_PC* sd = NULL;

	if( !data_isreference(data) )
		return;// not a variable/constant

	name = reference_getname(data);
	prefix = name[0];
	postfix = name[strlen(name) - 1];

	//##TODO use reference_tovariable(data) when it's confirmed that it works [FlavioJS]
	if( !reference_toconstant(data) && not_server_variable(prefix) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
		{// needs player attached
			if( postfix == '$' )
			{// string variable
				ShowWarning("script:get_val: cannot access player variable '%s', defaulting to \"\"\n", name);
				data->type = C_CONSTSTR;
				data->u.str = "";
			}
			else
			{// integer variable
				ShowWarning("script:get_val: cannot access player variable '%s', defaulting to 0\n", name);
				data->type = C_INT;
				data->u.num = 0;
			}
			return;
		}
	}

	if( postfix == '$' )
	{// string variable

		switch( prefix )
		{
		case '@':
			data->u.str = pc_readregstr(sd, data->u.num);
			break;
		case '$':
			data->u.str = mapreg_readregstr(data->u.num);
			break;
		case '#':
			if( name[1] == '#' )
				data->u.str = pc_readaccountreg2str(sd, name);// global
			else
				data->u.str = pc_readaccountregstr(sd, name);// local
			break;
		case '.':
			{
				struct linkdb_node** n =
					data->ref      ? data->ref:
					name[1] == '@' ? st->stack->var_function:// instance/scope variable
					                 &st->script->script_vars;// npc variable
				data->u.str = (char*)linkdb_search(n, (void*)reference_getuid(data));
			}
			break;
		default:
			data->u.str = pc_readglobalreg_str(sd, name);
			break;
		}

		if( data->u.str == NULL || data->u.str[0] == '\0' )
		{// empty string
			data->type = C_CONSTSTR;
			data->u.str = "";
		}
		else
		{// duplicate string
			data->type = C_STR;
			data->u.str = aStrdup(data->u.str);
		}

	}
	else
	{// integer variable

		data->type = C_INT;

		if( reference_toconstant(data) )
		{
			data->u.num = reference_getconstant(data);
		}
		else if( reference_toparam(data) )
		{
			data->u.num = pc_readparam(sd, reference_getparamtype(data));
		}
		else
		switch( prefix )
		{
		case '@':
			data->u.num = pc_readreg(sd, data->u.num);
			break;
		case '$':
			data->u.num = mapreg_readreg(data->u.num);
			break;
		case '#':
			if( name[1] == '#' )
				data->u.num = pc_readaccountreg2(sd, name);// global
			else
				data->u.num = pc_readaccountreg(sd, name);// local
			break;
		case '.':
			{
				struct linkdb_node** n =
					data->ref      ? data->ref:
					name[1] == '@' ? st->stack->var_function:// instance/scope variable
					                 &st->script->script_vars;// npc variable
				data->u.num = (int)linkdb_search(n, (void*)reference_getuid(data));
			}
			break;
		default:
			data->u.num = pc_readglobalreg(sd, name);
			break;
		}

	}

	return;
}

void push_val2(struct script_stack* stack, enum c_op type, int val, struct linkdb_node** ref);

/// Retrieves the value of a reference identified by uid (variable, constant, param)
/// The value is left in the top of the stack and needs to be removed manually.
void* get_val2(struct script_state* st, int uid, struct linkdb_node** ref)
{
	struct script_data* data;
	push_val2(st->stack, C_NAME, uid, ref);
	data = script_getdatatop(st, -1);
	get_val(st, data);
	return (data->type == C_INT ? (void*)data->u.num : (void*)data->u.str);
}

/*==========================================
 * Stores the value of a script variable
 * Return value is 0 on fail, 1 on success.
 *------------------------------------------*/
static int set_reg(struct script_state* st, TBL_PC* sd, int num, const char* name, const void* value, struct linkdb_node** ref)
{
	char prefix = name[0];

	if( is_string_variable(name) )
	{// string variable
		const char* str = (const char*)value;
		switch (prefix) {
		case '@':
			return pc_setregstr(sd, num, str);
		case '$':
			return mapreg_setregstr(num, str);
		case '#':
			return (name[1] == '#') ?
				pc_setaccountreg2str(sd, name, str) :
				pc_setaccountregstr(sd, name, str);
		case '.': {
			char* p;
			struct linkdb_node** n;
			n = (ref) ? ref : (name[1] == '@') ? st->stack->var_function : &st->script->script_vars;
			p = (char*)linkdb_erase(n, (void*)num);
			if (p) aFree(p);
			if (str[0]) linkdb_insert(n, (void*)num, aStrdup(str));
			}
			return 1;
		default:
			return pc_setglobalreg_str(sd, name, str);
		}
	}
	else
	{// integer variable
		int val = (int)value;
		if(str_data[num&0x00ffffff].type == C_PARAM)
			return pc_setparam(sd, str_data[num&0x00ffffff].val, val);

		switch (prefix) {
		case '@':
			return pc_setreg(sd, num, val);
		case '$':
			return mapreg_setreg(num, val);
		case '#':
			return (name[1] == '#') ?
				pc_setaccountreg2(sd, name, val) :
				pc_setaccountreg(sd, name, val);
		case '.': {
			struct linkdb_node** n;
			n = (ref) ? ref : (name[1] == '@') ? st->stack->var_function : &st->script->script_vars;
			if (val == 0)
				linkdb_erase(n, (void*)num);
			else 
				linkdb_replace(n, (void*)num, (void*)val);
			}
			return 1;
		default:
			return pc_setglobalreg(sd, name, val);
		}
	}
}

int set_var(TBL_PC* sd, char* name, void* val)
{
    return set_reg(NULL, sd, reference_uid(add_str(name),0), name, val, NULL);
}

void setd_sub(struct script_state *st, TBL_PC *sd, const char *varname, int elem, void *value, struct linkdb_node **ref)
{
	set_reg(st, sd, reference_uid(add_str(varname),elem), varname, value, ref);
}

/// Converts the data to a string
const char* conv_str(struct script_state* st, struct script_data* data)
{
	char* p;

	get_val(st, data);
	if( data_isstring(data) )
	{// nothing to convert
	}
	else if( data_isint(data) )
	{// int -> string
		CREATE(p, char, ITEM_NAME_LENGTH);
		snprintf(p, ITEM_NAME_LENGTH, "%d", data->u.num);
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
		data->u.str = "";
	}
	return data->u.str;
}

/// Converts the data to an int
int conv_num(struct script_state* st, struct script_data* data)
{
	char* p;
	long num;

	get_val(st, data);
	if( data_isint(data) )
	{// nothing to convert
	}
	else if( data_isstring(data) )
	{// string -> int
		// the result does not overflow or underflow, it is capped instead
		// ex: 999999999999 is capped to INT_MAX (2147483647)
		p = data->u.str;
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
	return data->u.num;
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
void push_val2(struct script_stack* stack, enum c_op type, int val, struct linkdb_node** ref)
{
	if( stack->sp >= stack->sp_max )
		stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.num = val;
	stack->stack_data[stack->sp].ref   = ref;
	stack->sp++;
}

/// Pushes a string into the stack
void push_str(struct script_stack* stack, enum c_op type, char* str)
{
	if( stack->sp >= stack->sp_max )
		stack_expand(stack);
	stack->stack_data[stack->sp].type  = type;
	stack->stack_data[stack->sp].u.str = str;
	stack->stack_data[stack->sp].ref   = NULL;
	stack->sp++;
}

/// Pushes a copy of the target position into the stack
void push_copy(struct script_stack* stack, int pos)
{
	switch( stack->stack_data[pos].type )
	{
	case C_CONSTSTR:
		push_str(stack, C_CONSTSTR, stack->stack_data[pos].u.str);
		break;
	case C_STR:
		push_str(stack, C_STR, aStrdup(stack->stack_data[pos].u.str));
		break;
	default:
		push_val2(
			stack,stack->stack_data[pos].type,
			stack->stack_data[pos].u.num,
			stack->stack_data[pos].ref
		);
		break;
	}
}

/// Removes the values in indexes [start,end[ from the stack
void pop_stack(struct script_stack* stack, int start, int end)
{
	struct script_data* data;
	int i;

	if( start < 0 )
		start = 0;
	if( end > stack->sp_max )
		end = stack->sp_max;
	if( start >= end )
		return;// nothing to pop

	// free stack elements
	for( i = start; i < end; i++ )
	{
		data = &stack->stack_data[i];
		if( data->type == C_STR )
			aFree(data->u.str);
		data->type = C_NOP;
	}
	// move the rest of the elements
	if( stack->sp > end )
		memmove(&stack->stack_data[start], &stack->stack_data[end], sizeof(stack->stack_data[0])*(stack->sp - end));
	stack->sp -= end - start;
}

///
///
///

/*==========================================
 * スクリプト依存変数、関数依存変数の解放
 *------------------------------------------*/
void script_free_vars(struct linkdb_node **node)
{
	struct linkdb_node* n = *node;
	while( n != NULL)
	{
		const char* name = get_str((int)(n->key)&0x00ffffff);
		if( is_string_variable(name) )
			aFree(n->data); // 文字型変数なので、データ削除
		n = n->next;
	}
	linkdb_final( node );
}

/*==========================================
 * Free's the whole stack. Invoked when clearing a character. [Skotlex]
 *------------------------------------------*/
void script_free_stack(struct script_stack *stack)
{
	int i;
	for(i = 0; i < stack->sp; i++) {
		if( stack->stack_data[i].type == C_STR ) {
			aFree(stack->stack_data[i].u.str);
			stack->stack_data[i].type = C_INT;
		} else if( i > 0 && stack->stack_data[i].type == C_RETINFO ) {
			struct linkdb_node** n = (struct linkdb_node**)stack->stack_data[i-1].u.num;
			script_free_vars( n );
			aFree( n );
		}
	}
	script_free_vars( stack->var_function );
	aFree(stack->var_function);
	aFree(stack->stack_data);
	aFree(stack);
}

void script_free_code(struct script_code* code)
{
	script_free_vars( &code->script_vars );
	aFree( code->script_buf );
	aFree( code );
}

//
// 実行部main
//
/*==========================================
 * コマンドの読み取り
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
 * 数値の所得
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

/*==========================================
 * スタックから値を取り出す
 *------------------------------------------*/
int pop_val(struct script_state* st)
{
	if(st->stack->sp<=0)
		return 0;
	st->stack->sp--;
	get_val(st,&(st->stack->stack_data[st->stack->sp]));
	if(st->stack->stack_data[st->stack->sp].type==C_INT)
		return st->stack->stack_data[st->stack->sp].u.num;
	return 0;
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
		flag = data->u.num;// 0 -> false
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
			char* buf = (char *)aMallocA((strlen(s1)+strlen(s2)+1)*sizeof(char));
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

	switch( op )
	{
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
		switch( op )
		{// operators that can overflow/underflow
		case C_ADD: ret = i1 + i2; ret_double = (double)i1 + (double)i2; break;
		case C_SUB: ret = i1 - i2; ret_double = (double)i1 - (double)i2; break;
		case C_MUL: ret = i1 * i2; ret_double = (double)i1 * (double)i2; break;
		default:
			ShowError("script:op_2num: unexpected number operator %s i1=%d i2=%d\n", script_op2name(op), i1, i2);
			script_reportsrc(st);
			script_pushnil(st);
			return;
		}
		if( ret_double < (double)INT_MIN )
		{
			ShowWarning("script:op_2num: underflow detected op=%s i1=%d i2=%d\n", script_op2name(op), i1, i2);
			script_reportsrc(st);
			ret = INT_MIN;
		}
		else if( ret_double > (double)INT_MAX )
		{
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
	struct script_data* left;
	struct script_data* right;

	left = script_getdatatop(st, -2);
	right = script_getdatatop(st, -1);

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
		script_removetop(st, -3, -1);// pop the two values before the top one
	}
	else if( data_isint(left) && data_isint(right) )
	{// ii => op_2num
		int i1 = left->u.num;
		int i2 = right->u.num;
		script_removetop(st, -2, 0);
		op_2num(st, op, i1, i2);
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

	i1 = data->u.num;
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


/*==========================================
 * 関数の実行
 *------------------------------------------*/
int run_func(struct script_state *st)
{
	int i,start_sp,end_sp,func;

	end_sp=st->stack->sp;
	for(i=end_sp-1;i>=0 && st->stack->stack_data[i].type!=C_ARG;i--);
	if(i<=0){ //Crash fix when missing "push_val" causes current pointer to become -1. from Rayce (jA)
		ShowError("function not found\n");
//		st->stack->sp=0;
		st->state=END;
		script_reportsrc(st);
		return 1;
	}
	start_sp=i-1;
	st->start=i-1;
	st->end=end_sp;
	func=st->stack->stack_data[st->start].u.num;

#ifdef DEBUG_RUN
	if(battle_config.etc_log) {
		ShowDebug("run_func : %s? (%d(%d)) sp=%d (%d...%d)\n", get_str(func), func, str_data[func].type, st->stack->sp, st->start, st->end);
		ShowDebug("stack dump :");
		for(i=0;i<end_sp;i++){
			switch(st->stack->stack_data[i].type){
			case C_INT:
				ShowMessage(" int(%d)", st->stack->stack_data[i].u.num);
				break;
			case C_NAME:
				ShowMessage(" name(%s)", get_str(st->stack->stack_data[i].u.num & 0xffffff);
				break;
			case C_ARG:
				ShowMessage(" arg");
				break;
			case C_POS:
				ShowMessage(" pos(%d)",st->stack->stack_data[i].u.num);
				break;
			case C_STR:
				ShowMessage(" str(%s)",st->stack->stack_data[i].u.str);
				break;
			case C_CONSTSTR:
				ShowMessage(" cstr(%s)",st->stack->stack_data[i].u.str);
				break;
			default:
				ShowMessage(" etc(%d,%d)",st->stack->stack_data[i].type,st->stack->stack_data[i].u.num);
			}
		}
		ShowMessage("\n");
	}
#endif

	if(str_data[func].type!=C_FUNC ){
		ShowError("run_func: '"CL_WHITE"%s"CL_RESET"' (type %d) is not function and command!\n", get_str(func), str_data[func].type);
//		st->stack->sp=0;
		st->state=END;
		script_reportsrc(st);
		return 1;
	}
#ifdef DEBUG_RUN
	ShowDebug("run_func : %s (func_no : %d , func_type : %d pos : 0x%x)\n", get_str(func),func,str_data[func].type,st->pos);
#endif
	if(str_data[func].func){
		if (str_data[func].func(st)) //Report error
			script_reportsrc(st);
	} else {
		ShowError("run_func : %s? (%d(%d))\n", get_str(func),func,str_data[func].type);
		script_pushint(st,0);
		script_reportsrc(st);
	}

	// Stack's datum are used when re-run functions [Eoe]
	if(st->state != RERUNLINE) {
		pop_stack(st->stack,start_sp,end_sp);
	}

	if(st->state==RETFUNC){
		// ユーザー定義関数からの復帰
		int olddefsp=st->stack->defsp;
		int i;

		pop_stack(st->stack,st->stack->defsp,start_sp);	// 復帰に邪魔なスタック削除
		if(st->stack->defsp<5 || st->stack->stack_data[st->stack->defsp-1].type!=C_RETINFO){
			ShowWarning("script:run_func(return) return without callfunc or callsub!\n");
			st->state=END;
			script_reportsrc(st);
			return 1;
		}
		script_free_vars( st->stack->var_function );
		aFree(st->stack->var_function);

		i = conv_num(st,& (st->stack->stack_data[st->stack->defsp-5]));					// 引数の数所得
		st->pos=conv_num(st,& (st->stack->stack_data[st->stack->defsp-1]));				// スクリプト位置の復元
		st->script=(struct script_code*)conv_num(st,& (st->stack->stack_data[st->stack->defsp-3]));	// スクリプトを復元
		st->stack->var_function = (struct linkdb_node**)st->stack->stack_data[st->stack->defsp-2].u.num; // 関数依存変数

		st->stack->defsp=conv_num(st,& (st->stack->stack_data[st->stack->defsp-4]));	// 基準スタックポインタを復元
		pop_stack(st->stack,olddefsp-5-i,olddefsp);		// 要らなくなったスタック(引数と復帰用データ)削除

		st->state=GOTO;
	}

	return 0;
}

/*==========================================
 * script execution
 *------------------------------------------*/
void run_script_main(struct script_state *st);

void run_script(struct script_code *rootscript,int pos,int rid,int oid)
{
	struct script_state *st;
	TBL_PC *sd=NULL;

	if(rootscript==NULL || pos<0)
		return;

	if (rid) sd = map_id2sd(rid);
	if (sd && sd->st && sd->st->scriptroot == rootscript && sd->st->pos == pos){
		//Resume script.
		st = sd->st;
	} else {
		st = (struct script_state*)aCalloc(sizeof(struct script_state), 1);
		// the script is different, make new script_state and stack
		st->stack = (struct script_stack*)aMalloc (sizeof(struct script_stack));
		st->stack->sp=0;
		st->stack->sp_max=64;
		st->stack->stack_data = (struct script_data *)aCalloc(st->stack->sp_max,sizeof(st->stack->stack_data[0]));
		st->stack->defsp = st->stack->sp;
		st->stack->var_function = (struct linkdb_node**)aCalloc(1, sizeof(struct linkdb_node*));
		st->state  = RUN;
		st->script = rootscript;
	}
	st->pos = pos;
	st->rid = rid;
	st->oid = oid;
	st->sleep.timer = INVALID_TIMER;
	run_script_main(st);
}

void script_stop_sleeptimers(int id)
{
	struct script_state* st;
	for(;;)
	{
		st = (struct script_state*)linkdb_erase(&sleep_db,(void*)id);
		if( st == NULL )
			break; // no more sleep timers
		if( st->sleep.timer != INVALID_TIMER )
			delete_timer(st->sleep.timer, run_script_timer);
		script_free_stack(st->stack);
		aFree(st);
	}
}

/*==========================================
 * 指定ノードをsleep_dbから削除
 *------------------------------------------*/
struct linkdb_node* script_erase_sleepdb(struct linkdb_node *n)
{
	struct linkdb_node *retnode;

	if( n == NULL)
		return NULL;
	if( n->prev == NULL )
		sleep_db = n->next;
	else
		n->prev->next = n->next;
	if( n->next )
		n->next->prev = n->prev;
	retnode = n->next;
	aFree( n );
	return retnode;		// 次のノードを返す
}

/*==========================================
 * sleep用タイマー関数
 *------------------------------------------*/
int run_script_timer(int tid, unsigned int tick, int id, intptr data)
{
	struct script_state *st     = (struct script_state *)data;
	struct linkdb_node *node    = (struct linkdb_node *)sleep_db;
	TBL_PC *sd = map_id2sd(st->rid);

	if((sd && sd->status.char_id != id) || (st->rid && !sd))
	{	//Character mismatch. Cancel execution.
		st->rid = 0;
		st->state = END;
	}
	while( node && st->sleep.timer != -1 ) {
		if( (int)node->key == st->oid && ((struct script_state *)node->data)->sleep.timer == st->sleep.timer ) {
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

/*==========================================
 * スクリプトの実行メイン部分
 *------------------------------------------*/
void run_script_main(struct script_state *st)
{
	int cmdcount=script_config.check_cmdcount;
	int gotocount=script_config.check_gotocount;
	TBL_PC *sd;
	//For backing up purposes
	struct script_state *bk_st = NULL;
	int bk_npcid = 0;
	struct script_stack *stack=st->stack;

	sd = st->rid?map_id2sd(st->rid):NULL;

	if(sd){
		if(sd->st != st){
			bk_st = sd->st;
			bk_npcid = sd->npc_id;
		}
		sd->st = st;
		sd->npc_id = st->oid;
	}

	if(st->state == RERUNLINE) {
		st->state = RUN;
		run_func(st);
		if(st->state == GOTO)
			st->state = RUN;
	} else if(st->state != END)
		st->state = RUN;

	while(st->state == RUN)
	{
		enum c_op c = get_com(st->script->script_buf,&st->pos);
		switch(c){
		case C_EOL:
			if( stack->sp != stack->defsp )
			{
				if( stack->sp > stack->defsp )
				{	//sp > defsp is valid in cases when you invoke functions and don't use the returned value. [Skotlex]
					//Since sp is supposed to be defsp in these cases, we could assume the extra stack elements are unneeded.
					pop_stack(stack, stack->defsp, stack->sp); //Clear out the unused stack-section.
				} else
					ShowError("script:run_script_main: unexpected stack position stack.sp(%d) != default(%d)\n", stack->sp, stack->defsp);
				stack->sp = stack->defsp;
			}
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
				if( gotocount>0 && (--gotocount)<=0 ){
					ShowError("run_script: infinity loop !\n");
					script_reportsrc(st);
					st->state=END;
				}
			}
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
			ShowError("unknown command : %d @ %d\n",c,st->pos);
			st->state=END;
			break;
		}
		if( cmdcount>0 && (--cmdcount)<=0 ){
			ShowError("run_script: infinity loop !\n");
			script_reportsrc(st);
			st->state=END;
		}
	}

	if(st->sleep.tick > 0) {
		//Delay execution
		st->sleep.charid = sd?sd->status.char_id:0;
		st->sleep.timer  = add_timer(gettick()+st->sleep.tick,
			run_script_timer, st->sleep.charid, (int)st);
		linkdb_insert(&sleep_db, (void*)st->oid, st);
		//Restore previous script
		if (sd) {
			sd->st = bk_st;
			sd->npc_id = bk_npcid;
			bk_st = NULL; //Remove tag for removal.
		}
	}
	else if(st->state != END && sd){
		//Resume later (st is already attached to player).
		if(bk_st) {
			ShowWarning("Unable to restore stack! Double continuation!\n");
			//Report BOTH scripts to see if that can help somehow.
			ShowDebug("Previous script (lost):\n");
			script_reportsrc(bk_st);
			ShowDebug("Current script:\n");
			script_reportsrc(st);
		}
	} else {
		//Dispose of script.
		if (sd)
		{	//Restore previous stack and save char.
			if(sd->state.using_fake_npc){
				clif_clearunit_single(sd->npc_id, 0, sd->fd);
				sd->state.using_fake_npc = 0;
			}
			//Restore previous script if any.
			sd->st = bk_st;
			sd->npc_id = bk_npcid;
			if (!bk_st)
				npc_event_dequeue(sd);
			else
				bk_st = NULL; //Remove tag for removal.
			if (sd->state.reg_dirty&2)
				intif_saveregistry(sd,2);
			if (sd->state.reg_dirty&1)
				intif_saveregistry(sd,1);
		}
		st->pos = -1;
		script_free_stack (st->stack);
		aFree(st);
	}

	if (bk_st)
	{	//Remove previous script
		bk_st->pos = -1;
		script_free_stack(bk_st->stack);
		aFree(bk_st);
		bk_st = NULL;
	}

}

int script_config_read(char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024];
	FILE *fp;


	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("file not found: [%s]\n", cfgName);
		return 1;
	}
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0] == '/' && line[1] == '/')
			continue;
		i=sscanf(line,"%[^:]: %[^\r\n]",w1,w2);
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
		else if(strcmpi(w1,"import")==0){
			script_config_read(w2);
		}
	}
	fclose(fp);

	return 0;
}

static int do_final_userfunc_sub (DBKey key,void *data,va_list ap)
{
	struct script_code *code = (struct script_code *)data;
	if(code){
		script_free_vars( &code->script_vars );
		aFree( code->script_buf );
		aFree( code );
	}
	return 0;
}

/*==========================================
 * 終了
 *------------------------------------------*/
int do_final_script()
{
#ifdef DEBUG_HASH
	if (battle_config.etc_log)
	{
		FILE *fp = fopen("hash_dump.txt","wt");
		if(fp) {
			int i,count[SCRIPT_HASH_SIZE];
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

	scriptlabel_db->destroy(scriptlabel_db,NULL);
	userfunc_db->destroy(userfunc_db,do_final_userfunc_sub);
	if(sleep_db) {
		struct linkdb_node *n = (struct linkdb_node *)sleep_db;
		while(n) {
			struct script_state *st = (struct script_state *)n->data;
			script_free_stack(st->stack);
			aFree(st);
			n = n->next;
		}
		linkdb_final(&sleep_db);
	}

	if (str_data)
		aFree(str_data);
	if (str_buf)
		aFree(str_buf);

	return 0;
}
/*==========================================
 * 初期化
 *------------------------------------------*/
int do_init_script()
{
	userfunc_db=strdb_alloc(DB_OPT_DUP_KEY,0);
	scriptlabel_db=strdb_alloc((DBOptions)(DB_OPT_DUP_KEY|DB_OPT_ALLOW_NULL_DATA),50);

	mapreg_init();
	
	return 0;
}

int script_reload()
{
	userfunc_db->clear(userfunc_db,do_final_userfunc_sub);
	scriptlabel_db->clear(scriptlabel_db, NULL);

	if(sleep_db) {
		struct linkdb_node *n = (struct linkdb_node *)sleep_db;
		while(n) {
			struct script_state *st = (struct script_state *)n->data;
			if( st->sleep.timer != INVALID_TIMER )
				delete_timer(st->sleep.timer, run_script_timer);
			script_free_stack(st->stack);
			aFree(st);
			n = n->next;
		}
		linkdb_final(&sleep_db);
	}

	mapreg_reload();
	return 0;
}


//-----------------------------------------------------------------------------
// buildin functions
//

#define BUILDIN_DEF(x,args) { buildin_ ## x , #x , args }
#define BUILDIN_DEF2(x,x2,args) { buildin_ ## x , x2 , args }
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
	TBL_PC* sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	clif_scriptmes(sd, st->oid, script_getstr(st, 2));
	return 0;
}

/// Displays the button 'next' in the npc dialog.
/// The dialog text is cleared and the script continues when the button is pressed.
///
/// next;
BUILDIN_FUNC(next)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	st->state = STOP;
	clif_scriptnext(sd, st->oid);
	return 0;
}

/// Ends the script and displays the button 'close' on the npc dialog.
/// The dialog is closed when the button is pressed.
///
/// close;
BUILDIN_FUNC(close)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	st->state = END;
	clif_scriptclose(sd, st->oid);
	return 0;
}

/// Displays the button 'close' on the npc dialog.
/// The dialog is closed and the script continues when the button is pressed.
///
/// close2;
BUILDIN_FUNC(close2)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	st->state = STOP;
	clif_scriptclose(sd, st->oid);
	return 0;
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

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	// TODO detect multiple scripts waiting for input at the same time, and what to do when that happens
	if( sd->state.menu_or_input == 0 )
	{
		struct StringBuf buf;
		struct script_data* data;

		if( script_lastdata(st) % 2 == 0 )
		{// argument count is not even (1st argument is at index 2)
			ShowError("script:menu: illegal number of arguments (%d).\n", (script_lastdata(st) - 1));
			st->state = END;
			return 1;
		}

		StringBuf_Init(&buf);
		sd->npc_menu = 0;
		for( i = 2; i < script_lastdata(st); i += 2 )
		{
			// menu options
			text = script_getstr(st, i);

			// target label
			data = script_getdata(st, i+1);
			if( !data_islabel(data) )
			{// not a label
				StringBuf_Destroy(&buf);
				ShowError("script:menu: argument #%d (from 1) is not a label or label not found.\n", i);
				script_reportdata(data);
				st->state = END;
				return 1;
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
		clif_scriptmenu(sd, st->oid, StringBuf_Value(&buf));
		StringBuf_Destroy(&buf);
		//TODO what's the maximum number of options that can be displayed and/or received? -> give warning
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
			ShowDebug("script:menu: unexpected selection (%d)\n", sd->npc_menu);
			st->state = END;
			return 1;
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
			ShowDebug("script:menu: selection is out of range (%d pairs are missing?) - please report this\n", sd->npc_menu);
			st->state = END;
			return 1;
		}
		if( !data_islabel(script_getdata(st, i + 1)) )
		{// TODO remove this temporary crash-prevention code (fallback for multiple scripts requesting user input)
			ShowError("script:menu: unexpected data in label argument\n");
			script_reportdata(script_getdata(st, i + 1));
			st->state = END;
			return 1;
		}
		pc_setreg(sd, add_str("@menu"), menu);
		st->pos = script_getnum(st, i + 1);
		st->state = GOTO;
	}
	return 0;
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

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

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
		clif_scriptmenu(sd, st->oid, StringBuf_Value(&buf));
		StringBuf_Destroy(&buf);
	}
	else if( sd->npc_menu == 0xff )
	{// Cancel was pressed
		sd->state.menu_or_input = 0;
		st->state = END;
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
	}
	return 0;
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

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

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
		clif_scriptmenu(sd, st->oid, StringBuf_Value(&buf));
		StringBuf_Destroy(&buf);
	}
	else if( sd->npc_menu == 0xff )
	{// Cancel was pressed
		sd->state.menu_or_input = 0;
		pc_setreg(sd, add_str("@menu"), 0xff);
		script_pushint(st, 0xff);
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
	}
	return 0;
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
		ShowError("script:goto: not a label\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return 1;
	}

	st->pos = script_getnum(st,2);
	st->state = GOTO;
	return 0;
}

/*==========================================
 * user-defined function call
 *------------------------------------------*/
BUILDIN_FUNC(callfunc)
{
	int i, j;
	struct linkdb_node** oldval;
	struct script_code *scr, *oldscr;
	const char* str = script_getstr(st,2);

	scr = (struct script_code*)strdb_get(userfunc_db, str);
	if( !scr )
	{
		ShowError("script:callfunc: function not found! [%s]\n", str);
		st->state = END;
		return 1;
	}

	for( i = st->start+3, j = 0; i < st->end; i++, j++ )
		push_copy(st->stack,i);

	script_pushint(st,j);                            // push argument count
	script_pushint(st,st->stack->defsp);             // push current stack pointer
	script_pushint(st,(int)st->script);              // push current script
	script_pushint(st,(int)st->stack->var_function); // push function-dependent variables
	push_val(st->stack,C_RETINFO,st->pos);           // push current script location

	oldscr = st->script;
	oldval = st->stack->var_function;

	st->pos = 0;
	st->script = scr;
	st->stack->defsp = st->start+5+j;
	st->state = GOTO;
	st->stack->var_function = (struct linkdb_node**)aCalloc(1, sizeof(struct linkdb_node*));

	for( i = 0; i < j; i++ )
	{
		struct script_data* s = script_getdatatop(st, -6-i);
		if( data_isreference(s) && !s->ref )
		{
			const char* name = reference_getname(s);
			if( name[0] == '.' && name[1] == '@' )
				s->ref = oldval;
			else if( name[0] == '.' )
				s->ref = &oldscr->script_vars;
		}
	}

	return 0;
}
/*==========================================
 * subroutine call
 *------------------------------------------*/
BUILDIN_FUNC(callsub)
{
	int i,j;
	struct linkdb_node** oldval;
	int pos = script_getnum(st,2);

	if( !data_islabel(script_getdata(st,2)) && !data_isfunclabel(script_getdata(st,2)) )
	{
		ShowError("script:callsub: argument is not a label\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return 1;
	}

	oldval = st->stack->var_function;

	for( i = st->start+3, j = 0; i < st->end; i++, j++ )
		push_copy(st->stack,i);

	script_pushint(st,j);                            // push argument count
	script_pushint(st,st->stack->defsp);             // push current stack pointer
	script_pushint(st,(int)st->script);              // push current script
	script_pushint(st,(int)st->stack->var_function); // push function-dependent variables
	push_val(st->stack,C_RETINFO,st->pos);           // push current script location

	st->pos = pos;
	st->stack->defsp = st->start+5+j;
	st->state = GOTO;
	st->stack->var_function = (struct linkdb_node**)aCalloc(1, sizeof(struct linkdb_node*));

	for(i = 0; i < j; i++)
	{
		struct script_data* s = script_getdatatop(st, -6-i);
		if( data_isreference(s) && !s->ref )
		{
			const char* name = reference_getname(s);
			if( name[0] == '.' && name[1] == '@' )
				s->ref = oldval;
		}
	}

	return 0;
}

/// Retrieves an argument provided to callfunc/callsub.
/// If the argument doesn't exist
///
/// getarg(<index>{,<default_value>}) -> <value>
BUILDIN_FUNC(getarg)
{
	int idx;
	int count;
	int stsp;

	if( st->stack->defsp < 5 || st->stack->stack_data[st->stack->defsp - 1].type != C_RETINFO )
	{
		ShowError("script:getarg: no callfunc or callsub!\n");
		st->state = END;
		return 1;
	}
	count = conv_num(st, &(st->stack->stack_data[st->stack->defsp - 5]));
	stsp = st->stack->defsp - count - 5;

	idx = script_getnum(st,2);

	if( idx < count )
		push_copy(st->stack, stsp + idx);
	else if( script_hasdata(st,3) )
		script_pushcopy(st, 3);
	else
	{
		ShowError("script:getarg: index (idx=%d) out of range (count=%d) and no default value found\n", idx, count);
		st->state = END;
		return 1;
	}

	return 0;
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
		if( data_isreference(data) )
		{
			const char* name = reference_getname(data);
			if( name[0] == '.' && name[1] == '@' )
			{// temporary script variable, convert to value
				get_val(st, data);
				if( data_isstring(data) )
				{// duplicate the string
					data->type = C_STR;
					data->u.str = aStrdup(data->u.str);
				}
			}
			else if( name[0] == '.' && !data->ref )
			{// script variable, link to current script
				data->ref = &st->script->script_vars;
			}
		}
	}
	else
	{// no return value
		script_pushnil(st);
	}
	st->state = RETFUNC;
	return 0;
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
	int max;

	if( script_hasdata(st,3) )
	{// min,max
		min = script_getnum(st,2);
		max = script_getnum(st,3);
		if( max < min )
			swap(min, max);
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
		script_pushint(st, rand()%range + min);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(warp)
{
	int ret;
	int x,y;
	const char* str;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	str = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);

	if(strcmp(str,"Random")==0)
		ret = pc_randomwarp(sd,3);
	else if(strcmp(str,"SavePoint")==0 || strcmp(str,"Save")==0)
		ret = pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
	else
		ret = pc_setpos(sd,mapindex_name2id(str),x,y,0);

	if( ret ) {
		ShowError("buildin_warp: moving player '%s' to \"%s\",%d,%d failed.\n", sd->status.name, str, x, y);
		script_reportsrc(st);
	}

	return 0;
}
/*==========================================
 * エリア指定ワープ
 *------------------------------------------*/
static int buildin_areawarp_sub(struct block_list *bl,va_list ap)
{
	int x,y;
	unsigned int map;
	map=va_arg(ap, unsigned int);
	x=va_arg(ap,int);
	y=va_arg(ap,int);
	if(map == 0)
		pc_randomwarp((TBL_PC *)bl,3);
	else
		pc_setpos((TBL_PC *)bl,map,x,y,0);
	return 0;
}
BUILDIN_FUNC(areawarp)
{
	int x,y,m;
	unsigned int index;
	const char *str;
	const char *mapname;
	int x0,y0,x1,y1;

	mapname=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	str=script_getstr(st,7);
	x=script_getnum(st,8);
	y=script_getnum(st,9);

	if( (m=map_mapname2mapid(mapname))< 0)
		return 0;

	if(strcmp(str,"Random")==0)
		index = 0;
	else if(!(index=mapindex_name2id(str)))
		return 0;

	map_foreachinarea(buildin_areawarp_sub, m,x0,y0,x1,y1,BL_PC, index,x,y);
	return 0;
}

/*==========================================
 * warpchar [LuzZza]
 * Useful for warp one player from 
 * another player npc-session.
 * Using: warpchar "mapname",x,y,Char_ID;
 *------------------------------------------*/
BUILDIN_FUNC(warpchar)
{
	int x,y,a;
	const char *str;
	TBL_PC *sd;
	
	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	a=script_getnum(st,5);

	sd = map_charid2sd(a);
	if( sd == NULL )
		return 0;

	if(strcmp(str, "Random") == 0)
		pc_randomwarp(sd, 3);
	else
	if(strcmp(str, "SavePoint") == 0)
		pc_setpos(sd, sd->status.save_point.map,sd->status.save_point.x, sd->status.save_point.y, 3);
	else	
		pc_setpos(sd, mapindex_name2id(str), x, y, 3);
	
	return 0;
} 
 
/*==========================================
 * Warpparty - [Fredzilla]
 * Syntax: warpparty "mapname",x,y,Party_ID;
 *------------------------------------------*/
BUILDIN_FUNC(warpparty)
{
	int x,y;
	const char *str;
	int p_id;
	int i;
	unsigned short mapindex;
	TBL_PC *pl_sd;
	struct party_data *p=NULL;
	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	p_id=script_getnum(st,5);
	if(p_id < 1)
		return 0;
	p = party_search(p_id);
	if (!p)
		return 0;
	if(strcmp(str,"Random")==0)
	{
		for (i = 0; i < MAX_PARTY; i++)
		{
			if ((pl_sd = p->data[i].sd))
			{
				if(map[pl_sd->bl.m].flag.nowarp)
					continue;
				pc_randomwarp(pl_sd,3);
			}
		}
	}
	else if(strcmp(str,"SavePointAll")==0)
	{
		for (i = 0; i < MAX_PARTY; i++)
		{
			if ((pl_sd = p->data[i].sd))
			{
				if(map[pl_sd->bl.m].flag.noreturn)
					continue;
				pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,3);
			}
		}
	}
	else if(strcmp(str,"SavePoint")==0)
	{
		pl_sd=script_rid2sd(st);
		if (!pl_sd) return 0;
	
		mapindex=pl_sd->status.save_point.map;
		x=pl_sd->status.save_point.x;
		y=pl_sd->status.save_point.y;
		
		for (i = 0; i < MAX_PARTY; i++)
		{
			if ((pl_sd = p->data[i].sd))
			{
				if(map[pl_sd->bl.m].flag.noreturn)
					continue;			
				pc_setpos(pl_sd,mapindex,x,y,3);
			}
		}
	}
	else if(strcmp(str,"Leader")==0)
	{
		for(i = 0; i < MAX_PARTY && !p->party.member[i].leader; i++);
		if (i == MAX_PARTY || !p->data[i].sd) //Leader not found / not online
			return 0;
		if(map[p->data[i].sd->bl.m].flag.nowarpto)
			return 0;
		mapindex = p->data[i].sd->mapindex;
		x = p->data[i].sd->bl.x;
		y = p->data[i].sd->bl.y;
		for (i = 0; i < MAX_PARTY; i++)
		{
			pl_sd = p->data[i].sd;
			if (!pl_sd)
				continue;
			if(map[pl_sd->bl.m].flag.noreturn || map[pl_sd->bl.m].flag.nowarp)
				continue;
			pc_setpos(pl_sd,mapindex,x,y,3);
		}
	}
	else
	{
		mapindex = mapindex_name2id(str);
		if (!mapindex) //Show source of npc error.
			return 1;
		for (i = 0; i < MAX_PARTY; i++)
		{
			if ((pl_sd = p->data[i].sd))
			{
				if(map[pl_sd->bl.m].flag.noreturn || map[pl_sd->bl.m].flag.nowarp)
					continue;
				pc_setpos(pl_sd,mapindex,x,y,3);
			}
		}
	}
	return 0;
}
/*==========================================
 * Warpguild - [Fredzilla]
 * Syntax: warpguild "mapname",x,y,Guild_ID;
 *------------------------------------------*/
BUILDIN_FUNC(warpguild)
{
	TBL_PC *sd;
	TBL_PC *pl_sd;
	struct guild* g;
	struct s_mapiterator* iter;
	int type;

	const char* str = script_getstr(st,2);
	int x           = script_getnum(st,3);
	int y           = script_getnum(st,4);
	int gid         = script_getnum(st,5);

	sd=script_rid2sd(st);
	if( sd == NULL )
		return 0;
	g = guild_search(gid);
	if( g == NULL )
		return 0;
	
	if(map[sd->bl.m].flag.noreturn || map[sd->bl.m].flag.nowarpto)
		return 0;
	
	type = ( strcmp(str,"Random")==0 ) ? 0
	     : ( strcmp(str,"SavePointAll")==0 ) ? 1
		 : ( strcmp(str,"SavePoint")==0 ) ? 2
		 : 3;

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if( pl_sd->status.guild_id != gid )
			continue;

		switch( type )
		{
		case 0: // Random
			if(!map[pl_sd->bl.m].flag.nowarp)
				pc_randomwarp(pl_sd,3);
		break;
		case 1: // SavePointAll
			if(!map[pl_sd->bl.m].flag.noreturn)
				pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,3);
		break;
		case 2: // SavePoint
			if(!map[pl_sd->bl.m].flag.noreturn)
				pc_setpos(pl_sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
		break;
		case 3: // m,x,y
			if(!map[pl_sd->bl.m].flag.noreturn && !map[pl_sd->bl.m].flag.nowarp)
				pc_setpos(pl_sd,mapindex_name2id(str),x,y,3);
		break;
		}
	}
	mapit_free(iter);

	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(heal)
{
	TBL_PC *sd;
	int hp,sp;
	
	sd = script_rid2sd(st);
	if (!sd) return 0;
	
	hp=script_getnum(st,2);
	sp=script_getnum(st,3);
	status_heal(&sd->bl, hp, sp, 1);
	return 0;
}
/*==========================================
 *
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
		return 0;
	}
	
	sd = script_rid2sd(st);
	if (!sd) return 0;
	pc_itemheal(sd,sd->itemid,hp,sp);
	return 0;
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
		return 0;
	}

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pc_percentheal(sd,hp,sp);
	return 0;
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
		
		sd = script_rid2sd(st);
		if( sd == NULL )
			return 0;

		pc_jobchange(sd, job, upper);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(jobname)
{
	int class_=script_getnum(st,2);
	script_pushconststr(st,job_name(class_));
	return 0;
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
	int uid;
	const char* name;
	int min;
	int max;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	data = script_getdata(st,2);
	if( !data_isreference(data) ){
		ShowError("script:input: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return 1;
	}
	uid = reference_getuid(data);
	name = reference_getname(data);
	min = (script_hasdata(st,3) ? script_getnum(st,3) : script_config.input_min_value);
	max = (script_hasdata(st,4) ? script_getnum(st,4) : script_config.input_max_value);

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
			size_t len = strlen(sd->npc_str);
			set_reg(st, sd, uid, name, (void*)sd->npc_str, script_getref(st,2));
			script_pushint(st, (len > max ? 1 : len < min ? -1 : 0));
		}
		else
		{
			int amount = sd->npc_amount;
			set_reg(st, sd, uid, name, (void*)cap_value(amount,min,max), script_getref(st,2));
			script_pushint(st, (amount > max ? 1 : amount < min ? -1 : 0));
		}
	}
	return 0;
}

/// Sets the value of a variable.
/// The value is converted to the type of the variable.
///
/// set(<variable>,<value>) -> <variable>
BUILDIN_FUNC(set)
{
	TBL_PC* sd = NULL;
	struct script_data* data;
	int num;
	const char* name;
	char prefix;

	data = script_getdata(st,2);
	if( !data_isreference(data) )
	{
		ShowError("script:set: not a variable\n");
		script_reportdata(script_getdata(st,2));
		st->state = END;
		return 1;
	}

	num = reference_getuid(data);
	name = reference_getname(data);
	prefix = *name;

	if( not_server_variable(prefix) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
		{
			ShowError("script:set: no player attached for player variable '%s'\n", name);
			return 0;
		}
	}

	if( is_string_variable(name) )
		set_reg(st,sd,num,name,(void*)script_getstr(st,3),script_getref(st,2));
	else
		set_reg(st,sd,num,name,(void*)script_getnum(st,3),script_getref(st,2));

	// return a copy of the variable reference
	script_pushcopy(st,2);

	return 0;
}

/////////////////////////////////////////////////////////////////////
/// Array variables
///

/// Returns the size of the specified array
static int32 getarraysize(struct script_state* st, int32 id, int32 idx, int isstring, struct linkdb_node** ref)
{
	int32 ret = idx;

	if( isstring )
	{
		for( ; idx < 128; ++idx )
		{
			char* str = (char*)get_val2(st, reference_uid(id, idx), ref);
			if( str && *str )
				ret = idx + 1;
			script_removetop(st, -1, 0);
		}
	}
	else
	{
		for( ; idx < 128; ++idx )
		{
			int32 num = (int32)get_val2(st, reference_uid(id, idx), ref);
			if( num )
				ret = idx + 1;
			script_removetop(st, -1, 0);
		}
	}
	return ret;
}

/// Sets values of an array, from the starting index.
/// ex: setarray arr[1],1,2,3;
///
/// setarray <array variable>,<value1>{,<value2>...};
BUILDIN_FUNC(setarray)
{
	struct script_data* data;
	const char* name;
	int32 start;
	int32 end;
	int32 id;
	int32 i;
	TBL_PC* sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:setarray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return 1;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:setarray: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return 1;// not supported
	}

	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return 0;// no player attached
	}

	end = start + script_lastdata(st) - 2;
	if( end > 127 )
		end = 127;

	if( is_string_variable(name) )
	{// string array
		for( i = 3; start < end; ++start, ++i )
			set_reg(st, sd, reference_uid(id, start), name, (void*)script_getstr(st,i), reference_getref(data));
	}
	else
	{// int array
		for( i = 3; start < end; ++start, ++i )
			set_reg(st, sd, reference_uid(id, start), name, (void*)script_getnum(st,i), reference_getref(data));
	}
	return 0;
}

/// Sets count values of an array, from the starting index.
/// ex: cleararray arr[0],0,1;
///
/// cleararray <array variable>,<value>,<count>;
BUILDIN_FUNC(cleararray)
{
	struct script_data* data;
	const char* name;
	int32 start;
	int32 end;
	int32 id;
	void* v;
	TBL_PC* sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:cleararray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return 1;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:cleararray: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return 1;// not supported
	}

	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return 0;// no player attached
	}

	if( is_string_variable(name) )
		v = (void*)script_getstr(st, 3);
	else
		v = (void*)script_getnum(st, 3);

	end = start + script_getnum(st, 4);
	if( end > 127 )
		end = 127;

	for( ; start <= end; ++start )
		set_reg(st, sd, reference_uid(id, start), name, v, script_getref(st,2));
	return 0;
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
	int32 count;
	TBL_PC* sd = NULL;

	data1 = script_getdata(st, 2);
	data2 = script_getdata(st, 3);
	if( !data_isreference(data1) || !data_isreference(data2) )
	{
		ShowError("script:copyarray: not a variable\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return 1;// not a variable
	}

	id1 = reference_getid(data1);
	id2 = reference_getid(data2);
	idx1 = reference_getindex(data1);
	idx2 = reference_getindex(data2);
	name1 = reference_getname(data1);
	name2 = reference_getname(data2);
	if( not_array_variable(*name1) || not_array_variable(*name2) )
	{
		ShowError("script:copyarray: illegal scope\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return 1;// not supported
	}

	if( is_string_variable(name1) != is_string_variable(name2) )
	{
		ShowError("script:copyarray: type mismatch\n");
		script_reportdata(data1);
		script_reportdata(data2);
		st->state = END;
		return 1;// data type mismatch
	}

	if( not_server_variable(*name1) || not_server_variable(*name2) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return 0;// no player attached
	}

	count = script_getnum(st, 4);
	if( count > 128 - idx1 )
		count = 128 - idx1;
	if( count <= 0 || (id1 == id2 && idx1 == idx2) )
		return 0;// nothing to copy

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
			if( idx2 + i < 128 )
			{
				v = get_val2(st, reference_uid(id2, idx2 + i), reference_getref(data2));
				set_reg(st, sd, reference_uid(id1, idx1 + i), name1, v, reference_getref(data1));
				script_removetop(st, -1, 0);
			}
			else// out of range - assume ""/0
				set_reg(st, sd, reference_uid(id1, idx1 + i), name1, (is_string_variable(name1)?(void*)"":(void*)0), reference_getref(data1));
		}
	}
	return 0;
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

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getarraysize: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return 1;// not a variable
	}

	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:getarraysize: illegal scope\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return 1;// not supported
	}

	script_pushint(st, getarraysize(st, reference_getid(data), reference_getindex(data), is_string_variable(name), reference_getref(data)));
	return 0;
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
	int start;
	int end;
	int id;
	TBL_PC *sd = NULL;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:deletearray: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return 1;// not a variable
	}

	id = reference_getid(data);
	start = reference_getindex(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:deletearray: illegal scope\n");
		script_reportdata(data);
		st->state = END;
		return 1;// not supported
	}

	if( not_server_variable(*name) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
			return 0;// no player attached
	}

	end = getarraysize(st, id, start, is_string_variable(name), reference_getref(data));
	if( start >= end )
		return 0;// nothing to free

	if( script_hasdata(st,3) )
	{
		int count = script_getnum(st, 3);
		if( count > end - start )
			count = end - start;
		if( count <= 0 )
			return 0;// nothing to free

		// move rest of the elements backward
		for( ; start + count < end; ++start )
		{
			void* v = get_val2(st, reference_uid(id, start + count), reference_getref(data));
			set_reg(st, sd, reference_uid(id, start), name, v, reference_getref(data));
			script_removetop(st, -1, 0);
		}
	}

	// clear the rest of the array
	if( is_string_variable(name) )
	{
		for( ; start < end; ++start )
			set_reg(st, sd, reference_uid(id, start), name, (void *)"", reference_getref(data));
	}
	else 
	{
		for( ; start < end; ++start )
			set_reg(st, sd, reference_uid(id, start), name, (void*)0, reference_getref(data));
	}
	return 0;
}

/// Returns a reference to the target index of the array variable.
/// Equivalent to var[index].
///
/// getelementofarray(<array variable>,<index>) -> <variable reference>
BUILDIN_FUNC(getelementofarray)
{
	struct script_data* data;
	const char* name;
	int32 id;
	int i;

	data = script_getdata(st, 2);
	if( !data_isreference(data) )
	{
		ShowError("script:getelementofarray: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return 1;// not a variable
	}

	id = reference_getid(data);
	name = reference_getname(data);
	if( not_array_variable(*name) )
	{
		ShowError("script:getelementofarray: illegal scope\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return 1;// not supported
	}

	i = script_getnum(st, 3);
	if( i < 0 || i >= 128 )
	{
		ShowWarning("script:getelementofarray: index out of range (%d)\n", i);
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return 1;// out of range
	}

	push_val2(st->stack, C_NAME, reference_uid(id, i), reference_getref(data));
	return 0;
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

	type=script_getnum(st,2);
	val=script_getnum(st,3);

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pc_changelook(sd,type,val);

	return 0;
}

BUILDIN_FUNC(changelook)
{ // As setlook but only client side
	int type,val;
	TBL_PC* sd;

	type=script_getnum(st,2);
	val=script_getnum(st,3);

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	clif_changelook(&sd->bl,type,val);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(cutin)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	clif_cutin(sd,script_getstr(st,2),script_getnum(st,3));
	return 0;
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
	
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	clif_viewpoint(sd,st->oid,type,x,y,id,color);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(countitem)
{
	int nameid, i;
	int count = 0;
	struct script_data* data;

	TBL_PC* sd = script_rid2sd(st);
	if (!sd) {
		script_pushint(st,0);
		return 0;
	}

	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ) {
		const char* name = conv_str(st,data);
		struct item_data* item_data;
		if((item_data = itemdb_searchname(name)) != NULL)
			nameid = item_data->nameid;
		else
			nameid = 0;
	} else
		nameid = conv_num(st,data);

	if (nameid < 500) {
		ShowError("wrong item ID : countitem(%i)\n", nameid);
		script_reportsrc(st);
		script_pushint(st,0);
		return 1;
	}

	for(i = 0; i < MAX_INVENTORY; i++)
		if(sd->status.inventory[i].nameid == nameid)
			count += sd->status.inventory[i].amount;

	script_pushint(st,count);
	return 0;
}

/*==========================================
 * countitem2(nameID,Identified,Refine,Attribute,Card0,Card1,Card2,Card3)	[Lupus]
 *	returns number of items that meet the conditions
 *------------------------------------------*/
BUILDIN_FUNC(countitem2)
{
	int nameid, iden, ref, attr, c1, c2, c3, c4;
	int count = 0;
	int i;	
	struct script_data* data;
	
	TBL_PC* sd = script_rid2sd(st);
	if (!sd) {
		script_pushint(st,0);
		return 0;
	}
	
	data = script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ) {
		const char* name = conv_str(st,data);
		struct item_data* item_data;
		if((item_data = itemdb_searchname(name)) != NULL)
			nameid = item_data->nameid;
		else
			nameid = 0;
	} else
		nameid = conv_num(st,data);
	
	iden = script_getnum(st,3);
	ref  = script_getnum(st,4);
	attr = script_getnum(st,5);
	c1 = (short)script_getnum(st,6);
	c2 = (short)script_getnum(st,7);
	c3 = (short)script_getnum(st,8);
	c4 = (short)script_getnum(st,9);
	
	if (nameid < 500) {
		ShowError("wrong item ID : countitem2(%i)\n", nameid);
		script_pushint(st,0);
		return 1;
	}
	
	for(i = 0; i < MAX_INVENTORY; i++)
		if (sd->status.inventory[i].nameid > 0 && sd->inventory_data[i] != NULL &&
			sd->status.inventory[i].amount > 0 && sd->status.inventory[i].nameid == nameid &&
			sd->status.inventory[i].identify == iden && sd->status.inventory[i].refine == ref &&
			sd->status.inventory[i].attribute == attr && sd->status.inventory[i].card[0] == c1 &&
			sd->status.inventory[i].card[1] == c2 && sd->status.inventory[i].card[2] == c3 &&
			sd->status.inventory[i].card[3] == c4
		)
			count += sd->status.inventory[i].amount;

	script_pushint(st,count);
	return 0;
}

/*==========================================
 * 重量チェック
 *------------------------------------------*/
BUILDIN_FUNC(checkweight)
{
	int nameid=0,amount,i;
	unsigned long weight;
	TBL_PC *sd;
	struct script_data *data;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,data);

	amount=script_getnum(st,3);
	if ( amount<=0 || nameid<500 ) { //if get wrong item ID or amount<=0, don't count weight of non existing items
		script_pushint(st,0);
		ShowError("buildin_checkweight: Wrong item ID or amount.\n");
		return 1;
	}

	weight = itemdb_weight(nameid)*amount;
	if(amount > MAX_AMOUNT || weight + sd->weight > sd->max_weight){
		script_pushint(st,0);
	} else { 
		//Check if the inventory ain't full.
		//TODO: Currently does not checks if you can just stack it on top of another item you already have....

		i = pc_search_inventory(sd,0);
		if (i >= 0) //Empty slot available.
			script_pushint(st,1);
		else //Inventory full
			script_pushint(st,0);
			
	}

	return 0;
}

/*==========================================
 * getitem <item id>,<amount>{,<character ID>};
 * getitem "<item name>",<amount>{,<character ID>};
 *------------------------------------------*/
BUILDIN_FUNC(getitem)
{
	int nameid,amount,get_count,i,flag = 0;
	struct item it;
	TBL_PC *sd;
	struct script_data *data;

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) )
	{// "<item name>"
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data == NULL ){
			ShowError("buildin_getitem: Nonexistant item %s requested.\n", name);
			return 1; //No item created.
		}
		nameid=item_data->nameid;
	} else if( data_isint(data) )
	{// <item id>
		nameid=conv_num(st,data);
		//Violet Box, Blue Box, etc - random item pick
		if( nameid < 0 ) {
			nameid=itemdb_searchrandomid(-nameid);
			flag = 1;
		}
		if( nameid <= 0 || !itemdb_exists(nameid) ){
			ShowError("buildin_getitem: Nonexistant item %d requested.\n", nameid);
			return 1; //No item created.
		}
	} else {
		ShowError("buildin_getitem: invalid data type for argument #1 (%d).", data->type);
		return 1;
	}

	// <amount>
	if( (amount=script_getnum(st,3)) <= 0)
		return 0; //return if amount <=0, skip the useles iteration

	memset(&it,0,sizeof(it));
	it.nameid=nameid;
	if(!flag)
		it.identify=1;
	else
		it.identify=itemdb_isidentified(nameid);

	if( script_hasdata(st,4) )
		sd=map_id2sd(script_getnum(st,4)); // <Account ID>
	else
		sd=script_rid2sd(st); // Attached player

	if( sd == NULL ) // no target
		return 0;

	//Check if it's stackable.
	if (!itemdb_isstackable(nameid))
		get_count = 1;
	else
		get_count = amount;

	for (i = 0; i < amount; i += get_count)
	{
		// if not pet egg
		if (!pet_create_egg(sd, nameid))
		{
			if ((flag = pc_additem(sd, &it, get_count)))
			{
				clif_additem(sd, 0, 0, flag);
				if( pc_candrop(sd,&it) )
					map_addflooritem(&it,get_count,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
                }
        }

	//Logs items, got from (N)PC scripts [Lupus]
	if(log_config.enable_logs&LOG_SCRIPT_TRANSACTIONS)
		log_pick_pc(sd, "N", nameid, amount, NULL);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(getitem2)
{
	int nameid,amount,get_count,i,flag = 0;
	int iden,ref,attr,c1,c2,c3,c4;
	struct item_data *item_data;
	struct item item_tmp;
	TBL_PC *sd;
	struct script_data *data;

	if( script_hasdata(st,11) )
		sd=map_id2sd(script_getnum(st,11)); // <Account ID>
	else
		sd=script_rid2sd(st); // Attached player

	if( sd == NULL ) // no target
		return 0;

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid=item_data->nameid;
		else
			nameid=UNKNOWN_ITEM_ID;
	}else
		nameid=conv_num(st,data);

	amount=script_getnum(st,3);
	iden=script_getnum(st,4);
	ref=script_getnum(st,5);
	attr=script_getnum(st,6);
	c1=script_getnum(st,7);
	c2=script_getnum(st,8);
	c3=script_getnum(st,9);
	c4=script_getnum(st,10);

	if(nameid<0) { // ランダム
		nameid=itemdb_searchrandomid(-nameid);
		flag = 1;
	}

	if(nameid > 0) {
		memset(&item_tmp,0,sizeof(item_tmp));
		item_data=itemdb_exists(nameid);
		if (item_data == NULL)
			return -1;
		if(item_data->type==IT_WEAPON || item_data->type==IT_ARMOR){
			if(ref > 10) ref = 10;
		}
		else if(item_data->type==IT_PETEGG) {
			iden = 1;
			ref = 0;
		}
		else {
			iden = 1;
			ref = attr = 0;
		}

		item_tmp.nameid=nameid;
		if(!flag)
			item_tmp.identify=iden;
		else if(item_data->type==IT_WEAPON || item_data->type==IT_ARMOR)
			item_tmp.identify=0;
		item_tmp.refine=ref;
		item_tmp.attribute=attr;
		item_tmp.card[0]=c1;
		item_tmp.card[1]=c2;
		item_tmp.card[2]=c3;
		item_tmp.card[3]=c4;

		//Check if it's stackable.
		if (!itemdb_isstackable(nameid))
			get_count = 1;
		else
			get_count = amount;

		for (i = 0; i < amount; i += get_count)
		{
			// if not pet egg
			if (!pet_create_egg(sd, nameid))
			{
				if ((flag = pc_additem(sd, &item_tmp, get_count)))
				{
					clif_additem(sd, 0, 0, flag);
					if( pc_candrop(sd,&item_tmp) )
						map_addflooritem(&item_tmp,get_count,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
				}
			}
		}

		//Logs items, got from (N)PC scripts [Lupus]
		if(log_config.enable_logs&0x40)
			log_pick_pc(sd, "N", nameid, amount, &item_tmp);
	}

	return 0;
}

/*==========================================
 * gets an item with someone's name inscribed [Skotlex]
 * getinscribeditem item_num, character_name
 * Returned Qty is always 1, only works on equip-able
 * equipment
 *------------------------------------------*/
BUILDIN_FUNC(getnameditem)
{
	int nameid;
	struct item item_tmp;
	TBL_PC *sd, *tsd;
	struct script_data *data;

	sd = script_rid2sd(st);
	if (sd == NULL)
	{	//Player not attached!
		script_pushint(st,0);
		return 0; 
	}
	
	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data == NULL)
		{	//Failed
			script_pushint(st,0);
			return 0;
		}
		nameid = item_data->nameid;
	}else
		nameid = conv_num(st,data);

	if(!itemdb_exists(nameid)/* || itemdb_isstackable(nameid)*/)
	{	//Even though named stackable items "could" be risky, they are required for certain quests.
		script_pushint(st,0);
		return 0;
	}

	data=script_getdata(st,3);
	get_val(st,data);
	if( data_isstring(data) )	//Char Name
		tsd=map_nick2sd(conv_str(st,data));
	else	//Char Id was given
		tsd=map_charid2sd(conv_num(st,data));
	
	if( tsd == NULL )
	{	//Failed
		script_pushint(st,0);
		return 0;
	}

	memset(&item_tmp,0,sizeof(item_tmp));
	item_tmp.nameid=nameid;
	item_tmp.amount=1;
	item_tmp.identify=1;
	item_tmp.card[0]=CARD0_CREATE; //we don't use 255! because for example SIGNED WEAPON shouldn't get TOP10 BS Fame bonus [Lupus]
	item_tmp.card[2]=tsd->status.char_id;
	item_tmp.card[3]=tsd->status.char_id >> 16;
	if(pc_additem(sd,&item_tmp,1)) {
		script_pushint(st,0);
		return 0;	//Failed to add item, we will not drop if they don't fit
	}

	//Logs items, got from (N)PC scripts [Lupus]
	if(log_config.enable_logs&0x40)
		log_pick_pc(sd, "N", item_tmp.nameid, item_tmp.amount, &item_tmp);

	script_pushint(st,1);
	return 0;
}

/*==========================================
 * gets a random item ID from an item group [Skotlex]
 * groupranditem group_num
 *------------------------------------------*/
BUILDIN_FUNC(grouprandomitem)
{
	int group;

	group = script_getnum(st,2);
	script_pushint(st,itemdb_searchrandomid(group));
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(makeitem)
{
	int nameid,amount,flag = 0;
	int x,y,m;
	const char *mapname;
	struct item item_tmp;
	struct script_data *data;

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		if( item_data )
			nameid=item_data->nameid;
		else
			nameid=UNKNOWN_ITEM_ID;
	}else
		nameid=conv_num(st,data);

	amount=script_getnum(st,3);
	mapname	=script_getstr(st,4);
	x	=script_getnum(st,5);
	y	=script_getnum(st,6);

	if(strcmp(mapname,"this")==0)
	{
		TBL_PC *sd;
		sd = script_rid2sd(st);
		if (!sd) return 0; //Failed...
		m=sd->bl.m;
	} else
		m=map_mapname2mapid(mapname);

	if(nameid<0) { // ランダム
		nameid=itemdb_searchrandomid(-nameid);
		flag = 1;
	}

	if(nameid > 0) {
		memset(&item_tmp,0,sizeof(item_tmp));
		item_tmp.nameid=nameid;
		if(!flag)
			item_tmp.identify=1;
		else
			item_tmp.identify=itemdb_isidentified(nameid);

		map_addflooritem(&item_tmp,amount,m,x,y,0,0,0,0);
	}

	return 0;
}
/*==========================================
 * script DELITEM command (fixed 2 bugs by Lupus, added deletion priority by Lupus)
 *------------------------------------------*/
BUILDIN_FUNC(delitem)
{
	int nameid=0,amount,i,important_item=0;
	TBL_PC *sd;
	struct script_data *data;

	if( script_hasdata(st,4) )
		sd=map_id2sd(script_getnum(st,4)); // <Account ID>
	else
		sd=script_rid2sd(st); // Attached player

	if( sd == NULL ) // no target
		return 0;

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		//nameid=UNKNOWN_ITEM_ID;
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,data);

	amount=script_getnum(st,3);

	if (nameid<500 || amount<=0 ) {//by Lupus. Don't run FOR if u got wrong item ID or amount<=0
		//eprintf("wrong item ID or amount<=0 : delitem %i,\n",nameid,amount);
		return 0;
	}
	//1st pass
	//here we won't delete items with CARDS, named items but we count them
	for(i=0;i<MAX_INVENTORY;i++){
		//we don't delete wrong item or equipped item
		if(sd->status.inventory[i].nameid<=0 || sd->inventory_data[i] == NULL ||
		   sd->status.inventory[i].amount<=0 || sd->status.inventory[i].nameid!=nameid)
			continue;
		//1 egg uses 1 cell in the inventory. so it's ok to delete 1 pet / per cycle
		if(sd->inventory_data[i]->type==IT_PETEGG &&
			sd->status.inventory[i].card[0] == CARD0_PET)
		{
			if (!intif_delete_petdata(MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2])))
				continue; //pet couldn't be sent for deletion.
		} else
		//is this item important? does it have cards? or Player's name? or Refined/Upgraded
		if(itemdb_isspecial(sd->status.inventory[i].card[0]) ||
			sd->status.inventory[i].card[0] ||
		  	sd->status.inventory[i].refine) {
			//this is important item, count it (except for pet eggs)
			if(sd->status.inventory[i].card[0] != CARD0_PET)
				important_item++;
			continue;
		}

		if(sd->status.inventory[i].amount>=amount){

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -amount, &sd->status.inventory[i]);

			pc_delitem(sd,i,amount,0);
			return 0; //we deleted exact amount of items. now exit
		} else {
			amount-=sd->status.inventory[i].amount;

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40) {
				log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -sd->status.inventory[i].amount, &sd->status.inventory[i]);
			}
			//Logs

			pc_delitem(sd,i,sd->status.inventory[i].amount,0);
		}
	}
	//2nd pass
	//now if there WERE items with CARDs/REFINED/NAMED... and if we still have to delete some items. we'll delete them finally
	if (important_item>0 && amount>0)
		for(i=0;i<MAX_INVENTORY;i++){
			//we don't delete wrong item
			if(sd->status.inventory[i].nameid<=0 || sd->inventory_data[i] == NULL ||
				sd->status.inventory[i].amount<=0 || sd->status.inventory[i].nameid!=nameid )
				continue;

			if(sd->status.inventory[i].amount>=amount){

				//Logs items, got from (N)PC scripts [Lupus]
				if(log_config.enable_logs&0x40)
					log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -amount, &sd->status.inventory[i]);

				pc_delitem(sd,i,amount,0);
				return 0; //we deleted exact amount of items. now exit
			} else {
				amount-=sd->status.inventory[i].amount;

				//Logs items, got from (N)PC scripts [Lupus]
				if(log_config.enable_logs&0x40)
					log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -sd->status.inventory[i].amount, &sd->status.inventory[i]);

				pc_delitem(sd,i,sd->status.inventory[i].amount,0);
			}
		}

	return 0;
}

/*==========================================
 * advanced version of delitem [modified by Mihilion]
 *------------------------------------------*/
BUILDIN_FUNC(delitem2)
{
	int nameid=0,amount,i=0;
	int iden,ref,attr,c1,c2,c3,c4;
	TBL_PC *sd;
	struct script_data *data;

	if( script_hasdata(st,11) )
		sd=map_id2sd(script_getnum(st,11)); // <Account ID>
	else
		sd=script_rid2sd(st); // Attached player

	if( sd == NULL ) // no target
		return 0;

	data=script_getdata(st,2);
	get_val(st,data);
	if( data_isstring(data) ){
		const char *name=conv_str(st,data);
		struct item_data *item_data = itemdb_searchname(name);
		//nameid=UNKNOWN_ITEM_ID;
		if( item_data )
			nameid=item_data->nameid;
	}else
		nameid=conv_num(st,data);

	amount=script_getnum(st,3);
	iden=script_getnum(st,4);
	ref=script_getnum(st,5);
	attr=script_getnum(st,6);
	c1=script_getnum(st,7);
	c2=script_getnum(st,8);
	c3=script_getnum(st,9);
	c4=script_getnum(st,10);

	if (!itemdb_exists(nameid) || amount<=0 ) {//by Lupus. Don't run FOR if u got wrong item ID or amount<=0
		 //eprintf("wrong item ID or amount<=0 : delitem %i,\n",nameid,amount);
		 return 0;
	}

	for(i=0;i<MAX_INVENTORY;i++){
	//we don't delete wrong item or equipped item
		if(sd->status.inventory[i].nameid<=0 || sd->inventory_data[i] == NULL ||
			sd->status.inventory[i].amount<=0 || sd->status.inventory[i].nameid!=nameid ||
			sd->status.inventory[i].identify!=iden || sd->status.inventory[i].refine!=ref ||
			sd->status.inventory[i].attribute!=attr || sd->status.inventory[i].card[0]!=c1 ||
			sd->status.inventory[i].card[1]!=c2 || sd->status.inventory[i].card[2]!=c3 ||
			sd->status.inventory[i].card[3]!=c4)
			continue;
	//1 egg uses 1 cell in the inventory. so it's ok to delete 1 pet / per cycle
		if(sd->inventory_data[i]->type==IT_PETEGG && sd->status.inventory[i].card[0] == CARD0_PET)
		{
			if (!intif_delete_petdata( MakeDWord(sd->status.inventory[i].card[1], sd->status.inventory[i].card[2])))
				continue; //Failed to send delete the pet.
		}

		if(sd->status.inventory[i].amount>=amount){

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -amount, &sd->status.inventory[i]);

			pc_delitem(sd,i,amount,0);
			return 0; //we deleted exact amount of items. now exit
		} else {
			amount-=sd->status.inventory[i].amount;

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -sd->status.inventory[i].amount, &sd->status.inventory[i]);

			pc_delitem(sd,i,sd->status.inventory[i].amount,0);
		}
	}
	return 0;
}

/*==========================================
 * Enables/Disables use of items while in an NPC [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(enableitemuse)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	if (sd)
		sd->npc_item_flag = st->oid;
	return 0;
}

BUILDIN_FUNC(disableitemuse)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	if (sd)
		sd->npc_item_flag = 0;
	return 0;
}

/*==========================================
 *キャラ関係のパラメータ取得
 *------------------------------------------*/
BUILDIN_FUNC(readparam)
{
	int type;
	TBL_PC *sd;

	type=script_getnum(st,2);
	if( script_hasdata(st,3) )
		sd=map_nick2sd(script_getstr(st,3));
	else
	sd=script_rid2sd(st);

	if(sd==NULL){
		script_pushint(st,-1);
		return 0;
	}

	script_pushint(st,pc_readparam(sd,type));

	return 0;
}
/*==========================================
 *キャラ関係のID取得
 *------------------------------------------*/
BUILDIN_FUNC(getcharid)
{
	int num;
	TBL_PC *sd;

	num = script_getnum(st,2);
	if( script_hasdata(st,3) )
		sd=map_nick2sd(script_getstr(st,3));
	else
		sd=script_rid2sd(st);

	if(sd==NULL){
		script_pushint(st,0);	//return 0, according docs
		return 0;
	}

	switch( num ) {
	case 0: script_pushint(st,sd->status.char_id); break;
	case 1: script_pushint(st,sd->status.party_id); break;
	case 2: script_pushint(st,sd->status.guild_id); break;
	case 3: script_pushint(st,sd->status.account_id); break;
	default:
		ShowError("buildin_getcharid: invalid parameter (%d).\n", num);
		script_pushint(st,0);
		break;
	}
		
	return 0;
}
/*==========================================
 *指定IDのPT名取得
 *------------------------------------------*/
char *buildin_getpartyname_sub(int party_id)
{
	struct party_data *p;

	p=party_search(party_id);

	if(p!=NULL){
		char *buf;
		buf=(char *)aMallocA(NAME_LENGTH*sizeof(char));
		memcpy(buf, p->party.name, NAME_LENGTH);
		buf[NAME_LENGTH-1] = '\0';
		return buf;
	}

	return 0;
}
BUILDIN_FUNC(getpartyname)
{
	char *name;
	int party_id;

	party_id=script_getnum(st,2);
	name=buildin_getpartyname_sub(party_id);
	if(name != NULL)
		script_pushstr(st,name);
	else
		script_pushconststr(st,"null");

	return 0;
}
/*==========================================
 *指定IDのPT人数とメンバーID取得
 *------------------------------------------*/
BUILDIN_FUNC(getpartymember)
{
	struct party_data *p;
	int i,j=0,type=0;

	p=party_search(script_getnum(st,2));

	if( script_hasdata(st,3) )
 		type=script_getnum(st,3);
	
	if(p!=NULL){
		for(i=0;i<MAX_PARTY;i++){
			if(p->party.member[i].account_id){
				switch (type) {
				case 2:
					mapreg_setreg(add_str("$@partymemberaid")+(j<<24),p->party.member[i].account_id);
					break;
				case 1:
					mapreg_setreg(add_str("$@partymembercid")+(j<<24),p->party.member[i].char_id);
					break;
				default:
					mapreg_setregstr(add_str("$@partymembername$")+(j<<24),p->party.member[i].name);
				}
				j++;
			}
		}
	}
	mapreg_setreg(add_str("$@partymembercount"),j);

	return 0;
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
		return 0;
	}

	switch (type) {
		case 1: script_pushint(st,p->party.member[i].account_id); break;
		case 2: script_pushint(st,p->party.member[i].char_id); break;
		case 3: script_pushint(st,p->party.member[i].class_); break;
		case 4: script_pushstrcopy(st,mapindex_id2name(p->party.member[i].map)); break;
		case 5: script_pushint(st,p->party.member[i].lv); break;
		default: script_pushstrcopy(st,p->party.member[i].name); break;
	}
	return 0;
}

/*==========================================
 *指定IDのギルド名取得
 *------------------------------------------*/
char *buildin_getguildname_sub(int guild_id)
{
	struct guild *g=NULL;
	g=guild_search(guild_id);

	if(g!=NULL){
		char *buf;
		buf=(char *)aMallocA(NAME_LENGTH*sizeof(char));
		memcpy(buf, g->name, NAME_LENGTH);
		buf[NAME_LENGTH-1] = '\0';
		return buf;
	}
	return NULL;
}
BUILDIN_FUNC(getguildname)
{
	char *name;
	int guild_id=script_getnum(st,2);
	name=buildin_getguildname_sub(guild_id);
	if(name != NULL)
		script_pushstr(st,name);
	else
		script_pushconststr(st,"null");
	return 0;
}

/*==========================================
 *指定IDのGuildMaster名取得
 *------------------------------------------*/
char *buildin_getguildmaster_sub(int guild_id)
{
	struct guild *g=NULL;
	g=guild_search(guild_id);

	if(g!=NULL){
		char *buf;
		buf=(char *)aMallocA(NAME_LENGTH*sizeof(char));
		memcpy(buf, g->master, NAME_LENGTH);
		buf[NAME_LENGTH-1] = '\0';
		return buf;
	}

	return 0;
}
BUILDIN_FUNC(getguildmaster)
{
	char *master;
	int guild_id=script_getnum(st,2);
	master=buildin_getguildmaster_sub(guild_id);
	if(master!=0)
		script_pushstr(st,master);
	else
		script_pushconststr(st,"null");
	return 0;
}

BUILDIN_FUNC(getguildmasterid)
{
	char *master;
	TBL_PC *sd=NULL;
	int guild_id=script_getnum(st,2);
	master=buildin_getguildmaster_sub(guild_id);
	if(master!=0){
		if((sd=map_nick2sd(master)) == NULL){
			script_pushint(st,0);
			return 0;
		}
		script_pushint(st,sd->status.char_id);
	}else{
		script_pushint(st,0);
	}
	return 0;
}

/*==========================================
 * キャラクタの名前
 *------------------------------------------*/
BUILDIN_FUNC(strcharinfo)
{
	TBL_PC *sd;
	int num;
	char *buf;

	sd=script_rid2sd(st);
	if (!sd) { //Avoid crashing....
		script_pushconststr(st,"");
		return 0;
	}
	num=script_getnum(st,2);
	switch(num){
		case 0:
			script_pushstrcopy(st,sd->status.name);
			break;
		case 1:
			buf=buildin_getpartyname_sub(sd->status.party_id);
			if(buf!=0)
				script_pushstr(st,buf);
			else
				script_pushconststr(st,"");
			break;
		case 2:
			buf=buildin_getguildname_sub(sd->status.guild_id);
			if(buf != NULL)
				script_pushstr(st,buf);
			else
				script_pushconststr(st,"");
			break;
		default:
			ShowWarning("buildin_strcharinfo: unknown parameter.");
			script_pushconststr(st,"");
			break;
	}

	return 0;
}

/*==========================================
 * 呼び出し元のNPC情報を取得する
 *------------------------------------------*/
BUILDIN_FUNC(strnpcinfo)
{
	TBL_NPC* nd;
	int num;
	char *buf,*name=NULL;

	nd = map_id2nd(st->oid);
	if (!nd) {
		script_pushconststr(st, "");
		return 0;
	}

	num = script_getnum(st,2);
	switch(num){
		case 0: // display name
			name = aStrdup(nd->name);
			break;
		case 1: // visible part of display name name
			if((buf = strchr(nd->name,'#')) != NULL)
			{
				name = aStrdup(nd->name);
				name[buf - nd->name] = 0;
			}
			break;
		case 2: // # fragment
			if((buf = strchr(nd->name,'#')) != NULL)
				name = aStrdup(buf+1);
			break;
		case 3: // unique name
			name = aStrdup(nd->exname);
			break;
	}

	if(name)
		script_pushstr(st, name);
	else
		script_pushconststr(st, "");

	return 0;
}


unsigned int equip[10]={EQP_HEAD_TOP,EQP_ARMOR,EQP_HAND_L,EQP_HAND_R,EQP_GARMENT,EQP_SHOES,EQP_ACC_L,EQP_ACC_R,EQP_HEAD_MID,EQP_HEAD_LOW};

/*==========================================
 * GetEquipID(Pos);     Pos: 1-10
 *------------------------------------------*/
BUILDIN_FUNC(getequipid)
{
	int i=-1,num;
	TBL_PC *sd;
	struct item_data* item;

	sd=script_rid2sd(st);
	if(sd == NULL)
		return 0;

	num=script_getnum(st,2);
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0){
		item=sd->inventory_data[i];
		if(item)
			script_pushint(st,item->nameid);
		else
			script_pushint(st,0);
	}else{
		script_pushint(st,-1);
	}
	return 0;
}

/*==========================================
 * 装備名文字列（精錬メニュー用）
 *------------------------------------------*/
BUILDIN_FUNC(getequipname)
{
	int i=-1,num;
	TBL_PC *sd;
	struct item_data* item;
	char *buf;

	static char pos[11][100] = {"Head","Body","Left hand","Right hand","Robe","Shoes","Accessory 1","Accessory 2","Head 2","Head 3","Not Equipped"};

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	buf=(char *)aMallocA(64*sizeof(char));
	num=script_getnum(st,2);
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0){
		item=sd->inventory_data[i];
		if(item)
			sprintf(buf,"%s-[%s]",pos[num-1],item->jname);
		else
			sprintf(buf,"%s-[%s]",pos[num-1],pos[10]);
	}else{
		sprintf(buf,"%s-[%s]",pos[num-1],pos[10]);
	}
	script_pushstr(st,buf);

	return 0;
}

/*==========================================
 * getbrokenid [Valaris]
 *------------------------------------------*/
BUILDIN_FUNC(getbrokenid)
{
	int i,num,id=0,brokencounter=0;
	TBL_PC *sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	num=script_getnum(st,2);
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute==1){
				brokencounter++;
				if(num==brokencounter){
					id=sd->status.inventory[i].nameid;
					break;
				}
		}
	}

	script_pushint(st,id);

	return 0;
}

/*==========================================
 * repair [Valaris]
 *------------------------------------------*/
BUILDIN_FUNC(repair)
{
	int i,num;
	int repaircounter=0;
	TBL_PC *sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	num=script_getnum(st,2);
	for(i=0; i<MAX_INVENTORY; i++) {
		if(sd->status.inventory[i].attribute==1){
				repaircounter++;
				if(num==repaircounter){
					sd->status.inventory[i].attribute=0;
					clif_equiplist(sd);
					clif_produceeffect(sd, 0, sd->status.inventory[i].nameid);
					clif_misceffect(&sd->bl, 3);
					break;
				}
		}
	}

	return 0;
}

/*==========================================
 * 装備チェック
 *------------------------------------------*/
BUILDIN_FUNC(getequipisequiped)
{
	int i=-1,num;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);

	if(i >= 0)
		script_pushint(st,1);
	else
		 script_pushint(st,0);
	return 0;
}

/*==========================================
 * 装備品精錬可能チェック
 *------------------------------------------*/
BUILDIN_FUNC(getequipisenableref)
{
	int i=-1,num;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0 && sd->inventory_data[i] && !sd->inventory_data[i]->flag.no_refine)
	{
		script_pushint(st,1);
	} else {
		script_pushint(st,0);
	}

	return 0;
}

/*==========================================
 * 装備品鑑定チェック
 *------------------------------------------*/
BUILDIN_FUNC(getequipisidentify)
{
	int i=-1,num;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0)
		script_pushint(st,sd->status.inventory[i].identify);
	else
		script_pushint(st,0);

	return 0;
}

/*==========================================
 * 装備品精錬度
 *------------------------------------------*/
BUILDIN_FUNC(getequiprefinerycnt)
{
	int i=-1,num;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0)
		script_pushint(st,sd->status.inventory[i].refine);
	else
		script_pushint(st,0);

	return 0;
}

/*==========================================
 * 装備品武器LV
 *------------------------------------------*/
BUILDIN_FUNC(getequipweaponlv)
{
	int i=-1,num;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0 && sd->inventory_data[i])
		script_pushint(st,sd->inventory_data[i]->wlv);
	else
		script_pushint(st,0);

	return 0;
}

/*==========================================
 * 装備品精錬成功率
 *------------------------------------------*/
BUILDIN_FUNC(getequippercentrefinery)
{
	int i=-1,num;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0 && sd->status.inventory[i].nameid && sd->status.inventory[i].refine < MAX_REFINE)
		script_pushint(st,percentrefinery[itemdb_wlv(sd->status.inventory[i].nameid)][(int)sd->status.inventory[i].refine]);
	else
		script_pushint(st,0);

	return 0;
}

/*==========================================
 * 精錬成功
 *------------------------------------------*/
BUILDIN_FUNC(successrefitem)
{
	int i=-1,num,ep;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0) {
		ep=sd->status.inventory[i].equip;

		//Logs items, got from (N)PC scripts [Lupus]
		if(log_config.enable_logs&0x40) 
			log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -1, &sd->status.inventory[i]);

		sd->status.inventory[i].refine++;
		pc_unequipitem(sd,i,2);

		clif_refine(sd->fd,0,i,sd->status.inventory[i].refine);
		clif_delitem(sd,i,1);

		//Logs items, got from (N)PC scripts [Lupus]
		if(log_config.enable_logs&0x40)
			log_pick_pc(sd, "N", sd->status.inventory[i].nameid, 1, &sd->status.inventory[i]);

		clif_additem(sd,i,1,0);
		pc_equipitem(sd,i,ep);
		clif_misceffect(&sd->bl,3);
		if(sd->status.inventory[i].refine == MAX_REFINE &&
			sd->status.inventory[i].card[0] == CARD0_FORGE &&
		  	sd->status.char_id == (int)MakeDWord(sd->status.inventory[i].card[2],sd->status.inventory[i].card[3])
		){ // Fame point system [DracoRPG]
	 		switch (sd->inventory_data[i]->wlv){
				case 1:
					pc_addfame(sd,1); // Success to refine to +10 a lv1 weapon you forged = +1 fame point
					break;
				case 2:
					pc_addfame(sd,25); // Success to refine to +10 a lv2 weapon you forged = +25 fame point
					break;
				case 3:
					pc_addfame(sd,1000); // Success to refine to +10 a lv3 weapon you forged = +1000 fame point
					break;
	 	 	 }
		}
	}

	return 0;
}

/*==========================================
 * 精錬失敗
 *------------------------------------------*/
BUILDIN_FUNC(failedrefitem)
{
	int i=-1,num;
	TBL_PC *sd;

	num=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0) {
		//Logs items, got from (N)PC scripts [Lupus]
		if(log_config.enable_logs&0x40)
			log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -1, &sd->status.inventory[i]);

		sd->status.inventory[i].refine = 0;
		pc_unequipitem(sd,i,3);
		// 精錬失敗エフェクトのパケット
		clif_refine(sd->fd,1,i,sd->status.inventory[i].refine);

		pc_delitem(sd,i,1,0);
		// 他の人にも失敗を通知
		clif_misceffect(&sd->bl,2);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(statusup)
{
	int type;
	TBL_PC *sd;

	type=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pc_statusup(sd,type);

	return 0;
}
/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(statusup2)
{
	int type,val;
	TBL_PC *sd;

	type=script_getnum(st,2);
	val=script_getnum(st,3);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pc_statusup2(sd,type,val);

	return 0;
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
	int val1;
	int val2 = 0;
	int val3 = 0;
	int val4 = 0;
	int val5 = 0;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0; // no player attached

	type = script_getnum(st,2);
	switch( type )
	{
	case SP_AUTOSPELL:
	case SP_AUTOSPELL_WHENHIT:
	case SP_SKILL_ATK:
	case SP_SKILL_HEAL:
	case SP_ADD_SKILL_BLOW:
	case SP_CASTRATE:
		// these bonuses support skill names
		val1 = ( script_isstring(st,3) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
		break;
	default:
		val1 = script_getnum(st,3);
		break;
	}

	switch( script_lastdata(st)-2 )
	{
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
		val2 = script_getnum(st,4);
		val3 = script_getnum(st,5);
		val4 = script_getnum(st,6);
		pc_bonus4(sd, type, val1, val2, val3, val4);
		break;
	case 5:
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

	return 0;
}

/// Bonus script that has a chance of being executed on attack.
BUILDIN_FUNC(bonusautoscript)
{
	int rate, flag = 0;
	const char *str;
	struct script_code *script;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	str  = script_getstr(st,2);
	rate = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	script = parse_script(str, "autoscript bonus", 0, 0);
	if (!script)
		return 1;
	if (!pc_autoscript_add(sd->autoscript, ARRAYLENGTH(sd->autoscript), rate, flag, script))
	{
		script_free_code(script);
		return 1;
	}
	return 0;	
}
/// Bonus script that has a chance of being executed when attacked.
BUILDIN_FUNC(bonusautoscript2)
{
	int rate, flag = 0;
	const char *str;
	struct script_code *script;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	str  = script_getstr(st,2);
	rate = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	script = parse_script(str, "autoscript2 bonus", 0, 0);
	if (!script)
		return 1;
	if (!pc_autoscript_add(sd->autoscript2, ARRAYLENGTH(sd->autoscript2), rate, flag, script))
	{
		script_free_code(script);
		return 1;
	}
	return 0;	
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
	int flag = 1;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	id = ( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	pc_skill(sd, id, level, flag);

	return 0;
}

/// Changes the level of a player skill.
/// like skill, but <flag> defaults to 2
///
/// addtoskill <skill id>,<amount>,<flag>
/// addtoskill <skill id>,<amount>
/// addtoskill "<skill name>",<amount>,<flag>
/// addtoskill "<skill name>",<amount>
///
/// @see skill
BUILDIN_FUNC(addtoskill)
{
	int id;
	int level;
	int flag = 2;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	id = ( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	if( script_hasdata(st,4) )
		flag = script_getnum(st,4);
	pc_skill(sd, id, level, flag);

	return 0;
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

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	id = ( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	level = script_getnum(st,3);
	for( i=0; i < level; i++ )
		guild_skillup(sd, id);

	return 0;
}

/// Returns the level of the player skill.
///
/// getskilllv(<skill id>) -> <level>
/// getskilllv("<skill name>") -> <level>
BUILDIN_FUNC(getskilllv)
{
	int id;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	id = ( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	script_pushint(st, pc_checkskill(sd,id));

	return 0;
}

/// Returns the level of the guild skill.
///
/// getgdskilllv(<guild id>,<skill id>) -> <level>
/// getgdskilllv(<guild id>,"<skill name>") -> <level>
BUILDIN_FUNC(getgdskilllv)
{
	int guild_id;
	int skill_id;
	struct guild* g;

	guild_id = script_getnum(st,2);
	skill_id = ( script_isstring(st,3) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	g = guild_search(guild_id);
	if( g == NULL )
		script_pushint(st, -1);
	else
		script_pushint(st, guild_checkskill(g,skill_id));

	return 0;
}

/// Returns the 'basic_skill_check' setting.
/// This config determines if the server checks the skill level of NV_BASIC 
/// before allowing the basic actions.
///
/// basicskillcheck() -> <bool>
BUILDIN_FUNC(basicskillcheck)
{
	script_pushint(st, battle_config.basic_skill_check);
	return 0;
}

/// Returns the GM level of the player.
///
/// getgmlevel() -> <level>
BUILDIN_FUNC(getgmlevel)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	script_pushint(st, pc_isGM(sd));

	return 0;
}

/// Terminates the execution of this script instance.
///
/// end
BUILDIN_FUNC(end)
{
	st->state = END;
	return 0;
}

/// Checks if the player has that effect state (option).
///
/// checkoption(<option>) -> <bool>
BUILDIN_FUNC(checkoption)
{
	int option;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	option = script_getnum(st,2);
	if( sd->sc.option&option )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return 0;
}

/// Checks if the player is in that body state (opt1).
///
/// checkoption1(<opt1>) -> <bool>
BUILDIN_FUNC(checkoption1)
{
	int opt1;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	opt1 = script_getnum(st,2);
	if( sd->sc.opt1 == opt1 )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return 0;
}

/// Checks if the player has that health state (opt2).
///
/// checkoption2(<opt2>) -> <bool>
BUILDIN_FUNC(checkoption2)
{
	int opt2;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	opt2 = script_getnum(st,2);
	if( sd->sc.opt2&opt2 )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return 0;
}

/// Changes the effect state (option) of the player.
/// <flag> defaults to 1
/// <flag>=0 : removes the option
/// <flag>=other : adds the option
///
/// setoption <option>,<flag>;
/// setoption <option>;
BUILDIN_FUNC(setoption)
{
	int option;
	int flag = 1;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	option = script_getnum(st,2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	else if( !option ){// Request to remove everything.
		flag = 0;
		option = OPTION_CART|OPTION_FALCON|OPTION_RIDING;
	}
	if( flag ){// Add option
		if( option&OPTION_WEDDING && !battle_config.wedding_modifydisplay )
			option &= ~OPTION_WEDDING;// Do not show the wedding sprites
		pc_setoption(sd, sd->sc.option|option);
	} else// Remove option
		pc_setoption(sd, sd->sc.option&~option);

	return 0;
}

/// Returns if the player has a cart.
///
/// checkcart() -> <bool>
///
/// @author Valaris
BUILDIN_FUNC(checkcart)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	if( pc_iscarton(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return 0;
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
/// setcart <type>;
/// setcart;
BUILDIN_FUNC(setcart)
{
	int type = 1;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	if( script_hasdata(st,2) )
		type = script_getnum(st,2);
	pc_setcart(sd, type);

	return 0;
}

/// Returns if the player has a falcon.
///
/// checkfalcon() -> <bool>
///
/// @author Valaris
BUILDIN_FUNC(checkfalcon)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	if( pc_isfalcon(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return 0;
}

/// Sets if the player has a falcon or not.
/// <flag> defaults to 1
///
/// setfalcon <flag>;
/// setfalcon;
BUILDIN_FUNC(setfalcon)
{
	int flag = 1;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);

	pc_setfalcon(sd, flag);

	return 0;
}

/// Returns if the player is riding.
///
/// checkriding() -> <bool>
///
/// @author Valaris
BUILDIN_FUNC(checkriding)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	if( pc_isriding(sd) )
		script_pushint(st, 1);
	else
		script_pushint(st, 0);

	return 0;
}

/// Sets if the player is riding.
/// <flag> defaults to 1
///
/// setriding <flag>;
/// setriding;
BUILDIN_FUNC(setriding)
{
	int flag = 1;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	if( script_hasdata(st,2) )
		flag = script_getnum(st,2);
	pc_setriding(sd, flag);

	return 0;
}

/// Sets the save point of the player.
///
/// save "<map name>",<x>,<y>
/// savepoint "<map name>",<x>,<y>
BUILDIN_FUNC(savepoint)
{
	int x;
	int y;
	short map;
	const char* str;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;// no player attached, report source

	str = script_getstr(st, 2);
	x   = script_getnum(st,3);
	y   = script_getnum(st,4);
	map = mapindex_name2id(str);
	if( map )
		pc_setsavepoint(sd, map, x, y);

	return 0;
}

/*==========================================
 * GetTimeTick(0: System Tick, 1: Time Second Tick)
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
	return 0;
}

/*==========================================
 * GetTime(Type);
 * 1: Sec     2: Min     3: Hour
 * 4: WeekDay     5: MonthDay     6: Month
 * 7: Year
 *------------------------------------------*/
BUILDIN_FUNC(gettime)	/* Asgard Version */
{
	int type;
	time_t timer;
	struct tm *t;

	type=script_getnum(st,2);

	time(&timer);
	t=localtime(&timer);

	switch(type){
	case 1://Sec(0~59)
		script_pushint(st,t->tm_sec);
		break;
	case 2://Min(0~59)
		script_pushint(st,t->tm_min);
		break;
	case 3://Hour(0~23)
		script_pushint(st,t->tm_hour);
		break;
	case 4://WeekDay(0~6)
		script_pushint(st,t->tm_wday);
		break;
	case 5://MonthDay(01~31)
		script_pushint(st,t->tm_mday);
		break;
	case 6://Month(01~12)
		script_pushint(st,t->tm_mon+1);
		break;
	case 7://Year(20xx)
		script_pushint(st,t->tm_year+1900);
		break;
	case 8://Year Day(01~366)
		script_pushint(st,t->tm_yday+1);
		break;
	default://(format error)
		script_pushint(st,-1);
		break;
	}
	return 0;
}

/*==========================================
 * GetTimeStr("TimeFMT", Length);
 *------------------------------------------*/
BUILDIN_FUNC(gettimestr)
{
	char *tmpstr;
	const char *fmtstr;
	int maxlen;
	time_t now = time(NULL);

	fmtstr=script_getstr(st,2);
	maxlen=script_getnum(st,3);

	tmpstr=(char *)aMallocA((maxlen+1)*sizeof(char));
	strftime(tmpstr,maxlen,fmtstr,localtime(&now));
	tmpstr[maxlen]='\0';

	script_pushstr(st,tmpstr);
	return 0;
}

/*==========================================
 * カプラ倉庫を開く
 *------------------------------------------*/
BUILDIN_FUNC(openstorage)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	storage_storageopen(sd);
	return 0;
}

BUILDIN_FUNC(guildopenstorage)
{
	TBL_PC* sd;
	int ret;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	ret = storage_guild_storageopen(sd);
	script_pushint(st,ret);
	return 0;
}

/*==========================================
 * アイテムによるスキル発動
 *------------------------------------------*/
/// itemskill <skill id>,<level>
/// itemskill "<skill name>",<level>
BUILDIN_FUNC(itemskill)
{
	int id;
	int lv;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL || sd->ud.skilltimer != INVALID_TIMER )
		return 0;

	id = ( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	lv = script_getnum(st,3);

	sd->skillitem=id;
	sd->skillitemlv=lv;
	clif_item_skill(sd,id,lv);
	return 0;
}
/*==========================================
 * アイテム作成
 *------------------------------------------*/
BUILDIN_FUNC(produce)
{
	int trigger;
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	trigger=script_getnum(st,2);
	clif_skill_produce_mix_list(sd, trigger);
	return 0;
}
/*==========================================
 * NPCでペット作る
 *------------------------------------------*/
BUILDIN_FUNC(makepet)
{
	TBL_PC* sd;
	int id,pet_id;

	id=script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pet_id = search_petDB_index(id, PET_CLASS);

	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0 && sd) {
		sd->catch_target_class = pet_db[pet_id].class_;
		intif_create_pet(
			sd->status.account_id, sd->status.char_id,
			(short)pet_db[pet_id].class_, (short)mob_db(pet_db[pet_id].class_)->lv,
			(short)pet_db[pet_id].EggID, 0, (short)pet_db[pet_id].intimate,
			100, 0, 1, pet_db[pet_id].jname);
	}

	return 0;
}
/*==========================================
 * NPCで経験値上げる
 *------------------------------------------*/
BUILDIN_FUNC(getexp)
{
	TBL_PC* sd;
	int base=0,job=0;
	double bonus;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	base=script_getnum(st,2);
	job =script_getnum(st,3);
	if(base<0 || job<0)
		return 0;

	// bonus for npc-given exp
	bonus = battle_config.quest_exp_rate / 100.;
	base = (int) cap_value(base * bonus, 0, INT_MAX);
	job = (int) cap_value(job * bonus, 0, INT_MAX);

	pc_gainexp(sd, NULL, base, job);

	return 0;
}

/*==========================================
 * Gain guild exp [Celest]
 *------------------------------------------*/
BUILDIN_FUNC(guildgetexp)
{
	TBL_PC* sd;
	int exp;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	exp = script_getnum(st,2);
	if(exp < 0)
		return 0;
	if(sd && sd->status.guild_id > 0)
		guild_getexp (sd, exp);

	return 0;
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
	sd=map_nick2sd(name);

	if (!sd)
		script_pushint(st,0);
	else
		script_pushint(st,guild_gm_change(guild_id, sd));

	return 0;
}

/*==========================================
 * モンスター発生
 *------------------------------------------*/
BUILDIN_FUNC(monster)
{
	const char* map   = script_getstr(st,2);
	int x             = script_getnum(st,3);
	int y             = script_getnum(st,4);
	const char* str   = script_getstr(st,5);
	int class_        = script_getnum(st,6);
	int amount        = script_getnum(st,7);
	const char* event = "";

	struct map_session_data* sd;
	int m;

	if( script_hasdata(st,8) )
	{
		event = script_getstr(st,8);
		check_event(st, event);
	}

	if (class_ >= 0 && !mobdb_checkid(class_)) {
		ShowWarning("buildin_monster: Attempted to spawn non-existing monster class %d\n", class_);
		return 1;
	}

	sd = map_id2sd(st->rid);

	if( sd && strcmp(map,"this") == 0 )
		m = sd->bl.m;
	else
		m = map_mapname2mapid(map);

	mob_once_spawn(sd,m,x,y,str,class_,amount,event);
	return 0;
}
/*==========================================
 * Request List of Monster Drops
 *------------------------------------------*/
BUILDIN_FUNC(getmobdrops)
{
	int class_ = script_getnum(st,2);
	int i, j = 0;
	struct item_data *i_data;
	struct mob_db *mob;

	if( !mobdb_checkid(class_) )
	{
		script_pushint(st, 0);
		return 0;
	}

	mob = mob_db(class_);

	for( i = 0; i < MAX_MOB_DROP; i++ )
	{
		if( mob->dropitem[i].nameid < 1 )
			continue;
		if( (i_data = itemdb_exists(mob->dropitem[i].nameid)) == NULL )
			continue;

		mapreg_setreg(add_str("$@MobDrop_item") + (j<<24), mob->dropitem[i].nameid);
		mapreg_setreg(add_str("$@MobDrop_rate") + (j<<24), mob->dropitem[i].p);

		j++;
	}

	mapreg_setreg(add_str("$@MobDrop_count"), j);
	script_pushint(st, 1);

	return 0;
}
/*==========================================
 * モンスター発生
 *------------------------------------------*/
BUILDIN_FUNC(areamonster)
{
	const char* map   = script_getstr(st,2);
	int x0            = script_getnum(st,3);
	int y0            = script_getnum(st,4);
	int x1            = script_getnum(st,5);
	int y1            = script_getnum(st,6);
	const char* str   = script_getstr(st,7);
	int class_        = script_getnum(st,8);
	int amount        = script_getnum(st,9);
	const char* event = "";

	struct map_session_data* sd;
	int m;

	if( script_hasdata(st,10) )
	{
		event = script_getstr(st,10);
		check_event(st, event);
	}

	sd = map_id2sd(st->rid);

	if( sd && strcmp(map,"this") == 0 )
		m = sd->bl.m;
	else
		m = map_mapname2mapid(map);

	mob_once_spawn_area(sd,m,x0,y0,x1,y1,str,class_,amount,event);
	return 0;
}
/*==========================================
 * モンスター削除
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
	return 0;
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
	return 0;
}
BUILDIN_FUNC(killmonster)
{
	const char *mapname,*event;
	int m,allflag=0;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);
	if(strcmp(event,"All")==0)
		allflag = 1;
	else
		check_event(st, event);

	if( (m=map_mapname2mapid(mapname))<0 )
		return 0;
		
	if( script_hasdata(st,4) ) {
		if ( script_getnum(st,4) == 1 ) {
			map_foreachinmap(buildin_killmonster_sub, m, BL_MOB, event ,allflag);
			return 0;
		}
	}
	
	map_foreachinmap(buildin_killmonster_sub_strip, m, BL_MOB, event ,allflag);
	return 0;
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
	int m;
	mapname=script_getstr(st,2);
	
	if( (m=map_mapname2mapid(mapname))<0 )
		return 0;
	
	if( script_hasdata(st,3) ) {
		if ( script_getnum(st,3) == 1 ) {
			map_foreachinmap(buildin_killmonsterall_sub,m,BL_MOB);
			return 0;
		}
	}
		
	map_foreachinmap(buildin_killmonsterall_sub_strip,m,BL_MOB);
	return 0;
}

/*==========================================
 * Creates a clone of a player.
 * clone map, x, y, event, char_id, master_id, mode, flag, duration
 *------------------------------------------*/
BUILDIN_FUNC(clone)
{
	TBL_PC *sd, *msd=NULL;
	int char_id,master_id=0,x,y, mode = 0, flag = 0, m;
	unsigned int duration = 0;
	const char *map,*event="";

	map=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);
	event=script_getstr(st,5);
	char_id=script_getnum(st,6);

	if( script_hasdata(st,7) )
		master_id=script_getnum(st,7);

	if( script_hasdata(st,8) )
		mode=script_getnum(st,8);

	if( script_hasdata(st,9) )
		flag=script_getnum(st,9);
	
	if( script_hasdata(st,10) )
		duration=script_getnum(st,10);

	check_event(st, event);

	m = map_mapname2mapid(map);
	if (m < 0) return 0;

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

	return 0;
}
/*==========================================
 * イベント実行
 *------------------------------------------*/
BUILDIN_FUNC(doevent)
{
	const char* event = script_getstr(st,2);
	check_event(st, event);
	npc_event(map_id2sd(st->rid),event,0);
	return 0;
}
/*==========================================
 * NPC主体イベント実行
 *------------------------------------------*/
BUILDIN_FUNC(donpcevent)
{
	const char* event = script_getstr(st,2);
	check_event(st, event);
	npc_event_do(event);
	return 0;
}

/// for Aegis compatibility
/// basically a specialized 'donpcevent', with the event specified as two arguments instead of one
BUILDIN_FUNC(cmdothernpc)	// Added by RoVeRT
{
	const char* npc = script_getstr(st,2);
	const char* command = script_getstr(st,3);
	char event[51];
	snprintf(event, 51, "%s::OnCommand%s", npc, command);
	check_event(st, event);
	npc_event_do(event);
	return 0;
}

/*==========================================
 * イベントタイマー追加
 *------------------------------------------*/
BUILDIN_FUNC(addtimer)
{
	int tick = script_getnum(st,2);
	const char* event = script_getstr(st, 3);
	TBL_PC* sd;

	check_event(st, event);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pc_addeventtimer(sd,tick,event);
	return 0;
}
/*==========================================
 * イベントタイマー削除
 *------------------------------------------*/
BUILDIN_FUNC(deltimer)
{
	const char *event;
	TBL_PC* sd;

	event=script_getstr(st, 2);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	check_event(st, event);
	pc_deleventtimer(sd,event);
	return 0;
}
/*==========================================
 * イベントタイマーのカウント値追加
 *------------------------------------------*/
BUILDIN_FUNC(addtimercount)
{
	const char *event;
	int tick;
	TBL_PC* sd;

	event=script_getstr(st, 2);
	tick=script_getnum(st,3);
	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

	check_event(st, event);
	pc_addeventtimercount(sd,event,tick);
	return 0;
}

/*==========================================
 * NPCタイマー初期化
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
			return 1;
		}
	}
	else
		nd=(struct npc_data *)map_id2bl(st->oid);

	if (!nd)
		return 0;
	if (flag) //Attach
	{
		TBL_PC* sd = script_rid2sd(st);
		if( sd == NULL )
			return 0;
		nd->u.scr.rid = sd->bl.id;
	}

	npc_settimerevent_tick(nd,0);
	npc_timerevent_start(nd, st->rid);
	return 0;
}
/*==========================================
 * NPCタイマー開始
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
			return 1;
		}
	}
	else
		nd=(struct npc_data *)map_id2bl(st->oid);

	if (!nd)
		return 0;
	if (flag) //Attach
	{
		TBL_PC* sd = script_rid2sd(st);
		if( sd == NULL )
			return 0;
		nd->u.scr.rid = sd->bl.id;
	}

	npc_timerevent_start(nd, st->rid);
	return 0;
}
/*==========================================
 * NPCタイマー停止
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
			return 1;
		}
	}
	else
		nd=(struct npc_data *)map_id2bl(st->oid);

	if (!nd)
		return 0;
	if (flag) //Detach
		nd->u.scr.rid = 0;

	npc_timerevent_stop(nd);
	return 0;
}
/*==========================================
 * NPCタイマー情報所得
 *------------------------------------------*/
BUILDIN_FUNC(getnpctimer)
{
	struct npc_data *nd;
	TBL_PC *sd;
	int type=script_getnum(st,2);
	int val=0;
	if( script_hasdata(st,3) )
		nd = npc_name2id(script_getstr(st,3));
	else
		nd = (struct npc_data *)map_id2bl(st->oid);
	
	if (!nd || nd->bl.type != BL_NPC)
	{
		script_pushint(st,0);
		ShowError("getnpctimer: Invalid NPC.\n");
		return 1;
	}

	switch(type){
	case 0: val=npc_gettimerevent_tick(nd); break;
	case 1:
		if (nd->u.scr.rid) {
			sd = map_id2sd(nd->u.scr.rid);
			if (!sd) {
				ShowError("buildin_getnpctimer: Attached player not found!\n");
				break;
			}
			val = (sd->npc_timer_id != -1);
		} else
			  val= (nd->u.scr.timerid !=-1);
		break;
	case 2: val= nd->u.scr.timeramount; break;
	}
	script_pushint(st,val);
	return 0;
}
/*==========================================
 * NPCタイマー値設定
 *------------------------------------------*/
BUILDIN_FUNC(setnpctimer)
{
	int tick;
	struct npc_data *nd;
	tick=script_getnum(st,2);
	if( script_hasdata(st,3) )
		nd=npc_name2id(script_getstr(st,3));
	else
		nd=(struct npc_data *)map_id2bl(st->oid);

	if (!nd || nd->bl.type != BL_NPC)
	{
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return 1;
	}

	npc_settimerevent_tick(nd,tick);
	script_pushint(st,0);
	return 0;
}

/*==========================================
 * attaches the player rid to the timer [Celest]
 *------------------------------------------*/
BUILDIN_FUNC(attachnpctimer)
{
	TBL_PC *sd;
	struct npc_data *nd;

	nd=(struct npc_data *)map_id2bl(st->oid);

	if (!nd || nd->bl.type != BL_NPC)
	{
		script_pushint(st,1);
		ShowError("setnpctimer: Invalid NPC.\n");
		return 1;
	}

	if( script_hasdata(st,2) )
		sd=map_nick2sd(script_getstr(st,2));
	else
		sd = script_rid2sd(st);

	if (!sd)
	{
		script_pushint(st,1);
		ShowWarning("attachnpctimer: Invalid player.\n");
		return 1;
	}

	nd->u.scr.rid = sd->bl.id;
	script_pushint(st,0);
	return 0;
}

/*==========================================
 * detaches a player rid from the timer [Celest]
 *------------------------------------------*/
BUILDIN_FUNC(detachnpctimer)
{
	struct npc_data *nd;
	if( script_hasdata(st,2) )
		nd=npc_name2id(script_getstr(st,2));
	else
		nd=(struct npc_data *)map_id2bl(st->oid);

	if (!nd || nd->bl.type != BL_NPC)
	{
		script_pushint(st,1);
		ShowError("detachnpctimer: Invalid NPC.\n");
		return 1;
	}

	nd->u.scr.rid = 0;
	script_pushint(st,0);
	return 0;
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
	return 0;
}

/*==========================================
 * 天の声アナウンス
 *------------------------------------------*/
BUILDIN_FUNC(announce)
{
	const char *str, *color=NULL;
	int flag;
	str=script_getstr(st,2);
	flag=script_getnum(st,3);
	if (script_hasdata(st,4))
		color=script_getstr(st,4);

	if(flag&0x0f){
		struct block_list *bl=(flag&0x08)? map_id2bl(st->oid) :
			(struct block_list *)script_rid2sd(st);
		if( bl == NULL )
			return 0;
		if (color)
			clif_announce(bl,str,(int)strlen(str)+1, strtol(color, (char **)NULL, 0),flag);
		else
			clif_GMmessage(bl,str,(int)strlen(str)+1,flag);
	}else {
		if (color)
			intif_announce(str,(int)strlen(str)+1, strtol(color, (char **)NULL, 0), flag);
		else
			intif_GMmessage(str,(int)strlen(str)+1,flag);
	}
	return 0;
}
/*==========================================
 * 天の声アナウンス（特定マップ）
 *------------------------------------------*/
static int buildin_mapannounce_sub(struct block_list *bl,va_list ap)
{
	char *str, *color;
	int len,flag;
	str=va_arg(ap,char *);
	len=va_arg(ap,int);
	flag=va_arg(ap,int);
	color=va_arg(ap,char *);
	if (color)
		clif_announce(bl,str,len, strtol(color, (char **)NULL, 0), flag|3);
	else
		clif_GMmessage(bl,str,len,flag|3);
	return 0;
}
BUILDIN_FUNC(mapannounce)
{
	const char *mapname,*str, *color=NULL;
	int flag,m;

	mapname=script_getstr(st,2);
	str=script_getstr(st,3);
	flag=script_getnum(st,4);
	if (script_hasdata(st,5))
		color=script_getstr(st,5);

	if( (m=map_mapname2mapid(mapname))<0 )
		return 0;

	map_foreachinmap(buildin_mapannounce_sub,
		m, BL_PC, str,strlen(str)+1,flag&0x10, color);
	return 0;
}
/*==========================================
 * 天の声アナウンス（特定エリア）
 *------------------------------------------*/
BUILDIN_FUNC(areaannounce)
{
	const char *map,*str,*color=NULL;
	int flag,m;
	int x0,y0,x1,y1;

	map=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	str=script_getstr(st,7);
	flag=script_getnum(st,8);
	if (script_hasdata(st,9))
		color=script_getstr(st,9);

	if( (m=map_mapname2mapid(map))<0 )
		return 0;

	map_foreachinarea(buildin_mapannounce_sub,
		m,x0,y0,x1,y1,BL_PC, str,strlen(str)+1,flag&0x10, color);
	return 0;
}

/*==========================================
 * ユーザー数所得
 *------------------------------------------*/
BUILDIN_FUNC(getusers)
{
	int flag=script_getnum(st,2);
	struct block_list *bl=map_id2bl((flag&0x08)?st->oid:st->rid);
	int val=0;
	switch(flag&0x07){
	case 0: val=map[bl->m].users; break;
	case 1: val=map_getusers(); break;
	}
	script_pushint(st,val);
	return 0;
}
/*==========================================
 * Works like @WHO - displays all online users names in window
 *------------------------------------------*/
BUILDIN_FUNC(getusersname)
{
	TBL_PC *sd, *pl_sd;
	int disp_num=1;
	struct s_mapiterator* iter;

	sd = script_rid2sd(st);
	if (!sd) return 0;

	iter = mapit_getallusers();
	for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
	{
		if( battle_config.hide_GM_session && pc_isGM(pl_sd) )
			continue; // skip hidden GMs

		if((disp_num++)%10==0)
			clif_scriptnext(sd,st->oid);
		clif_scriptmes(sd,st->oid,pl_sd->status.name);
	}
	mapit_free(iter);
	
	return 0;
}
/*==========================================
 * getmapguildusers("mapname",guild ID) Returns the number guild members present on a map [Reddozen]
 *------------------------------------------*/
BUILDIN_FUNC(getmapguildusers)
{
	const char *str;
	int m, gid;
	int i=0,c=0;
	struct guild *g = NULL;
	str=script_getstr(st,2);
	gid=script_getnum(st,3);
	if ((m = map_mapname2mapid(str)) < 0) { // map id on this server (m == -1 if not in actual map-server)
		script_pushint(st,-1);
		return 0;
	}
	g = guild_search(gid);

	if (g){
		for(i = 0; i < g->max_member; i++)
		{
			if (g->member[i].sd && g->member[i].sd->bl.m == m)
				c++;
		}
	}

	script_pushint(st,c);
	return 0;
}
/*==========================================
 * マップ指定ユーザー数所得
 *------------------------------------------*/
BUILDIN_FUNC(getmapusers)
{
	const char *str;
	int m;
	str=script_getstr(st,2);
	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return 0;
	}
	script_pushint(st,map[m].users);
	return 0;
}
/*==========================================
 * エリア指定ユーザー数所得
 *------------------------------------------*/
static int buildin_getareausers_sub(struct block_list *bl,va_list ap)
{
	int *users=va_arg(ap,int *);
	(*users)++;
	return 0;
}
BUILDIN_FUNC(getareausers)
{
	const char *str;
	int m,x0,y0,x1,y1,users=0;
	str=script_getstr(st,2);
	x0=script_getnum(st,3);
	y0=script_getnum(st,4);
	x1=script_getnum(st,5);
	y1=script_getnum(st,6);
	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return 0;
	}
	map_foreachinarea(buildin_getareausers_sub,
		m,x0,y0,x1,y1,BL_PC,&users);
	script_pushint(st,users);
	return 0;
}

/*==========================================
 * エリア指定ドロップアイテム数所得
 *------------------------------------------*/
static int buildin_getareadropitem_sub(struct block_list *bl,va_list ap)
{
	int item=va_arg(ap,int);
	int *amount=va_arg(ap,int *);
	struct flooritem_data *drop=(struct flooritem_data *)bl;

	if(drop->item_data.nameid==item)
		(*amount)+=drop->item_data.amount;

	return 0;
}
BUILDIN_FUNC(getareadropitem)
{
	const char *str;
	int m,x0,y0,x1,y1,item,amount=0;
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
		item=UNKNOWN_ITEM_ID;
		if( item_data )
			item=item_data->nameid;
	}else
		item=conv_num(st,data);

	if( (m=map_mapname2mapid(str))< 0){
		script_pushint(st,-1);
		return 0;
	}
	map_foreachinarea(buildin_getareadropitem_sub,
		m,x0,y0,x1,y1,BL_ITEM,item,&amount);
	script_pushint(st,amount);
	return 0;
}
/*==========================================
 * NPCの有効化
 *------------------------------------------*/
BUILDIN_FUNC(enablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,1);
	return 0;
}
/*==========================================
 * NPCの無効化
 *------------------------------------------*/
BUILDIN_FUNC(disablenpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,0);
	return 0;
}

/*==========================================
 * 隠れているNPCの表示
 *------------------------------------------*/
BUILDIN_FUNC(hideoffnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,2);
	return 0;
}
/*==========================================
 * NPCをハイディング
 *------------------------------------------*/
BUILDIN_FUNC(hideonnpc)
{
	const char *str;
	str=script_getstr(st,2);
	npc_enable(str,4);
	return 0;
}

/// Starts a status effect on the target unit or on the attached player.
///
/// sc_start <effect_id>,<duration>,<val1>{,<unit_id>};
BUILDIN_FUNC(sc_start)
{
	struct block_list* bl;
	enum sc_type type;
	int tick;
	int val1;
	int val4 = 0;

	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);
	if( script_hasdata(st,5) )
		bl = map_id2bl(script_getnum(st,5));
	else
		bl = map_id2bl(st->rid);

	if( tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_sc2skill(type) != 0 )
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill_get_time(status_sc2skill(type), val1);
	}

	if( potion_flag == 1 && potion_target )
	{	//skill.c set the flags before running the script, this must be a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
		val4 = 1;// Mark that this was a thrown sc_effect
	}

	if( bl )
		status_change_start(bl, type, 10000, val1, 0, 0, val4, tick, 2);

	return 0;
}

/// Starts a status effect on the target unit or on the attached player.
///
/// sc_start2 <effect_id>,<duration>,<val1>,<percent chance>{,<unit_id>};
BUILDIN_FUNC(sc_start2)
{
	struct block_list* bl;
	enum sc_type type;
	int tick;
	int val1;
	int val4 = 0;
	int rate;

	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);
	rate = script_getnum(st,5);
	if( script_hasdata(st,6) )
		bl = map_id2bl(script_getnum(st,6));
	else
		bl = map_id2bl(st->rid);

	if( tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_sc2skill(type) != 0 )
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill_get_time(status_sc2skill(type), val1);
	}

	if( potion_flag == 1 && potion_target )
	{	//skill.c set the flags before running the script, this must be a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
		val4 = 1;// Mark that this was a thrown sc_effect
	}

	if( bl )
		status_change_start(bl, type, rate, val1, 0, 0, val4, tick, 2);

	return 0;
}

/// Starts a status effect on the target unit or on the attached player.
///
/// sc_start4 <effect_id>,<duration>,<val1>,<val2>,<val3>,<val4>{,<unit_id>};
BUILDIN_FUNC(sc_start4)
{
	struct block_list* bl;
	enum sc_type type;
	int tick;
	int val1;
	int val2;
	int val3;
	int val4;

	type = (sc_type)script_getnum(st,2);
	tick = script_getnum(st,3);
	val1 = script_getnum(st,4);
	val2 = script_getnum(st,5);
	val3 = script_getnum(st,6);
	val4 = script_getnum(st,7);
	if( script_hasdata(st,8) )
		bl = map_id2bl(script_getnum(st,8));
	else
		bl = map_id2bl(st->rid);

	if( tick == 0 && val1 > 0 && type > SC_NONE && type < SC_MAX && status_sc2skill(type) != 0 )
	{// When there isn't a duration specified, try to get it from the skill_db
		tick = skill_get_time(status_sc2skill(type), val1);
	}

	if( potion_flag == 1 && potion_target )
	{	//skill.c set the flags before running the script, this must be a potion-pitched effect.
		bl = map_id2bl(potion_target);
		tick /= 2;// Thrown potions only last half.
	}

	if( bl )
		status_change_start(bl, type, 10000, val1, val2, val3, val4, tick, 2);

	return 0;
}

/// Ends one or all status effects on the target unit or on the attached player.
///
/// sc_end <effect_id>{,<unit_id>};
BUILDIN_FUNC(sc_end)
{
	struct block_list* bl;
	int type;

	type = script_getnum(st,2);
	if( script_hasdata(st,3) )
		bl = map_id2bl(script_getnum(st,3));
	else
		bl = map_id2bl(st->rid);
	
	if( potion_flag==1 && potion_target )
	{//##TODO how does this work [FlavioJS]
		bl = map_id2bl(potion_target);
	}

	if( !bl ) return 0;

	if( type >= 0 && type < SC_MAX )
	{
		struct status_change *sc = status_get_sc(bl);
		struct status_change_entry *sce = sc?sc->data[type]:NULL;
		if (!sce) return 0;
		//This should help status_change_end force disabling the SC in case it has no limit.
		sce->val1 = sce->val2 = sce->val3 = sce->val4 = 0;
		status_change_end(bl, (sc_type)type, INVALID_TIMER);
	} else
		status_change_clear(bl, 2);// remove all effects
	return 0;
}

/*==========================================
 * 状態異常耐性を計算した確率を返す
 *------------------------------------------*/
BUILDIN_FUNC(getscrate)
{
	struct block_list *bl;
	int type,rate;

	type=script_getnum(st,2);
	rate=script_getnum(st,3);
	if( script_hasdata(st,4) ) //指定したキャラの耐性を計算する
		bl = map_id2bl(script_getnum(st,4));
	else
		bl = map_id2bl(st->rid);

	if (bl)
		rate = status_get_sc_def(bl, (sc_type)type, 10000, 10000, 0);

	script_pushint(st,rate);
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(debugmes)
{
	const char *str;
	str=script_getstr(st,2);
	ShowDebug("script debug : %d %d : %s\n",st->rid,st->oid,str);
	return 0;
}

/*==========================================
 *捕獲アイテム使用
 *------------------------------------------*/
BUILDIN_FUNC(catchpet)
{
	int pet_id;
	TBL_PC *sd;

	pet_id= script_getnum(st,2);
	sd=script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pet_catch_process1(sd,pet_id);
	return 0;
}

/*==========================================
 * [orn]
 *------------------------------------------*/
BUILDIN_FUNC(homunculus_evolution)
{
	TBL_PC *sd;

	sd=script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if(merc_is_hom_active(sd->hd))
	{
		if (sd->hd->homunculus.intimacy > 91000)
			merc_hom_evolution(sd->hd);
		else
			clif_emotion(&sd->hd->bl, 4) ;	//swt
	}
	return 0;
}

// [Zephyrus]
BUILDIN_FUNC(homunculus_shuffle)
{
	TBL_PC *sd;

	sd=script_rid2sd(st);
	if( sd == NULL )
		return 0;

	if(merc_is_hom_active(sd->hd))
		merc_hom_shuffle(sd->hd);

	return 0;
}

//These two functions bring the eA MAPID_* class functionality to scripts.
BUILDIN_FUNC(eaclass)
{
	int class_;
	if( script_hasdata(st,2) )
		class_ = script_getnum(st,2);
	else {
		TBL_PC *sd;
		sd=script_rid2sd(st);
		if (!sd) {
			script_pushint(st,-1);
			return 0;
		}
		class_ = sd->status.class_;
	}
	script_pushint(st,pc_jobid2mapid(class_));
	return 0;
}

BUILDIN_FUNC(roclass)
{
	int class_ =script_getnum(st,2);
	int sex;
	if( script_hasdata(st,3) )
		sex = script_getnum(st,3);
	else {
		TBL_PC *sd;
		if (st->rid && (sd=script_rid2sd(st)))
			sex = sd->status.sex;
		else
			sex = 1; //Just use male when not found.
	}
	script_pushint(st,pc_mapid2jobid(class_, sex));
	return 0;
}

/*==========================================
 *携帯卵孵化機使用
 *------------------------------------------*/
BUILDIN_FUNC(birthpet)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	if( sd == NULL )
		return 0;

	clif_sendegg(sd);
	return 0;
}

/*==========================================
 * Added - AppleGirl For Advanced Classes, (Updated for Cleaner Script Purposes)
 *------------------------------------------*/
BUILDIN_FUNC(resetlvl)
{
	TBL_PC *sd;

	int type=script_getnum(st,2);

	sd=script_rid2sd(st);
	if( sd == NULL )
		return 0;

	pc_resetlvl(sd,type);
	return 0;
}
/*==========================================
 * ステータスリセット
 *------------------------------------------*/
BUILDIN_FUNC(resetstatus)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	pc_resetstate(sd);
	return 0;
}

/*==========================================
 * script command resetskill
 *------------------------------------------*/
BUILDIN_FUNC(resetskill)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	pc_resetskill(sd,1);
	return 0;
}

/*==========================================
 * Counts total amount of skill points.
 *------------------------------------------*/
BUILDIN_FUNC(skillpointcount)
{
	TBL_PC *sd;
	sd=script_rid2sd(st);
	script_pushint(st,sd->status.skill_point + pc_resetskill(sd,2));
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(changebase)
{
	TBL_PC *sd=NULL;
	int vclass;

	if( script_hasdata(st,3) )
		sd=map_id2sd(script_getnum(st,3));
	else
	sd=script_rid2sd(st);

	if(sd == NULL)
		return 0;

	vclass = script_getnum(st,2);
	if(vclass == JOB_WEDDING)
	{
		if (!battle_config.wedding_modifydisplay || //Do not show the wedding sprites
			sd->class_&JOBL_BABY //Baby classes screw up when showing wedding sprites. [Skotlex] They don't seem to anymore.
			) 
		return 0;
	}

	if(!sd->disguise && vclass != sd->vd.class_) {
		status_set_viewdata(&sd->bl, vclass);
		//Updated client view. Base, Weapon and Cloth Colors.
		clif_changelook(&sd->bl,LOOK_BASE,sd->vd.class_);
		clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
		if (sd->vd.cloth_color)
			clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->vd.cloth_color);
	}

	return 0;
}

/*==========================================
 * 性別変換
 *------------------------------------------*/
BUILDIN_FUNC(changesex)
{
	TBL_PC *sd = NULL;
	sd = script_rid2sd(st);

	chrif_changesex(sd);
	return 0;
}

/*==========================================
 * Works like 'announce' but outputs in the common chat window
 *------------------------------------------*/
BUILDIN_FUNC(globalmes)
{
	struct block_list *bl = map_id2bl(st->oid);
	struct npc_data *nd = (struct npc_data *)bl;
	const char *name=NULL,*mes;

	mes=script_getstr(st,2);	// メッセージの取得
	if(mes==NULL) return 0;
	
	if(script_hasdata(st,3)){	// NPC名の取得(123#456)
		name=script_getstr(st,3);
	} else {
		name=nd->name;
	}

	npc_globalmessage(name,mes);	// グローバルメッセージ送信

	return 0;
}

/////////////////////////////////////////////////////////////////////
// NPC waiting room (chat room)
//

/// Creates a waiting room (chat room) for this npc.
///
/// waitingroom "<title>",<limit>,<trigger>,"<event>";
/// waitingroom "<title>",<limit>,"<event>",<trigger>;
/// waitingroom "<title>",<limit>,"<event>";
/// waitingroom "<title>",<limit>;
BUILDIN_FUNC(waitingroom)
{
	struct npc_data* nd;
	const char* title;
	const char* ev = "";
	int limit;
	int trigger = 0;
	int pub = 1;

	title = script_getstr(st, 2);
	limit = script_getnum(st, 3);

	if( script_hasdata(st,5) )
	{
		struct script_data* last = script_getdata(st, 5);
		get_val(st, last);
		if( data_isstring(last) )
		{// ,<trigger>,"<event>"
			trigger = script_getnum(st, 4);
			ev = script_getstr(st, 5);
		}
		else
		{// ,"<event>",<trigger>
			ev = script_getstr(st, 4);
			trigger = script_getnum(st,5);
		}
	}
	else if( script_hasdata(st,4) )
	{// ,"<event>"
		ev = script_getstr(st, 4);
		trigger = limit;
	}
	if( (nd=(struct npc_data *)map_id2bl(st->oid)) != NULL )
		chat_createnpcchat(nd, title, limit, pub, trigger, ev);
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
	return 0;
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
		return 0;
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
	return 0;
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
	TBL_PC* sd;

	nd = (struct npc_data *)map_id2bl(st->oid);
	if( nd == NULL || (cd=(struct chat_data *)map_id2bl(nd->chat_id)) == NULL )
		return 0;

	map_name = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	n = cd->trigger&0x7f;

	if( script_hasdata(st,5) )
		n = script_getnum(st,5);

	for( i = 0; i < n && cd->users > 0; i++ )
	{
		sd = cd->usersd[0];
		if( sd == NULL )
		{
			ShowDebug("script:warpwaitingpc: no user in chat room position 0 (cd->users=%d,%d/%d)\n", cd->users, i, n);
			mapreg_setreg(add_str("$@warpwaitingpc")+(i<<24), 0);
			continue;// Broken npc chat room?
		}

		mapreg_setreg(add_str("$@warpwaitingpc")+(i<<24), sd->bl.id);

		if( strcmp(map_name,"Random") == 0 )
			pc_randomwarp(sd,3);
		else if( strcmp(map_name,"SavePoint") == 0 )
		{
			if( map[sd->bl.m].flag.noteleport )
				return 0;// can't teleport on this map

			pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 3);
		}
		else
			pc_setpos(sd, mapindex_name2id(map_name), x, y, 0);
	}
	mapreg_setreg(add_str("$@warpwaitingpcnum"), i);
	return 0;
}

/////////////////////////////////////////////////////////////////////
// ...
//

/*==========================================
 * RIDのアタッチ
 *------------------------------------------*/
BUILDIN_FUNC(attachrid)
{
	st->rid=script_getnum(st,2);
	script_pushint(st,(map_id2sd(st->rid)!=NULL));
	return 0;
}
/*==========================================
 * RIDのデタッチ
 *------------------------------------------*/
BUILDIN_FUNC(detachrid)
{
	st->rid=0;
	return 0;
}
/*==========================================
 * 存在チェック
 *------------------------------------------*/
BUILDIN_FUNC(isloggedin)
{
	TBL_PC* sd = map_id2sd(script_getnum(st,2));
	if (script_hasdata(st,3) && sd &&
		sd->status.char_id != script_getnum(st,3))
		sd = NULL;
	push_val(st->stack,C_INT,sd!=NULL);
	return 0;
}


/*==========================================
 *
 *------------------------------------------*/
BUILDIN_FUNC(setmapflagnosave)
{
	int m,x,y;
	unsigned short mapindex;
	const char *str,*str2;

	str=script_getstr(st,2);
	str2=script_getstr(st,3);
	x=script_getnum(st,4);
	y=script_getnum(st,5);
	m = map_mapname2mapid(str);
	mapindex = mapindex_name2id(str2);	
	
	if(m >= 0 && mapindex) {
		map[m].flag.nosave=1;
		map[m].save.map=mapindex;
		map[m].save.x=x;
		map[m].save.y=y;
	}

	return 0;
}

BUILDIN_FUNC(getmapflag)
{
	int m,i;
	const char *str;

	str=script_getstr(st,2);
	i=script_getnum(st,3);

	m = map_mapname2mapid(str);
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:			script_pushint(st,map[m].flag.nomemo); break;
			case MF_NOTELEPORT:		script_pushint(st,map[m].flag.noteleport); break;
			case MF_NOBRANCH:		script_pushint(st,map[m].flag.nobranch); break;
			case MF_NOPENALTY:		script_pushint(st,map[m].flag.noexppenalty); break;
			case MF_NOZENYPENALTY:	script_pushint(st,map[m].flag.nozenypenalty); break;
			case MF_PVP:			script_pushint(st,map[m].flag.pvp); break;
			case MF_PVP_NOPARTY:	script_pushint(st,map[m].flag.pvp_noparty); break;
			case MF_PVP_NOGUILD:	script_pushint(st,map[m].flag.pvp_noguild); break;
			case MF_GVG:			script_pushint(st,map[m].flag.gvg); break;
			case MF_GVG_NOPARTY:	script_pushint(st,map[m].flag.gvg_noparty); break;
			case MF_GVG_DUNGEON:	script_pushint(st,map[m].flag.gvg_dungeon); break;
			case MF_GVG_CASTLE:		script_pushint(st,map[m].flag.gvg_castle); break;
			case MF_NOTRADE:		script_pushint(st,map[m].flag.notrade); break;
			case MF_NODROP:			script_pushint(st,map[m].flag.nodrop); break;
			case MF_NOSKILL:		script_pushint(st,map[m].flag.noskill); break;
			case MF_NOWARP:			script_pushint(st,map[m].flag.nowarp); break;
			case MF_NOICEWALL:		script_pushint(st,map[m].flag.noicewall); break;
			case MF_SNOW:			script_pushint(st,map[m].flag.snow); break;
			case MF_CLOUDS:			script_pushint(st,map[m].flag.clouds); break;
			case MF_CLOUDS2:		script_pushint(st,map[m].flag.clouds2); break;
			case MF_FOG:			script_pushint(st,map[m].flag.fog); break;
			case MF_FIREWORKS:		script_pushint(st,map[m].flag.fireworks); break;
			case MF_SAKURA:			script_pushint(st,map[m].flag.sakura); break;
			case MF_LEAVES:			script_pushint(st,map[m].flag.leaves); break;
			case MF_RAIN:			script_pushint(st,map[m].flag.rain); break;
			case MF_INDOORS:		script_pushint(st,map[m].flag.indoors); break;
			case MF_NIGHTENABLED:	script_pushint(st,map[m].flag.nightenabled); break;
			case MF_NOGO:			script_pushint(st,map[m].flag.nogo); break;
			case MF_NOBASEEXP:		script_pushint(st,map[m].flag.nobaseexp); break;
			case MF_NOJOBEXP:		script_pushint(st,map[m].flag.nojobexp); break;
			case MF_NOMOBLOOT:		script_pushint(st,map[m].flag.nomobloot); break;
			case MF_NOMVPLOOT:		script_pushint(st,map[m].flag.nomvploot); break;
			case MF_NORETURN:		script_pushint(st,map[m].flag.noreturn); break;
			case MF_NOWARPTO:		script_pushint(st,map[m].flag.nowarpto); break;
			case MF_NIGHTMAREDROP:	script_pushint(st,map[m].flag.pvp_nightmaredrop); break;
			case MF_RESTRICTED:		script_pushint(st,map[m].flag.restricted); break;
			case MF_NOCOMMAND:		script_pushint(st,map[m].nocommand); break;
			case MF_JEXP:			script_pushint(st,map[m].jexp); break;
			case MF_BEXP:			script_pushint(st,map[m].bexp); break;
			case MF_NOVENDING:		script_pushint(st,map[m].flag.novending); break;
			case MF_LOADEVENT:		script_pushint(st,map[m].flag.loadevent); break;
			case MF_NOCHAT:			script_pushint(st,map[m].flag.nochat); break;
			case MF_PARTYLOCK:		script_pushint(st,map[m].flag.partylock); break;
			case MF_GUILDLOCK:		script_pushint(st,map[m].flag.guildlock); break;
		}
	}

	return 0;
}

BUILDIN_FUNC(setmapflag)
{
	int m,i;
	const char *str;
	const char *val=NULL;

	str=script_getstr(st,2);
	i=script_getnum(st,3);
	if(script_hasdata(st,4)){
		val=script_getstr(st,4);
	}
	m = map_mapname2mapid(str);
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:        map[m].flag.nomemo=1; break;
			case MF_NOTELEPORT:    map[m].flag.noteleport=1; break;
			case MF_NOBRANCH:      map[m].flag.nobranch=1; break;
			case MF_NOPENALTY:     map[m].flag.noexppenalty=1; map[m].flag.nozenypenalty=1; break;
			case MF_NOZENYPENALTY: map[m].flag.nozenypenalty=1; break;
			case MF_PVP:           map[m].flag.pvp=1; break;
			case MF_PVP_NOPARTY:   map[m].flag.pvp_noparty=1; break;
			case MF_PVP_NOGUILD:   map[m].flag.pvp_noguild=1; break;
			case MF_GVG:           map[m].flag.gvg=1; break;
			case MF_GVG_NOPARTY:   map[m].flag.gvg_noparty=1; break;
			case MF_GVG_DUNGEON:   map[m].flag.gvg_dungeon=1; break;
			case MF_GVG_CASTLE:    map[m].flag.gvg_castle=1; break;
			case MF_NOTRADE:       map[m].flag.notrade=1; break;
			case MF_NODROP:        map[m].flag.nodrop=1; break;
			case MF_NOSKILL:       map[m].flag.noskill=1; break;
			case MF_NOWARP:        map[m].flag.nowarp=1; break;
			case MF_NOICEWALL:     map[m].flag.noicewall=1; break;
			case MF_SNOW:          map[m].flag.snow=1; break;
			case MF_CLOUDS:        map[m].flag.clouds=1; break;
			case MF_CLOUDS2:       map[m].flag.clouds2=1; break;
			case MF_FOG:           map[m].flag.fog=1; break;
			case MF_FIREWORKS:     map[m].flag.fireworks=1; break;
			case MF_SAKURA:        map[m].flag.sakura=1; break;
			case MF_LEAVES:        map[m].flag.leaves=1; break;
			case MF_RAIN:          map[m].flag.rain=1; break;
			case MF_INDOORS:       map[m].flag.indoors=1; break;
			case MF_NIGHTENABLED:  map[m].flag.nightenabled=1; break;
			case MF_NOGO:          map[m].flag.nogo=1; break;
			case MF_NOBASEEXP:     map[m].flag.nobaseexp=1; break;
			case MF_NOJOBEXP:      map[m].flag.nojobexp=1; break;
			case MF_NOMOBLOOT:     map[m].flag.nomobloot=1; break;
			case MF_NOMVPLOOT:     map[m].flag.nomvploot=1; break;
			case MF_NORETURN:      map[m].flag.noreturn=1; break;
			case MF_NOWARPTO:      map[m].flag.nowarpto=1; break;
			case MF_NIGHTMAREDROP: map[m].flag.pvp_nightmaredrop=1; break;
			case MF_RESTRICTED:    map[m].flag.restricted=1; break;
			case MF_NOCOMMAND:     map[m].nocommand = (!val || atoi(val) <= 0) ? 100 : atoi(val); break;
			case MF_JEXP:          map[m].jexp = (!val || atoi(val) < 0) ? 100 : atoi(val); break;
			case MF_BEXP:          map[m].bexp = (!val || atoi(val) < 0) ? 100 : atoi(val); break;
			case MF_NOVENDING:     map[m].flag.novending=1; break;
			case MF_LOADEVENT:     map[m].flag.loadevent=1; break;
			case MF_NOCHAT:        map[m].flag.nochat=1; break;
			case MF_PARTYLOCK:     map[m].flag.partylock=1; break;
			case MF_GUILDLOCK:     map[m].flag.guildlock=1; break;
		}
	}

	return 0;
}

BUILDIN_FUNC(removemapflag)
{
	int m,i;
	const char *str;

	str=script_getstr(st,2);
	i=script_getnum(st,3);
	m = map_mapname2mapid(str);
	if(m >= 0) {
		switch(i) {
			case MF_NOMEMO:        map[m].flag.nomemo=0; break;
			case MF_NOTELEPORT:    map[m].flag.noteleport=0; break;
			case MF_NOSAVE:        map[m].flag.nosave=0; break;
			case MF_NOBRANCH:      map[m].flag.nobranch=0; break;
			case MF_NOPENALTY:     map[m].flag.noexppenalty=0; map[m].flag.nozenypenalty=0; break;
			case MF_PVP:           map[m].flag.pvp=0; break;
			case MF_PVP_NOPARTY:   map[m].flag.pvp_noparty=0; break;
			case MF_PVP_NOGUILD:   map[m].flag.pvp_noguild=0; break;
			case MF_GVG:           map[m].flag.gvg=0; break;
			case MF_GVG_NOPARTY:   map[m].flag.gvg_noparty=0; break;
			case MF_GVG_DUNGEON:   map[m].flag.gvg_dungeon=0; break;
			case MF_GVG_CASTLE:    map[m].flag.gvg_castle=0; break;
			case MF_NOZENYPENALTY: map[m].flag.nozenypenalty=0; break;
			case MF_NOTRADE:       map[m].flag.notrade=0; break;
			case MF_NODROP:        map[m].flag.nodrop=0; break;
			case MF_NOSKILL:       map[m].flag.noskill=0; break;
			case MF_NOWARP:        map[m].flag.nowarp=0; break;
			case MF_NOICEWALL:     map[m].flag.noicewall=0; break;
			case MF_SNOW:          map[m].flag.snow=0; break;
			case MF_CLOUDS:        map[m].flag.clouds=0; break;
			case MF_CLOUDS2:       map[m].flag.clouds2=0; break;
			case MF_FOG:           map[m].flag.fog=0; break;
			case MF_FIREWORKS:     map[m].flag.fireworks=0; break;
			case MF_SAKURA:        map[m].flag.sakura=0; break;
			case MF_LEAVES:        map[m].flag.leaves=0; break;
			case MF_RAIN:          map[m].flag.rain=0; break;
			case MF_INDOORS:       map[m].flag.indoors=0; break;
			case MF_NIGHTENABLED:  map[m].flag.nightenabled=0; break;
			case MF_NOGO:          map[m].flag.nogo=0; break;
			case MF_NOBASEEXP:     map[m].flag.nobaseexp=0; break;
			case MF_NOJOBEXP:      map[m].flag.nojobexp=0; break;
			case MF_NOMOBLOOT:     map[m].flag.nomobloot=0; break;
			case MF_NOMVPLOOT:     map[m].flag.nomvploot=0; break;
			case MF_NORETURN:      map[m].flag.noreturn=0; break;
			case MF_NOWARPTO:      map[m].flag.nowarpto=0; break;
			case MF_NIGHTMAREDROP: map[m].flag.pvp_nightmaredrop=0; break;
			case MF_RESTRICTED:    map[m].flag.restricted=0; break;
			case MF_NOCOMMAND:     map[m].nocommand=0; break;
			case MF_JEXP:          map[m].jexp=100; break;
			case MF_BEXP:          map[m].bexp=100; break;
			case MF_NOVENDING:     map[m].flag.novending=0; break;
			case MF_LOADEVENT:     map[m].flag.loadevent=0; break;
			case MF_NOCHAT:        map[m].flag.nochat=0; break;
			case MF_PARTYLOCK:     map[m].flag.partylock=0; break;
			case MF_GUILDLOCK:     map[m].flag.guildlock=0; break;
		}
	}

	return 0;
}

BUILDIN_FUNC(pvpon)
{
	int m;
	const char *str;
	TBL_PC* sd = NULL;
	struct s_mapiterator* iter;

	str = script_getstr(st,2);
	m = map_mapname2mapid(str);
	if( m < 0 || map[m].flag.pvp )
		return 0; // nothing to do

	map[m].flag.pvp = 1;
	clif_send0199(m,1);

	if(battle_config.pk_mode) // disable ranking functions if pk_mode is on [Valaris]
		return 0;

	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
	{
		if( sd->bl.m != m || sd->pvp_timer != -1 )
			continue; // not applicable

		sd->pvp_timer = add_timer(gettick()+200,pc_calc_pvprank_timer,sd->bl.id,0);
		sd->pvp_rank = 0;
		sd->pvp_lastusers = 0;
		sd->pvp_point = 5;
		sd->pvp_won = 0;
		sd->pvp_lost = 0;
	}
	mapit_free(iter);

	return 0;
}

static int buildin_pvpoff_sub(struct block_list *bl,va_list ap)
{
	TBL_PC* sd = (TBL_PC*)bl;
	clif_pvpset(sd, 0, 0, 2);
	if (sd->pvp_timer != -1) {
		delete_timer(sd->pvp_timer, pc_calc_pvprank_timer);
		sd->pvp_timer = INVALID_TIMER;
	}
	return 0;
}

BUILDIN_FUNC(pvpoff)
{
	int m;
	const char *str;

	str=script_getstr(st,2);
	m = map_mapname2mapid(str);
	if(m < 0 || !map[m].flag.pvp)
		return 0; //fixed Lupus

	map[m].flag.pvp = 0;
	clif_send0199(m,0);

	if(battle_config.pk_mode) // disable ranking options if pk_mode is on [Valaris]
		return 0;
	
	map_foreachinmap(buildin_pvpoff_sub, m, BL_PC);
	return 0;
}

BUILDIN_FUNC(gvgon)
{
	int m;
	const char *str;

	str=script_getstr(st,2);
	m = map_mapname2mapid(str);
	if(m >= 0 && !map[m].flag.gvg) {
		map[m].flag.gvg = 1;
		clif_send0199(m,3);
	}

	return 0;
}
BUILDIN_FUNC(gvgoff)
{
	int m;
	const char *str;

	str=script_getstr(st,2);
	m = map_mapname2mapid(str);
	if(m >= 0 && map[m].flag.gvg) {
		map[m].flag.gvg = 0;
		clif_send0199(m,0);
	}

	return 0;
}
/*==========================================
 *	Shows an emoticon on top of the player/npc
 *	emotion emotion#, <target: 0 - NPC, 1 - PC>
 *------------------------------------------*/
//Optional second parameter added by [Skotlex]
BUILDIN_FUNC(emotion)
{
	int type;
	int player=0;
	
	type=script_getnum(st,2);
	if(type < 0 || type > 100)
		return 0;

	if( script_hasdata(st,3) )
		player=script_getnum(st,3);
	
	if (player) {
		TBL_PC *sd = script_rid2sd(st);
		if (sd)
			clif_emotion(&sd->bl,type);
	} else
		clif_emotion(map_id2bl(st->oid),type);
	return 0;
}

static int buildin_maprespawnguildid_sub_pc(struct map_session_data* sd, va_list ap)
{
	int m=va_arg(ap,int);
	int g_id=va_arg(ap,int);
	int flag=va_arg(ap,int);

	if(!sd || sd->bl.m != m)
		return 0;
	if(
		(sd->status.guild_id == g_id && flag&1) || //Warp out owners
		(sd->status.guild_id != g_id && flag&2) || //Warp out outsiders
		(sd->status.guild_id == 0)	// Warp out players not in guild [Valaris]
	)
		pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,3);
	return 1;
}

static int buildin_maprespawnguildid_sub_mob(struct block_list *bl,va_list ap)
{
	struct mob_data *md=(struct mob_data *)bl;

	if(!md->guardian_data && md->class_ != MOBID_EMPERIUM)
		status_kill(bl);

	return 0;
}

BUILDIN_FUNC(maprespawnguildid)
{
	const char *mapname=script_getstr(st,2);
	int g_id=script_getnum(st,3);
	int flag=script_getnum(st,4);

	int m=map_mapname2mapid(mapname);

	if(m == -1)
		return 0;

	//Catch ALL players (in case some are 'between maps' on execution time)
	map_foreachpc(buildin_maprespawnguildid_sub_pc,m,g_id,flag);
	if (flag&4) //Remove script mobs.
		map_foreachinmap(buildin_maprespawnguildid_sub_mob,m,BL_MOB);
	return 0;
}

BUILDIN_FUNC(agitstart)
{
	if(agit_flag==1) return 0;      // Agit already Start.
	agit_flag=1;
	guild_agit_start();
	return 0;
}

BUILDIN_FUNC(agitend)
{
	if(agit_flag==0) return 0;      // Agit already End.
	agit_flag=0;
	guild_agit_end();
	return 0;
}

/*==========================================
 * Returns whether woe is on or off.	// choice script
 *------------------------------------------*/
BUILDIN_FUNC(agitcheck)
{
	script_pushint(st,agit_flag);
	return 0;
}

/// Sets the guild_id of this npc.
///
/// flagemblem <guild_id>;
BUILDIN_FUNC(flagemblem)
{
	TBL_NPC* nd;
	int g_id=script_getnum(st,2);

	if(g_id < 0) return 0;

	nd = (TBL_NPC*)map_id2nd(st->oid);
	if( nd == NULL )
	{
		ShowError("script:flagemblem: npc %d not found\n", st->oid);
	}
	else if( nd->subtype != SCRIPT )
	{
		ShowError("script:flagemblem: unexpected subtype %d for npc %d '%s'\n", nd->subtype, st->oid, nd->exname);
	}
	else
	{
		nd->u.scr.guild_id = g_id;
		clif_guild_emblem_area(&nd->bl);
	}
	return 0;
}

BUILDIN_FUNC(getcastlename)
{
	const char* mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	struct guild_castle* gc = guild_mapname2gc(mapname);
	const char* name = (gc) ? gc->castle_name : "";
	script_pushstrcopy(st,name);
	return 0;
}

BUILDIN_FUNC(getcastledata)
{
	const char* mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	int index = script_getnum(st,3);

	struct guild_castle* gc = guild_mapname2gc(mapname);

	if(script_hasdata(st,4) && index==0 && gc) {
		const char* event = script_getstr(st,4);
		check_event(st, event);
		guild_addcastleinfoevent(gc->castle_id,17,event);
	}

	if(gc){
		switch(index){
			case 0: {
				int i;
				for(i=1;i<18;i++) // Initialize[AgitInit]
					guild_castledataload(gc->castle_id,i);
				} break;
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
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
				script_pushint(st,gc->guardian[index-10].visible); break;
			default:
				script_pushint(st,0); break;
		}
		return 0;
	}
	script_pushint(st,0);
	return 0;
}

BUILDIN_FUNC(setcastledata)
{
	const char* mapname = mapindex_getmapname(script_getstr(st,2),NULL);
	int index = script_getnum(st,3);
	int value = script_getnum(st,4);

	struct guild_castle* gc = guild_mapname2gc(mapname);

	if(gc) {
		// Save Data byself First
		switch(index){
			case 1:
				gc->guild_id = value; break;
			case 2:
				gc->economy = value; break;
			case 3:
				gc->defense = value; break;
			case 4:
				gc->triggerE = value; break;
			case 5:
				gc->triggerD = value; break;
			case 6:
				gc->nextTime = value; break;
			case 7:
				gc->payTime = value; break;
			case 8:
				gc->createTime = value; break;
			case 9:
				gc->visibleC = value; break;
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
				gc->guardian[index-10].visible = value; break;
			default:
				return 0;
		}
		guild_castledatasave(gc->castle_id,index,value);
	}
	return 0;
}

/* =====================================================================
 * ギルド情報を要求する
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
	return 0;
}

/// Returns the number of cards that have been compounded onto the specified equipped item.
/// getequipcardcnt(<equipment slot>);
BUILDIN_FUNC(getequipcardcnt)
{
	int i=-1,j,num;
	TBL_PC *sd;
	int count;

	num=script_getnum(st,2);
	sd=script_rid2sd(st);
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);

	if (i < 0) {
		script_pushint(st,0);
		return 0;
	}

	if(itemdb_isspecial(sd->status.inventory[i].card[0]))
	{
		script_pushint(st,0);
		return 0;
	}

	count = 0;
	for( j = 0; j < MAX_SLOTS; j++ )
		if( sd->status.inventory[i].card[j] && itemdb_type(sd->status.inventory[i].card[j]) == IT_CARD )
			count++;

	script_pushint(st,count);
	return 0;
}

/// Removes all cards from the item found in the specified equipment slot of the invoking character,
/// and give them to the character. If any cards were removed in this manner, it will also show a success effect.
/// successremovecards <slot>;
BUILDIN_FUNC(successremovecards)
{
	int i=-1,j,c,cardflag=0;

	TBL_PC* sd = script_rid2sd(st);
	int num = script_getnum(st,2);

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);

	if (i < 0) {
		return 0;
	}

	if(itemdb_isspecial(sd->status.inventory[i].card[0])) 
		return 0;

	for( c = MAX_SLOTS - 1; c >= 0; --c )
	{
		if( sd->status.inventory[i].card[c] && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD )
		{// extract this card from the item
			int flag;
			struct item item_tmp;
			cardflag = 1;
			item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].card[c];
			item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=0;
			item_tmp.attribute=0;
			for (j = 0; j < MAX_SLOTS; j++)
				item_tmp.card[j]=0;

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", item_tmp.nameid, 1, NULL);

			if((flag=pc_additem(sd,&item_tmp,1))){	// 持てないならドロップ
				clif_additem(sd,0,0,flag);
				map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
		}
	}

	if(cardflag == 1)
	{	// カードを取り除いたアイテム所得
		int flag;
		struct item item_tmp;
		item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].nameid;
		item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=sd->status.inventory[i].refine;
		item_tmp.attribute=sd->status.inventory[i].attribute;
		for (j = 0; j < MAX_SLOTS; j++)
			item_tmp.card[j]=0;

		//Logs items, got from (N)PC scripts [Lupus]
		if(log_config.enable_logs&0x40)
			log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -1, &sd->status.inventory[i]);

		pc_delitem(sd,i,1,0);

		//Logs items, got from (N)PC scripts [Lupus]
		if(log_config.enable_logs&0x40)
			log_pick_pc(sd, "N", item_tmp.nameid, 1, &item_tmp);

		if((flag=pc_additem(sd,&item_tmp,1))){	// もてないならドロップ
			clif_additem(sd,0,0,flag);
			map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
		}

		clif_misceffect(&sd->bl,3);
	}
	return 0;
}

/// Removes all cards from the item found in the specified equipment slot of the invoking character.
/// failedremovecards <slot>, <type>;
/// <type>=0 : will destroy both the item and the cards.
/// <type>=1 : will keep the item, but destroy the cards.
/// <type>=2 : will keep the cards, but destroy the item.
/// <type>=? : will just display the failure effect.
BUILDIN_FUNC(failedremovecards)
{
	int i=-1,j,c,cardflag=0;

	TBL_PC* sd = script_rid2sd(st);
	int num = script_getnum(st,2);
	int typefail = script_getnum(st,3);

	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);

	if (i < 0)
		return 0;

	if(itemdb_isspecial(sd->status.inventory[i].card[0]))
		return 0;

	for( c = MAX_SLOTS - 1; c >= 0; --c )
	{
		if( sd->status.inventory[i].card[c] && itemdb_type(sd->status.inventory[i].card[c]) == IT_CARD )
		{
			cardflag = 1;

			if(typefail == 2)
			{// add cards to inventory, clear 
				int flag;
				struct item item_tmp;
				item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].card[c];
				item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=0;
				item_tmp.attribute=0;
				for (j = 0; j < MAX_SLOTS; j++)
					item_tmp.card[j]=0;

				//Logs items, got from (N)PC scripts [Lupus]
				if(log_config.enable_logs&0x40)
					log_pick_pc(sd, "N", item_tmp.nameid, 1, NULL);

				if((flag=pc_additem(sd,&item_tmp,1))){
					clif_additem(sd,0,0,flag);
					map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
				}
			}
		}
	}

	if(cardflag == 1)
	{
		if(typefail == 0 || typefail == 2){	// 武具損失
			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -1, &sd->status.inventory[i]);

			pc_delitem(sd,i,1,0);
		}
		if(typefail == 1){	// カードのみ損失（武具を返す）
			int flag;
			struct item item_tmp;
			item_tmp.id=0,item_tmp.nameid=sd->status.inventory[i].nameid;
			item_tmp.equip=0,item_tmp.identify=1,item_tmp.refine=sd->status.inventory[i].refine;
			item_tmp.attribute=sd->status.inventory[i].attribute;

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -1, &sd->status.inventory[i]);

			for (j = 0; j < MAX_SLOTS; j++)
				item_tmp.card[j]=0;
			pc_delitem(sd,i,1,0);

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", item_tmp.nameid, 1, &item_tmp);

			if((flag=pc_additem(sd,&item_tmp,1))){
				clif_additem(sd,0,0,flag);
				map_addflooritem(&item_tmp,1,sd->bl.m,sd->bl.x,sd->bl.y,0,0,0,0);
			}
		}
		clif_misceffect(&sd->bl,2);
	}

	return 0;
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
		return 0;

	if(!(index=mapindex_name2id(str)))
		return 0;

	switch(check_val){
		case 1:
			g = guild_search(check_ID);
			if (g){
				for( i=0; i < g->max_member; i++)
				{
					if(g->member[i].sd && g->member[i].sd->bl.m==m){
						pc_setpos(g->member[i].sd,index,x,y,3);
					}
				}
			}
			break;
		case 2:
			p = party_search(check_ID);
			if(p){
				for(i=0;i<MAX_PARTY; i++){
					if(p->data[i].sd && p->data[i].sd->bl.m == m){
						pc_setpos(p->data[i].sd,index,x,y,3);
					}
				}
			}
			break;
		default:
			map_foreachinmap(buildin_areawarp_sub,m,BL_PC,index,x,y);
			break;
	}

	return 0;
}

static int buildin_mobcount_sub(struct block_list *bl,va_list ap)	// Added by RoVeRT
{
	char *event=va_arg(ap,char *);
	if(strcmp(event,((struct mob_data *)bl)->npc_event)==0)
		return 1;
	return 0;
}

BUILDIN_FUNC(mobcount)	// Added by RoVeRT
{
	const char *mapname,*event;
	int m;
	mapname=script_getstr(st,2);
	event=script_getstr(st,3);
	check_event(st, event);

	if( (m=map_mapname2mapid(mapname))<0 ) {
		script_pushint(st,-1);
		return 0;
	}

	script_pushint(st,map_foreachinmap(buildin_mobcount_sub, m, BL_MOB, event));

	return 0;
}
BUILDIN_FUNC(marriage)
{
	const char *partner=script_getstr(st,2);
	TBL_PC *sd=script_rid2sd(st);
	TBL_PC *p_sd=map_nick2sd(partner);

	if(sd==NULL || p_sd==NULL || pc_marriage(sd,p_sd) < 0){
		script_pushint(st,0);
		return 0;
	}
	script_pushint(st,1);
	return 0;
}
BUILDIN_FUNC(wedding_effect)
{
	TBL_PC *sd=script_rid2sd(st);
	struct block_list *bl;

	if(sd==NULL) {
		bl=map_id2bl(st->oid);
	} else
		bl=&sd->bl;
	clif_wedding_effect(bl);
	return 0;
}
BUILDIN_FUNC(divorce)
{
	TBL_PC *sd=script_rid2sd(st);
	if(sd==NULL || pc_divorce(sd) < 0){
		script_pushint(st,0);
		return 0;
	}
	script_pushint(st,1);
	return 0;
}

BUILDIN_FUNC(ispartneron)
{
	TBL_PC *sd=script_rid2sd(st);
	TBL_PC *p_sd=NULL;

	if(sd==NULL || !pc_ismarried(sd) ||
            (p_sd=map_charid2sd(sd->status.partner_id)) == NULL) {
		script_pushint(st,0);
		return 0;
	}

	script_pushint(st,1);
	return 0;
}

BUILDIN_FUNC(getpartnerid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return 0;
    }

    script_pushint(st,sd->status.partner_id);
    return 0;
}

BUILDIN_FUNC(getchildid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return 0;
    }

    script_pushint(st,sd->status.child);
    return 0;
}

BUILDIN_FUNC(getmotherid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return 0;
    }

    script_pushint(st,sd->status.mother);
    return 0;
}

BUILDIN_FUNC(getfatherid)
{
    TBL_PC *sd=script_rid2sd(st);
    if (sd == NULL) {
        script_pushint(st,0);
        return 0;
    }

    script_pushint(st,sd->status.father);
    return 0;
}

BUILDIN_FUNC(warppartner)
{
	int x,y;
	unsigned short mapindex;
	const char *str;
	TBL_PC *sd=script_rid2sd(st);
	TBL_PC *p_sd=NULL;

	if(sd==NULL || !pc_ismarried(sd) ||
   	(p_sd=map_charid2sd(sd->status.partner_id)) == NULL) {
		script_pushint(st,0);
		return 0;
	}

	str=script_getstr(st,2);
	x=script_getnum(st,3);
	y=script_getnum(st,4);

	mapindex = mapindex_name2id(str);
	if (mapindex) {
		pc_setpos(p_sd,mapindex,x,y,0);
		script_pushint(st,1);
	} else
		script_pushint(st,0);
	return 0;
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
		script_pushint(st,0);
		return 0;
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
	return 0;
}

/*==========================================
 * Summon guardians [Valaris]
 * guardian("<map name>",<x>,<y>,"<name to show>",<mob id>{,"<event label>"}{,<guardian index>}) -> <id>
 *------------------------------------------*/
BUILDIN_FUNC(guardian)
{
	int class_=0,x=0,y=0,guardian=0;
	const char *str,*map,*evt="";
	struct script_data *data;
	bool has_index = false;

	map	  =script_getstr(st,2);
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
		data=script_getdata(st,7);
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
			return 1;
		}
	}

	check_event(st, evt);
	script_pushint(st, mob_spawn_guardian(map,x,y,str,class_,evt,guardian,has_index));

	return 0;
}
/*==========================================
 * Invisible Walls [Zephyrus]
 *------------------------------------------*/
BUILDIN_FUNC(setwall)
{
	const char *map, *name;
	int x, y, m, size, dir;
	bool shootable;
	
	map = script_getstr(st,2);
	x = script_getnum(st,3);
	y = script_getnum(st,4);
	size = script_getnum(st,5);
	dir = script_getnum(st,6);
	shootable = script_getnum(st,7);
	name = script_getstr(st,8);

	if( (m = map_mapname2mapid(map)) < 0 )
		return 0; // Invalid Map

	map_iwall_set(m, x, y, size, dir, shootable, name);
	return 0;
}
BUILDIN_FUNC(delwall)
{
	const char *name = script_getstr(st,2);
	map_iwall_remove(name);

	return 0;
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
		return 0;
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

	return 0;
}

/*==========================================
 * IDからItem名
 *------------------------------------------*/
BUILDIN_FUNC(getitemname)
{
	int item_id=0;
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
		return 0;
	}
	item_name=(char *)aMallocA(ITEM_NAME_LENGTH*sizeof(char));

	memcpy(item_name, i_data->jname, ITEM_NAME_LENGTH);
	script_pushstr(st,item_name);
	return 0;
}
/*==========================================
 * Returns number of slots an item has. [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(getitemslots)
{
	int item_id;
	struct item_data *i_data;

	item_id=script_getnum(st,2);

	i_data = itemdb_exists(item_id);

	if (i_data)
		script_pushint(st,i_data->slot);
	else
		script_pushint(st,-1);
	return 0;
}

/*==========================================
 * Returns some values of an item [Lupus]
 * Price, Weight, etc...
	getiteminfo(itemID,n), where n
		0 value_buy;
		1 value_sell;
		2 type;
		3 maxchance = Max drop chance of this item e.g. 1 = 0.01% , etc..
				if = 0, then monsters don't drop it at all (rare or a quest item)
				if = 10000, then this item is sold in NPC shops only
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
 *------------------------------------------*/
BUILDIN_FUNC(getiteminfo)
{
	int item_id,n;
	int *item_arr;
	struct item_data *i_data;

	item_id	= script_getnum(st,2);
	n	= script_getnum(st,3);
	i_data = itemdb_exists(item_id);

	if (i_data && n>=0 && n<14) {
		item_arr = (int*)&i_data->value_buy;
		script_pushint(st,item_arr[n]);
	} else
		script_pushint(st,-1);
	return 0;
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
				if = 10000, then this item is sold in NPC shops only
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
	int item_id,n,value;
	int *item_arr;
	struct item_data *i_data;

	item_id	= script_getnum(st,2);
	n	= script_getnum(st,3);
	value	= script_getnum(st,4);
	i_data = itemdb_exists(item_id);

	if (i_data && n>=0 && n<14) {
		item_arr = (int*)&i_data->value_buy;
		item_arr[n] = value;
		script_pushint(st,value);
	} else
		script_pushint(st,-1);
	return 0;
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

	num=script_getnum(st,2);
	slot=script_getnum(st,3);
	sd=script_rid2sd(st);
	if (num > 0 && num <= ARRAYLENGTH(equip))
		i=pc_checkequip(sd,equip[num-1]);
	if(i >= 0 && slot>=0 && slot<4)
		script_pushint(st,sd->status.inventory[i].card[slot]);
	else
		script_pushint(st,0);

	return 0;
}

/*==========================================
 * petskillbonus [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(petskillbonus)
{
	struct pet_data *pd;

	TBL_PC *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->bonus)
	{ //Clear previous bonus
		if (pd->bonus->timer != -1)
			delete_timer(pd->bonus->timer, pet_skill_bonus_timer);
	} else //init
		pd->bonus = (struct pet_bonus *) aMalloc(sizeof(struct pet_bonus));

	pd->bonus->type=script_getnum(st,2);
	pd->bonus->val=script_getnum(st,3);
	pd->bonus->duration=script_getnum(st,4);
	pd->bonus->delay=script_getnum(st,5);

	if (pd->state.skillbonus == 1)
		pd->state.skillbonus=0;	// waiting state

	// wait for timer to start
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->bonus->timer = INVALID_TIMER;
	else
		pd->bonus->timer = add_timer(gettick()+pd->bonus->delay*1000, pet_skill_bonus_timer, sd->bl.id, 0);

	return 0;
}

/*==========================================
 * pet looting [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(petloot)
{
	int max;
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);
	
	if(sd==NULL || sd->pd==NULL)
		return 0;

	max=script_getnum(st,2);

	if(max < 1)
		max = 1;	//Let'em loot at least 1 item.
	else if (max > MAX_PETLOOT_SIZE)
		max = MAX_PETLOOT_SIZE;
	
	pd = sd->pd;
	if (pd->loot != NULL)
	{	//Release whatever was there already and reallocate memory
		pet_lootitem_drop(pd, pd->msd);
		aFree(pd->loot->item);
	}
	else
		pd->loot = (struct pet_loot *)aMalloc(sizeof(struct pet_loot));

	pd->loot->item = (struct item *)aCalloc(max,sizeof(struct item));
	
	pd->loot->max=max;
	pd->loot->count = 0;
	pd->loot->weight = 0;

	return 0;
}
/*==========================================
 * PCの所持品情報読み取り
 *------------------------------------------*/
BUILDIN_FUNC(getinventorylist)
{
	TBL_PC *sd=script_rid2sd(st);
	char card_var[NAME_LENGTH];
	
	int i,j=0,k;
	if(!sd) return 0;
	for(i=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount > 0){
			pc_setreg(sd,add_str("@inventorylist_id")+(j<<24),sd->status.inventory[i].nameid);
			pc_setreg(sd,add_str("@inventorylist_amount")+(j<<24),sd->status.inventory[i].amount);
			pc_setreg(sd,add_str("@inventorylist_equip")+(j<<24),sd->status.inventory[i].equip);
			pc_setreg(sd,add_str("@inventorylist_refine")+(j<<24),sd->status.inventory[i].refine);
			pc_setreg(sd,add_str("@inventorylist_identify")+(j<<24),sd->status.inventory[i].identify);
			pc_setreg(sd,add_str("@inventorylist_attribute")+(j<<24),sd->status.inventory[i].attribute);
			for (k = 0; k < MAX_SLOTS; k++)
			{
				sprintf(card_var, "@inventorylist_card%d",k+1);
				pc_setreg(sd,add_str(card_var)+(j<<24),sd->status.inventory[i].card[k]);
			}
			j++;
		}
	}
	pc_setreg(sd,add_str("@inventorylist_count"),j);
	return 0;
}

BUILDIN_FUNC(getskilllist)
{
	TBL_PC *sd=script_rid2sd(st);
	int i,j=0;
	if(!sd) return 0;
	for(i=0;i<MAX_SKILL;i++){
		if(sd->status.skill[i].id > 0 && sd->status.skill[i].lv > 0){
			pc_setreg(sd,add_str("@skilllist_id")+(j<<24),sd->status.skill[i].id);
			pc_setreg(sd,add_str("@skilllist_lv")+(j<<24),sd->status.skill[i].lv);
			pc_setreg(sd,add_str("@skilllist_flag")+(j<<24),sd->status.skill[i].flag);
			j++;
		}
	}
	pc_setreg(sd,add_str("@skilllist_count"),j);
	return 0;
}

BUILDIN_FUNC(clearitem)
{
	TBL_PC *sd=script_rid2sd(st);
	int i;
	if(sd==NULL) return 0;
	for (i=0; i<MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {

			//Logs items, got from (N)PC scripts [Lupus]
			if(log_config.enable_logs&0x40)
				log_pick_pc(sd, "N", sd->status.inventory[i].nameid, -sd->status.inventory[i].amount, &sd->status.inventory[i]);

			pc_delitem(sd, i, sd->status.inventory[i].amount, 0);
		}
	}
	return 0;
}

/*==========================================
 * Disguise Player (returns Mob/NPC ID if success, 0 on fail)
 *------------------------------------------*/
BUILDIN_FUNC(disguise)
{
	int id;
	TBL_PC* sd = script_rid2sd(st);
	if (sd == NULL) return 0;

	id = script_getnum(st,2);

	if (mobdb_checkid(id) || npcdb_checkid(id)) {
		pc_disguise(sd, id);
		script_pushint(st,id);
	} else
		script_pushint(st,0);

	return 0;
}

/*==========================================
 * Undisguise Player (returns 1 if success, 0 on fail)
 *------------------------------------------*/
BUILDIN_FUNC(undisguise)
{
	TBL_PC* sd = script_rid2sd(st);
	if (sd == NULL) return 0;

	if (sd->disguise) {
		pc_disguise(sd, 0);
		script_pushint(st,0);
	} else {
		script_pushint(st,1);
	}
	return 0;
}

/*==========================================
 * NPCクラスチェンジ
 * classは変わりたいclass
 * typeは通常0なのかな？
 *------------------------------------------*/
BUILDIN_FUNC(classchange)
{
	int _class,type;
	struct block_list *bl=map_id2bl(st->oid);

	if(bl==NULL) return 0;

	_class=script_getnum(st,2);
	type=script_getnum(st,3);
	clif_class_change(bl,_class,type);
	return 0;
}

/*==========================================
 * NPCから発生するエフェクト
 *------------------------------------------*/
BUILDIN_FUNC(misceffect)
{
	int type;

	type=script_getnum(st,2);
	if(st->oid && st->oid != fake_nd->bl.id) {
		struct block_list *bl = map_id2bl(st->oid);
		if (bl)
			clif_misceffect2(bl,type);
	} else{
		TBL_PC *sd=script_rid2sd(st);
		if(sd)
			clif_misceffect2(&sd->bl,type);
	}
	return 0;
}
/*==========================================
 * サウンドエフェクト
 *------------------------------------------*/
BUILDIN_FUNC(soundeffect)
{
	TBL_PC* sd = script_rid2sd(st);
	const char* name = script_getstr(st,2);
	int type = script_getnum(st,3);

	if(sd)
	{
		if(!st->rid)
			clif_soundeffect(sd,map_id2bl(st->oid),name,type);
		else
			clif_soundeffect(sd,&sd->bl,name,type);
	}
	return 0;
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
	const char* name;
	int type;

	bl = (st->rid) ? &(script_rid2sd(st)->bl) : map_id2bl(st->oid);
	if (!bl)
		return 0;

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
		const char* map = script_getstr(st,4);
		map_foreachinmap(soundeffect_sub, map_mapname2mapid(map), BL_PC, name, type);
	}
	else
	if(script_hasdata(st,8))
	{	// specified part of map
		const char* map = script_getstr(st,4);
		int x0 = script_getnum(st,5);
		int y0 = script_getnum(st,6);
		int x1 = script_getnum(st,7);
		int y1 = script_getnum(st,8);
		map_foreachinarea(soundeffect_sub, map_mapname2mapid(map), x0, y0, x1, y1, BL_PC, name, type);
	}
	else
	{
		ShowError("buildin_soundeffectall: insufficient arguments for specific area broadcast.\n");
	}

	return 0;
}
/*==========================================
 * pet status recovery [Valaris] / Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(petrecovery)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	
	if (pd->recovery)
	{ //Halt previous bonus
		if (pd->recovery->timer != -1)
			delete_timer(pd->recovery->timer, pet_recovery_timer);
	} else //Init
		pd->recovery = (struct pet_recovery *)aMalloc(sizeof(struct pet_recovery));
		
	pd->recovery->type = (sc_type)script_getnum(st,2);
	pd->recovery->delay = script_getnum(st,3);
	pd->recovery->timer = INVALID_TIMER;

	return 0;
}

/*==========================================
 * pet healing [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(petheal)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != -1)
		{
			if (pd->s_skill->id)
				delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				delete_timer(pd->s_skill->timer, pet_heal_timer);
		}
	} else //init memory
		pd->s_skill = (struct pet_skill_support *) aMalloc(sizeof(struct pet_skill_support)); 
	
	pd->s_skill->id=0; //This id identifies that it IS petheal rather than pet_skillsupport
	//Use the lv as the amount to heal
	pd->s_skill->lv=script_getnum(st,2);
	pd->s_skill->delay=script_getnum(st,3);
	pd->s_skill->hp=script_getnum(st,4);
	pd->s_skill->sp=script_getnum(st,5);

	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->s_skill->timer = INVALID_TIMER;
	else
		pd->s_skill->timer = add_timer(gettick()+pd->s_skill->delay*1000,pet_heal_timer,sd->bl.id,0);

	return 0;
}

/*==========================================
 * pet attack skills [Valaris] //Rewritten by [Skotlex]
 *------------------------------------------*/
/// petskillattack <skill id>,<level>,<rate>,<bonusrate>
/// petskillattack "<skill name>",<level>,<rate>,<bonusrate>
BUILDIN_FUNC(petskillattack)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_skill_attack *)aMalloc(sizeof(struct pet_skill_attack));
				
	pd->a_skill->id=( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	pd->a_skill->lv=script_getnum(st,3);
	pd->a_skill->div_ = 0;
	pd->a_skill->rate=script_getnum(st,4);
	pd->a_skill->bonusrate=script_getnum(st,5);

	return 0;
}

/*==========================================
 * pet attack skills [Valaris]
 *------------------------------------------*/
/// petskillattack2 <skill id>,<level>,<div>,<rate>,<bonusrate>
/// petskillattack2 "<skill name>",<level>,<div>,<rate>,<bonusrate>
BUILDIN_FUNC(petskillattack2)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->a_skill == NULL)
		pd->a_skill = (struct pet_skill_attack *)aMalloc(sizeof(struct pet_skill_attack));
				
	pd->a_skill->id=( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	pd->a_skill->lv=script_getnum(st,3);
	pd->a_skill->div_ = script_getnum(st,4);
	pd->a_skill->rate=script_getnum(st,5);
	pd->a_skill->bonusrate=script_getnum(st,6);

	return 0;
}

/*==========================================
 * pet support skills [Skotlex]
 *------------------------------------------*/
/// petskillsupport <skill id>,<level>,<delay>,<hp>,<sp>
/// petskillsupport "<skill name>",<level>,<delay>,<hp>,<sp>
BUILDIN_FUNC(petskillsupport)
{
	struct pet_data *pd;
	TBL_PC *sd=script_rid2sd(st);

	if(sd==NULL || sd->pd==NULL)
		return 0;

	pd=sd->pd;
	if (pd->s_skill)
	{ //Clear previous skill
		if (pd->s_skill->timer != -1)
		{
			if (pd->s_skill->id)
				delete_timer(pd->s_skill->timer, pet_skill_support_timer);
			else
				delete_timer(pd->s_skill->timer, pet_heal_timer);
		}
	} else //init memory
		pd->s_skill = (struct pet_skill_support *) aMalloc(sizeof(struct pet_skill_support)); 
	
	pd->s_skill->id=( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	pd->s_skill->lv=script_getnum(st,3);
	pd->s_skill->delay=script_getnum(st,4);
	pd->s_skill->hp=script_getnum(st,5);
	pd->s_skill->sp=script_getnum(st,6);

	//Use delay as initial offset to avoid skill/heal exploits
	if (battle_config.pet_equip_required && pd->pet.equip == 0)
		pd->s_skill->timer = INVALID_TIMER;
	else
		pd->s_skill->timer = add_timer(gettick()+pd->s_skill->delay*1000,pet_skill_support_timer,sd->bl.id,0);

	return 0;
}

/*==========================================
 * Scripted skill effects [Celest]
 *------------------------------------------*/
/// skilleffect <skill id>,<level>
/// skilleffect "<skill name>",<level>
BUILDIN_FUNC(skilleffect)
{
	TBL_PC *sd;

	int skillid=( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int skilllv=script_getnum(st,3);
	sd=script_rid2sd(st);

	clif_skill_nodamage(&sd->bl,&sd->bl,skillid,skilllv,1);

	return 0;
}

/*==========================================
 * NPC skill effects [Valaris]
 *------------------------------------------*/
/// npcskilleffect <skill id>,<level>,<x>,<y>
/// npcskilleffect "<skill name>",<level>,<x>,<y>
BUILDIN_FUNC(npcskilleffect)
{
	struct block_list *bl= map_id2bl(st->oid);

	int skillid=( script_isstring(st,2) ? skill_name2id(script_getstr(st,2)) : script_getnum(st,2) );
	int skilllv=script_getnum(st,3);
	int x=script_getnum(st,4);
	int y=script_getnum(st,5);

	if (bl)
		clif_skill_poseffect(bl,skillid,skilllv,x,y,gettick());

	return 0;
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
		return 0;

	clif_specialeffect(bl, type, target);

	return 0;
}

BUILDIN_FUNC(specialeffect2)
{
	TBL_PC *sd=script_rid2sd(st);
	int type = script_getnum(st,2);
	enum send_target target = script_hasdata(st,3) ? (send_target)script_getnum(st,3) : AREA;

	if(sd==NULL)
		return 0;

	clif_specialeffect(&sd->bl, type, target);

	return 0;
}

/*==========================================
 * Nude [Valaris]
 *------------------------------------------*/
BUILDIN_FUNC(nude)
{
	TBL_PC *sd=script_rid2sd(st);
	int i,calcflag=0;

	if(sd==NULL)
		return 0;

	for(i=0;i<11;i++)
		if(sd->equip_index[i] >= 0) {
			if(!calcflag)
				calcflag=1;
			pc_unequipitem(sd,sd->equip_index[i],2);
		}

	if(calcflag)
		status_calc_pc(sd,0);

	return 0;
}

/*==========================================
 * gmcommand [MouseJstr]
 *
 * suggested on the forums...
 * splitted into atcommand & charcommand by [Skotlex]
 *------------------------------------------*/
BUILDIN_FUNC(atcommand)
{
	TBL_PC dummy_sd;
	TBL_PC* sd;
	int fd;
	const char* cmd;

	cmd = script_getstr(st,2);

	if (st->rid) {
		sd = script_rid2sd(st);
		fd = sd->fd;
	} else { //Use a dummy character.
		sd = &dummy_sd;
		fd = 0;

		memset(&dummy_sd, 0, sizeof(TBL_PC));
		if (st->oid)
		{
			struct block_list* bl = map_id2bl(st->oid);
			memcpy(&dummy_sd.bl, bl, sizeof(struct block_list));
			if (bl->type == BL_NPC)
				safestrncpy(dummy_sd.status.name, ((TBL_NPC*)bl)->name, NAME_LENGTH);
		}
	}

	// compatibility with previous implementation (deprecated!)
	if(cmd[0] != atcommand_symbol)
	{
		cmd += strlen(sd->status.name);
		while(*cmd != atcommand_symbol && *cmd != 0)
			cmd++;
	}

	is_atcommand_sub(fd, sd, cmd, 99);

	return 0;
}

BUILDIN_FUNC(charcommand)
{
	TBL_PC dummy_sd;
	TBL_PC* sd;
	int fd;
	const char* cmd;

	cmd = script_getstr(st,2);

	if (st->rid) {
		sd = script_rid2sd(st);
		fd = sd->fd;
	} else { //Use a dummy character.
		sd = &dummy_sd;
		fd = 0;

		memset(&dummy_sd, 0, sizeof(TBL_PC));
		if (st->oid)
		{
			struct block_list* bl = map_id2bl(st->oid);
			memcpy(&dummy_sd.bl, bl, sizeof(struct block_list));
			if (bl->type == BL_NPC)
				safestrncpy(dummy_sd.status.name, ((TBL_NPC*)bl)->name, NAME_LENGTH);
		}
	}

	// compatibility with previous implementation (deprecated!)
	if(cmd[0] != charcommand_symbol)
	{
		cmd += strlen(sd->status.name);
		while(*cmd != charcommand_symbol && *cmd != 0)
			cmd++;
	}

	is_charcommand_sub(fd, sd, cmd, 99);

	return 0;
}

/*==========================================
 * Displays a message for the player only (like system messages like "you got an apple" )
 *------------------------------------------*/
BUILDIN_FUNC(dispbottom)
{
	TBL_PC *sd=script_rid2sd(st);
	const char *message;
	message=script_getstr(st,2);
	if(sd)
		clif_disp_onlyself(sd,message,(int)strlen(message));
	return 0;
}

/*==========================================
 * All The Players Full Recovery
 * (HP/SP full restore and resurrect if need)
 *------------------------------------------*/
BUILDIN_FUNC(recovery)
{
	TBL_PC* sd;
	struct s_mapiterator* iter;

	iter = mapit_getallusers();
	for( sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); sd = (TBL_PC*)mapit_next(iter) )
	{
		if(pc_isdead(sd))
			status_revive(&sd->bl, 100, 100);
		else
			status_percent_heal(&sd->bl, 100, 100);
		clif_displaymessage(sd->fd,"You have been recovered!");
	}
	mapit_free(iter);
	return 0;
}
/*==========================================
 * Get your pet info: getpetinfo(n)  
 * n -> 0:pet_id 1:pet_class 2:pet_name
 * 3:friendly 4:hungry, 5: rename flag.
 *------------------------------------------*/
BUILDIN_FUNC(getpetinfo)
{
	TBL_PC *sd=script_rid2sd(st);
	TBL_PET *pd;
	int type=script_getnum(st,2);
	
	if(!sd || !sd->pd) {
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return 0;
	}
	pd = sd->pd;
	switch(type){
		case 0: script_pushint(st,pd->pet.pet_id); break;
		case 1: script_pushint(st,pd->pet.class_); break;
		case 2: script_pushstrcopy(st,pd->pet.name); break;
		case 3: script_pushint(st,pd->pet.intimate); break;
		case 4: script_pushint(st,pd->pet.hungry); break;
		case 5: script_pushint(st,pd->pet.rename_flag); break;
		default:
			script_pushint(st,0);
			break;
	}
	return 0;
}

/*==========================================
 * Get your homunculus info: gethominfo(n)  
 * n -> 0:hom_id 1:class 2:name
 * 3:friendly 4:hungry, 5: rename flag.
 * 6: level
 *------------------------------------------*/
BUILDIN_FUNC(gethominfo)
{
	TBL_PC *sd=script_rid2sd(st);
	TBL_HOM *hd;
	int type=script_getnum(st,2);

	hd = sd?sd->hd:NULL;
	if(!merc_is_hom_active(hd))
	{
		if (type == 2)
			script_pushconststr(st,"null");
		else
			script_pushint(st,0);
		return 0;
	}

	switch(type){
		case 0: script_pushint(st,hd->homunculus.hom_id); break;
		case 1: script_pushint(st,hd->homunculus.class_); break;
		case 2: script_pushstrcopy(st,hd->homunculus.name); break;
		case 3: script_pushint(st,hd->homunculus.intimacy); break;
		case 4: script_pushint(st,hd->homunculus.hunger); break;
		case 5: script_pushint(st,hd->homunculus.rename_flag); break;
		case 6: script_pushint(st,hd->homunculus.level); break;
		default:
			script_pushint(st,0);
			break;
	}
	return 0;
}

/*==========================================
 * Shows wether your inventory(and equips) contain
   selected card or not.
	checkequipedcard(4001);
 *------------------------------------------*/
BUILDIN_FUNC(checkequipedcard)
{
	TBL_PC *sd=script_rid2sd(st);
	int n,i,c=0;
	c=script_getnum(st,2);

	if(sd){
		for(i=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].amount){
				for(n=0;n<MAX_SLOTS;n++){
					if(sd->status.inventory[i].card[n]==c){
						script_pushint(st,1);
						return 0;
					}
				}
			}
		}
	}
	script_pushint(st,0);
	return 0;
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
			return 1;
		}

		pos=script_getnum(st,3);
		st->pos=pos;
		st->state=GOTO;
		// printf("script: jump_zero: jumpto : %d\n",pos);
	} else {
		// printf("script: jump_zero: fail\n");
	}
	return 0;
}

/*==========================================
 * GetMapMobs
	returns mob counts on a set map:
	e.g. GetMapMobs("prontera")
	use "this" - for player's map
 *------------------------------------------*/
BUILDIN_FUNC(getmapmobs)
{
	const char *str=NULL;
	int m=-1,bx,by;
	int count=0;
	struct block_list *bl;

	str=script_getstr(st,2);

	if(strcmp(str,"this")==0){
		TBL_PC *sd=script_rid2sd(st);
		if(sd)
			m=sd->bl.m;
		else{
			script_pushint(st,-1);
			return 0;
		}
	}else
		m=map_mapname2mapid(str);

	if(m < 0){
		script_pushint(st,-1);
		return 0;
	}

	for(by=0;by<=(map[m].ys-1)/BLOCK_SIZE;by++)
		for(bx=0;bx<=(map[m].xs-1)/BLOCK_SIZE;bx++)
			for( bl = map[m].block_mob[bx+by*map[m].bxs] ; bl != NULL ; bl = bl->next )
				if(bl->x>=0 && bl->x<=map[m].xs-1 && bl->y>=0 && bl->y<=map[m].ys-1)
					count++;

	script_pushint(st,count);
	return 0;
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

	if ((nd = npc_name2id(npc)) == NULL)
		return -1;

	npc_movenpc(nd, x, y);
	return 0;
}

/*==========================================
 * message [MouseJstr]
 *------------------------------------------*/
BUILDIN_FUNC(message)
{
	TBL_PC *sd;
	const char *msg,*player;
	TBL_PC *pl_sd = NULL;

	sd = script_rid2sd(st);

	player = script_getstr(st,2);
	msg = script_getstr(st,3);

	if((pl_sd=map_nick2sd((char *) player)) == NULL)
		return 0;
	clif_displaymessage(pl_sd->fd, msg);

	return 0;
}

/*==========================================
 * npctalk (sends message to surrounding area)
 *------------------------------------------*/
BUILDIN_FUNC(npctalk)
{
	const char* str;
	char message[255];

	struct npc_data* nd = (struct npc_data *)map_id2bl(st->oid);
	str = script_getstr(st,2);

	if(nd) {
		memcpy(message, nd->name, NAME_LENGTH);
		strtok(message, "#"); // discard extra name identifier if present
		strcat(message, " : ");
		strncat(message, str, 254); //Prevent overflow possibility. [Skotlex]
		clif_message(&(nd->bl), message);
	}

	return 0;
}

// change npc walkspeed [Valaris]
BUILDIN_FUNC(npcspeed)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	int x=0;

	x=script_getnum(st,2);

	if(nd) {
		nd->speed=x;
	}

	return 0;
}
// make an npc walk to a position [Valaris]
BUILDIN_FUNC(npcwalkto)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);
	int x=0,y=0;

	x=script_getnum(st,2);
	y=script_getnum(st,3);

	if(nd) {
		unit_walktoxy(&nd->bl,x,y,0);
	}

	return 0;
}
// stop an npc's movement [Valaris]
BUILDIN_FUNC(npcstop)
{
	struct npc_data *nd=(struct npc_data *)map_id2bl(st->oid);

	if(nd) {
		unit_stop_walking(&nd->bl,1);
	}

	return 0;
}


/*==========================================
 * getlook char info. getlook(arg)
 *------------------------------------------*/
BUILDIN_FUNC(getlook)
{
        int type,val;
        TBL_PC *sd;
        sd=script_rid2sd(st);

        type=script_getnum(st,2);
        val=-1;
        switch(type) {
        case LOOK_HAIR: val=sd->status.hair; break; //1
        case LOOK_WEAPON: val=sd->status.weapon; break; //2
        case LOOK_HEAD_BOTTOM: val=sd->status.head_bottom; break; //3
        case LOOK_HEAD_TOP: val=sd->status.head_top; break; //4
        case LOOK_HEAD_MID: val=sd->status.head_mid; break; //5
        case LOOK_HAIR_COLOR: val=sd->status.hair_color; break; //6
        case LOOK_CLOTHES_COLOR: val=sd->status.clothes_color; break; //7
        case LOOK_SHIELD: val=sd->status.shield; break; //8
        case LOOK_SHOES: break; //9
        }

        script_pushint(st,val);
        return 0;
}

/*==========================================
 *     get char save point. argument: 0- map name, 1- x, 2- y
 *------------------------------------------*/
BUILDIN_FUNC(getsavepoint)
{
	TBL_PC* sd;
	int type;

	sd = script_rid2sd(st);
	if (sd == NULL) {
		script_pushint(st,0);
		return 0;
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
	return 0;
}

/*==========================================
  * Get position for  char/npc/pet/mob objects. Added by Lorky
  *
  *     int getMapXY(MapName$,MapX,MapY,type,[CharName$]);
  *             where type:
  *                     MapName$ - String variable for output map name
  *                     MapX     - Integer variable for output coord X
  *                     MapY     - Integer variable for output coord Y
  *                     type     - type of object
  *                                0 - Character coord
  *                                1 - NPC coord
  *                                2 - Pet coord
  *                                3 - Mob coord (not released)
  *                                4 - Homun coord
  *                     CharName$ - Name object. If miss or "this" the current object
  *
  *             Return:
  *                     0        - success
  *                     -1       - some error, MapName$,MapX,MapY contains unknown value.
  *------------------------------------------*/
BUILDIN_FUNC(getmapxy)
{
	struct block_list *bl = NULL;
	TBL_PC *sd=NULL;

	int num;
	const char *name;
	char prefix;

	int x,y,type;
	char mapname[MAP_NAME_LENGTH];

	if( !data_isreference(script_getdata(st,2)) ){
		ShowWarning("script: buildin_getmapxy: not mapname variable\n");
		script_pushint(st,-1);
		return 1;
	}
	if( !data_isreference(script_getdata(st,3)) ){
		ShowWarning("script: buildin_getmapxy: not mapx variable\n");
		script_pushint(st,-1);
		return 1;
	}
	if( !data_isreference(script_getdata(st,4)) ){
		ShowWarning("script: buildin_getmapxy: not mapy variable\n");
		script_pushint(st,-1);
		return 1;
	}

	// Possible needly check function parameters on C_STR,C_INT,C_INT
	type=script_getnum(st,5);

	switch (type){
		case 0:	//Get Character Position
			if( script_hasdata(st,6) )
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);

			if (sd)
				bl = &sd->bl;
			break;
		case 1:	//Get NPC Position
			if( script_hasdata(st,6) )
			{
				struct npc_data *nd;
				nd=npc_name2id(script_getstr(st,6));
				if (nd)
					bl = &nd->bl;
			} else //In case the origin is not an npc?
				bl=map_id2bl(st->oid);
			break;
		case 2:	//Get Pet Position
			if(script_hasdata(st,6))
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);

			if (sd && sd->pd)
				bl = &sd->pd->bl;
			break;
		case 3:	//Get Mob Position
			break; //Not supported?
		case 4:	//Get Homun Position
			if(script_hasdata(st,6))
				sd=map_nick2sd(script_getstr(st,6));
			else
				sd=script_rid2sd(st);

			if (sd && sd->hd)
				bl = &sd->hd->bl;
			break;
	}
	if (!bl) { //No object found.
		script_pushint(st,-1);
		return 0;
	}

	x= bl->x;
	y= bl->y;
	safestrncpy(mapname, map[bl->m].name, MAP_NAME_LENGTH);
	
	//Set MapName$
	num=st->stack->stack_data[st->start+2].u.num;
	name=get_str(num&0x00ffffff);
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script_rid2sd(st);
	else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)mapname,script_getref(st,2));

	//Set MapX
	num=st->stack->stack_data[st->start+3].u.num;
	name=get_str(num&0x00ffffff);
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script_rid2sd(st);
	else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)x,script_getref(st,3));

	//Set MapY
	num=st->stack->stack_data[st->start+4].u.num;
	name=get_str(num&0x00ffffff);
	prefix=*name;

	if(not_server_variable(prefix))
		sd=script_rid2sd(st);
	else
		sd=NULL;
	set_reg(st,sd,num,name,(void*)y,script_getref(st,4));

	//Return Success value
	script_pushint(st,0);
	return 0;
}

/*==========================================
 * Allows player to write NPC logs (i.e. Bank NPC, etc) [Lupus]
 *------------------------------------------*/
BUILDIN_FUNC(logmes)
{
	const char *str;
	TBL_PC* sd;

	if( log_config.npc <= 0 )
		return 0;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 1;

	str = script_getstr(st,2);
	log_npc(sd,str);
	return 0;
}

BUILDIN_FUNC(summon)
{
	int _class, timeout=0;
	const char *str,*event="";
	TBL_PC *sd;
	struct mob_data *md;
	int tick = gettick();

	sd=script_rid2sd(st);
	if (!sd) return 0;
	
	str	=script_getstr(st,2);
	_class=script_getnum(st,3);
	if( script_hasdata(st,4) )
		timeout=script_getnum(st,4);
	if( script_hasdata(st,5) ){
		event=script_getstr(st,5);
		check_event(st, event);
	}

	clif_skill_poseffect(&sd->bl,AM_CALLHOMUN,1,sd->bl.x,sd->bl.y,tick);

	md = mob_once_spawn_sub(&sd->bl, sd->bl.m, sd->bl.x, sd->bl.y, str, _class, event);
	if (md) {
		md->master_id=sd->bl.id;
		md->special_state.ai=1;
		md->deletetimer = add_timer(tick+(timeout>0?timeout*1000:60000),mob_timer_delete,md->bl.id,0);
		mob_spawn (md); //Now it is ready for spawning.
		clif_misceffect2(&md->bl,344);
		sc_start4(&md->bl, SC_MODECHANGE, 100, 1, 0, MD_AGGRESSIVE, 0, 60000);
	}
	return 0;
}

/*==========================================
 * Checks whether it is daytime/nighttime
 *------------------------------------------*/
BUILDIN_FUNC(isnight)
{
	script_pushint(st,(night_flag == 1));
	return 0;
}

BUILDIN_FUNC(isday)
{
	script_pushint(st,(night_flag == 0));
	return 0;
}

/*================================================
 * Check whether another item/card has been
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------*/
// leave this here, just in case
#if 0
BUILDIN_FUNC(isequipped)
{
	TBL_PC *sd;
	int i, j, k, id = 1;
	int ret = -1;

	sd = script_rid2sd(st);
	
	for (i=0; id!=0; i++) {
		int flag = 0;
	
		FETCH (i+2, id) else id = 0;
		if (id <= 0)
			continue;
		
		for (j=0; j<EQI_MAX; j++) {
			int index;
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index) continue;
			if(j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index) continue;
			if(j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index)) continue;
			
			if(!sd->inventory_data[index])
				continue;

			if(itemdb_type(id) != IT_CARD) { //Non card
				if (sd->inventory_data[index]->nameid != id)
					continue;
				flag = 1;
				break;
			} else { //Card
				if (itemdb_isspecial(sd->status.inventory[index].card[0]))
					continue;
				for(k=0; k<sd->inventory_data[index]->slot; k++) {
					if (sd->status.inventory[index].card[k] == id)
					{
						flag = 1;
						break;
					}
				}
			}
			if (flag) break;
		}
		if (ret == -1)
			ret = flag;
		else
			ret &= flag;
		if (!ret) break;
	}
	
	script_pushint(st,ret);
	return 0;
}
#endif

/*================================================
 * Check how many items/cards in the list are
 * equipped - used for 2/15's cards patch [celest]
 *------------------------------------------------*/
BUILDIN_FUNC(isequippedcnt)
{
	TBL_PC *sd;
	int i, j, k, id = 1;
	int ret = 0;

	sd = script_rid2sd(st);
	if (!sd) { //If the player is not attached it is a script error anyway... but better prevent the map server from crashing...
		script_pushint(st,0);
		return 0;
	}
	
	for (i=0; id!=0; i++) {
		FETCH (i+2, id) else id = 0;
		if (id <= 0)
			continue;
		
		for (j=0; j<EQI_MAX; j++) {
			int index;
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index) continue;
			if(j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index) continue;
			if(j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index)) continue;
			
			if(!sd->inventory_data[index])
				continue;

			if (itemdb_type(id) != IT_CARD) { //No card. Count amount in inventory.
				if (sd->inventory_data[index]->nameid == id)
					ret+= sd->status.inventory[index].amount;
			} else { //Count cards.
				if (itemdb_isspecial(sd->status.inventory[index].card[0]))
					continue; //No cards
				for(k=0; k<sd->inventory_data[index]->slot; k++) {
					if (sd->status.inventory[index].card[k] == id) 
						ret++; //[Lupus]
				}				
			}
		}
	}
	
	script_pushint(st,ret);
	return 0;
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
	int i, j, k, id = 1;
	int index, flag;
	int ret = -1;
	//Original hash to reverse it when full check fails.
	unsigned int setitem_hash = 0, setitem_hash2 = 0;

	sd = script_rid2sd(st);
	
	if (!sd) { //If the player is not attached it is a script error anyway... but better prevent the map server from crashing...
		script_pushint(st,0);
		return 0;
	}

	setitem_hash = sd->setitem_hash;
	setitem_hash2 = sd->setitem_hash2;
	for (i=0; id!=0; i++)
	{
		FETCH (i+2, id) else id = 0;
		if (id <= 0)
			continue;
		flag = 0;
		for (j=0; j<EQI_MAX; j++)
		{
			index = sd->equip_index[j];
			if(index < 0) continue;
			if(j == EQI_HAND_R && sd->equip_index[EQI_HAND_L] == index) continue;
			if(j == EQI_HEAD_MID && sd->equip_index[EQI_HEAD_LOW] == index) continue;
			if(j == EQI_HEAD_TOP && (sd->equip_index[EQI_HEAD_MID] == index || sd->equip_index[EQI_HEAD_LOW] == index)) continue;
	
			if(!sd->inventory_data[index])
				continue;
			
			if (itemdb_type(id) != IT_CARD) {
				if (sd->inventory_data[index]->nameid != id)
					continue;
				flag = 1;
				break;
			} else { //Cards
				if (sd->inventory_data[index]->slot == 0 ||
					itemdb_isspecial(sd->status.inventory[index].card[0]))
					continue;

				for (k = 0; k < sd->inventory_data[index]->slot; k++)
				{	//New hash system which should support up to 4 slots on any equipment. [Skotlex]
					unsigned int hash = 0;
					if (sd->status.inventory[index].card[k] != id)
						continue;

					hash = 1<<((j<5?j:j-5)*4 + k);
					// check if card is already used by another set
					if ((j<5?sd->setitem_hash:sd->setitem_hash2) & hash)	
						continue;

					// We have found a match
					flag = 1;
					// Set hash so this card cannot be used by another
					if (j<5)
						sd->setitem_hash |= hash;
					else
						sd->setitem_hash2 |= hash;
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
	if (!ret)
  	{	//When check fails, restore original hash values. [Skotlex]
		sd->setitem_hash = setitem_hash;
		sd->setitem_hash2 = setitem_hash2;
	}
	script_pushint(st,ret);
	return 0;
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

	sd = script_rid2sd(st);
	
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
				ret+= sd->status.inventory[index].amount;
		} else {
			if (itemdb_isspecial(sd->status.inventory[index].card[0]))
				continue;
			for(k=0; k<sd->inventory_data[index]->slot; k++) {
				if (sd->status.inventory[index].card[k] == id)
					ret++;
			}				
		}
	}
	script_pushint(st,ret);
//	script_pushint(st,current_equip_item_index);
	return 0;
}

/*=======================================================
 * Returns the refined number of the current item, or an
 * item with inventory index specified
 *-------------------------------------------------------*/
BUILDIN_FUNC(getrefine)
{
	TBL_PC *sd;
	if ((sd = script_rid2sd(st))!= NULL)
		script_pushint(st,sd->status.inventory[current_equip_item_index].refine);
	else
		script_pushint(st,0);
	return 0;
}

/*=======================================================
 * Day/Night controls
 *-------------------------------------------------------*/
BUILDIN_FUNC(night)
{
	if (night_flag != 1) map_night_timer(night_timer_tid, 0, 0, 1);
	return 0;
}
BUILDIN_FUNC(day)
{
	if (night_flag != 0) map_day_timer(day_timer_tid, 0, 0, 1);
	return 0;
}

//=======================================================
// Unequip [Spectre]
//-------------------------------------------------------
BUILDIN_FUNC(unequip)
{
	int i;
	size_t num;
	TBL_PC *sd;

	num = script_getnum(st,2);
	sd = script_rid2sd(st);
	if( sd != NULL && num >= 1 && num <= ARRAYLENGTH(equip) )
	{
		i = pc_checkequip(sd,equip[num-1]);
		if (i >= 0)
			pc_unequipitem(sd,i,2);
		return 0;
	}
	return 0;
}

BUILDIN_FUNC(equip)
{
	int nameid=0,i;
	TBL_PC *sd;
	struct item_data *item_data;

	sd = script_rid2sd(st);

	nameid=script_getnum(st,2);
	if((item_data = itemdb_exists(nameid)) == NULL)
	{
		ShowError("wrong item ID : equipitem(%i)\n",nameid);
		return 1;
	}
	ARR_FIND( 0, MAX_INVENTORY, i, sd->status.inventory[i].nameid == nameid );
	if( i < MAX_INVENTORY )
		pc_equipitem(sd,i,item_data->equip);

	return 0;
}

BUILDIN_FUNC(autoequip)
{
	int nameid, flag;
	struct item_data *item_data;
	nameid=script_getnum(st,2);
	flag=script_getnum(st,3);
	if(nameid>=500 && (item_data = itemdb_search(nameid)) != NULL){
		item_data->flag.autoequip = flag>0?1:0;
	}
	return 0;
}

BUILDIN_FUNC(setbattleflag)
{
	const char *flag, *value;

	flag = script_getstr(st,2);
	value = script_getstr(st,3);
	
	if (battle_set_value(flag, value) == 0)
		ShowWarning("buildin_setbattleflag: unknown battle_config flag '%s'\n",flag);
	else
		ShowInfo("buildin_setbattleflag: battle_config flag '%s' is now set to '%s'.\n",flag,value);

	return 0;
}

BUILDIN_FUNC(getbattleflag)
{
	const char *flag;
	flag = script_getstr(st,2);
	script_pushint(st,battle_get_value(flag));
	return 0;
}

//=======================================================
// strlen [Valaris]
//-------------------------------------------------------
BUILDIN_FUNC(getstrlen)
{

	const char *str = script_getstr(st,2);
	int len = (str) ? (int)strlen(str) : 0;

	script_pushint(st,len);
	return 0;
}

//=======================================================
// isalpha [Valaris]
//-------------------------------------------------------
BUILDIN_FUNC(charisalpha)
{
	const char *str=script_getstr(st,2);
	int pos=script_getnum(st,3);

	int val = ( str && pos >= 0 && (unsigned int)pos < strlen(str) ) ? ISALPHA( str[pos] ) : 0;

	script_pushint(st,val);
	return 0;
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
	int class_ = -1, size = -1;
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
		ShowError("script:setnpcdisplay: expected a string or number\n");
		script_reportdata(data);
		return 1;
	}

	nd = npc_name2id(name);
	if( nd == NULL )
	{// not found
		script_pushint(st,1);
		return 0;
	}

	// update npc
	if( newname )
		npc_setdisplayname(nd, newname);

	if( size != -1 && size != nd->size )
		nd->size = size;
	else
		size = -1;

	if( class_ != -1 && nd->class_ != class_ )
		npc_setclass(nd, class_);
	else if( size != -1 )
	{ // Required to update the visual size
		clif_clearunit_area(&nd->bl, 0);
		clif_spawn(&nd->bl);
	}

	script_pushint(st,0);
	return 0;
}

BUILDIN_FUNC(atoi)
{
	const char *value;
	value = script_getstr(st,2);
	script_pushint(st,atoi(value));
	return 0;
}

// case-insensitive substring search [lordalfa]
BUILDIN_FUNC(compare)
{
	const char *message;
	const char *cmpstring;
	message = script_getstr(st,2);
	cmpstring = script_getstr(st,3);
	script_pushint(st,(stristr(message,cmpstring) != NULL));
	return 0;
}

// [zBuffer] List of mathematics commands --->
BUILDIN_FUNC(sqrt)
{
	double i, a;
	i = script_getnum(st,2);
	a = sqrt(i);
	script_pushint(st,(int)a);
	return 0;
}

BUILDIN_FUNC(pow)
{
	double i, a, b;
	a = script_getnum(st,2);
	b = script_getnum(st,3);
	i = pow(a,b);
	script_pushint(st,(int)i);
	return 0;
}
BUILDIN_FUNC(distance)
{
	int x0, y0, x1, y1;

	x0 = script_getnum(st,2);
	y0 = script_getnum(st,3);
	x1 = script_getnum(st,4);
	y1 = script_getnum(st,5);

	script_pushint(st,distance_xy(x0,y0,x1,y1));
	return 0;
}

// <--- [zBuffer] List of mathematics commands
// [zBuffer] List of dynamic var commands --->

BUILDIN_FUNC(setd)
{
	TBL_PC *sd=NULL;
	char varname[100];
	const char *value, *buffer;
	int elem;
	buffer = script_getstr(st, 2);
	value = script_getstr(st, 3);

	if(sscanf(buffer, "%99[^[][%d]", varname, &elem) < 2)
		elem = 0;

	if( not_server_variable(*varname) )
	{
		sd = script_rid2sd(st);
		if( sd == NULL )
		{
			ShowError("script:setd: no player attached for player variable '%s'\n", buffer);
			return 0;
		}
	}

	if(varname[strlen(varname)-1] != '$') {
		setd_sub(st,sd, varname, elem, (void *)atoi(value),NULL);
	} else {
		setd_sub(st,sd, varname, elem, (void *)value,NULL);
	}
	
	return 0;
}

#ifndef TXT_ONLY
int buildin_query_sql_sub(struct script_state* st, Sql* handle)
{
	int i, j;
	TBL_PC* sd = NULL;
	const char* query;
	struct script_data* data;
	const char* name;
	int max_rows = 128;// maximum number of rows
	int num_vars;
	int num_cols;

	// check target variables
	for( i = 3; script_hasdata(st,i); ++i )
	{
		data = script_getdata(st, i);
		if( data_isreference(data) && reference_tovariable(data) )
		{// it's a variable
			name = reference_getname(data);
			if( not_server_variable(*name) && sd == NULL )
			{// requires a player
				sd = script_rid2sd(st);
				if( sd == NULL )
				{// no player attached
					script_reportdata(data);
					st->state = END;
					return 1;
				}
			}
			if( not_array_variable(*name) )
				max_rows = 1;// not an array, limit to one row
		}
		else
		{
			ShowError("script:query_sql: not a variable\n");
			script_reportdata(data);
			st->state = END;
			return 1;
		}
	}
	num_vars = i - 3;

	// Execute the query
	query = script_getstr(st,2);
	if( SQL_ERROR == Sql_QueryStr(handle, query) )
	{
		Sql_ShowDebug(handle);
		script_pushint(st, 0);
		return 1;
	}

	if( Sql_NumRows(handle) == 0 )
	{// No data received
		Sql_FreeResult(handle);
		script_pushint(st, 0);
		return 0;
	}

	// Count the number of columns to store
	num_cols = Sql_NumColumns(handle);
	if( num_vars < num_cols )
	{
		ShowWarning("script:query_sql: Too many columns, discarding last %u columns.\n", (unsigned int)(num_cols-num_vars));
		script_reportsrc(st);
	}
	else if( num_vars > num_cols )
	{
		ShowWarning("script:query_sql: Too many variables (%u extra).\n", (unsigned int)(num_vars-num_cols));
		script_reportsrc(st);
	}

	// Store data
	for( i = 0; i < max_rows && SQL_SUCCESS == Sql_NextRow(handle); ++i )
	{
		for( j = 0; j < num_vars; ++j )
		{
			char* str = NULL;

			if( j < num_cols )
				Sql_GetData(handle, j, &str, NULL);

			data = script_getdata(st, j+3);
			name = reference_getname(data);
			if( is_string_variable(name) )
				setd_sub(st, sd, name, i, (void *)(str?str:""), reference_getref(data));
			else
				setd_sub(st, sd, name, i, (void *)(str?atoi(str):0), reference_getref(data));
		}
	}
	if( i == max_rows && max_rows < Sql_NumRows(handle) )
	{
		ShowWarning("script:query_sql: Only %d/%u rows have been stored.\n", max_rows, (unsigned int)Sql_NumRows(handle));
		script_reportsrc(st);
	}

	// Free data
	Sql_FreeResult(handle);
	script_pushint(st, i);
	return 0;
}
#endif

BUILDIN_FUNC(query_sql)
{
#ifndef TXT_ONLY
	return buildin_query_sql_sub(st, mmysql_handle);
#else
	//for TXT version, we always return -1
	script_pushint(st,-1);
	return 0;
#endif
}

BUILDIN_FUNC(query_logsql)
{
#ifndef TXT_ONLY
	return buildin_query_sql_sub(st, logmysql_handle);
#else
	//for TXT version, we always return -1
	script_pushint(st,-1);
	return 0;
#endif
}

//Allows escaping of a given string.
BUILDIN_FUNC(escape_sql)
{
	const char *str;
	char *esc_str;
	size_t len;

	str = script_getstr(st,2);
	len = strlen(str);
	esc_str = (char*)aMallocA(len*2+1);
#if defined(TXT_ONLY)
	jstrescapecpy(esc_str, str);
#else
	Sql_EscapeStringLen(mmysql_handle, esc_str, str, len);
#endif
	script_pushstr(st, esc_str);
	return 0;
}

BUILDIN_FUNC(getd)
{
	char varname[100];
	const char *buffer;
	int elem;

	buffer = script_getstr(st, 2);

	if(sscanf(buffer, "%[^[][%d]", varname, &elem) < 2)
		elem = 0;

	// Push the 'pointer' so it's more flexible [Lance]
	push_val(st->stack, C_NAME, (elem<<24) | add_str(varname));

	return 0;
}

// <--- [zBuffer] List of dynamic var commands
// Pet stat [Lance]
BUILDIN_FUNC(petstat)
{
	TBL_PC *sd = NULL;
	struct pet_data *pd;
	int flag = script_getnum(st,2);
	sd = script_rid2sd(st);
	if(!sd || !sd->status.pet_id || !sd->pd){
		if(flag == 2)
			script_pushconststr(st, "");
		else
			script_pushint(st,0);
		return 0;
	}
	pd = sd->pd;
	switch(flag){
		case 1: script_pushint(st,(int)pd->pet.class_); break;
		case 2: script_pushstrcopy(st, pd->pet.name); break;
		case 3: script_pushint(st,(int)pd->pet.level); break;
		case 4: script_pushint(st,(int)pd->pet.hungry); break;
		case 5: script_pushint(st,(int)pd->pet.intimate); break;
		default:
			script_pushint(st,0);
			break;
	}
	return 0;
}

BUILDIN_FUNC(callshop)
{
	TBL_PC *sd = NULL;
	struct npc_data *nd;
	const char *shopname;
	int flag = 0;
	sd = script_rid2sd(st);
	if (!sd) {
		script_pushint(st,0);
		return 0;
	}
	shopname = script_getstr(st, 2);
	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);
	nd = npc_name2id(shopname);
	if( !nd || nd->bl.type != BL_NPC || (nd->subtype != SHOP && nd->subtype != CASHSHOP) )
	{
		ShowError("buildin_callshop: Shop [%s] not found (or NPC is not shop type)\n", shopname);
		script_pushint(st,0);
		return 1;
	}
	
	if( nd->subtype == SHOP )
	{
		switch( flag )
		{
			case 1: npc_buysellsel(sd,nd->bl.id,0); break; //Buy window
			case 2: npc_buysellsel(sd,nd->bl.id,1); break; //Sell window
			default: clif_npcbuysell(sd,nd->bl.id); break; //Show menu
		}
	}
	else
		clif_cashshop_show(sd, nd);

	sd->npc_shopid = nd->bl.id;
	script_pushint(st,1);
	return 0;
}

BUILDIN_FUNC(npcshopitem)
{
	const char* npcname = script_getstr(st, 2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	int amount;

	if( !nd || nd->subtype != SHOP )
	{	//Not found.
		script_pushint(st,0);
		return 0;
	}

	// get the count of new entries
	amount = (script_lastdata(st)-2)/2;

	// generate new shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, amount);
	for( n = 0, i = 3; n < amount; n++, i+=2 )
	{
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return 0;
}

BUILDIN_FUNC(npcshopadditem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	int amount;

	if( !nd || nd->subtype != SHOP )
	{	//Not found.
		script_pushint(st,0);
		return 0;
	}

	// get the count of new entries
	amount = (script_lastdata(st)-2)/2;

	// append new items to existing shop item list
	RECREATE(nd->u.shop.shop_item, struct npc_item_list, nd->u.shop.count+amount);
	for( n = nd->u.shop.count, i = 3; n < nd->u.shop.count+amount; n++, i+=2 )
	{
		nd->u.shop.shop_item[n].nameid = script_getnum(st,i);
		nd->u.shop.shop_item[n].value = script_getnum(st,i+1);
	}
	nd->u.shop.count = n;

	script_pushint(st,1);
	return 0;
}

BUILDIN_FUNC(npcshopdelitem)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int n, i;
	int amount;
	int size;

	if( !nd || nd->subtype != SHOP )
	{	//Not found.
		script_pushint(st,0);
		return 0;
	}

	amount = script_lastdata(st)-2;
	size = nd->u.shop.count;

	// remove specified items from the shop item list
	for( i = 3; i < 3 + amount; i++ )
	{
		ARR_FIND( 0, size, n, nd->u.shop.shop_item[n].nameid == script_getnum(st,i) );
		if( n < size )
		{
			memmove(&nd->u.shop.shop_item[n], &nd->u.shop.shop_item[n+1], sizeof(nd->u.shop.shop_item[0])*(size-n));
			size--;
		}
	}

	RECREATE(nd->u.shop.shop_item, struct npc_item_list, size);
	nd->u.shop.count = size;

	script_pushint(st,1);
	return 0;
}

//Sets a script to attach to a shop npc.
BUILDIN_FUNC(npcshopattach)
{
	const char* npcname = script_getstr(st,2);
	struct npc_data* nd = npc_name2id(npcname);
	int flag = 1;

	if( script_hasdata(st,3) )
		flag = script_getnum(st,3);

	if( !nd || nd->subtype != SHOP )
	{	//Not found.
		script_pushint(st,0);
		return 0;
	}

	if (flag)
		nd->master_nd = ((struct npc_data *)map_id2bl(st->oid));
	else
		nd->master_nd = NULL;

	script_pushint(st,1);
	return 0;
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
	int item_id,n=0;
	const char *script;
	struct item_data *i_data;
	struct script_code **dstscript;

	item_id	= script_getnum(st,2);
	script = script_getstr(st,3);
	if( script_hasdata(st,4) )
		n=script_getnum(st,4);
	i_data = itemdb_exists(item_id);

	if (!i_data || script==NULL || script[0]!='{') {
		script_pushint(st,0);
		return 0;
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

	*dstscript = parse_script(script, "script_setitemscript", 0, 0);
	script_pushint(st,1);
	return 0;
}

/* Work In Progress [Lupus]
BUILDIN_FUNC(addmonsterdrop)
{
	int class_,item_id,chance;
	class_=script_getnum(st,2);
	item_id=script_getnum(st,3);
	chance=script_getnum(st,4);
	if(class_>1000 && item_id>500 && chance>0) {
		script_pushint(st,1);
	} else {
		script_pushint(st,0);
	}
}

BUILDIN_FUNC(delmonsterdrop)
{
	int class_,item_id;
	class_=script_getnum(st,2);
	item_id=script_getnum(st,3);
	if(class_>1000 && item_id>500) {
		script_pushint(st,1);
	} else {
		script_pushint(st,0);
	}
}
*/

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
		ShowError("buildin_getmonsterinfo: Wrong Monster ID: %i\n", mob_id);
		if ( !script_getnum(st,3) ) //requested a string
			script_pushconststr(st,"null");
		else
			script_pushint(st,-1);
		return -1;
	}
	mob = mob_db(mob_id);
	switch ( script_getnum(st,3) ) {
		case 0:  script_pushstrcopy(st,mob->jname); break;
		case 1:  script_pushint(st,mob->lv); break;
		case 2:  script_pushint(st,mob->status.max_hp); break;
		case 3:  script_pushint(st,mob->base_exp); break;
		case 4:  script_pushint(st,mob->job_exp); break;
		case 5:  script_pushint(st,mob->status.rhw.atk); break;
		case 6:  script_pushint(st,mob->status.rhw.atk2); break;
		case 7:  script_pushint(st,mob->status.def); break;
		case 8:  script_pushint(st,mob->status.mdef); break;
		case 9:  script_pushint(st,mob->status.str); break;
		case 10: script_pushint(st,mob->status.agi); break;
		case 11: script_pushint(st,mob->status.vit); break;
		case 12: script_pushint(st,mob->status.int_); break;
		case 13: script_pushint(st,mob->status.dex); break;
		case 14: script_pushint(st,mob->status.luk); break;
		case 15: script_pushint(st,mob->status.rhw.range); break;
		case 16: script_pushint(st,mob->range2); break;
		case 17: script_pushint(st,mob->range3); break;
		case 18: script_pushint(st,mob->status.size); break;
		case 19: script_pushint(st,mob->status.race); break;
		case 20: script_pushint(st,mob->status.def_ele); break;
		case 21: script_pushint(st,mob->status.mode); break;
		default: script_pushint(st,-1); //wrong Index
	}
	return 0;
}

BUILDIN_FUNC(checkvending) // check vending [Nab4]
{
	TBL_PC *sd = NULL;

	if(script_hasdata(st,2))
		sd = map_nick2sd(script_getstr(st,2));
	else
		sd = script_rid2sd(st);

	if(sd)
		script_pushint(st,(sd->vender_id != 0));
	else
		script_pushint(st,0);

	return 0;
}


BUILDIN_FUNC(checkchatting) // check chatting [Marka]
{
	TBL_PC *sd = NULL;

	if(script_hasdata(st,2))
		sd = map_nick2sd(script_getstr(st,2));
	else
		sd = script_rid2sd(st);

	if(sd)
		script_pushint(st,(sd->chatID != 0));
	else
		script_pushint(st,0);

	return 0;
}

int axtoi(const char *hexStg)
{
	int n = 0;         // position in string
	int m = 0;         // position in digit[] to shift
	int count;         // loop index
	int intValue = 0;  // integer value of hex string
	int digit[11];      // hold values to convert
	while (n < 10) {
		if (hexStg[n]=='\0')
			break;
		if (hexStg[n] > 0x29 && hexStg[n] < 0x40 ) //if 0 to 9
			digit[n] = hexStg[n] & 0x0f;            //convert to int
		else if (hexStg[n] >='a' && hexStg[n] <= 'f') //if a to f
			digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
		else if (hexStg[n] >='A' && hexStg[n] <= 'F') //if A to F
			digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
		else break;
		n++;
	}
	count = n;
	m = n - 1;
	n = 0;
	while(n < count) {
		// digit[n] is value of hex digit at position n
		// (m << 2) is the number of positions to shift
		// OR the bits into return value
		intValue = intValue | (digit[n] << (m << 2));
		m--;   // adjust the position to set
		n++;   // next digit to process
	}
	return (intValue);
}

// [Lance] Hex string to integer converter
BUILDIN_FUNC(axtoi)
{
	const char *hex = script_getstr(st,2);
	script_pushint(st,axtoi(hex));	
	return 0;
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
			default:
				ShowError("buildin_rid2name: BL type unknown.\n");
				script_pushconststr(st,"");
				break;
		}
	} else {
		ShowError("buildin_rid2name: invalid RID\n");
		script_pushconststr(st,"(null)");
	}
	return 0;
}

BUILDIN_FUNC(pcblockmove)
{
	int id, flag;
	TBL_PC *sd = NULL;

	id = script_getnum(st,2);
	flag = script_getnum(st,3);

	if(id)
		sd = map_id2sd(id);
	else
		sd = script_rid2sd(st);

	if(sd)
		sd->state.blockedmove = flag > 0;

	return 0;
}

BUILDIN_FUNC(pcfollow)
{
	int id, targetid;
	TBL_PC *sd = NULL;


	id = script_getnum(st,2);
	targetid = script_getnum(st,3);

	if(id)
		sd = map_id2sd(id);
	else
		sd = script_rid2sd(st);

	if(sd)
		pc_follow(sd, targetid);

    return 0;
}

BUILDIN_FUNC(pcstopfollow)
{
	int id;
	TBL_PC *sd = NULL;


	id = script_getnum(st,2);

	if(id)
		sd = map_id2sd(id);
	else
		sd = script_rid2sd(st);

	if(sd)
		pc_stop_following(sd);

	return 0;
}
// <--- [zBuffer] List of player cont commands
// [zBuffer] List of mob control commands --->
//## TODO always return if the request/whatever was successfull [FlavioJS]

/// Makes the unit walk to target position or map
/// Returns if it was successfull
///
/// unitwalk(<unit_id>,<x>,<y>) -> <bool>
/// unitwalk(<unit_id>,<map_id>) -> <bool>
BUILDIN_FUNC(unitwalk)
{
	struct block_list* bl;

	bl = map_id2bl(script_getnum(st,2));
	if( bl == NULL )
	{
		script_pushint(st, 0);
	}
	else if( script_hasdata(st,4) )
	{
		int x = script_getnum(st,3);
		int y = script_getnum(st,4);
		script_pushint(st, unit_walktoxy(bl,x,y,0));// We'll use harder calculations.
	}
	else
	{
		int map_id = script_getnum(st,3);
		script_pushint(st, unit_walktobl(bl,map_id2bl(map_id),65025,1));
	}

	return 0;
}

/// Kills the unit
///
/// unitkill <unit_id>;
BUILDIN_FUNC(unitkill)
{
	struct block_list* bl = map_id2bl(script_getnum(st,2));
	if( bl != NULL )
		status_kill(bl);

	return 0;
}

/// Warps the unit to the target position in the target map
/// Returns if it was successfull
///
/// unitwarp(<unit_id>,"<map name>",<x>,<y>) -> <bool>
BUILDIN_FUNC(unitwarp)
{
	int unit_id;
	int map;
	short x;
	short y;
	struct block_list* bl;

	unit_id = script_getnum(st,2);
	map = map_mapname2mapid(script_getstr(st, 3));
	x = (short)script_getnum(st,4);
	y = (short)script_getnum(st,5);

	bl = map_id2bl(unit_id);
	if( map >= 0 && bl != NULL )
		script_pushint(st, unit_warp(bl,map,x,y,0));
	else
		script_pushint(st, 0);

	return 0;
}

/// Makes the unit attack the target.
/// If the unit is a player and <action type> is not 0, it does a continuous 
/// attack instead of a single attack.
/// Returns if the request was successfull.
///
/// unitattack(<unit_id>,"<target name>"{,<action type>}) -> <bool>
/// unitattack(<unit_id>,<target_id>{,<action type>}) -> <bool>
BUILDIN_FUNC(unitattack)
{
	struct block_list* unit_bl;
	struct block_list* target_bl = NULL;
	struct script_data* data;
	int actiontype = 0;

	// get unit
	unit_bl = map_id2bl(script_getnum(st,2));
	if( unit_bl == NULL ) {
		script_pushint(st, 0);
		return 0;
	}
	
	data = script_getdata(st, 3);
	get_val(st, data);
	if( data_isstring(data) )
	{
		TBL_PC* sd = map_nick2sd(conv_str(st, data));
		if( sd != NULL )
			target_bl = &sd->bl;
	} else
		target_bl = map_id2bl(conv_num(st, data));
	// request the attack
	if( target_bl == NULL )
	{
		script_pushint(st, 0);
		return 0;
	}
	
	// get actiontype
	if( script_hasdata(st,4) )
		actiontype = script_getnum(st,4);

	switch( unit_bl->type )
	{
	case BL_PC:
		clif_parse_ActionRequest_sub(((TBL_PC *)unit_bl), actiontype > 0 ? 0x07 : 0x00, target_bl->id, gettick());
		script_pushint(st, 1);
		return 0;
	case BL_MOB:
		((TBL_MOB *)unit_bl)->target_id = target_bl->id;
		break;
	case BL_PET:
		((TBL_PET *)unit_bl)->target_id = target_bl->id;
		break;
	default:
		ShowError("script:unitattack: unsupported source unit type %d\n", unit_bl->type);
		script_pushint(st, 0);
		return 1;
	}
	script_pushint(st, unit_walktobl(unit_bl, target_bl, 65025, 2));
	return 0;
}

/// Makes the unit stop attacking and moving
///
/// unitstop <unit_id>;
BUILDIN_FUNC(unitstop)
{
	int unit_id;
	struct block_list* bl;

	unit_id = script_getnum(st,2);

	bl = map_id2bl(unit_id);
	if( bl != NULL )
	{
		unit_stop_attack(bl);
		unit_stop_walking(bl,0);
		if( bl->type == BL_MOB )
			((TBL_MOB*)bl)->target_id = 0;
	}

	return 0;
}

/// Makes the unit say the message
///
/// unittalk <unit_id>,"<message>";
BUILDIN_FUNC(unittalk)
{
	int unit_id;
	const char* message;
	struct block_list* bl;

	unit_id = script_getnum(st,2);
	message = script_getstr(st, 3);

	bl = map_id2bl(unit_id);
	if( bl != NULL )
	{
		struct StringBuf sbuf;
		StringBuf_Init(&sbuf);
		StringBuf_Printf(&sbuf, "%s : %s", status_get_name(bl), message);
		clif_message(bl, StringBuf_Value(&sbuf));
		if( bl->type == BL_PC )
			clif_displaymessage(((TBL_PC*)bl)->fd, StringBuf_Value(&sbuf));
		StringBuf_Destroy(&sbuf);
	}

	return 0;
}

/// Makes the unit do an emotion
///
/// unitemote <unit_id>,<emotion>;
///
/// @see e_* in const.txt
BUILDIN_FUNC(unitemote)
{
	int unit_id;
	int emotion;
	struct block_list* bl;

	unit_id = script_getnum(st,2);
	emotion = script_getnum(st,3);
	bl = map_id2bl(unit_id);
	if( bl != NULL )
		clif_emotion(bl, emotion);

	return 0;
}

/// Makes the unit cast the skill on the target or self if no target is specified
///
/// unitskilluseid <unit_id>,<skill_id>,<skill_lv>{,<target_id>};
/// unitskilluseid <unit_id>,"<skill name>",<skill_lv>{,<target_id>};
BUILDIN_FUNC(unitskilluseid)
{
	int unit_id;
	int skill_id;
	int skill_lv;
	int target_id;
	struct block_list* bl;

	unit_id  = script_getnum(st,2);
	skill_id = ( script_isstring(st,3) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	skill_lv = script_getnum(st,4);
	target_id = ( script_hasdata(st,5) ? script_getnum(st,5) : unit_id );

	bl = map_id2bl(unit_id);
	if( bl != NULL )
		unit_skilluse_id(bl, target_id, skill_id, skill_lv);

	return 0;
}

/// Makes the unit cast the skill on the target position.
///
/// unitskillusepos <unit_id>,<skill_id>,<skill_lv>,<target_x>,<target_y>;
/// unitskillusepos <unit_id>,"<skill name>",<skill_lv>,<target_x>,<target_y>;
BUILDIN_FUNC(unitskillusepos)
{
	int unit_id;
	int skill_id;
	int skill_lv;
	int skill_x;
	int skill_y;
	struct block_list* bl;

	unit_id  = script_getnum(st,2);
	skill_id = ( script_isstring(st,3) ? skill_name2id(script_getstr(st,3)) : script_getnum(st,3) );
	skill_lv = script_getnum(st,4);
	skill_x  = script_getnum(st,5);
	skill_y  = script_getnum(st,6);

	bl = map_id2bl(unit_id);
	if( bl != NULL )
		unit_skilluse_pos(bl, skill_x, skill_y, skill_id, skill_lv);

	return 0;
}

// <--- [zBuffer] List of mob control commands

/// Pauses the execution of the script, detaching the player
///
/// sleep <mili seconds>;
BUILDIN_FUNC(sleep)
{
	int ticks;
	TBL_PC* sd;
	
	ticks = script_getnum(st,2);
	sd = map_id2sd(st->rid);

	// detach the player
	if( sd && sd->npc_id == st->oid )
	{
		sd->npc_id = 0;
	}
	st->rid = 0;

	if( ticks <= 0 )
	{// do nothing
	}
	else if( st->sleep.tick == 0 )
	{// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = ticks;
	}
	else
	{// sleep time is over
		st->sleep.tick = 0;
	}
	return 0;
}

/// Pauses the execution of the script, keeping the player attached
/// Returns if a player is still attached
///
/// sleep2(<mili secconds>) -> <bool>
BUILDIN_FUNC(sleep2)
{
	int ticks;
	
	ticks = script_getnum(st,2);

	if( ticks <= 0 )
	{// do nothing
		script_pushint(st, (map_id2sd(st->rid)!=NULL));
	}
	else if( !st->sleep.tick )
	{// sleep for the target amount of time
		st->state = RERUNLINE;
		st->sleep.tick = ticks;
	}
	else
	{// sleep time is over
		st->sleep.tick = 0;
		script_pushint(st, (map_id2sd(st->rid)!=NULL));
	}
	return 0;
}

/// Awakes all the sleep timers of the target npc
///
/// awake "<npc name>";
BUILDIN_FUNC(awake)
{
	struct npc_data* nd;
	struct linkdb_node *node = (struct linkdb_node *)sleep_db;

	nd = npc_name2id(script_getstr(st, 2));
	if( nd == NULL )
		return 0;

	while( node )
	{
		if( (int)node->key == nd->bl.id )
		{// sleep timer for the npc
			struct script_state* tst = (struct script_state*)node->data;
			TBL_PC* sd = map_id2sd(tst->rid);

			if( tst->sleep.timer == -1 )
			{// already awake ???
				node = node->next;
				continue;
			}
			if( (sd && sd->status.char_id != tst->sleep.charid) || (tst->rid && !sd))
			{// char not online anymore / another char of the same account is online - Cancel execution
				tst->state = END;
				tst->rid = 0;
			}

			delete_timer(tst->sleep.timer, run_script_timer);
			node = script_erase_sleepdb(node);
			tst->sleep.timer = INVALID_TIMER;
			tst->sleep.tick = 0;
			run_script_main(tst);
		}
		else
		{
			node = node->next;
		}
	}
	return 0;
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
		ShowError("script:getvariableofnpc: not a variable\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return 1;
	}

	name = reference_getname(data);
	if( *name != '.' || name[1] == '@' )
	{// not a npc variable
		ShowError("script:getvariableofnpc: invalid scope (not npc variable)\n");
		script_reportdata(data);
		script_pushnil(st);
		st->state = END;
		return 1;
	}

	nd = npc_name2id(script_getstr(st,3));
	if( nd == NULL || nd->subtype != SCRIPT || nd->u.scr.script == NULL )
	{// NPC not found or has no script
		ShowError("script:getvariableofnpc: can't find npc %s\n", script_getstr(st,3));
		script_pushnil(st);
		st->state = END;
		return 1;
	}

	push_val2(st->stack, C_NAME, reference_getuid(data), &nd->u.scr.script->script_vars );
	return 0;
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
	if( bl == NULL )
	{
		ShowError("script:warpportal: npc is needed\n");
		return 1;
	}

	spx = script_getnum(st,2);
	spy = script_getnum(st,3);
	mapindex = mapindex_name2id(script_getstr(st, 4));
	tpx = script_getnum(st,5);
	tpy = script_getnum(st,6);

	if( mapindex == 0 )
		return 0;// map not found

	group = skill_unitsetting(bl, AL_WARP, 4, spx, spy, 0);
	if( group == NULL )
		return 0;// failed
	group->val2 = (tpx<<16) | tpy;
	group->val3 = mapindex;

	return 0;
}

BUILDIN_FUNC(openmail)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

#ifndef TXT_ONLY
	mail_openmail(sd);
#endif
	return 0;
}

BUILDIN_FUNC(openauction)
{
	TBL_PC* sd;

	sd = script_rid2sd(st);
	if( sd == NULL )
		return 0;

#ifndef TXT_ONLY
	clif_Auction_openwindow(sd);
#endif
	return 0;
}

/// Retrieves the value of the specified flag of the specified cell.
///
/// checkcell("<map name>",<x>,<y>,<type>) -> <bool>
///
/// @see cell_chk* constants in const.txt for the types
BUILDIN_FUNC(checkcell)
{
	int m = map_mapname2mapid(script_getstr(st,2));
	int x = script_getnum(st,3);
	int y = script_getnum(st,4);
	cell_chk type = (cell_chk)script_getnum(st,5);

	script_pushint(st, map_getcell(m, x, y, type));

	return 0;
}

/// Modifies flags of cells in the specified area.
///
/// setcell "<map name>",<x1>,<y1>,<x2>,<y2>,<type>,<flag>;
///
/// @see cell_* constants in const.txt for the types
BUILDIN_FUNC(setcell)
{
	int m = map_mapname2mapid(script_getstr(st,2));
	int x1 = script_getnum(st,3);
	int y1 = script_getnum(st,4);
	int x2 = script_getnum(st,5);
	int y2 = script_getnum(st,6);
	cell_t type = (cell_t)script_getnum(st,7);
	bool flag = (bool)script_getnum(st,8);

	int x,y;

	if( x1 > x2 ) swap(x1,x2);
	if( y1 > y2 ) swap(y1,y2);

	for( y = y1; y <= y2; ++y )
		for( x = x1; x <= x2; ++x )
			map_setcell(m, x, y, type, flag);

	return 0;
}

/******************
Questlog script commands
*******************/

BUILDIN_FUNC(getquest)
{

	TBL_PC * sd = script_rid2sd(st);
	struct quest qd;
	int i, count = 0;
	char * temp;

	memset(&qd, 0, sizeof(struct quest));

	qd.quest_id = script_getnum(st, 2);
	qd.time = script_getnum(st, 3);
	qd.state = Q_ACTIVE;
	
	for(i=0; i<(script_lastdata(st)-3) && (i/2) < MAX_QUEST_OBJECTIVES; i+=2)
	{
		temp = (char*)script_getstr(st, i+4);
		memcpy(&qd.objectives[i/2].name, temp, NAME_LENGTH);
		temp = NULL;
		qd.objectives[i/2].count = script_getnum(st, i+5);
		count++;
	}

	qd.num_objectives = count;

	quest_add(sd, &qd);

	return 0;

}

BUILDIN_FUNC(deletequest)
{

	TBL_PC * sd = script_rid2sd(st);
	int qid = script_getnum(st, 2);

	quest_delete(sd, qid);
	return 0;

}

BUILDIN_FUNC(setquestobjective)
{

	TBL_PC * sd = script_rid2sd(st);
	int qid = script_getnum(st, 2);
	int num = script_getnum(st, 3);
	const char * str = script_getstr(st, 4);
	int count = script_getnum(st, 5);

	quest_update_objective(sd, qid, num, str, count);

	return 0;
}

BUILDIN_FUNC(hasquest)
{

	TBL_PC * sd = script_rid2sd(st);
	int qid = script_getnum(st, 2);

	script_pushint(st, quest_has_quest(sd, qid)?1:0);

	return 0;
}

BUILDIN_FUNC(setqueststatus)
{

	TBL_PC * sd = script_rid2sd(st);
	int qid = script_getnum(st, 2);
	bool active = script_getnum(st, 3)?true:false;

	quest_update_status(sd, qid, active);

	return 0;
}


// declarations that were supposed to be exported from npc_chat.c
#ifdef PCRE_SUPPORT
BUILDIN_FUNC(defpattern);
BUILDIN_FUNC(activatepset);
BUILDIN_FUNC(deactivatepset);
BUILDIN_FUNC(deletepset);
#endif


struct script_function buildin_func[] = {
	// NPC interaction
	BUILDIN_DEF(mes,"s"),
	BUILDIN_DEF(next,""),
	BUILDIN_DEF(close,""),
	BUILDIN_DEF(close2,""),
	BUILDIN_DEF(menu,"sl*"),
	BUILDIN_DEF(select,"s*"), //for future jA script compatibility
	BUILDIN_DEF(prompt,"s*"),
	//
	BUILDIN_DEF(goto,"l"),
	BUILDIN_DEF(callsub,"i*"),
	BUILDIN_DEF(callfunc,"s*"),
	BUILDIN_DEF(return,"?"),
	BUILDIN_DEF(getarg,"i?"),
	BUILDIN_DEF(jobchange,"i*"),
	BUILDIN_DEF(jobname,"i"),
	BUILDIN_DEF(input,"v??"),
	BUILDIN_DEF(warp,"sii"),
	BUILDIN_DEF(areawarp,"siiiisii"),
	BUILDIN_DEF(warpchar,"siii"), // [LuzZza]
	BUILDIN_DEF(warpparty,"siii"), // [Fredzilla]
	BUILDIN_DEF(warpguild,"siii"), // [Fredzilla]
	BUILDIN_DEF(setlook,"ii"),
	BUILDIN_DEF(changelook,"ii"), // Simulates but don't Store it
	BUILDIN_DEF(set,"ii"),
	BUILDIN_DEF(setarray,"rv*"),
	BUILDIN_DEF(cleararray,"rvi"),
	BUILDIN_DEF(copyarray,"rri"),
	BUILDIN_DEF(getarraysize,"r"),
	BUILDIN_DEF(deletearray,"r?"),
	BUILDIN_DEF(getelementofarray,"ri"),
	BUILDIN_DEF(getitem,"vi?"),
	BUILDIN_DEF(getitem2,"iiiiiiiii*"),
	BUILDIN_DEF(getnameditem,"is"),
	BUILDIN_DEF2(grouprandomitem,"groupranditem","i"),
	BUILDIN_DEF(makeitem,"iisii"),
	BUILDIN_DEF(delitem,"ii?"),
	BUILDIN_DEF(delitem2,"iiiiiiiii?"),
	BUILDIN_DEF2(enableitemuse,"enable_items",""),
	BUILDIN_DEF2(disableitemuse,"disable_items",""),
	BUILDIN_DEF(cutin,"si"),
	BUILDIN_DEF(viewpoint,"iiiii"),
	BUILDIN_DEF(heal,"ii"),
	BUILDIN_DEF(itemheal,"ii"),
	BUILDIN_DEF(percentheal,"ii"),
	BUILDIN_DEF(rand,"i?"),
	BUILDIN_DEF(countitem,"i"),
	BUILDIN_DEF(countitem2,"iiiiiiii"),
	BUILDIN_DEF(checkweight,"ii"),
	BUILDIN_DEF(readparam,"i*"),
	BUILDIN_DEF(getcharid,"i*"),
	BUILDIN_DEF(getpartyname,"i"),
	BUILDIN_DEF(getpartymember,"i*"),
	BUILDIN_DEF(getpartyleader,"i?"),
	BUILDIN_DEF(getguildname,"i"),
	BUILDIN_DEF(getguildmaster,"i"),
	BUILDIN_DEF(getguildmasterid,"i"),
	BUILDIN_DEF(strcharinfo,"i"),
	BUILDIN_DEF(strnpcinfo,"i"),
	BUILDIN_DEF(getequipid,"i"),
	BUILDIN_DEF(getequipname,"i"),
	BUILDIN_DEF(getbrokenid,"i"), // [Valaris]
	BUILDIN_DEF(repair,"i"), // [Valaris]
	BUILDIN_DEF(getequipisequiped,"i"),
	BUILDIN_DEF(getequipisenableref,"i"),
	BUILDIN_DEF(getequipisidentify,"i"),
	BUILDIN_DEF(getequiprefinerycnt,"i"),
	BUILDIN_DEF(getequipweaponlv,"i"),
	BUILDIN_DEF(getequippercentrefinery,"i"),
	BUILDIN_DEF(successrefitem,"i"),
	BUILDIN_DEF(failedrefitem,"i"),
	BUILDIN_DEF(statusup,"i"),
	BUILDIN_DEF(statusup2,"ii"),
	BUILDIN_DEF(bonus,"iv"),
	BUILDIN_DEF2(bonus,"bonus2","ivi"),
	BUILDIN_DEF2(bonus,"bonus3","ivii"),
	BUILDIN_DEF2(bonus,"bonus4","iviii"),
	BUILDIN_DEF2(bonus,"bonus5","iviiii"),
	BUILDIN_DEF(bonusautoscript,"si?"),
	BUILDIN_DEF(bonusautoscript2,"si?"),
	BUILDIN_DEF(skill,"vi?"),
	BUILDIN_DEF(addtoskill,"vi?"), // [Valaris]
	BUILDIN_DEF(guildskill,"vi"),
	BUILDIN_DEF(getskilllv,"v"),
	BUILDIN_DEF(getgdskilllv,"iv"),
	BUILDIN_DEF(basicskillcheck,""),
	BUILDIN_DEF(getgmlevel,""),
	BUILDIN_DEF(end,""),
	BUILDIN_DEF(checkoption,"i"),
	BUILDIN_DEF(setoption,"i?"),
	BUILDIN_DEF(setcart,"?"),
	BUILDIN_DEF(checkcart,""),
	BUILDIN_DEF(setfalcon,"?"),
	BUILDIN_DEF(checkfalcon,""),
	BUILDIN_DEF(setriding,"?"),
	BUILDIN_DEF(checkriding,""),
	BUILDIN_DEF2(savepoint,"save","sii"),
	BUILDIN_DEF(savepoint,"sii"),
	BUILDIN_DEF(gettimetick,"i"),
	BUILDIN_DEF(gettime,"i"),
	BUILDIN_DEF(gettimestr,"si"),
	BUILDIN_DEF(openstorage,""),
	BUILDIN_DEF(guildopenstorage,"*"),
	BUILDIN_DEF(itemskill,"vi"),
	BUILDIN_DEF(produce,"i"),
	BUILDIN_DEF(monster,"siisii*"),
	BUILDIN_DEF(getmobdrops,"i"),
	BUILDIN_DEF(areamonster,"siiiisii*"),
	BUILDIN_DEF(killmonster,"ss?"),
	BUILDIN_DEF(killmonsterall,"s?"),
	BUILDIN_DEF(clone,"siisi*"),
	BUILDIN_DEF(doevent,"s"),
	BUILDIN_DEF(donpcevent,"s"),
	BUILDIN_DEF(cmdothernpc,"ss"),
	BUILDIN_DEF(addtimer,"is"),
	BUILDIN_DEF(deltimer,"s"),
	BUILDIN_DEF(addtimercount,"si"),
	BUILDIN_DEF(initnpctimer,"??"),
	BUILDIN_DEF(stopnpctimer,"??"),
	BUILDIN_DEF(startnpctimer,"??"),
	BUILDIN_DEF(setnpctimer,"i?"),
	BUILDIN_DEF(getnpctimer,"i?"),
	BUILDIN_DEF(attachnpctimer,"?"), // attached the player id to the npc timer [Celest]
	BUILDIN_DEF(detachnpctimer,"?"), // detached the player id from the npc timer [Celest]
	BUILDIN_DEF(playerattached,""), // returns id of the current attached player. [Skotlex]
	BUILDIN_DEF(announce,"si*"),
	BUILDIN_DEF(mapannounce,"ssi*"),
	BUILDIN_DEF(areaannounce,"siiiisi*"),
	BUILDIN_DEF(getusers,"i"),
	BUILDIN_DEF(getmapguildusers,"si"),
	BUILDIN_DEF(getmapusers,"s"),
	BUILDIN_DEF(getareausers,"siiii"),
	BUILDIN_DEF(getareadropitem,"siiiii"),
	BUILDIN_DEF(enablenpc,"s"),
	BUILDIN_DEF(disablenpc,"s"),
	BUILDIN_DEF(hideoffnpc,"s"),
	BUILDIN_DEF(hideonnpc,"s"),
	BUILDIN_DEF(sc_start,"iii?"),
	BUILDIN_DEF(sc_start2,"iiii?"),
	BUILDIN_DEF(sc_start4,"iiiiii?"),
	BUILDIN_DEF(sc_end,"i?"),
	BUILDIN_DEF(getscrate,"ii*"),
	BUILDIN_DEF(debugmes,"s"),
	BUILDIN_DEF2(catchpet,"pet","i"),
	BUILDIN_DEF2(birthpet,"bpet",""),
	BUILDIN_DEF(resetlvl,"i"),
	BUILDIN_DEF(resetstatus,""),
	BUILDIN_DEF(resetskill,""),
	BUILDIN_DEF(skillpointcount,""),
	BUILDIN_DEF(changebase,"i"),
	BUILDIN_DEF(changesex,""),
	BUILDIN_DEF(waitingroom,"si??"),
	BUILDIN_DEF(delwaitingroom,"?"),
	BUILDIN_DEF2(waitingroomkickall,"kickwaitingroomall","?"),
	BUILDIN_DEF(enablewaitingroomevent,"?"),
	BUILDIN_DEF(disablewaitingroomevent,"?"),
	BUILDIN_DEF2(enablewaitingroomevent,"enablearena",""),		// Added by RoVeRT
	BUILDIN_DEF2(disablewaitingroomevent,"disablearena",""),	// Added by RoVeRT
	BUILDIN_DEF(getwaitingroomstate,"i?"),
	BUILDIN_DEF(warpwaitingpc,"sii?"),
	BUILDIN_DEF(attachrid,"i"),
	BUILDIN_DEF(detachrid,""),
	BUILDIN_DEF(isloggedin,"i?"),
	BUILDIN_DEF(setmapflagnosave,"ssii"),
	BUILDIN_DEF(getmapflag,"si"),
	BUILDIN_DEF(setmapflag,"si*"),
	BUILDIN_DEF(removemapflag,"si"),
	BUILDIN_DEF(pvpon,"s"),
	BUILDIN_DEF(pvpoff,"s"),
	BUILDIN_DEF(gvgon,"s"),
	BUILDIN_DEF(gvgoff,"s"),
	BUILDIN_DEF(emotion,"i*"),
	BUILDIN_DEF(maprespawnguildid,"sii"),
	BUILDIN_DEF(agitstart,""),	// <Agit>
	BUILDIN_DEF(agitend,""),
	BUILDIN_DEF(agitcheck,""),   // <Agitcheck>
	BUILDIN_DEF(flagemblem,"i"),	// Flag Emblem
	BUILDIN_DEF(getcastlename,"s"),
	BUILDIN_DEF(getcastledata,"si*"),
	BUILDIN_DEF(setcastledata,"sii"),
	BUILDIN_DEF(requestguildinfo,"i*"),
	BUILDIN_DEF(getequipcardcnt,"i"),
	BUILDIN_DEF(successremovecards,"i"),
	BUILDIN_DEF(failedremovecards,"ii"),
	BUILDIN_DEF(marriage,"s"),
	BUILDIN_DEF2(wedding_effect,"wedding",""),
	BUILDIN_DEF(divorce,""),
	BUILDIN_DEF(ispartneron,""),
	BUILDIN_DEF(getpartnerid,""),
	BUILDIN_DEF(getchildid,""),
	BUILDIN_DEF(getmotherid,""),
	BUILDIN_DEF(getfatherid,""),
	BUILDIN_DEF(warppartner,"sii"),
	BUILDIN_DEF(getitemname,"i"),
	BUILDIN_DEF(getitemslots,"i"),
	BUILDIN_DEF(makepet,"i"),
	BUILDIN_DEF(getexp,"ii"),
	BUILDIN_DEF(getinventorylist,""),
	BUILDIN_DEF(getskilllist,""),
	BUILDIN_DEF(clearitem,""),
	BUILDIN_DEF(classchange,"ii"),
	BUILDIN_DEF(misceffect,"i"),
	BUILDIN_DEF(soundeffect,"si"),
	BUILDIN_DEF(soundeffectall,"si*"),	// SoundEffectAll [Codemaster]
	BUILDIN_DEF(strmobinfo,"ii"),	// display mob data [Valaris]
	BUILDIN_DEF(guardian,"siisi??"),	// summon guardians
	BUILDIN_DEF(guardianinfo,"sii"),	// display guardian data [Valaris]
	BUILDIN_DEF(petskillbonus,"iiii"), // [Valaris]
	BUILDIN_DEF(petrecovery,"ii"), // [Valaris]
	BUILDIN_DEF(petloot,"i"), // [Valaris]
	BUILDIN_DEF(petheal,"iiii"), // [Valaris]
	BUILDIN_DEF(petskillattack,"viii"), // [Skotlex]
	BUILDIN_DEF(petskillattack2,"viiii"), // [Valaris]
	BUILDIN_DEF(petskillsupport,"viiii"), // [Skotlex]
	BUILDIN_DEF(skilleffect,"vi"), // skill effect [Celest]
	BUILDIN_DEF(npcskilleffect,"viii"), // npc skill effect [Valaris]
	BUILDIN_DEF(specialeffect,"i*"), // npc skill effect [Valaris]
	BUILDIN_DEF(specialeffect2,"i*"), // skill effect on players[Valaris]
	BUILDIN_DEF(nude,""), // nude command [Valaris]
	BUILDIN_DEF(mapwarp,"ssii*"),		// Added by RoVeRT
	BUILDIN_DEF(atcommand,"*"), // [MouseJstr]
	BUILDIN_DEF(charcommand,"*"), // [MouseJstr]
	BUILDIN_DEF(movenpc,"sii"), // [MouseJstr]
	BUILDIN_DEF(message,"s*"), // [MouseJstr]
	BUILDIN_DEF(npctalk,"*"), // [Valaris]
	BUILDIN_DEF(mobcount,"ss"),
	BUILDIN_DEF(getlook,"i"),
	BUILDIN_DEF(getsavepoint,"i"),
	BUILDIN_DEF(npcspeed,"i"), // [Valaris]
	BUILDIN_DEF(npcwalkto,"ii"), // [Valaris]
	BUILDIN_DEF(npcstop,""), // [Valaris]
	BUILDIN_DEF(getmapxy,"siii*"),	//by Lorky [Lupus]
	BUILDIN_DEF(checkoption1,"i"),
	BUILDIN_DEF(checkoption2,"i"),
	BUILDIN_DEF(guildgetexp,"i"),
	BUILDIN_DEF(guildchangegm,"is"),
	BUILDIN_DEF(logmes,"s"), //this command actls as MES but rints info into LOG file either SQL/TXT [Lupus]
	BUILDIN_DEF(summon,"si*"), // summons a slave monster [Celest]
	BUILDIN_DEF(isnight,""), // check whether it is night time [Celest]
	BUILDIN_DEF(isday,""), // check whether it is day time [Celest]
	BUILDIN_DEF(isequipped,"i*"), // check whether another item/card has been equipped [Celest]
	BUILDIN_DEF(isequippedcnt,"i*"), // check how many items/cards are being equipped [Celest]
	BUILDIN_DEF(cardscnt,"i*"), // check how many items/cards are being equipped in the same arm [Lupus]
	BUILDIN_DEF(getrefine,"*"), // returns the refined number of the current item, or an item with index specified [celest]
	BUILDIN_DEF(night,""), // sets the server to night time
	BUILDIN_DEF(day,""), // sets the server to day time
#ifdef PCRE_SUPPORT
	BUILDIN_DEF(defpattern,"iss"), // Define pattern to listen for [MouseJstr]
	BUILDIN_DEF(activatepset,"i"), // Activate a pattern set [MouseJstr]
	BUILDIN_DEF(deactivatepset,"i"), // Deactive a pattern set [MouseJstr]
	BUILDIN_DEF(deletepset,"i"), // Delete a pattern set [MouseJstr]
#endif
	BUILDIN_DEF(dispbottom,"s"), //added from jA [Lupus]
	BUILDIN_DEF(getusersname,"*"),
	BUILDIN_DEF(recovery,""),
	BUILDIN_DEF(getpetinfo,"i"),
	BUILDIN_DEF(gethominfo,"i"),
	BUILDIN_DEF(checkequipedcard,"i"),
	BUILDIN_DEF(jump_zero,"ii"), //for future jA script compatibility
	BUILDIN_DEF(globalmes,"s*"),
	BUILDIN_DEF(getmapmobs,"s"), //end jA addition
	BUILDIN_DEF(unequip,"i"), // unequip command [Spectre]
	BUILDIN_DEF(getstrlen,"s"), //strlen [Valaris]
	BUILDIN_DEF(charisalpha,"si"), //isalpha [Valaris]
	BUILDIN_DEF(setnpcdisplay,"sv??"),
	BUILDIN_DEF(compare,"ss"), // Lordalfa - To bring strstr to scripting Engine.
	BUILDIN_DEF(getiteminfo,"ii"), //[Lupus] returns Items Buy / sell Price, etc info
	BUILDIN_DEF(setiteminfo,"iii"), //[Lupus] set Items Buy / sell Price, etc info
	BUILDIN_DEF(getequipcardid,"ii"), //[Lupus] returns CARD ID or other info from CARD slot N of equipped item
	// [zBuffer] List of mathematics commands --->
	BUILDIN_DEF(sqrt,"i"),
	BUILDIN_DEF(pow,"ii"),
	BUILDIN_DEF(distance,"iiii"),
	// <--- [zBuffer] List of mathematics commands
	// [zBuffer] List of dynamic var commands --->
	BUILDIN_DEF(getd,"*"),
	BUILDIN_DEF(setd,"*"),
	// <--- [zBuffer] List of dynamic var commands
	BUILDIN_DEF(petstat,"i"),
	BUILDIN_DEF(callshop,"si"), // [Skotlex]
	BUILDIN_DEF(npcshopitem,"sii*"), // [Lance]
	BUILDIN_DEF(npcshopadditem,"sii*"),
	BUILDIN_DEF(npcshopdelitem,"si*"),
	BUILDIN_DEF(npcshopattach,"s?"),
	BUILDIN_DEF(equip,"i"),
	BUILDIN_DEF(autoequip,"ii"),
	BUILDIN_DEF(setbattleflag,"ss"),
	BUILDIN_DEF(getbattleflag,"s"),
	BUILDIN_DEF(setitemscript,"is*"), //Set NEW item bonus script. Lupus
	BUILDIN_DEF(disguise,"i"), //disguise player. Lupus
	BUILDIN_DEF(undisguise,"*"), //undisguise player. Lupus
	BUILDIN_DEF(getmonsterinfo,"ii"), //Lupus
	BUILDIN_DEF(axtoi,"s"),
	BUILDIN_DEF(query_sql,"s*"),
	BUILDIN_DEF(query_logsql,"s*"),
	BUILDIN_DEF(escape_sql,"s"),
	BUILDIN_DEF(atoi,"s"),
	// [zBuffer] List of player cont commands --->
	BUILDIN_DEF(rid2name,"i"),
	BUILDIN_DEF(pcfollow,"ii"),
	BUILDIN_DEF(pcstopfollow,"i"),
	BUILDIN_DEF(pcblockmove,"ii"),
	// <--- [zBuffer] List of player cont commands
	// [zBuffer] List of mob control commands --->
	BUILDIN_DEF(unitwalk,"ii?"),
	BUILDIN_DEF(unitkill,"i"),
	BUILDIN_DEF(unitwarp,"isii"),
	BUILDIN_DEF(unitattack,"iv?"),
	BUILDIN_DEF(unitstop,"i"),
	BUILDIN_DEF(unittalk,"is"),
	BUILDIN_DEF(unitemote,"ii"),
	BUILDIN_DEF(unitskilluseid,"ivi?"), // originally by Qamera [Celest]
	BUILDIN_DEF(unitskillusepos,"iviii"), // [Celest]
// <--- [zBuffer] List of mob control commands
	BUILDIN_DEF(sleep,"i"),
	BUILDIN_DEF(sleep2,"i"),
	BUILDIN_DEF(awake,"s"),
	BUILDIN_DEF(getvariableofnpc,"rs"),
	BUILDIN_DEF(warpportal,"iisii"),
	BUILDIN_DEF2(homunculus_evolution,"homevolution",""),	//[orn]
	BUILDIN_DEF2(homunculus_shuffle,"homshuffle",""),	//[Zephyrus]
	BUILDIN_DEF(eaclass,"*"),	//[Skotlex]
	BUILDIN_DEF(roclass,"i*"),	//[Skotlex]
	BUILDIN_DEF(checkvending,"*"),
	BUILDIN_DEF(checkchatting,"*"),
	BUILDIN_DEF(openmail,""),
	BUILDIN_DEF(openauction,""),
	BUILDIN_DEF(checkcell,"siii"),
	BUILDIN_DEF(setcell,"siiiiii"),
	BUILDIN_DEF(getquest, "ii*"),
	BUILDIN_DEF(deletequest, "i"),
	BUILDIN_DEF(setquestobjective, "iisi"),
	BUILDIN_DEF(setqueststatus, "ii"),
	BUILDIN_DEF(hasquest, "i"),
	BUILDIN_DEF(setwall,"siiiiis"),
	BUILDIN_DEF(delwall,"s"),
	{NULL,NULL,NULL},
};
