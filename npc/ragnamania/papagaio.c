-	script	PapagaioConf	-1,{
end;
OnInit:
	bindatcmd "papagaio",strnpcinfo(3)+"::OnAtcommand",99,99;
	end;
OnAtcommand:
Start:
	mes "[Papagaio]";
	if($pap$ == "1") mes "As mensagens estao ^0000FFLIGADAS^000000";
	if($pap$ == "0") mes "As mensagens estao ^ff0000DESLIGADAS^000000";
	if (getgmlevel() < 99) goto Nogm;
	mes "O que voce gostaria de fazer??";
		menu 
		"Ver Mensagem",view,
		"Adicionar Mensagem",add,
		"Apagar todas as mensagens",erase,
		"Anunciar agora",checkannouncer,
		"Ligar anuncios",liga,
		"Desligar Anuncios",desliga,
		"Sair",exit;

Nogm: 
	mes "Sinto muito, nao posso te ajudar....";
	mes "Somente GMs podem alterar as mensagens";
	close;

liga:
	set $pap$,1;
	close2;
	goto Start;


desliga:
	set $pap$,0;
	close2;
	goto Start;

checkannouncer:
	if($pap$ == "0") { 
		mes "Sinto muito mas nao posso anunciar quando as mensagens estao desligadas"; 
		close2; 
		goto Start;
	} else {
		goto announcer;
	}

erase:
	set $mensagem1$,"";
	set $mensagem2$,"";
	set $mensagem3$,"";
	set $mensagem4$,"";
	set $mensagem5$,"";
	set $mensagem6$,"";
	set $mensagem7$,"";
	set $mensagem8$,"";
	set $mensagem9$,"";
	set $mensagem10$,"";
	mes "Todas as mensagens foram apagadas";
	close2;
	goto Start;

view:
	next;
	mes "[Papagaio]";
	mes "Esta eh a mensagem atual...";
	mes " ";
	mes "^4444FF "+$mensagem1$+" "+$mensagem2$+" "+$mensagem3$+" "+$mensagem4$+" "+$mensagem5$+" "+$mensagem6$+" "+$mensagem7$+" "+$mensagem8$+" "+$mensagem9$+" "+$mensagem10$+" ^000000";
	close2;
	goto Start;
		

add:
	next;
	mes "[Papagaio]";
	mes "Qual mesagem gostaria de adicionar?";
	mes "Para desabilitar uma mensagem deixe-a em branco.";
	menu "Voltar",-,
		"Primeira",addpri,
		"Segunda",addseg,
		"Terceira",addter,
		"Quarta",addqua,
		"Quinta",addqui,
		"Sexta",addsex,
		"Setima",addset,
		"Oitava",addoit,
		"Nona",addnon,
		"Decima",adddec;
	next;
	goto Start;

	addpri:
		mes "Primeira mensagem";
		input $mensagem1$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addseg:
		mes "Segunda mensagem";
		input $mensagem2$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addter:
		mes "Terceira mensagem";
		input $mensagem3$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addqua:
		mes "Quarta mensagem";
		input $mensagem4$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addqui:
		mes "Quinta mensagem";
		input $mensagem5$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addsex:
		mes "Sexta mensagem";
		input $mensagem6$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addset:
		mes "Setima mensagem";
		input $mensagem7$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addoit:
		mes "Oitava mensagem";
		input $mensagem8$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	addnon:
		mes "Nona mensagem";
		input $mensagem9$;
		menu "Proxima",-,"Voltar",Start,"Sair",exit;
	adddec:
		mes "Decima mensagem";
		input $mensagem10$;
		menu "Voltar",Start,"Sair",exit;

exit:
	if($pap$ == "1") mes "Adorei esta ideia... vou repetir isso o tempo inteiro!!";
	if($pap$ == "0") mes "Ahhh, vou ter que ficar quieto???? Nao gostei...";
	close;

OnClock0002:
OnClock0032:
OnClock0102:
OnClock0132:
OnClock0202:
OnClock0232:
OnClock0320:
OnClock0332:
OnClock0402:
OnClock0432:
OnClock0502:
OnClock0532:
OnClock0602:
OnClock0632:
OnClock0702:
OnClock0732:
OnClock0802:
OnClock0832:
OnClock0902:
OnClock0932:
OnClock1002:
OnClock1132:
OnClock1202:
OnClock1232:
OnClock1302:
OnClock1332:
OnClock1402:
OnClock1432:
OnClock1502:
OnClock1632:
OnClock1702:
OnClock1732:
OnClock1802:
onClock1832:
OnClock1902:
OnClock1932:
OnClock2002:
OnClock2032:
OnClock2102:
OnClock2132:
OnClock2202:
OnClock2232:
OnClock2302:
OnClock2332:
announcer:
	announce $mensagem1$+" "+$mensagem2$+" "+$mensagem3$+" "+$mensagem4$+" "+$mensagem5$+" "+$mensagem6$+" "+$mensagem7$+" "+$mensagem8$+" "+$mensagem9$+" "+$mensagem10$,bc_all;
	end;
}