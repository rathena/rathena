// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHARSAVE_H_
#define _CHARSAVE_H_

#include "status.h"

#ifndef TXT_ONLY
	int charsave_loadHomunculus(int hom_id, struct homun_data *p);
	int charsave_saveHomunculus(struct homun_data *hd);
	int charsave_saveHomunculusSkills(struct homun_data *hd);
	int charsave_deleteHomunculus(struct homun_data *hd);

	struct mmo_charstatus *charsave_loadchar(int charid);
	int charsave_savechar(int charid, struct mmo_charstatus *c);
	int charsave_load_scdata(int account_id, int char_id);
	void charsave_save_scdata(int account_id, int char_id, struct status_change* sc_data, int max_sc);
#endif

#endif
