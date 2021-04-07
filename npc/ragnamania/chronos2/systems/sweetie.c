-	script	Sweetie	-1,{
	function Go; 
	function Disp; 
	function Pick; 
	function Restrict;
	mes .n$;
	set .@time, gettime(3);
	mes "Good "+((.@time < 5 || .@time > 17)?"evening":((.@time < 12)?"morning":"afternoon"))+", "+strcharinfo(0)+"!";
	if(checkquest(64502) == 1) {
		mes strcharinfo(0)+"!! Omg!!";
		mes "Come over here! Let me have a nice look at you!";
		next;
		mes .n$;
		mes "You continue to be the most precious gem in the whole world!";
		mes "You are here for your gift, right?";
		next;
		mes .n$;
		mes "Here, take these... and please take care!";
		getitem 601,20; //Wing_Of_Fly
		getitem 602,5; //Wing_Of_Butterfly
		getitem 7059,5; //Cargo_Free_Ticket
		getitem 7060,5; //Warp_Free_Ticket
		completequest 64502;
		next;
	}
	mes "What can I do for you?";
	emotion ET_DELIGHT;
	next;
	disable_items;
	switch(select("Save Respawn Point:Open Storage:Open Guild Storage:Teleport Services:Healing Services:Rentals") ) {
		case 1:
			mes .n$;
			if (getmapflag(strcharinfo(3),2) == 0 ) {
				mes "Perfect! I got your savepoint.";
				close2;
				atcommand "@save";
				end;
			} else {
				mes "Sorry, but apparently we can't set your respawn to this map.";
				close;
			}
			break;
		case 2:
		OnOpenStorage:
			if(vip_status(1)) {
				mes .n$;
				mes "Which storage should we open?";
				next;
				switch(select("Premium Storage [^00DD00free^000000]:Standard Storage [^ff0000200z^000000]")){
					case 1:
						close2;
						openstorage2 10,STOR_MODE_GET|STOR_MODE_PUT;
						end;
						break;
					case 2:
						if(Zeny < 200) {
							mes .n$;
							mes "I am afraid you don't have enough Zeny for this.";
							close;
						} else {
							Zeny -= 200;
							close2;
							openstorage;
							end;
						}
						break;
				}
			} else {
				mes .n$;
				mes "This will cost you ^ff0000200z^000000. Do you want to continue?";
				next;
				if(select("Yes, please.:No, I've changed my mind.") == 2) {
					close;
				} else {
					if(Zeny < 200) {
						mes .n$;
						mes "I am sorry but you don't seem to have enough Zeny for this.";
						close;
					} else {
						Zeny -= 200;
						close2;
						openstorage;
						end;
					}
				}
			}
			break;
		case 3:
		OnOpenGuildStorage:
			mes .n$;
			if(getcharid(2) > 0) {
				mes "This will cost you ^ff00001.500z^000000";
				mes "Do you want to continue?";
				next;
				if(select("Yes, please.:No, thank you") == 2) {
					close;
				} else {
					if(Zeny < 1500) {
						mes .n$;
						mes "I am sorry, but you don't seem to have enough Zeny.";
						close;
					} else {
						Zeny -= 1500;
						close2;
						guildopenstorage;
						end;
					}
				}
			} else {
				mes "This service is exclusive to those in a guild.";
				close;
			}
			break;
		case 4: // Teleport
			menu	" ~ Towns",Towns,
		    	// " ~ Fields",Fields,
		    	" ~ Dungeons",Dungeons,
		    	" ~ Specials",Specials,
		    	// " ~ Instances",Instances,
				" ~ Territories",Territories,
				" ~ Castles",Castles;
			end;

			// --------------------------------------------------
				Towns:
			// --------------------------------------------------
			menu	"Prontera",T1, 
					"Alberta",T2, 
					"Aldebaran",T3, 
					"Amatsu",T4, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Ayothaya":":"),T5,
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Brasilis":":"),T6, 
					"Comodo",T7, 
					// "Dewata",T8, 
					// "Eclage",T9, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Einbech":":"),T10, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Einbroch":":"),T11, 
					// "El Dicastes",T12, 
					"Geffen",T13, 
					"Gonryun",T14, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Hugel":":"),T15,
					"Izlude",T16, 
					"Jawaii",T17, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Lighthalzen":":"),T18, 
					// "Lighthalzen Slums",T18A, 
					"Louyang",T19, 
					"Lutie",T20,
					// "Malangdo",T21, 
					// "Malaya",T22, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Manuk":":"),T23, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Midgarts Expedition Camp":":"),T24,
					// "Mora",T25, 
					"Morroc",T26, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Moscovia":":"),T27, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Nameless Island":":"),T28,
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Niflheim":":"),T29, 
					"Payon",T30, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Rachel":":"),T31, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Splendide":":"),T32, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Thor Camp":":"),T33,
					"Umbala",T34, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Veins":":"),T35, 
					"Yuno",T36;

			T1: Go("prontera",155,183);
			T2: Go("alberta",28,234);
			T3: Go("aldebaran",140,131);
			T4: Go("amatsu",198,84);
			T5: Go("ayothaya",208,166);
			T6: Go("brasilis",196,217);
			T7: Go("comodo",209,143);
			T8: Restrict("RE");
				Go("dewata",200,180);
			T9: Restrict("RE");
				Go("ecl_in01",48,53);
			T10: Go("einbech",63,35);
			T11: Go("einbroch",64,200);
			T12: Restrict("RE");
				Go("dicastes01",198,187);
			T13: Go("geffen",119,59);
			T14: Go("gonryun",160,120);
			T15: Go("hugel",96,145);
			T16: Go("izlude",128,(checkre(3)?146:114));
			T17: Go("jawaii",251,132);
			T18: Go("lighthalzen",158,92);
			T18A: Go("lighthalzen",307,310);
			T19: Go("louyang",217,100);
			T20: Go("xmas",147,134);
			T21: Restrict("RE");
				Go("malangdo",140,114);
			T22: Restrict("RE");
				Go("malaya",231,200);
			T23: Go("manuk",282,138);
			T24: Go("mid_camp",210,288);
			T25: Restrict("RE");
				Go("mora",55,146);
			T26: Go("morocc",156,93);
			T27: Go("moscovia",223,184);
			T28: Go("nameless_n",256,215);
			T29: Go("niflheim",202,174);
			T30: Go("payon",179,100);
			T31: Go("rachel",130,110);
			T32: Go("splendide",201,147);
			T33: Go("thor_camp",246,68);
			T34: Go("umbala",97,153);
			T35: Go("veins",216,123);
			T36: Go("yuno",157,51);

			// --------------------------------------------------
				Fields:
			// --------------------------------------------------
			menu	"Amatsu Fields",F1, 
					"Ayothaya Fields",F2, 
					"Bifrost Fields", F3,
					"Brasilis Fields",F4, 
					"Comodo Fields",F5, 
					"Dewata Fields",F6,
					"Eclage Fields",F7, 
					"Einbroch Fields",F8, 
					"El Dicastes Fields",F9,
					"Geffen Fields",F10, 
					"Gonryun Fields",F11, 
					"Hugel Fields",F12,
					"Lighthalzen Fields",F13, 
					"Louyang Field",F14, 
					"Lutie Field",F15,
					"Malaya Fields",F16, 
					"Manuk Fields",F17, 
					"Mjolnir Fields",F18,
					"Moscovia Fields",F19, 
					"Niflheim Fields",F20, 
					"Payon Forests",F21,
					"Prontera Fields",F22, 
					"Rachel Fields",F23, 
					"Sograt Deserts",F24,
					"Splendide Fields",F25, 
					"Umbala Fields",F26, 
					"Veins Fields",F27,
					"Yuno Fields",F28;

			F1: setarray @c[2],190,197;
				Disp("Amatsu Field",1,1); Pick("ama_fild");
			F2: setarray @c[2],173,134,212,150;
				Disp("Ayothaya Field",1,2); Pick("ayo_fild");
			F3: Restrict("RE");
				setarray @c[2],193,220,220,187;
				Disp("Bifrost Field",1,2); Pick("bif_fild");
			F4: setarray @c[2],74,32;
				Disp("Brasilis Field",1,1); Pick("bra_fild");
			F5: Restrict("Pre-RE",5);
				setarray @c[2],180,178,231,160,191,172,228,194,224,203,190,223,234,177,194,175,172,172;
				Disp("Comodo Field",1,9); Pick("cmd_fild");
			F6: Restrict("RE");
				setarray @c[2],371,212;
				Disp("Dewata Field",1,1); Pick("dew_fild");
			F7: Restrict("RE");
				setarray @c[2],97,314;
				Disp("Eclage Field",1,1); Pick("ecl_fild");
			F8: Restrict("Pre-RE",2,10);
				setarray @c[2],142,225,182,141,187,228,185,173,216,173,195,148,272,220,173,214,207,174,196,200;
				Disp("Einbroch Field",1,10); Pick("ein_fild");
			F9: Restrict("RE");
				setarray @c[2],143,132,143,217;
				Disp("El Dicastes Field",1,2); Pick("dic_fild");
			F10: Restrict("Pre-RE",13,15);
				setarray @c[0],46,199,213,204,195,212,257,192,188,171,166,263,248,158,195,191,17,217,221,117,178,218,136,328,240,181,235,235,211,185;
				Disp("Geffen Field",0,14); Pick("gef_fild",1);
			F11: setarray @c[2],220,227;
				Disp("Gonryun Field",1,1); Pick("gon_fild");
			F12: Restrict("Pre-RE",3,7);
				setarray @c[2],268,101,222,193,232,185,252,189,196,106,216,220,227,197;
				Disp("Hugel Field",1,7); Pick("hu_fild");
			F13: setarray @c[2],240,179,170,41,240,226;
				Disp("Lighthalzen Field",1,3); Pick("lhz_fild");
			F14: setarray @c[2],229,187;
				Disp("Louyang Field",1,1); Pick("lou_fild");
			F15: setarray @c[2],115,145;
				Disp("Lutie Field",1,1); Pick("xmas_fild");
			F16: Restrict("RE");
				setarray @c[2],40,272,207,180;
				Disp("Malaya Field",1,2); Pick("ma_fild");
			F17: setarray @c[2],35,236,35,262,84,365;
				Disp("Manuk Field",1,3); Pick("man_fild");
			F18: setarray @c[2],204,120,175,193,208,213,179,180,181,240,195,270,235,202,188,215,205,144,245,223,180,206,196,208;
				Disp("Mjolnir Field",1,12); Pick("mjolnir_");
			F19: setarray @c[2],82,104,131,147;
				Disp("Moscovia Field",1,2); Pick("mosk_fild");
			F20: setarray @c[2],215,229,167,234;
				Disp("Niflheim Field",1,2); Pick("nif_fild");
			F21: Restrict("Pre-RE",5,11);
				setarray @c[2],158,206,151,219,205,148,186,247,134,204,193,235,105,352,137,189,201,224,160,205,194,150;
				Disp("Payon Forest",1,11); Pick("pay_fild");
			F22: setarray @c[0],208,227,190,206,240,206,190,143,307,252,212,33,185,188,193,194,187,218,210,183,195,149,198,164;
				Disp("Prontera Field",0,11); Pick("prt_fild",1);
			F23: Restrict("Pre-RE",2,7,9,10,11,13);
				setarray @c[2],192,162,235,166,202,206,202,208,225,202,202,214,263,196,217,201,87,121,277,181,221,185,175,200,174,197;
				Disp("Rachel Field",1,13); Pick("ra_fild");
			F24: setarray @c[2],219,205,177,206,194,182,224,170,198,216,156,187,185,263,206,228,208,238,63,164,85,97,207,202,31,195,38,195;
				Disp("Sograt Desert 1:Sograt Desert 2:Sograt Desert 3:Sograt Desert 7:Sograt Desert 11:Sograt Desert 12:Sograt Desert 13:Sograt Desert 16:Sograt Desert 17:Sograt Desert 18:Sograt Desert 19:Sograt Desert 20:Sograt Desert 21:Sograt Desert 22");
				Pick("","moc_fild01","moc_fild02","moc_fild03","moc_fild07","moc_fild11","moc_fild12","moc_fild13","moc_fild16","moc_fild17","moc_fild18","moc_fild19","moc_fild20","moc_fild21","moc_fild22");
			F25: setarray @c[2],175,186,236,184,188,204;
				Disp("Splendide Field",1,3); Pick("spl_fild");
			F26: setarray @c[2],217,206,223,221,237,215,202,197;
				Disp("Umbala Field",1,4); Pick("um_fild");
			F27: Restrict("Pre-RE",5);
				setarray @c[2],186,175,196,370,222,45,51,250,202,324,150,223,149,307;
				Disp("Veins Field",1,7); Pick("ve_fild");
			F28: Restrict("Pre-RE",5,10);
				setarray @c[2],189,224,192,207,221,157,226,199,223,177,187,232,231,174,196,203,183,214,200,124,358,357,210,304;
				Disp("Yuno Field",1,12); Pick("yuno_fild");

			// --------------------------------------------------
				Dungeons:
			// --------------------------------------------------
			menu	(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Abyss Lakes":":"),D1,
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Amatsu Dungeon":":"),D2, 
					"Anthell",D3,
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Ayothaya Dungeon":":"),D4, 
					"Beach Dungeon",D5, 
					// "Bifrost Tower",D41,
					// "Bio Labs",D6, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Brasilis Dungeon":":"),D7, 
					"Byalan Dungeon",D8, 
					"Clock Tower",D9,
					"Coal Mines",D10, 
					"Culvert",D11, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Cursed Abbey":":"),D12, 
					// "Dewata Dungeon",D13,
					"Einbroch Dungeon",D14, 
					// "Gefenia",D15, 
					"Geffen Dungeon",D16,
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Glast Heim":":"),D17, 
					"Gonryun Dungeon",D18, 
					// "Hidden Dungeon",D19,
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Ice Dungeon":":"),D20, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Juperos":":"),D21, 
					// "Kiel Dungeon",D22, 
					"Louyang Dungeon",D23,
					"Magma Dungeon",D24, 
					// "Malangdo Dungeon",D25, 
					(#AQ_Yesterday==getvariableofnpc(.Day,"adventurer_quest")?"Moscovia Dungeon":":"),D26,
					// "Nidhogg's Dungeon",D27, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Odin Temple":":"),D28, 
					"Orc Dungeon",D29,
					"Payon Dungeon",D30, 
					"Pyramids",D31, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Rachel Sanctuary":":"),D32,
					// "Scaraba Hole",D33, 
					"Sphinx",D34, 
					"Sunken Ship",D35, 
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Thanatos Tower":":"),D36,
					(#AQ_LastWeek==getvariableofnpc(.Week,"adventurer_quest")?"Thor Volcano":":"),D37, 
					"Toy Factory",D38, 
					"Turtle Dungeon",D39, 
					"Umbala Dungeon",D40;
			D1: setarray @c[2],261,272;
				Disp("Abyss Lakes",0); Pick("abyss_");
			D2: setarray @c[2],228,11;
				Disp("Amatsu Dungeon",1); Pick("ama_dun");
			D3: setarray @c[2],35,262;
				Disp("Anthell",1); Pick("anthell");
			D4: setarray @c[2],275,19;
				Disp("Ancient Shrine Maze"); Pick("ayo_dun");
			D5: setarray @c[2],266,67;
				Disp("Beach Dungeon"); Pick("","beach_dun");
			D6: Restrict("RE",4);
				setarray @c[2],150,288;
				Disp("Bio Lab",1); Pick("lhz_dun");
			D7: setarray @c[2],87,47;
				Disp("Brasilis Dungeon",1); Pick("bra_dun");
			D8: Restrict("RE",6);
				setarray @c[0],168,168;
				Disp("Byalan Dungeon",1); Pick("iz_dun",1);
			D9: setarray @c[2],199,159;
				Disp("Clock Tower");
				Pick("","c_tower1");
			D10: setarray @c[2],52,17;
				Disp("Coal Mines",1); Pick("mjo_dun");
			D11: setarray @c[2],131,247;
				Disp("Culvert",1); Pick("","prt_sewb1");
			D12: setarray @c[2],51,14;
				Disp("Cursed Abbey",1); Pick("abbey");
			D13: Restrict("RE");
				setarray @c[2],285,160;
				Disp("Dewata Dungeon",1); Pick("dew_dun");
			D14: setarray @c[2],22,14;
				Disp("Einbroch Dungeon",1); Pick("ein_dun");
			D15: setarray @c[2],40,103;
				Disp("Gefenia",1); Pick("gefenia",0);
			D16: setarray @c[0],104,99;
				Disp("Geffen Dungeon",1); Pick("gef_dun",1);
			D17: setarray @c[2],370,304;
				Disp("Entrance");
				Pick("","glast_01");
			D18: setarray @c[2],153,53;
				Disp("Gonryun Dungeon",1); Pick("gon_dun");
			D19: setarray @c[2],176,7;
				Disp("Hidden Dungeon",1); Pick("prt_maze");
			D20: setarray @c[2],157,14;
				Disp("Ice Dungeon",1); Pick("ice_dun");
			D21: setarray @c[2],140,51;
				Disp("Entrance:Juperos");
				Pick("","jupe_cave");
			D22: setarray @c[2],28,226;
				Disp("Kiel Dungeon",1,2); Pick("kh_dun");
			D23: setarray @c[2],218,196;
				Disp("The Royal Tomb"); Pick("lou_dun");
			D24: setarray @c[2],126,68;
				Disp("Magma Dungeon",1); Pick("mag_dun");
			D25: Restrict("RE");
				setarray @c[2],33,230;
				Disp("Malangdo Dungeon",1); Pick("mal_dun");
			D26: setarray @c[2],189,48;
				Disp("Moscovia Dungeon",1); Pick("mosk_dun");
			D27: setarray @c[2],61,239;
				Disp("Nidhogg's Dungeon",1); Pick("nyd_dun");
			D28: setarray @c[2],298,167;
				Disp("Odin Temple",1); Pick("odin_tem");
			D29: setarray @c[2],32,170;
				Disp("Orc Dungeon",1); Pick("orcsdun");
			D30: setarray @c[0],21,183;
				Disp("Payon Dungeon",1); Pick("pay_dun",1);
			D31: Restrict("RE",7,8);
				setarray @c[2],192,9;
				Disp("Pyramids 1");
				Pick("","moc_pryd01");
			D32: setarray @c[2],140,11;
				Disp("Rachel Sanctuary",1); Pick("ra_san");
			D33: Restrict("RE");
				setarray @c[2],364,44;
				Disp("Scaraba Hole",1); Pick("dic_dun");
			D34: setarray @c[2],288,9;
				Disp("Sphinx",1); Pick("","in_sphinx1");
			D35: setarray @c[2],69,24;
				Disp("Sunken Ship",1); Pick("treasure");
			D36: setarray @c[2],150,39;
				Disp("Thanatos Tower",1); Pick("tha_t");
			D37: setarray @c[2],21,228;
				Disp("Thor Volcano",1); Pick("thor_v");
			D38: setarray @c[2],205,15;
				Disp("Toy Factory",1); Pick("xmas_dun");
			D39: setarray @c[2],154,49;
				Disp("Entrance"); Pick("tur_dun");
			D40: Restrict("Pre-RE",1);
				setarray @c[2],42,31;
				Disp("Carpenter's Shop in the Tree");
				Pick("","um_dun01");
			D41: Restrict("RE");
				setarray @c[2],57,13;
				Disp("Bifrost Tower",1); Pick("ecl_tdun");



			// --------------------------------------------------
				Specials:
			// --------------------------------------------------
			menu	"^f5b041Premium^000000 Room",X1,"Timeless Dungeons",X2;//, "Novice Grounds",X2;

			X1: Restrict("VIP",getcharid(0));
				Go("sala_premmy",40,38);
			X2: Go("yuno",282,288);
		//	X2: Go("new_1-2",99,99);


			// --------------------------------------------------
				Territories:
			// --------------------------------------------------
			menu	"Aldebaran",C1, 
					"Geffen",C2, 
					"Payon",C3,
					"Prontera",C4;

			C1: warp "alde_gld",274,159; end;
			C2: warp "gef_fild13",374,65; end;
			C3: warp "pay_gld",360,148; end;
			C4: warp "prt_gld",160,41; end;


			// --------------------------------------------------
				Castles:
			// --------------------------------------------------
			menu	"Arunafeltz Castles",C5, 
					"Schwaltzvalt Castles",C6;
					// "Classic Gloria",C7,
					// "Classic Aldebaran",C8,
					// "Kindergarten",C9;

			C5: setarray @c[2],200,354;
				Disp("Arunafeltz Castles");
				Pick("","aru_gld");
			C6: setarray @c[2],388,90;
				Disp("Schwaltzvalt Castles");
				Pick("","sch_gld");
			C7:	setarray @c[2],134,65,240,128,153,137,111,240,208,240;
				Disp("Gaebolg:Richard:Wigner:Heine:Nerious");
				Pick("","te_prt_gld","te_prt_gld","te_prt_gld","te_prt_gld","te_prt_gld");
			C8: setarray @c[2],48,83,95,249,142,85,239,242,264,90;
				Disp("Glaris:Defolty:Sorin:Bennit:W");
				Pick("","te_alde_gld","te_alde_gld","te_alde_gld","te_alde_gld","te_alde_gld","te_alde_gld");
			C9: setarray @c[2],102,93+rand(14);
				Disp("Kindergarten");
				Pick("","n_castle");

			// --------------------------------------------------
				Guild_Dungeons:
			// --------------------------------------------------
			menu	"Baldur",G1, "Luina",G2, "Valkyrie",G3, "Britoniah",G4,
					"Arunafeltz",G5, "Schwaltzvalt",G6;

			G1: Restrict("RE",2,3);
				setarray @c[2],119,93,119,93,120,130;
				Disp("Baldur F1:Baldur F2:Hall of Abyss");
				Pick("","gld_dun01","gld_dun01_2","gld2_pay");
			G2: Restrict("RE",2,3);
				setarray @c[2],39,161,39,161,147,155;
				Disp("Luina F1:Luina F2:Hall of Abyss");
				Pick("","gld_dun02","gld_dun02_2","gld2_ald");
			G3: Restrict("RE",2,3);
				setarray @c[2],50,44,50,44,140,132;
				Disp("Valkyrie F1:Valkyrie F2:Hall of Abyss");
				Pick("","gld_dun03","gld_dun03_2","gld2_prt");
			G4: Restrict("RE",2,3);
				setarray @c[2],116,45,116,45,152,118;
				Disp("Britoniah F1:Britoniah F2:Hall of Abyss");
				Pick("","gld_dun04","gld_dun04_2","gld2_gef");
			G5: Go("arug_dun01",199,195);
			G6: Go("schg_dun01",200,124);

			// --------------------------------------------------
				Instances:
			// --------------------------------------------------
			menu	"Bakonawa Lake",I1, "Bangungot Hospital 2F",I2, "Buwaya Cave",I3,
					"Endless Tower",I4, "Hazy Forest",I5, "Malangdo Culvert",I6, "Nidhoggur's Nest",I7,
					"Octopus Cave",I8, "Old Glast Heim",I9, "Orc's Memory",I10, "Sealed Shrine",I11,
					"Wolfchev's Laboratory",I12;

			I1: Restrict("RE");
				Go("ma_scene01",172,175);
			I2: Restrict("RE");
				Go("ma_dun01",151,8);
			I3: Restrict("RE");
				Go("ma_fild02",316,317);
			I4: Go("e_tower",72,112);
			I5: Restrict("RE");
				Go("bif_fild01",161,334);
			I6: Restrict("RE");
				Go("mal_in01",164,21);
			I7: Go("nyd_dun02",95,193);
			I8: Restrict("RE");
				Go("mal_dun01",152,230);
			I9: Restrict("RE");
				Go("glast_01",204,268);
			I10: Go("gef_fild10",240,198);
			I11: Go("monk_test",306,143);
			I12: Restrict("RE");
				Go("lhz_dun04",148,269);

			// --------------------------------------------------
				Special:
			// --------------------------------------------------
			menu	"Auction Hall",S1, "Battlegrounds",S2, "Casino",S3, "Eden Group Headquarters",S4,
					"Gonryun Arena",S5, "Izlude Arena",S6, "Monster Race Arena",S7, "Turbo Track",S8;

			S1: Go("auction_01",22,68);
			S2: Go("bat_room",154,150);
			S3: Go("cmd_in02",179,129);
			S4: Restrict("RE");
				Go("moc_para01",31,14);
			S5: Go("gon_test",48,10);
			S6: Go("arena_room",100,88);
			S7: Go("p_track01",62,41);
			S8: Go("turbo_room",99,114);
		break;
		case 5:
			mes .n$;
			.@hpCost = (vip_status(1)||getgmlevel()>10)?(BaseLevel*5000/99)+(99-BaseLevel*5):5000;
			mes "Full Heal : "+callfunc("F_InsertComma",.@hpCost)+" Zeny.";
			next;
			switch(select("^00AA00Full Heal:^AA0000Cancel^000000")) {
				case 2:
					mes .n$;
					mes "Sure! No problemo! Laters, then!";
					close;
					break;
				case 1:
					if (Zeny < .@hpCost) {
						mes .n$;
						mes "Sorry, darling... You don't have enough zeny";
						mes "but no worries, here... take this...";
						close2;
						getitem 569,1;
						end;
					}
					set Zeny, Zeny-.@hpCost;
					logmes strcharinfo(0) + " healed themself in Sweetie costing them " + .@hpCost;
					specialeffect2 EF_HEAL2; 
					percentheal 100,100;
					break;
			}
			mes .n$;
			mes "Pronto! Have a nice day babe and take care out there!";
			close;
			break;
		case 6: // Rental Services
			if (ismounting()) {
				mes .n$;
				mes "You must first remove your mount.";
				close;
			} else if ((eaclass()&EAJ_THIRDMASK==EAJ_RANGER) && !countitem(6124)) {
				if (!checkfalcon() && getskilllv("HT_FALCON") && !checkoption(Option_Wug) && !checkoption(Option_Wugrider)) {
					mes .n$;
					mes "Please select an option.";
					next;
					if(select(" ~ Falcon: ~ Warg") == 1) setfalcon;
					else getitem 6124,1;
				} else getitem 6124,1;
			} else if ((eaclass()&EAJ_THIRDMASK==EAJ_MECHANIC) && !checkcart() && getskilllv("MC_PUSHCART")) {
				if (!checkmadogear() && getskilllv("NC_MADOLICENCE")) {
					mes .n$;
					mes "Please select an option.";
					next;
					if(select(" ~ Cart: ~ Mado")==1) setcart;
					else setmadogear;
				} else setcart;
			} else if (!checkcart() && getskilllv("MC_PUSHCART")) setcart;
			else if (!checkfalcon() && getskilllv("HT_FALCON") && !checkoption(Option_Wug) && !checkoption(Option_Wugrider)) setfalcon;
			else if (!checkriding() && getskilllv("KN_RIDING")) {
				if (eaclass()&EAJ_THIRDMASK == EAJ_RUNE_KNIGHT) setdragon;
				else setriding;
			} else if (!checkmadogear() && getskilllv("NC_MADOLICENCE")) setmadogear;
			else {
				mes .n$;
				mes "You do not meet requirements to rent.";
				close;
			}

			specialeffect2 EF_TEIHIT3;
			mes .n$;
			mes "Here you go.";
			emotion ET_BEST;
			close;
			break;
		}

OnInit:
// --------------------- Config ---------------------
	set .n$, "^DD00DD[Sweetie]^000000";	// NPC Name
	end;

OnTouch:
	// We offer free identify and free repairs to Premium Players once they meet the daily challenge
	if((vip_status(1) && #AQ_Yesterday == getvariableofnpc(.Day,"adventurer_quest")) || getgmlevel()>60) {
		progressbar "0x0000FF",3;
		specialeffect2 EF_HEAL2;
		percentheal(100,100);
		repairall;
		identifyall;
		specialeffect2 EF_REFINEOK;
	}
	end;


	// ---- Functions ----

	function Go {
		set lastwarp$, getarg(0);
		set lastwarpx, getarg(1,0);
		set lastwarpy, getarg(2,0);
		//Biali Debuff for kindergarten
		if(getarg(0) == "n_castle") 
			callfunc "IsAllowed","Sweetie";
		warp getarg(0),getarg(1,0),getarg(2,0);
		end;
	}
	function Disp {
		if (getargcount() < 3)
			set @menu$, getarg(0);
		else {
			set @menu$,"";
			for (set .@i,getarg(1); .@i<=getarg(2); set .@i,.@i+1)
				set @menu$, @menu$+getarg(0)+" "+.@i+":";
		}
		return;
	}
	function Pick {
		set .@warp_block,@warp_block;
		set @warp_block,0;
		set .@select, select(@menu$);
		if (getarg(0) == "") {
			set .@i, .@select;
			set .@map$, getarg(.@i);
		} else {
			set .@i, .@select-getarg(1,0);
			set .@map$, getarg(0)+((.@i<10)?"0":"")+.@i;
		}
		if (.@warp_block & (1<<.@select)) {
			message strcharinfo(0),"This destination is not available yet.";//"This map is not enabled in "+((checkre(0))?"":"Pre-")+"Renewal.";
			end;
		}
		set .@x, @c[.@i*2];
		set .@y, @c[.@i*2+1];
		deletearray @c[0],getarraysize(@c);
		Go(.@map$,.@x,.@y);
	}

	function Restrict {
		if ((getarg(0) == "RE" && !checkre(0)) || (getarg(0) == "Pre-RE" && checkre(0))) {
			if (getarg(1,0)) {
				set @warp_block,0;
				for (set .@i,1; .@i<getargcount(); set .@i,.@i+1)
					set @warp_block, @warp_block | (1<<getarg(.@i));
			} else {
				message strcharinfo(0),"This destination is not available yet.";//"This map is not enabled in "+((checkre(0))?"":"Pre-")+"Renewal.";
				end;
			}
		}
		if(getarg(0)=="VIP") {
			if(vip_status(1)) 
				return;
			if(getgmlevel()>10) 
				return;
			mes .n$;
			mes "The chosen destination is available only for ^f5b041Premium^000000 players.";
			mes "Do you want to activate ^f5b041Premium^000000 on your account now?";
			if(select("Not now:Yes, please") == 2){
				mes "Right! Please wait a second as I transfer you...";
				next;
				callfunc "F_Premmy";
				end;
			}
			close;
		}
		return;
	}

}


// --------------------------------------------------
//	Duplicates:
// --------------------------------------------------
alb2trea,57,70,6	duplicate(Sweetie)	Sweetie#tre	4_F_MAID,3,3
alberta,28,240,4	duplicate(Sweetie)	Sweetie#alb	4_F_KAFRA6,3,3
aldebaran,145,118,4	duplicate(Sweetie)	Sweetie#alde	4_F_KAFRA5,3,3
amatsu,203,87,4	duplicate(Sweetie)	Sweetie#ama	4_F_KAFRA4,3,3
ayothaya,209,169,6	duplicate(Sweetie)	Sweetie#ayo	4_F_KAFRA3,3,3
comodo,194,158,4	duplicate(Sweetie)	Sweetie#com	4_F_KAFRA1,3,3
einbech,59,38,6	duplicate(Sweetie)	Sweetie#einbe	4_F_KAFRA2,3,3
einbroch,69,202,4	duplicate(Sweetie)	Sweetie#einbr	4_F_KAFRA7,3,3
gef_fild10,71,339,4	duplicate(Sweetie)	Sweetie#orc	4_F_KAFRA8,3,3
geffen,129,65,3	duplicate(Sweetie)	Sweetie#gef	4_F_KAFRA9,3,3
glast_01,372,308,4	duplicate(Sweetie)	Sweetie#glh	4_F_KAFRA1,3,3
gonryun,162,122,4	duplicate(Sweetie)	Sweetie#gon	4_F_KAFRA2,3,3
hugel,101,151,4	duplicate(Sweetie)	Sweetie#hug	4_F_KAFRA2,3,3
izlu2dun,110,92,4	duplicate(Sweetie)	Sweetie#izd	4_F_KAFRA4,3,3
izlude,132,120,4	duplicate(Sweetie)	Sweetie#izl	4_F_KAFRA5,3,3	//Pre-RE: (132,120)
jawaii,253,138,4	duplicate(Sweetie)	Sweetie#jaw	4_F_KAFRA1,3,3
lighthalzen,162,102,4	duplicate(Sweetie)	Sweetie#lhz	4_F_KAFRA2,3,3
louyang,208,103,6	duplicate(Sweetie)	Sweetie#lou	4_F_KAFRA8,3,3
manuk,274,146,6	duplicate(Sweetie)	Sweetie#man	4_F_KAFRA9,3,3
mid_camp,216,288,4	duplicate(Sweetie)	Sweetie#mid	4_F_KAFRA7,3,3
mjolnir_02,85,364,4	duplicate(Sweetie)	Sweetie#mjo	4_F_KAFRA3,3,3
moc_ruins,64,164,4	duplicate(Sweetie)	Sweetie#moc	4_F_KAFRA1,3,3
morocc,159,97,4	duplicate(Sweetie)	Sweetie#mor	4_F_KAFRA2,3,3
moscovia,229,191,4	duplicate(Sweetie)	Sweetie#mos	4_F_KAFRA6,3,3
nameless_n,259,213,4	duplicate(Sweetie)	Sweetie#nam	4_F_KAFRA2,3,3
niflheim,205,179,4	duplicate(Sweetie)	Sweetie#nif	4_F_KAFRA3,3,3
pay_arche,42,134,4	duplicate(Sweetie)	Sweetie#arc	4_F_KAFRA5,3,3
payon,178,104,4	duplicate(Sweetie)	Sweetie#pay	4_F_KAFRA4,3,3
prontera,159,192,4	duplicate(Sweetie)	Sweetie#prt	4_F_KAFRA1,3,3
prontera,150,286,4	duplicate(Sweetie)	Sweetie#prt2	4_F_KAFRA1,3,3
prontera,246,69,4	duplicate(Sweetie)	Sweetie#camp	4_F_KAFRA3,3,3
prt_fild05,279,223,6	duplicate(Sweetie)	Sweetie#cul	4_F_KAFRA2,3,3
rachel,135,116,4	duplicate(Sweetie)	Sweetie#rac	4_F_KAFRA6,3,3
splendide,205,153,4	duplicate(Sweetie)	Sweetie#spl	4_F_KAFRA1,3,3
thor_camp,249,76,4	duplicate(Sweetie)	Sweetie#thor	4_F_KAFRA7,3,3
umbala,106,150,3	duplicate(Sweetie)	Sweetie#umb	4_F_KAFRA4,3,3
veins,214,123,4	duplicate(Sweetie)	Sweetie#ve	4_F_KAFRA2,3,3
xmas,150,136,6	duplicate(Sweetie)	Sweetie#xmas	4_F_KAFRA5,3,3
yuno,162,47,4	duplicate(Sweetie)	Sweetie#yuno	4_F_KAFRA3,3,3
yuno,285,293,4	duplicate(Sweetie)	Sweetie#yuno2	4_F_KAFRA2,3,3
sala_premmy,28,33,0	duplicate(Sweetie)	Sweetie#premmy	4_F_KAFRA8,3,3
//new_1-2,90,112,4	duplicate(Sweetie)	Sweetie#novice	4_F_KAFRA1

// --------------------------------------------------
//	Duplicates (Renewal):
// --------------------------------------------------
brasilis,201,222,4	duplicate(Sweetie)	Sweetie#bra	4_F_KAFRA2
dewata,204,186,6	duplicate(Sweetie)	Sweetie#dew	4_F_KAFRA2
dicastes01,194,194,6	duplicate(Sweetie)	Sweetie#dic	4_F_KAFRA2
ecl_in01,51,60,4	duplicate(Sweetie)	Sweetie#ecl	4_F_KAFRA2
malangdo,134,117,6	duplicate(Sweetie)	Sweetie#mal	4_F_KAFRA2
malaya,231,204,4	duplicate(Sweetie)	Sweetie#ma	4_F_KAFRA2
mora,57,152,4	duplicate(Sweetie)	Sweetie#mora	4_F_KAFRA2

