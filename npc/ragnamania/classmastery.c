prontera,148,203,4	script	Class Mastery::CMSys	405,{
	set .@n$,"[Class Mastery]";
	mes .@n$;
	mes "How may I assist you?";
	next;
	switch(select(" ~ Purchase a manual: ~ Learn a New Class: ~ ^777777Close^000000")) {
		case 1:
			mes .@n$;
			if (!.Mode[3]) {
				mes "It seems like we're out of stock";
				mes "at the moment. My apologies.";
				close; }
			if (countitem(.ManualID)) {
				mes "We only allow one manual per character. Orders, orders...";
				close; }
			mes "Manuals cost ^FF0000"+.Mode[3]+" Zeny^000000.";
			mes "Would you like one?";
			next;
			switch(select(" ~ Sure!: ~ I'll think about it.")) {
				case 1:
					mes .@n$;
					if (Zeny < .Mode[3]) {
						mes "You don't appear to have";
						mes "enough Zeny... These are very";
						mes "valuable books, you know?";
						close; }
					set Zeny, Zeny-.Mode[3];
					getitem .ManualID,1;
					specialeffect2 441;
					mes "Enjoy!";
					close;
				case 2:
					mes .@n$;
					mes "Okay. Take your time.";
					close; }
		case 2:
			mes .@n$;
//			if (!.Mode[4]) {
//				mes "No classes are available for";
//				mes "purchase. You'll have to earn";
//				mes "them manually, I suppose...";
//				close; }
				
			if (!countitem(.ManualID)) {
				mes "You'll need a manual first!";
				close; }
		
//			mes "Entries cost ^FF0000"+.Mode[4]+" Zeny^000000 each.";
//			if (Zeny < .Mode[4]) {
//				mes "Go earn some more!";
//				close; }
			
			if (SkillPoint) {
				mes "Please use all your skill points before proceeding.";
				close;
			}
		    
			if ( JobLevel < 50 ) {
   				mes "No classes are available for";
   				mes "you until you master the";
   				mes "current one.";
   				mes " ";
   				.@jlvl = 50 - JobLevel;						// Brute Codded Level maximo a 50. Biali
   				mes "^AA0000You need " + ((.@jlvl > 0) ? .@jlvl + " more job levels " : "") + "to continue.^000000";
   				close; 
			}
			
			if ( Class > 6 ) {					// Brute Codded Level maximo a 50. Biali
				set .@menu$,"";
				set .@p, 1;
				for(set .@i,1; .@i<getarraysize(.JobList); .@i++) {
					if(.@i == 12 && sex == 0) .@dancer++; // Biali - Se for mulher, pula pra id 20, que eh dancer.
					if (!(Class_Mastery & pow(2,(.@i+.@dancer)))) {
						set .@menu$, .@menu$+" ~ "+jobname(.JobList[(.@i+.@dancer)]);
						setarray .@classes[.@p], .JobList[(.@i+.@dancer)];
						.@p++;
						set .@menu$, .@menu$+":";
					}
					if (.@dancer) break; 
				}
				set .@menu$, .@menu$+" ~ ^777777Cancel^000000";
				next;
				
				set .@i, select(.@menu$);
				if (.@i == getarraysize(.@classes)) close;
				mes .@n$;
				mes "Are you sure you want to purchase an entry for the ^0055FF"+jobname(.@classes[.@i])+" ^000000class?";
				next;
				if(select(" ~ I'm sure.: ~ No, wait...") == 2) {
					mes .@n$;
					mes "That's perfectly fine.";
					mes "Come back whenever you'd like.";
					close; }
				mes .@n$;
//				set Zeny, Zeny-.Mode[4];
				
				for (.@p = 1; .@p < getarraysize(.JobList); .@p++) {
					if (.JobList[.@p] == .@classes[.@i]) {
						.@i = .@p;
						break;
					}
				}
				set Class_Mastery, Class_Mastery | pow(2,.@i);
				specialeffect2 441;
				mes "There you go, all done!";
				message strcharinfo(0),jobname(.JobList[.@i])+" added to Class Manual.";
				close;
			} else {
   				mes "No, unfortunately there's";
   				mes "nothing I can do for you";
				mes "at this point.";
				mes "Please come back when you've";
				mes "reached Second Class";
				mes "maximum Job Level.";
				close;
			}
		case 3:
			close; }

OnTimer50:
	stopnpctimer;
	setnpctimer 0;
	end;

OnInit:
// --------------------- Config ---------------------
// Mode:
//	[0] Manual received automatically upon login? (1: yes / 0: no)
//	[1] Classes added automatically through leveling up? (1: yes / 0: no)
//	[2] Enable NPC to purchase manual/classes? (1: yes / 0: no)
//	[3] Price of manual, in Zeny (if NPC enabled; 0 to disable)
//	[4] Price per class, in Zeny (if NPC enabled; 0 to disable)
	setarray .Mode[0],0,0,1,50000,1000000;

// JobList: All the classes (ID) that are available.
// JLvlMax: The max job level of classes in JobList.
//	setarray .JobList[1],0, 4008,4009,4010,4011,4012,4013,4015,4016,4017,4018,4019,4020,4021,23,24,25,4046,4047,4049;
//	setarray .JLvlMax[1],10,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,99,70,70,  50,  50,  50;
	setarray .JobList[1], 7, 8, 9,10,11,12,14,15,16,17,18,19,20;
	setarray .JLvlMax[1],50,50,50,50,50,50,50,50,50,50,50,50,50;


	set .ManualID,3333;	// Item ID of manual
	set .CastTime,5;	// Seconds for class change
	set .Interrupt,1;	// Class change is interruptable? (1: yes / 0: no)
	set .BuildCount,12;	// Number of builds and Quick Change options (max 26)
	set .MaxLevel,$@MAXLEVEL;	// Maximum base level (to prevent stat overflow)
// --------------------------------------------------
//	if (!.Mode[2]) hideonnpc "CMSys";
	end;
}

