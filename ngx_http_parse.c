
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include "ngx_http_request.h"
#include "event_lea.h"
#include "http.h"
#define ngx_hash(key, c) ((int) key*31 + c)
static uint32_t  usual[] = {
    0xffffdbfe, /* 1111 1111 1111 1111  1101 1011 1111 1110 */

                /* ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!  */
    0x7fff37d6, /* 0111 1111 1111 1111  0011 0111 1101 0110 */

                /* _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@ */
#if (NGX_WIN32)
    0xefffffff, /* 1110 1111 1111 1111  1111 1111 1111 1111 */
#else
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
#endif

                /*  ~}| {zyx wvut srqp  onml kjih gfed cba` */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */

    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff, /* 1111 1111 1111 1111  1111 1111 1111 1111 */
    0xffffffff  /* 1111 1111 1111 1111  1111 1111 1111 1111 */
};


#if (NGX_HAVE_LITTLE_ENDIAN && NGX_HAVE_NONALIGNED)

#define ngx_str3_cmp(m, c0, c1, c2, c3)                                       \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define ngx_str3Ocmp(m, c0, c1, c2, c3)                                       \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define ngx_str4cmp(m, c0, c1, c2, c3)                                        \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define ngx_str5cmp(m, c0, c1, c2, c3, c4)                                    \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)             \
        && m[4] == c4

#define ngx_str6cmp(m, c0, c1, c2, c3, c4, c5)                                \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)             \
        && (((uint32_t *) m)[1] & 0xffff) == ((c5 << 8) | c4)

#define ngx_str7_cmp(m, c0, c1, c2, c3, c4, c5, c6, c7)                       \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)             \
        && ((uint32_t *) m)[1] == ((c7 << 24) | (c6 << 16) | (c5 << 8) | c4)

#define ngx_str8cmp(m, c0, c1, c2, c3, c4, c5, c6, c7)                        \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)             \
        && ((uint32_t *) m)[1] == ((c7 << 24) | (c6 << 16) | (c5 << 8) | c4)

#define ngx_str9cmp(m, c0, c1, c2, c3, c4, c5, c6, c7, c8)                    \
    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)             \
        && ((uint32_t *) m)[1] == ((c7 << 24) | (c6 << 16) | (c5 << 8) | c4)  \
        && m[8] == c8

#else /* !(NGX_HAVE_LITTLE_ENDIAN && NGX_HAVE_NONALIGNED) */

#define ngx_str3_cmp(m, c0, c1, c2, c3)                                       \
    m[0] == c0 && m[1] == c1 && m[2] == c2

#define ngx_str3Ocmp(m, c0, c1, c2, c3)                                       \
    m[0] == c0 && m[2] == c2 && m[3] == c3

#define ngx_str4cmp(m, c0, c1, c2, c3)                                        \
    m[0] == c0 && m[1] == c1 && m[2] == c2 && m[3] == c3

#define ngx_str5cmp(m, c0, c1, c2, c3, c4)                                    \
    m[0] == c0 && m[1] == c1 && m[2] == c2 && m[3] == c3 && m[4] == c4

#define ngx_str6cmp(m, c0, c1, c2, c3, c4, c5)                                \
    m[0] == c0 && m[1] == c1 && m[2] == c2 && m[3] == c3                      \
        && m[4] == c4 && m[5] == c5

#define ngx_str7_cmp(m, c0, c1, c2, c3, c4, c5, c6, c7)                       \
    m[0] == c0 && m[1] == c1 && m[2] == c2 && m[3] == c3                      \
        && m[4] == c4 && m[5] == c5 && m[6] == c6

#define ngx_str8cmp(m, c0, c1, c2, c3, c4, c5, c6, c7)                        \
    m[0] == c0 && m[1] == c1 && m[2] == c2 && m[3] == c3                      \
        && m[4] == c4 && m[5] == c5 && m[6] == c6 && m[7] == c7

