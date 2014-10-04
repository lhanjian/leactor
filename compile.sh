gcc event_lea.c -c event_lea.c -Wall -g -m32 -std=gnu99
gcc lea_heap.c -c event_lea.c -Wall -g -m32 -std=gnu99
gcc example.c -c event_lea.c -Wall -g -m32 -std=gnu99
gcc event_lea.o lea_heap.o example.o -o test.bin -Wall -g -m32 -std=gnu99
