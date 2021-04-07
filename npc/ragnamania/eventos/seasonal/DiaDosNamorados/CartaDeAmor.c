jawaii,182,173,4	script	Romantic Mail Officer::cupido	521,{
	function TakeZeny; function QueueEvent;

	if(getgmlevel() == 99) {
		mes "debug?";
		// playBGMall "1003";
		if(select("yes:no") == 1) {
			close2;
			goto OnDebug;
			end;
		}
	}

	if(gettime(DT_MONTH) != 6)
		end;
	if(gettime(DT_DAYOFMONTH) < 4 || gettime(DT_DAYOFMONTH) > 12)
		end;

	mes .n$;
	mes "So, it is ^FF0000Valentine's Day^000000 again!";
	mes "Planing anything special to your loved one?";
	next;
	switch(select("Yes, I always am:I don't have a soulmate just yet")) {
		case 1: // Yes, I always am
			mes .n$;
			mes "Own... That is so adorable!";
			mes "Well, I think I can help you!";
			mes "We've arranged some pretty special things this year.";
			mes "Let me present them to you:";
			next;
			mes .n$;
			mes "^FF0000Love Postcard^000000";
			mes "What about sending your loved one a very romantic Postcard?";
			mes "They will receive that the moment they log in and I promise they are gonna love the surprise!";
			mes "They will also get a copy in their mail so they can keep it forever";
			mes "You get to write it with your own words!";
			mes "That would cost you ^0000FF"+F_InsertComma(.LovePostcard)+"z^000000";
			next;
			mes .n$;
			mes "^FF0000Broadcast of Love^000000";
			mes "What about we broadcasting your votes to your loved one for the whole server to hear?";
			mes "We can do that the moment your loved one logs in so we make sure they get to see it and appreciate your efforts to make them feel so special.";
			mes "That would cost you ^0000FF"+F_InsertComma(.BroadcastOfLove)+"z^000000";
			next;
			mes .n$;
			mes "^FF0000Romantic Lootbox^000000";
			mes "Now, this is charming... What about sending your loved one a special Lootbox giving them a chance to get one of the cutest and most desireble itens ingame?";
			mes "Lovely, isn't it? And it will come unlocked, of course! hi hi";
			mes "That would cost you ^0000FF"+F_InsertComma(.RomanticLootbox)+"z^000000";
			next;
			mes .n$;
			mes "Beautiful options aren't they?";
			mes "Now, if you are premmy, you pay only half of the price! How good is that?";
			next;
			mes .n$;
			mes "So, which one are you choosing? I mean, you can send them all too, if you want!";
			mes "We must do it one at the time, though.";
			next;
			switch(select("Love Postcard:Broadcast of Love:Romantic Lootbox")) {
				case 1: //Love Postcard
					mes .n$;
					mes "Nice choice!";
					mes "Ok, lets get it done.";
					next;
					Fail:
					mes .n$;
					mes "First of all, what is the name of your loved one?";
					input .@name$;
					.@name$ = escape_sql(.@name$);
					.@c = query_sql("SELECT `account_id`,`char_id` FROM `char` WHERE `name` = '"+.@name$+"';",.@acc,.@char);
					next;
					if(.@c <=0){
						mes .n$;
						mes "Oops... Couldn't find that one.";
						mes "Please try again";
						next;
						goto Fail;
					}
					Redo:
					mes .n$;
					mes "Great...";
					mes "Now please write down your card.";
					mes "It is a postcard so please be brief as we have limited space in the card itself";
					input .@message$;
					.@message$ = escape_sql(.@message$);
					next;
					mes .n$;
					mes "Alright, here is your message:";
					mes " ";
					mes "^0000FF"+.@name$+",^000000";
					mes "^0000FF"+.@message$+"^000000";
					next;
					mes .n$;
					mes "Are you happy with that? Should we send it over to "+.@name$+"?";
					mes "It will be "+ F_InsertComma(.LovePostcard) + "z, please.";
					next;
					if(select("Yes, please:No, I wanna change it") == 2)
						goto Redo;
					
					TakeZeny(.LovePostcard);
					QueueEvent(1,.@acc,.@char,.@message$);

					mes .n$;
					mes "Perfect!";
					mes "All done! " + .@name$ + " will receive their adorable card on the 12th of June or whenever they log in after that.";
					close;
				case 2: // Broadcast of Love
					mes .n$;
					mes "Perfect! Lets get started then...";
					next;
					mes .n$;
					mes "In this package you will get:";
					mes " - A full message broadcasted to the whole server at some point when your beloved is logged in;";
					mes " - A special soundtrack for the occasion;";
					mes " - Some special itens sent direct to your loved one;";
					next;
					Fail2:
					mes .n$;
					mes "First of all, what is the name of your loved one?";
					input .@name$;
					.@name$ = escape_sql(.@name$);
					.@c = query_sql("SELECT `account_id`, `char_id` from `char` WHERE `name` = '"+.@name$+"';",.@acc,.@char);
					next;
					if(.@c<=0){
						mes .n$;
						mes "Oops... Couldn't find that one.";
						mes "Please try again";
						next;
						goto Fail2;
					}
					ChooseMes:
					mes .n$;
					mes "As for the messages, we've prepared a few options for you to choose from:";
					next;
					.@op = select("Não há maior felicidade...:Você é a melhor companhia:Cada segundo ao seu lado...:Primeiro dia dos namorados juntos"));
					switch(.@op) {
						case 1:
							mes "^0000FF"+.Mes1$+"^000000";
							break;
						case 2:
							mes "^0000FF"+.Mes2$+"^000000";
							break;
						case 3:
							mes "^0000FF"+.Mes3$+"^000000";
							break;
						case 4:
							mes "^0000FF"+.Mes4$+"^000000";
							break;
					}
					next;
					mes .n$;
					mes "Are you happy with this one or maybe you'd like to check the others?";
					mes " ";
					mes "^0000FF"+getd(".Mes"+.@op+"$")+"^000000";
					next;
					if(select("I am happy with this one:I am gonna check the others") == 2)
						goto ChooseMes;
					
					mes .n$;
					mes "Beautiful!";
					mes "Now we must choose the soundtrack for your big event.";
					next;
					mes .n$;
					mes "We've got four options for you to choose from:";
					next;
					.@music = select("Take my breath away - Berlin:Perfect - Ed Sheeran:Amor da sua cama - Felipe Araujo:Love me like you do - Ellie Gouding");
					mes .n$;
					mes "Fabulous choice! I absolutely love that song.";
					next;
					mes .n$;
					mes "I believe that is all we need! It will be " + F_InsertComma(.BroadCastOfLove)+"z. Do you want to proceed?";
					next;
					if(select("Yes, please:No, I've changed my mind") == 2)
						close;
					
					TakeZeny(.BroadCastOfLove);
					QueueEvent(2,.@acc,.@char,.@op,.@music);
					mes .n$;
					mes "Perfect!";
					mes "All done! " + .@name$ + " will receive their adorable surprise on the 12th of June or whenever they log in after that.";
					close;
					break;
				case 3: // Romantic Lootbox
					mes .n$;
					mes "Oh, how lovely!";
					mes "Ok, let get it ready...";
					next;
					Fail3:
					mes .n$;
					mes "First of all, what is the name of your loved one?";
					input .@name$;
					.@name$ = escape_sql(.@name$);
					.@c = query_sql("SELECT `account_id`,`char_id` FROM `char` WHERE `name` = '"+.@name$+"';",.@acc,.@char);
					next;
					if(.@c <=0){
						mes .n$;
						mes "Oops... Couldn't find that one.";
						mes "Please try again";
						next;
						goto Fail3;
					}
					Redo3:
					mes .n$;
					mes "Great...";
					mes "Now, would you please write down some words in the card that follows the Romantic Lootbox?";
					mes "It is just a small card so please be brief.";
					input .@message$;
					.@message$ = escape_sql(.@message$);
					next;
					mes .n$;
					mes "Alright, here is your message:";
					mes " ";
					mes "^0000FF"+.@name$+",^000000";
					mes "^0000FF"+.@message$+"^000000";
					next;
					mes .n$;
					mes "Are you happy with that? Should we start packing?";
					mes "It will be "+ F_InsertComma(.RomanticLootbox) + "z, please.";
					next;
					if(select("Yes, please:No, I wanna change it") == 2)
						goto Redo3;
					
					TakeZeny(.RomanticLootbox);
					QueueEvent(3,.@acc,.@char,.@message$);
					
					mes .n$;
					mes "Perfect!";
					mes "All done! " + .@name$ + " will receive their adorable card on the 12th of June or whenever they log in after that.";
					close;
					break;
			}
		case 2: // I don't have a soulmate just yet
			mes .n$;
			mes "Wh-what?";
			mes "How is that even possible?";
			mes "We must put an end to it! I can help you with that.";
			next;
			mes .n$;
			mes "What about we send a public message to the crush?";
			next;
			mes .n$;
			mes "We have a list of very hot pre-made messages we can broadcast so, next time you see the crush online just send me a message with their name and I will do the rest.";
			next;
			mes .n$;
			mes "Send a message to ^0000FFnpc:cupido^000000 with the `target's` name.";
			mes "That will cost you " + F_InsertComma(.Cantadas) + "z per message sent.";
			mes "Premmys pay half-price!";
			close;
			break;
	}
	end;

