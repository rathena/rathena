//===============================================
function	script	ZombieRUN	{
//===============================================
// "Chronos: --- Apocalipse Zumbi ---";
// "Chronos: A cada minuto serao sumonados ondas de mortos-vivos,";
// "Chronos: Sua missao eh permanecer vivo ate o final.";
// "Chronos: Caso voce seja um dos vencedores, ganhara 3 " +getitemname(7859)+".";
// "Chronos: Caso nao venca, voce ganha apenas um "+getitemname(7859)+".";
// "Chronos: Serao 10 ondas de monstros. Nao ataque os monstros!!";
// "Chronos: Ou voce perdera HP e SP.";
// "Chronos: Ah! Ia esquecendo! Quando voce entrar no mapa, todos seus itens e equips serao postos no armazem";
// "Chronos: e carrinho, falcao e mount removidos assim como qualquer buff.";


//callfunc("ZombieRUN","Apocalipse Zumbi","silk_lair",196,187,0,10,1036,3,1867,1,1870,1);	break;

	.eventname$ = "Apocalipse Zumbi";
	.eventmap$ 	= "silk_lair";
	.minplayers = 5;
	.eventid	= getarg(0);
	
	if(callfunc("DE_WarmUp",.eventname$,.eventmap$,.minplayers,.eventid) > 0)
		end;

	setarray .@Mobs[0],1036,10,1867,2,1870,1;
	sleep 10000;
	for(.@wave=1; .@wave < 4; .@wave++) {
		if(.@wave < 4){
			 if(.@wave == 1)
			 	mapannounce .eventmap$,"PRIMEIRA ONDA!",bc_map,0x00AAFF,0,16;
			 else
				mapannounce .eventmap$, .@wave + "a ONDA!",bc_map,0x00AAFF,0,16;
		} else {
			mapannounce .eventmap$,"ÚLTIMA ONDA!",bc_map,0xFF0000,0,32;
		}
		sleep 1000;

		//Contador - Countdown
		for(.@i=5; .@i > 0;.@i--){
			mapannounce .eventmap$,.@i+"...",bc_map,0xFF0000;
			sleep 1000;
		}

		switch(.@wave) {
			case 1:
				areamonster .eventmap$,171,192,225,184,"--ja--",.@Mobs[0], .@Mobs[1],"ZombiePrize::OnKillZombie";
				areamonster .eventmap$,194,215,204,164,"--ja--",.@Mobs[0], .@Mobs[1],"ZombiePrize::OnKillZombie";
			break;
			case 2:
				areamonster .eventmap$,171,192,225,184,"--ja--",.@Mobs[2], .@Mobs[3],"ZombiePrize::OnKillZombie";
				areamonster .eventmap$,194,215,204,164,"--ja--",.@Mobs[2], .@Mobs[3],"ZombiePrize::OnKillZombie";
			break;
			case 3:
				areamonster .eventmap$,171,192,225,184,"--ja--",.@Mobs[4], .@Mobs[5],"ZombiePrize::OnKillZombie";
				areamonster .eventmap$,194,215,204,164,"--ja--",.@Mobs[4], .@Mobs[5],"ZombiePrize::OnKillZombie";
			break;
		}

		.@players = getmapusers(.eventmap$);
		if(!.@players){
			set .@wave,4;
		} else {
			mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Contamos com "+getmapusers(.eventmap$)+ ((.@players>1)?" jogadores":" jogador") + " no mapa.",bc_map,0x00AAFF;
			sleep 56000;
		}
	}

	if(getmapusers(.eventmap$) >= 1) 
		callfunc "DE_Rewards", .eventname$,.eventmap$,2;
	else
		callfunc "DE_Failed", .eventname$,.eventmap$;
	
	end;
}

silk_lair,1,1,4	script	ZombiePrize	-1,{
	end;

OnKillZombie:
	dispbottom "Feira de Fim-de-Mês: Não, não! Não ataque eles! Fuja!!!";
	sc_start SC_DECREASEAGI,SC_DECREASEAGI,300000,10;
	percentheal -20,-100;
	heal -5,0;
	end;
}