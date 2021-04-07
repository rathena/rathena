prontera,145,200,4	script	Negociador Ambulante#CoinsTrader	733,{
	mes .n$;
	mes "Cotacao do dia:";
	mes "Hunting Coin 1x : " +F_InsertComma(.CustoHuntingCoin)+ " Zeny";

	mes "Quantas "+ getitemname(675) +" voce desejaria?";
	next;
	mes .n$;
	input(.@qt);
	.@cost = .@qt * .CustoHuntingCoin;
	mes "----------------------";
	mes "Valor total: " + F_InsertComma(.@cost) + "z";
	mes "----------------------";
	if(Zeny < .@cost)
		callsub noBalance;
	mes "Deseja continuar?";
	if(select("Sim, por favor:Não, cancele") == 2) {
		mes "Sem problema!";
		close;
	} else {
		if(Zeny < .@cost) {
			dispbottom "Esta ação foi reportada a staff";
			logmes "Tentou cheatar na troca de moedas";
			close;
		}
		set Zeny, Zeny - .@cost;
		getitem 675, .@qt;
		mes "Prontinho, muito obrigado.";
		close;
	}

noBalance:
	mes "^FF0000Sem saldo suficiente^000000";
	close;


OnInit:
	.n$ = "[ Negociador Ambulante ]";
	.CustoHuntingCoin = 250000;
	.CustoZeny = 300000; 
	end;

}