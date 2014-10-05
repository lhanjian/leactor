#include "event_lea.h"
//EVLIST_LEN
//ToDo Optimization 8 times
typedef struct {
    void *single_item;
    lt_memory_block_t *next;
} lt_memory_piece_t;

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

char *
lt_alloc(lt_memory_pool_t *pool)
{
    if (pool->pos){
        if (pool->pos->single_item != pool->tail->single_item) {


        }
        /*
        char *memory = pool->next;
        pool->pos = pool->next;//((char *)pool->pos) + pool->one_item_size;
        */
        return memory;

    } else {
        pool->next = lt_new_memory_pool(pool->one_item_size);
        pool = pool->next;

        return lt_alloc(pool);
    }
}

static void init_pool_list(lt_memory_pool_t *pool)
{
    pool->next = NULL;

    pool->all_item = malloc(pool->one_item_size * EVLIST_LEN);
    pool->head = all_item;
    pool->pos = pool->head;
    pool->tail = pool->pos;
}


void 
lt_free(lt_memory_pool_t *pool, void *pos)
{
//    void *old_next = pool->next;

    if (pool->pos != pool->tail) {//已经存在一个空洞
    }
    pool->pos = pos;
}


void lt_delete_memory_pool(lt_memory_pool_t *pool)
{
    


};//TODO





