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
        perror("connect:");
    }

    return 0;
}

int proxy_data_coming(event_t *ev, void *arg)
{
    connection_t *conn = (connection_t *)arg;
    int rv = lt_recv(conn->proxy_fd, conn->proxy_buf);
    if (rv == LAGAIN) {
    } else if (rv == LCLOSE) {
    } else if (rv == LERROR) {
    }

    if (conn->proxy_status == L_PROXY_WAITING_RESPONSE) {
        request_t *req = http_create_request(conn);
        http_process_request_line(conn, req);

        conn->handler = http_process_request_line;
        conn->handler_arg = req;
        return 0;
    } else {
        conn->handler(conn, conn->handler_arg);
    }
    //send_chains(ev->base, conn->fd, <#lt_chain_t *#>)
    return 0;
}

int proxy_send_to_upstream(connection_t *conn, request_t *req)
{
    int proxy_fd = conn->pair->fd;//= proxy_single->conn[conn->]->fd;
    base_t *base = conn->ev->base;
    connection_t *proxy_conn = conn->pair;

    lt_chain_t *send_chain = construct_chains(req);

    int rv = send_chains(base, proxy_fd, send_chain);
    switch (rv) {
        case LOK:
            proxy_conn->status = L_PROXY_WAITING_RESPONSE;
            //http_finish_request
            break;
        case LAGAIN:
            proxy_conn->status = L_PROXY_WRITING;
            break;
        case LERROR:
        case LCLOSE:
        default:
            proxy_conn->status = L_PROXY_ERROR;
            debug_print("%s", "ERROR\n");
    }

    lt_io_mod(base, conn->ev, LV_FDRD|LV_CONN, proxy_data_coming, conn->pair, NO_TIMEOUT);
/*
    lt_io_add(conn->ev->base, proxy_fd, LV_CONN|LV_FDRD, 
            proxy_data_coming, proxy_conn, NO_TIMEOUT);
            */

    return 0;
}
