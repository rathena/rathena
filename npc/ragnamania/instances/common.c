-	script	InstancesCommon	-1,{
	end;

	OnInit:
		setarray .tower$[0],"1@tower","2@tower","3@tower","4@tower","5@tower","6@tower";
		end;
	
	OnNPCKillEvent:
		if (instance_id(IM_PARTY) > 0) {
			getmapxy(.@map$,.@x,.@y,BL_PC);
			if(inarray(.tower$[0],.@map$)) {
				if(rand(0,1000) > 980)
					makeitem 675,1,.@map$,.@x,.@y;
			}
		}
		end;
}