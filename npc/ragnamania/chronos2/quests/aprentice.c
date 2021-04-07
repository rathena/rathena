new_1-1,148,112,1	script	nq_portal#1	45,2,2,{
OnTouch:
OnTouch_:
	if(instance_id(IM_CHAR) > 0) {
		instance_destroy instance_id(IM_CHAR);
		sleep2 500;
	}
		
	.@id = instance_create("Welcome",IM_CHAR,getcharid(0));
	switch(.@id) {
		case -1:
			debugmes "Invalid type.";
			break;
		case -2: 
			debugmes "Character/Party/Guild/Clan not found.";
			break;
		case -3: 
			debugmes "Instance already exists.";
			break;
		case -4: 
			debugmes "No free instances (MAX_INSTANCE exceeded).";
			break;
		default:
			debugmes "Instance created : " + instance_id();
			break;
	}

	.@id = instance_enter("Welcome",100,9,getcharid(0),instance_id(IM_CHAR));
	switch(.@id) {
		case IE_NOMEMBER:
			debugmes "Party/Guild/Clan not found (for party/guild/clan modes).";
			break;
		case IE_NOINSTANCE:
			debugmes "Character/Party/Guild/Clan does not have an instance.";
			break;
		case IE_OTHER:
			debugmes "Other errors (invalid instance name, instance doesn't match with character/party/guild/clan).";
			break;
		default:
			debugmes strcharinfo(0) + " has entereded the instance.";
			break;
	}
	end;
 }


// new_1-1,144,119,3	script	Lazy Cat	554,{

// 	// if(instance_id(IM_CHAR)){
// 	// 	//instance_enter("Welcome",100,9,getcharid(0),instence_id(IM_CHAR));
// 	// 	instance_destroy;
// 	// 	end;
// 	// }
// 	instance_destroy;
// 	//INTRO_QUEST = 0;
// 	#INTRO_QUEST = 0;
// 	debugmes "Instance ID : " + instance_id();
// 	end;
//  }


// --------------- PRIMEIRA SALA ------------------

new_1-2,102,12,3	script	Stuart#room1	2073,5,5,{
end;

OnTouch:
OnTouch_:
	if('INTRO_QUEST > 0)
		end;

	if('INTRO_QUEST == 0)
		'INTRO_QUEST = 1;

	npcspeed 200;
	sleep2 1000;
	npctalk "Oh, hello, " + strcharinfo(0) + " and welcome!";
	sleep2 3000;
	npctalk "I am here to introduce you to the basics of Ragnamania Chronos,";
	sleep2 4000;
	npctalk "Please follow me...";
	sleep2 1000;
	npcwalkto 101,28;
	sleep2 3000;
	npctalk "Some of our most honorable staff await for us in this next room.";
	sleep2 3000;
	npctalk "Please come with me and I will introduce you to them.";
	sleep2 2000;
	npcwalkto 99,30;
	sleep2 1000;
	'INTRO_QUEST = 2;
	//donpcevent "nq_portal#2::OnContinue";
	disablenpc instance_npcname(strnpcinfo(0));
	end;
 }

new_1-2,99,31,1	script	nq_portal#2	45,2,2,{
 end;

OnTouch:
OnTouch_:
	if('INTRO_QUEST < 2){
		end;
	} else {
		instance_warpall "new_1-2",99,72,instance_id();
		end;
	}
	end;
 }


 // --------------- SEGUNDA SALA ------------------

new_1-2,102,75,3	script	Stuart#room2	2073,5,5,{
end;

