// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/plugin.h"

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#define __USE_XOPEN
	#include <sys/types.h>
	#include <unistd.h>
	#include <poll.h>
	#include <string.h>
#endif
#include <stdio.h> // stdin, fgets

#define INPUT_BUFSIZE 4096
#define INPUT_INVALID 0
#define INPUT_READY   1
#define INPUT_WAITING 2
#define INPUT_READING 3
#define INPUT_CLOSED  4

//////////////////////////////
#ifdef WIN32
//////////////////////////////

// In windows the worker is a thread so it can access the same variables.
#define WORKER_FUNC_DECLARE(name) DWORD WINAPI worker_ ## name(LPVOID lpParameter)
#define WORKER_FUNC_START(name) DWORD WINAPI worker_ ## name(LPVOID lpParameter) { (void)lpParameter; {
#define WORKER_FUNC_END(name) } ExitThread(0); return 0; }
#define WORKER_EXECUTE(name,errvar) \
	do{ \
		buf.worker = CreateThread(NULL, 0, worker_ ## name, NULL, CREATE_SUSPENDED, NULL); \
		if( errvar ) \
			*errvar = ( buf.worker == NULL ); \
	}while(0)

/// Buffer for asynchronous input
typedef struct _buffer {
	char arr[INPUT_BUFSIZE];
	size_t len;
	HANDLE worker;
	HANDLE state_mux; // mutex for the state
	char state;
} BUFFER;

//////////////////////////////
#else
//////////////////////////////

/// In linux the worker is a process so it needs to comunicate through pipes.
#define WORKER_FUNC_DECLARE(name) void worker_ ## name(void)
#define WORKER_FUNC_START(name) void worker_ ## name(void) {
#define WORKER_FUNC_END(name) _exit(0); }
#define WORKER_EXECUTE(name,errvar) \
	do{ \
		int pid = fork(); \
		if( pid == 0 ){ \
			worker_ ## name(); \
		} \
		if( errvar ) \
			*errvar = (pid == -1); \
	}while(0)

#define PIPE_READ 0
#define PIPE_WRITE 1

/// Buffer for asynchronous input
typedef struct _buffer {
	char arr[INPUT_BUFSIZE];
	size_t len;
	int data_pipe[2]; // pipe to receive data
	int state_pipe[2]; // pipe to send state
	char state;
	unsigned close_unused_flag : 1;
} BUFFER;

//////////////////////////////
#endif
//////////////////////////////





////// Plugin information ////////





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
	{ "console_init",      EVENT_PLUGIN_INIT },
	{ "console_final",     EVENT_PLUGIN_FINAL },
	{ "console_autostart", EVENT_ATHENA_INIT },
	//{ "console_start",     EVENT_CONSOLE_START },//## add these events to the plugins framework
	//{ "console_stop",      EVENT_CONSOLE_STOP },
	{ "console_stop",      EVENT_ATHENA_FINAL },
	{ NULL, NULL }
};





///// Variables /////





// Imported functions
typedef int (*TimerFunc)(int tid, unsigned int tick, int id, intptr data);
int (*add_timer_func_list)(TimerFunc func, char* name);
int (*add_timer_interval)(unsigned int tick, TimerFunc func, int id, intptr data, int interval);
int (*delete_timer)(int tid, TimerFunc func);
unsigned int (*gettick)(void);
int (*parse_console)(char* buf);

// Locals
int tid; // timer id
BUFFER buf; // input buffer
WORKER_FUNC_DECLARE(getinput); // worker for the input buffer





//////// Asynchronous input functions //////////





//////////////////////////////
#ifdef WIN32
//////////////////////////////
//
// --=== Asynchronous console input ===--
//
// On windows a thread is used (both threads have access to the same data).
// The worker threads starts suspended and is resumed when data is required.
// After getting the data, the worker thread updates the state variable and 
// suspends itself.
//
// A mutex is used to synchronize access to the state variable between the 
// threads. Access and updates to state are probably already atomic so the 
// mutex shouldn't be needed, but using it is more correct so it stays.
//
// Note: The Worker thread only starts to get input data when further data is 
//    requested. This is a design choise and brings no real advantage or 
//    disadvantage I can think of.
//

