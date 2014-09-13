//Name: Leactor
//Author: lhanjian
//Start 20130407
#ifndef _LEA_EVENT_H_INCLUDED_
#define _LEA_EVENT_H_INCLUDED_
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
//system independence
#include <sys/epoll.h>
#define MAX_ACTIVE
#define MAX_READY

#define INIT_EPEV 32
#define EVLIST_LEN 4096
#define EPEV_MAX EVLIST_LEN
//static funtion didn't dispatch return value;
//reduce passing parameter;
typedef struct timespec 
                  lt_time_t;
typedef int       numlist_t;
typedef int       epfd_t;
typedef numlist_t numactlist_t;
typedef numlist_t numreadylist_t;
typedef int (*func_t)(int fd, void *arg);

typedef int res_t;
/*typedef union {
    int error;
    int correct;
} res_t;*/

typedef int flag_t;
typedef struct event {
    func_t  callback;
    void   *arg;
    flag_t  flag;
    int     fd;
} event_t;

typedef struct evlist {
    int       event_len;
    event_t **eventarray;
} readylist_t, activelist_t, evlist_t;

typedef struct base {
//active event list and its number
    activelist_t       *activelist;
//eventlist list and its number
    readylist_t        *readylist;
//epoll functions need it.
    epfd_t              epfd; 
    struct epoll_event *epevent;
    struct timeval      eptimeout;
	lt_time_t           now;
    
} base_t;
/*
//initialize a base
res_t base_init(base_t *base_null);

//push a io event to base
res_t io_add(base_t *base_rlve, int fd, flag_t flag_set,
        func_t callback, (void *) arg);

//core dispatch
res_t base_dispatch(base_t *base_dispatch
        /);
        */
/*ToDo001 timeout to limit dispatch time?*/

//Free a base*/
/*
res_t base_free(base_t *base_rlve);
*/
#endif
