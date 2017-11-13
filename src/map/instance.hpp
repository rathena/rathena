// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INSTANCE_HPP_
#define _INSTANCE_HPP_

#include "../common/cbasetypes.h"
#include "../common/mmo.h" // struct point
#include "../common/strlib.h"

#include "script.hpp" // struct reg_db

enum send_target : uint8;
struct block_list;

#define MAX_INSTANCE_DATA		300	// Essentially how many instances we can create, but instance creation is primarily decided by MAX_MAP_PER_SERVER
#define MAX_MAP_PER_INSTANCE	255	// Max number of maps per instance (Enter map is counted as one) - Supports up to 255 maps

#define INSTANCE_NAME_LENGTH (60+1)

enum instance_state {
	INSTANCE_FREE,
	INSTANCE_IDLE,
	INSTANCE_BUSY
};

enum instance_mode {
	IM_NONE,
	IM_CHAR,
	IM_PARTY,
	IM_GUILD,
	IM_CLAN,
	IM_MAX,
};

enum e_instance_enter {
	IE_OK = 0,
	IE_NOMEMBER,
	IE_NOINSTANCE,
	IE_OTHER
};

struct s_instance_map {
	int16 m, src_m;
};

struct instance_data {
	unsigned short type; ///< Instance DB ID
	enum instance_state state; ///< State of instance
	enum instance_mode mode; ///< Mode of instance
	int owner_id; ///< Owner ID of instance
	unsigned int keep_limit; ///< Life time of instance
	int keep_timer; ///< Remaining life time of instance
	unsigned int idle_limit; ///< Idle time of instance
	int idle_timer; ///< Remaining idle time of instance
	struct reg_db regs; ///< Instance variables for scripts
	struct s_instance_map **map; ///< Dynamic array of maps in instance
	uint8 cnt_map; ///< Number of maps in an instance
};

/// Instance DB entry struct
struct instance_db {
	unsigned short id; ///< Instance ID
	StringBuf *name; ///< Instance name
	unsigned int limit, ///< Duration limit
		timeout; ///< Timeout limit
	struct s_MapInfo {
		StringBuf *mapname; ///< Mapname, the limit should be MAP_NAME_LENGTH_EXT
		short x, y; ///< Map coordinates
	} enter;
	StringBuf **maplist; ///< Used maps in instance, the limit should be MAP_NAME_LENGTH_EXT
	uint8 maplist_count; ///< Number of used maps
};

extern int instance_start;
extern struct instance_data instance_data[MAX_INSTANCE_DATA];

struct instance_db *instance_searchtype_db(unsigned short instance_id);
struct instance_db *instance_searchname_db(const char* name);
void instance_getsd(unsigned short instance_id, struct map_session_data **sd, enum send_target *target);

int instance_create(int owner_id, const char *name, enum instance_mode mode);
int instance_destroy(unsigned short instance_id);
enum e_instance_enter instance_enter(struct map_session_data *sd, unsigned short instance_id, const char *name, short x, short y);
int instance_reqinfo(struct map_session_data *sd, unsigned short instance_id);
int instance_addusers(unsigned short instance_id);
int instance_delusers(unsigned short instance_id);
int16 instance_mapname2mapid(const char *name, unsigned short instance_id);
int instance_addmap(unsigned short instance_id);

void instance_addnpc(struct instance_data *im);
void instance_readdb(void);
void instance_reload(void);
void do_reload_instance(void);
void do_init_instance(void);
void do_final_instance(void);

#if MAX_MAP_PER_INSTANCE > 255
	#error Too many maps per instance defined! Please adjust MAX_MAP_PER_INSTANCE to a lower value.
#endif

#endif /* _INSTANCE_HPP_ */
