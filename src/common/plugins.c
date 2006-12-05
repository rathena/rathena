// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "plugin.h"
#include "plugins.h"
#include "../common/mmo.h"
#include "../common/core.h"
#include "../common/timer.h"
#include "../common/utils.h"
#include "../common/socket.h"
#include "../common/malloc.h"
#include "../common/version.h"
#include "../common/showmsg.h"

//////////////////////////////////////////////

typedef struct _Plugin_Event {
	void (*func)(void);
	struct _Plugin_Event *next;
} Plugin_Event;

typedef struct _Plugin_Event_List {
	char *name;
	struct _Plugin_Event_List *next;
	struct _Plugin_Event *events;
} Plugin_Event_List;

static int auto_search = 1;
static int load_priority = 0;
Plugin_Event_List *event_head = NULL;
Plugin *plugin_head = NULL;

Plugin_Info default_info = { "Unknown", PLUGIN_ALL, "0", PLUGIN_VERSION, "Unknown" };

static size_t call_table_size	= 0;
static size_t max_call_table	= 0;

////// Plugin Events Functions //////////////////

int register_plugin_func (char *name)
{
	Plugin_Event_List *evl;
	if (name) {
		evl = (Plugin_Event_List *) aMalloc(sizeof(Plugin_Event_List));
		evl->name = (char *) aMalloc (strlen(name) + 1);

		evl->next = event_head;
		strcpy(evl->name, name);
		evl->events = NULL;
		event_head = evl;
	}
	return 0;
}

Plugin_Event_List *search_plugin_func (char *name)
{
	Plugin_Event_List *evl = event_head;
	while (evl) {
		if (strcmpi(evl->name, name) == 0)
			return evl;
		evl = evl->next;
	}
	return NULL;
}

int register_plugin_event (void (*func)(void), char* name)
{
	Plugin_Event_List *evl = search_plugin_func(name);
	if (!evl) {
		// register event if it doesn't exist already
		register_plugin_func(name);
		// relocate the new event list
		evl = search_plugin_func(name);
	}
	if (evl) {
		Plugin_Event *ev;

		ev = (Plugin_Event *) aMalloc(sizeof(Plugin_Event));
		ev->func = func;
		ev->next = NULL;

		if (evl->events == NULL)
			evl->events = ev;
		else {
			Plugin_Event *ev2 = evl->events;
			while (ev2) {
				if (ev2->next == NULL) {
					ev2->next = ev;
					break;
				}
				ev2 = ev2->next;
			}
		}
	}
	return 0;
}

int plugin_event_trigger (char *name)
{
	int c = 0;
	Plugin_Event_List *evl = search_plugin_func(name);
	if (evl) {
		Plugin_Event *ev = evl->events;
		while (ev) {
			ev->func();
			ev = ev->next;
			c++;
		}
	}
	return c;
}

////// Plugins Call Table Functions /////////

int export_symbol (void *var, int offset)
{
	//printf ("0x%x\n", var);
	
	// add to the end of the list
	if (offset < 0)
		offset = call_table_size;
	
	// realloc if not large enough  
	if ((size_t)offset >= max_call_table) {
		max_call_table = 1 + offset;
		plugin_call_table = (void**)aRealloc(plugin_call_table, max_call_table*sizeof(void*));
		
		// clear the new alloced block
		malloc_tsetdword(plugin_call_table + call_table_size, 0, (max_call_table-call_table_size)*sizeof(void*));
	}

	// the new table size is delimited by the new element at the end
	if ((size_t)offset >= call_table_size)
		call_table_size = offset+1;
	
	// put the pointer at the selected place
	plugin_call_table[offset] = var;
	return 0;
}

////// Plugins Core /////////////////////////

