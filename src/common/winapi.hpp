// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef WINAPI_HPP
#define WINAPI_HPP

#include "../config/core.hpp"

#ifdef DEPRECATED_WINDOWS_SUPPORT
#ifndef NTDDI_VERSION
	#define NTDDI_VERSION 	0x05000000 // Windows 2000
#endif
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT  0x0500 // Windows 2000
#endif
#ifndef WINVER
	#define WINVER 0x0500 // Windows 2000
#endif
#ifndef _WIN32_IE
	#define _WIN32_IE 	0x0600 // Internet Explorer 6.0
#endif
#ifndef _WIN32_WINNT_VISTA
	#define _WIN32_WINNT_VISTA 0x0600 // Windows Vista
#endif
#else
#include <sdkddkver.h>
#endif

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOCOMM 
#define NOKANJI
#define NOHELP
#define NOMCX
#define NOCLIPBOARD
#define NOCOLOR
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <io.h>
#include <Windows.h>
#include <WinSock2.h>
#include <In6addr.h>
#include <Ws2tcpip.h>
#include <Mswsock.h>
#include <MMSystem.h>

#include "cbasetypes.hpp"

bool IsCurrentUserLocalAdministrator(void);

#endif /* WINAPI_HPP */
