#ifndef HTML_CONTENT_CC
#define HTML_CONTENT_CC

#include "html_content.h"

/**    Globe html list     */
list<html_content *> html;

html_content* find_html(const u_int32_t ack) {
    for(list<html_content *>::iterator it = html.begin();
        it != html.end(); it++) {

        if((*it)->tcp_ack == ack)
            return *it;
    }

    return NULL;
}

html_content* insert_html(const u_int32_t ack) {
    html_content *p = new html_content(ack);
    html.push_back(p);
    return p;
}

html_content* delete_html(const u_int32_t ack) {
    html_content *p = NULL;

    for(list<html_content *>::iterator it = html.begin();
        it != html.end(); ) {

        if((*it)->tcp_ack == ack) {
            p = *it;
            it = html.erase(it);
            break;
        }
        it++;
    }

    return p;
}
/***************************/

html_content::html_content(const u_int32_t ack): tcp_ack(ack) {
    memset(content, (int)'\0', MAX_HTML_SIZE);
    memset(img_format, (int)'\0', 8);
    content_len = 0;
    html_len = 0;
    flags = 0;
    seqs = new min_heap <u_int32_t>;
}

void html_content::add(const u_char *p, const int steps,
        u_int32_t this_seq) {

    int start;
    /** this packet is repeatly received */
    if(seqs->has_elem(this_seq)) {
        cout << "seqs->has_elem(this_seq)" << endl;
        return;
    }

    if(content_len + steps > MAX_HTML_SIZE) {
        cout  << tcp_ack << " add(): html content is overflow ! size: "
            << content_len << " + " << steps << endl;
        return;
    }

    if(seqs->empty() || this_seq < seqs->min_elem())
        start = 0;
    else // this_seq > seqs.min_elem()
        start = this_seq - seqs->min_elem();

    for(int i = content_len - 1; i >= start; i--)
        content[i + steps] = content[i];
    for(int i = start; i < start + steps; i++)
        content[i] = *p++;

    content_len += steps;
    seqs->push(this_seq);
}

void html_content::read(const u_char *p, int steps) {

}

int html_content::process_head() {
    if(!has_str((u_char *)content, "HTTP"))
        return -1;

    u_char *p = (u_char *)content + strlen("HTTP/1.1 200 OK\r\n");
    int head_len = 0;

    while(p) {

        if(*p == '\r' && has_str(p, "\r\n\r\n")) {
            head_len += strlen("\r\n\r\n");
            break;
        }

        /** If use chunked */
        if(*p == 'T' && has_str(p, "Transfer-Encoding: chunked")) {
            p += strlen("Transfer-Encoding: chunked");
            head_len += strlen("Transfer-Encoding: chunked");
            flags |= CHUNKED;
            continue;
        }

        if(*p == 'C' && has_str(p, "Content-")) {
            p += strlen("Content-");
            head_len += strlen("Content-");

        /** Content-Length = 0: maybe response packet */
            if(has_str(p, "Length")) {
                p += strlen("Length: ");
                head_len += strlen("Length: ");
                html_len = get_num(p, '\r');
                if(html_len == 0)
                    return -2;
            }

            if(has_str(p, "Encoding")) {
                p += strlen("Encoding: ");
                head_len += strlen("Encoding: ");
                if(has_str(p, "deflate")) flags |= DEFLATE;
                if(has_str(p, "gzip")) flags |= GZIP;
            }

            if(has_str(p, "Type")) {
                p += strlen("Type: ");
                head_len += strlen("Type: ");

                if(has_str(p, "text/html")) flags |= IS_TEXT;
                if(has_str(p, "image")) {
                    flags |= IS_IMAG;
                    p += strlen("image/");
                    head_len += strlen("image/");

                    for(int i = 0; ; i++) {
                        if(i >= 8) {
                            cout << "image format >= 8" << endl;
                            return -2;
                        }

                        if(*(p + i) == '\r' || *(p + i) == ';') {
                            img_format[i] = '\0';
                            break;
                        }
                        img_format[i] = *(p + i);
                    }
                }
            }
            continue;
        }

        p++;
        head_len++;
    }

    /** Current version: we only get image file! */
    if(!(flags & IS_IMAG)) return -2;

    if(*p == '\r') {
        flags |= HEAD_COMPLETED;

        for(int i = 0; i < head_len; i++)
            content[i] = content[i + head_len];
        content_len -= head_len;

        return 0;
    }
    return -1;
}

int html_content::process_tail() {

    if(!(flags & HEAD_COMPLETED)) return -1;

    if(!(flags & CHUNKED) && content_len >= html_len)
        return 0;
    if(flags & CHUNKED) {
        char *p = &content[content_len - 20];
        for(int i = 0; i < 20; i++) {
            if(has_str((u_char *)p, "\r\n0\r\n"))
                return 0;
            p++;
        }
    }
    return -1;
}

void html_content::write2file() {
    char buf[FILE_NAME_SIZE];
    memset(buf, (int)'\0', FILE_NAME_SIZE);
    int imgfd;

    if(flags & IS_IMAG)
        sprintf(buf, "imgs/%ld.%s", tcp_ack, img_format);
    else
        return;

    if((imgfd = open(buf, O_WRONLY | O_APPEND | O_CREAT)) < 0) {
        cout << "cli: no such file ";
        fputs(buf, stdout);
        cout << endl;
        return;
    }

    chmod(buf, S_IROTH | S_IWOTH | S_IXOTH);
    write(imgfd, content, content_len);
}

#endif
