prt_in,136,115,5	script	Sweetie	4_F_KAFRA2,3,3,{
function Get_Platinum; function Equip_Menu; function Equip_Menu2; function Cm; function Go; function Disp; function Pick; function Restrict;
	if(checkquest(64502) == 1) {
		mes .n$;
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
	mes .n$;
	set .@time, gettime(3);
	mes "Good "+((.@time < 5 || .@time > 17)?"evening":((.@time < 12)?"morning":"afternoon"))+", "+strcharinfo(0)+"!";
	mes "What can I do for you?";
	emotion ET_DELIGHT;
	next;
	disable_items;
	switch(select(.menu$)) {
	case 1:
		menu	"Last Warp ^777777["+lastwarp$+"]^000000",-,
		    	" ~ Towns",Towns,
		    	" ~ Fields",Fields,
		    	" ~ Dungeons",Dungeons,
		    	" ~ Specials",Specials,
		    	" ~ Instances",Instances,
				" ~ Castles",Castles;
			
			if (lastwarp$ == "")
				message strcharinfo(0),"You haven't warped anywhere yet.";
			else {
				if(lastwarp$ == "n_castle")
					callfunc "IsAllowed","Sweetie";
				warp lastwarp$,lastwarpx,lastwarpy;
			}
			end;

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
			return;
		}

		// --------------------------------------------------
			Towns:
		// --------------------------------------------------
		menu	"Prontera",T1, "Alberta",T2, "Aldebaran",T3, "Amatsu",T4, "Ayothaya",T5,
		    	"Brasilis",T6, "Comodo",T7, "Dewata",T8, "Eclage",T9, "Einbech",T10, 
		    	"Einbroch",T11, "El Dicastes",T12, "Geffen",T13, "Gonryun",T14, "Hugel",T15,
		    	"Izlude",T16, "Jawaii",T17, "Lighthalzen",T18, "Lighthalzen Slums",T18A, "Louyang",T19, "Lutie",T20,
		    	"Malangdo",T21, "Malaya",T22, "Manuk",T23, "Midgarts Expedition Camp",T24,
		    	"Mora",T25, "Morroc",T26, "Moscovia",T27, "Nameless Island",T28,
		    	"Niflheim",T29, "Payon",T30, "Rachel",T31, "Splendide",T32, "Thor Camp",T33,
		    	"Umbala",T34, "Veins",T35, "Yuno",T36;

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
		menu	"Amatsu Fields",F1, "Ayothaya Fields",F2, "Bifrost Fields", F3,
		    	"Brasilis Fields",F4, "Comodo Fields",F5, "Dewata Fields",F6,
		    	"Eclage Fields",F7, "Einbroch Fields",F8, "El Dicastes Fields",F9,
		    	"Geffen Fields",F10, "Gonryun Fields",F11, "Hugel Fields",F12,
		    	"Lighthalzen Fields",F13, "Louyang Field",F14, "Lutie Field",F15,
		    	"Malaya Fields",F16, "Manuk Fields",F17, "Mjolnir Fields",F18,
		    	"Moscovia Fields",F19, "Niflheim Fields",F20, "Payon Forests",F21,
		    	"Prontera Fields",F22, "Rachel Fields",F23, "Sograt Deserts",F24,
		    	"Splendide Fields",F25, "Umbala Fields",F26, "Veins Fields",F27,
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
		menu	"Abyss Lakes",D1, "Amatsu Dungeon",D2, "Anthell",D3,
		    	"Ayothaya Dungeon",D4, "Beach Dungeon",D5, "Bifrost Tower",D41,
		    	"Bio Labs",D6, "Brasilis Dungeon",D7, "Byalan Dungeon",D8, "Clock Tower",D9,
		    	"Coal Mines",D10, "Culvert",D11, "Cursed Abbey",D12, "Dewata Dungeon",D13,
		    	"Einbroch Dungeon",D14, "Gefenia",D15, "Geffen Dungeon",D16,
		    	"Glast Heim",D17, "Gonryun Dungeon",D18, "Hidden Dungeon",D19,
		    	"Ice Dungeon",D20, "Juperos",D21, "Kiel Dungeon",D22, "Louyang Dungeon",D23,
		    	"Magma Dungeon",D24, "Malangdo Dungeon",D25, "Moscovia Dungeon",D26,
		    	"Nidhogg's Dungeon",D27, "Odin Temple",D28, "Orc Dungeon",D29,
		    	"Payon Dungeon",D30, "Pyramids",D31, "Rachel Sanctuary",D32,
		    	"Scaraba Hole",D33, "Sphinx",D34, "Sunken Ship",D35, "Thanatos Tower",D36,
		    	"Thor Volcano",D37, "Toy Factory",D38, "Turtle Dungeon",D39, "Umbala Dungeon",D40;

		D1: setarray @c[2],261,272,275,270,116,27;
			Disp("Abyss Lakes",1); Pick("abyss_");
		D2: setarray @c[2],228,11,34,41,119,14;
			Disp("Amatsu Dungeon",1); Pick("ama_dun");
		D3: setarray @c[2],35,262,168,170;
			Disp("Anthell",1); Pick("anthell");
		D4: setarray @c[2],275,19,24,26;
			Disp("Ancient Shrine Maze:Inside Ancient Shrine"); Pick("ayo_dun");
		// D5: setarray @c[2],266,67,255,244,23,260;
		// 	Disp("Beach Dungeon",1); Pick("","beach_dun","beach_dun2","beach_dun3");
		D5: setarray @c[2],266,67,255,244,23,260;
			Disp("Beach Dungeon:Beach Dungeon 2:Beach Dungeon 3"); Pick("","beach_dun","beach_dun2","beach_dun3");
		D6: Restrict("RE",4);
			setarray @c[2],150,288,150,18,140,134,244,52;
			Disp("Bio Lab",1); Pick("lhz_dun");
		D7: setarray @c[2],87,47,262,262;
			Disp("Brasilis Dungeon",1); Pick("bra_dun");
		D8: Restrict("RE",6);
			setarray @c[0],168,168,253,252,236,204,32,63,26,27,141,187;
			Disp("Byalan Dungeon",1); Pick("iz_dun",1);
		D9: setarray @c[2],199,159,148,283,65,147,56,155,297,25,127,169,277,178,268,74;
			Disp("Clock Tower 1:Clock Tower 2:Clock Tower 3:Clock Tower 4:Basement 1:Basement 2:Basement 3:Basement 4");
			Pick("","c_tower1","c_tower2","c_tower3","c_tower4","alde_dun01","alde_dun02","alde_dun03","alde_dun04");
		D10: setarray @c[2],52,17,381,343,302,262;
			 Disp("Coal Mines",1); Pick("mjo_dun");
		D11: setarray @c[2],131,247,19,19,180,169,100,92;
			 Disp("Culvert",1); Pick("","prt_sewb1","prt_sewb2","prt_sewb3","prt_sewb4");
		D12: setarray @c[2],51,14,150,11,120,10;
			 Disp("Cursed Abbey",1); Pick("abbey");
		D13: Restrict("RE");
			 setarray @c[2],285,160,299,29;
			 Disp("Dewata Dungeon",1); Pick("dew_dun");
		D14: setarray @c[2],22,14,292,290;
			 Disp("Einbroch Dungeon",1); Pick("ein_dun");
		D15: setarray @c[2],40,103,203,34,266,168,130,272;
			 Disp("Gefenia",1); Pick("gefenia",0);
		D16: setarray @c[0],104,99,115,236,106,132,203,200;
			 Disp("Geffen Dungeon",1); Pick("gef_dun",1);
		D17: setarray @c[2],370,304,199,29,104,25,150,15,157,287,147,15,258,255,108,291,171,283,68,277,156,7,12,7,133,271,224,274,14,70,150,14;
			 Disp("Entrance:Castle 1:Castle 2:Chivalry 1:Chivalry 2:Churchyard:Culvert 1:Culvert 2:Culvert 3:Culvert 4:St. Abbey:Staircase Dungeon:Underground Cave 1:Underground Cave 2:Underground Prison 1:Underground Prison 2");
			 Pick("","glast_01","gl_cas01","gl_cas02","gl_knt01","gl_knt02","gl_chyard","gl_sew01","gl_sew02","gl_sew03","gl_sew04","gl_church","gl_step","gl_dun01","gl_dun02","gl_prison","gl_prison1");
		D18: setarray @c[2],153,53,28,113,68,16;
			 Disp("Gonryun Dungeon",1); Pick("gon_dun");
		D19: setarray @c[2],176,7,93,20,23,8;
			 Disp("Hidden Dungeon",1); Pick("prt_maze");
		D20: setarray @c[2],157,14,151,155,149,22,33,158;
			 Disp("Ice Dungeon",1); Pick("ice_dun");
		D21: setarray @c[2],140,51,53,247,37,63,150,285;
			 Disp("Entrance:Juperos");
			 Pick("","jupe_cave","juperos_01","juperos_02","jupe_core");
		D22: setarray @c[2],28,226,41,198;
			 Disp("Kiel Dungeon",1,2); Pick("kh_dun");
		D23: setarray @c[2],218,196,282,20,165,38;
			 Disp("The Royal Tomb"); Pick("lou_dun");
		D24: setarray @c[2],126,68,47,30;
			 Disp("Magma Dungeon",1); Pick("mag_dun");
		D25: Restrict("RE");
			 setarray @c[2],33,230;
			 Disp("Malangdo Dungeon",1); Pick("mal_dun");
		D26: setarray @c[2],189,48,165,30,32,135;
			 Disp("Moscovia Dungeon",1); Pick("mosk_dun");
		D27: setarray @c[2],61,239,60,271;
			 Disp("Nidhogg's Dungeon",1); Pick("nyd_dun");
		D28: setarray @c[2],298,167,224,149,266,280;
			 Disp("Odin Temple",1); Pick("odin_tem");
		D29: setarray @c[2],32,170,21,185;
			 Disp("Orc Dungeon",1); Pick("orcsdun");
		D30: setarray @c[0],21,183,19,33,19,63,155,159,201,204;
			 Disp("Payon Dungeon",1); Pick("pay_dun",1);
		D31: Restrict("RE",7,8);
			 setarray @c[2],192,9,10,192,100,92,181,11,94,96,192,8,94,96,192,8;
			 Disp("Pyramids 1");
			 Pick("","moc_pryd01","moc_pryd02","moc_pryd03","moc_pryd04","moc_pryd05","moc_pryd06","moc_prydn1","moc_prydn2");
		D32: setarray @c[2],140,11,32,21,8,149,204,218,150,9;
			 Disp("Rachel Sanctuary",1); Pick("ra_san");
		D33: Restrict("RE");
			 setarray @c[2],364,44,101,141;
			 Disp("Scaraba Hole",1); Pick("dic_dun");
		D34: setarray @c[2],288,9,149,81,210,54,10,222,100,99;
			 Disp("Sphinx",1); Pick("","in_sphinx1","in_sphinx2","in_sphinx3","in_sphinx4","in_sphinx5");
		D35: setarray @c[2],69,24,102,27;
			 Disp("Sunken Ship",1); Pick("treasure");
		D36: setarray @c[2],150,39,150,136,220,158,59,143,62,11,89,221,35,166,93,148,29,107,159,138,19,20,130,52;
			 Disp("Thanatos Tower",1); Pick("tha_t");
		D37: setarray @c[2],21,228,75,205,34,272;
			 Disp("Thor Volcano",1); Pick("thor_v");
		D38: setarray @c[2],205,15,129,133;
			 Disp("Toy Factory",1); Pick("xmas_dun");
		D39: setarray @c[2],154,49,148,261,132,189,100,192;
			 Disp("Entrance"); Pick("tur_dun");
		D40: Restrict("Pre-RE",1);
			 setarray @c[2],42,31,48,30,204,78;
			 Disp("Carpenter's Shop in the Tree");
			 Pick("","um_dun01","um_dun02","yggdrasil01");
		D41: Restrict("RE");
			 setarray @c[2],57,13,64,88,45,14,26,23;
			 Disp("Bifrost Tower",1); Pick("ecl_tdun");



		// --------------------------------------------------
			Specials:
		// --------------------------------------------------
		menu	"MvP Instances",X1;//, "Novice Grounds",X2;

		X1: Go("yuno",282,288);
	//	X2: Go("new_1-2",99,99);


		// --------------------------------------------------
			Castles:
		// --------------------------------------------------
		menu	"Aldebaran Castles",C1, "Geffen Castles",C2, "Payon Castles",C3,
		    	"Prontera Castles",C4, "Arunafeltz Castles",C5, "Schwaltzvalt Castles",C6,
				"Classic Gloria",C7,"Classic Aldebaran",C8,"Kindergarten",C9;

		C1: setarray @c[2],48,83,95,249,142,85,239,242,264,90;
			Disp("Neuschwanstein:Hohenschwangau:Nuenberg:Wuerzburg:Rothenburg");
			Pick("","alde_gld","alde_gld","alde_gld","alde_gld","alde_gld");
		C2: setarray @c[2],214,75,308,240,143,240,193,278,305,87;
			Disp("Repherion:Eeyolbriggar:Yesnelph:Bergel:Mersetzdeitz");
			Pick("","gef_fild13","gef_fild13","gef_fild13","gef_fild13","gef_fild13");
		
		C3: setarray @c[2],121,233,295,116,317,293,140,160,204,266;
			Disp("Bright Arbor:Scarlet Palace:Holy Shadow:Sacred Altar:Bamboo Grove Hill");
			Pick("","pay_gld","pay_gld","pay_gld","pay_gld","pay_gld");

		C4: setarray @c[2],134,65,240,128,153,137,111,240,208,240;
			Disp("Kriemhild:Swanhild:Fadhgridh:Skoegul:Gondul");
			Pick("","prt_gld","prt_gld","prt_gld","prt_gld","prt_gld");
		C5: setarray @c[2],158,272,83,47,68,155,299,345,292,107;
			Disp("Mardol:Cyr:Horn:Gefn:Banadis");
			Pick("","aru_gld","aru_gld","aru_gld","aru_gld","aru_gld");
		C6: setarray @c[2],293,100,288,252,97,196,137,90,71,315;
			Disp("Himinn:Andlangr:Viblainn:Hljod:Skidbladnir");
			Pick("","sch_gld","sch_gld","sch_gld","sch_gld","sch_gld");
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
	
	// REFINE
	case 2:
		function Refine_Item;
		mes .n$;
		mes "Select an option.";
		next;
		switch(select("~ Normal Refiner:~ Enriched Refiner:~ HD Refine:~ Purify Ore")) {
			case 2:
				mes .n$;
				mes "What do you want to refine?";
				next;
				Equip_Menu(1);
				setarray .@Materials[0],7619,7620,7620,7620,7620;
				setarray .@Safe[0],4,7,6,5,4;
				set .@WLv, getequipweaponlv(@i);
				mes .n$;
				mes "Item: +"+getequiprefinerycnt(@i)+" "+getequipname(@i);
				next;
				if(select(" ~ Refine item: ~ ^777777Cancel^000000") == 2)
					close;
				Refine_Item(1, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv], 1);
				close;
				break;
			case 4:
				callfunc "massPurify","Sweetie";
				end;
	 			break;
			case 1:
				mes .n$;
				mes "What do you want to refine?";
				next;
				Equip_Menu(1);
				setarray .@Materials[0],985,1010,1011,984,984;
				setarray .@Safe[0],4,7,6,5,4;
				set .@WLv, getequipweaponlv(@i);
				set .@SafeCount, .@Safe[.@WLv]-getequiprefinerycnt(@i);
				mes .n$;
				mes "Item: +"+getequiprefinerycnt(@i)+" "+getequipname(@i);
				next;
				switch(select(" ~ Refine once (^0055FF"+getequippercentrefinery(@i)+"^000000% success): ~ Refine multiple times (up to "+(10-getequiprefinerycnt(@i))+"): ~ Refine to safe limit ("+((.@SafeCount>0)?.@SafeCount+" refines":"^777777disabled^000000")+"): ~ ^777777Cancel^000000")) {
					case 1:
						Refine_Item(1, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv]);
						close;
					case 2:
						input .@Refines;
						if (.@Refines+getequiprefinerycnt(@i) > 10 || .@Refines < 1) {
							mes .n$;
							mes "Invalid number ("+.@Refines+").";
						} else
							Refine_Item(.@Refines, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv]);
						close;
					case 3:
						if (.@SafeCount < 1) {
							mes .n$;
							mes "The item has already reached the safe limit.";
						} else
							Refine_Item(.@SafeCount, .@Materials[.@WLv], .RP[.@WLv], .@Safe[.@WLv]);
						close;
					case 4:
						close;
				}
			case 3:
				mes "I am afraid I dont offer this service yet...";
				close;
				//callfunc "hdRefine","Sweetie",0;
				break;
		}
	break;
		
	// KAFRA SERVICES AND PERSONAL STORAGE
	case 3:
		switch(select("Save Respawn Point:Open Storage:Open Personal Storage:Open Guild Storage:Rentals") ) {
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
			close2;
			openstorage;
			end;
			break;
		case 3:
			close2;
			callfunc "F_CharStorage",getcharid(0);
			end;
			break;
		case 4:
			close2;
			guildopenstorage;
			end;
			break;
		case 5:
			mes .n$;
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

	// RESET STATS OR SKILLS
	case 9:
		mes .n$;
		if(!vip_status(1)) {
			mes "It costs ^0000FF" + .RESET_STATSKILL + "x " + getitemname(.ITEM) +"^000000 to reset either your stats or skills.";
		} else {
			mes "I see you are ^0000FFPremmy^000000";
			mes "It is half price for you, just ^0000FF" + (.RESET_STATSKILL / .VIP_DISCOUNT) + "x " + getitemname(.ITEM) +"^000000 to reset either your stats or skills.";
		}

		mes "You can also use ^dd00001x " + getitemname(6320) + " ^000000to reset for free either stats or skills.";

		mes "Please select an option.";
		next;
		set .@i, select(" ~ Stat reset: ~ Skill reset: ~ Cancel");
		if (.@i == 3)
			close;
		mes .n$;

		if(countitem(6320) > 0) {
			mes (.@i==1?"Do you want to use one of yours ^ff0000" + getitemname(6320) + "^000000 to reset your Stats?":"Do you want to use one of yours ^ff0000" + getitemname(6320) + "^000000 to reset your Skills?");
			if(select("No, I will use my Hunting Coins:Yes, please") == 2) {
				delitem 6320,1;
				logmes (.@i==1?"Consumed 1x " + getitemname(6320) + " to reset Stats.":"Consumed 1x " + getitemname(6320) + " to reset Skills.");
				goto L_Reset;
			}
		}

		.@pass = 0;
		if (vip_status(1)) {
			.@count = .RESET_STATSKILL / .VIP_DISCOUNT;
			if(countitem(.ITEM) >= .@count) 
				.@pass = 1;
		} else {
			.@count = .RESET_STATSKILL;
			if(countitem(.ITEM) >= .@count)
				.@pass = 1;
		}

		if ( .@pass == 1) {
			delitem .ITEM,.@count;
			logmes "Sweetie Reset Stats/Skill took " + .@count + "x " + .ITEM;
		} else {
			mes "You don't have enough ^FF0000" + getitemname(.ITEM) +"^000000.";
			close;
		}

		L_Reset:
		if (.@i == 1) {
			resetstatus;
			next;
			mes .n$;
			mes "Stats reset!";
		} else {
			resetskill;
			next;
			mes .n$;
			mes "Skills reset!";
		}
		
		mes " ";
		mes "Done! Thank you very much!";
		close;


	// PLATINUM SKILLS
	case 5:
		mes .n$;
		if(!vip_status(1)) {
			mes "It costs ^0000FF" + .PLATINUM_SKILLS_COST + "x " + getitemname(.ITEM) +"^000000 to get all your Platinum Skills.";
		} else {
			mes "I see you are ^0000FFPremmy^000000. This service is free for you, darling.";
		}
		next;
		mes .n$;
		if (countitem(.ITEM) < .PLATINUM_SKILLS_COST && !vip_status(1)) {
			mes "You don't have enough ^FF0000" + getitemname(.ITEM) +"^000000.";
			close;
		} else if (countitem(.ITEM) >= .PLATINUM_SKILLS_COST || vip_status(1)) {
			if ( !vip_status(1) ) 
				delitem .ITEM,.PLATINUM_SKILLS_COST;
			Get_Platinum;
			specialeffect2 EF_BLUECASTING;
			mes .n$;
			mes "Platinum skills received!";
			close;
		}
		
		function Get_Platinum {
			skill 142,1,0;
			switch(BaseClass) {
				case 0: if (Class !=23) skill 143,1,0; break;
				case 1: skill 144,1,0; skill 145,1,0; skill 146,1,0; break;
				case 2: skill 157,1,0; break;
				case 3: skill 147,1,0; skill 148,1,0; break;
				case 4: skill 156,1,0; break;
				case 5: skill 153,1,0; skill 154,1,0; skill 155,1,0; break;
				case 6: skill 149,1,0; skill 150,1,0; skill 151,1,0; skill 152,1,0; break;
				default: break;
			}
			switch(BaseJob) {
				case 7: skill 1001,1,0; break;
				case 8: skill 1014,1,0; break;
				case 9: skill 1006,1,0; break;
				case 10: skill 1012,1,0; skill 1013,1,0; break;
				case 11: skill 1009,1,0; break;
				case 12: skill 1003,1,0; skill 1004,1,0; break;
				case 14: skill 1002,1,0; break;
				case 15: skill 1015,1,0; skill 1016,1,0; break;
				case 16: skill 1007,1,0; skill 1008,1,0; skill 1017,1,0; skill 1018,1,0; skill 1019,1,0; break;
				case 17: skill 1005,1,0; break;
				case 18: skill 238,1,0; break;
				case 19: skill 1010,1,0; break;
				case 20: skill 1011,1,0; break;
				default: break;
			}
			return;
		}

	// IDENTIFY ITEMS
	case 6:
		setarray .@Total[0],0,0;
		getinventorylist;
		mes .n$;

		if(!vip_status(1)) {
			mes "It costs ^0000FF" + .IDENTIFY_ALL_COST + "x " + getitemname(.ITEM) +"^000000 to identify all the items in your inventory at once.";
		} else {
			mes "I see you are ^0000FFPremmy^000000. This service is free for you, darling.";
		}
		next;
		mes .n$;

		if (countitem(.ITEM) < .IDENTIFY_ALL_COST && !vip_status(1)) {
			mes "You don't have enough ^FF0000" + getitemname(.ITEM) +"^000000.";
			close;
		}

		if ( !vip_status(1) ) 
			delitem .ITEM,.IDENTIFY_ALL_COST;

		for(set .@i,0; .@i<@inventorylist_count; set .@i,.@i+1) {
			if (@inventorylist_identify[.@i]) 
				continue;
			delitem2 @inventorylist_id[.@i],1,0,0,0,0,0,0,0;
			getitem @inventorylist_id[.@i],1;
			setarray .@Total[0], .@Total[0]+1, .@Total[1]+.Prices[3];
		}

		if (!.@Total[0]) {
			mes "No items identified.";
			close;
		}
		specialeffect2 EF_REFINEOK;
		mes "Identified "+.@Total[0]+" items"+((.@Total[1])?" for "+.IDENTIFY_ALL_COST+"x " + getitemname(.ITEM):"")+".";
		close;

	// REMOVE CARDS 
	case 7:
		mes .n$;
		if(!vip_status(1)) {
			mes "It costs ^0000FF" + .REMOVE_CARDS_COST + "x " + getitemname(.ITEM) +"^000000 to remove cards.";
			mes "Please select an equipment.";
		} else {
			mes "I see you are ^0000FFPremmy^000000.";
			mes "It is half price for you, just ^0000FF" + .REMOVE_CARDS_COST / .VIP_DISCOUNT + "x " + getitemname(.ITEM) +"^000000 to remove cards.";
			mes "Please select an equipment.";
		}
		next;

		setarray .@indices[1], EQI_HEAD_TOP, EQI_ARMOR, EQI_HAND_L, EQI_HAND_R, EQI_GARMENT, EQI_SHOES, EQI_ACC_L, EQI_ACC_R, EQI_HEAD_MID, EQI_HEAD_LOW;
		for( set .@i,1; .@i <= 10; set .@i,.@i+1 ) {
			if( getequipisequiped(.@indices[.@i]) )
				set .@menu$, .@menu$ + F_getpositionname(.@indices[.@i]) + "-[" + getequipname(.@indices[.@i]) + "]";
			set .@menu$, .@menu$ + ":";
		}
		set .@part, .@indices[ select(.@menu$) ];
		if(!getequipisequiped(.@part)) {
			mes .n$;
			mes "Young one... Your not wearing anything there that I can remove cards from.";
			close;
		}
		if(getequipcardcnt(.@part) == 0) {
			mes .n$;
			mes "Young one... There are no cards compounded on this item. I can do nothing with it, I'm afraid.";
			close;
		}
		set .@cardcount,getequipcardcnt(.@part);
		
		mes .n$;
		mes "This item has " + .@cardcount + " cards compounded on it. Should we continue?";
		next;
		if(select("Very well. Do it.:Never mind.") == 2) {
			mes .n$;
			mes "Very well. Return at once if you seek my services.";
			close;
		}
		if ((countitem(.ITEM) < .REMOVE_CARDS_COST && !vip_status(1)) || (countitem(.ITEM) < (.REMOVE_CARDS_COST / .VIP_DISCOUNT) && vip_status(1))) {
			mes "You don't have enough ^FF0000" + getitemname(.ITEM) +"^000000.";
			close;
		}
		if(vip_status(1))
			delitem .ITEM, (.REMOVE_CARDS_COST / .VIP_DISCOUNT);
		else
			delitem .ITEM,.REMOVE_CARDS_COST;
		dispbottom ".: "+ getitemname(.ITEM) +" consumed :.";
		
		successremovecards .@part;
		mes .n$;
		mes "The process was a success. Here are your cards and your item. Farewell.";
		close;
	

	// ENDOW SERVICES
	case 8:
		mes .n$;
		mes "Greetings! How would you like to be endowed?";
		if(@End_Ele) .@Last_Ele$ = "Last Element - "+.Element$[@End_Ele]+":"; else .@Last_Ele$ = ":";
		select(.@Last_Ele$+.Element$[1]+":"+.Element$[2]+":"+.Element$[3]+":"+.Element$[4]+":Converters");
		if(@menu == 6) {callshop "conv_shop",1; end;}
		if(@menu != 1) .@Choice = @menu-1; else .@Choice = @End_Ele;
		if(!vip_status(1)) 
			.@Req_Item$ = " and a ^0055FF" + getitemname(.StoneReq[.@Choice]) + "^000000";
		clear;
		mes .n$;
		mes "So you would like a ^0055FF" + .Element$[.@Choice] + "^000000 endowment? You will require ^0055FF" + .Price + "^000000 zeny" + .@Req_Item$ + ".","Do you accept?";
		if(select("Yes:No") == 1) {
			if((vip_status(1) && Zeny < .Price) || (!vip_status(1) && countitem(.StoneReq[.@Choice]) == 0 || Zeny < .Price)) {
				clear;
				mes .n$;
				mes "You do not have the required money or items. Please come back when you do.";
				close;
			}
			@End_Ele = .@Choice;
			sc_start .EndowSC[@End_Ele],.Ticks,0;
			Zeny = Zeny - .Price;
			if(!vip_status(1))
				delitem .StoneReq[.@Choice],1;

			clear;
			mes .n$;
			mes "Thank you for your custom!";
			close;
		} else {
			clear;
			mes .n$;
			mes "Too bad! Let me endow you next time.";
			close;
		}
	break;

	// HEALING SERVICES
	 case 4:
	 	mes .n$;
	 	if( BaseLevel < 200 ) {
			mes "Sorry darling, this service is exclusive to players level 200 or more.";
			close;
		 }

		if (SW_LASTHEAL > gettimetick(2)) {
			mes "Sorry darling, but you are still in the cooldown. Please come back later.";
			close;
		}
		if(vip_status(1)){
			mes "Healing HP : "+callfunc("F_InsertComma",0)+" Zeny.";
			mes "Healing HP/SP : "+callfunc("F_InsertComma",10000)+" Zeny.";
		} else {
			mes "Healing HP : "+callfunc("F_InsertComma",5000)+" Zeny.";
			mes "Healing HP/SP : "+callfunc("F_InsertComma",20000)+" Zeny.";
		}

		next;
		switch(select("^00AA00Heal HP Only:^00AAFFHeal HP and SP:^AA0000Cancel^000000")) {
			case 3:
				mes .n$;
				mes "Sure! No problemo! Laters, then!";
				close;
				break;
			case 1:
				if(vip_status(1))
					.@Price = 0;
				else 
					.@Price = 5000;
				if (Zeny < .@Price) {
					mes .n$;
					mes "Sorry, darling... You don't have enough zeny";
					mes "but no worries, here... take this...";
					close2;
					getitem 569,1;
					end;
				}
				set Zeny, Zeny-.@Price;
				//logmes strcharinfo(0) + " healed themself in Sweetie costing them " + .@Price;

				specialeffect2 EF_HEAL2; 
				percentheal 100,0;
				break;
			case 2:
				if(vip_status(1))
					.@Price = 10000;
				else 
					.@Price = 20000;
				if (Zeny < .@Price) {
					mes .n$;
					mes "Sorry, darling... You don't have enough zeny";
					mes "but no worries, here... take this...";
					close2;
					getitem 569,1;
					end;
				}
				set Zeny, Zeny-.@Price;
				//logmes strcharinfo(0) + " healed themself in Sweetie costing them " + .@Price;

				specialeffect2 EF_HEAL2; 
				percentheal 100,100;
				break;
		}
		if (vip_status(1)) 
			set SW_LASTHEAL, gettimetick(2)+30;
		else
			set SW_LASTHEAL, gettimetick(2)+60;
		mes .n$;
		mes "Pronto! Have a nice day babe and take care out there!";
		close;

	}