OnTouch:
OnTouch_:
	if('INTRO_QUEST > 2)
		end;
	else
		'INTRO_QUEST = 3;

	npcspeed 200;
	sleep2 1000;
	npctalk "Here, please, come... come...";
	sleep2 1000;
	npcwalkto 97,85;
	sleep2 2000;
	npctalk "So, tell me, "+strcharinfo(0)+" are you excited?";
	sleep2 4000;
	npctalk "I am always very excited! A new friend in town... I bet you are gonna love Ragnamania Chronos!";
	npcwalkto 96,108;
	sleep2 5000;
	npctalk "Here we are! This is our good friend Arthur.";
	sleep2 4000;
	npctalk "He's gonna help you out from here.";
	sleep2 4000;
	npctalk "Uh! A new friend has just arrived! How exciting!!";
	sleep2 4000;
	npctalk "I need to go back to the entrance now, please excuse me!";
	sleep2 5000;
	npctalk "Enjoy your stay "+strcharinfo(0)+"! Hope to see you again soon!";
	sleep2 5000;
	npctalk "Bye now!";
	sleep2 2000;
	'INTRO_QUEST = 4;
	npcwalkto 97,85;
	disablenpc instance_npcname(strnpcinfo(0));
	end;
 }

new_1-2,99,113,4	script	Arthur#room2	405,{
	if('INTRO_QUEST < 4) {
		mes "^0000FF[ Arthur ]^000000";
		mes "Just one minute please...";
		close;
	}
	mes "^0000FF[ Arthur ]^000000";
	if(countitem(24501) < 1) {
	//if('INTRO_QUEST < 5) {
		mes "... Smart bird, isn't he?";
		mes "he he";
		mes "Hello, "+strcharinfo(0)+"! It is a pleasure to have you here.";
		next;
		mes "^0000FF[ Arthur ]^000000";
	
		if(#INTRO_QUEST == 0) {
			mes "I can see this is your first time here so we are gonna do the full tour, ok?";
			next;
			mes "^0000FF[ Arthur ]^000000";
			mes "Before anything else, please accept this...";
			next;
			mes "^0000FF[ Arthur ]^000000";
			mes "It is a very unique kind of Logbook here in Ragnamania.";
			mes "A must-have kind of Logbook.";
			getitem 24501,1;
			next;
			mes "^0000FF[ Arthur ]^000000";
			mes "Go ahead, check it out... it is in your inventory.";
			mes "Come back to me when you are finished.";
			mes "^0000FFDouble-Click the ^5500AA"+getitemname(24501)+"^0000FF in your inventory^000000";
		//	'INTRO_QUEST = 5;
			close;
		} else {
			mes "Hello mate! A new char, huh?";
			mes "Good for you!";
			mes "A "+getitemname(24501)+", right?";
			next;
			mes "^0000FF[ Arthur ]^000000";
			mes "Here you go!";
			getitem 24501,1;
			next;
			mes "^0000FF[ Arthur ]^000000";
			mes "If you want to skip the tutorial, please head to ^DD00DDSweetie^000000 and she will take you straight to ^0000FFProntera^000000.";
			close;
		}
	} else if(countitem(24501) > 0) {
		mes "Great! How did you like it?";
		next;
		mes "^0000FF[ Arthur ]^000000";
		mes "We don't call it "+getitemname(24501)+ " in vain";
		mes "Trust me! it will be very helpful so keep it safe!";
		next;
		mes "^0000FF[ Arthur ]^000000";
		mes "From it you can access many of our services including ^f5b041Premium^000000 services (aka VIP services).";
		mes "Oh! That reminds me...";
		next;
		mes "^0000FF[ Arthur ]^000000";
		mes "As a new player you'll get ^f5b0415 Days of Premium^000000!";
		mes "That is great isn't that?";
		next;
		mes "^0000FF[ Arthur ]^000000";
		mes "With a ^f5b041Premium^000000 (aka Premmy) life is much easier around here.";
		next;
		mes "^0000FF[ Arthur ]^000000";
		mes "But for more about that I am gonna ask you to talk to my friend on the right.";
		mes "She is a representative of the Premium Central.";
		next;
		mes "^0000FF[ Arthur ]^000000";
		mes "Talk to her and claim your ^f5b0415 Days of Premium^000000!";
		next;
		mes "^0000FF[ Arthur ]^000000";
		mes "Well, "+strcharinfo(0)+", I guess this is all I got for you at the moment.";
		mes "Wish you have a great time here at Ragnamania Chronos!";
		'INTRO_QUEST = 6;
		close;
	} 
 }

