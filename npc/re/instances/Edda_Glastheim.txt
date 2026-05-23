//===== Crazyarashi Services =====================================|
//= Edda Glastheim                                               =|
//===== By: ======================================================|
//= crazyarashi                                                  =|
//===== Current Version: =========================================|
//= 1.0 Initial Version                                          =|
//================================================================|

glast_01,244,290,0	script	#gh_fall_ev_00	HIDDEN_WARP_NPC,3,3,{
	end;
	
OnTouch:
	if(BaseLevel >= 130){
		cloaknpc("Oscar#gh_fall",false,getcharid(0));
		if(isbegin_quest(12458) == 2){
			cloaknpc("OSC1198#edda_fogh",false,getcharid(0));
			cloaknpc("OSC1127#edda_fogh",false,getcharid(0));
			cloaknpc("OSC1052#edda_fogh",false,getcharid(0));
			specialeffect EF_YELLOBODY,AREA,"OSC1198#edda_fogh";
			specialeffect EF_YELLOBODY,AREA,"OSC1127#edda_fogh";
			specialeffect EF_YELLOBODY,AREA,"OSC1052#edda_fogh";
			specialeffect EF_WHITEBODY,AREA,"OSC1198#edda_fogh";
		    specialeffect EF_WHITEBODY,AREA,"OSC1127#edda_fogh";
			specialeffect EF_WHITEBODY,AREA,"OSC1052#edda_fogh";
		}
	}
end;
}

glast_01,241,290,5	script	Oscar#gh_fall	4_ED_OSCAR,{
	if(isbegin_quest(12457) == 0){
		cutin "oscar02",2;
		mes "[ Oscar ]";
		mes "Oh! You can see me?";
		emotion ET_BIGTHROB;
		next;
		select("Yes, you look normal.");
		cutin "oscar01",2;
		mes "[ Oscar ]";
		mes "Huh, how many times have you been in and out of the dimensional gap?";
		next;
		mes "[ Oscar ]";
		mes "Hehehe~";
		mes "Oh! Really! You see, I can smell people who had been into other dimensions.";
		next;
		select("I think you're smelling yourself.");
		cutin "oscar02",2;
		mes "[ Oscar ]";
		mes "It's definitely you. The people in the dimensional gap have no smell.";
		next;
		select("Who are you?");
		cutin "oscar01",2;
		mes "[ Oscar ]";
		mes "You can call me ^0000CDOscar^000000.";
		mes "It's a hobby of mine to collect records from people that had been in and out of the dimensional gap.";
		next;
		mes "[ Oscar ]";
		mes "I was visiting my favorite time and space, I was about to leave but I was quite surprised to see you stare at me.";
		next;
		select("Sorry if I interrupted you.:I'm busy.");
		mes "[ Oscar ]";
		mes "Wait. How many times did you travel? Do you have a lot of experience? Where is the most impressive dimensional gap you've been to?";
		next;
		select("Please ask one question at a time.");
		cutin "oscar05",2;
		mes "[ Oscar ]";
		mes "Ah... I'm sorry. It's the first time I've met someone else with dimensional travel experience other than myself.";
		next;
		mes "[ Oscar ]";
		mes "I often speak with wise men like Varmundt, but even if I try to speak to him about this stuff...";
		next;
		select("Does Varmundt give you a hard time?");
		cutin "oscar04",2;
		mes "[ Oscar ]";
		mes "Yes, I try not to get too involve.";
		mes "I want to stay polite. I haven't talked to him much about dimensional travels.";
		next;
		cutin "",255;
		mes "[ "+strcharinfo(0)+" ]";
		mes "I don't know you well, but I know one thing for sure. You sure make some funny jokes.";
		next;
		cutin "oscar01",2;
		mes "[ Oscar ]";
		mes "That's quite normal though. It's no wonder that sages are shy when they're 500 year old.";
		next;
		select("Wait, what did you just say?");
		cutin "oscar02",2;
		mes "[ Oscar ]";
		mes "What? That sages are very shy...?";
		next;
		select("No, not that...");
		select("What about the rest?");
		mes "[ Oscar ]";
		mes "My thoughts are quite complex and hard to follow sometimes. Is it hot? I thought it was cold.";
		next;
		select("500 years old...");
		mes "[ Oscar ]";
		mes "Yes. Why are you suddenly like that?";
		next;
		select("Are you making fun of me?");
		cutin "oscar03",2;
		mes "[ Oscar ]";
		mes "Yeah, I always do that! I was about to observe time and space. Wait?! Why are you lying on the floor?!";
		next;
		select("Can you introduce yourself again?");
		cutin "oscar02",2;
		mes "[ Oscar ]";
		mes "Ah, you're an adventurer!";
		mes strcharinfo(0) + ". Now get up, it's time to go to my favorite time and space.";
		next;
		mes "[ Oscar ]";
		mes "Do you want to visit a dimensional gap you've never seen before?";
		next;
		select("Please show me, I'd be honored.");
		cutin "oscar01",2;
		mes "[ Oscar ]";
		mes "Good. I'm going to see the Fall of Glast Heim. I'm glad that you decided to go along with me.";
		next;
		select("Agree with him.");
		mes "[ Oscar ]";
		mes "I knew you'd be happy. Then get ready and help me open the dimensional gap.";
		setquest 12457;
		close3;
	}
	if(isbegin_quest(12457) == 1){
		.@md_name$ = "Glastheim Fall";
		.@party_id = getcharid(1);
		getpartymember .@party_id,3;
		if(!.@party_id || $@partymembercount != 1 || is_party_leader() == false){
			cutin "oscar01",2;
			mes "[ Oscar ]";
			mes "I can only activate it for you adventurer. Please create a party of your own and talk to me again.";
			close3;
		}
		cutin "oscar01",2;
		mes "[ Oscar ]";
		mes "Are you mentally ready? Focus your energy on my hand to follow me.";
		emotion ET_BIGTHROB;
		next;
		.@s = select(((instance_id(IM_PARTY))?"":"Create a Dimensional Gap."),((instance_id(IM_PARTY))?"Enter the Dimensional Gap.":""));
		switch(.@s){
			case 1:
				switch(instance_create(.@md_name$)){
					case -1:
						mes "[ Oscar ]";
						mes "Unknown Error Has Occurred.";
						close3;
						
					case -2:
						mes "[ Oscar ]";
						mes "I can only activate it for you adventurer. Please create a party of your own and talk to me again.";
						close3;
						
					case -3:
						mes "[ Oscar ]";
						mes "Oh, it seems you still have an active dimensional gap.";
						mes "Let's wait for it to fade away.";
						close3;
						
					case -4:
						mes "[ Oscar ]";
						mes "Instance reservation is full at the moment. Try again later.";
						close3;
						
					default:
						mes "[ Oscar ]";
						mes "We succesfully created the dimensional gap. Talk to me again to enter the dimensional gap.";
						close3;
				}
				
			case 2:
				switch(instance_enter(.@md_name$)){
					case IE_OTHER:
						mes "An unknown error has occurred.";
						end;
						
					case IE_NOINSTANCE:
						mes "^ff0000Character doesn't have an instance reserved.^000000";
						end;
						
					case IE_NOMEMBER:	
						mes "Only registered members can enter the " + .@md_name$ + " instance.";
						end;
						
					case IE_OK:
						mapannounce "glast_01", strcharinfo(0) + " of the party, "+ .@party_name$ +", is entering the " + .@md_name$ + ".", bc_map, "0x00FF99";
						end;
				}			
		}
	}
	if(isbegin_quest(12458) == 0){
		cutin "oscar05",2;
		mes "[ Oscar ]";
		mes "Unfortunately, there was no way to stop Schmidt from being swallowed by the orb.";
		next;
		select("Was the orb destroyed?");
		mes "[ Oscar ]";
		mes "Yes. After the incident, the orb was definitely stopped and there was no more sinister energy that drains the energy of the living. That's the good thing.";
		next;
		cutin "oscar04",2;
		mes "[ Oscar ]";
		mes "But stopping the orb's energy didn't stop the energy of death that covered the area.";
		next;
		cutin "oscar05",2;
		mes "[ Oscar ]";
		mes "Schmidt's memory fragment started to appear here and there.";
		next;
		select("So what happened to Schmidt...");
		cutin "oscar04",2;
		mes "[ Oscar ]";
		mes "I looked at whether or not Schmidt's soul remained inside the orb, I couldn't find his soul, perhap he entirely disappeared.";
		next;
		mes "[ Oscar ]";
		mes "However, Schmidt's commitment and beliefs were transformed into a political weakness for other rules of Midgard.";
		next;
		select("Why political...");
		cutin "oscar05",2;
		mes "[ Oscar ]";
		mes "The rumors spread very fast, the act of throwing your own body to save the lives of others turned into the fall of the king into madness.";
		next;
		mes "[ Oscar ]";
		mes "In the records, King Schmidt is mentioned as a king who became crazy and greedy for wealth.";
		next;
		cutin "oscar04",2;
		mes "[ Oscar ]";
		mes "I guess the real soul of Schmidt is alive somewhere. In my dimensional journeys, King Schmidt's thoughts fell ten of thousands of times.";
		next;
		mes "[ Oscar ]";
		mes "Collecting and exploring the traces from the corpses might someday reveal what truly happened to King Schmidt's soul...";
		next;
		select("Why do you go so far?");
		cutin "oscar01",2;
		mes "[ Oscar ]";
		mes "First of all, I want you to know this, but I don't cherish life and its preciousness the same way as other sages.";
		next;
		mes "[ Oscar ]";
		mes "I don't want to believe that sacrificing yourself is a noble action.";
		next;
		mes "[ Oscar ]";
		mes "I, Oscar, who controls the time and records. My mission is to go into different time and space.";
		next;
		select("Can I help in anyway?");
		cutin "oscar02",2;
		mes "[ Oscar ]";
		mes "Hmm... Maybe there's something you can be of help with.";
		next;
		cutin "oscar01",2;
		mes "[ Oscar ]";
		mes "We can purify Schmidt's beliefs by collecting crystal and the debris. Of course, I'm collecting them by myself, but the more the better.";
		next;
		mes "[ Oscar ]";
		mes "Ah. Let me introduce you to these machines in front of me. Putting the right crystals in them will reflect my will so you can trade them for enchants and other stuffs.";
		next;
		cloaknpc("OSC1198#edda_fogh",false,getcharid(0));
		cloaknpc("OSC1127#edda_fogh",false,getcharid(0));
		cloaknpc("OSC1052#edda_fogh",false,getcharid(0));
		select("I don't know how to do that.");
		cutin "oscar03",2;
		mes "[ Oscar ]";
		mes "It's called the sage's power. In any case, speak to me whenever you want. I'll open the dimension to purify his soul.";
		setquest 12458;
		completequest 12458;
		close3;
	}
	cutin "oscar03",2;
	mes "[ Oscar ]";
	mes "Is your mind all set? Then, please put your hand on my hand and concentrate.";
	next;
	switch(checkquest(12460,PLAYTIME)){
		case -1:
			.@normal = true;
			break;
			
		case 0:
		case 1:
			break;
			
		case 2:
			erasequest 12460;
			.@normal = true;
			break;
	}
	switch(checkquest(12461,PLAYTIME)){
		case -1:
			.@hard = true;
			break;
			
		case 0:
		case 1:
			break;
			
		case 2:
			erasequest 12461;
			.@hard = true;
			break;
	}
	if(.@normal == false && .@hard == false){
		cutin "oscar03",2;
		mes "[ Oscar ]";
		mes "^0000CDWe've purified enough for today.^000000 You need to rest up your body to do another dimensional travel. Come back again at dawn, adventurer.";
		close3;	
	}
	switch(select("Enter",((is_party_leader() == true && .@normal == true)?"Create Glastheim Purification":""),((is_party_leader() == true && BaseLevel >= 180 && .@hard == true)?"Create Glastheim Purification Hard":""))){
		case 1:
			if(select("Purification of Glastheim",((BaseLevel >= 180)?"^CC0000Purification of Glastheim Hard^000000":"")) == 1){
				.@md_name$ = "Glastheim Purification";
				.@cooldown = 12460;
				.@hunt = 12459;
				.@check = .@normal;
			} else {
				.@md_name$ = "Glastheim Purification Hard";
				.@cooldown = 12461;
				.@hunt = 12462;
				.@check = .@hard;
			}
			if(!.@check){
				cutin "oscar03",2;
				mes "[ Oscar ]";
				mes "^0000CDWe've purified enough for today.^000000 You need to rest up your body to do another dimensional travel. Come back again at dawn, adventurer.";
				close3;
			}
			switch(instance_enter(.@md_name$)){
				case IE_OTHER:
					mes "An unknown error has occurred.";
					close3;
					
				case IE_NOINSTANCE:
					mes "^ff0000Character doesn't have an instance reserved.^000000";
					close3;
					
				case IE_NOMEMBER:	
					mes "Only registered members can enter the " + .@md_name$ + " instance.";
					close3;
					
				case IE_OK:
					mapannounce "glast_01", strcharinfo(0) + " of the party, "+ .@party_name$ +", is entering the " + .@md_name$ + ".", bc_map, "0x00FF99";
					if(isbegin_quest(.@hunt))
						erasequest .@hunt;
					setquest .@hunt;
					setquest .@cooldown;
					end;
			}	
			
		case 2:
			.@party_id = getcharid(1);
			if(!.@party_id){
				mes "[ Oscar ]";
				mes "I can only activate it for you adventurer. Please create a party of your own and talk to me again.";
				close3;
			}
			.@md_name$ = "Glastheim Purification";
			switch(instance_create(.@md_name$)){
				case -1:
					mes "[ Oscar ]";
					mes "Unknown Error Has Occurred.";
					close3;
					
				case -2:
					mes "[ Oscar ]";
					mes "I can only activate it for you adventurer. Please create a party of your own and talk to me again.";
					close3;
					
				case -3:
					mes "[ Oscar ]";
					mes "Oh, it seems you still have an active dimensional gap.";
					mes "Let's wait for it to fade away.";
					close3;
					
				case -4:
					mes "[ Oscar ]";
					mes "Instance reservation is full at the moment. Try again later.";
					close3;
					
				default:
					mes "[ Oscar ]";
					mes "We succesfully created the dimensional gap. Talk to me again to enter the dimensional gap.";
					close3;
			}
			
		case 3:
			.@party_id = getcharid(1);
			if(!.@party_id){
				mes "[ Oscar ]";
				mes "I can only activate it for you adventurer. Please create a party of your own and talk to me again.";
				close3;
			}
			.@md_name$ = "Glastheim Purification Hard";
			switch(instance_create(.@md_name$)){
				case -1:
					mes "[ Oscar ]";
					mes "Unknown Error Has Occurred.";
					close3;
					
				case -2:
					mes "[ Oscar ]";
					mes "I can only activate it for you adventurer. Please create a party of your own and talk to me again.";
					close3;
					
				case -3:
					mes "[ Oscar ]";
					mes "Oh, it seems you still have an active dimensional gap.";
					mes "Let's wait for it to fade away.";
					close3;
					
				case -4:
					mes "[ Oscar ]";
					mes "Instance reservation is full at the moment. Try again later.";
					close3;
					
				default:
					mes "[ Oscar ]";
					mes "We succesfully created the dimensional gap. Talk to me again to enter the dimensional gap.";
					close3;
			}		
	}
}

