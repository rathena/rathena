// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////
// EAthena login-server remote administration tool
///////////////////////////////////////////////////////////////////////////

#include "../common/cbasetypes.h"
#include "../common/core.h"
#include "../common/strlib.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/version.h"
#include "../common/mmo.h"
#include "../common/md5calc.h"
#include "../common/showmsg.h"
#include "ladmin.h"

#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h> // str*
#include <stdarg.h> // valist


//-------------------------------INSTRUCTIONS------------------------------
// Set the variables below:
//   IP of the login server.
//   Port where the login-server listens incoming packets.
//   Password of administration (same of config_athena.conf).
// IMPORTANT:
//   Be sure that you authorize remote administration in login-server
//   (see login_athena.conf, 'admin_state' parameter)
//-------------------------------------------------------------------------
char loginserverip[16] = "127.0.0.1";        // IP of login-server
int loginserverport = 6900;                  // Port of login-server
char loginserveradminpassword[24] = "admin"; // Administration password
int passenc = 0;                             // Encoding type of the password
char ladmin_log_filename[1024] = "log/ladmin.log";
char date_format[32] = "%Y-%m-%d %H:%M:%S";

//-------------------------------------------------------------------------
//  LIST of COMMANDs that you can type at the prompt:
//    To use these commands you can only type only the first letters.
//    You must type a minimum of letters (you can not type 'a',
//      because ladmin doesn't know if it's for 'aide' or for 'add')
//    <Example> q <= quit, li <= list, pass <= passwd, etc.
//
//  Note: every time you must give a account_name, you can use "" or '' (spaces can be included)
//
//  aide/help/?
//    Display the description of the commands
//  aide/help/? [command]
//    Display the description of the specified command
//
//  add <account_name> <sex> <password>
//    Create an account with the default email (a@a.com).
//    Concerning the sex, only the first letter is used (F or M).
//    The e-mail is set to a@a.com (default e-mail). It's like to have no e-mail.
//    When the password is omitted, the input is done without displaying of the pressed keys.
//    <example> add testname Male testpass
//
//  ban/banish yyyy/mm/dd hh:mm:ss <account name>
//    Changes the final date of a banishment of an account.
//    Like banset, but <account name> is at end.
//
//  banadd <account_name> <modifier>
//    Adds or substracts time from the final date of a banishment of an account.
//    Modifier is done as follows:
//      Adjustment value (-1, 1, +1, etc...)
//      Modified element:
//        a or y: year
//        m:  month
//        j or d: day
//        h:  hour
//        mn: minute
//        s:  second
//    <example> banadd testname +1m-2mn1s-6y
//              this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
//  NOTE: If you modify the final date of a non-banished account,
//        you fix the final date to (actual time +- adjustments)
//
//  banset <account_name> yyyy/mm/dd [hh:mm:ss]
//    Changes the final date of a banishment of an account.
//    Default time [hh:mm:ss]: 23:59:59.
//  banset <account_name> 0
//    Set a non-banished account (0 = unbanished).
//
//  block <account name>
//    Set state 5 (You have been blocked by the GM Team) to an account.
//    Like state <account name> 5.
//
//  check <account_name> <password>
//    Check the validity of a password for an account
//    NOTE: Server will never send back a password.
//          It's the only method you have to know if a password is correct.
//          The other method is to have a ('physical') access to the accounts file.
//
//  create <account_name> <sex> <email> <password>
//    Like the 'add' command, but with e-mail moreover.
//    <example> create testname Male my@mail.com testpass
//
//  del <account name>
//    Remove an account.
//    This order requires confirmation. After confirmation, the account is deleted.
//
//  email <account_name> <email>
//    Modify the e-mail of an account.
//
//  getcount
//    Give the number of players online on all char-servers.
//
//  gm <account_name> [GM_level]
//    Modify the GM level of an account.
//    Default value remove GM level (GM level = 0).
//    <example> gm testname 80
//
//  id <account name>
//    Give the id of an account.
//
//  info <account_id>
//    Display complete information of an account.
//
//  kami <message>
//    Sends a broadcast message on all map-server (in yellow).
//  kamib <message>
//    Sends a broadcast message on all map-server (in blue).
//
//  list/ls [start_id [end_id]]
//    Display a list of accounts.
//    'start_id', 'end_id': indicate end and start identifiers.
//    Research by name is not possible with this command.
//    <example> list 10 9999999
//
//  listBan/lsBan [start_id [end_id]]
//    Like list/ls, but only for accounts with state or banished
//
//  listGM/lsGM [start_id [end_id]]
//    Like list/ls, but only for GM accounts
//
//  listOK/lsOK [start_id [end_id]]
//    Like list/ls, but only for accounts without state and not banished
//
//  memo <account_name> <memo>
//    Modify the memo of an account.
//    'memo': it can have until 253 characters (with spaces or not).
//
//  name <account_id>
//    Give the name of an account.
//
//  passwd <account_name> <new_password>
//    Change the password of an account.
//    When new password is omitted, the input is done without displaying of the pressed keys.
//
//  quit/end/exit
//    End of the program of administration
//
//  reloadGM
//    Reload GM configuration file
//
//  search <expression>
//    Seek accounts.
//    Displays the accounts whose names correspond.
//  search -r/-e/--expr/--regex <expression>
//    Seek accounts by regular expression.
//    Displays the accounts whose names correspond.
//
//  sex <account_name> <sex>
//    Modify the sex of an account.
//    <example> sex testname Male
//
//  state <account_name> <new_state> <error_message_#7>
//    Change the state of an account.
//    'new_state': state is the state of the packet 0x006a + 1. The possibilities are:
//                 0 = Account ok            6 = Your Game's EXE file is not the latest version
//                 1 = Unregistered ID       7 = You are Prohibited to log in until %s
//                 2 = Incorrect Password    8 = Server is jammed due to over populated
//                 3 = This ID is expired    9 = No MSG
//                 4 = Rejected from Server  100 = This ID has been totally erased
//                 5 = You have been blocked by the GM Team
//                 all other values are 'No MSG', then use state 9 please.
//    'error_message_#7': message of the code error 6 = Your are Prohibited to log in until %s (packet 0x006a)
//
//  timeadd <account_name> <modifier>
//    Adds or substracts time from the validity limit of an account.
//    Modifier is done as follows:
//      Adjustment value (-1, 1, +1, etc...)
//      Modified element:
//        a or y: year
//        m:  month
//        j or d: day
//        h:  hour
//        mn: minute
//        s:  second
//    <example> timeadd testname +1m-2mn1s-6y
//              this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
//  NOTE: You can not modify a unlimited validity limit.
//        If you want modify it, you want probably create a limited validity limit.
//        So, at first, you must set the validity limit to a date/time.
//
//  timeset <account_name> yyyy/mm/dd [hh:mm:ss]
//    Changes the validity limit of an account.
//    Default time [hh:mm:ss]: 23:59:59.
//  timeset <account_name> 0
//    Gives an unlimited validity limit (0 = unlimited).
//
//  unban/unbanish <account name>
//    Unban an account.
//    Like banset <account name> 0.
//
//  unblock <account name>
//    Set state 0 (Account ok) to an account.
//    Like state <account name> 0.
//
//  version
//    Display the version of the login-server.
//
//  who <account name>
//    Displays complete information of an account.
//
//-------------------------------------------------------------------------

int login_fd;
int login_ip;
int bytes_to_read = 0; // flag to know if we waiting bytes from login-server
char command[1024];
char parameters[1024];
int list_first, list_last, list_type, list_count; // parameter to display a list of accounts
int already_exit_function = 0; // sometimes, the exit function is called twice... so, don't log twice the message

//------------------------------
// Writing function of logs file
//------------------------------
int ladmin_log(char *fmt, ...)
{
	FILE *logfp;
	va_list ap;
	time_t t;
	char tmpstr[2048];

	va_start(ap, fmt);

	logfp = fopen(ladmin_log_filename, "a");
	if (logfp) {
		if (fmt[0] == '\0') // jump a line if no message
			fprintf(logfp, "\n");
		else {
			t = time(NULL);
			strftime(tmpstr, 24, date_format, localtime(&t));
			sprintf(tmpstr + strlen(tmpstr), ": %s", fmt);
			vfprintf(logfp, tmpstr, ap);
		}
		fclose(logfp);
	}

	va_end(ap);
	return 0;
}

//---------------------------------------------
// Function to return ordonal text of a number.
//---------------------------------------------
char* makeordinal(int number)
{
	if ((number % 10) < 4 && (number % 10) != 0 && (number < 10 || number > 20)) {
		if ((number % 10) == 1)
			return "st";
		else if ((number % 10) == 2)
			return "nd";
		else
			return "rd";
	} else {
		return "th";
	}
	return "";
}

//-----------------------------------------------------------------------------------------
// Function to test of the validity of an account name (return 0 if incorrect, and 1 if ok)
//-----------------------------------------------------------------------------------------
int verify_accountname(char* account_name)
{
	int i;

	for(i = 0; account_name[i]; i++) {
		if (account_name[i] < 32) {
			ShowMessage("Illegal character found in the account name (%d%s character).\n", i+1, makeordinal(i+1));
			ladmin_log("Illegal character found in the account name (%d%s character).\n", i+1, makeordinal(i+1));
			return 0;
		}
	}

	if (strlen(account_name) < 4) {
		ShowMessage("Account name is too short. Please input an account name of 4-23 bytes.\n");
		ladmin_log("Account name is too short. Please input an account name of 4-23 bytes.\n");
		return 0;
	}

	if (strlen(account_name) > 23) {
		ShowMessage("Account name is too long. Please input an account name of 4-23 bytes.\n");
		ladmin_log("Account name is too long. Please input an account name of 4-23 bytes.\n");
		return 0;
	}

	return 1;
}

//----------------------------------
// Sub-function: Input of a password
//----------------------------------
int typepasswd(char * password)
{
	char password1[1023], password2[1023];
	int letter;
	int i;

	ladmin_log("No password was given. Request to obtain a password.\n");

	memset(password1, '\0', sizeof(password1));
	memset(password2, '\0', sizeof(password2));

	ShowMessage("\033[1;36m Type the password > \033[0;32;42m");

	i = 0;
	while ((letter = getchar()) != '\n')
		password1[i++] = letter;

	ShowMessage("\033[0m\033[1;36m Verify the password > \033[0;32;42m");

	i = 0;
	while ((letter = getchar()) != '\n')
		password2[i++] = letter;

	ShowMessage("\033[0m");
	fflush(stdout);
	fflush(stdin);

	if (strcmp(password1, password2) != 0) {
		ShowMessage("Password verification failed. Please input same password.\n");
		ladmin_log("Password verification failed. Please input same password.\n");
		ladmin_log("  First password: %s, second password: %s.\n", password1, password2);
		return 0;
	}
	ladmin_log("Typed password: %s.\n", password1);
	strcpy(password, password1);
	return 1;
}

//------------------------------------------------------------------------------------
// Sub-function: Test of the validity of password (return 0 if incorrect, and 1 if ok)
//------------------------------------------------------------------------------------
int verify_password(char * password)
{
	int i;

	for(i = 0; password[i]; i++) {
		if (password[i] < 32) {
			ShowMessage("Illegal character found in the password (%d%s character).\n", i+1, makeordinal(i+1));
			ladmin_log("Illegal character found in the password (%d%s character).\n", i+1, makeordinal(i+1));
			return 0;
		}
	}

	if (strlen(password) < 4) {
		ShowMessage("Account name is too short. Please input an account name of 4-23 bytes.\n");
		ladmin_log("Account name is too short. Please input an account name of 4-23 bytes.\n");
		return 0;
	}

	if (strlen(password) > 23) {
		ShowMessage("Password is too long. Please input a password of 4-23 bytes.\n");
		ladmin_log("Password is too long. Please input a password of 4-23 bytes.\n");
		return 0;
	}

	return 1;
}

