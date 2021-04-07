//===============================================
function	script	ManHuntRUN	{
//===============================================
// "Chronos: --- Manhunt ---";
// "Chronos: O objetivo neste evento eh matar o alvo em 60 segundos.";
// "Chronos: Aquele que der o last hit no alvo, ganha o premio.";
// "Chronos: Mas se o alvo sobreviver, ele eh quem ganha o premio.";
// "Chronos: Se o player alvo morrer, ele sera mandado de volta para Prontera.";
// "Chronos: Mas se sobreviver, ganhara mais 1 " +getitemname(7859)+".";
// "Chronos: Cada vez que alguem mata o alvo tambem ganha 1 " +getitemname(7859)+".";
// "Chronos: Todos os participantes ganharao 1 " +getitemname(7859)+" ao entrar na arena.";
//callfunc("ManHuntRUN",<eventid>,<eventname>,<eventmap>,<length>,<minplayers>);
//"Manhunt","pvp_n_1-5",10,3

	.eventname$ = "Manhunt";
	.eventmap$ 	= "pvp_n_1-5";
	.minplayers = 2;
	.eventid	= getarg(0);
	.length		= 10;

	pvpoff .eventmap$;
	$@Target=1000;

	if(callfunc("DE_WarmUp",.eventname$,.eventmap$,.minplayers,.eventid) > 0)
		end;

	for(.@round=1; .@round < .length; .@round++) {

		if(getmapusers(.eventmap$) < .minplayers) { 

			mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Desculpa, mas não temos mais o mínimo de players necessários para continuar.",bc_map,0x00AAFF; 
			callfunc "DE_Failed", .eventname$,.eventmap$;
			end;
		} else {

			if(.@round > 1){ 
				mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: O próximo round começa em 10 segundos!",bc_map,0x00AAFF; 
				sleep 10000; 
			}

			mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: ROUND "+.@round,bc_map,0x00AAFF; 

			freeloop(1);
			while($@Target == 1000) {
				$@Target = rand(getarraysize($@EventPlayers));
				if(isloggedin($@EventPlayers[$@Target])) {
					if(strcharinfo(3,getcharid(0,rid2name($@EventPlayers[$@Target]))) != .eventmap$){
						$@Target = 1000;
					}
				}
			}
			freeloop(0);

			//.@passed = 0;
			mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Round "+.@round+" : Nosso alvo é "+rid2name($@EventPlayers[$@Target])+"!",bc_map,0xCC0000; 
			atcommand "#killable "+rid2name($@EventPlayers[$@Target]);
			atcommand "#size "+rid2name($@EventPlayers[$@Target])+" 2";
			sleep 60000;
			if($@Target != 1000) { // O alvo nao morreu e entao ganha o premio
				if(isloggedin($@EventPlayers[$@Target])) {
					if(strcharinfo(3,getcharid(0,rid2name($@EventPlayers[$@Target]))) == .eventmap$) {
						announce "Parabéns, "+strcharinfo(0,getcharid(0,rid2name($@EventPlayers[$@Target])))+"! Um dos vencedores do evento "+.eventname$+".",bc_all,0x00AA00;
						getitem 7859,1,$@EventPlayers[$@Target];
						//atcommand "#heal "+rid2name($@EventPlayers[$@Target])+"";
						set HP, 5000, getcharid(0,rid2name($@EventPlayers[$@Target]));
						atcommand "#killable "+rid2name($@EventPlayers[$@Target])+"";
						atcommand "#size "+rid2name($@EventPlayers[$@Target])+" 0";
						$@Target = 1000; // Em preparacao para o novo round
					}
				}
			}
		}
	}

	mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: O evento "+ .eventname$ +" terminou!",bc_map,0x00AAFF;
	sleep 3000;
	mapwarp .eventmap$,"prontera",155,181;
	announce "[ Feira de Fim-de-Mês ]: O evento "+ .eventname$ +" terminou. Muito obrigado a todos que participaram!",bc_all,0x00AAFF;
	set $@LastEvent,0;
	deletearray $@EventPlayers[0],getarraysize($@EventPlayers);

	end;
}