glast_01,241,296,5	script	OSC1198#edda_fogh	2_DROP_MACHINE,{
	disable_items;
	mes "[ OSC1198 ]";
	mes "I can dismantle your unnecessary Glastheim's Fall artifact and partially return it as materials.";
	next;
	switch(select("Cancel:Armor:Garment:Left Accessory")){
		case 1:
			mes "[ OSC1198 ]";
			mes "Wise decision. Think about it carefully.";
			end;
			
		case 2:
			.@slot = EQI_ARMOR;
			.@equip_id = getequipid(.@slot);
			.@bool = inarray(.equip_id,.@equip_id);
			break;
			
		case 3:
			.@slot = EQI_GARMENT;
			.@equip_id = getequipid(.@slot);
			.@bool = inarray(.equip_id,.@equip_id);
			break;
			
		case 4:
			.@slot = EQI_ACC_L;
			.@equip_id = getequipid(.@slot);
			.@bool = inarray(.insignia,.@equip_id);
			.@stone = 25740;
			break;
	}
	if(.@bool == -1){
		mes "[ OSC1198 ]";
		mes "Did you remove the equipment?";
		end;
	}
	for(.@i = 0; .@i < 4; .@i++){
		.@card[.@i] = getequipcardid(.@slot,.@i);
		.@check[.@i] = .@card[.@i];
	}
	.@refine = getequiprefinerycnt(.@slot);
	.@enchant_count = ((.@card[1] > 0 ? 3 : (.@card[2] > 0 ? 2 : (.@card[3] > 0 ? 1 : 0))));
	if(!.@stone)
		.@stone = 25739;	
	switch(.@enchant_count){
		case 0:
			mes "[ OSC1198 ]";
			mes "Your equipment must atleast have one enchant to get a partial refund.";
			end;
			
		case 1:
			if(.@stone == 25739){
				.@min = 3;
				.@max = 6;
			} else {
				.@min = 2;
				.@max = 6;
			}
			break;
			
		case 2:
			if(.@stone == 25739){
				.@min = 6;
				.@max = 10;
			} else {
				.@min = 4;
				.@max = 12;
			}
			break;
			
		case 3:
			if(.@stone == 25739){
				.@min = 9;
				.@max = 15;
			} else {
				.@min = 6;
				.@max = 18;
			}
			break;					
	}
	mes "[ OSC1198 ]";
	mes "Are you sure you want to dismantle the ^CC0000" + getitemname(.@equip_id) + "^000000 that you currently have equipped?";
	next;
	if(select("No.:Please stop!:Yes, I'm sure.") < 3){
		mes "[ OSC1198 ]";
		mes "Wise decision. Think about it carefully.";
		end;
	}
	if(callfunc("F_IsEquipIDHack", .@slot, .@equip_id) || callfunc("F_IsEquipRefineHack", .@slot, .@refine) || callfunc("F_IsEquipCardHack", .@slot, .@check[0], .@check[1], .@check[2], .@check[3]))
		end;
	mes "[ OSC1198 ]";
	mes "Dismantling the equipment...";
	progressbar_npc "00FF00",2;
	delequip .@slot;
	mes "Your equipment has been succesfully processed.";
	getitem .@stone,rand(.@min,.@max);
	if(rand(1,100) <= .chance){
		next;
		.@costume_id = .drooping_id[rand(getarraysize(.drooping_id))];
		mes "[ OSC1198 ]";
		mes "*Pi-pi-ping*";
		mes "It's your lucky day... I have succesfully created a memento out of the equipment that you recycled.";
		getitem .@costume_id,1;
	}
	end;
	
OnInit:
	cloaknpc(strnpcinfo(0),true);
	setarray .equip_id,15388,15389;
	setarray .insignia,32228,32229,32230,32231,32232,32233;
	setarray .drooping_id,31631,31632,31633;
	.chance = 3; //Chance to get a drooping costume
	setunittitle getnpcid(0),"[Dismantle Machine]";
end;
}

glast_01,243,296,5	script	OSC1127#edda_fogh	2_SLOT_MACHINE,{
	disable_items;
	mes "[ OSC1127 ]";
	mes "If you want to strengthen your Glastheim's Fall artifacts, then you're at the correct machine. Select the artifact that you want to enchance.";
	next;
	switch(select("Cancel:Armor:Garment:Left Accessory")){
		case 1:
			mes "[ OSC1127 ]";
			mes "Come back again when you have decided.";
			end;
			
		case 2:
			.@part = EQI_ARMOR;
			.@equip_id = getequipid(.@part);
			.@bool = inarray(.equip_id,.@equip_id);
			setarray .@requirement,25739,5,6608,10;
			
			break;
			
		case 3:
			.@part = EQI_GARMENT;
			.@equip_id = getequipid(.@part);
			.@bool = inarray(.equip_id,.@equip_id);
			setarray .@requirement,25739,5,6608,10;
			break;
			
		case 4:
			.@part = EQI_ACC_L;
			.@equip_id = getequipid(.@part);
			.@bool = inarray(.insignia,.@equip_id);
			setarray .@requirement,25740,10,6755,40;
			break;
	}
	if(.@bool == -1){
		mes "[ OSC1127 ]";
		mes "Did you remove the equipment?";
		end;
	}
	.@refine = getequiprefinerycnt(.@part);
	.@equip_name$ = getitemname(.@equip_id);
	for(.@i = 0; .@i < 4; .@i++){
		.@card[.@i] = getequipcardid(.@part,.@i);
		.@check[.@i] = .@card[.@i];
	}
	if(.@card[1] > 0){
		mes "[ OSC1127 ]";
		mes "Your equipment is already enchanted to the limit.";
		end;
	}
	.@slot = ((.@card[3] == 0 ? 3 : (.@card[2] == 0 ? 2 : 1)));
	switch(.@slot){
		case 1:
			if(.@part == EQI_ARMOR)
				setarray .@enchant_list,4702,4712,4722,4732,4742,4752,29302,29303,29304,29305,29306,29307,29308,29309;
			else if(.@part == EQI_GARMENT)
				setarray .@enchant_list,4808,4820,4821,4813,4812,4826,4817,4816,4843,4834,4835,4836,4863,4864,4865;
			else {
				setarray .@enchant_list,29587,29588,29589,29590,29591,29592;
				.@break = 30;
			}
			.@word$ = "Final";
			break;
			
		case 2:
			if(.@part == EQI_ARMOR)
				setarray .@enchant_list,4702,4703,4704,4712,4713,4714,4722,4723,4724,4732,4733,4734,4742,4743,4744,4752,4753,4754;
			else if(.@part == EQI_GARMENT)
				setarray .@enchant_list,4808,4820,4821,4813,4812,4826,4817,4816,4843,4834,4835,4836,4863,4864,4865;
			else
				setarray .@enchant_list,4821,4822,4812,4826,4843,4844,4835,4836,4864,4865;
			.@word$ = "Second";
			break;
		
		case 3:
			if(.@part == EQI_ARMOR)
				setarray .@enchant_list,4702,4703,4704,4712,4713,4714,4722,4723,4724,4732,4733,4734,4742,4743,4744,4752,4753,4754;
			else if(.@part == EQI_GARMENT)
				setarray .@enchant_list,4808,4820,4821,4813,4812,4826,4817,4816,4843,4834,4835,4836,4863,4864,4865;
			else
				setarray .@enchant_list,4702,4703,4704,4712,4713,4714,4722,4723,4724,4732,4733,4734,4742,4743,4744,4752,4753,4754;
			.@word$ = "First";
			break;
	}
	mes "[ OSC1127 ]";
	mes "^0000CD" + .@equip_name$ + "^000000";
	mes "Slot : " + .@word$ + " Enchant";
	mes "Requirement : ";
	for(.@i = 0; .@i < getarraysize(.@requirement); .@i += 2){
		mes "^0000CD" + .@requirement[.@i+1] + " " + getitemname(.@requirement[.@i]) + "^000000";
		if(countitem(.@requirement[.@i]) < .@requirement[.@i+1])
			.@miss++;
	}
	if(.@break){
		mes "";
		mes "This item has a " + .@break + "% chance to break on adding the final slot, do you want to proceed?";
	}
	next;
	if(select("Cancel:Proceed") == 1){
	}
	if(.@miss){
		mes "[ OSC1127 ]";
		mes "You don't have enough material to proceed with the enchant.";
		end;
	}
	.@enchant = .@enchant_list[rand(getarraysize(.@enchant_list))];
	.@card[.@slot] = .@enchant;
	for(.@i = 0; .@i < getarraysize(.@requirement); .@i += 2)
		delitem .@requirement[.@i],.@requirement[.@i+1];
	if(rand(1,100) > .@break){
		mes "[ OSC1127 ]";
		mes "Your equipment has been successfully enchanted.";
		specialeffect2 EF_REPAIRWEAPON;
		delequip .@part;
		getitem2 .@equip_id,1,1,.@refine,0,.@card[0],.@card[1],.@card[2],.@card[3];
	} else {
		mes "[ OSC1127 ]";
		mes "Your equipment broke through the enchantment process.";	
		specialeffect2 EF_REFINEFAIL;
		delequip .@part;
	}
	end;
	
OnInit:
	setarray .equip_id,15388,15389;
	setarray .insignia,32228,32229,32230,32231,32232,32233;
	cloaknpc(strnpcinfo(0),true);
	setunittitle getnpcid(0),"[Enchant Machine]";
end;
}

glast_01,245,296,5	script	OSC1052#edda_fogh	2_VENDING_MACHINE1,{
	mes "[ OSC1052 ]";
	mes "You have preserved what's left of Glastheim's Fall.";
	mes "Select the item you desire to view it's information.";
	next;
	switch(select("Cancel:Armor:Garment:Left Accessory")){
		case 1:
			mes "[ OSC1052 ]";
			mes "Come back whenever you can. Those who jump through dimensions are always welcome.";
			end;
			
		case 2:
			.@item_id = .equip_id[0];
			setarray .@requirement,25739,5,6607,10;
			.@item$ = " Suit[1]";
			break;
			
		case 3:
			.@item_id = .equip_id[1];
			setarray .@requirement,25739,5,6607,10;
			.@item$ = " Manteau[1]";
			break;
			
		case 4:
			mes "[ OSC1052 ]";
			mes "Select the type of accessory that you desire.";
			next;
			setarray .@menu$,"Power","Divine Power","Hundred Lucks","Flash","Celestial";
			.@s = select("Power:Divine Power:Hundred Lucks:Flash:Celestial") - 1;
			.@item_id = .insignia[.@s];
			setarray .@requirement,25740,5,6755,5;
			.@item$ = " " + .@menu$[.@s] + " Insignia[1]";
			break;
	}
	mes "[ OSC1052 ]";
	mes "[<ITEM>King Schmidt's" + .@item$ + "<INFO>" + .@item_id + "</INFO></ITEM>]";
	mes "Requirement : ";
	for(.@i = 0; .@i < getarraysize(.@requirement); .@i += 2){
		mes "^0000CD" + .@requirement[.@i+1] + " " + getitemname(.@requirement[.@i]) + "^000000";
		if(countitem(.@requirement[.@i]) < .@requirement[.@i+1])
			.@miss++;
	}
	next;
	if(select("Cancel:Buy Equipment") == 1){
		mes "[ OSC1052 ]";
		mes "Come back whenever you can. Those who jump through dimensions are always welcome.";
		end;	
	}
	if(.@miss){
		mes "[ OSC1052 ]";
		mes "You don't have enough materials to buy this equipment.";
		end;
	}
	for(.@i = 0; .@i < getarraysize(.@requirement); .@i += 2)
		delitem .@requirement[.@i],.@requirement[.@i+1];
	mes "[ OSC1052 ]"; 
	mes "Here's your <ITEM>King Schmidt's" + .@item$ + "<INFO>" + .@item_id + "</INFO></ITEM>";
	getitem .@item_id,1;
	end;
	
