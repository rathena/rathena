#include "map.h"

// IRC .conf file [Zido]
#define IRC_CONF	"irc_athena.conf"

// IRC Access levels [Zido]
#define	ACCESS_OWNER	5
#define	ACCESS_SOP		4
#define	ACCESS_OP		3
#define	ACCESS_HOP		2
#define	ACCESS_VOICE	1
#define ACCESS_NORM		0

#define MAX_CHANNEL_USERS	500

extern short use_irc;

extern short irc_announce_flag;
extern short irc_announce_mvp_flag;
extern short irc_announce_shop_flag;
extern short irc_announce_jobchange_flag;

void irc_announce(char *buf);
void irc_announce_jobchange(struct map_session_data *sd);
void irc_announce_shop(struct map_session_data *sd,int flag);
void irc_announce_mvp(struct map_session_data *sd, struct mob_data *md);

int irc_parse(int fd);
void do_final_irc(void);
void do_init_irc(void);
void irc_send(char *buf);
void irc_parse_sub(int fd, char *incoming_string);
int send_to_parser(int fd, char *input,char key[2]);
struct IRC_Session_Info {
    int state;
    int fd;
    char username[30];
    char password[33];
};

typedef struct IRC_Session_Info IRC_SI;

struct channel_data {
	struct {
		char name[256];
		int level;
	}user[MAX_CHANNEL_USERS];
};

int parse_names_packet(char *str); // [Zido]
int parse_names(char *str); // [Zido]
int set_access(char *nick,int level); // [Zido]
int get_access(char *nick); // [Zido]
int irc_rmnames(void); // [Zido]
int irc_read_conf(char *file); // [Zido]
