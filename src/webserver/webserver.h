#ifndef WEBSERV_H_
#define WEBSERV_H_

#define strcmpEx(x,y) (strcasecmp(x,y))

extern void ws_display_title(void);

extern int ws_config_read(const char *cfgName);

extern struct config {
	int show_title;
	int port;
} config;

extern char ws_password[17]; //16 chars + \0

extern char ws_header[128]; //!?

extern void ws_sigchld_handler(int s);

#endif
