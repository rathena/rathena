// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _PARTY_H_
#define _PARTY_H_

#include "../common/mmo.h" // struct party
struct block_list;
struct map_session_data;
struct party;
struct item;

#include <stdarg.h>

#define PARTY_BOOKING_JOBS 6
#define PARTY_BOOKING_RESULTS 10

struct party_member_data {
	struct map_session_data *sd;
	unsigned int hp; //For HP,x,y refreshing.
	unsigned short x, y;
};

struct party_data {
	struct party party;
	struct party_member_data data[MAX_PARTY];
	uint8 itemc; //For item distribution, position of last picker in party
	unsigned int instance_id;
	struct {
		unsigned monk : 1; //There's at least one monk in party?
		unsigned sg : 1;	//There's at least one Star Gladiator in party?
		unsigned snovice :1; //There's a Super Novice
		unsigned tk : 1; //There's a taekwon
	} state;
};

struct party_booking_detail {
	short level;
    short mapid;
    short job[PARTY_BOOKING_JOBS];
};

struct party_booking_ad_info {
	unsigned long index;
	char charname[NAME_LENGTH];
	long starttime;
	struct party_booking_detail p_detail;
};

extern int party_create_byscript;

void do_init_party(void);
void do_final_party(void);
struct party_data* party_search(int party_id);
struct party_data* party_searchname(const char* str);
int party_getmemberid(struct party_data* p, struct map_session_data* sd);
struct map_session_data* party_getavailablesd(struct party_data *p);

int party_create(struct map_session_data *sd,char *name, int item, int item2);
void party_created(int account_id,int char_id,int fail,int party_id,char *name);
int party_request_info(int party_id, int char_id);
int party_invite(struct map_session_data *sd,struct map_session_data *tsd);
void party_member_joined(struct map_session_data *sd);
int party_member_added(int party_id,int account_id,int char_id,int flag);
int party_leave(struct map_session_data *sd);
int party_removemember(struct map_session_data *sd,int account_id,char *name);
int party_removemember2(struct map_session_data *sd,int char_id,int party_id);
int party_member_withdraw(int party_id,int account_id,int char_id);
int party_reply_invite(struct map_session_data *sd,int party_id,int flag);
#define party_add_member(party_id,sd) party_reply_invite(sd,party_id,1)
int party_recv_noinfo(int party_id, int char_id);
int party_recv_info(struct party* sp, int char_id);
int party_recv_movemap(int party_id,int account_id,int char_id, unsigned short map,int online,int lv);
int party_broken(int party_id);
int party_optionchanged(int party_id,int account_id,int exp,int item,int flag);
int party_changeoption(struct map_session_data *sd,int exp,int item);
int party_setoption(struct party_data *party, int option, int flag);
int party_changeleader(struct map_session_data *sd, struct map_session_data *t_sd, struct party_data *p);
void party_send_movemap(struct map_session_data *sd);
void party_send_levelup(struct map_session_data *sd);
int party_send_logout(struct map_session_data *sd);
int party_send_message(struct map_session_data *sd,const char *mes,int len);
int party_recv_message(int party_id,int account_id,const char *mes,int len);
int party_skill_check(struct map_session_data *sd, int party_id, uint16 skill_id, uint16 skill_lv);
int party_send_xy_clear(struct party_data *p);
int party_exp_share(struct party_data *p,struct block_list *src,unsigned int base_exp,unsigned int job_exp,int zeny);
int party_share_loot(struct party_data* p, struct map_session_data* sd, struct item* item_data, int first_charid);
int party_send_dot_remove(struct map_session_data *sd);
int party_sub_count(struct block_list *bl, va_list ap);
int party_sub_count_class(struct block_list *bl, va_list ap);
int party_foreachsamemap(int (*func)(struct block_list *,va_list),struct map_session_data *sd,int range,...);

/*==========================================
 * Party Booking in KRO [Spiria]
 *------------------------------------------*/
void party_booking_register(struct map_session_data *sd, short level, short mapid, short* job);
void party_booking_update(struct map_session_data *sd, short* job);
void party_booking_search(struct map_session_data *sd, short level, short mapid, short job, unsigned long lastindex, short resultcount);
bool party_booking_delete(struct map_session_data *sd);

#endif /* _PARTY_H_ */
