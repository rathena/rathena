#pragma once


#define STRICT
#define NTDDI_VERSION 	NTDDI_WIN2K
#define _WIN32_WINNT  0x0500
#define WINVER 0x0500
#define _WIN32_IE 	0x0600
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


