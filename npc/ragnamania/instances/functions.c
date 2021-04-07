// **********************************************************************
function	script	F_CreateInstance	{
// **********************************************************************
// Arguments
// arg0 = quest_id for the PLAYTIME
// arg1 = Instance's name as per instance_db
// arg2 = minimum level to enter
// arg3 = max level enter
// arg4 = minimum number of party members to enter
// arg5 = maximum number of party members to enter
// arg6 = variavel de controle maximo da instancia
// arg7 = mapa de destino (inicio da instancia)
// arg8 = mvp id

	if( !is_party_leader())
		return;

	set .@quest, 		getarg(0,0);	
	set .@instance$, 	getarg(1);
	set .@var$, 		getarg(6);
	set .@party_id,		getcharid(1);
	set .@dmap$, 		getarg(7);
	set .@minL, 		getarg(2);
	set .@maxL, 		getarg(3);
	set .@minP, 		getarg(4);
	set .@maxP, 		getarg(5);
	set .@mvpid,		getarg(8);

	mes "^0000CC[ " + .@instance$ + " ]^000000";
	mes "Mapa : " + .@dmap$;
	mes "Duracao Maxima : 3 Horas";
	mes "Cooldown : 12 Horas";
	mes "Min/Max Nivel : ["+.@minL+"/"+.@maxL+"]";
	mes "Min/Max Membros : ["+.@minP+"/"+.@maxP+"]";
	next;

	.@create = instance_create(.@instance$,IM_PARTY);
	if (.@create < 0) {
		mes "^0000CC[ " + .@instance$ +" ]^000000";
		switch (.@create) {
			case -1: mes "ERROR(instance_create): Tipo Invalido."; break;
			case -2: mes "ERROR(instance_create): Party Nao Encontrada."; break;
			case -3: mes "ERROR(instance_create): Instancia ja existe."; break;
			case -4: mes "ERROR(instance_create): Nao ha instancias livres no momento."; break;
		}
		mes " ";
		mes "Timeless Dungeon creation has ^FF0000failed^000000.";
		emotion ET_HUK;
		close;
	}
	sleep2 500;
	if(getvariableofinstance(getd("'"+.@var$), instance_id(IM_PARTY)) < 1)
		set getvariableofinstance(getd("'"+.@var$), instance_id(IM_PARTY)), 1;
		
	set getvariableofinstance('quest, instance_id(IM_PARTY)), .@quest;
	set getvariableofinstance('instance$, instance_id(IM_PARTY)), .@instance$;
	set getvariableofinstance('dmap$, instance_id(IM_PARTY)), .@dmap$;
	set getvariableofinstance('maxplayers, instance_id(IM_PARTY)), getarg(5);
	set getvariableofinstance('var$, instance_id(IM_PARTY)), .@var$;
	set getvariableofinstance('mvpid, instance_id(IM_PARTY)), .@mvpid;

	callfunc "F_SetMapflags",.@dmap$,.@mapP;
	callfunc "F_InstanceEnter",.@quest,.@instance$,.@var$;

	return;
}


// **************************************
function	script	F_CheckParty	{
// **************************************

	set .@minL, getarg(0);
	set .@maxL, getarg(1);
	set .@minP, getarg(2);
	set .@maxP, getarg(3);
	set .@quest_id, getarg(4);

	if(getcharid(1) <= 0) {
		mes "^0000CC[ Timeless Dungeons ]^000000";
		mes "You need to be in a party";
		mes "in order to enter";
		mes "the Memorioal Dungeon.";
		close;
	}

	getpartymember(getcharid(1),1); // Pega os CharID
	set .@count,$@partymembercount;
	copyarray .@charid[0], $@partymembercid[0], .@count;

	getpartymember(getcharid(1),2); // Pega os Account ID (para o isLoggedin) Lixo.
	copyarray .@characc[0], $@partymemberaid[0], .@count;

	if( (.@count < .@minP || .@count > .@maxP) && getgmlevel() < 10 ) {
		mes "^0000CC[ Timeless Dungeons ]^000000";
		mes "The number of players";
		mes "in the party doesn't meet";
		mes "the requested by this";
		mes "Timeless Dungeon.";
		close;
	}

	if ( instance_check_party(.@party_id,0,.@minL,.@maxL) ) {		
		// Checa se algum membro da party ta salvo antes de o lider criar a instancia
		deletearray .@salvos[0],$@MAX_PARTY_MEMBERS; // MAX_PARTY_MEMBERS
		.@j =0;

		for( .@i=0; .@i < .@count; .@i++ ) {
			if(isLoggedin(.@characc[.@i],.@charid[.@i]) == 0) {
				mes "^0000CC[ Timeless Dungeons ]^000000";
				mes "A member of your party";
				mes "seems to be offline.";
				mes "We cannot continue.";
				close;
			}
			if(checkquest(.@quest_id,PLAYTIME,.@charid[.@i]) >= 0 ) {
				.@salvos[.@j] = .@charid[.@i];
				.@j++;
			}
		}
	
		if(.@j > 0) {
			mes "^0000CC[ Timeless Dungeons ]^000000";
			mes "^DD0000We've found [ " + .@j + "] ^000000member(s) in your party";
			mes "currently on cooldown for this";
			mes "Timeless Dungeon:";
			mes " ";
			
			for( .@i=0; .@i < getarraysize(.@salvos); .@i++ )
				mes " - ^DD0000"+strcharinfo(0,.@salvos[.@i])+"^000000";

			close;
		}

	} 

	return;
}

// **************************************
function	script	F_InstanceEnter	{
// **************************************

	.@quest_id 		= getarg(0,0);
	.@instance$		= getarg(1,"");
	.@var$			= getarg(2,"");

	if(checkquest(.@quest_id,PLAYTIME) == 2)
		erasequest .@quest_id;

	if(instance_id(IM_PARTY)) {
		if(getvariableofinstance('var$,instance_id(IM_PARTY)) == .@var$ && getvariableofinstance(getd("'"+.@var$),instance_id(IM_PARTY)) < 3 ) {
			mes "^0000CC[ Timeless Dungeons ]^000000";
			mes "You are about to enter";
			mes "the Timeless Dungeon";
			mes "^0000FF"+getvariableofinstance('instance$,instance_id(IM_PARTY))+"^000000";
			mes "Do you want to continue?";
			switch(select("Yes, Please:No, Thank you"+(is_party_leader()?":Destroy the Instance":""))) {
				case 2:
					close;
					break;
				case 1:
					.@enter = instance_enter(getvariableofinstance('instance$, instance_id(IM_PARTY)));
					if (.@enter != 0) {
						mes "^0000CC[ " + 'instance$ + " ]^000000";
						switch (.@enter) {
							case 1: mes "ERROR(instance_enter): Party not found."; break;
							case 2: mes "ERROR(instance_enter): Party doesn't have an instance."; break;
							case 3: mes "ERROR(instance_enter): Unknown Error."; break;
						}
						mes " ";
						mes "The instance creation has ^FF0000failed^000000.";
						emotion ET_HUK;
						close;
					} else {
						if(checkquest(getvariableofinstance('quest,instance_id(IM_PARTY)),PLAYTIME) < 0)
							setquest getvariableofinstance('quest,instance_id(IM_PARTY));
					}
					break;
				case 3:
					callfunc "F_DestroyInstance";
					end;
					break;
			}
		} else if(getvariableofinstance('var$,instance_id(IM_PARTY)) != .@var$ ) {
			mes "^0000CC[ " + getarg(1) +" ]^000000";
			mes "It seems you have unfinished business with another instance. You must conclude it first (or destroy it) before being able to proceed.";
			if(is_party_leader()) {
				if(select("Destroy the Instance:Cancel") ==2) {
					close;
				} else {
					callfunc "F_DestroyInstance";
					close;
				}
			} else {
				close;
			}
		} else {
			mes "^0000CC[ " + getvariableofinstance('instance$,instance_id(IM_PARTY)) + " ]^000000";
			mes "This Timeless Dungeon";
			mes "is now closed for entrance.";
			close;
		}
	} else if(checkquest(.@quest_id,PLAYTIME) >= 0 ) {
			callfunc("F_ResetInstance",.@instance$,.@quest_id);
	} else {
		if(!is_party_leader()) {
			mes "The Party Leader must";
			mes "start the instance first.";
			close;
		}

	}

	return;
}


// **************************************
function	script	F_SetMapflags	{
// **************************************
	.@dmap$ = getarg(0);
	.@maxP = getarg(1);
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nomobloot;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nomvploot;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_partylock;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nobaseexp;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nojobexp;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nobranch;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nomemo;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nosave;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_monster_noteleport,1;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_party_max,.@maxP;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_nerf_asura,60;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_player_dmg;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_pc_dmg_weapon,50;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_pc_dmg_magic,50;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_pc_dmg_misc,50;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_pc_dmg_short,50;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_pc_dmg_long,50;
	setmapflag instance_mapname(.@dmap$,instance_id(IM_PARTY)), mf_mob_dmg,500;

	return;
}


// **************************************
function	script	F_PrepareForLooting	{
// **************************************
	if(getvariableofinstance(getd("'" + 'var$),instance_id(IM_PARTY)) == 3) {
		if(!getvariableofinstance(getd("'" + 'var$ + "_" + getcharid(0)),instance_id(IM_PARTY))) {
			set getvariableofinstance(getd("'" + 'var$ + "_" + getcharid(0)),instance_id(IM_PARTY)), 1;
			callfunc "F_OpenLoot",getarg(0,1002);
		} else {
			dispbottom "You have already looted the chest.";
		}
		if(select("Leave the instance:Cancel") == 1) {
			if(getmapusers(instance_mapname(getvariableofinstance('dmap$,instance_id(IM_PARTY)),instance_id(IM_PARTY))) == 1) {
				warp "SavePoint",0,0;
				instance_destroy(instance_id(IM_PARTY));
			} else {
				warp "SavePoint",0,0;
			}
		}
	}
	return;
}

// **************************************
function	script	F_OpenLoot	{
// **************************************
	// 1		=> 'Lower Headgear',
	// 2		=> 'Main Hand',
	// 4		=> 'Garment',
	// 8		=> 'Accessory Right',
	// 16		=> 'Armor',
	// 32		=> 'Off Hand',
	// 64		=> 'Footgear',
	// 128		=> 'Accessory Left',
	// 256		=> 'Upper Headgear',
	// 512		=> 'Middle Headgear',
	// 1024	=> 'Costume Top Headgear',
	// 2048	=> 'Costume Mid Headgear',
	// 4096	=> 'Costume Low Headgear',
	// 8192	=> 'Costume Garment',
	// 32768	=> 'Ammo',
	// 65536	=> 'Shadow Armor',
	// 131072	=> 'Shadow Weapon',
	// 262144	=> 'Shadow Shield',
	// 524288	=> 'Shadow Shoes',
	// 1048576	=> 'Shadow Accessory Right (Earring)',
	// 2097152	=> 'Shadow Accessory Left (Pendant)',

	if (getmobdrops(getarg(0)) == 0 ) 
		return;

	.@count = $@MobDrop_count;
	copyarray .@ids[0],$@MobDrop_item[0],.@count;
	copyarray .@per[0],$@MobDrop_rate[0],.@count;

	set getvariableofinstance('bxp,instance_id(IM_PARTY)), (getmonsterinfo(getarg(0),3) / getvariableofinstance('maxplayers,instance_id(IM_PARTY))) * 50;
	
	set BaseExp, BaseExp + getvariableofinstance('bxp,instance_id(IM_PARTY));
	dispbottom "You have received " + getvariableofinstance('bxp,instance_id(IM_PARTY)) + " points in Base Experience.";
	
	set .@cID, getbattleflag("normal_dg_reserved_char_id");

	for( .@i = 0; .@i < getarraysize(.@ids) - 1; .@i++ ) {
		.@ratio = (.@per[.@i] * BaseLevel) / 255;
		if(.@ratio > 10000) set .@ratio, 10000;
		if(.@ratio == 0) set .@ratio, .@per[.@i];
		if(rand(0,10000) > (10000 - .@ratio)) {
			if (getiteminfo(.@ids[.@i],5) == 256) { // HELMS
				if(rand(10)>8) {
					//F_Rand(2200,2201,2202,2203)
					setarray .@attributes[0],RDMOPT_VAR_HPACCELERATION,RDMOPT_VAR_SPACCELERATION,RDMOPT_VAR_VITAMOUNT,RDMOPT_VAR_MDEFPOWER,RDMOPT_VAR_DEXAMOUNT,RDMOPT_VAR_AVOIDSUCCESSVALUE,RDMOPT_VAR_PLUSAVOIDSUCCESSVALUE;
					setarray .@OptID[0],getelementofarray(.@attributes,rand(getarraysize(.@attributes)));
					//setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]>3) setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]==5) setarray .@OptVal[0],rand(1,5);
					setarray .@OptVal[0],F_Rand(1,1,1,1,1,2,2,2,2,2,3,3,4,5);
					setarray .@OptParam[0],0;
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,.@OptID[0],.@OptVal[0],.@OptParam[0];
				} else {
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,0,0,0;
				}
			} else if (getiteminfo(.@ids[.@i],5) == 4) { // GARMENTS
				if(rand(10)>8) {
					setarray .@attributes[0],RDMOPT_NONE,RDMOPT_NONE,RDMOPT_NONE,RDMOPT_NONE,RDMOPT_NONE,RDMOPT_NONE,RDMOPT_ATTR_TOLERACE_NOTHING,RDMOPT_ATTR_TOLERACE_WATER,RDMOPT_ATTR_TOLERACE_GROUND,RDMOPT_ATTR_TOLERACE_FIRE,RDMOPT_ATTR_TOLERACE_WIND,RDMOPT_ATTR_TOLERACE_POISON,RDMOPT_ATTR_TOLERACE_SAINT,RDMOPT_ATTR_TOLERACE_DARKNESS,RDMOPT_ATTR_TOLERACE_TELEKINESIS,RDMOPT_ATTR_TOLERACE_UNDEAD,RDMOPT_ATTR_TOLERACE_ALL,RDMOPT_NONE,RDMOPT_ATTR_TOLERACE_NOTHING,RDMOPT_ATTR_TOLERACE_WATER,RDMOPT_ATTR_TOLERACE_GROUND,RDMOPT_ATTR_TOLERACE_FIRE,RDMOPT_ATTR_TOLERACE_WIND,RDMOPT_ATTR_TOLERACE_POISON,RDMOPT_ATTR_TOLERACE_SAINT,RDMOPT_ATTR_TOLERACE_DARKNESS,RDMOPT_ATTR_TOLERACE_TELEKINESIS,RDMOPT_ATTR_TOLERACE_UNDEAD;
					setarray .@OptID[0],getelementofarray(.@attributes,rand(getarraysize(.@attributes)));
					//setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]>3) setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]==5) setarray .@OptVal[0],rand(1,5);
					setarray .@OptVal[0],F_Rand(1,1,1,1,1,2,2,2,2,2,3,3,4,5);
					setarray .@OptParam[0],0;
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,.@OptID[0],.@OptVal[0],.@OptParam[0];
				} else {
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,0,0,0;
				}
			} else if (getiteminfo(.@ids[.@i],5) == 32) { // SHIELDS
				if(rand(10)>8) {
					setarray .@attributes[0],RDMOPT_VAR_VITAMOUNT,RDMOPT_VAR_MDEFPOWER,RDMOPT_VAR_INTAMOUNT,RDMOPT_VAR_ITEMDEFPOWER;
					setarray .@OptID[0],getelementofarray(.@attributes,rand(getarraysize(.@attributes)));
					//setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]>3) setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]==5) setarray .@OptVal[0],rand(1,5);
					setarray .@OptVal[0],F_Rand(1,1,1,1,1,2,2,2,2,2,3,3,4,5);
					setarray .@OptParam[0],0;
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,.@OptID[0],.@OptVal[0],.@OptParam[0];
				} else {
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,0,0,0;
				}
			} else if (getiteminfo(.@ids[.@i],5) == 136) { // ACCESSORIES
				if(rand(10)>8) {
					setarray .@attributes[0],RDMOPT_VAR_ITEMDEFPOWER,RDMOPT_VAR_MDEFPOWER,RDMOPT_VAR_STRAMOUNT,RDMOPT_VAR_AGIAMOUNT,RDMOPT_VAR_VITAMOUNT,RDMOPT_VAR_INTAMOUNT,RDMOPT_VAR_DEXAMOUNT,RDMOPT_VAR_LUKAMOUNT,RDMOPT_VAR_BASEATK;
					setarray .@OptID[0],getelementofarray(.@attributes,rand(getarraysize(.@attributes)));
					//setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]>3) setarray .@OptVal[0],rand(1,5); if(.@OptVal[0]==5) setarray .@OptVal[0],rand(1,5);
					setarray .@OptVal[0],F_Rand(1,1,1,1,1,2,2,2,2,2,3,3,4,5);
					setarray .@OptParam[0],0;
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,.@OptID[0],.@OptVal[0],.@OptParam[0];
				} else {
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,0,0,0;
				}
			} else if (getiteminfo(.@ids[.@i],2) == 5 ) { // WEAPONS
				if(rand(10)>8) {
					setarray .@attributes[0],RDMOPT_VAR_AGIAMOUNT,RDMOPT_VAR_DEXAMOUNT,RDMOPT_VAR_STRAMOUNT,RDMOPT_VAR_LUKAMOUNT,RDMOPT_VAR_INTAMOUNT,RDMOPT_VAR_PLUSASPDPERCENT,RDMOPT_VAR_ATKPERCENT,RDMOPT_VAR_AGIAMOUNT,RDMOPT_VAR_DEXAMOUNT,RDMOPT_VAR_STRAMOUNT,RDMOPT_VAR_LUKAMOUNT,RDMOPT_VAR_INTAMOUNT;
					setarray .@OptID[0],getelementofarray(.@attributes,rand(getarraysize(.@attributes)));
					//setarray .@OptVal[0],rand(1,3); if(.@OptVal[0]==3) setarray .@OptVal[0],rand(1,3);
					setarray .@OptVal[0],F_Rand(1,1,1,1,1,1,1,1,1,2,2,2,2,3);
					setarray .@OptParam[0],0;
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,.@OptID[0],.@OptVal[0],.@OptParam[0];
				} else {
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,0,0,0;
				}
			} else if (getiteminfo(.@ids[.@i],5) == 64) { // FOOTGEAR
				if(rand(10)>8) {
					setarray .@attributes[0],RDMOPT_VAR_MAXHPPERCENT,RDMOPT_VAR_MAXSPPERCENT,RDMOPT_RACE_TOLERACE_HUMAN,RDMOPT_VAR_MDEFPOWER;
					setarray .@OptID[0],getelementofarray(.@attributes,rand(getarraysize(.@attributes)));
					//setarray .@OptVal[0],rand(1,3); if(.@OptVal[0]==3) setarray .@OptVal[0],rand(1,3);
					setarray .@OptVal[0],F_Rand(1,1,1,1,1,1,1,1,1,2,2,2,2,3);
					setarray .@OptParam[0],0;
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,.@OptID[0],.@OptVal[0],.@OptParam[0];
				} else {
					callfunc "F_CreateEquip",.@ids[.@i],.@cID,0,0,0;
				}
			} else if (getiteminfo(.@ids[.@i],2) == 4 ) { // OTHER ARMOR PIECES
				callfunc "F_CreateEquip",.@ids[.@i],.@cID,0,0,0;
			} else {
				getitem .@ids[.@i],1;
			}
		}
		sleep2 500;
	}
	if(strcharinfo(0) == 'LootBoxWinner$)
		getitem 30027,1; // Ragnamania LootBox  //'

	// Char var that increases everytime a player finished the instance
	TIMELESS_POINTS++;

	return; 
}

function	script	F_CreateEquip	{
	.@ids	= getarg(0);
	.@cID 		= getarg(1);
	.@optID		= getarg(2,0);
	.@optVal	= getarg(3,0);
	.@optParam	= getarg(4,0);

	if(.@optID > 0)
		getitembound3 .@ids,1,1,0,0,254,0,.@cID&0xffff,(.@cID>>16)&0xffff,Bound_Special,.@optID,.@optVal,.@optParam;
	else
		getitembound2 .@ids,1,1,0,0,254,0,.@cID&0xffff,(.@cID>>16)&0xffff,Bound_Special;

	return;
}

// **************************************
function	script	F_Upper	{
// **************************************
	.@str$ = getarg(0);
	.@f$ = charat(.@str$,0);
	.@f$ = strtoupper(.@f$);
	.@str$ = setchar(.@str$,.@f$,0);

	return .@str$;
}

// **************************************
function	script	F_ResetInstance	{
// **************************************
	.@instance$ = getarg(0);
	.@quest = getarg(1);
	mes "^0000CC[ " + .@instance$ +" ]^000000";
	mes "You are in the cooldown for this instance.";
	if(!$@ENABLE_RESET && getgmlevel()<99) {
		close;
	}
	mes "Do you wanna use a " + getitemname(6320) + " to reset the instance?";
	next;
	if(select("No Thank you:Yes, Please.") == 1 ) {
		close;
	} else {
		if (countitem(6320) < 1) {
			mes "^0000CC[ " + .@instance$ +" ]^000000";
			mes "Sorry, you don't seem to have any " +getitemname(6320)+ " on you.";
			close;
		} else {
			delitem 6320,1;
			erasequest .@quest;
			mes "^0000CC[ " + .@instance$ +" ]^000000";
			mes "All done! Now you can enter the Timeless Dungeon again.";
			close;
		}
	}
}

// **************************************
function	script	F_DestroyInstance	{
// **************************************
	if(instance_id(IM_PARTY) && is_party_leader()) {
		if(getvariableofinstance('dmap$,instance_id(IM_PARTY)) != "" && getmapusers(instance_mapname(getvariableofinstance('dmap$,instance_id(IM_PARTY)),instance_id(IM_PARTY))) == 0 ) {
			instance_destroy(instance_id(IM_PARTY));
			return;
		}
	}
	dispbottom "[Timeless Dungeons Manager] : Instance could not be destroyed."; 
	return;
}

function	script	F_SpawnWave	{
	.@wave 	= getarg(0);
	.@var$	= getd('var$);
}

function	script	F_WaveControl	{
	if('wave > 0) {
		.@count = mobcount(instance_mapname(strnpcinfo(4)),instance_npcname(strnpcinfo(0))+"::OnWaveControl");
		.@mobqt = 'mobqt - .@count;
		if(.@num = getmapunits(BL_PC,instance_mapname(strnpcinfo(4)),.@players$[0]) > 0)
			for(.@i=0;.@i<getarraysize(.@players$);.@i++)
				dispbottom 'instance$ + " : " + getmonsterinfo(KilledRid,MOB_NAME) + "[" + .@mobqt + "/" + 'mobqt + "]",0xF0F0F0,getcharid(0,.@players$[.@i]);
	}
	
	if(.@mobqt == 'mobqt) {
		'wave++;
		switch('wave) {
		case 1:
			'mobid = getelementofarray(getd("$@"+strnpcinfo(2)),0);
			'mobqt = getelementofarray(getd("$@"+strnpcinfo(2)),1);
			break;
		case 2:
			'mobid = getelementofarray(getd("$@"+strnpcinfo(2)),2);
			'mobqt = getelementofarray(getd("$@"+strnpcinfo(2)),3);
			break;
		case 3:
			'mobid = getelementofarray(getd("$@"+strnpcinfo(2)),4);
			'mobqt = getelementofarray(getd("$@"+strnpcinfo(2)),5);
			break;
		case 4:
			'mob$ = getmonsterinfo('mvpid,MOB_NAME);
			instance_announce 0,"Somewhere around lurks an evil shadow",bc_npc,0xF0F0F0,FW_BOLD,24,1,0;
			monster instance_mapname(strnpcinfo(4)),0,0,'mob$,'mvpid,1,instance_npcname(strnpcinfo(0))+"::OnMvpDead";
			set 'mvpGID, $@mobid[0];
			set getd("'"+strnpcinfo(2)), 2;
			getmapunits(BL_PC,instance_mapname(strnpcinfo(4)),'players$[0]);
			set 'LootBoxWinner$, getelementofarray('players$,rand(getarraysize('players$)));
			end;
		}

		'mob$ = getmonsterinfo('mobid,MOB_NAME);
		sleep 3000;
		instance_announce 0,"The Next Wave is Starting Soon",bc_npc,0xF0F0F0,FW_BOLD,24,1,0;
		sleep 5000;
		monster instance_mapname(strnpcinfo(4)),0,0,'mob$,'mobid,'mobqt,instance_npcname(strnpcinfo(0))+"::OnWaveControl";
	}
	return;
}