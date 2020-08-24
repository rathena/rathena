// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

//eduardo
ACMD_FUNC(clonea){
	//char c[15];		
	char name[15];		//char type[15];
	int x=0,y=0,flag=0,master=0,i=0;
	struct map_session_data *pl_sd=NULL;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(sd->fd,msg_txt(sd,1323)); // You must enter a player name or ID.
		return 0;		}
	if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL) {
	 	clif_displaymessage(fd, msg_txt(sd,3));	// Character not found.			
	 	return 0;		}
	if(pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
		clif_displaymessage(fd, msg_txt(sd,126));	// Cannot clone a player of higher GM level than yourself.			
		return 0;		}

	flag = 2;

	if(pc_isdead(sd)){	clif_displaymessage(fd, msg_txt(sd,129+flag*2)); return 0; }

	master = sd->bl.id;

	//ShowMessage("\n %d \n", map_nick2sd(atcmd_player_name,true)->status.account_id);

	do { x = sd->bl.x + (rnd() % 10 - 5);	y = sd->bl.y + (rnd() % 10 - 5); } while (map_getcell(sd->bl.m,x,y,CELL_CHKNOPASS) && i++ < 10);
	if (i >= 10) {	x = sd->bl.x;	y = sd->bl.y; }

	sscanf(message, "%15s", name);

	if((x = mob_clone_spawn2(pl_sd, sd->bl.m, x, y, "", master, MD_NONE, flag?1:0, 0, "default", 
		map_nick2sd(atcmd_player_name,true)->status.char_id
	)) > 0) {
		clif_displaymessage(fd, "an MD_NONE 2 mercenary assistant is spawned");	
		return 0;		}
	return 0;
}

ACMD_FUNC(clonemp){
	//char c[15];		
	char name[15];		//char type[15];
	int x=0,y=0,flag=0,master=0,i=0;
	struct map_session_data *pl_sd=NULL;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(sd->fd,msg_txt(sd,1323)); // You must enter a player name or ID.
		return 0;		}
	if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL) {
	 	clif_displaymessage(fd, msg_txt(sd,3));	// Character not found.			
	 	return 0;		}
	if(pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
		clif_displaymessage(fd, msg_txt(sd,126));	// Cannot clone a player of higher GM level than yourself.			
		return 0;		}

	flag = 1;

	if(pc_isdead(sd)){	clif_displaymessage(fd, msg_txt(sd,129+flag*2)); return 0; }

	//master = sd->bl.id;

	do { x = sd->bl.x + (rnd() % 10 - 5);	y = sd->bl.y + (rnd() % 10 - 5); } while (map_getcell(sd->bl.m,x,y,CELL_CHKNOPASS) && i++ < 10);
	if (i >= 10) {	x = sd->bl.x;	y = sd->bl.y; }

	sscanf(message, "%15s", name);

	if((x = mob_clone_spawn2(pl_sd, sd->bl.m, x, y, "", master, MD_AGGRESSIVE, flag?1:0, 0, "default", 
		map_nick2sd(atcmd_player_name,true)->status.char_id)) > 0) {
		clif_displaymessage(fd, "an MD_AGGRESSIVE 1 masterless [marinespher & PVP]");	
		return 0;		}
	return 0;
}

ACMD_FUNC(clonepe){
	//char c[15];		
	char name[15];		//char type[15];
	int x=0,y=0,flag=0,master=0,i=0;
	struct map_session_data *pl_sd=NULL;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(sd->fd,msg_txt(sd,1323)); // You must enter a player name or ID.
		return 0;		}
	if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL) {
	 	clif_displaymessage(fd, msg_txt(sd,3));	// Character not found.			
	 	return 0;		}
	if(pc_get_group_level(pl_sd) > pc_get_group_level(sd)) {
		clif_displaymessage(fd, msg_txt(sd,126));	// Cannot clone a player of higher GM level than yourself.			
		return 0;		}

	flag = 1;

	if(pc_isdead(sd)){	clif_displaymessage(fd, msg_txt(sd,129+flag*2)); return 0; }

	//master = sd->bl.id;

	do { x = sd->bl.x + (rnd() % 10 - 5);	y = sd->bl.y + (rnd() % 10 - 5); } while (map_getcell(sd->bl.m,x,y,CELL_CHKNOPASS) && i++ < 10);
	if (i >= 10) {	x = sd->bl.x;	y = sd->bl.y; }

	sscanf(message, "%15s", name);

	if((x = mob_clone_spawn2(pl_sd, sd->bl.m, x, y, "", master, MD_NONE, flag?1:0, 0, "default", 
		map_nick2sd(atcmd_player_name,true)->status.char_id)) > 0) {
		clif_displaymessage(fd, "an MD_NONE 1 masterless (PVP) spawned");
		return 0;		}
	return 0;
}