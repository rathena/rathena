//##########################################################################
//By: ____                    _      __        __           ____    _____  #
//   |    \   ___ __    __ _ | |     \ \      / /_  _   _  / ___\  / ____| #
//   | ||  | / _ \\ \  / /| || |      \ \    / /| || \ | || | ___ | (___   #
//   | ||  ||  __/ \ \/ / | || |__     \ \/\/ / | ||  \| || |___|\ \___ \  #
// __|____/__\___|__\__/__|_||____|_____\_/\_/__|_||_|\__|_\_____/_____) | #
//|_____________________________________________________________________/  #
//                                                                         #
//##########################################################################
//                                                                         #
//== Dice Event                                                            #
//== Reformulado por Homer™                                                #
//##########################################################################
//                                                                         #
//                                                                         #
//                                                                         #                                                                         
//##########################################################################

-	script	Dice#announcer	-1,{

OnInit:
	//disablenpc "prtevent";
	atcommand "@hidenpc Dice#evnt1";
	end;

OnClock0600:
OnClock1200:
OnClock1800:
OnClock0000:
	if ($@dice_flag) end;
	atcommand "@hidenpc Dice#evnt1";
	announce "[Evento Dice] O evento será iniciado...",0;
	sleep2 10000;
	announce "[Evento Dice] Para aqueles que querem participar, utilize o comando @evento.",0;
	//enablenpc "prtevent";
	initnpctimer;
	set $@dice_flag, 1;
	end;

OnTimer30000:
	announce "[Evento Dice] 30 segundos...",0;
	end;
	
OnTimer50000:
	announce "[Evento Dice] 10 segundos...",0;
	end;
	
OnTimer60000:
	announce "[Evento Dice] O Evento Começou boa sorte a todos os participantes.",0;
	end;
	
OnTimer61000:
	//disablenpc "prtevent";
	donpcevent "Dice#evnt1::OnEnable";
	stopnpctimer;
	end;

}
	
//--------------------------------------------------
	
//prontera,155,223,0	warp	prtevent	2,2,quiz_01,204,90

-	script	DiceEvento#01	-1,{
OnNpcDiceEvent:
	if( $@dice_flag == 1 )
		warp "quiz_01",204,90;
	end;
}
	
//--------------------------------------------------

quiz_01,204,93,6	script	Dice#evnt1	715,{

//-------------------------------------------------

	mes "[Evento Dice]";
	mes "Por favor, diga o seu nome:";
	next;
	input .@name$;
	if(.@name$ != strcharinfo(0)) {
		mes "[Evento Dice]";
		mes "Você tem certeza que digitou o nome certo? O.O";
		close;
	}
	mes "[Evento Dice]";
	mes "Parabéns, você venceu!";	
	announce "[Evento Dice] Parabéns, temos um vencedor!! Seu nome é "+.@name$+".",0;
	atcommand "@hidenpc Dice#evnt1";
	getitem 7539,1;
	close2;
	warp "prontera",155,182;
	end;
	
OnEnable:
	mapannounce "quiz_01","[Evento Dice] Estamos prestes a iniciar o evento...",0;
	sleep2 10000;
	mapannounce "quiz_01","[Evento Dice] Boa Sorte a todos os participantes !",0;
	sleep2 10000;
	announce "[Evento Dice] O Acesso do evento está encerrado!",0;
	set $@dice_flag, 2;
	goto L_Start;
	end;
	
L_Start:
	if(getmapusers("quiz_01") == 1) goto L_Champ;
	if(getmapusers("quiz_01") == 0) goto L_None;
	if(getmapusers("quiz_01") > 1) {
	mapannounce "quiz_01","[Evento Dice] ...",0;
	set $@number, rand(1,4);
	sleep2 10000;
	mapannounce "quiz_01","[Evento Dice] Eu tenho um número. Por favor, escolha um... GO!",0;
	sleep2 10000;
	mapannounce "quiz_01","5.",0;
	sleep2 1000;
	mapannounce "quiz_01","4.",0;
	sleep2 1000;
	mapannounce "quiz_01","3.",0;
	sleep2 1000;
	mapannounce "quiz_01","2.",0;
	sleep2 1000;
	mapannounce "quiz_01","1.",0;
	sleep2 1000;
	mapannounce "quiz_01","[Evento Dice] Acabou o tempo.",0;
	donpcevent "evnt#1::OnEnable";
	mapannounce "quiz_01","[Evento Dice] O número perdedor é: "+$@number+".",0;
	if($@number == 1) goto L_Lose1;
	if($@number == 2) goto L_Lose2;
	if($@number == 3) goto L_Lose3;
	if($@number == 4) goto L_Lose4;
	set $@dice_flag, 0;
	end;
	}
	
L_Lose1:
	areawarp "quiz_01",183,81,191,59,"prontera",155,182;
	sleep2 1000;
	goto L_Start;
	end;

L_Lose2:
	areawarp "quiz_01",195,81,203,59,"prontera",155,182;
	sleep2 1000;
	goto L_Start;
	end;
	
L_Lose3:
	areawarp "quiz_01",207,81,215,59,"prontera",155,182;
	sleep2 1000;
	goto L_Start;
	end;
	
L_Lose4:
	areawarp "quiz_01",219,81,227,59,"prontera",155,182;
	sleep2 1000;
	goto L_Start;
	end;
	
L_Champ:
	mapannounce "quiz_01","[Evento Dice] Parabéns! Venha e me diga o seu nome...",0;
	atcommand "@shownpc Dice#evnt1";
	end;
	
L_None:
	announce "[Evento Dice] AH, que pena. Não tivemos vencedores :(",0;
	atcommand "@shownpc Dice#evnt1";
	end;
}
	
-	script	evnt#1	-1,{

OnEnable:
	areawarp "quiz_01",182,94,228,88,"prontera",155,182;
	areawarp "quiz_01",185,87,188,82,"prontera",155,182;
	areawarp "quiz_01",197,87,200,82,"prontera",155,182;
	areawarp "quiz_01",209,87,212,82,"prontera",155,182;
	areawarp "quiz_01",221,87,224,82,"prontera",155,182;
	end;
}


// Mapflags
quiz_01	mapflag	nomemo
quiz_01	mapflag	nowarp
quiz_01	mapflag	nowarpto
quiz_01	mapflag	noicewall
quiz_01	mapflag	nosave
quiz_01	mapflag	noteleport
quiz_01	mapflag	nopenalty
quiz_01	mapflag	nobranch
quiz_01	mapflag	nosave


// Mapflags
quiz_01	mapflag	nomemo
quiz_01	mapflag	nowarp
quiz_01	mapflag	nowarpto
quiz_01	mapflag	noicewall
quiz_01	mapflag	nosave
quiz_01	mapflag	noteleport
quiz_01	mapflag	nopenalty
quiz_01	mapflag	nobranch
quiz_01	mapflag	nosave	SavePoint