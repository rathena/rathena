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

extern int _ShowMessage(const char *string, enum msg_type flag);

/* MSG_XX */
	#define ShowMsg(string,flag) _ShowMessage(string,flag)
//	#define DisplayMsg(string,flag) _ShowMessage(string,flag)
//	#define ShowMessage(string,flag) _ShowMessage(string,flag)

/* MSG_STATUS */
	#define ShowStatus(string) _ShowMessage(string,MSG_STATUS)
//	#define DisplayStatus(string) _ShowMessage(string,MSG_STATUS)

/* MSG_SQL*/
	#define ShowSQL(string) _ShowMessage(string,MSG_SQL)
//	#define DisplaySQL(string) _ShowMessage(string,MSG_SQL)

/* MSG_INFORMATION */
	#define ShowInfo(string) _ShowMessage(string,MSG_INFORMATION)
//	#define DisplayInfo(string) _ShowMessage(string,MSG_INFORMATION)
//	#define ShowInformation(string) _ShowMessage(string,MSG_INFORMATION)
//	#define DisplayInformation(string) _ShowMessage(string,MSG_INFORMATION)

/* MSG_NOTICE */
	#define ShowNotice(string) _ShowMessage(string,MSG_NOTICE)
//	#define DisplayNotice(string) _ShowMessage(string,MSG_NOTICE)

/* MSG_WARNING */
	#define ShowWarning(string) _ShowMessage(string,MSG_WARNING)
//	#define DisplayWarning(string) _ShowMessage(string,MSG_WARNING)
//	#define Warn(string) _ShowMessage(string,MSG_WARNING)

/* MSG_DEBUG */
	#define ShowDebug(string) _ShowMessage(string,MSG_DEBUG)
	#define DisplayDebug(string) _ShowMessage(string,MSG_DEBUG)
	#define Debug(string) _ShowMessage(string,MSG_DEBUG)
	#define printDebug() _ShowMessage(striing,MSG_DEBUG)

/* MSG_ERROR */
	#define ShowError(string) _ShowMessage(string,MSG_ERROR)
//	#define DisplayError(string) _ShowMessage(string,MSG_ERROR)
//	#define OutputError(string) _ShowMessage(string,MSG_ERROR)

/* MSG_FATALERROR */
	#define ShowFatalError(string) _ShowMessage(string,MSG_FATALERROR)
//	#define DisplayFatalError(string) _ShowMessage(string,MSG_ERROR)
//	#define Terminate(string) _ShowMessage(string,MSG_FATALERROR)
//	#define Kill(string) _ShowMessage(string,MSG_FATALERROR)
//	#define AbortEx(string) _ShowMessage(string,MSG_FATALERROR)

#endif
