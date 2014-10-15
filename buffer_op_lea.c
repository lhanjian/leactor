#include "event_lea.h"

lt_buffer_t *lt_new_buffer(lt_memory_pool_t *pool, 
        lt_memory_pool_t *manager);

lt_buffer_t *lt_new_buffer_chain(lt_memory_pool_t *pool,
        lt_memory_pool_t *manager, size_t size)
{
    lt_buffer_t *buf = lt_new_buffer(pool, manager);
    if (buf == NULL) {
        return NULL;
    }

    buf->head = 1;

    if (size > DEFAULT_BUF_SIZE) {
        size_t now = DEFAULT_BUF_SIZE;
        lt_buffer_t *old_buf = buf;
        lt_buffer_t *new_buf;

        while (now < size) {
            new_buf = lt_new_buffer(pool, manager);
            now += DEFAULT_BUF_SIZE;
            old_buf->next = new_buf;
            old_buf = new_buf;
        }
        new_buf->next = old_buf;
    }

    
    return buf;
}

lt_buffer_t *lt_new_buffer(lt_memory_pool_t *pool, 
        lt_memory_pool_t *manager)
{
    lt_buffer_t *buf = lt_alloc(pool, manager);
    if (buf == NULL) {
        return NULL;
    }

    buf->pos = buf->start;
    buf->last = buf->end;
    buf->end = buf->last + DEFAULT_BUF_SIZE;
    buf->next = buf;

    return buf;
}

int send_buffer_chains_loop(int fd, lt_buffer_t *out_buf)
{
    size_t length;

    ssize_t rv = writev(fd, out_vector, vector_length);
    if (rv < length) {
        send_buffer_chains_loop(fd, out_buf);
    } else if (rv == -1) {
        perror("writev");
    }


}
