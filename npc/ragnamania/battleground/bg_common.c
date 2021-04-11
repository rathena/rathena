//===== rAthena Script =======================================
//= Battleground Extended +
//===== By: ==================================================
//= eAmod / Grenat
//===== Description: =========================================
//= Battleground Extended + scripts.
//===== Additional Comments: =================================
//= Adaptation for rAthena 2020.
//= This is a free release.
//= Please do NOT take ownership.
//= The original author will always stay eAmod,
//= It would be a disgrace claiming it's yours and making
//= money out of it. [Grenat]
//============================================================

//============================================================
// = Map Flags
//============================================================
bat_room	mapflag	nomemo
bat_room	mapflag	nowarpto
bat_room	mapflag	nobranch
bat_room	mapflag	nopenalty
bat_room	mapflag	noteleport
bat_room	mapflag	nosave	SavePoint


//============================================================
// = Configuration
//============================================================
-	script	BG_config	-1,{
	end;

	//=======================================================
	//= Main Config
	//=======================================================
OnInit:
	set .HappyHourRates, getbattleflag("bg_reward_rates") + 60;	//Happy Hour +60% rewards
	bindatcmd "joinbg",strnpcinfo(3)+"::OnAtcommand1";		//Join Battle Room
	bindatcmd "leavebg",strnpcinfo(3)+"::OnAtcommand2";		//Leave Battle Room
	bindatcmd "bghappyhourstart","BG_config::OnClock1600",99,99;	//Start Happy Hour
	bindatcmd "bghappyhourstop","BG_config::OnClock1800",99,99;	//Stop Happy Hour
	end;

	//=======================================================
	//= BG Happy Hours (Default: 4~6pm/8~10pm)
	//=======================================================
OnClock1600:
OnClock2000:
	announce "Battleground Happy Hour has begun",0,0x00FF00;
	setbattleflag "bg_reward_rates",.HappyHourRates; // Happy hour Rate
	end;
OnClock1800:
OnClock2200:
	announce "Battleground Happy Hour is over",0,0x00BFFF;
	setbattleflag "bg_reward_rates",getbattleflag("bg_reward_rates"); // Back to Normal Rates
	end;

	//==================================================
	//= Battleground Kill Count
	//==================================================
OnPCKillEvent:
	if(bg_teamid()>=0)
		bg_kill getcharid(3);
	end;

	//=======================================================
	// = Commands:
	// NO Need to TP to the battlefield.
	// BG Queue System is doing it with Join_Active_Event (db/battleground.yml)
	//=======================================================
OnAtcommand1:
	if(bg_teamid() >= 0) end;
	warp "bat_room",154,150;
	end;
OnAtcommand2:
	if (getbattleflag("feature.bgqueue"))
		bg_desert;
	else
		bg_leave;
	doevent "OnPCKickedBG::OnPCKickedBG";
	end;
}

//==================================================
//= Battleground Kicked Out
//==================================================
-	script	OnPCKickedBG	-1,{
OnPCKickedBG:
	if(countitem("Blue_Skull")) delitem "Blue_Skull",countitem("Blue_Skull");
	if(countitem("Red_Skull")) delitem "Red_Skull",countitem("Red_Skull");
	if(countitem("Green_Skull")) delitem "Green_Skull",countitem("Green_Skull");
	if(countitem("Bomb")) delitem "Bomb",countitem("Bomb");
	if(countitem("Bomb_Kick")) delitem "Bomb_Kick",countitem("Bomb_Kick");
	if(countitem("Fire_UP")) delitem "Fire_UP",countitem("Fire_UP");
	if(countitem("Bomb_UP")) delitem "Bomb_UP",countitem("Bomb_UP");
	if(countitem("Speed_UP")) delitem "Speed_UP",countitem("Speed_UP");
	if(countitem("SKILL_SHOOT")) delitem "SKILL_SHOOT",countitem("SKILL_SHOOT");
	if(countitem("SKILL_PUSH")) delitem "SKILL_PUSH",countitem("SKILL_PUSH");
	set @BBM_spawn, 0; set @BBM_HIT, 0; set @BBM_INVINCIBLE, 0; set @BBM_HIT, 0; set @ZONE_EXPLOSION, 0; set @BBM_DELAY, 0;
	sc_end SC_TIMERDOWN;
	cutin "",255;
	setpcblock PCBLOCK_ATTACK,0;
	atcommand "@size 0";
	end;
}

