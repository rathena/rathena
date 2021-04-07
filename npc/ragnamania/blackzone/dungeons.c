

// -	script	dg_entrance	-1,{
// 	.@i$ = callfunc("bz_get_i_name");
// 	.@main$ = callfunc("bz_remove_space",.@i$); // for the array name containing the DG unique confs

// 	@bz_respawn_map$ = getelementofarray(getd("$@"+.@main$+"$"),0);
// 	@bz_respawn_x = atoi(getelementofarray(getd("$@"+.@main$+"$"),1));
// 	@bz_respawn_y = atoi(getelementofarray(getd("$@"+.@main$+"$"),2));

// 	if(instance_id(IM_PARTY) && instance_live_info(ILI_NAME, instance_id(IM_PARTY)) == .@i$)
// 		callfunc "bz_i_enter";
// 	else if((!instance_id(IM_PARTY) && is_party_leader(getcharid(1))) || getcharid(1) == 0) {
// 		callfunc "bz_i_create";
// 		if(instance_id(IM_PARTY))
// 			callfunc "bz_i_enter";
// 	}
// 	end;
// }


// // This npc is instanced into every dungeon
// -	script	dg_exit	-1,{
// 	if(instance_id(IM_PARTY)) {
// 		instance_destroy();
// 	} else {
// 		warp @bz_respawn_map$,@bz_respawn_x,@bz_respawn_y;
// 	}
// 	end;

// OnInstanceInit:
// 	disablenpc instance_npcname(strnpcinfo(0));
// 	end;

// OnInstanceDestroy:
// 	debugmes "Instance destroy : " + instance_id();
// 	//TODO : Must clear the arrays when a instance is destroyed
// 	.@index = inarray($@bz_i_dg_id,instance_id());
// 	deletearray $@bz_i_dg_id[.@index],1;
// 	deletearray $@bz_i_dg_n$[.@index],1;
// 	end;
// }




// // Dungeon Entrances
// ars_fild01,82,148,0	duplicate(dg_entrance)	#ars_fild01	45,1,1
// ars_fild04,203,90,0	duplicate(dg_entrance)	#ars_fild04	45,1,1

// // Dungeon Exits
// ars_dun02,125,117,1	duplicate(dg_exit)	#CryptofGomorrah	45,1,1