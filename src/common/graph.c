// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// graph creation is enabled
// #define ENABLE_GRAPH

#ifdef ENABLE_GRAPH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
	#include <unistd.h>
#endif
#ifdef MINGW
	#include <io.h>
#endif

#include "../common/core.h"
#include "../common/timer.h"
#include "../common/grfio.h"
#include "../common/malloc.h"
#include "graph.h"

struct graph {
	int   width;
	int   height;
	int   pallet_count;
	int   png_len;
	int   png_dirty;
	unsigned char* raw_data;
	unsigned char* png_data;
	int          * graph_value;
	int            graph_max;
};

void graph_write_dword(unsigned char* p,unsigned int v) {
	p[0] = (unsigned char)((v >> 24) & 0xFF);
	p[1] = (unsigned char)((v >> 16) & 0xFF);
	p[2] = (unsigned char)((v >>  8) & 0xFF);
	p[3] = (unsigned char)(v         & 0xFF);
}

struct graph* graph_create(unsigned int x,unsigned int y) {
	struct graph *g = (struct graph*)aCalloc(sizeof(struct graph),1);
	if(g == NULL) return NULL;
	// 256 * 3   : パレットデータ
	// x * y * 2 : イメージのバッファ
	// 256       : チャンクデータなどの予備
	g->png_data = (unsigned char *) aMalloc(4 * 256 + (x + 1) * y * 2);
	g->raw_data = (unsigned char *) aCalloc( (x + 1) * y , 1);
	memcpy(
		g->png_data,
		"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A\x00\x00\x00\x0D\x49\x48\x44\x52"
		"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x08\x03\x00\x00\x00\xFF\xFF\xFF"
		"\xFF\x00\x00\x00\x03\x50\x4C\x54\x45\xFF\xFF\xFF\xA7\xC4\x1B\xC8",0x30
	);
	graph_write_dword(g->png_data + 0x10,x);
	graph_write_dword(g->png_data + 0x14,y);
	graph_write_dword(g->png_data + 0x1D,grfio_crc32(g->png_data+0x0C,0x11));
	g->pallet_count = 1;
	g->width        = x;
	g->height       = y;
	g->png_dirty    = 1;
	g->graph_value  = (int *) aCalloc(x,sizeof(int));
	g->graph_max    = 1;
	return g;
}

void graph_pallet(struct graph* g, int index,unsigned long c) {
	if(g == NULL || c >= 256) return;

	if(g->pallet_count <= index) {
		malloc_set(g->png_data + 0x29 + 3 * g->pallet_count,0,(index - g->pallet_count) * 3);
		g->pallet_count = index + 1;
	}
	g->png_data[0x29 + index * 3    ] = (unsigned char)((c >> 16) & 0xFF); // R
	g->png_data[0x29 + index * 3 + 1] = (unsigned char)((c >>  8) & 0xFF); // G
	g->png_data[0x29 + index * 3 + 2] = (unsigned char)( c        & 0xFF); // B
	graph_write_dword(g->png_data + 0x21,g->pallet_count * 3);
	graph_write_dword(
		g->png_data + 0x29 + g->pallet_count * 3,
		grfio_crc32(g->png_data + 0x25,g->pallet_count * 3 + 4)
	);
	g->png_dirty = 1;
}

void graph_setpixel(struct graph* g,int x,int y,int color) {
	if(g == NULL || color >= 256) { return; }
	if(x < 0) x = 0;
	if(y < 0) y = 0;
	if(x >= g->width)  { x = g->width  - 1; }
	if(y >= g->height) { y = g->height - 1; }
	if(color >= g->pallet_count) { graph_pallet(g,color,graph_rgb(0,0,0)); }

	g->raw_data[y * (g->width + 1) + x + 1] = (unsigned char)color;
	g->png_dirty = 1;
}

