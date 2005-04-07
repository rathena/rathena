#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malloc.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

// 独自メモリマネージャを使用する場合、次のコメントを外してください。
//#define USE_MEMMGR

#if !defined(DMALLOC) && !defined(GCOLLECT) && !defined(BCHECK) && !defined(USE_MEMMGR)

void* aMalloc_( size_t size, const char *file, int line, const char *func )
{
	void *ret;

//	printf("%s:%d: in func %s: malloc %d\n",file,line,func,size);
#ifdef MEMWATCH
	ret=mwMalloc(size,file,line);
#else
	ret=malloc(size);
#endif
	if(ret==NULL){
		printf("%s:%d: in func %s: malloc error out of memory!\n",file,line,func);
		exit(1);

	}
	return ret;
}
void* aCalloc_( size_t num, size_t size, const char *file, int line, const char *func )
{
	void *ret;

//	printf("%s:%d: in func %s: calloc %d %d\n",file,line,func,num,size);
#ifdef MEMWATCH
	ret=mwCalloc(num,size,file,line);
#else
	ret=calloc(num,size);
#endif
	if(ret==NULL){
		printf("%s:%d: in func %s: calloc error out of memory!\n",file,line,func);
		exit(1);

	}
	return ret;
}

void* aRealloc_( void *p, size_t size, const char *file, int line, const char *func )
{
	void *ret;

//	printf("%s:%d: in func %s: realloc %p %d\n",file,line,func,p,size);
#ifdef MEMWATCH
	ret=mwRealloc(p,size,file,line);
#else
	ret=realloc(p,size);
#endif
	if(ret==NULL){
		printf("%s:%d: in func %s: realloc error out of memory!\n",file,line,func);
		exit(1);

	}
	return ret;
}

char* aStrdup_( const void *p, const char *file, int line, const char *func )
{
	char *ret;
	
	// printf("%s:%d: in func %s: strdup %p\n",file,line,func,p);
#ifdef MEMWATCH
	ret=mwStrdup(p,file,line);
#else
	ret= strdup((char *) p);
#endif
	if(ret==NULL){
		printf("%s:%d: in func %s: strdup error out of memory!\n",file,line,func);
		exit(1);

	}
	return ret;
}

void aFree_( void *p, const char *file, int line, const char *func )
{
	// printf("%s:%d: in func %s: free %p\n",file,line,func,p);
#ifdef MEMWATCH
	mwFree(p,file,line);
#else
	free(p);
#endif
}

#elif defined(GCOLLECT)

void * _bcallocA(size_t size, size_t cnt) {
	void *ret = aMallocA(size * cnt);
	memset(ret, 0, size * cnt);
	return ret;
}

void * _bcalloc(size_t size, size_t cnt) {
	void *ret = aMalloc(size * cnt);
	memset(ret, 0, size * cnt);
	return ret;
}

char * _bstrdup(const char *chr) {
	int len = strlen(chr);
	char *ret = (char*)aMalloc(len + 1);
	strcpy(ret, chr);
	return ret;
}

#elif defined(USE_MEMMGR)

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
 *     ・ユニット同士はリンクリスト(block_prev,block_next) でつながり、同じサイ
 *       ズを持つユニット同士もリンクリスト(samesize_prev,samesize_nect) でつな
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
	int    unit_size;		/* ユニットのバイト数 0=未使用 */
	int    unit_hash;		/* ユニットのハッシュ */
	int    unit_count;		/* ユニットの数 */
	int    unit_used;		/* 使用済みユニット */
	char   data[BLOCK_DATA_SIZE];
};

