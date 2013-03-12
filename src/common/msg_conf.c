#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "malloc.h"
#include "msg_conf.h"
#include "showmsg.h"

//-----------------------------------------------------------
// Return the message string of the specified number by [Yor]
//-----------------------------------------------------------

const char* _msg_txt(int msg_number,int size, char ** msg_table)
{
    if (msg_number >= 0 && msg_number < size &&
	    msg_table[msg_number] != NULL && msg_table[msg_number][0] != '\0')
	return msg_table[msg_number];

    return "??";
}

/*==========================================
 * Read Message Data
 *------------------------------------------*/
int _msg_config_read(const char* cfgName,int size, char ** msg_table)
{
    int msg_number;
    char line[1024], w1[1024], w2[1024];
    FILE *fp;
    static int called = 1;

    if ((fp = fopen(cfgName, "r")) == NULL) {
	ShowError("Messages file not found: %s\n", cfgName);
	return 1;
    }

    if ((--called) == 0)
	memset(msg_table, 0, sizeof (msg_table[0]) * size);

    while (fgets(line, sizeof (line), fp)) {
	if (line[0] == '/' && line[1] == '/')
	    continue;
	if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
	    continue;

	if (strcmpi(w1, "import") == 0)
	    _msg_config_read(w2,size,msg_table);
	else {
	    msg_number = atoi(w1);
	    if (msg_number >= 0 && msg_number < size) {
		if (msg_table[msg_number] != NULL)
		    aFree(msg_table[msg_number]);
		msg_table[msg_number] = (char *) aMalloc((strlen(w2) + 1) * sizeof (char));
		strcpy(msg_table[msg_number], w2);
	    }
	}
    }

    fclose(fp);
    ShowInfo("Finished reading %s.\n",cfgName);

    return 0;
}

/*==========================================
 * Cleanup Message Data
 *------------------------------------------*/
void _do_final_msg(int size, char ** msg_table){
    int i;
    for (i = 0; i < size; i++)
	aFree(msg_table[i]);
}
