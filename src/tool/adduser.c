// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/*
	This program adds an user to account.txt
	Don't usr it When login-sever is working.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char *account_txt = "../save/account.txt";

//-----------------------------------------------------
// Function to suppress control characters in a string.
//-----------------------------------------------------
int remove_control_chars(char* str)
{
	int i;
	int change = 0;

	for(i = 0; str[i]; i++) {
		if (iscntrl((unsigned char)(str[i]))) {
			str[i] = '_';
			change = 1;
		}
	}

	return change;
}

int main(int argc, char *argv[])
{
	char username[24];
	char password[24];
	char sex[2];

	int next_id, id;
	char line[1024];
	FILE *FPaccin,*FPaccout;

	// Check to see if account.txt exists.
	printf("Checking if '%s' file exists...\n", account_txt);
	FPaccin = fopen(account_txt, "r");
	if (FPaccin == NULL) {
		printf("'%s' file not found!\n", account_txt);
		printf("Run the setup wizard please.\n");
		exit(EXIT_SUCCESS);
	}

	next_id = 2000000;
	while(fgets(line, sizeof(line), FPaccin))
	{
		if (line[0] == '/' && line[1] == '/') { continue; }
		if (sscanf(line, "%d\t%%newid%%\n", &id) == 1) {
			if (next_id < id) {
				next_id = id;
			}
		} else {
			sscanf(line,"%i%[^	]", &id);
			if (next_id <= id) {
				next_id = id +1;
			}
		}
	}
	fclose(FPaccin);
	printf("File exists.\n");

	printf("Don't create an account if the login-server is online!!!\n");
	printf("If the login-server is online, press ctrl+C now to stop this software.\n");
	printf("\n");

	strcpy(username, "");
	while (strlen(username) < 4 || strlen(username) > 23) {
		printf("Enter an username (4-23 characters): ");
		scanf("%s", &username);
		username[23] = 0;
		remove_control_chars(username);
	}

	strcpy(password, "");
	while (strlen(password) < 4 || strlen(password) > 23) {
		printf("Enter a password (4-23 characters): ");
		scanf("%s", &password);
		password[23] = 0;
		remove_control_chars(password);
	}

	strcpy(sex, "");
	while (strcmp(sex, "F") != 0 && strcmp(sex, "M") != 0) {
		printf("Enter a gender (M for male, F for female): ");
		scanf("%s", &sex);
	}

	FPaccout = fopen(account_txt, "r+");
	fseek(FPaccout, 0, SEEK_END);
	fprintf(FPaccout, "%i	%s	%s	-	%s	-\r\n", next_id, username, password, sex);
	fclose(FPaccout);

	printf("Account added.\n");
}
