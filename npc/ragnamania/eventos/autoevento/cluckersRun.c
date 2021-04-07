//===============================================
function	script	CluckersRun	{
//===============================================
//callfunc("CluckersRun",<eventid>,<eventname>,<Length>);
//,"Clucky Cluckers",20

	.eventname$ = "Clucky Cluckers";
	.eventmap$ 	= "prontera";
	.minplayers = 0;
	.eventid	= getarg(0);
	.length		= 40;

	if(callfunc("DE_WarmUp",.eventname$,"",.minplayers,.eventid) > 0)
		end;

	// set $@CurrentEvent,getarg(0,7);
	// for(.@i=5; .@i > 0; .@i--){
	// 	announce "[ Feira de Fim-de-Mês ]: O evento "+getarg(1)+" comecara em "+.@i+ " " + ((.@i > 1)?"minutos":"minuto")+".",bc_all,0x00AAFF,0,16;
	// 	announce "[ Feira de Fim-de-Mês ]: Para participar, basta digitar agora @joinevent",8;
	// 	sleep 60000;
	// }
	// announce "[ Feira de Fim-de-Mês ]: O evento "+getarg(1)+" comecou! Boa sorte a todos!",bc_all,0x00AAFF,0,16;
	// set $@LastEvent,$@CurrentEvent;
	// set $@CurrentEvent,0;
 //    end;
}


prontera,155,227,4	script	Clucky	800,{
	if(inarray($@EventPlayers,getcharid(3)) == -1) {
		dispbottom "[Feira de Fim-de-Mês] Você não está participando do evento.";
		end;
	}
	if ($@LastEvent == 7) {
		specialeffect2 EF_HIT3;
		switch(rand(15)) {
			case 0: npctalk "CLUUUUUUCK!!!"; unitkill getcharid(3); skilleffect "NPC_SELFDESTRUCTION",1; emotion ET_HUK; break;
			case 1: npctalk "Cluuuuuck!~"; break;
			case 2: unitkill getcharid(3); skilleffect "NPC_SELFDESTRUCTION",1; break;
			case 3: sc_start SC_Freeze,10000,0; break;
			case 4: npctalk "CLUUUUUUUUUCK!!!"; unitkill getcharid(3); skilleffect "NPC_SELFDESTRUCTION",1; emotion ET_HUK; break;
			case 5: sc_start SC_Sleep,10000,0; break;
			case 6: sc_start SC_Stone,10000,0; emotion ET_KIK; break;
			case 7: npctalk "CLUUUUUUCK!!!"; unitkill getcharid(3); skilleffect "NPC_SELFDESTRUCTION",1; emotion ET_HUK; break;
			case 8: npctalk "Cluck! CLUUUCK!!"; unitkill getcharid(3); skilleffect "NPC_SELFDESTRUCTION",1; emotion ET_HUK; break;
			case 9: sc_start SC_Stun,10000,0; break;
			case 10: sc_start SC_Sleep,10000,0; emotion ET_KIK; break;
			case 11: npctalk "Cluck! Cluck!"; break;
			case 12: sc_start SC_Stun,10000,0; break;
			case 13: unitkill getcharid(3); skilleffect "NPC_SELFDESTRUCTION",1; break;
			default:
				if (!rand(10)) {
					for(.@i = 0; .@i < getarraysize($@EventPlayers); .@i++){
						if(getcharid(3) == $@EventPlayers[.@i]){
							specialeffect2 EF_SPHERE;
							npctalk "Cluck! Cluck! Boom! " + strcharinfo(0) + " !!! Cluuuuuck!";
							getitem 7859,2;
							announce "[ Feira de Fim-de-Mês ]: Opa! Parece que temos um vencedor no evento Clucky Cluckers! Parabéns, "+strcharinfo(0)+"!",bc_all,0x00AAFF,0,12;
							set $@LastEvent,0;
							deletearray $@EventPlayers[0],getarraysize($@EventPlayers);
							sleep 10000;
							announce "[ Feira de Fim-de-Mês ]: O evento Clucky Cluckers terminou. Obrigado a todos que participaram!",bc_all,0x00AAFF,0,16;
						}
					}
				} else {
					npctalk "Cluck! CLUUUCK!!";
					unitkill getcharid(3); skilleffect "NPC_SELFDESTRUCTION",1;
					npcwalkto rand(152,159),rand(234,241);
				}
				break;
		}
		end;
	}
	mes "Cluck cluck! Cluuuuuck?";
	mes "Cluck....";
	close;
OnTimer15000:
	npcwalkto rand(152,159),rand(234,241);
	stopnpctimer;
	if ($@LastEvent == 7) {
		.i++;
	}
	if (.i >= 40) {
		announce "[ Feira de Fim-de-Mês ]: O evento Clucky Clutters terminou. Obrigado a todos que participaram!",bc_all,0x00AAFF,0,16;
		set $@LastEvent,0;
		deletearray $@EventPlayers[0],getarraysize($@EventPlayers);
		.i = 0;
	} 
	initnpctimer;
	end;

OnInit:
	npcspeed 75;
	npcwalkto rand(152,159),rand(234,241);
 	initnpctimer;
 	.i = 0;
end;
}