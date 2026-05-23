
1@whl,1,1,0	script	#19_airship_control	-1,{
	end;
	
OnInstanceInit:
	set_instance_var("map$",instance_mapname("1@whl"));
	instance_enable("#st_warp",true);
	set_instance_var("mode",1);
	set_instance_var("shield",1);  
	set_instance_var("class$","Economy");
	set_instance_var("difficulty$","C");
	set_instance_var("airship",0);
	set_instance_var("status",0);
	set_instance_var("summon",0);
	set_instance_var("summon_1",0);
	set_instance_var("summon_2",0);
	set_instance_var("summon_3",0);
	set_instance_var("summon_4",0);
	set_instance_var("walk",0);
	instance_enable("#19_g_1",false);
	instance_enable("#19_g_2",false);
	instance_enable("#19_g_3",false);
	instance_enable("#19_g_4",false);
	instance_enable("#19_y_1",false);
	instance_enable("#19_y_2",false);
	instance_enable("#19_box_control",false);
end;
}

1@whl,32,53,0	script	#st_warp	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(!is_party_leader()) end;
	instance_disable(strnpcinfo(0));
	instance_enable("#start_control",true);
	instance_event("#start_control","OnWalk",false);
	warp get_instance_var("map$"),160,160;
	set_instance_var("airship",5);
end;
}

1@whl,46,55,6	script	#start_control	20637,{
	if(!is_party_leader() || !get_instance_var("walk")) end;
	if(get_instance_var("status") > 0) end;
	mes "[Airship Management Doll]";
	mes "Are you a passenger? I will check your seat class. May I see your ticket?";
	mes "------------------";
	mes "Current Class - ^0000FF" + get_instance_var("class$") + "^000000";
	next;
	switch(select("Wait a moment:Show a random note:Change your seat's class")){
		case 1:
			mes "[Airship Management Doll]";
			mes "We'll provide services that meet your class reservation. Please refer to usage manual.";
			close;
			
		case 2:
			instance_event(strnpcinfo(0),"OnStart",true);
			end;
			
		case 3:
			mes "[Airship Management Doll]";
			mes "We offer services based on the class of the flight ticket your reservation. Would you like to upgrade your seat?";
			next;
			.@s = select("Quit:Economy:Business:First Class") - 1;
			switch(.@s){
				case 0:
					mes "[Airship Management Doll]";
					mes "^660000Didn't you want to change seats?^000000";
					next;
					instance_event(strnpcinfo(0),"OnStart",true);
					end;
					
				default:
					setarray .@class$[1],"Economy","Business","First Class";
					setarray .@dif$[1],"C","B","A";
					.@class$ = "Economy";
					mes "[Airship Management Doll]";
					mes  "Would you like to adjust your seat to a ^0000FF[" + .@class$[.@s] + "]^000000";
					next;
					if(select("Quit:Please change it.") == 1){
						mes "[Airship Management Doll]";
						mes "^660000Didn't you want to change seats?^000000";
						next;
						instance_event(strnpcinfo(0),"OnStart",true);
						end;
					}
					set_instance_var("mode",.@s);
					set_instance_var("difficulty$",.@dif$[.@s]);
					set_instance_var("class$",.@class$[.@s]);
					set_instance_var("mob_hp",2600000 + .@mode);
					set_instance_var("mob_def",62 + .@mode);
					set_instance_var("mob_mdef",30 + .@mode);
					mes "[Airship Management Doll]";
					mes "The seat upgrade process has been completed. Could you please show me your flight ticket?";
					next;
					instance_event(strnpcinfo(0),"OnStart",true);
					end;	
			}
	}
	end;
	
OnWalk:
	npcspeed 300;
	unitwalk getnpcid(0),36,54;
	sleep 800;
	emotion ET_THROB;
	sleep 2000;
	set_instance_var("walk",1);
end;

OnStart:
	mes "[Airship Management Doll]";
	mes "This. It looks a little different from our flight ticket. I will get back to you after I go through the verification process. Please wait a moment.";
	close2;
	set_instance_var("status",1);
	emotion ET_PROFUSELY_SWAT;
	sleep 1500;
	npctalk "...I've never seen a ticket like this before. I'll be right back after verifying it, so please wait a moment.";
	npcspeed 300;
	unitwalk getnpcid(0),46,55,instance_npcname(strnpcinfo(0)) + "::OnEvent";
