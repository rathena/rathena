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
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "../common/showmsg.h"
#include "webserver.h"

char ws_password[17];
char ws_header[128];

#define WEB_CONF "conf/webserver-athena.conf"
#define MAX_CONNECTIONS 10
#define HOME "home/"

struct config config;

int main(int argc, char *argv[])
{
	int server_fd, client_fd;
	int sin_size;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	struct sigaction sa;
	char recvin[1024];
	char path[1024];
	char line[1024];
	int optval = 1;
	if (ws_config_read(WEB_CONF)) exit(0);
	if (config.show_title)
		ws_display_title();
	else
		printf("eAthena Web Server (Second Edition)\n");
	if (strcmpEx(ws_password,"webpass")==0)
		ShowWarning("You are using the default password (webpass), we highly "
			"recommend\n           that you change it.\n");
	else if (strstr(ws_password,"webpass"))
		ShowWarning("Your password should not contain \"webpass\" in it, it is"
			" highly\n           recommended that you change it.\n");
	printf("Web Server Password: %s\n",ws_password);
	printf("Web Server Port: %d\n",config.port);

	if ((server_fd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		ShowError("In main() -> Could not open socket.\n");
		return 1;
	}
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
		ShowError("In main() -> Could not set socket options.\n");
		return 1;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(config.port);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(server_addr.sin_zero), '\0', 8);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
		snprintf(tmp_output,sizeof(tmp_output),"In main() -> Could not bind to port number: %d\n",config.port);
		ShowError(tmp_output);
		return 1;
	}

	if (listen(server_fd, MAX_CONNECTIONS) < 0) {
		snprintf(tmp_output,sizeof(tmp_output),"In main() -> Could not listen on port number: %d\n",config.port);
		ShowError(tmp_output);
		return 1;
	}

	sa.sa_handler = ws_sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		ShowError("In main() -> Invalid sigaction.\n");
		return 1;
	}
	ShowInfo("eAthena Web Server is now listening for incoming connections.\n");

	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);

		if (!fork())
		{
			close(server_fd);
			memset(recvin, 0x0, 500);
			recv(client_fd, recvin, 500, 0);
			char *html_output;
			int count = 0;
			if (sscanf(recvin,"GET %[a-zA-Z_-.+\%#@~] %*[^\n]",path)==1) {
				FILE *fp;
				strcpy(tmp_output,HOME);
				strcat(tmp_output,path);
				fp = fopen(tmp_output,"r+");
				if (fp==NULL) {
					send(client_fd,"File not found",strlen("File not found"), 0);
					close(client_fd);
				}
				memset(tmp_output,0x0,strlen(tmp_output));
				html_output = (char*)malloc(sizeof(char)*2);
				while (fgets(line,1023,fp)) {
					html_output = (char*)realloc(sizeof(html_output)+(sizeof(char)*count));
					strcat(html_output,line);	
					printf(line);
				}
				send(client_fd,tmp_output,sizeof(tmp_output),0);
				fclose(fp);
			}
		//	send(client_fd, ws_header, strlen(ws_header), 0);
		//	generate_page(password, client_fd, get_query(recvin), inet_ntoa(client_addr.sin_addr));
		//	log_visit(get_query(recvin), inet_ntoa(client_addr.sin_addr));
			close(client_fd);
			exit(0);
		}
		close(client_fd);	
	}
	return 0;
}
