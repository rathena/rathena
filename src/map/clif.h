// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CLIF_H_
#define _CLIF_H_

#include "../common/cbasetypes.h"
//#include "../common/mmo.h"
struct item;
struct storage_data;
struct guild_storage;
//#include "map.h"
struct block_list;
struct unit_data;
struct map_session_data;
struct homun_data;
struct pet_data;
struct mob_data;
struct npc_data;
struct chat_data;
struct flooritem_data;
struct skill_unit;
struct s_vending;
struct party;
struct party_data;
struct guild;
struct battleground_data;
struct quest;
struct party_booking_ad_info;
#include <stdarg.h>

enum
{// packet DB
	MAX_PACKET_DB  = 0xA00,
	MAX_PACKET_VER = 30,
	MAX_PACKET_POS = 20,
};

struct s_packet_db {
	short len;
	void (*func)(int, struct map_session_data *);
	short pos[MAX_PACKET_POS];
};

// packet_db[SERVER] is reserved for server use
#define SERVER 0
#define packet_len(cmd) packet_db[SERVER][cmd].len
extern struct s_packet_db packet_db[MAX_PACKET_VER+1][MAX_PACKET_DB+1];

// local define
typedef enum send_target {
	ALL_CLIENT,
	ALL_SAMEMAP,
	AREA,				// area
	AREA_WOS,			// area, without self
	AREA_WOC,			// area, without chatrooms
	AREA_WOSC,			// area, without own chatroom
	AREA_CHAT_WOC,		// hearable area, without chatrooms
	CHAT,				// current chatroom
	CHAT_WOS,			// current chatroom, without self
	PARTY,
	PARTY_WOS,
	PARTY_SAMEMAP,
	PARTY_SAMEMAP_WOS,
	PARTY_AREA,
	PARTY_AREA_WOS,
	GUILD,
	GUILD_WOS,
	GUILD_SAMEMAP,
	GUILD_SAMEMAP_WOS,
	GUILD_AREA,
	GUILD_AREA_WOS,
	GUILD_NOBG,
	DUEL,
	DUEL_WOS,
	CHAT_MAINCHAT,		// everyone on main chat
	SELF,
	BG,					// BattleGround System
	BG_WOS,
	BG_SAMEMAP,
	BG_SAMEMAP_WOS,
	BG_AREA,
	BG_AREA_WOS,
} send_target;

typedef enum emotion_type
{
	E_GASP = 0,     // /!
	E_WHAT,         // /?
	E_HO,
	E_LV,
	E_SWT,
	E_IC,
	E_AN,
	E_AG,
	E_CASH,         // /$
	E_DOTS,         // /...
	E_SCISSORS,     // /gawi --- 10
	E_ROCK,         // /bawi
	E_PAPER,        // /bo
	E_KOREA,
	E_LV2,
	E_THX,
	E_WAH,
	E_SRY,
	E_HEH,
	E_SWT2,
	E_HMM,          // --- 20
	E_NO1,
	E_NO,           // /??
	E_OMG,
	E_OH,
	E_X,
	E_HLP,
	E_GO,
	E_SOB,
	E_GG,
	E_KIS,          // --- 30
	E_KIS2,
	E_PIF,
	E_OK,
	E_MUTE,         // red /... used for muted characters
	E_INDONESIA,
	E_BZZ,          // /bzz, /stare
	E_RICE,
	E_AWSM,         // /awsm, /cool
	E_MEH,
	E_SHY,          // --- 40
	E_PAT,          // /pat, /goodboy
	E_MP,           // /mp, /sptime
	E_SLUR,
	E_COM,          // /com, /comeon
	E_YAWN,         // /yawn, /sleepy
	E_GRAT,         // /grat, /congrats
	E_HP,           // /hp, /hptime
	E_PHILIPPINES,
	E_MALAYSIA,
	E_SINGAPORE,    // --- 50
	E_BRAZIL,
	E_FLASH,        // /fsh
	E_SPIN,         // /spin
	E_SIGH,
	E_DUM,          // /dum
	E_LOUD,         // /crwd
	E_OTL,          // /otl, /desp
	E_DICE1,
	E_DICE2,
	E_DICE3,        // --- 60
	E_DICE4,
	E_DICE5,
	E_DICE6,
	E_INDIA,
	E_LUV,          // /love
	E_RUSSIA,
	E_VIRGIN,
	E_MOBILE,
	E_MAIL,
	E_CHINESE,      // --- 70
	E_ANTENNA1,
	E_ANTENNA2,
	E_ANTENNA3,
	E_HUM,
	E_ABS,
	E_OOPS,
	E_SPIT,
	E_ENE,
	E_PANIC,
	E_WHISP,        // --- 80
	//
	E_MAX
} emotion_type;

typedef enum clr_type
{
	CLR_OUTSIGHT = 0,
	CLR_DEAD,
	CLR_RESPAWN,
	CLR_TELEPORT,
	CLR_TRICKDEAD,
} clr_type;

