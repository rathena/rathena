-	script	MenuSetting	FAKE_NPC,{
OnInit:
	$@pre_level$ = "^FF0088¡ö¦^¨ì¤W¼h¿ï³æ¡ö^000000";
	$@pre_page$ = "^227700¡ô¤W¤@­¶¡ô^000000";
	$@next_page$ = "^227700¡õ¤U¤@­¶¡õ^000000";
	$@menu_search$ = "^FF8800¿é¤J·j´M¤å¦r^000000";
	//$@NumberPerPage = 30;
	end;
}

/*  MenuInit ()
 *  ²MªÅ¼È¦s@Menu
 */
function	script	MenuInit	{
	deletearray @Menu_menu$;
	deletearray @Menu_func$;
	deletearray @Menu_Group;
	
	@Menu_count = 0;
	@Menu_Limit = 30;
	@Menu_LastGroup = 0;
	
	if (getargcount() > 0)
	if (getarg(0) > 0)
		@Menu_Limit = getarg(0);
	
	return;
}

/*  MenuGroup (Name)
 *  ¦b¼È¦s@Menu¤¤·s¼W¤@­Ó Group ¶µ¥Ø
 */
function	script	MenuGroup	{
	for (.@i = 0; .@i < getargcount(); .@i++) {
		@Menu_menu$[.@i + @Menu_count] = getarg(.@i);
		@Menu_Group[.@i + @Menu_count] = 1;
		@Menu_LastGroup = .@i + @Menu_count;
	}

	deletearray getd("@Menu_type_"+@Menu_LastGroup+"$");
	deletearray getd("@Menu_name_"+@Menu_LastGroup+"$");
	@Menu_count += getargcount();
	return;
}

/*  MenuGroup (Name)
 *  ¦b¼È¦s@Menu¤¤·s¼W¤@­Ó Group ¶µ¥Ø
 */
function	script	MenuGroupAdd	{
	.@currentMneuCount = getarraysize(getd("@Menu_type_"+@Menu_LastGroup+"$"));
	setd2 "@Menu_type_"+@Menu_LastGroup+"$", .@currentMneuCount, getarg(0);
	setd2 "@Menu_name_"+@Menu_LastGroup+"$", .@currentMneuCount, getarg(1);
	return;
}

/*  MenuAdd (Name)
 *  ¦b¼È¦s@Menu¤¤·s¼W¤@­Ó Name ¶µ¥Ø
 */
function	script	MenuAdd	{
	for (.@i = 0; .@i < getargcount(); .@i++)
		@Menu_menu$[.@i + @Menu_count] = getarg(.@i);

	@Menu_count += getargcount();
	
	return;
}

/*  MenuAddFunc (Name, Funcname)
 *  ¦b¼È¦s@Menu¤¤·s¼W¤@­Ó Name ¶µ¥Ø¡A¨ÃÅý¨Ï¥ÎŽÍ¿ï¾Ü«á³sµ²¨ì Funcname ªºFunction
 */
function	script	MenuAddFunc	{
	@Menu_menu$[@Menu_count] = getarg(0);
	@Menu_func$[@Menu_count] = getarg(1);
	@Menu_count++;

	return;
}

/*  MenuAddFuncSelf (Name)
 *  ¦b¼È¦s@Menu¤¤·s¼W¤@­Ó Name ¶µ¥Ø¡A¨ÃÅý¨Ï¥ÎŽÍ¿ï¾Ü«á³sµ²¨ì F_Name ªºFunction
 */
function	script	MenuAddFuncSelf	{
	@Menu_menu$[@Menu_count] = getarg(0);
	@Menu_func$[@Menu_count] = "F_"+getarg(0);
	@Menu_count++;

	return;
}

/*  MenuSearch ()
 *  ±q¼È¦s@Menu·j´M¡A¨Ã¦^¶Ç index¡A­Y¨ú®ø·j´M«h¦^¶Ç-1
 */
