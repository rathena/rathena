//===== rAthena Script =======================================
//= Card Trader
//===== By: ==================================================
//= Euphy
//===== Current Version: =====================================
//= 1.1 
//===== Compatible With: =====================================
//= rAthena SVN
//===== Description: =========================================
//= Exchange cards for points.
//============================================================

prontera,165,196,6	script	Card Trader	90,{
	mes "[Card Trader]";
	mes "Hi, "+strcharinfo(0)+"!";
	mes "What can I do for you?";
	next;
	switch(select(" > Information: > Trade in cards: > Leave")) {
	case 1:
		mes "[Card Trader]";
		mes "Do you find that you've got";
		mes "useless cards lying around?";
		mes "I'll be glad to take them off";
		mes "your hands!";
		next;
		mes "[Card Trader]";
		mes "I'll give you ^0055FF 1x "+getitemname(.Coin)+"^000000";
		mes "for each card you give me.";
		mes "You can use those coins to join";
		mes "the events during the ^AA0000Feira de Fim-De-Mes^000000.";
		mes "and other possible events.";
		emotion ET_MONEY;
		close;
	case 2:
		mes "[Card Trader]";
		mes "Select the cards you";
		mes "want to trade in.";
		deletearray @sold_nameid[0],getarraysize(@sold_nameid);
        getinventorylist;
        for (.@i=0;.@i<@inventorylist_count;.@i++) {
            if(@inventorylist_id[.@i] > 4000 && @inventorylist_id[.@i] < 4700) {
                setarray .@Shop[getarraysize(.@Shop)],@inventorylist_id[.@i];
                setarray .@Shop[getarraysize(.@Shop)],@inventorylist_amount[.@i];
            }
        }
        npcshopdelitem "card_shop",909;
        for(set .@i,0; .@i<getarraysize(.@Shop); set .@i,.@i+2)
            npcshopadditem "card_shop",.@Shop[.@i],.@Shop[.@i+1];
		callshop "card_shop",2;
		npcshopattach "card_shop";
		end;
	case 4:
		mes "[Card Trader]";
		mes "*yawn*";
		mes "See you later!";
		emotion ET_SLEEPY;
		close;		
	}

OnSellItem:
	mes "Cards to sell:";
	mes "-----------------------------------";
	for(set .@i,0; .@i<getarraysize(@sold_nameid); set .@i,.@i+1)
		if (@sold_nameid[.@i] > 4000 && @sold_nameid[.@i] < 4700) {
			set .@card_id[getarraysize(.@card_id)], @sold_nameid[.@i];
			set .@card_amt[getarraysize(.@card_amt)], @sold_quantity[.@i];
			mes "  ^777777"+@sold_quantity[.@i]+"x "+getitemname(@sold_nameid[.@i])+"^000000";
			set .@card_total, .@card_total + (@sold_quantity[.@i] * 1);
		}
	deletearray @sold_nameid[0], getarraysize(@sold_nameid);
	deletearray @sold_quantity[0], getarraysize(@sold_quantity);
	if (!.@card_id) {
		mes "  ^777777(none)^000000";
		emotion ET_SWEAT;
		close;
	}
	mes " ";
	mes "-- Total: ^0055FF"+.@card_total+" "+getitemname(.Coin)+(.@card_total>1?"s":"")+"^000000 --";
	next;
	if(select(" > ^0055FFComplete trade...^000000: > ^777777Cancel^000000") == 2) {
		mes "[Card Trader]";
		mes "Oh, okay...";
		emotion ET_SCRATCH;
		close;
	}
	for(set .@i,0; .@i<getarraysize(.@card_id); set .@i,.@i+1)
		delitem .@card_id[.@i],.@card_amt[.@i];
	getitem .Coin,.@card_total;
	mes "[Card Trader]";
	mes "All done!";
	emotion ET_DELIGHT;
	close;

OnInit:
	set .Level,0;   		// Minimum monster level to trade corresponding cards.
	set .Coin,7539;	        // Variable to store points.
	// Card Shop items: <ID>,<point cost>
    
	end;
}
-	shop	card_shop	-1,909:-1