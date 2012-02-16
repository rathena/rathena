// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/timer.h"
#include "../common/utils.h" // findfile()
#include "../common/socket.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "plugins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif

//////////////////////////////////////////////

typedef struct _Plugin_Event {
	struct _Plugin_Event* next;
	Plugin_Event_Func* func;
} Plugin_Event;

typedef struct _Plugin_Event_List {
	struct _Plugin_Event_List* next;
	char* name;
	struct _Plugin_Event* events;
} Plugin_Event_List;

static int auto_search = 0;
static int load_priority = 0;
Plugin_Event_List* event_head = NULL;
Plugin* plugin_head = NULL;

static Plugin_Info default_info = { "Unknown", PLUGIN_ALL, "0", PLUGIN_VERSION, "Unknown" };

void** plugin_call_table;
static size_t call_table_size	= 0;
static size_t max_call_table	= 0;

////// Plugin Events Functions //////////////////

int register_plugin_func(char* name)
{
	Plugin_Event_List* evl;
	if( name ){
		//ShowDebug("register_plugin_func(%s)\n", name);
		CREATE(evl, Plugin_Event_List, 1);
		evl->next = event_head;
		evl->name = aStrdup(name);
		evl->events = NULL;
		event_head = evl;
	}
	return 0;
}

static Plugin_Event_List* search_plugin_func(char* name)
{
	Plugin_Event_List* evl = event_head;
	while( evl ){
		if( strcmpi(evl->name,name) == 0 )
			return evl;
		evl = evl->next;
	}
	return NULL;
}

int register_plugin_event(Plugin_Event_Func* func, char* name)
{
	Plugin_Event_List* evl = search_plugin_func(name);
	//ShowDebug("register_plugin_event(0x%x, %s)\n", func, name);
	if( !evl )
	{// event does not exist, register
		register_plugin_func(name);
		// get the new event list
		evl = search_plugin_func(name);
	}
	if( evl ){
		Plugin_Event* ev;

		CREATE(ev, Plugin_Event, 1);
		ev->func = func;
		ev->next = NULL;

		// insert event at the end of the linked list
		if( evl->events == NULL )
			evl->events = ev;
		else {
			Plugin_Event* last_ev = evl->events;
			while( last_ev ){
				if( last_ev->next == NULL ){
					last_ev->next = ev;
					break;
				}
				last_ev = last_ev->next;
			}
		}
	}
	return 0;
}

int plugin_event_trigger(char* name)
{
	int c = 0;
	Plugin_Event_List* evl = search_plugin_func(name);
	//ShowDebug("plugin_event_trigger(%s)\n", name);
	if( evl ){
		Plugin_Event* ev = evl->events;
		while( ev ){
			ev->func();
			//ShowDebug("plugin_event_trigger: Executing function 0x%x.\n", ev->func);
			ev = ev->next;
			++c;
		}
	}
	return c;
}

////// Plugins Call Table Functions /////////

int export_symbol(void* var, size_t offset)
{
	//ShowDebug("export_symbol(0x%x,%d)\n", var,offset);

	// add to the end of the list
	if( offset < 0 )
		offset = call_table_size;

	if( offset >= max_call_table )
	{// realloc if not large enough  
		max_call_table = 1 + offset;
		RECREATE(plugin_call_table, void*, max_call_table);

		// clear the new alloced block
		memset(plugin_call_table + call_table_size, 0, (max_call_table-call_table_size)*sizeof(void*));
	}

	// the new table size is delimited by the new element at the end
	if( offset >= call_table_size )
		call_table_size = offset+1;

	// put the pointer at the selected place
	plugin_call_table[offset] = var;
	return 0;
}

////// Plugins Core /////////////////////////

static int plugin_iscompatible(char* version)
{
	int req_major = 0;
	int req_minor = 0;
	int major = 0;
	int minor = 0;

	if( version == NULL )
		return 0;
	sscanf(version, "%d.%d", &req_major, &req_minor);
	sscanf(PLUGIN_VERSION, "%d.%d", &major, &minor);
	return ( req_major == major && req_minor <= minor );
}