function	script	MenuSearch	{
	while (1) {
		mes "¿é¤J­n·j´Mªº¦rµü";
		next;
		input .@search$;
		
		.@c = 0;
		for (.@i = 0; .@i < @Menu_count; .@i++) {
			if (compare(@Menu_menu$[.@i], .@search$)) {
				.@sel_menu$[.@c] = @Menu_menu$[.@i];
				.@sel_list[.@c] = .@i;
				.@c++;
			}
		}
		
		if (!.@c) {
			mes "["+.@search$+"] ·j´M¤£¨ì¬ÛÃöµ²ªG";
			next;
			
			if (select("Ä~Äò·j´M:¨ú®ø·j´M") == 2) {
				next;
				return -1;
			}
			
			continue;
		} else if (.@c >= @Menu_Limit) {
			mes "["+.@search$+"] ¤Ó¹L¼sªx¡A½Ð¿é¤J§óºë·Çªº¦rµü";
			next;
			
			if (select("Ä~Äò·j´M:¨ú®ø·j´M") == 2) {
				next;
				return -1;
			}
			
			continue;
		}
		
		for (.@i = 0; .@i < .@c; .@i++)
			.@menu$ += .@sel_menu$[.@i] + ":";
		
		return .@sel_list[select(.@menu$) - 1];
	}
}

/*  MenuSelect ({Funcname})
 *  ±q¼È¦s¸Ìªº @Menu ¤¤¿ï¾Ü¡A¦^¶Ç index
 *  ¦pªGFunctionName³Q«ü©w¡A·|·s¼W '¤W¤@¼h' ¿ï³æ¨Ãcallfunc FunctionName¡A³Ì«á¦^¨ì­ì¥»ªº¦ì¸m
 */
function	script	MenuSelect	{
	.@p = 0;
	.@max_page = @Menu_count / @Menu_Limit;   
	
	while (1) {
		.@menu$ = "";
		deletearray .@p_menu$;
		deletearray .@p_sel;

		if (.@max_page)
			.@p_menu$[getarraysize(.@p_menu$)] = $@menu_search$;

		if (.@p > 0)
			.@p_menu$[getarraysize(.@p_menu$)] = $@pre_page$;

		for (.@i = 0; .@i + .@p * @Menu_Limit < @Menu_count && .@i < @Menu_Limit; .@i++) {
			.@n = getarraysize(.@p_menu$);
			.@p_menu$[.@n] = @Menu_menu$[.@i+.@p*@Menu_Limit];
			.@p_sel[.@n] = .@i;
		}
		
		if (.@p < .@max_page)
			.@p_menu$[getarraysize(.@p_menu$)] = $@next_page$;

		if (getargcount() > 0)
			.@p_menu$[getarraysize(.@p_menu$)] = $@pre_level$;
		.@p_menu$[getarraysize(.@p_menu$)] = "µ²§ô";
			
		
		for (.@i = 0; .@i < getarraysize(.@p_menu$); .@i++)
			.@menu$ += .@p_menu$[.@i]+":";
		
		.@sel = select(.@menu$) - 1;
		
		if (getargcount() > 0 && .@p_menu$[.@sel] == $@pre_level$) {
			//callfunc getarg(0);
			return -1;
		}
		
		if (.@p_menu$[.@sel] == $@pre_page$) {
			.@p--;
			continue;
		} else if (.@p_menu$[.@sel] == $@next_page$) {
			.@p++;
			continue;
		} else if (.@p_menu$[.@sel] == $@menu_search$) {
			.@ret = MenuSearch();
			
			if (.@ret < 0)
				continue;
			else
				return .@ret;
		} else if (.@p_menu$[.@sel] == "µ²§ô") {
			cutin "",255;
			close;
		}
		
		.@sel = .@p_sel[.@sel];
		.@sel += .@p * @Menu_Limit;		
		break;
	}
	
	if (@Menu_func$[.@sel] != "")
		callfunc @Menu_func$[.@sel];
	
	if (@Menu_Group[.@sel]) {
		next;
		copyarray .@MenuType$, getd("@Menu_type_"+.@sel+"$"), getarraysize(getd("@Menu_type_"+.@sel+"$"));
		copyarray .@MenuName$, getd("@Menu_name_"+.@sel+"$"), getarraysize(getd("@Menu_name_"+.@sel+"$"));
		MenuInit;
		for (.@i = 0; .@i < getarraysize(getd("@Menu_type_"+.@sel+"$")); .@i++)
			callfunc getd2("@Menu_type_"+.@sel+"$", .@i), getd2("@Menu_name_"+.@sel+"$", .@i);
		callfunc "MenuSelect";
	}
	
	return .@sel;
}	

