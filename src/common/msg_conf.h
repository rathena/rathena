// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MSG_CONF_H
#define	MSG_CONF_H

#ifdef	__cplusplus
extern "C" {
#endif

const char* _msg_txt(int msg_number,int size, char ** msg_table);
int _msg_config_read(const char* cfgName,int size, char ** msg_table);
void _do_final_msg(int size, char ** msg_table);

#ifdef	__cplusplus
}
#endif

#endif	/* MSG_CONF_H */

