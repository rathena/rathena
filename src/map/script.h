// $Id: script.h,v 1.2 2004/09/25 05:32:19 MouseJstr Exp $
#ifndef _SCRIPT_H_
#define _SCRIPT_H_

extern struct Script_Config {
	int warn_func_no_comma;
	int warn_cmd_no_comma;
	int warn_func_mismatch_paramnum;
	int warn_cmd_mismatch_paramnum;
	int check_cmdcount;
	int check_gotocount;
} script_config;

struct script_data {
	int type;
	union {
		int num;
		char *str;
	} u;
};

struct script_stack {
	int sp,sp_max;
	struct script_data *stack_data;
};
struct script_state {
	struct script_stack *stack;
	int start,end;
	int pos,state;
	int rid,oid;
	char *script,*new_script;
	int defsp,new_pos,new_defsp;
};

char * parse_script(unsigned char *,int);
int run_script(char *,int,int,int);

struct dbt* script_get_label_db();
struct dbt* script_get_userfunc_db();

int script_config_read(char *cfgName);
int do_init_script();
int do_final_script();

extern char mapreg_txt[];

#endif

