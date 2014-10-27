#include "http.h"
int http_process_host(request_t *req, lt_string_t *host/*, int off*/)
{
    enum {
        sw_usual = 0,
        sw_literal,
        sw_rest
    } state;

    state = sw_usual;

    size_t dot_pos = host->length;
    size_t host_len = host->length;

    char *h = host->data;
    
    char ch;
    for (int i = 0; i < host->length; i++) {
        ch = h[i];
        switch(ch) {
            case '.':
                if (dot_pos == i - 1) {
                    return LERROR;
                }
                dot_pos = i;
                break;
            case ':':
                if (state == sw_usual) {
                    host_len = i;
                    state = sw_rest;
                }
                break;
            case '[':
                if (i == 0) {
                    state = sw_literal;
                }
                break;
            case ']':
                if (state == sw_literal) {
                    host_len = i + 1;
                    state = sw_rest;
                }
                break;
            case '\0':
                return LERROR;

            default:
                if (ch == '/') {
                    return LERROR;
                }
                break;
        }

    }

    if (dot_pos == host_len - 1) {
        host_len--;
    }

    if (host_len == 0) {
        return LERROR;
    }

    lowcase_key_copy_from_origin(host, host);

    host->length  = host_len;
    return LOK;
}

