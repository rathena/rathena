// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MERCENARY_H_
#define _MERCENARY_H_

#include "status.h" // struct status_data, struct status_change
#include "unit.h" // struct unit_data

// number of cells that a mercenary can walk to from it's master before being warped
#define MAX_MER_DISTANCE 15

enum {
	ARCH_MERC_GUILD,
	SPEAR_MERC_GUILD,
	SWORD_MERC_GUILD,
};

struct s_mercenary_db {
	int class_;
	char sprite[NAME_LENGTH], name[NAME_LENGTH];
	unsigned short lv;
	short range2, range3;
	struct status_data status;
	struct view_data vd;
	struct {
		unsigned short id, lv;
	} skill[MAX_MERCSKILL];
};

extern struct s_mercenary_db mercenary_db[MAX_MERCENARY_CLASS];

struct mercenary_data {
	struct block_list bl;
	struct unit_data ud;
	struct view_data *vd;
	struct status_data base_status, battle_status;
	struct status_change sc;
	struct regen_data regen;

	struct s_mercenary_db *db;
	struct s_mercenary mercenary;
	char blockskill[MAX_SKILL];

	int masterteleport_timer;
	struct map_session_data *master;
	int contract_timer;

	unsigned devotion_flag : 1;
};

bool mercenary_class(int class_);
struct view_data * mercenary_get_viewdata(int class_);

bool mercenary_create(struct map_session_data *sd, int class_, unsigned int lifetime);
bool mercenary_recv_data(struct s_mercenary *merc, bool flag);
void mercenary_save(struct mercenary_data *md);

void mercenary_heal(struct mercenary_data *md, int hp, int sp);
bool mercenary_dead(struct mercenary_data *md);

int mercenary_delete(struct mercenary_data *md, int reply);
void mercenary_contract_stop(struct mercenary_data *md);

int mercenary_get_lifetime(struct mercenary_data *md);
int mercenary_get_guild(struct mercenary_data *md);
int mercenary_get_faith(struct mercenary_data *md);
void mercenary_set_faith(struct mercenary_data *md, int value);
int mercenary_get_calls(struct mercenary_data *md);
void mercenary_set_calls(struct mercenary_data *md, int value);
void mercenary_kills(struct mercenary_data *md);

int mercenary_checkskill(struct mercenary_data *md, uint16 skill_id);

/**
 * atcommand.c required
 **/
void mercenary_readdb(void);
void mercenary_read_skilldb(void);

void do_init_mercenary(void);
void do_final_mercenary(void);

#endif /* _MERCENARY_H_ */
