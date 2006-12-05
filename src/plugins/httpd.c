
#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/db.h"
//mmo required for definition of stricmp
#include "../common/mmo.h"
#include "../common/utils.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/plugin.h"
//#include "httpd.h"

/** Created by End_of_Exam, ported to plugin and modified by Celest **/

PLUGIN_INFO = {
	"HttpDaemon",
	PLUGIN_CORE,
	"0.1",
	PLUGIN_VERSION,
	"HTTP Daemon"
};

PLUGIN_EVENTS_TABLE = {
	{ "do_init", "Plugin_Init" },
	{ "do_final", "Plugin_Final" },
	{ NULL, NULL }
};

enum HTTPD_STATUS {
	HTTPD_REQUEST_WAIT = 0,		// リクエスト待ち
	HTTPD_REQUEST_WAIT_POST,	// リクエスト待ち(post)
	HTTPD_REQUEST_OK,			// リクエスト解釈完了
	HTTPD_SEND_HEADER,			// ヘッダ送信完了
	HTTPD_WAITING_SEND			// データが送信し終わるまで待っている状態
};
enum {
	HTTPD_METHOD_UNKNOWN = 0,
	HTTPD_METHOD_GET,
	HTTPD_METHOD_POST
};

struct httpd_session_data {
	int fd;
	int status;
	int http_ver;
	int header_len;
	int data_len;
	int method;
	int persist;
	int request_count;
	unsigned int tick;
	const unsigned char* url;
	const unsigned char* query;
};

// undefine socket operations included from socket.h,
// since we are going to use 'sessiond' instead
#undef RFIFOP
#undef RFIFOREST
#undef WFIFOP

#define RFIFOP(fd,pos) (sessiond[fd]->rdata+sessiond[fd]->rdata_pos+(pos))
#define RFIFOREST(fd)  (sessiond[fd]->rdata_size-sessiond[fd]->rdata_pos)
#define WFIFOP(fd,pos) (sessiond[fd]->wdata+sessiond[fd]->wdata_size+(pos))

struct socket_data **sessiond;
char *server_type;
unsigned int (*gettick)();
int (*add_timer_interval)(unsigned int,int (*)(int,unsigned int,int,int),int,int,int);
int *max_fd;
int (*delete_sessiond)(int);
int (*_WFIFOSET)(int,int);
int (*_RFIFOSKIP)(int,int);

static int max_persist_requests	= 32;	// 持続通信での最大リクエスト数
static int request_timeout[]	= { 2500, 60*1000 };	// タイムアウト(最初、持続)
static char document_root[256]	= "./httpd/";	// ドキュメントルート

// httpd に入っているページと、呼び出すコールバック関数の一覧
struct dbt *httpd_files;

void httpd_send(struct httpd_session_data*, int, const char *, int, const void *);

int httpd_check (struct socket_data *sd)
{
	// httpd に回すどうかの判定がまだ行われてない
	// 先頭２バイトが GE ならhttpd に回してみる
	if (sd->rdata_size >= 2 &&
		sd->rdata[0] == 'G' && sd->rdata[1] == 'E')
		return 1;

	return 0;
}

int httpd_strcasencmp(const char *s1, const char *s2,int len)
{
	while(len-- && (*s1 || *s2) ) {
		if((*s1 | 0x20) != (*s2 | 0x20)) {
			return ((*s1 | 0x20) > (*s2 | 0x20) ? 1 : -1);
		}
		s1++; s2++;
	}
	return 0;
}

// httpd にページを追加する
// for などでページ名を合成できるように、key はstrdup()したものを使う

void httpd_pages (const char* url, void (*httpd_func)(struct httpd_session_data*, const char*))
{
	if (strdb_get(httpd_files,(unsigned char*)(url+1)) == NULL) {
		strdb_put(httpd_files, (unsigned char*)aStrdup(url+1), httpd_func);
	} else {
		strdb_put(httpd_files, (unsigned char*)(url+1), httpd_func);
	}
}

static void (*httpd_default)(struct httpd_session_data* sd,const char* url);

const char *httpd_get_error( struct httpd_session_data* sd, int* status )
{
	const char* msg;
	// httpd のステータスを決める
	switch(*status) {
	case 200: msg = "OK";           break;
	case 400: msg = "Bad Request";  break;
	case 401: msg = "Unauthorized"; break;	// 未使用
	case 403: msg = "Forbidden";    break;	// 未使用
	case 404: msg = "Not Found";    break;
	case 408: msg = "Request Timedout"; break;
	case 411: msg = "Length Required"; break;
	case 413: msg = "Request Entity Too Large"; break;
	default:
		*status = 500; msg = "Internal Server Error"; break;
	}
	return msg;
}