//============================================================
//= Recruiter/Information scripts.
//============================================================
-	script	BG_Register	FAKE_NPC,{
	mes "[^FFA500Battle Recruiter^000000]";
	mes "This is the Battleground Information service.";
	next;
	if(strcharinfo(3) == "prontera") .@warp$ = "Warp to Battle Room";
	switch(select("^FFA500Battleground Arenas^000000",.@warp$)) {
	case 1:
		mes "[^FFA500Battle Recruiter^000000]";
		mes "Battlegrounds Arena are different kinds of games where Teams fight for victory.";
		next;
			mes "[^FFA500Battle Recruiter^000000]";
			mes "Tell me... what battleground you don't understand?";
			next;
			switch(select("Capture the Flag:Team DeathMatch:Stone Control:Eye of Storm:Bossnia:Domination:Triple Inferno:Conquest:Rush:Poring Ball:Bomberman")) {
			case 1:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "The objective of the Flavius Battle CTF is to score 3 points before your enemy, by capture their Flag.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "To capture a Flag you need to take the enemy flag, and bring it to your base flag.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "If your base flag is captured too, you need to kill the flag carrier and take the flag back to your base.";
				break;
			case 2:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Kill all the enemy players to let their Team without points.";
				mes "Protect our army.";
				break;
			case 3:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Take the Stones in the middle of the battlefield and put in on your base, in the Stone Points.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Each Stone will give points to your team. First team reach 99 points wins the game.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Protect your stones from to be captured by the enemy.";
				break;
			case 4:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "There are two bases, North and South which teams can capture by holding position on the Base more than the other team.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Each base will give you points each 5 seconds of Domination. If your Team control both bases the amount of points increases.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "To get aditional points, in the middle there is a Flag spawn, capture it and put it on any of your team Bases.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "The first Team reach 99 points wins the match.";
				break;
			case 5:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Attack the enemy base and destroy each MVP Guardian. To do damage to the guardian your team must capture the Balance Flag in the middle base.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Each team have 5 guardian to be protected or killed.";
				break;
			case 6:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "There are three bases, North, Center and South which teams can capture by holding position on the Base more than the other team.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Each base will give you points each 5 seconds of Domination.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "The first Team reach 99 points wins the match.";
				break;
			case 7:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "There are 2 teams in the battlefield, your team and the opposite enemies.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Kill the enemy players, collect the skulls and bring them to the Sacrifice Totem to win points.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "You can collect your own team skulls and bring then to your Sacrifice Totem to avoid other teams to score but it won't count as points.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "If you get killed all your skulls will be drop to the floor, including your own skull. First Team to get 80 points wins the battle.";
				break;
			case 8:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "If you are Attacking, destroy the enemy defenses and it's Flag.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "If you are Defending, protect your castle defenses and the Flag.";
				break;
			case 9:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "Fight to capture the Castle and organize your team to defend it.";
				next;
				mes "[^FFA500Battle Recruiter^000000]";
				mes "If you fail on the first capture, kill the defender and take it for your team.";
				break;
			case 10:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "It is a soccer game, a poring (the ball) will be in the middle of the field. The match lasts 5min. The team that has the most goals win the game.";
				close;
			case 11:
				mes "[^FFA500Battle Recruiter^000000]";
				mes "The classic game specially made in Ragnarok Online. The goal is to defeat the enemies by putting bombs that will explode on them. It is a 2vs2 version of the game.";
				close;
			}
		close;
	case 2:
		mes "[^FFA500Battle Recruiter^000000]";
		mes "May the War God bless you.";
		close2;
		warp "bat_room",155,150;
		end;
	}
	end;
}

