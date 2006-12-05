#include <stdlib.h>

char filtered_query[2000];
char rdata[500];
char param_n[500];
char param_d[500];


char *get_query(char *inquery)
{
	memset(filtered_query, 0x0, 2000);
	sscanf(inquery, "GET %s %[$]", filtered_query);
	return(filtered_query);
}

void web_send(int sockin, char *in_data)
{
	send(sockin, in_data, strlen(in_data), 0);
}


//THIS IS BAD CODE BE CAREFULL WITH IT!
//Watch out for buffer overflow...
//When using please make sure to check the string size.

//Also note:
//I take no pride in this code, it is a really bad way of doing this...
char *get_param(char in_string[500], char swhat[500])
{
	int i = 0;
	int marker, iswitch, pint, dint;
	char flux[500];
	memset(flux, 0x0, 500);

	//Get the path of out "page"
	if (swhat == 0)
	{
		//while i is not equal to array size
		while (i != 500)
		{
			//if there is a question mark, halt!
			if (in_string[i] == '?')
			{
				i = 499;
			}
			else 
				rdata[i] = in_string[i];
	
			i++;
		}
		return rdata;
	}
	else //so, we want a param...
	{
		//calculate where param begins
		while (i != 500)
		{
			if (in_string[i] == '?')
			{
				marker = i + 1;
				i = 499;
			}
			i++;
		}

		i = 0;

		//keep morons from trying to crash this
		if ((marker > 500)||(marker < 1))
			marker = 500;

		while(marker != 500)
		{
			if ((in_string[marker] != '&') && (in_string[marker] != '\0'))
			{
				flux[i] = in_string[marker];
				i++;
			}
			else
			{

				//we have a param, now we must dig through it

				//clear temp vars
				memset(param_n, 0x0, 500);
				memset(param_d, 0x0, 500);
				iswitch = 0;
				pint = 0;
				dint = 0;
				i = 0;

				//split result into param_n and param_d
				while(i != 500)
				{
					if ( (flux[i] != '=') && (flux[i] != '\0') )
					{
						if (iswitch == 0)
						{
							param_n[pint] = flux[i];
							pint++;
						}
						else
						{
							param_d[dint] = flux[i];
							dint++;
						}
					}
					else
					{
						iswitch = 1;
					}
					if (flux[i] == '\0')
						i = 499;

					i++;
				}

				if ( strcmp(param_n, swhat) == 0 )
				{
					return param_d;
				}

				i = 0;
			}
			
			if (in_string[marker] == '\0')
			{
				marker = 499;
			}
			marker++;
		}
		return 0;
	}
}
			
