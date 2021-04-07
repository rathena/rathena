gonryun,151,130,5	script	Elder	572,{
	mes "[Elder] ";
    mes "Hello!";
    mes "I am here to make a ^DD00AArare item...^000000 for a fair price. ";
    next;
	mes "[Elder] ";
    mes "I am talking about an ancient, gnarled branch with a contract written in blood upon the bark.";
	mes "It says that can summon strong creatures. People call it ^DD00AABloody Branch.^000000  ";
    next;
	mes "[Elder] ";
    mes "Bring me these items and I will prepare everything:";
    next;
    mes "[Elder] ";
    mes "^DD00AA ~ 500 " + getitemname(604);
    mes "^DD00AA ~ 100 " + getitemname(7511);
    mes "^DD00AA ~ 100 " + getitemname(7563);
    mes "^DD00AA ~ 100 " + getitemname(702);
    mes "^DD00AA ~ 001 " + getitemname(7111);
    next;
	mes "^000000Do you have these items? ";
    mes "Oh, I almost forgot. I will also need ^DD00AA10.000.000 zeny.^000000 ";

	next;

	switch(select("Ok, I will pay.:No")) {
        case 2:
            mes "[Elder] ";
            mes "Ok, cya!";
            close;
        case 1:
            mes "[Elder] ";
            mes "Ok, I will start the ritual";
            next;         
            if(countitem(604) < 500 || countitem(7511) < 100 || countitem(7563) < 100 || countitem(702) < 100 || countitem(7111) < 1 || Zeny < 10000000)
                goto Lne;
            mes "[Elder] ";
            mes "It's done. Be careful.";
            delitem 604,500;
            delitem 7511,100;
            delitem 7563,100;
            delitem 702,100;
            delitem 7111,1;
            getitem 12103,1;
            set Zeny, Zeny - 10000000;
            close;
    }    
	Lne:
mes "[Elder] ";
mes "Well, I think you are mistaken.";
close;

//NPC só aparece entre os dias 01 e 10 de cada mês. Vai embora no dia 11.

OnInit:
    if(gettime(DT_DAYOFMONTH)>10)
		disablenpc strnpcinfo(0);
end;

OnClock0000:
if(gettime(DT_DAYOFMONTH) <= 10) {
   enablenpc strnpcinfo(0);
} else {
   disablenpc strnpcinfo(0);
}
end;

}