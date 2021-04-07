// WoE_Championship
// By Biali

prontera,144,219,6	duplicate(OutsideFlagsPR2)	WoE Championship#3	722
prontera,152,219,6	duplicate(OutsideFlagsPR3)	WoE Championship#2	722
prontera,159,219,6	duplicate(OutsideFlagsPR4)	WoE Championship#1	722
prontera,168,219,6	duplicate(OutsideFlagsPR5)	WoE Championship#4	722

// Information NPC
//============================================================
prontera,139,227,4	script	WOE Championship	835,{
	doevent "WOE_CH_CONTROL::OnMenu";
	end;
OnAgitStart:
	enablenpc "prtg-2_prtg-2";
	enablenpc "prtg-3_prtg-3";
	enablenpc "prtg-4_prtg-4";
	enablenpc "prtg-5_prtg-5";

	while(agitcheck()) {
		specialeffect EF_BEGINSPELL6;
		sleep 425;
	}
	end;
	
OnAgitEnd:
OnInit:
	disablenpc "prtg-2_prtg-2";
	disablenpc "prtg-3_prtg-3";
	disablenpc "prtg-4_prtg-4";
	disablenpc "prtg-5_prtg-5";
	end;
}

// Script Core
//============================================================
-	script	WOE_CH_CONTROL	-1,{
function Disp_Owner; function Add_Zero;

OnInit:

// -----------------------------------------------------------
//  Configuration settings.
// -----------------------------------------------------------

	set .CastleWarp,0;		// 1: Always enable all castle warps. | 0: Warp only to active castles.
	set .AutoKick,1;		// Automatically kick players from inactive castles during WOE? (1:yes / 0:no)
	set .NoOwner,1; 		// Automatically kick players from unconquered castles outside of WOE? (1:yes / 0:no)
	set .GMAccess,99;		// GM level required to access Session Manager.


// -----------------------------------------------------------
//  Constants (leave this section alone).
// -----------------------------------------------------------

	setarray .Castles$[0],
		"prtg_cas02","prtg_cas03","prtg_cas04","prtg_cas05";
	setarray .Days$[0],"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday";
	setarray .Regions$[0],"Prontera";
	setarray .Map$[0],"prt_gld";
	setarray .MapX[0],240,153,111,208;
	setarray .MapY[0],128,137,240,240;

// -----------------------------------------------------------

//	set .Size, getarraysize($WOE_CH_CONTROL);
	if (.AutoKick || .NoOwner)
		for(.@i=0; .@i<5; .@i++) {
			setmapflag .Castles$[.@i], mf_loadevent;
			setd "."+.Castles$[.@i], .@i;
		}
	if (!agitcheck()) sleep 4000;
	set .Init,1;

OnMinute00:
//	freeloop(1);
	if (agitcheck()) {
		// Checa se eh hora de encerrar o Evento // Sabado as 18 horas
		if (gettime(4) == 6 && gettime(3) == 18) {
		OnWOEEnd:
			if ($WOE_CH_RODADA == 3) {
				announce "Encerrou-se a " + $WOE_CH_EDICAO + "a edicao do WoE Championship Ragnamania!",bc_all|bc_woe;
				$WOE_CH_RODADA = 0;
				initnpctimer;
				// 7 dias de fogos nas cidades;
				setmapflag "prontera", mf_fireworks, 1;
				setmapflag "izlude", mf_fireworks, 1;
				setmapflag "alberta", mf_fireworks, 1;
				setmapflag "gefen", mf_fireworks, 1;
				setmapflag "aldebaran", mf_fireworks, 1;
				setmapflag "morocc", mf_fireworks, 1;
				setmapflag "yuno", mf_fireworks, 1;
				setmapflag "lighthalzen", mf_fireworks, 1;
				setmapflag "hugel", mf_fireworks, 1;
				setmapflag "ayothaya", mf_fireworks, 1;
			} else if($WOE_CH_RODADA < 3) {
				announce "Encerrou-se a "+ $WOE_CH_RODADA +"a rodada do WoE Championship Ragnamania!",bc_all|bc_woe;
			}
			AgitEnd;
			$WOE_CH_RODADA_CD = gettimetick(2) + (60 * 60 * 24 * 6); // 6 dias de cooldown so pra garantir

			sleep 1000;
			for(.@j=0; .@j<5; .@j++) {
				Disp_Owner(.Castles$[.@j],1);
				maprespawnguildid .Castles$[.@j],0,3;
			}
		}
//		break;
	}
	if (!agitcheck() || .Init) {
		if (!agitcheck()) 
			set .Init,0;
			
		// Checa o horario de termino do Evento // Sabados das 16 as 18 horas
		if (gettime(4) == 6 && gettime(3) >= 16 && gettime(3) < 18) {
			
		// Define as Seasons e checa se eh comeco de mes para iniciar uma nova Season.
		if (gettime(5) <= 7) {
			if ($WOE_CH_LASTSEASON < gettimetick(2))
				$WOE_CH_LASTSEASON = gettimetick(2) + (60 * 60 * 24 * 22);
				$WOE_CH_EDICAO++;

				// Esvazia os Castelos e suas variaveis
				for(.@c=0; .@c<getarraysize(.Castles$); .@c++) {
					SetCastleData .Castles$[.@c],1,0;	// Guild ID
					SetCastleData .Castles$[.@c],2,0;	// Economia
					SetCastleData .Castles$[.@c],3,0;	// Defesa
				}
			}
		}
					
		if (.Init) { 
			AgitEnd; 
		} else {
			if($WOE_CH_RODADA_CD < gettimetick(2)) 
				$WOE_CH_RODADA++;
			announce "Comecou a " + $WOE_CH_RODADA+ "a rodada do WoE Championship Ragnamania!",bc_all|bc_woe;
		}

		sleep 1000;
		AgitStart;
		
		switch ($WOE_CH_RODADA) {
		case 1:
			for(.@j=0; .@j<5; .@j++) {
				if (!.Init) 
					Disp_Owner(.Castles$[.@j],0);
			}
		case 2:
			for(.@j=0; .@j<5; .@j++) {
				if (!.Init) 
					Disp_Owner(.Castles$[.@j],0);

				donpcevent "Agit#"+.Castles$[.@j]+"::OnAgitEnd";
				killmonster .Castles$[.@j], "Agit#"+.Castles$[.@j]+"::OnAgitBreak";
			}
		}
//		break;
	}
	set .Init,0;
	end;

OnTimer604800000: // Remove 7 dias de fogos nas cidades;
	setmapflag "prontera", mf_fireworks, 0;
	setmapflag "izlude", mf_fireworks, 0;
	setmapflag "gefen", mf_fireworks, 0;
	setmapflag "alberta", mf_fireworks, 0;
	setmapflag "aldebaran", mf_fireworks, 0;
	setmapflag "morocc", mf_fireworks, 0;
	setmapflag "yuno", mf_fireworks, 0;
	setmapflag "lighthalzen", mf_fireworks, 0;
	setmapflag "hugel", mf_fireworks, 0;
	setmapflag "ayothaya", mf_fireworks, 0;
	stopnpctimer;
	end;

function Disp_Owner {
	set .@o, getcastledata(getarg(0),1);
	if (.@o) announce "O Castelo ["+getcastlename(getarg(0))+"] "+((getarg(1))?"foi conquistado pelo cla":"pertence ao cla")+" ["+getguildname(.@o)+"].",bc_all|bc_woe;
	else announce "O Castelo ["+getcastlename(getarg(0))+"] encontra-se vazio no momento.",bc_all|bc_woe;
	return;
}
function Add_Zero {
	return ((getarg(0)<10)?"0":"")+getarg(0)+(getarg(1,0)?".":":")+"00";
}

OnPCLoadMapEvent:
	if (!compare(strcharinfo(3),"g_cas")) end;
	if (((.AutoKick && .Active[0]) || (.NoOwner && !getcastledata(strcharinfo(3),1))) && !(.Active[0]&(1<<getd("."+strcharinfo(3))))) {
		if (getcharid(2) && getcastledata(strcharinfo(3),1) == getcharid(2)) end;
		sleep2 1000;
		message strcharinfo(0), getcastlename(strcharinfo(3))+" esta inativo.";
		sleep2 5000;
		if (compare(strcharinfo(3),"g_cas")) warp "SavePoint",0,0;
	}
	end;
}