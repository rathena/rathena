#ifndef _DB_H_
#define _DB_H_

#include <stdarg.h>

#define HASH_SIZE (256+27)

#define RED 0
#define BLACK 1

struct dbn {
	struct dbn *parent,*left,*right;
	int color;
	void *key;
	void *data;
};

struct dbt {
	int (*cmp)(struct dbt*,void*,void*);
	unsigned int (*hash)(struct dbt*,void*);
    // which 1 - key,   2 - data,  3 - both
	void (*release)(struct dbn*,int which);
	int maxlen;
	struct dbn *ht[HASH_SIZE];
};

#define strdb_search(t,k)   db_search((t),(void*)(k))
#define strdb_insert(t,k,d) db_insert((t),(void*)(k),(void*)(d))
#define strdb_erase(t,k)    db_erase ((t),(void*)(k))
#define strdb_foreach       db_foreach
#define strdb_final         db_final
#define numdb_search(t,k)   db_search((t),(void*)(k))
#define numdb_insert(t,k,d) db_insert((t),(void*)(k),(void*)(d))
#define numdb_erase(t,k)    db_erase ((t),(void*)(k))
#define numdb_foreach       db_foreach
#define numdb_final         db_final

struct dbt* strdb_init(int maxlen);
struct dbt* numdb_init(void);
void* db_search(struct dbt *table,void* key);
void* db_search2(struct dbt *table, const char *key); // [MouseJstr]
struct dbn* db_insert(struct dbt *table,void* key,void* data);
void* db_erase(struct dbt *table,void* key);
void db_foreach(struct dbt*,int(*)(void*,void*,va_list),...);
void db_final(struct dbt*,int(*)(void*,void*,va_list),...);

#endif
