
prontera,153,180,0	script	mapduplicate_test	444,{
	switch( select( "Create new duplicate to prontera", "Names list" ) ) {
	case 1:
		debugmes "Instance ID: " + instance_create("prontera instance", IM_NONE);
		end;
	case 2:
		.@size = instance_list(IM_NONE, "prontera");	// save the instance IDs in .@instance_list and return the size list
		if (.@size == 0) {
			mes "No maps on the list yet";
			end;
		}
		mes "Select a map to warp to it!";
		for ( .@i = 0; .@i < .@size; ++.@i )
			.@menu$ += instance_mapname("prontera", .@instance_list[.@i]) + ":";
		.@s = select(.@menu$) -1;
		if (instance_enter("prontera instance",155,183,getcharid(0),.@instance_list[.@s]) != IE_OK) {
			mes "The map doesn't exist anymore.";
			end;
		}
	}
}

//this for testing the NoNpc and NoMapflag options
prontera,155,179,0	script	mapduplicate_test2	444,{
	if(select("nodata:data") == 1){
		switch( select( "Create:Enter:Destroy:info" ) ) {
		case 1:
			$instance_id__ = instance_create("prontera instance", IM_NONE);
			debugmes "Instance ID: " + $instance_id__;
			end;
		case 2:
			if (instance_enter("prontera instance",155,183,getcharid(0),$instance_id__ ) != IE_OK) {
				mes "The map doesn't exist anymore.";
				end;
			}
			end;
		case 3:
			instance_destroy $instance_id__ ;
			end;
		case 4:
			debugmes instance_mapname("prontera",$instance_id__ );
			end;
		}
	}else{
		
		switch( select( "Create:Enter:Destroy:info" ) ) {
		case 1:
			$instance_id__2 = instance_create("prontera instance2", IM_NONE);
			debugmes "Instance ID: " + $instance_id__2;
			end;
		case 2:
			if (instance_enter("prontera instance2",155,183,getcharid(0),$instance_id__2) != IE_OK) {
				mes "The map doesn't exist anymore.";
				end;
			}
			end;
		case 3:
			instance_destroy $instance_id__2;
			end;
		case 4:
			debugmes instance_mapname("prontera",$instance_id__2);
			end;
		}
	}
}

prontera,151,178,4	script	PVP_LEADER	444,{
	.@s = select("PVP Room 1 (" + getmapusers($PVP_ROOM_1$) + "):PVP Room 2 (" + getmapusers($PVP_ROOM_2$) + "):PVP Room 3 (" + getmapusers($PVP_ROOM_3$) + ")");
	warp getd("$PVP_ROOM_" + .@s + "$"),0,0;
	end;
}

