// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INTIF_HPP
#define INTIF_HPP

#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

struct party_member;
struct guild_member;
struct guild_position;
struct s_pet;
struct s_homunculus;
struct s_mercenary;
struct s_elemental;
struct mail_message;
struct auction_data;
struct s_achievement_db;
class map_session_data;

int32 intif_parse(int32 fd);

int32 intif_broadcast( const char* mes, size_t len, int32 type );
int32 intif_broadcast2( const char* mes, size_t len, unsigned long fontColor, int16 fontType, int16 fontSize, int16 fontAlign, int16 fontY );
int32 intif_broadcast_obtain_special_item(map_session_data *sd, t_itemid nameid, uint32 sourceid, unsigned char type);
int32 intif_broadcast_obtain_special_item_npc(map_session_data *sd, t_itemid nameid);
int32 intif_main_message(map_session_data* sd, const char* message);

int32 intif_wis_message(map_session_data *sd, char *nick, char *mes, size_t mes_len);
int32 intif_wis_message_to_gm(char *Wisp_name, int32 permission, char *mes);

int32 intif_saveregistry(map_session_data *sd);
int32 intif_request_registry(map_session_data *sd, int32 flag);

bool intif_request_guild_storage(uint32 account_id, int32 guild_id);
bool intif_send_guild_storage(uint32 account_id, struct s_storage *gstor);

int32 intif_create_party(struct party_member *member,char *name,int32 item,int32 item2);
int32 intif_request_partyinfo(int32 party_id, uint32 char_id);

int32 intif_party_addmember(int32 party_id,struct party_member *member);
int32 intif_party_changeoption(int32 party_id, uint32 account_id, int32 exp, int32 item);
int32 intif_party_leave(int32 party_id, uint32 account_id, uint32 char_id, const char *name, enum e_party_member_withdraw type);
int32 intif_party_changemap(map_session_data *sd, int32 online);
int32 intif_break_party(int32 party_id);
int32 intif_party_message(int32 party_id, uint32 account_id, const char *mes, size_t len);
int32 intif_party_leaderchange(int32 party_id,uint32 account_id,uint32 char_id);
int32 intif_party_sharelvlupdate(uint32 share_lvl);

int32 intif_guild_create(const char *name, const struct guild_member *master);
int32 intif_guild_request_info(int32 guild_id);
int32 intif_guild_addmember( int32 guild_id, struct guild_member& m );
bool intif_guild_leave(int32 guild_id, uint32 account_id, uint32 char_id, int32 flag, const char *mes);
int32 intif_guild_memberinfoshort(int32 guild_id, uint32 account_id, uint32 char_id, int32 online, int32 lv, int32 class_);
int32 intif_guild_break(int32 guild_id);
int32 intif_guild_message(int32 guild_id, uint32 account_id, const char *mes, size_t len);
bool intif_guild_change_gm( int32 guild_id, const char* name, size_t len );
int32 intif_guild_change_basicinfo(int32 guild_id, int32 type, const void *data, int32 len);
int32 intif_guild_change_memberinfo(int32 guild_id, uint32 account_id, uint32 char_id, int32 type, const void *data, int32 len);
int32 intif_guild_position(int32 guild_id, int32 idx, struct guild_position *p);
int32 intif_guild_skillup(int32 guild_id, uint16 skill_id, uint32 account_id, int32 max);
int32 intif_guild_alliance(int32 guild_id1, int32 guild_id2, uint32 account_id1, uint32 account_id2, int32 flag);
int32 intif_guild_notice(int32 guild_id, const char *mes1, const char *mes2);
int32 intif_guild_emblem(int32 guild_id, int32 len, const char *data);
int32 intif_guild_emblem_version(int32 guild_id, int32 version);
bool intif_guild_castle_dataload( const std::vector<int32>& castle_ids );
int32 intif_guild_castle_datasave(int32 castle_id, int32 index, int32 value);
#ifdef BOUND_ITEMS
void intif_itembound_guild_retrieve(uint32 char_id, uint32 account_id, int32 guild_id);
#endif