enum map_property
{// clif_map_property
	MAPPROPERTY_NOTHING       = 0,
	MAPPROPERTY_FREEPVPZONE   = 1,
	MAPPROPERTY_EVENTPVPZONE  = 2,
	MAPPROPERTY_AGITZONE      = 3,
	MAPPROPERTY_PKSERVERZONE  = 4, // message "You are in a PK area. Please beware of sudden attacks." in color 0x9B9BFF (light red)
	MAPPROPERTY_PVPSERVERZONE = 5,
	MAPPROPERTY_DENYSKILLZONE = 6,
};

enum map_type
{// clif_map_type
	MAPTYPE_VILLAGE              = 0,
	MAPTYPE_VILLAGE_IN           = 1,
	MAPTYPE_FIELD                = 2,
	MAPTYPE_DUNGEON              = 3,
	MAPTYPE_ARENA                = 4,
	MAPTYPE_PENALTY_FREEPKZONE   = 5,
	MAPTYPE_NOPENALTY_FREEPKZONE = 6,
	MAPTYPE_EVENT_GUILDWAR       = 7,
	MAPTYPE_AGIT                 = 8,
	MAPTYPE_DUNGEON2             = 9,
	MAPTYPE_DUNGEON3             = 10,
	MAPTYPE_PKSERVER             = 11,
	MAPTYPE_PVPSERVER            = 12,
	MAPTYPE_DENYSKILL            = 13,
	MAPTYPE_TURBOTRACK           = 14,
	MAPTYPE_JAIL                 = 15,
	MAPTYPE_MONSTERTRACK         = 16,
	MAPTYPE_PORINGBATTLE         = 17,
	MAPTYPE_AGIT_SIEGEV15        = 18,
	MAPTYPE_BATTLEFIELD          = 19,
	MAPTYPE_PVP_TOURNAMENT       = 20,
	MAPTYPE_UNUSED               = 29,
};

