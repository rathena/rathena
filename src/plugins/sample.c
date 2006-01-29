// Sample Athena plugin

#include <stdio.h>
#include <string.h>
#include "../common/plugin.h"

////// Plugin information ////////
//
PLUGIN_INFO = {
// change only the following area
	"Test",			// Plugin name
	PLUGIN_ALL,		// Which servers is this plugin for
	"0.1",			// Plugin version
	PLUGIN_VERSION,		// Minimum plugin engine version to run
	"A sample plugin"	// Short description of plugin
};

////// Plugin event list //////////
// Format: <plugin function>,<event name>
// All registered functions to a event gets executed
// (In descending order) when its called.
// Multiple functions can be called by multiple events too,
// So it's up to your creativity ^^
//
PLUGIN_EVENTS_TABLE = {
// change only the following area
	{ "test_me", "Plugin_Test" },	// when the plugin is tested for compatibility
	{ "do_init", "Plugin_Init" },	// when plugins are loaded
	{ "do_final", "Plugin_Final" },	// when plugins are unloaded
	{ "some_function", "some_event" },
	{ "some_function", "another_event" },
	{ NULL, NULL }
};

///// Variables /////
char *server_type;
char *server_name;

//////// Plugin functions //////////
int do_init ()
{
	// import symbols from the server
	IMPORT_SYMBOL(server_type, 0);
	IMPORT_SYMBOL(server_name, 1);

	printf ("Server type is ");
	switch (*server_type) {
		case PLUGIN_LOGIN: printf ("Login\n"); break;
		case PLUGIN_CHAR: printf ("Char\n"); break;
		case PLUGIN_MAP: printf ("Map\n"); break;
	}
	printf ("Filename is %s\n", server_name);

	return 1;
}

int do_final ()
{
	printf ("Bye world\n");

	return 1;
}

int some_function ()
{
	printf ("Some function\n");
	return 0;
}

// return 1 if the testing passes, otherwise 0
// (where the plugin will be deactivated)
int test_me ()
{
	if (1 + 1 == 2)
		return 1;
	return 0;
}
