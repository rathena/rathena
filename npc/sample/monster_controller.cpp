// Variables Logging:
// .mc_moblist[] - ID list of mobs
prontera.gat,180,200,4	script	Monster Controller	123,{
	function display_info {
		getmobdata getarg(0), .@mob_data;
		set .@array_size, getarraysize(.@mob_data);
		for(set .@i, 0; .@i < .@array_size; set .@i, .@i + 1){
			mes .@i + " - " + .@mob_data[.@i];
		}
		return;
	}

	function remove_mob {
		removemob getarg(0);
		set .@mob_size, getarraysize(.mc_moblist);
		for(set .@i, 0; .@i < .@mob_size; set .@i, .@i + 1){
			if(.mc_moblist[.@i] == getarg(0))
				deletearray .mc_moblist[.@i], 1;
		}
	}

	function make_menu {
		set .@array_size, getarraysize(.mc_moblist);
		set .@tmp_str$, "";
		for(set .@i, 0; .@i < .@array_size; set .@i, .@i + 1){
			set .@tmp_str$, .@tmp_str$ + .mc_moblist[.@i] + ":";
		}
		select .@tmp_str$;
		return .mc_moblist[@menu-1];
	}

	function summon_mob {
		set .@mob_size, getarraysize(.mc_moblist);
		set .mc_moblist[.@mob_size], spawnmob("Slave - " + .@mob_size, getarg(0), "prontera.gat", 180, 200);
		mobattach .mc_moblist[.@mob_size];
		setmobdata .mc_moblist[.@mob_size], 25, 
			AI_ACTION_TYPE_ATTACK|
			AI_ACTION_TYPE_DETECT|
            AI_ACTION_TYPE_DEAD|
			AI_ACTION_TYPE_ASSIST|
			AI_ACTION_TYPE_KILL|
            AI_ACTION_TYPE_UNLOCK|
            AI_ACTION_TYPE_WALKACK|
            AI_ACTION_TYPE_WARPACK;
		return;
	}

	function list_mobs {
		set .@mob_size, getarraysize(.mc_moblist);
		for(set .@i, 0; .@i < .@mob_size; set .@i, .@i + 1){
			mes "- " + .mc_moblist[.@i];
		}
		return;
	}

	if(getarraysize(.ai_action) == 4){
		mapannounce "prontera.gat", "[Mob Control] AI Action Received from " + .ai_action[AI_ACTION_SRC] + "!",16;
		switch(.ai_action[AI_ACTION_TAR_TYPE]){
			case AI_ACTION_TAR_TYPE_PC:
				set .@action_from$, "Player";
				set .@action_name$, rid2name(.ai_action[AI_ACTION_TAR]);
				break;
			case AI_ACTION_TAR_TYPE_MOB:
				set .@action_from$, "Monster";
				set .@action_name$, rid2name(.ai_action[AI_ACTION_TAR]);
				break;
			case AI_ACTION_TAR_TYPE_PET:
				set .@action_from$, "Pet";
				set .@action_name$, rid2name(.ai_action[AI_ACTION_TAR]);
				break;
			case AI_ACTION_TAR_TYPE_HOMUN:
				set .@action_from$, "Homunculus";
				set .@action_name$, rid2name(.ai_action[AI_ACTION_TAR]);
				break;
			default:
				set .@action_from$, "Unknown";
				set .@action_name$, ""+.ai_action[AI_ACTION_TAR];
				break;
		}

		switch(.ai_action[AI_ACTION_TYPE]){
			case AI_ACTION_TYPE_ATTACK:
				set .@action_type$, "Attacked by";
				break;
			case AI_ACTION_TYPE_DETECT:
				set .@action_type$, "Detected";
				break;
			case AI_ACTION_TYPE_DEAD:
				set .@action_type$, "Killed by";
				remove_mob .ai_action[AI_ACTION_SRC];
				break;
			case AI_ACTION_TYPE_ASSIST:
				set .@action_type$, "Assisting";
				break;
			case AI_ACTION_TYPE_UNLOCK:
				set .@action_type$, "Unlocked target";
				break;
			case AI_ACTION_TYPE_KILL:
				set .@action_type$, "Killed";
				break;
			case AI_ACTION_TYPE_WALKACK:
				set .@action_type$, "Completed Walking";
				break;
			case AI_ACTION_TYPE_WARPACK:
				set .@action_type$, "Warped";
				break;
		}

		mapannounce "prontera.gat", "Details - " + .@action_type$ + " [" + .@action_from$ + "] " + .@action_name$ + "!", 16;
		deletearray .ai_action, 4;
		end;
	}

L_MainMenu:
	mes "[Monster Controller]";
	mes "Current active monsters:";
	list_mobs;
	switch(select("Summon","Remove","Information","Actions")){
		case 1: // Summon
			next;
			mes "[Monster Controller]";
			mes "Monster ID -";
			input @mob_id;
			next;
			summon_mob @mob_id;
			goto L_MainMenu;
			break;
		case 2: // Remove
			remove_mob make_menu();
			next;
			goto L_MainMenu;
			break;
		case 3: // Information
			set .@tmp, make_menu();
			next;
			mes "[Monster Info]";
			display_info .@tmp;
			next;
            		goto L_MainMenu;
			break;
		case 4: // Actions
			goto L_AttackMenu;
			break;
		default:
			end;
	}

L_AttackMenu:
	switch(select("Walk","Follow","Attack","Stop","Defend","Talk","Emote","Random Walk","Callback","Back")){
		case 1: // Walk
			set .@src, make_menu();
			input .@x;
			input .@y;
			mobwalk .@src,.@x,.@y; // Mode 1: Walk to location.
			break;
		case 2: // Follow
			set .@src, make_menu();
			input .@tar;
			mobwalk .@src, .@tar; // Mode 2: Walk to target.
			break;
		case 3: // Attack
			set .@src, make_menu();
			input .@tar;
			mobattack .@src, .@tar;
			break;
		case 4: // Stop
			set .@src, make_menu();
			mobstop .@src;
			break;
		case 5: // Defend/Assist
			set .@src, make_menu();
			input .@tar;
			mobassist .@src, .@tar;
			break;
		case 6: // Talk
			set .@src, make_menu();
			input .@text$;
			mobtalk .@src, .@text$;
			break;
		case 7: // Emote
			set .@src, make_menu();
			input .@emote;
			mobemote .@src, .@emote;
			break;
		case 8:
			set .@src, make_menu();
			input .@flag;
			mobrandomwalk .@src, .@flag;
			break;
		case 9:
			set .@src, make_menu();
			input .@flag;
			setmobdata .@src, 25, .@flag;
			break;
		case 9:
			next;
			goto L_MainMenu;
		default:
			end;
	}
	goto L_AttackMenu;
}