Plugin *plugin_open (const char *filename)
{
	Plugin *plugin;
	Plugin_Info *info;
	Plugin_Event_Table *events;
	void **procs;
	int init_flag = 1;

	//printf ("loading %s\n", filename);
	
	// Check if the plugin has been loaded before
	plugin = plugin_head;
	while (plugin) {
		// returns handle to the already loaded plugin
		if (plugin->state && strcmpi(plugin->filename, filename) == 0) {
			//printf ("not loaded (duplicate) : %s\n", filename);
			return plugin;
		}
		plugin = plugin->next;
	}

	plugin = (Plugin *)aCalloc(1, sizeof(Plugin));
	plugin->state = -1;	// not loaded

	plugin->dll = DLL_OPEN(filename);
	if (!plugin->dll) {
		//printf ("not loaded (invalid file) : %s\n", filename);
		plugin_unload(plugin);
		return NULL;
	}
	
	// Retrieve plugin information
	plugin->state = 0;	// initialising
	DLL_SYM (info, plugin->dll, "plugin_info");
	// For high priority plugins (those that are explicitly loaded from the conf file)
	// we'll ignore them even (could be a 3rd party dll file)
	if ((!info && load_priority == 0) ||
		(info && ((atof(info->req_version) < atof(PLUGIN_VERSION)) ||	// plugin is based on older code
		(info->type != PLUGIN_ALL && info->type != PLUGIN_CORE && info->type != SERVER_TYPE) ||	// plugin is not for this server
		(info->type == PLUGIN_CORE && SERVER_TYPE != PLUGIN_LOGIN && SERVER_TYPE != PLUGIN_CHAR && SERVER_TYPE != PLUGIN_MAP))))
	{
		//printf ("not loaded (incompatible) : %s\n", filename);
		plugin_unload(plugin);
		return NULL;
	}
	plugin->info = (info) ? info : &default_info;

	plugin->filename = (char *) aMalloc (strlen(filename) + 1);
	strcpy(plugin->filename, filename);

	// Initialise plugin call table (For exporting procedures)
	DLL_SYM (procs, plugin->dll, "plugin_call_table");
	if (procs) *procs = plugin_call_table;
	
	// Register plugin events
	DLL_SYM (events, plugin->dll, "plugin_event_table");
	if (events) {
		int i = 0;
		while (events[i].func_name) {
			if (strcmpi(events[i].event_name, "Plugin_Test") == 0) {
				int (*test_func)(void);
				DLL_SYM (test_func, plugin->dll, events[i].func_name);
				if (test_func && test_func() == 0) {
					// plugin has failed test, disabling
					//printf ("disabled (failed test) : %s\n", filename);
					init_flag = 0;
				}
			} else {
				void (*func)(void);
				DLL_SYM (func, plugin->dll, events[i].func_name);
				if (func) register_plugin_event (func, events[i].event_name);
			}
			i++;
		}
	}

	plugin->next = plugin_head;
	plugin_head = plugin;

	plugin->state = init_flag;	// fully loaded
	ShowStatus ("Done loading plugin '"CL_WHITE"%s"CL_RESET"'\n", (info) ? plugin->info->name : filename);

	return plugin;
}

void plugin_load (const char *filename)
{
	plugin_open(filename);
}

void plugin_unload (Plugin *plugin)
{
	if (plugin == NULL)
		return;
	if (plugin->filename) aFree(plugin->filename);
	if (plugin->dll) DLL_CLOSE(plugin->dll);
	aFree(plugin);
}

#ifdef _WIN32
char *DLL_ERROR(void)
{
	static char dllbuf[80];
	DWORD dw = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, 0, dllbuf, 80, NULL);
	return dllbuf;
}
#endif

////// Initialize/Finalize ////////////////////

int plugins_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while (fgets(line, 1020, fp)) {
		if(line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "auto_search") == 0) {
			if(strcmpi(w2, "yes")==0)
				auto_search = 1;
			else if(strcmpi(w2, "no")==0)
				auto_search = 0;
			else auto_search = atoi(w2);
		} else if (strcmpi(w1, "plugin") == 0) {
			char filename[128];
			sprintf (filename, "plugins/%s%s", w2, DLL_EXT);
			plugin_load(filename);
		} else if (strcmpi(w1, "import") == 0)
			plugins_config_read(w2);
	}
	fclose(fp);
	return 0;
}

void plugins_init (void)
{
	char *PLUGIN_CONF_FILENAME = "conf/plugin_athena.conf";
	register_plugin_func("Plugin_Init");
	register_plugin_func("Plugin_Final");
	register_plugin_func("Athena_Init");
	register_plugin_func("Athena_Final");

	// networking
	export_symbol (func_parse_table,	18);
	export_symbol (RFIFOSKIP,			17);
	export_symbol (WFIFOSET,			16);
	export_symbol (delete_session,		15);
	export_symbol (session,				14);
	export_symbol (&fd_max,				13);
	export_symbol (addr_,				12);
	// timers
	export_symbol (get_uptime,			11);
	export_symbol (delete_timer,		10);
	export_symbol (add_timer_func_list,	9);
	export_symbol (add_timer_interval,	8);
	export_symbol (add_timer,			7);
	export_symbol ((void *)get_svn_revision,	6);
	export_symbol (gettick,				5);
	// core
	export_symbol (&runflag,			4);
	export_symbol (arg_v,				3);
	export_symbol (&arg_c,				2);
	export_symbol (SERVER_NAME,			1);
	export_symbol (&SERVER_TYPE,		0);

	load_priority = 1;
	plugins_config_read (PLUGIN_CONF_FILENAME);
	load_priority = 0;

	if (auto_search)
		findfile("plugins", DLL_EXT, plugin_load);

	plugin_event_trigger("Plugin_Init");

	return;
}

void plugins_final (void)
{
	Plugin *plugin = plugin_head, *plugin2;
	Plugin_Event_List *evl = event_head, *evl2;
	Plugin_Event *ev, *ev2;

	plugin_event_trigger("Plugin_Final");

	while (plugin) {
		plugin2 = plugin->next;
		plugin_unload(plugin);
		plugin = plugin2;
	}

	while (evl) {
		ev = evl->events;
		while (ev) {
			ev2 = ev->next;
			aFree(ev);
			ev = ev2;
		}
		evl2 = evl->next;
		aFree(evl->name);
		aFree(evl);
		evl = evl2;
	}

	aFree(plugin_call_table);

	return;
}
