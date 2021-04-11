// Daily Hunting Missions
// ==============================================================================


// Rops por silvercoins shop : item_id, valor
//-	itemshop	SophieGSMshop	-1,675,5685:600,5209:600,2284:600,5302:700

prontera,0,0,3	script	HuntingQuestsSetup	-1,{
	end;

	OnInit:
		$@FORBIDEN_MOBS$ = "1198,1212,1229,1221,1234,1235,1231,1237,1238,1062,1183,1818,1684,1629,1668,1618,1220,1240,1241,1290,1563,1595,1675,1167,1601,1093,1669,1247,1845,1857,1855,1858,1859,1860,1035,1972,1929,1508,2026,1588,1618,1304,1862,1244,1861,1219,1300,1631,"; //Event mobs?
		$@FORBIDEN_MOBS$ += "1065,1088,1089,1090,1091,1092,1702,1093,1096,1108,1120,1200,1203,1204,1205,1231,1239,1250,1259,1262,1268,1270,1283,1285,1289,1291,1294,1295,1296,1299,1301,1302,1308,1307,1309,1313,1376,1388,1444,1447,1449,1456,1485,1486,1487,1527,1539,1548,1568,1569,1571,1576,1581,1582,1605,1658,1766,1767,1795,1851,1852,1853,1854,1879,1968,1701,1703,1839,1957"; //Mini-bosses
		$@QUEST_DEADLINE = 86400; // 24 hours in seconds
		$@RESET_ALLOWANCE = 86100; // 5 minutes less than the deadline
	end;
	
}

prontera,165,163,3	script	Contracts	857,{
//	function Build_Mission;
	callfunc "MissionInfo";
	end;
OnInit:
	// bindatcmd "share",strnpcinfo(3)+"::OnAtcommand",0,99;
	end;

OnShare:
OnAtcommand:
	callfunc "F_Share";
	end;
}


function	script	F_Share	{
	//	if( getgmlevel() < 99 ) end; // DEBUG ONLY 
	if( getcharid(1) == 0 ) {
		dispbottom "Hunting Quests: Can't find any party members to share the quest with.";
		end;
	}

	if( HM_MISSION_TICK < gettimetick(2) ) {
		dispbottom "Hunting Quests: The actual quest has expired and can't be shared.";
		end;
	}

	.@pid = getcharid(1); 			// Party ID
	.@owner$ = strcharinfo(0);		// Nome do compartilhador
	.@ocharid = getcharid(0);		// Charid do compartilhador
	.@oacc = getcharid(3);			// Conta do Compartilhador (para pular ele na hora de distribuir a quest)
	//.@orlv = HM_MISSION_RANK;		// Mission Rank do compartilhador
	.@orlv = BaseLevel;				// BaseLevel do compartilhador

	for( .@i=1; .@i < 6; .@i++ ) {
		.@mobs$ = .@mobs$ + getd("HM_MISSION_ID" + .@i) + "|";
	}

	$@partymembercount = 0;
	deletearray $@partymemberaid[0],128;

	getpartymember .@pid,2;
	set .@members, $@partymembercount;
	copyarray .@GID[0], $@partymemberaid[0], .@members;

	for( .@i=0; .@i < .@members; .@i++ ) {
		if ( isloggedin(.@GID[.@i]) == 1 && .@GID[.@i] != .@oacc ) {
			if( attachrid(.@GID[.@i]) == 1 ) {

				.@lvDiff = BaseLevel - .@orlv;
				.@lvDiff = callfunc("abs",.@lvDiff);
				//debugmes ".@lvDiff = " + .@lvDiff;

				if( HM_IN_MISSION || .@lvDiff > 20 ) {
					dispbottom "Hunting Quests: " + .@owner$ + " failed to share a Hunting Quest with you. Base Level difference is too high or you might be in a quest already.";
					dispbottom "Hunting Quests: " + strcharinfo(0) + " could not accept the quest this at time.",0xFF0000, .@ocharid;
				} else {
					mes "[ Agent Kafra ]";
					mes "^0000FF" + .@owner$ + "^000000 wants to share a Hunting Quest with you.";
					// next;
					// callfunc ("MissionInfo");
					//next;
					mes "Will you accept it?";
					if(select("Yes, accept it:Decline.") == 1) {
						callfunc "Build_Mission", .@mobs$;
						dispbottom "Hunting Quests: Quest Accepted!";
						dispbottom "Hunting Quests: " + strcharinfo(0) + " has accepted your quest.",0xFF0000, .@ocharid;
						close;
					} else {
						close;
					}
				}
			}
		}
	}
	attachrid(.@oacc);
	dispbottom "Hunting Quest succesfully shared among party members.";
	
	end;
}

