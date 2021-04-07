prontera,258,41,5	script	Emperium Breaker	743,{

// What Map will be used
set .Map$,"job3_arch01";
// How many Top Breaker will be Recorded
set .TopRank,10;
// Message to Inform Players while inside the Room
setarray .Instruction$[0],
			"Please Listen here carefully.",
			"This is Emperium Breaker Test Room , which is use to test your Breaking Time.",
			"Prepare youself...and do your best to Break it...",
			"Okay that's All i want to say , get Ready yourself and it will Start Soon.";
			
			
while( 1 ){
mes "This is Emperium Breaker test Room. ";
mes "You are able to calculate your Emperium Breaking Speed in this Room.";
next;
switch( select( ( getmapusers( .Map$ ) )?"^FF0000NOT Available":"^0000FFAvailable^000000",
				"Top ^FF0000"+.TopRank+"^000000 Breaker Ladder",
				( getgmlevel() < 90 )?"":"^FF0000[GM]^000000 Reset Room",
				( getgmlevel() < 90 )?"":"^FF0000[GM]^000000 Reset Ladder" )){
Case 1:
	if( getmapusers( .Map$ ) ){
		mes "There is another player inside it now.";
		next;
		break;
		}
	warp .Map$,0,0;
	killmonster .Map$,"All";
//	removemapflag .Map$,mf_noskill;
	donpcevent strnpcinfo(0)+"::OnReady";
	end;
Case 2:
	mes "^ED1ADCLatest Record^000000 : "+LatestRecord/1000+"."+LatestRecord%1000+" Seconds.";
	for( set .@i,0; .@i < .TopRank; set .@i,.@i + 1 ){
		if( !$TopRankTime[.@i] ) break;
		mes "^FF0000Top "+( .@i + 1 )+" : ^0000FF"+$TopName$[.@i]+" ";
		mes "^FF0000Time Taken : ^ED1ADC[ "+$TopRankTime[.@i]/1000+"."+$TopRankTime[.@i]%1000+" Sec. ]^000000 ";
	}
	next;
	break;

Case 3:
	mapannounce .Map$,"A GM has RESET the Breaker Test Room.",0,0x00FF00;
	killmonster .Map$,"All";
	sleep2 3000;
	mapwarp .Map$,"prontera",156,175;
	mes "Done Reset. and Player who are inside are warped out.";
	next;
	break;
Case 4:	
	switch( select( "^0000FFAll Player^000000:^FF0000One Player^000000" ) ){
		Case 1:
			deletearray $TopRankTime[0],getarraysize( $TopRankTime );
			deletearray $TopName$[0],getarraysize( $TopName$ );
			mes "RESETED WHOLE LADDER.";
			break;
		Case 2:
			mes "Please input the ^FF0000EXACT NAME^000000 of player you want to delete.";
			input .Name$;
			for( set .@i,0; .@i < .TopRank; set .@i,.@i + 1 ){
				if( .Name$ == $TopName$[.@i] ){
					deletearray $TopName$[.@i],1;
					deletearray $TopRankTime[.@i],1;
					mes "Removed ^FF0000"+.Name$+"^000000 from the List.";
				}
			}
		}
	next;
	break;
	}
}

OnReady:
sleep2 3000;
for( set .@i,0; .@i < getarraysize( .Instruction$ ); set .@i,.@i + 1 ){
	mapannounce .Map$,.Instruction$[.@i],0,0x00FF00;
	sleep2 2500;
	}
for( set .@i,5; .@i > 0; set .@i,.@i - 1 ){
	mapannounce .Map$,"Count Down : "+.@i+" Seconds.",0,0x00FF00;
	sleep2 1000;
	}
//mapannounce .Map$,"Hit the Emperium Now and Skills Disabled.",0,0x00FF00;
setmapflag .Map$,mf_noskill;
monster .Map$,29,27,"Breaker Test",1288,1,strnpcinfo(0)+"::OnBreaked";
set .Record,0;
sleep2 500;
initnpctimer;
end;

OnTimer600000:
if( !mobcount( .Map$,strnpcinfo(0)+"::OnBreaked" ) ) end;
	mapannounce .Map$,"You spent too much time. i think you are AFK , so you are out.",0,0xED1ADC;
	sleep2 3000;
	mapwarp .Map$,"prontera",156,175;
	stopnpctimer;
end;

OnBreaked:
	set .Time,getnpctimer(0);
	copyarray .@tempName$[0],$TopName$[0],getarraysize( $TopName$ );
	copyarray .@tempRecord[0],$TopRankTime[0],getarraysize( $TopRankTime[0] );
	set LatestRecord,.Time;
	for( set .@i,0; .@i < .TopRank; set .@i,.@i + 1 ){
		if( .Time < $TopRankTime[.@i] || !$TopRankTime[.@i] ){
				set $TopRankTime[.@i],.Time;
				set $TopName$[.@i],strcharinfo(0);
				copyarray $TopRankTime[.@i + 1],.@tempRecord[0],getarraysize( .@tempRecord );
				copyarray $TopName$[.@i + 1],.@tempName$[0],getarraysize( .@tempName$ );
			announce "[ "+strcharinfo(0)+" ] Achieved Top "+( .@i + 1 )+" in Emperium Breaking with "+.Time/1000+"."+.Time%1000+" Seconds !!!",bc_all,0xED1ADC;
			break;
			}
		}
	stopnpctimer;
	announce "Current Time Taken : "+.Time/1000+"."+.Time%1000+" Seconds. ",bc_self,0xED1ADC;
//	removemapflag .Map$,mf_noskill;
	sleep2 5000;
	mapwarp .Map$,"prontera",156,175;
	end;
}
	
job3_arch01	mapflag	nocommand	50
job3_arch01	mapflag	gvg	on
job3_arch01	mapflag	gvg_castle
job3_arch01	mapflag	monster_noteleport
job3_arch01	mapflag	nosave	SavePoint