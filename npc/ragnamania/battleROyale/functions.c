// Ragnamania battleROyale
// by Biali
// v1.0


// Inform other players of the number
// of players in the queue
// *****************************************************
function	script	UpdateQueueStatus	{
	freeloop(1);
	for (.@i=0; .@i<getarraysize($@br_queue); .@i++) {
		if(isloggedin(getcharid(3,strcharinfo(0,$@br_queue[.@i])))) {
			dispbottom "[ Battle ROyale ] You are in the queue for the next match. ("+getarraysize($@br_queue)+"/"+ $@ARENA_START_NUM_PLAYERS +") Type @queue to leave.",0x00DDFF,$@br_queue[.@i];
		} 
	}
	freeloop(0);
	return;
}


// Calculate the Distance from the Player 
// to the Center of the Arena
// *****************************************************
function	script	CalcDistance	{
	.@charid = getarg(0,0);
	
	if(!.@charid)
		return;

	getmapxy(.@mapPC$, .@pcX, .@pcY, BL_PC,strcharinfo(0,.@charid)); 
	getmapxy(.@mapNPC$, .@npcX, .@npcY, BL_NPC,"#ArenaController"); 
	.@distance = sqrt(pow((.@npcX - .@pcX),2) + pow((.@npcY - .@pcY),2));

	return .@distance;
}



function	script	ClearInventory	{
	.@cid = getarg(0,getcharid(0));

	if (!.@cid)
		return;
	
	debugmes "ClearInventory : Cleaning " + strcharinfo(0,.@cid) + "' inventory.";

	if(!getvar(@inventorylist_count,.@cid)){
		getinventorylist .@cid;
	}

	for(.c=0;.@c<getvar(@inventorylist_count,.@cid);.@c++) {
		.@itemid = getvar(@inventorylist_id[.@c],.@cid);
		.@itemqt = getvar(@inventorylist_amount[.@c],.@cid);
		if(.@itemid != 24501)  //Diario de bordo
			delitem .@itemid,.@itemqt,getcharid(3,strcharinfo(0,.@cid));
	}
	return;
}


function	script	OrdealOf	{
	.@var$ = getarg(0,-1);

	if(.@var$ == "-1")
		return 0;

	.@sz = getstrlen(.@var$);
	switch (charat(.@var$,.@sz)) {
		case 1:
			.@ordeal$ = "st";
			break;
		case 2:
			.@ordeal$ = "nd";
			break;
		case 3:
			.@ordeal$ = "rd";
			break;
		default:
			.@ordeal$ = "th";
			break;
	}

	return .@ordeal$;
}

function	script	blue	{
	return "^0000AA" + getarg(0) + "^000000";
}
function	script	green	{
	return "^00AA00" + getarg(0) + "^000000";
}
function	script	red	{
	return "^AA0000" + getarg(0) + "^000000";
}
function	script	grey	{
	return "^AAAAAA" + getarg(0) + "^000000";
}