-	script	MapDuplicateList	-1,{
OnInit:

	if(instance_mapname("guild_vs3",$instance_id) == ""){// if the map name was empty than the instance dose not exist
		$instance_id = instance_create("pvp system", IM_NONE);
		$PVP_ROOM_1$ = instance_mapname("guild_vs3", $instance_id);
		$PVP_ROOM_2$ = instance_mapname("pvp_n_1-5", $instance_id);
		$PVP_ROOM_3$ = instance_mapname("pvp_n_1-3", $instance_id);
		debugmes "instance has been created";
	}

	//guild_vs3
	setmapflag $PVP_ROOM_1$,mf_nomemo;
	setmapflag $PVP_ROOM_1$,mf_noteleport;
	setmapflag $PVP_ROOM_1$,mf_nowarp;
	setmapflag $PVP_ROOM_1$,mf_nogo;
	setmapflag $PVP_ROOM_1$,mf_nosave;
	setmapflag $PVP_ROOM_1$,mf_nobranch;
	setmapflag $PVP_ROOM_1$,mf_nopenalty;
	setmapflag $PVP_ROOM_1$,mf_nozenypenalty;
	setmapflag $PVP_ROOM_1$,mf_pvp;
	setmapflag $PVP_ROOM_1$,mf_pvp_noguild;
	setmapflag $PVP_ROOM_1$,mf_novending;
	
	//pvp_n_1-5
	setmapflag $PVP_ROOM_2$,mf_nomemo;
	setmapflag $PVP_ROOM_2$,mf_noteleport;
	setmapflag $PVP_ROOM_2$,mf_nowarp;
	setmapflag $PVP_ROOM_2$,mf_nogo;
	setmapflag $PVP_ROOM_2$,mf_nosave;
	setmapflag $PVP_ROOM_2$,mf_nobranch;
	setmapflag $PVP_ROOM_2$,mf_nopenalty;
	setmapflag $PVP_ROOM_2$,mf_nozenypenalty;
	setmapflag $PVP_ROOM_2$,mf_pvp;
	setmapflag $PVP_ROOM_2$,mf_pvp_noguild;
	setmapflag $PVP_ROOM_2$,mf_novending;
	
	//pvp_n_1-3
	setmapflag $PVP_ROOM_3$,mf_nomemo;
	setmapflag $PVP_ROOM_3$,mf_noteleport;
	setmapflag $PVP_ROOM_3$,mf_nowarp;
	setmapflag $PVP_ROOM_3$,mf_nogo;
	setmapflag $PVP_ROOM_3$,mf_nosave;
	setmapflag $PVP_ROOM_3$,mf_nobranch;
	setmapflag $PVP_ROOM_3$,mf_nopenalty;
	setmapflag $PVP_ROOM_3$,mf_nozenypenalty;
	setmapflag $PVP_ROOM_3$,mf_pvp;
	setmapflag $PVP_ROOM_3$,mf_pvp_noguild;
	setmapflag $PVP_ROOM_3$,mf_novending;
end;
}




/*
prontera,153,180,0	script	mapduplicate_test	444,{
	switch( select( "Create new duplicate to prontera", "Names list" ) ) {
	case 1:
		debugmes "Instance ID: " + instance_create("prontera instance", IM_NONE);
		end;
	case 2:
		.@size = instance_list(IM_NONE, "prontera");	// save the instance IDs in .@instance_list and return the size list
		if (.@size == 0) {
			mes "No maps on the list yet";
			end;
		}
		mes "Select a map to warp to it!";
		for ( .@i = 0; .@i < .@size; ++.@i )
			.@menu$ += instance_mapname("prontera", .@instance_list[.@i]) + ":";
		.@s = select(.@menu$) -1;
		if (instance_enter("prontera instance",155,183,getcharid(0),.@instance_list[.@s]) != IE_OK) {
			mes "The map doesn't exist anymore.";
		end;
	}
}
*/

/*==========================================
 * set the array passed as list of the live instances's ids.
 * .@size = getinstanceidlist(.@s);
 * .@s = an array of the instances ids
 * .@size = the array size
 *------------------------------------------*/
/*
BUILDIN_FUNC(getinstanceidlist)
{
	struct script_data* data = script_getdata(st, 2);

	if (!data_isreference(data))
	{
		ShowError("buildin_getinstanceidlist: not a variable\n");
		script_reportdata(data);
		st->state = END;
		return SCRIPT_CMD_FAILURE;
	}

	struct map_session_data* sd = NULL;
	int32 idx, id;
	const char* name;
	id = reference_getid(data);
	idx = reference_getindex(data);
	name = reference_getname(data);

	if (is_string_variable(name)) {
		ShowError("buildin_getinstanceidlist: Cannot use a string variable '%s'.\n", name);
		return SCRIPT_CMD_FAILURE;
	}

	if (not_server_variable(*name) && !script_rid2sd(sd)) {
		ShowError("buildin_getinstanceidlist: Cannot use a player variable '%s' if no player is attached.\n", name);
		return SCRIPT_CMD_FAILURE;
	}

	int i = 0;
	for (const auto& it : instances) {
		std::shared_ptr<s_instance_data> idata = it.second;
		if (!idata || idata->map.empty())
			continue;
		else {
			std::shared_ptr<s_instance_db> db = instance_db.find(idata->id);
			//mapreg_setreg(reference_uid(add_str("$@instance_id"), i), idata->id);
			set_reg_num(st, sd, reference_uid(id, idx + i), name, db->id, reference_getref(data));
			i++;
		}
	}
	script_pushint(st, i);
	return SCRIPT_CMD_SUCCESS;
}
*/