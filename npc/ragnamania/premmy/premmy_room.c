sala_premmy,43,57,1	script	HuntingQuests	111,{
end;
OnInit:
	end;
OnPCLoadMapEvent:
	if(strcharinfo(3) == "sala_premmy") {
		if(!vip_status(1) && getgmlevel()<10) 
			warp "SavePoint",0,0;
	}
	end;
}


//----------warps-------------
sala_premmy,71,38,0	warp	premmy_02a	1,1,sala_premmy,42,38
sala_premmy,46,38,0	warp	premmy_02b	1,1,sala_premmy,75,38
sala_premmy,71,54,0	warp	premmy_03a	1,1,sala_premmy,42,54
sala_premmy,46,54,0	warp	premmy_03b	1,1,sala_premmy,75,54
sala_premmy,34,66,0	warp	premmy_04a	1,1,sala_premmy,20,108
sala_premmy,20,103,0	warp	premmy_04b	1,1,sala_premmy,34,61
sala_premmy,84,66,0	warp	premmy_05a	1,1,sala_premmy,44,107
sala_premmy,44,103,0	warp	premmy_05b	1,1,sala_premmy,84,62
sala_premmy,32,128,0	warp	premmy_06a	1,1,sala_premmy,23,171
sala_premmy,22,167,0	warp	premmy_06b	1,1,sala_premmy,32,123
sala_premmy,90,47,0	warp	premmy_07a	1,1,sala_premmy,119,47
sala_premmy,115,47,0	warp	premmy_07b	1,1,sala_premmy,85,47
sala_premmy,126,75,0	warp	premmy_08a	1,1,sala_premmy,180,176
sala_premmy,180,171,0	warp	premmy_08b	1,1,sala_premmy,126,70
sala_premmy,175,71,0	warp	premmy_09	1,1,sala_premmy,136,64


//Mapflags
sala_premmy	mapflag	nowarpto
sala_premmy	mapflag	nomemo
sala_premmy	mapflag	nosave	SavePoint
sala_premmy	mapflag	nobranch
sala_premmy	mapflag	nowarp
sala_premmy	mapflag	noteleport
sala_premmy	mapflag	nochat
sala_premmy	mapflag	novending
sala_premmy	mapflag	loadevent