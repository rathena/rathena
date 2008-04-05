// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/core.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/md5calc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/timer.h"
#include "../common/version.h"
#include "login.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// temporary external imports
extern struct gm_account* gm_account_db;
int read_gm_account(void);
int mmo_auth_init(void);
int parse_login(int fd);
void login_log(uint32 ip, const char* username, int rcode, const char* message);

#ifdef TXT_ONLY
	extern struct mmo_account* auth_dat;
	void mmo_auth_sync(void);
	int check_GM_file(int tid, unsigned int tick, int id, int data);
	int check_auth_sync(int tid, unsigned int tick, int id, int data);
	void display_conf_warnings(void);
	extern bool admin_state;
	extern char admin_pass[24];
	extern uint32 admin_allowed_ip;
	extern char account_txt[1024];
	extern char GM_account_filename[1024];
	extern int gm_account_filename_check_timer;
	extern char login_log_filename[1024];
#else
	void mmo_db_close(void);
	void sql_config_read(const char* cfgName);
	int ip_ban_flush(int tid, unsigned int tick, int id, int data);
#endif


struct Login_Config login_config;

int login_fd; // login server socket
#define MAX_SERVERS 30
struct mmo_char_server server[MAX_SERVERS]; // char server data

#define sex_num2str(num) ( (num ==  0  ) ? 'F' : (num ==  1  ) ? 'M' : 'S' )
#define sex_str2num(str) ( (str == 'F' ) ?  0  : (str == 'M' ) ?  1  :  2  )


//Account registration flood protection [Kevin]
int allowed_regs = 1;
int time_allowed = 10; //in seconds

// Advanced subnet check [LuzZza]
struct s_subnet {
	uint32 mask;
	uint32 char_ip;
	uint32 map_ip;
} subnet[16];

int subnet_count = 0;


//-----------------------------------------------------
// Auth database
//-----------------------------------------------------
#define AUTH_TIMEOUT 30000

/*static*/ DBMap* auth_db; // int account_id -> struct auth_node*

//-----------------------------------------------------
// Online User Database [Wizputer]
//-----------------------------------------------------

/*static*/ DBMap* online_db; // int account_id -> struct online_login_data*
/*static*/ int waiting_disconnect_timer(int tid, unsigned int tick, int id, int data);

/*static*/ void* create_online_user(DBKey key, va_list args)
{
	struct online_login_data* p;
	CREATE(p, struct online_login_data, 1);
	p->account_id = key.i;
	p->char_server = -1;
	p->waiting_disconnect = -1;
	return p;
}

struct online_login_data* add_online_user(int char_server, int account_id)
{
	struct online_login_data* p;
	if( !login_config.online_check )
		return NULL;
	p = (struct online_login_data*)idb_ensure(online_db, account_id, create_online_user);
	p->char_server = char_server;
	if( p->waiting_disconnect != -1 )
	{
		delete_timer(p->waiting_disconnect, waiting_disconnect_timer);
		p->waiting_disconnect = -1;
	}
	return p;
}

void remove_online_user(int account_id)
{
	struct online_login_data* p;
	if( !login_config.online_check )
		return;
	p = (struct online_login_data*)idb_get(online_db, account_id);
	if( p == NULL )
		return;
	if( p->waiting_disconnect != -1 )
		delete_timer(p->waiting_disconnect, waiting_disconnect_timer);

	idb_remove(online_db, account_id);
}

/*static*/ int waiting_disconnect_timer(int tid, unsigned int tick, int id, int data)
{
	struct online_login_data* p = (struct online_login_data*)idb_get(online_db, id);
	if( p != NULL && p->waiting_disconnect == tid && p->account_id == id )
	{
		p->waiting_disconnect = -1;
		remove_online_user(id);
		idb_remove(auth_db, id);
	}
	return 0;
}

