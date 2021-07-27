
#pragma once

#ifdef _GUI

#define	WM_LOG_PAINT 0x64
#define WM_QUEUE_MSG_PROC 0x65

enum{
	HRED,
	MRED,
	HBLUE,
	MBLUE,
	MGREEN,
	HBLACK,
	UKN,
};

void LogTextPaint(HWND hWnd);
void LogAdd2(BYTE Color, char* szLog, ...);
void LogAdd(BYTE Color, char* szLog);
void loginit();
void MsgBox(char *szlog, ...);

extern HWND ghWnd;
extern HFONT hFont;

#endif