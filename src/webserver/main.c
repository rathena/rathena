/***************************************************************************
                                description
                             -------------------
    author               : (C) 2004 by Michael J. Flickinger
    email                : mjflick@cpan.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BLOG 10

char *header = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n";
char recvin[500], password[25];
int s_port;

void sigchld_handler(int s)
{
	while(wait(NULL) > 0);
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("eAthena Web Server\n");
		printf("usage: %s [password] [port]\n", argv[0]);
		exit(0);
	}

	s_port = atoi(argv[2]);

	if ((s_port < 1) || (s_port > 65534))
	{
		printf("Error: The port you choose is not valid port.\n");
		exit(0);
	} 

	if (strlen(argv[1]) > 25)
	{
		printf("Error: Your password is too long.\n");
		printf("It must be shorter than 25 characters.\n");
		exit(0);
	}

	memset(password, 0x0, 25);
	memcpy(password, argv[1], strlen(argv[1]));

	int sockfd, new_fd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;

	struct sigaction sa;

	int yes=1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM,0)) == -1)
	{
		perror("Darn, this is broken.");
		exit(0);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("Error... :-(");
	}

	//Now we know we have a working socket. :-)

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(s_port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8);

	if ( bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("can not bind to this port");
		exit(0);
	}

	if ( listen(sockfd, BLOG) == -1)
	{
		perror("can not listen on port");
		exit(0);
	}

	sa.sa_handler = sigchld_handler;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1)
	{
		perror("sigaction sucks");
		exit(0);
	}

	printf("The eAthena webserver is up and listening on port %i.\n", s_port);

	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

		if (!fork())
		{
			close(sockfd);
			memset(recvin, 0x0, 500);
			recv(new_fd, recvin, 500, 0);
			send(new_fd, header, strlen(header), 0);
			generate_page(password, new_fd, get_query(recvin), inet_ntoa(their_addr.sin_addr));
			log_visit(get_query(recvin), inet_ntoa(their_addr.sin_addr));

			close(new_fd);
			exit(0);
		}
		close(new_fd);	
	}

	return 0;
}