/*static*/ int online_db_setoffline(DBKey key, void* data, va_list ap)
{
	struct online_login_data* p = (struct online_login_data*)data;
	int server = va_arg(ap, int);
	if( server == -1 )
	{
		p->char_server = -1;
		if( p->waiting_disconnect != -1 )
		{
			delete_timer(p->waiting_disconnect, waiting_disconnect_timer);
			p->waiting_disconnect = -1;
		}
	}
	else if( p->char_server == server )
		p->char_server = -2; //Char server disconnected.
	return 0;
}

static int online_data_cleanup_sub(DBKey key, void *data, va_list ap)
{
	struct online_login_data *character= (struct online_login_data*)data;
	if (character->char_server == -2) //Unknown server.. set them offline
		remove_online_user(character->account_id);
	return 0;
}

static int online_data_cleanup(int tid, unsigned int tick, int id, int data)
{
	online_db->foreach(online_db, online_data_cleanup_sub);
	return 0;
} 


//--------------------------------------------------------------------
// Packet send to all char-servers, except one (wos: without our self)
//--------------------------------------------------------------------
int charif_sendallwos(int sfd, uint8* buf, size_t len)
{
	int i, c;

	for( i = 0, c = 0; i < MAX_SERVERS; ++i )
	{
		int fd = server[i].fd;
		if( session_isValid(fd) && fd != sfd )
		{
			WFIFOHEAD(fd,len);
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			++c;
		}
	}

	return c;
}


//-----------------------------------------------------
// periodic ip address synchronization
//-----------------------------------------------------
static int sync_ip_addresses(int tid, unsigned int tick, int id, int data)
{
	uint8 buf[2];
	ShowInfo("IP Sync in progress...\n");
	WBUFW(buf,0) = 0x2735;
	charif_sendallwos(-1, buf, 2);
	return 0;
}


//-----------------------------------------------------
// encrypted/unencrypted password check
//-----------------------------------------------------
bool check_encrypted(const char* str1, const char* str2, const char* passwd)
{
	char md5str[64], md5bin[32];

	snprintf(md5str, sizeof(md5str), "%s%s", str1, str2);
	md5str[sizeof(md5str)-1] = '\0';
	MD5_String2binary(md5str, md5bin);

	return (0==memcmp(passwd, md5bin, 16));
}

bool check_password(struct login_session_data* sd, int passwdenc, const char* passwd, const char* refpass)
{	
	if(passwdenc == 0)
	{
		return (0==strcmp(passwd, refpass));
	}
	else if(sd != NULL)
	{
		// password mode set to 1 -> (md5key, refpass) enable with <passwordencrypt></passwordencrypt>
		// password mode set to 2 -> (refpass, md5key) enable with <passwordencrypt2></passwordencrypt2>
		
		return ((passwdenc&0x01) && check_encrypted(sd->md5key, refpass, passwd)) ||
		       ((passwdenc&0x02) && check_encrypted(refpass, sd->md5key, passwd));
	}
	return false;
}


//--------------------------------------------
// Test to know if an IP come from LAN or WAN.
//--------------------------------------------
int lan_subnetcheck(uint32 ip)
{
	int i;
	ARR_FIND( 0, subnet_count, i, (subnet[i].char_ip & subnet[i].mask) == (ip & subnet[i].mask) );
	return ( i < subnet_count ) ? subnet[i].char_ip : 0;
}

//----------------------------------
// Reading Lan Support configuration
//----------------------------------
int login_lan_config_read(const char *lancfgName)
{
	FILE *fp;
	int line_num = 0;
	char line[1024], w1[64], w2[64], w3[64], w4[64];

	if((fp = fopen(lancfgName, "r")) == NULL) {
		ShowWarning("LAN Support configuration file is not found: %s\n", lancfgName);
		return 1;
	}

	ShowInfo("Reading the configuration file %s...\n", lancfgName);

	while(fgets(line, sizeof(line), fp))
	{
		line_num++;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\n' || line[1] == '\n')
			continue;

		if(sscanf(line,"%[^:]: %[^:]:%[^:]:%[^\r\n]", w1, w2, w3, w4) != 4)
		{
			ShowWarning("Error syntax of configuration file %s in line %d.\n", lancfgName, line_num);
			continue;
		}

		if( strcmpi(w1, "subnet") == 0 )
		{
			subnet[subnet_count].mask = str2ip(w2);
			subnet[subnet_count].char_ip = str2ip(w3);
			subnet[subnet_count].map_ip = str2ip(w4);

			if( (subnet[subnet_count].char_ip & subnet[subnet_count].mask) != (subnet[subnet_count].map_ip & subnet[subnet_count].mask) )
			{
				ShowError("%s: Configuration Error: The char server (%s) and map server (%s) belong to different subnetworks!\n", lancfgName, w3, w4);
				continue;
			}

			subnet_count++;
		}
	}

	ShowStatus("Read information about %d subnetworks.\n", subnet_count);

	fclose(fp);
	return 0;
}


