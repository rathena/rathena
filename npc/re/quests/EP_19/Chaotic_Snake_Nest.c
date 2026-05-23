1@jorchs,1,1,1	script	#191_csn_control	-1,{
	end;
	
OnInstanceInit:
	set_instance_var("map$",instance_mapname("1@jorchs"));
	set_instance_var("csn",0);
	set_instance_var("summon",0);
	set_instance_var("elite",0);
	set_instance_var("clear",0);
	set_instance_var("dev_1",0);
	set_instance_var("dev_2",0);
	set_instance_var("dev_3",0);
	set_instance_var("dev_4",0);
	set_instance_var("dev_5",0);
	set_instance_var("19m00",0);
	set_instance_var("19m01",0);
	set_instance_var("19m10",0);
	set_instance_var("19m11",0);
	
	//= First Wall
	setcell get_instance_var("map$"),139,59,139,59,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),138,58,138,58,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),137,57,137,57,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),139,59,139,59,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),138,58,138,58,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),137,57,137,57,CELL_SHOOTABLE,false;
	//= Second Wall
	setcell get_instance_var("map$"),120,129,120,129,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),121,128,121,128,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),122,127,122,127,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),123,127,123,127,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),124,126,124,126,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),125,125,125,125,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),126,125,126,125,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),120,129,120,129,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),121,128,121,128,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),122,127,122,127,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),123,127,123,127,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),124,126,124,126,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),125,125,125,125,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),126,125,126,125,CELL_SHOOTABLE,false;
	//= Third Wall
	setcell get_instance_var("map$"),249,177,249,177,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),248,176,248,176,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),247,176,247,176,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),246,176,246,176,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),245,176,245,176,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),244,176,244,176,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),243,175,243,175,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),242,174,242,174,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),249,177,249,177,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),248,176,248,176,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),247,176,247,176,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),246,176,246,176,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),245,176,245,176,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),244,176,244,176,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),243,175,243,175,CELL_SHOOTABLE,false;
	setcell get_instance_var("map$"),242,174,242,174,CELL_SHOOTABLE,false;
	//= Fourth Wall
	setcell get_instance_var("map$"),140,178,140,183,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),140,178,140,183,CELL_SHOOTABLE,false;
	//= Last Wall
	setcell get_instance_var("map$"),34,255,43,255,CELL_WALKABLE,false;
	setcell get_instance_var("map$"),34,255,43,255,CELL_SHOOTABLE,false;
			
	instance_enable("Frightened Rgan#19m00",false);
	instance_enable("Frightened Rgan#19m50",false);
	instance_enable("Frightened Rgan#19m01",false);
	instance_enable("Frightened Rgan#19m51",false);
	instance_enable("Stranded Rgan#19m10",false);
	instance_enable("Stranded Rgan#19m60",false);
	instance_enable("Stranded Rgan#19m11",false);
	instance_enable("Stranded Rgan#19m61",false);
	instance_enable("Iwin#19m10",false);
	instance_enable("Iwin#19m11",false);
	instance_enable("Horuru#19m10",false);
	instance_enable("Aurelie#19m10",false);
	instance_enable("Leon#19m10",false);
	instance_enable("Miriam#19m10",false);
	instance_enable("Voglinde#19m10",false);
	instance_enable("Iwin#19m20",false);
	instance_enable("Iwin#19m21",false);
	instance_enable("Iwin#19m22",false);
	instance_enable("Iwin#19m23",false);
	instance_enable("Horuru#19m20",false);
	instance_enable("Horuru#19m21",false);
	instance_enable("Heart Hunter#19m20",false);
	instance_enable("Heart Hunter#19m21",false);
	instance_enable("Heart Hunter#19m22",false);
	instance_enable("Voglinde#19m30",false);
	instance_enable("Miriam#19m30",false);
	instance_enable("Heart Hunter#19m30",false);
	instance_enable("Heart Hunter#19m31",false);
	instance_enable("Heart Hunter#19m32",false);
	instance_enable("#e19p01",false);
	instance_enable("#e19p02",false);
	instance_enable("#e19p03",false);
	instance_enable("Injection Device#19m30",false);
	instance_enable("Injection Device#19m31",false);
	instance_enable("Injection Device#19m32",false);
	instance_enable("Injection Device#19m40",false);
	instance_enable("Heart Hunter#19m40",false);
	instance_enable("Heart Hunter#19m41",false);
	instance_enable("Heart Hunter#19m42",false);
	instance_enable("Leon#19m40",false);
	instance_enable("Aurelie#19m40",false);
	instance_enable("Research Device#19m40",false);
	instance_enable("Research Device#19m41",false);
	instance_enable("Laboratory Traces#19m40",false);
	for(.@i = 0; .@i < 7; .@i++)
		instance_enable("Furious Rgan#19m5" + .@i,false);
	instance_enable("Bagot#19m50",false);
	instance_enable("Leon#19m60",false);
	instance_enable("Horuru#19m60",false);
	instance_enable("Aurelie#19m60",false);
	instance_enable("Voglinde#19m60",false);
	instance_enable("Miriam#19m60",false);
	instance_enable("Central Door#19m61",false);
	instance_enable("#19m60",false);
end;

OnEvent01:
	instance_announce false,"Wait, gather here everyone. We need to sort some things out.",bc_map,0x00FFCC;
	set_instance_var("search",0);
	set_instance_var("csn",2);
	instance_disable("Iwin#19m12");
	instance_disable("Iwin#19m13");
	instance_enable("Iwin#19m10",true);
	instance_enable("Iwin#19m11",true);
	instance_enable("Horuru#19m10",true);
	instance_enable("Aurelie#19m10",true);
	instance_enable("Leon#19m10",true);
	instance_enable("Miriam#19m10",true);
	instance_enable("Voglinde#19m10",true);
end;

OnEvent02:
	set_instance_var("csn",11);
	instance_announce false,"Oh, Everyone, let's take them down.",bc_map,0x00FFCC;
	instance_enable("Heart Hunter#19m40",true);
	instance_enable("Heart Hunter#19m41",true);
	instance_enable("Heart Hunter#19m42",true);
	instance_enable("Leon#19m40",true);
	instance_enable("Aurelie#19m40",true);
end;

OnEvent03:
	instance_announce false,"Who will be the first to arrive?",bc_map,0x00FFCC;
	instance_enable("Heart Hunter#19m50",true);
	instance_enable("Heart Hunter#19m51",true);
	instance_enable("Heart Hunter#19m52",true);
end;
}

1@jorchs,261,34,3	script	Aurelie#19m00	4_EP19_AURELIE,5,5,{
	end;
	
OnTouch:
	if(!is_party_leader()) end;
	if(get_instance_var("csn")) end;
	set_instance_var("csn",1);
	if(191_scroll_check()){
		npctalk "There's no reason to pretend to be a Rgan here.",instance_npcname("Aurelie#19m00");
		sleep 3000;
	}
	npctalk "The Iwin unit that we introduced a while ago already cleared up the surrounding Rgan.",instance_npcname("Horuru#19m00");
	sleep 3000;
	npctalk "We now need to go further inside and clean up.",instance_npcname("Voglinde#19m00");
	sleep 3000;
	npctalk "The goal is the secure Lasgand, Bagot, and the Heart of Ymir.",instance_npcname("Aurelie#19m00");
	sleep 3000;
	npctalk "We have to go in all at once and block their retreat. Speed will be the key.",instance_npcname("Horuru#19m00");
	sleep 3000;
	npctalk "There's only one entrance, but we'll have to check every place in case there is one that we don't know of.",instance_npcname("Aurelie#19m00");
	sleep 3000;
	npctalk "If we want to find their whereabouts, we'll have to clean up the remaining Rgans first.",instance_npcname("Horuru#19m00");
	sleep 3000;
	npctalk "According to the story of the advanced unit, the Rgans are not ordinary, so be prepared.",instance_npcname("Horuru#19m00");
	sleep 3000;
	npctalk "Be careful, the Rgans out there are good at using magic.",instance_npcname("Aurelie#19m00");
	sleep 3000;
	npctalk "Goodluck, everyone!",instance_npcname("Horuru#19m00");
	instance_disable("Aurelie#19m00");
	instance_disable("Leon#19m00");
	instance_disable("Miriam#19m00");
	instance_disable("Voglinde#19m00");
	instance_disable("Horuru#19m00");
	instance_disable("Iwin#19m00");
	instance_disable("Iwin#19m01");
	instance_enable("Frightened Rgan#19m00",true);
	instance_enable("Frightened Rgan#19m50",true);
	instance_enable("Frightened Rgan#19m01",true);
	instance_enable("Frightened Rgan#19m51",true);
	instance_event("#191_csn_mob","OnSummon",false);
end;
}

1@jorchs,260,35,3	duplicate(dummynpc2)	Leon#19m00	4_EP19_LEON
1@jorchs,256,34,5	duplicate(dummynpc2)	Miriam#19m00	4_EP18_MIRIAM
1@jorchs,257,35,5	duplicate(dummynpc2)	Voglinde#19m00	4_EP19_VOGLINDE
1@jorchs,258,32,7	duplicate(dummynpc2)	Horuru#19m00	EP19_NPC_IWIN_SR
1@jorchs,256,31,7	duplicate(dummynpc2)	Iwin#19m00	EP19_NPC_IWIN_SG
1@jorchs,259,31,7	duplicate(dummynpc2)	Iwin#19m01	EP19_NPC_IWIN_SG

1@jorchs,143,60,5	script	Iwin#19m11	EP19_NPC_IWIN_SG,5,5,{
	end;
	
