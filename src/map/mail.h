// Mail System for eAthena
// Created by Valaris

int mail_check(struct map_session_data *sd, int type);
int mail_read(struct map_session_data *sd, int message_id);
int mail_delete(struct map_session_data *sd, int message_id);
int mail_send(struct map_session_data *sd, char *name, char *message, int flag);

int do_init_mail(void);
