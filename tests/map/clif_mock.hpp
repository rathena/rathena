#ifndef CLIF_MOCK_HPP
#define CLIF_MOCK_HPP

#include "gmock/gmock.h"

class clif_mock : public Clif_Interface {
 public:
  MOCK_METHOD1(setip,
      int(const char* ip));
  MOCK_METHOD1(setbindip,
      void(const char* ip));
  MOCK_METHOD1(setport,
      void(uint16 port));
  MOCK_METHOD0(getip,
      uint32(void));
  MOCK_METHOD0(refresh_ip,
      uint32(void));
  MOCK_METHOD0(getport,
      uint16(void));
  MOCK_METHOD1(authok,
      void(struct map_session_data *sd));
  MOCK_METHOD2(authrefuse,
      void(int fd, uint8 error_code));
  MOCK_METHOD2(authfail_fd,
      void(int fd, int type));
  MOCK_METHOD2(charselectok,
      void(int id, uint8 ok));
  MOCK_METHOD2(dropflooritem,
      void(struct flooritem_data* fitem, bool canShowEffect));
  MOCK_METHOD2(clearflooritem,
      void(struct flooritem_data *fitem, int fd));
  MOCK_METHOD3(clearunit_single,
      void(int id, clr_type type, int fd));
  MOCK_METHOD2(clearunit_area,
      void(struct block_list* bl, clr_type type));
  MOCK_METHOD3(clearunit_delayed,
      void(struct block_list* bl, clr_type type, t_tick tick));
  MOCK_METHOD1(spawn,
      int(struct block_list *bl));
  MOCK_METHOD1(walkok,
      void(struct map_session_data *sd));
  MOCK_METHOD1(move,
      void(struct unit_data *ud));
  MOCK_METHOD4(changemap,
      void(struct map_session_data *sd, short m, int x, int y));
  MOCK_METHOD6(changemapserver,
      void(struct map_session_data* sd, unsigned short map_index, int x, int y, uint32 ip, uint16 port));
  MOCK_METHOD1(blown,
      void(struct block_list *bl));
  MOCK_METHOD3(slide,
      void(struct block_list *bl, int x, int y));
  MOCK_METHOD1(fixpos,
      void(struct block_list *bl));
  MOCK_METHOD2(npcbuysell,
      void(struct map_session_data* sd, int id));
  MOCK_METHOD2(buylist,
      void(struct map_session_data *sd, struct npc_data *nd));
  MOCK_METHOD1(selllist,
      void(struct map_session_data *sd));
  MOCK_METHOD2(npc_market_open,
      void(struct map_session_data *sd, struct npc_data *nd));
  MOCK_METHOD3(scriptmes,
      void(struct map_session_data *sd, int npcid, const char *mes));
  MOCK_METHOD2(scriptnext,
      void(struct map_session_data *sd,int npcid));
  MOCK_METHOD2(scriptclose,
      void(struct map_session_data *sd, int npcid));
  MOCK_METHOD2(scriptclear,
      void(struct map_session_data *sd, int npcid));
  MOCK_METHOD3(scriptmenu,
      void(struct map_session_data* sd, int npcid, const char* mes));
  MOCK_METHOD2(scriptinput,
      void(struct map_session_data *sd, int npcid));
  MOCK_METHOD2(scriptinputstr,
      void(struct map_session_data *sd, int npcid));
  MOCK_METHOD3(cutin,
      void(struct map_session_data* sd, const char* image, int type));
  MOCK_METHOD7(viewpoint,
      void(struct map_session_data *sd, int npc_id, int type, int x, int y, int id, int color));
  MOCK_METHOD4(additem,
      void(struct map_session_data *sd, int n, int amount, unsigned char fail));
  MOCK_METHOD3(dropitem,
      void(struct map_session_data *sd,int n,int amount));
  MOCK_METHOD4(delitem,
      void(struct map_session_data *sd,int n,int amount, short reason));
  MOCK_METHOD2(updatestatus,
      void(struct map_session_data *sd,int type));
  MOCK_METHOD3(changestatus,
      void(struct map_session_data* sd,int type,int val));
  MOCK_METHOD10(damage,
      int(struct block_list* src, struct block_list* dst, t_tick tick, int sdelay, int ddelay, int64 sdamage, int div, enum e_damage_type type, int64 sdamage2, bool spdamage));
  MOCK_METHOD2(takeitem,
      void(struct block_list* src, struct block_list* dst));
  MOCK_METHOD1(sitting,
      void(struct block_list* bl));
  MOCK_METHOD1(standing,
      void(struct block_list* bl));
  MOCK_METHOD6(sprite_change,
      void(struct block_list *bl, int id, int type, int val, int val2, enum send_target target));
  MOCK_METHOD3(changelook,
      void(struct block_list *bl,int type,int val));
  MOCK_METHOD2(changetraplook,
      void(struct block_list *bl,int val));
  MOCK_METHOD5(refreshlook,
      void(struct block_list *bl,int id,int type,int val,enum send_target target));
  MOCK_METHOD2(arrowequip,
      void(struct map_session_data *sd,int val));
  MOCK_METHOD2(arrow_fail,
      void(struct map_session_data *sd,int type));
  MOCK_METHOD1(arrow_create_list,
      void(struct map_session_data *sd));
  MOCK_METHOD4(statusupack,
      void(struct map_session_data *sd,int type,int ok,int val));
  MOCK_METHOD4(equipitemack,
      void(struct map_session_data *sd,int n,int pos,uint8 flag));
  MOCK_METHOD4(unequipitemack,
      void(struct map_session_data *sd,int n,int pos,int ok));
  MOCK_METHOD2(misceffect,
      void(struct block_list* bl,int type));
  MOCK_METHOD1(changeoption,
      void(struct block_list* bl));
  MOCK_METHOD1(changeoption2,
      void(struct block_list* bl));
  MOCK_METHOD4(useitemack,
      void(struct map_session_data *sd,int index,int amount,bool ok));
  MOCK_METHOD3(GlobalMessage,
      void(struct block_list* bl, const char* message,enum send_target target));
  MOCK_METHOD2(createchat,
      void(struct map_session_data* sd, int flag));
  MOCK_METHOD2(dispchat,
      void(struct chat_data* cd, int fd));
  MOCK_METHOD2(joinchatfail,
      void(struct map_session_data *sd,int flag));
  MOCK_METHOD2(joinchatok,
      void(struct map_session_data *sd,struct chat_data* cd));
  MOCK_METHOD2(addchat,
      void(struct chat_data* cd,struct map_session_data *sd));
  MOCK_METHOD2(changechatowner,
      void(struct chat_data* cd, struct map_session_data* sd));
  MOCK_METHOD2(clearchat,
      void(struct chat_data *cd,int fd));
  MOCK_METHOD3(leavechat,
      void(struct chat_data* cd, struct map_session_data* sd, bool flag));
  MOCK_METHOD1(changechatstatus,
      void(struct chat_data* cd));
  MOCK_METHOD1(refresh_storagewindow,
      void(struct map_session_data *sd));
  MOCK_METHOD1(refresh,
      void(struct map_session_data *sd));
  MOCK_METHOD2(emotion,
      void(struct block_list *bl,int type));
  MOCK_METHOD2(talkiebox,
      void(struct block_list* bl, const char* talkie));
  MOCK_METHOD1(wedding_effect,
      void(struct block_list *bl));
  MOCK_METHOD2(divorced,
      void(struct map_session_data* sd, const char* name));
  MOCK_METHOD1(callpartner,
      void(struct map_session_data *sd));
  MOCK_METHOD2(playBGM,
      void(struct map_session_data* sd, const char* name));
  MOCK_METHOD4(soundeffect,
      void(struct map_session_data* sd, struct block_list* bl, const char* name, int type));
  MOCK_METHOD4(soundeffectall,
      void(struct block_list* bl, const char* name, int type, enum send_target coverage));
  MOCK_METHOD4(parse_ActionRequest_sub,
      void(struct map_session_data *sd, int action_type, int target_id, t_tick tick));
  MOCK_METHOD1(hotkeys_send,
      void(struct map_session_data *sd));
  MOCK_METHOD2(traderequest,
      void(struct map_session_data* sd, const char* name));
  MOCK_METHOD2(tradestart,
      void(struct map_session_data* sd, uint8 type));
  MOCK_METHOD4(tradeadditem,
      void(struct map_session_data* sd, struct map_session_data* tsd, int index, int amount));
  MOCK_METHOD3(tradeitemok,
      void(struct map_session_data* sd, int index, int fail));
  MOCK_METHOD2(tradedeal_lock,
      void(struct map_session_data* sd, int fail));
  MOCK_METHOD1(tradecancelled,
      void(struct map_session_data* sd));
  MOCK_METHOD2(tradecompleted,
      void(struct map_session_data* sd, int fail));
  MOCK_METHOD1(tradeundo,
      void(struct map_session_data* sd));
  MOCK_METHOD4(storagelist,
      void(struct map_session_data* sd, struct item* items, int items_length, const char *storename));
  MOCK_METHOD3(updatestorageamount,
      void(struct map_session_data* sd, int amount, int max_amount));
  MOCK_METHOD4(storageitemadded,
      void(struct map_session_data* sd, struct item* i, int index, int amount));
  MOCK_METHOD3(storageitemremoved,
      void(struct map_session_data* sd, int index, int amount));
  MOCK_METHOD1(storageclose,
      void(struct map_session_data* sd));
  MOCK_METHOD1(skillinfoblock,
      void(struct map_session_data *sd));
  MOCK_METHOD5(skillup,
      void(struct map_session_data *sd, uint16 skill_id, int lv, int range, int upgradable));
  MOCK_METHOD3(skillinfo,
      void(struct map_session_data *sd,int skill_id, int inf));
  MOCK_METHOD2(addskill,
      void(struct map_session_data *sd, int skill_id));
  MOCK_METHOD2(deleteskill,
      void(struct map_session_data *sd, int skill_id));
  MOCK_METHOD8(skillcasting,
      void(struct block_list* bl, int src_id, int dst_id, int dst_x, int dst_y, uint16 skill_id, int property, int casttime));
  MOCK_METHOD1(skillcastcancel,
      void(struct block_list* bl));
  MOCK_METHOD4(skill_fail,
      void(struct map_session_data *sd,uint16 skill_id,enum useskill_fail_cause cause,int btype));
  MOCK_METHOD3(skill_cooldown,
      void(struct map_session_data *sd, uint16 skill_id, t_tick tick));
  MOCK_METHOD10(skill_damage,
      int(struct block_list *src,struct block_list *dst,t_tick tick,int sdelay,int ddelay,int64 sdamage,int div,uint16 skill_id,uint16 skill_lv,enum e_damage_type type));
  MOCK_METHOD5(skill_nodamage,
      bool(struct block_list *src,struct block_list *dst,uint16 skill_id,int heal,t_tick tick));
  MOCK_METHOD6(skill_poseffect,
      void(struct block_list *src,uint16 skill_id,int val,int x,int y,t_tick tick));
  MOCK_METHOD2(skill_estimation,
      void(struct map_session_data *sd,struct block_list *dst));
  MOCK_METHOD7(skill_warppoint,
      void(struct map_session_data* sd, uint16 skill_id, uint16 skill_lv, unsigned short map1, unsigned short map2, unsigned short map3, unsigned short map4));
  MOCK_METHOD2(skill_memomessage,
      void(struct map_session_data* sd, int type));
  MOCK_METHOD2(skill_teleportmessage,
      void(struct map_session_data *sd, int type));
  MOCK_METHOD3(skill_produce_mix_list,
      void(struct map_session_data *sd, int skill_id, int trigger));
  MOCK_METHOD5(cooking_list,
      void(struct map_session_data *sd, int trigger, uint16 skill_id, int qty, int list_type));
  MOCK_METHOD3(produceeffect,
      void(struct map_session_data* sd,int flag, unsigned short nameid));
  MOCK_METHOD4(getareachar_skillunit,
      void(struct block_list *bl, struct skill_unit *unit, enum send_target target, bool visible));
  MOCK_METHOD1(skill_delunit,
      void(struct skill_unit *unit));
  MOCK_METHOD1(skillunit_update,
      void(struct block_list* bl));
  MOCK_METHOD2(autospell,
      void(struct map_session_data *sd,uint16 skill_lv));
  MOCK_METHOD2(devotion,
      void(struct block_list *src, struct map_session_data *tsd));
  MOCK_METHOD1(spiritball,
      void(struct block_list *bl));
  MOCK_METHOD2(combo_delay,
      void(struct block_list *bl,t_tick wait));
  MOCK_METHOD3(bladestop,
      void(struct block_list *src, int dst_id, int active));
  MOCK_METHOD6(changemapcell,
      void(int fd, int16 m, int x, int y, int type, enum send_target target));
  MOCK_METHOD7(status_change,
      void(struct block_list *bl, int type, int flag, t_tick tick, int val1, int val2, int val3));
  MOCK_METHOD8(efst_status_change,
      void(struct block_list *bl, int tid, enum send_target target, int type, t_tick tick, int val1, int val2, int val3));
  MOCK_METHOD3(efst_status_change_sub,
      void(struct block_list *tbl, struct block_list *bl, enum send_target target));
  MOCK_METHOD5(wis_message,
      void(struct map_session_data* sd, const char* nick, const char* mes, int mes_len, int gmlvl));
  MOCK_METHOD2(wis_end,
      void(int fd, int result));
  MOCK_METHOD3(solved_charname,
      void(int fd, int charid, const char* name));
  MOCK_METHOD3(name,
      void(struct block_list* src, struct block_list *bl, send_target target));
  MOCK_METHOD2(use_card,
      void(struct map_session_data *sd,int idx));
  MOCK_METHOD4(insert_card,
      void(struct map_session_data *sd,int idx_equip,int idx_card,int flag));
  MOCK_METHOD1(inventorylist,
      void(struct map_session_data *sd));
  MOCK_METHOD1(equiplist,
      void(struct map_session_data *sd));
  MOCK_METHOD4(cart_additem,
      void(struct map_session_data *sd,int n,int amount,int fail));
  MOCK_METHOD2(cart_additem_ack,
      void(struct map_session_data *sd, uint8 flag));
  MOCK_METHOD3(cart_delitem,
      void(struct map_session_data *sd,int n,int amount));
  MOCK_METHOD1(cartlist,
      void(struct map_session_data *sd));
  MOCK_METHOD1(clearcart,
      void(int fd));
  MOCK_METHOD1(item_identify_list,
      void(struct map_session_data *sd));
  MOCK_METHOD3(item_identified,
      void(struct map_session_data *sd,int idx,int flag));
  MOCK_METHOD3(item_repair_list,
      void(struct map_session_data *sd, struct map_session_data *dstsd, int lv));
  MOCK_METHOD3(item_repaireffect,
      void(struct map_session_data *sd, int idx, int flag));
  MOCK_METHOD2(item_damaged,
      void(struct map_session_data* sd, unsigned short position));
  MOCK_METHOD1(item_refine_list,
      void(struct map_session_data *sd));
  MOCK_METHOD3(hat_effects,
      void(struct map_session_data* sd, struct block_list* bl, enum send_target target));
  MOCK_METHOD3(hat_effect_single,
      void(struct map_session_data* sd, uint16 effectId, bool enable));
  MOCK_METHOD3(item_skill,
      void(struct map_session_data *sd,uint16 skill_id,uint16 skill_lv));
  MOCK_METHOD1(mvp_effect,
      void(struct map_session_data *sd));
  MOCK_METHOD2(mvp_item,
      void(struct map_session_data *sd, unsigned short nameid));
  MOCK_METHOD2(mvp_exp,
      void(struct map_session_data *sd, unsigned int exp));
  MOCK_METHOD1(mvp_noitem,
      void(struct map_session_data* sd));
  MOCK_METHOD2(changed_dir,
      void(struct block_list *bl, enum send_target target));
  MOCK_METHOD2(openvendingreq,
      void(struct map_session_data* sd, int num));
  MOCK_METHOD3(showvendingboard,
      void(struct block_list* bl, const char* message, int fd));
  MOCK_METHOD2(closevendingboard,
      void(struct block_list* bl, int fd));
  MOCK_METHOD3(vendinglist,
      void(struct map_session_data* sd, int id, struct s_vending* vending));
  MOCK_METHOD4(buyvending,
      void(struct map_session_data* sd, int index, int amount, int fail));
  MOCK_METHOD3(openvending,
      void(struct map_session_data* sd, int id, struct s_vending* vending));
  MOCK_METHOD5(vendingreport,
      void(struct map_session_data* sd, int index, int amount, uint32 char_id, int zeny));
  MOCK_METHOD2(movetoattack,
      void(struct map_session_data *sd,struct block_list *bl));
  MOCK_METHOD2(party_created,
      void(struct map_session_data *sd,int result));
  MOCK_METHOD2(party_member_info,
      void(struct party_data *p, struct map_session_data *sd));
  MOCK_METHOD2(party_info,
      void(struct party_data* p, struct map_session_data *sd));
  MOCK_METHOD2(party_invite,
      void(struct map_session_data *sd,struct map_session_data *tsd));
  MOCK_METHOD3(party_invite_reply,
      void(struct map_session_data* sd, const char* nick, enum e_party_invite_reply reply));
  MOCK_METHOD3(party_option,
      void(struct party_data *p,struct map_session_data *sd,int flag));
  MOCK_METHOD5(party_withdraw,
      void(struct map_session_data *sd, uint32 account_id, const char* name, enum e_party_member_withdraw result, enum send_target target));
  MOCK_METHOD4(party_message,
      void(struct party_data* p, uint32 account_id, const char* mes, int len));
  MOCK_METHOD1(party_xy,
      void(struct map_session_data *sd));
  MOCK_METHOD2(party_xy_single,
      void(int fd, struct map_session_data *sd));
  MOCK_METHOD1(party_hp,
      void(struct map_session_data *sd));
  MOCK_METHOD4(hpmeter_single,
      void(int fd, int id, unsigned int hp, unsigned int maxhp));
  MOCK_METHOD1(party_job_and_level,
      void(struct map_session_data *sd));
  MOCK_METHOD1(party_dead,
      void(struct map_session_data *sd));
  MOCK_METHOD2(guild_created,
      void(struct map_session_data *sd,int flag));
  MOCK_METHOD1(guild_belonginfo,
      void(struct map_session_data *sd));
  MOCK_METHOD1(guild_masterormember,
      void(struct map_session_data *sd));
  MOCK_METHOD1(guild_basicinfo,
      void(struct map_session_data *sd));
  MOCK_METHOD1(guild_allianceinfo,
      void(struct map_session_data *sd));
  MOCK_METHOD1(guild_memberlist,
      void(struct map_session_data *sd));
  MOCK_METHOD1(guild_skillinfo,
      void(struct map_session_data* sd));
  MOCK_METHOD1(guild_send_onlineinfo,
      void(struct map_session_data *sd));
  MOCK_METHOD3(guild_memberlogin_notice,
      void(struct guild *g,int idx,int flag));
  MOCK_METHOD2(guild_invite,
      void(struct map_session_data *sd,struct guild *g));
  MOCK_METHOD2(guild_inviteack,
      void(struct map_session_data *sd,int flag));
  MOCK_METHOD3(guild_leave,
      void(struct map_session_data *sd,const char *name,const char *mes));
  MOCK_METHOD4(guild_expulsion,
      void(struct map_session_data* sd, const char* name, const char* mes, uint32 account_id));
  MOCK_METHOD2(guild_positionchanged,
      void(struct guild *g,int idx));
  MOCK_METHOD2(guild_memberpositionchanged,
      void(struct guild *g,int idx));
  MOCK_METHOD2(guild_emblem,
      void(struct map_session_data *sd,struct guild *g));
  MOCK_METHOD1(guild_emblem_area,
      void(struct block_list* bl));
  MOCK_METHOD1(guild_notice,
      void(struct map_session_data* sd));
  MOCK_METHOD4(guild_message,
      void(struct guild *g,uint32 account_id,const char *mes,int len));
  MOCK_METHOD3(guild_reqalliance,
      void(struct map_session_data *sd,uint32 account_id,const char *name));
  MOCK_METHOD2(guild_allianceack,
      void(struct map_session_data *sd,int flag));
  MOCK_METHOD3(guild_delalliance,
      void(struct map_session_data *sd,int guild_id,int flag));
  MOCK_METHOD2(guild_oppositionack,
      void(struct map_session_data *sd,int flag));
  MOCK_METHOD2(guild_broken,
      void(struct map_session_data *sd,int flag));
  MOCK_METHOD1(guild_xy,
      void(struct map_session_data *sd));
  MOCK_METHOD2(guild_xy_single,
      void(int fd, struct map_session_data *sd));
  MOCK_METHOD1(guild_xy_remove,
      void(struct map_session_data *sd));
  MOCK_METHOD1(bg_hp,
      void(struct map_session_data *sd));
  MOCK_METHOD1(bg_xy,
      void(struct map_session_data *sd));
  MOCK_METHOD1(bg_xy_remove,
      void(struct map_session_data *sd));
  MOCK_METHOD5(bg_message,
      void(struct battleground_data *bg, int src_id, const char *name, const char *mes, int len));
  MOCK_METHOD1(bg_updatescore,
      void(int16 m));
  MOCK_METHOD1(bg_updatescore_single,
      void(struct map_session_data *sd));
  MOCK_METHOD1(sendbgemblem_area,
      void(struct map_session_data *sd));
  MOCK_METHOD2(sendbgemblem_single,
      void(int fd, struct map_session_data *sd));
  MOCK_METHOD2(instance_create,
      void(unsigned short instance_id, int num));
  MOCK_METHOD2(instance_changewait,
      void(unsigned short instance_id, int num));
  MOCK_METHOD3(instance_status,
      void(unsigned short instance_id, unsigned int limit1, unsigned int limit2));
  MOCK_METHOD3(instance_changestatus,
      void(unsigned int instance_id, int type, unsigned int limit));
  MOCK_METHOD1(font,
      void(struct map_session_data *sd));
  MOCK_METHOD2(displaymessage,
      void(const int fd, const char* mes));
  MOCK_METHOD4(disp_message,
      void(struct block_list* src, const char* mes, int len, enum send_target target));
  MOCK_METHOD5(broadcast,
      void(struct block_list* bl, const char* mes, int len, int type, enum send_target target));
  MOCK_METHOD9(broadcast2,
      void(struct block_list* bl, const char* mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, enum send_target target));
  MOCK_METHOD3(heal,
      void(int fd,int type,int val));
  MOCK_METHOD2(resurrection,
      void(struct block_list *bl,int type));
  MOCK_METHOD3(map_property,
      void(struct block_list *bl, enum map_property property, enum send_target t));
  MOCK_METHOD4(pvpset,
      void(struct map_session_data *sd, int pvprank, int pvpnum,int type));
  MOCK_METHOD2(map_property_mapall,
      void(int map, enum map_property property));
  MOCK_METHOD4(refine,
      void(int fd, int fail, int index, int val));
  MOCK_METHOD3(upgrademessage,
      void(int fd, int result, unsigned short item_id));
  MOCK_METHOD1(catch_process,
      void(struct map_session_data *sd));
  MOCK_METHOD2(pet_roulette,
      void(struct map_session_data *sd,int data));
  MOCK_METHOD1(sendegg,
      void(struct map_session_data *sd));
  MOCK_METHOD1(send_petstatus,
      void(struct map_session_data *sd));
  MOCK_METHOD2(pet_emotion,
      void(struct pet_data *pd,int param));
  MOCK_METHOD3(pet_food,
      void(struct map_session_data *sd,int foodid,int fail));
  MOCK_METHOD1(friendslist_send,
      void(struct map_session_data *sd));
  MOCK_METHOD1(weather,
      void(int16 m));
  MOCK_METHOD3(specialeffect,
      void(struct block_list* bl, int type, enum send_target target));
  MOCK_METHOD3(specialeffect_single,
      void(struct block_list* bl, int type, int fd));
  MOCK_METHOD6(messagecolor_target,
      void(struct block_list *bl, unsigned long color, const char *msg, bool rgb2bgr, enum send_target type, struct map_session_data *sd));
  MOCK_METHOD4(specialeffect_value,
      void(struct block_list* bl, int effect_id, int num, send_target target));
  MOCK_METHOD2(GM_kickack,
      void(struct map_session_data *sd, int id));
  MOCK_METHOD2(GM_kick,
      void(struct map_session_data *sd,struct map_session_data *tsd));
  MOCK_METHOD2(manner_message,
      void(struct map_session_data* sd, uint32 type));
  MOCK_METHOD3(GM_silence,
      void(struct map_session_data* sd, struct map_session_data* tsd, uint8 type));
  MOCK_METHOD3(disp_overhead_,
      void(struct block_list *bl, const char* mes, enum send_target flag));
  MOCK_METHOD3(get_weapon_view,
      void(struct map_session_data* sd, unsigned short *rhand, unsigned short *lhand));
  MOCK_METHOD1(party_xy_remove,
      void(struct map_session_data *sd));
  MOCK_METHOD2(gospel_info,
      void(struct map_session_data *sd, int type));
  MOCK_METHOD3(feel_req,
      void(int fd, struct map_session_data *sd, uint16 skill_lv));
  MOCK_METHOD5(starskill,
      void(struct map_session_data* sd, const char* mapname, int monster_id, unsigned char star, unsigned char result));
  MOCK_METHOD3(feel_info,
      void(struct map_session_data* sd, unsigned char feel_level, unsigned char type));
  MOCK_METHOD4(hate_info,
      void(struct map_session_data *sd, unsigned char hate_level,int class_, unsigned char type));
  MOCK_METHOD3(mission_info,
      void(struct map_session_data *sd, int mob_id, unsigned char progress));
  MOCK_METHOD1(feel_hate_reset,
      void(struct map_session_data *sd));
  MOCK_METHOD3(hominfo,
      void(struct map_session_data *sd, struct homun_data *hd, int flag));
  MOCK_METHOD1(homskillinfoblock,
      int(struct map_session_data *sd));
  MOCK_METHOD2(homskillup,
      void(struct map_session_data *sd, uint16 skill_id));
  MOCK_METHOD3(hom_food,
      int(struct map_session_data *sd,int foodid,int fail));
  MOCK_METHOD3(send_homdata,
      void(struct map_session_data *sd, int state, int param));
  MOCK_METHOD3(configuration,
      void(struct map_session_data* sd, enum e_config_type type, bool enabled));
  MOCK_METHOD2(partytickack,
      void(struct map_session_data* sd, bool flag));
  MOCK_METHOD2(viewequip_ack,
      void(struct map_session_data* sd, struct map_session_data* tsd));
  MOCK_METHOD1(equipcheckbox,
      void(struct map_session_data* sd));
  MOCK_METHOD2(msg,
      void(struct map_session_data* sd, unsigned short id));
  MOCK_METHOD3(msg_value,
      void(struct map_session_data* sd, unsigned short id, int value));
  MOCK_METHOD3(msg_skill,
      void(struct map_session_data* sd, uint16 skill_id, int msg_id));
  MOCK_METHOD1(quest_send_list,
      void(struct map_session_data * sd));
  MOCK_METHOD1(quest_send_mission,
      void(struct map_session_data * sd));
  MOCK_METHOD2(quest_add,
      void(struct map_session_data * sd, struct quest * qd));
  MOCK_METHOD2(quest_delete,
      void(struct map_session_data * sd, int quest_id));
  MOCK_METHOD3(quest_update_status,
      void(struct map_session_data * sd, int quest_id, bool active));
  MOCK_METHOD3(quest_update_objective,
      void(struct map_session_data * sd, struct quest * qd, int mobid));
  MOCK_METHOD4(quest_show_event,
      void(struct map_session_data *sd, struct block_list *bl, short state, short color));
  MOCK_METHOD5(displayexp,
      void(struct map_session_data *sd, unsigned int exp, char type, bool quest, bool lost));
  MOCK_METHOD4(send,
      int(const uint8* buf, int len, struct block_list* bl, enum send_target type));
  MOCK_METHOD2(Mail_window,
      void(int fd, int flag));
  MOCK_METHOD2(Mail_read,
      void(struct map_session_data *sd, int mail_id));
  MOCK_METHOD3(mail_delete,
      void(struct map_session_data* sd, struct mail_message *msg, bool success));
  MOCK_METHOD3(Mail_return,
      void(int fd, int mail_id, short fail));
  MOCK_METHOD2(Mail_send,
      void(struct map_session_data* sd, enum mail_send_result result));
  MOCK_METHOD4(Mail_new,
      void(struct map_session_data* sd, int mail_id, const char *sender, const char *title));
  MOCK_METHOD3(Mail_refreshinbox,
      void(struct map_session_data *sd,enum mail_inbox_type type,int64 mailID));
  MOCK_METHOD4(mail_getattachment,
      void(struct map_session_data* sd, struct mail_message *msg, uint8 result, enum mail_attachment_type type));
  MOCK_METHOD5(Mail_Receiver_Ack,
      void(struct map_session_data* sd, uint32 char_id, short class_, uint32 level, const char* name));
  MOCK_METHOD4(mail_removeitem,
      void(struct map_session_data* sd, bool success, int index, int amount));
  MOCK_METHOD1(Auction_openwindow,
      void(struct map_session_data *sd));
  MOCK_METHOD4(Auction_results,
      void(struct map_session_data *sd, short count, short pages, uint8 *buf));
  MOCK_METHOD2(Auction_message,
      void(int fd, unsigned char flag));
  MOCK_METHOD2(Auction_close,
      void(int fd, unsigned char flag));
  MOCK_METHOD3(bossmapinfo,
      void(struct map_session_data *sd, struct mob_data *md, enum e_bossmap_info flag));
  MOCK_METHOD2(cashshop_show,
      void(struct map_session_data *sd, struct npc_data *nd));
  MOCK_METHOD2(Adopt_reply,
      void(struct map_session_data *sd, int type));
  MOCK_METHOD3(Adopt_request,
      void(struct map_session_data *sd, struct map_session_data *src, int p_id));
  MOCK_METHOD1(mercenary_info,
      void(struct map_session_data *sd));
  MOCK_METHOD1(mercenary_skillblock,
      void(struct map_session_data *sd));
  MOCK_METHOD2(mercenary_message,
      void(struct map_session_data* sd, int message));
  MOCK_METHOD2(mercenary_updatestatus,
      void(struct map_session_data *sd, int type));
  MOCK_METHOD3(rental_time,
      void(int fd, unsigned short nameid, int seconds));
  MOCK_METHOD3(rental_expired,
      void(int fd, int index, unsigned short nameid));
  MOCK_METHOD3(readbook,
      void(int fd, int book_id, int page));
  MOCK_METHOD2(party_show_picker,
      void(struct map_session_data * sd, struct item * item_data));
  MOCK_METHOD3(progressbar,
      void(struct map_session_data * sd, unsigned long color, unsigned int second));
  MOCK_METHOD1(progressbar_abort,
      void(struct map_session_data * sd));
  MOCK_METHOD2(progressbar_npc,
      void(struct npc_data *nd, struct map_session_data* sd));
  MOCK_METHOD2(PartyBookingRegisterAck,
      void(struct map_session_data *sd, int flag));
  MOCK_METHOD2(PartyBookingDeleteAck,
      void(struct map_session_data* sd, int flag));
  MOCK_METHOD4(PartyBookingSearchAck,
      void(int fd, struct party_booking_ad_info** results, int count, bool more_result));
  MOCK_METHOD2(PartyBookingUpdateNotify,
      void(struct map_session_data* sd, struct party_booking_ad_info* pb_ad));
  MOCK_METHOD2(PartyBookingDeleteNotify,
      void(struct map_session_data* sd, int index));
  MOCK_METHOD2(PartyBookingInsertNotify,
      void(struct map_session_data* sd, struct party_booking_ad_info* pb_ad));
  MOCK_METHOD2(bank_deposit,
      void(struct map_session_data *sd, enum e_BANKING_DEPOSIT_ACK reason));
  MOCK_METHOD2(bank_withdraw,
      void(struct map_session_data *sd,enum e_BANKING_WITHDRAW_ACK reason));
  MOCK_METHOD3(showdigit,
      void(struct map_session_data* sd, unsigned char type, int value));
  MOCK_METHOD1(buyingstore_open,
      void(struct map_session_data* sd));
  MOCK_METHOD3(buyingstore_open_failed,
      void(struct map_session_data* sd, unsigned short result, unsigned int weight));
  MOCK_METHOD1(buyingstore_myitemlist,
      void(struct map_session_data* sd));
  MOCK_METHOD1(buyingstore_entry,
      void(struct map_session_data* sd));
  MOCK_METHOD2(buyingstore_entry_single,
      void(struct map_session_data* sd, struct map_session_data* pl_sd));
  MOCK_METHOD1(buyingstore_disappear_entry,
      void(struct map_session_data* sd));
  MOCK_METHOD2(buyingstore_disappear_entry_single,
      void(struct map_session_data* sd, struct map_session_data* pl_sd));
  MOCK_METHOD2(buyingstore_itemlist,
      void(struct map_session_data* sd, struct map_session_data* pl_sd));
  MOCK_METHOD2(buyingstore_trade_failed_buyer,
      void(struct map_session_data* sd, short result));
  MOCK_METHOD5(buyingstore_update_item,
      void(struct map_session_data* sd, unsigned short nameid, unsigned short amount, uint32 char_id, int zeny));
  MOCK_METHOD4(buyingstore_delete_item,
      void(struct map_session_data* sd, short index, unsigned short amount, int price));
  MOCK_METHOD3(buyingstore_trade_failed_seller,
      void(struct map_session_data* sd, short result, unsigned short nameid));
  MOCK_METHOD1(search_store_info_ack,
      void(struct map_session_data* sd));
  MOCK_METHOD2(search_store_info_failed,
      void(struct map_session_data* sd, unsigned char reason));
  MOCK_METHOD1(open_search_store_info,
      void(struct map_session_data* sd));
  MOCK_METHOD3(search_store_info_click_ack,
      void(struct map_session_data* sd, short x, short y));
  MOCK_METHOD3(cashshop_result,
      void(struct map_session_data* sd, unsigned short item_id, uint16 result));
  MOCK_METHOD1(cashshop_open,
      void(struct map_session_data* sd));
  MOCK_METHOD2(display_pinfo,
      void(struct map_session_data *sd, int type));
  MOCK_METHOD1(roulette_open,
      void(struct map_session_data* sd));
  MOCK_METHOD1(elementalconverter_list,
      int(struct map_session_data *sd));
  MOCK_METHOD2(millenniumshield,
      void(struct block_list *bl, short shields));
  MOCK_METHOD1(spellbook_list,
      int(struct map_session_data *sd));
  MOCK_METHOD4(magicdecoy_list,
      int(struct map_session_data *sd, uint16 skill_lv, short x, short y));
  MOCK_METHOD2(poison_list,
      int(struct map_session_data *sd, uint16 skill_lv));
  MOCK_METHOD1(autoshadowspell_list,
      int(struct map_session_data *sd));
  MOCK_METHOD3(skill_itemlistwindow,
      int(struct map_session_data *sd, uint16 skill_id, uint16 skill_lv));
  MOCK_METHOD1(elemental_info,
      void(struct map_session_data *sd));
  MOCK_METHOD2(elemental_updatestatus,
      void(struct map_session_data *sd, int type));
  MOCK_METHOD1(spiritcharm,
      void(struct map_session_data *sd));
  MOCK_METHOD3(snap,
      void(struct block_list *bl, short x, short y));
  MOCK_METHOD2(monster_hp_bar,
      void(struct mob_data* md, int fd));
  MOCK_METHOD1(clan_basicinfo,
      void(struct map_session_data *sd));
  MOCK_METHOD3(clan_message,
      void(struct clan *clan,const char *mes,int len));
  MOCK_METHOD1(clan_onlinecount,
      void(struct clan* clan));
  MOCK_METHOD1(clan_leave,
      void(struct map_session_data* sd));
  MOCK_METHOD3(sale_start,
      void(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target));
  MOCK_METHOD3(sale_end,
      void(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target));
  MOCK_METHOD3(sale_amount,
      void(struct sale_item_data* sale_item, struct block_list* bl, enum send_target target));
  MOCK_METHOD1(sale_open,
      void(struct map_session_data* sd));
  MOCK_METHOD3(channel_msg,
      void(struct Channel *channel, const char *msg, unsigned long color));
  MOCK_METHOD2(ranklist,
      void(struct map_session_data *sd, int16 rankingType));
  MOCK_METHOD3(update_rankingpoint,
      void(struct map_session_data *sd, int rankingtype, int point));
  MOCK_METHOD3(crimson_marker,
      void(struct map_session_data *sd, struct block_list *bl, bool remove));
  MOCK_METHOD3(showscript,
      void(struct block_list* bl, const char* message, enum send_target flag));
  MOCK_METHOD3(party_leaderchanged,
      void(struct map_session_data *sd, int prev_leader_aid, int new_leader_aid));
  MOCK_METHOD3(account_name,
      void(int fd, uint32 account_id, const char* accname));
  MOCK_METHOD2(notify_bindOnEquip,
      void(struct map_session_data *sd, int n));
  MOCK_METHOD1(merge_item_open,
      void(struct map_session_data *sd));
  MOCK_METHOD4(broadcast_obtain_special_item,
      void(const char *char_name, unsigned short nameid, unsigned short container, enum BROADCASTING_SPECIAL_ITEM_OBTAIN type));
  MOCK_METHOD2(dressing_room,
      void(struct map_session_data *sd, int flag));
  MOCK_METHOD7(navigateTo,
      void(struct map_session_data *sd, const char* mapname, uint16 x, uint16 y, uint8 flag, bool hideWindow, uint16 mob_id));
  MOCK_METHOD1(SelectCart,
      void(struct map_session_data *sd));
  MOCK_METHOD1(achievement_list_all,
      void(struct map_session_data *sd));
  MOCK_METHOD3(achievement_update,
      void(struct map_session_data *sd, struct achievement *ach, int count));
  MOCK_METHOD3(achievement_reward_ack,
      void(int fd, unsigned char result, int ach_id));
  MOCK_METHOD3(ui_open,
      void(struct map_session_data *sd, enum out_ui_type ui_type, int32 data));
  MOCK_METHOD2(attendence_response,
      void(struct map_session_data *sd, int32 data));
  MOCK_METHOD1(weight_limit,
      void(struct map_session_data* sd));
  MOCK_METHOD3(guild_storage_log,
      void(struct map_session_data* sd, std::vector<struct guild_log_entry>& log, enum e_guild_storage_log result));
  MOCK_METHOD5(ccamerainfo,
      void(map_session_data*, bool, float, float, float));
  MOCK_METHOD1(equipswitch_list,
      void(struct map_session_data* sd));
  MOCK_METHOD4(equipswitch_add,
      void(struct map_session_data* sd,uint16 index, uint32 pos, bool failed));
  MOCK_METHOD4(equipswitch_remove,
      void(struct map_session_data* sd, uint16 index, uint32 pos, bool failed));
  MOCK_METHOD2(equipswitch_reply,
      void(struct map_session_data* sd, bool failed));
  
 // void ccamerainfo(map_session_data, bool, float, float, float) 
 // {
 // }
};
#endif
