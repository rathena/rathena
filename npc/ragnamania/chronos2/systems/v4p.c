prontera,143,278,0	script	Chefe Amante::v4p	625,{
	function CheckPoints;
	function UpdatePoints;
	function SetTargets;
	function AdjustRates;
	function SetBF;

	mes .n$;
	mes "Hello, " + strcharinfo(0)+"!";
	if(gettime(DT_DAYOFMONTH) > .ThirdDday && $vp_totalVotePoints < .ThirdTarget) {
		mes "We're short by " + (.ThirdTarget - $vp_totalVotePoints) + " vp this month.";
		mes "It is just too sad... ^0000AA=^FF0000(^000000";
		mes "Because of that there is nothing I can help you with until next month.";
		close;
	}
	mes "How can I help you today?";
	next;
	
	switch(select((gettime(DT_DAYOFMONTH)<.ThirdDday?"Redeem Vote Points":"Extend the Event")+":Collect my Rewards")) {
		case 1:
			if(gettime(DT_DAYOFMONTH)<.ThirdDday) {
				.@votes = CheckPoints();
				if( .@votes <= 0) {
					mes .n$;
					mes "Sorry, darling, you don't seem to have any voting points right now.";
					next;
					mes .n$;
					mes "You can always ALT+TAB and contribute, though. ^.~";
					close;
				}
				
				mes .n$;
				mes "Sure, thing! I can see you have ^AA000000" + .@votes + " Vote Points^000000 at the moment. Should we go ahead and contabilize them?";
				next;
				if(select("Yes, please:No, not now") == 2){
					mes .n$;
					mes "Absolutely! Not a problem.";
					mes "See you later then!";
					close;
				} else {
					UpdatePoints(getcharid(3),.@votes);
					mes .n$;
					mes "All done!";
					mes "I've just added your ^AA0000"+.@votes+" vote points^000000 to the stack and we now count with a total of ^0099AA"+$vp_totalVotePoints+" Vote Points^000000 this month so far!";
					next;
					mes .n$;
					mes "I've also taken note of your personal contribution. Please continue to contribute and come back by the end of the month for you prize!";
					next;
					mes .n$;
					mes "You've accumulated ^AA0000"+ #VP_VOTEPOINTS+" vote points^000000 until now. You need at least ^0099AA"+.MinVotes4Reward+"^000000 in order to qualify for the Personal Reward.";
					announce "Thank you " + strcharinfo(0) + ", for voting for us! And thank you for your contribution of " + .@votes + " vote points!",bc_all,0xFAFA00; 
					close;
				}
			} else if(gettime(DT_DAYOFMONTH) >= .ThirdDday && gettime(DT_DAYOFMONTH) < .dayOfMonth) {
				mes .n$;
				mes "From this point until the end of the month we can extend the bonuses using Zeny.";
				next;
				mes .n$;
				if ($vp_donations < .ExtensionCost)
					mes "To extend the event for another 24 hours we need to collect " + callfunc("F_InsertComma",(.ExtensionCost - $vp_donations)) + "z until midnidgt tonight.";
				else
					mes "We've got enough for another 24 hours of event but you can still donate if you want... That amount will be kept for the day after then.";
				next;
				if(select("I'd like to donate:Sorry, not now") == 2) {
					mes .n$;
					mes "Sure, no problem at all!";
					mes "See you around then!";
					close;
				} else {
					mes .n$;
					mes "Great! How much are you willing to donate to the cause?";
					next;
					input .@donation;
					if(.@donation > 0 && .@donation <= Zeny) {
						mes .n$;
						if($vp_donations < .ExtensionCost)
							mes "Right! So with your donation of ^0000AA"+.@donation+"z^000000 the outstanding balance left to unlock another 24 hours of bonus will be ^0000AA" + (.ExtensionCost - $vp_donations - .@donation) + "z^000000.";
						else
							mes "Right! So with your donation of ^0000AA"+.@donation+"z^000000 hopefuly we will grant another day with these marvelous buffs, right?";
						next;
						if(select("Proceed:Cancel") == 2) {
							mes .n$;
							mes "No problem!";
							mes "See you later, then";
							close;
						} else {
							mes .n$;
							if(Zeny < .@donation) {
								mes "Are you trying to cheat?";
								mes "I am reporting this to the admin team.";
								mes "Good-bye.";
								logmes "Chefe Amante: Cheater detected charid = " +getcharid(0);
								close;
							} else {
								set Zeny, Zeny - .@donation;
								set $vp_donations, $vp_donations + .@donation;
								set #VP_ZENY_DONATED, #VP_ZENY_DONATED + .@donation;
								mes "All done!";
								mes "Thank you very much!";
								announce "Thank you " + strcharinfo(0) + ", for your contribution to extend the Voting Points event!",bc_all,0xFAFA00; 
								close;
							}
						}
					} else {
						mes .n$;
						mes "I am sorry, you cannot donate that amount.";
						close;
					}
				}
			} else {
				mes .n$;
				mes "I am sorry but tomorrow the event will restart so we are not accepting any further donations today.";
				close;
			}
			break;
		case 2:
			if(#VP_VOTEPOINTS >= .MinVotes4Reward && gettime(DT_DAYOFMONTH) >= 20 && !#VP_COLLECTED) {
				.@qt = #VP_VOTEPOINTS * 3;
				getitem 675, .@qt;
				getitem 30023,10; // Bag of Manias
				vip_time $@Periodo; // Extends Premmy by 5 days
				set #VP_COLLECTED,1;
				mes .n$;
				mes "Thank you very much for your contribution! We've given you " + .@qt + " " + getitemname(675) + "s as a gesture of appreciation and we have also given you 5 days of Premmy!";
				next;
				mes .n$;
				mes "Hope you continue to do the good job as you did last month!";
				close;
			} else if(gettime(DT_DAYOFMONTH) < 20)  {
				mes .n$;
				mes "Hum... I am afraid the Rewards are distributed only from the "+.ExtensionStart+"th of each month. Please come back by then.";
				close;
			} else if(#VP_VOTEPOINTS < .MinVotes4Reward) {
				mes .n$;
				mes "Oh dear, I am afraid you don't have enough Vote Points to receive any rewards just now. Please continue to conttribute and come back later.";
				next;
				mes .n$;
				mes "You have " + #VP_VOTEPOINTS + " out of " + .MinVotes4Reward + " necessary for the monthly personal reward.";
			} else if(#VP_COLLECTED) {
				mes .n$;
				mes "Oh, my apologies, but you've already collected your reward for this month!";
				close;
			} else {
				mes .n$;
				mes "Hum... I am afraid there is nothing I can do for you at the moment. Please come back later and don't forget to vote for us in our website!";
				close;
			}
			break;
	}
	end;

OnHour00:	//-- Monthly Rewards
	.dayOfMonth = callfunc("F_getDaysOfMonth");
	if(gettime(DT_DAYOFMONTH) == 1) {
		query_sql("DELETE FROM `cp_v4p_voters`");
		query_sql("DELETE FROM `acc_reg_num` WHERE `key` = '#VP_VOTEPOINTS'");
		query_sql("DELETE FROM `acc_reg_num` WHERE `key` = '#VP_ZENY_DONATED'");
		query_sql("DELETE FROM `acc_reg_num` WHERE `key` = '#VP_COLLECTED'");
		set $vp_totalVotePoints, 0;	// Zera Bucket de pontos
		set $vp_donations, 0;		// Zera Bucket de Zenys
		logmes "Chefe Amante : Deletado o conteudo da tabela cp_v4p_voters";
		logmes "Chefe Amante : Deletado o #VP_VOTEPOINTS da tabela acc_reg_num";
		logmes "Chefe Amante : Deletado o #VP_COLLECTED da tabela acc_reg_num";
		logmes "Chefe Amante : Deletado o #VP_ZENY_DONATED da tabela acc_reg_num";
		logmes "Chefe Amante : Zerada a variavel $vp_totalVotePoints";
	}
	SetTargets();
	AdjustRates();
	end;

OnInit:
	.MinVotes4Reward 	= 60;
	.FirstDday 			= 5;
	.SecondDday 		= 10;
	.ThirdDday			= 15;
	.FirstTarget		= 1000;
	.SecondTarget		= 2500;
	.ThirdTarget		= 4000;
	.ExtensionStart		= 20;
	.ExtensionCost		= 100000000; //500 kk 
	.dayOfMonth 		= callfunc("F_getDaysOfMonth");
	.OriginalEquipsRates = getBattleFlag("item_rate_equip");
	.OriginalCardsRates	= getBattleFlag("item_rate_card");
	.BonusE1			= .OriginalEquipsRates *= 15/100;
	.BonusC1			= .OriginalEquipsRates *= 15/100;
	.BonusE2			= .OriginalEquipsRates *= 20/100;
	.BonusC2			= .OriginalEquipsRates *= 20/100;
	.BonusE3			= .OriginalEquipsRates *= 30/100;
	.BonusC3			= .OriginalEquipsRates *= 30/100;
	.n$ = "^00AAFF[ Chefe Amante ]^000000";
	SetTargets();
	AdjustRates();
	end;



	function CheckPoints {
		.@gid = getarg(0,getcharid(3));
		query_sql("SELECT points FROM `cp_v4p_voters` WHERE `account_id` = "+.@gid,.@votes);

		return .@votes;
	}

	function UpdatePoints	{
		set .@gid, getarg(0,getcharid(3));
		set .@votes, getarg(1,0);
		set $vp_totalVotePoints, $vp_totalVotePoints + .@votes;
		set #VP_VOTEPOINTS, #VP_VOTEPOINTS + .@votes;
		query_sql("DELETE FROM `cp_v4p_voters` WHERE `account_id` = "+.@gid);

		return;
	}

	function SetTargets {
		switch(gettime(DT_DAYOFMONTH)) {
			case 1:	case 2:	case 3:	case 4:	case 5:
				set .VotePointsTarget, .FirstTarget;
				break;
			case 6:	case 7:	case 8:	case 9:	case 10:
				set .VotePointsTarget, .SecondTarget;
				break;
			default:
				set .VotePointsTarget, .ThirdTarget;
				break;
		}
		return;
	}

	function AdjustRates {
		// Muito cedo no mes... do dia 1 ao dia 5 nada acontece:
		if(gettime(DT_DAYOFMONTH) < .FirstDday) {
			SetBF();
			return;
		}
		// Se o montante de VotePoints nao tiver atingido a meta, nada a fazer:
		if($vp_totalVotePoints < .VotePointsTarget) 
			return;

		// De tras pra frente agora: Se estivermos no periodo das extensoes (dia 20+):
		if(gettime(DT_DAYOFMONTH) >= .ExtensionStart) {
			// Se coletamos zeny o bastante, Extendemos os bonus por mais 24 horas:
			if($vp_donations >= .ExtensionCost) {
				SetBF(.BonusE3,.BonusC3);
				set $vp_donations, $vp_donations - .ExtensionCost;
				logmes "F_AdjustRates : Expandido bonus. Ajustado $vp_donations";
				return;
			// Caso contrario volta as rates originais:
			} else
				SetBF();
		} else if(gettime(DT_DAYOFMONTH) >= .ThirdDday)
			SetBF(.BonusE3,.BonusE3);
		else if(gettime(DT_DAYOFMONTH) >= .SecondDday)
			SetBF(.BonusE2,.BonusC2);
		else if(gettime(DT_DAYOFMONTH) >= .FirstDday)
			SetBF(.BonusE1,.BonusC1);
			
		return;
	}

	function SetBF {
		set .@equip, getarg(0,.OriginalEquipsRates);
		set .@card, getarg(0,.OriginalCardsRates);

		setBattleFlag "item_rate_equip",.@equip,true;
		setBattleFlag "item_rate_card",.@card,true;

		return;
	}
}