void httpd_send_error(struct httpd_session_data* sd,int status)
{
	const char* msg = httpd_get_error( sd, &status );
	httpd_send(sd, status, "text/plain",strlen(msg),msg);
}

void httpd_send_head (struct httpd_session_data* sd, int status, const char *content_type, int content_len)
{
	char head[256];
	int len;
	const char* msg;

	if (sd->status != HTTPD_REQUEST_OK)
		return;
	msg = httpd_get_error( sd, &status );
	
	if(content_len == -1 || ++sd->request_count >= max_persist_requests ) {
		// 長さが分からない or リクエスト限界を超えたので切断する
		len = sprintf(
			head,
			"HTTP/1.%d %d %s\r\nContent-Type: %s\r\nConnection: close\r\n\r\n",
			sd->http_ver,status,msg,content_type
		);
		sd->persist = 0;
		len = sprintf(
			head,
			"HTTP/1.%d %d %s\r\nContent-Type: %s\r\n\r\n",
			sd->http_ver,status,msg,content_type
		);
		sd->http_ver = 0; // 長さが分からないので、HTTP/1.0 扱い(自動切断)にする
	} else {
		len = sprintf(
			head,
			"HTTP/1.%d %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n",
			sd->http_ver,status,msg,content_type,content_len
		);
	}
	memcpy(WFIFOP(sd->fd,0),head,len);
	_WFIFOSET(sd->fd,len);
	sd->status   = HTTPD_SEND_HEADER;
	sd->data_len = content_len;
}

void httpd_send_data (struct httpd_session_data* sd, int content_len, const void *data)
{
	const char* msg = (const char*)data;
	if (sd->status == HTTPD_REQUEST_OK) {
		// ヘッダの送信忘れているので、適当に補う
		httpd_send_head(sd,200,"application/octet-stream",-1);
	} else if(sd->status != HTTPD_SEND_HEADER && sd->status != HTTPD_WAITING_SEND) {
		return;
	}
	sd->data_len -= content_len;

	// 巨大なサイズのファイルも送信出来るように分割して送る
	while (content_len > 0) {
		int send_byte = content_len;
		if(send_byte > 12*1024) send_byte = 12*1024;
		memcpy(WFIFOP(sd->fd,0),msg,send_byte);
		_WFIFOSET(sd->fd,send_byte);
		msg += send_byte; content_len -= send_byte;
	}
	sd->status = HTTPD_WAITING_SEND;
}

void httpd_send (struct httpd_session_data* sd, int status, const char *content_type, int content_len, const void *data)
{
	httpd_send_head(sd,status,content_type,content_len);
	httpd_send_data(sd,content_len,data);
}

void httpd_parse_header(struct httpd_session_data* sd);
void httpd_parse_request_ok(struct httpd_session_data *sd);

