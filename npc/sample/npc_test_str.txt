// 文字列変数のテスト
prontera.gat,164,188,1	script	文字列テスト	112,{
	set @str$, "文字列１";
	mes "文字列変数：" + @str$ ;
	mes "確認：" + @str$ + " ...OK?";
	next;
	mes "比較～eqOK：" + (@str$=="文字列１");
	mes "比較～eqNG：" + (@str$=="文字列");
	mes "比較～neOK：" + (@str$!="00000");
	mes "比較～neNG：" + (@str$!="文字列１");
	mes "比較～gtOK：" + ("aab">"aaa");
	mes "比較～ltNG：" + ("aab"<"aaa");
	next;
	input @str2$;
	mes "入力データは " + @str2$ + " です。";
	close;
}
