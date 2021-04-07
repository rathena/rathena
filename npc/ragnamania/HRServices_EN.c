// Reset Skills
-	shop	ChronosShop	-1,14527:4500,14581:4500,14582:3500,14583:3500,14584:3500,14585:3500,12211:5500,8906:2000000

-	script	Chronos Girl	-1,{
	// Garbage cleaning
	if( ADVJOB && Class >= Job_Lord_Knight && Class <= Job_Paladin2 )
		set ADVJOB, 0;
	// ****************

	mes "[^FFA500Chronos Girl^000000]";
	mes "Hello, welcome to Chronos RO, ^0000FF" + strcharinfo(0) + "^000000.";
	mes "I can offer you my services?";
	next;
	set @Cost, (BaseLevel / 10) * 200;

	switch( select("^0000FFSpecial Shop^000000","^FF0000Restore HP/SP^000000 (" + @Cost + " zeny)","^2E8B57Save^000000","^FFA500Teleport^000000 (" + @Cost + " zeny)","^FFA500Warp Portal^000000 (2.500 zeny)","Rent","^808080Reset Skills^000000","^808080Reset Stats^000000","^2F4F4FReset Total^000000","^556B2FReset Homunculus^000000","^0000FFSkills Platinum^000000","^FFA500Profesiones^000000") )
	{
		case 1: // Suministros
			mes "[^FFA500Chronos Girl^000000]";
			mes "Close this window to show the special shop High Rates Chronos for users";
			close2;
			callshop "ChronosShop",1;
			end;
		case 2: // Healer
			if( Zeny < @Cost )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You do not bring enough Zeny you, sorry.";
				close;
			}

			percentheal 100,100;
			specialeffect2 9;
			set Zeny, Zeny - @Cost;
			mes "[^FFA500Chronos Girl^000000]";
			mes "Your HP / SP has been restored!!";
			close;
		case 3: // Save
			mes "[^FFA500Chronos Girl^000000]";
			mes "Want to save in this location?";
			next;
			if( select("Yes, save my respawn here:No thanks") == 2 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Do not you realize any change.";
				close;
			}

			set @LocationID, atoi(strnpcinfo(2));
			switch( @LocationID )
			{
				case 2: set .@map$,"morocc"; set .@x, 167; set .@y, 78; break;
				case 3: set .@map$,"geffen"; set .@x, 112; set .@y, 66; break;
				case 4: set .@map$,"payon"; set .@x, 184; set .@y, 96; break;
				case 5: set .@map$,"pay_arche"; set .@x, 40; set .@y, 128; break;
				case 6: set .@map$,"alberta"; set .@x, 117; set .@y, 66; break;
				case 7: set .@map$,"izlude"; set .@x, 124; set .@y, 93; break;
				case 8: set .@map$,"aldebaran"; set .@x, 136; set .@y, 111; break;
				case 9: set .@map$,"xmas"; set .@x, 141; set .@y, 134; break;
				case 10: set .@map$,"comodo"; set .@x, 205; set .@y, 151; break;
				case 11: set .@map$,"yuno"; set .@x, 154; set .@y, 200; break;
				case 12: set .@map$,"amatsu"; set .@x, 202; set .@y, 96; break;
				case 13: set .@map$,"gonryun"; set .@x, 167; set .@y, 109; break;
				case 14: set .@map$,"umbala"; set .@x, 126; set .@y, 143; break;
				case 15: set .@map$,"niflheim"; set .@x, 205; set .@y, 194; break;
				case 16: set .@map$,"louyang"; set .@x, 221; set .@y, 106; break;
				case 17: set .@map$,"jawaii"; set .@x, 237; set .@y, 145; break;
				case 18: set .@map$,"ayothaya"; set .@x, 156; set .@y, 99; break;
				case 19: set .@map$,"einbroch"; set .@x, 71; set .@y, 185; break;
				case 20: set .@map$,"lighthalzen"; set .@x, 169; set .@y, 102; break;
				case 21: set .@map$,"einbech"; set .@x, 196; set .@y, 133; break;
				case 22: set .@map$,"hugel"; set .@x, 87; set .@y, 146; break;
				case 23: set .@map$,"rachel"; set .@x, 123; set .@y, 107; break;
				case 24: set .@map$,"veins"; set .@x, 221; set .@y, 105; break;
				case 25: set .@map$,"moscovia"; set .@x, 164; set .@y, 80; break;
				case 26: set .@map$,"mid_camp"; set .@x, 198; set .@y, 288; break;
				case 27: set .@map$,"brasilis"; set .@x, 198; set .@y, 221; break;
				default: set .@map$,"prontera"; set .@x, 142; set .@y, 168; break;
			}

			savepoint .@map$, .@x, .@y;
			mes "[^FFA500Chronos Girl^000000]";
			mes "Your respawn point has been changed.";
			close;
		
		case 4: // Teleport
		case 5: // Warp Portal
			if( @menu == 5 ) set @Cost, 2500;
			if( Zeny < @Cost )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "The cost of the operation is " + @Cost + " zeny";
				mes "When you have that money talk to me again.";
				close;
			}

			getmapxy .@map$, .@cx, .@cy, 0; // Current Char Position
			getmapxy .@mapn$, .@cxn, .@cyn, 1; // Current NPC Position
			
			if( @menu == 5 )
			{ // Warp Portal Range
				set .@x1, .@cxn - 2;
				set .@x2, .@cxn + 2;
				set .@y1, .@cyn - 2;
				set .@y2, .@cyn + 2;
				if( .@cx < .@x1 || .@cx > .@x2 || .@cy < .@y1 || .@cy > .@y2 )
				{
					mes "[^FFA500Chronos Girl^000000]";
					mes "To open a Warp Portal should be closer to me.";
					mes "Maximum 2 cells away.";
					close;
				}
			}
			
			set .@pmenu, @menu; // Previous Menu option
			set .@Zone, select("^FFA500LastWarp^000000","Cities Midgard","Schwarzwald Cities","Towns Arunafelz","Another World","Islands","Dungeons Classic","Dungeons New");
			set .@LastWarp, 0;
			set @dmap$, "";
			
			if( .@Zone == 1 )
			{
				if( LastWarpZ <= 1 || LastWarpM == 0 )
				{
					mes "[^FFA500Chronos Girl^000000]";
					mes "I'm sorry but you have not used the service before Warp or Teleport.";
					mes "Better when you decide to choose a specific area.";
					close;
				}

				set .@Zone, LastWarpZ;
				set .@Map, LastWarpM;
				set .@LastWarp, 1; // flag indicates to use last Map
			}

			switch( .@Zone )
			{
				case 2: // Cities Midgard
					if( !.@LastWarp )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Select the destination of the Warp.";
						next;
						set .@Map, select(
							       "^0000FF[" + getmapusers("prontera") + " users]^000000 Prontera",
							       "^0000FF[" + getmapusers("payon") + " users]^000000 Payon",
							       "^0000FF[" + getmapusers("morocc") + " users]^000000 Morroc",
							       "^0000FF[" + getmapusers("umbala") + " users]^000000 Umbala",
							       "^0000FF[" + getmapusers("comodo") + " users]^000000 Comodo",
							       "^0000FF[" + getmapusers("niflheim") + " users]^000000 Niflheim",
							       "^0000FF[" + getmapusers("aldebaran") + " users]^000000 Aldebaran",
							       "^0000FF[" + getmapusers("geffen") + " users]^000000 Geffen",
							       "^0000FF[" + getmapusers("alberta") + " users]^000000 Alberta",
							       "^0000FF[" + getmapusers("izlude") + " users]^000000 Izlude");
					}

					switch( .@Map )
					{
						case 1: set @dmap$, "prontera";		set @dx, 155;		set @dy, 91;	set @dname$, "Prontera"; break;
						case 2: set @dmap$, "payon"; 		set @dx, 176;		set @dy, 100;	set @dname$, "Payon"; break;;
						case 3: set @dmap$, "morocc";		set @dx, 156;		set @dy, 93;	set @dname$, "Morroc"; break;
						case 4: set @dmap$, "umbala";		set @dx, 119;		set @dy, 140;	set @dname$, "Umbala"; break;
						case 5: set @dmap$, "comodo";		set @dx, 198;		set @dy, 143;	set @dname$, "Comodo"; break;
						case 6: set @dmap$, "niflheim";		set @dx, 193;		set @dy, 185;	set @dname$, "Niflheim"; break;
						case 7: set @dmap$, "aldebaran";	set @dx, 139;		set @dy, 112;	set @dname$, "Aldebaran"; break;
						case 8: set @dmap$, "geffen";		set @dx, 120;		set @dy, 69;	set @dname$, "Geffen"; break;
						case 9: set @dmap$, "alberta";		set @dx, 116;		set @dy, 57;	set @dname$, "Alberta"; break;
						case 10: set @dmap$, "izlude";		set @dx, 128;		set @dy, 112;	set @dname$, "Izlude"; break;
					}
					break;
				case 3: // Cities Schwarzwald
					if( !.@LastWarp )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Select the destination of the Warp.";
						next;
						set .@Map, select(
							       "^0000FF[" + getmapusers("yuno") + " users]^000000 Yuno",
							       "^0000FF[" + getmapusers("hugel") + " users]^000000 Hugel",
							       "^0000FF[" + getmapusers("lighthalzen") + " users]^000000 Lighthalzen",
							       "^0000FF[" + getmapusers("einbroch") + " users]^000000 Einbroch",
							       "^0000FF[" + getmapusers("einbech") + " users]^000000 Einbech");
					}
					
					switch( .@Map )
					{
						case 1: set @dmap$, "yuno";		set @dx, 157;		set @dy, 182;	set @dname$, "Yuno"; break;
						case 2: set @dmap$, "hugel";		set @dx, 94;		set @dy, 145;	set @dname$, "Hugel"; break;
						case 3: set @dmap$, "lighthalzen";	set @dx, 158;		set @dy, 92;	set @dname$, "Lighthalzen"; break;
						case 4: set @dmap$, "einbroch";		set @dx, 64;		set @dy, 198;	set @dname$, "Einbroch"; break;
						case 5: set @dmap$, "einbech";		set @dx, 172;		set @dy, 129;	set @dname$, "Einbech"; break;
					}
					break;
				case 4: // Cities Arunafelz
					if( !.@LastWarp )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Select the destination of the Warp.";
						next;
						set .@Map, select(
							       "^0000FF[" + getmapusers("rachel") + " users]^000000 Rachel",
							       "^0000FF[" + getmapusers("veins") + " users]^000000 Veins");
					}
					
					switch( .@Map )
					{
						case 1: set @dmap$, "rachel";		set @dx, 130;		set @dy, 110;	set @dname$, "Rachel"; break;
						case 2: set @dmap$, "veins";		set @dx, 216;		set @dy, 128;	set @dname$, "Veins"; break;
					}
					break;
				case 5: // Another World
					if( !.@LastWarp )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Select the destination of the Warp.";
						next;
						set .@Map, select(
							       "^0000FF[" + getmapusers("mid_camp") + " users]^000000 Midgarts Expedition Camp",
							       "^0000FF[" + getmapusers("man_fild01") + " users]^000000 Manuk Field",
							       "^0000FF[" + getmapusers("spl_fild02") + " users]^000000 Splendide Field");
					}
					
					switch( .@Map )
					{
						case 1: set @dmap$, "mid_camp";		set @dx, 210;		set @dy, 290;	set @dname$, "Expedition Camp"; break;
						case 2: set @dmap$, "man_fild01";	set @dx, 35;		set @dy, 236;	set @dname$, "Manuk Field"; break;
						case 3: set @dmap$, "spl_fild02";	set @dx, 379;		set @dy, 143;	set @dname$, "Splendide Field"; break;
					}
					break;
				case 6: // Islas
					if( !.@LastWarp )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Select the destination of the Warp.";
						next;
						set .@Map, select(
							       "^0000FF[" + getmapusers("louyang") + " users]^000000 Louyang",
							       "^0000FF[" + getmapusers("gonryun") + " users]^000000 Gonryun",
							       "^0000FF[" + getmapusers("ayothaya") + " users]^000000 Ayothaya",
							       "^0000FF[" + getmapusers("moscovia") + " users]^000000 Moscovia",
							       "^0000FF[" + getmapusers("brasilis") + " users]^000000 Brasilia",
							       "^0000FF[" + getmapusers("xmas") + " users]^000000 Lutie",
							       "^0000FF[" + getmapusers("amatsu") + " users]^000000 Amatsu");
					}

					switch( .@Map )
					{
						case 1: set @dmap$, "louyang";		set @dx, 217;		set @dy, 104;	set @dname$, "Louyang"; break;
						case 2: set @dmap$, "gonryun";		set @dx, 159;		set @dy, 114;	set @dname$, "Gonryun"; break;
						case 3: set @dmap$, "ayothaya";		set @dx, 149;		set @dy, 117;	set @dname$, "Ayothaya"; break;
						case 4: set @dmap$, "moscovia";		set @dx, 223;		set @dy, 191;	set @dname$, "Moscovia"; break;
						case 5: set @dmap$, "brasilis";		set @dx, 198;		set @dy, 221;	set @dname$, "Brasilia"; break;
						case 6: set @dmap$, "xmas";		set @dx, 147;		set @dy, 133;	set @dname$, "Lutie"; break;
						case 7: set @dmap$, "amatsu";		set @dx, 198;		set @dy, 107;	set @dname$, "Amatsu"; break;
					}
					break;
				case 7: // Dungeons Clásicos
					if( !.@LastWarp )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Select the destination of the Warp.";
						next;
						set .@Map, select(
							       "^0000FF[" + getmapusers("ama_dun01") + " users]^000000 Amatsu Dungeon",
							       "^0000FF[" + getmapusers("anthell01") + " users]^000000 Ant Hell",
							       "^0000FF[" + getmapusers("ayo_dun01") + " users]^000000 Ayothaya Dungeon F1",
							       "^0000FF[" + getmapusers("ayo_dun02") + " users]^000000 Ayothaya Dungeon F2",
							       "^0000FF[" + getmapusers("iz_dun00") + " users]^000000 Undersea Tunel",
							       "^0000FF[" + getmapusers("prt_sewb1") + " users]^000000 Prontera Culvert",
							       "^0000FF[" + getmapusers("mjo_dun01") + " users]^000000 Mjolnir Dead Pit",
							       "^0000FF[" + getmapusers("ein_dun01") + " users]^000000 Mine Dungeon",
							       "^0000FF[" + getmapusers("gl_church") + " users]^000000 Glast Heim Church",
							       "^0000FF[" + getmapusers("gl_cas01") + " users]^000000 Glast Heim Castle",
							       "^0000FF[" + getmapusers("gl_dun01") + " users]^000000 Glast Heim Cave",
							       "^0000FF[" + getmapusers("gl_knt01") + " users]^000000 Glast Heim Chivalry",
							       "^0000FF[" + getmapusers("gl_prison1") + " users]^000000 Glast Heim Prison",
							       "^0000FF[" + getmapusers("gl_sew01") + " users]^000000 Glast Heim Culvert",
							       "^0000FF[" + getmapusers("gl_step") + " users]^000000 Glast Heim Staircase",
							       "^0000FF[" + getmapusers("gon_dun01") + " users]^000000 Gonryun Dungeon",
							       "^0000FF[" + getmapusers("mag_dun01") + " users]^000000 Nogg Road",
							       "^0000FF[" + getmapusers("orcsdun01") + " users]^000000 Orc Dungeon",
							       "^0000FF[" + getmapusers("pay_dun00") + " users]^000000 Payon Cave",
							       "^0000FF[" + getmapusers("moc_pryd01") + " users]^000000 Morroc Pyramid",
							       "^0000FF[" + getmapusers("in_sphinx1") + " users]^000000 Morroc Sphinx",
							       "^0000FF[" + getmapusers("treasure01") + " users]^000000 Sunken Ship",
							       "^0000FF[" + getmapusers("xmas_dun01") + " users]^000000 Toy Factory",
							       "^0000FF[" + getmapusers("tur_dun01") + " users]^000000 Turtle Island",
							       "^0000FF[" + getmapusers("gefenia01") + " users]^000000 Geffenia");
					}
					
					switch( .@Map )
					{
						case 1: set @dmap$, "ama_dun01";	set @dx, 230;		set @dy, 13;	set @dname$, "Amatsu Dungeon"; break;
						case 2: set @dmap$, "anthell01";	set @dx, 36;		set @dy, 262;	set @dname$, "Ant Hell"; break;
						case 3: set @dmap$, "ayo_dun01";	set @dx, 271;		set @dy, 18;	set @dname$, "Ayothaya Dungeon F1"; break;
						case 4: set @dmap$, "ayo_dun02";	set @dx, 150;		set @dy, 66;	set @dname$, "Ayothaya Dungeon F2"; break;
						case 5: set @dmap$, "iz_dun00";		set @dx, 166;		set @dy, 162;	set @dname$, "Undersea Tunel"; break;
						case 6: set @dmap$, "prt_sewb1";	set @dx, 126;		set @dy, 247;	set @dname$, "Prontera Culvert"; break;
						case 7: set @dmap$, "mjo_dun01";	set @dx, 51;		set @dy, 24;	set @dname$, "Mjolnir Dead Pit"; break;
						case 8: set @dmap$, "ein_dun01";	set @dx, 21;		set @dy, 21;	set @dname$, "Mine Dungeon"; break;
						case 9: set @dmap$, "gl_church";	set @dx, 155;		set @dy, 15;	set @dname$, "Glast Heim Church"; break;
						case 10: set @dmap$, "gl_cas01";	set @dx, 200;		set @dy, 158;	set @dname$, "Glast Heim Castle"; break;
						case 11: set @dmap$, "gl_dun01";	set @dx, 134;		set @dy, 268;	set @dname$, "Glast Heim Cave"; break;
						case 12: set @dmap$, "gl_knt01";	set @dx, 103;		set @dy, 199;	set @dname$, "Glast Heim Chivalry"; break;
						case 13: set @dmap$, "gl_prison1";	set @dx, 150;		set @dy, 17;	set @dname$, "Glast Heim Prison"; break;
						case 14: set @dmap$, "gl_sew01";	set @dx, 255;		set @dy, 255;	set @dname$, "Glast Heim Culvert"; break;
						case 15: set @dmap$, "gl_step";		set @dx, 11;		set @dy, 7;	set @dname$, "Glast Heim Staircase"; break;
						case 16: set @dmap$, "gon_dun01";	set @dx, 153;		set @dy, 53;	set @dname$, "Gonryun Dungeon"; break;
						case 17: set @dmap$, "mag_dun01";	set @dx, 125;		set @dy, 70;	set @dname$, "Nogg Road"; break;
						case 18: set @dmap$, "orcsdun01";	set @dx, 32;		set @dy, 169;	set @dname$, "Orc Dungeon"; break;
						case 19: set @dmap$, "pay_dun00";	set @dx, 21;		set @dy, 181;	set @dname$, "Payon Cave"; break;
						case 20: set @dmap$, "moc_pryd01";	set @dx, 190;		set @dy, 10;	set @dname$, "Morroc Pyramid"; break;
						case 21: set @dmap$, "in_sphinx1";	set @dx, 287;		set @dy, 15;	set @dname$, "Morroc Sphinx"; break;
						case 22: set @dmap$, "treasure01";	set @dx, 69;		set @dy, 28;	set @dname$, "Sunken Ship"; break;
						case 23: set @dmap$, "xmas_dun01";	set @dx, 206;		set @dy, 41;	set @dname$, "Toy Factory"; break;
						case 24: set @dmap$, "tur_dun01";	set @dx, 154;		set @dy, 43;	set @dname$, "Turtle Island"; break;
						case 25: set @dmap$, "gefenia01";	set @dx, 58;		set @dy, 169;	set @dname$, "Geffenia"; break;
					}
					break;
				case 8: // Dungeons Nuevos
					if( !.@LastWarp )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Select the destination of the Warp.";
						next;
						set .@Map, select(
							       "^0000FF[" + getmapusers("abyss_01") + " users]^000000 Abyss Lake",
							       "^0000FF[" + getmapusers("abbey01") + " users]^000000 Cursed Abbey",
							       "^0000FF[" + getmapusers("juperos_01") + " users]^000000 Juperos F1",
							       "^0000FF[" + getmapusers("juperos_02") + " users]^000000 Juperos F2",
							       "^0000FF[" + getmapusers("jupe_core") + " users]^000000 Juperos Core",
							       "^0000FF[" + getmapusers("lhz_dun01") + " users]^000000 Somatology Laboratory",
							       "^0000FF[" + getmapusers("odin_tem01") + " users]^000000 Odin Shrine",
							       "^0000FF[" + getmapusers("tha_t01") + " users]^000000 Thanatos Tower",
							       "^0000FF[" + getmapusers("ice_dun01") + " users]^000000 Ice Dungeon",
							       "^0000FF[" + getmapusers("kh_dun02") + " users]^000000 Robot Factory",
							       "^0000FF[" + getmapusers("ra_san01") + " users]^000000 Rachel Sanctuary",
							       "^0000FF[" + getmapusers("thor_v01") + " users]^000000 Thor Volcano",
							       "^0000FF[" + getmapusers("mosk_fild02") + " users]^000000 Okrestnosti of Moscovia");
					}

					switch( .@Map )
					{
						case 1: set @dmap$, "abyss_01";		set @dx, 263;		set @dy, 272;	set @dname$, "Abyss Lake"; break;
						case 2: set @dmap$, "abbey01";		set @dx, 50;		set @dy, 19;	set @dname$, "Cursed Abbey"; break;
						case 3: set @dmap$, "juperos_01";	set @dx, 53;		set @dy, 245;	set @dname$, "Juperos F1"; break;
						case 4: set @dmap$, "juperos_02";	set @dx, 128;		set @dy, 269;	set @dname$, "Juperos F2"; break;
						case 5: set @dmap$, "jupe_core";	set @dx, 149;		set @dy, 270;	set @dname$, "Juperos Core"; break;
						case 6: set @dmap$, "lhz_dun01";	set @dx, 149;		set @dy, 284;	set @dname$, "Somatology Laboratory"; break;
						case 7: set @dmap$, "odin_tem01";	set @dx, 98;		set @dy, 148;	set @dname$, "Odin Shrine"; break;
						case 8: set @dmap$, "tha_t01";		set @dx, 150;		set @dy, 40;	set @dname$, "Thanatos Tower"; break;
						case 9: set @dmap$, "ice_dun01";	set @dx, 156;		set @dy, 15;	set @dname$, "Ice Dungeon"; break;
						case 10: set @dmap$, "kh_dun02";	set @dx, 41;		set @dy, 198;	set @dname$, "Robot Factory"; break;
						case 11: set @dmap$, "ra_san01";	set @dx, 140;		set @dy, 135;	set @dname$, "Rachel Sanctuary"; break;
						case 12: set @dmap$, "thor_v01";	set @dx, 21;		set @dy, 229;	set @dname$, "Thor Volcano"; break;
						case 13: set @dmap$, "mosk_fild02";	set @dx, 117;		set @dy, 202;	set @dname$, "Okrestnosti of Moscovia"; break;
					}
					break;
			}
			// CODE!!

			// Stores info about Last Warp
			set LastWarpZ, .@Zone;
			set LastWarpM, .@Map;

			set Zeny, Zeny - @Cost;
			if( .@pmenu == 4 )
			{ // Simple Warp
				warp @dmap$, @dx, @dy;
				end;
			}
			
			mes "[^FFA500Chronos Girl^000000]";
			mes "Closes this conversation to give opening to Warp.";
			close2;
			specialeffect 58;
			switch( rand(3) )
			{
				case 0: npctalk "Warp Portal to " + @dname$ + "."; break;
				case 1: npctalk "Attention passengers, Portal to " + @dname$ + ". Is Now Open."; break;
				case 2: npctalk "Portal live " + @dname$ + " courtesy " + strcharinfo(0); break;
			}
			warpportal .@cx, .@cy, @dmap$, @dx, @dy;
			end;
		case 6: // Breeder
			mes "[^FFA500Chronos Girl^000000]";
			mes "Say, you want to rent?.";
			next;
			switch( select("Rent Cart (1000 zeny):Rent Falcon (1000 zeny):Rent Pecopeco (1000 zeny)") )
			{
				case 1: // Cart
					if( BaseClass != Job_Merchant && BaseJob != Job_SuperNovice )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Sorry, soil provides this service Merchats the Super Novice.";
						close;
					}
					
					if( checkcart() != 0 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You can only use one cart at a time.";
						close;
					}
					
					if( getskilllv("MC_PUSHCART") < 1 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not have the skill to use Shopping.";
						close;
					}
					
					if( Zeny < 1000 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not bring enough Zeny you, sorry.";
						close;
					}
					
					set Zeny, Zeny - 1000;					
					setcart;
					mes "[^FFA500Chronos Girl^000000]";
					mes "Here's your car. Good day!!";
					close;
				case 2: // Falcon
					if( BaseJob != Job_Hunter )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "This service is only for Hunters and Snipers.";
						close;
					}
					
					if( checkfalcon() != 0 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not need a Falcon if you already have one.";
						close;
					}
					
					if( getskilllv("HT_FALCON") < 1 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not have the skill to use Falcon.";
						close;
					}
					
					if( Zeny < 1000 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not bring enough Zeny you, sorry.";
						close;
					}
					
					set Zeny, Zeny - 1000;
					setfalcon;
					mes "[^FFA500Chronos Girl^000000]";
					mes "There's your falcon. Good day!!";
					close;
				case 3: // Peco Peco
					if( BaseJob != Job_Knight && BaseJob != Job_Crusader)
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "This service is only for Knights, Crusaders, Lord Knights and Paladins.";
						close;
					}
					
					if( checkriding() != 0 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Do not need a PecoPeco if you already have one.";
						close;
					}
					
					if( getskilllv("KN_RIDING") < 1 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not have the skill to mount PecoPeco.";
						close;
					}
					
					if( Zeny < 1000 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not bring enough Zeny you, sorry.";
						close;
					}
					
					set Zeny, Zeny - 1000;
					setriding;
					mes "[^FFA500Chronos Girl^000000]";
					mes "There's your PecoPeco. Good day!!";
					close;
			}
			end;
		case 7: // Reset Skills
			mes "[^FFA500Chronos Girl^000000]";
			mes "The cost of the Skills ^0000FFReset Skills^000000 Zeny is 250.000 zeny. To do this you must not exceed 1000 weight in your inventory and not bring Pecopeco, Falcon or truck.";
			mes "Only 1 reset of skills performed every 2 hours.";
			next;
			if( select("Proceed with reset","No thanks...") == 2 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Come back when you need some special service.";
				close;
			}
			
			if( Zeny < 250000 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You do not bring enough Zeny you, sorry.";
				close;
			}
		
			if( LastReset_SK > gettimetick(2) )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Only resets skills performed every 2 hours. You must wait a little longer to reset your skills again.";
				close;
			}
			
			if( Weight > 10000 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "I need to reset your skills that do not exceed more than 1000 items of weight with you, so leave some things in Storage and returns.";
				close;
			}
			
			if( checkriding() || checkfalcon() || checkcart() )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Reset Skills required to come without Car / PecoPeco / Falcon. remove them and talk again.";
				close;
			}
			
			sc_end SC_ALL;
			resetskill;
			set Zeny, Zeny - 250000;
			set LastReset_SK, gettimetick(2) + 7200;

			mes "[^FFA500Chronos Girl^000000]";
			mes "Your skills have been reset, think about your options. Luck!!";
			close;
		case 8:
			mes "[^FFA500Chronos Girl^000000]";
			mes "The cost for 0000FFReset of Stats ^ ^ 000000 zeny is 250,000. To do this you must not exceed 1000 weight in your inventory.";
			mes "Only one performed stats reset every two hours.";
			next;
			if( select("Proceed with reset","No Thanks...") == 2 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Come back when you need some special service.";
				close;
			}
			
			if( Zeny < 250000 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You do not bring enough Zeny you, sorry..";
				close;
			}
		
			if( LastReset_ST > gettimetick(2) )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Only stat resets performed every 2 hours. You must wait a little longer to get back to reset your stats.";
				close;
			}
			
			if( Weight > 10000 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "I need to reset your stats that do not exceed more than 1000 items of weight with you, so leave some things in Storage and returns.";
				close;
			}
			
			sc_end SC_ALL;
			resetstatus;
			set Zeny, Zeny - 250000;
			set LastReset_ST, gettimetick(2) + 7200;

			mes "[^FFA500Chronos Girl^000000]";
			mes "Your stats have been reset, think about your options. Luck!!";
			close;
		case 9:
			mes "[^FFA500Chronos Girl^000000]";
			mes "The cost of the ^0000FFReset Skills and Stats^000000 is 450.000 zeny. To do this you must not exceed more than 1000 weight and not bring Pecopeco, Falcon or truck.";
			mes "Only I can make a total reset if you have not done or stats reset skills in the last two hours.";
			next;
			if( select("Proceed with reset","No Thanks...") == 2 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Come back when you need some special service.";
				close;
			}
			
			if( Zeny < 450000 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You do not bring enough Zeny you, sorry.";
				close;
			}
		
			if( LastReset_SK > gettimetick(2) )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Only resets skills performed every 2 hours. You must wait a little longer to reset your skills again.";
				close;
			}

			if( LastReset_ST > gettimetick(2) )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Only stat resets performed every 2 hours. You must wait a little longer to get back to reset your stats.";
				close;
			}

			if( Weight > 10000 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "To reset your skills and stats need not overdo over 1000 items weight with you, so leave some things in Storage and returns.";
				close;
			}

			if( checkriding() || checkfalcon() || checkcart() )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Reset Skills required to come without Car / PecoPeco / Falcon. remove them and talk again.";
				close;
			}
			
			sc_end SC_ALL;
			resetskill;
			resetstatus;
			set Zeny, Zeny - 450000;
			set LastReset_SK, gettimetick(2) + 7200;
			set LastReset_ST, gettimetick(2) + 7200;

			mes "[^FFA500Chronos Girl^000000]";
			mes "Your skills and stats have been reset, think about your options. Luck!";
			close;
		case 10:
			mes "[^FFA500Chronos Girl^000000]";
			mes "The cost of the ^0000FFReset Homunculus^000000 is 300z.000 zeny. To do this you must have your active homunculus.";
			mes "I can only do Homunculus reset every two hours";
			next;
			if( select("Proceed with reset","No thanks...") == 2 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Come back when you need some special service.";
				close;
			}
			
			if( gethominfo(0) == 0 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Have one active homunculus.";
				close;
			}
			
			if( Zeny < 300000 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You do not bring enough Zeny you, sorry.";
				close;
			}
		
			if( LastReset_HM > gettimetick(2) )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Only resets homunculus performed every 2 hours. You must wait a little longer to get back to reset your homunculus.";
				close;
			}
			
			homshuffle;
			set Zeny, Zeny - 300000;
			set LastReset_HM, gettimetick(2) + 7200;

			mes "[^FFA500Chronos Girl^000000]";
			mes "Your Homunculus has been reset, you must reconfigure their skills. Luck!!";
			close;
		case 11:
			mes "[^FFA500Chronos Girl^000000]";
			mes "At no additional cost, I can give the platinum skills for your class.";
			mes "You're ready to get the skills?";
			next;
			if( select("Get skills Platinum","No thanks...") == 2 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "Come back when you need some special service.";
				close;
			}
			
			switch( BaseClass )
			{
				case Job_Novice:	skill 142,1,0; if( BaseJob == 0 ) skill 143,1,0; break;
				case Job_Swordman:	skill 142,1,0; skill 144,1,0; skill 145,1,0; skill 146,1,0; break;
				case Job_Mage:		skill 142,1,0; skill 157,1,0; break;
				case Job_Archer:	skill 142,1,0; skill 147,1,0; skill 148,1,0; break;
				case Job_Acolyte:	skill 142,1,0; skill 156,1,0; break;
				case Job_Merchant:	skill 142,1,0; skill 153,1,0; skill 154,1,0; skill 155,1,0; break;
				case Job_Thief:		skill 142,1,0; skill 149,1,0; skill 150,1,0; skill 151,1,0; skill 152,1,0; break;
			}

			mes "[^FFA500Chronos Girl^000000]";
			mes "You have received your first class skills, let's see if you can get to Second Class.";
			next;
			
			switch( Class )
			{
				case Job_Knight:
				case Job_Lord_Knight:
				case Job_Lord_Knight2:
				case Job_Baby_Knight:
					skill 1001,1,0;
					break;
				case Job_Crusader:
				case Job_Paladin:
				case Job_Paladin2:
				case Job_Baby_Crusader:
					skill 1002,1,0;
					break;
				case Job_Wizard:
				case Job_High_Wizard:
				case Job_Baby_Wizard:
					skill 1006,1,0;
					break;
				case Job_Sage:
				case Job_Professor:
				case Job_Baby_Sage:
					skill 1007,1,0;
					switch( sage_ele )
					{
						case 1: skill 1008,1,0; break;
						case 2: skill 1017,1,0; break;
						case 3: skill 1018,1,0; break;
						case 4: skill 1019,1,0; break;
						default:
							mes "[^FFA500Chronos Girl^000000]";
							mes "Select the item that you will use with Elemental Change.";
							next;
							switch( select ("Agua","Tierra","Fuego","Viento") )
							{
								case 1: skill 1008,1,0; set sage_ele,1; break;
								case 2: skill 1017,1,0; set sage_ele,2; break;
								case 3: skill 1018,1,0; set sage_ele,3; break;
								case 4: skill 1019,1,0; set sage_ele,4; break;
							}
							break;
					}
					break;
				case Job_Hunter:
				case Job_Sniper:
				case Job_Baby_Hunter:
					skill 1009,1,0;
					break;
				case Job_Bard:
				case Job_Clown:
				case Job_Baby_Bard:
					skill 1010,1,0;
					break;
				case Job_Dancer:
				case Job_Gypsy:
				case Job_Baby_Dancer:
					skill 1011,1,0;
					break;
				case Job_Priest:
				case Job_High_Priest:
				case Job_Baby_Priest:
					skill 1014,1,0;
					break;
				case Job_Monk:
				case Job_Champion:
				case Job_Baby_Monk:
					skill 1015,1,0;
					skill 1016,1,0;
					break;
				case Job_Blacksmith:
				case Job_Whitesmith:
				case Job_Baby_Blacksmith:
					skill 1012,1,0;
					skill 1013,1,0;
					break;
				case Job_Alchem:
				case Job_Creator:
				case Job_Baby_Alchem:
					skill 238,1,0;
					if( alch_bock != 1 ) getitem 7434,1;
					set alch_bock,1;
					break;
				case Job_Assassin:
				case Job_Assassin_Cross:
				case Job_Baby_Assassin:
					skill 1003,1,0;
					skill 1004,1,0;
					break;
				case Job_Rogue:
				case Job_Stalker:
				case Job_Baby_Rogue:
					skill 1005,1,0;
					break;
			}
			
			mes "[^FFA500Chronos Girl^000000]";
			mes "All you can receive any skills you have been granted.";
			close;
		case 12:
			mes "[^FFA500Chronos Girl^000000]";
			mes "This is the career change service, to do change quickly without having to do the quests.";
			next;
			if( Upper == 1 && Class >= Job_Lord_Knight )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You already have the maximum allowed for your class profession and I can not provide this service.";
				close;
			}

			if( SkillPoint != 0 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You need to use all your skill points before career change.";
				mes "Come back soon!!.";
				close;
			}
			
			if( JobLevel < 10 )
			{
				mes "[^FFA500Chronos Girl^000000]";
				mes "You do not meet the level requirements for this service job.";
				mes "Come back soon!!.";
				close;
			}
			
			switch( Class )
			{
				case Job_Novice_High:
				case Job_Baby:
				case Job_Novice:
					if( ADVJOB != 0 && Class == Job_Novice_High )
					{
						switch( ADVJOB )
						{
							case Job_High_Priest:
							case Job_Champion:
								set @target_job, Job_Acolyte_High;
								break;
							case Job_Sniper:
							case Job_Clown:
							case Job_Gypsy:
								set @target_job, Job_Archer_High;
								break;
							case Job_High_Wizard:
							case Job_Professor:
								set @target_job, Job_Mage_High;
								break;
							case Job_Whitesmith:
							case Job_Creator:
								set @target_job, Job_Merchant_High;
								break;
							case Job_Lord_Knight:
							case Job_Paladin:
								set @target_job, Job_Swordman_High;
								break;
							case Job_Assassin_Cross:
							case Job_Stalker:
								set @target_job, Job_Thief_High;
								break;
						}
					}
					else
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Please select the profession where you want to develop.";
						next;
						switch( select("Swordman:Mage:Archer:Acolyte:Merchant:Thief:Super Novice:Taekwon:Gunslinger:Ninja") )
						{
							case 7:
								if( Class == Job_Novice_High )
								{
									mes "[^FFA500Chronos Girl^000000]";
									mes "You do not meet the requirements to transform into the selected profession.";
									close;
								}
								
								if( BaseLevel < 45 )
								{
									mes "[^FFA500Chronos Girl^000000]";
									mes "You do not have enough level to become the selected job.";
									close;
								}
								
								if( Upper == 2 )
									set @target_job, Job_Super_Baby;
								else 
									set @target_job, Job_SuperNovice;
								break;
							case 8:
								if( Class == Job_Novice_High || Upper == 2 )
								{
									mes "[^FFA500Chronos Girl^000000]";
									mes "You do not meet the requirements to transform into the selected profession.";
									close;
								}
								
								set @target_job, Job_Taekwon;
								break;
							case 9:
							case 10:
								if( Class == Job_Novice_High || Upper == 2 )
								{
									mes "[^FFA500Chronos Girl^000000]";
									mes "You do not meet the requirements to transform into the selected profession.";
									close;
								}
								
								set @target_job, @menu + 15;
								break;
							default:
								set @target_job, @menu;
								if( Class == Job_Novice_High )
									set @target_job, @target_job + 4001;
								break;
						}
					}
					
					mes "[^FFA500Chronos Girl^000000]";
					mes "Are you sure you want to change your profession to ^0000FF" + jobname(@target_job) + "^000000?";
					next;
					if( select("Yes, I want to change.","No thanks") == 2 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "A taste help, come back when you decide.";
						close;
					}
					
					callfunc "Job_Change", @target_job;
					if( @target_job == Job_Gunslinger || @target_job == Job_Ninja || @target_job == Job_Taekwon )
						callfunc "F_ClearJobVar";
					
					mes "[^FFA500Chronos Girl^000000]";
					mes "Usually, good luck and again when you need some other service.";
					close;

				default:
					if( JobLevel < 40 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "You do not meet the level requirements for this service job.";
						mes "Come back soon!!.";
						close;
					}
					
					deletearray @job_opt, getarraysize(@job_opt);
					if( Class < Job_Knight || Class == Job_Taekwon || (Class > Job_Baby && Class < Job_Baby_Knight) || (Class > Job_Novice_High && Class < Job_Lord_Knight) )
					{ // First Class | First Advanced Class
						if( ADVJOB )
							set @target_job, ADVJOB;
						else
						{
							switch( Class )
							{
								case Job_Swordman_High:
								case Job_Baby_Swordman:
								case Job_Swordman:
									set @job_opt[0], Job_Knight;
									set @job_opt[1], Job_Crusader;
									break;
								case Job_Mage_High:
								case Job_Baby_Mage:
								case Job_Mage:
									set @job_opt[0], Job_Wizard;
									set @job_opt[1], Job_Sage;
									break;
								case Job_Archer_High:
								case Job_Baby_Archer:
								case Job_Archer:
									set @job_opt[0], Job_Hunter;
									if(Sex == 0)
										set @job_opt[1], Job_Dancer;
									else
										set @job_opt[1], Job_Bard;
									break;
								case Job_Acolyte_High:
								case Job_Baby_Acolyte:
								case Job_Acolyte:
									set @job_opt[0], Job_Priest;
									set @job_opt[1], Job_Monk;
									break;
								case Job_Merchant_High:
								case Job_Baby_Merchant:
								case Job_Merchant:
									set @job_opt[0], Job_Blacksmith;
									set @job_opt[1], Job_Alchemist;
									break;
								case Job_Thief_High:
								case Job_Baby_Thief:
								case Job_Thief:
									set @job_opt[0], Job_Assassin;
									set @job_opt[1], Job_Rogue;
									break;						
								default:
									set @job_opt[0], Job_Star_Gladiator;
									set @job_opt[1], Job_Soul_Linker;
									break;
							}
							
							mes "[^FFA500Chronos Girl^000000]";
							mes "Please select the profession in which you want to be converted.";
							next;
							set @target_job, @job_opt[ select(jobname(@job_opt[0]), jobname(@job_opt[1])) - 1 ];
							
							// La siguiente opcion no debería suceder pues ADVJOB debería tener un valor
							if( Class > Job_Novice_High && Class < Job_Lord_Knight )
								set @target_job, @target_job + 4001;
						}
						
						mes "[^FFA500Chronos Girl^000000]";
						mes "Are you ready to change careers to ^0000FF" + jobname(@target_job) + "^000000?";
						next;
						if( select("Yes, I want to change..","No thanks") == 2 )
						{
							mes "[^FFA500Chronos Girl^000000]";
							mes "A taste help, come back when you decide.";
							close;
						}
						
						callfunc "Job_Change", @target_job;
						if( ADVJOB ) set ADVJOB,0;

						if( @target_job == Job_Star_Gladiator || @target_job == Job_Soul_Linker )
							callfunc "F_ClearJobVar";

						mes "[^FFA500Chronos Girl^000000]";
						mes "Usually, good luck and again when you need some other service.";
						close;
					}
					
					// Advanced Class
					if( checkfalcon() || checkcart() || checkriding() )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "In order to proceed with this service and see if you reborn, you must take your Pecopoco / Falco / Truck.";
						close;
					}
					
					if( !(Class >= Job_Knight && Class <=Job_Crusader2) )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "For now I can not offer more services of this type..";
						close;
					}
					
					if( BaseLevel < 99 || JobLevel < 50 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "Your next career change can be processed when you are level 99/70.";
						close;
					}
					
					mes "[^FFA500Chronos Girl^000000]";
					mes "Are you ready for revival?";
					next;
					if( select("Yes, I want to change...","No thanks") == 2 )
					{
						mes "[^FFA500Chronos Girl^000000]";
						mes "A taste help, come back when you decide.";
						close;
					}
					
					set ADVJOB, Class + Job_Novice_High;
					if( ADVJOB == Job_Lord_Knight2 )
						set ADVJOB,Job_Lord_Knight;
					if( ADVJOB == Job_Paladin2 )
						set ADVJOB,Job_Paladin;
					
					jobchange Job_Novice_High;
					resetlvl(1);
					set MISC_QUEST,MISC_QUEST | 1024; //<-reset Skill Reset Event
					skill 142,1,0;
					skill 143,1,0;
					
					mes "[^FFA500Chronos Girl^000000]";
					mes "Usually, good luck and again when you need some other service.";
					close;
			}
	}
	end;

