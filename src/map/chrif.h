// $Id: chrif.h,v 1.3 2004/09/25 11:39:17 MouseJstr Exp $
#ifndef _CHRIF_H_
#define _CHRIF_H_

void chrif_setuserid(char*);
void chrif_setpasswd(char*);
void chrif_setip(char*);
void chrif_setport(int);

int chrif_isconnect(void);

int chrif_authreq(struct map_session_data *);
int chrif_save(struct map_session_data*);
int chrif_charselectreq(struct map_session_data *);

int chrif_changemapserver(struct map_session_data *sd,char *name,int x,int y,int ip,short port);

int chrif_searchcharid(int char_id);
int chrif_changegm(int id,const char *pass,int len);
int chrif_changeemail(int id, const char *actual_email, const char *new_email);
int chrif_char_ask_name(int id, char * character_name, short operation_type, int year, int month, int day, int hour, int minute, int second);
int chrif_saveaccountreg2(struct map_session_data *sd);
int chrif_reloadGMdb(void);
int chrif_ragsrvinfo(int base_rate,int job_rate, int drop_rate);
int chrif_char_offline(struct map_session_data *sd);
int chrif_changesex(int id, int sex);
int chrif_chardisconnect(struct map_session_data *sd);

int do_init_chrif(void);

#endif
