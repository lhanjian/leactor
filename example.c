#include "event_lea.h"

#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>

int incoming(int, void *);
int play_back(int, void *);

base_t *base;

int main(void)
{
    int my_sock;
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo(NULL, "12344", &hints, &res))) {
        fprintf(stderr, "gai:%s\n", gai_strerror(rv));
    }

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        my_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (my_sock == -1)  {
            perror("skt");
            continue;
        }

        int yes = 1;
        if (setsockopt(my_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("sso");
            exit(1);
        }
        
        if ( bind(my_sock, p->ai_addr, p->ai_addrlen) == -1 ) {
            close(my_sock);
            perror("bind");
            continue;
        }
    }
    freeaddrinfo(res);

    if (listen(my_sock, SOMAXCONN) == -1) {
        perror("listen");
        exit(1);
    }

    base = lt_base_init();
    
    lt_io_add(base, my_sock, LV_FDRD, incoming, &my_sock, INF);

    lt_base_loop(base, INF);

    return 0;
}

int incoming(int test, void *arg)
{
    struct sockaddr_storage sa;
    socklen_t addr_len = sizeof(sa);
    int my_sock = *(int *)arg;

    int *new_in_fd = malloc(sizeof(int));
    *new_in_fd = accept(my_sock, (struct sockaddr *)&sa, &addr_len);
    if (*new_in_fd == -1) {
        perror("acpt");
        return -1;;
    }

    lt_io_add(base, *new_in_fd, LV_FDRD|LV_CONN, play_back, new_in_fd, INF);

    return 0;
}

int play_back(int test, void *arg)
{
    char in_buff[32] = "testlhjtest\n\0";

    int in_fd = *(int *)arg;

    send(in_fd, in_buff, 32, 0);
//    printf("%s\n", in_buff);

    return 0;
}
