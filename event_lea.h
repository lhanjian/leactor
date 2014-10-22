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
#include <ctype.h>
//system independence
#include <sys/epoll.h>
#include <sys/uio.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define MAX_ACTIVE
#define MAX_READY
#define DEFAULT_BUF_SIZE (16384)
#define INIT_EPEV (512)
#define EVLIST_LEN (512*512)
#define EPEV_MAX EVLIST_LEN
#define LV_LAG (0x0000008)
#define LV_CONN (0x0000004)
#define LV_FDRD (0x0000001)
#define LV_FDWR (0x0000002)
#define LV_ONESHOT (0x0000010)
#define INF (LONG_MAX)
#define NO_TIMEOUT (-1L)
#define NULL_ARG (NULL)
#define DEFAULT_MMAP_THRESHOLD_MAX (4*1024*1024)
#define UNDELETED (0)

#define LEZERO  (-2)
#define LEAGAIN (-3)
#define LEINTR  (-4)
typedef struct lt_buffer {
    char *start;
    char *pos;
    char *last;
    char *end;
    int head;
    int written;
    char buf[DEFAULT_BUF_SIZE];
    struct lt_buffer *next;
} lt_buffer_t;
//#define 
//static funtion didn't dispatch return value;
//reduce passing parameter;
typedef struct timespec  lt_time_t;
typedef int              numlist_t;
typedef int              epfd_t;
typedef numlist_t        numactlist_t;
typedef numlist_t        numeeadylist_t;

typedef int res_t;
/*typedef union {
    int error;
    int correct;
} res_t;*/
typedef long to_t;

typedef struct lt_memory_piece {
    struct lt_memory_piece *next;
} lt_memory_piece_t;

typedef struct lt_memory_pool {
    size_t one_item_size;

    lt_memory_piece_t *head;
    lt_memory_piece_t *pos;
    lt_memory_piece_t *tail;

    void *all_item;

    struct lt_memory_pool  *next;
    struct lt_memory_pool  *manager;
} lt_memory_pool_t, lt_memory_manager_t;

lt_memory_manager_t *
                  lt_new_memory_pool_manager(lt_memory_manager_t *);
lt_memory_pool_t* lt_new_memory_pool(size_t one_item_size, lt_memory_manager_t *manager, lt_memory_pool_t *);
void*             lt_alloc(lt_memory_pool_t *pool, lt_memory_manager_t *manager);
void              lt_free(lt_memory_pool_t *pool, void *object_contents);
void              lt_destroy_memory_pool(lt_memory_pool_t *pool, lt_memory_pool_t *manager);

lt_buffer_t *lt_new_buffer_chain(lt_memory_pool_t *, lt_memory_pool_t *, size_t);
lt_buffer_t *lt_new_buffer(lt_memory_pool_t *, lt_memory_pool_t *);

typedef int flag_t;
typedef struct event {
    int         (*callback)(struct event *, void *arg);
    void         *arg;
    flag_t        flag;
    int           fd; 
    int           min_heap_idx;
    lt_time_t     endtime;
    int           deleted;
    int           pos_in_ready;
    struct event *next_active_ev;
//    int     epfd;
} event_t;

typedef            int (*func_t)(struct event *, void *arg);

typedef struct min_heap {
        event_t **p;
        unsigned  n;
        unsigned  a;
} min_heap_t;

typedef struct {
    int event_len;
    lt_memory_pool_t *event_pool;
    lt_memory_pool_t event_pool_manager;
//   int             hole_len;
//    event_t      ***hole_list;//deleted position
} ready_evlist_t;//, activelist_t, evlist_t;

typedef struct {
    int event_len;
    event_t **eventarray;
    event_t *head;
} active_evlist_t;


typedef struct {
    int event_len;
    event_t **eventarray;
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
    int                 timerfd;
//    int                 readylist_pos;
} base_t;

lt_time_t lt_gettime(void);
base_t*   lt_base_init(void);
event_t*  lt_io_add(base_t *base, int fd, flag_t flag_set, func_t callback, void *arg, to_t timeout);
void      lt_io_remove(base_t *base, event_t *ev);
res_t     lt_base_loop(base_t *base, int timeout);
lt_time_t lt_timeout_add(base_t *base, event_t *ev, to_t to);
//void      lt_free_evlist(evlist_t *list);
res_t     lt_ev_check_timeout(event_t *ev, lt_time_t timeout);
//res_t     lt_remove_from_readylist(event_t *ev, ready_evlist_t *evlist);
//res_t     lt_remove_from_readylist(event_t *ev, active_evlist_t *evlist);
#define time_a_gt_b(X,Y,Z) ((long long)X Y (unsigned long long)Z)
//#define lt_time_add(X, Y) ((lt_time_t)(X))
lt_time_t lt_time_addition(lt_time_t , to_t);
long long lt_time_a_sub_b(lt_time_t a, lt_time_t b);
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
#define LABORT (-6)
#define LCLOSE (-5)
#define LAPART (-4)
#define LAGAIN (-3)
#define LERROR (-2)
#define LOK    (0)
#define L_INVALID_METHOD (-7)


ssize_t lt_recv(int, lt_buffer_t *, size_t);
int lt_accept(int, struct sockaddr *sockaddr);

struct lt_string {
    int len;
    int free;
    char buf[];
};
#endif