prontera,165,172,3	script	Top Hunter	857,{
	mes "[^FFA500Best Hunter^000000]";
	if( $TMission_CharID ) {
		mes "Name : ^0000FF" + $TMission_Name$ + "^000000";
		mes "Job : ^0000FF" + $TMission_Job$ + "^000000";
		mes "Score : ^0000FF" + $TMission_Score + "^000000";
	} else {
		mes "Name : ^0000FF(none)^000000";
		mes "Job : ^0000FF(none)^000000";
		mes "Score : ^0000FF0^000000";
	}

	next;
	mes "[^FFA500Best Hunter^000000]";
	mes "The first position is assigned to the best hunter who has completed the most missions.";
	mes "His prize is the chance to get a random costume while while holding the position.";
	close;
}

prontera,164,169,3	script	Agent Kafra::HuntingSystem	859,{
	if(checkquest(64503) == 1) {
		mes "[^FFA500Agent Kafra^000000]";
		mes "Oh look who's here!";
		mes "How are you doing?";
		mes "Sophie said only good things about you!";
		next;
		mes "[^FFA500Agent Kafra^000000]";
		mes "So, you are you ready to start hunting?";
		next;
		mes "[^FFA500Agent Kafra^000000]";
		mes "Here, take these... they might be helpful.";
		getitem 569,200; //Novice_Potion
		completequest 64503;
		next;
	}

	mes "[^FFA500Agent Kafra^000000]";
	if( !HM_IN_MISSION ) {
		if( HM_MISSION_TICK > gettimetick(2)) {
			mes "Hello, already here?";
			mes "Let's see how you completed the mission...";
			next;
			if( (HM_MISSION_COUNT1 + HM_MISSION_COUNT2 + HM_MISSION_COUNT3 < 1) && HM_MISSION_ID1 > 0 ) {
				mes "[^FFA500Agent Kafra^000000]";
				mes "Great! You did it on time!";
				mes "Let me pay you for the job.";
				next;
				callfunc "GiveRewards";
				close;
			} 
		} else if ( HM_MISSION_INTRO ) {
			mes "Your new mission is ready.";
			mes "Are you ready to take it on?";
		} else {
			mes "Hello...";
			mes "Are you interested in a job?";
			next;
			mes "[^FFA500Agent Kafra^000000]";
			mes "Please allow me to exmplain this to you... Every day, I find many people interested in ^0000FFHunting Missions^000000.";
			mes "The missions consist of killing a list of monsters.";
			next;
			mes "[^FFA500Agent Kafra^000000]";
			mes "There are different difficulty level of Missions.";
			mes "But don't worry, you can work with your friends in groups for this.";
			next;
			mes "[^FFA500Agent Kafra^000000]";
			mes "You will have exactly 24 hours to complete a mission, finish it on time and come back to me for the prize.";
			HM_MISSION_INTRO = 1;
			next;
		}
		//next;
		if( select("Yes, what is my job?:No thanks, not today.") == 2 ) {
			mes "[^FFA500Agent Kafra^000000]";
			mes "Okay, you know where to find me.";
			close;
		}

		BuildMission:
		// mes "[^FFA500Agent Kafra^000000]";
		mes "Go ahead and check it out:";
		next;

		callfunc("Build_Mission");
		callfunc("MissionInfo");
		next;

		mes "[^FFA500Agent Kafra^000000]";
		mes "Good luck, and remember that you have 24 hours to complete it.";
		mes "Come back to me when you've finished it.";
		close;
	} else if(HM_MISSION_TICK < gettimetick(2)) {
		set HM_IN_MISSION, 0;
		set HM_MISSION_TICK, 0;
		mes "You've failed your last mission.";
		mes "You haven't completed it within the time limit.";
		next;
		mes "[^FFA500Agent Kafra^000000]";
		mes "There is another mission available for you today.";
		mes "Do you want to take it now?";

		next;
		if( select("Yes, what is my job?:No thanks, not today.") == 2 )
		{
			mes "[^FFA500Agent Kafra^000000]";
			mes "Okay, come back if you've changed your mind.";
			close;
		}

		goto BuildMission;
	} else {
		//mes "[^FFA500Agent Kafra^000000]";
		mes "You have not completed the last assigned quest yet.";
		next;
		callfunc "MissionInfo";

		if (countitem(6320) < 1 && HM_MISSION_TICK - $@RESET_ALLOWANCE < gettimetick(2) && HM_FREERESET == 1) {
			close;
		} else {
			next;
			mes "[^FFA500Agent Kafra^000000]";
			if( HM_MISSION_TICK - $@RESET_ALLOWANCE > gettimetick(2) || HM_FREERESET == 0) {
				mes "You are entitled to ^FF0000abandon the current Hunting Quest^000000 for a small fee. Do you want to do that?";
			} else if(countitem(6320) > 0 ) {
				mes "Do you want to ^FF0000abandon the current^000000 Hunting Quest using your ^0000FF" + getitemname(6320) + "^000000?";	
			} else {
				mes "Something went wrong. Please report this to the staff.";
				logmes "Agent Kafra : Dead End.";
				close;
			}
			if(select("No, I don't want it:Yes, please do it.") == 1) {
				close;
			} else {
				if(countitem(6320) > 0 || HM_MISSION_TICK - $@RESET_ALLOWANCE > gettimetick(2) || HM_FREERESET == 0) {
					if(HM_FREERESET == 0) {
						HM_FREERESET = 1;
						.@passou = 1;
					} else if(HM_MISSION_TICK - $@RESET_ALLOWANCE > gettimetick(2) && Zeny >= .ResetCost) {
						mes "It will cost you ^DD0000"+.ResetCost+"z^000000";
						mes "Do you want to continue?";
						if(select("No, Thanks:Yes, Please.") == 2) {
							if(Zeny >= .ResetCost){
								set Zeny, Zeny - .ResetCost;
								.@passou = 1;
							} else {
								mes "This action will be reported to the staff.";
								logmes "Tried to bypass the zeny cost to reset Hunting Quest";
								close;
							}
						}
					} else if(countitem(6320) > 0) {
						delitem 6320,1;
						.@passou = 1;
					} 

					if(.@passou == 1) {
						
						callfunc "reset_quest";
							
						//set HM_MISSION_ENDED, 0;
						next;
						specialeffect2 611;
						mes "[^FFA500Agent Kafra^000000]";
						mes "Done. You can take a new hunting quest now... Wish you better luck this time!";
						close;
					} else {
						logmes "Cheater detected.";
						dispbottom "This action has been reported to the staff.";
						end;
					}

					
				} else {
					next;
					mes "[^FFA500Agent Kafra^000000]";
					mes "Sorry, I can't help you this time.";
					close;
				}
			}
		}
	}

	//------------------------------------------------------------
	OnNPCKillEvent:
	// 	if ( HM_MISSION_TICK > gettimetick(2)) {
	// 		if(getcharid(1))
	// 			if(getpartyshare() == 0)
	// 				end;

	// 		callfunc ("UpdateQuestRecords",killedrid);
	// 	}
	// end;
	sleep2 rand(50,200); //TODO : CHECK IF THIS IS ISNT CRACKING THE SYSTEM
	if ( HM_MISSION_TICK > gettimetick(2)) {
		callfunc ("UpdateQuestRecords",killedrid);
	}
	end;
	

	//------------------------------------------------------------
	OnQuestTimeIsOver:
		if(HM_IN_MISSION){
			dispbottom(strnpcinfo(0) + ": Your hunting quest time has expired. Please look for me in Prontera.");
		}
		end;

	//------------------------------------------------------------
	OnPCLoginEvent:

		if(!HM_IN_MISSION)
			end;

		.@timeLeft = callfunc("ValidateQuestTime");
		if(.@timeLeft > 0) {
			.@timeLeft = callfunc("EpochToMs",.@timeLeft);
				addtimer .@timeLeft,"HuntingSystem::OnQuestTimeIsOver";
		} else {
			dispbottom(strnpcinfo(0) + ": Your hunting quest time has expired. Please look for me in Prontera.");
		}	
		end;

	OnPCLogoutEvent:
		if(HM_IN_MISSION)
			deltimer "HuntingSystem::OnQuestTimeIsOver";
		end;

	OnInit:
		.ResetCost = 10000;

}