OnInit:
	setarray .equip_id,15388,15389;
	setarray .insignia,32228,32229,32230,32231,32232,32233;
	cloaknpc(strnpcinfo(0),true);
	setunittitle getnpcid(0),"[Exchange Machine]";
end;
}

1@gl_prq,1,1,0	script	#gh_fall_control	-1,{
	end;

OnInstanceInit:
	set_instance_var("map$",instance_mapname("1@gl_prq"));
	.@type$ = instance_live_info(ILI_NAME);
	instance_enable("#edda_fence_0",false);
	instance_enable("#edda_warp_0",false);
	instance_enable("Orb#edda_fall_0",false);
	instance_enable("#gh_purification_ev_0",false);
	instance_enable("#gh_purification_mob",false);
	instance_enable("#gh_purification_boss",false);
	instance_enable("#gh_purification_fire",false);
	instance_enable("#schmidt_gh_purification",false);
	
	for(.@i = 0; .@i < 2; .@i++)
		instance_enable("Heinrich#gh_fall_" + .@i,false);
	for(.@i = 0; .@i < 3; .@i++){
		instance_enable("Oscar#gh_fall_" + .@i,false);
		instance_enable("#gh_fall_ev_" + .@i,false);
	}
	for(.@i = 0; .@i < 4; .@i++)
		instance_enable("Varmundt#gh_fall_" + .@i,false);
	for(.@i = 0; .@i < 5; .@i++){
		instance_enable("Schmidt#gh_fall_" + .@i,false);
		instance_enable("Oscar#gh_purification_" + .@i,false);
	}
	for(.@i = 0; .@i < 8; .@i++)
		instance_enable("Knight Survivor#gh_fall_" + .@i,false);
	for(.@i = 0; .@i < 16; .@i++)
		instance_enable("Victim#gh_fall_" + .@i,false);
	for(.@i = 0; .@i < 44; .@i++)
		instance_enable("#schmidt_dummy_" + .@i,false);
	
	//= Fall of Glastheim
	if(.@type$ == "Glastheim Fall"){
		instance_enable("#gh_fall_ev_0",true);
		for(.@i = 0; .@i < 4; .@i++){
			instance_enable("Varmundt#gh_fall_" + .@i,true);
			instance_hide("Varmundt#gh_fall_" + .@i,true);
		}
		for(.@i = 0; .@i < 5; .@i++){
			instance_enable("Schmidt#gh_fall_" + .@i,true);
			instance_hide("Schmidt#gh_fall_" + .@i,true);
		}
		for(.@i = 0; .@i < 2; .@i++){
			instance_enable("Heinrich#gh_fall_" + .@i,true);
			instance_hide("Heinrich#gh_fall_" + .@i,true);
		}
		for(.@i = 0; .@i < 6; .@i++){
			instance_enable("#gh_fall_guard_" + .@i,true);
			instance_hide("#gh_fall_guard_" + .@i,true);
		}
		for(.@i = 0; .@i < 3; .@i++){
			instance_enable("Oscar#gh_fall_" + .@i,true);
			instance_hide("Oscar#gh_fall_" + .@i,true);
		}
		for(.@i = 0; .@i < 44; .@i++){
			instance_enable("#schmidt_dummy_" + .@i,true);
			instance_hide("#schmidt_dummy_" + .@i,true);
		}
		end;
	}
	if(.@type$ == "Glastheim Purification"){	
		set_instance_var("maid",20392);
		set_instance_var("khalitzburg",20388);
		set_instance_var("knight",20390);
		set_instance_var("bloody",20394);
		set_instance_var("schmidt",20386);
		set_instance_var("hunt",12459);
	} else {
		set_instance_var("mode",1);
		set_instance_var("maid",20392);
		set_instance_var("khalitzburg",20389);
		set_instance_var("knight",20391);
		set_instance_var("bloody",20394);
		set_instance_var("schmidt",20387);	
		set_instance_var("hunt",12462);
	}
	instance_enable("#gh_purification_ev_0",true);
	instance_enable("Oscar#gh_purification_0",true);
	instance_enable("#gh_purification_mob",true);
	instance_enable("#gh_purification_boss",true);
	instance_enable("#gh_purification_fire",true);
	instance_event("#gh_purification_mob","OnSpawn00",false);
end;
}

1@gl_prq,299,17,0	script	#gh_fall_ev_0	HIDDEN_WARP_NPC,3,2,{
	end;
	
OnTouch:
	.@var = get_instance_var("gh_fall");
	set_instance_var("gh_fall",0);
	if(.@var == 0){
		set_instance_var("gh_fall",1);
		instance_enable(strnpcinfo(0),false);
		instance_hide("Varmundt#gh_fall_0",false);
		specialeffect EF_YELLOBODY,AREA,instance_npcname("Varmundt#gh_fall_0");
		specialeffect EF_WHITEBODY,AREA,instance_npcname("Varmundt#gh_fall_0");
	}
end;
}

1@gl_prq,299,30,4	script	Varmundt#gh_fall_0	4_M_BARMUND,5,4,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 2){
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "Please go back. This place is changing into a place where living beings cannot enter.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "Sage. What happened here?";
		mes "I cancelled all my plans to come back here as soon as possible. Where is Heinrich?";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "As you can see, the curse of Himmelmez is spreading throughout the castle. It's extremely dangerous to go inside now.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "However, there are many people who have not evacuated yet.";
		mes "Shouldn't we focus on that first?";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "Lord Heinrich has already started.";
		mes "What you should do now is remedy the situation after the evacuation, not recklessly dive into the castle.";
		next;
		cutin "schmidt03",0;
		mes "[ Schmidt ]";
		mes "You're right, but there are other reasons I want to go inside the castle.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "I know a few things about this type of curse. Perhaps the catalyst is within the castle. If we don't destroy it...";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "You mentioned the curse, but not the reason of it.";
		next;
		cutin "schmidt06",0;
		mes "[ Schmidt ]";
		mes "What is then? Do you know the energy source?!";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "I'll explain more about it later.";
		mes "Wouldn't you want to wait for me to be physically here?";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "You know it's not like that.";
		mes "I am the king of this continent and the owner of this castle. It's my duty to amend the current situation.";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "I know better than anyone, young king.";
		mes "I know that there is no lie nor hesitation in your heart.";
		next;
		mes "[ Varmundt's Illusion ]";
		mes "But your life is not only yours, it's the life of this continent itself, please go back.";
		next;
		cutin "schmidt03",0;
		mes "[ Schmidt ]";
		mes "Sage... I'm sorry, but I have to go.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "Can you please tell me more about the situation?";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "I want to tell you a little bit more, but since I'm pursuing Himmelmez, I can't afford to tell you right now.";
		next;
		mes "[ Varmundt's Illusion ]";
		mes "I'll tell you one thing is for certain. If you go back now, you can have faith in my abilities and be at rest.";
		next;
		mes "[ Varmundt's Illusion ]";
		mes "Of course, I cannot guarantee the lives of everyone in the castle, that cannot be helped.";
		next;
		cutin "schmidt06",0;
		mes "[ Schmidt ]";
		mes "My subordinates are my people. I want everyone to survive. I won't hear that story again!";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "I am going.";
		next;
		cutin "gl_barmund3",2;
		mes "[ Varmundt's Illusion ]";
		mes "Wait, young king! You must not go carelessly like that!";
		close2;
		cutin "",255;
		.@var = get_instance_var("gh_fall");
		if(.@var == 2){
			set_instance_var("gh_fall",3);
			.@mercenary = getmercinfo(8);
			if(.@mercenary != 0){
				if(unitexists(.@mercenary))
					unitkill .@mercenary;
			}
			instance_hide("Schmidt#gh_fall_0",true);
			instance_enable("Schmidt#gh_fall_0",false);
			instance_hide("Oscar#gh_fall_0",false);
			mercenary_create 20385,300000;
		}
		end;
	}
	if(.@var == 3){
		cutin "gl_barmund3",2;
		mes "[ Varmundt's Illusion ]";
		mes "Wait, young king! You must not go carelessly like that!";
		close3;
	}
	end;
	
OnTouch:
	specialeffect EF_YELLOBODY;
	.@var = get_instance_var("gh_fall");
	if(.@var == 1){
		set_instance_var("gh_fall",2);
		instance_hide("Schmidt#gh_fall_0",false);
		npctalk "Stop right there, Schmidt. This is not a place for you to simply jump in at any moment.";	
	}
end;
}

1@gl_prq,299,25,0	script	Schmidt#gh_fall_0	4_ED_SCHMIDT,{
	cutin "schmidt02",0;
	mes "[ Schmidt ]";
	mes "I'm talking to the sage. Please wait a moment...";
	close3;
}

1@gl_prq,280,32,3	script	Oscar#gh_fall_0	4_ED_OSCAR,3,3,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 3){
		cutin "oscar01",0;
		mes "[ Oscar ]";
		mes "What if someone sees me?";
		mes "Don't worry. My body has a dimensional barrier so that only you can see me.";
		next;
		mes "[ Oscar ]";
		mes "How about you? Do you think there's a way to change the course of history and bring these people out of a tragedy?";
		next;
		cutin "oscar05",0;
		mes "[ Oscar ]";
		mes "After all, the fate of these people are drastic.";
		next;
		mes "[ Oscar ]";
		mes "It's a future without dreams nor hope. A painful, distorted story.";
		mes "Wherever you look, there's not a glimpse of hope.";
		next;
		mes "[ Oscar ]";
		mes "Of course, what I really wanted is a happy ending, so I actually tried to change my own destiny a few times.";
		next;
		cutin "oscar01",0;
		mes "[ Oscar ]";
		mes "What I said was a joke?!";
		mes "You never take me too seriously aren't you?";
		next;
		cutin "oscar02",0;
		mes "[ Oscar ]";
		mes "Anyway. In my opinion, King Schmidt's military decision was left to be desired...";
		next;
		mes "[ Oscar ]";
		mes "It was tactically correct, but it failed strategically.";
		next;
		mes "[ Oscar ]";
		mes "You'll see it yourself anyway, but that's why Varmundt tried to prevent him from entering.";
		next;
		cutin "oscar05",0;
		mes "[ Oscar ]";
		mes "Poor Schmidt...";
		next;
		mes "[ Oscar ]";
		mes "I always think about how different the story would have been if he had been a little more selfish.";
		next;
		cutin "oscar01",0;
		mes "[ Oscar ]";
		mes "I'm moving. There are still events to witness!";
		close2;
		cutin "",255;
		set_instance_var("gh_fall",4);
		instance_enable("#gh_fall_ev_1",true);
		for(.@i = 0; .@i < 16; .@i++)
			instance_enable("Victim#gh_fall_" + .@i,true);	
		end;
	}
	cutin "oscar01",0;
	mes "[ Oscar ]";
	mes "I'm moving. There are still events to witness!";
	close3;
	
OnTouch:
	.@var = get_instance_var("gh_fall");
	if(.@var == 3)
		npctalk "Oh~ Did you have a good conversation with Varmundt?";
	if(.@var == 4)
		npctalk "Go ahead. It shouldn't take much to see the following events.";
end;
}

1@gl_prq,273,49,0	script	#gh_fall_ev_1	HIDDEN_WARP_NPC,4,3,{
	end;
	
OnTouch:
	.@var = get_instance_var("gh_fall");
	if(.@var == 4){
		set_instance_var("gh_fall",5);
		instance_hide(strnpcinfo(0),true);
		instance_enable(strnpcinfo(0),false);
		.@mercenary = getmercinfo(8);
		if(.@mercenary != 0){
			if(unitexists(.@mercenary))
				unitkill .@mercenary;
		}
		instance_enable("Schmidt#gh_fall_1",true);
		instance_hide("Schmidt#gh_fall_1",false);
		npctalk "Wait. Aren't my soldiers in the front?",instance_npcname("Schmidt#gh_fall_1");
	}
end;
}

