sala_premmy,184,126,4	script	Mighty Alchemist	665,{
	mes .n$;
	mes "Hello, " + strcharinfo(0);
	mes "Please be quick... I am very busy.";
	mes "I believe you are here to fix";
	mes "the mess you made with your ^00AAAAStats^000000";
	mes "and ^448800Skills^000000... I can see it.";
	mes "Am I right? of course I am right.";
	next;
	mes .n$;
	if(!vip_status(1) || (vip_status(1) && PM_RESET_CD < gettimetick(2))) {
		mes "Well, it won't come for free...";
		mes "We'll need:";
		mes "^0000FF1x " + getitemname($@RESET_STONE_ID) +"^000000";
		if(!vip_status(1)) mes "Plus commisison of ^0000FF" +callfunc("F_InsertComma",(BaseLevel+JobLevel)*1000)+"z^000000";
	} else {
		mes "Remember you can only reset either your stats or skills once every 7 days. You can choose to reset both at the same time too if you prefer.";
		mes " ";
		mes "^ff0000This applies to each character in your account, meaning you can reset any of your characters once every 7 days.^000000";
	}
	next;
	mes .n$;
	mes "If you are still ok with that, which option will you choose? I would go for a full reset if I was you... What a mess... Jeez!";
	set .@i, select(" ~ Reset Stats: ~ Reset Skills: ~ Reset both Stats and Skills: ~ Cancel");
	clear;
	if (.@i == 4) close;
	mes .n$;

	if(vip_status(1) && PM_RESET_CD < gettimetick(2)) {
		mes "I can see you are still";
		mes "in the cooldown until";
		mes "your next free reset...";
		next;
		mes .n$;
		mes "But no worries, I may be still able to help you";
		next;
	} else if(vip_status(1)) {
		mes "Ok, lets get ready then...";
		PM_RESET_CD = gettimetick(2) + .cd;
		next;
		goto L_Reset;
	}

	if(countitem($@RESET_STONE_ID)<1 || Zeny<(BaseLevel+JobLevel)*1000) {
		mes "If at least you had a ^ff0000" + getitemname($@RESET_STONE_ID) +"^000000 and the sum of " + callfunc("F_InsertComma",(BaseLevel+JobLevel)*1000)+"z^000000...";
		next;
		mes .n$;
		mes "But that is fine, next time then.";
		close;
	} else {
		mes "I am sorry but, payment first... Can I have the "+getitemname($@RESET_STONE_ID) +"^000000 and the sum of " + callfunc("F_InsertComma",(BaseLevel+JobLevel)*1000)+"z^000000 now, please?";
		next;
		if(select("^FF0000No, I've changed my mind:^00AA00Yes, please take it^000000") == 1 ){
			mes .n$;
			mes "What a waste of precious time...";
			mes "....";
			close;
		} else {
			if(countitem($@RESET_STONE_ID) >= 1 && Zeny >= (BaseLevel+JobLevel)*1000) {
				delitem $@RESET_STONE_ID,1;
				Zeny -= (BaseLevel+JobLevel)*1000;
				goto L_Reset;
			} else {
				close2;
				npctalk "I see what you tried to do there... Guards!!";
				logmes "Tried to cheat the Reset Stats Skills NPC";
				atcommand "@jailfor 5n" + strcharinfo(0);
				end;
			}
		}
	}

L_Reset:
	sc_end SC_ALL;
	switch(.@i){
		case 1: resetstatus; break;
		case 2: resetskill; break;
		case 3: resetstatus; resetskill; break;
		default: end;
	}
	specialeffect2 585;
	sleep2 1000;
	mes .n$;
	mes "Yessss!!";
	mes "Phew! I never told you but that is very dangerous... Glad you survived, though.";
	next;
	mes .n$;
	mes "Be more careful this time... don't mess up again.";
	mes "We never know how this is gonna end next time.";
	close;

OnInit:
	.n$ = "^AA0000[ Mighty Alchemist ]^000000";
	.cd = 7 * 24 * 60 * 60; // 7 days in Epoch time
	end;
}

geffen_in,80,56,2	duplicate(Mighty Alchemist)	Mighty Alchemist#2	665