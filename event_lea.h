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


#define LV_FDRD (0x0000001)
#define LV_FDWD (0x0000002)
#define INF (NULL)
#define NULL_ARG (NULL)
//#define 
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
typedef int to_t;

typedef int flag_t;
typedef struct event {
    func_t  callback;
    void   *arg;
    flag_t  flag;
    int     fd;
//    int     epfd;
} event_t;

typedef struct evlist {
    int             event_len;
    event_t        *eventarray[];
    struct evlist   hole_list;
} readylist_t, activelist_t, evlist_t;

typedef struct base {
//active event list and its number
    activelist_t        activelist;//ref? //TODO: certain binary tree?
//eventlist list and its number
    readylist_t         readylist;
//epoll functions need it.
    int                 epfd; 
    struct epoll_event  epevent;
    int                 eptimeout;
	lt_time_t           now;
} base_t;

lt_time_t lt_gettime(void);
int       lt_time_a_sub_b(lt_time_t a, lt_time_t b);
base_t*   lt_base_init(void);
res_t     lt_io_add(base_t *base, int fd, flag_t flag_set, func_t callback, void *arg, to_t *timeout);
res_t     lt_base_loop(base_t *base, int timeout);

void      lt_base_free_evlist(evlist_t *list);
#define time_a_gt_b(X) (X)
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
