#include "event_lea.h"

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int incoming(int, void *);
int play_back(int, void *);

base_t *base;

int main(void)
{
    int my_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (listen(my_sock,SOMAXCONN)) {
        perror(NULL);
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_storage local_addr;

    socklen_t local_addrlen = sizeof(local_addr);

    bind(my_sock, (struct sockaddr *)&local_addr, local_addrlen);

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