prontera,123,83,3	duplicate(BG_Register)	Battle Information#prt	4_F_JOB_KNIGHT
bat_room,159,164,4	duplicate(BG_Register)	Battle Recruiter#batroom	4_F_JOB_KNIGHT

//============================================================
//= General Guillaume scripts.
//============================================================
bat_room,160,159,3	script	General Guillaume	4_M_KY_KIYOM,{
	cutin "bat_kiyom2",2;
	mes "[General Guillaume]";
	mes "Hot-blooded adventurer, we need your ability to win this battle.";
	next;
	switch(select("What's the reason for the Battle?:Tell me about Prince Croix")) {
	case 1:
		cutin "bat_kiyom1",2;
		mes "[General Guillaume]";
		mes "Our great king, Marcel Marollo VII, is very sick lately.";
		mes "His Majesty has declared that he has chosen either me or Prince Croix as the next king amongst his 9 sons.";
		next;
		mes "[General Guillaume]";
		mes "Two kings can't share a nation! Only the one victorious from His Majesty's appointed battle will be enthroned.";
		next;
		mes "[General Guillaume]";
		mes "This is, however, not just a battle between us. This battle will determine the future of this country.";
		mes "I pledge on my honor to prove that I'm the one who can protect this Maroll from outside threats.";
		next;
		switch(select("Yes, I want to join you.:End Conversation")) {
		case 1:
			cutin "bat_kiyom2",2;
			mes "[General Guillaume]";
			mes "Welcome to my army, comrade.";
			mes "Your eyes tell me that you're a soldier that I can trust.";
			next;
			mes "[General Guillaume]";
			mes "Now, go upstairs and apply for battle with your comrades.";
			mes "I'm sure they'll welcome you whole-heartedly!";
			break;
		case 2:
			mes "[General Guillaume]";
			mes "I'll be the one who will capture the flag!";
			break;
		}
		break;
	case 2:
		cutin "bat_kiyom1",2;
		mes "[General Guillaume]";
		mes "The 5th Prince Croix is currently titled as the Prime Minister of Maroll.";
		mes "He thinks all national matters of a nation can be discussed and determined on a desk,";
		mes "and believes in peaceful co-existence with other countries.";
		next;
		mes "[General Guillaume]";
		mes "He's too ignorant to admit that so-called peace is built on countless lives that are sacrificed in wars while normal citizens and upper classes can live, oblivious to the horrors that allow them to live that way.";
		next;
		mes "[General Guillaume]";
		mes "He's too naive to understand the reality....";
		mes "I can't leave Maroll to someone like him who lives in a dream!";
		next;
		mes "[General Guillaume]";
		mes "His unrealistic beliefs will drown this country in poverty and make the people weak. If he becomes the king, Maroll will never rest from the onslaughts of other countries.";
		mes "I want to teach him what makes this small country so powerful and prosperous. It's military power!";
		next;
		switch(select("I want to join your army!:End Conversation")) {
		case 1:
			cutin "bat_kiyom2",2;
			mes "[General Guillaume]";
			mes "Welcome to my army, comrade.";
			mes "Your eyes tell me that you're a soldier that I can trust.";
			next;
			mes "[General Guillaume]";
			mes "Now, go upstairs and apply for battle from your comrades.";
			mes "I'm sure they'll welcome you whole-heartedly!";
			break;
		case 2:
			mes "[General Guillaume]";
			mes "I'll be the one who will capture the flag!";
			break;
		}
		break;
	}
	close2;
	cutin "bat_kiyom1",255;
	cutin "bat_kiyom2",255;
	end;
}