/// Returns the state of the input
char input_getstate()
{
	char state;

	WaitForSingleObject(buf.state_mux, INFINITE);
	state = buf.state;
	ReleaseMutex(buf.state_mux);

	return state;
}

/// Sets the state of the input
void input_setstate(char state)
{
	char oldstate;

	// update state
	WaitForSingleObject(buf.state_mux, INFINITE);
	oldstate = buf.state;
	buf.state = state;
	ReleaseMutex(buf.state_mux);

	if( state == INPUT_READY && oldstate == INPUT_READING )
	{// data has become available
		SuspendThread(buf.worker);
	} else if( state == INPUT_WAITING )
	{// input is waiting for data
		ResumeThread(buf.worker);
	//} else if( state == INPUT_READING )
	//{// worker is reading data
	} else if( state == INPUT_CLOSED )
	{// end the input
		CloseHandle(buf.state_mux);
		TerminateThread(buf.worker, 0);
	}
}

/// Gets the next state of the input
#define input_nextstate() input_getstate()

/// Returns if data is available from asynchronous input.
/// Requests data if none is available.
int input_hasdata(void)
{
	if( input_getstate() == INPUT_READY )
	{// buffer is ready
		if( buf.len > 0 )
			return 1; // data found ;)
		// request data from the worker
		input_setstate(INPUT_WAITING);
	}
	return 0; // no data
}

/// Initialize asynchronous input
int input_init(void)
{
	int err = 0;

	memset(&buf, 0, sizeof(buf));
	buf.state_mux = CreateMutex(NULL, FALSE, NULL);
	if( buf.state_mux == NULL )
	{// failed to create state mutex
		return 1;
	}
	buf.len = 0;
	input_setstate(INPUT_READY);
	WORKER_EXECUTE(getinput, &err);
	if( err )
	{// failed to start worker
		input_setstate(INPUT_CLOSED);
	}

	return err;
}

/// Finalize asynchronous input
int input_final(void)
{
	input_setstate(INPUT_CLOSED);
	return 0;
}

//////////////////////////////
#else
//////////////////////////////
//
// --=== Asynchronous console input ===--
//
// On the other systems a process is used and pipes are used to comunicate.
// The worker process receives status updates through one of the pipes either 
// requesting data or ending the worker.
// The other pipe is used by the worker to send the input data and is checked 
// for data by the main thread in the timer function.
//
// Note: The Worker thread only starts to get input data when further data is 
//    requested. This is a design choise and brings no real advantage or 
//    disadvantage I can think of.
//

/// Returns the state of the input
#define input_getstate() buf.state

/// Sets the state of the input
void input_setstate(char state)
{
	if( state == INPUT_READY && input_getstate() == INPUT_READING )
	{// send data from the worker to the main process
		write(buf.data_pipe[PIPE_WRITE], &buf.len, sizeof(buf.len));		
		write(buf.data_pipe[PIPE_WRITE], &buf.arr, buf.len);
	} else if( state == INPUT_WAITING ){
		if( buf.close_unused_flag == 0 )
		{// close unused pipe sides in the main process
			close(buf.data_pipe[PIPE_WRITE]);
			close(buf.state_pipe[PIPE_READ]);
			buf.close_unused_flag = 1;
		}
		// send the next state
		write(buf.state_pipe[PIPE_WRITE], &state, sizeof(state));
	} else if( state == INPUT_READING ){
		if( buf.close_unused_flag == 0 )
		{// close unused pipe sides in the worker process
			close(buf.data_pipe[PIPE_READ]);
			close(buf.state_pipe[PIPE_WRITE]);
			buf.close_unused_flag = 1;
		}
	} else if( state == INPUT_CLOSED )
	{// send next state to the worker and close the pipes
		write(buf.state_pipe[PIPE_WRITE], &state, sizeof(state));
		close(buf.data_pipe[PIPE_WRITE]);
		close(buf.data_pipe[PIPE_READ]);
		close(buf.state_pipe[PIPE_WRITE]);
		close(buf.state_pipe[PIPE_READ]);
	}
	buf.state = state;
}

