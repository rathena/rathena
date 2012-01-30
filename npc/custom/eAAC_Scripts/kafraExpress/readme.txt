%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%             Kafra Express Script Package Documentation                       %
%                                                                 - by Skotlex %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
This file's purpose is to give an indepth explanation about setting up, and
configuring the Kafra Express Script Package (KESP). It is intended to be
viewed by a text editor using fixed-width font and 80-character long lines.

Document Version v2.0 (27/September/2006)
////////////////////////////////////////////////////////////////////////////////
Table of Contents
////////////////////////////////////////////////////////////////////////////////
[00] Including the NPC files and menu editing
[01] Intro to module configuring and general options (config.txt)
[02] Module: Main Core (ke_main.txt)
[03] Module: Bank (ke_bank.txt)
[04] Module: Broadcast (ke_broadcast.txt)
[05] Module: Stylist (ke_dye.txt)
[06] Module: Job Changer (ke_jobchange.txt)
[07] Module: Job Swapper (ke_jobswap.txt)
[08] Module: Smithery (ke_refine.txt)
[09] Module: Renting (ke_refine.txt)
[10] Module: Kafra Shop (ke_shop.txt)
[11] Module: Stat/Skill Market (ke_statmarket.txt)
[12] Module: Stat/Skill Resets/Raising (ke_stats.txt)
[13] Module: Uncarding (ke_uncard.txt)
[14] Module: Town Warping (ke_warp_town.txt)
[15] Module: Dungeon Warping (ke_warp_dungeon.txt)
[16] Module: PvP Warping (ke_warp_pvp.txt)
[17] Module: WoE Warping (ke_warp_woe.txt)
[18] The kafras.txt file: About Kafra Definitions

////////////////////////////////////////////////////////////////////////////////
[00] Including the NPC files and menu editing
////////////////////////////////////////////////////////////////////////////////

Because of the script's complexity, placing everything in a single file
is out of the question, therefore the KESP comes bundled in different
files. Because of the dynamic nature of the script, there are two things
you need to do in order to enable/disable a particular service:

1. Include the txt file with the corresponding module.
To include the txt files, you need to know two things: where are the script
files located, and which is the configuration file where you place the npc
includes. We'll assume on this guide that you unpacked the KESP in
npc/custom/kafraExpress. The configuration file for scripts is
conf/map_athena.conf for eA1.0rc5, and npcs/scripts_custom.txt for current eA
SVN versions. Add the files you want in the following manner:

	npc: npc/custom/kafraExpress/ke_main.txt
	//npc: npc/custom/kafraExpress/ke_rent.txt

The first line is including the file ke_main.,txt, while the following
excludes ke_rent from being used. Placing the "//" at the beginning of the
line is known as commenting, and it's a useful way of quickly toggling a
certain script on/off. The following is the list of all files that come
bundled in the package:

npc: npc/custom/kafraExpress/kafras.txt
npc: npc/custom/kafraExpress/config.txt
npc: npc/custom/kafraExpress/ke_main.txt

npc: npc/custom/kafraExpress/ke_bank.txt
npc: npc/custom/kafraExpress/ke_broadcast.txt
npc: npc/custom/kafraExpress/ke_dye.txt
npc: npc/custom/kafraExpress/ke_jobchange.txt
npc: npc/custom/kafraExpress/ke_jobswap.txt
npc: npc/custom/kafraExpress/ke_rent.txt
npc: npc/custom/kafraExpress/ke_shop.txt
npc: npc/custom/kafraExpress/ke_statmarket.txt
npc: npc/custom/kafraExpress/ke_stats.txt
npc: npc/custom/kafraExpress/ke_refine.txt
npc: npc/custom/kafraExpress/ke_uncard.txt
npc: npc/custom/kafraExpress/ke_warp_dungeon.txt
npc: npc/custom/kafraExpress/ke_warp_pvp.txt
npc: npc/custom/kafraExpress/ke_warp_town.txt
npc: npc/custom/kafraExpress/ke_warp_woe.txt

What files can be commented, which files you want to include? On the minimum,
you need ke_main.txt, kafras.txt and config.txt to have the bare-bones functionality.
Refer to the table of context to see the relation between files and the
services they offer.

2. Configure the Menus
The second part of the configuration is adding/removing the menu entries that
lead to the specific services. Failure to do this can either A. leave you with
menu entries that lead to "Function Not found!" errors on the map server, or
B. Services that were included, but you can't pick because they don't show up
in the menu. The main file where you should configure the menus is ke_main.txt.