//============================================================
//= General Croix scripts.
//============================================================
bat_room,160,140,3	script	Prince Croix	4_M_CRU_CRUA,{
	cutin "bat_crua1",2;
	mes "[Prince Croix]";
	mes "Wise adventurer, why don't you lend us your power for victory?";
	next;
	switch(select("What's the reason for the Battle?:Tell me about General Guillaume")) {
	case 1:
		cutin "bat_crua2",2;
		mes "[Prince Croix]";
		mes "Maroll's great king, Marcel Marollo VII, is very sick lately.";
		mes "His Majesty has declared that he will be leaving the future of Maroll to me or the 3rd prince, General Guillaume.";
		next;
		mes "[Prince Croix]";
		mes "General Guillaume may have an advantage in this battle as he is the great general of Maroll, but that doesn't automatically mean he'll win.";
		mes "I want to win this battle so that I can bring prosperity to the people of Maroll. They've suffered enough from war...";
		next;
		switch(select("Yes, I want to join you.:End Conversation")) {
		case 1:
			cutin "bat_crua1",2;
			mes "[Prince Croix]";
			mes "Thank you so much. I feel like I can win with the help of adventurers like you.";
			mes "Now, please go downstairs and join your comrades in sharpening their skills to fight the enemy!";
			break;
		case 2:
			mes "[Prince Croix]";
			mes "For Maroll!";
			break;
		}
		break;
	case 2:
		cutin "bat_crua2",2;
		mes "[Prince Croix]";
		mes "The 3rd Prince Guillaume is the great general of Maroll.";
		mes "It's a waste of time to explain to you how great a leader or warlord he is, since he commands the great military power of Maroll.";
		next;
		mes "[Prince Croix]";
		mes "Unfortunately, there's something he and his followers are unaware of:";
		mes "Do the people of Maroll really want them to spend so much money on military power?";
		mes "We have suffered enough from wars.";
		mes "I believe weapons aren't the best way to bring prosperity to a nation.";
		next;
		mes "[Prince Croix]";
		mes "I do not wish to shed blood, but I have no choice but to fight for the possibility of peace and for the sake of my people.";
		next;
		switch(select("Yes, I want to join you.:End Conversation")) {
		case 1:
			cutin "bat_crua1",2;
			mes "[Prince Croix]";
			mes "Thank you so much. I feel like I can win with the help of adventurers like you.";
			mes "Now, please go downstairs and join your comrades in sharpening their skills to fight the enemy!";
			break;
		case 2:
			mes "[Prince Croix]";
			mes "For Maroll!";
			break;
		}
		break;
	}
	close2;
	cutin "bat_crua1",255;
	cutin "bat_crua2",255;
	end;
}
bat_room,161,141,3	script	Prince Croix's Aid::bat_aid	4_M_CRU_HEAD,{ end; }
bat_room,161,139,3	duplicate(bat_aid)	Prince Croix's Aid::bat_aid2	4_M_CRU_HEAD
bat_room,161,160,3	duplicate(bat_aid)	General Guillaume's Aid::bat_aid3	4_M_KY_HEAD
bat_room,161,158,3	duplicate(bat_aid)	General Guillaume's Aid::bat_aid4	4_M_KY_HEAD

//============================================================
//= Flag for duplication scripts.
//============================================================
-	script	Base Flag#bg	FAKE_NPC,{ end; }

//============================================================
//= Exit scripts.
//============================================================
bat_room,148,150,4	script	Teleporter#bat	4_F_TELEPORTER,{
	mes "[Teleporter]";
	mes "Do you wish to leave the battlefield? Use my service to return to town.";
	next;
	if (select("Leave:Don't Leave") == 2) {
		mes "[Teleporter]";
		mes "I'll be here whenever you're in need of my service.";
		close;
	}
	set .@spoint$, getsavepoint(0);
	set .@x, getsavepoint(1);
	set .@y, getsavepoint(2);
	mes "[Teleporter]";
	mes "You will be sent back to " + .@spoint$ + ".";
	close2;
	warp .@spoint$, .@x, .@y;
	end;
}

