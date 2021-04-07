-	script	Contestado	-1,{
	end;

OnClock1400:
OnClock1500:
OnClock1800:
OnClock1900:
OnClock2000:
OnClock2300:	
	// starting time checks
	if((gettime(DT_DAYOFWEEK) != SUNDAY) && (gettime(DT_HOUR)>=14 && gettime(DT_HOUR)<15) ||
		(gettime(DT_HOUR)>=19 && gettime(DT_HOUR)<20) ) {
		.RegiaoContestada = rand(1,6);
		.sz = getarraysize(getd(".Region"+.RegiaoContestada+"$"));
		callsub S_ContestadoStart;
		end;
	}

	// end time checks
	if ((gettime(DT_DAYOFWEEK) != SUNDAY) && (gettime(DT_HOUR)==15) || (gettime(DT_HOUR)==20)) { 
		callsub S_ContestadoEnd;
		end;
	}

	// termina com o periodo de bonus. termina o evento
	if ((gettime(DT_DAYOFWEEK) != SUNDAY) && (gettime(DT_HOUR)==18) || (gettime(DT_HOUR)==23)) { 
		callsub S_RemoveContestadoAntigo;
		end;
	}

S_RemoveContestadoAntigo:
	for(.@i=1;.@i<.sz;.@i++) {
		.@map$ = getelementofarray(getd(".Region"+.RegiaoContestada+"$"),.@i);
		removemapflag .@map$,mf_map_owner;
		removemapflag .@map$,mf_loadevent;
		removemapflag .@map$,mf_gvg_dungeon;		
	}
	announce "[ Guerra Territorial ] O " +getelementofarray(.Region$,.RegiaoContestada)+ " conquistou novamente sua independência",bc_all,0xffc300,0,32;
	return;

S_ContestadoStart:
	announce "[ Guerra Territorial ] : A Batalha pelo " + getelementofarray(.Region$,.RegiaoContestada) + " começou. ("+getelementofarray(.Mapa$,.RegiaoContestada)+")",bc_all,0xffc300,64;
	.ContestadoStatus = 1;
	for(.@i=1;.@i<.sz;.@i++) {
		.@map$ = getelementofarray(getd(".Region"+.RegiaoContestada+"$"),.@i);
		setmapflag .@map$,mf_gvg_dungeon;
		setmapflag .@map$,mf_loadevent;
		mapannounce .@map$,"[ Guerra Territorial ] Esta região esta sendo contestada pelos Clãs de Rune-Midgard",bc_map,0xffc300,0,32;
	}
	callsub SpawnDaPedra;
	return;

OnControl:
	announce "[ Guerra Territorial ] "+getelementofarray(.Region$,.RegiaoContestada)+" capturado pelo clã ["+getguildname(getcharid(2))+"].",bc_all,0x9ACD32,0,32;
	.DonoDoMapa = getcharid(2);
	//precisa setar senao a guild dona consegue atacar o emperium
	setmapflag strcharinfo(3),mf_map_owner,.DonoDoMapa;

	.owner = getattachedrid();
	addrid(1);
	showdigit 20,3;
	detachrid;
	attachrid(.owner);
	sleep 10000; //TODO se o kra deslogar? fode tudo! achar um jeito de rodar isso sem attached rid 
	callsub SpawnDaPedra;
	// sleep 120000; //debug
	// callsub S_ContestadoEnd; //debug
	end;

SpawnDaPedra:
	if(.ContestadoStatus == 1)
		monster .Mapa$[.RegiaoContestada],.X[.RegiaoContestada],.Y[.RegiaoContestada],.Region$[.RegiaoContestada],20403,1, strnpcinfo(0)+"::OnControl",2,AI_FLORA;
	return;

S_ContestadoEnd:
	killmonster .Mapa$[.RegiaoContestada], strnpcinfo(0)+"::OnControl";
	.ContestadoStatus = 0;
	for(.@i=1;.@i<.sz;.@i++) {
		.@map$ = getelementofarray(getd(".Region"+.RegiaoContestada+"$"),.@i);
		setmapflag .@map$,mf_map_owner,.DonoDoMapa;
	}
	announce "[ Guerra Territorial ] A batalha pelo " +getelementofarray(.Region$,.RegiaoContestada)+ " terminou com a vitória do clã " + getguildname(.DonoDoMapa),bc_all,0xffc300,0,20,FW_BOLD;
	 // sleep 180000; //debug
	 // callsub S_RemoveContestadoAntigo; //debug
	end;

OnPCLoadMapEvent:
	if(getmapflag(strcharinfo(3),mf_map_owner) == 0)
		announce "[ Guerra Territorial ] Esta região esta sendo contestada pelos Clãs de Rune-Midgard",bc_self,0xffc300,0,32;
	else
		announce "[ Guerra Territorial ] No momento esta região pertence ao clã " + getguildname(.DonoDoMapa),bc_self,0xffc300,0,32;
	end;

OnPCLoginEvent:
	if(.ContestadoStatus == 1)
		dispbottom "[ Guerra Territorial ] : A Batalha pelo " + getelementofarray(.Region$,.RegiaoContestada) + " está ativa",0xffc300;
	end;

OnNPCKillEvent:
	if(getcharid(2) > 0)
		if(getcharid(2) == getmapflag(strcharinfo(3),mf_map_owner)) 
			if(rand(1000) > 980)
				getitem 675,1;

	end;

OnPCKillEvent:
	.@charid = getcharid(0,rid2name(killedrid));
	// debugmes "charid : " + .@charid;
	// debugmes " meu charid : " + getcharid(0);
	if(getvar(KILLSICK,.@charid) > gettimetick(2)){
		dispbottom "Este personagem nao rende honra no momento.";
		end;
	}
	if(getmapflag(strcharinfo(3),MF_GVG_DUNGEON) == 0)
		end;

	set .@lvlCadaver, readparam(Baselevel,.@charid);
	.@lvDiff = BaseLevel - .@lvlCadaver;
	.@lvDiff = callfunc("abs",.@lvDiff);
	if(.@lvDiff > 20)
		end;

	getitem 7773,1; // War Badge
	attachrid(killedrid);
	set .@sick, gettimetick(2) + 300;
	set KILLSICK,.@sick;
	end;

OnInit:
	setarray .Region$[1],"Condado de Prontera","Condado de Geffen","Condado de Morroc","Condado de Payon","Condado de Yuno","Condado de Lighthalzen";
	setarray .Region1$[0],"prt_fild00","prt_fild01","prt_fild02","prt_fild03","prt_fild04","prt_fild05","prt_fild06","prt_fild07","prt_fild08","prt_fild09","prt_fild10","prt_fild11","prt_monk";
	setarray .Region2$[0],"gef_fild00","gef_fild01","gef_fild02","gef_fild03","gef_fild04","gef_fild05","gef_fild06","gef_fild07","gef_fild08","gef_fild09","gef_fild10","gef_fild11","gef_fild12","gef_fild13","gef_fild14","mjolnir_01","mjolnir_02","mjolnir_03","mjolnir_04","mjolnir_05","mjolnir_06","mjolnir_07","mjolnir_08","mjolnir_09","mjolnir_10","mjolnir_11","mjolnir_12";
	setarray .Region3$[0],"moc_fild01","moc_fild02","moc_fild03","moc_fild07","moc_fild11","moc_fild12","moc_fild13","moc_fild16","moc_fild17","moc_fild18","moc_fild19","moc_fild20","moc_fild21","moc_fild22","moc_ruins","cmd_fild01","cmd_fild02","cmd_fild03","cmd_fild04","cmd_fild05","cmd_fild06","cmd_fild07","cmd_fild08","cmd_fild09","um_fild01","um_fild02","um_fild03","um_fild04";
	setarray .Region4$[0],"pay_fild01","pay_fild02","pay_fild03","pay_fild04","pay_fild05","pay_fild06","pay_fild07","pay_fild08","pay_fild09","pay_fild10","pay_fild11","gon_fild01","ma_fild01","ma_fild02","bra_fild01","dew_fild01";
	setarray .Region5$[0],"yuno_fild01","yuno_fild02","yuno_fild03","yuno_fild04","yuno_fild05","yuno_fild06","yuno_fild07","yuno_fild08","yuno_fild09","yuno_fild10","yuno_fild11","yuno_fild12","hu_fild07","hu_fild05","hu_fild04","hu_fild01","hu_fild02","hu_fild03","hu_fild06","odin_tem01","odin_tem02","odin_tem03","ayo_fild01","ayo_fild02","ama_fild01","lou_fild01","xmas_fild01";
	setarray .Region6$[0],"ein_fild03","ein_fild04","lhz_fild02","lhz_fild03","ein_fild06","ein_fild07","ein_fild08","ein_fild09","ein_fild10","ra_fild01","ra_fild02","ra_fild03","ra_fild04","ra_fild05","ra_fild06","ra_fild07","ra_fild08","ra_fild09","ra_fild10","ra_fild11","ra_fild12","ra_fild13","ve_fild01","ve_fild02","ve_fild03","ve_fild04","ve_fild05","ve_fild06","ve_fild07","mosk_fild01","mosk_fild02";
	
	//Dados de cada Mapa onde esta a pedra
	setarray .Mapa$[1],"prt_fild05","gef_fild08","moc_fild18","pay_fild07","yuno_fild11","lhz_fild02";
	setarray .X[1],				179,		 191,		  239,		   108,		 	 210,		  151;
	setarray .Y[1],				301,		 175,		  211,	       260,		  	 237,		  247;

	.ContestadoStatus = 0;

	end;



// 	INSERT INTO `mob_skill_db2` (`MOB_ID`, `INFO`, `STATE`, `SKILL_ID`, `SKILL_LV`, `RATE`, `CASTTIME`, `DELAY`, `CANCELABLE`, `TARGET`, `CONDITION`, `CONDITION_VALUE`, `VAL1`, `VAL2`, `VAL3`, `VAL4`, `VAL5`, `EMOTION`, `CHAT`) VALUES
// (20403, 'Condado@NPC_SUMMONSLAVE', 'idle', 196, 3, 10000, 0, 600000, 'no', 'self', 'myhpltmaxrate', '80', 1899, 2, NULL, NULL, NULL, NULL, NULL),
// (20403, 'Condado@NPC_SUMMONSLAVE', 'idle', 196, 3, 10000, 0, 600000, 'no', 'self', 'myhpltmaxrate', '50', 1899, 2, 1830, 1, NULL, NULL, NULL);
// (20403, 'Condado@NPC_SUMMONSLAVE', 'idle', 196, 3, 10000, 0, 600000, 'no', 'self', 'myhpltmaxrate', '10', 1899, 3, 1830, 2, NULL, NULL, NULL);
}


