// $Id: db.c,v 1.2 2004/09/23 14:43:06 MouseJstr Exp $
// #define MALLOC_DBN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "mmo.h"
#include "utils.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define ROOT_SIZE 4096
#ifdef MALLOC_DBN
static struct dbn *dbn_root[512], *dbn_free;
static int dbn_root_rest=0,dbn_root_num=0;

static void * malloc_dbn(void)
{
	struct dbn* ret;

	if(dbn_free==NULL){
		if(dbn_root_rest<=0){
			CREATE(dbn_root[dbn_root_num], struct dbn, ROOT_SIZE);

			dbn_root_rest=ROOT_SIZE;
			dbn_root_num++;
		}
		return &(dbn_root[dbn_root_num-1][--dbn_root_rest]);
	}
	ret=dbn_free;
	dbn_free = dbn_free->parent;
	return ret;
}

static void free_dbn(struct dbn* add_dbn)
{
	add_dbn->parent = dbn_free;
	dbn_free = add_dbn;
}
#endif

static int strdb_cmp(struct dbt* table,void* a,void* b)
{
	if(table->maxlen)
		return strncmp(a,b,table->maxlen);
	return strcmp(a,b);
}

static unsigned int strdb_hash(struct dbt* table,void* a)
{
	int i;
	unsigned int h;
	unsigned char *p=a;

	i=table->maxlen;
	if(i==0) i=0x7fffffff;
	for(h=0;*p && --i>=0;){
		h=(h*33 + *p++) ^ (h>>24);
	}
	return h;
}

struct dbt* strdb_init(int maxlen)
{
	int i;
	struct dbt* table;

	CREATE(table, struct dbt, 1);

	table->cmp=strdb_cmp;
	table->hash=strdb_hash;
	table->maxlen=maxlen;
	for(i=0;i<HASH_SIZE;i++)
		table->ht[i]=NULL;
	return table;
}

static int numdb_cmp(struct dbt* table,void* a,void* b)
{
	int ia,ib;

	ia=(int)a;
	ib=(int)b;

	if((ia^ib) & 0x80000000)
		return ia<0 ? -1 : 1;

	return ia-ib;
}

static unsigned int numdb_hash(struct dbt* table,void* a)
{
	return (unsigned int)a;
}

struct dbt* numdb_init(void)
{
	int i;
	struct dbt* table;

	CREATE(table, struct dbt, 1);

	table->cmp=numdb_cmp;
	table->hash=numdb_hash;
	table->maxlen=sizeof(int);
	for(i=0;i<HASH_SIZE;i++)
		table->ht[i]=NULL;
	return table;
}

void* db_search(struct dbt *table,void* key)
{
	struct dbn *p;

	for(p=table->ht[table->hash(table,key) % HASH_SIZE];p;){
		int c=table->cmp(table,key,p->key);
		if(c==0)
			return p->data;
		if(c<0)
			p=p->left;
		else
			p=p->right;
	}
	return NULL;
}

void * db_search2(struct dbt *table, const char *key)
{
	int i,sp;
	struct dbn *p,*pn,*stack[64];
        int slen = strlen(key);

	for(i=0;i<HASH_SIZE;i++){
		if((p=table->ht[i])==NULL)
			continue;
		sp=0;
		while(1){
                        if (strnicmp(key, p->key, slen) == 0)
                           return p->data;
			if((pn=p->left)!=NULL){
				if(p->right){
					stack[sp++]=p->right;
				}
				p=pn;
			} else {
				if(p->right){
					p=p->right;
				} else {
					if(sp==0)
						break;
					p=stack[--sp];
				}
			}
		}
	}
        return 0;
}

static void db_rotate_left(struct dbn *p,struct dbn **root)
{
	struct dbn * y = p->right;
	p->right = y->left;
	if (y->left !=0)
		y->left->parent = p;
	y->parent = p->parent;

	if (p == *root)
		*root = y;
	else if (p == p->parent->left)
		p->parent->left = y;
	else
		p->parent->right = y;
	y->left = p;
	p->parent = y;
}