//============================================================
//= Kafra scripts.
//============================================================
bat_room,148,147,4	script	Kafra Staff::kaf_bat	861,{
	cutin "kafra_09",2;
	callfunc "F_Kafra",0,2,1,150,0;
}

//============================================================
//= Repairman scripts.
//============================================================
bat_room,138,144,4	script	Repairman#bg	86,{
	callfunc "repairmain","Repairman";
	end;
}

// ===========================================================
// Healer
// ===========================================================
-	script	Therapist in battle#bg::BGHeal	4_F_SISTER,{
	mes "[Therapist in battle]";
	mes "Just close your eyes, and take a deep breathe.";
	mes "You can be free from pain.";
	specialeffect 312;
	percentheal 100,100;
	repairall;
	close2;
	openstorage;
	end;
}

//============================================================
//= Function Rewards scripts.
//============================================================
function	script	BG_get_Rewards	{
	deletearray $@arenamembers[0],getarraysize($@arenamembers);
	bg_get_data(getarg(0),1);
	.@quantity = getarg(2) + ((getarg(2)*getbattleflag("bg_reward_rates"))/100);
	for( .@i = 0; .@i < getarraysize($@arenamembers); .@i++ )
		getitem getarg(1),getarg(2),$@arenamembers[.@i];
	return;
}