OnTouch:
	if(!is_party_leader()) end;
	if(get_instance_var("csn") == 2){
		set_instance_var("csn",3);
		npctalk "What in the world happened to the Rgans!! I've never seen a Rgan like that in my life!",instance_npcname("Horuru#19m10");
		sleep 2500;
		npctalk "I think they're an intermediate-grade Rgan, I couldn't recognize their shape.",instance_npcname("Leon#19m10");
		sleep 2500;
		npctalk "Why does an intermediate-grade Rgan look like that? Isn't that one a superior one?",instance_npcname("Aurelie#19m10");
		sleep 2500;
		npctalk "I think it has something to do with the Illusions, I think they modified them.",instance_npcname("Miriam#19m10");
		sleep 2500;
		npctalk "Wouldn't modifying make them more better? No matter how much you look at it...",instance_npcname("Horuru#19m10");
		sleep 2500;
		npctalk "They're either just caught up or to be disposed after using them.",instance_npcname("Leon#19m10");
		sleep 2500;
		npctalk "It's unusual, so let's go further inside.",instance_npcname("Voglinde#19m10");
		sleep 3000;
		npctalk "The place is complicated, so we'd better split up and search.",instance_npcname("Horuru#19m10");
		sleep 3000;
		npctalk "The Rgans that Lasgand has gathered is likely to be in his Residence.",instance_npcname("Miriam#19m10");
		sleep 3000;
		npctalk "Bagot was also working on something, so we should check the residence and the laboratories of the Illusion.",instance_npcname("Horuru#19m10");
		sleep 2500;
		npctalk "It's the deepest part of this place.",instance_npcname("Leon#19m10");
		sleep 2500;
		npctalk "Shall we go?",instance_npcname("Aurelie#19m10");
		setcell get_instance_var("map$"),139,59,139,59,CELL_WALKABLE,true;
		setcell get_instance_var("map$"),138,58,138,58,CELL_WALKABLE,true;
		setcell get_instance_var("map$"),137,57,137,57,CELL_WALKABLE,true;
		setcell get_instance_var("map$"),139,59,139,59,CELL_SHOOTABLE,true;
		setcell get_instance_var("map$"),138,58,138,58,CELL_SHOOTABLE,true;
		setcell get_instance_var("map$"),137,57,137,57,CELL_SHOOTABLE,true;		
		instance_enable("Iwin#19m10",false);
		instance_enable("Iwin#19m11",false);
		instance_enable("Horuru#19m10",false);
		instance_enable("Aurelie#19m10",false);
		instance_enable("Leon#19m10",false);
		instance_enable("Miriam#19m10",false);
		instance_enable("Voglinde#19m10",false);
		instance_enable("Iwin#19m21",true);
		instance_enable("Stranded Rgan#19m10",true);
		instance_enable("Stranded Rgan#19m60",true);
		instance_enable("Stranded Rgan#19m11",true);
		instance_enable("Stranded Rgan#19m61",true);
		instance_event("#191_csn_mob","OnSummon",false);
	}
end;
}

1@jorchs,142,58,5	duplicate(Iwin#19m11)	Horuru#19m10	EP19_NPC_IWIN_SR,5,5
1@jorchs,145,59,3	duplicate(dummynpc2)	Aurelie#19m10	4_EP19_AURELIE
1@jorchs,146,58,3	duplicate(dummynpc2)	Leon#19m10	4_EP19_LEON
1@jorchs,145,56,1	duplicate(dummynpc2)	Miriam#19m10	4_EP18_MIRIAM
1@jorchs,143,56,1	duplicate(dummynpc2)	Voglinde#19m10	4_EP19_VOGLINDE
1@jorchs,140,57,5	duplicate(dummynpc2)	Iwin#19m10	EP19_NPC_IWIN_SG
1@jorchs,140,56,5	duplicate(dummynpc2)	Iwin#19m12	EP19_NPC_IWIN_SB

1@jorchs,140,58,5	script	Iwin#19m13	EP19_NPC_IWIN_SB,5,5,{
	end;
	
OnTouch:
	npctalk "There is no search party beyond this area has been dispatched yet.",instance_npcname("Iwin#19m12");
	sleep 2000;
	npctalk "We're going to gather information here and enter, so focus on the search around here.",instance_npcname("Iwin#19m13");
end;
}

1@jorchs,114,119,3	script	Iwin#19m21	EP19_NPC_IWIN_SG,3,3,{
	if(get_instance_var("csn") < 4)
		npctalk "You're here! These Rgans, are impenetrable, doesn't attack, and strange. We'll find a way, so focus on your search!",instance_npcname("Iwin#19m21");
	end;
}

1@jorchs,112,119,5	script	Horuru#19m20	EP19_NPC_IWIN_SR,5,5,{
	end;
	
OnTouch:
	if(!is_party_leader()) end;
	if(get_instance_var("csn") == 4){
		set_instance_var("csn",5);
		npctalk "We can just keep going!",instance_npcname("Horuru#19m20");
		sleep 2000;
		npctalk "Wait!! There's a bunch of Rgan in front of us!",instance_npcname("Iwin#19m20");
		sleep 2000;
		npctalk "I'll just pierce them!",instance_npcname("Horuru#19m20");
		sleep 1500;
		instance_hide("Horuru#19m20",true);
		instance_hide("Iwin#19m20",true);
		instance_hide("Iwin#19m21",true);
		sleep 1500;
		instance_enable("Horuru#19m21",true);
		instance_enable("Iwin#19m22",true);
		instance_enable("Iwin#19m23",true);
		sleep 2000;
		specialeffect EF_HYOUSYOURAKU,AREA,instance_npcname("Horuru#19m21");
		specialeffect EF_HYOUSYOURAKU,AREA,instance_npcname("Iwin#19m22");
		specialeffect EF_HYOUSYOURAKU,AREA,instance_npcname("Iwin#19m23");
		specialeffect EF_HFLIMOON3,AREA,instance_npcname("Horuru#19m21");
		specialeffect EF_HFLIMOON3,AREA,instance_npcname("Iwin#19m22");
		specialeffect EF_HFLIMOON3,AREA,instance_npcname("Iwin#19m23");
		specialeffect EF_M03,AREA,instance_npcname("Horuru#19m21");
		specialeffect EF_M03,AREA,instance_npcname("Iwin#19m22");
		specialeffect EF_M03,AREA,instance_npcname("Iwin#19m23");
		npctalk "?!!!",instance_npcname("Horuru#19m21");
		sleep 1500;
		instance_disable("Horuru#19m21",true);
		instance_disable("Iwin#19m22",true);
		instance_disable("Iwin#19m23",true);
		sleep 1500;
		instance_hide("Horuru#19m20",false);
		instance_hide("Iwin#19m20",false);
		instance_hide("Iwin#19m21",false);
		npctalk "I can't pierce it! There are too many!!! How do we get through this?",instance_npcname("Horuru#19m20");
		sleep 2000;
		npctalk "Isn't it a bit strange?",instance_npcname("Iwin#19m20");
		sleep 2500;
		npctalk "We've been here for a while but they're not attacking us!",instance_npcname("Iwin#19m21");
		sleep 2000;
		instance_enable("Heart Hunter#19m20",true);
		npctalk "I just need to turn it on, what? They're already here?",instance_npcname("Heart Hunter#19m20");
		sleep 2000;
		npctalk "Are we late? Well, not like it matters.",instance_npcname("Heart Hunter#19m20");
		sleep 2000;
		npctalk "Who's that? Are they the Heart Hunters that the Illusions are making? ",instance_npcname("Horuru#19m20");
		sleep 2000;
		npctalk "This is the 5th defense line, requesting for support.",instance_npcname("Heart Hunter#19m20");
		sleep 2000;
		npctalk "There are intruders. It's difficult to continue expanding the line of defense due to obstacles. I repeat. Requesting for support.",instance_npcname("Heart Hunter#19m20");
		sleep 2000;
		npctalk "Calling for support!!",instance_npcname("Horuru#19m20");
		sleep 2000;
		instance_enable("Heart Hunter#19m21",true);
		instance_enable("Heart Hunter#19m22",true);
		npctalk "What's the obstacle?",instance_npcname("Heart Hunter#19m22");
		sleep 2000;
		npctalk "As you can see, that adventurer is already here.",instance_npcname("Heart Hunter#19m20");
		sleep 2000;
		npctalk "What is this? Are these Rgans really combined? It's just as Bagot said.",instance_npcname("Heart Hunter#19m21");
		sleep 2000;
		npctalk "I know. A well created and strong Rgans will be able to block the entrance on their own!",instance_npcname("Heart Hunter#19m20");
		sleep 2000;
		npctalk "Anyway, why are there so many people? Can't we just get rid of them?",instance_npcname("Heart Hunter#19m22");
		sleep 2000;
		npctalk "I tried to activate the device and create a blockade, but they're already here.",instance_npcname("Heart Hunter#19m20");
		sleep 2000;
		npctalk "Go and operate the device, we'll take care of them.",instance_npcname("Heart Hunter#19m20");
		npctalk "Got it.",instance_npcname("Heart Hunter#19m22");
		sleep 2000;
		instance_disable("Heart Hunter#19m22");
		npctalk "Huh?! They're splitting? Adventurer! We'll take care of the guy over there, handle the one over here!",instance_npcname("Horuru#19m20");
		sleep 2000;
		instance_hide("Horuru#19m20",true);
		instance_hide("Iwin#19m20",true);
		instance_hide("Iwin#19m21",true);
		instance_hide("Heart Hunter#19m20",true);
		instance_disable("Heart Hunter#19m21");
		instance_event("#191_csn_mob","OnSummon01",false);
		end;
	}
