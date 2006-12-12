// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CLIF_H_
#define _CLIF_H_

#include "map.h"

// protocol version
#define PACKETVER			7

// packet DB
#define MAX_PACKET_DB		0x25f
#define MAX_PACKET_VER		25

struct packet_db {
	short len;
	void (*func)(int, struct map_session_data *);
	short pos[20];
};

// local define
enum {
	ALL_CLIENT,
	ALL_SAMEMAP,
	AREA,
	AREA_WOS,
	AREA_WOC,
	AREA_WOSC,
	AREA_CHAT_WOC,
	CHAT,
	CHAT_WOS,
	CHAT_MAINCHAT,
	PARTY,
	PARTY_WOS,
	PARTY_SAMEMAP,
	PARTY_SAMEMAP_WOS,
	PARTY_AREA,
	PARTY_AREA_WOS,
	GUILD,
	GUILD_WOS,
	GUILD_SAMEMAP,	// [Valaris]
	GUILD_SAMEMAP_WOS,
	GUILD_AREA,
	GUILD_AREA_WOS,	// end additions [Valaris]
	SELF,
	DUEL,
	DUEL_WOS
};

extern struct packet_db packet_db[MAX_PACKET_VER + 1][MAX_PACKET_DB];

int clif_setip(char*);
void clif_setbindip(char*);
void clif_setport(int);

unsigned long clif_getip_long(void);
unsigned long clif_refresh_ip(void);
int clif_getport(void);
int clif_countusers(void);
void clif_setwaitclose(int);

int clif_authok(struct map_session_data *);
int clif_authfail_fd(int,int);
void clif_updatemaxid(int, int);
int clif_charselectok(int);
void check_fake_id(int fd, struct map_session_data *sd, int target_id);
int clif_dropflooritem(struct flooritem_data *);
int clif_clearflooritem(struct flooritem_data *,int);
int clif_clearchar(struct block_list*,int);	// area or fd
int clif_clearchar_delay(unsigned int,struct block_list *,int);
#define clif_clearchar_area(bl,type) clif_clearchar(bl,type)
int clif_clearchar_id(int,int,int);
int clif_spawn(struct block_list*);	//area
int clif_walkok(struct map_session_data*);	// self
int clif_move(struct block_list*);	// area
int clif_changemap(struct map_session_data*,short,int,int);	//self
int clif_changemapserver(struct map_session_data*,char*,int,int,int,int);	//self
int clif_blown(struct block_list *); // area
int clif_slide(struct block_list *,int,int); // area
int clif_fixpos(struct block_list *);	// area
int clif_fixpos2(struct block_list *);	// area
int clif_npcbuysell(struct map_session_data*,int);	//self
int clif_buylist(struct map_session_data*,struct npc_data*);	//self
int clif_selllist(struct map_session_data*);	//self
int clif_scriptmes(struct map_session_data*,int,char*);	//self
int clif_scriptnext(struct map_session_data*,int);	//self
int clif_scriptclose(struct map_session_data*,int);	//self
int clif_scriptmenu(struct map_session_data*,int,char*);	//self
int clif_scriptinput(struct map_session_data*,int);	//self
int clif_scriptinputstr(struct map_session_data *sd,int npcid);	// self
int clif_cutin(struct map_session_data*,char*,int);	//self
int clif_viewpoint(struct map_session_data*,int,int,int,int,int,int);	//self
int clif_additem(struct map_session_data*,int,int,int);	//self
int clif_delitem(struct map_session_data*,int,int);	//self
int clif_updatestatus(struct map_session_data*,int);	//self
int clif_changestatus(struct block_list*,int,int);	//area
int clif_damage(struct block_list *,struct block_list *,unsigned int,int,int,int,int,int,int);	// area
#define clif_takeitem(src,dst) clif_damage(src,dst,0,0,0,0,0,1,0)
int clif_changelook(struct block_list *,int,int);	// area
void clif_changetraplook(struct block_list *bl,int val); // area
void clif_refreshlook(struct block_list *bl,int id,int type,int val,int area); //area specified in 'area'
int clif_arrowequip(struct map_session_data *sd,int val); //self
int clif_arrow_fail(struct map_session_data *sd,int type); //self
int clif_arrow_create_list(struct map_session_data *sd);	//self
int clif_statusupack(struct map_session_data *,int,int,int);	// self
int clif_equipitemack(struct map_session_data *,int,int,int);	// self
int clif_unequipitemack(struct map_session_data *,int,int,int);	// self
int clif_misceffect(struct block_list*,int);	// area
int clif_misceffect2(struct block_list *bl,int type);
int clif_changeoption(struct block_list*);	// area
int clif_useitemack(struct map_session_data*,int,int,int);	// self
void clif_GlobalMessage(struct block_list *bl,char *message);
int clif_createchat(struct map_session_data*,int);	// self
int clif_dispchat(struct chat_data*,int);	// area or fd
int clif_joinchatfail(struct map_session_data*,int);	// self
int clif_joinchatok(struct map_session_data*,struct chat_data*);	// self
int clif_addchat(struct chat_data*,struct map_session_data*);	// chat
int clif_changechatowner(struct chat_data*,struct map_session_data*);	// chat
int clif_clearchat(struct chat_data*,int);	// area or fd
int clif_leavechat(struct chat_data*,struct map_session_data*);	// chat
int clif_changechatstatus(struct chat_data*);	// chat
int clif_refresh(struct map_session_data*);	// self