static void db_rotate_right(struct dbn *p,struct dbn **root)
{
	struct dbn * y = p->left;
	p->left = y->right;
	if (y->right != 0)
		y->right->parent = p;
	y->parent = p->parent;

	if (p == *root)
		*root = y;
	else if (p == p->parent->right)
		p->parent->right = y;
	else
		p->parent->left = y;
	y->right = p;
	p->parent = y;
}

static void db_rebalance(struct dbn *p,struct dbn **root)
{
	p->color = RED;
	while(p!=*root && p->parent->color==RED){ // rootは必ず黒で親は赤いので親の親は必ず存在する
		if (p->parent == p->parent->parent->left) {
			struct dbn *y = p->parent->parent->right;
			if (y && y->color == RED) {
				p->parent->color = BLACK;
				y->color = BLACK;
				p->parent->parent->color = RED;
				p = p->parent->parent;
			} else {
				if (p == p->parent->right) {
					p = p->parent;
					db_rotate_left(p, root);
				}
				p->parent->color = BLACK;
				p->parent->parent->color = RED;
				db_rotate_right(p->parent->parent, root);
			}
		} else {
			struct dbn* y = p->parent->parent->left;
			if (y && y->color == RED) {
				p->parent->color = BLACK;
				y->color = BLACK;
				p->parent->parent->color = RED;
				p = p->parent->parent;
			} else {
				if (p == p->parent->left) {
					p = p->parent;
					db_rotate_right(p, root);
				}
				p->parent->color = BLACK;
				p->parent->parent->color = RED;
				db_rotate_left(p->parent->parent, root);
			}
		}
	}
	(*root)->color=BLACK;
}

static void db_rebalance_erase(struct dbn *z,struct dbn **root)
{
	struct dbn *y = z, *x = NULL, *x_parent = NULL;

	if (y->left == NULL)
		x = y->right;
	else if (y->right == NULL)
		x = y->left;
	else {
		y = y->right;
		while (y->left != NULL)
			y = y->left;
		x = y->right;
	}
	if (y != z) { // 左右が両方埋まっていた時 yをzの位置に持ってきてzを浮かせる
		z->left->parent = y;
		y->left = z->left;
		if (y != z->right) {
			x_parent = y->parent;
			if (x) x->parent = y->parent;
			y->parent->left = x;
			y->right = z->right;
			z->right->parent = y;
		} else
			x_parent = y;
		if (*root == z)
			*root = y;
		else if (z->parent->left == z)
			z->parent->left = y;
		else
			z->parent->right = y;
		y->parent = z->parent;
		{ int tmp=y->color; y->color=z->color; z->color=tmp; }
		y = z;
	} else { // どちらか空いていた場合 xをzの位置に持ってきてzを浮かせる
		x_parent = y->parent;
		if (x) x->parent = y->parent;
		if (*root == z)
			*root = x;
		else if (z->parent->left == z)
			z->parent->left = x;
		else
			z->parent->right = x;
	}
	// ここまで色の移動の除いて通常の2分木と同じ
	if (y->color != RED) { // 赤が消える分には影響無し
		while (x != *root && (x == NULL || x->color == BLACK))
			if (x == x_parent->left) {
				struct dbn* w = x_parent->right;
				if (w->color == RED) {
					w->color = BLACK;
					x_parent->color = RED;
					db_rotate_left(x_parent, root);
					w = x_parent->right;
				}
				if ((w->left == NULL ||
					 w->left->color == BLACK) &&
					(w->right == NULL ||
					 w->right->color == BLACK)) {
					w->color = RED;
					x = x_parent;
					x_parent = x_parent->parent;
				} else {
					if (w->right == NULL ||
						w->right->color == BLACK) {
						if (w->left) w->left->color = BLACK;
						w->color = RED;
						db_rotate_right(w, root);
						w = x_parent->right;
					}
					w->color = x_parent->color;
					x_parent->color = BLACK;
					if (w->right) w->right->color = BLACK;
					db_rotate_left(x_parent, root);
					break;
				}
			} else {                  // same as above, with right <-> left.
				struct dbn* w = x_parent->left;
				if (w->color == RED) {
					w->color = BLACK;
					x_parent->color = RED;
					db_rotate_right(x_parent, root);
					w = x_parent->left;
				}
				if ((w->right == NULL ||
					 w->right->color == BLACK) &&
					(w->left == NULL ||
					 w->left->color == BLACK)) {
					w->color = RED;
					x = x_parent;
					x_parent = x_parent->parent;
				} else {
					if (w->left == NULL ||
						w->left->color == BLACK) {
						if (w->right) w->right->color = BLACK;
						w->color = RED;
						db_rotate_left(w, root);
						w = x_parent->left;
					}
					w->color = x_parent->color;
					x_parent->color = BLACK;
					if (w->left) w->left->color = BLACK;
					db_rotate_right(x_parent, root);
					break;
				}
			}
		if (x) x->color = BLACK;
	}
}