end;
}

1@jorchs,112,116,7	duplicate(dummynpc2)	Iwin#19m20	EP19_NPC_IWIN_SG
1@jorchs,119,123,3	duplicate(dummynpc2)	Heart Hunter#19m20	21536
1@jorchs,116,124,3	duplicate(dummynpc2)	Heart Hunter#19m21	21536
1@jorchs,123,124,3	duplicate(dummynpc2)	Heart Hunter#19m22	21536
1@jorchs,120,129,1	duplicate(dummynpc2)	Twisted Rgan#19m20	21535
1@jorchs,120,127,3	duplicate(dummynpc2)	Crushed Rgan#19m21	21535
1@jorchs,122,127,3	duplicate(dummynpc2)	Howling Rgan#19m22	21535
1@jorchs,123,127,1	duplicate(dummynpc2)	Modified Superior Rgan#19m23	21534
1@jorchs,124,126,3	duplicate(dummynpc2)	Agonizing Rgan#19m24	21535
1@jorchs,125,125,3	duplicate(dummynpc2)	Deformed Rgan#19m25	21535
1@jorchs,126,125,3	duplicate(dummynpc2)	Twisted Rgan#19m26	21535

1@jorchs,118,127,7	duplicate(dummynpc2)	Horuru#19m21	EP19_NPC_IWIN_SR
1@jorchs,122,126,7	duplicate(dummynpc2)	Iwin#19m22	EP19_NPC_IWIN_SG
1@jorchs,124,124,7	duplicate(dummynpc2)	Iwin#19m23	EP19_NPC_IWIN_SG

1@jorchs,244,176,0	script	#e19p04	HIDDEN_WARP_NPC,7,7,{
	end;
	
OnTouch:
	if(get_instance_var("csn") < 7){
		unittalk getcharid(3),strcharinfo(0) + " : This wall of Rgan, can't I attack or pierce it? Let's check again after the search.";
		end;
	}
	if(get_instance_var("csn") == 10){
		viewpoint 2,219,148,1,0xFFFF33;
		viewpoint 1,226,244,2,0xFFFF33;
		viewpoint 1,191,203,3,0xFFFF33;
	}
end;
}

1@jorchs,251,171,1	script	Voglinde#19m30	4_EP19_VOGLINDE,5,5,{
	end;
	
OnTouch:
	if(get_instance_var("csn") == 7){
		set_instance_var("csn",8);
		npctalk "Adventurer! You're safe.",instance_npcname("Miriam#19m30");
		sleep 2000;
		npctalk "The Rgans in front of us are in the control of the Heart Hunters, right?",instance_npcname("Voglinde#19m30");
		sleep 2500;
		npctalk "How can they use an ally as a barrier?",instance_npcname("Miriam#19m30");
		sleep 2000;
		npctalk "If they had that kind of conscience, they wouldn't have created the Heart Hunter in the first place.",instance_npcname("Voglinde#19m30");
		sleep 2000;
		instance_enable("Heart Hunter#19m30",true);
		instance_enable("Heart Hunter#19m31",true);
		instance_enable("Heart Hunter#19m32",true);
		npctalk "Are you talking about us? Don't worry. We're doing this because we want to.",instance_npcname("Heart Hunter#19m30");
		sleep 2500;
		npctalk "You can say the same for the Superior Rgans.",instance_npcname("Heart Hunter#19m31");
		sleep 2000;
		npctalk "Everyone lined up because it was their chance to become useful.",instance_npcname("Heart Hunter#19m32");
		sleep 2500;
		npctalk "The Intermediate Rgans, what happened to them?",instance_npcname("Voglinde#19m30");
		sleep 2000;
		npctalk "Well... there's no helping those intermediates one.",instance_npcname("Heart Hunter#19m30");
		sleep 2000;
		npctalk "Is it wrong to be in a dangerous place at the wrong time...?",instance_npcname("Heart Hunter#19m32");
		sleep 2000;
		npctalk "Where is everyone? What are you doing together?",instance_npcname("Miriam#19m30");
		sleep 2000;
		npctalk "There's no reason for me to tell yo that. I'm not a stupid person.",instance_npcname("Heart Hunter#19m32");
		sleep 2000;
		npctalk "Then, get out of our way and stop talking nonsense.",instance_npcname("Miriam#19m30");
		sleep 2500;
		npctalk "I guess people from Arunafeltz are all direct.",instance_npcname("Heart Hunter#19m31");
		npctalk "We can't let them pass. We are under orders too.",instance_npcname("Heart Hunter#19m30");
		sleep 2500;
		npctalk "To stop you people. I prepared something... Uh...",instance_npcname("Heart Hunter#19m32");
		sleep 2500;
		npctalk "I see, you're running out of time, aren't you?",instance_npcname("Voglinde#19m30");
		sleep 2500;
		npctalk "How did you know? It should be a little longer, but I don't know what happened! Come on!!!",instance_npcname("Heart Hunter#19m30");
		sleep 2000;
		specialeffect EF_SCREEN_QUAKE ,AREA,instance_npcname("Heart Hunter#19m32");
		npctalk "They told me to use it at a time like this. But, I don't think I saw them using it before?",instance_npcname("Heart Hunter#19m30");
		instance_announce false,"The injection device has been activated.",bc_map,0x00FFCC;
		sleep 2500;
		npctalk "They say it hatches a superior egg into a Modified Superior Rgan in no time.",instance_npcname("Heart Hunter#19m32");
		specialeffect EF_CLOUD4 ,AREA,instance_npcname("Heart Hunter#19m32");
		sleep 2000;
		npctalk "Is it going to give birth to a lot of Rgan?",instance_npcname("Heart Hunter#19m31");
		sleep 2500;
		npctalk "As long as the supply of mana continues, it will continue to create Rgans.",instance_npcname("Heart Hunter#19m32");
		sleep 2500;
		npctalk "To me, that sounds like we can cut off the mana supply, am I right? You are nicer than I thought.",instance_npcname("Voglinde#19m30");
		emotion ET_HUK,getnpcid(0,instance_npcname("Heart Hunter#19m30"));
		emotion ET_HUK,getnpcid(0,instance_npcname("Heart Hunter#19m31"));
		emotion ET_HUK,getnpcid(0,instance_npcname("Heart Hunter#19m32"));
		sleep 2500;
		npctalk "Well, do you think we'll let you? Besides, I also called for support!!",instance_npcname("Heart Hunter#19m31");
		sleep 2000;
		npctalk "Let's head to the next goal!",instance_npcname("Heart Hunter#19m30");
		sleep 2000;
		instance_disable("Heart Hunter#19m30");
		instance_disable("Heart Hunter#19m31");
		instance_disable("Heart Hunter#19m32");
		npctalk "Adventurer, we'll chase them down, kill the Rgans and stop the device...!",instance_npcname("Voglinde#19m30");
		sleep 2000;
		instance_disable("Voglinde#19m30");
		instance_disable("Miriam#19m30");
		instance_enable("#e19p01",true);
		instance_event("#191_csn_mob","OnSummon03",false);
		end;
	}
end;
}

1@jorchs,249,170,1	duplicate(dummynpc2)	Miriam#19m30	4_EP18_MIRIAM

1@jorchs,249,177,1	duplicate(dummynpc2)	Modified Superior Rgan#19m30	21534
1@jorchs,248,176,3	duplicate(dummynpc2)	Twisted Rgan#19m31	21535
1@jorchs,247,176,3	duplicate(dummynpc2)	Combined Rgan#19m32	21535
1@jorchs,246,176,1	duplicate(dummynpc2)	Modified Superior Rgan#19m33	21534
1@jorchs,245,176,3	duplicate(dummynpc2)	Modified Superior Rgan#19m34	21534
1@jorchs,244,176,3	duplicate(dummynpc2)	Crushed Rgan#19m35	21535
1@jorchs,242,174,3	duplicate(dummynpc2)	Twisted Rgan#19m37	21535
1@jorchs,230,160,0	duplicate(dummynpc2)	#e19c01	HIDDEN_WARP_NPC

1@jorchs,248,173,5	duplicate(dummynpc2)	Heart Hunter#19m30	21536
1@jorchs,246,173,5	duplicate(dummynpc2)	Heart Hunter#19m31	21536
1@jorchs,243,173,5	duplicate(dummynpc2)	Heart Hunter#19m32	21536

1@jorchs,243,166,0	script	#e19p01	HIDDEN_WARP_NPC,7,7,{
	end;
	
OnTouch:
	if(get_instance_var("csn") < 9)
		viewpoint 1,219,148,1,0xFFFF33;
end;
}

1@jorchs,219,148,5	script	Injection Device#19m30	4_ENERGY_BLUE,3,3,{
	if(get_instance_var("csn") == 8){
		instance_disable(strnpcinfo(0));
		viewpoint 2,219,148,1,0xFFFF33;
		set_instance_var("csn",9);
		npctalk "Injection device disabled";
		specialeffect EF_AGIUP,AREA,instance_npcname("#e19c01");
		specialeffect EF_GUARD2,AREA,instance_npcname("#e19c01");
		instance_event("#191_csn_mob","OnSummon04",false);
	}
	end;
}

1@jorchs,226,244,7	script	Injection Device#19m31	4_ENERGY_BLUE,{
	if(!get_instance_var("dev_1")) end;
	set_instance_var("dev_1",0);
	npctalk "Injection device disabled";
	instance_disable(strnpcinfo(0));
	if(!get_instance_var("dev_2")){
		if(!get_instance_var("clear"))
			instance_announce false,"There's still Rgans over there!",bc_map,0x00FFCC;
		else
			instance_event("#191_csn_control","OnEvent02",false);
	}
	end;
	
