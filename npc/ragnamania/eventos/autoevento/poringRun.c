//===============================================
function	script	PoringRUN	{
//===============================================
// "Chronos: --- Poring Hunt ---";
// "Chronos: Seus itens serao enviados para o armazem e peco, falcao, carrinho e montaria removidos assim como todos os buffs.";
// "Chronos: As regras sao simples: Mate o 'Poring' com o nome certo para ser levado de volta a Prontera com seu premio.";
// "Chronos: Mate o poring errado e voce ganha apenas o premio de consolacao.";
// "Chronos: Os vencedores ganha 3 " +getitemname(7859)+" e os participantes ganharao apenas 1.";
// "Chronos: Existem 2 Porings com o nome certo right poring e um limite de 30 minutos para encontra-los.";
//callfunc("PoringRUN",<eventid>,<eventname>,<eventmap>,<minplayers>,<winers>,<length>,<mobID>,<fakeqnt>,<fakenames>);
//"Poring Hunt","pvp_n_1-5",1,2,30,1725,5,"Porjng|P0ring|Porlng|Porimg|Porinq|Poporing|This One|Right|Kill Me|Poning|qoring|Pooring|Pouring|Polling|~Poring~|Winner|Prize|Porirg|Paring"
	
	.eventname$ = "Poring Hunt";
	.eventmap$ 	= "pvp_n_1-5";
	.minplayers = 5;
	.eventid	= getarg(0);
	.length		= 30;

	explode($@Mobs$,"Porjng|P0ring|Porlng|Porimg|Porinq|Poporing|This One|Right|Kill Me|Poning|qoring|Pooring|Pouring|Polling|~Poring~|Winner|Prize|Porirg|Paring","|");
	pvpoff .eventmap$;
	setmapflag .eventmap$,mf_noskill;

	if(callfunc("DE_WarmUp",.eventname$,.eventmap$,.minplayers,.eventid) > 0)
		end;

	for(.@i=2; .@i < getarraysize($@Mobs$); .@i++) {
		monster .eventmap$,0,0,$@Mobs$[.@i],1725,5,"PoringPrize::OnKillWrong";
	}

	monster .eventmap$,0,0,"--ja--",1725,1,"PoringPrize::OnKillRight";
	for(.@poring=0; .@poring < .length; .@poring++) {
		if(getmapusers(.eventmap$) < 1) 
			.@poring = .@poring + .length;
		mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: Temos ainda "+(.length - .@poring)+" minutos restantes.",bc_map,0x00AAFF; 
		sleep 60000;
	}

	mapannounce .eventmap$,"[ Feira de Fim-de-Mês ]: O tempo se esgotou!",bc_map,0x00AAFF;
	removemapflag .eventmap$,mf_noskill;
	sleep 3000;
	callfunc "DE_Failed", .eventname$,.eventmap$;

	// killmonsterall .eventmap$;
	// sleep 3000;
	// mapwarp .eventmap$,"prontera",155,181;
	// announce "[ Feira de Fim-de-Mês ]: O evento "+ .eventname$ +" terminou. Muito obrigado a todos que participaram!",bc_all,0x00AAFF,0,16;
	// set $@LastEvent,0;
	// deletearray $@EventPlayers[0],getarraysize($@EventPlayers);

	end;
}

pvp_n_1-5,1,1,1	script	PoringPrize	-1,{
	end;

OnKillWrong:
	set .@i,rand(getarraysize($@Mobs$));
	monster "pvp_n_1-5",0,0,$@Mobs$[.@i],1725,5,"PoringPrize::OnKillWrong";
	dispbottom "Ouch! Por pouco! Infelizmente aquele não era o Poring certo.";
	deletearray $@EventPlayers[inarray($@EventPlayers[0],getcharid(3))],1;
	warp "prontera",155,181;
	end;

OnKillRight:
	announce "Feira de Fim-de-Mês: Parabéns, "+strcharinfo(0)+"! Um dos vencedores do evento Poring Hunt!!",bc_all,0x00AAFF,0,12;
	getitem 7859,2;
	warp "prontera",155,181;
	end;
}