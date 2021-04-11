//Maths
function	script	abs	{
	.@i = getarg(0);
	return ( (.@i < 0) ? -.@i : .@i );
}

// Is leaping year?
function	script	F_isLeapYear	{
	.@year = gettime(DT_YEAR);
	if(.@year%4 == 0)
		if(.@year%100 == 0)
			if(.@year%400 == 0)
				return 1;

	return 0;
}

function	script	F_isVSmap	{
	.@m$ = getarg(0,strcharinfo(3));
	if(getmapflag(.@m$,MF_PVP) || 
	getmapflag(.@m$,MF_GVG) || 
	getmapflag(.@m$,MF_GVG_CASTLE)|| 
	getmapflag(.@m$,MF_GVG_DUNGEON)  || 
	getmapflag(.@m$,MF_BATTLEGROUND)) ||
	getmapflag(.@m$,MF_RPK)
		return 1;
	else
		return 0;
}


// function	script	F_GmBlessAsBars	{
// 	.@bars		= 15;
// 	.@totalxp 	= getvariableofnpc(.KickOff,"Gms_Blessing");
// 	.currxp		= @gms_blessing;
// 	.@blue 		= (.@currxp * .@bars) / .@totalxp;
// 	if(isblessed() > 0) 
// 		return "^0000FF|||||||||||||||^000000";
// 	freeloop(1);
// 	for(.@i=0;.@i<.@bars;.@i++) {
// 		if(.@i <= blue)
// 			.@m$ += "^0000FF|^000000";
// 		else
// 			.@m$ += "^FF0000|^000000";
// 	}
// 	freeloop(0);
// 	return .@m$;
// }

function	script	IsAllowed	{
	.n$ = "^AA00AA"+getarg(0)+"^000000";
	if (!getcharid(2)) {
		mes .n$;
		mes "^FF0000You have to enter a guild to be able to hit Emperium!^000000";
		close;
	} else if (getgdskilllv(getcharid(2),10013) || 
			(getgdskilllv(getcharid(2),10000) +
			getgdskilllv(getcharid(2),10001) +
			getgdskilllv(getcharid(2),10002) +
			getgdskilllv(getcharid(2),10003) +
			getgdskilllv(getcharid(2),10004) +
			getgdskilllv(getcharid(2),10005) +
			getgdskilllv(getcharid(2),10006) +
			getgdskilllv(getcharid(2),10007) +
			getgdskilllv(getcharid(2),10008) +
			getgdskilllv(getcharid(2),10009) +
			getgdskilllv(getcharid(2),10010) +
			getgdskilllv(getcharid(2),10011) +
			getgdskilllv(getcharid(2),10012) +
			getgdskilllv(getcharid(2),10013) +
			getgdskilllv(getcharid(2),10014) > 9)
			) {
		mes .n$;
		mes "I see... your guild has Emergency Call mastered.";
		mes "You cannot enter the Novice Castle area.";
		emotion ET_SCRATCH;
		emotion ET_KEK, playerattached();
		close;
	} else {
		mes "I'm a new usher of Novice Castles.";
		next;
		mes .n$;
		if ((eaclass()&(EAJL_2|EAJL_UPPER)) || BaseLevel>=99) {
			mes "I'm sorry, you can't enter the sacred Novice Castles place.";
			emotion ET_SORRY;
			close;
		} else {
			// remove several unallowed buffs
			sc_end SC_ASSUMPTIO;
			sc_end SC_IMPOSITIO;
			sc_end SC_SUFFRAGIUM;
			sc_end SC_MAGNIFICAT;
			sc_end SC_WEAPONPERFECTION;
			sc_end SC_GOSPEL;
			sc_end SC_BASILICA;
			sc_end SC_MAGICPOWER;
			sc_end SC_MARIONETTE;
			sc_end SC_MARIONETTE2;
			sc_end SC_DEVOTION;
			sc_end SC_SACRIFICE;
			sc_end SC_MAXOVERTHRUST;
			sc_end SC_SPIRIT;
		}
	}
	return;
}


function	script	Expire	{
	set .@Time_Left, getarg(0) - gettimetick(2);
	
	set .@Days, .@Time_Left / 86400;
	set .@Time_Left, .@Time_Left - (.@Days * 86400);
	set .@Hours, .@Time_Left / 3600;
	set .@Time_Left, .@Time_Left - (.@Hours * 3600);
	set .@Minutes, .@Time_Left / 60;
	set .@Time_Left, .@Time_Left - (.@Minutes * 60);
	
	set .@Time$, "";
	if( .@Days > 0 )
		set .@Time$, .@Time$ +"^FF0000"+ .@Days + "d^000000";

	if( .@Hours > 0 )
		set .@Time$, .@Time$ +"^00AA00"+ .@Hours + "h^000000";

	if( .@Minutes > 0 )
		set .@Time$, .@Time$ +"^0000AA"+ .@Minutes + "m^000000";

	return .@Time$;
}


