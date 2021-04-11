//===== rAthena Script =======================================
//= Battleground Extended + : Poring Ball
//===== By: ==================================================
//= Grenat
//===== Description: =========================================
//= Battleground Extended + scripts.
//===== Additional Comments: =================================
//= Adapted version of Soccer game for for Ragnarok Online
//= Originally made in 2006. Adaptation for rAthena 2020.
//= This is a free release.
//= Please do NOT take ownership.
//= Red Team -> East
//= Blue Team -> West
//= Special thanks to Olrox for the beautiful map
// ====	Documentation: =======================================
// [Skill Push]:
// A Player can push its opponent using the item Push
// When an opponent is pushed, there is a chance to fall (Sit)
// and not moving for one second. CD 6 sec
//
// [Skill Shoot]:
// A Player can target a specific clicked cell to throw the
// ball ! When the item Shoot is used, player has 3 sec
// to decide where to throw the ball if the ball is still
// at 2 cells distance minimum. CD 6 sec
//
// [Blockage]
// Sitting while being on the poring ball way will STOP
// the ball from going further
//
// [Attributes Effects]:
// STR/INT affect Poring Ball distance when hit by STR/INT%
// AGI affect Poring Ball walking speed by AGI/3%
// DEX affect chance of hitting the ball (you can "miss") by DEX/3%
// VIT reduces chance to be pushed by VIT% and 50+VIT/3% chance
//     to stop the ball when sitting
// LUK give chance to critical (distance +5, Speed -20)
//
// [PVP]:
// ON/OFF, it's up to you. PVP activates penalty feature
// mapflag noskill activated (doesn't allow skills, up to you)
//
// [Penalty]:
// Killing someone results in a penalty.
// The ball will go in the corner of the killer member team
// The opposite team will have 20 sec to shoot at the ball.
//
// Everything is configurable (See Configuration)
//============================================================

