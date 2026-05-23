1@iwp,1,1,0	script	#iwin_patrol_control	-1,{
	end;
	
OnInstanceInit:
	set_instance_var("map$",instance_mapname("1@iwp"));
	set_instance_var("iwin",0);
	cloaknpc(instance_npcname("Mysterious Young Man#dlunch"),true);
	instance_event("#iwin_patrol_mob","OnSummon",false);
	instance_enable("Patrol Leader#ediwp",false);
	set_instance_var("npc_1",0);
	set_instance_var("npc_2",0);
	set_instance_var("npc_3",0);
	set_instance_var("npc_4",0);
	set_instance_var("talk",0);
	
	setunittitle getnpcid(0,instance_npcname("Patrol Leader#stiwp")),"<Eagle Patrol>";
	setunittitle getnpcid(0,instance_npcname("Corporal Anvil#iwp_1")),"<Eagle Patrol>";
	setunittitle getnpcid(0,instance_npcname("New Recruit#iwp_2")),"<Eagle Patrol>";
	setunittitle getnpcid(0,instance_npcname("Private Loloru#iwp_3")),"<Eagle Patrol>";
	setunittitle getnpcid(0,instance_npcname("Private Horiryu#iwp_4")),"<Eagle Patrol>";
end;
}

1@iwp,260,390,4	script	Mysterious Young Man#dlunch	4_EP19_LUNCH,{
	if(isbegin_quest(5983) == 0){
		cutin "ep19_lunch01.png",2;
		mes "[Mysterious Young Man]";
		mes "Ugh! Are you here to get me?";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "No.";
		next;
		mes "[Mysterious Young Man]";
		mes "Oh! What a relief. I ran all the way here.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Ran away? Come to think of it, isn't your clothes too cold to be here?";
		next;
		mes "[Mysterious Young Man]";
		mes "Cold? Cold.. cold..";
		next;
		mes "[Mysterious Young Man]";
		mes "Oh! You're talking about cooling down. If that's what you're talking about, It's fine.";
		next;
		mes "[Mysterious Young Man]";
		mes "I was designed to withstand such conditions.";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "Huh? Designed? What are you talking about?";
		next;
		cutin "ep19_lunch02.png",2;
		mes "[Mysterious Young Man]";
		mes "I'm an experiment made by the Illusion.";
		next;
		mes "[Mysterious Young Man]";
		mes "I'm an improved version of specimen #000001 that the scientists accidentally made.";
		next;
		mes "[Mysterious Young Man]";
		mes "Compared to specimen #000001, you can say that I'm superior to him about 210426 times !";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "I.. I see. How did you get here?";
		next;
		cutin "ep19_lunch01.png",2;
		mes "[Mysterious Young Man]";
		mes "Oh! That's a sharp question. Abot how? I'm not sure about that either.";
		next;
		mes "[Mysterious Young Man]";
		mes "I arrived here after I ran away from the scientists who where doing other experiments and it got out a little out of control.";
		next;
		mes "[Mysterious Young Man]";
		mes "I don't think it's safe here either, considering the birds are always patrolling.";
		next;
		mes "[Mysterious Young Man]";
		mes "I think the plains is safer. I need to go to the plains now. I'll see you again when I get the chance.";
		next;
		mes "[Mysterious Young Man]";
		mes "It was nice talking with you.";
		next;
		cutin "",255;
		cloaknpc(instance_npcname("Mysterious Young Man#dlunch"),true,getcharid(0));
		setquest 5983;
		mes "The young man disappeared just like that. Didn't he say that he was going to the Frozen Scale Plain? Let's visit him when we have the time.";
		close;
	}
	cloaknpc(instance_npcname("Mysterious Young Man#dlunch"),true,getcharid(0));
	end;
	
OnInstanceInit:
	questinfo QTYPE_QUEST,QMARK_NONE,"isbegin_quest(5983) == 0";
end;
}

