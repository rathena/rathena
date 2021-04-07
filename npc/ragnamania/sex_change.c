prontera,239,312,3	script	Damares	79,{
	function CheckItems;
	mes "[ Damares ]";
	mes "Argh! Você ouviu o que andam falando por ai?";
	mes "^0000AAMeninos vestem azul^000000,";
	mes "^fa50e9Meninas vestem rosa^000000.";
	next;
	mes "[ Damares ]";
	mes "HAHAHAHAHAHA!";
	mes "Quem será que falou tamanha babaquice?";
	emotion ET_QUESTION;
	next;
	mes "[ Damares ]";
	mes "Bem, nao interessa porque eu estou aqui para solucionar todos os nossos problemas!";
	next;
	mes "[ Damares ]";
	mes "Você está aqui a procura da troca de sexo?";
	if(select("Sim:Não")&2) close;
	next;
	mes "[ Damares ]";
	mes "Ma-ra-vi-lha!";
	mes "Você sabe que o procedimento é complicado né? E que para irmos a diante preciso ter certeza que você sabe mesmo o que quer.";
	next;
	mes "[ Damares ]";
	mes "Você fornece os produtos... anote aí, vamos precisar:";
	mes "10x "+getitemname(970);
	mes "50x "+getitemname(1059);
	mes "1x " +getitemname(7235);


	mes "1x "+(Sex?getitemname(7291):getitemname(7295));
	mes "1x "+(Sex?getitemname(1054):getitemname(7035));
	mes "5x "+(Sex?getitemname(7325):getitemname(7201));

	next;
	mes "[ Damares ]";
	mes "Anotou certinho?";
	mes "Lembrando que eu ajudo você a realizar seu sonho e unicamente você nao os outros personagens da conta ok?";
	mes "Deseja continuar?";
	if(select("Sim:Não")&2) close;
	next;
	if(CheckItems() == 1) {
		mes "Só um minutinho agora...";
		next;
		mes "* Sente uma agulhada na nádega esquerda *";
		mes "OUCH!";
		next;
		changecharsex;
		end;
	} else {
		mes "[ Damares ]";
		mes "Me desculpe mas você ainda não possui todos os itens que precisamos.";
		close;
	}

	function	CheckItems	{
		if(countitem(970) < 10 || countitem(1059) < 50 || countitem(7235) < 1) {
			return 0;
		}

		if(sex == 1) {
			if(countitem(7291) < 1 || countitem(1054) < 1 || countitem(7325) < 5) {
			// Virar Mulher
				return 0;
			}
		} else {
			if(countitem(7295) < 1 || countitem(7035) < 1 || countitem(7201) < 5) {
			// Virar Homem
				return 0;
			}
		}

		delitem 970,10;
		delitem 1059,50;
		delitem 7235,1;
		if(sex == 1) {
			delitem 7291,1;
			delitem 1054,1;
			delitem 7325,5;
		} else {
			delitem 7295,1;
			delitem 7035,1;
			delitem 7201,5;
		}
		return 1;
	}
}