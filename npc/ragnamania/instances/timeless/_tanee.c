// --------------------------------------------------------------------------
yuno_in03,19,27,1	script	Lady Tanee#tanee	111,{
// --------------------------------------------------------------------------
callfunc("F_CheckParty",200,255,3,5,.quest_id);
callfunc "F_InstanceEnter",.quest_id,.instance_name$,strnpcinfo(2);
callfunc "F_CreateInstance",.quest_id,.instance_name$,200,255,3,5,strnpcinfo(2),instance_info(.instance_name$,IIT_ENTER_MAP),1688;
end;
OnInit:
	.instance_name$	= "Lady Tanee [Normal]";
	.quest_id		= 69505;
	end;
}
// --------------------------------------------------------------------------
ayo_dun02,1,1,3	script	Normal Chest#tanee	1324,{
// --------------------------------------------------------------------------
	if(!instance_id(IM_PARTY)) end;
    if(getd("'"+strnpcinfo(2))<3) end;
	callfunc "F_PrepareForLooting",'mvpid; //'
	end;
OnInit:
	disablenpc(strnpcinfo(0));
	end;
OnInstanceInit:
	enablenpc(instance_npcname(strnpcinfo(0)));
	.@num = getmapunits(BL_NPC,instance_mapname(strnpcinfo(4)),.@npcs[0]);
	for (.@i=0;.@i<.@num;.@i++)
		if(rid2name(.@npcs[.@i]) != instance_npcname(strnpcinfo(0)))
			disablenpc rid2name(.@npcs[.@i]);
	callfunc "F_WaveControl";
	end;
OnWaveControl:
	callfunc "F_WaveControl";
	end;
OnMvpDead:
	set getd("'"+strnpcinfo(2)), 3;
	getunitdata 'mvpGID, .@mvpdata;
	movenpc instance_npcname(strnpcinfo(0)),.@mvpdata[6],.@mvpdata[7];
	specialeffect 910,AREA; // Blue Sparks
	end;
}