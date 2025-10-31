// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef PARTY_HPP
#define PARTY_HPP

#include <cstdarg>

#include <common/mmo.hpp> // struct party

struct block_list;
class map_session_data;
struct party;
struct item;

#define MAX_PARTY_BOOKING_JOBS 6
#define MAX_PARTY_BOOKING_RESULTS 10

struct party_member_data {
	map_session_data *sd;
	uint32 hp; //For HP,x,y refreshing.
	uint16 x, y;
};

struct party_data {
	struct party party;
	struct party_member_data data[MAX_PARTY];
	uint8 itemc; //For item distribution, position of last picker in party
	int32 instance_id;
	struct {
		unsigned monk : 1; //There's at least one monk in party?
		unsigned sg : 1;	//There's at least one Star Gladiator in party?
		unsigned snovice :1; //There's a Super Novice
		unsigned tk : 1; //There's a taekwon
	} state;
};

struct party_booking_detail {
	int16 level;
	int16 mapid;
	int16 job[MAX_PARTY_BOOKING_JOBS];
};

struct party_booking_ad_info {
	unsigned long index;
	char charname[NAME_LENGTH];
	long starttime;
	struct party_booking_detail p_detail;
};

struct s_party_booking_requirement{
	uint16 minimum_level;
	uint16 maximum_level;
};

extern int32 party_create_byscript;

void do_init_party(void);
void do_final_party(void);
struct party_data* party_search(int32 party_id);
struct party_data* party_searchname(const char* str);
int32 party_getmemberid(struct party_data* p, map_session_data* sd);
map_session_data* party_getavailablesd(struct party_data *p);

int32 party_create( map_session_data& sd, char *name, int32 item, int32 item2 );
void party_created(uint32 account_id,uint32 char_id,int32 fail,int32 party_id,char *name);
int32 party_request_info(int32 party_id, uint32 char_id);
bool party_invite( map_session_data& sd, map_session_data* tsd );
void party_member_joined( map_session_data& sd );
int32 party_member_added(int32 party_id,uint32 account_id,uint32 char_id,int32 flag);
bool party_leave( map_session_data& sd, bool showMessage = false );
bool party_removemember( map_session_data& sd, uint32 account_id, const char *name );
int32 party_removemember2(map_session_data *sd,uint32 char_id,int32 party_id);
int32 party_member_withdraw(int32 party_id, uint32 account_id, uint32 char_id, char *name, enum e_party_member_withdraw type);
bool party_isleader( map_session_data* sd );
void party_join( map_session_data& sd, int32 party_id );
bool party_booking_load( uint32 account_id, uint32 char_id, struct s_party_booking_requirement* booking );
bool party_reply_invite( map_session_data& sd, int32 party_id, int32 flag );
#define party_add_member(party_id,sd) party_reply_invite(sd,party_id,1)
int32 party_recv_noinfo(int32 party_id, uint32 char_id);
int32 party_recv_info(struct party* sp, uint32 char_id);
int32 party_recv_movemap( int32 party_id, uint32 account_id, uint32 char_id, int32 online, int32 lv, const char* map );
int32 party_broken(int32 party_id);
int32 party_optionchanged(int32 party_id,uint32 account_id,int32 exp,int32 item,int32 flag);
int32 party_changeoption(map_session_data *sd,int32 exp,int32 item);
int32 party_setoption(struct party_data *party, int32 option, int32 flag);
int32 party_changeleader(map_session_data *sd, map_session_data *t_sd, struct party_data *p);
void party_send_movemap(map_session_data *sd);
void party_send_levelup(map_session_data *sd);
int32 party_send_logout(map_session_data *sd);
int32 party_send_message(map_session_data *sd,const char *mes, size_t len);
int32 party_recv_message( int32 party_id, uint32 account_id, const char *mes, size_t len );
int32 party_skill_check(map_session_data *sd, int32 party_id, uint16 skill_id, uint16 skill_lv);
int32 party_send_xy_clear(struct party_data *p);
void party_exp_share(struct party_data *p,block_list *src,t_exp base_exp,t_exp job_exp,int32 zeny);
int32 party_share_loot(struct party_data* p, map_session_data* sd, struct item* item, int32 first_charid);
int32 party_send_dot_remove(map_session_data *sd);
int32 party_sub_count(block_list *bl, va_list ap);
int32 party_sub_count_class(block_list *bl, va_list ap);
int32 party_foreachsamemap(int32 (*func)(block_list *,va_list),map_session_data *sd,int32 range,...);

/*==========================================
 * Party Booking in KRO [Spiria]
 *------------------------------------------*/
void party_booking_register(map_session_data *sd, int16 level, int16 mapid, int16* job);
void party_booking_update(map_session_data *sd, int16* job);
void party_booking_search(map_session_data *sd, int16 level, int16 mapid, int16 job, unsigned long lastindex, int16 resultcount);
bool party_booking_delete(map_session_data *sd);

#endif /* PARTY_HPP */
