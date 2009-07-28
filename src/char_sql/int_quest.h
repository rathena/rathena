// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _QUEST_H_
#define _QUEST_H_

/*questlog system*/
struct quest;

int inter_quest_parse_frommap(int fd);
int mapif_quest_delete(int char_id, int quest_id);
int mapif_quest_add(int char_id, struct quest qd);
int mapif_quest_update(int char_id, struct quest qd);

#endif

