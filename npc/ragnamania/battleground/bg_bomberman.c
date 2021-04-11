//===== rAthena Script =======================================
//= Bomberman BG
//===== By: ==================================================
//= Grenat
//===== Description: ===========================================
//= Adapted version of Bomberman for Ragnarok Online.
//===== Additional Comments: =================================
//= 2.0 New version and updated for the latest rAthena [Grenat]
//=     Originally made from the free release of Key
//=    *Supports now multiple amount of maps
//=    *Speed UP added
//============================================================

//==============================================================
//= Mapflags
//==============================================================
bomberman	mapflag	battleground	2
bomberman	mapflag	nomemo
bomberman	mapflag	noteleport
bomberman	mapflag	nobranch
bomberman	mapflag	nopenalty
bomberman	mapflag	nozenypenalty
bomberman	mapflag	noexppenalty
bomberman	mapflag	nomobloot
bomberman	mapflag	nodrop
bomberman	mapflag	noskill
bomberman	mapflag	nowarp
bomberman	mapflag	nowarpto
bomberman	mapflag	novending
bomberman	mapflag	notrade
bomberman	mapflag	hidemobhpbar
bomberman	mapflag	nosave	SavePoint
bg_bomber	mapflag	battleground	2
bg_bomber	mapflag	nomemo
bg_bomber	mapflag	noteleport
bg_bomber	mapflag	nobranch
bg_bomber	mapflag	nopenalty
bg_bomber	mapflag	nozenypenalty
bg_bomber	mapflag	noexppenalty
bg_bomber	mapflag	nomobloot
bg_bomber	mapflag	nodrop
bg_bomber	mapflag	noskill
bg_bomber	mapflag	nowarp
bg_bomber	mapflag	nowarpto
bg_bomber	mapflag	novending
bg_bomber	mapflag	notrade
bg_bomber	mapflag	hidemobhpbar
bg_bomber	mapflag	nosave	SavePoint
bg_bomber2	mapflag	battleground	2
bg_bomber2	mapflag	nomemo
bg_bomber2	mapflag	noteleport
bg_bomber2	mapflag	nobranch
bg_bomber2	mapflag	nopenalty
bg_bomber2	mapflag	nozenypenalty
bg_bomber2	mapflag	noexppenalty
bg_bomber2	mapflag	nomobloot
bg_bomber2	mapflag	nodrop
bg_bomber2	mapflag	noskill
bg_bomber2	mapflag	nowarp
bg_bomber2	mapflag	nowarpto
bg_bomber2	mapflag	novending
bg_bomber2	mapflag	notrade
bg_bomber2	mapflag	hidemobhpbar
bg_bomber2	mapflag	nosave	SavePoint

//==============================================================
//= Bomb item scripts.
//==============================================================

function	script	bomb_summon	{
	getmapxy .@map$, .@x, .@y;
	if(.@map$ == "bomberman") .@map = 1;
	else if(.@map$ == "bg_bomber") .@map = 2;
	else if(.@map$ == "bg_bomber2") .@map = 3;
	else end;
	// Check if event is on
	if ( ! getd("$@BBM_RUN"+.@map+"") )
		end ;
	// Check if registered
	if ( getvariableofnpc(getd(".PlayerList"+.@map+"[0]"),""+.@map+"Bomb#core") != getcharid(3) &&
		getvariableofnpc(getd(".PlayerList"+.@map+"[1]"),""+.@map+"Bomb#core") != getcharid(3) &&
		getvariableofnpc(getd(".PlayerList"+.@map+"[2]"),""+.@map+"Bomb#core") != getcharid(3) &&
		getvariableofnpc(getd(".PlayerList"+.@map+"[3]"),""+.@map+"Bomb#core") != getcharid(3) )
		end;
	// Maximum Bomb to put at the same time
	if ( @BBM_spawn > ( (countitem($@ID_DELAY_DOWN) > 7) ? 7 : countitem($@ID_DELAY_DOWN) ) )
		end ;

	@BBM_spawn ++;
	recalculatestat;	// Recalculate stats to count number of Speed UP

	// Collect a valid Bomb and trigger it
	for ( set .@i, 1; .@i < 42; set .@i, .@i+1 )
		if ( ! (getd("$@BBM_RESERVED"+.@map+"") & (1 << .@i)) )
		{
			setd("$@BBM_RESERVED"+.@map+""),  getd("$@BBM_RESERVED"+.@map+"") | 1 << .@i ;
			setd("$@BBM_NAME"+.@map+"["+.@i+"]"), getcharid(3);
			moveNPC ""+.@map+"Bomb#"+.@i, .@x, .@y;
			setwall getd("$@BombRMAP"+.@map+"$"), .@x, .@y, 1, 4, 0, "BBM"+.@map+"_"+.@x+"-"+.@y;
			initnpctimer ""+.@map+"Bomb#"+.@i;
			return 1;
		}
	return 0;
}