OnSummon:
	while(get_instance_var("dev_1")){
		if(!get_instance_var("dev_1")) end;
		specialeffect EF_AGIUP,AREA,instance_npcname("#e19c02");
		specialeffect EF_GUARD2,AREA,instance_npcname("#e19c02");
		monster get_instance_var("map$"),239,255,"Unstable Modified Rgan",21534,1;
		sleep 30000;
	}
end;
}

1@jorchs,239,255,0	duplicate(dummynpc2)	#e19c02	HIDDEN_WARP_NPC

1@jorchs,191,203,7	script	Injection Device#19m32	4_ENERGY_BLUE,{
	if(!get_instance_var("dev_2")) end;
	set_instance_var("dev_2",0);
	npctalk "Injection device disabled";
	instance_disable(strnpcinfo(0));
	if(!get_instance_var("dev_1")){
		if(!get_instance_var("clear"))
			instance_announce false,"There's still Rgans over there!",bc_map,0x00FFCC;
		else
			instance_event("#191_csn_control","OnEvent02",false);
	}
	end;
	
OnSummon:
	while(get_instance_var("dev_2")){
		if(!get_instance_var("dev_2")) end;
		specialeffect EF_AGIUP,AREA,instance_npcname("#e19c03");
		specialeffect EF_GUARD2,AREA,instance_npcname("#e19c03");
		monster get_instance_var("map$"),198,205,"Unstable Modified Rgan",21534,1;
		sleep 30000;
	}
end;
}

1@jorchs,198,205,0	duplicate(dummynpc2)	#e19c03	HIDDEN_WARP_NPC

1@jorchs,192,206,5	script	Injection Device#19m40	4_ENERGY_BLUE,3,3,{
	if(!get_instance_var("dev_3")) end;
	set_instance_var("dev_3",0);
	npctalk "Device Released";
	instance_disable(strnpcinfo(0));
	instance_event("#191_csn_mob","OnSummon05",false);
	end;
	
OnSummon:
	while(get_instance_var("dev_3")){
		if(!get_instance_var("dev_3")) end;
		specialeffect EF_AGIUP,AREA,instance_npcname("#e19c04");
		specialeffect EF_GUARD2,AREA,instance_npcname("#e19c04");
		monster get_instance_var("map$"),199,197,"Unstable Modified Rgan",21534,1;
		sleep 30000;
	}
end;
}

1@jorchs,199,197,0	duplicate(dummynpc2)	#e19c04	HIDDEN_WARP_NPC


1@jorchs,20,195,7	script	Research Device#19m40	4_ENERGY_BLUE,{
	if(!get_instance_var("dev_4")) end;
	set_instance_var("dev_4",0);
	instance_disable(strnpcinfo(0));
	instance_event("#191_csn_mob","OnSummon08",false);
	unittalk getcharid(3),strcharinfo(0) + " : Heart of Ymir... Of course not... I don't see anything useful.";
	instance_announce false,"There are still a lot of Rgans to deal with!",bc_map,0x00FFCC;
	end;
	
OnSummon:
	while(get_instance_var("dev_4")){
		if(!get_instance_var("dev_4")) end;
		specialeffect EF_AGIUP,AREA,instance_npcname("#e19c05");
		specialeffect EF_GUARD2,AREA,instance_npcname("#e19c05");
		monster get_instance_var("map$"),17,188,"Unstable Modified Rgan",21534,1;
		sleep 30000;
	}
end;
}

1@jorchs,17,188,0	duplicate(dummynpc2)	#e19c05	HIDDEN_WARP_NPC

1@jorchs,24,142,7	script	Research Device#19m41	4_ENERGY_BLUE,{
	if(!get_instance_var("dev_5")) end;
	set_instance_var("dev_5",0);
	instance_disable(strnpcinfo(0));
	instance_event("#191_csn_mob","OnSummon07",false);
	unittalk getcharid(3),strcharinfo(0) + " : This... is it important? I don't know. Did he take all the useful materials?";
	end;
	
OnSummon:
	while(get_instance_var("dev_5")){
		if(!get_instance_var("dev_5")) end;
		specialeffect EF_AGIUP,AREA,instance_npcname("#e19c06");
		specialeffect EF_GUARD2,AREA,instance_npcname("#e19c06");
		monster get_instance_var("map$"),17,133,"Unstable Modified Rgan",21534,1;
		sleep 30000;
	}
end;
}

1@jorchs,17,133,0	duplicate(dummynpc2)	#e19c06	HIDDEN_WARP_NPC


1@jorchs,16,135,3	script	Laboratory Traces#19m40	4_ENERGY_BLUE,{
	unittalk getcharid(3),strcharinfo(0) + " : There is a little left.";
	end;
}


1@jorchs,156,177,0	script	#e19p02	HIDDEN_WARP_NPC,7,7,{
	end;
	
OnTouch:
	if(get_instance_var("csn") == 12 || get_instance_var("csn") == 13){
		viewpoint 1,192,206,1,0xFFFF33;
		viewpoint 2,226,244,2,0xFFFF33;
		viewpoint 2,191,203,3,0xFFFF33;
	}
end;
}

1@jorchs,137,181,0	script	#e19p03	HIDDEN_WARP_NPC,7,7,{
	end;
	
OnTouch:
	if(get_instance_var("csn") < 18){
		viewpoint 2,192,206,1,0xFFFF33;
		viewpoint 1,20,195,2,0xFFFF33;
		viewpoint 1,24,142,3,0xFFFF33;
	}
end;
}

1@jorchs,150,177,1	script	Leon#19m40	4_EP19_LEON,3,3,{
	if(get_instance_var("csn") == 11){
		set_instance_var("csn",12);
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Heart Hunter#19m40");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Heart Hunter#19m41");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Heart Hunter#19m42");
		npctalk "What's wrong with you people? Are you the one causing trouble because you can't get in?",instance_npcname("Aurelie#19m40");
		sleep 3000;
		npctalk "I'm not trying to say anything, I just don't understand.",instance_npcname("Aurelie#19m40");
		sleep 3000;
		npctalk "Adventurer, you're here? We're...",instance_npcname("Leon#19m40");
		instance_announce false,"The injection device has activated.",bc_map,0x00FFCC;
		sleep 3000;
		npctalk "We'll keep them at bay, so you'd better find the device first and stop it.",instance_npcname("Aurelie#19m40");
		sleep 2000;
		npctalk "They tampered it and maybe restarted it?",instance_npcname("Leon#19m40");
		sleep 2000;
		npctalk "Let's talk without them, everyone.",instance_npcname("Heart Hunter#19m40");
		sleep 2000;
		npctalk "What is this, Why can't I move?",instance_npcname("Heart Hunter#19m41");
		sleep 2000;
		npctalk "It's because I'm great.",instance_npcname("Aurelie#19m40");
		sleep 2500;
		npctalk "Now, shall we finish our conversation?",instance_npcname("Leon#19m40");
		sleep 3000;
		npctalk "Leave this place to us.",instance_npcname("Aurelie#19m40");
		npctalk "Don't just stand there and stop the adventurer!",instance_npcname("Heart Hunter#19m42");
		instance_enable("#e19p02",true);
		set_instance_var("dev_3",1);
		instance_enable("Injection Device#19m40",true);
		instance_event("Injection Device#19m40","OnSummon",false);
		set_instance_var("csn",13);
		end;
	}
	if(get_instance_var("csn") == 13){
		npctalk "Leave this place to us and stop the device.",instance_npcname("Aurelie#19m40");
		end;
	}
	if(get_instance_var("csn") == 14){
		set_instance_var("csn",15);
		npctalk "Here. They're gathered inside, we need to go further.",instance_npcname("Aurelie#19m40");
		sleep 2000;
		npctalk "They called all the higher-grade Rgans into Lasgand's residence.",instance_npcname("Aurelie#19m40");
		sleep 2000;
		npctalk "Ple... please don't tell them that we told you. Please.",instance_npcname("Heart Hunter#19m40");
		sleep 2000;
		npctalk "I heard there's only one entrance. Is that really true?",instance_npcname("Leon#19m40");
		npctalk "We only told the truth. Please believe me.",instance_npcname("Heart Hunter#19m41");
		sleep 3000;
		npctalk "Anyway, I'm sure what Lasgand and Bagot want to achieve is just around the corner.",instance_npcname("Aurelie#19m40");
		sleep 2000;
		npctalk "We need to hurry.",instance_npcname("Aurelie#19m40");
		sleep 1500;
		npctalk "Everyone, me and them, please leave us out. We told you everything we know.",instance_npcname("Heart Hunter#19m42");
		sleep 1500;
		npctalk "Apart from that...",instance_npcname("Leon#19m40");
		emotion ET_SURPRISE,getnpcid(0,instance_npcname("Leon#19m40"));
		specialeffect EF_SCREEN_QUAKE,AREA,instance_npcname("Leon#19m40");
		instance_announce false,"The mana amplifier has powered up.",bc_map,0x00FFCC;
		sleep 3000;
		npctalk "Oh? The flow of mana is a bit strange?",instance_npcname("Leon#19m40");
		sleep 2000;
		npctalk "Yes. I don't think the others will be able to handle this.",instance_npcname("Aurelie#19m40");
		sleep 2500;
		npctalk "Adventurer, can we leave this to you?",instance_npcname("Leon#19m40");
		sleep 3000;
		npctalk "It's unusual, we better hurry up and take care of it. I'll head in first.",instance_npcname("Leon#19m40");
		sleep 2000;
		instance_disable("Leon#19m40");
		npctalk "Ha! By the way! Aren't you underestimating us?",instance_npcname("Heart Hunter#19m40");
		sleep 2000;
		npctalk "If the scary brother disappear, We can do it too!",instance_npcname("Heart Hunter#19m41");
		sleep 2000;
		npctalk "You better shut that mouth. Then, Please.",instance_npcname("Aurelie#19m40");
		instance_disable("Aurelie#19m40");
		sleep 500;
		instance_disable("Heart Hunter#19m40");
		instance_disable("Heart Hunter#19m41");
		instance_disable("Heart Hunter#19m42");
		instance_event("#191_csn_mob","OnSummon06",false);
		end;
	}
	end;
}

