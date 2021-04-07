moc_fild11,185,210,5	script	Mysterious Wanderer	494,{
	mes "[????] ";
    mes "What do you want? ";
    mes "My only business is selling a ^DD00AAspecial hat.^000000 ";
    next;
	mes "[????] ";
    mes "I think you cannot afford this. It costs ^DD00AA1.000.000.000.^000000 ";
	mes "Hmm, expensive, right? But it is a ^DD00AAWanderer's Sakkat .^000000 ";
    next;
	mes "[????] ";
    mes "Today I will make a special offer.";
    mes "Bring me these items and I will see what I can do for you:^DD00AA " + getitemname(2280) + "^000000 and a ^DD00AA" + getitemname(983) + ".^000000";
    mes " ";
	mes "^000000Do you wanna trade? ";

	next;

	switch(select("Yes:No")) {
        case 2:
            mes "[????] ";
            mes "Get out here.";
            close;
        case 1:
            mes "[????] ";
            mes "Ok, let's see what you brought...";
            next;         
            if(countitem(2280) < 1 || countitem(983) < 1)
                goto Lne;
            mes "[????] ";
            mes "Here it is!";
            delitem 2280,1;
            delitem 983,1;
            getitem 5579,1;
            close;
    }    
	Lne:
mes "[????] ";
mes "What the hell?!";
close;
}