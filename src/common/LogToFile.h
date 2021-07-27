#pragma once
#ifdef _GUI

#include <stdio.h>

class CLogToFile
{

public:

	void __cdecl Output(LPSTR fmt, ...);
	void Output2(char *sbuf);
	CLogToFile();
	~CLogToFile();
	void Init(const LPSTR LogFileName, const LPSTR LogDirectoryName, BOOL bWithDate);

private:

	FILE* m_fLogFile;	// 0
	char m_szLogFileName[260]; // 4
	char m_szLogDirectoryName[260]; // 108
	char * m_cBUFFER;	// 20C
	BOOL m_bWithDate; // 1020C
	CRITICAL_SECTION m_critLogToFile; 
};

#endif