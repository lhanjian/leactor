#include "event_lea.h"
//EVLIST_LEN
//ToDo Optimization 8 times

static void init_pool_list(lt_memory_pool_t *pool);

typedef struct {
    size_t one_item_size;

    lt_memory_piece_t *head;
    lt_memory_piece_t *pos;
    lt_memory_piece_t *tail;

    void *all_item;

    lt_memory_pool_t  *next;
} lt_memory_pool_t;

lt_memory_pool_t *
lt_new_memory_pool(size_t one_item_size)
{
//    lt_memory_pool_t *new = 
    lt_memory_pool_t *new = malloc(sizeof(lt_memory_pool_t));

    new->one_item_size = one_item_size;

    init_pool_list(new);

    return new;
}

void *
lt_alloc(lt_memory_pool_t *pool)
{
    char *allo_rv;

    if (((lt_memory_piece_t *)(pool->pos))->next) {
        alloc_rv = (char *)pool->pos->next + sizeof(lt_memory_piece_t);
        pool->pos = pool->pos->next;
    } else {
        pool->next = lt_new_memory_pool(pool->one_item_size);
        pool = pool->next;//单向循环或者双向?TODO

        alloc_rv = lt_alloc(pool);
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
    pool->head = all_item;
    pool->pos = pool->head;
    pool->tail = (char *)pool->pos + size_of_pool_with_all_item;

    lt_memory_piece_t *p = pool->head;
    for (int i = 0; i < EVLIST_LEN - 1; i++) {
        p->next = (char *)p + size_of_pool_with_all_item;
        p = p->next;
    }
    p->next = NULL;
}


void 
lt_free(lt_memory_pool_t *pool, void *object_contents)
{
//    void *old_next = pool->next;

    char *object_pos = (char *)object_contents - pool->one_item_size;

    char *old_next = pool->pos->next;
    pool->pos->next = object_pos;
    (lt_memory_piece_t *)object_pos->next = old_next;
}


void lt_delete_memory_pool(lt_memory_pool_t *pool)
{
    


};//TODO





