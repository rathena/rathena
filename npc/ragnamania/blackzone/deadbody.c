// The emulator will replicate this script to create the Deadbodies
// As part of the Full Loot system
prontera,1,1,1	script	deadbody	25006,{
	if(openlootbag() < 0) {
		mes "^0000FF[ "+strcharinfo(0)+" ]^000000";
		mes "Damn it!";
		mes "There is nothing useful in here.";
		mes "Rest in peace whoever you were.";
		close;
	}
	end;
}
