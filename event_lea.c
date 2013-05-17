//Name: Leactor
//Author: lhanjian
//Start 20130407

#include "event_lea.h"

static int
lt_alloc_evlist(evlist_t **evlist)
{
	*evlist = realloc(NULL, sizeof(evlist_t));
	if (!(*evlist)) {
		perror("realloc");
		return -1;
	}
	**evlist = (void *)0;
	return 0;
}

res_t
lt_base_init_set(base_t *base)
{
	if (lt_alloc_evlist(&base->readylist) == -1)
		return -1;
	if (lt_alloc_evlist(&base->activelist) == -1) {
		free(activelist);
		return -1;
	}

    base->readylist->event_len = 0;
    base->activelist->event_len = 0;

    return 0;
}

res_t
lt_add_to_epfd(base_t *base, int mon_fd, flag_t flag)
{
    res_t res;
    struct epoll_event *ep_event = &base->ep_event;

    events = 0;
    if (flag & LV_FDRD)
        ep_event.events |= EPOLLIN;
    if (flag & LV_FDWR)
        ep_event.events |= EPOLLOUT;
    ep_event_events |= EPOLLET;
    
    res = epoll_ctl(base->epfd, EPOLL_CTL_ADD, mon_fd, &ep_event);
	if(res) {
		perror("epoll_ctl");
		return res;
	}

    return res;
}

static res_t
lt_ev_ctr(event_t *event, 
        flag_t flag_set, int fd, 
        func_t callback, void *arg)
{
    event = realloc(NULL, sizeof(event_t));
    if (event == -1) {
        err_realloc(event);
        return -1;
    }

    event->callback = callback;
    event->arg = arg;
    event->flag = flag_set;
    event->fd = fd;

    return 0;
}

static inline res_t
lt_io_ctr(evlist_t *evlist)
{
	if (!evlist) {
		if (lt_alloc_evlist(&evlist) == -1)
			return -1;
	}
    if (!evlist->event_len || evlist->event_len == EVLIST_LEN) {
        evlist->eventarray = realloc(evlist->eventarray,
                sizeof(*eventarray)*(evlist->event_len>>2));
        if (evlist == -1) {
			perror("realloc");
            return -1;
        }
		++evlist->event_len;
    }

    return 0;
}
//evlist belongs to base, but it can make evlist_t opaque,
//so pass the evlist to this routine
static res_t
lt_add_to_evlist(event_t *event, evlist_t *evlist, base_t *base, 
    flag_t flag_set, int fd, func_t callback, (void *)arg)
{
    res_t res;

    res = lt_ev_ctr(event, flag_set, fd, callback, arg);
    if (res)
        return res;

    res = io_ctr(evlist);
    if (res)
        return res;

    evlist->eventarray[event_len] = event;
    ++evlist->event_len;

    return res;
}


//initialize a base
base_t *
lt_base_init()
{
    res_t res;
//alloc a memory for a new base 
    base_t *base = realloc(NULL, sizeof(base_t));
    if (!base) {
        err_realloc(base_init);
        return -1;
    }

//epoll create a epfd , then copy it to base
    epfd_t epfd = epoll_create1(EPOLL_CLOEXEC);
    if (base == -1) {
		fprintf(stderr, "epoll_create1\n");
        free(base_init, NULL);
        return -1
    }
    base->epfd = epfd;

//init base set
    res = base_init_set(base_t *base);
//init epoll_event
	base->epevent = malloc(N*sizeof(struct epoll_event));
	if (!base->epevent) {
		fprintf(stderr, "malloc\n");
		return -1;
	}

    return base;
}
    

//push a io event to base
res_t 
lt_io_add(base_t *base, int fd, flag_t flag_set,
        func_t callback, void *arg, to_t *timeout)
{

	event_t *event = malloc(sizeof(event_t));
    res_t    res = lt_add_to_evlist(event, base->readylist, 
			base, flag_set, fd, callback, arg);
	
	if (timeout)
		lt_timeout_add(timeout);
    
    res = lt_add_to_epfd(base, fd, flag_set);

    return res;
}

static//res_t
lt_ev_process_and_moveout(evlist_t *evlist)
{
    int len = evlist->event_len;
    for (int i = 0; i < len; i++) {
        event_t *event = evlist->eventarray[i];
		if (lt_ev_check_timeout(event)) {
			--evlist->event_len;
			continue;
		}
        event->callback(event->fd, event->arg);
        --evlist->event_len;
    }
}
static inline void
lt_base_init_actlist(base_t *base, int ready)
{
	evlist_t *actlist = base->activelist;
	evlist_t *readylist = base->readylist;
//TODO
	for (int i = 0; i < ready; i++) {
		actlist->eventarray[i] = 
			readylist->eventarray[i];
	}
	actlist->event_len = ready;
}
//core dispatch
res_t 
lt_base_loop(base_t *base, lt_time_t timeout)
{
	lt_time_t now;

    int i;
    int ready;

    for (;;) {
		//get time now
		now = lt_gettime();

		//core dispatch
        ready = epoll_wait(base->epfd, base->epevent, 
				base->readylist.event_len, base->eptimeout);
        if (ready == -1) {
			perror("epoll_wait");
            return -1;
        }

		//calc time cosumed
		now = lt_time_a_sub_b(lt_gettime(), now);
		if (time_a_gt_b(now > timeout)) {
			fprintf(stderr, "loop expired\n");
			break;
		}
        
		lt_base_init_actlist(base, ready);//should init ,but not only insert ready to action.

        lt_ev_process_and_moveout(base->activelist);
    }

    return 0;
}
/*ToDo001 timeout to limit dispatch time?*/

//gettime
lt_time_t
lt_gettime()
{
	//TODO
	return lt_time_t;
}

//remove base
void
lt_base_free(base_t *base)
{
	lt_free_evlist(&base->readylist);
	lt_free_evlist(&base->activelist);

	free(base);
}

//remove event