int httpd_parse (int fd)
{
	struct httpd_session_data *sd = (struct httpd_session_data *)sessiond[fd]->session_data2;
	if (sessiond[fd]->eof) {
		delete_sessiond(fd);
		return 0;
	}
	if (sd == NULL) {
		sd = (struct httpd_session_data*) aMalloc (sizeof(struct httpd_session_data));
		sd->fd = fd;
		sessiond[fd]->session_data2  = sd;
		sd->tick = gettick();
		sd->persist = 0;
		sd->request_count = 0;
	}
	printf ("status %d\n", sd->status);
	switch(sd->status) {
	case HTTPD_REQUEST_WAIT:
		// リクエスト待ち
		if(RFIFOREST(fd) > 1024) {
			// リクエストが長すぎるので、エラー扱いする
			sd->status = HTTPD_REQUEST_OK;
			httpd_send_error(sd,400); // Bad Request
		} else if( (int)( gettick() - sd->tick ) > request_timeout[sd->persist] ) {
			// リクエストに時間がかかりすぎているので、エラー扱いする
			sd->status = HTTPD_REQUEST_OK;
			httpd_send_error(sd,408); // Request Timeout
		} else if(sd->header_len == RFIFOREST(fd)) {
			// 状態が以前と同じなので、リクエストを再解析する必要は無い
		} else {
			int limit = RFIFOREST(fd);
			unsigned char *req = RFIFOP(fd,0);
			sd->header_len = RFIFOREST(fd);
			do {
				if(*req == '\n' && limit > 0) {
					limit--; req++;
					if(*req == '\r' && limit > 0) { limit--; req++; }
					if(*req == '\n') {
						// HTTPヘッダの終点を見つけた
						*req   = 0;
						sd->header_len = (req - RFIFOP(fd,0)) + 1;
						httpd_parse_header(sd);
						break;
					}
				}
			} while(req++,--limit > 0);
		}
		break;
	case HTTPD_REQUEST_WAIT_POST:
		if(RFIFOREST(sd->fd) >= sd->header_len) {
			unsigned char temp = RFIFOB(sd->fd,sd->header_len);
			RFIFOB(sd->fd,sd->header_len) = 0;
			httpd_parse_request_ok(sd);
			RFIFOB(sd->fd,sd->header_len) = temp;
		}
		break;
	case HTTPD_REQUEST_OK:
	case HTTPD_SEND_HEADER:
		// リクエストが終わったまま何も送信されていない状態なので、
		// 強制切断
		printf ("httpd: eof\n");
		sessiond[fd]->eof = 1;
		break;
	case HTTPD_WAITING_SEND:
		// データの送信が終わるまで待機
		//if(sessiond[fd]->wdata_size == sessiond[fd]->wdata_pos) {
		// i *hope* this is correct o.o;
		if(sessiond[fd]->wdata_size == 0) {
			// HTTP/1.0は手動切断
//			if(sd->http_ver == 0) {
			if(sd->persist == 0) {
				printf ("httpd: eof\n");
				sessiond[fd]->eof = 1;
			}
			// RFIFO からリクエストデータの消去と構造体の初期化
			_RFIFOSKIP(fd,sd->header_len);
			sd->status     = HTTPD_REQUEST_WAIT;
			sd->tick       = gettick();
			sd->header_len = 0;
			sd->query      = NULL;
//			sd->http_ver   = 0;	// ver は保持
			sd->method     = HTTPD_METHOD_UNKNOWN;
			printf("httpd_parse: [% 3d] request sended RFIFOREST:%d\n", fd, RFIFOREST(fd));
		}
		break;
	}
	return 0;
}

void httpd_parse_header_sub( struct httpd_session_data* sd, const char *p1, int* plen )
{
	int len = 0;
	// HTTPのバージョンを調査
	if(!strncmp(p1 ,"HTTP/1.1",8)) {
		sd->http_ver = 1;
		sd->persist  = 1;
	} else {
		sd->http_ver = 0;
		sd->persist  = 0;
	}

	p1  = strchr(p1,'\n');
	while(p1) {
		// Content-Length: の調査
		if(!httpd_strcasencmp(p1+1,"Content-Length: ",16)) {
			len = atoi(p1 + 17);
		}
		// Connection: の調査
		if(!httpd_strcasencmp(p1+1,"Connection: ",12)) {
			if( httpd_strcasencmp(p1+13,"close",5)==0 && sd->http_ver==1 )
				sd->persist = 0;
			if( httpd_strcasencmp(p1+13,"Keep-Alive",10)==0 && sd->http_ver==0 )
				sd->persist = 1;
		}
		p1 = strchr(p1+1,'\n');
	}
	if(plen) *plen = len;
	return;
}

