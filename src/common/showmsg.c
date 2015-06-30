// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/strlib.h" // StringBuf
#include "showmsg.h"
#include "core.h" //[Ind] - For SERVER_TYPE

#include <time.h>
#include <stdlib.h> // atexit

#ifdef WIN32
	#include "../common/winapi.h"

	#ifdef DEBUGLOGMAP
		#define DEBUGLOGPATH "log\\map-server.log"
	#else
		#ifdef DEBUGLOGCHAR
			#define DEBUGLOGPATH "log\\char-server.log"
		#else
			#ifdef DEBUGLOGLOGIN
				#define DEBUGLOGPATH "log\\login-server.log"
			#endif
		#endif
	#endif
#else
	#include <unistd.h>

	#ifdef DEBUGLOGMAP
		#define DEBUGLOGPATH "log/map-server.log"
	#else
		#ifdef DEBUGLOGCHAR
			#define DEBUGLOGPATH "log/char-server.log"
		#else
			#ifdef DEBUGLOGLOGIN
				#define DEBUGLOGPATH "log/login-server.log"
			#endif
		#endif
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////
/// behavioral parameter.
/// when redirecting output:
/// if true prints escape sequences
/// if false removes the escape sequences
int stdout_with_ansisequence = 0;

int msg_silent = 0; //Specifies how silent the console is.
int console_msg_log = 0;//[Ind] msg error logging
char console_log_filepath[32] = "./log/unknown.log";

///////////////////////////////////////////////////////////////////////////////
/// static/dynamic buffer for the messages

#define SBUF_SIZE 2054 // never put less that what's required for the debug message

#define NEWBUF(buf)				\
	struct {					\
		char s_[SBUF_SIZE];		\
		StringBuf *d_;			\
		char *v_;				\
		int l_;					\
	} buf ={"",NULL,NULL,0};	\
//define NEWBUF

#define BUFVPRINTF(buf,fmt,args)						\
	buf.l_ = vsnprintf(buf.s_, SBUF_SIZE, fmt, args);	\
	if( buf.l_ >= 0 && buf.l_ < SBUF_SIZE )				\
	{/* static buffer */								\
		buf.v_ = buf.s_;								\
	}													\
	else												\
	{/* dynamic buffer */								\
		buf.d_ = StringBuf_Malloc();					\
		buf.l_ = StringBuf_Vprintf(buf.d_, fmt, args);	\
		buf.v_ = StringBuf_Value(buf.d_);				\
		ShowDebug("showmsg: dynamic buffer used, increase the static buffer size to %d or more.\n", buf.l_+1);\
	}													\
//define BUFVPRINTF

#define BUFVAL(buf) buf.v_
#define BUFLEN(buf) buf.l_

#define FREEBUF(buf)			\
	if( buf.d_ )				\
	{							\
		StringBuf_Free(buf.d_);	\
		buf.d_ = NULL;			\
	}							\
	buf.v_ = NULL;				\
//define FREEBUF

///////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
// XXX adapted from eApp (comments are left untouched) [flaviojs]

