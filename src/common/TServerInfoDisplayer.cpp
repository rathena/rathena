// TServerInfoDisplayer.cpp: implementation of the TServerInfoDisplayer class.
//	GS-N	1.00.18	JPN	0x004A9500	-	Completed
//////////////////////////////////////////////////////////////////////

#include "winapi.h"
#include "TServerInfoDisplayer.h"
#include <stdio.h>
#include "core.h"

TServerInfoDisplayer g_ServerInfoDisplayer;
int tQueue = 0;


static char * ServerTypeText[] =
{
	0
};

static char * ErrorMessge[] = 
{
	0
};

TServerInfoDisplayer::TServerInfoDisplayer()
{
	this->InitGDIObject();
}

TServerInfoDisplayer::~TServerInfoDisplayer()
{
	this->DelGDIObject();
}


void TServerInfoDisplayer::InitGDIObject()
{
	this->m_hFont = CreateFont(80, 0, 0, 0, FW_THIN, FALSE, FALSE, FALSE, 
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, "Times");
}



void TServerInfoDisplayer::DelGDIObject()
{
	DeleteObject(this->m_hFont);
}


void TServerInfoDisplayer::Run(HWND hWnd)
{
	this->PaintAllInfo(hWnd, 0, 20);
}



void TServerInfoDisplayer::PaintAllInfo(HWND hWnd, int iTopLeftX, int iTopLeftY)
{
	HDC hDC = GetDC(hWnd);
	SIZE psize;
	int x,y;

	RECT rect;
	GetClientRect(hWnd, &rect);
	char tmsg[50]={0};

	SetBkMode(hDC, TRANSPARENT);

	SetTextColor(hDC, RGB(0, 0, 0));

	switch (SERVER_TYPE)
	{
	case ATHENA_SERVER_LOGIN:
		sprintf(tmsg, "Account Online : %d", loginusers);// , gGrantedctr, gFailedctr);
		break;
	}

	GetTextExtentPoint32A(hDC,tmsg,strlen(tmsg),&psize);
	
	y = rect.top + 5;
	x = (rect.right / 2) - (psize.cx / 2);
	
	TextOutA(hDC, x, y, tmsg, strlen(tmsg));
	
	ReleaseDC(hWnd, hDC);
}