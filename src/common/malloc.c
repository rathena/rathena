// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/malloc.h"
#include "../common/core.h"
#include "../common/showmsg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// no logging for minicore
#if defined(MINICORE) && defined(LOG_MEMMGR)
#undef LOG_MEMMGR
#endif

void* aMalloc_(size_t size, const char *file, int line, const char *func)
{
	void *ret = MALLOC(size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aMalloc %d\n",file,line,func,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aMalloc error out of memory!\n",file,line,func);
		exit(EXIT_FAILURE);
	}

	return ret;
}
void* aMallocA_(size_t size, const char *file, int line, const char *func)
{
	void *ret = MALLOCA(size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aMallocA %d\n",file,line,func,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aMallocA error out of memory!\n",file,line,func);
		exit(EXIT_FAILURE);
	}

	return ret;
}
void* aCalloc_(size_t num, size_t size, const char *file, int line, const char *func)
{
	void *ret = CALLOC(num, size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aCalloc %d %d\n",file,line,func,num,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aCalloc error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
void* aCallocA_(size_t num, size_t size, const char *file, int line, const char *func)
{
	void *ret = CALLOCA(num, size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aCallocA %d %d\n",file,line,func,num,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aCallocA error out of memory!\n",file,line,func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
void* aRealloc_(void *p, size_t size, const char *file, int line, const char *func)
{
	void *ret = REALLOC(p, size, file, line, func);
	// ShowMessage("%s:%d: in func %s: aRealloc %p %d\n",file,line,func,p,size);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aRealloc error out of memory!\n",file,line,func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
char* aStrdup_(const char *p, const char *file, int line, const char *func)
{
	char *ret = STRDUP(p, file, line, func);
	// ShowMessage("%s:%d: in func %s: aStrdup %p\n",file,line,func,p);
	if (ret == NULL){
		ShowFatalError("%s:%d: in func %s: aStrdup error out of memory!\n", file, line, func);
		exit(EXIT_FAILURE);
	}
	return ret;
}
void aFree_(void *p, const char *file, int line, const char *func)
{
	// ShowMessage("%s:%d: in func %s: aFree %p\n",file,line,func,p);
	if (p)
		FREE(p, file, line, func);

	p = NULL;
}

#ifdef GCOLLECT

void* _bcallocA(size_t size, size_t cnt)
{
	void *ret = MALLOCA(size * cnt);
	if (ret) memset(ret, 0, size * cnt);
	return ret;
}
void* _bcalloc(size_t size, size_t cnt)
{
	void *ret = MALLOC(size * cnt);
	if (ret) memset(ret, 0, size * cnt);
	return ret;
}
char* _bstrdup(const char *chr)
{
	int len = strlen(chr);
	char *ret = (char*)MALLOC(len + 1);
	if (ret) memcpy(ret, chr, len + 1);
	return ret;
}

#endif

#ifdef USE_MEMMGR

#define DEBUG_MEMMGR

/* USE_MEMMGR */

/*
 * メモリマネージャ
 *     malloc , free の処理を効率的に出来るようにしたもの。
 *     複雑な処理を行っているので、若干重くなるかもしれません。
 *
 * データ構造など（説明下手ですいません^^; ）
 *     ・メモリを複数の「ブロック」に分けて、さらにブロックを複数の「ユニット」
 *       に分けています。ユニットのサイズは、１ブロックの容量を複数個に均等配分
 *       したものです。たとえば、１ユニット32KBの場合、ブロック１つは32Byteのユ
 *       ニットが、1024個集まって出来ていたり、64Byteのユニットが 512個集まって
 *       出来ていたりします。（padding,unit_head を除く）
 *
 *     ・ブロック同士はリンクリスト(block_prev,block_next) でつながり、同じサイ
 *       ズを持つブロック同士もリンクリスト(samesize_prev,samesize_nect) でつな
 *       がっています。それにより、不要となったメモリの再利用が効率的に行えます。
 */

/* ブロックに入るデータ量 */
#define BLOCK_DATA_SIZE	80*1024

/* 一度に確保するブロックの数。 */
#define BLOCK_ALLOC		32

/* ブロックのアライメント */
#define BLOCK_ALIGNMENT	64

/* ブロック */
struct block {
	int    block_no;		/* ブロック番号 */
	struct block* block_prev;		/* 前に確保した領域 */
	struct block* block_next;		/* 次に確保した領域 */
	int    samesize_no;     /* 同じサイズの番号 */
	struct block* samesize_prev;	/* 同じサイズの前の領域 */
	struct block* samesize_next;	/* 同じサイズの次の領域 */
	size_t unit_size;		/* ユニットのバイト数 0=未使用 */
	size_t unit_hash;		/* ユニットのハッシュ */
	int    unit_count;		/* ユニットの数 */
	int    unit_used;		/* 使用済みユニット */
	char   data[BLOCK_DATA_SIZE];
};

struct unit_head {
	struct block* block;
	size_t size;
	const  char* file;
	int    line;
	unsigned int checksum;
};

struct chunk {
	char *block;
	struct chunk *next;
};

static struct block* block_first  = NULL;
static struct block* block_last   = NULL;
static struct block* block_unused = NULL;

/* ユニットへのハッシュ。80KB/64Byte = 1280個 */
static struct block* unit_first[BLOCK_DATA_SIZE/BLOCK_ALIGNMENT];		/* 最初 */
static struct block* unit_unfill[BLOCK_DATA_SIZE/BLOCK_ALIGNMENT];	/* 埋まってない */
static struct block* unit_last[BLOCK_DATA_SIZE/BLOCK_ALIGNMENT];		/* 最後 */

/* メモリを使い回せない領域用のデータ */
struct unit_head_large {
	struct unit_head_large* prev;
	struct unit_head_large* next;
	struct unit_head        unit_head;
};
static struct unit_head_large *unit_head_large_first = NULL;

static struct chunk *chunk_first = NULL;

static struct block* block_malloc(void);
static void   block_free(struct block* p);
static void memmgr_info(void);
static unsigned int memmgr_usage_bytes = 0;

void* _mmalloc(size_t size, const char *file, int line, const char *func )
{
	int i;
	struct block *block;
	size_t size_hash;

	if (((long) size) < 0) {
		printf("_mmalloc: %d\n", size);
		return 0;
	}
	
	size_hash = (size+BLOCK_ALIGNMENT-1) / BLOCK_ALIGNMENT;
	if(size == 0) {
		return NULL;
	}
	memmgr_usage_bytes += size;

	/* ブロック長を超える領域の確保には、malloc() を用いる */
	/* その際、unit_head.block に NULL を代入して区別する */
	if(size_hash * BLOCK_ALIGNMENT > BLOCK_DATA_SIZE - sizeof(struct unit_head)) {
		struct unit_head_large* p = (struct unit_head_large*)MALLOC(sizeof(struct unit_head_large)+size,file,line,func);
		if(p != NULL) {
			p->unit_head.block = NULL;
			p->unit_head.size  = size;
			p->unit_head.file  = file;
			p->unit_head.line  = line;
			p->prev = NULL;
			if (unit_head_large_first == NULL)
				p->next = NULL;
			else {
				unit_head_large_first->prev = p;
				p->next = unit_head_large_first;
			}
			unit_head_large_first = p;
#ifdef DEBUG_MEMMGR
			// set allocated data to 0xCD (clean memory)
			memset(p + sizeof(struct unit_head_large) - sizeof(int), 0xCD, size);
#endif
			*(int*)((char*)p + sizeof(struct unit_head_large) - sizeof(int) + size) = 0xdeadbeaf;
			return (char *)p + sizeof(struct unit_head_large) - sizeof(int);
		} else {
			ShowFatalError("Memory manager::memmgr_alloc failed (allocating %d+%d bytes at %s:%d).\n", sizeof(struct unit_head_large), size, file, line);
			exit(EXIT_FAILURE);
		}
	}

	/* 同一サイズのブロックが確保されていない時、新たに確保する */
	if(unit_unfill[size_hash] == NULL) {
		block = block_malloc();
		if(unit_first[size_hash] == NULL) {
			/* 初回確保 */
			unit_first[size_hash] = block;
			unit_last[size_hash] = block;
			block->samesize_no = 0;
			block->samesize_prev = NULL;
			block->samesize_next = NULL;
		} else {
			/* 連結作業 */
			unit_last[size_hash]->samesize_next = block;
			block->samesize_no   = unit_last[size_hash]->samesize_no + 1;
			block->samesize_prev = unit_last[size_hash];
			block->samesize_next = NULL;
			unit_last[size_hash] = block;
		}
		unit_unfill[size_hash] = block;
		block->unit_size  = size_hash * BLOCK_ALIGNMENT + sizeof(struct unit_head);
		block->unit_count = (int)(BLOCK_DATA_SIZE / block->unit_size);
		block->unit_used  = 0;
		block->unit_hash  = size_hash;
		/* 未使用Flagを立てる */
		for(i=0;i<block->unit_count;i++) {
			((struct unit_head*)(&block->data[block->unit_size * i]))->block = NULL;
		}
	}
	/* ユニット使用個数加算 */
	block = unit_unfill[size_hash];
	block->unit_used++;

	/* ユニット内を全て使い果たした */
	if(block->unit_count == block->unit_used) {
		do {
			unit_unfill[size_hash] = unit_unfill[size_hash]->samesize_next;
		} while(
			unit_unfill[size_hash] != NULL &&
			unit_unfill[size_hash]->unit_count == unit_unfill[size_hash]->unit_used
		);
	}

	/* ブロックの中の空きユニット捜索 */
	for(i=0;i<block->unit_count;i++) {
		struct unit_head *head = (struct unit_head*)(&block->data[block->unit_size * i]);
		if(head->block == NULL) {
			head->block = block;
			head->size  = size;
			head->line  = line;
			head->file  = file;
#ifdef DEBUG_MEMMGR
			// set allocated memory to 0xCD (clean memory)
			memset(head + sizeof(struct unit_head) - sizeof(int), 0xCD, size);
#endif
			*(int*)((char*)head + sizeof(struct unit_head) - sizeof(int) + size) = 0xdeadbeaf;
			return (char *)head + sizeof(struct unit_head) - sizeof(int);
		}
	}
	// ここに来てはいけない。
	ShowFatalError("Memory manager::memmgr_malloc() serious error (allocating %d+%d bytes at %s:%d)\n", sizeof(struct unit_head_large), size, file, line);
	memmgr_info();
	exit(EXIT_FAILURE);
	//return NULL;
};

void* _mcalloc(size_t num, size_t size, const char *file, int line, const char *func )
{
	void *p = _mmalloc(num * size,file,line,func);
	memset(p,0,num * size);
	return p;
}

void* _mrealloc(void *memblock, size_t size, const char *file, int line, const char *func )
{
	size_t old_size;
	if(memblock == NULL) {
		return _mmalloc(size,file,line,func);
	}

	old_size = ((struct unit_head *)((char *)memblock - sizeof(struct unit_head) + sizeof(int)))->size;
	if(old_size > size) {
		// サイズ縮小 -> そのまま返す（手抜き）
		return memblock;
	}  else {
		// サイズ拡大
		void *p = _mmalloc(size,file,line,func);
		if(p != NULL) {
			memcpy(p,memblock,old_size);
		}
		_mfree(memblock,file,line,func);
		return p;
	}
}

char* _mstrdup(const char *p, const char *file, int line, const char *func )
{
	if(p == NULL) {
		return NULL;
	} else {
		size_t len = strlen(p);
		char *string  = (char *)_mmalloc(len + 1,file,line,func);
		memcpy(string,p,len+1);
		return string;
	}
}

void _mfree(void *ptr, const char *file, int line, const char *func )
{
	struct unit_head *head;
	size_t size_hash;

	if (ptr == NULL)
		return; 

	head = (struct unit_head *)((char *)ptr - sizeof(struct unit_head) + sizeof(int));
	size_hash = (head->size+BLOCK_ALIGNMENT-1) / BLOCK_ALIGNMENT;

	if(head->block == NULL) {
		if(size_hash * BLOCK_ALIGNMENT > BLOCK_DATA_SIZE - sizeof(struct unit_head)) {
			/* malloc() で直に確保された領域 */
			struct unit_head_large *head_large = (struct unit_head_large *)((char *)ptr - sizeof(struct unit_head_large) + sizeof(int));
			if(
				*(int*)((char*)head_large + sizeof(struct unit_head_large) - sizeof(int) + head->size)
				!= 0xdeadbeaf)
			{
				ShowError("Memory manager: args of aFree is overflowed pointer %s line %d\n", file, line);
			}
			if(head_large->prev) {
				head_large->prev->next = head_large->next;
			} else {
				unit_head_large_first  = head_large->next;
			}
			if(head_large->next) {
				head_large->next->prev = head_large->prev;
			}
			head->block = NULL;
			memmgr_usage_bytes -= head->size;
#ifdef DEBUG_MEMMGR
			// set freed memory to 0xDD (dead memory)
			memset(ptr, 0xDD, size_hash - sizeof(struct unit_head_large) + sizeof(int) );
#endif
			FREE(head_large,file,line,func);
		} else {
			ShowError("Memory manager: args of aFree is freed pointer %s:%d@%s\n", file, line, func);
		}
		ptr = NULL;
		return;
	} else {
		/* ユニット解放 */
		struct block *block = head->block;
		if((unsigned long)block % sizeof(struct block) != 0) {
			ShowError("Memory manager: args of aFree is not valid pointer %s line %d\n", file, line);
		} else if(*(int*)((char*)head + sizeof(struct unit_head) - sizeof(int) + head->size) != 0xdeadbeaf) {
			ShowError("Memory manager: args of aFree is overflowed pointer %s line %d\n", file, line);
		} else {
			head->block = NULL;
#ifdef DEBUG_MEMMGR
			// set freed memory to 0xDD (dead memory)
			memset(ptr, 0xDD, head->size - sizeof(struct unit_head) + sizeof(int) );
#endif
			memmgr_usage_bytes -= head->size;
			if(--block->unit_used == 0) {
				/* ブロックの解放 */
				if(unit_unfill[block->unit_hash] == block) {
					/* 空きユニットに指定されている */
					do {
						unit_unfill[block->unit_hash] = unit_unfill[block->unit_hash]->samesize_next;
					} while(
						unit_unfill[block->unit_hash] != NULL &&
						unit_unfill[block->unit_hash]->unit_count == unit_unfill[block->unit_hash]->unit_used
					);
				}
				if(block->samesize_prev == NULL && block->samesize_next == NULL) {
					/* 独立ブロックの解放 */
					unit_first[block->unit_hash]  = NULL;
					unit_last[block->unit_hash]   = NULL;
					unit_unfill[block->unit_hash] = NULL;
				} else if(block->samesize_prev == NULL) {
					/* 先頭ブロックの解放 */
					unit_first[block->unit_hash] = block->samesize_next;
					(block->samesize_next)->samesize_prev = NULL;
				} else if(block->samesize_next == NULL) {
					/* 末端ブロックの解放 */
					unit_last[block->unit_hash] = block->samesize_prev; 
					(block->samesize_prev)->samesize_next = NULL;
				} else {
					/* 中間ブロックの解放 */
					(block->samesize_next)->samesize_prev = block->samesize_prev;
					(block->samesize_prev)->samesize_next = block->samesize_next;
				}
				block_free(block);
			} else {
				/* 空きユニットの再設定 */
				if(
					unit_unfill[block->unit_hash] == NULL ||
					unit_unfill[block->unit_hash]->samesize_no > block->samesize_no
				) {
					unit_unfill[block->unit_hash] = block;
				}
			}
			ptr = NULL;
		}
	}
}

/* 現在の状況を表示する */
static void memmgr_info(void)
{
	int i;
	struct block *p;
	ShowInfo("** Memory Manager Information **\n");
	if(block_first == NULL) {
		ShowMessage("Uninitialized.\n");
		return;
	}
	ShowMessage(
		"Blocks: %04u , BlockSize: %06u Byte , Used: %08uKB\n",
		block_last->block_no+1,sizeof(struct block),
		(block_last->block_no+1) * sizeof(struct block) / 1024
	);
	p = block_first;
	for(i=0;i<=block_last->block_no;i++) {
		ShowMessage("    Block #%04u : ",p->block_no);
		if(p->unit_size == 0) {
			ShowMessage("unused.\n");
		} else {
			ShowMessage(
				"size: %05u byte. used: %04u/%04u prev:",
				p->unit_size - sizeof(struct unit_head),p->unit_used,p->unit_count
			);
			if(p->samesize_prev == NULL) {
				ShowMessage("NULL");
			} else {
				ShowMessage("%04u",(p->samesize_prev)->block_no);
			}
			ShowMessage(" next:");
			if(p->samesize_next == NULL) {
				ShowMessage("NULL");
			} else {
				ShowMessage("%04u",(p->samesize_next)->block_no);
			}
			ShowMessage("\n");
		}
		p = p->block_next;
	}
}

/* ブロックを確保する */
static struct block* block_malloc(void)
{
	if(block_unused != NULL) {
		/* ブロック用の領域は確保済み */
		struct block* ret = block_unused;
		do {
			block_unused = block_unused->block_next;
		} while(block_unused != NULL && block_unused->unit_size != 0);
		return ret;
	} else {
		/* ブロック用の領域を新たに確保する */
		int i;
		int block_no;
		struct block* p;
		struct chunk* chunk;
		char *pb = (char *)CALLOC(sizeof(struct block),BLOCK_ALLOC+1,file,line,func);
		if(pb == NULL) {
			ShowFatalError("Memory manager::block_alloc failed.\n");
			exit(EXIT_FAILURE);
		}

		// store original block address in chunk
		chunk = (struct chunk *)MALLOC(sizeof(struct chunk),file,line,func);
		if (chunk == NULL) {
			ShowFatalError("Memory manager::block_alloc failed.\n");
			exit(EXIT_FAILURE);
		}
		chunk->block = pb;
		chunk->next = (chunk_first) ? chunk_first : NULL;
		chunk_first = chunk;

		// ブロックのポインタの先頭をsizeof(block) アライメントに揃える
		// このアドレスをfree() することはないので、直接ポインタを変更している。
		pb += sizeof(struct block) - ((unsigned long)pb % sizeof(struct block));
		p   = (struct block*)pb;
		if(block_first == NULL) {
			/* 初回確保 */
			block_no     = 0;
			block_first  = p;
		} else {
			block_no      = block_last->block_no + 1;
			block_last->block_next = p;
			p->block_prev = block_last;
		}
		block_last = &p[BLOCK_ALLOC - 1];
		/* ブロックを連結させる */
		for(i=0;i<BLOCK_ALLOC;i++) {
			if(i != 0) {
				p[i].block_prev = &p[i-1];
			}
			if(i != BLOCK_ALLOC -1) {
				p[i].block_next = &p[i+1];
			}
			p[i].block_no = block_no + i;
		}

		/* 未使用ブロックへのポインタを更新 */
		block_unused = &p[1];
		p->unit_size = 1;
		return p;
	}
}

static void block_free(struct block* p)
{
	/* free() せずに、未使用フラグを付けるだけ */
	p->unit_size = 0;
	/* 未使用ポインターを更新する */
	if(block_unused == NULL) {
		block_unused = p;
	} else if(block_unused->block_no > p->block_no) {
		block_unused = p;
	}
}

unsigned int memmgr_usage (void)
{
	return memmgr_usage_bytes / 1024;
}

#ifdef LOG_MEMMGR
static char memmer_logfile[128];
static FILE *log_fp;

static void memmgr_log (char *buf)
{
	time_t raw;
	struct tm* t;
	if( !log_fp )
	{
		log_fp = fopen(memmer_logfile,"w");
		if (!log_fp) log_fp = stdout;
		fprintf(log_fp, "Memory manager: Memory leaks found (Revision %s).\n", get_svn_revision());
	}
	time(&raw);
	t = localtime(&raw);
	fprintf(log_fp, "%04d%02d%02d%02d%02d%02d %s", (t->tm_year+1900), (t->tm_mon+1), t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, buf);
	return;
}
#endif

static void memmgr_final (void)
{
	struct block *block = block_first;
	struct chunk *chunk = chunk_first, *chunk2;
	struct unit_head_large *large = unit_head_large_first, *large2;
	int i;

#ifdef LOG_MEMMGR
	int count = 0;
	char buf[128];
#endif
	
	while (block) {
		if (block->unit_size) {
			for (i = 0; i < block->unit_count; i++) {
				struct unit_head *head = (struct unit_head*)(&block->data[block->unit_size * i]);
				if(head->block != NULL)
				{
				#ifdef LOG_MEMMGR
					sprintf (buf,
						"%04d : %s line %d size %lu\n", ++count,
						head->file, head->line, (unsigned long)head->size);
					memmgr_log (buf);
				#endif
					// get block pointer and free it [celest]
					_mfree ((char *)head + sizeof(struct unit_head) - sizeof(int), ALC_MARK);
				}
			}
		}
		//if (block->block_no >= block2->block_no + BLOCK_ALLOC - 1) {
			// reached a new block array
			//block = block->block_next;

		/* Okay wise guys... this is how block2 was allocated: [Skotlex]
		struct block* p;
		char *pb = (char *) CALLOC (sizeof(struct block),BLOCK_ALLOC + 1);
		pb += sizeof(struct block) - ((unsigned long)pb % sizeof(struct block));
		p   = (struct block*)pb;
		
		The reason we get an invalid pointer is that we allocated pb, not p.
		So how do you get pb when you only have p? 
		The answer is, you can't, because the original pointer was lost when
		memory-aligning the block. So we either forget this FREE or use a
		self-reference...
		Since we are already quitting, it might be ok to just not free the block
		as it is. 
		 */
		// didn't realise that before o.o -- block chunks are now freed below [celest]
		//	FREE(block2);
			//block2 = block;
			//continue;
		//}
		block = block->block_next;
	}

	// free the allocated block chunks
	chunk = chunk_first;
	while (chunk) {
		chunk2 = chunk->next;
		FREE(chunk->block,file,line,func);
		FREE(chunk,file,line,func);
		chunk = chunk2;
	}

	while(large) {
		large2 = large->next;
	#ifdef LOG_MEMMGR
		sprintf (buf,
			"%04d : %s line %d size %lu\n", ++count,
			large->unit_head.file, large->unit_head.line, (unsigned long)large->unit_head.size);
		memmgr_log (buf);
	#endif
		FREE(large,file,line,func);
		large = large2;
	}
#ifdef LOG_MEMMGR
	if(count == 0) {
		ShowInfo("Memory manager: No memory leaks found.\n");
	} else {
		ShowWarning("Memory manager: Memory leaks found and fixed.\n");
		fclose(log_fp);
	}
#endif
	return;
}

static void memmgr_init (void)
{
	#ifdef LOG_MEMMGR
		sprintf(memmer_logfile, "log/%s.leaks", SERVER_NAME);
		ShowStatus("Memory manager initialised: "CL_WHITE"%s"CL_RESET"\n", memmer_logfile);
	#endif
	return;
}
#endif


/*======================================
 * Initialise
 *--------------------------------------
 */

unsigned int malloc_usage (void)
{
#ifdef USE_MEMMGR
	return memmgr_usage ();
#else
	return 0;
#endif
}

void malloc_final (void)
{
#ifdef USE_MEMMGR
	memmgr_final ();
#endif
	return;
}

void malloc_init (void)
{
#ifdef USE_MEMMGR
	memmgr_init ();
#endif
	return;
}
