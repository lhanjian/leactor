#ifndef _INCLUDE_LEA_HEAP_H
#define _INCLUDE_LEA_HEAP_H
#include "event_lea.h"

void min_heap_constructor_(min_heap_t *timeheap);

void min_heap_elem_init_(event_t *ev);

int  min_heap_push_(min_heap_t *s, event_t *e);

int  min_heap_reserve_(min_heap_t *s, unsigned n);

void min_heap_shift_up_(min_heap_t *s, unsigned hole_index, event_t *e);

void min_heap_shift_down_(min_heap_t *s, unsigned hole_index, event_t *e);
#endif