enum useskill_fail_cause
{// clif_skill_fail
	USESKILL_FAIL_LEVEL = 0,
	USESKILL_FAIL_SP_INSUFFICIENT = 1,
	USESKILL_FAIL_HP_INSUFFICIENT = 2,
	USESKILL_FAIL_STUFF_INSUFFICIENT = 3,
	USESKILL_FAIL_SKILLINTERVAL = 4,
	USESKILL_FAIL_MONEY = 5,
	USESKILL_FAIL_THIS_WEAPON = 6,
	USESKILL_FAIL_REDJAMSTONE = 7,
	USESKILL_FAIL_BLUEJAMSTONE = 8,
	USESKILL_FAIL_WEIGHTOVER = 9,
	USESKILL_FAIL = 10,
	USESKILL_FAIL_TOTARGET = 11,
	USESKILL_FAIL_ANCILLA_NUMOVER = 12,
	USESKILL_FAIL_HOLYWATER = 13,
	USESKILL_FAIL_ANCILLA = 14,
	USESKILL_FAIL_DUPLICATE_RANGEIN = 15,
	USESKILL_FAIL_NEED_OTHER_SKILL = 16,
	USESKILL_FAIL_NEED_HELPER = 17,
	USESKILL_FAIL_INVALID_DIR = 18,
	USESKILL_FAIL_SUMMON = 19,
	USESKILL_FAIL_SUMMON_NONE = 20,
	USESKILL_FAIL_IMITATION_SKILL_NONE = 21,
	USESKILL_FAIL_DUPLICATE = 22,
	USESKILL_FAIL_CONDITION = 23,
	USESKILL_FAIL_PAINTBRUSH = 24,
	//XXX_USESKILL_FAIL_II_SURFACE_PAINTS = 25,
	USESKILL_FAIL_POS = 26,
	USESKILL_FAIL_HELPER_SP_INSUFFICIENT = 27,
	//XXX_USESKILL_FAIL_II_NECK_CANDY = 28,
	//XXX_USESKILL_FAIL_II_MISERABLE_TEAR = 29,
	//XXX_USESKILL_FAIL_II_PROTECT_NECK_CANDY = 30,
	USESKILL_FAIL_GC_WEAPONBLOCKING = 31,
	USESKILL_FAIL_GC_POISONINGWEAPON = 32,
	USESKILL_FAIL_MADOGEAR = 33,
	//XXX_USESKILL_FAIL_II_VULCANBULLET = 34,
	//XXX_USESKILL_FAIL_II_FUELGAS = 35,
	//XXX_USESKILL_FAIL_II_COLDSLOWERBULLET = 36,
	USESKILL_FAIL_CANONBALL = 37,
	//XXX_USESKILL_FAIL_II_MADOGEAR_ACCELERATION = 38,
	//XXX_USESKILL_FAIL_II_MADOGEAR_HOVERING_BOOSTER = 39,
	USESKILL_FAIL_MADOGEAR_HOVERING = 40,
	//XXX_USESKILL_FAIL_II_MADOGEAR_SELFDESTRUCTION_DEVICE = 41,
	//XXX_USESKILL_FAIL_II_MADOGEAR_SHAPESHIFTER = 42,
	USESKILL_FAIL_GUILLONTINE_POISON = 43,
	//XXX_USESKILL_FAIL_II_MADOGEAR_COOLING_DEVICE = 44,
	//XXX_USESKILL_FAIL_II_MADOGEAR_MAGNETICFIELD_GENERATOR = 45,
	//XXX_USESKILL_FAIL_II_MADOGEAR_BARRIER_GENERATOR = 46,
	//XXX_USESKILL_FAIL_II_MADOGEAR_OPTICALCAMOUFLAGE_GENERATOR = 47,
	//XXX_USESKILL_FAIL_II_MADOGEAR_REPAIRKIT = 48,
	//XXX_USESKILL_FAIL_II_MONKEY_SPANNER = 49,
	USESKILL_FAIL_MADOGEAR_RIDE = 50,
	USESKILL_FAIL_SPELLBOOK = 51,
	USESKILL_FAIL_SPELLBOOK_DIFFICULT_SLEEP = 52,
	USESKILL_FAIL_SPELLBOOK_PRESERVATION_POINT = 53,
	USESKILL_FAIL_SPELLBOOK_READING = 54,
	//XXX_USESKILL_FAIL_II_FACE_PAINTS = 55,
	//XXX_USESKILL_FAIL_II_MAKEUP_BRUSH = 56,
	USESKILL_FAIL_CART = 57,
	//XXX_USESKILL_FAIL_II_THORNS_SEED = 58,
	//XXX_USESKILL_FAIL_II_BLOOD_SUCKER_SEED = 59,
	USESKILL_FAIL_NO_MORE_SPELL = 60,
	//XXX_USESKILL_FAIL_II_BOMB_MUSHROOM_SPORE = 61,
	//XXX_USESKILL_FAIL_II_GASOLINE_BOOMB = 62,
	//XXX_USESKILL_FAIL_II_OIL_BOTTLE = 63,
	//XXX_USESKILL_FAIL_II_EXPLOSION_POWDER = 64,
	//XXX_USESKILL_FAIL_II_SMOKE_POWDER = 65,
	//XXX_USESKILL_FAIL_II_TEAR_GAS = 66,
	//XXX_USESKILL_FAIL_II_HYDROCHLORIC_ACID_BOTTLE = 67,
	//XXX_USESKILL_FAIL_II_HELLS_PLANT_BOTTLE = 68,
	//XXX_USESKILL_FAIL_II_MANDRAGORA_FLOWERPOT = 69,
	USESKILL_FAIL_MANUAL_NOTIFY = 70,
	USESKILL_FAIL_NEED_ITEM = 71,
	USESKILL_FAIL_NEED_EQUIPMENT = 72,
	USESKILL_FAIL_COMBOSKILL = 73,
	USESKILL_FAIL_SPIRITS = 74,
	USESKILL_FAIL_EXPLOSIONSPIRITS = 75,
	USESKILL_FAIL_HP_TOOMANY = 76,
	USESKILL_FAIL_NEED_ROYAL_GUARD_BANDING = 77,
	USESKILL_FAIL_NEED_EQUIPPED_WEAPON_CLASS = 78,
	USESKILL_FAIL_EL_SUMMON = 79,
};

int clif_setip(const char* ip);
void clif_setbindip(const char* ip);
void clif_setport(uint16 port);

uint32 clif_getip(void);
uint32 clif_refresh_ip(void);
uint16 clif_getport(void);

void clif_authok(struct map_session_data *sd);
void clif_authrefuse(int fd, uint8 error_code);
void clif_authfail_fd(int fd, int type);
void clif_charselectok(int id, uint8 ok);
void clif_dropflooritem(struct flooritem_data* fitem);
void clif_clearflooritem(struct flooritem_data *fitem, int fd);