prontera,164,166,3	script	Sophie	894,{
	if(checkquest(64500) == 1) {
		mes "[^FFA500Sophie^000000]";
		mes "Oh! "+strcharinfo(0)+"!!";
		mes "It is so nice to see you again!";
		next;
		mes "[^FFA500Sophie^000000]";
		mes "I am a little busy right now, as you can see...";
		mes "Did you talk to Agent Kafra right there?";
		mes "Have you taken a Hunting Quest?";
		next;
		mes "[^FFA500Sophie^000000]";
		mes "Here, take these... and please take care!";
		getitem 569,100; //Novice_Potion
		completequest 64500;
		next;
	}
	mes "[^FFA500Sophie^000000]";
	mes "Hey, I'm Agent Kafra's... er... Sophie.";
	mes "You can change your ^DD0000" +getitemname(675) +"^000000 for items and equipments.";
	next;
	mes "[^FFA500Sophie^000000]";
	mes "Your stats:";
	mes "- Mission Completed: ^0000FF" + HM_MISSION_COMPLETED + "^000000";
	mes "- "+ getitemname(675)+": ^0000DD" + countitem(675) + "^000000";
	next;
	switch( select("^999900Open Shop^000000:^FF0000Close^000000") )
	{
	case 1:
		mes "[^FFA500Sophie^000000]";
		mes "Let me show you what I've got for ^0000FF"+getitemname(675)+"^000000.";
		close2;
		// callshop "SophieGSMshop",1;
		// end;
		//dispbottom DisplayCurrency(.currency);
		callshop .shop_name$,1;
		npcshopattach .shop_name$;
		end;
		break;
	case 2:
		mes "[^FFA500Sophie^000000]";
		mes "Goodbye Sweetheart!";
		close;
	}

	OnBuyItem:
		BuyProcess(.priceList, .currency);

	OnInit:
		.@c = query_sql("SELECT `item_id`, `price` FROM `item_cash_db` WHERE `tab` <> 2 ORDER BY `tab`",.@id, .@price);
		
		for(.@i=0; .@i < .@c; .@i++) {
			//if(.@id[.@i] == 6320)
			//	continue;
			.items[.@j] += .@id[.@i];
			.items[.@j+1] += .@price[.@i]/10;
			.@j += 2;
		}

		.shop_name$ = "Rops4coinS";
		.npcName$ 	= strnpcinfo(1);	//Visiable name
		.currency 	= 675;				// 0: Zeny, else Item
						
		InitItemShop(.items,.shop_name$,.npcName$);
		end;
}

