// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAIL_H_
#define _MAIL_H_

#include "../common/mmo.h"

time_t mail_calctimes(void);

int mail_removeitem(struct map_session_data *sd, short flag);
int mail_removezeny(struct map_session_data *sd, short flag);
char mail_setitem(struct map_session_data *sd, int idx, int amount);
int mail_getattach(struct map_session_data *sd, struct mail_message *msg);
int mail_openmail(struct map_session_data *sd);

#endif /* _MAIL_H_ */
