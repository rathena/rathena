// ********************************************************************
// Gardener
// ********************************************************************

rentb1,27,12,4	script	Maia	1903	,{
    function    BuildPlantingMenu; function	Planta; function	Harvest;
    'gardenerwalk = 1;//'
    npcstop;
	if (atoi(#HOUSING$[0]) != 'HouseNumber) {//'
		mes "[ Maia ]";
		mes "Sorry, I can't speak right now...";
		mes "... may the boss see us and I don't want problems. I get paid by hour, you know.";
		close;
	} else {
	L_Start:
		mes "[ Maia ]";
		mes "Hello, " +strcharinfo(0)+ "!";
		mes "Can I help you?";
		next;
		switch(prompt("Plant Seeds:Give Maia some tips:^FF0000Cancel^000000")) {
        case 1: //Plant
		Plant:
            mes "[ Maia ]";
            mes "Right! Did you get the seeds?";
			next;
            if(!BuildPlantingMenu())
				goto OnDismiss;
			.@op = select(@menu$) -1;
			if(.@op == getarraysize(@itens))
				goto L_Start;

			.@i = inarray(.Seeds,@itens[.@op]);
			if(atoi(#HOUSING_CROPS$[.@i]) > 0){
				mes "[ Maia ]";
				mes "Oh, I am afraid we don't have more room to plant any ^00AA00"+getitemname(@itens[.@op])+"^000000. We should harvest the ones we've got planted before we can plant more.";
				next;
				goto Plant;
			}
			mes "[ Maia ]";
			mes "Great!";
			mes "We are planting some ^00AA00"+ getitemname(@itens[.@op])+ "^000000!";
			mes "I love them!";
			if(countitem(@itens[.@op]) < 1)
				goto OnDismiss;

			delitem @itens[.@op],1;
			//setarray #HOUSING_CROPS$[.@i], gettimetick(2) + (rand(1)?86400:43200);
			//Debug
			setarray #HOUSING_CROPS$[.@i], gettimetick(2) + (rand(1)?300:300);
			setarray #HOUSING_CROPS_QTY$[.@i], 4;
			next;
			mes "[ Maia ]";
			mes "I am gonna start with it right now! Please give it some time for mother nature to do her magic.";
			close2;
			Planta(.@i);
			end;
			break;
		case 2:
			mes "[ "+strcharinfo(0)+" ]";
			mes "Here, please take this...";
			mes "I know it is not much";
			mes "but hope it helps.";
			input .@tips;
			next;
			if(Zeny < .@tips) {
				mes "[ Maia ]";
				mes "But ...";
				mes "Seriously?";
				mes "... Thank you anyway.";
				next;
				emotion ET_SHY;
			} else {
				mes "[ Maia ]";
				mes "Oh, Are you sure?";
				mes "It is very kind of you!";
				mes "Thank you very much, "+strcharinfo(0);
				set Zeny, Zeny - .@tips;
				set #HOUSING_TIPS$[0], atoi(#HOUSING_TIPS$[0]) + .@tips;
				next;
				emotion ET_BEST;
			}
			goto L_Start;
			break;
		case 4: //Bye
			mes "[ "+strcharinfo(0)+" ]";
			mes "Nah, Thanks...";
			mes "Talk to you later, ok?";
            close2;
		case 255:
        default:
            'gardenerwalk = 0; //'
			end;
            break;
		}
	}
	
OnEnable:
	enablenpc instance_npcname(strnpcinfo(0));
	donpcevent instance_npcname("weed")+"::OnDisable";
	donpcevent instance_npcname("weed#2")+"::OnDisable";
	donpcevent instance_npcname("weed#3")+"::OnDisable";
	donpcevent instance_npcname("weed#4")+"::OnDisable";
	donpcevent instance_npcname("weed#5")+"::OnDisable";
	donpcevent instance_npcname("weed#6")+"::OnDisable";
    npcspeed 200;

OnWalk:
    while(1) {
		sleep 30000;
		if('gardenerwalk == 0) { //'
			getmapxy(.@m$,.@x,.@y,BL_NPC);
			npcwalkto .@x+rand(-5,5),.@y+rand(-5,5);
		}
	}
    end;

On_521: //Aloe
	Harvest(0);
	end;
On_578: //Strawberry
	Harvest(1);
	end;
On_610: //Ygg
	Harvest(2);
	end;
On_706: //4 leaf clover
	Harvest(3);
	end;
On_621: //Bitter herbs
	Harvest(4);
	end;
On_710: //Illusion flower
	Harvest(5);
	end;
On_514: //Grapes
	Harvest(6);
	end;
On_7298: //Fig
	Harvest(7);
	end;
On_703: //Hinalle
	Harvest(8);
	end;
On_708: //Ment
	Harvest(9);
	end;

OnDismiss:
	mes "[ Maia ]";
	mes "Oh I am afraid I can't help you.";
	close2;
	goto L_Start;
	end;

OnInstanceInit:
OnDisable:
	disablenpc instance_npcname(strnpcinfo(0));
	end;

OnInit:
	//					 Aloe,Straw,yggdr,4leav,bitte,illus,grape,fig  ,hinal,ment
	setarray .Seeds[0] ,30033,30034,30035,30036,30037,30038,30039,30040,30041,30042;
	setarray .Crops[0] ,  521,  578,  610,  706,  621,  710,  514, 7298,  703,  708;
	setarray .CropsY[0],   15,   14,   13,   12,   11,   10,    9,    8,    7,    6;
	end;


	function	BuildPlantingMenu	{
		@menu$ = "";
		.@j = 0;
		cleararray @itens[0],0,getarraysize(@itens);
		freeloop(1);
        for(.@i=30033;.@i<=30042;.@i++){
            if(countitem(.@i)) {
				@menu$ += getitemname(.@i)+":";
				setarray @itens[.@j],.@i;
				.@j++;
			}
		}
		freeloop(0);
		@menu$ += "^FF0000Cancel^000000";
		if(.@j == 0)
			return 0;
		
        return 1;
    }

	function	Planta	{
		.@i 	= getarg(0);
		.@item	= .Crops[.@i];
		.@y		= .CropsY[.@i];
		'gardenerwalk = 1; //'
		.@mobid	= 1078+.@i;
			if(.@i>4) .@mobid -= (.@i-5);
		for(.@j=29;.@j<=35;.@j+=2) {
			monster instance_mapname("rentb1"),.@j,.@y,getitemname(.@item)+" Sprout",.@mobid,1,"Maia::On_"+.Crops[.@i],1,AI_NONE;
		}
		'gardenerwalk = 0; //'
		callfunc "F_AdjustTipsPot",0,800; // 0 = gardener
		return;
	}

	function	Harvest	{
		.@i = getarg(0);
		callfunc "F_AdjustTipsPot",0,100; // 0 = gardener
		if(atoi(#HOUSING_CROPS$[.@i]) < gettimetick(2)) {
			set .@tips, atoi(#HOUSING_TIPS$[0])/5000;
			if (.@tips > 20) set .@tips, 20;
			flooritem2area .Crops[.@i],rand(6,(10+.@tips));
			flooritem2area 511,rand(2,(5+.@tips)); // Green herb;
			flooritem2area 509,rand(2,(5+.@tips)); // White herb;
			flooritem2area 510,rand(2,(5+.@tips)); // Blue herb;
			flooritem2area 508,rand(2,(5+.@tips)); // Blue herb;
		} else {
			npctalk "Hey! Don't do that! They are not ready yet!";
			callfunc "F_AdjustTipsPot",0,300; // 0 = gardener
			debugmes "Amount of " + getitemname(.Crops[.@i]) + " reduced to " + #HOUSING_CROPS_QTY$[.@i];
			debugmes "harvest in "+callfunc("Time2Str",atoi(#HOUSING_CROPS$[.@i]));
		}
		set #HOUSING_CROPS_QTY$[.@i],atoi(#HOUSING_CROPS_QTY$[.@i])-1;
			if(atoi(#HOUSING_CROPS_QTY$[.@i]) <= 0)
				set #HOUSING_CROPS$[.@i], 0;
		
		return;
	}

}