// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLIF_HPP
#define CLIF_HPP

#include "clif_interface.hpp"

class Clif : public Clif_Interface {
	public:
int setip(const char* ip);
void setbindip(const char* ip);
void setport(uint16 port);

uint32 getip(void);
uint32 refresh_ip(void);
uint16 getport(void);

void authok(struct map_session_data *sd);
void authrefuse(int fd, uint8 error_code);
void authfail_fd(int fd, int type);
void charselectok(int id, uint8 ok);
void dropflooritem(struct flooritem_data* fitem, bool canShowEffect);
void clearflooritem(struct flooritem_data *fitem, int fd);

void clearunit_single(int id, clr_type type, int fd);
void clearunit_area(struct block_list* bl, clr_type type);
void clearunit_delayed(struct block_list* bl, clr_type type, t_tick tick);
int spawn(struct block_list *bl);	//area
void walkok(struct map_session_data *sd);	// self
void move(struct unit_data *ud); //area
void changemap(struct map_session_data *sd, short m, int x, int y);	//self
void changemapserver(struct map_session_data* sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port);	//self
void blown(struct block_list *bl); // area
void slide(struct block_list *bl, int x, int y); // area
void fixpos(struct block_list *bl);	// area
void npcbuysell(struct map_session_data* sd, int id);	//self
void buylist(struct map_session_data *sd, struct npc_data *nd);	//self
void selllist(struct map_session_data *sd);	//self
void npc_market_open(struct map_session_data *sd, struct npc_data *nd);
void parse_NPCMarketClosed(int fd, struct map_session_data *sd);
void parse_NPCMarketPurchase(int fd, struct map_session_data *sd);
void scriptmes(struct map_session_data *sd, int npcid, const char *mes);	//self
void scriptnext(struct map_session_data *sd,int npcid);	//self
void scriptclose(struct map_session_data *sd, int npcid);	//self
void scriptclear(struct map_session_data *sd, int npcid);	//self
void scriptmenu(struct map_session_data* sd, int npcid, const char* mes);	//self
void scriptinput(struct map_session_data *sd, int npcid);	//self
void scriptinputstr(struct map_session_data *sd, int npcid);	// self
void cutin(struct map_session_data* sd, const char* image, int type);	//self
void viewpoint(struct map_session_data *sd, int npc_id, int type, int x, int y, int id, int color);	//self
void additem(struct map_session_data *sd, int n, int amount, unsigned char fail); // self
void dropitem(struct map_session_data *sd,int n,int amount);	//self
void delitem(struct map_session_data *sd,int n,int amount, short reason); //self
void updatestatus(struct map_session_data *sd,int type);	//self
void changestatus(struct map_session_data* sd,int type,int val);	//area
int damage(struct block_list* src, struct block_list* dst, t_tick tick, int sdelay, int ddelay, int64 sdamage, int div, enum e_damage_type type, int64 sdamage2, bool spdamage);	// area
void takeitem(struct block_list* src, struct block_list* dst);
void sitting(struct block_list* bl);
void standing(struct block_list* bl);
void sprite_change(struct block_list *bl, int id, int type, int val, int val2, enum send_target target);
void changelook(struct block_list *bl,int type,int val);	// area
void changetraplook(struct block_list *bl,int val); // area
void refreshlook(struct block_list *bl,int id,int type,int val,enum send_target target); //area specified in 'target'
void arrowequip(struct map_session_data *sd,int val); //self
void arrow_fail(struct map_session_data *sd,int type); //self
void arrow_create_list(struct map_session_data *sd);	//self
void statusupack(struct map_session_data *sd,int type,int ok,int val);	// self
void equipitemack(struct map_session_data *sd,int n,int pos,uint8 flag);	// self
void unequipitemack(struct map_session_data *sd,int n,int pos,int ok);	// self
void misceffect(struct block_list* bl,int type);	// area
void changeoption(struct block_list* bl);	// area
void changeoption2(struct block_list* bl);	// area
void useitemack(struct map_session_data *sd,int index,int amount,bool ok);	// self
void GlobalMessage(struct block_list* bl, const char* message,enum send_target target);
void createchat(struct map_session_data* sd, int flag);	// self
void dispchat(struct chat_data* cd, int fd);	// area or fd
void joinchatfail(struct map_session_data *sd,int flag);	// self
void joinchatok(struct map_session_data *sd,struct chat_data* cd);	// self
void addchat(struct chat_data* cd,struct map_session_data *sd);	// chat
void changechatowner(struct chat_data* cd, struct map_session_data* sd);	// chat
void clearchat(struct chat_data *cd,int fd);	// area or fd
void leavechat(struct chat_data* cd, struct map_session_data* sd, bool flag);	// chat
void changechatstatus(struct chat_data* cd);	// chat
void refresh_storagewindow(struct map_session_data *sd);
void refresh(struct map_session_data *sd);	// self

void emotion(struct block_list *bl,int type);
void talkiebox(struct block_list* bl, const char* talkie);
void wedding_effect(struct block_list *bl);
void divorced(struct map_session_data* sd, const char* name);
void callpartner(struct map_session_data *sd);
void playBGM(struct map_session_data* sd, const char* name);
void soundeffect(struct map_session_data* sd, struct block_list* bl, const char* name, int type);
void soundeffectall(struct block_list* bl, const char* name, int type, enum send_target coverage);
void parse_ActionRequest_sub(struct map_session_data *sd, int action_type, int target_id, t_tick tick);
void hotkeys_send(struct map_session_data *sd);

// trade
void traderequest(struct map_session_data* sd, const char* name);
void tradestart(struct map_session_data* sd, uint8 type);
void tradeadditem(struct map_session_data* sd, struct map_session_data* tsd, int index, int amount);
void tradeitemok(struct map_session_data* sd, int index, int fail);
void tradedeal_lock(struct map_session_data* sd, int fail);
void tradecancelled(struct map_session_data* sd);
void tradecompleted(struct map_session_data* sd, int fail);
void tradeundo(struct map_session_data* sd);

// storage
void storagelist(struct map_session_data* sd, struct item* items, int items_length, const char *storename);
void updatestorageamount(struct map_session_data* sd, int amount, int max_amount);
void storageitemadded(struct map_session_data* sd, struct item* i, int index, int amount);
void storageitemremoved(struct map_session_data* sd, int index, int amount);
void storageclose(struct map_session_data* sd);

void class_change_target(struct block_list *bl,int class_, int type, enum send_target target, struct map_session_data *sd);

void skillinfoblock(struct map_session_data *sd);
void skillup(struct map_session_data *sd, uint16 skill_id, int lv, int range, int upgradable);
void skillinfo(struct map_session_data *sd,int skill_id, int inf);
void addskill(struct map_session_data *sd, int skill_id);
void deleteskill(struct map_session_data *sd, int skill_id);

void skillcasting(struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, int property, int casttime);
void skillcastcancel(struct block_list* bl);
void skill_fail(struct map_session_data *sd,uint16 skill_id,enum useskill_fail_cause cause,int btype);
void skill_cooldown(struct map_session_data *sd, uint16 skill_id, t_tick tick);
int skill_damage(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int64 sdamage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type);
//int skill_damage2(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int damage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type);
bool skill_nodamage(struct block_list *src,struct block_list *dst,uint16 skill_id,int heal,t_tick tick);
void skill_poseffect(struct block_list *src,uint16 skill_id,int val,int x,int y,t_tick tick);
void skill_estimation(struct map_session_data *sd,struct block_list *dst);
void skill_warppoint(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, unsigned short map1, unsigned short map2, unsigned short map3, unsigned short map4);
void skill_memomessage(struct map_session_data* sd, int type);
void skill_teleportmessage(struct map_session_data *sd, int type);
void skill_produce_mix_list(struct map_session_data *sd, int skill_id, int trigger);
void cooking_list(struct map_session_data *sd, int trigger, uint16 skill_id, int qty, int list_type);

void produceeffect(struct map_session_data* sd,int flag, unsigned short nameid);

void getareachar_skillunit(struct block_list *bl, struct skill_unit *unit, enum send_target target, bool visible);
void skill_delunit(struct skill_unit *unit);
void skillunit_update(struct block_list* bl);

void autospell(struct map_session_data *sd,uint16 skill_lv);
void devotion(struct block_list *src, struct map_session_data *tsd);
void spiritball(struct block_list *bl);
void combo_delay(struct block_list *bl,t_tick wait);
void bladestop(struct block_list *src, int dst_id, int active);
void changemapcell(int fd, int16 m, int x, int y, int type, enum send_target target);

void status_change(struct block_list *bl, int type, int flag, t_tick tick, int val1, int val2, int val3);
void efst_status_change(struct block_list *bl, int tid, enum send_target target, int type, t_tick tick, int val1, int val2, int val3);
void efst_status_change_sub(struct block_list *tbl, struct block_list *bl, enum send_target target);

void wis_message(struct map_session_data* sd, const char* nick, const char* mes, int mes_len, int gmlvl);
void wis_end(int fd, int result);

void solved_charname(int fd, int charid, const char* name);
void name( struct block_list* src, struct block_list *bl, send_target target );

void use_card(struct map_session_data *sd,int idx);
void insert_card(struct map_session_data *sd,int idx_equip,int idx_card,int flag);

void inventorylist(struct map_session_data *sd);
void equiplist(struct map_session_data *sd);

void cart_additem(struct map_session_data *sd,int n,int amount,int fail);
void cart_additem_ack(struct map_session_data *sd, uint8 flag);
void cart_delitem(struct map_session_data *sd,int n,int amount);
void cartlist(struct map_session_data *sd);
void clearcart(int fd);

void item_identify_list(struct map_session_data *sd);
void item_identified(struct map_session_data *sd,int idx,int flag);
void item_repair_list(struct map_session_data *sd, struct map_session_data *dstsd, int lv);
void item_repaireffect(struct map_session_data *sd, int idx, int flag);
void item_damaged(struct map_session_data* sd, unsigned short position);
void item_refine_list(struct map_session_data *sd);
void hat_effects( struct map_session_data* sd, struct block_list* bl, enum send_target target );
void hat_effect_single( struct map_session_data* sd, uint16 effectId, bool enable );

void item_skill(struct map_session_data *sd,uint16 skill_id,uint16 skill_lv);

void mvp_effect(struct map_session_data *sd);
void mvp_item(struct map_session_data *sd, unsigned short nameid);
void mvp_exp(struct map_session_data *sd, unsigned int exp);
void mvp_noitem(struct map_session_data* sd);
void changed_dir(struct block_list *bl, enum send_target target);

// vending
void openvendingreq(struct map_session_data* sd, int num);
void showvendingboard(struct block_list* bl, const char* message, int fd);
void closevendingboard(struct block_list* bl, int fd);
void vendinglist(struct map_session_data* sd, int id, struct s_vending* vending);
void buyvending(struct map_session_data* sd, int index, int amount, int fail);
void openvending(struct map_session_data* sd, int id, struct s_vending* vending);
void vendingreport(struct map_session_data* sd, int index, int amount, uint32 char_id, int zeny);

void movetoattack(struct map_session_data *sd,struct block_list *bl);

// party
void party_created(struct map_session_data *sd,int result);
void party_member_info(struct party_data *p, struct map_session_data *sd);
void party_info(struct party_data* p, struct map_session_data *sd);
void party_invite(struct map_session_data *sd,struct map_session_data *tsd);
void party_invite_reply(struct map_session_data* sd, const char* nick, enum e_party_invite_reply reply);
void party_option(struct party_data *p,struct map_session_data *sd,int flag);
void party_withdraw(struct map_session_data *sd, uint32 account_id, const char* name, enum e_party_member_withdraw result, enum send_target target);
void party_message(struct party_data* p, uint32 account_id, const char* mes, int len);
void party_xy(struct map_session_data *sd);
void party_xy_single(int fd, struct map_session_data *sd);
void party_hp(struct map_session_data *sd);
void hpmeter_single(int fd, int id, unsigned int hp, unsigned int maxhp);
void party_job_and_level(struct map_session_data *sd);
void party_dead( struct map_session_data *sd );

// guild
void guild_created(struct map_session_data *sd,int flag);
void guild_belonginfo(struct map_session_data *sd);
void guild_masterormember(struct map_session_data *sd);
void guild_basicinfo(struct map_session_data *sd);
void guild_allianceinfo(struct map_session_data *sd);
void guild_memberlist(struct map_session_data *sd);
void guild_skillinfo(struct map_session_data* sd);
void guild_send_onlineinfo(struct map_session_data *sd); //[LuzZza]
void guild_memberlogin_notice(struct guild *g,int idx,int flag);
void guild_invite(struct map_session_data *sd,struct guild *g);
void guild_inviteack(struct map_session_data *sd,int flag);
void guild_leave(struct map_session_data *sd,const char *name,const char *mes);
void guild_expulsion(struct map_session_data* sd, const char* name, const char* mes, uint32 account_id);
void guild_positionchanged(struct guild *g,int idx);
void guild_memberpositionchanged(struct guild *g,int idx);
void guild_emblem(struct map_session_data *sd,struct guild *g);
void guild_emblem_area(struct block_list* bl);
void guild_notice(struct map_session_data* sd);
void guild_message(struct guild *g,uint32 account_id,const char *mes,int len);
void guild_reqalliance(struct map_session_data *sd,uint32 account_id,const char *name);
void guild_allianceack(struct map_session_data *sd,int flag);
void guild_delalliance(struct map_session_data *sd,int guild_id,int flag);
void guild_oppositionack(struct map_session_data *sd,int flag);
void guild_broken(struct map_session_data *sd,int flag);
void guild_xy(struct map_session_data *sd);
void guild_xy_single(int fd, struct map_session_data *sd);
void guild_xy_remove(struct map_session_data *sd);

// Battleground
void bg_hp(struct map_session_data *sd);
void bg_xy(struct map_session_data *sd);
void bg_xy_remove(struct map_session_data *sd);
void bg_message(struct battleground_data *bg, int src_id, const char *name, const char *mes, int len);
void bg_updatescore(int16 m);
void bg_updatescore_single(struct map_session_data *sd);
void sendbgemblem_area(struct map_session_data *sd);
void sendbgemblem_single(int fd, struct map_session_data *sd);

// Instancing
void instance_create(unsigned short instance_id, int num);
void instance_changewait(unsigned short instance_id, int num);
void instance_status(unsigned short instance_id, unsigned int limit1, unsigned int limit2);
void instance_changestatus(unsigned int instance_id, int type, unsigned int limit);

// Custom Fonts
void font(struct map_session_data *sd);

// atcommand
void displaymessage(const int fd, const char* mes);
void disp_message(struct block_list* src, const char* mes, int len, enum send_target target);
void broadcast(struct block_list* bl, const char* mes, int len, int type, enum send_target target);
void broadcast2(struct block_list* bl, const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target);
void heal(int fd,int type,int val);
void resurrection(struct block_list *bl,int type);
void map_property(struct block_list *bl, enum map_property property, enum send_target t);
void pvpset(struct map_session_data *sd, int pvprank, int pvpnum,int type);
void map_property_mapall(int map, enum map_property property);
void refine(int fd, int fail, int index, int val);
void upgrademessage(int fd, int result, unsigned short item_id);

//petsystem
void catch_process(struct map_session_data *sd);
void pet_roulette(struct map_session_data *sd,int data);
void sendegg(struct map_session_data *sd);
void send_petstatus(struct map_session_data *sd);
void pet_emotion(struct pet_data *pd,int param);
void pet_food(struct map_session_data *sd,int foodid,int fail);

//friends list
void friendslist_send(struct map_session_data *sd);
void friendslist_reqack(struct map_session_data *sd, struct map_session_data *f_sd, int type);

void weather(int16 m); // [Valaris]
void specialeffect(struct block_list* bl, int type, enum send_target target); // special effects [Valaris]
void specialeffect_single(struct block_list* bl, int type, int fd);
void messagecolor_target(struct block_list *bl, unsigned long color, const char *msg, bool rgb2bgr, enum send_target type, struct map_session_data *sd);
void specialeffect_value(struct block_list* bl, int effect_id, int num, send_target target);

void GM_kickack(struct map_session_data *sd, int id);
void GM_kick(struct map_session_data *sd,struct map_session_data *tsd);
void manner_message(struct map_session_data* sd, uint32 type);
void GM_silence(struct map_session_data* sd, struct map_session_data* tsd, uint8 type);

void disp_overhead_(struct block_list *bl, const char* mes, enum send_target flag);

void get_weapon_view(struct map_session_data* sd, unsigned short *rhand, unsigned short *lhand);

void party_xy_remove(struct map_session_data *sd); //Fix for minimap [Kevin]
void gospel_info(struct map_session_data *sd, int type);
void feel_req(int fd, struct map_session_data *sd, uint16 skill_lv);
void starskill(struct map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result);
void feel_info(struct map_session_data* sd, unsigned char feel_level, unsigned char type);
void hate_info(struct map_session_data *sd, unsigned char hate_level,int class_, unsigned char type);
void mission_info(struct map_session_data *sd, int mob_id, unsigned char progress);
void feel_hate_reset(struct map_session_data *sd);

// [blackhole89]
void hominfo(struct map_session_data *sd, struct homun_data *hd, int flag);
int homskillinfoblock(struct map_session_data *sd);
void homskillup(struct map_session_data *sd, uint16 skill_id);	//[orn]
int hom_food(struct map_session_data *sd,int foodid,int fail);	//[orn]
void send_homdata(struct map_session_data *sd, int state, int param);	//[orn]

void configuration( struct map_session_data* sd, enum e_config_type type, bool enabled );
void partytickack(struct map_session_data* sd, bool flag);
void viewequip_ack(struct map_session_data* sd, struct map_session_data* tsd);
void equipcheckbox(struct map_session_data* sd);

void msg(struct map_session_data* sd, unsigned short id);
void msg_value(struct map_session_data* sd, unsigned short id, int value);
void msg_skill(struct map_session_data* sd, uint16 skill_id, int msg_id);

//quest system [Kevin] [Inkfish]
void quest_send_list(struct map_session_data * sd);
void quest_send_mission(struct map_session_data * sd);
void quest_add(struct map_session_data * sd, struct quest * qd);
void quest_delete(struct map_session_data * sd, int quest_id);
void quest_update_status(struct map_session_data * sd, int quest_id, bool active);
void quest_update_objective(struct map_session_data * sd, struct quest * qd, int mobid);
void quest_show_event(struct map_session_data *sd, struct block_list *bl, short state, short color);
void displayexp(struct map_session_data *sd, unsigned int exp, char type, bool quest, bool lost);

int send(const uint8* buf, int len, struct block_list* bl, enum send_target type);
void do_init_clif(void);
void do_final_clif(void);

void Mail_window(int fd, int flag);
void Mail_read(struct map_session_data *sd, int mail_id);
void mail_delete(struct map_session_data* sd, struct mail_message *msg, bool success);
void Mail_return(int fd, int mail_id, short fail);
void Mail_send(struct map_session_data* sd, enum mail_send_result result);
void Mail_new(struct map_session_data* sd, int mail_id, const char *sender, const char *title);
void Mail_refreshinbox(struct map_session_data *sd,enum mail_inbox_type type,int64 mailID);
void mail_getattachment(struct map_session_data* sd, struct mail_message *msg, uint8 result, enum mail_attachment_type type);
void Mail_Receiver_Ack(struct map_session_data* sd, uint32 char_id, short class_, uint32 level, const char* name);
void mail_removeitem(struct map_session_data* sd, bool success, int index, int amount);
// AUCTION SYSTEM
void Auction_openwindow(struct map_session_data *sd);
void Auction_results(struct map_session_data *sd, short count, short pages, uint8 *buf);
void Auction_message(int fd, unsigned char flag);
void Auction_close(int fd, unsigned char flag);

void bossmapinfo(struct map_session_data *sd, struct mob_data *md, enum e_bossmap_info flag);
void cashshop_show(struct map_session_data *sd, struct npc_data *nd);

// ADOPTION
void Adopt_reply(struct map_session_data *sd, int type);
void Adopt_request(struct map_session_data *sd, struct map_session_data *src, int p_id);

// MERCENARIES
void mercenary_info(struct map_session_data *sd);
void mercenary_skillblock(struct map_session_data *sd);
void mercenary_message(struct map_session_data* sd, int message);
void mercenary_updatestatus(struct map_session_data *sd, int type);

// RENTAL SYSTEM
void rental_time(int fd, unsigned short nameid, int seconds);
void rental_expired(int fd, int index, unsigned short nameid);

// BOOK READING
void readbook(int fd, int book_id, int page);

// Show Picker
void party_show_picker(struct map_session_data * sd, struct item * item_data);

// Progress Bar [Inkfish]
void progressbar(struct map_session_data * sd, unsigned long color, unsigned int second);
void progressbar_abort(struct map_session_data * sd);
void progressbar_npc(struct npc_data *nd, struct map_session_data* sd);

void PartyBookingRegisterAck(struct map_session_data *sd, int flag);
void PartyBookingDeleteAck(struct map_session_data* sd, int flag);
void PartyBookingSearchAck(int fd, struct party_booking_ad_info** results, int count, bool more_result);
void PartyBookingUpdateNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad);
void PartyBookingDeleteNotify(struct map_session_data* sd, int index);
void PartyBookingInsertNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad);

