prontera,1,1,1	script	deadbody	25006,{
	if(openlootbag() < 0) {
		mes strcharinfo(0);
		mes "Damn it!";
		mes "There is nothing useful here.";
		close2;
		unloadnpc strnpcinfo(0);
	} else
		end;
}