#include "event_lea.h"

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
typedef struct connection {
    
} connection_t;
typedef struct http {
    struct base *base;
    struct connection *connection_list;
    struct addrinfo local_addr;
} http_t;
void ignore_sigpipe(void);
http_t *http_new(base_t *);