///////////////////////////////////////////////////////////////////////////////
//  ansi compatible printf with control sequence parser for windows
//  fast hack, handle with care, not everything implemented
//
// \033[#;...;#m - Set Graphics Rendition (SGR) 
//
//  printf("\x1b[1;31;40m");	// Bright red on black
//  printf("\x1b[3;33;45m");	// Blinking yellow on magenta (blink not implemented)
//  printf("\x1b[1;30;47m");	// Bright black (grey) on dim white
//
//  Style           Foreground      Background
//  1st Digit       2nd Digit       3rd Digit		RGB
//  0 - Reset       30 - Black      40 - Black		000
//  1 - FG Bright   31 - Red        41 - Red		100
//  2 - Unknown     32 - Green      42 - Green		010
//  3 - Blink       33 - Yellow     43 - Yellow		110
//  4 - Underline   34 - Blue       44 - Blue		001
//  5 - BG Bright   35 - Magenta    45 - Magenta	101
//  6 - Unknown     36 - Cyan       46 - Cyan		011
//  7 - Reverse     37 - White      47 - White		111
//  8 - Concealed (invisible)
//
// \033[#A - Cursor Up (CUU)
//    Moves the cursor up by the specified number of lines without changing columns. 
//    If the cursor is already on the top line, this sequence is ignored. \e[A is equivalent to \e[1A.
//
// \033[#B - Cursor Down (CUD)
//    Moves the cursor down by the specified number of lines without changing columns. 
//    If the cursor is already on the bottom line, this sequence is ignored. \e[B is equivalent to \e[1B.
//
// \033[#C - Cursor Forward (CUF)
//    Moves the cursor forward by the specified number of columns without changing lines. 
//    If the cursor is already in the rightmost column, this sequence is ignored. \e[C is equivalent to \e[1C.
//
// \033[#D - Cursor Backward (CUB)
//    Moves the cursor back by the specified number of columns without changing lines. 
//    If the cursor is already in the leftmost column, this sequence is ignored. \e[D is equivalent to \e[1D.
//
// \033[#E - Cursor Next Line (CNL)
//    Moves the cursor down the indicated # of rows, to column 1. \e[E is equivalent to \e[1E.
//
// \033[#F - Cursor Preceding Line (CPL)
//    Moves the cursor up the indicated # of rows, to column 1. \e[F is equivalent to \e[1F.
//
// \033[#G - Cursor Horizontal Absolute (CHA)
//    Moves the cursor to indicated column in current row. \e[G is equivalent to \e[1G.
//
// \033[#;#H - Cursor Position (CUP)
//    Moves the cursor to the specified position. The first # specifies the line number, 
//    the second # specifies the column. If you do not specify a position, the cursor moves to the home position: 
//    the upper-left corner of the screen (line 1, column 1).
//
// \033[#;#f - Horizontal & Vertical Position
//    (same as \033[#;#H)
//
// \033[s - Save Cursor Position (SCP)
//    The current cursor position is saved. 
//
// \033[u - Restore cursor position (RCP)
//    Restores the cursor position saved with the (SCP) sequence \033[s.
//    (addition, restore to 0,0 if nothinh was saved before)
//

// \033[#J - Erase Display (ED)
//    Clears the screen and moves to the home position
//    \033[0J - Clears the screen from cursor to end of display. The cursor position is unchanged. (default)
//    \033[1J - Clears the screen from start to cursor. The cursor position is unchanged.
//    \033[2J - Clears the screen and moves the cursor to the home position (line 1, column 1).
//
// \033[#K - Erase Line (EL)
//    Clears the current line from the cursor position
//    \033[0K - Clears all characters from the cursor position to the end of the line (including the character at the cursor position). The cursor position is unchanged. (default)
//    \033[1K - Clears all characters from start of line to the cursor position. (including the character at the cursor position). The cursor position is unchanged.
//    \033[2K - Clears all characters of the whole line. The cursor position is unchanged.


/*
not implemented

\033[#L
IL: Insert Lines: The cursor line and all lines below it move down # lines, leaving blank space. The cursor position is unchanged. The bottommost # lines are lost. \e[L is equivalent to \e[1L.
\033[#M
DL: Delete Line: The block of # lines at and below the cursor are deleted; all lines below them move up # lines to fill in the gap, leaving # blank lines at the bottom of the screen. The cursor position is unchanged. \e[M is equivalent to \e[1M.
\033[#\@
ICH: Insert CHaracter: The cursor character and all characters to the right of it move right # columns, leaving behind blank space. The cursor position is unchanged. The rightmost # characters on the line are lost. \e[\@ is equivalent to \e[1\@.
\033[#P
DCH: Delete CHaracter: The block of # characters at and to the right of the cursor are deleted; all characters to the right of it move left # columns, leaving behind blank space. The cursor position is unchanged. \e[P is equivalent to \e[1P.

Escape sequences for Select Character Set
*/

#define is_console(handle) (FILE_TYPE_CHAR==GetFileType(handle))

