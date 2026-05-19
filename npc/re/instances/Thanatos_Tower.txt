//===== rAthena script ===========================================|
//= Memory of Thanatos                                           =|
//===== By: ======================================================|
//= crazyarashi                                                  =|
//===== Current Version: =========================================|
//= 1.0 Initial Version                                          =|
//================================================================|

-	script	mot_init	-1,{
	end;
	
OnInit:
	disablenpc "#Charm Stone Admintt01";
	disablenpc "Gold Religious Statue#tt";
	disablenpc "Green Wiseman Statue#tt";
	disablenpc "Blue Angel Statue#tt";
	disablenpc "Bloody Knight Statue#tt";
	disablenpc "Dark Devil Statue#tt";
	disablenpc "#tteffect05";
	disablenpc "Memory Seal#tt5";
	disablenpc "#gateto_thanatos";
	disablenpc "#thanatos_seal";
	disablenpc "#sommon_thanatos";
	disablenpc "#cooltime_thana";
	for(.@i = 1; .@i < 5; .@i++){
		disablenpc "Shining Crystal#tt_r" + .@i;
		disablenpc "Shining Crystal#tt_y" + .@i;
		disablenpc "Shining Crystal#tt_b" + .@i;
		disablenpc "Shining Crystal#tt_g" + .@i;
		disablenpc "Memory Seal#tt" + .@i;
		disablenpc "#tteffect0" + .@i;
	}
end;
}

tha_t01,161,78,3	script	Solice#mot	4_F_CHNDOCTOR,{
	if(BaseLevel < 180){
		mes "[ Solice ]";
		mes "Do you need work? But, I think you're even weaker than me. Please come back when you're stronger, alright?";
		next;
		mes "^4D4DFFThe required level for this daily quest is Level 180 or above.^000000";
		end;
	}
	if(isbegin_quest(17408) == 0){
		mes "[ Solice ]";
		mes "Oh! You scared me! You're not currently in contract with us, so please go ahead and sign a contract.";
		end;
	}
	if(isbegin_quest(17415) < 2){
		mes "[ Solice ]";
		mes "Should I go up again? If I'm lucky again this time, I won't be blind and a victim.";
		next;
		mes "[ Solice ]";
		mes "You can't just leave your life to fate.";
		next;
		mes "[ Solice ]";
		mes "If I somehow survive here, I should register for the combat team. Huh? A person? An Adventurer! Finally, an adventure has appeared!";
		next;
		mes "[ Solice ]";
		mes "You're an adventurer who signed a contract with the Reckenbergs. Nice to meet you.";
		next;
		select("Why are you welcoming me?");
		mes "[ Solice ]";
		mes "Oh, we're from the Uno Monster Society. We collect and organize data from new monsters and provide the information to adventurers. We are here because a new monster was discovered";
		next;
		mes "[ Solice ]";
		mes "When I came here, I heard that the tower is cool, mysterious, and the monsters are very strong.";
		next;
		mes "[ Solice ]";
		mes "I came here with big expectations, but I know for sure that I'll die easily.";
		next;
		mes "[ Solice ]";
		mes "Wouldn't it have been more comfortable to just lie down and act dead? I cried because I was too excited. But, I decided to wait for others to come.";
		next;
		mes "[ Solice ]";
		mes "But, No. I thought I could do something. Because in our group, I was the strongest! I don't even forget to workout when I wake up!";
		next;
		mes "[ Solice ]";
		mes "Even though we're just person who handle the desks. If you work out, your strength will increase by > +1, am I right?";
		next;
		mes "[ Solice ]";
		mes "The lady beside me is the second strongest after me. That's why she is with me.";
		next;
		mes "[ Solice ]";
		mes "If we combine our strength, 1+1 it's just too weak. Don't you think so? If I had known, I would have come with my pet Poring.";
		next;
		mes "[ Solice ]";
		mes "I was going to study the pattern of the new monsters... Phew, but it's just too strong for me. I should be able to do it, If I know their move patterns.";
		next;
		mes "[ Solice ]";
		mes "That's not the only problem also.";
		next;
		mes "[ Solice ]";
		mes "That's why I waited for an adventurer who wants work over here. You know, adventurers who look stronger than those monsters. Can you help us? No, we'll hire you!";
		next;
		mes "[ Solice ]";
		mes "The association is in your hands, adventurer! I hope you don't reject this offer! We will pay you nicely!";
		next;
		mes "[ Solice ]";
		mes "Even if we don't have anything to offer, we have a lot of experience that we can provide.";
		next;
		if(select("Of course!:I'm too scared.") == 2){
			mes "[ Solice ]";
			mes "Oh, really? If you have second thoughts, please come.";
			next;
			mes "[ Solice ]";
			mes "Oh, no, adventurer. Are you going to leave... it's sad...";
			next;
			mes "[ Solice ]";
			mes "It's also our job to recommend a request to an adventurer who declines, sadly.";
			end;
		}
		mes "[ Solice ]";
		mes "Oh, that's a relief. That's a relief for sure. What job are you looking for?";
		next;
		mes "[ Solice ]";
		mes "Please. When you know other adventurers who's willing to help, would you let them know also?";
		next;
		mes "[ Solice ]";
		mes "But... Let's ask you first. You must have preferences too. What floor would you like to challenge?";
	} else {
		mes "[ Solice ]";
		mes "Adventurer? Which floor would you like to take a look at?";
	}
	next;
	switch(select("9th Floor:10th Floor:11th Floor:12th Floor:Nothing...")){
		case 1: 
			.@qid = 17416;
			.@cid = 17422;
			.@fragment = 7437;
			.@exp = 28750000;
			.@jexp = 20100000;
			break;

		case 2: 
			.@qid = 17417; 
			.@cid = 17423;
			.@fragment = 7439;
			.@exp = 35240500;
			.@jexp = 22005300;
			break;

		case 3: 
			.@qid = 17418; 
			.@qid2 = 17419;
			.@cid = 17424;
			.@fragment = 7438;
			.@exp = 44210500;
			.@jexp = 30900160;
			break;

		case 4:
			.@qid = 17420; 
			.@qid2 = 17421;
			.@cid = 17425;
			.@fragment = 7436;
			.@exp = 53190200;
			.@jexp = 37300300;
			break;

		case 5:
			mes "[ Solice ]";
			mes "Don't you have any interest in the tower and monsters? Then, there's nothing we can do. If we meet again next time, please help me then.";
			end;
	}
	mes "[ Solice ]";
	switch(checkquest(.@cid,PLAYTIME)){
		case -1: break;

		case 0:
		case 1:
			mes "We are still organizing the data that you collected. Please come back here after dawn.";
			end;

		case 2:
			erasequest .@cid;
			break;
	}
	switch(checkquest(.@qid,HUNTING)){
		case -1:
			switch(.@qid){
				case 17416:
					mes "The 9th floor? I was really curious about that place too! I couldn't even go in even though its right in front of me.";
					break;

				case 17417:
					mes "The 10th floor? When we went there, we immediately turned around right away and left. I was so upset.";
					break;

				case 17418:
					mes "The 11th floor? Wow~ We ran away as soon as we stepped in there. I was so frustrated when we went in there! Do you want to go to the 11th floor?";
					break;

				case 17420:
					mes "The 12th floor? Oh, we couldn't even go in there. The adventurer is brave. Do you want to go to the 12th floor?";
					break;
			}
			next;
			if(isbegin_quest(17415) == 2){
				if(select("I'll go.:No.") == 2){
					mes "[ Solice ]";
					mes "You're not interested? Then, there's nothing we can do. If we meet again next time, please help me then.";
					end;
				}
			}
			mes "[ Solice ]";
			switch(.@qid){
				case 17416:
					mes "On the 9th floor, you will face the ^0000CDCrow Duke and Crow Baron^000000. I think ^0000CD20 of each^000000 should be enough.";
					break;

				case 17417:
					mes "On the 10th floor, you can deal with ^0000CDVoid Mimic, Book of Death, and Eldest^000000. I think ^0000CD20 of each^000000 should be enough.";
					break;

				case 17418:
					mes "On the 11th floor, I want you to deal the with the following ^0000CDEmpathizer, Happy Giver, Pray Giver, and Smile Giver^000000. I think ^0000CD15 of each^000000 should be enough.";
					break;

				case 17420:
					mes "On the 12th floor, It will be hard but please deal with ^0000CDThanatos of Anger, Horror, Resentment, and Regret^000000. I think ^0000CD10 of each^000000 should be enough.";
					break;
			}
			next;
			mes "[ Solice ]";
			mes "Oh, also if you have time. Can you also bring me ^0000CD10 " + getitemname(.@fragment) + "^000000. It's optional, so don't feel pressured. But if you bring I'll give you an ^0000CDadditional reward.^000000";
			if(isbegin_quest(17415) == 0){
				setquest 17415;
				completequest 17415;
			}
			setquest .@qid;
			if(.@qid2) setquest .@qid2;
			next;
			mes "[ Solice ]";
			mes "Have a safe trip!";
			break;

		case 0:
		case 1:
			switch(.@qid){
				case 17416:
					mes "Oh, right. It was ^0000CDCrow Duke and Crow Baron^000000. You said you'd take care ^0000CD20 of each^000000, right?";
					break;			

				case 17417:
					mes "Hm, what was it again. Oh, right, it's ^0000CDVoid Mimic, Book of Death, and Eldest^000000. Please take care ^0000CD20 of each^000000.";
					break;

				case 17418:
					mes "I know this is quite a lot, but it's ^0000CDEmpathizer, Happy Giver, Pray Giver, and Smile Giver^000000. Please take care ^0000CD10 of each^000000.";
					break;

				case 17420:
					mes "This is quite hard. But please deal with ^0000CDThanatos of Anger, Horror, Resentment, and Regret^000000. ^0000CD10 of each^000000 should be enough.";
					break;
			}
			next;
			mes "[ Solice ]";
			mes "Oh, also if you have time. Can you also bring me ^0000CD10 " + getitemname(.@fragment) + "^000000. It's optional, so don't feel pressured. But if you bring I'll give you an ^0000CDadditional reward.^000000";
			next;
			mes "[ Solice ]";
			mes "Have a safe trip!";
			break;

		case 2:
			switch(.@qid){
				case 17418:
					if(checkquest(.@qid2,HUNTING) < 2){
						mes "I know this is quite a lot, but it's ^0000CDEmpathizer, Happy Giver, Pray Giver, and Smile Giver^000000. Please take care ^0000CD10 of each^000000.";
						next;
						mes "[ Solice ]";
						mes "Oh, also if you have time. Can you also bring me ^0000CD10 " + getitemname(.@fragment) + "^000000. It's optional, so don't feel pressured. But if you bring I'll give you an ^0000CDadditional reward.^000000";
						next;
						mes "[ Solice ]";
						mes "Have a safe trip!";
						end;
					}
					break;

				case 17420:
					if(checkquest(.@qid2,HUNTING) < 2){
						mes "This is quite hard. But please deal with ^0000CDThanatos of Anger, Horror, Resentment, and Regret^000000. ^0000CD10 of each^000000 should be enough.";
						next;
						mes "[ Solice ]";
						mes "Oh, also if you have time. Can you also bring me ^0000CD10 " + getitemname(.@fragment) + "^000000. It's optional, so don't feel pressured. But if you bring I'll give you an ^0000CDadditional reward.^000000";
						next;
						mes "[ Solice ]";
						mes "Have a safe trip!";
						end;
					}
					break;
			}
			mes "Cool. It's beautiful. Thanks to this we are able to achieve more quantitive and qualitative development on the monsters science.";
			mes "We'll be organizing the data, so please come back here after dawn.";
			erasequest .@qid;
			if(.@qid2) erasequest .@qid2;
			setquest .@cid;
			if(countitem(.@fragment) >= 10){
				delitem .@fragment,10;
				.@exp += 3000000;
				.@jexp += 1000000;
			}
			getexp .@exp,.@jexp;
			end;
	}
	end;