1@gl_prq,273,51,7	script	Schmidt#gh_fall_1	4_ED_SCHMIDT,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 5){
		cutin "schmidt02",2;
		mes "[ Schmidt ]";
		mes "There seems to be people who haven't evacuated yet. I have to tell them to return outside.";
		npctalk "Schmidt : You took your time. Aren't my soldiers the one ahead of me?";
		next;
		select("Please go back now.:You have orders.");
		mes "[ Schmidt ]";
		mes "No I can't. I won't go back...";
		next;
		mes "[ Schmidt ]";
		mes "I'm sorry, but I need you to help me. I need your help on finding the rest of the survivors.";
		next;
		mes "[ Schmidt ]";
		mes "There's a chance that they'll attack you if you're not with me. I think they've lost already half their sanity...";
		next;
		mes "[ Schmidt ]";
		mes "I'll met you again when our path crosses.";
		close2;
		cutin "",255;
		set_instance_var("gh_fall",6);
		.@mercenary = getmercinfo(8);
		if(.@mercenary != 0){
			if(unitexists(.@mercenary))
				unitkill .@mercenary;
		}
		instance_hide(strnpcinfo(0),true);
		instance_enable(strnpcinfo(0),true);
		instance_enable("#edda_fence_0",true);
		mercenary_create 20385,300000;
		for(.@i = 0; .@i < 8; .@i++)
			instance_enable("Knight Survivor#gh_fall_" + .@i,true);
	}
	end;
}

1@gl_prq,283,54,3	script	Victim#gh_fall_0	4_TOWER_06,5,5,{
	end;
	
OnTouch:
	specialeffect EF_VENOMDUST;
	instance_hide(strnpcinfo(0),true);
	instance_enable(strnpcinfo(0),false);
	getmapxy(.@map$,.@x,.@y,BL_NPC);
	monster .@map$,.@x,.@y,"Maggot",2467,1;
end;
}