From the main module, ke_main.txt, you must pick which services are available.
The file contains eight menu sections, which is made up of two pairs of
identitical menus. The only difference is that the first menu is displayed on
Kafras placed on towns, while the second menu is for Kafras placed on fields and
dungeons. The menu roughly looks like this:

	menu
		"- Leave",L_END,
		"- Heal Service ("<some code here>"z/10SP)",L_HEAL,
		"- Warp Service",L_WARP,
//		"- Use Storage ("<some code here>"z)",L_STORAGE,
//		"- Use Guild Storage ("<some code here>"z)",L_GUILD_STORAGE,
		"- Job Services",M_JOB,
		"- Other Services",M_OTHER,
		"- Save Respawn point",L_SAVE;

This is the main menu, as you can see the only service unavailable is to use
the storage. Like in the npc/config file, you can comment the lines of the
services you do not want.

WARNING: If you need to comment the last option of a menu, remember to replace
the comma for a semi-colon on the next-to-last option or script parsing errors
will occur.

The "Warp Service" leads to the second pair of menus you can configure: 

	menu
		"- Return",-,
		"- Dungeons",L_DUNGEON,
//		"- PvP Arena",L_PVP,
//		"- Guild Wars",L_GUILD_DUNGEON,
		"- Towns",L_TOWN;
	goto M_INIT;

As before, you can comment/uncomment the features you want or not. Likewise,
the third menu under "Job Services" contains features related to job changing,
stats/skills and renting. The fourth pair is the "Other Services" menu which
contains the rest of modules which are probably not going to be used
frequently.

For your convenience, the following is the list of all the menu options and the file(s) required for it to work:

//Main Menu (under labels M_INIT/MD_INIT)

   "- Heal Service"       -> ke_main.txt
   "- Warp Service"
   "- Use Storage"        -> ke_main.txt
   "- Use Guild Storage"  -> ke_main.txt
	"- Job Services"
	"- Other Services"
   "- Save Respawn point" -> ke_main.txt

//Warp Menu (under labels M_WARP/MD_WARP)

	"- Dungeons"   -> ke_warp_dungeon.txt
	"- PvP Arena"  -> ke_warp_pvp.txt
	"- Guild Wars" -> ke_warp_gvg.txt
	"- Towns"      -> ke_warp_town.txt

//Job Services Menu (under labels M_JOB/MD_JOB)
	
   "- Change Job"          -> ke_jobchange.txt
   "- Swap Job"            -> ke_jobswap.txt
   "- Stat/Skill Services" -> ke_stats.txt
   "- Stat/Skill Market"   -> ke_statmarket.txt
   "- Rental Service"      -> ke_rent.txt

//Other Services Menu (under labels M_OTHER/MD_OTHER)

   "- Bank Services"       -> ke_bank.txt
	"- Use Kafra Shop"      -> ke_shop.txt
   "- Broadcast a message" -> ke_broadcast.txt
   "- Refine Services"     -> ke_refine.txt
   "- Uncard Services"     -> ke_uncard.txt
   "- Stylist Service"     -> ke_dye.txt
   "- Use a Kafra Pass"    -> ke_main.txt

////////////////////////////////////////////////////////////////////////////////
[01] Intro to individual module configuration.
////////////////////////////////////////////////////////////////////////////////

For portability reasons, the configure options for every module is in the file
config.txt, which lets you upgrade to future versions without having to
readjust your settings every time.
The config file has the options separated per module to make it easier to
read. There may be bits of code in each section to avoid parsing configure
options for unneeded variables which should be left alone. In some rare
occassions variables from one module will be used in a different module (ie:
Broadcasting PvP messages uses the variables from the pvp module). all variables
follow the standard "ke<module initials>_variablename", so a variable called
"kewd_discount" refers to the discount variable in the deep warps module (wd).
For example, the renting module's configuration segment is:

	//-------------------------------------------------------------------------------
	//Config for the Renting Module
	//-------------------------------------------------------------------------------
OnLoadRent:
	set $@kert_cartOnly, 0;	//Set to 1 to enable only cart rental, 0 enables all add-ons.
	set $@kert_cartCost, 2000;	//Cost to rent a Cart.
	set $@kert_falconCost, 2000;	//Cost to rent a Falcon.
	set $@kert_pecoCost, 2000;	//Cost to rent a PecoPeco.
	end;

Variables are usually of two types: Exact value based or Percentage based.
Exact value variables are often price for different services, while the
percentage based are things like 30% discount when using Kafra Passes.
Percentage values are expressed per-hundredth (that is, 10 = 10%, 100 = 100%)
unless otherwise specified.