void clif_clearunit_single(int id, clr_type type, int fd);
void clif_clearunit_area(struct block_list* bl, clr_type type);
void clif_clearunit_delayed(struct block_list* bl, clr_type type, unsigned int tick);
int clif_spawn(struct block_list *bl);	//area
void clif_walkok(struct map_session_data *sd);	// self
void clif_move(struct unit_data *ud); //area
void clif_changemap(struct map_session_data *sd, short map, int x, int y);	//self
void clif_changemapserver(struct map_session_data* sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port);	//self
void clif_blown(struct block_list *bl); // area
void clif_slide(struct block_list *bl, int x, int y); // area
void clif_fixpos(struct block_list *bl);	// area
void clif_npcbuysell(struct map_session_data* sd, int id);	//self
void clif_buylist(struct map_session_data *sd, struct npc_data *nd);	//self
void clif_selllist(struct map_session_data *sd);	//self
void clif_scriptmes(struct map_session_data *sd, int npcid, const char *mes);	//self
void clif_scriptnext(struct map_session_data *sd,int npcid);	//self
void clif_scriptclose(struct map_session_data *sd, int npcid);	//self
void clif_scriptmenu(struct map_session_data* sd, int npcid, const char* mes);	//self
void clif_scriptinput(struct map_session_data *sd, int npcid);	//self
void clif_scriptinputstr(struct map_session_data *sd, int npcid);	// self
void clif_cutin(struct map_session_data* sd, const char* image, int type);	//self
void clif_viewpoint(struct map_session_data *sd, int npc_id, int type, int x, int y, int id, int color);	//self
void clif_additem(struct map_session_data *sd, int n, int amount, int fail); // self
void clif_dropitem(struct map_session_data *sd,int n,int amount);	//self
void clif_delitem(struct map_session_data *sd,int n,int amount, short reason); //self
void clif_updatestatus(struct map_session_data *sd,int type);	//self
void clif_changestatus(struct map_session_data* sd,int type,int val);	//area
int clif_damage(struct block_list* src, struct block_list* dst, unsigned int tick, int sdelay, int ddelay, int damage, int div, int type, int damage2);	// area
void clif_takeitem(struct block_list* src, struct block_list* dst);
void clif_sitting(struct block_list* bl);
void clif_standing(struct block_list* bl);
void clif_changelook(struct block_list *bl,int type,int val);	// area
void clif_changetraplook(struct block_list *bl,int val); // area
void clif_refreshlook(struct block_list *bl,int id,int type,int val,enum send_target target); //area specified in 'target'
void clif_arrowequip(struct map_session_data *sd,int val); //self
void clif_arrow_fail(struct map_session_data *sd,int type); //self
void clif_arrow_create_list(struct map_session_data *sd);	//self
void clif_statusupack(struct map_session_data *sd,int type,int ok,int val);	// self
void clif_equipitemack(struct map_session_data *sd,int n,int pos,int ok);	// self
void clif_unequipitemack(struct map_session_data *sd,int n,int pos,int ok);	// self
void clif_misceffect(struct block_list* bl,int type);	// area
void clif_changeoption(struct block_list* bl);	// area
void clif_changeoption2(struct block_list* bl);	// area
void clif_useitemack(struct map_session_data *sd,int index,int amount,bool ok);	// self
void clif_GlobalMessage(struct block_list* bl, const char* message);
void clif_createchat(struct map_session_data* sd, int flag);	// self
void clif_dispchat(struct chat_data* cd, int fd);	// area or fd
void clif_joinchatfail(struct map_session_data *sd,int flag);	// self
void clif_joinchatok(struct map_session_data *sd,struct chat_data* cd);	// self
void clif_addchat(struct chat_data* cd,struct map_session_data *sd);	// chat
void clif_changechatowner(struct chat_data* cd, struct map_session_data* sd);	// chat
void clif_clearchat(struct chat_data *cd,int fd);	// area or fd
void clif_leavechat(struct chat_data* cd, struct map_session_data* sd, bool flag);	// chat
void clif_changechatstatus(struct chat_data* cd);	// chat
void clif_refresh(struct map_session_data *sd);	// self

void clif_fame_blacksmith(struct map_session_data *sd, int points);
void clif_fame_alchemist(struct map_session_data *sd, int points);
void clif_fame_taekwon(struct map_session_data *sd, int points);

void clif_emotion(struct block_list *bl,int type);
void clif_talkiebox(struct block_list* bl, const char* talkie);
void clif_wedding_effect(struct block_list *bl);
void clif_divorced(struct map_session_data* sd, const char* name);
void clif_callpartner(struct map_session_data *sd);
void clif_playBGM(struct map_session_data* sd, const char* name);
void clif_soundeffect(struct map_session_data* sd, struct block_list* bl, const char* name, int type);
void clif_soundeffectall(struct block_list* bl, const char* name, int type, enum send_target coverage);
void clif_parse_ActionRequest_sub(struct map_session_data *sd, int action_type, int target_id, unsigned int tick);
void clif_parse_LoadEndAck(int fd,struct map_session_data *sd);
void clif_hotkeys_send(struct map_session_data *sd);

// trade
void clif_traderequest(struct map_session_data* sd, const char* name);
void clif_tradestart(struct map_session_data* sd, uint8 type);
void clif_tradeadditem(struct map_session_data* sd, struct map_session_data* tsd, int index, int amount);
void clif_tradeitemok(struct map_session_data* sd, int index, int fail);
void clif_tradedeal_lock(struct map_session_data* sd, int fail);
void clif_tradecancelled(struct map_session_data* sd);
void clif_tradecompleted(struct map_session_data* sd, int fail);
void clif_tradeundo(struct map_session_data* sd);

// storage
void clif_storagelist(struct map_session_data* sd, struct item* items, int items_length);
void clif_updatestorageamount(struct map_session_data* sd, int amount, int max_amount);
void clif_storageitemadded(struct map_session_data* sd, struct item* i, int index, int amount);
void clif_storageitemremoved(struct map_session_data* sd, int index, int amount);
void clif_storageclose(struct map_session_data* sd);

