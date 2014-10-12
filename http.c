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

http_t *http_new(base_t *base)
{
    http_t *http = calloc(1, sizeof http_t);
    if (!http) {
        perror("malloc http");
        return NULL;
    }
    return http;
}
