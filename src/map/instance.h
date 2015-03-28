// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INSTANCE_H_
#define _INSTANCE_H_

#define MAX_INSTANCE_DATA	300	// Essentially how many instances we can create, but instance creation is primarily decided by MAX_MAP_PER_SERVER	
#define MAX_MAP_PER_INSTANCE 	10	// Max number of maps per instance

#define INSTANCE_NAME_LENGTH (60+1)

typedef enum instance_state { INSTANCE_FREE, INSTANCE_IDLE, INSTANCE_BUSY } instance_state;

struct instance_data {
	unsigned short type, ///< Instance DB ID
		cnt_map;
	int state;
	int party_id;
	unsigned int keep_limit;
	int keep_timer;
	unsigned int idle_limit;
	int idle_timer;

	struct DBMap* vars; // Instance Variable for scripts
	struct {
		int m;
		int src_m;
	} map[MAX_MAP_PER_INSTANCE];
};

extern int instance_start;
extern struct instance_data instance_data[MAX_INSTANCE_DATA];

int instance_create(int party_id, const char *name);
int instance_destroy(short instance_id);
int instance_enter(struct map_session_data *sd, const char *name);
int instance_enter_position(struct map_session_data *sd, const char *name, short x, short y);
int instance_reqinfo(struct map_session_data *sd, short instance_id);
int instance_addusers(short instance_id);
int instance_delusers(short instance_id);
int instance_mapname2mapid(const char *name, short instance_id);
int instance_addmap(short instance_id);

void instance_addnpc(struct instance_data *im);
void instance_readdb(void);
void instance_reload(void);
void do_reload_instance(void);
void do_init_instance(void);
void do_final_instance(void);

#endif
