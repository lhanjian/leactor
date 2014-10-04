//Name: Leactor
//Author: lhanjian
//Start 20130407
#ifndef _LEA_EVENT_H_INCLUDED_
#define _LEA_EVENT_H_INCLUDED_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
//system independence
#include <sys/epoll.h>
#define MAX_ACTIVE
#define MAX_READY

#define INIT_EPEV (512)
#define EVLIST_LEN (DEFAULT_MMAP_THRESHOLD_MAX)
#define EPEV_MAX EVLIST_LEN

#define LV_CONN (0x0000004)
#define LV_FDRD (0x0000001)
#define LV_FDWR (0x0000002)
#define INF (LONG_MAX)
#define NULL_ARG (NULL)
#define DEFAULT_MMAP_THRESHOLD_MAX (4*1024*1024)
#define DELETED (0)
//#define 
//static funtion didn't dispatch return value;
//reduce passing parameter;
typedef struct timespec  lt_time_t;
typedef int       numlist_t;
typedef int       epfd_t;
typedef numlist_t numactlist_t;
typedef numlist_t numeeadylist_t;
typedef int (*func_t)(int fd, void *arg);

typedef int res_t;
/*typedef union {
    int error;
    int correct;
} res_t;*/
typedef long to_t;

typedef int flag_t;
typedef struct event {
    func_t       callback;
    void        *arg;
    flag_t       flag;
    int          fd; 
    int          min_heap_idx;
    lt_time_t    endtime;
    int          deleted;
    int          pos_in_ready;
//    int     epfd;
} event_t;

typedef struct min_heap {
        event_t **p;
        unsigned  n;
        unsigned  a;
} min_heap_t;

typedef struct {
    int event_len;
    event_t **eventarray;
    event_t *event_head;
//   int             hole_len;
//    event_t      ***hole_list;//deleted position
} ready_evlist_t;//, activelist_t, evlist_t;

typedef struct {
    int event_len;
    event_t ***eventarray;
} active_evlist_t;


typedef struct {
    int event_len;
    event_t ***eventarray;
} deleted_evlist_t;

typedef struct base {
//active event list and its number
    active_evlist_t     activelist;//ref? //TODO: certain binary tree?
//eventlist list and its number
    ready_evlist_t      readylist;
    deleted_evlist_t    deletedlist;
//epoll functions need it.
    int                 epfd; 
    struct epoll_event  *epevent;
    int                 eptimeout;
	lt_time_t           now;
    min_heap_t          timeheap;
//    int                 readylist_pos;
} base_t;

lt_time_t lt_gettime(void);
long      lt_time_a_sub_b(lt_time_t a, lt_time_t b);
base_t*   lt_base_init(void);
event_t*  lt_io_add(base_t *base, int fd, flag_t flag_set, func_t callback, void *arg, to_t timeout);
res_t     lt_base_loop(base_t *base, long timeout);
lt_time_t lt_timeout_add(base_t *base, event_t *ev, to_t to);
//void      lt_free_evlist(evlist_t *list);
res_t     lt_ev_check_timeout(event_t *ev, lt_time_t timeout);
res_t     lt_remove_from_readylist(event_t *ev, ready_evlist_t *evlist, deleted_evlist_t *deletedlist);
//res_t     lt_remove_from_readylist(event_t *ev, active_evlist_t *evlist);
#define time_a_gt_b(X) (X)
//#define lt_time_add(X, Y) ((lt_time_t)(X))
lt_time_t lt_time_addition(lt_time_t , to_t);
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