1@gl_prq,287,78,4	duplicate(Victim#gh_fall_0)	Victim#gh_fall_1	4_TOWER_08,5,5
1@gl_prq,294,94,2	duplicate(Victim#gh_fall_0)	Victim#gh_fall_2	4_TOWER_11,5,5
1@gl_prq,291,132,5	duplicate(Victim#gh_fall_0)	Victim#gh_fall_3	4_TOWER_13,5,5
1@gl_prq,295,149,4	duplicate(Victim#gh_fall_0)	Victim#gh_fall_4	4_TOWER_03,5,5
1@gl_prq,283,152,2	duplicate(Victim#gh_fall_0)	Victim#gh_fall_5	4_TOWER_01,5,5
1@gl_prq,278,146,3	duplicate(Victim#gh_fall_0)	Victim#gh_fall_6	4_TOWER_06,5,5
1@gl_prq,289,167,6	duplicate(Victim#gh_fall_0)	Victim#gh_fall_7	4_TOWER_08,5,5
1@gl_prq,284,176,8	duplicate(Victim#gh_fall_0)	Victim#gh_fall_8	4_TOWER_11,5,5
1@gl_prq,287,201,7	duplicate(Victim#gh_fall_0)	Victim#gh_fall_9	4_TOWER_13,5,5
1@gl_prq,297,209,1	duplicate(Victim#gh_fall_0)	Victim#gh_fall_10	4_TOWER_03,5,5
1@gl_prq,280,210,5	duplicate(Victim#gh_fall_0)	Victim#gh_fall_11	4_TOWER_01,5,5
1@gl_prq,299,267,7	duplicate(Victim#gh_fall_0)	Victim#gh_fall_12	4_TOWER_06,5,5
1@gl_prq,292,283,8	duplicate(Victim#gh_fall_0)	Victim#gh_fall_13	4_TOWER_08,5,5
1@gl_prq,285,294,8	duplicate(Victim#gh_fall_0)	Victim#gh_fall_14	4_TOWER_11,5,5
1@gl_prq,297,297,8	duplicate(Victim#gh_fall_0)	Victim#gh_fall_15	4_TOWER_13,5,5

1@gl_prq,290,51,7	script	Knight Survivor#gh_fall_0	4_F_KHALITZBURG,3,3,{
	end;
	
OnTouch:
	.@var = get_instance_var("gh_fall");
	if(.@var == 6){
		.@id = atoi(replacestr(strnpcinfo(2),"gh_fall_",""));
		unittalk getcharid(3),strcharinfo(0) + " : At the king's order. Please evacuate quickly.";
		setpcblock PCBLOCK_ALL,true;
		switch(.@id){
			case 0:
				.@mes$ = "Avoid the castle? Oh my king, you must not go inside. Please, avoid it.";
				break;
				
			case 1:
				.@mes$ = "A strange orb in the castle is sucking the life out of our people. All the people who came near it are turning undead.";
				break;
				
			case 2:
				.@mes$ = "The living are turning into undead. You have to avoid it. The knights are blocking the outer area, but it's the limit.";
				break;
				
			case 3:
				.@mes$ = "Ugh... I can feel my soul being eroded by the sound of the orb. Stay away from me.";
				break;
				
			case 4:
				.@mes$ = "It's not true that the knights inside the castle turned into monsters, right? I'm heading out.";
				break;
				
			case 5:
				.@mes$ = "Don't go in further, your highness! It's dangerous! Please come with me!";
				break;
				
			case 6:
				.@mes$ = "Leave me alone! Don't come near me!";
				break;
				
			case 7:
				.@mes$ = "I only take orders from his majesty, Schmidt!";
				break;
		}
		npctalk .@mes$;
		sleep2 3000;
		setpcblock PCBLOCK_ALL,false;
		instance_hide(strnpcinfo(0),true);
		instance_enable(strnpcinfo(0),false);
		set_instance_var("survivor",get_instance_var("survivor") + 1);
		if(.@id >= 6){
			getmapxy(.@map$,.@x,.@y,BL_NPC);
			monster .@map$,.@x,.@y,"Unreasonable Person",20388,1,instance_npcname(strnpcinfo(0)) + "::OnMobDead";
		}
		end;
	}
	
OnMobDead:
end;
}

1@gl_prq,295,102,5	duplicate(Knight Survivor#gh_fall_0)	Knight Survivor#gh_fall_1	4_F_KHALITZBURG,3,3
1@gl_prq,295,134,3	duplicate(Knight Survivor#gh_fall_0)	Knight Survivor#gh_fall_2	4_F_KHALITZBURG,3,3
1@gl_prq,287,156,4	duplicate(Knight Survivor#gh_fall_0)	Knight Survivor#gh_fall_3	4_F_KHALITZBURG,3,3
1@gl_prq,279,160,7	duplicate(Knight Survivor#gh_fall_0)	Knight Survivor#gh_fall_4	4_F_KHALITZBURG,3,3
1@gl_prq,278,165,6	duplicate(Knight Survivor#gh_fall_0)	Knight Survivor#gh_fall_5	4_F_KHALITZBURG,3,3
1@gl_prq,299,179,1	duplicate(Knight Survivor#gh_fall_0)	Knight Survivor#gh_fall_6	4_F_KHALITZBURG,3,3
1@gl_prq,300,193,2	duplicate(Knight Survivor#gh_fall_0)	Knight Survivor#gh_fall_7	4_F_KHALITZBURG,3,3

1@gl_prq,284,217,4	script	#edda_fence_0	4_ED_FENCE,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 6){
		if(get_instance_var("survivor") < 8){
			cutin "schmidt02",2;
			mes "[ Schmidt ]";
			mes "Adventurer. There seems to be more survivors nearby. Shouldn't we find them first?";
			close3;
		}
		cutin "schmidt02",2;
		mes "[ Schmidt ]";
		mes "The door is already worn out, so we'd better just break it down. Stand back a little, I'll take care of it.";
		close2;
		cutin "",255;
		set_instance_var("gh_fall",7);
		specialeffect EF_LORD;
		sleep2 3000;
		instance_hide(strnpcinfo(0),true);
		instance_enable("#edda_warp_0",true);
		mercenary_create 20385,300000;
	}
	end;
}

1@gl_prq,284,217,4	script	#edda_warp_0	WARPNPC,3,3,{
	end;
	
OnTouch:
	.@map$ = get_instance_var("map$");
	warp .@map$,284,229;
end;
}

1@gl_prq,286,247,7	script	Schmidt#gh_fall_2	4_ED_SCHMIDT,3,3,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 8){
		cutin "schmidt02",2;
		mes "[ Schmidt ]";
		mes "My vitality is slowly fading away as we get close. While we are standing here, this sinister energy is draining everyone's life and sanity away.";
		next;
		select("You must return now.");
		mes "[ Schmidt ]";
		mes "Going through the underground passage is difficult and the corridors above already collapsed. We don't have much time left.";
		next;
		mes "[ Schmidt ]";
		mes "Let's continue, and hurry as much as possible.";
		close2;
		cutin "",255;
		set_instance_var("gh_fall",9);
		instance_hide(strnpcinfo(0),true);
		mercenary_create 20385,300000;
		end;
	}
	end;
	
OnTouch:
	.@var = get_instance_var("gh_fall");
	if(.@var == 7){
		.@mercenary = getmercinfo(8);
		if(.@mercenary != 0){
			if(unitexists(.@mercenary))
				unitkill .@mercenary;
		}
		set_instance_var("gh_fall",8);
		instance_hide(strnpcinfo(0),false);
		npctalk "Schmidt : I have a bad feeling about this.";
	}
end;
}

1@gl_prq,297,294,5	script	Schmidt#gh_fall_3	4_ED_SCHMIDT,3,3,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 12){
		cutin "schmidt04",0;
		mes "[ Schmidt ]";
		mes "Heinrich! Tell me where the orb is.";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "King Schmidt! Now is not the time for this.";
		next;
		cutin "schmidt01",0;
		mes "[ Schmidt ]";
		mes "Master is the only sage in this continent. I'm worried about his standing.";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "But you're the king! The only king in this continent!";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "The king can be suceeded by someone else, but nobody can replace the sage, not until many centuries.";
		next;
		cutin "schmidt06",0;
		mes "[ Schmidt ]";
		mes "Can you imagine what will happen to the state of the continent if the sage suddenly disappeared?";
		mes "Who will bear the threats to this world and the threats from other dimensions?";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "This is my decision... guide me to where the sage went into, Heinrich. I'm sure you know the location.";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "It can't be helped.";
		next;
		cutin "heinrich_a02",2;
		mes "[ Heinrich ]";
		mes "I'll obey... your orders.";
		close2;
		cutin "",255;
		specialeffect EF_REDBODY;
		specialeffect EF_CLOAKING;
		specialeffect EF_REDBODY,AREA,instance_npcname("Heinrich#gh_fall_0");
		specialeffect EF_CLOAKING,AREA,instance_npcname("Heinrich#gh_fall_0");
		for(.@i = 0; .@i < 6; .@i++){
			instance_hide("#gh_fall_guard_" + .@i,true);
			instance_enable("#gh_fall_guard_" + .@i,false);
		}
		sleep 200;
		instance_hide("Oscar#gh_fall_1",false);
		sleep 1000;
		set_instance_var("gh_fall",13);
		npctalk "Oscar : The story hasn't changed this time either.",instance_npcname("Oscar#gh_fall_1");
		end;
	}
	if(.@var == 13){
		mes "^0000CDIt seems that I'm not in sync with this time and space. Starting a conversation won't work.";
		end;
	}
	end;
	
OnTouch:
	.@var = get_instance_var("gh_fall");
	if(.@var == 9){
		.@mercenary = getmercinfo(8);
		if(.@mercenary != 0){
			if(unitexists(.@mercenary))
				unitkill .@mercenary;
		}
		for(.@i = 0; .@i < 6; .@i++)
			instance_hide("#gh_fall_guard_" + .@i,false);
		instance_hide("Heinrich#gh_fall_0",false);
		instance_hide("Schmidt#gh_fall_3",false);
		set_instance_var("gh_fall",10);
		npctalk "Schmidt : Heinrich!",instance_npcname("Schmidt#gh_fall_3");
	}
end;
}

1@gl_prq,301,294,3	script	Heinrich#gh_fall_0	4_M_HEINRICH,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 10){
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "King Schmidt! I see that you've arrived.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "Heinrich, I'm glad that you're safe!";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "I told the soldiers to take you outside when they meet you, but they didn't disobey my orders. You must be stubborn in order to come here your majesty...";
		next;
		cutin "schmidt05",0;
		mes "[ Schmidt ]";
		mes "Oh, it's been a while since we've last met and you're nagging me already Heinrich. It's nice to see you again.";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "King Schmidt, there's no time to spare. You must leave this place with me at once.";
		next;
		cutin "schmidt01",0;
		mes "[ Schmidt ]";
		mes "You're the same as the great sage, don't tell me to leave quickly. What is in the castle that's causing all of this?";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "...";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "Now, tell me. Heinrich, what's making you make me go as far as possible.";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "The cursed orb was located inside the castle, it's presumed to be a work of Himmelmez. I couldn't figure it out, but it's energy is getting stronger.";
		next;
		cutin "schmidt04",0;
		mes "[ Schmidt ]";
		mes "Right. I don't understand why you're asking me to evacuate.";
		next;
		cutin "schmidt01",0;
		mes "[ Schmidt ]";
		mes "But if the orb is the problem, I don't think it will be resolve by me evacuating, don't you think?";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt's Illusion ]";
		mes "Correct. It's meaningless to run away. It's a cursed object that endlessly drains the energy of the living.";
		close2;
		cutin "",255;
		instance_hide("Varmundt#gh_fall_1",false);
		npctalk "Heinrich : Varmundt!";
		npctalk "Schmidt : Master!",instance_npcname("Schmidt#gh_fall_3");
		set_instance_var("gh_fall",11);
		end;
	}
	if(.@var < 13){
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "(We have to evacuate Schmidt no matter what.)";
		close3;
	}
	mes "^0000CDIt seems that I'm not in sync with this time and space. Starting a conversation won't work.";
	end;
}

1@gl_prq,299,297,4	script	Varmundt#gh_fall_1	4_M_BARMUND,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 11){
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "If I had chased a little more, I'd have caught up with him. I was worried about our king, I couldn't help but coming here myself.";
		next;
		cutin "schmidt04",0;
		mes "[ Schmidt ]";
		mes "Master...";
		next;
		cutin "gl_barmund3",2;
		mes "[ Varmundt ]";
		mes "What the hell are you doing Heinrich! I ordered you to ensure that the King wouldn't come here.";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "I'll evacuate Schmidt right away!";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "That's what I thought. I'll take care of the orb, so evacuate the castle immediately.";
		close2;
		cutin "",255;
		instance_hide("Varmundt#gh_fall_1",true);
		instance_enable("Varmundt#gh_fall_1",false);
		set_instance_var("gh_fall",12);
		end;
	}
	end;
}

1@gl_prq,299,297,4	script	Oscar#gh_fall_1	4_ED_OSCAR,{
	.@map$ = get_instance_var("map$");
	.@var = get_instance_var("gh_fall");
	if(.@var == 13){
		cutin "OSCAR01",2;
		mes "[ Oscar ]";
		mes "I think I've seen this happen about 824000 times.";
		next;
		mes "[ Oscar ]";
		mes "Sometimes I fall into another area, but the dimensions that you are able to see now are nothing but history.";
		next;
		cutin "OSCAR03",2;
		mes "[ Oscar ]";
		mes "For the time being, we must break through the dimensions and observe Schmidt's story. The timeline from now on is very dangerous.";
		next;
		mes "[ Oscar ]";
		mes "Schmidt nor Heinrich probably won't be able to detect you, even if Heinrich is a master of magic reconstruction.";
		next;
		mes "[ Oscar ]";
		mes "So, are you going to follow Schmidt and see what happens next? If you are not interested, you can stop here.";
		next;
		if(select("Hang on.:I'll go") == 1){
			mes "[ Oscar ]";
			mes "Well, that's your decision. People's interest are different I guess.";
			close3;
		}
		cutin "OSCAR01",2;
		mes "[ Oscar ]";
		mes "Good decision. Once you see it, you'll want to witness it until the end.";
		close2;
		cutin "",255;
		instance_enable("#gh_fall_ev_2",true);
		instance_enable("Orb#edda_fall_0",true);
		instance_enable("Varmundt#gh_fall_2",true);
		instance_hide("Varmundt#gh_fall_2",false);
		set_instance_var("gh_fall",14);
		warp .@map$,52,275;
	}
	end;
}

1@gl_prq,52,275,0	script	#gh_fall_ev_2	HIDDEN_WARP_NPC,1,1,{
	end;
	
OnTouch:
	.@var = get_instance_var("gh_fall");
	if(.@var == 14){
		.@v$ = instance_npcname("Varmundt#gh_fall_2");
		.@o$ = instance_npcname("Orb#edda_fall_0");
		.@s$ = instance_npcname("Schmidt#gh_fall_4");
		.@h$ = instance_npcname("Heinrich#gh_fall_1");
		instance_hide(strnpcinfo(0),true);
		instance_enable(strnpcinfo(0),false);
		sleep 1500;
		npctalk "Varmundt : Please, break this time!",.@v$;
		instance_effect(EF_LIGHTSPHERE_SUN,.@v$);
		sleep 3000;
		instance_effect(EF_TETRA,.@v$);
		sleep 3000;
		instance_effect(EF_TETRA,.@v$);
		sleep 2000;
		instance_effect(EF_TETRA,.@v$);
		sleep 2000;
		instance_effect(EF_TETRACASTING,.@v$);
		sleep 2000;
		instance_effect(EF_DQ9_CHARGE4,.@o$);
		sleep 2000;
		npctalk "Varmundt : Hmmm! Just a little more!",.@v$;
		sleep 3000;
		instance_effect(EF_PRIMECHARGE2,.@o$);
		sleep 3100;
		instance_effect(EF_BOTTOM_BASILICA2,.@s$);
		instance_effect(EF_BOTTOM_BASILICA2,.@h$);
		sleep 3000;
		npctalk "Varmundt : I failed.",.@v$;
		sleep 200;
		instance_effect(EF_THUNDERSTORM2,.@s$);
		instance_effect(EF_THUNDERSTORM2,.@h$);
		instance_hide("Varmundt#gh_fall_2",true);
		instance_hide("Varmundt#gh_fall_3",false);
		instance_hide("Schmidt#gh_fall_4",false);
		instance_hide("Heinrich#gh_fall_1",false);
		.@v$ = instance_npcname("Varmundt#gh_fall_3");
		npctalk "Varmundt : Huh?!",.@v$;
		sleep 2000;
		npctalk "Schmidt : Master, why are you trying to do this by yourself!",.@s$;
		sleep 2000;
		npctalk "Heinrich : Varmundt!",.@h$;
		sleep 3100;
		npctalk "Varmundt : Young king, you really are stubborn.",.@v$;
		sleep 5500;
		npctalk "Varmundt : Why don't you spend your time for an afternoon tea, rather than being here?",.@v$;
		sleep 5500;
		npctalk "Schmidt : That can wait, shouldn't we be more worried about that thing before our eyes?",.@s$;
		sleep 5500;
		npctalk "Varmundt : Sigh... young king. The biggest problem for me right now is that you are still here.",.@v$;
		sleep 3000;
		npctalk "Schmidt : Master!",.@s$;
		sleep 3000;
		npctalk "Varmundt : Return now. You will be helping me by going as far as possible from here.",.@v$;
		sleep 5500;
		npctalk "Schmidt : Master! If we join our with me, we can easily destroy that object!",.@s$;
		sleep 5000;
		npctalk "Varmundt : Of course. Did you really think that your master wouldn't know the way to break this object.",.@v$;
		sleep 3000;
		npctalk "Schmidt : ...",.@s$;
		sleep 3000;
		npctalk "Varmundt : Let me tell you why I wanted to keep you away from the castle. Lay down your hands first, I think I'll be able to destroy this orb this time.",.@v$;
		sleep 5500;
		npctalk "Heinrich : King Schmidt. Follow the words of the sage. This orb looks very dangerous!",.@h$;
		instance_hide("Varmundt#gh_fall_3",true);
		instance_hide("Varmundt#gh_fall_2",false);
		.@v$ = instance_npcname("Varmundt#gh_fall_2");
		sleep 3000;
		set_instance_var("gh_fall",15);
		instance_effect(EF_MARKING_USE_CHANGEMONSTER,.@v$);
		end;
	}
end;
}


1@gl_prq,55,281,3	script	Varmundt#gh_fall_2	4_M_BARMUND,{
	.@var = get_instance_var("gh_fall");
	'party_id = getcharid(1);
	if(.@var == 15){
		.@map$ = get_instance_var("map$");
		.@o$ = instance_npcname("Orb#edda_fall_0");
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "Listen to me, young king. Judging by my abilities to read mana, this orb has no weakness.";
		instance_effect(EF_DQ9_CHARGE4,.@o$);
		next;
		mes "[ Varmundt ]";
		mes "This object is constantly absorbing all the life around us and expanding its range.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "I guess that's why living creatures nearby are slowly turning undead.";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "The rate of absorption is slow, but the amount of life that it can absorb has no limit. It act as a channel to another dimension where the energy is being absorbed in.";
		next;
		cutin "schmidt07",0;
		mes "[ Schmidt ]";
		mes "Another dimension?! Does it just absorb life and simply throw it away into another dimension?";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "You can feel the malice of the creator of this Orb. The only way to destroy it is to overload it with a powerful life energy.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "Does it mean...";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "Someone with a strong life energy should pour all of his energy into the orb in a short time.";
		next;
		mes "[ Varmundt ]";
		mes "That is none other than me, Varmundt.";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "Don't say that, Varmundt!";
		instance_effect(EF_DQ9_CHARGE4,.@o$);
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "There's no other way, now you know the reason, Schmidt. The reason why I told you to go as far as possible.";
		next;
		mes "[ Varmundt ]";
		mes "You would surely figure out how to destroy this orb even if I hadn't taught you about it. And surely you would have given up your own life.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "Maybe so.";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "Your strength is the pinnacle of the Royal Guards, your life force is enough to destroy this orb. But you shouldn't die.";
		next;
		mes "[ Varmundt ]";
		mes "You usually don't listen to me, but I want you to listen to me this time.";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "How are you going to handle this master?";
		next;
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "I'm going to replace my life with mana and pour it into the orb. If you can concentrate on the orb for just a moment, it might be possible to destroy it.";
		next;
		cutin "schmidt06",0;
		mes "[ Schmidt ]";
		mes "Master! Doesn't that mean that your death will come soon?";
		next;
		cutin "gl_barmund1",2;
		mes "[ Varmundt ]";
		mes "Is there another way?";
		instance_effect(EF_DQ9_CHARGE4,.@o$);
		next;
		cutin "schmidt06",0;
		mes "[ Schmidt ]";
		mes "But master, you're the only sage in this continent. Have you thought about the chaos that will occur if you died?";
		next;
		cutin "gl_barmund1",2;
		mes "[ Varmundt ]";
		mes "......";
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "There will be some confusion if the king went missing. However, the ordinance determines who will act on behalf and who will succeed to the throne.";
		next;
		cutin "schmidt03",0;
		mes "[ Schmidt ]";
		mes "But will there be someone to replace you? I'm sure such person won't appear in a few centuries.";
		next;
		cutin "gl_barmund3",2;
		mes "[ Varmundt ]";
		mes "You're foolish!";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "Schmidt, please calm down for a bit. Varmundt, would you listen to me?";
		next;
		mes "[ Heinrich ]";
		mes "In my opinion, both of you are indispensable to this continent and the kingdom. Even only one of you is absent, it'll be a disaster.";
		next;
		mes "[ Heinrich ]";
		mes "How about this instead? I know I'm not... as powerful as either one of you, but if it's my life, it's probably enough to destroy this orb.";
		next;
		mes "[ Heinrich ]";
		mes "I think it's the most reasonable choice right now.";
		next;
		cutin "schmidt06",0;
		mes "[ Schmidt ]";
		mes "Not a chance! I'll feel guilty for the rest of my life, Heinrich!";
		next;
		mes "[ Schmidt ]";
		mes "You are a servant of this kingdom. You are my friend. I have to protect you as king.";
		instance_effect(EF_CLOUD4,.@o$);
		next;
		cutin "schmidt02",0;
		mes "[ Schmidt ]";
		mes "I will be the one to break the orb. It is the duty of the king to protect its people. Is there any objection here?";
		next;
		cutin "gl_barmund3",2;
		mes "[ Varmundt ]";
		mes "You're too reckless, Heinrich! And don't be too an idiot, Schmidt! What kind of a teacher would I be?";
		next;
		mes "[ Varmundt ]";
		mes "Would you still be able to call me your teacher if I let my disciples die for my sake?!";
		next;
		cutin "schmidt01",0;
		mes "[ Schmidt ]";
		mes "Master. While we are arguing here, the orb is expanding.";
		instance_effect(EF_DQ9_CHARGE4,.@o$);
		next;
		cutin "schmidt03",0;
		mes "[ Schmidt ]";
		mes "I don't want to delay this much longer. I will go on ahead first. Goodbye Heinrich, Varmundt!";
		next;
		cutin "heinrich02",2;
		mes "[ Heinrich ]";
		mes "Stop it, Schmidt. Stop!!";
		next;
		cutin "gl_barmund3",2;
		mes "[ Varmundt ]";
		mes "Don't do it! Schmidt!";
		close2;
		cutin "",255;
		set_instance_var("gh_fall",16);
		.@map$ = get_instance_var("map$");
		warp .@map$,55,280;
		instance_event("Orb#edda_fall_0","OnEvent",false);
		end;
	}
	if(.@var == 17){
		.@s$ = instance_npcname("#schmidt_dummy_5");
		.@o$ = instance_npcname(strnpcinfo(0));
		.@h$ = instance_npcname("Heinrich#gh_fall_1");
		.@v$ = instance_npcname("Varmundt#gh_fall_2");
		instance_effect(EF_CLOUD4,.@o$);
		instance_effect(EF_SCREEN_QUAKE,.@o$);
		cutin "schmidt07",0;
		mes "[ Schmidt ]";
		mes "Ugh!!! Ahhhhhh!!!!";
		next;
		cutin "gl_barmund3",2;
		mes "[ Varmundt ]";
		mes "Schmidt! Wait, I'll do whatever I can!";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);
		cutin "schmidt07",0;
		mes "[ Schmidt ]";
		mes "Ugh... No! Don't come near me!";
		next;
		cutin "HEINRICH_A02",2;
		mes "[ Heinrich ]";
		mes "Schmidt!";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);
		cutin "schmidt07",0;
		mes "[ Schmidt ]";
		mes "Same for you Heinrich! Don't come near me!";
		next;
		mes "[ Schmidt ]";
		mes "It's breaking! If I hadn't done this, we wouldn't have made it in time!";
		next;
		instance_effect(EF_CLOUD4,.@o$);
		instance_effect(EF_SCREEN_QUAKE,.@o$);	
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "My foolish disciple... Why. Why are you...";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);
		cutin "schmidt07",0;
		mes "[ Schmidt ]";
		mes "Ha... haha... Master. You are wise, I'm sure you already know the answer. What should I do now...";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);
		mes "[ Schmidt ]";
		mes "Soon, the orb will be destroyed, if you stay here, you will be caught up with the mass energy and be destroyed with it.";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);
		mes "[ Schmidt ]";
		mes "Hurry... Go...";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "I'll cast a spell to nullify your pain, you won't feel anything. That's the last thing I can do for you as your teacher.";
		instance_effect(EF_CLOUD4,.@o$);
		instance_effect(EF_SCREEN_QUAKE,.@o$);	
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);	
		cutin "schmidt07",0;
		mes "[ Schmidt ]";
		mes "Heinrich!! This is my final command! Escape with Varmundt immediately and get out the castle!";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);	
		cutin "HEINRICH_A02",2;
		mes "[ Heinrich ]";
		mes "I, Heinrich...";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);	
		cutin "HEINRICH_A03",2;
		mes "[ Heinrich ]";
		mes "Accept your comand.";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);	
		cutin "gl_barmund2",2;
		mes "[ Varmundt ]";
		mes "Schmidt, I...";
		next;
		instance_effect(EF_SCREEN_QUAKE,.@o$);	
		mes "[ Varmundt ]";
		mes "Will always remember you, as the disciple who hasn't listened to his teacher throughout his life.";
		next;
		instance_effect(EF_CLOUD4,.@o$);
		cutin "schmidt04",0;
		mes "[ Schmidt ]";
		mes "Thank you for remembering. Then, this is goodbye, Master!";
		close2;
		cutin "",255;
		set_instance_var("gh_fall",18);
		instance_event("Orb#edda_fall_0","OnEvent02",false);
		end;
	}
	end;
}

