#include "http.h"
#include "ngx_http_parse.h"

proxy_t *proxy_single;

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
        //SUCCESS
        lt_io_remove(ev->base, ev);
    }
    return 0;
}

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

int proxy_connect_backend(proxy_t *proxy, conf_t *conf)
{
    proxy->conn[0].peer_addr_c = "localhost";
    if (proxy_connect(&proxy->conn[0], conf)) {
    }
    return 0;
}

int proxy_connect(connection_t *conn, conf_t *conf)
{
    conn->peer_addr_in.sin_family = AF_INET;
    conn->peer_addr_in.sin_port = htons(80);
    inet_pton(conn->peer_addr_in.sin_family, 
              conn->peer_addr_c, 
             &conn->peer_addr_in.sin_addr);

    conn->fd = socket(conn->peer_addr_in.sin_family, 
                      SOCK_STREAM|SOCK_NONBLOCK,
                      IPPROTO_TCP);
    if (conn->fd == -1) {
        perror("connect backend");
        return -1;
    }
    conn->status = 0;
    
    int rv = connect(conn->fd, 
                     (struct sockaddr *)&conn->peer_addr_in, 
                     sizeof(struct sockaddr));
    if (rv < 0 && errno == EINPROGRESS) {
        conn->ev = lt_io_add(conf->base, conn->fd, LV_FDWR|LV_CONN, 
                proxy_connect_writable, conn, INF);
        return LAGAIN;
    } else if (!rv) {
        return LOK;
        //SUCCESS
    } else {
        perror("connect:");
    }

    return 0;
}

int proxy_send_to_upstream(connection_t *conn, request_t *req)
{
    int fd = proxy_single->conn->fd;

    lt_buffer_t *buf = req->header_in;
/*    buf = lt_new_buffer_chain(proxy_single->buf_pool, 
            &proxy_single->buf_pool_manager, DEFAULT_UPSTREAM_BUFFER_SIZE);*/

    lt_chain_t *chain;

    //create_chain_and_modify_req_to_chain

    lt_chain_t *new_chain = send_chains(fd, chain);

//    send_buffer_chains_loop(fd, buf);

    return 0;
}
