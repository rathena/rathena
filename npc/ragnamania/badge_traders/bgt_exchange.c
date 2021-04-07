// Badges Exchange
// *********************************************************************
prontera,145,206,3	script	Erundek	109,{
	mes "[Erundek]";		
	mes "Welcome, mighty warrior.";
	mes "What can I do for you today ?";
	next;
	switch( select("Exchange Bravery Badges","Exchange Valor Badges","Exchange War Badges","Hmm, nothing I guess.") )
	{
		case 4:
			mes "[Erundek]";
			mes "As you wish.";
			mes "See you later.";
			close;
		case 1: // Bravery Badges
			mes "[Erundek]";
			mes "So you want to exchange ^0000FFBravery Badges 1^000000 from the Battle of Tierra Valley.";
			mes "Close this window to open shop.";
			close2;
			callshop "BraveryExchanger",0;
			end;
		case 2: // Valor Badges
			mes "[Erundek]";
			mes "So you want to exchange ^FF0000Valor Badges 2^000000 from the Battle of Flavius.";
			mes "Close this window to open shop.";
			close2;
			callshop "ValorExchanger",0;
			end;
		case 3: // Heroism Badges
			mes "[Erundek]";
			mes "So you want to exchange ^FFA500War Badges 3^000000 from the Battle of Kriger Von Midgard.";
			mes "Close this window to open shop.";
			close2;
			callshop "HeroismExchanger",0;
			end;
	}
}

-	itemshop	BraveryExchanger	-1,7828,13036:100,13411:100,1425:100,1632:100,1634:100,1543:100,1924:100,1978:100,1574:100,1824:100,1183:100,1380:100,13305:100,1279:100,1739:100,13108:100,13172:100,2538:50,2539:50,2540:50,2435:50,2436:50,2437:50,2376:80,2377:80,2378:80,2379:80,2380:80,2381:80,2382:80,2720:500,2721:500,2722:500,2723:500,2724:500,2725:500,2733:500
-	itemshop	ValorExchanger	-1,7829,13037:100,13410:100,1633:100,1635:100,1542:100,1923:100,1977:100,1575:100,1823:100,1184:100,1482:100,1379:100,13306:100,1280:100,1738:100,13171:100,13173:100,13174:100,2538:50,2539:50,2540:50,2435:50,2436:50,2437:50,2376:80,2377:80,2378:80,2379:80,2380:80,2381:80,2382:80,2720:500,2721:500,2722:500,2723:500,2724:500,2725:500,2733:500
-	itemshop	HeroismExchanger	-1,7773,1187:3000,1281:3000,1282:3000,1310:3000,1382:3000,1426:3000,1486:3000,1546:3000,1576:3000,1577:3000,1640:3000,1641:3000,1743:3000,1826:3000,1827:3000,1927:3000,1981:3000,2002:3000,13042:3000,13110:3000,13176:3000,13177:3000,13178:3000,13179:3000,13307:3000,13416:3000,13417:3000,13418:3000,2394:1500,2395:500,2396:100,2444:750,2445:150,2446:50,2549:1000,2772:1800,2773:500,2774:100
