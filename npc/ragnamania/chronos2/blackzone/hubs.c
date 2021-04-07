coppa,1,1,1	script	coppa_hub	-1,{
	end;

OnInit:
	setcell "coppa",228,154,247,159,cell_basilica,1;
	setcell "coppa",214,154,227,179,cell_basilica,1;
	setcell "coppa",214,180,261,186,cell_basilica,1;
	setcell "coppa",248,154,261,179,cell_basilica,1;
	setcell "coppa",228,160,247,179,cell_basilica,1;
	setcell "coppa",236,187,239,197,cell_basilica,1;

	setcell "ars_fild22",175,206,193,213,cell_basilica,1;
	setcell "ars_fild22",194,194,205,225,cell_basilica,1;
	setcell "ars_fild22",206,182,213,237,cell_basilica,1;
	setcell "ars_fild22",214,184,226,235,cell_basilica,1;
	setcell "ars_fild22",227,188,238,227,cell_basilica,1;
	setcell "ars_fild22",238,194,242,226,cell_basilica,1;
	end;
}

-	script	#bz_portal	-1,{
	end;

OnTimer10100:
	misceffect 260;
	misceffect 247;
	stopnpctimer;
	initnpctimer;
	end;

OnInit:
	initnpctimer;
	misceffect 260;
	misceffect 247;
	end;

OnTouch:
	if(strnpcinfo(4) == "coppa")
		warp "gef_tower",52,100;
	else if(strnpcinfo(4)) == "ars_fild22"
		warp "lhz_in02",246,218;
	end;
}

ars_fild22,179,209,1	duplicate(#bz_portal)	#wp_ars22	45,1,1
coppa,237,170,1	duplicate(#bz_portal)	#wp_coppa	45,3,3


-	script	#mid2bz	-1,{
	end;
OnTouch:
	mes "You are about to be teleported to a full-loot pvp area. Are you sure you want to continue?";
	if(select("Yes, please.:No, thank you") == 2)
		close;
	if(strnpcinfo(4) == "gef_tower")
		warp "coppa",237,161;
	else if(strnpcinfo(4) == "lhz_in02")
		warp "ars_fild22",186,209;
	end;

OnTimer10100:
	misceffect 260;
	misceffect 247;
	stopnpctimer;
	initnpctimer;
	end;

OnInit:
	initnpctimer;
	misceffect 260;
	misceffect 247;
	end;
}

gef_tower,52,104,1	duplicate(#mid2bz)	#gef2bz	45,1,1
lhz_in02,249,221,1	duplicate(#mid2bz)	#lhz2bz	45,1,1



-	script	bzps1	2562,{
	end;
}

gef_tower,49,103,5	duplicate(bzps1)	Portal Summoner#1	792
gef_tower,54,103,4	duplicate(bzps1)	Portal Summoner#2	792
lhz_in02,247,222,3	duplicate(bzps1)	Portal Summoner#3	792
lhz_in02,250,219,3	duplicate(bzps1)	Portal Summoner#4	792