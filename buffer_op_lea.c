#include "http.h"
#include "event_lea.h"

#define LCLOSE (-5)
#define LAPART (-4)
#define LAGAIN (-3)
#define LERROR (-2)


ssize_t pospone_send_buffer_chains_loop(int fd, lt_buffer_t *out_buf);

lt_buffer_t *lt_new_buffer_chain(lt_memory_pool_t *pool,
        lt_memory_pool_t *manager, size_t size)
{
    lt_buffer_t *buf = lt_new_buffer(pool, manager);
    if (buf == NULL) {
        return NULL;
    }
//TODO
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
        new_buf->next = buf;//circular_list
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

    buf->end = buf->start + DEFAULT_BUF_SIZE;
    buf->pos = buf->start;
    buf->last = buf->start;

    buf->pool = pool;
    buf->next = buf;
    buf->head = 0;
    buf->written = 0;

    return buf;
}

lt_chain_t *send_chains(int fd, lt_chain_t *out_chain)
{
    int chain_len = 0;
    lt_chain_t *rv_chain;

    for (lt_chain_t *cur = out_chain; cur; cur = cur->next) { 
        chain_len++;
    }

    struct iovec out_vec[chain_len];

    ssize_t rv = writev(fd, out_vec, chain_len);
    if (rv == -1) {
        int errsv = errno;
        switch (errsv) {
            case EAGAIN://POST_SEND:TODO
                break;
            default:
                break;
        }
    }
    for (lt_chain_t *cur = out_chain; cur; cur = cur->next) {
        int iov_len = cur->buf.iov_len;
        if (rv > iov_len) {
            cur->buf.iov_len -= iov_len;
            rv -= iov_len;
            continue;
            //lt_free?not lt_Free until chains all has been seed out.
        } else {
            cur->buf.iov_base  = (char *)cur->buf.iov_base + iov_len;
            cur->buf.iov_len -= rv;
            rv_chain = cur;
            if (rv == iov_len) {
                rv -= iov_len;
                continue;
            } else /*if(rv < iov_len)*/ {
                return rv_chain;
            }
        }
    }


    return NULL;//all complete;
    
}

/*
int send_chains(int fd, lt_chain_t *out_chain)
{
    lt_chain_t *chain = out_chain;
    lt_chain_t *old_chain;
    lt_buffer_t *buf = out_chain->buf;

    int iov_i = 1;

    for (; chain; chain = chain->next) {
        for (; buf->next != buf; iov_i++, buf = buf->next) { };
        old_chain = chain;
    }
    lt_chain_t *tail_chain = old_chain;

    struct iovec out_vector[iov_i];

    lt_chain_t *cur_chain = out_chain;
    for (int i = 0; cur_chain; cur_chain = cur_chain->next) {
        lt_buffer_t *cur_buf = cur_chain->buf;

        while (1) {
            out_vector[i].iov_base = buf;
            out_vector[i].iov_len = buf->last - buf->pos;

            if (buf->next == buf) break; 
            else { i++;  buf = buf->next; }
        }
    }

    ssize_t n = writev(fd, out_vector, iov_i);
    if (n == -1) {
        int errsv = errno;
        switch(errsv) {
            case EAGAIN:break;
            default:return LERROR;
        }
    } else if (n > 0) {
        int remain = n;
        lt_buffer_t *cur_buf;
        int iov_len;

        for (int i = 0; i < iov_i; i++) {
            iov_len = out_vector[i].iov_len
            if (remain > iov_len) {
                cur_buf = out_vector[i].iov_base;
                cur_buf->pos += iov_len;
                remain -= iov_len;
                cur_buf = cur_buf->next;
            } else if (remain < iov_len){
                cur_buf->pos += remain;
                //post writev
                break;
            } else {
                //complete
            }
        }
    }


    return 0;
}
*/
/*
ssize_t send_buffer_chains_loop(int fd, lt_buffer_t *out_buf)
{
    size_t length = 0;

    lt_buffer_t *header_buf = out_buf; 
    lt_buffer_t *buf = out_buf; 

    int n = 1;

    for (; buf->next != buf; n++, buf = buf->next) { }

    struct iovec out_vector[n];

    for (int i = 0; i < n; i++, out_buf = out_buf->next) {
        out_vector[i].iov_base = out_buf->pos;
        out_vector[i].iov_len = out_buf->last - out_buf->pos;
        length += out_vector[i].iov_len;
    }

    ssize_t rv = writev(fd, out_vector, n);
    if (0 < rv && rv < length) {
        ssize_t written_count = rv / DEFAULT_BUF_SIZE;
        ssize_t written_offset = rv % DEFAULT_BUF_SIZE;
        ssize_t remain = length - rv;

        lt_buffer_t *written_buf = header_buf;
        for (int i = 0; i < written_count; i++, written_buf = written_buf->next) {
            written_buf->pos += DEFAULT_BUF_SIZE;
        }
        written_buf->pos += written_offset;
//        pospone_send_buffer_chains_loop(fd, out_buf);
        return LAGAIN;
        //remain;
        //Double Choice
        //A, keep it loop in event_list to send
        //B, Watch writable event and send
    } else if (rv == -1) {
        int errsv = errno;
        switch (errsv) {
            case EAGAIN: return LAGAIN;
            case EINTR:  return LABORT;
            default: break;//TODO
        }

    } else if (!rv) {
        fprintf(stderr, "writev returned zero\n");
        return LCLOSE;
    } else {
        return LOK;
    }

    return n;
}
*/

