
#ifndef	_DLL_H_
#define _DLL_H_

#ifdef _WIN32

	#include <windows.h>
	#define DLL_OPEN(x)		LoadLibrary(x)
	#define DLL_SYM(x,y,z)	(FARPROC)(x) = GetProcAddress(y,z)
	#define DLL_CLOSE(x)	FreeLibrary(x)
	#define DLL				HINSTANCE

#else

	#include <dlfcn.h>
	#define DLL_OPEN(x)		dlopen(x,RTLD_NOW)
	#define DLL_SYM(x,y,z)	(x) = (void *)dlsym(y,z)
	#define DLL_CLOSE(x)	dlclose(x)
	#define DLL				void *

#endif

#endif	// _DLL_H_


