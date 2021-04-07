-	script	HousingCfg	-1,{ 
	end;
OnInit:
	// Constantes com as coordenadas das entradas das casas
	// Localizacao do warp que leva da praca para o patio da casa
	setarray $@HDWarpMx[0],106,78,40,40,40,40,40,40,49,77,105,133,161,189,217,225,225,225,225,225,225,189,161,108,108,159,159;
	setarray $@HDWarpMy[0],42,42,49,77,105,133,161,189,225,225,225,225,225,225,225,188,160,132,104,76,48,42,42,108,159,159,108;
	// Localizacao do Respawn quando sai do patio da casa para a praca
	setarray $@HDRespMx[0],106,78,44,44,44,44,44,44,48,77,105,133,161,189,217,220,220,220,220,220,220,189,161,113,113,155,155;
	setarray $@HDRespMy[0],47,47,49,76,105,133,161,189,220,220,220,220,220,220,220,188,160,132,104,76,48,47,47,113,154,155,114;	
	
	// *** Uncommend these line to setup regions when the server runs for
	// *** the first time after a wipe.
		set $Region[23],1; // Midgardian Village

	// INFOSHEET //

	// #HOUSING_TIPS$[0]		: Gardener
	// #HOUSING_TIPS$[1]		: Housekeeper

	// #HOUSING$[0]				: House Id
	// #HOUSING$[1]				: Rend amount
	// #HOUSING$[2]				: Gardener
	// #HOUSING$[3] 			: Housekeeper
	// #HOUSING$[4]				: Claning Lady
	// #HOUSING$[5] 			: Oven
	// #HOUSING$[6] 			: Storage
	// #HOUSING$[7] 			: Beds
	// #HOUSING$[8]				: Wardrobe/Mirror
	end;
}