int clif_fame_blacksmith(struct map_session_data *, int);
int clif_fame_alchemist(struct map_session_data *, int);
int clif_fame_taekwon(struct map_session_data *, int);

void clif_emotion(struct block_list *bl,int type);
void clif_talkiebox(struct block_list *bl,char* talkie);
void clif_wedding_effect(struct block_list *bl);
void clif_divorced(struct map_session_data *sd, char *);
//void clif_sitting(int fd, struct map_session_data *sd);
//void clif_callpartner(struct map_session_data *sd);
void clif_adopt_process(struct map_session_data *sd);
void clif_sitting(struct map_session_data *sd);
void clif_soundeffect(struct map_session_data *sd,struct block_list *bl,char *name,int type);
int clif_soundeffectall(struct block_list *bl, char *name, int type, int coverage);
void clif_parse_ActionRequest_sub(struct map_session_data *sd, int action_type, int target_id, unsigned int tick);
void clif_parse_LoadEndAck(int fd,struct map_session_data *sd);

// trade
int clif_traderequest(struct map_session_data *sd,char *name);
int clif_tradestart(struct map_session_data *sd,int type);
int clif_tradeadditem(struct map_session_data *sd,struct map_session_data *tsd,int index,int amount);
int clif_tradeitemok(struct map_session_data *sd,int index,int fail);
int clif_tradedeal_lock(struct map_session_data *sd,int fail);
int clif_tradecancelled(struct map_session_data *sd);
int clif_tradecompleted(struct map_session_data *sd,int fail);

// storage
#include "storage.h"
void clif_storagelist(struct map_session_data *sd,struct storage *stor);
int clif_updatestorageamount(struct map_session_data *sd,struct storage *stor);
int clif_storageitemadded(struct map_session_data *sd,struct storage *stor,int index,int amount);
int clif_storageitemremoved(struct map_session_data *sd,int index,int amount);
int clif_storageclose(struct map_session_data *sd);
void clif_guildstoragelist(struct map_session_data *sd,struct guild_storage *stor);
int clif_updateguildstorageamount(struct map_session_data *sd,struct guild_storage *stor);
int clif_guildstorageitemadded(struct map_session_data *sd,struct guild_storage *stor,int index,int amount);

int clif_insight(struct block_list *,va_list);	// map_forallinmovearea callback
int clif_outsight(struct block_list *,va_list);	// map_forallinmovearea callback

int clif_class_change(struct block_list *bl,int class_,int type);
#define clif_mob_class_change(md, class_) clif_class_change(&md->bl, class_, 1)
int clif_mob_equip(struct mob_data *md,int nameid); // [Valaris]

int clif_skillinfo(struct map_session_data *sd,int skillid,int type,int range);
int clif_skillinfoblock(struct map_session_data *sd);
int clif_skillup(struct map_session_data *sd,int skill_num);

int clif_skillcasting(struct block_list* bl,
	int src_id,int dst_id,int dst_x,int dst_y,int skill_num,int casttime);
int clif_skillcastcancel(struct block_list* bl);
int clif_skill_fail(struct map_session_data *sd,int skill_id,int type,int btype);
int clif_skill_damage(struct block_list *src,struct block_list *dst,
	unsigned int tick,int sdelay,int ddelay,int damage,int div,
	int skill_id,int skill_lv,int type, int flag);
int clif_skill_damage2(struct block_list *src,struct block_list *dst,
	unsigned int tick,int sdelay,int ddelay,int damage,int div,
	int skill_id,int skill_lv,int type);
int clif_skill_nodamage(struct block_list *src,struct block_list *dst,
	int skill_id,int heal,int fail);
int clif_skill_poseffect(struct block_list *src,int skill_id,
	int val,int x,int y,int tick);
int clif_skill_estimation(struct map_session_data *sd,struct block_list *dst);
int clif_skill_warppoint(struct map_session_data *sd,int skill_num, int skill_lv,
	const char *map1,const char *map2,const char *map3,const char *map4);
int clif_skill_memo(struct map_session_data *sd,int flag);
int clif_skill_teleportmessage(struct map_session_data *sd,int flag);
int clif_skill_produce_mix_list(struct map_session_data *sd, int trigger);