-	itemshop	Rops4coinS	-1,675,501:50
//-	itemshop	SophieGSMs	-1,675,5685:600



function	script	reset_quest	{
	HM_MISSION_TICK = 0;
	HM_IN_MISSION = 0;

	for( .@i=1; .@i<4; .@i++ ){
		//mission_sethunting .@i,0,0;
		setd "HM_MISSION_ID" + .@i, .@MobID;
		setd "HM_MISSION_COUNT" + .@i, .@qtty;
	}
	deltimer "HuntingSystem::OnQuestTimeIsOver";

	return;
}


function	script	GiveRewards	{
	set .@hm_mission_lv, 0;
	set .@Mission_Exp, 0;
	set .@Mission_Job, 0;
	for( set .@i, 1; .@i < 4; set .@i, .@i + 1 ) {
		set .@MobID, getd("HM_MISSION_ID" + .@i);
		set .@hm_mission_lv, .@hm_mission_lv + strmobinfo(3, .@MobID);
		//set .@Mission_Exp, .@Mission_Exp + ((strmobinfo(6, .@MobID) * 30) * 50);
		//set .@Mission_Job, .@Mission_Job + ((strmobinfo(7, .@MobID) * 30) * 50);
		if(Baselevel <= 180) {
			set .@Mission_Exp, (strmobinfo(6, .@MobID) * 30) * 50;
			set .@Mission_Job, (strmobinfo(7, .@MobID) * 30) * 50;
			getexp .@Mission_Exp,.@Mission_Job;
		}
	}
	set .@Mission_Zeny, .@hm_mission_lv * 30;
	if ( .@Mission_Zeny > 1000000 )
		.@Mission_Zeny = 1000000;

	if( .@hm_mission_lv < 200 )
		set .@Mission_Points, 5;
	else if( .@hm_mission_lv < 500 )
		set .@Mission_Points, 10;
	else
		set .@Mission_Points, 15;
	
	// Mission Rewards...
	// =====================================================================
	//getexp .@Mission_Exp, .@Mission_Job;
	set Zeny, Zeny + .@Mission_Zeny;
//	set #KAFRAPOINTS, #KAFRAPOINTS + .@Mission_Points;
	if( HM_MISSION_RANK < 449 ) 
		set HM_MISSION_RANK, HM_MISSION_RANK + 1;
	set HM_MISSION_COMPLETED, HM_MISSION_COMPLETED + 1;
	if( HM_MISSION_COMPLETED > $TMission_Score ) {
		dispbottom "Congratulations, you're the new Top Hunter of the day!!";
		set $TMission_Score, HM_MISSION_COMPLETED;
		set $TMission_CharID, getcharid(0);
		set $TMission_Name$, strcharinfo(0);
		set $TMission_Job$, jobname(Class);
	}

	if(vip_status(1))
		getitem 675,5;
	else
		getitem 675,5;

	if( rand(100) == 50 )	
		getitem 675,5;

	specialeffect2 610;


	// =====================================================================
	
	// Clear Mission Data
	for( set .@i, 1; .@i < 4; set .@i, .@i + 1 )
		mission_sethunting .@i,0,0;
	//set HM_MISSION_ENDED, 0; // wtf is this for?!?!?!?
	set HM_MISSION_TICK, 0;
	set HM_IN_MISSION, 0;
	deltimer "HuntingSystem::OnQuestTimeIsOver";

	return;
}