// ===========================================================
// Configuration/Main scripts.
// ===========================================================
-	script	Poringball_Config	-1,{
OnInit:
	//= [Main Configs]
	.speed = 120;							// Base walking speed of the poring ball (Default: 120)
	.distance = 4;							// Base amount of cells the ball will move when touched (Default: 4)
	.chance = 66;							// Base percentage of chance to hit the ball (Default: 66)
	.pvp = 1;							// PVP mode (1 = PVP on, 0 = PVP off)
	.blockage = 1;							// Blockage mode: 'Sitting' can stop the ball (1 = Activated, 0 = Deactivated)
	.penalty = 1;							// Penalty mode (1 = Activated, 0 = Deactivated)
	.sound_poring = 1;						// Some rAthena version doesn't make walking sound of poring (1 = Activated, 0 = Already have sound)
	.skill_push_cells = 3;						// Skill Push how many cells away (Default: 3)
	.skill_push_chance = 25;					// Skill Push percentage of making targets sit for 1 sec (Default: 25)
	.skill_push_sit_time = 1;					// Skill Push sit down duration in seconds (Default: 1)

	//= [Olrox Poring Ball Map]
	// setarray .ball_center,78,79;					// Center
	// setarray .map_corner,41,116,49,108;				// Corners
	// setarray .map_surface,33,48,78,108,79,48,123,109;		// Map Surface
	// setarray .map_goal,72,85;					// Goal lenght
	// setarray .ball_corner1,36,107;					// Corner 1
	// setarray .ball_corner2,122,108;					// Corner 2
	// setarray .respawn,24,21,130,133;

	//= [Grenat Poring Ball Map]
	// db/battleground.yml Spawn Blue: 83,70. Spawn Red: 114,70	// Coordination of both team to modified
	setarray .ball_center,99,70;					// Center
	setarray .map_corner,24,175,24,117;				// Corners
	setarray .map_goal,62,77;					// Goal lenght
	setarray .ball_corner1,42,70;					// Corner 1
	setarray .ball_corner2,157,70;					// Corner 2
	MoveNPC "Healer#ball1",26,24;					// Healer
	MoveNPC "Healer#ball2",175,119;					// Healer
	MoveNPC "Return#ball2",30,24;					// Return
	MoveNPC "Return#ball1",170,119;					// Return
	MoveNPC "Poring Ball",99,70;					// Poring Ball
	MoveNPC "Blue Goal#poring1",24,77;				// Blue Goal 1
	MoveNPC "Blue Goal#poring2",24,62;				// Blue Goal 1
	MoveNPC "Red Goal#poring3",175,77;				// Blue Goal 1
	MoveNPC "Red Goal#poring4",175,62;				// Blue Goal 1
	setarray .respawn,83,70,114,70;

	end;

OnTeam1Active:
	warp "soccer3",.respawn[0],.respawn[1];
	end;
OnTeam2Active:
	warp "soccer3",.respawn[2],.respawn[3];
	end;

OnTeamDie1:
OnTeamDie2:
	if($@poringball != 3)||(!.penalty) end;
	if(getcharid(4) == $@BG_Team201) {
		mapannounce "soccer3", "Penalty for Red team ! Blue team kick at the red team's corner",bc_blue;
		$@poringballnext_teamid = $@BG_Team201;
		donpcevent "Poring Ball::OnPenaltyBlue";
	} else {
		mapannounce "soccer3", "Penalty for Blue team ! Red team kick at the blue team's corner",bc_blue;
		$@poringballnext_teamid = $@BG_Team202;
		donpcevent "Poring Ball::OnPenaltyRed";
	}
	end;
	
OnPoringBallQuit:
	if (getbattleflag("feature.bgqueue"))
		bg_desert;
	else
		bg_leave;
	end;
	
OnReady:
	$@BG20 = 1;
	bg_warp $@BG_Team201,"soccer3",.respawn[0],.respawn[1];
	bg_warp $@BG_Team202,"soccer3",.respawn[2],.respawn[3];
	$@poringball = 2;
	$@poringballnext_teamid = 0;
	$@score_poring_red = 0;
	$@score_poring_blue = 0;
	bg_updatescore "soccer3",$@score_poring_blue,$@score_poring_red;
	MoveNPC "Poring Ball",.ball_center[0],.ball_center[1];
	viewpointmap "soccer3",1, .ball_center[0], .ball_center[1], 1, 0xffffff;
	initnpctimer;
	end;

OnTimer1000:
	mapannounce "soccer3","[Poring Ball]: Start of the match in 10 seconds.",0;	
	end;
OnTimer4500:
	mapannounce "soccer3", "[Poring Ball]: Start of the match in 5 seconds.",0;	
	end;
OnTimer10000:
	mapannounce "soccer3","[Poring Ball]: Match has just begun !",0;
	mapannounce "soccer3","[Received] : Push and Shoot item skills",0;
	bg_warp $@BG_Team201,"soccer3",35,108;
	bg_warp $@BG_Team202,"soccer3",122,49;
	$@poringball = 3;
	getmapunits(BL_PC,"soccer3",.@array);
	for(.@i = 0; .@i < getarraysize(.@array); .@i++) {
		if(!countitem("SKILL_SHOOT",.@array[.@i])) getitem "SKILL_SHOOT",1,.@array[.@i];
		if(!countitem("SKILL_PUSH",.@array[.@i])) getitem "SKILL_PUSH",1,.@array[.@i];
		if(!.pvp) setpcblock PCBLOCK_ATTACK,1,.@array[.@i];
		//sc_start SC_TIMERDOWN,361000,360,10000,SCSTART_NOAVOID,.@array[.@i];
	}
	end;
OnTimer190000:
	mapannounce "soccer3", "[Poring Ball]: Half-time !",bc_map;	
	end;
OnTimer370000:
	mapannounce "soccer3", "[Poring Ball]: End of the match !",bc_map;	
	$@poringball = 4;	
	$@BG_Status = 2;
	bg_reserve "soccer3", true;
	end;
OnTimer376000:
	MoveNPC "Poring Ball",.ball_center[0],.ball_center[1];
	stopnpctimer;
	if ($@score_poring_blue<$@score_poring_red) {
		mapannounce "soccer3", "Congrats red team !!",bc_map;
		callfunc "BG_get_Rewards",$@BG_Team201,7829,1;
		callfunc "BG_get_Rewards",$@BG_Team202,7829,5;
	} else if ($@score_poring_blue>$@score_poring_red) {
		mapannounce "soccer3", "Congrats blue team !!",bc_map;	
		callfunc "BG_get_Rewards",$@BG_Team201,7829,5;
		callfunc "BG_get_Rewards",$@BG_Team202,7829,1;
	} else {
		mapannounce "soccer3", "Draw !!",bc_map;	
		callfunc "BG_get_Rewards",$@BG_Team201,7829,1;
		callfunc "BG_get_Rewards",$@BG_Team202,7829,1;
	}
	donpcevent "Poring Ball::OnBackToCenter";
	sleep 7000;
	set $@poringball,0;	
	set $@poringballnext_teamid, 0;
	cleanmap "soccer3";
	getmapunits(BL_PC,"soccer3",.@array);
	for(.@i = 0; .@i < getarraysize(.@array); .@i++) {
		if(countitem("SKILL_SHOOT",.@array[.@i])) delitem "SKILL_SHOOT",1,.@array[.@i];
		if(countitem("SKILL_PUSH",.@array[.@i])) delitem "SKILL_PUSH",1,.@array[.@i];
		if(!.pvp) setpcblock PCBLOCK_ATTACK,0,.@array[.@i];
		//sc_start SC_TIMERDOWN,361000,360,10000,SCSTART_NOAVOID,.@array[.@i];
	}
	mapwarp "soccer3","bat_room",154,150;
	if($@BG_Team201) { bg_destroy $@BG_Team201; $@BG_Team201 = 0; }
	if($@BG_Team202) { bg_destroy $@BG_Team202; $@BG_Team202 = 0; }
	$@BG20 = 0;
	bg_unbook "soccer3";
	end;
}