int clif_produceeffect(struct map_session_data *sd,int flag,int nameid);

int clif_skill_setunit(struct skill_unit *unit);
int clif_skill_delunit(struct skill_unit *unit);

int clif_01ac(struct block_list *bl);

int clif_autospell(struct map_session_data *sd,int skilllv);
int clif_devotion(struct map_session_data *sd);
int clif_marionette(struct block_list *src, struct block_list *target);
int clif_spiritball(struct map_session_data *sd);
int clif_combo_delay(struct block_list *src,int wait);
int clif_bladestop(struct block_list *src,struct block_list *dst,int bool_);
int clif_changemapcell(int m,int x,int y,int cell_type,int type);

int clif_status_load(struct block_list *bl,int type, int flag);
int clif_status_change(struct block_list *bl,int type,int flag);

int clif_wis_message(int fd,char *nick,char *mes,int mes_len);
int clif_wis_end(int fd,int flag);

int clif_solved_charname(struct map_session_data *sd,int char_id);
int clif_charnameack(int fd, struct block_list *bl);
int clif_charnameupdate(struct map_session_data *ssd);

int clif_use_card(struct map_session_data *sd,int idx);
int clif_insert_card(struct map_session_data *sd,int idx_equip,int idx_card,int flag);

void clif_inventorylist(struct map_session_data *sd);
void clif_equiplist(struct map_session_data *sd);

int clif_cart_additem(struct map_session_data*,int,int,int);
int clif_cart_delitem(struct map_session_data*,int,int);
void clif_cartlist(struct map_session_data *sd);

int clif_item_identify_list(struct map_session_data *sd);
int clif_item_identified(struct map_session_data *sd,int idx,int flag);
int clif_item_repair_list (struct map_session_data *sd, struct map_session_data *dstsd);
int clif_item_repaireffect(struct map_session_data *sd, int nameid, int flag);
int clif_item_refine_list(struct map_session_data *sd);

int clif_item_skill(struct map_session_data *sd,int skillid,int skilllv,const char *name);

int clif_mvp_effect(struct map_session_data *sd);
int clif_mvp_item(struct map_session_data *sd,int nameid);
int clif_mvp_exp(struct map_session_data *sd,unsigned long exp);
void clif_changed_dir(struct block_list *bl, int area);

// vending
int clif_openvendingreq(struct map_session_data *sd,int num);
int clif_showvendingboard(struct block_list* bl,char *message,int fd);
int clif_closevendingboard(struct block_list* bl,int fd);
int clif_vendinglist(struct map_session_data *sd,int id,struct vending *vending);
int clif_buyvending(struct map_session_data *sd,int index,int amount,int fail);
int clif_openvending(struct map_session_data *sd,int id,struct vending *vending);
int clif_vendingreport(struct map_session_data *sd,int index,int amount);

int clif_movetoattack(struct map_session_data *sd,struct block_list *bl);

// party
int clif_party_created(struct map_session_data *sd,int flag);
int clif_party_main_info(struct party_data *p, int fd);
int clif_party_join_info(struct party *p, struct map_session_data *sd);
int clif_party_info(struct party_data *p,int fd);
int clif_party_invite(struct map_session_data *sd,struct map_session_data *tsd);
int clif_party_inviteack(struct map_session_data *sd,char *nick,int flag);
int clif_party_option(struct party_data *p,struct map_session_data *sd,int flag);
int clif_party_leaved(struct party_data *p,struct map_session_data *sd,int account_id,char *name,int flag);
int clif_party_message(struct party_data *p,int account_id,char *mes,int len);
int clif_party_move(struct party *p,struct map_session_data *sd,int online);
int clif_party_xy(struct map_session_data *sd);
int clif_party_xy_single(int fd, struct map_session_data *sd);
int clif_party_hp(struct map_session_data *sd);
int clif_hpmeter(struct map_session_data *sd);