void httpd_parse_header(struct httpd_session_data* sd)
{
	int i;
	int status = 400; // Bad Request
	unsigned char* req    = RFIFOP(sd->fd,0);
	do {
		if(!strncmp(req,"GET /",5)) {
			// GET リクエスト
			req += 5;
			for(i = 0;req[i]; i++) {
				if(req[i] == ' ' || req[i] == '?') break;
				if(!isalnum(req[i]) && req[i] != '.' && req[i] != '_' && req[i] != '-') break;
			}
			if(req[i] == ' ') {
				req[i]     = 0;
				sd->url    = req;
				sd->query  = NULL;
				sd->status = HTTPD_REQUEST_OK;
			} else if(req[i] == '?') {
				req[i]    = 0;
				sd->query = &req[++i];
				for(;req[i];i++) {
					if(
						isalnum(req[i]) || req[i] == '+' || req[i] == '%' || req[i] == '&' ||
						req[i] == '='
					) {
						continue;
					} else {
						break;
					}
				}
				if(req[i] != ' ') {
					break;
				}
				req[i]     = 0;
				sd->url    = req;
			} else {
				break;
			}
			// ヘッダ解析
			httpd_parse_header_sub( sd, &req[i+1], NULL );
			
			printf("httpd: request %s %s\n", sd->url, sd->query);
			sd->method     = HTTPD_METHOD_GET;
			httpd_parse_request_ok(sd);
		} else if(!strncmp(req,"POST /",6)) {
			int  len;
			req += 6; status = 404;
			for(i = 0;req[i]; i++) {
				if(req[i] == ' ') break;
				if(!isalnum(req[i]) && req[i] != '.' && req[i] != '_' && req[i] != '-') break;
			}
			if(req[i] != ' ') {
				break;
			}
			req[i]     = 0;
			sd->url    = req;

			// ヘッダ解析
			httpd_parse_header_sub( sd, &req[i+1], &len );
			
			if(len <= 0 || len >= 32*1024) {
				// とりあえず32KB以上のリクエストは不正扱い
				status = ( len==0 )? 411 : ( len>32*1024 )? 413 : 400;
				break;
			}
			
			sd->query      =  RFIFOP(sd->fd,sd->header_len);
			sd->method     = HTTPD_METHOD_POST;
			sd->header_len += len;
			if(RFIFOREST(sd->fd) >= sd->header_len) {
				unsigned char temp = RFIFOB(sd->fd,sd->header_len);
				RFIFOB(sd->fd,sd->header_len) = 0;
				httpd_parse_request_ok(sd);
				RFIFOB(sd->fd,sd->header_len) = temp;
			} else {
				// POSTのデータが送られてくるのを待つ
				sd->status = HTTPD_REQUEST_WAIT_POST;
			}
		} else {
			break;
		}
	} while(0);
	if(sd->status == HTTPD_REQUEST_WAIT) {
		sd->status = HTTPD_REQUEST_OK;
		httpd_send_error(sd,status); 
	}
}

void httpd_parse_request_ok (struct httpd_session_data *sd)
{
	void (*httpd_parse_func)(struct httpd_session_data*,const char*);
	sd->status = HTTPD_REQUEST_OK;

	// ファイル名が求まったので、ページが無いか検索する
	// printf("httpd_parse: [% 3d] request /%s\n", fd, req);
	httpd_parse_func = strdb_get(httpd_files,(unsigned char*)sd->url);
	if(httpd_parse_func == NULL) {
		httpd_parse_func = httpd_default;
	}
	if(httpd_parse_func == NULL) {
		httpd_send_error(sd,404); // Not Found
	} else {
		httpd_parse_func(sd,sd->url);
		if(sd->status == HTTPD_REQUEST_OK) {
			httpd_send_error(sd,404); // Not Found
		}
	}
	if(sd->persist == 1 && sd->data_len) {
		// 長さが変なデータ(こんなの送るなよ…)
		printf("httpd_parse: send size mismatch when parsing /%s\n", sd->url);
		sessiond[sd->fd]->eof = 1;
	}
	if(sd->status == HTTPD_REQUEST_OK) {
		httpd_send_error(sd,404); 
	}
}

char* httpd_get_value(struct httpd_session_data* sd,const char* val)
{
	int src_len = strlen(val);
	const unsigned char* src_p = sd->query;
	if(src_p == NULL) return aStrdup("");

	do {
		if(!memcmp(src_p,val,src_len) && src_p[src_len] == '=') {
			break;
		}
		src_p = strchr(src_p + 1,'&');
		if(src_p) src_p++;
	} while(src_p);

	if(src_p != NULL) {
		// 目的の文字列を見つけた
		const unsigned char* p2;
		int   dest_len;
		char* dest_p;
		src_p += src_len + 1;
		p2     = strchr(src_p,'&');
		if(p2 == NULL) {
			src_len = strlen(src_p);
		} else {
			src_len = (p2 - src_p);
		}
		dest_p   = aMalloc(src_len + 1);
		dest_len = 0;
		while(src_len > 0) {
			if(*src_p == '%' && src_len > 2) {
				int c1 = 0,c2 = 0;
				if(src_p[1] >= '0' && src_p[1] <= '9') c1 = src_p[1] - '0';
				if(src_p[1] >= 'A' && src_p[1] <= 'F') c1 = src_p[1] - 'A' + 10;
				if(src_p[1] >= 'a' && src_p[1] <= 'f') c1 = src_p[1] - 'a' + 10;
				if(src_p[2] >= '0' && src_p[2] <= '9') c2 = src_p[2] - '0';
				if(src_p[2] >= 'A' && src_p[2] <= 'F') c2 = src_p[2] - 'A' + 10;
				if(src_p[2] >= 'a' && src_p[2] <= 'f') c2 = src_p[2] - 'a' + 10;
				dest_p[dest_len++] = (c1 << 4) | c2;
				src_p += 3; src_len -= 3;
			} else if(*src_p == '+') {
				dest_p[dest_len++] = ' ';
				src_p++; src_len--;
			} else {
				dest_p[dest_len++] = *(src_p++); src_len--;
			}
		}
		dest_p[dest_len] = 0;
		return dest_p;
	}
	return aStrdup("");
}