// ===========================================================
// Map Flags
// ===========================================================

soccer3	mapflag	nomemo
soccer3	mapflag	noteleport
soccer3	mapflag	nowarp
soccer3	mapflag	nosave	SavePoint
soccer3	mapflag	battleground	2
soccer3	mapflag	nopenalty
soccer3	mapflag	noskill

// ===========================================================
// Functions
// ===========================================================
function	script	CheckConditionsPB	{
	// Check if pc is hidden
	if(getstatus(SC_HIDING) || getstatus(SC_CLOAKING) || getstatus(SC_CHASEWALK) || getstatus(SC_CLOAKINGEXCEED) || getstatus(SC_CAMOUFLAGE) || getstatus(SC__INVISIBILITY)  ) {
		dispbottom "You cannot hit the ball while hiding.";	
		return 0;				
	} 

	// Check if the event is on
	if ($@poringball != 3) {
		npctalk "No match ! don't hit !";
		return 0;
	}

	// If ball is going towards a specific destination, we can't interrupt
	if (getvariableofnpc(.destination,"Poring Ball")) return 0;

	// Anti-cheat
	if ((getcharid(4) != $@BG_Team201) && (getcharid(4) != $@BG_Team202) && getgmlevel()<10) {
		warp "prontera",0,0;
		return 0;
	}

	return 1;
}

function	script	CheckPositionPB	{
	if (getarg(1) < getarg(0)) return 1;
	else if (getarg(1) > getarg(0)) return -1;
	else return 0;
}

function	script	CheckPoringClose	{
	getmapxy .@map$,.@x,.@y;
	getmapxy .@map2$,.@x2,.@y2,BL_NPC,"Poring Ball";
	if (distance(.@x,.@y,.@x2,.@y2)>2)||(.@map$ != "soccer3") return 0;
	return 1;
}

