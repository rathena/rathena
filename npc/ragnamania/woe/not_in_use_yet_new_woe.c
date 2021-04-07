// WoE Automatic Castle Rotation
// ==============================================================
// This scripts add a NPC on Prontera (near to the center)
// It will show info to the players about what castles are opened
// and what castles will be opened next WoE.
// Read the script for more information and Settings.
//
// ABOUT ANCIENT WOE:
// Ancient WoE is a replacement of Novice Castles, if you want
// to enable it, add the Ancient WoE scripts to your server.
// ==============================================================

prontera,164,174,3	script	WoE Info Board	837,{
	if( .Ready != 1 ) donpcevent "WoE Info Board::OnInit";

	mes "[^FFA500 WoE Info Board ^000000]";

// WoE Classic
	mes "- Opened Castles for Classic WoE";
	for( set .@k, 0; $Castles_FE[.@k] != 0; set .@k, .@k + 1 )
		mes "^0000FF" + getcastlename(.Castles_FE$[$Castles_FE[.@k]]) + "^000000 (" + .Castles_FE$[$Castles_FE[.@k]] + ")";

// WoE Second Edition
	mes "- Opened Castles for WoE SE";
	for( set .@k, 0; $Castles_SE[.@k] != 0; set .@k, .@k + 1 )
		mes "^0000FF" + getcastlename(.Castles_SE$[$Castles_SE[.@k]]) + "^000000 (" + .Castles_SE$[$Castles_SE[.@k]] + ")";

// Ancient WoE - Uncomment if you use it
/*	mes "- Opened Castles for Ancient WoE";
	for( set .@k, 0; $Castles_AE[.@k] != 0; set .@k, .@k + 1 )
		mes "^0000FF" + getcastlename(.Castles_AE$[$Castles_AE[.@k]]) + "^000000 (" + .Castles_AE$[$Castles_AE[.@k]] + ")";
*/
	next;
	mes "[^FFA500 WoE Info Board ^000000]";
	mes "- Next Castles for Classic WoE";
	for( set .@k, 0; $Castles_FE[.@k] != 0; set .@k, .@k + 1 )
	{
		if( set(.@c, $Castles_FE[.@k] + 1) > 20 )
			set .@c, 1;

		mes "^0000FF" + getcastlename(.Castles_FE$[.@c]) + "^000000 (" + .Castles_FE$[.@c] + ")";
	}
	mes "- Next Castles for WoE SE";
	for( set .@k, 0; $Castles_SE[.@k] != 0; set .@k, .@k + 1 )
	{
		if( set(.@c, $Castles_SE[.@k] + 1) > 6 )
			set .@c, 1;

		mes "^0000FF" + getcastlename(.Castles_SE$[.@c]) + "^000000 (" + .Castles_SE$[.@c] + ")";
	}

// Ancient WoE - Uncomment if you use it
/*	mes "- Next Castles for Ancient WoE";
	for( set .@k, 0; $Castles_AE[.@k] != 0; set .@k, .@k + 1 )
	{
		if( set(.@c, $Castles_AE[.@k] + 1) > 4 )
			set .@c, 1;

		mes "^0000FF" + getcastlename(.Castles_AE$[.@c]) + "^000000 (" + .Castles_AE$[.@c] + ")";
	}
*/
	next;
	mes "[^FFA500 WoE Info Board ^000000]";
	mes "Castle Rotation is programed each Friday at 5 a.m.";
	close;

OnInit:
	setarray .Castles_FE$[1], // This array holds all WoE First Edition Castles, starting from index 1
		"prtg_cas01",	// 1
		"payg_cas01",	// 2
		"gefg_cas01",	// 3
		"aldeg_cas01",	// 4
		"prtg_cas03",	// 5
		"payg_cas03",	// 6
		"gefg_cas03",	// 7
		"aldeg_cas03",	// 8
		"prtg_cas05",	// 9
		"payg_cas05",	// 10
		"gefg_cas05",	// 11
		"aldeg_cas05",	// 12
		"prtg_cas02",	// 13
		"payg_cas02",	// 14
		"gefg_cas02",	// 15
		"aldeg_cas02",	// 16
		"prtg_cas04",	// 17
		"payg_cas04",	// 18
		"gefg_cas04",	// 19
		"aldeg_cas04";	// 20
	setarray .Castles_AE$[1], // This array holds all Ancient Edition WoE Castles, starting from index 1
		"te_prtcas01",	// 1
		"te_prtcas02",	// 2
		"te_prtcas03",	// 3
		"te_prtcas04",	// 4
		"te_prtcas05",	// 5
		"te_aldecas1",	// 6
		"te_aldecas2",	// 7
		"te_aldecas3",	// 8
		"te_aldecas4",	// 9
		"te_aldecas5";	// 10
	setarray .Castles_SE$[1], // This array holds all WoE Second Edition Castles, starting from index 1
		"schg_cas01",	// 1
		"schg_cas02",	// 2
		"schg_cas03",	// 3
		"arug_cas01",	// 4
		"arug_cas02",	// 5
		"arug_cas03";	// 6
	setarray .Castles_NE$[1], // This array holds all Noob Edition WoE Castles, starting from index 1
		"nguild_alde",	// 1
		"nguild_gef",	// 2
		"nguild_pay",	// 3
		"nguild_prt",	// 4
	setarray .Castles_SE_Event$[1], // This array include the Events inside each castle of WoE Second Edition (need to be one on one with the SE Castle Array)
		"::OnRecvCastleSc01",
		"::OnRecvCastleSc02",
		"::OnRecvCastleSc03",
		"::OnRecvCastleAr01",
		"::OnRecvCastleAr02",
		"::OnRecvCastleAr03";

	// First Time Rotation Setting - CONFIGURATION PART:
	// =======================================================================
	// This arrays holds the current Opened Castles for each kind of WoE.
	// Obviously, FE, SE and AE points to First Edition, Second Edition and
	// Ancient Edition, just like the Castle Array.
	//
	// The next arrays, starting from Index 0, holds the INDEX of the previous
	// castle arrays. In the example, for FE:
	// 1 = prtg_cas01
	// 13 = prtg_cas02
	//
	// If this is the first time you are going to use this script, find the index
	// of your current Castles (your server) in the Castle Arrays, and set the
	// list in the next arrays. If your server is currently playing on payg_cas03,
	// aldeg_cas05 and prtg_cas04, the FE array should say: 6,12,17
	// Same for SE, and uncomment the AE if you use Ancient Edition.
	//
	// The Main unique rule on this script is: Do not use consecutive castles in
	// the castle array.
	// Just add more index if you use more castles, there is no problem.
	// If you are going to reduce castle amount, well, it will require DB edit
	// not only on guild_castle, also on mapreg, edition the Castle_FE or SE or AE
	// values, or using a script to edit it, because, this setting only run first
	// time.

	if( getarraysize($Castles_FE) == 0 ) setarray $Castles_FE[0],4,9;
	if( getarraysize($Castles_SE) == 0 ) setarray $Castles_SE[0],3;
	if( getarraysize($Castles_AE) == 0 ) setarray $Castles_AE[0],1,2,3,4,5,6,7,8,9,10;
	if( getarraysize($Castles_NE) == 0 ) setarray $Castles_SE[0],1,2,3,4;

	// Reset Castle index:
	// This is required if you are planing to reduce/increase opened castles
	// Uncomment to remove current index, but you will need to recalculate
	// and configure again the Onwer Indexs (previous arrays).

	// cleararray $Castles_FE[0],0,getarraysize($Castles_FE);
	// cleararray $Castles_SE[0],0,getarraysize($Castles_SE);
	// cleararray $Castles_AE[0],0,getarraysize($Castles_AE);
	
	// =======================================================================

	// Block Castles - FE
	for( set .@i, 1; .@i < 21; set .@i, .@i + 1 )
		setmapflag .Castles_FE$[.@i],mf_blocked;
	// Block Castles - SE
	for( set .@i, 1; .@i < 7; set .@i, .@i + 1 )
		setmapflag .Castles_SE$[.@i],mf_blocked;
	// Block Castles - AE
	for( set .@i, 1; .@i < 11; set .@i, .@i + 1 )
		setmapflag .Castles_AE$[.@i],mf_blocked;
	// Block Castles - NE
	for( set .@i, 1; .@i < 5; set .@i, .@i + 1 )
		setmapflag .Castles_NE$[.@i],mf_blocked;

	// UnBlock Current Castles
	for( set .@i, 0; $Castles_FE[.@i] != 0; set .@i, .@i + 1 )
		removemapflag .Castles_FE$[$Castles_FE[.@i]],mf_blocked;
	for( set .@i, 0; $Castles_SE[.@i] != 0; set .@i, .@i + 1 )
		removemapflag .Castles_SE$[$Castles_SE[.@i]],mf_blocked;
	for( set .@i, 0; $Castles_AE[.@i] != 0; set .@i, .@i + 1 )
		removemapflag .Castles_AE$[$Castles_AE[.@i]],mf_blocked;
	for( set .@i, 0; $Castles_NE[.@i] != 0; set .@i, .@i + 1 )
		removemapflag .Castles_NE$[$Castles_NE[.@i]],mf_blocked;

	set .Ready, 1;
	end;

OnWhisperGlobal:
// If you whisp to NPC:WoE Info Board the word rotate, a rotation will be made. It can be used to test the system.
	if( getgmlevel() < 60 )
		end;
	if( @whispervar0$ != "rotate" )
		end;

OnFri0500:
	if( .Ready != 1 ) donpcevent "WoE Info Board::OnInit";

	rankreset 0; // WoE Ranking Restart - Comment this if you don't want to reset WoE Ranking each week. You can use @rankreset 1 to reset it manually.

	announce "<- Castle Rotation Begins ->",0,0xFFA500;
	for( set .@i, 0; $Castles_FE[.@i] != 0; set .@i, .@i + 1 )
		callsub S_MoveFE,.@i;
	for( set .@i, 0; $Castles_SE[.@i] != 0; set .@i, .@i + 1 )
		callsub S_MoveSE,.@i;
	for( set .@i, 0; $Castles_AE[.@i] != 0; set .@i, .@i + 1 )
		callsub S_MoveAE,.@i;
	for( set .@i, 0; $Castles_NE[.@i] != 0; set .@i, .@i + 1 )
		callsub S_MoveNE,.@i;
	announce "<- Castle Rotation is Over ->",0,0xFFA500;
	end;

S_MoveFE:
	// Move to next First Edition Castle - Do not Edit
	// ========================================
	set .@Castle$, .Castles_FE$[$Castles_FE[getarg(0)]];

	// A. Store current Castle Information
	// ========================================
	for( set .@j, 17; .@j > 0; set .@j, .@j - 1 )
	{
		set .@TempData[.@j], getcastledata(.@Castle$,.@j);
		setcastledata .@Castle$,.@j,0; // Clear current Data
	}

	maprespawnguildid .@Castle$,0,3; // Kick Everybody inside
	killmonsterall .@Castle$; // Remove Monsters and Treasures
	setmapflag .@Castle$,mf_blocked; // Block Castle
	getcastledata .@Castle$,0,"Agit#" + .@Castle$ + "::OnRecvCastle"; // Refresh Flags

	// B. Transfer Data to New Castle
	// ========================================
	if( set($Castles_FE[getarg(0)],$Castles_FE[getarg(0)] + 1) > 20 )
		set $Castles_FE[getarg(0)],1; // Rotation circle

	set .@OldCastle$, .@Castle$;
	set .@Castle$, .Castles_FE$[$Castles_FE[getarg(0)]]; // New Castle

	// C. Recover Data
	// ========================================
	for( set .@j, 1; .@j < 18; set .@j, .@j + 1 )
		setcastledata .@Castle$,.@j,.@TempData[.@j];

	killmonsterall .@Castle$;
	removemapflag .@Castle$,mf_blocked; // UnBlock Castle
	getcastledata .@Castle$,0,"Agit#" + .@Castle$ + "::OnRecvCastle"; // Refresh Flags
	
	if( .@TempData[1] ) announce "Guild [" + getguildname(.@TempData[1]) + "] owner of [" + getcastlename(.@OldCastle$) + "] transfered to [" + getcastlename(.@Castle$) + "]",0,0x00BFFF;
	return;

S_MoveAE:
	// Move to next First Edition Castle - Do not Edit
	// ========================================
	set .@Castle$, .Castles_AE$[$Castles_AE[getarg(0)]];

	// A. Store current Castle Information
	// ========================================
	for( set .@j, 17; .@j > 0; set .@j, .@j - 1 )
	{
		set .@TempData[.@j], getcastledata(.@Castle$,.@j);
		setcastledata .@Castle$,.@j,0; // Clear current Data
	}

	maprespawnguildid .@Castle$,0,3; // Kick Everybody inside
	killmonsterall .@Castle$; // Remove Monsters and Treasures
	setmapflag .@Castle$,mf_blocked; // Block Castle
	getcastledata .@Castle$,0,"Agit#" + .@Castle$ + "::OnRecvCastle"; // Refresh Flags

	// B. Transfer Data to New Castle
	// ========================================
	if( set($Castles_AE[getarg(0)],$Castles_AE[getarg(0)] + 1) > 4 )
		set $Castles_AE[getarg(0)],1; // Rotation circle

	set .@OldCastle$, .@Castle$;
	set .@Castle$, .Castles_AE$[$Castles_AE[getarg(0)]]; // New Castle

	// C. Recover Data
	// ========================================
	for( set .@j, 1; .@j < 18; set .@j, .@j + 1 )
		setcastledata .@Castle$,.@j,.@TempData[.@j];

	killmonsterall .@Castle$;
	removemapflag .@Castle$,mf_blocked; // UnBlock Castle
	getcastledata .@Castle$,0,"Agit#" + .@Castle$ + "::OnRecvCastle"; // Refresh Flags
	
	if( .@TempData[1] ) announce "Guild [" + getguildname(.@TempData[1]) + "] owner of [" + getcastlename(.@OldCastle$) + "] transfered to [" + getcastlename(.@Castle$) + "]",0,0x00BFFF;
	return;

S_MoveSE:
	// Move to next First Edition Castle - Do not Edit
	// ========================================
	set .@Castle$, .Castles_SE$[$Castles_SE[getarg(0)]];

	// A. Store current Castle Information
	// ========================================
	for( set .@j, 17; .@j > 0; set .@j, .@j - 1 )
	{
		set .@TempData[.@j], getcastledata(.@Castle$,.@j);
		setcastledata .@Castle$,.@j,0; // Clear current Data
	}

	maprespawnguildid .@Castle$,0,3; // Kick Everybody inside
	killmonsterall .@Castle$; // Remove Monsters and Treasures
	setmapflag .@Castle$,mf_blocked; // Block Castle
	getcastledata .@Castle$,0,.Castles_SE_Event$[$Castles_SE[getarg(0)]]; // Refresh Flags

	// B. Transfer Data to New Castle
	// ========================================
	if( set($Castles_SE[getarg(0)],$Castles_SE[getarg(0)] + 1) > 6 )
		set $Castles_SE[getarg(0)],1; // Rotation circle

	set .@OldCastle$, .@Castle$;
	set .@Castle$, .Castles_SE$[$Castles_SE[getarg(0)]]; // New Castle

	// C. Recover Data
	// ========================================
	for( set .@j, 1; .@j < 18; set .@j, .@j + 1 )
		setcastledata .@Castle$,.@j,.@TempData[.@j];

	killmonsterall .@Castle$;
	removemapflag .@Castle$,mf_blocked; // UnBlock Castle
	getcastledata .@Castle$,0,.Castles_SE_Event$[$Castles_SE[getarg(0)]]; // Refresh Flags

	if( .@TempData[1] ) announce "Guild [" + getguildname(.@TempData[1]) + "] owner of [" + getcastlename(.@OldCastle$) + "] transfered to [" + getcastlename(.@Castle$) + "]",0,0x00BFFF;
	return;
}
S_MoveNE:
	// Move to next Noob Edition Castle - Do not Edit
	// ========================================
	set .@Castle$, .Castles_NE$[$Castles_NE[getarg(0)]];

	// A. Store current Castle Information
	// ========================================
	for( set .@j, 17; .@j > 0; set .@j, .@j - 1 )
	{
		set .@TempData[.@j], getcastledata(.@Castle$,.@j);
		setcastledata .@Castle$,.@j,0; // Clear current Data
	}

	maprespawnguildid .@Castle$,0,3; // Kick Everybody inside
	killmonsterall .@Castle$; // Remove Monsters and Treasures
	setmapflag .@Castle$,mf_blocked; // Block Castle
	getcastledata .@Castle$,0,"Agit#" + .@Castle$ + "::OnRecvCastle"; // Refresh Flags

	// B. Transfer Data to New Castle
	// ========================================
	if( set($Castles_NE[getarg(0)],$Castles_NE[getarg(0)] + 1) > 4 )
		set $Castles_NE[getarg(0)],1; // Rotation circle

	set .@OldCastle$, .@Castle$;
	set .@Castle$, .Castles_NE$[$Castles_NE[getarg(0)]]; // New Castle

	// C. Recover Data
	// ========================================
	for( set .@j, 1; .@j < 18; set .@j, .@j + 1 )
		setcastledata .@Castle$,.@j,.@TempData[.@j];

	killmonsterall .@Castle$;
	removemapflag .@Castle$,mf_blocked; // UnBlock Castle
	getcastledata .@Castle$,0,"Agit#" + .@Castle$ + "::OnRecvCastle"; // Refresh Flags
	
	if( .@TempData[1] ) announce "Guild [" + getguildname(.@TempData[1]) + "] owner of [" + getcastlename(.@OldCastle$) + "] transfered to [" + getcastlename(.@Castle$) + "]",0,0x00BFFF;
	return;