//
// Mostra as informacoes da missao atual
//

function	script	MissionInfo	{
	if(!HM_IN_MISSION)
		end;
	set .@hm_mission_lv, 0;
	set .@Mission_Exp, 0;
	set .@Mission_Job, 0;
	
	mes "[^FFA500Mission Contracts^000000]";
	for( set .@i, 1; .@i < 4; set .@i, .@i + 1 )
	{
		set .@MobID, getd("HM_MISSION_ID" + .@i);
		mes "^0000FF" + getd("HM_MISSION_COUNT" + .@i) + "^000000 x ^FFA500" + strmobinfo(2,.@MobID) + "^000000 (ID : " + .@MobID + ")";

		set .@hm_mission_lv, .@hm_mission_lv + strmobinfo(3, .@MobID);
		if(BaseLevel <= 180) {
			set .@Mission_Exp, .@Mission_Exp + (strmobinfo(6, .@MobID) * 50);
			set .@Mission_Job, .@Mission_Job + (strmobinfo(7, .@MobID) * 50);	
		}
		
	}

	set .@Mission_Zeny, .@hm_mission_lv * 1000;
	if ( .@Mission_Zeny > 1000000 )
		.@Mission_Zeny = 1000000;
	next;

	mes "[^FFA500Mission Details^000000]";
	mes "Mission Level : ^0000FF" + .@hm_mission_lv + "^000000";

	if( .@hm_mission_lv < 200 )
	{
		mes "Difficulty : ^008000Low^000000";
		set .@Mission_Points, 5;
	}
	else if( .@hm_mission_lv < 500 )
	{
		mes "Difficulty : ^000080Medium^000000";
		set .@Mission_Points, 10;
	}
	else
	{
		mes "Difficulty : ^FF0000High000000";
		set .@Mission_Points, 15;
	}
	
	set .@Time_Left, HM_MISSION_TICK - gettimetick(2);

	if( .@Time_Left <= 0 )
		mes "Time Remaining : ^FF0000Expired^000000";
	else if( .@Time_Left > 3600 )
		mes "Time Remaining : ^0000FFOver " + (.@Time_Left / 3600) + " hour(s)^000000.";
	else if( .@Time_Left > 60 )
		mes "Time Remaining : ^0000FFOver " + (.@Time_Left / 60) + " minute(s)^000000.";
	else
		mes "Time Remaining : ^0000FF" + (.@Time_Left) + " second(s)^000000.";
	next;

	mes "[^FFA500Mission Prizes^000000]";
	if(BaseLevel <= 180) {
		mes "Base Exp : ^0000FF" + .@Mission_Exp + "^000000";
		mes "Job Exp : ^0000FF" + .@Mission_Job + "^000000";	
	} else {
		mes "Base Exp : ^0000FFLevel too High^000000";
		mes "Job Exp : ^0000FFLevel too High^000000";
	}
	
	mes "Zeny : ^0000FF" + .@Mission_Zeny + "^000000";
	mes "Points : ^0000FF" + .@Mission_Points + "^000000";
//	mes "Cash Points : ^0000FF75^000000";

	return;
}


