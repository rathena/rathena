//===== rAthena Script =======================================
//= War of Emperium - nguild guardians script
//===== By: ==================================================
//= kobra_k88
//===== Current Version: =====================================
//= 1.0
//===== Compatible With: =====================================
//= rAthena Project; RO Episode 4+
//===== Description: =========================================
//= Re-spawns guardians on server start if they have been
//= purchased.  Also announces when a guardian dies.
//===== Additional Comments: =================================
//= Based off existing guild scripts.  Do not know if it is accurate.[kobra_k88]
//============================================================


//------------------------------------------------------------------------------
nguild_alde,216,24,0	script	Guardian_N01	-1,{
OnAgitInit:
	if (GetCastleData("nguild_alde",10) == 1) guardian "nguild_alde",18,219,"Soldier Guardian",1287,"Guardian_N01::OnGuardianDied",0;
	if (GetCastleData("nguild_alde",11) == 1) guardian "nguild_alde",117,42,"Soldier Guardian",1287,"Guardian_N01::OnGuardianDied",1;
	if (GetCastleData("nguild_alde",12) == 1) guardian "nguild_alde",207,153,"Soldier Guardian",1287,"Guardian_N01::OnGuardianDied",2;
	if (GetCastleData("nguild_alde",13) == 1) guardian "nguild_alde",68,70,"Archer Guardian",1285,"Guardian_N01::OnGuardianDied",3;
	if (GetCastleData("nguild_alde",14) == 1) guardian "nguild_alde",187,140,"Archer Guardian",1285,"Guardian_N01::OnGuardianDied",4;
	if (GetCastleData("nguild_alde",15) == 1) guardian "nguild_alde",62,204,"Knight Guardian",1286,"Guardian_N01::OnGuardianDied",5;
	if (GetCastleData("nguild_alde",16) == 1) guardian "nguild_alde",113,100,"Knight Guardian",1286,"Guardian_N01::OnGuardianDied",6;
	if (GetCastleData("nguild_alde",17) == 1) guardian "nguild_alde",211,174,"Knight Guardian",1286,"Guardian_N01::OnGuardianDied",7;
	end;

OnGuardianDied:
	// Event when Guardian dies
	MapAnnounce "nguild_alde","A Guardian Has Fallen",17;
	end;
}

//------------------------------------------------------------------------------
nguild_gef,198,182,0	script	Guardian_N02	-1,{
OnAgitInit:
	if (GetCastleData("nguild_gef",10) == 1) guardian "nguild_gef",30,178,"Soldier Guardian",1287,"Guardian_N02::OnGuardianDied",0;
	if (GetCastleData("nguild_gef",11) == 1) guardian "nguild_gef",64,180,"Soldier Guardian",1287,"Guardian_N02::OnGuardianDied",1;
	if (GetCastleData("nguild_gef",12) == 1) guardian "nguild_gef",61,25,"Soldier Guardian",1287,"Guardian_N02::OnGuardianDied",2;
	if (GetCastleData("nguild_gef",13) == 1) guardian "nguild_gef",61,44,"Archer Guardian",1285,"Guardian_N02::OnGuardianDied",3;
	if (GetCastleData("nguild_gef",14) == 1) guardian "nguild_gef",189,43,"Archer Guardian",1285,"Guardian_N02::OnGuardianDied",4;
	if (GetCastleData("nguild_gef",15) == 1) guardian "nguild_gef",51,192,"Knight Guardian",1286,"Guardian_N02::OnGuardianDied",5;
	if (GetCastleData("nguild_gef",16) == 1) guardian "nguild_gef",49,67,"Knight Guardian",1286,"Guardian_N02::OnGuardianDied",6;
	if (GetCastleData("nguild_gef",17) == 1) guardian "nguild_gef",181,14,"Knight Guardian",1286,"Guardian_N02::OnGuardianDied",7;
	end;

OnGuardianDied:
	// Event when Guardian dies
	MapAnnounce "nguild_gef","A Guardian Has Fallen",17;
	end;
}
//------------------------------------------------------------------------------
nguild_pay,139,139,0	script	Guardian_N03	-1,{
OnAgitInit:
	if (GetCastleData("nguild_pay",10) == 1) guardian "nguild_pay",210,120,"Soldier Guardian",1287,"Guardian_N03::OnGuardianDied",0;
	if (GetCastleData("nguild_pay",11) == 1) guardian "nguild_pay",69,26,"Soldier Guardian",1287,"Guardian_N03::OnGuardianDied",1; 
	if (GetCastleData("nguild_pay",12) == 1) guardian "nguild_pay",23,141,"Soldier Guardian",1287,"Guardian_N03::OnGuardianDied",2;
	if (GetCastleData("nguild_pay",13) == 1) guardian "nguild_pay",224,87,"Archer Guardian",1285,"Guardian_N03::OnGuardianDied",3;
	if (GetCastleData("nguild_pay",14) == 1) guardian "nguild_pay",81,45,"Archer Guardian",1285,"Guardian_N03::OnGuardianDied",4;
	if (GetCastleData("nguild_pay",15) == 1) guardian "nguild_pay",214,53,"Knight Guardian",1286,"Guardian_N03::OnGuardianDied",5;
	if (GetCastleData("nguild_pay",16) == 1) guardian "nguild_pay",69,26,"Knight Guardian",1286,"Guardian_N03::OnGuardianDied",6;
	if (GetCastleData("nguild_pay",17) == 1) guardian "nguild_pay",23,141,"Knight Guardian",1286,"Guardian_N03::OnGuardianDied",7;
	end;

OnGuardianDied:
	// Event when Guardian dies
	MapAnnounce "nguild_pay","A Guardian Has Fallen",17;
	end;
}
//------------------------------------------------------------------------------
nguild_prt,197,197,0	script	Guardian_N04	-1,{
OnAgitInit:
	if (GetCastleData("nguild_prt",10) == 1) guardian "nguild_prt",196,92,"Soldier Guardian",1287,"Guardian_N04::OnGuardianDied",0;
	if (GetCastleData("nguild_prt",11) == 1) guardian "nguild_prt",113,200,"Soldier Guardian",1287,"Guardian_N04::OnGuardianDied",1;
	if (GetCastleData("nguild_prt",12) == 1) guardian "nguild_prt",111,186,"Soldier Guardian",1287,"Guardian_N04::OnGuardianDied",2;
	if (GetCastleData("nguild_prt",13) == 1) guardian "nguild_prt",76,202,"Archer Guardian",1285,"Guardian_N04::OnGuardianDied",3;
	if (GetCastleData("nguild_prt",14) == 1) guardian "nguild_prt",90,26,"Archer Guardian",1285,"Guardian_N04::OnGuardianDied",4;
	if (GetCastleData("nguild_prt",15) == 1) guardian "nguild_prt",58,59,"Knight Guardian",1286,"Guardian_N04::OnGuardianDied",5;
	if (GetCastleData("nguild_prt",16) == 1) guardian "nguild_prt",112,200,"Knight Guardian",1286,"Guardian_N04::OnGuardianDied",6;
	if (GetCastleData("nguild_prt",17) == 1) guardian "nguild_prt",101,194,"Knight Guardian",1286,"Guardian_N04::OnGuardianDied",7;
	end;

OnGuardianDied:
	// Event when Guardian dies
	MapAnnounce "nguild_prt","A Guardian Has Fallen",17;
	end;
}
