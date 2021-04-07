//===============================================
function	script	PvpEventRUN	{
//===============================================
// "Chronos: --- Last Man Standing  ---";
// "Chronos: Esta eh uma arena PvP livre onde o ultimo sobrevivente sera o vencedor.";
// "Chronos: Ha um limite de 20 minutos e se nao tivermos um vencedor ate la o evento termina.";
// "Chronos: O vencedor ganha 3 " +getitemname(7859)+" e os participantes ganharao apenas 1.";
//callfunc("PvpEventRUN",<eventid>,<eventname>,<eventmap>,<minplayers>,<length>);
//"Last Man Standing","pvp_n_1-5",3,20

	.eventname$ = "Last Man Standing";
	.eventmap$ 	= "pvp_n_1-5";
	.minplayers = 2;
	.eventid	= getarg(0);
	.length		= 20;

	pvpoff .eventmap$;

	if(callfunc("DE_WarmUp",.eventname$,.eventmap$,.minplayers,.eventid) > 0)
		end;

	sleep 2000;
	mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Espalhem-se! O evento comeca em 10 segundos...",bc_map,0x00AAFF; 
	sleep 9000;
	for(set .@i,9; .@i > 0; .@i--){	
		mapannounce .eventmap$,""+.@i+"...",bc_map,0xFF0000;
		sleep 1000;	
	}
	pvpon .eventmap$;
	mapannounce .eventmap$,"FIGHT!",bc_map,0xFF0000;
	sleep 5000;
	for(.@PVP=0; .@PVP<.length; .@PVP++){
		if(getmapusers(.eventmap$) == 1) { 
			.@PVP = .@PVP + .length;
		} else if(getmapusers(.eventmap$) > 1) { 
			mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Temos ainda "+getmapusers(.eventmap$)+" participantes.",bc_map,0x00AAFF; 
			sleep 60000;
		} else {
			callfunc "DE_Failed",.eventname$,.eventmap$;
			end;
		}
	}
	pvpoff .eventmap$;

	// for(.@i = 0; .@i < getarraysize($@EventPlayers); .@i++){
	// 	if(strcharinfo(3,getcharid(0,rid2name($@EventPlayers[.@i]))) == .eventmap$){
	// 		attachrid($@EventPlayers[.@i]);
	// 		if(HP <= 0) {
	// 			warp "prontera",155,181;
	// 		}
	// 		detachrid;
	// 	}
	// }

	if(getmapusers(.eventmap$) == 1) {
		callfunc "DE_Rewards", .eventname$,.eventmap$,2; 
	} else {
		mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: O tempo se esgotou!",bc_map,0xFF0000;
		sleep 2000;
		callfunc "DE_Failed", .eventname$,.eventmap$;
		// announce "[ Feira de Fim-de-Mês ]: O evento "+ .eventname$ +" terminou sem um vencedor. Muito obrigado a todos que participaram!",bc_all,0x00AAFF;
		// set $@LastEvent,0;
		// deletearray $@EventPlayers[0],getarraysize($@EventPlayers);
	}

	end;
}

// pvp_n_1-5,1,1,1	script	pvpEventWarper	-1,{
// 	end;

// OnPCKillEvent:
// 	deletearray $@EventPlayers[inarray($@EventPlayers[0],killedrid)],1;
// 	warp "prontera",155,181, getcharid(0,rid2name(killedrid));
// 	end; 
// }


