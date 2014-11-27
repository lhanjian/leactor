#include "http.h"
#include "event_lea.h"

#define LCLOSE (-5)
#define LAPART (-4)
#define LAGAIN (-3)
#define LERROR (-2)


ssize_t pospone_send_buffer_chains_loop(int fd, lt_buffer_t *out_buf);
int send_buffers(base_t *, int fd, lt_buffer_t *out_buf);

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
        //new_buf->next = buf;//circular_list
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
    buf->next = NULL;
    buf->head = 0;
    buf->written = 0;

    return buf;
}

int resend_chains(event_t *ev, void *arg)
{
    lt_chain_t *out = (lt_chain_t *)arg;
    int rv = send_chains(ev->base, ev->fd, out);
    return 0;
}

int resend_buffers(event_t *ev, void *arg)
{
    lt_buffer_t *buf = (lt_buffer_t *)buf;
    send_buffers(ev->base, ev->fd, buf);
    return 0;
}

int send_chains(base_t *base, int fd, lt_chain_t *out_chain)
{
    int chain_len = out_chain->chain_len;
    //lt_chain_t *rv_chain;
/*
    for (lt_chain_t *cur = out_chain; cur; cur = cur->next) { 
        chain_len++;
    } */
    struct iovec out_vec[chain_len];
    lt_chain_t *cur_chain = out_chain;
    for (int i = 0; i < chain_len; i++) {
        out_vec[i] = cur_chain->buf;
        cur_chain = cur_chain->next;
    }

    ssize_t rv = writev(fd, out_vec, chain_len);
    if (rv == -1) {
        int errsv = errno;
        switch (errsv) {
            case EAGAIN://POST_SEND:TODO
                lt_new_post_callback(base, resend_chains, fd, out_chain);//TOMORROW TODAY
                return LOK;//TODO
            case EPIPE:
                return LCLOSE;
            default:
                perror("writev:");
                return LERROR;
                break;
        }
    }

    if (rv == 0) {
        return LCLOSE;
    }
    
    //rv > 0
    for (lt_chain_t *cur = out_chain; 
                     cur; //LOK
                     cur = cur->next) {
        int iov_len = cur->buf.iov_len;
        if (rv > iov_len) {
            cur->buf.iov_len -= iov_len;
            rv -= iov_len;
            chain_len--;
            continue;
            //lt_free?not lt_Free until chains all has been seed out.
        } else {
            cur->buf.iov_base  = (char *)cur->buf.iov_base + iov_len;
            cur->buf.iov_len -= rv;
            //rv_chain = cur;
            if (rv == iov_len) {
                rv -= iov_len;
                chain_len--;
                continue;
            } else /*if(rv < iov_len)*/ {
                cur_chain->chain_len = chain_len;
                lt_new_post_callback(base, resend_chains, fd, cur_chain);
                return LAGAIN;
            }
        }
    }

    return LOK;//all complete;
}
/*
int http_is_chunked_tail(request_t *rep, lt_buffer_t *buf)
{
    lt_buffer_t *b = buf;
    lt_buffer_t *prev;
    for (prev = b; b->next; b = b->next, prev = b) {
    }

    char *CRLF_zero_CRLF = "0\r\n\r\n";

    char *p = b->last;
    int i = 4;
    while (i < 0) {
        if (*p != CRLF_zero_CRLF[i]) {
            return 0;
        }
        if (p == b->start) {
            p = prev->last;
        }
        p--;
        i--;
    }

    return 1;
}
*/

int send_buffers(base_t *base, int fd, lt_buffer_t *out_buf)
{
    if(!out_buf) {
        return 0;
    }

    if (out_buf->pos == out_buf->last) {
        return 0;
    }
    
    lt_buffer_t *buf = out_buf;


    int iov_i;
    for (iov_i = 0; out_buf; out_buf = out_buf->next) {
        iov_i++;
    }

    struct iovec out_vector[iov_i];

    for (int i = 0; i < iov_i; i++, buf = buf->next) {
        out_vector[i].iov_base = buf->pos;
        out_vector[i].iov_len = buf->last - buf->pos;
    }

    ssize_t n = writev(fd, out_vector, iov_i);
    if (n == -1) {
        int errsv = errno;
        switch(errsv) {
            case EAGAIN:
                lt_new_post_callback(base, resend_buffers, fd, out_buf);
                break;
            case EPIPE: 
                return LCLOSE;
            default:return LERROR;
        }
    } else if (n > 0) {
        int remain = n;
        lt_buffer_t *cur_buf;
        int iov_len;

        for (int i = 0; i < iov_i; i++) {
            iov_len = out_vector[i].iov_len;
            if (remain > iov_len) {
                cur_buf = out_vector[i].iov_base;
                cur_buf->pos += iov_len;
                remain -= iov_len;
            } else if (remain < iov_len){
                cur_buf->pos += remain;
                lt_new_post_callback(base, resend_buffers, fd, cur_buf);
                //post writev
                return LAGAIN;
            } else {
                return LOK;
                //complete
            }
        }
    } else if (n == 0) {
        return LCLOSE;
    }



    __builtin_unreachable();
}

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
    ssize_t recv_len = 0;
    for (;;) {
        ssize_t length = lt_buf->end - lt_buf->last;
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
                lt_buffer_t *new_buf = lt_new_buffer(lt_buf->pool, lt_buf->pool->manager);
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
#define _GNU_SOURCE
    int conn_fd = accept4(fd, peer, &socklen, SOCK_NONBLOCK);
    if (conn_fd == -1) {
        int err = errno;
        switch(err) { 
            case EAGAIN:
                return LAGAIN;//complete?
            case ECONNABORTED:
                perror("accept4:ECONNABORTED");
                return LABORT;//continue
            case EMFILE:
            case ENFILE:
                perror("accept4:file descriptor Crashed"); 
            default:
                fprintf(stderr, "unknown accept4 error\n"); 
                return LERROR;
        }
    } 

    return conn_fd;
}

int lt_set_reuseaddr(int sockfd, int yes)
{
    int rv = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    if (rv == -1) {
        perror("setsockopt_REUSE_ADDR");
        return -1;
    }
    return 0;
}

int lt_set_reuseport(int sockfd, int yes)
{
    int rv = setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof yes);
    if (rv == -1) {
        perror("setsockopt_REUSE_PORT");
        return -1;
    }
    return 0;
}

int lt_set_keepalive(int sockfd, int yes)
{
    int rv = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof yes);
    if (rv == -1) {
        perror("setsockopt_KEEPALIVE");
        return -1;
    }
    return 0;
}

int lt_set_nodelay(int sockfd, int yes)
{
    int rv = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof yes);
    if (rv == -1) {
        perror("setsockopt_TCP_NODELAY");
        return -1;
    }
    return 0;
}