1@jorchs,151,179,1	duplicate(Leon#19m40)	Aurelie#19m40	4_EP19_AURELIE,5,5

1@jorchs,149,179,5	duplicate(dummynpc2)	Heart Hunter#19m40	21536
1@jorchs,149,178,5	duplicate(dummynpc2)	Heart Hunter#19m41	21536
1@jorchs,150,180,5	duplicate(dummynpc2)	Heart Hunter#19m42	21536


1@jorchs,140,183,1	duplicate(dummynpc2)	Modified Superior Rgan#19m40	21534
1@jorchs,140,182,3	duplicate(dummynpc2)	Crushed Rgan#19m41	21535

1@jorchs,139,181,3	script	Modified Superior Rgan#19m42	21534,5,5,{
	end;
	
OnTouch:
	if(get_instance_var("csn") < 11)
		unittalk getcharid(3),strcharinfo(0) + " : I can't go through the wall of Rgan. I have to finish the search first.";
end;
}

1@jorchs,139,180,1	duplicate(dummynpc2)	Modified Superior Rgan#19m43	21534
1@jorchs,138,179,3	duplicate(dummynpc2)	Twisted Rgan#19m44	21535
1@jorchs,138,178,3	duplicate(dummynpc2)	Modified Superior Rgan#19m45	21534

1@jorchs,39,251,5	script	Heart Hunter#19m50	21536,5,5,{
	end;
	
OnTouch:
	if(get_instance_var("csn") == 18){
		set_instance_var("csn",19);
		npctalk "Who's that?",instance_npcname("Heart Hunter#19m50");
		sleep 2000;
		npctalk "Adventurer?",instance_npcname("Heart Hunter#19m52");
		sleep 2000;
		npctalk "The adventurer is the first one to arrive?",instance_npcname("Heart Hunter#19m51");
		sleep 1500;
		npctalk "We were wondering who would get here first.",instance_npcname("Heart Hunter#19m52");
		npctalk "I won.",instance_npcname("Heart Hunter#19m50");
		sleep 2500;
		instance_enable("Bagot#19m50",true);
		npctalk "Only the adventurer came?",instance_npcname("Bagot#19m50");
		sleep 2000;
		npctalk "Yes. The others seem to be dealing with the Rgans.",instance_npcname("Heart Hunter#19m50");
		sleep 2000;
		npctalk "Really? You must really wanted to meet me, adventurer?",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "Did you think you could fool me with such a cute trick the other day?!",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "I have a present that I prepared. I mean this.",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "I guess the other people is coming. The road crossed.",instance_npcname("Bagot#19m50");
		instance_enable("Furious Rgan#19m50",true);
		instance_enable("Furious Rgan#19m51",true);
		instance_enable("Furious Rgan#19m52",true);
		instance_enable("Furious Rgan#19m53",true);
		instance_enable("Furious Rgan#19m55",true);
		instance_enable("Furious Rgan#19m56",true);
		npctalk "Ugh---",instance_npcname("Furious Rgan#19m50");
		npctalk "Grr--- rrrr---",instance_npcname("Furious Rgan#19m51");
		npctalk "Ah... can't... stop..",instance_npcname("Furious Rgan#19m52");
		npctalk ".........",instance_npcname("Furious Rgan#19m53");
		npctalk "Ugh... Oh.. .Uh...",instance_npcname("Furious Rgan#19m54");
		npctalk "Uh Uh Uh Uh--- Uh---",instance_npcname("Furious Rgan#19m55");
		npctalk "Where... this...",instance_npcname("Furious Rgan#19m56");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m50");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m51");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m52");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m53");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m55");
		specialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m56");
		specialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m50");
		specialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m51");
		specialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m52");
		specialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m53");
		specialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m55");
		specialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m56");
		sleep 3000;
		npctalk "But it doesn't matter. I'm happy that I was able to give it to you in person.",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "You must be wondering what I will do, right? You must have been so curious that you act impulsively.",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "But I won't fall for such shallow trick.",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "I will tell you directly now. I made them follow me, that's why I left unnecessary trails.",instance_npcname("Bagot#19m50");
		sleep 2000;
		npctalk "It's a little annoying to think that flies will stick in to the new world that I'll create.",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "And Isn't Lasgand also very angry at the humans who deceived him?",instance_npcname("Bagot#19m50");
		sleep 3000;
		npctalk "Well, since it's a gift from me, I'll just take my leave here.",instance_npcname("Bagot#19m50");
		sleep 500;
		instance_disable("Bagot#19m50");
		instance_event("#191_csn_mob","OnSummon09",false);
		end;
	}
end;
}

1@jorchs,36,251,5	duplicate(dummynpc2)	Heart Hunter#19m51	21536
1@jorchs,41,253,5	duplicate(dummynpc2)	Heart Hunter#19m52	21536

1@jorchs,38,255,3	script	Modified Superior Rgan#19m54	21534,5,5,{
	end;
	
OnTouch:
	if(get_instance_var("csn") < 18)
		unittalk getcharid(3),strcharinfo(0) + " : There's a wall of Rgan. I'll have to come back here after searching.";
end;
}

1@jorchs,38,256,5	duplicate(Modified Superior Rgan#19m54)	Modified Superior Rgan#19m64	21534,5,5

1@jorchs,34,255,1	duplicate(dummynpc2)	Modified Superior Rgan#19m50	21534
1@jorchs,35,255,3	duplicate(dummynpc2)	Modified Superior Rgan#19m51	21534
1@jorchs,36,255,3	duplicate(dummynpc2)	Modified Superior Rgan#19m52	21534
1@jorchs,37,255,1	duplicate(dummynpc2)	Modified Superior Rgan#19m53	21534
1@jorchs,39,255,3	duplicate(dummynpc2)	Modified Superior Rgan#19m55	21534
1@jorchs,40,255,1	duplicate(dummynpc2)	Modified Superior Rgan#19m56	21534
1@jorchs,41,255,3	duplicate(dummynpc2)	Modified Superior Rgan#19m57	21534
1@jorchs,42,255,3	duplicate(dummynpc2)	Modified Superior Rgan#19m58	21534
1@jorchs,43,255,3	duplicate(dummynpc2)	Modified Superior Rgan#19m59	21534
1@jorchs,35,256,5	duplicate(dummynpc2)	Modified Superior Rgan#19m61	21534
1@jorchs,37,256,1	duplicate(dummynpc2)	Modified Superior Rgan#19m63	21534
1@jorchs,39,256,3	duplicate(dummynpc2)	Modified Superior Rgan#19m65	21534
1@jorchs,41,256,1	duplicate(dummynpc2)	Modified Superior Rgan#19m67	21534
1@jorchs,42,256,5	duplicate(dummynpc2)	Modified Superior Rgan#19m68	21534

1@jorchs,31,249,5	duplicate(dummynpc2)	Furious Rgan#19m50	21590
1@jorchs,47,237,1	duplicate(dummynpc2)	Furious Rgan#19m51	21590
1@jorchs,47,249,3	duplicate(dummynpc2)	Furious Rgan#19m52	21590
1@jorchs,31,237,7	duplicate(dummynpc2)	Furious Rgan#19m53	21590
1@jorchs,27,243,5	duplicate(dummynpc2)	Furious Rgan#19m54	21590
1@jorchs,50,243,3	duplicate(dummynpc2)	Furious Rgan#19m55	21590
1@jorchs,39,235,1	duplicate(dummynpc2)	Furious Rgan#19m56	21590

1@jorchs,37,252,5	duplicate(dummynpc2)	Bagot#19m50	4_EP18_BAGOT
1@jorchs,39,271,1	duplicate(dummynpc2)	Leon#19m60	4_EP19_LEON
1@jorchs,40,274,1	duplicate(dummynpc2)	Horuru#19m60	EP19_NPC_IWIN_SR
1@jorchs,38,272,1	duplicate(dummynpc2)	Aurelie#19m60	4_EP19_AURELIE
1@jorchs,36,272,7	duplicate(dummynpc2)	Voglinde#19m60	4_EP19_VOGLINDE
1@jorchs,35,271,7	duplicate(dummynpc2)	Miriam#19m60	4_EP18_MIRIAM

1@jorchs,36,257,0	script	#19m60	WARPNPC,2,2,{
	end;
	
OnTouch:
	warp "jor_back3",67,316;
end;
}

