prontera,170,276,4	script	Registrar	680,{
    mes "[ Registrar ]";
    mes "Hi, "+strcharinfo(0);
    if(countitem(24501) < 1) {
        mes "Hum... I see you've misplaced";
        mes "your "+getitemname(24501)+".";
        next;
        mes "[ Registrar ]";
        mes "Here, take a new one and";
        mes "please be more careful this time.";
        close2;
        getitem 24501,1;
        end;
    } else {
        mes "Sorry I am a little busy";
        mes "at the moment.";
        close;
    }
}