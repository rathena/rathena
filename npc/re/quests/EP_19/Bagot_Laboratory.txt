1@jorlab,1,1,1	script	#191_jlab_control	HIDDEN_WARP_NPC,{
	end;
	
OnInstanceInit:
	set_instance_var("map$",instance_mapname("1@jorlab"));
	instance_enable("Juncea#ep19re3",false);
	instance_enable("Juncea#ep19re4",false);
	instance_enable("Summoning Device#ep19re1",false);
	set_instance_var("lab",0);
end;
}

1@jorlab,58,45,0	script	#jorlab_hw1	HIDDEN_WARP_NPC,4,4,{
	end;
	
OnTouch:
	if(!is_party_leader()) end;
	if(isbegin_quest(16660) == 1)
		instance_enable("Juncea#ep19re3",true);
	else
		instance_enable("Summoning Device#ep19re1",true);
end;
}

1@jorlab,58,67,3	script	Juncea#ep19re3	4_EP19_JUNCEA,{
	if(isbegin_quest(16660) == 0 || isbegin_quest(16660) == 2 || !is_party_leader()) end; //safecheck
	if(get_instance_var("lab") == 0){
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Oh, you're here. Senekiogand. Welcome.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "What do you want to talk about with just the two of us?";
		mes "I also have a question for you human Juncea.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Oh, I knew it.";
		mes "But listen to me first.";
		next;
		mes "[Juncea]";
		mes "You too, must be going through a lot.";
		mes "It must be hard to act when you don't fit in.";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "What do you mean? You're strange. Juncea is human. I'm scared.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "What are you afraid of? You're good at pretending that we're close. Why are you acting scared all of a sudden?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "I didn't pretend that we're close. I really wanted to be close with human Juncea.";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Hoho, is that why you're gossiping so hard on me to Bagot? That's a good way, that's a good way.";
		mes "There's no other way to curse a person other than to get close to them.";
		next;
		mes "[Juncea]";
		mes "But, you picked the wrong person. Foolish.";
		next;
		mes "[Juncea]";
		mes "The great... Bagot, Do you think... that I'm... going to fall for it?";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "The great Bagot?";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "I..., Who am I... what is this.......";
		next;
		mes "[Juncea]";
		mes "I am Juncea... you're trying to ... use me.";
		next;
		mes "[Juncea]";
		mes "So I.......";
		next;
		mes "[Juncea]";
		mes "I... Who am I? This is......";
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(Juncea is strange..., did Bagot do something with her?)";
		next;
		cutin "ep19_juncea04.png",2;
		mes "[Juncea]";
		mes "Even if you run... You are already in the palm of Bagot's hand.";
		next;
		cutin "ep19_juncea00.png",1;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA_M);
		mes "[Juncea]";
		mes "Here... I'll take care of it...!";
		set_instance_var("lab",1);
		next;
		cutin "",255;
		mes "[" + strcharinfo(0) + "]";
		mes "(...I don't have the time to call for help! I have to do something by myself!)";
		close;
	}
	if(get_instance_var("lab") == 1){
		cutin "ep19_juncea04.png",2;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA);
		mes "[Juncea]";
		mes "Ahh... Gahhhhhh!";
		next;
		cutin "ep19_juncea00.png",1;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA_M);
		mes "[Juncea]";
		mes "Everything is... is Bagot's... fault...!";
		next;
		cutin "ep19_juncea04.png",2;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA);
		mes "[Juncea]";
		mes "Ahhhhhhgg!";
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA_M);
		sleep2 100;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA);
		sleep2 100;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA_M);
		sleep2 100;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA);
		next;
		cutin "ep19_juncea00.png",1;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_JUNCEA_M);
		mes "[Juncea]";
		mes "As you wish... I will punish you!";
		close2;
		cutin "",255;
		set_instance_var("lab",2);
		instance_disable(strnpcinfo(0));
		instance_event("#191_jlab_mob","OnStorySummon",false);
		end;
	}
	end;
}