rent_mb,134,58,4	script	Properties Manager::mngr	1902,{
	function	ExtendContract; function	HireService; function	WipeData;
	if(atoi(#HOUSING$[0]) > 0 && atoi(#HOUSING$[1]) < gettimetick(2)) {
		WipeData();
		mes .n$;
		mes "Oh Lord, you've missed your contract renew date. I am afraid I have just rented your previous house to someone else.";
		next;
		mes .n$;
		mes "I was ver clear the first time: Not a second late!";
		next;
	} else {
		mes .n$;
		mes "Howdy, "+strcharinfo(0);
		mes "How ya' doing today?";
		next;
		mes .n$;
		mes "Busy as usual, I can see...";
		mes "Well, how can I be of your help, then?";
		next;
	}
	switch(select("I'd like to rent a house:About my contract")) {
	RentHouse:
	case 1:
		if(atoi(#HOUSING$[1]) > gettimetick(2)) {
			mes .n$;
			mes strcharinfo(0) + ", my dear, I am sorry but you can only have one house at the time.";
			next;
			mes .n$;
			mes "Your place at number ^AA11AA"+#HOUSING$[0]+"^000000 is paid and yours to use for another ^AA11AA"+ callfunc("Time2Str",atoi(#HOUSING$[1]))+"^000000.";
			mes "Do you wanna extent your contract now?";
			next;
			if(select("Yes, please.:No, not now. Thanks") == 2)
				close;
			ExtendContract();
			mes .n$;
			mes "All done! Thank you very much!";
			mes "Your new contract ends in ^AA11AA" + callfunc("Time2Str",atoi(#HOUSING$[1])) + "^000000.";
			close;
			end;
		}
		mes .n$;
		mes "Of course, why not?";
		mes "Well, it's been pretty busy, recently...";
		next;
		mes .n$;
		mes "Because of that the prices are a bit in a steep right now.";
		next;
		mes .n$;
		mes "But worry not!";
		mes "I have this lovely two-bedroom house available for a very competitive price.";
		mes "Not sale, though. Renting.";
		next;
		mes .n$;
		mes "That one will cost you "+F_InsertComma(.Rent)+"z per week.";
		mes "What do you think?";
		next;
		if(select("That is fine, I will take it.:No, that is too much.") == 2) {
			mes .n$;
			mes "I understand... Just don't wait too much. They are limited, you know...";
			close;
		}
		mes .n$;
		mes "Fantastic!";
		mes "It will be "+F_InsertComma(.Rent)+"z paid in advance, please.";
		mes "I will then show you your new home!";
		next;
		if(select("Pay now:Cancel") == 2)
			close;
		ExtendContract();
		mes .n$;
		mes "Perfect!";
		mes "Your new place is ^AA5500house number " + #HOUSING$[0] + "^000000.";
		next;
		mes .n$;
		mes "Are you excited to meet the place? I bet you are gonna love it and this neighbourhood is so lively!";
		next;
		mes .n$;
		mes "Remember! Your rent is due to renew in ^5599AA" + callfunc("Time2Str",atoi(#HOUSING$[1])) + "^000000.";
		next;
		mes .n$;
		mes "I'd appreciate if you paid before the deadline. I won't tolerate any delays and will take the house back the second after the expiration time.";
		next;
		mes .n$;
		mes "If you force me to do that, you are gonna lose any improvements you've carried out in the house may you want to rent it again.";
		next;
		mes .n$;
		mes "You can always pay rent in advance, though. Any time, just talk to me and we can extend your contract.";
		next;
		mes .n$;
		mes "Well, that covers every thing, I belive.";
		mes "Should we meet at the house?";
		mes "See you there!";
		close;
		break;
	case 2:
		if(getarraysize(#HOUSING$) == 0) {
			mes .n$;
			mes "I am afraid I could not find anything on your name in my records...";
			close;
		}
		mes .n$;
		mes "^AA0000-- Contract for House # " + #HOUSING$[0] + " --^000000";
		mes "Rent: " + (atoi(#HOUSING$[1])<gettimetick(2)?F_InsertComma(.Rent)+"z p/w":callfunc("Expire",atoi(#HOUSING$[1]))+ " ("+.Rent+"z p/w)");
		mes "Gardener: " + (atoi(#HOUSING$[2])<gettimetick(2)?F_InsertComma(.Gardener)+"z p/w":callfunc("Expire",atoi(#HOUSING$[2]))+ " ("+.Gardener+"z p/w)");
		mes "Housekeeper: " + (atoi(#HOUSING$[3])<gettimetick(2)?F_InsertComma(.Housekeeper)+"z p/w":callfunc("Expire",atoi(#HOUSING$[3]))+" ("+.Housekeeper+"z p/w)");
		.@total = callfunc("F_CalcCosts");
		mes "Total per Week: " + F_InsertComma(.@total);
		next;
		switch(select("Renew Rent:Gardening Services:Housekeeping Services:Cancel")) {
		case 1:
			mes .n$;
			if( atoi(#HOUSING$[0]) == 0 || atoi(#HOUSING$[1]) < gettimetick(2) ) {
				// First time or after expiration
				mes .n$;
				mes "I am afraid we don't have any contracts in your name. You may want to rent a house, maybe?";
				next;
				goto RentHouse;
			} else if(atoi(#HOUSING$[1]) > gettimetick(2)) {
				mes .n$;
				mes "Absolutely, that will be ^AA11AA"+F_InsertComma(.Rent)+"^000000z for another week of rent.";
				next;
				ExtendContract();
				mes .n$;
				mes "All done! Thank you very much!";
				mes "Your new contract ends in ^AA11AA" + callfunc("Time2Str",atoi(#HOUSING$[1])) + "^000000.";
				close;
			}
			break;
		case 2:
			mes .n$;
			if(atoi(#HOUSING$[2]) < gettimetick(2)) {
				mes "Do you wanna hire a gardener to help you with the chores and perhaps even plot the land and produce a some delicious fruits and veggies?";
				next;
				mes .n$;
				mes "We have Maia available, she is very good. Reliable, hard worker...";
				mes "She is available for " + F_InsertComma(.Gardener) + "z per week.";
				next;
				if(select("Hire Maia:Cancel") == 2)
					goto Dismiss;
				else {
					HireService(2,.Gardener);
					mes .n$;
					mes "All done... She is ready to start.";
					mes "Thank you for your custom.";
					close;
				}
			} else {
				mes "Do you want to extend your contract with ^AA00AAMaia^000000 for another week? That would be another "+ F_InsertComma(.Gardener) +"z. Payment in advance. Always.";
				next;
				if(select("Yes, please:No, Thank you") == 2)
					goto Dismiss;
				HireService(2,.Gardener);
				mes .n$;
				mes "All done! Thank you very much!";
				mes "Your new contract with ^AA00AAMaia^000000 ends in ^AA11AA" + callfunc("Time2Str",atoi(#HOUSING$[2])) + "^000000.";
				close;
			}
			break;
		case 3:
			mes .n$;
			if(atoi(#HOUSING$[3]) < gettimetick(2)) {
				mes "Do you wanna hire a housekeeper to help you with the pool? I bet there are a couple of very interesing things one can do with a pool around.";
				next;
				mes .n$;
				mes "We have Alfonso available, he is highly recommended!";
				mes "He is available for " + F_InsertComma(.Housekeeper) + "z per week.";
				next;
				if(select("Hire Alfonso:Cancel") == 2)
					goto Dismiss;
				else {
					if(Zeny < .Housekeeper)
						goto OnNoZeny;
					set Zeny, Zeny - .Housekeeper;
					set #HOUSING$[3],gettimetick(2)+.Period;
					mes .n$;
					mes "All done... He is on his way!";
					mes "Thank you for your custom.";
					close;
				}
			}
			break;
		case 4:
			close;
			break;
		}

	}
	end;

OnNoZeny:
	mes .n$;
	mes "Oh, " + strcharinfo(0) + ", what is going on here? You don't seem to have enough Zeny for this.";
	close;

Dismiss:
	mes .n$;
	mes "No problem, I understand.";
	mes "See you around!";
	close;

OnInit:
	.n$ 		= "^0000FF[ Properties Manager ]^000000";
	.Rent 		= 40000;
	.Period 	= 604800; 	// 7 dias
	.Gardener 	= 12000;
	.Housekeeper= 18000;
	end;

	function	ExtendContract	{
		if(Zeny < .Rent){
			goto OnNoZeny;
			end;
		}
		set Zeny, Zeny - .Rent;
		if(atoi(#HOUSING$[1]) < gettimetick(2)) {
			set #HOUSING$[0], rand(1,23);				// House Id
			set #HOUSING$[1], gettimetick(2) + .Period;	// Rent amount
			set #HOUSING$[2], 0; 						// Gardener
			set #HOUSING$[3], 0; 						// Housekeeper
			set #HOUSING$[4], 0; 						// Claning Lady
			set #HOUSING$[5], 0; 						// Oven
			set #HOUSING$[6], 0; 						// Storage
			set #HOUSING$[7], 0; 						// Beds
			set #HOUSING$[8], 0; 						// Wardrobe/Mirror
		} else {
			set #HOUSING$[1], atoi(#HOUSING$[1]) + .Period;	// Rent amount
		}
		return;
	}

	function	HireService	{
		.@index 	= getarg(0);
		.@cost		= getarg(1);
		if(Zeny < .@cost) {
			goto OnNoZeny;
			end;
		}
		set Zeny, Zeny - .@cost;
		if(atoi(#HOUSING$[.@index]) < gettimetick(2)) { // New Service or it was expired
			switch(atoi(#HOUSING$[.@index])) {
			case 2: // Gardener
				setarray #HOUSING_CROPS$[0],		"0","0","0","0","0","0","0","0","0","0";
				setarray #HOUSING_CROPS_QTY$[0],	"0","0","0","0","0","0","0","0","0","0";
				set #HOUSING_TIPS$[0],				"0";
				break;
			case 3: // Housekeeper
				break;
			}
			set #HOUSING$[.@index],gettimetick(2) + .Period;
		} else { // Renewing before expiration
			set #HOUSING$[.@index],atoi(#HOUSING$[.@index]) + .Period;
		}
		return;
	}

	function	WipeData	{
		// services array
		deletearray #HOUSING$[0],getarraysize(#HOUSING$);
		deletearray #HOUSING_TIPS$[0], getarraysize(#HOUSING_TIPS$);
		// gardener-related array
		deletearray #HOUSING_CROPS$[0],getarraysize(#HOUSING_CROPS$);
		deletearray #HOUSING_CROPS_QTY$[0],getarraysize(#HOUSING_CROPS_QTY$);

		return;
	}
}


// ********************************************
// Plaquinhas das casas
// ********************************************
rent_mb,111,45,1	script	House Plate#1	837,{
	mes "[Home Sweet Home]";
	mes "Welcome to my house!";
	mes " ";
	mes "House Number ^0000FF *~~ [" + strnpcinfo(2) + "] ~~* ^000000";
	close;
OnInit:
	waitingroom "House # "+strnpcinfo(2),0;
}



// ********************************************
// Housing PostBox
// ********************************************
-	script	MailBox#Mid	-1,{
	
	if(((getcharid(5) < 4) && (strnpcinfo(4) == "rent_ma")) || ((getcharid(5) > 3) && (strnpcinfo(4) == "rent_mb"))) {
		dispbottom "I have no interest in these bastards mails...";
		end;
	}
	
	if(HOUSING$[0] != strnpcinfo(2)) {
		dispbottom "I cannot read other people's mails... Shame on me!";
		end;
	}
  openmail;
  end;
}


// ********************************************************************
// Housing Warps 

// ********************************************************************
// House Entrance (door steps, outside -> in)
// ********************************************************************
rentb1,19,22,0	script	PortaDeCasa#b1	45,2,2,{
	end;

OnInstanceInit:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnTouch:
	.@map$ = "rentin"+strnpcinfo(2);
	warp instance_mapname(.@map$,instance_id(IM_CHAR)),26,7;
	end;
}

// ********************************************************************
// House Entrance (door steps, inside -> out)
// ********************************************************************
rentinb1,34,3,0	script	FrontDoorIn#b1	45,2,2,{
	end;

OnInstanceInit:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnTouch:
	.@map$ = "rent"+strnpcinfo(2);
	warp instance_mapname("rentb1",instance_id(IM_CHAR)),20,17;
	end;
}

// ********************************************************************
// Front Gate of the House (inside -> out)
// ********************************************************************
rentb1,25,2,0	script	FrontGate#rent_mb	45,2,2,{
	end;

OnInstanceInit:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnTouch:
	warp strnpcinfo(2),$@HDRespMx[atoi(#HOUSING$[0])-1],$@HDRespMy[atoi(#HOUSING$[0])-1];
	if (instance_id(IM_CHAR))
		instance_destroy;

	end;
}


// ********************************************************************
// Front Gate of the House (outside -> in)
// ********************************************************************

rent_mb,106,42,0	script	PortaoDaCasa#1	45,2,2,{
	end;

OnTouch:
	if(atoi(#HOUSING$[0]) == 0)
		message strcharinfo(0),"What am I doing? I don't even have a house! ";
	else if(#HOUSING$[0] != strnpcinfo(2))
		message strcharinfo(0),"Omg! What am I doing? My house is number " + #HOUSING$[0];
	else
		callfunc "F_HouseInstancing",atoi(strnpcinfo(2));

	end;
}








// ********************************************************************
// Weed, Seaweed and Pest
// ********************************************************************

rentb1,30,15,0	script	weed	1083,{
end;

OnInstanceInit:
OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

//OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;
}



rentb1,16,12,0	script	seaweed	1579,{
end;

OnInstanceInit:
OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

//OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;
}



rentinb1,4,4,0	script	#rentb1	-1,{
	end;
OnInstanceInit:
	areamonster instance_mapname(strnpcinfo(4)),4,12,14,23,"Slug",1007,1,instance_npcname("#"+strnpcinfo(2))+"::OnMobDead";
	end;

OnMobDead:
	areamonster instance_mapname(strnpcinfo(4)),4,12,14,23,"Slug",1007,1,instance_npcname("#"+strnpcinfo(2))+"::OnMobDead";
	end;
	
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;
}



// ********************************************************************
// Storage
// ********************************************************************

rentinb1,9,5,0	script	#Storage	844,{
	if (getcharid(0) == getpartyleader(getcharid(1),2)) {
		callfunc("F_CheckKafCode");	//check your storage password, if set
		openstorage;
	} else {
		dispbottom "I shouldn't be messying with my friend's stuff...";
	}
	end;

OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;
}


// ********************************************************************
// Beds
// ********************************************************************

rentinb1,26,34,0	script	#Beds	844,{
	if (getcharid(0) == getpartyleader(getcharid(1),2)) {
		switch(select("Rest:Cancel")) {
		case 1:
			message strnpcinfo(0), "Hope I have one of those dreams....";
			sleep2 5000;
			percentheal 100,100;
			specialeffect2 EF_BLESSING; sc_start SC_BLESSING,600000,10;
			specialeffect2 EF_INCAGILITY; sc_start SC_INCREASEAGI,600000,10;
			if (countitem(521) >= 1){
				delitem 521,1;
				warp "dreamland",128,132;
			}
			end;
			break;
		case 2:
			end;
			break;
		}
	} else {
		dispbottom "I shouldn't be messying with my friend's stuff...";
	}
	end;

OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;
}



// ********************************************************************
// Oven
// ********************************************************************

rentinb1,6,7,0	script	#Oven	844,{
	if (getcharid(0) == getpartyleader(getcharid(1),2)) {
		if(countitem(6251) > 0){
			delitem 6251,1;
			cooking 16;
		} else {
			message strcharinfo(0), "Hum... I am afraid I'll need some " + getitemname(6251) + " first.";
		}
		end;
	} else {
		dispbottom "I shouldn't be messying with my friend's stuff...";
	}
	end;

OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;

}


// ********************************************************************
// Library
// ********************************************************************
-	script	Books#inscription	-1,{
	callfunc "Inscription_Info",InscriptionLvl;
	end;

OnInstanceInit:
OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;

}


// ** Duplications
// ********************************************************************


// Plaquinhas das casas
// ******************************************************************

// Midigardians Village
rent_mb,83,46,1	duplicate(House Plate#1)	House Plate#2	837
rent_mb,43,53,1	duplicate(House Plate#1)	House Plate#3	837
rent_mb,43,72,1	duplicate(House Plate#1)	House Plate#4	837
rent_mb,43,100,1	duplicate(House Plate#1)	House Plate#5	837
rent_mb,43,128,1	duplicate(House Plate#1)	 House Plate#6	837
rent_mb,43,156,1	duplicate(House Plate#1)	House Plate#7	837
rent_mb,43,184,1	duplicate(House Plate#1)	House Plate#8	837
rent_mb,44,222,1	duplicate(House Plate#1)	House Plate#9	837
rent_mb,72,222,1	duplicate(House Plate#1)	House Plate#10	837
rent_mb,100,222,1	duplicate(House Plate#1)	House Plate#11	837
rent_mb,128,222,1	duplicate(House Plate#1)	House Plate#12	837
rent_mb,156,222,1	duplicate(House Plate#1)	House Plate#13	837
rent_mb,184,222,1	duplicate(House Plate#1)	House Plate#14	837
rent_mb,212,222,1	duplicate(House Plate#1)	House Plate#15	837
rent_mb,222,193,1	duplicate(House Plate#1)	House Plate#16	837
rent_mb,222,165,1	duplicate(House Plate#1)	House Plate#17	837
rent_mb,222,137,1	duplicate(House Plate#1)	House Plate#18	837
rent_mb,222,109,1	duplicate(House Plate#1)	House Plate#19	837
rent_mb,222,81,1	duplicate(House Plate#1)	House Plate#20	837
rent_mb,222,53,1	duplicate(House Plate#1)	House Plate#21	837
rent_mb,193,45,1	duplicate(House Plate#1)	House Plate#22	837
rent_mb,165,45,1	duplicate(House Plate#1)	House Plate#23	837

// Caixas de Correio
// ******************************************************************

// *** Midgardian Village
rent_mb,102,45,0	duplicate(MailBox#Mid)	Mailbox#1	888
rent_mb,74,45,0	duplicate(MailBox#Mid)	Mailbox#2	888
rent_mb,43,44,0	duplicate(MailBox#Mid)	Mailbox#3	888
rent_mb,43,81,0	duplicate(MailBox#Mid)	Mailbox#4	888
rent_mb,44,109,0	duplicate(MailBox#Mid)	Mailbox#5	888
rent_mb,43,137,0	duplicate(MailBox#Mid)	Mailbox#6	888
rent_mb,43,165,0	duplicate(MailBox#Mid)	Mailbox#7	888
rent_mb,43,193,0	duplicate(MailBox#Mid)	Mailbox#8	888
rent_mb,53,222,0	duplicate(MailBox#Mid)	Mailbox#9	888
rent_mb,81,222,0	duplicate(MailBox#Mid)	Mailbox#10	888
rent_mb,109,222,0	duplicate(MailBox#Mid)	Mailbox#11	888
rent_mb,137,222,0	duplicate(MailBox#Mid)	Mailbox#12	888
rent_mb,165,222,0	duplicate(MailBox#Mid)	Mailbox#13	888
rent_mb,193,222,0	duplicate(MailBox#Mid)	Mailbox#14	888
rent_mb,221,222,0	duplicate(MailBox#Mid)	Mailbox#15	888
rent_mb,222,184,0	duplicate(MailBox#Mid)	Mailbox#16	888
rent_mb,222,156,0	duplicate(MailBox#Mid)	Mailbox#17	888
rent_mb,222,128,0	duplicate(MailBox#Mid)	Mailbox#18	888
rent_mb,222,100,0	duplicate(MailBox#Mid)	Mailbox#19	888
rent_mb,222,72,0	duplicate(MailBox#Mid)	Mailbox#20	888
rent_mb,222,44,0	duplicate(MailBox#Mid)	Mailbox#21	888
rent_mb,184,45,0	duplicate(MailBox#Mid)	Mailbox#22	888
rent_mb,156,45,0	duplicate(MailBox#Mid)	Mailbox#23	888

// Portao da casa (main entrance warp)
// ******************************************************************

// Midgardian Village
rent_mb,106,42,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#1	45,2,2
rent_mb,78,42,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#2	45,2,2
rent_mb,40,49,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#3	45,2,2
rent_mb,40,77,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#4	45,2,2
rent_mb,40,105,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#5	45,2,2
rent_mb,40,133,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#6	45,2,2
rent_mb,40,161,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#7	45,2,2
rent_mb,40,189,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#8	45,2,2
rent_mb,49,225,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#9	45,2,2
rent_mb,77,225,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#10	45,2,2
rent_mb,105,225,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#11	45,2,2
rent_mb,133,225,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#12	45,2,2
rent_mb,161,225,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#13	45,2,2
rent_mb,189,225,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#14	45,2,2
rent_mb,217,225,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#15	45,2,2
rent_mb,225,188,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#16	45,2,2
rent_mb,225,160,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#17	45,2,2
rent_mb,225,132,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#18	45,2,2
rent_mb,225,104,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#19	45,2,2
rent_mb,225,76,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#20	45,2,2
rent_mb,225,48,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#21	45,2,2
rent_mb,189,42,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#22	45,2,2
rent_mb,161,42,0	duplicate(PortaoDaCasa#1)	PortalDaCasa#23	45,2,2


// Portas e Portoes das casas
// ******************************************************************
// rentb1,19,23,0	duplicate(PortaDeCasa#a1)	PortaDeCasa#b1	45,2,2
// rentinb1,33,3,0	duplicate(FrontDoorIn#a1)	FrontDoorIn#b1	45,2,2
// rentb1,25,2,0	duplicate(FrontGate#rent_ma)	FrontGate#rent_mb	45,2,2


// Properties Manager and House Appliances
// ******************************************************************
// rent_mb,134,62,4	duplicate(StreetDealer)	Midgardian Manager#Mng::ManagerMid	5500,4,4
//rentb1,34,17,4	duplicate(Gardener)	Maia#rentb1	1903	

renta1,4,17,4	duplicate(#Housekeeper)	#HousekeeperB	5502
rentina1,31,26,0	duplicate(#Storage)	#StorageB	844
rentina1,42,26,0	duplicate(#Beds)	#BedsB	844
rentina1,10,14,0	duplicate(#Oven)	#OvenB	844

// Weed
//rentb1,30,15,0	duplicate(weed)	weed#1	1083
rentb1,33,13,0	duplicate(weed)	weed#2	1078
rentb1,29,11,0	duplicate(weed)	weed#3	1081
rentb1,32,10,0	duplicate(weed)	weed#4	1084
rentb1,34,6,0	duplicate(weed)	weed#5	1083
rentb1,31,6,0	duplicate(weed)	weed#6	1118
//rentb1,15,10,0	duplicate(seaweed)	seaweed#1	1579
rentb1,15,10,0	duplicate(seaweed)	seaweed#2	1216
rentb1,13,8,0	duplicate(seaweed)	seaweed#3	1161
rentb1,9,9,0	duplicate(seaweed)	seaweed#4	1579
rentb1,7,10,0	duplicate(seaweed)	seaweed#5	1216
rentb1,7,6,0	duplicate(seaweed)	seaweed#6	1161

renta1,30,15,0	duplicate(weed)	weed#b1	1083
renta1,33,13,0	duplicate(weed)	weed#b2	1078
renta1,29,11,0	duplicate(weed)	weed#b3	1081
renta1,32,10,0	duplicate(weed)	weed#b4	1084
renta1,34,6,0	duplicate(weed)	weed#b5	1083
renta1,31,6,0	duplicate(weed)	weed#b6	1118
renta1,6,12,0	duplicate(seaweed)	seaweed#b1	1579
renta1,9,14,0	duplicate(seaweed)	seaweed#b2	1216
renta1,7,16,0	duplicate(seaweed)	seaweed#b3	1161
renta1,7,14,0	duplicate(seaweed)	seaweed#b4	1579
renta1,7,10,0	duplicate(seaweed)	seaweed#b5	1216
renta1,7,6,0	duplicate(seaweed)	seaweed#b6	1161
rentina1,4,4,0	duplicate(#rentb1)	#renta1	-1



// // Inscription books in the house;
// rentina1,32,7,0	duplicate(Books#inscription)	Inscription Books#Mid	844
// rentinb1,17,33,0	duplicate(Books#inscription)	Inscription Books#Bal	844


// Intro Only NPCs
renta1,24,10,4	duplicate(#b1)	#a1	5500,4,4	
rentina1,30,12,4	duplicate(Manager#inb)	Manager#ina	5500,4,4





// ********************************************************************
// Properties Manager - Inside House
// ********************************************************************

rentb1,24,12,4	script	#b1	1902,3,5,{
end;

OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
	set 'name$, instance_npcname(strnpcinfo(0));
	setnpcdisplay('name$, "??");
	end;

OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;

OnTouch:
//	if(intro < 6) intro = 2; // intro is 2 after the Misty Woods introduction
	switch('intro) {
	case 0:
//		set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
//		set 'name$, instance_npcname(strnpcinfo(0));
		pcblockmove getcharid(3),1; // impede o char de se mover
//		setnpcdisplay('name$, "??");
		sleep2 2000;
		npctalk "Welcome "+strcharinfo(0);
		sleep2 3000;
		unittalk getcharid(3), "... Sorry, do I know you?";
		sleep2 3000;
		npctalk "Pardon me.. I am Moreth, the Properties Manager. Nice to meet you!";
		sleep2 5000;
		unittalk getcharid(3), "Oh, of course! Sorry... Nice to meet you too, Mr. Moreth!";
		setnpcdisplay('name$, "Manager");
		sleep2 4000;
		npctalk "No Problem! Here... come closer, please...";
		sleep2 3000;
		pcblockmove getcharid(3),0;
		////unitwalk getcharid(3),'gid; TODO
		sleep2 2000;
		mes "^0000FF[ Manager ]^000000";
		mes "How are you doing?";
		mes "I mean, It must have been";
		mes "very difficult for you";
		mes "as it has been for the others";
		mes "to be forced to move";
		mes "to a new place, new house.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "But you will be allright!";
		mes "You are better here. Safer.";
		next;
		select("Thank you Mr. Moreth");
		mes "^0000FF[ Manager ]^000000";
		mes "No Worries...";
		mes "We take care of each other here.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "And the neighbourhood";
		mes "is really nice and welcomming.";
		mes (getcharid(5)<4)?"They are also ^0000FFMidgardians^000000, just like you.":"They are also ^0000FFBaldurians^000000 just like you.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "Ok! Lests move on!";
		mes "There are a couple of things";
		mes "I'd like to show you";
		mes "before I go and also";
		mes "you must be really tired.";
		close2;
		'intro = 1;
		pcblockmove getcharid(3),1;
		npcwalkto 32,15; //31,19
		sleep2 2000;
		pcblockmove getcharid(3),0;
		
		// if(getcharid(5)<4)
		// 	//unitwalk getcharid(3),30,16;
		// else
		//  	pcfollow getcharid(3), 'gid;
		
		end;
		break;
	case 1:
		pcblockmove getcharid(3),1;
		npctalk "Oh! Hello Maia! You still here?";
		sleep2 2000;
		mes "^0000FF[ Manager ]^000000";
		mes "Maia is a ^990066Gardener^000000. A good one.";
		mes "She used to work for the previous";
		mes "owner of this house before he";
		mes "err...";
		mes "...died?!";
		next;
		donpcevent instance_npcname((getcharid(5)<4)?"Maia#rentb1":"Maia#renta1")+"::OnTalk";
		sleep2 4000;
		npctalk "That's what I've said, dear Maia...";
		sleep2 3000;
		mes "^0000FF[ Manager ]^000000";
		mes "Well, you'd may consider";
		mes "to hire a gardener to take care";
		mes "of your house...";
		mes "They are really handy.";
		next;
		npctalk "Maia... May I ask you to leave? Talk to you later.";
		sleep2 3000;
		donpcevent instance_npcname((getcharid(5)<4)?"Maia#rentb1":"Maia#renta1")+"::OnWalkAway";
		mes "^0000FF[ Manager ]^000000";
		mes "In case you're interested";
		mes "Come talk to me when it's";
		mes "convenient for you.";
		mes " ";
		mes "I am always around";
		mes "at the Village Centre.";
		close2;
		'intro = 2; // Pq ele passa pelo char enqto ta indo pra piscina. Senao buga tudo.
		npcwalkto 18,14;
		sleep2 1000;
		npctalk "The pool! What a lovely pool, huh? Do you like pool parties?";
		sleep2 1000;
		pcstopfollow getcharid(3); // Senao ele para antes de tocar no OnTouch
		pcblockmove getcharid(3),0;
		//unitwalk getcharid(3),20,13;
//		//unitwalk getcharid(3), 'gid;
		'intro = 3;
		end;
		break;
	
	case 2:
		end;
	break;
	
	case 3:
		pcblockmove getcharid(3),1;
		pcfollow getcharid(3), 'gid;  //so pra teste. qqr coisa deleta
		unittalk getcharid(3), "Oh yes! Sure thing!! Who doesn't, huh?";
//		sleep2 2000;
		mes "^0000FF[ Manager ]^000000";
		mes "Well, the gardeners don't";
		mes "mantain pools. So you'll need";
		mes "a ^990066Housekeeper^000000 for that";
		mes "... we can also provide it, though.";
//		next;
		close2;
		npcwalkto 7,14;
		sleep2 1000;
		'intro = 4;
		pcblockmove getcharid(3),0;
//		//unitwalk getcharid(3), 'gid;
		//unitwalk getcharid(3),10,14;
//		pcfollow getcharid(3), 'gid;

		end;
		break;
		
	case 4:
		mes "^0000FF[ Manager ]^000000";
		mes "Err... That is Borba.";
		next;
		donpcevent instance_npcname((getcharid(5)<4)?"#Housekeeper":"#HousekeeperB")+"::OnTalk";
		mes "^0000FF[ Manager ]^000000";
		mes "...";
		emotion ET_HNG;
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "What can I say...";
		mes "The housekeeper, he does his job.";
		mes "He will keep your pool clean";
		mes "and also the patio.";
		next;
		npctalk "Borba! I want you gone when I am back, got it?";
		sleep2 2000;
		mes "^0000FF[ Manager ]^000000";
		mes "If you want to hire a housekeeper,";
		mes "again, look for me at the";
		mes "Village's plaza.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "Now, come with me...";
		mes "Let's see inside the house.";
		close2;
		'intro = 5;
		pcstopfollow getcharid(3);
		pcblockmove getcharid(3),1;
		//npcwalkto 19,20;
		////unitwalk 'gid, getnpcid(0,instance_npcname((getcharid(5)<4)?"PortaDeCasa#b1":"PortaDeCasa#a1")); TODO
		sleep2 2000;
		'intro = 6;
		pcblockmove getcharid(3),0;
		//unitwalk getcharid(3), getnpcid(0,instance_npcname((getcharid(5)<4)?"PortaDeCasa#b1":"PortaDeCasa#a1"));
		end;

	case 5:
		end;
		break;
	
	case 6:
//		pcstopfollow getcharid(3);
//		warp instance_mapname("rentinb1"),26,7;
//		setnpcdisplay(instance_npcname("Manager"), 'name$);
		disablenpc instance_npcname((getcharid(5)<4)?"#Housekeeper":"#HousekeeperB");
		disablenpc 'name$;
		end;
	}
}

// ********************************************************************
// Properties Manager - Inside House - Cont.
// ********************************************************************

rentinb1,30,12,4	script	Manager#inb	5500,3,3,{
end;

OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	end;

OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;
	
OnTouch:
	switch('intro) {
	case 6:
		set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
		pcfollow getcharid(3),'gid; 
		mes "^0000FF[ Manager ]^000000";
		mes "Look at this place!";
		mes "Wonderful, isn't it?";
		mes "Are you impressed?";
		next;
		select("One could say so...");
		mes "^0000FF[ Manager ]^000000";
		mes "These new houses are imaculate!";
		mes "Hope you take care of yours";
		mes "specially during paries, huh?";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "Is is still missing some pieces";
		mes "of furniture though";
		mes " ";
		mes "Let me show you very quckily.";
		close2;
		
		if(getcharid(5)<4)
//		if(strnpcinfo(4) == "rentinb1")
			npcwalkto 10,5;
		else
			npcwalkto 31,26;
			
		sleep2 3000;
		'intro = 7;
	//	pcblockmove getcharid(3),1;
		end;
		break;
		
	case 7:
		'intro = 8; // O npc vai passar pelo char
		mes "^0000FF[ Manager ]^000000";
		mes "Here you have some enough space";
		mes "to install a ^990066Storage^000000";
		mes " ";
		mes "We can provide that. Just come";
		mes "and talk to me later if you want.";
		if(getcharid(5)<4){
//		if(strnpcinfo(4) == "rentinb1") {
			next;
			npcwalkto 6,8;	
		} else {
			next;
			set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
			//unitwalk 'gid,37,23;
			sleep2 2000;
			mes "^0000FF[ Manager ]^000000";
			mes "These beds need to be replaced.";
			mes "We still have some in stock.";
			next;
			mes "^0000FF[ Manager ]^000000";
			mes "There is no illness that";
			mes "resist to a good sleeping,";
			mes "don't you agree?";
			next;
			emotion ET_SMILE;
			close2;
			
			set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
			//unitwalk 'gid, 25,23;
			sleep2 2000;
			//unitwalk 'gid,11,15;
//			sleep2 1000;
			pcfollow getcharid(3),'gid; 
			sleep2 4000;
		}

		mes "^0000FF[ Manager ]^000000";
		mes "Here you can install an ^990066Oven^000000.";
		mes "I highly recommend that!";
		mes "With the famine at our door";
		mes "it is always good to stock";
		mes "suplies and cook your own food.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "Ah! We sell those too.";
		close2;
//		'intro = 8;
		
		if(getcharid(5)<4) {
			set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
			//unitwalk 'gid,17,10;
			sleep2 2000;
			//unitwalk 'gid,21,22;
			sleep2 1000;
			//unitwalk 'gid,28,28;
			
		} else {
			set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
			//unitwalk 'gid,29,32;
		}
		sleep2 3000;
		'intro = 9;
		end;
		break;
		
	case 8:
		end;
		break;
		
	case 9:
		if(getcharid(5)<4){
//		if(strnpcinfo(4) == "rentinb1") {
			mes "^0000FF[ Manager ]^000000";
			mes "These beds need to be replaced.";
			mes "We still have some in stock.";
			next;
			mes "^0000FF[ Manager ]^000000";
			mes "There is no illness that";
			mes "resist to a good sleeping,";
			mes "don't you agree?";
			next;
			emotion ET_SMILE;
		}
		mes "^0000FF[ Manager ]^000000";
		mes "I think that is it, "+strcharinfo(0)+".";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "I shall leave now";
		mes "and visit the other new";
		mes "villagers that just arrived.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "Wish you all the best";
		mes "and that you enjoy your stay";
		mes "to its must!";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "If is there anything I can do";
		mes "for you, please, don't ";
		mes "hesitate to contact me.";
		mes "I am aways at the Village Plaza.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "Well... not aways.";
		mes " ";
		mes "^FF0000If you hear the sirens";
		mes "run to your house!!";
		mes "That means we are being atacked";
		mes (getcharid(5)<4)?"by the Baldurians.^000000":"by the Midgardians.^000000";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "You can help the ^990066Dandelions^000000";
		mes "to defend our Village, though.";
		next;
		mes "^0000FF[ Manager ]^000000";
		mes "I mean, if you feel strong and";
		mes "confident enough for that, of course.";
		close2;
		'intro = 10;
		pcstopfollow getcharid(3);
		pcblockmove getcharid(3),1;
		set 'gid, getnpcid(0,instance_npcname(strnpcinfo(0)));
		if(getcharid(5)<4) {
			//unitwalk 'gid, 20,22;
			sleep2 2000;
			//unitwalk 'gid, 21,12;
		} else {
			//unitwalk 'gid, 22,22;
			sleep2 3000;
			//unitwalk 'gid, 25,8;
		}
		sleep2 2000;
		intro = 11;
		pcblockmove getcharid(3),0;
		//achieve(103);
		disablenpc instance_npcname(strnpcinfo(0));
		end;	
	}
}

// Shops
rent_mb,230,215,3	shop	McDonald	765,30033:15000,30034:15000,30035:15000,30036:15000,30037:15000,30038:15000,30039:15000,30040:15000,30041:15000,30042:15000;