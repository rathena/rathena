#!/usr/bin/perl

##########################################################################
# Athena用データバックアップツール
#
# 　Athenaの各種データファイル*.txtをバックアップするツール
#
#-------------------------------------------------------------------------
# 設定方法
# 　実行する時のカレントフォルダからのデータへのパス、ファイルのリストを
# 　正しく設定します。バックアップ先のフォルダは自動作成されないので、
# 　自分で作成しておく必要があります。
# 　フォルダの最後の「/」は省略できません。
#
# 　フォルダは引数でも指定できます。例＞./backup ../save/ ./backup_data/
# 　フォルダの最後の「/」は省略できません。
#
# 　実行するとバックアップ先のフォルダへ、ファイル名に現在の日付と時刻を
# 　つけてファイルをコピーします。
#
#   * toolフォルダ内にbackup_dataフォルダを作成し、
# 　  athena.shの中に「./tool/backup ./save/ ./tool/backup_data/」
#     という行を追加すると、athenaを起動するたびにバックアップが取れます
#
#   復元するときは引数に「-r 日付と時刻」を指定します。
# 　またその後ろにフォルダを指定することも出来ます
# 　例１＞ ./backup -r 200309191607
# 　例２＞ ./backup -r 200309191607 ../save ./backup_data/
# 　この例では2003/09/19の16:07分にバックアップしたデータを復元しています
#
# 　復元するとき、Athenaディレクトリにあるデータは *.bak に名前を変更して
# 　残しているので、いらない場合は rm *.bak などで消してください。
#
##########################################################################

$sdir="../save/";		#バックアップ元(Athenaのディレクトリ/save/)
$tdir="./backup_data/";		#バックアップ先

@files=(			#ファイルのリスト
	"account","athena","storage","party","guild","castle","pet"
);


#-------------------------------設定ここまで-----------------------------











if($ARGV[0]=~/^\-r$/i || $ARGV[0]=~/\-\-(recover|restore)/i){
	#復元処理

	$file=$ARGV[1];
	$sdir=$ARGV[2]||$sdir;
	$tdir=$ARGV[3]||$tdir;
	&restorecopy($_) foreach @files;
	exit(0);
}

#バックアップ処理
$sdir=$ARGV[0]||$sdir;
$tdir=$ARGV[1]||$tdir;

unless( -d $tdir ){
	print "$0: \"$tdir\" : No such directory\n";
	exit(1);
}

(undef,$min,$hour,$day,$month,$year)=localtime;

$file=sprintf("%04d%02d%02d%02d%02d",
	$year+1900, $month+1, $day, $hour, $min );

&backupcopy($_) foreach @files;
exit(0);

sub backupcopy {
	my($name)= @_;
	system("cp $sdir$name.txt $tdir$name$file.txt");
}

sub restorecopy {
	my($name)= @_;
	unless( -f "$sdir$name.txt" ){
		printf("$0: \"$sdir$name.txt\" not found!\n");
		return 0;
	}
	unless( -f "$tdir$name$file.txt" ){
		printf("$0: \"$tdir$name$file.txt\" not found!\n");
		return 0;
	}
	rename "$sdir$name.txt","$sdir$name.bak";
	system("cp $tdir$name$file.txt $sdir$name.txt");
}