1@gl_prq,55,281,3	duplicate(dummynpc2)	Varmundt#gh_fall_3	4_M_BARMUND
1@gl_prq,52,285,5	duplicate(dummynpc2)	Schmidt#gh_fall_4	4_ED_SCHMIDT
1@gl_prq,48,281,5	duplicate(dummynpc2)	Heinrich#gh_fall_1	4_M_HEINRICH

1@gl_prq,52,277,5	script	Orb#edda_fall_0	4_ED_ORB,{
	end;
	
OnEvent:
	sleep 1200;
	specialeffect EF_DQ9_CHARGE4;
	specialeffect EF_CLOUD4;
	instance_hide("Schmidt#gh_fall_4",true);
	instance_enable("Schmidt#gh_fall_4",false);
	for(.@i = 0; .@i < 44; .@i++)
		instance_hide("#schmidt_dummy_" + .@i,false);
	sleep 200;
	for(.@i = 0; .@i < 44; .@i++)
		instance_effect("#schmidt_dummy_" + .@i,EF_REDBODY);
	sleep 200;
	for(.@i = 0; .@i < 44; .@i++)
		instance_effect("#schmidt_dummy_" + .@i,EF_CLOAKING);
	sleep 300;
	for(.@i = 0; .@i < 44; .@i++){
		instance_hide("#schmidt_dummy_" + .@i,true);
		instance_enable("#schmidt_dummy_" + .@i,false);
	}
	sleep 2000;
	.@s$ = instance_npcname("#schmidt_dummy_5");
	.@o$ = instance_npcname(strnpcinfo(0));
	.@h$ = instance_npcname("Heinrich#gh_fall_1");
	.@v$ = instance_npcname("Varmundt#gh_fall_2");
	instance_enable("#schmidt_dummy_5",true);
	instance_effect(EF_WHITEBODY,.@s$);
	sleep 5500;
	instance_effect(EF_MAPCHAIN,.@o$);
	sleep 1000;
	npctalk "Stop it... No-!!",.@s$;
	instance_effect(EF_CLOUD4,.@o$);
	sleep 3000;
	instance_effect(EF_CLOUD4,.@o$);
	npctalk "Heinrich : Your majesty! How... How can this happen!",.@h$;
	sleep 5000;
	npctalk "Varmundt : Schmidt, you idiot!",.@v$;
	set_instance_var("gh_fall",17);
	instance_effect(EF_MARKING_USE_CHANGEMONSTER,.@v$);
end;

OnEvent02:
	.@o$ = instance_npcname(strnpcinfo(0));
	.@h$ = instance_npcname("Heinrich#gh_fall_1");
	.@v$ = instance_npcname("Varmundt#gh_fall_2");
	.@s$ = instance_npcname("#schmidt_dummy_5");
	.@map$ = get_instance_var("map$");
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_EVILS_PAW,.@o$);
	instance_effect(EF_CLOUD4,.@o$);
	mapannounce .@map$,"Schmidt's Cry : To all who remains in the castle! Run away by any means at once!",bc_map,0xFFFF00;
	sleep 2000;
	instance_effect(EF_CLOUD4,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	mapannounce .@map$,"Schmidt's Cry : This is your king!",bc_map,0xFFFF00;
	sleep 2000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_CLOUD4,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	npctalk "Heinrich : King Schmidt! I'll be with you soon... Let's go, Varmunt.",.@h$;
	sleep 2000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	instance_effect(EF_ENHANCE,.@v$);
	sleep 1000;
	instance_enable("Varmundt#gh_fall_2",false);
	sleep 1000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	instance_effect(EF_ENHANCE,.@h$);
	sleep 1000;
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	instance_enable("Heinrich#gh_fall_1",false);
	sleep 2000;
	npctalk "Then let's get this over with!",.@s$;
	sleep 2000;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	npctalk "Oh, I forgot to tell Heinrich to act as the new king...",.@s$;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	npctalk "Heinrich, I know you'll do well... I wish we could have atleast a last drink together.",.@s$;
	sleep 2000;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	sleep 2000;
	instance_effect(EF_LORD,.@o$);
	instance_effect(EF_SCREEN_QUAKE,.@o$);
	warpparty .@map$,53,263,'party_id;
	instance_enable("#schmidt_dummy_5",false);
	instance_enable("Orb#edda_fall_0",false);
	instance_hide("Oscar#gh_fall_2",false);
	sleep 2000;
	npctalk "Oscar : Woah. If I only missed even for a few second, It would have been too late.",instance_npcname("Oscar#gh_fall_2");
	set_instance_var("gh_fall",19);
end;
}

1@gl_prq,52,270,4	script	Oscar#gh_fall_2	4_ED_OSCAR,2,2,{
	.@var = get_instance_var("gh_fall");
	if(.@var == 19){
		cutin "OSCAR01",0;
		mes "[ Oscar ]";
		mes "He used all his energy to the dimensional orb and destroyed it, but the energy waves that it created were so huge, that it almost swept everyone nearby.";
		next;
		mes "[ Oscar ]";
		mes "Once you've seen the story, you'll feel their pain.";
		next;
		mes "[ Oscar ]";
		mes "^0000CDHere is some experience for joining me.^000000";
		getexp 600000,600000;
		if(isbegin_quest(12457) == 1)
			completequest 12457;
		next;
		mes "[ Oscar ]";
		mes "In the end, Heinrich didn't listen to Schmidt's final words.";
		next;
		mes "[ Oscar ]";
		mes "There was too much work to be done after the king passed(officially...) after the Fall of Glastheim.";
		next;
		mes "[ Oscar ]";
		mes "Well, let's take another look at it again next time, let's get out of here for now. I'm going to move fast because the dimensional channels are collapsing quickly.";
		close2;
		warp "glast_01",244,290;
		end;
	}
	end;
}


1@gl_prq,303,296,3	duplicate(dummynpc2)	#gh_fall_guard_0	4_F_KHALITZBURG
1@gl_prq,303,294,3	duplicate(dummynpc2)	#gh_fall_guard_1	4_F_KHALITZBURG
1@gl_prq,303,292,3	duplicate(dummynpc2)	#gh_fall_guard_2	4_F_KHALITZBURG
1@gl_prq,305,296,7	duplicate(dummynpc2)	#gh_fall_guard_3	4_F_KHALITZBURG
1@gl_prq,305,294,6	duplicate(dummynpc2)	#gh_fall_guard_4	4_F_KHALITZBURG
1@gl_prq,305,292,1	duplicate(dummynpc2)	#gh_fall_guard_5	4_F_KHALITZBURG

1@gl_prq,52,282,4	duplicate(dummynpc2)	#schmidt_dummy_0	4_ED_SCHMIDT
1@gl_prq,52,281,4	duplicate(dummynpc2)	#schmidt_dummy_1	4_ED_SCHMIDT
1@gl_prq,52,280,4	duplicate(dummynpc2)	#schmidt_dummy_2	4_ED_SCHMIDT
1@gl_prq,52,279,4	duplicate(dummynpc2)	#schmidt_dummy_3	4_ED_SCHMIDT
1@gl_prq,52,278,4	duplicate(dummynpc2)	#schmidt_dummy_4	4_ED_SCHMIDT
1@gl_prq,52,277,4	duplicate(dummynpc2)	#schmidt_dummy_5	4_ED_SCHMIDT
1@gl_prq,52,276,4	duplicate(dummynpc2)	#schmidt_dummy_6	4_ED_SCHMIDT
1@gl_prq,52,275,4	duplicate(dummynpc2)	#schmidt_dummy_7	4_ED_SCHMIDT
1@gl_prq,52,274,4	duplicate(dummynpc2)	#schmidt_dummy_8	4_ED_SCHMIDT
1@gl_prq,52,273,4	duplicate(dummynpc2)	#schmidt_dummy_9	4_ED_SCHMIDT
1@gl_prq,52,272,4	duplicate(dummynpc2)	#schmidt_dummy_10	4_ED_SCHMIDT
1@gl_prq,47,277,6	duplicate(dummynpc2)	#schmidt_dummy_11	4_ED_SCHMIDT
1@gl_prq,48,277,6	duplicate(dummynpc2)	#schmidt_dummy_12	4_ED_SCHMIDT
1@gl_prq,49,277,6	duplicate(dummynpc2)	#schmidt_dummy_13	4_ED_SCHMIDT
1@gl_prq,50,277,6	duplicate(dummynpc2)	#schmidt_dummy_14	4_ED_SCHMIDT
1@gl_prq,51,277,6	duplicate(dummynpc2)	#schmidt_dummy_15	4_ED_SCHMIDT
1@gl_prq,52,277,6	duplicate(dummynpc2)	#schmidt_dummy_16	4_ED_SCHMIDT
1@gl_prq,53,277,6	duplicate(dummynpc2)	#schmidt_dummy_17	4_ED_SCHMIDT
1@gl_prq,54,277,6	duplicate(dummynpc2)	#schmidt_dummy_18	4_ED_SCHMIDT
1@gl_prq,55,277,6	duplicate(dummynpc2)	#schmidt_dummy_19	4_ED_SCHMIDT
1@gl_prq,56,277,6	duplicate(dummynpc2)	#schmidt_dummy_20	4_ED_SCHMIDT
1@gl_prq,57,277,6	duplicate(dummynpc2)	#schmidt_dummy_21	4_ED_SCHMIDT
1@gl_prq,52,272,8	duplicate(dummynpc2)	#schmidt_dummy_22	4_ED_SCHMIDT
1@gl_prq,52,273,8	duplicate(dummynpc2)	#schmidt_dummy_23	4_ED_SCHMIDT
1@gl_prq,52,274,8	duplicate(dummynpc2)	#schmidt_dummy_24	4_ED_SCHMIDT
1@gl_prq,52,275,8	duplicate(dummynpc2)	#schmidt_dummy_25	4_ED_SCHMIDT
1@gl_prq,52,276,8	duplicate(dummynpc2)	#schmidt_dummy_26	4_ED_SCHMIDT
1@gl_prq,52,277,8	duplicate(dummynpc2)	#schmidt_dummy_27	4_ED_SCHMIDT
1@gl_prq,52,278,8	duplicate(dummynpc2)	#schmidt_dummy_28	4_ED_SCHMIDT
1@gl_prq,52,279,8	duplicate(dummynpc2)	#schmidt_dummy_29	4_ED_SCHMIDT
1@gl_prq,52,280,8	duplicate(dummynpc2)	#schmidt_dummy_30	4_ED_SCHMIDT
1@gl_prq,52,281,8	duplicate(dummynpc2)	#schmidt_dummy_31	4_ED_SCHMIDT
1@gl_prq,52,282,8	duplicate(dummynpc2)	#schmidt_dummy_32	4_ED_SCHMIDT
1@gl_prq,57,277,2	duplicate(dummynpc2)	#schmidt_dummy_33	4_ED_SCHMIDT
1@gl_prq,56,277,2	duplicate(dummynpc2)	#schmidt_dummy_34	4_ED_SCHMIDT
1@gl_prq,55,277,2	duplicate(dummynpc2)	#schmidt_dummy_35	4_ED_SCHMIDT
1@gl_prq,54,277,2	duplicate(dummynpc2)	#schmidt_dummy_36	4_ED_SCHMIDT
1@gl_prq,53,277,2	duplicate(dummynpc2)	#schmidt_dummy_37	4_ED_SCHMIDT
1@gl_prq,52,277,2	duplicate(dummynpc2)	#schmidt_dummy_38	4_ED_SCHMIDT
1@gl_prq,51,277,2	duplicate(dummynpc2)	#schmidt_dummy_39	4_ED_SCHMIDT
1@gl_prq,50,277,2	duplicate(dummynpc2)	#schmidt_dummy_40	4_ED_SCHMIDT
1@gl_prq,49,277,2	duplicate(dummynpc2)	#schmidt_dummy_41	4_ED_SCHMIDT
1@gl_prq,48,277,2	duplicate(dummynpc2)	#schmidt_dummy_42	4_ED_SCHMIDT
1@gl_prq,47,277,2	duplicate(dummynpc2)	#schmidt_dummy_43	4_ED_SCHMIDT