OnTouch: // Restauration Aura
	if( rand(1,100) < 80 ) end;
	sc_start4 SC_L_LIFEPOTION,20000,-10,2,0,0;
	sc_start4 SC_SPREGEN,20000,-10,2,0,0;
	emotion e_kis;
	if( sex == 1 ) emotion e_lv,1;
	else emotion ET_HUK,1;
	end;
}

prontera,142,171,3	duplicate(Chronos Girl)	Chronos Girl#1	831,2,2
morocc,167,81,2	duplicate(Chronos Girl)	Chronos Girl#2	831,2,2
geffen,109,66,5	duplicate(Chronos Girl)	Chronos Girl#3	831,2,2
payon,184,99,3	duplicate(Chronos Girl)	Chronos Girl#4	831,2,2
pay_arche,37,128,6	duplicate(Chronos Girl)	Chronos Girl#5	831,2,2
alberta,120,69,3	duplicate(Chronos Girl)	Chronos Girl#6	831,2,2
izlude,121,93,5	duplicate(Chronos Girl)	Chronos Girl#7	831,2,2
aldebaran,133,111,5	duplicate(Chronos Girl)	Chronos Girl#8	831,2,2
xmas,138,134,5	duplicate(Chronos Girl)	Chronos Girl#9	831,2,2
comodo,202,151,5	duplicate(Chronos Girl)	Chronos Girl#10	831,2,2
yuno,151,200,5	duplicate(Chronos Girl)	Chronos Girl#11	831,2,2
amatsu,202,99,3	duplicate(Chronos Girl)	Chronos Girl#12	831,2,2
gonryun,167,112,3	duplicate(Chronos Girl)	Chronos Girl#13	831,2,2
umbala,126,146,3	duplicate(Chronos Girl)	Chronos Girl#14	831,2,2
niflheim,205,191,0	duplicate(Chronos Girl)	Chronos Girl#15	831,2,2
louyang,221,109,5	duplicate(Chronos Girl)	Chronos Girl#16	831,2,2
jawaii,234,145,4	duplicate(Chronos Girl)	Chronos Girl#17	831,2,2
ayothaya,156,102,3	duplicate(Chronos Girl)	Chronos Girl#18	831,2,2
einbroch,68,185,1	duplicate(Chronos Girl)	Chronos Girl#19	831,2,2
lighthalzen,169,105,5	duplicate(Chronos Girl)	Chronos Girl#20	831,2,2
einbech,193,133,7	duplicate(Chronos Girl)	Chronos Girl#21	831,2,2
hugel,87,149,5	duplicate(Chronos Girl)	Chronos Girl#22	831,2,2
rachel,120,107,7	duplicate(Chronos Girl)	Chronos Girl#23	831,2,2
veins,221,108,5	duplicate(Chronos Girl)	Chronos Girl#24	831,2,2
moscovia,236,187,3	duplicate(Chronos Girl)	Chronos Girl#25	831,2,2
mid_camp,198,291,4	duplicate(Chronos Girl)	Chronos Girl#26	831,2,2
brasilis,200,223,3	duplicate(Chronos Girl)	Chronos Girl#27	831,2,2