/// Waits for the next state of the input
char input_nextstate()
{
	char state = INPUT_CLOSED;
	int bytes = 0;

	while( bytes == 0 )
		bytes = read(buf.state_pipe[PIPE_READ], &state, sizeof(state));
	if( bytes == -1 )
	{// error, terminate worker
		input_setstate(INPUT_CLOSED);
	}
	return state;
}

/// Returns if data is available from asynchronous input.
/// If data is available, it's put in the local buffer.
int input_hasdata()
{
	struct pollfd fds;
	int hasData;

	if( input_getstate() == INPUT_READY )
	{// start getting data
		input_setstate(INPUT_WAITING);
		return 0;
	}
	// check if data is available
	fds.fd = buf.data_pipe[PIPE_READ];
	fds.events = POLLRDNORM;
	hasData = ( poll(&fds,1,0) > 0 );
	if( hasData )
	{// read the data from the pipe
		read(buf.data_pipe[PIPE_READ], &buf.len, sizeof(buf.len));
		read(buf.data_pipe[PIPE_READ], buf.arr, buf.len);
		input_setstate(INPUT_READY);
	}

	return hasData;
}

/// Initialize asynchronous input
int input_init(void)
{
	int err = 0;

	memset(&buf, 0, sizeof(buf));
	if( pipe(buf.data_pipe) )
	{// error creating data pipe
		return 1;
	}
	if( pipe(buf.state_pipe) )
	{// error creating state pipe
		close(buf.data_pipe[PIPE_READ]);
		close(buf.data_pipe[PIPE_WRITE]);
		return 1;
	}
	buf.len = 0;
	input_setstate(INPUT_READY);
	WORKER_EXECUTE(getinput, &err);
	if( err ){
		//printf("input_init failed to start worker (%d)\n", err);
		input_setstate(INPUT_CLOSED);
	}

	return err;
}

/// Finalize asynchronous input
int input_final(void)
{
	close(buf.data_pipe[PIPE_READ]);
	close(buf.data_pipe[PIPE_WRITE]);
	close(buf.state_pipe[PIPE_READ]);
	close(buf.state_pipe[PIPE_WRITE]);
	return 0;
}

//////////////////////////////
#endif
//////////////////////////////



/// Returns the input data array
#define input_getdata() buf.arr

/// Returns the input data length
#define input_getlen() buf.len

/// Clear the input data
#define input_clear() ( buf.len = 0 )

/// Worker thread/process that gets input
WORKER_FUNC_START(getinput)
	while( input_nextstate() != INPUT_CLOSED )
	{// get input
		input_setstate(INPUT_READING);
		buf.arr[0] = '\0';
		fgets(buf.arr, INPUT_BUFSIZE, stdin);
		buf.len = strlen(buf.arr);
		input_setstate(INPUT_READY);
	}
WORKER_FUNC_END(getinput)





//////// Plugin console functions //////////





/// Timer function that checks if there's assynchronous input data and feeds parse_console()
/// The input reads one line at a time and line terminators are removed.
int console_getinputtimer(int tid, unsigned int tick, int id, intptr data)
{
	char* cmd;
	size_t len;

	if( input_hasdata() ){

		// get data (removes line terminators)
		cmd = input_getdata();
		len = input_getlen();
		while( len > 0 && (cmd[len-1] == '\r' || cmd[len-1] == '\n') )
			cmd[--len] = '\0';

		// parse data
		parse_console(cmd);
		input_clear();
	}

	return 0;
}

/// Start the console
void console_start(void)
{
	if( input_init() ){
		return;
	}
	//##TODO add a 'startupcmd' config options
	//parse_console("help");
	add_timer_func_list(console_getinputtimer,"console_getinputtimer");
	tid = add_timer_interval(gettick(),console_getinputtimer,0,0,250);//##TODO add a 'timerperiod' config option
}

void console_autostart(void)
{//##TODO add an 'autostart' config option
	console_start();
}

/// Stop the console
void console_stop(void)
{
	if( tid != -1 ){
		delete_timer(tid, console_getinputtimer);
		input_final();
	}
	return;
}

/// Test the console for compatibility
int console_test(void)
{// always compatible at the moment, maybe test if standard input is available?
	return 1;
}


/// Initialize the console
void console_init(void)
{
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
}

/// Finalize the console
void console_final(void)
{
}
