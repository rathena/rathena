
void generate_page(char password[25], int sock_in, char *query, char *ip)
{
	char *page = get_param(query, 0);
	char *ppass = get_param(query, "password");


	if ( (ppass == 0) || (strcmp(password, ppass) != 0) )
	{
		web_send(sock_in, html_header("Enter your password"));
		web_send(sock_in, "<H1>NOT LOGGED IN!</H1><form action=\"/\" method=\"GET\">\n");
                web_send(sock_in, "Enter your password:<br>\n<input type=\"text\" name=\"password\">\n");
                web_send(sock_in, "<input type=\"submit\" value=\"Login\">\n");
	}
	else
	{


		//To make this simple, we will have a bunch of if statements
		//that then shoot out data off into functions.

	
		//The 'index'
		if ( strcmp(page, "/") == 0 )
			generate_notdone(sock_in, query, ip);


		//About page:
		if ( strcmp(page, "/about.html") == 0 )
			generate_about(sock_in, query, ip);

	
		//Test page:
		if ( strcmp(page, "/testing/") == 0 )
			generate_sample(sock_in, query, ip);

	}
}
