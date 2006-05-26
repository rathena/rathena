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
		return;
	}

	function list_mobs {
		set .@mob_size, getarraysize(.mc_moblist);
		for(set .@i, 0; .@i < .@mob_size; set .@i, .@i + 1){
			mes "- " + .mc_moblist[.@i];
		}
		return;
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
	}

L_AttackMenu:
	switch(select("Walk","Follow","Attack","Stop","Defend","Talk","Emote","Random Walk","Back")){
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
			next;
			goto L_MainMenu;
	}
	goto L_AttackMenu;
}