//==============================================================
//= Main Script.
//==============================================================
bomberman,0,0,4	script	1Bomb#core	-1,{

	function do_init;          // Initialisation des variables + map
	function resetplayervar;   // Clean/Initialise les variables des joueurs
	function build_explosion;  // Prépare une explosion
	function do_explosion;     // Fait l'explosion
	function ExploseBombin;    // Explosion en Chaîne
	function isPlayerin;       // Retourne 1 si une personne est trouvée sur <x>,<y>
	function checkplayerinarea;// Fait une action sur les joueurs dans une zone
	function setmapflaglist;   // Function raccourcie de setmapflag
	function build_arena;      // Construit la map
	function destroy_arena;    // Main function pour détruire l'arene
	function destroy_xy;       // Sub function pour détruire une rangée
	function rewardend;        // Rewards and end
	function onemoregame;	   // Launch another round

	// Spawn bomb
	OnPush:
		.@map = atoi(strnpcinfo(1));
		// Check item "bomb"
		if ( ! countitem($@ID_BOMB_KICK) )
			end;

		// Bug if we don't stop it
		npcstop;

		getmapxy .@map$, .@x, .@y ;
		getmapxy .@map$, .@x_, .@y_, BL_NPC ;

		// Long range hack
		if ( distance(.@x, .@y, .@x_, .@y_) > 2 ) {
			message strcharinfo(0), "You must be a square away to be able to push the Bomb !";
			end;
		}

		// Block diagonal movement
		if ( (.@x!=.@x_)^(.@y!=.@y_) ) {
			set .@dist_x, ( .@x_ - .@x ) ;
			set .@dist_y, ( .@y_ - .@y ) ;

			// Check max range where the bomb can go without meeting obstacles
			do {
				set .@case,.@case+1;
				set .@futur_x, .@dist_x * .@case + .@x_;
				set .@futur_y, .@dist_y * .@case + .@y_;
			} while( checkcell(getd("$@BombRMAP"+.@map+"$"), .@futur_x ,.@futur_y ,cell_chkpass) ); 

			// In case the bomb meets a wall
			if ( .@case-1 ) {
				// Removed blocking wall, Bomb movement, visual effect.
				delwall "BBM"+.@map+"_"+.@x_+"-"+.@y_;
				set .@futur_x, .@dist_x * (.@case-1) + .@x_;
				set .@futur_y, .@dist_y * (.@case-1) + .@y_;
				soundeffectall "bomberman_kick.wav",0,strnpcinfo(4);
				npcwalkto .@futur_x, .@futur_y ;
				specialeffect(53);

				// Stop the Bomb if someone is in the way (player or Bomb or wall)
				do {
					setd ".BBM"+.@map+"_XY_"+.@x_+"-"+.@y_ , 0;
					getmapxy .@map$, .@x_, .@y_, BL_NPC;
					setd ".BBM"+.@map+"_XY_"+.@x_+"-"+.@y_ , atoi(strnpcinfo(2));

					set .@look_x, .@dist_x*2  + .@x_;
					set .@look_y, .@dist_y*2  + .@y_;

					if ( isPlayerin(.@look_x,.@look_y) || getd(".BBM"+.@map+"_XY_"+.@look_x+"-"+.@look_y) )
					{
						setd ".BBM"+.@map+"_XY_"+.@x_+"-"+.@y_ , 0;
						npcstop;

						// movenpc to correct a visual bug square
						set .@x, .@dist_x+.@x_;
						set .@y, .@dist_y+.@y_;
						movenpc strnpcinfo(0), .@x , .@y ;

						setwall getd("$@BombRMAP"+.@map+"$"), .@x, .@y, 1, 4, 0, "BBM"+.@map+"_"+.@x+"-"+.@y;
						setd ".BBM"+.@map+"_XY_"+.@x+"-"+.@y , atoi(strnpcinfo(2));
						break;

					}
					sleep(30);
				} while( .@futur_x!=.@x_ || .@futur_y!=.@y_ );
			}
		}
		end;

	// Préparation du jeu.
	OnStart:
		.@map = atoi(strnpcinfo(1));
		if(.@map == 1) setarray .@pos[0],25,42,35,32,25,32,35,42;
		else if(.@map == 2) setarray .@pos[0],141,164,155,164,155,150,141,150;
		else if(.@map == 3) setarray .@pos[0],94,85,106,85,106,73,94,73;
		deletearray getd(".PlayerList"+.@map+"[0]"), getarraysize(getd(".PlayerList"+.@map+""));

		do_init();

		deletearray $@arenamembers[0],getarraysize($@arenamembers);
		bg_get_data(getd("$@bomberman"+.@map+"1"),1);
		setd(".PlayerList"+.@map+"[0]"), $@arenamembers[0];
		setd(".PlayerList"+.@map+"[1]"), $@arenamembers[1];
		setpcblock PCBLOCK_ATTACK,1,$@arenamembers[0];
		if($@arenamembers[1]) setpcblock PCBLOCK_ATTACK,1,$@arenamembers[1];
		deletearray $@arenamembers[0],getarraysize($@arenamembers);
		bg_get_data(getd("$@bomberman"+.@map+"2"),1);
		setd(".PlayerList"+.@map+"[2]"), $@arenamembers[0];
		setd(".PlayerList"+.@map+"[3]"), $@arenamembers[1];
		setpcblock PCBLOCK_ATTACK,1,$@arenamembers[0];
		if($@arenamembers[1]) setpcblock PCBLOCK_ATTACK,1,$@arenamembers[1];
		for( .@i = 0; .@i < getarraysize(getd(".PlayerList"+.@map+"")); .@i++ )
			if(getd(".PlayerList"+.@map+"["+.@i+"]"))
				warpchar getd("$@BombRMAP"+.@map+"$"),.@pos[.@i*2],.@pos[.@i*2+1], getcharid(0,rid2name(getd(".PlayerList"+.@map+"["+.@i+"]")));
		setd("$@BBM_RUN"+.@map+""), 1;
		setd("$@BBM_DESTROY"+.@map+""), 0;
		setd("$@score_bomber"+.@map+"_red"),0;
		setd("$@score_bomber"+.@map+"_blue"),0;

		resetplayervar(1);  // Initialisation des variables + items
		build_arena();      // Construction de la map
		initnpctimer;       // Début du timer
		sleep 800;
		soundeffectall "bomberman_intro.wav",0,strnpcinfo(4);
		sleep 1500;
		bg_reserve strnpcinfo(4), true;
		end;

	// PCstatcalc
	OnPCStatCalcEvent:
	if(compare(strcharinfo(3),"bomber"))
		bonus bSpeedAddRate,10*countitem("Speed_UP");
	end;

	// Debug.
	OnDebug:
		do_init();
		end;
	// Explosion en chaîne !
	OnExplose:
		specialeffect( .EXPLOSION_EFFECT ) ;
		soundeffectall "bomberman_explode.wav",0,strnpcinfo(4);
		end;
	// Invulnérabilité qui prend fin.
	OnInvincibleEnd:
		set @BBM_INVINCIBLE, 0;
		end;
	// Initialisation du jeu.
	OnInit:
		if(compare(strnpcinfo(0),"bomb")) {
			setnpcdisplay( strnpcinfo(0), strnpcinfo(0), 1904, 1 );
			npcspeed 110;
		}
		do_init();
		end;

	// Dire que la Bomb se trouve à <x>,<y>.
	// (erreur fixed)
	OnTimer1:
		soundeffectall "bomberman_bomb.wav",0,strnpcinfo(4);
		if( strnpcinfo(2) != "core" ) {
			getmapxy .@map$, .@x, .@y, BL_NPC;
			setd ".BBM"+atoi(strnpcinfo(1))+"_XY_"+.@x+"-"+.@y , atoi(strnpcinfo(2));
		}
		end;

	// Vibration sur la Bomb.
	OnTimer1000: OnTimer2000:
		if( strnpcinfo(2) != "core" )
			specialeffect( 458 ) ;
		end;

	// Explosion de la Bomb.
	OnTimer3000: OnComboExplose:
		.@map = atoi(strnpcinfo(1));
		if( strnpcinfo(2) != "core" ) {
			if ( attachrid(getd("$@BBM_NAME"+.@map+"["+atoi(strnpcinfo(2))+"]")) ) {
				set @BBM_spawn, @BBM_spawn > 0 ? @BBM_spawn-1 : 0 ;
				detachrid;
			}
			stopnpctimer;
			getmapxy .@map$, .@x, .@y, BL_NPC;
			setd ".BBM"+.@map+"_XY_"+.@x+"-"+.@y , 0;
			specialeffect( .EXPLOSION_EFFECT ) ;
			soundeffectall "bomberman_explode.wav",0,strnpcinfo(4);
			build_explosion(.@x,.@y);
			if ( getd("$@BBM_RESERVED"+.@map+"") & (1 << atoi(strnpcinfo(2))) )
				setd("$@BBM_RESERVED"+.@map+""),  getd("$@BBM_RESERVED"+.@map+"") - (1 << atoi(strnpcinfo(2))) ;
			setd("$@BBM_NAME"+.@map+"["+atoi(strnpcinfo(2))+"]"), 0;
		}
		end;

	// Fin du main timer, destruction de l'arêne.
	OnTimer300000:
		destroy_arena();
		resetplayervar();
		do_init();
		end ;

	// When someone leaves
	OnBombermanQuit:
		//sc_end SC_TIMERDOWN;
		cutin "",255;
		setpcblock PCBLOCK_ATTACK,0;
		bg_desert;
		end;

	function do_init {
		// Configurations
		set .BBM_BARRIERE_ID1   , 20403;	// Mob ID barriers Map 1
		set .BBM_BARRIERE_ID2   , 20404;	// Mob ID barriers Map 2
		set .BBM_BARRIERE_ID3   , 20405;	// Mob ID barriers Map 3
		set .BBM_BARRIERE_SIZE1   , 0;		// Mob Size barriers Map 1
		set .BBM_BARRIERE_SIZE2   , 1;		// Mob Size barriers Map 2
		set .BBM_BARRIERE_SIZE3   , 0;		// Mob Size barriers Map 3
		set .EXPLOSION_EFFECT   , 634;		// Visual effect ID of bomb explosion
		set .ZONE_EXPLOSION     , 1;		// explosion range (1 case autour donc 3*3)
		set .game_over          , 3;		// Amount of time dead before le player to leave the scene
		set .chance_drop	, 65;		// pourcentage chance to obtain an item
		set $@ID_BOMB_BOX       , 8968;		// Item ID Bomb
		set $@ID_BOMB_KICK      , 8969;		// Item ID Bomb Kick
		set $@ID_INCR_EXPL      , 8970;		// Item ID Fire UP
		set $@ID_DELAY_DOWN     , 8971;		// Item ID Bomb UP
		set $@ID_SPEED_UP	, 8972;		// Item ID Speed UP
		set $@BombRMAP1$	, "bomberman";	// Map 1 : Zombie/Halloween map
		set $@BombRMAP2$	, "bg_bomber";	// Map 2 : Xmas theme
		set $@BombRMAP3$	, "bg_bomber2";	// Map 3 : Prontera/Urbain style
		setarray .arena_settings1[0], 24, 31 ;		// <x>,<y> from bottom west of arena
		setarray .arena_settings2[0], 140, 149 ;	// <x>,<y> from bottom west of arena
		setarray .arena_settings3[0], 93, 72 ;		// <x>,<y> from bottom west of arena
		setarray .dead_respawn1[0], 33, 29 ;		// <x>,<y> of respawn for those who are game over
		setarray .dead_respawn2[0], 148, 148 ;		// <x>,<y> of respawn for those who are game over
		setarray .dead_respawn3[0], 90, 79 ;		// <x>,<y> of respawn for those who are game over
		setarray .lenght1, 11 ; 			// lenght of the 1st map
		setarray .lenght2, 15 ; 			// lenght of the 2nd map
		setarray .lenght3, 13 ; 			// lenght of the 3rd map
		killmonster strnpcinfo(4), "All";
		cleanmap strnpcinfo(4);
		stopnpctimer;
	}

	function setmapflaglist {
		for( set .@i,1; getarg(.@i,-1)+1; set .@i, .@i+1 ) 
			setmapflag getarg(0),getarg(.@i);
	}

	function build_explosion {
		.@map = atoi(strnpcinfo(1));
		npcstop;

		// Explosion zone
		if ( attachrid(getd("$@BBM_NAME"+.@map+"["+atoi(strnpcinfo(2))+"]")) )
		{
			set .@ZONE_EXPLOSION, countitem($@ID_INCR_EXPL)+1 > 6 ? 6 : countitem($@ID_INCR_EXPL)+1 ;
			detachrid;
		}
		else
			set .@ZONE_EXPLOSION, 1 ;

		// Moving the npc before explosion (infinity loop if misplaced).
		moveNPC strnpcinfo(0), 0, 0;

		// Explosion on ourselves
		do_explosion(getarg(0),getarg(1));

		setarray .@matrix[0], 0,  1, 1,  0, 0, -1, -1, 0; 

		// Zone explosion
		for ( set .@face,0; .@face<4; set .@face, .@face+1)
			for ( set .@zone,1; .@zone<=.@ZONE_EXPLOSION; set .@zone,.@zone+1 )
			{
				set .@x, getarg(0) + .@matrix[.@face*2] * .@zone ;
				set .@y, getarg(1) + .@matrix[.@face*2+1] * .@zone ;
				if( checkcell(getd("$@BombRMAP"+.@map+"$"),.@x,.@y,cell_chkpass) )
				{
					do_explosion(.@x,.@y);
					continue;
//				} else if ( mobcount(getd("$@BombRMAP"+.@map+"$"),""+.@map+"Bomb#core::OnBBM"+.@map+"_"+.@x+"-"+.@y) || getd(".BBM"+.@map+"_XY_"+.@x+"-"+.@y) )
				} else if ( getareaunits(BL_MOB,getd("$@BombRMAP"+.@map+"$"),.@x,.@y,.@x,.@y) || getd(".BBM"+.@map+"_XY_"+.@x+"-"+.@y) )
					do_explosion(.@x,.@y);
				break;
			}
	}

	function do_explosion {

		.@map = atoi(strnpcinfo(1));
		set .@x, getarg(0) ;
		set .@y, getarg(1) ;

		// Création de l'explosion.
		moveNPC "Explosion"+.@map+"#BBM", .@x, .@y;
		donpcevent "Explosion"+.@map+"#BBM::OnExplose";
		if(checkwall("BBM"+.@map+"_"+.@x+"-"+.@y)) delwall "BBM"+.@map+"_"+.@x+"-"+.@y;
		if(.@x) cleanarea getd("$@BombRMAP"+.@map+"$"),.@x,.@y,.@x,.@y;
		// Item Drops
//		if ( mobcount(getd("$@BombRMAP"+.@map+"$"),""+.@map+"Bomb#core::OnBBM"+.@map+"_"+.@x+"-"+.@y) )
		if( getareaunits(BL_MOB,getd("$@BombRMAP"+.@map+"$"),.@x,.@y,.@x,.@y,.@array) )
		{
			unitkill .@array[0];
//			killmonster getd("$@BombRMAP"+.@map+"$"), ""+.@map+"Bomb#core::OnBBM"+.@map+"_"+.@x+"-"+.@y;

			// 1/2 chance de spam item
			if ( rand(0,100) <= .chance_drop ) {
				//18%
				if( set(.@r, rand(1,100)) < 18 )
					makeitem $@ID_BOMB_KICK, 1, getd("$@BombRMAP"+.@map+"$"), .@x, .@y ;
				//29%
				else if ( .@r < 47 )
					makeitem $@ID_INCR_EXPL, 1, getd("$@BombRMAP"+.@map+"$"), .@x, .@y ;
				//24%
				else if ( .@r < 71 )
					makeitem $@ID_SPEED_UP, 1, getd("$@BombRMAP"+.@map+"$"), .@x, .@y ;
				//29%
				else
					makeitem $@ID_DELAY_DOWN, 1, getd("$@BombRMAP"+.@map+"$"), .@x, .@y ;
			}
		}
		else
			checkplayerinarea( .@x, .@y, 1 );

		// Explosion en chaine.
		ExploseBombin(.@x,.@y, atoi(strnpcinfo(2)));

	}

	function checkplayerinarea {

		.@map = atoi(strnpcinfo(1));
		set .@x, getarg(0);
		set .@y, getarg(1);
		set .@malus, getarg(2);

		// For all players...
		for( set .@i,0; .@i<getarraysize(getd(".PlayerList"+.@map+"")); set .@i, .@i+1 ) {

			// ... that are online and on the map...
			if( attachrid(getd(".PlayerList"+.@map+"["+.@i+"]")) ) {
				getmapxy .@map$, .@x_, .@y_, BL_PC, rid2name(getd(".PlayerList"+.@map+"["+.@i+"]"));

				// ... that are in the explosion and not invincible...
				if( .@map$ == getd("$@BombRMAP"+.@map+"$") && !distance(.@x,.@y,.@x_,.@y_) ) {
				
					if ( @BBM_INVINCIBLE &&  ! getarg(3,0) )
						continue;

					// ... dégâts + effets
					set @BBM_HIT, @BBM_HIT+.@malus;
					specialeffect2( 372 );
					soundeffectall "bomberman_audience.wav",0,strnpcinfo(4);

					// Gain pts for the other team
					if(bg_teamid()) setd("$@score_bomber"+.@map+"_blue"),getd("$@score_bomber"+.@map+"_blue")+1;
					else setd("$@score_bomber"+.@map+"_red"),getd("$@score_bomber"+.@map+"_red")+1;
					bg_updatescore strnpcinfo(4),getd("$@score_bomber"+.@map+"_blue"),getd("$@score_bomber"+.@map+"_red");

					// too much dead => game over
					if ( @BBM_HIT >= .game_over )
					{
						// Delete player from array
						deletearray getd(".PlayerList"+.@map+"["+.@i+"]"), 1 ;
						set .@i, .@i-1;

						// Clean var + items.
						set @BBM_spawn, 0;
						set @BBM_HIT, 0;
						set @BBM_INVINCIBLE, 0;
						set @BBM_HIT, 0;
						set @ZONE_EXPLOSION, 0;
						set @BBM_DELAY, 0;
						delitem $@ID_INCR_EXPL , countitem($@ID_INCR_EXPL) ;
						delitem $@ID_BOMB_BOX  , countitem($@ID_BOMB_BOX)  ;
						delitem $@ID_DELAY_DOWN, countitem($@ID_DELAY_DOWN);
						delitem $@ID_BOMB_KICK , countitem($@ID_BOMB_KICK) ;
						delitem $@ID_SPEED_UP , countitem($@ID_SPEED_UP) ;
						atcommand "@size 0";
						//sc_end SC_TIMERDOWN;
						cutin "",255;
						setpcblock PCBLOCK_ATTACK,0;

						warp .@map$,getd(".dead_respawn"+.@map+"[0]"),getd(".dead_respawn"+.@map+"[1]");

						// Not enough players, winner is here
						if ( getarraysize(getd(".PlayerList"+.@map+"")) < 2 ) {
							announce "[Bomberman] "+ rid2name(getd(".PlayerList"+.@map+"[0]")) + " just won this round ! 1 more point for this team !",bc_area, 0xffbbaa;
							if(getcharid(4,rid2name(getd(".PlayerList"+.@map+"[0]"))) == getd("$@bomberman"+.@map+"1"))
								setd("$@score_bomber"+.@map+"_blue"),getd("$@score_bomber"+.@map+"_blue")+1;
							else
								setd("$@score_bomber"+.@map+"_red"),getd("$@score_bomber"+.@map+"_red")+1;
							bg_updatescore strnpcinfo(4),getd("$@score_bomber"+.@map+"_blue"),getd("$@score_bomber"+.@map+"_red");
							if(getd("$@BBM_DESTROY"+.@map+""))
								rewardend();
							else
								onemoregame();
						}
					} else {
						// Début de l'invulnérabilité de 4 secondes.
						specialeffect2(120);
						set @BBM_INVINCIBLE, 1;
						addtimer 4000, strnpcinfo(0)+"::OnInvincibleEnd";
					}
				}
				detachrid;
			} else {
				// Delete Player
				deletearray getd(".PlayerList"+.@map+"["+.@i+"]"), 1 ;
				.@i -= 1;
			}
		}
	}

	function resetplayervar {

		.@map = atoi(strnpcinfo(1));
		// For all players...
		for( .@i = 0; .@i<getarraysize(getd(".PlayerList"+.@map+"")); .@i++ ) {
			// ... online...
			if( attachrid(getd(".PlayerList"+.@map+"["+.@i+"]")) ) {
				// Clean var + items
				set @BBM_INVINCIBLE, 0;
				set @BBM_HIT, 0;
				set @BBM_spawn, 0;
				delitem $@ID_INCR_EXPL , countitem($@ID_INCR_EXPL) ;
				delitem $@ID_BOMB_BOX  , countitem($@ID_BOMB_BOX)  ;
				delitem $@ID_DELAY_DOWN, countitem($@ID_DELAY_DOWN);
				delitem $@ID_BOMB_KICK , countitem($@ID_BOMB_KICK) ;
				delitem $@ID_SPEED_UP , countitem($@ID_SPEED_UP) ;

				if ( getarg(0,0) ){
					set @BBM_DELAY, 3;
					atcommand "@size 1";
					set @ZONE_EXPLOSION, .ZONE_EXPLOSION;
					getitem $@ID_BOMB_BOX,1;
					//sc_start SC_TIMERDOWN,301000,300;
				} else {
					set @ZONE_EXPLOSION, 0;
					set @BBM_DELAY, 0;
					bg_desert;
					//sc_end SC_TIMERDOWN;
					cutin "",255;
					setpcblock PCBLOCK_ATTACK,0;
					warp "bat_room",154,150;
					atcommand "@size 0";
				}

				detachrid;
			} else {
				// Delete Player
				deletearray getd(".PlayerList"+.@map+"["+.@i+"]"), 1 ;
				.@i -= 1;
			}
		}
	}

	function build_arena {
		.@map = atoi(strnpcinfo(1));
		for( set .@x,0; .@x<=getd(".lenght"+.@map+"")+1; set .@x,.@x+1 )
		{
			set .@x_, getd(".arena_settings"+.@map+"[0]")+.@x; 
			for( set .@y,0; .@y<=getd(".lenght"+.@map+"")+1; set .@y, .@y+1 )
			{

				set .@y_, getd(".arena_settings"+.@map+"[1]")+.@y;
				
				// Check if there is a monster/wall
				if(checkwall("BBM"+.@map+"_"+.@x_+"-"+.@y_)) delwall "BBM"+.@map+"_"+.@x_+"-"+.@y_;
				killmonster getd("$@BombRMAP"+.@map+"$"), ""+.@map+"Bomb#core::OnBBM"+.@map+"_"+.@x_+"-"+.@y_;

				// Pas de murs
				if ( checkcell(getd("$@BombRMAP"+.@map+"$"),.@x_,.@y_,cell_chkpass) )
				{
					// Une chance sur deux de spam un mur.
					if ( rand(2) )
					{
						 // zone d'apparition, pas de spam.
						if ( ( .@x < 3 && (.@y < 3 || .@y > 9 ) ) || ( .@x > 9 && ( .@y < 3 || .@y > 9) ) )
							continue;
						monster getd("$@BombRMAP"+.@map+"$"), .@x_, .@y_, "", getd(".BBM_BARRIERE_ID"+.@map+""), 1, strnpcinfo(0)+"::OnBBM"+.@map+"_"+.@x_+"-"+.@y_, getd(".BBM_BARRIERE_SIZE"+.@map+""), AI_BOMB;
						setwall getd("$@BombRMAP"+.@map+"$"), .@x_, .@y_, 1, 4, 0, "BBM"+.@map+"_"+.@x_+"-"+.@y_;
					}
				}
			}
		}
	}

	function isPlayerin {
		.@map = atoi(strnpcinfo(1));
		for( set .@i,0; .@i<getarraysize(getd(".PlayerList"+.@map+"")); set .@i, .@i+1 ) {
			if( attachrid(getd(".PlayerList"+.@map+"["+.@i+"]")) ) {	
				getmapxy .@map$, .@x, .@y, BL_PC, rid2name(getd(".PlayerList"+.@map+"["+.@i+"]"));
				if( .@map$ == getd("$@BombRMAP"+.@map+"$") && !distance(.@x,.@y,getarg(0),getarg(1)) )
					return 1;
			} else {
				deletearray getd(".PlayerList"+.@map+"["+.@i+"]"), 1 ;
				.@i -= 1;
			}
		}
		return 0;
	}
	
	function ExploseBombin {
		.@map = atoi(strnpcinfo(1));
		if ( set(.@npc,getd(".BBM"+.@map+"_XY_"+getarg(0)+"-"+getarg(1))) ){
			donpcevent .@map+"Bomb#"+.@npc+"::OnComboExplose";
		}
	}

	function destroy_xy {
		.@map = atoi(strnpcinfo(1));
		for(
			set .@i, getarg(0);
			(getarg(0) < getarg(1) ? .@i<=getarg(1) : .@i>=getarg(1)) ;
			set .@i,.@i+1 *( getarg(0) < getarg(1) ? 1 : -1 )
		){
			set .@x, getd(".arena_settings"+.@map+"[0]")+.@i; 
			for(
				set .@j, getarg(2);
				(getarg(2) < getarg(3) ? .@j<=getarg(3) : .@j>=getarg(3)) ;
				set .@j,.@j+1 *( getarg(2) < getarg(3) ? 1 : -1 )
			){
				set .@y, getd(".arena_settings"+.@map+"[1]")+.@j;
				if ( mobcount(getd("$@BombRMAP"+.@map+"$"),""+.@map+"Bomb#core::OnBBM"+.@map+"_"+.@x+"-"+.@y) ){
					killmonster getd("$@BombRMAP"+.@map+"$"), ""+.@map+"Bomb#core::OnBBM"+.@map+"_"+.@x+"-"+.@y;
					delwall "BBM"+.@map+"_"+.@x+"-"+.@y;
				}
				if(checkcell(getd("$@BombRMAP"+.@map+"$"),.@x,.@y,cell_chkpass)){
					monster getd("$@BombRMAP"+.@map+"$"), .@x, .@y, "", getd(".BBM_BARRIERE_ID"+.@map+""), 1, "", getd(".BBM_BARRIERE_SIZE"+.@map+""), AI_BOMB;
					setwall getd("$@BombRMAP"+.@map+"$"), .@x, .@y, 1, 4, 0, "BBM"+.@map+"_"+.@x+"-"+.@y;
				}

				cleanarea getd("$@BombRMAP"+.@map+"$"),.@x,.@y,.@x,.@y;
				checkplayerinarea(.@x,.@y,3,1);
				if (  ! getd("$@BBM_DESTROY"+.@map+"") )
					return 1;

				sleep(100);
			}
		}
	}

	function destroy_arena {
		soundeffectall "bomberman_timeup.wav",0,strnpcinfo(4);
		.@map = atoi(strnpcinfo(1));
		setd("$@BBM_DESTROY"+.@map+""), 1;
		for( .@i = 0; .@i<5; .@i++) {
			destroy_xy(1+1*.@i,(getd(".lenght"+.@map+"")-(1*.@i)),getd(".lenght"+.@map+"")-1*.@i,getd(".lenght"+.@map+"")-1*.@i);
			destroy_xy(getd(".lenght"+.@map+"")-1*.@i,getd(".lenght"+.@map+"")-1*.@i,(getd(".lenght"+.@map+"")-1)-1*.@i,1+1*.@i);
			destroy_xy((getd(".lenght"+.@map+"")-1)-1*.@i,1+1*.@i,1+1*.@i,1+1*.@i);
			destroy_xy(1+1*.@i,1+1*.@i,2+1*.@i,(getd(".lenght"+.@map+"")-1)-1*.@i);
		}
		bg_reserve strnpcinfo(4), true;
		sleep 10000;
		rewardend();
	}

	function rewardend {
		.@map = atoi(strnpcinfo(1));
		bg_unbook strnpcinfo(4);
		if(getd("$@score_bomber"+.@map+"_red") < getd("$@score_bomber"+.@map+"_blue")) {
			mapannounce strnpcinfo(4),"[Bomberman] The Blue team has the most points !",0xffbbaa;
			callfunc "BG_get_Rewards",getd("$@bomberman"+.@map+"1"),7829,3;
		} else if(getd("$@score_bomber"+.@map+"_blue") < getd("$@score_bomber"+.@map+"_red")) {
			mapannounce strnpcinfo(4),"[Bomberman] The Red team has the most points !",0xffbbaa;
			callfunc "BG_get_Rewards",getd("$@bomberman"+.@map+"2"),7829,3;
		} else {
			mapannounce strnpcinfo(4),"[Bomberman] DRAW !!",0xffbbaa;
			callfunc "BG_get_Rewards",getd("$@bomberman"+.@map+"1"),7829,1;
			callfunc "BG_get_Rewards",getd("$@bomberman"+.@map+"2"),7829,1;
		}
		setd("$@BBM_RUN"+.@map+""), 0;
		setd("$@score_bomber"+.@map+"_blue"),0;
		setd("$@score_bomber"+.@map+"_red"),0;
		playBGMall "182",strnpcinfo(4);
		soundeffectall "bomberman_end.wav",0,strnpcinfo(4);
		sleep 4000;
		resetplayervar();
		killmonster strnpcinfo(4), "All";
		cleanmap strnpcinfo(4);
		bg_reserve strnpcinfo(4), true;
		mapwarp strnpcinfo(4),"bat_room",154,150;
		if(getd("$@bomberman"+.@map+"1")) { bg_destroy getd("$@bomberman"+.@map+"1"); setd("$@bomberman"+.@map+"1"),0; }
		if(getd("$@bomberman"+.@map+"2")) { bg_destroy getd("$@bomberman"+.@map+"2"); setd("$@bomberman"+.@map+"2"),0; }
		bg_unbook strnpcinfo(4);
		do_init();
		if ( getarg(3,0) ){
			setd(".BBM_DESTROY"+.@map+""),0;
			return;
		}
	}

	function onemoregame {
		playBGMall "182",strnpcinfo(4);
		soundeffectall "bomberman_end.wav",0,strnpcinfo(4);
		sleep 3500;
		.@map = atoi(strnpcinfo(1));
		if(.@map == 1) setarray .@pos[0],25,42,35,32,25,32,35,42;
		else if(.@map == 2) setarray .@pos[0],141,164,155,164,155,150,141,150;
		else if(.@map == 3) setarray .@pos[0],94,85,106,85,106,73,94,73;
		deletearray getd(".PlayerList"+.@map+"[0]"), getarraysize(getd(".PlayerList"+.@map+""));
		killmonster strnpcinfo(4), "All";
		cleanmap strnpcinfo(4);

		deletearray $@arenamembers[0],getarraysize($@arenamembers);
		bg_get_data(getd("$@bomberman"+.@map+"1"),1);
		setd(".PlayerList"+.@map+"[0]"), $@arenamembers[0];
		setd(".PlayerList"+.@map+"[1]"), $@arenamembers[1];
		setpcblock PCBLOCK_ATTACK,1,$@arenamembers[0];
		if($@arenamembers[1]) setpcblock PCBLOCK_ATTACK,1,$@arenamembers[1];
		deletearray $@arenamembers[0],getarraysize($@arenamembers);
		bg_get_data(getd("$@bomberman"+.@map+"2"),1);
		setd(".PlayerList"+.@map+"[2]"), $@arenamembers[0];
		setd(".PlayerList"+.@map+"[3]"), $@arenamembers[1];
		setpcblock PCBLOCK_ATTACK,1,$@arenamembers[0];
		if($@arenamembers[1]) setpcblock PCBLOCK_ATTACK,1,$@arenamembers[1];
		for( .@i = 0; .@i < getarraysize(getd(".PlayerList"+.@map+"")); .@i++ )
			if(getd(".PlayerList"+.@map+"["+.@i+"]"))
				warpchar getd("$@BombRMAP"+.@map+"$"),.@pos[.@i*2],.@pos[.@i*2+1], getcharid(0,rid2name(getd(".PlayerList"+.@map+"["+.@i+"]")));

		resetplayervar(1);  // Initialisation des variables + items
		build_arena();      // Construction de la map
		sleep 100;
		soundeffectall "bomberman_intro.wav",0,strnpcinfo(4);
		sleep 2000;
		playBGMall "M81",strnpcinfo(4);
	}
}

