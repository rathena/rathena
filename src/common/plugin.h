// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_PLUGIN_H_
#define _PLUGIN_H_

#ifndef _CBASETYPES_H_
#include "../common/cbasetypes.h"
#endif

////// Plugin functions ///////////////

// Plugin version <major version>.<minor version>
// * <major version> is increased and <minor version> reset when at least one 
//   export of the previous version becomes incompatible
// * <minor version> is increased if the previous version remains compatible
// 
// Compatible plugins have:
// - equal major version
// - lower or equal minor version
#define PLUGIN_VERSION "1.03"

typedef struct _Plugin_Info {
	char* name;
	char type;
	char* version;
	char* req_version;
	char* description;
} Plugin_Info;

typedef struct _Plugin_Event_Table {
	char* func_name;
	char* event_name;
} Plugin_Event_Table;

// Format of the test function
typedef int Plugin_Test_Func(void);
#define EVENT_PLUGIN_INIT		"Plugin_Init"  // Initialize the plugin
#define EVENT_PLUGIN_FINAL		"Plugin_Final" // Finalize the plugin
#define EVENT_ATHENA_INIT		"Athena_Init"  // Server started
#define EVENT_ATHENA_FINAL		"Athena_Final" // Server ended

// Format of event functions
typedef void Plugin_Event_Func(void);
#define EVENT_PLUGIN_TEST		"Plugin_Test" // Test the plugin for compatibility

////// Plugin Export functions /////////////

#define PLUGIN_ALL			0
#define PLUGIN_LOGIN		1
#define PLUGIN_CHAR			2
#define PLUGIN_MAP			8
#define PLUGIN_CORE			16

#define IMPORT_SYMBOL(s,n)	(s) = plugin_call_table[n]

#define SYMBOL_SERVER_TYPE				0
#define SYMBOL_SERVER_NAME				1
#define SYMBOL_ARG_C					2
#define SYMBOL_ARG_V					3
#define SYMBOL_RUNFLAG					4
#define SYMBOL_GETTICK					5
#define SYMBOL_GET_SVN_REVISION			6
#define SYMBOL_ADD_TIMER				7
#define SYMBOL_ADD_TIMER_INTERVAL		8
#define SYMBOL_ADD_TIMER_FUNC_LIST		9
#define SYMBOL_DELETE_TIMER				10
#define SYMBOL_GET_UPTIME				11
#define SYMBOL_ADDR						12
#define SYMBOL_FD_MAX					13
#define SYMBOL_SESSION					14
#define SYMBOL_DELETE_SESSION			15
#define SYMBOL_WFIFOSET					16
#define SYMBOL_RFIFOSKIP				17
#define SYMBOL_FUNC_PARSE_TABLE			18
// 1.03
#define SYMBOL_PARSE_CONSOLE			19

////// Global Plugin variables /////////////

#define PLUGIN_INFO			struct _Plugin_Info plugin_info
#define PLUGIN_EVENTS_TABLE	struct _Plugin_Event_Table plugin_event_table[]

#ifndef	_PLUGINS_H_
void** plugin_call_table;
#endif

#endif /* _PLUGIN_H_ */
