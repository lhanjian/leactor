//Author: lhanjian
//Start 20130407

#include "event_lea.h"

#include <errno.h>
/*
static int
lt_alloc_evlist(evlist_t *evlist)
{
	evlist = realloc(NULL, sizeof(evlist_t));
	if (!(*evlist)) {
		perror("realloc");
		return -1;
	}
	evlist->event_len = 0;
    evlist->hole_len = 0;
	return 0;
}
    */

res_t
lt_base_init_set(base_t *base)
{
	if (lt_alloc_evlist(&base->readylist) == -1)
		return -1;
	if (lt_alloc_evlist(&base->activelist) == -1) {
//		free(&base->activelist);
		return -1;
	}

    base->readylist.event_len = -1;
    base->activelist.event_len = -1;

    return 0;
}

res_t
lt_add_to_epfd(int epfd, event_t *event, int mon_fd, flag_t flag)
{
    res_t res;
    struct epoll_event ev;//

    ev.events = 0;
    if (flag & LV_FDRD)
        ev.events |= EPOLLIN;
    if (flag & LV_FDWR)
        ev.events |= EPOLLOUT;
    if (flag & LV_CONN) { 
        ev.events |= EPOLLET;//maybe need EPOLL LT
    }
    
    ev.data.ptr = event;
    
    res = epoll_ctl(epfd, EPOLL_CTL_ADD, mon_fd, &ev);
	if(res) {
		perror("epoll_ctl");
		return res;
	}

    return res;
}

static res_t
lt_ev_constructor_(event_t *event, 
        flag_t flag_set, int fd, //int epfd,
        func_t callback, void *arg, int deleted)
{
//    event = malloc(sizeof(event_t));
    if (event == NULL) {
    //    err_realloc(event);
        return -1;
    }

    event->callback = callback;
    event->arg = arg;
    event->flag = flag_set;
    event->fd = fd;
    event->deleted = deleted;

    return 0;
}

static inline res_t
lt_eventarray_constructor_(evlist_t *evlist)
{
/* 	if (!evlist) {
		if (lt_alloc_evlist(evlist) == -1)
			return -1;
*/	}
    if (evlist->event_len == -1)/* || evlist->event_len == EVLIST_LEN)*/ {
        evlist->eventarray = malloc(sizeof(event_t) * EVLIST_LEN);
            //malloc(realloc(evlist->eventarray,//TODO realloc is wrong
//                (sizeof(event_t)) * (evlist->event_len>>2));
        if (evlist->eventarray == NULL) {
			perror("malloc eventarray");
            return -1;
        }
	//	++evlist->event_len;
    } else if (evlist->event_len == EVLIST_LEN) {
        //TODO
        //make a memory pool?
    }

    return 0;
}

//evlist belongs to base, but it can make evlist_t opaque,
//so pass the evlist to this routine
static res_t
lt_add_to_evlist(event_t *event, evlist_t *evlist, base_t *base, 
    flag_t flag_set, int fd, func_t callback, void *arg)
{
    res_t res;

    res = lt_ev_constructor_(event, flag_set, fd, callback, arg, DELETED);
    if (res)
        return res;

    res = lt_eventarray_constructor_(evlist);
    if (res)
        return res;

    
    if (!evlist->hole_len) {
        evlist->eventarray[evlist->event_len] = event;
        ++evlist->event_len;
    } else {
        *(evlist->hole_list[evlist->hole_len-- - 1]) = event;//pop a ev
    }

    return res;
}


//initialize a base
base_t *
lt_base_init(void)
{
//    res_t res;
//alloc a memory for a new base 
    base_t *base = realloc(NULL, sizeof(base_t));
    if (!base) {
//        err_realloc(base);//TODO
        return NULL;
    }

//epoll create a epfd , then copy it to base
    int epfd = epoll_create1(EPOLL_CLOEXEC);
    if (epfd == -1) {
		fprintf(stderr, "epoll_create1\n");
        free(base);//TODO
        return NULL;
    }

    base->epfd = epfd;

//init base set
    lt_base_init_set(base);
    
    min_heap_constructor_(&base->timeheap);
//init epoll_event 
//Fxxk you, libevent;
//	base->epevent = malloc(INIT_EPEV*sizeof(struct epoll_event));
//	if (!base->epevent) {
//		fprintf(stderr, "malloc\n");
//		return NULL;
//	}

    return base;
}
    

//push a io event to base_t
//POSITION 
//evlist using TCP-like buffer window？
//if add the same fd?
event_t *
lt_io_add(base_t *base, int fd, flag_t flag_set,
        func_t callback, void *arg, to_t timeout)
{

	event_t *event = malloc(sizeof(event_t));
    if (event == NULL) {
        perror("malloc event");
        exit(-1);
    }
    res_t    res = lt_add_to_evlist(event, &base->readylist, 
			base, flag_set, fd, callback, arg);

    if (res) {
        fprintf(stderr, "lt_add_to_evlist error");
        return NULL;
    }
	
	if (timeout) {
		event->endtime = lt_timeout_add(base, event, timeout);//lt_timeout_add TODO
    }
    
    res = lt_add_to_epfd(base->epfd, event, fd, flag_set);

    return event;
}
/*
res_t
lt_io_mod(base_t *base, event_t *ev, flag_t flag_set,
        func_t callback, void *arg, to_t timeout)
{

    return 

}
*/


