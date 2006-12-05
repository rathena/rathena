// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_PLUGIN_H_
#define _PLUGIN_H_

////// Plugin functions ///////////////

#define PLUGIN_VERSION "1.02"

typedef struct _Plugin_Info {
	char *name;
	char type;
	char *version;
	char *req_version;
	char *description;
} Plugin_Info;

typedef struct _Plugin_Event_Table {
	char *func_name;
	char *event_name;
} Plugin_Event_Table;

////// Plugin Export functions /////////////

#define PLUGIN_ALL			0
#define PLUGIN_LOGIN		1
#define PLUGIN_CHAR			2
#define PLUGIN_MAP			8
#define PLUGIN_CORE			16

#define IMPORT_SYMBOL(s,n)	(s) = plugin_call_table[n]

////// Global Plugin variables /////////////

#define PLUGIN_INFO			struct _Plugin_Info plugin_info
#define PLUGIN_EVENTS_TABLE	struct _Plugin_Event_Table plugin_event_table[]
void **plugin_call_table;

#endif	// _PLUGIN_H_
