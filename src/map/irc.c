// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/core.h"
#include "../common/socket.h"
#include "../common/malloc.h"
#include "../common/db.h"
#include "../common/timer.h"
#include "../common/strlib.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/version.h"
#include "../common/nullpo.h"

#include "map.h"
#include "pc.h"
#include "irc.h"
#include "intif.h" //For GM Broadcast [Zido]

short use_irc=0;

short irc_autojoin=0;

short irc_announce_flag=1;
short irc_announce_mvp_flag=1;
short irc_announce_jobchange_flag=1;
short irc_announce_shop_flag=1;

IRC_SI *irc_si=NULL;

char irc_nick[30]="";
char irc_password[32]="";

char irc_channel[32]="";
char irc_channel_pass[32]="";
char irc_trade_channel[32]="";

unsigned char irc_ip_str[128]="";
unsigned long irc_ip=0;
unsigned short irc_port = 6667;
int irc_fd=0;

struct channel_data cd;
int last_cd_user=0;

int irc_connect_timer(int tid, unsigned int tick, int id, int data)
{
	if(irc_si && session[irc_si->fd])
		return 0;
	//Ok, this ShowInfo and printf are a little ugly, but they are meant to 
	//debug just how long the code freezes here. [Skotlex]
	ShowInfo("(IRC) Connecting to %s... ", irc_ip_str);
	irc_fd = make_connection(irc_ip,irc_port);
	if(irc_fd > 0){
		printf("ok\n");
		session[irc_fd]->func_parse = irc_parse;
	} else
		printf("failed\n");
	return 0;
}