static void//res_t
lt_ev_process_and_moveout(base_t *base, lt_time_t nowtime)
{
    int len = base->activelist.event_len;
    event_t ***evlist = base->activelist.eventarray;
    for (int i = 0; i < len; i++) {//Why not use Tree?

        event_t *event = *evlist->eventarray[i];
        --evlist->event_len;//ev_persist  DONE/ev_oneshot  TODO

		if (lt_ev_check_timeout(event, nowtime)) {//TODO
            lt_remove_from_readylist(event, &base->readylist);
            continue;
        } else { 
            event->callback(event->fd, event->arg);
        }
    }
    return;
}

static inline void
lt_loop_init_actlist(base_t *base, struct epoll_event ev_array[], int ready)
{
	evlist_t *actlist = &base->activelist;
//	evlist_t *readylist = &base->readylist;
//TODO
//memset

    if (actlist->event_len == -1) {
        actlist->eventarray = malloc(sizeof(event_t *) * EVLIST_LEN);
        if (actlist->eventarray == NULL) {
            perror("malloc eventarray");
            exit(-1);
        }
    }

    event_t **act_ev;
	for (int i = 0; i < ready; i++) {
        act_ev = actlist->eventarray + i; 
        *act_ev = (event_t *)ev_array[i].data.ptr;
//			readylist->eventarray[i];
	}
	actlist->event_len = ready;
}
//core dispatch
res_t 
lt_base_loop(base_t *base, /*lt_time_t*/long timeout)
{
	lt_time_t start, /*now,*/ after;

    int diff;
//    int i;
    int ready;

    for (;;) {
		//get time now
		start = lt_gettime();

        /*
        int errsv = errno;
        puts(strerror(errsv));
        fprintf(stderr, "sth failed errsv:%d\n", errsv);
        */
        struct epoll_event epevents[INIT_EPEV];

		//core dispatch
        ready = epoll_wait(base->epfd, /*base->*/epevents, 
				/*base->readylist.event_len*/INIT_EPEV, -1);
        if (ready == -1) {
			perror("epoll_wait");
            return -1;
        } /*(else (ready == 0) {
            perror("epoll time out");
            return -1;
        }*/
    

		//calc loop time cosumed
        after = lt_gettime();
		diff = lt_time_a_sub_b(after, start);//SUB TODO
		if (time_a_gt_b(diff > timeout)) {
			fprintf(stderr, "loop expired\n");
			break;
		}
        
		lt_loop_init_actlist(base, epevents, ready);//should init ,but not only insert ready to action.

        lt_ev_process_and_moveout(&base->activelist, after);
    }

    return 0;
}
/*ToDo001 timeout to limit dispatch time?*/


lt_time_t
lt_gettime()
{
	//TODO done?
    int rv;
    lt_time_t time_now;

    rv = clock_gettime(CLOCK_MONOTONIC_RAW, &time_now);

    if (rv == -1) {
        perror("gettime error");
    }

	return time_now;
}

void
lt_free_evlist(evlist_t *list)
{
    free(list->eventarray);
}
//remove base
void
lt_base_free(base_t *base)
{
	lt_free_evlist(&base->readylist);
	lt_free_evlist(&base->activelist);

	free(base);
}


res_t
lt_remove_from_readylist(event_t *ev, ready_evlist_t *readylist, deleted_evlist_t *deletedlist)
{
 //   evlist->hole_list[evlist->hole_len++] = &ev;//push a pos of ev to hole_list

    return 0;
}

void//res_t
lt_io_remove(base_t *base, event_t *ev)//Position TODO
{
    lt_remove_from_evlist(ev, &base->readylist);
//    free(ev);
    //TODO readylist is too long?
}

res_t
lt_ev_check_timeout(event_t *ev, lt_time_t nowtime)
{ 
    time_t sec_diff = ev->endtime.tv_sec - nowtime.tv_sec;
    long  nsec_diff = ev->endtime.tv_nsec - nowtime.tv_nsec;

    if (sec_diff > 0) {
        return 1;
    } else if (sec_diff == 0) {
        if (nsec_diff >= 0) { return 1;}
        else {return 0;}
    } else /*(sec_diff < 0)*/ {
        return 0;
    }
}

lt_time_t
lt_timeout_add(base_t *base, event_t *ev, to_t to)//add to a tree?
{
    lt_time_t endtime = lt_time_addition(lt_gettime(), to);
    
    min_heap_elem_init_(ev);
    min_heap_push_(&base->timeheap, ev);

    return endtime;
}

lt_time_t
lt_time_addition(lt_time_t time, to_t to)
{
    long nsec, sec;
    nsec = to + time.tv_nsec;
    sec  = time.tv_sec;

    while (nsec > 1000000000L) {//1E9 LONG
        nsec = nsec - 1000000000L;
        sec += 1L;
    }

    return (lt_time_t) { 
        .tv_sec  = sec, 
        .tv_nsec = nsec 
    };
}

long 
lt_time_a_sub_b(lt_time_t a, lt_time_t b)
{
    long res;
    lt_time_t ans = {
        .tv_sec = a.tv_sec - b.tv_sec, 
        .tv_nsec = a.tv_nsec - b.tv_nsec
    };
//when a must gt b 
    res = ans.tv_nsec + ans.tv_sec * 1000000000L;
    return res;
}

