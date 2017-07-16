#ifndef HTML_CONTENT_CC
#define HTML_CONTENT_CC

#include "html_content.h"

list<html_content *> html;
pthread_mutex_t list_lock;

html_content* find_html(const u_int32_t ack) {
    pthread_mutex_lock(&list_lock);
    for(list<html_content *>::iterator it = html.begin();
        it != html.end(); it++) {

        if((*it)->tcp_ack == ack) {
            pthread_mutex_unlock(&list_lock);
            return *it;
        }
    }
    pthread_mutex_unlock(&list_lock);
    return NULL;
}

html_content* find_html() {
    pthread_mutex_lock(&list_lock);
    for(list<html_content *>::iterator it = html.begin();
        it != html.end(); it++) {

        if((*it)->v_html()) {
            pthread_mutex_unlock(&list_lock);
            return *it;
        }
    }
    pthread_mutex_unlock(&list_lock);
    return NULL;
}

html_content* insert_html(const u_int32_t ack) {
    pthread_mutex_lock(&list_lock);
    html_content *p = new html_content(ack);
    html.push_back(p);
    pthread_mutex_unlock(&list_lock);
    return p;
}

html_content* delete_html(const u_int32_t ack) {
    pthread_mutex_lock(&list_lock);

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
    pthread_mutex_unlock(&list_lock);
    return p;
}
/***************************/

html_content::html_content(const u_int32_t ack): tcp_ack(ack) {
    pthread_mutex_init(&html_lock, NULL);
    pthread_mutex_lock(&html_lock);
    memset(content, (int)'\0', MAX_HTML_SIZE);
    memset(img_format, (int)'\0', 8);
    buf_start = 0, buf_len = 0;
    flags = 0x00;
    content_len = 0;
    updated = false;
    pthread_mutex_unlock(&html_lock);
}

void html_content::add(const u_char *p, const int steps) {

    if(steps + buf_len > MAX_HTML_SIZE) {
        cout  << tcp_ack << " add(): html content is overflow ! size: "
            << buf_len << " + " << steps << endl;
        return;
    }

    pthread_mutex_lock(&html_lock);

    for(int i = 0; i < steps; i++)
        content[NEXT_LOC(buf_len + i)] = *p++;
    buf_len += steps;

    if(flags & HEAD_COMPLETED && !(flags & CHUNKED))
        content_len -= steps;

    pthread_mutex_unlock(&html_lock);
}

/** !!! DANGER: if buf_start is next to MAX_HTML_SIZE
  * has_str() and get_num() may cause error! */
int html_content::process_head() {
    pthread_mutex_lock(&html_lock);

    if(flags & HEAD_COMPLETED) {
        pthread_mutex_unlock(&html_lock);
        return 0;
    }
//    if(!has_str((u_char *) &content[buf_start], "HTTP/1.1 200 OK\r\n"))
//        return -2;

    u_char *p = (u_char *) &content[buf_start]
        + strlen("HTTP/1.1 200 OK\r\n");

    int head_len = strlen("HTTP/1.1 200 OK\r\n");
    while(p) {

        if(*p == '\r' && has_str(p, "\r\n\r\n")) {
            head_len += strlen("\r\n\r\n");
            flags |= HEAD_COMPLETED;
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
                content_len = get_num(p, '\r');
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
//                            return -2;
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

    if(!(flags & HEAD_COMPLETED)) {
        pthread_mutex_unlock(&html_lock);
        return -1;
    }

    /** Current version: we only get image file! */
    if(!(flags & IS_IMAG)) {
        pthread_mutex_unlock(&html_lock);
        return -2;
    }

    if(!content_len && !(flags & CHUNKED)) {
        pthread_mutex_unlock(&html_lock);
        return -2;
    }

    if(!(flags & CHUNKED))
        content_len -= buf_len;
    buf_start = NEXT_LOC(head_len);
    buf_len -= head_len;

    pthread_mutex_unlock(&html_lock);
    return 0;
}

int html_content::process_tail() {
    pthread_mutex_lock(&html_lock);

    if(!(flags & HEAD_COMPLETED)) {
        pthread_mutex_unlock(&html_lock);
        return -1;
    }

    if(!(flags & CHUNKED) && content_len <= 0) {
        pthread_mutex_unlock(&html_lock);
        return 0;
    }

    if(flags & CHUNKED) {
        char *p = &content[NEXT_LOC(buf_len - 20)];
        for(int i = 0; i < 20; i++) {
            if(has_str((u_char *)p, "\r\n0\r\n")) {
                pthread_mutex_unlock(&html_lock);
                return 0;
            }
            p++;
        }
    }
    pthread_mutex_unlock(&html_lock);
    return -1;
}

int html_content::write2file() {
    pthread_mutex_lock(&html_lock);

    char name[FILE_NAME_SIZE];
    memset(name, (int)'\0', FILE_NAME_SIZE);
    int imgfd, n;

    if(flags & IS_IMAG)
        sprintf(name, "imgs/%ld.%s", tcp_ack, img_format);
    else {
        pthread_mutex_unlock(&html_lock);
        return -1;
    }

    if((imgfd = open(name, O_WRONLY | O_APPEND | O_CREAT)) < 0) {
        cout << "write2file(): no such file ";
        fputs(name, stdout);
        cout << endl;
        pthread_mutex_unlock(&html_lock);
        return -1;
    }
    chmod(name, S_IROTH | S_IWOTH | S_IXOTH);

    while(buf_start + buf_len >= MAX_HTML_SIZE) {
        if((n = write(imgfd, &content[buf_start],
            MAX_HTML_SIZE - buf_start)) < 0) {
                pthread_mutex_unlock(&html_lock);
                return -1;
            }
        buf_start = (buf_start + n) % MAX_HTML_SIZE;
        buf_len -= n;
        /** now the buf_start should be 0 */
    }

    if(buf_len) {
        if((n = write(imgfd, &content[buf_start], buf_len)) < 0) {
            pthread_mutex_unlock(&html_lock);
            return -1;
        }
        buf_start += n;
        buf_len -= n;
    }
    close(imgfd);

    pthread_mutex_unlock(&html_lock);
    if(buf_len == 0)
        return 0;
    return -1;
}

#endif
