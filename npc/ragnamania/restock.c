// =====================================================
//              _    _                 
// /\   /\_   _| | _(_)_ __ ___   ___  
// \ \ / / | | | |/ / | '_ ` _ \ / _ \ 
//  \ V /| |_| |   <| | | | | | | (_) |
//   \_/  \__, |_|\_\_|_| |_| |_|\___/ 
//        |___/                        
//                                     
// =====================================================
// = Name :
// @Restock by Vykimo [free script]
// =====================================================
// = Version :
// 2.0
// =====================================================
// = Description :
// @restock <itemid> <itemid> ...
// @restock off
// Consume 1 coin if non VIP Player
// =====================================================

-	script	restocking_atcmd	-1,{

function	TrouverIndice	{
	for(.@i=0;.@i<getarraysize(getarg(0));.@i++)
		if(getelementofarray(getarg(0),.@i) == getarg(1)) break;
		
	if(.@i == getarraysize(getarg(0)) && getelementofarray(getarg(0),.@i) != getarg(1)) 
		return -1;
	else return .@i;
}

OnInit:
	// MODIFY THE COIN ID FOR NON VIP PLAYERS
	// -1 to disable coin requirement
	.CoinID = 675;
	// ===================
	bindatcmd "restock",strnpcinfo(3)+"::OnAtcommand";
	end;
	
OnAtcommand:
	if(.@atcmd_numparameters == 1 && strtolower(.@atcmd_parameters$[0]) == "off") {
		deletearray RestockList[0],getarraysize(RestockList);
		dispbottom "@restock : List erased.";
		end;
	}
	if(.@atcmd_numparameters == 0) {
		dispbottom "Command Usage : @restock <itemid> <itemid> ...";
	} else {
		if(.CoinID > 0 && !vip_status(1)) {
			mes "[@Restock]";
			mes "You are not Premmy, you have to pay 1x "+getitemname(.CoinID)+" to use this command.";
			mes "^0000FFCommand Usage : @restock <itemid> <itemid> ...^000000";
			next;
			if(select("Pay 1x "+getitemname(.CoinID)+":Cancel") == 2) close;
			if(countitem(.CoinID)<1) {
				mes "[@Restock]";
				mes "You don't have enough "+getitemname(.CoinID)+".";
				close;
			}
			close2;
		}
		.@pay = 0;
		for(.@i=0;.@i<.@atcmd_numparameters;.@i++) {
			.@id = atoi(.@atcmd_parameters$[.@i]);
			if(getitemname(.@id) != "null") {
				if(getiteminfo(.@id,2) == 0 || getiteminfo(.@id,2) == 2 || getiteminfo(.@id,2) == 3 || getiteminfo(.@id,2) == 11) {	
					if((.@array_id = TrouverIndice(RestockList,.@id)) < 0) {
						setarray RestockList[getarraysize(RestockList)],.@id;
						dispbottom "@restock : Item '"+getitemname(.@id)+"' (id:"+.@id+") successfully added!";
						.@pay = 1;
					} else {
						dispbottom "@restock : Item '"+getitemname(.@id)+"' (id:"+.@id+") already in list.";
					}
				}
				else dispbottom "@restock : you can put only usable items. (id "+.@id+" non available)";
			} else dispbottom "@restock : this item don't exist. (id "+.@id+")";
		}
		dispbottom "@restock - Current list :";
		for(.@i=0;.@i<getarraysize(RestockList);.@i++)
			dispbottom "- "+getitemname(RestockList[.@i])+" ("+restockitem(RestockList[.@i])+" left in storage)";
		if(.@i == 0)
			dispbottom "Empty";
		
		if(.CoinID > 0 && !vip_status(1) && .@pay == 1) {
			dispbottom ".: "+ getitemname(.CoinID) +" consumed :.";
			delitem .CoinID,1;
		}
	}
	end;
OnItemUsed:
	.@useditem = itemusedid;
	if(.@useditem > 0 && (.@array_id = TrouverIndice(RestockList,.@useditem)) >= 0) {
		if(restockitem(.@useditem) > 0) {
			storagedelitem .@useditem,1;
			getitem .@useditem,1;
			//getnameditem
			//getequipcardid
		} else {
			deletearray RestockList[.@array_id],1;
		}
	}
	end;
	
}
