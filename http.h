#include "event_lea.h"
#include <features.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/eventfd.h>
typedef struct conf {
    int efd_distributor;
    int pfd[2];
} conf_t;
typedef struct request {

} request_t;

typedef struct connection {
    int fd;
    struct sockaddr peer_addr;
    char *peer_port;

    func_t callback;
    void *conn_callback_arg;

    
    struct lt_memory_pool *request_pool;
    struct lt_memory_pool *request_pool_manager;
    struct request *request_list;
} connection_t;

typedef struct listening {
    int fd;
//    struct addrinfo local_addr;
    char *bind_addr;
    char *bind_port;
    struct sockaddr saddr;

    struct lt_memory_pool *connection_pool;
    struct lt_memory_pool *connection_pool_manager;
    struct connection listen_conn;
    struct connection *client_list;
    struct connection *downstream_list;;
    struct event *ev;

} listening_t;

typedef struct http {
    struct base *base;
    struct connection *connection_list;
    struct listening listen;
    int core_amount;
} http_t;

void ignore_sigpipe(void);

http_t *http_master_new(base_t *, conf_t *);