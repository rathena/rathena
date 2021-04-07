//===== Athena Script ========================================
//= Poring Race System
//===== By: ==================================================
//= Zell_ff8
//=  -Made for Daegaladh, www.Sha-Ro.com Admin
//=  -and RUNE, http://ro-enhanced.net
//= Some Rights Reserved
//=     http://creativecommons.org/licenses/by-nc-sa/2.5/ar/
//===== Current Version: =====================================
//= 0.78
//===== Compatible With: =====================================
//= eAthena SVN 9991+ && Cronus SVN 1268+
//===== Description: =========================================
//= Poring Race
//===== Additional Comments: =================================
//= Tested, eA SVN Stable r10128 working 100%
//= Need a lot of clean up... but, WORKS!! WTH xD
//= Sorry to C&P the position check after a selection, it's a
//= fast fix and don't have time to make it a function.
//===== Mapflags: =============================================
p_track02	mapflag	nobranch
p_track02	mapflag	noicewall
p_track02	mapflag	nomemo
p_track02	mapflag	noreturn
p_track02	mapflag	noteleport
p_track02	mapflag	nowarpto	
p_track02	mapflag	nowarp
p_track02	mapflag	pvp	off
p_track02	mapflag	nosave
//====== Teleporte: ============================================
-	script	Poring Race#prace0	-1,{
	
	OnMenuCorrida:

	if(agitcheck() == 1) {mes "A Guerra do Emperium está acontecendo neste momento!"; close;}

	if($prace_gate == 0){
	mes "[Warper]";
	mes "Desculpe, mas a Corrida dos Porings está fechada,";
	mes "o evento ocorre as 5:25h, 11:25h, 17:25h e 23:25h!";
	close;
	}
	mes "[Warper]";
	mes "Olá "+StrCharInfo(0)+", você quer ir";
	mes "para a Corrida dos Porings?";
	if(select("Sim, claro.:Não, obrigado.") == 2) close;
	close2; warp "p_track02",55,41; end;
}
//====== Warps: ================================================
//hugel,45,57,0	warp	p_track001	2,2,p_track02,75,42
p_track02,78,42,0	warp	p_track002	1,3,prontera,156,191
//====== Checker: ==============================================
p_track02,44,41,0	script	Checker#prace0	-1,{
end;

OnChequeo:
	set .@prace_prize, 7720;		// ID do prêmio para quem vencer.[Padrão: ????]
	set .@prace_prize_quantity, 1; // Quantidade do prêmio.		 [Padrão: 1]

	set $prace_gate,0;
	for(set .@i,0; .@i < (getarraysize($prace_bidders)); set .@i, .@i + 1){
	if(attachrid($prace_bidders[.@i])){
	if (@prace_playing!=1) end;
	set @z, rand(150000,400000);
	dispbottom "O vencedor é o "+$prace_winner$+" e você apostou em "+@prace_winner$+".";
	if (@prace_winner$==$prace_winner$ && @prace_winner$!=""){  
   		dispbottom "Você venceu! Prêmio: 1 "+getitemname(@id); mapannounce "p_track02"," Parabéns! "+strcharinfo(0)+" venceu!",1,0xFFAB54;
   		getitem .@prace_prize,1; emotion 21,1; warp "prontera",150,170;
   	} else { dispbottom "Você perdeu."; emotion 28,1; warp "prontera",150,170; }
   		set @prace_winner$,"";set @prace_playing,0;
  	} //else { announce .@i+" || "+$prace_bidders[.@i],bc_all; }  //debug
} 	
	for(set .@i,0; .@i < (getarraysize($prace_bidders)); set .@i, .@i + 1) set $prace_bidders[.@i],0;
}

