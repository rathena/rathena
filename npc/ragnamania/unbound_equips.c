// remove atributo de account_bound de um headgear ou garment na lista de itens da lootbox
// by biali

umbala,156,251,1	script	Oye	572,{
	function PickUp; function GetEquips;
	
	if(UNBOUNDDATA$ != "")
		PickUp();

	mes .n$;
	mes "Howdy!";
	mes "I can unbund headgear you got from " +getitemname(30027)+ " and make them tradeble.";
	next;
	mes .n$;
	mes "For that, I am gonna need some items and, of course, my fee. Please find check my services menu:";
	next;
	for(.@i=0;.@i<3;.@i++){

		switch(.@i){
			case 0: mes "^DD0000The cheapest option:^000000"; break;
			case 1: mes "^550088The 50/50 option:^000000"; break;
			case 2: mes "^0000FFThe hardcore option:^000000"; break;
		}
		mes .reagQt[.@i]+"x "+getitemname(.reagent);
		mes "5x "+ getitemname(.elu);
		mes callfunc("F_InsertComma",.cost[.@i]) +" zeny";
		mes "1x The headgear to be unbound";
		mes "Success chance : "+ .odds[.@i] +"% success";
		mes "Waiting time : "+ .time/60/60 +" hours";
		next;
	}
	GetEquips();
	mes .n$;
	if(getarraysize(@list)==0){
		mes "Sorry but it doesn't seem like you have any headgear on you that could benefit from my services.";
		close;
	}
	.@menu$ = "";
	freeloop(1);
	for(.@i=0;.@i<getarraysize(@list);.@i++){
		.@menu$ += getitemname(@list[.@i]) + ":";
	}
	freeloop(0);
	.@menu$ += "^FF0000Cancel^000000";

	mes "Please, just to make sure, which headgear are we working on?";
	next;

	.@eq = @list[select(.@menu$)-1];
	if(!.@eq)
		close;

	mes .n$;
	mes "Got it.";
	mes "Should we go ahead then and try to unbound this beauty from you?";
	next;
	if(select("Yes, please:No, thank you") == 2)
		close;
	mes .n$;
	mes "Which service?";
	next;
	.@service = select("The cheapest option:The 50/50 option:The hardcore option") - 1;
	mes .n$;
	mes "Ok. I need to collect your headgear now, my fee and all the other items necessary for the task.";
	next;
	mes .n$;
	mes "It will take "+.time/60/60+" hours to complete the job. Are you ready to proceed?";
	next;

	if(select("Yes, please:No, thank you")==2)
		close;
	mes .n$;

	if(countitem(.@eq) < 1)						.@error  = 1;
	if(countitem(.elu) < 5)						.@error += 2;
	if(countitem(.reagent) < .reagQt[.@service]).@error += 4;
	if(Zeny < .cost[.@service])					.@error += 8;

	if(.@error > 0) {
		mes "I am sorry,";
		if(.@error&1) mes "Where is your "+ getitemname(.@eq)+"?";
		if(.@error&2) mes "I couldn't find the 5x "+ getitemname(.elu);
		if(.@error&4) mes "I couldn't find the "+ .reagQt[.@service] +"x "+ getitemname(.reagent);
		if(.@error&8) mes "You don't seem to have all the "+ callfunc("F_InsertComma",.cost[.@service]) +"z I asked for.";
		close;
	}
	delitem .@eq,1; 					// The headgear
	delitem .elu,5;						// enriched eluniums
	delitem .reagent,.reagQt[.@service];// acid bottles

	set Zeny, Zeny - .cost[.@service];
	setarray .@temp$[0],gettimetick(2)+.time,.odds[.@service],.@eq;
	UNBOUNDDATA$ = implode(.@temp$,":");
	mes "All done... I will start on this right now. Please come back again in ^0000AA"+ callfunc("Time2Str",atoi(.@temp$[0]))+ "^000000 and hopefuly you will get your unbound piece of headgear!";
	close;

OnInit:
	.n$ = "^FF0000Oye^000000";
	.time = 60;//debug 24 * 60 * 60;
	.elu = 7619; 
	.reagent = 972; //Karvodalinrol
	setarray .reagQt[0], 30,50,80;
	setarray .cost[0], 5000000,10000000,50000000;
	setarray .odds[0], 30,50,80;
	setarray .eqp[0], 31062,31186,31437,31299,20183,19823,19530,20514,31057,20317,19539,20500,20746,31089,31151,20401,31667,31055,20496,20761,20764,20132,19537,30007,19609,19604,19602,19603,20105,20110,20111,20130,20133,20148,20153,20154,20160,20162,20163,20171,20172,20174,20175,20177,20179,19726,19541,19742,19643,20023,18740,19953,30010,5381,5214,5380,5226,5227,5228,5229,5230,5231,5232,5233,5234,5235,5236,5237,5283,5313,5324,5376,5505,5286,5263,5098,5382,5241,5242,31322,31245,31263,31320,31368,19847,31141,19974,20232,20190,20456,19775,2207,2210,2211,2213,2214,2215,2220,2236,2253,2257,2259,2272,2275,2279,2280,2282,2283,2289,2290,2293,2294,2298,5010,5011,5015,5020,5024,5028,5037,5039,5041,5048,5049,5050,5052,5056,5057,5059,5060,5061,5062,5064,5065,5075,5076,5077,5078,5079,5080,5082,5083,5097,5099,5100,5105,5106,5109,5111,5114,5115,5116,5117,5118,5129,5134,5136,5139,5144,5146,5148,5149,5150,5153,5172,5178,5179,5180,5182,5183,5186,5189,5198,5199,5200,5201,5202,5207,5209,5213,5218,5222,5226,5235,5236,5237,5246,5247,5252,5255,5259,5260,5261,5262,5264,5267,5280,5283,5286,5289,5290,5291,5292,5293,5301,5302,5303,5316,5317,5318,5321,5322,5323,5324,5333,5334,5335,5336,5337,5338,5339,5346,5356,5368,5369,5370,5372,5378,5380,5385,5390,5393,5405,5406,5408,5411,5413,5414,5415,5422,5430,5433,5436,5444,5447,5458,5459,5460,5464,5478,5479,5480,5486,5487,5488,5489,5496,5499,5500,5501,5503,5506,5507,5508,5509,5510,5512,5513,5514,5515,5526,5527,5540,5541,5542,5543,5544,5545,5546,5549,5550,5552,5554,5555,5559,5560,5561,5569,5570,5578,5581,5582,5583,5584,5587,5590,5600,5658,5659,5660,5668,5683,5684,5685,5686,5687,5691,5692,5741,5742,5792,5806,5812,5816,5819,5821,5827,18596,18656,20220,20221,20222,20223,20224,20225,20226,20227,20228,20230,20231,20232,20233,20234,20235,20236,20237,20238,20239,20241,20242,20243,20244,20245,20246,20247,20248,20249,20250,20251,20252,20253,20254,20255,20256,20257,20258,20259,20260,20262,20263,20264,20265,20266,20267,20268,20269,20270,20271,20272,20273,20274,20277,20278,20279,20280,20281,20282,20283,20284,20285,20286,20287,20288,20291,20292,20293,20294,20295,20296,20297,20298,20299,20300,20301,20302,20303,20304,20307,20311,20312,20313,20314,20315,20316,20317,20318,20319,20320,20321,20322,20323,20324,20325,20326,20327,20328,20329,20330,20331,20332,20333,20334,20335,20338,20339,20340,20341,20342,20345,20346,20347,20348,20350,20351,20352,20353,20354,20355,20356,20357,20358,20359,20360,20361,20362,20363,20364,20365,20366,20367,20368,20369,20370,20371,20372,20373,20374,20375,20376,20377,20378,20379,20380,20381,20382,20383,20384,20386,20391,20392,20396,20397,20398,20399,20400,20401,20402,20403,20404,20405,20406,20407,20408,20416,20417,20418,20419,20420,20421,20422,20423,20424,20425,20426,20427,20428,20429,20431,20432,20433,20434,20435,20436,20437,20438,20440,20441,20442,20447,20448,20449,20450,20451,20452,20455,20456,20457,20459,20460,20461,20462,20463,20464,20465,20466,20467,20470; 
	//.BG_CHARID = 165100;
	set .BG_CHARID, getbattleflag("bg_reserved_char_id");
	set .PVP_CHARID, getbattleflag("pvp_reserved_char_id");
	set .WOE_CHARID, getbattleflag("woe_reserved_char_id");
	end;


	function	PickUp	{
		explode(.@unboundData$,UNBOUNDDATA$, ":"); // expire,odds,itemid
		if(atoi(.@unboundData$[0]) > gettimetick(2)) {
			mes .n$;
			mes "In a hurry, huh? Your ^0000AA"+getitemname(atoi(.@unboundData$[2]))+"^000000 is not ready yet. I told you before, it won't be  ready until ^0000AA"+ callfunc("Time2Str",atoi(.@unboundData$[0])) +"^000000.";
			close;
		}
		mes .n$;
		mes "Howdy!";
		mes "I am glad you are back! I've been waiting for you. It's about your ^0000AA"+getitemname(atoi(.@unboundData$[2]))+"^000000.";
		next;
		if(rand(100)>atoi(.@unboundData$[1])) {
		   mes .n$;
		   mes "I am terribly sorry but the process ended up ^FF0000unsuccessfully^000000.";
		   next;
		   mes .n$;
		   mes "Here, please, take your item back.";
		   getitembound atoi(.@unboundData$[2]),1,bound_Account;
		   set UNBOUNDDATA$, "";
		   close;
		}

		mes .n$;
		mes "Howdy, "+strcharinfo(0);
		mes "Your ^0000AA"+getitemname(atoi(.@unboundData$[2]))+"^000000 is ready!";
		next;
		mes .n$;
		mes "Hope you'll like it better now.";
		getitem atoi(.@unboundData$[2]),1;
		set UNBOUNDDATA$, "";
		close;
	}

	function	GetEquips	{
		getinventorylist;
		deletearray @list[0],getarraysize(@list);
		deletearray @list_loc[0],getarraysize(@list);
		freeloop(1);
		for (.@i=0;.@i<@inventorylist_count;.@i++){
			.@loc = getiteminfo(@inventorylist_id[.@i],5);
			//Precisa ser equip bound to account
			if(@inventorylist_bound[.@i] == 1 ) {
				//Localizacao do item preciza ser headgear/garment ou costume (headgear/garment)
				if(.@loc>0 && (.@loc&1 || .@loc&4 || .@loc&256 || .@loc&512 || .@loc&1024 || .@loc&2048 || .@loc&4096 || .@loc&8196)){
					//set .@signer, @inventorylist_card3[.@i] | (@inventorylist_card4[.@i] << 0x10);
					//Nao pode ter cartas equipadas
					if(!(@inventorylist_card1[.@i]>0 || @inventorylist_card2[.@i]>0 || @inventorylist_card3[.@i]>0 || @inventorylist_card4[.@i]>0)){
						//Precisa conter na lista de equips dados pela ragnamania lootbox
						if(inarray(.eqp[0], @inventorylist_id[.@i])) {
							setarray @list[getarraysize(@list)], @inventorylist_id[.@i];
							setarray @list_loc[getarraysize(@list)], .@loc;
						}
					}
				}
			}
		}
		freeloop(0);
		return;
	}
}