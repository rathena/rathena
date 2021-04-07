prontera,141,288,6	script	Exchange	549,{
	mes .n$;
	mes "Hi, "+strcharinfo(0)+",";
	mes "I am here to change ^DD00AAMania$^000000 into ^AA00DD"+getitemname(30022)+"^000000 and vice-versa.";
	switch(select("Change Manias into Coins:Change Coins into Manias:Cancel")) {
	case 1:
		next;
		mes .n$;
		mes "I give you 1x ^AA00DD" + getitemname(30022) + "^000000 in exchange for 10 ^DD00AAMania$^000000";
		mes "Remember the maximum amount of " +getitemname(30022)+ " one can hold is 30.000!";
		if(#CASHPOINTS <= 0) {
			close;
		} else {
			mes "Do you wanna trade?";
			next;
			if(select("^009911Yes please:^AA0011No, thanks^000000") == 1) {
				mes .n$;
				mes "You have, at the moment, ^DD00AA" + #CASHPOINTS + " Mania$^000000. Do you wanna to convert the whole amount into ^AA00DD"+getitemname(30022)+"^000000?";
				next;
				mes .n$;
				switch(select("^009911Yes please:^AA0011No, I want an specific amount:^FF0000Cancel^000000")) {
				case 1:
					set .@qt, #CASHPOINTS / 10;
					if(countitem(30022) + .@qt > 30000) {
						mes "Sorry, You'd end up with over 30.000 " + getitemname(30022) + " and I cannot allow that.";
						close;
					}
					set #CASHPOINTS, 0;
					getitem 30022,.@qt;
					logmes strcharinfo(0)+ " converted " + .@qt * 10 + " Manias into " + .@qt + " " + getitemname(30022);
					mes "Thank you very much! Here you have your " + .@qt + " " + getitemname(30022);
					mes "Have a nice day!";
					close;
					break;
				case 2:
					mes "How many " + getitemname(30022)+" you'd like?";
					input .@qt;
					set .@cost, .@qt * 10;
					if(.@qt < 1 ) {
						mes "Sorry, I am busy right now.";
						close;
					}
					mes " ";
					mes "That will cost you " + .@cost + " Manias";
					mes "Do you wanna proceed?";
					next;
					if(select("^009911Yes please:^AA0011No, thanks^000000") == 1) {
						if(#CASHPOINTS < .@cost) {
							mes .n$;
							mes "Sorry, I am busy right now.";
							close;
						} else {
							if(countitem(30022) + .@qt > 30000) {
								mes "Sorry, You'd end up with over 30.000 " + getitemname(30022) + " and I cannot allow that.";
								close;
							}
							set #CASHPOINTS, #CASHPOINTS - .@cost;
							getitem 30022,.@qt;
							logmes strcharinfo(0)+ " converted " + .@cost + " Manias into " + .@qt + " " + getitemname(30022);
							mes .n$;
							mes "Thank you very much! Here you have your " + .@qt + " " + getitemname(30022);
							mes "Your balance in Mania$ is " + #CASHPOINTS;
							mes "Have a nice day!";
							close;
						}
					} else 
						callsub Dismiss;
					break;
				case 3:
					callsub Dismiss;
					break;
				}
			} else
				callsub Dismiss;
		}
		break;
	case 2:
		next;
		mes .n$;
		mes "For each ^AA00DD" + getitemname(30022) + "^000000 I give you 10 ^DD00AAMania$^000000";
		mes "How much in Manias do you want?";
		input .@manias;
		next;
		if(.@manias < 10){
			mes .n$;
			mes "Sorry you must imput a minimum of 10.";
			close;
		}
			
		set .@qt, .@manias / 10;
		mes .n$;
		mes "That will cost you " + .@qt + " " + getitemname(30022)+". Do you wanna proceed?";
		next;
		if(select("^009911Yes please:^AA0011No, thanks^000000") == 1) {
			if(countitem(30022) < .@qt)
				goto Dismiss;
			delitem 30022,.@qt;
			set #CASHPOINTS, #CASHPOINTS + .@manias;
			logmes strcharinfo(0)+ " converted " + .@qt + " " + getitemname(30022) + " into " + .@manias + " Manias.";
			mes .n$;
			mes "All done! Thank you!";
			close;
		} else {
			goto Dismiss;
		}
		break;
	case 3:
		goto Dismiss;
		break;
	}
	end;


Dismiss:
	mes .n$;
	mes "No problem, latters then!";
	close;

OnInit:
	.n$ = "^0000AA[ Exchange ]^000000";
	end;
}