OnInit:

// --------------------- Config ---------------------
	setarray .MainMenu$[0],
		"Teleport",							// [1]
		"Refiner",							// [2]
		"Kafra Services",					// [4]
		"Healing Services",					// [8]
		"Platinum Skills",					// [16]
		"Identify All",						// [32]
		"Remove Cards",						// [64]
		"Endow Services",					// [128]
		"Reset Stats/Skills";				// [256]


	set .@MenuOption,1|2|4|8|16|32|64|128|256;

//	setarray .Coin[0],29200,1000000;	// Coin item ID, coin price <- Biali :: WTF? Usei pra converter por manias
	setarray .Prices[0],3000000,10000,0,100;	// Zeny to remove cards [0], reset [1], rent [2], identify [3]

	set .EnrichedRefine,1;					// Allow enriched refiner option? (1: yes / 0: no)
	setarray .RP[0],2000,50,200,5000,20000;	// Zeny for armor, lv1, lv2, lv3, lv4 refines

	setarray .SexChangeF[0],2213,1,1558,1,7152,50;	// Items to turn into Female (ID,Count)
	setarray .SexChangeM[0],5016,1,2241,1,1017,50;	// Items to turn into Male (ID,Count)

	set .n$, "^DD00DD[Sweetie]^000000";	// NPC Name

	// Cost in Silver Coins
	set .ITEM, 675;
	set .VIP_DISCOUNT, 2; // dividend which cost will be devided by to get the final price for vips.
	set .REMOVE_CARDS_COST, 5;
	set .IDENTIFY_ALL_COST, 1;
	set .PLATINUM_SKILLS_COST, 10;
	set .RESET_STATSKILL, 30;
	set .UNIVERSAL_RENT, 1;

	// Endow
	.Price = 20000;			// Price for endowment.
	.Ticks = 1800000;		// Time in ticks for duration of buff.
	setarray .Element$[1],"Water","Wind","Fire","Earth";
	setarray .EndowSC[1],SC_WATERWEAPON,SC_WINDWEAPON,SC_FIREWEAPON,SC_EARTHWEAPON;
	setarray .StoneReq[1],991,992,990,993;


