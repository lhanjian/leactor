#include "event_lea.h"

#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>

static inline int ex_nonblocking(int fd)
{ return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);};

int incoming(int, void *);
int play_back(int, void *);

base_t *base;
event_t *eventarray[128];
int n = 0;

int main(void)
{
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo(NULL, "12344", &hints, &res))) {
        fprintf(stderr, "gai:%s\n", gai_strerror(rv));
    }

    int listen_sock;
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        listen_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listen_sock == -1)  {
            perror("skt");
            continue;
        }

        int yes = 1;
        if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("sso");
            exit(1);
        }
        
        if (bind(listen_sock, p->ai_addr, p->ai_addrlen) == -1 ) {
            close(listen_sock);
            perror("bind");
            continue;
        }
    }
    freeaddrinfo(res);

    if (listen(listen_sock, SOMAXCONN) == -1) {
        perror("listen");
        exit(1);
    }

    ex_nonblocking(listen_sock);

    base = lt_base_init();
    
    lt_io_add(base, listen_sock, LV_FDRD, incoming, &listen_sock, NO_TIMEOUT);

    lt_base_loop(base, NO_TIMEOUT);

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

    ex_nonblocking(*new_in_fd);
    eventarray[*new_in_fd] = lt_io_add(base, *new_in_fd, LV_FDRD|LV_CONN, play_back, new_in_fd, NO_TIMEOUT);
    n++;

    return 0;
}

int play_back(int test, void *arg)
{
    char in_buff[32] = "testlhjtest";

    int in_fd = *(int *)arg;
    int rcv_size = 32;
    char rcv_buff[33];
    int rv;

    if ((rv = recv(in_fd, rcv_buff, rcv_size, 0)) < 0) {
        perror("send");
        return -1;
    } else if (!rv) {
        lt_io_remove(base, eventarray[in_fd]);
        close(in_fd);
    } else if (rv != rcv_size) {
        printf("rcv_size:%d\n", rv);
        rcv_buff[33] = '\0';
        write(STDOUT_FILENO, rcv_buff, rv);
    } else if (rv == rcv_size) {
        printf("perfect:%d\n", rv);
    }

    if ((rv = send(in_fd, in_buff, 32, 0)) < 0) {
        fprintf(stderr, "send rv:%d\n", rv);
        perror("send");
        return -1;
    }
//    printf("%s\n", in_buff);

    return 0;
}