OnDebug:
OnMinute03:
OnMinute13:
OnMinute23:
OnMinute33:
OnMinute43:
OnMinute53:
	// if(gettime(DT_MONTH) != 6)
	// 	end;
	// if(gettime(DT_DAYOFMONTH) < 12)
	// 	end;

	.@max = 0;
	freeloop(1);
	for(.@j=0;.@j<getarraysize($Val_type);.@j++)
		if($Val_type[.@j] == 2) 
			if(isloggedin($Val_Acc[.@j],$Val_Dest[.@j]))
				.@max++;

	freeloop(0);

	if(.@max > 6)
		set .@max, 6;
	
	if(.@max == 0)
		end;

	freeloop(1);
	for(.@j=0;.@j<getarraysize($Val_Dest);.@j++) {
		if($Val_type[.@j] == 2) {
			.@acc			= $Val_Acc[.@j];
			.@dest			= $Val_Dest[.@j];
			if(isloggedin(.@acc,.@dest)) {
				.@dest$ 	= strcharinfo(0,.@dest);
				.@sender$ 	= $Val_Sender$[.@j];
				.@message$ 	= getd(".Mes"+$Val_Mes$[.@j]+"$");
				.@music$ 	= $Val_music[.@j] + 1000;

				callfunc "F_ClearRecords",.@j;

				sleep 15000;
				attachrid(.@acc);
				cutin "namo.bmp",2;
				detachrid;
				playBGMall .@music$;
				sleep 5000;
				explode(.@mes$,.@message$,",");
				announce "*~ "+ .@dest$ +" ~*",bc_all,0xff6699,fw_normal,32;
				sleep 10000;
				for(.@k=0;.@k<getarraysize(.@mes$);.@k++){
					specialeffect2 1038,AREA,.@dest$;
					announce .@mes$[.@k],bc_all,0xff6699,fw_normal,32;
					sleep 15000;
				}
				announce "- " + .@sender$,bc_all,0xff6699,fw_normal,32;
				setarray .@mailitem[0], 14546,12062,12235,5548;
				setarray .@mailamount[0],   5,    3,    3,   1;
				mail .@dest,.@sender$,"Feliz dia dos Namorados 2020",.@message$,0,.@mailitem,.@mailamount;
				sleep 5000;
				// getitem 12062,3,.@acc;
				// getitem 12235,3,.@acc;
				// getitem 5548,1,.@acc;
				deletearray .@mes$[0],getarraysize(.@mes$);
				attachrid(.@acc);
				cutin "",255;
				detachrid;
			}
		}
	}
	freeloop(0);
	end;