int clif_insight(struct block_list *bl,va_list ap);	// map_forallinmovearea callback
int clif_outsight(struct block_list *bl,va_list ap);	// map_forallinmovearea callback

void clif_class_change(struct block_list *bl,int class_,int type);
#define clif_mob_class_change(md, class_) clif_class_change(&md->bl, class_, 1)

void clif_skillinfoblock(struct map_session_data *sd);
void clif_skillup(struct map_session_data *sd,int skill_num);
void clif_skillinfo(struct map_session_data *sd,int skill, int inf);
void clif_addskill(struct map_session_data *sd, int id);
void clif_deleteskill(struct map_session_data *sd, int id);

void clif_skillcasting(struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, int skill_num, int property, int casttime);
void clif_skillcastcancel(struct block_list* bl);
void clif_skill_fail(struct map_session_data *sd,int skill_id,enum useskill_fail_cause cause,int btype);
void clif_skill_cooldown(struct map_session_data *sd, int skillid, unsigned int tick);
int clif_skill_damage(struct block_list *src,struct block_list *dst,unsigned int tick,int sdelay,int ddelay,int damage,int div,int skill_id,int skill_lv,int type);
//int clif_skill_damage2(struct block_list *src,struct block_list *dst,unsigned int tick,int sdelay,int ddelay,int damage,int div,int skill_id,int skill_lv,int type);
int clif_skill_nodamage(struct block_list *src,struct block_list *dst,int skill_id,int heal,int fail);
void clif_skill_poseffect(struct block_list *src,int skill_id,int val,int x,int y,int tick);
void clif_skill_estimation(struct map_session_data *sd,struct block_list *dst);
void clif_skill_warppoint(struct map_session_data* sd, short skill_num, short skill_lv, unsigned short map1, unsigned short map2, unsigned short map3, unsigned short map4);
void clif_skill_memomessage(struct map_session_data* sd, int type);
void clif_skill_teleportmessage(struct map_session_data *sd, int type);
void clif_skill_produce_mix_list(struct map_session_data *sd, int skillid, int trigger);
void clif_cooking_list(struct map_session_data *sd, int trigger, int skill_id, int qty, int list_type);

void clif_produceeffect(struct map_session_data* sd,int flag,int nameid);

void clif_skill_setunit(struct skill_unit *unit);
void clif_skill_delunit(struct skill_unit *unit);

void clif_skillunit_update(struct block_list* bl);

void clif_autospell(struct map_session_data *sd,int skilllv);
void clif_devotion(struct block_list *src, struct map_session_data *tsd);
void clif_spiritball(struct map_session_data *sd);
void clif_combo_delay(struct block_list *bl,int wait);
void clif_bladestop(struct block_list *src, int dst_id, int active);
void clif_changemapcell(int fd, int m, int x, int y, int type, enum send_target target);

#define clif_status_load(bl, type, flag) clif_status_change((bl), (type), (flag), 0, 0, 0, 0)
void clif_status_change(struct block_list *bl,int type,int flag,int tick,int val1, int val2, int val3);

void clif_wis_message(int fd, const char* nick, const char* mes, int mes_len);
void clif_wis_end(int fd, int flag);

void clif_solved_charname(int fd, int charid, const char* name);
void clif_charnameack(int fd, struct block_list *bl);
void clif_charnameupdate(struct map_session_data *ssd);

void clif_use_card(struct map_session_data *sd,int idx);
void clif_insert_card(struct map_session_data *sd,int idx_equip,int idx_card,int flag);

void clif_inventorylist(struct map_session_data *sd);
void clif_equiplist(struct map_session_data *sd);

void clif_cart_additem(struct map_session_data *sd,int n,int amount,int fail);
void clif_cart_delitem(struct map_session_data *sd,int n,int amount);
void clif_cartlist(struct map_session_data *sd);
void clif_clearcart(int fd);

void clif_item_identify_list(struct map_session_data *sd);
void clif_item_identified(struct map_session_data *sd,int idx,int flag);
void clif_item_repair_list(struct map_session_data *sd, struct map_session_data *dstsd);
void clif_item_repaireffect(struct map_session_data *sd, int nameid, int flag);
void clif_item_damaged(struct map_session_data* sd, unsigned short position);
void clif_item_refine_list(struct map_session_data *sd);

void clif_item_skill(struct map_session_data *sd,int skillid,int skilllv);

void clif_mvp_effect(struct map_session_data *sd);
void clif_mvp_item(struct map_session_data *sd,int nameid);
void clif_mvp_exp(struct map_session_data *sd, unsigned int exp);
void clif_mvp_noitem(struct map_session_data* sd);
void clif_changed_dir(struct block_list *bl, enum send_target target);