end;

OnEvent:
	instance_disable(strnpcinfo(0));
	sleep 3000;
	instance_announce false,"Airship Management Doll: A fake airline ticket has been brought into the airship! All dolls are to enter a freeloader response sequence.",bc_map,0xFFDDAA;
	sleep 3000;
	instance_announce false,"Airship Management Doll: The person has forged into " + get_instance_var("class$") + ". The corresponding response level is grade " + get_instance_var("difficulty$") + ".",bc_map,0xFFDDAA;
	set_instance_var("airship",1);
end;
}

1@whl,53,74,0	script	#191_wh1_wp_1	WARPNPC,2,2,{
	end;
	
OnTouch:
	.@id = atoi(replacestr(strnpcinfo(2),"191_wh1_wp_",""));
	if(get_instance_var("airship") < .@id){
		if(.@id > 1)
			instance_announce false,"SYSTEM: Warping is prohibited in the airship in an active combat situation",bc_map,0xFF4444;
		end;
	}
	switch(.@id){
		case 1:
			if(get_instance_var("airship") >= 6){
				.@x = 160;
				.@y = 140;
			} else {				
				.@x = 53;
				.@y = 86;
			}
			break;
			
		case 2:
			.@x = 53;
			.@y = 110;
			break;	
		
		case 3:
			.@x = 139;
			.@y = 24;
			break;
			
		case 4:
			.@x = 160;
			.@y = 53;
			break;
			
		case 5:
			warp get_instance_var("map$"),160,140;
			end;
	}
	warp get_instance_var("map$"),.@x,.@y;
	if(!get_instance_var("summon_" + .@id)){
		set_instance_var("summon_" + .@id,1);
		instance_event("#19_airship_mob","OnSummon",false);
	}
end;
}

