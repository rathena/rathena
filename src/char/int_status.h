// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STATUS_H_
#define _INT_STATUS_H_

struct status_change_data;

struct scdata {
	int account_id, char_id;
	int count;
	struct status_change_data* data;
};

extern char scdata_txt[1024];

struct scdata* status_search_scdata(int aid, int cid);
void status_delete_scdata(int aid, int cid);
void inter_status_save(void);
void status_init(void);
void status_final(void);

#endif /* _INT_STATUS_H_ */
