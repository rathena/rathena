// Hell Gates
// by Biali
// v1.1

-	script	hell_gates	-1,{
	end;

	OnInit:
		$@HG_PARTY_SIZE = 3;
		$@HG_CONCURRENT_HGS = 0; 		// Number of open hell gates before they start being available for a second party
		$@HG_MIN_LEVEL = 60; // Minimum base level to enter
		
		deletearray $@hg_instance_ids[0],getarraysize($@hg_instance_ids); 	// 127 is the maximum number of concurrent hell gates 
		callfunc "hg_setmapflag","hell";
		end;

	OnPCDieEvent:
		if(getmapflag(strcharinfo(3),mf_hellgate)){
			callfunc "F_Fullloot";
			.@hg_id = @hg_id;
			callfunc "hg_clear_vars";
			warp @hg_map$,@hg_x,@hg_y;
			sleep 500;
			callfunc "hg_instance_destroy",.@hg_id;
		}
		end;

	OnPcLogoutEvent:
		if(getmapflag(strcharinfo(3),mf_hellgate)) {
			.@hg_id = @hg_id;
			callfunc "hg_clear_vars";
			sleep 500;
			debugmes "chegou";
			callfunc "hg_instance_destroy",.@hg_id;
		}
		end;

	OnPCStatCalcEvent:
		if(getmapflag(strcharinfo(3),mf_hellgate,1)) {
			if(MaxHp > 100000)
				Hp = MaxHp = 100000;
		}
		end;
}


// Spawn the Guardian
// getd("$@"+strnpcinfo(4)+"Guardian") = 2 : The Guardian is Up
// getd("$@"+strnpcinfo(4)+"Guardian") = 1 : Guardian is Down, Warp is Up
// getd("$@"+strnpcinfo(4)+"Guardian") = 0 : The Guardian is Down and Warp is also down. Waiting respawn of the Guardian
-	script	#hg_guardian	-1,{
	if(getd("$@"+strnpcinfo(4)+"Guardian") == 1 ) {
		if(getd("$@"+strnpcinfo(4)+"Party") == getcharid(1) && @hg_id > 0 && BaseLevel >= @HG_MIN_LEVEL) {
			progressbar "0xff0000",5;
			set getvariableofinstance('hg_party_count,@hg_id), getvariableofinstance('hg_party_count,@hg_id) + 1;
			if(getvariableofinstance('hg_party_count,@hg_id) <= $@HG_PARTY_SIZE) { //'
				getmapxy(@hg_map$,@hg_x,@hg_y);
				if(@hg_created == 0) {
					instance_enter("Hell",72,160,getcharid(0),@hg_id);
				} else {
					instance_enter("Hell",216,160,getcharid(0),@hg_id);
				}
			} else {
				set @hg_id,0;
				dispbottom "Limit reached. Only three members of the same party can descend to hell each time.";
			}
		}
	}
	end;

	OnInit:
		callfunc "hg_setmapflag",strnpcinfo(4);
	OnMinute00:
	OnMinute10:
	OnMinute20:
	OnMinute30:
	OnMinute40:
	OnMinute50:
	OnMinute05:
	OnMinute15:
	OnMinute25:
	OnMinute35:
	OnMinute45:
	OnMinute55:
		getmapxy(.@m$,.@x,.@y,BL_NPC);
		if(getd("$@"+.@m$+"Guardian") < 2) {
			monster .@m$,.@x,.@y,"Guardian",25238,1,strnpcinfo(0)+"::OnGuardianDead",0,AI_NONE,DIR_SOUTH,0;
			set getd("$@"+.@m$+"Guardian"), 2;
		}
		end;
	
	OnGuardianDead:
		if(getattachedrid() == 0) {
			monster .@m$,.@x,.@y,"Guardian",25238,1,strnpcinfo(0)+"::OnGuardianDead",0,AI_NONE,DIR_SOUTH,0;
			end;
		}
		initnpctimer;
		if(getcharid(1) == 0) {
			party_create(md5(getcharid(0)),getcharid(0),0,0);
			sleep2 300;
		}
		set getd("$@"+strnpcinfo(4)+"Party"), getcharid(1);
		set getd("$@"+strnpcinfo(4)+"Guardian"),1;

		setnpcdisplay(strnpcinfo(0),10007);

		// Find an instanced hell or create one
		if(getarraysize($@hg_instance_ids) <= $@HG_CONCURRENT_HGS) {
			set .@hg_id, callfunc("hg_create_instance");
			setarray $@hg_instance_ids[getarraysize($@hg_instance_ids)],.@hg_id;
			callfunc "hg_update_party_members",.@hg_id,1;
			debugmes "Created instance for hell gates : Instance " + .@hg_id;
		} else {
			.@hg_id = $@hg_instance_ids[0];
			debugmes "Attempting to enter hell gates instance " + .@hg_id;
			deletearray $@hg_instance_ids[0],1;
			callfunc "hg_update_party_members",.@hg_id,0;
		}
		end;


	OnTimer60000:
		stopnpctimer;
		setnpcdisplay(strnpcinfo(0),111);
		set getd("$@"+strnpcinfo(4)+"Guardian"),0;
		end;

	
}


