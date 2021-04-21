prontera,160,180,3	script	Faction Sample	100,{
mes "[Faction Sample]", "Hello!", "I'm the sample NPC for faction system script commands";
next;
mes "[Faction Sample]", "Let's start...", "Input faction ID 1~4 and i'll make you a faction member";
next;
input(@faction_id,1,4);
mes "[Faction Sample]","You choose '"+factioninfo(@faction_id,0)+"' faction:","What are you want?";
next;
L_Menu:
mes "[Faction Sample]";
	switch(select("Give me more Info about this faction:Make me a faction member of '"+factioninfo(@faction_id,0)+"':FvF On:FvF Off:Spawn Faction Monster"))
	{
		case 1: callsub(S_FACTION_INFO); break;
		case 2: callsub(S_FACTION_SET); break;
		case 3: callsub(S_FACTION_FVF_SET,1); break;
		case 4: callsub(S_FACTION_FVF_SET,0); break;
		case 5: callsub(S_FACTION_SPAWN); break;
	}
close;

S_FACTION_SPAWN:
	factionmonster(@faction_id,"this",159,180,factioninfo(@faction_id,1),1002,1);
	close;

S_FACTION_FVF_SET:
	.@map$ = strnpcinfo(4);
	.@s = getmapflag(.@map$,mf_fvf);
	switch(getarg(0))
	{
		case 1: if( !.@s ) fvfon(.@map$); break;
		default: if( .@s ) fvfoff(.@map$); break;
	}
	close;

S_FACTION_SET:
	setfaction(@faction_id);
	mes "Now you're in Faction '"+factioninfo(@faction_id,0)+"'";
	next;
	close;

S_FACTION_INFO:
	mes "Faction id:" +@faction_id;
	mes "Name:" +factioninfo(@faction_id,0);
	mes "Unit name:" +factioninfo(@faction_id,1);
	mes "Home map:" +factioninfo(@faction_id,2);
	mes "Home map X:" +factioninfo(@faction_id,3);
	mes "Home map Y:" +factioninfo(@faction_id,4);
	mes "Race:" +factioninfo(@faction_id,5);
	mes "Element:" +factioninfo(@faction_id,6);
	mes "Element lvl:" +factioninfo(@faction_id,7);
	mes "Size:" +factioninfo(@faction_id,8);
	mes "Clothes color:" +factioninfo(@faction_id,9);
	mes "Voting state:" +factioninfo(@faction_id,10);

	mes "Alliance #1:" +factioninfo(@faction_id,11,0);
	mes "Alliance #2:" +factioninfo(@faction_id,11,1);
	mes "Alliance #3:" +factioninfo(@faction_id,11,2);

	mes "Aura effect #1:" +factioninfo(@faction_id,12,0);
	mes "Aura effect #2:" +factioninfo(@faction_id,12,1);
	mes "Aura effect #3:" +factioninfo(@faction_id,12,2);
	next;
	goto L_Menu;
}

/**
 * Example shop
 * Faction ID = 0, Discount -50% for Faction ID 1, Overcharge 50% for Faction ID 4
 **/
prontera,152,180,3,0,[1:-50,4:50]	shop	Example_Shop_0	100,7227:100;

/**
 * 'floating' shop
 **/
//-,[1:15,4:-5,2:30]	shop	Example_Floating	100,7227:100;

/**
 * Example script
 * Faction ID = 2
 **/
// prontera,154,180,3,2	script	Example_Script_2	100,{
// 	mes "Hello!";
// 	close;
// }

/**
 * Example duplicate
 * Duplicate of 'Example_Script_2' but with Faction ID = 3
 **/
//prontera,156,180,5,3	duplicate(Example_Script_2)	Example_Script_3	100


prontera,154,180,3	script	Clan Join	100,{
	mes "Hello!";
	switch(select("Join a Clan:Leave the Clan:")){
		case 1:
			if (clan_join(SWORDCLAN)) {
				sc_start2 SC_SWORDCLAN, INFINITE_TICK, 0, SWORDCLAN;
				mes "[Raffam Oranpere]";
				mes "Welcome to the ^3131FFSword Clan^000000!";
				mes "You can review Clan info with ^3131FFCtrl+G^000000.";
				mes "At your service!";
			} else
				mes "An error has occurred.";
			close;
		case 2:
			mes "done.";
			clan_leave();
			close;
	}
}