1@iwp,375,229,3	script	Patrol Leader#stiwp	21518,{
	if(!is_party_leader()) end;
	if(get_instance_var("iwin")) end;
	if(isbegin_quest(5972) == 1){
		mes "[Patrol Leader]";
		mes "It's your first time, so let me explain.";
		next;
		mes "[Patrol Leader]";
		mes "So you'll walk with out reliable eagle patrols.. Also hmm..";
		next;
	}
	mes "[Patrol Leader]";
	mes "You can go in an independent patrol. Well the actual patrol will be done by the units..";
	next;
	mes "[Patrol Leader]";
	mes "When patrolling it's more safe to do it in groups.";
	next;
	mes "[Patrol Leader]";
	mes "Then go!";
	close2;
	set_instance_var("iwin",1);
	instance_event(strnpcinfo(0),"OnWalk",false);
	instance_event("Corporal Anvil#iwp_1","OnWalk",false);
	instance_event("#iwp_talk","OnTalk",false);
	sleep 400;
	instance_event("New Recruit#iwp_2","OnWalk",false);
	sleep 400;
	instance_event("Private Loloru#iwp_3","OnWalk",false);
	sleep 400;
	instance_event("Private Horiryu#iwp_4","OnWalk",false);
	end;
	
OnWalk:
	unitwalk getnpcid(0),383,229,instance_npcname(strnpcinfo(0)) + "::OnDisable";
end;
	
OnDisable:
	instance_disable(strnpcinfo(0));
end;
}


1@iwp,1,1,0	script	#iwp_talk	HIDDEN_WARP_NPC,{
	end;
	
OnTalk:
	.@npc$ = instance_npcname("Corporal Anvil#iwp_1");
	.@gid = getnpcid(0,.@npc$);
	setarray .@x,370,369,298,286,286,260,135,143,117,128,128,129,220,243;
	setarray .@y,222,219,180,164,151,90,114,124,164,196,222,252,273,359;
	while(!get_instance_var("patrol_end")){
		sleep 10;
		getunitdata .@gid,.@data;
		if(.@data[UNPC_X] == .@x[.@z] && .@data[UNPC_Y] == .@y[.@z])
			instance_event(strnpcinfo(0),"OnUnitTalk",false);
		else
			continue;
		.@z++;
	}
end;

