#include "event_lea.h"
#include <stdarg.h>
//EVLIST_LEN
//ToDo Optimization 8 times

static void init_pool_list(lt_memory_pool_t *pool);

/*typedef struct {
    size_t one_item_size;

    lt_memory_piece_t *head;
    lt_memory_piece_t *pos;
    lt_memory_piece_t *tail;

    void *all_item;

    lt_memory_pool_t  *next;
} lt_memory_pool_t;*/

lt_memory_pool_t *
lt_new_memory_pool(size_t one_item_size, lt_memory_pool_t *manager, lt_memory_pool_t *pos)
{
//    lt_memory_pool_t *new = 
/*    va_list ap;
    va_start(ap, manager);*/

    lt_memory_pool_t *new;
//    lt_memory_pool_t *pos = va_arg(ap, lt_memory_pool_t *);
//    va_end(ap);

    if (!pos) {
        new = malloc(sizeof(lt_memory_pool_t));
    } else {
        new = pos;
    }

    new->one_item_size = one_item_size;

    init_pool_list(new);

    if (manager->next == NULL) {
        manager->next = new;
    }

    new->next = manager->next;

    return new;
}

void *
lt_alloc(lt_memory_pool_t *pool, lt_memory_pool_t *manager)
{
    char *alloc_rv;

    if (((lt_memory_piece_t *)(pool->pos))->next) {
        alloc_rv = (char *)pool->pos->next + sizeof(lt_memory_piece_t);
        pool->pos = pool->pos->next;
    } else {
        pool->next = lt_new_memory_pool(pool->one_item_size, manager, NULL);
        pool = pool->next;//单向循环或者双向?TODO
        pool->next = manager;
        alloc_rv = lt_alloc(pool, manager);
    }

    return alloc_rv;
}
    /*
    if (pool->pos){
        if (pool->pos->single_item != pool->tail->single_item) {


        }
        char *memory = pool->next;
        pool->pos = pool->next;//((char *)pool->pos) + pool->one_item_size;
        return memory;

    } else {
        pool->next = lt_new_memory_pool(pool->one_item_size);
        pool = pool->next;

        return lt_alloc(pool);
    }
    */

static void init_pool_list(lt_memory_pool_t *pool)
{
    pool->next = NULL;
    size_t size_of_pool_with_all_item = 
       /*EVLIST_LEN */ (pool->one_item_size  + sizeof(lt_memory_piece_t));

    pool->all_item = malloc(EVLIST_LEN * size_of_pool_with_all_item);
    pool->head = pool->all_item;
    pool->pos = pool->head;
    pool->tail = (lt_memory_piece_t *)
        ((char *)pool->pos + size_of_pool_with_all_item * EVLIST_LEN);

    lt_memory_piece_t *p = pool->head;
    for (int i = 0; i < EVLIST_LEN - 1; i++) {
        p->next = (lt_memory_piece_t *)
            ((char *)p + size_of_pool_with_all_item);
        p = p->next;
    }
    p->next = NULL;
}

void 
lt_free(lt_memory_pool_t *pool, void *object_contents)
{
//    void *old_next = pool->next;
    char *object_pos = (char *)object_contents - pool->one_item_size;

    lt_memory_piece_t *old_next = pool->pos->next;
    pool->pos->next = (lt_memory_piece_t *)object_pos;
    ((lt_memory_piece_t *)object_pos)->next = old_next;
}


void lt_destroy_memory_pool(lt_memory_pool_t *pool, 
        lt_memory_pool_t *manager)
{
    /*
    free(pool->all_item);
    free(pool->next);//TODO cirular list
    free(pool);
    */
};//TODO

lt_memory_pool_t *lt_new_memory_pool_manager(lt_memory_pool_t *pos)
{
    lt_memory_pool_t *manager;
    if (!pos) {
        manager = malloc(sizeof(lt_memory_pool_t)); 
    } else {
        manager = pos;
    }

    manager->next = NULL;
    return manager;
}
