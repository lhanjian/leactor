#include "http.h"
#include "ngx_http_parse.h"
//static proxy_t *proxy_single;

int proxy_connect_writable(event_t *ev, void *arg)
{
    connection_t *conn = (connection_t *)arg;

    int err;
    socklen_t len = sizeof(err);

    if (getsockopt(conn->fd, SOL_SOCKET, SO_ERROR, &err, &len)) {
        perror("getsockopt CONNECT_writable:");
        return LERROR;
    }
    if (err == 0) {
        conn->status = L_PROXY_CONNECTED;
        conn->buf = lt_new_buffer_chain(conn->buf_pool, 
                conn->buf_pool_manager, DEFAULT_UPSTREAM_BUFFER_SIZE);
        //SUCCESS
//        lt_io_remove(ev->base, ev);
    }
    return 0;
}
/*
proxy_t *proxy_worker_new(base_t *base, conf_t *conf)
{
    proxy_t *proxy = calloc(1, sizeof(proxy_t));
    if (!proxy) {
        perror("malloc proxy");
        return NULL;
    }

    proxy->base = base; 

    lt_new_memory_pool_manager(&proxy->buf_pool_manager);
    proxy->buf_pool = lt_new_memory_pool(sizeof(lt_buffer_t), 
            &proxy->buf_pool_manager, NULL);

    int rv = proxy_connect_backend(proxy, conf);
    if (rv) {
        free(proxy);
        return NULL;
    }

    proxy_single = proxy;
    return proxy;
}
*/
/*
connection_t *proxy_connect_backend(proxy_t *proxy, conf_t *conf)
{
    proxy = proxy_single;
    proxy->conn_list[0].peer_addr_c = "localhost";
    if (proxy_connect(&proxy->conn_list[0], conf)) {
        return &proxy->conn_list[0];
    }

    return NULL;
}
*/
/*
int proxy_process_header(request_t *req)
{
    int rc;
    for (;;) {
        if (req->header_in->pos == req->header_in->end) {
            
        }

        if (rc == LAGAIN) {
        }
//        int rc = ngx_htt
        rc = ngx_http_parse_header_line(req, req->header_in, 1);
        if (rc == LOK) {


        }
    }
    return 0;
}
*/

char *proxy_get_upstream_addr()//TODO
{
    char *upstream = "127.0.0.1";//TODO
    return  upstream;
}

int proxy_connect(http_t *http, connection_t *conn)
{
    conn->peer_addr_in.sin_family = AF_INET;
    conn->peer_addr_in.sin_port = htons(80);
    inet_pton(conn->peer_addr_in.sin_family, 
              conn->peer_addr_c, 
             &conn->peer_addr_in.sin_addr);
//proxy connection initiazation
    conn->pair = lt_alloc(http->listen.connection_pool, 
            &http->listen.connection_pool_manager);
    conn->pair->request_pool_manager = conn->request_pool_manager;
    conn->pair->buf_pool = conn->buf_pool;
    conn->pair->buf_pool_manager = conn->buf_pool_manager;
    conn->pair->request_pool = conn->request_pool;
    conn->pair->request_pool_manager = conn->request_pool_manager;
        
    conn->pair->pair = conn;

    conn->pair->fd = socket(conn->peer_addr_in.sin_family, 
                      SOCK_STREAM|SOCK_NONBLOCK,
                      IPPROTO_TCP);
    if (conn->pair->fd == -1) {
        perror("socket proxy backend");
        return LERROR;
    }
    conn->pair->status = L_PROXY_CONNECTING;
    
    int rv = connect(conn->pair->fd, 
                     (struct sockaddr *)&conn->peer_addr_in, 
                     sizeof(struct sockaddr));
    if (rv < 0 && errno == EINPROGRESS) {
        conn->pair->ev = lt_io_add(conn->ev->base, conn->pair->fd, 
                LV_FDWR|LV_CONN|LV_ONESHOT, proxy_connect_writable, conn->pair, INF);
        return LAGAIN;
    } else if (!rv) {
        conn->pair->status = L_PROXY_CONNECTED;
        return LOK;
        //SUCCESS
    } else {
        return LERROR;
        perror("connect:");
    }

    return LERROR;
}