function	script	Skill_Shoot	{
	if(callfunc("CheckPoringClose")) {
		if(getvariableofnpc(.busy,"Poring Ball")) end;
		dispbottom "[Shoot] : Throw the Poring Ball within the next 3 sec.";
		setporingball;
		sleep2 3000;
		removeporingball;
	}
	return;
}

function	script	Skill_Push	{
	getmapxy .@map$,.@x,.@y;
	.@bgteam = bg_teamid();
	.@launcher = getcharid(3);
	if(.@map$ != "soccer3") end;
	showscript "Push !!";
	specialeffect2 457;
	detachrid;
	getareaunits(BL_PC,.@map$,.@x-1,.@y-1,.@x+1,.@y+1,.@array);
	for (.@i = 0; .@i < getarraysize(.@array); .@i++) {
		if(.@array[.@i] == .@launcher) || (bg_teamid(.@array[.@i]) == .@bgteam)
			continue;
		specialeffect2 2,AREA,convertpcinfo(.@array[.@i],CPC_NAME);
		if(rand(0,100) >= readparam(bVit,convertpcinfo(.@array[.@i],CPC_NAME)))
			unitpush .@array[.@i],rand(0,7),getvariableofnpc(.skill_push_cells,"Poringball_Config");
		if(rand(0,100) <= getvariableofnpc(.skill_push_chance,"Poringball_Config")-(readparam(bVit,convertpcinfo(.@array[.@i],CPC_NAME))/5)) {
			sit convertpcinfo(.@array[.@i],CPC_NAME);
			setpcblock PCBLOCK_SITSTAND, true, .@array[.@i];
		}
	}
	sleep 1000*getvariableofnpc(.skill_push_sit_time,"Poringball_Config");
	for (.@i = 0; .@i < getarraysize(.@array); .@i++) {
		setpcblock PCBLOCK_SITSTAND, false, .@array[.@i];
	}
	return;
}

// ===========================================================
// Throw Ball on a clicked cell scripts.
// ===========================================================
soccer3,0,0,0	script	TryThrowBall	-1,{
	if(callfunc("CheckPoringClose")) {
		if(!callfunc("CheckConditionsPB")) end;
		specialeffect 1,AREA,"Poring Ball";
		showscript "Shoot !!";
		.poringx = getporingballx();
		.poringy = getporingbally();
		doevent "Poring Ball::OnShoot";
		removeporingball;
	}
	end;
}

