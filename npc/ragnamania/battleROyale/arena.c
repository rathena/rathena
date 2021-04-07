// Ragnamania battleROyale
// by Biali
// v1.0

// Main Script Arena Controller
// *****************************************************
verus01,140,140,0	script	#ArenaController	-1,{
	function F_UnsetPoisonCells;
end;

OnStartArena:
	enablenpc strnpcinfo(0);
	copyarray $@playersInTheArena[0], $@br_queue[0],$@ARENA_START_NUM_PLAYERS;
	deletearray $@br_queue[0],$@ARENA_START_NUM_PLAYERS;
	debugmes "#ArenaController iniciado...";
	end;

OnPcLogOutEvent:
	if(.@index = inarray($@playersInTheArena,getcharid(0)) >= 0) {
		debugmes "ArenaController : " + strcharinfo(0,$@playersInTheArena[.@index]) + " deslogou.";
		deletearray $@playersInTheArena[.@index],1;
		
		set BR_ARENA_POINTS, BR_ARENA_POINTS -10;
		if(BR_ARENA_POINTS < 0)
			BR_ARENA_POINTS = 0;

		// Remove as skills especiais para a arena
		skill 411,0,SKILL_PERM_GRANT;	//TK_RUN
		skill 426,0,SKILL_PERM_GRANT;	//TK_HIGHJUMP
		skill 150,0,SKILL_PERM_GRANT;	//TF_BACKSLIDING
		skill 142,0,SKILL_PERM_GRANT; 	//First Aid

		//callfunc "ClearInventory",getcharid(0);

		if(strcharinfo(3) == $@ARENA_MAP_NAME$)
			if(getmapusers($@ARENA_MAP_NAME$) <= $@ARENA_MINIMUM_PLAYERS)
				donpcevent "#ArenaController::OnGameOver";

	} else if(.@index = inarray($@br_queue,getcharid(0)) >= 0) {
		deletearray $@br_queue[.@index],1;
		dispbottom "[ Battle ROyale ] You've left the queue. To join again type @queue",0xDD0000;
	}
	end;

OnPcKillEvent:
	if(strcharinfo(3) != $@ARENA_MAP_NAME$)
		end;
	if(.@index = inarray($@playersInTheArena[0],getcharid(0)) >= 0) { 
		mapannounce strnpcinfo(4),strcharinfo(0)+" has just killed "+rid2name(killedrid),bc_map,0xAA0000,0,18,0,0;

		set BR_ARENA_POINTS, BR_ARENA_POINTS + getvar(BR_ARENA_POINTS, getcharid(0,rid2name(killedrid)));
		dispbottom "You've got " + getvar(BR_ARENA_POINTS, getcharid(0,rid2name(killedrid))) + " Arena Points from " + rid2name(killedrid);
	}
	end;

OnPcDieEvent:
	if(strcharinfo(3) != $@ARENA_MAP_NAME$)
		end;
	if(.@index = inarray($@playersInTheArena[0],getcharid(0)) >= 0){
		deletearray $@playersInTheArena[.@index],1;
	}
	
	set BR_ARENA_POINTS, BR_ARENA_POINTS -10;
	if(BR_ARENA_POINTS < 0) {
		BR_ARENA_POINTS = 0;
	}

	// Remove as skills especiais para a arena
	skill 411,0,SKILL_PERM_GRANT;	//TK_RUN
	skill 426,0,SKILL_PERM_GRANT;	//TK_HIGHJUMP
	skill 150,0,SKILL_PERM_GRANT;	//TF_BACKSLIDING
	skill 142,0,SKILL_PERM_GRANT; 	//First Aid

	//Lets get the dead body location...
	getmapxy(.@map$, .@x, .@y, BL_PC);
	
	//And their inventory list...
	getinventorylist;

	//Now we transfer the list of items and their quantity to an array on our char
	//because getinvetorylist will generate the arrays on the dead char.
	for(.@c=0;.@c<@inventorylist_count;.@c++) {
		.@itemid = @inventorylist_id[.@c];
		setarray getd("@lb_"+getcharid(0)+"["+.@c+"]"),.@itemid;
		debugmes "Added 1x " + getitemname(.@itemid);
		//We cant forget to delete the items from the dead char:
		//delitem .@itemid,.@itemqt,getcharid(3,strcharinfo(0,.@cid));
		delitem .@itemid,1;
		
		// .@id++;
	}

	//Then we spawn the bag:
	monster .@map$,.@x,.@y,"Loot Bag",1351,1,strnpcinfo(0) + "::OnCollectLootBag",2;
	.@gid = $@mobid[0];

	//Now we have the loot bag GID so we create a new array with that GID to store
	//the deceased player's loot into. We also delete the char-based array
	copyarray getd(".lb_"+.@gid+"[0]"),getd("@lb_"+getcharid(0)+"[0]"),getarraysize(getd("@lb_"+getcharid(0)));
	deletearray getd("@lb_"+getcharid(0)+"[0]"),getarraysize(getd("@lb_"+getcharid(0)));

	warp $@LOBBY_MAP_NAME$, $@LOBBY_START_X, $@LOBBY_START_Y;
	recovery 0;
	
	if(getmapusers(strnpcinfo(4)) == 1)
		callsub OnGameOver;
	end;

