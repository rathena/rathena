// -	script	bz_setup	-1,{
// 	end;
// OnInit:
// //  setarray $@bz_i_dg_id[0],0;		Charid of a char in an Existing Instance array
// //  setarray $@bz_i_dg_n$[0],""; 	Existing Instance Name array
// //	setarray $@bz_i_dg_in[0],-1; 	Existing Instance Invaded or not (-1/1) array
// 	setarray $@bz_m_name$[0],"ars_fild01","ars_fild04";
// 	setarray $@bz_i_name$[0],"Crypt of Gomorrah","Lighthouse of Traiana";

// 	setarray $@CryptofGomorrah$[0],"ars_fild01",82,144;
// 	end;
// }


// function	script	bz_i_create	{
// 	if(getcharid(1) < 1)
// 		callfunc "bz_party_up";
// 	else if(getcharid(1) && !is_party_leader(getcharid(1)))
// 		return;

// 	.@i$ = callfunc("bz_get_i_name");
// 	.@index = callfunc("bz_get_i_available",.@i$);
// 	mes "^0000FF"+.@i$+" Entrance^000000";
// 	if(.@index < 0) {
// 		callfunc "bz_i_create_sub",.@i$;
// 		return;
// 	}
// 	mes "Do you want to descend the dungeon or invade someone else's?";
// 	if(select("^0000CCEnter the Dungeon^000000:^0000CCInvade Someone Else's Dungeon^000000") == 1) {
// 		callfunc "bz_i_create_sub",.@i$;
// 	} else {
// 		if(.@index = callfunc("bz_get_i_available",.@i$) >= 0) {
// 			.@m$ = strcharinfo(3,$@bz_i_dg_id[.@index]);
// 			.@x  = instance_info($@bz_i_dg_n$[.@index],IIT_ENTER_X);
// 			.@y  = instance_info($@bz_i_dg_n$[.@index],IIT_ENTER_Y);

// 			debugmes "bz_i_dg_n : " + $@bz_i_dg_n$[.@index];
// 			debugmes "bz_i_dg_id :" + $@bz_i_dg_id[.@index];
// 			debugmes .@m$;
// 			debugmes "x : " + .@x + " y : " + .@y;

// 			$@bz_i_dg_in[.@index] = 1;
// 			warp .@m$,.@x,.@y;
// 		} else {
// 			dispbottom "No Instance found. Please try again.";
// 			return;
// 		}
	
// 	}
// 	return;
// }


// function	script	bz_i_create_sub	{
// 	@iname$ = getarg(0,"");//callfunc("bz_get_i_name");
// 	@iid = instance_create(@iname$);
// 	if (@iid < 0) {
// 		switch (@iid) {
// 			case -1: dispbottom "ERROR(instance_create): Tipo Invalido.",0x0000CC; break;
// 			case -2: dispbottom "ERROR(instance_create): Party Nao Encontrada.",0x0000CC; break;
// 			case -3: dispbottom "ERROR(instance_create): Instancia ja existe.",0x0000CC; break;
// 			case -4: dispbottom "ERROR(instance_create): Nao ha instancias livres no momento.",0x0000CC; break;
// 		}
// 		dispbottom "Dungeon creation failed",0x0000CC;
// 		end;
// 	}
// 	.@index = getarraysize($@bz_i_dg_id);
// 	setarray $@bz_i_dg_id[.@index],getcharid(0);
// 	setarray $@bz_i_dg_n$[.@index],@iname$;
// 	setarray $@bz_i_dg_in[.@index],-1; 

// 	return;
// }


// function	script	bz_get_i_name	{
// 	.@map$ = getarg(0,strcharinfo(3));
// 	.@index = inarray($@bz_m_name$, .@map$);
	
// 	return getelementofarray($@bz_i_name$,.@index);
// }


// // Returns a Dungeon occupied by a party but without an invader in it 
// function	script	bz_get_i_available	{
// 	.@n$ = getarg(0,"");
// 	if(.@n$ == "") return;
// 	.@j = getarraysize($@bz_i_dg_id);
// 	for(.@i=0;.@i<.@j;.@i++)
// 		if($@bz_i_dg_n$[.@i] == .@n$ && $@bz_i_dg_in[.@i] < 0)
// 			return  .@i; //Return the index of a free instance to be invaded

// 	return -1;
// }


// function	script	bz_i_enter	{
// 	.@enter = instance_enter(instance_live_info(ILI_NAME,instance_id(IM_PARTY)));
// 	if (.@enter != 0) {
// 		switch (.@enter) {
// 			case 1: debugmes "ERROR(instance_enter): Party not found."; break;
// 			case 2: debugmes "ERROR(instance_enter): Party doesn't have an instance."; break;
// 			case 3: debugmes "ERROR(instance_enter): Unknown Error."; break;
// 		}
// 	}
// 	return;
// }


// function	script	bz_party_up	{
// 	.@ms$ = gettimetick(2);
// 	party_create(md5(.@ms$));
// 	sleep2 200;
// 	return;
// }


// function	script	bz_remove_space	{
// 	.@s$ = getarg(0);
// 	while(.@i < getstrlen(.@s$)){
// 		.@i++;
// 		if(charat(.@s$,.@i) == " ")
// 			delchar(.@s$,.@i);
// 	}
// 	return .@s$;
// }