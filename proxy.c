#include "http.h"
#include "ngx_http_parse.h"

static proxy_t *proxy_single;

int proxy_connect_writable(event_t *ev, void *arg)
{
    connection_t *conn = (connection_t *)arg;

    int err;
    socklen_t len = sizeof(err);

    if (getsockopt(conn->proxy_fd, SOL_SOCKET, SO_ERROR, &err, &len)) {
        perror("getsockopt CONNECT_writable:");
        return LERROR;
    }
    if (err == 0) {
        conn->status = L_PROXY_CONNECTED;
        //SUCCESS
        lt_io_remove(ev->base, ev);
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

int proxy_connect(connection_t *conn, http_t *http)
{
    conn->peer_addr_in.sin_family = AF_INET;
    conn->peer_addr_in.sin_port = htons(80);
    inet_pton(conn->peer_addr_in.sin_family, 
              conn->peer_addr_c, 
             &conn->peer_addr_in.sin_addr);

    conn->proxy_fd = socket(conn->peer_addr_in.sin_family, 
                      SOCK_STREAM|SOCK_NONBLOCK,
                      IPPROTO_TCP);
    if (conn->proxy_fd == -1) {
        perror("socket proxy backend");
        return LERROR;
    }
    conn->status = L_PROXY_CONNECTING;
    
    int rv = connect(conn->proxy_fd, 
                     (struct sockaddr *)&conn->peer_addr_in, 
                     sizeof(struct sockaddr));
    if (rv < 0 && errno == EINPROGRESS) {
        conn->ev = lt_io_add(http->base, conn->proxy_fd, LV_FDWR|LV_CONN|LV_ONESHOT, 
                proxy_connect_writable, conn, INF);
        return LAGAIN;
    } else if (!rv) {
        conn->status = L_PROXY_CONNECTED;
        return LOK;
        //SUCCESS
    } else {
        perror("connect:");
    }

    return 0;
}

int proxy_send_to_upstream(connection_t *conn, request_t *req)
{
    int proxy_fd = conn->proxy_fd;;//= proxy_single->conn[conn->]->fd;

 //   lt_buffer_t *buf = req->header_in;
/*    buf = lt_new_buffer_chain(proxy_single->buf_pool, 
            &proxy_single->buf_pool_manager, DEFAULT_UPSTREAM_BUFFER_SIZE);*/
//    lt_chain_t *chain = construct_chains();
    //create_chain_and_modify_req_to_chain
//    lt_chain_t *new_chain = send_chains(fd, chain);
//    send_buffer_chains_loop(fd, buf);

    lt_chain_t *send_chain = construct_chains(req);

//    lt_chain_t *remain_chain = send_chains(proxy_fd, send_chain);
//  应当一口气发完，然后让下层通知上层
    lt_io_add(conn->ev->base, conn->proxy_fd, LV_CONN|LV_FDRD, proxy_data_coming, conn, NO_TIMEOUT);

    return 0;
}