/* Bank System [Yommy/Hercules] */
void bank_deposit (struct map_session_data *sd, enum e_BANKING_DEPOSIT_ACK reason);
void bank_withdraw (struct map_session_data *sd,enum e_BANKING_WITHDRAW_ACK reason);
void parse_BankDeposit (int fd, struct map_session_data *sd);
void parse_BankWithdraw (int fd, struct map_session_data *sd);
void parse_BankCheck (int fd, struct map_session_data *sd);
void parse_BankOpen (int fd, struct map_session_data *sd);
void parse_BankClose (int fd, struct map_session_data *sd);

void showdigit(struct map_session_data* sd, unsigned char type, int value);

/// Buying Store System
void buyingstore_open(struct map_session_data* sd);
void buyingstore_open_failed(struct map_session_data* sd, unsigned short result, unsigned int weight);
void buyingstore_myitemlist(struct map_session_data* sd);
void buyingstore_entry(struct map_session_data* sd);
void buyingstore_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd);
void buyingstore_disappear_entry(struct map_session_data* sd);
void buyingstore_disappear_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd);
void buyingstore_itemlist(struct map_session_data* sd, struct map_session_data* pl_sd);
void buyingstore_trade_failed_buyer(struct map_session_data* sd, short result);
void buyingstore_update_item(struct map_session_data* sd, unsigned short nameid, unsigned short amount, uint32 char_id, int zeny);
void buyingstore_delete_item(struct map_session_data* sd, short index, unsigned short amount, int price);
void buyingstore_trade_failed_seller(struct map_session_data* sd, short result, unsigned short nameid);