//-----------------------
// Console Command Parser [Wizputer]
//-----------------------
int parse_console(char* buf)
{
	char command[256];

	memset(command, 0, sizeof(command));

	sscanf(buf, "%[^\n]", command);

	ShowInfo("Console command :%s", command);

	if( strcmpi("shutdown", command) == 0 ||
	    strcmpi("exit", command) == 0 ||
	    strcmpi("quit", command) == 0 ||
	    strcmpi("end", command) == 0 )
		runflag = 0;
	else
	if( strcmpi("alive", command) == 0 ||
	    strcmpi("status", command) == 0 )
		ShowInfo(CL_CYAN"Console: "CL_BOLD"I'm Alive."CL_RESET"\n");
	else
	if( strcmpi("help", command) == 0 ) {
		ShowInfo(CL_BOLD"Help of commands:"CL_RESET"\n");
		ShowInfo("  To shutdown the server:\n");
		ShowInfo("  'shutdown|exit|quit|end'\n");
		ShowInfo("  To know if server is alive:\n");
		ShowInfo("  'alive|status'\n");
	}

	return 0;
}


void login_set_defaults()
{
	login_config.login_ip = INADDR_ANY;
	login_config.login_port = 6900;
	login_config.ip_sync_interval = 0;
	login_config.log_login = true;
	safestrncpy(login_config.date_format, "%Y-%m-%d %H:%M:%S", sizeof(login_config.date_format));
	login_config.console = false;
	login_config.new_account_flag = true;
	login_config.case_sensitive = true;
	login_config.use_md5_passwds = false;
	login_config.login_gm_read = true;
	login_config.min_level_to_connect = 0;
	login_config.online_check = true;
	login_config.check_client_version = false;
	login_config.client_version_to_connect = 20;

	login_config.ipban = true;
	login_config.dynamic_pass_failure_ban = true;
	login_config.dynamic_pass_failure_ban_interval = 5;
	login_config.dynamic_pass_failure_ban_limit = 7;
	login_config.dynamic_pass_failure_ban_duration = 5;
	login_config.use_dnsbl = false;
	safestrncpy(login_config.dnsbl_servs, "", sizeof(login_config.dnsbl_servs));
}

//-----------------------------------
// Reading main configuration file
//-----------------------------------
int login_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp = fopen(cfgName, "r");
	if (fp == NULL) {
		ShowError("Configuration file (%s) not found.\n", cfgName);
		return 1;
	}
	ShowInfo("Reading configuration file %s...\n", cfgName);
	while(fgets(line, sizeof(line), fp))
	{
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) < 2)
			continue;

		if(!strcmpi(w1,"timestamp_format"))
			strncpy(timestamp_format, w2, 20);
		else if(!strcmpi(w1,"stdout_with_ansisequence"))
			stdout_with_ansisequence = config_switch(w2);
		else if(!strcmpi(w1,"console_silent")) {
			ShowInfo("Console Silent Setting: %d\n", atoi(w2));
			msg_silent = atoi(w2);
		}
		else if( !strcmpi(w1, "bind_ip") ) {
			char ip_str[16];
			login_config.login_ip = host2ip(w2);
			if( login_config.login_ip )
				ShowStatus("Login server binding IP address : %s -> %s\n", w2, ip2str(login_config.login_ip, ip_str));
		}
		else if( !strcmpi(w1, "login_port") ) {
			login_config.login_port = (uint16)atoi(w2);
			ShowStatus("set login_port : %s\n",w2);
		}
		else if(!strcmpi(w1, "log_login"))
			login_config.log_login = (bool)config_switch(w2);

