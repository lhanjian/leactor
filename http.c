#include "http.h"
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

http_t *http_new(base_t *base, conf_t *conf)
{
    http_t *http = calloc(1, sizeof http_t);
    if (!http) {
        perror("malloc http");
        return NULL;
    }

    http->listen.bind_addr = NULL;//ALL available localaddr TODO:sustitute with JSON(conf file)???
    http->listen.bind_port = "80";//same to UP
    return http;
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

    for (struct addrinfo *p = res; p != NULL; p->next) {
        int listen_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listen_sock == -1) {
            perror("listen_sock_fd_get");
            continue;
        }

        int yes = 1;
        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, 
                    &yes, sizeof yes) == -1) {
            perror("setsockopt_REUSE_ADDR");
            return -1;
        }

        if (bind(listen_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind error");
            continue;
        }
        http->listen.fd = listen_sock;
        http->listen.saddr = p->ai_addr;
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server:failed to bind");
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}

int http_add_listen(http)
{
    int rv;
    rv = get_addrinfo_with_bind(http);
    if (rv) {
        fprintf(stderr, "get_addrinfo_with_bind error");
        return -1
    }

    rv = listen(http->fd, SOMAXCONN);
    if (rv) {
        fprintf(stderr, "listen error");
        return -1;
    }

    return 0;
}
