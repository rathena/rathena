//===============================================
function	script	1234RUN	{
//===============================================
// "Chronos: --- Dado Da Morte ---";
// "Chronos: Cada jogador escolhe uma caixa: 1, 2, 3 ou 4.";
// "Chronos: Comecaremos uma contagem regressiva de 10 ate 0.";
// "Chronos: Quando chegarmos a 0, um dado da morte com quadro lados sera lancado.";
// "Chronos: Aquele(s) que estiver(em) na caixa com o numero sorteado sera(o) eliminado";
// "Chronos: assim como qualquer um que esteja nas escadas ou na plataforma.";
// "Chronos: O numero de jogadores remanecentes sera anunciado";
// "Chronos: e o processo se repete ate que tenhamos um unico vencedor.";
// "Chronos: O vencedor ganha 3 " +getitemname(7859)+" e os perdedores ganharao apenas 1.";
// "Chronos: Seus itens serao enviados para o armazem e peco, falcao, carrinho e montaria removidos assim como todos os buffs.";
//callfunc("1234RUN",<eventID>,<eventname>,<eventmap>,<minplayers>,<winers>,<maxtime>);

	.eventname$ = "Dado Da Morte";
	.eventmap$ 	= "quiz_01";
	.minplayers = 5;
	.eventid	= getarg(0);
	.length		= 100;

	if(callfunc("DE_WarmUp",.eventname$,.eventmap$,.minplayers,.eventid) > 0)
		end;

	sleep 9000;
	freeloop(1);
	for( .@1234=0; .@1234<.length; .@1234++){
		for(.@i=10; .@i>0; .@i--) {
			mapannounce .eventmap$,.@i+"...",bc_map,0xFF0000;
			sleep 1000;	
		}
		set .@i,rand(1,4);
		mapannounce .eventmap$,"0...",bc_map,0xFF0000;
		mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Tchauzinho caixa "+.@i+"!",bc_blue|bc_map;
		switch(.@i){
			case 1:	areawarp .eventmap$,183,81,191,59,"prontera",155,181;	break;
			case 2:	areawarp .eventmap$,195,81,203,59,"prontera",155,181;	break;
			case 3:	areawarp .eventmap$,207,81,215,59,"prontera",155,181;	break;
			case 4:	areawarp .eventmap$,219,81,227,59,"prontera",155,181;	break;
		}
		areawarp .eventmap$,182,94,228,88,"prontera",155,181;	//Upper Area
		areawarp .eventmap$,185,87,188,82,"prontera",155,181;	//Stairs 1
		areawarp .eventmap$,197,87,200,82,"prontera",155,181;	//Stairs 2
		areawarp .eventmap$,209,87,212,82,"prontera",155,181;	//Stairs 3
		areawarp .eventmap$,221,87,224,82,"prontera",155,181;	//Stairs 4
		if(getmapusers(.eventmap$) == 1 ) {	
			.@1234 = .length + 1;
		} else if(getmapusers(.eventmap$) > 1) {
			sleep 3000;
			mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Restam "+getmapusers(.eventmap$)+" jogadores.",bc_map,0x00AAFF;
			sleep 3000;
			mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Um tempinho para quem quiser trocar de caixa...",bc_map,0x00AAFF;
			sleep 10000;
		} else {
			callfunc "DE_Failed", .eventname$,.eventmap$;
			end;
		}
	}
	freeloop(0);
	callfunc "DE_Rewards", .eventname$,.eventmap$,2;
	end;
}