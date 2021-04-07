 //============================================================
//= Main Refiner Function
//============================================================
//= To allow auto safe refining/multiple refining set the
//= @features variable to 1
//============================================================
function script refinemain {
	set @features,1;
	mes "[" + @name$ + "]";
	mes "I am the Armsmith... I can refine any weapon or piece of armor you choose!";
	mes "Which piece of equipment do you want to refine?";
	M_Menu:
	next;
	menu getequipname(1),PART1,getequipname(2),PART2,getequipname(3),PART3,getequipname(4),PART4,getequipname(5),PART5,
	getequipname(6),PART6,getequipname(7),PART7,getequipname(8),PART8,getequipname(9),PART9,getequipname(10),PART10;
	//Head Gear
	PART1:
	set @part,1;
	if (getequipisequiped(1)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "Do you want me to refine your dumb brain?";
	emotion 6;
	goto M_Menu;
	//Armor
	PART2:
	set @part,2;
	if (getequipisequiped(2)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "Do you want me to melt your body with blazing heat...?";
	emotion 6;
	goto M_Menu;
	//Left Hand
	PART3:
	set @part,3;
	if (getequipisequiped(3)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "I can't make your left hand into an ultimate weapon...";
	emotion 4;
	goto M_Menu;
	//Right Hand
	PART4:
	set @part,4;
	if (getequipisequiped(4)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "I can't make your right hand into an ultimate weapon...";
	emotion 4;
	goto M_Menu;
	//Garment
	PART5:
	set @part,5;
	if (getequipisequiped(5)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "Look here... you don't have any Garments on...";
	goto M_Menu;
	//Foot Gear
	PART6:
	set @part,6;
	if (getequipisequiped(6)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "Ack!! Those are some stinky feet. I definitely can't refine those... uck!!";
	emotion 16;
	goto M_Menu;
	//Accessory1
	PART7:
	set @part,7;
	if (getequipisequiped(7)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "What do you mean by Accessory? Which One?";
	emotion 20;
	goto M_Menu;
	//Accessory2
	PART8:
	set @part,8;
	if (getequipisequiped(8)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "What do you mean by Accessory? Which One?";
	emotion 20;
	goto M_Menu;
	PART9:
	set @part,9;
	if (getequipisequiped(9)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "What do you want from me? There's nothing equipped there...";
	emotion 20;
	goto M_Menu;
	PART10:
	set @part,10;
	if (getequipisequiped(10)) goto CHECK1;
	mes "[" + @name$ + "]";
	mes "What do you want from me? There's nothing equipped there...";
	emotion 20;
	goto M_Menu;
	//Check if the item is refinable...
	CHECK1:
	if(getequipisenableref(@part)) goto CHECK2;
	mes "[" + @name$ + "]";
	mes "I can't work on this item...";
	close;
	//Check if the item is identified... (Don't know why this is in here... but kept it anyway)
	CHECK2:
	if(getequipisidentify(@part)) goto CHECK3;
	mes "[" + @name$ + "]";
	mes "You must appraise this item first.";
	close;
	//Check to see if the items is already +10
	CHECK3:
	if(getequiprefinerycnt(@part) < 10) goto REFINE0;
	mes "[" + @name$ + "]";
	mes "This weapon is already at its maximum level and can no longer be refined.";
	close;
	//Refine Armor
	REFINE0:
	set @refineitemid, getequipid(@part); // save id of the item
	set @refinerycnt, getequiprefinerycnt(@part); //save refinery count
	if(getequipweaponlv(@part) > 0) goto REFINE1;
	set @matname$,"Elunium";
	set @material,985;
	set @price,2000;
	set @safe,4;
	if(@features == 1) goto L_refinefeatures;
	goto L_refinenormal;
	//Refine Level 1 Weapon
	REFINE1:
	if(getequipweaponlv(@part) > 1) goto REFINE2;
	set @matname$,"Phracon";
	set @material,1010;
	set @price,50;
	set @safe,7;
	if(@features == 1) goto L_refinefeatures;
	goto L_refinenormal;
	//Refine Level 2 Weapon
	REFINE2:
	if(getequipweaponlv(@part) > 2) goto REFINE3;
	set @matname$,"Emveretarcon";
	set @material,1011;
	set @price,200;
	set @safe,6;
	if(@features == 1) goto L_refinefeatures;
	goto L_refinenormal;
	//Refine Level 3 Weapon
	REFINE3:
	if(getequipweaponlv(@part) > 3) goto REFINE4;
	set @matname$,"Oridecon";
	set @material,984;
	set @price,5000;
	set @safe,5;
	if(@features == 1) goto L_refinefeatures;
	goto L_refinenormal;
	//Refine Level 4 Weapon
	REFINE4:
	set @matname$,"Oridecon";
	set @material,984;
	set @price,20000;
	set @safe,4;
	if(@features == 1) goto L_refinefeatures;
	goto L_refinenormal;
	L_refinenormal:
	mes "[" + @name$ + "]";
	mes "To refine this stuff, I need ^ff9999" + @matname$ + "^000000 and the fee " + @price + " Zeny.";
	mes "Continue?";
	next;
	menu "Yes",-,"No",Lcancel;
	if (getequippercentrefinery(@part) == 100) goto L_Sub;
	mes "[" + @name$ + "]";
	mes "Hmm... Hold on! This piece of equipment has already been refined to its maximum safety level.";
	mes "I must warn you if it is refined ANYMORE, It could be DESTROYED and become USELESS!!";
	next;
	mes "["+@name$+"]";
	mes "Do you still wish you refine it? If so I will not be able to guarantee my work...";
	next;
	menu "Yes",-,"No",Lcancel1;
	L_Sub:
	if ((countitem(@material) < 1) || (Zeny < @price)) goto Lcancel2;
	set Zeny,Zeny-@price;
	delitem @material,1;
	Lrefine:
	if (getequipisequiped(@part) == 0) goto LNoItem; // hacker has removed the item (not changed, why?)
	if (getequipid(@part) != @refineitemid) goto LNoFake; // hacker has changed the item
	if (getequiprefinerycnt(@part) != @refinerycnt) goto LNoFake; // hacker has changed the item
	if (getequippercentrefinery(@part) <= rand(100)) goto Lfail;
	mes "["+@name$+"]";
	mes "Clang! Clang! Clang!";
	successrefitem @part;
	next;
	mes "["+@name$+"]";
	mes "HAHA! It seems my skills haven't gotten rusty yet! Splendid... just splendid...";
	emotion 21;
	close;
	Lfail:
	mes "[" + @name$ + "]";
	mes "Clang! Clang! Clang!";
	failedrefitem @part;
	next;
	mes "["+@name$+"]";
	mes "Aaahhh!! Oh no...!!";
	emotion 16;
	next;
	mes "["+@name$+"]";
	mes "Eh..Ehem... I'm sorry but the refining process ^ff0000failed^000000.";
	next;
	mes "["+@name$+"]";
	mes "I am deeply ashamed of what I've done... but I DID warn you earlier about the risks.";
	close;
	LNoItem:
	mes "[" + @name$ + "]";
	mes "Look here... you don't have any Items on...";
	close;
	LNoFake:
	mes "[" + @name$ + "]";
	mes "Clan... No, but Did you imagine I could be so stupid !?!";
	mes "You have changed it...";
	mes "Go out before I stun you with my Hammer!!!";
	close;
	Lcancel:
	mes "[" + @name$ + "]";
	mes "You said so..Hmm so be it...";
	close;
	Lcancel1:
	mes "[" + @name$ + "]";
	mes "Good Choice.";
	mes "Ah... good choice. I'd feel awful if I'd destroyed another persons piece of equipment with my own hands.";
	close;
	Lcancel2:
	mes "[" + @name$ + "]";
	mes "Is that all you got? Unfortunately I can't work for you at a lower price. Try putting yourself in my shoes.";
	close;
	// New Refining Functions ========================
	L_refinefeatures:
	if(getequiprefinerycnt(@part) >= @safe) goto Lnosafe;
	mes "[" + @name$ + "]";
	mes "I can refine this to the safe limit or a desired number of times... it's your choice...";
	next;
	menu "To the safe limit please.",Lsafe,"I'll decide how many times.",Lnosafe,"I've changed my mind...",Lcancel;
	Lsafe:
	set @refinecnt,@safe - getequiprefinerycnt(@part);
	set @fullprice,@price * @refinecnt;
	mes "[" + @name$ + "]";
	mes "That will cost you " + @refinecnt + " " + @matname$ + " and " + @fullprice + " Zeny. Is that ok?";
	next;
	menu "Yes",-,"No...",Lcancel;
	if((countitem(@material) < @refinecnt) || (Zeny < @fullprice)) goto Lcancel2;
	set Zeny,Zeny - @fullprice;
	delitem @material,@refinecnt;
	goto L_refinesafe;
	Lnosafe:
	mes "[" + @name$ + "]";
	mes "So how many times would you like me to refine your item?";
	next;
	input @refinecnt;
	if (@refinecnt<1) goto Lcancel3; //fixed by Lupus
	set @refinecheck,@refinecnt + getequiprefinerycnt(@part);
	if(@refinecheck > 10) goto Lcancel3;
	set @fullprice,@price * @refinecnt;
	mes "[" + @name$ + "]";
	mes "This will cost you " + @refinecnt + " " + @matname$ + " and " + @fullprice + " Zeny... Is that ok?";
	next;
	menu "Yes...",-,"No...",Lcancel;
	if(@refinecheck > @safe) goto Lwarn;
	if((countitem(@material) < @refinecnt) || (Zeny < @fullprice)) goto Lcancel2;
	set Zeny,Zeny - @fullprice;
	delitem @material,@refinecnt;
	goto L_refinenumber;
	end;
	Lwarn:
	set @refinecheck,@refinecheck - @safe;
	mes "[" + @name$ + "]";
	mes "This will try to refine the equipment " + @refinecheck + " times past the safe limit. Your equipment may be destroyed... is that ok?";
	next;
	menu "Yes",-,"No...",Lcancel1;
	if((countitem(@material) < @refinecnt) || (Zeny < @fullprice)) goto Lcancel2;
	set Zeny,Zeny - @fullprice;
	delitem @material,@refinecnt;
	goto L_refinenumber;
	Lcancel3:
	mes "[" + @name$ + "]";
	mes "I can't refine this item that many times.";
	close;
	// SubFunction: Safe Refine ---------------------
	L_refinesafe:
	if (getequipisequiped(@part) == 0) goto LNoItem; // hacker has removed the item (no changed, why?)
	if (getequipid(@part) != @refineitemid) goto LNoFake; // hacker has changed the item
	if (getequippercentrefinery(@part) < 100) goto LNoFake; // hacker has changed the item (it is not safe anymore)
	mes "Clang, clang!!!";
	successrefitem @part;
	emotion 21;
	set @refinecnt,@refinecnt - 1;
	next;
	if(@refinecnt == 0) goto Lend;
	goto L_refinesafe;
	Lend:
	mes "[" + @name$ + "]";
	mes "All finished... Come again soon.";
	close;
	// SubFunction: Refine
	L_refinenumber:
	if (getequipisequiped(@part) == 0) goto LNoItem; // hacker has removed the item (no changed, why?)
	if (getequipid(@part) != @refineitemid) goto LNoFake; // hacker has changed the item
	mes "Clang, clang!!!";
	if (getequippercentrefinery(@part)<=rand(100)) goto Lfail_number;
	successrefitem @part;
	emotion 21;
	set @refinecnt,@refinecnt - 1;
	next;
	if(@refinecnt == 0) goto Lend;
	goto L_refinenumber;
	Lfail_number:
	failedrefitem @part;
	emotion 23;
	mes "[" + @name$ + "]";
	mes "WAHHHH!!! I'm so sorry... I warned you this could happen...";
	set @refinecnt,@refinecnt - 1;
	if(@refinecnt == 0) goto Lend2;
	mes "Here's the unused Zeny and Material back...";
	getitem @material,@refinecnt;
	set @fullprice,@refinecnt * @price;
	set Zeny,Zeny + @fullprice;
	Lend2:
	close;
}