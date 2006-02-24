// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _SHOWMSG_H_
#define _SHOWMSG_H_

#define SHOW_DEBUG_MSG 1

// for help with the console colors look here:
// http://www.edoceo.com/liberum/?doc=printf-with-color
// some code explanation (used here):
// \033[2J : clear screen and go up/left (0, 0 position)
// \033[K  : clear line from actual position to end of the line
// \033[0m : reset color parameter
// \033[1m : use bold for font

#ifdef _WIN32
	#define	CL_RESET	""
	#define CL_CLS		""
	#define CL_CLL		""
	#define	CL_BOLD		""
	#define CL_NORMAL	CL_RESET
	#define CL_NONE		CL_RESET
	#define	CL_WHITE	""
	#define	CL_GRAY		""
	#define	CL_RED		""
	#define	CL_GREEN	""
	#define	CL_YELLOW	""
	#define	CL_BLUE		""
	#define	CL_MAGENTA	""
	#define	CL_CYAN		""
	#define	CL_BT_YELLOW	""
	#define	CL_WTBL			""
	#define	CL_XXBL			""
	#define CL_PASS			""
#else
	#define	CL_RESET	"\033[0;0m"
	#define CL_CLS		"\033[2J"
	#define CL_CLL		"\033[K"

	// font settings
	#define	CL_BOLD		"\033[1m"
	#define CL_NORMAL	CL_RESET
	#define CL_NONE		CL_RESET

	#define	CL_WHITE	"\033[1;37m"
	#define	CL_GRAY		"\033[1;30m"
	#define	CL_RED		"\033[1;31m"
	#define	CL_GREEN	"\033[1;32m"
	#define	CL_YELLOW	"\033[1;33m"
	#define	CL_BLUE		"\033[1;34m"
	#define	CL_MAGENTA	"\033[1;35m"
	#define	CL_CYAN		"\033[1;36m"
	
	#define	CL_BT_YELLOW	"\033[1;33m"
	#define	CL_WTBL			"\033[37;44m"	// white on blue
	#define	CL_XXBL			"\033[0;44m"	// default on blue
	#define CL_PASS			"\033[0;32;42m"	// green on green
#endif

extern int msg_silent; //Specifies how silent the console is. [Skotlex]
extern char timestamp_format[20]; //For displaying Timestamps [Skotlex]
extern char tmp_output[1024];

enum msg_type {
	MSG_NONE,
	MSG_STATUS,
	MSG_SQL,
	MSG_INFORMATION,
	MSG_NOTICE,
	MSG_WARNING,
	MSG_DEBUG,
	MSG_ERROR,
	MSG_FATALERROR
};

extern void ClearScreen(void);
extern int ShowMessage(const char *, ...);
extern int ShowStatus(const char *, ...);
extern int ShowSQL(const char *, ...);
extern int ShowInfo(const char *, ...);
extern int ShowNotice(const char *, ...);
extern int ShowWarning(const char *, ...);
extern int ShowDebug(const char *, ...);
extern int ShowError(const char *, ...);
extern int ShowFatalError(const char *, ...);

#endif
