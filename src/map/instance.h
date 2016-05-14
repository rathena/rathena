// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#include "../common/mmo.h" // struct point
#include "script.h" // struct reg_db

struct block_list;

#define MAX_INSTANCE_DATA	300	// Essentially how many instances we can create, but instance creation is primarily decided by MAX_MAP_PER_SERVER	
#define MAX_MAP_PER_INSTANCE 	10	// Max number of maps per instance

#define INSTANCE_NAME_LENGTH (60+1)

typedef enum instance_state {
	INSTANCE_FREE,
	INSTANCE_IDLE,
	INSTANCE_BUSY
} instance_state;

enum instance_mode {
	IM_NONE,
	IM_CHAR,
	IM_PARTY,
	IM_GUILD,
	IM_MAX,
};

struct instance_data {
	unsigned short type, ///< Instance DB ID
		cnt_map;
	char name[INSTANCE_NAME_LENGTH];
	enum instance_state state;
	enum instance_mode mode;
	int owner_id;
	unsigned int keep_limit;
	int keep_timer;
	unsigned int idle_limit;
	int idle_timer;

	struct reg_db regs; ///< Instance variables for scripts

	struct {
		int m;
		int src_m;
	} map[MAX_MAP_PER_INSTANCE];
};

extern int instance_start;
extern struct instance_data instance_data[MAX_INSTANCE_DATA];

void instance_getsd(unsigned short instance_id, struct map_session_data **sd, enum send_target *target);

int instance_create(int owner_id, const char *name, enum instance_mode mode);
int instance_destroy(unsigned short instance_id);
int instance_enter(struct map_session_data *sd, unsigned short instance_id, const char *name);
int instance_enter_position(struct map_session_data *sd, unsigned short instance_id, const char *name, short x, short y);
int instance_reqinfo(struct map_session_data *sd, unsigned short instance_id);
int instance_addusers(unsigned short instance_id);
int instance_delusers(unsigned short instance_id);
int instance_mapname2mapid(const char *name, unsigned short instance_id);
int instance_addmap(unsigned short instance_id);

void instance_addnpc(struct instance_data *im);
void instance_readdb(void);
void instance_reload(void);
void do_reload_instance(void);
void do_init_instance(void);
void do_final_instance(void);

#endif
