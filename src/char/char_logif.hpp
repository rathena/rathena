// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_LOGIF_HPP
#define CHAR_LOGIF_HPP

#include <common/cbasetypes.hpp>
#include <common/timer.hpp>

struct char_session_data;

#if PACKETVER_SUPPORTS_PINCODE
void chlogif_pincode_notifyLoginPinError( uint32 account_id );
void chlogif_pincode_notifyLoginPinUpdate( uint32 account_id, char* pin );
void chlogif_pincode_start(int32 fd, struct char_session_data* sd);
#endif
TIMER_FUNC(chlogif_send_acc_tologin);
TIMER_FUNC(chlogif_broadcast_user_count);
void chlogif_send_usercount(int32 users);
void chlogif_upd_global_accreg(uint32 account_id, uint32 char_id);
void chlogif_prepsend_global_accreg(void);
void chlogif_send_global_accreg(const char *key, uint32 index, int64 int_value, const char* string_value, bool is_string);
void chlogif_request_accreg2(uint32 account_id, uint32 char_id);
void chlogif_send_reqaccdata(int32 fd, struct char_session_data *sd);
void chlogif_send_setacconline(int32 aid);
void chlogif_send_setallaccoffline(int32 fd);
void chlogif_send_setaccoffline(int32 fd, int32 aid);

int32 chlogif_parse_ackconnect(int32 fd);
int32 chlogif_parse_ackaccreq(int32 fd);
int32 chlogif_parse_reqaccdata(int32 fd);
int32 chlogif_parse_keepalive(int32 fd);
void chlogif_parse_change_sex_sub(int32 sex, int32 acc, int32 char_id, int32 class_, int32 guild_id);
int32 chlogif_parse_ackchangesex(int32 fd);
int32 chlogif_parse_ackchangecharsex(int32 char_id, int32 sex);
int32 chlogif_parse_ack_global_accreg(int32 fd);
int32 chlogif_parse_accbannotification(int32 fd);
int32 chlogif_parse_askkick(int32 fd);
int32 chlogif_parse_updip(int32 fd);

int32 chlogif_parse_vipack(int32 fd);
int32 chlogif_reqvipdata(uint32 aid, uint8 flag, int32 timediff, int32 mapfd);
int32 chlogif_req_accinfo(int32 fd, int32 u_fd, int32 u_aid, int32 account_id);

int32 chlogif_parse(int32 fd);

int32 chlogif_isconnected();
TIMER_FUNC(chlogif_check_connect_logserver);
void do_init_chlogif(void);
void chlogif_reset(void);
void chlogif_check_shutdown(void);
void chlogif_on_disconnect(void);
void chlogif_on_ready(void);
void do_final_chlogif(void);

#define loginif_check(a) { if(!chlogif_isconnected()) return a; }

#endif /* CHAR_LOGIF_HPP */
