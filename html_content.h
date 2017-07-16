#ifndef HTML_CONTENT_H
#define HTML_CONTENT_H

#include "def1.h"
#include "tools.h"

#define MAX_HTML_SIZE 65536
#define TIMEOUT_SEC 3
#define FILE_NAME_SIZE 50

#define CHUNKED 0x01
#define DEFLATE 0x02
#define GZIP 0x04
#define HEAD_COMPLETED 0x80
#define IS_IMAG 0x40
#define IS_TEXT 0x20

#define NEXT_LOC(x) ((x + buf_start) % MAX_HTML_SIZE)

class html_content {
private:
    char content[MAX_HTML_SIZE];
    char img_format[8];
    int buf_start, buf_len;
	/** Weather this http is chunked or compressed */
	u_int8_t flags;
	/** if the http is chunked, content_len is useless */
	u_int32_t content_len;
	volatile bool updated;
	pthread_mutex_t html_lock;

public:
    const u_int32_t	tcp_ack;

    html_content(const u_int32_t ack);
    ~html_content() { pthread_mutex_destroy(&html_lock); };
    void add(const u_char *p, int steps);
    int write2file();
    bool no_compression() {
        return (flags & HEAD_COMPLETED
            && !(flags & (DEFLATE | GZIP)));
    };
    int process_head();
    int process_tail();
    void p_html() {
        pthread_mutex_lock(&html_lock);
        updated = true;
        pthread_mutex_unlock(&html_lock);
    };
    bool v_html() {
        pthread_mutex_lock(&html_lock);
        if(!updated) {
            pthread_mutex_unlock(&html_lock);
            return false;
        }
        updated = false;
        pthread_mutex_unlock(&html_lock);
        return true;
    };
};

html_content* find_html(const u_int32_t ack);
html_content* find_html();
html_content* insert_html(const u_int32_t ack);
html_content* delete_html(const u_int32_t ack);


#endif

