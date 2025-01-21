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
/*==========================================
 * @automine (Hyroshima)
 *----------------------------------------*/
TIMER_FUNC(automine_timer)
{
    map_session_data *sd = map_id2sd(id);
	
    if(sd != NULL)
	{
		int64 mining = pc_readreg(sd,reference_uid(add_str("@mining"),0));
		int autofarm = static_cast<int>(pc_readreg(sd,reference_uid(add_str("@autofarm"),0)));
		
        if(sd)
		{
			if(!autofarm || sd->vender_id || sd->state.trading || sd->state.storage_flag || sd->npc_id)
			{
				pc_setreg(sd, add_str("@autofarm"),0);
				pc_setreg(sd, add_str("@autofarm_call"),0);
			}
			else if(!mining)
			{
				npc_event(sd,"MiningSYS::OnSMsysAutoMine",0);
				add_timer(gettick()+6000,automine_timer,sd->bl.id,0);
			}
			else
				add_timer(gettick()+(mining?10000:3000),automine_timer,sd->bl.id,0);
		}
	}
    return 0;
}
//==
ACMD_FUNC(automine)
{
	pc_setreg(sd, add_str("@autofarm"), ( pc_readreg(sd,reference_uid(add_str("@autofarm"),0)) ? 0 : 1 ) );
	
	if(pc_readreg(sd,reference_uid(add_str("@autofarm"),0)))
	{
		clif_displaymessage(fd,"Auto-mine ON");
		add_timer(gettick()+1500,automine_timer,sd->bl.id,0);
	}
	else
		clif_displaymessage(fd,"Auto-mine OFF");

return 0;
}