
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
	#include <unistd.h>
#else
	#define getpid GetCurrentProcessId
#endif
#ifdef MINGW
	#include <process.h>
	#include <io.h>
#endif
#include "../common/plugin.h"

PLUGIN_INFO = {
	"ProcessId",
	PLUGIN_ALL,
	"1.0",
	PLUGIN_VERSION,
	"Logs the process ID"
};

PLUGIN_EVENTS_TABLE = {
	{ "pid_create", "Plugin_Init" },
	{ "pid_delete", "Plugin_Final" },
	{ NULL, NULL }
};

char pid_file[256];
char *server_name;

void pid_create ()
{
	FILE *fp;
	int len;
	
	IMPORT_SYMBOL(server_name, 1);
	len = strlen(server_name);
	strcpy(pid_file, server_name);
	if(len > 4 && pid_file[len - 4] == '.') {
		pid_file[len - 4] = 0;
	}
	strcat(pid_file, ".pid");
	fp = fopen(pid_file, "w");
	if (fp) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}
}

void pid_delete ()
{
	unlink(pid_file);
}
