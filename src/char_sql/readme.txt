//Encoded with UTF-8 (UNICODE)
//---------------------------------------------
// V.018 - Aarlex
1. ADD Makefile & GNUmakefile
2. fix guild_leave.
3. fix char select windows HP & SP value error.

// V.017 - Aarlex
1. fix guild member view job update.(For mod-0728)
   inter.c
   int_guild.c

// V.016 - by Aarlex
1. Add e-mail check when you Delete char data.
2. modify storage save func like 014.
2. remove Lan_support func.

// v.014 - by Aarlex
I rewrite save function.
besause myfriend find that Mysql will use more than 40% CPU.
And log file is too big (4 days 22G ..= =+)
(maybe he sets autosave_time less then 1 min.)
but. i still rewrite save func.
char server will delete all of user item(inventory & Cart) data then insert them again before.
so i use two struct to save item data from map & database.
then compare two struct to get different .
AND add some debug message.but message maybe too much XD.


1. ADDED itemdb.c itemdb.h
2. modify mmo_char_tosql().
3. ADDED memitemdata_to_sql().
4. ADDED some debug message in memitemdata_to_sql().
5. modify make.sh 

// v.013 - by Aarlex
1. some SQL sentance fix in old version Mysql .

2. in_guild.c mapif_guild_leaved() 
   unsigned char buf[64] -> unsigned char buf[128]

3. in_pet.c inter_pet_tosql()
   if (sql_res) - > if (mysql_num_rows(sql_res)!=0)

4. in_char.c mmo_char_send006b()

   WFIFOW(fd, offset+(i*106)+42) = char_dat[0].hp     -> WFIFOW(fd,offset+(i*106)+42) = (char_dat[j].hp > 0x7fff)? 0x7fff:char_dat[j].hp;
   WFIFOW(fd, offset+(i*106)+44) = char_dat[0].max_hp -> WFIFOW(fd,offset+(i*106)+44) = (char_dat[j].max_hp > 0x7fff)? 0x7fff:char_dat[j].max_hp;
   WFIFOW(fd, offset+(i*106)+46) = char_dat[0].sp     -> WFIFOW(fd,offset+(i*106)+46) = (char_dat[j].sp > 0x7fff)? 0x7fff:char_dat[j].sp
   WFIFOW(fd, offset+(i*106)+48) = char_dat[0].max_sp -> WFIFOW(fd,offset+(i*106)+48) = (char_dat[j].max_sp > 0x7fff)? 0x7fff:char_dat[j].max_sp;

   in_char.c parse_char()  

   WFIFOW(fd, offset+(i*106)+42) = char_dat[0].hp     -> WFIFOW(fd,offset+(i*106)+42) = (char_dat[j].hp > 0x7fff)? 0x7fff:char_dat[j].hp;
   WFIFOW(fd, offset+(i*106)+44) = char_dat[0].max_hp -> WFIFOW(fd,offset+(i*106)+44) = (char_dat[j].max_hp > 0x7fff)? 0x7fff:char_dat[j].max_hp;
   WFIFOW(fd, offset+(i*106)+46) = char_dat[0].sp     -> WFIFOW(fd,offset+(i*106)+46) = (char_dat[j].sp > 0x7fff)? 0x7fff:char_dat[j].sp
   WFIFOW(fd, offset+(i*106)+48) = char_dat[0].max_sp -> WFIFOW(fd,offset+(i*106)+48) = (char_dat[j].max_sp > 0x7fff)? 0x7fff:char_dat[j].max_sp;

// v.012 - by Jazz
1. 0627 official versionに合わせて修正しました.
2. no more binary files are supplied.

//---------------------------------------------
// v.011 - by Mark
1. Fixed a couple bugs which would cause segfaults under linux :)

//---------------------------------------------
// v.010 - by Jazz
1. added some debug info - for reporting.

//---------------------------------------------
// v.009 - by Jazz
1. code added for debug.
2. some SQL sentance fix.

//---------------------------------------------
// v.009 - by Jazz
1. fix crash bug. - pet db.

//---------------------------------------------
// v.008 - by Jazz
1. DB table fix. - char.fix-from.007.to.008.sql

既存の table 構造には矢が装着解除される場合があります.

item.equipは 'unsigned short' 形式です. 
このために SQL tableを修正しなければなりません.

修正コードは char.fix-from.007.to.008.sql です.
MySQLで一度行ってくれれば適用されます. 既存のデータは安全です.

//---------------------------------------------
// v.007 - by Jazz
1. domain 解釈に対する問題を修正しました.

//---------------------------------------------
// v.006 - by Jazz
1. crash bug fix. - when your pet DB is empty

//---------------------------------------------
// v.005 - by Jazz
1. 0590 official versionに合わせて修正しました.

//---------------------------------------------
// v.004 - by Jazz
1. 0586 official versionに合わせて修正しました.

//---------------------------------------------
// v.003 - by Jazz
1. officialの guild.c と party.c ファイルでまた再修正しました.

//---------------------------------------------
// v.002 - by Jazz
1. aphostropy 問題を解決しました. これは保安と連関がある問題です.
2. SQLの構造を修正しました.

//---------------------------------------------
// v.001 - by Jazz
1. 一番目安定 versionです. alpha versionに問題点を多数修正しました.

//------------------------------------------------------------------------
//For JAPANESE USER
Athena Char-Server for MySQL. 005

