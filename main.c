#include "event_lea.h"
#include "http.h"

int child(conf_t *conf);

base_t *father(conf_t *conf);

int main()
{
    int efd = eventfd(0, EFD_NONBLOCK);
    conf_t *conf = malloc(sizeof(conf_t));
    conf->efd_distributor = efd;
/*
    if (pipe(conf->pfd)) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    */

    base_t *base = father(conf);
    switch (fork()) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
            break;
        case 0:
            sleep(15);
            child(conf);
            exit(EXIT_SUCCESS);
        default:
            lt_base_loop(base, NO_TIMEOUT);
            break;
    }
    return 0;
}

base_t *father(conf_t *conf/*start restart ...  other conf*/)
{
   base_t *base = lt_base_init();
   if (!base) {
       fprintf(stderr, "Create base failed\n");
       exit(EXIT_FAILURE);
   }

   http_t *father = http_master_new(base, conf);
   if (father == NULL) {
       fprintf(stderr, "father's http_t fail to create\n");
       exit(EXIT_FAILURE);
   }

   return base;
}

int child(conf_t *conf)
{
    base_t *base = lt_base_init();
    if (base == NULL) {
        fprintf(stderr, "child base_init error");
        exit(EXIT_FAILURE);
    }
    http_t *child = http_worker_new(base, conf);
    if (child == NULL) {
        fprintf(stderr, "child's http_t fail to create\n");
        exit(EXIT_FAILURE);
    }

    lt_base_loop(base, NO_TIMEOUT);
    return 0;
}
