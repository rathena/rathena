/******************************************************************************
 #	            ______  __    __                                 			  #
 #	           /\  _  \/\ \__/\ \                                			  #
 #	         __\ \ \L\ \ \ ,_\ \ \___      __    ___      __     			  #
 #	       /'__`\ \  __ \ \ \/\ \  _ `\  /'__`\/' _ `\  /'__`\   			  #
 #  	  /\  __/\ \ \/\ \ \ \_\ \ \ \ \/\  __//\ \/\ \/\ \L\.\_ 			  #
 #        \ \____\\ \_\ \_\ \__\\ \_\ \_\ \____\ \_\ \_\ \__/.\_\			  #
 #  	   \/____/ \/_/\/_/\/__/ \/_/\/_/\/____/\/_/\/_/\/__/\/_/			  #
 #					eAthena Web Server (Second Edition)						  #
 #								by MC Cameri								  #
 #		  -------------------------------------------------------			  #
 #							-Website/Forum-									  #
 #					http://eathena.deltaanime.net/							  #
 #							-Download URL- 									  #
 #					http://eathena.systeminplace.net/						  #
 #							-IRC Channel-									  #
 #					irc://irc.deltaanime.net/#athena						  #
 ******************************************************************************/

#include <stdio.h>
#include <strings.h>
#include "../common/showmsg.h"
#include "webserver.h"

char ws_password[17] = "pass";
char ws_header[128] = {'\0'};

/* Displays the eAthena Logo */
void ws_display_title(void)
{
	printf("\033[2J");
	printf("\033[37;44m          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1;33m        (c)2004 eAthena Development Team presents        \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m       ______  __    __                                  \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m      /\\  _  \\/\\ \\__/\\ \\                                 \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m    __\\ \\ \\_\\ \\ \\ ,_\\ \\ \\___      __    ___      __      \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m  /'__`\\ \\  __ \\ \\ \\/\\ \\  _ `\\  /'__`\\/' _ `\\  /'__`\\    \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m /\\  __/\\ \\ \\/\\ \\ \\ \\_\\ \\ \\ \\ \\/\\  __//\\ \\/\\ \\/\\ \\_\\.\\_  \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m \\ \\____\\\\ \\_\\ \\_\\ \\__\\\\ \\_\\ \\_\\ \\____\\ \\_\\ \\_\\ \\__/.\\_\\ \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m  \\/____/ \\/_/\\/_/\\/__/ \\/_/\\/_/\\/____/\\/_/\\/_/\\/__/\\/_/ \033[0;44m)\033[K\033[0m\n");
	printf("\033[0;44m          (\033[1m   _   _   _   _   _   _   _     _   _   _   _   _   _   \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m  / \\ / \\ / \\ / \\ / \\ / \\ / \\   / \\ / \\ / \\ / \\ / \\ / \\  \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m ( e | n | g | l | i | s | h ) ( A | t | h | e | n | a ) \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m  \\_/ \\_/ \\_/ \\_/ \\_/ \\_/ \\_/   \\_/ \\_/ \\_/ \\_/ \\_/ \\_/  \033[0;44m)\033[K\033[0m\n"); // 1: bold char, 0: normal char
	printf("\033[0;44m          (\033[1m                                                         \033[0;44m)\033[K\033[0m\n"); // yellow writing (33)
	printf("\033[0;44m          (\033[1;33m  Advanced Fusion Maps (c) 2003-2004 The Fusion Project  \033[0;44m)\033[K\033[0m\n"); // yellow writing (33)
	printf("\033[37;44m          (=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)\033[K\033[0m\n\n"); // reset color
}
/* Returns a boolean value given character string */
int ws_config_switch(const char *str) {
	if (strcmpEx(str, "on") == 0 || strcmpEx(str, "yes") == 0 ||
		strcmpEx(str, "oui") == 0 || strcmpEx(str, "ja") == 0 ||
		strcmpEx(str, "si") == 0 || strcmpEx(str,"true") == 0)
		return 1;
	if (strcmpEx(str, "off") == 0 || strcmpEx(str, "no") == 0 ||
		strcmpEx(str, "non") == 0 || strcmpEx(str, "nein") == 0 ||
		strcmpEx(str, "false") == 0)
		return 0;
	return atoi(str);
}

/*  Reads the eAthena Web Server's configuration file */
int ws_config_read(const char *cfgName)
{
	int i;
	char line[1024],w1[1024],w2[1024],temp[1024];
	FILE *fp;

	/* Default values */
	config.show_title=0;
	config.port=80;


	fp=fopen(cfgName,"r");
	if(fp==NULL){
		snprintf(temp,sizeof(temp),"Could not open \033[1;29m%s\033[0;0m, file not found.\n",cfgName);
		ShowMessage(temp,MSG_ERROR);
		return 1;
	}
	while(fgets(line,1020,fp)){
		const struct {
			char str[128];
			int *val;
		} data[] ={
			//List of variables
			{ "show_title",		&config.show_title	},
			{ "port",			&config.port		},
		};

		if((line[0] == '/' && line[1] == '/') || (line[0]=='#') ||
			(sscanf(line,"%1023[^:]:%1023[^\n]",w1,w2) !=2))
			continue;
		for(i=0;i<sizeof(data)/(sizeof(data[0]));i++) {
			if(strcmpEx(w1,data[i].str)==0){
				*data[i].val=ws_config_switch(w2);
				break;
			}
		}
		if(strcmpEx(w1,"import")==0) {
			ws_config_read(w2);
			continue;
		}
		if(strcmpEx(w1,"password")==0) {
			if (strlen(w2)>16) {
				ShowError("The Web Server password is too long, maximum passwor"
					"d length is 16 characters.\n");
				return 1;
			}
			strcpy(ws_password,w2);
			continue;
		}
		if(strcmpEx(w1,"header")==0) {
			if (strlen(w2)>127) {
				ShowError("The Web Server header is too long, maximum header"
					"d length is 127 characters.\n");
				return 1;
			}
			strcpy(ws_header,w2);
			continue;
		}
	}
	fclose(fp);

	//Correct values
	if(config.show_title < 0)
		config.show_title = 0;
	if(config.port < 1 || config.port > 65534)
		config.port=80;

	return 0;
}

void ws_sigchld_handler(int s)
{
	while(wait(NULL) > 0);
}
