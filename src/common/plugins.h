// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef	_PLUGINS_H_
#define _PLUGINS_H_

////// Dynamic Link Library functions ///////////////

#ifdef _WIN32

	#include <windows.h>
	#define DLL_OPEN(x)		LoadLibrary(x)
	#define DLL_SYM(x,y,z)	(FARPROC)(x) = GetProcAddress(y,z)
	#define DLL_CLOSE(x)	FreeLibrary(x)
	#define DLL_EXT			".dll"
	#define DLL				HINSTANCE
	char *DLL_ERROR(void);

#else

	#include <dlfcn.h>
	#define DLL_OPEN(x)		dlopen(x,RTLD_NOW)
	#define DLL_SYM(x,y,z)	(x) = (void *)dlsym(y,z)
	#define DLL_CLOSE(x)	dlclose(x)
	#define DLL_ERROR		dlerror

	#ifdef CYGWIN
		#define DLL_EXT		".dll"
	#else
		#define DLL_EXT		".so"
	#endif
	#define DLL				void *

#endif

////// Plugin Definitions ///////////////////

typedef struct _Plugin {
	DLL dll;
	char state;
	char *filename;
	struct _Plugin_Info *info;
	struct _Plugin *next;	
} Plugin;

/////////////////////////////////////////////

int register_plugin_func (char *);
int register_plugin_event (void (*)(void), char *);
int plugin_event_trigger (char *);

int export_symbol (void *, int);
#define EXPORT_SYMBOL(s)	export_symbol((s), -1);

Plugin *plugin_open (const char *);
void plugin_load (const char *);
void plugin_unload (Plugin *);
void plugins_init (void);
void plugins_final (void);

#endif	// _PLUGINS_H_