1@jorchs,37,274,5	script	Central Door#19m61	4_ENERGY_BLUE,3,3,{
	if(get_instance_var("csn") == 20){
		if(!is_party_leader()) end;
		set_instance_var("csn",21);
		instance_enable("Aurelie#19m60",true);
		instance_enable("Leon#19m60",true);
		setpcblock PCBLOCK_NPC,true;
		npctalk "Adventurer, you're safe.",instance_npcname("Aurelie#19m60");
		sleep2 1500;
		npctalk "I got stranded because a superior Rgan with lots of mana came at me suddenly.",instance_npcname("Leon#19m60");
		sleep2 1500;
		instance_enable("Horuru#19m60",true);
		npctalk "Oh~ This. I'm so unlucky! What is this thing here?",instance_npcname("Horuru#19m60");
		sleep2 1500;
		instance_enable("Voglinde#19m60",true);
		instance_enable("Miriam#19m60",true);
		npctalk "Looks like everyone is here.",instance_npcname("Voglinde#19m60");
		sleep2 1500;
		npctalk "Did anyone find anything?",instance_npcname("Miriam#19m60");
		sleep2 2000;
		npctalk "I searched this complicated place thoroughly, but I couldn't a trace of anything.",instance_npcname("Horuru#19m60");
		sleep2 2000;
		npctalk "All of the Illusion's research that was organized is gone, and this is the only place left.",instance_npcname("Aurelie#19m60");
		sleep2 2000;
		npctalk "The Heart Hunter that I grabbed told me, that everyone moved to the center.",instance_npcname("Voglinde#19m60");
		sleep2 1500;
		npctalk "No one has escaped using the airship. They won't be able to do it.",instance_npcname("Miriam#19m60");
		sleep2 2000;
		npctalk "Isn't this place the center?",instance_npcname("Leon#19m60");
		sleep2 2000;
		npctalk "So if we open this door, will we be able to catch both the Illusion and Lasgand?",instance_npcname("Horuru#19m60");
		sleep2 2000;
		npctalk "They abandoned the place where all upper-grade Rgans lived and moved to a place where no was allowed to enter?",instance_npcname("Aurelie#19m60");
		sleep2 1500;
		npctalk "Why?",instance_npcname("Leon#19m60");
		sleep2 1500;
		npctalk "Let's open it up. And ask them directly.",instance_npcname("Horuru#19m60");
		sleep2 1500;
		specialeffect EF_SCREEN_QUAKE,AREA,instance_npcname("Central Door#19m61");
		npctalk "Aren't this locked too tight?",instance_npcname("Leon#19m60");
		sleep2 2500;
		npctalk "It's not just this place, is it? They brought everything they have.",instance_npcname("Horuru#19m60");
		sleep2 2500;
		specialeffect EF_M03,AREA,instance_npcname("Central Door#19m61");
		sleep2 500;
		specialeffect EF_NPC_EARTHQUAKE,AREA,instance_npcname("Central Door#19m61");
		sleep2 500;
		specialeffect EF_HYOUSYOURAKU,AREA,instance_npcname("Central Door#19m61");
		specialeffect EF_DESPERADO,AREA,instance_npcname("Central Door#19m61");
		specialeffect EF_SPREADATTACK ,AREA,instance_npcname("Central Door#19m61");
		specialeffect EF_DRAGONFEAR,AREA,instance_npcname("Central Door#19m61");
		sleep2 1000;
		npctalk "What did they do to this door? Open! Open!!! I know they're inside!!",instance_npcname("Horuru#19m60");
		sleep2 2000;
		npctalk "I can't even open it even with the key that I got from the Heart Hunter.",instance_npcname("Voglinde#19m60");
		sleep2 1500;
		npctalk "It's the same when you inject a mana.",instance_npcname("Aurelie#19m60");
		sleep2 1500;
		npctalk "Isn't this an impregnable fortress?",instance_npcname("Horuru#19m60");
		sleep2 2000;
		npctalk "Then, I'll set up a surveillance and leave for today.",instance_npcname("Voglinde#19m60");
		sleep2 2000;
		npctalk "What about the Rgan's nest? Shall we burn it to the ground?",instance_npcname("Horuru#19m60");
		sleep2 2000;
		npctalk "Well, we'd better just leave it.",instance_npcname("Aurelie#19m60");
		sleep2 2500;
		npctalk "To make them believe that gave up and left?",instance_npcname("Leon#19m60");
		sleep2 2500;
		npctalk "Yes. Then they might open the door at some point and catch them off guard.",instance_npcname("Aurelie#19m60");
		sleep2 2000;
		npctalk "I don't like the idea that much, but can't help it, we'll just have to strengthen the surveillance here.",instance_npcname("Horuru#19m60");
		sleep2 2000;
		npctalk "Let's go. I'll open up a warp portal down there, so we can return comfortably.",instance_npcname("Aurelie#19m60");
		instance_enable("#19m60",true);
		specialeffect EF_ELECTRIC4,AREA,instance_npcname("#19m60");
		specialeffect EF_YELLOWFLY3,AREA,instance_npcname("#19m60");
		if(isbegin_quest(17648) == 1){
			completequest 17648;
			setquest 17649;
		}
		setpcblock PCBLOCK_NPC,false;
		end;
	}
	end;
}


1@jorchs,1,1,0	script	#191_csn_mob	-1,{
	end;
	
OnSummon:
	set_instance_var("clear",0);
	setarray .@name$,"Twisted","Crushed","Deformed","Split";
	switch(get_instance_var("summon")){
		case 0:
			setarray .@mob,
			221,37,21535,
			237,61,21535,
			242,66,21535,
			204,38,21535,
			181,48,21535,
			135,29,21535,
			124,26,21535;
			break;
			
		case 1:
			setarray .@mob,
			39,76,21535,
			26,90,21535,
			31,59,21535,
			51,45,21535,
			96,64,21535,
			75,23,21535,
			96,64,21535,
			119,80,21535,
			118,103,21535;
			break;
			
		case 2:
			monster get_instance_var("map$"),142,144,"Howling Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),159,151,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),180,139,"Agonizing Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),200,131,"Deformed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),216,135,"Crushed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),226,141,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),236,151,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			end;
			
		case 3:
			monster get_instance_var("map$"),228,211,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),245,235,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),210,204,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),194,200,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),173,179,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),247,278,"Combined Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),224,273,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),198,272,"Crushed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),270,198,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),273,254,"Howling Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			end;
			
		case 4:
			monster get_instance_var("map$"),25,172,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),24,188,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),82,221,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),86,165,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),79,154,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),77,146,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),32,135,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),18,133,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),103,215,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),45,153,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			monster get_instance_var("map$"),121,190,"Crushed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
			end;
	}
	for(.@i = 0; .@i < getarraysize(.@mob); .@i += 3)
		monster get_instance_var("map$"),.@mob[.@i],.@mob[.@i+1],.@name$[rand(4)] + " Rgan",.@mob[.@i+2],1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
end;

OnMobKill:
	if(mobcount(get_instance_var("map$"),instance_npcname(strnpcinfo(0)) + "::OnMobKill")) end;
	.@id = get_instance_var("summon");
	set_instance_var("summon",.@id + 1);
	set_instance_var("clear",1);
	switch(.@id){
		case 0:
			if(!get_instance_var("search"))
				instance_announce false,"Somebody.. help me...",bc_map,0x00FFCC;
			else
				instance_event("#191_csn_control","OnEvent01",false);
			end;
			
		case 1:
			if(get_instance_var("search")){
				set_instance_var("csn",4);
				instance_announce false,"Wah!!! What is this!!!",bc_map,0x00FFCC;
				instance_enable("Horuru#19m20",true);
				instance_enable("Iwin#19m20",true);
			}
			break;
			
		case 2:
			set_instance_var("csn",7);
			instance_enable("Voglinde#19m30",true);
			instance_enable("Miriam#19m30",true);
			break;
			
		case 3:
			if(get_instance_var("dev_1") == 0 && get_instance_var("dev_2") == 0)
				instance_event("#191_csn_control","OnEvent02",false);
			else
				instance_announce false,"There's still Rgans over there!",bc_map,0x00FFCC;
			break;
			
		case 4:
			set_instance_var("csn",get_instance_var("csn") + 1);
			if(get_instance_var("csn") == 18)
				instance_event("#191_csn_control","OnEvent03",false);
			break;
	}
end;

OnSummon01:
	monster get_instance_var("map$"),119,123,"Modified Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill01";
	monster get_instance_var("map$"),123,124,"Modified Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill01";
	monster get_instance_var("map$"),120,127,"Crushed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill01";
	monster get_instance_var("map$"),124,125,"Agonizing Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill01";
end;

OnMobKill01:
	if(mobcount(get_instance_var("map$"),instance_npcname(strnpcinfo(0)) + "::OnMobKill01")) end;
	instance_hide("Heart Hunter#19m20",false);
	instance_announce false,"?- Do you think it'll end like this? It's not like we're not prepared!",bc_map,0x00FFCC;
	sleep 1500;
	instance_event(strnpcinfo(0),"OnSummon02",false);
end;

OnSummon02:
	set_instance_var("count",5);
	instance_disable("Heart Hunter#19m20");
	instance_hide("Twisted Rgan#19m20",true);
	instance_hide("Howling Rgan#19m22",true);
	instance_hide("Agonizing Rgan#19m24",true);
	instance_hide("Deformed Rgan#19m25",true);
	monster get_instance_var("map$"),119,128,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill02";
	monster get_instance_var("map$"),125,124,"Deformed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill02";
	sleep 3000;
	instance_hide("Crushed Rgan#19m21",true);
	instance_hide("Modified Superior Rgan#19m23",true);
	instance_hide("Twisted Rgan#19m26",true);
	monster get_instance_var("map$"),121,126,"Howling Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill02";
	monster get_instance_var("map$"),124,124,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill02";
	monster get_instance_var("map$"),123,126,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill02";
end;

