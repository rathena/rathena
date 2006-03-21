#include <ctype.h>
#include <string.h>

#ifdef __WIN32
#define __USE_W32_SOCKETS
#include <windows.h>
#include <io.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifndef SIOCGIFCONF
#include <sys/sockio.h> // SIOCGIFCONF on Solaris, maybe others? [Shinomori]
#endif

#endif

#include "../common/core.h"
#include "../common/socket.h"
#include "../common/malloc.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/strlib.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/version.h"

#include "nullpo.h"
#include "map.h"
#include "pc.h"
#include "irc.h"

short use_irc=0;

short irc_announce_flag=1;
short irc_announce_mvp_flag=1;
short irc_announce_jobchange_flag=1;
short irc_announce_shop_flag=1;

IRC_SI *irc_si=NULL;

char irc_nick[30]="";
char irc_password[32]="";

char irc_channel[32]="";
char irc_trade_channel[32]="";

unsigned char irc_ip_str[128]="";
unsigned long irc_ip=0;
unsigned short irc_port = 6667;
int irc_fd=0;

int irc_connect_timer(int tid, unsigned int tick, int id, int data)
{
	if(irc_si && session[irc_si->fd])
		return 0;
	irc_fd = make_connection(irc_ip,irc_port);
	if(irc_fd > 0){
		session[irc_fd]->func_parse = irc_parse;
	}
	return 0;
}

void irc_announce(char *buf)
{
	char send_string[256];
	memset(send_string,'\0',256);

	sprintf(send_string,"PRIVMSG %s :%s",irc_channel, buf);
	irc_send(send_string);
}

void irc_announce_jobchange(struct map_session_data *sd)
{
	char send_string[256];
	
	nullpo_retv(sd);
	memset(send_string,'\0',256);

	sprintf(send_string,"PRIVMSG %s :%s has changed into a %s.",irc_channel,sd->status.name,job_name(sd->status.class_));
	irc_send(send_string);
}

void irc_announce_shop(struct map_session_data *sd, int flag)
{
	char send_string[256];
	char mapname[16];
	int maplen = 0;
	nullpo_retv(sd);

	memset(send_string,'\0',256);
	memset(mapname,'\0',16);

	if(flag){
		strcpy(mapname, map[sd->bl.m].name);
		maplen = strcspn(mapname,".");
		mapname[maplen] = '\0';
		mapname[0]=toupper(mapname[0]);

		sprintf(send_string,"PRIVMSG %s :%s has opened a shop, %s, at <%d,%d> in %s.",irc_trade_channel,sd->status.name,sd->message,sd->bl.x,sd->bl.y,mapname);
	} else
		sprintf(send_string,"PRIVMSG %s :%s has closed their shop.",irc_trade_channel,sd->status.name);

	irc_send(send_string);
}

void irc_announce_mvp(struct map_session_data *sd, struct mob_data *md)
{
	char send_string[256];
	char mapname[16];
	int maplen = 0;

	nullpo_retv(sd);
	nullpo_retv(md);

	memset(send_string,'\0',256);
	memset(mapname,'\0',16);
	mapname[16]='\0';
	strcpy(mapname, map[md->bl.m].name);
	maplen = strcspn(mapname,".");
	mapname[maplen] = '\0';
	mapname[0]=toupper(mapname[0]);

	sprintf(send_string,"PRIVMSG %s :%s the %s has MVP'd %s in %s.",irc_channel,sd->status.name,job_name(sd->status.class_),md->name, mapname);
	irc_send(send_string);
}

int irc_parse(int fd)
{
	if (session[fd]->eof){
		do_close(fd);
		add_timer(gettick() + 15000, irc_connect_timer, 0, 0);
      	return 0;
	}
	if (session[fd]->session_data == NULL){
		irc_si = (struct IRC_Session_Info*)aCalloc(1, sizeof(struct IRC_Session_Info));
		irc_si->fd = fd;
		irc_si->state = 0;
		session[fd]->session_data = irc_si;
	}
	irc_si = (struct IRC_Session_Info*)session[fd]->session_data;
	if(RFIFOREST(fd) > 0){
		char *incoming_string=aCalloc(RFIFOREST(fd),sizeof(char));
		memcpy(incoming_string,RFIFOP(fd,0),RFIFOREST(fd));
		send_to_parser(fd,incoming_string,"\n");
		RFIFOSKIP(fd,RFIFOREST(fd));
		aFree(incoming_string);
	}
	return 0;
}

