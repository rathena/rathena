// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

///////////////////////////////////////////////////////////////////////////
// EAthena login-server remote administration tool
// Ladamin in C by [Yor]
// if you modify this software, modify ladmin in tool too.
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
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
void Gettimeofday(struct timeval *timenow)
{
	time_t t;
	t = clock();
	timenow->tv_usec = t;
	timenow->tv_sec = t / CLK_TCK;
	return;
}
#define gettimeofday(timenow, dummy) Gettimeofday(timenow)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> // gettimeofday
#include <sys/ioctl.h>
#include <unistd.h> // close
#include <arpa/inet.h> // inet_addr
#include <netdb.h> // gethostbyname
#endif
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
//   Displayed language of the sofware (if not correct, english is used).
// IMPORTANT:
//   Be sure that you authorize remote administration in login-server
//   (see login_athena.conf, 'admin_state' parameter)
//-------------------------------------------------------------------------
char loginserverip[16] = "127.0.0.1";        // IP of login-server
int loginserverport = 6900;                  // Port of login-server
char loginserveradminpassword[24] = "admin"; // Administration password
int passenc = 0;                             // Encoding type of the password
char defaultlanguage = 'E';                  // Default language (F: Français/E: English)
                                             // (if it's not 'F', default is English)
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
//  language <language>
//    Change the language of displaying.
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
	struct timeval tv;
	char tmpstr[2048];

	va_start(ap, fmt);

	logfp = fopen(ladmin_log_filename, "a");
	if (logfp) {
		if (fmt[0] == '\0') // jump a line if no message
			fprintf(logfp, "\n");
		else {
			gettimeofday(&tv, NULL);
			strftime(tmpstr, 24, date_format, localtime((const time_t*)&(tv.tv_sec)));
			sprintf(tmpstr + strlen(tmpstr), ".%03d: %s", (int)tv.tv_usec / 1000, fmt);
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
	if (defaultlanguage == 'F') {
		if (number == 0)
			return "";
		else if (number == 1)
			return "er";
		else
			return "ème";
	} else {
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
			if (defaultlanguage == 'F') {
				ShowMessage("Caractère interdit trouvé dans le nom du compte (%d%s caractère).\n", i+1, makeordinal(i+1));
				ladmin_log("Caractère interdit trouvé dans le nom du compte (%d%s caractère).\n", i+1, makeordinal(i+1));
			} else {
				ShowMessage("Illegal character found in the account name (%d%s character).\n", i+1, makeordinal(i+1));
				ladmin_log("Illegal character found in the account name (%d%s character).\n", i+1, makeordinal(i+1));
			}
			return 0;
		}
	}

	if (strlen(account_name) < 4) {
		if (defaultlanguage == 'F') {
			ShowMessage("Nom du compte trop court. Entrez un nom de compte de 4-23 caractères.\n");
			ladmin_log("Nom du compte trop court. Entrez un nom de compte de 4-23 caractères.\n");
		} else {
			ShowMessage("Account name is too short. Please input an account name of 4-23 bytes.\n");
			ladmin_log("Account name is too short. Please input an account name of 4-23 bytes.\n");
		}
		return 0;
	}

	if (strlen(account_name) > 23) {
		if (defaultlanguage == 'F') {
			ShowMessage("Nom du compte trop long. Entrez un nom de compte de 4-23 caractères.\n");
			ladmin_log("Nom du compte trop long. Entrez un nom de compte de 4-23 caractères.\n");
		} else {
			ShowMessage("Account name is too long. Please input an account name of 4-23 bytes.\n");
			ladmin_log("Account name is too long. Please input an account name of 4-23 bytes.\n");
		}
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

	if (defaultlanguage == 'F') {
		ladmin_log("Aucun mot de passe n'a été donné. Demande d'un mot de passe.\n");
	} else {
		ladmin_log("No password was given. Request to obtain a password.\n");
	}

	memset(password1, '\0', sizeof(password1));
	memset(password2, '\0', sizeof(password2));
	if (defaultlanguage == 'F')
		ShowMessage("\033[1;36m Entrez le mot de passe > \033[0;32;42m");
	else
		ShowMessage("\033[1;36m Type the password > \033[0;32;42m");
		i = 0;
		while ((letter = getchar()) != '\n')
			password1[i++] = letter;
	if (defaultlanguage == 'F')
		ShowMessage("\033[0m\033[1;36m Ré-entrez le mot de passe > \033[0;32;42m");
	else
		ShowMessage("\033[0m\033[1;36m Verify the password > \033[0;32;42m");
		i = 0;
		while ((letter = getchar()) != '\n')
			password2[i++] = letter;

	ShowMessage("\033[0m");
	fflush(stdout);
	fflush(stdin);

	if (strcmp(password1, password2) != 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Erreur de vérification du mot de passe: Saisissez le même mot de passe svp.\n");
			ladmin_log("Erreur de vérification du mot de passe: Saisissez le même mot de passe svp.\n");
			ladmin_log("  Premier mot de passe: %s, second mot de passe: %s.\n", password1, password2);
		} else {
			ShowMessage("Password verification failed. Please input same password.\n");
			ladmin_log("Password verification failed. Please input same password.\n");
			ladmin_log("  First password: %s, second password: %s.\n", password1, password2);
		}
		return 0;
	}
	if (defaultlanguage == 'F') {
		ladmin_log("Mot de passe saisi: %s.\n", password1);
	} else {
		ladmin_log("Typed password: %s.\n", password1);
	}
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
			if (defaultlanguage == 'F') {
				ShowMessage("Caractère interdit trouvé dans le mot de passe (%d%s caractère).\n", i+1, makeordinal(i+1));
				ladmin_log("Caractère interdit trouvé dans le nom du compte (%d%s caractère).\n", i+1, makeordinal(i+1));
			} else {
				ShowMessage("Illegal character found in the password (%d%s character).\n", i+1, makeordinal(i+1));
				ladmin_log("Illegal character found in the password (%d%s character).\n", i+1, makeordinal(i+1));
			}
			return 0;
		}
	}

	if (strlen(password) < 4) {
		if (defaultlanguage == 'F') {
			ShowMessage("Nom du compte trop court. Entrez un nom de compte de 4-23 caractères.\n");
			ladmin_log("Nom du compte trop court. Entrez un nom de compte de 4-23 caractères.\n");
		} else {
			ShowMessage("Account name is too short. Please input an account name of 4-23 bytes.\n");
			ladmin_log("Account name is too short. Please input an account name of 4-23 bytes.\n");
		}
		return 0;
	}

	if (strlen(password) > 23) {
		if (defaultlanguage == 'F') {
			ShowMessage("Mot de passe trop long. Entrez un mot de passe de 4-23 caractères.\n");
			ladmin_log("Mot de passe trop long. Entrez un mot de passe de 4-23 caractères.\n");
		} else {
			ShowMessage("Password is too long. Please input a password of 4-23 bytes.\n");
			ladmin_log("Password is too long. Please input a password of 4-23 bytes.\n");
		}
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
	if (strncmp(command, "aide", 2) == 0 && strncmp(command, "aide", strlen(command)) == 0) // not 1 letter command: 'aide' or 'add'?
		strcpy(command, "aide");
	else if (strncmp(command, "help", 1) == 0 && strncmp(command, "help", strlen(command)) == 0)
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
	else if ((strncmp(command, "language", 2) == 0 && strncmp(command, "language", strlen(command)) == 0)) // not 1 letter command: 'language' or 'list'?
		strcpy(command, "language");
	else if ((strncmp(command, "list", 2) == 0 && strncmp(command, "list", strlen(command)) == 0) || // 'list' is default list command // not 1 letter command: 'language' or 'list'?
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
void display_help(char* param, int language)
{
	char command[1023];
	int i;

	memset(command, '\0', sizeof(command));

	if (sscanf(param, "%s ", command) < 1 || strlen(command) == 0)
		strcpy(command, ""); // any value that is not a command

	if (command[0] == '?') {
		if (defaultlanguage == 'F')
			strcpy(command, "aide");
		else
			strcpy(command, "help");
	}

	// lowercase for command
	for (i = 0; command[i]; i++)
		command[i] = TOLOWER(command[i]);

	// Analyse of the command
	check_command(command); // give complete name to the command

	if (defaultlanguage == 'F') {
		ladmin_log("Affichage des commandes ou d'une commande.\n");
	} else {
		ladmin_log("Displaying of the commands or a command.\n");
	}

	if (language == 1) {
		if (strcmp(command, "aide") == 0) {
			ShowMessage("aide/help/?\n");
			ShowMessage("  Affiche la description des commandes\n");
			ShowMessage("aide/help/? [commande]\n");
			ShowMessage("  Affiche la description de la commande specifiée\n");
		} else if (strcmp(command, "help") == 0 ) {
			ShowMessage("aide/help/?\n");
			ShowMessage("  Display the description of the commands\n");
			ShowMessage("aide/help/? [command]\n");
			ShowMessage("  Display the description of the specified command\n");
// general commands
		} else if (strcmp(command, "add") == 0) {
			ShowMessage("add <nomcompte> <sexe> <motdepasse>\n");
			ShowMessage("  Crée un compte avec l'email par défaut (a@a.com).\n");
			ShowMessage("  Concernant le sexe, seule la première lettre compte (F ou M).\n");
			ShowMessage("  L'e-mail est a@a.com (e-mail par défaut). C'est comme n'avoir aucun e-mail.\n");
			ShowMessage("  Lorsque motdepasse est omis, la saisie se fait sans que la frappe se voit.\n");
			ShowMessage("  <exemple> add testname Male testpass\n");
		} else if (strcmp(command, "ban") == 0) {
			ShowMessage("ban/banish aaaa/mm/jj hh:mm:ss <nom compte>\n");
			ShowMessage("  Change la date de fin de bannissement d'un compte.\n");
			ShowMessage("  Comme banset, mais <nom compte> est à la fin.\n");
		} else if (strcmp(command, "banadd") == 0) {
			ShowMessage("banadd <nomcompte> <Modificateur>\n");
			ShowMessage("  Ajoute ou soustrait du temps à la date de banissement d'un compte.\n");
			ShowMessage("  Les modificateurs sont construits comme suit:\n");
			ShowMessage("    Valeur d'ajustement (-1, 1, +1, etc...)\n");
			ShowMessage("    Elément modifié:\n");
			ShowMessage("      a ou y: année\n");
			ShowMessage("      m:      mois\n");
			ShowMessage("      j ou d: jour\n");
			ShowMessage("      h:      heure\n");
			ShowMessage("      mn:     minute\n");
			ShowMessage("      s:      seconde\n");
			ShowMessage("  <exemple> banadd testname +1m-2mn1s-6a\n");
			ShowMessage("            Cette exemple ajoute 1 mois et une seconde, et soustrait 2 minutes\n");
			ShowMessage("            et 6 ans dans le même temps.\n");
			ShowMessage("NOTE: Si vous modifez la date de banissement d'un compte non bani,\n");
			ShowMessage("      vous indiquez comme date (le moment actuel +- les ajustements)\n");
		} else if (strcmp(command, "banset") == 0) {
			ShowMessage("banset <nomcompte> aaaa/mm/jj [hh:mm:ss]\n");
			ShowMessage("  Change la date de fin de bannissement d'un compte.\n");
			ShowMessage("  Heure par défaut [hh:mm:ss]: 23:59:59.\n");
			ShowMessage("banset <nomcompte> 0\n");
			ShowMessage("  Débanni un compte (0 = de-banni).\n");
		} else if (strcmp(command, "block") == 0) {
			ShowMessage("block <nom compte>\n");
			ShowMessage("  Place le status d'un compte à 5 (You have been blocked by the GM Team).\n");
			ShowMessage("  La commande est l'équivalent de state <nom_compte> 5.\n");
		} else if (strcmp(command, "check") == 0) {
			ShowMessage("check <nomcompte> <motdepasse>\n");
			ShowMessage("  Vérifie la validité d'un mot de passe pour un compte\n");
			ShowMessage("  NOTE: Le serveur n'enverra jamais un mot de passe.\n");
			ShowMessage("        C'est la seule méthode que vous possédez pour savoir\n");
			ShowMessage("        si un mot de passe est le bon. L'autre méthode est\n");
			ShowMessage("        d'avoir un accès ('physique') au fichier des comptes.\n");
		} else if (strcmp(command, "create") == 0) {
			ShowMessage("create <nomcompte> <sexe> <email> <motdepasse>\n");
			ShowMessage("  Comme la commande add, mais avec l'e-mail en plus.\n");
			ShowMessage("  <exemple> create testname Male mon@mail.com testpass\n");
		} else if (strcmp(command, "delete") == 0) {
			ShowMessage("del <nom compte>\n");
			ShowMessage("  Supprime un compte.\n");
			ShowMessage("  La commande demande confirmation. Après confirmation, le compte est détruit.\n");
		} else if (strcmp(command, "email") == 0) {
			ShowMessage("email <nomcompte> <email>\n");
			ShowMessage("  Modifie l'e-mail d'un compte.\n");
		} else if (strcmp(command, "getcount") == 0) {
			ShowMessage("getcount\n");
			ShowMessage("  Donne le nombre de joueurs en ligne par serveur de char.\n");
		} else if (strcmp(command, "gm") == 0) {
			ShowMessage("gm <nomcompte> [Niveau_GM]\n");
			ShowMessage("  Modifie le niveau de GM d'un compte.\n");
			ShowMessage("  Valeur par défaut: 0 (suppression du niveau de GM).\n");
			ShowMessage("  <exemple> gm nomtest 80\n");
		} else if (strcmp(command, "id") == 0) {
			ShowMessage("id <nom compte>\n");
			ShowMessage("  Donne l'id d'un compte.\n");
		} else if (strcmp(command, "info") == 0) {
			ShowMessage("info <idcompte>\n");
			ShowMessage("  Affiche les informations sur un compte.\n");
		} else if (strcmp(command, "kami") == 0) {
			ShowMessage("kami <message>\n");
			ShowMessage("  Envoi un message général sur tous les serveurs de map (en jaune).\n");
		} else if (strcmp(command, "kamib") == 0) {
			ShowMessage("kamib <message>\n");
			ShowMessage("  Envoi un message général sur tous les serveurs de map (en bleu).\n");
		} else if (strcmp(command, "language") == 0) {
			ShowMessage("language <langue>\n");
			ShowMessage("  Change la langue d'affichage.\n");
			ShowMessage("  Langues possibles: 'Français' ou 'English'.\n");
		} else if (strcmp(command, "list") == 0) {
			ShowMessage("list/ls [Premier_id [Dernier_id]]\n");
			ShowMessage("  Affiche une liste de comptes.\n");
			ShowMessage("  'Premier_id', 'Dernier_id': indique les identifiants de départ et de fin.\n");
			ShowMessage("  La recherche par nom n'est pas possible avec cette commande.\n");
			ShowMessage("  <example> list 10 9999999\n");
		} else if (strcmp(command, "listban") == 0) {
			ShowMessage("listBan/lsBan [Premier_id [Dernier_id]]\n");
			ShowMessage("  Comme list/ls, mais seulement pour les comptes avec statut ou bannis.\n");
		} else if (strcmp(command, "listgm") == 0) {
			ShowMessage("listGM/lsGM [Premier_id [Dernier_id]]\n");
			ShowMessage("  Comme list/ls, mais seulement pour les comptes GM.\n");
		} else if (strcmp(command, "listok") == 0) {
			ShowMessage("listOK/lsOK [Premier_id [Dernier_id]]\n");
			ShowMessage("  Comme list/ls, mais seulement pour les comptes sans statut et non bannis.\n");
		} else if (strcmp(command, "memo") == 0) {
			ShowMessage("memo <nomcompte> <memo>\n");
			ShowMessage("  Modifie le mémo d'un compte.\n");
			ShowMessage("  'memo': Il peut avoir jusqu'à 253 caractères (avec des espaces ou non).\n");
		} else if (strcmp(command, "name") == 0) {
			ShowMessage("name <idcompte>\n");
			ShowMessage("  Donne le nom d'un compte.\n");
		} else if (strcmp(command, "password") == 0) {
			ShowMessage("passwd <nomcompte> <nouveaumotdepasse>\n");
			ShowMessage("  Change le mot de passe d'un compte.\n");
			ShowMessage("  Lorsque nouveaumotdepasse est omis,\n");
			ShowMessage("  la saisie se fait sans que la frappe ne se voit.\n");
		} else if (strcmp(command, "reloadgm") == 0) {
			ShowMessage("reloadGM\n");
			ShowMessage("  Reload GM configuration file\n");
		} else if (strcmp(command, "search") == 0) {
			ShowMessage("search <expression>\n");
			ShowMessage("  Cherche des comptes.\n");
			ShowMessage("  Affiche les comptes dont les noms correspondent.\n");
//			ShowMessage("search -r/-e/--expr/--regex <expression>\n");
//			ShowMessage("  Cherche des comptes par expression regulière.\n");
//			ShowMessage("  Affiche les comptes dont les noms correspondent.\n");
		} else if (strcmp(command, "sex") == 0) {
			ShowMessage("sex <nomcompte> <sexe>\n");
			ShowMessage("  Modifie le sexe d'un compte.\n");
			ShowMessage("  <exemple> sex testname Male\n");
		} else if (strcmp(command, "state") == 0) {
			ShowMessage("state <nomcompte> <nouveaustatut> <message_erreur_7>\n");
			ShowMessage("  Change le statut d'un compte.\n");
			ShowMessage("  'nouveaustatut': Le statut est le même que celui du packet 0x006a + 1.\n");
			ShowMessage("               les possibilités sont:\n");
			ShowMessage("               0 = Compte ok\n");
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
			ShowMessage("  'message_erreur_7': message du code erreur 6 =\n");
			ShowMessage("                      = Your are Prohibited to log in until... (packet 0x006a)\n");
		} else if (strcmp(command, "timeadd") == 0) {
			ShowMessage("timeadd <nomcompte> <modificateur>\n");
			ShowMessage("  Ajoute/soustrait du temps à la limite de validité d'un compte.\n");
			ShowMessage("  Le modificateur est composé comme suit:\n");
			ShowMessage("    Valeur modificatrice (-1, 1, +1, etc...)\n");
			ShowMessage("    Elément modifié:\n");
			ShowMessage("      a ou y: année\n");
			ShowMessage("      m:      mois\n");
			ShowMessage("      j ou d: jour\n");
			ShowMessage("      h:      heure\n");
			ShowMessage("      mn:     minute\n");
			ShowMessage("      s:      seconde\n");
			ShowMessage("  <exemple> timeadd testname +1m-2mn1s-6a\n");
			ShowMessage("            Cette exemple ajoute 1 mois et une seconde, et soustrait 2 minutes\n");
			ShowMessage("            et 6 ans dans le même temps.\n");
			ShowMessage("NOTE: Vous ne pouvez pas modifier une limite de validité illimitée. Si vous\n");
			ShowMessage("      désirez le faire, c'est que vous voulez probablement créer un limite de\n");
			ShowMessage("      validité limitée. Donc, en premier, fixé une limite de valitidé.\n");
		} else if (strcmp(command, "timeadd") == 0) {
			ShowMessage("timeset <nomcompte> aaaa/mm/jj [hh:mm:ss]\n");
			ShowMessage("  Change la limite de validité d'un compte.\n");
			ShowMessage("  Heure par défaut [hh:mm:ss]: 23:59:59.\n");
			ShowMessage("timeset <nomcompte> 0\n");
			ShowMessage("  Donne une limite de validité illimitée (0 = illimitée).\n");
		} else if (strcmp(command, "unban") == 0) {
			ShowMessage("unban/unbanish <nom compte>\n");
			ShowMessage("  Ote le banissement d'un compte.\n");
			ShowMessage("  La commande est l'équivalent de banset <nom_compte> 0.\n");
		} else if (strcmp(command, "unblock") == 0) {
			ShowMessage("unblock <nom compte>\n");
			ShowMessage("  Place le status d'un compte à 0 (Compte ok).\n");
			ShowMessage("  La commande est l'équivalent de state <nom_compte> 0.\n");
		} else if (strcmp(command, "version") == 0) {
			ShowMessage("version\n");
			ShowMessage("  Affiche la version du login-serveur.\n");
		} else if (strcmp(command, "who") == 0) {
			ShowMessage("who <nom compte>\n");
			ShowMessage("  Affiche les informations sur un compte.\n");
// quit
		} else if (strcmp(command, "quit") == 0 ||
		           strcmp(command, "exit") == 0 ||
		           strcmp(command, "end") == 0) {
			ShowMessage("quit/end/exit\n");
			ShowMessage("  Fin du programme d'administration.\n");
// unknown command
		} else {
			if (strlen(command) > 0)
				ShowMessage("Commande inconnue [%s] pour l'aide. Affichage de toutes les commandes.\n", command);
			ShowMessage(" aide/help/?                             -- Affiche cet aide\n");
			ShowMessage(" aide/help/? [commande]                  -- Affiche l'aide de la commande\n");
			ShowMessage(" add <nomcompte> <sexe> <motdepasse>     -- Crée un compte (sans email)\n");
			ShowMessage(" ban/banish aaaa/mm/jj hh:mm:ss <nom compte> -- Fixe la date finale de banismnt\n");
			ShowMessage(" banadd/ba <nomcompte> <modificateur>    -- Ajout/soustrait du temps à la\n");
			ShowMessage("   exemple: ba moncompte +1m-2mn1s-2y       date finale de banissement\n");
			ShowMessage(" banset/bs <nomcompte> aaaa/mm/jj [hh:mm:ss] -- Change la date fin de banisemnt\n");
			ShowMessage(" banset/bs <nomcompte> 0                 -- Dé-banis un compte.\n");
			ShowMessage(" block <nom compte>  -- Mets le status d'un compte à 5 (blocked by the GM Team)\n");
			ShowMessage(" check <nomcompte> <motdepasse>          -- Vérifie un mot de passe d'un compte\n");
			ShowMessage(" create <nomcompte> <sexe> <email> <motdepasse> -- Crée un compte (avec email)\n");
			ShowMessage(" del <nom compte>                        -- Supprime un compte\n");
			ShowMessage(" email <nomcompte> <email>               -- Modifie l'e-mail d'un compte\n");
			ShowMessage(" getcount                                -- Donne le nb de joueurs en ligne\n");
			ShowMessage("  gm <nomcompte> [Niveau_GM]              -- Modifie le niveau de GM d'un compte\n");
			ShowMessage(" id <nom compte>                         -- Donne l'id d'un compte\n");
			ShowMessage(" info <idcompte>                         -- Affiche les infos sur un compte\n");
			ShowMessage(" kami <message>                          -- Envoi un message général (en jaune)\n");
			ShowMessage(" kamib <message>                         -- Envoi un message général (en bleu)\n");
			ShowMessage(" language <langue>                       -- Change la langue d'affichage.\n");
			ShowMessage(" list/ls [Premier_id [Dernier_id] ]      -- Affiche une liste de comptes\n");
			ShowMessage(" listBan/lsBan [Premier_id [Dernier_id] ] -- Affiche une liste de comptes\n");
			ShowMessage("                                             avec un statut ou bannis\n");
			ShowMessage(" listGM/lsGM [Premier_id [Dernier_id] ]  -- Affiche une liste de comptes GM\n");
			ShowMessage(" listOK/lsOK [Premier_id [Dernier_id] ]  -- Affiche une liste de comptes\n");
			ShowMessage("                                            sans status et non bannis\n");
			ShowMessage(" memo <nomcompte> <memo>                 -- Modifie le memo d'un compte\n");
			ShowMessage(" name <idcompte>                         -- Donne le nom d'un compte\n");
			ShowMessage(" passwd <nomcompte> <nouveaumotdepasse>  -- Change le mot de passe d'un compte\n");
			ShowMessage(" quit/end/exit                           -- Fin du programme d'administation\n");
			ShowMessage(" reloadGM                              -- Recharger le fichier de config des GM\n");
			ShowMessage(" search <expression>                     -- Cherche des comptes\n");
//			ShowMessage(" search -e/-r/--expr/--regex <expression> -- Cherche des comptes par REGEX\n");
			ShowMessage(" sex <nomcompte> <sexe>                  -- Modifie le sexe d'un compte\n");
			ShowMessage(" state <nomcompte> <nouveaustatut> <messageerr7> -- Change le statut d'1 compte\n");
			ShowMessage(" timeadd/ta <nomcompte> <modificateur>   -- Ajout/soustrait du temps à la\n");
			ShowMessage("   exemple: ta moncompte +1m-2mn1s-2y       limite de validité\n");
			ShowMessage(" timeset/ts <nomcompte> aaaa/mm/jj [hh:mm:ss] -- Change la limite de validité\n");
			ShowMessage(" timeset/ts <nomcompte> 0                -- limite de validité = illimitée\n");
			ShowMessage(" unban/unbanish <nom compte>             -- Ote le banissement d'un compte\n");
			ShowMessage(" unblock <nom compte>             -- Mets le status d'un compte à 0 (Compte ok)\n");
			ShowMessage(" version                                 -- Donne la version du login-serveur\n");
			ShowMessage(" who <nom compte>                        -- Affiche les infos sur un compte\n");
			ShowMessage(" Note: Pour les noms de compte avec des espaces, tapez \"<nom compte>\" (ou ').\n");
		}
	} else {
		if (strcmp(command, "aide") == 0) {
			ShowMessage("aide/help/?\n");
			ShowMessage("  Display the description of the commands\n");
			ShowMessage("aide/help/? [command]\n");
			ShowMessage("  Display the description of the specified command\n");
		} else if (strcmp(command, "help") == 0 ) {
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
		} else if (strcmp(command, "language") == 0) {
			ShowMessage("language <language>\n");
			ShowMessage("  Change the language of displaying.\n");
			ShowMessage("  Possible languages: Français or English.\n");
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
			ShowMessage(" language <language>                  -- Change the language of displaying.\n");
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
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un nom de compte, un sexe et un mot de passe svp.\n");
				ShowMessage("<exemple> add nomtest Male motdepassetest\n");
				ladmin_log("Nombre incorrect de paramètres pour créer un compte (commande 'add').\n");
			} else {
				ShowMessage("Please input an account name, a sex and a password.\n");
				ShowMessage("<example> add testname Male testpass\n");
				ladmin_log("Incomplete parameters to create an account ('add' command).\n");
			}
			return 136;
		}
		strcpy(email, "a@a.com"); // default email
	} else { // 1: create command
		if (sscanf(param, "\"%[^\"]\" %s %s %[^\r\n]", name, sex, email, password) < 3 && // password can be void
		    sscanf(param, "'%[^']' %s %s %[^\r\n]", name, sex, email, password) < 3 && // password can be void
		    sscanf(param, "%s %s %s %[^\r\n]", name, sex, email, password) < 3) { // password can be void
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un nom de compte, un sexe et un mot de passe svp.\n");
				ShowMessage("<exemple> create nomtest Male mo@mail.com motdepassetest\n");
				ladmin_log("Nombre incorrect de paramètres pour créer un compte (commande 'create').\n");
			} else {
				ShowMessage("Please input an account name, a sex and a password.\n");
				ShowMessage("<example> create testname Male my@mail.com testpass\n");
				ladmin_log("Incomplete parameters to create an account ('create' command).\n");
			}
			return 136;
		}
	}
	if (verify_accountname(name) == 0) {
		return 102;
	}

	sex[0] = TOUPPER(sex[0]);
	if (strchr("MF", sex[0]) == NULL) {
		if (defaultlanguage == 'F') {
			ShowMessage("Sexe incorrect [%s]. Entrez M ou F svp.\n", sex);
			ladmin_log("Sexe incorrect [%s]. Entrez M ou F svp.\n", sex);
		} else {
			ShowMessage("Illegal gender [%s]. Please input M or F.\n", sex);
			ladmin_log("Illegal gender [%s]. Please input M or F.\n", sex);
		}
		return 103;
	}

	if (strlen(email) < 3) {
		if (defaultlanguage == 'F') {
			ShowMessage("Email trop courte [%s]. Entrez une e-mail valide svp.\n", email);
			ladmin_log("Email trop courte [%s]. Entrez une e-mail valide svp.\n", email);
		} else {
			ShowMessage("Email is too short [%s]. Please input a valid e-mail.\n", email);
			ladmin_log("Email is too short [%s]. Please input a valid e-mail.\n", email);
		}
		return 109;
	}
	if (strlen(email) > 39) {
		if (defaultlanguage == 'F') {
			ShowMessage("Email trop longue [%s]. Entrez une e-mail de 39 caractères maximum svp.\n", email);
			ladmin_log("Email trop longue [%s]. Entrez une e-mail de 39 caractères maximum svp.\n", email);
		} else {
			ShowMessage("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
			ladmin_log("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
		}
		return 109;
	}
	if (e_mail_check(email) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Email incorrecte [%s]. Entrez une e-mail valide svp.\n", email);
			ladmin_log("Email incorrecte [%s]. Entrez une e-mail valide svp.\n", email);
		} else {
			ShowMessage("Invalid email [%s]. Please input a valid e-mail.\n", email);
			ladmin_log("Invalid email [%s]. Please input a valid e-mail.\n", email);
		}
		return 109;
	}

	if (strlen(password) == 0) {
		if (typepasswd(password) == 0)
			return 108;
	}
	if (verify_password(password) == 0)
		return 104;

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour créer un compte.\n");
	} else {
		ladmin_log("Request to login-server to create an account.\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte et un modificateur svp.\n");
			ShowMessage("  <exemple> banadd nomtest +1m-2mn1s-6y\n");
			ShowMessage("            Cette exemple ajoute 1 mois et 1 seconde, et soustrait 2 minutes\n");
			ShowMessage("            et 6 ans dans le même temps.\n");
			ladmin_log("Nombre incorrect de paramètres pour modifier la fin de ban d'un compte (commande 'banadd').\n");
		} else {
			ShowMessage("Please input an account name and a modifier.\n");
			ShowMessage("  <example>: banadd testname +1m-2mn1s-6y\n");
			ShowMessage("             this example adds 1 month and 1 second, and substracts 2 minutes\n");
			ShowMessage("             and 6 years at the same time.\n");
			ladmin_log("Incomplete parameters to modify the ban date/time of an account ('banadd' command).\n");
		}
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

	if (defaultlanguage == 'F') {
		ShowMessage(" année:   %d\n", year);
		ShowMessage(" mois:    %d\n", month);
		ShowMessage(" jour:    %d\n", day);
		ShowMessage(" heure:   %d\n", hour);
		ShowMessage(" minute:  %d\n", minute);
		ShowMessage(" seconde: %d\n", second);
	} else {
		ShowMessage(" year:   %d\n", year);
		ShowMessage(" month:  %d\n", month);
		ShowMessage(" day:    %d\n", day);
		ShowMessage(" hour:   %d\n", hour);
		ShowMessage(" minute: %d\n", minute);
		ShowMessage(" second: %d\n", second);
	}

	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Vous devez entrer un ajustement avec cette commande, svp:\n");
			ShowMessage("  Valeur d'ajustement (-1, 1, +1, etc...)\n");
			ShowMessage("  Element modifié:\n");
			ShowMessage("    a ou y: année\n");
			ShowMessage("    m:      mois\n");
			ShowMessage("    j ou d: jour\n");
			ShowMessage("    h:      heure\n");
			ShowMessage("    mn:     minute\n");
			ShowMessage("    s:      seconde\n");
			ShowMessage("  <exemple> banadd nomtest +1m-2mn1s-6y\n");
			ShowMessage("            Cette exemple ajoute 1 mois et 1 seconde, et soustrait 2 minutes\n");
			ShowMessage("            et 6 ans dans le même temps.\n");
			ladmin_log("Aucun ajustement n'est pas un ajustement (commande 'banadd').\n");
		} else {
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
		}
		return 137;
	}
	if (year > 127 || year < -127) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement d'années correct (de -127 à 127), svp.\n");
			ladmin_log("Ajustement de l'année hors norme (commande 'banadd').\n");
		} else {
			ShowMessage("Please give a correct adjustment for the years (from -127 to 127).\n");
			ladmin_log("Abnormal adjustment for the year ('banadd' command).\n");
		}
		return 137;
	}
	if (month > 255 || month < -255) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de mois correct (de -255 à 255), svp.\n");
			ladmin_log("Ajustement du mois hors norme (commande 'banadd').\n");
		} else {
			ShowMessage("Please give a correct adjustment for the months (from -255 to 255).\n");
			ladmin_log("Abnormal adjustment for the month ('banadd' command).\n");
		}
		return 137;
	}
	if (day > 32767 || day < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de jours correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des jours hors norme (commande 'banadd').\n");
		} else {
			ShowMessage("Please give a correct adjustment for the days (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the days ('banadd' command).\n");
		}
		return 137;
	}
	if (hour > 32767 || hour < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement d'heures correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des heures hors norme (commande 'banadd').\n");
		} else {
			ShowMessage("Please give a correct adjustment for the hours (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the hours ('banadd' command).\n");
		}
		return 137;
	}
	if (minute > 32767 || minute < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de minutes correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des minutes hors norme (commande 'banadd').\n");
		} else {
			ShowMessage("Please give a correct adjustment for the minutes (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the minutes ('banadd' command).\n");
		}
		return 137;
	}
	if (second > 32767 || second < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de secondes correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des secondes hors norme (commande 'banadd').\n");
		} else {
			ShowMessage("Please give a correct adjustment for the seconds (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the seconds ('banadd' command).\n");
		}
		return 137;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour modifier la date d'un bannissement.\n");
	} else {
		ladmin_log("Request to login-server to modify a ban date/time.\n");
	}

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
	time_t ban_until_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
	struct tm *tmtime;
	WFIFOHEAD(login_fd,30);

	year = month = day = hour = minute = second = 0;
	ban_until_time = 0;
	tmtime = localtime(&ban_until_time); // initialize

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (atoi(date) != 0 &&
	    ((sscanf(date, "%d/%d/%d", &year, &month, &day) < 3 &&
	      sscanf(date, "%d-%d-%d", &year, &month, &day) < 3 &&
	      sscanf(date, "%d.%d.%d", &year, &month, &day) < 3) ||
	     sscanf(time, "%d:%d:%d", &hour, &minute, &second) < 3)) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez une date et une heure svp (format: aaaa/mm/jj hh:mm:ss).\n");
			ShowMessage("Vous pouvez aussi mettre 0 à la place si vous utilisez la commande 'banset'.\n");
			ladmin_log("Format incorrect pour la date/heure (commande'banset' ou 'ban').\n");
		} else {
			ShowMessage("Please input a date and a time (format: yyyy/mm/dd hh:mm:ss).\n");
			ShowMessage("You can imput 0 instead of if you use 'banset' command.\n");
			ladmin_log("Invalid format for the date/time ('banset' or 'ban' command).\n");
		}
		return 102;
	}

	if (atoi(date) == 0) {
		ban_until_time = 0;
	} else {
		if (year < 70) {
			year = year + 100;
		}
		if (year >= 1900) {
			year = year - 1900;
		}
		if (month < 1 || month > 12) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un mois correct svp (entre 1 et 12).\n");
				ladmin_log("Mois incorrect pour la date (command 'banset' ou 'ban').\n");
			} else {
				ShowMessage("Please give a correct value for the month (from 1 to 12).\n");
				ladmin_log("Invalid month for the date ('banset' or 'ban' command).\n");
			}
			return 102;
		}
		month = month - 1;
		if (day < 1 || day > 31) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un jour correct svp (entre 1 et 31).\n");
				ladmin_log("Jour incorrect pour la date (command 'banset' ou 'ban').\n");
			} else {
				ShowMessage("Please give a correct value for the day (from 1 to 31).\n");
				ladmin_log("Invalid day for the date ('banset' or 'ban' command).\n");
			}
			return 102;
		}
		if (((month == 3 || month == 5 || month == 8 || month == 10) && day > 30) ||
		    (month == 1 && day > 29)) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un jour correct en fonction du mois (%d) svp.\n", month);
				ladmin_log("Jour incorrect pour ce mois correspondant (command 'banset' ou 'ban').\n");
			} else {
				ShowMessage("Please give a correct value for a day of this month (%d).\n", month);
				ladmin_log("Invalid day for this month ('banset' or 'ban' command).\n");
			}
			return 102;
		}
		if (hour < 0 || hour > 23) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez une heure correcte svp (entre 0 et 23).\n");
				ladmin_log("Heure incorrecte pour l'heure (command 'banset' ou 'ban').\n");
			} else {
				ShowMessage("Please give a correct value for the hour (from 0 to 23).\n");
				ladmin_log("Invalid hour for the time ('banset' or 'ban' command).\n");
			}
			return 102;
		}
		if (minute < 0 || minute > 59) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez des minutes correctes svp (entre 0 et 59).\n");
				ladmin_log("Minute incorrecte pour l'heure (command 'banset' ou 'ban').\n");
			} else {
				ShowMessage("Please give a correct value for the minutes (from 0 to 59).\n");
				ladmin_log("Invalid minute for the time ('banset' or 'ban' command).\n");
			}
			return 102;
		}
		if (second < 0 || second > 59) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez des secondes correctes svp (entre 0 et 59).\n");
				ladmin_log("Seconde incorrecte pour l'heure (command 'banset' ou 'ban').\n");
			} else {
				ShowMessage("Please give a correct value for the seconds (from 0 to 59).\n");
				ladmin_log("Invalid second for the time ('banset' or 'ban' command).\n");
			}
			return 102;
		}
		tmtime->tm_year = year;
		tmtime->tm_mon = month;
		tmtime->tm_mday = day;
		tmtime->tm_hour = hour;
		tmtime->tm_min = minute;
		tmtime->tm_sec = second;
		tmtime->tm_isdst = -1; // -1: no winter/summer time modification
		ban_until_time = mktime(tmtime);
		if (ban_until_time == -1) {
			if (defaultlanguage == 'F') {
				ShowMessage("Date incorrecte.\n");
				ShowMessage("Entrez une date et une heure svp (format: aaaa/mm/jj hh:mm:ss).\n");
				ShowMessage("Vous pouvez aussi mettre 0 à la place si vous utilisez la commande 'banset'.\n");
				ladmin_log("Date incorrecte. (command 'banset' ou 'ban').\n");
			} else {
				ShowMessage("Invalid date.\n");
				ShowMessage("Please input a date and a time (format: yyyy/mm/dd hh:mm:ss).\n");
				ShowMessage("You can imput 0 instead of if you use 'banset' command.\n");
				ladmin_log("Invalid date. ('banset' or 'ban' command).\n");
			}
			return 102;
		}
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour fixer un ban.\n");
	} else {
		ladmin_log("Request to login-server to set a ban.\n");
	}

	WFIFOW(login_fd,0) = 0x794a;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOL(login_fd,26) = (int)ban_until_time;
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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte, une date et une heure svp.\n");
			ShowMessage("<exemple>: banset <nom_du_compte> aaaa/mm/jj [hh:mm:ss]\n");
			ShowMessage("           banset <nom_du_compte> 0    (0 = dé-bani)\n");
			ShowMessage("           ban/banish aaaa/mm/jj hh:mm:ss <nom du compte>\n");
			ShowMessage("           unban/unbanish <nom du compte>\n");
			ShowMessage("           Heure par défaut [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Nombre incorrect de paramètres pour fixer un ban (commande 'banset' ou 'ban').\n");
		} else {
			ShowMessage("Please input an account name, a date and a hour.\n");
			ShowMessage("<example>: banset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
			ShowMessage("           banset <account_name> 0   (0 = un-banished)\n");
			ShowMessage("           ban/banish yyyy/mm/dd hh:mm:ss <account name>\n");
			ShowMessage("           unban/unbanish <account name>\n");
			ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Incomplete parameters to set a ban ('banset' or 'ban' command).\n");
		}
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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte, une date et une heure svp.\n");
			ShowMessage("<exemple>: banset <nom_du_compte> aaaa/mm/jj [hh:mm:ss]\n");
			ShowMessage("           banset <nom_du_compte> 0    (0 = dé-bani)\n");
			ShowMessage("           ban/banish aaaa/mm/jj hh:mm:ss <nom du compte>\n");
			ShowMessage("           unban/unbanish <nom du compte>\n");
			ShowMessage("           Heure par défaut [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Nombre incorrect de paramètres pour fixer un ban (commande 'banset' ou 'ban').\n");
		} else {
			ShowMessage("Please input an account name, a date and a hour.\n");
			ShowMessage("<example>: banset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
			ShowMessage("           banset <account_name> 0   (0 = un-banished)\n");
			ShowMessage("           ban/banish yyyy/mm/dd hh:mm:ss <account name>\n");
			ShowMessage("           unban/unbanish <account name>\n");
			ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Incomplete parameters to set a ban ('banset' or 'ban' command).\n");
		}
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
	     strlen(name) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemple>: banset <nom_du_compte> aaaa/mm/jj [hh:mm:ss]\n");
			ShowMessage("           banset <nom_du_compte> 0    (0 = dé-bani)\n");
			ShowMessage("           ban/banish aaaa/mm/jj hh:mm:ss <nom du compte>\n");
			ShowMessage("           unban/unbanish <nom du compte>\n");
			ShowMessage("           Heure par défaut [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Nombre incorrect de paramètres pour fixer un ban (commande 'unban').\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<example>: banset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
			ShowMessage("           banset <account_name> 0   (0 = un-banished)\n");
			ShowMessage("           ban/banish yyyy/mm/dd hh:mm:ss <account name>\n");
			ShowMessage("           unban/unbanish <account name>\n");
			ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Incomplete parameters to set a ban ('unban' command).\n");
		}
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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemple> check testname motdepasse\n");
			ladmin_log("Nombre incorrect de paramètres pour tester le mot d'un passe d'un compte (commande 'check').\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<example> check testname password\n");
			ladmin_log("Incomplete parameters to check the password of an account ('check' command).\n");
		}
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

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour test un mot de passe.\n");
	} else {
		ladmin_log("Request to login-server to check a password.\n");
	}

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
	     strlen(name) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemple> del nomtestasupprimer\n");
			ladmin_log("Aucun nom donné pour supprimer un compte (commande 'delete').\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<example> del testnametodelete\n");
			ladmin_log("No name given to delete an account ('delete' command).\n");
		}
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	memset(confirm, '\0', sizeof(confirm));
	while ((confirm[0] != 'o' || defaultlanguage != 'F') && confirm[0] != 'n' && (confirm[0] != 'y' || defaultlanguage == 'F')) {
		if (defaultlanguage == 'F')
			ShowMessage("\033[1;36m ** Etes-vous vraiment sûr de vouloir SUPPRIMER le compte [$userid]? (o/n) > \033[0m");
		else
			ShowMessage("\033[1;36m ** Are you really sure to DELETE account [$userid]? (y/n) > \033[0m");
		fflush(stdout);
		memset(confirm, '\0', sizeof(confirm));
		i = 0;
		while ((letter = getchar()) != '\n')
			confirm[i++] = letter;
	}

	if (confirm[0] == 'n') {
		if (defaultlanguage == 'F') {
			ShowMessage("Suppression annulée.\n");
			ladmin_log("Suppression annulée par l'utilisateur (commande 'delete').\n");
		} else {
			ShowMessage("Deletion canceled.\n");
			ladmin_log("Deletion canceled by user ('delete' command).\n");
		}
		return 121;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour détruire un compte.\n");
	} else {
		ladmin_log("Request to login-server to delete an acount.\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte et une email svp.\n");
			ShowMessage("<exemple> email testname nouveauemail\n");
			ladmin_log("Nombre incorrect de paramètres pour changer l'email d'un compte (commande 'email').\n");
		} else {
			ShowMessage("Please input an account name and an email.\n");
			ShowMessage("<example> email testname newemail\n");
			ladmin_log("Incomplete parameters to change the email of an account ('email' command).\n");
		}
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (strlen(email) < 3) {
		if (defaultlanguage == 'F') {
			ShowMessage("Email trop courte [%s]. Entrez une e-mail valide svp.\n", email);
			ladmin_log("Email trop courte [%s]. Entrez une e-mail valide svp.\n", email);
		} else {
			ShowMessage("Email is too short [%s]. Please input a valid e-mail.\n", email);
			ladmin_log("Email is too short [%s]. Please input a valid e-mail.\n", email);
		}
		return 109;
	}
	if (strlen(email) > 39) {
		if (defaultlanguage == 'F') {
			ShowMessage("Email trop longue [%s]. Entrez une e-mail de 39 caractères maximum svp.\n", email);
			ladmin_log("Email trop longue [%s]. Entrez une e-mail de 39 caractères maximum svp.\n", email);
		} else {
			ShowMessage("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
			ladmin_log("Email is too long [%s]. Please input an e-mail with 39 bytes at the most.\n", email);
		}
		return 109;
	}
	if (e_mail_check(email) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Email incorrecte [%s]. Entrez une e-mail valide svp.\n", email);
			ladmin_log("Email incorrecte [%s]. Entrez une e-mail valide svp.\n", email);
		} else {
			ShowMessage("Invalid email [%s]. Please input a valid e-mail.\n", email);
			ladmin_log("Invalid email [%s]. Please input a valid e-mail.\n", email);
		}
		return 109;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour changer une email.\n");
	} else {
		ladmin_log("Request to login-server to change an email.\n");
	}

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
	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour obtenir le nombre de joueurs en jeu.\n");
	} else {
		ladmin_log("Request to login-server to obtain the # of online players.\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte et un niveau de GM svp.\n");
			ShowMessage("<exemple> gm nomtest 80\n");
			ladmin_log("Nombre incorrect de paramètres pour changer le Niveau de GM d'un compte (commande 'gm').\n");
		} else {
			ShowMessage("Please input an account name and a GM level.\n");
			ShowMessage("<example> gm testname 80\n");
			ladmin_log("Incomplete parameters to change the GM level of an account ('gm' command).\n");
		}
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (GM_level < 0 || GM_level > 99) {
		if (defaultlanguage == 'F') {
			ShowMessage("Niveau de GM incorrect [%d]. Entrez une valeur de 0 à 99 svp.\n", GM_level);
			ladmin_log("Niveau de GM incorrect [%d]. La valeur peut être de 0 à 99.\n", GM_level);
		} else {
			ShowMessage("Illegal GM level [%d]. Please input a value from 0 to 99.\n", GM_level);
			ladmin_log("Illegal GM level [%d]. The value can be from 0 to 99.\n", GM_level);
		}
		return 103;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour changer un niveau de GM.\n");
	} else {
		ladmin_log("Request to login-server to change a GM level.\n");
	}

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
	     strlen(name) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemple> id nomtest\n");
			ladmin_log("Aucun nom donné pour rechecher l'id d'un compte (commande 'id').\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<example> id testname\n");
			ladmin_log("No name given to search an account id ('id' command).\n");
		}
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour connaître l'id d'un compte.\n");
	} else {
		ladmin_log("Request to login-server to know an account id.\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un id ayant une valeur positive svp.\n");
			ladmin_log("Une valeur négative a été donné pour trouver le compte.\n");
		} else {
			ShowMessage("Please input a positive value for the id.\n");
			ladmin_log("Negative value was given to found the account.\n");
		}
		return 136;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour obtenir le information d'un compte (par l'id).\n");
	} else {
		ladmin_log("Request to login-server to obtain information about an account (by its id).\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un message svp.\n");
			if (type == 0) {
				ShowMessage("<exemple> kami un message\n");
			} else {
				ShowMessage("<exemple> kamib un message\n");
			}
			ladmin_log("Le message est vide (commande 'kami(b)').\n");
		} else {
			ShowMessage("Please input a message.\n");
			if (type == 0) {
				ShowMessage("<example> kami a message\n");
			} else {
				ShowMessage("<example> kamib a message\n");
			}
			ladmin_log("The message is void ('kami(b)' command).\n");
		}
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