#ifdef TXT_ONLY
		else if(!strcmpi(w1, "login_log_filename") == 0)
			safestrncpy(login_log_filename, w2, sizeof(login_log_filename));
		else if(!strcmpi(w1, "admin_state") == 0)
			admin_state = (bool)config_switch(w2);
		else if(!strcmpi(w1, "admin_pass") == 0)
			safestrncpy(admin_pass, w2, sizeof(admin_pass));
		else if(!strcmpi(w1, "admin_allowed_ip") == 0)
			admin_allowed_ip = host2ip(w2);
		else if(!strcmpi(w1, "account_txt") == 0)
			safestrncpy(account_txt, w2, sizeof(account_txt));
		else if(!strcmpi(w1, "gm_account_filename") == 0)
			safestrncpy(GM_account_filename, w2, sizeof(GM_account_filename));
		else if(!strcmpi(w1, "gm_account_filename_check_timer") == 0)
			gm_account_filename_check_timer = atoi(w2);
#else
		else if(!strcmpi(w1, "ipban"))
			login_config.ipban = (bool)config_switch(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban"))
			login_config.dynamic_pass_failure_ban = (bool)config_switch(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban_interval"))
			login_config.dynamic_pass_failure_ban_interval = atoi(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban_limit"))
			login_config.dynamic_pass_failure_ban_limit = atoi(w2);
		else if(!strcmpi(w1, "dynamic_pass_failure_ban_duration"))
			login_config.dynamic_pass_failure_ban_duration = atoi(w2);
#endif

		else if(!strcmpi(w1, "new_account"))
			login_config.new_account_flag = (bool)config_switch(w2);
		else if(!strcmpi(w1, "start_limited_time"))
			login_config.start_limited_time = atoi(w2);
		else if(!strcmpi(w1, "check_client_version"))
			login_config.check_client_version = (bool)config_switch(w2);
		else if(!strcmpi(w1, "client_version_to_connect"))
			login_config.client_version_to_connect = atoi(w2);
		else if(!strcmpi(w1, "use_MD5_passwords"))
			login_config.use_md5_passwds = (bool)config_switch(w2);
		else if(!strcmpi(w1, "min_level_to_connect"))
			login_config.min_level_to_connect = atoi(w2);
		else if(!strcmpi(w1, "date_format"))
			safestrncpy(login_config.date_format, w2, sizeof(login_config.date_format));
		else if(!strcmpi(w1, "console"))
			login_config.console = (bool)config_switch(w2);
		else if(!strcmpi(w1, "case_sensitive"))
			login_config.case_sensitive = config_switch(w2);
		else if(!strcmpi(w1, "allowed_regs")) //account flood protection system
			allowed_regs = atoi(w2);
		else if(!strcmpi(w1, "time_allowed"))
			time_allowed = atoi(w2);
		else if(!strcmpi(w1, "online_check"))
			login_config.online_check = (bool)config_switch(w2);
		else if(!strcmpi(w1, "use_dnsbl"))
			login_config.use_dnsbl = (bool)config_switch(w2);
		else if(!strcmpi(w1, "dnsbl_servers"))
			safestrncpy(login_config.dnsbl_servs, w2, sizeof(login_config.dnsbl_servs));
		else if(!strcmpi(w1, "ip_sync_interval"))
			login_config.ip_sync_interval = (unsigned int)1000*60*atoi(w2); //w2 comes in minutes.
		else if(!strcmpi(w1, "import"))
			login_config_read(w2);
	}
	fclose(fp);
	ShowInfo("Finished reading %s.\n", cfgName);
	return 0;
}


//--------------------------------------
// Function called at exit of the server
//--------------------------------------
void do_final(void)
{
	int i, fd;

	login_log(0, "login server", 100, "login server shutdown");
	ShowStatus("Terminating...\n");

#ifdef TXT_ONLY
	mmo_auth_sync();
#else
	mmo_db_close();
#endif
	online_db->destroy(online_db, NULL);
	auth_db->destroy(auth_db, NULL);

#ifdef TXT_ONLY
	if(auth_dat) aFree(auth_dat);
#endif
	if(gm_account_db) aFree(gm_account_db);

	for (i = 0; i < MAX_SERVERS; i++) {
		if ((fd = server[i].fd) >= 0) {
			memset(&server[i], 0, sizeof(struct mmo_char_server));
			server[i].fd = -1;
			do_close(fd);
		}
	}
	do_close(login_fd);

	ShowStatus("Finished.\n");
}

//------------------------------
// Function called when the server
// has received a crash signal.
//------------------------------
void do_abort(void)
{
}

void set_server_type(void)
{
	SERVER_TYPE = ATHENA_SERVER_LOGIN;
}

//------------------------------
// Login server initialization
//------------------------------
int do_init(int argc, char** argv)
{
	int i;

	login_set_defaults();

	// read login-server configuration
	login_config_read((argc > 1) ? argv[1] : LOGIN_CONF_NAME);
#ifdef TXT_ONLY
	display_conf_warnings(); // not in login_config_read, because we can use 'import' option, and display same message twice or more
#else
	sql_config_read(INTER_CONF_NAME);
#endif
	login_lan_config_read((argc > 2) ? argv[2] : LAN_CONF_NAME);

	srand((unsigned int)time(NULL));

	for( i = 0; i < MAX_SERVERS; i++ )
		server[i].fd = -1;

	// Accounts database init
	mmo_auth_init();

	// Online user database init
	online_db = idb_alloc(DB_OPT_RELEASE_DATA);
	add_timer_func_list(waiting_disconnect_timer, "waiting_disconnect_timer");

	// Interserver auth init
	auth_db = idb_alloc(DB_OPT_RELEASE_DATA);

	// Read account information.
#ifdef TXT_ONLY
	read_gm_account();
#else
	if(login_config.login_gm_read)
		read_gm_account();
#endif

	// set default parser as parse_login function
	set_defaultparse(parse_login);

#ifdef TXT_ONLY
	// every 60 sec we check if we must save accounts file (only if necessary to save)
	add_timer_func_list(check_auth_sync, "check_auth_sync");
	add_timer_interval(gettick() + 60000, check_auth_sync, 0, 0, 60000);

	// every x sec we check if gm file has been changed
	if( gm_account_filename_check_timer ) {
		add_timer_func_list(check_GM_file, "check_GM_file");
		add_timer_interval(gettick() + gm_account_filename_check_timer * 1000, check_GM_file, 0, 0, gm_account_filename_check_timer * 1000); 
	}
#else
	// ban deleter timer
	add_timer_func_list(ip_ban_flush, "ip_ban_flush");
	add_timer_interval(gettick()+10, ip_ban_flush, 0, 0, 60*1000);
#endif

	// every 10 minutes cleanup online account db.
	add_timer_func_list(online_data_cleanup, "online_data_cleanup");
	add_timer_interval(gettick() + 600*1000, online_data_cleanup, 0, 0, 600*1000);

	// add timer to detect ip address change and perform update
	if (login_config.ip_sync_interval) {
		add_timer_func_list(sync_ip_addresses, "sync_ip_addresses");
		add_timer_interval(gettick() + login_config.ip_sync_interval, sync_ip_addresses, 0, 0, login_config.ip_sync_interval);
	}

	if( login_config.console )
	{
		//##TODO invoke a CONSOLE_START plugin event
	}

	// server port open & binding
	login_fd = make_listen_bind(login_config.login_ip, login_config.login_port);

	ShowStatus("The login-server is "CL_GREEN"ready"CL_RESET" (Server is listening on the port %u).\n\n", login_config.login_port);
	login_log(0, "login server", 100, "login server started");

	return 0;
}
