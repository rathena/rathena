-	script	DiarioDeBordo	-1,{
OnInit:
 	set .SG_RESET_COST, 10; // Custo em hunting coins pra resetar hatred e feeling de SG 
 	end;
}

function	script	RM_DiarioDeBordo	{

	if(inarray($@EventPlayers[0],getcharid(3)) >= 0) {
		mes "^0000FF[ Dashboard ]^000000";
		mes "Sorry, the use of the Dashboard is temporarily disabled during the event.";
		close;
	}

	set .SG_RESET_COST, getvariableofnpc(.SG_RESET_COST,"DiarioDeBordo");
	getmapxy(.@mapName$,.@mapX,.@mapY,BL_PC,strcharinfo(0));

L_Menu:
	mes "^0000FF[ Dashboard ]^000000";
	if( vip_status(1) ) {
		mes "Conta: ^00CCAA[ Premium ]^000000";

	} else
		mes "Conta: ^00AAFF[ Regular ]^000000";
		
	mes (#CASHPOINTS)?"Mania$: ^0000FF" +#CASHPOINTS+ "^000000.":"Mania$: ^FF000000.00^000000.";
	mes "Hunting Quests: ^0000FF" + HM_MISSION_COMPLETED + "^000000";
	mes "Personal Vote Points: ^0000FF[" + #VP_VOTEPOINTS + "/" + getvariableofnpc(.MinVotes4Reward,"v4p") + "]^000000";
	if(gettime(DT_DAYOFMONTH) < getvariableofnpc(.ThirdDday,"v4p"))
		mes "Global Vote Points: ^0000FF[" + $vp_totalVotePoints + "/" + $@VotePointsTarget + "]^000000";
	else
		mes "Global Zeny Donation: ^0000FF[" + callfunc("F_InsertComma",$vp_donations) + "/" + callfunc("F_InsertComma",getvariableofnpc(.ExtensionCost,"v4p")) + "]^000000";
	mes "Voting Event ends in ^FF0000" + (gettime(DT_DAYOFMONTH)>=getvariableofnpc(.ThirdDday,"v4p")?getvariableofnpc(.dayOfMonth,"v4p")-gettime(DT_DAYOFMONTH):(gettime(DT_DAYOFMONTH)>=getvariableofnpc(.SecondDday,"v4p")?getvariableofnpc(.ThirdDday,"v4p")-gettime(DT_DAYOFMONTH):getvariableofnpc(.SecondDday,"v4p")-gettime(DT_DAYOFMONTH) ) +" days.^000000");
	mes getvariableofnpc(.gm$,"Gms_Blessing") + " Bless: " + F_GmBlessAsBars();
	next;

	switch(select("Premmy Services:Teleport:Hunting Quests:Battlegrounds:Other Services"+(Class == 4047?":Star Gladiator Services":""))) {
		case 1: // PREMMY SERVICES

			mes "^0000FF[ Dashboard ]^000000";
			mes "Do you want to activate or extend your premmy?";
			next;

			if(select("Not now:Yes, please") == 2) {
				//donpcevent "Premmy Central::OnTalk";
				callfunc "F_Premmy";
				end;
			}

			if( !vip_status(1) ) {
				mes "^0000FF[ Dashboard ]^000000";
				mes "This service is for Premmy users only.";
				close;
			} 

			switch(select("Open storage:Open personal storage:Open guild storage:Repair:Purify ore:Utility store")){
				case 1:
					close2;
					openstorage;
					end;
					break;
				case 2:
					close2;
					callfunc "F_CharStorage",getcharid(0);
					end;
					break;
				case 3:
					close2;
					guildopenstorage;
					end;
					break;
				case 4:
					if(inarray($@EventPlayers,getcharid(3)) >= 0 || getmapflag(strcharinfo(3),MF_PVP) == 1 || getmapflag(strcharinfo(3),MF_GVG) == 1 || getmapflag(strcharinfo(3),MF_GVG_CASTLE) == 1 || getmapflag(strcharinfo(3),MF_GVG_DUNGEON) == 1 || getmapflag(strcharinfo(3),MF_BATTLEGROUND) == 1 ) { 
						mes "Sorry, this service is not available now.";
						close;
					} else {
						while (getbrokenid(1)) {
					        repair(1);
					        set .@i, .@i +1;
					    }
					    if (.@i) dispbottom "Dashboard: " + .@i + " Done!";
					    end;
					}
					break;
				case 5:
					callfunc "orimain","Dashboard";
					break;
				case 6:
					if( getmapflag(strcharinfo(3),33) == 1 || getmapflag(strcharinfo(3),6) == 1 || getmapflag(strcharinfo(3),9) == 1 ) {  
						mes "Sorry, this service is not available now.";
						close;
					} else {
						mes "Select the store.";
						close2;
						switch( select("Utilities:Arrows:Bullets:Kunais:Gemstones:Stones:Alchemy") ) {
							case 1:
								callshop "Utilities",1;
								break;
							case 2:
								callshop "Arrows",1;
								break;
							case 3:
								callshop "Munitions",1;
								break;
							case 4:
								callshop "Kunais",1;
								break;
							case 5:
								callshop "Gemstones",1;
								break;
							case 6:
								callshop "Stones",1;
								break;
							case 7:
								callshop "Alchemy",1;
								break;
						}
						end;
					}
					break;
				default:
					close;
				break;
			}
		case 2: // TELEPORTE
			if(inarray($@EventPlayers,getcharid(3)) >= 0 || getmapflag(strcharinfo(3),MF_PVP) == 1 || getmapflag(strcharinfo(3),MF_GVG) == 1 ||  getmapflag(strcharinfo(3),MF_BATTLEGROUND) == 1 || compare(strcharinfo(3),"new_") == 1) { 
				mes "Sorry, this service is not available now.";
				close;
			}
			switch(select("Prontera:Geffen:Payon:Morroc:Aldebaran:Alberta:Juno:Lighthalzen:Moscovia:Einbroch:Hugel:Rachel:Veins:Ayothaya:Amatsu:Louyang:Gonryun:Comodo")) {
				case 1: warp "prontera",156,171; end;
				case 2: warp "geffen",126,63; end;
				case 3: warp "payon",172,101; end;
				case 4: warp "morocc",156,95; end;
				case 5: warp "aldebaran",143,116; end;
				case 6: warp "alberta",31,238; end;
				case 7: warp "yuno",158,39; end;
				case 8: warp "lighthalzen",159,105; end;
				case 9: warp "moscovia",232,194; end;
				case 10: warp "einbroch",71,199; end;
				case 11: warp "hugel",98,148; end;
				case 12: warp "rachel",132,114; end;
				case 13: warp "veins",211,123; end;
				case 14: warp "ayothaya",206,169; end;
				case 15: warp "amatsu",197,83; end;
				case 16: warp "louyang",211,101; end;
				case 17: warp "gonryun",160,120; end;
				case 18: warp "comodo",193,148; end;
				case 19: warp "umbala",100,154; end;
				case 20: warp "niflheim",49,146; end;
				break;
			}
		case 3: // HUNTING QUESTS
			switch(select("Check Contract:Share Quest:^DD0000Drop/Release Quest^000000")){
				case 1:
					if( HM_IN_MISSION ) {
						callfunc "MissionInfo";
						close;
					} else {
						mes "^0000FF[ Dashboard ]^000000";
						mes "You are not in a Hunting Quest.";
						close;
					}
					break;
				case 2:
					// mes "Esta funcao nao esta disponivel no momento.";
					// close;
					if( HM_IN_MISSION ) {
						//donpcevent "Contracts::OnShare";
						close2;
						callfunc "F_Share";
						end;
					} else {
						mes "^0000FF[ Dashboard ]^000000";
						mes "You are not in a Hunting Quest.";
						close;
					}
					break;
				case 3:
					if(countitem(6320) > 0 ) {
						mes "^0000FF[ Dashboard ]^000000";
						mes "If your quest is expired you can abandon this one and get a new one in the Agent Kafra in Prontera for free.";
						mes "Do you want to ^FF0000abandon the current^000000 Hunting Quest using your ^0000FF" + getitemname(6320) + "^000000?";
						if(select("No, thanks:Yes, please") == 1) {
							mes "Ok, no problem.";
							close;
						} else {
							delitem 6320,1;
							callfunc "reset_quest";
							mes " ";
							mes "Done! You can now get another quest from Agent Kafra in Prontera.";
							close;
						}
					} else {
						mes "^0000FF[ Dashboard ]^000000";
						mes "Oh, I don't have a ^DD0000"+getitemname(6320)+" ^000000with me.";
						close;
					}
				default:
					close;
			}
			break;
			
		case 4: // BATTLEGROUNDS
			switch(select("Join BG:Leave BG:Cancel")) {
				case 1:
					if(!getmapflag(strcharinfo(3),MF_TOWN)){
						mes "^0000FF[ Dashboard ]^000000";
						mes "I am sorry but you can only join Battlegrounds from a City or Town.";
						close;
					}
					mes "^0000FF[ Dashboard ]^000000";
					if( checkquest(8506,PLAYTIME) == 2 )
						erasequest 8506;
					if( checkquest(8506,PLAYTIME) != -1 ) {
						mes "You can't enter in Battlegrounds until the deserter penalty expires.";
						close;
					} else {
						mes "Welcome to the Battlegrounds queue.";
					}
					bg_queue_join getvariableofnpc(.BG_Queue,"BG_Queue_Join");
					close;

					break;
				case 2:
					mes "^0000FF[ Dashboard ]^000000";
					mes "You left the Battlegrounds queue.";
					bg_queue_leave getvariableofnpc(.BG_Queue,"BG_Queue_Join");
					close;

					break;
				case 3:
					mes "No problem!";
					close;
					break;
			}
			break;
		case 5: // OUTROS Servicos
			switch(select("Mania transfer")) {
				case 1:
					mes "This function is disable";
					mes "Look for NPC Exchange in Prontera (141 288)";
					close;

					// if(#CASHPOINTS > 0) {
					// 	set @GMid, playerattached(); 
					// 	mes "Manias: " + #CASHPOINTS;
					// 	mes "----------------------";
					// 	mes "Para quem Voc? deseja enviar Manias?";
					// 	input @Nombre$;
					// 	set @RID, getcharid(3,@Nombre$);
					// 	if (@RID < 1) {
					// 		mes "Jogador n?o encontrado";
					// 		close;
					// 	}
					// 	mes "Destinatario: " + @Nombre$;
					// 	mes "Quantas Manias Voc? deseja transferir?";
					// 	input @manias;
					// 	mes "Transferir: " + @manias;
						
					// 	switch(select("Cancelar:Transferir")) {
					// 		case 1: 
					// 			close;
					// 			break;
					// 		case 2:
					// 			setd ".valor"+@RID, @manias;
					// 			setd ".sender"+@RID, @GMid;
								
					// 			if( #CASHPOINTS < @manias) {
					// 				mes "^FF0000SALDO INSUFICIENTE^000000";
					// 				close;
					// 			} else {
					// 				#CASHPOINTS -= @manias;
					// 				mes "^0000FF[ Di?rio de Bordo ]^000000";
					// 				if (attachrid(@RID) > 0) {
					// 					set @manias, getd(".valor" + playerattached());
					// 					set @remetente, getd(".sender" + playerattached());
					// 					#CASHPOINTS += @manias;
					// 					logmes "Tansferencia de Manias: Recebeu " + @manias + " de " + @remetente + " balanco final: " + #CASHPOINTS;
					// 					dispbottom "Voc? recebeu " + @manias + " de " + @remetente + ".";
					// 					if (attachrid(@remetente) > 0) {
					// 						mes "^0000FF[ Di?rio de Bordo ]^000000";
					// 						mes "Prontinho. Transferencia concluida.";
					// 						mes "Seu novo saldo: " + #CASHPOINTS + " Manias.";
					// 						close;
					// 					} else {
					// 						#CASHPOINTS -= @manias;
					// 						logmes "Transferencia de Manias: Sender deslogou muito rapido. " + @manias + " foram excluidas!";
					// 					}
					// 				}
					// 			}
					// 	}
					// } else {
					// 	mes "Voc? n?o possui Manias.";
					// 	mes "Por favor, visite nosso site para entender como adquirir se tornar um Doador e adquirir Manias";
					// 	close;
					// }
					break;
				default:
					close;
			}
		case 6: // StarGladiator resets
			mes "^0000FF[ Dashboard ]^000000";
			mes "You can reset ^FF0000Hatred^000000 or ^0000FFFeeling^000000 for ^888888"+ .SG_RESET_COST + "x " + getitemname(675) + "^000000 once every 24h.";
			menu "Feeling reset",feeling,"Hatred reset",hatred;

		feeling:
			if(SG_RESET_FEELING > gettimetick(2))
				goto inCoolDown;
			if(countitem(675) < .SG_RESET_COST && !vip_status(1))
				goto NoCoins;
			if(!vip_status(1))
				delitem 675,.SG_RESET_COST;
			atcommand "@feelreset";
			misceffect 582;
			set SG_RESET_FEELING, gettimetick(2) + 68400;
			next;
			mes "^0000FF[ Dashboard ]^000000";
			mes "Prontinho.";
			close;

		hatred:
			if(SG_RESET_HATRED > gettimetick(2))
				goto inCoolDown;
			if(countitem(675) < .SG_RESET_COST && !vip_status(1))
				goto NoCoins;
			if(!vip_status(1))
				delitem 675,.SG_RESET_COST;
			set PC_HATE_MOB_MOON, 0;
			set PC_HATE_MOB_STAR, 0;
			set PC_HATE_MOB_SUN, 0;
			misceffect 582;
			set SG_RESET_HATRED, gettimetick(2) + 68400;
			next;
			mes "^0000FF[ Dashboard ]^000000";
			mes "Done.";
			mes "You need to logout.";
			mes "Do you want to logout now?";
			menu "Yes, I want to logout",relog,"I will do this later",later;

		relog:
		   atcommand "@kick "+strcharinfo(0);
		   end;

		later:
			next;
			mes "^0000FF[ Dashboard ]^000000";
			mes "Ok, your reset will be complete only after you logout.";
			close;

		NoCoins:
			next;
			mes "^0000FF[ Dashboard ]^000000";
			mes "Sorry, looks like you don't have "+getitemname(675)+" enough.";
			close;

		inCoolDown:
			next;
			mes "^0000FF[ Dashboard ]^000000";
			mes "Sorry, you can only reset once every 24h.";
			close;

		break;
		default:
			close;
			break;
	}
}

-	shop	Utilities	-1,601:-1,602:-1,501:-1,502:-1,503:-1,504:-1,522:-1,523:-1,525:-1,610:-1,645:-1,656:-1,657:-1,1065:-1,611:-1
-	shop	Arrows		-1,1750:1,1751:3,1752:3,1753:4,1770:2,1754:5,1755:5,1756:5,1757:5,1758:10,1759:10,1760:10,1761:10,1762:10,1763:10,1764:20,1765:25,1766:40,1767:5,1768:10,1772:5
-	shop	Munitions	-1,13200:1,13201:15,13202:30,13203:80,13204:80,13205:80,13206:80,13207:80
-	shop	Kunais		-1,13256:10,13294:100,13259:10,13258:10,13257:10,13255:10
-	shop	Gemstones	-1,717:-1,716:-1,715:1200
-	shop	Stones		-1,7521:-1,7522:-1,7523:-1,7524:-1
-	shop	Alchemy		-1,657:-1,713:-1,937:-1,939:-1,952:-1,972:-1,7033:-1,905:-1,1092:-1,1059:-1,1061:2000,929:2000,1044:2000