OnUnitTalk:
	set_instance_var("talk",get_instance_var("talk") + 1);
	switch(get_instance_var("talk")){
		case 1:
			npctalk "Anvil: Captain Hororyo, Corporal Anvil! Will begin to work.",instance_npcname("Corporal Anvil#iwp_1");
			break;
			
		case 2:
			npctalk "Anvil: New Recruits!",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: New Recruit! Yes, corporal anvil.",instance_npcname("New Recruit#iwp_2");
			sleep 2000;
			npctalk "Anvil: It'll be windy from here, open your feathers well.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: Understood.",instance_npcname("New Recruit#iwp_2");
			sleep 2000;
			npctalk "Anvil: Also you too..",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: Yes! Corporal Anvil.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Yes! Corporal Anvil.",instance_npcname("Private Horiryu#iwp_4");
			break;
			
		case 3:
			npctalk "Anvil: Be careful, there are sharp ices in this place.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Anvil: You'll lose a lump of feather if the ice scratches you..",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: Yes!",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: Yes!",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Yes!",instance_npcname("Private Horiryu#iwp_4");
			break;
			
		case 4:
			npctalk "Horiryu: (Whisper)",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Loloru: (Quietly Talking)",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Let's avoid chatting while patrolling.",instance_npcname("Corporal Anvil#iwp_1");
			npctalk "Loloru: ...",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: ...",instance_npcname("Private Horiryu#iwp_4");
			break;
			
		case 5:
			npctalk "Loloru: Hey! New Recruit!",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Recruit: New Recruit! Yes!",instance_npcname("New Recruit#iwp_2");
			sleep 2000;
			npctalk "Loloru: You don't have to go that far..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			emotion ET_CRY,getnpcid(0,instance_npcname("Private Loloru#iwp_3"));
			npctalk "Loloru: I'm just glad to have a new comer..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Aren't you cold?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 550;
			npctalk "Recruit: All fine.",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: All fine.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: All fine.",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			emotion ET_QUESTION,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			npctalk "Anvil: What? Am I the only one who's cold?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			emotion ET_QUESTION,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			npctalk "Anvil: Have you seen any Rgans on the hills?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: I haven't seen one.",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: I haven't seen either.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Same here~",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: We are on the open plains so I guess it won't come down here.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Horiryu: (Whisper)",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Loloru: (Quietly Talking)",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: No pointless chatting while patrolling.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: ...",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: ...",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: I heard this before I came out from the office, this week special will be Calamaring.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			emotion ET_SURPRISE,getnpcid(0,instance_npcname("New Recruit#iwp_2"));
			emotion ET_SURPRISE,getnpcid(0,instance_npcname("Private Loloru#iwp_3"));
			emotion ET_SURPRISE,getnpcid(0,instance_npcname("Private Horiryu#iwp_4"));
			npctalk "Horiryu: Isn't that hard to catch? Did someone caught it using a spear?",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: I don't know.. Just enjoy it when it's there~",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: It just a thought, but isn't that for the executives only?",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Ah! I didn't think of that..",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			emotion ET_OTL,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			emotion ET_OTL,getnpcid(0,instance_npcname("New Recruit#iwp_2"));
			emotion ET_OTL,getnpcid(0,instance_npcname("Private Loloru#iwp_3"));
			emotion ET_OTL,getnpcid(0,instance_npcname("Private Horiryu#iwp_4"));
			sleep 3000;
			npctalk "Anvil: Look above you when you pass under the big stone.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Anvil: Sometimes frozen icecles fall from the stone. It would hurt if it hits you, right?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: Yes!",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: Yes!",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Yes!",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: Let's be careful. It's a loss if someone get hurts.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: Yes!",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: Yes!",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Yes!",instance_npcname("Private Horiryu#iwp_4");
			break;
			
		case 6:
			npctalk "Loloru: Hey! New Recruit!",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Recruit: New Recruit! Yes!",instance_npcname("New Recruit#iwp_2");
			sleep 2000;
			npctalk "Loloru: You don't have to go that far..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			emotion ET_CRY,getnpcid(0,instance_npcname("Private Loloru#iwp_3"));
			npctalk "Loloru: I'm just glad to have a new comer..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Horiryu: (Whisper)",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Loloru: (Quietly Talking)",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Let's avoid chatting while patrolling.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: ...",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: ...",instance_npcname("Private Horiryu#iwp_4");
			sleep 3000;
			npctalk "Loloru: Corporal Anvil, I have a question.",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: ?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: I talked to the supply officer..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Oh, you're talking to that agent?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: Yes I am. Is there something wrong?",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: I don't know but.. just forget it.",instance_npcname("Corporal Anvil#iwp_1");
			break;
			
		case 7:
			npctalk "Horiryu: (Whisper)",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Loloru: (Quietly Talking)",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Let's avoid chatting while patrolling.",instance_npcname("Corporal Anvil#iwp_1");
			npctalk "Loloru: ...",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: ...",instance_npcname("Private Horiryu#iwp_4");
			break;
			
		case 8:
			npctalk "Anvil: Watch your head!",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: Watch your head!",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: Watch your head!",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Watch your head!",instance_npcname("Private Horiryu#iwp_4");
			break;
			
		case 9:
			npctalk "Anvil: Aren't you cold?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 550;
			npctalk "Recruit: All fine.",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: All fine.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: All fine.",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			emotion ET_QUESTION,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			npctalk "Anvil: What? Am I the only one who's cold?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			emotion ET_QUESTION,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			npctalk "Anvil: Have you seen any Rgans on the hills?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: I haven't seen one.",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: I haven't seen either.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Same here~",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: We are on the open plains so I guess it won't come down here.",instance_npcname("Corporal Anvil#iwp_1");
			break;
			
		case 10:
			npctalk "Anvil: I heard this before I came out from the office, this week special will be Calamaring.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			emotion ET_SURPRISE,getnpcid(0,instance_npcname("New Recruit#iwp_2"));
			emotion ET_SURPRISE,getnpcid(0,instance_npcname("Private Loloru#iwp_3"));
			emotion ET_SURPRISE,getnpcid(0,instance_npcname("Private Horiryu#iwp_4"));
			npctalk "Horiryu: Isn't that hard to catch? Did someone caught it using a spear?",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: I don't know.. Just enjoy it when it's there~",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: It just a thought, but isn't that for the executives only?",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Ah! I didn't think of that..",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			emotion ET_OTL,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			emotion ET_OTL,getnpcid(0,instance_npcname("New Recruit#iwp_2"));
			emotion ET_OTL,getnpcid(0,instance_npcname("Private Loloru#iwp_3"));
			emotion ET_OTL,getnpcid(0,instance_npcname("Private Horiryu#iwp_4"));
			break;
			
		case 11:
			emotion ET_QUESTION,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			npctalk "Anvil: Have you seen any Rgans on the hills?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: I haven't seen one.",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: I haven't seen either.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Same here~",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: We are on the open plains so I guess it won't come down here.",instance_npcname("Corporal Anvil#iwp_1");
			break;
			
		case 12:
			npctalk "Loloru: Hey! New Recruit!",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Recruit: New Recruit! Yes!",instance_npcname("New Recruit#iwp_2");
			sleep 2000;
			npctalk "Loloru: You don't have to go that far..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			emotion ET_CRY,getnpcid(0,instance_npcname("Private Loloru#iwp_3"));
			npctalk "Loloru: I'm just glad to have a new comer..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Horiryu: (Whisper)",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Loloru: (Quietly Talking)",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Let's avoid chatting while patrolling.",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: ...",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: ...",instance_npcname("Private Horiryu#iwp_4");
			sleep 3000;
			npctalk "Loloru: Corporal Anvil, I have a question.",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: ?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: I talked to the supply officer..",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Oh, you're talking to that agent?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Loloru: Yes I am. Is there something wrong?",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: I don't know but.. just forget it.",instance_npcname("Corporal Anvil#iwp_1");
			break;
			
		case 13:
			npctalk "Horiryu: (Whisper)",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Loloru: (Quietly Talking)",instance_npcname("Private Loloru#iwp_3");
			sleep 2000;
			npctalk "Anvil: Let's avoid chatting while patrolling.",instance_npcname("Corporal Anvil#iwp_1");
			npctalk "Loloru: ...",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: ...",instance_npcname("Private Horiryu#iwp_4");
			break;
			
		case 14:
			npctalk "Anvil: Aren't you cold?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 550;
			npctalk "Recruit: All fine.",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: All fine.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: All fine.",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			emotion ET_QUESTION,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			npctalk "Anvil: What? Am I the only one who's cold?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			emotion ET_QUESTION,getnpcid(0,instance_npcname("Corporal Anvil#iwp_1"));
			npctalk "Anvil: Have you seen any Rgans on the hills?",instance_npcname("Corporal Anvil#iwp_1");
			sleep 2000;
			npctalk "Recruit: I haven't seen one.",instance_npcname("New Recruit#iwp_2");
			npctalk "Loloru: I haven't seen either.",instance_npcname("Private Loloru#iwp_3");
			npctalk "Horiryu: Same here~",instance_npcname("Private Horiryu#iwp_4");
			sleep 2000;
			npctalk "Anvil: We are on the open plains so I guess it won't come down here.",instance_npcname("Corporal Anvil#iwp_1");
			break;
	}
