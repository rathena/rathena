void generate_notdone(int sock_in, char *query, char *ip)
{
	web_send(sock_in, "<title>Not here!</title>\n");
	web_send(sock_in, "<h2><center>This page/feature is not done yet.</center>\n</h2>");	
}
