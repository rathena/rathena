// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _SCRIPT_H_
#define _SCRIPT_H_

extern int potion_flag; //For use on Alchemist improved potions/Potion Pitcher. [Skotlex]
extern int potion_hp, potion_per_hp, potion_sp, potion_per_sp;
extern int potion_target;

extern struct Script_Config {
	unsigned warn_func_mismatch_paramnum : 1;
	int check_cmdcount;
	int check_gotocount;
	int input_min_value;
	int input_max_value;

	const char *die_event_name;
	const char *kill_pc_event_name;
	const char *kill_mob_event_name;
	const char *login_event_name;
	const char *logout_event_name;
	const char *loadmap_event_name;
	const char *baselvup_event_name;
	const char *joblvup_event_name;
} script_config;

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
	C_L_SHIFT // a << b
} c_op;

struct script_data {
	enum c_op type;
	union script_data_val {
		int num;
		char *str;
	} u;
	struct linkdb_node** ref;
};

// Moved defsp from script_state to script_stack since
// it must be saved when script state is RERUNLINE. [Eoe / jA 1094]
struct script_code {
	int script_size;
	unsigned char* script_buf;
	struct linkdb_node* script_vars;
};

struct script_stack {
	int sp;// number of entries in the stack
	int sp_max;// capacity of the stack
	int defsp;
	struct script_data *stack_data;// stack
	struct linkdb_node **var_function;	// ŠÖ”ˆË‘¶•Ï”
};

struct script_state {
	struct script_stack* stack;
	int start,end;
	int pos,state;
	int rid,oid;
	struct script_code *script, *scriptroot;
	struct sleep_data {
		int tick,timer,charid;
	} sleep;
};

struct script_reg {
	int index;
	int data;
};

struct script_regstr {
	int index;
	char* data;
};

enum script_parse_options {
	SCRIPT_USE_LABEL_DB = 0x1,// records labels in scriptlabel_db
	SCRIPT_IGNORE_EXTERNAL_BRACKETS = 0x2,// ignores the check for {} brackets around the script
	SCRIPT_RETURN_EMPTY_SCRIPT = 0x4// returns the script object instead of NULL for empty scripts
};

const char* skip_space(const char* p);
const char* script_print_line(const char* p, const char* mark, int line);
void script_error(const char *src,const char *file,int start_line, const char *error_msg, const char *error_pos);

struct script_code* parse_script(const char* src,const char* file,int line,int options);
void run_script_sub(struct script_code *rootscript,int pos,int rid,int oid, char* file, int lineno);
void run_script(struct script_code*,int,int,int);

int set_var(struct map_session_data *sd, char *name, void *val);
int conv_num(struct script_state *st,struct script_data *data);
const char* conv_str(struct script_state *st,struct script_data *data);
void setd_sub(struct script_state *st, struct map_session_data *sd, char *varname, int elem, void *value, struct linkdb_node **ref);
int run_script_timer(int tid, unsigned int tick, int id, intptr data);
void run_script_main(struct script_state *st);

void script_stop_sleeptimers(int id);
struct linkdb_node* script_erase_sleepdb(struct linkdb_node *n);
void script_free_stack(struct script_stack*); 
void script_free_code(struct script_code* code);
void script_free_vars(struct linkdb_node **node);

struct DBMap* script_get_label_db(void);
struct DBMap* script_get_userfunc_db(void);

int script_config_read(char *cfgName);
int do_init_script(void);
int do_final_script(void);
int add_str(const char *p);
int script_reload(void);

extern char mapreg_txt[];

#endif /* _SCRIPT_H_ */