// ===========================================================
// Ball scripts.
// ===========================================================
soccer3,78,79,4	script	Poring Ball	1002,{
	getmapxy .@map$,.@x,.@y;
OnStart:
	if(!callfunc("CheckConditionsPB")) end;

	// If pc is too far, pc will automatically walk towards the ball and hit it
	if (distance(getnpcinfo(NPC_X),getnpcinfo(NPC_Y),.@x,.@y)>2) {
		unitwalk getcharid(3),getnpcinfo(NPC_X),getnpcinfo(NPC_Y),"Poring Ball::OnArrived";
		end;
	}

	// Check which team's turn 
	if ($@poringballnext_teamid != 0) && (getcharid(4) != $@poringballnext_teamid) {
		dispbottom "Wait your turn.";
		if(!.busy)
			goto OnStartTimer;
		end;
	}

	//=======================================================
	// = Attributes influences
	//=======================================================
	// Distance
	.distance = getvariableofnpc(.distance,"Poringball_Config");
	if(readparam(bStr) > readparam(bInt)) .distance += .distance*readparam(bStr)/100;	// STR affects distance by STR% if STR > INT
	else .distance += .distance*readparam(bInt)/100;					// INT affects distance by INT% if INT > STR
	.distance = rand(.distance-2,.distance);
	// Ball speed
	.@speed = getvariableofnpc(.speed,"Poringball_Config");					// AGI affects ball walking speed by AGI/3%
	.@speed -= .@speed*(readparam(bAgi)/3)/100;						// AGI affects ball walking speed by AGI/3%
	// Chance of touch
	if(rand(0,100) > getvariableofnpc(.chance,"Poringball_Config")+readparam(bDex)/3){	// DEX affects chance of hitting by DEX/3%.
		specialeffect2 611;
		end;
	}
	// Critical
	if(rand(0,100) < 20+(readparam(bLuk)/2)) {						// 20+LUK/2% chance to critical
		.@crit = 1;
		.distance += rand(2,4);
		.@speed -= 20;
	}
	// Caps
	if(.distance >= 15) .distance = 15;	// Distance max
	if(.@speed <= 50) .@speed = 50;		// Speed max
	// Penalty
	if(.penaltyGO) {
		.distance += rand(15,20);
		.@speed -= 20;
		.@crit = 1;
	}

	.@movingX = callfunc("CheckPositionPB",getnpcinfo(NPC_X),.@x);
	.@movingY = callfunc("CheckPositionPB",getnpcinfo(NPC_Y),.@y);
	.PlayerName$ = strcharinfo(0);
	$@poringballnext_teamid = 0;

	if (getnpcinfo(NPC_X)+(.@movingX*.distance) >= getvariableofnpc(.map_corner[1],"Poringball_Config")) .distance = getvariableofnpc(.map_corner[1],"Poringball_Config") - getnpcinfo(NPC_X);
	else if (getnpcinfo(NPC_X)+(.@movingX*.distance) <= getvariableofnpc(.map_corner[0],"Poringball_Config")) .distance = getnpcinfo(NPC_X) - getvariableofnpc(.map_corner[0],"Poringball_Config");
	else if (getnpcinfo(NPC_Y)+(.@movingY*.distance) >= getvariableofnpc(.map_corner[3],"Poringball_Config")) .distance = getvariableofnpc(.map_corner[3],"Poringball_Config") - getnpcinfo(NPC_Y);
	else if (getnpcinfo(NPC_Y)+(.@movingY*.distance) <= getvariableofnpc(.map_corner[2],"Poringball_Config")) .distance = getnpcinfo(NPC_Y) - getvariableofnpc(.map_corner[2],"Poringball_Config");
	if(!.distance) .distance ++;
	npcspeed .@speed;
	npcwalkto getnpcinfo(NPC_X)+.@movingX*.distance,getnpcinfo(NPC_Y)+.@movingY*.distance;

	//=======================================================
	// = Visual effects
	//=======================================================
	specialeffect2 457;
	if(readparam(bStr) > readparam(bInt)) specialeffect 2;
	else if(readparam(bDex) > readparam(bStr)) specialeffect 3;
	else misceffect 435;
	if(.@crit) {
		misceffect 1;
		misceffect 466;
		misceffect 400;
	} else misceffect 2;
OnMove:
	// Beginning of the movement
	for(.@i = 0; .@i < .distance; .@i++) {
		viewpointmap "soccer3",1, getnpcinfo(NPC_X), getnpcinfo(NPC_Y), 1, 0xffffff;
		if (.destination){
			for(.@i2 = 0; .@i2 < 1; .@i2++) {
				switch(.destination){
					case 0: break; 
					case 1: // Back to the center
						.@goingtoX = getvariableofnpc(.ball_center[0],"Poringball_Config");
						.@goingtoY = getvariableofnpc(.ball_center[1],"Poringball_Config");
						.@speed = 80;
						break;
					case 2: // corner blue team
						.@goingtoX = getvariableofnpc(.ball_corner1[0],"Poringball_Config");
						.@goingtoY = getvariableofnpc(.ball_corner1[1],"Poringball_Config");
						.@speed = 80;
						break;
					case 3: // corner red team
						.@goingtoX = getvariableofnpc(.ball_corner2[0],"Poringball_Config");
						.@goingtoY = getvariableofnpc(.ball_corner2[1],"Poringball_Config");
						.@speed = 80;
						break;
					case 4:	// Throw ball on a specific Cell
						.@goingtoX = getvariableofnpc(.poringx,"TryThrowBall");
						.@goingtoY = getvariableofnpc(.poringy,"TryThrowBall");
						.@speed = 50;
						break;
				}

				.@movingX = callfunc("CheckPositionPB",.@goingtoX,getnpcinfo(NPC_X));
				.@movingY = callfunc("CheckPositionPB",.@goingtoY,getnpcinfo(NPC_Y));

				npcspeed .@speed;
				viewpointmap "soccer3",1, getnpcinfo(NPC_X), getnpcinfo(NPC_Y), 1, 0xffffff;
				npcwalkto getnpcinfo(NPC_X)+.@movingX,getnpcinfo(NPC_Y)+.@movingY;

				if (getnpcinfo(NPC_X)==.@goingtoX) && (getnpcinfo(NPC_Y)==.@goingtoY) {
					if (.destination == 2)
						goto OnPenalty;
					else if (.destination == 3)
						goto OnPenalty;
					.destination = 0;
				} else .@i2 -= 1;
				sleep .@speed;
			}
			end;
		}

		// Check if a player can stop the ball by being sit on its way
		getareaunits(BL_PC,"soccer3",getnpcinfo(NPC_X)-1,getnpcinfo(NPC_Y)-1,getnpcinfo(NPC_X)+1,getnpcinfo(NPC_Y)+1,.@array);
		for(.@i3 = 0; .@i3 < getarraysize(.@array); .@i3++)
			if(ispcsit(.@array[.@i3]))&&(getvariableofnpc(.blockage,"Poringball_Config"))
				if(rand(0,100) >= 50+readparam(bVit,convertpcinfo(.@array,CPC_NAME))/3)
					unitstopwalk getnpcid(0),USW_FIXPOS;
		.@ballx = getnpcinfo(NPC_X)+.@movingX;
		.@bally = getnpcinfo(NPC_Y)+.@movingY;
		if(getvariableofnpc(.sound_poring,"Poringball_Config")) soundeffectall "poring_move.wav",0,"soccer3";
		if(.@moving) {
			npcspeed .@speed;
			npcwalkto .@ballx,.@bally;
			.@moving -= 1;
		}
OnGoalCheck:
		// Goal for the red Team
		if (.@ballx >= getvariableofnpc(.map_corner[1],"Poringball_Config")) .@teamclose$ = "blue";
		else if (.@ballx <= getvariableofnpc(.map_corner[0],"Poringball_Config")) .@teamclose$ = "red";
		if(!.penaltyGO)&&(.@teamclose$ != "") {
			if (.@bally >= getvariableofnpc(.map_goal[0],"Poringball_Config"))&&(.@bally <= getvariableofnpc(.map_goal[1],"Poringball_Config")){
				misceffect 8;
				npctalk "Goal !! I'm going back to the middle :>";
				setd("$@score_poring_"+.@teamclose$+""),getd("$@score_poring_"+.@teamclose$+"")+1;
				bg_updatescore "soccer3",$@score_poring_blue,$@score_poring_red;
				if(.@teamclose$=="red") $@poringballnext_teamid = $@BG_Team201; else $@poringballnext_teamid = $@BG_Team202;
				mapannounce "soccer3", ""+.PlayerName$+" scored a goal for the "+.@teamclose$+" team. First kick for the other team.",bc_blue;
				goto OnBackToCenter;
			}
			.distance += 3;
			if(.@teamclose$ == "red") .@movingX -= 1; else .@movingX += 1;
			.@moving += 3;
			.@teamclose$ = "";
			misceffect 426;
		} else if (!.penaltyGO)&&(.@bally >= getvariableofnpc(.map_corner[3],"Poringball_Config")) {
			.distance += 3;
			.@movingY -= 1;
			.@moving += 3;
			misceffect 426;
		} else if (!.penaltyGO)&&(.@bally <= getvariableofnpc(.map_corner[2],"Poringball_Config")) {
			.distance += 3;
			.@movingY += 1;
			.@moving += 3;
			misceffect 426;
		}
		sleep .@speed;
	}
	if(.penaltyGO)
		.penaltyGO = 0;
	end;

OnBackToCenter:	
	.destination = 1;
	.distance = 1;
	goto OnMove;
OnPenaltyRed:	
	.destination = 2;
	.distance = 1;
	npctalk "Penalty !!";
	goto OnMove;
OnPenaltyBlue:	
	.destination = 3;
	.distance = 1;
	npctalk "Penalty !!";
	goto OnMove;
OnPenalty:
	.destination = 0;
	.penaltyGO = 1;
	for(.@i = 20; .@i > 0; .@i--) {
		if(.penaltyGO) {
			sleep 1000;
			npctalk "Time left : "+.@i;
		} else end;
	}
	.penaltyGO = 0;
	$@poringballnext_teamid = 0;
	end;
OnShoot:
	.destination = 4;
	.distance = 1;
	.@speed = 20;
	goto OnMove;
OnStartTimer:
	.busy = 1;
	sleep 2000;
	if($@poringballnext_teamid) mapannounce "soccer3", "Ball is for "+($@poringballnext_teamid==$@BG_Team201?"blue":"red")+" team for 5 seconds.",bc_blue;
	sleep 5000;
	set $@poringballnext_teamid, 0;
	.busy = 0;
	end;
OnArrived:
	getmapxy .@map$,.@x,.@y;
	switch (getunitdir(getcharid(3))) {
		case DIR_NORTH: .@y --; break;
		case DIR_NORTHWEST: .@y --; .@x ++; break;
		case DIR_WEST: .@x ++; break;
		case DIR_SOUTHWEST: .@y ++; .@x ++; break;
		case DIR_SOUTH: .@y ++; break;
		case DIR_SOUTHEAST: .@y ++; .@x --; break;
		case DIR_EAST: .@x --; break;
		case DIR_NORTHEAST: .@y --; .@x --; break;
	}
	goto OnStart;
}