int32 intif_create_pet(uint32 account_id, uint32 char_id, int16 pet_type, int16 pet_lv, t_itemid pet_egg_id, t_itemid pet_equip, int16 intimate, int16 hungry, char rename_flag, char incubate, const char *pet_name);
int32 intif_request_petdata(uint32 account_id, uint32 char_id, int32 pet_id);
int32 intif_save_petdata(uint32 account_id, struct s_pet *p);
int32 intif_delete_petdata(int32 pet_id);
int32 intif_rename(map_session_data *sd, int32 type, char *name);
#define intif_rename_pc(sd, name) intif_rename(sd, 0, name)
#define intif_rename_pet(sd, name) intif_rename(sd, 1, name)
#define intif_rename_hom(sd, name) intif_rename(sd, 2, name)
int32 intif_homunculus_create(uint32 account_id, struct s_homunculus *sh);
int32 intif_homunculus_requestload(uint32 account_id, int32 homun_id);
int32 intif_homunculus_requestsave(uint32 account_id, struct s_homunculus* sh);
int32 intif_homunculus_requestdelete(int32 homun_id);

/******QUEST SYTEM*******/
void intif_request_questlog(map_session_data * sd);
int32 intif_quest_save(map_session_data * sd);

// MERCENARY SYSTEM
int32 intif_mercenary_create(struct s_mercenary *merc);
int32 intif_mercenary_request(int32 merc_id, uint32 char_id);
int32 intif_mercenary_delete(int32 merc_id);
int32 intif_mercenary_save(struct s_mercenary *merc);

// MAIL SYSTEM
int32 intif_Mail_requestinbox(uint32 char_id, unsigned char flag, enum mail_inbox_type type);
int32 intif_Mail_read(int32 mail_id);
bool intif_mail_getattach( map_session_data* sd, struct mail_message *msg, enum mail_attachment_type type );
int32 intif_Mail_delete(uint32 char_id, int32 mail_id);
int32 intif_Mail_return(uint32 char_id, int32 mail_id);
int32 intif_Mail_send(uint32 account_id, struct mail_message *msg);
bool intif_mail_checkreceiver(map_session_data* sd, char* name);
// AUCTION SYSTEM
int32 intif_Auction_requestlist(uint32 char_id, int16 type, int32 price, const char* searchtext, int16 page);
int32 intif_Auction_register(struct auction_data *auction);
int32 intif_Auction_cancel(uint32 char_id, uint32 auction_id);
int32 intif_Auction_close(uint32 char_id, uint32 auction_id);
int32 intif_Auction_bid(uint32 char_id, const char* name, uint32 auction_id, int32 bid);
// ELEMENTAL SYSTEM
int32 intif_elemental_create(struct s_elemental *ele);
int32 intif_elemental_request(int32 ele_id, uint32 char_id);
int32 intif_elemental_delete(int32 ele_id);
int32 intif_elemental_save(struct s_elemental *ele);
// CLAN SYSTEM
int32 intif_clan_requestclans();
int32 intif_clan_message( int32 clan_id, uint32 account_id, const char *mes, size_t len );
int32 intif_clan_member_joined( int32 clan_id );
int32 intif_clan_member_left( int32 clan_id );
// ACHIEVEMENT SYSTEM
void intif_request_achievements(uint32 char_id);
int32 intif_achievement_save(map_session_data *sd);
int32 intif_achievement_reward(map_session_data *sd, struct s_achievement_db *adb);

int32 intif_request_accinfo( int32 u_fd, int32 aid, int32 group_lv, char* query );

// STORAGE
bool intif_storage_request(map_session_data *sd, enum storage_type type, uint8 stor_id, uint8 mode);
bool intif_storage_save(map_session_data *sd, struct s_storage *stor);

int32 CheckForCharServer(void);

#endif /* INTIF_HPP */
