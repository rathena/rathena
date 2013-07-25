// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _DUEL_H_
#define _DUEL_H_

#ifdef	__cplusplus
extern "C" {
#endif

struct duel {
	int members_count;
	int invites_count;
	int max_players_limit;
};

#define MAX_DUEL 1024 //max number of duels on server
extern struct duel duel_list[MAX_DUEL]; //list of current duel
extern int duel_count; //current number of duel on server

//Duel functions // [LuzZza]
int duel_create(struct map_session_data* sd, const unsigned int maxpl);
void duel_invite(const unsigned int did, struct map_session_data* sd, struct map_session_data* target_sd);
void duel_accept(const unsigned int did, struct map_session_data* sd);
void duel_reject(const unsigned int did, struct map_session_data* sd);
void duel_leave(const unsigned int did, struct map_session_data* sd);
void duel_showinfo(const unsigned int did, struct map_session_data* sd);
int duel_checktime(struct map_session_data* sd);

void do_init_duel(void);
void do_final_duel(void);

#ifdef	__cplusplus
}
#endif

#endif /* _DUEL_H_ */
