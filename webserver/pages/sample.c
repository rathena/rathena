

void generate_sample(int sock_in, char *query, char *ip)
{

	char *name = get_param(query, "name");

	web_send(sock_in, "<title>SAMPLE</title>\n");


	//If a name was not entered...
	if ( name == '\0' )
	{
		web_send(sock_in, "<form action=\"/testing/\" method=\"GET\">\n");
		web_send(sock_in, "<input type=\"text\" name=\"name\">\n");
		web_send(sock_in, "<input type=\"submit\">\n");
	}
	else
	{
		web_send(sock_in, "Your name is: ");
		web_send(sock_in, get_param(query, "name"));
	}
printf("OK!\n");
}
