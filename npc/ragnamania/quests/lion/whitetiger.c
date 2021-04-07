gld_dun01,224,201,5	script	Uddga	505,{
	mes "[Uddga]";
    mes "Everyone knows that I am much stronger than my brother.";
    mes "I'm just hiding here for strategic reasons.";
    next;
	mes "[Uddga]";
    mes "If you help me, I can give you a reward.";
	mes "To prove my strength, I will need^DD00AA 20 x Tiger Skin^000000 and^DD00AA 10x Tiger's Footskin.";
    next;
	mes "[Uddga]";
    mes "Please, help me and I will give you a:^DD00AA " + getitemname(5497) + ".^000000";

	next;

	switch(select("Ok, I already have the necessary:No, Eddga is stronger:")) {
        case 2:
            mes "[Uddga] ";
            mes "Ok...if you change your mind I will be here.";
            close;
        case 1:
            mes "[Uddga] ";
            mes "Thank you!";
            next;         
            if(countitem(1029) < 20 || countitem(1030) < 10)
                goto Lne;
            mes "[Uddga] ";
            mes "Here it is!";
            delitem 1029,20;
            delitem 1030,10;
            getitem 5497,1;
            close;
    }    
	Lne:
mes "[Uddga] ";
mes "Wait... why are you trying to fool me?";
close;
}