OnMobKill02:
	set_instance_var("count",get_instance_var("count") - 1);
	if(get_instance_var("count")) end;
	set_instance_var("csn",6);
	instance_hide("Horuru#19m20",false);
	instance_hide("Iwin#19m20",false);
	instance_hide("Iwin#19m21",false);
	npctalk "Ho- You've taken care of everything. Anyway, there were lots of unknown devices everywhere.",instance_npcname("Horuru#19m20");
	sleep 2500;
	npctalk "I caught a Heart Hunter trying to operate it. What was he trying to do?",instance_npcname("Horuru#19m20");
	sleep 2500;
	npctalk "How did the Heart Hunter get through that pile of Rgans?",instance_npcname("Iwin#19m20");
	sleep 2500;
	npctalk "Perhaps the Heart Hunter was doing something to avoid or manipulate the Rgans.",instance_npcname("Horuru#19m20");
	sleep 2500;
	npctalk "That's why the walls of Rgan's were dismantled when we disposed of the Heart Hunters!",instance_npcname("Iwin#19m21");
	sleep 2500;
	npctalk "It won't be easy. Adventurer, we'll share this information with the others. Let's go.",instance_npcname("Horuru#19m20");
	sleep 500;
	instance_disable("Horuru#19m20");
	instance_disable("Iwin#19m20");
	instance_disable("Iwin#19m21");
	setcell get_instance_var("map$"),120,129,120,129,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),121,128,121,128,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),122,127,122,127,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),123,127,123,127,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),124,126,124,126,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),125,125,125,125,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),126,125,126,125,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),120,129,120,129,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),121,128,121,128,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),122,127,122,127,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),123,127,123,127,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),124,126,124,126,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),125,125,125,125,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),126,125,126,125,CELL_SHOOTABLE,true;
	specialeffect EF_WIND,AREA,instance_npcname("Twisted Rgan#19m20");
	specialeffect EF_WIND,AREA,instance_npcname("Crushed Rgan#19m21");
	specialeffect EF_WIND,AREA,instance_npcname("Howling Rgan#19m22");
	specialeffect EF_WIND,AREA,instance_npcname("Modified Superior Rgan#19m23");
	specialeffect EF_WIND,AREA,instance_npcname("Agonizing Rgan#19m24");
	specialeffect EF_WIND,AREA,instance_npcname("Deformed Rgan#19m25");
	instance_event("#191_csn_mob","OnSummon",false);
	sleep 10000;
	instance_enable("Twisted Rgan#19m20",false);
	instance_enable("Howling Rgan#19m22",false);
	instance_enable("Agonizing Rgan#19m24",false);
	instance_enable("Deformed Rgan#19m25",false);
	instance_enable("Crushed Rgan#19m21",false);
	instance_enable("Modified Superior Rgan#19m23",false);
	instance_enable("Twisted Rgan#19m26",false);
end;

OnSummon03:
	set_instance_var("count",7);
	instance_disable("Modified Superior Rgan#19m30");
	instance_disable("Twisted Rgan#19m37");
	monster get_instance_var("map$"),249,176,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	monster get_instance_var("map$"),242,173,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	sleep 3000;
	instance_disable("Twisted Rgan#19m31");
	instance_disable("Crushed Rgan#19m35");
	monster get_instance_var("map$"),248,175,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	monster get_instance_var("map$"),244,175,"Crushed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	sleep 3000;
	instance_disable("Combined Rgan#19m32");
	instance_disable("Modified Superior Rgan#19m33");
	instance_disable("Modified Superior Rgan#19m34");
	monster get_instance_var("map$"),247,175,"Combined Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	monster get_instance_var("map$"),246,175,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	monster get_instance_var("map$"),245,175,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
end;

OnMobKill03:
	set_instance_var("count",get_instance_var("count") - 1);
	if(get_instance_var("count")) end;
	instance_enable("Injection Device#19m30",true);
end;

OnSummon04:
	monster get_instance_var("map$"),224,151,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill04";
	unittalk $@mobid[0],"The device is up and running, but why there's only one animal that has been Modified?";
	monster get_instance_var("map$"),225,141,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill04";
	unittalk $@mobid[0],"Did you turn off the device? They didn't say that it would wither and die so fast!!!";
	monster get_instance_var("map$"),217,141,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill04";
	unittalk $@mobid[0],"What? Where's everyone? Are you the one who called me out?";
	monster get_instance_var("map$"),230,160,"Unstable Modified Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill04";
end;

OnMobKill04:
	if(mobcount(get_instance_var("map$"),instance_npcname(strnpcinfo(0)) + "::OnMobKill04")) end;
	set_instance_var("csn",10);
	instance_announce false,"An additional injection device has been activated",bc_map,0x00FFCC;
	instance_enable("Injection Device#19m31",true);
	instance_enable("Injection Device#19m32",true);
	set_instance_var("dev_1",1);
	set_instance_var("dev_2",1);
	instance_event("Injection Device#19m31","OnSummon",false);
	instance_event("Injection Device#19m32","OnSummon",false);
	instance_event("#191_csn_mob","OnSummon",false);
	setcell get_instance_var("map$"),249,177,249,177,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),248,176,248,176,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),247,176,247,176,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),246,176,246,176,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),245,176,245,176,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),244,176,244,176,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),243,175,243,175,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),242,174,242,174,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),249,177,249,177,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),248,176,248,176,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),247,176,247,176,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),246,176,246,176,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),245,176,245,176,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),244,176,244,176,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),243,175,243,175,CELL_SHOOTABLE,true;
	setcell get_instance_var("map$"),242,174,242,174,CELL_SHOOTABLE,true;
end;

OnSummon05:
	instance_announce false,"Come over here and turn off the injection device.",bc_map,0x00FFCC;
	monster get_instance_var("map$"),191,199,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill05";
	unittalk $@mobid[0],"I told you to keep operating it so it doesn't turn off!";
	monster get_instance_var("map$"),199,197,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill05";
	unittalk $@mobid[0],"Wouldn't be better to get out of here?";
	monster get_instance_var("map$"),202,207,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill05";
	unittalk $@mobid[0],"You're in big trouble.";
end;

OnMobKill05:
	if(mobcount(get_instance_var("map$"),instance_npcname(strnpcinfo(0)) + "::OnMobKill05")) end;
	set_instance_var("csn",14);
end;

OnSummon06:
	set_instance_var("count",9);
	instance_disable("Crushed Rgan#19m41");
	instance_disable("Heart Hunter#19m40");
	monster get_instance_var("map$"),141,182,"Crushed Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	monster get_instance_var("map$"),149,178,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	monster get_instance_var("map$"),149,179,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	monster get_instance_var("map$"),150,180,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	sleep 3000;
	instance_disable("Modified Superior Rgan#19m40");
	instance_disable("Modified Superior Rgan#19m42");
	instance_disable("Modified Superior Rgan#19m43");
	instance_disable("Twisted Rgan#19m44");
	monster get_instance_var("map$"),141,183,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	monster get_instance_var("map$"),141,181,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	monster get_instance_var("map$"),141,180,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	monster get_instance_var("map$"),141,179,"Twisted Rgan",21535,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
	sleep 3000;
	instance_disable("Modified Superior Rgan#19m45");
	monster get_instance_var("map$"),141,178,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill06";
end;

OnMobKill06:
	set_instance_var("count",get_instance_var("count") - 1);
	if(get_instance_var("count")) end;
	setcell get_instance_var("map$"),140,178,140,183,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),140,178,140,183,CELL_SHOOTABLE,true;
	instance_announce false,"The Illusion's residence is dangerous. Someone...!",bc_map,0x00FFCC;
	instance_event("#191_csn_mob","OnSummon",false);
	instance_enable("#e19p03",true);
	instance_enable("Research Device#19m40",true);
	instance_enable("Research Device#19m41",true);
	instance_enable("Laboratory Traces#19m40",true);
	set_instance_var("dev_4",1);
	set_instance_var("dev_5",1);
	instance_event("Research Device#19m40","OnSummon",false);
	instance_event("Research Device#19m41","OnSummon",false);
end;

OnSummon07:
	monster get_instance_var("map$"),20,136,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill07";
	unittalk $@mobid[0],"You pretended to be a Rgan, did you come here to reminisce?";
	monster get_instance_var("map$"),30,139,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill07";
	unittalk $@mobid[0],"I told you not to touch it!";
end;

OnMobKill07:
	if(mobcount(get_instance_var("map$"),instance_npcname(strnpcinfo(0)) + "::OnMobKill07")) end;
	set_instance_var("csn",get_instance_var("csn") + 1);
	if(get_instance_var("csn") == 18)
		instance_event("#191_csn_control","OnEvent03",false);
end;

OnSummon08:
	monster get_instance_var("map$"),26,194,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill08";
	unittalk $@mobid[0],"Are you here looking for information?";
	monster get_instance_var("map$"),26,188,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill08";
	unittalk $@mobid[0],"Hands up!!";
	monster get_instance_var("map$"),19,185,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill08";
	unittalk $@mobid[0],"Can't you see what will happen?";
end;

OnMobKill08:
	if(mobcount(get_instance_var("map$"),instance_npcname(strnpcinfo(0)) + "::OnMobKill08")) end;
	set_instance_var("csn",get_instance_var("csn") + 1);
	if(get_instance_var("csn") == 18)
		instance_event("#191_csn_control","OnEvent03",false);
end;

