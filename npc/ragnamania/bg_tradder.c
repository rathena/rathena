// //===== eAthena Script =======================================
// //= BattleGround Supplier
// //===== By: ==================================================
// //= Brian
// //===== Current Version: =====================================
// //= 1.0
// //===== Compatible With: =====================================
// //= eAthena SVN (+ source edits)
// //===== Description: =========================================
// //= Sells consumables that can ONLY be used on 'bg_consume' maps.
// //===== Additional Comments: =================================
// //= http://www.eathena.ws/board/index.php?showtopic=268355
// //= 1. To use, create a char named "Battleground"
// //= 2. edit the #define BG_CHARID in battleground.h
// //= 3. edit the 'set .@BG_CHARID' in this file (line 43)
// //============================================================

 // bat_room,165,146,3	script	Telma2	701,{
 // 	OnInit:
 // 	setmapflag "bat_room",mf_bg_consume;
 // 	debugmes "entrou";
 // 	end;
 // }
// 	mes "[Telma]";
// 	mes "Welcome, mighty warrior.";
// 	mes "Do you need supplies for your battles?";
// 	mes "I can exchange supplies for your badges...";
// 	next;
	
// 	switch(select("150 Battleground's Condensed White Potion:90 Battleground's Blue Potion:3 Battleground's Poison Bottle:30 Battleground's Fire Bottle:30 Battleground's Acid Bottle:30 Battleground's Plant Bottle:30 Battleground's Marine Sphere Bottle:15 Battleground's Glistening Coat:50 Battleground's Yellow Gemstone:50 Battleground's Red Gemstone:100 Battleground's Blue Gemstone:5 Battleground's Speed Potion:20 Battleground's Cobweb")) {
// 		case 1:		callsub S_BuyConsumableBG,547,150;	// Condensed White Potion
// 		case 2:		callsub S_BuyConsumableBG,505,90;	// Blue Potion
// 		case 3:		callsub S_BuyConsumableBG,678,3;	// Poison Bottle
// 		case 4:		callsub S_BuyConsumableBG,7135,30;	// Fire Bottle
// 		case 5:		callsub S_BuyConsumableBG,7136,30;	// Acid Bottle
// 		case 6:		callsub S_BuyConsumableBG,7137,30;	// Plant Bottle
// 		case 7:		callsub S_BuyConsumableBG,7138,30;	// Marine Sphere Bottle
// 		case 8:		callsub S_BuyConsumableBG,7139,15;	// Glistening Coat
// 		case 9:		callsub S_BuyConsumableBG,715,50;	// Yellow Gemstone
// 		case 10:	callsub S_BuyConsumableBG,716,50;	// Red Gemstone
// 		case 11:	callsub S_BuyConsumableBG,717,100;	// Blue Gemstone
// 		case 12:	callsub S_BuyConsumableBG,12016,5;	// Speed Potion
// 		case 13:	callsub S_BuyConsumableBG,1025,20;	// Cobweb
// 	}
// 	end;

// S_BuyConsumableBG:
// 	set .@BG_CHARID, 999996; // character named "Battleground"
// S_BuyConsumable:
// 	set .@item_id, getarg(0);
// 	set .@amt, getarg(1);
	
// 	mes "[Telma]";
// 	mes "How many sets of ^0000FF"+ .@amt +" "+ getitemname(.@item_id) +"s^000000 do you want?";
// 	mes "1 = "+ .@amt   +" supplies = 1 badge";
// 	mes "2 = "+ .@amt*2 +" supplies = 2 badges";
// 	mes "3 = "+ .@amt*3 +" supplies = 3 badges";
// 	mes "etc..";
// 	next;
// 	input .@sets;
// 	mes "[Telma]";
// 	if (.@sets < 1) {
// 		mes "Enter a positive amount, greater than zero.";
// 		close;
// 	}
// 	mes "So you want ^0000FF"+ (.@amt*.@sets) +" "+ getitemname(.@item_id) +"s^000000...";
// 	if (.@BG_CHARID) mes "Remember these can only be used in Battlegrounds.";
// 	mes "It will cost you "+ .@sets +" badge(s)...";
// 	next;
// 	switch(select("Give her "+ .@sets +" Bravery Badge(s):Give her "+ .@sets +" Valor Badge(s):Give her "+ (3*.@sets) +" Heroism Badges")) {
// 		case 1: set .@badge,7828; set .@cost,1; break;
// 		case 2: set .@badge,7829; set .@cost,1; break;
// 		case 3: set .@badge,7773; set .@cost,3; break;
// 	}
// 	mes "[Telma]";
// 	if (countitem(.@badge) < (.@cost*.@sets)) {
// 		mes "You do not have enough ^FF0000"+getitemname(.@badge)+"s^000000 to buy "+(.@amt*.@sets)+" "+getitemname(.@item_id)+"s.";
// 	} else if (!checkweight(.@item_id, .@amt*.@sets)) {
// 		mes "You are overweight or have too many items in your inventory.";
// 	} else {
// 		delitem .@badge, .@cost*.@sets;
// 		if (.@BG_CHARID) { // Battleground's Consumables
// 			getitem2 .@item_id,(.@amt*.@sets),1,0,0, 254, 0, .@BG_CHARID & 0xFFFF, .@BG_CHARID >> 0x10;
// 		} else {
// 			getitem .@item_id,(.@amt*.@sets);
// 		}
// 		mes "Here are your: ^0000FF"+(.@amt*.@sets)+" "+getitemname(.@item_id)+"s^000000!";
// 		if (.@BG_CHARID) mes "Remember, they can only be used in Battlegrounds.";
// 	}
// 	close;
// }

// bat_a01	mapflag	bg_consume
// bat_a02	mapflag	bg_consume
// bat_b01	mapflag	bg_consume
// bat_b02	mapflag	bg_consume
// bat_c01	mapflag	bg_consume
// bat_c02	mapflag	bg_consume
// bat_c03	mapflag	bg_consume