1@gl_prq,79,22,0	script	#gh_purification_ev_0	HIDDEN_WARP_NPC,2,2,{
	end;
	
OnTouch:
	set_instance_var("party_id",getcharid(1));
	instance_hide(strnpcinfo(0),true);
	instance_enable(strnpcinfo(0),false);
	.@map$ = get_instance_var("map$");
	instance_event("#gh_purification_mob","OnSpawn00",false);
	mapannounce .@map$,"Oscar : This place once used to be the kitchen.",bc_map,0xFF66FF;
	sleep 2800;
	mapannounce .@map$,"Oscar : Unfortunately... The maids who were preparing the meals were affected by the curse the most.",bc_map,0xFF66FF;
	sleep 2800;
	mapannounce .@map$,"Oscar : The only thing we can do for them is to free them from their pain and lay them to rest.",bc_map,0xFF66FF;
	sleep 2800;
	mapannounce .@map$,"Oscar : We will still have time for the next quarters, if you purify the maids here that have been cursed.",bc_map,0xFF66FF;
	sleep 2800;
	mapannounce .@map$,"Oscar : This dimension's time and space is distorted making it hard to move around.",bc_map,0xFF66FF;
	sleep 2800;
	mapannounce .@map$,"Oscar : Oh, I almost forgot to remind you, If you ever encountered a Flaming Thorn, try to avoid it. It's the most troublesome being in this dimension.",bc_map,0xFF66FF;
end;
}

1@gl_prq,127,63,4	script	Oscar#gh_purification_0	4_ED_OSCAR,3,3,{
	end;

OnTouch:
	.@var = get_instance_var("purification");
	.@map$ = get_instance_var("map$");
	if(.@var == 0){
		set_instance_var("purification",1);
		npctalk "Oscar : This placed used to be the Knight's workplace.";
		sleep 3000;
		instance_event("#gh_purification_mob","OnSpawn01",false);
		npctalk "Oscar : Those who were resting after their work wasn't able to evacuate in time.";
		sleep 3000;
		npctalk "Oscar : They went insane because of the curse, just like the maids who were preparing the meals.";
		sleep 3000;
		npctalk "Oscar : I'll be heading to the next area, please put the knights to rest.";
		sleep 1000;
		instance_enable(strnpcinfo(0),false);
	}
end;
}

1@gl_prq,148,121,4	script	Oscar#gh_purification_1	4_ED_OSCAR,3,3,{
	end;

OnTouch:
	.@var = get_instance_var("purification");
	.@map$ = get_instance_var("map$");
	if(.@var == 1){
		set_instance_var("purification",2);
		npctalk "Oscar : This place used to be a guest area, with a lounge over there.";
		sleep 3000;
		instance_event("#gh_purification_mob","OnSpawn02",false);
		npctalk "Oscar : The foods was distributed in the morning by the maids. The curse also affected the food and spread to the people who ate it.";
		sleep 3000;
		npctalk "Oscar : They went insane because of the curse, just like the maids who were preparing the meals.";
		sleep 3000;
		npctalk "Oscar : I'll be heading to the next area, please put the people around here to rest.";
		sleep 1000;
		instance_enable(strnpcinfo(0),false);
	}
end;
}

1@gl_prq,67,176,4	script	Oscar#gh_purification_2	4_ED_OSCAR,3,3,{
	end;

OnTouch:
	.@var = get_instance_var("purification");
	.@map$ = get_instance_var("map$");
	if(.@var == 2){
		set_instance_var("purification",3);
		npctalk "Oscar : This place used to be a guest area, with a lounge over there.";
		sleep 3000;
		instance_event("#gh_purification_mob","OnSpawn03",false);
		npctalk "Oscar : This place was where they keep literature and military information.";
		sleep 3000;
		npctalk "Oscar : It was mainly used and guarded by commanders.";
		sleep 3000;
		npctalk "Oscar : The people in this area is quite powerful. Be careful.";
		sleep 3000;
		npctalk "Oscar : I'll be heading to the final area, please put the commanders around here to rest.";
		sleep 1000;
		instance_enable(strnpcinfo(0),false);
	}
end;
}

1@gl_prq,52,270,4	script	Oscar#gh_purification_3	4_ED_OSCAR,3,3,{
	end;

OnTouch:
	.@var = get_instance_var("purification");
	.@map$ = get_instance_var("map$");
	if(.@var == 3){
		set_instance_var("purification",4);
		npctalk "Oscar : Since the dimensional barrier is not present, you might actually get hurt or worst. Be cautious.";
		sleep 3000;
		npctalk "Oscar : As you can see, there are many corpses around.";
		sleep 3000;
		npctalk "Oscar : That's King Schmidt's appearance, but this is not the real Schmidt. Don't hold back on your attacks.";
		sleep 3000;
		npctalk "Oscar : I'll gather the dimensional energy. The battle will begin, prepare yourself.";
		instance_hide(strnpcinfo(0),true);
		instance_enable(strnpcinfo(0),false);
		instance_hide("#schmidt_gh_purification",true);
		instance_enable("#schmidt_gh_purification",false);
		instance_event("#gh_purification_boss","OnSummon",false);
		instance_event("#gh_purification_fire","OnSummon",false);
	}
end;
}

1@gl_prq,52,270,4	script	Oscar#gh_purification_4	4_ED_OSCAR,3,3,{
	.@var = get_instance_var("hunt");
	.@mode = get_instance_var("mode");
	cutin "oscar01",2;
	mes "[ Oscar ]";
	mes "Traveling to different timelines ang engaging in combat takes a lot of energy.";
	next;
	mes "[ Oscar ]";
	mes "Oh, these? These are fragments of King Schmidt's memories.";
	next;
	mes "[ Oscar ]";
	mes "Oh, I think this dimension will start to collapse. Let's get out first.";
	if(checkquest(.@var,HUNTING) == 2){
		if(!.@mode)
			getitem 25739,9;
		else {
			getitem 25739,5;
			getitem 25740,6;
		}
		erasequest .@var;
	}
	close2;
	warp "glast_01",244,290;
	end;
}

1@gl_prq,52,277,4	duplicate(dummynpc2)	#schmidt_gh_purification	4_ED_SCHMIDT

1@gl_prq,1,1,0	script	#gh_purification_boss	-1,{
	end;
	
OnSummon:
	.@boss = get_instance_var("schmidt");
	.@map$ = get_instance_var("map$");
	.@gid = monster(.@map$,52,277,"Cursed King Schmidt",.@boss,1,instance_npcname(strnpcinfo(0)) + "::OnBossKill");
	set_instance_var("gid",.@gid);
	set_instance_var("boss_alive",1);
end;

OnBossKill:
	instance_enable("Oscar#gh_purification_4",true);
end;
}

1@gl_prq,1,1,0	script	#gh_purification_fire	-1,{
	end;
	
OnSummon:
	instance_event(strnpcinfo(0),"OnSummonLeft",false);
end;

OnSummonLeft:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnLeftFireDead";
	.@gid = get_instance_var("gid");
	for(.@i = 10; .@i < 96; .@i++){
		if(!unitexists(.@gid))
			end;
		if(.@i == 10)
			instance_event(strnpcinfo(0),"OnSummonRight",false);
		if(.@i == 55){
			instance_event(strnpcinfo(0),"OnSummonTop",false);
			instance_event(strnpcinfo(0),"OnSummonBottom",false);
		}
		.@left[0] = monster(.@map$,.@i,248,"",20426,1,.@event$);
		.@left[1] = monster(.@map$,.@i,251,"",20426,1,.@event$);
		.@left[2] = monster(.@map$,.@i,254,"",20426,1,.@event$);
		.@left[3] = monster(.@map$,.@i,257,"",20426,1,.@event$);
		.@left[4] = monster(.@map$,.@i,260,"",20426,1,.@event$);
		.@left[5] = monster(.@map$,.@i,263,"",20426,1,.@event$);
		.@left[6] = monster(.@map$,.@i,266,"",20426,1,.@event$);
		.@left[7] = monster(.@map$,.@i,269,"",20426,1,.@event$);
		.@left[8] = monster(.@map$,.@i,281,"",20426,1,.@event$);
		.@left[9] = monster(.@map$,.@i,284,"",20426,1,.@event$);
		.@left[10] = monster(.@map$,.@i,287,"",20426,1,.@event$);
		.@left[11] = monster(.@map$,.@i,290,"",20426,1,.@event$);
		.@left[12] = monster(.@map$,.@i,293,"",20426,1,.@event$);
		.@left[13] = monster(.@map$,.@i,296,"",20426,1,.@event$);
		.@left[14] = monster(.@map$,.@i,299,"",20426,1,.@event$);
		.@left[15] = monster(.@map$,.@i,302,"",20426,1,.@event$);
		.@left[16] = monster(.@map$,.@i,305,"",20426,1,.@event$);
		.@left[17] = monster(.@map$,.@i,308,"",20426,1,.@event$);
		sleep 300;
		killmonster .@map$,.@event$;
	}
OnLeftFireDead:
end;

OnSummonRight:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnRightFireDead";
	.@gid = get_instance_var("gid");
	for(.@i = 95; .@i > 10; .@i--){
		if(!unitexists(.@gid))
			end;
		.@right[0] = monster(.@map$,.@i,248,"",20426,1,.@event$);
		.@right[1] = monster(.@map$,.@i,251,"",20426,1,.@event$);
		.@right[2] = monster(.@map$,.@i,254,"",20426,1,.@event$);
		.@right[3] = monster(.@map$,.@i,257,"",20426,1,.@event$);
		.@right[4] = monster(.@map$,.@i,260,"",20426,1,.@event$);
		.@right[5] = monster(.@map$,.@i,263,"",20426,1,.@event$);
		.@right[6] = monster(.@map$,.@i,266,"",20426,1,.@event$);
		.@right[7] = monster(.@map$,.@i,269,"",20426,1,.@event$);
		.@right[8] = monster(.@map$,.@i,272,"",20426,1,.@event$);
		.@right[9] = monster(.@map$,.@i,275,"",20426,1,.@event$);
		.@right[10] = monster(.@map$,.@i,278,"",20426,1,.@event$);
		.@right[11] = monster(.@map$,.@i,287,"",20426,1,.@event$);
		.@right[12] = monster(.@map$,.@i,290,"",20426,1,.@event$);
		.@right[13] = monster(.@map$,.@i,293,"",20426,1,.@event$);
		.@right[14] = monster(.@map$,.@i,296,"",20426,1,.@event$);
		.@right[15] = monster(.@map$,.@i,299,"",20426,1,.@event$);
		.@right[16] = monster(.@map$,.@i,302,"",20426,1,.@event$);
		.@right[17] = monster(.@map$,.@i,305,"",20426,1,.@event$);
		.@right[18] = monster(.@map$,.@i,308,"",20426,1,.@event$);
		sleep 300;
		killmonster .@map$,.@event$;
	}
OnRightFireDead:	
end;

OnSummonBottom:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnBottomFireDead";
	.@gid = get_instance_var("gid");
	for(.@i = 248; .@i < 309; .@i++){
		if(!unitexists(.@gid))
			end;
		.@bottom[0] = monster(.@map$,12,.@i,"",20426,1,.@event$);
		.@bottom[1] = monster(.@map$,15,.@i,"",20426,1,.@event$);
		.@bottom[2] = monster(.@map$,18,.@i,"",20426,1,.@event$);
		.@bottom[3] = monster(.@map$,21,.@i,"",20426,1,.@event$);
		.@bottom[4] = monster(.@map$,24,.@i,"",20426,1,.@event$);
		.@bottom[5] = monster(.@map$,27,.@i,"",20426,1,.@event$);
		.@bottom[6] = monster(.@map$,30,.@i,"",20426,1,.@event$);
		.@bottom[7] = monster(.@map$,33,.@i,"",20426,1,.@event$);
		.@bottom[8] = monster(.@map$,36,.@i,"",20426,1,.@event$);
		.@bottom[9] = monster(.@map$,49,.@i,"",20426,1,.@event$);
		.@bottom[10] = monster(.@map$,52,.@i,"",20426,1,.@event$);
		.@bottom[11] = monster(.@map$,55,.@i,"",20426,1,.@event$);
		.@bottom[12] = monster(.@map$,58,.@i,"",20426,1,.@event$);
		.@bottom[13] = monster(.@map$,61,.@i,"",20426,1,.@event$);
		.@bottom[14] = monster(.@map$,64,.@i,"",20426,1,.@event$);
		.@bottom[15] = monster(.@map$,67,.@i,"",20426,1,.@event$);
		.@bottom[16] = monster(.@map$,70,.@i,"",20426,1,.@event$);
		.@bottom[17] = monster(.@map$,73,.@i,"",20426,1,.@event$);
		.@bottom[18] = monster(.@map$,76,.@i,"",20426,1,.@event$);
		.@bottom[19] = monster(.@map$,79,.@i,"",20426,1,.@event$);
		.@bottom[20] = monster(.@map$,82,.@i,"",20426,1,.@event$);
		.@bottom[21] = monster(.@map$,85,.@i,"",20426,1,.@event$);
		.@bottom[22] = monster(.@map$,88,.@i,"",20426,1,.@event$);
		.@bottom[23] = monster(.@map$,91,.@i,"",20426,1,.@event$);
		.@bottom[24] = monster(.@map$,94,.@i,"",20426,1,.@event$);
		sleep 300;
		killmonster .@map$,.@event$;
	}
