// **************************************
// ** M V P s
// **************************************
// Observacoes:
// O nome do NPC precisa ser xxx#yyy onde xxx sera o nome do mvp e yyy a variavel de controle

-	script	MvpInstancesConfig	-1,{
end;
OnInit:
	$@MVPI_CHESTID = 1324;
	$@MAX_PARTY_MEMBERS = 24;
	$@ENABLE_RESET = 0;			// 1 to activate reset using a Reset Stone (6320)
	setarray $@eddga[0],	1277,30,1180,50,1321,40;
	setarray $@gtb[0],		1054,20,1111,20,1209,20;
	setarray $@phara[0],	1037,20,1154,50,1098,20;
	setarray $@mist[0],		1118,40,1139,40,1099,40;
	setarray $@phree[0],	1165,30,1119,30,1387,30;
	setarray $@snake[0],	1413,30,1412,30,1416,30;
	setarray $@tanee[0],	1179,20,1587,30,1584,60;
	setarray $@orchero[0],	1273,30,1189,20,1213,40;
	setarray $@orclord[0],	1211,50,1189,40,1023,30;
	setarray $@general[0],	1318,30,1315,30,1208,30;
	setarray $@stormy[0],	1249,40,1248,40,1515,10;
	setarray $@lotd[0],		1504,30,1505,30,1507,20;
	setarray $@garm[0],		1243,20,1887,20,1515,20;
	setarray $@dark[0],		1191,20,1192,40,1117,30;
	setarray $@bapho[0],	1037,20,1294,20,1101,20;
	setarray $@dracu[0],	1111,30,1130,40,1061,30;
	setarray $@doppel[0],	1061,40,1109,40,1035,20;
	setarray $@gunka[0],	1256,20,1255,30,1148,60;
	setarray $@drake[0],	1191,20,1216,40,1208,20;
	setarray $@moon[0],		1939,30,1180,30,1307,10;
	setarray $@maya[0],		1111,50,1194,30,1294,30;
	setarray $@osiris[0],	1146,20,1029,50,1191,20;
	setarray $@amon[0],		1191,20,1194,20,1297,20;
	setarray $@incs[0],		1375,20,1405,40,1401,40;
	setarray $@bacs[0],		1512,25,1514,25,1631,25;
	
	end;
}