void irc_announce(char *buf)
{
	char send_string[256];
	// memset(send_string,'\0',256); // NOT REQUIRED

	sprintf(send_string,"PRIVMSG %s :",irc_channel);
	strcat(send_string, buf);
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
	mapname[15]='\0'; // 15 is the final index, not 16 [Lance]
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
		irc_si = NULL;
		add_timer(gettick() + 15000, irc_connect_timer, 0, 0);
      	return 0;
	}
	if (session[fd]->session_data == NULL){
		irc_si = (struct IRC_Session_Info*)aMalloc(sizeof(struct IRC_Session_Info));
		irc_si->fd = fd;
		irc_si->state = 0;
		session[fd]->session_data = irc_si;
	} else if  (!irc_si) {
		irc_si = (struct IRC_Session_Info*)session[fd]->session_data;
		irc_si->fd = fd;
	}
	if(RFIFOREST(fd) > 0){
		char *incoming_string=aMalloc(RFIFOREST(fd)*sizeof(char));
		RFIFOHEAD(fd);
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
	char *state_mgr=NULL;
	
	char cmd1[256];
	char cmd2[256];
	char cmdname[256];
	char cmdargs[256];

	int users=0;
	int i=0;

	struct map_session_data **allsd;
	
	memset(source,'\0',256);
	memset(command,'\0',256);
	memset(target,'\0',256);
	memset(message,'\0',8192);
	memset(send_string,'\0',8192);

	memset(cmd1,'\0',256);
	memset(cmd2,'\0',256);
	memset(cmdname,'\0',256);
	memset(cmdargs,'\0',256);

	sscanf(incoming_string, ":%255s %255s %255s :%4095[^\r\n]", source, command, target, message);
	if (source != NULL) {
		if (strstr(source,"!") != NULL) {
			source_nick = strtok_r(source,"!",&state_mgr);
			source_ident = strtok_r(NULL,"@",&state_mgr);
			source_host = strtok_r(NULL,"%%",&state_mgr);
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
			sprintf(send_string, "JOIN %s %s", irc_channel, irc_channel_pass);
			irc_send(send_string);
			sprintf(send_string,"NAMES %s",irc_channel);
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

		else if((strcmpi(target,irc_channel)==0)||(strcmpi(target,irc_channel+1)==0)) {

			// Broadcast [Zido] (Work in Progress)
			if((strcmpi(command,"privmsg")==0)&&(sscanf(message,"@%255s %255[^\r\n]",cmdname,cmdargs)>0)&&(target[0]=='#')) {
				if(strcmpi(cmdname,"kami")==0) {
					if(get_access(source_nick)<ACCESS_OP)
						sprintf(send_string,"NOTICE %s :Access Denied",source_nick);
					else {
						sprintf(send_string,"%s: %s",source_nick,cmdargs);
						intif_GMmessage(send_string,strlen(send_string)+1,0);
						sprintf(send_string,"NOTICE %s :Message Sent",source_nick);
					}
					irc_send(send_string);
				// Number of users online [Zido]
				} else if(strcmpi(cmdname,"users")==0) {
					map_getallusers(&users);
					sprintf(send_string,"PRIVMSG %s :Users Online: %d",irc_channel,users);
					irc_send(send_string);
				// List all users online [Zido]
				} else if(strcmpi(cmdname,"who")==0) {
					allsd=map_getallusers(&users);
					if(users>0) {
					sprintf(send_string,"NOTICE %s :%d Users Online",source_nick,users);
					irc_send(send_string);
						for(i=0;i<users;i++) {
							sprintf(send_string,"NOTICE %s :Name: \"%s\"",source_nick,allsd[i]->status.name);
							irc_send(send_string);
						}
					} else {
						sprintf(send_string,"NOTICE %s :No Users Online",source_nick);
						irc_send(send_string);
					}
				}
			}

			// Refresh Names [Zido]
			else if((strcmpi(command,"join")==0)||(strcmpi(command,"part")==0)||(strcmpi(command,"mode")==0)||(strcmpi(command,"nick")==0)) {
				ShowInfo("IRC: Refreshing User List");
				irc_rmnames();
				printf("...");
				sprintf(send_string,"NAMES %s",irc_channel);
				printf("...");
				irc_send(send_string);
				printf("Done\n");
			}

			// Autojoin on kick [Zido]
			else if((strcmpi(command,"kick")==0)&&(irc_autojoin==1)) {
				sprintf(send_string, "JOIN %s %s", target, irc_channel_pass);
				irc_send(send_string);
			}
		}

		// Names Reply [Zido]
		else if((strcmpi(command,"353")==0)) {
			ShowInfo("IRC: NAMES recieved\n");
			parse_names_packet(incoming_string);
		}
	}

	return;
}

int send_to_parser(int fd, char *input,char key[2])
{
	char *temp_string=NULL;
	char *state_mgr=NULL;
	int total_loops=0;

	temp_string = strtok_r(input,key,&state_mgr);
	while (temp_string != NULL){
		total_loops = total_loops+1;
		irc_parse_sub(fd,temp_string);
		temp_string = strtok_r(NULL,key,&state_mgr);
	}
	return total_loops;
}

void do_final_irc(void)
{

}

void do_init_irc(void)
{
	if(!use_irc)
		return;
	if (irc_ip_str[strlen(irc_ip_str)-1] == '\n') 
		irc_ip_str[strlen(irc_ip_str)-1] = '\0'; 
	irc_ip = resolve_hostbyname(irc_ip_str, NULL, irc_ip_str);
	if (!irc_ip)
	{
		ShowError("Unable to resolve %s! Cannot connect to IRC server, disabling irc_bot.\n", irc_ip_str);
		use_irc = 0;
		return;
	}

	irc_connect_timer(0, 0, 0, 0);

	add_timer_func_list(irc_connect_timer, "irc_connect_timer");
	add_timer_func_list(irc_keepalive_timer, "irc_keepalive_timer");
	add_timer(gettick() + 30000, irc_keepalive_timer, 0, 0);
}

//NAMES Packet(353) parser [Zido]
int parse_names_packet(char *str) {
	char *tok;
	char source[256];
	char numeric[10];
	char target[256];
	char channel[256];
	char names[1024];

	memset(source,'\0',256);
	memset(numeric,'\0',10);
	memset(target,'\0',256);
	memset(channel,'\0',256);
	memset(names,'\0',1024);

	tok=strtok(str,"\r\n");
	sscanf(tok,":%255s %10s %255s %*1[=@] %255s :%1023[^\r\n]",source,numeric,target,channel,names);
	if(strcmpi(numeric,"353")==0)
		parse_names(names);

	while((tok=strtok(NULL,"\r\n"))!=NULL) {
		sscanf(tok,":%255s %10s %255s %*1[=@] %255s :%1023[^\r\n]",source,numeric,target,channel,names);
		if(strcmpi(numeric,"353")==0)
			parse_names(names);
	}

	return 0;
}

//User access level prefix parser [Zido]
int parse_names(char *str) {
	char *tok;
	if (str == NULL)  return 0; //Nothing to parse!
	tok=strtok(str," ");
	switch(tok[0]) {
			case '~':
				set_access(tok+1,ACCESS_OWNER);
				break;
			case '&':
				set_access(tok+1,ACCESS_SOP);
				break;
			case '@':
				set_access(tok+1,ACCESS_OP);
				break;
			case '%':
				set_access(tok+1,ACCESS_HOP);
				break;
			case '+':
				set_access(tok+1,ACCESS_VOICE);
				break;
			default:
				set_access(tok,ACCESS_NORM);
				break;	
	}

	while((tok=strtok(NULL," "))!=NULL) {
		switch(tok[0]) {
			case '~':
				set_access(tok+1,ACCESS_OWNER);
				break;
			case '&':
				set_access(tok+1,ACCESS_SOP);
				break;
			case '@':
				set_access(tok+1,ACCESS_OP);
				break;
			case '%':
				set_access(tok+1,ACCESS_HOP);
				break;
			case '+':
				set_access(tok+1,ACCESS_VOICE);
				break;
			default:
				set_access(tok,ACCESS_NORM);
				break;	
		}
	}
	
	return 1;
}

//Store user's access level [Zido]
int set_access(char *nick,int newlevel) {
	int i=0;
	
	for(i=0;i<=MAX_CHANNEL_USERS;i++) {
		if(strcmpi(cd.user[i].name,nick)==0) {
			cd.user[i].level=newlevel;
			return 1;
		}
	}

	strcpy(cd.user[last_cd_user].name,nick);
	cd.user[last_cd_user].level=newlevel;
	last_cd_user++;

	return 0;
}

//Returns users access level [Zido]
int get_access(char *nick) {
	int i=0;
	
	for(i=0;i<=MAX_CHANNEL_USERS;i++) {
		if(strcmpi(cd.user[i].name,nick)==0) {
			return (cd.user[i].level);
		}
	}

	return -1;
}

int irc_rmnames() {
	int i=0;
	
	for(i=0;i<=MAX_CHANNEL_USERS;i++) {
		//memset(cd.user[i].name,'\0',256);
		cd.user[i].level=0;
	}

	last_cd_user=0;
	

	return 0;
}

int irc_read_conf(char *file) {
	FILE *fp=NULL;
	char w1[256];
	char w2[256];
	char path[256];
	char row[1024];

	memset(w1,'\0',256);
	memset(w2,'\0',256);
	memset(path,'\0',256);
	memset(row,'\0',256);

	sprintf(path,"conf/%s",file);

	if(!(fp=fopen(path,"r"))) {
		ShowError("Cannot find file: %s\n",path);
		return 0;
	}

	while(fgets(row,1023,fp)!=NULL) {
		if(row[0]=='/'&&row[1]=='/')
			continue;
		sscanf(row,"%[^:]: %255[^\r\n]",w1,w2);
		if(strcmpi(w1,"use_irc")==0) {
			if(strcmpi(w2,"on")==0)
				use_irc=1;
			else
				use_irc=0;
		}
		else if(strcmpi(w1,"irc_server")==0)
			strcpy(irc_ip_str,w2);
		else if(strcmpi(w1,"irc_port")==0)
			irc_port=atoi(w2);
		else if(strcmpi(w1,"irc_autojoin")==0)
			irc_autojoin=atoi(w2);
		else if(strcmpi(w1,"irc_channel")==0)
			strcpy(irc_channel,w2);
		else if(strcmpi(w1,"irc_channel_pass")==0)
			strcpy(irc_channel_pass,w2);
		else if(strcmpi(w1,"irc_trade_channel")==0)
			strcpy(irc_trade_channel,w2);
		else if(strcmpi(w1,"irc_nick")==0)
			strcpy(irc_nick,w2);
		else if(strcmpi(w1,"irc_pass")==0) {
			if(strcmpi(w2,"0")!=0)
				strcpy(irc_password,w2);
		}
	}

	ShowInfo("IRC Config read successfully\n");

	return 1;
}