// MapFlags - Castle Configurations
// ========================================

aldeg_cas01	mapflag	woe_set	1
aldeg_cas02	mapflag	woe_set	1
aldeg_cas03	mapflag	woe_set	1
aldeg_cas04	mapflag	woe_set	1
aldeg_cas05	mapflag	woe_set	1

gefg_cas01	mapflag	woe_set	1
gefg_cas02	mapflag	woe_set	1
gefg_cas03	mapflag	woe_set	1
gefg_cas04	mapflag	woe_set	1
gefg_cas05	mapflag	woe_set	1

payg_cas01	mapflag	woe_set	1
payg_cas02	mapflag	woe_set	1
payg_cas03	mapflag	woe_set	1
payg_cas04	mapflag	woe_set	1
payg_cas05	mapflag	woe_set	1

prtg_cas01	mapflag	woe_set	1
prtg_cas02	mapflag	woe_set	1
prtg_cas03	mapflag	woe_set	1
prtg_cas04	mapflag	woe_set	1
prtg_cas05	mapflag	woe_set	1

// WoE SE

schg_cas01	mapflag	woe_set	2
schg_cas02	mapflag	woe_set	2
schg_cas03	mapflag	woe_set	2
schg_cas04	mapflag	woe_set	2
schg_cas05	mapflag	woe_set	2
arug_cas01	mapflag	woe_set	2
arug_cas02	mapflag	woe_set	2
arug_cas03	mapflag	woe_set	2
arug_cas04	mapflag	woe_set	2
arug_cas05	mapflag	woe_set	2

schg_cas04	mapflag	blocked
schg_cas05	mapflag	blocked
arug_cas04	mapflag	blocked
arug_cas05	mapflag	blocked


// Ancient WoE

te_prtcas01	mapflag	woe_set	3
te_prtcas02	mapflag	woe_set	3
te_prtcas03	mapflag	woe_set	3
te_prtcas04	mapflag	woe_set	3
te_prtcas05	mapflag	woe_set	3

te_prtcas01	mapflag	ancient
te_prtcas02	mapflag	ancient
te_prtcas03	mapflag	ancient
te_prtcas04	mapflag	ancient
te_prtcas05	mapflag	ancient


// Noob WoE

nguild_alde	mapflag	woe_set	4
nguild_gef	mapflag	woe_set	4
nguild_pay	mapflag	woe_set	4
nguild_prt	mapflag	woe_set	4


// About MapFlags:
// ====================================================
// blocked = cannot be acceced, only by Game Masters.
// guild_max	<value> = limit the number of the same guild members inside this map
// ancient = required to limit this castle to ancient items and classes.
// woe_set	<value> = required, do not edit. This marks the map to know when it will be opened for woe, or blocked if other woe is running. Check agit_controller.txt for more information.