// MIMEタイプ判定。主要なものだけ判定して、残りはapplication/octet-stream
static const char* httpd_mimetype(const char* url)
{
	char *ext = strrchr(url,'.');
	if(ext) {
		if(!strcmp(ext,".html")) return "text/html";
		if(!strcmp(ext,".htm"))  return "text/html";
		if(!strcmp(ext,".css"))  return "text/css";
		if(!strcmp(ext,".js"))   return "text/javascript";
		if(!strcmp(ext,".txt"))  return "text/plain";
		if(!strcmp(ext,".gif"))  return "image/gif";
		if(!strcmp(ext,".jpg"))  return "image/jpeg";
		if(!strcmp(ext,".jpeg")) return "image/jpeg";
		if(!strcmp(ext,".png"))  return "image/png";
		if(!strcmp(ext,".xbm"))  return "image/xbm";
		if(!strcmp(ext,".zip"))  return "application/zip";
	}
	return "application/octet-stream";
}

void httpd_send_file(struct httpd_session_data* sd, const char* url)
{
	FILE *fp;
	int  file_size;
	char file_buf[8192];
	if(sd->status != HTTPD_REQUEST_OK) return;
	if(url[0] == '\0') url = "index.html";
	
	// url の最大長は約1010バイトなので、バッファオーバーフローの心配は無し
	sprintf(file_buf, "%s%s", document_root, url);

	fp = fopen(file_buf,"rb");
	if(fp == NULL) {
		httpd_send_error(sd,404);
	} else {
		fseek(fp,0,SEEK_END);
		file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		httpd_send_head(sd,200,httpd_mimetype(url),file_size);
		while(file_size > 0) {
			int read_byte = file_size;
			if(file_size > 8192) read_byte = 8192;
			fread(file_buf,1,read_byte,fp);
			httpd_send_data(sd,read_byte,file_buf);
			file_size -= read_byte;
		}
		fclose(fp);
	}
}


char* httpd_binary_encode(const char* val)
{
	char *buf = aMalloc(strlen(val) * 3 + 1);
	char *p   = buf;
	while(*val) {
		if(isalnum((unsigned char)*val)) {
			*(p++) = *(val++);
		} else {
			unsigned char c1 = *(val++);
			unsigned char c2 = (c1 >> 4);
			unsigned char c3 = (c1 & 0x0F);
			*(p++) = '%';
			*(p++) = c2 + (c2 >= 10 ? 'A'-10 : '0');
			*(p++) = c3 + (c3 >= 10 ? 'A'-10 : '0');
		}
	}
	*p = 0;
	return buf;
}

char* httpd_quote_meta(const char* p1)
{
	char *buf = aMalloc(strlen(p1) * 6 + 1);
	char *p2  = buf;
	while(*p1) {
		switch(*p1) {
		case '<': memcpy(p2,"&lt;",4);   p2 += 4; p1++; break;
		case '>': memcpy(p2,"&gt;",4);   p2 += 4; p1++; break;
		case '&': memcpy(p2,"&amp;",5);  p2 += 5; p1++; break;
		case '"': memcpy(p2,"&quot;",6); p2 += 6; p1++; break;
		default: *(p2++) = *(p1++);
		}
	}
	*p2 = 0;
	return buf;
}

///////// Graph / HTML snippets functions /////////////////////////////

struct file_entry {
	char *filename;
	struct file_entry *next;
};
struct file_entry *fileentry_head = NULL;

