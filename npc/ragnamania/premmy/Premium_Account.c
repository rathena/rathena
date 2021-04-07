prontera,159,286,5	script	Lilian#PremmyCentral	4_F_KAFRA1,{
	if(checkquest(64505) == 1) {
		if(#FREE_PREMMY > 0){
			erasequest 64505;
			mes "^FF7F00[ Lilian ]^000000";
			mes "Oh, I am sorry... I can see you've already gotten your free Premmy, maybe with a different char?";
			next;
			mes "^FF7F00[ Lilian ]^000000";
			mes "I am afraid there is nothing I can do for you in regards to that...";
			close;
		}
		mes "^FF7F00[ Lilian ]^000000";
		mes "Omg, "+strcharinfo(0)+" you made it!";
		mes "How are you doing? It is so good to see you!";
		next;
		mes "^FF7F00[ Lilian ]^000000";
		mes "You are here for your 5 days free Premmy, right?";
		mes "Let me sort that out for you...";
		next;
		completequest 64505;
		vip_time $@Periodo;
		#FREE_PREMMY = gettimetick(2);
		dispbottom "^00AAAA[Premmy Central]^000000 : 5 days of Premmy Activated.";
		logmes "[Premmy Central] : Free Premmy Activated.";
		mes "^FF7F00[ Lilian ]^000000";	
		mes "All done! Thank you!";
		next;
	}
	callfunc "F_Premmy";
	end;

 OnInit:
 	set $@Periodo, 60 * 24 * 5; //	5 dias em minutos
	set $@EndTime, 60 * 60 * 24 * 5; // 5 dias em segundos 
// 	.npc$ = "^FF7F00[ Premmy Central ]^000000";						// Name of NPC;
// 	.bcor$ = "^008aff[*]^000000 ";									// Option Button Symbol;
// 	.rcor$ = "^ff0000[*]^000000 ";									// Cancel button symbol;
// 	setarray .vip_day,30;										// Number of days that can be purchased;
// 	setarray .vip_cashpoint,20000;						// Price in #CASHPOINTS;
// 	.map$ = "sala_premmy";												// Premmy Room Map;
// 	setarray .cord,35,36;											// X and Y Coordinate;
// 	end;
}


function	script	F_Premmy	{
	.npc$ = "^FF7F00[ Premmy Central ]^000000";						// Name of NPC;
	.bcor$ = "^008aff[*]^000000 ";									// Option Button Symbol;
	.rcor$ = "^ff0000[*]^000000 ";									// Cancel button symbol;
	setarray .vip_day,30,10;										// Number of days that can be purchased;
	setarray .vip_cashpoint,20000,8000;								// Price in #CASHPOINTS;
	setarray .vip_hunting,800,300;									// Price in Hunting Coins
	.map$ = "sala_premmy";											// Premmy Room Map;
	setarray .cord,35,36;	

	mes .npc$;
	mes (gettime(3)>= 6&&gettime(3)<= 12?"Good Morning":(gettime(3)>=13&&gettime(3)<=18?"Good Afternoon":"Good Evening"))+", ^008aff"+strcharinfo(0)+"^000000 !";
	mes "Welcome to Premmy Central.";
	if (vip_status(1)) {
		mes "^008affYour Premmy access will expire on " + callfunc("Time2Str",vip_status(2)) + ".^000000";
		mes " ";		
	}
	mes "How can I help?";
	next;
	switch (select(((vip_status(1))?""+.bcor$+"I want to extend my time ^008affPremmy^000000.":""+.bcor$+"I wish to become a ^008affPremmy^000000 player."),""+.rcor$+"I am good, thank you.")){
		case 1:
			mes .npc$;
			mes "How many days of ^008affPremmy^000000 access do you want to acquire?";
			next;
			.@i = select(
				.bcor$+""+.vip_day[0] + " Days."
				,.bcor$+""+.vip_day[1] + " Days."
				) - 1;
				
			mes .npc$;
			mes "You chose the plan of:";
			mes "^4527A0Premmy:^000000 "+.vip_day[.@i]+" Premmy days.";
			mes "^4527A0Cost:^000000 "+F_InsertComma(.vip_cashpoint[.@i])+" Manias, or";
			mes "^4527A0Cost:^000000 "+.vip_hunting[.@i]+"x " + getitemname(675)+"s.";
			sleep2 1000;
			mes "What do you want to do?";
			next;
			switch(select(.bcor$+"Purchase with Manias:"+.bcor$+"Purchase with Hunting Coins:"+.rcor$+"Cancel")){
				case 1:
					if ( #CASHPOINTS < .vip_cashpoint[.@i] ) {
					mes .npc$;
					mes "You do not have the quantity of ^008aff"+F_InsertComma(.vip_cashpoint[.@i])+" #CASHPOINTS^000000.";
					close;
				}
				else {
					#CASHPOINTS -= .vip_cashpoint[.@i];
					vip_time (.vip_day[.@i] * 1440);
					mes .npc$;
					mes "Congratulations you just got "+.vip_day[.@i]+" days as Premmy.";
					logmes "[Premmy Central] Comprou/Extendeu Premmy com Manias";
					if(.vip_day[.@i] >= 30) {
						#FREE_BOOSTER = gettimetick(2);
						getitem 30001,1;
					}
					close;
				}
				end;
				case 2:
					if ( countitem(675) < .vip_hunting[.@i] ) {
						mes .npc$;
						mes "You do not have enough ^008aff"+getitemname(675)+"s^000000.";
						close;
					}
					else {
						delitem 675, .vip_hunting[.@i];
						vip_time (.vip_day[.@i] * 1440);
						mes .npc$;
						mes "Congratulations you just got "+.vip_day[.@i]+" days as Premmy.";
						logmes "[Premmy Central] Comprou/Extendeu Premmy com Hunting Coins";
						if(.vip_day[.@i] >= 30) {
							#FREE_BOOSTER = gettimetick(2);
							getitem 30001,1;
						}
						close;
					}
					end;

				case 3:
					mes .npc$;
					mes "All right, come back whenever you want.";
					close;
			}
		case 2:
			close;
	}
}