1@whl,53,97,0	duplicate(#191_wh1_wp_1)	#191_wh1_wp_2	WARPNPC,2,2
1@whl,37,162,0	duplicate(#191_wh1_wp_1)	#191_wh1_wp_3	WARPNPC,2,2
1@whl,160,43,0	duplicate(#191_wh1_wp_1)	#191_wh1_wp_4	WARPNPC,2,2
1@whl,160,117,0	duplicate(#191_wh1_wp_1)	#191_wh1_wp_5	WARPNPC,2,2

1@whl,160,166,5	script	#19_boss_control	EP19_MD_AQUILA,{
	if(!is_party_leader()) end;
	if(get_instance_var("airship") == 5){
		mes "[Aquila]";
		mes "It's still not too late. If you stop destroying and head back, I will not take any further action.";
		next;
		if(select("I'll just go back.:I can't just go back, right?") == 1){
			mes "[Aquila]";
			mes "A wise decision.";
			close2;
			warp get_instance_var("map$"),31,54;
			end;
		}
		close2;
		setnpcdisplay instance_npcname(strnpcinfo(0)),CLEAR_NPC;
		instance_disable(strnpcinfo(0));
		instance_event("#19_airship_boss","OnBossSummon",false);
		end;
	}
	end;
}

1@whl,1,1,0	script	#19_airship_mob	-1,{
	end;
	
OnSummon:
	sleep 2000;
	.@summon = get_instance_var("summon");
	switch(.@summon){
		case 0:
			.@zone = 2;
			setarray .@mob,
			56,88,20342,
			51,93,20631,
			65,84,20633,
			42,96,20639,
			43,90,20641,
			48,90,20641,
			41,96,20641,
			67,87,20641,
			60,90,20633,
			57,85,20639,
			37,96,20639,
			54,97,20641,
			54,83,20639,
			56,96,20631,
			56,93,20641,
			57,87,20633,
			67,80,20641;
			instance_announce false,"???: Alert! An unauthorized presence has infiltrated the airship. Patrols focus on searching Zone 2.",bc_map,0xFFEE66;
			break;
			
		case 1:
			.@zone = 3;
			setarray .@mob,
			54,120,20633,
			52,120,20641,
			51,123,20639,
			56,123,20633,
			40,122,20639,
			42,123,20639,
			56,115,20639,
			66,122,20631,
			49,122,20639,
			52,132,20639,
			51,137,20639,
			55,139,20641,
			58,140,20641,
			45,141,20639,
			47,143,20639,
			58,144,20631,
			61,144,20641,
			64,144,20631,
			61,145,20631,
			50,148,20633,
			61,149,20631,
			60,149,20633,
			60,149,20631,
			53,151,20631,
			58,151,20631,
			58,153,20342,
			59,153,20633,
			50,153,20633,
			53,153,20639,
			60,157,20641,
			55,158,20641,
			68,143,20639,
			49,160,20641,
			52,161,20631,
			63,124,20631,
			41,119,20639,
			52,118,20639,
			54,117,20633,
			52,117,20641,
			52,117,20639,
			54,117,20633,
			54,115,20639,
			72,122,20631,
			73,124,20641,
			73,122,20641,
			39,122,20639,
			34,124,20631,
			36,146,20631;
			instance_announce false,"???: Notify all patrols. Search and stop the intruder who entered the cargo hold and the airship's rest area. I repeat...",bc_map,0xFFEE66;
			break;
			
		case 2:
			.@zone = 4;
			setarray .@mob,
			177,31,20342,
			156,34,20639,
			184,41,20639,
			187,31,20639,
			173,30,20641,
			156,34,20639,
			168,34,20633,
			169,35,20641,
			177,37,20641,
			141,35,20633,
			163,29,20633,
			163,35,20641,
			178,31,20641,
			177,37,20641;
			instance_announce false,"???: The intruder that was on the rest area is now in the engine room. Cargo search personnels should stop searching and support the engine room. I repeat...",bc_map,0xFFEE66;
			break;
			
		case 3:
			.@zone = 5;
			setarray .@mob,	
			154,96,20342,
			160,77,20639,
			166,76,20633,
			168,97,20631,
			173,98,20639,
			174,74,20641,
			146,100,20641,
			151,100,20641,
			171,100,20633,
			159,97,20641,
			159,100,20641,
			167,99,20633,
			155,95,20639,
			149,97,20633,
			173,98,20639,
			147,97,20641,
			159,99,20641,
			160,98,20641,
			163,94,20639,
			151,106,20641,
			153,101,20639,
			151,92,20639,
			161,94,20633,
			153,82,20641,
			162,93,20639,
			163,94,20639,
			155,77,20639,
			160,77,20639,
			158,74,20631,
			159,73,20633,
			150,73,20641,
			154,73,20639,
			164,68,20641,
			156,66,20641,
			158,63,20639,
			146,53,20639,
			146,50,20631,
			166,76,20633,
			168,50,20633,
			170,63,20641,
			174,53,20639,
			174,74,20641,
			170,75,20639,
			149,90,20633,
			151,92,20639,
			152,95,20641,
			160,98,20641,
			173,98,20639,
			159,100,20641,
			152,100,20641,
			145,101,20641,
			153,101,20639,
			151,106,20641,
			176,98,20641,
			177,96,20631;
			instance_announce false,"???: Destroy all the intruders who have infiltrated the engine room. Prevent them from approaching the core. I repeat...",bc_map,0xFFEE66;
			break;
	}
	.@size = getarraysize(.@mob);
	set_instance_var("count",.@size/3);
	for(.@i = 0; .@i < .@size; .@i += 3){
		monster get_instance_var("map$"),.@mob[.@i],.@mob[.@i+1],"Zone " + .@zone + " " + (.@i == 0 ? "Key":"Patrol"),.@mob[.@i+2],1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
		set_instance_var $@mobid[0],UMOB_MAXHP,get_instance_var("mob_hp");
		set_instance_var $@mobid[0],UMOB_HP,get_instance_var("mob_hp");
		set_instance_var $@mobid[0],UMOB_DEF,get_instance_var("mob_def");
		set_instance_var $@mobid[0],UMOB_MDEF,get_instance_var("mob_mdef");
		getunitdata $@mobid[0],.@data;
		set_instance_var $@mobid[0],UMOB_STR,.@data[UMOB_STR] * (30 + get_instance_var("mode")); 
		set_instance_var $@mobid[0],UMOB_AGI,.@data[UMOB_AGI] * (30 + get_instance_var("mode")); 
		set_instance_var $@mobid[0],UMOB_VIT,.@data[UMOB_VIT] * (20 + get_instance_var("mode")); 
		set_instance_var $@mobid[0],UMOB_INT,.@data[UMOB_INT] * (30 + get_instance_var("mode")); 
		set_instance_var $@mobid[0],UMOB_DEX,.@data[UMOB_DEX] * (50 + get_instance_var("mode")); 
		set_instance_var $@mobid[0],UMOB_LUK,.@data[UMOB_LUK] * (10 + get_instance_var("mode"));
		set_instance_var $@mobid[0],UMOB_ATKMIN,.@data[UMOB_ATKMIN] * (2000 + get_instance_var("mode")); 
		set_instance_var $@mobid[0],UMOB_ATKMAX,.@data[UMOB_ATKMAX] * (2000 + get_instance_var("mode")); 
		unitskilluseid $@mobid[0],"NPC_RELIEVE_ON",(.@i == 0 ? 9 : get_instance_var("shield"));
		unitskilluseid $@mobid[0],(rand(1,2) == 1 ? "NPC_CHANGEDARKNESS" : "NPC_CHANGEHOLY"),1;
		sleep 500;
	}
end;

OnMobKill:
	.@count = get_instance_var("count");
	set_instance_var("count",.@count - 1);
	if(.@count - 1 == 0){
		set_instance_var("summon",get_instance_var("summon") + 1);
		set_instance_var("airship",get_instance_var("airship") + 1);
	} else {
		switch(get_instance_var("summon")){
			case 1:
				if(get_instance_var("count") == 43)
					instance_announce false,"???: How about we have a peaceful conversation, foreign organisms? Violence does not resolve the situation.",bc_map,0x00CCFF;
				if(get_instance_var("count") == 38)
					instance_announce false,"???: It seems that you intend to keep moving forward. If you do not stop, we will have no choice but to respond with force.",bc_map,0x00CCFF;
				if(get_instance_var("count") == 28)
					instance_announce false,"???: Put your everything into it, little pitiful patrols. Your fate belongs in this airship, If the airship is destroyed, you will go down with it.",bc_map,0xFFDD66;
				if(get_instance_var("count") == 5)
					instance_announce false,"???: We better ignore the life response in the cargo hold. Do not expense anymore combat force. Attack all the prohibited foreign organisms that are headed in the engine room.",bc_map,0xFFDD66;
				end;
				
			case 2:
				if(get_instance_var("count") == 11)
					instance_announce false,"???: Thank you patrols for not learning the emotion, anger. If you had that emotion, you would immediately become powdered with this airship.",bc_map,0xFFDD66;
				if(get_instance_var("count") == 5)
					instance_announce false,"???: It seems that you intend to keep moving forward. If you do not stop, we will have no choice but to respond with force.",bc_map,0x00CCFF;
				end;
				
			case 3:
				if(get_instance_var("count") == 50)
					instance_announce false,"???: I wonder what were you doing before entering this airship. Foreign organism, you're completely used to destroying the patrols. Was your life about collecting scrap metals in Einbech?",bc_map,0x00CCFF;
				if(get_instance_var("count") == 25)
					instance_announce false,"???: If you go back now, I will spare your life, foreign organism. Choose wisely.",bc_map,0x00CCFF;
				end;
		}
	}
end;
}

1@whl,1,1,0	script	#19_airship_boss	-1,{
	end;
	
OnBossSummon:
	monster get_instance_var("map$"),159,158,"Aquila",21531,1,instance_npcname(strnpcinfo(0)) + "::OnBossKill";
	set_instance_var("gid",$@mobid[0]);
	if(get_instance_var("mode") > 1){
		getunitdata $@mobid[0],.@data;
		setunitdata $@mobid[0],UMOB_MAXHP,.@data[UMOB_MAXHP] * get_instance_var("mode");
		setunitdata $@mobid[0],UMOB_HP,.@data[UMOB_HP] * get_instance_var("mode");
		setunitdata $@mobid[0],UMOB_STR,.@data[UMOB_STR] * (50 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_AGI,.@data[UMOB_AGI] * (20 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_VIT,.@data[UMOB_VIT] * (20 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_INT,.@data[UMOB_INT] * (50 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_DEX,.@data[UMOB_DEX] * (50 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_LUK,.@data[UMOB_LUK] * (20 + get_instance_var("mode"));
		setunitdata $@mobid[0],UMOB_RES,.@data[UMOB_RES] * (50 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_MRES,.@data[UMOB_MRES] * (50 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_ATKMIN,.@data[UMOB_ATKMIN] * (3000 + get_instance_var("mode")); 
		setunitdata $@mobid[0],UMOB_ATKMAX,.@data[UMOB_ATKMAX] * (3000 + get_instance_var("mode"));
	}
	unitskilluseid $@mobid[0],"NPC_RELIEVE_ON",9;
	instance_event(strnpcinfo(0),"OnBossSkill",false);
	instance_event(strnpcinfo(0),"OnBossSkill2",false);
end;

OnBossKill:
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnSkillDead";
	killmonster get_instance_var("map$"),.@event$;
	set_instance_var("airship",8);
	instance_enable("#19_box_control",true);
end;

OnBossSkill:
	.@gid = get_instance_var("gid");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnSkillDead";
	while(unitexists(.@gid)){
		sleep 1000;
		getunitdata .@gid,.@data;
		.@DAMAGE = (.@data[UMOB_MAXHP]/10) * 9; //The boss will summon the same time as first max pain at 90% HP
		if(.@data[UMOB_HP] > .@DAMAGE) continue;
		instance_announce false,"The unauthorized outsiders have already intruded the core management room. Cleaning robots are requested to proceed with the cleaning procedure immediately.",bc_map,0x00FFFF;
		.@amount = rand(15,25);
		.@clear = rand(1,2);
		for(.@i = 0; .@i < .@amount; .@i++){
			if(!unitexists(.@gid)){
				killmonster get_instance_var("map$"),.@event$;
				end;
			}
			switch(rand(1,3)){
				case 1:
					areamonster get_instance_var("map$"),140,140,179,179,"Broken Cleaning Robot",20641,1,.@event$;
					break;
				case 2:
					areamonster get_instance_var("map$"),134,148,185,170,"Broken Cleaning Robot",20641,1,.@event$;
					break;
				case 3:
					areamonster get_instance_var("map$"),152,134,167,187,"Broken Cleaning Robot",20641,1,.@event$;
					break;
			}
			set_instance_var("robot_" + .@i,$@mobid[0]);
			setunitdata $@mobid[0],UMOB_MODE,(.@mode|MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC);
			unitskilluseid $@mobid[0],"NPC_RELIEVE_ON",1;
			unitskilluseid $@mobid[0],"LK_AURABLADE",1;
			sleep 500;
		}
		instance_announce false,"The robots that are already in place will proceed with the self-purification sequence.",0x00FFFF;
		set_instance_var("amount",.@amount);
		set_instance_var("clear",.@clear);
		sleep 10000;
		instance_announce false,"Running the purification sequence.",bc_map,0x00FFFF;
		for(.@i = 0; .@i < .@amount; .@i++){
			if(unitexists(get_instance_var("robot_" + .@i))){
				unittalk get_instance_var("robot_" + .@i),"Purifying contaminants!!";
				unitskilluseid get_instance_var("robot_" + .@i),"SJ_LIGHTOFSUN",1;
			}
		}
		sleep 1000;
		for(.@i = 0; .@i < .@amount; .@i++){
			if(unitexists(get_instance_var("robot_" + .@i)))
				unitskilluseid get_instance_var("robot_" + .@i),"NPC_SELFDESTRUCTION",1;
		}
		set_instance_var("clear",0);
		sleep 19000;
	}
OnSkillDead:
end;

OnBossSkill2:
	.@gid = get_instance_var("gid");
	while(unitexists(.@gid)){
		sleep 1000;
		getunitdata .@gid,.@data;
		.@DAMAGE = (.@data[UMOB_MAXHP]/10) * 8;
		if(.@data[UMOB_HP] > .@DAMAGE) continue;
		if(!get_instance_var("berserk")){
			unitskilluseid get_instance_var("gid"),"DE_BERSERKAIZER",1;
			set_instance_var("berserk",1);
			setunitdata .@gid,UMOB_ATKRANGE,6;
		}
		if(get_instance_var("pattern_1") && get_instance_var("pattern_2")) continue;
		.@type = rand(1,2);
		if(.@type == 1){
			if(!get_instance_var("pattern_1"))
				instance_event(strnpcinfo(0),"OnYellow",false);
			else {
				if(!get_instance_var("pattern_2"))
					instance_event(strnpcinfo(0),"OnGreen",false);
			}
		} else {
			if(!get_instance_var("pattern_2"))
				instance_event(strnpcinfo(0),"OnGreen",false);
			else {
				if(!get_instance_var("pattern_1"))
					instance_event(strnpcinfo(0),"OnYellow",false);
			}
		}
		sleep 29000;
	}
end;

OnYellow:
	.@map$ = get_instance_var("map$");
	.@x = rand(130,189);
	.@y = rand(140,185);
	while(!checkcell(.@map$,.@x,.@y,cell_chkpass)){
		.@x = rand(130,189);
		.@y = rand(140,185);
		sleep 10;
	}
	set_instance_var("yellow",2);
	unitwarp get_instance_var("gid"),.@map$,.@x,.@y;
	//setunitdata get_instance_var("gid"),UMOB_VSIZE,Size_Large; kRO use some kind of size packet to increase the boss size
	unitskilluseid get_instance_var("gid"),"LK_BERSERK",1;
	instance_announce false,"The core's yellow ether has depleted due to long operations. Please recharge it immediately. The ether depletion will make the core go into overload mode.",bc_map,0xFFCC00;
	instance_enable("#19_y_1",true);
	instance_enable("#19_y_2",true);
end;

OnGreen:
	.@amount = rand(1,4);
	set_instance_var("green",.@amount);
	for(.@i = 0; .@i < .@amount; .@i++){
		.@var = rand(1,4);
		while(inarray(.@rand,.@var) != -1)
			.@var = rand(1,4);
		.@rand[getarraysize(.@rand)] = .@var;
		instance_enable("#19_g_" + .@var,true);
	}
	unitskilluseid get_instance_var("gid"),"NPC_RELIEVE_OFF",1;
	unitskilluseid get_instance_var("gid"),"NPC_RELIEVE_ON",10;
	instance_announce false,"The core's green ether has depleted due to long operations. The core will enter in defense mode until the ether is recharged.",bc_map,0xFFDDAA;
end;
}

1@whl,159,173,1	script	#19_b_memo	CLEAR_NPC,{
	if(get_instance_var("airship") == 8){
		if(checkquest(12561,HUNTING) == 2){
			switch(get_instance_var("mode")){
				case 1:
					setarray .@rewards,1000608,4,1000811,1;
					break;
					
				case 2:
					setarray .@rewards,1000608,9,1000811,2;
					break;
					
				case 3:
					setarray .@rewards,1000608,20,1000811,5;
					break;
			}
			erasequest 12561;
			mes "======== Reward =======";
			for(.@i = 0; .@i < 4; .@i += 2){
				mes .@rewards[.@i+1] + " <ITEM>[" + getitemname(.@rewards[.@i]) + "]<INFO>" + .@rewards[.@i] + "</INFO></ITEM>";
				getitem .@rewards[.@i],.@rewards[.@i+1];
			}
			mes "You have gained an experience and job experience.";
			getexp 5096730 * get_instance_var("mode"),3644000 * get_instance_var("mode");
			next;
			.@reputation = 5 + (get_instance_var("mode") * 5);
			mes "^0000CDYour Ice Castle reputation has increased by " + .@reputation + " points.^000000";
			addreputation("Ice Castle",.@reputation);
			close;
		}
		end;
	}
	if(get_instance_var("berserk")){
		mes "Press this switch when the core is in remote mode and is out of control.";
		close;
	}
	specialeffect EF_HOMUNCASTING;
	specialeffect EF_STORMKICK7;
	progressbar "3132FF",2;
	set_instance_var("berserk",0);
	setunitdata get_instance_var("gid"),UMOB_ATKRANGE,1;
	specialeffect EF_STORMKICK7;
	specialeffect EF_CHIMTO2;
	end;
}

1@whl,146,181,5	script	#19_clear_1	CLEAR_NPC,{
	.@id = atoi(replacestr(strnpcinfo(2),"19_clear_",""));
	if(.@id != get_instance_var("clear")){
		mes "Press this switch to send a dispatch signal to another zone, if the amount of the cleaning robots are high.";
		close;
	}
	progressbar "3132FF",3;
	set_instance_var("clear",0);
	
	for(.@i = 0; .@i < get_instance_var("amount")/2; .@i++){
		.@var = rand(get_instance_var("amount"));
		while(inarray(.@index,.@var) != -1)
			.@var = rand(get_instance_var("amount"));
		.@index[getarraysize(.@index)] = .@var;
		if(unitexists(get_instance_var("robot_" + .@var))){
			//unitwarp get_instance_var("robot_" + .@var),get_instance_var("map$"),0,0; //idk if they are warped or killed
			unitkill get_instance_var("robot_" + .@var);
			set_instance_var("robot_" + .@var,0);
		}
	}
	end;
}

1@whl,173,181,5	duplicate(#19_clear_1)	#19_clear_2	CLEAR_NPC

1@whl,160,160,5	script	#19_box_control	PORTAL,{
	if(get_instance_var("airship") == 8){
		if(checkquest(12561,HUNTING) == 2){
			switch(get_instance_var("mode")){
				case 1:
					setarray .@rewards,1000608,4,1000811,1;
					break;
					
				case 2:
					setarray .@rewards,1000608,9,1000811,2;
					break;
					
				case 3:
					setarray .@rewards,1000608,20,1000811,5;
					break;
			}
			erasequest 12561;
			mes "======== Reward =======";
			for(.@i = 0; .@i < 4; .@i++){
				mes .@rewards[.@i+1] + " <ITEM>[" + getitemname(.@rewards[.@i]) + "]<INFO>" + .@rewards[.@i] + "</INFO></ITEM>";
				getitem .@rewards[.@i],.@rewards[.@i+1];
				mes "You have gained an experience and job experience.";
				getexp 5096730 * get_instance_var("mode"),3644000 * get_instance_var("mode");
			}
			next;
			.@reputation = 5 + (get_instance_var("mode") * 5);
			mes "^0000CDYour Ice Castle reputation has increased by " + .@reputation + " points.^000000";
			next;
		}
		mes "^0000CDWhen the airship core went into dormant mode, the way out was activated.^000000";
		next;
		if(select("Leave.:Stay a bit.") == 2)
			end;
		warp "jor_nest",24,248;
		end;
	}
	end;
}

1@whl,138,173,1	script	#19_g_1	20847,3,3,{
	end;
	
OnTouchNPC:
	if(!get_instance_var("green")) end;
	instance_enable(strnpcinfo(0),false);
	set_instance_var("green",get_instance_var("green") - 1);
	if(get_instance_var("green") > 0) end;
	if(!unitexists(get_instance_var("gid"))) end;
	specialeffect3 EF_TETRA,AREA,get_instance_var("gid");
	.@map$ = get_instance_var("map$");
	.@x = rand(130,189);
	.@y = rand(140,185);
	while(!checkcell(.@map$,.@x,.@y,cell_chkpass)){
		.@x = rand(130,189);
		.@y = rand(140,185);
		sleep 10;
	}
	unitwarp get_instance_var("gid"),.@map$,.@x,.@y;
	unitskilluseid get_instance_var("gid"),"NPC_RELIEVE_OFF",1;
	unitskilluseid get_instance_var("gid"),"NPC_RELIEVE_ON",9;
end;
}

1@whl,138,146,1	duplicate(#19_g_1)	#19_g_2	20847,3,3
1@whl,181,146,1	duplicate(#19_g_1)	#19_g_3	20847,3,3
1@whl,181,173,1	duplicate(#19_g_1)	#19_g_4	20847,3,3

1@whl,173,161,1	script	#19_y_1	20847,3,3,{
	end;
	
OnTouchNPC:
	if(!get_instance_var("yellow")) end;
	instance_enable(strnpcinfo(0),false);
	set_instance_var("yellow",get_instance_var("yellow") - 1);
	if(get_instance_var("yellow") > 0) end;
	if(!unitexists(get_instance_var("gid"))) end;
	specialeffect3 EF_TETRA,AREA,get_instance_var("gid");
	.@map$ = get_instance_var("map$");
	.@x = rand(130,189);
	.@y = rand(140,185);
	while(!checkcell(.@map$,.@x,.@y,cell_chkpass)){
		.@x = rand(130,189);
		.@y = rand(140,185);
		sleep 10;
	}
	unitwarp get_instance_var("gid"),.@map$,.@x,.@y;
	//setunitdata get_instance_var("gid"),UMOB_VSIZE,Size_Small;
	sc_end SC_BERSERK,get_instance_var("gid");
end;
}

1@whl,146,161,1	duplicate(#19_y_1)	#19_y_2	20847,3,3