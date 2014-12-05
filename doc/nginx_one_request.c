sudo strace -p 8900  
Process 8900 attached
epoll_wait(7, {{EPOLLIN, {u32=763293712, u64=140373179428880}}}, 512, 
-1) = 1
accept4(4, {sa_family=AF_INET, sin_port=htons(50896), 
sin_addr=inet_addr("127.0.0.1")}, [16], SOCK_NONBLOCK) = 5
epoll_ctl(7, EPOLL_CTL_ADD, 5, {EPOLLIN|EPOLLRDHUP|EPOLLET, 
{u32=763294144, u64=140373179429312}}) = 0
epoll_wait(7, {{EPOLLIN, {u32=763294144, u64=140373179429312}}}, 512, 
60000) = 1
recvfrom(5, "GET / HTTP/1.1\r\nUser-Agent: curl"..., 1024, 0, NULL, 
NULL) = 73
stat("/usr/share/nginx/html/index.html", {st_mode=S_IFREG|0644, 
st_size=612, ...}) = 0
open("/usr/share/nginx/html/index.html", O_RDONLY|O_NONBLOCK) = 9
fstat(9, {st_mode=S_IFREG|0644, st_size=612, ...}) = 0
writev(5, [{"HTTP/1.1 200 OK\r\nServer: nginx/1"..., 237}], 1) = 237
sendfile(5, 9, [0], 612)                = 612
write(3, "127.0.0.1 - - [05/Dec/2014:13:53"..., 86) = 86
close(9)                                = 0
setsockopt(5, SOL_TCP, TCP_NODELAY, [1], 4) = 0
recvfrom(5, 0x7fab2ebff7b0, 1024, 0, 0, 0) = -1 EAGAIN (Resource 
temporarily unavailable)
epoll_wait(7, {{EPOLLIN|EPOLLRDHUP, {u32=763294144, 
u64=140373179429312}}}, 512, 65000) = 1
recvfrom(5, "", 1024, 0, NULL, NULL)    = 0
close(5)                                = 0
epoll_wait(7, 

