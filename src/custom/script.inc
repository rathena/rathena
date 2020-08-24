// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

//eduardo
BUILDIN_FUNC(partner_create)
{
	struct map_session_data *sd;
	int class_, contract_time;

	if( !script_rid2sd(sd) || sd->md || sd->status.mer_id != 0 )
		return SCRIPT_CMD_SUCCESS;

	class_ = script_getnum(st,2);

	contract_time = script_getnum(st,3);
	partner_create(sd, class_, contract_time);

	return SCRIPT_CMD_SUCCESS;
}