// // ------------------------------------------
// // Premium Mounts System
// // ------------------------------------------

// itemmall,165,71,7	script	Premium Mounts::All_Riding	875,{
// 	if( .Ready == 0 ) donpcevent "All_Riding::OnInit";

// 	mes "[^FFA500Premium Mounts^000000]";
// 	mes "I sell premium mounts. You can enable it for this ^0000FFGame Account for only 20.000 Cash Points^000000.";
// 	mes "While using this mounts, you cannot attack or cast skills, but your speed will increase.";
// 	next;
// 	if( #ALL_RIDING )
// 	{
// 		mes "[^FFA500Premium Mounts^000000]";
// 		mes "You already own this Service.";
// 		mes "To summon your Mount use : ^0000FF@mon^000000";
// 		mes "To hide your Mount use : ^0000FF@moff^000000";
// 		close;
// 	}
// 	if( BaseClass == Job_Swordman )
// 	{
// 		mes "[^FFA500Premium Mounts^000000]";
// 		mes "I am sorry, this service is not available for Swordman Class.";
// 		mes "Come back with another character from this account if you want to purchase.";
// 		close;
// 	}

// 	if( select("Great, I want to purchase my Mount for this Account:Nah!, not interested") == 2 )
// 	{
// 		mes "[^FFA500Premium Mounts^000000]";
// 		mes "Come back again if you change your mind.";
// 		close;
// 	}
	
// 	if( ##MANIAS < 20000 )
// 	{
// 		mes "[^FFA500Premium Mounts^000000]";
// 		mes "I am sorry, you don't have enough Cash Points.";
// 		close;
// 	}
	
// 	set ##MANIAS, ##MANIAS - 20000;
// 	set #ALL_RIDING, 1;

// 	mes "[^FFA500Premium Mounts^000000]";
// 	mes "Your account is ready!!";
// 	mes "To summon your Mount use : @mon";
// 	mes "To hide your Mount use : @moff";
// 	close;

// OnInit:
// 	bindatcmd "mon","All_Riding::OnMountEnable",0,99;
// 	bindatcmd "moff","All_Riding::OnMountDisable",0,99;
// 	set .Ready, 1;
// 	end;

// OnMountEnable:
// 	if( #ALL_RIDING == 0 )
// 	{
// 		dispbottom "[Command only available if you purchase Premium Mount Pack]";
// 		end;
// 	}
// 	if( BaseClass == Job_Swordman )
// 	{
// 		dispbottom "[Premium Mounts not available for Swordman Classes]";
// 		end;
// 	}

// 	unitskilluseid getcharid(3),"ALL_RIDING",1;
// 	// sc_start SC_ALL_RIDING,-1,0;
// 	end;

// OnMountDisable:
// 	if( #ALL_RIDING == 0 )
// 	{
// 		dispbottom "[Command only available if you purchase Premium Mount Pack]";
// 		end;
// 	}

// 	sc_end SC_ALL_RIDING;
// 	end;
// }