new_1-2,115,115,4	script	Lilian#new	4_F_KAFRA1,{
	if('INTRO_QUEST < 6) {
		mes .n$;
		mes "Just one minute please...";
		close;
	}

	mes .n$;
	mes "Good day, "+strcharinfo(0)+"!";
	mes "How are things going, so far?";
	mes "I bet you are counting down time to dig into your adventure, aren't you?";
	next;
	mes .n$;
	mes "I am sure you have heard about Ragnamania's Reward System...";
	next;
	mes .n$;
	mes "aka ^f5b041Premium^000000 Account or Premmy for short.";
	next;
	mes .n$;
	mes "Well, we, the Premium Central, are responsible to maintain that marvelous system functional and appealing to every player.";
	next;
	while(.@op != 3){
		mes .n$;
		mes "Please let me know how I could be of your help?";
		switch(.@op = select("What is a ^f5b041Premium^000000 Account?:How do I become ^f5b041Premium^000000?:I am fine for now")){
			case 1:
				next;
				mes .n$;
				mes "^f5b041Premium^000000 account is a temporary boost you get which grants you access to a couple of benefits...";
				next;
				mes .n$;
				mes "The most common are the ^00DD00full heal^000000, ^0000DDbuffs^000000 and ^DDAAFFrepair^000000 in our friend ^DD00DDSweetie^000000.";
				next;
				mes .n$;
				mes "But one also gets access to a special section in the "+getitemname(24501)+" with a teleporter, shops and the blacksmith services.";
				next;
				mes .n$;
				mes "It also grants temporary access to a list of special commands.";
				mes " ";
				mes "^0000FFFor the complete list of benefits, please refer to our website.^000000";
				next;
				break;
			case 2:
				next;
				mes .n$;
				mes "Ragnamania depends on donations to keep itself running, and retributes its players with Mania$ on each donation they make.";
				next;
				mes .n$;
				mes "For every US Dollar donated you get 1000 Mania$.";
				next;
				mes .n$;
				mes "These Mania$ are put into your account the moment you login ingame after the donation's got confirmed. It is automatic and you will receive a message confirming the transaction went through.";
				next;
				mes .n$;
				mes "With your Mania$ you can buy things from the ^0000AAMania Shop^000000 and also ativate your ^f5b041Premium^000000 account.";
				next;
				mes .n$;
				mes "At the present, you can activate/extend your ^f5b041Premium^000000 Account by 10 days or 30 days";
				next;
				mes .n$;
				mes "You can also activate/extend your ^f5b041Premium^000000 Account using "+getitemname(675)+", a special coin you get ingame by playing Hunting Quests among a bunch of other things.";
				next;
				mes .n$;
				mes "You can find more about Hunting Quests talking to my colleague, Sophie.";
				next;
				mes .n$;
				mes "As I was saying... With the Mania$ or the "+getitemname(675)+" in hand, you can activate/extend your ^f5b041Premium^000000 Account via our representative in (the north of) Prontera or in your "+getitemname(24501)+".";
				next;
				mes .n$;
				mes "Ah! I almost forgot!!";
				next;
				mes .n$;
				mes "Every time you activate/extend your ^f5b041Premium^000000 Account by 30 days (and 30 days only!) you will get an extra bonus: A ^AA00AA"+ getitemname(30001)+"^000000 containing 30x "+ getitemname(30000)+", a special scroll that boost your rates of EXP and Drops for a period of time. That is amazing isn't that?";
				next;
				mes .n$;
				mes "I believe this is it....";
				mes " ";
				mes "^0000FFFor more information on our Rewards System, ^f5b041Premium^000000, Mania$, etc, please refer to our website!^000000";
				next;
				break;
			case 3:
			default:
				next;
				mes .n$;
				mes "Perfect! Wish you all the best, "+strcharinfo(0)+"!";
				next;
				break;
		}
	}
	if(#FREE_PREMMY == 0 && checkquest(64505) < 0) {
		mes .n$;
		mes "Ah! I almost forgot! I am sure you've heard we offer 5 days worth of ^f5b041Premium^000000 Account for newcomers, right?";
		next;
		mes .n$;
		mes "Please bear with me as I setup that for you.... It won't take a second!";
		next;
		setquest 64505;
		mes .n$;
		mes "Sorry, I am a litle busy here... Please meet me in Prontera when you are done here and I will give you your 5 days of free ^f5b041Premium^000000.";
		next;
		mes .n$;
		mes "I've included a note in your quest log so you won't forget. See you there!";
		close2;
	} else {
		mes .n$;
		mes "Thank you very much!";
		close2;
	}
	'INTRO_QUEST = 7;
	end;
	
