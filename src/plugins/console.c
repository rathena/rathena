// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/plugin.h"

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <unistd.h>
	#include <poll.h>
	#include <string.h>
#endif
#include <stdio.h> // stdin, fgets

#ifdef WIN32

#define THREAD_FUNC_START(name) DWORD WINAPI thread_ ## name(LPVOID lpParameter) { (void)lpParameter; {
#define THREAD_FUNC_END(name) } ExitThread(0); return 0; }
#define THREAD_EXECUTE(name,errvar) \
	do{ \
		int _fail_ = (CreateThread(NULL,0,thread_ ## name,NULL,0,NULL) == NULL); \
		if( errvar ) \
			*errvar = _fail_; \
	}while(0)
#define sleep Sleep

#define pipe_create(p) (CreatePipe(&p[PIPE_READ], &p[PIPE_WRITE], NULL, 1) == 0)
#define pipe_read(p,data,len) do{ DWORD _b_; ReadFile(p[PIPE_READ], data, len, &_b_, NULL); }while(0)
#define pipe_write(p,data,len) do{ DWORD _b_; WriteFile(p[PIPE_WRITE], data, len, &_b_, NULL); }while(0)
#define pipe_close(p,side) CloseHandle(p[side])
typedef HANDLE PIPE[2];

#else

#define THREAD_FUNC_START(name) void thread_ ## name(void) {
#define THREAD_FUNC_END(name) _exit(0); }
#define THREAD_EXECUTE(name,errvar) \
	do{ \
		int pid = fork(); \
		if( pid == 0 ){ \
			thread_ ## name(); \
		} \
		if( errvar ) \
			*errvar = (pid == -1); \
	}while(0)

#define pipe_create(p) pipe(p)
#define pipe_read(p,data,len) read(p[PIPE_READ], data, len)
#define pipe_write(p,data,len) write(p[PIPE_WRITE], data, len)
#define pipe_close(p,side) close(p[side])
typedef int PIPE[2];

#endif

#define PIPE_READ 0
#define PIPE_WRITE 1
#define INPUT_BUFSIZE 4096

////// Plugin information ////////
//
PLUGIN_INFO = {
	"Console", // Name
	PLUGIN_ALL, // Target servers
	"0.1", // Version
	"1.03", // Minimum plugin engine version to run
	"Console parser" // Short description
};

////// Plugin event list //////////
// Format: <plugin function>,<event name>
// All registered functions to a event gets executed
// (In descending order) when its called.
// Multiple functions can be called by multiple events too,
// So it's up to your creativity ^^
//
PLUGIN_EVENTS_TABLE = {
	{ "console_init",  EVENT_PLUGIN_INIT },
	{ "console_final", EVENT_PLUGIN_FINAL },
	{ "console_start", EVENT_ATHENA_INIT },
	{ "console_stop",  EVENT_ATHENA_FINAL },
	{ NULL, NULL }
};

///// Variables /////

typedef int TimerFunc(int tid, unsigned int tick, int id, int data);
int (*add_timer_func_list)(TimerFunc func, char* name);
int (*add_timer_interval)(unsigned int tick, TimerFunc* func, int id, int data, int interval);
int (*delete_timer)(int tid, TimerFunc* func);
unsigned int (*gettick)(void);
int (*parse_console)(char* buf);

int tid;
PIPE data;
PIPE next;

//////// Plugin functions //////////

static int pipe_hasdata(PIPE p)
{
#ifdef WIN32
	return ( WaitForSingleObject(p[PIPE_READ],0) == WAIT_OBJECT_0 );
#else
	struct pollfd fds;
	fds.fd = p[PIPE_READ];
	fds.events = POLLRDNORM;
	return ( poll(&fds,1,0) > 0 );
#endif
}

int console_parsebuf(int tid, unsigned int tick, int id, int data_)
{
	//printf("console_parsebuf\n");
	//delete_timer(tid, console_parsebuf);
	if( pipe_hasdata(data) ){
		char buf[INPUT_BUFSIZE];
		size_t len;
		//printf("console_parsebuf pipe_hasdata\n");
		// receive string
		pipe_read(data, &len, sizeof(size_t));
		pipe_read(data, buf, len);
		buf[len] = '\0';
		//printf("console_parsebuf buf='%s'\n", buf);
		// parse it
		parse_console(buf);
		//printf("console_parsebuf writing next\n");
		// send next state
		buf[0] = 'R';
		pipe_write(next, buf, 1);
		//printf("console_parsebuf done with next\n");
	}
	return 0;
}

THREAD_FUNC_START(readinput)
	char buf[INPUT_BUFSIZE];
	char state = 'R';
	size_t len;

	//printf("thread_readinput START\n");
	pipe_close(data, PIPE_READ);
	pipe_close(next, PIPE_WRITE);
	buf[sizeof(buf)-1] = '\0';
	for( ; state != 'X'; )
	{
		//printf("thread_readinput getting data\n");
		buf[0] = '\0';
		fgets(buf, sizeof(buf)-1, stdin);
		len = strlen(buf);
		//printf("thread_readinput buf='%s'\n", buf);
		// send string
		pipe_write(data, &len, sizeof(size_t));
		pipe_write(data, buf, len);
		// receive next state
		pipe_read(next, &state, sizeof(char));
	}
	pipe_close(data, PIPE_WRITE);
	pipe_close(next, PIPE_READ);
	//printf("thread_readinput STOP (%d)\n", state);
THREAD_FUNC_END(readinput)

void console_start(void)
{
	int error = 0;
	//printf("console_start\n");
	if( pipe_create(data) ){
		//printf("console_start data pipe failed\n");
		return;
	}
	if( pipe_create(next) ){
		//printf("console_start next pipe failed\n");
		pipe_close(data, PIPE_READ);
		pipe_close(data, PIPE_WRITE);
		return;
	}
	THREAD_EXECUTE(readinput, &error);
	if( error ){
		//printf("console_start thread start error\n");
		pipe_close(data, PIPE_READ);
		pipe_close(next, PIPE_WRITE);
	} else {
		//printf("console_start thread started\n");
		//parse_console("help");
		add_timer_func_list(console_parsebuf,"console_parsebuf");
		tid = add_timer_interval(gettick(),console_parsebuf,0,0,1); // run once every cycle
	}
	pipe_close(data, PIPE_WRITE);
	pipe_close(next, PIPE_READ);
}

void console_stop(void)
{
	char c = 'X';
	//printf("console_stop\n");
	if( tid != -1 ){
		delete_timer(tid, console_parsebuf);
		pipe_write(next, &c, sizeof(char));
	}
	return;
}

void console_init(void)
{
	//printf("console_init\n");
	// import symbols
	IMPORT_SYMBOL(add_timer_interval, SYMBOL_ADD_TIMER_INTERVAL);
	IMPORT_SYMBOL(add_timer_func_list, SYMBOL_ADD_TIMER_FUNC_LIST);
	IMPORT_SYMBOL(delete_timer, SYMBOL_DELETE_TIMER);
	IMPORT_SYMBOL(gettick, SYMBOL_GETTICK);
	IMPORT_SYMBOL(parse_console, SYMBOL_PARSE_CONSOLE);
	//printf("%d -> add_timer_func_list=0x%x\n", SYMBOL_ADD_TIMER_FUNC_LIST, (int)add_timer_func_list);
	//printf("%d -> add_timer_interval=0x%x\n", SYMBOL_ADD_TIMER_INTERVAL, (int)add_timer_interval);
	//printf("%d -> delete_timer=0x%x\n", SYMBOL_DELETE_TIMER, (int)delete_timer);
	//printf("%d -> gettick=0x%x\n", SYMBOL_GETTICK, (int)gettick);
	//printf("%d -> parse_console=0x%x\n", SYMBOL_PARSE_CONSOLE, (int)parse_console);

	return;
}

void console_final(void)
{
	//printf("console_final\n");
	return;
}