static void httpd_graph_load (const char *filename)
{
	struct file_entry *entry;
	char type = *server_type + 'a';
	int len = strlen(filename);

	if (len <= 7 || filename[len - 7] != type)
		return;

	entry = fileentry_head;
	while (entry) {
		if (strcmpi(entry->filename, filename) == 0)
			return;
		entry = entry->next;
	}

	entry = (struct file_entry *) aMalloc (sizeof(struct file_entry));
	entry->filename = aStrdup(filename);
	entry->next = fileentry_head;
	fileentry_head = entry;
}

// scan for available html snippets
static int httpd_graph_find (int tid, unsigned int tick, int id, int data)
{
	findfile("httpd", ".graph", httpd_graph_load);
	return 0;
}

static void httpd_graph_parse (struct httpd_session_data *sd,const char* url)
{
	// output html
	struct file_entry *entry = fileentry_head;
	char buf[8192];
	char *p = buf;
	FILE *fp;

	p += sprintf(p,"<html><head><title>Athena Sensors</title></head>\n\n<body>\n");
	p += sprintf(p,"<h1>Athena Sensors</h1>\n\n");

	while (entry) {
		// insert snippets into html
		char line[1024];
		fp = fopen(entry->filename, "r");
		if (fp == NULL) {
			entry = entry->next;
			continue;
		}
		while(fgets(line, sizeof(line) -1, fp))
			p += sprintf(p, line);
		fclose(fp);
		entry = entry->next;
	}
	p += sprintf(p,"</body></html>\n");
	httpd_send(sd,200,"text/html",p - buf,buf);
}

//////////////// Initialise / Finalise /////////////////////////////

void do_final (void)
{
	int fd;
	struct file_entry *entry = fileentry_head, *entry2;

	// clear up graph entries
	while (entry) {
		entry2 = entry->next;
		aFree(entry->filename);
		aFree(entry);
		entry = entry2;
	}
	// clear up existing http connections
	for (fd = 0; fd < *max_fd; fd++)
		if (sessiond[fd] && sessiond[fd]->type == SESSION_HTTP)
			delete_sessiond(fd);

	httpd_files->destroy(httpd_files,NULL);
	// clear up the database
	db_final();
	// clear up allocated memory
	// note: the memory manager, if enabled, would be
	// separate from the parent program, which is also
	// why we need to delete our http sessions
	// separately above
	malloc_final();
}

void do_init (void)
{
	struct func_parse_table *parse_table;
	int enable_httpd = 1;

	do {
		char line[1024], w1[1024], w2[1024];
		FILE *fp = fopen("plugins/httpd.conf","r");
		if (fp == NULL)
			break;

		while(fgets(line, sizeof(line) -1, fp)) {
			if (line[0] == '/' && line[1] == '/')
				continue;
			if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
				if(strcmpi(w1,"enable_httpd")==0){
					enable_httpd = atoi(w2);
				} else if(strcmpi(w1,"document_root")==0){
					strcpy(document_root, w2);
				} else if(strcmpi(w1,"request_timeout_first")==0){
					request_timeout[0] = atoi(w2);
				} else if(strcmpi(w1,"request_timeout_persist")==0){
					request_timeout[1] = atoi(w2);
				} else if(strcmpi(w1,"max_persist_request")==0){
					max_persist_requests = atoi(w2);
				}
			}
		}
		fclose(fp);
	} while (0);

	if (!enable_httpd)
		return;

	malloc_init();
	db_init();
	IMPORT_SYMBOL(server_type, 0);
	IMPORT_SYMBOL(gettick, 5);
	IMPORT_SYMBOL(add_timer_interval, 8);
	IMPORT_SYMBOL(max_fd, 13);
	IMPORT_SYMBOL(sessiond, 14);
	IMPORT_SYMBOL(delete_sessiond, 15);	
	IMPORT_SYMBOL(_WFIFOSET, 16);
	IMPORT_SYMBOL(_RFIFOSKIP, 17);
	IMPORT_SYMBOL(parse_table, 18);

	// register http parsing function
	parse_table[SESSION_HTTP].check = httpd_check;
	parse_table[SESSION_HTTP].func = httpd_parse;

	httpd_files = db_alloc(__FILE__,__LINE__,DB_STRING,DB_OPT_RELEASE_KEY,50);
	httpd_default = httpd_send_file;

	httpd_pages ("/graph", httpd_graph_parse);
	add_timer_interval(gettick()+10000,httpd_graph_find,0,0,10000);

	return;
}
