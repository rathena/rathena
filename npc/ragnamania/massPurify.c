// Ori/Elu Functions
//============================================================
function	script	massPurify	{
	if (checkweight(1201,1) == 0) {
		mes "- Wait a minute !! -";
		mes "- Currently you're carrying -";
		mes "- too many items with you. -";
		mes "- Please try again -";
		mes "- after you lose some weight. -";
		close;
	}
	mes "[" + getarg(0) + "]";
	mes "I can purify your";
	mes "Rough Oridecons or";
	mes "Rough Eluniums. I'll need";
	mes "5 Rough Stones to make";
	mes "1 pure one for you.";
	next;
	switch(select("Make Oridecon:Make Elunium")) {
	case 1:
		if (countitem(756) > 4) {
			mes "I can make ^FF0000" + countitem(756)/5 + "^000000 Oridecon";
			mes "out of your ^0000FF" + countitem(756) + "^000000 Rough Oridecon.";
			if (select("Go ahead","No") == 1) {
				if (countitem(756) > 4) {
					set .@ori,countitem(756)/5;
					delitem 756, .@ori * 5; //delete the rough oridecons
					getitem 984, .@ori; //get oridecons
					specialeffect 101;
					mes " ";
					mes "Here you are your Oridecon(s)!";
				} else {
					mes " ";
					mes "This action will be reported to the Staff.";
					logmes "Tried to bypass the refine system.";
				}
			}
			close;
		}
		else {
			mes "[" + getarg(0) + "]";
			mes "You have to be kidding!";
			mes "I have just told you that I need 5 Rough Oridecons to make a purified Oridecon.";
			close;
		}
		case 2:
			if (countitem(757) > 4) {
				mes "I can make up to ^FF0000" + countitem(757)/5 + "^000000 Elunium";
				mes "out of your ^0000FF" + countitem(757) + "^000000 Rough Elunium.";
				if (select("Go ahead","No") == 1) {
					if (countitem(757) > 4) {
						set .@elu,countitem(757)/5;
						delitem 757, .@elu * 5; //delete the rough eluniums
						getitem 985, .@elu; //get eluniums
						specialeffect 101;
						mes " ";
						mes "Here you are your Elunium(s)!";
					} else {
						mes " ";
						mes "This action will be reported to the Staff.";
						logmes "Tried to bypass the refine system.";
					}
				}
				close;
			}
		else {
			mes "[" + getarg(0) + "]";
			mes "You have to be kidding!";
			mes "I have just told you that I need 5 Rough Eluniums to make a purified Elunium.";
			close;
		}
	}
}