///////////////////////////////////////////////////////////////////////////////
int	VFPRINTF(HANDLE handle, const char *fmt, va_list argptr)
{
	/////////////////////////////////////////////////////////////////
	/* XXX Two streams are being used. Disabled to avoid inconsistency [flaviojs]
	static COORD saveposition = {0,0};
	*/

	/////////////////////////////////////////////////////////////////
	DWORD written;
	char *p, *q;
	NEWBUF(tempbuf); // temporary buffer

	if(!fmt || !*fmt)
		return 0;

	// Print everything to the buffer
	BUFVPRINTF(tempbuf,fmt,argptr);

	if( !is_console(handle) && stdout_with_ansisequence )
	{
		WriteFile(handle, BUFVAL(tempbuf), BUFLEN(tempbuf), &written, 0);
		return 0;
	}

	// start with processing
	p = BUFVAL(tempbuf);
	while ((q = strchr(p, 0x1b)) != NULL)
	{	// find the escape character
		if( 0==WriteConsole(handle, p, (DWORD)(q-p), &written, 0) ) // write up to the escape
			WriteFile(handle, p, (DWORD)(q-p), &written, 0);

		if( q[1]!='[' )
		{	// write the escape char (whatever purpose it has) 
			if(0==WriteConsole(handle, q, 1, &written, 0) )
				WriteFile(handle,q, 1, &written, 0);
			p=q+1;	//and start searching again
		}
		else
		{	// from here, we will skip the '\033['
			// we break at the first unprocessible position
			// assuming regular text is starting there
			uint8 numbers[16], numpoint=0;
			CONSOLE_SCREEN_BUFFER_INFO info;

			// initialize
			GetConsoleScreenBufferInfo(handle, &info);
			memset(numbers,0,sizeof(numbers));

			// skip escape and bracket
			q=q+2;
			for(;;)
			{
				if( ISDIGIT(*q) ) 
				{	// add number to number array, only accept 2digits, shift out the rest
					// so // \033[123456789m will become \033[89m
					numbers[numpoint] = (numbers[numpoint]<<4) | (*q-'0');
					++q;
					// and next character
					continue;
				}
				else if( *q == ';' )
				{	// delimiter
					if(numpoint<sizeof(numbers)/sizeof(*numbers))
					{	// go to next array position
						numpoint++;
					}
					else
					{	// array is full, so we 'forget' the first value
						memmove(numbers,numbers+1,sizeof(numbers)/sizeof(*numbers)-1);
						numbers[sizeof(numbers)/sizeof(*numbers)-1]=0;
					}
					++q;
					// and next number
					continue;
				}
				else if( *q == 'm' )
				{	// \033[#;...;#m - Set Graphics Rendition (SGR)
					uint8 i;
					for(i=0; i<= numpoint; ++i)
					{
						if( 0x00 == (0xF0 & numbers[i]) )
						{	// upper nibble 0
							if( 0 == numbers[i] )
							{	// reset
								info.wAttributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
							}
							else if( 1==numbers[i] )
							{	// set foreground intensity
								info.wAttributes |= FOREGROUND_INTENSITY;
							}
							else if( 5==numbers[i] )
							{	// set background intensity
								info.wAttributes |= BACKGROUND_INTENSITY;
							}
							else if( 7==numbers[i] )
							{	// reverse colors (just xor them)
								info.wAttributes ^= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
													BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
							}
							//case '2': // not existing
							//case '3':	// blinking (not implemented)
							//case '4':	// unterline (not implemented)
							//case '6': // not existing
							//case '8': // concealed (not implemented)
							//case '9': // not existing
						}
						else if( 0x20 == (0xF0 & numbers[i]) )
						{	// off

							if( 1==numbers[i] )
							{	// set foreground intensity off
								info.wAttributes &= ~FOREGROUND_INTENSITY;
							}
							else if( 5==numbers[i] )
							{	// set background intensity off
								info.wAttributes &= ~BACKGROUND_INTENSITY;
							}
							else if( 7==numbers[i] )
							{	// reverse colors (just xor them)
								info.wAttributes ^= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE |
													BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
							}
						}
						else if( 0x30 == (0xF0 & numbers[i]) )
						{	// foreground
							uint8 num = numbers[i]&0x0F;
							if(num==9) info.wAttributes |= FOREGROUND_INTENSITY;
							if(num>7) num=7;	// set white for 37, 38 and 39
							info.wAttributes &= ~(FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
							if( (num & 0x01)>0 ) // lowest bit set = red
								info.wAttributes |= FOREGROUND_RED;
							if( (num & 0x02)>0 ) // second bit set = green
								info.wAttributes |= FOREGROUND_GREEN;
							if( (num & 0x04)>0 ) // third bit set = blue
								info.wAttributes |= FOREGROUND_BLUE;
						}
						else if( 0x40 == (0xF0 & numbers[i]) )
						{	// background
							uint8 num = numbers[i]&0x0F;
							if(num==9) info.wAttributes |= BACKGROUND_INTENSITY;
							if(num>7) num=7;	// set white for 47, 48 and 49
							info.wAttributes &= ~(BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
							if( (num & 0x01)>0 ) // lowest bit set = red
								info.wAttributes |= BACKGROUND_RED;
							if( (num & 0x02)>0 ) // second bit set = green
								info.wAttributes |= BACKGROUND_GREEN;
							if( (num & 0x04)>0 ) // third bit set = blue
								info.wAttributes |= BACKGROUND_BLUE;
						}
					}
					// set the attributes
					SetConsoleTextAttribute(handle, info.wAttributes);
				}
				else if( *q=='J' )
				{	// \033[#J - Erase Display (ED)
					//    \033[0J - Clears the screen from cursor to end of display. The cursor position is unchanged.
					//    \033[1J - Clears the screen from start to cursor. The cursor position is unchanged.
					//    \033[2J - Clears the screen and moves the cursor to the home position (line 1, column 1).
					uint8 num = (numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F);
					int cnt;
					DWORD tmp;
					COORD origin = {0,0};
					if(num==1)
					{	// chars from start up to and including cursor
						cnt = info.dwSize.X * info.dwCursorPosition.Y + info.dwCursorPosition.X + 1;
					}
					else if(num==2)
					{	// Number of chars on screen.
						cnt = info.dwSize.X * info.dwSize.Y;
						SetConsoleCursorPosition(handle, origin); 
					}
					else// 0 and default
					{	// number of chars from cursor to end
						origin = info.dwCursorPosition;
						cnt = info.dwSize.X * (info.dwSize.Y - info.dwCursorPosition.Y) - info.dwCursorPosition.X; 
					}
					FillConsoleOutputAttribute(handle, info.wAttributes, cnt, origin, &tmp);
					FillConsoleOutputCharacter(handle, ' ',              cnt, origin, &tmp);
				}
				else if( *q=='K' )
				{	// \033[K  : clear line from actual position to end of the line
					//    \033[0K - Clears all characters from the cursor position to the end of the line.
					//    \033[1K - Clears all characters from start of line to the cursor position.
					//    \033[2K - Clears all characters of the whole line.

					uint8 num = (numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F);
					COORD origin = {0,info.dwCursorPosition.Y}; //warning C4204
					SHORT cnt;
					DWORD tmp;
					if(num==1)
					{
						cnt = info.dwCursorPosition.X + 1;
					}
					else if(num==2)
					{
						cnt = info.dwSize.X;
					}
					else// 0 and default
					{
						origin = info.dwCursorPosition;
						cnt = info.dwSize.X - info.dwCursorPosition.X; // how many spaces until line is full
					}
					FillConsoleOutputAttribute(handle, info.wAttributes, cnt, origin, &tmp);
					FillConsoleOutputCharacter(handle, ' ',              cnt, origin, &tmp);
				}
				else if( *q == 'H' || *q == 'f' )
				{	// \033[#;#H - Cursor Position (CUP)
					// \033[#;#f - Horizontal & Vertical Position
					// The first # specifies the line number, the second # specifies the column. 
					// The default for both is 1
					info.dwCursorPosition.X = (numbers[numpoint])?(numbers[numpoint]>>4)*10+((numbers[numpoint]&0x0F)-1):0;
					info.dwCursorPosition.Y = (numpoint && numbers[numpoint-1])?(numbers[numpoint-1]>>4)*10+((numbers[numpoint-1]&0x0F)-1):0;

					if( info.dwCursorPosition.X >= info.dwSize.X ) info.dwCursorPosition.Y = info.dwSize.X-1;
					if( info.dwCursorPosition.Y >= info.dwSize.Y ) info.dwCursorPosition.Y = info.dwSize.Y-1;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q=='s' )
				{	// \033[s - Save Cursor Position (SCP)
					/* XXX Two streams are being used. Disabled to avoid inconsistency [flaviojs]
					CONSOLE_SCREEN_BUFFER_INFO info;
					GetConsoleScreenBufferInfo(handle, &info);
					saveposition = info.dwCursorPosition;
					*/
				}
				else if( *q=='u' )
				{	// \033[u - Restore cursor position (RCP)
					/* XXX Two streams are being used. Disabled to avoid inconsistency [flaviojs]
					SetConsoleCursorPosition(handle, saveposition);
					*/
				}
				else if( *q == 'A' )
				{	// \033[#A - Cursor Up (CUU)
					// Moves the cursor UP # number of lines
					info.dwCursorPosition.Y -= (numbers[numpoint])?(numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F):1;

					if( info.dwCursorPosition.Y < 0 )
						info.dwCursorPosition.Y = 0;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q == 'B' )
				{	// \033[#B - Cursor Down (CUD)
					// Moves the cursor DOWN # number of lines
					info.dwCursorPosition.Y += (numbers[numpoint])?(numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F):1;

					if( info.dwCursorPosition.Y >= info.dwSize.Y )
						info.dwCursorPosition.Y = info.dwSize.Y-1;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q == 'C' )
				{	// \033[#C - Cursor Forward (CUF)
					// Moves the cursor RIGHT # number of columns
					info.dwCursorPosition.X += (numbers[numpoint])?(numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F):1;

					if( info.dwCursorPosition.X >= info.dwSize.X )
						info.dwCursorPosition.X = info.dwSize.X-1;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q == 'D' )
				{	// \033[#D - Cursor Backward (CUB)
					// Moves the cursor LEFT # number of columns
					info.dwCursorPosition.X -= (numbers[numpoint])?(numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F):1;

					if( info.dwCursorPosition.X < 0 )
						info.dwCursorPosition.X = 0;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q == 'E' )
				{	// \033[#E - Cursor Next Line (CNL)
					// Moves the cursor down the indicated # of rows, to column 1
					info.dwCursorPosition.Y += (numbers[numpoint])?(numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F):1;
					info.dwCursorPosition.X = 0;

					if( info.dwCursorPosition.Y >= info.dwSize.Y )
						info.dwCursorPosition.Y = info.dwSize.Y-1;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q == 'F' )
				{	// \033[#F - Cursor Preceding Line (CPL)
					// Moves the cursor up the indicated # of rows, to column 1.
					info.dwCursorPosition.Y -= (numbers[numpoint])?(numbers[numpoint]>>4)*10+(numbers[numpoint]&0x0F):1;
					info.dwCursorPosition.X = 0;

					if( info.dwCursorPosition.Y < 0 )
						info.dwCursorPosition.Y = 0;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q == 'G' )
				{	// \033[#G - Cursor Horizontal Absolute (CHA)
					// Moves the cursor to indicated column in current row.
					info.dwCursorPosition.X = (numbers[numpoint])?(numbers[numpoint]>>4)*10+((numbers[numpoint]&0x0F)-1):0;

					if( info.dwCursorPosition.X >= info.dwSize.X )
						info.dwCursorPosition.X = info.dwSize.X-1;
					SetConsoleCursorPosition(handle, info.dwCursorPosition);
				}
				else if( *q == 'L' || *q == 'M' || *q == '@' || *q == 'P')
				{	// not implemented, just skip
				}
				else
				{	// no number nor valid sequencer
					// something is fishy, we break and give the current char free
					--q;
				}
				// skip the sequencer and search again
				p = q+1; 
				break;
			}// end while
		}
	}
	if (*p)	// write the rest of the buffer
		if( 0==WriteConsole(handle, p, (DWORD)strlen(p), &written, 0) )
			WriteFile(handle, p, (DWORD)strlen(p), &written, 0);
	FREEBUF(tempbuf);
	return 0;
}

int	FPRINTF(HANDLE handle, const char *fmt, ...)
{
	int ret;
	va_list argptr;
	va_start(argptr, fmt);
	ret = VFPRINTF(handle,fmt,argptr);
	va_end(argptr);
	return ret;
}

#define FFLUSH(handle)

#define STDOUT GetStdHandle(STD_OUTPUT_HANDLE)
#define STDERR GetStdHandle(STD_ERROR_HANDLE)

#else // not _WIN32


#define is_console(file) (0!=isatty(fileno(file)))

//vprintf_without_ansiformats
int	VFPRINTF(FILE *file, const char *fmt, va_list argptr)
{
	char *p, *q;
	NEWBUF(tempbuf); // temporary buffer

	if(!fmt || !*fmt)
		return 0;

	if( is_console(file) || stdout_with_ansisequence )
	{
		vfprintf(file, fmt, argptr);
		return 0;
	}

	// Print everything to the buffer
	BUFVPRINTF(tempbuf,fmt,argptr);

	// start with processing
	p = BUFVAL(tempbuf);
	while ((q = strchr(p, 0x1b)) != NULL)
	{	// find the escape character
		fprintf(file, "%.*s", (int)(q-p), p); // write up to the escape
		if( q[1]!='[' )
		{	// write the escape char (whatever purpose it has) 
			fprintf(file, "%.*s", 1, q);
			p=q+1;	//and start searching again
		}
		else
		{	// from here, we will skip the '\033['
			// we break at the first unprocessible position
			// assuming regular text is starting there

			// skip escape and bracket
			q=q+2;
			while(1)
			{
				if( ISDIGIT(*q) ) 
				{
					++q;
					// and next character
					continue;
				}
				else if( *q == ';' )
				{	// delimiter
					++q;
					// and next number
					continue;
				}
				else if( *q == 'm' )
				{	// \033[#;...;#m - Set Graphics Rendition (SGR)
					// set the attributes
				}
				else if( *q=='J' )
				{	// \033[#J - Erase Display (ED)
				}
				else if( *q=='K' )
				{	// \033[K  : clear line from actual position to end of the line
				}
				else if( *q == 'H' || *q == 'f' )
				{	// \033[#;#H - Cursor Position (CUP)
					// \033[#;#f - Horizontal & Vertical Position
				}
				else if( *q=='s' )
				{	// \033[s - Save Cursor Position (SCP)
				}
				else if( *q=='u' )
				{	// \033[u - Restore cursor position (RCP)
				}
				else if( *q == 'A' )
				{	// \033[#A - Cursor Up (CUU)
					// Moves the cursor UP # number of lines
				}
				else if( *q == 'B' )
				{	// \033[#B - Cursor Down (CUD)
					// Moves the cursor DOWN # number of lines
				}
				else if( *q == 'C' )
				{	// \033[#C - Cursor Forward (CUF)
					// Moves the cursor RIGHT # number of columns
				}
				else if( *q == 'D' )
				{	// \033[#D - Cursor Backward (CUB)
					// Moves the cursor LEFT # number of columns
				}
				else if( *q == 'E' )
				{	// \033[#E - Cursor Next Line (CNL)
					// Moves the cursor down the indicated # of rows, to column 1
				}
				else if( *q == 'F' )
				{	// \033[#F - Cursor Preceding Line (CPL)
					// Moves the cursor up the indicated # of rows, to column 1.
				}
				else if( *q == 'G' )
				{	// \033[#G - Cursor Horizontal Absolute (CHA)
					// Moves the cursor to indicated column in current row.
				}
				else if( *q == 'L' || *q == 'M' || *q == '@' || *q == 'P')
				{	// not implemented, just skip
				}
				else
				{	// no number nor valid sequencer
					// something is fishy, we break and give the current char free
					--q;
				}
				// skip the sequencer and search again
				p = q+1; 
				break;
			}// end while
		}
	}
	if (*p)	// write the rest of the buffer
		fprintf(file, "%s", p);
	FREEBUF(tempbuf);
	return 0;
}
int	FPRINTF(FILE *file, const char *fmt, ...)
{
	int ret;
	va_list argptr;
	va_start(argptr, fmt);
	ret = VFPRINTF(file,fmt,argptr);
	va_end(argptr);
	return ret;
}

#define FFLUSH fflush

#define STDOUT stdout
#define STDERR stderr

#endif// not _WIN32

char timestamp_format[20] = ""; //For displaying Timestamps

int _vShowMessage(enum msg_type flag, const char *string, va_list ap)
{
	va_list apcopy;
	char prefix[100];
#if defined(DEBUGLOGMAP) || defined(DEBUGLOGCHAR) || defined(DEBUGLOGLOGIN)
	FILE *fp;
#endif
	
	if (!string || *string == '\0') {
		ShowError("Empty string passed to _vShowMessage().\n");
		return 1;
	}
	/**
	 * For the buildbot, these result in a EXIT_FAILURE from core.c when done reading the params.
	 **/
#if defined(BUILDBOT)
	if( flag == MSG_WARNING ||
	    flag == MSG_ERROR ||
	    flag == MSG_SQL ) {
		buildbotflag = 1;
	}
#endif
	if(
		( flag == MSG_WARNING && console_msg_log&1 ) ||
		( ( flag == MSG_ERROR || flag == MSG_SQL ) && console_msg_log&2 ) ||
		( flag == MSG_DEBUG && console_msg_log&4 ) ) {//[Ind]
		FILE *log = NULL;
		if( (log = fopen(console_log_filepath ? console_log_filepath : "./log/unknown.log","a+")) ) {
			char timestring[255];
			time_t curtime;
			time(&curtime);
			strftime(timestring, 254, "%m/%d/%Y %H:%M:%S", localtime(&curtime));
			fprintf(log,"(%s) [ %s ] : ",
				timestring,
				flag == MSG_WARNING ? "Warning" :
				flag == MSG_ERROR ? "Error" :
				flag == MSG_SQL ? "SQL Error" :
				flag == MSG_DEBUG ? "Debug" :
				"Unknown");
			va_copy(apcopy, ap);
			vfprintf(log,string,apcopy);
			va_end(apcopy);
			fclose(log);
		}
	}
	if(
	    (flag == MSG_INFORMATION && msg_silent&1) ||
	    (flag == MSG_STATUS && msg_silent&2) ||
	    (flag == MSG_NOTICE && msg_silent&4) ||
	    (flag == MSG_WARNING && msg_silent&8) ||
	    (flag == MSG_ERROR && msg_silent&16) ||
	    (flag == MSG_SQL && msg_silent&16) ||
	    (flag == MSG_DEBUG && msg_silent&32)
	)
		return 0; //Do not print it.

	if (timestamp_format[0] && flag != MSG_NONE)
	{	//Display time format. [Skotlex]
		time_t t = time(NULL);
		strftime(prefix, 80, timestamp_format, localtime(&t));
	} else prefix[0]='\0';

	switch (flag) {
		case MSG_NONE: // direct printf replacement
			break;
		case MSG_STATUS: //Bright Green (To inform about good things)
			strcat(prefix,CL_GREEN"[Status]"CL_RESET":");
			break;
		case MSG_SQL: //Bright Violet (For dumping out anything related with SQL) <- Actually, this is mostly used for SQL errors with the database, as successes can as well just be anything else... [Skotlex]
			strcat(prefix,CL_MAGENTA"[SQL]"CL_RESET":");
			break;
		case MSG_INFORMATION: //Bright White (Variable information)
			strcat(prefix,CL_WHITE"[Info]"CL_RESET":");
			break;
		case MSG_NOTICE: //Bright White (Less than a warning)
			strcat(prefix,CL_WHITE"[Notice]"CL_RESET":");
			break;
		case MSG_WARNING: //Bright Yellow
			strcat(prefix,CL_YELLOW"[Warning]"CL_RESET":");
			break;
		case MSG_DEBUG: //Bright Cyan, important stuff!
			strcat(prefix,CL_CYAN"[Debug]"CL_RESET":");
			break;
		case MSG_ERROR: //Bright Red  (Regular errors)
			strcat(prefix,CL_RED"[Error]"CL_RESET":");
			break;
		case MSG_FATALERROR: //Bright Red (Fatal errors, abort(); if possible)
			strcat(prefix,CL_RED"[Fatal Error]"CL_RESET":");
			break;
		default:
			ShowError("In function _vShowMessage() -> Invalid flag passed.\n");
			return 1;
	}

	if (flag == MSG_ERROR || flag == MSG_FATALERROR || flag == MSG_SQL)
	{	//Send Errors to StdErr [Skotlex]
		FPRINTF(STDERR, "%s ", prefix);
		va_copy(apcopy, ap);
		VFPRINTF(STDERR, string, apcopy);
		va_end(apcopy);
		FFLUSH(STDERR);
	} else {
		if (flag != MSG_NONE)
			FPRINTF(STDOUT, "%s ", prefix);
		va_copy(apcopy, ap);
		VFPRINTF(STDOUT, string, apcopy);
		va_end(apcopy);
		FFLUSH(STDOUT);
	}

#if defined(DEBUGLOGMAP) || defined(DEBUGLOGCHAR) || defined(DEBUGLOGLOGIN)
	if(strlen(DEBUGLOGPATH) > 0) {
		fp=fopen(DEBUGLOGPATH,"a");
		if (fp == NULL)	{
			FPRINTF(STDERR, CL_RED"[ERROR]"CL_RESET": Could not open '"CL_WHITE"%s"CL_RESET"', access denied.\n", DEBUGLOGPATH);
			FFLUSH(STDERR);
		} else {
			fprintf(fp,"%s ", prefix);
			va_copy(apcopy, ap);
			vfprintf(fp,string,apcopy);
			va_end(apcopy);
			fclose(fp);
		}
	} else {
		FPRINTF(STDERR, CL_RED"[ERROR]"CL_RESET": DEBUGLOGPATH not defined!\n");
		FFLUSH(STDERR);
	}
#endif

	return 0;
}

void ClearScreen(void)
{
#ifndef _WIN32
	ShowMessage(CL_CLS);	// to prevent empty string passed messages
#endif
}
int _ShowMessage(enum msg_type flag, const char *string, ...)
{
	int ret;
	va_list ap;
	va_start(ap, string);
	ret = _vShowMessage(flag, string, ap);
	va_end(ap);
	return ret;
}

// direct printf replacement
void ShowMessage(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_NONE, string, ap);
	va_end(ap);
}
void ShowStatus(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_STATUS, string, ap);
	va_end(ap);
}
void ShowSQL(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_SQL, string, ap);
	va_end(ap);
}
void ShowInfo(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_INFORMATION, string, ap);
	va_end(ap);
}
void ShowNotice(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_NOTICE, string, ap);
	va_end(ap);
}
void ShowWarning(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_WARNING, string, ap);
	va_end(ap);
}
void ShowConfigWarning(config_setting_t *config, const char *string, ...)
{
	StringBuf buf;
	va_list ap;
	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, string);
	StringBuf_Printf(&buf, " (%s:%d)\n", config_setting_source_file(config), config_setting_source_line(config));
	va_start(ap, string);
	_vShowMessage(MSG_WARNING, StringBuf_Value(&buf), ap);
	va_end(ap);
	StringBuf_Destroy(&buf);
}
void ShowDebug(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_DEBUG, string, ap);
	va_end(ap);
}
void ShowError(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_ERROR, string, ap);
	va_end(ap);
}
void ShowFatalError(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_FATALERROR, string, ap);
	va_end(ap);
}