end;
}

1@iwp,370,224,7	script	Corporal Anvil#iwp_1	21517,{
	end;
	
OnWalk:
	if(get_instance_var("patrol_end")) end;
	.@id = atoi(replacestr(strnpcinfo(2),"iwp_",""));
	.@var = get_instance_var("npc_" + .@id);
	set_instance_var("npc_" + .@id,.@var + 1);
	npcspeed 400;
	switch(.@var + 1){
		case 1: unitwalk getnpcid(0),370,219,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 2: unitwalk getnpcid(0),355,219,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 3: unitwalk getnpcid(0),340,219,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 4: unitwalk getnpcid(0),332,227,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 5: unitwalk getnpcid(0),309,227,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 6: unitwalk getnpcid(0),309,202,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 7: unitwalk getnpcid(0),309,191,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 8: unitwalk getnpcid(0),286,168,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 9: unitwalk getnpcid(0),286,140,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 10:  unitwalk getnpcid(0),306,125,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 11:  unitwalk getnpcid(0),306,120,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 12:  unitwalk getnpcid(0),306,116,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 13:  unitwalk getnpcid(0),311,111,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 14:  unitwalk getnpcid(0),311,93,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 15:  unitwalk getnpcid(0),306,88,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 16:  
			set_instance_var("cp",1);
			unitwalk getnpcid(0),306,80,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
			end;
			
		case 17:  unitwalk getnpcid(0),279,80,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 18:  unitwalk getnpcid(0),269,90,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 19:  unitwalk getnpcid(0),253,90,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 20:  unitwalk getnpcid(0),242,101,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 21:  unitwalk getnpcid(0),230,101,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 22:  unitwalk getnpcid(0),216,87,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 23:  unitwalk getnpcid(0),192,70,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 24:  unitwalk getnpcid(0),192,56,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 25:  unitwalk getnpcid(0),167,56,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 26:  unitwalk getnpcid(0),142,56,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 27:  unitwalk getnpcid(0),115,56,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 28:  unitwalk getnpcid(0),115,77,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 29:  
			set_instance_var("cp",2);
			unitwalk getnpcid(0),131,93,instance_npcname(strnpcinfo(0)) + "::OnWalk"; 
			end;
			
		case 30:  unitwalk getnpcid(0),131,110,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 31:  unitwalk getnpcid(0),143,122,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 32:  unitwalk getnpcid(0),143,138,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 33:  unitwalk getnpcid(0),142,139,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 34:  unitwalk getnpcid(0),134,139,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 35:  unitwalk getnpcid(0),117,156,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 36:  unitwalk getnpcid(0),117,169,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 37:  unitwalk getnpcid(0),128,180,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 38:  unitwalk getnpcid(0),128,205,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 39:  unitwalk getnpcid(0),128,222,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 40:  unitwalk getnpcid(0),139,233,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 41:  unitwalk getnpcid(0),139,242,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 42:  unitwalk getnpcid(0),129,252,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 43:  unitwalk getnpcid(0),129,260,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 44:  unitwalk getnpcid(0),150,281,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 45:  unitwalk getnpcid(0),153,281,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 46:  unitwalk getnpcid(0),153,284,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 47:  
			set_instance_var("cp",3);
			unitwalk getnpcid(0),160,291,instance_npcname(strnpcinfo(0)) + "::OnWalk"; 
			end;
			
		case 48:  unitwalk getnpcid(0),145,306,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 49:  unitwalk getnpcid(0),145,312,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 50:  unitwalk getnpcid(0),137,320,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 51:  unitwalk getnpcid(0),137,327,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 52:  unitwalk getnpcid(0),157,327,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 53:  unitwalk getnpcid(0),177,327,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 54:  unitwalk getnpcid(0),180,327,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 55:  unitwalk getnpcid(0),186,333,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 56:  unitwalk getnpcid(0),194,333,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 57:  unitwalk getnpcid(0),194,346,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 58:  unitwalk getnpcid(0),202,354,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 59:  unitwalk getnpcid(0),215,354,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 60:  unitwalk getnpcid(0),215,373,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 61:  
			set_instance_var("cp",4);
			unitwalk getnpcid(0),240,373,instance_npcname(strnpcinfo(0)) + "::OnWalk"; 
			end;
			
		case 62:  unitwalk getnpcid(0),240,362,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 63:  unitwalk getnpcid(0),244,358,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 64:  unitwalk getnpcid(0),244,343,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 65:  unitwalk getnpcid(0),252,335,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 66:  
			set_instance_var("cp",5);
			unitwalk getnpcid(0),264,335,instance_npcname(strnpcinfo(0)) + "::OnWalk"; 
			end;
		case 67:  unitwalk getnpcid(0),264,320,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 68:  unitwalk getnpcid(0),287,320,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 69:  unitwalk getnpcid(0),294,314,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 70:  unitwalk getnpcid(0),298,314,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 71:  unitwalk getnpcid(0),299,315,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 72:  unitwalk getnpcid(0),306,315,instance_npcname(strnpcinfo(0)) + "::OnWalk"; end;
		case 73:  
			switch(.@id){
				case 1:
					unitwalk getnpcid(0),306,305,instance_npcname(strnpcinfo(0)) + "::OnWalk";
					break;
				case 2:
					unitwalk getnpcid(0),306,308,instance_npcname(strnpcinfo(0)) + "::OnWalk";
					break;
				case 3:
					unitwalk getnpcid(0),306,311,instance_npcname(strnpcinfo(0)) + "::OnWalk";
					break;
				case 4:
					unitwalk getnpcid(0),306,314,instance_npcname(strnpcinfo(0)) + "::OnWalk";
					break;
			}
	}