OnTouch:
	if(isbegin_quest(17415) < 2)
		npctalk "Ah... I really need help. Please...","",bc_self;
end;

OnInit:
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"BaseLevel >= 180 && isbegin_quest(17408) != 0 && isbegin_quest(17415) == 0";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17415) == 2 && isbegin_quest(17416) == 0 && checkquest(17422,PLAYTIME) == -1 || checkquest(17422,PLAYTIME) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17415) == 2 && isbegin_quest(17417) == 0 && checkquest(17423,PLAYTIME) == -1 || checkquest(17423,PLAYTIME) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17415) == 2 && isbegin_quest(17418) == 0 && checkquest(17424,PLAYTIME) == -1 || checkquest(17424,PLAYTIME) == 2";
	questinfo QTYPE_DAILYQUEST,QMARK_YELLOW,"isbegin_quest(17415) == 2 && isbegin_quest(17420) == 0 && checkquest(17425,PLAYTIME) == -1 || checkquest(17425,PLAYTIME) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(17416,HUNTING) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(17417,HUNTING) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(17418,HUNTING) == 2 && checkquest(17419,HUNTING) == 2";
	questinfo QTYPE_QUEST,QMARK_YELLOW,"checkquest(17420,HUNTING) == 2 && checkquest(17421,HUNTING) == 2";
	setunittitle getnpcid(0),"[Thanatos Tower Staff]";
end;
}

tha_t01,162,77,3	script	Jei#mot	4_F_HUWOMAN,{
	if(isbegin_quest(17408) == 0){
		mes "[ Jei ]";
		mes "We can't work together, since you haven't signed a contract yet. Hurry and get a contract from the guide next to us.";
		end;
	}
	mes "[ Jei ]";
	mes "If I were able to get out of here alive... I will really... Oh. If you have something to say, tell Solice. I'm a little busy. I need to get out...!";
	end;
	
OnInit:
	setunittitle getnpcid(0),"[Thanatos Tower Staff]";
end;
}

tha_t01,139,83,5	script	Mini Robe#mot	4_F_02,{
	mes "[ Mini ]";
	mes "We provide rewards to those who worked hard contributing to the development of Thanatos Tower.";
	mes "If you bring any proof of cooperation, we will reward you as promised.";
	next;
	switch(select("Fragment Exchange.:Sin Accessories.:Goodwill Accessories.:Reward?:Not interested.")){
		case 1:
			mes "[ Mini ]";
			mes "If you have ^0000CDvarious shards^000000, you can exchange it here to a Fragment of ^0000CDSin^000000 or ^0000CDGoodwill^000000, respectively.";
			close2;
			callshop "mot_shop_0",1;
			break;
			
		case 2:
			mes "[ Mini ]";
			mes "You can exchange your ^0000CDFragment of Sins^000000 to a ^0000CDSin Accessories^000000. Please take a look.";
			close2;
			callshop "mot_shop_2",2;
			break;
			
		case 3:
			mes "[ Mini ]";
			mes "You can exchange your ^0000CDFragment of Goodwill^000000 to a ^0000CDGoodwill Accessories^000000. Please take a look.";
			close2;
			callshop "mot_shop_3",2;
			break;
			
		case 4:
			mes "[ Mini ]";
			mes "The Reckenberg's is gathering people to explore and develop the Thanatos Tower.";
			next;
			mes "[ Mini ]";
			mes "If you sign a contract with us, hunt down monsters and bring back proof of cooperation, we will compensate you.";
			next;
			mes "[ Mini ]";
			mes "You can use the compensation freely.";
			break;
			
		case 5:
			mes "[ Mini ]";
			mes "If you can hunt down monsters in the Tower, make sure you bring proof. We will compensate you anytime.";
			break;
	}
	end;
	
OnInit:
	setunittitle getnpcid(0),"[Thanatos Tower Staff]";
end;
}

tha_t01,135,83,5	script	Tamilla#mot	4064,{
	if(checkweight(1201,1) == 0 || MaxWeight - Weight < 1000){
		mes "You can't proceed with the conversation because you have a large quantity of items.";
		mes "Please organize your items and try again.";
		close;
	}
	disable_items;
	mes "[ Tamilla ]";
	mes "Welcome. Did you bring something that I can work on?";
	next;
	switch(select("Information.:Enchant Accessories.:Initialize Enchant.")){
		case 1:
			setarray .@sin,490044,490046,490048,490050,490052,490054,490045,490047,490049,490051,490053,490055;
			setarray .@light,490056,490058,490060,490062,490064,490066,490057,490059,490061,490063,490065,490067;
			mes "[ Tamilla ]";
			mes "First and foremost, I have to tell you which equipments I can work on.";
			next;
			for(.@i = 0; .@i < getarraysize(.@sin); .@i++)
				mes "[<ITEM>" + getitemname(.@sin[.@i]) + "<INFO>" + .@sin[.@i] + "</INFO></ITEM>]";
			next;
			for(.@i = 0; .@i < getarraysize(.@light); .@i++)
				mes "[<ITEM>" + getitemname(.@light[.@i]) + "<INFO>" + .@light[.@i] + "</INFO></ITEM>]";
			next;
			mes "[ Tamilla ]";
			mes "It's possible for me to enchant you equipments I described on the sockets 3~4, the required fee varies depending on the method. I'll let you know the right price when we're about to begin.";
			next;
			mes "[ Tamilla ]";
			mes "I can also initialize the enchants on your equipments, so if you don't like the effect, you can always try again next time.";
			next;
			mes "[ Tamilla ]";
			mes "It's best for you to try it yourself rather than me explaining it, no? If you're looking to strenghtening your equipment, you know where to find me.";
			break;
			
		case 2:
			function check_equipment;
			function enchant;
			check_equipment(false);
			
		case 3:
			function check_equipment;
			function reset;
			check_equipment(true);
	}
	end;
	
function	check_equipment	{
	mes "[ Tamilla ]";
	mes "You can choose right or left accessory. Which side do you want to work on?";
	next;
	switch(select("Cancel.:Right.:Left")){
		case 1:
			mes "[ Tamilla ]";
			mes "Please come back once you have decided.";
			end;
			
		case 2: .@part = EQI_ACC_R; break;
		case 3: .@part = EQI_ACC_L; break;
	}
	.@equip_id = getequipid(.@part);
	if(!.@equip_id){
		mes "[ Tamilla ]";
		mes "Hm. You don't have anything there. Please come back once you have equipped your accessory.";
		end;
	}
	.@idx = inarray(.sin_id,.@equip_id);
	if(.@idx != -1) 
		.@type = 1;
	else 
		.@idx = inarray(.light_id,.@equip_id);
	if(.@idx == -1){
		mes "[ Tamilla ]";
		mes "This equipment is not an item that can be processed by our branch.";
		end;
	}
	if(!getarg(0))
		enchant(.@part,.@equip_id,.@type);
	else
		reset(.@part,.@equip_id);
}
	
function	enchant	{
	.@part = getarg(0);
	.@equip_id = getarg(1);
	.@type = getarg(2);
	if(.@type)
		.@enchant$ = "sin";
	else
		.@enchant$ = "light";
	for(.@i = 0; .@i < 4; .@i++){
		.@card[.@i] = getequipcardid(.@part,.@i);
		.@check[.@i] = getequipcardid(.@part,.@i);
	}
	.@equip_name$ = getitemname(.@equip_id);
	.@refine = getequiprefinerycnt(.@part);
	if(.@card[2] || .@card[3]){
		mes "[ Tamilla ]";
		mes "I apologize but this equipment already has effects.";
		mes "If you're unhappy with the effects, you can use the initialize method to remove the enchants.";
		end;
	}
	mes "[ Tamilla ]";
	mes "I can enchant your equipment with two methods. The first one is the normal method, the enchants will be completely random but there will be ^FF0000two enchants^000000 on the slot 3~4.";
	mes "The other one is you'll be able to select a specific enchant. However, I will only be able to give it ^FF0000one enchant.^000000";
	next;
	switch(select("Proceed with normal method.:Specific enchant method.:Cancel")){
		case 1:
			break;
		case 2:
			.@method = 1;
			break;
		case 3:
			mes "[ Tamilla ]";
			mes "Please come back once you have decided.";
			end;
	}
	if(!.@method){
		.@amount = 10;
		if(!.@type)
			.@item_id = 1000263;
		else
			.@item_id = 1000257;
		mes "[ Tamilla ]";
		mes "The enchant fee for this specific equipment for this method will be, ";
		mes "^0000CD" + .@amount + " " + getitemname(.@item_id) + "^000000.";
		next;
		mes "[ Tamilla ]";
		mes "Do you want to proceed enchanting your ^0000CD" + .@equip_name$ + "^000000?";
		next;
		if(select("Cancel.:Proceed.") == 1){
			mes "[ Tamilla ]";
			mes "Please come back once you have decided.";
			end;		
		}
		if(countitem(.@item_id) < .@amount){
			mes "[ Tamilla ]";
			mes "Our branch supports fair services, I won't be able to empower your equipment since you don't have enough materials for the fee.";
			end;
		}
		.@enchant = getd("." + .@enchant$ + "[" + rand(getarraysize(getd("." + .@enchant$))) + "]");
		.@card[2] = .@enchant;
		.@enchant = getd("." + .@enchant$ + "_2[" + rand(getarraysize(getd("." + .@enchant$ + "_2"))) + "]");
		.@card[3] = .@enchant;
	} else {
		.@slot = 2;
		.@amount = 50;
		if(!.@type){
			setarray .@types$,"empathy","happy","shelter","solace","divine","destructive";
			.@item_id = 1000263;
		} else {
			setarray .@types$,"anger","horror","resentment","regret","inverse","dragon";
			.@item_id = 1000257;
		}
		.@size = getarraysize(.@types$);
		.@menu$ = "Cancel:";
		for(.@i = 0; .@i < .@size; .@i++)
			.@menu$ += setchar(.@types$[.@i],strtoupper(charat(.@types$[.@i],0)),0) + ":";
		mes "[ Tamilla ]";
		mes "The enchant fee for this specific equipment for this method will be, ";
		mes "^0000CD" + .@amount + " " + getitemname(.@item_id) + "^000000.";
		mes "Then, please select the type of enchant you want to add in your equipment.";
		next;
		.@s = select(.@menu$);
		switch(.@s){
			case 1:
				mes "[ Tamilla ]";
				mes "Please come back once you have decided.";
				end;

			default:
				.@s -= 2;
				mes "[ Tamilla ]";
				mes "Are you sure you want to proceed with the " + .@types$[.@s] + " enchant for your ^0000CD" + .@equip_name$ + "^000000?";
				next;
				if(select("Cancel.:Proceed.") == 1){
					mes "[ Tamilla ]";
					mes "Please come back once you have decided.";
					end;		
				}
				if(countitem(.@item_id) < .@amount){
					mes "[ Tamilla ]";
					mes "Our branch supports fair services, I won't be able to empower your equipment since you don't have enough for the fee.";
					end;
				}
				if(.@s >= 4) .@slot = 3;
				.@enchant = getd("." + .@types$[.@s] + "[" + rand(getarraysize(getd("." + .@types$[.@s]))) + "]");
				.@card[.@slot] = .@enchant;
				break;
		}
	}
	delitem .@item_id,.@amount;
	if(callfunc("F_IsEquipIDHack", .@part, .@equip_id) || callfunc("F_IsEquipRefineHack", .@part, .@refine) || callfunc("F_IsEquipCardHack", .@part, .@check[0], .@check[1], .@check[2], .@check[3]))
		end;
	specialeffect2 EF_REPAIRWEAPON;
	delequip .@part;
	getitem2 .@equip_id,1,1,.@refine,0,.@card[0],.@card[1],.@card[2],.@card[3];
	mes "[ Tamilla ]";
	mes "Thank you for using my services, If you're unhappy with the effects, you can use the initialize method to remove the enchants.";
	end;
}