// vending
void clif_openvendingreq(struct map_session_data* sd, int num);
void clif_showvendingboard(struct block_list* bl, const char* message, int fd);
void clif_closevendingboard(struct block_list* bl, int fd);
void clif_vendinglist(struct map_session_data* sd, int id, struct s_vending* vending);
void clif_buyvending(struct map_session_data* sd, int index, int amount, int fail);
void clif_openvending(struct map_session_data* sd, int id, struct s_vending* vending);
void clif_vendingreport(struct map_session_data* sd, int index, int amount);

void clif_movetoattack(struct map_session_data *sd,struct block_list *bl);

// party
void clif_party_created(struct map_session_data *sd,int result);
void clif_party_member_info(struct party_data *p, struct map_session_data *sd);
void clif_party_info(struct party_data* p, struct map_session_data *sd);
void clif_party_invite(struct map_session_data *sd,struct map_session_data *tsd);
void clif_party_inviteack(struct map_session_data* sd, const char* nick, int result);
void clif_party_option(struct party_data *p,struct map_session_data *sd,int flag);
void clif_party_withdraw(struct party_data* p, struct map_session_data* sd, int account_id, const char* name, int flag);
void clif_party_message(struct party_data* p, int account_id, const char* mes, int len);
void clif_party_xy(struct map_session_data *sd);
void clif_party_xy_single(int fd, struct map_session_data *sd);
void clif_party_hp(struct map_session_data *sd);
void clif_hpmeter_single(int fd, int id, unsigned int hp, unsigned int maxhp);

// guild
void clif_guild_created(struct map_session_data *sd,int flag);
void clif_guild_belonginfo(struct map_session_data *sd, struct guild *g);
void clif_guild_masterormember(struct map_session_data *sd);
void clif_guild_basicinfo(struct map_session_data *sd);
void clif_guild_allianceinfo(struct map_session_data *sd);
void clif_guild_memberlist(struct map_session_data *sd);
void clif_guild_skillinfo(struct map_session_data* sd);
void clif_guild_send_onlineinfo(struct map_session_data *sd); //[LuzZza]
void clif_guild_memberlogin_notice(struct guild *g,int idx,int flag);
void clif_guild_invite(struct map_session_data *sd,struct guild *g);
void clif_guild_inviteack(struct map_session_data *sd,int flag);
void clif_guild_leave(struct map_session_data *sd,const char *name,const char *mes);
void clif_guild_expulsion(struct map_session_data* sd, const char* name, const char* mes, int account_id);
void clif_guild_positionchanged(struct guild *g,int idx);
void clif_guild_memberpositionchanged(struct guild *g,int idx);
void clif_guild_emblem(struct map_session_data *sd,struct guild *g);
void clif_guild_emblem_area(struct block_list* bl);
void clif_guild_notice(struct map_session_data* sd, struct guild* g);
void clif_guild_message(struct guild *g,int account_id,const char *mes,int len);
int clif_guild_skillup(struct map_session_data *sd,int skill_num,int lv);
void clif_guild_reqalliance(struct map_session_data *sd,int account_id,const char *name);
void clif_guild_allianceack(struct map_session_data *sd,int flag);
void clif_guild_delalliance(struct map_session_data *sd,int guild_id,int flag);
void clif_guild_oppositionack(struct map_session_data *sd,int flag);
void clif_guild_broken(struct map_session_data *sd,int flag);
void clif_guild_xy(struct map_session_data *sd);
void clif_guild_xy_single(int fd, struct map_session_data *sd);
void clif_guild_xy_remove(struct map_session_data *sd);

// Battleground
void clif_bg_hp(struct map_session_data *sd);
void clif_bg_xy(struct map_session_data *sd);
void clif_bg_xy_remove(struct map_session_data *sd);
void clif_bg_message(struct battleground_data *bg, int src_id, const char *name, const char *mes, int len);
void clif_bg_updatescore(int m);
void clif_bg_updatescore_single(struct map_session_data *sd);
void clif_sendbgemblem_area(struct map_session_data *sd);
void clif_sendbgemblem_single(int fd, struct map_session_data *sd);

// Instancing
int clif_instance(int instance_id, int type, int flag);
void clif_instance_join(int fd, int instance_id);
void clif_instance_leave(int fd);

// Custom Fonts
void clif_font(struct map_session_data *sd);

// atcommand
void clif_displaymessage(const int fd, const char* mes);
void clif_disp_onlyself(struct map_session_data *sd, const char *mes, int len);
void clif_disp_message(struct block_list* src, const char* mes, int len, enum send_target target);
void clif_broadcast(struct block_list* bl, const char* mes, int len, int type, enum send_target target);
void clif_MainChatMessage(const char* message); //luzza
void clif_broadcast2(struct block_list* bl, const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target);
void clif_heal(int fd,int type,int val);
void clif_resurrection(struct block_list *bl,int type);
void clif_map_property(struct map_session_data* sd, enum map_property property);
void clif_pvpset(struct map_session_data *sd, int pvprank, int pvpnum,int type);
void clif_map_property_mapall(int map, enum map_property property);
void clif_refine(int fd, int fail, int index, int val);
void clif_upgrademessage(int fd, int result, int item_id);