//============================================================
//= Rewards scripts.
//============================================================
bat_room,160,150,3	script	Erundek	4_M_MANAGER,{
	if (checkweight(1201,1) == 0) {
		mes "- Wait a minute !! -";
		mes "- Currently you're carrying -";
		mes "- too many items with you. -";
		mes "- Please try again -";
		mes "- after you loose some weight. -";
		close;
	}
	mes "[Erundek]";
	mes "Do you have the battlefield badges?";
	mes "I can exchange Bravery Badges and Valor Badges for reward items.";
	next;
	switch(select("Exchange Badges", "Check the Catalog")) {
		case 1:
			mes "[Erundek]";
			mes "Which type of items would you like to exchange?";
			mes "To check more information about the reward items, please use our ^3131FFCatalog^000000.";
			next;
			switch(select("Weapon", "Armor", "Accessory", "Consumable")) {
				case 1:
					mes "[Erundek]";
					mes "You chose ^3131FFWeapon^000000.";
					mes "The following weapons are available for exchange with the battlefield badges.";
					mes "Please note that items for ^3131FFBravery Badges are indicated as (BB)^000000, and ^3131FFValor Badges as (VB)^000000.";
					next;
					switch(select("Dagger/OneSword/TwoSword/TwoSpear", "Staff/Mace/TwoAxe/Shuriken", "Bow/Katar/Music/Whip", "Book/Knuckle", "Revolver/Rifle/Gatling/Shotgun/Launcher")) {
						case 1:
							mes "[Erundek]";
							mes "The following items are available in the ^3131FFDagger, One-Handed Sword, Two-Handed Sword, and Two-Handed Spear^000000 category.";
							next;
							setarray .@Weapons[0],13036,7828,13037,7829,13411,7828,13410,7829,1183,7828,1184,7829,1425,7828,1482,7829;
							break;
						case 2:
							mes "[Erundek]";
							mes "The following items are available in the ^3131FFStaff / Mace / Two-Handed Axe / Huuma Shuriken^000000 category.";
							next;
							setarray .@Weapons[0],1632,7828,1633,7829,1634,7828,1635,7829,1543,7828,1542,7829,1380,7828,1379,7829,13305,7828,13306,7829;
							break;
						case 3:
							mes "[Erundek]";
							mes "The following weapons are available in the ^3131FFBow / Katar / Musical Instrument / Whip^000000 category.";
							next;
							setarray .@Weapons[0],1739,7828,1738,7829,1279,7828,1280,7829,1924,7828,1923,7829,1978,7828,1977,7829;
							break;
						case 4:
							mes "[Erundek]";
							mes "The following weapons are available in the ^3131FFBook / Knuckle^000000 category.";
							next;
							setarray .@Weapons[0],1574,7828,1575,7829,1824,7828,1823,7829;
							break;
						case 5:
							mes "[Erundek]";
							mes "The following weapons are available in the ^3131FFRevolver / Rifle / Gatling Gun / Shotgun / Grenade Launcher^000000 category.";
							next;
							setarray .@Weapons[0],13108,7828,13171,7829,13172,7828,13173,7829,13174,7829;
							break;
					}
					.@menu$ = "";
					for (.@i = 0; .@i < getarraysize(.@Weapons); .@i += 2)
						.@menu$ += getitemname(.@Weapons[.@i])+((.@Weapons[.@i+1]==7828)?"(BB)":"(VB)")+":";
					.@i = (select(.@menu$)-1)*2;
					.@type$ = ((.@Weapons[.@i+1]==7828)?"(BB)":"(VB)");
					mes "[Erundek]";
					mes "You chose ^3131FF"+getitemname(.@Weapons[.@i])+.@type$+"^000000.";
					mes "You can exchange for this item with ^FF0000100 "+getitemname(.@Weapons[.@i+1])+"^000000.";
					mes "Would you like to exchange?";
					next;
					switch(select("Do not exchange", "Exchange")) {
						case 1:
							break;
						case 2:
							mes "[Erundek]";
							mes "Would you like to spend ^FF0000100 "+getitemname(.@Weapons[.@i+1])+"^000000 and receive a ^3131FF"+getitemname(.@Weapons[.@i])+.@type$+"^000000?";
							next;
							mes "[Erundek]";
							mes "Remember, Battleground Reward Items are ^FF0000Character Bound^000000. Are you sure you want this item?";
							next;
							switch(select("Yes", "No")) {
							case 1:
								mes "[Erundek]";
								if (countitem(.@Weapons[.@i+1]) >= 100) {
									mes "Thank you for exchanging.";
									delitem .@Weapons[.@i+1],100;
									getitem .@Weapons[.@i],1;
								}
								else mes "I'm sorry, but you don't have enough badges to exchange.";
								close;
							case 2:
								break;
							}
							break;
					}
					mes "[Erundek]";
					mes "Do you need more time to check the items?";
					close;
				case 2:
					mes "[Erundek]";
					mes "You chose ^3131FFArmor^000000.";
					mes "The following armors are available for exchange with the battlefield badges.";
					next;
					switch(select("Garments / Shoes", "Armor")) {
						case 1:
							setarray .@items[0],2538,50,2539,50,2540,50,2435,50,2436,50,2437,50;
							break;
						case 2:
							setarray .@items[0],2376,80,2377,80,2378,80,2379,80,2380,80,2381,80,2382,80;
							break;
						}
					break;
				case 3:
					mes "[Erundek]";
					mes "You chose ^3131FFAccessory^000000.";
					mes "You can exchange the Medal of Honors with your Badges according to the job classes, as follows:";
					next;
					setarray .@items[0],2733,500,2720,500,2721,500,2722,500,2723,500,2724,500,2725,500;
					.@menu1$ = "Gunslinger:Swordman/Taekwon Master:Thief:Acolyte:Magician:Archer:Merchant";
					break;
				case 4:
					mes "[Erundek]";
					mes "You chose ^3131FFConsumable^000000.";
					mes "The following consumable items are available for exchange with the battlefield badges:";
					next;
					setarray .@items[0],12269,10,12270,10,12271,5,12272,10,12273,10;
					break;
			}
			break;
		case 2:
			mes "[Erundek]";
			mes "We have many items, so please take a look and purchase deliberately.";
			close2;
			readbook 11010,1;
			end;
	}
	.@menu$ = "";
	if (.@menu1$ != "") .@menu$ = .@menu1$;
	else for (.@i = 0; .@i < getarraysize(.@items); .@i += 2)
		.@menu$ += getitemname(.@items[.@i])+":";
	.@i = (select(.@menu$)-1)*2;
	mes "[Erundek]";
	mes "You chose ^3131FF"+getitemname(.@items[.@i])+"^000000.";
	switch(.@items[.@i]) {
		case 2720: mes "This item is for Swordman and Taekwon Master Class only."; break;
		case 2721: mes "This item is for Thief Class only."; break;
		case 2722: mes "This item is for Acolyte Class only."; break;
		case 2723: mes "This item is for Magician Class only."; break;
		case 2724: mes "This item is for Archer Class only."; break;
		case 2725: mes "This item is for Merchant Class only."; break;
		case 2733: mes "This item is for Gunslinger only."; break;
		default: break;
	}
	mes "You can exchange for this item with ^FF0000"+.@items[.@i+1]+" "+getitemname(7828)+" or "+.@items[.@i+1]+" "+getitemname(7829)+"^000000.";
	mes "Would you like to exchange?";
	next;
	switch(select("Do not exchange", "Exchange")) {
		case 1:
			mes "[Erundek]";
			mes "Do you need more time to check the items?";
			break;
		case 2:
			mes "[Erundek]";
			mes "Which Badge do you want to exchange?";
			mes "You need ^3131FF"+.@items[.@i+1]+" Badges^000000 to exchange.";
			next;
			if (.@item[0] < 12269 || .@item[0] > 12273) {
				mes "[Erundek]";
				mes "Remember, Battleground Reward Items are ^FF0000Character Bound^000000. Are you sure you want this item?";
				next;
			}
			.@j = select("Bravery Badge", "Valor Badge", "Cancel");
			mes "[Erundek]";
			if (.@j == 3) {
				mes "You cancelled the exchange.";
				break;
			}
			.@cost = ((.@j==1)?7828:7829);
			if (countitem(.@cost) >= .@items[.@i+1]) {
				mes "Thank you for exchanging.";
				delitem .@cost, .@items[.@i+1];
				getitem .@items[.@i],1;
			}
			else mes "You do not have enough "+getitemname(.@cost)+"s.";
			break;
	}
	close;
}