function	script	ValidateQuestTime	{
	.@hm_mission_tick = getarg(0,HM_MISSION_TICK);
	.@timeLeft = .@hm_mission_tick - gettimetick(2);

	if( .@timeLeft < $@QUEST_DEADLINE )
		return .@timeLeft;
	else
		return -1;
}


function	script	EpochToMs	{
	.@timeInSeconds = getarg(0,0);
	.@timeInMiliseconds = .@timeInSeconds * 1000;

	return .@timeInMiliseconds;
}

//
// Gera Hunting Mission
//
function	script	Build_Mission	{
	if (getarg(0, "") != "") {
		explode(.@MobID$, getarg(0), "|");
	} 
	//mission_settime gettimetick(2) + 86400;  wtf is this for??????
	// Mission Difficulty
	set HM_MISSION_TICK, gettimetick(2) + $@QUEST_DEADLINE;

	//debugmes "tick atual : " + gettimetick(2);
	//debugmes "hm_mission_tick : " + HM_MISSION_TICK;

	addtimer $@QUEST_DEADLINE * 1000,"HuntingSystem::OnQuestTimeIsOver";
	HM_IN_MISSION = 1;
	set .@HM_MISSION_RANK, ((HM_MISSION_RANK % 45) * 2) + 1;
	// Bonus Difficulty
	set .@Mission_Bonus, (HM_MISSION_RANK / 45) * 5;
	// Final Difficulty
	if( set(.@HM_MISSION_RANK, .@HM_MISSION_RANK + .@Mission_Bonus) > 99 ) 
		set .@HM_MISSION_RANK, 99;
	
	//.@lv = (BaseLevel * 10) / 39;
	.@lv = BaseLevel - 20;
	if (.@lv <= 0)
		.@lv = 1;
	else if(.@lv >= 75)
		.@lv = 75;

	.@maxlv = .@lv + 5;

	// if (.@maxlv > 80)
	// 	.@maxlv = 80
	// else if(.@maxlv < 1)
	// 	.@maxlv = 1;

	.@total_mob = query_sql( "select `id` from `mob_db` where `mode` & "+( MD_CANMOVE | MD_CANATTACK ) + " AND `MEXP` = 0 AND `DropCardid` > 0 AND `lv` BETWEEN " + .@lv + " AND " + .@maxlv + " AND `id` NOT IN ("+$@FORBIDEN_MOBS$+") ORDER BY RAND() LIMIT 5", .@mobid );
	
	for( set .@i, 1; .@i < 4; set .@i, .@i + 1 )
	{
		if(atoi(.@MobID$[.@i-1]) < 1) {
			set .@MobID, .@mobid[.@i-1];
		} else {
			set .@MobID, atoi(.@MobID$[.@i-1]);
		}
		// Mission Index - Mob ID - Mob Amount
		//set .@qtty, 40 - (.@HM_MISSION_RANK / 2) + (.@Mission_Bonus / 2);
		//if(.@qtty < 25) set .@qtty, 25;
		.@qtty = 30;
		//mission_sethunting .@i, .@MobID, .@qtty; wtf is this for???
		setd "HM_MISSION_ID" + .@i, .@MobID;
		setd "HM_MISSION_COUNT" + .@i, .@qtty;
	}

	return;
}