-	script	Timers#prace0	-1,{

	OnClock0100: callsub OnCalll;
	OnClock0700: callsub OnCalll;
	OnClock1300: callsub OnCalll;
	OnClock1900: callsub OnCalll;

	OnCalll:
		if(agitcheck() == 1) {end;}
		set $prace_gate,1;
		announce "A 'Corrida dos Porings' irá começar em breve, façam suas apostas!",bc_all|bc_yellow;
		end;
}
//====== Principal: ============================================
p_track02,58,41,2	script	Bidder#prace0	765,{ 

	set .@prace_zeny,3000;	// Preço para apostar. [Padrão: 3500]

	if ($prace_random < 1) callsub OnInit;
	if (@prace_playing==1) callsub AlreadyPlaying;

	if($prace_gate == 0){ mes "[Bidder]"; mes "Uma corrida está em andamento..."; close; }

	getmapxy(.@mapname$,.@x1,.@y,1,"Poring#prace1");
	getmapxy(.@mapname$,.@x2,.@y,1,"Angeling#prace2");
	getmapxy(.@mapname$,.@x3,.@y,1,"Metaling#prace3");
	getmapxy(.@mapname$,.@x4,.@y,1,"Deviling#prace4");
	getmapxy(.@mapname$,.@x5,.@y,1,"Poring Noel#prace5");
	getmapxy(.@mapname$,.@x6,.@y,1,"Poporing#prace6");
	if (.@x1 != 58 || .@x2 != 58 || .@x3 != 58 || .@x4 != 58 || .@x5 != 58 || .@x6 != 58){
	mes "[Bidder]"; mes "Uma corrida está em andamento..."; close;
	}
	mes "[Bidder]"; mes "Escolha o poring em que deseja apostar:";mes "Irá custar "+.@prace_zeny+" Zeny.";next;
	switch(select("Poring","Angeling","Metaling","Deviling","Poring Noel","Poporing")){
	case 1: callfunc "OnBid","Poring";
	case 2: callfunc "OnBid","Angeling";
	case 3: callfunc "OnBid","Metaling";
	case 4: callfunc "OnBid","Deviling";
	case 5: callfunc "OnBid","Poring Noel";
	case 6: callfunc "OnBid","Poporing";
	}

	OnReady:
		set $prace_bets,$prace_bets+1;
 		set $prace_bidders[$prace_bets],getcharid(3);
		set @prace_playing,1;
 		mes "[Bidder]";
 		mes "Eu tenho "+$prace_bets+" apostas.";
 		setnpctimer 60000;
		startnpctimer;
 		npctalk strcharinfo(0)+" fez sua aposta!";
 		close;

	Start1:
	setnpctimer 0;
	startnpctimer;
	end;
	
	AlreadyPlaying: mes "[Bidder]"; mes "Você apostou em ^00bb00"+@prace_winner$+"^000000."; close;
	
	StartRace:
		donpcevent "Metaling#prace3::OnRace";
		donpcevent "Poring#prace1::OnRace";
		donpcevent "Poporing#prace6::OnRace";
		donpcevent "Angeling#prace2::OnRace";
		donpcevent "Poring Noel#prace5::OnRace";
		donpcevent "Deviling#prace4::OnRace";
		return;

	OnStopRace:
		donpcevent "Poring#prace1::OnStop";
		donpcevent "Angeling#prace2::OnStop";
		donpcevent "Metaling#prace3::OnStop";
		donpcevent "Deviling#prace4::OnStop";
		donpcevent "Poring Noel#prace5::OnStop";
		donpcevent "Poporing#prace6::OnStop";
		if ($prace_winner$!="") callsub WinRace;
		end;

	ReturnRace:
		donpcevent "Poring#prace1::OnReturn";
		donpcevent "Angeling#prace2::OnReturn";
		donpcevent "Metaling#prace3::OnReturn";
		donpcevent "Deviling#prace4::OnReturn";
		donpcevent "Poring Noel#prace5::OnReturn";
		donpcevent "Poporing#prace6::OnReturn";
		end;

	WinRace:
		set $prace_gate,0;
		mapannounce "p_track02","O vencedor é o "+$prace_winner$+".",1,0xFFAB54;
		donpcevent "Checker#prace0::OnChequeo";
		setnpctimer 30000;startnpctimer;
		end;

	OnInit:
		set $prace_random,70;
		set $prace_random2,600;
		set $prace_winner$,"";
		set $prace_bets,0;
		set $prace_bidders,0;
		end;
 
	InitRace:
	mapannounce "p_track02","Porings, em suas marcas...",1,0xFFAB54;
	sleep(2000);
	mapannounce "p_track02","...3...",1,0xFFAB54;
	sleep(1000);
	mapannounce "p_track02","...2...",1,0xFFAB54;
	sleep(1000);
	mapannounce "p_track02","...1...",1,0xFFAB54;
	sleep(1000);
	callsub StartRace;
	mapannounce "p_track02","Gooo!!!",1,0xFFAB54;
	set $prace_winner$,"";
	set $prace_bets,0;
	callsub ReturnRace;
	
	OnTimer90000: npctalk "Eu tenho "+$prace_bets+" aposta(s). Alguém mais?"; end;
	OnTimer110000: npctalk "A corrida irá começar em breve. Última chance."; end;
	OnTimer120000: callsub InitRace;
}

