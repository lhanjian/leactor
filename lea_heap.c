#include "event_lea.h"
#include "lea_heap.h"
/*
typedef struct min_heap {
    event_t **p;
    unsigned  n;
    unsigned  a;
} min_heap_t;
*/
/*
static inline int min_heap_push_(min_heap_t *, event_t *);
static inline int min_heap_reserve_(min_heap_t *, unsigned);
static inline void min_heap_shift_up_(min_heap_t *, unsigned, event_t *);
static inline void min_heap_shift_down_(min_heap_t *, unsigned, event_t *);
*/

#define min_heap_elem_greater(a, b) \
        (lt_time_a_sub_b((a)->endtime, (b)->endtime))

void 
min_heap_elem_init_(event_t *ev)
{ 
    ev->min_heap_idx = -1;
    return;
}

int 
min_heap_push_(min_heap_t* s, event_t *e)
{
	if (min_heap_reserve_(s, s->n + 1))
		return -1;
	min_heap_shift_up_(s, s->n++, e);
	return 0;
}

event_t *
min_heap_pop_(min_heap_t *s)
{
    if (s->n) {
        event_t *e = *s->p;
        min_heap_shift_down_(s, 0u, s->p[--s->n]);
        e->min_heap_idx = -1;
        return e;
    } else {
        return 0;
    }
}

void 
min_heap_shift_up_(min_heap_t *s, unsigned hole_index, event_t *e)
{
    unsigned parent = (hole_index - 1) / 2;
    while (hole_index && min_heap_elem_greater(s->p[parent], e)) {//min time on time
        s->p[hole_index] = s->p[parent];
        s->p[hole_index]->min_heap_idx = hole_index;//TODO ev_timeout_pos???
        hole_index = parent;
        parent = (hole_index - 1) / 2;
    }
    s->p[hole_index] = e;
    s->p[hole_index]->min_heap_idx = hole_index;//

}

void
min_heap_shift_down_(min_heap_t * s, unsigned hole_index, event_t *e)
{
    unsigned min_child = 2 * (hole_index + 1);
    while (min_child <= s->n) {
        min_child -= 
            ((min_child == s->n) || min_heap_elem_greater(s->p[min_child], s->p[min_child - 1]));
        if (!min_heap_elem_greater(e, s->p[min_child])) break;
        s->p[hole_index] = s->p[min_child];
        s->p[hole_index]->min_heap_idx = hole_index;
        hole_index = min_child;
        min_child = 2 * (hole_index + 1);
    }
    s->p[hole_index] = e;
    s->p[hole_index]->min_heap_idx = hole_index;
}


int min_heap_reserve_(min_heap_t *s, unsigned n)
{
    if (s->a < n) {
        event_t **p;
        unsigned a = s->a? s->a*2 : 8;
        if (a < n) { a = n; }
        if ( !(p = realloc(s->p, a* sizeof *p)) ) { return -1; }
        s->p = p;
        s->a = a;
    }
    return 0;
}
