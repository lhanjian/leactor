#include "event_lea.h"
#include <stdint.h>

typedef struct obj {
    int32_t a;
    int32_t b;
} obj_t;
int main(void)
{
    lt_memory_pool_t *pool = lt_new_memory_pool(sizeof(obj_t));

    obj_t *block_01 = lt_alloc(pool);
    block_01->a = 133;
    block_01->b = 145;

    printf("block_01:add %p:%d\n", block_01, block_01->a);
    printf("block_01:add %p:%d\n", block_01, block_01->b);

    obj_t *block_02 = lt_alloc(pool);
    block_02->a = 145;
    block_02->b = 1450;

    printf("block_02:add %p:%d\n", block_02, block_02->a);
    printf("block_02:add %p:%d\n", block_02, block_02->b);

    lt_free(pool, block_01);
    printf("block_01:add %p:%d\n", block_01, block_01->a);

    obj_t *block_03 = lt_alloc(pool);
    printf("block_03:add %p:%d\n", block_03, block_03->a);

    lt_destroy_memory_pool(pool);

    return 0;
}
