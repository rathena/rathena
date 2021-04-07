sala_premmy,36,45,6	script	Estilista	122,{
set @mh,50;	   // Maximo estilo de cabelo
set @mhc,253;     // Maximo cor de Cabelo
set @mcc,553;     // Maximo Cor de Roupa

if (vip_status(VIP_STATUS_ACTIVE) == false ) end;

mes "[^FF0000Estilista^000000]";
    mes "Bem-vind"+ (Sex?"o":"a") +" "+strcharinfo(0)+",";
    mes "o que você gostaria de alterar?";
    next;
    if(select("Estilo do Cabelo","Cor do Cabelo","Cor da Roupa","Cancelar") == 4) close;
    if(@menu == 1) setarray .@cor[0],1,getbattleflag ("min_hair_style"),@mh;
    if(@menu == 2) setarray .@cor[0],6,getbattleflag ("min_hair_color"),@mhc;
    if(@menu == 3) setarray .@cor[0],7,getbattleflag ("min_cloth_color"),@mcc;
    while(1) {
        if(select("Próximo","Anterior","Digitar","Cancelar") == 4) close;
        if(@menu == 1)
            if(getlook(.@cor[0]) >= .@cor[2]) setlook .@cor[0],.@cor[1];
            else setlook .@cor[0],getlook(.@cor[0])+1;
        if(@menu == 2)
            if(getlook(.@cor[0]) <= .@cor[1]) setlook .@cor[0],.@cor[2];
            else setlook .@cor[0],getlook(.@cor[0])-1;
        if(@menu == 3) {
            input .@num,.@cor[1],.@cor[2];
            setlook .@cor[0],.@num;
        }
        mes "Número ^FF0000"+getlook(.@cor[0])+"^000000/"+.@cor[2];
        specialeffect2 339;        
 
	}
}