//------------------------------------------------------------------
// Sub-function: Check the name of a command (return complete name)
//-----------------------------------------------------------------
int check_command(char * command)
{
// help
	if (strncmp(command, "help", 1) == 0 && strncmp(command, "help", strlen(command)) == 0)
		strcpy(command, "help");
// general commands
	else if (strncmp(command, "add", 2) == 0 && strncmp(command, "add", strlen(command)) == 0) // not 1 letter command: 'aide' or 'add'?
		strcpy(command, "add");
	else if ((strncmp(command, "ban", 3) == 0 && strncmp(command, "ban", strlen(command)) == 0) ||
	         (strncmp(command, "banish", 4) == 0 && strncmp(command, "banish", strlen(command)) == 0))
		strcpy(command, "ban");
	else if ((strncmp(command, "banadd", 4) == 0 && strncmp(command, "banadd", strlen(command)) == 0) || // not 1 letter command: 'ba' or 'bs'? 'banadd' or 'banset' ?
	         strcmp(command,   "ba") == 0)
		strcpy(command, "banadd");
	else if ((strncmp(command, "banset", 4) == 0 && strncmp(command, "banset", strlen(command)) == 0) || // not 1 letter command: 'ba' or 'bs'? 'banadd' or 'banset' ?
	         strcmp(command,   "bs") == 0)
		strcpy(command, "banset");
	else if (strncmp(command, "block", 2) == 0 && strncmp(command, "block", strlen(command)) == 0)
		strcpy(command, "block");
	else if (strncmp(command, "check", 2) == 0 && strncmp(command, "check", strlen(command)) == 0) // not 1 letter command: 'check' or 'create'?
		strcpy(command, "check");
	else if (strncmp(command, "create", 2) == 0 && strncmp(command, "create", strlen(command)) == 0) // not 1 letter command: 'check' or 'create'?
		strcpy(command, "create");
	else if (strncmp(command, "delete", 1) == 0 && strncmp(command, "delete", strlen(command)) == 0)
		strcpy(command, "delete");
	else if ((strncmp(command, "email", 2) == 0 && strncmp(command, "email", strlen(command)) == 0) || // not 1 letter command: 'email', 'end' or 'exit'?
	         (strncmp(command, "e-mail", 2) == 0 && strncmp(command, "e-mail", strlen(command)) == 0))
		strcpy(command, "email");
	else if (strncmp(command, "getcount", 2) == 0 && strncmp(command, "getcount", strlen(command)) == 0) // not 1 letter command: 'getcount' or 'gm'?
		strcpy(command, "getcount");
//	else if (strncmp(command, "gm", 2) == 0 && strncmp(command, "gm", strlen(command)) == 0) // not 1 letter command: 'getcount' or 'gm'?
//		strcpy(command, "gm");
//	else if (strncmp(command, "id", 2) == 0 && strncmp(command, "id", strlen(command)) == 0) // not 1 letter command: 'id' or 'info'?
//		strcpy(command, "id");
	else if (strncmp(command, "info", 2) == 0 && strncmp(command, "info", strlen(command)) == 0) // not 1 letter command: 'id' or 'info'?
		strcpy(command, "info");
//	else if (strncmp(command, "kami", 4) == 0 && strncmp(command, "kami", strlen(command)) == 0) // only all letters command: 'kami' or 'kamib'?
//		strcpy(command, "kami");
//	else if (strncmp(command, "kamib", 5) == 0 && strncmp(command, "kamib", strlen(command)) == 0) // only all letters command: 'kami' or 'kamib'?
//		strcpy(command, "kamib");
	else if ((strncmp(command, "list", 2) == 0 && strncmp(command, "list", strlen(command)) == 0) || // 'list' is default list command // not 1 letter command: 'list'?
	         strcmp(command,   "ls") == 0)
		strcpy(command, "list");
	else if ((strncmp(command, "listban", 5) == 0 && strncmp(command, "listban", strlen(command)) == 0) ||
	         (strncmp(command, "lsban", 3) == 0 && strncmp(command, "lsban", strlen(command)) == 0) ||
	         strcmp(command, "lb") == 0)
		strcpy(command, "listban");
	else if ((strncmp(command, "listgm", 5) == 0 && strncmp(command, "listgm", strlen(command)) == 0) ||
	         (strncmp(command, "lsgm", 3) == 0 && strncmp(command, "lsgm", strlen(command)) == 0) ||
	         strcmp(command,  "lg") == 0)
		strcpy(command, "listgm");
	else if ((strncmp(command, "listok", 5) == 0 && strncmp(command, "listok", strlen(command)) == 0) ||
	         (strncmp(command, "lsok", 3) == 0 && strncmp(command, "lsok", strlen(command)) == 0) ||
	         strcmp(command, "lo") == 0)
		strcpy(command, "listok");
	else if (strncmp(command, "memo", 1) == 0 && strncmp(command, "memo", strlen(command)) == 0)
		strcpy(command, "memo");
	else if (strncmp(command, "name", 1) == 0 && strncmp(command, "name", strlen(command)) == 0)
		strcpy(command, "name");
	else if ((strncmp(command, "password", 1) == 0 && strncmp(command, "password", strlen(command)) == 0) ||
	         strcmp(command, "passwd") == 0)
		strcpy(command, "password");
	else if (strncmp(command, "reloadgm", 1) == 0 && strncmp(command, "reloadgm", strlen(command)) == 0)
		strcpy(command, "reloadgm");
	else if (strncmp(command, "search", 3) == 0 && strncmp(command, "search", strlen(command)) == 0) // not 1 letter command: 'search', 'state' or 'sex'?
		strcpy(command, "search"); // not 2 letters command: 'search' or 'sex'?
//	else if (strncmp(command, "sex", 3) == 0 && strncmp(command, "sex", strlen(command)) == 0) // not 1 letter command: 'search', 'state' or 'sex'?
//		strcpy(command, "sex"); // not 2 letters command: 'search' or 'sex'?
	else if (strncmp(command, "state", 2) == 0 && strncmp(command, "state", strlen(command)) == 0) // not 1 letter command: 'search', 'state' or 'sex'?
		strcpy(command, "state");
	else if ((strncmp(command, "timeadd", 5) == 0 && strncmp(command, "timeadd", strlen(command)) == 0) || // not 1 letter command: 'ta' or 'ts'? 'timeadd' or 'timeset'?
	         strcmp(command,  "ta") == 0)
		strcpy(command, "timeadd");
	else if ((strncmp(command, "timeset", 5) == 0 && strncmp(command, "timeset", strlen(command)) == 0) || // not 1 letter command: 'ta' or 'ts'? 'timeadd' or 'timeset'?
	         strcmp(command,  "ts") == 0)
		strcpy(command, "timeset");
	else if ((strncmp(command, "unban", 5) == 0 && strncmp(command, "unban", strlen(command)) == 0) ||
	         (strncmp(command, "unbanish", 4) == 0 && strncmp(command, "unbanish", strlen(command)) == 0))
		strcpy(command, "unban");
	else if (strncmp(command, "unblock", 4) == 0 && strncmp(command, "unblock", strlen(command)) == 0)
		strcpy(command, "unblock");
	else if (strncmp(command, "version", 1) == 0 && strncmp(command, "version", strlen(command)) == 0)
		strcpy(command, "version");
	else if (strncmp(command, "who", 1) == 0 && strncmp(command, "who", strlen(command)) == 0)
		strcpy(command, "who");
// quit
	else if (strncmp(command, "quit", 1) == 0 && strncmp(command, "quit", strlen(command)) == 0)
		strcpy(command, "quit");
	else if (strncmp(command, "exit", 2) == 0 && strncmp(command, "exit", strlen(command)) == 0) // not 1 letter command: 'email', 'end' or 'exit'?
		strcpy(command, "exit");
	else if (strncmp(command, "end", 2) == 0 && strncmp(command, "end", strlen(command)) == 0) // not 1 letter command: 'email', 'end' or 'exit'?
		strcpy(command, "end");

	return 0;
}