function	script	Class_Mastery	{
function Class_Change; function	Get_Menu; function Save_Build; function	Load_Build; function	Save_Skills; function	Load_Skills; function	Get_Platinum;

	set .@allow, checkquest(69299,PLAYTIME);
	if (!Class_Mastery) { message strcharinfo(0),"There is nothing here that would apply to me now."; end; }
	if(.@allow == 0 || .@allow == 1) {
		message strcharinfo(0),"Hum... There is a minimum wait of 10 minutes between swapping classes. shit.";
		end;
	}
	while(1) {
		message strcharinfo(0)," ~ Class Mastery System ~ ";
		switch(select(" > Job Change: > ^777777Close^000000")) {
			case 1:
				message strcharinfo(0),"Job Change:";
				set .@index, Get_Menu(1,getvariableofnpc(.BuildCount,"CMSys"));
				if (.@index == 999) break;
				Save_Build;
				Save_Skills;
				Class_Change(.@index);
				Load_Build(getd("Build_"+getcharid(0) + "_" + Class + "$"));
				Load_Skills;
				Get_Platinum;
				if(checkquest(69299,PLAYTIME) == 2)
					erasequest(69299);
				setquest(69299);
				atcommand "@refresh";
				end;
			case 2:
				end; }
//		sleep2 250; 
	}



// **************************************************************************************************************
	function Class_Change {
		if (SkillPoint) {
			message strcharinfo(0), "I must have to distribute all my skill points first...";
			end;
		}

		// *****
		// Iniciando o processo de Mudanca de Classe
		// Dando um tempinho antes para o player repensar se eh mesmo isso que quer
		// Antes de fazer o backup da build atual no banco e realizar a mudanca de classe
		// *****
		specialeffect2 348;
		if (getvariableofnpc(.Interrupt,"CMSys")) {
			message strcharinfo(0),"Casting...";
			initnpctimer "CMSys",1;
			progressbar "", getvariableofnpc(.CastTime,"CMSys"); }
		else for(set .@i,0; .@i<getvariableofnpc(.CastTime,"CMSys"); set .@i,.@i+1) {
			message strcharinfo(0),(getvariableofnpc(.CastTime,"CMSys")-.@i)+" seconds remaining...";
			sleep2 1000; }
			
		jobchange getvariableofnpc(.JobList[getarg(0)],"CMSys");
		skilleffect 292,0;

		return; 
	}
// **************************************************************************************************************
	function Get_Menu {
		set .@menu$,"";
		switch(getarg(0)) {
		case 1:
			set .@p, 1;
			for(set .@i,1; .@i<=13; .@i++) { 
				if (Class_Mastery & pow(2,(.@i))) {	
					set .@menu$, .@menu$+" > ^0055FF"+jobname(getvariableofnpc(.JobList[.@i],"CMSys"))+"^000000";
					setarray .@classes[.@p], getvariableofnpc(.JobList[.@i],"CMSys");
//					debugmes ".@classes[.@p] recebeu " + getvariableofnpc(.JobList[.@i],"CMSys");
					.@p++;
					set .@menu$, .@menu$+":";
				}
			}
			set .@menu$, .@menu$+" > ^777777Back^000000";

			set .@i, select(.@menu$);

			
			if ( .@i == getarraysize(.@classes))
				 return 999;
				 
			for (.@p = 1; .@p <= 13; .@p++) {
				if (getvariableofnpc(.JobList[.@p],"CMSys") == .@classes[.@i]) {
//					debugmes "joblist na posicao p(" + .@p +") eh " + getvariableofnpc(.JobList[.@p],"CMSys") + " e .@classes na posicao i (" + .@i + ") eh " + .@classes[.@i];
					.@i = .@p;
					break; 
				} 
			}
			break; 
		}
		return .@i;
	}


	function Save_Build {
		message strcharinfo(0),"Build Manager > Saving current Status...";
		setarray .@ar$[0],"s","a","v","i","d","l","p";
		set .@s$,"";
		for(set .@i,0; .@i<7; set .@i,.@i+1)
			set .@s$,.@s$+.@ar$[.@i]+((.@i==6)?StatusPoint:readparam(.@i+13))+"|";
		set .@s$,.@s$+"j"+JobLevel+"|"; 
		set .@s$,.@s$+"e"+JobExp+"|";
		set .@s$,.@s$+"b"+BaseLevel+"|";
		
		setd("Build_"+getcharid(0) + "_" + Class + "$"), .@s$;
		message strcharinfo(0), jobname(Class) + "'s stats successfuly saved. ("+getd("Build_"+getcharid(0) + "_" + Class + "$")+").";
		
		return; }


	function Load_Build {
		ResetStatus;
		if(getarg(0) != "") {
			setarray .@num$[0],"0","1","2","3","4","5","6","7","8","9";
			setarray .@ar$[0],"s","a","v","i","d","l","p","j","e","b";
			for(set .@j,13; .@j<23; .@j++) {
				set .@var$,"";
				set .@s$,.@ar$[.@j-13];
				while(1) {
					for(set .@i,0; .@i<10; .@i++) {
						if (compare(getarg(0),.@s$+.@num$[.@i])) {
							set .@var$,.@var$+.@num$[.@i];
							set .@s$,.@s$+.@num$[.@i];
							break; }
					} if (compare(getarg(0),.@s$+"|")) break; 
				}
				if (.@j<19) statusup2 .@j, atoi(.@var$)-1; 
			//	if (.@j==19) set StatusPoint, atoi(.@var$);
				if (.@j==19) set .@stats, atoi(.@var$);
				if (.@j==20) set JobLevel, atoi(.@var$);
				if (.@j==21) set JobExp, atoi(.@var$);
				if (.@j==22) {
					debugmes ".@var$ esta com " + .@var$;
					if(atoi(.@var$) < BaseLevel)
						.@stats = $@sp[BaseLevel] - $@sp[atoi(.@var$)] + .@stats;
						debugmes "valor 1 : " + $@sp[BaseLevel] + " - valor 2 : " + $@sp[atoi(.@var$)] + " + stats : " + .@stats;
				}
				StatusPoint = .@stats;
			}
		}
		// *****
		// Repoe os Boosters de Status ganhados com quests e achievements
		// *****
		statusup2 bStr, BONUS_STR;
		statusup2 bAgi, BONUS_AGI;
		statusup2 bVit, BONUS_VIT;
		statusup2 bInt, BONUS_INT;
		statusup2 bDex, BONUS_DEX;
		statusup2 bLuk, BONUS_LUK;
		dispbottom "Stats bonuses from quests and achievements recovered...";
		
		return .@existent; 
	}
	
	function Save_Skills {
		message strcharinfo(0),"Build Manager > Saving current Skills...";		
		getskilllist;
		
		set .@s$,"";
		set .@l$,"";
		set .@f$,"";
		
		for(set .@i,0; .@i<@skilllist_count; .@i++) {
			set .@s$,.@s$+@skilllist_id[.@i]+"|";
			set .@l$,.@l$+@skilllist_lv[.@i]+"|";
			set .@f$,.@f$+@skilllist_flag[.@i]+"|";
		}
		setd("Skills_id_"+getcharid(0) + "_" + Class + "$"), .@s$;
		setd("Skills_lv_"+getcharid(0) + "_" + Class + "$"), .@l$;
		setd("Skills_flag_"+getcharid(0) + "_" + Class + "$"), .@f$;
		
		deletearray @skilllist_id;
		deletearray @skilllist_lv;
		deletearray @skilllist_flag;
		@skilllist_count = 0;
		
		debugmes "Salvando as skills da classe atual: (" + Jobname(Class) + ").";
		debugmes "Skills Salvas: " + getd("Skills_id_"+getcharid(0) + "_" + Class + "$");
		
		message strcharinfo(0), jobname(Class) + "'s skills successfuly saved.";
		
		return; 
	}
	
	function Load_Skills {
		if(getd("Skills_id_"+getcharid(0) + "_" + Class + "$") != "") {
			set .@skills, getstrlen(getd("Skills_id_"+getcharid(0) + "_" + Class + "$"));
			message strcharinfo(0), "Recovering skills for the current class...";
			debugmes "recuperado a arvore de skill: Skills_id_"+getcharid(0) + "_" + Class + "$";
			debugmes "recebido: " + getd("Skills_id_"+getcharid(0) + "_" + Class + "$");
//			debugmes ".@skills recebeu = " + .@skills;
			ResetSkill;
			SkillPoint = 0;
			set .@sid$, getd("Skills_id_"+getcharid(0) + "_" + Class + "$");
			set .@slv$, getd("Skills_lv_"+getcharid(0) + "_" + Class + "$");
			set .@sfl$, getd("Skills_flag_"+getcharid(0) + "_" + Class + "$");

			for(set .@i,0; .@i<.@skills; .@i++) {
				if (charat(.@sid$,.@i) != "|") {
					set .@var$,.@var$+charat(.@sid$,.@i);
				} else {
					setarray .@skill_id[.@p], atoi(.@var$);
					.@var$ = "";
					.@p++;
				} 
			}
			set .@var$,"";
			set .@p, 0;
			for(set .@i,0; .@i<.@skills; .@i++) {
				if (charat(.@slv$,.@i) != "|") {
					set .@var$,.@var$+charat(.@slv$,.@i);
				} else {
					.@skill_lv[.@p] = atoi(.@var$);
					.@var$ = "";
					.@p++;
				} 
			}
			set .@var$,"";
			set .@p, 0;
			for(set .@i,0; .@i<.@skills; .@i++) {
				if (charat(.@sfl$,.@i) != "|") {
					set .@var$,.@var$+charat(.@sfl$,.@i);
				} else {
					.@skill_fl[.@p] = atoi(.@var$);
					.@var$ = "";
					.@p++;
				} 
			}

			for (.@i=0; .@i <getarraysize(.@skill_id); .@i++)
				skill .@skill_id[.@i], .@skill_lv[.@i], 0; // Removido flag para testar se deleta a skill
		} else {
			ResetSkill;
			if(SkillPoint > 49)
				SkillPoint = 49;
		}
		message strcharinfo(0), "Skills/Skill Points recovered accordingly.";
		return; 
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
}