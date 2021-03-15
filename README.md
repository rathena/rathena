New Commands

@setfaction - Set faction
@reloadfactiondb - Reloading faction data bases
@fvfon - FvF on
@fvfoff - FvF off
@home - Return character to Faction home location
@vote - Vote for faction leader
@factionchat - Like @mainchat, but with faction members
@alliancechat - Like @mainchat, but with faction + faction alliance members
@factionmonster - Spawn faction monster
@factionleader - Set faction leader
@factionannounce - Announce to all faction members (Faction Leader feature)



New Script Commands

*setfaction(<Faction ID>{,<Char ID>});

*setfactionleader(<Faction ID>,<Char ID>);
*relicadd(<Faction ID>,<Item ID>,<Slot>);
*relicgetinfo(<Faction ID>,<Slot>);
*relicactivate(<Faction ID>,<Slot>,<Val>);
*relicdel(<Faction ID>,<Slot>);
*votinginfo(<Faction ID>);
*vote(<Char ID>{,<Amount of Votes>});
*addvotes(<Char ID>{,<Amount of Votes>});
*votingstart({<Faction ID>});
*votingstop({<Faction ID>});
*votingend({<Faction ID>});
*factionmonster(<faction ID>,"<map name>",<x>,<y>,"<name to show>",<mob ID>,<amount>{,"<event label>"});
*areafactionmonster (<faction ID>,"<map name>",<x1>,<y1>,<x2>,<y2>,"<name to show>",<mob ID>,<amount>{,"<event label>"});
*fvfon "<map name>"{,Faction ID};
*fvfoff "<map name>";


New mapflag: fvf ( mf_fvf )
New map cell: nofvf ( cell_nofvf, cell_chknofvf )


Main Features:

Faction versus Faction wars allowed only on FvF maps ( Min.lvl -> fvf_min_lvl: 55 )
Custom aura effects ( max is 3 by default, but you can increase it )
Aura can displaying for unit types in faction.conf: NPCs, Monsters, Pets, etc...( not only for Player )
Monster and other can change own base status to status from faction_db.txt ( race, element... )
If faction have Undead race or status faction members in this function can heal themselves ( if it allowed in faction_db.txt )
NPC shops can sell items to different factions at different prices.
Now saving faction leader and relics of each faction.
Multi - alliance support.
Voting system ( voting for faction leader ).
Relics support.
Logging Faction chat.