OnCollectLootBag:
	//TODO : NAO DELETA ESSE CODIGO ATE TESTAR COM MAIS GENTE LOOTANDO CORPOS AO MESMO TEMPO
	// freeloop(1);
	// for(.@c=0;.@c<getarraysize(getd(".lb_"+killedgid));.@c++) {
	// 	if(!countitem(getelementofarray(getd(".lb_"+killedgid),.@c)))
	// 		getitem getelementofarray(getd(".lb_"+killedgid),.@c),1;
	// }
	// freeloop(0);
	// deletearray getd(".lb_"+killedgid+"[0]"), getarraysize(getd(".lb_"+killedgid));


	npcshopdelitem "DeadBody",512;
	freeloop(1);
	for(.@c=0;.@c<getarraysize(getd(".lb_"+killedgid));.@c++) {
		npcshopadditem "DeadBody",getelementofarray(getd(".lb_"+killedgid),.@c),0;
	}
	freeloop(0);
	callshop "DeadBody",1;
	deletearray getd(".lb_"+killedgid+"[0]"), getarraysize(getd(".lb_"+killedgid));

	end;

OnGameOver:
	//TODO : Sera seguro trocar isso pela variavel de players no mapa de cada instancia? .@playersInTheArena$[0]
	if(.@num = getmapunits(BL_PC,$@ARENA_MAP_NAME$,$@acc[0]) == 1 ) {
		debugmes "ArenaController : Game Over. total de players na $@playersInTheArena[0] : " + getarraysize($@playersInTheArena);

		if(getgmlevel(getcharid(0,rid2name($@acc[0]))) == 99)
			end;

		F_UnsetPoisonCells();
		callfunc "F_Dispell",$@acc[0];
		.@name$ = rid2name($@acc[0]);
		recovery 0,getcharid(0,.@name$);
		atcommand "#effect " +.@name$+ " 68";
		atcommand "#effect " +.@name$+ " 709";

		// GAME OVER WE HAVE A WINNER
		$arena_edition++;
		.@ordeal$ = callfunc("OrdealOf",$arena_edition);
		announce "The winner of the " + $arena_edition + .@ordeal$ + " edition of Ragnamania Battle ROyale is " + .@name$,bc_all;
		
		//TODO : Record this in the mysql with, date, time, etc

		sleep 5000;
		mapwarp $@ARENA_MAP_NAME$,$@LOBBY_MAP_NAME$,$@LOBBY_START_X,$@LOBBY_START_Y;

	} else if(.@num > 1) {
		end;
	}

	//TODO limpar variaveis incluindo as arrays de loot bags

	donpcevent "Battle ROyale : #TimeControl::OnDeactivate";
	donpcevent "Battle ROyale : #PoisonControl::OnDeactivate";
	disablenpc strnpcinfo(0);
	deletearray $@playersInTheArena[0],$@ARENA_START_NUM_PLAYERS;
	debugmes "Battle ROyale : Arena is available again";
	end;

OnInit:
	//deletearray $@br_queue[0],$@ARENA_START_NUM_PLAYERS;
	//eletearray $@playersInTheArena[0],$@ARENA_START_NUM_PLAYERS;
	sleep 100;
	F_UnsetPoisonCells();
	end;


	// Clear Poisonous Cells
	// *****************************************************
	function	F_UnsetPoisonCells	{
		setcell $@ARENA_MAP_NAME$,1,1,$@MAP_SIZE_X,$@MAP_SIZE_Y,CELL_POISON,0;
		debugmes "Battle ROyale : Limpando celulas envenenadas na sessao anterior...";
		return;
	}
}


verus01,1,1,1	script	#PoisonControl	-1,{
	function	ApplyPoison; function	UpdateMiniMapa;
	end;

OnDeactivate:
OnInit:
	stopnpctimer;
	disablenpc strnpcinfo(0);
	debugmes "#PoisonControl Desativado...";
	end;

OnStartArena:
	enablenpc strnpcinfo(0);
	initnpctimer;
	.k=0; // Controla o ID q cada viewpoint precisa ter e precisa ser unico;
	$@count = getmapunits(BL_PC,$@ARENA_MAP_NAME$,$@acc[0]);
	debugmes "#PoisonControl iniciado...";
	end;

OnTimer60000:
	ApplyPoison($@DISTANCE_FIRST_SHRINK);
	UpdateMiniMapa($@DISTANCE_FIRST_SHRINK);
	end;

OnTimer120000:
	ApplyPoison($@DISTANCE_SECOND_SHRINK);
	UpdateMiniMapa($@DISTANCE_SECOND_SHRINK);
	end;

