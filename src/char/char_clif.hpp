// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_CLIF_HPP
#define CHAR_CLIF_HPP

#include <common/cbasetypes.hpp>
#include <common/timer.hpp> //time_t

struct char_session_data;
enum pincode_state : uint8;

void chclif_moveCharSlotReply( int32 fd, struct char_session_data* sd, uint16 index, int16 reason );
int32 chclif_parse_moveCharSlot( int32 fd, struct char_session_data* sd);
#if PACKETVER_SUPPORTS_PINCODE
void chclif_pincode_sendstate( int32 fd, struct char_session_data* sd, enum pincode_state state );
int32 chclif_parse_reqpincode_window(int32 fd, struct char_session_data* sd);
int32 chclif_parse_pincode_check( int32 fd, struct char_session_data* sd );
int32 chclif_parse_pincode_change( int32 fd, struct char_session_data* sd );
int32 chclif_parse_pincode_setnew( int32 fd, struct char_session_data* sd );
#endif

void chclif_reject(int32 fd, uint8 errCode);
void chclif_refuse_delchar(int32 fd, uint8 errCode);
void chclif_charlist_notify( int32 fd, struct char_session_data* sd );
void chclif_block_character( int32 fd, struct char_session_data* sd );
int32 chclif_mmo_send006b(int32 fd, struct char_session_data* sd);
void chclif_mmo_send082d(int32 fd, struct char_session_data* sd);
void chclif_mmo_send099d(int32 fd, struct char_session_data *sd);
void chclif_mmo_char_send(int32 fd, struct char_session_data* sd);
void chclif_send_auth_result(int32 fd,char result);
void chclif_char_delete2_ack(int32 fd, uint32 char_id, uint32 result, time_t delete_date);
void chclif_char_delete2_accept_ack(int32 fd, uint32 char_id, uint32 result);
void chclif_char_delete2_cancel_ack(int32 fd, uint32 char_id, uint32 result);

int32 chclif_parse_char_delete2_req(int32 fd, struct char_session_data* sd);
int32 chclif_parse_char_delete2_accept(int32 fd, struct char_session_data* sd);
int32 chclif_parse_char_delete2_cancel(int32 fd, struct char_session_data* sd);

int32 chclif_parse_maplogin(int32 fd);
int32 chclif_parse_reqtoconnect(int32 fd, struct char_session_data* sd,uint32 ipl);
int32 chclif_parse_req_charlist(int32 fd, struct char_session_data* sd);
int32 chclif_parse_charselect(int32 fd, struct char_session_data* sd,uint32 ipl);
int32 chclif_parse_createnewchar(int32 fd, struct char_session_data* sd,int32 cmd);
int32 chclif_parse_delchar(int32 fd,struct char_session_data* sd, int32 cmd);
int32 chclif_parse_keepalive(int32 fd);
int32 chclif_parse_reqrename(int32 fd, struct char_session_data* sd);
int32 chclif_parse_ackrename(int32 fd, struct char_session_data* sd);
int32 chclif_ack_captcha(int32 fd);
int32 chclif_parse_reqcaptcha(int32 fd);
int32 chclif_parse_chkcaptcha(int32 fd);
void chclif_block_character( int32 fd, struct char_session_data* sd);

int32 chclif_parse(int32 fd);

#endif /* CHAR_CLIF_HPP */