// Hell Gates Entrances
moc_fild18,244,212,0	duplicate(#hg_guardian)	#hg_moc_fild18	111
pay_fild07,279,284,0	duplicate(#hg_guardian)	#hg_pay_fild07	111
ra_fild13,225,277,0	duplicate(#hg_guardian)	#hg_ra_fild13	111
prt_fild00,88,248,0	duplicate(#hg_guardian)	#hg_prt_fild00	111
hu_fild03,346,262,0	duplicate(#hg_guardian)	#hg_hu_fild03	111



hell,1,1,1	script	hg_controller	-1,{
	function mob;
	end;
	
OnInstanceInit:
	mob(246,132,25003,"Infernal Imp");
	mob(260,129,25003,"Infernal Imp");
	mob(237,90,25003,"Infernal Imp");
	mob(244,82,25003,"Infernal Imp");
	mob(197,19,25003,"Infernal Imp");
	mob(181,35,25003,"Infernal Imp");
	mob(187,151,25003,"Infernal Imp");
	mob(187,168,25003,"Infernal Imp");
	mob(181,227,25003,"Infernal Imp");
	mob(196,228,25003,"Infernal Imp");
	mob(246,187,25003,"Infernal Imp");
	mob(261,186,25003,"Infernal Imp");
	mob(133,227,25003,"Infernal Imp");
	mob(156,227,25003,"Infernal Imp");
	mob(173,83,25003,"Infernal Imp");
	mob(173,97,25003,"Infernal Imp");
	mob(117,83,25003,"Infernal Imp");
	mob(117,97,25003,"Infernal Imp");
	mob(37,124,25003,"Infernal Imp");
	mob(37,136,25003,"Infernal Imp");
	mob(108,226,25003,"Infernal Imp");
	mob(93,227,25003,"Infernal Imp");
	mob(29,187,25003,"Infernal Imp");
	mob(44,187,25003,"Infernal Imp");
	mob(108,35,25003,"Infernal Imp");
	mob(93,18,25003,"Infernal Imp");
	
	mob(244,65,25002,"Infernal Vulture");
	mob(237,59,25002,"Infernal Vulture");
	mob(232,18,25002,"Infernal Vulture");
	mob(161,47,25002,"Infernal Vulture");
	mob(145,66,25002,"Infernal Vulture");
	mob(221,91,25002,"Infernal Vulture");
	mob(212,82,25002,"Infernal Vulture");
	mob(149,115,25002,"Infernal Vulture");
	mob(140,115,25002,"Infernal Vulture");
	mob(189,123,25002,"Infernal Vulture");
	mob(180,114,25002,"Infernal Vulture");
	mob(168,205,25002,"Infernal Vulture");
	mob(212,249,25002,"Infernal Vulture");
	mob(248,215,25002,"Infernal Vulture");
	mob(141,208,25002,"Infernal Vulture");
	mob(148,208,25002,"Infernal Vulture");
	mob(119,250,25002,"Infernal Vulture");
	mob(170,250,25002,"Infernal Vulture");
	mob(93,100,25002,"Infernal Vulture");
	mob(68,90,25002,"Infernal Vulture");
	mob(77,82,25002,"Infernal Vulture");
	mob(45,66,25002,"Infernal Vulture");
	mob(53,59,25002,"Infernal Vulture");
	mob(40,94,25002,"Infernal Vulture");
	mob(122,205,25002,"Infernal Vulture");
	mob(264,115,25002,"Infernal Vulture");
	mob(76,250,25002,"Infernal Vulture");
	mob(41,214,25002,"Infernal Vulture");
	mob(128,46,25002,"Infernal Vulture");
	mob(54,18,25002,"Infernal Vulture");
	mob(199,99,25002,"Infernal Vulture");

	// Mini Bosses
	mob(264,37,25001,"Fallen Angel","hg_controller::OnAngelDead");
	mob(145,278,25001,"Fallen Angel","hg_controller::OnAngelDead");
	mob(29,35,25001,"Fallen Angel","hg_controller::OnAngelDead");
	end;

	OnAngelDead:
		set getvariableofinstance('hg_angel_dead, @hg_id), getvariableofinstance('hg_angel_dead, @hg_id) + 1;
		if(getvariableofinstance('hg_angel_dead, @hg_id) >= 3) {//'
			instance_announce @hg_id,"The Lord of the Fallen will spawn at the altar in one minute",bc_map,0xFFFFFF,FW_BOLD,32,1;
			.@hg_id = @hg_id;
			sleep 60000;
			monster instance_mapname("hell",.@hg_id),144,159,"Lord of the Fallen",25000,1,"hg_controller::OnDemonDead";
		}
		end;

	OnDemonDead:
		.@hg_id = @hg_id;
		enablenpc instance_npcname("Tartarean Chest",.@hg_id);
		enablenpc instance_npcname("#hg_warp_back",.@hg_id);
		set getvariableofinstance('hg_winner_party, @hg_id), getcharid(1);//'
		donpcevent instance_npcname("Tartarean Chest",.@hg_id) + "::OnEnableNPC";
		// .@n$ = instance_npcname(strnpcinfo(0),.@hg_id);
		// specialeffect 901,AREA,.@n$;
		end;

	OnInstanceDestroy:
		.@index = inarray($@hg_instance_ids,instance_id());
		if(.@index > -1) {
			debugmes "hg_update_queue : cleaning instance " + $@hg_instance_ids[.@index];
			deletearray $@hg_instance_ids[.@index],1;
			debugmes "hg_update_queue : new arraysize : " + getarraysize($@hg_instance_ids);
		}
		end;


	function mob	{
		.@x		= getarg(0);
		.@y 	= getarg(1);
		.@id 	= getarg(2);
		.@n$	= getarg(3);
		.@arg$	= getarg(4,"");

		monster(instance_mapname("hell"),.@x,.@y,.@n$,.@id,1,.@arg$,0,AI_NONE,DIR_SOUTH,0);
		// pcblockmove $@mobid[0],1;
		return;
	}
}


hell,145,159,1	script	Tartarean Chest	25005,{
	if(getcharid(1) != getvariableofinstance('hg_winner_party, @hg_id)) //'
		end;

	progressbar "0xFFFFFF",3;

	if(!getvariableofinstance(getd("'hg_" + getcharid(0)),@hg_id)) {
		getitem 30023,rand(1,6); //Bag of Manias
		getitem 30027,rand(1,3); //Ragnamania Lootbox
		getitem 675,rand(5,20); //Hunting Coins
		getitem 12134,rand(1,12);//Red Envelope
		.@eac = eaclass();
		switch(.@eac&EAJ_BASEMASK) {
			case EAJ_SWORDMAN:
				getitem 6814,1;
				break;
			case EAJ_MAGE:
				getitem 6817,1;
				break;
			case EAJ_ARCHER:
				getitem 6818,1;
				break;
			case EAJ_ACOLYTE:
				getitem 6819,1;
				break;
			case EAJ_MERCHANT:
				getitem 6815,1;
				break;
			case EAJ_THIEF:
				getitem 6816,1;
				break;
			case EAJ_TAEKWON: 
			case EAJ_GUNSLINGER: 
			case EAJ_NINJA:
				.@item = F_Rand(6917,6860,6954,6885,6907,6916,6837,6955,6866,6912,6904,6862);
				getitem .@item,1;
				break;
		}
		set getvariableofinstance(getd("'hg_" + getcharid(0)),@hg_id), 1;
		end;
	} else {
		mes strcharinfo(0);
		mes "I have looted the chest already...";
		close;
	}

	OnInstanceInit:
		disablenpc instance_npcname(strnpcinfo(0));
		end;

	OnEnableNPC:
		specialeffect 901,AREA,.@n$;
		end;

}


hell,147,164,1	script	#hg_warp_back	10007,{
	progressbar "0xFFFFFF",5;
	.@hg_id = @hg_id;
	callfunc "hg_clear_vars";
	warp @hg_map$,@hg_x,@hg_y;
	sleep 500;
	callfunc "hg_instance_destroy",.@hg_id;
	end;

	OnInstanceInit:
		disablenpc instance_npcname(strnpcinfo(0));
		end;
}


// Exit warps
-	script	#hg_exit	-1,{
	end;
	OnTouch:
		if((@hg_created == 0 && strnpcinfo(0) == "#hg_exit1") || (@hg_created == 1 && strnpcinfo(0) == "#hg_exit2")){
			progressbar "0xFFFFFF",30;
			.@hg_id = @hg_id;
			callfunc "hg_clear_vars";
			warp @hg_map$,@hg_x,@hg_y;
			sleep 500;
			callfunc "hg_instance_destroy",.@hg_id;
		}
		end;
}
hell,77,160,0	duplicate(#hg_exit)	#hg_exit1	45,1,1
hell,211,160,0	duplicate(#hg_exit)	#hg_exit2	45,1,1




function	script	hg_create_instance	{
	.@hg_id = instance_create("Hell",IM_NONE);
	if (.@hg_id < 0) {
		switch (.@hg_id) {
			case -1: dispbottom "ERROR(instance_create): Tipo Invalido.",0x0000CC; break;
			case -2: dispbottom "ERROR(instance_create): Party Nao Encontrada.",0x0000CC; break;
			case -3: dispbottom "ERROR(instance_create): Instancia ja existe.",0x0000CC; break;
			case -4: dispbottom "ERROR(instance_create): Nao ha instancias livres no momento.",0x0000CC; break;
		}
		dispbottom "Dungeon creation failed",0x0000CC;
		end;
	}
	// setarray $@hg_instance_ids[getarraysize($@hg_instance_ids)],.@hg_id;
	
	return .@hg_id;
}


function	script	hg_update_party_members	{
	set .@hg_id, getarg(0);
	set .@hg_create, getarg(1,0);
	getpartymember getcharid(1), 1;
	getpartymember getcharid(1), 2;
	.@count = $@partymembercount;
	copyarray .@partymemberaid, $@partymemberaid, .@count;
	copyarray .@partymembercid, $@partymembercid, .@count;

	for ( .@i = 0; .@i < .@count; .@i++ ) {
		if ( isloggedin( .@partymemberaid[.@i], .@partymembercid[.@i] ) ) {
			set @hg_id,.@hg_id,.@partymembercid[.@i];
			set @hg_created,.@hg_create,.@partymembercid[.@i];
		}
	}
	return;
}


function	script	hg_instance_destroy	{
	.@hg_id = getarg(0,0);
	if(getmapusers(instance_mapname("hell",.@hg_id)) == 0)
		instance_destroy(.@hg_id);

	return;
}

function	script	hg_setmapflag	{
	.@m$ = getarg(0,"");
	if(.@m$ == "") return;

	// Common mapflags (applies to both, arena and hell gates entrance maps)
	setarray .hg_mapflags[0],mf_noteleport,mf_nowarp,mf_nobranch,mf_nosave,mf_nochat,mf_novending,mf_noicewall,mf_nosunmoonstarmiracle,mf_loadevent,mf_pk,mf_fullloot;
	for(.@i=0;.@i<getarraysize(.hg_mapflags);.@i++) {
		setmapflag .@m$,.hg_mapflags[.@i];
	}

	if(.@m$ == "hell") {
		// Hell mapflags applies only to the arena
		setmapflag .@m$, mf_hellgate;
		setmapflag .@m$, mf_noloot;
		setmapflag .@m$, mf_partylock;
		setmapflag .@m$, mf_guildlock;
		setmapflag .@m$, mf_nobaseexp;
		setmapflag .@m$, mf_nojobexp;
		setmapflag .@m$, mf_nomemo;
		setmapflag .@m$, mf_monster_noteleport,1;
		setmapflag .@m$, $@HG_PARTY_SIZE;
		setmapflag .@m$, mf_nerf_asura,80;
		setmapflag .@m$, mf_notrade;
		setmapflag .@m$, mf_nodrop;
		setmapflag .@m$, mf_nopenalty;
		// setmapflag .@m$, mf_fog;



		
		// setmapflag .@m$, mf_player_dmg;
		// setmapflag .@m$, mf_pc_dmg_weapon,50;
		// setmapflag .@m$, mf_pc_dmg_magic,50;
		// setmapflag .@m$, mf_pc_dmg_misc,50;
		// setmapflag .@m$, mf_pc_dmg_short,50;
		// setmapflag .@m$, mf_pc_dmg_long,50;
		// setmapflag .@m$, mf_mob_dmg,500;
		// setmapflag .@m$, mf_nomobloot;
		// setmapflag .@m$, mf_nomvploot;
	}
	return;
}

function	script	hg_clear_vars	{
	if(getattachedrid() == 0)
		return;

	recovery 0;
	set @hg_id,0;
	set @hg_created, 0;
	recalculatestat;

	return;
}