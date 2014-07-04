//===== rAthena Script ======================================= 
//= Mercenary related NPCs
//===== By: ================================================== 
//= L0ne_W0lf
//===== Current Version: ===================================== 
//= 1.5
//===== Compatible With: ===================================== 
//= rAthena Project
//===== Description: ========================================= 
//= [Official Conversion]
//= Sells Lancer, Sword, and Archer mercenaries,
//= along with related mercenary items.
//===== Additional Comments: ================================= 
//= 1.0 First version. [L0ne_W0lf]
//= 1.1 Optimization. [Zephyrus]
//= 1.2 Removed redundent input. [L0ne_W0lf]
//= 1.3 Duplicates now spawned from floating NPCs. [L0ne_W0lf]
//= 1.4 Optimized and fixed wrong item id. [Joseph]
//= 1.5 Added Mercenary Guild Switch & fixed a bug
//=		in izlude duplicate. [Capuche]
//= 1.5a Change to F_GM_NPC function in Mercenary Guild Switch
//============================================================ 

-	script	Mercenary Manager#main	-1,{
	setarray .@name$, 	"Spear", 			"Sword", 			"Bow";
	setarray .@faith$, 	"SPEAR_MERC_GUILD",	"SWORD_MERC_GUILD",	"ARCH_MERC_GUILD";
	setarray .@item, 	12182,				12172,				12162;
	.@npc$ = strnpcinfo(2);
	for (.@size = getarraysize(.@name$); .@type < .@size; .@type++)
		if( compare( .@npc$,.@name$[.@type] ) ) break;
	.@faith_merc = mercenary_get_faith(getd(.@faith$[.@type]));
	mes "[Mercenary Manager]";
	mes "Welcome to the";
	mes .@name$[.@type] + " Mercenary Guild.";
	mes "What can I do for you?";
	next;
	switch(select("Hire Mercenary:Mercenary Info:Nothing:10th Grade Mercenaries")) {
		case 1:
			mes "[Mercenary Manager]";
			mes "You want to hire a";
			mes .@name$[.@type] + " Mercenary?";
			mes "Which Grade were you";
			mes "interested in hiring?";
			next;
			for (.@i = 1; .@i <= 9; .@i++)
				.@menu$ = .@menu$ + callfunc("F_GetNumSuffix",.@i) + " Grade " + .@name$[.@type] + " Mercenary:";
			.@Grade = select(.@menu$);
			.@BaseLevel = 5 + (.@Grade * 10);
			.@BaseLevel = (.@val > 90)? 90 : .@BaseLevel;
			.@ZenyCost = 7 * .@Grade;
			setarray .@FaithCost[7], 50,100,300;
			.@FaithCost = .@FaithCost[.@Grade];
			mes "[Mercenary Manager]";
			mes "So you want to hire a " + callfunc("F_GetNumSuffix",.@Grade);
			mes "Grade " + .@name$[.@type] + " Mercenary?";
			mes "You need to have attained";
			mes "Base Level " + .@BaseLevel + " or higher, and";
			mes "must pay the " + .@ZenyCost + ",000 zeny fee.";
			next;
			if(select("Yes:No") == 2) {
				mes "[Mercenary Manager]";
				mes "Oh, really? Well, now";
				mes "might not be a good time";
				mes "for you to consider hiring";
				mes "a Mercenary, but please feel";
				mes "free to come back if your";
				mes "needs change. Thank you~";
			}
			else if(.@FaithCost && .@faith_merc < .@FaithCost) {
				mes "[Mercenary Manager]";
				mes "Oh... Your Loyalty rating";
				mes "with the " + .@name$[.@type] + " Mercenary";
				mes "Guild isn't high enough to";
				mes "hire this Mercenary. Please";
				mes "come back after you earn";
				mes "" + .@FaithCost + " or more Loyalty with us.";
			}
			else if (BaseLevel < .@BaseLevel) {
				mes "[Mercenary Manager]";
				mes "I'm sorry, but your Base";
				mes "Level isn't high enough";
				mes "to hire this Mercenary.";
				mes "Please come back to me";
				mes "once you reach Base Level " + .@BaseLevel + ".";
			}
			else if (Zeny < .@ZenyCost * 1000) {
				mes "[Mercenary Manager]";
				mes "I'm sorry, but you";
				mes "don't have enough zeny";
				mes "to hire this Mercenary.";
				mes "The hiring fee is " + .@ZenyCost + ",000 zeny.";
			}
			else {
				mes "[Mercenary Manager]";
				mes "Great! Our Mercenaries";
				mes "are sincere and devoted";
				mes "to protecting their clients.";
				mes "Summoned Mercenaries will";
				mes "offer their support to you for";
				mes "30 minutes. Take care now.";
				Zeny = Zeny - (.@ZenyCost * 1000);
				getitem .@item[.@type] - 10 + .@Grade, 1;
			}
			close;
		case 2:
			mes "[Mercenary Manager]";
			mes "Mercenaries are soldiers";
			mes "that will fight at your side";
			mes "on the battlefield, but there";
			mes "are a few terms and conditions";
			mes "you must fulfill to hire them.";
			next;
			mes "[Mercenary Manager]";
			mes "You must fulfill a level";
			mes "requirement and pay a zeny";
			mes "fee to hire a Mercenary.";
			mes "Higher grade Mercenaries";
			mes "will also require that you";
			mes "build a Loyalty rating with us.";
			next;
			mes "[Mercenary Manager]";
			mes "Mercenary contracts can't be";
			mes "transferred to other people,";
			mes "and we only allow a 5 Base Level difference between the Mercenary";
			mes "and client so you can't hire one much stronger than you.";
			next;
			mes "[Mercenary Manager]";
			mes "Well, you can figure out the";
			mes "details when you actually form";
			mes "a contract with one of our";
			mes "Mercenaries, and receive";
			mes "the Summon Scroll that will";
			mes "call a Mercenary to your side.";
			next;
			mes "[Mercenary Manager]";
			mes "You can't give this scroll";
			mes "to anyone else, and the";
			mes "Mercenary will only remain";
			mes "with you for 30 minutes after";
			mes "you summon him. Don't forget";
			mes "about the time limit, okay?";
			close;
		case 3:
			mes "[Mercenary Manager]";
			mes "No? You didn't need any";
			mes "help? Well, feel free to";
			mes "ask me if you have any";
			mes "questions about Mercenaries.";
			close;
		case 4:
			mes "[Mercenary Manager]";
			mes "10th Grade Mercenaries are";
			mes "the best we have to offer,";
			mes "and we use different criteria";
			mes "for our clients to hire them.";
			mes "There's no zeny fee, but you";
			mes "must have 500 Loyalty.";
			next;
			mes "[Mercenary Manager]";
			mes "Once you make a contract";
			mes "with a 10th Grade Mercenary,";
			mes "your Loyalty rating will be";
			mes "decreased by 400. In other";
			mes "words, you pay 400 Loyalty";
			mes "to hire a 10th Grade Mercenary.";
			next;
			mes "[Mercenary Manager]";
			mes "You must also be at";
			mes "Base Level 90 or higher to";
			mes "hire a 10th Grade Mercenary.";
			mes "Are you still interested in";
			mes "forming this contract?";
			next;
			if (select("Yes:No") == 2) {
				mes "[Mercenary Manager]";
				mes "I understand... It takes";
				mes "hard work and sacrifice to";
				mes "even reach the point where";
				mes "you can hire a 10th Grade";
				mes "Mercenary. Have you considered";
				mes "hiring a lower grade Mercenary?";
			}
			else if (.@faith_merc < 500) {
				mes "[Mercenary Manager]";
				mes "I'm sorry, but your";
				mes "Loyalty rating is too";
				mes "low to hire a 10th Grade";
				mes "Mercenary. You must have";
				mes "500 or more Loyalty to";
				mes "form a contract with one.";
			}
			else if (BaseLevel < 90) {
				mes "[Mercenary Manager]";
				mes "I'm sorry, but you must";
				mes "be at Base Level 90 or";
				mes "higher to form a contract";
				mes "with a 10th Grade Mercenary.";
			}
			else {
				mes "[Mercenary Manager]";
				mes "Congratulations! It looks";
				mes "like you're planning in taking";
				mes "on some very dangerous work";
				mes "since you're hiring a 10th";
				mes "Grade Mercenary. I wish you";
				mes "the best of luck with him.";
				mercenary_set_faith getd(.@faith$[.@type]),-400;
				getitem .@item[.@type],1;
			}
			close;
	}
}
prontera,41,337,5	duplicate(Mercenary Manager#main)	Mercenary Manager#Spear	8W_SOLDIER
pay_arche,99,167,4	duplicate(Mercenary Manager#main)	Mercenary Manager#Bow	4_M_JOB_HUNTER

// Mercenary Merchant NPCs
//============================================================
-	script	Mercenary Merchant#dummy::MercMerchant	-1,{
	mes "[Mercenary Goods Merchant]";
	mes "Hello, I sell goods";
	mes "that Mercenaries can";
	mes "use. Is there anything";
	mes "in particular that";
	mes "you're looking for?";
	next;
	setarray .@item, 12184,	12185,	12241,	12242,	12243;
	setarray .@cost, 1750,	3000,	560,	1050,	2100;
	for (.@size = getarraysize(.@item); .@i < .@size; .@i++)
		.@menu$ = .@menu$ + getitemname(.@item[.@i]) + ":";
	.@m = select(.@menu$)-1;
	.@str$ = .@cost[.@m] + "";
	.@len = getstrlen(.@str$);
	.@str_cost$ = (.@len <= 3)? .@str$ : insertchar(.@str$,",",.@len-3);
	mes "[Mercenary Goods Merchant]";
	mes getitemname(.@item[.@m]);
	mes "each cost " + .@str_cost$ + " zeny.";
	mes "How many would you like?";
	next;
	input .@input;
	if (!.@input) {
		mes "[Mercenary Goods Merchant]";
		mes "You changed your mind?";
		mes "Alright, feel free to come";
		mes "back to me whenever you want";
		mes "to buy any Mercenary Potions.";
		close;
	}
	else if (.@input < 0 || .@input > 10000) {
		mes "[Mercenary Goods Merchant]";
		mes "I'm sorry, but you";
		mes "can only buy up to";
		mes "10,000 of these potions";
		mes "at a time. Please enter";
		mes "a number from 1 to 10,000.";
		close;
	}
	else if (set(.@total_po, .@input * .@cost[.@m]) > Zeny) {
		mes "[Mercenary Goods Merchant]";
		mes "I'm sorry, but you don't";
		mes "have enough zeny for this";
		mes "many potions. Well, I'll be";
		mes "be here when you're ready";
		mes "to purchase something";
		mes "for your Mercenaries.";
		close;
	}
	else if (!checkweight(.@item[.@m], .@input)) {
		mes "[Mercenary Goods Merchant]";
		mes "If I gave you that many";
		mes "potions, you wouldn't be";
		mes "able to carry them with you.";
		mes "Please come back after";
		mes "you free up some space";
		mes "in your Inventory.";
		close;
	}
	mes "[Mercenary Goods Merchant]";
	if (.@input == 1) 
		mes "Here's your " + getitemname(.@item[.@m]) + ".";
	else {
		mes "Here you are, this is exactly";
		mes callfunc("F_InsertPlural",.@input,getitemname(.@item[.@m])) + ".";
	}
	mes "Thank you, and please come";
	mes "again when you need more";
	mes "potions for your Mercenaries.";
	Zeny = Zeny - .@total_po;
	getitem .@item[.@m], .@input;
	close;
}
prontera,30,337,4	duplicate(MercMerchant)	Mercenary Merchant#Spear	8_F_GIRL
pay_arche,102,167,5	duplicate(MercMerchant)	Mercenary Merchant#Bow	4_F_CAPEGIRL

// Mercenary Switch NPCs
//============================================================
-	script	Mercenary_Switch	-1,{

	setarray .@name$, "Spear","Sword","Bow";
	.@npc$ = strnpcinfo(2);
	for (.@size = getarraysize(.@name$); .@type < .@size; .@type++)
		if (.@npc$ == .@name$[.@type]) break;

	mes "[Checker]";
	mes "Please input a password.";
	next;
	.@i = callfunc("F_GM_NPC",1854,0,0,10000);
	mes "[Checker]";
	if (.@i == -2)
		mes "Error.";
	else if (.@i == 0)
		mes "Wrong number";
	else if (.@i == 1) {
		mes "Please select.";
		next;
		switch( select( "Turn off Mercenary NPC","Turn on Mercenary NPC" ) ) {
		case 1:
			mes "NPCs are turned off.";
			disablenpc "Mercenary Manager#" + .@name$[.@type];
			disablenpc "Mercenary Merchant#" + .@name$[.@type];
			break;
		case 2:
			mes "NPCs are turned on.";
			enablenpc "Mercenary Manager#" + .@name$[.@type];
			enablenpc "Mercenary Merchant#" + .@name$[.@type];
			break;
		}
	}
	close;
}
prontera,299,379,5	duplicate(Mercenary_Switch)	Mercenary Switch#Spear	8W_SOLDIER
pay_arche,170,185,5	duplicate(Mercenary_Switch)	Mercenary Switch#Bow	4_F_CAPEGIRL
izlude,245,250,4	duplicate(Mercenary_Switch)	Mercenary Switch#Sword	4_F_HUWOMAN
