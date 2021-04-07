-	script	Season_Config	-1,{
	end;

OnInit:
	if(!$SEASON_CURRENT) $SEASON_CURRENT = 1;
	//$SEASON_CURRENT = xxx;
	end;

OnClock0000:
	if(gettime(DT_DAYOFWEEK) == MONDAY) $SEASON_WEEK++;
	debugmes "ENLIVEN SEASON : " + $SEASON_CURRENT + "   WEEK : " + $SEASON_WEEK;
	end;
}