OnInit:
	.n$ = "^FF7F00[ Lilian ]^000000";
	end;
}


new_1-2,120,101,3	script	Sophie#new	894,{
	if('INTRO_QUEST < 7){
		mes .n$;
		mes "Just one minute please...";
		close;
	}
	mes .n$;
	mes "Oh, hi "+strcharinfo(0)+"!";
	mes "How are you going? Bet you must be tired of so much bla bla bla, aren't you? hi hi";
	next;
	mes .n$;
	mes "I promise I will keep this as simple as it can be.";
	mes " ";
	mes "So, "+getitemname(675)+", right?";
	next;
	mes .n$;
	mes "Here at Ragnamania Chronos these coins are worth quite a lot of zeny!";
	next;
	mes .n$;
	mes "And that is because there is just so much you can do with them!";
	next;
	mes .n$;
	mes "Well, and also because in order to get them you will need to get your hands dirty.";
	next;
	mes .n$;
	mes "I mean, virtually of course! With these rumours out there about that virus and stuff, did you hear that?";
	next;
	mes .n$;
	mes "It is pretty scary isn't it! So make sure you keep your hands clean the best you can!";
	next;
	mes .n$;
	mes "Oh, where were we?";
	mes " ";
	sleep2 1000;
	mes "Ah! Of Course! "+getitemname(675)+"s!!";
	next;
	mes .n$;
	mes "So, one can visit my shop in ^0000FFProntera^000000 and get all the items from the ^FF0000Mania Shop^000000 not with Mania$ but with..."+getitemname(675)+"!! How awesome is that?";
	mes " ";
	mes "^0000FF* That excludes Costume items *^000000";
	next;
	mes .n$;
	mes "What else... Ah!";
	mes "You can also activate/extend your Premmy account with these coins!";
	next;
	mes .n$;
	mes ".... and reset stats or skills...";
	next;
	mes .n$;
	mes ".... and remove cards from equipments...";
	next;
	mes .n$;
	mes "Ah! and you can also use them to convert ordinary hats in costumes!!";
	next;
	mes .n$;
	mes "Uh there is just so much you can do with them, I can't remember them all, sorry!";
	mes " ";
	mes "^0000FFVisit our website for more info on everything you can get with "+getitemname(675)+"s.^000000";
	next;
	mes .n$;
	mes "Now you must be saying to yourself:";
	mes "Ok Sophie... but how on Earth do I get these precious, precious coins?";
	mes "hi hi hi ~*";
	next;
	mes .n$;
	mes "I tell you right now!";
	next;
	mes .n$;
	mes "In Prontera, right next to the Central Plaza you are gonna find my...";
	next;
	mes .n$;
	mes "partner?!";
	mes "I guess...";
	next;
	mes .n$;
	mes "The strong, brave, gentile....";
	mes "Agent Kafra!";
	next;
	mes .n$;
	mes "No no... that is not her real name but she prefers to be called just like that.";
	next;
	mes .n$;
	mes "Anyways, I am gonna be there as well, next to her <3";
	mes " ";
	mes "...";
	next;
	mes .n$;
	mes "Oh sorry! I got distracted!";
	mes "So, from the Agent Kafra one can take ^AA0000Hunting Quests^000000.";
	next;
	mes .n$;
	mes "They are 24 hours quests, daily quests...";
	next;
	mes .n$;
	mes "You take them, hunt the list of monsters she gives you, come back and...";
	next;
	mes .n$;
	mes "BANG!";
	next;
	mes .n$;
	mes "You get lots of EXP,";
	mes "by lots I mean LOTS!";
	next;
	mes .n$;
	mes "You also get some Zeny,";
	mes "....";
	mes "and of course:";
	mes "shining, precious";
	mes "^FFA500"+getitemname(675)+"s^000000";
	next;
	mes .n$;
	mes "Later on in the game you will find out that you can also get them from our colleague ^0000FFGeferus^000000 in Geffen.";
	next;
	mes .n$;
	mes "And also from the ^0000FFBatalhas Territoriais^000000... a GvG event that happens Monday to Saturday at 2pm and then again at 7pm.";
	next;
	mes .n$;
	mes "But that is all I know... To hear more about our PvP Events please refer to my friend accross the room...";
	next;
	mes .n$;
	mes "She is the Arena Master in whole Midgard!";
	next;
	mes .n$;
	mes "Can you believe that?";
	mes "...";
	mes "She is tough but she is nice, go on, talk to her!";
	next;
	mes .n$;
	mes "I believe this is it, "+strcharinfo(0)+".";
	mes "I told you this was gonna be brief!";
	mes "All the best in your journey!";
	mes "And see you in Prontera!";
	if(checkquest(64500) < 0 ){
		setquest 64500;
		setquest 64503;
	}
	'INTRO_QUEST = 8; 
	close;

