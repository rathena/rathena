// Encoded UTF-8
//---------------------------------------------
// 007 - by Jazz
1. 0590 official versionに合わせて修正しました.

//---------------------------------------------
// 006 - by Jazz
1. パスワード決まった回数間違い時自動 IP遮断具現.
2. login_athena.confを修正

//---------------------------------------------
// 005 - by Jazz
1. 許容しない IDが接近の時自動で IP遮断機能具現.
2. 自動で遮断解除機能具現.

//---------------------------------------------
// 004 - by Jazz
1. aphostropy 問題を解決しました. これは保安と連関がある問題です.
2. SQLの構造を修正しました.
3. server 状態を SQLで出力.

//---------------------------------------------
// 003 - by Jazz
1. 接続記録を DBで出力.
2. IP基盤の接続遮断具現.

//---------------------------------------------
// 002 - by Jazz
1. いくつかの詳細問題点修正.

//---------------------------------------------
// 001 - by Jazz
1. 一番目安定 versionです.


// notice some..
In this program, new parts are under BSD License.
and imported parts are under GPL as athena did.

this program is not public license. but modification and editing are
unlimit permitted. but auther don't have any responsibility on that.

this realase does not gurantee working perfectly. and, I would
answer neither that question nor technical one.

I use...
P4 2.4Gh/512MB/WinXP/cygwin

athena is under GPL license.
Ragnarok is Gravity's trademark.
login-db for athena is BSD license except code from original athena.
cygwin is Redhat's trademark.

// About compile.
You need mysqlclient library to compile this. You can get this by
compiling mysql source.

"-lmysqlclient" option needed when you did compile.

some patch need on mysql when you did compile under cygwin.

// 몇가지 정보에 대하여

이 프로그램에서 새로 만들어진 부분은 BSD 라이센스 아래에 배포 됩니다.
그리고 import 된 부분은 원래 athena의 GPL 라이센스 아래에 있습니다.

이 프로그램은 public license가 아닙니다. 하지만 변경, 개조, 수정은
무제한 허용되며 그에 대한 책임은 저자가 지지 않습니다.

현재 이 릴리즈는 완벽히 동작함을 보장하지 않습니다. 그리고 그에 대한
상담 또한 절대 받지 않습니다.

기술적인 부분의 상담은 받지 않습니다.

개발 환경
P4 2.4Gh/512MB/WinXP/cygwin

athena는 GPL 라이센스를 준수 합니다.
Ragnarok는 Gravity의 trademark입니다.
login-db for athena는 athena에서 가져온 부분을 제외하면 BSD 라이센스를 따릅니다.
cygwin은 Redhat의 trademark입니다.

// 컴파일에 대하여.

이 프로그램을 컴파일하기 위하여 mysqlclient에 대한 library가 필요합니다.
이는 mysql의 소스를 컴파일하면 얻을수 있습니다.

컴파일시에 -lmysqlclient 옵션이 필요합니다.

cygwin의 경우는 몇가지 패치가 필요합니다.

// いくつかの情報に対して

このプログラムで新たに作られた部分は BSD ライセンスの下に配布になります.
そして import になった部分は元々 athenaの GPL ライセンスの下にあります.

このプログラムは public licenseではないです. しかし変更, 改造, 修正は
無制限許容されてそれに対する責任は著者が負けないです.

現在このリリースは完壁に動作することを保障しないです. そして彼に大韓
相談も絶対受けないです.

技術的な部分の相談は受けないです.

開発環境
P4 2.4Gh/512MB/WinXP/cygwin

athenaは GPL ライセンスを守ります.
Ragnarokは Gravityの trademarkです.
login-db for athenaは athenaから持って来た部分を除けば BSD ライセンスに付きます.
cygwinは Redhatの trademarkです.

// コンパイルに対して.

このプログラムをコンパイルするために mysqlclientに対する libraryが必要です.
これは mysqlのソースをコンパイルすれば得ることができます.

コンパイルの時に -lmysqlclient オプションが必要です.

cygwinの場合はいくつかのパッチが必要です.