// guild
int clif_guild_created(struct map_session_data *sd,int flag);
int clif_guild_belonginfo(struct map_session_data *sd,struct guild *g);
int clif_guild_basicinfo(struct map_session_data *sd);
int clif_guild_allianceinfo(struct map_session_data *sd);
int clif_guild_memberlist(struct map_session_data *sd);
int clif_guild_skillinfo(struct map_session_data *sd);
int clif_guild_send_onlineinfo(struct map_session_data *sd); //[LuzZza]
int clif_guild_memberlogin_notice(struct guild *g,int idx,int flag);
int clif_guild_invite(struct map_session_data *sd,struct guild *g);
int clif_guild_inviteack(struct map_session_data *sd,int flag);
int clif_guild_leave(struct map_session_data *sd,const char *name,const char *mes);
int clif_guild_expulsion(struct map_session_data *sd,const char *name,const char *mes,int account_id);
int clif_guild_positionchanged(struct guild *g,int idx);
int clif_guild_memberpositionchanged(struct guild *g,int idx);
int clif_guild_emblem(struct map_session_data *sd,struct guild *g);
int clif_guild_notice(struct map_session_data *sd,struct guild *g);
int clif_guild_message(struct guild *g,int account_id,const char *mes,int len);
int clif_guild_skillup(struct map_session_data *sd,int skill_num,int lv);
int clif_guild_reqalliance(struct map_session_data *sd,int account_id,const char *name);
int clif_guild_allianceack(struct map_session_data *sd,int flag);
int clif_guild_delalliance(struct map_session_data *sd,int guild_id,int flag);
int clif_guild_oppositionack(struct map_session_data *sd,int flag);
int clif_guild_broken(struct map_session_data *sd,int flag);
int clif_guild_xy(struct map_session_data *sd);
int clif_guild_xy_single(int fd, struct map_session_data *sd);
int clif_guild_xy_remove(struct map_session_data *sd);


// atcommand
int clif_displaymessage(const int fd,char* mes);
int clif_disp_onlyself(struct map_session_data *sd,char *mes,int len);
void clif_disp_message(struct block_list *src, char *mes, int len, int type);
int clif_GMmessage(struct block_list *bl,char* mes,int len,int flag);
void clif_MainChatMessage(char* message); //luzza
int clif_announce(struct block_list *bl, char* mes, int len, unsigned long color, int flag);
int clif_heal(int fd,int type,int val);
int clif_resurrection(struct block_list *bl,int type);
int clif_set0199(int fd,int type);
int clif_pvpset(struct map_session_data *sd, int pvprank, int pvpnum,int type);
int clif_send0199(int map,int type);
int clif_refine(int fd,struct map_session_data *sd,int fail,int index,int val);

//petsystem
int clif_catch_process(struct map_session_data *sd);
int clif_pet_rulet(struct map_session_data *sd,int data);
int clif_sendegg(struct map_session_data *sd);
int clif_send_petdata(struct map_session_data *sd,int type,int param);
int clif_send_petstatus(struct map_session_data *sd);
int clif_pet_emotion(struct pet_data *pd,int param);
int clif_pet_performance(struct block_list *bl,int param);
int clif_pet_equip(struct pet_data *pd);
int clif_pet_food(struct map_session_data *sd,int foodid,int fail);
int clif_send(unsigned char *buf, int len, struct block_list *bl, int type);

//friends list
int clif_friendslist_toggle_sub(struct map_session_data *sd,va_list ap);
void clif_friendslist_send(struct map_session_data *sd);
void clif_friendslist_reqack(struct map_session_data *sd, struct map_session_data *f_sd, int type);

// [Valaris]
int clif_mob_hp(struct mob_data *md);
int clif_weather(int m); // [Valaris]
int clif_specialeffect(struct block_list *bl,int type, int flag); // special effects [Valaris]
int clif_message(struct block_list *bl, char* msg); // messages (from mobs/npcs) [Valaris]

int clif_GM_kickack(struct map_session_data *sd,int id);
int clif_GM_kick(struct map_session_data *sd,struct map_session_data *tsd,int type);
int clif_GM_silence(struct map_session_data *sd,struct map_session_data *tsd,int type);
int clif_timedout(struct map_session_data *sd);

int clif_foreachclient(int (*)(struct map_session_data*,va_list),...);
int clif_disp_overhead(struct map_session_data *sd, char* mes);

int do_final_clif(void);
int do_init_clif(void);

void clif_get_weapon_view(TBL_PC* sd, unsigned short *rhand, unsigned short *lhand);

int clif_party_xy_remove(struct map_session_data *sd); //Fix for minimap [Kevin]
void clif_gospel_info(struct map_session_data *sd, int type);
void clif_parse_ReqFeel(int fd, struct map_session_data *sd, int skilllv); 
void clif_feel_info(struct map_session_data *sd, unsigned char feel_level, unsigned char type);
void clif_hate_info(struct map_session_data *sd, unsigned char hate_level,int class_, unsigned char type);
void clif_mission_info(struct map_session_data *sd, int mob_id, unsigned char progress);
void clif_feel_hate_reset(struct map_session_data *sd);

// [blackhole89]
int clif_spawnhomun(struct homun_data *hd);
int clif_hominfo(struct map_session_data *sd, struct homun_data *hd, int flag);
int clif_homskillinfoblock(struct map_session_data *sd);
void clif_homskillup(struct map_session_data *sd, int skill_num) ;	//[orn]
int clif_hom_food(struct map_session_data *sd,int foodid,int fail);	//[orn]
void clif_send_homdata(struct map_session_data *sd, int type, int param);	//[orn]
int clif_hwalkok(struct homun_data *hd);	//[orn]

#endif