#define ngx_str9cmp(m, c0, c1, c2, c3, c4, c5, c6, c7, c8)                    \
    m[0] == c0 && m[1] == c1 && m[2] == c2 && m[3] == c3                      \
        && m[4] == c4 && m[5] == c5 && m[6] == c6 && m[7] == c7 && m[8] == c8

#endif


/* gcc, icc, msvc and others compile these switches as an jump table */

int
ngx_http_parse_request_line(request_t *r, lt_buffer_t *b)
{
    char  c, ch, *p, *m;
    enum {
        sw_start = 0,
        sw_method,
        sw_spaces_before_uri,
        sw_schema,
        sw_schema_slash,
        sw_schema_slash_slash,
        sw_host_start,
        sw_host,
        sw_host_end,
        sw_host_ip_literal,
        sw_port,
        sw_host_http_09,
        sw_after_slash_in_uri,
        sw_check_uri,
        sw_check_uri_http_09,
        sw_uri,
        sw_http_09,
        sw_http_H,
        sw_http_HT,
        sw_http_HTT,
        sw_http_HTTP,
        sw_first_major_digit,
        sw_major_digit,
        sw_first_minor_digit,
        sw_minor_digit,
        sw_spaces_after_digit,
        sw_almost_done
    } state;

    state = r->state;

    for (p = b->pos; p < b->last; p++) {
        ch = *p;

        switch (state) {

        /* HTTP methods: GET, HEAD, POST */
        case sw_start:
            r->request_start = p;

            if (ch == '\r' || ch == '\n') {
                break;
            }

            if ((ch < 'A' || ch > 'Z') && ch != '_') {
                return NGX_HTTP_PARSE_INVALID_METHOD;
            }

            state = sw_method;
            break;

        case sw_method:
            if (ch == ' ') {
                r->method_end = p - 1;
                m = r->request_start;

                switch (p - m) {

                case 3:
                    if (ngx_str3_cmp(m, 'G', 'E', 'T', ' ')) {
                        r->method = NGX_HTTP_GET;
                        break;
                    }

                    if (ngx_str3_cmp(m, 'P', 'U', 'T', ' ')) {
                        r->method = NGX_HTTP_PUT;
                        break;
                    }

                    break;

                case 4:
                    if (m[1] == 'O') {

                        if (ngx_str3Ocmp(m, 'P', 'O', 'S', 'T')) {
                            r->method = NGX_HTTP_POST;
                            break;
                        }

                        if (ngx_str3Ocmp(m, 'C', 'O', 'P', 'Y')) {
                            r->method = NGX_HTTP_COPY;
                            break;
                        }

                        if (ngx_str3Ocmp(m, 'M', 'O', 'V', 'E')) {
                            r->method = NGX_HTTP_MOVE;
                            break;
                        }

                        if (ngx_str3Ocmp(m, 'L', 'O', 'C', 'K')) {
                            r->method = NGX_HTTP_LOCK;
                            break;
                        }

                    } else {

                        if (ngx_str4cmp(m, 'H', 'E', 'A', 'D')) {
                            r->method = NGX_HTTP_HEAD;
                            break;
                        }
                    }

                    break;

                case 5:
                    if (ngx_str5cmp(m, 'M', 'K', 'C', 'O', 'L')) {
                        r->method = NGX_HTTP_MKCOL;
                        break;
                    }

                    if (ngx_str5cmp(m, 'P', 'A', 'T', 'C', 'H')) {
                        r->method = NGX_HTTP_PATCH;
                        break;
                    }

                    if (ngx_str5cmp(m, 'T', 'R', 'A', 'C', 'E')) {
                        r->method = NGX_HTTP_TRACE;
                        break;
                    }

                    break;

                case 6:
                    if (ngx_str6cmp(m, 'D', 'E', 'L', 'E', 'T', 'E')) {
                        r->method = NGX_HTTP_DELETE;
                        break;
                    }

                    if (ngx_str6cmp(m, 'U', 'N', 'L', 'O', 'C', 'K')) {
                        r->method = NGX_HTTP_UNLOCK;
                        break;
                    }

                    break;

                case 7:
                    if (ngx_str7_cmp(m, 'O', 'P', 'T', 'I', 'O', 'N', 'S', ' '))
                    {
                        r->method = NGX_HTTP_OPTIONS;
                    }

                    break;

                case 8:
                    if (ngx_str8cmp(m, 'P', 'R', 'O', 'P', 'F', 'I', 'N', 'D'))
                    {
                        r->method = NGX_HTTP_PROPFIND;
                    }

                    break;

                case 9:
                    if (ngx_str9cmp(m,
                            'P', 'R', 'O', 'P', 'P', 'A', 'T', 'C', 'H'))
                    {
                        r->method = NGX_HTTP_PROPPATCH;
                    }

                    break;
                }

                state = sw_spaces_before_uri;
                break;
            }

            if ((ch < 'A' || ch > 'Z') && ch != '_') {
                return NGX_HTTP_PARSE_INVALID_METHOD;
            }

            break;

        /* space* before URI */
        case sw_spaces_before_uri:

            if (ch == '/') {
                r->uri_start = p;
                state = sw_after_slash_in_uri;
                break;
            }

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'z') {
                r->schema_start = p;
                state = sw_schema;
                break;
            }

            switch (ch) {
            case ' ':
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_schema:

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'z') {
                break;
            }

            switch (ch) {
            case ':':
                r->schema_end = p;
                state = sw_schema_slash;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_schema_slash:
            switch (ch) {
            case '/':
                state = sw_schema_slash_slash;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_schema_slash_slash:
            switch (ch) {
            case '/':
                state = sw_host_start;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_host_start:

            r->host_start = p;

            if (ch == '[') {
                state = sw_host_ip_literal;
                break;
            }

            state = sw_host;

            /* fall through */

        case sw_host:

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'z') {
                break;
            }

            if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') {
                break;
            }

            /* fall through */

        case sw_host_end:

            r->host_end = p;

            switch (ch) {
            case ':':
                state = sw_port;
                break;
            case '/':
                r->uri_start = p;
                state = sw_after_slash_in_uri;
                break;
            case ' ':
                /*
                 * use single "/" from request line to preserve pointers,
                 * if request line will be copied to large client buffer
                 */
                r->uri_start = r->schema_end + 1;
                r->uri_end = r->schema_end + 2;
                state = sw_host_http_09;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_host_ip_literal:

            if (ch >= '0' && ch <= '9') {
                break;
            }

            c = (u_char) (ch | 0x20);
            if (c >= 'a' && c <= 'z') {
                break;
            }

            switch (ch) {
            case ':':
                break;
            case ']':
                state = sw_host_end;
                break;
            case '-':
            case '.':
            case '_':
            case '~':
                /* unreserved */
                break;
            case '!':
            case '$':
            case '&':
            case '\'':
            case '(':
            case ')':
            case '*':
            case '+':
            case ',':
            case ';':
            case '=':
                /* sub-delims */
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_port:
            if (ch >= '0' && ch <= '9') {
                break;
            }

            switch (ch) {
            case '/':
                r->port_end = p;
                r->uri_start = p;
                state = sw_after_slash_in_uri;
                break;
            case ' ':
                r->port_end = p;
                /*
                 * use single "/" from request line to preserve pointers,
                 * if request line will be copied to large client buffer
                 */
                r->uri_start = r->schema_end + 1;
                r->uri_end = r->schema_end + 2;
                state = sw_host_http_09;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* space+ after "http://host[:port] " */
        case sw_host_http_09:
            switch (ch) {
            case ' ':
                break;
            case '\r':
                r->http_minor = 9;
                state = sw_almost_done;
                break;
            case '\n':
                r->http_minor = 9;
                goto done;
            case 'H':
                r->http_protocol.data = p;
                state = sw_http_H;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;


        /* check "/.", "//", "%", and "\" (Win32) in URI */
        case sw_after_slash_in_uri:

            if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
                state = sw_check_uri;
                break;
            }

            switch (ch) {
            case ' ':
                r->uri_end = p;
                state = sw_check_uri_http_09;
                break;
            case '\r':
                r->uri_end = p;
                r->http_minor = 9;
                state = sw_almost_done;
                break;
            case '\n':
                r->uri_end = p;
                r->http_minor = 9;
                goto done;
            case '.':
                r->complex_uri = 1;
                state = sw_uri;
                break;
            case '%':
                r->quoted_uri = 1;
                state = sw_uri;
                break;
            case '/':
                r->complex_uri = 1;
                state = sw_uri;
                break;
#if (NGX_WIN32)
            case '\\':
                r->complex_uri = 1;
                state = sw_uri;
                break;
#endif
            case '?':
                r->args_start = p + 1;
                state = sw_uri;
                break;
            case '#':
                r->complex_uri = 1;
                state = sw_uri;
                break;
            case '+':
                r->plus_in_uri = 1;
                break;
            case '\0':
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            default:
                state = sw_check_uri;
                break;
            }
            break;

        /* check "/", "%" and "\" (Win32) in URI */
        case sw_check_uri:

            if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
                break;
            }

            switch (ch) {
            case '/':
#if (NGX_WIN32)
                if (r->uri_ext == p) {
                    r->complex_uri = 1;
                    state = sw_uri;
                    break;
                }
#endif
                r->uri_ext = NULL;
                state = sw_after_slash_in_uri;
                break;
            case '.':
                r->uri_ext = p + 1;
                break;
            case ' ':
                r->uri_end = p;
                state = sw_check_uri_http_09;
                break;
            case '\r':
                r->uri_end = p;
                r->http_minor = 9;
                state = sw_almost_done;
                break;
            case '\n':
                r->uri_end = p;
                r->http_minor = 9;
                goto done;
#if (NGX_WIN32)
            case '\\':
                r->complex_uri = 1;
                state = sw_after_slash_in_uri;
                break;
#endif
            case '%':
                r->quoted_uri = 1;
                state = sw_uri;
                break;
            case '?':
                r->args_start = p + 1;
                state = sw_uri;
                break;
            case '#':
                r->complex_uri = 1;
                state = sw_uri;
                break;
            case '+':
                r->plus_in_uri = 1;
                break;
            case '\0':
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* space+ after URI */
        case sw_check_uri_http_09:
            switch (ch) {
            case ' ':
                break;
            case '\r':
                r->http_minor = 9;
                state = sw_almost_done;
                break;
            case '\n':
                r->http_minor = 9;
                goto done;
            case 'H':
                r->http_protocol.data = p;
                state = sw_http_H;
                break;
            default:
                r->space_in_uri = 1;
                state = sw_check_uri;
                p--;
                break;
            }
            break;


        /* URI */
        case sw_uri:

            if (usual[ch >> 5] & (1 << (ch & 0x1f))) {
                break;
            }

            switch (ch) {
            case ' ':
                r->uri_end = p;
                state = sw_http_09;
                break;
            case '\r':
                r->uri_end = p;
                r->http_minor = 9;
                state = sw_almost_done;
                break;
            case '\n':
                r->uri_end = p;
                r->http_minor = 9;
                goto done;
            case '#':
                r->complex_uri = 1;
                break;
            case '\0':
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* space+ after URI */
        case sw_http_09:
            switch (ch) {
            case ' ':
                break;
            case '\r':
                r->http_minor = 9;
                state = sw_almost_done;
                break;
            case '\n':
                r->http_minor = 9;
                goto done;
            case 'H':
                r->http_protocol.data = p;
                state = sw_http_H;
                break;
            default:
                r->space_in_uri = 1;
                state = sw_uri;
                p--;
                break;
            }
            break;

        case sw_http_H:
            switch (ch) {
            case 'T':
                state = sw_http_HT;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HT:
            switch (ch) {
            case 'T':
                state = sw_http_HTT;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HTT:
            switch (ch) {
            case 'P':
                state = sw_http_HTTP;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        case sw_http_HTTP:
            switch (ch) {
            case '/':
                state = sw_first_major_digit;
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* first digit of major HTTP version */
        case sw_first_major_digit:
            if (ch < '1' || ch > '9') {
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_major = ch - '0';
            state = sw_major_digit;
            break;

        /* major HTTP version or dot */
        case sw_major_digit:
            if (ch == '.') {
                state = sw_first_minor_digit;
                break;
            }

            if (ch < '0' || ch > '9') {
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_major = r->http_major * 10 + ch - '0';
            break;

        /* first digit of minor HTTP version */
        case sw_first_minor_digit:
            if (ch < '0' || ch > '9') {
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_minor = ch - '0';
            state = sw_minor_digit;
            break;

        /* minor HTTP version or end of request line */
        case sw_minor_digit:
            if (ch == '\r') {
                state = sw_almost_done;
                break;
            }

            if (ch == '\n') {
                goto done;
            }

            if (ch == ' ') {
                state = sw_spaces_after_digit;
                break;
            }

            if (ch < '0' || ch > '9') {
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }

            r->http_minor = r->http_minor * 10 + ch - '0';
            break;

        case sw_spaces_after_digit:
            switch (ch) {
            case ' ':
                break;
            case '\r':
                state = sw_almost_done;
                break;
            case '\n':
                goto done;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
            break;

        /* end of request line */
        case sw_almost_done:
            r->request_end = p - 1;
            switch (ch) {
            case '\n':
                goto done;
            default:
                return NGX_HTTP_PARSE_INVALID_REQUEST;
            }
        }
    }

    b->pos = p;
    r->state = state;

    return LAGAIN;

done:

    b->pos = p + 1;

    if (r->request_end == NULL) {
        r->request_end = p;
    }

    r->http_version = r->http_major * 1000 + r->http_minor;
    r->state = sw_start;

    if (r->http_version == 9 && r->method != NGX_HTTP_GET) {
        return NGX_HTTP_PARSE_INVALID_09_METHOD;
    }

    return 0;
}


int
ngx_http_parse_header_line(request_t *r, lt_buffer_t *b,
    int allow_underscores)
{
    char      c, ch, *p;
    int  hash, i;
    enum {
        sw_start = 0,
        sw_name,
        sw_space_before_value,
        sw_value,
        sw_space_after_value,
        sw_ignore_line,
        sw_almost_done,
        sw_header_almost_done
    } state;

    /* the last '\0' is not needed because string is zero terminated */

    static char  lowcase[] =
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0" "0123456789\0\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0abcdefghijklmnopqrstuvwxyz\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    state = r->state;
    hash = r->header_hash;
    i = r->lowcase_index;

    for (p = b->pos; p < b->last; p++) {
        ch = *p;

        switch (state) {

        /* first char */
        case sw_start:
            r->header_name_start = p;
            r->invalid_header = 0;

            switch (ch) {
            case '\r':
                r->header_end = p;
                state = sw_header_almost_done;
                break;
            case '\n':
                r->header_end = p;
                goto header_done;
            default:
                state = sw_name;

                c = lowcase[(unsigned char)ch];

                if (c) {
                    hash = ngx_hash(0, c);//TODO
                    r->lowcase_header[0] = c;
                    i = 1;
                    break;
                }

                if (ch == '_') {
                    if (allow_underscores) {
                        hash = ngx_hash(0, ch);
                        r->lowcase_header[0] = ch;
                        i = 1;

                    } else {
                        r->invalid_header = 1;
                    }

                    break;
                }

                if (ch == '\0') {
                    return NGX_HTTP_PARSE_INVALID_HEADER;
                }

                r->invalid_header = 1;

                break;

            }
            break;

        /* header name */
        case sw_name:
            c = lowcase[(unsigned char)ch];

            if (c) {
                hash = ngx_hash(hash, c);
                r->lowcase_header[i++] = c;
                i &= (NGX_HTTP_LC_HEADER_LEN - 1);
                break;
            }

            if (ch == '_') {
                if (allow_underscores) {
                    hash = ngx_hash(hash, ch);
                    r->lowcase_header[i++] = ch;
                    i &= (NGX_HTTP_LC_HEADER_LEN - 1);

                } else {
                    r->invalid_header = 1;
                }

                break;
            }

            if (ch == ':') {
                r->header_name_end = p;
                state = sw_space_before_value;
                break;
            }

            if (ch == '\r') {
                r->header_name_end = p;
                r->header_start = p;
                r->header_end = p;
                state = sw_almost_done;
                break;
            }

            if (ch == '\n') {
                r->header_name_end = p;
                r->header_start = p;
                r->header_end = p;
                goto done;
            }

            /* IIS may send the duplicate "HTTP/1.1 ..." lines */
            if (ch == '/'
                && r->upstream
                && p - r->header_name_start == 4
                && strncmp(r->header_name_start, "HTTP", 4) == 0)
            {
                state = sw_ignore_line;
                break;
            }

            if (ch == '\0') {
                return NGX_HTTP_PARSE_INVALID_HEADER;
            }

            r->invalid_header = 1;

            break;

        /* space* before header value */
        case sw_space_before_value:
            switch (ch) {
            case ' ':
                break;
            case '\r':
                r->header_start = p;
                r->header_end = p;
                state = sw_almost_done;
                break;
            case '\n':
                r->header_start = p;
                r->header_end = p;
                goto done;
            case '\0':
                return NGX_HTTP_PARSE_INVALID_HEADER;
            default:
                r->header_start = p;
                state = sw_value;
                break;
            }
            break;

        /* header value */
        case sw_value:
            switch (ch) {
            case ' ':
                r->header_end = p;
                state = sw_space_after_value;
                break;
            case '\r':
                r->header_end = p;
                state = sw_almost_done;
                break;
            case '\n':
                r->header_end = p;
                goto done;
            case '\0':
                return NGX_HTTP_PARSE_INVALID_HEADER;
            }
            break;

        /* space* before end of header line */
        case sw_space_after_value:
            switch (ch) {
            case ' ':
                break;
            case '\r':
                state = sw_almost_done;
                break;
            case '\n':
                goto done;
            case '\0':
                return NGX_HTTP_PARSE_INVALID_HEADER;
            default:
                state = sw_value;
                break;
            }
            break;

        /* ignore header line */
        case sw_ignore_line:
            switch (ch) {
            case '\n':
                state = sw_start;
                break;
            default:
                break;
            }
            break;

        /* end of header line */
        case sw_almost_done:
            switch (ch) {
            case '\n':
                goto done;
            case '\r':
                break;
            default:
                return NGX_HTTP_PARSE_INVALID_HEADER;
            }
            break;

        /* end of header */
        case sw_header_almost_done:
            switch (ch) {
            case '\n':
                goto header_done;
            default:
                return NGX_HTTP_PARSE_INVALID_HEADER;
            }
        }
    }

    b->pos = p;
    r->state = state;
    r->header_hash = hash;
    r->lowcase_index = i;

    return LAGAIN;

done:

    b->pos = p + 1;
    r->state = sw_start;
    r->header_hash = hash;
    r->lowcase_index = i;

    return 0;

header_done:

    b->pos = p + 1;
    r->state = sw_start;

    return NGX_HTTP_PARSE_HEADER_DONE;
}



