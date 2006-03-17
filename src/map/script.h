// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _SCRIPT_H_
#define _SCRIPT_H_

extern int potion_flag; //For use on Alchemist improved potions/Potion Pitcher. [Skotlex]
extern int potion_hp, potion_per_hp, potion_sp, potion_per_sp;
extern int potion_target;

extern struct Script_Config {
	unsigned verbose_mode : 1;
	unsigned warn_func_no_comma : 1;
	unsigned warn_cmd_no_comma : 1;
	unsigned warn_func_mismatch_paramnum : 1;
	unsigned warn_cmd_mismatch_paramnum : 1;
	int check_cmdcount;
	int check_gotocount;

	unsigned event_script_type : 1;
	unsigned event_requires_trigger : 1;
	char die_event_name[NAME_LENGTH];
	char kill_pc_event_name[NAME_LENGTH];
	char kill_mob_event_name[NAME_LENGTH];
	char login_event_name[NAME_LENGTH];
	char logout_event_name[NAME_LENGTH];
	char loadmap_event_name[NAME_LENGTH];
	char baselvup_event_name[NAME_LENGTH];
	char joblvup_event_name[NAME_LENGTH];
} script_config;

struct script_data {
	int type;
	union {
		int num;
		char *str;
	} u;
};

// Moved defsp from script_state to script_stack since
// it must be saved when script state is RERUNLINE. [Eoe / jA 1094]
struct script_stack {
	int sp,sp_max,defsp;
	struct script_data *stack_data;
};
struct script_state {
	struct script_stack *stack;
	int start,end;
	int pos,state;
	int rid,oid;
	unsigned char *script,*new_script;
	int new_pos,new_defsp;
};

unsigned char * parse_script(unsigned char *,int);
int run_script(unsigned char *,int,int,int);

int set_var(struct map_session_data *sd, char *name, void *val);
int conv_num(struct script_state *st,struct script_data *data);
char* conv_str(struct script_state *st,struct script_data *data);

struct dbt* script_get_label_db(void);
struct dbt* script_get_userfunc_db(void);

int script_config_read(char *cfgName);
void script_free_stack(struct script_stack*); 
int do_init_script(void);
int do_final_script(void);
int script_reload(void);

extern char mapreg_txt[];

#endif

