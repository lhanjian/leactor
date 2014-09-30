#include "event_lea.h"

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
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "12344", &hints, &res);

    for (struct addrinfo *p; p != NULL; p = p->ai_next) {
        my_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        bind(my_sock, p->ai_addr, p->ai_addrlen);
    }
    freeaddrinfo(res);

    listen(my_sock, SOMAXCONN);

    base_t *base = lt_base_init();
    
    lt_io_add(base, my_sock, LV_FDRD, incoming, &my_sock, INF);

    lt_base_loop(base, INF);

    return 0;
}

int incoming(int test, void *arg)
{
    struct sockaddr_storage sa;
    socklen_t addr_len = sizeof(sa);
    int my_sock = *(int *)arg;

    int new_in_fd = accept(my_sock, (struct sockaddr *)&sa, &addr_len);

    lt_io_add(base, new_in_fd, LV_FDRD, play_back, &new_in_fd, INF);

    return 0;
}

int play_back(int test, void *arg)
{
    char in_buff[32];

    int in_fd = *(int *)arg;

    in_buff[31] = '\0';
    recv(in_fd, in_buff, 32, 0);
    printf("%s\n", in_buff);

    return 0;
}
