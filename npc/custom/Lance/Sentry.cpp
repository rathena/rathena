//(=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)
//(        (c)2006 eAthena Development Team presents        )
//(       ______  __    __                                  )
//(      /\  _  \/\ \__/\ \                     v 1.00.00   )
//(    __\ \ \_\ \ \ ,_\ \ \___      __    ___      __      )
//(  /'__`\ \  __ \ \ \/\ \  _ `\  /'__`\/' _ `\  /'__`\    )
//( /\  __/\ \ \/\ \ \ \_\ \ \ \ \/\  __//\ \/\ \/\ \_\.\_  )
//( \ \____\\ \_\ \_\ \__\\ \_\ \_\ \____\ \_\ \_\ \__/.\_\ )
//(  \/____/ \/_/\/_/\/__/ \/_/\/_/\/____/\/_/\/_/\/__/\/_/ )
//(   _   _   _   _   _   _   _     _   _   _   _   _   _   )
//(  / \ / \ / \ / \ / \ / \ / \   / \ / \ / \ / \ / \ / \  )
//( ( e | A | t | h | e | n | a ) ( S | c | r | i | p | t ) )
//(  \_/ \_/ \_/ \_/ \_/ \_/ \_/   \_/ \_/ \_/ \_/ \_/ \_/  )
//(                                                         )
//(=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=)
// Programmed by [Lance] ver. 1.1
// ---------------------------------------------------------
// [ Sentry System ]
// - Guards main towns against aggresive monsters and bad
//   players.
// [ Customization ]
// - See OnInit:
// =========================================================

-	script	sentry_system	-1,{
	function spawn_guardian {
		set .mob_id[getarg(0)], mobspawn("Guardian Sentry",1904,.mob_map$[getarg(0)],.mob_x[getarg(0)],.mob_y[getarg(0)]);
		mobattach .mob_id[getarg(0)]; // Attach events to this script.
		setmobdata .mob_id[getarg(0)], 24, 1; // Enable killer mode.
		setmobdata .mob_id[getarg(0)], 25, 
			AI_ACTION_TYPE_DETECT|
			AI_ACTION_TYPE_KILL|
			AI_ACTION_TYPE_UNLOCK|
			AI_ACTION_TYPE_DEAD|
			AI_ACTION_TYPE_ATTACK; // Define engine callback routines.
		setmobdata .mob_id[getarg(0)], 26, 1; // Prevents random walking.
		setmobdata .mob_id[getarg(0)], 10, 1; // Enable AI mode 1.
		getmobdata .mob_id[getarg(0)], .@temp;
		set .@temp[9], .@temp[9]^(0x400&.@temp[9]); // Check and remove MD_CHANGECHASE mode flag.
		setmobdata .mob_id[getarg(0)], 9, .@temp[9];
		return;
	}

	function search_entry {
		set .@tmp, getarraysize(getarg(0));
		for(set .@i, 0; .@i < .@tmp; set .@i, .@i + 1){
			if(getelementofarray(getarg(0),.@i) == getarg(1))
				break;
		}
		if(.@i == .@tmp)
			return -1;
		else
			return .@i;
	}

	// Script Entry Point - When an event from the script engine is received.
	if(getarraysize(.ai_action) == 4){ // Checks if the data is formatted correctly.
		set .@tmp, search_entry(.mob_id, .ai_action[AI_ACTION_SRC]);
		switch(.ai_action[AI_ACTION_TYPE]){
			case AI_ACTION_TYPE_DETECT: // We see something...
				if(.ai_busy[.@tmp] == 0){ // Not busy
					switch(.ai_action[AI_ACTION_TAR_TYPE]){ // Check what have we here.
							case AI_ACTION_TAR_TYPE_PC: // It's a player
								if(Karma > .karma){ // pkarma is higher?
									unittalk .ai_action[AI_ACTION_SRC], "Who goes there!";
									unitemote .ai_action[AI_ACTION_SRC], e_gasp; // !
									unitattack .ai_action[AI_ACTION_SRC],.ai_action[AI_ACTION_TAR];
									// We're currently busy.
									set .ai_busy[.@tmp], .ai_action[AI_ACTION_TAR];
								}
								break;
							case AI_ACTION_TAR_TYPE_MOB: // It's a monster
								if(.ai_action[AI_ACTION_TAR] != .ai_action[AI_ACTION_SRC]){
									getmobdata .ai_action[AI_ACTION_TAR], .@temp;
									if(.@temp[9]&0x804){ // In Aggressive mode?
										unittalk .ai_action[AI_ACTION_SRC], "Protect the villagers we must!";
										unitemote .ai_action[AI_ACTION_SRC], e_gasp; // !
										unitattack .ai_action[AI_ACTION_SRC],.ai_action[AI_ACTION_TAR];
										// We're currently busy.
										set .ai_busy[.@tmp], .ai_action[AI_ACTION_TAR];
									}
								}
								break;
					}
				}
				break;
			case AI_ACTION_TYPE_KILL: // We eliminated the criminal
				if(.ai_action[AI_ACTION_TAR_TYPE] == AI_ACTION_TAR_TYPE_PC)
					set Karma, 0;
			case AI_ACTION_TYPE_UNLOCK: // Target lost :(
				if(.@tmp != -1){
					set .ai_busy[.@tmp], 0; // Remove him, we're free.
				}
				// Walk back to where we came from.
				unitwalk .ai_action[AI_ACTION_SRC],.mob_x[.@tmp],.mob_y[.@tmp];
				break;
			case AI_ACTION_TYPE_DEAD: // We got killed :(
				if(.ai_action[AI_ACTION_TAR_TYPE] == AI_ACTION_TAR_TYPE_PC){ // Attacker is a player?
					if(Karma < 250)
						set Karma, Karma + 5;
					else
						set Karma, 255;
				}
				sleep 10000; // 10 seconds until reinforcements arrive
				spawn_guardian .@tmp;
				break;
			case AI_ACTION_TYPE_ATTACK: // Someone attacked us
				if(.ai_action[AI_ACTION_TAR_TYPE] == AI_ACTION_TAR_TYPE_PC){ // Attacker is a player?
					if(Karma < 250)
						set Karma, Karma + 1;
					else
						set Karma, 255;
				}
				// The system's AI will auto attack any attackers. So we leave it here.
				break;
		}
	}
	deletearray .ai_action, getarraysize(.ai_action); // Cleans up and frees up memory
	end;

OnInit:
	// Customization ---------------------------------------------------------------------
	setarray .mob_map$, "prt_fild08.gat", "prt_fild05.gat", "prt_fild06.gat", "prt_gld.gat";
	setarray .mob_x,176,369,29,165;
	setarray .mob_y,372,201,187,37;
	set .karma, 5;
	// -----------------------------------------------------------------------------------
	set .@tmp, getarraysize(.mob_map$);
	for(set .@i, 0; .@i < .@tmp; set .@i, .@i + 1){
		spawn_guardian .@i;
	}
	debugmes "[Sentry System] Spawned " + .@i + " guardians.";
	end;

}