OnBottomFireDead:
end;

OnSummonTop:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnTopFireDead";
	.@gid = get_instance_var("gid");
	for(.@i = 308; .@i > 247; .@i--){
		if(!unitexists(.@gid))
			end;	
		if(.@i == 260)
			instance_event(strnpcinfo(0),"OnSummonLeft",false);
		.@bottom[0] = monster(.@map$,13,.@i,"",20426,1,.@event$);
		.@bottom[1] = monster(.@map$,16,.@i,"",20426,1,.@event$);
		.@bottom[2] = monster(.@map$,19,.@i,"",20426,1,.@event$);
		.@bottom[3] = monster(.@map$,22,.@i,"",20426,1,.@event$);
		.@bottom[4] = monster(.@map$,25,.@i,"",20426,1,.@event$);
		.@bottom[5] = monster(.@map$,28,.@i,"",20426,1,.@event$);
		.@bottom[6] = monster(.@map$,31,.@i,"",20426,1,.@event$);
		.@bottom[7] = monster(.@map$,34,.@i,"",20426,1,.@event$);
		.@bottom[8] = monster(.@map$,37,.@i,"",20426,1,.@event$);
		.@bottom[9] = monster(.@map$,40,.@i,"",20426,1,.@event$);
		.@bottom[10] = monster(.@map$,43,.@i,"",20426,1,.@event$);
		.@bottom[11] = monster(.@map$,46,.@i,"",20426,1,.@event$);
		.@bottom[12] = monster(.@map$,49,.@i,"",20426,1,.@event$);
		.@bottom[13] = monster(.@map$,52,.@i,"",20426,1,.@event$);
		.@bottom[14] = monster(.@map$,55,.@i,"",20426,1,.@event$);
		.@bottom[15] = monster(.@map$,58,.@i,"",20426,1,.@event$);
		.@bottom[16] = monster(.@map$,70,.@i,"",20426,1,.@event$);
		.@bottom[17] = monster(.@map$,73,.@i,"",20426,1,.@event$);
		.@bottom[18] = monster(.@map$,76,.@i,"",20426,1,.@event$);
		.@bottom[19] = monster(.@map$,79,.@i,"",20426,1,.@event$);
		.@bottom[20] = monster(.@map$,82,.@i,"",20426,1,.@event$);
		.@bottom[21] = monster(.@map$,85,.@i,"",20426,1,.@event$);
		.@bottom[22] = monster(.@map$,88,.@i,"",20426,1,.@event$);
		.@bottom[23] = monster(.@map$,91,.@i,"",20426,1,.@event$);
		.@bottom[24] = monster(.@map$,94,.@i,"",20426,1,.@event$);
		sleep 300;
		killmonster .@map$,.@event$;
	}
OnTopFireDead:
end;
}

1@gl_prq,1,1,0	script	#gh_purification_mob	-1,{
	end;
	
OnSpawn00:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill00";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnDummy";
	.@maid = get_instance_var("maid");
	setarray .@bone_xy,
	45,35,
	32,41,
	61,45,
	70,59,
	19,62,
	31,50;
	for(.@i = 0; .@i < getarraysize(.@bone_xy); .@i += 2){
		.@bone = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"Flaming Thorn",1960,1,.@event2$);
		.@flame = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"",20426,1,.@event2$);
		getunitdata .@bone,.@mode;
		setunitdata .@bone,UMOB_MODE,(.@mode[UMOB_MODE]|MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC|MD_KNOCKBACK_IMMUNE|MD_DETECTOR);
		setunitdata .@flame,UMOB_DMGIMMUNE,true;
	}
	setarray .@xy,
	20,41,.@maid,
	70,72,.@maid,
	62,36,.@maid,
	20,57,.@maid,
	67,36,.@maid,
	20,67,.@maid,
	56,72,.@maid,
	56,40,.@maid,
	26,61,.@maid,
	38,36,.@maid,
	63,75,.@maid,
	20,56,.@maid,
	62,36,.@maid,
	34,61,.@maid,
	64,72,.@maid,
	63,47,.@maid,
	35,61,.@maid,
	64,72,.@maid,
	20,57,.@maid,
	20,56,.@maid,
	61,36,.@maid;
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 3){
		monster .@map$,.@xy[.@i],.@xy[.@i+1],"--ja--",.@xy[.@i+2],1,.@event$;
		.@x++;
		if(.@x % 4 == 0) //Spawning has intervals
			sleep 3000;
	}
end;

OnMobKill00:
	//There is 21 maids but event triggers on 20th maid kill.
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill00";
	.@party_id = get_instance_var("party_id");
	set_instance_var("phase_one",get_instance_var("phase_one") + 1);
	if(get_instance_var("phase_one") == 20){
		killmonster .@map$,.@event$;
		instance_enable("Oscar#gh_purification_0",true);
		mapannounce .@map$,"Oscar : The time flow in the first area has returned to normal. Let's go to the next area.",bc_map,0xFF66FF;
		sleep 3000;
		warpparty .@map$,123,63,.@party_id;
	}
end;

OnSpawn01:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill01";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnDummy";
	.@kh = get_instance_var("khalitzburg");
	.@bloody = get_instance_var("bloody");
	setarray .@bone_xy,
	151,70,
	161,33,
	137,57,
	160,38,
	138,39,
	151,54,
	148,56,
	139,66;
	for(.@i = 0; .@i < getarraysize(.@bone_xy); .@i += 2){
		.@bone = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"Flaming Thorn",1960,1,.@event2$);
		.@flame = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"",20426,1,.@event2$);
		getunitdata .@bone,.@mode;
		setunitdata .@bone,UMOB_MODE,(.@mode[UMOB_MODE]|MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC|MD_KNOCKBACK_IMMUNE|MD_DETECTOR);
		setunitdata .@flame,UMOB_DMGIMMUNE,true;
	}
	setarray .@xy,
	167,47,.@kh,
	160,21,.@kh,
	167,59,.@kh,
	165,66,.@kh,
	169,21,.@kh,
	144,21,.@kh,
	158,66,.@kh,
	167,64,.@kh,
	134,34,.@kh,
	154,49,.@kh,
	160,66,.@kh,
	158,66,.@kh,
	134,43,.@kh,
	134,31,.@kh,
	134,21,.@kh,
	167,62,.@kh,
	156,21,.@kh,
	134,31,.@kh,
	134,47,.@kh,
	134,35,.@kh,
	134,33,.@kh,
	167,47,.@kh,
	167,67,.@bloody,
	181,67,.@bloody;
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 3){
		if(.@xy[.@i+2] == .@kh)
			monster .@map$,.@xy[.@i],.@xy[.@i+1],"Insane Knight",.@xy[.@i+2],1,.@event$;
		else
			monster .@map$,.@xy[.@i],.@xy[.@i+1],"--ja--",.@xy[.@i+2],1,.@event$;
		.@x++;
		if(.@x % 6 == 0)
			sleep 3000;
	}
end;

OnMobKill01:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill01";
	.@party_id = get_instance_var("party_id");
	set_instance_var("phase_two",get_instance_var("phase_two") + 1);
	if(get_instance_var("phase_two") == 24){
		killmonster .@map$,.@event$;
		instance_enable("Oscar#gh_purification_1",true);
		mapannounce .@map$,"Oscar : The time flow in the second area has returned to normal. Let's go to the next area.",bc_map,0xFF66FF;
		sleep 3000;
		warpparty .@map$,148,116,.@party_id;
	}
end;

OnSpawn02:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill02";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnDummy";
	.@knight = get_instance_var("knight");
	.@bloody = get_instance_var("bloody");
	setarray .@bone_xy,
	136,173,
	154,148,
	142,171,
	181,120,
	158,161,
	148,140,
	178,133,
	161,157,
	149,146,
	143,120;
	for(.@i = 0; .@i < getarraysize(.@bone_xy); .@i += 2){
		.@bone = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"Flaming Thorn",1960,1,.@event2$);
		.@flame = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"",20426,1,.@event2$);
		getunitdata .@bone,.@mode;
		setunitdata .@bone,UMOB_MODE,(.@mode[UMOB_MODE]|MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC|MD_KNOCKBACK_IMMUNE|MD_DETECTOR);
		setunitdata .@flame,UMOB_DMGIMMUNE,true;
	}
	setarray .@xy,
	178,141,.@knight,
	165,123,.@knight,
	148,139,.@knight,
	178,141,.@knight,
	171,141,.@knight,
	164,173,.@knight,
	148,141,.@knight,
	131,165,.@knight,
	173,123,.@knight,
	169,173,.@knight,
	131,131,.@knight,
	147,173,.@knight,
	160,123,.@knight,
	164,173,.@knight,
	148,172,.@knight,
	164,173,.@knight,
	159,141,.@knight,
	160,141,.@knight,
	182,123,.@knight,
	185,123,.@knight,
	148,145,.@knight,
	136,173,.@knight,
	140,173,.@knight,
	132,173,.@knight,
	131,159,.@knight,
	171,173,.@knight,
	148,150,.@knight,
	134,141,.@bloody,
	137,141,.@bloody,
	131,141,.@bloody;
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 3){
		monster .@map$,.@xy[.@i],.@xy[.@i+1],"--ja--",.@xy[.@i+2],1,.@event$;
		.@x++;
		if(.@x % 10 == 0)
			sleep 3000;
	}
end;

OnMobKill02:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill02";
	.@party_id = get_instance_var("party_id");
	set_instance_var("phase_three",get_instance_var("phase_three") + 1);
	if(get_instance_var("phase_three") == 30){
		killmonster .@map$,.@event$;
		instance_enable("Oscar#gh_purification_2",true);
		mapannounce .@map$,"Oscar : The time flow in the third area has returned to normal. Let's go to the next area.",bc_map,0xFF66FF;
		sleep 3000;
		warpparty .@map$,76,178,.@party_id;
	}
end;

OnSpawn03:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnDummy";
	.@knight = get_instance_var("knight");
	.@bloody = get_instance_var("bloody");
	setarray .@bone_xy,
	59,142,
	56,177,
	63,178,
	24,146,
	40,147,
	26,153,
	45,159,
	26,162,
	40,128,
	20,163,
	46,128,
	56,164;
	for(.@i = 0; .@i < getarraysize(.@bone_xy); .@i += 2){
		.@bone = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"Flaming Thorn",1960,1,.@event2$);
		.@flame = monster(.@map$,.@bone_xy[.@i],.@bone_xy[.@i+1],"",20426,1,.@event2$);
		getunitdata .@bone,.@mode;
		setunitdata .@bone,UMOB_MODE,(.@mode[UMOB_MODE]|MD_IGNOREMELEE|MD_IGNOREMAGIC|MD_IGNORERANGED|MD_IGNOREMISC|MD_KNOCKBACK_IMMUNE|MD_DETECTOR);
		setunitdata .@flame,UMOB_DMGIMMUNE,true;
	}
	setarray .@xy,
	32,165,.@knight,
	37,178,.@knight,
	32,163,.@knight,
	40,164,.@knight,
	41,164,.@knight,
	36,178,.@bloody,
	41,130,.@bloody,
	55,164,.@bloody,
	32,141,.@bloody,
	26,130,.@bloody,
	25,164,.@bloody,
	17,164,.@bloody,
	47,164,.@bloody,
	42,130,.@bloody,
	27,130,.@bloody;
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 3){
		monster .@map$,.@xy[.@i],.@xy[.@i+1],"--ja--",.@xy[.@i+2],1,.@event$;
		.@x++;
		if(.@x % 7 == 0)
			sleep 3000;
	}
end;

OnMobKill03:
	.@map$ = get_instance_var("map$");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill03";
	.@party_id = get_instance_var("party_id");
	set_instance_var("phase_four",get_instance_var("phase_four") + 1);
	if(get_instance_var("phase_four") == 15){
		killmonster .@map$,.@event$;
		instance_enable("Oscar#gh_purification_3",true);
		instance_enable("#schmidt_gh_purification",true);
		mapannounce .@map$,"Oscar : That was the last room. Let's go see if King Schmidt's spirit is still roaming around.",bc_map,0xFF66FF;
		sleep 3000;
		warpparty .@map$,52,244,.@party_id;
	}
end;

OnDummy:
end;
}