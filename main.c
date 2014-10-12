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
int child();
int father();

int main()
{
    int rv = fork();
    switch (rv) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
            break;
        case 0:
            child();
        default:
            father();
            break;
    }
    return 0;
}

int child(/*start restart ...  other conf*/)
{

   base_t *base = lt_base_init();
   if (!base) {
       fprintf(stderr, "Create base failed\n");
       return EXIT_FAILURE;
   }

   conf_t *conf = NULL;
   http_t *main = http_new(base, conf);
   if (!main) {
       fprintf(stderr, "main http_t fail to create\n");
       return EXIT_FAILURE;
   }



    return 0;
}