/*  MenuSelectString ({Funcname})
 *  ±q¼È¦s¸Ìªº @Menu ¤¤¿ï¾Ü¡A¦^¶Ç @Menu_menu$
 *  ¦pªGFunctionName³Q«ü©w¡A·|·s¼W '¤W¤@¼h' ¿ï³æ¨Ãcallfunc FunctionName¡A³Ì«á¦^¨ì­ì¥»ªº¦ì¸m
 */
function	script	MenuSelectString	{
	if (getargcount() > 0)
		.@sel = MenuSelect(getarg(0));
	else
		.@sel = MenuSelect();
	
	if (.@sel < 0)
		return "-1";
	else
		return @Menu_menu$[.@sel];
}	

/*  MenuContinue ([msg])
 *  Â²³æ½T»{¸ß°Ý¡A­Y¦³msg¡Aªþ¥[msg¨ì½T»{¤§«á
 */
function	script	MenuContinue	{
	if (getargcount() > 0)
		.@postfix$ = getarg(0);
	mes "­nÄ~Äò¶i¦æ¶Ü¡H";
	next;

	if (select("½T»{"+.@postfix$+":¨ú®ø") == 2) {
		cutin "", 255;
		close;
	}
	
	return;
}

/*  MenuYesOrNo ()
 *  Â²³æ½T»{¸ß°Ý
 */
function	script	MenuYesOrNo	{
	next;

	if (select("½T»{:¨ú®ø") == 2) {
		cutin "", 255;
		close;
	}
	
	return;
}

/*  MenuSelectNumbers (min, max, {Prefix}, {Postfix})
 *  «Ø¥ß±q min ¨ì max ªº²M³æ¡AÅý¨Ï¥ÎªÌ¿ï¾Ü
 *  ¨Ã¦^¶Ç©Ò¿ïªº­È
 */
function	script	MenuSelectNumbers	{
	.@min = getarg(0);
	.@max = max(getarg(1), .@min);
	
	if (getargcount() > 2)
		.@prefix$ = getarg(2);
	
	if (getargcount() > 3)
		.@postfix$ = getarg(3);
	
	MenuInit;
	for (.@i = .@min; .@i <= .@max; .@i++) {
		MenuAdd .@prefix$+.@i+.@postfix$;
		.@valuelist[.@c++] = .@i;
	}
	
	return .@valuelist[MenuSelect()];
}

/*  ArraySelect (Array, {Funcname})
 *  ±qArray²M³æ¤¤«Ø¥ß Menu¡A¨ÃÅý¨Ï¥ÎŽÍ¿ï¾Ü¤§¡A³Ì«á¦^¶Ç¿ï¾Üªº¶µ¥Ø(index)
 *  ¦pªGFunctionName³Q«ü©w¡AArrayªº¶µ¥Ø·|¥ý¶Ç¶iFunctionName¤§«á¦b¶i¤JMenuAdd
 */
function	script	ArraySelect	{ 
	MenuInit;
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++) {
		if (getargcount()>1)
			MenuAdd callfunc(getarg(1),getelementofarray(getarg(0), .@i));
		else
			MenuAdd getelementofarray(getarg(0), .@i);
	}
	
	return MenuSelect();
}

/*  ArrayFind (Array, target)
 *  ±qArray´M§ä²Å¦Xtargetªº¶µ¥Ø¨Ã¦^¶Çindex
 */
function	script	ArrayFind	{ // Array, target
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++)
	if (getelementofarray(getarg(0), .@i) == getarg(1))
		break;
	return .@i;
}

/*  ArrayExist (Array, target)
 *  ±qArray´M§ä²Å¦Xtargetªº¶µ¥Ø¡A­Y¦s¦b¦^¶Ç1¡A§_«h¦^¶Ç0
 */
