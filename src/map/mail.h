// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAIL_H_
#define _MAIL_H_

#include "../common/mmo.h"

time_t mail_calctimes(void);

void mail_clear(struct map_session_data *sd);
int mail_removeitem(struct map_session_data *sd, short flag);
int mail_removezeny(struct map_session_data *sd, short flag);
unsigned char mail_setitem(struct map_session_data *sd, int idx, int amount);
bool mail_setattachment(struct map_session_data *sd, struct mail_message *msg);
void mail_getattachment(struct map_session_data* sd, int zeny, struct item* item);
int mail_openmail(struct map_session_data *sd);
void mail_deliveryfail(struct map_session_data *sd, struct mail_message *msg);

#endif /* _MAIL_H_ */