OnInit:
	.n$ = "[^FFA500Sophie^000000]";
	end;
}



new_1-2,79,101,6	script	Arena Master#new	430,{
	if('INTRO_QUEST < 8){
		mes .n$;
		mes "Just one minute please...";
		close;
	}
	mes .n$;
	mes "Howdy,"+strcharinfo(0);
	mes "How's that strengh?";
	next;
	mes .n$;
	mes "No need to answer.";
	next;
	mes .n$;
	mes "So you've finally got your shit together and decided to join us, huh?";
	mes "Good for you!";
	next;
	mes .n$;
	mes "Actually, good for us all...";
	mes "Strange days, these days";
	mes "And we need brave warriors";
	mes "And wize Sourcerers";
	mes "More than never";
	next;
	mes .n$;
	mes "So I am the Battle Master here in Ragnamania Chronos and I am responsible for all the PvP events...";
	next;
	mes .n$;
	mes "You will find me in Prontera, in the Central Plaza... You should come and talk to me at certain point.";
	next;
	mes .n$;
	mes "You know, PvP and GvG events are quite a thing after the Ragnarok and that keeps our mind busy and body strong.";
	next;
	mes .n$;
	mes "There is a lot going on:";
	mes "PvP Arena";
	mes "GvG Arena";
	mes "War of the Emperium";
	mes "Battlegrounds Arenas";
	mes "Batalhas Territoriais";
	next;
	mes .n$;
	mes "I can exmplain them one by one to you, if you want...";
	next;
	while (.@op != 6){
		mes .n$;
		mes "Want to hear more about something in particular?";
		next;
		switch(.@op = select("PvP Arena:GvG Arena:War of the Emperium:Battlegrounds Arenas:Batalhas Territoriais:No, I am fine, thanks")){
			case 1:
				mes .n$;
				mes "We count with one PvP Arena free for wall. I guard its entrance, in Prontera. You need to be at least level 180 in order to enter the arena, though.";
				next;
				mes .n$;
				mes "Killing other players in the PvP arena or in 1x1 duels will grant you "+getitemname(7829)+" which can be used to buy stuff you can use in PvP arenas from specialized sellers in Prontera (in the shop to the left of the Central Plaza)";
				next;
				mes .n$;
				mes "You can also use these badges to buy PvP specialized equipments from our friend ^0000FFErundek^000000 in Prontera. Only good stuff!";
				next;
				mes .n$;
				mes "There is a monthly PvP Ranking with some nice prizes for the Top 10! You should have a look into that!";
				mes " ";
				mes "^0000FFMore about our PvP Monthly Ranking and Prizes in our website.^000000";
				next;
				break;
			case 2:
				mes .n$;
				mes "We count with one single GvG Arena. I guard its entrance, in Prontera. You need to be at least level 180 in order to enter the arena.";
				next;
				mes .n$;
				mes "Killing other players in the GvG arena as well as in the WoE or during the Batalhas Territoriais will grant you "+getitemname(7773)+" which can be used to buy stuff you can use in GvG arenas and the WoE from specialized sellers in Prontera (in the shop to the left of the Central Plaza)";
				next;
				mes .n$;
				mes "You can also use these badges to buy PvP specialized equipments from our friend ^0000FFErundek^000000 in Prontera. Only good stuff!";
				next;
				break;
			case 3:
				mes .n$;
				mes "In the War of the Emperium, you know, it is all about breaking the damn stone!";
				next;
				mes .n$;
				mes "We host WoEs on Sundays, at 4pm (GMT -3)";
				mes " ";
				mes "^0000FFWe tend to rotate castles so please check for updated information in our website or with a friend.^000000";
				next;
				mes .n$;
				mes "During the WoE, when defending your castle you get more "+getitemname(7773)+" than when attacking.";
				next;
				mes .n$;
				mes "Again, you can use these badges to buy WoE stuff from the specialized sellers in Prontera (in the shop to left of the Central Plaza) or from ^0000FFErunked^000000, also in Prontera.";
				next;
				break;
			case 4:
				mes .n$;
				mes "We have at least two different modalities of Battlegrounds running simultaneously. You need to be at least level 160 to join them.";
				next;
				mes .n$;
				mes "To join the queue for the Battleground Arenas, use your "+getitemname(24501)+".";
				next;
				mes .n$;
				mes "Killing other players in the Battlegrounds will grant you "+getitemname(7828)+" which can be used to buy stuff you can use in Battlegrounds from specialized sellers in Prontera (in the shop to the left of the Central Plaza)";
				next;
				mes .n$;
				mes "You can also use these badges to buy PvP specialized equipments from our friend ^0000FFErundek^000000 in Prontera. Only good stuff!";
				next;
				break;
			case 5:
				mes .n$;
				mes "Batalhas Territoriais is custom content exclusive to Ragnamania Chronos.";
				next;
				mes .n$;
				mes "Every day, from Monday to Saturday, at 2pm and at 7pm a new region become contested and the guilds fight for ownage.";
				next;
				mes .n$;
				mes "The mechanics are similar to woe: Break the stone, defend the stone... But it all happens in the open field.";
				next;
				mes .n$;
				mes "The winner guild will have some bonuses in EXP from mobs in certain fields and dungeons related to the respective region.";
				mes " ";
				mes "^0000FFFor more on Batalhas Terriotorias please refer to our website.^000000";
				next;
				mes .n$;
				mes "Killing other players during the Batalhas Territoriais will grant you "+getitemname(7773)+" which can be used to buy stuff you can use in Battlegrounds from specialized sellers in Prontera (in the shop to the left of the Central Plaza)";
				next;
				mes .n$;
				mes "It will also grant you the chance to drop "+getitemname(675)+"s from monsters in the fields and dungeons of the contested region.";
				next;
				mes .n$;
				mes "You can also use these badges to buy PvP specialized equipments from our friend ^0000FFErundek^000000 in Prontera. Only good stuff!";
				next;
				break;
			case 6:
			default:
				next;
				mes .n$;
				mes "Stay strong! Stay focused!";
				next;
				break;
		}
	}
	mes .n$;
	mes "See you in Prontera!";
	mes "179 levels from here.";
	mes "But no worries, that is gonna be super fast.";
	next;
	mes .n$;
	mes "Actually, meet me in Prontera once you are finnished here... I might have a thing or two for you there to help you in your journey.";
	setquest 64501;
	'INTRO_QUEST = 9;
	close;

