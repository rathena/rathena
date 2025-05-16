// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "msg_conf.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "malloc.hpp"
#include "showmsg.hpp"
#include "strlib.hpp"

/*
 * Return the message string of the specified number by [Yor]
 * (read in table msg_table, with specified lenght table in size)
 */
const char* _msg_txt(int32 msg_number,int32 size, char ** msg_table)
{
	if (msg_number >= 0 && msg_number < size &&
		msg_table[msg_number] != nullptr && msg_table[msg_number][0] != '\0')
	return msg_table[msg_number];

	return "??";
}


/*
 * Read txt file and store them into msg_table
 */
int32 _msg_config_read(const char* cfgName,int32 size, char ** msg_table)
{
	uint16 msg_number, msg_count = 0, line_num = 0;
	char line[1024], w1[8], w2[512];
	FILE *fp;
	static int32 called = 1;

	if ((fp = fopen(cfgName, "r")) == nullptr) {
		ShowError("Messages file not found: %s\n", cfgName);
		return -1;
	}

	if ((--called) == 0)
		memset(msg_table, 0, sizeof (msg_table[0]) * size);

	while (fgets(line, sizeof (line), fp)) {
		line_num++;
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%7[^:]: %511[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "import") == 0)
			_msg_config_read(w2,size,msg_table);
		else {
			msg_number = atoi(w1);
			if (msg_number >= 0 && msg_number < size) {
				if (msg_table[msg_number] != nullptr)
					aFree(msg_table[msg_number]);
				size_t len = strnlen(w2,sizeof(w2)) + 1;
				msg_table[msg_number] = (char *) aMalloc(len * sizeof (char));
				safestrncpy(msg_table[msg_number], w2, len);
				msg_count++;
			}
			else
				ShowWarning("Invalid message ID '%s' at line %d from '%s' file.\n",w1,line_num,cfgName);
		}
	}

	fclose(fp);
	ShowInfo("Done reading " CL_WHITE "'%d'" CL_RESET " messages in " CL_WHITE "'%s'" CL_RESET ".\n",msg_count,cfgName);

	return 0;
}

/*
 * Destroy msg_table (freeup mem)
 */
void _do_final_msg(int32 size, char ** msg_table){
	int32 i;
	for (i = 0; i < size; i++)
		aFree(msg_table[i]);
}

/*
 * lookup a langtype string into his associate langtype number
 * return -1 if not found
 */
int32 msg_langstr2langtype(char * langtype){
	int32 lang=-1;
	if (!strncmpi(langtype, "eng",2)) lang = 0;
	else if (!strncmpi(langtype, "rus",2)) lang = 1;
	else if (!strncmpi(langtype, "spn",2)) lang = 2;
	else if (!strncmpi(langtype, "grm",2)) lang = 3;
	else if (!strncmpi(langtype, "chn",2)) lang = 4;
	else if (!strncmpi(langtype, "mal",2)) lang = 5;
	else if (!strncmpi(langtype, "idn",2)) lang = 6;
	else if (!strncmpi(langtype, "frn",2)) lang = 7;
	else if (!strncmpi(langtype, "por",2)) lang = 8;
	else if (!strncmpi(langtype, "tha",2)) lang = 9;

	return lang;
}

/*
 * lookup a langtype into his associate lang string
 * return ?? if not found
 */
const char* msg_langtype2langstr(int32 langtype){
	switch(langtype){
		case 0: return "English (ENG)";
		case 1: return "Russkiy (RUS)"; //transliteration
		case 2: return "Espanol (SPN)";
		case 3: return "Deutsch (GRM)";
		case 4: return "Hanyu (CHN)"; //transliteration
		case 5: return "Bahasa Malaysia (MAL)";
		case 6: return "Bahasa Indonesia (IDN)";
		case 7: return "Francais (FRN)";
		case 8: return "Portugues Brasileiro (POR)";
		case 9: return "Thai (THA)";
		default: return "??";
	}
}

/*
 * verify that the choosen langtype is enable
 * return
 *  1 : langage enable
 * -1 : false range
 * -2 : disable
 */
int32 msg_checklangtype(int32 lang, bool display){
	uint16 test= (1<<(lang-1));
	if(!lang) return 1; //default english
	else if(lang < 0 || test > LANG_MAX) return -1; //false range
	else if (LANG_ENABLE&test) return 1;
	else if(display) {
		ShowDebug("Unsupported langtype '%d'.\n",lang);
	}
	return -2;
}
