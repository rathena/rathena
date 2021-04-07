-	script	Gms_Blessing	-1,{
	function	Bless;
	end;

OnClock0400:
	query_sql("DELETE FROM `acc_reg_num` WHERE `key` = '#GMS_BLESSING'");
	addrid(0);
	#GMS_BLESSING = @gms_blessing = 0;
	setbless(0);
	end;

OnClock0000:
	switch(gettime(DT_DAYOFWEEK)) {
		case 6:	case 0:
			set .gm$, "[AD]Chris";
			set .bless, 3; // exp/drop
			break;
		case 1: case 3: case 5:
			set .gm$, "[AD]Biali";
			set .bless, 1; // exp
			break;
		case 2: case 4:
			set .gm$, "[AD]Lion";
			set .bless, 2; // drop
			break;
	}
	end;

OnNPCKillEvent:
	if(isblessed() > 0)
		end;
	@gms_blessing += getmonsterinfo(killedrid,MOB_BASEEXP);
	if(@gms_blessing >= .KickOff)
		Bless();
	end;

OnPCLogoutEvent:
	if(isblessed() > 0)
		set #GMS_BLESSING, .KickOff;
	else
		set #GMS_BLESSING, @gms_blessing;
	end;

OnPCLoginEvent:
	if(#GMS_BLESSING >= .KickOff)
		Bless();
	else
		set @gms_blessing, #GMS_BLESSING;
	end;



OnInit:
	set .KickOff, 100000000; // 100kk de xp
	goto OnClock0000;
	end;

	function	Bless	{
		setbless(.bless);
		dispbottom "You've received " + .gm$ + " blessings.",0xfeb6ff;
		dispbottom "Rates increased accordingly. Go in peace and kick ass!",0xfeb6ff;
		return;
	}

}