OnInit:
	.n$ = "^FF0000Arena Master^000000";
	setmapflag "new_1-2",mf_noteleport,1;
	
	end;
}


new_1-2,84,115,4	script	Sweetie#new	4_F_KAFRA2,{
	// if(#INTRO_QUEST == 1)
	// 	goto OnContinue;

	if('INTRO_QUEST == 10)
		goto OnContinue;

	if('INTRO_QUEST < 9){
		mes .n$;
		mes "Just one minute please...";
		next;
		mes .n$;
		mes "Unless you'd want to skip the training grounds and go straight to Prontera...";
		next;
		mes .n$;
		mes "Do you wanna do that?";
		next;
		if(select("No, not yet:Yes please take me to Prontera") == 1) {
			mes .n$;
			mes "hi hi hi no problem!";
			close;
		} else {
			goto OnContinue;
		}

	}

	mes .n$;
	mes "Oh, hi precious!!";
	mes "Sorry I was a bit busy before...";
	mes "So, I guess we are almost ready";
	mes "to adventure around Midgard, right?";
	next;
	mes .n$;
	mes "Oh, silly me... I haven't introuced myself!";
	next;
	mes .n$;
	mes "My name is Suelen, but people around just call me Sweetie and I am fine with that.";
	next;
	mes .n$;
	mes "Me and my co-workers actually are all over Midgard and you can always count on us!";
	next;
	mes .n$;
	mes "Do you remember the old Kafra Corp.?";
	mes "Well, think of us as the Kafra Corp 2.0!";
	mes "ha ha ha";
	next;
	mes .n$;
	mes "We can take you to towns, fields and even to dungeons!";
	mes "brr.... creepy!";
	next;
	mes .n$;
	mes "We can also";
	mes "heal, buff, repair";
	mes "refine, reset stats,";
	mes "reset skills...";
	next;
	mes .n$;
	mes "... Platinum skills,";
	mes "remove cards, identify items,";
	mes "exchange coins, phew!";
	mes "literaly dozens of services!";
	'INTRO_QUEST = 10;
	next;
OnContinue:
	mes .n$;
	if(checkquest(64500) == 1 && checkquest(64501) == 1 && checkquest(64503) == 1 && checkquest(64505) == 1) {
		mes "Hum... I guess you are pretty much done here!";
		mes "Do you want me to ask someone to take you to Prontera?";
		next;
		if(select("No, thank you:Yes, please!") == 2) {
			mes .n$;
			mes "Absolutely!";
			mes "Please look for me in Prontera or any other town...";
			mes "I will have something ready for you there!";
			if(checkquest(64502) < 0)
				setquest 64502;
			#INTRO_QUEST = 1;
			close2;
			savepoint "prontera",162,189;
			warp "prontera",155,54;
			//sleep2 1000;
			instance_destroy;
			end;
		} else {
			mes .n$;
			mes "Not a problem!";
			mes "Please take all the time you need";
			mes "See you later, then";
			close;
		}
	} else {
		mes "In a hurry, huh?";
		mes "I don't blame you!";
		mes " ";
		mes "^FF0000You will be sent to Prontera without all the Tutorial Quests. Are you sure you want to proceed with that?";
		next;
		if(select("No, please ignore it:Yes, please take me to Prontera") == 2 ) {
			mes .n$;
			mes "Sure! Have fun!";
			close2;
			warp "prontera",155,54;
			instance_destroy;
			end;
		} else {
			mes .n$;
			mes "Of course! See you later then!";
			close;
		}
	}

OnInit:
	.n$ = "^DD00DD[Sweetie]^000000";
	end;
}