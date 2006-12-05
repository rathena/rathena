#ifndef _HTTPD_H_
#define _HTTPD_H_

struct httpd_session_data;

// NOTE by Celest: This file is not used by httpd.c, but included only as an API reference.

// 注意
//   1.athena内蔵のhttpd で大きなファイルを送信することはお勧めしません。
//     200KB を超えるようなファイルは、別のソフトを利用することを勧めます。
//   2.ファイル名に使える文字は、[A-Za-z0-9-_\.] です。他の文字を使うと、
//     BAD REQUEST で弾かれます。



void httpd_pages(const char* url,void(*httpd_func)(struct httpd_session_data* sd,const char* url));

// 指定されたURL に対するコールバック関数を設定する。この関数は、以下のように
// 実装する必要がある。
//
// 1. URL は、先頭のスラッシュが省かれたファイル名です。例えば、"GET / HTTP/1.0"
//    という風にリクエストされた時、URL には""(空文字)が入り、"GET /hoge HTTP/1.0"
//    の時には、"hoge"が入ります。
// 2. リクエストされたページが見つかったら、httpd_send() または、httpd_send_head()
//    とhttpd_send_data() の組を呼び出し、データを出力する。
// 3. httpd_send_file を指定すると、httpd/ 以下にあるファイルを出力する。ファイルに
//    空文字が指定された時は、index.htmlが指定されたものとみなされる。



char* httpd_get_value(struct httpd_session_data* sd,const char* val);

// リクエストされたアドレスに渡されたフォームデータのうち、該当する文字列を返す。
// 例えば、"GET /status/graph?image=users HTTP/1.0"というリクエストの場合、
// httpd_get_value(sd,"image"); は、 "users"を返す。この関数の戻り値は、呼び出し元が
// 解放しなければならない。また、該当する文字列が無い時は、空の文字列を返す。

unsigned int httpd_get_ip(struct httpd_session_data *sd);

// クライアントのIPを返す。


void httpd_default_page(void(*httpd_func)(struct httpd_session_data* sd,const char* url));

// 指定されたURL が登録されていない時に呼び出す関数を設定する。この関数を呼び出さないか、
// 関数の引数にNULLを指定すると、404 Not Found を返す。




void httpd_send(struct httpd_session_data* sd,int status,const char *content_type,int content_len,const void *data);

//	HTTPヘッダ、データを組にして送信する。この関数を呼び出した後に、httpd_send_data を
//  呼び出してはならない。
// 
//		sd           : httpd_set_parse_func() に渡されたものをそのまま渡すこと。
//		status       : HTTPヘッダに加えるstatus。通常は200。
//		content_type : 送信するデータのタイプ。text/html , image/jpeg など。
//		content_len  : 送信するデータの長さ。
//		data         : 送信するデータへのポインタ



void httpd_send_head(struct httpd_session_data* sd,int status,const char *content_type,int content_len);

//	HTTPヘッダを送信する。
//
//		sd           : 同上
//		status       : 同上
//		content_type : 同上
//      content_len  : content_lenを-1に指定することで、この関数が呼ばれた時点で
//                     長さが分からないデータを送信することができる。この場合は
//                     強制的にHTTP/1.0 接続となり、オーバーヘッドが大きくなるので、
//                     あまりお勧めはしない。




void httpd_send_data(struct httpd_session_data* sd,int content_len,const void *data);

// データを送信する。この関数を、httpd_send_head() を呼び出す前に呼び出された場合、
// content_type = application/octet-stream, content_len = -1 としてヘッダが送信される。
//		sd           : 同上
//      content_len  : 送信するデータのdata長さを指定する。
//      data         : 送信するデータ



void httpd_send_file(struct httpd_session_data* sd,const char* url);

// ファイルを送信する。この関数は、httpd_send_head() を呼び出す前に呼び出さなければ
// ならない。ファイルに空文字が指定されたときは、index.htmlが指定されたと見なされる。



void httpd_send_error(struct httpd_session_data* sd,int status);

// HTTPエラーメッセージを送信する。status はHTTPのエラーコードと同じ。
// 	400 Bad Request, 404 Not Found, 500 Internal Server Error など。

int  httpd_parse(int fd);

// 初期化処理
void do_init_httpd(void);
void do_final_httpd(void);

#endif