int graph_getpixel(struct graph* g,int x,int y) {
	if(x < 0) x = 0;
	if(y < 0) y = 0;
	if(x >= g->width)  { x = g->width  - 1; }
	if(y >= g->height) { y = g->height - 1; }
	return g->raw_data[y * (g->width + 1) + x + 1];
}

const unsigned char* graph_output(struct graph* g,int *len) {
	unsigned long inflate_len;
	unsigned char *p;

	if(g == NULL) return NULL;
	if(g->png_dirty == 0) {
		*len = g->png_len;
		return g->png_data;
	}

	p = g->png_data + 0x2D + 3 * g->pallet_count;
	inflate_len = 2 * (g->width + 1) * g->height;
	memcpy(p + 4,"IDAT",4);
	encode_zip(p + 8,&inflate_len,g->raw_data,(g->width + 1) * g->height);
	graph_write_dword(p,inflate_len);
	graph_write_dword(p + 8 + inflate_len,grfio_crc32(p + 4, inflate_len + 4));

	p += 0x0C + inflate_len;
	memcpy(p,"\x00\x00\x00\x00\x49\x45\x4E\x44\xAE\x42\x60\x82",0x0C);
	p += 0x0C;
	g->png_len   = p - g->png_data;
	g->png_dirty = 0;
	*len = g->png_len;
	return g->png_data;
}

void graph_free(struct graph* g) {
	if(g != NULL) {
		aFree(g->png_data);
		aFree(g->raw_data);
		aFree(g->graph_value);
		aFree(g);
	}
}

// とりあえず不効率版。後ほど書き直し予定
void graph_square(struct graph* g,int x,int y,int xe,int ye,int color) {
	int i,j;
	if(g == NULL) return;
	if(x < 0) { x = 0; }
	if(y < 0) { y = 0; }
	if(xe > g->width)  { xe = g->width;  }
	if(ye > g->height) { ye = g->height; }
	for(i = y;i < ye ; i++) {
		for(j = x; j < xe ; j++) {
			graph_setpixel(g,j,i,color);
		}
	}
}

// とりあえず不効率版。後ほど書き直し予定
void graph_scroll(struct graph* g,int n,int color) {
	int x,y;
	if(g == NULL) return;
	for(y = 0; y < g->height; y++) {
		for(x = 0; x < g->width - n; x++) {
			graph_setpixel(g,x,y,graph_getpixel(g,x + n,y));
		}
		for( ; x < g->width; x++) {
			graph_setpixel(g,x,y,color);
		}
	}
}

void graph_data(struct graph* g,int value) {
	int i, j, start;
	if(g == NULL) return;
	memmove(&g->graph_value[0],&g->graph_value[1],sizeof(int) * (g->width - 1));
	g->graph_value[g->width - 1] = value;
	if(value > g->graph_max) {
		// 最大値を超えたので再描画
		g->graph_max = value;
		graph_square(g,0,0,g->width,g->height,0);
		start = 0;
	} else {
		// スクロールしてポイント打つ
		graph_scroll(g,1,0);
		start = g->width - 1;
	}
	for(i = start; i < g->width; i++) {
		int h0 = (i == 0 ? 0 : g->graph_value[i - 1]) * g->height / g->graph_max;
		int h1 = (g->graph_value[i]                 ) * g->height / g->graph_max;
		int h2 = (h0 < h1 ? 1 : -1);
		for(j = h0; j != h1; j += h2) {
			graph_setpixel(g,i,g->height - 1 - j,1);
		}
		graph_setpixel(g,i,g->height - 1 - h1,1);
	}
}

// 上の関数群を利用して、自動的にグラフを作成するタイマー群

#define GRP_WIDTH		300 // グラフの幅
#define GRP_HEIGHT		200 // グラフの高さ
#define GRP_COLOR		graph_rgb(0,0,255) // グラフの色
#define GRP_INTERVEL	60*1000 // グラフの更新間隔

#define GRP_PATH		"httpd/"

struct graph_sensor {
	struct graph* graph;
	char* str;
	char hash[32];
	int   scanid;
	int   drawid;
	int   interval;
	unsigned int (*func)(void);
};

