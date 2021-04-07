-	script	HotFixes	-1,{
	end;

	OnInit:
	end;

	OnPCLoginEvent:
		if(HotFix_StatsReset == 1){
			end;
		} else {
			resetstatus();
			set HotFix_StatsReset, 1;
			dispbottom "Seus pontos de Stats foram resetados automaticamente.",0xFFFFFF;
			dispbottom "devido a um problema que tivemos no banco de dados.",0xFFFFFF;
			dispbottom "Pedimos desculpa pelo inconveniente :(",0xFFFFFF;
		}
	end;	
}