bat_room,160,148,3	script	Telma	701,{
	mes "[Telma]";
	mes "Welcome, mighty warrior.";
	mes "Do you need supplies for your battles?";
	mes "I can exchange supplies for your badges...";
	next;
	
	switch(select("150 Condensed White Potion:90 Blue Potion:3 Poison Bottle:30 Fire Bottle:30 Acid Bottle:30 Plant Bottle:30 Marine Sphere Bottle:15 Glistening Coat:50 Yellow Gemstone:50 Red Gemstone:100 Blue Gemstone:5 Speed Potion:20 Cobweb")) {
		case 1:		callsub S_BuyConsumableBG,547,150;	// Condensed White Potion
		case 2:		callsub S_BuyConsumableBG,505,90;	// Blue Potion
		case 3:		callsub S_BuyConsumableBG,678,3;	// Poison Bottle
		case 4:		callsub S_BuyConsumableBG,7135,30;	// Fire Bottle
		case 5:		callsub S_BuyConsumableBG,7136,30;	// Acid Bottle
		case 6:		callsub S_BuyConsumableBG,7137,30;	// Plant Bottle
		case 7:		callsub S_BuyConsumableBG,7138,30;	// Marine Sphere Bottle
		case 8:		callsub S_BuyConsumableBG,7139,15;	// Glistening Coat
		case 9:		callsub S_BuyConsumableBG,715,50;	// Yellow Gemstone
		case 10:	callsub S_BuyConsumableBG,716,50;	// Red Gemstone
		case 11:	callsub S_BuyConsumableBG,717,100;	// Blue Gemstone
		case 12:	callsub S_BuyConsumableBG,12016,5;	// Speed Potion
		case 13:	callsub S_BuyConsumableBG,1025,20;	// Cobweb
	}
	end;

S_BuyConsumableBG:
	set .@item_id, getarg(0);
	set .@amt, getarg(1);
	
	mes "[Telma]";
	mes "Do you want it for?...";
	next;
	if (select("~ Battleground:~ War of Emperium") == 1) .@bg_item = 1;
	mes "[Telma]";
	mes "How many sets of ^0000FF"+ .@amt +" "+ getitemname(.@item_id) +"s^000000 do you want?";
	mes "1 = "+ .@amt   +" supplies = 1 badge";
	mes "2 = "+ .@amt*2 +" supplies = 2 badges";
	mes "3 = "+ .@amt*3 +" supplies = 3 badges";
	mes "etc..";
	next;
	input .@sets;
	mes "[Telma]";
	if (.@sets < 1) {
		mes "Enter a positive amount, greater than zero.";
		close;
	}
	mes "So you want ^0000FF"+ (.@amt*.@sets) +" "+ getitemname(.@item_id) +"s^000000...";
	mes "Remember these can only be used in "+((.@item_id)?"BattleGround":"WoE Maps");
	mes "It will cost you "+ .@sets +" badge(s)...";
	next;
	switch(select("Give her "+ .@sets +" Bravery Badge(s):Give her "+ .@sets +" Valor Badge(s):Give her "+ (3*.@sets) +" Heroism Badges")) {
		case 1: set .@badge,7828; set .@cost,1; break;
		case 2: set .@badge,7829; set .@cost,1; break;
		case 3: set .@badge,7773; set .@cost,3; break;
	}
	.@bgchar = getBattleFlag("bg_reserved_char_id");
	.@woechar = getBattleFlag("woe_reserved_char_id");
	mes "[Telma]";
	if (countitem(.@badge) < (.@cost*.@sets)) {
		mes "You do not have enough ^FF0000"+getitemname(.@badge)+"s^000000 to buy "+(.@amt*.@sets)+" "+getitemname(.@item_id)+"s.";
	} else if (!checkweight(.@item_id, .@amt*.@sets)) {
		mes "You are overweight or have too many items in your inventory.";
	} else {
		delitem .@badge, .@cost*.@sets;
		if (.@bg_item) // Battleground's Consumables
			getitem2 .@item_id,(.@amt*.@sets),1,0,0, 254, 0, .@bgchar & 0xFFFF, .@bgchar >> 0x10;
		else
			getitem2 .@item_id,(.@amt*.@sets),1,0,0, 254, 0, .@woechar & 0xFFFF, .@woechar >> 0x10;
		mes "Here are your: ^0000FF"+(.@amt*.@sets)+" "+getitemname(.@item_id)+"s^000000!";
		mes "Remember they can only be used in "+((.@item_id)?"BattleGround":"WoE Maps");
	}
	close;
}

//============================================================
// = Rewards Shops
// = Create your own shop !
// = ID , Value
// = Example : 512 = Apple, 10 = Badge Quantity required
//============================================================
//bat_room,162,155,2	script	Malebolgia#1 	115,{ callshop "BF_Badge1",1; end; OnInit: waitingroom "BG Exchange Shop",0,"",1; end; }
//bat_room,160,152,2	script	Hishigi 	116,{ callshop "BGBadge",1; end; OnInit: waitingroom "KVM Exchange Shop",0,"",1; end; }
//bat_room,162,146,2	script	Yang Xia#1 	117,{ callshop "BF_Badge2",1; end; OnInit: waitingroom "Valor Exchange Shop",0,"",1; end; }
//-	itemshop	BGBadge	-1,6376,512:10;			// KVM Badge
//-	itemshop	BF_Badge1	-1,7828,512:10;		// Bravery Badge
//-	itemshop	BF_Badge2	-1,7829,512:10;		// Valor Badge