1@jorlab,58,67,3	script	Juncea#ep19re4	4_EP19_JUNCEA_D,{
	if(isbegin_quest(16660) == 1){
		mes "[" + strcharinfo(0) + "]";
		mes "(I managed to overpower her. Juncea lost her mind...)";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(Bagot... what have you done to Juncea? Did he turned her into a monster?)";
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(If everything went well, I would have convinced her to go outside, but I'll have to take her outside even in this state.)";
		instance_announce false,"Bagot : Oh, it looks like Juncea is not good enough as a vessel.",bc_map,0x00FFFF;
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(As planned, I'll use the Rgan transformation scroll to Juncea...)";
		instance_announce false,"Bagot : My prediction was wrong this time.",bc_map,0x00FFFF;
		next;
		setnpcdisplay(instance_npcname(strnpcinfo(0)),4_EP19_RGAN_R1);
		mes "[" + strcharinfo(0) + "]";
		mes "(First, let's take her back to the village.)";
		instance_announce false,"Bagot : Anyway, to think that there's a scroll like that... I knew that you're deceiving me, but the existence of such thing is interesting.",bc_map,0x00FFFF;
		next;
		mes "[" + strcharinfo(0) + "]";
		mes "(I think I've heard something... Am I just hearing things?)";
		instance_announce false,"Bagot : *Laughing*.",bc_map,0x00FFFF;
		completequest 16660;
		setquest 16661;
		close2;
		warp "icecastle",27,123;
		end;
	}
	end;
	
OnInstanceInit:
	questinfo QTYPE_QUEST,QMARK_YELLOW,"isbegin_quest(16660) == 1";
	instance_enable("Juncea#ep19re4",false);
end;
}


1@jorlab,58,67,3	script	Summoning Device#ep19re1	PORTAL,{
	if(get_instance_var("boss_killed")){
		mes "[Summoning Device]";
		mes "Battle completed, data has been saved.";
		mes "Press the Yes button to leave the laboratory.";
		next;
		if(select("Yes:No") == 2){
			mes "[Summoning Device]";
			mes "Conversation Ended.";
			close;
		}
		mes "[Summoning Device]";
		mes "Ready to move. In 3,2,1.";
		close2;
		warp "jor_nest",63,257;
		end;
	}
	if(get_instance_var("summoned")) end;
	if(!is_party_leader()) end;
	if(get_instance_var("unknown_error")){ //IDK What's this for but I got this error in kRO
		mes "[Summoning Device]";
		mes "- I'm currently talking to another party member. Listen to them for a moment. -";
		mes "- If the conversation is ended due to an error, the conversation will revert back to an available state in 5 minutes after it has occured. -";
		close;
	}
	mes "[Summoning Device]";
	mes "Start the Battle Simulation.";
	mes "When you are ready, click the Yes button.";
	next;
	mes "[Summoning Device]";
	mes "Are you ready?";
	next;
	if(select("Yes:No") == 2){
		mes "[Summoning Device]";
		mes "Conversation Ended.";
		close;
	}
	mes "[Summoning Device]";
	mes "Summoning the Simulation Juncea.";
	mes "In 3, 2, 1. Ready to summon.";
	close2;
	instance_disable(strnpcinfo(0));
	instance_event("#191_jlab_mob","OnBossSummon",false);
	end;
}

1@jorlab,1,1,1	script	#191_jlab_mob	HIDDEN_WARP_NPC,{
	end;
	
OnStorySummon:
	monster get_instance_var("map$"),58,67,"Monster Juncea",21532,1,instance_npcname(strnpcinfo(0)) + "::OnStoryKill";
end;

OnStoryKill:
	instance_enable("Juncea#ep19re4",true);
	questinfo_refresh;
end;

OnBossSummon:
	set_instance_var("summoned",1);
	monster get_instance_var("map$"),58,67,"Simulation Juncea",21533,1,instance_npcname(strnpcinfo(0)) + "::OnBossKill";
	set_instance_var("gid",$@mobid[0]);
end;

OnBossKill:
	instance_enable("Summoning Device#ep19re1",true);
	set_instance_var("boss_killed",1);
end;
}