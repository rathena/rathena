-	script	onlinechecker	-1,{
	function	Plural;
    end;
OnInit:
    initnpctimer;
    end;
OnTimer60000:
    stopnpctimer;
    initnpctimer;
	.others = .extra;
	.extra = 0;
    if(.@sz = getarraysize($@online$) >= 3) {
        .names$ = implode($@online$,", ");
        cleararray $@online$[0],"",getarraysize($@online$);
		if(.others == 0) {
			addrid(0);
        	dispbottom .names$ + " have recently logged in and we are now " + getusers(1) + " players online.",0xff8533; 
		} else {
			addrid(0);
			dispbottom .names$ + " and another " +.others+ " "+(Plural(.others)?"players":"player")+" have recently logged in. We are now " + getusers(1) + " players online.",0xff8533; 
		}
    }
    end;

OnPCLoginEvent:
	if(getgmlevel() < 99)
		if(getarraysize($@online$)>3)
			.extra += 1;
		else
		    setarray $@online$[getarraysize($@online$)],strcharinfo(0);
    end;

OnPCLogoutEvent:
    if( (.@i = inarray($@online$,strcharinfo(0))) > -1)
        deletearray $@online$[.@i],1;
    end;

	function	Plural	{
		if(getarg(0) > 1) return 1;	else return 0;
	}
}