Each variable has a small description next to it, hence the need of this
document. In this document the variable type is identified next to it by: (1)
when it's boolean, (%) when it's a percentage, ($) when it's a price, (#) for
numbers and (") for strings.

////////////////////////////////////////////////////////////////////////////////
[02] Module: Main Core (ke_main.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
This module offers the core functionality of the Kafra Express. Contains the
main menu and handles the Kafra Pass Reserve Points system.

Variables
================================================================================

(1) ke_saveOnSpot
-----------------
When 1, a character's respawn point is saved exactly where they are standing.
otherwise, the respawn point is specified by the kafra definition (see Section
18).

($) keh_hpCost 
($) keh_spCost 
--------------
The cost of healing sp per every 10 sp. For example, if the cost is 1, it
costs 100z to heal 1000.

($) kes_cost
($) kegs_cost
-------------
The cost of using the storage, guild storage (respectively)

(#) kekp_reset
--------------
For Kafra Passes. When set, the Pass expires after starting a chat with the
Kafra the amount of times specified. Otherwise the variable remains active a
pretty long time (probably all session). For example, if set to 2, after
activating the pass and opening the storage, the Pass will still be active
the next time you speak to a kafra, it will expire on the "third" time you
speak to her.

($) kekp_reserveCost 
--------------------
The cost in zeny of reserve points. When using a Kafra Pass many services will
be cheaper, the amount of zeny saved is "used" to grant the player reserve
points. For example, if the reserve cost is 100, for every 100z the player
saves, he'll earn 1 reserve points. Reserve points can be used in scripts, and
by default is used in the Al De Baran Kafra Headquarters to gain items. Use a
value of 0 to disable Reserve Point gaining.

(#) kekp_minReserve
(#) kekp_maxReserve
---------------
What is the minimum/maximum reserve points the player can gain when using a
Kafra Pass per transaction? This only applies when the player has saved at
least 1z.

(") ked_users
-------------
Certain modules (in particular, the warping ones) have the ability to display
the number of players related to the function (ie: number of users in a
dungeon), in such cases this variable is used to display the 'unit' of said
count. For example, if ked_users is "kids" then the related modules might
display things like "- Glast Heim (6 kids)".

////////////////////////////////////////////////////////////////////////////////
[03] Module: Bank (ke_bank.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
This modules enables characters to "store" zeny on a virtual bank account
which is shared among characters of the same account. There can be deposits,
withdrawals, which can have transactions fees associated.

It is also possible to establish a monthly maintenance fee that has to be
payed, when the player does not has enough money in the account to pay this
fee, they stop gaining interests until they deposit enough to pay it up.
Finally, all fees charged by the bank can be stored in a server-wide variable
which may be used by other custom scripts.

Variables
================================================================================

(%) kebk_depositCost
(%) kebk_withdrawCost
---------------------
The Fee in % charged whenever a player does a deposit/withdrawal. For example,
when a player deposits 100z and the fee is 3%, only 97z are deposited, and 3z
are charged. Likewise, if a player withdraws 100z when the fee is of 5%,
they'll withdraw 100z and an additional 5z will be removed from their accounts
as fee cost.

($) kebk_minTransact
($) kebk_maxTransact
--------------------
The minimum/maximum values of a single transaction (deposit or withdrawal)

($) kebk_capacity
-----------------
Indicates what is the bank account capacity for players. That is, what is the
maximum zeny their account can hold. You can't deposit anymore once the max
has been reached, and daily interests are lost while maxed.

(%) kebk_dayInterest
--------------------
The daily interests that the account makes. The value is in 0.01% units, so a
value of 100 equals 1% daily interests.

($) kebk_monMaintenance
-----------------------
Monthly flat fee charged for maintenance.

(1) kebk_useGlobalBank
----------------------
When 1, every fee charged from the player goes into a server variable
($ke_globalbank), which can then be used by other scripts. is 0 by default
because none of the Kafra Express modules uses it.

////////////////////////////////////////////////////////////////////////////////
[04] Module: Broadcast (ke_broadcast.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Provides broadcasting services for players. Broadcasts can be local (current
map only) or global (server announce), there are also four types of broadcast:
Requests for a party, PvP Challenge invitations, General Broadcasts (player
input the string they want to say) and General Anonymous Broadcasts (player
name is not displayed when doing the broadcast).

Note that the variables from ke_warp_pvp.txt will be used for auto-configuring
the pvp broadcasts.

Variables:
================================================================================

(1) kebc_showOnline
-------------------
When 1, the total count of players will be displayed in the menu (map users
next to the local broadcast entry, server users next to the global broadcast
entry).

($) kebc_partyCost
($) kebc_pvpCost
($) kebc_cost
($) kebc_anonCost
------------------
Respective base costs for doing Party-Requests/Pvp Challenge/General/Anonymous
broadcasts.

(%) kebc_globalFactor
---------------------
When the broadcast is global, the base cost is multipled by this factor. If
the factor is 500, then global broadcasts cost 5x times the cost of the map's
broadcast.

(%) kebc_discount
-----------------
Discount on broadcast prices when the Kafra Pass is active.

////////////////////////////////////////////////////////////////////////////////
[05] Module: Stylist (ke_dye.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Offers cloth dye, hair dye and hair style changes, both by input and by wheel
browsing. Does not consumes dyestuffs because it is designed for custom dye
packages.

Variables
================================================================================

(1) kedy_enableHairstyle
------------------------
If one, the menu will include hair-style changing options, otherwise only
dye-changes are offered

(#) kedy_styles
---------------
Specifies the number of available hair styles

(#) kedy_hair
-------------
Specificies the number of hair dyes

(#) kedy_clothJN
(#) kedy_clothJ1ST
(#) kedy_clothJ2ND
(#) kedy_clothJSN
------------------
Specifies the number of cloth dyes based on job-type: Novices, First Classes,
Second Classes, Super Novices.
Note that special classes like Xmas or Wedding tend to not have palettes, and
it should be handled server-side so that switching palettes with this module
will not cause you client crashes.

////////////////////////////////////////////////////////////////////////////////
[06] Module: Job Changer (ke_jobchange.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Offers job changing, including rebirth and baby classes. The rebirth system
can allow people free changing through a different path, or forcing the
correct path. Zeny can be charged/granted as well as weapons on job-change.
Remaining Skill Points can be wiped, ignored or prevent the change from
happening. Before changing players can review the consequences of doing so,
including weapons to gain.

Finally, there's an option that makes it possible to skip the novice class
altogether and change directly to first jobs.

Variables
================================================================================

(#) kejc_skillsPolicy
---------------------
Determines what to do with remaining skill points upon change:
	0: No job changing until points are used.
	1: Extra Skill Points are wiped.
	2: Extra skill points are conserved.

(#) kejc_upperPolicy
--------------------
What to do about the advanced classes?
	0: Free for all, players can pick any advanced job regardless of the previous.
	1: Force mode, classes are auto-selected from the previous path. In the
	case the path could not be determined (players changed jobs previously
	using other npcs), players will be able to select their next job.

(#) kejc_disable
----------------
Permits disabling some job trees from the changer (add as required):
	1: You can't change to a S. Novice
	2: You can't change to Taekwon (but if you are a Taekwon already, you can
	   still change to Soul Linker/Star Gladiator)
	4: Can't change to GunSlinger
	8: Can't change to Ninja

(1) kejc_announce
-----------------
When 1, a global announce will be done upon change.

(1) kejc_resetDye
-----------------
If one, the cloth dye is reset upon changing.

(1) kejc_skipNovice
-------------------
If one, players can skip the novice class and directly into their first job.
Exploit proof, skills are wiped when changing to a 1st class this way, and
their basic skill level is set to 9.

(#) kejc_baseSN
---------------
Base Level required before changing into a Super Novice.

(#) kejc_base2ND
(#) kejc_job2ND
(#) kejc_cost2ND
----------------
Base level, Job level and zeny required to change into a second job. If the
cost is below zero, zeny will be given to the player instead of charged.

(#) kejc_baseRebirth
(#) kejc_jobRebirth
(#) kejc_costRebirth
--------------------
Base level, Job level and zeny required before doing a rebirth (change to High
Novice).

(1) kejc_rebirthReset
---------------------
If 1, when changing into a HighNovice characters will have their level reset
to 1 (with the additional 100 stat points)

(1) kejc_weaponPolicy
---------------------
If 1, characters will get a weapon upon job change. For each first&second
class there are two weapons to specify, the standard weapon and the "premium"
one.

(#) kejc_wBonusLv
-----------------
When characters reach this job level, they will receive the premium weapon instead of the normal one. If 0, premium weapons are disabled.

(#) 	kejc_weapon1[]
(#) 	kejc_weapon_21[]
(#) 	kejc_weapon_22[]
-----------------------
These arrays contain the IDs of the normal weapons received upon job change
for first, 2-1 and 2-2 classes (if weapon policy is in effect). Refer to the
comments in the config file for identifying which position is for which job.

(#) 	kejc_weapon2_21[]
(#) 	kejc_weapon2_22[]
--------------------
These arrays contain the ID of the premium weapons received upon job change
(if bonus weapon policy is in effect). Note that first classes can't get a
bonus weapon, and that Bard/Dancers both receive the same weapon. Refer to the
comments in the config file for identifying which position is for which job.

////////////////////////////////////////////////////////////////////////////////
[07] Module: Job Swapper (ke_jobswap.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Permits characters to change jobs among other jobs of their same "level".
This is, between first classes or between second classes. It is also possible
to store the last job used and revert to it at a later date. Dye, Job Level
and Skill Point count is saved, however the skill-tree is not saved and skills
need be reallocated. When reverting to the previous class, characters might be
able to return from an adv class to a normal one, but if they are baby
classes, they can't change back to a normal one.

Variables
================================================================================

(#) kejs_disable
-----------------
You can disable some classes from swapping using this setting (add numbers as
appropiate):
	1: Super Novices may not swap/swap to.
	2: Disable swapping for Taekwon/Star Gladiator/Soul Linker classes.
	4: Disable swapping to/from GunSlinger.
	8: Disable swapping to/from Ninja.
For example, if you set it to 12 (8+4), the swap menu will not include
Gunslinger nor Ninja, and they will find that they can't swap to other classes
neither.

(#) kejs_revertPolicy
---------------------
Determines if Reverting classes is possible and when:
	0: Cannot go back to the previous job.
	1: Can only go back if the previous job belongs to the same type as the
	first (is also a 1st/2nd job and is the same normal/adv/baby category).
	2: Can return to the previous job regardless (exception: when one of the
	two jobs is a baby job and the other is not).

(1) kejs_announce
-----------------
If 1 does a server announce when swapping jobs.

($) kejs_revertCost
-------------------
Cost of changing to the previous job.

(1) kejs_saveDye
----------------
If one, the dye is saved when swapping and restored upon revert.

(1) kejs_resetDye
-----------------
If 1 the clothe dye is reset upon swap.

(%) kejs_swapDiscount
(%) kejs_revertDiscount
-----------------------
Discount % to apply when the kafra pass is active for swapping/reverting.

(#) kejs_job1ST
(#) kejs_job2ND
---------------
Minimum job level before being able to swap among 1st/2nd classes.

($) kejs_cost1ST
($) kejs_cost2ND
----------------
Base cost of swaping jobs.

(%) kejs_discount1ST
(%) kejs_discount2ND
--------------------
Discount % to apply to the base cost for every job level above the minimum
required. For example, if the discount is 1% per level and you change when you
have +10 level more than the min necessary, you get a 10% discount.

(%) kejs_preserve1ST
(%) kejs_preserve2ND
--------------------
Indicates how much of the previous job level to preserve when changing. For
example, if the preserve value is 50 (50%) and you change from a level 40
Knight into Priest, you'll become a lv 20 Priest. Skill points are adjusted
accordingly so it's exploit-free.

////////////////////////////////////////////////////////////////////////////////
[08] Module: Smithery (ke_refine.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Offers the services usually found in the town smithery and then some more:
Item refining, Item Repairing, Ore Purification (rough oris -> pure ones).
Also sells Phracon and Emveretarcon.

Item refining can follow all the rules of normal refining, but it can also
override them to refine everything, or safe refines up to max level.

Ore Purification can be extended to include all the rough materials that
blacksmiths can purify (star dust -> star crumbs, for example).

Variables
================================================================================

(#) kerf_maxLv
--------------
Maximum refinement level of equipment.

(1) kerf_safe
-------------
If 1 success chances are ignoring and refining never fails.

(1) kerf_showChance
-------------------
If 1 the chance of success/failure is actually shown before confirming.

(1) kerf_refineAll
------------------
If 1 then unrefinable items will be refined too (like accesories)

(%) kerf_discount
-----------------
Discount rate to be applied during forging when the Kafra Pass is active.

($) kerf_armorCost
($) kerf_weaponLv1Cost
($) kerf_weaponLv2Cost
($) kerf_weaponLv3Cost
($) kerf_weaponLv4Cost
----------------------
Cost per level to refine armors, and level 1/2/3/4 weapons.

(1) kerf_purifyAll
------------------
If 1, the extended purify menu will be used, which shows how to purify steels,
star crumbs, etc; otherwise, just elus and oris are available.

($) kerf_repairCost
-------------------
Cost of repairing a broken weapon.

(%) kerf_repairDiscount
-----------------------
Repair Discount% when the kafra pass is active.

(1) kerf_repairSteel
--------------------
If 1, a steel will be required to repair items.

////////////////////////////////////////////////////////////////////////////////
[09] Module: Renting (ke_refine.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Allows renting of Cart/Falcon/PecoPeco. Note that the only restriction applied
is whether the character has "PushCart", "Falcon Taming" or "PecoPeco Riding",
it does not checks for the character's class.

It must also be noted that renting is free while the Kafra Pass is active.

Variables
================================================================================

(1) kert_cartOnly
-----------------
If 1, this module only offers cart rentals. Otherwise you can rent any of the three.

($) kert_cartCost
($) kert_falconCost
($) kert_pecoCost
-------------------
Cost for renting carts/falcons/pecos.

////////////////////////////////////////////////////////////////////////////////
[10] Module: Kafra Shop (ke_shop.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Allows invoking of a shop from the Kafra. Note that the shops must be of
invisible type (ke_shop.txt has an example of two such shops). The config
enables you to select whether the shop will allow buying, selling or
both, as well as the name of the shops to use in towns or dungeons (if you
want to use a custom shop defined elsewhere or want to use the same shop for
both).

Variables
================================================================================

(#) $@kesh_towntype
(#) $@kesh_duntype
-------------------
Specifies the type of transaction allowed at the shop. Use 1 to enable only
buying of items, 2 for only selling, or any other value for both.

(#) $@kesh_townshop$
(#) $@kesh_dunshop$
--------------------
Specifies the name of the shop to use for buying/selling of items. By default
the file includes two such shops which you can use, or you can disable them
and specify your own in the config changing these variables.

////////////////////////////////////////////////////////////////////////////////
[11] Module: Stat/Skill Market (ke_statmarket.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Not really a "market", this module lets players sell and buy stat/skill
points. Also allows for direct trading between stats/skills.

NOTE: Selling stat points can lead to zeny exploits if your server uses the
stat_db.txt file, because players can then sell all their stat points, do a
reset and get them back! To prevent this, set the following  battle config
option:
	use_statpoint_table: no
Alternatively, you can set the selling point of statpoints to be 0z.

Variables
================================================================================

($) kesm_stBuyPrice
($) kesm_stSellPrice
--------------------
Price for every stat point to be bought/sold.

($) kesm_skBuyPrice
($) kesm_skSellPrice
--------------------
Price for every skill point to be bought/sold.

(%) kesm_discount
-----------------
Discount price when kafra pass is active. Only applies to buying stats/skills.

(#) kesm_skTradePrice
(#) kesm_stTradePrice
---------------------
These two define how many stat points are traded per each skill point when
doing a direct conversion between stats/skills. The idea is that trading
directly one for the other should be cheaper than selling them and then buying
from the other. You can set both to the same value and the trading will have
no loss.
Notice that skTradePrice is the cost for converting stats to skills and
stTradePrice is the cost for converting skills to stats. So... NEVER set
stTradePrice higher than skTradePrice or you allow an easy exploit of infinite
stats/skills!

////////////////////////////////////////////////////////////////////////////////
[11] Module: Stat/Skill Resets/Raising (ke_stats.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Does stat/skills resets, and additionally helps characters raise their basic
stats faster. Optionally, skill resets will not touch the basic-skill level.
Price have two components: base fee and per level fee, so characters with
higher levels will have to pay more for a reset than low level ones.
For stat resetting, base level is taken into account. For skill resets, job
levels. For a dual reset, the addition of both is considered. Also, when taken
into consideration job level for second classes, the job-level at which one
changed jobs is also considered. So a Job level 1 Knight actually has job
level of 41~51.

NOTE: Be careful with quest skills. If your server is configured to reset
quest skills and players can get their quest skills for free, you are letting
them exploit the system and get unlimited skill points! (even worse if they
can sell'em in the Stat/Skills market module). Be sure to either remove free
quest skills npcs or make quest skills not resetable.

Variables:
================================================================================

($) kest_stResetCost
--------------------
Base cost of doing a stat reset.

($) kest_skResetCost
--------------------
Base cost of doing a skill reset.

($) kest_resetCost
------------------
Base cost of a dual reset.

($) kest_BaseLvCost
-------------------
Cost per Base level for doing a stat reset.

($) kest_JobLvCost
------------------
Cost per Job level for doing a skill reset.

($) kest_BothLvCost
-------------------
Cost per Base+Job level for doing a dual reset.

(%) kest_discount
-----------------
Discount % applied when Kafra Pass is active.

(1) kest_resetBasic
-------------------
If 1, the skill "basic skill" is also reset.

////////////////////////////////////////////////////////////////////////////////
[12] Module: Uncarding (ke_uncard.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
This module allows people to remove cards from their equipped items. Prices
are modified by the item type, the amount of cards and the refine level of the
item. There's also a chance the removal will fail and: Nothing is lost, the
card or item is lost, both are lost. The procedure may also require up to two
different items (and each one with their own specified qty).

Variables:
================================================================================

($) keuc_BaseCost
-----------------
Base cost of uncarding, modified by the following conditions:

($) keuc_CardCost
-----------------
Each card to be removed increases the total by this value.

($) keuc_UpgradeCostA
($) keuc_UpgradeCostW1
($) keuc_UpgradeCostW2
($) keuc_UpgradeCostW3
($) keuc_UpgradeCostW4
----------------------
These indicate the price increase per refine level for armors and weapons
levels 1/2/3/4. A +10 armor gets a price increases of ten times
keuc_UpgradeCostA.

(%) keuc_discount
-----------------
Discount % to apply when kafra pass is active.

(#) keuc_Mat1
(#) keuc_Qty1
-------------
Id and Qty of the first material that is needed to uncard. If the qty is zero,
then it's disabled.

(#) keuc_Mat2
(#) keuc_Qty2
-------------
id & Qty of the second material to use. Only valid if the first material was
also defined, use qty=0 to disable.

(%) keuc_Fail0Chance
--------------------
This is the safe failure chance (0-1000, where 1000 = 100.0%). A Safe failure
means the original item remains intact, but you are still charged the money
and the materials. Use 0 to disable this type of failure.

(%) keuc_Fail1Chance
--------------------
This is the partial failure chance (0-1000). Partial failures are when either
the cards or the item is lost. The player gets to choose which one is more
important before proceeding. Use 0 to disable.

(%) keuc_Fail2Chance
--------------------
Total failure chance (0-1000). When this triggers, both item and cards are
lost. Use 0 to disable.

////////////////////////////////////////////////////////////////////////////////
[14] Module: Town Warping (ke_warp_town.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
This module enables warping to towns. Currently all towns have the same
warping cost, independent of how far away they are placed. The only exception
is Niflheim, which is more of a dungeon than a town.

Variables
================================================================================

(1) kewt_showOnline
-------------------
If set to 1, the menus will display the online count of players in
towns.

($) kewt_cost
-------------
Cost of warping to a town.

($) kewt_niflCost
-----------------
Cost of warping to Niflheim. Different cost since Niflheim is more of a
dungeon than a city. Also, players can't warp to Niflheim until they do the
Niflheim Piano Key Quest.

(#) kewt_travel
---------------
Allows enabling the traveller system. The traveller system makes it so you can't
warp to a town until you have been there first by some other means (usually
walking) and saved with the Kafra Express in that town.
There are three valid values for this variable: 0, 1, 2.
0 - Disables this mode.
1 - Uses the mode on a per character basis; that is, each character needs to
travel to that town and save and that unlocks warping only for that
character.
2 - Uses the mode on a per account basis; that is, once a character has saved
on a town, all other (and future) characters from the same account have the
warp unlocked.

(1) kewt_free
-------------
If 1, players will be able to warp to towns even if they run out of money.
Niflheim excepted.

(%) kewt_discount
-----------------
Discount % to apply when warping while the Kafra Pass is active.

////////////////////////////////////////////////////////////////////////////////
[15] Module: Dungeon Warping (ke_warp_dungeon.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
Offers warps to all dungeons. Dungeon warping has two main operation modes:
Deep Dungeon Warping and Short Dungeon Warping. Deep Dungeon warping enables
players to warp to any level of any dungeon, while Short Dungeon Warping is
restricted to warping only to the entrance of the dungeon. Traveller's mode is
also available for dungeons.

Variables
================================================================================

(1) kewd_showOnline
-------------------
If set to 1, the menus will display the online online count of characters in
the dungeons.

(%) kewd_discount
-----------------
Discount % to apply when warping while the Kafra Pass is active.

(#) kewd_travel
---------------
Enables the traveller system. Just like the traveller's system for towns,
you can't warp to any dungeon to which you have not visited first on foot and
saved with the corresponding Kafra Express first.
If 1, traveller's mode is enabled on a per character basis (so once a
character saves, only that character has unlocked the warp).
If 2, traveller's mode is enabled on a per account basis (so once a character
saves, all charaters of the corresponding account have the warp unlocked).

(1) kewd_deep
-------------
This variable decides whether deep or short warps will be used. As explained
on the description, enabling deep warps allows warping to any dungeon level.

(%) kewd_levelCost
------------------
This variable only applies to deep warps. It indicates in percentage the cost
increase per level. For example, if the dungeon costs 1000z to warp to, and
the levelCost increase is of 50 (50%) then warping to level 2 costs 1500, lv3
costs 2000, and so on.

(%) kewd_entryDiscount
----------------------
The discount for using the Kafra in the dungeon entrance to warp within the
dungeon. For example, if you use the Kafra next to Payon dungeon to warp to
Payon Dungeon lv5, this discount is then applied. This variable only makes
sense on deep warp mode.

(1) kewd_turtleCave
-------------------
Only used on short warps. If 1, then warping to Turtle Dungeon should lead
directly to the cave's entrance, otherwise it warps you to the Island's
entrance.

($) kewd_<dungeon name>
---------------------
There is a config variable for every dungeon, it specifies the base cost of
warping to that dungeon (which is, the entrance level cost).

////////////////////////////////////////////////////////////////////////////////
[16] Module: PvP Warping (ke_pvp.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
The PvP warping module leads to the pvp maps named pvp_n_*-*. It has two
modes: simple and advanced. On simple mode, every player of every level gets
thrown into the pvp_n_1-* maps to nuke it out, on advanced mode each of
the map groups gets their own range of permissible levels to enter. Since
there is no sure way how characters should escape from a pvp map, they are
currently given a butterfly wing when they warp.

Variables
================================================================================

(1) kewp_showOnline
-------------------
If 1, the menu entries will display the online count of players of each entry.

(1) kewp_advanced
-----------------
Use the advanced pvp system if 1 (see module description)

($) kewp_cost
-------------
Cost of warping to the pvp rooms used in non-advanced rooms and "free for all"
in advanced mode.

($) kewp_cost1
($) kewp_cost2
($) kewp_cost3
($) kewp_cost4
($) kewp_cost5
($) kewp_cost6
($) kewp_cost7
--------------
The costs for each of the pvp room groups in advanced mode.

(#) kewp_baseLv1
(#) kewp_baseLv2
(#) kewp_baseLv3
(#) kewp_baseLv4
(#) kewp_baseLv5
(#) kewp_baseLv6
(#) kewp_baseLv7
----------------
These indicate the nominal base level for each room (which base level should
characters be around to join it). Applicable only to advanced mode.

(#) kewp_range
--------------
Specifies how far away the character's level can be from the nominal value to
still be allowed within the room. If the nominal level is 50 and the range is
3, only characters with levels 47-53 may join.

////////////////////////////////////////////////////////////////////////////////
[17] Module: WoE Warps (warp_woe.txt)
////////////////////////////////////////////////////////////////////////////////

Description
================================================================================
This module allows characters to warp into the WoE grounds during (or out of)
War of Emperium times. Players may warp directly into the Guild Dungeons if
the proper lines are uncommented on the warp menu (See warp_woe.txt's header
for the location of the menu entries):

 menu "- Cancel",-,
// "- Guild Dungeons",M_DUNGEON,
   "- Al De Baran Guild ("<some code>"z)",L_ALDEBARAN,
   "- Geffen Guild ("<some code>"z)",L_GEFFEN,
   "- Payon Guild ("<some code>"z)",L_PAYON,
   "- Prontera Guild ("<some code>"z)",L_PRONTERA;
   return;

Variables
================================================================================

(1) kewg_check
--------------
Does a guild check. If 1, only characters who belong to a guild can use these
warps.

(1) kewg_checkAgit
------------------
Does the WoE times check. If 1, only during War of Emperium the warps will be
active.

(1) kewg_showOnline
-------------------
Set to 1 to display in the menus the online count of players in each of the
guild areas. It also adds a menu entry that displays the summary of players
within each castle and guild grounds.

(%) kewg_discount
-----------------
Discount % to be applied for warps while the Kafra Pass is active.

($) kewg_baldur
($) kewg_luina
($) kewg_valkyrie
($) kewg_britoniah
------------------
Costs to the respective guild dungeons when enabled.

($) kewg_alDeBaran
($) kewg_geffen
($) kewg_payon
($) kewg_prontera
------------------
Costs to each of the guild grounds.

////////////////////////////////////////////////////////////////////////////////
[18] The kafras.txt file: About Kafra Definitions
////////////////////////////////////////////////////////////////////////////////

The file kafras.txt contains the definition of the Kafras, which is the actual
sprite on-screen that characters speak with. An enabled kafra may look like
this:

//Alberta
alberta,113,53,7 script   Kafra Express  116,{
   callfunc "F_KafraExpress","Kafra Express","kafra_02",0,"alberta",116,57;
}

And a disabled/commented Kafra would look like this:

//Prontera Guild Grounds
//prt_gld,127,163,5   script   Kafra Express  115,{
//   callfunc "F_KafraExpress","Kafra Express","kafra_03",1,"prt_gld",129,170;
//}

For scripters, the way to define an NPC is not new, and beyond the scope of
this document, so I'll only document the function "F_KafraExpress":

F_KafraExpress (String "Kafra's name", String "kafra image file", int location, String map, int x, int save y)

The first parameter, the Kafra's name, is the name that will be displayed all
over the dialogue windows. The Kafra Image file is the image that is to be
displayed on the screen during the npc chat (without the extension). You can
use "" to disable the image.

Location refers to the type of Kafra. Type=0 refers to Kafras in towns while
Type=1 refers to Kafras placed in the wild, ie: in dungeons. Type=2 is for
Kafras that should count as both town & dungeon. Under these situations, the
menus displayed are those of the town, the type is used for the traveller's
warping mode. Finally, Type=3 is a special type used only for the Niflheim
Kafra.

The last three Parameters are used to define the save location when you save
your respawn. They are not needed if you use the "Save-on-spot" feature, but
it's recommended to pass them nevertheless. Note that the map name will still
be used when using traveller's mode.