ssize_t
lt_recv(int fd, lt_buffer_t *lt_buf)
{
    for (;;) {
        ssize_t length = lt_buf->end - lt_buf->last;
        ssize_t recv_len = 0;
        ssize_t n = recv(fd, lt_buf->last, length, 0/*TODO: recv TWICE*/);

        if (n == 0) {
            return LCLOSE;
        } else if (n > 0) {
            lt_buf->last += n;
            recv_len += n;
            if (n < length) {
                continue;
            }
            if (n == length) { 
                lt_buffer_t *new_buf = lt_alloc(new_buf->pool, new_buf->pool->manager);
                lt_buf->next = new_buf;
                lt_buf = new_buf;
                continue;
            }
        } else if (n == -1) {
            int errsv = errno;
            if (errsv == EAGAIN) {
                if (recv_len > 0) {
                    return recv_len;
                } else if (recv_len == 0) {
                    return LAGAIN;
                }
            } else {
                perror("recv:");
                return LERROR;
            }
        } else { }
    }

    return 0;
}

int 
lt_accept(int fd, struct sockaddr *peer)
{
    socklen_t socklen = sizeof(struct sockaddr);
    int conn_fd = accept(fd, peer, &socklen);
    if (conn_fd == -1) {
        int err = errno;
        perror("accept4");
        switch(err) { 
            case EAGAIN:
                return LAGAIN;//complete?
            case ECONNABORTED:
                return LABORT;//continue
            case EMFILE:
            case ENFILE:
                perror("file descriptor Crashed"); 
                return LERROR;//FILE D
            default:
                fprintf(stderr, "unknown accept4 error\n"); 
                return LERROR;
        }
    } 

    return conn_fd;
}

lt_chain_t *construct_chains(request_t *req)
{
    lt_new_memory_pool_manager(&req->chain_pool_manager);
    req->chain_pool = lt_new_memory_pool(sizeof(lt_chain_t), &req->chain_pool_manager, NULL);

    lt_chain_t *chain_request_line = lt_alloc(req->chain_pool, &req->chain_pool_manager);
    chain_request_line->buf.iov_base = req->request_start;
    chain_request_line->buf.iov_len = req->request_length;

    lt_http_header_element_t *element = req->element_head;
    lt_chain_t *chain_request_header_field = lt_alloc(req->chain_pool, &req->chain_pool_manager);
    chain_request_line->next = chain_request_header_field;

    lt_chain_t *new_chain;
    lt_chain_t *old_chain = chain_request_line;

    for (;;) {
        //insert(old_chain, &chain_request_header_field, element);
        //old_chain->next = insertion_chain; //old后插
        //element->value/key //修改element
        //insertion_chain->next = chain_request_header_field; //chain前插
        //
        //
        chain_request_header_field->buf.iov_base = element->value.data;
        chain_request_header_field->buf.iov_len = element->value.length;
        chain_request_header_field->next = NULL;

        if (element == req->element_tail) {
            break;
        }

        element = element->next;
        new_chain = lt_alloc(req->chain_pool, &req->chain_pool_manager);
        old_chain = chain_request_header_field;
        chain_request_header_field->next = new_chain;
        chain_request_header_field = new_chain;
    }


    return chain_request_line;
}