function	reset	{
	.@part = getarg(0);
	.@equip_id = getarg(1);
	.@equip_name$ = getitemname(.@equip_id);
	for(.@i = 0; .@i < 4; .@i++){
		.@card[.@i] = getequipcardid(.@part,.@i);
		.@check[.@i] = getequipcardid(.@part,.@i);
	}
	if(!.@card[2] && !.@card[3]){
		mes "[ Tamilla ]";
		mes "Hm. Adventurer, this equipment doesn't have any enchants in it. Please talk to me again when you find the correct one.";
		end;
	}
	mes "[ Tamilla ]";
	mes "Initializing your equipment, removes the 3~4 slot enchants of your equipment.";
	mes "Same with the enchanting, our branch offers two method for initializing your equipment.";
	next;
	mes "[ Tamilla ]";
	mes "You can either pay with with " + .zeny_fee + " Zeny or pay with " + .premium_amount + " " + getitemname(.premium_reset) + ".";
	mes "However, paying with zeny has a ^FF0000" + .break_chance + "% break chance^000000 upon initializing your equipment.";
	next;
	if(select("Pay with Zeny(" + (100 - .break_chance) + "%)","Pay with " + getitemname(.premium_reset) + "(100%)")){
		mes "[ Tamilla ]";
		mes "Are you sure you want to proceed initializing your ^0000CD" + .@equip_name$ + "^000000 using Zeny?";
		mes "There is a ^FF0000" + .break_chance + " chance that your equipment will break^000000.";
		next;
		if(select("Cancel.:Proceed.") == 1){
			mes "[ Tamilla ]";
			mes "Please come back once you have decided.";
			end;
		}
		if(Zeny < .zeny_fee){
			mes "[ Tamilla ]";
			mes "Our branch supports fair services, I won't be able to empower your equipment since you don't have enough for the fee.";
			end;
		}
		Zeny -= .zeny_fee;
		.@break_chance = .break_chance;
	} else {
		mes "[ Tamilla ]";
		mes "Are you sure you want to initialize your " + .@equip_name$ + " using " + getitemname(.premium_reset) + "?";
		next;
		if(select("Cancel.:Proceed.") == 1){
			mes "[ Tamilla ]";
			mes "Please come back once you have decided.";
			end;
		}
		if(countitem(.premium_reset) < .premium_amount){
			mes "[ Tamilla ]";
			mes "Our branch supports fair services, I won't be able to empower your equipment since you don't have enough for the fee.";
			end;
		}
		delitem .premium_reset,.premium_amount;
		.@break_chance = 0;
	}
	if(rand(1,100) > .@break_chance){
		specialeffect EF_REPAIRWEAPON;
		delequip .@part;
		getitem2 .@equip_id,1,1,.@refine,0,.@card[0],.@card[1],0,0;
		mes "[ Tamilla ]";
		mes "Thank you for using my services, With this you can try enchant this piece of equipment again.";
	} else {
		specialeffect EF_REFINEFAIL;
		delequip .@part;
		mes "[ Tamilla ]";
		mes "Kak! I didn't mean to break it... I'm sorry, but it looks like reinforcing the slot broke it from the inside.";
		emotion ET_HUK;
	}
	end;
}
	
OnInit:
	.premium_reset = 6909; //ITEM ID FOR PREMIUM RESET
	.premium_amount = 3;
	.zeny_fee = 1000000;
	.break_chance = 30;
	setunittitle getnpcid(0),"[Thanatos Tower Staff]";
	
	setunitdata getnpcid(0),UNPC_SEX,SEX_FEMALE;
	setunitdata getnpcid(0),UNPC_HAIRSTYLE,4; //kRO Palette
	setunitdata getnpcid(0),UNPC_HAIRCOLOR,2; //kRO Palette
	setunitdata getnpcid(0),UNPC_HEADTOP,547;
	setarray .@weight,
	35, //Level 1
	30, //Level 2
	20, //Level 3
	10, //Level 4
	5; // Level 5
	setarray .@empathy,310217,310218,310219,310220,310221;
	setarray .@happy,310222,310223,310224,310225,310226;
	setarray .@shelter,310227,310228,310229,310230,310231;
	setarray .@solace,310232,310233,310234,310235,310236;
	setarray .@divine,310247,310248,310249,310250,310251;
	setarray .@destructive,310252,310253,310254,310255,310256;
	setarray .@anger,310197,310198,310199,310200,310201;
	setarray .@horror,310202,310203,310204,310205,310206;
	setarray .@resentment,310207,310208,310209,310210,310211;
	setarray .@regret,310212,310213,310214,310215,310216;
	setarray .@inverse,310237,310238,310239,310240,310241;
	setarray .@dragon,310242,310243,310244,310245,310246;
	for(.@i = 0; .@i < 5; .@i++){
		for(.@j = 0; .@j < .@weight[.@i]; .@j++){
			.light[getarraysize(.light)] = .@empathy[.@i];
			.light[getarraysize(.light)] = .@happy[.@i];
			.light[getarraysize(.light)] = .@shelter[.@i];
			.light[getarraysize(.light)] = .@solace[.@i];
			.light_2[getarraysize(.light_2)] = .@divine[.@i];
			.light_2[getarraysize(.light_2)] = .@destructive[.@i];
			.empathy[getarraysize(.empathy)] = .@empathy[.@i];
			.happy[getarraysize(.happy)] = .@happy[.@i];
			.shelter[getarraysize(.shelter)] = .@shelter[.@i];
			.solace[getarraysize(.solace)] = .@solace[.@i];
			.divine[getarraysize(.divine)] = .@divine[.@i];
			.destructive[getarraysize(.destructive)] = .@destructive[.@i];
			.sin[getarraysize(.sin)] = .@anger[.@i];
			.sin[getarraysize(.sin)] = .@horror[.@i];
			.sin[getarraysize(.sin)] = .@resentment[.@i];
			.sin[getarraysize(.sin)] = .@regret[.@i];
			.sin_2[getarraysize(.sin_2)] = .@inverse[.@i];
			.sin_2[getarraysize(.sin_2)] = .@dragon[.@i];
			.anger[getarraysize(.anger)] = .@anger[.@i];
			.horror[getarraysize(.horror)] = .@horror[.@i];
			.resentment[getarraysize(.resentment)] = .@resentment[.@i];
			.regret[getarraysize(.regret)] = .@regret[.@i];
			.inverse[getarraysize(.inverse)] = .@inverse[.@i];
			.dragon[getarraysize(.dragon)] = .@dragon[.@i];	
		}
	}
	for(.@i = 490044; .@i < 490056; .@i++)
		.sin_id[getarraysize(.sin_id)] = .@i;
	for(.@i = 490056; .@i < 490068; .@i++)
		.light_id[getarraysize(.light_id)] = .@i;
end;
}

thana_step,154,367,3	script	Lumin#mot	4_M_AC_RUMIN,{
	if(BaseLevel < 180){
		cutin "lumin_ac_01",0;
		mes "[ Lumin ]";
		mes "Hello.";
		mes "Hm... It's my pleasure to see your brilliant performance..";
		mes "But I recommend you no to challenge the upper floors of the tower.";
		next;
		mes "[ Lumin ]";
		mes "Well, you won't be able to climb to the end anyway.";
		mes "Anyways, I'm rooting for you so cheer up.";
		next;
		cutin "",255;
		mes "^4D4DFF This place is an MD that can be progressed if you're level 180 or above. ^000000";
		end;
	}
	if(checkquest(18026,HUNTING) == 2 || checkquest(18027,HUNTING) == 2) .@finish = true;
	if(.@finish){
		cutin "lumin_ac_03",0;
		mes "[ Lumin ]";
		mes "Looking at your expression, the results of the tower exploration must have went well.";
		mes "I won't worry too much If I were you, we're the only ones involved in this tower anyway...";
		next;
		mes "[ Lumin ]";
		mes "Take this. This is the only thing I can give you.";
		erasequest 18026;
		erasequest 18027;
		setquest 18028;
		getexp 15000000,10000000;
		close3;
	}
	switch(checkquest(18028,PLAYTIME)){
		case -1:
			break;
			
		case 0:
		case 1:
			cutin "lumin_ac_01",0;
			mes "[ Lumin ]";
			mes "Hmm. It's a pleasure to see your performance... but I suggest that you don't climb the tower right now. Rest up and talk to me again tomorrow.";
			close3;
			
		case 2:
			erasequest 18028;
			setquest 18026;
			setquest 18027;
			break;
	}
	if(!isbegin_quest(18026)){
		cutin "lumin_ac_01",0;
		mes "[ Lumin ]";
		mes "Can you feel it? The flow of mana is different from the usual.";
		mes "I came here to investigate after receiving an information that a famous senior from the past appeared here.";
		next;
		mes "[ Lumin ]";
		mes "It feels slightly different from just climbing a tower, it's like being stuck in a loop and every moment is repeating.";
		next;
		mes "[ Lumin ]";
		mes "Even though it's from the past, you'll inherit the thoughts and feelings of it. Things like that.";
		mes "If you're curious, I'll open the way for you.";
		next;
		if(select("Investigate the Memory of Thanatos.:No.") == 2){
			mes "[ Lumin ]";
			mes "Haha... Okay. I like your determined look.";
			close3;
		}
		mes "[ Lumin ]";
		mes "The memories of the magic swordsman Thanatos were witnessed in two different forms.";
		mes "Either way is fine, I hope you'll experience it and tell me the details.";
		setquest 18026;
		setquest 18027;
		next;
		cutin "lumin_ac_02",0;
		mes "[ Lumin ]";
		mes "It's dangerous to go alone, if you have anyone you can rely on, ask them to join.";
		mes "No matter how good you are, it'll be annoying to deal with his thoughts.";
		next;
		cutin "lumin_ac_04",2;
		mes "[ Lumin ]";
		mes "Then, talk me again when you're ready.";
		mes "I'll open the way up to the top of the tower.";
		close3;
	}
	.@party_id = getcharid(1);
	if(!.@party_id) instance_warning(1);
	.@md_name$ = "Thanatos Tower";
	cutin "lumin_ac_04",2;
	mes "[ Lumin ]";
	mes "Now, it's time to explore the Thanatos Tower";
	mes "Of course, if it's you, you'd be ready, right?";
	next;
	if(select("Apply for 'Thanatos Tower':Enter 'Thanatos Tower'") == 1){
		if(!is_party_leader()) instance_warning(4);
		if(instance_create(.@md_name$) == -3) instance_warning(2,.@md_name$);
		mes "[ Lumin ]";
		mes "If you're lucky, the upper floors will open soon. The stairs is not the usual that you use to go up.";
	} else {
		if(instance_enter(.@md_name$) == IE_OK){
			.@party_name$ = getpartyname(.@party_id);
			mapannounce "thana_step", strcharinfo(0) + " of the party, "+ .@party_name$ +", is entering the " + .@md_name$ + ".", bc_map, "0x00FF99";
		} else {
			mes "[ Lumin ]";
			mes "This place has been in dispute for hundreds of years causing the flow of mana to be always unstable. It may take some time open.";	
		}
	}
	close3;
}

