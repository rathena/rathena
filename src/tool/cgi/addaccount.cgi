#!/usr/bin/perl

#=========================================================================
# addaccount.cgi  ver.1.00
#	ladminをラップした、アカウントを作成するCGI。
#	ladmin ver.1.04での動作を確認。
#
# ** 設定方法 **
#
# - 下の$ladmin変数にladminへのパスを設定すること。
# - UNIX系OSで使用する場合はladminと共に改行コードを変換すること、また
#   ファイル先頭行をperlの正しいパスにすること。例> $ which perl
# - サーバープログラムやブラウザによっては $cgiuri にこのファイルへの
#   完全なURIをセットしなければならない場合もある。
# - perlにパスが通っていない場合は $perl をperlへの正しいパスにすること。
# - 他は普通のCGIと同じです。（実行権やcgi-binフォルダなど）
#
# ** その他 **
#   addaccount.cgi をブラウザで開くとサンプルHTML（そのまま使えます）が
#   開きます。また、このcgiはブラウザから送られるAccept-Languageが
#   jaで始まっていればメッセージの一部を日本語に変換します。
#   (IEならインターネットオプションの言語設定で一番上に日本語を置く)
#	それ以外の場合は英語のまま出力します。
#-------------------------------------------------------------------------

my($ladmin)	= "../ladmin";			# ladminのパス(おそらく変更が必要)

my($cgiuri)	= "./addaccount.cgi";	# このファイルのURI
my($perl)	= "perl";				# perlのコマンド名



#--------------------------- 設定ここまで --------------------------------






use strict;
use CGI;

my($cgi)= new CGI;
my(%langconv)=(
	'Athena login-server administration tool.*' => '',
	'logged on.*' => '',
);

# ----- 日本語環境なら変換テーブルをセット -----
if($ENV{'HTTP_ACCEPT_LANGUAGE'}=~/^ja/){
	my(%tmp)=(
		'Account \[(.+)\] is successfully created.*'
			=> 'アカウント "$1" を作成しました.',
		'Account \[(.+)\] creation failed\. same account exists.*'
			=> 'アカウント "$1" は既に存在します.',
		'Illeagal charactor found in UserID.*'
			=> 'IDの中に不正な文字があります.',
		'Illeagal charactor found in Password.*'
			=> 'Passwordの中に不正な文字があります.',
		'input UserID 4-24 bytes.'
			=> 'IDは半角4～24文字で入力してください.',
		'input Password 4-24 bytes.'
			=> 'Passwordは半角4～24文字で入力してください.',
		'Illeagal gender.*'
			=> '性別がおかしいです.',
		'Cant connect to login server.*'
			=> 'ログインサーバーに接続できません.',
		'login error.*'
			=> 'ログインサーバーへの管理者権限ログインに失敗しました',
		"Can't execute ladmin.*"
			=> 'ladminの実行に失敗しました',
		'UserID "(.+)" is already used.*'
			=> 'ID "$1" は既に使用されています.',
		'You can use UserID \"(.+)\".*'
			=> 'ID "$1" は使用可能です.',
		
		'account making'	=>'アカウント作成',
		'\>UserID'			=>'>ＩＤ',
		'\>Password'		=>'>パスワード',
		'\>Gender'			=>'>性別',
		'\>Male'			=>'>男性',
		'\>Female'			=>'>女性',
		'\"Make Account\"'	=>'"アカウント作成"',
		'\"Check UserID\"'	=>'"IDのチェック"',
	);
	map { $langconv{$_}=$tmp{$_}; } keys (%tmp);
}

# ----- 追加 -----
if( $cgi->param("addaccount") ){
	my($userid)= $cgi->param("userid");
	my($passwd)= $cgi->param("passwd");
	my($gender)= lc(substr($cgi->param("gender"),0,1));
	if(length($userid)<4 || length($userid)>24){
		HttpError("input UserID 4-24 bytes.");
	}
	if(length($passwd)<4 || length($passwd)>24){
		HttpError("input Password 4-24 bytes.");
	}
	if($userid=~/[^0-9A-Za-z\@\_\-\']/){
		HttpError("Illeagal charactor found in UserID.");
	}
	if($passwd=~/[\x00-\x1f\x80-\xff\']/){
		HttpError("Illeagal charactor found in Password.");
	}
	if($gender!~/[mf]/){
		HttpError("Gender error.");
	}
	open PIPE,"$perl $ladmin --add $userid $gender $passwd |"
		or HttpError("Can't execute ladmin.");
	my(@msg)=<PIPE>;
	close PIPE;
	HttpMsg(@msg);
}
# ----- 存在チェック -----
elsif( $cgi->param("check") ){
	my($userid)= $cgi->param("userid");
	if(length($userid)<4 || length($userid)>24){
		HttpError("input UserID 4-24 bytes.");
	}
	if($userid=~/[^0-9A-Za-z\@\_\-\']/){
		HttpError("Illeagal charactor found in UserID.");
	}
	open PIPE,"$perl $ladmin --search --regex \\b$userid\\b |"
		or HttpError("Can't execute ladmin.");
	my(@msg)=<PIPE>;
	close PIPE;
	if(scalar(@msg)==6 && (split /[\s\0]+/,substr($msg[4],11,24))[0] eq $userid){
		HttpMsg("NG : UserID \"$userid\" is already used.");
	}elsif(scalar(@msg)==5){
		HttpMsg("OK : You can use UserID \"$userid\"");
	}
	HttpError("ladmin error ?\n---output---\n",@msg);
}

# ----- フォーム -----
else{
	print LangConv( <<"EOM" );
Content-type: text/html\n
<html>
 <head>
  <title>Athena account making cgi</title>
 </head>
 <body>
  <h1>Athena account making cgi</h1>
  <form action="$cgiuri" method="post">
   <table border=2>
    <tr>
     <th>UserID</th>
     <td><input name="userid" size=24 maxlength=24></td>
    </tr>
    <tr>
     <th>Password</th>
     <td><input name="passwd" size=24 maxlength=24 type="password"></td>
    </tr>
    <tr>
     <th>Gender</th>
     <td>
      <input type="radio" name="gender" value="male">Male
      <input type="radio" name="gender" value="female">Female
     </td>
    </tr>
    <tr>
     <td colspan=2>
      <input type="submit" name="addaccount" value="Make Account">
      <input type="submit" name="check" value="Check UserID">
     </td>
    </tr>
   </table>
  </form>
 </body>
</html>
EOM
	exit;
}

sub LangConv {
	my(@lst)= @_;
	my($a,$b,@out)=();
	foreach $a(@lst){
		foreach $b(keys %langconv){
			$a=~s/$b/$langconv{$b}/g;
			my($rep1)=$1;
			$a=~s/\$1/$rep1/g;
		}
		push @out,$a;
	}
	return @out;
}

sub HttpMsg {
	my($msg)=join("", LangConv(@_));
	$msg=~s/\n/<br>\n/g;
	print LangConv("Content-type: text/html\n\n"),$msg;
	exit;
}

sub HttpError {
	my($msg)=join("", LangConv(@_));
	$msg=~s/\n/<br>\n/g;
	print LangConv("Content-type: text/html\n\n"),$msg;
	exit;
}

