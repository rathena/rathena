#include <time.h>

void log_visit(char *query, char *ip)
{
	time_t timer;
	timer=time(NULL);
	printf("%s - \"%s\" - %s", ip, query, asctime(localtime(&timer)));
}