end;

OnInstanceInit:
	npcspeed 400;
end;
}

1@iwp,370,227,7	duplicate(Corporal Anvil#iwp_1)	New Recruit#iwp_2	21514,{ OnInstanceInit: npcspeed 400; }
1@iwp,370,230,7	duplicate(Corporal Anvil#iwp_1)	Private Loloru#iwp_3	21515,{ OnInstanceInit: npcspeed 400; }
1@iwp,370,233,7	duplicate(Corporal Anvil#iwp_1)	Private Horiryu#iwp_4	21516,{ OnInstanceInit: npcspeed 400; }


1@iwp,279,80,0	script	checkpoint#pcp01	HIDDEN_WARP_NPC,10,10,{
	end;
	
OnTouch:
	if(get_instance_var("cp") < 1) end;
	if(get_instance_var("cp1_" + getcharid(0)) == 0){
		emotion ET_SURPRISE,getcharid(3);
		set_instance_var("cp1_" + getcharid(0),1);
	}
end;
}

1@iwp,131,110,0	script	checkpoint#pcp02	HIDDEN_WARP_NPC,10,10,{
	end;
	
OnTouch:
	if(get_instance_var("cp") < 2) end;
	if(get_instance_var("cp1_" + getcharid(0)) < 1) end;
	emotion ET_SURPRISE,getcharid(3);
	set_instance_var("cp2_" + getcharid(0),1);
end;
}

1@iwp,145,306,0	script	checkpoint#pcp03	HIDDEN_WARP_NPC,10,10,{
	end;
	
OnTouch:
	if(get_instance_var("cp") < 3) end;
	if(get_instance_var("cp2_" + getcharid(0)) < 1) end;
	emotion ET_SURPRISE,getcharid(3);
	set_instance_var("cp3_" + getcharid(0),1);
end;
}