//==============================================================
//= Duplicates
//==============================================================
bg_bomber,0,0,4	duplicate(1Bomb#core)	2Bomb#core	139
bg_bomber2,0,0,4	duplicate(1Bomb#core)	3Bomb#core	139

bomberman,0,0,4	duplicate(1Bomb#core)	Explosion1#BBM	139
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#1	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#2	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#3	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#4	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#5	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#6	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#7	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#8	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#9	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#10	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#11	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#12	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#13	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#14	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#15	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#16	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#17	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#18	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#19	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#20	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#21	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#22	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#23	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#24	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#25	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#26	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#27	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#28	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#29	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#30	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#31	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#32	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#33	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#34	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#35	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#36	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#37	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#38	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#39	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#40	1904
bomberman,0,0,4	duplicate(1Bomb#core)	1Bomb#41	1904

bg_bomber,0,0,4	duplicate(2Bomb#core)	Explosion2#BBM	139
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#1	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#2	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#3	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#4	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#5	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#6	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#7	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#8	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#9	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#10	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#11	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#12	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#13	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#14	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#15	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#16	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#17	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#18	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#19	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#20	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#21	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#22	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#23	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#24	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#25	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#26	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#27	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#28	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#29	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#30	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#31	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#32	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#33	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#34	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#35	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#36	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#37	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#38	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#39	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#40	1904
bg_bomber,0,0,4	duplicate(2Bomb#core)	2Bomb#41	1904

bg_bomber2,0,0,4	duplicate(3Bomb#core)	Explosion3#BBM	139
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#1	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#2	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#3	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#4	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#5	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#6	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#7	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#8	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#9	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#10	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#11	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#12	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#13	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#14	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#15	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#16	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#17	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#18	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#19	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#20	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#21	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#22	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#23	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#24	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#25	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#26	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#27	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#28	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#29	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#30	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#31	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#32	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#33	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#34	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#35	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#36	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#37	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#38	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#39	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#40	1904
bg_bomber2,0,0,4	duplicate(3Bomb#core)	3Bomb#41	1904