/// Search Store System
void search_store_info_ack(struct map_session_data* sd);
void search_store_info_failed(struct map_session_data* sd, unsigned char reason);
void open_search_store_info(struct map_session_data* sd);
void search_store_info_click_ack(struct map_session_data* sd, short x, short y);

/// Cash Shop
void cashshop_result( struct map_session_data* sd, unsigned short item_id, uint16 result );
void cashshop_open( struct map_session_data* sd );
#ifdef VIP_ENABLE
void display_pinfo(struct map_session_data *sd, int type);
#endif
/// Roulette
void roulette_open(struct map_session_data* sd);

int elementalconverter_list(struct map_session_data *sd);

void millenniumshield(struct block_list *bl, short shields);

int spellbook_list(struct map_session_data *sd);

int magicdecoy_list(struct map_session_data *sd, uint16 skill_lv, short x, short y);

int poison_list(struct map_session_data *sd, uint16 skill_lv);

int autoshadowspell_list(struct map_session_data *sd);

int skill_itemlistwindow( struct map_session_data *sd, uint16 skill_id, uint16 skill_lv );
void elemental_info(struct map_session_data *sd);
void elemental_updatestatus(struct map_session_data *sd, int type);

void spiritcharm(struct map_session_data *sd);

void snap( struct block_list *bl, short x, short y );
void monster_hp_bar( struct mob_data* md, int fd );

