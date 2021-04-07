prontera,156,182,3	script	Faction Monsters	55,{
    mes "You can summon a guardian";
    mes " to protect the base";
    mes "for 100 zeny";
    next;
    mes "Do you want";
    mes "to summon one?";
    menu "Yes",-,"No",no;
    if (zeny <100) {mes "Not enough zeny"; close;}
        set zeny, zeny - 100;
    if(#faction==1)
        factionguardian "Faction1",1504;
    else
        factionguardian "Faction2",1504;

    no:
    close;
}