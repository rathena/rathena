// ********************************************************************
// Housekeeper
// ********************************************************************

rentb1,5,5,6	script	Alfonso	1854,{
	if (atoi(#HOUSING$[0]) != 'HouseNumber) { //'
		mes "[ Alfonso ]";
		mes "Sorry, I can't speak right now...";
		mes "... may the boss see us and I don't want problems. I get paid by hour, you know.";
		close;
	} else {
		L_Start:
		mes "[ Alfonso ]";
		mes "Hello, " +strcharinfo(0)+ "!";
		mes "Gorgeous day, isn't it?";
		mes "Do you need something?";
		next;
		switch(select("Say Goodbye:Got anything for me?:Give some tips")) {
		case 2:
			if(KEEPER_LOOT$ == gettimestr("%Y%m%d",21)) {
				mes "[ Alfonso ]";
				mes "But, "+strcharinfo(0)+ ",";
				mes "I am afraid I've given";
				mes "everything for you earlier today.";
				mes "Haven't I?!";
				next;
				mes "[ Alfonso ]";
				mes "Let me check again...";
				next;
			}
			
			if(KEEPER_LOOT$ != gettimestr("%Y%m%d",21)) {
				mes "[ Alfonso ]";
				mes "Actually I got something here...";
				mes "Do you want to collect them now?";
				mes " ";
				mes "^FF0000Make sure you are not overweighted";
				mes "and have enough room in your inventory^000000.";
				next;
				if(select("Yes, please:No, I will do it later") == 2) {
					mes "[ Alfonso ]";
					mes "Ok! Anytime.";
					next;
					goto L_Start;
				} else {
					mes "[ Alfonso ]";
					mes "Ok! Here, "+strcharinfo(0);
					mes "Hope they can be helpful!";
					next;
					setarray .@commonitems[0],962,938,517,7013,962,938,525,716,1052,910,938,713,1016,919,949,528,928,955,1010,915,938,716,1002;
					setarray .@rareitems[0],1011,971,525,719,970,630,645,701,1011,1152,729,1116,2102,737,2304,2303;
				
					for( .@i = 1; .@i <= rand(5,10); .@i++ ) {
						if(.@i % 5 == 0)
							if(rand(3) == 0) {
								getitem .@rareitems[rand(getarraysize(.@rareitems))],1;
							}
						getitem .@commonitems[rand(getarraysize(.@commonitems))],1;
						sleep2 500;
					}
				}
				mes "[ Alfonso ]";
				mes "Got them all? ";
				mes "Neat stuff, huh?";
				KEEPER_LOOT$ = gettimestr("%Y%m%d",21);
				next;
				goto L_Start;
			} else {
				mes "[ Alfonso ]";
				mes "Really sorry, "+strcharinfo(0);
				mes "Maybe later?";
				mes "Or tomorrow, for sure!";
				mes "Weeds tend to grow over night.";
				next;
				goto L_Start;
			}
		case 3:
			mes "[ "+strcharinfo(0)+" ]";
			mes "Here, please take this...";
			mes "I know it is not much";
			mes "But it is more than 20%";
			mes "of all my money. Lol";
			input .@tips;
			next;
			if(Zeny < .@tips) {
				mes "[ Alfonso ]";
				mes "What a joke...";
				mes "Seriously?";
				mes "Ok, thank you anyway.";
				next;
				emotion ET_SHY;
			} else {
				mes "[ Alfonso ]";
				mes "Oh, Are you sure?";
				mes "It is very kind of you!";
				mes "Thank you very much, "+strcharinfo(0);
				set Zeny, Zeny - .@tips;
				KEEPER_TIPS = KEEPER_TIPS + .@tips;
				next;
				emotion ET_BEST;
			}
			goto L_Start;
			break;
		case 1:
			mes "[ "+strcharinfo(0)+" ]";
			mes "Nah, Thanks...";
			mes "Talk to you later, ok?";
			close;
		}
	}

OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	donpcevent instance_npcname("#rentb1")+"::OnDisable";
	donpcevent instance_npcname("seaweed")+"::OnDisable";
	donpcevent instance_npcname("seaweed#2")+"::OnDisable";
	donpcevent instance_npcname("seaweed#3")+"::OnDisable";
	donpcevent instance_npcname("seaweed#4")+"::OnDisable";
	donpcevent instance_npcname("seaweed#5")+"::OnDisable";
	donpcevent instance_npcname("seaweed#6")+"::OnDisable";
	end;

// OnMinute00:
// OnMinute05:
// OnMinute10:
// OnMinute15:
// OnMinute30:
// OnMinute45:
// OnMinute50:
// OnMinute55:
// 	if('fishesOn == 0) {//'
// 		enablenpc instance_npcname("#fishing");
// 		donpcevent instance_npcname("#fishing",instance_id(IM_CHAR))+"::OnFishes";
// 		set 'fishesOn,1; //'
// 	}
// 	end;

OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;
}

rentb1,14,9,0	script	#fishing	111,{
	set getvariableofinstance('count,instance_id(IM_CHAR)), getvariableofinstance('count,instance_id(IM_CHAR))+1; //'
	progressbar "0xFF0000",rand(3,8);
	if(getvariableofinstance('count,instance_id(IM_CHAR)) == 3) { //'
		set getvariableofinstance('fishesOn,instance_id(IM_CHAR)),0;//'
		disablenpc instance_npcname(strnpcinfo(0));
	}
	end;

OnFishes:
	misceffect 1097;
	sleep 600;
	misceffect 1097;
	end;
}

-	shop	FishingShop	-1,2764:3500