// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MAIL_HPP
#define MAIL_HPP

#include <common/mmo.hpp>

enum mail_attach_result {
	MAIL_ATTACH_SUCCESS = 0,
#if PACKETVER >= 20150513
	MAIL_ATTACH_WEIGHT = 1,
	MAIL_ATTACH_ERROR = 2,
	MAIL_ATTACH_SPACE = 3,
	MAIL_ATTACH_UNTRADEABLE = 4,
#else
	MAIL_ATTACH_WEIGHT = 1,
	MAIL_ATTACH_ERROR = 1,
	MAIL_ATTACH_SPACE = 1,
	MAIL_ATTACH_UNTRADEABLE = 1,
#endif

	// Unofficial
	MAIL_ATTACH_EQUIPSWITCH = 99,
};

void mail_clear(map_session_data *sd);
int32 mail_removeitem(map_session_data *sd, int16 flag, int32 idx, int32 amount);
bool mail_removezeny(map_session_data *sd, bool flag);
enum mail_attach_result mail_setitem(map_session_data *sd, int16 idx, uint32 amount);
bool mail_setattachment(map_session_data *sd, struct mail_message *msg);
void mail_getattachment(map_session_data* sd, struct mail_message* msg, int32 zeny, struct item* item);
int32 mail_openmail(map_session_data *sd);
void mail_deliveryfail(map_session_data *sd, struct mail_message *msg);
bool mail_invalid_operation(map_session_data *sd);
void mail_send(map_session_data *sd, const char *dest_name, const char *title, const char *body_msg, int32 body_len);
void mail_refresh_remaining_amount( map_session_data* sd );

#endif /* MAIL_HPP */
