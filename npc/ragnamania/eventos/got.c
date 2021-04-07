//Winter is Comming
//mobdemolition

-	script	configuracao#got	-1,{
end;
OnInit:
	// potencia das bombas.
	setarray $@got_bombas[0], 1500,3000,10000,50000;

	//- Quantidade de moedas
	setarray $@got_premio[0], 1,2,3,4,5,30;

end;
}


// -----------------------
// Canhoes
// -----------------------
muralha,159,49,0	script	Canhão#1	111,{
	.qtde = 5;
	.area = 10;
	.power = $@got_bombas[2];

	//mob_demolition(m,x,y,ratio,type,amount,power);
	mobdemolition "muralha",57,104,1,0,1,20000;

end;
}


muralha,147,103,0	script	canhao#2	111,{
end;
OnBoom:
end;
}

muralha,142,156,0	script	canhao#3	111,{
end;
OnBoom:
end;
}


//Debug
muralha,53,112,6	duplicate(Canhão#1)	Oi Eu sou um Canhao#1	412