static struct graph_sensor *sensor;
static int sensor_max;

static int graph_scan_timer(int tid,unsigned int tick,int id,int data)
{
	if(id >= 0 && id < sensor_max)
		graph_data(sensor[id].graph,sensor[id].func());
	return 0;
}

// modified by Celest -- i'm trying to separate it from httpd if possible ^^;
static int graph_draw_timer(int tid,unsigned int tick,int id,int data)
{
	char png_file[24];
	FILE *fp;
	
	// create/update the png file
	do {
		const char *png_data;
		int len;
		sprintf (png_file, GRP_PATH"%s.png", sensor[id].hash);
		fp = fopen(png_file, "w");
		// if another png of the same hash exists
		// (i.e 2nd login server with the same sensors)
		// this will fail = not good >.<
		if (fp == NULL)
			break;
		png_data = graph_output(sensor[id].graph, &len);
		fwrite(png_data,1,len,fp);
		fclose(fp);
	} while (0);
	
	// create/update text snippet
	do {
		char buf[8192], *p;
		p = buf;
		sprintf (png_file, GRP_PATH"%s.graph", sensor[id].hash);
		fp = fopen(png_file, "w");
		if (fp == NULL)
			break;
		p += sprintf(p,"<h2>%s</h2>\n\n",
			sensor[id].str);
		p += sprintf(p,"<p><img src=\"%s.png\" width=\"%d\" height=\"%d\"></p>\n",
			sensor[id].hash, GRP_WIDTH,GRP_HEIGHT);
		p += sprintf(p,"<p>Max: %d, Interval: %d sec</p>\n\n",
			sensor[id].graph->graph_max, sensor[id].interval / 1000);
		fprintf(fp, buf);
		fclose(fp);
	} while (0);

	return 0;
}

void graph_add_sensor(const unsigned char* string, int interval, unsigned int (*callback_func)(void))
{
	int draw_interval = interval * 2;
	struct graph *g = graph_create(GRP_WIDTH,GRP_HEIGHT);
	graph_pallet(g,1,GRP_COLOR);

	sensor = (struct graph_sensor *) aRealloc(sensor, sizeof(struct graph_sensor) * (sensor_max + 1));
	sensor[sensor_max].graph    = g;
	sensor[sensor_max].str      = aStrdup(string);
	// create crc32 hash of the sensor's name
	sprintf (sensor[sensor_max].hash, "%lu%c", grfio_crc32(string,strlen(string)), 'a' + SERVER_TYPE);
	sensor[sensor_max].func     = callback_func;
	sensor[sensor_max].scanid   = add_timer_interval(gettick() + 500, graph_scan_timer, sensor_max, 0, interval);
	sensor[sensor_max].drawid   = add_timer_interval(gettick() + 1000, graph_draw_timer, sensor_max, 0, draw_interval < 60000 ? 60000 : draw_interval);
	sensor[sensor_max].interval = interval;
	sensor_max++;

}

void graph_final (void)
{
	int i;
	for(i = 0; i < sensor_max; i++) {
		char png_file[24];
		// remove the png and snippet file
		sprintf (png_file, GRP_PATH"%s.png", sensor[i].hash);
		unlink (png_file);
		sprintf (png_file, GRP_PATH"%s.graph", sensor[i].hash);
		unlink (png_file);
		graph_free(sensor[i].graph);
		aFree(sensor[i].str);
		//delete_timer(sensor[i].scanid,graph_scan_timer);
		//delete_timer(sensor[i].drawid,graph_draw_timer);
	}
	aFree(sensor);
	sensor_max = 0;
}

void graph_init (void)
{
	graph_add_sensor ("Memory Usage", 1000, malloc_usage);
	add_timer_func_list(graph_scan_timer, "graph_scan_timer");
	add_timer_func_list(graph_draw_timer, "graph_draw_timer");
}

#else
void graph_init (void) {}
void graph_final (void) {}
#endif
