//===== rAthena Script ======================================= 
//= Renters
//===== By: ================================================== 
//= kobra_k88, mod by Lupus
//===== Current Version: ===================================== 
//= 2.2
//===== Compatible With: ===================================== 
//= rAthena Project
//===== Description: ========================================= 
//= Knight and Crusader Peco Peco Breeders, Falcon Breeder scripts
//===== Additional Comments: ================================= 
//= Fully working
//= Added another Falcon Master into the Hunters Guild [Lupus]
//= 1.2: replaced checkoption(x) with checkriding,checkfalcon [Lupus]
//= 1.3: Added support Adv Classes + Baby Class [Lupus]
//= 1.4: Added different prices for normal, advanced and baby classes
//= but you could set them to the same [Lupus]
//= 1.5: Fixed spelling mistakes [Nexon]
//= 1.5a: Small fix (Falcon Taming -> Falcon Mastery) [Komurka]
//= 1.6: Moved the Falcon Master to Hugel [Poki#3]
//= 1.7 Updated to Aegis 10.3 standards. [L0ne_W0lf]
//= 1.8 Added 3rd Job creature NPCs (Dragon/Gryphon) [L0ne_W0lf]
//= 1.9 Enabled Gryphon Renter NPC and added Mado Gear NPC. [Masao]
//= 2.0 Fixed problems with third classes and new mounts. [Euphy]
//= 2.1 Moved some renters to a separate renewal file. [Daegaladh]
//= 2.2 Added warg checks. [Euphy]
//============================================================ 

// PecoPeco Breeder (for Knights) :: breeder
//============================================================
prontera,55,350,5	script	Peco Peco Breeder#knt	105,{
	if(Upper==0) set .@price,2500; //Normal Peco - default price
	if(Upper==1) set .@price,2500; //Armored Peco
	if(Upper==2) set .@price,2500; //Baby Peco

	mes "[Peco Peco Breeder]";
	if (BaseJob == Job_Knight && Class < Job_Rune_Knight) {
		mes "Welcome.";
		mes "Honorable Knight,";
		mes "would you like to rent";
		mes "a Peco Peco? The rental";
		mes "fee is "+.@price+" zeny.";
		next;
		switch(select("Rent Peco Peco:Cancel")) {
		case 1:
			if (Zeny < .@price) {
				mes "[Peco Peco Breeder]";
				mes "You do not";
				mes "have enough zeny.";
				mes "Are you...";
				mes "bankrupt?";
				close;
			}
			else if(getskilllv("KN_RIDING") == 0) {
				mes "[Peco Peco Breeder]";
				mes "I'm sorry, but you're";
				mes "not eligible for this";
				mes "service. Please go learn";
				mes "the Peco Peco Ride skill first.";
				close;
			}
			else if(checkriding()) {
				mes "[Peco Peco Breeder]";
				mes "You're already";
				mes "mounted on a";
				mes "Peco Peco.";
				close;
			}
			else if(ismounting()) {
				mes "[Peco Peco Breeder]";
				mes "Please remove your cash mount.";
				close;
			}
			set Zeny, Zeny-.@price;
			setriding;
			close;
		case 2:
			mes "[Peco Peco Breeder]";
			mes "I see.";
			mes "Well then,";
			mes "have a good day.";
			close;
		}
	} else {
		mes "I'm sorry, but these";
		mes "Peco Pecos are only";
		mes "available for Knights";
		mes "and Lord Knights.";
		close;
	}
}

// Grand PecoPeco Breeder (for Crusaders)
//============================================================
prontera,232,318,3	script	Peco Peco Breeder#cru	105,{
	if(Upper==0) set .@price,3500; //Normal Peco - default price
	if(Upper==1) set .@price,3500; //Armored Peco
	if(Upper==2) set .@price,3500; //Baby Peco

	mes "[PecoPeco Breeder]";
	if (BaseJob == Job_Crusader && Class < Job_Rune_Knight) {
		if(Upper != 1 ) mes "Welcome, Crusader.";
		else mes "Welcome, Paladin.";
		mes "We have a special";
		mes "Peco Peco prepared";
		mes "for you. To rent one";
		mes "will cost "+.@price+" zeny.";
		next;
		switch(select("Rent a PecoPeco:Quit")) {
		case 1:
			if (Zeny < .@price) {
				mes "[Peco Peco Breeder]";
				mes "You do not";
				mes "have enough zeny.";
				mes "If you would like";
				mes "a Peco Peco please";
				mes "bring "+.@price+" zeny...";
				close;
			}
			else if(getskilllv("KN_RIDING") == 0) {
				mes "[Peco Peco Breeder]";
				mes "You must first learn";
				mes "to ride a PecoPeco before";
				mes "I can rent one to you.";
				close;
			}
			else if(checkriding()) {
				mes "[Peco Peco Breeder]";
				mes "You are already";
				mes "mounted on a Peco Peco.";
				close;
			}
			else if(ismounting()) {
				mes "[Peco Peco Breeder]";
				mes "Please remove your cash mount.";
				close;
			}
			set Zeny, Zeny-.@price;
			setriding;
			close;
		case 2:
			mes "[PecoPeco Breeder]";
			mes "See you around.";
			close;
		}
	} else {
		mes "What can I do for you?";
		mes "Please be aware that";
		mes "this Peco Peco rental";
		mes "service is strictly for";
		mes "Crusaders and Paladins.";
		close;
	}
}

// Falcon Master :: breeder
//============================================================
hu_in01,381,304,5	script	Falcon Breeder#hnt	105,{
	if(Upper==0) set .@price,2500; //Normal Falcon - default price
	if(Upper==1) set .@price,2500; //Scarf Falcon
	if(Upper==2) set .@price,2500; //Baby Falcon

	mes "[Falcon Breeder]";
	if (BaseJob == Job_Hunter) {
		if (checkwug()) {
			mes "Um...";
			mes "You can't rent a Falcon";
			mes "until you dismiss your warg first!";
			close;
		}
		mes "Do you need a Falcon?";
		mes "You can rent your own";
		mes "trusty bird of prey for a";
		mes "fee of just "+.@price+" zeny~";
		next;
		switch(select("Rent Falcon:Cancel")) {
		case 1:
			if (checkwug()) {
				mes "[Falcon Breeder]";
				mes "Um...";
				mes "You can't rent a Falcon";
				mes "until you dismiss your warg first!";
				close;
			}
			if (Zeny < .@price) {
				mes "[Falcon Breeder]";
				mes "What is this?";
				mes "You don't have";
				mes "enough zeny?!";
				mes "You better start";
				mes "hunting money";
				mes "instead of monsters~";
				close;
			}
			else if(getskilllv("HT_FALCON") == 0) {
				mes "[Falcon Breeder]";
				mes "Gosh~";
				mes "Go learn how to";
				mes "manage a Falcon";
				mes "first! I can't rent one";
				mes "to you if you can't";
				mes "handle it, you know.";
				close;
			}
			else if(checkfalcon()) {
				mes "[Falcon Breeder]";
				mes "Um...";
				mes "You already have";
				mes "a Falcon. It's right";
				mes "there, can't you see it?";
				close;
			}
			set Zeny, Zeny-.@price;
			setfalcon;
			close;
		case 2:
			mes "[Falcon Breeder]";
			mes "W-wait, where're";
			mes "you goin'? These";
			mes "Falcons are top notch,";
			mes "I guarantee it! C'mon, yo~";
			close;
		}
	} else {
		mes "Young fool!";
		mes "Falcons can only";
		mes "be used by Hunters";
		mes "and Snipers, capish?";
		mes "...Heh heh, jealous?";
		close;
	}
}