OnTimer180000:
	stopnpctimer;
	ApplyPoison($@DISTANCE_THIRD_SHRINK);
	UpdateMiniMapa($@DISTANCE_THIRD_SHRINK);
	end;



	function	ApplyPoison	{
		debugmes "Battle ROyale : ApplyPoison : Executando...";
		.PoisonBorder = getarg(0);
		//$@count = getmapunits(BL_PC,$@ARENA_MAP_NAME$,$@acc[0]);
		
		if($@count <= $@ARENA_MINIMUM_PLAYERS) {
			stopnpctimer;
			end;
		}

		mapannounce $@ARENA_MAP_NAME$,"The map has shrunk",bc_map,0xAA00AA,0,24;

		debugmes "Battle ROyale : Atualizando celulas com veneno...";
		freeloop(1);
		for(.@a=.PoisonBorder; .@a<=150; .@a++) {
			drawacircle(.@a,150,150);
			for(.@i=0; .@i<360; .@i++) {
				if(checkcell("verus01",$@circle_x[.@i],$@circle_y[.@i],cell_chkpoison) == 0) {
					setcell $@ARENA_MAP_NAME$,$@circle_x[.@i],$@circle_y[.@i],$@circle_x[.@i],$@circle_y[.@i],CELL_POISON,1;
				}
			}
		}
		freeloop(0);

		return;
	}



	function	UpdateMiniMapa	{
		debugmes "Battle ROyale : Atualizando minimapa...";
		.PoisonBorder = getarg(0);
		
		freeloop(1);
		for(.@pl=0;.@pl<$@count;.@pl++) {
			attachrid($@acc[.@pl]);
			for(.@a=150; .@a>.PoisonBorder; .@a-=18) {
				drawacircle(.@a,150,150);
				for(.@i=0; .@i <360;.@i=.@i+5){
					.k++;
					viewpoint 1,$@circle_x[.@i],$@circle_y[.@i],.k,0xFF0000;
				}
			}
			detachrid;
		}
		freeloop(0);

		return;
	}

}




// Control Events in the Arena that are Time related
// *****************************************************
verus01,1,1,1	script	#TimeControl	-1,{
	end;

OnDeactivate:
OnInit:
	disablenpc strnpcinfo(0);
	end;

OnStartArena:
	enablenpc strnpcinfo(0);
	sleep 61000;
	debugmes "#TimeControl iniciado...";
	while($@count = getmapunits(BL_PC,$@ARENA_MAP_NAME$,$@acc[0]) <= $@ARENA_MINIMUM_PLAYERS) {	
		$@count = getmapunits(BL_PC,$@ARENA_MAP_NAME$,$@acc[0]);
		for(.@i=0; .@i<$@count;.@i++) {
			getmapxy(.@mapPC$, .@pcX, .@pcY, BL_PC,rid2name($@acc[.@i]));
			if(checkcell(.@mapPC$, .@pcX, .@pcY,CELL_CHKPOISON)) {
				if (getstatus(SC_BLEEDING,0,getcharid(0,rid2name($@acc[.@i]))) == 0) {
					sc_start SC_BLEEDING,5000,20,10000,SCSTART_NOAVOID,$@acc[.@i];
				}
				atcommand "#effect "+ rid2name($@acc[.@i]) + " 772";//" 923";
			} else 
				sc_end SC_BLEEDING,$@acc[.@i];
		}
		debugmes "#TimeControl : $@count = " + $@count;
		sleep 5000;
	}
	end;
}



-	script	#LootBoxes	-1,{
	end;

OnStartArena:
	freeloop(1);
	while (.@count < $@NUMBER_OF_LOOT_BOXES) {
		.@x = rand(10,($@MAP_SIZE_X - 10));
		.@y = rand(10,($@MAP_SIZE_Y - 10));

			if(checkcell($@ARENA_MAP_NAME$,.@x,.@y,cell_chkpass)) {
				monster $@ARENA_MAP_NAME$,.@x,.@y,"Box",1351,1,strnpcinfo(0) + "::OnCollection";
//				viewpoint 0,.@x,.@y,.@count,0xFF0000; //Comment this one to do not show loot boxes in the map
				.@count++;
			}
	}
	freeloop(0);
	end;

OnCollection:
	//.@id = getrandgroupitem(IG_BattleROyale_Guns),1,1,1; 	// Guns
	.@id = getelementofarray($@LIST_OF_GUNS,rand(getarraysize($@LIST_OF_GUNS)));
	if(!countitem(.@id)) {
		if(select("Equipar ^DD0000"+getitemname(.@id)+"^000000:Cancelar") == 1) {
			getitem .@id,1;
			equip .@id;
		}
	}
//	getrandgroupitem(101),10;		// Ammo
	end;
}


-	shop	DeadBody	-1,512:-1;


verus01,131,186,1	script	Testando	644,{
	sleep2 10000;
	donpcevent "#PoisonControl::OnTimer60000";
	end;
}