function	script	ArrayExist	{ // Array, target
	if (ArrayFind(getarg(0), getarg(1)) < getarraysize(getarg(0)))
		return 1;
	
	return 0;
}

/*  ArrayAdd (Array, target)
 *  §âtarget¥[¤J¶iArray
 */
function	script	ArrayAdd	{ // Array, target
	.@count = getarraysize(getarg(0));
	copyarray .@temp, getarg(0), .@count;
	.@temp[.@count] = getarg(1);
	copyarray getarg(0), .@temp, getarraysize(.@temp);
	return;
}

/*  ArrayAddString (Array, target)
 *  ArrayAdd ¦r¦êª©
 */
function	script	ArrayAddString	{ // Array, target
	.@count = getarraysize(getarg(0));
	copyarray .@temp$, getarg(0), .@count;
	.@temp$[.@count] = getarg(1);
	copyarray getarg(0), .@temp$, getarraysize(.@temp$);
	return;
}

/*  ArrayAddUnique (Array, target)
 *  ­Ytarget¤£¦s¦b©óArray¤¤¡A«h§âtarget¥[¤J¶iArray
 */
function	script	ArrayAddUnique	{ // Array, target
	if (ArrayExist(getarg(0), getarg(1)))
		return;
	
	ArrayAdd(getarg(0), getarg(1));
	return;
}

/*  ArrayAddUniqueString (Array, target)
 *  ArrayAddUnique ¦r¦êª©
 */
function	script	ArrayAddUniqueString	{ // Array, target
	if (ArrayFind(getarg(0), getarg(1)) < getarraysize(getarg(0)))
		return;
	
	ArrayAddString(getarg(0), getarg(1));
	return;
}

/*  ArrayRemove (Array, target)
 *  ­Ytarget¦s¦b©óArray¤¤¡A«h§âtarget±qArray²¾°£
 */
function	script	ArrayRemove	{ // Array, target
	while(1) {
		if ((.@i= ArrayFind(getarg(0), getarg(1))) < getarraysize(getarg(0))) {
			deletearray getelementofarray(getarg(0), .@i), 1;
			continue;
		}
		
		return;
	}
}

/*  ArrayCount (Array, target)
 *  ±qArray´M§ä²Å¦Xtargetªº¼Æ¶q¨Ã¦^¶ÇÁ`¼Æ
 */
function	script	ArrayCount	{ // Array, target
	.@c = 0;
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++)
	if (getelementofarray(getarg(0), .@i) == getarg(1))
		.@c++;
	return .@c;
}

/*  ArrayForeach (Array, Func)
 *  §âArrayªº¶µ¥Ø¨Ì§Çrun Func
 */
function	script	ArrayForeach	{ // Array, Func
	.@argcount = getargcount();
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++) {
		switch(.@argcount) {
		case 2: callfunc(getarg(1), getelementofarray(getarg(0), .@i)); break;
		case 3: callfunc(getarg(1), getelementofarray(getarg(0), .@i), getarg(2)); break;
		case 4: callfunc(getarg(1), getelementofarray(getarg(0), .@i), getarg(2), getarg(3)); break;
		case 5: callfunc(getarg(1), getelementofarray(getarg(0), .@i), getarg(2), getarg(3), getarg(4)); break;
		case 6: callfunc(getarg(1), getelementofarray(getarg(0), .@i), getarg(2), getarg(3), getarg(4), getarg(5)); break;
		}
	}
	return .@c;
}

/*  ArraySelectBin (Array)
 *  ±qArray¤¤¿ï¾Ü¤Ä¿ï¶µ¥Ø¨Ã¦^¶Ç¥X¨äBinÁ`©M
 */
 