int proxy_data_coming(event_t *ev, void *arg)
{
    connection_t *conn = (connection_t *)arg;
    conn->buf = lt_new_buffer_chain(conn->buf_pool, conn->buf_pool_manager, DEFAULT_UPSTREAM_BUFFER_SIZE);
    int rv = lt_recv(conn->fd, conn->buf);//nginx just recv a part
    if (rv == LAGAIN) {
        conn->status = L_PROXY_WAITING_RESPONSE;
    } else if (rv == LCLOSE) {
        conn->status = L_PROXY_CLOSING;
    } else if (rv == LERROR) {
        conn->status = L_PROXY_ERROR;
    }

    if (conn->status == L_PROXY_WAITING_RESPONSE) {
        request_t *req = http_create_request(conn);
        http_process_response_line(conn, req);

        debug_print("%s", "SUCCESS parse response\n");
        //send_chains(ev->base, conn->pair->fd, &chain);
//        http_process_response_line(conn, req);
//        http_process_request_line(conn, req);

        return 0;
    } else {
//        conn->handler(conn, conn->handler_arg);
    }
    //send_chains(ev->base, conn->fd, <#lt_chain_t *#>)
    return 0;
}

int proxy_send_to_upstream(connection_t *conn, request_t *req)
{
    connection_t *proxy_conn = conn->pair;
    connection_t *client_conn = conn;

    int proxy_fd = conn->pair->fd;//= proxy_single->conn[conn->]->fd;
    base_t *base = conn->ev->base;

    lt_chain_t *send_chain = construct_request_chains(req);

    int rv = send_chains(base, proxy_fd, send_chain);
    switch (rv) {
        case LOK:
            client_conn->status = L_HTTP_WAITING_RESPONSE;
            proxy_conn->status = L_PROXY_WAITING_RESPONSE;
            //http_finish_request
            break;
        case LAGAIN:
            proxy_conn->status = L_PROXY_WRITING;
            break;
        case LERROR:
            proxy_conn->status = L_PROXY_ERROR;
            debug_print("%s", "ERROR\n");
            break;
        case LCLOSE:
            proxy_conn->status = L_PROXY_CLOSING;
            //connection_closed
            break;
        default:
            proxy_conn->status = L_PROXY_ERROR;
            debug_print("%s", "ERROR\n");
    }

    lt_io_mod(base,              proxy_conn->ev, LV_FDRD|LV_CONN, 
              proxy_data_coming, proxy_conn,     NO_TIMEOUT);
/*
    lt_io_add(conn->ev->base, proxy_fd, LV_CONN|LV_FDRD, 
            proxy_data_coming, proxy_conn, NO_TIMEOUT);
            */
    return 0;
}

lt_chain_t *construct_response_chains(request_t *rep)
{
//    int chain_len = 0;
    lt_new_memory_pool_manager(&rep->chain_pool_manager);
    rep->chain_pool = lt_new_memory_pool(sizeof(lt_chain_t), &rep->chain_pool_manager, NULL);
/*
    lt_chain_t *http_version = lt_alloc(rep->chain_pool, &rep->chain_pool_manager);
    http_version->buf.iov_base = rep->http_protocol.data;
    http_version->buf.iov_len = rep->http_protocol.length + 1;//' '
    chain_len++;

    lt_chain_t *status_chain = lt_alloc(rep->chain_pool, &rep->chain_pool_manager);
    http_version->next = status_chain;
    status_chain->buf.iov_base = rep->status.start;//OPTIM TODO: struct iovec 和 string 类似
    status_chain->buf.iov_len = rep->status.count;
    chain_len++;
    
    lt_chain_t *
    */
    lt_chain_t *status_line = lt_alloc(rep->chain_pool, &rep->chain_pool_manager);
    status_line->buf.iov_base = rep->request_line.data;
    status_line->buf.iov_len = rep->request_line.length + 2;

    int chain = 1;

    lt_chain_t *out_chain = status_line;
    lt_http_header_element_t *element = rep->element_head;
    lt_chain_t *cur_chain = lt_alloc(rep->chain_pool, &rep->chain_pool_manager);
    out_chain->next = cur_chain;
    for (;;) {
        cur_chain->buf.iov_base = element->key.data;
        cur_chain->buf.iov_len = element->key.length + 2;
        chain++;

        cur_chain->next = lt_alloc(rep->chain_pool, &rep->chain_pool_manager);
        cur_chain->next->buf.iov_base = element->value.data;
        cur_chain->next->buf.iov_len = element->value.length + 2;
        chain++;

        if (element == rep->element_tail) {
            cur_chain->next->buf.iov_len += 2;
            break;
        }
        element = element->next;
        lt_chain_t *new_chain = lt_alloc(rep->chain_pool, &rep->chain_pool_manager);

        cur_chain->next->next = new_chain;
        cur_chain = new_chain;
    }

    lt_chain_t *tail_chain = lt_alloc(rep->chain_pool, &rep->chain_pool_manager);
    chain++;
    out_chain->next = tail_chain;
    tail_chain->buf.iov_base = rep->header_end + 2;
    tail_chain->buf.iov_len = rep->header_in->last - (rep->header_end + 2);
    tail_chain->next = NULL;

    out_chain->chain_len = chain;
    return out_chain;
}
