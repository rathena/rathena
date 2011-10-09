// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INTIF_H_
#define _INFIF_H_

//#include "../common/mmo.h"
struct party_member;
struct guild_member;
struct guild_position;
struct s_pet;
struct s_homunculus;
struct s_mercenary;
struct mail_message;
struct auction_data;

int intif_parse(int fd);

int intif_broadcast(const char* mes, int len, int type);
int intif_broadcast2(const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY);

int intif_wis_message(struct map_session_data *sd,char *nick,char *mes,int mes_len);
int intif_wis_message_to_gm(char *Wisp_name, int min_gm_level, char *mes);

int intif_saveregistry(struct map_session_data *sd, int type);
int intif_request_registry(struct map_session_data *sd, int flag);

int intif_request_guild_storage(int account_id, int guild_id);
int intif_send_guild_storage(int account_id, struct guild_storage *gstor);


int intif_create_party(struct party_member *member,char *name,int item,int item2);
int intif_request_partyinfo(int party_id, int char_id);

int intif_party_addmember(int party_id,struct party_member *member);
int intif_party_changeoption(int party_id, int account_id, int exp, int item);
int intif_party_leave(int party_id,int account_id, int char_id);
int intif_party_changemap(struct map_session_data *sd, int online);
int intif_break_party(int party_id);
int intif_party_message(int party_id, int account_id, const char *mes,int len);
int intif_party_leaderchange(int party_id,int account_id,int char_id);


int intif_guild_create(const char *name, const struct guild_member *master);
int intif_guild_request_info(int guild_id);
int intif_guild_addmember(int guild_id, struct guild_member *m);
int intif_guild_leave(int guild_id, int account_id, int char_id, int flag, const char *mes);
int intif_guild_memberinfoshort(int guild_id, int account_id, int char_id, int online, int lv, int class_);
int intif_guild_break(int guild_id);
int intif_guild_message(int guild_id, int account_id, const char *mes, int len);
int intif_guild_change_gm(int guild_id, const char* name, int len);
int intif_guild_change_basicinfo(int guild_id, int type, const void *data, int len);
int intif_guild_change_memberinfo(int guild_id, int account_id, int char_id, int type, const void *data, int len);
int intif_guild_position(int guild_id, int idx, struct guild_position *p);
int intif_guild_skillup(int guild_id, int skill_num, int account_id, int max);
int intif_guild_alliance(int guild_id1, int guild_id2, int account_id1, int account_id2, int flag);
int intif_guild_notice(int guild_id, const char *mes1, const char *mes2);
int intif_guild_emblem(int guild_id, int len, const char *data);
int intif_guild_castle_dataload(int castle_id, int index);
int intif_guild_castle_datasave(int castle_id, int index, int value);

int intif_create_pet(int account_id, int char_id, short pet_type, short pet_lv, short pet_egg_id,
                     short pet_equip, short intimate, short hungry, char rename_flag, char incuvate, char *pet_name);
int intif_request_petdata(int account_id, int char_id, int pet_id);
int intif_save_petdata(int account_id, struct s_pet *p);
int intif_delete_petdata(int pet_id);
int intif_rename(struct map_session_data *sd, int type, char *name);
#define intif_rename_pc(sd, name) intif_rename(sd, 0, name)
#define intif_rename_pet(sd, name) intif_rename(sd, 1, name)
#define intif_rename_hom(sd, name) intif_rename(sd, 2, name)
int intif_homunculus_create(int account_id, struct s_homunculus *sh);
int intif_homunculus_requestload(int account_id, int homun_id);
int intif_homunculus_requestsave(int account_id, struct s_homunculus* sh);
int intif_homunculus_requestdelete(int homun_id);

/******QUEST SYTEM*******/
int intif_request_questlog(struct map_session_data * sd);
int intif_quest_save(struct map_session_data * sd);

// MERCENARY SYSTEM
int intif_mercenary_create(struct s_mercenary *merc);
int intif_mercenary_request(int merc_id, int char_id);
int intif_mercenary_delete(int merc_id);
int intif_mercenary_save(struct s_mercenary *merc);

#ifndef TXT_ONLY
// MAIL SYSTEM
int intif_Mail_requestinbox(int char_id, unsigned char flag);
int intif_Mail_read(int mail_id);
int intif_Mail_getattach(int char_id, int mail_id);
int intif_Mail_delete(int char_id, int mail_id);
int intif_Mail_return(int char_id, int mail_id);
int intif_Mail_send(int account_id, struct mail_message *msg);
// AUCTION SYSTEM
int intif_Auction_requestlist(int char_id, short type, int price, const char* searchtext, short page);
int intif_Auction_register(struct auction_data *auction);
int intif_Auction_cancel(int char_id, unsigned int auction_id);
int intif_Auction_close(int char_id, unsigned int auction_id);
int intif_Auction_bid(int char_id, const char* name, unsigned int auction_id, int bid);
#endif

int CheckForCharServer(void);

#endif /* _INTIF_H_ */