一応 guildと partyはよく呼び出しされる部分なので既存の athena char-serverが使っている file 基盤の構造を持って行きます.
これは, 一度にメモリーで皆読んで, もう一度に皆 fileで使う方が部下が少なくかかると思います.
そして char データに比べて, lostになっても問題がもっと少ないと判断してそうしました.

MySQLバージョンの compileは MySQL Clients Libraryが必要です. windows(cygwin) バージョンにコンパイルされた binaryを添付しました.

設置:
まだ text->DBの converterはまともに支援しないです. 内部的にちょっとバグが発見されてバグを修正中です.
できるだけ早く uploadをします.

1. char.sqlを MySQLに dumpします.

2. inter_athena.confに次を追加します. そして自分の DBサーバーの情報に合わせてくれます.
ここで windows(cygwin)の場合には localhostを使ってはいけないです. ipで書いてくれなければなりません.
localhostで使う場合 UNIX domain socketが作動するから連結が不可能になります.

//3306 is default
db_server_port: 3306
//DB ip
db_server_ip: 127.0.0.1
//DB id
db_server_id: ragnarok
//DB pass
db_server_pw: ragnarok
//DB name
db_server_logindb: ragnarok

3. MySQL バージョンは 2個の MySQL connect sessionを持ちます.
一つはキャラクターデータを読んで書くのに使われて, 二番目は inter serverのために連結します.

開発とテスト環境は P4 2.4a/1024MB/WinXP pro(MediaCenter Edition 2K4 KR)/Cygwin GCC です.
本人が韓国人だから韓国語開発者の連絡も歓迎します.
韓国語と日本語の上手な方は連絡をください. 日本語の使用に難しさを感じています.

現在 DarkWeissが login serverに MySQLを支援し始めました.
しかし athenaのそれは DarkWeiss よりもっとよく作動すると思います.

contact : jazz@creper.com

//------------------------------------------------------------------------
//For KOREAN USER
Athena Char-Server for MySQL. 005

일단 guild와 party는 자주 호출되는 부분이라서 기존의 athena char-server가 사용하고 있는 file 기반의 구조를 가지고 갑니다.
이 것은, 한번에 메모리로 모두 읽고, 다시 한번에 모두 file로 쓰는 쪽이 부하가 적게 걸린다고 생각합니다.
그리고 char 데이터에 비해, lost가 되더라도 문제가 더 적다고 판단해서 그렇게 하였습니다.

MySQL버전의 compile은 MySQL Clients Library가 필요합니다. windows(cygwin) 버전으로 컴파일된 binary를 첨부하였습니다.

설치:
아직 text->DB의 converter는 제대로 지원하지 않습니다. 내부적으로 약간 버그가 발견되어 버그를 수정중입니다.
되도록 빨리 upload를 하겠습니다.

1. char.sql을 MySQL에 dump합니다.

2. inter_athena.conf에 다음을 추가 합니다. 그리고 자신의 DB서버의 정보에 맞추어 줍니다.
여기에서 windows(cygwin)의 경우에는 localhost를 쓰면 안됩니다. ip로 적어주어야 합니다.
localhost로 사용하는 경우 UNIX domain socket이 작동 하기 때문에 연결이 불가능해 집니다.

//3306 is default
db_server_port: 3306
//DB ip
db_server_ip: 127.0.0.1
//DB id
db_server_id: ragnarok
//DB pass
db_server_pw: ragnarok
//DB name
db_server_logindb: ragnarok

3. MySQL 버전은 2개의 MySQL connect session을 가집니다.
하나는 캐릭터 데이터를 읽고 쓰는데 사용되며, 두번째는 inter server를 위해서 연결합니다.

개발과 테스트 환경은 P4 2.4a/1024MB/WinXP pro(MediaCenter Edition 2K4 KR)/Cygwin GCC 입니다.
본인이 한국인이기 때문에 한국어 개발자의 연락도 환영합니다.
한국어와 일본어가 능숙한 분은 연락을 주세요. 일본어의 사용에 어려움을 느끼고 있습니다.

현재 DarkWeiss가 login server에 MySQL을 지원하기 시작했습니다.
하지만 athena의 그것은 DarkWeiss 보다 더 잘 작동한다고 생각합니다.

contact : jazz@creper.com

//------------------------------------------------------------------------
//For ENGLISH USER
Athena Char-Server for MySQL. alpha 005

= hehe. My English is poor. and I have no time to write. :)

anyway this version use guild and party data on text file base system.
It accesses many times, so memory dumping is useful for less cpu consume.

MySQL version need MySQL Clients Library to compile. This include Win32-binary compiled by cygwin-gcc.

Install:
not yet text->DB converter. I found some bug on it, so It's under debug progress.

1. dump char.sql to MySQL.

2. add below on inter_athena.conf. and set your own information.
do not use 'localhost' as domain on cygwin. you must set as ip.
if you use localhost on cygwin, cygwin tries to connect MySQL as UNIX domain socket.
but, MySQL does not support UNIX domain socket on windows.

//3306 is default
db_server_port: 3306
//DB ip
db_server_ip: 127.0.0.1
//DB id
db_server_id: ragnarok
//DB pass
db_server_pw: ragnarok
//DB name
db_server_logindb: ragnarok

3. MySQL version has 2 MySQL connect session.
one is for char-server and the other one is for inter-server.

developement enviroment)
 P4 2.4a/1024MB/WinXP pro(MediaCenter Edition 2K4 KR)/Cygwin GCC

I'm korean, so contect if U're Korean developer.
Please contact me, If U can use Korean & Japanese well.

DarkWeiss starts to support MySQL version of login server, but Athena's one works better, I thnik.

contact : jazz@creper.com