int irc_keepalive_timer(int tid, unsigned int tick, int id, int data)
{
	char send_string[128];
	memset(send_string,'\0',128);

	sprintf(send_string,"PRIVMSG %s : ", irc_nick);
	irc_send(send_string);
	add_timer(gettick() + 30000, irc_keepalive_timer, 0, 0);
	return 0;
}

void irc_send_sub(int fd, char transmit[4096])
{
	sprintf(transmit,"%s%c",transmit,10);
	WFIFOHEAD(fd,strlen(transmit));
	memcpy(WFIFOP(fd,0),transmit, strlen(transmit));
	WFIFOSET(fd,strlen(transmit));
}

void irc_send(char *buf)
{
	char transmit[4096];
	
	if(!irc_si || !session[irc_si->fd])
		return;

	memset(transmit,'\0',4096);

	sprintf(transmit,buf);
	irc_send_sub(irc_si->fd,transmit);
}

void irc_parse_sub(int fd, char *incoming_string)
{
	char source[256];
	char command[256];
	char target[256];
	char message[8192];
	char send_string[8192];
	char *source_nick=NULL;
	char *source_ident=NULL;
	char *source_host=NULL;
//	char *state_mgr=NULL;
	
	memset(source,'\0',256);
	memset(command,'\0',256);
	memset(target,'\0',256);
	memset(message,'\0',8192);
	memset(send_string,'\0',8192);

	sscanf(incoming_string, ":%255s %255s %255s :%4095[^\n]", source, command, target, message);
	if (source != NULL) {
		if (strstr(source,"!") != NULL) {
			sscanf(source,"%s!%s@%s",source_nick, source_ident, source_host);
			//source_nick = strtok_r(source,"!",&state_mgr);
			//source_ident = strtok_r(NULL,"@",&state_mgr);
			//source_host = strtok_r(NULL,"%%",&state_mgr);
		}
	}
	if (irc_si->state == 0){
		sprintf(send_string, "NICK %s", irc_nick);
		irc_send(send_string);
		sprintf(send_string, "USER eABot 8 * : eABot");
		irc_send(send_string);
		irc_si->state = 1;
	}
	else if (irc_si->state == 1){
		if(!strcmp(command,"001")){
			ShowStatus("IRC: Connected to IRC.\n");
			sprintf(send_string, "PRIVMSG nickserv :identify %s", irc_password);
			irc_send(send_string);
			sprintf(send_string, "JOIN %s", irc_channel);
			irc_send(send_string);
			irc_si->state = 2;
		}
		else if(!strcmp(command,"433")){
	            ShowError("IRC: Nickname %s is already taken, IRC Client unable to connect.\n", irc_nick);
	        	sprintf(send_string, "QUIT");
			irc_send(send_string);
			if(session[fd])
				session[fd]->eof=1;
		}
	}
	else if (irc_si->state == 2){
		if(!strcmp(source, "PING")){
	        	sprintf(send_string, "PONG %s", command);
			irc_send(send_string);
		}
	}

	return;
}

int send_to_parser(int fd, char *input,char key[2])
{
	char format[4];
	char *temp_string=NULL;
	char *next_string=NULL;
//	char *state_mgr=NULL;
	int total_loops=0;

	//temp_string = strtok_r(input,key,&state_mgr);
	sprintf(format,"%s%s%s","%s",key,"%s");
	sscanf(input, format, temp_string, next_string);
	while (temp_string != NULL){
		total_loops = total_loops+1;
		irc_parse_sub(fd,temp_string);
		//temp_string = strtok_r(NULL,key,&state_mgr);
		sscanf(next_string, format, temp_string, next_string);
	}
	return total_loops;
}

void do_final_irc(void)
{

}

void do_init_irc(void)
{
	struct hostent *irc_hostname;
	
	if(!use_irc)
		return;
	if (irc_ip_str[strlen(irc_ip_str)-1] == '\n') 
		irc_ip_str[strlen(irc_ip_str)-1] = '\0'; 
	irc_hostname=gethostbyname(irc_ip_str);
	irc_ip_str[0]='\0';
	sprintf(irc_ip_str, "%d.%d.%d.%d", (unsigned char)irc_hostname->h_addr[0], (unsigned char)irc_hostname->h_addr[1],
					   (unsigned char)irc_hostname->h_addr[2], (unsigned char)irc_hostname->h_addr[3]);

	irc_ip=inet_addr(irc_ip_str);

	irc_connect_timer(0, 0, 0, 0);

	add_timer_func_list(irc_connect_timer, "irc_connect_timer");
	add_timer_func_list(irc_keepalive_timer, "irc_keepalive_timer");
	add_timer(gettick() + 30000, irc_keepalive_timer, 0, 0);
}

