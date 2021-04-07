prontera,142,289,5	script	Amigo Ragnamania	97,{
	mes "[Amigo]";
	mes "É um prazer ter você brincando aqui conosco!";
	next;
	mes "Você já ouviu falar das ^0000FFContas de Doador^000000 do Ragnamania?";
	next;
	mes "Talvez você as conheça por ^FF0000Conta Premmy^000000, é assim que a maioria aqui as chama. ^^";
	next;
	menu "Sim, Já sou Parceiro Ragnamania.",SouParceiro,"Hum...Ainda não.",NaoConheco,"Não tenho interesse em ter uma.",-;
	mes"Ok! Seja bem vindo ao Ragnamania!!";
	close;	

	SouParceiro:
		if (getgmlevel() < 1) goto L_Enter;
		mes "[Amigo]";
		mes "Wow! Como pude não lhe reconhecer!";
		mes "Mil desculpas ^ff0000"+ strcharinfo(0) + "^000000,Prometo prestar mais atenção.";
		Next;
		warp "sala_premmy",41,38;
		end;
		L_Enter:
			mes "[Amigo]";
			mes "Hum... Não me lembro de você...";
			mes "Sugiro que entre em contato com meu amigo Biali, ou alguém da equipe de GMs... Pode estar havendo algum mal-entendido.";
			mes "Por favor, não nos leve a mal...Mas só posso deixar entrar quem tem o nome na minha lista... Desculpe-me.";
			close;

	NaoConheco:
		mes "[Amigo]";
		mes "Bem, o Ragnamania tem um custo fixo mensal para se manter online...";
		mes "Atualmente o Ragnamania conta com 3 servidores hosteados no exterior.";
		next;
		mes "[Amigo]";
		mes "Porém, o Ragnamania vem crescendo a cada dia! e como vovê deve saber,";
		mes "Manter um servidor com as proporções do Ragnamania custa algum dinheiro todo mês...";
		next;
		mes "[Amigo]";
		mes "Por isso, foi criado o sistema de ^0000ffContas de Doador^000000...";
		mes "Uma Conta de Doador te dá acesso à ^0000ffPremium Room^000000";
		mes "Uma sala desenvolvida especialmente para aqueles players que colaboram com o Ragnamania.";
		mes "Ou seja, se você realizar uma doação ao Ragnamania de pelo menos R$5,00";
		mes "O Ragnamania libera seu acesso à Premium Room por um mês como gratidão pela sua colaboração!!";
		next;
		mes "[Amigo]";
		mes "Na premium Room você encontrará NPCs especiais. Eles estão lá como uma forma de agradecimento àqueles que nos ajudam...";
		mes "Você ajuda o Ragnamania, e o Ragnamania ajuda você... Fantástico não?";
		next;
		mes "[Amigo]";
		mes "Está curioso pra saber quem te espera lá na Premium Room?!";
		next;
		mes "[Amigo]";
		mes "HUm... Infelizmente eu não posso te contar :( ";
		mes "Mas, posso dar uma dica...";
		next;
		mes "[Amigo]";
		mes "Você deve ter sentido a falta de alguns NPCs nesta nova versão do Ragnamania, não é?";
		mes "Pois é! Quem sabe eles não estão lá? hehehe";
		next;
		menu "Wow! Como faço pra ter uma Conta de Doador",CPremium,"Legal! Mas agora não posso ter uma...",close;
			CPremium:
				mes "[Amigo]";
				mes "Bem, o Ragnamania trabalha com depósito em conta...";
				mes "Como eu te disse anteriormente, com R$5,00 doados ao Ragnamania você já recebe sua Conta de Doador";
				next;
				mes "[Amigo]";
				mes "Ela leva em média 24 horas para ser ativada...";
				mes "Você dôa, comprova o depósito, e nós transformamos sua conta em uma Conta de Doador!";
				mes "Para tirar melhor suas dúvidas, visite nosso site: www.ragnamania.com.br";
				mes "Até Mais!";
				close;

}