-	script	fullloot	-1,{
	function full_loot; function	clear_vars;
    end;

OnInit:
    .FL_BOUND_BREAK_CHANCE = 40;	// % Chance bound items will break when killed by a player
    .FL_ARMOR_BREAK_CHANCE = 30;	// % Chance bound items will break when killed by a player
    .FL_WEAPON_BREAK_CHANCE = 30;	// % Chance bound items will break when killed by a player
    end;

OnCollectLootBag:
    freeloop(1);
    for(.@c=0;.@c<getarraysize(getd("$@fl_"+killedgid+"_iid"));.@c++) {
        .@identify = .@refine = .@attribute = .@card1 = .@card2 = .@card3 = .@card4 = 0;
        cleararray .@RandomIDArray[0],0,5;
        cleararray .@RandomValueArray[0],0,5;
        cleararray .@RandomParamArray[0],0,5;
        //before we start, lets check this item is an equip and if so, lets check it has or not random attributes
        //if it does, we need to put them all in arrays
        if(getiteminfo(getd("$@fl_"+killedgid+"_iid["+.@c+"]"),2) == 4 || getiteminfo(getd("$@fl_"+killedgid+"_iid["+.@c+"]"),2) == 5) {
            if(getd("$@fl_"+killedgid+"_oi1["+.@c+"]") > 0) {
                for(.@i=1;.@i<6;.@i++) {
                    setarray .@RandomIDArray[.@i],getd("$@fl_"+killedgid+"_oi"+.@i+"["+.@c+"]");
                    setarray .@RandomValueArray[.@i],getd("$@fl_"+killedgid+"_ov"+.@i+"["+.@c+"]");
                    setarray .@RandomParamArray[.@i],getd("$@fl_"+killedgid+"_op"+.@i+"["+.@c+"]");
                }
            }
            .@identify	= 1;
            .@refine	= getd("$@fl_"+killedgid+"_ref["+.@c+"]");
            .@attribute	= 1;
            .@card1		= getd("$@fl_"+killedgid+"_rca1["+.@c+"]");
            .@card2		= getd("$@fl_"+killedgid+"_rca2["+.@c+"]");
            .@card3		= getd("$@fl_"+killedgid+"_rca3["+.@c+"]");
            .@card4		= getd("$@fl_"+killedgid+"_rca4["+.@c+"]");
            
        }
        .@itemid	= getd("$@fl_"+killedgid+"_iid["+.@c+"]");
        .@amount	= getd("$@fl_"+killedgid+"_qtt["+.@c+"]");
        
        getitem3 .@itemid,.@amount,.@identify,.@refine,.@attribute,.@card1,.@card2,.@card3,.@card4,.@RandomIDArray,.@RandomValueArray,.@RandomParamArray;
        //sleep2 1000;
    }
    freeloop(0);
	clear_vars(killedgid);
    end;

	OnPCLoadMapEvent:
		if(getmapflag(strcharinfo(3),mf_fullloot) && !getmapflag(strcharinfo(3),mf_hellgate) && !getmapflag(strcharinfo(3),mf_map_tier)) {
			announce "/!\\ F u l l - L o o t   P v P   A r e a /!\\",bc_self,0xFF0000,FW_BOLD,32,0;
			dispbottom "You will lose everything in your inventory may you die to another player",0xFFFFFF;
		}
		end;

	OnPcDieEvent:
		if(getmapflag(strcharinfo(3),mf_fullloot)){
			if(killerrid > 100000000) end;
			full_loot();
			sleep2 200;
			warp "SavePoint",0,0;
			percentheal 100,100;
		}
		end;

	function	full_loot	{
		if(!getmapflag(strcharinfo(3),mf_fullloot)) 
			return;

		// Killer gets 1% of kill's total zeny (except in hell gates)
		if(!getmapflag(strcharinfo(3),mf_hellgate) && killerrid) {
			.@z = Zeny/100;
			set Zeny, Zeny + .@z,getcharid(0,rid2name(killerrid));
			dispbottom "You've got " + callfunc("F_InsertComma",.@z) + "z from " + strcharinfo(0),0xFAFA00,getcharid(0,rid2name(killerrid));
			set Zeny, Zeny - .@z;
		}

		//Lets get the deceased's inventory list.
		getinventorylist;

		// nothing in the inventory? just return...
		if(@inventorylist_count == 0 )
			return;

		//Lets get the dead body location...
		getmapxy(.@map$, .@x, .@y, BL_PC);

		//... spawn a bag...
		.@n$ = strcharinfo(0) + "'s Body";
		monster .@map$,.@x,.@y,.@n$,25006,1,"fullloot::OnCollectLootBag";
		.@gid$ = "fl_" + $@mobid[0];
		
		//Now we transfer the list of items and their quantity to an array on the instance
		//because getinvetorylist will generate the arrays on the dead char.
		for(.@c=0;.@c<@inventorylist_count;.@c++) {

			//Debug
			// for(.@i=0;.@i<16;.@i++)
			// 	debugmes "item info "+.@i+" : " + getiteminfo(@inventorylist_id[.@c],.@i);

			//items with weight 0 never drop
			if(getiteminfo(@inventorylist_id[.@c],6) == 0)
				continue;

			//Costumes never break, never drop
			switch (getiteminfo(@inventorylist_id[.@c],5)) {
				case 1024: case 2048: case 4096: case 8192:
					continue;
					break;	
			}

			//As said, costumes never break, never drop. Even the ones created via NPC
			if(@inventorylist_card1[.@c] == 254 && @inventorylist_card2[.@c] == 0 && @inventorylist_card3[.@c] == 16959 && @inventorylist_card4[.@c] == 15) 
				continue;

			//Rental Equips always break
			if (@inventorylist_expire[.@c] > 0) {
				delitem @inventorylist_id[.@c],@inventorylist_amount[.@c];
				continue;
			}

			//Bound items sometimes break (40% chance)
			if (@inventorylist_bound[.@c] > 0) {
				if(.@r = rand(100) <= .FL_BOUND_BREAK_CHANCE) {
					delitem @inventorylist_id[.@c],@inventorylist_amount[.@c];
					continue;
				}
			}

			//Armor have a chance to break...
			if(getiteminfo(@inventorylist_id[.@c],2) == 4) {
				if(.@r = rand(100) <= .FL_ARMOR_BREAK_CHANCE) {
					delitem @inventorylist_id[.@c],1;
					continue;
				}
			}
			//...So does the weapon
			if(getiteminfo(@inventorylist_id[.@c],2) == 5) {
				if(.@r = rand(100) <= .FL_WEAPON_BREAK_CHANCE) {
					delitem @inventorylist_id[.@c],1;
					continue;
				}
			}
			setarray getd("$@"+.@gid$ +"_iid["+.@id+"]"), @inventorylist_id[.@c];
			setarray getd("$@"+.@gid$ +"_qtt["+.@id+"]"), @inventorylist_amount[.@c];
			setarray getd("$@"+.@gid$ +"_ref["+.@id+"]"), @inventorylist_refine[.@c];
			setarray getd("$@"+.@gid$ +"_ca1["+.@id+"]"), @inventorylist_card1[.@c];
			setarray getd("$@"+.@gid$ +"_ca2["+.@id+"]"), @inventorylist_card2[.@c];
			setarray getd("$@"+.@gid$ +"_ca3["+.@id+"]"), @inventorylist_card3[.@c];
			setarray getd("$@"+.@gid$ +"_ca4["+.@id+"]"), @inventorylist_card4[.@c];
			setarray getd("$@"+.@gid$ +"_bnd["+.@id+"]"), @inventorylist_bound[.@c];
			setarray getd("$@"+.@gid$ +"_oi1["+.@id+"]"), @inventorylist_option_id1[.@c];
			setarray getd("$@"+.@gid$ +"_ov1["+.@id+"]"), @inventorylist_option_value1[.@c];
			setarray getd("$@"+.@gid$ +"_op1["+.@id+"]"), @inventorylist_option_parameter1[.@c];
			setarray getd("$@"+.@gid$ +"_oi2["+.@id+"]"), @inventorylist_option_id2[.@c];
			setarray getd("$@"+.@gid$ +"_ov2["+.@id+"]"), @inventorylist_option_value2[.@c];
			setarray getd("$@"+.@gid$ +"_op2["+.@id+"]"), @inventorylist_option_parameter2[.@c];
			setarray getd("$@"+.@gid$ +"_oi3["+.@id+"]"), @inventorylist_option_id3[.@c];
			setarray getd("$@"+.@gid$ +"_ov3["+.@id+"]"), @inventorylist_option_value3[.@c];
			setarray getd("$@"+.@gid$ +"_op3["+.@id+"]"), @inventorylist_option_parameter3[.@c];
			setarray getd("$@"+.@gid$ +"_oi4["+.@id+"]"), @inventorylist_option_id4[.@c];
			setarray getd("$@"+.@gid$ +"_ov4["+.@id+"]"), @inventorylist_option_value4[.@c];
			setarray getd("$@"+.@gid$ +"_op4["+.@id+"]"), @inventorylist_option_parameter4[.@c];
			setarray getd("$@"+.@gid$ +"_oi5["+.@id+"]"), @inventorylist_option_id5[.@c];
			setarray getd("$@"+.@gid$ +"_ov5["+.@id+"]"), @inventorylist_option_value5[.@c];
			setarray getd("$@"+.@gid$ +"_op5["+.@id+"]"), @inventorylist_option_parameter5[.@c];

			//We cant forget to delete the items from the dead char:				
			delitem @inventorylist_id[.@c],@inventorylist_amount[.@c];
			
			//Then we increase our counter for the next record in the array
			.@id++;
		}
		
		return;
	}


	function	clear_vars	{
		.@gid$ = "fl_" + getarg(0);
		.@sz = getarraysize(getd("$@"+.@gid$ +"_iid"));
		deletearray getd("$@"+.@gid$ +"_iid[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_qtt[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ref[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ca1[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ca2[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ca3[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ca4[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_bnd[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_oi1[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ov1[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_op1[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_oi2[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ov2[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_op2[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_oi3[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ov3[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_op3[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_oi4[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ov4[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_op4[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_oi5[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_ov5[0]"), .@sz;
		deletearray getd("$@"+.@gid$ +"_op5[0]"), .@sz;
		
		return;
	}
}