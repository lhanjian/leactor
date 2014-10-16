#include "http.h"

lt_buffer_t *lt_new_buffer(lt_memory_pool_t *pool, 
        lt_memory_pool_t *manager);

ssize_t pospone_send_buffer_chains_loop(int fd, lt_buffer_t *out_buf);

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

    buf->head = 0;
    buf->written = 0;


    return buf;
}

ssize_t send_buffer_chains_loop(int fd, lt_buffer_t *out_buf)
{
    size_t length;

    int n = 1;
    
    for (lt_buffer_t *buf = out_buf; 
            buf != buf->next && buf->next != out_buf; 
            n++, buf = buf->next) { }

    struct iovec out_vector[n];

    for (int i = 0; i < n; i++) {
        out_vector[i].iov_base = out_buf->pos;
        out_vector[i].iov_len = out_buf->last - out_buf->pos;
        length += out_vector[i].iov_len;
    }

    ssize_t rv = writev(fd, out_vector, n);
    if (0 < rv && rv < length) {
        ssize_t written_count = rv / DEFAULT_BUF_SIZE;
        ssize_t written_offset = rv % DEFAULT_BUF_SIZE;
        ssize_t remain = length - rv;
        int i;
        for (i = 0; i < written_count; i++) {
            ((lt_buffer_t *)out_vector[i].iov_base)->written = 1;
        }
        ((lt_buffer_t *)out_vector[i].iov_base)->pos += written_offset;
        
//        pospone_send_buffer_chains_loop(fd, out_buf);

        return remain;
        //Double Choice
        //A, keep it loop in event_list to send
        //B, Watch writable event and send
    } else if (rv == -1) {
        int errsv = errno;
        switch (errsv) {
            case EAGAIN: return LEAGAIN;
            case EINTR:  return LEINTR;;
            defaut: return;
        }

    } else if (!rv) {
        fprintf(stderr, "writev returned zero\n");
        return LEZERO;
    }


}
