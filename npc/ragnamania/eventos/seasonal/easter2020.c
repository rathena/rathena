prontera,155,196,5	script	Bunny	674,{
	mes "[Bunny] ";
    mes "Hello, how are you today? ";
    mes "If you are here it is because you heard about the ^DD00AARagnamania Easter Event 2020.^000000 ";
    next;
	mes "[Bunny] ";
    mes "To celebrate that date, I can give you a ^DD00AAstronger version of the Bunny Band for 14 days.^000000 ";
	mes "Oh, I can also give you a permanent souvenir, a ^DD00AABunny Top Hat.^000000 ";
    next;
	mes "[Bunny] ";
    mes "For the ^DD00AA" + getitemname(12948) + "^000000 you have to bring me a common ^DD00AA" + getitemname(2214) + ".^000000";
	mes "For the ^5F18C5" + getitemname(5378) + "^000000 bring me a common ^5F18C5" + getitemname(2214) + "^000000 and a ^5F18C5" + getitemname(2220) + ".";
    mes " ";
	mes "^000000Which one will you want this time? ";

	next;

	switch(select("Bunny Band for 14 days:Bunny Top Hat:Leave")) {
        case 3:
            mes .n$;
            mes "No problem! See you 'round!";
            close;
        case 1:
            mes .n$;
            mes "Good choice! let me prepare one for you.";
            next;
            mes .n$;
            if(countitem(2214) < 1)
                goto Lne;
            mes "Here it is!";
            delitem 2214,1;
            getitem 12948,1;
            close;
        case 2:
            mes .n$;
            mes "Good choice! let me prepare one for you.";
            next;
            mes .n$;
          	if(countitem(2214) < 1 || countitem(2220) < 1)
          		goto Lne;
            mes "Here it is!";
            delitem 2214,1;
            delitem 2220,1;
            getitem 5378,1;
            close;
    }    
	Lne:
mes "Are you trying to fool me!?";
close;
}