function	script	ArraySelectBin	{
	.@bin = 0;
	
	while(1) {
		MenuInit;
		MenuAdd "³]©w§¹²¦";
		for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++)
			MenuAdd ((.@bin & (pow(2, .@i)))?"[¡´] ":"[¡³] ")+getelementofarray(getarg(0), .@i);
		
		.@sel = MenuSelect() - 1;
		
		if (.@sel < 0)
			break;
		
		.@sel_bin = pow(2, .@sel);
		if ((.@bin & .@sel_bin) == .@sel_bin)
			.@bin -= .@sel_bin;
		else
			.@bin |= .@sel_bin;
	}
	
	return .@bin;
}

/*  ArrayShowBinMes (Array, Bin, {Prefix})
 *  Mes¥XArray¤¤»PBin¹ïÀ³ªº¶µ¥Ø
 */
 
function	script	ArrayShowBinMes	{
	.@bin = getarg(1);
	
	if (getargcount()>2)
		.@prefix$ = getarg(2);
	
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++)
	if (.@bin & (pow(2, .@i)))
		mes .@prefix$+getelementofarray(getarg(0), .@i);

	return;
}

/*  ArrayBinForeach (Array, Bin, func)
 *  Array¤¤»PBin¹ïÀ³ªº¶µ¥Ø¡A±N¨ä­È¶Çµ¹func
 */
 
function	script	ArrayBinForeach	{
	.@bin = getarg(1);
	.@func$ = getarg(2);
	
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++)
	if (.@bin & (pow(2, .@i)))
		callfunc(.@func$, getelementofarray(getarg(0), .@i));

	return;
}

function	script	F_ServerAnnounce	{ // data
	announce getarg(0), bc_all;
	return;
}

function	script	F_Dispbottom	{ // data
	dispbottom getarg(0);
	return;
}

function	script	F_ServerDebugMes	{ // data
	debugmes getarg(0);
	return;
}

/*  ArrayMessage (Array)
 *  §âArrayªº¶µ¥Ø¨Ì§Ç³z¹L mes °T®§±À°e¥X¥h
 */
function	script	ArrayMessage	{ // Array
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++)
		mes getelementofarray(getarg(0), .@i);
	return;
}

function	script	ArrayDispbottom	{ // Array
	for(.@i = 0; .@i < getarraysize(getarg(0)); .@i++) {
		if (getargcount() > 1)
			dispbottom getelementofarray(getarg(0), .@i), getarg(1);
		else
			dispbottom getelementofarray(getarg(0), .@i);
	}
	return;
}

function	script	ArrayRandomPickSingle	{ // Source_Array
	return getelementofarray(getarg(0), rand(getarraysize(getarg(0))));
}

function	script	ArrayRandomPickMultiple	{ // Source_Array, TargetArray, size
	freeloop(1);
	
	if (getarraysize(getarg(0)) < getarg(2))
		return;
	
	for(.@i = 0; .@i < getarraysize(getarg(0));) {
		.@n = ArrayRandomPickSingle(getarg(0));
		
		if (ArrayFind(.@pick_array, .@n) == getarraysize(.@pick_array)) {
			.@pick_array[getarraysize(.@pick_array)] = .@n;
			.@i++;
		}
	}
	
	copyarray getarg(1), .@pick_array, getarg(2);
	return;
}

/*  ItemID2Name (ItemID)
 *  ¦^¶Ç ItemID ªº item name
 */
function	script	ItemID2Name	{
	return getitemname(getarg(0));
}

function	script	MobID2Name	{
	return strmobinfo(2,getarg(0));
}

function	script	MobID2NameMVP	{
	return ((strmobinfo(9,getarg(0))==2)?"[MVP]":"")+strmobinfo(2,getarg(0));
}

function	script	ItemIsSpecial	{
	.@n = getarg(0);
	return (.@n == 255 || .@n == 254 || .@n == -256);
}

function	script	ShowDatetime	{
	.@datetime$ = gettimestr("%Y-%m-%d %H:%M:%S", 21, getarg(0));
	return .@datetime$;
}

