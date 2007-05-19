// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "nullpo.h"
#include "../common/showmsg.h"
// #include "logs.h" // 布石してみる

static void nullpo_info_core(const char *file, int line, const char *func, 
                             const char *fmt, va_list ap);

/*======================================
 * Nullチェック 及び 情報出力
 *--------------------------------------*/
int nullpo_chk_f(const char *file, int line, const char *func, const void *target,
                 const char *fmt, ...)
{
	va_list ap;
	
	if (target != NULL)
		return 0;
	
	va_start(ap, fmt);
	nullpo_info_core(file, line, func, fmt, ap);
	va_end(ap);
	return 1;
}

int nullpo_chk(const char *file, int line, const char *func, const void *target)
{
	if (target != NULL)
		return 0;
	
	nullpo_info_core(file, line, func, NULL, NULL);
	return 1;
}


/*======================================
 * nullpo情報出力(外部呼出し向けラッパ)
 *--------------------------------------*/
void nullpo_info_f(const char *file, int line, const char *func, 
                 const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	nullpo_info_core(file, line, func, fmt, ap);
	va_end(ap);
}

void nullpo_info(const char *file, int line, const char *func)
{
	nullpo_info_core(file, line, func, NULL, NULL);
}


/*======================================
 * nullpo情報出力(Main)
 *--------------------------------------*/
static void nullpo_info_core(const char *file, int line, const char *func, 
                             const char *fmt, va_list ap)
{
	if (file == NULL)
		file = "??";
	
	func =
		func == NULL    ? "unknown":
		func[0] == '\0' ? "unknown":
		                  func;
	
	ShowMessage("--- nullpo info --------------------------------------------\n");
	ShowMessage("%s:%d: in func `%s'\n", file, line, func);
	if (fmt != NULL)
	{
		if (fmt[0] != '\0')
		{
			vprintf(fmt, ap);
			
			// 最後に改行したか確認
			if (fmt[strlen(fmt)-1] != '\n')
				ShowMessage("\n");
		}
	}
	ShowMessage("--- end nullpo info ----------------------------------------\n");
	
	// ここらでnullpoログをファイルに書き出せたら
	// まとめて提出できるなと思っていたり。
}
