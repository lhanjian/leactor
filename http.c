#include "http.h"
#include "ngx_http_parse.h"
static int get_addrinfo_with_bind(http_t *http);
static int http_add_listen(http_t *http, conf_t *conf);
static int http_bind_listenfd_with_handle(http_t *http, conf_t *conf);
static int http_accept_distributor(event_t *ev, void* http);
/*
void 
ignore_sigpipe(void)
{
    struct sigaction act;
    memset(&act, 0, sizeof act);
    act.sa_handler = SIG_IGN;
    act.sa_flags = SA_RESTART;
    if(sigaction(SIGPIPE, &act, NULL)) {
        perror("sigaction");
    }
}
*/

http_t *http_master_new(base_t *base, conf_t *conf)
{
    http_t *http = calloc(1, sizeof(http_t));
    if (!http) {
        perror("malloc http");
        return NULL;
    }

    http->listen.bind_addr = NULL;//ALL available localaddr 
//TODO:sustitute with JSON conf file
    http->listen.bind_port = "80";//same to UP

    int rv = http_add_listen(http, conf);
    if (rv) {
        free(http);
        return NULL;
    }

    http->listen.ev = lt_io_add(http->base, http->listen.fd, 
            LV_LAG|LV_FDRD, http_accept_distributor/*TODO http_cb*/, 
            http/*TODO http_cb_args*/, INF);
    return http;
}

int send_to_child(int seq, int fd)
{
    uint64_t count = 1;
    int rv = write(seq, &count, sizeof(count));
    if (rv != sizeof(count)) {
        perror("eventfd write failed");
        return -1;
    }
    return rv;
}

int http_accept_distributor(event_t *ev, void *arg)
{
    http_t *http = (http_t *)arg;
    static unsigned long seq_count = 0;
    int rv = send_to_child(seq_count % http->core_amount, 
            http->listen.fd);
    if (rv == -1) {
        fprintf(stderr, "eventfd send failed");
    }

    seq_count++;

    return 0;
}

