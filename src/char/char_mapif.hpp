// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_MAPIF_HPP
#define CHAR_MAPIF_HPP

#include <common/cbasetypes.hpp>

int32 chmapif_sendall(unsigned char *buf, uint32 len);
int32 chmapif_sendallwos(int32 sfd, unsigned char *buf, uint32 len);
int32 chmapif_send(int32 fd, unsigned char *buf, uint32 len);
int32 chmapif_send_fame_list(int32 fd);
void chmapif_update_fame_list(int32 type, int32 index, int32 fame);
void chmapif_sendall_playercount(int32 users);
int32 chmapif_parse_getmapname(int32 fd, int32 id);
int32 chmapif_parse_askscdata(int32 fd);
int32 chmapif_parse_getusercount(int32 fd, int32 id);
int32 chmapif_parse_regmapuser(int32 fd, int32 id);
int32 chmapif_parse_reqsavechar(int32 fd, int32 id);
int32 chmapif_parse_authok(int32 fd);
int32 chmapif_parse_req_saveskillcooldown(int32 fd);
int32 chmapif_parse_req_skillcooldown(int32 fd);
int32 chmapif_parse_reqchangemapserv(int32 fd);
int32 chmapif_parse_askrmfriend(int32 fd);
int32 chmapif_parse_reqcharname(int32 fd);
int32 chmapif_parse_reqnewemail(int32 fd);
int32 chmapif_parse_fwlog_changestatus(int32 fd);
int32 chmapif_parse_updfamelist(int32 fd);
void chmapif_send_ackdivorce(int32 partner_id1, int32 partner_id2);
int32 chmapif_parse_reqdivorce(int32 fd);
int32 chmapif_parse_setcharoffline(int32 fd);
int32 chmapif_parse_setalloffline(int32 fd, int32 id);
int32 chmapif_parse_setcharonline(int32 fd, int32 id);
int32 chmapif_parse_reqfamelist(int32 fd);
int32 chmapif_parse_save_scdata(int32 fd);
int32 chmapif_parse_keepalive(int32 fd);
int32 chmapif_parse_reqauth(int32 fd, int32 id);
int32 chmapif_parse_updmapip(int32 fd, int32 id);

int32 chmapif_vipack(int32 mapfd, uint32 aid, uint32 vip_time, uint32 groupid, uint8 flag);
int32 chmapif_parse_reqcharban(int32 fd);
int32 chmapif_parse_reqcharunban(int32 fd);
int32 chmapif_bonus_script_get(int32 fd);
int32 chmapif_bonus_script_save(int32 fd);

void chmapif_connectack(int32 fd, uint8 errCode);
void chmapif_charselres(int32 fd, uint32 aid, uint8 res);
void chmapif_changemapserv_ack(int32 fd, bool nok);

int32 chmapif_parse(int32 fd);
int32 chmapif_init(int32 fd);
void chmapif_server_init(int32 id);
void chmapif_server_destroy(int32 id);
void do_init_chmapif(void);
void chmapif_server_reset(int32 id);
void chmapif_on_disconnect(int32 id);
void do_final_chmapif(void);

#endif /* CHAR_MAPIF_HPP */
