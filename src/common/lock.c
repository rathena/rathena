
#include <stdio.h>
#include "lock.h"

// 書き込みファイルの保護処理
// （書き込みが終わるまで、旧ファイルを保管しておく）

// 新しいファイルの書き込み開始
FILE* lock_fopen(const char* filename,int *info) {
	char newfile[512];
	FILE *fp;
	int  no = 0;

	// 安全なファイル名を得る（手抜き）
	do {
		sprintf(newfile,"%s_%04d.tmp",filename,++no);
	} while((fp = fopen(newfile,"r")) && (fclose(fp), no<9999) );
	*info = no;
	return fopen(newfile,"w");
}

// 旧ファイルを削除＆新ファイルをリネーム
int lock_fclose(FILE *fp,const char* filename,int *info) {
	int  ret = 0;
	char newfile[512];
	if(fp != NULL) {
		ret = fclose(fp);
		sprintf(newfile,"%s_%04d.tmp",filename,*info);
		remove(filename);
		// このタイミングで落ちると最悪。
		rename(newfile,filename);
		return ret;
	} else {
		return 1;
	}
}

