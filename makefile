CC = clang
CFLAGS = -g -Wall -std=gnu11
INCLUDES = -I./
SRCS = buffer_op_lea.c http.c lt_pool.c http_parse.c main.c event_lea.c lea_heap.c proxy.c ngx_http_parse.c
OBJS = $(SRCS:.c=.o)
MAIN = leactor_mktg

.PHONY: depend clean

all: $(MAIN)
	@echo compile success
	
$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS)
	
.c.o:
	$(CC) $(CFLAGS) $(INCLUEDS) -c $< -o $@
	
clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^
	