//--------------------------------------------
// Sub-function: Change language of displaying
//--------------------------------------------
int changelanguage(char* language)
{
	if (strlen(language) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez une langue svp.\n");
			ShowMessage("<exemple> language english\n");
			ShowMessage("          language français\n");
			ladmin_log("La langue est vide (commande 'language').\n");
		} else {
			ShowMessage("Please input a language.\n");
			ShowMessage("<example> language english\n");
			ShowMessage("          language français\n");
			ladmin_log("The language is void ('language' command).\n");
		}
		return 136;
	}

	language[0] = TOUPPER(language[0]);
	if (language[0] == 'F' || language[0] == 'E') {
		defaultlanguage = language[0];
		if (defaultlanguage == 'F') {
			ShowMessage("Changement de la langue d'affichage en Français.\n");
			ladmin_log("Changement de la langue d'affichage en Français.\n");
		} else {
			ShowMessage("Displaying language changed to English.\n");
			ladmin_log("Displaying language changed to English.\n");
		}
	} else {
		if (defaultlanguage == 'F') {
			ShowMessage("Langue non paramétrée (langues possibles: 'Français' ou 'English').\n");
			ladmin_log("Langue non paramétrée (Français ou English nécessaire).\n");
		} else {
			ShowMessage("Undefined language (possible languages: Français or English).\n");
			ladmin_log("Undefined language (must be Français or English).\n");
		}
	}

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

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour obtenir la liste des comptes de %d à %d.\n", list_first, list_last);
	} else {
		ladmin_log("Request to login-server to obtain the list of accounts from %d to %d.\n", list_first, list_last);
	}

	WFIFOW(login_fd,0) = 0x7920;
	WFIFOL(login_fd,2) = list_first;
	WFIFOL(login_fd,6) = list_last;
	WFIFOSET(login_fd,10);
	bytes_to_read = 1;

	//          0123456789 01 01234567890123456789012301234 012345 0123456789012345678901234567
	if (defaultlanguage == 'F') {
		ShowMessage(" id_compte GM nom_utilisateur         sexe   count statut\n");
	} else {
		ShowMessage("account_id GM user_name               sex    count state\n");
	}
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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte et un mémo svp.\n");
			ShowMessage("<exemple> memo nomtest nouveau memo\n");
			ladmin_log("Nombre incorrect de paramètres pour changer le mémo d'un compte (commande 'email').\n");
		} else {
			ShowMessage("Please input an account name and a memo.\n");
			ShowMessage("<example> memo testname new memo\n");
			ladmin_log("Incomplete parameters to change the memo of an account ('email' command).\n");
		}
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (strlen(memo) > 254) {
		if (defaultlanguage == 'F') {
			ShowMessage("Mémo trop long (%lu caractères).\n", (unsigned long)strlen(memo));
			ShowMessage("Entrez un mémo de 254 caractères maximum svp.\n");
			ladmin_log("Mémo trop long (%d caractères). Entrez un mémo de 254 caractères maximum svp.\n", strlen(memo));
		} else {
			ShowMessage("Memo is too long (%lu characters).\n", (unsigned long)strlen(memo));
			ShowMessage("Please input a memo of 254 bytes at the maximum.\n");
			ladmin_log("Email is too long (%d characters). Please input a memo of 254 bytes at the maximum.\n", strlen(memo));
		}
		return 102;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour changer un mémo.\n");
	} else {
		ladmin_log("Request to login-server to change a memo.\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un id ayant une valeur positive svp.\n");
			ladmin_log("Id négatif donné pour rechecher le nom d'un compte (commande 'name').\n");
		} else {
			ShowMessage("Please input a positive value for the id.\n");
			ladmin_log("Negativ id given to search an account name ('name' command).\n");
		}
		return 136;
	}

	if (defaultlanguage == 'F')
		ladmin_log("Envoi d'un requête au serveur de logins pour connaître le nom d'un compte.\n");
	else
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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemple> passwd nomtest nouveaumotdepasse\n");
			ladmin_log("Nombre incorrect de paramètres pour changer le mot d'un passe d'un compte (commande 'password').\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<example> passwd testname newpassword\n");
			ladmin_log("Incomplete parameters to change the password of an account ('password' command).\n");
		}
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

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour changer un mot de passe.\n");
	} else {
		ladmin_log("Request to login-server to change a password.\n");
	}

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

	if (defaultlanguage == 'F') {
		ladmin_log("Demande de recharger le fichier de configuration des GM envoyée.\n");
		ShowMessage("Demande de recharger le fichier de configuration des GM envoyée.\n");
		ShowMessage("Vérifiez les comptes GM actuels (après rechargement):\n");
	} else {
		ladmin_log("Request to reload the GM configuration file sended.\n");
		ShowMessage("Request to reload the GM configuration file sended.\n");
		ShowMessage("Check the actual GM accounts (after reloading):\n");
	}
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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte et un sexe svp.\n");
			ShowMessage("<exemple> sex nomtest Male\n");
			ladmin_log("Nombre incorrect de paramètres pour changer le sexe d'un compte (commande 'sex').\n");
		} else {
			ShowMessage("Please input an account name and a sex.\n");
			ShowMessage("<example> sex testname Male\n");
			ladmin_log("Incomplete parameters to change the sex of an account ('sex' command).\n");
		}
		return 136;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	sex[0] = TOUPPER(sex[0]);
	if (strchr("MF", sex[0]) == NULL) {
		if (defaultlanguage == 'F') {
			ShowMessage("Sexe incorrect [%s]. Entrez M ou F svp.\n", sex);
			ladmin_log("Sexe incorrect [%s]. Entrez M ou F svp.\n", sex);
		} else {
			ShowMessage("Illegal gender [%s]. Please input M or F.\n", sex);
			ladmin_log("Illegal gender [%s]. Please input M or F.\n", sex);
		}
		return 103;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour changer un sexe.\n");
	} else {
		ladmin_log("Request to login-server to change a sex.\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez une des statuts suivantes svp:\n");
			ShowMessage("  0 = Compte ok             6 = Your Game's EXE file is not the latest version\n");
		} else {
			ShowMessage("Please input one of these states:\n");
			ShowMessage("  0 = Account ok            6 = Your Game's EXE file is not the latest version\n");
		}
		ShowMessage("  1 = Unregistered ID       7 = You are Prohibited to log in until + message\n");
		ShowMessage("  2 = Incorrect Password    8 = Server is jammed due to over populated\n");
		ShowMessage("  3 = This ID is expired    9 = No MSG\n");
		ShowMessage("  4 = Rejected from Server  100 = This ID has been totally erased\n");
		ShowMessage("  5 = You have been blocked by the GM Team\n");
		if (defaultlanguage == 'F') {
			ShowMessage("<exemples> state nomtest 5\n");
			ShowMessage("           state nomtest 7 fin de votre ban\n");
			ShowMessage("           block <nom compte>\n");
			ShowMessage("           unblock <nom compte>\n");
			ladmin_log("Valeur incorrecte pour le statut d'un compte (commande 'state', 'block' ou 'unblock').\n");
		} else {
			ShowMessage("<examples> state testname 5\n");
			ShowMessage("           state testname 7 end of your ban\n");
			ShowMessage("           block <account name>\n");
			ShowMessage("           unblock <account name>\n");
			ladmin_log("Invalid value for the state of an account ('state', 'block' or 'unblock' command).\n");
		}
		return 151;
	}

	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (state != 7) {
		strcpy(error_message, "-");
	} else {
		if (strlen(error_message) < 1) {
			if (defaultlanguage == 'F') {
				ShowMessage("Message d'erreur trop court. Entrez un message de 1-19 caractères.\n");
				ladmin_log("Message d'erreur trop court. Entrez un message de 1-19 caractères.\n");
			} else {
				ShowMessage("Error message is too short. Please input a message of 1-19 bytes.\n");
				ladmin_log("Error message is too short. Please input a message of 1-19 bytes.\n");
			}
			return 102;
		}
		if (strlen(error_message) > 19) {
			if (defaultlanguage == 'F') {
				ShowMessage("Message d'erreur trop long. Entrez un message de 1-19 caractères.\n");
				ladmin_log("Message d'erreur trop long. Entrez un message de 1-19 caractères.\n");
			} else {
				ShowMessage("Error message is too long. Please input a message of 1-19 bytes.\n");
				ladmin_log("Error message is too long. Please input a message of 1-19 bytes.\n");
			}
			return 102;
		}
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour changer un statut.\n");
	} else {
		ladmin_log("Request to login-server to change a state.\n");
	}

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
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte et un statut svp.\n");
			ShowMessage("<exemples> state nomtest 5\n");
			ShowMessage("           state nomtest 7 fin de votre ban\n");
			ShowMessage("           block <nom compte>\n");
			ShowMessage("           unblock <nom compte>\n");
			ladmin_log("Nombre incorrect de paramètres pour changer le statut d'un compte (commande 'state').\n");
		} else {
			ShowMessage("Please input an account name and a state.\n");
			ShowMessage("<examples> state testname 5\n");
			ShowMessage("           state testname 7 end of your ban\n");
			ShowMessage("           block <account name>\n");
			ShowMessage("           unblock <account name>\n");
			ladmin_log("Incomplete parameters to change the state of an account ('state' command).\n");
		}
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
	     strlen(name) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemples> state nomtest 5\n");
			ShowMessage("           state nomtest 7 fin de votre ban\n");
			ShowMessage("           block <nom compte>\n");
			ShowMessage("           unblock <nom compte>\n");
			ladmin_log("Nombre incorrect de paramètres pour changer le statut d'un compte (commande 'unblock').\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<examples> state testname 5\n");
			ShowMessage("           state testname 7 end of your ban\n");
			ShowMessage("           block <account name>\n");
			ShowMessage("           unblock <account name>\n");
			ladmin_log("Incomplete parameters to change the state of an account ('unblock' command).\n");
		}
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
	     strlen(name) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemples> state nomtest 5\n");
			ShowMessage("           state nomtest 7 fin de votre ban\n");
			ShowMessage("           block <nom compte>\n");
			ShowMessage("           unblock <nom compte>\n");
			ladmin_log("Nombre incorrect de paramètres pour changer le statut d'un compte (commande 'block').\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<examples> state testname 5\n");
			ShowMessage("           state testname 7 end of your ban\n");
			ShowMessage("           block <account name>\n");
			ShowMessage("           unblock <account name>\n");
			ladmin_log("Incomplete parameters to change the state of an account ('block' command).\n");
		}
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
	    sscanf(param, "%s %[^\r\n]", name, modif) < 2) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte et un modificateur svp.\n");
			ShowMessage("  <exemple> timeadd nomtest +1m-2mn1s-6y\n");
			ShowMessage("            Cette exemple ajoute 1 mois et 1 seconde, et soustrait 2 minutes\n");
			ShowMessage("            et 6 ans dans le même temps.\n");
			ladmin_log("Nombre incorrect de paramètres pour modifier une date limite d'utilisation (commande 'timeadd').\n");
		} else {
			ShowMessage("Please input an account name and a modifier.\n");
			ShowMessage("  <example>: timeadd testname +1m-2mn1s-6y\n");
			ShowMessage("             this example adds 1 month and 1 second, and substracts 2 minutes\n");
			ShowMessage("             and 6 years at the same time.\n");
			ladmin_log("Incomplete parameters to modify a limit time ('timeadd' command).\n");
		}
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

	if (defaultlanguage == 'F') {
		ShowMessage(" année:   %d\n", year);
		ShowMessage(" mois:    %d\n", month);
		ShowMessage(" jour:    %d\n", day);
		ShowMessage(" heure:   %d\n", hour);
		ShowMessage(" minute:  %d\n", minute);
		ShowMessage(" seconde: %d\n", second);
	} else {
		ShowMessage(" year:   %d\n", year);
		ShowMessage(" month:  %d\n", month);
		ShowMessage(" day:    %d\n", day);
		ShowMessage(" hour:   %d\n", hour);
		ShowMessage(" minute: %d\n", minute);
		ShowMessage(" second: %d\n", second);
	}

	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Vous devez entrer un ajustement avec cette commande, svp:\n");
			ShowMessage("  Valeur d'ajustement (-1, 1, +1, etc...)\n");
			ShowMessage("  Elément modifié:\n");
			ShowMessage("    a ou y: année\n");
			ShowMessage("    m:      mois\n");
			ShowMessage("    j ou d: jour\n");
			ShowMessage("    h:      heure\n");
			ShowMessage("    mn:     minute\n");
			ShowMessage("    s:      seconde\n");
			ShowMessage("  <exemple> timeadd nomtest +1m-2mn1s-6y\n");
			ShowMessage("            Cette exemple ajoute 1 mois et 1 seconde, et soustrait 2 minutes\n");
			ShowMessage("            et 6 ans dans le même temps.\n");
			ladmin_log("Aucun ajustement n'est pas un ajustement (commande 'timeadd').\n");
		} else {
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
		}
		return 137;
	}
	if (year > 127 || year < -127) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement d'années correct (de -127 à 127), svp.\n");
			ladmin_log("Ajustement de l'année hors norme ('timeadd' command).\n");
		} else {
			ShowMessage("Please give a correct adjustment for the years (from -127 to 127).\n");
			ladmin_log("Abnormal adjustment for the year ('timeadd' command).\n");
		}
		return 137;
	}
	if (month > 255 || month < -255) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de mois correct (de -255 à 255), svp.\n");
			ladmin_log("Ajustement du mois hors norme ('timeadd' command).\n");
		} else {
			ShowMessage("Please give a correct adjustment for the months (from -255 to 255).\n");
			ladmin_log("Abnormal adjustment for the month ('timeadd' command).\n");
		}
		return 137;
	}
	if (day > 32767 || day < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de jours correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des jours hors norme ('timeadd' command).\n");
		} else {
			ShowMessage("Please give a correct adjustment for the days (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the days ('timeadd' command).\n");
		}
		return 137;
	}
	if (hour > 32767 || hour < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement d'heures correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des heures hors norme ('timeadd' command).\n");
		} else {
			ShowMessage("Please give a correct adjustment for the hours (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the hours ('timeadd' command).\n");
		}
		return 137;
	}
	if (minute > 32767 || minute < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de minutes correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des minutes hors norme ('timeadd' command).\n");
		} else {
			ShowMessage("Please give a correct adjustment for the minutes (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the minutes ('timeadd' command).\n");
		}
		return 137;
	}
	if (second > 32767 || second < -32767) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un ajustement de secondes correct (de -32767 à 32767), svp.\n");
			ladmin_log("Ajustement des secondes hors norme ('timeadd' command).\n");
		} else {
			ShowMessage("Please give a correct adjustment for the seconds (from -32767 to 32767).\n");
			ladmin_log("Abnormal adjustment for the seconds ('timeadd' command).\n");
		}
		return 137;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour modifier une date limite d'utilisation.\n");
	} else {
		ladmin_log("Request to login-server to modify a time limit.\n");
	}

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
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
	struct tm *tmtime;
	WFIFOHEAD(login_fd,30);

	memset(name, '\0', sizeof(name));
	memset(date, '\0', sizeof(date));
	memset(time, '\0', sizeof(time));
	year = month = day = hour = minute = second = 0;
	connect_until_time = 0;
	tmtime = localtime(&connect_until_time); // initialize

	if (sscanf(param, "\"%[^\"]\" %s %[^\r\n]", name, date, time) < 2 && // if date = 0, time can be void
	    sscanf(param, "'%[^']' %s %[^\r\n]", name, date, time) < 2 && // if date = 0, time can be void
	    sscanf(param, "%s %s %[^\r\n]", name, date, time) < 2) { // if date = 0, time can be void
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte, une date et une heure svp.\n");
			ShowMessage("<exemple>: timeset <nom_du_compte> aaaa/mm/jj [hh:mm:ss]\n");
			ShowMessage("           timeset <nom_du_compte> 0    (0 = illimité)\n");
			ShowMessage("           Heure par défaut [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Nombre incorrect de paramètres pour fixer une date limite d'utilisation (commande 'timeset').\n");
		} else {
			ShowMessage("Please input an account name, a date and a hour.\n");
			ShowMessage("<example>: timeset <account_name> yyyy/mm/dd [hh:mm:ss]\n");
			ShowMessage("           timeset <account_name> 0   (0 = unlimited)\n");
			ShowMessage("           Default time [hh:mm:ss]: 23:59:59.\n");
			ladmin_log("Incomplete parameters to set a limit time ('timeset' command).\n");
		}
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
	     sscanf(time, "%d:%d:%d", &hour, &minute, &second) < 3)) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez 0 ou une date et une heure svp (format: 0 ou aaaa/mm/jj hh:mm:ss).\n");
			ladmin_log("Format incorrect pour la date/heure ('timeset' command).\n");
		} else {
			ShowMessage("Please input 0 or a date and a time (format: 0 or yyyy/mm/dd hh:mm:ss).\n");
			ladmin_log("Invalid format for the date/time ('timeset' command).\n");
		}
		return 102;
	}

	if (atoi(date) == 0) {
		connect_until_time = 0;
	} else {
		if (year < 70) {
			year = year + 100;
		}
		if (year >= 1900) {
			year = year - 1900;
		}
		if (month < 1 || month > 12) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un mois correct svp (entre 1 et 12).\n");
				ladmin_log("Mois incorrect pour la date ('timeset' command).\n");
			} else {
				ShowMessage("Please give a correct value for the month (from 1 to 12).\n");
				ladmin_log("Invalid month for the date ('timeset' command).\n");
			}
			return 102;
		}
		month = month - 1;
		if (day < 1 || day > 31) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un jour correct svp (entre 1 et 31).\n");
				ladmin_log("Jour incorrect pour la date ('timeset' command).\n");
			} else {
				ShowMessage("Please give a correct value for the day (from 1 to 31).\n");
				ladmin_log("Invalid day for the date ('timeset' command).\n");
			}
			return 102;
		}
		if (((month == 3 || month == 5 || month == 8 || month == 10) && day > 30) ||
		    (month == 1 && day > 29)) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez un jour correct en fonction du mois (%d) svp.\n", month);
				ladmin_log("Jour incorrect pour ce mois correspondant ('timeset' command).\n");
			} else {
				ShowMessage("Please give a correct value for a day of this month (%d).\n", month);
				ladmin_log("Invalid day for this month ('timeset' command).\n");
			}
			return 102;
		}
		if (hour < 0 || hour > 23) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez une heure correcte svp (entre 0 et 23).\n");
				ladmin_log("Heure incorrecte pour l'heure ('timeset' command).\n");
			} else {
				ShowMessage("Please give a correct value for the hour (from 0 to 23).\n");
				ladmin_log("Invalid hour for the time ('timeset' command).\n");
			}
			return 102;
		}
		if (minute < 0 || minute > 59) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez des minutes correctes svp (entre 0 et 59).\n");
				ladmin_log("Minute incorrecte pour l'heure ('timeset' command).\n");
			} else {
				ShowMessage("Please give a correct value for the minutes (from 0 to 59).\n");
				ladmin_log("Invalid minute for the time ('timeset' command).\n");
			}
			return 102;
		}
		if (second < 0 || second > 59) {
			if (defaultlanguage == 'F') {
				ShowMessage("Entrez des secondes correctes svp (entre 0 et 59).\n");
				ladmin_log("Seconde incorrecte pour l'heure ('timeset' command).\n");
			} else {
				ShowMessage("Please give a correct value for the seconds (from 0 to 59).\n");
				ladmin_log("Invalid second for the time ('timeset' command).\n");
			}
			return 102;
		}
		tmtime->tm_year = year;
		tmtime->tm_mon = month;
		tmtime->tm_mday = day;
		tmtime->tm_hour = hour;
		tmtime->tm_min = minute;
		tmtime->tm_sec = second;
		tmtime->tm_isdst = -1; // -1: no winter/summer time modification
		connect_until_time = mktime(tmtime);
		if (connect_until_time == -1) {
			if (defaultlanguage == 'F') {
				ShowMessage("Date incorrecte.\n");
				ShowMessage("Ajoutez 0 ou une date et une heure svp (format: 0 ou aaaa/mm/jj hh:mm:ss).\n");
				ladmin_log("Date incorrecte. ('timeset' command).\n");
			} else {
				ShowMessage("Invalid date.\n");
				ShowMessage("Please add 0 or a date and a time (format: 0 or yyyy/mm/dd hh:mm:ss).\n");
				ladmin_log("Invalid date. ('timeset' command).\n");
			}
			return 102;
		}
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour fixer une date limite d'utilisation.\n");
	} else {
		ladmin_log("Request to login-server to set a time limit.\n");
	}

	WFIFOW(login_fd,0) = 0x7948;
	memcpy(WFIFOP(login_fd,2), name, 24);
	WFIFOL(login_fd,26) = (int)connect_until_time;
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
	     strlen(name) == 0) {
		if (defaultlanguage == 'F') {
			ShowMessage("Entrez un nom de compte svp.\n");
			ShowMessage("<exemple> who nomtest\n");
			ladmin_log("Aucun nom n'a été donné pour trouver le compte.\n");
		} else {
			ShowMessage("Please input an account name.\n");
			ShowMessage("<example> who testname\n");
			ladmin_log("No name was given to found the account.\n");
		}
		return 136;
	}
	if (verify_accountname(name) == 0) {
		return 102;
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Envoi d'un requête au serveur de logins pour obtenir le information d'un compte (par le nom).\n");
	} else {
		ladmin_log("Request to login-server to obtain information about an account (by its name).\n");
	}

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
	if (defaultlanguage == 'F')
		ladmin_log("Envoi d'un requête au serveur de logins pour obtenir sa version.\n");
	else
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
		if (defaultlanguage == 'F')
			ShowMessage("\033[32mPour afficher les commandes, tapez 'Entrée'.\033[0m\n");
		else
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
			if (defaultlanguage == 'F') {
				strcpy(buf, "aide");
				strcpy(command, "aide");
			} else {
				strcpy(buf, "help");
				strcpy(command, "help");
			}
		}

		// Analyse of the command
		check_command(command); // give complete name to the command

		if (strlen(parameters) == 0) {
			if (defaultlanguage == 'F') {
				ladmin_log("Commande: '%s' (sans paramètre)\n", command, parameters);
			} else {
				ladmin_log("Command: '%s' (without parameters)\n", command, parameters);
			}
		} else {
			if (defaultlanguage == 'F') {
				ladmin_log("Commande: '%s', paramètres: '%s'\n", command, parameters);
			} else {
				ladmin_log("Command: '%s', parameters: '%s'\n", command, parameters);
			}
		}

		// Analyse of the command