OnSummon09:
	set_instance_var("count",26);
	instance_announce false,"Preparation for mana injection is ready. Stage 1 of the mana infusion has began.",bc_map,0x00FFCC;
	instance_disable("Modified Superior Rgan#19m51");
	instance_disable("Modified Superior Rgan#19m58");
	monster get_instance_var("map$"),35,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),42,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Modified Superior Rgan#19m53");
	instance_disable("Modified Superior Rgan#19m56");
	monster get_instance_var("map$"),37,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),40,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Furious Rgan#19m50");
	instance_disable("Furious Rgan#19m51");
	monster get_instance_var("map$"),31,249,"Furious Rgan",21590,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),47,237,"Furious Rgan",21590,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	removespecialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m50");
	removespecialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m51");
	removespecialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m50");
	removespecialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m51");
	instance_announce false,"Preparation for mana injection is ready. Stage 2 of the mana infusion has began.",bc_map,0x00FFCC;
	instance_disable("Modified Superior Rgan#19m50");
	instance_disable("Modified Superior Rgan#19m59");
	monster get_instance_var("map$"),34,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),43,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Modified Superior Rgan#19m54");
	instance_disable("Modified Superior Rgan#19m55");
	monster get_instance_var("map$"),38,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),39,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Modified Superior Rgan#19m52");
	instance_disable("Modified Superior Rgan#19m57");
	monster get_instance_var("map$"),36,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),41,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Furious Rgan#19m52");
	instance_disable("Furious Rgan#19m53");
	monster get_instance_var("map$"),47,249,"Furious Rgan",21590,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),31,237,"Furious Rgan",21590,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	removespecialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m52");
	removespecialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m53");
	removespecialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m52");
	removespecialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m53");
	instance_announce false,"Preparation for mana injection is ready. Stage 3 of the mana infusion has began.",bc_map,0x00FFCC;
	sleep 5000;
	instance_disable("Modified Superior Rgan#19m61");
	instance_disable("Modified Superior Rgan#19m65");
	monster get_instance_var("map$"),35,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),39,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Modified Superior Rgan#19m63");
	instance_disable("Modified Superior Rgan#19m67");
	monster get_instance_var("map$"),37,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),41,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Modified Superior Rgan#19m64");
	instance_disable("Modified Superior Rgan#19m68");
	monster get_instance_var("map$"),38,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),42,254,"Modified Superior Rgan",21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Heart Hunter#19m50");
	instance_disable("Heart Hunter#19m51");
	instance_disable("Heart Hunter#19m52");
	monster get_instance_var("map$"),39,250,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),36,250,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),41,252,"Heart Hunter",21536,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	sleep 5000;
	instance_disable("Furious Rgan#19m55");
	instance_disable("Furious Rgan#19m56");
	monster get_instance_var("map$"),27,243,"Furious Rgan",21590,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),50,243,"Furious Rgan",21590,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	monster get_instance_var("map$"),39,235,"Furious Rgan",21590,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill09";
	removespecialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m54");
	removespecialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m55");
	removespecialeffect EF_NPC_STOP,AREA,instance_npcname("Furious Rgan#19m56");
	removespecialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m54");
	removespecialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m55");
	removespecialeffect EF_GLASSWALL3,AREA,instance_npcname("Furious Rgan#19m56");
end;

OnMobKill09:
	set_instance_var("count",get_instance_var("count") - 1);
	if(get_instance_var("count")) end;
	set_instance_var("csn",20);
	instance_announce false,"Is that Lasgand's residence?",bc_map,0x00FFCC;
	setcell get_instance_var("map$"),34,255,43,255,CELL_WALKABLE,true;
	setcell get_instance_var("map$"),34,255,43,255,CELL_SHOOTABLE,true;
	instance_enable("Leon#19m60",true);
	instance_enable("Horuru#19m60",true);
	instance_enable("Aurelie#19m60",true);
	instance_enable("Central Door#19m61",true);
end;
}

1@jorchs,249,79,7	script	Frightened Rgan#19m00	21600,5,5,{
	end;
	
OnTouch:
	if(get_instance_var(strnpcinfo(2))) end;
	set_instance_var(strnpcinfo(2),1);
	.@id = atoi(replacestr(strnpcinfo(2),"19m0",""));
	switch(.@id){
		case 0:
			instance_hide("Frightened Rgan#19m00",true);
			instance_enable("Frightened Rgan#19m50",true);
			npctalk "I'm scared... what's wrong with my body?",instance_npcname("Frightened Rgan#19m50");
			sleep 2000;
			npctalk "You... a human??? Is this all your fault?? What have you done to me?!",instance_npcname("Frightened Rgan#19m50");
			sleep 2000;
			npctalk "Filthy human beings! Bring my body back...! Agh...!!",instance_npcname("Frightened Rgan#19m50");
			sleep 2000;
			specialeffect EF_FOOD02,AREA,instance_npcname("Frightened Rgan#19m50");
			instance_hide("Frightened Rgan#19m50",true);
			break;
			
		case 1:
			instance_hide("Frightened Rgan#19m01",true);
			instance_enable("Frightened Rgan#19m51",true);
			npctalk "Am, am I supposed to be like this...?",instance_npcname("Frightened Rgan#19m51");
			sleep 2000;
			npctalk "I lived a good life, what did they to do me...",instance_npcname("Frightened Rgan#19m51");
			sleep 2000;
			npctalk "Ah... it's cold....",instance_npcname("Frightened Rgan#19m51");
			sleep 2000;
			npctalk "...?! Human? Human?? Are you the one who turned me into this???",instance_npcname("Frightened Rgan#19m51");
			sleep 2000;
			specialeffect EF_FOOD02,AREA,instance_npcname("Frightened Rgan#19m51");
			instance_hide("Frightened Rgan#19m51",true);
			break;
			
	}
	getmapxy(.@m$,.@x,.@y,BL_NPC);
	monster get_instance_var("map$"),.@x,.@y,strnpcinfo(1),21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
end;

OnMobKill:
	set_instance_var("elite",get_instance_var("elite") + 1);
	if(get_instance_var("elite") == 2){
		set_instance_var("elite",0);
		set_instance_var("search",1);
		if(!get_instance_var("clear"))
			instance_announce false,"We'd better do a more thorough search.",bc_map,0x00FFCC;
		else
			instance_event("#191_csn_control","OnEvent01",false);
	}
	instance_enable(strnpcinfo(0),false);
end;
}

1@jorchs,114,20,3	duplicate(Frightened Rgan#19m00)	Frightened Rgan#19m01	21600,5,5
1@jorchs,249,79,7	duplicate(dummynpc2)	Frightened Rgan#19m50	21600
1@jorchs,114,20,3	duplicate(dummynpc2)	Frightened Rgan#19m51	21600

1@jorchs,23,57,5	script	Stranded Rgan#19m10	21600,5,5,{
	end;
	
OnTouch:
	if(get_instance_var(strnpcinfo(2))) end;
	set_instance_var(strnpcinfo(2),1);
	.@id = atoi(replacestr(strnpcinfo(2),"19m1",""));
	switch(.@id){
		case 0:
			instance_hide("Stranded Rgan#19m10",true);
			instance_enable("Stranded Rgan#19m60",true);
			npctalk "Where is everyone... They told me to keep an eye of the eggs, But I can't leave here...",instance_npcname("Stranded Rgan#19m60");
			sleep 2000;
			npctalk "Humans? Humans can't come here! Get out!",instance_npcname("Stranded Rgan#19m60");
			sleep 2000;
			npctalk "My!! Body...! They said it was a...? Supplement? ?...!!",instance_npcname("Stranded Rgan#19m60");
			sleep 2000;
			specialeffect EF_FOOD02,AREA,instance_npcname("Frightened Rgan#19m51");
			instance_hide("Stranded Rgan#19m60",true);
			break;
			
		case 1:
			instance_hide("Stranded Rgan#19m11",true);
			instance_enable("Stranded Rgan#19m61",true);
			npctalk "I need to hurry and get to the center. How do I get there? There some weird noises...",instance_npcname("Stranded Rgan#19m61");
			sleep 2000;
			npctalk "When am I going to change. I need to transform to get through it.",instance_npcname("Stranded Rgan#19m61");
			sleep 2000;
			specialeffect EF_FOOD02,AREA,instance_npcname("Frightened Rgan#19m51");
			npctalk "Oh! Finally!!! I'll have the chance to turn into a Bishop...!!! ?!!!",instance_npcname("Stranded Rgan#19m61");
			instance_hide("Stranded Rgan#19m61",true);
			break;
	}
	getmapxy(.@m$,.@x,.@y,BL_NPC);
	monster get_instance_var("map$"),.@x,.@y,strnpcinfo(1),21534,1,instance_npcname(strnpcinfo(0)) + "::OnMobKill";
end;

OnMobKill:
	set_instance_var("elite",get_instance_var("elite") + 1);
	if(get_instance_var("elite") == 2){
		set_instance_var("elite",0);
		set_instance_var("search",1);
		if(!get_instance_var("clear"))
			instance_announce false,"There are still Rgans in the search area, you'd better clear it up quickly.",bc_map,0x00FFCC;
		else {
			set_instance_var("csn",4);
			instance_announce false,"Wah!!! What is this!!!",bc_map,0x00FFCC;
			instance_enable("Horuru#19m20",true);
			instance_enable("Iwin#19m20",true);
		}
	}
	instance_enable(strnpcinfo(0),false);
end;
}

1@jorchs,21,97,5	duplicate(Stranded Rgan#19m10)	Stranded Rgan#19m11	21600,5,5
1@jorchs,23,57,5	duplicate(dummynpc2)	Stranded Rgan#19m60	21600
1@jorchs,21,97,5	duplicate(dummynpc2)	Stranded Rgan#19m61	21600
