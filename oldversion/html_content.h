#ifndef HTML_CONTENT_H
#define HTML_CONTENT_H

#include "def1.h"
#include "min_heap.h"
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

class html_content{
private:
    char content[MAX_HTML_SIZE];
	min_heap <u_int32_t> *seqs;
	u_int32_t content_len, html_len;
	/** Weather this http is chunked or compressed */
	u_int8_t flags;
	char img_format[8];
public:
    const u_int32_t	tcp_ack;

    html_content(const u_int32_t ack);
    ~html_content() { if(!seqs->empty()) delete seqs; };
    /** add(): using min_heap */
    void add(const u_char *p, int steps, u_int32_t this_seq);
    /** read(): write to file without considering
      * disorderly or repeatedly receiving of packets */
    void read(const u_char *p, int steps);
    void write2file();
    bool no_compression() {
        return (flags & HEAD_COMPLETED
            && !(flags & (DEFLATE | GZIP)));
    };
    int process_head();
    int process_tail();
};

html_content* find_html(const u_int32_t ack);
html_content* insert_html(const u_int32_t ack);
html_content* delete_html(const u_int32_t ack);


#endif