// --------------------------------------------------
	set .menu$,"";
	for (set .@i,0; .@i < getarraysize(.MainMenu$); set .@i,.@i+1) {
		if (.@MenuOption & (1<<.@i))
			set .menu$, .menu$+" ~ "+.MainMenu$[.@i];
		set .menu$, .menu$+":";
	}
	end;
	
OnTouch:
	if ( BaseLevel < 200) {
		percentheal 100,100;
	}
	if ( vip_status(1) ) {
		skilleffect 34,0; sc_start SC_BLESSING,600000,10; // 10 minutos
		skilleffect 29,0; sc_start SC_INCREASEAGI,600000,10;
		repairall;
	} else {
		progressbar "0x0000FF",1;
		skilleffect 34,0; sc_start SC_BLESSING,300000,10; // 5 minutos
		skilleffect 29,0; sc_start SC_INCREASEAGI,300000,10;
	}
	end;

	function Equip_Menu {
		setarray .@pos$[1],"Left Accessory","Footgear","Garment","Lower Headgear","Mid Headgear","Upper Headgear","Armor","Left hand","Right hand","Right Accessory";
		set .@menu$,"^0000AACancel^000000:";
		for (set .@i,2; .@i < 10; set .@i,.@i+1) {
			if (getequipisequiped(.@i))
				set .@menu$, .@menu$+.@pos$[.@i]+" [^0055FF"+getequipname(.@i)+"^000000]";
			set .@menu$, .@menu$+":";
		}
		set @i, select(.@menu$);
		if(.@i == 1) 
			close;
		if (!getequipisequiped(@i)) {
			mes .n$;
			mes "Nothing is equipped there!";
			close;
		}
		if (getarg(0)) {
			if (!getequipisenableref(@i)) {
				mes .n$;
				mes getequipname(@i)+" cannot be refined.";
				close;
			}
			if (getequiprefinerycnt(@i) >= 10 ) {
				mes .n$;
				mes "+"+getequiprefinerycnt(@i)+" "+getequipname(@i)+" cannot be refined any further.";
				close;
			}
		}
		return;
	}

	function Equip_Menu2 {
		setarray .@pos$[1],"Right Accessory","Footgear","Garment","Lower Headgear","Mid Headgear","Upper Headgear","Armor","Left Hand","Right hand","Left Accessory";
		set .@menu$,"";
		for (set .@i,1; .@i <= 10; set .@i,.@i+1) {
			if (getequipisequiped(.@i))
				set .@menu$, .@menu$+.@pos$[.@i]+" [^0055FF"+getequipname(.@i)+"^000000]";
			set .@menu$, .@menu$+":";
		}
		set @i, select(.@menu$);
		if (!getequipisequiped(@i)) {
			mes .n$;
			mes "Nothing is equipped there!";
			close;
		}
		if (getarg(0)) {
			if (!getequipisenableref(@i)) {
				mes .n$;
				mes getequipname(@i)+" cannot be refined.";
				close;
			}
			if (getequiprefinerycnt(@i) >= 10 ) {
				mes .n$;
				mes "+"+getequiprefinerycnt(@i)+" "+getequipname(@i)+" cannot be refined any further.";
				close;
			}
		}
		return;
	}

	function Refine_Item {
		mes .n$;
		set .@price, getarg(0)*getarg(2);
		mes "I'll need "+getarg(0)+"x "+getitemname(getarg(1))+" and "+Cm(.@price)+" Zeny.";
		if (countitem(getarg(1)) < getarg(0) || Zeny < .@price) {
			mes "Come back when you have the required materials.";
			close;
		}
		if (getequiprefinerycnt(@i)+getarg(0) > getarg(3))
			mes "The item will be refined above the safe limit. It may break.";
		next;
		if(select(" ~ ^0055FFContinue^000000: ~ ^777777Cancel^000000") == 2)
			close;
		mes .n$;
		set .@j, getarg(0);
		while(.@j) {
			delitem getarg(1),1;
			set Zeny, Zeny-getarg(2);
			if (getarg(4,0)) {
				if (getequippercentrefinery(@i) <= rand(100) && getequippercentrefinery(@i) <= rand(100)) {
					mes getequipname(@i)+" broke while refining.";
					failedrefitem @i;
					close;
				}
			} else {
				if (getequippercentrefinery(@i) <= rand(100)) {
					mes getequipname(@i)+" broke while refining.";
					failedrefitem @i;
					close;
				}
			}
			successrefitem @i;
			set .@j, .@j-1;
		}
		mes "All done!";
		close;
	}

	function Cm {
		set .@str$, getarg(0);
		for(set .@i,getstrlen(.@str$)-3; .@i>0; set .@i,.@i-3)
			set .@str$, insertchar(.@str$,",",.@i);
		return .@str$;
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
izlude,134,150,4	duplicate(Sweetie)	Sweetie#izl	4_F_KAFRA5,3,3	//Pre-RE: (132,120)
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
sala_premmy,40,60,4	duplicate(Sweetie)	Sweetie#premmy	4_F_KAFRA8,3,3
//new_1-2,90,112,4	duplicate(Sweetie)	Sweetie#novice	4_F_KAFRA1,3,3

// --------------------------------------------------
//	Duplicates (Renewal):
// --------------------------------------------------
brasilis,201,222,4	duplicate(Sweetie)	Sweetie#bra	4_F_KAFRA2,3,3
dewata,204,186,6	duplicate(Sweetie)	Sweetie#dew	4_F_KAFRA2,3,3
dicastes01,194,194,6	duplicate(Sweetie)	Sweetie#dic	4_F_KAFRA2,3,3
ecl_in01,51,60,4	duplicate(Sweetie)	Sweetie#ecl	4_F_KAFRA2,3,3
malangdo,134,117,6	duplicate(Sweetie)	Sweetie#mal	4_F_KAFRA2,3,3
malaya,231,204,4	duplicate(Sweetie)	Sweetie#ma	4_F_KAFRA2,3,3
mora,57,152,4	duplicate(Sweetie)	Sweetie#mora	4_F_KAFRA2,3,3


// Endow
-	shop	conv_shop	-1,12114:10000,12115:10000,12116:10000,12117:10000