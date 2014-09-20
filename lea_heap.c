#include "event_lea.h"

typedef struct min_heap {
    event_t **p;
    unsigned  n;
    unsigned  a;
} min_heap_t;

static inline int min_heap_push_(min_heap_t *, event_t *);
static inline int min_heap_reserve_(min_heap_t *, unsigned);
static inline void min_heap_shift_up_(min_heap_t *, unsigned, event_t *);

int min_heap_push_(min_heap_t* s, event_t *e)
{
	if (min_heap_reserve_(s, s->n + 1))
		return -1;
	min_heap_shift_up_(s, s->n++, e);
	return 0;
}

void min_heap_shift_up_(min_heap_t *s, unsigned hole_index, event_t *e)
{
    unsigned parent = (hole_index - 1) / 2;
    while (hole_index && min_heap_elem_greater(s->p[parent], e)) {
    }

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