1@iwp,240,373,0	script	checkpoint#pcp04	HIDDEN_WARP_NPC,10,10,{
	end;
	
OnTouch:
	if(isbegin_quest(5983) == 0){
		navigateto(get_instance_var("map$"),260,390);
		cloaknpc(instance_npcname("Mysterious Young Man#dlunch"),false,getcharid(0));
		sleep2 1000;
		viewpoint 1,260,390,1,0xFF00FF;
		unittalk getcharid(3),strcharinfo(0) + " : Huh?! Isn't there something in the corner there?",bc_self;
	}
	if(get_instance_var("cp") < 4) end;
	if(get_instance_var("cp3_" + getcharid(0)) < 1) end;
	emotion ET_SURPRISE,getcharid(3);
	set_instance_var("cp4_" + getcharid(0),1);
end;
}

1@iwp,264,310,0	script	checkpoint#pcp05	HIDDEN_WARP_NPC,10,20,{
	end;
	
OnTouch:
	if(get_instance_var("cp") < 5) end;
	if(get_instance_var("cp4_" + getcharid(0)) < 1) end;
	if(!get_instance_var("leader")){
		set_instance_var("leader",1);
		instance_enable("Patrol Leader#ediwp",true);
	}
	if(isbegin_quest(5972) == 1)
		completequest 5972;
	if(isbegin_quest(5974) == 1){
		emotion ET_SURPRISE,getcharid(3);
		erasequest 5974;
		setquest 5975;
		questinfo_refresh;
	}
