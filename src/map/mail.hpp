// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAIL_HPP_
#define _MAIL_HPP_

#include "../common/mmo.hpp"

enum mail_attach_result {
	MAIL_ATTACH_SUCCESS = 0,
#if PACKETVER >= 20150513
	MAIL_ATTACH_WEIGHT = 1,
	MAIL_ATTACH_ERROR = 2,
	MAIL_ATTACH_SPACE = 3,
	MAIL_ATTACH_UNTRADEABLE = 4
#else
	MAIL_ATTACH_WEIGHT = 1,
	MAIL_ATTACH_ERROR = 1,
	MAIL_ATTACH_SPACE = 1,
	MAIL_ATTACH_UNTRADEABLE = 1
#endif
};

void mail_clear(struct map_session_data *sd);
int mail_removeitem(struct map_session_data *sd, short flag, int idx, int amount);
bool mail_removezeny(struct map_session_data *sd, bool flag);
enum mail_attach_result mail_setitem(struct map_session_data *sd, short idx, uint32 amount);
bool mail_setattachment(struct map_session_data *sd, struct mail_message *msg);
void mail_getattachment(struct map_session_data* sd, struct mail_message* msg, int zeny, struct item* item);
int mail_openmail(struct map_session_data *sd);
void mail_deliveryfail(struct map_session_data *sd, struct mail_message *msg);
bool mail_invalid_operation(struct map_session_data *sd);
void mail_send(struct map_session_data *sd, const char *dest_name, const char *title, const char *body_msg, int body_len);
void mail_refresh_remaining_amount( struct map_session_data* sd );

#endif /* _MAIL_HPP_ */