function	script	UpdateQuestRecords	{

	.@mobid = getarg(0,0);

	if(.@mobid == 0)
		end;

	.@map$ = strcharinfo(3);		// map name
	.@pid = getcharid(1); 			// Party ID

	.@pshare = getpartyshare(.@pid);
	if(!.@pid || .@pshare == 0) {
		for(.@i=1; .@i<4; .@i++) {
			set .@Mob, getd("HM_MISSION_ID" + .@i);
			if(.@Mob == .@mobid) {
				set .@qtt, getd("HM_MISSION_COUNT" + .@i);
				.@qtt--;
				if(.@qtt < 0)
					.@qtt = 0;
				setd "HM_MISSION_COUNT" + .@i, .@qtt;

				dispbottom("Hunting Quest: (" + (30 - .@qtt) + "/30) " + strmobinfo(2,.@Mobid)) + " killed.";

				if( HM_MISSION_COUNT1 + HM_MISSION_COUNT2 + HM_MISSION_COUNT3  < 1 ) {
					announce "Hunting Quest completed",bc_self;
					HM_IN_MISSION = 0;
				}
				break;
			}
		}
	} else {
		$@partymembercount = 0;
		deletearray $@partymemberaid[0],128;

		getpartymember .@pid,2;
		set .@members, $@partymembercount;
		copyarray .@GID[0], $@partymemberaid[0], .@members;

		.map$ = .@map$;
		.mobid = .@mobid;

		addrid(2,0,.@pid);
		.@map$ = .map$;
		.@mobid = .mobid;
		.@mapZ$ = strcharinfo(3);

		 if(.@mapZ$ == .@map$){
			for(.@i=1; .@i<4; .@i++) {
				set .@Mob, getd("HM_MISSION_ID" + .@i);
				if(.@Mob == .@mobid) {
					set .@qtt, getd("HM_MISSION_COUNT" + .@i);
					.@qtt--;
					if(.@qtt < 0)
						.@qtt = 0;
					setd "HM_MISSION_COUNT" + .@i, .@qtt;

						dispbottom("Hunting Quest: (" + (30 - .@qtt) + "/30) " + strmobinfo(2,.@Mobid)) + " killed.";

					if( HM_IN_MISSION && HM_MISSION_COUNT1 + HM_MISSION_COUNT2 + HM_MISSION_COUNT3 < 1 ) {
						announce "Hunting Quest completed",bc_self;
						HM_IN_MISSION = 0;
					}
					break;
				}
			}
		}

	}
	end;
}
