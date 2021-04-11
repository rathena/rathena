-	script	DiarioDeBordo	-1,{
	end;

OnPcLoginEvent:
	if(strcharinfo(3) == "new_1-1") end;
	if(countitem(24501) < 1 )
		getitem 24501,1;
		end;
}

function	script	isVs	{
	.@m$ = getarg(0);
	for(.@i=0;.@i<getarraysize($@MF_VS);.@i++)
		if(getmapflag(.@m$,$@MF_VS[.@i]))
			return 1;
	return 0;
}

function	script	RM_DiarioDeBordo	{

	if(inarray($@EventPlayers[0],getcharid(3)) >= 0) {
		mes "^0000FF[ Logbook ]^000000";
		mes "Sorry, the use of the Logbook is temporarily disabled during the event.";
		close;
	}

	if(callfunc("isVs",strcharinfo(3))) {
		mes "^0000FF[ Logbook ]^000000";
		mes "Sorry, the use of the Logbook is not allowed in this map.";
		close;
	}

	set .SG_RESET_COST, getvariableofnpc(.SG_RESET_COST,"DiarioDeBordo");
	getmapxy(.@mapName$,.@mapX,.@mapY,BL_PC,strcharinfo(0));

L_Menu:
	mes "^0000FF[ Logbook ]^000000";
	if( vip_status(1) ) mes "Account: ^00CCAA[ Premium ]^000000";
	else mes "Account: ^00AAFF[ Regular ]^000000";
	mes (#CASHPOINTS)?"Mania$: ^0000FF" +#CASHPOINTS+ "^000000.":"Mania$: ^FF000000.00^000000.";
	mes "-------------------";
	mes "Infamy: ^0000FF" + callfunc("F_InsertComma",readparam(136)) + " (+"+callfunc("F_InsertComma",readparam(136)/10000)+"% bonus)^000000";
	mes "Total PK Kills: ^0000FF" + callfunc("F_InsertComma",readparam(137)) + "^000000";
	mes "Total PK Deaths: ^0000FF" + callfunc("F_InsertComma",readparam(138)) + "^000000";
	mes "Total PK Score: ^0000FF" + callfunc("F_InsertComma",readparam(139)) + "^000000";
	mes "-------------------";
	mes "Total PvP Kills: ^0000FF" + callfunc("F_InsertComma",readparam(140)) + "^000000";
	mes "Total PvP Deaths: ^0000FF" + callfunc("F_InsertComma",readparam(141)) + "^000000";
	mes "Total PvP Score: ^0000FF" + callfunc("F_InsertComma",readparam(142)) + "^000000";
	mes "-------------------";
	mes "Total BG Wins: ^0000FF" + callfunc("F_InsertComma",readparam(143)) + "^000000";
	mes "Total BG Losses: ^0000FF" + callfunc("F_InsertComma",readparam(144)) + "^000000";
	mes "Total BG Ties: ^0000FF" + callfunc("F_InsertComma",readparam(145)) + "^000000";
	mes "-------------------";
	mes "Total WoE Score: ^0000FF" + callfunc("F_InsertComma",readparam(146)) + "^000000";
	mes "-------------------";
	mes "Personal Vote Points: ^0000FF[" + #VP_VOTEPOINTS + "/" + getvariableofnpc(.MinVotes4Reward,"v4p") + "]^000000";
	if(gettime(DT_DAYOFMONTH) < getvariableofnpc(.ThirdDday,"v4p"))	
		mes "Global VP: ^0000FF[" + $vp_totalVotePoints + "/" + getvariableofnpc(.VotePointsTarget,"v4p") + "]^000000";
	else if(gettime(DT_DAYOFMONTH) > getvariableofnpc(.ThirdDday,"v4p") && $vp_totalVotePoints > getvariableofnpc(.ThirdTarget,"v4p"))
		mes "Global Donation: ^0000FF[" + callfunc("F_InsertComma",$vp_donations) + "/" + callfunc("F_InsertComma",getvariableofnpc(.ExtensionCost,"v4p")) + "]^000000";
	.@days = (gettime(DT_DAYOFMONTH)>=getvariableofnpc(.ThirdDday,"v4p")?getvariableofnpc(.dayOfMonth,"v4p")-gettime(DT_DAYOFMONTH):(gettime(DT_DAYOFMONTH)>=getvariableofnpc(.SecondDday,"v4p")?getvariableofnpc(.ThirdDday,"v4p")-gettime(DT_DAYOFMONTH):getvariableofnpc(.SecondDday,"v4p")-gettime(DT_DAYOFMONTH)));
	mes "Voting Event ends in ^FF0000" + .@days + "^000000 days.";
	mes "-------------------";
	mes "^0000DDAdventurer's Challenge^000000";
	mes "Daily: " + F_DailyBar();
	mes "Weekly: " + F_WeeklyBar();
	mes "Monthly: " + F_MonthlyBar();
	next;

	L_Loop:
	switch(select("^f5b041Premium^000000 Services:Teleport Services")) {
		case 1: // PREMMY SERVICES
			mes "^0000FF[ Logbook ]^000000";
			if( !vip_status(1) && getgmlevel() < 10) {
				mes "Do you want to activate ^f5b041Premium^000000 on your account?";
				next;
				if(select("Not now:Yes, please") == 2) {
					callfunc "F_Premmy";
					end;
				} else {
					mes "^0000FF[ Logbook ]^000000";
					mes "I am afraid the options in this menu are for ^f5b041Premium^000000 players only.";
					next;
					goto L_Menu;
				}
			} 

			switch(select("Open storage:Open guild storage:Repair:Purify Ores:Utility store")){
				case 1: // Open storage
					if(inarray($@EventPlayers,getcharid(3)) >= 0 || .@vs = callfunc("F_isVSmap") == 1 || compare(strcharinfo(3),"new_") == 1) { 
						mes "Sorry, this service is not available now.";
						close;
					}
					mes "Connecting to Sweetie system...";
					close2;
					doevent "Sweetie::OnOpenStorage";
					end;
					break;
				case 2: // Open Guild Storage
					if(inarray($@EventPlayers,getcharid(3)) >= 0 || .@vs = callfunc("F_isVSmap") == 1 || compare(strcharinfo(3),"new_") == 1) { 
						mes "Sorry, this service is not available now.";
						close;
					}
					mes "Connecting to Sweetie system...";
					close2;
					doevent "Sweetie::OnOpenGuildStorage";
					end;
					break;
				case 3: // Repair all
					if(inarray($@EventPlayers,getcharid(3)) >= 0 || .@vs = callfunc("F_isVSmap") == 1 || compare(strcharinfo(3),"new_") == 1) { 
						mes "Sorry, this service is not available now.";
						close;
					} else {
						if(getbrokenid(1) == 0) {
							mes "All your equipments seem fine. No repairs are needed.";
							close;
						} 
						mes "We have some equipments in the inventory that need repairing. Repairs cost ^ff0000500z^000000 per equipment. Do you want to continue?";
						next;
						if(select("Yes, please:No, cancel it") == 2)
							close;
						else {
							while (.@id = getbrokenid(1)) {
								if(Zeny < 500) {
									dispbottom "Logbook: Not enough Zeny to continue. Aborting";
									break;
								}
								Zeny -= 500;
								.@cost += 500;
								repair(1);
								set .@i, .@i +1;
								dispbottom "Logbook: Equipment repaired ["+getitemname(.@id)+"]";
							}
							if (.@i) dispbottom "Logbook: " + .@i + " equipments repaired. Total cost: " + callfunc("F_InsertComma",.@cost)+"z.";
						}
						end;
					}
					break;
				case 4: //Refine ores
					mes "Connecting to the Legendary Smith system...";
					next;
					callfunc "massPurify","Legenday Smith";
					break;
				case 5:
					if( inarray($@EventPlayers,getcharid(3)) >= 0 || .@vs = callfunc("F_isVSmap") == 1 ) {  
						mes "Sorry, this service is not available now.";
						close;
					} else {
						mes "Select the store.";
						next;
						switch(select("^FF0000Sell Items^000000:Consumables:Miscellaneous:Arrows:Kunais:Ammo:Pet Foods:Pet Accessories")) {
							close2;
							case 1: callshop "Tools", 2; end;
							case 2: callshop "Consumables", 1; end;
							case 3: callshop "Miscellaneous", 1; end;
							case 4: callshop "Arrows", 1; end;
							case 5: callshop "Ninja", 1; end;
							case 6: callshop "Gunslinger", 1; end;
							case 7: callshop "Pet Foods", 1; end;
							case 8: callshop "Pet Accessories", 1; end;
						}
						end;
					}
					break;
				default:
					close;
				break;
			}
		case 2: // TELEPORTE
			if(inarray($@EventPlayers,getcharid(3)) >= 0 || .@vs = callfunc("F_isVSmap") == 1 || compare(strcharinfo(3),"new_") == 1) { 
				mes "Sorry, this service is not available now.";
				close;
			}
			switch(select("Cities:Specials")){
				case 1:
					menu "Prontera",T1, 
					"Alberta",T2, 
					"Aldebaran",T3, 
					"Amatsu",T4, 
					"Comodo",T7, 
					"Geffen",T13, 
					"Izlude",T16, 
					"Jawaii",T17, 
					"Morroc",T26, 
					"Payon",T30,
					"Yuno",T36;

					T1: warp "prontera",155,183; end;
					T2: warp "alberta",28,234; end;
					T3: warp "aldebaran",140,131; end;
					T4: warp "amatsu",198,84; end;
					T7: warp "comodo",209,143; end;
					T13: warp "geffen",119,59; end;
					T16: warp "izlude",128,114; end;
					T17: warp "jawaii",251,132; end;
					T26: warp "morocc",156,93; end;
					T30: warp "payon",179,100; end;
					T36: warp "yuno",157,51; end;
					break;
				case 2:
					menu "^f5b041Premium^000000 Room",X1,"Timeless Dungeons",X2;
					X1: warp "sala_premmy",40,38; end;
					X2: warp "yuno",282,288; end;
					break;
			}			
		default:
			close;
			break;
	}
}

// Sellers are now in premmy_quickmall.c
// -	shop	Utilities	-1,601:-1,602:-1,501:-1,502:-1,503:-1,504:-1,522:-1,523:-1,525:-1,610:-1,645:-1,656:-1,657:-1,1065:-1,611:-1
// -	shop	Arrows		-1,1750:1,1751:3,1752:3,1753:4,1770:2,1754:5,1755:5,1756:5,1757:5,1758:10,1759:10,1760:10,1761:10,1762:10,1763:10,1764:20,1765:25,1766:40,1767:5,1768:10,1772:5
// -	shop	Ammo	-1,13200:1,13201:15,13202:30,13203:80,13204:80,13205:80,13206:80,13207:80
// -	shop	Kunais		-1,13256:10,13294:100,13259:10,13258:10,13257:10,13255:10
// -	shop	Gemstones	-1,717:-1,716:-1,715:1200
// -	shop	Stones		-1,7521:-1,7522:-1,7523:-1,7524:-1
// -	shop	Alchemy		-1,657:-1,713:-1,937:-1,939:-1,952:-1,972:-1,7033:-1,905:-1,1092:-1,1059:-1,1061:2000,929:2000,1044:2000