//petsystem
void clif_catch_process(struct map_session_data *sd);
void clif_pet_roulette(struct map_session_data *sd,int data);
void clif_sendegg(struct map_session_data *sd);
void clif_send_petstatus(struct map_session_data *sd);
void clif_send_petdata(struct map_session_data* sd, struct pet_data* pd, int type, int param);
#define clif_pet_equip(sd, pd) clif_send_petdata(sd, pd, 3, (pd)->vd.head_bottom)
#define clif_pet_equip_area(pd) clif_send_petdata(NULL, pd, 3, (pd)->vd.head_bottom)
#define clif_pet_performance(pd, param) clif_send_petdata(NULL, pd, 4, param)
void clif_pet_emotion(struct pet_data *pd,int param);
void clif_pet_food(struct map_session_data *sd,int foodid,int fail);

//friends list
int clif_friendslist_toggle_sub(struct map_session_data *sd,va_list ap);
void clif_friendslist_send(struct map_session_data *sd);
void clif_friendslist_reqack(struct map_session_data *sd, struct map_session_data *f_sd, int type);

void clif_weather(int m); // [Valaris]
void clif_specialeffect(struct block_list* bl, int type, enum send_target target); // special effects [Valaris]
void clif_specialeffect_single(struct block_list* bl, int type, int fd);
void clif_messagecolor(struct block_list* bl, unsigned long color, const char* msg); // Mob/Npc color talk [SnakeDrak]
void clif_message(struct block_list* bl, const char* msg); // messages (from mobs/npcs) [Valaris]
void clif_specialeffect_value(struct block_list* bl, int effect_id, int num, send_target target);

void clif_GM_kickack(struct map_session_data *sd, int id);
void clif_GM_kick(struct map_session_data *sd,struct map_session_data *tsd);
void clif_manner_message(struct map_session_data* sd, uint32 type);
void clif_GM_silence(struct map_session_data* sd, struct map_session_data* tsd, uint8 type);

void clif_disp_overhead(struct map_session_data *sd, const char* mes);

void clif_get_weapon_view(struct map_session_data* sd, unsigned short *rhand, unsigned short *lhand);

void clif_party_xy_remove(struct map_session_data *sd); //Fix for minimap [Kevin]
void clif_gospel_info(struct map_session_data *sd, int type);
void clif_feel_req(int fd, struct map_session_data *sd, int skilllv);
void clif_starskill(struct map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result);
void clif_feel_info(struct map_session_data* sd, unsigned char feel_level, unsigned char type);
void clif_hate_info(struct map_session_data *sd, unsigned char hate_level,int class_, unsigned char type);
void clif_mission_info(struct map_session_data *sd, int mob_id, unsigned char progress);
void clif_feel_hate_reset(struct map_session_data *sd);

// [blackhole89]
void clif_hominfo(struct map_session_data *sd, struct homun_data *hd, int flag);
int clif_homskillinfoblock(struct map_session_data *sd);
void clif_homskillup(struct map_session_data *sd, int skill_num);	//[orn]
int clif_hom_food(struct map_session_data *sd,int foodid,int fail);	//[orn]
void clif_send_homdata(struct map_session_data *sd, int state, int param);	//[orn]

void clif_equiptickack(struct map_session_data* sd, int flag);
void clif_viewequip_ack(struct map_session_data* sd, struct map_session_data* tsd);
void clif_viewequip_fail(struct map_session_data* sd);
void clif_equipcheckbox(struct map_session_data* sd);

void clif_msg(struct map_session_data* sd, unsigned short id);
void clif_msg_value(struct map_session_data* sd, unsigned short id, int value);
void clif_msg_skill(struct map_session_data* sd, unsigned short skill_id, int msg_id);

//quest system [Kevin] [Inkfish]
void clif_quest_send_list(struct map_session_data * sd);
void clif_quest_send_mission(struct map_session_data * sd);
void clif_quest_add(struct map_session_data * sd, struct quest * qd, int index);
void clif_quest_delete(struct map_session_data * sd, int quest_id);
void clif_quest_update_status(struct map_session_data * sd, int quest_id, bool active);
void clif_quest_update_objective(struct map_session_data * sd, struct quest * qd, int index);
void clif_quest_show_event(struct map_session_data *sd, struct block_list *bl, short state, short color);
void clif_displayexp(struct map_session_data *sd, unsigned int exp, char type, bool quest);

int clif_send(const uint8* buf, int len, struct block_list* bl, enum send_target type);
int do_init_clif(void);