struct unit_head {
	struct block* block;
	int    size;
	const  char* file;
	int    line;
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

static struct block* block_malloc(void);
static void   block_free(struct block* p);
static void memmgr_info(void);

void* aMalloc_(size_t size, const char *file, int line, const char *func ) {
	int i;
	struct block *block;
	int size_hash = (size+BLOCK_ALIGNMENT-1) / BLOCK_ALIGNMENT;
	size = size_hash * BLOCK_ALIGNMENT; /* アライメントの倍数に切り上げ */

	if(size == 0) {
		return NULL;
	}

	/* ブロック長を超える領域の確保には、malloc() を用いる */
	/* その際、unit_head.block に NULL を代入して区別する */
	if(size > BLOCK_DATA_SIZE - sizeof(struct unit_head)) {
#ifdef MEMWATCH
		struct unit_head_large* p = (struct unit_head_large*)mwMalloc(sizeof(struct unit_head_large) + size,file,line);
#else
		struct unit_head_large* p = (struct unit_head_large*)malloc(sizeof(struct unit_head_large) + size);
#endif
		if(p != NULL) {
			p->unit_head.block = NULL;
			p->unit_head.size  = size;
			p->unit_head.file  = file;
			p->unit_head.line  = line;
			if(unit_head_large_first == NULL) {
				unit_head_large_first = p;
				p->next = NULL;
				p->prev = NULL;
			} else {
				unit_head_large_first->prev = p;
				p->prev = NULL;
				p->next = unit_head_large_first;
				unit_head_large_first = p;
			}
			return (char *)p + sizeof(struct unit_head_large);
		} else {
			printf("MEMMGR::memmgr_alloc failed.\n");
			exit(1);
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
		block->unit_size  = size + sizeof(struct unit_head);
		block->unit_count = BLOCK_DATA_SIZE / block->unit_size;
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
			return (char *)head + sizeof(struct unit_head);
		}
	}
	// ここに来てはいけない。
	printf("MEMMGR::memmgr_malloc() serious error.\n");
	memmgr_info();
	exit(1);
	return NULL;
};

void* aCalloc_(size_t num, size_t size, const char *file, int line, const char *func ) {
	void *p = aMalloc_(num * size,file,line,func);
	memset(p,0,num * size);
	return p;
}

void* aRealloc_(void *memblock, size_t size, const char *file, int line, const char *func ) {
	size_t old_size;
	if(memblock == NULL) {
		return aMalloc_(size,file,line,func);
	}

	old_size = ((struct unit_head *)((char *)memblock - sizeof(struct unit_head)))->size;
	if(old_size > size) {
		// サイズ縮小 -> そのまま返す（手抜き）
		return memblock;
	}  else {
		// サイズ拡大
		void *p = aMalloc_(size,file,line,func);
		if(p != NULL) {
			memcpy(p,memblock,old_size);
		}
		aFree_(memblock,file,line,func);
		return p;
	}
}

char* aStrdup_(const void *p, const char *file, int line, const char *func ) {
	if(p == NULL) {
		return NULL;
	} else {
		int  len = strlen(p);
		char *string  = (char *)aMalloc_(len + 1,file,line,func);
		memcpy(string,p,len+1);
		return string;
	}
}

void aFree_(void *ptr, const char *file, int line, const char *func ) {
	struct unit_head *head = (struct unit_head *)((char *)ptr - sizeof(struct unit_head));
	if(ptr == NULL) {
		return;
	} else if(head->block == NULL && head->size > BLOCK_DATA_SIZE - sizeof(struct unit_head)) {
		/* malloc() で直に確保された領域 */
		struct unit_head_large *head_large = (struct unit_head_large *)((char *)ptr - sizeof(struct unit_head_large));
		if(head_large->prev) {
			head_large->prev->next = head_large->next;
		} else {
			unit_head_large_first  = head_large->next;
		}
		if(head_large->next) {
			head_large->next->prev = head_large->prev;
		}
		free(head_large);
		return;
	} else {
		/* ユニット解放 */
		struct block *block = head->block;
		if(head->block == NULL) {
			printf("memmgr: args of aFree is freed pointer %s line %d\n",file,line);
		} else {
			head->block = NULL;
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
		}
	}
}

/* 現在の状況を表示する */
static void memmgr_info(void) {
	int i;
	struct block *p;
	printf("** Memory Maneger Information **\n");
	if(block_first == NULL) {
		printf("Uninitialized.\n");
		return;
	}
	printf(
		"Blocks: %04u , BlockSize: %06u Byte , Used: %08uKB\n",
		block_last->block_no+1,sizeof(struct block),
		(block_last->block_no+1) * sizeof(struct block) / 1024
	);
	p = block_first;
	for(i=0;i<=block_last->block_no;i++) {
		printf("    Block #%04u : ",p->block_no);
		if(p->unit_size == 0) {
			printf("unused.\n");
		} else {
			printf(
				"size: %05u byte. used: %04u/%04u prev:",
				p->unit_size - sizeof(struct unit_head),p->unit_used,p->unit_count
			);
			if(p->samesize_prev == NULL) {
				printf("NULL");
			} else {
				printf("%04u",(p->samesize_prev)->block_no);
			}
			printf(" next:");
			if(p->samesize_next == NULL) {
				printf("NULL");
			} else {
				printf("%04u",(p->samesize_next)->block_no);
			}
			printf("\n");
		}
		p = p->block_next;
	}
}

/* ブロックを確保する */
static struct block* block_malloc(void) {
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
		int  block_no;
		struct block* p = (struct block *)calloc(sizeof(struct block),BLOCK_ALLOC);
		if(p == NULL) {
			printf("MEMMGR::block_alloc failed.\n");
			exit(1);
		}
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

static void block_free(struct block* p) {
	/* free() せずに、未使用フラグを付けるだけ */
	p->unit_size = 0;
	/* 未使用ポインターを更新する */
	if(block_unused == NULL) {
		block_unused = p;
	} else if(block_unused->block_no > p->block_no) {
		block_unused = p;
	}
}

static char memmer_logfile[128];

static FILE* memmgr_log(void) {
	FILE *fp = fopen(memmer_logfile,"w");
	if(!fp) { fp = stdout; }
	fprintf(fp,"memmgr: memory leaks found\n");
	return fp;
}

static void memmer_exit(void) {
	FILE *fp = NULL;
	int i;
	int count = 0;
	struct block *block = block_first;
	struct unit_head_large *large = unit_head_large_first;
	while(block) {
		if(block->unit_size) {
			if(!fp) { fp = memmgr_log(); }
			for(i=0;i<block->unit_count;i++) {
				struct unit_head *head = (struct unit_head*)(&block->data[block->unit_size * i]);
				if(head->block != NULL) {
					fprintf(
						fp,"%04d : %s line %d size %d\n",++count,
						head->file,head->line,head->size
					);
				}
			}
		}
		block = block->block_next;
	}
	while(large) {
		if(!fp) { fp = memmgr_log(); }
		fprintf(
			fp,"%04d : %s line %d size %d\n",++count,
			large->unit_head.file,
			large->unit_head.line,large->unit_head.size
		);
		large = large->next;
	}
	if(!fp) {
		printf("memmgr: no memory leaks found.\n");
	} else {
		printf("memmgr: memory leaks found.\n");
		fclose(fp);
	}
}
#endif

int do_init_memmgr(const char* file) {
	#ifdef USE_MEMMGR
		sprintf(memmer_logfile,"%s.log",file);
		atexit(memmer_exit);
		printf("memmgr: initialised: %s\n",memmer_logfile);
	#endif
	return 0;
}
