#ifndef _SHOWMSG_H_
#define _SHOWMSG_H_

//davidsiaw, 'lookee' here!
#define SHOW_DEBUG_MSG 1

#define	CL_RESET	"\033[0;0m"
#define CL_NORMAL	CL_RESET
#define CL_NONE		CL_RESET
#define	CL_WHITE	"\033[1;29m"
#define	CL_GRAY		"\033[1;30m"
#define	CL_RED		"\033[1;31m"
#define	CL_GREEN	"\033[1;32m"
#define	CL_YELLOW	"\033[1;33m"
#define	CL_BLUE		"\033[1;34m"
#define	CL_MAGENTA	"\033[1;35m"
#define	CL_CYAN		"\033[1;36m"

extern char tmp_output[1024];

enum msg_type {MSG_STATUS, MSG_SQL, MSG_INFORMATION,MSG_NOTICE,MSG_WARNING,MSG_DEBUG,MSG_ERROR,MSG_FATALERROR};

extern int _ShowMessage(enum msg_type flag, const char *string, ...);

#if __GNUC__ >= 2
/* GCC用 */
/* MSG_XX */
	#define ShowMsg(flag,string...) _ShowMessage(flag, ## string)
//	#define DisplayMsg(flag,string,...) _ShowMessage(flag, ## string)
//	#define ShowMessage(flag,string,...) _ShowMessage(flag, ## string)

/* MSG_STATUS */
	#define ShowStatus(string...) _ShowMessage(MSG_STATUS, ## string)
//	#define DisplayStatus(string...) _ShowMessage(MSG_STATUS, ## string)

/* MSG_SQL*/
	#define ShowSQL(string...) _ShowMessage(MSG_SQL, ## string)
//	#define DisplaySQL(string...) _ShowMessage(MSG_SQL, ## string)

/* MSG_INFORMATION */
	#define ShowInfo(string...) _ShowMessage(MSG_INFORMATION, ## string)
//	#define DisplayInfo(string...) _ShowMessage(MSG_INFORMATION, ## string)
//	#define ShowInformation(string...) _ShowMessage(MSG_INFORMATION, ## string)
//	#define DisplayInformation(string...) _ShowMessage(MSG_INFORMATION, ## string)

/* MSG_NOTICE */
	#define ShowNotice(string...) _ShowMessage(MSG_NOTICE, ## string)
//	#define DisplayNotice(string...) _ShowMessage(MSG_NOTICE, ## string)

/* MSG_WARNING */
	#define ShowWarning(string...) _ShowMessage(MSG_WARNING, ## string)
//	#define DisplayWarning(string...) _ShowMessage(MSG_WARNING, ## string)
//	#define Warn(string...) _ShowMessage(MSG_WARNING, ## string)

/* MSG_DEBUG */
	#define ShowDebug(string...) _ShowMessage(MSG_DEBUG, ## string)
//	#define DisplayDebug(string,...) _ShowMessage(MSG_DEBUG, ## string)
//	#define Debug(string,...) _ShowMessage(MSG_DEBUG, ## string)
//	#define printDebug() _ShowMessage(MSG_DEBUG, ## string)

/* MSG_ERROR */
	#define ShowError(string...) _ShowMessage(MSG_ERROR, ## string)
//	#define DisplayError(string...) _ShowMessage(MSG_ERROR, ## string)
//	#define OutputError(string...) _ShowMessage(MSG_ERROR, ## string)

/* MSG_FATALERROR */
	#define ShowFatalError(string...) _ShowMessage(MSG_FATALERROR, ## string)
//	#define DisplayFatalError(string...) _ShowMessage(MSG_ERROR, ## string)
//	#define Terminate(string...) _ShowMessage(MSG_FATALERROR, ## string)
//	#define Kill(string...) _ShowMessage(MSG_FATALERROR, ## string)
//	#define AbortEx(string...) _ShowMessage(MSG_FATALERROR, ## string)

// 可変引数マクロに関する条件コンパイル
#elif __STDC_VERSION__ >= 199901L
/* C99に対応 */

/* MSG_XX */
	#define ShowMsg(flag,string...) _ShowMessage(flag, string, __VA_ARGS__)
//	#define DisplayMsg(flag,string,...) _ShowMessage(flag, string, __VA_ARGS__)
//	#define ShowMessage(flag,string,...) _ShowMessage(flag, string, __VA_ARGS__)

/* MSG_STATUS */
	#define ShowStatus(string...) _ShowMessage(MSG_STATUS, string, __VA_ARGS__)
//	#define DisplayStatus(string...) _ShowMessage(MSG_STATUS, string, __VA_ARGS__)

/* MSG_SQL*/
	#define ShowSQL(string...) _ShowMessage(MSG_SQL, string, __VA_ARGS__)
//	#define DisplaySQL(string...) _ShowMessage(MSG_SQL, string, __VA_ARGS__)

/* MSG_INFORMATION */
	#define ShowInfo(string...) _ShowMessage(MSG_INFORMATION, string, __VA_ARGS__)
//	#define DisplayInfo(string...) _ShowMessage(MSG_INFORMATION, string, __VA_ARGS__)
//	#define ShowInformation(string...) _ShowMessage(MSG_INFORMATION, string, __VA_ARGS__)
//	#define DisplayInformation(string...) _ShowMessage(MSG_INFORMATION, string, __VA_ARGS__)

/* MSG_NOTICE */
	#define ShowNotice(string...) _ShowMessage(MSG_NOTICE, string, __VA_ARGS__)
//	#define DisplayNotice(string...) _ShowMessage(MSG_NOTICE, string, __VA_ARGS__)

/* MSG_WARNING */
	#define ShowWarning(string...) _ShowMessage(MSG_WARNING, string, __VA_ARGS__)
//	#define DisplayWarning(string...) _ShowMessage(MSG_WARNING, string, __VA_ARGS__)
//	#define Warn(string...) _ShowMessage(MSG_WARNING, string, __VA_ARGS__)

/* MSG_DEBUG */
	#define ShowDebug(string...) _ShowMessage(MSG_DEBUG, MSGSTRING)
//	#define DisplayDebug(string,...) _ShowMessage(MSG_DEBUG, string, __VA_ARGS__)
//	#define Debug(string,...) _ShowMessage(MSG_DEBUG, string, __VA_ARGS__)
//	#define printDebug() _ShowMessage(MSG_DEBUG, string, __VA_ARGS__)

/* MSG_ERROR */
	#define ShowError(string...) _ShowMessage(MSG_ERROR, string, __VA_ARGS__)
//	#define DisplayError(string...) _ShowMessage(MSG_ERROR, string, __VA_ARGS__)
//	#define OutputError(string...) _ShowMessage(MSG_ERROR, string, __VA_ARGS__)

/* MSG_FATALERROR */
	#define ShowFatalError(string...) _ShowMessage(MSG_FATALERROR, string, __VA_ARGS__)
//	#define DisplayFatalError(string...) _ShowMessage(MSG_ERROR, string, __VA_ARGS__)
//	#define Terminate(string...) _ShowMessage(MSG_FATALERROR, string, __VA_ARGS__)
//	#define Kill(string...) _ShowMessage(MSG_FATALERROR, string, __VA_ARGS__)
//	#define AbortEx(string...) _ShowMessage(MSG_FATALERROR, string, __VA_ARGS__)

#else

#endif

#endif
