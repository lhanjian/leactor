#include "event_lea.h"
#include <features.h>
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

#define DEFAULT_HEADER_BUFFER_SIZE (16384)
typedef struct string {
    int length;
    char *data;
} lt_string_t;
static inline void lt_string_assign_new(lt_string_t *p, int length, char *data) 
{
    p->data = data;
    p->length = length;
    p->data[p->length] = '\0';
}

typedef struct conf {
    int efd_distributor;
    int pfd[2];
} conf_t;

struct upstream {
} upstream_t;
typedef struct request {
    lt_buffer_t *header_in;
    int state;
    char *request_start;
    char *request_end;
    char *uri_start;
    char *host_start;
    char *schema_start;
    char *args_start;
    char *schema_end;
    char *uri_end;
    char *uri_ext;
    char *host_end;
    char *port_end;
    char *header_name_start;
    char *header_name_end;
    char *header_start;
    char *header_end;

    int header_hash;
    int lowcase_index;
    int invalid_header; 
    int complex_uri;
    int quoted_uri;
    int plus_in_uri;
    int space_in_uri;
    int http_version;

    int http_major;
    int http_minor;

    char lowcase_header[32];

    struct upstream *upstream;

    struct string request_line;
    int request_length;

    struct string method_name;
    char *method_end;
    int method;

    struct string http_protocol;
    char *http_protocol_end;
    
    struct string uri;
    struct string unparsed_uri;

    int valid_unparsed_uri;

    struct lt_memory_pool *header_pool;
    struct lt_memory_pool  header_pool_manager;
} request_t;


typedef struct connection {
    int fd;
    struct sockaddr peer_addr;
    char *peer_port;

    func_t callback;
    void *conn_callback_arg;

    func_t close_callback;
    void *conn_callback_close_arg;

    struct event *ev;

    struct lt_memory_pool *request_pool;
    struct lt_memory_pool request_pool_manager;
    struct request *request_list;

    lt_buffer_t *buf;

    func_t conn_handler;

    int status;

    struct connection *next;
    int timeout;
    int close;
    //buffer
    //header_size
    //body_size
    //state
} connection_t;

typedef struct listening {
    int fd;
//    struct addrinfo local_addr;
    char *bind_addr;
    char *bind_port;
    struct sockaddr saddr;

    struct lt_memory_pool *connection_pool;
    struct lt_memory_pool connection_pool_manager;
    struct connection listen_conn;
    struct connection *client_list;
    struct connection *downstream_list;;
    struct event *ev;

        struct lt_memory_pool *buf_pool;
        struct lt_memory_pool buf_pool_manager;
} listening_t;

typedef struct http {
    struct base *base;
//    struct connection *connection_list;
    struct listening listen;
    int core_amount;
} http_t;

void ignore_sigpipe(void);

http_t *http_master_new(base_t *, conf_t *);

typedef struct http_header_element {
    int hash;
    struct string key;
    struct string value;
    struct string lowcase_key;
} lt_http_header_element_t;
