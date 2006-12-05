--------------------
//1162 by pizza
・スパイラルピアース・ソウルブレーカー・発勁・ファルコンアサルトについて本鯖準拠に修正

	(db)
		skill_db.txt
			スパイラルピアース・ソウルブレーカーの射程
			ソウルブレーカーが詠唱妨害可
		skill_cast_db.txt
			ソウルブレーカーの詠唱時間
			
	(src/map)
		battle.c	
			発勁・ファルコンアサルトの計算式
			スパイラルピアースがニュマで無効化

--------------------
//1161 by Nameless

・バイオプラントによる召還mobのIDとスキルを本鯖準拠に修正
　※呼び出せる数についてはまだ未実装…

	(db)
		mob_avail.txt
			クライアントによって発生する可能性のあるグラ問題の
			暫定対応
		mob_db.txt
			バイオプラント用mobのステを一部修正
		mob_skill_db.txt
			バイオプラント用mobにスキルを修正

	(src/map)
		skill.c			- case AM_CANNIBALIZE: 修正

--------------------
//1160 by Nameless

・1158のfix
　フェアリーフの非移動化と呼び出されたMOBのHPを下方修正

	(src/map)
		skill.c			- case AM_CANNIBALIZE: 修正

--------------------
//1158 by もっさり
・mob後退実装 　自分が向いてる方とは逆にskilllvの分ぐらい動きます
 離れすぎる呼び戻されないので取り巻き呼び戻しを修正
 IWの斜め位置がおかしかったのを修正（バグ報告スレッド part8 >>110)

	(src/map)
		skill.c			呼び戻し修正、後退追加、IW修正
		skill.h	
		mob.c
		map.h
	(db)
		skill_db.txt
--------------------
//1158 by Nameless

・アルケミのバイオプラントを修正
　各LVにあわせて、マンドラゴラ、ヒドラ、フローラ、フェアリーフ、ジオグラファー
　を呼び出すようにした

	(src/map)
		skill.c			- case AM_CANNIBALIZE: 修正

--------------------
//1157 by eigen

・バードダンサースキルの使用でMAP鯖が落ちる不具合を修正

	(src/map)
		skill.c			- skill_unit_onout() 修正

--------------------
//1156 by eigen

・聖体降福使用時、モンクが人数カウントされていなかった不具合を修正
（thanks to 本鯖相違スレpart3 >>121氏）
・バードダンサースキルの効果が切れなかった不具合を修正

	(src/map)
		skill.c			- skill_unit_onout(), skill_check_condition_char_sub() 修正

--------------------
//1155 by latte
・ディボーションに詠唱時間付与
・サクリファイス：倍率修正とボスに有効に。
・グランドクロスのエフェクトの修正

	(db)
		skill_cast_db.txt
		skill_db.txt
	(src/map)
		battle.c

--------------------
//1154 by eigen

・バードダンサースキル使用でMAP鯖が落ちる問題を修正
・ゴスペルの実装
・マグナムブレイクの仕様を本鯖に合わせて変更(火属性追加ダメージは未実装です)

	(db)
		skill_cast_db.txt	- ゴスペル, マグナムブレイクに関する修正
		skill_require_db.txt	- マグナムブレイクに関する修正
		skill_unit_db.txt	- ゴスペルに関する修正
	(src/map)
		battle.c		- battle_calc_pet_weapon_attack(), battle_calc_mob_weapon_attack(),
						battle_calc_pc_weapon_attack(), battle_calc_magic_attack() 修正
		clif.c			- clif_parse_UseSkillToId(), clif_parse_UseSkillToId(),
						clif_parse_WalkToXY(), clif_parse_ActionRequest(),
						clif_parse_UseSkillToId(), clif_parse_UseSkillMap() 修正
		map.h			- MAX_STATUSCHANGEの増加
		pc.c			- pc_natural_heal_sub() 修正
		skill.h			- マグナムブレイク, ゴスペルに関する状態異常テーブル追加
		skill.c			- skill_castend_damage_id(), skill_castend_nodamage_id(),
						skill_unit_onout(), skill_unit_onplace_timer(),
						skill_init_unit_layout() 修正
		status.c		- status_change_start(), status_change_end() 修正

--------------------
//1153 by ぽえ

・ヒール、サンクの修正
　(イビルドルイドC装備中にPv,Gv以外だとダメージが出ないように修正)
　(該当PCにヒールを使用した場合SPだけ消費)
・放置されてるversion.hの更新
	(src/map)
		skill.c			- skill_castend_id(),skill_unit_onplace_timer() 修正
	(src/common)
		version.h		- mod version 1153
--------------------
//1152 by p

・鉱石発見修正
	(db)
		item_db.txt			- 古い巻物の getitem 番号を戻し
	(src/map)
		itemdb.c			- 鉱石発見時生成処理の変更
		mob.c				- 鉱石発見処理の変更
--------------------
//1151 by p

・ブラックスミススキル鉱石発見の実装(仮)
	(conf)
		battle_athena.conf	- 鉱石発見率の指定
	(db)
		item_findingore.txt	- 鉱石ドロップ率の指定
		item_db.txt			- 古い巻物の getitem 番号変更
	(src/map)
		itemdb.c			- db/item_findingore.txt の読み込みと発見時生成
		battle.h			- 設定保持用の項目追加
		battle.c			- 設定読み込み処理追加
		mob.c				- 鉱石発見処理追加

--------------------
//1150 by Theia

・ベノムスプラッシャーをjRO仕様に変更
　(完全ではないので補完希望)
・シャープシューティングの計算式を変更
　(今までの計算式だと必中していた)
	(db)
		skill_cast.txt
		skill_require_db.txt
	(src/map)
		skill.c			- ベノムスプラッシャーの発動条件を変更
		battle.c		- ベノムスプラッシャー,シャープシューティングの倍率を変更

--------------------
//1149 by eigen

・一部のダンサーバードスキルの演奏スキル上から出るとMAP鯖が落ちる不具合を修正

	(src/map)
		skill.c			- skill_unit_onout() 修正

--------------------
//1148 by eigen

・ストリップスキルが詠唱中断されないよう変更
・ストリップスキル成功率のスキルレベル比重を5に変更
・バックスタブの射程を武器に関係なく1に変更
・バックスタブ使用時、弓を装備しているならダメージ半減に変更
・アシッドテラーとデモンストレーションが詠唱中断されるよう変更
・アシッドテラー使用後、相手の鎧を破壊することに成功した場合ショックエモを出すよう変更
・メルトダウンで破壊できる箇所を武器と鎧のみに変更
・ダンサーバードの演奏スキル範囲外に出ても効果が20秒持続するよう変更
（ただし私を忘れないでと合奏スキルは除く）
・倉庫の最大収容量を300に変更
（以上thanks to 本鯖相違スレPart3 >>115氏）
・メルトダウン鎧破壊確率を0.7～7%に変更

	(db)
		skill_db.txt		- cast_cancel、rangeの修正
		skill_unit_db.txt	- (1148-fixの取り込み)
	(src/common)
		mmo.h			- MAX_STORAGEを300に
	(src/map)
		battle.c		- battle_calc_pc_weapon_attack() 修正
		skill.c			- skill_additional_effect(), skill_castend_nodamage_id(),
						skill_castend_damage_id(), skill_unit_onout() 修正

--------------------
//1147 by eigen

・インデュア使用後、10秒経たないと再使用できないよう変更
・シーズモードではインデュアを使用するとMDEFが上がるだけに変更
・残影使用後、2秒経たないと阿修羅を使用できないよう変更

	(src/map)
		map.h		- #define MAX_SKILL_ID, unsigned int skillstatictimer[MAX_SKILL_ID] 追加
		clif.c		- clif_parse_UseSkillToId(), clif_damage() 修正
		skill.c		- skill_castend_nodamage_id(), skill_castend_pos2(), skill_use_id() 修正
		pc.c		- pc_setnewpc(), pc_authok() 修正
		status.c	- status_get_dmotion() 修正
		battle.c	- battle_calc_damage() 修正

--------------------
//1146 by eigen

・インデュア使用時Lvに応じてMDEFが上がるように変更
・インデュア使用中7回ダメージを受けると解除するよう変更
・石投げの固定ダメージを50に変更

	(src/map)
		battle.c	- battle_calc_damage(), battle_calc_misc_attack() 修正
		status.c	- status_calc_pc(), status_change_start(), status_change_end() 修正

--------------------
//1145 by End_of_exam

・start のチェック間隔が短すぎたのを修正(start)
・skill_unit_effect() から無限ループに突入して、スタックオーバーフローで落ちる
　可能性があるバグを修正(skill.c)
・ペットの読み込みに失敗した時に落ちるバグを修正(pet.c)
・２重ログインの切断処理が違っていたバグを修正(map.c)

・1142のマグヌスエクソシズムの修正を元に戻す(skill.c)
・メディタティオのSP回復量修正の取り込み(skill.c thanks to ななしさん)

	(/)
		start		- チェック間隔を修正

	(src/map)
		map.c		- map_quit() 修正
		pet.c		- pet_recv_petdata() 修正
		skill.c		- skill_unit_onplace_timer() , skill_unit_effect() 修正
		status.c	- status_calc_pc() 修正

--------------------
//1144 by 聖

・VCでコンパイルしたとき警告が出るのを修正。
・簡易アイテム・モンスター召還コマンド@imを追加。
・@im追加に伴いAEGISで使われている/item,/monsterを実装。
　(AEGISの仕様に則り装備は1個単位・未鑑定で
　ほかのアイテムは30個単位・鑑定済みで出ます。)
・@monsterを召還匹数入力なしで召還できるようにした。
・コマンド入力の際とある条件を満たすと
　バッファオーバーフローが発生するバグを修正。
	(src/map)
		atcommand.h 修正。
		atcommand.c
			atcommand_monster() 修正。
			atcommand_itemmonster() 追加。
		clif.c
			clif_parse_GMkillall() 修正。
			clif_parse_GMsummon() 修正。
			clif_parse_GMitemmonster() 追加。
		status.c
			status_change_start() 修正。
	(db)
		packet_db.txt 修正。
	(conf)
		msg_athena.conf 修正。
		atcommand_athena.conf 修正。

--------------------
//1143 by End_of_exam

・map_quit(), pc_setpos() を色々整理(map.c pc.c)
・モンスターがバシリカを使うと落ちるバグを修正(skill.c)
・ボスモンスターにロキの叫びが効いていたのを修正(mob.c)
・ダンス途中にサーバー内の別のマップに移動した場合、スキルユニットが消えない
　（転送前のマップに残っている）バグを修正。(pc.c)
・1134でサーバー間のワープポータルを使った時に、スキル使用者が乗ったらサーバーが
　落ちるバグを修正(skill.c)
・1134でハエの羽を使ってサーバー間を移動した場合、アイテムが減らないバグを修正(pc.c)

	(src/map)
		map.c		- map_quit() 修正
		mob.c		- mobskill_castend_id() , mobskill_castend_pos(),
					  mobskill_use_id(), mobskill_use_pos() 修正
		pc.c		- pc_useitem(), pc_setpos() 修正、pc_remove_map() 追加
		pc.h		- pc_remove_map() 追加
		skill.c		- skill_castend_nodamege_id(), skill_unit_onplace() 修正

--------------------
//1142 by づん
・マグヌスエクソシズムで種族にアンデットを持つモンスターに当たらなかったのを修正
	(src/map)
	skill.c		- && race!=1を追加

--------------------
//1141.1 by BDPQ銀 [ 2005/02/21 ]
・1141の添付忘れの追加です。申し訳ありませんでした。
・GMコマンドを行った時のメッセージを追加しました。

	(conf)
		msg_athena.conf	- 113～117		追加	(@reload～ を行った時のメッセージを追加)

--------------------
//1141 by BDPQ銀 [ 2005/02/20 ]
・GMコマンドを追加
	@reloadatcommand	- atcommand_athena.conf を再読込する
	@reloadbattleconf	- battle_athena.conf を再読込する
	@reloadgmaccount	- gm_account_filename (デフォルト GM_account.txt ) を再読込する
	@reloadstatusdb		- job_db1.txt / job_db2.txt / job_db2-2.txt / refine_db.txt / size_fix.txt を再読込する
	@reloadpcdb			- exp.txt / skill_tree.txt / attr_fix.txt を再読込する
・GMコマンド「@reloadmobdb」でペットのデータベースも再読込するように変更
		* @reload～ にはクライアントのリログが必要な場合も有ります。
・GMコマンド「@who+」でレベルも表示するように変更
・ヒールを何レベル以上で9999固定にするかのオプション(heal_counterstop)追加

	(conf)
		atcommand_athena.conf	- reloadatcommand reloadbattleconf reloadgmaccount reloadstatusdb reloadpcdb	追加	(デフォルト99)
		battle_athena.conf		- heal_counterstop		追加	(デフォルト11)
		help.txt				- reloadatcommand reloadbattleconf reloadgmaccount reloadstatusdb reloadpcdb who+ の説明を追加

	(doc)
		conf_ref.txt	- 5. conf/battle_athena.conf	編集	(heal_counterstop の説明とサンプルを追加)
						- 6. atcommand_athena.conf		編集	(説明とサンプルに再読込関連を追加)

	(src/map)
		atcommand.c		- AtCommandInfo atcommand_info	編集	(構造体定義)
						- atcommand_whop()				編集	(プレイヤーのレベルも表示するよう変更)
						- atcommand_reloadatcommand()	追加	(atcommand_athena.conf 再読込)
						- atcommand_reloadbattleconf()	追加	(battle_athena.conf 再読込)
						- atcommand_reloadgmaccount()	追加	(gm_account_filename 再読込)
						- atcommand_reloadstatusdb()	追加	(ステータス関連DB 再読込)
						- atcommand_reloadpcdb()		追加	(プレイヤー関連DB 再読込)
						- atcommand_reloadmobdb()		編集	(ペットのデータベースも読込むよう変更)
		atcommand.h		- AtCommandType					編集	(構造体定義)

		battle.c		- battle_config_read()			編集	(heal_counterstop の追加)
		battle.h		- Battle_Config					編集	(heal_counterstop の追加)

		skill.c			- skill_castend_nodamage_id()	編集	(9999ヒール部を battle_athena.conf を参照するよう変更)
		
		pet.h			- int read_petdb();				追加	(ペット関連DB 再読込用)

		pc.h			- int pc_readdb(void);			追加	(プレイヤー関連DB 再読込用)

		status.h		- int status_readdb(void);		追加	(ステータス関連DB 再読込用)

--------------------
//1140 by eigen
・一部の環境でathena-startとstartが正常に動作していなかったバグを修正

	athena-start		- 改行コードを0Aに統一
	start			- 改行コードを0Aに統一

--------------------
//1139 by もっさり
・NPC取り巻き呼び戻しスキル実装
・コメントされてる9999ヒール(skilllv>10の時)、広範囲メテオ(skilllv>10の時)、広範囲ハンマーフォール(skilllv>5の時)のコメント取り外し。
・広範囲lovの付け加え(skilllv>10の時)
例
1312,取り巻き呼び戻し＠タートルジェネラル,attack,354,1,3000,0,0,no,self,always,0,,,,,,10
1063,9999ヒール＠ルナティック,idle,28,11,10,2000,60000,yes,self,always,0,,,,,,　
	
	(src/map)
		skill.c	  npc_recallスキル追加,上記のコメント取り外し
		skill.h   NPC_RECALL = 354を追加
		mob.c     スキル追加のために「取り巻きモンスターの処理」部分に付け加え
		mob.h     int mob_countslave(struct mob_data *md);を追加
		map.h     struct mob_dataにrecall_flagとrecallmob_countメンバー追加
	(db)
		skill_db.txt	スキル追加

--------------------
//1138 by End_of_exam

・1132のsocket.cに紛れ込んでいたかなり深刻なバグ（送信データがランダムに
　書き換わる可能性があるバグ）を修正(socket.c)
・1134で組み込んだアイテムdupe対策が不完全だったのを修正(pc.c party.c guild.c)

	(src/common/)
		socket.c		- send_from_fifo() 修正

	(src/map)
		pc.c			- pc_setpos() 修正
		party.c			- 色々修正
		guild.c			- 色々修正

--------------------
//1137 by いど

・サーバースナップショット

--------------------
//1136 by by eigen

・1135で消えていたbattle_athena.confの項目とデフォルト値を復活
・conf_ref.txtにnext_exp_limitの説明を追加

	(conf)
		battle_athena.conf	- 消えた項目とデフォルト値を復活
	(doc)
		conf_ref.txt		- next_exp_limitの説明を追加

--------------------
//1135 by by Toshi^2
・パッチ1125で修正された、経験値の上限設定を従来方式の制限無しも選べるように変更

	(db)
		battle_athena.conf	- next_exp_limitを追加。
	(src/map)
		battle.c	- battle_config_read() 修正
		battle.h	- struct Battle_Config{}に int next_exp_limit; を追加。
		pc.c		- pc_gainexp() 修正

--------------------
//1134 by End_of_exam

・1132で#undef closeを忘れていたバグを修正(socket.c)
・1133のアイテムdupe対策が不完全だったのを修正(map.c)
・athena-start stop , kill の順番をmap -> char -> login に変更
　　　　(athena-start thanks to eigenさん)

	(/)
		athena-start	- athena-start stop , kill の順番修正

	(src/common)
		socket.c		- #undef close 追加

	(src/map)
		map.c			- map_quit() 修正

--------------------
//1133 by End_of_exam

・mapflag nosave が指定されたマップで死んでリスタートする時に、セーブポイントが
　別マップサーバーにあると、(nul,0,0)に飛ばされていたバグを修正(pc.c)
・マップサーバーを分配している時に、細工をした特殊なツールを使うことによって、
　アイテムがdupeできたバグを修正。(pc.c)
・buildin_menu, buildin_select() がバッファオーバーフローを起こしていた
　バグを修正(script.c)

	(src/map)
		pc.c			- pc_makesavestatus(), pc_setpos(), pc_autosave_sub() 修正
		script.c		- buildin_menu(), buildin_select() 修正

--------------------
//1132 by End_of_exam
・@users コマンド(サーバー内の人数マップを表示)を追加(atcommand.c / h)
・guild_check_alliance() を呼び出すときのチェックを追加(mob.c battle.c)
・マップサーバー分配時にギルドのメンバーが抜けた時、そのギルドメンバーが
　一人もログインしていないマップサーバーが落ちていたのを修正(guild.c)
・1130で見切りの回避率上昇が消えていたのを戻す(status.c)
・pid 対応版のstart, athena-start を統合(start , athena_start)
・田代砲対策、Shinomoriさんの do_sendrecv() 高速化を組み込む
　(socket.c socket.conf Makefile)
・socket の高速化
　　1. FIFOFLUSH が実行される頻度を下げる(socket.c char.c)
　　2. 不正なfdを0 に変更(socket.c socket.h chrif.c char.c)

	(/)
		start			- pid ファイルに対応するように修正
		sthena-start	- pid ファイルに対応するように修正
		Makefile		- "-D_XOPEN_SOURCE -D_BSD_SOURCE" 追加

	(conf/)
		help.txt		- @users 追加、@mes の修正
		socket.conf		- アクセス制限の設定ファイル

	(src/common/)
		socket.c		- アクセス制限の追加、色々高速化
		socket.h		- FIFO命令の高速化

	(src/char/)
		char.c			- parse_tologin(), parse_char() 更新

	(src/map/)
		atcommand.c		- @users 追加
		atcommand.h		- @users 追加
		battle.c		- battle_calc_damage() 修正
		chrif.c			- 不正なfdを0 に変更したのに伴う修正
		guild.c			- guild_member_leaved() 修正
		mob.c			- mob_gvmobcheck() 修正
		status.c		- status_calc_pc() 修正

--------------------
//1131 by eigen
・ギルド拡張の人数増分を+2/Lvから+4/Lvに変更
・メテオストームにスタンがかかるよう修正
・ロードオブヴァーミリオンに暗闇がかかるよう修正
・ヒルトバインディングを取っていればSTR+1 ATK+4が付くよう変更
・ヒルトバインディングを取っていればAR・OT・WPがの効果時間が10%長くなるよう変更
・AR・OTのパーティーメンバー効果時間減少を撤廃
・フロストダイバーで凍結する際、凍結時間がMDEFに影響されるよう変更
・skill_db.txt、skill_require_db.txt、skill_cast_db.txtをOWNや各職Wikiなどを参考に修正

	(src/map)
		skill.c
		status.c
	(db)
		skill_db.txt
		skill_cast_db.txt
		skill_require_db.txt

--------------------
//1130 by eigen
・所持限界量増加の+100/Lvを+200/Lvに修正
・シーフの上位職に於いて回避率増加のFlee上昇率+3/Lvを+4/Lvに修正
・アサシン系が回避率増加を取得している場合、移動速度が+0.5%/Lvになるよう修正
・プレッシャーのSP攻撃を実装
・プリザーブ、フルストリップ、武器精錬、スリムピッチャー、フルケミカルチャージ
をdbに追加

	(src/map)
		skill.c			- skill_additional_effect() 修正
		status.c		- status_calc_pc() 修正
	(db)
		skill_db.txt
		skill_cast_db.txt
		skill_require_db.txt
		skill_tree.txt

--------------------
//1129 by En_of_exam

・NPC イベントが重複した場合のメモリ解放手順が違っていたバグを修正
　　(npc.c thanks to TOSHI^2さん)

	(src/map)
		npc.c		- npc_parse_script() 修正

--------------------
//1128 by 悩める人
・アイテムを消費せずに使用するかのオプション追加
・カード、装備品、エル・オリのドロップ率を別に設定出来るようにオプション追加
・battle_athena.confの初期設定で矢・聖水等を作成時に名前を付けないように変更
　（本鯖ではまだ来てないと思ったので初期設定を変えました）
	(src/map)
		battle.c
		mob.c
		pc.c
		battle.h
	(conf)
		battle_athena.conf

--------------------
//1127 by End_of_exam

・getarraysize() が正しい値を返さないバグを修正(script.c)
　このバグの影響で、deletearray() 命令の動作が正常なものと異なっていました。

・buildin_deletearray() の最適化(script.c)
・シグナル処理中に再度シグナルが呼ばれる可能性に対処する(core.c)
・委託販売を追加してみる(npc_test_seller.txt)

	(src/map)
		script.c	- getarraysize() , buildin_deletearray() 修正

	(src/common)
		core.c		- sig_proc() 修正

	(script/sample)
		npc_test_seller.txt		- 委託販売NPC

--------------------
//1126 by eigen
・メモライズの効果回数と詠唱短縮比率をそれぞれ5回、1/2に修正

	(src/map)
		skill.c		- 1/3になっているのを1/2に修正
		status.c	- 3回になっているのを5回に修正

--------------------
//1125 by lizorett
・ブランディッシュスピアのノックバックを3セルにし、ミス時にはノックバックしない
よう変更
・スピアスタブを対象から自分に向かって4マスの範囲攻撃に変更(本鯖仕様)
・鷹/投石をニュマで防げるよう変更
・ボウリングバッシュが対象にミスした場合にはノックバックしないよう変更
・ソウルブレイカーのダメージ計算、ニュマでミスになるよう変更
・獲得経験値の上限(現レベルの必要経験値-1)を設定
・バジリカ展開時に展開者はノックバックしないよう変更
・メテオアサルトを即時発動、使用者中心、詠唱500ms固定、エフェクト有に変更
・ストリップウェポン時のmobの攻撃力低下を10%に変更
・掛けられているものより低レベルのブレスにより呪い/石化が解除できるよう変更
・ソウルバーン/マインドブレーカー/ソウルチェンジ実装
・シャープシューティングを射線にいる敵にもダメージを与えるよう変更、クリティカル
確率+20%で防御無視ダメージに変更
・投石など一部のスキルが草などに1ダメージにならない問題を修正

	(db)
		skill_db.txt- BDS/メテオアサルト変更、スキル追加
		skill_cast_db.txt
					- スキル追加
		skill_require_db.txt
					- スキル追加
	(src/map)
		battle.c	- ソウルブレイカーのダメージ計算を変更
					- シャープシューティングのクリティカル確率修正
					- 鷹/投石をニュマで防げるよう変更
		skill.h		- SC_MINDBREAKER追加
		skill.c		- BDS/BBのノックバックを修正
					- スピアスタブを範囲攻撃に変更
					- メテオアサルト修正
					- ソウルバーン/マインドブレーカー/ソウルチェンジ実装
		path.c		- シャープシューティングの射線計算を追加
		pc.c		- 獲得経験値の上限(前のレベルの経験値-1)を設定
		status.c	- マインドブレーカーのmatk上昇/mdef減少の実装
		map.h		- シャープシューティングの射線計算用構造体を追加

--------------------
//1124 by もっさり
敵が使う爆裂波動実装
効果
atk1,atk2 1000*skilllv加算
hit 20*skilllv加算

	(src/map)
		skill.c
		skill.h 	NPC_EXPLOSIONSPIRITS関係を追加
		status.c　　　　
	(db)
		skill_db.txt
		skill_cast_db.txt	

		

--------------------
//1123 by Nameless
・Athenaサービス化キットを追加しました。(NT/2000/XP/2003/LH)
　詳しい方法はdoc内のinstasv.txtを参照してください

	(bin/tool)
		instasv.bat	- サービス登録用バッチ
		delasv.bat	- サービス抹消用バッチ
	(doc/)
		instasv.txt	- 説明書(テキスト版)

--------------------
//1122 by End_of_exam

・1120のstrdb のキーを保存し忘れていたバグ修正（db.c）
・念のため1121、1120のreadme をマージして、両方に含まれていたファイルを添付する

	(src/char)
		char.c		- 1121のものを添付

	(src/common)
		mmo.h		- 1121のものを添付
		db.h		- 1120のものを添付
		db.c		- strdb のキーを保存するようにする

	(src/map)
		battle.c	- 1121のものを添付
		guild.c		- 1121のものを添付
		guild.h		- 1121のものを添付
		mob.c		- 1121のものを添付
		skill.c		- 1121のものを添付
		skill.h		- 1121のものを添付

--------------------
//1121 by _

・ロードナイト/パラディンのログイン時のエラー対策
・Gvでの同盟の扱いを修正
　エンペリウム攻撃不可、ガーディアンから攻撃されないように修正
・新追加スキル用の定数追加修正

	(src/char)
		char.c
			修正	mmo_char_send006b()
	(src/common)
		mmo.h
			修正	MAX_SKILL=500
			追加	新ギルドスキル(コメントアウトしてます)
	(src/map)
		battle.c
			修正	battle_calc_damage()
		guild.c
		guild.h
			追加	guild_check_alliance()
		mob.c
			修正	mob_gvmobcheck()
		skill.c
			修正	SkillStatusChangeTable[] (420-490)
		skill.h
			修正	MAX_SKILL_DB=500
			追加	475以降の新スキルID

--------------------
//1120 by End_of_exam

・db_foreach()の呼び出し先でdb_erase()が呼び出されているされている場合、
　複数回同じキーで関数を呼び出す可能性があるバグを修正(db.h db.c)

　cygwin上で２重freeをした場合、プログラムが暴走する可能性があります。
　char鯖との接続が切れたmap 鯖が暴走するバグは、これに起因しています。

	(src/common)
		db.c		- db_eraseを一時的にロックする機能追加
		db.h		- db_eraseを一時的にロックする機能追加

--------------------
//1119 by ICO

・NPCスキル(ブレイクウェポン、ブレイクアーマー、ブレイクヘルム、ブレイクシールド)の実装
・battle_athena.confにmonster_damage_delayを追加。
　noを指定するとFW等のノックバックスキルの挙動が多少本鯖に近づくかも…？

	(db)
		skill_db.txt
		skill_cast_db.txt
	(conf/)
		battle_athena.conf
			monster_damage_delay 追加
	(map/)
		battle.c
		battle.h
		mob.c
			monster_damage_delay関連を追加
		skill.c
		skill.h
			skill_additional_effect,skill_castend_damage_id 修正

--------------------
//1118 by BDPQ銀 [ 2005/02/10 ]
■データベースが変更されています。導入時には御注意ください■
・スキルの固定詠唱時間を skill_cast_db.txt に移動。
  詠唱時間の計算は、 (通常詠唱 + 固定詠唱)*メモライズ補正 となります。
  skill_cast_dbの書式は
    [ID],[cast_list(通常詠唱)],[fixed_cast_list(固定詠唱)],[delay_list(ディレイ)],[upkeep_time(維持時間)],[upkeep_time2(維持時間2)] です。
・アブラカタブラをディレイにASPDによるディレイを付加しないよう修正(即発動スキル用)
・新2次職のskill_cast_dbに関する項目の修正

	(src/map)
		skill.c			-	skill_use_id()			修正	(詠唱時間計算部 ・ メモライズ/魔法力増幅 固定詠唱時間部削除)
															(アブラカタブラの修正)
							skill_use_pos()			修正	(詠唱時間計算部)
							skill_readdb()			修正	(cast_db 読込部)
		skill.h			-	skill_db				修正	(fixedcastの追加)
							skill_get_fixedcast()	追加	(dbから固定詠唱時間の取得)

	(db)
		skill_cast_db.txt-	fixed_cast_list			追加	(固定詠唱時間) 
															魔法力増幅-700、メモライズ-5000に設定

							361(アスムプティオ)		修正	( R.O.M 776を参考に詠唱/ディレイを修正 )
							365(マジッククラッシャー)修正	( R.O.M 776を参考に詠唱/ディレイを追加 )
							373(ライフ置き換え)		修正	( R.O.M 776を参考にディレイを修正 )
							375(ソウルバーン)		追加	( R.O.M 776を参考にディレイを追加 ) ( スキル効果は実装していません )
							381(ファルコンアサルト)	修正	( R.O.M 776を参考にディレイを修正 )
							383(ウィンドウォーク)	修正	( R.O.M 776を参考に詠唱/ディレイ/効果時間を修正 )	
							384(メルトダウン)		修正	( R.O.M 776を参考に詠唱/ディレイを修正 )	
							387(カートブースト)		修正	( R.O.M 776を参考に効果時間を修正 )	
							398(ヘッドクラッシュ)	修正	( R.O.M 776を参考にディレイ持続時間を修正 )
							406(メテオアサルト)		修正	( R.O.M 776を参考に詠唱/ディレイを追加 )

	(doc)
		db_ref.txt		-	1. db/skill_cast_db.txt	修正	(fixed_cast_listの項目を追加)

--------------------
//1117 by End_of_exam

・ベナムスプラッシャーを毒状態の敵に使用したが、失敗した時（敵モンスターの
　HPが2/3 以上だった時）に深刻なメモリリークが起きていたバグを修正(skill.c)
・あなたに逢いたいが失敗した時に深刻なメモリリークが起きていたバグを修正(skill.c)

　上２つは、共にmap_freeblock_unlock() が抜けている為に発生していました。
　ドロップアイテム、スキルユニット、取り巻きなどで確保されたメモリが、
　以降全く開放されなくなるというかなり深刻なメモリリークのバグです。
　map_freeblock_lock() を呼ぶルーチンを修正する場合、ルーチンを抜けるときに、
　map_freeblock_unlock() が呼ばれるように気を付けてください(return に注意!)。

・map_freeblock_unlock() を忘れても良いように、定期的にblock_free_lockを
　クリアするように修正(map.c)
・Debian好き さんのMPVモンスターのHP計算がオーバーフローするバグ修正の取り込み(status.c)

	(src/map)
		skill.c		- skill_castend_nodamage_id() 修正
		map.c		- map_freeblock_timer() 追加、 do_init() 修正
		status.c	- status_get_max_hp() 修正

--------------------
//1116 by End_of_exam

・copyarray で同じ配列を指定した時、コピー先の要素番号がコピー元の要素番号より
　大きい時の動作が不定になっていたバグを修正(script.c npc_test_array.txt)
・関数宣言せずに関数定義したユーザー定義関数を呼び出そうとすると、エラーが出る
　バグを修正(script.c)
・スクリプトのオーバーフロー判定基準を緩和させる(script.c)
・ギルドの告知に\nが使えるバグを修正(int_guild.c)
・イベントdbのメモリリーク修正が不完全だったのを修正(npc.c)
・db_foreachのチェック方法を変更(db.c)
・起動時に*.pid (プロセスIDのファイル)を作成するようにする(core.c)
・経験値所得が全体発言になっているバグを修正(clif.c)
・叫ぶを全体発言に変更(clif.c)
・testerさん作成のVC++ Toolkit2003 用のバッチファイルを同伴(vc07_make.bat)

	(/)
		vc07_make.bat	- testerさん作成のバッチファイルを同伴

	(src/common)
		db.c			- db_foreach() 修正
		core.c			- main() 修正 , pid_create() , pid_delete() 追加

	(src/char)
		int_guild.c		- mapif_parse_GuildPosition() 修正

	(src/map)
		clif.c			- clif_disp_onlyself() , clif_onlymessage() 修正
		npc.c			- npc_parse_script() 修正
		script.c		- buildin_copyarray() , parse_syntax() 修正

	(script/sample)
		npc_test_array.txt	- チェック項目の追加

--------------------
//1115 by いど

・サーバースナップショット

--------------------
//1114-fix1 by 稀枝

・zlibをmap-server内部に取り込めるオプションを追加
・makeがMinGW+Msysで正常に通るよう修正
・win32_start.batにチェック追加

	(src/common/zlib)
		trees.h		- anybody's guess上のzlib_1_2_1_staticlibより取り込み
		inffixed.h	- 同上
		inffast.h	- 同上
		crc32.h		- 同上
		compress.c	- 同上
		deflate.h	- 同上
		inftrees.h	- 同上
		zutil.c		- 同上
		crc32.c		- 同上
		inflate.h	- 同上
		inffast.c	- 同上
		trees.c		- 同上
		inflate.c	- 同上
		zconf.h		- 同上
		deflate.c	- 同上
		inftrees.c	- 同上
		zutil.h		- 同上
		zlib.h		- 同上
		adler32.c	- 同上
		Makefile	- LOCALZLIBが指定されている時のみコンパイルします。

	(src/map/)
		Makefile	- MinGWの場合、引数-wsock32を追加します。
				- LOCALZLIBが指定されている場合リンクします。
				- LOCALZLIBが無い場合だけzlib.aをリンクします。
	(src/char/)
		Makefile	- MinGWの場合、引数-wsock32を追加します。
	(src/login/)
		Makefile	- MinGWの場合、引数-wsock32を追加します。
	(src/common/grfio.c)	- Zlibを内包した際に_WIN32と競合しないよう変更
				- zlib_win32.h zconf_win32.hを廃止

	(./)
		Makefile	- #Link Zlib(NOTrecommended)、zlibを内包します。
		win32_start.bat	- athena-startの半クローン化。初期起動でこけなくなるはずです。

--------------------
//1113 by End_of_exam

・linux 環境で大量のwarning が出ていたのを修正(malloc.h)
・map_quit() でcharid_db のデータを削除しないように変更(map.c thanks to lemitさん)
・pc_eventtimer(), npc_event_timer() のfree()で警告が出ていたのを修正(pc.c npc.c)
・map_eraseipport() がメモリリークしていたバグを修正(map.c)
・addtimer 命令に指定するイベント名が２３文字に制限されていたのを無制限にする(pc.c)
・pc_cleareventtimer() , pc_deleventtimer() がメモリリークしていたバグを修正
　(pc.c thanks to Shinomoriさん)

	(src/common/)
		malloc.h	- "#undef strdup" を追加

	(src/map/)
		npc.c		- npc_event_timer() 修正
		pc.c		- pc_eventtimer() , pc_addeventtimer() , pc_cleareventtimer(),
					  pc_deleventtimer() 修正
		map.c		- map_quit() , map_eraseipport() 修正

--------------------
//1112 by lizorett
・PCがマップ移動中に、そのPCが設置したスキルユニットのskill_unit_onoutが呼ばれ
ない問題(map-severが落ちる可能性あり)を修正
・バジリカを仕様に併せて修正
・正常終了時にchar-serverがコアダンプする問題を修正
・mobがウォータボール使用時のヒット数を修正(skill_db.txtに指定した数ヒット)
・コーティングされている場合にはストリップできないよう変更
・属性場を使用した際、前に出していた属性場が消えないことがある問題を修正

	(db)
		skill_db.txt
				- mobのウォータボールのカウント数をDBにいれた
		skill_unit_db.txt
				- バジリカを修正
	(char)
		char.c	- do_final()のchar_datのメモリ開放位置を変更
	(map)
		clif.c	- バジリカ時に攻撃などができないよう変更
		map.c	- バジリカ位置をセルのフラグに入れるよう変更
		map.h	- バジリカ用のセルフラグ追加
		mob.c	- バジリカに進入できないように変更
		pc.c	- 移動時(蝿など)にバジリカを消すよう修正
		skill.c	- バジリカ修正
				- ウォータボール修正
				- skill_unit_onoutの呼び出しを修正
				- コーティングされている箇所はストリップ不可に変更

--------------------
//1111 by Toshi^2
・pc系mobに転生＆養子を指定できるように変更。
　db/mob_avail.txtに説明文を追加したので、それを参照してください。

	(db)
		mob_avail.txt	- 引数の説明を追加。
	(src/map)
		clif.c	- clif_mob0078() clif_mob007b() clif_pet0078() clif_pet007b() 修正
		mob.c	- mob_readdb_mobavail() 修正
		mob.h	- 構造体mob_dbに「short trans」を追加、mob_availのtransフラグを格納。

--------------------
//1110 by lizorett
・ユニット系スキル(ニュマ、ダンス等)でmap_server.exeが落ちる問題を修正
 (トレースではskill_unit_onplace/skill_unit_onoutで落ちる)
・サンクチュアリの人数カウント方法を変更(本鯖仕様)
・マグヌスの範囲を広げ、使用したユニットが削除されるようにする(本鯖仕様)
・デボーションの距離が短くなる問題を修正(バグ報告スレッド part8 >>15)
・デボーションでnullpoが出る問題を修正
・mobのインティミデイトが成功するとmap-serverが落ちる問題を修正(バグ報告スレッ
ド part8 >>42)
・ウォーターボールの仕様を本鯖に近づける(水場が少ない場合にはhit数が減る、
デリュージ上で実行するとユニットが欠ける)
・ファーマシーの製造成功確率のコードを変更

	(db)
		skill_unit_db.txt
				- ユニットID/配置などをdb化しています
	(src/map)
		map.h	- skill_unit_groupのメンバ変更
		mob.c	- 移動時にスキルユニット判断(skill_unit_out_all/skill_unit_move)
				を追加
				- 足元置き/重複置き判断を変更
		pc.c	- 移動時にスキルユニット判断(同上)を追加
				- 無敵時間が終わる際にスキルユニット判断(同上)を追加
		skill.h	- skill_dbの参照関数をdefineに変更
				- スキル配置を入れるskill_unit_layout構造体を定義
				- SC_WATERBALL削除
		skill.c	- unit_idをdb化(skill_unit_db.txt)
				- スキルユニットのレイアウトを起動時に定義
				- 移動時にスキルユニット判断(同上)を追加
				- 足元置き/重複置き判断を変更
				- スキルユニットの移動処理を変更
				- デボーションの修正
				- mobのインティミデイトで落ちる問題を修正
				- ウォータボールの仕様変更
		status.c- SC_WATERBALLの処理を削除

--------------------
//1109 by End_of_exam

1108に引き続きメモリリークのバグ修正です。２つ共に深刻なバグなので、
最新版に更新しない方でも修正することをおすすめします。

・ペットが床にアイテムを落とす時、ペットを卵に戻す時にメモリリークが発生
　していたバグを修正。(pet.c)

・キャラクター依存一時変数の利用したキャラがログアウトするとメモリリークが
　発生していたバグを修正(map.c)

	(src/map)
		map.c	- map_quit() 修正
		pet.c	- pet_remove_map(), pet_return_egg() pet_lootitem_drop() 修正

--------------------
//1108 by End_of_exam

・以前作ったメモリマネージャーを統合。(malloc.c core.c)
　有効にするには、malloc.c内部のコメントを外す必要があります。開発に協力して
　頂ける方は、メモリマネージャを有効にして、チェック結果(map-server.logなど)を
　アップロードしてくれると助かります。

　　1. guild.c がコンパイルエラーになったので修正(guild.c)
　　2. pet.c がメモリ解放し忘れていたので、do_final_pet() を追加(pet.c)
　　3. do_final_socket を追加して、終了時に全ての接続を切断する(socket.c)
　　4. deplicate の元スクリプトが終了時にfreeされないバグを修正(npc.c)
　　5. do_final_script で開放されないメモリがあるバグを修正(script.c)
　　6. do_init_*** の呼ばれる順番がおかしかったのを修正(map.c)
　　7. イベント名が重複したときにメッセージを出すように変更(npc.c)
　　8. map_quit() 内部でcharid_db をfreeし忘れているバグを修正(map.c)

　特に8.は最重要で、キャラがログアウトする度にメモリリークが発生するという、
　最悪な結果になっていました。気になる方は修正しておきましょう。

・delete_session でNULLチェックを怠っていたバグを修正(socket.c)
・chrif_disconnect_sub でdelete_session を呼ぶように変更(chrif.c)
・マルチラインコメント（/* ～ */）の解析を忘れていたバグを修正(npc.c)
・銀行などのNPC でZenyがMAX_ZENYにならないバグを修正(pc.c)
・1107の製造確率が一部消されていたのを修正(skill.c thanks to lizorettさん)
・セージ転職試験のイベントが衝突を起こしていたのを修正(npc.c)
　　npc_parse_script : dup event jobsage_2nd::OnTimer150000
　　npc_parse_script : dup event jobsage_2nd::OnTimer30000
　　npc_parse_script : dup event jobsage_success::OnTimer7000
　　npc_parse_script : dup event jobsage_success::OnTimer3000

	(src/common)
		core.c		- do_init_memmgr() 追加
		malloc.c	- メモリマネージャの追加
		malloc.h	- メモリマネージャの追加
		socket.c	- delete_sessionのバグ、do_final_socketの追加

	(src/map)
		chrif.c		- chrif_disconnect_sub() を修正
		guild.c		- guild_recv_info(), guild_castledataloadack() 修正
		map.c		- map_quit() のメモリリーク、do_final,do_init 修正
		npc.c		- npc_parse_script_line() , npc_parse_script() 他修正
		pc.c		- pc_setparam() 修正
		pet.c		- do_final_pet() 追加
		pet.h		- do_final_pet() 追加
		script.c	- do_init_script(), do_final_script() 修正
		skill.c		- skill_produce_mix() 修正

--------------------
//1107 by code
・@npctalk, @pettalkコマンド追加
・ダメージの遅延を実装
・@mesを全体発言に修正
・ファーマシーの製造成功確率修正
・@storageで倉庫が二重で開くことがないよう修正
・scriptに globalmes, getmapmobs 関数を追加

	(/src/map)
		atcommand.c
		atcommand.h
		battle.c
		clif.c
		clif.h
		npc.c
		npc.h
		script.c
		skill.c
		storage.c

--------------------
//1106 by sylpheed

・item_rate_details:1が動かなかったのを修正

	(src/map/)
		mob.c

--------------------
//1105 by End_of_exam

・1101のマップの再分配が上手くいかないバグを修正(char.c thanks to Mystleさん)

	(src/char/)
		char.c		- parse_frommap() 修正

--------------------
//1104 by nameless
・BCC32のコンパイルオプションなどの最適化
・BCC32/VC++で最適な最適化オプションを見つけるためのベンチ
・bcc32_clean.batとbcc32_make.batを統合、クリーンビルドの失敗をしないように。

※P4だからとかOpteronだから特定オプションで早いということではないようです。
※P4でもロットによっては-5が最適だったり-3 -O2が最適だったりするものがあるようです
※思い込みでオプションをつけないようにするために作りました。
※少しでもレスポンスを上げて運用したいという人は活用してください。

	(/)
		bcc32_make.bat

		最適化オプションの追加と警告メッセージで深刻ではないものを
		完全に表示しないように設定、bcc32_clean.batをmakeに統合した
		ので確実にクリーンビルドできるようになりました

		bench.bat
		bench.c

		最適なコンパイルオプションを見つけるためのベンチです。
		bench.batでコンパイル＆実行が行われます。
		結果はbench.txtに格納されますので数値の一番小さいものを選ん
		でbcc32_make.batの23行目に追加・修正してあげてください。
		※初期状態ではbcc32用になっていますので
--------------------
//1103 by End_of_exam

・char_athena.conf のdefault_map_typeが0 になっている時に、PVPガイドで
　セーブした後、PVPエリア内でログアウトしたキャラがログインできなくなる
　バグを修正。(npc_etc_pvp.txt) 多くの方々からの情報提供感謝します。
　（npc_etc_pvp.txt 内部の ".gat" の付け忘れと、このミスに対応していない
　　pc.c のバグです。このパッチを当てないでこのバグを修正したい場合、
　　添付した修正ファイルを参考にしながら、npc_etc_pvp.txtに".gat"を
　　付加してください。）

・do_final内部で不正な処理を行う場合があるのを修正(map.c thanks to lizorettさん)
・マップキャッシュの読み込みに失敗したときにメモリリークしていたバグを修正(map.c)

	(src/map)
		pc.c	 - pc_setsavepoint() 修正
		map.c	 - do_final(), map_cache_read() 修正

	(src/char)
		char.c	 - search_mapserver() , parse_char() 修正

	(script/npc/etc)
		npc_etc_pvp.txt - ".gat" を付加する

--------------------
//1102 by 人柱さんA
・バグ報告スレ >>35-37にあった修正版
　詳しいことはスレを見てください
	(src/map)
		pc.c

--------------------
//1101 by End_of_exam

・socket関連の修正(socket.c socket.h)

　1. FIFO関連をfd が不正(fd<=0)の時にも正常に動作するように変更
　2. socket.h の内部を色々と整理
　3. make_connection() が接続に失敗した時にエラーを返さないバグを修正
　　　その変更に合わせて、chrif.c check_connect_char_server() , char.c
　　　check_connect_login_server() を修正。これでサーバーゾンビ化のバグは
　　　解決したと思いますが、再発したら報告をお願いします。

・char - map 間のコネクションを見直し(char.c chrif.c map.c map.h)

　1. char - map 間のコネクションが切れたら、map 鯖に接続しているキャラを
　　全て切断するようにする。これは、同期を取るのが難しいのと、char鯖との
　　通信が必要な処理（パーティ、ギルド、ペット他）ができなくなるためです。
　2. 複数のmap 鯖で同じマップを担当することがあるバグを修正
　3. map 鯖の割り当て方法の見直し
　　複数のmap 鯖で同じマップを読み込めば、どれか１つが落ちている時でも、
　　正常なmap 鯖にログインできるようになりました。例えば、同じマップを
　　map鯖AとBに読み込ませておけば、Aが落ちている時にはBに、Bが落ちている
　　時にはAに転送されます。ただし、優先順位の指定はまだ出来てないので、
　　１つのmap 鯖に人数が集中しすぎる可能性があります。

	(src/map)
		chrif.c		- map 鯖の割り当て方法の見直し
		map.c		- map 鯖の割り当て方法の見直し
		map.h		- map 鯖の割り当て方法の見直し

	(src/char)
		char.c		- map 鯖の割り当て方法の見直し

	(src/common)
		socket.c	- 色々修正（上記参照）
		socket.h	- 色々修正（上記参照）

--------------------
//1100 by nyankochan
・1098の修正
	(src/map)
		pc.c

--------------------
//1099 by End_of_exam

・mapflag nosave が不正な時に起動を中断させるようにする(npc.c)
・pc_autosave が呼ばれる回数が異常に高くなるバグを修正(pc.c)

pc_autosave() の内部が、

> interval = autosave_interval/(clif_countusers()+1);
> if(interval <= 0)
> 	interval = 1;

という風になっているので、１マップサーバーに200人のキャラが接続してると、
autosave_interval(def:15 * 1000) / 200 = 0.075 秒ごとに関数が呼ばれます。
さすがにこの状態だとchar鯖が厳しくなるので、関数を呼び出す最小間隔を
0.2 秒に変更しました。

	(src/map)
		pc.c		- pc_autosave が呼ばれる回数が異常に高くなるバグを修正
		npc.c		- mapflag nosave が不正な時に起動を中断させる

--------------------
//1098 by nyankochan
・入手装備品の個数1固定
	(src/map)
		pc.c

--------------------
//1097 by End_of_exam

主にバグ修正です。バグ報告してくれた皆様に感謝、感謝。

・Zeny増殖対策(pc.c trade.c script.c)
	1. 交換、pc_setparam でMAX_ZENY を超える場合があるバグを修正
	2. スクリプトにオーバーフロー対策を追加

・ログイン成功時・アカウント変数更新時に無条件にmmo_auth_sync を
　呼んでいたのをタイマーを使用した定期更新に変更(login.c login_athena.conf)

・db関係にバグが潜んでいる模様なので、チェック機構を追加する(db.c db.h)
　一部アカウントのみログイン不可、倉庫ロスト、@whoで表示されるキャラが
　一部消えるなどのバグの原因がdb関連にある模様です。
　「db_foreach : data lost %d of %d item(s)」というメッセージが表示
　された場合はバグがある（dbに入っているはずのデータが消えた）ので、
　報告をお願いします。

・ある方法で通常より強いキャラが作れてしまうバグの修正(char.c)
・ギルドの役職名に不正な文字が使えるバグを修正(int_guild.c)
・スクリプト内で０での除算時が起こった時にINT_MAXを返すようにする(script.c)

	(conf/)
		login_athena.conf - autosave_time の追加

	(src/common)
		db.h		- チェック機構の追加
		db.c		- チェック機構の追加

	(src/login)
		login.c		- mmo_auth_sync にタイマーを適用

	(src/char)
		char.c		- 通常より強いキャラが作れてしまうバグの修正
		int_guild.c	- ギルドの役職名に不正な文字が使えるバグを修正

	(src/map)
		trade.c		- MAX_ZENY を超える場合があるバグを修正
		pc.c		- MAX_ZENY を超える場合があるバグを修正
		script.c	- オーバーフロー対策、０での除算時の処理を追加

--------------------
//1096 by lizorett
・壁越しにスキルが撃ててしまうバグを修正(バグ報告スレッド part8 >>28)

	(src/map)
		path.c	- 壁越しにスキルが撃ててしまうバグを修正

--------------------
//1095 by lizorett
・スクリプトのエスケープ判断を変更
・スキルユニットグループのgroup_idの範囲を変更
・skill_unitsetting()で全てのスキルでskill_get_time()を使用するよう変更
・サイトラッシャーをユニットスキルから範囲攻撃魔法に変更(本鯖仕様)

	(src/map)
		skill.c	- skill_unitgrouptickset_* で skill_id/group_idが重ならないよう
				にgroup_idの範囲を制限
				- SkillStatusChangeTableにSC_SAFETYWALL,SC_PNEUMA追加
				- skill_unitsetting()で全てのスキルでskill_get_time()を使用する
				よう変更
				- サイトラッシャーを範囲攻撃魔法に変更
		npc.c	- エスケープ判断をparse_simpleexpr()と同様にし、全角判断を削除
	(db)
		skill_cast_db.txt
				- TS/MS/LoV/FN/SG/HD/GXにupkeep_timeを設定

--------------------
//1094 by End_of_exam

・サブルーチン呼び出し構文の追加(script.c npc.c npc_convertlabel_db())
・逆アセンブル処理の追加(script.c , DEBUG_DISASM を有効にしてください。)
・switch の一時変数消去位置を変更(script.c)
・RERUNLINEの衝動が怪しかったので修正(script.c / h , map.h)
・ソースを読みやすくするためにbuildin_*をファイル最後に移動(script.c)
・新しく加わった構文のサンプルとして「ハノイの塔」を追加(npc_test_hanoi.txt)
・buildin_getitemname 修正(script.c , 質問スレッド Part14 >>129-130)

・bcc でコンパイルした時に落ちるバグを修正(map.c map_id2bl 内部)
・Windowsでコンパイルした時に、gettick()のキャッシュが無効になっていたのを
　修正(timer.c , thanks to Shinomori)

	(src/common)
		timer.c			gettick() のバグ修正

	(src/map)
		script.c		色々変更（上記参照）
		script.h		struct script_state 修正
		npc.c			npc_convertlabel_db() で落ちるのを修正
		map.c			map_id2bl() を修正(落ちるのはbcc だけ？)
		map.h			map_session_data 修正

	(doc/)
		script_ref.txt	function 構文の追加
	
	(script/sample/)
		npc_test_hanoi.txt	ハノイの塔

--------------------
//1093 by いど

・サーバースナップショット

--------------------
//1092 by lizorett
・遠距離攻撃のパス検索アルゴリズムを本鯖と同じになるよう変更
・ストームガスト、ロードオブヴァーミリオンを重ねた場合、片方からだけダメージを
受けるよう修正(仕様が不明なので先に見つけたユニットから攻撃するようにしています)
・アイテムが一つしかない場合、アイテムを使用してもエフェクトが表示されない問題
を修正
・セーフティーウォール使用時にアドレス不正となる場合がある問題を修正
・map_getcell/map_setcellの仕様変更
・1085のビットマップ対応の痕跡の消去
・1088のアイスウォールの変更を巻き戻し(本鯖に合わせる)
・全角判断(npc.c)を修正(For English User Forum >>54)

 (注意) map_athena.confのマップキャッシュ指定を行うパラメータ名を変更しています

	(conf)
		map_athena.conf	- read_map_from_bitmapをread_map_from_cache に変更
						- map_bitmap_pathをmap_cache_fileに変更
	(src/map)
		map.h			- セルタイプ名称変更(CELL_CHKHIGH,CELL_CHKTYPE)、
						削除(CELL_SET*)
						- skill_unit_group_ticksetメンバ名変更(group_id -> id)
						- ビットマップ関連の記述の痕跡を削除
		map.c			- map_getcell() セルタイプ名称変更と若干のコード変更
						- map_setcell()を1084以前の仕様に戻し、CELL_SETNPCの
						フラグを追加
						- map_cache関連の細かな修正
		npc.c			- 全角判断(is_zenkaku)を正確に行うよう変更
						- map_getcell()のセルタイプ名称変更に追従
		pc.c			- アイテムが一つしかない場合、使用時のエフェクトが表示
						されない問題を修正
						- map_getcell()のセルタイプ名称変更に追従
		skill.c			- ストームガスト、ロードオブヴァーミリオンを重ねた場合
						片方からだけダメージを受けるよう修正
						- セーフティーウォール使用時にアドレス不正となる場合が
						ある問題を修正
						- map_getcell()のセルタイプ名称変更に追従
		skill.h			- 関数定義変更
		path.c			- 遠距離攻撃のパス検索処理を追加(path_search_long)
						- map_getcell()のセルタイプ名称変更に追従
		battle.c		- 遠距離攻撃のパス検索を使用するよう変更

--------------------
//1091 by End_of_exam

＊＊　注意　＊＊

　今回のパッチは改造内容が複雑なので、導入は慎重に行ってください。
　status.c / h への分離は、関数名の置き換えだけに留めたつもりですが、
　思わぬバグが潜んでいる可能性があります。

・スパゲティ対策の一環として、ステータス計算、状態異常に関わる部分を
　status.c / h として分離。battle.c から39KB程 , skill.c から41KB程 ,
　pc.c から38KB程移動できました。少々強引ですが、スキル使用や攻撃などの
　処理の流れをつかみやすくするためには、battle.c / skill.c の中身を
　減らす必要があると考えたためです。

	battle_get_*          => status_get_*
	skill_status_change_* => status_change_*
	pc_calcstatus         => status_calc_pc
	pc_calc_sigma         => status_calc_sigma
	pc_getrefinebonus     => status_getrefinebonus
	pc_percentrefinery    => status_percentrefinery

・battle.c , script.c の巻き戻りを戻す
・npc.c の怪しい変更を戻し、きちんとNUL を付け加えるようにする
・script.c のミスを直す（jump_non_zero => jump_zero　意味が逆になってました…）
・skill.c の武器修理 のコメントミスを修正
・WIN32でコンパイルした時、最大接続人数が60人程に制限されていたバグを修正

	(/)
	athena.dsw , athena.dsp , bcc32_make.bat , src/login/login.dsp
	src/char/char.dsp , src/map/map.dsp
		コンパイル条件の変更

	(src/map/)
		上の分離に合わせてコンパイルエラーの出ないように修正

--------------------
//1090 by Sapientia
・チャットの便利さのために叫ぶ追加 (ギルドチャットと区分するためにウェチギシの前に [叫ぶこと]が付き)
・atcomand_athena.conf で onlymes を 0で設定して皆使うように活性化
  オリジナルなのでデフォルトで GMだけ使うことができるように設定しました.
・@mes [言うこと] で使用

	(src/map)
		atcommand.c	atcommand_charkami 追加
		atcommand.h
		clif.c		clif_onlymessage 追加
		clif.h
	(src/conf)
		atcommand_athena.conf	onlymes 追加
		help.txt	@mes 説明追加


--------------------
//1089 by 聖
・VC.NET2003でコンパイルすると大量に警告が出るのを修正
・その他バッファオーバーフロー等の細かいバグ修正

	(src/char)
		int_guild.c 警告箇所を修正

	(src/map)
		atcommand.c, battle.c, clif.c, itemdb.c, pc.c, pc.h, script.c, skill.c
			警告箇所を修正
		npc.c 警告箇所とバッファオーバーフロー修正

--------------------
//1088 by Sapientia
・ウィザドスキル Icewall このキャラクターやモンスター足もとに設置されることを防止
・ロードナイトスキル Berserk 使用の時 HPが 1/3になれば回復するバグ修正

	(src/map)
		pc.c	Berserk 修正
		skill.c	Icewall 修正

--------------------
//1087 by End_of_exam

・マップキャッシュに圧縮機能を追加(1MB程に縮まるようです)
・npc.c の巻き戻りを修正(質問スレッド Part14 , 111)
・map_athena.conf のコメントアウトを修正(Athena雑談スレッドPart7 , 146)
・Windows 用の起動スクリプトを追加してみる(eAthena のを元に改造）

	(/)
		win32_start.bat		Windows 用の起動ファイル

	(src/map)
		map.c	圧縮機能の追加
		npc.c	巻き戻りを修正

	(src/common)
		grfio.c		decode_zip , encode_zip のエクスポート
		grfio.h		decode_zip , encode_zip のエクスポート

	(conf/)
		map_athena.conf		修正

--------------------
//1086 by End_of_exam

主に1085のバグ修正だったりもします。
「てめー、１から書き直しやがって」という突っ込みだけは勘弁してくださいませ。

・データ構造の大変更(map.c)
	マップを削除＆追加しても正しく動くように変更
	マップキャッシュ作成中に強制終了すると再起動時に不安定になるバグを修正
	圧縮フラグの追加（需要あるのか不明。compressを真にすると、現在のソースで
	読めなくなります。）

・なんか衝動があやしすぎるので、ビットマップ処理を撤廃する(map.c map.h)
	npc_touch_areanpc : some bug　がたくさん出てくる -> 原因不明？
	恐らく通行可能判定が正しく設定されていないっぽいんですが謎です。
	＃read_map_from_bitmap の設定を省くとログイン時に落ちるバグを修正

・キャッシュ内に全てのマップがあれば、grf 無しでも動作するように変更。(grfio.c map.c)

	(src/map)
		map.c	バグ修正他
		map.h	バグ修正他

	(src/common)
		grfio.c		ファイルが見つからない時にexit を呼ばないように修正

--------------------
//1085 by zalem
・マップデータの読み込みはビットマップファイルから行なえるような機能追加

		grfファイルから一度ビットマップファイルを作成して以後はその
	作成されたビットマップフォーマットのファイルからマップ情報を読み込む
	という方法を採ることによって、map-serverが立ち上がる時マップ情報を読み取る
	のに掛かる時間がほとんどなくなる、また1intに32個のセル情報が格納できるの
	で、map情報に関するメモリ使用量も３割り近くまで減るので(そのかわりに
	ある程度CPUの負担が大きくなる)、追加してみた。
		conf/map_athena.confのread_map_from_bitmapオプションで利用する
	かどうかを指定でき、その下にあるmap_bitmap_pathでファイル名を変更する
	(デフォルトでdb/map.info)
		まだテスト段階なので、導入はご慎重に(一応Linuxで、いろいろと
	テストしてみたが...)

・map_getcell()に4番目引数の追加とmap_setcell()の4番目引数の変更
		
		関数の利用意図がわかりやすいように、そしてこれからの変更を容易にする
	ために、map_getcell()とmap_setcell()のそれぞれ4番目の引数を追加、変更してみた、
	map_getcell()の4番目の引数はmap.hで定義されてるCELL_CHK列挙型、map_setcell()
	の4番目の引数はmap.hで定義されてるCELL_SET列挙型をとるように変更.また、上の
	Featureに対応するため、map_getcell()をポインタに変更した。
		
	主な変更点：

	src/map/map.h   read_gat(),read_gatp()マクロの変更
			列挙型 CELL_CHK,CELL_SETを追加,map_getcell(),map_setcel()用
			map_data構造体にメンバーint* gat_fileused[MAX_CELL_TYPE+2]追加
	src/map/map.c	map_getcell()を関数型ポインタに変更,map_getcellp()をread_gatp()
		     のために追加,実際に下の四つの関数のどっちに指すかはmap_read_flagによる
		     	map_getcell_gat(),map_getcell_bitmap()	追加
		     	map_getcellp_gat(),map_getcellp_bitmap()	追加
		     	map_setcell()	変更
			map_createbitmap()	追加
			map_readmapfromfile()	追加
			map_readallmap()	変更
			map_config_read()	変更
			do_final()	変更
	以下の*.cファイル内のmap_getcell(),map_setcell(),read_gat(),read_gatp()を呼出した部分をすべて変更
	src/map/atcommand.c
	src/map/mob.c
	src/map/npc.c
	src/map/path.c
	src/map/pc.c
	src/map/pet.c
	src/map/skill.c
			
	conf/map_athena.conf	read_map_from_bitmap,map_bitmap_path	追加

--------------------
//1084 by lizorett
・経験値獲得のバグ修正(バグ報告スレッド part7 >>134)
	(src/map)
		mob.c	経験値計算修正

--------------------
//1083 by End_of_exam special thanks to lizorettさん
・ソケットのデストラクタ処理の追加
		(common/socket.c common/socket.h login/login.c char/char.c map/clif.c map/chrif.c)
	ソケットを閉じる時の処理の流れが変更になります。今までソケットを閉じる場合は、
	まずsession[fd]->eof を真にした後、パーズルーチン内で後処理（メモリ解放など）
	していました。ですが、close(fd); が２重に実行されてサーバーが落ちるなどの
	バグが発生していたり、処理の流れがつかみにくいといった理由から、socket.c 内部で
	全て処理するように変更しました。ソケットを閉じる時の主な流れは次の通りです。

	1. ソース内からsession[fd]->eof = 1; をする
	2. socket.c 内からsession[fd]->destruct() が呼ばれる
	3. メモリの解放＆後処理(socket.c delete_session内部)

	close(fd) は、session[fd]->eof = 1; に置き換えました(#define)。
	また、delete_session() を明示的に呼ぶ必要はありません。

・マップ鯖分配時のアイテムdupe問題修正(map/map.c map/pc.c map/chrif.c)
	ソケット切断時に倉庫データのキャッシュを消すように変更
	２重ログイン時にマップサーバーが違った場合にも切断できるように修正

・古いバージョンでログインした時にmap鯖が落ちるバグを修正(map/clif.c)
	clif_parse() 内部

	if(packet_db[cmd].len==0) {
	-> if(cmd<MAX_PACKET_DB && packet_db[cmd].len==0) {

・gcc でコンパイルした時にtimer.c でwarning が出たのを修正(common/timer.c)
	timer.c:116: warning: `check_timer_heap' defined but not used

	(src/common/)
		socket.c	ソケットのデストラクタ処理を追加
		socket.h	ソケットのデストラクタ処理を追加
		timer.c		warning 修正

	(src/map/)
		clif.c		ソケットのデストラクタ処理を追加
		chrif.c		ソケットのデストラクタ処理を追加
		map.c		マップ鯖分配時のアイテムdupe問題修正
		pc.c		マップ鯖分配時のアイテムdupe問題修正

	(src/char/)
		char.c		ソケットのデストラクタ処理を追加

	(src/login/)
		login.c		ソケットのデストラクタ処理を追加

--------------------
//1082 by lizorett (2004/12/18) special thanks to 名無し様@ｇ＠ｍｅ
・白刃取りをボスに無効に変更
・1079の変更部分にNULLチェックを追加
・カードの効果が乗らないスキルにエンチャントデッドリーポイズン効果が乗らないよ
 うに変更
・エンチャントデッドリーポイズンの効果に左手が載らないように変更
・サクリファイスを実装
・ストームガストのノックバックがスキル指定位置を中心とするよう変更
・スキルの射程距離から1セル離れた場所を指定してスキルを使うと何も起こらない問題
 を修正
・経験値の配分を修正(ダメージを与えた人がいない場合や、毒ダメージがある場合に経
 験値が少なくなっていた)
・装備していない箇所へのストリップスキルが失敗するよう変更
・パッチアップスレッド Part 6？の>>116,>>125,>>126 のファイルを念のためマージ

	(db/)
		skill_db.txt, skill_cast_db.txt, skill_require_db.txt
					- サクリファイスの記述を修正/追加
	(src/map/)
		battle.c	- エンチャントデッドリーポイズンの変更
					- サクリファイスの実装
					- ストームガストのノックバック方向を変更
					- 白刃取りをボスに無効に変更
		skill.h		- SC_SACRIFICEを追加
		skill.c		- サクリファイスの実装
					- skill_castend_damage_id()のMG_FROSTDIVER/MG_STONECURSEに
					 NULLチェックを追加
		mob.c		- 経験値の配分を修正
		script.c	- 個別に出されていたファイルをマージ(>>125)
		npc.c		- 個別に出されていたファイルをマージ(>>126)
	(src/common)
		core.c		- 個別に出されていたファイルをマージ(>>116)

--------------------
//1081 by End_of_exam
・「ループ構文の方も実装してください」という要望を貰ったので、
　for , while , do - while 構文を導入。個人的に余り需要は無いと思うのですが…。

・elseが完全に解析できてなかったバグを修正。
・switch のbreak; が場所によってはコンパイルエラーになるバグを修正。

	(src/map/)
		script.c : 構文を拡張。色々整理。

	(doc/)
		script_ref.txt : 上の修正に合わせて変更。

--------------------
//1080 by End_of_exam

・スクリプトを if - else if - else 構文 , switch 構文に対応させました。
　多重ネストが可能ですので、今までより見やすいスクリプトが書けると思います。
　if(aa) { aaa(); } else if(bb) { cc; if(dd) { ee() } else { ff(); } }
　それに伴い、__ から始まる変数やラベルを用いると、不都合が生じる可能性があります。

・スクリプトに新しい関数(select関数・menu命令の関数版)を追加しました。

	(src/map/)
		script.c : 構文を拡張
		npc.c : npc_perse_script の修正( { , } のネストに対応 )

	(script/)
		npc/town/npc_town_alberta.txt : 一カ所 goto が抜けてたので修正
		sample/npc_debug_pota.txt     : switch , select を使って書き直し
		                                (デバッグに使わせて貰いました)

	(doc/)
		script_ref.txt : 上の修正に合わせて変更
--------------------
//1079 by Yuuki
・石化中にストーンカースを使うと石化解除
・FDでスキル追加効果を使うと氷化中ダメージ判定で先に割れてもう一度氷化判定がくるのでスキル追加効果つかわず 
・BBで睡眠石化氷化が割れないバグの修正(独自のダメージ判定使ってたので消して正規のダメージ判定に戻した) 
・ディレイ0のスキルにadelay/2追加(G鯖でTS使って検証した結果最もこれが近かった通常攻撃よりはやかったので) 

	(src/map)
		skill.c

--------------------
//1078 by End_of_exam

・Visual C++ 6.0 / bcc32 でコンパイル出来るように修正(別途zlib.dll が必要)
・1074は欠番にします。色々とご迷惑をかけた事をお詫びします。

＊＊　注意　＊＊
	今回のバージョンの完全な動作確認はしていません（人柱版扱いにしてください）。
	本格的な運用に踏み切る前には、必ず動作確認をするようにしてください。
	場合によっては、コンパイル出来ない、不正な動作になる…等々の問題が起こるかも
	しれませんが、そのときは、騒がず、慌てずに、ネ申の降臨を待つようお願いします。

＊＊ お願い ＊＊
	このパッチを完全版にしてくれる方、使用感レポートを投稿してくれる方を募集します。
	パッチを公開するついでに、大量のwarning を修正してくれたら嬉しいな～、と思ってみたり。

   (/)
		bcc32_make.bat , bcc32_clean.bat
			bcc32 でコンパイル / クリーン　を簡単にするためのバッチファイル。

		athena.dsp , athena.dsw , src/login/login.dsp , src/char/char.dsp , 
		src/map/map.dsp
			Visual C++ 用のプロジェクトファイル & ワークスペース

	(src/)
		コンパイル出来るように色々修正。

	(src/common/timer.c)
		独自の手抜きアルゴリズム（２分ソート）を採用したバージョン。

--------------------
//1077 by sylpheed
・サーバースナップショット
・下記二つを取り込み
質問スレッド Part14-41 Plalaさん
バグ報告スレッド part7-68 ...さん

1074は取り込んでいません。

--------------------
//1076 by mare
・韓国公知の通りアリスとジルタスのエサの変更。
・GMのアブラカタブラ専用スキルのフラグが消えてたので復活。
・ブラックスミスギルド員にプリーストの場合のセリフを追加。
・クラスチェンジで全ての指定IDボスが出るように（なってるといいなぁ）
	(db)
		pet_db.txt
	(conf)
		battle_athena.conf
	(script/npc/job)
		npc_job_10blacksmith.txt
	(src/map)
		mob.c
--------------------
//1075 by kag
・弓手用の指貫の効果実装の布石
・効果がはっきりとしないのでitem_dbの修正はいれていません。
・1075番でいいのかなぁと思ったり。

    (db)
		const.txt
			bWeponAtk=1073とbWeponAtkRate=1074を追加

    (src/map)
        battle.c
			int battle_get_baseatk()修正
			static struct Damage battle_calc_pc_weapon_attack()修正
		map.h
			int weapon_atk[16],weapon_atk_rate[16];
			SP_WEPON_ATK,SP_WEPON_ATK_RATE, // 1073-1074を追加
        pc.c
			memset(sd->weapon_atk,0,sizeof(sd->weapon_atk));
			memset(sd->weapon_atk_rate,0,sizeof(sd->weapon_atk_rate));を追加
			
			pc_bonus2() 修正
	(doc)
		item_bonus.txt
			bWeponAtk,bWeponAtkRate,bHPDrainValue,bSPDrainValue　追加。

--------------------
//1073	by LP@@
・アコスキルの一部及びサイトスキルの修正(日本には今月末に来る筈？@2004/12/06)
速度減少は移動速度減少量が、シグナムクルシスは成功率、DEF減少量の増加量が
はっきりしなかったので弄っていません。
   (src/map)
		battle.c
			デーモンベイン、ディバインプロテクション計算式を修正。
		skill.c
			ルアフ、サイトの有効範囲を修正（両方10x10→ルアフ5x5,サイト7x7）。

   (db)
		skill_cast.db
			アクアベネディクタの詠唱及びディレイを修正。

//1072	by kag
・転生スキルを中心に修正
    (src/map)
		battle.c
			魔法力増幅をスキルレベル*5%に修正。
			矢撃ちに矢のATKが乗らなくなるように修正。
			オーラブレードの追加ダメージを100に修正。
			バーサークの与ダメを２倍になるように修正。
			ヘッドクラッシュの計算式を修正。
			スパイラルピアースの計算式だけ修正。
			プレッシャーの計算式を修正。
			連柱崩撃の計算式を修正。
			ソードリジェクトの反射率をスキルレベル*15%に修正。
			アローバルカンの計算式を修正。矢の属性が乗るように修正。
			ファルコンアサルトの計算式を修正。頑強フラグで１ダメになるように修正。
			ブリッツビートを頑強フラグで１ダメになるように修正。
			デモンストレーションを必中、カードを乗らないように修正。
			アシッドテラーを防御無視、必中、無属性、カードを乗らないように修正。
		skill.c
			魔法力増幅に0.7秒の固定詠唱追加。
			LP@@ さんのアスムキリエ重複不可を追加。

	(db)
		skill_db.txt
			狂気孔を詠唱妨害できるように修正。
		skill_cast_db.txt
			プレッシャーの詠唱、ディレイを追加。
			猛虎硬派山のディレイを追加。
			ライフ置き換えのディレイを追加。
			ファルコンアサルトの詠唱、ディレイを追加。
			ウインドウォークのディレイ、持続時間を変更。
			アローバルカンの詠唱、ディレイを追加。
			クリエイトデッドリーポイズンのディレイを追加。
		skill_require_db.txt
			オーラブレードの消費SPを修正。
			コンセントレーションの消費SPを修正。
			魔法力増幅の消費SPを修正。	
			ナパームバルカンの消費SPを修正。
			プレッシャーの消費SPを修正。
			サクリファイスの消費SPを修正。
			猛虎硬派山の消費SPを修正。
			リジェクトソードの消費SPを修正。

//1071 by ICO
・npc_job_09wizard.txt,npc_job_16sage.txtを修正
・取り巻きが召喚系スキルを使用する際の挙動を修正

    (src/map)
		mob.c
			取り巻きの召喚系スキルを規制するタイミングを変更
		skill.c
			召喚系スキルの発動条件を修正

//1070 by sylpheed
・ドロップ率のレートごとの調整を追加
  ドロップ設定1～9 10～99 100～999 1000～10000で個別に倍率と最低/最高値の設定が可能です
・@weather 0 が動かないのを修正
・転生二次職のHP/SP25％上昇実装
ドロップ倍率はeAthenaのアイテム種類毎の設定を移植しても良かったのですが
種類毎だとレートの開きが大きい場合があり、あまり意味がないため
このような形でレート詳細設定を追加してみました。
mob_dbを弄るより楽にドロップ率の変更ができると思います。

weather 0が動かないのは理由が良くわからなかったので
動くように修正するついでに、雨フラグ消去時に
虹が出るように追加してみました。

転生のHP/SPに関しては、転生していても二次職になってない場合
増加はしないようなので、そのように実装してみました。
wedding_modifydisplay: yes の時にドレス/タキシードを装備解除すると
増加分が消えてしまいます･･･
わかる方いたら修正お願いしますorz

	(src/map)
		atcommand.c
			@weather関連の修正(case 0を実行するように修正)
			雨フラグを消した場合虹が出るようにしてみました(JP蔵で使えるか不明)
			(マップ移動しないと天候が消えないのはragexeの仕様かな？)
		battle.c
		battle.h
		mob.c
			item_rate_details関連を追加
		pc.c
			転生二次職時のHP/SP最大値25%増加を追加
			転生していても二次職でない場合はHP/SP増加は無しのようです
	(conf)
		help.txt		修正
		msg_athena.conf	112追加

	(doc)
		conf_ref.txt	修正
		help.txt		修正

//1069 by lizorett (2004/11/26) special thanks to 名無し様@ｇ＠ｍｅ
・メルトダウンの実装
・魔法力増幅が詠唱のある魔法で有効にならなかった問題を修正
・map-serverがダウンする問題の修正(質問スレッド Part13 >>55)
・map-serverに不正IDでログインするとchar-serverがダウンする問題の修正
・サフラギウムが無詠唱スキルで取り消されない問題を修正
・重複して猛毒状態とならないよう修正
・マグナムブレイクのダメージを修正
・バーサク中にHP/SP吸収、毒などのダメージを受けるよう修正
・月光剣のSP吸収量が増加していく問題を修正(暫定)
・@helpにキーワード検索機能を追加

	(src/map)
		battle.c	- マジックパワーによるMATK増加の実装
					- 武器鎧破壊をpc_break_equip()に書き換え
					- HP/SP吸収を修正
		map.h		- 装備破壊の定義を追加
		pc.c		- pc_break_armor()/pc_break_weapon()を、新規の装備破壊の
					関数pc_break_equip()に統合
					- 魔法力増幅のコードを削除(battle.cで行う)
					- HP/SP吸収関係の変数の初期化を追加
		pc.h		- 関数定義を修正
		skill.c		- メルトダウンによる装備破壊を実装
					- 魔法力増幅に関する修正
					- 重複して猛毒状態とならないよう修正
		clif.c		- map-serverがダウンする問題の修正
		atcommand.c	- @helpにキーワード検索を追加("@help jobchange"など)
	(src/char)
		char.c		- char-serverがダウンする問題の修正
	(db)
		const.txt	- bUnbreakableHelm, bUnbreakableShield追加
		skill_cast_db.txt
					- メルトダウンの状態異常時間を設定

//1068 by huge
・atcommandをいくつか追加と、eAthenaからいくつか移植。(help.txt参照)
・非GMキャラが@で始まる発言をした時、そのまま表示するようにした。(GMレベル0のコマンドは実行)
・天気コマンドに合わせて、mapflag設定できるように。
・scriptをいくつか追加と、eAthenaから一つ移植。
・初期HP倍率とSP倍率をbattle_athenaで設定できるようにした。
・全キャラが、GMには位置・HPを通知させるようbattle_athenaで設定できるようにした。
・ドクロドロップについて、battle_athenaで設定できるようにした。
・GMのアイテムドロップや交換についてGMレベルを制限できるようにした。
・ディテクティングスキル修正。
※デフォルトの日本クライアントだと、雨で落ちます。

	(conf/)
		atcommand_athena.conf	修正
		battle_athena.conf	修正
		help.txt	修正
		msg_athena.conf	修正

	(doc/)
		help.txt	修正
		script_ref.txt	修正
		conf_ref.txt	修正

	(src/map/)
		atcommand.c
		atcommnad.h
			guild.h party.hを読む様に修正
			is_atcommand() 修正
			atcommand() 修正
			※移植・新規コマンドについては help.txt を確認して下さい。
		battle.c
		battle.h
			hp_rate,sp_rate,hp_meter,bone_drop 追加
		clif.c
		clif.h
			clif_spawnpc() 修正
			clif_hpmeter() 追加
		guild.c
		guild.h
			guild_searchname() 追加
			guild_searchname_sub() 追加
		map.h
			天候に関するflag追加
		npc.c
			npc_parse_mapflag() 修正
		party.c
		party.h
			party_searchname() 追加
			party_searchname_sub() 追加
		pc.c
		pc.h
			pc_calcstatus() 修正
			pc_damage() 修正
			pc_walk() 修正
			pc_can_drop() 追加
		script.c
			gmcommand	移植追加
			dispbottom		追加
			getusersname		追加
			recovery		追加
			petinfo		追加
			checkequipedcard		追加
			getexp	削除(setを使うようにして下さい)
		skill.c
		skill.h
			skill_castend_pos2() 修正

	＋Athena雑談スレッドPart7 >>54 もっさりさんの分
	記述漏れがあったらすいません。
--------------------
//1067 by kai
・メディテイティオによるヒール回復量増加の効果を修正
・アドバンスドカタール研究の修正
・PvP時におけるアスムプティオの効果を修正

	(src/map)
		skill.c
			2120行　heal += heal*(skill*2)/100; //メディテイティオの修正

		battle.c
			1723行　damage += dmg*(10+(skill * 2))/100; //アドバンスドカタール研究の修正

			2598～2599、3580～3582行　アスムプティオの修正
				if(map[target->m].flag.pvp)としダメージ計算を2/3に修正

--------------------
//1066 by lizorett (2004/11/17) special thanks to 名無し様@ｇ＠ｍｅ
・デッドリーポイズン作成実装
・エンチャントデッドリーポイズン実装
・ソウルブレイカー実装
・battle_weapon_attackで未初期化の値が戻されるバグ修正
・他使用SPの変更等の細かい修正

	(db)
		const.txt       - 猛毒(SC_DPoison)を追加
		item_db.txt     - 緑ハーブ,緑ポーション,万能薬に猛毒を治す効果を追加
		produce_db.txt  - デッドリーポイズンの材料の定義を追加
		skill_cast_db.txtエンチャントデッドリーポイズン、ソウルブレイカー
						の定義を修正/追加
		skill_db.txt    - ソウルブレイカー/エンチャントデッドリーポイズン/
						デッドリーポイズン作成の定義を修正
						- ナパームバルカンのヒット数修正
		skill_require_db.txt
						- エンチャントデッドリーポイズンが毒薬の瓶を使用する
						ように修正
						- ソウルブレーカー/メテオアサルトの使用SPを修正
	(src/map)
		battle.h		- battle_config.cdp_rateを追加
		battle.c        - ソウルブレーカー実装
						- メテオアサルトにカード効果がかからないよう変更
						- 4213行の条件で未初期化の値が戻されるバグ修正
						- battle_config.cdp_rateを追加
		skill.h			- デッドリーポイズン作成の材料が7つなので、テーブル
						を追加。材料の最大数をdefineで変更できるよう修正
						- SC_DPOISON(182)/SC_EDP(183)を追加
		skill.c			- 猛毒効果を追加
						- デッドリーポイズン作成を実装
--------------------
//1065 by End_of_exam

・サーバーゾンビ化に暫定対処
・calc_index(path.c)の不都合を修正

	(common/)
	timer.c
		TIMER_MIN_INTERVEL(タイマーの最小インターバル）を追加しました。
		それに伴い、do_timer() の戻り値の最低を 10ms から 50ms に変更しました。

		selectが10ms以内に終わらない環境だと、モンスターを大量召還した時などに、
		クライアントからのパケットに反応しなくなる模様です。
		ローカルテスト（モンスター大量召還）をしてみたところ、25msに変更した時点で
		改善しましたが、環境によっては50msでも不十分かもしれません。
		その場合は、TIMER_MIN_INTERVELを増やして様子を見てください。

		同様の現象として、多数のクライアントが接続するとサーバーが反応しなくなる
		というのがありますが、このパッチによってある程度改善されるかもしれません。

	(map/)
	map.h : MAX_WALKPATH
		path.c 内の calc_indexが不都合を起こす(48*48-1 = 1000 1111 1111(b)) ため、
		MAX_WALKPATHを 48 から 32 に変更しました。(32*32-1 = 0011 1111 1111(b))

		#define calc_index(x,y) (((x)+(y)*MAX_WALKPATH) & (MAX_WALKPATH*MAX_WALKPATH-1))
--------------------
//1064 by nameless
・GCC 3.3.0/3.3.1が内包する0/0=変数最大値の問題対策(int:65535/long:4294967294)
・魔法防御・防御・属性防御が正しく適用されない問題を修正
・ナパームバルカンを単体魔法のコードに修正し、呪い効果を実装

	(src/map)
		skill.c
			442～448行 不等号[<]を[>]に修正
			512～519行 不等号[>]を[<]に修正
			529～538行 コンパイラバグの訂正方法と例/*～*/の部分を
			           gccのバージョンに合わせて生かしたり殺したり
			           してください。3.3.2からは直っているようです
			1857行     ナパームバルカンを追加
			660～664行 ナパームバルカンの呪い効果を追加
	(src/char)
		char.c
			修正しきれていなかった部分を修正しました。

--------------------
//1063 by 七誌
・月光剣の効果実装
・左手カードによる、武器攻撃時のHP/SP吸収効果が右手に影響する様に修正(本鯖仕様)
  (left_cardfix_to_rightがyesの時のみ。noだと従来通り)

    (db)
        const.txt
            bHPDrainValue=1071とbSPDrainValue=1072を追加
        item_db.txt
            月光剣のEquipScriptにbonus2 bSPDrainValue,100,3を追加

    (src/map)
        battle.c
            battle_weapon_attack() 修正
        map.h
            short hp_drain_value,sp_drain_value,hp_drain_value_,sp_drain_value_;を追加
            SP_HP_DRAIN_VALUE,SP_SP_DRAIN_VALUE, // 1071-1072を追加
        pc.c
            pc_bonus2() 修正

--------------------
//1062 by nameless
・パケット送受信での誤送信修正
・バグ報告スレッド part7 からの取り込み・修正 >> 56,57,58
・fix1059の取り込み
・ハイウィズのネイパームバルカンの実装(eA取り込み)
・CPU最適化コードの修正(athlon系)・追加(玄人箱/Linux Zaurus)

    (/)
		Makefile
			athlon系最適化の修正
			玄人箱(Kuro-Box 200MHz版[PPC 603x])
			玄人箱(Kuro-Box 266MHz版[PPC 604x])
			Linux Zaurus (SL-C7xx)
			の最適化コードを記述

    (db)
		skill_cast_db.txt
			400,500,1200:1600:2000:2400:2500,0,0 //HW_NAPALMVULCAN#ナパームバルカン#
    (src/map)
		clif.c
			clif_parse() 修正
    (src/char)
		char.c
			1367行 cmd = RFIFOW(fd,0);追加
    (src/map)
		skill.c
			switch(skillid)にcase HW_NAPALMVULCAN:以下20行ほど追加
--------------------
//1061 by lizorett (2004/11/9)
・不正なギルドスキルパケットを受けた場合にmap/charサーバが落ちる現象を修正
・不正なパケットを受けた場合にcharサーバが落ちる現象を修正
・結婚したキャラを削除するとcharサーバが落ちる現象を修正
・青箱を一つだけ持った状態で青箱から青箱がでると、青箱が消えるたように見える
 (リログすると見える)問題を修正
・スナッチャーのスキル失敗を表示しないようにできる設定を追加
・デボーションのレベル制限(10レベル差)を変更できる設定を追加

    (src/common)
		mmo.h
			ギルドスキルのenumを追加(src/map/skill.hから移動)
    (src/map)
		skill.h
			ギルドスキルのenumを削除(src/common/mmo.hに移動)
		guild.c
			guild_checkskill スキルの範囲チェックを追加
			guild_skillup スキルの範囲チェックを追加
		skill.c
			skill_additional_effect display_snatcher_skill_failの処理を追加
			skill_castend_nodamage_id devotion_level_differenceの処理を追加
		battle.c
			display_snatcher_skill_fail,devotion_level_differenceの初期化を追加
	(src/char)
		int_guild.c
			guild_checkskill スキルの範囲チェックを追加
			mapif_parse_GuildSkillUp スキルの範囲チェックを追加
		char.c
			parse_char 不正なパケットを受けた場合にはダンプするよう修正
			char_divorce i<MAX_INVENTORYをj<MAX_INVENTORYに変更
	(conf)
		battle_athena.conf
			display_snatcher_skill_fail,devotion_level_differenceを追加
--------------------
//1060 by mosya
・モンスターがスキル詠唱中に座標ズレを起こす問題を修正
・MOBのAI変更。前衛がタゲを取っているのに、後衛に攻撃をしにゆくのを修正
	(src/map)
		mob.c
			mob_ai_sub_hard() 変更
			mobskill_castend_id() 変更
			mobskill_castend_pos() 変更
			mobskill_use_id() 変更
			mobskill_use_pos() 変更
--------------------
//1059 by SPDFMember
・eAthenaを参考にリロードDB系統を追加しました。
	(conf/)
		atcommand_athena.conf
			reloaditemdb
			reloadmobdb
			reloadskilldbを追加
		help.txt
			reloaditemdb
			reloadmobdb
			reloadskilldbの説明を追加
		msg_athena.conf
			89
			90
			91を追加		
	(src/map)
		atcommand.c
			reloaditemdb
			reloadmobdb
			reloadskilldbを追加。
		atcommand.h
			AtCommand_ReloadItemDB,
			AtCommand_ReloadMobDB,
			AtCommand_ReloadSkillDB,を追加
		itemdb.c
			static int itemdb_readdb(void);追加
		itemdb.h
			void itemdb_reload(void);追加
		mob.c
			void mob_reload(void)追加
		mob.h
			void mob_reload(void);追加
		skill.c
			void skill_reload(void)追加
		skill.h
		 	void skill_reload(void);追加
--------------------
//1058 by lizorett
・ギルド倉庫アイテムがMAX_STORAGE個以上あると取り出せないものがある問題を修正
	(src/map)
		clif.c
			clif_parse_DropItem item_index/item_amountの範囲チェックを削除 
								(pc_dropitem でチェックする)
			clif_parse_MoveToKafra item_amountのチェックを削除
								(storage_*storageaddでチェックされている)
			clif_parse_MoveFromKafra item_index/item_amountのチェックを削除
								(storage_*storagegetでチェックされている)
		pc.c
			pc_dropitem n/amountの範囲チェックを追加
--------------------
//1057 by BDPQ銀
・ウォーターボールのダメージを本鯖告知値→実測値に修正。
  [ MATK+SkillLv*30 → MATK*(1+SkillLv*0.3) ]

	(src/map)
		battle.c
			battle_calc_magic_attack 変更 [ 3848行目あたり ]

--------------------
//1056 by robert
順番を検査するのは不法かどうか
	(src/map)
		clif.c	
			clif_parse_DropItem
			clif_parse_MoveToKafra
			clif_parse_MoveFromKafra
--------------------
//1055 by Nameless
・各種CPUに対するコード最適化オプションを設定(全27種) ※GCC 3.3.1 準拠
・Pentium 3のオプションがSSE2になっていたのを訂正
・稀枝さん、robertさんのPacket情報の取り込み(お二人に感謝)

	(/)
		makefile
			i486/586/p54c/mmx/P3/P4/Cele
			k6/k6-2/k6-3/athlon系
			Via C3(Eden)
			PowerPC/G4系

	(src/map/)
		clif.c
			clif_parse_MoveToKafra{}
			コード取り込み

--------------------
//1054 by Nameless
・Cygwinでコンパイルしたathenaが接続53～58人付近で接続不能になるバグを解消

	(src/common/)
		socket.h
			#ifdef CYGWIN
			#undef FD_SETSIZE
			#define FD_SETSIZE 4096
			#endif
		以上の行を削除
	(/)
		makefile
			OS_TYPE = -DCYGWIN
				↓
			OS_TYPE = -DCYGWIN -DFD_SETSIZE=4096
		に修正

		#optimize for Athlon-4(mobile Athlon)
		#CFLAGS += -march=athlon -mcpu=athlon-4 -mfpmath=sse

		#optimize for Athlon-mp
		#CFLAGS += -march=athlon -mcpu=athlon-mp -mfpmath=sse

		#optimize for Athlon-xp
		#CFLAGS += -march=athlon -mcpu=athlon-xp -mfpmath=sse

		#optimize for pentium3
		#CFLAGS += -march=i686 -mcpu=pentium3 -mfpmath=sse -mmmx -msse2

		各種CPUの最適化を追加gcc3.1以上を使っている場合は#を外し
		て最適化を行うことができると思います。

	※makefileでセットしておかないと、コンパイルの際に一部
	  FD_SETSIZEが小さくなってしまうバグ(?)があるらしく、接続人数が
	  60人弱で接続不能が発生してしまいます。

--------------------
//1053 by TEILU
・パケットパーサーをタイマー呼出しに変更できる設定の追加
・精錬の可否判定でＤＢにどのような設定を入れてもアクセサリは
  精錬できなかった不具合を修正

	(conf/)
		map_athena.conf
			packet_parse_time: 0 追加
	(src/common/)
		core.c
			packet_parse_time 追加
			main() 変更
		socket.c
			parsepacket_timer() 追加
		socket.h
			parsepacket_timer() 追加
	(src/map/)
		map.c
			packet_parse_time 追加
			map_config_read() 変更
		script.c
			buildin_getequipisenableref() 変更

--------------------
//1052 by
居場所つきで検索コマンド追加。
	GMコマンド追加
	@who+	居場所つき検索

	(conf/)
		atcommand_athena.conf
			who+: 1 追加
	(/src/common/)
		version.h
			Ver 1051->1052 変更
	(/src/map)
		atcommand.c
			ATCOMMAND_FUNC(whop);  追加
			{ AtCommand_WhoP,"@who+",0, atcommand_whop }, 追加
		atcommand.h
			AtCommand_WhoP, 追加
--------------------
//1051 by Plala
・ペコペコ騎乗時MAX Weightを増やせるようにしました。
battle_athena.confで設定可能です。

	(conf/)
		battle_athena.conf
			riding_weight 追加
	(map/)
		battle.c
			battle_config.riding_weight 追加
		battle.h
			int riding_weight; 追加
		clif.c
			clif_parse_RemoveOption 変更
		pc.c
			int pc_calcstatus 変更
				1415にsd->max_weight +=battle_config.riding_weight; を追加
	(common/)
		version.h
			Ver 1050->1051 変更
--------------------
//1050 by code
・各種天候操作コマンドの見直し
※いちいち@misceffectで呼び出さずに簡単に天候操作を行えるように
※クライアント依存は落ち葉ではなく雨でした、間違いました(^-^;

	(src/common/)
		version.h
			Ver 1049->1050 変更
	(/src/map)
		atcommand.c
			AtCommand_Rain{} 変更
			AtCommand_Snow{} 変更
			AtCommand_Cherry{} 変更
			AtCommand_Fog{} 変更
			AtCommand_Maple{} 変更

--------------------
//1049 by code
・霧、落葉の2つの天候操作コマンドを追加しました。
※落ち葉に関してはクライアント依存なのでjROだと落ちるかも…

	GMコマンド追加
	@fog       霧
	@maple     落葉

	(conf/)
		atcommand_athena.conf
			fog: 1 追加
			maple: 1 追加

		msg_athena.conf
			87: 霧が立ち込めました。 追加
			88: 落ち葉が降ってきました。 追加
	(/src/common/)
		version.h
			Ver 1048->1049 変更
	(/src/map)
		atcommand.c
			ATCOMMAND_FUNC(fog);  追加
			ATCOMMAND_FUNC(maple);  追加
			{ AtCommand_fog,"@fog",0, atcommand_fog }, 追加
			{ AtCommand_maple,"@maple",0, atcommand_maple }, 追加

		atcommand.h
			AtCommand_Fog, 追加
			AtCommand_Maple, 追加

--------------------
//1048 by code
・雨、雪、サクラ吹雪の3つの天候操作コマンドを追加しました。
※衝動的に追加してしまったのでバグがあるかも知れません(汗

	GMコマンド追加
	@rain       雨
	@snow       雪
	@cherry     サクラ吹雪

	(conf/)
		atcommand_athena.conf
			rain: 1 追加
			snow: 1 追加
			cherry: 1 追加

		msg_athena.conf
			84: 雨が降り出しました。 追加
			85: 雪が降り出しました。 追加
			86: サクラ吹雪を降らせます。 追加
	(/src/common/)
		version.h
			Ver 1047->1048 変更
	(/src/map)
		atcommand.c
			ATCOMMAND_FUNC(rain);  追加
			ATCOMMAND_FUNC(snow);  追加
			ATCOMMAND_FUNC(cherry);  追加
			{ AtCommand_rain,"@rain",0, atcommand_rain }, 追加
			{ AtCommand_snow,"@snow",0, atcommand_snow }, 追加
			{ AtCommand_cherry,"@cherry",0, atcommand_cherry }, 追加

		atcommand.h
			AtCommand_Rain, 追加
			AtCommand_Snow, 追加
			AtCommand_Cherry, 追加

--------------------
//1047 by SVN
・製造アイテムの製作者の名前を引くmap←→charのパケットが間違っていたのを修正
・二重ログインをした時にchar-serverが落ちる可能性があったのを修正
・ガーディアンをGv時間外に殴れた、ID指定のスキルが当たった、skill_unitなスキル攻撃が当たったのを修正
・グラフィティのRangeをとりあえず3にしてみた
	(db/)
		skill_db.txt
	(char/)
	char.c
		parse_frommap()
	(map/)
	clif.c
		clif_parse_ActionRequest() 変更
		clif_parse_UseSkillToId() 変更
	mob.c
		mob_gvmobcheck() 追加
	mob.h
		mob_gvmobcheck() 定義追加
	skill.c
		skill_attack() 変更

--------------------
//1046 by SVN
※db/packet_db.txtは2004-09-06aSakexe用なので、jROクライアントで使用する際は「//jROはここまで」以下をコメントアウトするか削除してください

・変更点が多いので詳細はファイル、関数ごとに記述しています
・1045にそのまま上書きしても動作しますが、使われないファイルがいくつか残ります
・script/以下は名前が変更されているファイルが大量にあるのでscript/の変更点を参考にしてください
・スクリプト用の関数や命令が増えたり仕様が変更されているものがあります
	基本的には互換性があるはずですが、getgdskilllv()だけは第二引数を数字ではなくスキル名(GD_APPROVALなど)に置き換える必要があります
	getgdskilllv()が使われている/script/npc/gvg_big5/*.* は中国語で必要ないので削除してください
	追加や変更に関しては概ねscript_ref.txtに反映してありますが、詳細はscript.cの変更点を読んでください
・追加された@コマンドについては@helpとatcommand.cの変更点を読んでください
・追加された設定はconf_ref.txtを参照してください
・記述漏れの変更点もあるかもしれません

追加したファイルは「A」
変更したファイルは「C」
削除したファイルは「D」
移動したファイルは「M」
	/
		C athena-start
			./conf/import 以下を起動時に自動作成するように変更
			seqコマンドを使わないようにしてFreeBSDでも動作するように変更
			start
				すでにAthenaが起動している時は何もしないように変更
				execで ./ がダブっていたので削除
			stop
				FreeBSDでもシェルに戻るように変更
			kill
				同上
		C Makefile
			PACKETDEF に PACKETVER=6 を追加
			FreeBSDを判定して make と gmake を切り替えるように変更
			CFLAGS を分解して弄りやすいように変更
		C start
			1行目のシェル指定が正しくなかったのを変更
			すでにAthenaが起動している時は何もしないように変更
			起動確認の条件式を athena-start と同じ物に変更
		bin/tool/
			シェルスクリプトの改行コードを CRLF から LF のみに変更
			C getlogincount
				ログインバージョンを $loginversion として変更できるように変更
			C ladmin
				アカウント名に「-」を使えるように変更

		conf/
			D import/
				配布物から削除(athena-start start で自動作成される)
			C atcommand_athena.conf
				shuffle maintenance misceffect 追加
			C char_athena.conf
				default_map_type default_map_name 追加
			C help.txt
				説明追加
			C login_athena.conf
				login_version login_type 追加
			C map_athena.conf
				npc map 色々変更
			C mapflag.txt
				最新版に更新
			C msg_athena.conf
				81以降追加
			C water_height.txt
				最新版に更新
		db/
			C castle_db.txt
				ギルド解体時に発生する OnGuildBreak イベントのために <Event_Name> 追加
			C const.txt
				GvGの開始時間等を設定できるように追加
				マップフラグ mf_notrade mf_noskill 追加
				パラメータ PartnerId Cart 追加
				ボーナス bBreakWeaponRate bBreakArmorRate bAddStealRate bUnbreakableWeapon bUnbreakableArmor 追加
				ステータス変化 SC_WEDDING 追加
				スクリプト命令 getgdskilllv 用に GD_APPROVAL など追加
			C exp.txt
				転生二次職がBaseLv12になるときに必要な経験値を41→481に修正
			C item_db.txt
			C mob_db.txt
			C mob_skill_db.txt
			C skill_cast_db.txt
			C skill_db.txt
			C skill_require_db.txt
			C skill_tree.txt
				最新版に更新
			A packet_db.txt
				パケット定義ファイル追加
		doc/
			C client_packet.txt
				新しく判明したパケットをいくつか追加
			C conf_ref.txt
				新しく追加した設定の説明を追加
			C db_ref.txt
				skill_cast_db.txt の list_hp_rate list_sp_rate で負数を指定した時の挙動を追加
			C help.txt
				conf/help.txt 同様に変更
			C inter_server_packet.txt
				新規で追加したパケットを追加と実態とあっていなかった部分を修正
			C item.txt
				最新版に更新
			C item_bonus.txt
				新規で追加されたボーナスを追加
			C script_ref.txt
				新規命令の追加と既存命令の変更など
			C serverlink_packet.txt
				新規で追加したパケットを追加と既存で書かれていなかったパケットの追加
		script/
			mob/
				C npc_monster.txt
					最新版に更新
					ニブルヘイムのMobはnpc_parse_mob()の変更サンプルになってます
			npc/
				ほぼすべてのNPCをnpc_function.txtを使ったユーザー定義関数と複製などで書き換え
				移動したファイルに関しては移動後のファイルが含まれているので移動前のファイルのみ削除してください
				例) etc/npc_etc_cTower.txt は含まれているので npc_cTower.txt を削除
				M npc_cTower.txt
					→etc/npc_etc_cTower.txt
				A npc_function.txt
				M npc_pota.txt
					→../sample/npc_debug_pota.txt
				M npc_pvp.txt
				M npc_pvproom.txt
					2ファイル合体
					→etc/npc_etc_pvp.txt
				M npc_resetJ.txt
					→../sample/npc_debug_reset.txt
				A etc/
					A npc_etc_gefenia.txt
				C gvg/
					すべてev_agit_common.txtを使ったユーザー定義関数で書き換え
					すべて削除してからこのパッチを当ててください
					ギルド解散時に砦を放棄するためにev_agit_砦.txtにOnGuildBreakイベントを追加
					D test/
					A ev_agit_common.txt
					D ev_agit_event.txt
					D TEST_prtg_cas01_AbraiJ.txt
					D TEST_prtg_cas01_mob.txt
				D gvg_big5/
				C job/
					転職スクリプトの名前をnpc_job_[JOB番号][ジョブ名].txtに変更
					すべて削除してからこのパッチを当ててください
				C quest/
					M npc_event_arrow.txt
						→../../sample/npc_debug_arrow.txt
					A npc_event_hat2.txt
						ニブル同時実装の新頭装備スクリプトを追加
		src/
			calloc() realloc() を極力各型にキャストするように変更
			calloc() 後に memset() で \0 を埋めていたのを削除
			calloc() なのに(サイズ*個数,1)で指定していたのを(個数,サイズ)に変更
			メモリ確保をエラー処理をまとめた関数に書き換え
				malloc() → aMalloc()
				calloc() → aCalloc()
				realloc() → aRealloc()
			strcpy() を strncpy() に極力変更

			char/
				C char.c
					スペースでインデントされていたところをタブで統一
					A isGM()
					A read_gm_account()
						GMアカウントが必要になったので追加
					C mmo_char_tostr()
					C mmo_char_fromstr()
						nullpoチェック追加
					C count_users()
						必要ない{}を削除
					C char_delete()
						nullチェック追加
						削除メッセージをコンソールに表示するようにした
						削除時に接続しているキャラを切断するようmap-serverに通知(0x2b19パケット)するようにした
					C parse_tologin()
						C 0x2713
							char-serverメンテナンス設定時はGM以外入れないようにした
							接続数制限で最大値でもGMは接続できるようにした
						C 0x272a
							0x2730を0x272aに変更して番号を詰めた
							アカウント削除した時にキャラが一部消されない問題を修正
						A 0x272c
							受信時にlogin-serverに通知(0x2b15パケット)してメンテナンス状態になるようにした
					A char_erasemap()
						map-server切断時に他map-serverにマップの削除を通知(0x2b16パケット)するようにした
						これにより他map-server管轄のマップに移動しようとして、そのmap-serverが切断されていたら、
						pc_setpos()で存在しないマップということで移動しようとしなくなるので、
						クライアントが「永遠にお待ちください状態」にならないようになります
					C parse_frommap()
						map-server切断時にchar_erasemap()を実行するようにした
						map-server切断時に管轄のマップにキャラが残っていたら切断を他map-serverに通知(0x2b17パケット)するようにした
						C 0x2afc
							認証失敗時に char_dat[].mapip/mapport を 0 にするようにした
							認証成功時に char_dat[].mapip/mapport をmap-serverのIPアドレスとポートにするようにした
							認証成功時に他map-serverにキャラがログインしたことを通知(0x2b09パケット)するようにした
						C 0x2b02
						C 0x2b05
							コンソールへの出力にパケット番号を記述するようにした
						C 0x2b08
							パケット仕様を変更して account_id mapip mapport も通知するように変更
							map-serverに接続していない時はすべて 0 が入ります
						A 0x2b13
							map-server起動途中などでキャラがログインできないようにする server[].active フラグを操作する
							active = 0 でキャラはログインできずに切断される
							map-serverが起動を完了したときに active = 1 にするパケットが送られてくる
						A 0x2b14
							char-serverをメンテナンス状態にする char_maintenance フラグを操作する
							login-serverにも通知(0x272bパケット)してワールド選択画面でメンテナンス表示をする
							メンテナンス状態ではGM以外のユーザーはログインできません
						A 0x2b18
							キャラクターの切断を他map-serverに通知(0x2b17パケット)するようにした
					C search_mapserver()
						引数に struct mmo_charstatus *cd を追加
						cdが渡された＆探しているマップが接続しているmap-serverに無かったときに、最初に見つけたmap-serverの最初に見つけたマップに接続するようにした
						これは char_athena.conf default_map_type: 2 の時の挙動です
					C parse_char()
						C 0x65
							メンテナンス状態の時にGM以外を切断するようにした
							最大接続数が設定されていて最大接続数のときもGMは接続できるようにした
						C 0x66
							char-serverに接続しているmap-serverにlast_pointが見つけられなかったときに、
							default_map_type&1 の時は default_map_name に接続する
							default_map_type&2 の時は 最初に見つけたmap-serverの最初に見つけたマップに接続する
							それでも見つからない時は切断するようにした
							コンソールへの出力にパケット番号を記述するようにした
						C 0x2af8
							他マップに接続しているキャラ情報を通知(0x2b09パケット)するようにした
						C 0x187
							S 0187パケットを返信するようにした
					C check_connect_login_server()
						char_portをWFIFOLで送っていたのをWFIFOWに修正
						80と82の間が空いていたのを詰めてパケット長を86→84に変更
					C char_config_read()
						default_map_type default_map_name を読み込むようにした
					A gm_account_db_final()
						確保した gm_account_db のメモリを終了時に開放するようにした
					C do_final()
						inter.c など他ファイルの終了処理(do_final_*)を追加した
						exit_dbn() でdb用のメモリを開放するようにした
						接続されているmap-serverのセッションを削除するようにした
						do_final_timer()でtimerを終了させるようにした
					C do_init()
						server[].active = 0 で初期化
						read_gm_account() でGMアカウントファイルを読み込むようにした
				C char.h
					mmo_map_server に active フラグを追加した
				C int_guild.c
					C mapif_parse_GuildSkillUp()
						ギルドポイントを消費しないでギルドスキルを上げるために int flag を追加
					C inter_guild_parse_frommap()
						0x303C
							パケット定義を変更してflagを追加した
					A guild_db_final()
					A castle_db_final()
					A do_final_int_guild()
						終了時にメモリを開放するようにした
				C int_guild.h
					A do_final_int_guild()
						定義を追加
				C int_party.c
					A party_db_final()
					A do_final_int_party()
						終了時にメモリを開放するようにした
				C int_party.h
					A do_final_int_party()
						定義を追加
				C int_pet.c
					A pet_db_final()
					A do_final_int_pet()
						終了時にメモリを開放するようにした
				C int_pet.h
					A do_final_int_pet()
						定義を追加
				C int_storage.c
					A storage_db_final()
					A guild_storage_db_final()
					A do_final_int_storage()
						終了時にメモリを開放するようにした
				C int_storage.h
					A do_final_int_storage()
						定義を追加
				C inter.c
					パケット長定義を変更
					A mapif_parse_CharPosReq()
						0x3090パケットへの対応
						キャラの位置要求をmap-serverに通知(0x3890パケット)する
					A mapif_parse_CharPos()
						0x3091パケットへの対応
						キャラの位置要求をしたキャラに位置情報を通知(0x3891パケット)する
					A mapif_parse_CharMoveReq()
						0x3092パケットへの対応
						要求したキャラまで対象のキャラを飛ばす要求を通知(0x3892パケット)をする
					A mapif_parse_DisplayMessage()
						0x3093パケットへの対応
						キャラにメッセージを送信(0x3893パケット)する
					C inter_parse_frommap()
						各追加パケットへの対応を追加した
					A wis_db_final()
					A accreg_db_final()
					A do_final_inter()
						終了時にメモリを開放するようにした
				C inter.h
					A do_final_inter()
						定義を追加
				C Makefile
					A nullpo.o nullpo.h を追加
					A malloc.o malloc.h を追加
			common/
				C db.c
					A exit_dbn()
						終了時にメモリを開放するために追加
				C db.h
					A exit_dbn()
						定義追加
				A malloc.h
				A malloc.c
					メモリ確保関数のまとめ
				C mmo.h
					C mmp_charstatus
						mapip mapport 追加
					C guild_castle
						castle_event 追加
				C Makefile
					A malloc.o malloc.h malloc.c を追加
				C nullpo.h
					古いgccでコンパイルできるように\を削除
				C socket.c
					C recv_to_fifo()
						汎用性を高めるために read() を recv() に変更
					C send_from_fifo()
						汎用性を高めるために write() を send() に変更
				C timer.c
					A do_final_timer()
						終了時にメモリを開放するようにした
				C timer.h
					A do_final_timer()
						定義追加
			login/
				C login.c
					A login_version login_type
						clientinfo.xmlで指定する login_version login_type でログインを規制するときに使います
					C parse_fromchar()
						A 0x272b
							server[].maintenance フラグを変更する
							変更した内容をchar-serverに返信(0x272cパケット)する
					C parse_admin()
						C 0x7932
							0x2730→0x272a に変更
					C parse_login()
						C 0x64 0x01dd
							login_version login_type を判定するようにした
						C 0x2710
							パケット長の定義が正しくなかったのを修正した
							char.c check_connect_login_server の変更を反映
					C login_config_read()
						login_version login_typeの読み込みを追加
					A gm_account_db_final()
					C do_final()
						終了時にメモリを開放するように変更
				C Makefile
					A malloc.o malloc.h を追加
			map/
				C atcommand.c
					コンパイルオプションでmemwatchを読み込めるようにした
					C atcommand_where()
						他map-serverにいるキャラの居場所も表示できるようにした
					C atcommand_jumpto()
						他map-serverにいるキャラにも飛べるようにした
					C atcommand_who()
						ワールド内のすべてにいるキャラを表示するようにした
					C atcommand_go()
						ニブルヘルムの移動ポイントを変更
					C atcommand_recall()
						他map-serverにいるキャラも呼び出せるようにした
					A atshuffle_sub()
					A atcommand_shuffle()
						PCとMOBのシャッフルを行う @shuffle を追加
					A atcommand_maintenance()
						char-serverをメンテナンス状態にする @maintenance を追加
					A atcommand_misceffect()
						実行したキャラから0x1f3パケットを発信してエフェクトを表示する @misceffect を追加
					A atcommand_summon()
						コールホムンクルスと叫んで指定したMobを召喚する @summon を追加
						召喚されたMobは他のMob(Pv等では敵対PC含む)を攻撃します
						召喚されたMobに攻撃されたMobは召喚主を攻撃します(バイオプラントのフローラと同じ挙動)
						召喚されたMobは1分後に消滅します
						ネタなので隠しコマンドとしてhelp.txtには記述してません
				C atcommand.h
					追加した@コマンドを AtCommandType に追加
					C msg_table[] を外から使えるようにするためにexternした
				C battle.c
					コンパイルオプションでmemwatchを読み込めるようにした
					A battle_config.castle_defense_rate
						本鯖で防御値がどのように影響するか具体的には分からなかったので、砦の防御値を反映させる率を設定できるようにした
					C battle_get_opt1()
					C battle_get_opt2()
					C battle_get_option()
						NPCのオプションも返すようにした
					A battle_get_opt3()
						opt3を返すように追加
					C battle_calc_damage()
						砦内のMobへのダメージは防御値で減算(ダメージ*(防御値/100)*(castle_defense_rate/100))されるようにした
						ガーディアンにはスキルが効くようにした
					C battle_calc_pet_weapon_attack()
					C battle_calc_mob_weapon_attack()
					C battle_calc_pc_weapon_attack()
						ベナムスプラッシャーのダメージ計算をするようにした
					C battle_weapon_attack()
						武器攻撃による即死の仕様を変更
						武器、鎧破壊の確率計算をするように変更
						ベナムスプラッシャーが解除されるように変更
					C battle_check_target()
						src じゃなくて ss からparty_id guild_idを取得するように修正
						target=BCT_NOENEMY なスキル 口笛、ハミングなどがPvP、GvGの時にも他PCに影響するようにした
						Mobがspecial_aiならMobを敵とみなすようにした
					C battle_config_read()
						castle_defense_rate を読み込むようにした
				C battle.h
					A battle_get_opt3()
						定義追加
					C Battle_Config
						castle_defense_rate 追加
				C chat.c
					C chat_createnpcchat()
						引数に int pub を追加
						pub=3 ではチャットの看板に(0/20)のような表示がされなくなります
					A do_final_chat()
						何もしてないけどとりあえず追加
				C chat.h
					C chat_createnpcchat()
						定義を変更
					A do_final_chat()
						定義を追加
				C chrif.c
					パケット長テーブルを拡張
					C chrif_connect()
					C chrif_changemapserver()
						WFIFOLでポートを送っていたのをWFIFOWに修正
					A chrif_recverasemap()
						他map-serverが切断されたことがchar-serverより通知された時に、そのmap-serverが管理していたマップの情報を削除するようにした
					A chrif_mapactive()
						map-server起動準備中に0、完了時に1をchar-serverに通知(0x2b13)して起動途中にユーザーがログインできないようにした
					A chrif_maintenance()
						char-serverをメンテナンス状態にしたり解除したりを通知(0x2b14)する
					A chrif_maintenanceack()
						char-serverをメンテナンス状態にした時の応答
						メンテナンス状態にした旨をマップ内に通知する
					A chrif_chardisconnect()
						char-serverにキャラが切断されたことを通知(0x2b18)する
					A chrif_parse_chardisconnectreq()
						char-serverからのキャラ切断要求を受けて、対象キャラがいる場合には切断する
					C chrif_parse()
						C 0x2b09
							map_addchariddb() の引数増加に対応
						A 0x2b15
						A 0x2b16
						A 0x2b17
						A 0x2b19
							各新規パケットに対応
					C check_connect_char_server()
						接続時char-serverにmap-serverの準備が出来たことを通知するようにした
					A do_final_chrif()
						終了時にchar-serverとの接続を削除するようにした
				C chrif.h
					A chrif_mapactive()
					A chrif_maintenance()
					A chrif_chardisconnect()
					A do_final_chrif()
						定義を追加
				C clif.c
					すべてのパケット定義を packet_db から読み込むように変更
					packet_db.txtを変更することで、度々変更される韓国クライアントのパケット定義に対応しやすくなります
					packet_len_table[] は packet_db[].len に置き換わりました
					<time.h> をinclude
					clif_parse_*を先頭で宣言するようにした
					clif_parse_*内のRFIFO系で使われている第2引数は packet_db[cmd].pos[] で表記されるようになりました
					パケット番号の最大値を MAX_PACKET_DB で定義するようにした
					C clif_set0078()
					C clif_set007b()
						パケットの内容が本鯖と違っていたのでguild_emblem_id,manner,opt3を正しく送るように変更
					C clif_class_change()
						Mob以外のNPCでも使えるようにした
					C clif_mob0078()
					C clif_mob007b()
						ガーディアンにギルドエンブレムを表示するように変更
					C clif_npc0078()
						ワープポータルをギルドフラッグにするオプションを有効にした時にmap-serverが落ちる問題を修正
					C clif_spawnnpc()
						NPCが無効でもHide状態の時はパケットを送るように変更
					C clif_quitsave()
						キャラ終了時にchar-serverに切断を通知するようにした
					C clif_scriptmenu()
					C clif_dispchat()
					C clif_changechatstatus()
						lenに1バイト追加
					C clif_updatestatus()
						マナーポイントを送信するようにした
					A clif_changestatus()
						周囲に赤エモ状態であることを送信
					A clif_misceffect2()
						エフェクトを発生させるパケットを送信
						@misceffect, misceffect命令で使用
					C clif_changeoption()
						状態異常時以外は状態異常アイコン表示パケットを送らないようにした
						PCの時は clif_changelook() を送信するようにした(結婚衣裳表示用？)
					C clif_traderequest()
						取引パケットの 0xe5 → 0x1f4 に仮対応(本鯖での算出式は不明なのでとりあえず char_id を送信)
					C clif_tradestart()
						取引パケットの 0xe7 → 0x1f5 に仮対応(本鯖での算出式は不明なのでとりあえず char_id を送信)
					C clif_getareachar_pc()
						マナーポイントが負数の時には赤エモ表示パケットを送信するようにした
					C clif_getareachar_npc()
						NPCのHide状態に対応
					C clif_getareachar_skillunit()
					C clif_skill_nodamage()
						自爆の時はhealを負数にできるようにした
					C clif_skill_setunit()
						グラフィティに対応
					A clif_item_repair_list()
						武器修理スキルに対応しようとしたけどパケットが分からないので頓挫中
					C clif_produceeffect()
						map_addchariddb() の引数増加に対応
					C clif_guild_skillinfo()
						未実装ギルドスキル カリスマを表示しないようにした
					C clif_callpartner()
						あなたに逢いたい 使用時に相手の名前を叫ぶようにした
					C clif_sitting()
						引数 fd は不要なので削除
					C clif_GM_kick()
						フラグを0にするようにした
					A clif_wisexin()
						Wis拒否許可の応答を送信
					A clif_wisall()
						Wis全拒否許可の応答を送信
					A clif_soundeffect()
						SEを鳴らすパケットを送信
						soundeffect命令で使用
					C clif_parse_LoadEndAck()
						結婚後のウェディングドレスやタキシードの状態をログアウトしても1時間は継続されるようにした
						赤エモ状態はログアウトしてもログインした時からまた継続するようにした
					C clif_parse_QuitGame()
					C clif_parse_Restart()
						終了できない条件を pc_isquitable() にまとめた
					C clif_parse_GlobalMessage()
					C clif_parse_Wis()
					C clif_parse_PartyMessage()
					C clif_parse_GuildMessage()
						赤エモ状態では発言できないようにした
					C clif_parse_ActionRequest()
						ギルド未加入などの場合はガーディアンやエンペリウムを殴れないようにした
						clif_sitting()の引数変更に対応
					C clif_parse_UseItem()
						赤エモ状態ではアイテムを使えないようにした
					C clif_parse_EquipItem()
						アイテムが破壊されている時は装備できないようにした
					C clif_parse_TradeRequest()
					C clif_parse_TradeAck()
						notradeマップでは取引要請を送れないようにした
					C clif_parse_UseSkillToId()
					C clif_parse_UseSkillToPos()
					C clif_parse_UseSkillMap()
						noskillマップではスキルを使用できないようにした
						チャット中はスキルを使用できないようにした
						赤エモ中はスキルを使用できないようにした
						ウェディング状態ではスキルを使用できないようにした
					C clif_parse_MoveToKafra()
						itemdb_isdropable()==0 は倉庫に入れられないようにした
					C clif_parse_GMReqNoChat()
						GM右クリックで赤エモを付与・解除できるようにした
					C clif_parse_GMReqNoChatCount()
						本鯖での返答パケットがよく分からないので仮対応
						本当はアカウント名が返るのかな？
					C clif_parse_sn_explosionspirits()
						クライアントからパケットが来た時にコンソールにログを表示するようにした
						BaseLv99以上の時に0で除算する可能性があるのを回避
					A pstrcmp()
						clif_parse_wisexin()のqsort()で使用
					A clif_parse_wisexin()
						Wis拒否許可に対応
					A clif_parse_wisexlist()
						Wis拒否リスト表示に対応
					A clif_parse_wisall()
						Wis全拒否許可に対応
					A clif_parse_GMkillall()
						GMコマンド/killall(=@kickall)に対応
					A clif_parse_GMsummon()
						GMコマンド/summon(=@recall)に対応
					A clif_parse_GMshift()
						GMコマンド/shift(=@jumpto)に対応
					A clif_parse_debug()
						packet_db.txtのデバグ用に追加
						パケット内容をダンプします
					C clif_parse()
						clif_parse_func_table を削除(packet_db[cmd].funcに入るようになりました)
					A packetdb_readdb()
						packet_db.txtを読み込みます
						フォーマットは パケット番号,パケット長[,コマンド,コマンド引数の位置(:区切りで複数指定)]
						コマンド引数の位置は各コマンドに対応する関数内で設定されているのでclif.cを読まないと分からない難解なフォーマットです
						変更されたパケットはpacket_db.txtの末尾に追加します
						古いクライアントを利用する場合には不要な定義を末尾から削除すればよいようにします
					A do_final_clif()
						終了時にセッションを削除するようにした
					C do_init_clif()
						packet_dbを読み込むようにした
						終了時にセッションを削除できるように make_listen_port() の戻り値を map_fd に入れるようにした
				C clif.h
					A MAX_PACKET_DB
					A struct packet_db
					A clif_changestatus()
					A clif_misceffect2()
					A clif_callpartner()
					A clif_sitting()
					A clif_soundeffect()
					A clif_item_repair_list()
					A do_final_clif()
						定義を追加
					C clif_class_change
						clif_mob_class_change() から変更
				C guild.c
					C guild_read_castledb()
						castle_event を読み込むようにした
					C guild_skillup()
						引数を変更
						flag=1 でギルドポイントを使用しないようにした
					C guild_broken()
						ギルド解散時に所有砦を破棄するための OnGuildBreak イベントを追加
					A guild_db_final()
					A castle_db_final()
					A guild_expcache_db_final()
					A guild_infoevent_db_final()
					A do_final_guild()
						終了時にメモリを開放するようにした
				C guild.h
					C guild_skillup()
						定義を変更
					A do_final_guild()
						定義を追加
				C intif.c
					atcommand.h をinclude
					packet_len_table[] 拡張
					C intif_guild_skillup()
						引数 flag 追加
					A intif_charposreq()
						キャラの場所要求パケットを送信
						flag=1 @jumpto
						flag=0 @where
					A intif_jumpto()
						他map-serverのキャラに @jumpto 出来るようにした
					A intif_where()
						他map-serverのキャラに @where 出来るようにした
					A intif_charmovereq()
						キャラを呼び寄せる
						flag=1 @recall
						flag=0 あなたに逢いたい
					A intif_displaymessage()
						他map-serverのキャラにメッセージを送れるようにした
						(Wisではなくて送りっぱなし。@recall 成功時用)
					C intif_parse_WisMessage()
						Wis拒否の判定をするようにした
					A intif_parse_CharPosReq()
						キャラの居場所をInterへ返答
					A intif_parse_CharPos()
						キャラの居場所がInterから送られてきたので
						flag=1 キャラの場所へ移動(@jumpto)
						flag=0 キャラの場所を表示(@where)
					A intif_parse_CharMoveReq()
						キャラがいたら指定位置に移動させる
						flag=1 @recall なのでGMレベルを比較、メッセージを表示
					A intif_parse_DisplayMessage()
						指定キャラにメッセージを送信
					C intif_parse()
						新パケットを追加
				C intif.h
					C intif_guild_skillup()
						定義変更
					A intif_jumpto()
					A intif_where()
					A intif_charmovereq()
					A intif_displaymessage()
						定義の追加
				C itemdb.c
					A itemdb_isdropable()
						アイテムが捨てられるかどうかの判定をする
					A itemdb_read_cardillustnametable()
						grfファイルから num2cardillustnametable.txt を読み込む
						cutincard命令で使用
					C do_init_itemdb()
						itemdb_read_cardillustnametable() を追加
				C itemdb.h
					C struct item_data
						char cardillustname[64] 追加
					A itemdb_isdropable()
						定義の追加
				C Makefile
					A malloc.o malloc.h を追加
				C map.c
					C struct charid2nick
						@whoで他map-serverのキャラも表示できるように account_id ip port を追加
					C map_freeblock()
					C map_freeblock_unlock()
						二重free()対策でNULLを代入するようにした
					C map_delblock()
						見やすいように繰り返し使用される変数をまとめた
					C map_addchariddb()
						charid2nick の拡張にあわせて引数を増やした
					A map_delchariddb()
						charid_db からキャラを削除(実際にはip portを0に)する
					C map_quit()
						結婚状態中はログアウトしても1時間は状態が続くようにPCグローバル変数 PC_WEDDING_TIME に開始時間を記録するようにした
					C map_id2bl()
						見やすいように書き換え
					A map_eraseipport()
						他map-server管理のマップを map_db から削除する
					A map_who_sub()
					A map_who()
						他map-serverにいるキャラも @who で表示されるようにした
						表示上キャラが残ることがあるのは調査中
					A id_db_final()
					A map_db_final()
					A nick_db_final()
					A charid_db_final()
					C do_final()
						終了時にメモリを開放するように変更
				C map.h
					A MAX_WIS_REFUSAL
						Wis拒否リストの保存最大値
					C struct map_session_data
						C special_state
							A unbreakable_weapon
								武器が絶対に壊れない
							A unbreakable_armor
								鎧が絶対に壊れない
						A opt3
							画面外から入ってきたキャラの状態
						A areanpc_id
							OnTouchイベントを実行したNPCのID
						A wis_refusal[][]
							Wis拒否リスト
						A wis_all
							Wis全拒否フラグ
						A break_weapon_rate
							武器破壊率
						A break_armor_rate
							鎧破壊率
						A add_steal_rate
							追加スティール率
					C struct npc_data
						A opt1,opt2,opt3,option
							PCと同じ
						C u.scr
							A src_id
								終了時のメモリ開放用
					C struct mob_data
						A opt3
							PCと同じ
						A guild_id
							ガーディアンなどで使用
						D exclusion_*
							関連関数を消したので削除した
					C struct map_data
						C flag
							A notrade
								取引禁止マップフラグ
							A noskill
								スキル使用禁止マップフラグ
					定数の追加
						SP_PARTNER SP_CART
						SP_BREAK_WEAPON_RATE SP_BREAK_ARMOR_RATE SP_ADD_STEAL_RATE
						SP_UNBREAKABLE_WEAPON SP_UNBREAKABLE_ARMOR
					D talkie_mes[]
						定義削除
					C map_addchariddb()
						定義変更
					A map_delchariddb()
					A map_eraseipport()
					A map_who()
						定義追加
				C mob.c
					D mob_exclusion_add()
					D mob_exclusion_check()
						意味がある使用をされていないのとbattle_check_target()で代用できるので削除した
					C mob_stop_walking()
						type&4で目的の場所まで距離があれば1歩進んで止まるようにした
					C mob_attack()
						MobがMobを攻撃できるようにした
					C mob_target()
					C mob_ai_sub_hard_slavemob()
						mob_exclusion_check()を削除
					C mob_ai_sub_hard_activesearch()
					C mob_ai_sub_hard()
						special_mob_aiな場合はMobも索敵するようにした
						ルートモンスターが目標のアイテムを見失った時は目的の場所まで歩かないようにした
					C mob_damage()
						スフィアマインが殴られた時に自爆しなかったのを修正した
						スフィアマインが殴られて自爆する時に移動するようにした
						srcがMobの時はsrcのターゲットを外すようにした
					C mob_skillid2skillidx()
						インデックスが0から始まるのにエラーも0を返すしていたのを修正した
						スフィアマインが殴られても自爆しない原因はこれ
					C mobskill_use()
						自爆状態ではスキルを使用できないようにした
					C mob_spawn()
						ガーディアンとエンペリウムが砦で発生した場合は guild_id を設定
						opt3 を 0 で初期化
					C mob_can_reach()
						GvG以外ではガーディアンは何もしないようにした
					C mob_catch_delete()
						Mobが消えるときのエフェクトを type で指定できるようにした
					C mob_timer_delete()
						スフィアマインとバイオプラントが消えるときはテレポエフェクトで消えるようにした
					C mob_deleteslave_sub()
						nullチェック前に代入している部分を修正
					C mob_class_change()
						clif_class_change() の変更に対応
				C mob.h
					C mob_catch_delete()
						定義変更
					D mob_exclusion_add()
					D mob_exclusion_check()
						定義削除
				C npc.c
					C struct npc_src_list
						A prev
							終了時のメモリ開放用に追加
					C npc_checknear()
						イベントPCの場合に常にOKを返していなかったのを修正
					A npc_enable_sub()
						npc_enable() から呼ばれて周囲のPCにOnTouchイベントを実行する
					C npc_enable()
						flag による挙動を追加
						flag=2 NPCのHide状態を解除する
						flag=4 NPCをHide状態にする
						HideしているNPCは無効になります
						有効にした時に npc_enable_sub() を呼ぶようにした
					C npc_event()
						エラー時は1を返すように変更
						OnTouchイベントから呼ばれたときはイベントが見つからないエラーを返さないようにした
					C npc_touch_areanpc()
						PCがエリア内を通った時に何度も実行されるのを修正
						NPCにOnTouchイベントがあった場合には実行するようにした
						互換性を保つためにOnTouchイベントが無い場合は今までと同じように動きます
					C npc_parse_warp()
						option,opt1,opt2,opt3 を 0 で初期化
					C npc_parse_warp()
					C npc_parse_shop()
						IDをnpc_get_new_npc_id()で取得するようにした
						option,opt1,opt2,opt3 を 0 で初期化
					C npc_convertlabel_db()
						メモリ確保後にnullかどうか確認していないのを修正
					C npc_parse_script()
						bad duplicate name!エラー表示が改行されていなかったのを修正
						終了時メモリ開放用にduplicateで src_id を挿入
						IDをnpc_get_new_npc_id()で取得するようにした
						option,opt1,opt2,opt3 を 0 で初期化
					C npc_parse_mob()
						memwatch対策でメモリを一括確保しないようにした
						モンスター名に --ja-- --en-- を指定するとmob_dbの名前を使うようにした
						IDをnpc_get_new_npc_id()で取得するようにした
					C npc_parse_mapflag()
						notrade noskill を読み込むようにした
					A ev_db_final()
					A npcname_db_final()
					A do_final_npc()
						終了時にメモリを開放するようにした
					C do_init_npc()
						メモリを開放するようにした
				C npc.h
					A do_final_npc()
						定義の追加
				C party.c
					A party_db_final()
					A do_final_party()
						終了時にメモリを開放するようにした
				C party.h
					A do_final_party()
						定義の追加
				C pc.c
					A pc_numisGM()
						account_idでGMかどうか判断する
					A pc_isquitable()
						PCが終了できる状態にあるかどうか判断する
						1を返すときは終了できない
					C pc_counttargeted_sub()
						Mob状態によって値を正しく返さないような気がするので条件を仮変更
					C pc_makesavestatus()
						マナーポイントが正数の場合は 0 にする
					C pc_authok()
						wis_all を 0 で初期化
						map_addchariddb() の変更に対応と常に実行するようにした
					C pc_calcstatus()
						break_weapon_rate break_armor_rate add_steal_rate を 0 で初期化
						結婚状態では歩く速度が半分になるようにした
					C pc_bonus()
						SP_UNBREAKABLE_WEAPON SP_UNBREAKABLE_ARMOR SP_BREAK_WEAPON_RATE SP_BREAK_ARMOR_RATE SP_ADD_STEAL_RATE
						処理を追加
					C pc_dropitem()
						アイテムを捨てられるかどうか判定するようにした
					C pc_putitemtocart()
						アイテムをカートに移動できるか判定するようにした
					C pc_steal_item()
						スティール率に add_steal_rate を加算するようにした
					C pc_walk()
					C pc_movepos()
						範囲NPCがいないときには areanpc_id=0 にした
					C pc_checkbaselevelup()
						スパノビがレベルアップした時にかかるスキルのレベルを本鯖にあわせた
					C pc_skillup()
						guild_skillup() の変更に対応
					C pc_damage()
						スパノビがExp99%でHPが0になるとHPが回復して金剛状態になるようにした
					C pc_readparam()
						nullチェック前にsdを使っていたのを修正
						A SP_PARTNER
							結婚相手のchar_id
						A SP_CART
							カートを引いている場合は0以上が返る
					C pc_jobchange()
						マナーポイントが負数の場合は赤エモ表示するようにした
					A pc_break_weapon()
						武器破壊をする
					A pc_break_armor()
						鎧破壊をする
					C pc_natural_heal_sp()
						スパノビは爆裂状態でもSPが自然回復するようにした
					A gm_account_db_final()
					A do_final_pc()
						終了時にメモリ開放するようにした
				C pc.h
					A pc_numisGM()
					A pc_isquitable()
					A pc_break_weapon()
					A pc_break_armor()
					A do_final_pc()
						定義追加
				C pet.c
					C pet_data_init()
					C pet_lootitem_drop()
						メモリ確保できたかどうか確認していなかったので修正
					C pet_catch_process2()
						mob_catch_delete() の変更に対応
				C script.c
					追加した関数のプロトタイプを先頭に追加
					buildin_func[]に追加した命令や関数を追加
					演算子に C_R_SHIFT C_L_SHIFT を追加
					C parse_subexpr()
						演算子 >> << 追加
					C get_val()
						PC主体の変数でPCがアタッチされていなかったらエラーを出すようにした
						PC主体の変数でsd=NULLだった場合にはpc_read*で取得に行かないようにした
					A buildin_close2()
						スクリプトを中断してCloseボタンを表示します
					C buildin_areawarp_sub()
						RandomだけでなくSavePointにも飛ばせるようにした
					A buildin_cutincard()
						カードのアイテムIDを指定することでカード画像を表示します
					C buildin_getitem()
						引数を変更して鑑定した状態で渡すかどうかを指定できるようにした
						account_idを指定することで、そのPCにアイテムを渡せるようにした(結婚用拡張)
					C buildin_getitem2()
						account_idを指定することで、そのPCにアイテムを渡せるようにした(結婚用拡張)
					C buildin_readparam()
						キャラ名を指定することで、そのPCのパラメータを読み取れるようにした
					C buildin_getcharid()
						キャラ名を指定することで、そのPCの関係IDを取得できるようにした
					A buildin_getpartymember()
						指定IDのパーティ人数の取得とパーティーメンバーのIDを配列で取得できます
					A buildin_guildskill()
						ギルドスキルを覚えることができます
					C buildin_getgdskilllv()
						ギルドスキルIDをGD_APPROVALのようなスキル名で指定するようにした
					A buildin_hideoffnpc()
						Hide状態のNPCを表示する
					A buildin_hideonnpc()
						NPCをHide状態にする
					C buildin_sc_start()
						ID指定したキャラを状態異常にできるようにした
					A buildin_sc_start2()
						確率指定でキャラを状態異常にできます(アイス、おもち等で使用)
					A buildin_getscrate()
						状態異常耐性を計算した確率を返す
					C buildin_changebase()
						IDで指定したキャラの見た目を変更することができるようにした
					C buildin_waitingroom()
						limit=0の時は(1/10)を表示しないようにした
					C buildin_setmapflag()
						MF_NOTRADE MF_NOSKILL を追加
					C buildin_flagemblem()
						NPCが特定できなかったときにmap-serverが落ちる問題を修正
					A buildin_getinventorylist()
						配列で所持品を返します
					A buildin_getskilllist()
						配列で所有スキルを返します
					A buildin_clearitem()
						所持アイテムを削除します
					A buildin_getrepairableitemcount()
						壊れているアイテムを数えます
					A buildin_repairitem()
						壊れているアイテムをすべて修理します
					A buildin_classchange()
						NPCをクラスチェンジします
					A buildin_misceffect()
						エフェクトを表示します
					A buildin_soundeffect()
						指定したSEを鳴らします
					C op_2num()
					C run_script_main()
						シフト演算子を追加
					A mapreg_db_final()
					A mapregstr_db_final()
					A scriptlabel_db_final()
					A userfunc_db_final()
					C do_final_script()
						終了時にメモリを開放するようにした
				C skill.c
					<timer.h> intif.h をinclude
					コメントのスキル名をjRO仕様に書き換え
					C SkillStatusChangeTable[]
						ベナムスプラッシャー グラフィティ 自爆 自爆2 を追加
					C skill_additional_effect()
						ベナムスプラッシャー追加
						アンクルスネアを削除
					C skill_attack()
						チャット中にスキルが影響しないようにした(チャットキャンセル)
						ベナムスプラッシャーはSkillLv=-1でclif_skill_damage()するようにした
						自爆はダメージ表示しないようにした
					C skill_castend_damage_id()
						アシッドテラーで武器破壊をするようにした
						ベナムスプラッシャーが3*3の範囲攻撃をするようにした
						自爆の処理を変更した
					C skill_castend_nodamage_id()
						sdとdstsdでPCかどうかを判定するようにした
						スパノビの嫁がヒールを使うと回復量が2倍になるようにした
						clif_sitting()の変更に対応
						武器修理はパケットが分からないのでコメントアウト
						ストリップ～、ケミカル～をスキルユニットに使用した場合、map-serverが落ちる問題を修正
						君だけは護るよ、あなたの為に犠牲になりますの計算をMAX_HPまたはMAX_SPからするようにした
						あなたに逢いたい を相手の名前を叫ぶ、複数設置できない等、本鯖風にした
						アンクルスネアでPCが引っかかっている時にリムーブトラップしてもPCが動けるようにならなかったのを修正
						アンコールを叫ぶようにした
						ベナムスプラッシャーを実装した
						自爆で自爆状態を開始するようにした
					C skill_castend_pos2()
						バイオプラント、スフィアマイン
							パケット順番を変更
							指定した場所に設置するようにした
							効果時間をskill_cast_db.txtで指定するようにした
							mob_exclusion_add()を削除
						グラフィティを実装、1個しか置けません
					C skill_castend_map()
						ワープポータルは実際の設置時にブルージェムストーンを消費するようにした
					C skill_unitsetting()
						グラフィティのスキルユニットを1個に修正
						トーキーボックス、グラフィティの文字列は sd->message に格納するようにした
					C skill_unit_onplace()
						チャット時はスキルユニットが動作しないようにした(チャットキャンセル)
						アンクルスネアにかかる処理をskill_additional_effect()から移動
						ワープポータルに術者が乗ったら消えるようにした
						デモンストレーションによる武器破壊をするようにした
						アンクルスネア、スパイダーウェッブでメモリアクセス違反が起きる可能性があったのを修正
					C skill_unit_onout()
						アンクルスネアで「}」が足りなかったために近くを通りかかっただけで1秒後罠に戻ってしまったのを修正
					C skill_unit_onlimit()
						ワープポータル発動前の処理を削除
						あなたに逢いたいを他map-serverにいても呼べるようにした
					A skill_check_condition_mob_master_sub()
						マップ内で同じPCから出たバイオプラントやスフィアマインの数を数える
					C skill_check_condition()
						hp_rateとsp_rateに負数を指定すると消費計算をMax値からするようにした
						あなたに逢いたいを結婚していない状態で使ったら使用失敗を表示するようにした
						バイオプラントとスフィアマインの設置数をskill_cast_db.txtで設定できるようにした
						ファイアーウォールの数制限を skill_use_pos() から移動
					C skill_use_id()
						バジリカをGvGでは使用できないようにした
						ベナムスプラッシャーは対象が毒状態でなければ使用失敗
					C skill_use_pos()
						ファイアーウォールの数制限を skill_check_condition() に移動
					C skill_status_change_end()
						opt3の処理を追加
						結婚状態の終了を追加
						ベナムスプラッシャーを追加
						自爆を追加
					C skill_status_change_timer()
						結婚状態と赤エモ状態のタイマー再設定を追加
						自爆状態では1秒ごとに速度が変化するようにした
					C skill_status_change_start()
						opt3の処理を追加
						グラフィティは追加で置いたら前のは消えるようにした
						結婚状態と赤エモ状態を追加
						グラフィティは状態異常開始時にスキルユニットを設置するようにした
						ベナムスプラッシャーは特に何も追加はなし
						自爆は詠唱パケットをここで送るようにした
					C skill_status_change_clear()
						opt3の処理を追加
					C skill_unit_timer_sub()
						ワープポータル発動前が時間切れになるときに見た目を変更して本鯖のように効果音が出るようにした
						ブラストマイン以外の罠は時間切れで罠に戻るようにした
				C skill.h
					状態異常にスキル名をいくつかつけたり、新規の状態異常を増やした
				C storage.c
					A storage_db_final()
					A guild_storage_db_final()
					C do_final_storage()
						終了時にメモリを開放するようにした
				C trade.c
					C trade_tradeadditem()
					C trade_tradecommit()
						itemdb_isdropable()で交換できないアイテムを判定するようにした
				C vending.c
					vending_purchasereq()
						金額計算をdoubleでするようにしてintで桁あふれしないようにした

--------------------
//1045 by TEILU

・スティール、スティールコイン、スナッチャーの失敗メッセージが
  レベルが１～９の時に変だったので修正。
	(map/)
	skill.c

・精錬の可否を情報サイトを元にＤＢに設定。（頭装備はrusiさん作成分を使用）
	(db/)
	item_db.txt

・ファイアーウォールが制限数を超えたときにスキル使用失敗が
  出るタイミングを変更。
	(map/)
	skill.c

・アブラカタブラ専用スキルをgm_all_skill設定時に表示できるように変更。
  battle_athena.confのgm_all_skill_add_abraにyesを設定すれば
  スキルリストに表示されるようになります。
	(conf/)
	battle_athena.conf
	(db/)
	skill_require_db.txt
	(map/)
	battle.c
	battle.h
	pc.c

--------------------
//1044 by TEILU

・1042で@itemidentifyの権限の設定を忘れていたので追加。
	(conf/)
	atcommand_athena.conf

・スティール、スティールコイン、スナッチャーの失敗時に
  失敗メッセージを表示するように変更。
	(map/)
	skill.c

・アイテムＤＢに精錬可否フラグのカラムを追加
  精錬の可否をＤＢを参照するように修正
	※とりあえず中段、下段、中下段の頭装備とアクセサリ以外の
	  装備品はすべて精錬可の設定でＤＢを作りました。
	  ＤＢは修正が必要になります。
	(db/)
	item_db.txt
	(map/)
	itemdb.c
	itemdb.h
	script.c

--------------------
//1043 by dusk
・docフォルダ・confフォルダ内のhelp.txtに1042(TEILUさん)の説明追加
	@itemidentifyの説明は７２行目の@itemresetの下に。

・Valkyrie Realms 5 (右上)の旗の修正
	Valkyrie Realms 5 (右上)の旗を見ると未取得状態コメントばっかり出ていたのを
	ちゃんと確認できるように。
	※ Valkyrie Realmsの各砦に戻る旗とは違います。
	prtg_cas05.txt内のギルドダンジョンに入るレバー以外の
	getcastledata "prtg_cas05.gat",1,@GIDp5;を
	set @GIDp5,getcastledata("prtg_cas05.gat",1);に修正。

--------------------
//1042 by TEILU

・@healコマンドに何も渡さないと完全回復するように変更。
	(map/)
	atcommand.c

・@itemitemidentifyコマンドの追加
  未鑑定の所持アイテムを全て鑑定します。
	(conf/)
	msg_athena.conf
	(map/)
	atcommand.c
	atcommand.h

--------------------
//1041 by mare
	FIX NPC Script Command - buildin_getgdskilllv()
        Add NPC Sctipt Command - buildin_agitcheck()
	(script/npc/job/)
	npc_job_wizard.txt
		ラウレルさんの台詞、ノビとプリの場合の分追加
-------------------
//1040 by 胡蝶蘭

・サーバー間接続のパケット表追加
	(doc/)
	serverlink_packet.txt
		inter<->map 以外のサーバー間接続のパケット表

・仕様スレの Login_ID2 関係でごにょごにょ
・サーバー間接続のパケット一部変更
	(login/)
	login.c
		auth_fifo に ip メンバ追加
		パケット変更に伴う変更他
	(char/)
	char.c
		auth_fifo に login_id2, ip メンバ追加
		パケット変更に伴う変更他
	(map/)
	chrif.c
		chrif_authok()追加
		パケット変更に伴う変更他

・自動再起動スクリプト start にコメントで簡単な説明追加
	start
		コメント追加

--------------------
//1039 by Ni+S
	・ギルド関係のスクリプト
	所有者の居ないアジトから、ギルドに所属してないキャラなら
	ギルドダンジョンに入れてしまうという不具合がありました

	これは、所有者の居ないアジトの値が0であり、
	getcharid(2)でギルドIDを返すのですが、
	ギルドに所属していないキャラはgetcharid(2)で0を返す為、
	値が一致してしまい起こっていた現象でした
	未所属キャラが入れないように修正しました

	・ファーマシー/ポーション作成DB
		レッドスリムポーション
		イエロースリムポーション
		ホワイトスリムポーション
	の材料を、空のポーション瓶から試験管に修正

--------------------
//1038 by Plala
・転職NPC関連の重大なバグ修正
	(script/npc/job)
	npc_job_aco.txt 修正
	npc_job_merchant.txt 修正
	npc_job_thief.txt 修正

	・上記NPCで途中までクエストを進めて他の職に転職すると、
	再び転職可能だった点を修正しました


--------------------
//1037 by 胡蝶蘭

** FOR ENGLISH DEVELOPERS **
DO NOT UPLOAD IF YOU DON'T USE JAPANESE ENCODE (SHIFT-JIS) !
WHY WE(JAPANESE) REPAIR ERROR CHARACTER AFTER EVERY YOUR UPLOADING ?
BREAKING IS EASY, REPAIRING IS VERY DIFFICULT !

** 英語圏の開発者の方へ(日本語訳) **
日本語エンコード(シフトJIS)を使う気が無いならアップロードしないてください！
なぜ我々（日本人）があなた方のアップロードのたびに文字化けを直さなければならないんですか？
壊すのは簡単ですが、直すのはとても難しいんです！

・文字化けを根性で修正
	(map/)
	script.c

・ladminがPOSIX必須に。Digest::MD5が無くても実行できるように修正
・serverstatus.cgiで、Net::Pingが無くても実行できるように修正
	(bin/tool/)
	ladmin
	(bin/tool/cgi/)
	serverstatus.cgi

・script_refで抜けてるものでわかるものを修正
	何故かscript_refから抜けてるもの(getargなど)の一部を再び追加
	** アップロードするときは最新パッチからの差分をアップロードしましょう **
	(doc/)
	script_ref.txt

・account_making.txt修正
	ladminスクリプトのパス
	(doc/)
	accoun_tmaking.txt

--------------------
//1036 by Michael
・追加 Script Command:
  getequipid(EquipPos);		EquipPos: 1-10
  gettimetick(Type);		Type: 0 SystemTick, 1 TimeSecondTick(0-86399)
  gettime(Type);		Type: 1 Sec, 2 Min, 3 Hour, 4 Weekday, 5, Monthday, 6 Month, 7 Year
  gettimestr("TimeFMT", Len);	TimeFMT: Time format strinf / Len: String Length

	(map/)
	script.c
		buildin_getequipid(); 追加
		buildin_gettimetick(); 追加
		buildin_gettime(); 追加
		buildin_gettimestr(); 追加

--------------------
//1035 by Michael
・追加 GVG Script NPC edit from Aegis NPC(Chinese-big5 version), Please someone translate to Japanese.
・修正 NPC Script Command - buildin_getgdskilllv()
  getgdskilllv(Guild_ID, Skill_ID);

	(map/)
	script.c
		buildin_getgdskilllv() 修正

--------------------
//1034 by (Pepermint)
	FIX NPC Script Command - buildin_getgdskilllv()
        Add NPC Sctipt Command - buildin_agitcheck()
	(map/)
	script.c
		buildin_getgdskilllv() 修正
		buildin_agitcheck() 追加

--------------------
//1033 by Michael
・追加 NPC Script Command - buildin_getgdskilllv()
  getgdskilllv(Guild_ID, Skill_ID);
  skill_id = 1:GD_APPROVAL,2:GD_KAFRACONTACT,3:GD_GUARDIANRESEARCH,4:GD_CHARISMA,5:GD_EXTENSION

	(map/)
	script.c
		buildin_getgdskilllv() 追加

--------------------
//1032 by (凸)
・1031で何故か削除されていたbuildin_getitemname()を復活
・バグスレなどに出た修正を反映
・その他細かい修正

	(map/)
	clif.c
		clif_disp_onlyself() NULLチェック追加
	map.c
		map_nick2sd() nickがNULLだとすぐNULLを返すように変更
	mob.c
		mob_setdelayspawn() NULLチェック変更
		mob_delete() 修正
	npc.c
		npc_parse_warp() 修正
	script.c
		buildin_getitemname() 復活

--------------------
//1031 by huge
・NPCのscriptに、makepetを追加。
    makepet 卵ID; で、ペットを作成します。
・NPCのscriptに、getexpを追加。
    getexp Base,Job; で、それぞれの経験値を増やします。
・ペットの卵をNPCのdelitemで消したりshopで売った時、ペットセーブデータから削除するよう修正。
・ディボーション成功条件修正。(未確認)
・経験値表示を可能にしてみました。confにて設定してください。

	(conf/)
	battle_athena.conf 修正
	(doc/)
	conf_ref.txt 修正
	script_ref.txt 修正
	(map/)
	battle.c
	battle.h
		disp_experience 追加
	clif.c
	clif.h
		clif_disp_onlyself() 追加
	pc.c
		pc_gainexp() 修正
	script.c
		buildin_delitem() 修正
		buildin_makepet() 追加
		buildin_getexp() 追加
	npc.c
		#include 修正
		npc_selllist() 修正
	skill.c
		skill_castend_nodamage_id() 修正


--------------------
//1030 by (凸)
・map_athena.confに新婚島ザワイをコメントアウトして追加
・クローンスキルで覚えたスキルを自動セーブごとに忘れていたのでとりあえずログオフ時にのみ忘れるように変更したつもり
・mobskill_castend_posの無害nullpoチェックを変更
・Emotionの設定がないMobがスキルを使用するときに/!を出していたのを修正
・バグスレに投げたtrade.cを添付。とりあえず相手が所持できる種類の限界を超えた場合は渡さずに元に戻すように変更

	(conf/)
	map_athena.conf ザワイ追加
	(doc/)
	client_packet.txt パケット長の更新
	(map/)
	map.c
		map_quit() 終了時にクローンスキルで覚えたスキルを忘れるように変更
	mob.c
		mobskill_castend_pos() nullpoチェック変更
		mob_readskilldb() 修正
	pc.c
		pc_makesavestatus() クローンスキルを忘れるのをmap_quitに任せた
	trade.c
		trade_tradecommit() 所持できる種類以上を取引した場合にアイテムが消えないように

--------------------
//1029 by (凸)
・20040619RagexeHC_jp.rgzの0x204と0x20bパケットに対応
・charとloginも知らないパケットが来たらパケットダンプを出力するようにclif.cからコピペ

	(doc/)
	client_packet.txt 新パケット追加
	(char/)
	char.c
		parse_char() 0x20b対応
	(login/)
	login.c
		parse_login() 0x204対応

--------------------
//1028 by (凸)
・ウンバラ以降、Mobがスキルを使用するときにエモーションを出すようになったので、mob_skill_dbを拡張
	サンプルでオークウォーリアーが喫煙すると「/…」を出すのとオークレディが速度を使うと「/ちゅ」を出します
・アイテム682,683を使用すると30秒間ATKやMATKが増えるらしいのでそれっぽく
・job_db2.txtに謎の行が2行あったのを削除
・範囲魔法などでスキルユニット相手にステータス変更をかけようとした場合にnullpoが出たのを修正

	(db/)
	const.txt SC_INCATK SC_INCMATK追加
	item_db.txt ↑を682,683に追加
	job_db2.txt 謎の2行を削除
	mob_skill_db.txt Emotion追加
	(map/)
	mob.c
		mobskill_use() スキル使用時にエモーションを出すように変更
		mob_readskilldb() Emotionを読み込むように変更
	mob.h 変更
	pc.c
		pc_calcstatus() 変更
	skill.c

		skill_status_change_end() 変更
		skill_status_change_start() 変更とNULLチェック修正
	skill.h 変更

--------------------
//1027 by Ni+S
・getitemname関数追加
・スクリプトにgetitemname関数を追加
・itemidより、jnameを文字列で返します
・詳しくはscript_ref.txtで。

	script.c
		getitemname()追加

--------------------
//1026 by (凸)
・1023で入れてなかったclif.hを同梱
・バイオプラントとスフィアーマインで出したmobを倒すとmob_timer_delete()でnullpoが出る問題を解決したつもり
・バグ報告スレッド part6 >>63 Destさんのscript.c修正を取り込み

	(map/)
	clif.h 変更
	map.h 変更
	mob.c
		mob_changestate() 変更
	skill.c
		skill_castend_pos2() 変更
		mob_spawn() 変更
		do_init_mob() add_timer_func_listにmob_timer_deleteが無かったので追加してみた
	script.c
		script_load_mapreg() 変更

------------------------
//1025 by Sel
・ロードナイトのジョブ補正が間違っていたのを修正
・オーラブレード効果時間を修正
・オーラブレード武器制限を素手以外全てに修正
・コンセントレーション武器制限を両手槍のみから片手槍+両手槍へ修正
・トゥルーサイト効果時間を修正

	(db/)
	job_db2-2.txt 変更
	skill_cast_db.txt 変更
	skill_require_db.txt 変更
--------------------
//1024 by mare
・ファーマシー材料の変更、製造可能薬品の追加
	6/8日本鯖にきたものと同じにしました

	(db/)
	produce_db.txt 変更

--------------------
//1023 by (凸)
・1022でエンバグしたnpc_parse_script()を戻し
・スパイラルピアースの重量追加ダメージ計算式をちょっと変更
・魔法力増幅の計算式をちょっと変更
・テンションリラックスが座って使うのではなく使うと座るという情報を見かけたので変更
・↑に伴いskill_requireのsitting条件廃止
・バーサークをGvGで使用できないように変更

	(db/)
	skill_cast_db.txt 変更
	skill_require_db.txt 変更
	(doc/)
	db_ref.txt 修正
	(map/)
	battle.c
		battle_calc_pc_weapon_attack() 変更
	clif.c
		clif_sitting() 追加
		clif_parse_ActionRequest() 変更
	npc.c
		npc_parse_script() 修正
	pc.h 変更
	pc.c
		pc_calcstatus() 変更
		pc_natural_heal_hp() 変更
		pc_setstand() 追加
	skill.c
		skill_castend_nodamage_id() テンションリラックスを使うと座るように
		skill_check_condition() ST_SITTING廃止
		skill_use_id() バーサークをGvGで使用できないように
		skill_status_change_timer() テンションリラックスは10秒ごとにSPを12消費
		skill_status_change_start() テンションリラックス変更
		skill_readdb() sitting廃止
	skill.h 変更

--------------------
//1022 by (凸)
・NULLチェック変更
・login,char,map終了時に開放されていなかったメモリを微妙に開放する努力をしてみた
・スパノビ爆裂波動実装、クリティカル+50
・スパノビボーナスを変更、最初から一度も死んでないJobLv70にAll+15、BaseLv99にMHP+2000

	(doc/)
	client_packet.txt S 01ed追加
	(char/)
	do_final() 変更
	do_init() 変更
	(login/)
	do_final() 追加
	do_init() 変更
	(map/)
	atcommand.c Destさんの変更を取り込み
	battle.c 同上
	chat.c NULLチェック変更
	chrif.c NULLチェック変更
	clif.c NULLチェック変更
		clif_parse() 変更
		clif_parse_sn_explosionspirits() 追加
		clif_parse_sn_doridori() 名前変更
	guild.c NULLチェック変更
	intif.c NULLチェック変更
	itemdb.c NULLチェック変更
	map.c NULLチェック変更
		do_final() 変更
	mob.c NULLチェック変更
	npc.c NULLチェック変更
		npc_parse_script() buf開放忘れ？を開放
	party.c NULLチェック変更
	path.c NULLチェック変更
	pc.c NULLチェック変更
		pc_calcstatus() スパノビ変更
	pet.c NULLチェック変更
	storage.c NULLチェック変更
	trade.c NULLチェック変更
	vending.c NULLチェック変更

--------------------
//1021 by Kalen
・プロンテラ冠婚品NPCにてタキシード販売
・Umbalaのワープ見直し
	D2Fのワープを本鯖使用に変更。重複ポイント修正
・UmbalaNPC修正
	ラベルを使用しなくいい場所は極力削除(-)
	セーブポイント修正
	バンジージャンプ台追加
	骸骨門追加
	分解、合成処理追加
	カプラ、案内要員をあるべき場所へ移動。
	 ※一部Emoについて、癌呆自身が逆に取り違えているみたいなので独自で変えました。
	 見てもらえば分かりますが18と28を逆にするとNPCの会話内容に合うEmoが出たので
	クエストフラグの条件追加
	　これによりすでに終えている場合でも途中になる可能性があります。
・スパノビ転職NPC追加
	凸さんのログを基に作成しました。
・アルケミストギルドのノビの場合の対応修正(凸さんからのログより)
・結婚NPC追加
	ただし、まだテスト段階です。問題点があるため結婚不可能です。
	(/script)
	 (/warp)
	  npc_warp_umbala.txt
	 (/npc)
	  (/town)
	   npc_town_umbala.txt
	   npc_town_kafra.txt
	   npc_town_guide.txt
	   npc_town_prontera.txt
	  (/quest)
	   npc_event_marriage.txt(新・テスト)
	  (/job)
	   npc_job_alchemist.txt
	   npc_job_supernovice.txt(新)

--------------------
//1020 by (凸)
・nullpoの変更に対応してmap_athena.confの設定削除＆skill.c書き換え
・座っているとHPRとSPRが通常の半分で発動したのを修正
・スパノビが一定条件(クライアント依存)で/doridoriするとSPR回復量が倍になるように変更
・結婚式用のエフェクトをスクリプトからwedding命令で発生させることが出来るようした
・合奏を開始したPCは合奏中に終了できないようにしたつもり(未確認)

	(conf/)
	map_athena.conf nullpo_check削除
	(doc/)
	client_packet.txt 更新
	conf_ref.txt nullpo_check削除
	(map/)
	map.c
		map_config_read() nullpo_check削除
	map.h 同上
	skill.c NULLチェック再度総入れ替え
	clif.c
		clif_wedding_effect() 追加
		clif_parse_QuitGame() 合奏開始者は合奏中に終了できないように変更
		clif_parse_doridori() 追加
		clif_parse() doridori追加
	clif.h 変更
	pc.c
		pc_authok() doridori初期化追加
		pc_natural_heal_hp() 座っているときのHPR時間修正
		pc_natural_heal_sp() 座っているときのHPR時間修正、doridori追加
	script.c
		buildin_wedding_effect() 追加


--------------------
//1019 by Dest
・nullpoモジュールにコーディングミス発見/修正
・同、voidな関数から呼ばれた時のnullpo_retv_f()を追加
・同、条件コンパイルに対応

	(common/)
	nullpo.c
		nullpo_info_core() コーディングミス修正
	nullpo.h
		NULLPO_CHECKフラグによる条件コンパイル追加
		nullpo_retv_f() 追加

--------------------
//1018 by chloe
・ウンバラモンスターを追加

	(script/mob/)
		npc_monster.txt 変更
			各ウンバラマップにMob配置
	(db/)
		mob_db.txt 変更
			1495,STONE_SHOOTER,フレイムシューター 修正
			1511,AMON_RA,アモンラー 修正

--------------------
//1017 by (凸)
・バグ報告スレッド part6 >>46 Destさんのnullpoモジュールを追加＆voidな関数から呼ばれた時のnullpo_retv()を追加(とりあえずskill.cのNULLチェックだけ入れ替え)
・同>>39 Selさんから報告があったコンセントレーションを修正
・仕様について語り合うスレッド >>33 Kalenさんの情報をclient_packet.txtに反映
・本鯖相違スレッド part3 >>24 M ＠zqcM6jBwさんの情報を↑
・同>>30 はちさんの修正を反映

	(conf/)
	map_athena.conf nullpo_check追加
	(db/)
	item_db.txt 結婚指輪を武器-アクセサリに変更＆最新版
	(doc/)
	client_packet.txt 更新
	conf_ref.txt nullpo_check追加
	(common/)
	Makefile 変更
	nullpo.c 追加
	nullpo.h 追加
	(map/)
	Makefile 変更
	map.c
		map_config_read() nullpo_check追加
	map.h 同上
	skill.c NULLチェック総入れ替え
		skill_status_change_start() コンセントレーション修正
		skill_castend_nodamage_id() テレポート修正
	clif.c
		clif_skill_setunit() コメント修正
	pc.c
		pc_calcstatus() コンセントレーション修正

--------------------
//1016 by な。
・Athena雑談スレッドPart4 42 かる氏作のウンバラNPCを追加

	(script/npc/town/)
		npc_town_umbala.txt追加
			イベントデバッグ様(230～256行目)はコメントアウト
	(conf/)
		map_athena.conf town に npc: script/npc/town/npc_town_umbala.txt 追加

--------------------
//1015 by (凸)
・リムーブトラップを本鯖仕様とAthena仕様で選べるようにした
・スパノビにAll+10する条件が良く分からなかったけど最初から+10じゃないのは確かなのでとりあえずBase99で一度も死んでなければという条件に変更
・ダンス中に吹き飛ばされてもエフェクトは移動しないそうなので変更
・@go 13微調整


	(conf/)
	battle_athena.conf skill_removetrap_type追加
	(doc/)
	client_packet.txt 本鯖相違スレッド part3 23 M ＠zqcM6jBwさんの情報を追加
	conf_ref.txt skill_removetrap_type追加
	(map/)
	atcommand.c
		atcommand_go() ウンバラの出現位置微調整
	battle.c
		battle_config_read() skill_removetrap_type追加
	battle.h 同上
	pc.c
		pc_calcstatus() スパノビAll+10条件変更
	skill.c
		skill_blown() ダンス中の吹き飛ばしを元に戻した
		skill_castend_nodamage_id() リムーブトラップ仕様変更

 --------------------
//1014 by (Pepermint)
I fixed again the problem if you put minus sign(-) in front of digits,
the error comes up when you puchase a item.
When you put a minus sign(-), the error sign will be changed shrotage of
amount as original server dose.

I tested with it in ver. 1013, it was working

--------------------
//1013 by (凸)
・サーバースナップショット

--------------------
//1012 by (凸)
・help.txtにある@goの説明から13と14を削除
　機能は消えてないので使えることは使えますが、helpに載せるのはjROに来てからということで
・cast_db.txtをskill_cast_db.txtに改名
・ついでにmake cleanでGNUmakefileも消してみることにする
・u-0さんの検証などを総合してダンスユニット関連を変更
　演奏・ダンス中のハエはユニット付きで飛ぶそうです
　演奏・ダンス中にワープポイントに乗ったら状態が解除されるようです
　合奏中に片方がハエ飛びしたらエフェクトは元の場所に残り、飛んだ先では合奏状態が継続して動けない＆時間ごとにSP消費するようです
・PC_DIE_COUNTERがスクリプトから変更された時にすぐに反映されるように変更

	(conf/)
	help.txt @go説明変更
	(db/)
	skill_cast_db.txt 改名
	(map/)
	pc.c
		pc_setpos() ダンス中断のタイミング変更
		pc_setglobalreg() PC_DIE_COUNTER特別処理追加
	skill.c
		skill_castend_id() NULLチェック変更
		skill_stop_dancing() マップ移動などでの挙動を変更
		skill_readdb() skill_cast_dbに改名
		skill_blown() 飛ばされたらダンス移動
	npc.c
		npc_touch_areanpc() ワープポイントに乗ったらダンス解除

--------------------
//1011 by パイン
・パーティ要請やギルド要請を出しているときに他の要請を拒否る処理を追加
　battle_athena.confで切り替えが出来るようになっています。
・make clean で事が足りるので、objectdel.batを消去

	(map)
	clif.c
		clif_party_invite()変更(nullpoの文言違ってました)
	guild.c
		guild_invite()変更
	party.c
		party_invite()変更
	battle.c
		battle_config_read()変更
	battle.h変更
	trade.c
		trade_traderequest()変更

--------------------
//1010 by (凸)
・gcc 2.95対策
・スパノビの1度死ぬまでAll+10をスクリプト変数で実装してみる試み
	PC_DIE_COUNTER 死ぬと+1、転職すると0になります
	↑はキャラクタ永続なスクリプト変数なのでNPCで参照＆再設定とか比較的楽かも

	(map/)
	chrif.c
		chrif_divorce() gcc2.95対策
	skill.c
		skill_attack() gcc2.95対策
		skill_unit_move_unit_group() gcc2.95対策
	map.h map_session_dataにdie_counterを追加
	npc.c
		npc_click() nullpoメッセージを修正
	pc.c
		pc_setrestartvalue() nullpoメッセージを修正
		pc_authok() die_counter初期化の追加
		pc_calcstatus() die_counter=0のスパノビはAll+10
		pc_damage() PC_DIE_COUNTER設定
		pc_jobchange() PC_DIE_COUNTER設定
		pc_readparam() gcc2.95対策
		pc_divorce() gcc2.95対策
		pc_get_partner() gcc2.95対策

--------------------
//1009 by ぴざまん
・ダンス移動軽量化実装
　battle_athena.confで切り替え可能です。詳しくはconf_refを
　オリジナルアップデートの為、デフォルトではoffにしています
　また、この軽量化モードは回線負荷を大幅に削減できる(つもり)ですが、
　その分サーバー側の処理が重たくなります(といってもある程度のCPUがあれば全然問題にならない程度の負荷ですが)
・結婚システム／結婚スキル実装
　結婚指輪の特別扱いが未実装です(落とせたり取引に出せたりします)
　離婚のみ未テストです。
・結婚用スクリプト(marriage、divorce)追加。
　・marriage <partner_name>
　　<partner_name>: 結婚相手の名前
　　戻り値: 成功:1　失敗:0
　　結婚処理を行います。対象は話し掛けたプレイヤーと<partner_name>のキャラクターで、どちらかが既婚の場合は失敗します。
　　1回のmarriageスクリプトで新郎→新婦と新婦→新郎の結婚処理を同時に行います。
　　また、このスクリプトで結婚指輪は与えられません。
　・devorce
　　引数無し
　　戻り値: 成功:1　失敗:0
　　離婚処理を行います。対象は話し掛けたプレイヤーで、未婚の場合は失敗します。
　　marriageスクリプトと同様に互いの離婚処理を同時に行います。
　　また、このスクリプトが成功すると自動的に対象キャラクターの結婚指輪は剥奪されます。

　どちらのスクリプトの処理も、2人共ログインしていないと成功しません。

	(map/)
	pc.c
		pc_ismarried()、pc_marriage()、pc_divorce()、pc_authok()変更
		pc_get_partner() 追加
	map.c
		map_quit() 変更
	skill.c
		skill_use_id()、skill_castend_nodamage_id() 変更
		skill_unitsetting()、skill_unit_onlimit() 変更
	script.c
		buildin_marriage()、buildin_devorce() 追加
	pc.h 変更
	battle.h 変更

--------------------
//1008 by ぴざまん
・GMセッション隠し実装(未テスト)
　GMアカウントのキャラクターを@who等で表示するかどうか設定できます。
　詳しくはconf_ref.txtを参照してください。
・結婚システム仮実装
　char-map間通信のみ実装です。まだ結婚することは出来ません。
　尚、このパッチからathena.txtのバージョンが変わります。
　データの互換性は保ったつもりですが、念の為バックアップを取っておく事を強く推奨します。

	(map/)
	clif.c
		clif_countusers() 変更
	battle.c
		battle_config_read() 変更
	atcommand.c
		atcommand_who() 変更
	pc.c
		pc_ismarried()、pc_marriage()、pc_divorce() 追加
	chrif.c
		chrif_divorce() 追加
		chrif_parse() 変更
	(char/)
	char.c
		char_divorce() 追加
		char_delete()、mmo_char_fromstr()、mmo_char_tostr() 変更

--------------------
//1007 by (凸)
・NULLチェックの見直し

	(map/)
	clif.c
		clif_send() 変更
	skill.c
		skill_delunit() 変更

--------------------
//1006 by (ruhu)
･@goに洛陽とニフルヘイム追加

	(conf/)
	help.txt ニフルヘイム、洛陽追加
	(map/)
	atcommand.c 
		atcommand go() ニフルヘイム、洛陽追加
--------------------
//1005 by (凸)
・NULLチェックの見直しと追加、大量に変更したので問題が出る可能性が大です
・ウンバラ分割パッチが配布されたので@go 12にウンバラを追加
・ロードナイト バーサークをちょっとそれっぽく(ASPD増加などは未実装)
・クラウン･ジプシー 月明りの泉に落ちる花びらの使用条件を合奏と同じに変更(効果等は未実装)
・ブレッシングが本鯖ではステータスアップ→エフェクトだったのでそのように変更
・本鯖相違スレッド part2 >>145 zzzさんの報告を反映
・同スレ >>143 plalaさんの報告を反映

	(db/)
	skill_require_db.txt 変更
	(conf/)
	help.txt ウンバラ追加
	map_athena.conf ウンバラ関連のコメントアウトを解除
	(map/)
	atcommand.c NULLチェック強化
		atcommand() @mapmoveなどで落ちる問題を修正
		atcommand_go() ウンバラ追加
	battle.c NULLチェック強化
		battle_calc_mob_weapon_attack() バーサーク時ダメージ150%
		battle_calc_pc_weapon_attack() バーサーク時ダメージ150%
	chat.c NULLチェック強化
	chrif.c NULLチェック強化
	clif.c NULLチェック強化
		clif_parse_ActionRequest() ダンス時の処理を若干変更
		clif_parse_GlobalMessage() バーサーク時は会話が出来ないように変更
		clif_parse_Wis() バーサーク時は会話が出来ないように変更
		clif_parse_PartyMessage() 同上
		clif_parse_GuildMessage() 同上
		clif_parse_TakeItem() バーサーク時はアイテムを取れないように変更
		clif_parse_DropItem() バーサーク時はアイテムを落とせないように変更
		clif_parse_UseItem() バーサーク時はアイテムを使えないように変更
		clif_parse_EquipItem() バーサーク時は装備できないように変更
		clif_parse_UnequipItem() バーサーク時は装備解除できないように変更
		clif_parse_UseSkillToId() バーサーク時はスキル使用ができないように変更
		clif_parse_UseSkillToPos() 同上
		clif_parse_UseSkillMap() 同上
	guild.c NULLチェック強化
	intif.c NULLチェック強化
	itemdb.c NULLチェック強化
	map.c NULLチェック強化
		map_quit() バーサーク時にログアウトするとHP 100/SP 0になるように変更
	mob.c
		mob_attack() clif_fixmobpos()を送信しないように変更
		mob_timer() NULLチェック条件を変更
		mobskill_castend_id() 同上、バーサーク時スキルを使えないように変更
		mobskill_castend_pos() バーサーク時スキルを使えないように変更
		mobskill_use_id() 同上
		mobskill_use_pos() 同上
	npc.c NULLチェック強化
	party.c NULLチェック強化
	path.c NULLチェック強化
	pc.c NULLチェック強化
		pc_calcstatus() バーサーク時は速度UP＆MHP3倍、メディテイティオのSP回復増加をSPRではなく通常回復にかかるようにした
		pc_heal() バーサーク時は回復しない
		pc_jobchange() 転職直後1歩動かないと服の色が反映されなかったのを修正
		pc_natural_heal_sub() バーサーク中はSPが自然回復しないように変更(HPは良く分からなかったので回復するようにしてある)
	pet.c NULLチェック強化
	skill.c
		skill_castend_nodamage_id() ブレッシング等のパケット順を変更、ストーンカースを不死には無効にした
		skill_castend_map() しつこいくらいにバーサーク時にスキルを使えないように変更
		skill_check_condition() 同上
		skill_use_pos() 同上
		skill_use_id() 同上＆月明りの泉に落ちる花びら追加
		skill_status_change_end() バーサーク時にはIAアイコンを消去するように変更
		skill_status_change_timer() NULLチェック条件を変更、月明りの泉に落ちる花びら時SP消費、バーサーク時HP100以上なら10秒あたり1%減らすように変更
		skill_status_change_start() バーサーク時にはIAアイコンを表示するように変更
		skill_delunit() NULLチェック条件を変更
		skill_check_condition_char_sub() 月明りの泉に落ちる花びら追加
		skill_check_condition_use_sub() 同上
		skill_is_danceskill() 同上
		skill_initunitgroup() 同上
	trade.c NULLチェック強化
	vending.c NULLチェック強化

--------------------
//1004 by (凸)
・NULLチェックでエンバグしていたところをいくつか修正と他のNULLチェック強化
・skill.cでもnullpoを表示するように変更

	(map/)
	battle.c
		battle_damage() NULLチェック強化
		battle_heal() 同上
	clif.c
		clif_damage() 同上
	map.c
		map_addflooritem() NULLチェック強化
	mob.c
		mob_once_spawn() 余計なNULLチェックを削除
		mob_once_spawn_area() 同上
		mob_damage() 同上
		mob_counttargeted() 同上
		mobskill_castend_id() 同上
		mob_summonslave() メッセージ間違いを修正
	pc.c
		pc_damage() NULLチェック強化
	skill.c ほぼ全部 NULL関連修正

--------------------
//1003 by (凸)
・jROクライアントで/account使用時に繋がらない原因の0x200パケット問題を修正
・0x1c9パケットの穴埋めを引き続き継続中
・一人で聖体や合奏を使用できる設定(player_skill_partner_check)を追加
	一人で合奏を実行した場合には通常のダンスと挙動が同じになります
・プロボックは不死に対して発動しないように変更
・FWの吹き飛ばし判定を変更
・デリュージに水場判定を追加、聖水を作れるのを確認
・ハイディングの有効時間が正しく機能するように修正
・アスペルシオを不死に使用した場合、聖属性の40ダメージを与えるように変更
・アスペルシオを不死以外のMOBに使用しても効果が無いように変更
・合奏、ダンス、演奏中の経過時間によるSP消費を実装
・報告のあったガーディアンがギルド未加入PCをターゲットしたらmapが落ちる問題の修正(ガーディアン実装当時からずっと落ちてた？)
・mob.c、storage.cのnullチェックを強化
	エラーで落ちるべきところを無理やり通常処理に戻しているので他で問題がでるかもしれません
	その場合、コンソールに「関数名 nullpo」と表示されるので表示された場合は報告をお願いします
	もしかしたら正常な処理でも表示されるかもしれませんが、その際も報告をお願いします
	本来エラー、実は正常、どちらにしてもこれが表示されるのはバグです

	(conf/)
	battle_athena.conf player_skill_partner_check追加
	(db/)
	skill_db.txt アスペルシオの属性を聖に変更
	(doc/)
	conf_ref.txt player_skill_partner_check説明追加
	(login/)
	login.c
		parse_login() 0x200パケット対応
	(map/)
	mob.c NULLチェックでほぼ全部
	storage.c NULLチェックでほぼ全部
	battle.h 変更
	battle.c
		battle_calc_magic_attack() アスペルシオを追加、FWを変更
		battle_config_read() 設定追加
	clif.c
		clif_getareachar_skillunit() 調査結果の反映
		clif_skill_setunit() 同上
	[1001と1002の間の変更点]
	skill.c NULLチェックを沢山
		skill_castend_damage_id() アスペルシオ追加
		skill_castend_nodamage_id() アスペルシオ、プロボック処理変更
		skill_castend_id() アスペルシオ処理変更
		skill_check_condition_char_sub() player_skill_partner_checkに対応
		skill_check_condition_use_sub() 同上
		skill_use_id() 同上
		skill_check_condition() 同上＆デリュージ対応
		skill_status_change_timer() ハイディング修正、ダンス演奏合奏中のSP消費実装
		skill_initunitgroup() ダンスSP消費用変更
		skill_status_change_start() 同上＆プロボックをボスに効かないように(でもbattle.cで止めてるから通常ここまで来ない)

--------------------
//1002 by ぴざまん
・ポータルバグ修正
・スキル関係のNullチェック強化(by(凸))
	(map/)
	skill.c
		skill_castend_map() 修正

--------------------
//1001 by (凸)
・0x1c9でいくつかパケットを見比べて変化のないところを固定値で埋め込み(今後情報が集まると変わる可能性大)
・ダンス中はSPだけ回復しないように変更
・聖体で相方のSPが10以下だと使用失敗にして使用したらSPを10減らしてみる(未確認)
・合奏のスキル使用パケットを若干変更

	(map/)
	clif.c
		clif_getareachar_skillunit() 0x1c9の穴埋め開始
		clif_skill_setunit() 0x1c9の穴埋め開始
	pc.c
		pc_natural_heal_sub() ダンス中はSPのみ回復しないように変更
	skill.c
		skill_check_condition() 変更
		skill_check_condition_char_sub() 聖体は相方のSPチェックするように変更
		skill_check_condition_use_sub() 聖体は相方のSPを減らすように変更

--------------------
//1000 by ぴざまん
・0999の変更取り戻し
・トラップの巻き込み実装
・イドゥンの林檎でNPCまで回復した(ように見える)問題修正(未テスト)
・デモンストレーションのエフェクト変更(こちらに明記)
	(map/)
	skill.c
		skill_count_target() 追加
		skill_unit_onplace()、skill_trap_splash() 変更
--------------------
//0999 by eAthena Dev Team (Yor's Fixes)
(login/)
	added email for accounts
(char/)
	added email for character deletion
--------------------
//0998 by (凸)
・battle.cで引数の存在を確認せずに値を見に行ってる関数をいくつか修正
・ダンス中は座れないようにした(本鯖相違スレッド part2 >>114 DoTさん)
・ダンス中はHP、SPが回復しないように変更(同上)
・ダンス中は通常攻撃できないようにした(本鯖相違スレッド part2 >>116 ・・・さん)
・合奏中片方が落ちた場合、残ったほうで演奏を継続するようにした(同上)
・合奏中はアドリブ以外できないように変更(同上)
・合奏発動条件にダンス中じゃない＆座っていないを追加(本鯖相違スレッド part2 >>118 ろろさん)
・阿修羅使用後にHPも回復しなかったのを修正

	(map/)
	battle.c
		battle_counttargeted() 修正
		battle_get系 たぶん全部修正
	clif.c
		clif_parse_WalkToXY() 合奏の判断方法の変更
		clif_parse_ActionRequest() ダンス中は殴らない座らないように変更
	map.c
		map_quit() ダンス中断追加
	mob.c
		mob_damage() skill_stop_dancingの引数増加による変更
	pc.c
		pc_setpos() 同上
		pc_damage() 同上
		pc_equipitem() 同上
		pc_natural_heal_sub() ダンス中は自然回復しないように＆阿修羅時はSPのみ回復しないように
	skill.c
		skill_castend_nodamage_id() skill_stop_dancingの引数増加による変更
		skill_status_change_start() 同上
		skill_check_condition_char_sub() 相手がダンス中や座っていても合奏できないように変更
		skill_check_condition_use_sub() 同上
		skill_use_id() 合奏の判断方法の変更＆合奏中はアドリブ以外禁止に
		skill_status_change_end() 合奏のダンス状態解除は相手のval4を0にするようにした
		skill_is_danceskill() 戻り値変更
		skill_stop_dancing() 引数増加、合奏で片方だけ落ちたときの処理追加
		skill_delunitgroup() 合奏時のステータス変更処理を↑に移した
		skill_clear_unitgroup() 自分のbl->idとユニットグループのgroup->src_idが違うときは削除しないように変更
	skill.h 変更

--------------------
//0997 by (凸)
・二人で合奏、三人で聖体を実装
	合奏 バード・ダンサーが隣接したセルにいて同じスキルを持っている時に発動
		スキルレベルは両者のスキルレベルの中間
		本鯖仕様が分からないけど斜め隣接もOK
	聖体 使用者のX軸で-1と+1の場所に一人ずつアコライトかプリーストがいる時に発動すると思う(未テスト)
		使用者 ○ アコプリ ●
		OK ●○●

		NG ●
		   ○●
・トーキーボックスを使って設置時に座っていた時はスキル使用失敗にしてみた

	(db/)
	item_db.txt 村正の呪い率を5%に(by e2さん)
	(map/)
	skill.c
		skill_check_condition_char_sub() 追加
		skill_check_condition_use_sub() 追加
		skill_check_condition() 変更
		skill_use_id() 変更
		skill_initunitgroup() 変更
		skill_delunitgroup() 変更
	clif.c
		clif_parse_UseSkillToPos() 変更

--------------------
//0996 by (凸)
・アドリブの5秒制限をcast_db.txtのupkeep_time2で制御できるように変更
・村正で自分を呪わせるためにbonus2 bAddEff2を追加
・呪われていて終了できない時にも「今は終了できません」が表示されるようにした
・ダンスエフェクト等移動時に効果の適用判断が移動前に行われていたのを移動後に変更＆使わなくていい変数を廃止

	(db/)
	cast_db.txt
	const.txt
	item_db.txt
	(doc/)
	item_bonus.txt
	(src/)
	clif.c
		clif_parse_QuitGame() 変更
	map.h addeff2,arrow_addeff2 追加
	pc.c
		pc_calcstatus() 変更
		pc_bonus2() 変更
	skill.c
		skill_additional_effect() 変更
		skill_check_condition() 変更
		skill_unit_move_unit_group() 変更

--------------------
//0995 by (凸)
・battle.pet_lootitemのデフォルトがyesになっていたのを修正
・battle.pet_lootitemの適用をforeach前でやるように修正
・ペットの初期化でpd->lootitemがある時しか初期化されていなかったのを修正
・領域が移動して対象が領域から抜けても効果が解除されない問題を修正
・アドリブはダンス発動から5秒以上経たないと使用できないようにした

	(map/)
	pet.c
		pet_data_init() 修正
		pet_ai_sub_hard() 修正
		pet_ai_sub_hard_lootsearch() 修正
	skill.c
		skill_blown() 変更
		skill_unit_onlimit() エラーメッセージ修正
		skill_check_condition() 変更
		skill_initunitgroup() sd_data[SC_DANCING].val3にgettick()
		skill_unit_move_unit_group_sub() 追加
		skill_unit_move_unit_group() 変更
	skill.h skill_unit_move_unit_group() 引数変更
	battle.c
		battle_config_read() pet_lootitem=0に修正
	pc.c
		pc_walk() 変更

--------------------
//0994 by huge
・battle.pet_lootitemが適応されていなかったのを修正。
・ペットにパフォーマンスをさせた後、10秒間くらいは拾わせないように。

	(map/)
	map.h	pet_dataにlootitem_timer追加。
	pet.c
		pet_ai_sub_hard_lootsearch() 修正。
		pet_lootitem_drop() 修正。

--------------------
//0993 by (凸)
・一人で合奏だけど合奏スキル中は動けないように
・アンコール実装。直前に使ったダンススキルを半分のSPで使えます
・ダンス中に移動するとエフェクトも移動するようにした
・未実装のスキルが使われるとUnknown skillと表示されることがあるかもしれません
・Sageのキャストキャンセルで実は前のスキルを覚えていなかった問題の修正

	(map/)
	clif.c
		clif_parse_WalkToXY() 変更
	map.h アンコール用変数の追加
	pc.c
		pc_walk()
	skill.c
		skill_blown
		skill_castend_nodamage_id
		skill_unitsetting() 変更
		skill_unit_onplace() 罠で同じ処理をしているcaseをまとめた
		skill_unit_onout() 使われてないunit2を削除
		skill_check_condition() 変更
		skill_use_id
		skill_initunitgroup() 使ったダンススキルを変数に入れるようにした
		skill_unit_move_unit_group() 追加
	skill.h 変更

--------------------
//0992 by nokia

map_quitを修正してメモリを釈放する時何度もを釈放するためメモリの間違いが起こる問題を招く

	(map/)
	map.c
		map_quit()

--------------------
//0991 by (凸)
・トーキーボックスを自分が踏んでも発動しない本鯖仕様に変更
・スキル詠唱中とディレイ中はクライアントを終了できないようにしたけど、敵に攻撃されているときは終了できます(ごめんなさいっ！！)

	(map/)
	clif.c
		clif_parse_QuitGame() 変更
	skill.c
		skill_unitsetting() 変更

--------------------
//0990 by ぴざまん
・一部のトラップを範囲攻撃に変更（巻き込みは未実装）。
・mapflagにpvp_nocalcrankとpvp_nightmaredrop追加。
	・<gatname><tab>mapflag<tab>pvp_nocalcrank<tab>dummy
	　PvPによるランキング計算をしないようにします。
	・<gatname><tab>mapflag<tab>pvp_nightmaredrop<tab><item>,<type>,<per>
	　PvPにて死亡した場合、<per>の確率でアイテムをドロップします。
	　<item>: ドロップするアイテムIDを指定します。randomと記述すると所持品からランダムにドロップします。
	　<type>: ドロップするアイテムのタイプを指定します。inventory:所持品 equip:装備品 all:全部
	　<per>: ドロップする確率です。万分率で指定します。
	(map/)
	skill.c
		skill_unit_onplace() 変更
		skill_trap_splash() 追加
	pc.c
		pc_damage() 変更
	npc.c
		npc_parse_mapflag() 変更
	map.h
		map_dataにメンバ追加
	(conf/)
	mapflag.txt
		ナイトメアモードにドロップ設定追加

--------------------
//0989 by (凸)
・スプリングトラップを某所で見たSSを元に実装してみた。違ったら相違スレへ
・ディテクティングを某所で見た解説文を元に実装してみた。違ったら相違スレへ
・キャストキャンセル時のdelete_timerエラーに追加でスキルIDを表示するようにした。あわせて報告するとエラーの原因が分かるかも

	(map/)
	skill.c
		skill_castend_nodamage_id() 変更
		skill_castend_pos2() 変更
		skill_castcancel() 変更

--------------------
//0988 by (凸)
・トーキーボックスは常に足元に置けるように変更とKalenさんに貰った情報等を元にパケットを本鯖にあわせてみた
・ショックウェーブトラップに効果を追加したつもり(未確認)
・ブラストマインとクレイモアトラップを殴ると吹き飛ぶようにした
・リムーブトラップで罠(skill_require_dbで指定したアイテム)を回収できるようにした
・↑の実装により罠の時間切れで罠が出るオリジナル仕様はコメントアウト
・ブラストマインの効果時間が長いのを修正

	(db/)
	cast_db.txt
	(map/)
	battle.c
		battle_check_target() 変更
	mob.c
		mobskill_castend_pos() 変更
	skill.c
		skill_additional_effect() 変更
		skill_blown() 変更
		skill_castend_nodamage_id() 変更
		skill_castend_pos2() 変更
		skill_unitsetting() 変更
		skill_unit_onplace() 変更
		skill_unit_ondamaged() 変更
		skill_castend_pos() 変更
		skill_unit_timer_sub() 変更

--------------------
//0987 by 胡蝶蘭

・ユーザー定義関数/サブルーティンに引数を指定可能に
	詳しくは script_ref.txt を参照
	
	(map/)
	scirpt.c
		buildin_getarg()追加
		buildin_callsub(),buildin_callfunc(),run_func()修正
	(doc/)
	script_ref.txt
		引数についての説明追加

・getguildname,getpartyname,getcastlename,strcharinfo修正
	C_STRで定数文字列(C_CONSTSTR)を返していた問題修正
	strcharinfoでギルド名などを所得する際、未所属だったときの問題
	
	(map/)
	script.c
		buildin_getguildname(),buildin_getpartyname()
		buildin_getcastlename(),buildin_strcharinfo()

--------------------
//0986 by (凸)
・client_packetの記述から推察してトーキーボックスを実装してみたつもり

	(db/)
	skill_db.txt
	(map/)
	clif.c
		clif_talkiebox() 追加
		clif_parse_UseSkillToPos() 変更
		clif_parse() 変更
	clif.h 変更
	map.h 変更
	skill.c
		skill_castend_pos2() 変更
		skill_unitsetting() 変更
		skill_unit_onplace() 変更

--------------------
//0985 by (凸)
・サーバーsnapshot
・/script/extensionに語り部を追加したり
・0984でAthena雑談スレッド part3>>92 Michaelさんの修正を取り込んでた
・バグ報告スレッド part5 >>45に転載されてた あやねさんの修正を取り込み
・バグ報告スレッド part5 >>54 rariさんのたぬき帽修正を取り込み

	(script/extension/)
	npc_event_kataribe6.txt 追加
	npc_event_kataribe7.txt 追加
	(script/npc/)
	quest/npc_event_hat.txt 修正
	job/npc_job_magician.txt 修正

--------------------
//0984 by (凸)
・セージのフリーキャストというスキルの存在を知らなかったので0983の変更を一部戻し
・アイテム更新したけど垂れ猫とインディアンバンダナが装備できない？
・マップフラグとモンスター配置を最新版に

	(db/)
	item_db.txt
	(conf/)
	mapflag.txt
	(script/mob/)
	npc_monster.txt
	(map/)
	clif.c
		clif_parse_WalkToXY() 修正

--------------------
//0983 by (凸)
・長い詠唱中に歩きまわれた気がするので修正
・ペットルーレット回転中に対象が叩き殺されたらmap-serverが困るのを修正
・プロフェッサー ライフ置き換えの手抜き処理をちょっとマシにした

	(map/)
	clif.c
		clif_parse_WalkToXY() 修正
	pet.c
		pet_catch_process2() 修正
	skill.c
		skill_castend_nodamage_id() 修正

--------------------
//0982 by (凸)
・転生スキルの～
	ストーカー リジェクトソード 相手がPCの場合は剣じゃなければ跳ね返さない予定
	プロフェッサー メモライズ 12秒の固定キャストタイム、その後スキル使用のキャストタイムが3回だけ1/3になる
	プロフェッサー ライフ置き換え HPを10%減らしてSPを増やす。減ったHPのエフェクトは無しにしてみた
・見切りと回避率増加のFlee上昇がステータス変化時にしか反映されていなかったのを修正
・0981で自分以外は歩いたら服の色が戻ってしまうのをなんとかしたつもり

	(db/)
	skill_require_db.txt
	(map/)
	battle.c
		battle_calc_damage() 修正
	clif.c
		clif_movechar() 修正
		clif_getareachar_pc() 修正
	pc.c
		pc_calcstatus() 修正
	skill.c
		SkillStatusChangeTable 変更
		skill_castend_nodamage_id() 変更
		skill_use_id() 変更
		skill_use_pos() 変更
		skill_status_change_timer() 変更
		skill_status_change_start() 変更
	skill.h 変更
	
--------------------
//0981 by (凸)
・転生スキルの～
	ストーカー リジェクトソード 一定確率でダメージを半分にして減らした分を相手に跳ね返すようにした…が、跳ね返したダメージのエフェクト出ません
・服の色を変更＆保存している場合にリログすると元の色に戻ってるように見える問題の修正

	(db/)
	skill_db.txt
	(map/)
	clif.c
		clif_parse_LoadEndAck() 修正
	battle.c
		battle_calc_damage() 修正
	skill.c
		skill_status_change_timer() 修正
		skill_status_change_start() 修正

--------------------
//0980 by (凸)
・例によって転生スキルとモンク関連
	スナイパー シャープシューティング クリティカル率調整？
	ハイウィザード ソウルドレイン MSP増量とMobを倒したときにSP(mobLv*(65+15*SkillLv)/100)回復。でも本当は範囲攻撃の場合は回復しないらしい？
	ハイウィザード 魔法力増幅 使ったらMATKがSkillLv%増量。次のスキル使用時に元に戻る
	モンク 気奪 ちょっと先取りして20%の確率で敵のLv*2のSPを吸収。成功したときはターゲットを取得するようにしてみた
・アイテム名をjROウンバラに準拠させてみたつもり

	(db/)
	cast_db.txt
	item_db.txt
	(map/)
	battle.c
		battle_calc_pc_weapon_attack() 変更
	mob.c
		mob_damage() 変更
	pc.c
		pc_calcstatus() 変更
	skill.c
		skill_castend_nodamage_id() 変更
		skill_use_id() 変更
		skill_use_pos() 変更
		skill_status_change_end() 変更
		skill_status_change_timer() 変更
		skill_status_change_start() 変更

--------------------
//0979 by (凸)
・転生スキルをちょっと調整
	ハイウィザード マジッククラッシャー 武器攻撃でBaseATK計算をMATK2でしてみる
・息吹を本鯖仕様風にHPとSPの回復タイマーを分けて座っていなくても動かなければタイマーが進むようにした
・阿修羅使用後にHPとSPが5分間自然回復しない本鯖使用風にした(csat_db.txtのupkeep_time2で調整可能)
・古いgccでskill_unit_timer_sub_onplace()あたりでコンパイルエラーになったのを修正

	(db/)
	cast_db.txt
	(map/)
	battle.c
		battle_calc_pet_weapon_attack() 変更
		battle_calc_mob_weapon_attack() 変更
		battle_calc_pc_weapon_attack() 変更
	map.h
	pc.c
		pc_authok() 変更
		pc_walk() 変更
		pc_spirit_heal() 削除
		pc_spirit_heal_hp() 追加
		pc_spirit_heal_sp() 追加
		pc_natural_heal_sub() 変更
	skill.c
		skill_additional_effect() 変更
		skill_castend_damage_id() 変更
		skill_status_change_start() 変更
		skill_unit_timer_sub_onplace() 修正

--------------------
//0978 by (凸)
・転生スキルを修正したり色々
	スナイパー ファルコンアサルト とりあえず飛ぶだけだと思ってください
	スナイパー シャープシューティング ダメージ増加だけど1体だけ
	クラウン・ジプシー アローバルカン ダメージ増加と9回攻撃
	ハイウィザード マジッククラッシャー エフェクトだけ
	
・アイテム交換で重量の計算が違っていたのを修正
	(doc/)
	client_packet.txt パケット長テーブル更新
	(map/)
	battle.c
		battle_calc_misc_attack() 変更
		battle_calc_pet_weapon_attack() 変更
		battle_calc_mob_weapon_attack() 変更
		battle_calc_pc_weapon_attack() 変更
	clif.c パケット長の定義を更新
	skill.c
		skill_castend_damage_id() 変更
	trade.c
		trade_tradeadditem() 修正
	
--------------------
//0977 by (凸)
・転生スキルを修正したり色々
	アサシンクロス メテオアサルト エフェクトが違う？
	ロードナイト プレッシャー 必中ダメージにしてみた
	ロードナイト オーラブレードの必中damage2が他でも適用されていたのを修正
	ロードナイト ゴスペル エフェクト出現位置の調整
	ハイプリースト アシャンプティオ 効果実装
	ハイプリースト メディテイティオ 効果実装
	ハイプリースト バジリカ SGみたいにMobが侵入しようとすると吹き飛ばされるようにした
	ホワイトスミス カートブースト 効果実装
	ホワイトスミス メルトダウン エフェクトと状態異常時間だけ(実際の状態変化は無し)
	ホワイトスミス クリエイトコイン 名前入りの金貨とか作れるだけ
	ストーカー リジェクトソード エフェクトと状態異常時間だけ(実際の状態変化は無し)
	クラウン・ジプシー マリオネットコントロール エフェクトと状態異常時間だけ(実際の状態変化は無し)
	プロフェッサー フォグウォール エフェクトと有効時間だけ
	スナイパー ウインドウォーク 速度上昇とQM、私を忘れないでがかかると解除されるようにした
	スナイパー トゥルーサイト QM、私を忘れないでで解除されるようにしてみた
・トゥルーサイトの綴り間違いを修正
・storage.cでコンパイル警告が出ないようにしたつもり

	(db/)
	cast_db.txt
	skill_db.txt
	skill_require_db.txt
	produce_db.txt
	(map/)
	battle.c
		battle_get_str() 修正
		battle_get_agi() 修正
		battle_get_vit() 修正
		battle_get_int() 修正
		battle_get_dex() 修正
		battle_get_luk() 修正
		battle_get_flee() 修正
		battle_get_hit() 修正
		battle_get_critical() 修正
		battle_get_baseatk() 修正
		battle_get_atk() 修正
		battle_get_atk2() 修正
		battle_get_def() 修正
		battle_get_def2() 修正
		battle_get_speed() 修正
		battle_calc_damage() 修正
		battle_calc_pet_weapon_attack() 変更
		battle_calc_mob_weapon_attack() 変更
		battle_calc_pc_weapon_attack() 変更
	pc.c
		pc_calcstatus() 修正
	skill.c
		skill_get_unit_id() 修正
		skill_additional_effect() 修正
		skill_castend_nodamage_id() 修正
		skill_castend_pos2() 修正
		skill_unit_group() 修正
		skill_unit_onplace() 修正
		skill_unit_onout() 修正
		skill_castend_pos() 修正
		skill_check_condition() 修正
		skill_status_change_end() 修正
		skill_status_change_start() 修正
		skill_can_produce_mix() 修正
		skill_produce_mix() 修正
	skill.h 修正
	storage.c
		storage_comp_item() 修正
	storage.h 修正

--------------------
//0976 by (凸)
・転生スキルを修正したり色々
・準備だけして実装できてないスキルもあります
	ロードナイト オーラブレード 多分こんな感じ？
	ロードナイト パリイング 跳ね返すけど攻撃を1回止めるのは未実装
	ロードナイト コンセントレーション インデュア～は良く分からないので放置
	ロードナイト スパイラルピアース 通常ダメージ増加と重量ダメージ増加と一応5回攻撃(なんか違う気がする)
	ロードナイト ヘッドクラッシュ ダメージ増加とステータス変更？
	ロードナイト ジョイントビート ダメージ増加とステータス変更？
	アサシンクロス アドバンスドカタール研究 たぶんこんな感じ？
	スナイパー トゥルーサイト たぶんこんな感じ？
	スナイパー ウィンドウォーク たぶんこんな感じ？でも速度上昇とかと競合した時の処理は未実装
	スパイダーウェッブ とりあえずアンクルスネアと同じような感じ＆回避率半減
	チャンピオン 狂気功 適当に増やしていたのをちゃんと増やすようにした
・出血状態と骨折状態の取り扱いがよくわかりませんっ！！

	(db/)
	cast_db.txt
	skill_db.txt
	skill_require_db.txt
	(doc/)
	db_ref.txt
	(map/)
	battle.c
		battle_get_str() 修正
		battle_get_agi() 修正
		battle_get_vit() 修正
		battle_get_int() 修正
		battle_get_dex() 修正
		battle_get_luk() 修正
		battle_get_flee() 修正
		battle_get_hit() 修正
		battle_get_critical() 修正
		battle_get_baseatk() 修正
		battle_get_atk() 修正
		battle_get_atk2() 修正
		battle_get_def() 修正
		battle_get_def2() 修正
		battle_get_speed() 修正
		battle_calc_damage() 修正
	clif.c
		clif_parse_WalkToXY() 修正
	mob.c
		mob_can_move() 修正
		mobskill_castend_pos() 修正
	pc.c
		pc_calcstatus() 修正
		pc_checkallowskill() 修正
	skill.c
		skill_get_unit_id() 修正
		skill_additional_effect() 修正
		skill_castend_nodamage_id() 修正
		skill_castend_pos2() 修正
		skill_unit_group() 修正
		skill_unit_onplace() 修正
		skill_unit_onout() 修正
		skill_castend_pos() 修正
		skill_check_condition() 修正
		skill_status_change_end() 修正
		skill_status_change_start() 修正
		skill_readdb() 修正
	skill.h 修正
--------------------
//0975 by Sin
・0973で実装されたスクリプトによるBaseLv, JobLvの変更時に、
　ステータスポイントやスキルポイントを取得できるように修正。
　※質問スレpart5 >>115 悩める人 さんのpc.cを参考にさせて頂きました。多謝。

	(map/)
	pc.c
		pc_setparam()
		case SP_BASELEVEL: 修正
		case SP_JOBLEVEL: 修正
--------------------
//0974 by latte
・グランドクロスを本鯖に基づき修正
	アンデッド悪魔強制暗闇付与
	反動ダメージ半減、モーションなし
	MOB(PC)が重なったときのHIT数
	%UP系武器カード効果なし
	属性相性二重計算
	MOBダメージ表示白

	後半4項目は設定可

・戦闘基本計算を本鯖に基づき微修正（DEXサイズ補正、弓最低ダメ、PCサイズ補正）

・完全回避
	スタン等で完全回避が発生しなかったのを修正
	AGIVITペナルティが完全回避の敵もカウントして計算されていたのを修正　設定可
		オートカウンターは未修正

・倉庫を閉じるとき、アイテムIDでソートするようにした

・kalenさんのプロ１執事NPCスクリプトを改造して
	商業防衛値、投資金額、宝箱の数を本鯖に準拠(商業値は1~100)
	全砦に設置
	
	(/script/npc)
	aldeg_cas01.txt ... prtg_cas05.txt 修正
	(/script/npc/gvg)
	aldeg_cas01.txt ... prtg_cas05.txt 追加
	tbox.txt 追加

	(/conf)
	battle_athena.conf
		6項目追加

	(/map)
	mob.c/mob.h
		mob_attack() 修正
		mob_counttargeted_sub(),mob_counttargeted() 修正
		mobskill_use() 修正（↑の引数だけ）
	pc.c/pc.h
		pc_counttargeted_sub(),pc_counttargeted() 修正
		pc_attack_timer() 修正
	pet.c
		pet_attack() 修正

	map.h
		pc_data, mob_data, map_data 変数１つ追加
		enum1つ追加
	map.c/map.h
		map_count_oncell() 追加 skill.cでよかったかも・・・

	skill.c	
		GX関連修正(skill_additional_effect(), skill_attack(), skill_castend_damage_id(), skill_unit_onplace())

	battle.c 修正
	battle.h 修正

	storage.c/storage.h
		storage_comp_item() 追加
		sortage_sortitem(), sortage_gsortitem() 追加
		storage_storageclose(), storage_guild_storageclose() 修正
--------------------
//0973 by 獅子o^.^o
・スクリプトのBASELEVEL,JOBLEVEL命令追加
	例: set BASELEVEL,1;
	例: set JOBLEVEL,1;
	(map/)
	pc.c
		pc_setparam()
		case SP_BASELEVEL: 項目追加
		case SP_JOBLEVEL: 項目追加
		
--------------------
//0972 by (凸)
・転生スキルをエフェクトだけいくつか追加したり
・状態変化はそのうち誰かが
	エフェクト(ステータス変化アイコン含む)のみ
		SC_AURABLADE:		/* オーラブレード */
		SC_PARRYING:		/* パリイング */
		SC_CONCENTRATION:	/* コンセントレーション */
		SC_TENSIONRELAX:	/* テンションリラックス */
		SC_BERSERK:		/* バーサーク */
		SC_ASSUMPTIO:		/*  */
		SC_TURESIGHT:		/* トゥルーサイト */
		SC_CARTBOOST:		/* カートブースト */
		SC_WINDWALK:		/* ウインドウォーク */

	(db/)
	cast_db.txt
	(map/)
	skill.h
	skill.c
		SkillStatusChangeTable[] 項目追加
		skill_castend_nodamage_id()  項目追加
		skill_status_change_end()  項目追加
		skill_status_change_start()  項目追加
		
--------------------
//0971 by (凸)
・atcommand.hに残っていたjobchange2とかの残骸を削除
・転生スキルをエフェクトだけいくつか追加したりチャンピオンはそれなりに追加したり
	エフェクトのみ
		ハイプリースト バジリカ(HP_BASILICA)
		ホワイトスミス カートブースト(WS_CARTBOOST)
		スナイパー トゥルーサイト(SN_SIGHT)
		ジプシー 月明りの泉に落ちる花びら(CG_MOONLIT)
		パラディン ゴスペル(PA_GOSPEL)
	追加ダメージ等なし
		ロードナイト ヘッドクラッシュ(LK_HEADCRUSH)
		ロードナイト ジョイントビート(LK_JOINTBEAT)
		ロードナイト スパイラルピアース(LK_SPIRALPIERCE)
		パラディン プレッシャー(PA_PRESSURE)
		パラディン サクリファイス(PA_SACRIFICE)
	それなり(コンボは繋がりますがディレイは適当、ダメージ追加はあるけどそれ以外の追加効果は無し)
		チャンピオン 猛虎硬派山(CH_PALMSTRIKE)
		チャンピオン 伏虎拳(CH_TIGERFIST)
		チャンピオン 連柱崩撃(CH_CHAINCRUSH)
		チャンピオン 狂気功(CH_SOULCOLLECT)

	(db/)
	cast_db.txt
	skill_db.txt
	skill_require_db.txt
	(map/)
	atcommand.h ゴミ削除
	battle.c
		battle_calc_pet_weapon_attack() 変更
		battle_calc_mob_weapon_attack() 変更
		battle_calc_pc_weapon_attack() 変更
	clif.c
		clif_parse_UseSkillToId() 変更
	skill.c
		skill_get_unit_id() 項目追加
		skill_attack() チャンピオンコンボ処理追加
		skill_castend_damage_id() 変更
		skill_castend_nodamage_id() 変更
		skill_castend_id() 変更
		skill_unitsetting() 変更
		skill_check_condition() 変更
		skill_use_id() 変更

--------------------
//0970 by (凸)
・ドレイクのウォーターボールが異常に痛い(121発食らう)のでLv5以上の場合は25発に制限
・シグナムクルシスの計算式を14+SkillLvから10+SkillLv*2変更
・ソースの気が向いたところに落書き
・DB関係をまとめて同梱

	(map/)
	battle.c コメント＿〆(。。)ｶｷｶｷ
	skill.c skill_status_change_start()
	(db/)
	cast_db.txt
	item_db.txt
	mob_skill_db.txt
	skill_db.txt
	skill_require_db.txt
	skill_tree.txt

--------------------
//0969 by ぴざまん

・白刃取り状態で片方が死亡した場合、片方の白刃取りが解除されない問題修正
・battle_athena.confに項目追加
　ペット・プレイヤー・モンスターの無属性通常攻撃を属性無しにするか否かを設定できます
　詳しくはconf_refを。
・＠コマンド@idsearch実装
　ロードしたitem_dbから検索語句にマッチするアイテムとIDを羅列するコマンドです
　例えば「@idsearch レイ」と入力した場合、ブレイドやレイドリックカード等が引っかかります
・アシッドテラーとデモンストレーション実装
　装備破壊は未実装です
・イドゥンの林檎の回復仕様を丸ごと変更。
	(map/)
	battle.c
	battle.h
		属性補正の修正やアシッドテラー・デモンストレーションのダメージ算出式追加等。
		battle_configに項目追加
	skill.c
		skill_idun_heal()追加。foreachinareaで処理するように変更
		アシッドテラーとデモンストレーションの処理追加。
	atcommand.c
	atcommand.h
		@idsearch追加。

--------------------
//0968 by 胡蝶蘭

・キャラクターIDが使いまわされないように修正
・キャラクター削除時、パーティー、ギルドを脱退するように修正
・アカウント削除時、キャラクターと倉庫を削除するように修正
・倉庫/ギルド倉庫削除時、倉庫内のペットを削除するように修正
	・注意：ログインしているアカウントを削除した場合の動作は不明

	(char/)
	char.c
		パケット2730の処理、char_delete()追加、削除処理修正など
	int_storage.c/int_party.c/int_guild.c/int_party.h/int_guild.h
		inter_party_leave(),inter_guild_leave()追加、
		inter_storage_delete(),inter_guild_storage_delete()修正など
	(login/)
	login.c
		parse_admin()をアカウント削除時にパケット2730を送るように修正

・athena-start stop で停止させた場合、データが保存されない問題を修正
	killで送るシグナルをSIGKILLからSIGTERMに変更。
	どうしてもSIGKILLを送りたい場合は athena-start kill を使ってください。
	
	athena-start
		stop修正、kill追加

--------------------
//0967 by Asong
・モンスターの残影を実装。
　通常モンスターはスキルによるフィルターがかからないので残像が出ません。
　ＰＣ型モンスターには残像が出ます。
・モンスタースキル使用対象を追加。
　around5～around8はターゲットの周辺セルを対象にします。
　
　	(map)
　	mob.c
　		mobskill_use() 修正
　		mob_readskill() 修正
　	mob.h 修正
　	skill.c
　		skill_castend_pos2() 修正

--------------------
//0966 by (凸)
・サーバーsnapshot
・ディレクトリ構造を変更(common,login,char,mapは/src以下に移転)
　それに伴うMakefile等のパス書き換え
・npc_turtle.txtをnpc_town_alberta.txtに統合
・モンクのコンボに関するディレイを変更
・battle_config.enemy_criticalのデフォルトをnoに変更
・転生職等を無効にするenable_upper_classの追加
・@joblvup,@charjlvlでJobレベルが最高のときに負数を指定してもレベルを下げられなかった問題を修正

	(conf)
	battle_athena.conf 修正
	(doc)
	conf_ref.txt 修正
	(map)
	atcommand.c
		atcommand_joblevelup() 修正
		atcommand_character_joblevel() 修正
	battle.c
		battle_calc_attack() 修正
		battle_config_read() 修正
	battle.h 修正
	pc.c
		pc_calc_skilltree() 修正
		pc_calc_base_job() 修正
		pc_jobchange() 修正
		pc_readdb() 修正
	skill.c
		skill_attack() 修正
--------------------
//0965 by ぴざまん
・@mapexit実行時全セッションをkickするように変更。
・白刃取り時に片方が倒れても、もう片方の白刃が解除されない問題修正。(未テスト)
・スティール情報公開機能実装。(未テスト)
　スティールに成功すると、何をスティールしたのか
　画面内のPTメンバー全員に知らせる機能です。
　battle_athena.confのshow_steal_in_same_partyで設定できます。
　オリジナルアップデートの為、デフォルトはnoにしています。
・イドゥンの林檎の回復効果実装。

	(conf/)
	battle_athena.confに項目追加。
	(map/)
	atcommand.c
		atcommand_mapexit() 修正。
	pc.c
		pc_steal_item() 修正。
		pc_show_steal() 追加。
	skill.c
		skill_unitsetting()、skill_unit_onplace() 修正。
	battle.c
		battle_config_read() 修正。
	battle.h 修正。
	(doc/)
	conf_ref.txt 抜けてたのを色々追加。

--------------------
//0964 by (凸)

・この前追加したskill_tree2.txtを廃止したので削除してください
・skill_tree.txtのフォーマットを変更＆Kalenさんなどの情報を元に転生ツリーの見直し
・それにともなってpc.cのファイル読み出し部分などを変更
・Athena雑談スレッド part3 >>14 miyaさんの指摘があるまですっかり忘れていたatcommand_athena.confの修正を同梱

	(conf/)
	atcommand_athena.conf 修正
	(db/)
	skill_tree.txt 修正
	skill_tree2.txt 廃止
	(map/)
	map.h PC_CLASS_BASE等追加
	pc.c
		pc_calc_skilltree() 修正
		pc_allskillup() 修正
		pc_readdb() 修正

--------------------
//0963 by (凸)

・@jobchange2, @jobchange3廃止 @jobchangeに引数追加 @help参照
	例: @jobchange2 10 → @jobchange 10 1
・同様に@charjob2, @charjob3廃止 @charjobに引数追加 @help参照
	例: @charjob2 10 ほげほげ → @charjob 10 1 ほげほげ
・同様にスクリプトのjobchange2, jobchange3命令廃止 jobchangeに引数追加 script_ref.txt参照
	例: jobchange2 10; → jobchange 10,1;
・↑どれも追加された引数は省略可能です。なので、転生ノービスは現状のスクリプトで転生一次職に転職できます。
	例:	Novice High → @jobchange 10 → Whitesmith
		Novice → @jobchange 10 → Blacksmith
・スクリプトから転生しているか判定するためにUpperを追加しました。Upper 0=通常, 1=転生, 2=養子
	Upper=0の時にBaseLevel=99なら転生させる～とかそういうスクリプト誰か書いてください
	その時に元の職業は記憶していないので永続変数とかで覚えさせて判定させないと転生後何にでも転職できちゃう？
・バイオプラントとスフィアマインで呼び出されるMobの名前を--ja--にしてmob_db.txtから読むようにした

	(conf/)
	help.txt 修正
	(db/)
	const.txt
	(doc/)
	help.txt 修正
	script_ref.txt 修正
	(map/)
	atcommand.c
		atcommand_jobchange() 修正
		atcommand_jobchange2() 削除
		atcommand_jobchange3() 削除
		atcommand_character_job() 修正
		atcommand_character_job2() 削除
		atcommand_character_job3() 削除
	map.h 修正
	pc.c
		pc_readparam() 修正
		pc_jobchange() 修正
	pc.h 修正
	script.c
		buildin_jobchange() 修正
		buildin_jobchange2() 削除
		buildin_jobchange3() 削除
	skill.c
		skill_castend_pos2() 修正

--------------------
//0962 by (凸)

・職業は0～23で処理したいので転生職用のスキルツリー追加、eAthenaを参考に拡張
	っていうか韓国本サーバでの実装の資料が見当たらないので適当
・sakexe.exeを解析してskill_db.txt変更、これもeAthenaを参考に拡張
	どれが本サーバで実装されているスキルか分かりませんっ！！
※スキルツリーが表示されたからといって使えるわけじゃありませんっ！！

	(common/)
	mmo.h 定数修正
	(db/)
	skill_db.txt 変更
	skill_require_db.txt 変更
	skill_tree2.txt 追加
	(map/)
	skill.h 定数修正
	pc.c
		pc_calcstatus() 修正
		pc_allskillup() 修正
		pc_calc_skilltree() 修正
		pc_readdb() 修正
	
--------------------
//0961 by 胡蝶蘭

・スクリプトにサブルーチン/ユーザー定義関数機能追加
	詳しくはサンプルとscript_ref.txtを読んでください。
	地味に大改造なので、スクリプト関係でバグがあるかもしれません。
	
	(map/)
	map.h/map.c
		struct map_session_data にスクリプト情報退避用のメンバ追加
		map_quit()修正
	script.h/script.c
		色々修正(run_script(),run_func()が主)
	npc.c
		npc_parse_function()追加他
	(conf/sample)
	npc_test_func.txt
		ユーザー定義関数/サブルーティンのテストスクリプト
	(doc/)
	script_ref.txt
		サブルーティンなどの説明追加

--------------------
//0960 by (凸)
・本鯖相違スレッド part2 >>62 KKさんのアンクルスネア修正を同梱
・バグ報告スレッド part5 >>14-16 rbさんのバグ修正を同梱
・For English User Forum >>15 Mugendaiさんの指摘で0x1d7を使うのはVal>255に修正(0xc3のValは1バイトだから0x1d7を使うのかと納得)
・pc_calc_base_job()を変更して元jobだけでなくノビか一次職か二次職(type)、通常か転生か養子(upper)を返すようにした

	(map/)
	atcommand.c
		atcommand_joblevelup() 修正
		atcommand_character_joblevel() 修正
	clif.c
		clif_changelook() 修正
	pc.h 修正
	pc.c
		pc_setrestartvalue() 修正
		pc_equippoint() 修正
		pc_isequip() 修正
		pc_calc_skilltree() 修正
		pc_calcstatus() 修正
		pc_isUseitem() 修正
		pc_calc_base_job() 修正
		pc_allskillup() 修正
		pc_damage() 修正
		pc_jobchange() 修正
		pc_equipitem() 修正
	script.c
		buildin_changesex() 修正
	skill.c
		skill_castend_nodamage_id() 修正
		skill_unit_onplace() 修正
		
--------------------
//0959 by (凸)
・help.txtがdocじゃなくてconfのが読み出されてた＿|￣|○
・gamejokeを参考に転生二次職のステータス加重値をjob_db2-2.txtに記述
・スクリプトにjobchange2とjobchange3を追加それぞれ転生職と養子職へ転職させる命令です

	(conf/)
	help.txt 修正
	(db/)
	job_db2.txt 修正
	job_db2-2.txt 追加
	(doc/)
	help.txt 修正
	script_ref.txt 修正
	(map/)
	pc.c
		pc_calcstatus() 修正
		pc_readdb() 修正
	script.c
		buildin_jobchange() 修正
		buildin_jobchange2() 追加
		buildin_jobchange3() 追加

--------------------
//0958 by (凸)
・転生職方面の実装を色々
・重量制限は良く分からないので元の職業の値をそのまま使っています(モンク＝チャンピオン等)
・装備品も同上、HPやSPのテーブルも同上なので、転生してもHPなどが増えないガッカリ仕様です

	(map/)
	atcommand.c
		atcommand_joblevelup() 修正
		atcommand_character_joblevel() 修正
	pc.c
		pc_setrestartvalue() 修正
		pc_equippoint() 修正
		pc_isequip() 修正
		pc_calcstatus() 修正
		pc_isUseitem() 修正
		pc_calc_base_job() 追加
		pc_damage() 修正
		pc_jobchange() 修正
		pc_equipitem() 修正
	pc.h 修正
	script.c
		buildin_changesex() 修正
	skill.c
		skill_castend_nodamage_id() 修正

--------------------
//0957 by (凸)
・@charjob2と@charjob3を追加、関係としては@charjob⇔@jobchange、@charjob2⇔@jobchange2、(ry
・@mapexitを追加、map-serverを落とすコマンドですatcommand_athena.confでは99設定にされてますので使用には十分注意してください。

	(map/)
	atcommand.c
		atcommand_character_job2() 追加
		atcommand_character_job3() 追加
	atcommand.h 修正
	(conf/)
	atcommand_athena.conf 修正
	(doc/)
	help.txt 修正

--------------------
//0956 by (凸)
・転生職仮実装(@jobchange2)、見た目と経験値テーブルだけです
・養子職仮実装(@jobchange3)、見た目だけです現状では経験値は転生二次職と同じというマゾ仕様
※上記2点は転生職が実装されているクライアントでなければ実行するとエラー落ちするので注意！！
　その後直接セーブデータを弄らないとキャラセレにも行けなくなります！！
・Athena雑談スレッド part2 >>149 稀枝さんの報告を元にガーディアンを修正
・砦以外でガーディアンとかエンペリウムを殴るとmap-serverが落ちていたのも修正(未確認)
・スパノビのJobテーブルはFor English User Forum >>13 kingboさんのデータを元に修正
・転生職の経験値テーブルはOWNを参照してBase99の経験値は不明だったので適当に設定

	(map/)
	atcommand.c
		atcommand_jobchange() 修正
		atcommand_jobchange2() 追加
		atcommand_jobchange3() 追加
	atcommand.h 修正
	clif.c
		clif_changelook() 修正
	pc.c
		pc_nextbaseexp() 修正
		pc_nextjobexp() 修正
		pc_jobchange() 修正
		pc_readdb() 修正
	battle.c
		battle_calc_damage() 修正
		mob_can_reach() 修正

--------------------
//0955 by huge
・ペットのルート機能。
  ・仕様はmobのルートに近い感じですが、射程を短くしてます。
  ・拾ったアイテムは、パフォーマンスをすると床に落とします。
  ・卵に戻したり、ログオフしたときはPCの手元に入るようにしました。(重量超過はドロップ)
  ・拾える個数はルートmob同様の10個ですが、11個目は拾いに行きません。
  ・ルート権の問題から、アイテムにfirst_idが入っていて、それが飼い主以外だったら、何秒経とうと拾いません。(未確認)
  ・それと、荷物持ちにされると可哀想なので、重量制限もつけました。これはconfで設定可能です。
・atcommandで、@whereがうまく働いてなかったので修正(またウチだけかなぁ･･･）
・@memoでmemoする時は、mapflagを無視するように。
・スフィアマインの名前だけ修正。

	(conf/)
	battle_athena.conf
		pet_lootitem,pet_weight 追加
	(doc/)
	conf_ref.txt 修正
	(map/)
	atcommand.c
		atcommand_memo() 修正
		atcommand_where() 修正
	battle.c
		battle_config_read() 修正
	battle.h 修正
	map.c
		map_quit() 修正
	map.h
		pet_data{} 修正
	pc.c
		pc_memo() 修正
	pet.c
		pet_performance() 修正
		pet_return_egg() 修正
		pet_data_init() 修正
		pet_ai_sub_hard() 修正
		pet_lootitem_drop() 追加
		pet_delay_item_drop2() 追加
		pet_ai_sub_hard_lootsearch() 追加
	pet.h 修正
	skill.c
		skill_castend_pos2() 修正

--------------------
//0954 by (凸)
・object_del.batで各server.exeも削除するようにした
・For English User Forum >>11 kingboさんの修正を取り込み
・バグ報告スレッド part5 >>10 Sinさんの修正を取り込み
・ついでに見かけたatcommand_character_joblevelの不具合を修正
・@コマンドでジョブレベルを上げるときにスパノビはJob70まで対応(未確認)

	(/)
	object_del.bat 修正
	(map/)
	atcommand.c
		atcommand_joblevelup() 修正
		atcommand_character_joblevel() 修正
		atcommand_character_baselevel() 修正

	code by kingbo 2004/4/29 PM 06:15
	base on 0953
	now i sure it works well
	(map/)
	mob.c
		mob_can_reach() fix

--------------------
//0953 by (凸)
・mob_skill_db.txtの条件値に0以外入っていなかったのを訂正
・gcc 2.95でコンパイルできるように訂正(by バグスレpart5 >>2 茜さん)
・↑やLinuxなどを考慮してstartやMakefileなどの改行をLFに変更
・0952で出たコンパイル警告を出ないように修正
・0952で更新されなかったconf_ref.txtを修正

	(/)
	start 改行コード変更
	athena-start 改行コード変更
	(db/)
	mob_skill_db.txt 修正
	(doc/)
	conf_ref.txt 修正
	(login/)
	Makefile 改行コード変更
	(map/)
	Makefile 改行コード変更
	atcommand.c
		atcommand() 宣言位置修正
		atcommand_where() 宣言位置修正
	battle.c 
		battle_calc_pet_weapon_attack() 修正
		battle_calc_mob_weapon_attack() 修正
		battle_calc_pc_weapon_attack() 修正
		battle_calc_magic_attack() 修正
	clif.c
		clif_skill_fail() 宣言位置修正
	guild.c
		guild_gvg_eliminate_timer() 宣言位置修正
	mob.c
		mob_damage() 宣言位置修正
	script.c
		buildin_deletearray() 宣言位置修正
		buildin_getequipcardcnt() 宣言位置修正
		buildin_successremovecards() 宣言位置修正

--------------------
//0952 by CG
・confでDEFとMDEFの計算方法を選択できるように。

	(conf/)
	battle_athena.conf 変更
	(map/)
	battle.c 
		battle_calc_pet_weapon_attack() 修正
		battle_calc_mob_weapon_attack() 修正
		battle_calc_pc_weapon_attack() 修正
		battle_calc_magic_attack() 修正
	battle.h 修正

--------------------
//0951 by (凸)
・サーバーsnapshot
・バグ報告スレッド part4 >>95 KAJIKENさんの修正を同梱
・同 >>138 バグかな？さんの修正を同梱
・Athena雑談スレッド part2 >>112 名無しさんのPVPナイトメアモードのアンダークロスマップワープポイントを同梱
・同 >>96 稀枝さんのnpc_gldcheck.txtを同梱
・スナップショットにsave/を入れるのをやめました。無い場合はathena-startが作ってくれます
・athena-startでlog/が無い場合に作るように変更
・その他？

	(/)
	athena-start 変更
	(db/)
	mob_db.txt 変更
	(conf/)
	map_athena.conf 変更
	(conf/extension/)
	npc_gldcheck.txt 追加
	(conf/npc/)
	npc_event_ice.txt 変更
	npc_job_alchemist.txt 変更
	npc_event_valentine.txt 変更
	npc_town_geffen.txt 変更
	npc_event_whiteday.txt 変更
	npc_event_potion.txt 変更
	npc_town_comodo.txt 変更
	(conf/warp/)
	npc_warp_pvp.txt 追加

--------------------
//0950 by (凸)
・mob_dbのModeフラグに以下の物を追加
	0x40(64) ダメージを1に固定(草やクリスタルなど)
	0x80(128) 攻撃を受けたときに反撃をする
・上記の変更のためmob_db.txtほぼ全部変更、mob_db2.txtを作っている人は
	草など1ダメ固定にはModeに64を足さないと普通にダメージ
	その他MobはModeに128を足さないと反撃してこなくなるので注意

	(db/)
	mob_db.txt 修正
	(map/)
	battle.c
		battle_calc_pet_weapon_attack()
		battle_calc_mob_weapon_attack()
		battle_calc_pc_weapon_attack()
		battle_calc_magic_attack()
	mob.c
		mob_once_spawn()
		mob_attack()
		mob_target()
		mob_ai_sub_hard()

--------------------
//0949 by ぴざまん

・ステータス異常耐性全面修正。
　耐性算出式全面修正。
　ステータス異常耐性100%のキャラクターには状態異常を行わないように修正。
・ディスペルの仕様変更。
　解除したらシステム上問題のあるステータス変化以外片っ端から解除するように修正。
・フロストダイバーの仕様変更。
　凍結率修正(マジスレテンプレ準拠)。
・リカバリーの仕様変更。
　ノンアクティブモンスターに使用するとターゲットがリセットされるように修正。
・クァグマイアの仕様がアレだったので修正。
　演奏や属性場と同様にrangeで処理するように修正。
　DEX/AGI半減の影響が詠唱以外にも及ぶ様に修正。
・スキルターゲット中に死んだ振りを使用してもスキルが回避できない問題修正。
・白刃取りが動作しない問題修正(cast_dbが抜けてました)。
	(map/)
	pc.c
		pc_calcstatus()修正
	skill.c
		skill_additional_effect()、skill_attack() 修正
		skill_status_change_start()、skill_unitsetting() 修正
	(db/)
	cast_db.txt 修正。

--------------------
//0948 by 胡蝶蘭

・warpwaitingpcが正しくPCを転送できない問題を修正
・スクリプトの読み込み時にエラーまたは警告が出る場合、警告音を鳴らすように。
  （流れたログを見ない人対策です）

	(map/)
	script.c
		buildin_warpwaitingpc()修正
		disp_error_message()修正

・atcommand.c修正
	・atcommand_athena.confのmapmoveを読むように
	・@strなどの省略時の必須レベルを0に。
	・@paramは使わないのでコメント化
	
	(map/)
	atcommand.c
		該当個所修正

・mobが最大15秒ほど移動しない場合がある問題修正
	・手抜きでないmob処理で、移動しない時間が7秒以上続かないように修正

	(map/)
	mob.c
		mob_ai_sub_hard()修正

・快速船員の伊豆港行きの判別式修正 (by ID:F8nKKuY)
	(conf/npc/)
	npc_town_comodo.txt

--------------------
//0947 by (凸)
・取り巻きは取り巻きを召喚しないように修正
・露天の販売価格の上限をbattle_athena.confで設定できるように修正

	(conf/)
	battle_athena.conf
		vending_max_value追加
	(doc/)
	conf_ref.txt 修正
	(map/)
	skill.c
		skill_castend_nodamage_id() 修正
	battle.c
		battle_config_read() 修正
	battle.h 修正。
	vending.c
		vending_openvending() 修正。

--------------------
//0946 by Kalen
・プロ北Warp見直し
参考：本鯖(1F,2F)らぐなの何か(3F)
	(conf/warp/)
	npc_warp.txt

・語り部の2週3週追加(どうせ見ないと思いますが…)
	(conf/npc/)
	npc_event_kataribe.txt

--------------------
//0945 by 胡蝶蘭

・NPCタイマーラベルデータが正しくインポートされない問題を修正
・NPCタイマー初期値やタイマーIDが正しく初期化されない問題を修正
・NPCのduplicateを行うとアクセス違反が起こる場合がある問題を修正

	(map/)
	npc.c
		npc_parse_script修正

・パッチアップスレ４の87のpc.cとりこみ
	(map/)
	pc.c
		カード重量制限を元に戻したもの

--------------------
//0944 by huge
・ギルドの上納経験値の上限を、confで制限できるように。
・露店の販売価格を10Mまでに制限。
・カートの重量制限が一桁下がってたんですが、ウチだけですか？修正してみましたが。

	(conf/)
	battle_athena.conf
		guild_exp_limit追加
	(doc/)
	conf_ref.txt 修正
	(map/)
	atcommand.c
		蘇生時のSP回復で、細かい修正。
	battle.c
		battle_config_read() 修正
	battle.h 修正。
	guild.c
		guild_change_position() 修正。
	pc.c
		pc_calcstatus() 修正。
	vending.c
		vending_openvending() 修正。

--------------------
//0943 by (凸)
・battle_athena.confでdead_branch_activeをyesにすると古木の枝で召喚されるモンスターがアクティブになるように変更
・微妙に変更したclient_packet.txtを同梱

	(conf/)
	battle_athena.conf
		dead_branch_active追加
	(doc/)
	client_packet.txt 修正
	conf_ref.txt 修正
	(map/)
	battle.c
		battle_config_read() 修正
	battle.h 修正
	map.h 修正
	mob.c
		mob_once_spawn() 修正
		mob_attack() 修正
		mob_target() 修正
		mob_ai_sub_hard_lootsearch() 修正
		mob_ai_sub_hard() 修正
		
・英語スレのkingboさんの変更を同梱
	code by kingbo 2004/4/16 PM 09:47

	support guildcastle guardian
	maybe still have problems..need to try
	Good Luck Q^^Q
	P.S: sorry my poor english ^^a

	(map/)
	mob.c
		mob_can_reach() fix
	battle.c
		battle_calc_damage() fix

	(conf/gvg/)
		prtg_cas01_guardian.txt

--------------------
//0942 by 胡蝶蘭

・アクセスコントロールで不正なメモリにアクセスする場合があるバグを修正
	(login/)
	login.c
		check_ipmask()修正

・スクリプトリファレンス少し追加と修正
	(doc/)
	script_ref.txt
		修正

--------------------
//0941 by (凸)

・e2さんの報告を元に召喚された手下のスピードを召喚主と同じにしてみる

	(map/)
	battle.c
		battle_get_speed() 修正
	mob.c
		mob_spawn() 修正
		mob_summonslave() 修正

--------------------
//0940 by End_of_exam

・ヒールやポーションピッチャーを使用しても回復しないバグを修正(0938～)。

　Thanks for Pepermint, reporting the bug that using PotionPitcher with
　BluePotion was no effective.
　（＝ポーションピッチャー＋青Ｐで効果がない事を報告してくれたPepermint氏に感謝）

	(map/)
	battle.c
		battle_heal() 修正

--------------------
//0939 by (凸)
・cutinパケットを0x145(ファイル名16文字)から0x1b3(64文字)に変更
・ついでに雑談スレに上げたathena-startを同梱

	(/)
	athena-start saveファイルが無いときに作るように
	(map/)
	clif.c
		clif_cutin() 本鯖パケット準拠に変更
	(doc/)
	client_packet.txt 修正

--------------------
//0938 by ぴざまん

・ポーションピッチャーで青ポを投げてもエフェクトだけだったバグ修正。
・露店開設が特定のアイテム配置で失敗するバグ修正。
・スクリプト関数getareadropitem実装。
　指定エリア内のドロップアイテムをカウントする関数です

　書式：getareadropitem <mapname>,<x0>,<y0>,<x1>,<y1>,<item>;
　　mapname：対象マップ名(例：prontera.gat)
　　x0とx1：対象X座標範囲
　　y0とy1：対象Y座標範囲
　　item：カウントする対象アイテム

　戻り値：mapname内座標(x0,y0)-(x1,y1)の範囲内に落ちているitemの総個数
　　　　　取得失敗時には-1を返します。
　・itemの値はIDでもアイテム名("Red_Jemstone"とか)でもいいです。

	(map/)
	battle.c
		battle_heal() 修正。
	vending.c
		vending_openvending() 修正。
	script.c
		ローカルプロトタイプ宣言修正。
		struct buildin_func[] 修正。
		buildin_getareadropitem()、buildin_getareadropitem_sub() 追加。

--------------------
//0937 by netwarrior

- Fix Japanese remarks problem in 0936
- Fix minor problem in battle_heal()

--------------------
//0936 by Pepermint

Retouch about problem of increase in quantity at the CART,
when enter the an minus quantity in the CLIENT.

Retouch about problem of not recovery,use POTIONPITCHER skill.

	(map/)
	battle.c
		battle_heal()

	vending.c
		vending_purchasereq()

--------------------
//0935 by 胡蝶蘭

・内容の同じスクリプトNPCを何度も記述しなくても言いように修正
	・NPC定義の"script"と書く部分を"duplicate(NPC名)"とすると、
	  該当のNPCとスクリプトを共有するように。NPC名は表示名ではなく
	  エクスポートされる名前を指定します。
	  <例>
prontera.gat,165,195,1	duplicate(カプラ職員)	カプラ職員2	112

	・共有元のNPCは同じマップサーバーに存在する必要があるため、
	  同じマップでない場合はduplicateすべきではない。
	  ただし、NPCの位置を"-"にすることで、マップ上には存在しないが、
	  マップサーバー内には存在するNPCを作成できるので、
	  そのNPCを共有元にするのであればどのマップへも共有できる。
	  <例>

-	script	テスト::test1	112,{	// このNPCグラフィックIDは使用しない
//	(略)
}
prontera.gat,165,195,1	duplicate(test1)	テスト2	112
geffen.gat,99,99,1	duplicate(test1)	テスト3	112

	・上のマップに存在しないNPCはイベントにもすることができる。
	  （どのマップサーバーからでも必ず呼び出せるイベントになる）

	(map/)
	map.h
		struct npc_label_list追加,struct npc_data修正
	npc.c
		npc_parse_script()修正
		不要になったラベルデータベース関連の関数を削除

--------------------
//0934 by ぴざまん

・MOBの状態異常耐性がやたら高かったのを修正。
・速度減少の仕様変更(成功率計算式変更・失敗時にはエフェクト無し)。
・何時の間にか状態異常の継ぎ足し禁止がコメントアウトされていたので戻し。
・ポイズンリアクトのアイコン表示が無くなっていたので修正(でも出るだけ…)
・白刃取り実装。

	(map/)
	battle.c
		battle_weapon_attack() 修正。
	clif.c
		clif_parse_WalkToXY()、clif_parse_ActionRequest() 修正。
		clif_parse_TakeItem()、clif_parse_UseItem() 修正。
		clif_parse_DropItem()、clif_parse_EquipItem() 修正。
		clif_parse_UnequipItem() 修正。
	mob.c
		mob_can_move()、mob_attack()、mob_ai_sub_hard() 修正。
		mobskill_use_id()、mobskill_use_pos() 修正。
		mobskill_castend_id()、mobskill_castend_pos() 修正。
	pc.c
		pc_attack_timer()、pc_setpos() 修正。
	skill.c
		SkillStatusChangeTable[] 修正。
		skill_additional_effect()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_check_condition() 修正。
		skill_status_change_start()、skill_status_change_end() 修正。
		skill_use_id() 修正。
	skill.h 修正。
	(db/)
	cast_db.txt 修正。

--------------------
//0933 by 胡蝶蘭

・ウィザード転職所NPC仮実装
	・eathenaのデータの翻訳、および某所の昔のデータを元に作ったので
	  癌鯖とは微妙に異なってると思います。
	  「古い巻物」とか使えませんし。
	・現行のjob_2nd.txtと一緒に読み込むと、NPCが重なるので、
	  job_2nd.txtの該当スクリプトをコメント化してください。
	  ただし、その場合はセージに転職させてもらえないので注意してください。
	
	(conf/npc/)
	npc_job_wizard.txt
		追加
	(conf/warp/)
	npc_warp_job.txt
		ウィザード転職所のワープをコメント化

・バグ修正
	・gotoやmenuでラベルが見つからないときスクリプトの実行を中断するように.
	・１回のスクリプトの実行において、実行命令数が約8000を超えるか、
	  gotoやmenuの実行回数が約500回を超えると実行を中断するように。
	・関数/命令実行時にもエラーチェックを入れた
	・ギルド/パーティ/ペットの名前に日本語が使えない問題修正


	(char/)
	int_guild.c/int_party.c
		名前問題修正
	(map/)
	pet.c
		名前問題修正
	script.c
		修正

--------------------
//0932 by End_of_exam

・0930でギルド倉庫の中身が消えるバグを修正。

	(char/)
	int_storage.c guild_storage_tostr() 修正。

--------------------
//0931 by (凸)
・サーバーsnapshot
・AthenaDB計画から更新のあった物を反映
・atcommand_athena.confにstr等を追加
・ワープポータルの本鯖相違点を修正
・霧さんのaldeg_cas03～05を同梱
・pさんのnpc_event_kataribe.txt同梱
・KAJIKENさんのnpc_warp_louyang.txt同梱

	(db/)
	item_db.txt 変更
	(conf/)
	atcommand_athena.conf 変更
	map_athena.conf 変更
	(conf/gvg/)
	aldeg_cas03.txt 変更
	aldeg_cas04.txt 変更
	aldeg_cas05.txt 変更
	(conf/mob/)
	npc_monster.txt 変更
	(conf/npc/)
	npc_event_kataribe.txt 追加
	(conf/warp/)
	npc_warp_louyang.txt 追加
	(map/)
	pc.c pc_memo() 変更

--------------------
//0930 by 胡蝶蘭

	既存の char/ にあるlock.cとlock.hは削除してください。
	これらは common/ に移動されます。

・スクリプト追加修正
	・setarray（配列へリスト代入）追加
	・cleararray（配列を指定値でクリア）追加
	・copyarray（配列をコピー）追加
	・getarraysize（配列の有効な要素数を求める）追加
	・deletearray（配列の要素を削除してつめる）追加
	・warpwaitingpc：人数やアカウントIDをマップ変数にセットするように修正

	(map/)
	script.c
		色々
	(doc/)
	script_ref.txt
		命令追加など
	(npc/sample/)
	npc_test_array.txt
		配列系テスト用NPCスクリプト

・バグ修正など
	・キャラクター名/アカウント名/パーティ名/ギルド名/ペット名に
	  コントロールコードを使えないように修正。
	・char.cなどの保存の効率を更にアップ。
	・倉庫ファイル/ギルド倉庫ファイルに空行が残らないように修正
	・lock.*をcommon/に移動、ファイルが保存できなくなるバグ修正、
	  アカウントとマップ変数の保存にもlock_fopenを使うように。

	(common/)
	lock.c/lock.h
		フォルダ移動、lock_fopen修正
	(login/)
	login.c
		mmo_auth_new()修正
	(char/)
	char.c
		mmo_char_tostr(),make_new_char()修正
	int_storage.c
		storage_tostr(),guild_storage_tostr(),
		inter_storage_save_sub(),inter_guild_storage_save_sub()修正
	int_party.c
	int_guild.c
		名前問題修正
	(map/)
	pet.c
		名前問題修正

--------------------
//0929 by ぴざまん

・ランドプロテクター完全実装。
・プロボックのスキル仕様変更(詠唱妨害・凍結、石化、睡眠解除)。
・リカバリーのスキル仕様変更(アンデッドに使用すると一定確率で暗闇効果)。
・状態異常中でも装備の切り替えが出来るように変更。
・アイテム自動取得中に@killmonsterを使用すると落ちるバグ修正。
・胡蝶蘭さんが上げていたnpc.cを同梱しました。
・skill_dbで気になってたとこをちょこっと修正。

	(map/)
	clif.c
		clif_parse_EquipItem() 修正。
	skill.c
		skill_castend_nodamage_id() 修正。
		skill_unitsetting()、skill_unit_onplace() 修正。
		skill_landprotector() 追加。
	mob.c
		mob_delay_item_drop()、mob_delay_item_drop2() 修正。
	npc.c 修正

	(db/)
	skill_db.txt 修正。

--------------------
//0928 by End_of_exam

・キャラや倉庫内アイテムが消える問題に暫定対処（キャラ鯖の改良）。

　1.キャラデータ・倉庫データのデータ変換処理を改良。
　　(char/char.c , char/int_storage.c)

　2.ファイル書き出しが終わるまで、旧ファイルを残すように修正。
　（char/lock.c, char/lock.h の追加。メイクファイルの修正。
　　char/int_storage.c,int_guild.c,int_party.c,int_pet.c,char.c,inter.c
　　内にある、データ書き出し処理を変更。）

--------------------
//0927 by ぴざまん

・武器攻撃以外に種族補正が入っていなかったのを修正。
・演奏中に楽器・鞭以外の武器に持ち変えると演奏が止まる様に修正。
・演奏の効果が演奏者自身にはかからないように修正。
・アイテム自動取得機能実装
　敵を倒した時に、アイテムがドロップされるのではなく、その敵に一番多くのダメージを
　与えた人(ルート権1位の人)にドロップアイテムが自動で与えられる機能です。
　battle_athena.confのitem_auto_getで設定できます。
　オリジナルアップデートの為、battle_athena.confでのデフォルトではnoにしています。
・属性場(デリュージ・バイオレントゲイル・ボルケーノ・ランドプロテクター)仮実装
　一部の機能が未実装です
　　デリュージ：水場を用いたウォーターボール
　　バイオレントゲイル：ファイアーウォールの持続時間補正
　　ボルケーノ：アイスウォール禁止
　　ランドプロテクター：オブジェクト設置系スキル禁止(つまり出るだけのLPです)

	(map/)
	battle.c
		battle_calc_damage()、battle_calc_magic_attack() 修正。
		battle_calc_misc_attack()、battle_config_read() 修正。
	mob.c
		mob_delay_item_drop()、mob_delay_item_drop2() 修正。
	pc.c
		pc_calcstatus()、pc_equipitem() 修正。
	skill.c
		ローカルプロトタイプ宣言修正。
		SkillStatusChangeTable[] 修正。
		skill_castend_nodamage_id()、skill_castend_pos2() 修正。
		skill_unitsetting()、skill_unit_onplace() 修正。
		skill_unit_onout()、skill_unit_ondelete() 修正。
		skill_status_change_start()、skill_status_change_end() 修正。
		skill_clear_element_field() 追加。
	battle.h 修正。
	skill.h 修正。
	(conf/)
	battle_athena.conf
		item_auto_get項目を追加。
	(db/)
	cast_db.txt 修正。

--------------------
//0926 by 胡蝶蘭

・ギルド城の初期化方法変更
	・全てのギルド城（＋占拠ギルド情報）を所得したときにOnAgitInitが
	  よばれるように。GetCastleDataやRequestGuildInfoの必要がなくなります。
	  従って、現在のgvgスクリプトのOnInterIfInitやOnRecvCastleXXXは
	  必要なくなります。（おそらく現行のスクリプトも動作は可能です）
	  初期化が必要なNPCは変わりにOnAgitInitを使ってください。
	  なお、不要になった命令などを削除する予定はありません。
	
	(char/)
	inter.c/inter.h/char.c/int_guild.c/int_guild.h
		マップサーバー接続時に処理を行えるように修正
		接続時にギルド城データを送信するように
	(map/)
	intif.c/guild.c/guild.h
		接続時のギルド城データ一括受信処理＆ギルド情報要求、
		OnAgitInitの呼び出しなど
	chrif.c
		旧OnAgitInitの呼び出し削除
	(doc/)
	inter_server_packet.txt
		ギルド城データ一括送信パケット0x3842追加

・スクリプトにNPC主体イベント実行命令追加
	・donpcevent(NPC主体イベント実行)追加
	  こちらはdoeventと違いブロードキャスト可能です。詳細はscript_ref。
	・isloggedinのコンパイル警告がでないように修正
	(map/)
	script.c
		buildin_donpcevent()追加など
	(doc/)
	script_ref.txt
		doevent,donpcevent,OnAgitInit追加など

・その他修正
	(map/)
	clif.c
		clif_parse_GMReqNoChat()追加


--------------------
//0925 by 胡蝶蘭

・スクリプトのバグ修正
	・monster,areamonsterの問題修正
	  これらはscript_rid2sdを使用しないようにしました。
	  sdがNULLのとき、レベルチェックを行わないようにしました。
	
	(map/)
	script.c/mob.c
		buildin_monster(),buildin_areamonster(),mob_once_spawn()他

・スクリプトの機能追加
	・detachrid命令（プレイヤーのデタッチ）追加
	・isloggedin関数（プレイヤーのログインチェック）追加
	・getitem,getitem2命令,countitem,checkweight関数で
	  nameidにアイテムの名前を指定可能に。
	  (item_dbなどに依存するため、使用すべきではありませんが、一応)

	(map/)
	script.c
		色々
	(doc/)
	script_ref.txt
		変更部分とエラー説明修正

・athena-startとstartを修正
	・athena-start stopでプロセスが終了するまで待つように
	・startで再起動させるときathena-start restartを呼ぶように。

--------------------
//0924 by (凸)

・バグ報告スレッド part4 >>66のnpcを取り込み
・同 >>51のguild.cを反映
・同 >>38のatcommand_athena.confへの変更を取り込み
・なぜかnpcフォルダにあってmap_athena.confないファイルを一覧に追加。ついでにギルドフラッグのコメントアウトを解除
・conf/npc/npc_job_1st.txt npc_script2.txt npc_shop3.txtはスナップショットから削除してください

	(map/)
	guild.c 修正
	(conf/)
	map_athena.conf 修正
	atcommand_athena.conf 修正
	(conf/npc/)
	npc_event_hat.txt 修正
	npc_event_oni.txt 修正
	npc_job_1st.txt 削除
	npc_script2.txt 削除
	npc_shop3.txt 削除

--------------------
//0923 by 胡蝶蘭

・スクリプトのバグ修正
・script_rid2sdが失敗してもサーバーを終了しないように変更
	・monster,areamonsterなどが実行できない問題修正

	(map/)
	script.c/mob.c
		buildin_monster()など修正

	(doc/)
	script_ref.txt
		ラベルとエラーの説明修正
	
--------------------
//0922 by 胡蝶蘭

・スクリプトエラーの行番号が正しく表示されるように
	
	(map/)
	itemdb.c / npc.c
		itemdb_readdb(),npc_parse_script()修正

・キャラクター情報にアクセスできない状態でアクセスするとエラーを出すように修正
・goto/menuでラベルが指定される場所にラベル以外が指定されると警告を出すように
・script_refにエラーメッセージの説明追加
・イベント起動されたスクリプトでキャラクターを使用できるようにする関数追加
	・attachrid（指定したIDの情報を使用できるようにする）追加
	・getcharid（3でアカウントIDを所得できるように）修正
	(map/)
	script.c
		script_rid2sd(),buildin_attachrid()追加
		多々修正。
	(doc/)
	script_ref.txt
		エラーメッセージの説明追加、変数/ラベルの説明修正
		他修正

--------------------
//0921 by RR
・スクリプトバグ修正(ご迷惑をおかけしました)
	(conf/npc/)
	npc_event_tougijou.txt

・steal率修正(自DEX - 敵DEX + SLv*3 +10の部分で一度判定をしていたので)
・0918で0914以前に巻き戻ってしまっていた部分を元に戻した
	(map/)
	pc.c
		pc_steal_item() 修正
	itemdb.c
	pet.c
	skill.c 修正

--------------------
//0920 by 獅子o^.^o
・ Steal率 = Drop率 * (自DEX - 敵DEX + SLv*3 +10) /100
	(map/)
	pc.c
		int pc_steal_item()修正

--------------------
//0919 by RR
・atcommandのlvupを使うと取得ステータスポイントがおかしい問題の修正
・バグ修正(バグ報告スレで修正の出たものの取り込み　胡蝶蘭さん、pさん、共にお疲れ様です)
	(map/)
	atcommand.c
		atcommand_baselevelup()修正
	guild.c
		guild_gvg_eliminate_timer()修正
	pc.c
		pc_setreg(),pc_setregstr()修正
	(login/)
	login.c 
		parse_login()修正
--------------------
//0918 by 聖
・item_db自体がオーバーライド可能になったので、class_equip_db.txtの廃止。
・pet_db.txt、produce_db.txtもオーバーライド可能に修正。
	(map/)
	itemdb.c
		do_init_itemdb() 修正。
	pet.c
		read_petdb() 修正。
	skill.c
		skill_readdb() 修正。

--------------------
//0917 by RR
・スクリプト修正
	桃太郎イベントと闘技場イベントをNPCタイマーに変更
	(conf/npc/)
	npc_event_momotarou.txt
	npc_event_tougijou.txt 修正
・スキルツリー修正(バグ報告スレ25より)
	(db/)
	skill_db.txt 修正

--------------------
//0916 by (凸)
・npc_monsterにnpc_mob_jobを統合。npc_mob_job.txtは削除してかまいません

	(conf/)
	map_athena.conf npc_mob_jobを削除
	(conf/mob/)
	npc_monster.txt 更新

--------------------
//0915 by 胡蝶蘭

・NPCタイマー関係の命令追加＆修正他
	・delwaitingroom（NPCチャット終了）引数を見てなかったので修正
	・initnpctimer（NPCタイマー初期化）追加
	・stopnpctimer（NPCタイマー停止）追加
	・startnpctimer（NPCタイマー開始）追加
	・getnpctimer（NPCタイマー情報所得）追加
	・setnpctimer（NPCタイマー値設定）追加

	既存のaddtimerなどはプレイヤー単位のため、NPC単位のタイマーを作りました。
	こちらは、addtimerなどとは違い、OnTimerXXXという風にラベルを指定します。
	詳しくはサンプルとscrit_ref.txtを参照。
	
	(map/)
	map.h
		struct npc_data 修正、struct npc_timerevent_list追加
	npc.c / npc.h
		npc_timerevent(),npc_timerevent_start(),npc_timerevent_stop(),
		npc_gettimerevent_tick(),npc_settimerevent_tick()追加
		npc_parse_script()修正
	script.c
		buildin_*npctimer()追加など
	(conf/sample/)
	npc_test_npctimer.txt
		NPCタイマー使用サンプル
	(doc/)
	script_ref.txt
		NPCタイマー関係の命令/関数追加、定数ラベルの説明修正

・Sageのアーススパイクの所得条件修正
	(db/)
	skill_tree.txt
		アーススパイクの行（サイズミックウェポンをLv1に）

--------------------
//0914 by p
・範囲スキル使用時に解放済みメモリを参照していた問題に対応
・メモリを初期化せずに使用していた領域を、初期化してから使用するように変更
	(common/)
	db.c
	grfio.c
	socket.c
	timer.c
	(char/)
	char.c
	int_guild.c
	int_party.c
	int_pet.c
	int_storage.c
	inter.c
	(login/)
	login.c
	(map/)
	ほとんど.c

--------------------
//0913 by Kalen

・GVGScriptの修正
　911対応
　フラグからアジトへ戻る機能追加
　戻るときに聞かれるように修正(TESTscript)
　砦取得時::OnRecvCastleXXXを発動するように修正
	(conf/gvg/)
	ほとんど.txt

--------------------
//0912 by (凸)
・このファイルの文字化けとTEST_prtg_cas01_AbraiJの文字化けを修正
・バグ報告スレの>>19-20を取り込み
・昔やっちまったbattle_athena.confの誤字の訂正

	(common)
	mmo.h
		#define MAX_STAR 3に修正
	(conf)
	battle_athena.conf
	(conf/gvg/)
	TEST_prtg_cas01_AbraiJ.txt
	(map)
	atcommand.c
		get_atcommandinfo_byname() 修正


--------------------
//0911 by Michael_Huang

	Mounting Emblem of the Flag-NPC.
	(Added Script Command: FlagEmblem).

(conf/gvg/)
	TEST_prtg_cas01_AbraiJ.txt (FlagEmblem Test)

	(map/)
	map.h struct npc_data{}
	clif.c clif_npc0078()
	script.c buildin_flagemblem()

--------------------
//0910 by RR
・スクリプトの間違いを修正
(conf/gvg/)
	ev_agit_payg.txt
	ev_agit_gefg.txt

・ひな祭りに一度入ったらマップ変数が残ったままになるので、マップ変数を使わないよう変更
(一時的マップ変数にすれば問題ないとも言えますが、
town_guideとtown_kafraに時期限定の物が常駐してしまうのが気になったので、
それらをevent_hinamatsuriへ移動し、普段のをdisableしています)
	(conf/npc/)
	npc_event_hinamatsuri
	npc_town_guide
	npc_town_kafra

・スキルリセット時のスキル取得制限判定をスキルポイント48以上消費から、
	スキルポイント58以上消費か残りスキルポイントがJOBLEVELより小さくなったときに変更
・@model時の服色染色制限を緩和(男アサ、ローグのみへ)
	(map/)
	pc.c pc_calc_skilltree()
	atcommand.c atcommand_model()


--------------------
//0909 by 胡蝶蘭

・NPCチャット関係の命令追加
	・waitingroom（NPCチャット作成）修正（イベントを起こす人数を指定可能）
	・delwaitingroom（NPCチャット終了）追加
	・enablewaitingroomevent（NPCチャットイベント有効化）追加
	・disablewaitingroomevent（NPCチャットイベント無効化）追加
	・getwaitingroomstate（NPCチャット状態所得）追加
	・warpwaitingpc（NPCチャットメンバーワープ）修正
	詳しくはscript_ref.txtを参照

	(map/)
	script.c/npc.c/npc.h/chat.c/chat.h/clif.c
		多々修正
	(doc/)
	script_ref.txt
		修正
	(conf/sample/)
	npc_test_chat.txt
		追加命令のテストスクリプト

・スクリプトの間違いを修正
	(conf/npc/)
	npc_event_skillget.txt
	npc_event_yuno.txt
	npc_town_lutie.txt
	npc_turtle.txt
		謎命令additemをgetitemに置換
	npc_town_guide.txt
		謎命令scriptlabelをコメント化
	npc_event_momotaro.txt
	npc_job_swordman.txt
	npc_job_magician.txt
		';'付け忘れ修正
	(conf/gvg/)
	ev_agit_aldeg.txt
		@GID4を@GIDa4に置換
	ev_agit_gefg.txt
	ev_agit_payg.txt
		Annouceに色指定と';'の付け忘れを習性
	

・AthenaDB計画のデータとりこみ、その他修正
	安定しているデータかどうかわかりませんが。

	(db/)
	item_db.txt/mob_db.txt/mob_skill_db.txt
		AthenaDB計画のデータとりこみ
	mob_skill_db.txt.orig
		以前のデータ（コメント部分などの参考に）
	(conf/)
	water_height.txt/mapflag.txt
		AthenaDB計画のデータとりこみ
	map_athena.conf
		npc_monster3*.txtを削除
		追加マップデータ (by ID:UVsq5AE)
	(conf/mob/)
	npc_monster.txt
		AthenaDB計画のデータとりこみ

--------------------
//0908 by 胡蝶蘭

・スクリプトのエラーチェック処理を増やした
	・文字列の途中で改行があるとエラーを出すように。
	・関数呼び出し演算子'('の直前に関数名以外があるとエラーを出すように。
	・命令があるべきところに関数名以外があるとエラーを出すように。
	・命令および関数の引数区切りの','を省略すると警告を出すように。
	・命令および関数の引数の数が異なると警告を出すように。

	(map/)
	script.c
		色々修正

・NPCスクリプト修正
	(conf/npc/)
	npc_town_guide.txt
		４行目はいらないようなのでエラーが出ないようにコメント化
	npc_event_hat.txt
		コモドの仮面職人とフェイヨンの青年 (by ID:dS8kRnc)
	(conf/sample/)
	npc_card_remover.txt
		@menuを使って短くした＆文章少し修正

・その他
	(db/)
	skill_tree.txt
		Sage応急手当

--------------------
//0907 by p
・atcommand() の肥大化がひどいのでリファクタリング
  @ コマンドを追加する場合は、atcommand.h 内で定数を、atcommand.c 内で
  関数定義マクロとマッピングテーブル、処理用の関数を記述してください。
・global 変数の atcommand_config を消去。
  @ コマンド毎のレベルは get_atcommand_level() で取得してください。
・一部のキャラ名を取る @ コマンドで、半角スペースを含む名前のキャラを
  正常に処理できていなかった問題を修正。
  この影響により、@rura+ など、キャラ名がパラメータの途中にあったものは
  全て最後に回されています。
・@ コマンドの文字列を正常に取得できなかった場合に、バッファの内容を
  チェックせずに処理を行おうとしていた部分を修正しました。

	(common/)
	mmo.h
	(map/)
	atcommand.h
	atcommand.c
	clif.h
	clif.c

--------------------
//0906 by Selena
・胡蝶蘭さんの修正にあわせて、バルキリーレルム１以外のスクリプトの修正。
・@コマンド入力ミスの際にエラーメッセージを表示。
	(conf/gvg/)
	ev_agit_aldeg.txt
	ev_agit_gefg.txt
	ev_agit_payg.txt
	ev_agit_prtg.txt
	aldeg_cas01～05.txt
	gefg_cas01～05.txt
	payg_cas01～05.txt
	prtg_cas02～05.txt
	(map/)
	atcommand.c

--------------------
//0905 by 管理人

・サーバーsnapshot
・前スレのファイル取り忘れた人がいるかもしれないので

--------------------
//0904 by 胡蝶蘭

・スクリプト処理修正
	・char/interサーバーに接続した時にOnCharIfInit/OnInterIfInitイベントが
	  呼ばれるようになりました。
	  OnAgitInitはOnInterIfInitに変更すべきです。
	・getcastledata命令で第２パラメータが0のとき、第３パラメータに
	  イベント名を設定できます。このイベントはギルド城のデータを
	  Interサーバーから所得完了したときに実行されます。
	・起こすNPCイベント名を"::"で始めると、同名ラベルを持つ全NPCのイベント
	  を実行できます。
	  たとえば、getcastledata "prtg_cas01.gat",0,"::OnRecvCastleP01";
	  とすると全てのNPCの OnRecvCastleP01ラベルが実行されます。
	・requestguildinfo命令追加。特定ギルドの情報をInterサーバーに
	  要求できます。第１パラメータはギルドID、第２パラメータはイベント名で
	  このイベントはギルド情報をInterサーバーから所得完了したときに
	  実行されます。
 
	(map/)
	guild.c/guild.h/npc.c/npc.h/script.c/intif.c/chrif.c
		色々修正

・ギルド城関連NPC修正
  （バルキリーレルム１のみ修正。他の城のスクリプトは各自で弄ってください。
    というか、むしろ弄ったらあっぷしましょう）
  	・初期化処理をOnAgitInitでなくOnInterIfInitに変更。
	・城データ所得完了処理としてOnRecvCastleP01を追加。
	・鯖再起動時、ギルド専属カプラが正しく表示されるように。
	・ギルド専属カプラの名前を"カプラ職員::kapra_prtg01"に変更。
	  （"::"以降はエクスポートされる名前で、"::"以前が表示名）
	  "カプラ職員#prt"より名前を長くして競合しにくくするためです。
	  この関係で、disablenpcなどのパラメータを"kapra_prtg01"に修正。
	(conf/gvg/)
	prtg_cas01.txt
		ギルド専属カプラ修正
	ev_agit_prtg.txt
		初期化処理修正（バルキリーレルム１のみ）
	TEST_prtg_cas01_AbraiJ.txt
		ギルド専属カプラ雇用/城破棄修正

・NPCの修正
	(conf/npc/)
	npc_job_swordman.txt
	npc_event_hat.txt
		修正

・アカウントを削除してもアカウントIDを再利用しないように修正
・ギルド/パーティについても一応同等の処理追加（コメント化されています。
  ギルドやパーティはIDを再利用してもおそらく問題ないため）

	(login/)
	login.c
		読み込み/保存処理修正
	(char/)
	int_guild.c/int_party.c
		読み込み/保存処理修正

--------------------
//0903 by 胡蝶蘭

・l14/l15およびプレフィックスlを"推奨されない(deprecated)"機能としました。
	・まだ使用できますが、今後の動作が保障されないので、速やかに代替機能を
	  使用するように移行してください。
	・プレフィックス'l'は代替機能のプレフィックス'@'を使用してください。
	・l15は代替機能の@menuを使用してください。
	・l14は代替機能はありません。input命令の引数を省略しないで下さい。
	・これらの推奨されない機能を使用すると警告メッセージがでます。

	(map/)
	script.c
		parse_simpleexpr()修正
	(conf/warp/)
	npc_warp.txt/npc_warp25.txt/npc_warp30.txt
		変数名l0を@warp0に修正
	(conf/npc/)
	npc_event_hat.txt
		変数名l15を@menuに修正
	(doc/)
	script_ref.txt
		配列変数の説明追加
		変数のプレフィックス'l'、input命令のl14、menu命令のl15の
		説明を修正
	
--------------------
//0902 by 胡蝶蘭

・スクリプトが配列変数に対応。
	・array[number]のように使います。数値型、文字列型両方使えます。
	・使えるプレフィックスは @, $, $@ です。
	  （一時的キャラクター変数、一時的/永続的マップサーバー変数）
	・number==0は配列じゃない変数と値を共有します。
	  （@hoge[0]と@hogeは同じ変数を表す）
	・まだ仮実装段階なのでバグ報告よろしくお願いします。
・マップサーバー変数の読込中にCtrl+Cをするとデータ破損の可能性がある問題を修正.
・マップファイル読み込み画面がさびしいのでせめてファイル名を表示するように。

	(conf/sample/)
	npc_test_array.txt
		配列変数テストスクリプト
	(map/)
	script.c
		buildin_set(),buildin_input(),get_val(),
		parse_simpleexpr()修正
		buildin_getelementofarray()追加
		do_final_script()修正など
	map.c
		map_readmap(),map_readallmap()修正

--------------------
//0901 by ぴざまん

・露店バグの修正

	(map/)
	pc.c
		pc_cartitem_amount() 追加。
	vending.c
		vending_openvending() 修正。
	clif.c
		clif_parse_NpcClicked() 修正。
	pc.h 修正。

--------------------
//0900 by ぴざまん

・アブラカダブラのランダムスキル発動率をabra_db.txtで設定できるように。
・スフィアーマインとバイオプラントの微修正。
・Noreturnマップで蝶が消費だけされるバグ修正。
・一部のアブラ固有スキルが正しく動作しなかったバグ修正。
	(map/)
	mob.c
		mob_damage()、mobskill_use() 修正。
		mob_skillid2skillidx() 追加。
	skill.c
		skill_readdb()、skill_abra_dataset() 修正。
		skill_castend_nodamage_id()、skill_castend_pos2() 修正。
	script.c
		buildin_warp() 修正。

	skill.h 修正。
	map.h 修正。
	(db/)
	abra_db.txt 追加。
	skill_db.txt 修正。

--------------------
//0899 by 胡蝶蘭

・取り巻きMOBの処理修正
	・取り巻き召喚でコアを吐くバグ修正
	・主が別マップに飛ぶと、テレポートで追いかけるように修正
	・取り巻き処理をより軽く変更

	(map/)
	mob.c
		mob_ai_sub_hard_mastersearch()をmob_ai_sub_hard_slavemob()
		に名前を変えて処理修正。
		mob_summonslave()修正

--------------------
//0898 by 胡蝶蘭

・eathenaからCardRemoverNPCの取り込み
	NPCデータも日本語訳してますが、かなり適当です。

	(map/)
	script.c
		buildin_getequipcardcnt(),buildin_successremovecards()
		buildin_failedremovecards()追加
	(conf/sample/)
	npc_card_remover.txt
		カード取り外しNPCの日本語訳
		プロンテラの精錬所の中の左下の部屋にいます

・ポータルで別マップに飛ばしたMOBがそのマップに沸き直すバグ修正
	(map/)
	map.h
		struct mob_dataにmメンバ追加
	mob.c
		mob_spawn(),mob_once_spawn()修正
	npc.c
		npc_parse_mob()修正
	

--------------------
//0897 by ぴざまん

・細かい調整
・ストリップ系とケミカルプロテクション系スキルの全実装
　本鯖での細かい仕様が分ったので実装しました。
　確率は暫定です。

	(map/)
	pc.c
		pc_isequip() 修正
	skill.c
		skill_status_change_start()、skill_castend_nodamage_id() 修正。
		skill_abra_dataset() 修正。
	battle.c
		battle_get_def()、battle_get_atk2() 修正。
		battle_get_vit()、battle_get_int() 修正。
	(db/)
	const.txt 修正。
	skill_db.txt 修正。
	cast_db.txt 修正。

--------------------
//0896 by 胡蝶蘭

・永続的マップ変数機能追加
・マップ変数を文字列型変数としても使用できるようにした
	・今までのプレフィックス $ は永続的になります。
	  一時的マップ変数を使用する場合はプレフィックス $@ を指定してください.

	・永続的/一時的ともに文字列型に対応しています。
	  文字列型のポストフィックスは$です。

	<例> $@hoge 数値型一時マップ変数、$hoge$ 文字列型永続マップ変数
	・永続マップ変数はデフォルトでは save/mapreg.txt に保存されます。
	  これはmap_athena.confのmapreg_txtで設定できます。

・str_dataが再割り当てされるとマップ変数が正常に使用できないバグ修正
	・strdbからnumdbにして、変数名はstr_bufに入れるように。

・map_athena.confのdelnpc,npc:clearが正しく働かないバグ修正

	(map/)
	npc.c
		npc_delsrcfile(),npc_clearsrcfile()修正
	script.c / script.h
		マップ変数系かなり修正
	map.c
		map_read_config()修正など
	(conf/)
	map_athena.conf
		mapreg_txt追加
	(doc/)
	conf_ref.txt
		mapreg_txt,help_txt,motd_txt追加
	script_ref.txt
		文字列型変数の説明修正

--------------------
//0895 by Selena

・mapflagにnozenypenaltyを追加。
　GVGや街中のテロなどで死亡した際に、Zenyペナルティー発生を外す用。

	(map/)
	pc.c
		pc_setrestartvalue() 修正
	script.c
		buildin_setmapflag()、buildin_removemapflag() 修正
	npc.c
		npc_parse_mapflag() 修正
	map.h
		map_data() 修正
	(db/)
	const.txt 修正。

--------------------
//0894 by ぴざまん

・コーマ以外のアブラカダブラ固有スキル全実装。
　オートスペルにはレベルアップ以外多分全部乗せれます。(オートスペルレベルアップは未テスト)
・アブラカダブラ仮実装
　発動スキルがレベル依存じゃありません。
　全ての発動率が理論上均一です。
　アイテムスキルを使って実装しているので一部の使用条件を無視します（ジェム罠気球等）
・アイテムスキルがキャスト・ディレイ無しだったのを修正。

	(map/)
	skill.c
		skill_castend_nodamage_id()、skill_use_id()、skill_use_pos() 修正。
		skill_abra_dataset() 追加。
	(db/)
	skill_db.txt 修正。

--------------------
//0893 by 胡蝶蘭

・他マップからポータルの上にワープしてきたPCがワープしない問題を修正
・チャット中のPCをワープポータルで飛ばすかどうか設定可能に
・MOBをワープポータルで飛ばすかどうか設定可能に
	MOBのワープポータルを許可すると、テロが簡単にできるので注意。

・アカウント変数変更と同時にファイルに書き出すように修正
・マップデータのロード部分のログ表示はあまり重要じゃないと思うので変更。

	(char/)
	inter.c
		mapif_parse_AccReg()でinter_accreg_save()を呼ぶように修正
	(map/)
	mob.c/mob.h
		mob_warp()の引数変更と修正
	battle.c/battle.h
		mob_warp()呼び出しの引数修正
		battle_config関連
	map.c
		map_readallmap(),map_readmap()修正
	pc.c
		pc_setpos()修正
	skill.c
		mob_warp()呼び出しの引数修正
		skill_unit_onplace()修正
	(conf/)
	battle_athena.conf
		chat_warpportal,mob_warpportalの追加
	(doc/)
	conf_ref.txt
		chat_warpportal,mob_warpportalの追加

--------------------
//0892 by 胡蝶蘭

・各種confファイルで別ファイルをインポートできるようにした
	・自分のサーバー用の設定を別ファイルに記述できるようになります。
	・全て「import: ファイル名」形式で記述します。
	・各種confファイル（login,char,map,inter,atcommand,battle）の最後に
	  conf/import/*_conf を読むように指定したので、そこに自分用の設定を
	  書いておけば、変更部分のみオーバーライドします。
	  msg,scriptのconfについては、この限りではありませんが、import命令の
	  処理は追加されているので、自分でimport命令を書けば動きます。
	・新しいスナップショットが出た場合などに、このconf/importフォルダを
	  昔のAthenaからコピーするだけで自分用の設定を適用できるようになります.

・map_athena.confのmapとnpcで追加したファイルを削除できるようにした
	・上に関連する変更です。
	・delmap,delnpc命令を使用すれば、map,npc命令で追加したファイルを
	  読み込まないように指定できます。ここでファイル名ではなく、
	  all と指定するとそれまでに指定されたファイルを全て読み込まなくします.
	・map,npc命令で、ファイル名にclearを指定すると、
	  delmap,delnpcのallと同等の動作をするようになりました。

・login_athena.confのallowとdenyをクリアできるようにｓた
	・allowおよびdeny命令でclearを指定すると以前のホスト情報を全削除します.

	(conf/)
	各種confファイルの最後にimport命令追加
	(conf/import)
	*.txt
		インポートされるファイル。これらに自分用の設定を書くとよい。
	(login/)
	login.c
		login_read_config()修正
	(char/)
	char.c/inter.c
		char_read_config(),inter_read_config()修正
	(map/)
	map.c
		map_read_config(),map_addmap()修正、map_delmap()追加
	npc.c
		npc_addsrcfile()修正,npc_delsrcfile(),npc_clearsrcfile()追加
	battle.c/atcommand.c/script.c
		battle_read_config(),atcommand_read_config(),
		msg_read_config(),script_read_config()修正
	(doc/)
	conf_ref.txt
		修正

--------------------
//0891 by (凸)

・「スキル使用の後は、しばらくお待ちください」を表示するかどうか設定できるようにした。
    ・本鯖相違スレッド　其のⅡ>>5さんのコードをパクリました。
	(doc/)
	conf_ref.txt 修正。
	(conf/)
	battle_athena.conf 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_config_read() 修正。
	clif.c
		clif_skill_fail() 修正。

--------------------
//0890 by 死神

・ギルド倉庫を一度に一人だけが使用するように変更。(未テスト)
・battle_athena.confからplayer_undead_nofreeze 削除。
・@コマンド@gstorage 追加。
・スクリプトguildstorageをguildopenstorageに変更。
・その他細かいバグ修正。
	(doc/)
	conf_ref.txt 修正。
	script_ref.txt 修正。
	(conf/)
	atcommand_athena.conf 修正。
	battle_athena.conf 修正。
	help.txt 修正。
	(conf/sample/)
	gstorage_test.txt 追加。
	(char/)
	makefile 修正。
	int_storage.h 修正。
	int_storage.c
		inter_storage_delete()、inter_guild_storage_delete() 追加。
	int_guild.c
		guild_check_empty()、mapif_parse_BreakGuild() 修正。
	(map/)
	makefile 修正。
	battle.h 修正。
	battle.c
		battle_config_read() 修正。
	guild.c
		guild_broken() 修正。
	storage.h 修正。
	storage.c
		storage_guild_storageopen() 修正。
		storage_delete()、guild_storage_delete() 追加。
	script.c
		buildin_guildstorage() を buildin_guildopenstorage()に変更。
	intif.c
		intif_parse_LoadGuildStorage() 修正。
	mob.c
		mob_summonslave()、mob_damage()、mob_delete() 修正。
		mob_catch_delete()、mob_readdb() 修正。
	skill.c
		skill_castend_nodamage_id()、skill_status_change_start() 修正。
	clif.c
		clif_parse_ActionRequest() 修正。
	atcommand.h 修正。
	atcommand.c
		atcommand() 修正。

--------------------
//0889 by 胡蝶蘭

・文字列型一時的キャラクター変数機能追加。
	・プレフィックス@,ポストフィックス$を使用します。（@hoge$など）
	・inputで文字列変数を指定すると文字列入力になります。
	・関係演算子（比較演算子）で文字列どうしを指定すると文字列の比較が
	  できます。数値と文字列を混ぜて比較することはできません。
	・とりあえずサンプル付けてます。

	(map/)
	map.h
		struct map_session_dataにnpc_str,regstr,regstr_numメンバ追加
	script.c
		buildin_set(),get_val(),buildin_input(),op_2num()など修正
		op_2str(),op_2()追加
	clif.c / clif.h
		01d5パケット長修正
		clif_parse_NpcStringInput(),clif_scriptinputstr()追加
	pc.c / pc.h
		pc_readregstr(),pc_setregstr()追加
	(doc/)
	script_ref.txt
		演算子の説明追加、変数の説明修正、input,menu修正
	(conf/sample/)
	npc_test_str.txt
		文字列変数を使用したスクリプトの例。
		文字列の代入、結合、比較、入力などのテストを行うもの。

--------------------
//0888 by 死神

・設計から間違っていたギルド倉庫修正。(ただ複数人の使用によるバグがある可能性はまだあります。)
・細かいバグ修正。
	(doc/)
	inter_server_packet.txt 修正。
	conf_ref.txt 修正。
	(conf/)
	inter_athena.conf 修正。
	help.txt 修正。
	(common/)
	mmo.h 修正。
	(char/)
	makefile 修正。
	int_storage.h 修正。
	int_storage.c
		account2storage()、inter_storage_init()、storage_fromstr() 修正。
		inter_storage_save()、mapif_load_storage() 修正。
		mapif_parse_SaveStorage() 修正。
		guild_storage_fromstr()、guild_storage_tostr() 追加。
		inter_storage_save_sub()、inter_guild_storage_save_sub() 追加。
		inter_guild_storage_save()、mapif_parse_LoadGuildStorage() 追加。
		mapif_parse_SaveGuildStorage()、mapif_load_guild_storage() 追加。
		mapif_save_guild_storage_ack()、guild2storage() 追加。
	int_party.c
		inter_party_init() 修正。
	int_guild.h 修正。
	int_guild.c
		inter_guild_init() 修正。
		inter_guild_search() 追加。
	int_pet.c
		inter_pet_init() 修正。
	inter.c
		inter_init()、inter_save()、inter_config_read() 修正。
	(map/)
	makefile 修正。
	map.h 修正。
	map.c
		map_quit()、do_init() 修正。
	pc.c
		pc_setpos() 修正。
	storage.h 修正。
	storage.c
		do_init_storage()、do_final_storage()、account2storage() 修正。
		storage_storageopen()、storage_storageadd()、storage_storageget() 修正。
		storage_storageaddfromcart()、storage_storagegettocart() 修正。
		storage_storageclose()、storage_storage_quit() 修正。
		storage_storage_save() 修正。
		guild2storage()、storage_guild_storageopen() 追加。
		guild_storage_additem() 、guild_storage_delitem() 追加。
		storage_guild_storageadd()、storage_guild_storageget() 追加。
		storage_guild_storageaddfromcart()、storage_guild_storagegettocart() 追加。
		storage_guild_storageclose()、storage_guild_storage_quit() 追加。
	intif.h 修正。
	intif.c
		intif_send_storage()、intif_parse_LoadStorage()、intif_parse() 修正。
		intif_request_guild_storage()、intif_send_guild_storage() 追加。
		intif_parse_SaveGuildStorage()、intif_parse_LoadGuildStorage() 追加。
	clif.h 修正。
	clif.c
		clif_additem()、clif_parse_MoveToKafra() 修正。
		clif_parse_MoveFromKafra()、clif_parse_MoveToKafraFromCart() 修正。
		clif_parse_MoveFromKafraToCart()、clif_parse_CloseKafra() 修正。
		clif_parse_LoadEndAck() 修正。
		clif_guildstorageitemlist()、clif_guildstorageequiplist() 追加。
		clif_updateguildstorageamount()、clif_guildstorageitemadded() 追加。
	guild.c
		guild_broken() 修正。
	script.c
		buildin_openstorage()、buildin_guildstorage() 修正。
	skill.c
		skill_castend_nodamage_id() 修正。
	mob.c
		mob_summonslave()、mob_damage() 修正。
	atcommand.c
		atkillmonster_sub()、atcommand() 修正。

--------------------
//0887 by 獅子o^.^o

・(db/)
	skill_tree.txt 修正

--------------------
//0886 by ぴざまん

・サーバーsnapshot
・ファイル調整

--------------------
//0885 by huge

・ギルド共有倉庫の実装。guildstorageで開けます。
    自分の鯖で実験はしてみましたが、過疎地なので多人数ギルドになるとどう動くか分かりません。
    (念のためバックアップは必ず取っておいて下さい)
・areawarpで、対象マップ名を"Random"にすると、同マップ内でランダムに飛ぶように修正。
・GMコマンドで生き返したときにSPも全回復するように修正。
・ディボーションの条件をちょっと修正。

	(char/)
	int_storage.c
		mapif_load_storage() 修正。
		mapif_parse_SaveStorage() 修正。
	inter.c
		inter_send_packet_length[] 修正。
		inter_recv_packet_length[] 修正。
	(map/)
	atcommand.c
		@alive,@raise,@raisemap 修正。
	intif.c
		packet_len_table[] 修正。
		intif_request_storage() 修正。
		intif_send_storage() 修正。
		intif_parse_LoadStorage() 修正。

	map.h
		map_session_data stateにstorage_flag 追加。
	script.c
		buildin_areawarp_sub() 修正。
		buildin_openstorage() 修正。
		buildin_guildstorage() 追加。
	skill.c
		skill_castend_nodamage_id() 修正。
	storage.c
		account2storage() 修正。
		storage_storageopen() 修正。
		storage_storage_save() 修正。

--------------------
//0884 by 死神

・細かいバグ修正。
・battle_athena.confにpet_str、zeny_penalty、resurrection_exp 追加。
・0878の銀行関係のコードはもういらないので全て削除。
・zeny_penaltyを設定して使う場合は手数料はなくした方がいいかも。
・ポーションピッチャーでpercenthealにもPPとLPによる回復ボーナスが付くように変更。(ただvitやint、HPR、MPRによる回復ボーナスが付きません。)
・ほとんど未テスト。
	(common/)
	mmo.h 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	map.c
		do_init()、do_final() 修正。
	script.c
		buildin_openbank() 削除。
		buildin_failedrefitem() 修正。
	storage.h 修正。
	storage.c
		do_init_bank()、do_final_bank()、account2bank() 削除。
		storage_bank()、storage_readbank() 削除。
	skill.c
		skill_castend_nodamage_id()、skill_attack() 修正。
	battle.h 修正。
	battle.c
		battle_calc_pet_weapon_attack()、battle_config_read() 修正。
	pc.c
		pc_setrestartvalue() 修正。
	clif.c
		clif_skill_nodamage()、clif_refine() 修正。
	itemdb.c
		itemdb_isequip3() 修正。
	atcommand.c
		atcommand() 修正。

--------------------
//0883 by Kalen

・Warp色々修正
	・アサシンギルド周り修正(昔のままのリンクだったので現在の状態に修正。)
	・YunoのWarp全面見直し(YumilLoop修正、SageCastleRandomWarp追加、女医さんの家追加)
	・モンクギルド周り追加
・NPC色々修正
	・帽子作成NPCを別ファイルへ。一部追加(ep2.5追加分)
	　参考Data(R.O.M776): ttp://green.sakura.ne.jp/~youc/ro/data/itemmaking.html#04
	・アサシンギルド修正
	・二次職転職関係NPC一部追加(これでコモド小劇場へ行けます)
	・マスターアルケミストの台詞修正
	・アルデバランの案内要員を移動&台詞修正&イメージ追加
	・BBSにあがっていたコモドスクリプト追加(event_hat等へ分散)
	・コンロンクエスト関係NPC一部追加(女医[yuno]、ネル[prontera])
	(conf/warp/)
		npc_warp.txt
		npc_warp30.txt
		npc_warp_job.txt
	(conf/npc/)
		npc_event_hat.txt(新規)
		npc_job_2nd.txt
		npc_job_alchemist.txt
		npc_town_aldebaran.txt
		npc_town_comodo.txt
		npc_town_gonryun.txt
		npc_town_guide.txt
		npc_town_yuno.txt
		npc_town_lutie.txt

--------------------
//0882 by 胡蝶蘭

・スクリプトに0881相当のアカウント共有変数機能のプレフィックス変更
	・0881のアカウント変数はプレフィックス##になりました。
	・0881のアカウント変数は全ワールドで共有されます。
	・変数の個数はmmo.hのACCOUNT_REG2_NUMで定義されています(16)。
・ワールド内のアカウント共有変数機能追加
	・変数名のプレフィックスは#です。
	・変数の個数はmmo.hのACCOUNT_REG_NUMで定義されています(16)。
	・0881の銀行スクリプトはこちらを使用するようになります。
	  よって以前のデータがつかえないのであらかじめ引き出しておいてください.
	・変数データは save/accreg.txt に保存されます。
	  このファイル名は inter_athena.conf で変更可能です。conf_ref.txt参照。

	(common/)
	mmo.h
		ACCOUNT_REG_NUMを16に、ACCOUNT_REG_NUM2追加
		struct mmo_charstatusにaccount_reg2_num,account_reg2メンバ追加
	(login/)
	login.c
		account_regを全てaccount_reg2に置き換え
	(char/)
	char.c
		account_regを全てaccount_reg2に置き換え
	inter.c
		ワールド内アカウント変数機能追加。
		inter_accreg*()追加、accreg_db追加など。
	(map/)
	chrif.c/chrif.h
		account_regを全てaccount_reg2に置き換え
		0881でのバグを修正
	intif.c/intif.h
		ワールド内アカウント変数機能追加。
	pc.c/pc.h
		pc_*accountreg()=>pc_*accountreg2()に。
		pc_setaccountreg(),pc_readaccountreg()追加。
	script.c
		buildin_set(),buildin_get_val(),buildin_input()修正
	(doc/)
	inter_server_packet.txt
		ワールド内アカウント変数関係
	conf_ref.txt
		accreg_txt追加
	
--------------------
//0881 by 胡蝶蘭

・スクリプトにアカウント共有変数機能追加
	・変数名にプレフィックス#を付けることでアカウント共有変数になります。
	・アカウント変数は変更した時点で全サーバーにポストされるので
	  頻繁に書き換えるとサーバー間通信が肥大化します。
	・アカウント変数は変更した時点（そしてそれがlogin鯖に届いた時点）で
	  account.txtに書き出されます。
	・グローバル変数（永続変数）の個数を96に減らし、減った32個分を
	  アカウント変数にしていますが、mmo_charstatusのサイズが
	  16000byteを超えない限り増やすことができます。＜0879の変更を参照
	  変数の個数はmmo.hのACCOUNT_REG_NUMで定義されています。
	・0878の銀行をアカウント変数を使用するように修正
	  bank.txtのデータが使えなくなるのであらかじめ引き出しておいて下さい。
	
	(common/)
	mmo.h
		GLOBAL_REG_NUMを96に、ACCOUNT_REG_NUMを追加
		struct mmo_charstatusにaccount_reg_num,account_regメンバ追加
	(login/)
	login.c
		パケット2728処理追加
	(char/)
	char.c
		パケット2729,2b10処理追加
	(map/)
	chrif.c
		chrif_saveaccountreg(),chrif_accountreg()
		(パケット2b10,2b11処理)追加。
	pc.c/pc.h
		pc_readaccountreg(),pc_setaccountreg()追加
	script.c
		buildin_set(),buildin_get_val(),buildin_input()修正
	(conf/sample/)
	bank_test.txt
		アカウント変数使用版の銀行スクリプト

--------------------
//0880 by 死神

・ポーションピッチャーを正しく実装とちょっと機能拡張。
・ポーションピッチャーでレベル別に使えるアイテムをskill_require_db.txtに設定できるようにしました。ただポーションピッチャーで使えるアイテムはitemheal、percentheal、sc_start、sc_end以外の物が入っていると正しく動作しません。
レベル5までは本鯖に合わせていますが最大レベルを10まで拡張するとレベル6 - マステラの実、7 - ローヤルゼリー、8 - イグドラシルの種、9 - イグドラシルの実、10 - バーサークポーションに設定しています。skill_db.txtを修正すればこれが有効になります。(どこを修正するかもわからない人は諦めることです。) ポーションピッチャーによるアイテム使用は使用条件を無視します。少しはアルケミストに希望ができたかも...(多分無理...)
・battle_athane.confにproduce_item_name_input、produce_potion_name_input、making_arrow_name_input、holywater_name_input 追加。
・パーティ員にだけ使うスキルとギルド員にだけ使うスキルを設定できるように修正。
・その他細かい修正。
	(conf/)
	battle_athane.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	db_ref.txt 修正。
	(db/)
	skill_db.txt 修正。
	skill_require_db.txt 修正。
	(map/)
	map.h 修正。
	skill.h 修正。
	skill.c
		skill_status_change_timer()、skill_attack()、skill_use_id() 修正。
		skill_castend_nodamage_id()、skill_castend_damage_id() 修正。
		skill_castend_id()、skill_castend_pos()、skill_produce_mix() 修正。
		skill_arrow_create()、skill_check_condition() 修正。
		skill_status_change_clear()、skill_readdb() 修正。
	mob.c
		mobskill_use_id()、mob_changestate() 修正。
	pc.c
		pc_itemheal()、pc_percentheal()、pc_calcstatus() 修正。
	battle.h 修正。
	battle.c
		battle_delay_damage()、battle_damage()、battle_heal() 修正。
		battle_get_adelay()、battle_get_amotion() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_weapon_attack()、battle_config_read() 修正。
	clif.c
		clif_skill_fail() 修正。
	script.c
		buildin_sc_start()、buildin_sc_end() 修正。
	makefile 修正。

--------------------
//0879 by 胡蝶蘭

・送信FIFOのバッファオーバーフローの脆弱性の修正
	・2048バイト以上のパケットを送るとき、FIFOが満杯に近ければ
	  バッファオーバーフローによる不正アクセスが起こっていた問題修正。
	・FIFOが満杯に近いときWFIFOSETされたパケットが捨てられていた問題修正。
	・FIFOがオーバーフローする場合、自動的にFIFOを拡張するようにした。
	  （ただし、一度にWFIFOSETするパケットが16384バイト以下と仮定している）
	・「socket: ? wdata expanded to ???? bytes」はFIFOが拡張されたときに
	  でるログだが、エラーではなく、パケットは正しく送信される。
	・「socket: ? wdata lost !!」はパケットが喪失したことを表すログで、
	  エラーであるが64KBを超える超巨大なパケットをWFIFOSETしないと出ない。
	・16384バイトを超えるパケットをWFIFOSETするとエラーメッセージなしに、
	  不正アクセスが起こる可能性があるので、超えないようにすること。
	
	(common/)
	socket.c /socket.h
		WFIFOSET()をマクロから関数に変更
		realloc_fifo()追加
	
・サーバー間通信FIFOのバッファサイズを大きくした
	・大量のデータが通信されたときにデータ処理遅延が起きにくくするため。
	・メモリ使用量が増えた。(ぎりぎりの人は65536に設定すると元通りになる)
	・サーバー間通信のFIFOサイズは mmo.h で定義されている。
	  変更する場合は64KB(65536)以上の値にすること。
	  大きくすると巨大データ受信時の遅延が減るがメモリを多く使う。
	・@kickall時などにデータ送信が激しくなるので変更したが、
	  同時ログイン人数が少ないと増やしても意味は無い。

	(common/)
	mmo.h
		FIFOSIZE_SERVERLINKマクロ追加。
	(login/)
	login.c
		2710パケットでrealloc_fifo()を呼ぶように
	(char/)
	char.c
		2af8パケットでrealloc_fifo()を呼ぶように
		check_connect_login_server()でrealloc_fifo()を呼ぶように
	(map/)
	chrif.c
		check_connect_char_server()でrealloc_fifo()を呼ぶように

--------------------
//0878 by huge

・カプラ銀行サービス。
	自分の鯖で実装してたんですが、意外と好感触だったので出してみます。
	NPCscriptで、openbank(0);で預金額を返して、中に数字を入れると出し入れします。
	詳しくはサンプルを同封したので、それを参照。

	(common/)
	mmo.h
		struct bank 追加。
	(map/)
	map.c
		do_final(),do_init() 修正。
	script.c
		buildin_openbank() 追加。
	storage.c
	storage.h
		グローバル変数追加。
		do_init_bank(),do_final_bank(),account2bank() 追加。
		storage_bank(),storage_readbank() 追加。

--------------------
//0877 by 胡蝶蘭

・login鯖のアクセスコントロールがネットマスク表記に対応
	192.168.0.0/24 や 192.168.0.0/255.255.0.0 といった表記に対応。
・battle_athena.confにGMが無条件で装備品を装備できる＆
  無条件でスキルを使用できる設定追加
	これらはデバグ用なので動作に不都合があるかもしれません。

	(login/)
	login.c
		check_ip()修正,check_ipmask()追加
	(map/)
	battle.c/battle.h
		battle_configにgm_allequip,gm_skilluncond追加
		battle_config_read()修正更
	skill.c
		skill_check_conditio()修正
	pc.c
		pc_isequp()修正
	(doc/)
	conf_ref.txt
		allow変更、gm_all_equipment、gm_skill_unconditional追加

--------------------
//0876 by 死神

・細かいバグ修正。
・@コマンドにテストの為に入れていた物が入っていたので修正。
・ハンマーフォールの射程を5から4に修正(本鯖射程は不明)とリザレクションが無属性だったのを聖属性に修正。
	(db/)
	skill_db.txt 修正。
	(map/)
	mob.c
		mob_catch_delete()、mob_stop_walking() 修正。
	storage.c
		storage_additem() 修正。
	pc.c
		pc_damage()、pc_stop_walking() 修正。
	clif.c
		clif_parse_UseSkillToId()、clif_parse_UseSkillToPos() 修正。
	battle.c
		battle_calc_magic_attack() 修正。
	skill.c
		skill_check_condition() 修正。
	atcommand.c 修正。

--------------------
//0875 by 胡蝶蘭

・party_share_levelをinter_athena.confに移した
  (パーティ関連の処理の管轄がinter鯖のため)
・inter_athena.confにinter_log_file項目追加
・ギルド作成/解散/城占領/城破棄がログに残るように
・ギルド解散時にメモリリークしていた問題を修正
	(char/)
	char.c/char.h
		party_share_level関連
	(inter/)
	inter.c/inter.h
		party_share_level / inter_log_file 関連
		ログ出力用にinter_log()追加
	int_guild.c
		作成/解散/城占領/城破棄をログに出力
		メモリリーク修正
	(doc/)
	conf_ref.txt
		修正

・サーバー状態確認用CGIスクリプト添付など
	・自己責任＆詳細な解説無し、質問されてもスルーする可能性有り
	・エディタで開いたら少し説明有り
	・CGI設置の基本さえわかれば問題ないはず
	
	(tool/cgi/)
	serverstatus.cgi
		サーバー状態確認用CGIスクリプト
	addaccount.cgi
		説明修正

--------------------
//0874 by Kalen
・WhiteDayイベント追加
	conf/npc/npc_event_whiteday.txt(新規)
	 ただ、お菓子売ってるだけみたい…GMがなにやるのかは知りませんが。
	 sakROのほうではホワイトチョコらしきものが追加されたのに
	 jROで追加されたのは雛壇撤去パッチのみ(*´д`;)…

・Alchemistギルドで乳鉢、製造書を変えるように
	conf/npc/npc_job_alchemist.txt(新規)
	 転職クエストが分からなかったので温めていましたが
	 買えないと不便と聞いたので、追加

・染色NPC実装
	conf/npc/npc_event_dye.txt(更新)
	 髪型変更がsakROに来たらしいので
	 なんとなーく更新

--------------------
//0873 by 死神

・@コマンドitem2とkillmonster 追加。
・スクリプトgetitem2とkillmonsterall 追加。
・矢作成で作られた矢も製造者の名前が付くように修正。
・battle_athena.confにmonster_class_change_full_recover追加。
・装備スクリプトにbWeaponComaEleとbWeaponComaRace 追加。
・少し間違いがあったダメージ計算式修正。
・bInfiniteEndureの処理をインデュア表示なしで内部処理するように変更。
・オートスペルでcastend_nodamage_id()を呼ぶスキルも使用できるように修正。
・その他細かい修正とバグ修正。
・ほとんど未テストなのでバグがあったら報告お願いします。
	(conf/)
	help.txt 修正。
	atcommand_athena.conf 修正。
	battle_athena.conf 修正。
	char_athena.conf 修正。
	(db/)
	const.txt 修正。
	item_db.txt 修正。
	(doc/)
	item_bonus.txt 修正。
	script_ref.txt 修正。
	conf_ref.txt 修正。
	(map/)
	map.h 修正。
	map.c
		map_quit() 修正。
	skill.h 修正。
	skill.c
		skill_castend_nodamage_id()、skill_status_change_clear() 修正。
		skill_castend_id()、skill_castend_pos()、skill_arrow_create() 修正。
		skill_status_change_timer() 修正。
	pc.c
		pc_calcstatus()、pc_bonus2()、pc_equipitem() 修正。
		pc_unequipitem()、pc_damage() 修正。
	battle.h 修正。
	battle.c
		battle_get_dmotion()、battle_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_calc_magic_attack()、battle_config_read() 修正。
	clif.c
		clif_parse_LoadEndAck()、clif_damage()、clif_skill_damage() 修正。
		clif_skill_damage2() 修正。
	itemdb.h 修正。
	itemdb.c
		itemdb_isequip3() 追加。
	mob.h 修正。
	mob.c
		mob_delay_item_drop()、mob_damage()、mob_changestate() 修正。
		mob_class_change()、mob_delete()、mob_catch_delete() 修正。
	script.c
		buildin_getitem() 修正。
		buildin_killmonsterall_sub()、buildin_killmonsterall() 追加。
	atcommand.h 修正。
	atcommand.c
		atcommand() 修正。
		atkillmonster_sub() 追加。

--------------------
//0872 by ElFinLazz

・スキルポーションピッチャー修正
・スキルギムソバングドンボルオッネ具現
・スキルアブラカダブなら義コーマ具現
・コーマの武器オプション追加(種族, 千分率)
・オプション説明追加
	(db/)
	const.txt 修正.
	(doc/)
	item_bonus.txt 修正.
	(map/)
	map.h 修正.
	skill.c
		skill_castend_nodamage_id(), skill_unit_group(), skill_status_change_start() 修正.
	pc.c
		pc_calcstatus(), pc_bonus2(), pc_gainexp() 修正.
	battle.c
		battle_weapon_attack() 修正.

--------------------
//0871 by 死神

・0869のバグ修正。
・char_athena.confとlogin_athena.confに項目追加。(キャラ鯖とログイン鯖のログファイルを変えることができるようにしました。デフォルトでlog/フォルダーに入るのでlogフォルダーを作る必要があります。)
・エナジーコートの処理を少し修正。モンスターが使った場合はスキルレベル*6%の物理ダメージを減らすように変更。
・武器以外の物でも製造者の名前を表示するように変更。(本鯖ではプレゼントボックスと手作りチョコレット以外は表示されませんがパケットはあることだし入れてみました。)
・その他スキル関係の細かい修正。
・@コマンド一つとスクリプト一つを追加しましたが説明は後のパッチで書きます。
	(conf/)
	char_athena.conf 修正。
	login_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(login/)
	login.c
		parse_login()、login_config_read()、login_log() 修正。
	(char/)
	char.h 修正。
	char.c
		char_config_read()、make_new_char()、parse_char() 修正。
	int_party.c 修正。
	int_storage.c 修正。
	int_guild.c 修正。
	int_pet.c 修正。
	(map/)
	map.h 修正。
	skill.c
		skill_status_change_start()、skill_additional_effect() 修正。
		skill_castend_nodamage_id()、skill_check_condition() 修正。
		skill_status_change_clear()、skill_produce_mix() 修正。
		skill_status_change_timer() 修正。
	pc.c
		pc_calcstatus()、pc_insert_card()、pc_additem()、pc_cart_additem() 修正。
	storage.c
		storage_additem() 修正。
	battle.c
		battle_get_adelay()、battle_get_amotion()、battle_calc_damage() 修正。
	clif.c
		clif_additem()、clif_equiplist()、clif_storageequiplist() 修正。
		clif_tradeadditem()、clif_storageitemadded()、clif_use_card() 修正。
		clif_cart_additem()、clif_cart_equiplist()、clif_vendinglist() 修正。
		clif_openvending()、clif_arrow_create_list() 修正。
		clif_skill_produce_mix_list()、clif_parse_SelectArrow() 修正。
		clif_parse_ProduceMix() 修正。
	script.c
		buildin_produce() 修正。
		buildin_getitem2() 追加。
	atcommand.c
		atcommand() 修正。

--------------------
//0870 by shuto

・mapflagの攻城戦MAPにnomemo追加
・ギルド宝箱で、宝箱出現と同時にMAP鯖が落ちる問題修正(by ぴざまん)

--------------------
//0869 by 死神

・battle_athena.confにplayer_land_skill_limit、monster_land_skill_limit、party_skill_penaly 追加。
・char_athena.confにparty_share_level 追加。
・その他細かい修正。
	(conf/)
	char_athena.conf 修正。
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(char/)
	char.h 修正。
	char.c
		char_config_read() 修正。
	int_party.c
		party_check_exp_share() 修正。
	(map/)
	map.h 修正。
	skill.c
		skill_attack()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_status_change_start() 修正。
		skill_castend_pos() 修正。
	pc.c
		pc_calcstatus() 修正。
	mob.c
		mobskill_castend_pos() 修正。
	battle.h
	battle.c
		battle_get_adelay()、battle_get_amotion()、battle_calc_damage() 修正。
		battle_config_read() 修正。
	pet.c
		pet_data_init() 修正。

--------------------
//0868 by 死神

・マジックロッド実装とスペルブレイカー修正。
・マジックロッドの場合本鯖で使ってもなんの表示もなく発動の前には使ったかどうかの確認ができないのでスキル詠唱パケット(0x13e)を利用して使用する時スキル名が出るようにしています。(本鯖と違うぞとかで文句がこないように)
・スペルブレイカー詠唱キャンセルに関係なくskill_db.txtに設定されてるskill_typeがmagicのスキルのみ破ることができます。(ラグナゲートの説明を適用)
・skill_db.txtの書式が変わったので注意してください。ノックバック距離の設定もできますが念の為にいっておきますがA鯖でのテストでFWのノックバック距離は2でサンクも2であることを確認しています。韓国の2003年11月19日パッチ前の鯖ではありますが2-2は適用されている所なので本鯖の違いはないと思います。
・その他スキル関係の細かい修正。
・0867で書き忘れ。モンスターのヒールでアンデッドモンスターが攻撃されて自滅するのでヒールやリザの場合mob_skill_db.txtのval1(値1)に1を入れるとアンデッドモンスターも攻撃を受けず回復するようになります。本鯖ではモンスターのヒールはアンデッドに関係なく回復するようです。ただ個人的にはゾンビがヒールして自滅する方が正しいと思うのでmob_skill_db.txtで設定できるようにしております。
	(doc/)
	db_ref.txt 修正。
	(db/)
	cast_db.txt 修正。
	skill_db.txt 修正。
	(map/)
	skill.h 修正。
	skill.c
		skill_status_change_start()、skill_status_change_end() 修正。
		skill_castend_damage_id()、skill_castend_nodamage_id() 修正。
		skill_attack()、skill_status_change_timer()、skill_castcancel() 修正。
		skill_unit_onplace()、skill_use_id()、skill_castend_id() 修正。
		skill_readdb() 修正。
		skill_get_blewcount() 追加。
	mob.c
		mobskill_use_id()、mob_spawn()、mob_attack() 修正。
	battle.c
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_calc_misc_attack()、battle_weapon_attack() 修正。
	clif.c
		clif_damage() 修正。
	pet.c
		pet_attack() 修正。
	pc.c
		pc_attack_timer()、pc_authok() 修正。
		pc_spirit_heal()、pc_natural_heal_sub() 修正。

--------------------
//0867 by 死神

・スキル関係の細かい修正。
・battle_athena.confにplayer_undead_nofreeze追加。
・新しいアイテムパケットに対応。(PACKETVERを5以上にする必要があります。)
・mob_avail.txtでプレイヤーの姿を指定した時ペコペコや鷹を付けることができるように変更。頭下段次にオプションを設定できます。(ただハイディングとクローキングは指定できないようになっています。)
	makefile 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	client_packet.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_config_read() 修正。
	clif.c
		clif_mob_class_change()、clif_spawnmob()、clif_spawnpet() 修正。
		clif_damage()、clif_skill_damage()、clif_skill_damage2() 修正。
		clif_itemlist()、clif_cart_itemlist()、clif_storageitemlist() 修正。
		clif_mob0078()、clif_mob007b()、clif_pet0078()、clif_pet007b() 修正。
	pc.c
		pc_attack_timer() 修正。
	skill.c
		skill_castend_nodamage_id()、skill_additional_effect() 修正。
		skill_status_change_start() 修正。
	mob.h 修正。
	mob.c
		mobskill_castend_id()、mob_getfriendstatus_sub() 修正。
		mob_readdb_mobavail() 修正。

--------------------
//0866 by ぴざまん

・MOTDのメッセージを全て編集できるように変更。
・クローンスキル実装。
　ドル服のヒールアタックによるヒール習得は未テストです。
・ギルド宝箱仮実装。
　ヴァルキリー１のみです。
　商業投資による宝箱個数の算出式は適当です(初期個数4個としか知らないので)。
　Onclockイベントで動作させています。任意の時刻に変更してください。
・AthenaDB計画のmob_db.txtとmapflag.txtを入れておきました。

	(map/)
	pc.c
		pc_makesavestatus()、pc_calc_skilltree() 修正。
		pc_allskillup()、pc_calc_skillpoint() 修正。
		pc_resetskill()、pc_authok() 修正。
	skill.c
		skill_attack() 修正。
	map.h 修正。
	(conf/)
	gvg/TEST_prtg_cas01_AbraiJ.txt 修正。
	motd.txt 修正。
	mapflag.txt 修正。
	(db/)
	mob_db.txt 修正。

--------------------
//0865 by ぴざまん

・自分が占領しているアジトのエンペリウムを攻撃できたバグ修正。
・アブライが占領ギルドメンバー全員をマスターとみなしていたバグ修正。
　この修正に伴ってスクリプトリファレンスに改変があります。
	・getcharid(0)で、自分のcharIDを返すように。
	・getguildmasterid(<n>)追加。
	　<n>=ギルドID
	　該当ギルドのマスターのcharIDを返します。

	(map/)
	guild.c
		guild_mapname2gc() 追加。
	battle.c
		battle_calc_damage() 修正。
	script.c
		buildin_getcharid() 修正。
		buildin_getguildmasterid() 追加。
		ローカルプロトタイプ宣言の一部を修正、追加。
	guild.h 修正。

--------------------
//0864 by 胡蝶蘭

・inter鯖のwisの処理変更
	・自前リンクリストからdb.hで提供されているデータベースを使用するように
	・WISのIDを16ビットから32ビットに増やした（パケットも修正）
	・メッセージのサイズチェックを入れた
	・パケットスキップが二回行われる可能性があるバグ修正
	
	(char/)
	inter.c
		wis関係大幅変更
	(map/)
	intif.c
		wis関係の修正。主にパケット処理。
	(doc/)
	inter_server_packet.txt
		パケット3002,3801を変更

--------------------
//0863 by 死神

・細かい修正。
・battle_athena.confにplayer_attack_direction_change追加。
・mob_skill_db.txtを修正する時挑発の修正を間違って修正。
・モンスターのスキル自爆問題修正。(未テスト)
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(db/)
	mob_skill_db.txt 修正。
	(map/)
	mob.c
		mobskill_use_id()、mobskill_use()、mobskill_castend_id() 修正。
	pc.c
		pc_skill()、pc_attack_timer() 修正。
	skill.c
		skill_castend_damage_id() 修正。
	battle.h 修正。
	battle.c
		battle_weapon_attack()、battle_config_read() 修正。

--------------------
//0862 by 胡蝶蘭

・mobスキル使用条件追加
	・friendhpltmaxrate : 味方のHPが指定％未満のとき(テスト済み)
	・friendstatuson  : 味方が指定したステータス異常になっているとき
	・friendstatusoff : 味方が指定したステータス異常になっていないとき
	・mystatuson  : 自分が指定したステータス異常になっているとき
	・mystatusoff : 自分が指定したステータス異常になっていないとき
	ステータス系は未テストです。mob_skill_db.txtに指定方法を書いています。
	たとえば自分が毒かどうかは mystatus,poison で、
	ハイディング中かどうかは mystatuson,hiding で指定します。
・mobスキル使用ターゲット追加
	・friend : 味方
	・around : 自分の周囲（現在の仕様では周囲81マス）のどれか
	・around1～around4 : 自分の周囲９,25,49,81マスのどれか(範囲を明示)
	friendは条件がfriend系(friendhpltmaxrateなど)のときに使用可能。
	around系は場所指定スキルで使用可能。

	(map/)
	mob.c / mob.h
		mob_getfriend*()追加、mobskill_use()修正など
	(db/)
	mob_skill_db.txt
		最初の説明のみ修正。データは修正していません。

--------------------
//0861 by いど

・サーバーsnapshot

--------------------
//0860 by J

・死神さんの手下召喚の修正に合わせてMOBスキルDBを修正
(/conf)
	mob_skill_db.txt 修正。

--------------------
//0859 by 獅子o^.^o
Alchemist warp 修正(Aegis参考)
(/conf)
    (/warp)
		npc_warp_job.txt 修正
		
--------------------
//0858 by 死神

・細かい修正。
・MAX_MOBSKILLを24から32に変更。(ただ少しですがまたメモリー使用量が増えます。)
・プロボケーションで取る行動をmob_skill_db.txtのval1(値1)で設定できるように修正。
・手下召喚で複数の種類を設定出切るように修正。(最大5つまで)
・メタモルフォーシスとトランスフォーメーションも複数の種類を設定できるように修正。
	(db/)
	skill_db.txt 修正。
	mob_skill_db.txt 修正。
	(map/)
	skill.c
		skill_castend_damage_id()、skill_castend_nodamage_id() 修正。
	map.h 修正。
	mob.h 修正。
	mob.c
		mob_readskilldb()、mob_summonslave()、mob_class_change() 修正。

--------------------
//0857 by J

・OWN Ragnarokにのっていた情報を元にMOBスキルを修正。
・chase(突撃)が実装されているとのことなので突撃(?)をchaseにかえて
コメントアウトをはずしました。
・死神さんが実装したMOBスキルを使用するモンスターを狩場情報に載ってる情報を元に実装。
	(/conf)
		mob_skill_db.txt

--------------------
//0856 by 死神

・バグ修正と細かい修正。
・battle_athena.confにmonster_attack_direction_change追加。
・battle_athena.confのbasic_skill_checkとカプラの倉庫利用を合わせていましたがいつのまにかなくなったので取り戻し。(basic_skill_checkがnoなら基本機能スキルレベルに関係なく倉庫を使えます。)
・ピアーシングアタックの射程を3セルに変更して近接攻撃として認識するように修正。
・A鯖でのテストでアンデッドの認識を属性によってすることがわかったのでundead_detect_typeのデフォルトを0に変更。
・メタモルフォーシスやトランスフォーメーションで見た目がプレイヤーなら0x1b0パケットを送らないように変更。
・ニューマバグは修正してみましたがスキルユニットの時間による作動仕様はまだ分析が完全じゃないので他の不具合が出てくるかも...
	(conf/)
	battle_athena.conf 修正。
	mapflag.txt 修正。(普通のダンジョンがシーズモードであるはずがないので)
	(conf/npc/)
	npc_town_kafra.txt 修正。
	(db/)
	skill_db.txt 修正。
	(doc/)
	conf_ref.txt 修正。
	script_ref.txt 修正。
	(map/)
	pc.c
		pc_modifybuyvalue()、pc_modifysellvalue() 修正。
	battle.h
	battle.c
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_weapon_attack() 修正。
		battle_config_read() 修正。
	skill.c
		skill_unitsetting()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id() 修正。
	mob.c
		mob_attack() 修正。
	pet.c
		pet_attack() 修正。
	clif.c
		mob_class_change() 修正。

--------------------
//0855 by asong

・メタモルフォーシスでＰＣとして表示するＭＯＢを指定した場合倉落ちするバグを「暫定」修正。
・0x1b0パケでは無く0x7bを使うことで何とかしています。
・使い分けをしたいところですが当方Ｃの知識が無く条件分岐が上手くいきませんでした。
・もしかしたらプパの孵化（羽化？）がおかしくなってるかもしれません。
	(/map)
		clif.c
			mob_class_change() 修正。

--------------------
//0854 by Kalen

・不足していた一次職転職クエスト追加及び、それに伴うWarp、Mob修正)
	(/conf)
	   (/npc)
		npc_job_archer.txt
		npc_job_swordman.txt
		npc_job_thief.txt(台詞修正、点数処理変更)
		npc_job_magician.txt
	   (/warp)
		npc_warp25.txt(一部移動)
		npc_warp.txt(一部移動)
		npc_warp_job.txt(新設)
	   (/mob)
		npc_mob_job.txt
・雛祭りQuest追加及び、それに伴うNPC修正。アマツ行き船で料金を取らなかった問題修正
	(/conf)
		npc_event_hinamatsuri.txt
		npc_town_amatsu.txt
		npc_town_guide.txt
		npc_town_kafra.txt
	雛祭りを有効にするとアマツカプラをWに、
	アルベルタ南カプラを削除にするようにしています。

--------------------
//0853 by 死神

・バグ修正とNPCスキル関係の修正。
・ダークブレスをMISC攻撃に変更。(ただ命中判定有り)
・クリティカルスラッシュ、コンボアタック、ガイデッドアタック、スプラッシュアタック、ブラインドアタック、カースアタック、ペトリファイアタック、ポイズンアタック、サイレンスアタック、スリープアタック、スタンアタック、ランダムアタック、ダークネスアタック、ファイアアタック、グラウンドアタック、ホーリーアタック、ポイズンアタック、テレキネスアタック、ウォーターアタック、ウィンドアタック、マジカルアタック、ブラッドドレイン、メンタルブレイカーはモンスターの武器射程に変更。そしてこれらのスキルをモンスターの攻撃射程によって遠距離攻撃と近距離攻撃になるように変更。
・ピアーシングアタックは武器射程+2に変更。
・エナジードレイン、ハルシネーションは魔法射程に変更。
・ダークブレッシングの射程を4に変更とかかる確率を50+スキルレベル*5%に変更。(一応これも魔法なので少し射程を広くしました。基本魔法射程である8に変えるべきなのかどうかは微妙...)
・ガイデッドアタックはセイフティウォールとニューマを無効にする報告がありましたのでセイフティウォールとニューマが効かないように修正。
・ディフェンダーはエフェクトだけ出るように修正。(スキルの仕様等をわかる方は情報をお願いします。)
・トランスフォーメーション実装。(メタモーフォシスと同じ物だそうです。ただこれは全然関係ない別のモンスターになる物らしいです。ニフルヘイムに使うやつがいるみたいです。)
・Athena雑談スレッド 其の弐の80をscript_ref.txtとして追加とちょっと修正。
	(db/)
	skill_db.txt 修正。
	(doc/)
	script_ref.txt 追加。
	(map/)
	battle.c
		battle_calc_damage()、battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_calc_magic_attack()、battle_calc_misc_attack() 修正。
	skill.c
		castend_damage_id()、castend_nodamage_id()、skill_use_pos() 修正。
	clif.c
		clif_spawnnpc()、clif_parse_Restart()、clif_parse_QuitGame() 修正。
	mob.c
		mobskill_castend_id()、mobskill_castend_pos() 修正。
		mobskill_use_id()、mobskill_use_pos() 修正。

--------------------
//0852 by ぴざまん

・亀島4F・蟻D2F・アマツD1Fをテレポ不可、シーズモードに変更。
・nosaveの引数にSavePointが指定できてなかったので追加。
・PVPのmapflagをmapflag.txtに統合。
	(map/)
	npc.c
		npc_parse_mapflag() 修正。
	(conf/)
		mapflag.txt 修正。
		npc/npc_pvp.txt 修正。

--------------------
//0851 by 胡蝶蘭

・ログイン時の暗号化keyが常に同じという大きな問題があったので修正
・ログイン管理者ログイン(ladminで使用)でパスワードの暗号化に対応
	(login/)
	login.c
		login_session_data作成、暗号化keyをクライアントごとに作成など
	(tool/)
	ladmin
		ver.1.05に。デフォルトでパスワードを暗号化するように。
		暗号化のためにDigest::MD5モジュールを使用します。
		Digest::MD5が無い場合はパスワードの暗号化を行いません。
	(doc/)
	admin_packet.txt
		ログインサーバー管理ログイン部分変更

--------------------
//0850 by 死神

・NPCスキル実装。(ハルシネーション、キーピング、リック、メンタルブレイカー、プロボケーション、バリヤー、ダークブレッシング、ダークブレス)
・スキル自爆の制限はmob_skill_db.txtでやればいいものなので取り戻し。
・battle_athena.confにpet_hungry_friendly_decrease追加。
・ペットの腹が完全に減ると支援攻撃を中止するように変更。
・属性変更スキルが作動しなかった問題修正。
・メンタルブレイカーは10+スキルレベル*5%のSPを減らす。(攻撃は通常武器スキル攻撃)
・リックは必中でSP-100、スタン確率スキルレベル*5%。(ダメージは無し、bNoWeaponDamageで無効)
・プロボケーションはモーションが準備されてないモンスターは入れてもなんの効果もなし。
・ダークブレッシングはかかるとHPが1になる。耐性は魔法防御で適用。
・ダークブレスは500+(スキルレベル-1)*1000+rand(0,1000)のダメージ。回避できるが防御無視で近距離物理攻撃だがセイフティウォールは無視して闇属性攻撃。(本鯖の計算式にあっている可能性はないかも。ただダメージ量と命中補正以外は本鯖合わせ)
・NPCスキルの維持時間は適度に設定。
・モンスターの属性攻撃とガイデッドアタックがセイフティウォールを無視するとの報告を受けたのですが修正するかどうかはちょっと微妙。(スプラッシュアタックもセイフティウォール無視かも)
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(db/)
	cast_db.txt 修正。
	skill_db.txt 修正。
	(map/)
	mob.c
		mob_damage() 修正。
	clif.h 修正。
	clif.c
		clif_skill_estimation()、clif_damage()、clif_skill_damage() 修正。
		clif_skill_damage2()、clif_pet_performance() 修正。
	pet.c
		pet_performance()、pet_target_check()、pet_hungry() 修正。
	skill.h 修正。
	skill.c
		skill_additional_effect()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_status_change_start() 修正。
	battle.h 修正。
	battle.c
		battle_get_def()、battle_get_mdef()、battle_calc_damage() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_config_read() 修正。

--------------------
//0849 by lapis

・街中のテストギルドフラグの表示がおかしかったのを修正。
・ギルドメンバーは旗からアジトに飛べるように修正。
	(conf/gvg)
		TEST_prtg_cas01_AbraiJ.txt 修正。

--------------------
//0848 by huge

・スキル自爆を、HPが全回復している時は使えないよう修正。
・スフィアマイン・バイオプラント・・・ターゲット変更できない...。
・mobにターゲット無視IDを設定できるようにしました。(Gv用mobに使える？)
	(map/)
	map.h 修正。
		mob_dataに int exclusion_src,exclusion_party,exclusion_guild 追加。
	mob.h 修正。
	mob.c
		mob_exclusion_add() 追加。
		mob_exclusion_check() 追加。
		mob_timer_delete() 追加。
		mob_attack() 修正。
		mob_target() 修正。
		mob_ai_sub_hard_activesearch() 修正。
		mob_ai_sub_hard_mastersearch() 修正。
		mob_ai_sub_hard() 修正。
	skill.c
		skill_castend_damage_id() 修正。
		skill_castend_pos2() 修正。

--------------------
//0847 by 死神

・露店バグ修正。
	(map/)
	clif.c
		clif_vendinglist()、clif_openvending() 修正。
	vending.c
		vending_openvending() 修正。
	skill.c
		skill_castend_nodamage_id() 修正。

--------------------
//0846 by 死神

・バグ修正と細かい修正。
・battle_athena.confのenemy_strがペットにも適用するように変更。
・bHPDrainRateとbSPDrainRateでxがマイナスでも作動するように変更。
・PCやNPCの姿をしたモンスターも死ぬと5秒後マップから消えるように変更。
	(map/)
	battle.c
		battle_calc_pet_weapon_attack()、battle_weapon_attack() 修正。
	skill.c
		skill_attack()、skill_castend_damage_id() 修正。
	pc.c
		pc_allskillup() 修正。
	clif.h 修正。
	clif.c
		clif_openvending()、do_init_clif() 修正。
		clif_clearchar_delay()、clif_clearchar_delay_sub() 追加。
	mob.c
		mob_damage() 修正。

--------------------
//0845 by ぽぽぽ

・mob_avail.txtでPCグラフィック(0～23)を指定したペットが出現したときクライアントエラーがでるのを暫定修正。
・mob_avail.txtでペットにもPCキャラの性別・髪型&色・武器・盾・頭装備を指定できるようにしました。
・MOBのATK計算にSTRを適用するかどうか設定可能にした。
	(map/)
	clif.c
		clif_pet0078()、clif_pet007b()、clif_spawnpet()修正。
	battle.h修正。
	battle.c
		battle_config_read()、battle_calc_mob_weapon_attack()修正。

--------------------
//0844 by ぽぽぽ

・mob_avail.txtでPCグラフィック(0～23)を指定したMOBが出現したときクライアントエラーがでるのを暫定修正。
・mob_avail.txtでPCキャラの性別・髪型&色・武器・盾・頭装備を指定できるようにしました。
　グラフィックすり替え先IDが0～23の時だけ有効で、指定方法は
	MOB-ID,グラフィックすり替え先ID,性別(0=female,1=male),髪型,髪色,武器,盾,上段頭装備,中段頭装備,下段頭装備
　となります。装備はitem_dbのView欄参照のこと。
	(map/)
	clif.c
		clif_mob_0078()、clif_mob007b()、clif_spawnmob()修正。
	mob.h修正。
	mob.c
		mob_get_sex()、mob_get_hair()、mob_get_hair_color()、ob_get_weapon()、
		mob_get_shield()、mob_get_head_top()、mob_get_head_mid()、mob_get_head_buttom()追加。
		mob_readdb()、mob_readdb_mobavail()修正。
		
--------------------
//0843 by 死神

・リフレクトシールド実装。
・アイテムスクリプトにbShortWeaponDamageReturnと
bLongWeaponDamageReturn 追加。
・その他スキル関係や他の所修正。
	(db/)
	item_db.txt 修正。
	skill_db.txt 修正。
	cast_db.txt 修正。
	const.txt 修正。
	(doc/)
	item_bonus.txt 修正。
	(map/)
	map.h 修正。
	battle.c
		battle_get_def()、battle_get_def2()、battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_weapon_attack()、battle_calc_magic_attack() 修正。
	pc.c
		pc_calcstatus()、pc_bonus()、pc_bonus2()、pc_equipitem() 修正。
		pc_unequipitem()、pc_checkallowskill() 修正。
	skill.c
		skill_attack()、skill_unit_onplace()、skill_status_change_start() 修正。
		skill_status_change_end()、skill_status_change_timer() 修正。
		skill_castend_nodamage_id() 修正。
	clif.c
		clif_additem()、clif_equiplist()、clif_storageequiplist() 修正。
		clif_tradeadditem()、clif_storageitemadded()、clif_cart_additem() 修正。
		clif_cart_equiplist()、clif_vendinglist()、clif_openvending() 修正。
		clif_damage()、clif_skill_damage()、clif_parse_LoadEndAck() 修正。

--------------------
//0842 by 死神

・スキル関係の修正と細かい修正。
・aegis鯖で色々と検証した物を適用。
・メテオの範囲を7*7、LoV13*13、SG11*11、FN5*5に修正。
・シグナム実装。(ただPVPでプレイヤーにかかるかどうかがわからなかったので
かかる方向で実装。)これで1次職業のスキルはクリアかも...
・装備スクリプトにbHPDrainRateとbSPDrainRate追加。
・その他細かい修正少し。
	(doc/)
	item_bonus.txt 修正。
	(db/)
	cast_db.txt 修正。
	item_db.txt 修正。
	const.txt 修正。
	(map/)
	map.h 修正。
	skill.c
		skill_castend_damage_id()、skill_castend_nodamage_id() 修正。
		skill_unitsetting()、skill_castend_pos2()、skill_castend_id() 修正。
		skill_status_change_start()、skill_status_change_timer() 修正。
		skill_status_change_end()、skill_unit_onplace() 修正。
		skill_frostjoke_scream()、skill_attack() 修正。
		skill_attack_area() 追加。
	battle.c
		battle_calc_magic_attack()、battle_get_element()、battle_get_def() 修正。
		battle_get_def2()、battle_get_mdef()、battle_damage() 修正。
		battle_calc_damage()、battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_weapon_attack() 修正。
	mob.c
		mobskill_castend_id() 修正。
	pc.c
		pc_calcstatus()、pc_bonus2()、pc_attack_timer() 修正。
	clif.c
		clif_spawnmob()、clif_spawnpet()、clif_spawnnpc() 修正。
		clif_parse_ActionRequest() 修正。

--------------------
//0841 by Kalen

・水溶液が作れなかったので追加
	conf/npc/npc_job_magician.txt

--------------------
//0840 by Kalen

・鬼イベント追加
	conf/npc/npc_event_oni.txt

・map_athena.conf修正(バレンタインコメントアウト。鬼追加)
	conf/map_athena.conf

--------------------
//0839 by shuto

・コンロンNPC追加(カン ソンソンが抜けてた)

--------------------
//0838 by 死神

・スキルサイトラッシャー実装。
・モンスターのクローキングとマキシマイズパワーは持続時間をレベル*5秒に変更。
・その他細かいバグ修正。
	(db/)
	skill_db.txt 修正。
	(map/)
	skill.c
		skill_castend_damage_id()、skill_castend_nodamage_id() 修正。
		skill_castend_pos2()、skill_unitsetting()、skill_get_unit_id() 修正。
		skill_status_change_start() 修正。
	battle.c
		battle_calc_magic_attack() 修正。

--------------------
//0837 by 死神

・スキル関係の細かい修正。
・フロストノヴァをユニット設置式に変更。
・ロードオブヴァーミリオンの範囲を11*11に修正と40ヒットするように変更。(ラグナゲートの情報。
13*13説もありますが...)
・ユピテルサンダーのノックバックを2~7に変更。
・ストームガストの攻撃回数をレベル依存から10回に固定。
・サンクチュアリのノックバックを3から2に変更。(aegis鯖でノックバックがあることは確認しましたがどれぐらいなのかが不明だったので少し減らしてみました。)
・モンスターの詠唱時間が早くなっていた問題修正。(dex補正が入ってしまったせいです。)
・その他オートスペル当たりの細かい修正。
	(db/)
	skill_db.txt 修正。
	(map/)
	skill.c
		skill_castfix()、skill_delayfix()、skill_timerskill() 修正。
		skill_castend_pos2()、skill_unitsetting()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_get_unit_id()、skill_attack() 修正。
	battle.c
		battle_calc_magic_attack()、battle_weapon_attack() 修正。

--------------------
//0836 by 釈尊

・モンスターの取り巻き召還の間隔を中ボス以外完全修正。
	(db/)
	mob_skill_db.txt 修正。

--------------------
//0835 by (凸)

・白刃取りでポーズをとるようにした。
(ポーズだけなので、実際に攻撃を受け止めたりはできません)
clif_bladestop()を呼ぶことで白刃取り状態のON、OFFのパケットが送れます。

	(map/)
	clif.h 修正。
	clif,c
		clif_bladestop() 追加
	skill.c
		skill_castend_nodamage_id() 修正。

--------------------
//0834 by 釈尊

・モンスター取り巻き召還の間隔が短すぎるとの事で応急処置。(今回は黄金蟲のみ)
	(db/)
	mob_skill_db.txt 修正。

--------------------
//0833 by (凸)

・memo禁止地域で/memo時の修正。
・ついでにitem_dbを更新。

	(doc/)
	client_packet.txt
		R 0189 更新。
	(db/)
	item_db.txt 最新版へ更新。
	(map/)
	pc.c
		pc_memo() 修正。

--------------------
//0832 by 死神

・コード最適化と細かい修正。
・オートスペルを地面魔法に対応。
・サンダーストームとヘヴンズドライブをユニット設置式に変更。
・ディフェンダーの攻撃速度低下を本鯖にあわせ。
・その他細かい修正。
	(doc/)
	item_bonus.txt 修正。
	(db/)
	skill_require_db.txt 修正。
	cast_db.txt 修正。
	(map/)
	map.h 修正。
	path.c
		calc_index()、path_search() 修正。
	skill.c
		skill_unitsetting()、skill_castend_pos2()、skill_get_unit_id() 修正。
		skill_status_change_timer_sub()、skill_castend_nodamage_id() 修正。
		skill_additional_effect()、skill_frostjoke_scream() 修正。
	pc.c
		pc_calcstatus()、pc_skill()、pc_allskillup() 修正。
	battle.c
		battle_get_speed()、battle_get_adelay()、battle_get_amotion() 修正。
		battle_weapon_attack() 修正。

--------------------
//0831 by 死神

・少し修正。
・オートスペル修正。装備による物とスキルによる物を別々に適用、発動確率修正。
・装備によるオートスペルは指定したレベルより2つ下まで判定をします。つまりレベル5を設定するとレベル3から5まで発動します。
・battle_athana.confのplayer_cloak_check_wall、monster_cloak_check_wallをplayer_cloak_check_type、monster_cloak_check_typeに変更。
・アイテムルート権限時間を本鯖に合わせて修正。
・その他スキル関係の細かい修正。
	(doc/)
	conf_ref.txt 修正。
	db_ref.txt 修正。
	item_bonus.txt 修正。
	(conf/)
	battle_athana.conf 修正。
	(db/)
	item_db.txt 修正。
	(map/)
	map.h
	map.c
		block_free_max、BL_LIST_MAX 修正。
	skill.h 修正。
	skill.c
		skill_additional_effect()、skill_attack()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_unit_onplace() 修正。
		skill_status_change_end()、skill_status_change_start() 修正。
		skill_initunitgroup()、skill_unitsetting()、skill_castfix() 修正。
		skill_delayfix()、skill_autospell()、skill_use_id()、skill_use_pos() 修正。
		skill_check_cloaking()、skill_unit_timer_sub()、skill_check_condition() 修正。
	battle.h 修正。
	battle.c
		battle_damage()、battle_get_agi()、battle_get_speed() 修正。
		battle_get_adelay()、battle_get_amotion()、battle_get_flee() 修正。
		battle_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_config_read() 修正。
	mob.c
		mob_attack()、mob_damage()、mobskill_use_id() 修正。
		mobskill_use_pos()、mob_spawn()、mob_class_change() 修正。
		mob_can_move() 修正。
	pc.c
		pc_attack_timer()、pc_checkweighticon()、pc_calcstatus() 修正。
		pc_damage()、pc_equipitem()、pc_unequipitem() 修正。
		pc_bonus2()、pc_bonus3()、pc_memo()、pc_authok() 修正。
		pc_isUseitem() 修正。
	clif.h 修正。
	clif.c
		clif_changeoption()、clif_parse_LoadEndAck()、clif_autospell() 修正。
		clif_skill_memo() 修正。
		clif_skill_teleportmessage() 追加。
	script.c
		buildin_sc_start() 修正。
	atcommnad.c
		atcommand() 修正。

--------------------
//0830 by huge

・オートスペルで、自分の習得してるレベルよりも高いレベルで
  魔法が発動していたであろう問題を修正。
・オートスペルでも、ちゃんとSPが減るように修正(ど忘れ)
	(map/)
	battle.c
		battle_weapon_attack() 修正。
	skill.c
		skill_autospell() 修正。

--------------------
//0829 by Kalen

・mob_db修正
	自鯖用のと間違ってUPしてしまったようです。
	本来のものに直しました。

--------------------
//0828 by 聖

・サーバのSnapshot
・MVPボス系からバカンスチケットが大量に出ていた問題を修正。
	(common/)
	version.h 修正。
	(db/)
	mob_db.txt 修正。

--------------------
//0827 by J

・固定MOBのはずのメガリスと人面桃樹が歩いていたのを修正。
・闘技場のMOBにスキルを実装。
・パサナとファラオがスキルが設定されていなかったのを実装。
	(db/)
	mob_db.txt 修正。
	mob_skill_db.txt 修正。	

--------------------
//0826 by ぴざまん

・一部の演奏スキルを使うと鯖が落ちるバグ修正。
・クローキング中にスキルを使用でき、
　使用するとクローキングが解除されるように修正。
・Kalenさんのmob_db.txtをまとめました。
	(map/)
	skill.c
		skill_unit_onplace()、skill_unit_onout() 修正。
		skill_status_change_start()、skill_status_change_timer() 修正。
		skill_use_id()、skill_use_pos() 修正。
	(db/)
	mob_db.txt 修正。

--------------------
//0825 by 死神

・細かい修正。(細かいことのわりには修正した所が多いけど...)
・ワープポータルの中に止まった時以外はワープしないように修正。
・battle_athena.confにplayer_skill_nofootset、monster_skill_nofootset 追加。
・NPCをクリックした後露店をクリックして露店を閉じると動けなくなるバグ修正。
ただ露店を閉じる時何のパケットも転送してこないので露店をクリックするとNPCの処理から抜けるようにしました。(本鯖ではNPCの処理が抜けないらしいですがそれ以外方法がなかったのっで。)
・killmonsterのAllで召喚されたモンスターだけ消すように変更。
・ソース最適化やスキル関係の細かい修正多数。
	(doc/)
	conf_ref.txt 修正。
	db_ref.txt 修正。
	(conf/)
	battle_athana.conf 修正。
	atcommnad_athena.conf 修正。
	(db/)
	item_db.txt 修正。
	skill_db.txt 修正。
	(login/)
	parse_fromchar() 修正。
	(map/)
	map.h 修正。
	clif.c
		clif_closevendingboard()、clif_parse_VendingListReq() 修正。
		clif_mob0078()、clif_mob007b()、clif_pet0078()、clif_pet007b() 修正。
	skill.h 修正。
	skill.c
		skill_check_condition()、skill_castend_pos2() 修正。
		skill_castend_damage_id()、skill_castend_nodamage_id() 修正。
		skill_castend_id()、skill_status_change_start()、skill_castfix() 修正。
		skill_delayfix()、skill_check_unit_range_sub() 修正。
		skill_check_unit_range()、skill_castend_pos()、skill_stop_dancing() 修正。
		skill_unit_onplace()、skill_readdb()、skill_timerskill()、skill_blown() 修正。
		skill_check_unit_range2_sub()、skill_check_unit_range2() 追加。
		skill_get_maxcount() 追加。
	mob.c
		mobskill_castend_id()、mobskill_castend_pos()、mob_deleteslave() 修正。
		mob_stop_walking()、mob_walk()、mob_damage() 修正。
	pc.c
		pc_calcstatus()、pc_checkskill()、pc_stop_walking() 修正。
		pc_walk()、pc_damage() 修正。
	npc.c
		npc_touch_areanpc() 修正。
	pet.c
		pet_stop_walking() 修正。
	script.c
		buildin_killmonster()、buildin_killmonster_sub() 修正。
	battle.h
	battle.c
		battle_calc_magic_attack()、battle_get_flee()、battle_get_flee2() 修正。
		battle_get_adelay()、battle_get_amotion()、battle_get_max_hp() 修正。
		battle_get_hit()、battle_get_critical()、battle_get_atk2() 修正。
		battle_damage()、battle_config_read() 修正。
	atcommand.h 修正。
	atcommand.c 修正。

--------------------
//0824 by ぴざまん

・セイフティウォール・ニューマの足元置きができなかった問題修正。
・エンペリウムにヒール等の支援スキルが効いていた問題修正。
・闘技場でモンスターリセットができなかった問題修正。
　killmonsterは"killmonster <mapname>,<eventname>"と記述して
　該当eventnameを持つモンスターを削除しますが
　eventnameにAllと入れると該当MAPの全モンスターを消去するようにしました。

	(map/)
	skill.c
		skill_check_unit_range_sub()、skill_castend_nodamage_id() 修正。
	script.c
		buildin_killmonster()、buildin_killmonster_sub() 修正。
	(conf/npc/)
	npc_event_tougijou.txt 修正。

--------------------
//0823 by Kalen

・闘技場データ揃ったので、完成
	conf/npc/npc_event_tougijou.txt
	ただし、こちらでチェックしたところkillmonsterがうまくいかず、
	失敗、時間切れした場合モンスターリセットが出来ません。
	イベントが設定されているモンスターは処理できないのかと思いましたが
	AgitのほうのエンペのKillmonsterはちゃんと動いてますし…
	原因分かる方お願いします<(_ _)>

・gon_testのmapflag追加
	conf/mapflag.txt

・mob_db更新
	1419～1491が既存のMobの定義ばかりなので追加しませんでしたが、
	調べた所闘技場のMobのデータであることが分かりました(Dropを弄ったもの)
	本鯖では闇りんごが報告されています。が、こちらはDrop空白で処理しました。
	クライアント上では同名でしたが、区別のため接頭にG_をつけて区別してます。
	mob_skill_dbありがとうございました↓

--------------------
//0822 by ぴざまん

・演奏スキルでの補正をダンサーにも適用。
・サンクチュアリバグ修正。
・KalenさんのMOBスキルデータベースをまとめときました。

	(map/)
	skill.c
		skill_status_change_start()、skill_unit_onplace() 修正。
	battle.c
		battle_get_critical()、battle_get_hit() 修正。
	pc.c
		pc_calcstatus() 修正。

--------------------
//0821 by huge

・オートスペル仮実装。
・timerで判定しようかとも思いましたが、装備の無限オートスペルの為にsc_[].val1で見てマス。
・bonus2 bAutoSpell追加。一応どんなスキルでも指定できるようにしてますが(番号はskill_tree参照)
  skill_castend_damage_idのタイプ以外のスキルを指定しないでください。
  発動確率は、Lv1:50%、Lv2:35%、Lv3:15%、それ以上は 5%固定です。
  あと、スキルレベルも指定できますが、限界を超えた数字を入れると墜ちるかもしれません。
■書き方例：（ファイアボルトLv3の時）bonus2 bAutoSpell 19,3;

	(db/)
	const.txt 修正。
	(map/)
	battle.c
		battle_weapon_attack() 修正。
	clif.h
	clif.c
		packet_len_table 修正。
		clif_autospell() 追加。
		clif_parse_AutoSpell() 追加。
	map.h 修正。
	pc.c
		pc_bonus2() 修正。
		pc_equipitem() 修正。
		pc_unequipitem() 修正。
	skill.h
	skill.c
		skill_castend_nodamage_id() 修正。
		skill_autospell() 追加。
		skill_status_change_end() 修正。
		skill_status_change_start() 修正。
		status_changeの番号テーブル修正。

--------------------
//0820 by ぴざまん

・アドリブのメッセージが入ってなかったので修正
・バードの演奏スキルで楽器の練習や自ステータスの補正が入ってなかったのを修正。
　struct status_changeのvalが3つ必要だったので(val4は予約されてたっぽいので)val5を追加しました
	(map/)
	map.h 修正。
	skill.c
		skill_status_change_start()、skill_castend_nodamage_id() 修正。
		skill_castfix()、skill_delayfix() 修正。
	battle.c
		battle_get_flee()、battle_get_max_hp() 修正。
		battle_get_adelay()、battle_get_amotion() 修正。
		battle_calc_misc_attack() 修正。
	pc.c
		pc_calcstatus() 修正。

--------------------
//0819 by Kalen

・コンロン(NPC、Warp)修正
	conf/npc/npc_town_gonryun.txt(案内員補充)
	conf/npc/npc_event_tougijou.txt
	conf/warp/npc_warp_gonryun.txt(宿2FとD2Fなど)

・MOB修正
	conf/mob/npc_monster30.txt(一反木綿不足追加)
	conf/mob/npc_monster35.txt(コンロンMob追加)

・DB修正
	db/mob_db.txt(コンロン[全て]+ウンバラ[定義]追加。Aspeed等適当です。まぁ無いよりましということで)
	db/mob_skill_db.txt(情報を元にコンロンのMob分追加)
	db/item_db.txt(Athena DB Project 2/19 21:10DL分)

--------------------
//0818 by あゆみ

・テレポートスキルLv1で、選択ウインドウが出てこないバグを修正。
・重量が90％以上の場合でも、一部のスキルが使用可能だったバグを修正。
・@allskillコマンドの修正とか。

	(conf/)
	msg_athena.conf 修正。
	(map/)
	atcommand.c
		atcommand() 修正。
	pc.c
		pc_allskillup() 修正。
	skill.c
		skill_castend_nodamage_id() 修正。
		skill_check_condition() 修正。

--------------------
//0817 by huge

・ディボーションの処理修正
  ・糸の出し方はパケを貰ったのでできましたが、アイコンの方はまだ分からないです。
  ・あと、自分の環境で2人以上に同時に掛けれなかったので、複数人にかけた場合
    多分0の羅列の所に2人目、3人目・・・のIDが入るんじゃないかなぁという予測でやってます。
・ハイディング中、及びクローキング中にダメージを受けると解けるよう修正。

	(map/)
	battle.c
		battle_damage() 修正。
	clif.c
		clif_devotion() 修正。
	pc.c
		pc_walk() 修正。
	skill.c
	skill.h
		skill_castend_nodamage_id() 修正。
		skill_devotion() skill_devotion2() 修正。
		skill_devotion3()skill_devotion_end() 修正。

--------------------
//0816 by ぴざまん
・ファイアーウォール3枚制限実装。
・重ね置き禁止をプレイヤー・モンスターにも適用するように修正。
・寒いジョーク・スクリームのPvP・GvGで、効果が自分にも及ぶバグ修正。
　ついでにPTメンバーには低確率でかかるのも実装。
・寒いジョーク・スクリーム・スピアブーメランのディレイ修正。
　ミュージカルストライク・矢撃ちの詠唱時間修正。
	(map/)
	skill.c
		skill_check_condition()、skill_check_unit_range_sub() 修正。
		skill_check_unit_range()、skill_delunitgroup() 修正。
		skill_castend_pos2()、skill_frostjoke_scream() 修正。
	map.h 修正。
	(db/)
	cast_db.txt 修正。

--------------------
//0815 by 死神

・0814のバグ修正と細かい修正。
・mapflag monster_noteleport、noreturn追加とnoteleportの仕様変更。
noteleportはプレイヤーのハエとテレポート、ワープスキルの制限をするが蝶は制限しないように変更、monster_noteleportはモンスターのテレポートを制限する物でnoreturnは蝶の使用を制限する物です。ただmapflag.txtは修正していません。(noreturnを設定する必要があります。)
・battle_athena.confのplayer_auto_counter_typeとmonster_auto_counter_typeが説明通りに機能しなかった問題修正。
・battle_athena.confにplayer_cloak_check_wall とmonster_cloak_check_wall 追加。
・ボスモンスターの認識をMVP経験とmodeの0x20で行なっていた物をmodeだけにするように変更。(本鯖のイベントモンスターでMVP経験をくれるが状態異常に掛かるやつがあったらしく修正。HPが1億もあって毒じゃないと倒せなかったらしいので...) つまりMVP経験があってもボス扱いではないモンスターを作ることも可能です。
・状態異常に掛かった状態で接続切断ができないように修正。(ただタイマーチェックではなくopt1とopt2をチェックするだけなので見た目が変わる状態異常だけに適用されます。)
・今さらですが昔のyareCVS(2003年9月バージョン)で適用されていたラグを減らす為の処理を入れてみました。どんな効果があるかは自分でもわかりません。(ただ入れてみただけ...)
・シーズモードとPVPで禁止装備が外されても効果が消えないバグ修正。
・その他細かい修正。
・未テストの物もかなりあります。
	(common/)
	socket.c
		connect_client()、make_listen_port()、make_connection() 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_calc_magic_attack()、battle_config_read() 修正。
	skill.c
		skill_unit_onplace()、skill_status_change_timer() 修正。
		skill_castend_nodamage_id()、skill_use_id() 修正。
		skill_check_unit_range_sub()、skill_timerskill() 修正。
		skill_additional_effect()、skill_attack()、skill_status_change_start() 修正。
		skill_check_cloaking() 修正。
	clif.c
		clif_item_identify_list()、clif_parse_QuitGame()、clif_GM_kick() 修正。
	pc.c
		pc_attack_timer()、pc_isUseitem()、pc_checkitem 修正。
	mob.c
		mob_warp()、mob_walk()、mob_attack()、mob_target() 修正。
		mob_ai_sub_hard_activesearch()、mob_ai_sub_hard_mastersearch() 修正。
		mob_ai_sub_lazy()、mob_damage() 修正。
	npc.c
		npc_parse_mapflag() 修正。
	map.h 修正。

--------------------
//0814 by 死神

・バグ修正と細かい修正。
・battle_athena.confのplayer_auto_counter_typeとmonster_auto_counter_typeの仕様を変更。(本鯖ではスキル反撃はできないみたいなので設定できるように変更。)
・毒と石化によるHP減少を本鯖に合わせて修正と完全石化の前では動けるように変更。(毒は1秒に3+最大HPの1.5%(モンスターは0.5%)、石化は5秒に1%) 未テスト
・MVP経験値は本鯖でいつも入るようになったので修正。
・スティールの確率を少し下げ。
・モンスターのハイディング、クローキング、マキシマイズパワーがすぐに解除される問題修正。(モンスターにはSPがないせいです。取り敢えずクローキングはハイディングの時間を適用してマキシマイズパワーはウエポンパーフェクションの時間を適用します。) 未テスト
・サンクチュアリを人数から回数に変更。
・PVPで自分のトラップに攻撃対象になるように変更。
・vitペナルティの適用で乗算防御も減るように変更。(未テスト)
・その他細かいバグ修正。
	(conf/)
	battle_athena.conf
	(doc/)
	conf_ref.txt
	(db/)
	skill_db.txt
	(map/)
	map.h 修正。
	script.c
		buildin_itemskill() 修正。
	mob.c
		mob_can_move()、mob_ai_sub_hard()、mob_damage() 修正。
	skill.c
		skill_unitsetting()、skill_unit_onplace()、skill_castend_nodamage_id() 修正。
		skill_attack()、skill_status_change_start() 修正。
		skill_status_change_timer()、skill_status_change_timer_sub() 修正。
		skill_addtimerskill()、skill_cleartimerskill() 修正。
		skill_check_unit_range_sub() 修正。
	battle.c
		battle_calc_damage()、battle_check_target() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack() 修正。
	pc.c
		pc_steal_item() 修正。
	clif.c
		clif_parse_WalkToXY()、clif_closevendingboard() 修正。

--------------------
//0813 by Kalen
・コンロンNPC追加
	conf/npc/npc_town_gonryun.txt(クエストは不明
	conf/npc/npc_town_kafra.txt
	conf/npc/npc_town_guide.txt
	(conf/npc/npc_event_tougijou.txt)データ不足

・全GvGMAPのMobデータ追加
	conf/mob/npc_monster_gvg.txt

・TESTmobからテストギルドフラグ移動
	conf/gvg/TEST_prtg_cas01_AbraiJ.txt

--------------------
//0812 by huge

・ディボーションの仮実装
  ・パケットが全然分かりませんでしたので、
    青い糸も出ないし、アイコンすら表示できません。
  ・ダメ移動だけで、ノックバック・オートガードは適応されません。

	(map/)
	clif.c
	clif.h
		clif_devotion() 追加。
	skill.c
		skill_castend_nodamage_id() 修正。
		skill_devotion() skill_devotion2() 追加。
		skill_devotion3() skill_devotion_end() 追加。
		skill_status_change_end() 修正。
		skill_status_change_start() 修正。
		skill_brandishspear_first() 修正。
		skill_brandishspear_dir() 修正。
	pc.c
		pc_authok() 修正。
		pc_walk() 修正。
		pc_damage() 修正。
	battle.c
		battle_damage() 修正。
	map.h
		map_session_data{}
			 struct square dev 追加。
	skill.h のbrandishをsquareに改名し、
	(common/) mmo.h に移動

--------------------
//0811 by ぴざまん

・攻城中は蝶が使えるように修正
・寒いジョーク・スクリーム実装(PTメンバーに低確率で云々は未実装です)
・GVGスクリプトを修正(試行錯誤しすぎてどこをどうしたか覚えてません…)
　GVGスクリプトに関してですが、既知のバグがあります
　それは、占領ギルドマスター以外のPCが、占領ギルドマスターより先にNPCに話し掛けると
　マップサーバが落ちるというものです。
　これは、getguildmaster・getguildnameを使用しているスクリプトすべてに起こり得ることであり
　先に占領ギルドマスター以外で話し掛けると、guild_searchが何故か(該当IDのギルドがあるにも関わらず)
　NULLを返す事に起因します。
　正直スクリプト関係はよくわかっていないので、これは私の技術では修正のしようがありません。
　暫定的な対処としてNULLを返してMAP鯖が落ちるくらいなら文字列「null」を返すようにしました。

	(map/)
	skill.c
		skill_frostjoke_scream() 追加。
		skill_additional_effect()、skill_timerskill() 修正。
		skill_castend_nodamage_id() 修正。
	pc.c
		pc_isUseitem() 修正。
	script.c
		buildin_getpartyname()、buildin_getguildname() 修正。
		buildin_getguildmaster() 修正。
	(db/)
	cast_db.txt 修正。
	(conf/gvg/)
	ev_*.txt以外のtxt全てを修正。

--------------------
//0810 by 聖

・MVPの処理を変更。(確率で10000があっても、他のアイテムも出るようにしました)
・MVPで装備を入手した場合、鑑定済みで入手していた問題を修正。
・スキル詠唱中にイグ葉や拡大鏡を使うとプレーヤーの使用可能な
　全スキルのLVが1に固定されてしまう問題を修正。
・オークアーチャー等、モンスターによる罠スキルの有効期限が切れたときに、
　設置用トラップが出る問題を修正。
・ログインエラー(パスワード入力ミスやBAN等)のエラーメッセージが
　正しくクライアントに通知されない問題を修正。
・その他細かな修正。
	(common/)
	version.h 修正。
	(login/)
	login.c
		parse_login() 修正。
		parse_fromchar() 修正。
	(char/)
	int_guild.c
		mapif_parse_GuildLeave() 修正。
	(map/)
	itemdb.c
		itemdb_read_itemnametable() 修正。
	atcommand.c
		atcommand() 修正。
	skill.c
		skill_unit_timer_sub() 修正。
	script.c
		buildin_itemskill() 修正。
	mob.c
		mob_damage() 修正。

--------------------
//0809 by Kalen

・東湖城ワープポイント修正
・アマツ寿司屋修正
・バグスレ129の問題修正？


--------------------
//0808 by ぴざまん

・装備制限が上手く動作していなかったのを修正。
・モンハウギルドはGvG開始時に作られるように変更。
・GvG開始時に該当マップにいる全PC(占領ギルド員以外)をセーブポイントに戻すように修正。
・モンハウギルドアジトでエンペを壊すと、モンハウが消えるように修正。
　この修正に伴ってmaprespawnguildidの引数のflagの仕様を変更しました
　flagはビットフラグになり、
　　1ビット目：占領ギルド員をセーブポイントに戻すか
　　2ビット目：占領ギルド員以外をセーブポイントに戻すか
　　3ビット目：エンペ・ガーディアン以外のMOBを消すか
　いずれも、0=NO、1=YESになります

	(conf/gvg/)
	ev_agit_aldeg.txt 修正。
	ev_agit_gefg.txt 修正。
	ev_agit_payg.txt 修正。
	ev_agit_prtg.txt 修正。
	TEST_prtg_cas01_AbraiJ.txt 修正。
	TEST_prtg_cas01_mob.txt 修正。
	(map/)
	pc.c
		pc_checkitem() 修正。
	script.c
		buildin_maprespawnguildid_sub() 修正。
		buildin_maprespawnguildid() 修正。

--------------------
//0807 by 死神

・0805でFD_SETSIZEを修正する所を間違ったので修正しました。56名止まりが治ると言う保証はありませんが...
・一度に転送するパケットの長さを32768bytesから65536bytesに変更。
	(common/)
	mmo.h 修正。
	socket.h 修正。
	socket.c 修正。

--------------------
//0806 by Kalen

・agitフォルダ→gvgフォルダへ移行
	諸意見あると思いますが、jROでは攻城戦をgvgと呼ぶことが一般的なのでこちらに統合します。
	conf/gvg/
	###agitフォルダを削除してください###　(Please delete the "agit" folder.)
	getmaster対応

・アマツの寿司屋バグ修正と項目追加(thx 114
	conf/npc/npc_town_amatsu.txt

・map_flag再修正
	conf/map_flag.txt
	[GVGMAP]確かに常にシーズモードなら問題ないですが、削除されましたので
	常にシーズモードではありません。従って解除時(時間外)には枝、テレポが使えます
	時間前に枝撒き、まだ実装してませんが宝箱奪取も可能になるので枝、テレポは常に使用不可で問題ないと思います。

後前回書き忘れましたが、momotaroイベントですが、ちょっと不安定な可能性があります。
原因がわからないのですが、ループしてるかもしれません。一応コメントアウトしてあります

--------------------
//0805 by 死神

・文字化け修正。
・シーズモードでのテレポート禁止や古木の枝使用禁止はソースレベルで
処理しているのでmapflag.txtから削除。(因みにnopenaltyもソースレベルで
処理しています。)
・battle_athena.confのagit_eliminate_timeをgvg_eliminate_timeに変更。
・@コマンド@GM削除。
・FD_SETSIZEかcygwinで64に設定されていたのせいで最大接続人数が56名を
越えるとマップ鯖が無限ループする問題修正。(ただテストができなかった物なので本当に大丈夫になったかどうかは不明です。あくまでも自分の予測にすぎない物ですが...)
・文字化けのせいでどこをどう修正したか覚えてないので修正したファイルだけ。
	(conf/)
	atcommand_athena.conf
	battle_athena.conf
	mapflag.txt
	(db/)
	castle_db.txt
	(doc/)
	conf_ref.txt
	(common/)
	mmo.h
	(login/)
	login.c
	(char/)
	inter.c
	int_guild.c
	(map/)
	atcommand.h
	atcommand.c
	battle.h
	battle.c
	chrif.c
	guild.h
	guild.c
	intif.h
	intif.c
	map.h
	map.c
	mob.c
	npc.c
	npc.h
	script.c
	skill.c
	pc.c
	makefile

--------------------
//0804 by 釈尊

・アルベルタのぬいぐるみイベントでうさぎのぬいぐるみをあげるとサーバーが落ちるバグ修正

	(conf/npc/)
	npc_event_doll.txt 修正。

--------------------
//0803 by ぴざまん

	GvGでエンペリウム崩壊時gvg_eliminate_timeの値に関わらず即座に退去させられていたバグ修正
	GvGのセリフを一部修正
	inter鯖でcastle.txtがないと起こる色々なエラーを修正
	help.txtを修正(@gvgstart→@agitstart云々)

	(conf/)
	gvg/TEST_prtg_cas01_AbraiJ.txt 修正。
	agit/ev_agit_prtgJ.txt 修正。
	help.txt 修正。
	(map/)
	int_guild.c
		inter_guild_init() 修正。

--------------------
//0802 by Michael_Huang

  Added NPC Script - 'GetGuildMaster' Command.
	(common/)
	version.h
		Mod_Version 0802
	(map/)
	script.c
		buildin_getguildmaster_sub() buildin_getguildmaster()

--------------------
//0801 by Kalen
・アマツ修正
　　実装前のデータ、抜けてるデータなどを調査し修正
	conf/npc/npc_town_guide.txt
	conf/npc/npc_town_amatsu.txt
	conf/npc/npc_event_momotaro.txt
	conf/npc/npc_event_alchemist.txt
	conf/mob/npc_monster35.txt
	conf/warp/npc_warp_amatsu.txt
・map_flag修正
	[GVGMAP]枝、テレポは常に使用不可
・GVG関係
	0800のコマンドに対応

--------------------
//0800 by Michael_Huang

  Added Agit NPC Script & Command.
  Fix FreeBSD GCC compatibility.
  Attachment Agit Demo NPCs. 

	(char/)
	int_guild.c
		mapif_guild_castle_dataload() mapif_guild_castle_datasave()
		int mapif_parse_GuildCastleDataLoad() int mapif_parse_GuildCastleDataSave()
		inter_guild_parse_frommap() inter_guildcastle_tostr() inter_guildcastle_fromstr()
	inter.c
		inter_send_packet_length[] inter_recv_packet_length[]
	(common/)
	mmo.h
		GLOBAL_REG_NUM, struct global_reg {}
	version.h
		Mod_Version 0799.
	(conf/)
	atcommand_athena.conf
		agitstart: 1,agitend: 1
	battle_athena.conf
		agit_eliminate_time: 7000
	map_athena.conf
		conf/agit/ev_agit_*.txt
	(doc/)
	conf_ref.txt
		battle_athena.cnf
	agitdb_ref.txt
	(login/)
	login.c
		parse_login()
	(map/)
	atcommand.h
		agitster, agitend

	atcommand.c
		@agitstart, @agitend
	battle.h
		battle_config.agit_eliminate_time
	battle.c
		battle_config_read()
	chrif.c
		chrif_changedsex() chrif_connectack()
	guild.h
		guild_agit_start() guild_agit_end() guild_agit_break()
	guild.c
		guild_read_castledb() do_init_guild()
		guild_agit_start() guild_agit_end() guild_agit_eliminate_timer() guild_agit_break()
	intif.h
		intif_guild_castle_dataload() intif_guild_castle_datasave()
	intif.c
		packet_len_table[] intif_guild_castle_dataload() intif_guild_castle_datasave()
		intif_parse_GuildCastleDataLoad() intif_parse_GuildCastleDataSave() intif_parse()
	map.h
		agit_flag
	map.c
		agit_flag
	npc.h
		npc_event_doall() npc_event_do()
	npc.c
		npc_event_do_sub() npc_event_do()
	script.c
		buildin_maprespawnguildid() buildin_agitstart() buildin_agitend()
		buildin_getcastlename() buildin_getcastledata()	buildin_setcastledata()
	skill.c
		skill_unit_onplace()
		skill_gangster_count()

--------------------
//0799 by ぴざまん

・GvG実装の為にinter-map間の通信仕様変更
・0798のコンパイルエラー修正(byバグ報告スレ82氏)
	(login/)
	login.c
		parse_login() 修正。
	(map/)
	intif.c
		packet_len_table[] 修正。
		intif_parse_GuildCastleInfo() 修正。
		intif_parse_GuildCastleChange()をintif_parse_GuildCastleChangeErr()に改名・修正。
		intif_parse() 修正。
	guild.c
		guild_read_castledb() 修正。

	(char/)
	inter.c
		inter_send_packet_length[] 修正。
	int_guild.c
		inter_guildcastle_tostr() 修正。
		inter_guildcastle_fromstr() 修正。
		mapif_parse_GuildChangeCastle() 修正。
		mapif_parse_GuildCastleInfo() 修正。
		mapif_guild_castle_info() 修正。
		mapif_guild_change_castle()をmapif_guild_change_castle_err()に改名・修正。
	(common/)
	mmo.h 修正。
	version.h 修正。

--------------------
//0798 by 胡蝶蘭

・login-serverのログイン失敗パケットの長さがおかしかったのを修正
・login-serverにアクセスコントロール機能追加
	・login_athena.cnfにorder,allow,denyを記述することで、
	  IP単位(前方一致)でアクセスを禁止する機能。
	・指定方法は doc/conf_ref.txt を参照
	
	(doc/)
	conf_ref.txt
		login_athena.cnfの部分修正
	(login/)
	login.c
		グローバル変数 access_* 追加
		parse_login()修正,check_ip()追加

・アカウント作成用CGIスクリプト追加
	・自己責任＆詳細な解説無し、質問されてもスルーする可能性有り
	・エディタで開いたら少し説明有り
	・CGI設置の基本さえわかれば問題ないはず
	・メッセージは英語、日本語両対応
	  （Accept-Languageがjaなら日本語に変換します）
	・管理者パスワードなしで動くのでセキュリティには注意(.htaccessなど推奨)

	(tool/cgi/)
	addaccount.cgi
		アカウント作成用CGI。

・その他
	(tool/)
	backup
		castle.txtもバックアップするように

--------------------
//0797 by 死神

・少し修正。
・battle_athena.confの項目変更。(lootitem_time 削除、item_first_get_time、
item_second_get_time、item_third_get_time、mvp_item_first_get_time、
mvp_item_second_get_time、mvp_item_third_get_time 追加。)
・アイテムルート権限を正しく実装。最初攻撃ではなく与えたダメージの
量によって収得権限を与えるように変更。(最初収得権限のみテスト)
パーティの場合パーティの設定に合わせる必要がありますがまだパケットが
不明な所がある為同じパーティなら収得できるようになっています。
・ボウリングバッシュのバグ修正。(多分修正されたはず...)
・装備スクリプトbonusにbSplashRangeとbSplashAddRange追加。
bSplashRangeとbSplashAddRangeは武器でダメージを与えた時のみ発動、通常の武器攻撃扱いなので避けられるが(Flee2による完全回避は不可能)クリは出ないようになっていて武器による状態異常は発生しません。本鯖仕様なんて知りません。
・スキルの重ね置き処理を本鯖に合わせて修正。
・mapflagのgvgはいつもなっているわけじゃないので削除。
・その他細かい修正。
	athena-start 修正。
	(common/)
	mmo.h 修正。
	(conf/)
	mapflag.txt 修正。
	battle_athena.conf 修正。
	(db/)
	const.txt 修正。
	item_db.txt 修正。
	(doc/)
	conf_ref.txt 修正。
	item_bonus.txt 修正。
	(map/)
	mob.c
		mob_spawn()、mob_damage()、mob_class_change()、mob_warp() 修正。
		mob_delay_item_drop()、mob_delay_item_drop2() 修正。
		mobskill_castend_pos() 修正。
	pc.c
		pc_takeitem()、pc_dropitem()、pc_equipitem() 修正。
		pc_calcstatus()、pc_bonus() 修正。
	skill.c
		skill_attack()、skill_additional_effect()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_produce_mix() 修正。
		skill_arrow_create()、skill_unit_timer_sub()、skill_castend_pos() 修正。
	map.h 修正。
	map.c
		map_addflooritem() 修正。
	script.c
		buildin_getitem() 修正。
	pet.c
		pet_return_egg()、pet_get_egg()、pet_unequipitem() 修正。
	battle.h 修正。
	battle.c
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack() 修正。
		battle_weapon_attack()、battle_config_read() 修正。

--------------------
//0796 by huge

・細かい修正
	(conf/npc/)
	npc_event_valentine.txt 修正。

	(map/)
	pc.c
		pc_takeitem() 修正。
	skill.c
		skill_unit_timer_sub() 修正。

--------------------
//0795 by Kalen

・雑談341のnpc_warp_niflheim.txt追加
	conf/warp/npc_warp_niflheim.txt

・mapflag.txt修正(GVGMAP設定)
	　nosaveはしていません

・map_athena.confの修正
	　umbala niflheim等の追加
	　コメントアウトしてあります。

・バレンタインスクリプト追加
	conf/npc/npc_event_valentine.txt
	　チョコ達のDropは弄っていません。お好みでどうぞ
	　尚、手作りチョコを食べたときのエフェクトは
	　最新クライアントにすれば見えます。

・GVGScript追加
	conf/gvg/以下
	　Weiss時代に作成したものをAthena用にコンバート&台詞修正
	　prtg_cas01以外は旗のみです。
	　GVGDATAは鯖再起動で消滅します。
	　エラーが出るようならコメントアウトしてください。
	　MobData、執事Scriptもprtg_cas01のみです。(TestScript)
	　あくまでテストスクリプトです。GVG実装の参考にどうぞ

--------------------
//0794 by DRG

・skillusedで指定したIW,QMなどにのってる間MOBスキルを使用するように変更
・アップスレ３の7のathena-startを一応含めときました

	athena-start
	(map/)
	skill.c
		skill_unit_onplace() 修正。

--------------------
//0793 by huge

・サーバーsnapshot
・サーバーがクラッシュするバグを修正
・発動せずに罠が消えたら、設置用トラップが返ってくるように修正。
・ルート権限で、同じパーティーのキャラはすぐ拾えるように修正。
・バグ報告スレ３ >>54 のバグ修正。
・ログイン時にサーバー側にIDを表示するようにしました。
	(login/)
	login.c
		parse_login() 修正。
	(conf/npc/)
		npc_event_doll.txt 修正。
	(map/)
	skill.c
		skill_unit_timer_sub() 修正。
	mob.c
		mob_spawn_dataset() 修正。
		mob_damage() 修正。
	pc.c
		pc_additem() 修正。

--------------------
//0791 by 聖

・マップサーバがクラッシュするバグ修正。
・イグ葉を使ってもジェムが必要になる問題を修正。
・PvP強制送還実装。
・PvPでリザレクションが出来なかった問題を修正。
・その他細かな修正。
	(map/)
	guild.c
	mob.c
	pc.c
	skill.c

--------------------
//0790 by 死神

・バグ修正。
	(conf/)
	battle_athena.conf 誤字修正。
	(doc/)
	conf_ref.txt 誤字修正。
	(common/)
	mmo.h 修正。
	(map/)
	itemdb.h 修正。
	map.h 修正。
	skill.c
		skill_check_condition()、skill_use_pos()、skill_unit_onplace() 修正。

--------------------
//0789 by huge

・ドロップアイテムにルート権限を実装。
・最初に攻撃した人以外がドロップアイテムを拾えるまでの時間を設定できるように。
	(/conf)
	battle_athena.conf 項目追加。

	(/doc)
	conf_doc.txt 説明追加。

	(/map)
	battle.h
		Battle_Config{} 修正。
	battle.c
		battle_config_read() 修正。
	itemdb.h
		item_data {} 修正。
	map.h
		flooritem_data {} 修正。
		mob_data {} 修正。
	map.c
		map_addflooritem() 修正。
	mob.c
		delay_item_drop{} 修正。
		mob_spawn() 修正。
		mob_damage() 修正。
		mob_delay_item_drop() 修正。
		mob_warp() 修正。
	pc.c
		pc_takeitem() 修正。

--------------------
//0788 by あゆみ

・cardスキルの処理？を修正。
・@allskillコマンドの再修正。

	(map/)
	pc.c
		pc_skill() 修正。
		pc_allskillup() 修正。

--------------------
//0787 by ぽぽぽ

・ペットにもmob_avail.txtの設定を適用するようにした。
・MOBスキルのskillusedでval1に0を入れるとあらゆるスキルに対して発動するようにした。
・skillusedで発動したスキルの対象を、発動させたPCにするかどうか設定できるようにした(対IWハメなど？)。
	(/map)
	clif.c
		clif_pet0078()、clif_pet007b()修正。
	mob.c
		mobskill_use()修正。
	skill.c
		skill_attack() 、skill_castend_damage_id()修正。
	battle.h
	battle.c
		battle_config_read()修正。

--------------------
//0786 by huge

・BDS修正
    前から吹き飛ばして行くと良くないかもしれないので、後ろから処理
    有効範囲の修正

	(/map)

	skill.h
	skill.c
		skill_castend_damage_id() 修正。
		skill_castend_nodamage_id() 修正。
		skill_brandishspear_first() 修正。
		skill_brandishspear_dir() 修正。

--------------------
//0785 by 死神

・本鯖に合わせて修正。(韓国鞍のパッチnoticeを参考して修正しました。)
・BBとBSのキャスティング時間を0.7秒にしてディレイは0に変更。
・ghostの変わりにマップ移動後の無敵時間を設定。この時間の間はどんな攻撃も受けないが移動や攻撃、スキル使用、アイテム使用でこの時間はなくなります。シーズモードではこの無敵時間を2倍として適用。
・シーズモードで死んでも経験が減らないように修正。(mapflagのnopenaltyを設定する必要はありません。)
・スキッドで滑べる距離増加。
・既に沈黙にかかってる対象にレックスディビーナを使うと沈黙が解除されるように変更。
・呪いにかかると移動速度も減るように修正。
・battle_athena.confに項目変更。
・スキルの重ね置きを判断処理を少し変更。
・HPの自然回復時間が4秒ではなく6秒だとわかったのでデフォルト修正とbattle_athena.conf修正。
・その他細かい修正やバグ修正。
・殆どテストしてません。
	(conf/)
	atcommand_athena.conf 修正。
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(db/)
	cast_db.txt 修正。
	(map/)
	atcommand.c 修正。
	clif.c
		clif_mob0078()、clif_mob007b()、clif_skill_estimation() 修正。
		clif_mob_class_change()、clif_parse_WalkToXY() 修正。
		clif_parse_ActionRequest()、clif_parse_LoadEndAck() 修正。
		clif_parse_UseItem()、clif_parse_UseSkillToId() 修正。
		clif_parse_UseSkillToPos()、clif_parse_UseSkillMap() 修正。
	mob.h 修正。
	mob.c
		mob_get_viewclass()、mob_attack()、mob_target() 修正。
		mob_ai_sub_hard_activesearch()、mob_ai_sub_hard() 修正。
		mobskill_castend_id()、mobskill_castend_pos() 修正。
	skill.h 修正。
	skill.c
		skill_can_produce_mix()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_castend_id() 修正。
		skill_castend_pos()、skill_use_id()、skill_readdb() 修正。
		skill_check_condition()、skill_unit_onplace()、skill_unitsetting() 修正。
		skill_additional_effect()、skill_check_unit_range() 修正。
		skill_check_unit_range_sub()、skill_status_change_end() 修正。
		skill_status_change_start() 修正。
	pc.h
	pc.c
		pc_ghost_timer()、pc_setghosttimer()、pc_delghosttimer() 削除。
		pc_gvg_invincible_timer() -> pc_invincible_timer()に変更。
		pc_setgvginvincibletimer() -> pc_setinvincibletimer()に変更。
		pc_delgvginvincibletimer() -> pc_delinvincibletimer()に変更。
		pc_authok()、pc_attack_timer()、pc_calcstatus() 修正。
		pc_setrestartvalue()、pc_damage()、pc_allskillup() 修正。
		do_init_pc() 修正。
	battle.h 修正。
	battle.c
		battle_config_read()、battle_weapon_attack()、battle_check_target() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_get_speed() 修正。
	map.h 修正。
	map.c
		map_quit() 修正。

--------------------
//0784 by あゆみ

・カードスキルを覚えている状態で@allskillコマンドを使用するとmap-serverが暴走する問題を修正。

	(map/)
	pc.c
		pc_allskillup() 修正。

--------------------
//0783 by huge

・ブランディッシュスピアの修正
	範囲指定、斜めの際の格子範囲、攻撃力計算
	多分合ってると思うんですけど、イマイチ自信が持てない・・・
	(参考)みすとれ巣 -スキル関連豆情報
・スペルブレイカーをちょっと修正
・プロボックをMVPmobに効かないよう修正
・バグ報告スレ３ >>8 で報告されたものの取り込み

	(/db)
	create_arrow.txt 修正。
	skill_db.txt 修正。

	(/map)
	battle.c
		battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack() 修正。

	skill.h
	skill.c
		skill_brandishspear_first() 追加。
		skill_brandishspear_dir() 追加。
		skill_castend_nodamage_id() 修正。
		skill_castend_damage_id() 修正。
		skill_unit_group() 修正。

--------------------
//0782 by ぴざまん
・@allskillコマンドの使用条件が参照されていなかったバグ修正
	(/map)
	atcommand.c
		atcommand() 修正。

--------------------
//0781 by Chunglyeng
・バド, ダンサー音楽具現
	(/map)
	skill.c 修正。

--------------------
//0780 by reia
・ペコペコの卵などが孵化するとノビになってしまう問題修正。
・GMコマンド「@kickall」が何時の間にか無効になっていたので修正。

	(/conf)
	atcommand_athena.conf 修正。
	(db/)
	mob_skill_db.txt 修正。
	(map/)
	atcommand.c
		atcommand_config_read() 修正。

--------------------
//0779 by あゆみ

・全スキル取得コマンドの追加。
	・GMで全スキルを覚えられるようにしている場合や、スキルの数が多い職業は一部のスキル表示がおかしくなります。その場合はリログして下さい。
	@allskill : 現在の職業で取得可能な全スキルを取得する。(クエストスキル含む)

	(conf/)
	battle_athena.conf 修正。
	help.txt 修正。
	(map/)
	atcommand.h 修正。
	atcommand.c
		atcommand() 修正。
	pc.h 修正。
	pc.c
		pc_allskillup() 追加。
	(doc/)
	conf_ref.txt 修正。
	help.txt 修正。

--------------------
//0778 by huge

・スペルブレイカーの修正
  ・キャスティングタイムの無いスキルには効かないように修正。
  ・使用された相手の消費SPの修正。

	(map/)
	skill.c
		skill_castend_nodamage_id() 修正。

--------------------
//0777 by ぴざまん

・ギルド攻城戦仮実装
　・基本的な部分だけしか実装してない上、いくつかの点で本鯖と相違があります。
　・攻城戦開始と終了は@コマンドで行います
	@gvgstart : 攻城戦開始
	@gvgend : 攻城戦終了
　・battle_athena.confに項目追加。
	(char/)
	int_guild.c
		inter_guild_init()、inter_guild_readdb()、inter_guild_save() 修正。
		inter_castle_save_sub()、mapif_guild_castle_info() 追加。
		mapif_guild_change_castle()、mapif_parse_GuildCastleInfo() 追加。
		mapif_parse_GuildChangeCastle() 追加。
	inter.c
		inter_config_read() 修正。
		inter_send_packet_length[]、inter_recv_packet_length[] 修正。
	int_guild.h 修正。
	(map/)
	atcommand.c
		atcommand()、atcommand_config_read() 修正。
	battle.c
		battle_config_read() 修正。
	guild.c
		guild_castle_search()、guild_read_castledb()追加。
		do_init_guild() 修正。
		guild_gvg_init()、guild_gvg_final()、guild_gvg_final_sub() 追加。
		guild_gvg_eliminate()、guild_gvg_eliminate_sub() 追加。
		guild_gvg_eliminate_timer()、guild_gvg_empelium_pos() 追加。
		guild_gvg_break_empelium() 追加。
	intif.c
		intif_parse()、packet_len_table[] 修正。
		intif_parse_GuildCastleInfo()、intif_parse_GuildCastleChange() 追加。
		intif_guild_castle_info()、intif_guild_castle_change() 追加。
	mob.c
		mob_damage() 修正。
	atcommand.h 修正。
	battle.h 修正。
	guild.h 修正。
	intif.h 修正。
	mob.h 修正。
	(common/)
	mmo.h 修正。
	(conf/)
	battle_athena.conf 修正。
	inter_athena.conf 修正。
	msg_athena.conf 修正。
	atcommand_athena.conf 修正。
	(db/)
	castle_db.txt 追加。
--------------------
//0776 by 死神

・NPCスキル孵化実装。(mob_skill_db.txtのval1を使います。)
・mob_skill_db.txtの確率を千分率から万分率に変更。(ただmob_skill_db.txtの修正はしてません。)
・モンスターがダブルアタックする問題修正。(修正されたかどうかの自信はありませんが...)
・その他細かい修正。
	(db/)
	mob_skill_db.txt 修正。
	skill_db.txt 修正。
	(map/)
	map.h 修正。
	mob.h 修正。
	mob.c
		mob_spawn_dataset()、mob_spawn() 修正。
		mob_changestate()、mobskill_use() 修正。
		mob_class_change() 追加。
	npc.c
		npc_parse_mob() 修正。
	battle.c
		battle_check_target() 修正。
	clif.h 修正。
	clif.c
		clif_mob_class_change() 追加。
	skill.c
		skill_castend_nodamage_id() 修正。

--------------------
//0775 by 死神

・シーズモードの処理修正。
・シーズモードの無敵時間の間はどんな攻撃も受けないように修正。
・シーズモードの無敵時間が時間切れになる前には解除されないように修正。
・battle_athena.confに項目追加。
・@hideや/hideによるGMハイディング中は自分に自動使用されるスキル以外のスキル使用や攻撃を受けないように修正。
・ハイディング中地属性スキル以外の攻撃を受けないように修正。(トラップやクァグマイア等のスキルは影響を受けるかどうか不明なので今までと同じように影響を受けるように処理。)
・トンネルドライブの移動速度を本鯖に合わせました。
・その他バグ修正や細かい修正。(殆ど未テスト)
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_check_target()、battle_calc_damage() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_calc_misc_attack() 修正。
		battle_config_read()、battle_weapon_attack() 修正。
	pc.h 修正。
	pc.c
		do_init_pc()、pc_authok() 修正。
		pc_attack()、pc_attack_timer() 修正。
		pc_setgvg_ghosttimer()、pc_delgvg_ghosttimer()を
			pc_setgvginvincibletimer()、pc_delgvginvincibletimer()に修正。
		pc_gvg_invincible_timer() 追加。
		pc_attack_timer()、pc_steal_item()、pc_calcstatus() 修正。
	clif.c
		clif_parse_ActionRequest()、clif_parse_UseItem() 修正。
		clif_parse_UseSkillToId()、clif_parse_UseSkillToPos() 修正。
		clif_parse_UseSkillMap()、clif_parse_WalkToXY() 修正。
	map.h 修正。
	map.c
		map_quit() 修正。
	mob.c
		mob_attack()、mob_target()、mob_ai_sub_hard_activesearch() 修正。
		mob_ai_sub_hard_mastersearch()、mob_ai_sub_hard() 修正。
		mob_damage()、mobskill_castend_id()、mobskill_castend_pos() 修正。
	skill.c
		skill_castend_damage_id()、skill_attack() 修正。
		skill_castend_id()、skill_castend_pos()、skill_castend_map() 修正。

--------------------
//0774 by 獅子o^.^o
・Monk job bouns 修正
・ドケビ 修正
(db/)
	job_db2.txt 修正
	pet_db.txt 修正

--------------------
//0773 by 聖

・細かいバグ修正
	(map/)
	skill.c 修正。
	battle.c 修正。

--------------------
//0772 by ぴざまん

・シーズモード下で以下の点を修正
　・連続して攻撃できなくなっていたバグ修正
　・ダメージ軽減率が正しく設定できなかったバグ修正
　・無敵時間実装。battle_athena.confのgvg_ghost_timeで設定できます

・ハイディングで魔法攻撃等を回避できなかったバグ修正

	(map/)
	skill.c
		skill_attack()、skill_unit_onplace()、skill_check_condition() 修正。
	clif.c
		clif_parse_ActionRequest()、clif_parse_UseItem() 修正。
		clif_parse_UseSkillToId()、clif_parse_UseSkillToPos() 修正。
		clif_parse_UseSkillMap()、clif_parse_WalkToXY() 修正。
	pc.c
		do_init_pc()、pc_authok() 修正。
		pc_attack()、pc_attack_timer() 修正。
		pc_setgvg_ghosttimer()、pc_delgvg_ghosttimer() 追加。
		pc_gvg_ghost_timer() 追加。
	map.c
		map_quit() 修正
	battle.c
		battle_config_read()、battle_weapon_attack() 修正。
	battle.h 修正。
	pc.h 修正。

--------------------
//0771 by huge

・スペルブレイカー実装
	(map/)
	skill.c
		skill_castend_nodamage_id() 修正。
	(db/)
	skill_db.txt 修正。

--------------------
//0770 by 聖

・青箱系の処理変更
・その他バグ修正
	(map/)
	battle.c 修正。
	itemdb.c 修正。
	mob.c 修正。
	script.c 修正。

--------------------
//0769 by 死神

・シーズモード修正。
・無敵時間の方はghost_timer以外の方法で実装するつもりなので今は削除しています。
・スクリプトviewpointが正しく動作しない問題修正。
・produce_db.txtを修正。(乳鉢はskill_require_db.txtで処理しています。そしてアイテムの数を0にすれば消耗はされないけど作る時必要なアイテムになります。)
・その他細かい修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(db/)
	produce_db.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_calc_damage()、battle_calc_weapon_attack() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_calc_misc_attack()、battle_check_target()、battle_config_read() 修正。
	skill.c
		skill_unit_onplace()、skill_castend_damage_id()、skill_castend_id() 修正。
		skill_use_id()、skill_use_pos()、skill_check_condition() 修正。
		skill_can_produce_mix() 修正。
	pc.c
		pc_attack_timer()、pc_attack()、pc_isUseitem()、pc_delitem() 修正。
		pc_damage() 修正。
	mob.c
		mob_damage()、mobskill_use_id()、mobskill_use_pos() 修正。
	script.c
		buildin_viewpoint()、buildin_emotion() 修正。
	makefile 修正。

--------------------
//0768 by ぴざまん

・シーズモード下で、以下の点を修正
　・死亡したらセーブポイントに強制送還するように修正
　・特定のアイテムが使用できないように修正(アンティペインメント・ハエの羽)
　・特定のスキルが使用できないように修正(ワープポータル・バックステップ・インティミデイト・テレポート・インデュア)
　・同盟ギルドには無条件で攻撃できないように修正
　・敵対ギルドには無条件で攻撃できるように修正
　・無敵時間中は両者とも攻撃できないように修正

ソースレベルでテレポ、ハエの使用を禁じているため攻城戦MAPのmapflagにnoteleportは必要ありません
また、無敵時間はghost_timer依存です。つまりbattle_athena.conf内のghost_timeが無敵時間になります

	(map/)
	skill.c
		skill_castend_damage_id()、skill_castend_id() 修正
		skill_check_condition() 修正
	pc.c
		pc_damage() 修正
	battle.c
		battle_weapon_attack() 修正

--------------------
//0767 by huge

・ファーマシーで、製造の書が減る問題を修正
・武器製造DBで、いくつか抜けていたのを修正

	(map/)
	skill.c 修正。
	(db/)
	produce_db.txt 修正。

--------------------
//0766 by ぴざまん

・シーズモード下で、以下の点を修正
　・正規ギルド承認がないとエンペリウムに攻撃が効かないように修正
　・エンペリウムに対するスキル攻撃が効かないように修正
　・魔法攻撃、遠距離攻撃、罠のダメージ補正を実装
　　魔法攻撃：50%　遠距離攻撃：75%　罠：60%
　　これは人にもエンペリウムにも適用されます
	(map/)
	battle.c
		#include "guild.h" 追加
		battle_calc_damage()、battle_calc_weapon_attack() 修正

--------------------
//0765 by ぴざまん

・装備制限実装
・装備制限がかかった装備品は該当マップに移動した際に自動的に装備が外れ、
　再装備もできなくなります
・制限できるのは装備品のみです。カード類は制限できません
	(db/)
	item_noequip.txt 追加
	(map/)
	itemdb.h 修正
	itemdb.c
		do_init_itemdb()、itemdb_search() 修正
		itemdb_read_noequip 追加
	pc.c
		pc_checkitem()、pc_isequip() 修正

--------------------
//0764 by 死神

・全てのダメージが1になる防御を10000から1000000に変更。
・battle_athena.confに項目追加。
・モンスターから経験値を貰う処理を本鯖のように修正。
・スキルスローポイズン実装。
・交換バグ修正。
・その他細かい修正。
・テストは殆どしてません。
	(db/)
	mob_db.txt 修正。
	skill_db.txt 修正。
	(doc/)
	conf_ref.txt 修正。
	db_ref.txt 修正。
	(conf/)
	battle_athena.conf 修正。
	(map/)
	makefile 修正。
	battle.h 修正。
	battle.c
		battle_get_def()、battle_get_mdef() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_calc_misc_attack()、battle_config_read() 修正。
	skill.h 修正。
	skill.c
		skill_castend_nodamage_id()、skill_castend_damage_id() 修正。
		skill_check_condition()、skill_status_change_timer() 修正。
		skill_status_change_end()、skill_status_change_start() 修正。
		skill_additional_effect()、skill_produce_mix()、skill_unit_timer_sub() 修正。
		skill_check_unit_sub()を skill_check_unit_range_sub()に変更。
		skill_check_unit_range() 追加。
		skill_castend_pos()、skill_area_sub_count() 修正。
	mob.c
		mobskill_castend_pos()、mob_damage() 修正。
	clif.c
		clif_parse_TradeRequest()、clif_parse_TradeAck() 修正。
		clif_parse_TradeAddItem()、clif_parse_TradeOk() 修正。
		clif_parse_TradeCansel()、clif_parse_TradeCommit() 修正。
	map.h 修正。
	map.c
		do_init() 修正。
	pc.c
		pc_calc_skilltree()、pc_calcstatus() 修正。
	tarde.c
		trade_tradeack() 修正。

--------------------
//0763 by 胡蝶蘭

・サーバーsnapshot
	フォルダを整理したので注意してください。
	以前のセーブデータ(account.txtなど)はsaveフォルダに置いてください
	cnfファイルは各種ファイルのパスが変わってるので、
	この古いものをコピーではなく、新しく書き換えなおしてください。

・フォルダ整理
	confフォルダ
		NPC関連をconf/npc/、MOB配置関連をconf/mob/、WARP関連をconf/warp/
		テストやサンプルをconf/sample/に移動しました。
	dbフォルダ
		sampleフォルダのうちdb関係を db/sampleに移動しました。
	help.txt/motd.txt
		confフォルダに移動しました
	account.txt/athena.txt/guild.txt/party.txt/pet.txt/storage.txt
		saveフォルダに移動しました
	tool/backup
		パス修正

・motd.txt/help.txtのパスをmap_athena.cnfで指定できるように
	map.h/map.c/atcommand.c/atcommand.h修正

・athena-startにrestartオプション追加
	./athena-start restartでAthenaを再起動します

--------------
//0761 by ぽぽぽ

・MOBのMDEFに10000以上指定してもファイアピラーで普通にダメージを与えてしまうのを修正。
	(map/)
	battle.c

--------------
//0760 by ll3y

・文字化け修正
	(map/)
	script.c

--------------
//0759 by 獅子o^.^o
・スピアクイッケン 修正
・Dancer skill tree 修正
(db/)
	cast_db.txt 修正
	skill_tree.txt 修正

--------------
//0758 by hack
・Put GM messages into msg_table which is loaded from msg_athena.conf.
(Easy to translate into other language)
	(map/)
	atcommand.h
	atcommand.c
		msg_conf_read()	Read conf/msg_athena.conf
		Put messages into msg_table which is loaded from msg_athena.conf.
	map.c
		do_init()
	(conf/)
	msg_athena.conf	Store the message of atcommand, easy to translate into other language.
	
--------------
//0757 by Michael
	(map/)
	script.c 
		buildin_viewpoint()
		Fix packet sequence of viewpoint command.

--------------
//0756 by ll3y

・Interix(Windows Services for Unix 3.5)でコンパイルが通るように修正
  Interop Systems(http://www.interopsystems.com/)よりgmakeとzlibを取ってくるか、
  自前でInterix用を用意する必要があります。
	(common/)
	socket.h 修正。

--------------
//0755 by 死神

・バグ修正と説明追加。(報告されたのは多分全て修正されたのかと...)
・0751でスキルの最大レベルを100まで設定できるようにしました。
・cast_db.txtに入っている状態異常の維持時間は自分が適度に入れた物です。本鯖の仕様なんて知りませんので。
	(doc/)
	db_ref.txt 修正。
	(db/)
	cast_db.txt 修正。
	skill_db.txt 修正。
	(map/)
	skill.h 修正。
	skill.c
		skill_check_unit_sub()、skill_castend_id()、skill_use_id() 修正。
		skill_status_change_end()、skill_status_change_start() 修正。
		skill_castend_map() 修正。
	mob.c
		mobskill_castend_id()、mobskill_castend_pos() 修正。
	pc.c
		pc_calcstatus() 修正。
	battle.c
		battle_calc_pc_weapon_attack()
		battle_calc_mob_weapon_attack()
		battle_calc_magic_attack()

--------------
//0754 by 獅子o^.^o
(db/)
	cast_db.txt 修正

--------------
//0753 by 聖

・IWの発生ポイントを指定するとメテオのエフェクトが一切出なくなる
　問題が復活していたので修正。
・warningを出ないようにコード修正。
	(map/)
	skill.c
			skill_castend_pos2() 修正。
	chrif.h

--------------
//0752 by ぴざまん

・changesexスクリプト実装。性別を反転させることができます
　性別反転成功後は、そのプレイヤーは強制的に接続を切断されます
　また、ダンサー・バードの互換性はかなり怪しいです
　ダンサー・バードがスロット内どこかに居るアカウントでの反転は、以下の点に注意して下さい
　・必ず反転させる前にそのキャラクターをスキルリセットしてください
　　そのまま反転させると、共通するスキル(楽器の練習等)しか残らなくなってしまいます
　・ダンサー・バード専用武器を装備している場合は、外してから反転させてください
　　そのまま反転させると、そのキャラクターの開始時に
　　クライアントエラーが出ます（出るだけで、落ちることはないのですが…）
・データベース修正 by 獅子o^.^o
	(map/)
	chrif.c
		packet_len_table[]、chrif_parse()修正
		chrif_changesex()、chrif_changedsex()追加
	chrif.h 修正
	(char/)
	char.c
		parse_frommap()、parse_tologin()修正
	(login/)
	login.c
		parse_fromchar()修正
	(db/)
	cast_db.txt 修正
	skill_require_db.txt 修正

--------------
//0751 by 死神

・skill_db.txtとcast_db.txtの変更とskill_require_db.txtの追加。
・毒にかかるとHPが減るように変更。HPは1秒に最大HPの1%減ります。(未テスト)
・石化を進行中の物と完全な物に分けてHPが減るように変更。(1秒に最大HPの1%)ブレッシングで完全石化だけ治せるように修正。(未テスト)
・ハンターのトラップにエフェクト実装。ただランドマインとショックウェーブは爆発エフェクトが出ないようです。ランドマインはファイアピラーの爆発エフェクトが出るように変えています。
・オートカウンターの方向チェックをするように変更と本鯖仕様に合わせました。
・バックスタブも方向チェックをするように変更。
・インティミデイトの処理変更。
・ディフェンダーの移動速度減少を本鯖に合わせました。ASPDは勝手ながら
(30 - (skilllv*5))%が減るようにしましたが本鯖でいくら程減るのかの情報をお願いします。
・トンネルドライブLV1で移動速度が150から312になるのが確認されて計算を変更しましたがレベルによってどれぐらい増加するかは不明です。情報を求めます。(今の計算式は適度に作った物です。)
・ポーション製造の計算式変更とちょっと修正。
・一部地面スキルの重ね置きを禁止。
・bNoMagicDamageで魔法による異常やステータスアップ効果が出ないように修正。(リザレクション以外の魔法は無効になります。)
・battle_athena.confに項目追加。
・その他色々と修正。
・変更されたskill_db.txt、castdb.txtと追加されたskill_require_db.txtの構造は今の所自分しか知らないのでdb_ref.txtに説明を追加する予定なのでそれまではこれらの変更は控えてください。
	(char/)
	int_guild.c 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(db/)
	skill_db.txt 修正。
	skill_require_db.txt 修正。
	cast_db.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_counttargeted()、battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_weapon_attack()、battle_config_read() 修正。
	skill.h 修正。
	skill.c
		skill_attack()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_status_change_start() 修正。
		skill_check_condition()、skill_castend_pos() 修正。
		skill_use_id()、skill_use_pos() 修正。
		skill_status_change_timer()、skill_status_change_start() 修正。
		skill_check_unit_sub() 追加。
	pc.h 修正。
	pc.c
		pc_damage()、pc_counttargeted()、pc_counttargeted_sub() 修正。
	mob.h 修正。
	mob.c
		mob_countslave()、mob_counttargeted()、mob_counttargeted_sub() 修正。
		mobskill_use()、mob_can_move()、mob_damage() 修正。
		mobskill_use_id()、mobskill_use_pos()、mobskill_castend_id() 修正。
		mobskill_castend_pos() 修正。
	map.c
		map_quit() 修正。

--------------
//0750 by CHRIS

・スキル関係のDBを調整
	(db/)
	skill_db.txt
	cast_db.txt
	skill_require_db.txt

--------------
//0749 by 死神

・色々と変更と修正。
・スキルの仕様変更や実装、状態異常の仕様変更や実装。
・スキルの使用条件をdbに設定できるように変更。
・skill_db.txtとcast_db.txtの仕様変更。
・マップ鯖の無限ループ可能性がある部分を修正。(あくまでも可能性が
あっただけの物です。無限ループの原因とは断言できません。)
・トラップの発動実装。(ただ実際に動作はまだ修正していません。
見た目が変わっただけです。)
・battle_athena.confに項目追加を削除。
・0748の修正削除と文字化け修正。
・skill_db.txt、cast_db.txt、skill_require_db.txtの方がまだ完成されていないので
かなりの量のスキルが正しく動作しません。(db_ref.txtに設定方法を入れないと
 けないのですが時間がなかったので...) そして修正はしましたがテストは
殆んどしてませんので注意してください。
	(char/)
	char.c 修正。
	int_party.h 修正。
	int_party.c 修正。
	int_guild.h 修正。
	int_guild.c 修正。
	int_pet.h 修正。
	int_pet.c 修正。
	int_storage.h 修正。
	int_storage.c 修正。
	charの方は大した修正はしてません。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(db/)
	skill_db.txt 修正。
	cast_db.txt 修正。
	skill_require_db.txt 追加。
	produce_db.txt 修正。
	(map/)
	map.h 修正。
	map.c
		map_check_dir() 追加。
		map_readmap()、map_addblock()、map_delblock() 修正。
		map_foreachinarea()、map_foreachinmovearea() 修正。
		map_addflooritem() 修正。
	pc.h 修正。
	pc.c
		pc_spiritball_timer()、pc_addspiritball()、pc_delspiritball() 修正。
		pc_steal_item()、pc_steal_coin()、pc_calcstatus() 修正。
		pc_checkallowskill()、pc_jobchange()、pc_checkweighticon() 修正。
		pc_damage()、pc_equipitem()、pc_walk()、pc_stop_walking() 修正。
		pc_authok()、pc_counttargeted()、pc_counttargeted_sub() 修正。
		pc_damage()、pc_setpos() 修正。
	skill.h 修正。
	skill.c
		skill_get_range()、skill_get_sp()、skill_get_num() 修正。
		skill_get_cast()、skill_get_delay() 修正。
		skill_get_hp()、skill_get_zeny()、skill_get_time() 追加。
		skill_get_time2()、skill_get_weapontype() 追加。
		skill_get_unit_id()、skill_blown()、skill_additional_effect() 修正。
		skill_attack()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id()、skill_castend_id() 修正。
		skill_castend_pos()、skill_unit_onplace() 修正。
		skill_unit_timer_sub_onplace()、skill_unitsetting() 修正。
		skill_use_id()、skill_use_pos()、skill_check_condition() 修正。
		skill_status_change_end()、skill_status_change_timer() 修正。
		skill_status_change_start()、skill_can_produce_mix() 修正。
		skill_produce_mix()、skill_gangsterparadise() 修正。
		skill_gangster_out()、skill_gangster_in() 修正。
		skill_gangster_count() 追加。
		skill_readdb() 修正。
	battle.h 修正。
	battle.c
		distance()、battle_counttargeted()、battle_get_range() 追加。
		battle_get_dir() 追加。
		battle_get_maxhp()、battle_get_str()、battle_get_agi() 修正。
		battle_get_vit()、battle_get_dex()、battle_get_int() 修正。
		battle_get_luk()、battle_get_flee()、battle_get_hit() 修正。
		battle_get_flee2()、battle_get_critical()、battle_get_baseatk() 修正。
		battle_get_atk()、battle_get_atk2()、battle_get_def() 修正。
		battle_get_def2()、battle_get_mdef()、battle_get_speed() 修正。
		battle_get_adelay()、battle_get_amotion()、battle_get_party_id() 修正。
		battle_get_guild_id()、battle_get_size() 修正。
		battle_check_undead() 追加。
		battle_check_target()、battle_addmastery() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_weapon_attack() 修正。
	clif.h 修正。
	clif.c
		clif_skillinfo()、clif_skillinfoblock()、clif_skillup() 修正。
		clif_item_skill()、clif_changeoption()、clif_parse_LoadEndAck() 修正。
		clif_01ac() 追加。
		clif_parse_WalkToXY()、clif_parse_ActionRequest() 修正。
		clif_parse_TakeItem()、clif_parse_DropItem() 修正。
	mob.h 修正。
	mob.c
		mobskill_castend_id()、mobskill_castend_pos() 修正。
		mobskill_use_id()、mobskill_use_pos()、mob_heal() 修正。
		mob_spawn()、mob_damage()、mob_walk() 修正。
		mob_stop_walking()、mob_warp()、mob_counttargeted() 修正。
		mob_counttargeted_sub()、mob_countslave() 修正。
		mob_attack()、mob_target()、mob_ai_sub_hard_activesearch() 修正。
		mob_ai_sub_hard_mastersearch()、mob_ai_sub_hard() 修正。
	script.c
		buildin_sc_start() 修正。
	path.c
		can_move() 修正。
	pet.c
		pet_data_init()、pet_stop_walking() 修正。
	npc.c
		npc_parse_warp()、npc_parse_shop()、npc_parse_script() 修正。

--------------
//0748 by Michael
	(map/)
	pc.c 
		pc_walk();
		Fix Player cannot move in ICEWALL but have Path.
	mob.c
		mob_walk();
		Fix Monster cannot move in ICEWALL but have Path.
	path.c
		can_move();
		Fix Player&Monster cannot move in ICEWALL.

--------------
//0747 by 聖
・ペットがエモを出すとmap-serverが落ちることがあった問題を修正。
	(map/)
		clif_parse_SendEmotion() 修正。

--------------
//0746 by Michael
	(map/)
	script.c 
		Add Script command - checkoption(type);
		Attach a npc_testchkoption.txt npc script!

--------------
//0745 by ぴざまん
・ギャングスターパラダイス実装
・PvPエリアのmapflagを修正(同士討ちが無くなったかと思います)
・シーズモードでノックバックがあったバグを修正
・インティミの遅延時間を少し調整
	(map/)
	skill.c
		skill_attack()、skill_additional_effect()修正
		skill_gangsterparadise()、skill_gangster_in()、skill_gangster_out()追加
	clif.c
		clif_parse_ActionRequest()修正
	mob.c
		mob_target()、mob_attack()修正
		mob_ai_sub_hard()、mob_ai_sub_hard_mastersearch()修正
		mob_ai_sub_hard_activesearch()修正
	map.h 修正
	skill.h 修正
	(conf/)
	npc_pvp.txt 修正

--------------
//0744 by 聖

・アイスウォール、メテオストームのコンボでメテオストームのエフェクトが表示されなくなる問題を修正。
・HP吸収スキルのエフェクト修正。
・battle_athena.confに項目追加。
・パケ周りの細かい修正。
	(conf/)
	battle_athena.conf
	(doc/)
	conf_ref.txt
	(map/)
	battle.c
	battle.h
	clif.c
	pc.c
	pet.c
	skill.c

--------------
//0743 by J

・取り巻き召喚などを本鯖に似せる為の修正。
　あと本鯖相違スレにあったゴスリンの取り巻きを修正。
　デリーターの空と地のスキルが逆になっていたのを修正。
	(db/)
	mob_skill_db.txt 修正
  
--------------
//0742 by ぴざまん

・インティミデイトを実装
　攻撃とワープの分別がうまくいかなかったので
　SC_INTIMIDATEを使って遅延処理をしました
・skill_dbの誤字等を修正
	(map/)
	skill.c
		skill_additional_effect()、skill_castend_map()修正
		skill_castend_nodamage_id()、修正
		skill_status_change_start()、skill_status_change_end()修正
	map.h 修正
	skill.h 修正
	(db/)
	skill_db.txt 修正

--------------------
//0741 by whitedog

snapshot

--------------
//0740 by ぽぽぽ
・PCがMOBにタゲられたとき3匹目から防御と回避が減るようにした。
　1匹につき回避は10%、防御は5%減ります。
	(map/)
	pc.h
	pc.c
		pc_counttargeted()、pc_counttargeted_sub()追加
	battle.c
		battle_get_flee()、battle_get_def()、battle_get_def2()修正。

--------------
//0739 by 聖
・ファイアーウォール等の設置系スキルが正しく表示されない問題を修正。
・マリンスフィアが自爆するとサンダーストーム等のダメージが表示されなくなる問題を修正。
・HP吸収系スキルで敵が回復してるエフェクトが出るよう修正。
	(map/)
	skill.c
		skill_castend_damage_id() 修正。
	battle.c
		battle_calc_misc_attack() 修正。
	clif.c
		clif_getareachar_skillunit() 修正。
		clif_skill_setunit() 修正。

--------------
//0738 by ぴざまん
・ストームガストを完全に本鯖仕様に修正(3回で絶対凍結＆凍結状態の敵はSGをくらわない)
・サフラギウムが自分にかけられるバグ修正
	(map/)
	skill.c
		skill_additional_effect()、skill_attack()修正
		skill_castend_nodamage_id()修正
	map.h 修正

--------------
//0737 by ぽぽぽ
・アンクルが歩いている敵に効かない&複数の敵に効くのを修正。
	(map/)
	skill.c
		skill_unit_onplace()、skill_unit_onout()修正
	mob.c
		mob_stop_walking()修正

--------------
//0736 by ぴざまん
・状態異常耐性が効果時間にも及ぶ様に修正。発動率と同率で効果時間が割り引かれます
・ストーンカースの効果時間を永久からマジスレテンプレ準拠に
・攻撃を受けた時にペットの支援攻撃を受けられないよう修正(コメントアウトしただけ)
　これはVIT型にペットを付けて放置するだけで自動でレベル上げができるのを
　防ぐための暫定的な処置です
	(map/)
	skill.c
		skill_castend_nodamage_id()、skill_addisional_effect()修正
		skill_status_change_start()修正
	pc.c
		pc_damage() 修正

--------------
//0735 by ぽぽぽ

・敵を倒してレベルが上がったときPT公平範囲のチェックをするようにした。
・オートカウンター仮実装。
　向きや射程チェックはしていません。またタイミングがおかしいかもしれません。
　MOBスキルとして使うときはターゲットをselfにしてください。
	(conf/)
	battle_athena.conf項目追加
	(doc/)
	conf_ref修正
	(map/)
	battle.h
	battle.c
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack()
		battle_config_read()修正
	pc.c
		pc_checkbaselevelup()、pc_attack_timer()修正
	skill.c
		skill_castend_nodamage_id()、skill_status_change_start()修正
	clif.c
		clif_parse_WalkToXY()修正
	mob.c
		mob_attack()修正

--------------
//0734 by 死神

・player_skillup_limitの処理修正と細かい修正。
・player_skillup_limitがyesの場合skill_tree.txtで設定されてるその下位職業の
スキルツリーを使いますのでその職業では無くなるはずのスキルが出ることが
ありますがこれは仕様でありバグではありません。バグ報告されても無視します。
	(doc/)
	conf_ref.txt 修正。
	(char/)
	char.c
		mmo_char_sync_timer()、do_init() 修正。
	inter.c
		inter_init() 修正。
		inter_save_timer() 削除。
	(map/)
	pc.c
		pc_calc_skilltree() 修正。
		pc_resetskill() 修正。

--------------
//0733 by 死神

・バグ修正と細かい修正。
・死んだ後にすぐにセーブポイントに戻らずにしばらく放置してると、
放置してる時間によって経験値が減少するバグ修正。(未テスト)
・mob_availe.txtで設定したモンスターにモンスター情報を使うち鞍落ちする問題修正。
・battle_athena.confに項目追加。
・その他細かい修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	clif.c
		clif_skill_estimation()、clif_parse_Restart() 修正。
	pc.c
		pc_setrestartvalue()、pc_makesavestatus() 修正。
		pc_read_gm_account()、pc_calc_skilltree() 修正。
		pc_calc_skillpoint() 追加。
	map.c
		map_quit() 修正。
	mob.c
		mob_damage() 修正。
	skill.c
		skill_unit_timer_sub()、skill_unit_timer() 修正。
	battle.h 修正。
	battle.c
		battle_config_read() 修正。

--------------
//0732 by Kalen

・npc_town_kafra.txtの全面見直し
	カプラ利用券の廃止
	倉庫利用料を本鯖(jRO)と同一価格に調整
	カート使用料を本鯖(jRO)と同一価格に調整
	ポイント参照変更
	ジュノーのセーブポイント修正
	アマツのセーブポイント修正

--------------
//0731 by ぽぽぽ

・服の色を保存するかbattle_athena.confで選択できるように。
　弊害があるので保存しないようにと書いてあったのでデフォルトでは保存しません。
・スクリプト追加
　strcharinfo(1)	自分のパーティー名を取得します。
　strcharinfo(2)	自分のギルド名を取得します。
　getcharid(1)		自分のパーティーIDを取得します。
　getcharid(2)		自分のギルドIDを取得します。
　getpartyname(ID)	IDで指定したパーティーの名前を取得します。
　getguildname(ID)	IDで指定したギルドの名前を取得します。
	(map/)
	battle.h
	battle.c
		battle_config_read()修正
	pc.c
		pc_makesavestatus()修正
	script.c
		buildin_strcharinfo()修正
		buildin_getcharid()、buildin_getpartyname()、buildin_getpartyname_sub()
		buildin_getguildname()、buildin_getguildname_sub()追加

--------------
//0730 by ぴざまん

・ストームガストの凍結時間を本鯖にあわせて修正(スキルレベルに関係なく一定の凍結時間(10秒)になります)
・スタン、暗闇、沈黙、毒の状態異常時間の「継ぎ足し」ができないように修正
・状態異常が掛かりにくすぎてたのでMOBの状態異常耐性を緩和(また調整するかも)
	(map/)
	skill.c
		skill_castend_nodamage_id()、skill_addisional_effect()修正
		skill_status_change_start()修正

--------------
//0729 by DRG

・カートレボリューションがカートなしで使えた不具合の修正
・カートレボリューションがJOBLV30で覚えれたのを修正
	(conf/)
	npc_event_skillget.txt
		カートレボリューション項修正
	(map/)
	skill.c
		skill_check_condition()修正

--------------
//0728 by ぽぽぽ

・職が変わってもギルドの職業欄が更新されない不具合の修正。

	(char/)
	inter.c
		パケット長リスト修正。
	int_guild.c
		mapif_guild_memberinfoshort()、mapif_parse_GuildChangeMemberInfoShort()、
		inter_guild_parse_frommap()修正
	(map/)
	intif.h
	intif.c
		intif_guild_memberinfoshort()、intif_parse_GuildMemberInfoShort()
		intif_parse()修正
	guild.h
		guild_send_memberinfoshort()、guild_recv_memberinfoshort()修正
	

--------------
//0727 by 聖

・武器研究スキルによってホルグレンなどの精錬NPCが
　正常に動作しない問題を修正。

	(map/)
	pc.c
		pc_percentrefinery() 修正。

--------------
//0726 by 胡蝶蘭

・mob_skill_db2.txtがあればmob_skill_db.txtをオーバーライドするように修正
	オリジナルのMOB使用時や、現行MOBの使用スキルを変更したい場合に。

・mob_skill_db.txtでmob_idの次のダミー文字列が"clear"だった場合、
  そのMOBのスキルを初期化する機能追加。
  	・mob_skill_db2.txtであるMOBのスキルを完全に書き換えるときに使用して
	  ください。
	・clearしなかった場合はmob_skill_db.txtのものに追加されます。
  
	mob.c
		mob_readskilldb()修正


・アイテム名/MOB名が全角12文字（24バイト）あるアイテム/MOBが、
  @コマンドで取り寄せ/召喚できない問題修正。
  	mob.c
		mobdb_searchname()修正
	itemdb.c
		itemdb_searchname_sub()修正

・現在時刻でイベントを起こす「時計イベント」機能を追加
	・OnInitと同じようにそれぞれのNPCで、On～で始まるラベルを定義します。
		OnMinute??	：毎時、??分にイベントを起こします。(0-59)
		OnHour??	：毎日、??時にイベントを起こします。(0-23)
		OnClock????	：毎日、??時??分にイベントを起こします。
		OnDate????	：毎年、??月??日にイベントを起こします。
	・詳しくは npc_test_ev.txt を参照
	
	(conf/)
	npc_test_ev.txt
		内容追加
	(map/)
	npc.c
		色々修正

・その他
	clif.c
		コンパイル警告が出ないように修正

--------------
//0725 by 死神

・鯖落ちバグ修正。
・モンスターにイベントが設定されていて自殺やなにかでダメージを与えた
物がない場合鯖落ち確定なのでそのマップにあるプレイヤーを利用して
イベントスクリプトを実行するように変更。
	(map/)
	makefile 修正。
	mob.c
		mob_timer()、mob_damage() 修正。

--------------
//0724 by 死神

・バグ修正と安定化の為の修正。
・ペットの攻撃でイベントが処理されず鯖落ちになる問題修正。(未テスト)
・モンスターの大量発生で鯖が落ちる問題修正。(モンスターを10000匹を呼んで
魔法で倒すことを5回程テスト。ただ動かないやつのみ。)
・取り巻きがボスと一緒に死ぬ時アイテムを落とさないように変更。(未テスト)
・battle_athena.confのpc_skillfleeをplayer_skillfreeに変更して処理を変更。
・アイスウォールにskill_unit_settingを使うスキルで攻撃できないように修正。
・その他細かい修正少し。安定化されたかどうかはまだわかりませんがXP1800+、512M、モンスター配置50%で10000匹召喚して異常なかったので大丈夫になったと思います。大丈夫じゃなくても責任はとれませんが...
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	mob.h 修正。
	mob.c
		mob_timer()、mob_deleteslave_sub()、mob_damage() 修正。
	npc.c
		npc_event() 修正。
	skill.c
		skill_area_sub()、skill_unit_onplace()、skill_castend_nodamage_id() 修正。
	clif.c
		clif_parse_GMKick() 修正。
	battle.h
	battle.c
		battle_damage()、battle_check_target()、battle_config_read() 修正。
	pc.c
		pc_calc_skilltree()、pc_checkskill() 修正。
	map.h 修正。
	map.c
		map_foreachinarea()、map_foreachinmovearea() 修正。
		map_foreachobject() 修正。

--------------
//0723 by DRG

・0719の修正
	(map/)
	pc.c		pc_calc_skilltree()修正

--------------
//0722 by パイン

・gcc 2.29系列でもコンパイルが通るように修正。
　これは以前にも直したはずなのですが、なぜか元に戻っていましたので
　皆さん注意をお願いします。
　あと、gcc3系列なら定数はどこに書いても問題ないのですが、
　gcc2.29系列では「必ずブロック要素の一番最初」に書かないとコンパイルが
　通りませんのでこちらもご注意願います。

コンパイルが通る例
void hoge() {
    const char booboo = 1;
    …

コンパイルが通らない例
void hoge() {
    …
    const char booboo = 1;
    …

	(map/)
	skill.h		マクロを修正
	skill.c		skill_addisional_effect()修正

--------------
//0721 by 聖

・ボスにレックスデビーナが効いた問題を修正。
・ボスにカードによる状態異常が効かなかった問題を修正。
　本鯖ではマリナカード等でオークヒーローなどを殴ると時々凍結します。
　(結構微妙な実装方法なので、何か問題があった場合
　 その辺詳しい方おりましたら修正してやってください(^^; ))

--------------
//0720 by 胡蝶蘭

・PCにIWを重ねるとMOBが攻撃してこない問題を修正
	・IWに重なっていても、隣接可能ならMOBが近寄ってきます
	・どんな地形にいても、隣接しているなら攻撃可能になります
	・ただし、MOBが遠距離攻撃可能で、攻撃範囲内にPCがいても、
	  隣接不可能なら攻撃してきません。これの解決はかなり面倒なので。

	mob.c
		mob_can_reach()修正
	battle.c
		battle_check_range()修正

--------------
//0719 by DRG

・下位スキルがない場合は上位スキルがふれないようにしました。
　battle_athena.confのskillfleeで設定可能です。
　下位スキルがないまま上位スキルをふった状態で、このオプションを使う場合はスキルリセットする必要があります。
　一般アカにスキルリセットを解放したいときに使ってやって下さい。
	(conf/)
	battle_athena.conf
	(map/)
	battle.c
	battle.h
	pc.c		pc_calc_skilltree(),pc_checkskill()修正

--------------
//0718 by 死神

・色々と修正。
・毒によって防御が減るように変更。(HPはまだ減りません。)
・アイスウォールに攻撃できるように変更。(今は全ての攻撃に当たります。)
ただ鞍のバグらしくアイスウォールをクリックすると鞍から0x89パケットが30回以上連続で送ってくることが起こりますが原因は不明です。多分鞍のバグだと思いますが...)
・戦闘に関わる計算等を修正。
・ゼニが増えるバグ修正。(多分これでこのバグはなくなると思いますがどうなのか報告をお願いします。)
・二刀流の左手武器の種族、属性、Sizeのダメージ補正を右手武器に適用するかどうかを設定できるように変更。
・その他修正はしたはずですが覚えてません。(修正してない物もありますがdiff当ての途中でどれを作業したのかを忘れたので...)
	(common/)
	mmo.h 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_get_baseatk()、battle_get_speed()、battle_get_adelay() 追加。
		battle_get_amotion() 、battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_get_atk()、battle_get_atk_()、battle_get_atk2() 修正。
		battle_get_attack_element()、battle_get_attack_element2() 修正。
		battle_get_str()、battle_get_agi()、battle_get_vit()、battle_get_int() 修正。
		battle_get_dex()、battle_get_luk()、battle_get_flee() 修正。
		battle_get_flee2()、battle_get_hit()、battle_get_critical() 修正。
		battle_get_def()、battle_get_def2()、battle_get_mdef() 修正。
		battle_get_element()、battle_check_target()、battle_check_range() 修正。
		battle_weapon_attack()、battle_config_read() 修正。
	clif.c
		clif_skill_estimation()、clif_mob0078()、clif_mob007b() 修正。
	pc.c
		pc_attack_timer()、pc_attack()、pc_calcstatus()、pc_payzeny() 修正。
		pc_getzeny() 修正。
	npc.c
		npc_buylist()、npc_selllist() 修正。
	pet.c
		pet_attack()、pet_randomwalk()、pet_ai_sub_hard() 修正。
	mob.h 修正。
	mob.c
		calc_next_walk_step()、mob_attack()、mobskill_castend_id() 修正。
		mobskill_use_id()、mobskill_use_pos()、mob_ai_sub_hard() 修正。
		mob_damage()、mob_changestate() 修正。
		mob_get_adelay()、mob_get_speed() 削除。
	skill.h 修正。
	skill.c
		skill_unitsetting()、skill_unit_ondamaged()、skill_unit_timer_sub() 修正。
		skill_unit_timer()、skill_area_sub()、skill_unit_onplace() 修正。
		skill_status_change_start() 修正。
	chat.c 修正。
	makefile 修正。
	chrif.c 修正。
	guild.c 修正。
	itemdb.c 修正。
	map.c 修正。
	party.c 修正。
	script.c 修正。
	path.c 修正。

--------------
//0717 by 聖

・大量にモンスターを召還して一度に倒すとmap-serverが落ちる問題を修正。
　(カホを100体ずつ召還して50回テストをしたので恐らく大丈夫だと思います。)
・その他結構細かい修正
	(common/)
	mmo.h
	(map/)
	chat.c
	chrif.c
	clif.c
	guild.c
	itemdb.c
	map.c
	mob.c
	npc.c
	party.c
	path.c
	pc.c
	pet.c
	script.c
	skill.c
	skill.h

--------------
//0716 by 聖

・精錬成功率に対してBSの武器研究が正しく適用されていなかった問題を修正。
	(map/)
	pc.c
		pc_percentrefinery() 修正。

--------------
//0715 by 死神

・マップサーバーから表示される物を表示するかどうかの設定ができるようにしました。スキル表示だけでもなくしてやるとサーバーがかなり楽になったりもします。
開発やバグトレースの時は表示することをお勧めします。
・その他細かい修正。
・修正した所を全て書けないのでファイルだけ。
	(doc/)
	conf_ref.txt
	(conf/)
	battle_athena.conf
	(map/)
	makefile
	skill.c
	script.c
	pet.c
	pc.c
	path.c
	party.c
	npc.c
	itemdb.c
	intif.c
	guild.c
	chat.c
	battle.h
	battle.c
	chrif.c
	atcommand.c
	clif.c
	mob.c
	map.c

--------------
//0714 by 死神

・細かい修正。
・シールドブーメランで盾の重量と精錬によってダメージが増えるように修正。精錬ダメージを足す時適用でダメージ+重量+盾精錬*4(この4はrefine_db.txtの防具の過剰精錬ボーナスを使ってるので変更可能です。)になります。
・スキルによる吹き飛ばし処理で0x88パケットを使っていましたがそのパケットの優先順位がかなり低いらしく後で来るパケットによって無視されることもあるようなのでプレイヤーだけに適用してモンスターには0x78を使うように変更しました。
でも位置ずれは完全になくならないようです。(恐らく鞍のバグだと思います。鯖の
座標を確認してみましたが鯖の方は問題がありませんでした。)
プレイヤーの場合0x78(PACKETVERが4以上なら0x1d8)が使えません。分身を作ってしまうので...
・バグ報告スレッド2 の47を取り込みました。
・その他修正した所少しあり。
	(db/)
	refine_db.txt 修正。
	item_db.txt 修正。
	(map/)
	battle.c
		battle_stopattack()、battle_stopwalking() 修正。
		battle_get_attack_element2()、battle_calc_pc_weapon_attack() 修正。
		battle_weapon_attack() 修正。
	path.c
		path_blownpos() 修正。
	pc.h 修正。
	pc.c
		pc_stop_walking()、pc_damage() 修正。
		pc_getrefinebonus() 追加。
	mob.c
		mob_damage() 修正。
	pet.c
		pet_target_check()、pet_stop_walking()、pet_performance() 修正。
	skill.c
		skill_attack()、skill_blown()、skill_status_change_start() 修正。
		skill_castend_damage_id() 修正。
	makefile 修正。

--------------
//0713 by ぽぽぽ

・mob_avail.txt追加。item_avail.txtと同様の指定でモンスターの見た目を他のIDのものに変更します。
　モンスターのID以外を指定したりするとPCやNPCの姿をしたMOBに一方的に攻撃される場合があるので注意。
	(db/)
	mob_avail.txt 追加。
	(map/)
	clif.c
		clif_mob0078()、clif_mob007b() 修正。
	mob.h 修正。
	mob.c
		mob_readdb_mobavail()、mob_get_viewclass()追加。
		do_init_mob()、mob_readdb() 修正。

--------------
//0712 by 死神

・シールドチャージ、シールドブーメラン実装。
・オートガードはとりあえずエフェクトが出るように変更しました。
・0708で書き忘れ。ディフェンダーを使った時ASPDと移動速度は20%低下します。
本鯖で低下するのは確かのようですがどれぐらい下がるのかはさっぱりわかりまんので...
・その他細かい修正。
	(db/)
	cast_db.txt 修正。
	skill_db.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_calc_damage()、battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_calc_magic_attack()、battle_calc_misc_attack() 修正。
	skill.c
		skill_additional_effect()、skill_attack()、skill_castend_nodamage_id() 修正。
		skill_check_condition()、skill_status_change_start() 修正。
		skill_castend_damage_id() 修正。
	pc.h 修正。
	pc.c
		pc_calcstatus()、pc_checkallowskill()、pc_unequipitem() 修正。

--------------
//0711 by npc

・鉱石製造エフェクトの修正
・スクリプト埋め込み変数にHp,MaxHp,Sp,MaxSpを追加(読み込みのみ)
	(map/)
	skill.c
		skill_produce_mix()修正。
	pc.c
		pc_readparam()修正。
	(db/)
		const.txt 修正。

--------------
//0710 by 胡蝶蘭

・名前に半角スペースが入ったパーティーを作成したとき、および、
  半角スペースが入った名前のPCをパーティメンバにしたとき、
  party.txtが正しく読み込めなくなる問題を修正

	(char/)
	int_party.c
		inter_party_fromstr()修正

・Message of the Day 機能追加
	・ログインしたユーザーにMOTDを表示させることが出来ます。
	・map-server.exe実行時のカレントディレクトリ（help.txtと同じ
	  ディレクトリ）にmotd.txtを作ると表示します。
	・MOTDが表示されるタイミングは、
	  「マップサーバーにログインした直後の、マップロード完了時」です。
	  つまり、ログイン直後、キャラセレ直後および、
	  マップサーバー間移動の時(マップサーバーの分散を行っている場合のみ)
	  のマップロードが終わった時に表示されます。
	・表示方法はhelp.txtと同じで普通のメッセージとして送信します。
	  （ギルド告知メッセージは文字数制限があり、GMアナウンスは長時間
	    画面の上部に表示されてしまうため）
	・会話と区別がつくように、"< Message of the Day >"、"< End of MOTD >"
	  の文で上下を囲います。
	
	(map/)
	pc.c
		pc_authok()修正


--------------
//0709 by ぽぽぽ

・スクリプトにemotion追加
　emotion n;と使うとNPCがエモを出します。nは0～33が使用可能。
・精錬と街ガイドのNPCを本鯖の台詞に合わせて修正。
	(conf/)
	npc_town_refine.txt、npc_town_guide.txt 修正。
	(map/)
	script.c
		buildin_emotion() 追加。

--------------
//0708 by 死神

・スキルキャストキャンセル、ディフェンダー、オートガード実装。
・オートガードの場合ガードしてもエフェクトは出ません。ミスになるだけです。本鯖の方は表示されるかどうかもわからないしパケット情報もないので...
・ディフェンダーは未テスト。bLongAtkDefを使ってるのでホルンカードのようにbLongAtkDefを上げる物を装備して使うと遠距離物理攻撃を全て無効にできます。(これも本鯖の仕様がどうなのかはわかりません。)
・その他細かい修正。
	(db/)
	cast_db.txt 修正。
	(map/)
	map.h 修正。
	map.c
		map_quit() 修正。
	skill.h 修正。
	skill.c
		skill_castend_nodamage_id()、skill_use_id()、skill_check_condition() 修正。
		skill_castend_id()、skill_castend_nodamage_id()、skill_castcancel() 修正。
	pc.c
		pc_calcstatus()、pc_setpos()、pc_damage() 修正。
	battle.c
		battle_calc_damage()、battle_damage() 修正。
	clif.c
		clif_parse_UseSkillToId()、clif_parse_UseSkillToPos() 修正。
	mob.c
		mob_damage() 修正。
	itemdb.c
		itemdb_searchrandomid() 修正。

--------------
//0707 by 死神

・0705の阿修羅覇鳳拳のバグ修正。
	(db/)
	skill_db.txt
	(map/)
	skill.c
		skill_castend_id()
		skill_castend_pos()
	battle.c
		battle_calc_pc_weapon_attack()
	clif.c
		clif_parse_UseSkillToId()

--------------
//0706 by kalen
・修正
	conf/npc_warp_umbala.txt

--------------
//0705 by 死神

・色々と修正。
・プレイヤーのクリティカル計算にバグがあったので修正。
・爆裂波動の処理修正。
・モンクのコンボを修正。
・阿修羅覇鳳拳の使用によってマップ鯖の無限ループバグ修正。(これかなり致命的な物だったようです。)
・コンボで使う阿修羅覇鳳拳は敵をクリックする必要がないように修正。
・猛龍拳で敵を吹き飛ばす距離を5セルに変更。よってコンボで使う阿修羅覇鳳拳は距離チェックをしません。5セル飛ばされた敵は阿修羅覇鳳拳の射程から離れたわけなので距離チェックなしで発動します。(本鯖の仕様なんて知りません。)
・マップの名前を16byteから24bytesに変更。(大した意味はありませんが安全の為の物です。)
・ウェディングキャラによる鞍落ちを防ぐ為に修正。
・その他少し修正。(テストは殆んどしてません。)
	(conf/)
	battle_athena.conf 修正。
	(db/)
	skill_db.txt 修正。
	(common/)
	mmo.h 修正。
	(doc/)
	conf_ref.txt 修正。
	item_bonus.txt 修正。
	(map/)
	battle.h 修正。
	battle.c
		battle_get_flee2()、battle_calc_pet_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_config_read() 修正。
	skill.h 修正。
	skill.c
		skill_status_change_start()、skill_castend_damage_id() 修正。
		skill_check_condition()、skill_use_id()、skill_blown() 修正。
		skill_castend_map()、skill_unit_onlimit()、skill_attack() 修正。
	pc.c
		pc_attack_timer()、pc_setpos()、pc_setsavepoint() 修正。
		pc_movepos()、pc_calcstatus()、pc_bonus() 修正。
	clif.h 修正。
	clif.c
		clif_set0078()、clif_set007b() 修正。
		clif_updatestatus()、clif_initialstatus()、clif_parse_UseSkillToId() 修正。
		clif_skillinfo() 追加。
	map.h 修正。
	map.c
		map_setipport()、map_addmap() 修正。
	その他抜けた所少しあり。

--------------------
//0704 by kalen

・Umbala Warp追加
	conf/npc_warp_umbala.txt

--------------------
//0703 by いど

・サーバーsnapshot

--------------
//0702 by ぽぽぽ

・ファーマシーのエフェクトを本来のものに変更
・スクリプトでの埋め込み変数にBaseExp,JobExp,NextBaseExp,NextJobExp追加
	(map/)
	skill.c
		skill_produce_mix() 修正。
	pc.c
		pc_readparam()、pc_setparam() 修正。
	(db/)
		const.txt 修正。

--------------
//0701 by ぴざまん

・ステータス異常判別式導入。各ステータス異常の発動率がVIT/INT/MDEFに影響するようになります。持続時間短縮はまた今度で_|￣|○
・不死に凍結が効いたバグ修正。
	(map/)
	skill.c
		skill_additional_effect()、skill_castend_nodamage_id() 修正。

--------------
//0700 by 南

・697のバグ修正。
　　　　(db/)
        mob_db.txt

--------------
//0699 by 死神

・装備のボーナスクリティカルは自分の間違いだったのでbCriticalRateをbCriticalに変更。それと0695で書き忘れですがASPDを上げるカードや装備の一部をbAspdAddRateからbAspdRateに変更しました。みすとれ巣のシミュレーターによるとドッペルカードは複数でも一つしか適用されないみたいだったので。
	(db/)
	item_db.txt

--------------
//0698 by 死神

・一部のキャラに重量が０になってカプラなど何もＰＣ，ＮＣＰが表示されなくなるバグ修正。(それだけ)
	(common/)
	mmo.h 修正。
	(map/)
	clif.c
		clif_updatestatus() 修正。
	pc.c
		pc_calcstatus() 修正。

--------------
//0697 by 南

・mob_db修正
　ドロップを中心に修正。
　　　　(db/)
        mob_db.txt

--------------
//0696 by 死神

・バグ修正。
・テレポートやワープ等の時スキルユニットから抜ける処理が入って
なかったのでSAFETYWALL等によって鯖落ちが起こったようです。(確か報告も
あったと思いますが...) よって修正はしましたが確認はしてません。報告を
お願いします。
・スキルによる吹き飛ばし処理をちょっと修正とモンスターのコードを少し修正。
多分変になったことはないと思いますが変だったら報告してください。
・その他細かい修正。
	(map/)
	skill.h 修正。
	skill.c
		skill_blown()、skill_attack()、skill_unit_move() 修正。
		skill_castend_nodamage_id()、skill_castend_damage_id() 修正。
		skill_unit_out_all()、skill_unit_out_all_sub() 追加。
	mob.c
		mob_stop_walking()、mob_spawn()、mob_warp() 修正。
		mob_can_move()、mob_changestate() 修正。
	map.h 修正。
	pc.c
		pc_setpos() 修正。
	battle.c
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack() 修正。

--------------
//0695 by 死神

・少し修正。
・プレイヤーの基本パラメータを2byteに拡張。
・item_db.txtをラグナゲートの説明に合わせて修正。
・bAddEffとbResEffの確率を百分率から万分率に変更。
・スクリプトstatusupとstatusup2追加。
statusup bStr; のように使って機能はステータスポイントを減らして
基本パラメータを1上げる。
statusup2 bInt,n; のように使って機能はステータスポイントを減らさずに
基本パラメータをn上げる。
・その他細かい修正。
	(conf/)
	battle_athena.conf 修正。
	(db/)
	item_db.txt 修正。
	const.txt 修正。
	(doc/)
	item_bonus.txt 修正。
	conf_ref.txt 修正。
	(common/)
	mmo.h 修正。
	(char/)
	char.c
		mmo_char_send006b()、parse_char() 修正。
	(map/)
	map.h 修正。
	clif.h 修正。
	clif.c
		clif_initialstatus()、clif_updatestatus() 修正。
	pc.h 修正。
	pc.c
		pc_bonus()、pc_calcstatus()、pc_equippoint()、pc_equipitem() 修正。
		pc_jobchange()、pc_checkbaselevelup()、pc_statusup() 修正。
		pc_statusup2() 追加。
	battle.h 修正。
	battle.c
		battle_calc_pet_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pc_weapon_attack()、battle_config_read() 修正。
	skill.c
		skill_additional_effect()、skill_status_change_start() 修正。
	script.c
		buildin_statusup()、buildin_statusup2() 追加。
	atcommnad.c 修正。

--------------
//0694 by 死神

・バグ修正と細かい修正。
・bCriticalRateが正しく適用されなかった問題修正。
・ペットによるステータスボーナス追加。ステータスボーナスは装備の
スクリプトによって設定します。ただペットによるボーナスはカードによる物と同じ扱いをします。そして属性の適用は一番優先順位低いです。今は何も入ってませんが...
	(conf/)
	battle_athena.conf 修正。
	(db/)
	const.txt 修正。
	pet_db.txt 修正。
	(doc/)
	item_bonus.txt 修正。
	conf_ref.txt 修正。
	db_ref.txt 修正。
	(map/)
	map.h 修正。
	map.c
		map_quit() 修正。
	battle.h 修正。
	battle.c
		battle_calc_pc_weapon_attack()、battle_weapon_attack() 修正。
		battle_calc_magic_attack()、battle_calc_misc_attack() 修正。
		battle_config_read() 修正。
	pc.c
		pc_calcstatus()、pc_bonus()、pc_setpos()、pc_authok() 修正。
		pc_damage()、pc_autosave_sub() 修正。
	pet.h 修正。
	pet.c
		pet_hungry()、pet_birth_process()、pet_recv_petdata()、pet_food() 修正。
		pet_return_egg()、pet_ai_sub_hard()、read_petdb() 修正。
	clif.c
		clif_sendegg()、clif_parse_LoadEndAck() 修正。
	atcommand.c 修正。
	makefile 修正。

--------------
//0693 by 胡蝶蘭

・SC_*の列挙表をリナンバリング
	クライアントに通知するのを64未満から128未満に増やした
	パケット情報に合うようにリナンバー
	StatusChangeの配列を128から192に増やしたのでメモリ使用量が増えます。
	
	(db/)
	const.txt
		SC_* の数値を変更
	(map/)
	skill.h
		SC_* の列挙の数値を変更
	map.h
		MAX_STATUSCHANGEを128から192に増やした
	skill.c
		skill_status_change_start(),skill_status_change_end(),
		skill_status_change_clear()の通知処理を変更

・演奏/ダンスの処理を変更
	演奏/ダンス中かどうかをSC_DANCINGで判定するように変更
	（判定処理が多少高速化されたはず）
	ワープ(マップ移動や蝿など)すると演奏/ダンスを中断するように変更
	
	skill.h/skill.c
		skill_check_dancing()削除、skill_stop_dancing()追加
		skill_delunitgroup(),skill_initunitgroup()変更
		skill_status_change_start()変更
		skill_castend_nodamage_id()変更
		書き損じがあるかも・・
	pc.c
		pc_calcstatus(),pc_setpos(),pc_damage()変更

・不協和音スキルの修正
	(db/)
	skill_db.txt
		不協和音スキルのHIT数修正
	(map/)
	skill.c
		skill_status_change_timer()変更
	battle.c
		battle_calc_misc_attack()修正

--------------
//0692 by 胡蝶蘭

・アドリブスキルが使用できない問題修正（skill_dbの添付し忘れ）
	(db/)
	skill_db.txt
		アドリブの消費SPを1に修正

・mob_db2.txtがあればmob_db.txtにオーバーライドするように
	オリジナルmobを作ってる人は使うと便利かもしれません。

	mob.c
		mob_readdb()

・鯖落ちバグ報告時のためのスタックバックトレースログ所得方法を紹介
	鯖落ちバグの報告時に、この情報をコピペすると開発者が喜びます。
	Cygwinでcoreの吐かせる方法も紹介してます。

	(doc/)
		coredump_report.txt

--------------
//0691 by 胡蝶蘭

・item_db2.txtがあればitem_db.txtにオーバーライドするように
	オリジナルアイテムを作ってる人は使うと便利かもしれません。

	itemdb.c
		itemdb_readdb()修正

・演奏/ダンス系スキル仮実装
	・演奏/ダンス中は移動が遅く、スキルも使えないようになりました
	・アドリブスキルで演奏/ダンスを中断できるようになりました
	・演奏/ダンスは石化などの異常、MHPの1/4以上のダメージで中断します
	・キャラクターグラフィックは演奏/ダンスしません
	・演奏/ダンス中のSP消費は未実装です
	・移動しても効果範囲はついてきません
	・重複しても不協和音などに変化しません
	・エフェクトが出ても効果は未実装のものがあります
	・ほとんど未テストなので多数の不都合があると思います

	skill.h
		SC_* の列挙表を修正
	skill.c
		skill_check_dancing()追加
		SkillStatusChangeTable[]修正
		skill_unit_onout(), skill_status_change_start(),
		skill_status_change_timer(),skill_unitsetting(),
		skill_castend_id(),skill_castend_pos(),skill_castend_map(),
		skill_castend_nodamage_id()修正
		その他は忘れました
	pc.c
		pc_calcstatus(),pc_damage()修正

--------------
//0690 by 波浪

・細かい修正
	(db/)
	item_db.txt 錐とメギンギョルドのbonusを修正。
	(doc/)
	item_bonus.txt 修正。

--------------
//0689 by 死神

・倉庫バグ修正と細かい修正。
	(map/)
	pc.c
		pc_modifybuyvalue()、pc_modifysellvalue() 修正。
	storage.c
		storage_storageopen() 修正。
		storage_storage_quit()、storage_storage_save() 修正。

--------------
//0688 by 聖

・ディスカウント、コムパルションディスカウント、オーバーチャージが適用されなかった問題を修正。
	(map/)
	pc.c
		pc_modifybuyvalue() 修正。
		pc_modifysellvalue() 修正。

--------------
//0687 by 死神

・少し修正。
・battle_athena.confに項目追加。(詳しいことはconf_ref.txtで)
・item_avail.txtの処理を変更。アイテムIDの後に0を入れると今まで通りに使用不可能になるが0以外の数値を入れると使用不可能ではなくその数値をアイテムのIDとして見た目だけをそれに変更します。よって鞍落ちアイテムを別の物に表示して鞍落ちを防ぐことができます。(表示だけ変えて鯖の処理は本当のアイテムIDの物として認識します。修正は全てしたと思いますが抜けた所があるかも知りませので見た目変更したアイテムで鞍落ちが起こったら報告してください。) 鯖の処理はこれが限界です。(少なくとも自分には) アイテムが同じ物が二つ表示されて間違い安いとかどうこうとかの文句を言いたい人は鞍作れよ。以上。
・ジルタスとアリスのコマントアウト解除。item_avail.txtで卵をルビーとアクアマリンで表示して捕獲アイテムも他の物に表示するように変更しています。
・ダメージ計算のバグ修正。(大した物じゃありませんが弓だけちょっと問題があったようです。)
・青箱等のアイテムで得た装備品は未鑑定になるように変更。
・装備ボーナスの内部処理修正と少し変更。(詳しいことはitem_bonus.txtで)
・キャラ鯖にテータを送る時キャラ、倉庫、ペットのテータを同時に送るように変更。(キャラ鯖とマップ鯖の間の転送量が増えるかも知りれませんがデータを同期化の為です。)
・FWの動作間隔を0.25秒から0.1秒に変更。(これで摺り抜は少し減るはずです。)
・カートレボリュションでどんな状態異常もかからないように変更。
	(conf/)
	battle_athena.conf 修正。
	(db/)
	const.txt 修正。
	item_avail.txt 修正。
	pet_db.txt 修正。
	(doc/)
	conf_ref.txt 修正。
	item_bonus.txt 修正。
	(map/)
	map.h 修正。
	map.c
		map_quit() 修正。
	battle.h 修正。
	battle.c
		battle_calc_pc_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_get_dmotion()、battle_config_read() 修正。
	pc.c
		pc_calcstatus()、pc_bonus()、pc_bonus3()、pc_setpos() 修正。
		pc_makesavestatus()、pc_autosave_sub()、pc_modifybuyvalue() 修正。
		pc_modifysellvalue()、pc_stop_walking() 修正。
	skill.c
		skill_additional_effect()、skill_unitsetting() 修正。
	clif.c
		clif_buylist()、clif_selllist()、clif_set009e()、clif_set0078() 修正。
		clif_set007b()、clif_additem()、clif_itemlist()、clif_equiplist() 修正。
		clif_storageitemlist()、clif_storageequiplist()、clif_changelook() 修正。
		clif_arrow_create_list()、clif_useitemack()、clif_tradeadditem() 修正。
		clif_storageitemadded()、clif_getareachar_item() 修正。
		clif_skill_produce_mix_list()、clif_cart_additem()、clif_cart_itemlist() 修正。
		clif_cart_equiplist()、clif_vendinglist()、clif_openvending() 修正。
		clif_produceeffect()、clif_sendegg()、clif_pet_equip()、clif_mvp_item() 修正。
		clif_pet0078()、clif_pet007b() 修正。
	itemdb.h 修正。
	itemdb.c
		itemdb_searchrandomid()、itemdb_search()、itemdb_readdb() 修正。
		itemdb_read_itemavail()、itemdb_read_itemvaluedb() 修正。
		itemdb_equippoint() 削除。
	storage.h 修正。
	storage.c
		storage_storage_quitsave() ->storage_storage_quit()に変更と修正。
		storage_storageclose() 修正。
	atcommand.c 修正。
	pet.c
		pet_change_name()、pet_equipitem()、pet_unequipitem() 修正。
		pet_birth_process()、pet_return_egg() 修正。
	script.c
		buildin_getitem() 修正。
	mob.c
		mob_stop_walking() 修正。
	makefile 修正。

--------------
//0686 by 聖

・細かい修正。
	(map/)
	pc.h 修正。

--------------
//0685 by 波浪

・0683、0684でのbonusの追加にともなってitem_db.txtを修正
・他色々修正
	(db/)
	item_db.txt 修正。
	(doc/)
	item_bonus.txt 修正。

--------------
//0684 by 死神

・細かい修正。
・死んだふりの時スキルとアイテムが使えないように変更。
・bInfiniteEndure追加。機能は無限インデュア。
・ダメージ表示の処理少し変更。
	(db/)
	const.txt 修正。
	(doc/)
	item_bonus.txt 修正。
	(map/)
	map.h 修正。
	pc.c
		pc_calcstatus() 修正。
		pc_equipitem()、pc_unequipitem() 修正。
	clif.c
		clif_parse_UseItem()、clif_parse_UseSkillToId() 修正。
		clif_parse_UseSkillToPos()、clif_parse_UseSkillMap() 修正。
		clif_damage()、clif_skill_damage()、clif_skill_damage2() 修正。
		clif_parse_LoadEndAck() 修正。
	skill.c
		skill_status_change_timer() 修正。

--------------
//0683 by 死神

・バグ修正とbonus追加。
・倉庫バグ、属性バグ修正とその他のバグ修正。
・スクリプトbonus3追加。今はbAddMonsterDropItemだけが対応になっています。
・bonus bRestartFullRecover;n;等でnは無意味だけど消すのはちょっとまずいですので0にして入れた方がいいです。bonusは2つの数値が必要なスクリプトなので。
・bDefRatioAtkを防御無視に変更。
・0677で書き忘れ。
・武器の属性適用優先順位を製造>カード>武器に変更。製造が最優先です。(属性がある時に話です。属性がない場合属性ある物に上書きされたりはしません。)
・装備で適用される効果の優先順位を右手>左手>体>頭上>頭中>頭下>ローブ>靴>アクセサリー1>アクセサリー2>矢に設定。(本鯖仕様がどうなのか分かることができそうな物でもないのでアテナの仕様と言うことで。) 右手が最優先です。
・武器の射程を右手と左手の武器の中で長い物を適用するように変更。
	(db/)
	const.txt 修正。
	(doc/)
	item_bonus.txt 修正。
	(map/)
	map.h 修正。
	battle.c
		battle_calc_pc_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_calc_pet_weapon_attack()、battle_calc_magic_attack() 修正。
		battle_damage() 修正。
	pc.c
		pc_autosave_sub()、pc_calcstatus() 修正。
		pc_bonus()、pc_bonus2() 修正。
		pc_bonus3() 追加。
	script.c
		buildin_bonus3() 追加。
	mob.c
		mob_once_spawn()、mob_damage() 修正。
	storage.h 修正。
	storage.c
		storage_storage_save() 追加
	atcommand.c 修正。

--------------
//0682 by 聖

・スピードアップポーション系のバグ修正
	(map/)
	pc.c
		pc_calcstatus() 修正。

--------------
//0681 by 死神

・装備バグ修正。
	(map/)
	pc.c
		pc_equipitem() 修正。

--------------
//0680 by 聖

・細かい修正。
・「@monster」コマンドでモンスターIDの指定に「-1」、「-2」等を指定すると
　モンスターをランダムで召還できる機能を追加。
	(map/)
	mob.c
	atcommand.c

--------------
//0679 by 波浪

・0676で新しいアイテム効果が実装されたので、item_db.txtを修正(bonus bAddMonsterDropItem,n,x; は、種族判定ができないのでとりあえず保留しました。)
・他色々修正
	(db/)
	item_db.txt
	job_db1.txt
	(doc/)
	item_bonus.txt

--------------
//0678 by 聖

・召還関連の細かい修正。
	(map/)
	mob.c
		mob_once_spawn_area() 修正。

--------------
//0677 by 死神

・細かい修正。
・アイテム売買によって得られる経験値をカードによるスキルでは得られないように修正。
・毒に掛かると自然回復できないように修正。
・0676で書き忘れ。製造武器の場合製造によって与えた属性が武器の属性より優先して適用されるように変更。(製造武器が無属性の場合は適用されません。)
	(doc/)
	item_bonus.txt 誤字修正。
	(map/)
	npc.c
		npc_buylist()、npc_selllist() 修正。
	pc.c
		pc_calcstatus()、pc_natural_heal_sub() 修正。

--------------
//0676 by 死神

・色々と修正。
・battle_athena.confに項目追加。(詳しいことはconf_ref.txtで)
・みすとれ巣を参考してダメージ計算を少し修正。
・装備bonusに色々と追加。(詳しいことはitem_bonus.txtで)
・自動セーブする時(キャラ鯖にデータを送る時)倉庫のデータも送るように変更。
・0667で言い忘れ。カートを外してもアイテムが消えないように変更。(本鯖で消えるのが仕様だと思っていたけど修正されたみたいなので。)
・取引要請を受ける側は基本スキルをチェックしないように修正。(受ける側の基本スキルチェックは自分が入れた物ではないです。いつの間にか入っていたので削除しました。)
・防具の精錬ボーナスを端数無視に変更。(これが本鯖の仕様みたいなので)
・アンクルの処理少し変更。(かからないと言う報告がありましたので...)
・プレイヤーのステータス計算で問題ありそうな所修正。
・カードのIDで機能が決まっていたカードもスクリプトによって変えることができるように変更。(詳しいことはitem_bonus.txtで)
・aspd計算方法少し変更。
・矢にbCritical、bAtkEle、bHit、bAddEle、bAddRace、bAddSize、bAddEffを適用できるように変更。矢を使うスキルや弓による攻撃だけに矢のbCritical、bAtkEle、bHit、bAddEle、bAddRace、bAddSize、bAddEffが適用されるように修正。
・キリの実装に為に修正はしましたがキリが防御無視なのかどうかがわからなかったので防御無視はしないようになっています。
・テストした物はbAddMonsterDropItemとbGetZenyNumだけなので正常に動作するかどうかの報告が欲しい所です。(ついでにitem_dbの修正も...これで吸収系とオートスペル系以外は殆ど実装できるはずです。多分...)
・その他は覚えてないけど修正した所が少しあるかも...
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	item_bonus.txt 修正。
	(db/)
	const.txt 修正。
	(char/)
	inter.c
		inter_init() 修正。
	int_storage.c
		mapif_parse_SaveStorage() 修正。
	(map/)
	trade.c
		trade_traderequest() 修正。
	pc.h 修正。
	pc.c
		pc_autosave_sub()、pc_calcstatus() 修正。
		pc_bonus()、pc_bonus2() 修正。
		pc_setrestartvalue()、pc_setequipindex() 修正。
		pc_check_equip_wcard()、pc_check_equip_dcard()、pc_check_equip_card() 削除
		その他修正。
	skill.h 修正。
	skill.c 修正。
		skill_castend_nodamage_id()、skill_unit_onplace() 修正。
		skill_check_condition()、skill_additional_effect() 修正。
		skill_attack()、skill_status_change_start() 修正。
	map.h 修正。
	battle.h 修正。
	battle.c
		battle_get_def()、battle_get_mdef2() 修正。
		battle_weapon_attack()、battle_damage() 修正。
		battle_calc_magic_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pet_weapon_attack() 修正。
		battle_calc_misc_attack()、battle_config_read() 修正。
	mob.c
		mob_damage() 修正。
	pet.c
		pet_target_check() 修正。
	clif.c
		clif_set0078()、clif_set007b()、clif_changelook() 修正。
	atcommand.c 修正。

--------------
//0675 by 波浪

・韓鯖で村正の効果が変更されたので報告を元にitem_db.txtを修正
・job_db1.txtを修正
	(db/)
	item_db.txt
	job_db1.txt

--------------
//0674 by npc

・ファーマシーの仮実装。
	(db/)
	produce_db.txt
	(map/)
	skill.c

--------------
//0673 by 南

・集中力向上に装備品の補正が入っていなかったのを修正。
　　　　　(map/)　　　
　　　　　 pc.c

--------------
//0672 by 南

・集中力向上に装備品の補正が入っていなかったのを修正。
　　　　　(map/)　　　
　　　　　 pc.c

--------------
//0672 by 聖

・モンスター系のバグ修正(すみません、まだ残ってました(^^; )
	(map/)
	mob.c

--------------
//0671 by 聖

・IDチェック範囲の修正他。
・@monsterで数を指定しなくても召還できるように修正。
	(map/)
	atcommand.c
	battle.c
	battle.h
	mob.c
	pet.c
	(conf/)
	battle_athena.conf
	(doc/)
	conf_ref.txt

--------------
//0670 by RR

・モンスタードロップ率を修正(本鯖準拠、DBでの設定+1/10000)。
・落下率０に設定したアイテムを落とすかどうかをbattle_athena.confで設定可能に。
	(map/)
	mob.c
		mob_damage() 修正。
	battle.c
	battle.h
	(conf/)
	battle_athena.conf

--------------
//0669 by 聖

・モンスタードロップの修正。
	(map/)
	mob.c
		mob_damage() 修正。

--------------
//0668 by 聖

・モンスターIDの範囲チェックを修正。
	(map/)
	mob.c
		mob_db、mob_once_spawn()、mob_once_spawn_area()、
		mob_summonslave()、mob_read_randommonster()、mob_readdb() 修正。

--------------
//0667 by 死神

・最大HP計算式をミストレ巣を参考して修正。(多分これで本鯖にあっていると思います。)
・防具の精錬ボーナスを0.7に変更。(今は端数を四捨五入していますが本鯖が端数無視なら修正しておきます。)
・@refineコマンドで装備場所IDに0を入れると装備している全ての装備を精錬するように変更。
・その他細かい修正。
	(db/)
	item_db.txt
		7140、7142を元に戻して0666の物はコマントアウトしました。
	job_db1.txt 修正。
	refine_db.txt 修正。
	(map/)
	mob.c
		mob_once_spawn() 修正。
	itemdb.c
		itemdb_read_randomitem() 修正。
	pet.c
		pet_food() 修正。
	pc.c
		pc_readdb()、do_init_pc()、pc_calcstatus()、pc_setoption() 修正。
		pc_calc_sigma() 追加。
		その他修正。
	map.h 修正。
	battle.c
		battle_calc_magic_attack()、battle_calc_misc_attack() 修正。
	atcommand.c 修正。

--------------
//0666 by 聖

・ランダムアイテムの細かい修正。
・battle_athena.confの項目追加。
・古木の枝で召還するモンスターの確率を設定出来るようにしました。
・モンスター召還アイテムを複数作る事が出来るようにしました。
・召還アイテムのサンプルとして
　生命の種子をポリン系召還、
　エンブリオをMVPボス系召還にしてみました。
　あまりいいサンプルを思いつかなかったので、
　何かいい案を思いついた人は書き換えてやってください(^^;
	(conf/)
	battle_athena.conf
	(doc/)
	conf_ref.txt
	(map/)
	mob.h
		mob_db 修正。
	mob.c
		mob_once_spawn()、mob_makedummymobdb()、mob_readdb() 修正。
		mob_readbranch() -> mob_read_randommonster()に変更。
	battle.h
		battle_config 修正。
	battle.c
		battle_config_read() 修正。
	itemdb.c
		itemdb_read_randomitem() 修正。
	(db/)
	item_db.txt
	item_bluebox.txt
	item_cardalbum.txt
	item_giftbox.txt
	item_scroll.txt
	item_violetbox.txt
	mob_branch.txt
	mob_poring.txt 追加。
	mob_boss.txt 追加。

--------------
//0665 by J

・怨霊武士の取り巻きがカブキ忍者になっていたのを酒天狗に修正。
・オットーにフェイクエンジェルが出すはずの取り巻きがついてたのを修正。
	(db/)
	mob_skill_db.txt

--------------
//0664 by 聖

・精錬失敗時他のプレーヤーにもエフェクトが表示されるように修正。
	(map/)
	script.c
		buildin_failedrefitem() 修正。

--------------
//0663 by lide

・ブランディッシュスピア修正
	(map/)
	battle.c
	skill.c

--------------
//0662 by 死神

・細かい修正とバグ修正。
・プロボックによってモンスターは乗算防御と減算防御が減るように修正してプレイヤーは減算防御だけ減るように修正。
・スクリプトgetgmlevel追加。機能はそのNPCと話しているプレイヤーのGMレベルを返します。
・0659の書き忘れですがペットのパフォマンスの種類が親密度によって増えるように変更しました。
	(map/)
	clif.c
	pc.c
	script.c

--------------
//0661 by 死神

・細かい修正。
・接続した時のペットのメッセージを親密度がきわめて親しいの時のみに出るように変更。
・0659で書き忘れですがペットの支援攻撃は親密度がきわめて親しいの時のみに発生します。(それと親密度によって支援攻撃確率が少し変化します。)
・ジルタスとアリスの卵のIDをitem_db.txtに合わせました。(自分が作ったpet_db.txtの方が自分勝手に設定していた物でしたので。て言うか未実装アイテムだから番号がわからなかっただけですが...)
・pet_db.txtのattack_rateが正しく適用されなかったバグ修正。
	(db/)
	pet_db.txt
	(map/)
	clif.c
		clif_parse_LoadEndAck() 修正。
	pc.c
		pc_attack_timer() 修正。

--------------------
//0660 by いど

・サーバーsnapshot

--------------
//0659 by 死神

・ペットを色々と修正。(ペットのコードをほとんど変えました。)
・手動的だったペットの動きをモンスターのようにAIとして処理。
・接続した時のペットのメッセージ実装。(本鯖はどうなのかわかりませんが
Athenaは接続すると100%話すようになっています。)
・ペットのスペシャルパフォマンス実装。(ただ台詞がちょっと変です。いくら探しても該当するパケットが見つからなかったので。)
・ペットの台詞を他のペットの物に変更する機能追加。(詳しいことはdb_ref.txtとpet_db.txtで。)
・ペットによる支援攻撃変更。pet_db.txtで攻撃する時と攻撃を受けた時の支援攻撃
確率を別々に設定できます。攻撃する時の場合攻撃する度にチェックをしますので
攻撃速度が速いと支援攻撃を受けやすくなります。攻撃を受けた時も同じです。(こちらはダメージを喰らう度になりますが。) 支援攻撃確率はソヒー、ジルタス、アリスだけ自分勝手に設定しています。(他のは全部1%に。ペットの支援攻撃は同じモンスターにはできないようになっています。そしてペットの戦闘能力はモンスターと同じです。)
・/hideコマンド実装。
・プロボックによって乗算防御も減るように修正。
・フリーキャストのバグ修正。
・ノービスのステータスボーナス削除。
・battle_athena.confの項目追加と削除。
・修正したファイルだけ。未テストした物もかなりありますので問題があったら報告をお願いします。
	(conf/)
	battle_athena.conf
	(doc/)
	conf_ref.txt
	db_ref.txt 追加。(今説明が入っているのはpet_db.txtのみです。 )
	client_packet.txt
	(db/)
	pet_db.txt
	job_db2.txt
	(map)
	clif.h
	clif.c
	map.h
	map.c
	pet.h
	pet.c
	pc.c
	mob.h
	mob.c
	npc.c
	atcommand.c
	skill.c
	battle.h
	battle.c

--------------
//0658 by huge

・ペットがとどめをさすと、飼い主に経験値が入るようにしました。
・固定値ダメージじゃ味が無いのでATK1～ATK2の間で乱数を取るようにしました。
・あと、ペットがとどめをさすかどうかの設定を、battle_athena.confに加えました。

	(conf/)
	battle_athena.conf pet_finish追加。
	(map/)
	battle.c
		battle_config_read() 修正。
	battle.h 修正。
	pet.c
		pet_attack() 修正。
	(doc/)
	conf_ref.txt 説明追記。

--------------
//0657 by huge

・ペットによる攻撃を実装。
・ペットを持っていて、ペットが装備品をつけてて、さらにランダムによる判定で発動します。
・ただの遊び心ですｗ
・battle_athena.confで頻度を設定できます。詳細はdocで。

	(conf/)
	battle_athena.conf pet_attack追加。

	(map/)
	battle.c
		battle_config_read() 修正。
	battle.h 修正。
	pc.c
		pc_attack_timer() 修正。
	pet.c
	pet.h
		pet_attack() 追加。
	(doc/)
	conf_ref.txt 説明追記。

 とりあえず、ペットが動いてるなぁって感じと、ダメ回数を増やした程度です。

--------------
//0656 by 死神

・グランドクロスの修正。(おいおい何度目だ...)
・グランドクロス計算式間違いで修正。(÷3がまずかったみたいです。)
でもまだ反射ダメージがみすとれ巣よりちょっと高いです。(10ぐらいだから
関係ないかも)
・モンクの気球を必中に修正。(自分の間違いのようですので...)
	(map/)
	skill.c 修正。
	battle.c 修正。

--------------
//0655 by 死神

・グランドクロスの修正。
・自分なりに情報を収集してみた結果グランドクロス反射ダメージは
プレイヤーキャラがそのキャラ自身にグランドクロスを使った時の
ダメージだそうなので修正しました。(みすとれ巣の計算とはかなり違うような
気もしますが...)
・魔法とトラップ、鷹の攻撃にも属性耐性と種族耐性を適用するように修正。
(本鯖の仕様にあっているかどうかは不明ですが適用した方が正しいと思ったので
修正しました。)
	(map/)
	skill.c 修正。
	map.h 修正。
	battle.c 修正。

--------------
//0654 by 死神

・グランドクロスの修正と細かい修正。(計算式間違いで修正。)
・0653で書き忘れ。気功による追加ダメージは必中ではないらしいので
修練の加算と同じ所に計算するように変更しました。
・カートにバグがありそうだったのでちょっと修正。
・ダメージ計算をほんの少し修正。(ダメージ量が変わったりはしません。)
	(map/)
	battle.c
		battle_calc_magic_attack() 修正。
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
	skill.c
		pc_damage_feedback() -> skill_damage_feedback()に変更。
		skill_unit_timer() 修正。
	pc.c
		pc_setoption() 修正。
	atcommand.c 修正。

--------------
//0653 by 死神

・0652の修正と細かい修正。今まで通り未テストも多いです。
・グランドクロスの処理修正。(本鯖にあっているかどうかの自身はありません。)
ラグナーゲートの説明によると始めに現在HPの20%が消耗されてその後敵に与えた
ダメージの中で一番高い物が戻ってくるようです。そしてその戻ってきた
ダメージは聖の属性を持ちトラストによって聖の耐性が50%になっているので
半分を喰らうことになるようです。(聖の耐性上がる装備をしていれば戻ってくる
ダメージは受けないようです。)
問題なのはプレイヤーの防御属性を計算するかどうかです。今は防御属性計算の
後で聖の属性を計算しています。そして戻ってくるダメージはHPバーは減るけど
表示はされません。本鯖の方がどうなのか不明なので...
それと一応モンスターもグランドクロスの使用が可能です。ただモンスターの場合
現在HPの20%消耗の後のダメージは受けません。(モンスターが使う
グランドクロスのテストはしてません。)
・ダメージによるディレイ中にまたディレイがかからないように修正。(大した意味はないかも...)
・値段がゼロのアイテムも売れるように変更。
・@コマンドhealの処理少し修正。
・移動コード少し修正。
	(map/)
	clif.c
		clif_selllist() 修正。
	battle.c
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_calc_magic_attack()、battle_calc_misc_attack() 修正。
	skill.c
		skill_additional_effect()、skill_unit_onplace()  修正。
		skill_status_change_start()、skill_unit_onplace() 修正。
		skill_castend_damage_id()、skill_castend_id()、skill_attack() 修正。
		skill_unitsetting()、skill_check_condition() 修正。
		skill_use_id()、skill_use_pos() 修正。
	npc.c
		npc_parse_script() 修正。
	pc.h 修正。
	pc.c
		pc_walk()、pc_walktoxy_sub()、pc_stop_walking() 修正。
	map.h 修正。
	mob.h 修正。
	mob.c
		mob_stop_walking()、mob_changestate()、mob_walk() 修正。
	pet.c
		pet_changestate() 修正。
	atcommand.c 修正。
	(db/)
	skill_db.txt グランドクロス修正。
	cast_db.txt グランドクロス修正。

--------------
//0652 by 月詠み

・グランドクロスを仮実装
	(db/)
	skill_db.txt
	cast_db.txt
	(map/)
	battle.c
		Damage  battle_calc_misc_attack
		Damage  battle_calc_magic_attack
	skill.c
		skill_additional_effect
		skill_castend_damage_id
		skill_castend_pos2
		skill_unit_group *skill_unitsetting
		skill_unit_onplace
		skill_check_condition

--------------
//0651 by 波浪

・item_db.txtを修正
	(db/)
	item_db.txt
		装備品のアイテム効果を修正

--------------
//0650 by 死神

・三段掌の発動条件を弓と二刀流以外に変更。
・表示をせずに内部で処理だけするNPCのCLASSを111から32767に変更。
・細かい修正。
	(map/)
	clif.c
		clif_getareachar_npc()、clif_spawnnpc()、clif_pcoutsight() 修正。
	npc.h 修正。
	battle.c
		battle_calc_pc_weapon_attack()、battle_calc_mob_weapon_attack() 修正。

--------------
//0649 by 波浪

・DBとDOC修正
	(db/)
	item_db.txt
		グングニールはLV4武器なので、韓国のデータにあわせて風属性ではなくします。
	mob_db.txt
	size_fix.txt
		楽器は大型に対して75%だそうです。
	(doc/)
	item_bonus.txt

--------------
//0648 by 死神

・ショップの値段に-を入れると鯖が落ちる問題修正。(itemdbの初期化をnpcより
先にするように変更。) それだけです。
	(map/)
	map.c
		do_init()

--------------
//0647 by nini

・item_db修正
・スナッチャー仕様変更。弓以外のすべての武器で出るようになってます。
	(/map/)
	battle.c
		三段掌の発動条件追加
	skill.c
		スナッチャーの発動条件追加
	(/db/)
	item_db.txt
		シルクハットにSP上昇追加

--------------
//0646 by last

・item_db.txtの修正(属性関連)
	(/db/)
	item_db.txt

--------------
//0645 by るるる（＆ree_ron）

・item_value_db.txtにディスカウント＆オーバーチャージ等のスキルによる価格変動を受けるかどうかのフラグメントを追加。
　実際の形式はサンプルとして用意したitem_value_db.sample.txtを見てください。（設定価格は完全に独断と偏見です）
　同様のサンプルとしてＮＰＣ設置スクリプトも添付しておきます。
・item_value_db.txtのアイテム価格設定で、売値と買値の設定を独立。（item_db.txtは従来どおり買値は売値の半額として自動処理）
・ＮＰＣショップにて、１ＮＰＣで扱えるアイテム数を最大64から最大100に変更。（クラ自体は120ぐらいまで可能ですが）
	(/db)
	item_value_db.txt
		カラム数を整理しただけです。内容はまったく変更していません。
	(/map/)
	clif.c
		clif_buylist() clif_selllist()  変更
	itemdb.h
		item_data 構造体変更
		itemdb_value_buy() itemdb_value_sell() itemdb_value_notdc() itemdb_value_notoc() マクロ追加
	itemdb.c
		itemdb_search() itemdb_readdb() itemdb_read_itemvaluedb()  変更
		itemdb_sellvalue()  削除
	npc.c
		npc_buylist() npc_selllist() npc_parse_shop()  変更
	(/sample/)
		オマケです。次回SnapShotには含まないで宜しいです。

コメント
原型は私の友人ree_ronが行い、私が更に細かいミスを直しただけですが、テストはしましたので大丈夫でしょう。
元々この処理を導入する理由として、特定アイテムの売値が1z固定にできないものか、という点だったからです。
そしてやっていくうちに、ＮＰＣショップを利用したレアアイテムの販売とかで本鯖露店に近いことが出来るのではないか、
ということが判ってきたわけです。
それで一応はデータを用意しましたが、あくまでもサンプルとして利用してください。もし可能ならば、
さらに修正を加えてアテナ独自として本採用としたデータをパッチアップしてくれればとも思いますがｗ


--------------
//0644 by nini

・DBの間違い、643で追加されたスクリプト追加。
	(/db/)
	item_db.txt
	cast_db.txt
		チャージアローのキャスト追加。
	exp_guild.txt
		46-50までのexp抜けに追加。
	size_fix.txt
		楽器、鞭、ナックルのサイズ補正修正。

--------------
//0643 by 死神

・色々と修正。
・bMVPaddAtkRate削除。bAddRaceで処理するように変更。
・bIgnoreDefEleとbIgnoreDefRace追加。
bonus bIgnoreDefEle,n;	n属性の敵の防御無視
bonus bIgnoreDefRace,n;	n種族の敵の防御無視
・bMatkRate追加。魔法攻撃力を+n%上げます。よってbattle.cで計算していたロッドによる魔法攻撃力増幅の計算はなくしました。ステータス画面に上がった数値は表示されません。ダメージ計算の時に適用しています。
・bCriticalDefに-を入れるとクリティカルを喰らう確率が上がるように変更。
・NPC番号111は透明NPCですが落とし穴等のことを考えて表示を一切せずに
内部で処理だけするように変更。(flagを使うと何とかなりそうですがその
処理が全然わからなかったので透明NPCにクリックや名前の表示もできないように変更しました。)
・ショップの値段に-を入れるとitem_db.txtもしくはitem_value_db.txtの物を使うように変更。
・スキルルアフのエフェクトがサイトと同じだったので修正。ついでにルアフの
ダメージも修正。
・みすとれ巣によるとモンスター情報で表示される防御と魔法防御は乗算ではなく減算みたいなので修正。
・他力本願ですがitem_db.txtの修正をお願いします。(全てのロッドにbonus bMatkRate,15; を入れる必要があります。その他の修正も必要です。)
・テストしていない物もかなりありますので問題があったら報告してください。
	(map/)
	map.h 修正。
	map.c
		map_quit() 修正。
	pc.h 修正。
	pc.c
		pc_walk()、pc_stop_walking()、pc_setpos()、pc_authok() 修正。
		pc_calcstatus()、pc_bonus()、pc_natural_heal_sub() 修正。
	npc.h 修正。
	npc.c
		npc_touch_areanpc()、npc_parse_shop() 修正。
	clif.c
		clif_quitsave()、clif_getareachar_npc()、clif_spawnnpc() 修正。
		clif_skill_estimation() 修正。
	battle.c
		battle_calc_magic_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_calc_mob_weapon_attack() 修正。
	mob.c
		mobskill_use() 修正。
	skill.c
		skill_status_change_end()、skill_status_change_timer() 修正。
		skill_status_change_start() 修正。
	(db/)
	const.txt 修正。
	(doc/)
	item_bonus.txt 修正。

--------------
//0642 by 死神

・装備バグ修正。(それだけ)
	(map/)
	pc.c
		pc_authok()、pc_checkitem() 修正。

--------------
//0641 by 死神

・bAspdRateとbSpeedRateのバグ修正。(それだけ)0640で計算をちょっと変えて見ましたがそれがまずかったみたいです。今度も計算式を変えましたがもう大丈夫だと思います。(多分)
	(map/)
	pc.c
		pc_calcstatus()、pc_bonus()、pc_delitem()

--------------
//0640 by 死神

・バグ修正と少し修正。
・みすとれ巣を参考してダメージ計算を少し修正。
・battle_athena.confに項目追加。(詳しいことはconf_ref.txtで)
・キャラのHPとSPを2byteから4byteに変更。(テストはしていますがバグが出る
可能性もかなりあります。ただキャラセレクト画面でHPやSPが32768を越える時
表示は32768になるけど内部の処理は正常に動きますのでそれはバグではありません。
パケットの長さのせいでそれ以外手段がなかったので...)
・bCriticalDef(クリティカルを喰らわない確率+n%)の処理変更。100にすれば
クリティカルを喰らわないようになります。)
・bInnerAtkをbBaseAtkに変更。みすとれ巣でカードの攻撃は基本攻撃力の方に足されるとありましたので変更しました。今度は上がった攻撃力が表示されます。
・bDoubleRateの処理変更。確率を足さずに一番高い物だけ適用します。それと左手
装備の場合無視するように変更しまた。(左手はダブルが適用されませんので)
・bDoubleAddRate追加。機能はダブルアタック確率+n%(武器無視)です。
左手装備は無視されます。
・0635で攻撃力表示を本鯖にあわせました。そして今度は弓だけではなく
楽器とムチもdexによって攻撃力が上がるように変更しました。
・装備した武器が消えるバグ修正の為に少し修正はしましたが本当に
大丈夫なのかは不明です。報告をお願いします。
	(conf/)
	battle_athena.conf 修正。
	(db/)
	const.txt 修正。
	item_db.txt 修正。
	(doc/)
	item_bonus.txt 修正。
	conf_ref.txt 修正。
	(map/)
	map.h 修正。
	pc.c
		pc_calcstatus()、pc_bonus()、pc_equipitem() 修正。
	battle.h 修正。
	battle.c
		battle_calc_mob_weapon_attack()、battle_calc_pc_weapon_attack() 修正。
		battle_config_read() 修正。
	clif.c
		clif_updatestatus()、clif_parse_LoadEndAck()、clif_party_hp() 修正。
	(common/)
	mmo.h 修正。
	(char/)
	char.c
		mmo_char_send006b()、parse_char() 修正。

--------------
//0639 by 胡蝶蘭

・ladminの修正など
	・プロンプトの入力にTerm::ReadLineを使うようにした
	　（入力履歴やコマンドラインの編集が可能に）
	・POSIX関係の処理の例外エラーをトラップするようにしました
	　（POSIXが全く使えない環境でも最低限、動くようになったかもしれない）
	・細部修正

	(tool/)
	ladmin
		Ver.1.04に。

・MODバージョンがおかしい問題を修正
	(common/)
	version.h
		ATHENA_MOD_VERSIONが８進数で記述されている問題を修正
		数字の頭に0をつけると８進数になるので注意してください

--------------
//0638 by 波浪

・0635・0637で新しくアイテム効果が実装されたので、それに伴ってitem_db.txtを修正
・item_bonus.txtを修正
	(db/)
	item_db.txt 修正
	(doc/)
	item_bonus.txt 修正

--------------
//0637 by 死神

・0635のバグ修正。
・battle_athena.confに項目追加。(詳しいことはconf_ref.txtを見てください。)
・時間が遅すぎて0635で説明してなかったです。(寝不足だったので...)
まず仕様が変わったのは二刀流のダメージを武器別に完全に分けて行うように
変更とアサシンじゃなくても左手修練を覚えていれば二刀流を使えるように
変更しました。それとダメージの計算をちょっと修正。
そしてbonusに追加されたのは
bonus bInnerAtk,n;		内部攻撃力+n
カードの引き上げダメージ用です。表示はされないけどダメージに計算されます。
bonus bSpeed,n;		移動速度+n
移動速度をn上げます。
bonus bAspd,n;		攻撃速度+n
攻撃速度をn上げます。
bonus bSpeedRate,n;		移動速度+n%
移動速度をn%上げます。
bonus bAspdRate,n;		攻撃速度+n%
攻撃速度をn%上げます。
bonus bHPrecovRate,n;	HP自動回復率+n%
自動回復するHPの量をn%上げます。スキルによる回復には影響がありません。本鯖の仕様とあっているかは不明です。
bonus bSPrecovRate,n;	SP自動回復率+n%
自動回復するSPの量をn%上げます。スキルによる回復には影響がありません。本鯖の仕様とあっているかは不明です。
bonus bCriticalDef,n;		クリティカルを喰らわない確率+n%
クリティカルの耐性をn上げます。10000以上にするとクリティカルを喰らいません。
bonus bMVPaddAtkRate,n;	MVPモンスターにn%の追加ダメージ
ボスモンスターにn%の追加ダメージを与えます。深淵の騎士カード用。
bonus bNearAtkDef,n;		近距離攻撃のダメージをn%の減らす
全て近距離攻撃のダメージをn%の減らします。(魔法とトラップ、鷹を除く)
bonus bLongAtkDef,n;	遠距離攻撃のダメージをn%の減らす
全て遠距離攻撃のダメージをn%の減らします。(魔法とトラップ、鷹を除く)
bonus bDoubleRate,n;	ダブルアタック確率+n%(武器無視)
武器に関係なく発動するダブルアタック確率をn%上げます。
ダブルアタックスキルと別の判定を行う為ダブルアタックスキルが
あってもスキルによるダブルアタック確率が上がるわけではありません。
サイドワインダーカード用。
	(map/)
	pc.c
		pc_bonus()、pc_calcstatus() 修正。
		pc_natural_heal_sub() 修正。
	battle.h
		struct Battle_Config {} 修正。
	battle.c
		battle_calc_pc_weapon_attack()、battle_calc_mob_weapon_attack() 修正。
		battle_config_read() 修正。
	(db/)
	skill_db.txt
		スティールのSPを10に修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。

--------------
//0636 by Sin

・デバッグ用ポタ子さんスクリプト(npc_pota.txt)のアマツ・コンロンへの対応。
　すでに自力実装されていらっしゃる方々も多いかと思いますが…。
　コンロンダンジョンの名前がわからないため「崑崙D1」などとさせていただいています。
	(conf/)	npc_pota.txt

--------------
//0635 by 死神

・battle_athena.confに項目追加。(詳しいことはconf_ref.txtを見てください。)
・bonusにbInnerAtk(カード等で表示はされないけど実際には攻撃力に反映される物用です。)等を追加。他のはitem_bonus.txtを見てください。(追加はしたけどitem_db.txtは殆んど修正してません。)
・その他バグ修正や仕様変更もやりましたが一々書く時間がないので...
	(map/)
	makeile 修正。
	pc.c 修正。
	map.h 修正。
	clif.c 修正。
	battle.h 修正。
	battle.c 修正。
	itemdb.c 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	item_bonus.txt 修正。
	(db/)
	const.txt 修正。
	item_db.txt 修正。

--------------
//0634 by 死神

・weddingクラスには転職できないように修正。
・スーパーノービスの為にexpテーブルをbase 4つjob 4つに拡張。
exp.txtが変わりスーパーノービスだけの経験値を設定できます。(exp.txtの
4つ目がスーパーノービスのbase expで8つ目がjob expです。今は2次職業の物を
コピーした物に過ぎませんが。) exp.txtの設定方法も知らない方はいないと
思いますので説明は省略します。
・スーパーノービスは転生のテストの為に韓国サクライだけ実装している物と
思われますが(転生が実装されればなくなると予測しています。)それを
実装していいのかと思ったりもしますが...
・結婚衣裳は既に0629で実装しているのにまたパッチとして
アップされるのもちょっと変(？)ですね。そういえば説明してなかったんですね。
・装備専用スクリプトであるchangebaseの追加によってタキシードと
ウェディングドレスが実装しています。これは職業を変更せずに見た目だけ
変える物です。weddingクラス以外の適用も可能で変装セットとかも作れる
わけですが内部処理は変更せずに見た目だけ変えているので0631で説明したように
装備できない物を装備している場合鞍落ちが起こる可能性がありますので
他の職業で使うのはお勧めしません。仮実装なのは今の仕様はタキシードと
ウェディングドレスを装備するだけで見た目が変わるからです。韓国サクライの
方では何かの条件が必要だと思っているのでその条件がまだ実装されいないから
仮実装です。それにweddingクラスを職業にしてしまうと結婚するとスキル等が
リセットされるか変になるかのどちらなので変だと思ってなかったのでしょうか？
	help.txt 修正。
	(db/)
	job_db1.txt 修正。
	exp.txt 修正。
	(map/)
	pc.c
		pc_jobchange()、pc_readdb() 修正。
		pc_nextbaseexp()、pc_nextjobexp() 修正。

--------------
//0633 by 波浪

・装備の設定修正。結婚衣裳の職は、実際に転職するのではなくペコナイト(13)、ペコクルセ(21)のように画像を使うだけだと思うので
　何も装備できない設定にしました。スパノビはノビが装備できるものだけ設定しました。
・古木の枝の出現モンスターを追加
・アマツのモンスの沸き具合を本鯖に近くなるように修正(まだまだ違いますが・・・)
	(conf/)
	npc_monster.txt		モンス名修正
	npc_monster_amatsu.txt	修正
	(db/)
	item_avail.txt		鞍落ちアイテム追加
	item_db.txt		装備設定を修正、他多数
	mob_branch.txt		修正
	mob_db.txt		モンス名修正
	skill_tree.txt		修正

--------------
//0632 by nini

・@jobchangeで結婚衣裳とスーパーノービスになれるように。(注意：韓国桜井クライアントのみ)
・Sノビのステ、スキルなども暫定追加。(ノービスのコピーですが)
　とりあえず見た目だけということで、結婚衣裳でも攻撃できますが(ただしノーモーション)、本来はできません。
・上にあわせてitem_db編集。
　結婚衣裳で武器もつとact、sprエラー出すので、結婚衣裳では武器を持てないようにした(はず)。
	(db/)
	job_db1.txt
	job_db2.txt
	item_db.txt
		結婚衣裳、Sノビのデータ
	skill_tree.txt
		Sノビのスキル
	(map/)
	map.h
		MAX_PC_CLASSに追加

--------------
//0631 by 死神

・細かい修正。
・タキシードとウェディングドレスの表示をbattle_athena.confで設定できる
ように変更。
・武器グラパッチについてですがパッチ前は使えない職業が装備をしても表示は
されないだけで鞍落ちまでは起こらなかったけど武器グラパッチの後はその武器を
装備することができない職業(本鯖で)が装備してしまった場合鞍落ちが起こる
ことがありますので注意してください。
	(db/)
	item_db.txt
		1161、2338、7170 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	battle.h
		struct Battle_Configにwedding_modifydisplay 追加。
	battle.c
		battle_config_read() 修正。
	pc.h
		pc_cart_delitem() 修正。
	pc.c
		pc_jobchange()、pc_additem()、pc_delitem()、pc_cart_delitem() 修正。
		pc_checkitem()、pc_getitemfromcart() 修正。
	clif.c
		clif_changelook()、clif_send()、clif_parse_GlobalMessage() 修正。
	script.c
		buildin_changebase() 修正。
	storage.c
		storage_storageaddfromcart() 修正。
	vending.c
		vending_purchasereq() 修正。

--------------
//0630 by 引退人

・ギルド脱退時にcharサーバが落ちることがあるのを修正
・water_height.txtを更新
・0627に関連してmodifydisplay関連を削除
	(char/)
	int_guild.c
		mapif_guild_leaved()のバッファ容量が足りなかったので、
		unsigned char buf[64]; -> unsigned char buf[128];
		と修正。
	(conf/)
	battle_athena.conf
		equip_modifydisplayを削除
	water_height.txt
		prt_fild04.gatとmoc_fild01.gatの分を追加
	(doc/)
	conf_ref.txt
		equip_modifydisplayの説明を削除
	(map/)
	battle.h
		struct Battle_Config からequip_modifydisplayを削除

--------------
//0629 by 死神

・0627のバグ一部修正と新しい移動パケットに対応。(自分の間違いでした。
モンスターやNPCも新しい移動パケットを使うと思っていたのですが
新しい移動パケットはプレイヤーのみのようです。)
・タキシードとウェディングドレス仮実装。(韓国のサクライ鞍じゃないと
鞍落ちされます。使用する時はitem_db.txtのコマントアウトされている
2338と7170を解除して使ってください。)
・SP回復アイテムもintによって効果が増えるように変更。
・0627で書き忘れですがカートのアイテム計算とitemdb_を呼ぶのを最小化する
処理を入ってるせいでpc_additem()、pc_delitem()、pc_cart_additem()、pc_cart_delitem()以外の方法でカートアイテムやアイテムに変動がある場合
正常に動作する保証がないので修正のさいには注意してください。
	readme0754をreadme0574に修正。
	makefile 修正。
	(map/)
	map.h 修正。
	clif.c 修正。
	pc.c 修正。
	battle.c 修正。
	mob.c 修正。
	script.c 修正。
	(db/)
	item_db.txt 修正。
	class_equip_db.txt 修正。
	skill_db.txt 修正。(誤字を治しただけです。)
	(conf/)
	npc_event_doll.txt 修正。(流浪人さんありがとうございます。)

--------------
//0628 by NOCTURNE

・snapshot
・snapshotからsnapshotまでのReadme分割
・要望が多かったのでsnap作成（プログラム的な変更点は無し

--------------
//0627 by 死神

・コードの最適化と少し修正。(少しは軽くなると思います。)
・ @modifydisplayコマンド削除。
・新しい移動パケットに対応だと思ったら0x1d8、0x1d9、0x1daパケットの一部が
0x78、0x79、0x7bと変わってるみたいです。つまり今のままでは対応できません。
X,Yの座標の部分の書式が変わったのかと予測はしていますが...
情報を求みます。(makefileのDPACKETVERを4にすれば0x1d8、0x1d9、0x1daを
使いますが座標がずれたらしく何も標示されません。)
・100000からだったchar_idを150000からに変更。(ペットの卵の問題で武器の名前がちょっと変になったので修正しました。)
・ペットのコード少し修正。(pet_idをcard[2]とcard[3]からcard[1]とcard[2]に変更しました。よって前に作った卵は使えません。鞍でcard[3]の機能が変わったので
仕方なく修正しました。今度はconvertツールがありません。作る時間がなかったので...)
・最適化の為に修正した所が多いですが全て正常動作する保証はありません。
鯖落ちバグが発生したら報告お願いします。(batte.cはまだ最適化してません。)
カートのアイテム計算やアイテムの重量の計算を最初にだけするようにしているので表示に少し問題があるかも...
・修正したファイルだけ書いておきます。
	makefile
	help.txt
	(common/)
	mmo.h
	(map/)
	map.h
	atcommnad.h
	atcommnad.c
	pc.h
	pc.c
	clif.c
	script.c
	trade.c
	itemdb.h
	itemdb.c
	battle.h
	battle.c
	pet.c
	map.c
	mob.c
	(char/)
	char.c
	(conf/)
	battle_athena-conf
	atcommand_athena.conf 

--------------
//0626 by 引退人

・パケット長テーブル（新移動パケットなど）修正
	(doc/)
	client_packet.txt	パケット解析スレ Mさんの情報を反映
	conf_ref.txt		0624に合わせて修正
	(map/)
	clif.c
		packet_len_table[]	client_packet.txtに合わせて修正

--------------
//0625 by 引退人

・@hide透明化をBOSSなどに見破られないように修正
	(map/)
	pc.h
		#define pc_iscarton(sd)	修正
		#define pc_isinvisible(sd)	追加
	mob.c
		mob_attack()
		mob_target()
		mob_ai_sub_hard_activesearch()
		mob_ai_sub_hard_mastersearch()
		mob_ai_sub_hard()
			透明（pc_isinvisible(sd)!=0）で死人と同様に判定されるように修正
	(conf/)
	npc_cTower.txt	修正（thx to holyzardさん）

--------------
//0624 by るるる

・武器画像表示処理の一新（新移動パケット使用）
・上と関連して、@modifydisplayコマンドを設けた
　　　　機能としては、現在のアサシン武器などのがおかしい場合に、または気に入らないとかで、
　　　　キャラ毎に旧パケを使用するようにしている。

	(map/)
	atcommand.c
		atcommand()  @modifydisplayコマンドを追加
	atcommand.h
		struct Atcommand_Config {
'7d  変更
	clif.c
		clif_set0078_and01d8() , clif_set007b_and01da()  関数名＆処理の変更
		clif_spawnpc() , clif_movechar() , clif_changelook() , clif_getareachar_pc() , 
		clif_fixpcpos() , clif_parse_LoadEndAck()  変更
	map.h
		struct map_session_data ・b}  変更
	pc.c
		pc_setnewpc() , pc_calcstatus() , pc_equiplookall() , pc_changelook()  変更

	(conf/)
	atcommand_athena.conf
		equip_modifydisplay  追加

	変更箇所は全てキーワード「modifydisplay」でサーチすればほぼわかるかと。

コメント：もうこれで問題は無いはず。実は大いなる勘違いをしてた個所があったのは内緒（マテw

--------------
//0623 by 引退人

・@hideで透明化（見られない＆MOBにタゲられない）するように修正など
	(map/)
	atcommand.c
		@hideのoption設定を0x04から0x40に変更
	mob.c
		mob_attack()
		mob_target()
		mob_ai_sub_hard_activesearch()
		mob_ai_sub_hard_mastersearch()
		mob_ai_sub_hard()
			option判定を0x06から0x46に修正
	(conf/)
	npc_event_potion.txt	MORISON_MEAT修正（thx to holyzardさん）

--------------
//0622 by 引退人

・mobがスキル使用に失敗した場合、通常攻撃するように
	(map/)
	mob.c
		mobskill_use_id()	スキル使用失敗で0、成功で1を返すように修正
		mobskill_use_pos()	スキル使用失敗で0、成功で1を返すように修正
		mobskill_use()		上記を反映して失敗時には0を返すように修正

--------------
//0621 by 胡蝶蘭

・アイテムチェックを行うかどうかconf/battle_athena.cnfに書けるように
・アイテムチェックで不正と判断するかどうかをdb/item_avail.txtに書けるように
・@itemcheckで明示的にアイテムチェックできるように

	デバグやテストなどで色々なアイテムIDを使用したい場合は
	アイテムチェックを無効にして下さい。(item_check: off)
	無効にした場合でも@itemcheckコマンドでチェックすることが出来ます。
	cnfファイルは用意してないので必要なら各自適当に書き換えてください。

	(db/)
	item_avail.txt
		新規追加。不正アイテムの列挙に使用。未完成。他力本願。
		item_db.txtに定義されてるが実際には使用できないアイテムを書く。
	(doc/)
	conf_ref.txt
		battle_athena.cnfとatcommand_athena.cnfの説明修正
	(map/)
	itemdb.c/itemdb.h
		itemdb_availableマクロ追加
		itemdb_read_itemavail()追加
		itemdb_readdb()でavailable=1にするように
		itemdb_search()で存在しないIDはavailable=0でデータを作るように
		do_init_itemdb()でitemdb_read_itemavail()を呼ぶように
	pc.c/pc.h
		pc_checkitem()をエクスポート
		pc_checkitem()でavailableとbattleconfigをチェックするように
	atcommand.c/atcommand.h
		@itemでbattleconfigをチェックするように
		@itemcheckコマンド追加
		atcommandconfigにitemcheckメンバ追加
	battle.c/battle.h
		battle_configにitem_checkメンバ追加

・ladminの修正など
	アカウント追加、パスワード変更の際にパスワードを省略すると、
	パスワード用のエコーしない専用プロンプトで入力できます（＆入力確認）。
	追加の際にパスワードが表示されたら困る場合などに。
	パスワード入力中はCtrl+Cが効かないので注意してください。

	パスワードの不正文字の表示が、何文字目かで表示するようになりました。
	その他微妙にチェック追加など。

	Cygwinでしか動作確認していません。POSIXモジュールを使っているので、
	POSIXでない(＆エミュレーションもできない)プラットフォームだと
	動かないかもしれません。
	UNIX系ではnkfなどで改行コードを変換しないとだめかも？

	(tool/)
	ladmin
		Ver.1.03に。

-------------
//0620 by 月詠み

・ホーリークロス実装

	(db)
		skill_db.txt 修正
	(map)
		skill.c
			skill_additional_effect()修正(コメントのみ)
		battle.c
			Damage battle_calc_weapon_attack()修正

-------------
//0619 by るるる

・パッチ0617のでやり忘れと微妙な修正

	clif.c
		clif_movechar(),clif_parse_LoadEndAck() 修正

--------------
//0618 by nini

・リザレクションの詠唱、ディレイ追加。回復量修正。
・消費SP修正
・アローシャワーの範囲を5*5にして2セル吹き飛ばし。
・チャージアローの使用武器条件無し。
・スピアスタブの飛距離を6セルに。
	(/db)
		cast_db.txt 修正
		skill_db.txt 修正
	(/map)
		battle.c
			battle_calc_weapon_attack() 修正
		skill.c
			skill_castend_damage_id() 修正
			skill_check_condition() 修正
			skill_castend_nodamage_id() 修正

--------------
//0617 by るるる

・武器画像表示で他キャラが表示されないのを「とりあえず」修正
・靴表示のパケットを送信停止（現時点ではムダ。コメントしただけですが）
	clif.c
		clif_spawnpc(),clif_getareachar_pc(),clif_fixpcpos(),clif_changelook()修正
	pc.c
		clif_changelook()がある部分を修正（武器ー＞盾と順になるように処理の入れ替え）

コメント。
新マップ移動パケ(0x1d8～0x1da）を色々とやったが、そのパケ１つで武器表示が新式のに対応してる
というわけではないっぽい。旧移動パケだと自分以外のキャラが移動すると旧式表示になってしなう。
更に、新武器表示パケは武器と盾の同時処理が出来てない。おそらくクライアントの問題だと思う。
とりあえず、キャラが動くたびに新武器パケ＝＞旧盾パケの２つの装備パケを送ることで解決させている。
本鯖ではどうなのかの実際のところのデータが無いため、これ以上のことはムリ。

--------------
//0616 by 胡蝶蘭

・water_height.txtを読んでいないとサーバーが落ちるバグ修正
	map.c
		map_waterheight()修正

・PCのマップ移動時のアイテムチェックでアイテムIDの存在をチェックするように修正
・一部の＠コマンドでアイテムIDの存在をチェックするように修正
	pc.c
		pc_checkitem()修正
		pc_authok()修正 pc_checkitem()追加
	clif.c
		clif_parse_LoadEndAck()修正
	itemdb.c
		itemdb_exists()追加（itemdb_searchと同じだが、dbに存在しない
			場合は新しいデータを作らずにNULLを返す）
		itemdb_read_classequipdb()修正 itemdb_search=>itemdb_exists
		itemdb_read_itemnametable()修正 itemdb_search=>itemdb_exists
		itemdb_read_itemvaluedb()修正 itemdb_search=>itemdb_exists
	atcommand.c
		@item修正 itemdb_search=>itemdb_exists
		@produce修正 itemdb_existsでチェックするように

--------------
//0615 by 波浪

・アイテムDATA大幅修正
　主な修正箇所は、回復アイテムの回復量の修正、消費アイテムをclass_equip_db.txt無しでも使用できる様に修正、
　装備品の装備可能職を全て修正、カード効果を修正、etc・・・です。

--------------
//0614 by Nikita

・アイテムDATAの修正（主に回復量）
・スキル解毒の射程修正
・0612の細かい修正
	(conf/)
		npc_town_prontera.txt 修正
	(db/)
		item_db.txt 修正
		skill_db.txt 修正

--------------
//0613 by 引退人
・checkweight修正
	(conf/)
		npc_event_making.txt checkweight部分を修正
		npc_event_potion.txt ポーション、ジュースNPCのcheckweight修正

--------------
//0612 by nini

・アイテムDATA修正
	(db/)
		item_db.txt 修正
	(conf/)
		npc_town_***.txt 修正
		R.O.M776さんを参照しました。

--------------
//0611 by 死神

・アイテム使用条件があわない時0xa8パケットを送るように変更。(バグ報告スレッド 243のno nameさん情報提供ありがとうございます。)
・QMで集中力向上と速度上昇、アドレナリンラッシュ、ラウドボイス、スピアクイッケン、ツーハンドクイッケンを解除するように修正。
・速度上昇と速度減少で逆のスキルが解除されるように修正。
・0609で書き忘れ。モンスターがQMの範囲から抜けても効果が維持するように
変更とブレッシングで呪いと石化が解除されるように修正。
	clif.c
		clif_useitemack() 修正。
	skill.c
		skill_status_change_start() 修正。
	pc.c
		pc_insert_card() 修正。(これはカードバグとは関係ない修正です。そのバグの修正は自分が05xx当たりで修正しましたので。)

--------------
//0610 by 波浪

・アイテムDATA修正
	(db/)
		item_db.txt 修正

--------------
//0609 by 死神

・色々と修正。
・モンスターが止まるように動く問題修正。
・指弾のディレイ修正。
・矢作成のコード片付け。
・敵がスキル範囲から逃げた場合スキルが失敗するように変更。
・class_equip_db.txtの仕様変更。
	性別と装備レベルも設定可能に変更と使用アイテムの使用職業、性別と使用
	レベルの設定ができるように変更。(ただデータが多いせいで修正した
	class_equip_db.txtはサンプル程度の物です。埋めてください。他力本願ですが...)
	それとアイテム使用条件があわないとアイテムが使わないようにしては
	いますが01c8パケットの<type>を0にしてもアイテムを使用した時と同じ
	エフェクトが出ます。本鯖のアイテム使用パケットが分からないままじゃ
	こうするしかなかったのですが...
・battle_athena.confに項目追加。
・その他スキル少し修正と細かい修正。
・修正した所を全て覚えてませんのでファイルだけ。
	(map/)
	clif.c 修正。
	mob.c 修正。
	mob.h 修正。
	pc.c 修正。
	map.h 修正。
	skill.c 修正。
	skill.h 修正。
	itemdb.c 修正。
	battle.c 修正。
	battle.h 修正。
	(conf/)
	battle_athena.conf 修正。
	(db/)
	cast_db.txt 修正。
	skill_db.txt 修正。
	create_arrow_db.txt 修正。
	class_equip_db.txt 修正。
	item_db.txt 修正。
	(doc/)
	client_packet.txt 修正。
	conf_ref.txt 修正。

--------------
//0608 by sk
・アマツNPC追加
	(conf/)
		npc_town_amatsu.txt 城内NPC追加
		npc_warp_amatsu.txt 城内ワープポイント追加

--------------
//0607 by J
・アサルトタートルの手下召喚のバグ修正(報告ありがとうございます るるるさん)
	(db/)
		mob_skill_db.txt アサルトの修正ついでにテレポを使うMOBのスキルディレイも修正

--------------
//0606 by 引退人
・スキルレベル最大値以上にクリックした時点で他スキルが上げられなくなるバグを修正（Thanx to 227さん）
	(map/)
	clif.c
		clif_skillup()
		スキルレベルが最大値のとき、パケット末尾を0にするように修正

--------------
//0605 by るるる

・武器属性付与スキルの不都合修正
	武器を持ち替えたり外したりした場合も、属性付与を解除するようにしました。
	但し、素手＝＞武器装備のみ状態維持します。
・スピアクイッケンのステータスアイコンを正しく表示
・２ＨＱ、スピアクイッケン、アドレナリンラッシュで該当以外の武器持ち替えで状態消滅
	スピアクイッケンは未確認ですが、２ＨＱは確実なので２ＨＱと不公平な仕様とは
	考えにくいので同様なパターンとしました。本鯖と相違がある場合は報告願います。
・敵のＱＭで集中力向上と速度上昇を解除
	ＱＭで影響するスキルはこれ２つだけかな？　私の記憶と掲示板での報告とで
	判断したのですが、もし相違がありましたら報告願います。

	(map/)
	clif.c
		clif_parse_UnequipItem()  修正
	pc.c
		pc_checkallowskill() pc_equipitem()  修正
	skill.h
		skill_encchant_eremental_end()  追加
	skill.c
		skill_status_change_start() skill_status_change_end()
		skill_status_change_clear() skill_encchant_eremental_end()  修正
	その他細かいところ少々

--------------
//0604 by J
・MOBスキル再修正
・MOBDB修正
	(db/)
	mob_skill_db.txt
		アークエンジェリングとタートルジェネラルが1回に2種類までしか
		MOBを出さなかったのを修正
	mob_db.txt 
		ロードオブデスのドロップでエラーが出るのを修正(未確認)
		怨霊武士のドロップとMVPを追加(未確認)
--------------
//0603 by 引退人
・新規アイテム時にも所持可能個数チェックをするように修正
	(map/)
	pc.c
		pc_checkadditem()
			新規アイテム時にMAX_AMOUNTを超えていたら
			ADDITEM_OVERAMOUNTを返すように修正

--------------
//0602 by 引退人
・Geffen鍛冶屋で落ちる問題を修正
	(conf/)
	npc_town_geffen.txt	if (!checkweight(,)) から if (!(checkweight(,))) に修正

--------------
//0601 by J
・MOBスキルの危ない所をいくらか修正
・覚醒と狂気の使える職を修正
・ゲフェニアダンジョンの配置をカボチャイベントで入れたときの配置に修正
	ただしボスがDOP2体ではなくドラキュラにしています。
	(conf/)
		npc_monster.txt モンスター配置微変更
	(db/)
		mob_skill_db.txt	怪しい設定などの修正
		item_db.txt		増速POTの修正

--------------
//0600 by 引退人
・プロンテラ精錬所の横のファンに話し掛けると固まる問題を修正
・MOBスキル取り込み（Thanx to Jさん）
	(conf/)
		npc_event_skillget.txt	ファンのLabelを修正
		npc_town_prontera.txt	ファンが重複していたので削除
	(db/)
		mob_skill_db.txt	ジュノー以降のMOBスキル追加

--------------
//0599 by るるる

・セージの武器属性付与スキルの不都合修正とステータスアイコン表示
	アスペルシオとエンチャントポイズンとで多重にかかってしまってたので、
	最後に付与したもの１つになるようにしました。
	そのついでにステータスアイコンも表示するようにもしました。
	（未テストですが、アイコン出なかったスピアクイッケンもでるはずです。）
・アイテムDBにて、増速ポーションの使用制限を追加
	Jazzさん提供です。
	それと私の趣味でラグナロクTシャツをアレナニしましたがｗ　気に入らなければ
	消すなり元通りに修正するなりしてしちゃってくださいませ～。

	(conf/)
		battle_athena.conf  598での入れ忘れ
	(map/)
		skill_encchant_eremental_end()  追加
		skill_status_change_end() skill_status_change_start() skill_status_change_clear()  修正
	(db/)
		iten_db.txt  修正

--------------
//0598 by るるる

・装備武器の画像変更に対応
	一応ながらクルセイダーの両手槍とかプリーストの鈍器とかはテストしましたが、
	全ての職をチェックはいません。またこの時点ではクライアント自体の表示データに
	問題の有るのが多いのも付け加えておきます。
	あと、靴も一応は対応しました。但しこれは現時点では本鯖すらも未対応なのですが。
	表示が化けて嫌だという場合は従来のやり方も出来ます。

	(conf/)
	battle_athena.conf
		オプション equip_modifydisplay を追加
	(map/)
	battle.h
		Battle_Config  修正
	battle.c
		battle_config_read()  修正
	clif.c
		packet_len_table[]  clif_changelook()  修正
	map.h
		enum {} 修正
	pc.c
		pc_calcstatus() pc_equiplookall() pc_changelook()  修正
	(common/)
	mmo.h
		mmo_charstatus {}  修正

--------------
//0597 by 波浪

・アマツに関する修正＆微修正
	(conf/)
	npc_mob_job.txt
	npc_monster.txt
	npc_monster30.txt
		モンス名修正
	npc_monster_amatsu.txt
		追加（モンス数がかなり手抜きです・・・
	npc_town_amatsu.txt
			ショップNPCを統合（npc_shop3.txtを消してもOKです
	(db/)
	mob_db.txt
		アマツのモンスデータを現在分かる範囲で修正＆草ときのこのdef,mdefを修正

--------------
//0596 by 死神

・0595の修正と細かい修正。
・フリーキャストでキャストしている間は攻撃可能ですがキャストした後の
ディレイタイムでは攻撃できないようになっています。本鯖の仕様がどうなのかは
わかりません。
・動いているPCにモンスターが攻撃できない問題修正。(テストしてませんが
多分これで大丈夫かと。)
	(map/)
	skill.h
		SC_FREECAST 削除。
	skill.c
		skill_use_id()、skill_use_pos() 修正。
		skill_castend_id()、skill_castend_pos() 修正。
		その他少し修正。
	pc.c
		calc_next_walk_step()、pc_attack_timer()、pc_calcstatus() 修正。
	clif.c
		 clif_parse_ActionRequest()、clif_parse() 修正。
	map.h
		struct map_session_dataにprev_speed追加。
	mob.c
		mob_ai_sub_hard()、mob_changestate()、mob_attack() 修正。

--------------
//0595 by PRevEv
・フリーキャスト修正、実装(キャスティング中攻撃もできます。)
	(/map)
	pc.c
		pc_calcstatus() 修正。
		calc_next_walk_step() 修正。
		pc_attack_timer() 修正。
	skill.c
		skill_castend_id()、skill_castend_pos()、skill_use_id()、skill_use_pos() 修正。

--------------
//0594 by 死神

・韓国鞍のパーティ問題修正と細かい修正。
・@partyコマンド修正と@guildコマンド追加。
・battle_athena.confにguild_emperium_check追加。
・スキル使用が失敗してもディレイがかかる問題修正。
	help.txt 修正。
	(map/)
	clif.c
		clif_parse_CreateParty2() 追加。
		clif_parse_ItemIdentify() 修正。
		その他少し修正。
	atcommand.h、atcommand.c 修正。
	battle.h
		struct Battle_Configにguild_emperium_check追加。
	battle.c
		battle_config_read() 修正。
	guild.c
		guild_create()、guild_created() 修正。
	skill.c
		skill_castend_id()、skill_castend_pos() 修正。
	Makefile 修正。
	(doc/)
	client_packet.txt
		パケット0x01e8 追加。
	conf_ref.txt 修正。
	(conf/)
	atcommand_athena.conf 修正。
	battle_athena.conf 修正。

--------------
//0593 by 死神

・メテオとバミリオンのダメージ修正と細かい修正。
・メテオの隕石が落ちてくる範囲を7*7から5*5に変更。(これで中央は全ての隕石の
ダメージを受けることになります。)
・自動回復計算式変更。
	スキルレベル*5 + (max_hp/50)から
	スキルレベル*5 + (max_hp*スキルレベル/500)に変更。(SPと息吹も同じように変更。)
・GM右クリック命令「使用者強制終了」でatcommand_athena.confのkickの
レベルをチェックするように変更。
	(db/)
	skill_db.txt
		スキル気功のspを10から8に修正。
	(map/)
	map.c
		NO_WATERを100から1000000に変更。
	battle.c
		battle_calc_magic_attack() 修正。
	skill.c
		skill_castend_pos2() 修正。
	pc.c
		pc_natural_heal_hp()、pc_natural_heal_sp()、pc_spirit_heal() 修正。
	clif.c
		clif_parse_GMKick() 修正。
	(doc/)
		conf_ref.txt 修正。

--------------
//0592 by 引退人

・水場の有り無しをwater_height.txtだけで決めるように変更。npc_water.txtは不要に。
・カードスキルではレベル上げできないようになったため不要になった処理を削除。
	(conf/)
	map_athena.conf
		npc: conf/npc_water.txt 消去
	water_height.txt
		デフォルト高さ3のマップ分を追加＆all_waterを高さ-100としてコメントで追加
	(map/)
	map.c
		waterlistはmap_readwater()内でメモリ確保
		gat設定後は不要なのでmap_readallmap()でメモリ開放しています
	map.h
		struct map_dataのflagからwater_flagを消去
	npc.c
		npc_parse_mapflag()
			マップフラグwaterとall_waterを消去
	pc.c
		pc_skillup()
			裏でスキルLvUPできなくなったのでskill[id].flagの分は消去
	skill.c
		skill_check_condition()
			map_getcellで水場判定するように修正

--------------
//0591 by CHRIS

・モンクが氣弾をもっているとき、氣弾*3の必中ダメージが入る様になりました。─　battle.c修正
・モンクスキル「指弾」と「発勁」と「気孔」の詠唱時間が正しく修正されました。─　cast_db.txt修正

--------------
//0590 by 死神

・grf-files.txtやconfファイル、account.txtファイルの名前と位置を変えるように変更。
・マップ移動による鯖落ちを防ぐ為に修正。(マップが二度ロードされて鯖落ちが
起こったとの報告を受けたので。)
・0586をちょっと修正。装備によるスキルの場合レベル上げができないように修正。
ただ装備によるスキルをスキルポイントを使って上げる時は装備を外す必要が
あります。
・@partyちょっと修正。(名前に空白があっても大丈夫なように。)
・水場の高さをwater_height.txtで読み込むように変更。
・confファイルで設定する物をファイルのパスに空白があっても大丈夫な
ように変更。
・GM右クリック命令「使用者強制終了」でモンスターを倒せるように変更。
(原因は不明ですがこれでモンスターを殺すと鯖がめちゃくちゃに遅くなることが
あります。)
・その他少し修正。
・テストは殆んどしてませんので注意してください。
	athena-start 修正。
	(map/)
	pc.c
		pc_skill()、pc_resetskill()、pc_setpos()、pc_read_gm_account() 修正。
		pc_set_gm_account_fname() 追加。
	pc.h
		pc_set_gm_account_fname() 追加。
	clif.c
		clif_skillinfoblock()、clif_parse_LoadEndAck()、clif_parse_GMKick() 修正。
		clif_changemap() 修正。
	atcommand.c
		@partty 修正。
	skill.c
		skill_castend_nodamage_id() 修正。
	map.c
		map_config_read()、map_readwater() 修正。
	script.c
		script_config_read()、do_init_script() 修正。
	script.h
		script_config_read() 追加。
	(common/)
	version.h 修正。
	grfio.h
	grfio.c
		grfio_init() 修正。
	mmo.h
		GRF_PATH_FILENAME 追加。
	(conf/)
	map_athena.conf 修正。
	npc_water.txt 修正。
	water_height.txt 追加。
	login_athena.conf 修正。
	(login/)
	login.c
		login_config_read()、read_gm_account() 修正。
	(char/)
	char.c
		do_init() 修正。
		char_config_read() 追加。
	inter.c
		inter_config_read() 修正。
	(doc/)
	conf_ref.txt 修正。

--------------
//0589 by 胡蝶蘭

・GMの右クリックで切断される問題を修正
	とりあえず01dfパケットを無視するようにしました。
	なんとなくこのパケットはチャット禁止回数とは関係無いような気も……

	clif.c
		clif_parse_GMReqNoChatCount()追加

・startをathena.shでなくathena-startを使うように変更
	start
		athena.sh => athena-start startに置き換えしただけ

--------------
//0588 by Kalen

・AmatsuNPC追加

--------------
//0587 by 胡蝶蘭

・loginサーバーの管理パケットの仕様を変更（0579のログイン拒否情報に対応）
	(login/)
	login.c
		アカウントバン状態変更パケット追加(7936,7937)
		アカウントリスト所得パケット修正(7921)
	(doc/)
	admin_packet.txt

・ladminの機能追加
	・バン状態を変更するコマンド追加
	・リスト表示と検索でバン状態も表示されるようになった
	・"?"でもヘルプが出るように修正
	・シンボリックリンクにstateaccountが追加されました。
	  使う人はladminの--makesymlinkをもう一度実行してください

	(tool/)
	ladmin
		機能追加

--------------
//0586 by 引退人
・カードスキルを修正
	(/map)
	pc.c
		pc_calc_skilltree()
		pc_skill()
			覚えられないスキルならskill[id].flag=1とする
			またはskill[id].flagに本来のlvを+2して記憶
		pc_skillup()
			skill[id].flagも増やす
	clif.c
		clif_skillinfoblock()
			skill[id].flag==1なら覚えられないスキル
	(/char)
	char.c
		mmo_char_tostr()
			skill[id].flagから本来のlv値を保存する

--------------
//0585 by kalen
・script修正
	npc_town_guid.txt	外見変更

--------------
//0584 by 引退人
・カードスキルを修正
	(/map)
	pc.c
		pc_calc_skilltree() cardスキルを忘れさせる処理を追加
		pc_skill() 整理

--------------
//0583 by kalen
・script修正
	npc_event_doll.txt	抜けていた部分の会話追加
	npc_town_guid.txt	町の案内要員の画像を表示できるように修正
				基本的に最新jROで問題なしです。

--------------
//0582 by PRevEv
・580のバグ修正。
	(/map)
	skill.c
		skill_use_pos() 修正。
--------------
//0581 by 引退人
・水場高さ設定関連を少し修正
	(/map)
	map.c
		map_waterheight()
		map_readwater()
		map_readmap()
			waterlist[512] -> *waterlistにして、mallocでメモリ確保するように修正。
		map_readallmap()
			free(waterlist);追加

・バグ報告スレッドの修正パッチを取り込み
	(conf/)
	npc_town_refine.txt セミコロン抜け修正
	(db/)
	item_db.txt 1161,バルムン修正

--------------
//0580 by PRevEv
・フリーキャスト仮実装(キャスティング中攻撃は不可能)
	(/map)
	clif.c
		clif_parse_WalkToXY() 修正。
	pc.c
		pc_calcstatus() 修正。
	skill.c
		skill_castend_id()、skill_castend_pos()、skill_use_id()、skill_use_pos() 修正。
--------------
//0579 by Aの人
・ログインを弾く処理を仮実装
	conf/login.c
		auth_dat構造体にstateを追加
		mmo_auth関数修正
		mmo_auth_new関数修正
		mmo_auth_sync関数修正
		mmo_auth_init関数修正

	この値を変更するツール、改善策。他力本願です（＞＜；
--------------
//0578 by 引退人
・バグ報告スレッドの修正パッチを取り込みなど（thanx to るるるさん,Athefansさん,sageさん,zupportさん）
	help.txt	@goの説明ちょっと修正
	(conf/)
	npc_mob_job.txt	第4列目をTab区切りに修正
	npc_water.txt	水場高さ修正
	(db/)
	item_db.txt	640,...,{ pet 1155; },{},,に修正
	mob_db.txt	1162,RAFFLESIA,ラフレシア...修正

--------------
//0577 by るるる
・@コマンドを追加＆修正
	atcommand.c
		atcommand()		@itemresetコマンド追加  @goコマンド修正（アマツ・コンロンを追加）
		atcommand_config_read()	上に合わせてitemreset使用レベル指定を追加

	doc/conf_ref.txt
	conf/atcommand_athena.conf	itemreset使用レベル指定を追加
	help.txt			@itemresetの説明追加と@goの説明修正

-------------
//0576 by V&S 
・ゴーストリングカードとバースリーカードの効果が逆になっていたのを修正
	{ bonus bDefEle,7; }→ゴーストリングカード(念)
	{ bonus bDefEle,8; }→バースリーカード(闇)
	↑だったのを↓に修正
	{ bonus bDefEle,7; }→バースリーカード(闇)
	{ bonus bDefEle,8; }→ゴーストリングカード(念)

	鋼鉄の重量を修正

--------------
//0575 by 引退人
・水場ファイルが"conf/npc_water.txt"固定だったのを修正
（conf/map_athena.confのnpc:に書かれているファイルをみて水場高さ設定するように）
	map.c
		struct waterlist[512];	新規追加。マップファイル名と水場高さを記憶。
		map_waterheight()	新規追加。水場の高さを返す。
		map_readwater()		水場ファイルをみてwaterlistを設定するように修正。
		map_readmap()		map_waterheight()を呼ぶように修正。
		map_config_read()	"npc"でmap_readwater(w2);追加。

--------------
//0574 by いど

・サーバーSnapshot
・conf/shop_*.txtの内容をconf/npc_town_*.txtに統合

--------------
//0573 by Jazz

・mapの penalty, nomemo, noteleport, nobranchの optionを設定。
・map サーバーが cpuを無限占有することを検査するための script 添付. cygwin環境で作成と実験をしました.
	(/conf)
	mapflag.txt 追加。
	(/tool)
	mapcheck.sh 追加。

--------------
//0572 by 引退人
・"conf/npc_water.txt"の第4列で水場高さ設定
　（ちゃんとした水場判定が実装されるまでのつなぎとして・・・）
	(/conf)
	npc_water.txt サンプル修正。
		・第4列で水場の高さを設定します。
		・高さを書かなかった場合のデフォルト値は3になります。
	(/map)
	map.c
		・水場高さ設定関数 map_readwater() 追加。

--------------
//0571 by code
天津フィールドのMOBの配置とワープポイントの設定の修正
天津パッチに崑崙が含まれているのを確認したので崑崙のワープとmobを配置

conf/npc_monster35.txt
     mobの配置

conf/npc_warp_amatsu.txt
　　 天津warp pointの設置

conf/npc_warp_gonryun.txt
     崑崙warp pointの設置

conf/npc_town_amatsu.txt
　　 暫定的にプロンテラ噴水前←→天津港／プロンテラ噴水前←→崑崙の接続NPC

--------------
//0570 by code
天津フィールドのMOBの配置とワープポイントの設定です。
conf/npc_monster35.txt
     mobの配置

conf/npc_warp_amatsu.txt
　　 warp pointの設置

conf/npc_town_amatsu.txt
　　 暫定的にプロンテラ噴水前←→天津港の接続NPC

--------------
//0569 by 死神

・0561の@jobcange での♀バード＆♂ダンサーによる鞍落ち防止をpc_jobchange()でするように変更。
・@コマンド@party追加。パーティを作る命令です。韓国鞍を使うとパーティを作る時止まるので臨時的にこれを使ってパーティを作ってください。
・水の判断処理修正。
・マップフラグにwaterとall_water追加。詳しくはnpc_water.txtを参考してください。
iz_dun0xだけ入力していますので他のは埋めてください。マップフラグwaterかall_waterが入ってないとセルのtypeが3でも水として認識しません。そして水だらけのiz_dun02から04までは全て水として認識するようにall_waterを入れています。(これ以外は方法がなかったので...)
・battle_athena.confに項目追加。一部はWeissを参考して作った物です。
・テストしてない物も少しあります。
	(/conf)
	atcommand_athena.conf 修正。
	battle_athena.conf 修正。
	map_athena.conf 修正。
	npc_water.txt 追加。
	(/doc)
	conf_ref.txt 修正。
	(/map)
	atcommand.h、atcommand.c 修正。
	battle.h 修正。
	battle.c
		battle_config_read() 修正。
	pc.c
		pc_jobchange()、pc_stop_walking() 修正。
	npc.c
		npc_parse_warp()、do_init_npc()、npc_parse_mapflag() 修正。
	mob.c
		mob_ai_sub_hard() 修正。
	pet.c
		pet_food() 修正。
	skill.c
		skill_check_condition() 修正。
	map.h
		struct map_data 修正。

--------------
//0568 by 引退人

・アクアベネディクタ水場判定など
・ウォーターボール水場判定（read_gat(m,x,y)==3で水場と判定）
	skill.c
		skill_castend_nodamage_id()
			case AL_HOLYWATER:	アクアベネディクタ（聖水取得）
		skill_check_condition()
			case AL_HOLYWATER:	アクアベネディクタ（水場判定）
			case WZ_WATERBALL:	ウォーターボール（水場判定）

--------------
//0567 by るるる

・アコライトのアクアベネディクタを仮実装（水場限定使用のみ未実装）
・プリーストのアスペルシオ、セージのフレイムランチャーでスキル使用時にアイテム消費
・ミストレスカード装備時にセージの属性原石＆ハンターの罠が消費されないバグを修正

	(/map)
	skill.c
		skill_check_condition() 修正

--------------
//0566 by パイン

・0563のスキル解除条件が間違っていたので修正。武器をはずす&武器を変えた場合は
　無条件で解除するようにした。

# pc_checkallowskill について(前回説明書くのを忘れていたので…)
　一応今後の含みとしてreturnを返すようにしていますが、現在は(戻り先では)使っていません。
　今現在は騎士・クルセイダーくらいしかスキル使用時の武器制限がありませんが、今後出てこない
　とも限らないので、もし(2次上位か3次？)出てきたらここでチェックしてください。

	(/map)
	pc.c
		pc_checkallowskill() 修正

--------------
//0565 by 引退人

・マップ移動時に矢装備が外れないように修正
・ログイン時に矢装備が表示されるように修正
	(/common)
	mmo.h
		（矢装備は0x8000なので）shortだとintへのキャスト時などに
		負値となってしまうためunsigned shortに修正
		struct item
			short equip; -> unsigned short equip;
	(/map)
	clif.c
		clif_itemlist() アイテムリストの矢のついでに矢装備もチェック
		clif_arrowequip() シンプル化
	pc.c
		pc_equipitem() 修正

--------------
//0564 by 紅葉

・@modelの服染め不可能判定修正。
・@modelで、選べるハズの髪形に変更出来なかった部分を修正。
　上記変更点に合わせてhelp.txtの修正。

--------------
//0563 by パイン

・MOBの暗闇スキルを食らった後に回復しないのを修正。…なんだけど、適正な値が分からないので
　毒や沈黙と同じ時間にしてあります
　今後、また手を加えるかもしれません。
・2HQとスピアクイッケンを使用中に武器を変えた場合は解除するように変更。

	(/map)
	skill.c
		skill_castend_damage_id() 修正。
		skill_status_change_timer() 修正。
	pc.c
		pc_checkallowskill() 新設。
		pc_equipitem() 修正。
	pc.h
		pc_checkallowskill() 新設。

--------------
//0562 by huge

・矢を弓装備時以外でも装備できるように戻しました。
・矢の属性を適用するのを弓装備時のみに修正。

	pc.c
		pc_equipitem() 修正。
		pc_calcstatus() 修正。

--------------
//0561 by 引退人

・Linuxでもコンパイルできるように
	(/map)
	skill.c
		skill_castend_damage_id()	変数dx,dyの宣言位置変更
	Makefile
		LIBS に -lm 追加

・@jobcange での♀バード＆♂ダンサーによる鞍落ち防止。 by (no name)さん
	atcommand.c
		@jobchange,@charjobに性別チェック追加

// ナナスさん修正
・clif.c内でatcommand.hを２度includeしていたので一つ削除。
・パーティー会話、ギルド会話でも@コマンドをチェックするように修正。
	(/map)
	clif.c
		clif_parse_PartyMessage()、clif_parse_GuildMessage 修正。

--------------
//0560 by パイン

・0559 の athena-start を Unix Like OS でも動くようにリファイン。

--------------
//0559 by rowla

・athena.shを全面的に書き直し、athena-startに。athena-start startで開始、athena-start stopでサーバー停止。cygwinでテスト、*BSD|Linuxでは未テスト(環境がないため)。

--------------
//0558 by 死神

・ブリッツビートを自動だけ弓を装備していないと発動できないように変更。(手動は武器に関係なく使えます。) 未テスト。
・トラップの重さ修正。(何故かは知らないけど100になっていたのを10に修正。因みに倉の表示は100が10で10が1です。)
・弓で使うスキルの場合矢が減らないのが仕様だったと覚えているので矢をチェックしないように修正。
・モンクスキル三段掌の表示をパッシブに変更。
・マップフラグをセットする時dummyがなくてもセットできるように修正。
(mapflag nomomo dummyからmapflag nomemoでも大丈夫なように変更。)
未テスト。
	(/db)
	item_db.txt 修正。
	skill_db.txt 修正。
	(/map)
	skill.c
		skill_check_condition()、skill_additional_effect() 修正。
		skill_status_change_start() 修正。
	npc.c
		do_init_npc() 修正。

--------------
//0557 by huge

・矢を、弓装備時のみ装備できるように修正。
・弓を装備から外したら、矢も外れるように修正。
・矢を消費するスキルをいくつか修正。
・鷹を、弓を装備しているときのみ発動するように修正。(未テスト)

	pc.c
		pc_equipitem() 修正。
		pc_unequipitem() 修正。
	skill.c
		skill_additional_effect() 修正。
		skill_check_condition() 修正。

--------------
//0555 by 死神

・細かい修正とプレゼントボックス、古い巻物のバグ修正。
・@コマンド@refine、@produce少し修正。
・サーバーのIPにDNS名を使えるように変更。(今さらですがYareから
持ってきた物です。)
・スティール計算式変更とMVPアイテム処理変更。
・店NPCを利用によるジョブ経験値獲得計算式変更。
	獲得ジョブ経験値 = ln(金*スキルレベル) * shop_exp / 100
・ほとんどテストしてないのでバグの可能性があります。
	help.txt 修正。
	(/conf)
	atcommand_athena.conf 修正。
	battle_athena.conf 修正。
	(/db)
	item_db.txt 修正。
	(/doc)
	conf_ref.txt 修正。
	(/char)
	char.c
		do_init()、check_connect_login_server() 修正と少し修正。
	(/map)
	mob.c
		mob_damage() 修正。
	pc.c
		pc_getitemfromcart()、pc_steal_item() 修正。
	pet.c
		pet_return_egg()、pet_get_egg()、pet_unequipitem() 修正。
	script.c
		buildin_getitem() 修正。
	skill.c
		skill_produce_mix() 修正。
	storage.c
		storage_storageget() 修正。
	atcommand.c 修正。
	map.c
		map_config_read() 修正と少し修正。
	chrif.c
		check_connect_char_server()、do_init_chrif()、chrif_setip() 修正と少し修正。
	npc.c
		npc_buylist()、npc_selllist() 修正。

--------------
//0554 by NOCTURNE
・サーバーSnapShot
・too/addaccountの削除
・help.txtの更新

--------------
//0553 by 胡蝶蘭

・ladminのバグ修正と機能追加
	・キーワードによるアカウント検索機能追加
	・シェルコマンドとして使用できるようにプロンプトを使わないモード追加
	・追加機能についてはladminを見てください
	・ ladminの--makesymlinkにより、シンボリックリンクとしてaddaccountを
	  作成するため、以前のaddaccountは削除する必要があります。
	  これらのシンボリックリンク(Cygwinではショートカット)と、
	  古いaddaccountは鯖snapshotには含まないで下さい。
	
	(tool/)
	ladmin
		機能追加と修正

・女性アカウントしか作成できないバグ修正
・ladmin、checkversion使用時loginサーバーが暴走するバグ修正
・GMアカウント周辺のIDを避けるためにSTART_ACCOUNT_NUMを変更
	(既にGMアカウントは避ける仕様になっていますが、混乱防止のため)

	(login/)
	login.h
		START_ACCOUNT_NUMを500000から2000000に変更
	login.c
		7532(切断)パケットの処理修正
		mmo_auth_new()修正

・backupがバックアップするファイルにpet.txtを追加
	(tool/)
	backup
		ファイル追加修正

--------------
//0552 by 死神

・安定性を上げる為の修正ですが本当に安定性上がったか
どうかは不明です。
・PVPによりクライアントが落ちる問題修正。
	atcommand.c
		@pvpoff、@pvpon、@gvgon、@gvgoff 修正。
	script.c
		buildin_pvpon()、buildin_pvpoff()、buildin_gvgon()、buildin_gvgoff() 修正。
	clif.c
		clif_pvpset() 修正。
	skill.c
		skill_attack()、skill_unit_onplace()、skill_unit_onout() 修正。
		skill_unit_ondelete() 修正。

--------------
//0551 by Kalen
・DB修正
	db/create_arrow_db.txt 完成
	SourceID順にソートしました。

--------------
//0550 by huge

・矢作成スキル実装

	clif.c
	clif.h
		clif_arrow_create_list() 追加
		clif_arrow_created() 追加
		clif_parse() 修正

	pc.c
		pc_search_inventory() 修正

	skill.c
	skill.h
		skill_arrow_db() 追加
		skill_readdb() 修正
		skill_castend_damage_id() 修正

	db/create_arrow_db.txt 追加
	db/skill_db.txt 修正

	まだdbは未完成です。

--------------
//0549 by Kalen

・map_athena.conf
	オリジナルスクリプト、季節限定スクリプトを整頓
	shop3.txt追加

・各種NPC追加＆修正
	npc_event_yuno.txt	[追加]ジュノーイベント(青石5個GET)
	npc_cTower.txt		[追加]地上地下の鍵NPC
	npc_town_yuno.txt	[修正]台詞修正

	npc_event_carnival.txt	[追加]旧鯖カーニバルイベント時のNPC

--------------
//0548 by huge

・矢を装備した時の表示バグ問題を修正。
	clif.c
		clif_arrowequip() 修正。
	pc.c
		pc_equipitem() 修正。

あとは、マップを移動するたびに装備が外れちゃう点ですね・・・。

--------------
//0547 by 死神

・安定性を上げる為の修正と細かい修正。
・スキルユニットの判定をしている間メモリーを解除できないように変更。
	map.c
		map_foreachinarea()、map_foreachinmovearea() 修正。
		map_foreachobject() 修正。
		block_free_maxを32000から50000に変更。
	pc.c
		pc_calcstatus()  修正。
	skill.c
		do_init_skill()、skill_unit_timer()、skill_status_change_clear() 修正。
	skill.c、battle.c、battle.h
		struct battle_configのsanctury_typeをsanctuary_typeに変更。
		(英語スペル間違いで修正。)
	battle_athena.conf
		sanctury_typeをsanctuary_typeに変更。
	conf_ref.txt
		sanctury_typeをsanctuary_typeに変更。

--------------
//0546 by 獅子o^.^o

conf/npc_shop2.txt
．バ一ド、ダンサ一用の武器。コモドの武器屋で販売している。
．モンク用の武器。カピト一リナ修道院で販売している。

--------------
//0545 by 死神

・ブリッツのダメージを自動で分散、手動で普通になるように変更。
・オートブリッツバグ修正。(これで大丈夫だといいけど...)
	map.c
		block_free_maxを16000から32000に修正。
		block_list_maxを4096から5120に修正。
	battle.c
		battle_weapon_attack() 修正。
	skill.c
		skill_attack()、skill_castend_damage_id() 修正。

--------------
//0544 by Diex
・猛龍拳から阿修羅覇凰拳へのコンボ実装。
・阿修羅覇凰拳発動後、敵の背後に移動するよう、修正。
・三段掌のダメージ修正。
	(/map)
	skill.c
		skill_castend_damage_id() 修正。
		skill_check_condition() 修正。
		skill_use_id() 修正。
	pc.c	
		pc_attack_timer() 修正。
		pc_authok() 修正。
	battle.c
		battle_calc_weapon_attack() 修正。
	battle.h
		struct Battle_Config 修正。
	map.h
		struct map_session_data 修正。
	(/conf)
	battle_athena.conf 修正。

はっきりいってコンボ繋げづらいです。そのため阿修羅へのコンボはかなり甘い判定に
してます（一時的にですが）。繋げづらければbattle_athena.confのほうでデュレイ時
間を大きくしてみてください。
  動画見てて気づいたのですが、阿修羅覇凰拳は猛龍拳が発動した後、即時発動のスキ
ルに変わってるようなのです。他力本願ですが、阿修羅までのコンボのパケを記録した
物をどなたかアップしてもらえないでしょうか？詳細がわかり次第、修正します。

--------------
//0543 by 死神

・ブリッツのダメージを分散されるように変更。
・普通のアカウント作りではGMアカウントを作れないように変更。
(前に自分が入れた物がなくなったので戻しただけですが...)
・取り巻きが主と一緒に死ぬように変更。(ただちょっと重くなる
可能性があります。) 未テスト。
・MVP経験値が表示だけされて実際には入ってない問題修正。
	(/login)
	login.c
		mmo_auth_new() 修正。
	(/map)
	skill.c
		skill_castend_damage_id() 修正。
	battle.c
		battle_calc_misc_attack() 修正。
	mob.c
		mob_damage() 修正。
		mob_deleteslave()、mob_deleteslave_sub() 追加。

--------------
//0542 by 死神

・オートブリッツバグ修正。(今度こそ大丈夫のはず...)
・自分に使ったヒールでは経験が入らないように変更。
・店NPCを利用によるジョブ経験値獲得計算式変更。
	獲得ジョブ経験値 = ln(金) * shop_exp / 100
になります。
logを使うことで金が多くても入る経験値が多く入らないように変更しました。
	(/map)
	battle.c
		battle_damage() 修正。
	skill.c
		skill_attack()、skill_castend_damage_id() 修正。
		skill_castend_nodamage_id 修正。
	npc.c
		npc_buylist()、npc_selllist() 修正。
	map.c
		map_foreachinarea()、map_foreachinmovearea()、map_foreachobject()
		修正。(大した修正ではないです。)
	(/conf)
	battle_athena.conf 修正。
	(/doc)
	conf_ref.txt 修正。

--------------
//0541 by huge

・矢をまとめて持てるように修正。
・弓で攻撃したときに、装備している矢を消費するように修正。

	itemdb.c
		itemdb_search() 修正
		itemdb_isequip() 修正

	battle.c
		battle_weapon_attack() 修正
		battle_calc_weapon_attack() 修正

	clif.c
	clif.h
		clif_arrow_fail() 追加
		clif_parse_EquipItem() 修正

--------------
//0540 by 死神

・バグ修正と問題ありそうな所修正。(これでWZ_FIREPILLARとブリッツに
よる鯖ダウンはなくなるはず...)
	map.c
		map_foreachinarea()、map_foreachinmovearea() 修正。
	skill.c
		skill_unitsetting()、skill_delunitgroup() 修正。
	pc.c
		pc_damage() 修正。
	battle.c
		battle_damage() 修正。
	npc.c
		npc_parse_mob() 修正。
	mob.c
		mob_spawn_dataset() 修正。

--------------
//0539 by 死神

・clif_pvpset()をマップからAREAかマップかを設定できるように変更。(pvpの時の処理は0535以前の物に戻そました。自分だけに転送してもいいような気もしますが...)
	clif.h、clif.c
		clif_pvpset() 修正。
		clif_parse_LoadEndAck() 修正。
	script.c
		buildin_pvpoff() 修正。
		buildin_pvpon() 修正。
	atcommand.c 修正。
・攻撃途中でアイテムを拾うと攻撃が止まるように修正。
	pc.c
		pc_takeitem() 修正。
・0535説明が爆裂波動になっているがそれは金剛に間違いです。
・0537で説明を忘れましたがモンスターのdefとmdefを10000以上に設定すれば全ての攻撃に1ダメージになるモンスターになります。そしてモンスター情報でdefと
mdefが10000以上の場合def 100、mdef 99に表示するように変更。本鯖仕様に
するにはmob_db.txtを修正してください。

--------------
//0538 by huge

・グリムトゥースを範囲攻撃に修正
・サプライズアタック実装 (有効範囲って、これであってるのかな？)
・バックスタブの仮実装
	本鯖でやってる人から話を聞いて、場所指定じゃなくて
	タゲ取っても良さそうだったので変更しました。（やりやすかったので (^^;
	まだ、mobの後ろに居るかどうかの判定は入ってません。

・battle.c
	battle_calc_weapon_attack() 修正

・skill.c
	skill_additional_effect() 修正
	skill_castend_damage_id() 修正
	skill_check_condition() 修正
	skill_use_id() 修正
	skill_castend_nodamage_id() 修正

・skilldb.txt
	バックスタブの種類を[場所]から[敵]へ変更

--------------
//0537 by 死神

・スティールバグ修正とbattle_athena.confの項目追加、仕様変更と細かい修正です。
(スティールは計算式に問題があったので修正して確率を更に落としました。)
	battle.h
		finger_offencive_typeをfinger_offensive_typeに修正。(英語スペル間違いで修正しました。)
		struct battle_configにrestart_hp_rate、restart_sp_rate 追加。
	battle.c
		battle_calc_weapon_attack()、battle_calc_magic_attack() 修正。
	skill.c
		skill_attack() 修正。
		clif_skill_nodamage()にスキルレベルを送るように変更。(Mさんの指摘により修正。)
	clif.c
		clif_skill_estimation() 修正。
	conf_ref.txt
		finger_offencive_typeをfinger_offensive_typeに修正と少し追加。
	mob.c
		mob_ai_sub_hard()、mob_target()、mob_damage() 修正。
	pc.c
		pc_steal_item() 修正。
	atcommnad.c、atcommnd.h
		@コマンド@gvgon , @gvgoff 追加。
	battle_athena.conf
		finger_offencive_typeをfinger_offensive_typeに修正と少し追加。

--------------
//0536 by hogefuga3 (Athena staff)

・新GRFファイルフォーマット対応
　- Athena staff 様の作成されたパッチを適用しました。
    更新履歴の部分はパッチミスになったので手動で組み込み。

（変更）
	common/
		grfio.c

--------------
//0535 by 死神

・0533の問題がありそうな部分全て修正。修正した所を全部チェックしてなかったので修正したファイルだけ...
・スクリプトsetmapflagnosave 追加。
	setmapflagnosave マップ名、セーブするマップ名、座標(X、Y)
		nosaveフラグをonにします。
・battle_athena.confに追加と一部仕様変更。(詳しくはconf_ref.txtを参考してください。)
・モンスターのdefとmdefが10000以上の場合全ての攻撃(クリティカル含めて)が1ダメージになるように変更。(トラップやブリッツの場合両方が10000以上の場合のみ1になります。) 草とキノコに１ダメージ固定は削除しました。(元々本鯖でも1固定ではないです。精練等による引き上げダメージはそのまま出ますので... 固定したいのならdefとmdefを10000にしてください。実はこれはクリスタルに為に作ろうとした物ですが...)
・爆裂波動の時アイテムによるスキルは使用できるように修正。
・その他少し修正。(修正の物の中にテストしてない物もあります。)
	(/doc)
	conf_ref.txt 修正。
	(/conf)
	battle_athena.conf 修正。
	(/map)
	battle.h 修正。
	battle.c 修正。
	mob.h 修正。
	mob.c 修正。
	skill.c 修正。
	npc.c 修正。
	pc.c 修正。
	script.c 修正。
	clif.c 修正。
	chrif.c 修正。

--------------
//0534 by Diex

・コンボシステム仮実装
	map/
		battle.c
			battle_weapon_attack()	修正。
		clif.c
		clif.h
			clif_combo_delay()　関数追加。
		map.h
			map_session_data　変数追加。
		pc.c
			pc_authok()　変数追加。
			pc_attack_timer()　修正。
		skill.c
			skill_castend_damage_id()　修正。
			skill_check_condition　修正。
			skill_use_id　修正。
	db/
		skill_db.txt　修正。

注）猛龍拳から阿修羅覇鳳拳にはまだつなげません。
    阿修羅覇鳳拳を放った後、PCはMOBの背後(?)に移動してるっぽいのですが、
　　そこらへんの情報が足りません。情報提供お願いします。

--------------
//0533 by るるる

・草とキノコに１ダメージ固定
battle.c の battle_weapon_attack() と battle_calc_attack() を修正
battle.c の battle_get_mobid() を追加
mob.c の mob_makedummymobdb() と mob_readdb() を修正

・スキルログにモブの固有番号＆ＰＣのＩＤ番号を表示
（battle.c の battle_get_mobid() を追加したのでそのついでに）
mob.c skill.c の変更箇所多数（汗
（"MOB %d" もしくは "PC %d" で検索すれば変更箇所がわかるかと）

・ハンターの罠を使ったスキルで罠を消費するようにした
batttle.c の skill_check_condition() を修正
（ジェム消費処理の流用っぽいことをやってるんだけど処理中身は理解してないｗ）

・サンクチュアリ＆マグヌスのダメージ判定を不死属性＆悪魔種族に再度修正
0532で再び元に戻ってしまったのをなおしました。
ただし、回数＆人数判定には手を加えていません（ってか自分にはまだムリ）

以上。
切った貼ったの見様見真似でやったので言語的に果たしてこれでよいのか。。。
もし処理方法に問題有りだったら修正なりをしていただけると嬉しいです。

--------------
//0532 by 死神

・修正した所を全然チェックしてなかったので修正したファイルだけ...汗
・mapflagにnopenalty追加。使用方法は
	mapflag	nopenalty	dummy
です。機能はそのマップで死んだ時経験が減らないようにします。
・mapflagにpvp_noparty、pvp_noguild、gvg、gvg_noparty追加。
pvp_nopartyはPVPモードで同じパーティに攻撃が当たらない、pvp_noguildはPVPモードで同じギルドに攻撃が当たらない、gvgはシーズモードに、gvg_nopartyはシーズモードで同じパーティに攻撃が当たらない物です。
・可動してないタイマーは全て-1になるように変更。
・吹き飛ばし処理修正。
・マップロード直後気功が見えない問題修正。
・残影の処理修正。
・マップをロードすると死んだふりが解除されるように変更。
・PVPを少し変更。
・古木の枝で出るモンスターを自分のレベルより高い物は出ないように変更。
・加速ポーションのsc_start SC_SpeedPot0,1,0;をsc_start SC_SpeedPot0,1800,0;のように変更。SC_SpeedPot？の後の数値は持続時間です。(単位は秒)
・@コマンド@pvpを@pvponに変更と@pvponと@pvpoff、@gatの機能変更。
・battle_athena.confのpvp削除。
・battle_athena.confにdeath_penalty_type追加。
・ペナルティの適用を死んだ時から死んだ後リスタートした時に変更。(リザで復活すると経験が減りません。本鯖の仕様がかなり気にいらなかったので変更しました。)
・スクリプトsetmapflag、removemapflag、pvpon、pvpoff、gvgon、gvgoff追加。
	setmapflag マップ名、マップフラグタイプ
		指定したマップフラグをonします。(ただpvp、gvgはpvpon、gvgonでできるので指定しても動作しません。あとnosaveの場合処理がちょっと複雑になるので対応してません。)
	removemapflag マップ名、マップフラグタイプ
		指定したマップフラグをoffします。(ただpvp、gvgはpvpoff、gvgoffでできるので指定しても動作しません。こちらはnosaveも可能です。)
	pvpon マップ名
		指定したマップをPVPモードにします。
	pvpoff マップ名
		指定したマップのPVPモードを解除します。
	gvgon マップ名
		指定したマップをシーズモードにします。
	gvgoff マップ名
		指定したマップのシーズモードを解除します。
ただ全てのスクリプトの動作は確認してませんので注意してください。
・サンクチュアリ、マグヌスエクソシズムの処理を0529に戻しました。
自分の調査ではサンクチュアリは人数の制限があります。(レベル1で4名で
1レベルに一人ずつ増えます。)
・その後少し修正。(したはず...)
	(db/)
	const.txt 修正。
	item_db.txt 修正。
	(conf/)
	battle_athena.conf 修正。
	(doc/)
	conf_ref.txt 修正。
	(map/)
	clif.h、clif.c 修正。
	mob.c 修正。
	pc.h、pc.c 修正。
	skill.c 修正。
	pet.c 修正。
	npc.c 修正。
	map.h、map.c 修正。
	battle.h、battle.c 修正。
	atcommand.h、atcommand.c 修正。
	script.c 修正。
	makefile 修正。

--------------
//0531 by 獅子o^.^o

conf/npc_turtle.txt
．タートルアイランドに行く時、サ一バ一を落って問題修正
．npc_turtle.txtの508行目
set Zeny - 10000,0; --> set Zeny,Zeny-10000; 修正

--------------
//0530 by RR
・スキル「サンクチュアリ」で攻撃対象をアンデット/悪魔種族から不死属性/悪魔種族に変更
・スキル「マグヌスエクソシズム」で攻撃対象をアンデット/悪魔種族から不死属性/悪魔種族に変更
・スキル「サンクチュアリ」の回復回数を人数からカウントに変更
	skill.c
		skill_unit_onplace()修正
		skill_unit_onout()修正

多分この仕様で合ってるはずです…。

--------------
//0529 by 胡蝶蘭

・MOBがスキル「ヒール」を使用するとサーバーが落ちる場合があった問題を修正
・スキル「サンクチュアリ」で攻撃対象を不死属性からアンデット/悪魔種族に変更

	skill.c
		skill_unit_onplace()修正
		skill_castend_nodamage_id()修正

・ログインサーバーのアカウントデータベース保守ツールを添付
	Perl製なので実行にはPerlが必要です。
	使用方法などはエディタで開いて見てください。
	使い方が良くわからない人は手を出さないほうがいいです。
	
	特に理由が無い限りアカウント作成もこちらのツールを使ってください。
	addaccountはパケットの都合上パスワード文字数の制限がきついので。

	アカウントを削除してもキャラクターデータ、倉庫データ、
	その他のアカウント以外のデータは消えません。相手がログイン中だった場合
	強制切断はされませんが、次回からはログインできないはずです。
	（つまりは、単にlogin-server上のアカウントを消しているだけです）

	(login/)
	login.c
		parse_admin()追加、parse_login()修正
	(doc/)
	admin_packet.txt
		新規追加。管理パケット情報
	(tool/)
	ladmin
		login-server administration toolのPerlスクリプト


--------------
//0528 by RR
・スキル「ヒール」を使用した際に回復量に比例した分だけジョブ経験値が獲得できるように変更
・商人系職業が店NPCを利用した際にジョブ経験値が獲得できるように変更
・両方ともbattle_athena.confで調整可能にしました。初期設定は0倍（非適用）
・map_athena.confにてかぼちゃクエストのものが入ってなかったのでコメントアウトしながら追加。

	map_athena.conf

	battle.c  battle_config_read()
	battle.h  Battle_Config
	battle_athena.conf
		以上、battle_athena.conf利用ために変更

	pc.c pc_heal()
		戻り値をhp+spに。戻り値を利用してる部分がなさそうだったので使わせて貰いました。バグが起きたらすみません。

	skill.c skill_casted_nodamage_id()のヒール部にてジョブ経験値獲得するよう変更

	npc.c npc_buylist()
	      npc_selllist() 変更
	      これらとの兼ね合いでskill.hをinclude。


商人の店利用ジョブ経験値獲得ですが、計算式はまだ考え中なので仮で。
計算式はアイテム購入が　代金 * スキルレベル（ディスカウント）/ ((1＋300/アイテム個数) * 4000)
アイテム売却が　代金 * スキルレベル（オーバーチャージ） / ((1＋500/アイテム個数) * 4000)です。
常に矢をたくさん一緒に購入することで経験値を多量に稼ぐことが可能ですね…。
どなたかいい式を思いついたら変更お願いします。

ところで転職NPCが一部かぶってるんですが修正しないでいいんでしょうか？

--------------
//0527 by 死神

・0526のバグ修正。(テストの為に変えていた物を入れたままアップしてしまったのが原因でした。)
	skill.c 修正。
	pc.c 修正。
	mob.c 修正。
	clif.c 修正。

--------------
//0526 by 死神

・0525のリザレクションを死んだキャラに使えないバグ修正。(テストはしてませんが治ったはずです。多分...)
	skill.c 修正。
	clif.c 修正。

--------------
//0525 by 死神

・dmotionの間はキャラが動かないように変更。(テストしてません。)
・メテオのダメージ表示タイミング修正。(少し遅い気もしますが...)
・バックスライディングの時にモーションが出るように変更。(スキル使用後
0.2秒後にスキル使用パケットを送るように変更しました。ラグ等によって変な動作をする可能性もあります。)
・0524の修正。
・ハイディングしている時自然回復しないように変更。
・0519で間違ったconfの修正と細かい所修正。
	map.h
		struct map_session_dataのcanmove_tickをcanact_tickに変更。
		skillcanmove_tickをcanmove_tickに変更。
	skill.c 修正。
	pc.c 修正。
	pc.h 修正。
	clif.c 修正。
	battle.c 修正。
	battle.h 修正。
	mob.c 修正。
	mob.h 修正。
	char_athena.conf 修正。
	map_athena.conf 修正。

--------------
//0524 by huge

・ローグ トンネルドライブ実装
	clif.c
		clif_parse_WalkToXY()
	pc.c
		pc_calcstatus()

	どのくらい速度減少するのか分からなかったので、暫定的に
	speed += speed * (20-スキルレベル)/40
	と、しました。本鯖仕様が分かる方居ましたら修正お願いします。

--------------
//0523 by NOCTURNE

・npc_event_rental.txtにクルセイダー用のペコペコ管理兵を追加
--------------
//0522 by 波浪

・mob_db.txtをジュノー後のデータに修正

--------------
//0521 by 胡蝶蘭

・mapサーバーに繋がらない問題を修正
	clif.c
		clif_parse()の修正

--------------
//0520 by 胡蝶蘭

・charサーバーログの「set map X.Y HOGE.gat」が表示されなくなりました
	変わりに、「set map M from XX.YY.ZZ.WW:PP (CC maps)」
	というふうに何個のマップをセットしたかだけを表示するようになります。

	char/char.c
		parse_frommap修正

・複数mapサーバーに仮対応
	・NPCのマップサーバー変数は鯖間では共有されません。共有すべき変数を
	  持つNPCがいるマップ同士は同じmapサーバーで動かすべきです。
	  おそらくPCのグローバル変数は共有できると思います(未テスト)
	・暫定的に動くようにしただけなので、不都合があるかもしれません。
	  特に、パーティ/ギルド/倉庫/ペット/Wisなどのinterサーバーを使う機能が
	  正しく作動するか全くチェックしていません。
	・「recv map on XX.YY.ZZ.WW:PP (CC maps)」というログが表示されます。
	  これは他のmapサーバーが担当するマップのリストが、このmapサーバーに
	  正常に受信されたという意味です。

	(char/)
	char.c/char.h
		parse_frommap()修正
		mapif_sendallwos()追加
	(map/)
	map.c/map.h
		map_setipport()を修正
		struct map_session_dataのstateメンバにwaitingdisconnect追加
	chrif.c/chrif.h
		色々追加
	clif.c
		waitingdisconnectが１ならパケットを無視するようにした
	pc.c
		pc_setpos()修正（マップサーバー変更処理など）
		pc_setnewpc()修正

--------------
//0519 by 死神

・サーバーsnapshotと色々修正。
・死んだキャラに攻撃が当たるバグ修正。(テストしてません。本当に治ったかどうか報告お願いします。)
・0517のアイスウォールの処理を少し変更。
・メテオをモンスターも使えるように変更。(テストしてません。報告お願いします。) でもちょっとメモリーの使用量が増えました。(約10M程上がるようです。)
・ボスの取り巻きがボスと一緒に行動するように変更。(テストしてませんのでどんな動きをするかは確認してません。攻撃も受けてないのにボスの隣から勝手に離れるかどうかの確認をお願いします。)
・その他細かい物修正。
	client-packet.txt 修正。
	map.h
		AREA_SIZEを15から20に変更。
		struct map_session_data、struct mob_data、struct skill_timerskill 修正。
	map.c
		map_quit() 修正。
	clif.h
		clif_changemapcell() 修正。
	clif.c
		clif_getareachar_skillunit()、clif_clearchar_skillunit()、clif_changemapcell() 修正。
	skill.c
		skill_unitsetting()、skill_unit_onlimit()、skill_castend_pos2() 修正。
		skill_castend_nodamage_id()、skill_check_condition()、skill_attack() 修正。
		skill_timerskill()、skill_addtimerskill()、skill_cleartimerskill() 修正。
		その他少し修正。
	skill.h
		skill_addtimerskill()、skill_cleartimerskill() 修正。
	pc.c
		pc_movepos()、pc_walk()、pc_authok() 修正。
	mob.c
		mob_spawn_dataset()、mob_spawn() 修正。
		mob_changestate()、mob_damage() 修正。
		mob_ai_sub_hard_mastersearch()、mob_ai_sub_hard() 修正。
		その他少し修正。
	battle.c
		battle_calc_weapon_attack()、battle_weapon_attack() 修正。
		その他少し修正。

--------------
//0518 by Kalen
・Event_pumpkin関連のフラグ不具合修正

--------------
//0517 by 死神

・アイスウォールで摺り抜る問題修正と少し修正。(Mさんパケットの提供ありがとうございます。)
	clif.h
		clif_changemapcell() 追加。
	clif.c
		clif_changemapcell() 追加。
	skill.h
		SC_STEELBODYを84から87に変更。
	skill.c
		skill_unitsetting()、skill_unit_onlimit() 修正。
		skill_status_change_end()、skill_status_change_start() 修正。
	client_packet.txt 修正。

--------------
//0516 by 死神

・モンスターのメテオによる鯖ダウンを臨時に防いで置きました。(テストはしてません。) モンスターのスキルについてちょっと分析不足ですので分析した後に修正して置きます。
・スキル指弾の仕様をbattle_athena.confで決めるように変更。(0515の物がちょっともったいなかったので...)
	skill.c
		skill_castend_pos2() 修正。
	battlc.h、battle.c
		battle_configにfinger_offencive_type 追加。
		battle_calc_weapon_attack() 修正。
	battle_athena.conf 修正。
	conf_ref.txt 修正。

--------------
//0515 by 死神

・スキルメテオと指弾修正とパケット修正、0512の落とし物修正と少しだけの仕様変更です。
・指弾の場合説明を見てこんな感じかなと思って作った物です。以前の物が本鯖にあっているなら元に戻します。
・メテオの1発の範囲は5*5セル(range = 2)です。
・アイテム使用パケットを新しい物に変更したがエフェクトが出ない物は出ないようです。(色々エフェクトが入っているみたいだから後は使って確認ですけどね。)
・0512でhitrateが10000以上で必中ではなく100000以上で必中ですのでコードの修正のさいには気をっつけてください。
・battle_athenaに設定されている武器の製造率とペットの捕獲確率の計算方法を少し変えました。(気にする必要もない物ですけどね。)
	skill.h
		skill_addtimerskill()、skill_cleartimerskill 追加。
	skill.c
		skill_attack() fix、skill_use_id()、skill_use_pos() 修正。
		skill_castend_damage_id()、skill_castend_nodamage_id() 修正。
		skill_timerskill()、skill_addtimerskill()、skill_cleartimerskill 追加。
		skill_castcancel()、skill_castend_pos2()、skill_unitsetting() 修正。
		skill_produce_mix()、do_init_skill() 修正。
	mob.c
		mob_damage() 修正。
	battle.c
		battle_calc_weapon_attack() 修正。
	map.h
		struct skill_timerskill 追加。
		struct map_session_data 修正。
	map.c
		map_quit() 修正。
	pc.c
		pc_authok() 修正。
		pc_damage() 修正。
	clif.c
		clif_parse_WalkToXY() 修正。
		clif_useitemack() 修正。(資料提供: Kalenさん)
	pet.c
		pet_catch_process2() 修正。
	skill_db.txt
		気功のSPを10に変更。(ネットの検索では10だったので変更しました。韓国蔵では15と表示されますが...)
	client_packet.txt
		01c8の変更です。Kalenさん情報提供ありがとうございます。

--------------
//0514 by Kalen

・script修正+追加
	
	conf/npc_event_pumpkin.txt	(新規)カボチャイベント
	conf/npc_town_guide.txt		(修正)Junoの「+」アイコンカラー修正
	conf/npc_town_lutie.txt		(一部追加)カボチャイベントに影響するNPCの会話追加

--------------
//0513 by RR

・転職時に装備が全て外れるようにしました。関数位置の変更してないので、ひょっとしたらおかしくなってるかもしれません。うちの環境(win2k cygwin)では平気でしたのでそのままにしてあります。。
・ノービス時の死亡では、最大HPの２分の１で復活できるよう修正。(スキルによる復活は未確認)
・デスペナルティによる経験値減少を追加。battle_athena.confにて、減少率を変更できるように設定。減る経験値は小数点以下切り捨てなので、必要経験値が低いうちにはちょうどその％分引かれるという風にはなりません。
	battle.h
		Battle_Configにdeath_penalty_baseとdeath_penalty_jobを追加。
	battle.c
		battle_config_readでdeath_penalty_baseとdeath_penalty_jobを読むように修正。
	pc.c
		pc_makesavestatus() 修正。
		pc_damege() 修正。
		pc_jobchange() 修正。
	battle_athena.conf
		death_penalty_base,death_penalty_job追加。

--------------
//0512 by 死神

・問題になりそうな部分の修正と新しいパケットの対応がメインです。後バグも少し治しました。(動けない状態異常になっても動く問題の修正等です。)
	athena.sh 修正。(いつも鯖を個別に実行していたので気がつきませんでした。)
	makefile
		DPACKETVERを2から3 に修正。ジューノ以後の蔵を使うのなら3にして使ってください。(その以前なら2か1)
	clif.c
		DPACKETVER=3に対応(今の所0x114を0x1deに変換と0x11fを01c9に変換するのみ対応)
		clif_skill_damage3() 削除。
		clif_skillcastcancel() 追加。
		clif_skill_damage()、clif_getareachar_skillunit()、clif_skill_setunit() 修正。
		clif_fixmobpos()、clif_fixpetpos()、clif_fixpcpos() 修正。
		他に少し修正。
	clif.h
		clif_skill_damage3() 削除。
		clif_skillcastcancel() 追加。
	battle.c
		battle_calc_weapon_attack() 修正。
		hitrateを10000以上にすれば必中になるように変更。(今の仕様ではモンスターの必中攻撃以外は必中になりません。)
		他に少し修正。
	client_packet.txt
		新しいパケット情報追加。
	pc.c
		pc_spiritball_timer()、pc_delspiritball() 修正。
		pc_damage()、pc_skill() 修正。
	skill.h
		SC_EXPLOSIONSPIRITSを89から86に変更。(86 = 0x56)
		SC_DELUGEを86から89に変更。
	skill.c
		skill_castcancel()、skill_use_id()、skill_use_pos() 修正。
		skill_check_condition() 修正。
		skill_castend_damage_id()、skill_castend_nodamage_id 修正。
		skill_status_change_end()、skill_status_change_start() 修正。
	skill_db.txt
		気功の消費SPを修正。(前の15が本鯖にあっているみたいですので...)
	mob.c
		mobskill_use_id()、mobskill_use_pos() 修正。
	map.c
		map_quit() 修正。
	atcommand.h
	atcommand.c
		@コマンド@spiritball追加。(機能は使えばわかります。ただ1000以上は入れない方がいいです。蔵がパンクしますので...)
	atcommand_athena.conf
		修正。
	conf_ref.txt
		修正。
・0x196パケットに新しい物が追加されているので状態変化に直接に関係ないSC_xxxxの番号を調整する必要があります。(今はSC_EXPLOSIONSPIRITSにだけ対応しました。) それとskill_status_change_end()、skill_status_change_start()でclif_status_change()を呼ぶtypeの範囲が64(0x40)未満になっているがそれも追加されている物に合わせて修正する必要がありますが追加されている物が全てわかったわけでもないのでSC_EXPLOSIONSPIRITSにだけ対応しました。今度からは爆裂波動の解除が正確に見えます。金剛はデータを見つけられませんでした。
※新しいパケットに対応する作業をしていますが情報が不足です。
イグドラシルの実やイグドラシルの種のエフェクトが出るようにする為に01c8を使ってみましたが駄目でした。client_packet.txtのデータでは何も起こらないので何方が本鯖でイグドラシルの実やイグドラシルの種を使った時のパケットを提供してくれませんか？S 00a7の後00a8が来るのかそれとも01c8が来るのかの確認と00a8の後に01c8が来るのかの確認ができれば何とかなると思いますが...
それと01c9の後に来る?.81bがわかればアイスウォールを摺り抜る問題も解決できると思いますが...
情報提供をお願いします。

--------------
//0511 by Diex

・指弾の攻撃回数修正。
・阿修羅覇鳳拳、発勁が修練を無視し、無属性になるように修正。
・金剛使用時、MDEFが正しく表示されてなかったバグを修正。
・気功の消費SPを修正。
	pc.c
		pc_calcstatus() 修正。
	battle.c
		battle_calc_weapon_attack() 修正。
	skill.c
		skill_check_condition() 修正。

	skill_db.txt 修正。

--------------
//0510 by Diex

・三段掌の表示バグ修正
・指弾が気弾が無くても撃てるバグを修正
	map.h
		struct map_session_dataにspiritball_old変数追加。
	skill.c
		skill_check_condition() 修正。
	clif.c
		clif_skill_damage3() 修正。
	battle.c
		battle_weapon_attack()、battle_calc_weapon_attack 修正。

--------------
//0509 by 

・npc_warp.txt
	プロ城→プロフィールドになっていたのを、
	プロ城→ヴァルキリーレルムに修正。
	プロフィールド→プロ城になっていたのをプロフィールド→ヴァルキリーレルムに修正

--------------
//0508 by 死神

・バグ修正と息吹、気功、気奪の修正がメインです。(今度からは他の人にも気がちゃんと見えます。)
・死んだキャラに経験値が入る問題修正。(テストはしてません。どうなのか報告をお願いします。)
	pc.h
		pc_addspiritball()、pc_delspiritball() 追加。
		pc_is50overweight() 修正。
	pc.c
		pc_gainexp() 修正。
		pc_insert_card()、pc_item_identify() 修正。(大した修正じゃありませんが...)
		pc_authok() 修正。
		pc_addspiritball()、pc_delspiritball() 追加。
		pc_spiritball_timer() 追加。
		do_init_pc()、pc_calcstatus()修正。
		pc_spirit_heal() 追加。
		pc_natural_heal()に関わる物の修正。
	map.h
		struct map_session_data 修正。
	map.c
		map_quit() 修正。
		map_addflooritem() 修正。
	clif.h
		clif_spiritball_int()をclif_spiritball()に変更。
		clif_spiritball_ext() 削除。
	clif.c
		clif_spiritball_int() をclif_spiritball()に変えて修正。
		clif_spiritball_ext() 削除。
		clif_set01e1() 追加。
		clif_getareachar_pc() 修正。
	skill.h
		SC_CALLSPIRITS 削除。
	skill.c
		SC_CALLSPIRITS 削除。
		skill_castend_nodamage_id()、skill_check_condition() 修正。
		skill_status_change_start() 修正。
・床に落ちたアイテムが消えるまでの時間を設定できるように変更。
	battle.h、battle.c
		battle_config_read() 修正。
	conf_ref.txt 修正。
	battle_athena.conf 修正。

--------------
//0507 by Diex
0505の修正
・三段掌のエフェクト実装

--------------
//0506 by hoenny
全体的に少し式修正
セイジのスキルがアイテムを消耗するように修正
(スキルのDBに zeny, spiritball, item, equipを入れたいが時間がなくて臨時的に ...)

--------------
//0505 by Diex

・阿修羅覇鳳拳のダメージ修正
・猛龍拳のダメージ修正
・指弾実装
・発勁実装
・金剛が減算DEFと減算MDEFが90に固定されていたのを乗算DEFと乗算MDEFが90になるように修正
・三段掌実装（ただしコンボは未実装）
	map/clif.c
	map/clif.h
		clif_skill_damage3()追加
	map/pc.c
	map/skill.c
	map/battle.c
		修正・及び追加
（計算式は+ Acolyte Maniax +を参考にしました。）

--------------
//0504 by 死神

・atcommand.c 修正。(たった2文字を追加しただけです。)
atcommnad_gm_onlyがnoじゃなくても使用レベル設定を0にしたコマンドはGMじゃないキャラでも使えます。テストはしてません。

--------------
//0503 by nabe

・精錬修正とLinux用にちょっと変更など
	conf/map_athena.conf
		npc_event_doll.txt,
		npc_turtle.txt,
		//npc_pota.txt項目追加
	conf/npc_pota.txt追加（socieさん作のダンジョンポタ子さん）
	conf/npc_shop.txt
		イズルード武器商人の価格修正
	conf/script_athena.conf
		0499での文字化け？修正
	login/login.c
		#include <time.h>追加
	map/script.c
		buildin_getequipname()
			精錬メニューのmallocを修正
		buildin_getequipisenableref()
			精錬可能条件修正（Athefansさんの条件文にしてみました）
	map/skill.c
		skill_check_condition()変数宣言位置の変更のみ

--------------
//0502 by 死神

・カプラの倉庫もbattle_athena.confのbasic_skill_checkによって基本スキルが足りなくても使用することができるように修正しました。(プロンテラ中央のカプラのみ確認。)
・クェストスキルの取得は0492のせいです。0481にskillの最後が,2じゃなく,0だと書いたのですが何故か0492で,2になっていたので,0に修正しました。
・スクリプトgetbaseskillcheckをbaseskillcheckに変更と自分で修正しましたが命令の後になんの数値も入らない場合はスクリプトが正しく作動しなかったので使用する場合はbaseskillcheck(0)にして使わないと正しい結果を得ることができません。
(詳しくはnpc_town_kafra.txt参照。)
	npc_event_skillget.txt 修正。
	npc_town_kafra.txt 修正。
	script.c 修正。
・GM_account.txtに自分がテストの為に使っていた500000が入っていたので削除しました。(GM_account.txt作った理由は自分勝手にアカウントをGMにすることができるようにする為です。つまりGMとして表示されなくてもGMとして色んな権限を使うことができます。さすがにGM専用の右クリックコマンドは使えませんが... でもこれを活用している方はいないみたいなんですね...)

--------------
//0501 by hoenny

・500の問題点修正
-HP回復力向上,SP回復力向上
	map/pc.c
		pc_natural_heal_hp()修正
		pc_natural_heal_sp()修正

500SP回復力向上
--------------
//0500 by hoenny

・修道僧の息吹実装
-基本的に座った時 10秒ごとに回復します.
-所持量が 50%をオーバーした場合 20秒ごとに回復します.
・修道僧の気奪実装
-他の修道僧の氣球も吸収が可能です.
・修道僧の金剛実装
-金剛状態ではすべてのアクティブスキルを使うことができないです.
	db/skill_db.txt
		気奪修正
		金剛修正
	map/pc.h
		pc_is50overweight()追加
		pc_is90overweight()追加
	map/pc.c
		pc_calcstatus()修正
		pc_spheal()修正
		pc_hpheal()修正
		pc_natural_heal_hp()修正
		pc_natural_heal_sp()修正
		pc_natural_heal_sub()修正
	map/skill.c
		skill_check_condition()修正
		skill_castend_nodamage_id()修正
		skill_status_change_start()修正

--------------
//0499 by 死神

・サーバーsnapshotとバグ修正。
・スティールコインのゼニ量をモンスターレベル*10 + rand(100)に変更。
・キャスティングタイムがないスキルはタイマーを使わないように変更。
・カードの使用によるマップ鯖ダウンを防ぐ為に修正。(今度こそ治ったはず...) そして拡大鏡も同じようなことができるので修正。(これは鯖ダウンまでは起こさないようですが...)
・キャラにマップのロードが終わるまでペットのデータが来ないとマップ鯖が落ちる問題修正。(滅多なことがない限り起こらないですけどね。)
・オートバーサーク、重さのアイコンとチェックが正しく適用されるように修正。
・増速ポーションの適用順番変更。今まではスピードアップポーションが最優先で次がハイスピードポーション、最後がバーサークポーションだったのですが順番を逆に変更しました。
・アイテムで使うスキルはキャスティングタイムとディレイが0になるように変更。
・アイテムで使うスキルのレベルがitem_dbに設定しているレベルより高くなるバグ修正。
	pc.h
		pc_move()をpc_movepos()に変更。
	pc.c
		pc_steal_coin()、pc_insert_card()、pc_item_identify()、pc_authok()、
		pc_calcstatus()、pc_checkweighticon()、pc_damage() 修正。
	skill.c
		skill_castend_pos2()、skill_check_condition()、skill_use_id()、
		skill_use_pos() 修正。
	pet.c
		pet_recv_petdata()、pet_change_name() 修正。
	map.h
		struct map_session_dataにskillitemlv 追加。
	script.c
		buildin_itemskill() 修正。
	clif.c
		clif_parse_UseSkillToId()、clif_parse_UseSkillToPos()、
		clif_parse_LoadEndAck() 修正。
	mob.c
		mobskill_use_id()、mobskill_use_pos() 修正。

--------------
//0498 by hoenny

．氣球がすっかり見えるように修正(消耗スキル使用の時消耗するように修正)
．スキル残影を使用の時氣球を消耗するように修正
	doc/client_packet.txt
		0x1d0追加
	map/map.h
		sdのstructureに spiritball追加
	map/clif.h
		clif_spiritball_del() -> clif_spiritball_int()修正
		clif_spiritball_cre() -> clif_spiritball_ext()修正
	map/clif.c
		packet_len_table[]修正
		clif_spiritball_del() -> clif_spiritball_int()修正
		clif_spiritball_cre() -> clif_spiritball_ext()修正
	map/pc.h
		pc_item_steal() -> pc_steal_item()修正
		pc_coin_steal() -> pc_steal_coin()修正
	map/pc.c
		pc_item_steal() -> pc_steal_item()修正
		pc_coin_steal() -> pc_steal_coin()修正
		pc_calcstatus()修正
	map/skill.c
		skill_check_conditon()修正

--------------
//0497 by 死神

・0491のスキル残影のバグ修正とスティールとスティールコイン、スナッチャーの修正、mob_targetのバグ修正。
・交換、座り、パーティ結成等の時に基本スキルをチェックするように修正。(battle_athena.confでチェックするかどうかを決めることができます。)
ただカプラの倉庫はスクリプトで制限をかけるしかありません。
・スクリプトgetbasicskillcheck追加。
	使用方法＞ getbasicskillcheck
	戻り値はbattle_athena.confのbasic_skill_checkです。0の場合は基本スキルのチェックなしで1の場合は基本スキルをチェックするのを意味します。
	skill.c
		skill_castend_pos2() 修正。
		skill_additional_effect() 修正。
	pc.c
		pc_move() 追加。
		pc_item_steal()、pc_coin_steal() 修正。
	pc.h
		pc_move() 追加。
	map.h
		struct mob_data 修正。
	mob.c
		mob_spawn() 修正。
		mob_target() 修正。
	clif.c
		clif_pcinsight()、clif_pcoutsight() 修正。
		clif_parse_ActionRequest()、clif_parse_Emotion()、
		clif_parse_TradeRequest()、clif_parse_CreateParty()、
		clif_parse_ReplyPartyInvite() 修正。
	battle_athena.conf 修正。
	conf_ref.txt 修正。
	skill_db.txt
		スティールの射程を3から1に修正。
	battle.h、battle.c
		battle_configにbasic_skill_check 追加。
		battle_config_read() 修正。
	trade.c
		trade_traderequest() 修正。
	script.c
		buildin_getbasicskillcheck() 追加。
	map/makefile 修正。
※スティールとスティールコインの計算式は適当に作った物です。本鯖の方がどうなのか全然わからないので...

	スティール率 = (モンスターのアイテムdrop率 * (キャラレベル*0.5 + dex*0.4 +スキルレベル*5))%
	スティールコイン率 = (スキルレベル + (キャラレベル - モンスターのレベル)*0.3 + dex*0.2 + luk*0.2)%
	スナッチャー発動率 = (5.5 + スキルレベル*1.5 +スティールのスキルレベル)%

本鯖の計算式がわかる方は情報提供をお願いします。
スティールコインのゼニの量はモンスターのレベル*100になっています。これについても情報提供をお願いします。
※残影の場合使った後普通に歩く前にはペットの装備が見えないバグがありますが原因がわからないので放置することにしました。
※基本スキルが足りない時出るメッセージは殆ど合わせていますがパーティに入る時に基本レベルが足りない時に合う物がなかったのでパーティを作れない(基本スキルレベル7の物)と表示して勧誘した方には拒絶されたと表示されます。

--------------
//0496 by hoenny

．WZのメテオストーム実装
．スティールコイン修正
	db/skill_db.txt
		メテオストーム修正
	map/pc.c
		pc_coin_steal()修正
	map/skill.c
		skill_castend_pos2()修正

--------------
//495 by nini

・ARが片手斧、両手斧でしか発動しなかったところ修正→片手斧、両手斧、鈍器
	map/skill.c
		skill_check_condition()　修正

前回修正のとき鈍器入れ忘れてたようです。	

--------------
//0494 by 獅子o^.^o

conf/mpc_warp.txt
．ハンタ一転職地出ていない修正

--------------
//0493 by 波浪

・script修正
	npc_town_comodo.txt	コモド案内要員部分を削除(npc_town_guide.txtと重複していたので)
	npc_town_guide.txt	ジュノー案内要員を追加(viewpointの色が…)
	npc_town_kafra.txt	ジュノーカプラ部分をnpc_town_yuno.txtから移動
	npc_town_refine.txt	ジュノー精錬所部分をnpc_town_yuno.txtから移動
	npc_town_yuno.txt	案内要員とカプラと精錬所部分を削除
	npc_turtle.txt		会話を微修正

--------------
//0492 by Kalen

・script修正+追加
	
	conf/npc_event_doll.txt		(新規)
	conf/npc_turtle.txt		(新規)亀島関連NPC+亀島クエスト(航海日誌)追加

	conf/npc_event_skillget.txt	(修正)応急処置の不具合+へんなtab削除etc..
	conf/npc_town_alberta.txt	(修正)Turtle分離、ちびっ子削除(Event_dollへ移動)
	conf/npc_town_guide.txt		(修正)台詞がかなり変更されていたので、修正


--------------
//0491 by 死神

・スキル残影(韓国クライアントでは弓身彈影)実装。(ただ気弾のチェックはしてません。)
	pc.h
	pc.c
		pc_can_reach() 追加。
	skill.c
		skill_check_condition()、skill_castend_pos2() 修正。
	skill_db.txt
		残影修正。
・script.c
	set_posword() 修正。
※一人でテストは済んでいますが他の人に正しく見えるかどうかは未確認です。
変なのかどうか報告をお願いします。

--------------
//0490 by nabe

・場所スキルエフェクトでマップ鯖が落ちることがあったのを修正。
	clif.c
		clif_skill_poseffect()の
		unsigned char buf[16];を、unsigned char buf[32];に修正。

--------------
//0489 by 死神

・0483のバグ修正。battle_athena.confのquest_skill_learnが正しく適用されるように変更と問題があった部分の修正。(テスト済み)
	pc.c
		pc_calc_skilltree()、pc_skill() 修正。
	atcommand.c
		@lostskill 少し修正。
・カードの使用によるマップ鯖ダウンを防ぐために少し修正。(ただカードの使用によるマップ鯖ダウンを再現できなかったので本当に治ったかどうかは不明...汗)
	pc.c
		pc_insert_card() 修正。
・pc.h
	pc_ishiding() 修正。

--------------
//0488 by hoenny

・RGのスティールコイン実装
・スティール修正
(二スキル皆一度スチールした場合またスチールすることができない.そしてスキル成功の時モンスターは攻撃するように修正した.エフェクトは成功の時だけ出るように修正した.)
	db/skill_db.txt
		スティールコイン修正
	map/pc.h
		pc_coin_steal()追加
	map/pc.c
		pc_coin_steal()追加
		pc_item_steal()修正
	map/skill.c
		skill_castend_nodamage_id()修正

--------------
//0487 by hoenny

・485の問題点ちょっと修正
	map/pc.c
		pc_item_steal()修正
	map/skill.c
		skill_castend_nodamage_id()修正

--------------
//0486 by 獅子o^.^o

db/class_equip_db.txt修正
裂けた大地の書、燃える太陽の書、乾いてる風の書、默示録、プリーストは装備することができない問題修正

--------------
//0485 by hoenny

・ RGのスナッチャー実装
・ スティール修正
・ 露店開設の時 skill_check_conditionで状態をチェクするように修正
	map/pc.h
		pc_ishiding()追加
		pc_item_steal()追加
	map/pc.c
		pc_item_steal()追加
	map/skill.c
		skill_castend_nodamage_id()修正
		skill_additional_effect()修正
		skill_check_condition()修正

--------------
//0484 by 胡蝶蘭

・覚えてないクエストスキルにスキルポイントを振れる問題修正
	pc.c
		pc_calc_skilltree()でクエストスキルのチェック追加
	skill.c
		skill_readdb()でinf2を読むように修正

・ペコペコ騎乗、ファルコンのアイコンがログイン直後には表示されない問題修正
	clif.c
		clif_parse_LoadEndAck()修正

--------------
//0483 by 死神

・0482の適用。
	npc_event_skillget.txt 修正。
	script.c 修正。
	skill.c 修正。
	pc.c 修正。

--------------
//0482 by 胡蝶蘭

・クエストスキルのスクリプト少し修正
・スクリプトgetskilllvを呼ぶとマップサーバーが落ちるバグ修正
	(conf/)
	npc_event_skillget.txt
		出来るだけ変数を使わないように修正（未テスト）
	(map/)
	script.c
		buildin_getskilllv()修正

・スティールで失敗時のエフェクト変更
・同じMOBには１回しかスティールできないように修正
	(map/)
	skill.c
		skill_castend_nodamage_id()修正
	map.h
		struct mob_dataにsteal_countメンバ追加
	mob.c
		mob_spawn()修正、steal_countを0に初期化するように

・イドゥンの林檎でHPが32767を超えるとサーバーが落ちるバグ修正（未テスト）
	(map/)
	pc.c
		pc_calcstatus()修正

--------------
//0481 by 死神

・これの適用には気をつけてください。0478の胡蝶蘭さんの物を Athena.txtのデータ形式変更せずにクェストスキルを覚えるように作った物です。自分が作ってる最中に胡蝶蘭さんが同じ物をアップしてくれたのですがデータは変えない方がいいと思って自分の物もアップしました。注意することは0478のathena.txtは使えないと言うことです。0478前の物を使ってください。
・skill_db.txtにinf2を追加してこれを使ってクェストスキルかどうかを判断する仕組みです。
	skill.h 修正。
	skill.c
		skill_readdb()修正と少し修正。
		skill_get_inf2() 追加。
	skill_db.txt 修正。
	skill_tree.txt 修正。(0478前の物)
	clif.c
		clif_skillinfoblock() 修正。
	char.c 修正。(0478前の物)
・battle_athena.confにquest_skill_learn追加。
	battle.h 修正。
	battle.c
		battle_config_read() 修正。
	battle_athena.conf 修正。
・/resetskillをbattle_athena.confにquest_skill_learnの設定に合わせてquest_skill_learnがyesの場合はスキルポイントに加算してquest_skill_learnがnoならリセットはされるがスキルポイントに加算されません。
	pc.c
		pc_skill()、pc_resetskill() 修正と少し修正。
	pc.h 修正。
	atcommand.c 修正。
	atcommnad_athena.conf 修正。
・スクリプトのskillコマンドでクエストスキルを覚えられるのは同じですが最後のフラグが2から0に変わってますので注意してください。
	npc_test_skill.txt 修正。
	npc_event_skillget.txt 修正。
	conf_ref.txt 修正。
	client_packet.txt 修正。

--------------
//0480 by Kalen

・Eventskill追加
	conf/npc_event_skillget.txt

・map_athena.conf変更
	warp.txtの読み込み優先度を変更
	prt_castle等、旧EPのワープと異なる場所に変更された場合
	先に読み込んだ方が優先されるので、EPの高い順のがよろしいかと
	conf/map_athena.conf

--------------
//0478 by 胡蝶蘭

***
 Athena.txtのデータ形式変更!! (自動的に変換されます)
 バックアップを忘れずに!
 Data format of athena.txt is changed!! (convert automatically)
 DONT FORGET BACKUP!! 
***

・クエストスキル実装
・スクリプトでスキルレベルをチェックできるように
	・スクリプトのskillコマンドでクエストスキルを覚えられます。
	  使用方法＞ skill スキルID,スキルLV[,フラグ]
	  フラグは省略可能で、省略すると１を指定したことになります。
	  １で装備品による一時的な習得、２でクエストによる恒久的な習得です。
	  恒久的な習得の場合、skill_tree.txtに依存します
	・getskilllvコマンド追加
	  使用方法＞ getskilllv(スキルID)  戻り値はレベルです。0で未習得。
	
	(conf/)
	npc_test_skill.txt
		サンプル
	(db/)
	skill_tree.txt
		クエストスキルとして必要スキルIDに-1を設定。
	(char/)
	char.c
		フラグもathena.txtに保存するように。
		以前の形式のデータも読み込めます。
	(map/)
	pc.c/pc.h
		pc_skill(),pc_calc_skilltree()など修正
	script.c
		buildin_skill(),buildin_getskillid()など修正

・@questskill,@lostskill追加
	・@questskill スキルID でクエストスキルを覚えます。(クエストスキルのみ)
	・@lostskill スキルID でスキルを忘れます。(クエストスキル以外もOK)

	atcommand.c/atcommand.h
		struct Atcommand_Configにlostskill,questskillメンバ追加
		@questskill,@lostskill処理追加

--------------
//0477 by nabe

・一部変数の宣言位置の変更のみ（Linux等でコンパイルしやすいように）。
	atcommand.c,battle.c,clif.c,mob.c,npc.c,skill.c

--------------
//0476 by nabe

・conf/ ちょっと整理
	conf/map_athena.conf修正。
	tortoise.txtをnpc_town_alberta.txt中に移動。
	npc_script2.txtのコモドガイドをnpc_town_comodo.txt中に移動。

・npcがキャラ名を喋るときのバグ修正
	map/script.c
		buildin_strcharinfo()でキャラ名用のメモリを
		staticに確保してしまっていたのを、mallocに修正。

--------------
//0475 by hoenny

泥棒のスティール実装。
ギルド生成の時エンペリウム消耗するように修正。
	map/guild.c
		guild_create()修正。
		guild_created()修正。
	map/skill.c
		skill_castend_nodamage_id()修正。

--------------
//0474 by 死神

・0471の精練の時表示される文字の設定をmap_athena.confからscript_athena.confに変更。
	script.c
		do_init_script() 修正と少し修正。
	script.h 修正。
	script_athena.conf 追加。
	map_athena.conf 修正。
	map.c
		map_config_read() 修正。
・古い巻物、プレゼントボックス実装と少し仕様変更。
ランダムでアイテムを得る物にデフォルトで出るアイテムを設定できるように変更。今の仕様では1000回までアイテムが選択されなかったらデフォルトアイテムが出るようになっています。デフォルトアイテムが0の場合はアイテムを得られません。
設定する確率を*1000から*10000に変更。ただitem_~.txtの修正はやっていません。誰かやってください。(他力本願)
	itemdb.c
		temdb_read_randomitem() 修正。
		itemdb_searchrandomid() 修正。
	item_purplebox.txt から item_violetbox.txt に修正。
	item_giftbox.txt、item_scroll.txt 追加。(moveさんありがとう。)
	item_db.txt
		古い巻物、プレゼントボックス 修正。
・trade.c
	trade_tradecommit() 修正。pc_delitem()を使うように変更。
※テストはやっていませんので問題があったら報告してください。

--------------
//0473 by Kuro

・class_equip_dbを一部修正
	db/class_equip_db.txt

--------------
//0471 by hoenny

製錬の時出る文を変えることができるようにしました.(map_athena.confで調節可能)
気功の数字が市廛の時実際水路表示図緑修正
	conf/map_athena.conf
		refine_posword:追加。
	map/map.c
		map_config_read()修正。
	map/script.h
		do_set_posword()追加。
	map/script.c
		do_set_posword()追加。
		buildin_getequipname()修正。
	map/skill.c
		skill_status_change_start()修正。

--------------
//0470 by 死神

・製造の時属性石が二度減る問題修正。(実は二度減るように見えるだけでマップを移動すると正しく表示されますが...)
	pc.h、pc.c
		pc_delitem() 修正。
	npc.c、script.c、storage.c、pet.c
		pc_delitem()を全て修正。
	skill.c
		skill_produce_mix() 修正。

--------------
//469 by 波浪

・npc_mob_job.txt、npc_monster.txt、npc_monster30.txt、mob_db.txtのモンス名を修正
・item_db.txtの回復アイテムの回復量をジュノー後のものに修正

--------------
//468 by Kuro

・魔剣製作クエスト追加
	conf/npc_event_ma_sword.txt

--------------
//467 by nini

・BBが両手剣でしか発動しなかったところ修正→すべての武器で
・ARが両手斧でしか発動しなかったところ修正→片手斧、両手斧、鈍器
・スピアクイッケン発動を槍だけに
・キャストキャンセルされないものにグランドクロス、ローグのストリップシリーズ追加
・ブリッツビートがキャストキャンセルされなくなってた点修正
	map/skill.c
		skill_use_id()　修正
		skill_check_condition()　修正
・2-2職スキルのキャスト・ディレイ追加		
	db/cast_db.txt	
		

--------------
//466 by hoenny

・阿修羅覇鳳拳修正(公式修正及び sp消耗がすぐ見えるように)
・蓄気の時気弾が見えるように修正(Mr.NO NAME様のパケ情報ありがとうございます.気弾が消えるパケ情報が不足です.)
・パリの羽やテレポート1を連続使用の時,鯖オーバーが発生しないように仮初めで修正
	doc/client_packet.txt
		0x1e1パケ情報追加
	map/battle.c
		battle_calc_weapon_attack()修正
	map/clif.h
		clif_spiritball_cre()追加
		clif_spiritball_del()追加
	map/clif.c
		packet_len_table[]修正
		clif_spiritball_cre()追加
		clif_spiritball_del()追加
		clif_changemap()修正
	map/skill.c
		skill_castend_nodamage_id()修正
		skill_check_condition()修正

--------------
//0465 by 死神

・リザレクションとハイディング、ブリッツビートのバグ修正。(ブリッツビートは報告はなかったのですが分析したら問題があったので修正。)
	skill.c
		skill_use_id() 修正。
		skill_castend_nodamage_id() 修正。
・0455のNPCを元に戻しました。
	npc_event_ice.txt 修正。
	npc_event_potion.txt 修正。
	npc_town_geffen.txt 修正。（454の物に戻しました。）
・0451のαマップをコマントアウトしました。必要な方はコマントアウトをなくして使ってください。
	map_athena.conf 修正。
・リザレクションは0442の問題でハイディング、ブリッツビートは0445の問題でした。それと0445の修正でスキル番号をenumで宣言した文字に変えていますがそれに落としがあるようです。(ハイディング、ブリッツビートはそのせいでした。)前の番号ソースと比べて問題がある部分は修正する必要があります。ちょっと面倒ですが...

--------------
//464 by 波浪

・モンクスキルの部分について修正(未実装スレに書かれていたものを追加しただけです。
	skill.c
		skill_use_id()修正
	cast_db.txt
		モンクスキル追加

--------------
//463 by 胡蝶蘭

・462のバグ修正
	・NPCのSHOPの不都合修正
	・READMEの間違い修正（warpwaitingpcがwarpwaitingroomになっていた）
	
	map.h
		struct npc_dataのchat_idの位置を修正

--------------
//462 by 胡蝶蘭

・NPCチャット作成
	・waitingroom命令でNPCチャットを作成します。
	  引数は waitingroom "チャット名",制限人数,イベント名 です。
	  イベント名は人数が最大になったときに起こすイベント名で、省略可能。
	・warpwaitingpc命令で、チャット内にいるPC全員をワープできます。
	  引数はwarpと同じで、warpwaitingpc "マップ名",x,y です。

	map.h
		struct npc_dataとchat_dataを修正
	script.c
		buildin_waitingroom(),buildin_warpwaitingpc()追加
	chat.c/chat.h
		色々修正
	clif.c
		clif_getareachar_npc()、clif_joinchatok()など修正
	
・NPCのOnInitイベントをサーバー起動時に呼ぶように。
・エクスポートするときのNPC名と表示上のNPC名を別々に設定可能に。
	・同じNPC名のイベントは重複できないため、エクスポート用NPC名を使い、
	  同じNPC名でも別のNPCとして識別できるようにしなければなりません。
	  (もちろん、イベント処理を行わない場合はその必要はありません。)
	・npc_*.txtのscript命令でNPCの名前を設定するとき、
	  「表示名::エクスポート名」とすると、表示する名前と、イベント用に
	  エクスポートする名前を別々に指定できます。
	  表示名が全く同じ別々のNPCでイベントを動作させるときに使用します。
	・ややこしいのでPVPのnpcスクリプトを見てイメージを掴んでください。

	npc.c/npc.h
		npc_parse_*()の修正
		npc_event_do_oninit(),npc_event_do_oninit_sub()など追加
	map.c/map.h
		do_init()でnpc_event_do_oninit()を呼ぶように
		struct npc_data修正

・スクリプトgetmapusers、getareausersの致命的なバグ修正
	・該当マップが存在しない場合、マップサーバーが落ちるのを修正。
	・マップが存在しないと、-1を返すようにした。
	
	script.c
		buildin_getmapusers(),buildin_getareausers()修正

・pvpのスクリプト修正
	・チャットルームを作るようにした
	
	(conf/)
	npc_pvproom.txt
		全てのNPCのエクスポート名(pvp??r)設定
		OnInit:でwaitingroomを実行するように

--------------
//461 by Kuro
・アコライト転職クエスト一部修正
	conf/npc_job_aco.txt

--------------
//460 by sagitarius
・item_dbの間違い修正
	4032,Ambernite_Card,アンバーナイトカード,6,20,,10,,2,,,,,32,,,,{},{},,修正

--------------
//459 by hoenny
・アドレナリンラッシュ使用の時斧チェック(自分だけ)
・Old_Blue_Boxの確率が高いという報告によって修正
・Script.cは0455以前ことで引換(NPCをここに合わせて修正してください)
・阿修羅覇鳳拳修正(公式を修正したんですが, 正確かはよく分からないですね.)
・修道僧の蓄気,爆期の仮実装.(阿修羅覇鳳拳使用の時蓄気,爆期状態をチェックします. )
・ペコペコに乗った後に、ADPDが間違ったこと修正(バグを直してあげたが, 直す前ことに変わるせいでまた修正)
その外にも修正をしたようなのに覚えないですね.そして誤った部分があれば指摘してください.
	conf/npc_event_ice.txt
		checkweight修正
	conf/npc_town_geffen.txt
		checkweight修正
	db/item_purplebox.txt
		Old_Blue_Box修正
	map/battle.c
		battle_calc_weapon_attack()修正
	map/pc.c
		pc_spheal()修正
		pc_calcstatus()修正
	map/script.c
		buildin_checkweight()修正
	map/skill.h
		SC_ EXPLOSIONSPIRITS追加
	map/skill.c
		SkillStatusChangeTable[]修正
		skill_castend_nodamage_id()修正
		skill_check_condition()修正

--------------
//458 by Kuro
・アコライト転職クエスト追加
	conf/npc_job_aco.txt
		会話文が分からなかったので適当にしてあります。また、NPCの外見の変え方が分からなかったので適当にしてあります。
		分かる方は修正しておいて下さい。	

--------------
//0457 by Kalen

・PVP関連のNPC追加
	conf/npc_pvp.txt
	conf/npc_pvproom.txt

--------------
//0456 by 死神

・モンスターの最初攻撃時間が長すぎる問題修正。
モンスターの最初攻撃時間を今まではmob_dbのaDelayを使っていましたがこれをaMotionに変更しました。今まではmob_dbのaMotionはゴミでしたが今度からはゴミではありません。aMotionさえ正しければモンスターの攻撃のモーションの前にダメージが出てくる問題もなくなるはずです。
	mob.c
		mob_changestate() 修正。
・アイテムを入手できない時その理由に当たるメッセージが出るように変更。
	pc.c
		pc_additem() 修正。
・job_db1.txtので問題になった.を,に修正。

--------------
//455 by Mr.NO NAME
・NPCとのアイテム交換や買い物関係のScriptが出来上がった当初の
　NPCデータ(npc_event_making.txt、npc_town_geffen.txt等)に乗っ取り、以下を修正。
	conf/npc_event_ice.txt
	     npc_event_potion.txt
	     npc_town_geffen.txt（454以前の物に戻しました。）
	map/script.c
		buildin_checkweight()を修正。

--------------
//454 by Kuro
・ゲッフェン鍛冶屋で買い物が出来るように修正
	conf/npc_town_geffen.txt

--------------
//0451 by code
・ 今更ですがαクライアントに対応(αクライアントのdata.grfをadata.grfとしてgrf-files.txtのadataのところに書いてください)
     common/grfio.c
		grfio_setadatafile()追加
           /grfio.h
		grfio_setadatafile()追加
     conf/map_athena.conf
		αクライアントのマップを読み込むように変更
         /npc_warp_a.txt
                αマップのワープポイントの設定(ちょっとずれてるかも)
         /grf-files.txt
		αクライアントのdata.grfをadata.grfとして読み込むように設定
		adata: に記述
※αクライアントは
　ttp://www.castledragmire.com/ragnarok/
　あたりから入手してください。

--------------
//0450 by hoenny
・ 騎兵修練実装
・ コムパルションディスカウント実装
・ ディスカウント・オーバーチャージ修正(数が高い場合計算法が間違ったことを直しました.)
・ 鉄拳修正(素手な時も適用されるように)
・ 斧修練修正(片手斧な時も適用されるように)
・ ボンゴンが攻撃するように修正
 	map/pc.c
 		pc_calcstatus()修正
 		pc_modifybuyvalue()修正
 		pc_modifysellvalue()修正
 	map/battle.c
 		battle_addmastery()修正
 	db/mob_db.txt
		ボンゴン修正 

--------------
//0449 by 死神

・変になった所修正。
	const.txt
		bAtkとbDef 追加。
	battle.c
		battle_calc_weapon_attack()を元に戻しました。(0445の物)
	map.h
		map_session_dataを元に戻しました。(0445の物)
	pc.c
		pc_calcstatus() 修正。
		pc_bonus() 修正。
	item_db.txtを元に戻しました。(0446の物)

--------------
//0448 by hoenny
・増速修正(pc_walk()から pc_calcstatus()に移動)
・最大所持量修正
	map/pc.c
	pc_calcstatus()修正。

--------------
//0447 by ゆう
・二刀流・矢の属性を正しく適応
・ATKの上がるカードの効果を武器サイズ修正なしの底上げに変更
・ATK・DEFの上がるカードの効果の適応の仕方を変更

map.h
	map_session_dataにcatk(カードATK)を追加

pc.c
	pc_calcstatus()
	  アサシンの二刀流の攻撃速度を修正した
	  スクリプトによる属性を左右正しく適応するようにした
	  矢の属性を正しく適応するようにした（弓の属性優先）
	  ただし、矢がすべての攻撃に適応されます
	  カードATKの処理を追加した

battle.c
	battle_calc_weapon_attack()
	  カートATKを底上げダメージとして計算するようにした

item_db.txt
	カードの bonus bAtk、bDef を削除
	かわりに、装備と同様にATKとDEFを設定
	（変更前をitem_db2.txtとしているので、不具合があれば戻してください）


--------------
//0446 by hoenny
・ミストレスカード実装。
・スキル使用の時ジェムストーン消費。
・スキル使用の時装備チェック。(ハンマーフォールだけ修正しようとしたが...)
・ハンマーフォールの範囲を 半径5セル(全25セル)ロ修正
	map/skill.c
		skill_check_condition()修正。
		skill_castend_pos2()修正。

・ミストレスカード修正。
	db/item_db.txt

--------------
//0445 by Aya

・基本ASPDと計算処理を修正。
	db/job_db1.txt
	map/pc.c
・SP係数と計算処理を修正。
	db/job_db1.txt
	map/pc.c
・スキル名をenumで宣言し、それを使うように変更。
	map/skill.h
	map/battle.c
	map/pc.c
	map/skill.c
・リカバリーのスキルIDがスローポイズンになっていたのを修正。
	map/skill.c
・集中力向上にカード効果が適用されていた問題の修正。
	map/pc.c
・リムーブトラップ、スプリングトラップ、ポイズンリアクトのターゲットを修正。
	db/skill_db.txt
・GMアカウントをjROのclientinfo.xmlから追加。
	conf/GM_account.txt
・warning修正。
	map/party.c
・キャラセレ認証時にlogin_id2はチェックしないように変更。
	login/login.c
・object_def.bat以外全ファイルの改行コードをLFに変更。
・*.cnfファイルを*.confファイルに名前変更。

--------------
//0444 by 死神

・GMコマンドや@コマンドにコマンド別に使用レベルを設定できるように変更と@コマンド少し修正。(@whereと@day、@nightの修正と他のキャラに使うコマンドの場合GMレベルが自分以上の場合使えないように修正。)
	atcommand.h 修正。
	atcommand.c 修正。
	clif.c 修正。
	map/makefile 修正。
	map.c
		do_init() 修正。
	conf/atcommand_athena.cnf 追加。
・細かい修正。
	pc.c
		pc_setghosttimer()、pc_skill() 修正。
	script.c
		buildin_skill() 修正。
・conf_ref.txt 修正。
・item_db.txt
	彼女の想い修正。

--------------
//0442 by 胡蝶蘭

・増速ポーション実装
	・Lvや職業判定は行いません

	(db/)
	const.txt
		SC_SpeedPot0,SC_SpeedPot1,SC_SpeedPot2追加
	item_db.txt
		増速ポーションのスクリプト追加
	(map/)
	skill.c
		skill_status_change_start()修正
	pc.c
		pc_calcstatus()修正

・PvPシステムの仮実装
	・pvpマップでは自動的に、PCのpvpフラグon、順位通知などを行います。
	・マップにpvpフラグをつけるサンプルをnpc_pvp.txtとして添付しています。
	・pvpの詳しいルールがよくわからなかったので、次のようにしています。
		・最初の持ち点は5点、倒すと1点、倒されると-5点。
		・0点以下のPCはリザレクションが掛からない
	・GMはpvpマップにいても足元にサークルが出現しないようです。
	  （クライアントの仕様？）
	・pvpマップで@pvpoff/@pvpすると休憩したり、休憩をやめたりできますが、
	  使用するべきではありません。
	
	(conf/)
	npc_pvp.txt
		pvpフラグを入れるサンプル。
		nosaveフラグや受け付けnpcなどを追加するとよいと思われる。
	(map/)
	clif.c
		clif_parse_LoadEndAck()修正
	npc.c
		npc_parse_mapflag()修正
	skill.c
		skill_castend_nodamage_id()修正
	pc.c
		pc_damage()引数修正
	atcommand.c
		pc_damage()引数修正に伴う修正
	battle.c

・その他修正
	・@pvpoff/@pvpで順位やサークルの表示をやめた
	・@jumptoでスペースの入ったキャラクターも指定できるように
	・@kamibコマンド復活（青文字天の声）
	・非PVPのときに、対象が敵のスキル使用時、敵味方判定を行うように
	
	skill.c
		skill_castend_id()で敵味方判定
	atcommand.c
		各コマンド修正

--------------
//0440 by 中の人

・本家を再現する方向なら意味はないかもしれませんが
　pc.c「スクリプトによるスキル所得」を若干変更して
  カードによるスキル一時習得の際でも1レベル以上を設定できるように致しました。

　単純に符号を変えてごまかしただけですので
　必要にあわせて修正をして下さい。

--------------
//0439 by hoenny
・阿修羅覇鳳拳の修正。
	db/skill_db.txt
・モンスター情報の修正。
	map/clif.c
・見切りの実装。
	map/pc.c

--------------
//0438 by Ａの人
・古木の枝が使える場所をＮＰＣスクリプトから制御可能
　mapflagにnobranchとすればそのマップは古木の枝使用不可になります。
	map.h
		enumにMF_NOBRANCH 追加。
	npc.c
		npc_parse_mapflag() 修正。
	pc.c
		pc_useitem() 修正。
ソース汚くしてしまったかも・・・.
勉強不足です

--------------
//0437 by 波浪
・item_db.txtの英名を大幅修正。(s付きとそうでない武器の英名がいつの間にやら
　同じになっていたのでそれを直すついでに他の部分も修正しました。
  まったく違う名前になってるものもありますが、こっちの方が正しいと思います。
・item_purplebox.txtを本家仕様っぽく作成(大体こんな感じかと
・アルベルタとイズルードNPCを修正

--------------
//0436 by hoenny
・morocc 宝石商人の修正
	conf/npc_shop.txt
・ハンマーフォールの実装(Alchemist氏ソースを参照ありがとう！)
	map/skill.c
以前に文字化けは低のせい!
次から気を付けます.

--------------
//0434 by Avethes

・タートルアイランドへ行くNPC修正
・ユノーNPC修正
（前回のバグはすみませんでした）

--------------
//0433 by 死神

・製造バグ修正。
	何故かはわからないがskill.cのskill_readdb()が変になっていたので修正。(自分がやった修正ではありませんが...)
	skill.c
		skill_readdb() 修正。

--------------
//0432 by 死神

・0429で一部のアイテムのスキルが出ない問題修正。
	clif.c
		clif_parse_UseSkillToId()、clif_parse_UseSkillToPos() 修正。
・skill.c
	skill_use_id() 修正。(大した修正ではないです。)
・item_db.txtの文字化け修正。言語設定が日本語ではない場合保存する時には気をつけましょう。
・攻撃されたモンスターの反撃が早すぎる問題修正。始めての攻撃がモンスターの攻撃ディレイに関係なく100ms後になっていたので攻撃ディレイに合わせるように変更。(ただ少し反撃が遅いと思われたりもしますが...)
	mob.c
		mob_changestate() 修正。
・鯖に接続する最大人数を決めるように変更。
	char.c 修正。
	conf_ref.txt 修正。
	char_athena.cnf 修正。

--------------
//0430 by Avethes

・ＮＰＣ関係。ほとんどテスト。
本家会話情報が揃えば修正。

--------------
//0429 by 死神

・ギルドのレベルアップをキャラのレベルアップのように変更。
	int_guild.c
		guild_calcinfo() 修正。
		guild_next_exp() 追加。
	exp_guild.txt 修正。(レベルが上がらないようにしたいレベルのexpに0を入れればそれ以上にレベルが上がらなくなります。)
・スクリプトresetstatus、resetskill 追加。
	pc.c
		pc_resetskill() 修正。
	script.c
		buildin_resetstatus()、buildin_resetskill() 追加。
・0425の続きで少し修正。
	clif.c
		clif_parse_ を少し修正。
・ショートカットに覚えているスキルレベル以上のスキルが登録されていても覚えているスキルレベルまでのスキルを使うように変更。
	clif.c
		clif_parse_UseSkillToId()、clif_parse_UseSkillToPos() 修正。
・メモの最大数を10個に変更。(あくまでも拡張の為の物です。まだ機能はしません。)
	mmo.h
		struct mmo_charstatusのmemo_pointを3から10に変更。
	char.c
		mmo_char_tostr() 修正。
・mob,c
	mob_once_spawn()、mob_summonslave() 修正。(別に意味がある修正じゃありませんが...)
・@monster コマンドで座標を指定しない時モンスターが一か所に集中して出るのをキャラの10*10マス以内にランダムで現れるように変更。
	atcomand.c 修正。

--------------
//0428 by Avethes

・conf/npc_smilegirl.txt
	スマイルマスクガールスクリプト。
	0427のおかしい部分とか修正。
	提供された各都市の座標に配置。（NONAMEさん提供ありがとう！）

--------------
//0426 by 胡蝶蘭

・アイテムの名前をdata.grfから読み込むようにした
	itemdb.cのITEMDB_OVERRIDE_NAMEを定義しなければ読み込みません。
	ITEMDB_OVERRIDE_NAME_VERBOSEはitemdb.txtのデバグ用にどうぞ。
	普通は変える必要はないと思うのでbattle_configには入れていません。
	
	itemdb.c
		itemdb_read_itemnametable()追加
		do_init_itemdb()修正

・データベース読み込み部の不安定性の修正(結構致命的だったみたいです)
	なくても問題ないDB（item_value_db.txtなど）のファイルがない場合に
	鯖が落ちたりする現象が発生していた場合はこれで直っているかもしれません.

	skill.c
		skill_readdb()でNULLポインタチェックを追加
	itemdb.c
		item_readdb()を複数に分けた。
		ランダムアイテムデータベースの読み込み部を１つに纏めた。
		do_init_itemdb()修正

・細かいバグ修正
	・ワープポータルの開くまでの秒数調整
	
	skill.c
		skill_unitsetting()修正

・その他修正（by 某M氏）
	db/job_db1.txt
		ちょこっと修正
	db/job_db2.txt
		2-2次職の足りないJobボーナスを追加(参考:R.O.M 776)
	conf/npc_town_kafra.txt
		オークD前と炭鉱前にカプラ配置(動作未確認)
	conf/npc_shop3.txt
		ジューノ販売NPC(拾い物)
	conf/npc_town_yuno.txt
		ジューノNPC(拾い物を改良。動作未確認)

--------------
//0425 by 死神

・0419で書き忘れた物ですがスキルラーニングポーションがSP回復アイテムにも効果があるように変更。
・今度はバグ修正がメインです。鯖落ちがなりそうな所の修正とテレポートの時死んだまま移動できる問題と0419でアクティブモンスターの先攻問題修正、死んでいるのに他の人には死んだように見えない問題の修正です。少しテストはしましたが本当に治ったかどうかは不明です。報告をお願いします。
	pc.c
		pc_attack_timer()、pc_damage()、pc_walk() 修正。
	map.c
		map_quit() 修正。
	mob_db.txt
		ビッグフットのmodeを修正(アクティブになっていた為)
	clif.c
		clif_parse_WalkToXY()、clif_pcoutsight()、clif_pcinsight()、
		clif_getareachar_pc()、clif_getareachar_mob()、clif_getareachar_pet() 修正。
	mob.c
		mob_ai_sub_hard_activesearch()、mob_ai_sub_hard_mastersearch()、
		mob_walk() 修正。
	pet.c
		pet_walk() 修正。

--------------
//0424 by hoenny

・クリップボーナス SP 10追加
	db/item_db.txt
・warp_test_yuno.txtを npc_warp30.txtに含んで, ちょっと修正
	conf/npc_warp30.txt
・他のサーバーが落ちても復旧されるように修正
	/startクリップ

--------------
//0420 by 紅葉

・EP 3.0でのカード効果変更に解る範囲で対応。
　ほぼ全ての変更点について、出来る限り修正してあります。
　ATK修正が正しく適用されているようなので追加してあります。(アンドレCなど)

--------------
//0419 by 死神

・0414で書き忘れた物ですが MOBのmodeで0x20(32)を復活させました。ボスじゃなくてもmodeに0x20が入っている場合普通のMOBでも死んだふりを破れます。
(今の所機能はそれだけです。本鯖はAI強化みたいですが...) ただゴーストはボスでも破ることはできません。
それと取り巻きのAIで取り巻きがターゲットした時主がターゲットしてないと主が取り巻きのターゲットをターゲットする部分をコマントアウトしました。(これが本鯖にあっていると思いましたので...)
・古い青い箱、古い紫色の箱、古いカード帖で出るアイテムをファイルで設定できるように変更。
	script.c
		buildin_getitem() 修正。
	item_db.txt
		古い青い箱、古い紫色の箱、古いカード帖修正。
	item_bluebox.txt、item_purplebox.txt、item_cardalbum.txt 追加。(使用例程度の物です。どのアイテムが出るようにするかは自分で設定して使ってください。ただクライアントを落とすアイテムは出ないように設定してください。)
	itemdb.h
		struct random_item_data 追加。
	itemdb.c
		itemdb_searchrandomid()、itemdb_readdb() 修正。
・mob.c
	mob_target()、mob_ai_sub_hard() 修正。(問題がありそうな部分だけ修正。)
・pc.c
	 pc_itemheal()、pc_walktoxy_sub() 修正。
・ペットの出現をMOBと同じように変更。
	clif.c
		clif_spawnpet() 修正。
	pet.c
		pet_change_name() 修正。
・0418を少し修正。(if文の条件を少し修正しただけです。)

--------------

//0418 by hoenny
・ /mm(/mapmove) /nb /b /bb /resetskill /resetstate GM 命令語使用の制限
clif_parse_MapMove ,clif_parse_ResetChar ,clif_parse_GMmessage 修正
	map/clif.c

--------------
//0417 by れあ

・0412でitem_db.txtがおかしくなっていたのを修正

--------------
//0416 by 紅葉

・ジュノー周辺のワープ定義と敵の配置。
　ワープ定義はnpc_warp30.txtとし、追加する形にしてあります。
　敵の配置についてもnpc_monster.txtとは統合せず、npc_monster30.txtとしてあります。
　問題が無いようであれば統合して下さい。
・上記定義ファイル追加に従いmap_athena.cnfを変更。
・@goコマンドへジュノー追加。
　要望があったようなので追加しました。

--------------
//0415 by 中の人

・今は亡き旧ROエミュ鯖開発スレッド Lv02での死神氏の説明に従って
　モンスター定義データを若干変更させて頂きました。
	・過去のnpc_monster.txtから通常マップ上（ルティエ等除く）にいるサンタポリン、アンソニを抽出し
	　新たに作った「npc_x-masmonster.txt」に移転
	・上記の修正にあわせてmap_athena.cnfを修正。
	　map_athenaにコメントアウト状態で「npc: conf/npc_x-masmonster.txt」を追加しました。
	　必要にあわせてコメントアウトをして下さい。

--------------
//0414 by 死神

・strcasecmpをstrcmpiに変更。
・dbや設定ファイルを読む時// をコマントアウトとして認識するように修正。
・ペットと離れすぎるとペットが早く動くように変更。(キャラの2倍の速度で動きます。)
・ルートモンスターがアイテムをターゲットした時攻撃を受けても攻撃してこない問題修正。
・同族モンスターのAIを変更。今まではtraget_idを使うせいでモンスターが攻撃した相手を攻撃する仕組みだったが今度はattacked_idを使う為攻撃してきた相手を攻撃するように変更。
ただ今の仕様だと同族モンスターを攻撃して逃げる場合攻撃を受けた時その場になかったモンスターはついて来なくなっています。本鯖の仕様にあってるかどうかは不明ですので情報提供をお願いします。(attacked_idはいつもリセットされる為です。対策がいないわけでもないですが本鯖の仕様を知らないので...)
・メモリーの使用量を減らす為struct mob_dataとstruct npc_dataを変更。(0412で
map-serverのメモリーの使用量が164???KBytesだったが0414では152???KBytesになりました。ほんの少し減っただけですが増えるよりはましだと思いますので...)
・ゴーストタイム実装。
 マップ移動やテレポート、復活した時に敵に狙われない時間を重力ではゴーストタイム呼んでいます。そのゴーストタイムの実装です。
battle_athena.cnfで時間を設定できます。時間を0にするとゴーストタイムは作動しません。ただこのゴーストタイムは攻撃行動、スキル使用、アイテム使用をするとなくなります。
	char/int_guild.c
	char/int_party.c
	conf/battle_athena.cnf
	db/mob_db.txt
	doc/conf_ref.txt
	login/login.c
	map/atcommand.c
	map/battle.c
	map/battle.h
	map/clif.c
	map/itemdb.c
	map/map.c
	map/map.h
	map/mob.c
	map/npc.c
	map/pc.c
	map/pc.h
	map/pet.c
	map/skill.c を修正。(db/mob_db.txtは//を入れただけですが...)
		修正した所を全て覚えてませんのでファイルだけ知らせます。

--------------
//0412 by いど

・モンスター定義データ(日本語)の再整理
　	旧掲示板で指摘のあった事項について大体の範囲で修正
	snapshot387のバージョンをベースに修正しました。
		conf/npc_monster.txt

・アイテム名の定義を大幅修正
	(root)
		item.list
	(db/)
		item_db.txt
		item_value_db.txt

・マップデータの定義でコメントアウトしていたジュノー関連マップのコメントアウトを解除
	conf/map_athena.cnf

--------------
//0411 by 死神

・鯖snapshotです。それとlogin_port、char_port、map_portの設定がなくても
デフォルトで6900、6121、5121を使うように変更。
・login.c、char.c、chrif.c、clif.c 少し修正。
・conf_ref.txt 修正。
・login_portを6900から他の物に変えた場合はclientinfo.xmlを変える必要があります。

--------------
//0410 by 死神

GM用右クリックメニュー「（name）使用者強制終了」実装。(テストはしてません。@コマンドはテスト済みですが...)
0407のEXPに関する修正に問題があるらしいので修正しました。今度はテスト済みです。
GMのアカウントIDを設定できるように変更とGMをレベル別に分けるように変更。
(GMのレベルによる@コマンド等に制限をかけるつもりですが今制限がかけている物は@kick、@kickallのみになっています。)
・pc.c
	pc_readdb()、pc_gainexp()、pc_nextbaseexp()、pc_nextjobexp()、
	pc_checkbaselevelup()、pc_checkjoblevelup() 修正。
	pc_isGM()、pc_read_gm_account() 追加。
・pc.h
	pc_isGM() 修正。
	pc_read_gm_account() 追加。
・exp.txt
	レベルが上がらない数値を999999999から0以下に変更。
	レベルを上げる為に必要なEXPを999999999以上にすることも可能。
・clif.c
	clif_GM_kickack()、clif_GM_kick()、clif_parse_GMKick() 追加。
・clif.h
	clif_GM_kickack()、clif_GM_kick() 追加。
・atcomand.c
	strncmpiをstrcmpiに変更。
	@kick、@kickall コマンド追加。
	@kick <キャラ名>
		自分以外のキャラの接続を強制終了させる。(自分よりGMレベルが
		低いキャラにしか使えない。GMではないキャラのGMレベルは0)
	@kickall
		鯖に接続している全てのキャラの接続を強制終了させる。(自分と
		GMを含めて) 鯖ダウン用のコマンドです。GMレベルが99じゃないと
		使えない。
・conf/GM_account.txt 追加。
	GMとして認識するアカウントIDを設定するファイルです。
・mmo.h
	DEFAULT_WALK_SPEEDを140から150に変更。(これが本鯖にあってる数値
	みたいですので...)
	struct gm_account 追加。
・client_packet.txt
	パケット0x00cd 追加。
・login_portをcnfで読むように変更。(ただ6900からポートを変えるとクライアントが認識できない模様なので無駄なことだったりもしますが...)
	char.c、login.c、char_athena.cnf、login_athena.cnf 修正。
・普通のアカウント作成ではGMになれないようにlogin.cを変更。
・login/makefile、map/makefile 修正。

--------------
//0408 by 胡蝶蘭

・405の新しい＠コマンドを以前のatcommand.cに取り込みました。
	・@kamiを修正
	・@kill,@recall,@charjob,@revive,@charstats,@charoption,@charsave,
	  @night,@day,@doom,@doommap,@raise,@raisemap,@charbaselvl,@charjlvl
	  を追加＆メッセージを日本語に変更＆少し修正

	atcommand.c
		追加と修正

・一部のスキルの効果実装
	・不死身のジークフリード、イドゥンの林檎、幸運のキス、
	  フレイムランチャー、フロストウェポン、ライトニングローダー、
	  サイズミックウェポン
	
	map.h
		struct skill_unitにrangeを追加。
	skill.c
		色々修正
	skill.h
		enumの修正など
--------------
//0407 by 死神

・ペットのバグ修正。(ただ自分で再現できなかったので問題になりそうな所だけ修正しました。)
・ペットの移動速度をpet_dbに追加。
	pet.h
		struct pet_dbにspeed追加。
	pet.c
		pet_catch_process2()、read_petdb() 修正。
	pet_db.txt
		移動速度追加。
		(コマントアウトしているのはジルタスとアリスです。捕獲用の
		アイテムが存在することとパフォーマンスをすることから考えて
		追加される予定の物と考えられます。ただその捕獲用のアイテムが
		あるとクライアントを落ちますので注意してください。追加しても
		台詞はポリンの物ですので... 捕獲用のアイテム以外は適当に入れた
		物です。)
・pc.c、clif.c
	pc_equipitem() 修正。
	clif_parse_EquipItem() 修正。
	pc_equipitem()の未鑑定アイテムのチェックをclif_parse_EquipItem()に
		移動しました。(ペットの装備もありますので...)
・レベルを99以上にあげるように変更と職業別にベースレベルの限界レベルを設定できるように修正。
	map.h
		MAX_LEVEL追加。
	pc.c
		pc_nextbaseexp(), pc_nextjobexp() 修正。
		pc_readdb() 修正。
・exp.txt 修正。職業レベルと同じようにベースレベルもEXPテーブルを3つ作りました。レベルアップを止めたいレベルのexpを999999999にすればそれ以上レベルが上がりません。つまりnovice、1次職業と2次職業のベースレベルの限界を違うように設定できます。そしてベースレベル99以上に上がるようにすることもできます。(exp.txtの修正が必要ですが本鯖と違うように設定したい場合に修正して使ってください。)
・属性による回復をbattle_athena.cnfで設定できるように変更。
	attr_fix.txt 修正。
	battle.h
		struct Battle_Configにattr_recover 追加。
	battle.c
		battle_config_read() 修正。
	battle_athena.cnf 修正。
・conf_ref.txt 修正。
・client_packet.txt 修正。ペットパケット追加と少し修正。

--------------
//0402 by 胡蝶蘭

・400のバグを一部修正
	・掛かってないスキル効果によるステータス計算が行われてしまうバグ修正
	・効果修正：あくまで効果の計算の修正で、使えないスキルは使えません。
	  スピアクィッケン、プロヴィデンス、戦太鼓の響き、
	  夕陽のアサシンクロス、口笛、不死身のジークフリード、
	  イドゥンの林檎、サービスフォーユー、幸運のキス
	・効果追加：あくまで効果の計算の追加で、使えないスキルは使えません。
	  ハミング、私を忘れないで…、ニーベルングの指輪(武器レベル無視)、
	  エターナルカオス、ドラゴノロジー
	・効果付加系はちょっとでも怪しいスキルは全て使用できないように修正
	・攻撃系スキルはほとんど見てないのでたぶんバグ多いです。
	・全て未テストです。怪しすぎる部分を修正しただけです。
	
	map.h
		MAX_STATUSCHANGEを128に修正
	pc.c
		pc_calcstatus()修正
	skill.c/skill.h
		enumを修正
		skill_status_change_start()修正
	battle.c
		battle_calc_weapon_attack()など修正

--------------
//0400 by AppleGirl

Can Someone Help Me.
2-2 Skills added.
All The Mastery Skills.
SpearQuicken,Providence
New Bard Skill Assassin Cross Of Sunset
Providence
Frost Joke
Apple of Idun
Service For You
Meteor Strike (Different Style)
Assassin Cross Of Sunset (not tested)
All Masteries Done
Providence
Musical Strike
Throw Arrow
Frost Weapon << (Problems with elements)?
Flame Launcher << (Problems with elements)?
Seismic Weapon << (Problems with elements)?
Lightning Loader << (Problems with elements)?
Spirit Recovery
Potion Pitcher (Tato)
Axe Mastery (Tato)
Spear Quicken
Not Totally Working:
Combo Finish
Quadruple strike
Triple Attack
(skills in skill.c) (need to be finished.)
CP_ARMOR
CP_HELM
CP_SHIELD
CP_WEAPON
STRIP_HELM
STRIP_WEAPON
STRIP_SHIELD
STRIP_ARMOR

* 適当な和訳 *
2-2次職スキルを追加しました
全ての修練スキル、スピアクイッケン、プロヴィデンス、
夕陽のアサシンクロス（未テスト）、寒いジョーク、イドゥンの林檎、
サービスフォーユー、メテオストライク（少し違う）、
ミュージカルストライク、矢撃ち、フロストウェポン(属性が問題あり？)
フレームランチャー(〃)、サイズミックウェポン(〃)、ライトニングローダー(〃)
息吹、ポーションピッチャー
完全には働かないスキル：
猛龍拳、漣環全身掌、三段掌
(skills in skill.c) (完了される必要がある)
ケミカルアーマーチャージ、ケミカルヘルムチャージ、
ケミカルシールドチャージ、ケミカルウェポンチャージ、
ストリップヘルム、ストリップウェポン
ストリップシールド、ストリップアーマー

*注意 !! CAUTION !! by 胡蝶蘭*
この400にはバグが大量に含まれています。注意してください。
there are many many BUGS in this update(400) !! Be careful !!

--------------
//0399 by 胡蝶蘭

・MOBスキル使用条件や行動を修正
	・無行動MOBが待機時のスキルを使用できない問題を修正
	・条件スキル反応(skillused)がどのスキルにも反応していたバグ修正
	・非移動MOBが追撃してくる問題を修正

	mob.c
		mob_ai_sub_hard()修正
		mobskill_event()修正
		mobskill_use()修正
	skill.c
		skill_attack()修正

・MOBスキル一部実装
	・自決(エフェクト無し?)、自爆、タバコを吸う、範囲攻撃
	  HP吸収２つ(通常/魔法）（回復エフェクト無し?）実装

	(db)
	skill_db.txt
		スモーキングなどを修正
	(map/)
	skill.c
		skill_castend_damage_id(),skill_castend_nodamage_id()修正
	battle.c
		battle_calc_misc_damage()修正

・未鑑定アイテムが装備できなくなりました
・未鑑定アイテムにカードがさせなくなりました

	pc.c
		pc_equipitem(),pc_insert_card()修正
	clif.c
		clif_use_card()修正

・battle_athena.cnfにMOBの配置割合を定義できるようになりました
	・配置数が１のMOBについては適用されません
	・計算後の配置数が１未満の場合１に修正されます。
	
	(conf/)
	battle_athena.cnf
		mob_count_rate追加
	(doc/)
	conf_ref.txt
		修正
	(map/)
	battle.c/battle.h
		struct BattleConfig に mob_count_rate メンバ追加
	npc.c
		npc_parse_mob()の修正

・ボーリングバッシュが相手が１匹でもとりあえず当たるようになった。

	skill.c
		skill_castend_damage_id()修正

・学生帽作成イベントの修正

	(conf/)
	npc_event_making.txt
		アロエベラ(606)をアロエ(704)に。

・パケット情報修正

	(doc/)
	client_packet.txt
		0199パケット修正

--------------
//0397 by いど

・モンスター定義データ(日本語)の整理
	・npc_monster25.txtをnpc_monster.txtにリネームし、内容を整理(現在mob数:13450)
	・その軽量版としてnpc_monster_lite.txtを作成(現在mob数:11959)
	・上記の修正にあわせてmap_athena.cnfを修正

--------------
//0395 by 胡蝶蘭

・取り巻きMOBの行動修正
	・アンクルなどで移動できない場合主に近づかないように修正
	・ロックしていると主に近寄る処理をしないように修正
	・主がテレポートすると追いかけるように修正(付近10x10マス程度)
	・主のそばにいるときはランダム歩行をしないように修正

	mob.c
		mob_ai_sub_hard_mastersearch()修正
		mob_can_move()追加
		mob_ai_sub_hard()修正

・MOBの行動修正
	・スキル使用ディレイ処理がおかしかったのを修正
	・詠唱のないスキルはtimerを使わないように修正(死亡時処理対策)
	
	mob.c
		mobskill_use(),mobskill_use_id()修正

・MOBエモーションの実装
	・エモーションの種類がわからないものは全て「!」になります。
	  抜けているデータを埋めてくれるとうれしいです。
	
	(db/)
	mob_skill_db.txt
		いくつかのMOBのエモーションの項目の値1に種類を入れた。

	(map/)
	skill.c
		skill_castend_nodamage_id()修正
	clif.c/clif.h
		clif_emotion()追加

・パケット解析.txtをclient_packet.txtに改名＆修正

	(doc/)
	client_packet.txt
		エモーションの説明追加

・どうやら取り巻きMOBの種類は古いデータだったっぽいです。
  しかもMOB召喚では手下召喚と違うMOBを召喚するみたいですね。
  詳しい人はmob_skill_db.txtを直してくれると。

--------------
//0393 by いど

・char鯖でのlogin鯖のポート設定を6900に固定し、変更できないようにした
　(login側でポート6900固定になっていたのでchar側もそれに合わせました。)
	char/char.c
	conf/char_athena.cnf
	doc/conf_ref.txt

--------------
//0392 by 胡蝶蘭

・MOBの行動修正
	・何故かlast_thinktickが初期化されていない問題修正
	・上に関連してPCが近くにいても手抜き処理が行われる問題修正
	  （どうやら初期からのバグだった模様？  このバグと、
	    新しい手抜き処理の仕様がタッグを組んで残像を作っていた模様）
	・取り巻きMOB用のAI処理追加（まだ怪しいです）
	・MOBのスキルディレイをスキル項目ごとに持つように変更
	・スキルディレイが大きな項目ではオーバーフローしていた問題を修正
	
	map.h
		struct mob_dataの skilldelayを配列にしてunsigned intに変更
	mob.h
		struct mob_skillのcasttime,delayをintに変更
	mob.c
		mob_ai_sub_hard_mastersearch()追加
		mob_changestate(),mob_delete(),mob_catch_delete(),mob_damage(),
		mobskill_use(),mobskill_use_id(),mobskill_use_pos(),
		mobskill_castend_id(),mobskill_castend_pos(),
		mob_ai_sub_hard(),mob_ai_sub_lazy()など修正

・MOBスキルの手下召喚とモンスター召喚実装
	・mob_skill_db.txtの書式変更（最後に値を１つ追加、取り巻きMOBのID）
	・取り巻きMOBがわからなかったものはコメント化しています
	  わかる人は入力よろしくお願いします。
	・現在は取り巻きは一度倒したら沸きなおしません。
	・ボスがテレポートしても取り巻きは追いかけません。
	・本鯖でどうなってるのか知らないので、間違ってる場合は教えてください。
	
	(db/)
	mob_skill_db.txt
		手下召喚などのデータ修正
	
	(map/)
	skill.c
		skill_castend_nodamage_id()修正

--------------
//0391 by 死神

・ペットの移動中にパフォーマンスをするとペットが停止するように変更。
	(ペットの位置がずれるため修正しました。)
	pet.c
		pet_performance() 修正。
・死んだモンスターはどんな行動もとれないように変更。(これで無敵
	モンスターがいなくなるといいですが...)
	mob.c
		mob_changestate(),mob_delete(),mob_catch_delete(),mob_damage(),
		mob_ai_sub_hard(),mob_ai_sub_lazy()  修正。
・PC、NPC、床アイテムが使うIDの範囲を調整。
	床アイテムは0から500000まででPCは500000から100000000、NPC
	(モンスターを含めて)は110000000から約21億までになります。
	(-を含めるともっと範囲が広くなりますがさすがにそこまでは必要ないと
	思いますので...)
	map.h
		MAX_FLOORITEM 追加(これを変えると床アイテムの最大数を変える
		ことができます。今は100000になっています。ただこれは必ず
		500000以下にしてください。そうしないと正しく動くかどうか
		保証できません。)
	map.c
		map.hに合わせて少し修正。
	npc.h
		START_NPC_NUM 追加。
	npc.c
		npc.hに合わせて少し修正。
	login.h
		START_ACCOUNT_NUMとEND_ACCOUNT_NUM 追加。
	login.c
		login.hに合わせて修正。END_ACCOUNT_NUM以上にaccountを
		作れないように変更。
・カートレボリューションに武器研究を２回適用するように変更。
	(結局は元に戻すことになりました...^^;)
	battle.c
		Damage battle_calc_weapon_attack()  修正。
・mobのスキル使用をbattle_athena.cnfで決めるように変更。
	mob.c
		mobskill_use() 修正。
	battle.h
	battle.c
		struct Battle_Configにmob_skill_use追加。
	battle_athena.cnf
		mob_skill_use追加。(設定しないとnoです。)
・battle_athena.cnf
	mobを二重で読めないようにnpc: conf/npc_monster.txtを削除。
	(最新はnpc_monster25.txtなので...)

--------------
//390 by 胡蝶蘭

・バージョン情報所得部分を少し変更
	・MODバージョンを定義できるようになりました。詳細はversion.hを。
	  気が向いたときか、大きな更新があるときなどに変更してください。
	・バージョンcheck時のset eofログが出ないようにパケット7532追加。
	
	(common/)
	version.h
		MODバージョンを定義できるように。
	(tool/)
	checkversion
		MODバージョンを表示するように。
	(login/char/map)
	login.c/char.c/clif.c
		MODバージョンの処理追加、
		パケット7532（切断）処理追加。

・その他色々修正
	・こまごました修正ばかりですが、あまり覚えていません。
	・MOBスキル条件でslavelt,attackpcgt処理実装（未テスト）。
	・MOBの手下召喚のための機構追加（まだ召喚できません）。
	・範囲スキル効果範囲に死亡PCがいると鯖が落ちるバグ修正。
	・MOB残像が出なく…なってたらいいな。
	
	(map/)
	mob.c/mob.h/map.h/battle.c
		色々追加

	(db/)
	mob_skill_db.txt
		ルート時処理と、属性変更スキルのコメントを外した。
		（属性変更は本鯖で動いてないらしいものもコメントを外してます。
		  問題がある場合は再びコメント化してください）

--------------
//389 by いど

・388の変更
	バージョン情報をcommon/version.h内の定数を使用するように変更

--------------
//388 by 胡蝶蘭

・バージョン情報所得ツール添付
	Perl製なので実行にはPerlが必要です。
	使用方法などはエディタで開いて見てください。
	使い方が良くわからない人は手を出さないほうがいいです。

	バージョンを確認する用途よりは、サーバーの生存確認用といったかんじです
	パケット7530/7531の詳細はソースを見てください。

	(tool/)
	checkversion
		バージョン確認ツールPerlスクリプト

	(login/)
	login.c
		パケット7530/7531の処理追加
	(char/)
	char.c
		パケット7530/7531の処理追加	
	(map/)
	clif.c
		パケット7530/7531の処理追加

・384以前のathena.txtも読み込めるようにしました
	・convertが面倒な人向け。
	・正しく読み込める保証無し。バックアップを忘れずに。
	
	(char/)
	char.c
		384の方式で読み込めないデータは384以前の方式も試すように。

・conf_ref.txt/help.txt/getaccount修正
	help.txt
		petコマンドの説明追加
	(doc/)
	conf_ref.txt
		pet関連の設定の説明追加
	(tool/)
	getlogincount
		表示の修正

--------------
//387 by いど
・confフォルダ内のNPC定義データの整理
	以下のファイルを削除しました
		npc_kafraJ.txt
		npc_mind_prtmons.txt
		npc_script2J.txt(npc_event_mobtim.txtに同じものがあったため)
		npc_testJ.txt(ほぼ同じことが@コマンドで出来るため)
		npc_warp25.txt(npc_warp.txtに統合)

	以下のファイルの名前を変更しました
		npc_monster3.txt	-> nop_monster2E.txt
		npc_monster3J.txt	-> npc_monster25.txt
		npc_monster.txt		-> npc_monsterE.txt
		npc_monsterJ.txt	-> npc_monster.txt
		npc_sampleJ.txt		-> npc_sample.txt
		npc_script3j.txt	-> npc_script2.txt
		npc_script25J.txt	-> npc_town_lutie.txt
		npc_shop1J.txt		-> npc_shop_test.txt
		npc_shop2J.txt		-> npc_shop_mobtim.txt
		npc_shop3J.txt		-> npc_shop2.txt
		npc_shop.txt		-> npc_shopE.txt
		npc_shopJ.txt		-> npc_shop.txt
		npc_testJ.txt		-> npc_test.txt
		npc_warp3.txt		-> npc_warp2.txt
		npc_warp4.txt		-> npc_warp25.txt

・マップ定義の追加
	ジュノーアップデートで追加されるマップと、韓鯖独自(?)のクイズゾーン
	(コモドアップデート)と天津アップデートのマップ定義を追加
	現在、日鞍に無いものに関してはコメントアウトしていま。
		conf/map_athena.cnf

--------------
//385 by 胡蝶蘭

・MOBの行動修正
	・手抜き処理で移動しないモードのMOBも歩く問題修正
	・MOBを倒したとき、再spawn時刻がおかしな値になる場合がある問題修正
	  （MOBが沸かなくなる問題が修正されたはず）
	・MOBのワープで場所検索に1000回失敗したら元の場所に出るように修正
	・MOBを詠唱中に倒すと、タイマーを削除するように修正
	
	mob.c
		mob_delete(),mob_catch(),mob_damage(),
		mob_ai_sub_lazy(),mob_ai_sub_hard()など修正
		mobskill_deltimer()追加

--------------
//0384 by 死神

・ペット実装。
思ったより長くかかりました。一周もかかったせいで何処を修正したか
覚えてない問題がありますが... それで念の為にmapとcharのファイルは全て
含めてアップします。
それとmakefileとathena.shは自分が使ている物です。
Yare-launcherは使てませんがいつも鯖の実行ファイルで実行していますので...
	char/char.c、char/char.h、char/inter.c、char/makefile 修正。
	char/int_pet.c、char/int_pet.h 追加。
	map/makefile 修正。
	map/intif.c、map/intif.h、map/map.c、map/map.h、map/mob.c、map/mob.h、
	map/npc.c、map/npc.h、map/battle.c、map/battle.h、atcomand.c、map/pc.c、
	map/clif.c、map/clif.h、map/script.c 色々修正。
	map/pet.c、map/pet.h は殆どを自分の物に書き換えました。
	common/mmo.h 修正。
	db/pet_db.txt 修正。
	db/item_db.txt 修正。(携帯卵孵化機のbpet スクリプトが抜けていたので
		入れただけですが...)
	doc/INTER鯖パケット.txtの名前をinter_server_packet.txtに変更とペットの
		保存等に使うパケットを追加。
* 今度のペット実装によりキャラファイルの構造が変わり以前の物と互換できない
	ので tool/convert.c を追加しました。
	単独でコンパイルできますのでコンパイルしたあと実行してキャラ
	ファイルを変換してください。そうしないとキャラが全部飛びますので...
* ペットの親密度が0になるとペットはその場で動けなくなりその状態で他の
	マップに移動するか終了するとペットは消滅します。一応ペットの逃走を
	実装するつもりで作ったのですが本鯖にあってるかどうかはわかりません。
* 移動速度が遅いペットの場合離れ過ぎるとついて来れなくなります。でも
	この場合マップを移動してもちゃんとついて来ます。
	消滅したりはしません。
* 移動速度が速いペットはキャラより先に移動します。本鯖の方がどうなのか
	わからないのでペットの移動はモンスターの移動速度で移動する
	ようにしました。
・battle_athena.cnf
pet_catch_rate 追加。
	ペットの捕獲倍率を設定します。(設定しないと100)
	基本的にペットの捕獲に使ってる公式は
		(pet_db.txtの捕獲率 + (キャラレベル - モンスターレベル)*0.3 + luk *0.2)
		* (2 - モンスターの現在HP/モンスターの最大HP)
	になります。自分なりに作った物ですので本鯖とはかなりの
	違いがあるかも知れません。(モンスターのHPを減らせば減らす程捕獲率が
	上がる仕組みですが...)
pet_rename 追加。
	ペットの名前を変更するかどうかを決めます。(設定しないとno)
	yesは何度でも名前の変更が可能。
	noは一度変更するともう変更不可能になる
pet_hungry_delay_rate 追加。
	ペットの腹が減る時間の倍率です。(設定しないと100)
	倍率が高いと腹が減り難くなります
mvp_exp_rate 変更。
	すでにstruct mob_dbのmexpperはゴミになっているので(MVP EXPは
		MVPアイテムが取れなかった場合入るので意味がありません。)
	MVP EXPの量の倍率になるように変更。(mob.cを修正)
・char_athena.cnf
autosave_time 追加。
	自動保存する時間を決めます。(設定しないと300)
	單位は秒です。(ファイルに保存する時間の間隔です。)
・map_athena.cnf
autosave_time 追加。
	自動保存する時間を決めます。(設定しないと60)
	單位は秒です。(キャラ鯖にデータを送る時間の間隔です。これは
		ファイルに保存する時間の間隔じゃありません。)
・inter_athena.cnf
pet_txt 追加。
	ペットのデータを保存するファイルを決めます。(設定しないとpet.txt)
・@makepet コマンド追加。
	ペットの実装によって@itemで作った卵は使っても無駄になりますので
	これを使って卵を作ってください。
	@makepet <モンスターのID or 卵のID>
・@petfriendly コマンド追加。
	@petfriendly <数字>
	ペットを連れている時にペットの親密度を変更。(0~1000)
・@pethungry コマンド追加。
	@pethungry <数字>
	ペットを連れている時にペットの満腹度を変更。(0~100)
・@petrename コマンド追加。
	@petrename
	ペットを連れている時にペットの名前を変更できるように変更。
・int_guild.c、int_party.c 読み込むファイルにエラーがあってもプログラムを
	終了せずに進むように変更。
・pc_walk 123 != 1234 等のエラーが出ないように
	if((i=calc_next_walk_step(sd))>0) {
		sd->walktimer=add_timer(tick+i/2,pc_walk,id,sd->walkpath.path_pos);
	を
	if((i=calc_next_walk_step(sd))>0) {
		i = i/2;
		if(i <= 0)
			i = 1;
		sd->walktimer=add_timer(tick+i,pc_walk,id,sd->walkpath.path_pos);
	のように変更しました。
	tickが同じ数値になるのを防いたのですがこれでどんな影響が出るかは
	さっぱりわかりません。
	pc.c、mob.cを修正。
	でもこの修正をしても連続でクリックしたりするとキャラがしばらく
		止まるようです。(ペットのせいと思いましたがペットがなくても
		同じだったので他の原因かと...)
* doc/code_ref.txtとhelp.txtは面倒くさいので修正してません。
・gm_all_skill: yesで2-2のスキルも表示されるように変更。(試いせはいませんが...)
	pc.c
		pc_calc_skilltre()  修正。
・カートレボリューションのダメージ計算を修正。
	武器研究を二重計算していたので修正。
	battle.c
		Damage battle_calc_weapon_attack() 修正。

--------------
//381 by 胡蝶蘭

・MOBの行動修正
	・PCのいないマップのMOBは時々ワープするようになりました
	・PCのいるマップのMOBは歩く以外に、時々沸き直すようになりました
	（これまたパフォーマンスに影響があるかもしれません：少し重くなるかも）
	・手抜き処理でブロックの有効判定を行うようにしました
	（HP無限MOB問題修正？）
	・ルート時スキル使用機構実装

	mob.c/mob.h
		mob_ai_sub_lazy(),mob_ai_sub_hard()修正
		MSS_LOOT追加,mob_readskilldb()修正

・MOBスキルの属性変更を実装しました。
	
	map.h
		struct mob_dataに def_eleメンバ追加
	mob.c
		mob_spawn()でdef_eleをセットするように変更
	battle.c
		battle_get_element()でdef_eleを読むように変更
	skill.c
		skill_castend_nodamage_id()修正

・クァグマイアの効果範囲から出ると効果が切れるようになりました

	skill.c
		ユニット系処理修正

--------------
//380 by Ａの人

・カートレヴォリューションのダメージ計算実装
	battle.cを変更。

CHRISさん、ありがとう御座います。
ノックバック実装できなくて、困ってました（＞＜；

--------------
//379 by CHRIS

・カートレヴォリューションの実装
	skill.cとbattle.cを変更。

・マグナムブレイクにノックバックを追加。
	battle.cを変更。

（ソースを弄ったのは初めてなので、有ってるかどうか分かりませんが、自分では出来ました。）
（プログラム関係の書籍を買って勉強して初めて弄ったのです・・・。ガンバリマス！。）

--------------
//377 by 胡蝶蘭

・MOBの行動修正
	・近くにPCのいないMOBが時々ワープする仕様を止めました。
	・PCのいないマップのMOBは全く動かなくなりました。
	・PCのいるマップで、近くにPCのいないMOBは時々歩くようになりました。
	・その他細かいところ修正
	（パフォーマンスに影響があるかもしれません：少し重くなるかも）

	mob.c
		mob_randomwalk()追加
		mob_ai_sub_lazy(),mob_ai_sub_hard()修正など

・スキル修正
	・MOBがテレポートできるようになりました
	
	mob.c
		mob_warp()追加
	skill.c
		skill_castend_nodamage_id()修正

・ステータス異常の一部を実装/修正
	・PC/MOBともに速度減少の効果が現れるように（AGIの表示は変わらず）
	・PCのエンジェラス、インポシティオマヌス、速度上昇の効果を修正
	・MOBの2HQ、アドレナリンラッシュ、エンジェラス、インポシティオマヌス、
	  速度上昇/減少、グロリア、ブレッシングなどの効果実装
	・睡眠、凍結、スタンの必中効果実装
	・睡眠のクリティカル倍効果実装
	・暗黒の命中率、回避率減少効果実装
	・呪いのATK減少効果、LUK減少効果実装
	
	battle.c
		battle_get_*()修正
		battle_calc_weapon_damage()修正
	mob.c
		mob_get_speed(),mob_get_adelay()追加
	pc.c
		pc_calcstatus()修正

・item_value_db.txtでアイテムの価格を設定できるようになりました
	・価格データをオーバーライドできるようにしました。
	・これでitem_db2.txtを用意する必要がありません。
	
	(db/)
	item_value_db2.txt
		item_db2.txtの価格データ。
		item_value_db.txtにリネームすると読み込みます。
	(map/)
	itemdb.c
		itemdb_readdb()修正

・古木の枝を使うとMOBの名前が 0 になる問題の修正

	(db/)
	item_db.txt
		古木の枝のデータ修正

--------------
//375 by 胡蝶蘭

・MOB専用スキルの効果をいくつか実装
	多段攻撃、毒などの追加効果付与攻撃、属性付き攻撃、魔法打撃攻撃
	必中攻撃、防御無視攻撃、ランダムATK攻撃など。
	ただし、**全くテストしてません**。
	
	(db/)
	skill_db.txt
		MOB用スキルのデータを修正
	mob_skill_db.txt
		少し追加
	(map/)
	skill.c
		skill_castend_damage_id()修正
		skill_status_change_start()修正
		skill_additional_effect()修正
	battle.c
		battle_calc_weapon_attack()修正
	
・スキルを少し修正
	・ウォーターボールで敵が死んでいても撃つモーションをする問題修正
	
	skill.c
		skill_status_change_timer()修正

・MOBデータが変なので某Ｗのデータベースを流用してみる
	・データの並び順とか全く同じなんですね

	(db/)
	mob_db.txt
		某Ｗのmob_db.txt

・各種confのリファレンスを添付
	あくまでリファレンスなので、HowToなんかは書いてません。

	(doc/)
	conf_ref.txt
		confのリファレンス＋α


--------------
//373 by 胡蝶蘭

・MOBスキル使用機構仮実装
	・スキル使用時の処理はプレイヤーと共用(skill.c)です。
	・不都合が多いと思うので報告お願いします。
	・mob_skill_db.txtを埋めてくれる人も募集。
	  このデータは「ラグナロクのたまご」を参考にしています。
	
	(db/)
	mob_skill_db.txt
		MOBスキルデータベース(未完成)
		テスト用のデータしか入ってません。
	(map/)
	mob.c/mob.h
		mobskill_*追加、その他多数修正
	map.h
		struct mob_data に skill* 追加
	skill.c/skill.h
		skill_castcancel()やスキルユニット処理をMOBに対応させた
	battle.c
		battle_calc_damage()など修正

・ギルドのスキルが触れない問題修正
	・いつのまにかpc_skillupが古いものに変わっていたので修正
	
	pc.c
		pc_skillup(),pc_checkskill()修正

--------------
//368 by 胡蝶蘭

・MOB系の修正など
	・MOBが策敵範囲内のPC/アイテムを等確率でロックするようになりました
	  （アクティブ、ルート：いままでは該当ブロックのリンクリストの順などに
	    依存していた）
	・射程範囲内かつ、到達不可能地帯のPCをMOBがロックすると、
	  MOBが停止したり、その場で暴れだしたりする問題の修正
	・MOBロック中にIWなどで到達不可能になった場合、ロックを解除するように。
	・AEGIS方式で敵の移動を計算して移動不可能なら、Athena式で計算するように
	・ロックが解除されるときに数秒その場で停止するようにした
	・歩行が遅いMOBがとまらない/次の歩行開始が早すぎる問題を修正しました
	・ルート関連処理を少し修正

	mob.c
		mob_ai_sub_hard*()修正
		mob_can_reach()追加

・スキル使用時にターゲットブロックの有効性判定を行うように修正
・ルアフのダメージが武器計算になっているのバグを魔法計算に修正

	skill.c
		skill_castend_id()修正
		skill_status_change_timer_sub()修正


----------
//364 by いど
・以下のパケットの説明を変更
	doc/パケット解析.txt
		R 006a <error No>.B
		R 0081 <type>.B

・363でビルド時にwarningが出る不具合を修正
	map/guild.h

--------------
//363 by 胡蝶蘭

・ギルドの修正
	・ログインしていないPCを追放するとマップ鯖が落ちるバグ修正
	・メンバー追加直後に追加されたPCがギルド表示に追加されない問題修正
	・同じギルドに同垢別キャラが要るPCが脱退する/追放されると別キャラが
	  脱退してしまう場合があるバグ修正
	・メンバーがいるのに解散しようとするとマップ鯖が落ちるバグ修正
	
	(char/)
	int_guild.c
		guild_calcinfo(),mapif_parse_GuildAddMember()修正
	(map/)
	guild.c
		guild_member_leaved(),guild_member_added()
		guild_recv_info(),guild_break()修正

--------------
//362 by 胡蝶蘭

・ギルド解散実装

	(char/)
	int_guild.c
		解散処理を追加
	(map/)
	guild.c/guild.h
		guild_break(),guild_broken(),guild_broken_sub()など追加
	clif.c/clif.h
		clif_guild_broken(),clif_parse_GuildBreak()追加
	intif.c/intif.h
		intif_parse_GuildBroken()追加

--------------
//361 by いど

・360での@healの変更間違いを訂正
	map/atcommand.c

--------------
//360 by いど

・353の修正を削除
・@healで変更後の値がマイナスにならないように修正

--------------
//359 by いど

・class_equip_db.txtの文字化け修正

--------------
//358 by 胡蝶蘭

・ログイン時のdelete_timerのエラーを出ないようにした
	pc.c
		pc_authok()の修正

・ギルド関係の修正
	・メンバ勧誘時に最大人数の確認を行うように
	・データ通知処理をいくつか修正

	(char/)
	int_guild.c
		色々修正
	(map/)
	clif.c/clif.h
		clif_guild_inviting_refused()をclif_guild_inviteack()に改名
	guild.c/intif.c
		色々修正
	
・@guildlvupコマンド作成。ギルドレベルが調整できます。

	(char/)
	int_guild.c
		色々修正
	(map/)
	atcommand.c
		@guildlvup処理追加

・Makefikeのclean部分を修正
	
	(char/ map/ login/)
	Makefile
		・削除する実行ファイルのパスを ../athena/ から ../ に修正

--------------
//357 by 胡蝶蘭

・pc.cの文字化け修正
	文字化けしたファイルをアップするのも、それを改造するのも禁止しませんか？
	直すの面倒くさすぎます。

	pc.c
		文字化けの修正

・パーティやギルドに勧誘された状態でマップ移動やログアウトすると、
  勧誘を拒否するように修正
 
	pc.c
		pc_setpos()修正
	map.c
		map_quit()修正

・I-Athena自動復旧システム(B-NSJ氏作)をAthena用に改造して添付しました
	プログラムの性質上./toolフォルダではなく./にあります。
	athena.shの変わりにstartで起動するとmap鯖が落ちても10秒程度で復旧します
	プロセスは「map」で調べてますが他のプロセスに反応するときは
	「map-server」などに変えてみてください。

	start
		map鯖自動復旧システムのシェルスクリプト


--------------
//0356 by 死神

・athena.shを使わなくてもYare-launcherを使えるように変更。(自分試してましたが
一応動きました。でも窓の場合login-server.exeがlogin-server.exに登録されてしまい
Yare-launcherがlogin-server.exe続けて実行する問題があります。これはathenaの
問題ではありませんが...)
	comm/makefile以外のmakefile全てを修正。
	athena.sh修正。
	実行ファイルは.,/athena フォルダーじゃなく./ フォルダーに作られます。
・ 新規accountの許容するかどうかをlogin_athena.cnfで決めるように変更。(これは
	YareCVSを参考した物です。)
	login.c
		int mmo_auth() 修正。
	login_athena.cnf
		new_account 追加。
・char.c、login.c、inter.c、map.c、battle.cで一部のstrcmpをstrcmpiに変更。

--------------
//355 by ゆう

・左手装備も考慮した二刀流に修正
　（ダメージ計算のみで見た目等は変更なし）

map.h
	map_session_dataに左手用の変数を追加

battle.h
	battle_get_attack_element2()追加

battle.c
	battle_get_attack_element2()追加
	battle_calc_weapon_attack()に
	　二刀流の処理を追加修正
	　クリティカルよりダブルアタックを先に判定するように修正
	　過剰精錬の追加ダメージを精錬ダメージの次に処理するように修正
	　（これらは独自に調べたもので間違っている可能性あり）

pc.c
	pc_calcstatus()に左手用の変数に値を入れる処理を追加		
	pc_equipitem()の二刀流装備の場所がおかしかったのを修正

--------------
//353 by いど

・Yare-launcherを使うことが出来るようにMakefileとathena.shを変更

--------------
//352 by 胡蝶蘭

・詠唱中にクライアントを終了するとmap鯖が落ちる問題の修正
	skill.c
		skill_castend_id(),skill_castend_pos(),skill_castend_map()修正
	map.c
		map_quit()修正


・データバックアップ用のツール添付
	Perl製なので実行にはPerlが必要です。
	使用方法などはエディタで開いて見てください。
	使い方が良くわからない人は手を出さないほうがいいです。
	データが消えても責任は持ちません

	(tool/)
	backup
		データバックアップ用Perlスクリプト

--------------
//0351 by 死神
skill.c
	skill_use_id()に詠唱反応モンスターの処理を変更。(攻撃状態以外の場合
		詠唱反応を最優先にします。)
mob.c
	mob_ai_sub_hard_castsearch() 詠唱反応モンスターを二重処理して
		いたので削除。
	mob_ai_sub_hard() 詠唱反応モンスターを二重処理しないように変更。
pet.c - 0344に戻しました。(修正は少し分析をしてからにします。)
pet.h - 0344に戻しました。
char.h 
	CHAR_CONF_NAME 追加。
char.c
	do_init() 実行する時ファイル名が入力されていないとCHAR_CONF_NAMEを
		使うように変更。
map.h
	MAP_CONF_NAME 追加。
map.c
	do_init() 実行する時ファイル名が入力されていないとMAP_CONF_NAMEを
		使うように変更。
これでlogin.exe、char.exe、map.exeをathenaフォルダーにコピーした後名前を
login-server.exe、char-server.exe、map-server.exeに変更するとYare-launcherを使う
ことができます。これを使うと鯖が落ちる度に自動的に再実行してくれます。

--------------
//0345 by 死神
・キャスティング探知実装。
	mob.c
		mob_ai_sub_hard_lootsearch() 修正。
		mob_ai_sub_hard() 修正。
		mob_ai_sub_hard_castsearch() 追加。
		mob_target(), mob_ai_sub_hard_activesearch() ボスモンスターを
			mvp経験値によって認識するように変更。
		mob_ai_sub_hard_linksearch() 修正。
		mob_attack() 死んだふり、ハイディングをチェックするように変更。
		mob_readdb() 修正。
・pet.h
	MAX_PET_DBを100に変更。
・pet.c
	read_petdb() 修正。

--------------
//0344 by　過去の人i1
・　ペット腹減り実装およびそのほか色々修正
・　ペット餌やり実装

	pet.c
		pet_calcrate(struct map_session_data *sd);
		ペットの獲得確率計算
		pet_food(struct map_session_data *sd);
		ペット餌やりシステム
		pet_hungry_change( int tid, unsigned int tick, int id,int data );
		ペットが腹を減るロジック
		pet_status_int(struct map_session_data *sd);
		親密度計算
		pet_status_hungry(struct map_session_data *sd);
		満腹度計算
		pet_status_1a3(struct map_session_data *sd);
		パケット1a3設定関数
		pet_initstate(struct map_session_data *sd);
		ペットが初めて生まれたときの初期ステータス設定
	pet.h
		int pet_calcrate(struct map_session_data *sd);
		int pet_food(struct map_session_data *sd);
		int pet_hungry_change( int tid, unsigned int tick, int id,int data );
		int pet_status_int(struct map_session_data *sd);
		int pet_status_hungry(struct map_session_data *sd);
		int pet_status_1a3(struct map_session_data *sd);
		int pet_initstate(struct map_session_data *sd);
		を追加
	clif.c
		clif_pet_emotion(int fd,struct map_session_data *sd)
		餌をあげたときにエモーションを行う
	clif.h
		clif_pet_emotion(int fd,struct map_session_data *sd);

--------------
//0341 by 死神
・ルートモンスター実装。
	map.h
		LOOTITEM_SIZEを20に修正。
		struct mob_dataにint lootitem_count 追加。
	mob.c
		mob_spawn() 少し修正。
		mob_ai_sub_hard_lootsearch() 追加。
		mob_ai_sub_hard() 修正。
		struct delay_item_drop2 追加。
		mob_delay_item_drop2() 追加。
		mob_damage() 修正。
	battle.h
		struct Battle_Configにint monster_loot_type 追加。
	battle.c
		battle_config_read() 修正。
	battle_athena.cnf
		monster_loot_type: 0 追加。(基本的に0になっています。
			0の場合はLOOTITEM_SIZEまでアイテムを食べても
			またアイテムを食べて前のアイテムが消える仕様です。
			1の場合はLOOTITEM_SIZEまでアイテムを食べると
			もうアイテムを食べなくなります。

--------------
//0340 by 死神
・mvpバグ修正。
	mob.c
		mob_damage()でjに変えたはずの物に見落としありましたので
		修正しました。これでmvpアイテムで変な物が出なくなるはずです。
・class_equip_db.txt
	EUC-JISをS-JISに変更。(意味はありませんが他のファイルは
	全部S-JISだったので...単なるミスですが...)

----------
//339 by いど

・338を適用した状態でビルドエラーが発生する不具合を修正

----------
//338 by 過去の人i1

・　pet_db.txtに対応しました。
・　pet_dbをつかったプログラムの書き方に修正しました。
・　ペットの名前を変更する事が出来ます
・　ペットにアクセサリーをつける事が出来ます。
・　現在ペット餌やり進行中

	(map/)
	clif.c/clif.h
	・ pet関連の関数をほぼ修正及び追加いたしました。
	・ clif_parse_EquipItem()内部でペット用装備であるかどうかの判定を行ってます
	・ clif_parse()を修正しました。

	battle.h/battle.c
	・　battle_config.pet_rate変数を増やしました。mobに対する卵の獲得率
	　　を設定する事が可能となります

	pet.c/pet.h
	・ pet_initstate(struct map_session_data *sd);
	　初期のペットステータスを設定する関数です
	・ pet_npcid(struct map_session_data *sd,int egg_name_id);
	　ペットに割り当てられたnpc_idを返します
	・ pet_itemid(struct map_session_data *sd,int mob_id);
	　モンスターIDから卵のIDを割り出します
	・ pet_equip(struct map_session_data *sd,int equip_id);
	　ペットのアクセサリー装備です
	・ pet_unequip(struct map_session_data *sd);
	　ペットのアクセサリー解除です
	・ pet_calcrate(struct map_session_data *sd);
	　卵獲得確率計算を行い1or0を返します。
	・ pet_food(struct map_session_data *sd);
	　ペット餌やり考案です。まだ正常に動作しません。
	・ read_petdb()
	　pet_db.txtを読み込みpet_db[]に値を入れる関数です
	
	・ do_init_pet()
	　map鯖初期化でよびだしpet_db[]を使えるようにする為の
	ペット情報初期化関数です。
	
	map.c/map.h
	・　BL_PET変数を加えました
	・　map鯖初期化の時にdo_init_pet()を呼び出します。
	
	mmo.h
	・　s_pet構造体に変数追加。キャラクターがペットのデータを保持する為のシステム
	　の為今後も変数はそのつど増加する予定

	npc.c
	改善しました。

	(conf/)
	battle_athena.cnfに卵の獲得確率pet_rateを加えました。

----------
//337 by 胡蝶蘭

・ギルドの追加と修正
	・ギルドに経験値を上納すると、上納されるEXPが異常な値になるバグ修正
	・ギルドの敵対関係の追加

	guild.c/guild.h
		guild_payexp()の修正(上納EXP処理)
		guild_opposition()追加
		guild_allianceack(),guild_reqalliance(),
		guild_reply_reqalliance()の修正
	clif.c/clif.h
		clif_guild_oppositionack(),clif_parse_GuildOpposition追加

・ディレイ時間がdexの影響を受けるかどうかをbattle_athena.cnfに書けるように
	
	(conf/)
	battle_athena.conf
		delay_dependon_dex を追加
	(map/)
	skill.c
		skill_delay_fix()の修正
	battle.c/battle.h
		struct Battle_Configにdelay_dependon_dex追加
		battle_config_read()の修正(読み込み処理も変えてます)

--------------
//0336 by 死神
・スキルインデュアを少し修正。
・clif.c
	clif_skill_damage()、clif_skill_damage2() インデュア合わせて修正。
		(ただスキルや魔法になるとモーションが出ないパケットを
		見つけなかったので完全じゃありません。)
	clif_parse_ActionRequest()、clif_parse_UseSkillToId()、clif_parse_UseSkillToPos() 
		スキルディレイの時にメッセージが出るように修正。
・バックステップ実装、オリデオコン研究実装。
・skill.c
	skill_castend_damage_id()にあったスキルバックステップの処理を
		skill_castend_nodamage_id()に移動しました。
	スキルバックステップの処理でclif_skill_damage2()を呼ぶのをclif_fixpos()を
		呼ぶように変更。(これでダメージのモーションが出ずに
		移動できます。)
	バックステップと叫ぶように変更。
	 skill_produce_mix() オリデオコン研究適用。武器レベルが3以上の時に
		スキルレベル*1%がボーナスとして製造確率に付きます。
		エルニウムの確率判定追加。
・skill_db.txt - バックステップのnkを0から1に変更。(スキル番号150の物です。)
・produce_db.txt オリデオコン,エルニウムを追加。(これは本鯖にはない物です。
	よってクライアントには必要なアイテムが表示されません。)
	オリデオコン研究を少し使える物にするために追加しました。
	オリデオコンの場合はオリデオコン原石3つと石炭1つが必要で
	エルニウムはエルニウム原石3つと石炭1つが必要です。
・pc.c 少し修正。
	pc_heal()pc_percentheal() 少しだけ修正。
	pc_gainexp() ギルドにexpを上納する時にexpがマイナスにならないように
		修正。同時に2つ以上のレベルが上がるように変更。
		最大レベル以上にレベルが上がらないように修正。
	pc_checkbaselevelup()、pc_checkjoblevelup() 追加。レベルアップを
		チェックします。
	pc_itemheal() 追加。アイテムを使う時にVITとスキルによってボーナスが
		付く物です。スキルラーニングポーション実装。
・pc.h
	pc_checkbaselevelup(),pc_checkjoblevelup() 追加。
	pc_itemheal() 追加。
・script.c - スクリプトfixhealを除去。itemhealを追加。healがfixhealの機能をする
	ように変更。
	buildin_fixheal() を消しbuildin_heal()を元の物に戻しました。(つまり
		buildin_heal()がbuildin_fixheal()になりました。)
	buildin_itemheal() 追加。アイテムによる回復はこれを呼ぶようにして
		ください。
	buildin_heal()からボーナスの計算を除去。
・item_db.txt、 item_db2.txt - healをitemhealに変更。
・mob.c
	mob_damage() 0335でmvpに少し間違いがありましたので修正しました。
・skill.h
	MAX_SKILL_PRODUCE_DBを64から100に変更。


//0335 by 死神
・char/char2.cの一部にRETCODEが適用されてなかったのでそれを修正。
・char/char2.cのparse_char()でキャラを消す時に問題がありそうな所を修正。
・char/cha2.c,login/ login2.cをchar/cha.c, login/login.cに変更。
・char/makefile,login/makefileを変更。
・makefileとcommon/mmo.hを変更してOSを自動認識してRETCODEを自動に
	適用するように変更。
・common/grfio.cのgrfio_init()を修正。(コードをちょっときれい(？)に
	しただけですが,,,)
・インデュア実装。よってアンティペインメントも実装。
・map/clif.c
	clif_parse_LoadEndAck() 韓国クライアントのパッチに合わせて少し変更。
		(マップが変わる度に武器とシールドが見えなくなるためです。まだ
		日本クラとは関係ありませんが...)
	clif_skillinfoblock() upはいらないと思うので消しました。スキルポイントが
		256、512等の時スキルツリーが正しく表示されないことは
		もうありません。
	clif_guild_skillinfo() 同じようにupを消しました。
	clif_birthpet() pc_delitem() 呼ぶように変更。
	clif_damage() インデュアに対応するように変更。
・map/pc.c
	pc_percentheal() マイナスを入れても動くように変更。少し修正。
	pc_heal() 少し修正。
	natual_heal() 少し修正。
	do_init_pc() natual_healの修正に合わせて変更。
	pc_calcstatus() 弓を装備してないとワシの目が適用されないように変更。
		トラスト実装。
	pc_damage() インデュアに対応するように変更。
・map/pc.h
	pc_checkoverhp(), pc_checkoversp()を追加。
・map/map.h
	MAX_PC_CLASSを+1に。
・map/atcomand.c
	comandをcommandに変更。
	strncmpをstrcmpiに変更。よってコマンドが大文字、小文字を区別する
		必要がなくなりました。
・map/npc.h
	npc_parse_mob()を追加。(意味はありませんが...)
・map/temdb.c
	itemdb_readdb()でclass_equip_db.txtを読むように変更。
・db/class_equip_db.txt を追加。ここで装備するクラスを指定します。ない場合は
	item_db.txtにあるjobを使います。含まれてる物は完全な物ではなく
	使用例程の物です。
・map/skill.c - skill_status_change_start() インデュアの時間を正しく変更。
・map/battle.h
	battle_configのexp_rateをbase_exp_rateに変更。,job_exp_rateを追加。
	battle_get_mexp()を追加。
・map/battle.c
	battle_configのexp_rateをbase_exp_rateに変更。,job_exp_rateを追加。
	battle_get_mexp()を追加。
	battle_calc_magic_attack()を変更。ダーンアンデッドでボスの認識をmvp
		expでするように変更。
・map/mob.c
	mob_readdb() base_exp_rate,job_exp_rateに対応。
	mob_readdb() ボスの認識をmvp経験値でするように変更。
	mob_damage() mvpを取る時の処理を変更。重さが50%を越えると床に
		落ちるように変更と色々。
・conf/battle_athena.cnf
	base_exp_rate、,job_exp_rateを追加。
・0308で忘れた物
	古く青い箱、古いカード帖、古い紫色の箱の使用で得られたアイテムを
	持ってなくなったらアイテムを床に落とすように変更。
	製造はすでに0302で適用。

----------
//334 by C}{RIS

・あぷろだの332.txtをpet_db.txtとして同梱。

・各種テキストファイルのミスを修正
	・attr_fix.txt　属性修正がマイナスに働いて敵が回復する問題を修正。
		＞元の回復仕様に戻したい場合attr_fix.txtをリネームし、attr_fix_old.txtをattr_fix.txtにリネームして下さい。
	・mob_db.txt　モンスターの日本語名を本鯖と統一。
	・cast_db.txt　詠唱時間、ディレイを本鯖と統一。

----------
//333 by 胡蝶蘭

・ギルド機能追加
	・ギルドの同盟と同盟解消

	(char/)
	inter.c/int_guild.c
		パケット長/ギルド処理追加
	(map/)
	clif.c/clif.h
		clif_guild_reqalliance,clif_guild_allianceack,
		clif_guild_delalliance,clif_parse_GuildRequestAlliance,
		lif_parse_GuildReplyAlliance,clif_parse_GuildDelAlliance追加
		（ギルド同盟関係のパケット処理）
	intif.c/intif.h
		ギルド同盟関係のパケット処理追加
	guild.c/guild.h
		ギルド同盟関係の処理追加
	map.h
		struct map_session_dataにguild_alliance,guild_alliance_account追加

・ギルドエンブレムの変更がマップ鯖を再起動しないと有効にならないバグ修正

	guild.c/guild.h
		guild_emblem_changedの修正

----------
//331 by 過去の人i1

・ペットシステム修正(完成度25%)

	・各種捕獲用アイテムをそれぞれ対応する敵に対して使用することで
	　正しく卵が手に入るようになりました。
	・各種捕獲用アイテムを対応しない敵に使用した場合はルーレットが必ず失敗します。
	・ペットが瞬時に表示されるようになりました。
	・ペットを右クリックするとメニューが出るようになりました
	・他色々危険な要素を修正しました。(アイテムの数の減少等のバグを直しました)
	
	(db/)
	item_db.txt
	各種捕獲用アイテムに対応するようpetコマンドを正しく書きました。
	
	(common/)
	mmo.h
		s_pet	ペット構造体にpet_item_idを追加

	(map/)
		clif.c
		clif_birthpet()を修正。正しく卵が減る、正しい卵のIDを取得するよう修正
		これによって、正しくnpc_pet関数が動きます。
		
		clif_spawnnpc()を修正（WBUFPOS(buf,36,nd->bl.x,nd->bl.y)と、数値を26から36へ変更しました)
		これによってペットが瞬時に表示されるようになりました。

		npc.c
		npc_pet()を修正。

		pet.c/pet.h
		ペット関連の細かな関数をこちらにまとめる為、追加しました。
		現在はペットの判定関係をまとめてあります。

		script.c
		特に大きな修正はありません。
----------
//330 by 過去の人i1

・ペットシステム導入(完成度２０％)
	・敵に対して捕獲用アイテムを使用する事が可能となりました。
	　(まだ熟していないリンゴのみ使用可能、敵につかってもポリンの卵が手に入ります。)
	・各種卵に対してペットを表示する事ができます、ただしなぜか一度画面外に
	　でないとペットが表示されません。
	　(恐らくペット表示の際に行っているNPCステータスが正しく入っていない)
	・表示されたペットがパフォーマンスを行います。

	(db/)
	 item_db.txt
		携帯卵孵化機を使用可能に bpetコマンド(携帯卵孵化機使用)
		「まだ熟してないリンゴ」を使用可能に petコマンド(あるmobに対して使用可能にする)
	(common/)
	mmo.h
		s_pet　ペット構造体追加
		mmo_charstatus　ペット構造体宣言変数追加(pet)
	(map/)
		clif.c/clif.h
		
		int clif_catchpet(struct map_session_data *sd,int pet_id);
		void clif_ruletpet(int fd,struct map_session_data *sd);
		int clif_judgepet(struct map_session_data *sd,int target_id);
		int clif_sendegg(struct map_session_data *sd);
		void clif_listpet(int fd,struct map_session_data *sd);
		int clif_birthpet(struct map_session_data *sd,int pet_id);
		void clif_menupet(int fd,struct map_session_data *sd);
		
		以上の関数を追加しペットに関する処理を行っております。
		(ルーレット、卵選択窓、パフォーマンス、そのほか色々)

		npc.c/npc.h
		int npc_pet(struct map_session_data *sd,int name_id);
		ペット表示の為の関数を追加
		(この関数内部でペットの表示処理を行っています。適切に修正してください)

		script.c
		int buildin_catchpet(struct script_state *st);
		int buildin_birthpet(struct script_state *st);
		を追加しました。スクリプトにpet,bpetを追加しました。
----------
//329 by 胡蝶蘭

・ギルド機能追加
	・ギルドへのEXPの上納
	・ギルドレベルアップ
	・ギルドスキルの割り振り

	(db/)
	exp_guild.txt
		ギルドレベルの経験値データベース
	(common/)
	mmo.h
		GBI_*,GMI_*の定義の追加など
	(char/)
	int_guild.c
		EXPやレベルアップ、スキルアップ処理追加など
	inter.c
		パケット長追加
	(map/)
	guild.c/guild.h
		ギルド処理追加
	intif.c/intif.h
		ギルドパケット処理追加
	clif.c
		clif_guild_skillinfo()修正
		clif_guild_skillup()追加
	pc.c
		pc_skillup()でギルドスキルの場合はguild_skillup()を呼ぶように
		pc_gainexp()で上納EXPのためにguild_payexp()を呼ぶように

・ギルド機能修正
	・ギルドメンバーがログインやログアウトすると、
	  ログイン中のギルドメンバーにギルド系パケットが送られなくなる問題修正

	guild.c
		guild_recv_memberinfoshort()の修正

----------
//328 by 胡蝶蘭

・ギルド機能の追加など
	・追放機能仮実装（追放したキャラも再加入可能＆一部情報がダミー）
	・スキルの表示（表示だけです。上げたりは出来ません）

	(common/)
	mmo.h
		struct guild_explusionの変更
	(char/)
	int_guild.c
		追放処理追加
		空ギルドチェックなど追加
	(map/)
	guild.c/guild.h
		ギルドスキル関係のアクセサなど追加
	clif.c
		clif_guild_skillinfo()の修正
		clif_guild_explusionlist()追加

・ターンアンデッドがボス系アンデッドでMISSになるバグ修正
	・ターンアンデッド失敗時のダメージが使われます

	battle.c
		battle_calc_magic_damage()の修正

----------
//327 by いど
・経験値テーブルに１箇所間違いがあったので修正
	db/exp.txt

----------
//326 by いど
・char鯖の新鯖,メンテナンス中のフラグ情報を設定できるようにした
・char鯖の鯖名の長さが16バイトになっていた部分を20バイトに修正

	(login/)
		login2.c
			parse_login()の修正
		login.h
			struct mmo_char_serverの修正
	(char/)
		char2.c
			check_connect_login_server(),do_init()の修正
	(conf/)
		char_athena.cnf
　			・char_maintenanceを1にするとログイン人数の後ろに(点検中)がつく
　			・char_newを1にすると鯖名の前に[新]がつく
	(doc/)
		パケット解析.txt
			・今回の変更に伴う一部修正

----------
//325 by Mind Twist(224&0293)
・敵(NPC)スキルの追加（ツリーに追加のみ）
	db/skill_db.txt
	・ギルドスキル相変わらず不明…この形式じゃないのかな？

----------
//324 by non

・mobのターゲット後移動を修正
・mobの移動速度をDBから反映させるように

	(map/)
	mob.c
		mob_ai_sub_hard()の修正

・斜め判定を修正
	・FW等での斜め範囲を本鯖仕様に

	(map/)
	map.c
		map_calc_dir()の修正

----------
//322 by 胡蝶蘭

・ログイン時に必ずパーティーから除名されるバグ修正
	・0318の「パーティを除名されたのに～」での修正ミス

	(char/)
	int_party.c
		パーティー競合時のメッセージに改行追加
	(map/)
	party.c
		party_check_member()の修正

・ギルド会話実装＆修正など
	(char/)
	int_guild.c
		ギルド競合時のメッセージに改行追加
	(map/)
	guild.c/guild.c
		guild_send_message(),guild_recv_message()など追加
	intif.c/intif.h
		intif_parse_GuildMessage()など追加
	clif.c/clif.h
		clif_guild_message(),clif_parse_GuildMessage()など追加
		clif_guild_basicinfo()のパケットを0150から01b6に変更

・バックステップがものすごく怪しいけど仮実装
	・使うとダメージを１喰らったように見えます。(実際はダメージ０です)

	skill.c
		skill_castend_damage_id()修正

------------------
//321 by　過去の人i
・弓矢装備効果のみ実装(多々おかしい点は存在するが装備が出来、矢の効果が出る)
	map/clif.c map/clif.h
		clif_itemlist()追加
		持ち物の中で弓矢に割り当てるパケット番号を0x8000とする処理追加

		clif_arrowequip(struct map_session_data *sd,int val); //self
		を追加。この中で弓矢装備パケット処理を行ってます
	map/pc.c
		pc_equipitem()　弓矢装備追加
	
	残された問題点：
	・　矢が減らない
	・　リログすると矢が１になる
	・　装備している矢が表示されない(←多分装備posの設定をしていない為かと)
	・　矢を装備解除できない(他の矢を装備しなおせば装備した矢の効果になります

----------
//320 by いど
・以下のパケットのエラーコードの説明を追加
	doc/パケット解析.txt
		R 006a <error No>.B
		R 0070 <error No>.B
		R 0081 <type>.B

----------
//319 by mk
・NPCとの会話中に装備変更、アイテム、スキルを使用できないようにしました
	map/clif.c
		各所でsd->npc_idをチェックしてます

・一部の被り物でも精錬後に装備箇所がおかしくなるバグを修正
	map/script.c
		buildin_successrefitem のコードを修正

・カードによる追加効果および異常耐性発動処理の修正
	map/skill.c
		インデックス初期値が間違っていると思われるので修正（SC_POISON -> SC_STONE）

・ボウリングバッシュの仮実装、グリムトゥースの使用条件追加
	・ボウリングバッシュを仮実装。吹き飛ばし処理やってるので
	　battle.c のblewcountをコメントアウトしました
	・グリムトゥースの使用条件（ハイディング＆カタール装備）チェック追加
	　こんな感じで他のスキル使用条件も追加してもよいのかな？
	・skill_blown の吹き飛ばし方向をターゲットとの位置が重なっているときは
	　ランダムではなくキャラクターを後ろへ吹き飛ばすように変更しました
	map/skill.c
		skill_castend_damage_id()変更
		skill_check_condition()変更
		skill_blown()変更

・アサシン二刀流処理に関する修正?
	・装備一覧へdrag&drop時に両手が赤くマーキングされるように修正
	　アサシンでは武器装備箇所が両手になるようitem_equippointを変更
	　それに伴いpc.cのpc_equipitemを修正
	・逆手ダメージを表示してみました
	　まともにダメージ計算やってないので攻撃回数の確認だけです
	・カタール追撃と左手攻撃のMISSを無理やり表示
	(map/)
	itemdb.c (itemdb.h、clif.c、pc.c)
		itemdb_equippoint() を変更 （二刀流を考慮）
		引数を(int nameid) -> (struct map_session_data *sd,int nameid)に
		引数変更に伴い宣言(itemdb.h)と呼び出し側(clif.c ,pc.c)も変更
	pc.c
		pc_equipitem(), pc_checkitem()装備チェックを修正
	battle.c
		battle_calc_weapon_attack(),battle_weapon_attack()
		上にも書きましたが左手のダメージ計算は行ってません
		(カード、属性等も未適用)右手と同じダメージ入れて左右修練適用してます
		カタール追撃と左手ダメージをミスさせる方法(パケット?)がわからないので
		計算後のダメージが-1の場合、damage=0を送るようにしています
		もっとよい方法があるのでしたら修正お願いします

・完全回避の計算をLukではなくFlee2で判断するように修正
	map/battle.c
		battle_calc_weapon_attack()
	
	(db)
	item_db.txt、item_db2.txt
		装備品の一部ボーナス効果を追加
	mob_db.txt
		某所でうｐされていたものを少し修正
		まだModeやDropに問題点があるかもしれません

----------
//0318 by 胡蝶蘭

・ある公平分配PTとそのPTに属さないPCが共闘すると鯖が暴走するバグ修正
	
	mob.c
		mob_damage()のEXP分配処理修正

・パーティを除名されたのに所属しているように見えるバグ修正
	・該当キャラがログアウト状態の時に除名され、その後、同垢別キャラが
	  同パーティに所属しなおし、さらに元のキャラでログインすると、
	  除名判定に失敗してパーティに所属したままであるとされてしまう問題修正
	
	party.c
		party_check_member()の修正
	

----------
//0317 by nabe

・「～さんから取引を要請されました。」が自分の名前になっていたのを修正
	trade.c/clif.c/clif.h
		clif_traderequest()で取引相手のキャラ名を渡すように変更

----------
//0316 by nabe

・両手武器を精錬すると片手装備になるバグを修正
	script.c
		buildin_successrefitem()で、両手武器の場合等に装備箇所チェック

----------
//0315 by 胡蝶蘭

・PCのSPAWNタイミングを変更
	・マップ移動(ログイン)時、ローディング終了後にSPAWNするように。
	・ロード中に攻撃されたりしなくなります。
	・ギルド/パーティ情報読み込み前に名前解決パケットが来る問題も
	  修正されるはずです。
	
	pc.c
		pc_setpos()でmap_addblock,clif_spawnpcの呼び出しを止めた。
	clif.c
		clif_parse_LoadEndAckでmap_addblock,clif_spawnpcを呼ぶように。

・inter鯖パケット処理の重大な問題を修正
	・一度にパケットを送信できなかった場合、無限ループに陥る問題修正

	(char/)
	char2.c
		parse_frommap()修正
	inter.c
		inter_parse_frommap()修正
	(map/)
	chrif.c
		chrif_parse()修正
	intif.c
		intif_parse()修正

・ギルドの機能追加
	・他人のエンブレムが見えるように。
	・ログイン直後の自分のエンブレムが見えるように。
	・脱退できるように。（追放はまだです）
	
	<パケット情報引き続き模集>
	・016c,016d,0163,015cなどの詳細な情報
	・ギルドスキルのIDがわかる人、教えてほしいです。
	  158,205,331あたり調べましたがダメな模様。
	
	(common/)
	mmo.h
		MAX_GUILDを36に。
	(char/)
	int_guild.c
		脱退のパケット変更
	inter.c
		パケット長修正
	(map/)
	clif.c/clif.h
		clif_set0078,clif_set007bの修正
		clif_guild_belonginfoの修正
		clif_guild_skillinfo追加
	guild.c/guild.h
		脱退などの処理追加
	intif.c/intif.h
		脱退などの処理追加など


----------
//0314 by いど

・char鯖とmap鯖の鯖数上限を30に引き上げ
	login/login.h
	char/char.h

・map_athena1.cnfをmap_athena.cnfにリネーム
　それに伴い、athena.shを変更

・パケット解析資料をdoc/に移動

	
----------
//0313 by 胡蝶蘭

・ギルドの機能追加
	・ギルド勧誘/役職内容変更/メンバーの役職変更など
	・guild.txtの書式がまた変わりましたが、前のデータも読み込めるはずです。
	
	<パケット情報引き続き模集>
	・016c,016d,0163,015cなどの詳細な情報
	・自分以外のPCの所属ギルドIDを通知するパケット
	
	(common/)
	mmo.h
		struct guild を変更
	(char/)
	int_guild.c/inter.c
		処理追加/パケット長追加
	(map/)
	guild.c/guild.h/intif.c/intif.h
		処理追加
	clif.c/clif.h
		ギルド関係のパケット処理追加

・help.txtを修正
	help.txt

・0311による文字化けを修正
	README
	map/pc.c

----------
//0312 by いど

・@hコマンドを@helpに変更
・読み込むマップデータの定義部分を変更

----------
//0311 by tk44
・Assassin 二刀流装備、ASPD問題修正
	- map\pc.c
		pc_equipitem(), pc_calcstatus(), pc_checkitem()

・二刀流傷害計算修正まだ処理中

----------
//0310 by 胡蝶蘭

・ログやデータに使う改行コードをmmo.hで設定可能に
	(common/)
	mmo.h
		RETCODEで改行コードを文字列で定義します。
		Windows系だとCR/LFなので"\r\n",UNIX系だと"\n"です。
		別に正しく指定しなくてもAthena自体は問題なく動作するはずです。
	(login/)
	login2.c
	(char/)
	char2.c/int_storage.c/int_party.c/int_guild.c
		保存する改行コードをRETCODE依存に変更。
		改行コードに依存せずに読めるように修正。

・クライアントから不明なパケットが来たらダンプするように
	・#define DUMP_UNKNOWN_PACKET 1 をコメント化すればダンプしません。
	
	clif.c
		不明パケットの処理でダンプ処理を追加。

・ギルド機能の追加
	・エンブレム変更/告知変更実装
	・guild.txtの書式が変わりましたが、前のデータも読み込めるはずです
	
	(char/)
	int_guild.c/inter.c
		ギルドパケット処理、パケット長
	(map/)
	guild.c/guild.h
		機能追加
	intif.c/intif.h
		ギルド関係パケット追加
	clif.c/clif.h
		ギルド関係パケット追加

----------
//0309 by C}{RIS

・ボウリングバッシュを範囲攻撃化しました。
・プロンテラに装備品販売NPCを追加しました。
	map_athena1.cnfの
	npc_shop1J.txtをコメントアウトすれば無効に出来ます。
・アイテムの名前とIDを書いたテキストファイルを添付。（item.list）

----------
//0308 by 死神
・自然回復のプログラムを修正しました。
	これで大丈夫だといいですが...
	pc.c
		pc_natural_heal()等を修正。
		pc_percentheal()を少し修正。
		pc_checkskill()を少し修正。スキルがない場合0を返すように
		変更しました。他の.cファイルも修正する必要がありましたので
		修正しました。
・4人目と5人目のキャラを消せない問題を修正。
・始めからナイフとコットンシャツを持つように変更。
・LOOK_SHEILDをLOOK_SHIELDに修正。
・mmo_charstatusのsheildをshieldに修正。
・.logファイルやaccount.txtファイルをnotepadで開くと列が全部繋いでいる
	問題を修正。
・乱数を時間によって初期化するように変更。
	map.c
		do_init()を少し修正。

他に変更したファイルもありますが全部覚えてませんので...

----------
//0307 by 胡蝶蘭

・新規PCの初期位置をchar_athena.cnfに書けるようにした
	start_point: マップ名,x,y のように指定します。
	<例> start_point: new_1-1.gat,53,111

	(char/)
	char2.c

・ギルドの一部機能
	・ギルド作成くらいしか動きません
	・勧誘/脱退/解散/情報の変更/エンブレム/告知などはすべて未実装です
	
	<切実な要望>
	ギルド関係のパケット情報が全然足りません。わかる人は教えてください。
	現在のままではエンブレムと告知くらいしか実装できない可能性が…。

	(common/)
	mmo.h
		ギルド関係の構造体と定数追加
	(char/)
	inter.c
		パケット長情報追加
	int_guild.c/int_guild.h
		実際の処理追加
	(map/)
	map.h
		struct map_session_dataにギルド関係のメンバ追加
	guild.c/guild.h
		新規追加。ギルド機能用
	pc.c
		pc_authok()でギルド所属時、guild_request_info()を呼ぶように。
	clif.c/clif.h
		ギルドパケット追加
	intif.c/intif.h
		ギルドパケット追加

・0303での修正「MAXHPなどがサーバーとクライアントで～」を元に戻した
	・新PCを作るときに正しくHPなどを計算してくれるようになったので
	  戻しても平気だろうと予測。
	・ログイン直後に重量警告が出てしまうため。

	pc.c
		pc_authok()の修正

・範囲指定沸きの処理修正
	・できるだけ指定した数と同じだけ沸くように
	  (障害物などによる沸き妨害の回避失敗時、前の回避結果を使う)

	mob.c
		mob_once_spawn_area()の修正

----------
//0305 by いど
・新規PCの位置を初心者修練場に変更。
・map鯖がchar鯖に接続できない不具合の修正。

----------
//0304 by 死神
・自然回復の量と時間を変更。韓国鯖に適用されてる物ですが日本にも
	適用されてるはずです。(多分... やってませんのでわかりません。汗)
	HPは毎4秒に 1 + vit/6 + max_hp/200 を回復、
	SPは毎8秒に 1 + int/6 + max_sp/100 を回復します。
・スキルHP回復力向上による回復を
	スキルレベル*5 + max_hp/50に変更。
・スキルSP回復力向上による回復を
	スキルレベル*3 + max_sp/50に変更。
・スキル移動時HP回復実装。
	今の所止まってるのと比べて1/4の量を回復します。(時間は同じです。)
・vitとintによって回復時間が短くなるのではなく回復量が増えます。
・最大HPと最大SPの計算公式を変更。
	map.h
		int inchealtickの変わりにint inchealhptick;と int inchealsptick;を追加。
		int parame[6] を追加。最大SPの計算の為の物で装備によって上がった
		パラメータを持っつ。
	pc.c
		pc_hpheal(),pc_spheal(),pc_natural_heal_sub(),pc_natural_heal()を
		自分のコードに書き換えましたが一応正常に動きますが
		他のコードに影響がないかどうかはわかりません。
		pc_additem()を少しだけ修正。
		hp_coefficientをintからdoubleに変更。
		pc_calcstatus()とpc_readdb()を修正。
	job_db1.txt
		職業の計数を変更しました。(クルセイダー等のデータは
		完全な物じゃありません。)

・char2.cを少しだけ変更。
	char2.c
		make_new_char()を少しだけ変更。(作った直後にHPとSPが完全に
		回復してるように変えました。)
		parse_char()を少しだけ修正。韓国のクライアントで繋いても
		異常がないようにしました。(0x187パケットの処理を入れただけ
		ですが... これはYareから持ってきた物です。)
・strcmpi等のdefineをatcomand.hからmmo.hに移動しました。
	atcomand.h, mmo.h 修正。
・回復アイテムを使用する時vitとスキルHP回復力向上によるボーナスが付く
	ように変更。ボーナスは
	回復量 *(1 + HP回復力向上スキルレベル*0.1 + vit/100)
	になります。
・イグドラシルの実とイグドラシルの種を仮実装。(回復はしますがエフェクトが
	出ません。item_dbでタイプを変えても駄目でした。)
	script.c
		buildin_fixheal()とbuildin_percentheal()を追加。
		buildin_fixheal()はbuildin_heal()がスキルとvitによって回復量が変わる
		仕様になったので元のbuildin_heal()の名前だけを変えた物です。
		buildin_percentheal()は入力された数字を%としてHPとSPを最大HPと
		最大SPを %比率に回復します。
		スクリプトfixheal 、 percentheal 追加。使用方法はhealと同じです。
		fixhealはvitとスキルHP回復力向上によるボーナスがない物で
		percentheal は後の数字を %に認識します。
	pc.h
	pc.c
		pc_percentheal()を追加。
	item_db.txt、item_db2.txt
		イグドラシルの実とイグドラシルの種を変更。

----------
//0303 by 胡蝶蘭

・*.grfのパスをmap_athena.cnfにも書けるようにした。
	・map_athena.cnfに「data_grf: ../data/data.grf」や
	  「sdata_grf: ../sakurai/sdata.grf」のようにパス指定できます。
	・grf-files.txtがある場合そちらの設定が優先されます

	(common/)
	grfio.c/grfio.h
		grfio_setdatafile(),grfio_setsdatafile()追加。
		data_file,sdata_fileをファイルローカルなグローバル変数に変更。

・@stpoint,@skpointに負値指定の範囲チェックを追加
・@zenyコマンド追加（ゼニーの調整）
・@str,@agi,@vit,@int,@dex,@lukコマンド追加（基本パラメータ調整）

	atcommand.c
		@stpoint,@skpoint修正
		@zeny,@str,@agi,@vit,@int,@dex,@luk追加

・メマーナイトを使うとぼったくられていた問題を修正
・武器製造部分のコードを多少変更
	・材料消費処理をアイテムが複数インデックスに分かれている場合に対応させた
	  （３万個限界を超えると別インデックスを使う仕様だった気がするので）
	・失敗時にも周りに通知するようにした

	skill.c
		skill_check_condition()の修正
		skill_produce_mix()の修正

・武器製造確率をconfファイルで倍率指定できるように修正

	(conf/)
	battle_athena.cnf
		weapon_produce_rate追加
	(map/)
	skill.c
		skill_produce_mix()の修正

・武器ATKサイズ補正テーブルを外部から読み込むようにした
・精錬成功確率/精錬ボーナスなどを外部から読み込むようにした
・過剰精錬ボーナスによる追加ダメージ実装

	(db/)
	size_fix.txt
		サイズ補正テーブル
	refine_db.txt
		精錬関係データ
	(map/)
	pc.c
		pc_readdb()で読み込み
	battle.c
		battle_calc_weapon_attack()に過剰精錬ボーナス処理追加

・MAXHPなどがサーバーとクライアントで違う値に見えるバグ修正
	ログイン直後のステータス計算の結果を直ちに送信するようにした

	(map/)
	pc.c
		pc_authok()でのpc_calcstatus()のフラグを0にした
		これでpc_calcstatus()のフラグパラメータは未使用？

・item_dbの「忍者ス－ツ」を「忍者スーツ」に修正
	(db/)
	item_db.txt/item_db2.txt
		忍者スーツの名称変更

・ログイン人数を調べるツールを添付
	Perl製なので実行にはPerlが必要です。
	使用方法などはエディタで開いて見てください。
	使い方が良くわからない人は手を出さないほうがいいです。

	(tool/)
	getlogincount
		ログイン人数所得Perlスクリプト

----------
//0302 by 死神
・アイテム製造 確率判定実装。
	鉄の場合成功率は
		(20 + base_level*0.3 + DEX*0.2 + LUK*0.1 + 要求スキルレベル*6)%
	鋼鉄と属性石、星のかけらの場合
		(10 + base_level*0.3 + DEX*0.2 + LUK*0.1 + 要求スキルレベル*5)%
	武器は
		((2.5 + base_level*0.15 + DEX*0.1 + LUK*0.05 + 要求スキルレベル*5 +
		金敷 -  属性石と星のかけら) * (1 - (武器レベル - 1)*0.2) +
		スキル武器研究レベル*1)%
			金敷: ない場合 -5%で金敷は 0%、オリデオコンの金敷は
			2.5%、黄金の金敷は 5%、エンペリウムの金敷は 7.5% 
			属性石と星のかけら: 属性石がある場合 5%で更に
			星のかけらの数 * 5%を足します。
	になりますがちょっと確率が低すぎる気もしますので
	base_level*0.3 + DEX*0.2 + LUK*0.1をbase_level*0.5 + DEX*0.4 + LUK*0.3に
	base_level*0.15 + DEX*0.1 + LUK*0.05をbase_level*0.4 + DEX*0.3 + LUK*0.2
	程度に変えた方がいいかも知りません。
	skill.c
		skill_can_produce_mix() と skill_produce_mix() を修正。
	produce_db.txt
		星のかけらをスキル属性石製造が必要に変更。
・*.grf等を設置せずディレクトリからの読み込むように修正。(これはYareから
	持ち込んだ物ですが...)
	grfio.c
		grfio_init()を修正。
	grf-file.txt
		新規追加。grfファイルがあるディレクトリ設定用。
・読み込むマップの最大数を512に修正。
	mmo.h
		MAX_MAP_PER_SERVERを384から512に修正。
・pc.cにpc_search_inventory()を追加。
	機能はitem_idのアイテムを持ってるかどうかを確認して
	持ってる場合そのindexを返す。
	item_idが0の場合は空けてる所のindexを返す。
	pc_additem()とpc_takeitem()だけを少し修正。
・GMコマンドに@stpointと@skpointを追加。
	@stpoint 数字 - ステータスポイントを上げる。
	@skpoint 数字 - スキルポイントを上げる。
	atcomand.c
		修正。
	atcomand.h
		strcmpi等をLinuxでも使えるように修正。

----------
//0301 by 胡蝶蘭

・最大HPが32767を超えると異常な値になる問題の修正
・Lvが99を超えるときもエフェクトを出すようにした(自分には見えない模様)
・配置MOBによるイベントでイベント名が４バイト以上という制限をつけた
・teleport時に取引中断、チャット退室、倉庫保存処理をするようにした

	pc.c
		pc_calcstatus()の修正(HP計算)
		pc_setpos()の修正(取引中断など)
	clif.c
		clif_set0078(),clif_set007b(),clif_spawnpc()の修正(Lv99エフェクト)
	npc.c
		npc_parse_mob()の修正

・@hでhelp.txtが読めないときに落ちるバグ修正
・@lvup/@joblvupで負値を入れるとLvダウンが可能になった

	atcommand.c
		@h,@lvup,@joblvup処理の修正

・テレポートなどの消滅エフェクトの修正

	skill.c
		テレポの消滅エフェクトを変更

・状態異常に関するスクリプト実装 [sc_start]と[sc_end]。
・緑POT、緑ハーブなど実装
・装備ボーナスデータ追加
	
	(db/)
	item_db.txt/item_db2.txt
		装備ボーナスデータを追加
		緑POT、緑ハーブなどのスクリプト追加
	(map/)
	script.c
		buildin_warp()で消滅エフェクトを変更
		buildin_sc_start(),buildin_sc_end()追加

----------
//0299 by 胡蝶蘭

・NPCイベントでエクスポートされたラベルを指定できるようにした
	NPCスクリプトでOn～で始まるラベルを定義すると、エクスポートします。
	NPCイベントで"NPC名(orイベント名)::ラベル名"とすると、
	指定したラベルから実行できます。
	ラベル名は24バイト以内にして下さい。
	あとプログラム的にメモリ効率悪いです。後日修正予定
	<例>
		NPC「test」のスクリプト内で OnEvent: とラベル定義した場合、
		NPCイベント「test::OnEvent」で指定位置から実行できます。

	(conf/)
	npc_test_ev.txt
		ラベル指定のサンプルもちょこっと追加
	(map/)
	script.c/script.h
		script_get_label_db()などの追加。
		parse_scriptでscriptlabel_dbにラベルデータを追加する
	npc.c/npc.h
		npc_event_export()など追加
		npc_parse_scriptでラベルデータをエクスポートする
	map.h
		struct map_session_data のeventqueueのイベント名のサイズを
		50バイトにした。

・AGIとDEXによるASPD計算の最大値を180から190に変更
	pc.c
		pc_calcstatus()のASPD計算修正

・skill_db.txt/cast_db.txtの読み込みをskill.cに変更

	pc.c
		pc_readdb()の修正
	skill.c
		skill_readdb()の追加

・アイテム製造仮実装
	確率判定が未実装です。必ず成功します。
	
	(db/)
	item_db.txt/item_db2.txt
		アイテム製造に対応（携帯用溶鉱炉、金槌など）
	produce_db.txt
		新規追加。製造リスト。
	(map/)
	skill.c/skill.h
		struct skill_produce_db追加
		skill_readdb()でproduce_db.txtを読むように
	clif.c/clif.h
		clif_skill_produce_mix_list(),clif_parse_ProduceMix()追加
	script.c/script.h
		製造用コマンド[produce]作成。
		引数は製造用数値で、1-4が武器製造(Lv)、16が鉱石


----------
//0298 by 胡蝶蘭

・Login鯖のパスワード暗号化タイプを自動認識できるように変更
	login.hのPASSWORDENCを3にすると自動認識します。
	最初にpasswordencryptでチェックし、失敗すれば
	passwordencrypt2でチェックします。

	(login/)
	login2.c/login.h
		暗号化パスワードの照合部分を修正

・アカウント作成ツールを添付
	Perl製なので実行にはPerlが必要です。
	使用方法などはエディタで開いて見てください。
	使い方が良くわからない人は手を出さないほうがいいです。

	(tool/)
	addaccount
		アカウント作成ツールPerlスクリプト

・スキルの追加修正
	・ファイヤーウォールの回数制限をグループ毎からユニット毎に修正
	・クァグマイア仮実装 （敵の移動速度、キャラの表示数値は変化せず）
	・ウォーターボール仮実装（動作が正しいのか不明）
	・フロストノヴァ仮実装（エフェクトが良くわからないので適当）
	・ベノムダスト仮実装（範囲とかが正しいかどうか不安）
	・プロボック、オートバーサーク、聖体降福、砂まき、石投げの実装
	・エンチャントポイズンの毒付与実装
	
	*注意* 毒状態は見た目だけで効果は未実装
	
	(db/)
	skill_db.txt
		砂まき/石投げ/ウォーターボールなど修正
	(map/)
	skill.c/skill.h
		色々修正
	mob.c/mob.h
		mob_target()追加。MOBのタゲ用
	battle.c
		battle_get_*()系修正など
	pc.c
		pc_calcstatus()修正

----------
//0297 by 胡蝶蘭

・Login鯖がパスワード暗号化に対応
	暗号化keyは鯖起動時に一度だけ作成します。
	
	**注意**
	暗号化パスワードを使っている場合は、アカウントを作成できません。
	アカウントを作る場合はclientinfo.xmlを編集するなどして、
	パスワードを暗号化しないクライアントを使う必要があります。
	
	(login/)
	login2.c/login.h
		暗号化パスワードのパケット処理追加
		暗号化keyの生成処理追加
	md5calc.c/md5calc.h
		新規追加。md5計算用


・スキル使用ディレイにDEXが反映されないように修正
	skill.c
		skill_delay_fix()の修正

・死亡後も一部の状態異常の効果が持続する問題を修正
	pc.c
		pc_damage()で死亡時にpc_calcstatus()を呼ぶように修正
	atcommand.c
		死亡時処理を一本化するため@dieではpc_damageを呼ぶように修正


----------
//0295 by 胡蝶蘭

・クレイモアートラップなどにスキルを使うとマップ鯖が落ちる問題の修正

	battle.c
		battle_get_*()などでBL_PC,BL_MOBじゃないときの処理を追加
	skill.c/skill.h
		skill_unit_ondamage()追加

・約21Mzを超えるアイテムをNPCで扱うときOC,DC計算で値段がおかしくなるバグ修正
	DCでは20Mz、OCでは70Mzを超えるアイテムはdouble型にして計算します

	pc.c
		pc_modifysellvalue(),pc_modifysellvalue()の修正

----------
//0294 by 胡蝶蘭

・スクリプトコマンドでエリア指定のMOB沸き命令を作成
	areamonster "マップ名",x0,y0,x1,y1,"MOB表示名",MOBのclass,数,"イベント名"
	座標指定が(x0,y0)-(x1,y1)の任意ポイントになるだけで他はmonster命令と同じです

	script.c
		buildin_areamonster()追加
	mob.c/mob.h
		mob_once_spawn_area()追加

・アイスウォールの鯖側処理仮実装
	攻撃できないなどの問題はあるものの、鯖側ではとりあえず動きます。
	ただ、クライアントに進入不可能エリアを教えるパケットがわからないので、
	鯖側ではIWの回り込みを行う場合でも、クライアント側ではすり抜けている
	ように見えます。

	skill.c
		該当処理追加など

----------
//0293
・2-2次職のスキルコメントの修正（一部追加）
	(db/)
	skill_db.txt
	skill_tree.txt


----------
//0292 by 胡蝶蘭

・SHOP NPCに話し掛けるとNPCが反応しなくなる問題のまともな？修正Part2
	・売買できなかった問題修正
	
	map.h
		struct map_session_data にnpc_shopidメンバ追加
	npc.c
		npc_click()など修正

・スクリプト命令追加
	・指定エリアのユーザー数の所得
	  getareausers("マップ名",x0,y0,x1,y1)
	  指定マップの(x0,y0)-(x1,y1)にいるPCの数を計算
	・指定エリアのユーザーのワープ
	  areawarp "転送元マップ名",x0,y0,x1,y1,"転送先マップ名",x,y;
	  指定マップの(x0,y0)-(x1,y1)にいる全PCを指定マップの(x,y)に転送。
	
	script.c
		buildin_areawarp(),buildin_getareausers()追加

・スキル修正
	・テレポート使用時に「テレポート!!」と叫ぶように。
	・ストーンカースの成功率が低いのを修正
	
	skill.c
		skill_castend_nodamage_id()修正

----------
//0291 by 胡蝶蘭

・スクリプト命令追加
	・NPCの有効無効が切り替えられるようになりました
	  disablenpc "NPC名"で無効化、enablenpc "NPC名"で有効化。
	  NPC名が重複しているときの動作は不定です。
	  主にワープポイントを無効化するときに使います。
	
	・タイマーのカウントを変更するスクリプト命令追加
	  addtimercount "イベント名",ミリ秒
	  で、タイマの期限を延ばせます（負値を指定して減らすことも出来ます）

	・アナウンスの拡張
	  mapannounce "マップ名","アナウンス文字列",フラグ
	  で指定マップにアナウンスを流します。フラグは0で黄色、16で青です。
	  areaannounce "マップ",x0,y0,x1,y1,"文字列",フラグ
	  で指定マップの(x0,y0)-(x1,y1)のエリアにアナウンスを流します。
	  フラグはmapannounceと同じで、0で黄色、0x10で青です。
	
	(conf/)
	npc_test_arena.txt
		サンプルの修正
	(map/)
	script.c
		buildin_disablenpc(),buildin_enablenpc(),
		buildin_mapannounce(),buildin_areaannounce(),
		buildin_addtimercount()の追加
	npc.c/npc.h
		NPCの有効無効処理追加
	map.h
		struct npc_dataにflagメンバ追加(1ビット目が無効フラグ)
	clif.c
		clif_getareachar_npc()の修正
	pc.c/pc.h
		pc_addeventtimercount()追加

・SHOP NPCに話し掛けるとNPCが反応しなくなる問題のまともな？修正
	SHOP NPCと取引中でもイベントが起こるようになります。
	これはROの仕様上避けるのが難しいためこのような結果で落ち着きました。
	
	npc.c
		npc_click()等の修正

・スキッドトラップで残像が残る問題修正
	skill.c
		skill_blown()でclif_walkok()などを呼ぶように。
		skill_unit_onplace()のclif_fix*pos()を削除。

----------
//0290 by 胡蝶蘭

・スクリプトでMAP鯖内共有変数が使えるようになりました。
	変数名を$で開始すると、MAP鯖内の全員で共有する変数になります。

	言葉の問題ですが、PCのglobalregは「大域的」というより「永続性のある」
	変数であって、MAP鯖内共有変数のほうが大域的ってイメージが強いんですが…

	script.c
		mapval_db定義
		buildin_set(),buildin_input()の修正
		do_init_script()追加
	map.c
		do_init()でdo_init_script()を呼ぶように。

・イベントキューが実装されました
	・キューサイズは２です。必要なら増やしますが。

	サンプルの[ev_doテスト]がちゃんと動くようになったと思います。

	map.h
		struct map_session_dataにeventqueueメンバ追加
	npc.c
		npc_event_timer()追加
	script.c
		run_script()でEND処理でキューの処理追加

・スクリプトでタイマーが使用できるようになりました
	使用方法は、addtimer ミリ秒,"イベント名" でタイマー追加、
	deltimer "イベント名" でタイマー削除。

	(common/)
	timer.c/timer.h
		get_timer(),addtick_timer()追加
	(map/)
	map.c/map.h
		struct map_session_dataにeventtimerメンバ追加
		map_quit()でpc_cleareventtimer()を呼ぶように。
	pc.c
		pc_addeventtimer(),pc_deleventtimer(),pc_eventtimer(),
		pc_cleaereventtimer()追加
		pc_authok()でeventimerの初期化
	script.c
		buildin_addtimer(),buildin_deltimer()追加

・スクリプトの追加
	・getusers,getmapusers,killmonsterの追加
	  getusers(x)はユーザー数所得、x=0でPCのMAP,1=全MAP,8=NPCのMAP。
	  getmapusers("マップ名")は指定マップのユーザー数を所得する。
	  killmonster "マップ名","イベント名"で該当のマップにいる、
	  該当のイベント駆動指定モンスターを全て削除。
	・announceコマンド拡張
	  フラグの0x08ビットが1ならマップやエリア計算にPCでなくNPCを使う

	mob.c/mob.h
		mob_delete()追加
	script.c
		buildin_getusers(),buildin_getmapusers(),
		buildin_killmonster()追加
	clif.c/clif.h
		clif_GMmessage()の引数変更

・イベントサンプル追加
	簡単なアリーナのサンプルを追加。

	(conf/)
	npc_test_ev.txt
		従来のサンプルの修正
	npc_test_arena.txt
		簡単なアリーナのサンプル
		ワープポイントの無効化コマンドなどが必要と思われる。

・SHOP NPCに話し掛けるとNPCが反応しなくなる問題修正
	
	npc.c
		npc_buylist(),npc_selllist()修正

----------
//0289 by 胡蝶蘭

・イベント駆動型スクリプトの修正など
	・他のNPCに話し掛けているときはイベントが無視されるようになりました
	  =>キューに入れるなどの処理がいると思われる。

	この関係で、サンプルの[ev_doテスト]NPCをクリックしても
	IDエラーが出て何も起きません。イベントキューを作れば直るはず。

	npc.c
		npc_event(),npc_click()にnpc_idチェックを追加
	script.c
		終了時にnpc_idをクリアするように

・スクリプトコマンド[announce]の追加
	・GMメッセージによるannounce。
	  第1引数は文字列、第2引数はフラグで、
	  フラグの下位４ビットが0=全て、1=同じマップ、
	  2=画面内、3=自分のみ、4=同じマップ鯖に送信。
	  フラグの４ビット目は色フラグで、0x10=青、0x00=黄色

	script.c
		buildin_announce()の追加
	clif.c
		clif_send()でSELFの処理追加
		clif_GMmessage()の引数変更
	intif.c
		intif_GMmessage()の引数変更

・メモ禁止、テレポ禁止、セーブ禁止マップが指定できるようになりました。
	・NPCで、mapflagというタイプで、名前を nomemo 、 noteleportで
	  メモとテレポ禁止。nosave で、引数にセーブするマップ名と座標を指定。
	  
	詳しくは同梱のconf/npc_test_ev.txtを参照。

	(conf/)
	npc_test_ev.txt
		修正
	(map/)
	map.h
		struct map_data にflag,savemap,savex,saveyメンバ追加
	npc.c
		npc_parse_mapflag()追加
		do_init_npc()の修正
	pc.c
		pc_memo()でメモ禁止かどうかを確認
		pc_makesavestatus()でセーブ禁止ならマップを変更
		pc_randomwarp()でテレポ禁止かどうか確認
	skill.c
		テレポとポタでテレポ禁止か確認


・ファイヤーウォールで落ちる問題修正…だといいな
	
	skill.c
		さらにチェックを追加
		skill_blown()に落ちる原因っぽいもの発見したので修正

----------
//0288 by 胡蝶蘭

・自動鷹発動時に「ブリッツビート!!」と叫ばなくなりました
	skill.c
		skill_attack(),skill_additional_effect(),
		skill_castend_damage_id()の修正

・イベント駆動型スクリプトが記述できるようになりました
・モンスターを倒したときにイベントスクリプトを動かせるようになりました

	NPC定義のscriptで表示クラスを-1にするとイベント扱いになります。
	NPC定義のmonsterにイベント名を設定できます。
	スクリプトのmonsterコマンドにイベントを起こす引数追加。
	詳しくはサンプルを見てください。
	今後タイマーでイベントを起こせるようにしようと思っています。

	現状では、NPCウィンドウ操作中にイベントがおきて、
	そのイベントのスクリプトでNPCウィンドウを出すと問題が起きます。
	この辺は今後の課題ということで。

	(db/)
	item_db.txt/item_db2.txt
		monsterコマンドの変更による修正（古木の枝）。
	(conf/)
	npc_test_ev.txt
		サンプル
	(map/)
	npc.c
		npc_event()追加
		npc_parse_script()修正
		npc_checknear()修正
	clif.c
		clif_getareachar_npc()修正
	map.h
		struct mob_dataにnpc_eventメンバ追加
	mob.c/mob.h
		mob_once_spawn()の引数変更
		mob_damage()で死亡時にイベントを起こすように
	atcommand.h
		mob_once_spawn()の引数変更
	script.c
		buildin_monster()の修正

----------
//0287 by 胡蝶蘭

・モンスター情報スキルでＨＰが65535を越えていると正常な値が見れないバグ修正

	clif.c
		clif_skill_estimation()の修正

・古木の枝アイテムでクライアントがリソースエラーを出す問題が修正されました
・古木の枝アイテムで召喚できる敵が指定できるようになりました

	(db/)
	mob_branch.txt
		召喚可能な敵のリスト
	(map/)
	mob.c/mob.h
		struct mob_dataにsummonflagメンバ追加。召喚可能性。
		mob_once_spawn()の修正
		mob_readbranch()の追加
		do_init_mob()でmob_readbranch()を呼ぶように。

・古く青い箱、古い紫色の箱が実装されました。
	一部の未実装アイテムも出ます。item_db.txtにあるデータを検索しています。
	スクリプトgetitemで負の値を指定すると、その絶対値をフラグとして
	ランダムにアイテムを選択します。

	(db/)
	item_db.txt/item_db2.txt
		該当部分のスクリプト修正
	(map/)
	script.c
		buildin_getitem()の修正
	itemdb.c/itemdb.h
		itemdb_searchrandomid(),itemdb_searchrandomid_sub()追加

・詠唱データの一部修正
	(db/)
	cast_db.txt
		速度上昇などの修正

・NPCの向き修正など
	(conf/)
	npc_*.txt

----------
//0286 by 胡蝶蘭

・モンスター情報スキルでクライアントが落ちるバグ修正
	clif.c
		clif_skill_estimation()の修正

・詠唱反応モンスターが反応しないことがある問題を修正
	skill.c
		skill_use_id(),詠唱反応時、最低追跡距離を13に設定するように。

・スクリプトコマンド[warp]でセーブポイント移動やランダム移動が可能になりました
・ハエの羽、蝶の羽アイテムが実装されました
	スクリプトwarpでマップ名に"SavePoint"や"Random"が指定できます。

	(db/)
	item_db.txt/item_db2.txt
		ハエの羽、蝶の羽のスクリプト修正
	script.c
		buildin_warp()の修正

・@monsterコマンドによるMOBが復活しないようになりました
・スクリプトコマンド[monster]でMOBを発生させることができるようになりました
・古木の枝アイテムが実装されました

	スクリプト引数は monster マップ名,x,y,MOB名,MOBのID,数 です。
	マップ名が"this"の場合、スクリプトを実行したプレイヤーのいるマップ、
	x,yが-1ならプレイヤーの座標（どちらか一方のみそろえることも可能）、
	MOB名が"--en--"の場合、本来の英語名になり、"--ja--"の場合、
	本来の日本語名になります。MOBのIDが-1の場合、適当なIDになります。

	(db/)
	item_db.txt/item_db2.txt
		古木の枝のスクリプト修正
	(map/)
	mob.c/mob.h
		mob_once_spawn()追加
		mob_setdelayspwan()で復活禁止処理追加。
	npc.c/npc.h
		npc_get_new_npc_id()追加
	script.c
		buildin_monster()追加
	atcommand.c
		@monsterの修正

・@itemコマンドの修正（装備品などの問題）

	atcommand.c
		@itemの修正

----------
//0284 by 胡蝶蘭

・障害物があると遠距離攻撃ができなくなりました
・対地スキルが障害物上に使用できなくなりました

	path.c
		path_search(),can_move()の修正、can_place()の追加
	battle.c/battle.h
		battle_check_range()追加、射程と障害物判定。
		battle_weapon_attack()でbattle_check_range()を呼ぶ。
	skill.c
		skill_use_id()、skill_use_pos()でbattle_check_range()を呼ぶ。
	mob.c
		mob_ai_sub_hard()の処理を修正

・詠唱反応/リンクモンスターが実装されました
	skill.c
		skill_use_id()に詠唱反応モンスの処理追加
	mob.c
		mob_ai_sub_hard_linksearch()の追加
		mob_ai_sub_hard()にリンク処理を修正

----------
//0283 by れあ
・リザレクションの修正
	0282で生きてるＰＣにリザがかけれるのに、
	死んでるＰＣにはリザがかからなくなって
	いたのを修正しました。

----------
//0282 by 胡蝶蘭

・スキルの修正と追加実装
	・キリエエレイソンのエフェクトの問題修正。
	・リザレクションが生きているPCには掛けられないようになりました
	・ターンアンデッド/攻撃リザレクションがBOSSには効かないようになりました
	・ファイヤーウォールのヒット数制限を実装
	・ストームガストの実装
	  ただし、本鯖と違い凍結は確率のみで、最大ヒット数などが変です。
	
	skill.c
		skill_castend_nodamage_id()の修正
		ストームガストの処理追加

・スキルユニット処理に意地になって安全性チェックを追加。
	（落ちなくなる日は遠い？？）

	map.h
		MAX_SKILLUNITGROUPを32に増やした
	skill.c
		skill_status_change_*()にNULLチェック追加
	battle.c
		battle_calc_damage()に生存チェック追加
	map.c
		map_freeblock_unlock()にロック数チェックを追加

・その他修正
	・PCの最大HPが30000に制限されました。
	・PCの回復処理が修正されました
	・吹き飛ばし処理の修正
	・0281のitem_db.txtの変更をitem_db2.txtにも適用
	
	(map/)
	pc.c
		pc_heal(),pc_calcstatus()の修正
	path.c
		path_blownpos()の修正
	(db/)
	item_db2.txt
		0281の名前修正などを適用

----------
//0280 by 胡蝶蘭

・管理者のシステムを作成
	"conf/login_athena.cnf"の作成、管理者パス、GMパスの設定。
	アカウントを作るとき、
	＜例＞	ID: hoge_M	Pass: foobar@admin
	のように、パスワードの後ろに「@管理者パス」が必要に。
	login_athena.cnfのadmin_passの欄を消せば、今までのようにもつかえます。
	（その場合、admin_passの後ろだけでなく、行ごと消してください）
	
	(login/)
	login.h
		設定ファイルのデフォルト名追加
	login2.c
		アカウント作成のところを修正
		設定ファイルの読み込み追加

・@GMコマンド復活
	ただし、「@gm GMパス」として使います。
	GMパスはlogin_athena.cnfのものです。
	鯖の再起動の必要はありませんが、
	クライアントはリログする必要があります。
	
	＜注意＞
	同じアカウントの全てのキャラはPTから抜けて置いてください。
	また、倉庫のアイテムは全部引き出して置いてください。
	そうしないとゴミデータが残ります。

	(login/)
	login2.c
		アカウントID変更処理追加
	(char/)
	char2.c
		アカウントID変更処理追加
	(map/)
	chrif.c/chrif.h
		chrif_changegm(),chrif_changedgm()追加

・@pvpoffコマンド復活
	clif.c/clif.h
		clif_pvpoff()追加
	atcommand.c
		@pvpoffの処理追加

・空の倉庫データは保存されないように変更
	(char/)
	int_storage.c
		inter_storage_save()、storage_tostr()を修正

・@memoコマンド追加。
	任意の記憶域にメモを取れるようになりました。
	
	atcommand.c
		@memoの処理追加
	

----------
//0279 by 胡蝶蘭

・スキルユニット処理の問題対策
	とりあえずひたすらチェックを入れました。
	
	skill.c
		skill_unit_timer_sub(),skill_unit_move_sub(),
		skill_delunit()にユニットの生存判定を追加。
		skill_unitgrouptickset_search(),skill_unitgrouptickset_delete()
		skill_delunitgroup()にNULLポインタチェックを追加。

・スキルの追加実装と修正
	テレポート、ワープポータルの実装
	キリエエレイソンをMOBに掛けると落ちるバグ修正

	(db/)
	cast_db.txt
		ワープポータルの詠唱時間設定
	(map/)
	map.h
		struct skill_unit_groupのvalstrをポインタに変更
	clif.c/clif.h
		clif_parse_UseSkillMap(),clif_skill_warppoint()、
		clif_parse_Memo(),clif_skill_memo()追加
	skill.c/skill.h
		skill_castend_map(),skill_unit_onlimit()の追加
		skill_unit_*系の処理いろいろ追加。
		skill_status_change_start()のキリエの処理修正
	pc.c/pc.h
		pc_randomwarp(),pc_memo()追加

----------
//0278 by nabe

・スキルポイントを振ったときにステータスを更新
	pc.c
		pc_skillup()でpc_calc_skilltree()の代わりにpc_calcstatus()
・所持量増加を修正
	pc.c
		pc_calcstatus()の所持量増加によるmax_weight増分をskill*1000に

----------
//0277 by nabe

・付属品（カート、鷹、ペコ）の付け外しを改良
	(conf/)
	npc_event_rental.txt
		鷹、ペコを付けるスクリプトコマンドを変更
	(map/)
	battle.c
		battle_addmastery(),battle_calc_weapon_attack()で
		ペコペコ騎乗時の槍攻撃力補正を実装
	clif.c
		clif_parse_CartOffをclif_parse_RemoveOptionに変更
	pc.c/pc.h
		pc_calcstatus()でカート、ペコペコ乗りによる速度変化を計算
		pc_setoption(),pc_setcart()改良
		pc_setfalcon(),pc_setriding()追加
		pc.hにpc_isfalcon(),pc_isriding()マクロ追加
	script.c
		buildin_setfalcon()	鷹付加
		buildin_setriding()	ペコペコ乗り

----------
//0276 by nabe

・精錬NPC実装
	(conf/)
	npc_town_refine.txt
		精錬NPCスクリプトファイル新規追加
	(map/)
	pc.c/pc.h
		精錬成功率の表percentrefinery[5][10]を追加
		script.cから呼ばれる関数
			pc_percentrefinery()	精錬成功率
			pc_equipitemindex()	装備品インデックス
		を追加
	script.c
		buildin_getequipname()	装備名文字列（精錬メニュー用）
		buildin_getequipisequiped()	装備チェック
		buildin_getequipisenableref()	装備品精錬可能チェック
		buildin_getequipisidentify()	装備品鑑定チェック
		buildin_getequiprefinerycnt()	装備品精錬度
		buildin_getequipweaponlv()	装備品武器LV
		buildin_getequippercentrefinery()	装備品精錬成功率
		buildin_successrefitem()	精錬成功
		buildin_failedrefitem()	精錬失敗
		を追加

・スクリプトにWeight,MaxWeightパラメータを追加
	const.txt
		Weight,MaxWeightを追加

・スクリプトでのキャラ名表示方式を変更
	(conf/)
	npc_job_merchant.txt/npc_job_thief.txt/npc_town_kafra.txt
		mes "$charaname"; を mes strcharinfo(0); に変更
	(map/)
	script.c
		buildin_strcharinfo()を追加

----------
//0275 by 胡蝶蘭

・MVPの実装
	MVPの判定にはdmglogを使ってます。すなわち与ダメだけが計算対象です。
	被ダメは考慮されてません。
	経験値は無条件で入り、確率でさらにアイテムが入ります。
	アイテムは複数手に入ることもあります。

	clif.c/clif.h
		clif_mvp_effect(),clif_mvp_item(),clif_mvp_exp()追加
	mob.c
		mob_damage()にMVP処理追加

・スキルの追加実装と修正
	・マグナムブレイク、アローシャワー実装
	・吹き飛ばし系スキルが一部使用されないバグ修正
	・ダブルストレイフィングのダメージ計算式修正
	・武器攻撃系属性付きスキルで属性が反映されない問題修正
	・場所指定スキルが攻撃しながら詠唱できた問題を修正

	battle.c
		battle_calc_weapon_attack()の該当個所修正＆追加
	skill.c
		skill_castend_damage_id()に処理追加
		skill_use_pos()に攻撃停止処理追加

・カードスキルがカードを外しても使用可能な問題を修正
	pc.c
		pc_calc_skilltree()を修正

・アイテムドロップ率、exp所得倍率などの調整機能追加
	battle_athena.cnfで調整できる項目が増えました。
	詳しくはそちらを参照してください。
	
	(conf/)
	battle_athena.cnf
		mvp_hp_rate,item_rate,exp_rate,mvp_item_rate,mvp_exp_rate追加
	(map/)
	battle.c/battle.h
		増えた項目を読み込むように処理追加
	mob.c
		mob_db.txt読み込み時、データを調整する処理追加

----------
//0274 by 胡蝶蘭

・スキルの追加実装
	スキッドトラップ、ランドマイン、ブラストマイン、クレイモアートラップ、
	フリージングトラップ、サンドマン、アンクルスネア
	
	睡眠や凍結などの確率は適当です。
	罠発動時のエフェクトが出ません。ていうか出し方がわかりません。
	わかる人は教えてください。もしくは本鯖で罠発動時の複合化済みの
	生パケットデータでもいいので教えてください。
	
	(db/)
	skill_db.txt
		属性の修正
	(map/)
	skill.c
		該当個所
	battle.c/battle.h
		battle_calc_misc_damage()の該当個所
		battle_stopwalking()追加
	clif.c/clif.h
		clif_fixpcpos()追加
		clif_parse_WalkToXY()にアンクルで動けなくする処理追加
	mob.c
		mob_ai_sub_hard()にアンクルで動けなくする処理追加

・装備などのクリティカルボーナスが1/10になっているバグが修正されました
	battle.c
		battle_calc_weapon_attack()に追加分を計算する処理修正

・ブリッツビートの計算式が全然違うバグが修正されました
	battle.c
		battle_calc_attack()のBF_MISCの処置が間違っていたのを修正

・SWとニューマが武器攻撃ならどんなレンジの攻撃でも防いでいた問題を修正
	battle.c
		battle_calc_damage()のレンジ判定を修正

・オーバートラストとウェポンパーフェクションがPTメンバにもかかるように修正。
	効果は使用者とPTメンバで違いはありません。

	skill.c
		skill_castend_nodamage_id()の修正。

----------
//0273 by 胡蝶蘭

・キャラクターが消失したり幻影が出る問題が修正されました
	・吹き飛ばしスキルを受けると発生していた
	・斜め以外の歩行で発生していた

	map.c
		map_foreachinmovearea()の修正。
	skill.c
		skill_blown()に表示範囲更新処理を追加。
	mob.c
		mob_walk()で歩き終わったときに位置を再送信するように修正
	pc.c
		pc_walk()で歩き終わったときに位置を再送信するように修正

・スキルの追加実装と修正
	ファイヤーウォール、ファイヤーピラーの実装
	サンクチュアリでのノックバック方向を修正

	(db/)
	skill_db.txt
		ファイヤーピラー、ブリッツビートのヒット数修正
	(map/)
	skill.c
		skill_blown()に対象の向きによるノックバック処理追加
		その他必要な場所修正
	mob.c
		mob_walk(),mob_attack()で向きを保存
	pc.c
		pc_walk(),pc_attck()で向きを保存
	map.c
		map_calc_dir()追加。相対的な方向を求める


・クリティカル増加装備が戦闘時に計算されてないバグが修正されました
	battle.c
		battle_calc_weapon_attack()に追加分を計算する処理追加


・防御ユニット（SW/ニューマ）が敵に影響を及ぼすかどうかを
  battle_athena.cnfで制御できるようになりました
  	デフォルトは「及ぼさない」です。
  
	(conf/)
	battle_athena.cnf
		項目defunit_not_enemyを追加
	(map/)
	battle.c/battle.h
		struct Battle_Config に defnotenemyメンバ追加。
		battle_read_config()の処理を修正。
	skill.c
		skill_unitsetting()でSW/ニューマの処理を修正

・フェンカード装備時、死んでも詠唱が続くバグを修正
	（詠唱終了前に復活すれば魔法が発動する問題も修正になります）

	pc.c
		pc_damage()で死亡時skill_castcancel()を呼ぶように修正

・敵味方判定処理にバグがあったのを修正
	battle.c
		battle_check_target()の修正

----------
//0272 by 胡蝶蘭

・スキルの追加実装と修正
	・セイフティウォール、ニューマが実装されました。
	・ロードオブバーミリオン発動中に効果範囲外から範囲内に入ってきたとき、
	  敵味方の区別無く攻撃が当たる問題が修正されました。
	・サンクチュアリの射程が修正されました。
	・範囲魔法で倒した敵がHP0で残る場合がある問題が修正されました。

	(db/)
	skill_db.txt
		セイフティウォールとサンクチュアリの射程を8に変更
	(map/)
	skill.c
		skill_unit_onplace(),~ondelete(),~onout()などに、
		セイフティウォールとニューマの処理追加。
		skill_unit_move()にターゲットの敵味方判定を追加。
		skill_unit_timer_onplace(),~ondelete()にユニット生存判定を追加.
		skill_clear_unitgroup()追加。ユニットグループの全削除をする。
	battle.c
		battle_calc_damage()にセイフティウォールとニューマの処理追加。
		map_foreachinarea()など修正
	map.c
		map_quit()でskill_clear_unitgroup()を呼ぶように。

・スキルの吹き飛ばし処理を実装
	ユピテルサンダー、サンクチュアリ、スピアスタブ、
	ボーリングバッシュ、チャージアローの吹き飛ばし処理実装

	path.c/map.h
		path_blownpos()追加
	battle.c/battle.h
		struct Damageにblewcountメンバ追加
		battle_calc_*_damage()でblewcountをセットするように。
	skill.c/skill.h
		skill_blown()追加。吹き飛ばし処理。
		skill_attack()でskill_blown()を呼ぶように。
		skill_attack()のflagの吹き飛ばしビットは未使用に。

・歩行中のモンスターに攻撃したとき、モンスターにディレイが入るようになりました
	（攻撃のモーションの遅延を考えてないのであんまり意味がないかも？）

	(map/)
	mob.c/mob.h
		stateにMS_DELAYを追加。
		mob_damage(),mob_timer()などの修正

・歩行中のモンスターに攻撃したとき、位置がずれる問題の応急処置
	（まだ位置はずれるようです）

	clif.c/clif.h
		clif_fixmobpos()を追加
	mob.c
		mob_attack()でclif_fixmobpos()を呼ぶように。

・その他修正
	pc.c
		pc_stop_walking()でpath_lenを初期化するように。

----------
//0271 by れあ

・PTに関して少しだけ修正
	公平にしてからキャラを加入させると公平が解除されないバグを修正

----------
//0270 by 胡蝶蘭

・スキルの追加実装と修正
	ロードオブバーミリオン、サンクチュアリ、マグヌスエクソシズム

	(db/)
	skill_db.txt
		マグヌスのヒット数、属性調整
		サンクチュアリの属性調整
	(map/)
	map.h
		struct map_session_dataの修正
	clif.c/clif.h
		clif_skill_setunit(),clif_skill_delunit()、
		clif_getareachar_skillunit(),clif_clearchar_skillunit()追加
		clif_pcoutsight(),clif_pcinsight(),clif_getareachar()修正
	skill.c/skill.h
		忘れるほど多数変更。主にスキルユニット関連部分。
	pc.c
		pc_authok()でskillunit,skillunittickを初期化するように。
		pc_walk()でskill_unit_move()を呼ぶように。
	mob.c
		mob_spwan()でskillunittickを初期化するように。
		mob_walk()でskill_unit_move()を呼ぶように。
	battle.c/battle.h
		battle_calc_magic_attack()修正
		battle_check_target()修正
	map.c
		map_foreachobject()など修正

・こまかいバグ修正など
	・mobが回復しない問題修正

	battle.c
		battle_damage()修正

----------
//0266 by 胡蝶蘭

・魔法計算式の修正
	魔法倍率をダメージに掛けていたのをMATKにかけるようにしました。
	…こっちが正しいとしていいのかな？違うなら教えてください。
	
	battle.c
		battle_calc_magic_attack()の修正

・スキルの追加実装
	サイト、ルアフ、ロードオブヴァーミリオン
	
	ロードオブバーミリオンは３回の判定時に詠唱音が鳴ります…。
	clif_skill_damage()のtypeを色々変えてみましたがどうもうまくいきません。
	直せる人は直してくれるとうれしいです。
	（うーん、ひょっとしたら本来はグラフィックのないスキルユニットを
	  設置して、そのユニットのIDでダメージを与えるのかも？？）
	
	(db/)
	skill_db.txt
		ロードオブバーミリオンのヒット数を３から10に変更。
	(map/)
	skill.c
		skill_status_change_timer_sub()追加。
		skill_status_change_*()に処理追加。

・blockのメモリ解放の安全性の向上
	map_foreachinarea,party_foreachsamemapで回っているときに
	blockをチェインから削除すると、うまく回らない可能性がある問題修正。
	さらに、blockをメモリから解放すると危険な問題も修正。

	・foreach内で関数を呼ぶ前にblockがチェインから外れてないかチェック。
	・foreachに入ったときにロックしてメモリから解放されないようにする。
	  これはユーザーがfreeじゃなくmap_freeblockによって解放するように
	  プログラムする必要がある。（ループから呼ばれる可能性のある関数を作る
	  場合のことで、普通はfreeでもいちおう動く。）
	・map_foreachinmoveareaについては改良していないが、
	  このループでblockを削除することはありえない気がするのでいいとする。
	
	これは今後を見越した改良であって、現在の不安定さを直すものではない。
	（現在はforeach内でメモリを解放していない…はずなので。
	  ただ、スキルユニットなど一時オブジェクトを多用し始めると効果がある）
	
	map.c
		map_freeblock(),map_freeblock_lock(),~_unlock()追加。
		map_delobject()のfree()をmap_freeblock()に置換。
		map_foreachinareaでロックと安全性チェック。
	party.c
		party_foreachsamemap()でロックと安全性チェック


・スキルユニット機構実装
	設置系のスキルのための機構実装。実際のスキルの実装はまだです。
	
	skill.c
		なんかもう色々追加しました。
	map.c
		do_init()でdo_skill_init()を呼ぶように。
	map.h
		struct skill_unit,skill_unit_groupなど追加。
		map_session_dataの書き換えなど。

・その他細かいところを修正したと思うけど忘れました。

----------
//0264 by nabe

・$charanameを喋るNPCと話した時、map鯖が落ちることがあるバグを修正しました。
	script.c
		replacestr()がおかしかったのを手直ししました。

----------
//0263 by nabe

・露店開設中にカートアイテムを出し入れできないよう修正
	pc.c
		pc_putitemtocart(),pc_getitemfromcart()に、露店判定を追加

・露店アイテム購入のチェックを追加
	vending.c
		vending_purchasereq()で諸々の条件判定を追加

----------
//0261 by 胡蝶蘭

・拡大鏡、イグドラシルの葉が実装されました
	スクリプトにitemskillコマンド作成。一時的にスキルが使用できます。

	(map/)
	script.c
		buildin_itemskill()の追加など。
	skill.c
		アイテムスキルならSPなどを検査＆消費しないように修正
	clif.c/clif.h
		clif_item_skill()の追加。
	(db/)
	item_db.txt/item_db2.txt
		スキル使用アイテムのスクリプト修正

・パーティスキルの実装
	アンゼルス、マグニフィカート、グロリア、アドレナリンラッシュが
	画面内のパーティ全員に効果を及ぼすようになりました。
	
	skill.c
		skill_castend_nodamage_id()の該当個所の修正
	party.c
		party_foreachsamemap()の修正

・スキル関係の修正	
	キリエエレイソンが即時発動になっているのを修正。
	ストーンカースでエフェクトが存在しないバグ修正。

	(db/)
	skill_db.txt
		キリエエレイソイン修正
	(map/)
	skill.c/skill.h
		skill_check_condition()追加。スキル使用条件検査の一本化。
		skill_castend_nodamage_id()でストーンカース修正
		
・スクリプトのコードを整理
	get_val()でconst.txtの定数を所得できるように修正。

	(map/)
	script.c
		get_val()の修正（const.txtのtype==0の値が所得可能に）
		bonus(),bonus2()などの修正。
	(db/)
	const.txt
		type=1である必要が無いものを0に。
	item_db.txt/item_db.txt
		const.txtの変更に伴う修正。

----------
//0260 by 胡蝶蘭

・戦闘関係の設定がファイルに書けるになりました
	map鯖の第2引数にファイル名が設定されていると、それを使い、
	設定されてない場合は "conf/battle_athena.cnf"を使います。

	あと、一応範囲攻撃スキルについて説明。
	鯖が常にPVPに設定されている場合、パーティメンバじゃないPCにも範囲攻撃が
	あたります。嫌な場合はパーティを組むか、常にPVPをoffにして下さい。
	常にPVPがoffでも、@pvpでpvpフラグを入れた人の間では攻撃が当たります。
	ただし、一度pvpをonにすると、リログするまでonのままなので注意。

	(conf/)
	battle_athena.cnf
		中に説明書いてるので各自好きなように書き換えてください。

	(map/)
	battle.c/battle.h
		struct Battle_Configの定義。
		battle_config_read()など追加。
	skill.c
		CASTFIX,DELAYFIXの廃止とBattle_Configによる修正の追加。
	atcommand.c
		@pvpコマンドでpvpフラグをセットするように。
		（鯖設定の常にPVPがoffの時、両人がpvpをonにしてたら戦闘可能）
	map.c/map.h
		struct map_session_dataにpvp_flagを追加
		do_init()でbattle_config_read()を読むように。

・戦闘関係のコードが少し整理されました
	battle.c/battle.h
		battle_weapon_attack()追加。
		battle_calc_weapon_attack()の引数変更
		battle_calc_attack()を追加してbattle_calc_*_attack()を一本化。
	skill.c/skill.h
		skill_weapon_attack(),~_magic_~(),~_misc_~()の廃止、
		skill_attack()に一本化。
	pc.c/mob.c
		攻撃処理をbattle_weapon_attack()に一本化。

・アイテム鑑定スキルを実装
	商人のスキルの方です。虫眼鏡はまだです。
	
	skill.c
		スキル処理追加
	pc.c/pc.h
		pc_item_identify()追加
	clif.c/clif.h
		clif_item_identify_list(),clif_item_identified()追加
		clif_parse_ItemIdentify()追加
	
・スキルデータベースのコメント修正
	(db/)
	skill_db.txt
		商人のスキルのコメントがずれていたのを修正

----------
//0259 by れあ
・mob_db.txtの修正
	亀島モンスターやBOSSのステータス調整
	亀島モンスに適当にドロップを付けました。
	本鯖と異なる物を落とす場合もあります。
	
----------
//0258 by 胡蝶蘭

・パーティで一度公平にしたら各自所得に戻せないバグ修正
	(char/)
	int_party.c
		mapif_parse_PartyChangeOption()の判定修正

・スキルの追加実装（主に範囲攻撃系）
	ナパームビート（分散対応）、ファイヤーボール、
	サンダーストーム、ヘブンズドライブ、
	ブリッツビート（自動鷹込み）、スチールクロウ
	スキンテンパリング
	
	(db/)
	skill_db.txt/skill_tree.txt
		一部修正
	(map/)
	battle.c/battle.h
		battle_check_target()を追加。対象になるかを検討する。
		battle_calc_magic_damage()の引数変更。ダメージ分散処理追加。
		battle_calc_misc_damage()追加。
		battle_calc_weapon_damage()修正。
	clif.c/clif.h
		clif_skill_damage(),clif_skill_damage2()の引数変更。
		clif_skill_poseffect()追加。
	skill.c/skill.h
		skill_weapon_attack(),skill_magic_attack()に微妙に処理を纏めた.
		skill_area_sub()追加。範囲スキル用。
		skill_area_sub_count()追加。skill_area_sub()用、敵カウント。
		skill_castend_damage_id()修正。引数と処理を追加。
		skill_castend_nodamage_id()修正。引数と処理を追加。
		skill_misc_attack()追加。
		skill_additional_effect()修正（自動鷹）
		skill_castend_pos()修正。
		skill_castend_pos2()追加。

・弓で攻撃したとき計算にDEXでなくSTRが使われる問題を修正。
	battle.c
		battle_calc_weapon_damage()修正。

----------
//0257 by 胡蝶蘭

・item_db.txtの職業フラグと、カードの装備個所フラグを修正
	装備品はI-Athenaのデータを参考にして機械的にコンバートさせました。
	I-Athena側にない装備品は、あきらかに変なのは修正しましたが、
	知らないものが多すぎて、ほとんど放置です。
	カードは、武器用カードの装備個所が0になってるのを2(左手)に修正。
	両手武器の場合は別に判定してるので両手武器も問題ないはず。

	(db/)
	item_db.txt/item_db2.txt
		該当個所修正

・カード追加実装
	スタンなどの追加効果、それらへの耐性系統、オークヒーローカード実装

	(db/)
	item_db.txt/item_db2.txt
		スクリプトの修正
	(map/)
	map.h
		struct map_session_dataにaddeffなどのメンバを追加
	pc.c
		pc_calcstatus()、pc_bonus2()の修正
		pc_attack()でskill_additional_effct()を呼ぶように。
	skill.c/skill.h
		skill_additional_effect()でカードによる判定追加
		skill_status_change_start()で耐性を付けた。
	battle.c
		battle_calc_weapon_attack()でオークヒーローカード
		（クリティカル耐性）の処理を追加

・回避判定の修正
	攻撃者がPCの場合、最大命中率９５％制限をなしにしました。
	battle.c
		battle_calc_weapon_attack()を修正
	

・完全回避を実装
	へんてこな処理してます＆計算式適当です。

	battle.c
		battle_calc_weapon_attack()に処理追加。

・倉庫を開いたままログアウトしたときmap鯖内では開きっぱなしになってる問題を修正
	storage.c
		storage_storage_quitsave()を修正

・@item,@monster,@produceで名前指定できるように変更
	英語名、日本語名どちらでもOK。英語の場合は大文字小文字区別しません。
	
	atcommand.c
		該当個所修正
	itemdb.c/itemdb.h
		itemdb_searchname(),itemdb_searchname_sub()追加
	mob.c/mob.h
		mobdb_searchname()追加

・@refineで上げる数値を指定できるように変更
	atcommand.c
		該当個所修正

・@produceによる製造時のエフェクトを正しいものに修正
	clif.c/clif.h
		clif_produceeffect()追加
	atcommand.c
		該当個所修正

・露店スキル使用時の処理を少し修正
	skill.c
		skill_castend_id()でなく、skill_castend_nodamage_id()で
		露店開設を呼ぶようにした。

・stricmpの変わりにstrcasecmpを使うようにした
	（_WIN32か__EMX__が定義されているとstricmpを使います）

	(char/)
	int_party.c
	(map/)
	itemdb.c/mob.c
		マクロ定義の修正など

・スキルを少し修正
	グリムトゥースがハイディングで使えない問題修正
	武器研究の命中修正を実装

	skill.c
		skill_use_id()の修正
	pc.c
		pc_calcstatus()で武器研究に従って命中修正

----------
//0256 by nabe

・露店アイテム購入のバグ修正
	clif.c
		clif_vendinglist()で売り切れたアイテムは表示しないように

----------
//0255 by nabe

・露店アイテム購入のバグ修正
	vending.c
		vending_purchasereq()でzeny,weight部分修正

----------
//0254 by nabe

・露店を実装
	vending.c/vending.h
		新規追加。露店メイン処理
	skill.c
		skill_castend_id()に露店開設スキル処理を追加
	clif.h/clif.h
		露店関連パケット処理を追加
	map.h
		struct map_session_dataに、
			int vender_id;
			int vend_num;
			char message[80];
			struct vending vending[12];
		を追加

----------
//0253 by 胡蝶蘭

・stricmp未定義エラーがでる環境用の修正
	エラーが出た場合、int_party.cの最初のマクロ定義のコメント化のうち、
	どちらかを外してやり直してみると、うまくいくかも。
	最悪、下を有効にしたらうまくいくはず。（大文字小文字を区別するようになります）

	(char/)
	int_party.c
		コメント化済みのマクロ定義追加

----------
//0252 by 胡蝶蘭

・カードの一部実装
	（ステータス変化全般、武器属性、スキルはすでに実装済み）
	防具属性、詠唱時間変化、属性攻撃耐性、種族耐性、種族追加ダメージ、
	属性追加ダメージ、サイズ追加ダメージ、MAXHP、MAXSP増減、使用SP変化系、
	フェン、ドレイク、ホルン、深淵の騎士、黄金蟲、オシリスカードを実装

	(db/)
	const.txt
		bonus用の定数追加、bonus2の定数も追加
	item_db.txt/item_db2.txt
		カードのスクリプト追加
	(map/)
	map.h
		struct map_session_dataにhprateなど多数メンバ追加
	script.c
		bonus2コマンド追加
		buildin_bonus2()追加
	pc.c/pc.h
		pc_bonus2()追加
		pc_bonus()の処理追加
		pc_calcstatus()で各種追加メンバの初期化を行うようにし、
		hprateやsprateに従いmax_hp,max_spの調整もするように変更。
		pc_makesavestatus()でオシリスカード修正
	skill.c
		skill_castfix()でcastrateに従い、詠唱時間を調整。
		skill_castend_id()でdsprateに従い、使用SPを調整。
		skill_castend_nodamage_id()でカード修正を追加
	battle.c
		battle_calc_weapon_attack()でカード修正を追加
		battle_calc_magic_attack()でカード修正を追加
		battle_damage()でフェンカード修正を追加

・ステータス割り振りの表示上の問題修正
	STRを上げてもATKが変わらない問題、INTを上げてもMATKが変わらない問題修正

	map.h
		struct map_session_dataにmatk1,matk2メンバ追加
	pc.c
		pc_calcstatus()の修正
	clif.c
		clif_initialstatus()の修正
	battle.c
		battle_calc_magic_attack()の修正


----------
//0251 by nabe

・0250のバグ修正など
	カートを付けずにログインまたはマップ移動した後にカートを付けると、
	カートの中身が2倍の量に表示されてしまっていたのを修正。
	カートのアイテム数を更新するように修正。
	pc.h/pc.c
		pc_iscarton()マクロを追加
		pc_cart_additem(),pc_cart_delitem()にそれぞれ
		sd->cart_num++;とsd->cart_num--;処理を追加
	clif.c
		clif_parse_LoadEndAck()で、
		カートを付けているときのみカート情報を送信するようにした

----------
//0250 by nabe

・カートOFF、チェンジカート実装。
	(map/)
	pc.c/pc.h
		pc_setcart()を追加
	script.c
		buildin_setcart()を追加
		スクリプトコマンド「setcart;」でカートがつく
	clif.c/clif.h
		clif_parse_CartOff()追加。（カートをはずす）
		clif_parse_ChangeCart()追加。（チェンジカートのカート選択）
	(conf/)
	npc_town_kafra.txt
		カートサービスを「setcart;」に置換


----------
//0249 by 胡蝶蘭

・パーティのデータベースの矛盾を出来るだけ抑えるように。
	複数パーティに所属してるデータの検査、追加に失敗したときに脱退など。
	
	(char/)
	int_party.c
		party_check_conflict(),party_check_conflict_sub(),
		mapif_parse_PartyCheck()追加
	inter.c
		パケット長リストに0x3028追加
	INTER鯖パケット.txt
		パケット0x3028追加
	(map/)
	party.c/party.h
		party_check_conflict()追加。
		party_invite()で同アカウント所属チェックを行うように。
		party_member_added(),party_send_movemap()で
		party_check_conflict()を呼ぶように。
	intif.c/intif.h
		intif_party_checkconflict()追加

・パーティの座標、ＨＰ通知を実装
	変化があれば１秒に一回送信。

	map.h
		struct map_session_dataにparty_x,~_y,~_hpの３メンバ追加
	party.c/party.h
		party_send_xyhp_timer_sub(),party_send_xyhp_timer(),
		party_send_xy_clear(),party_send_hp_check()追加。
		party_recv_movemap()でsd->party_*を初期化するように。
	clif.c/clif.h
		clif_sendのPARTY*フラグを有効に。
		（PARTY,PARTY_SAMEMAP,PARTY_AREA,PARTY*_WOSの６種）
		clif_party_xy(),clif_party_hp()追加。
	pc.c/pc.h
		pc_authok()でsd->party_*を初期化するように。
		pc_walk()でパーティメンバが視界内に入ってきたときに
		party_hpを初期化するように。

・パーティのexp公平分配を実装
	party.c/party.h
		party_share_exp()追加
	mob.c/mob.h
		mob_damage()で公平分配処理追加

・スキルの修正と追加実装
	バッシュ、ピアースの命中率修正実装
	ピアースのサイズによる回数変動実装（プレイヤーは中型と仮定）
	バッシュ、ソニックブロウのスタン効果実装
	ストーンカース、フロストダイバ、インベナム、
	アスペルシオ、エンチャントポイズン、レックスデビーナ実装

	skill.c
		skill_additional_effect()追加
		skill_castend_damage_id()該当個所修正
		skill_castend_nodamage_id()該当個所修正
		skill_use_id(),skill_use_pos()でスキルが使用できないときは
		何もしないように修正。
	battle.c
		battle_calc_weapon_attack()の該当個所修正
		battle_get_dmotion(),battle_get_attack_element()修正
	clif.c
		clif_mob007b(),clif_mob0078でoptionなどを送るように修正
	pc.c
		pc_attack(),pc_walktoxy()で行動不可能なときは何もしないように。
	mob.c
		mob_stopattack()修正
		mob_ai_sub_hard()で行動不能なときは何もしないように。

・攻撃射程の判定追加
	相手が移動して届かないときは、移動パケットを送信

	clif.c/clif.h
		clif_movetoattack()追加
	pc.c
		pc_attack()で射程判定、届かないならclif_movetoattack()を呼ぶ。

----------
//0248 by nabe

・パーティ作成時に既にパーティに所属していた場合の処理を追加
	party.c
		party_create()に、既にパーティに所属していた場合
		clif_party_created(sd,2)を追加

・ディスカウント、オーバーチャージを計算
	pc.c
		pc_modifybuyvalue()、pc_modifysellvalue()で値段を計算


----------
//0247 by 胡蝶蘭

・パーティ実装
	公平分配は設定しても実際には公平分配されてない。
	パーティスキルはまだ自分にしかかからない

	(char/)
	int_party.c/int_party.h
		まともに実装
	inter.c
		パケット長リスト追加
	INTER鯖パケット.txt
		パーティのパケット追加
	(map/)
	party.c/party.h
		新規追加
	map.c/map.h
		struct map_session_dataにparty_sendedメンバ追加
		do_init()でdo_party_init()を呼ぶ
		map_quit()でparty_send_logout()を呼ぶ
	intif.c/intif.h
		パーティ関連の部分追加
	clif.c/clif.h
		パーティ関連の部分追加
		clif_parse_LoadEndAck()でparty_send_movemap()を呼び出す
	pc.c
		pc_authok()でparty_request_info()を呼ぶようにし、
		party_sendedを初期化するように。
	
・詠唱妨害されたとき画面上で詠唱をやめるように修正
	(map/)
	skill.c
		skill_castcancel()で詠唱中止パケ(合ってるのかな？)を送信

・超遠距離攻撃だと敵が反撃してこない問題を修正
	(map/)
	map.h
		struct mob_dataにmin_chaseメンバ追加（最低追跡距離）
	mob.c
		mob_attack()でmin_chaseを13に初期化する
		mob_walk()でmin_chaseが13より大きいなら少しずつ引いていく
		mob_ai_sub_hard()でmin_chaseにより追跡を判断、
		攻撃を受けた時にmin_chaseを彼我距離+13に設定

----------
//0246 by 胡蝶蘭

・カート実装
	map.h
		struct map_session_dataにcart_weightなど４つメンバ追加
	pc.c/pc.h
		pc_cart_additem(),pc_cart_delitem(),
		pc_cart_putitemtocart(),pc_cart_getitemfromcart()追加
		pc_calcstatus()でカート重量や個数などの情報を計算
	clif.c/clif.h
		clif_cart_itemlist(),clif_cart_equiplist(),
		clif_cart_additem(),clif_cart_delitem(),
		clif_parse_PutItemToCart(),clif_parse_GetItemFromCart()追加
		clif_parse_LoadEndAck()でカート情報、内容送信
		clif_updatestatus()でSP_CARTINFOでカート情報を送れるように
		clif_parse_MoveFromKafraToCart(),~ToKafraFromCart()追加
	storage.c/storage.h
		storage_additem(),storage_delitem()追加
		storage_storageadditemfromcart,~getitemtocart()追加
		storage_storageadd(),storage_storageget()で、
		storage_additem(),storage_delitem()を呼ぶように変更

・スキル詠唱ディレイなど実装
	clif.c
		clif_parse_WalkToXY()にskilltimerによる移動可否を追加
		clif_parse_UseSkillToId(),clif_parse_UseSkillToPos()に
		canmove_tickによる攻撃可否追加
	skill.c/skill.h
		skill_castcancel()を追加
		skill_use_id(),skill_use_pos()でディレイ時間計算および、
		canmove_tickの設定
	battle.c
		battle_damage()でskill_castcancel()の呼び出し追加

・0245のアイテムデータベース修正の通常価格版用意
	(db/)
	item_db.txt
		item_db2.txtに前のitem_db.txtの価格情報をマージしただけです。

----------
//0245 by れあ
	また例によって、相場修正版のみです。
・item_db2.txtの修正
	亀島新装備の効果を実装しました。
	ウィザードが杖を装備できないのを修正
	ウィザードがマジシャンハット、とんがり帽を
	装備できないのを修正
----------
//0244 by れあ
・mob_db.txtの修正
	亀島モンスターのデータをいれました。
	ただ、間違ってる部分がかなりあります。
	Speed,Delayは適当です。
	また、わからないのは韓国版のデータなので
	Mdefとか異常に高い気も。
----------
//0242 by 胡蝶蘭

・取引関連の変更と修正
	取引に使う変数をmmo_charstatusからmap_session_dataに移動しました
	
	(common/)
	mmo.h
		struct mmo_charstatusから取引関係のメンバ削除
	(map/)
	map.h
		struct map_session_dataに取引関係のメンバ追加
	trade.c
		構造体の変更にあわせて修正
	map.c
		map_quit()で取引中ならキャンセルするようにした

・カードの組み合わせ実装
	pc.c/pc.h
		pc_insert_card()でカードを実際に挿入する
	clif.c/clif.h
		clif_parse_UseCard(),clif_parse_InsertCard()追加
		clif_use_card(),clif_insert_card()追加

・一部のカード効果実装
	スキル習得カード、ステータスボーナスカードなど。

	(map/)
	map.h
		struct map_session_dataに装備カード検索用の変数追加
	pc.c/pc.h
		pc_calcstatus()でカードの処理追加
		あるIDのカードが装備済みか検索するための関数、
		pc_equip_card(),pc_equip_wcard(),pc_equip_dcard()を用意

・重量オーバー/鷹/騎乗アイコンの表示
	(map/)
	pc.c/pc.h
		pc_checkweighticon()追加、重量のアイコン処理
	clif.c
		clif_updatestatus()で重量送信時にpc_checkweighticon()の実行
		clif_changeoption()で鷹と騎乗のアイコン処理

・0241のアイテムデータベース修正の通常価格版用意
	(db/)
	item_db.txt
		item_db2.txtに前のitem_db.txtの価格情報をマージしただけです。


----------
//0241 by れあ
・アイテムデータベースの修正
	新頭装備のグラフィックが異なるのを修正
	装備の効果の実装
	上段・中段が間違ってたのを少し修正
	速報版ってことで間違え多いかも。
	テストもあまりしてません。
	あと、相場調整版しか用意してません。
	
	item_db2.txt
		亀島にあわせて調整

----------
//0240 by nabe

・取引を実装しました。
	(common/)
	mmo.h
		struct mmo_charstatus に
			int trade_partner;
			int deal_item_index[10];
			int deal_item_amount[10];
			int deal_zeny;
			short deal_locked;
		を追加
	(map/)
	clif.c,clif.h
		clif_traderequest()	: 0xe5（取り引き要請受け）
		clif_tradestart()	: 0xe7（取り引き要求応答）
		clif_tradeadditem()	: 0xe9（相手方からのアイテム追加）
		clif_tradeitemok()	: 0xea（アイテム追加成功）
		clif_tradedeal_lock()	: 0xec（ok押し）
		clif_tradecancelled()	: 0xee（取り引きキャンセル）
		clif_tradecompleted()	: 0xf0（取り引き完了）
		を追加。
	trade.c,trade.h
		trade_traderequest()	: 取引要請を相手に送る
		trade_tradeack()	: 取引要請
		trade_tradeadditem()	: アイテム追加
		trade_tradeok()	: アイテム追加完了(ok押し)
		trade_tradecancel()	: 取引キャンセル
		trade_tradecommit()	: 取引許諾(trade押し)
		を実装。それぞれclif.c::clif_parse_Trade*から呼ばれる。


----------
//0238 by れあ

・速度変更に関して少し修正
	atcommand.c
		速度変更の部分を少し修正
		これで一応動くみたい？
	pc.c
		ついでにですが
		速度上昇で歩行速度が上がるようにした。
		一応動くみたいですが適当なので
		おかしなところがあればお願いします。

----------
//0236 by nabe

・スクリプトでmenuで飛んだ先で直ぐmenuを書くと誤動作するバグを修正しました。
	script.c
		goto動作の後のRERUNLINEに対処するため、
		goto,menuで飛んだ後には、st.state==GOTOでrerun_posを更新。


----------
//0233 by nabe

・アイテムを装備する際の装備判定を追加しました。
	pc.c
		pc_equipitem()に装備判定（性別判定、装備LV判定、職業判定）追加

・重量判定スクリプトコマンドを追加しました。
	if (checkweight(アイテムID,アイテム数量))
	でそのアイテム×数量を取得できるかどうか判定できます。
	script.c
		buildin_checkweight()を追加

・スクリプト詰め合わせをathena dev-2.1.1用に移植しました。
	map_athena1.cnf
	npc_event_*.txt	イベントNPC
	npc_job_*.txt	転職NPC
	npc_mob_job.txt	転職用モンスター
	npc_town_*.txt	町NPC


----------
//0232 by 胡蝶蘭

・装備ボーナスが実装されました
	ボーナスに使うスクリプト(bonus,skill)を実装
	スクリプトはI-Athenaのデータを使ってコンバートしました。
	（まだカードには対応していません）

	(common/)
	mmo.h
		struct skillにflagメンバ追加（カードスキルかどうか）
	(map/)
	map.h
		struct map_session_dataにatk_eleなどのメンバ追加
		enumでSP_ATKELEMENTなど追加
	pc.c
		pc_bonus()の実装、pc_skill()追加
	script.c
		buildin_skill()の追加
		buildin_bonus()の修正(const.txtの定数が使えるように)
	clif.c
		clif_skillinfoblock()の修正(カードスキルは上げられない)
	(db/)
	const.txt
		bonusに使うための定数追加
	item_db.txt
		標準のデータに装備スクリプトを追加したもの
	item_db2.txt
		0213で相場調整されたデータに装備スクリプトを追加したもの

・詠唱関係のバグが修正されました
	(map/)
	skill.c
		skill_use_id(),skill_use_pos()を修正
	(db/)
	cast_db.txt
		少し追加（ブリッツビートなど）

・攻撃属性が適用されるようになりました
・星のかけらの修正が適用されるようになりました
	map.h
		struct map_session_dataにstarメンバ追加
	pc.c
		pc_calcstatus()で属性初期化
	battle.c
		battle_get_element(),battle_get_attack_element()修正
		battle_calc_weapon_damage()の該当個所修正
		
・杖装備時にMATK+15%が適用されるようになりました
	battle.c
		battle_calc_magic_damage()の該当個所修正

・製造武器のキャラクター名が正しく表示されるようになりました
	
	原理としては、map鯖内のキャラクタ名データベースを検索して、
	存在すれば即返信、存在しなければchar鯖に解決要求を出す。
	このとき、名前を要求してきたクライアントのIDをデータベースに登録する。
	char鯖から名前データがくると、対応するデータベースに名前をセットし、
	要求してきたクライアントに名前を返信する。
	未解決の同じキャラID解決を複数のクライアントが要求してきた場合、
	最後に要求してきたクライアントにしか返信しないが、
	返信されなかったクライアントは数秒後に再び解決要求を送ってくる
	（そしてそのときはmap鯖から即返信される）ので大きな問題はない。

	パケット0x2b08,0x2b09でmap鯖とchar鯖が通信してます。
	
	(char/)
	char.h
		UNKNOWN_CHAR_NAME定義（キャラデータが無いときに返される名前）
	char2.c
		parse_frommap()にパケット0x2b08の処理を追加
		
	(map/)
	chrif.c/chif.h
		chrif_searchcharid()追加
		chrif_parse()で0x2b09の処理追加
	map.c
		データベース charid_db 宣言
		struct charid2nick宣言。nickは名前、
		req_idは0で名前解決済み、0以外で未解決で解決待ちのブロックID
		map_addchariddb()追加。データベースへ名前登録、要求に返信。
		map_reqchariddb()追加。要求があったことをデータベースへ追加。
		map_charid2nick()でデータベースの検索
		do_init()で charid_db の初期化を追加
	clif.c/clif.h
		clif_parse_SolveCharName(),clif_solved_charname()追加


----------
//0231 by nabe

・スクリプトで mes "$charaname"; 等と書くとキャラの名前をしゃべる機能を追加。
	script.c
		buildin_mes()内で
		mes内部の$charanameをキャラの名前に置換する処理を追加。
		＃同様にして変数の値などをmes内部で表示するようにすることも
		＃できますが、これについては未実装です・・・。
		＃とりあえず
		＃	mes Global_Val;
		＃のように直接書くことで対処してください。

・敵に攻撃されたときにmap鯖が落ちることがあるのを修正。
	battle.c
		battle_calc_weapon_attack()の
		ディバインプロテクションのスキルチェック部分、
		pc_checkskill(sd,22)を、
		pc_checkskill(tsd,22)に。

----------
//0230 by nabe

・回避率増加スキルをステータスに反映。
	pc.c
		0228でのpc_calcstatus()の回避率増加分を元に戻しfleeを増加。
	battle.c
		battle_calc_weapon_attack()のhitrate計算で回避率保証を計算。
・グローバル変数を実装。
　	'@'もしくは'l'で始まらない変数名は、全てグローバル変数とみなされます。
	mmo.h
		struct mmo_charstatus に
			int global_reg_num;
			struct global_reg global_reg[GLOBAL_REG_NUM];
		を追加。
	pc.c
		pc_readglobalreg(),pc_setglobalreg()を追加。
	script.c
		get_val(),buildin_input(),buildin_set()に
		グローバル変数のための処理を追加。
	char2.c
		mmo_char_tostr(),mmo_char_fromstr()に
		グローバル変数のための処理を追加。

----------
//0229 by 胡蝶蘭

・一部スキルの実装/修正
	ディバインプロテクション、デーモンベイン、ビーストベイン実装
	エナジーコート修正（魔法による攻撃にはスキルが働かないように修正）
	武器攻撃系スキル修正（エフェクトを通常攻撃からスキルに変更）

	battle.c
		battle_addmastery()でベイン系追加
		battle_calc_damage()でエナジーコート修正
	skill.c
		skill_castend_damage_id()の武器攻撃系スキルの部分を修正

・敵攻撃計算をPCのものと一本化
	これでPCvsPC、PCvsMOB、MOBvsPC、MOBvsMOB(!?)を１つの関数で計算できます

	battle.c/battle.h
		battle_calc_weapon_attack()を修正
		battle_calc_weapon_attack_pc(),~mob()を削除
	mob.c
		mob_attack()で計算にbattle_calc_weapon_attack()を使うように修正

・詠唱時間データがない場合のデフォルトの詠唱時間を０に変更
	今までは１秒にしてましたが、バッシュとかがおかしくなるので。
	（バッシュとかのデータを用意すればこうしなくても直るんですが）
	
	pc.c
		pc_readdb()で1000msをセットするのを止めた

・遠距離攻撃してこないバグ、その他を修正
	mob.c
		mob_attack()の射程を修正し忘れていた
		mob_ai_sub_hard()で射程距離外の時、無移動の敵は
		ターゲットを外すようにした


----------
//0228 by nabe

・ダブルアタックのSkillIDを修正。
	battle.c
		battle_calc_weapon_attack_pc()で
		pc_checkskill(sd,49) -> pc_checkskill(sd,48)に。
・回避率向上を陽に表さない
	pc.c
		pc_calcstatus()でのfleeの回避率向上分を削除し、
	mob.c
		mob_attack()のhitrate計算で回避率向上を計算。
・盗蟲、盗蟲雌、盗蟲雄を正常化。
	npc_monster3J.txt
		mob_db.txtに合わせて、たぶん正しいと思われるIDに修正。
		盗蟲	1006 -> 1051
		盗蟲雌	1017 -> 1053
		盗蟲雄	1021 -> 1054
・デバッグメッセージの消し忘れ（？）を削除。
	pc.c
		printf("pc.c 63 clif_clearchar_area\n");をコメントアウト

----------
//0227 by 胡蝶蘭

・一部のスキル効果が実装されました
	HP回復向上、SP回復向上、マグニフィカート、
	ハイディング、クローキング、死んだふり、応急手当

	map.h
		struct map_session_data に inchealtick メンバ追加
	pc.c
		pc_spheal()でマグニフィカート処理追加
		pc_natual_heal_sub()で回復向上スキル処理追加
		pc_authok()でinchealtickを初期化するように変更
		pc_walk()でincheaktickを再設定するように変更
		pc_walk()でクローキングの終了条件を調査するように変更
		pc_walktoxy()で状態によって移動不可能にした
	skill.c/skill.h
		skill_status_change_start(),~timer(),~end()に処理追加
		skill_check_cloaking()追加、クローキングの終了条件を検査
	battle.c/battle.h
		battle_stopattack()追加
		battle_calc_weapon_attack()で攻撃を止める処理追加
	mob.c
		mob_ai_sub_hard()で攻撃を止める処理追加

・通常攻撃処理、対MOB、対PCを共用に。
	pc.c
		pc_attack_mob(),pc_attack_pc()削除
		pc_attack()に攻撃処理追加

・モンスターの行動の一部実装
	アクティヴ、無反応、移動しない、遠距離攻撃一部

	mob.c
		mob_ai_sub_hard()に行動追加
		mob_ai_sub_hard_activesearch()追加、近くのPCへの策敵

・オーバートラストの増加倍率が100倍になっているバグが修正されました
	battle.c
		battle_calc_weapon_attack()で、該当個所を修正


----------
//0226 by 胡蝶蘭

やっぱりテストはあんまりしていません

・一部のスキル効果が実装されました
	速度増加、エンジェラス、キュアー
	インポシティオマヌス、サフラギウム、リカバリー、グロリア
	ふくろうの目、ワシの目、集中力向上、回避率向上、解毒
	所持量増加、ラウドボイス、アドレナリンラッシュ、オーバートラスト
	ウェポンパーフェクション、マキシマイズパワー、２ＨＱ

	(map/)
	map.h
		struct map_session_dataにwatk2,def2など追加
	pc.c
		pc_calcstatus()にスキル修正追加
		atk2なども送信するように変更
	battle.c/battle.h
		battle_get_def2()など多数追加
		battle_calc_weapon_damage()で敵減算防御の所得を
		battle_get_def2()に変更
		battle_calc_magic_damage()で敵減算魔法防御の所得を
		battle_get_mdef2()に変更
		battle_calc_weapon_damage()でスキル修正を追加
	skill.c/skill.h
		skill_use_nodamage_id()の該当個所追加
		skill_status_change_start()の該当個所追加
	clif.c
		clif_updatestatus()のatk2などの処理追加
		clif_initialstatus()でatk2などの扱い変更、aspdなど送信追加
	
・精錬ダメージ修正/精錬防御修正が適用されました
	(map/)
	pc.c
		pc_calcstatus()でwatk2とdefの追加計算追加
	battle.c
		battle_calc_weapon_damage()でwatk2をダメージに追加

・inter鯖のパケット解析部の致命的な問題が修正されました
	TCP/IPプログラムでやってはいけないことをそのままやってました(汗
	inter鯖のパケット長データをinter.cに持つように修正されました。

	(char/)
	inter.c/inter.h
		パケット長データ inter_*_packet_length[] を追加
		パケット長チェック inter_check_length() を追加
		mapif_parse_*()でRFIFOSKIPをなしに変更
	int_storage.c/int_storage.h
		mapif_parse_*()でRFIFOSKIPをなしに変更
	int_party.c/int_guild.c
		仕様変更に対応させた変更
	INTER鯖パケット.txt
		パケット長リスト追加

・ちょっとした修正
	(char/)
	inter.h
		inter_cfgNameを"conf/inter_athena.cnf"に修正
	char2.c
		char.exe第２引数省略時、inter_cfgNameを使うように修正
	(db/)
	cast_db.txt
		ホーリーライトの詠唱時間追加（ディレイは適当）
		詳しい人追加求む

----------
//0225 by 胡蝶蘭

なんかかなり弄りましたが相変わらずテストはあんまりしてません。

・スキル使用時の変数を変更
	よく見たら最初から用意されてましたね。

	map.h
		struct map_session_dataのcast_*を削除
	skill.c
		cast_*の変数をskill*に変更。

・ステータス異常スキルの処理を追加（効果は未実装）
	見かけ上、ステータス異常に掛かったりとかだけ。
	効果はまだなし。

	skill.c/skill.h
		skill_status_change_start(),~end(),~timer(),~clear()追加。
		それぞれステータス異常の開始、終了、タイマ処理、全消去。
	map.c/map.h
		map_quit()でskill_status_change_clear()を呼ぶようにした。
		struct map_session_dataにsc_data,sc_count追加。
		struct mob_dataにsc_data,sc_count,option,opt1,opt2追加。
	pc.c
		pc_authok()でsc_data,sc_countを初期化するようにした。
		pc_setoptionでclif_changeoption()の引数変更。
		pc_damage()で死亡時にskill_status_change_clear()を呼ぶように。
	mob.c
		mob_spawn()でsc_data,sc_countを初期化するようにした。
		mob_attack()でbattle_calc_damage()を呼ぶようにした。
		mob_damage()で死亡時にskill_status_change_clear()を呼ぶように。
	battle.c/battle.h
		battle_get_*()たくさん追加。
		battle_calc_damage()追加。最終的なダメージ計算用。
		battle_calc_magic_attack(),battle_calc_weapon_attack()で
		battle_calc_damage()を呼ぶようにした。
	clif.h/clif.c
		clif_status_change()追加。ステータス異常アイコン表示用。
		clif_changeoption()の引数変更。
	atcommand.c
		clif_changeoption()を呼んでいる２ヶ所で引数変更。
		@dieでskill_status_change_clear()を呼ぶように。

・マグヌスエクソシズムの習得条件が間違っているのを修正。
	db/skill_tree.txt
		該当個所修正。（レックスエーテルナの必要Lvを１に）

・アクティブな敵は攻撃するとき時々ターゲットが変わるようになりました
	mob.c
		mob_ai_sub_hard()の攻撃されたか確認する部分に
		アクティブなら25%の確率でターゲットが変わるように変更。

・一部のスキル効果が実装されました
	キリエエレイソン、エナジーコート、レックスエーテルナ、
	ホーリーライト、リザレクション、ターンアンデッド、モンスター情報

	skill.c/skill.h
		skill_castend_nodamage_id()にスキルの処理を追加。
		skill_castend_*_id()の引数を変更
	battle.c
		battle_calc_damage()にスキルの処理を追加。
		battle_damage(),battle_heal()の引数変更
		battle_calc_weapon_damage(),battle_calc_magic_damage()引数変更
	clif.c/clif.h
		clif_skill_estimation()追加。モンスター情報送信用
	pc.c
		battle_calc_weapon_damage()呼び出しの引数変更

・storage.txtが無い場合inter鯖が強制終了する仕様を変更しました
	(char/)
	int_storage.c
		inter_storage_init()でファイルが読めないとexitしてたのを修正


----------
//0224
・2-2次職のスキルをツリーに追加しました（実装はまだです）
	(db/)
	skill_db.txt
	skill_tree.txt


----------
//0223 by 胡蝶蘭
・カプラ倉庫をinter鯖に対応させました
	いままでのstorage.txtはそのまま使えます。
	inter鯖用の設定ファイルとしてconf/inter_athena.cnfを使います。
	（設定ファイルはchar.exeの第２引数で他のファイルを指定できます）

	カプラ倉庫のinter鯖実装の概要
	
	inter鯖はstorage.txtの全データを持つ。map鯖はアカウントが要求するまで
	そのアカウントの倉庫データを持たない。クライアントから倉庫を開く要求が
	あったとき、map鯖は対応するアカウントの倉庫データをinter鯖に要求する。
	inter鯖からデータが届くとクライアントに倉庫データを送る。
	倉庫の出し入れはクライアントとmap鯖間の通信だけで行われる。
	クライアントが倉庫を閉じるか終了すると、map鯖は該当アカウントの
	倉庫データをinter鯖に送る。このときinter鯖の応答を待たずにクライアントに
	倉庫クローズを送る。inter鯖は倉庫データを受け取ると、
	全員分のデータをファイルに保存して、map鯖に成功ステータスを返す。
	map鯖は成功ステータスを無視する。(デバッグ用に画面に出力するだけ)
	inter鯖終了時にも倉庫データをファイルに保存する。

	map鯖でaccount2storageで新しい倉庫データを作るとき、
	すでに閉じられている倉庫データのメモリを使いまわしたほうがメモリが
	節約できるかも？（これは実装していません）

	(common/)
	mmo.h
		struct storage を map/storage.h から移動。
		inter鯖とmap鯖両方で使用するため。
	(char/)
	char2.c
		do_final()を作成、終了時にmmo_char_sync()以外にinter_save()を
		呼ぶようにした（これでinter_*_save()は全部呼ばれます）
		inter_init()をchar.exeの第２引数もしくは"conf/inter.cnf"で
		呼ぶようにした（athena.shにinter鯖コンフィグファイルを指定できます）
	inter.c/inter.h
		inter_storage_init(),inter_storage_save(),
		inter_storage_parse_frommap()を呼ぶように。
		inter_init()にコンフィグファイル名の引数を付けた。
		inter_config_read()追加、コンフィグファイルから
		倉庫とパーティー、ギルドのファイル名を読み込みます。
	int_storage.c/int_storage.h
		新規追加。倉庫部分のinter鯖機能。
	int_party.h/int_party.c/int_guild.h/int_guild.c/
		ファイル名変数の宣言追加
	INTER鯖パケット.txt
		倉庫パケットの解説追加

	(map/)
	storage.h/storage.c
		storage_fromstr(),storage_tostr()をchar/int_storage.cに移動。
		同じくdo_init,do_finalでのファイル処理も移動。
		do_final()は処理なし、do_init()は変数初期化のみに変更。
		storage_storageopen()では単にintif_request_storage()を呼ぶだけに。
		storage_storageclose()にintif_send_storage()を追加
		storage_storage_quitsave()追加。クライアント終了時に
		カプラ倉庫が開いていればintif_send_storage()を呼ぶ関数。
	intif.h/intif.c
		intif_parse_LoadStorage(),intif_parse_SaveStorage(),
		intif_send_storage(),intif_request_storage()追加
	map.c
		map_quit()でstorage_storage_quitsave()を呼ぶように。

	(conf/)
	inter_athena.cnf
		新規追加。inter鯖用のコンフィグレーションファイル


----------
//0221 by 胡蝶蘭

・スキルターゲットのIDが正しく所得できない問題修正
	clif.c
		clif_parse_UseSkillToId()でIDをWORDとして扱ってたのをLONGに修正

・スキル詠唱時間と属性表、および魔法系スキルの属性修正実装
	属性ダメージ修正は battle_attr_fix() で計算します。
	atk_elemは属性そのまま、def_elemは（属性lv*20＋属性）です。
	詠唱時間はskill.cのCASTFIXの値を変えることで倍率を調整できます

	pc.c
		pc_readdb()でcast_db.txtとattr_fix.txtの読み込み追加
	skill.c/skill.h
		struct skill_db にcast,delay追加、それらのアクセサも追加
		スキル詠唱時間を skill_get_cast() で所得するようにした
	battle.c/battle.h
		attr_fix_table定義
		battle_attr_fix()追加、属性修正を計算する
		属性系アクセサ(battle_get_element()など)を追加
		battle_calc_magic_damage()に属性修正を追加
	cast_db.txt
		新規追加。詠唱時間とディレイのデータベース
		全然足りないので、誰か追加希望。
	attr_fix.txt
		新規追加。属性修正テーブル

・ヒールの実装
	clif.c/clif.h
		clif_skill_nodamage()追加、支援系や回復のエフェクト
	skill.c/skill.h
		skill_castend_damage_id()、skill_castend_nodamage_id()追加、
		攻撃系と支援/回復系で関数を分けた
		ヒール計算マクロ skill_calc_heal() 追加
	battle.c
		battle_calc_magic_damage()でヒールのダメージ計算追加


----------
//0220 by れあ

0216の修正
HITの計算がおかしかったので修正してみました。
間違ってたらごめんなさい。

・battle.c
	256行目の
	hitrate=battle_get_hit(&sd->bl) - battle_get_flee(&sd->bl) + 80;
	がたぶん、自分のＨＩＴと自分のＦＬＥＥで計算してる気がするので
	hitrate=battle_get_hit(&sd->bl) - battle_get_flee(target) + 80;
	に修正しました。


----------
//0218 by 胡蝶蘭

実際に分散させてテストしていなかったり。

・map鯖分散処理用にinter鯖機能をつけてみる（今後のための拡張）
	char鯖にinter鯖を寄生させました。複数のmap鯖間の通信に利用します。
	map鯖を分散して処理できるようにするための機能です。
	今後partyやguild実装時にきっと役にたってくれるかと。
	
	倉庫の実装もinter鯖に移動すべきかもしれません。
	どのキャラクターがどのmap鯖にいるか検索する機能もいるかも。

	使うパケットのIDは以下のようになります
		map鯖=>inter鯖はパケット0x3000～
		inter鯖=>map鯖はパケット0x3800～
	パケットを作った場合は、INTER鯖パケット.txtに書いてください

	この機能によるメリット
		map鯖分散にも対応できる
	この機能によるデメリット
		inter鯖経由の全ての命令の動作速度が落ちる
		（一回inter鯖まで渡すため）
		鯖とクライアントを同じPCで使っているとつらいかも

	(char/)
	char2.c/char.h
		mapif_sendall()追加（全MAP鯖にパケットを送る）
		mapif_send()追加（特定MAP鯖に送る：生存判定付き）
		parse_frommap()でinter_parse_frommap()を呼ぶようにした
		(inter鯖のmap鯖解析部をchar鯖に寄生させたことになる)
	inter.h/inter.c
		新規追加。inter鯖の中核。
		inter_parse_frommapでMAP鯖からのパケットを解析します。
	int_party.h/int_party.c/int_guild.h/int_guild.c
		新規追加。今後のための予約。パーティやギルド機能用
		initでデータを読んで、saveで保存すべき？
		saveはまだ呼ばれない。parseでパケット解析。
		common/mmo.hあたりでパーティーやギルドの構造体を
		定義する必要があると思われる。
	INTER鯖パケット.txt
		パケットのリスト
	
	(map/)
	intif.h/intif.c
		inter鯖と通信する部分。
		inter_parse()でinter鯖からのパケットを解析します。
		inter鯖へデータを送るときはinter_fdを使います。
	chrif.h/chrif.c
		chrif_parse()でinter_parse()を呼ぶようにした
		（intif.cのinter鯖解析部をchar鯖解析部に寄生させたことになる)

・@kamiコマンドをinter鯖経由に変更
	原理としては次のような感じです
	クライアント＝＞map鯖＝＞inter鯖＝＞全map鯖＝＞全クライアント
	
	(char/)
	inter.c
		mapif_GMmessage()追加
	(map/)
	intif.h/intif.c
		intif_GMmessage()追加
		intif_parseでGMメッセージの処理を追加
	clif.c/clif.h
		clif_GMmessage()の引数を変更
	atcommand.c
		@kami部分でintif_GMmessage()を呼ぶようにした

・Wisをinter鯖経由に変更
	原理としては次のような感じです
	
	送り主クライアント＝＞送り主map鯖＝＞inter鯖＝＞全マップ鯖＝＞(分岐A)
	[分岐A]
	1.相手の人いるmap鯖＝＞相手のクライアント
	　　　　〃　　   　＝＞inter鯖＝＞送り主map鯖＝＞送り主クライアント 
	2.相手のいないmap鯖＝＞inter鯖（分岐B）
	[分岐B]
	1.全map鯖が応答したinter鯖 ＝＞送り主map鯖＝＞送り主クライアント
	2.(全部は応答してないときは、全map鯖の応答を待つ)

	ものすごい複雑になってますね。

	(char/)
	inter.c
		struct WisList 定義（Wisデータのリンクリスト）
		add_wislist(),del_wislist(),search_wislist(),
		check_ttl_wislist()追加,リンクリストを扱う関数群
		mapif_wis_message(),mapif_wis_end()追加
	(map/)
	intif.h/intif.c
		intif_wis_message(),intif_wis_end()追加
		intif_parse_WisMessage()追加,intif_parse()から呼ばれるように
	clif.c/clif.h
		clif_wis_message(),clif_wis_end()追加
		clif_parse_Wis()を変更,intif_wis_message()を呼ぶようにした

・スキル使用時のヒット数/消費SP所得のバグ修正
	skill.c
		skill_get_sp(),skill_get_num()で参照する配列インデックスをlv-1にした


----------
//0216 by 胡蝶蘭

いつもどおりテストほとんどしてないので、バグ大量かも。

・0213の修正？のよくわからないところ修正
	itemdb.c
		コンパイルが通らないのでitemdb_equipointの引数リスト変更

・Athena dev 2.1.1の適用
	dev-2.1.1で適用された修正を適用しました

	timer.c
		2.1.1のものと差し替え
	script.c
		C_NE: の修正の適用
	README
		最後の文章を2.1.1のものに差し替え

・スキルデータベースの修正
	一部の消費SPやヒット数などを修正。

	skill_db.txt
		該当個所の修正

・スキル攻撃の実装変更＆追加実装
	バッシュ、メマーナイト、ダブルストレイフィング、ピアース
	スピアブーメラン、スピアスタブ、ボーリングバッシュ
	ソニックブロー、グリムトゥース などの実装変更

	ナパームビート、ソウルストライク、
	ファイヤーボルト、コールドボルト、ライトニングボルト、アーススパイク、
	ユピテルサンダー などを追加実装
	（全て、範囲攻撃やステータス異常などは未実装）

	pc.c/pc.h
		0213の変更をなかったことにした
		pc_attack_mob()の修正、計算はbattle_calc_weapon_attack()に任せ、
		その計算結果を適用するだけに変更
	clif.c/clif.h
		clif_skill_fail(),clif_skill_damage(),clif_skill_damage2()追加
		それぞれ使用失敗、使用エフェクト、吹き飛ばし付き使用エフェクト
	skill.c/skill.h
		0213の変更をなかったことにした（ダメージ倍率計算がおかしい）
		skill_castend_id()にSP/Zeny確認と消費部分を追加、
		種類別に処理を追加。
	battle.c/battle.h
		新規追加
		武器攻撃計算用にbattle_calc_weapon_attack(),
		魔法攻撃計算用にbattle_calc_magic_attack()を用意
		（双方とも、MOBとPC両方計算可能なはず）
		ファイル増やしすぎという意見も…(汗)


----------
//0214 by れあ
・ダブルアタックがおかしかったところを修正。
・スキルの一部実装
	バッシュ・メマーナイト・ダブルストレイフィング・ピアース
	スピアブーメラン・スピアスタブ・ボーリングバッシュ
	ソニックブロー・グリムトゥースなどです。

	適当なのでどこか、不具合があるかもしれません。
	あと、テストもあまりしてませんのでおかしいところがあったら修正をお願いします。
	他にも問題があったら手直しをお願いします。
	変更内容は以下の通りです。

	clif.c,clif.h
		clif_skill_damage()を追加しました。

	pc.c,pc.h
		pc_attack_mob()の引数を一つ追加。
		ダブルアタックがおかしかったので正常に動作するように修正。

	skill.c
		一部スキルの実装をしてみました。


----------
//0213 by れあ
・0208の＠コマンドで少し修正
	atcommand.c
		@itemで個数指定が無い場合、入手個数を１個にするようにした。
		@itemでIDの指定が無い場合、アイテムを入手してたことになって
		いたのを修正
	itemdb.c
		item_db.txtでSellの項目を店売りの値段としてみた。
	item_db2.txt
		試しにカードやレアアイテムの店売り価格を値段を本鯖の相場にし
		てみたもの。使用する場合はitem_db.txtと差し替えてください。


----------
//0208 by nabe

・＠コマンド実装。
	atcommand.h,atcommand.c
		ほぼI-Athenaの＠コマンド相当ですが、@GMとPVPは未実装です。
		help.txtも同梱しています。
		GM（アカウントID＝704554～704583）専用にするには、
		atcommand.cの該当部分のコメントアウトを解除して下さい。
	clif.h,clif.c
		clif_displaymessage()
		clif_GMmessage()
		clif_heal()
		clif_resurrection()
		clif_pvpon()
		clif_pvpset()
		clif_refine()
		を追加しました。
		clif_parse_GlobalMessage()内でatcommand()を呼んでいます。

・ちょっとだけ修正。
	script.c
		{buildin_openstorage,"openstorage","s"},
		から
		{buildin_openstorage,"openstorage",""},
		に修正しました。


----------
//0206 by 胡蝶蘭
・スキルツリー/スキル使用機構の実装
	mmo.h
		MAX_SKILLを増やした
	char2.c
		mmo_char_fromstr()
		mmo_charstatusのskillのインデックスにスキル番号を使うようにした
		=>スキルの検索高速化のため（かわりにメモリ使用量が増える）
	pc.h/pc.c
		pc_skillup(),pc_calc_skilltree()追加
		pc_checkskill()変更（インデックスをスキル番号に）
		pc_readdb()でskill_db.txtも読むようにした
		pc_authok()でcast_timerを初期化するようにした
		pc_calcstatus()でpc_calc_skilltree()とclif_skillinfoblock()を
		呼ぶようにした
	clif.c/clif.h
		clif_skillinfoblock(),clif_skillcasting(),
		clif_skillup()を追加
		clif_parse_SkillUp(),clif_parse_UseSkillToId(),
		clif_parse_UseSkillToPos()を実装
	skill.h/skill.c
		ファイル追加(map/)
	map.h
		struct map_session_dataにcast_*を追加
	skill_db.txt
		ファイル追加(db/)
		(I-Athena0200のskill_info2.txtをコンバートしたもの)
	(スキル使用部分開発者向け情報)
		スキルの効果を実装する場所はskill.cの
		skill_castend_id(),skill_castend_pos()です。
		ターゲットや使用スキルは sd->cast_* から得ます
		スキルデータベースへは skill_get_* でアクセスしてください
		今後、キャスティングタイムもデータベースに入れる予定

----------
//0205 by nabe

・storage.cのバグフィクス。
・倉庫データを、マップ鯖起動時に読み、マップ鯖終了時に書くように変更。
	storage.h,storage.c
		storage_init()をdo_init_storage()に改名。
		storage_save()をdo_final_storage()に改名。
		fcloseを忘れていたのを追加。
	map.c
		#include "storage.h"を追加。
		do_final()にdo_final_storage()を追加。
		do_init()にdo_init_storage()を追加。

----------

//0203(unofficial) by なみ 

item_db.txtの書き換えのみです。

・アイテムの回復量を追加/変更
	赤ポーション　　　　　　　　　HP  30- 44
	紅ポーション　　　　　　　　　HP  70- 89
	黄色いポーション　　　　　　　HP 175-234
	白いポーション　　　　　　　　HP 350-429
	青いポーション　　　　　　　　SP  40- 99
	赤いハーブ　　　　　　　　　　HP  12- 19
	黄色いハーブ　　　　　　　　　HP  21- 29
	白いハーブ　　　　　　　　　　HP  80-111
	青いハーブ　　　　　　　　　　SP  15- 44
	リンゴ　　　　　　　　　　　　HP  12- 15
	バナナ　　　　　　　　　　　　HP  11- 16
	ブドウ　　　　　　　　　　　　SP  10- 24
	いも　　　　　　　　　　　　　HP  11- 15
	にく　　　　　　　　　　　　　HP  70- 99
	ハチの蜜　　　　　　　　　　　HP  72- 97 / SP  20- 59
	ミルク　　　　　　　　　　　　HP  25- 34
	キャンディ　　　　　　　　　　HP  31- 74
	スティックキャンディ　　　　　HP  46-109
	リンゴジュース　　　　　　※　HP  28- 32
	バナナジュース　　　　　　　　HP  27- 33
	ブドウジュース　　　　　　　　SP  15- 39
	ニンジンジュース　　　　　※　HP  29- 32
	カボチャ　　　　　　　　　　　HP  14
	ペットフード　　　　　　　　　HP  53- 83
	よく焼いたクッキー　　　　　　HP  80-177
	ひとくちケーキー　　　　　　　HP 251-359
	ひなあられ　　　　　　　　　　HP 175-234
	菱餅　　　　　　　　　　　　　HP 350-429
	レッドスリムポーション　　※　HP  30- 44
	イエロスリムポーション　　※　HP 175-234
	ホワイトスリムポーション　※　HP 350-429
　現在のAthenaではVITやスキルによるボーナスは加味されません。
　（適用する場合はscript.c内のbuildin_heal関数あたりにに手を加える必要あり）
　なお、※付のアイテムのデータは適当です。
・古いカード帖を実装(UseScript)
・その他修正
　　ひなあられ　　　　　　　　　　重量なし→重量0.1に修正
　　菱餅　　　　　　　　　　　　　重量なし→重量0.1に修正
　　バルムン　　　　　　　　　　　重量0.1S4片手剣→重量100S0両手剣に修正
　なお、Sellの項目はあるだけ無駄っぽいので全部消しました。

----------

//0202 by nabe

・カプラ倉庫の「同一アカウントなのに共有できないバグ」を改良しました。
	各キャラに倉庫データを持たせるのは無駄が多い気がするので、
	アカウントIDで管理するように仕様を変更しました。
	ついでに、倉庫データは全てstorage.cでまかない、
	char鯖は関与しないようにしました。
	これに伴い、char_athena.cnf,mmo.h,char2.cは元に戻しました。
	また、倉庫ファイル名は“storage.txt”に固定しています。

	改変、追加したのは次のファイルです。
	map/storage.h,
	map/storage.c,
	map/clif.h,//引数変更だけ
	map/clif.c,//引数変更だけ
	conf/char_athena.cnf,//元に戻しただけ
	common/mmo.h,//元に戻しただけ
	char/char2.c,//元に戻しただけ
	map/itemdb.h,//itemdb_equippoint()引数宣言変更だけ
	map/itemdb.c,//itemdb_equippoint()引数宣言変更だけ
	map/pc.c,//itemdb_equippoint()引数宣言変更だけ

----------

//0201 by nabe

・カプラ倉庫を実装しました。

	スクリプトから呼び出すには、スクリプト内で
		openstorage;
	としてください。
	サンプルとしてnpc_kafraJ.txtを付けてあります。
	併せてnpc_script3J.txtの該当部分も改変しました。

	char_athena.cnfの
		stor_txt: 
	で倉庫ファイル名を指定しています。

	改変、追加したのは次のファイルです。
	map/Makefile,
	map/storage.c,
	map/storage.h,
	map/clif.c,
	map/clif.h,
	map/script.c,
	char/char2.c,
	common/mmo,h
	詳しくは、上記ファイルのコメントなどを参考にしてください。

・カプラ倉庫実装に伴い、map_athena1.cnfを少し書き換えました。

・全てのコメント文をEUCからSJISに変換しました。

----------

    Athena Dev. v2.1.1       Released: Middle July, 2003
    (c) 2003 Athena Project.
     http://project-yare.de/

1. Athena(アテナ)について
2. このリリースについて
3. 必要な物
4. 使い方
5. 現在の仕様
6. 祝辞
7. 免責事項
8. 募集
9. English


1. アテナについて
    アテナとは2003年1月半ばにでた0052.lzhをベースとして作られているエミュレータの一つです。
    基本的なライセンスはオリジナルがGPLの下に配布されている為、
    これに従いGPLの下配布を許可します。
    /*
        改良版を配布する場合は必ずこのREADMEを書き換えてください。
        何処を改良したのか報告(athena@project-yare.deまで)して貰えると助かります。
        バイナリのみの配布はGPL違反ですので"必ず"ソースも添付してください。
     */
    動作の確認は以下の通りのみ行っています。
    // ただし完璧に動く事を保証するものでありません
    対象CPU: Intel Pentium系   // PentiumII以上で確認.
        FreeBSD 4.8R, 4.6.2R
        Linux RedHat 7.3
        cygwin + gcc 3.2 20020927 (prerelease)
    開発元URL: http://project-yare.de/


2. このリリースについて
    今回のリリースは前回(V2.1)同様開発版のリリースのみです。
    2.1に比べ下記の点が修正されています。
        mapのデフォルト設定が韓国data.grfのみ正常に動作するようになっていた点
        common/timer.cやmap/script.cの幾つかのバグ

    迅速にUpdateを強く推奨するものではありませんが各自の判断で行って下さい。


3. 必要な物
    data.grf      //sdata.grfは必要に応じて
    account.txt   //存在しない場合athena.shが自動生成します
    conf/*.cnf    //Map用とChar用の二種類あります
    conf/npc*.txt //npc設定用ファイルです。複数のファイルに分けることが可能です。
    db/*.txt      //アイテム、job情報など


4. 使い方
    > tar xvfz athena-d?.?.tar.gz
    > cd athena-d?.?.tar.gz
    > make
    > vi conf/char_athena.cnf       //IP(127.0.0.1)の部分を環境に合わせて変更してください
    > vi conf/map_athena.cnf        //同上、またmap設定などは、このファイルで行います。
    > ./athena.sh
    上記を行えば"たぶん"起動します。

    補足:
    conf/npc_sampleJ.txtにはスクリプトの書き方について色々な説明が記載されています。
    もし、独自のMap設定を行ってみたい人や、スクリプトを弄りたい方は参考にしてください。
    ただし、開発中のためスクリプトの仕様が変更される可能性が高いです。
    command.txtには実装済みの特殊コマンドについての説明を記載しています。


5. 現在の仕様
    本鯖と比べておかしい(例えばプバが歩く、ポリンがアイテムを拾わないなど)点は、
    全て現在開発中に因るものです。
    現状としてキャラクタ系及びモンスター系のバグ報告は無視される可能性が高いです。

    バグ報告について必ず発生条件をお書き下さい。
    下にある報告用テンプレートを使って報告して頂くと助かります。
    報告先はエミュ板の開発スレにでも。
    ---- Athena v 2.0 (stable or develop) ----
    【gcc ver】gcc -vを実行時に表示される内容
    【動作システム】FreeBSD, Linux(ディストリビュージョンも), cygwinなど
    【発生内容】mapが落ちてしまった時の表示されていたデバッグ情報など具体的に書いてください。
    【操作内容】具体的にどんな操作を行ったかを書いてください。
    ------------------ END -------------------
    理想はテンプレに加えてmap.coreなどcoreファイルをUploaderにアップして頂くことですが
    問題のMapだけの状態にしcoreの吐く容量に注意してください。
    /*
        確認した限りでは324個ほどmapデータを読み込ませると、
         40MB近いcoreファイルを吐き出します @FreeBSD
         cygwinの場合はstackdumpというファイルになるそうです。
        しかし、coreファイルなどをgzip圧縮などすれば大幅に小さくなります。
         大凡30MBのcoreファイルが2.9MBほどになるようです。
        ですので、もしアップロードする場合はgzip圧縮など各自行ってください。
     */

    今回のリリースだけでなくHISTORYを作成すると大量に記述が必要な為省略しています。
    // 多い日だと本当に結構ありますので‥‥。


6. 祝辞
    今回このAthena開発版を出すに当たって感謝したい方々(順番不同)
        Lemming氏 (Project YARE)
        0052氏 (Uploader)
        35氏 (エミュ開発スレ)
        Johan Lindh氏(Author of memwatch)
        YARE forumのNPC情報を作成した方々
        weiss研究会BBSの様々な情報ファイルを作成した方々
        最後に、.coreファイル達


7. 免責事項
    Athena Projectは一切Athenaの動作に関する保証等は行いません。
    つまり、Athenaは無保証です。
    athena@project-yare.deに動作・操作等に関する質問などを送られても一切お答えできません。
    又Athenaを用いたことにより生じた被害・問題等の責任は一切Athena Projectは負いません。


8. 募集
    athenaの開発に参加したい//興味があるという方ご連絡下さい。
    我々は貴方の参加をお待ちしています。
    // 最新版が欲しいだけで何ら協力して頂けないという方はお断りです;-)
    [募集要項: プログラマ(2-3人)]
        年齢: 不問
        性別: 不問
        言語: 日本語が理解可能
        内容: C言語もしくはC++による開発。(特にネットワークやDBの経験が有る方大募集!)
    [募集要項: 翻訳(?人)]
        年齢: 不問
        性別: 不問
        言語: 日本語、英語が理解可能
        内容: 仏蘭西語、独逸語、西班牙語、伊太利亜語、泰(タイ)語、朝鮮語、中国語へ文献、サイトなどの翻訳
    連絡先: athena@project-yare.de 雑務担当まで。


9. English
     This release is just fixed some bugs in timer.c, script.c and map_athena1.conf.


(c) 2003 Athena Project.
