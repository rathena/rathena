// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _GRAPH_H_
#define _GRAPH_H_

void graph_init (void);
void graph_final (void);

struct graph* graph_create(unsigned int x,unsigned int y);
void graph_pallet(struct graph* g, int index,unsigned long c);
const unsigned char* graph_output(struct graph* g,int *len);
void graph_setpixel(struct graph* g,int x,int y,int color);
void graph_scroll(struct graph* g,int n,int color);
void graph_square(struct graph* g,int x,int y,int xe,int ye,int color);

// athenaの状態を調査するセンサーを追加する。
// string        : センサーの名称(Login Users など)
// inetrval      : センサーの値を所得する間隔(msec)
// callback_func : センサーの値を返す関数( unsigned int login_users(void); など) 

void graph_add_sensor(const char* string, int interval, unsigned int (*callback_func)(void));

#define graph_rgb(r,g,b) (((r) << 16) | ((g) << 8) | (b))

#endif

