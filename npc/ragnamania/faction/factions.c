prontera,160,180,3	script	Faction Sample	100,{
    mes "[Faction Sample]", "Hello!", "I'm the sample NPC for faction system script commands";
    next;
    mes "[Faction Sample]", "Let's start...", "Input faction ID 1~4 and i'll make you a faction member";
    next;
    input(@faction_id,1,2);
    mes "[Faction Sample]"," Good choice.";
    next;
L_Menu:
    mes "[Faction Sample]";
	switch(select("Make me a faction member of '"+@faction_id+"':FvF On:FvF Off:Spawn Faction Monster"))
	{
		case 1: callsub(S_FACTION_SET); break;
		case 2: callsub(S_FACTION_FVF_SET,1); break;
		case 3: callsub(S_FACTION_FVF_SET,0); break;
		case 4: callsub(S_FACTION_SPAWN); break;
	}
close;

S_FACTION_SPAWN:
	factionmonster(@faction_id,"this",159,180,strcharinfo(0),1002,1);
	close;

S_FACTION_FVF_SET:
	.@map$ = strnpcinfo(4);
	.@s = getmapflag(.@map$,mf_hostile);
	switch(getarg(0))
	{
		case 1: if( !.@s ) fvfon(.@map$); break;
		default: if( .@s ) fvfoff(.@map$); break;
	}
	close;

S_FACTION_SET:
	setfaction(@faction_id);
	mes "Now you're in Faction '"+@faction_id+"'";
	next;
	close;
}

/**
 * Example shop
 * Faction ID = 0, Discount -50% for Faction ID 1, Overcharge 50% for Faction ID 4
 **/
prontera,152,180,3,0,[1:-50,4:50]	shop	Example_Shop_0	100,7227:100;
/**
 * 'floating' shop
 **/
-,[1:15,4:-5,2:30]	shop	Example_Floating	100,7227:100;

/**
 * Example script
 * Faction ID = 2
 **/
prontera,154,180,3,2	script	Example_Script_2	100,{
	mes "Hello!";
	close;
}

/**
 * Example duplicate
 * Duplicate of 'Example_Script_2' but with Faction ID = 3
 **/
prontera,156,180,5,3	duplicate(Example_Script_2)	Example_Script_3	100