function	script	DispBigAmount	{
	function	Sub	{
		if (getarg(0) < 1)
			return "000";
		if (getarg(0) < 10)
			return "00"+getarg(0);
		if (getarg(0) < 100)
			return "0"+getarg(0);
		else
			return getarg(0);
	}

	.@neg = 0;
	.@amount = getarg(0);
	if (.@amount < 0) {
		.@neg = 1;
		.@amount *= -1;
	}
	.@msg$ = "";
	
	if (!.@amount)
		return 0;
	
	while(.@amount/1000) {
		if (.@level)
			.@msg$ = ","+.@msg$;
		.@msg$ = ""+Sub(.@amount % 1000)+""+.@msg$;
		.@amount /= 1000;
		.@level++;
	}
	
	if (.@level)
		.@msg$ = ","+.@msg$;
	.@msg$ = ""+.@amount+""+.@msg$;
	
	return ((.@neg)?"-":"")+.@msg$;
}

function	script	DispBigAmount2	{
	.@amount = getarg(0);
	
	if (!.@amount)
		return 0;
	
	
	.@neg = 0;
	if (.@amount < 0)
		.@neg = 1;
	
	if (.@amount/100000000 > 0)
		.@msg$ += (.@amount/100000000)+"»õ";
	if ((.@amount%100000000)/10000 > 0)
		.@msg$ += ((.@amount%100000000)/10000)+"¸U";
	if (.@amount%10000 > 0)
		.@msg$ += ""+(.@amount%10000)+"";
	
	return ((.@neg)?"-":"")+.@msg$;
}

function	script	SelectInventory	{ // (.@retVal, .@amount, .@npc_name$, .@func$, .@no_pre, {.@offset})
	function	check	{
		for (.@i = 0; .@i < getarraysize(getarg(0)); .@i++)
		if (getelementofarray(getarg(0), .@i) == getarg(1)+1)
			return .@i;
		
		return -1;
	}
	// .@target_retVal = getarg(0);
	.@amount = getarg(1);
	.@npc_name$ = getarg(2);
	.@func$ = getarg(3);
	
	freeloop(1);
	disable_items;
	
	getinventorylist();
	deletearray .@retVal;
	
	if (getargcount() > 4) {
		.@no_pre = getarg(4);
		
		if (getargcount() > 5)
			.@offset = 3;
	}

	// »s§@ª««~²M³æ
	for(.@i = .@c = 0; .@i < @inventorylist_count; .@i++) {
		if (callfunc(.@func$, SI_EXAM+.@offset, .@i)) {
			.@list[.@c++] = .@i;
			
			if (!.@no_pre)
			if (getarraysize(.@retVal) < .@amount)
				.@retVal[getarraysize(.@retVal)] = .@i + 1;
		}
	}
	
	if (.@c == 0) {
		mes .@npc_name$;
		mes "§ä¤£¨ì²Å¦X¥H¤U±ø¥óªºª««~¡G";
		callfunc .@func$, SI_RULE+.@offset;
		close;
	}
	
	// ¿ï¾Ü
	while(1) {
		mes .@npc_name$;
		if (.@amount >=0)
			mes "½Ð¿ï¾Ü "+.@amount+" ¶µª««~";
		else
			mes "½Ð¿ï¾Üª««~";
		mes "»Ý²Å¦X¥H¤U±ø¥ó¡G";
		callfunc .@func$, SI_RULE+.@offset;
		next;
		
		MenuInit;
		if (.@amount < 0 || getarraysize(.@retVal) == .@amount)
			MenuAdd "¿ï¾Ü§¹²¦¡A¤w¸g¿ï¾Ü¤F "+getarraysize(.@retVal)+" ¶µ";
		else
			MenuAdd "¤w¸g¿ï¾Ü¤F "+getarraysize(.@retVal)+" ¶µ";
		
		for (.@i = 0; .@i < .@c; .@i++)
			MenuAdd (check(.@retVal, .@list[.@i])>-1?"^0000FF[¡´]^000000":"[¡³]")+callfunc(.@func$, SI_DISP+.@offset, .@list[.@i]);
		
		.@sel = MenuSelect()-1;
		
		if (.@sel < 0) {
			if (.@amount < 0 || getarraysize(.@retVal) == .@amount) {
				copyarray getarg(0), .@retVal, getarraysize(.@retVal);
				return .@retVal[0]-1;
			} else {
				mes .@npc_name$;
				mes "§A¿ï¾Ü¤£¨¬ "+.@amount+" ¥ó";
				mes "»Ý²Å¦X¥H¤U±ø¥ó¡G";
				callfunc .@func$, SI_RULE+.@offset;
				close;
			}
		}
		
		if ((.@r = check(.@retVal, .@list[.@sel])) > -1) {
			deletearray .@retVal[.@r],1;
			continue;
		}
		
		if (getarraysize(.@retVal) >= .@amount && .@amount >= 0) {
			mes .@npc_name$;
			mes "¤w¸g¶W¹L­n¿ï¾Üªº¼Æ¶q»Ý¨D";
			mes "½Ð¥ý¨ú®ø¿ï¾Ü¨ä¥L¸Ë³Æ¡C";
			next;
			continue;
		}
		
		.@retVal[getarraysize(.@retVal)] = .@list[.@sel] + 1;
		
		if (.@amount == 1) {
			copyarray getarg(0), .@retVal, getarraysize(.@retVal);
			return .@retVal[0]-1;
		}
	}
}

