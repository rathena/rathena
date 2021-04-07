// Ragnamania battleROyale
// by Biali
// v1.0


// MAPFLAGS //
//*************************************************
verus01	mapflag	nochat
verus01	mapflag	novending
verus01	mapflag	partylock
verus01	mapflag	guildlock
verus01	mapflag	nodrop
verus01	mapflag	noexp
verus01	mapflag	nosave
verus01	mapflag	pvp
verus01	mapflag	pvp_noparty
verus01	mapflag	pvp_noguild
verus01	mapflag	pvp_nocalcrank
verus01	mapflag	skillnorequirements	15
verus01	mapflag	noloot
verus01	mapflag	nomobloot
verus01	mapflag	nomvploot

prontera	mapflag	loadevent
prontera	mapflag	skillnorequirements	15


// System Constants and Setup
// ****************************************************
-	script	Setup	-1,{
end;

OnInit:
	$@LOBBY_MAP_NAME$ 			= 	"prontera";
	$@LOBBY_START_X				=	151;
	$@LOBBY_START_y				=	149;
	$@ARENA_MAP_NAME$			=	"verus01";
	$@ARENA_START_NUM_PLAYERS 	=	2;
	$@ARENA_MINIMUM_PLAYERS		=	1;
	$@DISTANCE_FIRST_SHRINK		=	90;
	$@DISTANCE_SECOND_SHRINK	=	70;
	$@DISTANCE_THIRD_SHRINK		=	40;
	$@MAP_SIZE_X				=	300;
	$@MAP_SIZE_Y				=	300;
	$@NUMBER_OF_LOOT_BOXES		=	50;
	$@SHRINK_INSTERVAL			=	60000;

	setarray $@LIST_OF_GUNS[0],	13100,13102,13104,13106,13107,13108,13112,13113,13150,13151,13153,13154,13155,13156,13157,13158,13160,13162,13163,13165,13167,13168,13170;
	setarray $@COSTUMES[0],		1786,1796,1668,1839,1444,1264;
	setarray $@COSTUMES_COST[0],2000,2000,2000,2000,2000,2000;

	//Clear vars just in case...
	deletearray $@lobbyqueue[0],5000;

	end;
}


