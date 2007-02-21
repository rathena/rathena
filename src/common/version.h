// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _VERSION_H_
#define _VERSION_H_

#define ATHENA_MAJOR_VERSION	1	// Major Version
#define ATHENA_MINOR_VERSION	0	// Minor Version
#define ATHENA_REVISION			0	// Revision

#define ATHENA_RELEASE_FLAG		1	// 1=Develop,0=Stable
#define ATHENA_OFFICIAL_FLAG	1	// 1=Mod,0=Official

#define ATHENA_SERVER_NONE	0	// not defined
#define ATHENA_SERVER_LOGIN	1	// login server
#define ATHENA_SERVER_CHAR	2	// char server
#define ATHENA_SERVER_INTER	4	// inter server
#define ATHENA_SERVER_MAP	8	// map server

// ATHENA_MOD_VERSIONはパッチ番号です。
// これは無理に変えなくても気が向いたら変える程度の扱いで。
// （毎回アップロードの度に変更するのも面倒と思われるし、そもそも
// 　この項目を参照する人がいるかどうかで疑問だから。）
// その程度の扱いなので、サーバーに問い合わせる側も、あくまで目安程度の扱いで
// あんまり信用しないこと。
// 鯖snapshotの時や、大きな変更があった場合は設定してほしいです。
// C言語の仕様上、最初に0を付けると8進数になるので間違えないで下さい。
#define ATHENA_MOD_VERSION	1249	// mod version (patch No.)

#endif /* _VERSION_H_ */
