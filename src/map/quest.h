// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUEST_H_
#define _QUEST_H_

int quest_pc_login(TBL_PC * sd);
int quest_load_info(TBL_PC * sd, struct mmo_charstatus * st);
int quest_make_savedata(TBL_PC * sd);

#endif