Plugin* plugin_open(const char* filename)
{
	Plugin* plugin;
	Plugin_Info* info;
	Plugin_Event_Table* events;
	void** procs;
	int init_flag = 1;

	//ShowDebug("plugin_open(%s)\n", filename);
	
	// Check if the plugin has been loaded before
	plugin = plugin_head;
	while (plugin) {
		// returns handle to the already loaded plugin
		if( plugin->state && strcmpi(plugin->filename, filename) == 0 ){
			ShowWarning("plugin_open: not loaded (duplicate) : '"CL_WHITE"%s"CL_RESET"'\n", filename);
			return plugin;
		}
		plugin = plugin->next;
	}

	CREATE(plugin, Plugin, 1);
	plugin->state = -1;	// not loaded

	plugin->dll = DLL_OPEN(filename);
	if( !plugin->dll ){
		ShowWarning("plugin_open: not loaded (invalid file) : '"CL_WHITE"%s"CL_RESET"'\n", filename);
		plugin_unload(plugin);
		return NULL;
	}

	// Retrieve plugin information
	plugin->state = 0;	// initialising
	info = (Plugin_Info*)DLL_SYM(plugin->dll, "plugin_info");
	// For high priority plugins (those that are explicitly loaded from the conf file)
	// we'll ignore them even (could be a 3rd party dll file)
	if( !info )
	{// foreign plugin
		//ShowDebug("plugin_open: plugin_info not found\n");
		if( load_priority == 0 )
		{// not requested
			//ShowDebug("plugin_open: not loaded (not requested) : '"CL_WHITE"%s"CL_RESET"'\n", filename);
			plugin_unload(plugin);
			return NULL;
		}
	} else if( !plugin_iscompatible(info->req_version) )
	{// incompatible version
		ShowWarning("plugin_open: not loaded (incompatible version '%s' -> '%s') : '"CL_WHITE"%s"CL_RESET"'\n", info->req_version, PLUGIN_VERSION, filename);
		plugin_unload(plugin);
		return NULL;
	} else if( (info->type != PLUGIN_ALL && info->type != PLUGIN_CORE && info->type != SERVER_TYPE) ||
		(info->type == PLUGIN_CORE && SERVER_TYPE != PLUGIN_LOGIN && SERVER_TYPE != PLUGIN_CHAR && SERVER_TYPE != PLUGIN_MAP) )
	{// not for this server
		//ShowDebug("plugin_open: not loaded (incompatible) : '"CL_WHITE"%s"CL_RESET"'\n", filename);
		plugin_unload(plugin);
		return NULL;
	}

	plugin->info = ( info != NULL ? info : &default_info );
	plugin->filename = aStrdup(filename);

	// Initialise plugin call table (For exporting procedures)
	procs = (void**)DLL_SYM(plugin->dll, "plugin_call_table");
	if( procs )
		*procs = plugin_call_table;
	//else ShowDebug("plugin_open: plugin_call_table not found\n");

	// Register plugin events
	events = (Plugin_Event_Table*)DLL_SYM(plugin->dll, "plugin_event_table");
	if( events ){
		int i = 0;
		//ShowDebug("plugin_open: parsing plugin_event_table\n");
		while( events[i].func_name ){
			if( strcmpi(events[i].event_name, EVENT_PLUGIN_TEST) == 0 ){
				Plugin_Test_Func* test_func;
				test_func = (Plugin_Test_Func*)DLL_SYM(plugin->dll, events[i].func_name);
				//ShowDebug("plugin_open: invoking "EVENT_PLUGIN_TEST" with %s()\n", events[i].func_name);
				if( test_func && test_func() == 0 ){
					// plugin has failed test, disabling
					//ShowDebug("plugin_open: disabled (failed test) : %s\n", filename);
					init_flag = 0;
				}
			} else {
				Plugin_Event_Func* func;
				func = (Plugin_Event_Func*)DLL_SYM(plugin->dll, events[i].func_name);
				if (func)
					register_plugin_event(func, events[i].event_name);
				else
					ShowError("Failed to locate function '%s' in '%s'.\n", events[i].func_name, filename);
			}
			i++;
		}
	}
	//else ShowDebug("plugin_open: plugin_event_table not found\n");

	plugin->next = plugin_head;
	plugin_head = plugin;

	plugin->state = init_flag;	// fully loaded
	ShowStatus("Done loading plugin '"CL_WHITE"%s"CL_RESET"'\n", (info) ? plugin->info->name : filename);

	return plugin;
}

void plugin_load(const char* filename)
{
	plugin_open(filename);
}

void plugin_unload(Plugin* plugin)
{
	if( plugin == NULL )
		return;
	if( plugin->filename )
		aFree(plugin->filename);
	if( plugin->dll )
		DLL_CLOSE(plugin->dll);
	aFree(plugin);
}

#ifdef WIN32
char *DLL_ERROR(void)
{
	static char dllbuf[80];
	DWORD dw = GetLastError();
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, 0, dllbuf, 80, NULL);
	return dllbuf;
}
#endif