struct dbn* db_insert(struct dbt *table,void* key,void* data)
{
	struct dbn *p,*priv;
	int c,hash;

	hash = table->hash(table,key) % HASH_SIZE;
	for(c=0,priv=NULL ,p = table->ht[hash];p;){
		c=table->cmp(table,key,p->key);
		if(c==0){ // replace
                        if (table->release)
                            table->release(p, 3);
			p->data=data;
			p->key=key;
			return p;
		}
		priv=p;
		if(c<0){
			p=p->left;
		} else {
			p=p->right;
		}
	}
#ifdef MALLOC_DBN
	p=malloc_dbn();
#else
	CREATE(p, struct dbn, 1);
#endif
	if(p==NULL){
		printf("out of memory : db_insert\n");
		return NULL;
	}
	p->parent= NULL;
	p->left  = NULL;
	p->right = NULL;
	p->key   = key;
	p->data  = data;
	p->color = RED;
	if(c==0){ // hash entry is empty
		table->ht[hash] = p;
		p->color = BLACK;
	} else {
		if(c<0){ // left node
			priv->left = p;
			p->parent=priv;
		} else { // right node
			priv->right = p;
			p->parent=priv;
		}
		if(priv->color==RED){ // must rebalance
			db_rebalance(p,&table->ht[hash]);
		}
	}
	return p;
}

void* db_erase(struct dbt *table,void* key)
{
	void *data;
	struct dbn *p;
	int c,hash;

	hash = table->hash(table,key) % HASH_SIZE;
	for(c=0,p = table->ht[hash];p;){
		c=table->cmp(table,key,p->key);
		if(c==0)
			break;
		if(c<0)
			p=p->left;
		else
			p=p->right;
	}
	if(!p)
		return NULL;
	data=p->data;
	db_rebalance_erase(p,&table->ht[hash]);
#ifdef MALLOC_DBN
	free_dbn(p);
#else
	free(p);
#endif
	return data;
}

void db_foreach(struct dbt *table,int (*func)(void*,void*,va_list),...)
{
	int i,sp;
	// red-black treeなので64個stackがあれば2^32個ノードまで大丈夫
	struct dbn *p,*pn,*stack[64];
	va_list ap;

	va_start(ap,func);
	for(i=0;i<HASH_SIZE;i++){
		if((p=table->ht[i])==NULL)
			continue;
		sp=0;
		while(1){
			func(p->key,p->data,ap);
			if((pn=p->left)!=NULL){
				if(p->right){
					stack[sp++]=p->right;
				}
				p=pn;
			} else {
				if(p->right){
					p=p->right;
				} else {
					if(sp==0)
						break;
					p=stack[--sp];
				}
			}
		}
	}
	va_end(ap);
}

void db_final(struct dbt *table,int (*func)(void*,void*,va_list),...)
{
	int i,sp;
	struct dbn *p,*pn,*stack[64];
	va_list ap;

	va_start(ap,func);
	for(i=0;i<HASH_SIZE;i++){
		if((p=table->ht[i])==NULL)
			continue;
		sp=0;
		while(1){
			if(func)
				func(p->key,p->data,ap);
			if((pn=p->left)!=NULL){
				if(p->right){
					stack[sp++]=p->right;
				}
			} else {
				if(p->right){
					pn=p->right;
				} else {
					if(sp==0)
						break;
					pn=stack[--sp];
				}
			}
#ifdef MALLOC_DBN
			free_dbn(p);
#else
			free(p);
#endif
			p=pn;
		}
	}
	free(table);
	va_end(ap);
}