end;
}

1@iwp,1,1,0	script	#iwin_patrol_mob	-1,{
	end;
	
OnSummon:
	setarray .@mobs,21525,30,21524,30,21523,20; //Doesn't respawn on kill
	for(.@i = 0; .@i < getarraysize(.@mobs); .@i += 2){
		for(.@x = 0; .@x < .@mobs[.@i+1]; .@x++)
			monster get_instance_var("map$"),0,0,"--ja--",.@mobs[.@i],1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
	}
end;

OnMobKill:
end;
}

1@iwp,385,229,0	script	#iwin_patrol_warp_1	WARPNPC,2,2,{
	end;
	
OnTouch:
	mes "[Guide]";
	mes "Iwin Patrol";
	mes "You're going out of the memorial dungeon. Some progress may not be saved.";
	next;
	if(select("Don't go out.:Go out.") == 1) end;
	switch(atoi(replacestr(strnpcinfo(2),"iwin_patrol_warp_",""))){
		case 1:
			warp "icecastle",20,123;
			break;
			
		case 2:
			warp "jor_tail",98,284;
			break;
			
		case 3:
			warp "jor_tail",139,280;
			break;
			
		case 4:
			warp "jor_tail",219,291;
			break;
			
		case 5:
			warp "jor_back2",229,17;
			break;
	}
end;
}

1@iwp,98,10,0	duplicate(#iwin_patrol_warp_1)	#iwin_patrol_warp_2	WARPNPC,2,2
1@iwp,131,10,0	duplicate(#iwin_patrol_warp_1)	#iwin_patrol_warp_3	WARPNPC,2,2
1@iwp,222,19,0	duplicate(#iwin_patrol_warp_1)	#iwin_patrol_warp_4	WARPNPC,2,2
1@iwp,214,390,0	duplicate(#iwin_patrol_warp_1)	#iwin_patrol_warp_5	WARPNPC,2,2

1@iwp,301,315,6	script	Patrol Leader#ediwp	EP19_NPC_IWIN_SR,{
	if(isbegin_quest(5975) == 1){
		erasequest 5975;
		setquest 5973;
		addreputation("Ice Castle",5);
		getitem 1000608,2 + (getreputation("Ice Castle") >= 3000 ? 1 : 0);
		mes "[Patrol Leader]";
		mes "Today's patrol was hard. Here's a small reward for this patrol.";
		next;
	}
	mes "[Patrol Leader]";
	mes "Are you going anywhere? If you want to return to the Ice Castle, I'll take you there.";
	next;
	if(select("Return to the Ice Castle..:I have something to do.") == 2){
		mes "[Patrol Leader]";
		mes "Then, should we split up here? I'll see you tomorrow.";
		close;
	}
	mes "[Patrol Leader]";
	mes "Then, shall we go to the Ice Castle?";
	close2;
	warp "icecastle",20,123;
	end;
}