////// Initialize/Finalize ////////////////////

static int plugins_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName, "r");
	if( fp == NULL ){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while( fgets(line, sizeof(line), fp) )
	{
		if( line[0] == '/' && line[1] == '/' )
			continue;
		if( sscanf(line,"%[^:]: %[^\r\n]",w1,w2) != 2 )
			continue;

		if( strcmpi(w1,"auto_search") == 0 ){
			if( strcmpi(w2,"yes") == 0 )
				auto_search = 1;
			else if( strcmpi(w2,"no") == 0 )
				auto_search = 0;
			else
				auto_search = atoi(w2);
		} else if( strcmpi(w1,"plugin") == 0 ){
			char filename[128];
			sprintf(filename, "plugins/%s%s", w2, DLL_EXT);
			plugin_load(filename);
		} else if( strcmpi(w1,"import") == 0 )
			plugins_config_read(w2);
	}
	fclose(fp);
	return 0;
}

void plugins_init(void)
{
	// Sugested functionality:
	// add atcommands/script commands (Borf)
	char* PLUGIN_CONF_FILENAME = "conf/plugin_athena.conf";
	//ShowDebug("plugins_init()\n");
	register_plugin_func(EVENT_PLUGIN_INIT);
	register_plugin_func(EVENT_PLUGIN_FINAL);
	register_plugin_func(EVENT_ATHENA_INIT);
	register_plugin_func(EVENT_ATHENA_FINAL);

	// networking
	EXPORT_SYMBOL(RFIFOSKIP,  SYMBOL_RFIFOSKIP);
	EXPORT_SYMBOL(WFIFOSET,   SYMBOL_WFIFOSET);
	EXPORT_SYMBOL(do_close,   SYMBOL_DELETE_SESSION);
	EXPORT_SYMBOL(session,    SYMBOL_SESSION);
	EXPORT_SYMBOL(&fd_max,    SYMBOL_FD_MAX);
	EXPORT_SYMBOL(addr_,      SYMBOL_ADDR);
	// timers
	EXPORT_SYMBOL(get_uptime,              SYMBOL_GET_UPTIME);
	EXPORT_SYMBOL(delete_timer,            SYMBOL_DELETE_TIMER);
	EXPORT_SYMBOL(add_timer_func_list,     SYMBOL_ADD_TIMER_FUNC_LIST);
	EXPORT_SYMBOL(add_timer_interval,      SYMBOL_ADD_TIMER_INTERVAL);
	EXPORT_SYMBOL(add_timer,               SYMBOL_ADD_TIMER);
	EXPORT_SYMBOL((void*)get_svn_revision, SYMBOL_GET_SVN_REVISION);
	EXPORT_SYMBOL(gettick,                 SYMBOL_GETTICK);
	// core
	EXPORT_SYMBOL(parse_console, SYMBOL_PARSE_CONSOLE);
	EXPORT_SYMBOL(&runflag,      SYMBOL_RUNFLAG);
	EXPORT_SYMBOL(arg_v,         SYMBOL_ARG_V);
	EXPORT_SYMBOL(&arg_c,        SYMBOL_ARG_C);
	EXPORT_SYMBOL(SERVER_NAME,   SYMBOL_SERVER_NAME);
	EXPORT_SYMBOL(&SERVER_TYPE,  SYMBOL_SERVER_TYPE);

	load_priority = 1;
	plugins_config_read(PLUGIN_CONF_FILENAME);
	load_priority = 0;

	if( auto_search )
		findfile("plugins", DLL_EXT, plugin_load);

	plugin_event_trigger(EVENT_PLUGIN_INIT);

	return;
}

void plugins_final(void)
{
	Plugin* plugin = plugin_head;
	Plugin* next_plugin;
	Plugin_Event_List* evl = event_head;
	Plugin_Event_List* next_evl;
	Plugin_Event* ev;
	Plugin_Event* next_ev;

	//ShowDebug("plugins_final()\n");
	plugin_event_trigger(EVENT_PLUGIN_FINAL);

	while( plugin ){
		next_plugin = plugin->next;
		plugin_unload(plugin);
		plugin = next_plugin;
	}

	while( evl ){
		ev = evl->events;
		while( ev ){
			next_ev = ev->next;
			aFree(ev);
			ev = next_ev;
		}
		next_evl = evl->next;
		aFree(evl->name);
		aFree(evl);
		evl = next_evl;
	}

	aFree(plugin_call_table);

	return;
}
