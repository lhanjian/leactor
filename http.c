#include "http.h"
#include "ngx_http_parse.h"

static int get_addrinfo_with_bind(http_t *http);
static int start_accept(event_t *ev, void *arg);
static int http_add_listen(http_t *http, conf_t *conf);
int http_process_host(request_t *, lt_string_t * /*, 28*/);

unsigned int HostHash;

char *bind_addr() { char *addr = "127.0.0.1"; return addr; }
char *bind_port() { char *port = "8080"; return port; }

unsigned int BKDRhash(char *str, int length)
{
    unsigned hash = 0;
    for (int i = 0; i < length; i++) {
        char ch = str[i];
        hash = hash * 31 + ch;
    }
    return hash;
}

int http_check_chunked(lt_buffer_t *buf)
{
    return 0;
}
int http_find_host(request_t *req)
{
    return LOK;
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

        http->listen.fd = listen_sock;

        lt_set_reuseaddr(listen_sock, 1);
        lt_set_reuseport(listen_sock, 1);
//        lt_set_keepalive(listen_sock, 1);

        if (bind(listen_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(listen_sock);
            perror("bind error");
            continue;
        }
        http->listen.saddr = *p->ai_addr;
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server:failed to bind");
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}

int http_add_listen(http_t *http, conf_t *conf)
{
    http->listen.bind_addr = bind_addr();
    http->listen.bind_port = bind_port();

    int rv = get_addrinfo_with_bind(http);
    if (rv) {
        fprintf(stderr, "get_addrinfo_with_bind error");
        return -1;
    }

    rv = listen(http->listen.fd, SOMAXCONN);
    if (rv) {
        fprintf(stderr, "listen error");
        return -1;
    }

    http->listen.ev = lt_io_add(http->base, http->listen.fd, 
            LV_CONN|LV_FDRD, start_accept, http, NO_TIMEOUT);

    return 0;
}

request_t *http_create_request(connection_t *conn)
{
    request_t *req;

    if (conn->request_free_head) {
        req = conn->request_free_head;
        conn->request_free_head = req->next;
    } else {
        req = lt_alloc(&conn->request_pool_manager);
    }

    req->conn = conn;
    req->header_in = conn->buf;
    req->state = 0;

    lt_new_memory_pool_manager(&req->header_pool_manager,
    		sizeof(struct http_header_element), DEFAULT_HEADER_ELEMENT_COUNT);

    return req;
}

int http_status_line_parsed(request_t *req, int rv)
{
    return 0;
}

int http_request_line_parsed(request_t *req, int rv)
{
    req->request_line.length = req->request_end - req->request_start;
    req->request_line.data = req->request_start;
    req->request_length = req->header_in->pos - req->request_start;

    req->method_name.length = req->method_end - req->request_start + 1;
    req->method_name.data = req->request_line.data;

    req->http_protocol.length = req->request_end - req->http_protocol.data;

    //process_request_uri
/*    if (req->args_start) {//NO URI ARGS PARSE
        req->uri.length = req->args_start - 1 - req->uri_start;//BUG???
    } else */

    {
        req->uri.length = req->uri_end - req->uri_start;
    }

    if (req->complex_uri || req->quoted_uri) {
        //TODO
    } else {
        req->uri.data = req->uri_start;
    }

    req->unparsed_uri.length = req->uri_end - req->uri_start;
    req->unparsed_uri.data = req->uri_start;

    req->valid_unparsed_uri = req->space_in_uri ? 0 : 1;16

    if (req->uri_ext) {
        //TODO
    }
/*
    if (req->args_start && req->uri_end > req->args_start) {
        req->args.len = req->uri_end - req->args_start;
        req->args.data = req->args_start;
        //TODO:NO ARGS PARSE
    }
    */
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

void lowcase_key_copy_from_origin(struct string *low, struct string *origin)
{
    low->data = origin->data;
    low->length = origin->length;
    for (int i = 0; i < origin->length; i++) {
        low->data[i] = tolower(origin->data[i]);//TODO new data
    }
    return ;
}

int http_process_element(request_t *req, lt_http_header_element_t *element)
{            
    enum {
        fl_host = 0
    };

    element->length = req->header_length;

    if (element->hash == HostHash) {//TODO use hash-find?
        http_process_host(req, &element->value/*, 28*/);
        if (http_find_host(req)) {
            return LERROR;
        }
    }
//  if (hash == ContentLengthHash) {//TODO
//  //for HTTP POST
//  }
//
    return LOK;
}

int http_process_request_headers(connection_t *conn, void *arg)
{
//    if (timeout) TODO
    request_t *req = (request_t *)arg;
    int rc = LAGAIN;
    lt_http_header_element_t *prev_header_element = NULL;
    req->element_head = NULL;
    req->element_tail = NULL;
    for (;;) {
        if (rc == LAGAIN) {
            if (req->header_in->pos == req->header_in->end) {
            //TODO:we have toooooooo big header
            }
            
            if (req->header_in->pos == req->header_in->last) {
            }
            //TEMP: 单纯抛弃
            //TODO: 挂上IO等可读
            //ssize_t n = http_read_request_header(req);
        }

        rc = ngx_http_parse_header_line(req, req->header_in, 1/*TODO?*/);

        if (rc == LOK) {
            req->request_length += req->header_in->pos - req->header_name_start;
            if (req->invalid_header) {
                continue;
            }
            lt_http_header_element_t *header_element = 
                lt_alloc(&req->header_pool_manager);

            if (req->element_head == NULL) {
                req->element_head = header_element;
                req->element_tail = header_element;
                prev_header_element = header_element;
            } else {
                prev_header_element->next = header_element;
                req->element_tail = header_element;
                header_element->next = NULL;
                prev_header_element = header_element;
            }
            //MUST BE COMPLETED
            header_element->hash = req->header_hash;//inline can reduce code number
            lt_string_assign_new(&header_element->key, 
                    req->header_name_end - req->header_name_start, req->header_name_start);
            lt_string_assign_new(&header_element->value, 
                    req->header_end - req->header_start, req->header_start);
            //TODO:chain
            
            lowcase_key_copy_from_origin(&header_element->lowcase_key, &header_element->key);

            if (http_process_element(req, header_element) == LERROR) {
                //close connection
            }
        }
        //MUST BE COMPLETED
        if (rc == HTTP_PARSE_HEADER_DONE) {
            /*
            req->request_length += req->header_in->pos - req->header_name_start;
            debug_print("%s", "DONE\n");
            if (conn->pair->status == L_PROXY_SENDING_RESPONSE_TO_CLIENT) {

            conn->status = 
            }
            */
            //if (conn->status == L_PROXY_SENDING_
            return LOK;
//            req->http_state = HTTP_PROCE
//            rc = lt_recv(ev->fd, <#lt_buffer_t *#>, <#size_t#>)
//            http_validation_host(req);
        }
    }
    return 0;
}


int http_process_response_line(connection_t *conn, void *arg)
{
    request_t *req = (request_t *)arg;

    int rv = ngx_http_parse_status_line(req, conn->buf, &req->status);

    if (rv == LOK) {
        req->request_line.data = req->request_start;
        req->request_line.length = req->status.end - req->request_start;

        req->http_protocol.length = req->http_protocol_end - req->http_protocol.data;
        
        int rc = http_process_request_headers(conn, arg);
        if (rc == LOK) {
            return http_send_to_client(conn, req);
        }
        //HTTP_VERSION:T
    }

    return 0;
}

int http_finish_request(connection_t *conn, request_t *req)
{
    lt_destroy_memory_pool(&req->header_pool_manager);

    req->next = NULL;
    if (conn->request_free_tail) {
        conn->request_free_tail->next = req;
        conn->request_free_tail = req;
    } else {
        conn->request_free_tail = req;
        conn->request_free_head = req;
    }
    
    return 0;
}

int http_process_request_line(connection_t *conn, void *arg)
{
    request_t *req = (request_t *)arg;
//    http_read_request_header(r);
    int rv = ngx_http_parse_request_line(req, req->header_in);

    if (rv == LOK) {
        //HTTP_VERSION:TODO
        http_request_line_parsed(req, rv);

        rv = http_process_request_headers(conn, req);
        if (rv == LOK) {
            lt_chain_t *send_chain = http_construct_request_chains(req);
            req->out_chain = send_chain;
            rv = http_send_to_upstream(req);
        }

        return http_process_request_line(conn, arg);
    }

    return rv;//试图简化pipeline的流程
}

int http_data_coming(event_t *ev, void *arg)
{
    connection_t *conn = (connection_t *)arg;

    if (conn->timeout && conn->close) {
        //http_close_connecting
    }

    conn->buf = lt_new_buffer_chain(//when to release TODO
            conn->buf_pool_manager, DEFAULT_HEADER_BUFFER_SIZE);
    //TODO:pipeline Coming

    int rv = lt_recv(conn->fd, conn->buf);
    if (rv == LAGAIN) {
//        set_http_data_coming_timer();
//        conn->status = EFAULT;
        //push back lt_buffer to pool TODO
//        return 0;
//
    } else if (rv == LCLOSE) {
        debug_print("%s", "HTTP CLIENT FD CLOSED\n");
        //http_close_connecting
    } else if (rv == LERROR) {
        //http_close_connecting 
    }

    if (conn->status == L_CONNECTING_ACCEPTED) { //New connection
        request_t *req = http_create_request(conn);
        http_process_request_line(conn, req);

        //proxy_send_to_upstream(conn, req);//NEXT upstream

        return 0;
    } 
    
    {//state machine???
        //if waiting remaining part of packet???
//        conn->handler(conn, conn->handler_arg);
    }
//    ev->callback = http_process_request_line;
//    ev->arg = req;
//    状态处理
    return 0;
}

connection_t *
http_init_connection(http_t *http, int fd, struct sockaddr peer_addr)
{
    connection_t *conn = lt_alloc(&http->listen.connection_pool_manager);

    conn->fd = fd;

    memcpy(&conn->peer_addr, &peer_addr, sizeof(struct sockaddr));

    conn->buf_pool_manager = &http->listen.buf_pool_manager;
    conn->status = L_CONNECTING_ACCEPTED;

    lt_new_memory_pool_manager(&conn->request_pool_manager,
    		sizeof (request_t), 4);

    conn->ev = lt_io_add(http->base, fd, LV_FDRD|LV_CONN/*|LV_LAG*/, 
            http_data_coming, conn, NO_TIMEOUT);
    
    conn->peer_addr_c = proxy_get_upstream_addr();
    int rv = proxy_connect(http, conn);//pair connection
    if (rv == LERROR) {//TODO
    	//proxy_next
        return NULL;
    }
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

        http_init_connection(http, fd, peer_addr);

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

    lt_new_memory_pool_manager(&http->listen.connection_pool_manager,
    		sizeof(connection_t), CONNECTION_POOL_LENGTH);

    HostHash = BKDRhash("Host", sizeof("Host"));

    http->base = base;

    int rv = http_add_listen(http, conf);
    if (rv) {
        free(http);
        return NULL;
    }

    rv = lt_ignore_sigpipe();
    if (rv) {
        free(http);
        return NULL;
    }

    lt_new_memory_pool_manager(&http->listen.buf_pool_manager ,
    		sizeof(lt_buffer_t), BUFFER_POOL_LENGTH);
    return http;
}

lt_chain_t *construct_response_chains(request_t *rep)
{
//    int chain_len = 0;
    lt_new_memory_pool_manager(&rep->chain_pool_manager,
    		sizeof(lt_chain_t), CHAIN_POOL_LENGTH);

    lt_chain_t *status_line = lt_alloc(&rep->chain_pool_manager);
    status_line->buf.iov_base = rep->request_line.data;
    status_line->buf.iov_len = rep->request_line.length + 2;

    int chain = 1;

    lt_chain_t *out_chain = status_line;
    lt_http_header_element_t *element = rep->element_head;

    lt_chain_t *cur_chain = lt_alloc(&rep->chain_pool_manager);
    out_chain->next = cur_chain;
    for (;;) {
        cur_chain->buf.iov_base = element->key.data;
        cur_chain->buf.iov_len = element->key.length + 2;
        chain++;

        cur_chain->next = lt_alloc(&rep->chain_pool_manager);
        cur_chain->next->buf.iov_base = element->value.data;
        cur_chain->next->buf.iov_len = element->value.length + 2;
        chain++;

        if (element == rep->element_tail) {
            cur_chain->next->buf.iov_len += 2;

            goto done;

        }
        element = element->next;
        lt_chain_t *new_chain = lt_alloc(&rep->chain_pool_manager);

        cur_chain->next->next = new_chain;
        cur_chain = new_chain;
    }
done:
    out_chain->chain_len = chain;
    return out_chain;
}

int http_send_to_client(connection_t *conn, request_t *req)
{
    lt_chain_t *out_chain = construct_response_chains(req);

    connection_t *client_conn = conn->pair;

    int rv = send_chains(conn->ev->base, client_conn->fd, &out_chain);
    switch (rv) {
        case LOK:
            conn->status = L_HTTP_WROTE_RESPONSE_HEADER;
            break;
        case LAGAIN://NO USE
            conn->status = L_HTTP_WRITING_RESPONSE_HEADER;
            break;
        case LCLOSE:
            conn->status = L_HTTP_CLOSING;
            break;
        case LERROR:
        default:
            conn->status = L_HTTP_ERROR;
            debug_print("%s", "ERROR\n");
    }

    return 0;
}
//TODO:key-value pair

int destructor_chains(request_t *req, lt_chain_t *chain)
{
	lt_destroy_memory_pool(&req->chain_pool_manager);
    return 0;
}

lt_chain_t *http_construct_request_chains(request_t *req)
{
    lt_new_memory_pool_manager(&req->chain_pool_manager, sizeof(lt_chain_t), CHAIN_POOL_LENGTH);
/*
    lt_chain_t *chain_request_line = lt_alloc(req->chain_pool, &req->chain_pool_manager);
    chain_request_line->buf.iov_base = req->request_start;
    chain_request_line->buf.iov_len = req->request_length;
    */
//    lt_chain_t *old_chain = chain_request_line;

    int chain_len = 0;
//TODO 合并本就在连续地址上的chain
    lt_chain_t *method_chain = lt_alloc(&req->chain_pool_manager);
    method_chain->buf.iov_base = req->method_name.data;//method
    method_chain->buf.iov_len = req->method_name.length + 1;//" "
    chain_len++;

    lt_chain_t *chain_uri = lt_alloc(&req->chain_pool_manager);
    method_chain->next = chain_uri;
    chain_uri->buf.iov_base = req->uri.data;//uri
    chain_uri->buf.iov_len = req->uri.length + 1;//" "
    chain_len++;

    lt_chain_t *http_version = lt_alloc(&req->chain_pool_manager);
    chain_uri->next = http_version;
    http_version->buf.iov_base = req->http_protocol.data;
    http_version->buf.iov_len = req->http_protocol.length + 2;//"\r\n"
    chain_len++;

    lt_chain_t *new_chain;

    lt_http_header_element_t *element = req->element_head;
    if (!element) {
        //NO HTTP HEADER FIELD
        goto done;
    }

    lt_chain_t *chain_request_header_field = lt_alloc(&req->chain_pool_manager);
    http_version->next = chain_request_header_field;
    for (;;) {

        chain_request_header_field->buf.iov_base = element->key.data;
        chain_request_header_field->buf.iov_len = element->key.length + 2;
        chain_len++;

        chain_request_header_field->next = 
            lt_alloc(&req->chain_pool_manager);
        chain_request_header_field->next->buf.iov_base = element->value.data;
        chain_request_header_field->next->buf.iov_len = element->value.length + 2;
        chain_len++;

        if (element == req->element_tail) {
            chain_request_header_field->next->buf.iov_len += 2;
            break;
        }

        element = element->next;
        new_chain = lt_alloc(&req->chain_pool_manager);

        chain_request_header_field->next->next = new_chain;
        chain_request_header_field = new_chain;
    }

done:
    method_chain->chain_len = chain_len;
    return method_chain;
}