/*  MenuInputNumber (min_number, max_number)
 */
function	script	MenuInputNumber	{
	.@min_number = getarg(0);
	.@max_number = getarg(1);
	
	if (.@max_number < .@min_number) {
		mes "¨t²Î¥X¿ù¡A½Ð¦^³øºÞ²z­û";
		close;
	}
	
	while (1) {
		input .@number;
		
		if (.@number > .@max_number || .@number < .@min_number) {
			mes "½Ð¿é¤J "+.@min_number+" ~ "+.@max_number+" ½d³ò¤ºªº¼Æ¦r";
			cutin "", 255;
			next;
			continue;
		}
		
		break;
	}
	
	return .@number;
}

function	script	getarmorenhanceid	{ // (enhance_kind, lv, stack)
	.@kind = getarg(0);
	
	if (getarg(1) <= 0)
		return 0;
	
	if (.@kind >= ENCHANT_MAX_KIND)
		return 0;
	
	if (.@kind <= ENCHANT_PATIENT)
		return 50000 + getarg(0) * 1000 + (getarg(1)-1) * 10 + getarg(2);
	
	.@kind -= ENCHANT_FIGHT;
	return 50000 + .@kind * 1000 + 400 + (getarg(1)-1) * 10 + getarg(2);
}

function	script	Talk	{
	mes "["+strnpcinfo(1)+"]";
	for (.@i = 0; .@i < getargcount(); .@i++)
		mes getarg(.@i);
	return;
}

function	script	ShowTime	{
	if (getarg(0) > 3600) {
		.@h = getarg(0) / 3600;
		.@rest = getarg(0) - .@h * 3600;
		return .@h + " ®É " + (.@rest / 60) + " ¤À " + (.@rest % 60) + " ¬í";
	} else if (getarg(0) > 60)
		return (getarg(0) / 60) + " ¤À " + (getarg(0) % 60) + " ¬í";
	else
		return getarg(0) + " ¬í";
}

function	script	CalcTime	{
	.@hour = getarg(0);
	.@minute = getarg(1);
	
	if (getargcount() <= 2) {
		return .@hour * 60 + .@minute;
	} else {
		.@second = getarg(2);
		return .@hour * 60 * 60 + .@minute * 60 + .@second;
	}
}

/*  SelectEquip ( [Array] )
 *  ±q $@NormalRefineEQI_List ²M³æ¤¤¿ï¾Ü¨­¤Wªº¸Ë³Æ
 *  ­Y¦³«ü©w Array «h±q Array ¤¤¿ï¾Ü
 */

