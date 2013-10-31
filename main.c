#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>


void dump_memory_map(void);

int main(int argc, char **argv) {
	//dump_memory_map();
    void *m1 = malloc(550);  // should allocate 64 bytes
    dump_memory_map();
    //printf("malloced first 50 (64) bytes\n");
    void *m2 = malloc(100); // should allocate 128 bytes
    dump_memory_map();
    freeMine(m1);
    void *m3 = malloc(56);  // should allocate 64 bytes
    void *m4 = malloc(11);  // should allocate 32 bytes
    dump_memory_map();
    freeMine(m3);
    void *m5 = malloc(30);  // should allocate 64 bytes
    void *m6 = malloc(120); // should allocate 128 bytes
    freeMine(m2);
    return 0;
}