OnWhisperGlobal:
	.@name$ = escape_sql(@whispervar0$);
	if(getcharid(3,.@name$) > 0) {
		TakeZeny(.Cantadas,1);
		announce .@name$ + ", " + .Cantadas$[rand(getarraysize(.Cantadas$))] + " - " + strcharinfo(0),bc_all,0xff6699,fw_normal,26;
		dispbottom "[ Romantic Post Officer ] : Payment accepted. Message sent.";
	} else {
		dispbottom "[ Romantic Post Officer ] : Hum... I can't find that person in our registrar! Please try again.";
	}
	end;

OnInit:
	.n$ = "^AA00AA[ Romantic Mail Officer ]^000000";
	.LovePostcard = 500000;
	.BroadCastOfLove = 5000000;
	.RomanticLootbox = 10000000;
	.Cantadas = 200000;
	.Mes1$ = "Não há maior felicidade neste mundo que comemorar com você mais um dia dos namorados, Independentemente da troca de presentes, o mais importante desta data é relembrar todos os bons momentos que já vivemos juntos, Feliz Dia dos Namorados meu amor!";
	.Mes2$ = "Você é a melhor companhia do mundo, me provoca múltiplos sorrisos e me encanta sempre com palavras lindas, Tenho certeza que fomos feitos um para o outro e não há ninguém melhor que você para passar o resto da minha vida, Feliz Dia dos Namorados meu amor!";
	.Mes3$ = "Cada segundo ao seu lado é um tempo bem aproveitado, pois com você sou feliz como nunca fui, Espero que o nosso amor dure para sempre e que qualquer obstáculo possa ser facilmente superado, Jamais esqueça que você é o amor da minha vida! Feliz Dia dos Namorados meu amor!";
	.Mes4$ = "No nosso primeiro dia dos namorados juntos e espero que os nossos laços jamais sejam desfeitos, pois você desperta tudo de melhor que há em mim, e hoje já não imagino minha vida sem você ao meu lado, Vamos fazer deste o melhor de todos os dias dos namorados.";
	setarray .Cantadas$[0],"Você tem um mapa? Porque me perdi no brilho dos seus olhos.";
	setarray .Cantadas$[1],"Não sou carro, mas sou Para ti.";
	setarray .Cantadas$[2],"Eu não sou a Casas Bahia, mas prometo dedicação total a você.";
	setarray .Cantadas$[3],"Estou fazendo uma campanha de doação de órgãos! Então, não quer doar seu coração pra mim?";
	setarray .Cantadas$[4],"Você não é pescoço mais mexeu com a minha cabeça!";
	setarray .Cantadas$[5],"Me chama de Baphomet e deixa eu te possuir.";
	setarray .Cantadas$[6],"Você é o ovo que faltava na minha marmita.";
	setarray .Cantadas$[7],"Casando comigo você nunca vai pegar na enxada! Só vai ter que fazer amor desde a manhã até a madrugada!";
	setarray .Cantadas$[8],"Me chama de tabela periódica e diz que rola uma química entre nós.";
	setarray .Cantadas$[9],"Seu pai é dono da Latam? Como é que faz pra ter um avião desse em casa?";
	setarray .Cantadas$[10],"Entre Star Wars e Star trek, o que eu queria mesmo era star com você.";
	setarray .Cantadas$[11],"Eu não sou plano de saúde, mas se você quiser eu acabo com a sua carência.";
	setarray .Cantadas$[12],"Seu pai é mecânico? Porque você é uma graxinha!";
	end;

	function	TakeZeny	{
		.@op = getarg(1,0);
		if((vip_status(1) && Zeny < getarg(0)/2) || (!vip_status(1) && Zeny < getarg(0))) {
			if(.@op) {
				dispbottom "Oh dear, it seems like you don't have enough Zenys for that!";
				end;
			} else {
				mes "Oh dear, it seems like you don't have enough Zenys for that!";
				close;
			}
		}
		if(vip_status(1)) set Zeny, Zeny - (getarg(0)/2);
		else set Zeny, Zeny - getarg(0);
		return;
	}




	function	QueueEvent	{
		.@type		= getarg(0);
		.@acc 		= getarg(1);
		.@char 		= getarg(2);
		.@message$ 	= getarg(3);
		.@sender$ 	= strcharinfo(0);
		.@music 	= getarg(4,1);

		set .@i, getarraysize($Val_Dest);
		setarray $Val_Dest[.@i],.@char;
		setarray $Val_Acc[.@i],.@acc;
		setarray $Val_Mes$[.@i],.@message$;
		setarray $Val_Sender$[.@i],.@sender$;
		setarray $Val_type[.@i],.@type;
		setarray $Val_music[.@i],.@music;
		return;
	}
}