//-----------------------------------------
// Sub-function: Display commands of ladmin
//-----------------------------------------
void display_help(char* param)
{
	char command[1023];
	int i;

	memset(command, '\0', sizeof(command));

	if (sscanf(param, "%s ", command) < 1 || strlen(command) == 0)
		strcpy(command, ""); // any value that is not a command

	if (command[0] == '?')
		strcpy(command, "help");

	// lowercase for command
	for (i = 0; command[i]; i++)
		command[i] = TOLOWER(command[i]);

	// Analyse of the command
	check_command(command); // give complete name to the command

	ladmin_log("Displaying of the commands or a command.\n");

	if (strcmp(command, "help") == 0 ) {
		ShowMessage("aide/help/?\n");
		ShowMessage("  Display the description of the commands\n");
		ShowMessage("aide/help/? [command]\n");
		ShowMessage("  Display the description of the specified command\n");
// general commands
	} else if (strcmp(command, "add") == 0) {
		ShowMessage("add <account_name> <sex> <password>\n");
		ShowMessage("  Create an account with the default email (a@a.com).\n");
		ShowMessage("  Concerning the sex, only the first letter is used (F or M).\n");
		ShowMessage("  The e-mail is set to a@a.com (default e-mail). It's like to have no e-mail.\n");
		ShowMessage("  When the password is omitted,\n");
		ShowMessage("  the input is done without displaying of the pressed keys.\n");
		ShowMessage("  <example> add testname Male testpass\n");
	} else if (strcmp(command, "ban") == 0) {
		ShowMessage("ban/banish yyyy/mm/dd hh:mm:ss <account name>\n");
		ShowMessage("  Changes the final date of a banishment of an account.\n");
		ShowMessage("  Like banset, but <account name> is at end.\n");
	} else if (strcmp(command, "banadd") == 0) {
		ShowMessage("banadd <account_name> <modifier>\n");
		ShowMessage("  Adds or substracts time from the final date of a banishment of an account.\n");
		ShowMessage("  Modifier is done as follows:\n");
		ShowMessage("    Adjustment value (-1, 1, +1, etc...)\n");
		ShowMessage("    Modified element:\n");
		ShowMessage("      a or y: year\n");
		ShowMessage("      m:  month\n");
		ShowMessage("      j or d: day\n");
		ShowMessage("      h:  hour\n");
		ShowMessage("      mn: minute\n");
		ShowMessage("      s:  second\n");
		ShowMessage("  <example> banadd testname +1m-2mn1s-6y\n");
		ShowMessage("            this example adds 1 month and 1 second, and substracts 2 minutes\n");
		ShowMessage("            and 6 years at the same time.\n");
		ShowMessage("NOTE: If you modify the final date of a non-banished account,\n");
		ShowMessage("      you fix the final date to (actual time +- adjustments)\n");
	} else if (strcmp(command, "banset") == 0) {
		ShowMessage("banset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
		ShowMessage("  Changes the final date of a banishment of an account.\n");
		ShowMessage("  Default time [hh:mm:ss]: 23:59:59.\n");
		ShowMessage("banset <account_name> 0\n");
		ShowMessage("  Set a non-banished account (0 = unbanished).\n");
	} else if (strcmp(command, "block") == 0) {
		ShowMessage("block <account name>\n");
		ShowMessage("  Set state 5 (You have been blocked by the GM Team) to an account.\n");
		ShowMessage("  This command works like state <account_name> 5.\n");
	} else if (strcmp(command, "check") == 0) {
		ShowMessage("check <account_name> <password>\n");
		ShowMessage("  Check the validity of a password for an account.\n");
		ShowMessage("  NOTE: Server will never send back a password.\n");
		ShowMessage("        It's the only method you have to know if a password is correct.\n");
		ShowMessage("        The other method is to have a ('physical') access to the accounts file.\n");
	} else if (strcmp(command, "create") == 0) {
		ShowMessage("create <account_name> <sex> <email> <password>\n");
		ShowMessage("  Like the 'add' command, but with e-mail moreover.\n");
		ShowMessage("  <example> create testname Male my@mail.com testpass\n");
	} else if (strcmp(command, "delete") == 0) {
		ShowMessage("del <account name>\n");
		ShowMessage("  Remove an account.\n");
		ShowMessage("  This order requires confirmation. After confirmation, the account is deleted.\n");
	} else if (strcmp(command, "email") == 0) {
		ShowMessage("email <account_name> <email>\n");
		ShowMessage("  Modify the e-mail of an account.\n");
	} else if (strcmp(command, "getcount") == 0) {
		ShowMessage("getcount\n");
		ShowMessage("  Give the number of players online on all char-servers.\n");
	} else if (strcmp(command, "gm") == 0) {
		ShowMessage("gm <account_name> [GM_level]\n");
		ShowMessage("  Modify the GM level of an account.\n");
		ShowMessage("  Default value remove GM level (GM level = 0).\n");
		ShowMessage("  <example> gm testname 80\n");
	} else if (strcmp(command, "id") == 0) {
		ShowMessage("id <account name>\n");
		ShowMessage("  Give the id of an account.\n");
	} else if (strcmp(command, "info") == 0) {
		ShowMessage("info <account_id>\n");
		ShowMessage("  Display complete information of an account.\n");
	} else if (strcmp(command, "kami") == 0) {
		ShowMessage("kami <message>\n");
		ShowMessage("  Sends a broadcast message on all map-server (in yellow).\n");
	} else if (strcmp(command, "kamib") == 0) {
		ShowMessage("kamib <message>\n");
		ShowMessage("  Sends a broadcast message on all map-server (in blue).\n");
	} else if (strcmp(command, "list") == 0) {
		ShowMessage("list/ls [start_id [end_id]]\n");
		ShowMessage("  Display a list of accounts.\n");
		ShowMessage("  'start_id', 'end_id': indicate end and start identifiers.\n");
		ShowMessage("  Research by name is not possible with this command.\n");
		ShowMessage("  <example> list 10 9999999\n");
	} else if (strcmp(command, "listban") == 0) {
		ShowMessage("listBan/lsBan [start_id [end_id]]\n");
		ShowMessage("  Like list/ls, but only for accounts with state or banished.\n");
	} else if (strcmp(command, "listgm") == 0) {
		ShowMessage("listGM/lsGM [start_id [end_id]]\n");
		ShowMessage("  Like list/ls, but only for GM accounts.\n");
	} else if (strcmp(command, "listok") == 0) {
		ShowMessage("listOK/lsOK [start_id [end_id]]\n");
		ShowMessage("  Like list/ls, but only for accounts without state and not banished.\n");
	} else if (strcmp(command, "memo") == 0) {
		ShowMessage("memo <account_name> <memo>\n");
		ShowMessage("  Modify the memo of an account.\n");
		ShowMessage("  'memo': it can have until 253 characters (with spaces or not).\n");
	} else if (strcmp(command, "name") == 0) {
		ShowMessage("name <account_id>\n");
		ShowMessage("  Give the name of an account.\n");
	} else if (strcmp(command, "password") == 0) {
		ShowMessage("passwd <account_name> <new_password>\n");
		ShowMessage("  Change the password of an account.\n");
		ShowMessage("  When new password is omitted,\n");
		ShowMessage("  the input is done without displaying of the pressed keys.\n");
	} else if (strcmp(command, "reloadgm") == 0) {
		ShowMessage("reloadGM\n");
		ShowMessage("  Reload GM configuration file\n");
	} else if (strcmp(command, "search") == 0) {
		ShowMessage("search <expression>\n");
		ShowMessage("  Seek accounts.\n");
		ShowMessage("  Displays the accounts whose names correspond.\n");
//			ShowMessage("search -r/-e/--expr/--regex <expression>\n");
//			ShowMessage("  Seek accounts by regular expression.\n");
//			ShowMessage("  Displays the accounts whose names correspond.\n");
	} else if (strcmp(command, "sex") == 0) {
		ShowMessage("sex <account_name> <sex>\n");
		ShowMessage("  Modify the sex of an account.\n");
		ShowMessage("  <example> sex testname Male\n");
	} else if (strcmp(command, "state") == 0) {
		ShowMessage("state <account_name> <new_state> <error_message_#7>\n");
		ShowMessage("  Change the state of an account.\n");
		ShowMessage("  'new_state': state is the state of the packet 0x006a + 1.\n");
		ShowMessage("               The possibilities are:\n");
		ShowMessage("               0 = Account ok\n");
		ShowMessage("               1 = Unregistered ID\n");
		ShowMessage("               2 = Incorrect Password\n");
		ShowMessage("               3 = This ID is expired\n");
		ShowMessage("               4 = Rejected from Server\n");
		ShowMessage("               5 = You have been blocked by the GM Team\n");
		ShowMessage("               6 = Your Game's EXE file is not the latest version\n");
		ShowMessage("               7 = You are Prohibited to log in until...\n");
		ShowMessage("               8 = Server is jammed due to over populated\n");
		ShowMessage("               9 = No MSG\n");
		ShowMessage("               100 = This ID has been totally erased\n");
		ShowMessage("               all other values are 'No MSG', then use state 9 please.\n");
		ShowMessage("  'error_message_#7': message of the code error 6\n");
		ShowMessage("                      = Your are Prohibited to log in until... (packet 0x006a)\n");
	} else if (strcmp(command, "timeadd") == 0) {
		ShowMessage("timeadd <account_name> <modifier>\n");
		ShowMessage("  Adds or substracts time from the validity limit of an account.\n");
		ShowMessage("  Modifier is done as follows:\n");
		ShowMessage("    Adjustment value (-1, 1, +1, etc...)\n");
		ShowMessage("    Modified element:\n");
		ShowMessage("      a or y: year\n");
		ShowMessage("      m:  month\n");
		ShowMessage("      j or d: day\n");
		ShowMessage("      h:  hour\n");
		ShowMessage("      mn: minute\n");
		ShowMessage("      s:  second\n");
		ShowMessage("  <example> timeadd testname +1m-2mn1s-6y\n");
		ShowMessage("            this example adds 1 month and 1 second, and substracts 2 minutes\n");
		ShowMessage("            and 6 years at the same time.\n");
		ShowMessage("NOTE: You can not modify a unlimited validity limit.\n");
		ShowMessage("      If you want modify it, you want probably create a limited validity limit.\n");
		ShowMessage("      So, at first, you must set the validity limit to a date/time.\n");
	} else if (strcmp(command, "timeadd") == 0) {
		ShowMessage("timeset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
		ShowMessage("  Changes the validity limit of an account.\n");
		ShowMessage("  Default time [hh:mm:ss]: 23:59:59.\n");
		ShowMessage("timeset <account_name> 0\n");
		ShowMessage("  Gives an unlimited validity limit (0 = unlimited).\n");
	} else if (strcmp(command, "unban") == 0) {
		ShowMessage("unban/unbanish <account name>\n");
		ShowMessage("  Remove the banishment of an account.\n");
		ShowMessage("  This command works like banset <account_name> 0.\n");
	} else if (strcmp(command, "unblock") == 0) {
		ShowMessage("unblock <account name>\n");
		ShowMessage("  Set state 0 (Account ok) to an account.\n");
		ShowMessage("  This command works like state <account_name> 0.\n");
	} else if (strcmp(command, "version") == 0) {
		ShowMessage("version\n");
		ShowMessage("  Display the version of the login-server.\n");
	} else if (strcmp(command, "who") == 0) {
		ShowMessage("who <account name>\n");
		ShowMessage("  Displays complete information of an account.\n");
// quit
	} else if (strcmp(command, "quit") == 0 ||
	           strcmp(command, "exit") == 0 ||
	           strcmp(command, "end") == 0) {
		ShowMessage("quit/end/exit\n");
		ShowMessage("  End of the program of administration.\n");
// unknown command
	} else {
		if (strlen(command) > 0)
			ShowMessage("Unknown command [%s] for help. Displaying of all commands.\n", command);
		ShowMessage(" aide/help/?                          -- Display this help\n");
		ShowMessage(" aide/help/? [command]                -- Display the help of the command\n");
		ShowMessage(" add <account_name> <sex> <password>  -- Create an account with default email\n");
		ShowMessage(" ban/banish yyyy/mm/dd hh:mm:ss <account name> -- Change final date of a ban\n");
		ShowMessage(" banadd/ba <account_name> <modifier>  -- Add or substract time from the final\n");
		ShowMessage("   example: ba apple +1m-2mn1s-2y        date of a banishment of an account\n");
		ShowMessage(" banset/bs <account_name> yyyy/mm/dd [hh:mm:ss] -- Change final date of a ban\n");
		ShowMessage(" banset/bs <account_name> 0           -- Un-banish an account\n");
		ShowMessage(" block <account name>     -- Set state 5 (blocked by the GM Team) to an account\n");
		ShowMessage(" check <account_name> <password>      -- Check the validity of a password\n");
		ShowMessage(" create <account_name> <sex> <email> <passwrd> -- Create an account with email\n");
		ShowMessage(" del <account name>                   -- Remove an account\n");
		ShowMessage(" email <account_name> <email>         -- Modify an email of an account\n");
		ShowMessage(" getcount                             -- Give the number of players online\n");
		ShowMessage(" gm <account_name> [GM_level]         -- Modify the GM level of an account\n");
		ShowMessage(" id <account name>                    -- Give the id of an account\n");
		ShowMessage(" info <account_id>                    -- Display all information of an account\n");
		ShowMessage(" kami <message>                       -- Sends a broadcast message (in yellow)\n");
		ShowMessage(" kamib <message>                      -- Sends a broadcast message (in blue)\n");
		ShowMessage(" list/ls [First_id [Last_id]]         -- Display a list of accounts\n");
		ShowMessage(" listBan/lsBan [First_id [Last_id] ]  -- Display a list of accounts\n");
		ShowMessage("                                         with state or banished\n");
		ShowMessage(" listGM/lsGM [First_id [Last_id]]     -- Display a list of GM accounts\n");
		ShowMessage(" listOK/lsOK [First_id [Last_id] ]    -- Display a list of accounts\n");
		ShowMessage("                                         without state and not banished\n");
		ShowMessage(" memo <account_name> <memo>           -- Modify the memo of an account\n");
		ShowMessage(" name <account_id>                    -- Give the name of an account\n");
		ShowMessage(" passwd <account_name> <new_password> -- Change the password of an account\n");
		ShowMessage(" quit/end/exit                        -- End of the program of administation\n");
		ShowMessage(" reloadGM                             -- Reload GM configuration file\n");
		ShowMessage(" search <expression>                  -- Seek accounts\n");
//			ShowMessage(" search -e/-r/--expr/--regex <expressn> -- Seek accounts by regular-expression\n");
		ShowMessage(" sex <nomcompte> <sexe>               -- Modify the sex of an account\n");
		ShowMessage(" state <account_name> <new_state> <error_message_#7> -- Change the state\n");
		ShowMessage(" timeadd/ta <account_name> <modifier> -- Add or substract time from the\n");
		ShowMessage("   example: ta apple +1m-2mn1s-2y        validity limit of an account\n");
		ShowMessage(" timeset/ts <account_name> yyyy/mm/dd [hh:mm:ss] -- Change the validify limit\n");
		ShowMessage(" timeset/ts <account_name> 0          -- Give a unlimited validity limit\n");
		ShowMessage(" unban/unbanish <account name>        -- Remove the banishment of an account\n");
		ShowMessage(" unblock <account name>               -- Set state 0 (Account ok) to an account\n");
		ShowMessage(" version                              -- Gives the version of the login-server\n");
		ShowMessage(" who <account name>                   -- Display all information of an account\n");
		ShowMessage(" who <account name>                   -- Display all information of an account\n");
		ShowMessage(" Note: To use spaces in an account name, type \"<account name>\" (or ').\n");
	}
}

//-----------------------------
// Sub-function: add an account
//-----------------------------
int addaccount(char* param, int emailflag)
{
	char name[1023], sex[1023], email[1023], password[1023];
//	int i;
	WFIFOHEAD(login_fd,91);

	memset(name, '\0', sizeof(name));
	memset(sex, '\0', sizeof(sex));
	memset(email, '\0', sizeof(email));
	memset(password, '\0', sizeof(password));

	if (emailflag == 0) { // add command
		if (sscanf(param, "\"%[^\"]\" %s %[^\r\n]", name, sex, password) < 2 && // password can be void
		    sscanf(param, "'%[^']' %s %[^\r\n]", name, sex, password) < 2 && // password can be void
		    sscanf(param, "%s %s %[^\r\n]", name, sex, password) < 2) { // password can be void
			ShowMessage("Please input an account name, a sex and a password.\n");
			ShowMessage("<example> add testname Male testpass\n");
			ladmin_log("Incomplete parameters to create an account ('add' command).\n");
			return 136;
		}
		strcpy(email, "a@a.com"); // default email
	} else { // 1: create command
		if (sscanf(param, "\"%[^\"]\" %s %s %[^\r\n]", name, sex, email, password) < 3 && // password can be void
		    sscanf(param, "'%[^']' %s %s %[^\r\n]", name, sex, email, password) < 3 && // password can be void
		    sscanf(param, "%s %s %s %[^\r\n]", name, sex, email, password) < 3) { // password can be void
			ShowMessage("Please input an account name, a sex and a password.\n");
			ShowMessage("<example> create testname Male my@mail.com testpass\n");
			ladmin_log("Incomplete parameters to create an account ('create' command).\n");
			return 136;
		}
	}
	if (verify_accountname(name) == 0) {
		return 102;
	}

	sex[0] = TOUPPER(sex[0]);
	if (strchr("MF", sex[0]) == NULL) {
		ShowMessage("Illegal gender [%s]. Please input M or F.\n", sex);
		ladmin_log("Illegal gender [%s]. Please input M or F.\n", sex);
		return 103;
	}

	if (strlen(email) < 3) {
		ShowMessage("Email is too short [%s]. Please input a valid e-mail.\n", email);
		ladmin_log("Email is too short [%s]. Please input a valid e-mail.\n", email);
		return 109;
	}
	if (strlen(email) > 39) {
		ShowMessage("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
		ladmin_log("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
		return 109;
	}
	if (e_mail_check(email) == 0) {
		ShowMessage("Invalid email [%s]. Please input a valid e-mail.\n", email);
		ladmin_log("Invalid email [%s]. Please input a valid e-mail.\n", email);
		return 109;
	}

	if (strlen(password) == 0) {
		if (typepasswd(password) == 0)
			return 108;
	}
	if (verify_password(password) == 0)
		return 104;

	ladmin_log("Request to login-server to create an account.\n");

	WFIFOW(login_fd,0) = 0x7930;
	memcpy(WFIFOP(login_fd,2), name, 24);
	memcpy(WFIFOP(login_fd,26), password, 24);
	WFIFOB(login_fd,50) = sex[0];
	memcpy(WFIFOP(login_fd,51), email, 40);
	WFIFOSET(login_fd,91);
	bytes_to_read = 1;

	return 0;
}

//---------------------------------------------------------------------------------
// Sub-function: Add/substract time to the final date of a banishment of an account
//---------------------------------------------------------------------------------
int banaddaccount(char* param)
{
	char name[1023], modif[1023];
	int year, month, day, hour, minute, second;
	char * p_modif;
	int value, i;
	WFIFOHEAD(login_fd,38);

	memset(name, '\0', sizeof(name));
	memset(modif, '\0', sizeof(modif));
	year = month = day = hour = minute = second = 0;

	if (sscanf(param, "\"%[^\"]\" %[^\r\n]", name, modif) < 2 &&
	    sscanf(param, "'%[^']' %[^\r\n]", name, modif) < 2 &&
	    sscanf(param, "%s %[^\r\n]", name, modif) < 2) {
		ShowMessage("Please input an account name and a modifier.\n");
		ShowMessage("  <example>: banadd testname +1m-2mn1s-6y\n");
		ShowMessage("             this example adds 1 month and 1 second, and substracts 2 minutes\n");
		ShowMessage("             and 6 years at the same time.\n");
		ladmin_log("Incomplete parameters to modify the ban date/time of an account ('banadd' command).\n");
		return 136;
	}
	if (verify_accountname(name) == 0) {
		return 102;
	}

	// lowercase for modif
	for (i = 0; modif[i]; i++)
		modif[i] = TOLOWER(modif[i]);
	p_modif = modif;
	while (strlen(p_modif) > 0) {
		value = atoi(p_modif);
		if (value == 0) {
			p_modif++;
		} else {
			if (p_modif[0] == '-' || p_modif[0] == '+')
				p_modif++;
			while (strlen(p_modif) > 0 && p_modif[0] >= '0' && p_modif[0] <= '9') {
				p_modif++;
			}
			if (p_modif[0] == 's') {
				second = value;
				p_modif++;
			} else if (p_modif[0] == 'm' && p_modif[1] == 'n') {
				minute = value;
				p_modif += 2;
			} else if (p_modif[0] == 'h') {
				hour = value;
				p_modif++;
			} else if (p_modif[0] == 'd' || p_modif[0] == 'j') {
				day = value;
				p_modif += 2;
			} else if (p_modif[0] == 'm') {
				month = value;
				p_modif++;
			} else if (p_modif[0] == 'y' || p_modif[0] == 'a') {
				year = value;
				p_modif++;
			} else {
				p_modif++;
			}
		}
	}

	ShowMessage(" year:   %d\n", year);
	ShowMessage(" month:  %d\n", month);
	ShowMessage(" day:    %d\n", day);
	ShowMessage(" hour:   %d\n", hour);
	ShowMessage(" minute: %d\n", minute);
	ShowMessage(" second: %d\n", second);

	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		ShowMessage("Please give an adjustment with this command:\n");
		ShowMessage("  Adjustment value (-1, 1, +1, etc...)\n");
		ShowMessage("  Modified element:\n");
		ShowMessage("    a or y: year\n");
		ShowMessage("    m: month\n");
		ShowMessage("    j or d: day\n");
		ShowMessage("    h: hour\n");
		ShowMessage("    mn: minute\n");
		ShowMessage("    s: second\n");
		ShowMessage("  <example> banadd testname +1m-2mn1s-6y\n");
		ShowMessage("            this example adds 1 month and 1 second, and substracts 2 minutes\n");
		ShowMessage("            and 6 years at the same time.\n");
		ladmin_log("No adjustment isn't an adjustment ('banadd' command).\n");
		return 137;
	}
	if (year > 127 || year < -127) {
		ShowMessage("Please give a correct adjustment for the years (from -127 to 127).\n");
		ladmin_log("Abnormal adjustment for the year ('banadd' command).\n");
		return 137;
	}
	if (month > 255 || month < -255) {
		ShowMessage("Please give a correct adjustment for the months (from -255 to 255).\n");
		ladmin_log("Abnormal adjustment for the month ('banadd' command).\n");
		return 137;
	}
	if (day > 32767 || day < -32767) {
		ShowMessage("Please give a correct adjustment for the days (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the days ('banadd' command).\n");
		return 137;
	}
	if (hour > 32767 || hour < -32767) {
		ShowMessage("Please give a correct adjustment for the hours (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the hours ('banadd' command).\n");
		return 137;
	}
	if (minute > 32767 || minute < -32767) {
		ShowMessage("Please give a correct adjustment for the minutes (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the minutes ('banadd' command).\n");
		return 137;
	}
	if (second > 32767 || second < -32767) {
		ShowMessage("Please give a correct adjustment for the seconds (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the seconds ('banadd' command).\n");
		return 137;
	}

	ladmin_log("Request to login-server to modify a ban date/time.\n");

	WFIFOW(login_fd,0) = 0x794c;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOW(login_fd,26) = (short)year;
	WFIFOW(login_fd,28) = (short)month;
	WFIFOW(login_fd,30) = (short)day;
	WFIFOW(login_fd,32) = (short)hour;
	WFIFOW(login_fd,34) = (short)minute;
	WFIFOW(login_fd,36) = (short)second;
	WFIFOSET(login_fd,38);
	bytes_to_read = 1;

	return 0;
}

//-----------------------------------------------------------------------
// Sub-function of sub-function banaccount, unbanaccount or bansetaccount
// Set the final date of a banishment of an account
//-----------------------------------------------------------------------
int bansetaccountsub(char* name, char* date, char* time)
{
	int year, month, day, hour, minute, second;
	time_t unban_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
	struct tm *tmtime;
	WFIFOHEAD(login_fd,30);

	year = month = day = hour = minute = second = 0;
	unban_time = 0;
	tmtime = localtime(&unban_time); // initialize

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (atoi(date) != 0 &&
	    ((sscanf(date, "%d/%d/%d", &year, &month, &day) < 3 &&
	      sscanf(date, "%d-%d-%d", &year, &month, &day) < 3 &&
	      sscanf(date, "%d.%d.%d", &year, &month, &day) < 3) ||
	     sscanf(time, "%d:%d:%d", &hour, &minute, &second) < 3))
	{
		ShowMessage("Please input a date and a time (format: yyyy/mm/dd hh:mm:ss).\n");
		ShowMessage("You can imput 0 instead of if you use 'banset' command.\n");
		ladmin_log("Invalid format for the date/time ('banset' or 'ban' command).\n");
		return 102;
	}

	if (atoi(date) == 0) {
		unban_time = 0;
	} else {
		if (year < 70) {
			year = year + 100;
		}
		if (year >= 1900) {
			year = year - 1900;
		}
		if (month < 1 || month > 12) {
			ShowMessage("Please give a correct value for the month (from 1 to 12).\n");
			ladmin_log("Invalid month for the date ('banset' or 'ban' command).\n");
			return 102;
		}
		month = month - 1;
		if (day < 1 || day > 31) {
			ShowMessage("Please give a correct value for the day (from 1 to 31).\n");
			ladmin_log("Invalid day for the date ('banset' or 'ban' command).\n");
			return 102;
		}
		if (((month == 3 || month == 5 || month == 8 || month == 10) && day > 30) ||
		    (month == 1 && day > 29)) {
			ShowMessage("Please give a correct value for a day of this month (%d).\n", month);
			ladmin_log("Invalid day for this month ('banset' or 'ban' command).\n");
			return 102;
		}
		if (hour < 0 || hour > 23) {
			ShowMessage("Please give a correct value for the hour (from 0 to 23).\n");
			ladmin_log("Invalid hour for the time ('banset' or 'ban' command).\n");
			return 102;
		}
		if (minute < 0 || minute > 59) {
			ShowMessage("Please give a correct value for the minutes (from 0 to 59).\n");
			ladmin_log("Invalid minute for the time ('banset' or 'ban' command).\n");
			return 102;
		}
		if (second < 0 || second > 59) {
			ShowMessage("Please give a correct value for the seconds (from 0 to 59).\n");
			ladmin_log("Invalid second for the time ('banset' or 'ban' command).\n");
			return 102;
		}
		tmtime->tm_year = year;
		tmtime->tm_mon = month;
		tmtime->tm_mday = day;
		tmtime->tm_hour = hour;
		tmtime->tm_min = minute;
		tmtime->tm_sec = second;
		tmtime->tm_isdst = -1; // -1: no winter/summer time modification
		unban_time = mktime(tmtime);
		if (unban_time == -1) {
			ShowMessage("Invalid date.\n");
			ShowMessage("Please input a date and a time (format: yyyy/mm/dd hh:mm:ss).\n");
			ShowMessage("You can imput 0 instead of if you use 'banset' command.\n");
			ladmin_log("Invalid date. ('banset' or 'ban' command).\n");
			return 102;
		}
	}

	ladmin_log("Request to login-server to set a ban.\n");

	WFIFOW(login_fd,0) = 0x794a;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOL(login_fd,26) = (int)unban_time;
	WFIFOSET(login_fd,30);
	bytes_to_read = 1;

	return 0;
}

//---------------------------------------------------------------------
// Sub-function: Set the final date of a banishment of an account (ban)
//---------------------------------------------------------------------
int banaccount(char* param)
{
	char name[1023], date[1023], time[1023];

	memset(name, '\0', sizeof(name));
	memset(date, '\0', sizeof(date));
	memset(time, '\0', sizeof(time));

	if (sscanf(param, "%s %s \"%[^\"]\"", date, time, name) < 3 &&
	    sscanf(param, "%s %s '%[^']'", date, time, name) < 3 &&
	    sscanf(param, "%s %s %[^\r\n]", date, time, name) < 3) {
		ShowMessage("Please input an account name, a date and a hour.\n");
		ShowMessage("<example>: banset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
		ShowMessage("           banset <account_name> 0   (0 = un-banished)\n");
		ShowMessage("           ban/banish yyyy/mm/dd hh:mm:ss <account name>\n");
		ShowMessage("           unban/unbanish <account name>\n");
		ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
		ladmin_log("Incomplete parameters to set a ban ('banset' or 'ban' command).\n");
		return 136;
	}

	return bansetaccountsub(name, date, time);
}

//------------------------------------------------------------------------
// Sub-function: Set the final date of a banishment of an account (banset)
//------------------------------------------------------------------------
int bansetaccount(char* param)
{
	char name[1023], date[1023], time[1023];

	memset(name, '\0', sizeof(name));
	memset(date, '\0', sizeof(date));
	memset(time, '\0', sizeof(time));

	if (sscanf(param, "\"%[^\"]\" %s %[^\r\n]", name, date, time) < 2 && // if date = 0, time can be void
	    sscanf(param, "'%[^']' %s %[^\r\n]", name, date, time) < 2 && // if date = 0, time can be void
	    sscanf(param, "%s %s %[^\r\n]", name, date, time) < 2) { // if date = 0, time can be void
		ShowMessage("Please input an account name, a date and a hour.\n");
		ShowMessage("<example>: banset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
		ShowMessage("           banset <account_name> 0   (0 = un-banished)\n");
		ShowMessage("           ban/banish yyyy/mm/dd hh:mm:ss <account name>\n");
		ShowMessage("           unban/unbanish <account name>\n");
		ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
		ladmin_log("Incomplete parameters to set a ban ('banset' or 'ban' command).\n");
		return 136;
	}

	if (time[0] == '\0')
		strcpy(time, "23:59:59");

	return bansetaccountsub(name, date, time);
}

//-------------------------------------------------
// Sub-function: unbanishment of an account (unban)
//-------------------------------------------------
int unbanaccount(char* param)
{
	char name[1023];

	memset(name, '\0', sizeof(name));

	if (strlen(param) == 0 ||
	    (sscanf(param, "\"%[^\"]\"", name) < 1 &&
	     sscanf(param, "'%[^']'", name) < 1 &&
	     sscanf(param, "%[^\r\n]", name) < 1) ||
	     strlen(name) == 0)
	{
		ShowMessage("Please input an account name.\n");
		ShowMessage("<example>: banset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
		ShowMessage("           banset <account_name> 0   (0 = un-banished)\n");
		ShowMessage("           ban/banish yyyy/mm/dd hh:mm:ss <account name>\n");
		ShowMessage("           unban/unbanish <account name>\n");
		ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
		ladmin_log("Incomplete parameters to set a ban ('unban' command).\n");
		return 136;
	}

	return bansetaccountsub(name, "0", "");
}

//---------------------------------------------------------
// Sub-function: Asking to check the validity of a password
// (Note: never send back a password with login-server!! security of passwords)
//---------------------------------------------------------
int checkaccount(char* param)
{
	char name[1023], password[1023];
	WFIFOHEAD(login_fd,50);

	memset(name, '\0', sizeof(name));
	memset(password, '\0', sizeof(password));

	if (sscanf(param, "\"%[^\"]\" %[^\r\n]", name, password) < 1 && // password can be void
	    sscanf(param, "'%[^']' %[^\r\n]", name, password) < 1 && // password can be void
	    sscanf(param, "%s %[^\r\n]", name, password) < 1) { // password can be void
		ShowMessage("Please input an account name.\n");
		ShowMessage("<example> check testname password\n");
		ladmin_log("Incomplete parameters to check the password of an account ('check' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (strlen(password) == 0) {
		if (typepasswd(password) == 0)
			return 134;
	}
	if (verify_password(password) == 0)
		return 131;

	ladmin_log("Request to login-server to check a password.\n");

	WFIFOW(login_fd,0) = 0x793a;
	memcpy(WFIFOP(login_fd,2), name, 24);
	memcpy(WFIFOP(login_fd,26), password, 24);
	WFIFOSET(login_fd,50);
	bytes_to_read = 1;

	return 0;
}

//------------------------------------------------
// Sub-function: Asking for deletion of an account
//------------------------------------------------
int delaccount(char* param)
{
	char name[1023];
	char letter;
	char confirm[1023];
	int i;
	WFIFOHEAD(login_fd,26);

	memset(name, '\0', sizeof(name));

	if (strlen(param) == 0 ||
	    (sscanf(param, "\"%[^\"]\"", name) < 1 &&
	     sscanf(param, "'%[^']'", name) < 1 &&
	     sscanf(param, "%[^\r\n]", name) < 1) ||
	     strlen(name) == 0)
	{
		ShowMessage("Please input an account name.\n");
		ShowMessage("<example> del testnametodelete\n");
		ladmin_log("No name given to delete an account ('delete' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	memset(confirm, '\0', sizeof(confirm));
	while (confirm[0] != 'n' && confirm[0] != 'y')
	{
		ShowMessage("\033[1;36m ** Are you really sure to DELETE account [$userid]? (y/n) > \033[0m");
		fflush(stdout);
		memset(confirm, '\0', sizeof(confirm));
		i = 0;
		while ((letter = getchar()) != '\n')
			confirm[i++] = letter;
	}

	if (confirm[0] == 'n') {
		ShowMessage("Deletion canceled.\n");
		ladmin_log("Deletion canceled by user ('delete' command).\n");
		return 121;
	}

	ladmin_log("Request to login-server to delete an acount.\n");

	WFIFOW(login_fd,0) = 0x7932;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOSET(login_fd,26);
	bytes_to_read = 1;

	return 0;
}

//----------------------------------------------------------
// Sub-function: Asking to modification of an account e-mail
//----------------------------------------------------------
int changeemail(char* param)
{
	char name[1023], email[1023];
	WFIFOHEAD(login_fd,66);

	memset(name, '\0', sizeof(name));
	memset(email, '\0', sizeof(email));

	if (sscanf(param, "\"%[^\"]\" %[^\r\n]", name, email) < 2 &&
	    sscanf(param, "'%[^']' %[^\r\n]", name, email) < 2 &&
	    sscanf(param, "%s %[^\r\n]", name, email) < 2) {
		ShowMessage("Please input an account name and an email.\n");
		ShowMessage("<example> email testname newemail\n");
		ladmin_log("Incomplete parameters to change the email of an account ('email' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (strlen(email) < 3) {
		ShowMessage("Email is too short [%s]. Please input a valid e-mail.\n", email);
		ladmin_log("Email is too short [%s]. Please input a valid e-mail.\n", email);
		return 109;
	}
	if (strlen(email) > 39) {
		ShowMessage("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
		ladmin_log("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
		return 109;
	}
	if (e_mail_check(email) == 0) {
		ShowMessage("Invalid email [%s]. Please input a valid e-mail.\n", email);
		ladmin_log("Invalid email [%s]. Please input a valid e-mail.\n", email);
		return 109;
	}

	ladmin_log("Request to login-server to change an email.\n");

	WFIFOW(login_fd,0) = 0x7940;
	memcpy(WFIFOP(login_fd,2), name, 24);
	memcpy(WFIFOP(login_fd,26), email, 40);
	WFIFOSET(login_fd,66);
	bytes_to_read = 1;

	return 0;
}

//-----------------------------------------------------
// Sub-function: Asking of the number of online players
//-----------------------------------------------------
int getlogincount(void)
{
	WFIFOHEAD(login_fd,2);
	ladmin_log("Request to login-server to obtain the # of online players.\n");

	WFIFOW(login_fd,0) = 0x7938;
	WFIFOSET(login_fd,2);
	bytes_to_read = 1;

	return 0;
}

//----------------------------------------------------------
// Sub-function: Asking to modify the GM level of an account
//----------------------------------------------------------
int changegmlevel(char* param)
{
	char name[1023];
	int GM_level;
	WFIFOHEAD(login_fd,27);

	memset(name, '\0', sizeof(name));
	GM_level = 0;

	if (sscanf(param, "\"%[^\"]\" %d", name, &GM_level) < 1 &&
	    sscanf(param, "'%[^']' %d", name, &GM_level) < 1 &&
	    sscanf(param, "%s %d", name, &GM_level) < 1) {
		ShowMessage("Please input an account name and a GM level.\n");
		ShowMessage("<example> gm testname 80\n");
		ladmin_log("Incomplete parameters to change the GM level of an account ('gm' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (GM_level < 0 || GM_level > 99) {
		ShowMessage("Illegal GM level [%d]. Please input a value from 0 to 99.\n", GM_level);
		ladmin_log("Illegal GM level [%d]. The value can be from 0 to 99.\n", GM_level);
		return 103;
	}

	ladmin_log("Request to login-server to change a GM level.\n");

	WFIFOW(login_fd,0) = 0x793e;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOB(login_fd,26) = GM_level;
	WFIFOSET(login_fd,27);
	bytes_to_read = 1;

	return 0;
}

//---------------------------------------------
// Sub-function: Asking to obtain an account id
//---------------------------------------------
int idaccount(char* param)
{
	char name[1023];
	WFIFOHEAD(login_fd,26);

	memset(name, '\0', sizeof(name));

	if (strlen(param) == 0 ||
	    (sscanf(param, "\"%[^\"]\"", name) < 1 &&
	     sscanf(param, "'%[^']'", name) < 1 &&
	     sscanf(param, "%[^\r\n]", name) < 1) ||
	     strlen(name) == 0)
	{
		ShowMessage("Please input an account name.\n");
		ShowMessage("<example> id testname\n");
		ladmin_log("No name given to search an account id ('id' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	ladmin_log("Request to login-server to know an account id.\n");

	WFIFOW(login_fd,0) = 0x7944;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOSET(login_fd,26);
	bytes_to_read = 1;

	return 0;
}

//----------------------------------------------------------------------------
// Sub-function: Asking to displaying information about an account (by its id)
//----------------------------------------------------------------------------
int infoaccount(int account_id)
{
	WFIFOHEAD(login_fd,6);
	if (account_id < 0) {
		ShowMessage("Please input a positive value for the id.\n");
		ladmin_log("Negative value was given to found the account.\n");
		return 136;
	}

	ladmin_log("Request to login-server to obtain information about an account (by its id).\n");

	WFIFOW(login_fd,0) = 0x7954;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
	bytes_to_read = 1;

	return 0;
}

//---------------------------------------
// Sub-function: Send a broadcast message
//---------------------------------------
int sendbroadcast(short type, char* message)
{
	int len = strlen(message);
	WFIFOHEAD(login_fd,9+len);
	if (len == 0) {
		ShowMessage("Please input a message.\n");
		if (type == 0) {
			ShowMessage("<example> kami a message\n");
		} else {
			ShowMessage("<example> kamib a message\n");
		}
		ladmin_log("The message is void ('kami(b)' command).\n");
		return 136;
	}
	len++; //+'\0'
	WFIFOW(login_fd,0) = 0x794e;
	WFIFOW(login_fd,2) = type;
	WFIFOL(login_fd,4) = len;
	memcpy(WFIFOP(login_fd,8), message, len);
	WFIFOSET(login_fd,8+len);
	bytes_to_read = 1;

	return 0;
}

//--------------------------------------------------------
// Sub-function: Asking to Displaying of the accounts list
//--------------------------------------------------------
int listaccount(char* param, int type)
{
//int list_first, list_last, list_type; // parameter to display a list of accounts
	int i;
	WFIFOHEAD(login_fd,10);

	list_type = type;

	// set default values
	list_first = 0;
	list_last = 0;

	if (list_type == 1) { // if listgm
		// get all accounts = use default
	} else if (list_type == 2) { // if search
		for (i = 0; param[i]; i++)
			param[i] = TOLOWER(param[i]);
		// get all accounts = use default
	} else if (list_type == 3) { // if listban
		// get all accounts = use default
	} else if (list_type == 4) { // if listok
		// get all accounts = use default
	} else { // if list (list_type == 0)
		switch(sscanf(param, "%d %d", &list_first, &list_last)) {
		case 0:
			// get all accounts = use default
			break;
		case 1:
			list_last = 0;
			// use tests of the following value
		default:
			if (list_first < 0)
				list_first = 0;
			if (list_last < list_first || list_last < 0)
				list_last = 0;
			break;
		}
	}

	ladmin_log("Request to login-server to obtain the list of accounts from %d to %d.\n", list_first, list_last);

	WFIFOW(login_fd,0) = 0x7920;
	WFIFOL(login_fd,2) = list_first;
	WFIFOL(login_fd,6) = list_last;
	WFIFOSET(login_fd,10);
	bytes_to_read = 1;

	//          0123456789 01 01234567890123456789012301234 012345 0123456789012345678901234567
	ShowMessage("account_id GM user_name               sex    count state\n");
	ShowMessage("-------------------------------------------------------------------------------\n");
	list_count = 0;

	return 0;
}

//--------------------------------------------
// Sub-function: Asking to modify a memo field
//--------------------------------------------
int changememo(char* param)
{
	char name[1023], memo[1023];
	WFIFOHEAD(login_fd,28+255);

	memset(name, '\0', sizeof(name));
	memset(memo, '\0', sizeof(memo));

	if (sscanf(param, "\"%[^\"]\" %[^\r\n]", name, memo) < 1 && // memo can be void
	    sscanf(param, "'%[^']' %[^\r\n]", name, memo) < 1 && // memo can be void
	    sscanf(param, "%s %[^\r\n]", name, memo) < 1) { // memo can be void
		ShowMessage("Please input an account name and a memo.\n");
		ShowMessage("<example> memo testname new memo\n");
		ladmin_log("Incomplete parameters to change the memo of an account ('email' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (strlen(memo) > 254) {
		ShowMessage("Memo is too long (%lu characters).\n", (unsigned long)strlen(memo));
		ShowMessage("Please input a memo of 254 bytes at the maximum.\n");
		ladmin_log("Email is too long (%d characters). Please input a memo of 254 bytes at the maximum.\n", strlen(memo));
		return 102;
	}

	ladmin_log("Request to login-server to change a memo.\n");

	WFIFOW(login_fd,0) = 0x7942;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOW(login_fd,26) = strlen(memo);
	if (strlen(memo) > 0)
		memcpy(WFIFOP(login_fd,28), memo, strlen(memo));
	WFIFOSET(login_fd,28+strlen(memo));
	bytes_to_read = 1;

	return 0;
}

//-----------------------------------------------
// Sub-function: Asking to obtain an account name
//-----------------------------------------------
int nameaccount(int id)
{
	WFIFOHEAD(login_fd,6);
	if (id < 0) {
		ShowMessage("Please input a positive value for the id.\n");
		ladmin_log("Negativ id given to search an account name ('name' command).\n");
		return 136;
	}

	ladmin_log("Request to login-server to know an account name.\n");

	WFIFOW(login_fd,0) = 0x7946;
	WFIFOL(login_fd,2) = id;
	WFIFOSET(login_fd,6);
	bytes_to_read = 1;

	return 0;
}

//------------------------------------------
// Sub-function: Asking to modify a password
// (Note: never send back a password with login-server!! security of passwords)
//------------------------------------------
int changepasswd(char* param)
{
	char name[1023], password[1023];
	WFIFOHEAD(login_fd,50);

	memset(name, '\0', sizeof(name));
	memset(password, '\0', sizeof(password));

	if (sscanf(param, "\"%[^\"]\" %[^\r\n]", name, password) < 1 &&
	    sscanf(param, "'%[^']' %[^\r\n]", name, password) < 1 &&
	    sscanf(param, "%s %[^\r\n]", name, password) < 1) {
		ShowMessage("Please input an account name.\n");
		ShowMessage("<example> passwd testname newpassword\n");
		ladmin_log("Incomplete parameters to change the password of an account ('password' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (strlen(password) == 0) {
		if (typepasswd(password) == 0)
			return 134;
	}
	if (verify_password(password) == 0)
		return 131;

	ladmin_log("Request to login-server to change a password.\n");

	WFIFOW(login_fd,0) = 0x7934;
	memcpy(WFIFOP(login_fd,2), name, 24);
	memcpy(WFIFOP(login_fd,26), password, 24);
	WFIFOSET(login_fd,50);
	bytes_to_read = 1;

	return 0;
}

//----------------------------------------------------------------------
// Sub-function: Request to login-server to reload GM configuration file
// this function have no answer
//----------------------------------------------------------------------
int reloadGM(void)
{
	WFIFOHEAD(login_fd,2);
	WFIFOW(login_fd,0) = 0x7955;
	WFIFOSET(login_fd,2);
	bytes_to_read = 0;

	ladmin_log("Request to reload the GM configuration file sended.\n");
	ShowMessage("Request to reload the GM configuration file sended.\n");
	ShowMessage("Check the actual GM accounts (after reloading):\n");
	listaccount(parameters, 1); // 1: to list only GM

	return 180;
}

//-----------------------------------------------------
// Sub-function: Asking to modify the sex of an account
//-----------------------------------------------------
int changesex(char* param)
{
	char name[1023], sex[1023];
	WFIFOHEAD(login_fd,27);

	memset(name, '\0', sizeof(name));
	memset(sex, '\0', sizeof(sex));

	if (sscanf(param, "\"%[^\"]\" %[^\r\n]", name, sex) < 2 &&
	    sscanf(param, "'%[^']' %[^\r\n]", name, sex) < 2 &&
	    sscanf(param, "%s %[^\r\n]", name, sex) < 2) {
		ShowMessage("Please input an account name and a sex.\n");
		ShowMessage("<example> sex testname Male\n");
		ladmin_log("Incomplete parameters to change the sex of an account ('sex' command).\n");
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	sex[0] = TOUPPER(sex[0]);
	if (strchr("MF", sex[0]) == NULL) {
		ShowMessage("Illegal gender [%s]. Please input M or F.\n", sex);
		ladmin_log("Illegal gender [%s]. Please input M or F.\n", sex);
		return 103;
	}

	ladmin_log("Request to login-server to change a sex.\n");

	WFIFOW(login_fd,0) = 0x793c;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOB(login_fd,26) = sex[0];
	WFIFOSET(login_fd,27);
	bytes_to_read = 1;

	return 0;
}

//-------------------------------------------------------------------------
// Sub-function of sub-function changestate, blockaccount or unblockaccount
// Asking to modify the state of an account
//-------------------------------------------------------------------------
int changestatesub(char* name, int state, char* error_message7)
{
	char error_message[1023]; // need to use, because we can modify error_message7
	WFIFOHEAD(login_fd,50);

	memset(error_message, '\0', sizeof(error_message));
	strncpy(error_message, error_message7, sizeof(error_message)-1);

	if ((state < 0 || state > 9) && state != 100) { // Valid values: 0: ok, or value of the 0x006a packet + 1
		ShowMessage("Please input one of these states:\n");
		ShowMessage("  0 = Account ok            6 = Your Game's EXE file is not the latest version\n");
		ShowMessage("  1 = Unregistered ID       7 = You are Prohibited to log in until + message\n");
		ShowMessage("  2 = Incorrect Password    8 = Server is jammed due to over populated\n");
		ShowMessage("  3 = This ID is expired    9 = No MSG\n");
		ShowMessage("  4 = Rejected from Server  100 = This ID has been totally erased\n");
		ShowMessage("  5 = You have been blocked by the GM Team\n");
		ShowMessage("<examples> state testname 5\n");
		ShowMessage("           state testname 7 end of your ban\n");
		ShowMessage("           block <account name>\n");
		ShowMessage("           unblock <account name>\n");
		ladmin_log("Invalid value for the state of an account ('state', 'block' or 'unblock' command).\n");
		return 151;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (state != 7) {
		strcpy(error_message, "-");
	} else {
		if (strlen(error_message) < 1) {
			ShowMessage("Error message is too short. Please input a message of 1-19 bytes.\n");
			ladmin_log("Error message is too short. Please input a message of 1-19 bytes.\n");
			return 102;
		}
		if (strlen(error_message) > 19) {
			ShowMessage("Error message is too long. Please input a message of 1-19 bytes.\n");
			ladmin_log("Error message is too long. Please input a message of 1-19 bytes.\n");
			return 102;
		}
	}

	ladmin_log("Request to login-server to change a state.\n");

	WFIFOW(login_fd,0) = 0x7936;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOL(login_fd,26) = state;
	memcpy(WFIFOP(login_fd,30), error_message, 20);
	WFIFOSET(login_fd,50);
	bytes_to_read = 1;

	return 0;
}

//-------------------------------------------------------
// Sub-function: Asking to modify the state of an account
//-------------------------------------------------------
int changestate(char* param)
{
	char name[1023], error_message[1023];
	int state;

	memset(name, '\0', sizeof(name));
	memset(error_message, '\0', sizeof(error_message));

	if (sscanf(param, "\"%[^\"]\" %d %[^\r\n]", name, &state, error_message) < 2 &&
	    sscanf(param, "'%[^']' %d %[^\r\n]", name, &state, error_message) < 2 &&
	    sscanf(param, "%s %d %[^\r\n]", name, &state, error_message) < 2) {
		ShowMessage("Please input an account name and a state.\n");
		ShowMessage("<examples> state testname 5\n");
		ShowMessage("           state testname 7 end of your ban\n");
		ShowMessage("           block <account name>\n");
		ShowMessage("           unblock <account name>\n");
		ladmin_log("Incomplete parameters to change the state of an account ('state' command).\n");
		return 136;
	}

	return changestatesub(name, state, error_message);
}

//-------------------------------------------
// Sub-function: Asking to unblock an account
//-------------------------------------------
int unblockaccount(char* param)
{
	char name[1023];

	memset(name, '\0', sizeof(name));

	if (strlen(param) == 0 ||
	    (sscanf(param, "\"%[^\"]\"", name) < 1 &&
	     sscanf(param, "'%[^']'", name) < 1 &&
	     sscanf(param, "%[^\r\n]", name) < 1) ||
	     strlen(name) == 0)
	{
		ShowMessage("Please input an account name.\n");
		ShowMessage("<examples> state testname 5\n");
		ShowMessage("           state testname 7 end of your ban\n");
		ShowMessage("           block <account name>\n");
		ShowMessage("           unblock <account name>\n");
		ladmin_log("Incomplete parameters to change the state of an account ('unblock' command).\n");
		return 136;
	}

	return changestatesub(name, 0, "-"); // state 0, no error message
}

//-------------------------------------------
// Sub-function: Asking to unblock an account
//-------------------------------------------
int blockaccount(char* param)
{
	char name[1023];

	memset(name, '\0', sizeof(name));

	if (strlen(param) == 0 ||
	    (sscanf(param, "\"%[^\"]\"", name) < 1 &&
	     sscanf(param, "'%[^']'", name) < 1 &&
	     sscanf(param, "%[^\r\n]", name) < 1) ||
	     strlen(name) == 0)
	{
		ShowMessage("Please input an account name.\n");
		ShowMessage("<examples> state testname 5\n");
		ShowMessage("           state testname 7 end of your ban\n");
		ShowMessage("           block <account name>\n");
		ShowMessage("           unblock <account name>\n");
		ladmin_log("Incomplete parameters to change the state of an account ('block' command).\n");
		return 136;
	}

	return changestatesub(name, 5, "-"); // state 5, no error message
}

//---------------------------------------------------------------------
// Sub-function: Add/substract time to the validity limit of an account
//---------------------------------------------------------------------
int timeaddaccount(char* param)
{
	char name[1023], modif[1023];
	int year, month, day, hour, minute, second;
	char * p_modif;
	int value, i;
	WFIFOHEAD(login_fd,38);

	memset(name, '\0', sizeof(name));
	memset(modif, '\0', sizeof(modif));
	year = month = day = hour = minute = second = 0;

	if (sscanf(param, "\"%[^\"]\" %[^\r\n]", name, modif) < 2 &&
	    sscanf(param, "'%[^']' %[^\r\n]", name, modif) < 2 &&
	    sscanf(param, "%s %[^\r\n]", name, modif) < 2)
	{
		ShowMessage("Please input an account name and a modifier.\n");
		ShowMessage("  <example>: timeadd testname +1m-2mn1s-6y\n");
		ShowMessage("             this example adds 1 month and 1 second, and substracts 2 minutes\n");
		ShowMessage("             and 6 years at the same time.\n");
		ladmin_log("Incomplete parameters to modify a limit time ('timeadd' command).\n");
		return 136;
	}
	if (verify_accountname(name) == 0) {
		return 102;
	}

	// lowercase for modif
	for (i = 0; modif[i]; i++)
		modif[i] = TOLOWER(modif[i]);
	p_modif = modif;
	while (strlen(p_modif) > 0) {
		value = atoi(p_modif);
		if (value == 0) {
			p_modif++;
		} else {
			if (p_modif[0] == '-' || p_modif[0] == '+')
				p_modif++;
			while (strlen(p_modif) > 0 && p_modif[0] >= '0' && p_modif[0] <= '9') {
				p_modif++;
			}
			if (p_modif[0] == 's') {
				second = value;
				p_modif++;
			} else if (p_modif[0] == 'm' && p_modif[1] == 'n') {
				minute = value;
				p_modif += 2;
			} else if (p_modif[0] == 'h') {
				hour = value;
				p_modif++;
			} else if (p_modif[0] == 'd' || p_modif[0] == 'j') {
				day = value;
				p_modif += 2;
			} else if (p_modif[0] == 'm') {
				month = value;
				p_modif++;
			} else if (p_modif[0] == 'y' || p_modif[0] == 'a') {
				year = value;
				p_modif++;
			} else {
				p_modif++;
			}
		}
	}

	ShowMessage(" year:   %d\n", year);
	ShowMessage(" month:  %d\n", month);
	ShowMessage(" day:    %d\n", day);
	ShowMessage(" hour:   %d\n", hour);
	ShowMessage(" minute: %d\n", minute);
	ShowMessage(" second: %d\n", second);

	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		ShowMessage("Please give an adjustment with this command:\n");
		ShowMessage("  Adjustment value (-1, 1, +1, etc...)\n");
		ShowMessage("  Modified element:\n");
		ShowMessage("    a or y: year\n");
		ShowMessage("    m:      month\n");
		ShowMessage("    j or d: day\n");
		ShowMessage("    h:      hour\n");
		ShowMessage("    mn:     minute\n");
		ShowMessage("    s:      second\n");
		ShowMessage("  <example> timeadd testname +1m-2mn1s-6y\n");
		ShowMessage("            this example adds 1 month and 1 second, and substracts 2 minutes\n");
		ShowMessage("            and 6 years at the same time.\n");
		ladmin_log("No adjustment isn't an adjustment ('timeadd' command).\n");
		return 137;
	}
	if (year > 127 || year < -127) {
		ShowMessage("Please give a correct adjustment for the years (from -127 to 127).\n");
		ladmin_log("Abnormal adjustment for the year ('timeadd' command).\n");
		return 137;
	}
	if (month > 255 || month < -255) {
		ShowMessage("Please give a correct adjustment for the months (from -255 to 255).\n");
		ladmin_log("Abnormal adjustment for the month ('timeadd' command).\n");
		return 137;
	}
	if (day > 32767 || day < -32767) {
		ShowMessage("Please give a correct adjustment for the days (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the days ('timeadd' command).\n");
		return 137;
	}
	if (hour > 32767 || hour < -32767) {
		ShowMessage("Please give a correct adjustment for the hours (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the hours ('timeadd' command).\n");
		return 137;
	}
	if (minute > 32767 || minute < -32767) {
		ShowMessage("Please give a correct adjustment for the minutes (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the minutes ('timeadd' command).\n");
		return 137;
	}
	if (second > 32767 || second < -32767) {
		ShowMessage("Please give a correct adjustment for the seconds (from -32767 to 32767).\n");
		ladmin_log("Abnormal adjustment for the seconds ('timeadd' command).\n");
		return 137;
	}

	ladmin_log("Request to login-server to modify a time limit.\n");

	WFIFOW(login_fd,0) = 0x7950;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOW(login_fd,26) = (short)year;
	WFIFOW(login_fd,28) = (short)month;
	WFIFOW(login_fd,30) = (short)day;
	WFIFOW(login_fd,32) = (short)hour;
	WFIFOW(login_fd,34) = (short)minute;
	WFIFOW(login_fd,36) = (short)second;
	WFIFOSET(login_fd,38);
	bytes_to_read = 1;

	return 0;
}

//-------------------------------------------------
// Sub-function: Set a validity limit of an account
//-------------------------------------------------
int timesetaccount(char* param)
{
	char name[1023], date[1023], time[1023];
	int year, month, day, hour, minute, second;
	time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	struct tm *tmtime;
	WFIFOHEAD(login_fd,30);

	memset(name, '\0', sizeof(name));
	memset(date, '\0', sizeof(date));
	memset(time, '\0', sizeof(time));
	year = month = day = hour = minute = second = 0;
	expiration_time = 0;
	tmtime = localtime(&expiration_time); // initialize

	if (sscanf(param, "\"%[^\"]\" %s %[^\r\n]", name, date, time) < 2 && // if date = 0, time can be void
	    sscanf(param, "'%[^']' %s %[^\r\n]", name, date, time) < 2 && // if date = 0, time can be void
	    sscanf(param, "%s %s %[^\r\n]", name, date, time) < 2) { // if date = 0, time can be void
		ShowMessage("Please input an account name, a date and a hour.\n");
		ShowMessage("<example>: timeset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
		ShowMessage("           timeset <account_name> 0   (0 = unlimited)\n");
		ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
		ladmin_log("Incomplete parameters to set a limit time ('timeset' command).\n");
		return 136;
	}
	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (time[0] == '\0')
		strcpy(time, "23:59:59");

	if (atoi(date) != 0 &&
	    ((sscanf(date, "%d/%d/%d", &year, &month, &day) < 3 &&
	      sscanf(date, "%d-%d-%d", &year, &month, &day) < 3 &&
	      sscanf(date, "%d.%d.%d", &year, &month, &day) < 3 &&
	      sscanf(date, "%d'%d'%d", &year, &month, &day) < 3) ||
	     sscanf(time, "%d:%d:%d", &hour, &minute, &second) < 3))
	{
		ShowMessage("Please input 0 or a date and a time (format: 0 or yyyy/mm/dd hh:mm:ss).\n");
		ladmin_log("Invalid format for the date/time ('timeset' command).\n");
		return 102;
	}

	if (atoi(date) == 0) {
		expiration_time = 0;
	} else {
		if (year < 70) {
			year = year + 100;
		}
		if (year >= 1900) {
			year = year - 1900;
		}
		if (month < 1 || month > 12) {
			ShowMessage("Please give a correct value for the month (from 1 to 12).\n");
			ladmin_log("Invalid month for the date ('timeset' command).\n");
			return 102;
		}
		month = month - 1;
		if (day < 1 || day > 31) {
			ShowMessage("Please give a correct value for the day (from 1 to 31).\n");
			ladmin_log("Invalid day for the date ('timeset' command).\n");
			return 102;
		}
		if (((month == 3 || month == 5 || month == 8 || month == 10) && day > 30) ||
		    (month == 1 && day > 29)) {
			ShowMessage("Please give a correct value for a day of this month (%d).\n", month);
			ladmin_log("Invalid day for this month ('timeset' command).\n");
			return 102;
		}
		if (hour < 0 || hour > 23) {
			ShowMessage("Please give a correct value for the hour (from 0 to 23).\n");
			ladmin_log("Invalid hour for the time ('timeset' command).\n");
			return 102;
		}
		if (minute < 0 || minute > 59) {
			ShowMessage("Please give a correct value for the minutes (from 0 to 59).\n");
			ladmin_log("Invalid minute for the time ('timeset' command).\n");
			return 102;
		}
		if (second < 0 || second > 59) {
			ShowMessage("Please give a correct value for the seconds (from 0 to 59).\n");
			ladmin_log("Invalid second for the time ('timeset' command).\n");
			return 102;
		}
		tmtime->tm_year = year;
		tmtime->tm_mon = month;
		tmtime->tm_mday = day;
		tmtime->tm_hour = hour;
		tmtime->tm_min = minute;
		tmtime->tm_sec = second;
		tmtime->tm_isdst = -1; // -1: no winter/summer time modification
		expiration_time = mktime(tmtime);
		if (expiration_time == -1) {
			ShowMessage("Invalid date.\n");
			ShowMessage("Please add 0 or a date and a time (format: 0 or yyyy/mm/dd hh:mm:ss).\n");
			ladmin_log("Invalid date. ('timeset' command).\n");
			return 102;
		}
	}

	ladmin_log("Request to login-server to set a time limit.\n");

	WFIFOW(login_fd,0) = 0x7948;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOL(login_fd,26) = (int)expiration_time;
	WFIFOSET(login_fd,30);
	bytes_to_read = 1;

	return 0;
}

//------------------------------------------------------------------------------
// Sub-function: Asking to displaying information about an account (by its name)
//------------------------------------------------------------------------------
int whoaccount(char* param)
{
	char name[1023];
	WFIFOHEAD(login_fd,26);

	memset(name, '\0', sizeof(name));

	if (strlen(param) == 0 ||
	    (sscanf(param, "\"%[^\"]\"", name) < 1 &&
	     sscanf(param, "'%[^']'", name) < 1 &&
	     sscanf(param, "%[^\r\n]", name) < 1) ||
	     strlen(name) == 0)
	{
		ShowMessage("Please input an account name.\n");
		ShowMessage("<example> who testname\n");
		ladmin_log("No name was given to found the account.\n");
		return 136;
	}
	if (verify_accountname(name) == 0) {
		return 102;
	}

	ladmin_log("Request to login-server to obtain information about an account (by its name).\n");

	WFIFOW(login_fd,0) = 0x7952;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOSET(login_fd,26);
	bytes_to_read = 1;

	return 0;
}

//--------------------------------------------------------
// Sub-function: Asking of the version of the login-server
//--------------------------------------------------------
int checkloginversion(void)
{
	WFIFOHEAD(login_fd,2);
	ladmin_log("Request to login-server to obtain its version.\n");

	WFIFOW(login_fd,0) = 0x7530;
	WFIFOSET(login_fd,2);
	bytes_to_read = 1;

	return 0;
}

//---------------------------------------------
// Prompt function
// this function wait until user type a command
// and analyse the command.
//---------------------------------------------
int prompt(void)
{
	int i, j;
	char buf[1024];
	char *p;

	// while we don't wait new packets
	while (bytes_to_read == 0) {
		// for help with the console colors look here:
		// http://www.edoceo.com/liberum/?doc=printf-with-color
		// some code explanation (used here):
		// \033[2J : clear screen and go up/left (0, 0 position)
		// \033[K  : clear line from actual position to end of the line
		// \033[0m : reset color parameter
		// \033[1m : use bold for font
		ShowMessage("\n");
		ShowMessage("\033[32mTo list the commands, type 'enter'.\033[0m\n");
		ShowMessage("\033[0;36mLadmin-> \033[0m");
		ShowMessage("\033[1m");
		fflush(stdout);

		// get command and parameter
		memset(buf, '\0', sizeof(buf));
		fflush(stdin);
		fgets(buf, sizeof(buf), stdin);

		ShowMessage("\033[0m");
		fflush(stdout);

		// remove final \n
		if((p = strrchr(buf, '\n')) != NULL)
			p[0] = '\0';
		// remove all control char
		for (i = 0; buf[i]; i++)
			if (buf[i] < 32) {
				// remove cursor control.
				if (buf[i] == 27 && buf[i+1] == '[' &&
				    (buf[i+2] == 'H' || // home position (cursor)
				     buf[i+2] == 'J' || // clear screen
				     buf[i+2] == 'A' || // up 1 line
				     buf[i+2] == 'B' || // down 1 line
				     buf[i+2] == 'C' || // right 1 position
				     buf[i+2] == 'D' || // left 1 position
				     buf[i+2] == 'G')) { // center cursor (windows)
					for (j = i; buf[j]; j++)
						buf[j] = buf[j+3];
				} else if (buf[i] == 27 && buf[i+1] == '[' && buf[i+2] == '2' && buf[i+3] == 'J') { // clear screen
					for (j = i; buf[j]; j++)
						buf[j] = buf[j+4];
				} else if (buf[i] == 27 && buf[i+1] == '[' && buf[i+3] == '~' &&
				           (buf[i+2] == '1' || // home (windows)
				            buf[i+2] == '2' || // insert (windows)
				            buf[i+2] == '3' || // del (windows)
				            buf[i+2] == '4' || // end (windows)
				            buf[i+2] == '5' || // pgup (windows)
				            buf[i+2] == '6')) { // pgdown (windows)
					for (j = i; buf[j]; j++)
						buf[j] = buf[j+4];
				} else {
					// remove other control char.
					for (j = i; buf[j]; j++)
						buf[j] = buf[j+1];
				}
				i--;
			}

		// extract command name and parameters
		memset(command, '\0', sizeof(command));
		memset(parameters, '\0', sizeof(parameters));
		sscanf(buf, "%1023s %[^\n]", command, parameters);
		command[1023] = '\0';
		parameters[1023] = '\0';

		// lowercase for command line
		for (i = 0; command[i]; i++)
			command[i] = TOLOWER(command[i]);

		if (command[0] == '?' || strlen(command) == 0) {
			strcpy(buf, "help");
			strcpy(command, "help");
		}

		// Analyse of the command
		check_command(command); // give complete name to the command

		if (strlen(parameters) == 0) {
			ladmin_log("Command: '%s' (without parameters)\n", command, parameters);
		} else {
			ladmin_log("Command: '%s', parameters: '%s'\n", command, parameters);
		}

		// Analyse of the command
// help
		if (strcmp(command, "help") == 0 ) {
			display_help(parameters);
// general commands
		} else if (strcmp(command, "add") == 0) {
			addaccount(parameters, 0); // 0: no email
		} else if (strcmp(command, "ban") == 0) {
			banaccount(parameters);
		} else if (strcmp(command, "banadd") == 0) {
			banaddaccount(parameters);
		} else if (strcmp(command, "banset") == 0) {
			bansetaccount(parameters);
		} else if (strcmp(command, "block") == 0) {
			blockaccount(parameters);
		} else if (strcmp(command, "check") == 0) {
			checkaccount(parameters);
		} else if (strcmp(command, "create") == 0) {
			addaccount(parameters, 1); // 1: with email
		} else if (strcmp(command, "delete") == 0) {
			delaccount(parameters);
		} else if (strcmp(command, "email") == 0) {
			changeemail(parameters);
		} else if (strcmp(command, "getcount") == 0) {
			getlogincount();
		} else if (strcmp(command, "gm") == 0) {
			changegmlevel(parameters);
		} else if (strcmp(command, "id") == 0) {
			idaccount(parameters);
		} else if (strcmp(command, "info") == 0) {
			infoaccount(atoi(parameters));
		} else if (strcmp(command, "kami") == 0) {
			sendbroadcast(0, parameters); // flag for normal
		} else if (strcmp(command, "kamib") == 0) {
			sendbroadcast(0x10, parameters); // flag for blue
		} else if (strcmp(command, "list") == 0) {
			listaccount(parameters, 0); // 0: to list all
		} else if (strcmp(command, "listban") == 0) {
			listaccount(parameters, 3); // 3: to list only accounts with state or bannished
		} else if (strcmp(command, "listgm") == 0) {
			listaccount(parameters, 1); // 1: to list only GM
		} else if (strcmp(command, "listok") == 0) {
			listaccount(parameters, 4); // 4: to list only accounts without state and not bannished
		} else if (strcmp(command, "memo") == 0) {
			changememo(parameters);
		} else if (strcmp(command, "name") == 0) {
			nameaccount(atoi(parameters));
		} else if (strcmp(command, "password") == 0) {
			changepasswd(parameters);
		} else if (strcmp(command, "reloadgm") == 0) {
			reloadGM();
		} else if (strcmp(command, "search") == 0) { // no regex in C version
			listaccount(parameters, 2); // 2: to list with pattern
		} else if (strcmp(command, "sex") == 0) {
			changesex(parameters);
		} else if (strcmp(command, "state") == 0) {
			changestate(parameters);
		} else if (strcmp(command, "timeadd") == 0) {
			timeaddaccount(parameters);
		} else if (strcmp(command, "timeset") == 0) {
			timesetaccount(parameters);
		} else if (strcmp(command, "unban") == 0) {
			unbanaccount(parameters);
		} else if (strcmp(command, "unblock") == 0) {
			unblockaccount(parameters);
		} else if (strcmp(command, "version") == 0) {
			checkloginversion();
		} else if (strcmp(command, "who") == 0) {
			whoaccount(parameters);
// quit
		} else if (strcmp(command, "quit") == 0 ||
		           strcmp(command, "exit") == 0 ||
		           strcmp(command, "end") == 0) {
			ShowMessage("Bye.\n");
			exit(EXIT_SUCCESS);
// unknown command
		} else {
			ShowMessage("Unknown command [%s].\n", buf);
			ladmin_log("Unknown command [%s].\n", buf);
		}
	}

	return 0;
}

//-------------------------------------------------------------
// Function: Parse receiving informations from the login-server
//-------------------------------------------------------------
int parse_fromlogin(int fd)
{
	struct char_session_data *sd;
	int id;
	RFIFOHEAD(fd);
	if (session[fd]->flag.eof) {
		ShowMessage("Impossible to have a connection with the login-server [%s:%d] !\n", loginserverip, loginserverport);
		ladmin_log("Impossible to have a connection with the login-server [%s:%d] !\n", loginserverip, loginserverport);
		do_close(fd);
		exit(EXIT_FAILURE);
	}

//	ShowMessage("parse_fromlogin : %d %d %d\n", fd, RFIFOREST(fd), RFIFOW(fd,0));
	sd = (struct char_session_data*)session[fd]->session_data;

	while(RFIFOREST(fd) >= 2) {
		switch(RFIFOW(fd,0)) {
		case 0x7919:	// answer of a connection request
			if (RFIFOREST(fd) < 3)
				return 0;
			if (RFIFOB(fd,2) != 0) {
				ShowMessage("Error at login:\n");
				ShowMessage(" - incorrect password,\n");
				ShowMessage(" - administration system not activated, or\n");
				ShowMessage(" - unauthorised IP.\n");
				ladmin_log("Error at login: incorrect password, administration system not activated, or unauthorised IP.\n");
				set_eof(fd);
				//bytes_to_read = 1; // not stop at prompt
				return 0;
			} else {
				ShowMessage("Established connection.\n");
				ladmin_log("Established connection.\n");
				ShowMessage("Reading of the version of the login-server...\n");
				ladmin_log("Reading of the version of the login-server...\n");
				//bytes_to_read = 1; // unchanged
				checkloginversion();
			}
			RFIFOSKIP(fd,3);
			break;

		case 0x01dc:	// answer of a coding key request
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		  {
			char md5str[64] = "", md5bin[32];
			WFIFOHEAD(login_fd, 20);
			if (passenc == 1) {
				strncpy(md5str, (const char*)RFIFOP(fd,4), RFIFOW(fd,2) - 4);
				strcat(md5str, loginserveradminpassword);
			} else if (passenc == 2) {
				strncpy(md5str, loginserveradminpassword, sizeof(loginserveradminpassword));
				strcat(md5str, (const char*)RFIFOP(fd,4));
			}
			MD5_String2binary(md5str, md5bin);
			WFIFOW(login_fd,0) = 0x7918; // Request for administation login (encrypted password)
			WFIFOW(login_fd,2) = passenc; // Encrypted type
			memcpy(WFIFOP(login_fd,4), md5bin, 16);
			WFIFOSET(login_fd,20);
			ShowMessage("Receiving of the MD5 key.\n");
			ladmin_log("Receiving of the MD5 key.\n");
			ShowMessage("Sending of the encrypted password...\n");
			ladmin_log("Sending of the encrypted password...\n");
		  }
			bytes_to_read = 1;
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		case 0x7531:	// Displaying of the version of the login-server
			if (RFIFOREST(fd) < 10)
				return 0;
			ShowMessage("  Login-Server [%s:%d]\n", loginserverip, loginserverport);
			if (((int)RFIFOB(login_fd,5)) == 0) {
				ShowMessage("  eAthena version stable-%d.%d", (int)RFIFOB(login_fd,2), (int)RFIFOB(login_fd,3));
			} else {
				ShowMessage("  eAthena version dev-%d.%d", (int)RFIFOB(login_fd,2), (int)RFIFOB(login_fd,3));
			}
			if (((int)RFIFOB(login_fd,4)) == 0)
				ShowMessage(" revision %d", (int)RFIFOB(login_fd,4));
			if (((int)RFIFOB(login_fd,6)) == 0)
				ShowMessage("%d.\n", RFIFOW(login_fd,8));
			else
				ShowMessage("-mod%d.\n", RFIFOW(login_fd,8));
			bytes_to_read = 0;
			RFIFOSKIP(fd,10);
			break;

		case 0x7921:	// Displaying of the list of accounts
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			if (RFIFOW(fd,2) < 5) {
				ladmin_log("  Receiving of a void accounts list.\n");
				if (list_count == 0)
					ShowMessage("No account found.\n");
				else if (list_count == 1)
					ShowMessage("1 account found.\n");
				else
					ShowMessage("%d accounts found.\n", list_count);
				bytes_to_read = 0;
			} else {
				int i;
				WFIFOHEAD(login_fd,10);
				ladmin_log("  Receiving of a accounts list.\n");
				for(i = 4; i < RFIFOW(fd,2); i += 38) {
					int j;
					char userid[24];
					char lower_userid[24];
					memcpy(userid, RFIFOP(fd,i + 5), sizeof(userid));
					userid[sizeof(userid)-1] = '\0';
					memset(lower_userid, '\0', sizeof(lower_userid));
					for (j = 0; userid[j]; j++)
						lower_userid[j] = TOLOWER(userid[j]);
					list_first = RFIFOL(fd,i) + 1;
					// here are checks...
					if (list_type == 0 ||
					    (list_type == 1 && RFIFOB(fd,i+4) > 0) ||
					    (list_type == 2 && strstr(lower_userid, parameters) != NULL) ||
					    (list_type == 3 && RFIFOL(fd,i+34) != 0) ||
					    (list_type == 4 && RFIFOL(fd,i+34) == 0)) {
						ShowMessage("%10d ", (int)RFIFOL(fd,i));
						if (RFIFOB(fd,i+4) == 0)
							ShowMessage("   ");
						else
							ShowMessage("%2d ", (int)RFIFOB(fd,i+4));
						ShowMessage("%-24s", userid);
						if (RFIFOB(fd,i+29) == 0)
							ShowMessage("%-5s ", "Femal");
						else if (RFIFOB(fd,i+29) == 1)
							ShowMessage("%-5s ", "Male");
						else
							ShowMessage("%-5s ", "Servr");
						ShowMessage("%6d ", (int)RFIFOL(fd,i+30));
						switch(RFIFOL(fd,i+34)) {
						case 0:
							ShowMessage("%-27s\n", "Account OK");
							break;
						case 1:
							ShowMessage("%-27s\n", "Unregistered ID");
							break;
						case 2:
							ShowMessage("%-27s\n", "Incorrect Password");
							break;
						case 3:
							ShowMessage("%-27s\n", "This ID is expired");
							break;
						case 4:
							ShowMessage("%-27s\n", "Rejected from Server");
							break;
						case 5:
							ShowMessage("%-27s\n", "Blocked by the GM Team"); // You have been blocked by the GM Team
							break;
						case 6:
							ShowMessage("%-27s\n", "Your EXE file is too old"); // Your Game's EXE file is not the latest version
							break;
						case 7:
							ShowMessage("%-27s\n", "Banishement or");
							ShowMessage("                                                   Prohibited to login until...\n"); // You are Prohibited to log in until %s
							break;
						case 8:
							ShowMessage("%-27s\n", "Server is over populated");
							break;
						case 9:
							ShowMessage("%-27s\n", "No MSG");
							break;
						default: // 100
							ShowMessage("%-27s\n", "This ID is totally erased"); // This ID has been totally erased
							break;
						}
						list_count++;
					}
				}
				// asking of the following acounts
				ladmin_log("Request to login-server to obtain the list of accounts from %d to %d (complement).\n", list_first, list_last);
				WFIFOW(login_fd,0) = 0x7920;
				WFIFOL(login_fd,2) = list_first;
				WFIFOL(login_fd,6) = list_last;
				WFIFOSET(login_fd,10);
				bytes_to_read = 1;
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		case 0x7931:	// Answer of login-server about an account creation
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id != -1) {
				ShowMessage("Account [%s] creation failed. Return code %d.\n", RFIFOP(fd,6), id);
				ladmin_log("Account [%s] creation failed. Return code %d.\n", RFIFOP(fd,6), id);
			} else {
				ShowMessage("Account [%s] is successfully created.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] is successfully created.\n", RFIFOP(fd,6));
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7933:	// Answer of login-server about an account deletion
			if (RFIFOREST(fd) < 30)
				return 0;
			if (RFIFOL(fd,2) == -1) {
				ShowMessage("Account [%s] deletion failed. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] deletion failed. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("Account [%s][id: %d] is successfully DELETED.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
				ladmin_log("Account [%s][id: %d] is successfully DELETED.\n", RFIFOP(fd,6), RFIFOL(fd,2));
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7935:	// answer of the change of an account password
			if (RFIFOREST(fd) < 30)
				return 0;
			if (RFIFOL(fd,2) == -1) {
				ShowMessage("Account [%s] password changing failed.\n", RFIFOP(fd,6));
				ShowMessage("Account [%s] doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account password changing failed. The compte [%s] doesn't exist.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("Account [%s][id: %d] password successfully changed.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
				ladmin_log("Account [%s][id: %d] password successfully changed.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7937:	// answer of the change of an account state
			if (RFIFOREST(fd) < 34)
				return 0;
			if (RFIFOL(fd,2) == -1) {
				ShowMessage("Account [%s] state changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] state changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				char tmpstr[256];
				sprintf(tmpstr, "Account [%s] state successfully changed in [", RFIFOP(fd,6));
				switch(RFIFOL(fd,30)) {
				case 0:
					strcat(tmpstr, "0: Account OK");
				case 1:
					strcat(tmpstr, "1: Unregistered ID");
					break;
				case 2:
					strcat(tmpstr, "2: Incorrect Password");
					break;
				case 3:
					strcat(tmpstr, "3: This ID is expired");
					break;
				case 4:
					strcat(tmpstr, "4: Rejected from Server");
					break;
				case 5:
					strcat(tmpstr, "5: You have been blocked by the GM Team");
					break;
				case 6:
					strcat(tmpstr, "6: [Your Game's EXE file is not the latest version");
					break;
				case 7:
					strcat(tmpstr, "7: You are Prohibited to log in until...");
					break;
				case 8:
					strcat(tmpstr, "8: Server is jammed due to over populated");
					break;
				case 9:
					strcat(tmpstr, "9: No MSG");
					break;
				default: // 100
					strcat(tmpstr, "100: This ID is totally erased");
					break;
				}
				strcat(tmpstr, "]");
				ShowMessage("%s\n", tmpstr);
				ladmin_log("%s\n", tmpstr);
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,34);
			break;

		case 0x7939:	// answer of the number of online players
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		  {
			// Get length of the received packet
			int i;
			char name[20];
			ladmin_log("  Receiving of the number of online players.\n");
			// Read information of the servers
			if (RFIFOW(fd,2) < 5) {
				ShowMessage("  No server is connected to the login-server.\n");
			} else {
				ShowMessage("  Number of online players (server: number).\n");
				// Displaying of result
				for(i = 4; i < RFIFOW(fd,2); i += 32) {
					memcpy(name, RFIFOP(fd,i+6), sizeof(name));
					name[sizeof(name) - 1] = '\0';
					ShowMessage("    %-20s : %5d\n", name, RFIFOW(fd,i+26));
				}
			}
		  }
			bytes_to_read = 0;
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		case 0x793b:	// answer of the check of a password
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("The account [%s] doesn't exist or the password is incorrect.\n", RFIFOP(fd,6));
				ladmin_log("The account [%s] doesn't exist or the password is incorrect.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("The proposed password is correct for the account [%s][id: %d].\n", RFIFOP(fd,6), id);
				ladmin_log("The proposed password is correct for the account [%s][id: %d].\n", RFIFOP(fd,6), id);
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x793d:	// answer of the change of an account sex
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] sex changing failed.\n", RFIFOP(fd,6));
				ShowMessage("Account [%s] doesn't exist or the sex is already the good sex.\n", RFIFOP(fd,6));
				ladmin_log("Account sex changing failed. The compte [%s] doesn't exist or the sex is already the good sex.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("Account [%s][id: %d] sex successfully changed.\n", RFIFOP(fd,6), id);
				ladmin_log("Account [%s][id: %d] sex successfully changed.\n", RFIFOP(fd,6), id);
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x793f:	// answer of the change of an account GM level
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] GM level changing failed.\n", RFIFOP(fd,6));
				ShowMessage("Account [%s] doesn't exist, the GM level is already the good GM level\n", RFIFOP(fd,6));
				ShowMessage("or it's impossible to modify the GM accounts file.\n");
				ladmin_log("Account GM level changing failed. The compte [%s] doesn't exist, the GM level is already the good sex or it's impossible to modify the GM accounts file.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("Account [%s][id: %d] GM level successfully changed.\n", RFIFOP(fd,6), id);
				ladmin_log("Account [%s][id: %d] GM level successfully changed.\n", RFIFOP(fd,6), id);
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7941:	// answer of the change of an account email
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] e-mail changing failed.\n", RFIFOP(fd,6));
				ShowMessage("Account [%s] doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account e-mail changing failed. The compte [%s] doesn't exist.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("Account [%s][id: %d] e-mail successfully changed.\n", RFIFOP(fd,6), id);
				ladmin_log("Account [%s][id: %d] e-mail successfully changed.\n", RFIFOP(fd,6), id);
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7943:	// answer of the change of an account memo
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] memo changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] memo changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("Account [%s][id: %d] memo successfully changed.\n", RFIFOP(fd,6), id);
				ladmin_log("Account [%s][id: %d] memo successfully changed.\n", RFIFOP(fd,6), id);
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7945:	// answer of an account id search
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Unable to find the account [%s] id. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Unable to find the account [%s] id. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				ShowMessage("The account [%s] have the id: %d.\n", RFIFOP(fd,6), id);
				ladmin_log("The account [%s] have the id: %d.\n", RFIFOP(fd,6), id);
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7947:	// answer of an account name search
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (strcmp((const char*)RFIFOP(fd,6), "") == 0) {
				ShowMessage("Unable to find the account [%d] name. Account doesn't exist.\n", id);
				ladmin_log("Unable to find the account [%d] name. Account doesn't exist.\n", id);
			} else {
				ShowMessage("The account [id: %d] have the name: %s.\n", id, RFIFOP(fd,6));
				ladmin_log("The account [id: %d] have the name: %s.\n", id, RFIFOP(fd,6));
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7949:	// answer of an account validity limit set
			if (RFIFOREST(fd) < 34)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					ShowMessage("Validity Limit of the account [%s][id: %d] successfully changed to [unlimited].\n", RFIFOP(fd,6), id);
					ladmin_log("Validity Limit of the account [%s][id: %d] successfully changed to [unlimited].\n", RFIFOP(fd,6), id);
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					ShowMessage("Validity Limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					ladmin_log("Validity Limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,34);
			break;

		case 0x794b:	// answer of an account ban set
			if (RFIFOREST(fd) < 34)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
					ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,34);
			break;

		case 0x794d:	// answer of an account ban date/time changing
			if (RFIFOREST(fd) < 34)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
					ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,34);
			break;

		case 0x794f:	// answer of a broadcast
			if (RFIFOREST(fd) < 4)
				return 0;
			if (RFIFOW(fd,2) == (unsigned short)-1) {
				ShowMessage("Message sending failed. No online char-server.\n");
				ladmin_log("Message sending failed. No online char-server.\n");
			} else {
				ShowMessage("Message successfully sended to login-server.\n");
				ladmin_log("Message successfully sended to login-server.\n");
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,4);
			break;

		case 0x7951:	// answer of an account validity limit changing
			if (RFIFOREST(fd) < 34)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				ladmin_log("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					ShowMessage("Validity limit of the account [%s][id: %d] unchanged.\n", RFIFOP(fd,6), id);
					ShowMessage("The account have an unlimited validity limit or\n");
					ShowMessage("the changing is impossible with the proposed adjustments.\n");
					ladmin_log("Validity limit of the account [%s][id: %d] unchanged. The account have an unlimited validity limit or the changing is impossible with the proposed adjustments.\n", RFIFOP(fd,6), id);
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					ShowMessage("Validity limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					ladmin_log("Validity limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,34);
			break;

		case 0x7953:	// answer of a request about informations of an account (by account name/id)
			if (RFIFOREST(fd) < 150 || RFIFOREST(fd) < (size_t)(150 + RFIFOW(fd,148)))
				return 0;
		{
			char userid[24], error_message[20], lastlogin[24], last_ip[16], email[40], memo[255];
			time_t unban_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
			time_t expiration_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
			safestrncpy(userid, (char*)RFIFOP(fd,7), sizeof(userid));
			safestrncpy(error_message, (char*)RFIFOP(fd,40), sizeof(error_message));
			safestrncpy(lastlogin, (char*)RFIFOP(fd,60), sizeof(lastlogin));
			safestrncpy(last_ip, (char*)RFIFOP(fd,84), sizeof(last_ip));
			safestrncpy(email, (char*)RFIFOP(fd,100), sizeof(email));
			expiration_time = (time_t)RFIFOL(fd,140);
			unban_time = (time_t)RFIFOL(fd,144);
			memset(memo, '\0', sizeof(memo));
			safestrncpy(memo, (const char*)RFIFOP(fd,150), RFIFOW(fd,148));
			id = RFIFOL(fd,2);
			if (id == -1) {
				ShowMessage("Unabled to find the account [%s]. Account doesn't exist.\n", parameters);
				ladmin_log("Unabled to find the account [%s]. Account doesn't exist.\n", parameters);
			} else if (strlen(userid) == 0) {
				ShowMessage("Unabled to find the account [id: %s]. Account doesn't exist.\n", parameters);
				ladmin_log("Unabled to find the account [id: %s]. Account doesn't exist.\n", parameters);
			} else {
				ladmin_log("Receiving information about an account.\n");
				ShowMessage("The account is set with:\n");
				if (RFIFOB(fd,6) == 0) {
					ShowMessage(" Id:     %d (non-GM)\n", id);
				} else {
					ShowMessage(" Id:     %d (GM level %d)\n", id, (int)RFIFOB(fd,6));
				}
				ShowMessage(" Name:   '%s'\n", userid);
				if (RFIFOB(fd,31) == 0)
					ShowMessage(" Sex:    Female\n");
				else if (RFIFOB(fd,31) == 1)
					ShowMessage(" Sex:    Male\n");
				else
					ShowMessage(" Sex:    Server\n");
				ShowMessage(" E-mail: %s\n", email);
				switch(RFIFOL(fd,36)) {
				case 0: ShowMessage(" State: 0 [Account OK]\n"); break;
				case 1: ShowMessage(" State: 1 [Unregistered ID]\n"); break;
				case 2: ShowMessage(" State: 2 [Incorrect Password]\n"); break;
				case 3: ShowMessage(" State: 3 [This ID is expired]\n"); break;
				case 4: ShowMessage(" State: 4 [Rejected from Server]\n"); break;
				case 5: ShowMessage(" State: 5 [You have been blocked by the GM Team]\n"); break;
				case 6: ShowMessage(" State: 6 [Your Game's EXE file is not the latest version]\n"); break;
				case 7: ShowMessage(" State: 7 [You are Prohibited to log in until %s]\n", error_message); break;
				case 8: ShowMessage(" State: 8 [Server is jammed due to over populated]\n"); break;
				case 9: ShowMessage(" State: 9 [No MSG]\n"); break;
				default: // 100
					ShowMessage(" State: %d [This ID is totally erased]\n", (int)RFIFOL(fd,36));
					break;
				}
				if (unban_time == 0) {
					ShowMessage(" Banishment: not banished.\n");
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&unban_time));
					ShowMessage(" Banishment: until %s.\n", tmpstr);
				}
				if (RFIFOL(fd,32) > 1)
					ShowMessage(" Count:  %d connections.\n", (int)RFIFOL(fd,32));
				else
					ShowMessage(" Count:  %d connection.\n", (int)RFIFOL(fd,32));
				ShowMessage(" Last connection at: %s (ip: %s)\n", lastlogin, last_ip);
				if (expiration_time == 0) {
					ShowMessage(" Validity limit: unlimited.\n");
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&expiration_time));
					ShowMessage(" Validity limit: until %s.\n", tmpstr);
				}
				ShowMessage(" Memo:   '%s'\n", memo);
			}
		  }
			bytes_to_read = 0;
			RFIFOSKIP(fd,150 + RFIFOW(fd,148));
			break;

		default:
			ShowMessage("Remote administration has been disconnected (unknown packet).\n");
			ladmin_log("'End of connection, unknown packet.\n");
			set_eof(fd);
			return 0;
		}
	}

	// if we don't wait new packets, do the prompt
	prompt();

	return 0;
}

//------------------------------------
// Function to connect to login-server
//------------------------------------
int Connect_login_server(void)
{
	ShowMessage("Attempt to connect to login-server...\n");
	ladmin_log("Attempt to connect to login-server...\n");

	login_fd = make_connection(login_ip, loginserverport);
	if (login_fd == -1)
	{	//Might not be the most elegant way to handle this, but I've never used ladmin so I dunno what else you could do. [Skotlex]
		ShowMessage("Error: Failed to connect to Login Server\n");
		exit(EXIT_FAILURE);
	}
	session[login_fd]->flag.server = 1;
	if (passenc == 0) {
		WFIFOHEAD(login_fd,28);
		WFIFOW(login_fd,0) = 0x7918; // Request for administation login
		WFIFOW(login_fd,2) = 0; // no encrypted
		memcpy(WFIFOP(login_fd,4), loginserveradminpassword, 24);
		WFIFOSET(login_fd,28);
		bytes_to_read = 1;
		ShowMessage("Sending of the password...\n");
		ladmin_log("Sending of the password...\n");
	} else {
		WFIFOHEAD(login_fd,2);
		WFIFOW(login_fd,0) = 0x791a; // Sending request about the coding key
		WFIFOSET(login_fd,2);
		bytes_to_read = 1;
		ShowMessage("Request about the MD5 key...\n");
		ladmin_log("Request about the MD5 key...\n");
	}

	return 0;
}

// sends a ping packet to login server (will receive pong 0x2718)
int ping_login_server(int tid, unsigned int tick, int id, intptr data)
{
	if (login_fd > 0 && session[login_fd] != NULL)
	{
		WFIFOHEAD(login_fd,2);
		WFIFOW(login_fd,0) = 0x2719;
		WFIFOSET(login_fd,2);
	}
	return 0;
}

//-----------------------------------
// Reading general configuration file
//-----------------------------------
int ladmin_config_read(const char *cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowMessage("\033[0mConfiguration file (%s) not found.\n", cfgName);
		return 1;
	}

	ShowMessage("\033[0m---Start reading of Ladmin configuration file (%s)\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			remove_control_chars(w1);
			remove_control_chars(w2);

			if(strcmpi(w1,"login_ip")==0) {
				uint32 ipl = host2ip(w2);
				if (ipl != 0) {
					ip2str(ipl, loginserverip);
					ShowMessage("Login server IP address: %s -> %s\n", w2, loginserverip);
				} else
					memcpy(loginserverip, w2, 16);
			} else if (strcmpi(w1, "login_port") == 0) {
				loginserverport = atoi(w2);
			} else if (strcmpi(w1, "admin_pass") == 0) {
				strncpy(loginserveradminpassword, w2, sizeof(loginserveradminpassword));
				loginserveradminpassword[sizeof(loginserveradminpassword)-1] = '\0';
			} else if (strcmpi(w1, "passenc") == 0) {
				passenc = atoi(w2);
				if (passenc < 0 || passenc > 2)
					passenc = 0;
			} else if (strcmpi(w1, "ladmin_log_filename") == 0) {
				strncpy(ladmin_log_filename, w2, sizeof(ladmin_log_filename));
				ladmin_log_filename[sizeof(ladmin_log_filename)-1] = '\0';
			} else if (strcmpi(w1, "date_format") == 0) {
				strncpy(date_format, w2, sizeof(date_format));
			} else if(strcmpi(w1,"stdout_with_ansisequence") == 0) {
				stdout_with_ansisequence = config_switch(w2);
			} else if (strcmpi(w1, "import") == 0) {
				ladmin_config_read(w2);
			}
		}
	}
	fclose(fp);

	login_ip = str2ip(loginserverip);

	ShowMessage("---End reading of Ladmin configuration file.\n");

	return 0;
}

//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
}

//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void)
{
	if (already_exit_function == 0) {

		ShowMessage("\033[0m----End of Ladmin (normal end with closing of all files).\n");
		ladmin_log("----End of Ladmin (normal end with closing of all files).\n");

		already_exit_function = 1;
		do_close(login_fd);
	}
}

//------------------------
// Main function of ladmin
//------------------------
int do_init(int argc, char **argv)
{
	int next;
	socket_init();

	// read ladmin configuration
	ladmin_config_read((argc > 1) ? argv[1] : LADMIN_CONF_NAME);

	ladmin_log("");
	ladmin_log("Configuration file readed.\n");

	srand(time(NULL));

	set_defaultparse(parse_fromlogin);

	ShowMessage("EAthena login-server administration tool.\n");
	ShowMessage("(for eAthena version %d.%d.%d.)\n", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION);

	ladmin_log("Ladmin is ready.\n");
	ShowMessage("Ladmin is \033[1;32mready\033[0m.\n\n");

	Connect_login_server();

	// keep the char-login connection alive
	add_timer_func_list(ping_login_server, "ping_login_server");
	add_timer_interval(gettick() + 1000, ping_login_server, 0, 0, ((int)stall_time-2) * 1000);

	// minimalist core doesn't have sockets parsing,
	// so we have to do this ourselves
	while (runflag) {
		next = do_timer(gettick_nocache());
		do_sockets(next);
	}

	return 0;
}