1@thts,1,1,0	script	#mot_control	HIDDEN_WARP_NPC,{
	end;
	
OnInstanceInit:
	instance_enable("Watcher#mot",false);
	instance_enable("Sealed Hatred#mot",false);
	instance_enable("mot_memory_3",false);
	instance_enable("mot_memory_4",false);
	instance_enable("mot_memory_5",false);
	instance_enable("mot_memory_6",false);
	instance_enable("Memory Piece#mot_0",false);
	instance_enable("Memory Piece#mot_1",false);
	for(.@i = 0; .@i < 4; .@i++){
		instance_enable("Shining Crystal#mot_0_" + .@i,false);
		instance_enable("Shining Crystal#mot_1_" + .@i,false);
		instance_enable("Shining Crystal#mot_2_" + .@i,false);
		instance_enable("Shining Crystal#mot_3_" + .@i,false);
	}
	for(.@i = 0; .@i < 5; .@i++){
		instance_enable("#mot_return_" + .@i,false);
		instance_enable("#mot_exit_" + .@i,false);
		instance_enable("Stone Barrier#mot_" + .@i + "-2",false);
	}
	instance_enable("#mot_exit_5",false);
	instance_enable("#mot_exit_6",false);
	instance_enable("Lucille#motend",false);
	//Top Floor Wall
	.@map$ = instance_mapname("8@thts");
	setcell .@map$,168,152,168,156,CELL_WALKABLE,0;
	setcell .@map$,168,152,168,156,CELL_SHOOTABLE,0;
	setcell .@map$,161,113,163,117,CELL_WALKABLE,0;
	setcell .@map$,161,113,163,117,CELL_SHOOTABLE,0;
	setcell .@map$,110,112,112,117,CELL_WALKABLE,0;
	setcell .@map$,110,112,112,117,CELL_SHOOTABLE,0;
	setcell .@map$,104,155,105,159,CELL_WALKABLE,0;
	setcell .@map$,104,155,105,159,CELL_SHOOTABLE,0;
	setcell .@map$,138,174,143,175,CELL_WALKABLE,0;
	setcell .@map$,138,174,143,175,CELL_SHOOTABLE,0;
	instance_event(strnpcinfo(0),"OnRandomize",false);
end;

OnRandomize:
	set_instance_var("exit_0",2);
	for(.@i = 1; .@i < 5; .@i++){
		.@v = rand(3,6);
		while(inarray(.@floor,.@v) != -1)
			.@v = rand(3,6);
		.@floor[getarraysize(.@floor)] = .@v;
		if(.@i == 1)
			set_instance_var("exit_" + .@i,.@v);	
		else 
			set_instance_var("exit_" + (get_instance_var("last_floor") - 1),.@v);
		set_instance_var("last_floor",.@v);
	}
	set_instance_var("exit_" + (get_instance_var("last_floor") - 1),7);
	set_instance_var("Hatred",3);
	set_instance_var("Agony",4);
	set_instance_var("Misery",5);
	set_instance_var("Despair",6);
end;
}

1@thts,156,371,3	script	Lucille#mot	4_F_LUCILE,{
	if(!is_party_leader()) end;
	if(!get_instance_var("mot")){
		set_instance_var("mot",1);
		for(.@i = 0; .@i < 6; .@i++)
			instance_enable("#mot_exit_" + .@i,true);
		instance_enable("Watcher#mot",true);
		mes "[ Lucille ]";
		mes " ... ...";
		sleep2 1000;
		mes " ... ...";
		sleep2 1000;
		cutin "tnm_lucile01",2;
		mes "A person...?";
		next;
		mes "[ Lucille ]";
		mes "My greetings are late.";
		mes "I'm Lucille, a girl.";
		mes "What brings you here?";
		next;
		mes "[ Lucille ]";
		mes "No, no, I'm sure this is fate. Can I ask you a favor?";
		next;
		cutin "tnm_lucile02",2;
		mes "[ Lucille ]";
		mes "I tried to erase the remnant of the pasts left in this tower, but I wasn't able to leave this place.";
		next;
		if(select("Ask for details.:What should I do?") == 1){
			cutin "tnm_lucile01",2;
			mes "[ Lucille ]";
			mes "There's no turning back once you climb this tower.";
			mes "Originally, this place is a gate that's connected to the demon realm.";
			mes "In the past, the demon called Morroc connected the gates by force and descended on the human realm.";
			next;
			mes "[ Lucille ]";
			mes "The warriors or the continent fought against him, and eventually Morroc fled to the far south and met his demise.";
			mes "In the aftermath, the whole area became a desert...";
			next;
			mes "[ Lucille ]";
			mes "During that time, there was a person who guarded the last floor of the tower.";
			mes "His remnants and memories must have flowed into the demon realm...";
			next;
			mes "[ Lucille ]";
			mes "I don't know if it's due to the peculiarities of the tower being used as a gate";
			mes "The remnants of those memories and emotions are endlessly appearing in the tower.";
			next;
			mes "[ Lucille ]";
			mes "I have discovered it recently, and climbed to the top quickly to deal with it";
			mes "There's no way that the celestial beings will just leave the gate to the demon realm unattended.";
			next;
			mes "[ Lucille ]";
			mes "It seems that they sealed the tower itself.";
			mes "That's why I can't proceed anymore and stopped here.";
			next;
			cutin "tnm_lucile02",2;
			mes "[ Lucille ]";
			mes "... Truth is, I'm also suspicious that a fragment of my memory is also tied to the tower during my time here...";
			next;
			mes "[ Lucille ]";
			mes "Otherwise, I won't be able to stay in the stairs and I won't be able to endure the pressure, right?";
			next;
			cutin "tnm_lucile01",2;
			mes "[ Lucille ]";
			mes "But, I already know that the answer can't be found with myself alone.";
			mes "Therefore, I dare to ask you.";
			next;
			mes "[ Lucille ]";
			mes "Please climb this tower to the end, and erase the bounded memories of 'Thanatos'.";
		} else {
			cutin "tnm_lucile01",2;
			mes "[ Lucille ]";
			mes "Please erase the bounded memories of 'Thanatos' that is bounded to this tower.";
			mes "The celestial beings are on guard, but I'm sure you'll figure something out, right?";
			next;
			mes "[ Lucille ]";
			mes "The memories that you'll face will be different each time you leave the stairs.";
			mes "I wish you luck.";
		}
		next;
		mes "[ Lucille ]";
		mes "Also, This is something from my memory.";
		mes "^4D4DFFThere is a Shining Crystal with a magic stone somewhere in the tower.";
		mes "Don't forget to collect those 4 colored magic stones. It's necessary to reach the top of the tower.";
		npctalk "You need the 4 colored magic stones to reach the top of the tower.";
		close3;
	}
	cutin "tnm_lucile01",2;
	mes "[ Lucille ]";
	mes "The celestial beings interference will be quite strong, but I believe in your will that brought you all the way here.";
	mes "Please climb the tower.";
	next;
	if(select("Leave it to me.:Ask for details.") == 1){
		mes "[ Lucille ]";
		mes "^4D4DFFDon't forget to take the magic stones from the Shining Crystals.^000000";
		mes "I wish you luck.";
	} else {
		cutin "tnm_lucile01",2;
		mes "[ Lucille ]";
		mes "There's no turning back once you climb this tower.";
		mes "Originally, this place is a gate that's connected to the demon realm.";
		mes "In the past, the demon called Morroc connected the gates by force and descended on the human realm.";
		next;
		mes "[ Lucille ]";
		mes "The warriors or the continent fought against him, and eventually Morroc fled to the far south and met his demise.";
		mes "In the aftermath, the whole area became a desert...";
		next;
		mes "[ Lucille ]";
		mes "During that time, there was a person who guarded the last floor of the tower.";
		mes "His remnants and memories must have flowed into the demon realm...";
		next;
		mes "[ Lucille ]";
		mes "I don't know if it's due to the peculiarities of the tower being used as a gate";
		mes "The remnants of those memories and emotions are endlessly appearing in the tower.";
		next;
		mes "[ Lucille ]";
		mes "I have discovered it recently, and climbed to the top quickly to deal with it";
		mes "There's no way that the celestial beings will just leave the gate to the demon realm unattended.";
		next;
		mes "[ Lucille ]";
		mes "It seems that they sealed the tower itself.";
		mes "That's why I can't proceed anymore and stopped here.";
		next;
		cutin "tnm_lucile02",2;
		mes "[ Lucille ]";
		mes "... Truth is, I'm also suspicious that a fragment of my memory is also tied to the tower during my time here...";
		next;
		mes "[ Lucille ]";
		mes "Otherwise, I won't be able to stay in the stairs and I won't be able to endure the pressure, right?";
		next;
		cutin "tnm_lucile01",2;
		mes "[ Lucille ]";
		mes "But, I already know that the answer can't be found with myself alone.";
		mes "Therefore, I dare to ask you.";
		next;
		mes "[ Lucille ]";
		mes "Please climb this tower to the end, and erase the bounded memories of 'Thanatos'.";
	}
	close3;
}