// Control Player Events
// ****************************************************
-	script	PlayerController	-1,{
end;

OnPCLoginEvent:
	//Exploit Control
	//set manner, 0;

	//First Login
	if(BaseLevel < 99 && #BATTLEROYALE) {
		// jobchange 24;
		// BaseLevel = 99;
		// //JobLevel = 70;
		// SkillPoint = 0;
		// StatusPoint = 0;
		// atcommand "@allstats";
		// recalculatestat;
		// //save "new_1-1", 143,114;
		// //Initial Skills
		// //skill 4001,1,SKILL_PERM_GRANT;	//Capture Net
		// //skill 4002,1,SKILL_PERM_GRANT;	//Whetstone
		// //skill 142,1,SKILL_PERM_GRANT; 	//First Aid
		// //skill 143,1,SKILL_PERM_GRANT; 	//Play Dead
		// skill 261,1,SKILL_PERM_GRANT;	//MO_CALLSPIRITS
		// skill 264,1,SKILL_PERM_GRANT;	//MO_BODYRELOCATION
		// skill 411,10,SKILL_PERM_GRANT;	//TK_RUN
		// skill 426,5,SKILL_PERM_GRANT;	//TK_HIGHJUMP
		// skill 150,1,SKILL_PERM_GRANT;	//TF_BACKSLIDING
		// skill 151,1,SKILL_PERM_GRANT; 	//Find Stone
		// skill 152,1,SKILL_PERM_GRANT; 	//Stone Fling
		recovery(0,getcharid(0));
		callfunc "ClearInventory", getcharid(0);
	}

	//Get the player in the queue
	// if(inarray($@br_queue,getcharid(0)) == -1) {
	// 	dispbottom "[ Battle ROyale ] Voc� n�o est� na fila para o Battle ROyale. Para entrar na fila digite @queue";
	// }
	end;

OnPCLoadMapEvent:
	//if(strcharinfo(3) == $@LOBBY_MAP_NAME$) {
	if(getmapflag(strcharinfo(3),MF_TOWN) == 1) {
		// getinventorylist;
		// callfunc "ClearInventory",getcharid(0);
		// //sc_end SC_POISON;
		// sc_end SC_BLEEDING;
		// undisguise;
		// recovery(0,getcharid(0));
		// sleep2 5000;
		if (inarray($@br_queue,getcharid(0)) == -1) {
			dispbottom "[ Battle ROyale ] You are not in the queue. To join type @queue";
			//callfunc "QueueForArena";
		}
	}
	end;

OnInit:
	bindatcmd "distance","PlayerController::OnAtcommand";
	bindatcmd "queue","PlayerController::OnAtcommand";
	end;

OnAtcommand:
	if(.@atcmd_command$ == "@distance") {
		getmapxy(.@mapPC$, .@pcX, .@pcY, BL_PC);
		if(.@index = inarray($@playersInTheArena[0],getcharid(0)) >= 0) {
			.@distance = callfunc("CalcDistance",getcharid(0));
			dispbottom "The distance to the center is : " + .@distance;
			dispbottom "Poisoned Cell " + (checkcell(.@mapPC$,.@pcX,.@pcY,CELL_CHKPOISON)?"YES":"NO");
		}
	} else if(.@atcmd_command$ == "@queue") {
		if(inarray($@br_queue,getcharid(0)) == -1 ) {
			if(strcharinfo(3) == $@ARENA_MAP_NAME$) {
				end;
			}
			if(.@c < $@ARENA_START_NUM_PLAYERS) {
				setarray $@br_queue[getarraysize($@br_queue)], getcharid(0);
				callfunc "UpdateQueueStatus";
			} else {
				dispbottom "[ Battle ROyale ] The waiting list is full. Please wait for the next match.";
				end;
			}

			if (getarraysize($@br_queue) == $@ARENA_START_NUM_PLAYERS) {
				freeloop(1);
				detachrid;
				while(getmapusers($@ARENA_MAP_NAME$) == 0) {
					sleep 5000;
					for (.@i=0; .@i < $@ARENA_START_NUM_PLAYERS; .@i++) {
						attachrid(getcharid(3,strcharinfo(0,$@br_queue[.@i])));
						if(BR_ACTIVE_COSTUME)
							disguise BR_ACTIVE_COSTUME;
						callfunc "F_Dispell";
						//callfunc "ClearInventory",getcharid(0);
						Hp = 20000;
						skill 411,5,SKILL_PERM_GRANT;	//TK_RUN
						skill 426,3,SKILL_PERM_GRANT;	//TK_HIGHJUMP
						skill 150,1,SKILL_PERM_GRANT;	//TF_BACKSLIDING
						skill 142,1,SKILL_PERM_GRANT; 	//First Aid
						JobLevel = 1;
						ResetSkill;
						SkillPoint = 0;
						warp $@ARENA_MAP_NAME$,0,0;
						detachrid;
					}
					donpcevent "#ArenaController::OnStartArena";
					donpcevent "#TimeControl::OnStartArena";
					donpcevent "#PoisonControl::OnStartArena";
					donpcevent "#LootBoxes::OnStartArena";
					end;
				}
				freeloop(0);
			}

		} else {
			deletearray $@br_queue[.@index],1;
			dispbottom "[ Battle ROyale ] You've left the queue. To join again type @queue",0xDD0000;
		}
	}
	end;
}

-	script	OnPCLoginEvent	-1,{
	end;
OnPCLoginEvent:
	if(class == 24) {
		skill 411,0,SKILL_PERM_GRANT;	//TK_RUN
		skill 426,0,SKILL_PERM_GRANT;	//TK_HIGHJUMP
		skill 150,0,SKILL_PERM_GRANT;	//TF_BACKSLIDING
		skill 142,0,SKILL_PERM_GRANT; 	//First Aid
	}
	end;
}


