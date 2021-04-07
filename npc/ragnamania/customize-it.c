// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// eAmod Project - Scripts
// --------------------------------------------------------------------------
// Script Name : Headgear to Costume converter
// --------------------------------------------------------------------------
// Description :
// Allows a user to convert the equipped headgear (on Top, Mid or Low) into a
// costume item. It will remove any card and refine of the Item.
// --------------------------------------------------------------------------

-	script	CostumeIT	-1,{
	mes "[Luna]";
	mes "Olá e bem-vindo ao Atelier da Luna.";
	mes "Aqui você pode converter seus equipamentos de cabeça em visuais.";
	mes "O custo do meu serviço é 10.000 Manias ou 100 "+getitemname(675)+"s.";
	next;
	mes "[Luna]";
	mes "Por favor, selecione qual equipamento você deseja converter.";
	mes "Lembre-se que todas as cartas e qualquer refinamento serão perdidos no processo.";
	next;

	setarray .@Position$[1],"Top","Mid","Low";
	setarray .@Position[1],6,5,4;

	set .@Menu$,"";
	for( set .@i, 1; .@i < 4; set .@i, .@i + 1 )
	{
		if( getequipisequiped(.@Position[.@i]) )
			set .@Menu$, .@Menu$ + .@Position$[.@i] + "-" + "[" + getequipname(.@Position[.@i]) + "]";

		set .@Menu$, .@Menu$ + ":";
	}

	set .@Part, .@Position[ select(.@Menu$) ];
	if( !getequipisequiped(.@Part) )
	{
		mes "[Luna]";
		mes "Você não está usando nada neste slot...";
		close;
	}

	mes "[Luna]";
	mes "Você deseja transformar o seu " + getitemname(getequipid(.@Part)) + " em um item visual?";
	next;
	if( select("Sim, prossiga:Não, eu mudei de ideia.") == 2 )
	{
		mes "[Luna]";
		mes "Precisa de um tempo pra pensar, huh?";
		mes "Sem problemas, eu compreendo.";
		close;
	}
	mes "[Luna]";
	mes "Você pode pagar pelos meus serviços com Manias ou com "+ getitemname(675)+"s.";
	mes "Como você gostaria de pagar?";
	next;
	if(select("Pagar com Manias:Pagar com "+getitemname(675)+"s") == 1) {
		if(#CASHPOINTS < 10000 ) {
			mes "[Luna]";
			mes "Infelizmente você não possui Manias o suficiente.";
			close;
		} else {
			set #CASHPOINTS, #CASHPOINTS - 10000;
			.@temp = getitemname(getequipid(.@Part));
			costume .@Part; // Convert the Headgear
			mes "[Luna]";
			mes "Prontinho, espero que você goste do seu novo visual!";
			logmes "[Luna] Criou o visual " + getitemname(.@temp) + " por Manias";
			close;
		}
	} else {
		if(countitem(675) < 100 ) {
			mes "[Luna]";
			mes "Infelizmente você não possui "+getitemname(675)+"s o suficiente.";
			close;
		} else {
			delitem 675, 100;
			.@temp = getitemname(getequipid(.@Part));
			costume .@Part; // Convert the Headgear
			mes "[Luna]";
			mes "Prontinho, espero que você goste do seu novo visual!";
			logmes "[Luna] Criou o visual " + getitemname(.@temp) + " por Hunting Coins";
			close;
		}
	}
}

// --------------------------------------------------------------------------
// Use duplicates to put your npc on different cities
// --------------------------------------------------------------------------
prontera,166,201,6	duplicate(CostumeIT)	Luna#1	666