function	script	F_LootBox	{
	if(countitem(30027)>0) {
		delitem 30027,1; 
		delitem 30028,1; 
		getrandgroupitem IG_Ragnamania_Food1; 
		getrandgroupitem IG_Ragnamania_Food2; 
		getitem 30022,rand(10,50);
		.@rand = rand(1000);
		if(.@rand > 800)
			getitem 14220,1; // enriched ori x5
		else if(.@rand > 600)
			getitem 14219,1; // enriched elu x5

		.@rand = rand(1000);
		if(.@rand > 200) {
			getitembound groupranditem(IG_RAGNAMANIA_TIER3),1,bound_Account;
			if(getgmlevel() == 99)
				dispbottom "Caiu item tier 3";
		} else if(.@rand > 50) {
			getitembound groupranditem(IG_RAGNAMANIA_TIER2),1,bound_Account;
			if(getgmlevel() == 99)
				dispbottom "Caiu item tier 2";
		} else {
			getitembound groupranditem(IG_RAGNAMANIA_TIER1),1,bound_Account;
			if(getgmlevel() == 99)
				dispbottom "Caiu item tier 1";
		}

	} else { 
		npctalk "The key alone can't do much. I need at least one "+getitemname(30027)+".","",bc_self; 
	} 
	return;
}

function	script	F_Romantic_LootBox	{
	getrandgroupitem IG_Ragnamania_Romantic;
	getitem 5548,1; // Scarlet Rose
	getitem 12235,5; // Chocolate Strawberry

	return;
}

function	script	F_CharStorage	{
	.@cID = getarg(0,999);
	query_sql("SELECT char_num FROM `char` WHERE char_id = "+getcharid(0), .@char_num$);
	if (.@char_num$ == "0")	openstorage2 10,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "1")	openstorage2 11,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "2")	openstorage2 12,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "3")	openstorage2 13,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "4")	openstorage2 14,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "5")	openstorage2 15,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "6")	openstorage2 16,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "7")	openstorage2 17,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "8")	openstorage2 18,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "9")	openstorage2 19,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "10")	openstorage2 20,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "11")	openstorage2 21,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "12")	openstorage2 22,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "13")	openstorage2 23,STOR_MODE_GET|STOR_MODE_PUT;
	else if (.@char_num$  == "14")	openstorage2 24,STOR_MODE_GET|STOR_MODE_PUT;
	else dispbottom "Can`t open storage. Unknown char_id.";
	end;
}



function	script	F_getDaysOfMonth	{
switch (gettime(DT_MONTH)) {
	case 2: // February
		set .@days, callfunc("F_isLeapYear",gettime(DT_YEAR)? 29 : 28);
		break;
	case 9: // September
	case 4: // April
	case 6: // June
	case 11: // November
		set .@days, 30;
		break;
	default:
		set .@days, 31;
		break;
	}
	return .@days;
}



function	script	F_GetRandomPartyMember	{
	
	.@pid = getarg(0,0);
	if(.@pid <= 0)
		return;

	getpartymember getcharid(1), 2, .@accountids;
	.@count = $@partymembercount;

	// loop through both and use 'isloggedin' to count online party members
	while (.@i < getarraysize(.@accountids)) {
		if ( !isloggedin( .@accountids[.@i]) ) {
			deletearray .@accountids[.@i],1;
		}
		.@i++;
	}

	.@winner = getelementofarray(.@accountids,rand(getarraysize(.@accountids)));

	return .@winner;
}



function	script	F_Dispell	{
	.@gid = getarg(0,0);
	freeloop(1);
	while( .@i < 553 ) {
		if( .@i != SC_WEIGHT50 && .@i != SC_WEIGHT90 && .@i != SC_JAILED && .@i != SC_NOCHAT ){
			if(!.@gid)
				sc_end .@i;
			else
				sc_end .@i,.@gid;
		}
		.@i++;
	}
	freeloop(0);
	return;
}

function	script	F_AddZero	{
	return ((getarg(0)<10)?"0":"")+getarg(0)+(getarg(1,0)?".":":")+"00";
}


function	script	GetCurrencyText	{
	.@currency = getarg(0,0);
	return .@currency ? getitemname(.@currency): "Zeny";
}

