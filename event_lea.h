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
#include <signal.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef struct lt_memory_pool {
	uintptr_t start, end;
	uintptr_t pos;

	struct lt_memory_pool *next;
	struct lt_memory_pool_manager *manager;
} lt_memory_pool_t;

typedef struct lt_memory_pool_manager {
	lt_memory_pool_t *cur;
	size_t one_element_size;
	size_t element_count;
	struct lt_memory_pool *head;
	struct lt_memory_pool *tail;
	struct lt_memory_pool_manager *heterogeneous_next;
} lt_memory_pool_manager_t;

int accept4(int, struct sockaddr *, socklen_t *, int flags);

#define debug_print(fmt, ...) \
    do {if (DEBUG) \
            fprintf(stderr, fmt, __VA_ARGS__); \
    }  while(0)

#define EVENT_POOL_LENGTH (64)
#define MAX_ACTIVE
#define MAX_READY
#define DEFAULT_BUF_SIZE (16384)
#define INIT_EPEV (512) //copy from Nginx1.7.6
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

typedef struct lt_buffer {
    char start[DEFAULT_BUF_SIZE];
    char *pos;
    char *last;
    char *end;
    int head;
    int written;
    struct lt_buffer *next;
    struct lt_memory_pool_manager *pool;
} lt_buffer_t;

typedef struct lt_chain {
//    struct lt_buffer *buf;
    struct iovec buf;
    struct lt_chain *next;
    struct lt_memory_pool *pool;
    int chain_len;
} lt_chain_t;

typedef struct lt_fd_with_chains {
    struct lt_chain *chain;
    int fd;
} lt_send_t;
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

lt_buffer_t *lt_new_buffer_chain(lt_memory_pool_manager_t *, size_t);
lt_buffer_t *lt_new_buffer(lt_memory_pool_manager_t *);

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

    struct event *next;
    struct event *prev;

    struct base  *base;
//    int     epfd;
} event_t;

typedef            int (*func_t)(struct event *, void *arg);

typedef struct min_heap {
    event_t **p;
    unsigned  n;
    unsigned  a;
} min_heap_t;
/*
typedef struct {
    lt_memory_pool_t event_pool_manager;
} ready_evlist_t;*/
//, activelist_t, evlist_t;

typedef struct {
    int event_len;
//    event_t **eventarray;
    event_t *head;
    event_t *tail;
} active_evlist_t;

/*
typedef struct {
    int event_len;
    event_t **eventarray;
} deleted_evlist_t;
*/
typedef struct base {
//active event list and its number
    active_evlist_t     activelist;//ref? //TODO: certain binary tree?
//eventlist list and its number
    //ready_evlist_t      readylist;
//deleted_evlist_t    deletedlist;
//epoll functions need it.
    struct lt_memory_pool_manager event_pool_manager;
    struct event       *free_ev_head;
    int                 epfd; 
    struct epoll_event *epevent;
    int                 eptimeout;
	lt_time_t           now;
    min_heap_t          timeheap;
    int                 timerfd;
//    int                 readylist_pos;
} base_t;

lt_time_t lt_gettime(void);
base_t*   lt_base_init(void);
event_t*  lt_io_add(base_t *base, int fd, flag_t flag_set, func_t callback, void *arg, to_t timeout);
event_t*  lt_io_mod(base_t *base, event_t *ev, flag_t flag_set, func_t callback, void *arg, to_t timeout);
int lt_new_post_callback(base_t *, func_t, int fd, void *arg);
event_t*  lt_new_event(base_t *);
void      lt_io_remove(base_t *base, event_t *ev);
res_t     lt_base_loop(base_t *base, int timeout);
lt_time_t lt_timeout_add(base_t *base, event_t *ev, to_t to);
//void      lt_free_evlist(evlist_t *list);
res_t     lt_ev_check_timeout(event_t *ev, lt_time_t timeout);
//void      lt_remove_from_readylist(event_t *, ready_evlist_t *);
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
#define LEZERO  (-2)
#define LEAGAIN (-3)
#define LEINTR  (-4)

#define L_INVALID_METHOD (-7)
#define LABORT (-6)
#define LCLOSE (-5)
#define LAPART (-4)
#define LAGAIN (-3)
#define LERROR (-2)
#define LOK    (0)


ssize_t lt_recv(int, lt_buffer_t *);
        ///*, lt_memory_manager_t *);
//, size_t);
int lt_accept(int, struct sockaddr *sockaddr);
int send_buffers(base_t *, int fd, lt_buffer_t *buf);
int send_chains(base_t *, int fd, lt_chain_t **);

int lt_set_reuseaddr(int fd, int yes);
int lt_set_reuseport(int fd, int yes);
int lt_set_keepalive(int fd, int yes);
int lt_ignore_sigpipe(void);

struct lt_string {
    int len;
    int free;
    char buf[];
};
#endif