// help
		if (strcmp(command, "aide") == 0) {
			display_help(parameters, 1); // 1: french
		} else if (strcmp(command, "help") == 0 ) {
			display_help(parameters, 0); // 0: english
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
		} else if (strcmp(command, "language") == 0) {
			changelanguage(parameters);
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
			if (defaultlanguage == 'F') {
				ShowMessage("Au revoir.\n");
			} else {
				ShowMessage("Bye.\n");
			}
			exit(EXIT_SUCCESS);
// unknown command
		} else {
			if (defaultlanguage == 'F') {
				ShowMessage("Commande inconnue [%s].\n", buf);
				ladmin_log("Commande inconnue [%s].\n", buf);
			} else {
				ShowMessage("Unknown command [%s].\n", buf);
				ladmin_log("Unknown command [%s].\n", buf);
			}
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
	if (session[fd]->eof) {
		if (defaultlanguage == 'F') {
			ShowMessage("Impossible de se connecter au serveur de login [%s:%d] !\n", loginserverip, loginserverport);
			ladmin_log("Impossible de se connecter au serveur de login [%s:%d] !\n", loginserverip, loginserverport);
		} else {
			ShowMessage("Impossible to have a connection with the login-server [%s:%d] !\n", loginserverip, loginserverport);
			ladmin_log("Impossible to have a connection with the login-server [%s:%d] !\n", loginserverip, loginserverport);
		}
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
				if (defaultlanguage == 'F') {
					ShowMessage("Erreur de login:\n");
					ShowMessage(" - mot de passe incorrect,\n");
					ShowMessage(" - système d'administration non activé, ou\n");
					ShowMessage(" - IP non autorisée.\n");
					ladmin_log("Erreur de login: mot de passe incorrect, système d'administration non activé, ou IP non autorisée.\n");
				} else {
					ShowMessage("Error at login:\n");
					ShowMessage(" - incorrect password,\n");
					ShowMessage(" - administration system not activated, or\n");
					ShowMessage(" - unauthorised IP.\n");
					ladmin_log("Error at login: incorrect password, administration system not activated, or unauthorised IP.\n");
				}
				set_eof(fd);
				//bytes_to_read = 1; // not stop at prompt
				return 0;
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Connexion établie.\n");
					ladmin_log("Connexion établie.\n");
					ShowMessage("Lecture de la version du serveur de login...\n");
					ladmin_log("Lecture de la version du serveur de login...\n");
				} else {
					ShowMessage("Established connection.\n");
					ladmin_log("Established connection.\n");
					ShowMessage("Reading of the version of the login-server...\n");
					ladmin_log("Reading of the version of the login-server...\n");
				}
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
			if (defaultlanguage == 'F') {
				ShowMessage("Réception de la clef MD5.\n");
				ladmin_log("Réception de la clef MD5.\n");
				ShowMessage("Envoi du mot de passe crypté...\n");
				ladmin_log("Envoi du mot de passe crypté...\n");
			} else {
				ShowMessage("Receiving of the MD5 key.\n");
				ladmin_log("Receiving of the MD5 key.\n");
				ShowMessage("Sending of the encrypted password...\n");
				ladmin_log("Sending of the encrypted password...\n");
			}
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
				if (defaultlanguage == 'F') {
					ladmin_log("  Réception d'une liste des comptes vide.\n");
					if (list_count == 0)
						ShowMessage("Aucun compte trouvé.\n");
					else if (list_count == 1)
						ShowMessage("1 compte trouvé.\n");
					else
						ShowMessage("%d comptes trouvés.\n", list_count);
				} else {
					ladmin_log("  Receiving of a void accounts list.\n");
					if (list_count == 0)
						ShowMessage("No account found.\n");
					else if (list_count == 1)
						ShowMessage("1 account found.\n");
					else
						ShowMessage("%d accounts found.\n", list_count);
				}
				bytes_to_read = 0;
			} else {
				int i;
				WFIFOHEAD(login_fd,10);
				if (defaultlanguage == 'F')
					ladmin_log("  Réception d'une liste des comptes.\n");
				else
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
						if (defaultlanguage == 'F') {
							if (RFIFOB(fd,i+29) == 0)
								ShowMessage("%-5s ", "Femme");
							else if (RFIFOB(fd,i+29) == 1)
								ShowMessage("%-5s ", "Male");
							else
								ShowMessage("%-5s ", "Servr");
						} else {
							if (RFIFOB(fd,i+29) == 0)
								ShowMessage("%-5s ", "Femal");
							else if (RFIFOB(fd,i+29) == 1)
								ShowMessage("%-5s ", "Male");
							else
								ShowMessage("%-5s ", "Servr");
						}
						ShowMessage("%6d ", (int)RFIFOL(fd,i+30));
						switch(RFIFOL(fd,i+34)) {
						case 0:
							if (defaultlanguage == 'F')
								ShowMessage("%-27s\n", "Compte Ok");
							else
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
				if (defaultlanguage == 'F')
					ladmin_log("Envoi d'un requête au serveur de logins pour obtenir la liste des comptes de %d à %d (complément).\n", list_first, list_last);
				else
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
			id=RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec à la création du compte [%s]. Un compte identique existe déjà.\n", RFIFOP(fd,6));
					ladmin_log("Echec à la création du compte [%s]. Un compte identique existe déjà.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] creation failed. Same account already exists.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] creation failed. Same account already exists.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Compte [%s] créé avec succès [id: %d].\n", RFIFOP(fd,6), id);
					ladmin_log("Compte [%s] créé avec succès [id: %d].\n", RFIFOP(fd,6), id);
				} else {
					ShowMessage("Account [%s] is successfully created [id: %d].\n", RFIFOP(fd,6), id);
					ladmin_log("Account [%s] is successfully created [id: %d].\n", RFIFOP(fd,6), id);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7933:	// Answer of login-server about an account deletion
			if (RFIFOREST(fd) < 30)
				return 0;
			if (RFIFOL(fd,2) == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec de la suppression du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec de la suppression du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] deletion failed. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] deletion failed. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Compte [%s][id: %d] SUPPRIME avec succès.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
					ladmin_log("Compte [%s][id: %d] SUPPRIME avec succès.\n", RFIFOP(fd,6), RFIFOL(fd,2));
				} else {
					ShowMessage("Account [%s][id: %d] is successfully DELETED.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
					ladmin_log("Account [%s][id: %d] is successfully DELETED.\n", RFIFOP(fd,6), RFIFOL(fd,2));
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7935:	// answer of the change of an account password
			if (RFIFOREST(fd) < 30)
				return 0;
			if (RFIFOL(fd,2) == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec de la modification du mot de passe du compte [%s].\n", RFIFOP(fd,6));
					ShowMessage("Le compte [%s] n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec de la modification du mot de passe du compte. Le compte [%s] n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] password changing failed.\n", RFIFOP(fd,6));
					ShowMessage("Account [%s] doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account password changing failed. The compte [%s] doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Modification du mot de passe du compte [%s][id: %d] réussie.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
					ladmin_log("Modification du mot de passe du compte [%s][id: %d] réussie.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
				} else {
					ShowMessage("Account [%s][id: %d] password successfully changed.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
					ladmin_log("Account [%s][id: %d] password successfully changed.\n", RFIFOP(fd,6), (int)RFIFOL(fd,2));
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7937:	// answer of the change of an account state
			if (RFIFOREST(fd) < 34)
				return 0;
			if (RFIFOL(fd,2) == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec du changement du statut du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec du changement du statut du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] state changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] state changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				char tmpstr[256];
				if (defaultlanguage == 'F') {
					sprintf(tmpstr, "Statut du compte [%s] changé avec succès en [", RFIFOP(fd,6));
				} else {
					sprintf(tmpstr, "Account [%s] state successfully changed in [", RFIFOP(fd,6));
				}
				switch(RFIFOL(fd,30)) {
				case 0:
					if (defaultlanguage == 'F')
						strcat(tmpstr, "0: Compte Ok");
					else
						strcat(tmpstr, "0: Account OK");
					break;
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
			if (defaultlanguage == 'F') {
				ladmin_log("  Réception du nombre de joueurs en ligne.\n");
			} else {
				ladmin_log("  Receiving of the number of online players.\n");
			}
			// Read information of the servers
			if (RFIFOW(fd,2) < 5) {
				if (defaultlanguage == 'F') {
					ShowMessage("  Aucun serveur n'est connecté au login serveur.\n");
				} else {
					ShowMessage("  No server is connected to the login-server.\n");
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("  Nombre de joueurs en ligne (serveur: nb):\n");
				} else {
					ShowMessage("  Number of online players (server: number).\n");
				}
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
				if (defaultlanguage == 'F') {
					ShowMessage("Le compte [%s] n'existe pas ou le mot de passe est incorrect.\n", RFIFOP(fd,6));
					ladmin_log("Le compte [%s] n'existe pas ou le mot de passe est incorrect.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("The account [%s] doesn't exist or the password is incorrect.\n", RFIFOP(fd,6));
					ladmin_log("The account [%s] doesn't exist or the password is incorrect.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Le mot de passe donné correspond bien au compte [%s][id: %d].\n", RFIFOP(fd,6), id);
					ladmin_log("Le mot de passe donné correspond bien au compte [%s][id: %d].\n", RFIFOP(fd,6), id);
				} else {
					ShowMessage("The proposed password is correct for the account [%s][id: %d].\n", RFIFOP(fd,6), id);
					ladmin_log("The proposed password is correct for the account [%s][id: %d].\n", RFIFOP(fd,6), id);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x793d:	// answer of the change of an account sex
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec de la modification du sexe du compte [%s].\n", RFIFOP(fd,6));
					ShowMessage("Le compte [%s] n'existe pas ou le sexe est déjà celui demandé.\n", RFIFOP(fd,6));
					ladmin_log("Echec de la modification du sexe du compte. Le compte [%s] n'existe pas ou le sexe est déjà celui demandé.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] sex changing failed.\n", RFIFOP(fd,6));
					ShowMessage("Account [%s] doesn't exist or the sex is already the good sex.\n", RFIFOP(fd,6));
					ladmin_log("Account sex changing failed. The compte [%s] doesn't exist or the sex is already the good sex.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Sexe du compte [%s][id: %d] changé avec succès.\n", RFIFOP(fd,6), id);
					ladmin_log("Sexe du compte [%s][id: %d] changé avec succès.\n", RFIFOP(fd,6), id);
				} else {
					ShowMessage("Account [%s][id: %d] sex successfully changed.\n", RFIFOP(fd,6), id);
					ladmin_log("Account [%s][id: %d] sex successfully changed.\n", RFIFOP(fd,6), id);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x793f:	// answer of the change of an account GM level
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec de la modification du niveau de GM du compte [%s].\n", RFIFOP(fd,6));
					ShowMessage("Le compte [%s] n'existe pas, le niveau de GM est déjà celui demandé\n", RFIFOP(fd,6));
					ShowMessage("ou il est impossible de modifier le fichier des comptes GM.\n");
					ladmin_log("Echec de la modification du niveau de GM du compte. Le compte [%s] n'existe pas, le niveau de GM est déjà celui demandé ou il est impossible de modifier le fichier des comptes GM.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] GM level changing failed.\n", RFIFOP(fd,6));
					ShowMessage("Account [%s] doesn't exist, the GM level is already the good GM level\n", RFIFOP(fd,6));
					ShowMessage("or it's impossible to modify the GM accounts file.\n");
					ladmin_log("Account GM level changing failed. The compte [%s] doesn't exist, the GM level is already the good sex or it's impossible to modify the GM accounts file.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Niveau de GM du compte [%s][id: %d] changé avec succès.\n", RFIFOP(fd,6), id);
					ladmin_log("Niveau de GM du compte [%s][id: %d] changé avec succès.\n", RFIFOP(fd,6), id);
				} else {
					ShowMessage("Account [%s][id: %d] GM level successfully changed.\n", RFIFOP(fd,6), id);
					ladmin_log("Account [%s][id: %d] GM level successfully changed.\n", RFIFOP(fd,6), id);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7941:	// answer of the change of an account email
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec de la modification de l'e-mail du compte [%s].\n", RFIFOP(fd,6));
					ShowMessage("Le compte [%s] n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec de la modification de l'e-mail du compte. Le compte [%s] n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] e-mail changing failed.\n", RFIFOP(fd,6));
					ShowMessage("Account [%s] doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account e-mail changing failed. The compte [%s] doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Modification de l'e-mail du compte [%s][id: %d] réussie.\n", RFIFOP(fd,6), id);
					ladmin_log("Modification de l'e-mail du compte [%s][id: %d] réussie.\n", RFIFOP(fd,6), id);
				} else {
					ShowMessage("Account [%s][id: %d] e-mail successfully changed.\n", RFIFOP(fd,6), id);
					ladmin_log("Account [%s][id: %d] e-mail successfully changed.\n", RFIFOP(fd,6), id);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7943:	// answer of the change of an account memo
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec du changement du mémo du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec du changement du mémo du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] memo changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] memo changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Mémo du compte [%s][id: %d] changé avec succès.\n", RFIFOP(fd,6), id);
					ladmin_log("Mémo du compte [%s][id: %d] changé avec succès.\n", RFIFOP(fd,6), id);
				} else {
					ShowMessage("Account [%s][id: %d] memo successfully changed.\n", RFIFOP(fd,6), id);
					ladmin_log("Account [%s][id: %d] memo successfully changed.\n", RFIFOP(fd,6), id);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7945:	// answer of an account id search
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Impossible de trouver l'id du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Impossible de trouver l'id du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Unable to find the account [%s] id. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Unable to find the account [%s] id. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Le compte [%s] a pour id: %d.\n", RFIFOP(fd,6), id);
					ladmin_log("Le compte [%s] a pour id: %d.\n", RFIFOP(fd,6), id);
				} else {
					ShowMessage("The account [%s] have the id: %d.\n", RFIFOP(fd,6), id);
					ladmin_log("The account [%s] have the id: %d.\n", RFIFOP(fd,6), id);
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7947:	// answer of an account name search
			if (RFIFOREST(fd) < 30)
				return 0;
			id = RFIFOL(fd,2);
			if (strcmp((const char*)RFIFOP(fd,6), "") == 0) {
				if (defaultlanguage == 'F') {
					ShowMessage("Impossible de trouver le nom du compte [%d]. Le compte n'existe pas.\n", id);
					ladmin_log("Impossible de trouver le nom du compte [%d]. Le compte n'existe pas.\n", id);
				} else {
					ShowMessage("Unable to find the account [%d] name. Account doesn't exist.\n", id);
					ladmin_log("Unable to find the account [%d] name. Account doesn't exist.\n", id);
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Le compte [id: %d] a pour nom: %s.\n", id, RFIFOP(fd,6));
					ladmin_log("Le compte [id: %d] a pour nom: %s.\n", id, RFIFOP(fd,6));
				} else {
					ShowMessage("The account [id: %d] have the name: %s.\n", id, RFIFOP(fd,6));
					ladmin_log("The account [id: %d] have the name: %s.\n", id, RFIFOP(fd,6));
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,30);
			break;

		case 0x7949:	// answer of an account validity limit set
			if (RFIFOREST(fd) < 34)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec du changement de la validité du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec du changement de la validité du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					if (defaultlanguage == 'F') {
						ShowMessage("Limite de validité du compte [%s][id: %d] changée avec succès en [illimité].\n", RFIFOP(fd,6), id);
						ladmin_log("Limite de validité du compte [%s][id: %d] changée avec succès en [illimité].\n", RFIFOP(fd,6), id);
					} else {
						ShowMessage("Validity Limit of the account [%s][id: %d] successfully changed to [unlimited].\n", RFIFOP(fd,6), id);
						ladmin_log("Validity Limit of the account [%s][id: %d] successfully changed to [unlimited].\n", RFIFOP(fd,6), id);
					}
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					if (defaultlanguage == 'F') {
						ShowMessage("Limite de validité du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Limite de validité du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
					} else {
						ShowMessage("Validity Limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Validity Limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					}
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
				if (defaultlanguage == 'F') {
					ShowMessage("Echec du changement de la date finale de banissement du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec du changement de la date finale de banissement du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					if (defaultlanguage == 'F') {
						ShowMessage("Date finale de banissement du compte [%s][id: %d] changée avec succès en [dé-bannie].\n", RFIFOP(fd,6), id);
						ladmin_log("Date finale de banissement du compte [%s][id: %d] changée avec succès en [dé-bannie].\n", RFIFOP(fd,6), id);
					} else {
						ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
						ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
					}
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					if (defaultlanguage == 'F') {
						ShowMessage("Date finale de banissement du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Date finale de banissement du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
					} else {
						ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					}
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
				if (defaultlanguage == 'F') {
					ShowMessage("Echec du changement de la date finale de banissement du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec du changement de la date finale de banissement du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] final date of banishment changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					if (defaultlanguage == 'F') {
						ShowMessage("Date finale de banissement du compte [%s][id: %d] changée avec succès en [dé-bannie].\n", RFIFOP(fd,6), id);
						ladmin_log("Date finale de banissement du compte [%s][id: %d] changée avec succès en [dé-bannie].\n", RFIFOP(fd,6), id);
					} else {
						ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
						ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to [unbanished].\n", RFIFOP(fd,6), id);
					}
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					if (defaultlanguage == 'F') {
						ShowMessage("Date finale de banissement du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Date finale de banissement du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
					} else {
						ShowMessage("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Final date of banishment of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					}
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,34);
			break;

		case 0x794f:	// answer of a broadcast
			if (RFIFOREST(fd) < 4)
				return 0;
			if (RFIFOW(fd,2) == (unsigned short)-1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec de l'envoi du message. Aucun server de char en ligne.\n");
					ladmin_log("Echec de l'envoi du message. Aucun server de char en ligne.\n");
				} else {
					ShowMessage("Message sending failed. No online char-server.\n");
					ladmin_log("Message sending failed. No online char-server.\n");
				}
			} else {
				if (defaultlanguage == 'F') {
					ShowMessage("Message transmis au server de logins avec succès.\n");
					ladmin_log("Message transmis au server de logins avec succès.\n");
				} else {
					ShowMessage("Message successfully sended to login-server.\n");
					ladmin_log("Message successfully sended to login-server.\n");
				}
			}
			bytes_to_read = 0;
			RFIFOSKIP(fd,4);
			break;

		case 0x7951:	// answer of an account validity limit changing
			if (RFIFOREST(fd) < 34)
				return 0;
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Echec du changement de la validité du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
					ladmin_log("Echec du changement de la validité du compte [%s]. Le compte n'existe pas.\n", RFIFOP(fd,6));
				} else {
					ShowMessage("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
					ladmin_log("Account [%s] validity limit changing failed. Account doesn't exist.\n", RFIFOP(fd,6));
				}
			} else {
				time_t timestamp = RFIFOL(fd,30);
				if (timestamp == 0) {
					if (defaultlanguage == 'F') {
						ShowMessage("Limite de validité du compte [%s][id: %d] inchangée.\n", RFIFOP(fd,6), id);
						ShowMessage("Le compte a une validité illimitée ou\n");
						ShowMessage("la modification est impossible avec les ajustements demandés.\n");
						ladmin_log("Limite de validité du compte [%s][id: %d] inchangée. Le compte a une validité illimitée ou la modification est impossible avec les ajustements demandés.\n", RFIFOP(fd,6), id);
					} else {
						ShowMessage("Validity limit of the account [%s][id: %d] unchanged.\n", RFIFOP(fd,6), id);
						ShowMessage("The account have an unlimited validity limit or\n");
						ShowMessage("the changing is impossible with the proposed adjustments.\n");
						ladmin_log("Validity limit of the account [%s][id: %d] unchanged. The account have an unlimited validity limit or the changing is impossible with the proposed adjustments.\n", RFIFOP(fd,6), id);
					}
				} else {
					char tmpstr[128];
					strftime(tmpstr, 24, date_format, localtime(&timestamp));
					if (defaultlanguage == 'F') {
						ShowMessage("Limite de validité du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Limite de validité du compte [%s][id: %d] changée avec succès pour être jusqu'au %s.\n", RFIFOP(fd,6), id, tmpstr);
					} else {
						ShowMessage("Validity limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
						ladmin_log("Validity limit of the account [%s][id: %d] successfully changed to be until %s.\n", RFIFOP(fd,6), id, tmpstr);
					}
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
			time_t ban_until_time; // # of seconds 1/1/1970 (timestamp): ban time limit of the account (0 = no ban)
			time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
			memcpy(userid, RFIFOP(fd,7), sizeof(userid));
			userid[sizeof(userid)-1] = '\0';
			memcpy(error_message, RFIFOP(fd,40), sizeof(error_message));
			error_message[sizeof(error_message)-1] = '\0';
			memcpy(lastlogin, RFIFOP(fd,60), sizeof(lastlogin));
			lastlogin[sizeof(lastlogin)-1] = '\0';
			memcpy(last_ip, RFIFOP(fd,84), sizeof(last_ip));
			last_ip[sizeof(last_ip)-1] = '\0';
			memcpy(email, RFIFOP(fd,100), sizeof(email));
			email[sizeof(email)-1] = '\0';
			connect_until_time = (time_t)RFIFOL(fd,140);
			ban_until_time = (time_t)RFIFOL(fd,144);
			memset(memo, '\0', sizeof(memo));
			strncpy(memo, (const char*)RFIFOP(fd,150), RFIFOW(fd,148));
			id = RFIFOL(fd,2);
			if (id == -1) {
				if (defaultlanguage == 'F') {
					ShowMessage("Impossible de trouver le compte [%s]. Le compte n'existe pas.\n", parameters);
					ladmin_log("Impossible de trouver le compte [%s]. Le compte n'existe pas.\n", parameters);
				} else {
					ShowMessage("Unabled to find the account [%s]. Account doesn't exist.\n", parameters);
					ladmin_log("Unabled to find the account [%s]. Account doesn't exist.\n", parameters);
				}
			} else if (strlen(userid) == 0) {
				if (defaultlanguage == 'F') {
					ShowMessage("Impossible de trouver le compte [id: %s]. Le compte n'existe pas.\n", parameters);
					ladmin_log("Impossible de trouver le compte [id: %s]. Le compte n'existe pas.\n", parameters);
				} else {
					ShowMessage("Unabled to find the account [id: %s]. Account doesn't exist.\n", parameters);
					ladmin_log("Unabled to find the account [id: %s]. Account doesn't exist.\n", parameters);
				}
			} else {
				if (defaultlanguage == 'F') {
					ladmin_log("Réception d'information concernant un compte.\n");
					ShowMessage("Le compte a les caractéristiques suivantes:\n");
				} else {
					ladmin_log("Receiving information about an account.\n");
					ShowMessage("The account is set with:\n");
				}
				if (RFIFOB(fd,6) == 0) {
					ShowMessage(" Id:     %d (non-GM)\n", id);
				} else {
					if (defaultlanguage == 'F') {
						ShowMessage(" Id:     %d (GM niveau %d)\n", id, (int)RFIFOB(fd,6));
					} else {
						ShowMessage(" Id:     %d (GM level %d)\n", id, (int)RFIFOB(fd,6));
					}
				}
				if (defaultlanguage == 'F') {
					ShowMessage(" Nom:    '%s'\n", userid);
					if (RFIFOB(fd,31) == 0)
						ShowMessage(" Sexe:   Femme\n");
					else if (RFIFOB(fd,31) == 1)
						ShowMessage(" Sexe:   Male\n");
					else
						ShowMessage(" Sexe:   Serveur\n");
				} else {
					ShowMessage(" Name:   '%s'\n", userid);
					if (RFIFOB(fd,31) == 0)
						ShowMessage(" Sex:    Female\n");
					else if (RFIFOB(fd,31) == 1)
						ShowMessage(" Sex:    Male\n");
					else
						ShowMessage(" Sex:    Server\n");
				}
				ShowMessage(" E-mail: %s\n", email);
				switch(RFIFOL(fd,36)) {
				case 0:
					if (defaultlanguage == 'F')
						ShowMessage(" Statut: 0 [Compte Ok]\n");
					else
						ShowMessage(" State: 0 [Account OK]\n");
					break;
				case 1:
					ShowMessage(" Statut: 1 [Unregistered ID]\n");
					break;
				case 2:
					ShowMessage(" State: 2 [Incorrect Password]\n");
					break;
				case 3:
					ShowMessage(" Statut: 3 [This ID is expired]\n");
					break;
				case 4:
					ShowMessage(" State: 4 [Rejected from Server]\n");
					break;
				case 5:
					ShowMessage(" Statut: 5 [You have been blocked by the GM Team]\n");
					break;
				case 6:
					ShowMessage(" State: 6 [Your Game's EXE file is not the latest version]\n");
					break;
				case 7:
					ShowMessage(" Statut: 7 [You are Prohibited to log in until %s]\n", error_message);
					break;
				case 8:
					ShowMessage(" State: 8 [Server is jammed due to over populated]\n");
					break;
				case 9:
					ShowMessage(" Statut: 9 [No MSG]\n");
					break;
				default: // 100
					ShowMessage(" State: %d [This ID is totally erased]\n", (int)RFIFOL(fd,36));
					break;
				}
				if (defaultlanguage == 'F') {
					if (ban_until_time == 0) {
						ShowMessage(" Banissement: non banni.\n");
					} else {
						char tmpstr[128];
						strftime(tmpstr, 24, date_format, localtime(&ban_until_time));
						ShowMessage(" Banissement: jusqu'au %s.\n", tmpstr);
					}
					if (RFIFOL(fd,32) > 1)
						ShowMessage(" Compteur: %d connexions.\n", (int)RFIFOL(fd,32));
					else
						ShowMessage(" Compteur: %d connexion.\n", (int)RFIFOL(fd,32));
					ShowMessage(" Dernière connexion le: %s (ip: %s)\n", lastlogin, last_ip);
					if (connect_until_time == 0) {
						ShowMessage(" Limite de validité: illimité.\n");
					} else {
						char tmpstr[128];
						strftime(tmpstr, 24, date_format, localtime(&connect_until_time));
						ShowMessage(" Limite de validité: jusqu'au %s.\n", tmpstr);
					}
				} else {
					if (ban_until_time == 0) {
						ShowMessage(" Banishment: not banished.\n");
					} else {
						char tmpstr[128];
						strftime(tmpstr, 24, date_format, localtime(&ban_until_time));
						ShowMessage(" Banishment: until %s.\n", tmpstr);
					}
					if (RFIFOL(fd,32) > 1)
						ShowMessage(" Count:  %d connections.\n", (int)RFIFOL(fd,32));
					else
						ShowMessage(" Count:  %d connection.\n", (int)RFIFOL(fd,32));
					ShowMessage(" Last connection at: %s (ip: %s)\n", lastlogin, last_ip);
					if (connect_until_time == 0) {
						ShowMessage(" Validity limit: unlimited.\n");
					} else {
						char tmpstr[128];
						strftime(tmpstr, 24, date_format, localtime(&connect_until_time));
						ShowMessage(" Validity limit: until %s.\n", tmpstr);
					}
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
	if (defaultlanguage == 'F') {
		ShowMessage("Essai de connection au server de logins...\n");
		ladmin_log("Essai de connection au server de logins...\n");
	} else {
		ShowMessage("Attempt to connect to login-server...\n");
		ladmin_log("Attempt to connect to login-server...\n");
	}

	login_fd = make_connection(login_ip, loginserverport);
	if (login_fd == -1)
	{	//Might not be the most elegant way to handle this, but I've never used ladmin so I dunno what else you could do. [Skotlex]
		ShowMessage("Error: Failed to connect to Login Server\n");
		exit(EXIT_FAILURE);
	}
	if (passenc == 0) {
		WFIFOHEAD(login_fd,28);
		WFIFOW(login_fd,0) = 0x7918; // Request for administation login
		WFIFOW(login_fd,2) = 0; // no encrypted
		memcpy(WFIFOP(login_fd,4), loginserveradminpassword, 24);
		WFIFOSET(login_fd,28);
		bytes_to_read = 1;
		if (defaultlanguage == 'F') {
			ShowMessage("Envoi du mot de passe...\n");
			ladmin_log("Envoi du mot de passe...\n");
		} else {
			ShowMessage("Sending of the password...\n");
			ladmin_log("Sending of the password...\n");
		}
	} else {
		WFIFOHEAD(login_fd,2);
		WFIFOW(login_fd,0) = 0x791a; // Sending request about the coding key
		WFIFOSET(login_fd,2);
		bytes_to_read = 1;
		if (defaultlanguage == 'F') {
			ShowMessage("Demande de la clef MD5...\n");
			ladmin_log("Demande de la clef MD5...\n");
		} else {
			ShowMessage("Request about the MD5 key...\n");
			ladmin_log("Request about the MD5 key...\n");
		}
	}

	return 0;
}

// sends a ping packet to login server (will receive pong 0x2718)
int ping_login_server(int tid, unsigned int tick, int id, int data)
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
		if (defaultlanguage == 'F') {
			ShowMessage("\033[0mFichier de configuration (%s) non trouvé.\n", cfgName);
		} else {
			ShowMessage("\033[0mConfiguration file (%s) not found.\n", cfgName);
		}
		return 1;
	}

	if (defaultlanguage == 'F') {
		ShowMessage("\033[0m---Début de lecture du fichier de configuration Ladmin (%s)\n", cfgName);
	} else {
		ShowMessage("\033[0m---Start reading of Ladmin configuration file (%s)\n", cfgName);
	}
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
					if (defaultlanguage == 'F') {
						ShowMessage("Adresse du serveur de logins: %s -> %s\n", w2, loginserverip);
					} else {
						ShowMessage("Login server IP address: %s -> %s\n", w2, loginserverip);
					}
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
			} else if (strcmpi(w1, "defaultlanguage") == 0) {
				if (w2[0] == 'F' || w2[0] == 'E')
					defaultlanguage = w2[0];
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

	if (defaultlanguage == 'F') {
		ShowMessage("---Lecture du fichier de configuration Ladmin terminée.\n");
	} else {
		ShowMessage("---End reading of Ladmin configuration file.\n");
	}

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

		if (defaultlanguage == 'F') {
			ShowMessage("\033[0m----Fin de Ladmin (fin normale avec fermeture de tous les fichiers).\n");
			ladmin_log("----Fin de Ladmin (fin normale avec fermeture de tous les fichiers).\n");
		} else {
			ShowMessage("\033[0m----End of Ladmin (normal end with closing of all files).\n");
			ladmin_log("----End of Ladmin (normal end with closing of all files).\n");
		}

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
	if (defaultlanguage == 'F') {
		ladmin_log("Fichier de configuration lu.\n");
	} else {
		ladmin_log("Configuration file readed.\n");
	}

	srand(time(NULL));

	set_defaultparse(parse_fromlogin);

	if (defaultlanguage == 'F') {
		ShowMessage("Outil d'administration à distance de eAthena.\n");
		ShowMessage("(pour eAthena version %d.%d.%d.)\n", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION);
	} else {
		ShowMessage("EAthena login-server administration tool.\n");
		ShowMessage("(for eAthena version %d.%d.%d.)\n", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION);
	}

	if (defaultlanguage == 'F') {
		ladmin_log("Ladmin est prêt.\n");
		ShowMessage("Ladmin est \033[1;32mprêt\033[0m.\n\n");
	} else {
		ladmin_log("Ladmin is ready.\n");
		ShowMessage("Ladmin is \033[1;32mready\033[0m.\n\n");
	}

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
