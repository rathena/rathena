#include "winapi.h"
#include "LogToFile.h"

#ifdef _GUI
char OldLogFileName[260]={0};

CLogToFile::~CLogToFile()
{
	delete[] this->m_cBUFFER;
	DeleteCriticalSection(&this->m_critLogToFile);
}

CLogToFile::CLogToFile()
{
}

void CLogToFile::Init(const LPSTR LogFileName,const LPSTR LogDirectoryName, BOOL bWithDate)
{
	this->m_cBUFFER = new char[1024];

	if (strcmp(LogFileName, "") == 0)
	{
		strcpy(this->m_szLogFileName , "LOGDATA");
	}
	else
	{
		strcpy(this->m_szLogFileName , LogFileName);
	}

	
	if (!strcmp(LogDirectoryName, ""))
	{
		strcpy(this->m_szLogDirectoryName , "LOG");
	}
	else
	{
		strcpy(this->m_szLogDirectoryName  , LogDirectoryName);
	}
	
	this->m_bWithDate  = bWithDate;	

	CreateDirectory(LogDirectoryName, NULL);

	InitializeCriticalSection(&this->m_critLogToFile );
	
}

void CLogToFile::Output2(char *sbuf)
{
	if (sbuf == NULL)
		return;

	char szLogFileName[260]; // Is StringZero Terminated
	SYSTEMTIME strSystime;
	EnterCriticalSection(&this->m_critLogToFile);
	GetLocalTime(&strSystime);


	wsprintf(szLogFileName, "%s\\%s_%04d-%02d-%02d.txt",
		&this->m_szLogDirectoryName[0], &this->m_szLogFileName[0],
		strSystime.wYear, strSystime.wMonth, strSystime.wDay);

	if (strncmp(szLogFileName, OldLogFileName, strlen(szLogFileName)) != 0)
	{
		if (this->m_fLogFile)
		{
			fclose(this->m_fLogFile);
		}

		this->m_fLogFile = fopen(szLogFileName, "a+");
		strcpy(OldLogFileName, szLogFileName);
	}

	if (this->m_fLogFile == NULL)
	{
		LeaveCriticalSection(&this->m_critLogToFile);
	}
	else
	{
		if (this->m_bWithDate == 0)
		{
			fprintf(this->m_fLogFile, "%s\n", sbuf);
		}
		else
		{
			fprintf(this->m_fLogFile, "%02d:%02d:%02d  %s\n", strSystime.wHour, strSystime.wMinute, strSystime.wSecond, sbuf);
		}

		LeaveCriticalSection(&this->m_critLogToFile);
	}
}

void CLogToFile::Output( LPSTR fmt, ...)
{
	va_list argptr;
	int iChrWritten;
	char szLogFileName[260]; // Is StringZero Terminated

	SYSTEMTIME strSystime;
	
	EnterCriticalSection(&this->m_critLogToFile );

	va_start(argptr, fmt);
	iChrWritten=vsprintf(this->m_cBUFFER , fmt, argptr);
	va_end(argptr);

	GetLocalTime(&strSystime);


	wsprintf(szLogFileName, "%s\\%s_%04d-%02d-%02d.txt",
		&this->m_szLogDirectoryName[0] , &this->m_szLogFileName [0],
		strSystime.wYear, strSystime.wMonth, strSystime.wDay);

	if(strncmp(szLogFileName,OldLogFileName,strlen(szLogFileName)) != 0)
	{
		if(this->m_fLogFile)
		{
			fclose(this->m_fLogFile );
		}

		this->m_fLogFile = fopen(szLogFileName, "a+t");
		strcpy(OldLogFileName,szLogFileName);
	}

	if ( this->m_fLogFile == NULL )
	{
		LeaveCriticalSection(&this->m_critLogToFile );	
	}
	else
	{
		if (this->m_bWithDate ==0)
		{
			fprintf(this->m_fLogFile , "%s\n", this->m_cBUFFER);
		}
		else
		{
			fprintf(this->m_fLogFile , "%02d:%02d:%02d  %s\n", strSystime.wHour, strSystime.wMinute, strSystime.wSecond, this->m_cBUFFER);
		}

		LeaveCriticalSection(&this->m_critLogToFile );
	}
}
#endif