8@thts,140,130,3	script	Lucille#motend	4_F_LUCILE,{
	mes "[ Lucille ]";
	mes "It doesn't feel good to see him disappear even though he's not real.";
	mes "On behalf of him, I would like to thank you.";
	next;
	mes "[ Lucille ]";
	mes "The same goes for me who is bounded in this flow of time that doesn't exist anymore.";
	mes "I'm just probably a memory of the past too.";
	mes "But, unlike me you have a place to return to.";
	next;
	mes "[ Lucille ]";
	mes "It seems that it's time for everyone to return to reality.";
	mes "Should I guide you out?";
	next;
	if(select("Please.:Wait a bit.") == 2){
		mes "[ Lucille ]";
		mes "Please hurry up. This reality won't last any longer.";
		end;
	}
	mes "[ Lucille ]";
	mes "Alright.";
	close2;
	warp "thana_step",150,370;
	end;
}

//= Floor 2
2@thts,57,171,3	script	Watcher#mot	20800,{
	if(!is_party_leader()) end;
	.@var = get_instance_var("obs");
	if(.@var > 2) end;
	set_instance_var("obs",.@var + 1);
	switch(.@var){
		case 0:
			npctalk "Watcher : Head back. This place is not for humans like you to be in.";
			break;
			
		case 1:
			npctalk "Watcher : Do not ignore my warning.";
			break;
			
		case 2:
			npctalk "Watcher : You will regret it. I will deal with you with all my might.";
			sleep 1000;
			instance_disable(strnpcinfo(0));
			set_instance_var("type",2);
			instance_event("#mot_mob","OnSummon",false);
			break;
	}
	end;
}

//= Floor 3
3@thts,28,43,0	script	#mot_ev_3	HIDDEN_WARP_NPC,2,2,{
	end;

OnTouch:
	.@id = atoi(replacestr(strnpcinfo(2),"mot_ev_",""));
	instance_disable(strnpcinfo(0));
	set_instance_var("type",.@id);
	instance_event("#mot_mob","OnSummon",false);
	switch(.@id){
		case 3:
			instance_enable("Sealed Hatred#mot",true);
			instance_enable("mot_memory_3",true);
			instance_hide("mot_memory_3",true);
			break;

		case 4:
			instance_enable("mot_memory_4",true);
			instance_hide("mot_memory_4",true);
			break;

		case 5:
			instance_enable("mot_memory_5",true);
			instance_hide("mot_memory_5",true);
			instance_enable("Memory Piece#mot_0",true);
			instance_event("Memory Piece#mot_0","OnWalk",false);
			break;

		case 6:
			instance_enable("mot_memory_6",true);
			instance_hide("mot_memory_6",true);
			break;
	}
end;
}

3@thts,70,105,3	duplicate(dummynpc2)	Sealed Hatred#mot	20796

3@thts,70,105,3	script	Memory of Hatred::mot_memory_3	4_ENERGY_RED,{
	.@id = get_instance_var(replacestr(strnpcinfo(0),"Memory of ",""));
	if(!get_instance_var("status_" + .@id) || !is_party_leader()) end; //Party members can take this in kRO
	if(get_instance_var("status_" + .@id) == 1){
		set_instance_var("status_" + .@id,2);
		switch(.@id){
			case 3: 
				.@item = 7438; 
				mapannounce instance_mapname("3@thts"),"The memories of hatred has been shaped into a fragment. The seal on the gate has been released.",bc_map,0x00FF00;
				break;
			case 4: 
				.@item = 7436;
				mapannounce instance_mapname("4@thts"),"The memories of agony has been shaped into a fragment. The seal on the gate has been released.",bc_map,0x00FF00;
				break;

			case 5:
				.@item = 7437;
				mapannounce instance_mapname("5@thts"),"The memories of agony has been shaped into a fragment. The seal on the gate has been released.",bc_map,0x00FF00;
				break;

			case 6:
				.@item = 7439;
				mapannounce instance_mapname("5@thts"),"The memories of despair has been shaped into a fragment. The seal on the gate has been released.",bc_map,0x00FF00;
				break;
		}
		getitem .@item,1;
		instance_enable("#mot_return_" + (.@id - 2),true);
		specialeffect EF_CARTTER;
		sleep 1000;
		instance_disable("mot_memory_" + .@id);
	}
	end;

OnSummon:
	switch(get_instance_var(replacestr(strnpcinfo(0),"Memory of ",""))){
		case 3:
			instance_disable("Sealed Hatred#mot");
			mapannounce instance_mapname("3@thts"),"The seal on the center has been released. The crystals has been imbued with magical power.",bc_map,0x00FF00;
			monster instance_mapname("3@thts"),59,116,"Hatred of Thanatos",20796,1,strnpcinfo(3) + "::OnMobKill";
			for(.@i = 0; .@i < 4; .@i++)
				instance_enable("Shining Crystal#mot_" + .@i + "_" + rand(4),true);
			break;

		case 4:
			mapannounce instance_mapname("4@thts"),"A fragment of Thanatos' memory, agony has appeared.",bc_map,0x00FF00;
			monster instance_mapname("4@thts"),52,133,"Agony of Thanatos",20799,1,strnpcinfo(3) + "::OnMobKill";
			break;

		case 5:
			instance_enable("Memory Piece#mot_0",false);
			instance_enable("Memory Piece#mot_1",false);
			set_instance_var("memory_stop",1);
			mapannounce instance_mapname("5@thts"),".... They are all dead .... Then, you have to die too ....",bc_map,0xFF0000;
			mapannounce instance_mapname("5@thts"),"A fragment of Thanatos' memory, misery has appeared on the northern statue.",bc_map,0x00FF00;
			monster instance_mapname("5@thts"),129,155,"Misery of Thanatos",20798,1,strnpcinfo(3) + "::OnMobKill";
			break;

		case 6:
			mapannounce instance_mapname("6@thts"),"A fragment of Thanatos' memory, despair has appeared.",bc_map,0x00FF00;
			monster instance_mapname("6@thts"),48,55,"Despair of Thanatos",20797,1,strnpcinfo(3) + "::OnMobKill";
			break;	
	}
end;

OnMobKill:
	.@var = get_instance_var(replacestr(strnpcinfo(0),"Memory of ",""));
	instance_hide("mot_memory_" + .@var,false);
	set_instance_var("status_" + .@var,1); //Extra check
	if(.@var == 3)
		mapannounce instance_mapname("3@thts"),"The memories of hatred have taken form.",bc_map,0x00FF00;
end;
}

3@thts,90,153,0	script	Shining Crystal#mot_0_0	4_ENERGY_RED,{
	if(!is_party_leader() || get_instance_var(strnpcinfo(2))) end; //Party members can take this in kRO
	getunitdata getnpcid(0,instance_npcname(strnpcinfo(0))),.@data;
	switch(.@data[UNPC_CLASS]){
		case 4_ENERGY_RED: .@stone = 7426; break;
		case 4_ENERGY_YELLOW: .@stone = 7427; break;
		case 4_ENERGY_BLUE: .@stone = 7428; break;
		case 4_ENERGY_WHITE: .@stone = 7429; break;
	}
	set_instance_var(strnpcinfo(2),1);
	specialeffect EF_SPELLBREAKER;
	getitem .@stone,1;
	instance_disable(strnpcinfo(0));
	end;
}

