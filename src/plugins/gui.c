// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common/plugin.h"
//Needed for strcmpi
#include "../common/mmo.h"

// "I'm Alive" and "Flush stdout" Originally by Mugendai
// Ported to plugin by Celest

PLUGIN_INFO = {
	"AthenaGUI",
	PLUGIN_CORE,
	"1.0",
	PLUGIN_VERSION,
	"Core plugin for Athena GUI functions"
};

PLUGIN_EVENTS_TABLE = {
	{ "gui_init", "Plugin_Init" },
	{ NULL, NULL }
};

typedef int (*TimerFunc)(int tid, unsigned int tick, int id, intptr data);
unsigned int (*gettick)();
int (*add_timer_func_list)(TimerFunc func, char* name);
int (*add_timer_interval)(unsigned int tick, TimerFunc func, int id, intptr data, int interval);

//-----------------------------------------------------
//I'm Alive Alert
//Used to output 'I'm Alive' every few seconds
//Intended to let frontends know if the app froze
//-----------------------------------------------------
int imalive_timer(int tid, unsigned int tick, int id, intptr data)
{
	printf("I'm Alive\n");
	return 0;
}

//-----------------------------------------------------
//Flush stdout
//stdout buffer needs flushed to be seen in GUI
//-----------------------------------------------------
int flush_timer(int tid, unsigned int tick, int id, intptr data)
{
	fflush(stdout);
	return 0;
}

void gui_init ()
{
	char line[1024], w1[1024], w2[1024];
	int flush_on = 0;
	int flush_time = 100;
	int imalive_on = 0;
	int imalive_time = 30;
	char **argv;
	int *argc;
	FILE *fp;
	int i;

	IMPORT_SYMBOL(argc, 2);
	IMPORT_SYMBOL(argv, 3);
	IMPORT_SYMBOL(gettick, 5);
	IMPORT_SYMBOL(add_timer_interval, 8);
	IMPORT_SYMBOL(add_timer_func_list, 9);

	do {
		fp = fopen("plugins/gui.conf","r");
		if (fp == NULL)
			break;

		while(fgets(line, sizeof(line), fp))
		{
			if (line[0] == '/' && line[1] == '/')
				continue;
			if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
				if(strcmpi(w1,"imalive_on")==0){
					imalive_on = atoi(w2);
				} else if(strcmpi(w1,"imalive_time")==0){
					imalive_time = atoi(w2);
				} else if(strcmpi(w1,"flush_on")==0){
					flush_on = atoi(w2);
				} else if(strcmpi(w1,"flush_time")==0){
					flush_time = atoi(w2);
				}
			}
		}
		fclose(fp);
	} while (0);

	for (i = 1; i < *argc ; i++)
		if (strcmp(argv[i], "--gui") == 0)
			flush_on = imalive_on = 1;

	if (flush_on) {
		add_timer_func_list(flush_timer, "flush_timer");
		add_timer_interval(gettick()+1000,flush_timer,0,0,flush_time);
	}
	if (imalive_on) {
		add_timer_func_list(imalive_timer, "imalive_timer");
		add_timer_interval(gettick()+10, imalive_timer,0,0,imalive_time*1000);
	}
}