int get_addrinfo_with_bind(http_t *http)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv = getaddrinfo(http->listen.bind_addr, http->listen.bind_port, 
            &hints, &res);
    if (rv) {
        fprintf(stderr, "gai:%s\n", gai_strerror(rv));
        return -1;
    }

    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {
        int listen_sock = socket(p->ai_family, p->ai_socktype|SOCK_NONBLOCK, 
                p->ai_protocol);
        if (listen_sock == -1) {
            perror("listen socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, 
                    &yes, sizeof yes) == -1) {
            perror("setsockopt_REUSE_ADDR");
            return -1;
        }

        if (bind(listen_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(listen_sock);
            perror("bind error");
            continue;
        }
        http->listen.saddr = *p->ai_addr;
//        ignore_sigpipe();
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server:failed to bind");
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}

int send_listenfd_to_child(int pfd[2], int fd)
{
    close(pfd[0]);
    write(pfd[1], &fd, sizeof(fd));
    close(pfd[1]);

    return 0;
}

int http_add_listen(http_t *http, conf_t *conf)
{
    int rv = get_addrinfo_with_bind(http);
    if (rv) {
        fprintf(stderr, "get_addrinfo_with_bind error");
        return -1;
    }

    send_listenfd_to_child(conf->pfd, http->listen.fd);

    rv = listen(http->listen.fd, SOMAXCONN);
    if (rv) {
        fprintf(stderr, "listen error");
        return -1;
    }


    return 0;
}

void recv_listenfd_to_child(int pfd[2], int *fd)
{
    close(pfd[1]);
    read(pfd[0], fd, sizeof(int));
    close(pfd[0]);
}

int set_http_data_coming_timeout();

void http_create_request(connection_t *conn)
{
    request_t *req = 
        lt_alloc(conn->request_pool, &conn->request_pool_manager);

    req->header_in = conn->buf;
    req->state = 0;

    lt_new_memory_pool_manager(&req->header_pool_manager);
    req->header_pool = lt_new_memory_pool(sizeof(lt_http_header_element_t), 
                                                 req->header_pool);
}
/*
int http_read_request_header(request_t *req)
{
    ssize_t n = req->header_in->last - req->header_in->pos;
    if (n > 0) {
        return n;
    }

}
*/
int http_request_line_parsed(request_t *req, int rv)
{
    req->request_line.length = req->request_end - req->request_start;
    req->request_line.data = req->request_start;
    req->request_length = req->header_in->pos - req->request_start;

    req->method_name.length = req->method_end - req->request_start + 1;
    req->method_name.data = req->request_line.data;

    if (req->http_protocol.data) {
        req->http_protocol.length = req->request_end - req->http_protocol.data;
    }

    //process_request_uri
    if (req->args_start) {
        req->uri.length = req->args_start - 1 - req->uri_start;
    } else {
        req->uri.length = req->uri_end - req->uri_start;
    }

    if (req->complex_uri || req->quoted_uri) {
        //TODO
    } else {
        req->uri.data = req->uri_start;
    }

    req->unparsed_uri.length = req->uri_end - req->uri_start;
    req->unparsed_uri.data = req->uri_start;

    req->valid_unparsed_uri = req->space_in_uri ? 0 : 1;

    if (req->uri_ext) {
        //TODO
    }

    if (req->args_start && req->uri_end > req->args_start) {
        //TODO
    }
    //process_request_uri OVER
    if (req->host_start && req->host_end) {
        //TODO???
    }

    if (req->http_version < 1000) {
        //TODO 
    }
    
    //TODO init_list
    return 0;
}

static inline lowcase_key_copy_from_origin(struct string *low, struct string *origin)
{
    low->data = origin->data;
    low->length = origin->length;
    for (int i = 0; i < origin->length; i++) {
        low->data[i] = tolower(origin->data[i]);
    }
}
int http_process_request_headers(event_t *ev, void *arg)
{
//    if (timeout) TODO
    request_t *req = (request_t *)arg;
    int rc = LAGAIN;
    for (;;) {
        if (rc == LAGAIN) {
            if (req->header_in->pos == req->header_in->end) {
            //TODO
            }

//            ssize_t n = http_read_request_header(req);
        }

        rc = ngx_http_parse_header_line(req, req->header_in, 0/*TODO?*/);

        if (rc == LOK) {
            req->request_length += req->header_in->pos - req->header_name_start;
            if (req->invalid_header) {
                continue;
            }
            struct http_header_element *header_element = lt_alloc(req->header_pool, &req->header_pool_manager);

            header_element->hash = req->header_hash;//inline can reduce code number
            lt_string_assign_new(&header_element->key, 
                    req->header_name_end - req->header_name_start, req->header_name_start);
            lt_string_assign_new(&header_element->value, 
                    req->header_end - req->header_start, req->header_start);
            
            lowcase_key_copy_from_origin(&header_element->lowcase_key, &header_element->key);
        }
    }
    return 0;
}

int http_process_request_line(event_t *event, void *arg)
{
    request_t *req = (request_t *)arg;

//    http_read_request_header(r);
    int rv = ngx_http_parse_request_line(req, req->header_in);

    if (rv == LOK) {
        http_request_line_parsed(req, rv);
        //event->callback = 0

        event->callback = http_process_request_headers;
        http_process_request_headers(event, req);

    }

    return 0;
}


int http_data_coming(event_t *ev, void *arg)
{
    connection_t *conn = (connection_t *)arg;

    if (conn->timeout && conn->close) {
        //http_close_connecting
    }

    lt_buffer_t *buf;
    if (conn->buf) {
        buf = conn->buf;
    } else {
    }

    int rv = lt_recv(conn->fd, buf, DEFAULT_HEADER_BUFFER_SIZE);
    if (rv == LAGAIN) {
//        set_http_data_coming_timer();
//        conn->status = EFAULT;
        //push back lt_buffer to pool
        return 0;
    } if (rv == LCLOSE) {
        //http_close_connecting
    }

    http_create_request(conn);
    conn->ev->callback = http_process_request_line;
    http_process_request_line(conn->ev, conn);

    return 0;
}

connection_t *
http_init_connection(http_t *http, int fd, struct sockaddr peer_addr)
{
    connection_t *conn = lt_alloc(http->listen.connection_pool, 
            &http->listen.connection_pool_manager);

    conn->fd = fd;
    memcpy(&conn->peer_addr, &peer_addr, sizeof(struct sockaddr));
    
    conn->buf = lt_new_buffer_chain(http->listen.buf_pool, &http->listen.buf_pool_manager, 
            DEFAULT_HEADER_BUFFER_SIZE);

    conn->status = 0;

    lt_new_memory_pool_manager(&conn->request_pool_manager);

    conn->request_pool = lt_new_memory_pool(sizeof(request_t), 
            &conn->request_pool_manager);

    conn->ev = lt_io_add(http->base, fd, LV_FDRD|LV_CONN|LV_LAG, 
            http_data_coming, conn, INF);
    //add_timer
    return conn;
}

int start_accept(event_t *ev, void *arg)
{
    http_t *http = (http_t *)arg;

    for (int i = 0; i < SOMAXCONN; i++) {//TODO
        struct sockaddr peer_addr;
        int fd = lt_accept(http->listen.fd, &peer_addr);// maybe 512
        if (fd == LAGAIN) {
            return -1;
        } else if (fd == LABORT) {
            continue;
        } else if (fd == LERROR) {
            exit(EXIT_FAILURE);
        }

        connection_t *conn = http_init_connection(http, fd, peer_addr);


        continue;
    }
    return 0;
}

http_t *http_worker_new(base_t *base, conf_t *conf)
{
    http_t *http = malloc(sizeof(http_t));
    if (http == NULL) {
        perror("malloc http");
        return NULL;
    }

    lt_new_memory_pool_manager(&http->listen.connection_pool_manager);

    http->listen.connection_pool = lt_new_memory_pool(sizeof(connection_t), 
                                    &http->listen.connection_pool_manager);

    recv_listenfd_to_child(conf->pfd, &http->listen.fd);

    http->listen.ev = lt_io_add(base, conf->efd_distributor, LV_FDRD, 
            start_accept, http, NO_TIMEOUT);

    return http;
}

//TODO:key-value pair
