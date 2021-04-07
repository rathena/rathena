-	script	Treasure Spawner	-1,{
	Onclock0030:
	Onclock0130:
	Onclock0230:
	Onclock0310:
	Onclock0430:
	Onclock0530:
	Onclock0630:
	Onclock0730:
	Onclock0830:
	Onclock0930:
	Onclock1030:
	Onclock1130:
	Onclock1230:
	Onclock1330:
	Onclock1430:
	Onclock1530:
	Onclock1630:
	Onclock1730:
	Onclock1830:
	Onclock1930:
	Onclock2030:
	Onclock2130:
	Onclock2230:
	Onclock2330:
switch ( rand(0,23) ) 

	{

	case 0: set $cityspawn$, "prontera"; set $cityname$, "Prontera"; 
		initnpctimer "Treasure Oracle";
		end;

	case 1: set $cityspawn$, "morocc"; set $cityname$, "Morroc"; 
		initnpctimer "Treasure Oracle";
		end;

	case 2: set $cityspawn$, "geffen"; set $cityname$, "Geffen"; 
		initnpctimer "Treasure Oracle";
		end;

	case 3: set $cityspawn$, "payon"; set $cityname$, "Payon"; 
		initnpctimer "Treasure Oracle";
		end;

	case 4: set $cityspawn$, "alberta"; set $cityname$, "Alberta"; 
		initnpctimer "Treasure Oracle";
		end;

	case 5: set $cityspawn$, "izlude"; set $cityname$, "Izlude"; 
		initnpctimer "Treasure Oracle";
		end;

	case 6: set $cityspawn$, "xmas"; set $cityname$, "Lutie"; 
		initnpctimer "Treasure Oracle";
		end;

	case 7: set $cityspawn$, "comodo"; set $cityname$, "Comodo"; 
		initnpctimer "Treasure Oracle";
		end;

	case 8: set $cityspawn$, "yuno"; set $cityname$, "Yuno"; 
		initnpctimer "Treasure Oracle";
		end;

	case 9: set $cityspawn$, "amatsu"; set $cityname$, "Amatsu"; 
		initnpctimer "Treasure Oracle";
		end;

	case 10: set $cityspawn$, "gonryun"; set $cityname$, "Gonryun"; 
		initnpctimer "Treasure Oracle";
		end;

	case 11: set $cityspawn$, "moscovia"; set $cityname$, "Moscovia"; 
		initnpctimer "Treasure Oracle";
		end;

	case 12: set $cityspawn$, "jawaii"; set $cityname$, "Jawaii"; 
		initnpctimer "Treasure Oracle";
		end;

	case 13: set $cityspawn$, "ayothaya"; set $cityname$, "Ayotaya"; 
		initnpctimer "Treasure Oracle";
		end;

	case 14: set $cityspawn$, "einbroch"; set $cityname$, "Einbroch"; 
		initnpctimer "Treasure Oracle";
		end;

	case 15: set $cityspawn$, "einbech"; set $cityname$, "Einbech"; 
		initnpctimer "Treasure Oracle";
		end;

	case 16: set $cityspawn$, "lighthalzen"; set $cityname$, "Lighthalzen"; 
		initnpctimer "Treasure Oracle";
		end;

	case 17: set $cityspawn$, "hugel"; set $cityname$, "Hugel"; 
		initnpctimer "Treasure Oracle";
		end;

	case 18: set $cityspawn$, "veins"; set $cityname$, "Veins"; 
		initnpctimer "Treasure Oracle";
		end;

	case 19: set $cityspawn$, "rachel"; set $cityname$, "Rachel"; 
		initnpctimer "Treasure Oracle";
		end;

	case 20: set $cityspawn$, "umbala"; set $cityname$, "Umbala"; 
		initnpctimer "Treasure Oracle";
		end;

	case 21: set $cityspawn$, "aldebaran"; set $cityname$, "Al De Baran"; 
		initnpctimer "Treasure Oracle";
		end;
	case 22: set $cityspawn$, "louyang"; set $cityname$, "Louyang"; 
		initnpctimer "Treasure Oracle";
		end;

	case 23:
		set $cityspawn$, "brasilis"; set $cityname$, "Brasilis"; 
		initnpctimer "Treasure Oracle";
		end;
	
		

	}

}

