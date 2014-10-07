gcc -std=gnu99 -c event_pool.c -Wall -g
gcc -std=gnu99 -c pool_ex.c -Wall -g
gcc event_pool.o pool_ex.o -o pool_bin -std=gnu99 -Wall -g
