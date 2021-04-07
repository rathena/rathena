// ======================================================================
//		  				Ragnarok Brasil Service
// ======================================================================
//
// Versões:
// 1.0 - Versão inicial.
// 1.1 - Adicionada configuração de bonus de doação e nome do servidor.
//
//	OBS: Anexado ao fim do NPC a tabela de log necessária, instale no seu
//	banco de dados caso a tabela não exista.
//
// ======================================================================

-	script	AutoCash	-1,{

OnPCLoginEvent:
	query_sql "SELECT `balance` FROM `cp_credits`  WHERE `account_id` = '"+getcharid(3)+"'",.@balance;
	if( .@balance == 0 ) end;
	if( .promo > 0 && .@balance >= .promo ) {
		set .@bonusPromo,(.@balance*.bonus)/100;
		set #CASHPOINTS,#CASHPOINTS+.@bonusPromo;
		dispbottom "[^9810c1 Ragnamania Chronos ^000000]";
		dispbottom "You've received ^1042c1"+.@bonusPromo+"^000000 bonus in Manias from your donation to the server.";
		logmes "Received "+.@bonusPromo+" Manias for his donation.";
	} else {
		set #CASHPOINTS,#CASHPOINTS+.@balance;
		dispbottom "[^9810c1 Ragnamania Chronos ^000000]";
		dispbottom "You've received "+.@balance+" Manias for your donation to the server.";
		logmes "Received "+.@balance+" Manias for his donation.";
	}
	dispbottom "Thank you very much for your donation to "+.nomeServidor$+"!";
	query_sql "UPDATE `cp_credits` set `balance` = '0'  WHERE `account_id` = '"+getcharid(3)+"'";
	query_sql "INSERT INTO  `cashlog` (`id` , `time` ,`char_id` ,`type` ,`cash_type` ,`amount` ,`map`) VALUES ( NULL , NOW( ) ,  '"+getcharid(0)+"',  'N',  'C',  '"+.@bonusPromo+"',  'NPC_CASH_AUTO')";
	end;
	
// CONFIGURAÇÕES
OnInit:
	set .promo,50000;						// Promoção ativa para doações de 50.000 cash ou mais. 0 = Desativado.
	set .bonus,110;							// 120 = 20% de bonus de cash.
											// 130 = 30%
											// 140 = 40%
											// 150 = 50%
											// ...
	set .nomeServidor$,"Ragnamania Chronos";	// Nome do servidor
	end;
}



/*			  ANEXO - Tabela de LOG
 *	Instale caso não tenha no seu banco de dados.
 *
 *	Tabela:
 *

	--
	-- Tabela `cashlog`
	--

	CREATE TABLE IF NOT EXISTS `cashlog` (
	  `id` int(11) NOT NULL AUTO_INCREMENT,
	  `time` datetime NOT NULL,
	  `char_id` int(11) NOT NULL DEFAULT '0',
	  `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','$') NOT NULL DEFAULT 'S',
	  `cash_type` enum('O','K','C') NOT NULL DEFAULT 'O', 
	  `amount` int(11) NOT NULL DEFAULT '0',
	  `map` varchar(11) NOT NULL DEFAULT '',
	  PRIMARY KEY (`id`),
	  INDEX `type` (`type`)
	) ENGINE=MyISAM AUTO_INCREMENT=1;


*/