// Clan System
void clan_basicinfo( struct map_session_data *sd );
void clan_message(struct clan *clan,const char *mes,int len);
void clan_onlinecount( struct clan* clan );
void clan_leave( struct map_session_data* sd );

// Bargain Tool
void sale_start(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void sale_end(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void sale_amount(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target);
void sale_open(struct map_session_data* sd);

void channel_msg(struct Channel *channel, const char *msg, unsigned long color);


void ranklist(struct map_session_data *sd, int16 rankingType);
void update_rankingpoint(struct map_session_data *sd, int rankingtype, int point);

void crimson_marker(struct map_session_data *sd, struct block_list *bl, bool remove);

void showscript(struct block_list* bl, const char* message, enum send_target flag);
void party_leaderchanged(struct map_session_data *sd, int prev_leader_aid, int new_leader_aid);

void account_name(int fd, uint32 account_id, const char* accname);
void notify_bindOnEquip(struct map_session_data *sd, int n);

void merge_item_open(struct map_session_data *sd);

void broadcast_obtain_special_item(const char *char_name, unsigned short nameid, unsigned short container, enum BROADCASTING_SPECIAL_ITEM_OBTAIN type);

void dressing_room(struct map_session_data *sd, int flag);
void navigateTo(struct map_session_data *sd, const char* mapname, uint16 x, uint16 y, uint8 flag, bool hideWindow, uint16 mob_id );
void SelectCart(struct map_session_data *sd);

/// Achievement System
void achievement_list_all(struct map_session_data *sd);
void achievement_update(struct map_session_data *sd, struct achievement *ach, int count);
void achievement_reward_ack(int fd, unsigned char result, int ach_id);

void ui_open( struct map_session_data *sd, enum out_ui_type ui_type, int32 data );
void attendence_response( struct map_session_data *sd, int32 data );

void weight_limit( struct map_session_data* sd );

void ccamerainfo( struct map_session_data* sd, bool show, float range = 0.0f, float rotation = 0.0f, float latitude = 0.0f );

/// Equip Switch System
void equipswitch_list( struct map_session_data* sd );
void equipswitch_add( struct map_session_data* sd,uint16 index, uint32 pos, bool failed );
void equipswitch_remove( struct map_session_data* sd, uint16 index, uint32 pos, bool failed );
void equipswitch_reply( struct map_session_data* sd, bool failed );

};
#endif /* CLIF_HPP */
