//	Author: Ehwaz
//	Version: 2018/06/01

// Dummy shop to insert items into:
-	itemshop	rops4coins	-1,501:50.

prontera,150,168,5	script	Coins Trader	1_M_WIZARD,{
	dispbottom DisplayCurrency(.currency);
	callshop .shop_name$,1;
	npcshopattach .shop_name$;
	end;
OnBuyItem:
	BuyProcess(.priceList, .currency);

OnInit:
	query_sql("SELECT `item_id`, `price` FROM `item_cash_db` ORDER BY `tab`"),.itemList;
	for(.@i=0;.@i<getarraysize(.itemList);.@i+2) {
		.items[.@i] += .itemList[.@i];
		.items[.@i+1] += .itemList[.@i+1];
	}

	.shop_name$ = "rops4coins";
	.npcName$ 	= strnpcinfo(1);	//Visiable name
	.currency 	= 675;				// 0: Zeny, else Item
					
	InitItemShop(.items,.shop_name$,.npcName$);
	end;
}