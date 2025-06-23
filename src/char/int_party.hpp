// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_PARTY_HPP
#define INT_PARTY_HPP

#include <common/cbasetypes.hpp>

//Party Flags on what to save/delete.
enum e_PartySaveWhatFlag {
	PS_CREATE = 0x01, //Create a new party entry (index holds leader's info) 
	PS_BASIC = 0x02, //Update basic party info.
	PS_LEADER = 0x04, //Update party's leader
	PS_ADDMEMBER = 0x08, //Specify new party member (index specifies which party member)
	PS_DELMEMBER = 0x10, //Specify member that left (index specifies which party member)
	PS_BREAK = 0x20, //Specify that this party must be deleted.
};

//struct party;

int32 inter_party_parse_frommap(int32 fd);
int32 inter_party_sql_init(void);
void inter_party_sql_final(void);
int32 inter_party_leave(int32 party_id,uint32 account_id, uint32 char_id, char *name);
int32 inter_party_charname_changed(int32 party_id, uint32 char_id, char *name);
int32 inter_party_CharOnline(uint32 char_id, int32 party_id);
int32 inter_party_CharOffline(uint32 char_id, int32 party_id);

#endif /* INT_PARTY_HPP */