function	script	DisplayCurrency	{
		.@currency		= getarg(0,0);
		.@textCurrency$ = GetCurrencyText(.@currency);
		if(.@textCurrency$ != "Zeny"){
			.@numCurr$ = "("+countitem(.@currency)+")";
			return "Welcome to "+ .@textCurrency$ + " Shop " + .@numCurr$;
		}
		return .@textCurrency$ == "Zeny" ? "" : "Welcome to "+ .@textCurrency$ + " Shop" ;
}

//////////////////////////////////

function	script	InitItemShop	{

	copyarray(.@items[0],getarg(0),getarraysize(getarg(0)));
	
	.@shop_name$ 	= getarg(1);
	.@npcName$ 		= getarg(2);
	.@total_items 	= getarraysize(.@items);

	for(.@i = 0; .@i < .@total_items; .@i+=2){
		
		.@itemid = .@items[.@i];
		.@price  = .@items[.@i+1];
		
		.@priceList[.@itemid] = .@price;
		
		set getvariableofnpc(.priceList[.@itemid],.@npcName$),.@price;
		npcshopadditem .@shop_name$, .@itemid , .@price;
	}

	npcshopdelitem .@shop_name$,501;
}

//////////////////////////////////

function	script	BuyProcess	{

	.@bought_nameid_size = getarraysize(@bought_nameid);
	copyarray(.@price_arr[0],getarg(0),getarraysize(getarg(0)));
	
	.@itemCurrencyId	= getarg(1,0);
	.@isZeny 			= .@itemCurrencyId ? 0 : 1;
	.@textCurrency$		= GetCurrencyText(.@itemCurrencyId);
	.@paid 				= 0;
	
	for (.@i = 0; .@i < .@bought_nameid_size; .@i++){
	
		.@boughtId 	= @bought_nameid[.@i];
		.@quanty 	= @bought_quantity[.@i];
		.@totalPrice = .@price_arr[.@boughtId] * .@quanty;
		
		.@canPay	= 1;
		
		//Check weight
		if( !checkweight(.@boughtId,.@quanty) )
		{
			message strcharinfo(0),sprintf("Need more weight to get %d %s",.@quanty, getitemname(.@boughtId));
			break;
		}
		
		if(.@isZeny)
		{
			if(Zeny >= .@totalPrice){
				Zeny -= .@totalPrice;
				getitem .@boughtId,.@quanty;
				.@paid += .@totalPrice;
			}
			else .@canPay = 0;
		}
		else 
		{
			if(countitem(.@itemCurrencyId) >= .@totalPrice){
				delitem .@itemCurrencyId,.@totalPrice;
				getitem .@boughtId,.@quanty;
				.@paid += .@totalPrice;
			}
			else .@canPay = 0;
		}
		
		if(!.@canPay)
		{
			message strcharinfo(0),sprintf("Not enought %s to pay %d %s",.@textCurrency$,.@quanty, getitemname(.@boughtId));
			break;
		}	
	}
	
	if(.@paid)
		dispbottom "Paid: "+F_InsertComma(.@paid)+" "+.@textCurrency$+"";
	
	deletearray @bought_quantity, getarraysize(@bought_quantity);
	deletearray @bought_nameid, .@bought_nameid_arr_size;
	end;
}


// WoeTETimeStart(<seconds>) : return true if the woe te will start in X seconds or less, false otherwise
function	script	WoeTETimeStart	{
	.@woe_status = agitcheck3();
	.@min_today  = gettime(DT_MINUTE);
	.@hour_today = gettime(DT_HOUR);
	.@day_today  = gettime(DT_DAYOFWEEK);

	if (getvariableofnpc( getd( ".day_"+ .@day_today ),"woe_TE_contoller" )) {
		.@h = getvariableofnpc( getd( ".hour_start_"+ .@hour_today ),"woe_TE_contoller" );
		if (.@h > .@hour_today) {
			.@time = (60 - .@min_today) *60 + ( .@h - .@hour_today -1 ) *3600;
			if (.@time <= getarg(0))
				return true;
		}
	}
	return false;
}

function	script	F_22507	{// ShabbyOldScroll
	.@r = rand(1,10);
	if (.@r == 1) warp "pay_fild01",141,211;
	else if (.@r == 2) warp "payon_in03",99,190;
	else if (.@r == 3) warp "pay_dun04",120,116;
	else if (.@r == 4) warp "pay_gld",202,186;
	else if (.@r == 5) warp "gld_dun01",61,155;
	else if (.@r == 6) warp "pay_fild04",348,333;
	else if (.@r == 7) warp "payon_in02",17,62;
	else if (.@r == 8) warp "payon_in01",144,15;
	else if (.@r == 9) warp "pay_fild07",200,186;
	else if (.@r == 10) warp "pay_fild10",147,267;
	end;
}
