#ifndef _SHOWMSG_H_
#define _SHOWMSG_H_

extern char tmp_output[1024];

enum msg_type {MSG_STATUS,/* MSG_SQL, */MSG_INFORMATION,MSG_NOTICE,MSG_WARNING,MSG_ERROR,MSG_FATALERROR};

extern int _ShowMessage(const char *string, enum msg_type flag);

/* MSG_XX */
	#define ShowMsg(string,flag) _ShowMessage(string,flag)
//	#define DisplayMsg(string,flag) _ShowMessage(string,flag)
//	#define ShowMessage(string,flag) _ShowMessage(string,flag)

/* MSG_STATUS */
	#define ShowStatus(string) _ShowMessage(string,MSG_STATUS)
//	#define DisplayStatus(string) _ShowMessage(string,MSG_STATUS)

/* MSG_SQL*/
//	#define ShowSQL(string) _ShowMessage(string,MSG_SQL)
//	#define DisplaySQL(string) _ShowMessage(string,MSG_SQL)

/* MSG_INFORMATION */
	#define ShowInfo(string) _ShowMessage(string,MSG_INFORMATION)
//	#define DisplayInfo(string) _ShowMessage(string,MSG_INFORMATION)
//	#define ShowInformation(string) _ShowMessage(string,MSG_INFORMATION)
//	#define DisplayInformation(string) _ShowMessage(string,MSG_INFORMATION)

/* MSG_NOTICE */
	#define ShowNotice(string) _ShowMessage(string,MSG_NOTICE)
//	#define DisplayNotice(string) _ShowMessage(string,MSG_NOTICE)

/*  */
	#define ShowWarning(string) _ShowMessage(string,MSG_WARNING)
//	#define DisplayWarning(string) _ShowMessage(string,MSG_WARNING)
//	#define Warn(string) _ShowMessage(string,MSG_WARNING)

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
