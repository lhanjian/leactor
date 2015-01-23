#define DEBUG (1)

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
#include <arpa/inet.h>
#include <sys/eventfd.h>

#define DEFAULT_HEADER_BUFFER_SIZE (16384)
#define DEFAULT_UPSTREAM_BUFFER_SIZE (16384)
#define HTTP_PARSE_HEADER_DONE (1)
#define CHAIN_POOL_LENGTH (32)
#define CONNECTION_POOL_LENGTH (32)
#define BUFFER_POOL_LENGTH (64)
#define DEFAULT_HEADER_ELEMENT_COUNT (16)

typedef struct string {
    int length;
    char *data;
    unsigned hash;
} lt_string_t;

typedef struct hash {
    struct string *key;
    struct string *value;
    unsigned hash_value;
} lt_hash_t;

typedef struct hash_table {
    struct hash **buckets;//buckets array of struct hash pointer
    int size;
} lt_hash_table_t;

static inline void lt_string_assign_new(lt_string_t *p, int length, char *data) 
{
    p->data = data;
    p->length = length;
//    p->data[p->length] = '\0';
}

typedef struct conf {
    int efd_distributor;
//    int pfd[2];
    int listen_fd;
    base_t *base;
} conf_t;

/*struct upstream {
} upstream_t;*/

typedef struct status {
    int http_version;
    int code;
    int count;
    char *start;
    char *end;
} http_status_t;

typedef struct request {
    lt_buffer_t *header_in;//parse pos
    int state;

//request_line part
    struct string method_name;
    char *method_end;
    int method;
    struct string http_protocol;
    char *http_protocol_end;
    struct string uri;
    struct string unparsed_uri;
    int valid_unparsed_uri;
    char *uri_start;
    char *uri_end;
    char *uri_ext;
    char *port_end;
    char *host_start;
    char *host_end;
    char *args_start;
    char *schema_start;
    char *schema_end;

    int http_version;
    int complex_uri;
    int quoted_uri;
    int plus_in_uri;
    int space_in_uri;
//    int response_code;
//request_line
    struct string request_line;
    int request_length;
    int line_length;
    char *request_start;//request line start
    char *request_end;//request line end
//request_header
    char *header_start; //one header field start
    char *header_end; //one header field end
//request_header_field
    char *header_name_start;// one header field name start
    char *header_name_end;//one header field name end
    struct http_header_element *element_head;
    struct http_header_element *element_tail;
//request_header_part
    int header_hash;
    int header_length;
    int lowcase_index;
    int invalid_header; 
    int http_major;
    int http_minor;

    char lowcase_header[32];

    struct upstream *upstream;//http_parse
    struct status status;//http_parse

    struct lt_memory_pool_manager  header_pool_manager;

    struct lt_memory_pool_manager  chain_pool_manager;

    struct connection *conn;
    lt_chain_t *out_chain;
    struct request *next;
} request_t;


typedef struct connection {
    int fd;
//    int proxy_fd;
    struct sockaddr peer_addr;
    struct sockaddr_in peer_addr_in;
    char *peer_addr_c;
    char *peer_port;

    int (*handler)(struct connection *, void *);
    void *handler_arg;

    int (*proxy_handler)(struct connection *, void *);
    void *proxy_handler_arg;

    struct event *ev;
    struct event *proxy_ev;

    struct lt_memory_pool_manager header_pool_manager;

    struct lt_memory_pool_manager request_pool_manager;
    struct request *request_free_head;
    struct request *request_free_tail;

    lt_buffer_t *buf;
    lt_buffer_t *proxy_buf;

    struct lt_memory_pool_manager *buf_pool;//copy from proxy_t/listen_t
    struct lt_memory_pool_manager *buf_pool_manager;//copy from proxy_t/listen_t

    int status;
//    int proxy_status;

    struct connection *next;
    struct connection *pair;

    lt_chain_t *upstream_chain;
    lt_chain_t *downstream_chain;
    
    int timeout;
    int close;
} connection_t;

typedef int (*conn_cb_t)(struct connection *, void *);

typedef struct listening {
    int fd;
//    struct addrinfo local_addr;
    char *bind_addr;

    char *bind_port;
    struct sockaddr saddr;

    struct lt_memory_pool_manager connection_pool_manager;
    struct connection listen_conn;
    struct connection *client_list;
    struct connection *downstream_list;
    struct event *ev;

    struct lt_memory_pool_manager buf_pool_manager;
} listening_t;

typedef struct http {
    struct base *base;
//    struct connection *connection_list;
    struct listening listen;
    int core_amount;
    int efd;

} http_t;

void ignore_sigpipe(void);

http_t *http_master_new(base_t *, conf_t *);
http_t *http_worker_new(base_t *, conf_t *);
request_t *http_create_request(connection_t *);
int http_process_request_line(connection_t *, void *arg);
int http_process_response_line(connection_t *, void *arg);

typedef struct http_header_element {
    int hash;
    int length;
    struct string key;
    struct string value;
    struct string lowcase_key;
    struct http_header_element *next;
} lt_http_header_element_t;

void lowcase_key_copy_from_origin(struct string *, struct string *);
typedef struct proxy {
    base_t *base;
    connection_t *conn_list;//TEMP TODO
    struct lt_memory_pool *buf_pool;
    struct lt_memory_pool buf_pool_manager;
} proxy_t;

proxy_t *proxy_worker_new(base_t *, conf_t *);
connection_t *proxy_connect_backend(proxy_t *, conf_t *);
int proxy_connect(http_t *, connection_t *);
int proxy_send_to_upstream(connection_t *conn, request_t *req);
lt_chain_t *http_construct_request_chains(request_t *req);
int http_send_to_upstream(request_t *);
lt_chain_t *construct_response_chains(request_t *rep);
char *proxy_get_upstream_addr();
int http_send_to_client(connection_t *, request_t *);
int http_is_chunked_tail(request_t *, lt_buffer_t *);
int destructor_chains(request_t *, lt_chain_t *);

#define L_PROXY_CONNECTED (-1)
#define L_PROXY_CONNECTING (-2)
#define L_CONNECTING_ACCEPTED (-3)//CONNECTED is equal to ACCEPTED
#define L_HTTP_WROTE_RESPONSE (-3)
#define L_PROXY_WAITING_RESPONSE (-4)
#define L_HTTP_WRITING_RESPONSE_HEADER (-4)
#define L_PROXY_WRITING (-5) 
#define L_HTTP_WAITING_RESPONSE (-5)
#define L_PROXY_ERROR (-6)
#define L_HTTP_ERROR (-6)
#define L_PROXY_SENDING_RESPONSE_TO_CLIENT (-7)
#define L_PROXY_CLOSING (-8)
#define L_HTTP_CLOSING (-8)
#define L_HTTP_WROTE_RESPONSE_HEADER (-9)
