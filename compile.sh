gcc event_pool.c -c -Wall -g -m32 -std=gnu99
gcc event_lea.c -c -Wall -g -m32 -std=gnu99
gcc lea_heap.c -c -Wall -g -m32 -std=gnu99
gcc example.c -c -Wall -g -m32 -std=gnu99
gcc event_lea.o lea_heap.o example.o event_pool.o -o test.bin -Wall -g -m32 -std=gnu99