function	script	SelectEquip	{
	copyarray .@eqi_list, $@NormalRefineEQI_List, getarraysize($@NormalRefineEQI_List);
	MenuInit;
	.@c = 0;
	for(.@i = 0; .@i < getarraysize(.@eqi_list); .@i++) {
		.@n = .@eqi_list[.@i];
		.@position$ = $@EQI_NAME$[.@n];
		MenuAdd .@position$+" "+((getequipisequiped(.@n))?("+"+getequiprefinerycnt(.@n)+" " + getequipname(.@n)):"- [¨S¦³¸Ë³Æ]");
		.@c++;
	}
	
	if (getargcount() > 0) {
		if (getarg(0) == 1) {
			MenuAdd "^FF0000ºë·ÒNPC¬G»Ù±Æ°£^000000";
		}
	}
	.@sel = MenuSelect();
	
	if (.@sel == .@c) {
		close2;
		atcommand "@unloadnpc GP_Refiner";
		atcommand "@loadnpc npc/icero/GamePoints/Refine.c";
		end;
	}
	
	return .@eqi_list[.@sel];
}

function	script	DispPointArray	{
	for (.@i = 1; .@i <= 20; .@i++)
		.@gap_name$[.@i] = "»õ("+.@i+")";

	copyarray .@points, getarg(0), getarraysize(getarg(0));
	for (.@i = getarraysize(.@gap_name$) - 1; .@i >= 0; .@i--) {
		if (.@points[.@i]) {
			.@gap = .@i+1;
			break;
		}
	}
	
	.@i = 0;
	.@lv = .@gap - (1 + .@i);
	
	if (.@lv < 0)
		return "0";
	
	.@disp_lv = .@gap - (1 + .@i);
	.@msg$ += DispBigAmount2(.@points[.@lv]);
	.@i++;
	
	if (.@i < .@gap) {
		.@lv = .@gap - (1 + .@i);
		if (.@points[.@lv] >= 10000000)
			.@msg$ += "."+(.@points[.@lv]/1000000)+"";
		else if (.@points[.@lv] >= 1000000)
			.@msg$ += ".0"+(.@points[.@lv]/1000000)+"";
	}
	
	.@msg$ += .@gap_name$[.@disp_lv];
	
	return .@msg$;
}

function	script	DispNumbers	{
	for (.@i = 1; .@i <= 20; .@i++)
		.@gap_name$[.@i] = "»õ("+.@i+")";

	copyarray .@points, getarg(0), getarraysize(getarg(0));
	for (.@i = getarraysize(.@gap_name$) - 1; .@i >= 0; .@i--) {
		if (.@points[.@i]) {
			.@gap = .@i+1;
			break;
		}
	}
	
	.@i = 0;
	.@lv = .@gap - (1 + .@i);
	
	if (.@lv < 0)
		return "0";
	
	.@disp_lv = .@gap - (1 + .@i);
	.@msg$ += DispBigAmount2(.@points[.@lv]);
	.@i++;
	
	if (.@i < .@gap) {
		.@lv = .@gap - (1 + .@i);
		if (.@points[.@lv] >= 10000000)
			.@msg$ += "."+(.@points[.@lv]/1000000)+"";
		else if (.@points[.@lv] >= 1000000)
			.@msg$ += ".0"+(.@points[.@lv]/1000000)+"";
	}
	
	.@msg$ += .@gap_name$[.@disp_lv];
	
	return .@msg$;
}

function	script	DispPointArrayDetail	{
	for (.@i = 1; .@i <= 20; .@i++)
		.@gap_name$[.@i] = "»õ("+.@i+")";

	copyarray .@points, getarg(0), getarraysize(getarg(0));
	for (.@i = getarraysize(.@gap_name$) - 1; .@i >= 0; .@i--) {
		if (.@points[.@i]) {
			.@gap = .@i+1;
			break;
		}
	}
	
	.@i = 0;
	.@lv = .@gap - (1 + .@i);
	
	if (.@lv < 0)
		return "0";
	
	.@disp_lv = .@gap - (1 + .@i);
	.@msg$ += DispBigAmount2(.@points[.@lv]);
	.@i++;
	
	if (.@i < .@gap) {
		.@lv = .@gap - (1 + .@i);
		if (.@points[.@lv] >= 10000000)
			.@msg$ += "."+(.@points[.@lv]/1000000)+"";
		else if (.@points[.@lv] >= 1000000)
			.@msg$ += ".0"+(.@points[.@lv]/1000000)+"";
	}
	
	.@msg$ += .@gap_name$[.@disp_lv];
	
	return .@msg$;
}
