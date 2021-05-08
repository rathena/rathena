ars_dun01,122,29,0	script	#ars_dun01	45,1,1,{
	function setmapflags; function spawnmobs; function respawnTime;
	end;
	OnInit:
		setmapflags();
		spawnmobs();
		.bosses = mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnBossDead");
		.mobs = mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnMobDead");
		debugmes " bosses no mapa : " + .bosses;
		debugmes " mobs no mapa : " + .mobs;
		end;

	OnMobDead:
		if(!.occupied)
			set .occupied, 1;
		if(.mobs - mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnMobDead") <= 0) {
			set .allMobsDead,1;
			if(!.allBossesDead) disablenpc "#fld01_dun01";
		}
		end;

    OnBossDead:
		if(!.occupied)
			set .occupied, 1;
		if(.bosses - mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnBossDead") <= 0) {
			set .allBossesDead,1;
			if(!.allMobsDead) disablenpc "#fld01_dun01";
		}
        end;

	// in case there is no one in the map but any of the mobs or the bosses were killed, close the entrance and wait for respawn time
	OnMinute00:
		if(.try2respawn) end;
		if(getmapusers(strnpcinfo(4)) == 0 && (mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnMobDead") < .mobs || mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnBossDead") < .bosses))
			if(!.allMobsDead && !.allBossesDead) // if any of these are true, it means the entrance is already closed
				disablenpc "#fld01_dun01";
		end;
	
	// in case there is no one in the map, respawn the dungeon
	// otherwise start the timer to check every minute for the empty and respawn it
	OnMinute15:
		if(.try2respawn) end;
		if(getmapusers(strnpcinfo(4)) == 0){
			respawnTime();
		} else {
			initnpctimer;
			set .try2respawn,1;
		}
		end;

	// check every minute if the dungeon is empty of players, in case it is, respawn it
	// if not, restart the timer to check again in one minute
	OnTimer60000:
		stopnpctimer;
		if(getmapusers(strnpcinfo(4)) == 0){
			respawnTime();
		} else
			initnpctimer;
		end;


	// Dungeon Exit
	OnTouch:
		warp "ars_fild01",82,140;
		end;


	function	respawnTime	{
		if(mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnMobDead") < .mobs || mobcount(strnpcinfo(4),strnpcinfo(0)+"::OnBossDead") < .bosses) {
			set .allMobsDead,0;
			set .allBossesDead,0;
			set .occupied,0;
			set .try2respawn,0;
			sleep rand(30000,180000); // Randomize the respawn a little bit
			clearunits(strnpcinfo(4));
			spawnmobs();
			enablenpc "#fld01_dun01";
		}
		return;
	}

	function	setmapflags	{
		setmapflag strnpcinfo(4),mf_noteleport;
		setmapflag strnpcinfo(4),mf_nomemo;
		setmapflag strnpcinfo(4),mf_nobranch;
		setmapflag strnpcinfo(4),mf_rpk,RPK_MAP_TIER,5;
		setmapflag strnpcinfo(4),mf_rpk,RPK_ISDG,true;
		setmapflag strnpcinfo(4),mf_loadevent;
		setmapflag strnpcinfo(4),mf_monster_noteleport;
		setmapflag strnpcinfo(4),mf_bexp,120;
		setmapflag strnpcinfo(4),mf_jexp,120;
		return;
	}

	function	spawnmobs	{
		monster "ars_dun01",120,57,"Necromancer",1870,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",128,57,"Necromancer",1870,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",80,85,"Necromancer",1870,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",87,86,"Necromancer",1870,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",96,76,"Necromancer",1870,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",189,127,"Necromancer",1870,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_NORTHWEST,0;
		monster "ars_dun01",183,144,"Necromancer",1870,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;

		monster "ars_dun01",124,57,"Possessed Body",1920,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;

		monster "ars_dun01",72,60,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",78,56,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",84,49,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",41,81,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",33,83,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",160,87,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHWEST,0;
		monster "ars_dun01",158,47,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_WEST,0;
		monster "ars_dun01",118,104,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_EAST,0;
		monster "ars_dun01",121,103,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_EAST,0;
		monster "ars_dun01",120,99,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_EAST,0;
		monster "ars_dun01",106,127,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_EAST,0;
		monster "ars_dun01",106,123,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_EAST,0;
		monster "ars_dun01",144,122,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",149,124,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",148,129,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",123,145,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",117,141,"Gibbet",1503,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTHEAST,0;

		monster "ars_dun01",21,103,"Banshee",1867,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",21,119,"Banshee",1867,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",29,101,"Banshee",1867,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",29,112,"Banshee",1867,1,strnpcinfo(0)+"::OnMobDead",2,AI_NONE,DIR_SOUTH,0;

		monster "ars_dun01",25,124,"Fallen Bishop Hibram",1871,1,strnpcinfo(0)+"::OnBossDead",2,AI_NONE,DIR_SOUTH,0;
		monster "ars_dun01",29,30,"Dark Lord",1272,1,strnpcinfo(0)+"::OnBossDead",2,AI_NONE,DIR_NORTH,0;
		monster "ars_dun01",142,94,"Sword Guardian",1829,1,strnpcinfo(0)+"::OnBossDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",154,165,"Sword Guardian",1829,1,strnpcinfo(0)+"::OnBossDead",2,AI_NONE,DIR_SOUTHEAST,0;
		monster "ars_dun01",111,114,"Bacsojin",1630,1,strnpcinfo(0)+"::OnBossDead",2,AI_NONE,DIR_EAST,0;
		monster "ars_dun01",185,135,"Dracula",1389,1,strnpcinfo(0)+"::OnBossDead",2,AI_NONE,DIR_SOUTHWEST,0;
		monster "ars_dun01",154,227,"Snake Lord",1418,1,strnpcinfo(0)+"::OnBossDead",2,AI_NONE,DIR_SOUTH,0;
		return;
	}
}


// Dungeon Entrance
ars_fild01,82,146,0	script	#fld01_dun01	45,1,1,{
	end;

OnTouch:
	warp "ars_dun01",126,41;
	end;
}


// Random mobs and patrols
ars_dun01,0,0,0,0	monster	Ghostring	1120,2,2700000
ars_dun01,0,0,0,0	monster	Drainliar	1111,8,1800000
ars_dun01,79,54,100,69	monster	Lude	1509,3,1800000
ars_dun01,139,64,159,88	monster	Lude	1509,3,1800000

