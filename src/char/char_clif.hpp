// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_CLIF_HPP
#define CHAR_CLIF_HPP

#include <common/cbasetypes.hpp>
#include <common/timer.hpp> //time_t

struct char_session_data;
enum pincode_state : uint8;

#if PACKETVER_SUPPORTS_PINCODE
void chclif_pincode_sendstate( int32 fd, char_session_data& sd, enum pincode_state state );
#endif

void chclif_reject(int32 fd, uint8 errCode);
void chclif_refuse_delchar(int32 fd, uint8 errCode);
void chclif_charlist_notify( int32 fd, struct char_session_data* sd );
void chclif_mmo_char_send( int32 fd, char_session_data& sd );
void chclif_send_auth_result(int32 fd,char result);
void chclif_char_delete2_ack(int32 fd, uint32 char_id, uint32 result, time_t delete_date);
void chclif_char_delete2_accept_ack(int32 fd, uint32 char_id, uint32 result);
void chclif_char_delete2_cancel_ack(int32 fd, uint32 char_id, uint32 result);

int32 chclif_parse(int32 fd);

#endif /* CHAR_CLIF_HPP */