// ===========================================================
// Goals
// ===========================================================
soccer3,41,85,6	script	Blue Goal#poring1	1_FLAG_LION,{ end; }
soccer3,41,73,6	duplicate(Blue Goal#poring1)	Blue Goal#poring2	1_FLAG_LION
soccer3,116,85,2	duplicate(Blue Goal#poring1)	Red Goal#poring3	1_FLAG_EAGLE
soccer3,116,72,2	duplicate(Blue Goal#poring1)	Red Goal#poring4	1_FLAG_EAGLE

// ===========================================================
// Respawn
// ===========================================================
soccer3,133,137,3	script	Return#ball1	4_ENERGY_RED,{
	if($@poringball == 3) {
		attachnpctimer;
		initnpctimer;
	}
	end;
OnTimer1000:
	npctalk "5",strnpcinfo(0),bc_self;
	end;
OnTimer2000:
	npctalk "4",strnpcinfo(0),bc_self;
	end;
OnTimer3000:
	npctalk "3",strnpcinfo(0),bc_self;
	end;
OnTimer4000:
	npctalk "2",strnpcinfo(0),bc_self;
	specialeffect2 501;
	end;
OnTimer5000:
	npctalk "1",strnpcinfo(0),bc_self;
	specialeffect2 34;
	end;
OnTimer6000:
	stopnpctimer;
	if(getcharid(4) == $@BG_Team202) {
		if(rand(0,1)) warp "soccer3",121,107;
		else warp "soccer3",121,50;	
	} else {
		if(rand(0,1)) warp "soccer3",36,50;
		else warp "soccer3",36,107;
	}
	sleep2 200;
	specialeffect2 6;
	end;
}

soccer3,21,26,3	duplicate(Return#ball1)	Return#ball2	4_ENERGY_BLUE

// ===========================================================
// Healer
// ===========================================================
soccer3,127,138,3	duplicate(BGHeal)	Healer#ball1	95
soccer3,25,28,3	duplicate(BGHeal)	Healer#ball2	95