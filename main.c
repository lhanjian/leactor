#include "event_lea.h"
#include "http.h"
/*
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
*/
int child(conf_t *conf);
int father(conf_t *conf);
int main()
{
    int efd = eventfd(0, EFD_NONBLOCK);
    conf_t *conf = malloc(sizeof(conf_t));
    conf->efd_distributor = efd;

    if (pipe(conf->pfd)) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    switch (fork()) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
            break;
        case 0:
            child(conf);
            exit(EXIT_SUCCESS);
        default:
            father(conf);
            break;
    }
    return 0;
}

int father(conf_t *conf/*start restart ...  other conf*/)
{
   base_t *base = lt_base_init();
   if (!base) {
       fprintf(stderr, "Create base failed\n");
       exit(EXIT_FAILURE);
   }

   http_t *main = http_master_new(base, conf);
   if (!main) {
       fprintf(stderr, "main http_t fail to create\n");
       exit(EXIT_FAILURE);
   }
   
   lt_base_loop(base, NO_TIMEOUT);
   return 0;
}

int child(conf_t *conf)
{
    base_t *base = lt_base_init();
    if (base == NULL) {
        fprintf(stderr, "child base_init error");
        exit(EXIT_FAILURE);
    }

    lt_base_loop(base, NO_TIMEOUT);
    return 0;
}