// MAIL SYSTEM
void clif_Mail_window(int fd, int flag);
void clif_Mail_read(struct map_session_data *sd, int mail_id);
void clif_Mail_delete(int fd, int mail_id, short fail);
void clif_Mail_return(int fd, int mail_id, short fail);
void clif_Mail_send(int fd, bool fail);
void clif_Mail_new(int fd, int mail_id, const char *sender, const char *title);
void clif_Mail_refreshinbox(struct map_session_data *sd);
void clif_Mail_getattachment(int fd, uint8 flag);
// AUCTION SYSTEM
void clif_Auction_openwindow(struct map_session_data *sd);
void clif_Auction_results(struct map_session_data *sd, short count, short pages, uint8 *buf);
void clif_Auction_message(int fd, unsigned char flag);
void clif_Auction_close(int fd, unsigned char flag);
void clif_parse_Auction_cancelreg(int fd, struct map_session_data *sd);

void clif_bossmapinfo(int fd, struct mob_data *md, short flag);
void clif_cashshop_show(struct map_session_data *sd, struct npc_data *nd);

// ADOPTION
void clif_Adopt_reply(struct map_session_data *sd, int type);

// MERCENARIES
void clif_mercenary_info(struct map_session_data *sd);
void clif_mercenary_skillblock(struct map_session_data *sd);
void clif_mercenary_message(struct map_session_data* sd, int message);
void clif_mercenary_updatestatus(struct map_session_data *sd, int type);

// RENTAL SYSTEM
void clif_rental_time(int fd, int nameid, int seconds);
void clif_rental_expired(int fd, int index, int nameid);

// BOOK READING
void clif_readbook(int fd, int book_id, int page);

// Show Picker
void clif_party_show_picker(struct map_session_data * sd, struct item * item_data);

// Progress Bar [Inkfish]
void clif_progressbar(struct map_session_data * sd, unsigned long color, unsigned int second);
void clif_progressbar_abort(struct map_session_data * sd);

void clif_PartyBookingRegisterAck(struct map_session_data *sd, int flag);
void clif_PartyBookingDeleteAck(struct map_session_data* sd, int flag);
void clif_PartyBookingSearchAck(int fd, struct party_booking_ad_info** results, int count, bool more_result);
void clif_PartyBookingUpdateNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad);
void clif_PartyBookingDeleteNotify(struct map_session_data* sd, int index);
void clif_PartyBookingInsertNotify(struct map_session_data* sd, struct party_booking_ad_info* pb_ad);

void clif_showdigit(struct map_session_data* sd, unsigned char type, int value);

/// Buying Store System
void clif_buyingstore_open(struct map_session_data* sd);
void clif_buyingstore_open_failed(struct map_session_data* sd, unsigned short result, unsigned int weight);
void clif_buyingstore_myitemlist(struct map_session_data* sd);
void clif_buyingstore_entry(struct map_session_data* sd);
void clif_buyingstore_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd);
void clif_buyingstore_disappear_entry(struct map_session_data* sd);
void clif_buyingstore_disappear_entry_single(struct map_session_data* sd, struct map_session_data* pl_sd);
void clif_buyingstore_itemlist(struct map_session_data* sd, struct map_session_data* pl_sd);
void clif_buyingstore_trade_failed_buyer(struct map_session_data* sd, short result);
void clif_buyingstore_update_item(struct map_session_data* sd, unsigned short nameid, unsigned short amount);
void clif_buyingstore_delete_item(struct map_session_data* sd, short index, unsigned short amount, int price);
void clif_buyingstore_trade_failed_seller(struct map_session_data* sd, short result, unsigned short nameid);

/// Search Store System
void clif_search_store_info_ack(struct map_session_data* sd);
void clif_search_store_info_failed(struct map_session_data* sd, unsigned char reason);
void clif_open_search_store_info(struct map_session_data* sd);
void clif_search_store_info_click_ack(struct map_session_data* sd, short x, short y);
/**
 * 3CeAM
 **/
void clif_msgtable(int fd, int line);
void clif_msgtable_num(int fd, int line, int num);

int clif_elementalconverter_list(struct map_session_data *sd);

void clif_millenniumshield(struct map_session_data *sd, short shields );

int clif_spellbook_list(struct map_session_data *sd);

int clif_magicdecoy_list(struct map_session_data *sd, int skill_lv, short x, short y);

int clif_poison_list(struct map_session_data *sd, int skill_lv);

int clif_autoshadowspell_list(struct map_session_data *sd);

int clif_status_load_notick(struct block_list *bl,int type,int flag,int val1, int val2, int val3);
int clif_status_load_single(int fd, int id,int type,int flag,int val1, int val2, int val3);


int clif_skill_itemlistwindow( struct map_session_data *sd, int skill_id, int skill_lv );
void clif_elemental_info(struct map_session_data *sd);
void clif_elemental_updatestatus(struct map_session_data *sd, int type);

/**
 * Color Table
 **/
enum clif_colors {
	COLOR_RED,

	COLOR_MAX
};
unsigned long color_table[COLOR_MAX];
int clif_colormes(struct map_session_data * sd, enum clif_colors color, const char* msg);

#define clif_menuskill_clear(sd) (sd)->menuskill_id = (sd)->menuskill_val = (sd)->menuskill_val2 = 0;

#endif /* _CLIF_H_ */
