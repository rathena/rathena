sala_premmy,143,51,3	script	SG Resetter	644,{
	cutin "arang01",2;
	if (Class != Job_Star_Gladiator) {
		mes "[SG Resetter]";
		mes "Sorry, but I am here to help Star Gladiators only.";
		close2;
		cutin "",255;
		end;
	} 
    if(gettime(4) != 5) {
		mes "[SG Resetter]";
		mes "Sorry, but I do reset services only on Fridays.";
		close2;
		cutin "",255;
		end;
	}

    mes "[SG Resetter]";
    mes "What do you wish to reset?";
    menu "Reset Feeling",feeling,"Reset Hatred",hatred;

feeling:
    atcommand "@feelreset";
    mes "Done.";
    close2;
    cutin "",255;
    end;

hatred:
    set PC_HATE_MOB_MOON, 0;
    set PC_HATE_MOB_STAR, 0;
    set PC_HATE_MOB_SUN, 0;
    mes "Done.";
    next;
    mes "[SG Resetter]";
    mes "Hatred will be reset upon relogging.";
    mes "Do you wish to relog now?";
    menu "Relog now",relog,"Later",later;

relog:
    atcommand "@kick "+strcharinfo(0);
    end;

later:
    next;
    mes "[SG Resetter]";
    mes "Ok then, but don't forget changes won't take effect until you relog.";
    close2;
    cutin "",255;
    end;
}