-	script	DiaDosNamorados	-1,{
	end;

OnPCLoginEvent:
	if(gettime(DT_MONTH) != 6)
		end;
	if(gettime(DT_DAYOFMONTH) < 12)
		end;
CheckAgain:
	if(.@i = inarray($Val_Dest,getcharid(0)) == -1) {
		end;
	} else {
		switch($Val_Type[.@i]) {
			case 1:
				.@sender$ = $Val_Sender$[.@i];
				.@message$ = $Val_Mes$[.@i];
				callfunc "F_ClearRecords",.@i;
				cutin "namo.bmp",4;
				mes "^AA00AA"+strcharinfo(0)+",^000000";
				mes " ";
				mes "^0000AA"+.@message$+"^000000";
				mes " ";
				mes "^0000FF     - "+.@sender$+"^000000";
				setarray .@mailitem[0], 14546;
				setarray .@mailamount[0],   5;
				mail getcharid(0),.@sender$,"Feliz dia dos Namorados 2020",.@message$,0,.@mailitem, .@mailamount;
				if(inarray($Val_Dest,getcharid(0)) == -1) {
					close2;
					cutin "",255;
					end;
				} else {
					next;
					cutin "",255;
					goto CheckAgain;
				}
				break;
			case 3:
				.@sender$ = $Val_Sender$[.@i];
				.@message$ = $Val_Mes$[.@i];
				callfunc "F_ClearRecords",.@i;
				mes "[ Postman Officer ]";
				mes "Special delivery for ^AA00AA"+strcharinfo(0)+"^000000!";
				next;
				cutin "namo.bmp",4;
				mes "^AA00AA"+strcharinfo(0)+",^000000";
				mes " ";
				mes "^0000AA"+.@message$+"^000000";
				mes " ";
				mes "^0000FF     - "+.@sender$+"^000000";
				next;
				mes "[ Postman Officer ]";
				mes "Don't forget to check your RodEX, I left something for you in there.";
				setarray .@mailitem[0], 22777,14546;
				setarray .@mailamount[0], 1,5;
				mail getcharid(0),.@sender$,"Feliz dia dos Namorados 2020",.@message$,0,.@mailitem, .@mailamount;
				if(inarray($Val_Dest,getcharid(0)) == -1) {
					close2;
					cutin "",255;
					end;
				} else {
					next;
					cutin "",255;
					goto CheckAgain;
				}
				break;
		}
	}
	end;
}

function	script	F_ClearRecords	{
	.@i = getarg(0);
	deletearray $Val_Dest[.@i],1;
	deletearray $Val_Acc[.@i],1;
	deletearray $Val_Mes$[.@i],1;
	deletearray $Val_Sender$[.@i],1;
	deletearray $Val_type[.@i],1;
	deletearray $Val_music[.@i],1;
	return;
}