-	script	Treasure Oracle	-1,{

	
	OnTimer0001:
	set $noone,1;
	set $spawner,1;
	announce "[Evento]: Um raro Tesouro Asiático será arremessado em 30 segundos em "+$cityname$+".",bc_all,0xffffff;
	end;


	OnTimer30000:
	mapannounce ""+$cityspawn$+"","[Evento]: 2 Baús foram arremessados! Não se deixe levar pela ganância, apenas um deles contém o mistérioso Tesouro.",bc_all,0xffffff;
	monster $cityspawn$,0,0,"Baú do Tesouro",1798,1,"Treasure Oracle::OnBoxBreak";
	monster $cityspawn$,0,0,"Baú do Tesouro",601,1,"Treasure Oracle::OnBoxBreak2";
	end;
	
	OnTimer1800000:
	killmonsterall ""+$cityspawn$+"";
	set $noone,0;
	stopnpctimer;
	end;


OnBoxBreak:
    set $player_name$,strcharinfo(0);
 
 
	getmapxy @map$,@x,@y,0;
	set $noone,0;
	set $tesourowins$, strcharinfo(0);
	if (getcharid(2)){
	set $GIDBOX,getcharid(2);

	announce "O Verdadeiro Tesouro Asiático já foi encontrado e destruído por: ["+strcharinfo(0)+"] da guild ["+GetGuildName($GIDBOX)+"].",bc_all,0xffffff;
	killmonsterall ""+$cityspawn$+"";
	getitem 7558,1;
	set $GIDBOX2,1;
		set .ccoin,rand(1,10);
		if ( .ccoin == 1) { getitem 7539,1; end;}
		if ( .ccoin == 2) { getitem 7539,1; end;}
		if ( .ccoin == 3) { getitem 7539,1; end;}
		if ( .ccoin == 4) { getitem 7539,1; end;}
		if ( .ccoin == 5) { getitem 7539,1; end;}
		if ( .ccoin == 6) { getitem 7539,1; end;}
		if ( .ccoin == 7) { getitem 7539,1; end;}
		if ( .ccoin == 8) { getitem 7539,1; end;}
		if ( .ccoin == 9) { getitem 7539,1; end;}
		if ( .ccoin == 10) {getitem 7539,1; end;}
	}
	else
	    {

		announce "O Verdadeiro Tesouro Asiático já foi encontrado e destruído por: ["+strcharinfo(0)+"].",bc_all,0xffffff;
		killmonsterall ""+$cityspawn$+"";
		set $GIDBOX2,0;
		set .ccoin,rand(1,10);
		if ( .ccoin == 1) { getitem 7539,12103,1; end;}
		if ( .ccoin == 2) { getitem 7539,12103,1; end;}
		if ( .ccoin == 3) { getitem 7539,12103,1; end;}
		if ( .ccoin == 4) { getitem 7539,12103,1; end;}
		if ( .ccoin == 5) { getitem 7539,12103,1; end;}
		if ( .ccoin == 6) { getitem 7539,12103,1; end;}
		if ( .ccoin == 7) { getitem 7539,12103,1; end;}
		if ( .ccoin == 8) { getitem 7539,12103,1; end;}
		if ( .ccoin == 9) { getitem 7539,12103,1; end;}
		if ( .ccoin == 10) {getitem 7539,12103,1; end;}

    
  }
OnBoxBreak2:
	
	percentheal -99,-99;

	sleep2 2000;
	emotion e_what,1;
	dispbottom "Você encontrou o Baú errado.";	
	end;
}