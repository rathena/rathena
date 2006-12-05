char output[10000];

char *html_header(char *title)
{
	memset(output, 0x0, 10000);
	char *text = "<body text=\"#000000\" bgcolor=\"#939393\" link=\"#0033FF\">\n"
	"<br><table width=\"92%\" cellspacing=\"1\" cellpadding=\"0\" border=\"0\"\n"
	"align=\"center\" class=\"bordercolor\"><tbody><tr><td class=\"bordercolor\" width=\"100%\">\n"
	"<table bgcolor=\"#ffffff\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">\n"
	"<tbody><tr><td><table border=\"0\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\" bgcolor=\"#ffffff\">\n"
	"<tbody><tr><img src=\"http://eathena.sourceforge.net/athena.jpg\" alt=\"Athena\">\n"
	"<td bgcolor=\"#ffffff\"></td></tr></tbody></table></td></tr></tbody></table>\n"
	"</td></tr><tr align=\"left\"><td class=\"bordercolor\"><table bgcolor=\"#c6c6c6\" width=\"100%\" cellspacing=\"0\"\n"
	"cellpadding=\"0\" style=\"text-align: left; margin-right: auto; margin-left: 0px;\">\n";
        "<tbody><tr><td width=\"100%\" align=\"center\"><table border=\"0\" width=\"100%\" cellpadding=\"3\"\n"
 	"cellspacing=\"0\" bgcolor=\"#c6c6c6\" align=\"center\"><tbody><tr>"
	"<td valign=\"middle\" bgcolor=\"#c6c6c6\" align=\"center\"><a href=\"/cgi-bin/forum/YaBB.cgi\">"
	"<span style=\"text-decoration: underline;\"><span style=\"font-weight: bold;\">\n"
	"To the Forum</span></span></a><br></td></tr></tbody></table></td></tr></tbody>\n"
	"</table></td></tr><tr><td class=\"bordercolor\" align=\"center\">\n"
	"<table bgcolor=\"#ffffff\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" align=\"center\">\n"
        "<tbody><tr><td width=\"100%\" align=\"center\"><table border=\"0\" width=\"100%\" cellpadding=\"5\"\n"
	"cellspacing=\"0\" bgcolor=\"#ffffff\" align=\"center\"><tbody><tr>\n"
        "<td valign=\"middle\" bgcolor=\"#ffffff\" align=\"center\"><font size=\"2\" color=\"#6e94b7\">\n"
	"<b>Athena</b> &laquo; Portal &raquo;</font></td></tr></tbody></table></td></tr></tbody>"
        "</table></td></tr></tbody></table>\n";

	sprintf(output, "<title>%s</title>\n%s\n", title, text);

	return output;
}



char *html_start_form(char *location, char *action)
{
	memset(output, 0x0, 10000);
	sprintf(output, "<form action=\"%s\" method=\"%s\">", location, action);
	return output;


}


char *html_end_forum(void)
{
	return "</form>";
}



