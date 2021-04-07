sala_premmy,158,127,6	script	Stella	552,{
	if(!vip_status(1) && getgmlevel()<10) end;
	mes .n$;
	mes "Oh, "+strcharinfo(0)+",";
	if(Class != 4047) {
		mes "I understand you may want";
		mes "to help me but I afraid";
		mes "I can't give you anything";
		mes "in exchange.";
		next;
		mes .n$;
		mes "But if perhaps you get to";
		mes "know any ^AAAA00Star Gladiator^000000";
		mes "would you be so kind and tell";
		mes "them I can help them reset";
		mes "their ^0000AAFeeling^000000 or ^0000AAHatred^000000?";
		mes "Thank you!";
		close;
	}
	
	mes "Please help me over here!";
	mes "I am in trouble and now";
	mes "half of my tribe in Ayothaya";
	mes "want to peel me like a banana.";
	next;
	mes .n$;
	mes "I learned this technique to reset ";
	mes "^0000AAFeeling^000000 or ^0000AAHatred^000000 with mai auntie";
	mes "Perhaps you'd be interested?";
	next;
	mes .n$;
	mes "I can only do that once a day, though...";
	mes "For you, it would be only ^0000FF"+ callfunc("F_InsertComma",(BaseLevel+JobLevel)*2000)+"z^000000";
	mes "Are you interested?";
	next;
	menu "Feeling reset",feeling,"Hatred reset",hatred,"No, Thank you",cancel;

feeling:
	if(SG_RESET_FEELING > gettimetick(2))
		goto inCoolDown;
	if(Zeny < (BaseLevel+JobLevel)*2000)
		goto NoCoins;

	Zeny -= (BaseLevel+JobLevel)*2000;
	atcommand "@feelreset";
	specialeffect2 582;
	set SG_RESET_FEELING, gettimetick(2) + 68400;
	next;
	mes .n$;
	mes "Phew, it is done!";
	mes "Thank you very much!!";
	close;

hatred:
	if(SG_RESET_HATRED > gettimetick(2))
		goto inCoolDown;
	if(Zeny < (BaseLevel+JobLevel)*2000)
		goto NoCoins;

	set PC_HATE_MOB_MOON, 0;
	set PC_HATE_MOB_STAR, 0;
	set PC_HATE_MOB_SUN, 0;
	specialeffect2 582;
	set SG_RESET_HATRED, gettimetick(2) + 68400;
	next;
	mes .n$;
	mes "Done.";
	mes "You need to logout.";
	mes "Do you want to logout now?";
	menu "Yes, I want to logout",relog,"I will do this later",later;

relog:
	atcommand "@kick "+strcharinfo(0);
	end;

later:
	next;
	mes .n$;
	mes "Ok, your reset will be complete only after you logout.";
	close;

cancel:
	next;
	mes .n$;
	mes "No, problem... I understand.";
	close;

NoCoins:
	next;
	mes .n$;
	mes "Sorry, it looks like you don't have enough Zeny.";
	close;

inCoolDown:
	next;
	mes .n$;
	mes "Sorry, you can only reset once every 24 hours";
	close;

OnInit:
	.n$ = "^0000FF[ Stella ]^000000";
	end;
}