3@thts,90,62,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_0_1	4_ENERGY_RED
3@thts,49,59,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_0_2	4_ENERGY_RED
3@thts,49,144,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_0_3	4_ENERGY_RED
3@thts,49,153,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_1_0	4_ENERGY_YELLOW
3@thts,90,150,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_1_1	4_ENERGY_YELLOW
3@thts,90,59,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_1_2	4_ENERGY_YELLOW
3@thts,49,56,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_1_3	4_ENERGY_YELLOW
3@thts,49,65,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_2_0	4_ENERGY_BLUE
3@thts,49,150,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_2_1	4_ENERGY_BLUE
3@thts,90,147,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_2_2	4_ENERGY_BLUE
3@thts,90,56,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_2_3	4_ENERGY_BLUE
3@thts,90,65,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_3_0	4_ENERGY_WHITE
3@thts,49,62,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_3_1	4_ENERGY_WHITE
3@thts,49,147,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_3_2	4_ENERGY_WHITE
3@thts,90,144,0	duplicate(Shining Crystal#mot_0_0)	Shining Crystal#mot_3_3	4_ENERGY_WHITE

//= Floor 4
4@thts,20,96,0	duplicate(#mot_ev_3)	#mot_ev_4	HIDDEN_WARP_NPC,2,2
4@thts,52,133,3	duplicate(mot_memory_3)	Memory of Agony::mot_memory_4	20799

//= Floor 5
5@thts,155,100,0	duplicate(#mot_ev_3)	#mot_ev_5	HIDDEN_WARP_NPC,2,2
5@thts,129,155,3	duplicate(mot_memory_3)	Memory of Misery::mot_memory_5	20798
5@thts,101,97,8	script	Memory Piece#mot_0	20798,{
	instance_enable(strnpcinfo(0),false);
	movenpc instance_npcname(strnpcinfo(0)),101,97,8;
	instance_enable(strnpcinfo(0),true);
	instance_event(strnpcinfo(0),"OnWalk",false);
	end;

OnWalk:
	npcspeed 150;
	unitwalk getnpcid(0,instance_npcname(strnpcinfo(0))),131,97,instance_npcname(strnpcinfo(0)) + "::OnContinueWalk";
end;

OnContinueWalk:
	unitwalk getnpcid(0,instance_npcname(strnpcinfo(0))),156,97,instance_npcname(strnpcinfo(0)) + "::OnEvent";
end;

OnEvent:
	instance_enable(strnpcinfo(0),false);
	movenpc instance_npcname(strnpcinfo(0)),101,97,8;
	if(get_instance_var("memory_stop")) end;
	instance_event("#mot_floor_ev","OnEvent05",false);
	mapannounce instance_mapname("5@thts"),"A memory fragment has activated the eastern statue.",bc_map,0x00FF00;
	instance_enable("Memory Piece#mot_1",true);
	instance_event("Memory Piece#mot_1","OnWalk",false);
end;
}

5@thts,156,97,2	script	Memory Piece#mot_1	20798,{
	instance_enable(strnpcinfo(0),false);
	movenpc instance_npcname(strnpcinfo(0)),156,97,2;
	instance_enable(strnpcinfo(0),true);
	instance_event(strnpcinfo(0),"OnWalk",false);
	end;

OnWalk:
	npcspeed 150;
	unitwalk getnpcid(0,instance_npcname(strnpcinfo(0))),131,97,instance_npcname(strnpcinfo(0)) + "::OnContinueWalk";
end;

OnContinueWalk:
	unitwalk getnpcid(0,instance_npcname(strnpcinfo(0))),101,97,instance_npcname(strnpcinfo(0)) + "::OnEvent";
end;

OnEvent:
	instance_enable(strnpcinfo(0),false);
	movenpc instance_npcname(strnpcinfo(0)),156,97,2;
	if(get_instance_var("memory_stop")) end;
	instance_event("#mot_floor_ev","OnEvent05",false);
	mapannounce instance_mapname("5@thts"),"A memory fragment has activated the western statue.",bc_map,0x00FF00;
	instance_enable("Memory Piece#mot_0",true);
	instance_event("Memory Piece#mot_0","OnWalk",false);
end;
}

//= Floor 6
6@thts,50,17,0	duplicate(#mot_ev_3)	#mot_ev_6	HIDDEN_WARP_NPC,2,2
6@thts,48,53,3	duplicate(mot_memory_3)	Memory of Despair::mot_memory_6	20797

//= Floor 7
7@thts,161,57,0	script	Gold Religious Statue#mot_0	4_ENERGY_WHITE,{
	.@id = atoi(replacestr(strnpcinfo(2),"mot_",""));
	.@map$ = instance_mapname("7@thts");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill";
	switch(.@id){
		case 0: 
			mapannounce .@map$,"Do you resent fate?",bc_map,0x00FF00;
			.@mob_id = 20794;
			break;

		case 1:
			mapannounce .@map$,"Do you regret that choice?",bc_map,0x00FF00;
			.@mob_id = 20795;
			break;

		case 2:
			mapannounce .@map$,"Are you afraid?",bc_map,0x00FF00;
			.@mob_id = 20793;
			break;

		case 3:
			mapannounce .@map$,"Anger, fill yourself with anger...",bc_map,0x00FF00;
			.@mob_id = 20792;
			break;

		case 4:
			if(get_instance_var("statue") < 4) end;
			instance_hide(strnpcinfo(0),true);
			mapannounce .@map$,"Dark Devil Statue : Hahaha it's nothing but a useless struggle!!!",bc_map,0x00FF00;
			set_instance_var("statue",5);
			getitem 7430,1;
			mapannounce .@map$,"The stone barriers has activated.",bc_map,0x00FF00;
			instance_enable(strnpcinfo(0),false);
			end;
	}
	instance_hide(strnpcinfo(0),true);
	areamonster .@map$,120,45,140,60,"--ja--",.@mob_id,5,.@event$;
	end;

OnMobKill:
	.@map$ = instance_mapname("7@thts");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill";
	if(!mobcount(.@map$,.@event$)){
		set_instance_var("statue",get_instance_var("statue") + 1);
		instance_enable(strnpcinfo(0),false);
	}
end;
}

7@thts,154,17,0	duplicate(Gold Religious Statue#mot_0)	Green Wiseman Statue#mot_1	4_ENERGY_WHITE
7@thts,103,17,0	duplicate(Gold Religious Statue#mot_0)	Blue Angel Statue#mot_2	4_ENERGY_WHITE
7@thts,96,57,0	duplicate(Gold Religious Statue#mot_0)	Bloody Knight Statue#mot_3	4_ENERGY_WHITE
7@thts,129,86,0	duplicate(Gold Religious Statue#mot_0)	Dark Devil Statue#mot_4	4_ENERGY_WHITE

7@thts,134,52,0	script	Stone Barrier#mot_0	CLEAR_NPC,{
	if(get_instance_var("statue") < 5) end;
	.@id = atoi(replacestr(strnpcinfo(2),"mot_",""));
	.@v = get_instance_var("barrier");
	switch(.@id){
		case 0: .@item_id = 7427; .@color$ = "Yellow"; break;
		case 1: .@item_id = 7428; .@color$ = "Blue";break;
		case 2: .@item_id = 7429; .@color$ = "Green";break;
		case 3: .@item_id = 7426; .@color$ = "Red"; break;
		case 4: .@item_id = 7430; .@color$ = "Black"; break;
	}
	if(!countitem(.@item_id)){
		mes "A hint of " + .@color$ + " light is shining on the empty slot of the stone barrier.";
		end;
	}
	delitem .@item_id,1;
	set_instance_var("barrier", .@v + 1);
	instance_disable(strnpcinfo(0));
	instance_enable("Stone Barrier#mot_" + .@id + "-2",true);
	specialeffect EF_BEGINSPELL3,AREA,instance_npcname("Stone Barrier#mot_" + .@id + "-2");
	specialeffect EF_PINKBODY,AREA,instance_npcname("Stone Barrier#mot_" + .@id + "-2");
	mapannounce instance_mapname("7@thts"),"A part of the seal has been unlocked.",bc_map,0x00FF00;
	if(.@v + 1 == 5){
		mapannounce instance_mapname("7@thts"),"All the seals have been unlocked, the gate has opened.",bc_map,0x00FF00;
		instance_enable("#mot_exit_6",true);
	}
	end;
}

7@thts,132,47,0	duplicate(Stone Barrier#mot_0)	Stone Barrier#mot_1	CLEAR_NPC
7@thts,127,47,0	duplicate(Stone Barrier#mot_0)	Stone Barrier#mot_2	CLEAR_NPC
7@thts,125,52,0	duplicate(Stone Barrier#mot_0)	Stone Barrier#mot_3	CLEAR_NPC
7@thts,129,56,0	duplicate(Stone Barrier#mot_0)	Stone Barrier#mot_4	CLEAR_NPC

7@thts,134,52,0	script	Stone Barrier#mot_0-2	4_NFWISP,{
	.@var$ = replacestr(strnpcinfo(2),"mot_","");
	.@var2$ = replacestr(.@var$,"-2","");
	switch(atoi(.@var2$)){
		case 0: .@color$ = "Yellow"; break;
		case 1: .@color$ = "Blue";break;
		case 2: .@color$ = "Green";break;
		case 3: .@color$ = "Red"; break;
		case 4: .@color$ = "Black"; break;
	}
	mes "The " + .@color$ + " magic stone melted into the stone barrier.";
	end;
}
7@thts,132,47,0	duplicate(Stone Barrier#mot_0-2)	Stone Barrier#mot_1-2	4_NFWISP
7@thts,127,47,0	duplicate(Stone Barrier#mot_0-2)	Stone Barrier#mot_2-2	4_NFWISP
7@thts,125,52,0	duplicate(Stone Barrier#mot_0-2)	Stone Barrier#mot_3-2	4_NFWISP
7@thts,129,56,0	duplicate(Stone Barrier#mot_0-2)	Stone Barrier#mot_4-2	4_NFWISP

//= Boss Floor
8@thts,135,140,0	script	Sealed Memory#mot	4_ENERGY_WHITE,{
	.@map$ = instance_mapname("8@thts");
	setarray .@var$,"b_misery","b_agony","b_despair","b_hatred";
	setarray .@names$,"misery","agony","despair","hatred";
	setarray .@id,7437,7436,7439,7438;
	for(.@i = 0; .@i < 4; .@i++){
		if(!get_instance_var(.@var$[.@i]) && countitem(.@id[.@i])){
			delitem .@id[.@i],1;
			set_instance_var(.@var$[.@i],1);
			set_instance_var("thanatos",get_instance_var("thanatos") + 1);
			npctalk "An unknown entity reacted to the shard of " + .@names$[.@i] + ".";
			mapannounce .@map$,"A part of the magic swordsman Thanatos' memories has been released.",bc_map,0x7B68EE;
			specialeffect EF_BLUELINE;
			specialeffect EF_DQ9_CHARGE3;
		}
	}
	if(get_instance_var("thanatos") == 4){
		instance_hide(strnpcinfo(0),true);
		sleep 3000;
		mapannounce .@map$,"...break.. crush.. spread out.. the tears I shed, it's all coming back..",bc_map,0xFF0000;
		specialeffect EF_DQ9_CHARGE3;
		sleep 3000;
		mapannounce .@map$,"... shattered and ravaged..... taking a deep breath...",bc_map,0xFF0000;
		specialeffect EF_DQ9_CHARGE3;
		sleep 3000;
		mapannounce .@map$,"... with a strong scent of blood..... it becomes a dagger that's stuck in your heart...",bc_map,0xFF0000;
		specialeffect EF_DQ9_CHARGE3;
		sleep 3000;
		mapannounce .@map$,".....those who want to invade this land... I will... destroy.....!!",bc_map,0xFF0000;
		specialeffect EF_DQ9_CHARGE3;
		sleep 3000;
		mapannounce .@map$,"...I.. ... Thanatos! The devil... will destroy you all!!!",bc_map,0xFF0000;
		specialeffect EF_DQ9_CHARGE3;
		instance_event("#mot_boss","OnSummon",false);
	}
	end;
}

8@thts,1,1,0	script	#mot_boss	HIDDEN_WARP_NPC,1,1,{
	end;

OnSummon:
	.@map$ = instance_mapname("8@thts");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnBossKill";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnDummyKill";
	set_instance_var("shield",7);
	monster .@map$,136,140,"Thanatos",20784,1,.@event$;
	set_instance_var("gid",$@mobid[0]);
	unitskilluseid $@mobid[0],"NPC_RELIEVE_ON",7;
	instance_event(strnpcinfo(0),"OnSummonBuff",false);
	setarray .@xy,
	133,143,20792,131,145,20792,133,146,20792,134,145,20792,139,143,20793,141,167,20793,141,177,20793,141,145,20793,
	139,137,20795,141,135,20795,141,173,20795,142,159,20795,131,135,20794,133,137,20794,137,140,20794,138,136,20794;
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 3)
		monster .@map$,.@xy[.@i],.@xy[.@i+1],"--ja--",.@xy[.@i+2],1,.@event2$;
	instance_event(strnpcinfo(0),"OnBossSkill",false);
	initnpctimer;
end;

OnBossSkill:
	.@map$ = instance_mapname("8@thts");
	.@gid = get_instance_var("gid");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnSkillDead";
	while(unitexists(.@gid)){
		sleep 6900;
		if(!unitexists(.@gid)) end;
		getunitdata .@gid,.@data;
		.@x = .@data[UMOB_X];
		.@y = .@data[UMOB_Y];
		monster .@map$,.@x,.@y,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		sleep 200;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x + 5,.@y,-10000;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x - 5,.@y,-10000;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x,.@y - 5,-10000;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x,.@y + 5,-10000;
		sleep 200;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x + 5,.@y + 5,-10000;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x - 5,.@y + 5,-10000;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x - 5,.@y - 5,-10000;
		unitskillusepos $@mobid[0],"NPC_GROUNDDRIVE",5,.@x + 5,.@y - 5,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 5,.@y,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 5,.@y,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x,.@y - 5,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x,.@y + 5,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 5,.@y + 5,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 5,.@y + 5,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 5,.@y - 5,-10000;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 5,.@y - 5,-10000;
		sleep 100;
		monster .@map$,.@x + 10,.@y,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 10,.@y,-10000;
		monster .@map$,.@x - 10,.@y,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 10,.@y,-10000;
		monster .@map$,.@x,.@y - 10,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x,.@y - 10,-10000;
		monster .@map$,.@x,.@y + 10,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x,.@y + 10,-10000;
		monster .@map$,.@x + 10,.@y + 10,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 10,.@y + 10,-10000;
		monster .@map$,.@x - 10,.@y + 10,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 10,.@y + 10,-10000;
		monster .@map$,.@x - 10,.@y - 10,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 10,.@y - 10,-10000;
		monster .@map$,.@x + 10,.@y - 10,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 10,.@y - 10,-10000;
		sleep 100;
		monster .@map$,.@x + 15,.@y,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 15,.@y,-10000;
		monster .@map$,.@x - 15,.@y,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 15,.@y,-10000;
		monster .@map$,.@x,.@y - 15,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x,.@y - 15,-10000;
		monster .@map$,.@x,.@y + 15,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x,.@y + 15,-10000;
		monster .@map$,.@x + 15,.@y + 15,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 15,.@y + 15,-10000;
		monster .@map$,.@x - 15,.@y + 15,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 15,.@y + 15,-10000;
		monster .@map$,.@x - 15,.@y - 15,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x - 15,.@y - 15,-10000;
		monster .@map$,.@x + 15,.@y - 15,"",20562,1,.@event$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"MG_THUNDERSTORM",5,.@x + 15,.@y - 15,-10000;
		sleep 100;
		killmonster .@map$,.@event$;
	}
OnSkillDead:
end;

OnTimer120000:
	stopnpctimer;
	.@map$ = instance_mapname("8@thts");
	.@gid = get_instance_var("gid");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnBossKill";
	if(!unitexists(.@gid)) end;
	getunitdata .@gid,.@data;
	.@DAMAGE = (.@data[UMOB_MAXHP]/10) * 5; //20% HP
	if(.@data[UMOB_HP] < .@DAMAGE){
		if(!unitexists(.@gid)) end;
		mapannounce .@map$,"I will make you disappear with the hell beings.",bc_map,0xFF0000;
		sleep 2000;
		if(!unitexists(.@gid)) end;
		unittalk .@gid,"... Ugh ..., to think that you've driven me this far...";
		sleep 3000;
		if(!unitexists(.@gid)) end;
		mapannounce .@map$,"I will never give in!!!",bc_map,0xFF0000;
		unitskilluseid .@gid,"NPC_EARTHQUAKE",5,.@gid,-10000;
		set_instance_var("broken",1);
		unitkill .@gid;
		monster  .@map$,.@data[UMOB_X],.@data[UMOB_Y],"Broken Thanatos",20785,1,.@event$;
		set_instance_var("gid",$@mobid[0]);
	} else
		initnpctimer;
end;

OnBossKill:
	if(get_instance_var("broken")){
		set_instance_var("broken",0);
		end;
	}
	.@map$ = instance_mapname("8@thts");
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnDummyKill";
	stopnpctimer;
	killmonster .@map$,.@event2$;
	instance_enable("Lucille#motend",true);
	instance_enable(strnpcinfo(0),false);
	mapannounce .@map$,"...I.. won't... be able... to protect this place...",bc_map,0xFF0000;
end;

OnSummonBuff:
	.@map$ = instance_mapname("8@thts");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnBuffKill";
	setcell .@map$,168,152,168,156,CELL_WALKABLE,1;
	setcell .@map$,168,152,168,156,CELL_SHOOTABLE,1;
	setcell .@map$,161,113,163,117,CELL_WALKABLE,1;
	setcell .@map$,161,113,163,117,CELL_SHOOTABLE,1;
	setcell .@map$,110,112,112,117,CELL_WALKABLE,1;
	setcell .@map$,110,112,112,117,CELL_SHOOTABLE,1;
	setcell .@map$,104,155,105,159,CELL_WALKABLE,1;
	setcell .@map$,104,155,105,159,CELL_SHOOTABLE,1;
	setcell .@map$,138,174,143,175,CELL_WALKABLE,1;
	setcell .@map$,138,174,143,175,CELL_SHOOTABLE,1;
	setarray .@names$[1],"Agony","Anger","Despair","Hatred","Misery";
	for(.@i = 1; .@i < 6; .@i++){
		.@var = rand(1,5);
		while(inarray(.@idx,.@var) != -1)
			.@var = rand(1,5);
		.@idx[getarraysize(.@idx)] = .@var;
		set_instance_var("names_" + .@i + "$",.@names$[.@var]);
	}
	for(.@i = 0; .@i < 3; .@i++){
		.@var = rand(1,5);
		while(inarray(.@buff,.@var) != -1)
			.@var = rand(1,5);
		.@buff[getarraysize(.@buff)] = .@var;
		set_instance_var("buff_" + .@var,1);
		mapannounce .@map$,"Thanatos is protected by " + get_instance_var("names_" + .@var + "$") + ".",bc_map,0x00FF00;
	}
	.@z = 1;
	setarray .@xy,90,86,68,168,141,214,205,162,191,86;
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 2){
		monster .@map$,.@xy[.@i],.@xy[.@i+1],get_instance_var("names_" + .@z + "$"),20786,1,.@event$;
		.@z++;
	}
end;

OnBuffKill:
	.@map$ = instance_mapname("8@thts");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnBuffKill";
	.@gid = get_instance_var("gid");
	getunitdata killedgid,.@data;
	switch(.@data[UMOB_X]){
		case 90: .@x = 1; break;
		case 68: .@x = 2; break;
		case 141: .@x = 3; break;
		case 205: .@x = 4; break;
		case 191: .@x = 5; break;
	}
	if(get_instance_var("buff_" + .@x)){
		set_instance_var("shield",get_instance_var("shield") - 2);
		set_instance_var("protection",get_instance_var("protection") + 1);
		mapannounce .@map$,"The guardian memory " + get_instance_var("names_" + .@x + "$") +  " is destroyed. Removing the protection of Thanatos' Memory.",bc_map,0x00FF00;
		unitskilluseid .@gid,"NPC_RELIEVE_OFF",1;
		if(get_instance_var("protection") == 3){
			killmonster .@map$,.@event$;
			mapannounce .@map$,"A part of the guardian memory is destroyed, and the rest of the guardians disappeared. The rest of the guardian memories protection remains.",bc_map,0x00FF00;
		} else
			unitskilluseid .@gid,"NPC_RELIEVE_ON",get_instance_var("shield");
	} else {
		killmonster .@map$,.@event$;
		mapannounce .@map$,"A part of the guardian memory is destroyed, and the rest of the guardians disappeared. The rest of the guardian memories protection remains.",bc_map,0x00FF00;
		unitskilluseid .@gid,"NPC_RELIEVE_OFF",1;
		unitskilluseid .@gid,"NPC_RELIEVE_ON",get_instance_var("shield");
	}
OnDummyKill:
end;
}

1@thts,1,1,0	script	#mot_mob	HIDDEN_WARP_NPC,1,1,{
	end;
	
OnSummon:
	.@type = get_instance_var("type");
	.@map$ = instance_mapname(.@type + "@thts");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill";
	switch(.@type){
		case 2:
			setarray .@xy,
			57,171,20800,49,173,20800,49,171,20800,60,162,20800,60,160,20800,58,163,20800,61,177,20800,60,145,20800,57,144,20800,47,171,20800,62,181,20800,60,181,20800,
			64,143,20800,60,117,20800,61,114,20800,58,111,20800,36,123,20800,34,119,20800,33,123,20800,32,92,20800,33,92,20800,32,89,20800,40,68,20800,37,64,20800,33,63,20800,
			37,40,20800,35,43,20800,35,41,20800,92,41,20800,93,38,20800,97,36,20800,102,64,20800,105,64,20800,81,92,20800,80,95,20800,81,96,20800,103,81,20800,106,79,20800,
			107,78,20800,100,125,20800,102,125,20800,103,129,20800,84,147,20800,82,142,20800,80,142,20800,95,165,20800,96,164,20800,97,168,20800;
			break;

		case 3:
			setarray .@xy,
			35,117,20787,35,120,20787,33,121,20787,36,150,20787,33,151,20787,37,151,20787,34,158,20787,32,160,20787,54,172,20787,55,172,20787,57,178,20787,57,172,20787,57,171,20787,62,150,20787,
			63,150,20787,63,148,20787,102,55,20787,102,54,20787,103,64,20787,103,95,20787,101,94,20787,103,93,20787,79,86,20787,75,89,20787,76,85,20787,78,84,20787,79,80,20787,79,66,20787,
			77,65,20787,77,65,20787,95,59,20787,97,59,20787,55,32,20788,37,65,20788,36,65,20788,38,66,20788,35,89,20788,34,92,20788,33,92,20788,29,95,20788,40,96,20788,61,61,20788,57,33,20788,
			50,31,20788,49,30,20788,50,30,20788,64,63,20788,62,63,20788,76,154,20788,77,153,20788,82,178,20788,83,178,20788,83,180,20788,99,114,20788,75,153,20788,105,157,20788,107,159,20788,
			105,156,20788,106,154,20788,108,149,20788,104,124,20788,102,121,20788,104,121,20788,100,114,20788;
			break;
			
		case 4:
			setarray .@xy,
			29,97,20790,23,102,20790,28,110,20790,28,97,20790,27,102,20790,29,107,20790,27,98,20790,35,96,20790,33,111,20790,38,106,20790,49,123,20790,
			55,122,20790,49,128,20790,52,129,20790,53,129,20790,67,140,20790,66,147,20790,75,142,20790,72,151,20790,76,151,20790,82,141,20790,83,139,20790,
			66,154,20790,78,154,20790,79,117,20790,83,116,20790,77,115,20790,80,114,20790,78,112,20790,83,156,20790,36,143,20790,38,141,20790,29,140,20790,
			33,143,20790,31,145,20790,35,152,20790,36,153,20790,31,149,20790,28,148,20790,30,156,20790,27,153,20790,23,139,20790;
			instance_event("#mot_floor_ev","OnEvent04",false);
			break;

		case 5:
			setarray .@xy,
			99,108,20791,109,108,20791,109,109,20791,105,105,20791,104,111,20791,101,102,20791,105,114,20791,108,128,20791,107,128,20791,106,128,20791,105,129,20791,105,130,20791,109,131,
			20791,107,134,20791,134,119,20791,138,124,20791,138,120,20791,141,120,20791,142,122,20791,145,119,20791,151,136,20791,152,132,20791,153,136,20791,153,141,20791,153,136,20791,155,130,20791,
			157,130,20791,150,157,20791,152,158,20791,149,159,20791,153,159,20791,149,160,20791,133,152,20791,133,153,20791,133,153,20791,129,153,20791,125,155,20791,140,118,20791;
			break;

		case 6:
			setarray .@xy,
			29,18,20789,29,19,20789,27,22,20789,23,30,20789,24,29,20789,23,23,20789,24,16,20789,27,34,20789,72,19,20789,27,35,20789,45,54,20789,45,54,20789,46,56,20789,55,42,20789,55,42,20789,53,42,20789,
			65,40,20789,67,43,20789,67,43,20789,76,25,20789,78,22,20789,76,21,20789,81,26,20789,69,70,20789,71,74,20789,71,72,20789,72,77,20789,72,82,20789,75,73,20789,77,80,20789,46,68,20789,47,68,20789,
			50,69,20789,30,79,20789,30,82,20789,28,84,20789,24,77,20789,23,77,20789,21,79,20789,21,85,20789;
			instance_event("#mot_floor_ev","OnEvent06",false);
			break;
	}
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 3)
		monster .@map$,.@xy[.@i],.@xy[.@i+1],"--ja--",.@xy[.@i+2],1,.@event$;
end;

OnMobKill:
	.@type = get_instance_var("type");
	.@map$ = instance_mapname(.@type + "@thts");
	.@event$ = instance_npcname(strnpcinfo(0)) + "::OnMobKill";
	if(!mobcount(.@map$,.@event$))
		instance_event(strnpcinfo(0),"OnEvent0" + .@type,false);
end;

OnEvent02:
	mapannounce instance_mapname("2@thts"),"The seal of the gate has been released.",bc_map,0x00FF00;
	instance_enable("#mot_return_0",true);
end;

OnEvent03:
	instance_event("mot_memory_3","OnSummon",false);
end;

OnEvent04:
	instance_event("mot_memory_4","OnSummon",false);
end;

OnEvent05:
	instance_event("mot_memory_5","OnSummon",false);
end;

OnEvent06:
	instance_event("mot_memory_6","OnSummon",false);
end;
}

1@thts,1,1,0	script	#mot_floor_ev	HIDDEN_WARP_NPC,{
	end;

OnEvent04:
	sleep 3000;
	.@map$ = instance_mapname("4@thts");
	.@event$ = instance_npcname("#mot_mob") + "::OnMobKill";
	if(!mobcount(.@map$,.@event$)) end;
	mapannounce .@map$,"The stone statues has called the flames..",bc_map,0x00FF00;
	instance_event(strnpcinfo(0),"OnFire",false);
end;

OnFire:
	.@map$ = instance_mapname("4@thts");
	.@event$ = instance_npcname("#mot_mob") + "::OnMobKill";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnFireDead";
	function Firewall;
	.@x = 20;
	for(.@y = 154; .@y > 116; .@y--){
		if(!mobcount(.@map$,.@event$)) break;
		Firewall(.@x,.@y+1,.@x,.@y,.@event2$);
		Firewall(.@x+3,.@y+1,.@x+3,.@y,.@event2$);
		Firewall(.@x+6,.@y+1,.@x+6,.@y,.@event2$);
		.@x++;
		sleep 100;
	}
	sleep 5000;
	instance_event(strnpcinfo(0),"OnFire2",false);
	sleep 1000;
	killmonster .@map$,.@event2$;
OnFireDead:
end;

OnFire2:
	.@map$ = instance_mapname("4@thts");
	.@event$ = instance_npcname("#mot_mob") + "::OnMobKill";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnFireDead2";
	function Firewall;
	.@y = 161;
	for(.@x = 84; .@x > 16; .@x--){
		if(!mobcount(.@map$,.@event$)) break;
		Firewall(.@x+1,.@y,.@x,.@y,.@event2$);
		.@y--;
		sleep 100;
	}
	sleep 5000;
	instance_event(strnpcinfo(0),"OnFire3",false);
	sleep 1000;
	killmonster .@map$,.@event2$;
OnFireDead2:
end;

OnFire3:
	.@map$ = instance_mapname("4@thts");
	.@event$ = instance_npcname("#mot_mob") + "::OnMobKill";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnFireDead3";
	function Firewall;
	.@y = 161;
	for(.@x = 84; .@x > 16; .@x--){
		if(!mobcount(.@map$,.@event$)) break;
		Firewall(.@x+1,.@y,.@x,.@y,.@event2$);
		.@y--;
		sleep 100;
	}
	sleep 5000;
	instance_event(strnpcinfo(0),"OnFire4",false);
	sleep 1000;
	killmonster .@map$,.@event2$;
OnFireDead3:
end;

OnFire4:
	.@map$ = instance_mapname("4@thts");
	.@event$ = instance_npcname("#mot_mob") + "::OnMobKill";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnFireDead4";
	.@x = 80;
	for(.@y = 104; .@y < 165; .@y++){
		if(!mobcount(.@map$,.@event$)) break;
		Firewall(.@x,.@y,.@x,.@y+1,.@event2$);
		Firewall(.@x+3,.@y,.@x+3,.@y+1,.@event2$);
		sleep 100;
	}
	sleep 5000;
	instance_event(strnpcinfo(0),"OnFire",false);
	sleep 1000;
	killmonster .@map$,.@event2$;
OnFireDead4:
end;

function	Firewall	{
	.@map$ = instance_mapname("4@thts");
	monster .@map$,getarg(0),getarg(1),"",20562,1,getarg(4);
	setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
	unitskillusepos $@mobid[0],"MG_FIREWALL",1,getarg(2),getarg(3),-10000;
	unitskilluseid $@mobid[0],"NPC_INVISIBLE",1,-10000;
	return;
}

OnEvent05:
	.@map$ = instance_mapname("5@thts");
	.@event$ = instance_npcname("#mot_mob") + "::OnMobKill";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnEventDead";
	if(mobcount(.@map$,.@event2$)) killmonster .@map$,.@event2$;
	setarray .@xy,
	120,97,130,97,140,97,150,97, //First Horizontal
	106,105,106,115,106,125, //First Vertical
	116,125,126,125,136,125,146,125, //Second Horizontal
	154,138,138,153,128,153,118,153,108,153; //Diagonal/Final Horizontal
	for(.@i = 0; .@i < getarraysize(.@xy); .@i += 2){
		if(!mobcount(.@map$,.@event$)) break;
		monster .@map$,.@xy[.@i],.@xy[.@i+1],"",20562,1,.@event2$;
		setunitdata $@mobid[0],UMOB_DMGIMMUNE,true;
		unitskillusepos $@mobid[0],"WZ_STORMGUST",10,.@xy[.@i],.@xy[.@i+1],-10000;
		unitskilluseid $@mobid[0],"NPC_INVISIBLE",1,-10000;
	}
	sleep 10000;
	if(mobcount(.@map$,.@event2$)) killmonster .@map$,.@event2$;
end;

OnEvent06:
	.@map$ = instance_mapname("6@thts");
	.@event$ = instance_npcname("#mot_mob") + "::OnMobKill";
	.@event2$ = instance_npcname(strnpcinfo(0)) + "::OnEventDead";
	monster .@map$,38,18,"",20562,1,.@event2$;
	.@gid = $@mobid[0];
	monster .@map$,23,20,"",20562,1,.@event2$;
	.@gid2 = $@mobid[0];
	sleep 5000;
	while(mobcount(.@map$,.@event$)){ // Repeating until the floor is clear
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"The ground shook that day.",bc_map,0x00FF00;
		unitskilluseid .@gid,"NPC_EARTHQUAKE",5,.@gid,-10000;
		unitskilluseid .@gid2,"NPC_EARTHQUAKE",5,.@gid2,-10000;
		sleep 5000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"The feeling that you're trying to hold out, but had no place to stand in the midst of endless shaking.",bc_map,0x00FF00;
		unitskilluseid .@gid,"NPC_EARTHQUAKE",5,.@gid,-10000;
		unitskilluseid .@gid2,"NPC_EARTHQUAKE",5,.@gid2,-10000;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"It was terrible.",bc_map,0x00FF00;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"Despair comes so unexpectedly.",bc_map,0x00FF00;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"It came without even giving me a place to stand up.",bc_map,0x00FF00;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"It takes all the hope from walking beings.",bc_map,0x00FF00;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"I looked at the sky turning black. I was in despair ",bc_map,0x00FF00;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"nevertheless, I stood on the ground, firmly supporting both of my feet, breathing for hope.",bc_map,0x00FF00;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
		mapannounce .@map$,"But there was no hope.",bc_map,0x00FF00;
		sleep 10000;
		if(!mobcount(.@map$,.@event$)) break;
	}
	killmonster .@map$,.@event2$;
end;

OnEventDead:
end;
}

2@thts,113,166,0	script	#mot_return_0	WARPNPC,1,1,{
	end;
	
OnTouch:
	.@id = atoi(replacestr(strnpcinfo(2),"mot_return_",""));
	switch(.@id){
		case 0: .@x = 69; .@y = 287; break; //2@thts
		case 1: .@x = 30; .@y = 223; break; //3@thts
		case 2: .@x = 180; .@y = 223; break; //4@thts
		case 3: .@x = 14; .@y = 73; break; //5@thts
		case 4: .@x = 180; .@y = 73; break; //6@thts
	}
	warp instance_mapname("1@thts"),.@x,.@y;
end;
}

3@thts,113,143,0	duplicate(#mot_return_0)	#mot_return_1	WARPNPC,1,1
4@thts,92,146,0	duplicate(#mot_return_0)	#mot_return_2	WARPNPC,1,1
5@thts,170,138,0	duplicate(#mot_return_0)	#mot_return_3	WARPNPC,1,1
6@thts,92,36,0	duplicate(#mot_return_0)	#mot_return_4	WARPNPC,1,1

1@thts,173,372,0	script	#mot_exit_0	WARPNPC,1,1,{ //1@thts first exit
	end;
	
OnTouch:
	.@id = atoi(replacestr(strnpcinfo(2),"mot_exit_",""));
	.@z = get_instance_var("exit_" + .@id);
	switch(.@z){
		case 2: .@x = 30; .@y = 166; break;
		case 3: .@x = 28; .@y = 43; break;
		case 4: .@x = 20; .@y = 96; break;
		case 5: .@x = 155; .@y = 100; break;
		case 6: .@x = 50; .@y = 17; break;
		case 7: .@x = 115; .@y = 16; break;
	}
	warp instance_mapname(.@z + "@thts"),.@x,.@y;
end;
}

1@thts,174,288,0	duplicate(#mot_exit_0)	#mot_exit_1	WARPNPC,1,1 //exit after 2@thts
1@thts,32,166,0	duplicate(#mot_exit_0)	#mot_exit_2	WARPNPC,1,1  //exit after 3@thts
1@thts,182,166,0	duplicate(#mot_exit_0)	#mot_exit_3	WARPNPC,1,1 //exit after 4@thts
1@thts,15,15,0	duplicate(#mot_exit_0)	#mot_exit_4	WARPNPC,1,1 //exit after 5@thts
1@thts,181,15,0	duplicate(#mot_exit_0)	#mot_exit_5	WARPNPC,1,1 //exit after 6@thts

7@thts,130,52,0	script	#mot_exit_6	WARPNPC,1,1,{
	end;

OnTouch:
	warp instance_mapname("8@thts"),136,118;
end;
}