//====== Função Apostar: =======================================
function	script	OnBid	{

	getmapxy(.@mapname$,.@x1,.@y,1,"Poring#prace1");
	getmapxy(.@mapname$,.@x2,.@y,1,"Angeling#prace2");
	getmapxy(.@mapname$,.@x3,.@y,1,"Metaling#prace3");
	getmapxy(.@mapname$,.@x4,.@y,1,"Deviling#prace4");
	getmapxy(.@mapname$,.@x5,.@y,1,"Poring Noel#prace5");
	getmapxy(.@mapname$,.@x6,.@y,1,"Poporing#prace6");
	if (.@x1 == 58 || .@x2 == 58 || .@x3 == 58 || .@x4 == 58 || .@x5 == 58 || .@x6 == 58){
	if (Zeny < .@prace_zeny) { callsub OnZeny; } else { set Zeny,Zeny-.@prace_zeny; }
 	set @prace_winner$,getarg(0); callsub OnReady;
	}
	mes "[Bidder]"; mes "Uma corrida está em andamento..."; close;

	OnZeny: set @prace_winner$,""; mes "[Bidder]"; mes "Você não tem Zeny suficiente."; close;

	OnReady: set $prace_bets,$prace_bets+1; set $prace_bidders[$prace_bets],getcharid(3);
		set @prace_playing,1; mes "[Bidder]"; mes "Eu tenho "+$prace_bets+" apostas.";
 		setnpctimer 60000; startnpctimer; npctalk strcharinfo(0)+" fez sua aposta!"; close;
}
//====== Monstros: ==============================================
p_track02,58,38,2	script	Poring#prace1	1002,{ 
end;
	OnRace: initnpctimer; startnpctimer; end;
	OnStop: stopnpctimer; end;
	OnReturn: npcwalkto 58,38; end;
	OnTimer1100:
		getmapxy(.@mapname$,.@x,.@y,1,"Poring#prace1");
		if(rand(100) < $prace_random) npcwalkto .@x-1,.@y;
 		setnpctimer rand($prace_random2); startnpctimer;
 		if ((.@x-1) == 29) { set $prace_winner$,"Poring"; emotion 29; donpcevent "Bidder#prace0::OnStopRace"; }
		setnpctimer 0;
		initnpctimer;
}
p_track02,58,36,2	script	Angeling#prace2	1096,{
end;
	OnRace: initnpctimer; startnpctimer; end;
	OnStop: stopnpctimer; end;
	OnReturn: npcwalkto 58,36; end;
	OnTimer1100:
		getmapxy(.@mapname$,.@x,.@y,1,"Angeling#prace2");
		if(rand(100) < $prace_random) npcwalkto .@x-1,.@y;
 		setnpctimer rand($prace_random2); startnpctimer;
 		if ((.@x-1) == 29) { set $prace_winner$,"Angeling"; emotion 29; donpcevent "Bidder#prace0::OnStopRace"; }
		setnpctimer 0;
		initnpctimer;
}
p_track02,58,34,2	script	Metaling#prace3	1613,{
end;
	OnRace: initnpctimer; startnpctimer; end;
	OnStop: stopnpctimer; end;
	OnReturn: npcwalkto 58,34; end;
	OnTimer1100:
		getmapxy(.@mapname$,.@x,.@y,1,"Metaling#prace3");
		if(rand(100) < $prace_random) npcwalkto .@x-1,.@y;
 		setnpctimer rand($prace_random2); startnpctimer;
 		if ((.@x-1) == 29) { set $prace_winner$,"Metaling"; emotion 29; donpcevent "Bidder#prace0::OnStopRace"; }
		setnpctimer 0;
		initnpctimer;
}
p_track02,58,32,2	script	Deviling#prace4	1582,{
end;
	OnRace: initnpctimer; startnpctimer; end;
	OnStop: stopnpctimer; end;
	OnReturn: npcwalkto 58,32; end;
	OnTimer1100:
		getmapxy(.@mapname$,.@x,.@y,1,"Deviling#prace4");
		if(rand(100) < $prace_random) npcwalkto .@x-1,.@y;
 		setnpctimer rand($prace_random2); startnpctimer;
 		if ((.@x-1) == 29) { set $prace_winner$,"Deviling"; emotion 29; donpcevent "Bidder#prace0::OnStopRace"; }
		setnpctimer 0;
		initnpctimer;
}
p_track02,58,30,2	script	Poring Noel#prace5	1062,{
end;
	OnRace: initnpctimer; startnpctimer; end;
	OnStop: stopnpctimer; end;
	OnReturn: npcwalkto 58,30; end;
	OnTimer1100:
		getmapxy(.@mapname$,.@x,.@y,1,"Poring Noel#prace5");
		if(rand(100) < $prace_random) npcwalkto .@x-1,.@y;
 		setnpctimer rand($prace_random2); startnpctimer;
 		if ((.@x-1) == 29) { set $prace_winner$,"Poring Noel"; emotion 29; donpcevent "Bidder#prace0::OnStopRace"; }
		setnpctimer 0;
		initnpctimer;
}
p_track02,58,28,2	script	Poporing#prace6	1031,{
end;
	OnRace: initnpctimer; startnpctimer; end;
	OnStop: stopnpctimer; end;
	OnReturn: npcwalkto 58,28; end;
	OnTimer1100:
		getmapxy(.@mapname$,.@x,.@y,1,"Poporing#prace6");
		if(rand(100) < $prace_random) npcwalkto .@x-1,.@y;
 		setnpctimer rand($prace_random2); startnpctimer;
 		if ((.@x-1) == 29) { set $prace_winner$,"Poporing"; emotion 29; donpcevent "Bidder#prace0::OnStopRace"; }
		setnpctimer 0;
		initnpctimer;
}