//===============================================
function	script	MazeRUN	{
//===============================================
// "Chronos: --- Labirinto Maldito  ---";
// "Chronos: Seus itens serao enviados para o armazem e peco, falcao, carrinho e montaria removidos assim como todos os buffs.";
// "Chronos: As regras sao simples: O primeiro a encontrar e tocar no meu ajudante, ganha.";
// "Chronos: Mas para isso voce precisara atravessar um labirinto insano!";
// "Chronos: O vencedor ganha 3 " +getitemname(7859)+" e os participantes ganharao apenas 1.";
//callfunc("MazeRUN",<eventid>,<eventname>,<eventmap>,<minplayers>,<length>);
//"Labirinto Maldito","maze",1,30

	.eventname$ = "Labirinto Maldito";
	.eventmap$ 	= "maze";
	.minplayers = 3;
	.eventid	= getarg(0);
	.length		= 30;

	if(callfunc("DE_WarmUp",.eventname$,.eventmap$,.minplayers,.eventid) > 0)
		end;

	disablenpc "mazeroadblock";
	enablenpc "Ajudante#Maze";
	set $@mazetouched,0;

	for(.@maze=0; .@maze < .length; .@maze++){
		if(getmapusers(.eventmap$) < 1) 
			.@maze = .@maze + .length;
		mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: "+(.length - .@maze)+" minutos restantes",bc_map,0x00AAFF; 
		sleep 60000;
	}

	mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: O tempo se esgotou!",bc_map,0xFF0000;
	sleep 3000;
	mapwarp .eventmap$,"prontera",155,181;
	callfunc "DE_Failed", .eventname$,.eventmap$;
	// announce "[ Feira de Fim-de-Mês ]: O evento "+ .eventname$ +" terminou. Obrigado a todos que participaram!",bc_all,0x00AAFF,0,16;
	// set $@LastEvent,0;
	// deletearray $@EventPlayers[0],getarraysize($@EventPlayers);

	disablenpc "Ajudante#Maze";
	end;
}

maze,358,148,0	warp	mazeroadblock	2,2,maze,371,149


maze,369,162,4	script	Ajudante#Maze	405,4,4,{
	npctalk "Venha, rápido! Você precisa vir ate aqui para ganhar!";
	end;

OnTouch:
	if($@mazetouched) 
		end;

	set $@mazetouched,1;
	callfunc "DE_Rewards", .eventname$,.eventmap$,2;

	// announce "Feira de Fim-de-Mês: Parabens, "+strcharinfo(0)+"! Vencedor do evento Labirinto Maldito!!",bc_all,0x00AA00,0,12;
	// getitem 7859,2;
	// set $@LastEvent,0;
	// deletearray $@EventPlayers[0],getarraysize($@EventPlayers);
	// mapannounce "maze","Feira de Fim-de-Mês: O evento Labirinto Maldito terminou! Muito obrigado a todos que participaram!!",bc_map,0x00AAFF,0,16;
	// sleep 3000;
	// mapwarp "maze","prontera",155,181;
	// announce "Feira de Fim-de-Mês: O evento Labirinto Maldito terminou. O proximo evento sera em aproximadamente uma hora.",bc_all,0x00AAFF,0,16;
	end;

OnInit:
	.eventname$ = "Labirinto Maldito";
	.eventmap$ 	= "maze";
	end;
}

