// -	script	newPlayersPremmy	-1,{
// 	end;

// OnInit:
// 	set $@Periodo, 60 * 24 * 5; //	5 dias em minutos
// 	set $@EndTime, 60 * 60 * 24 * 5; // 5 dias em segundos 
// 	end;

// // OnPCLoginEvent:

// // 	if (vip_status(1)) {

// // 		if((#FREE_PREMMY + $@EndTime < gettimetick(2)) && (#FREE_BOOSTER == 0) ) {
// // 			#FREE_BOOSTER = gettimetick(2);
// // 			getitem 30001,1;
// // 		}

// // 	} else if (getgmlevel() < 20) {
// // 		if(#FREE_PREMMY == 0) {
// // 			//set #FREE_PREMMY = gettimetick(2) + $@Periodo;
// // 			vip_time $@Periodo;
// // 			#FREE_PREMMY = gettimetick(2);
// // 			dispbottom "^00AAAA[Premmy System]^000000 : Premmy Activated.";
// // 			logmes "[Premmy System] : Premmy Activated.";
// // 		}
// // 	}
// // end;
// }