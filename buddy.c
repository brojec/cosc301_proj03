#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// function declarations
void *malloc(size_t);
void freeMine(void *);
void dump_memory_map(void);


const int HEAPSIZE = (1*1024*1024); // 1 MB
const int MINIMUM_ALLOC = sizeof(int) * 2;

// global file-scope variables for keeping track
// of the beginning of the heap.
void *heap_begin = NULL; 
int *first_free = NULL; //I REALLY don't think this should be an int*.  Either an int or a void*.


void *malloc(size_t request_size) {
//	printf("\nHello?");
	int power = 8;
    while(power < request_size+8){  //should give us the size of memory needed
    power = power*2;
    }
    printf("request size: %d \npower: %d\n",request_size,power);
    //printf("\n");
    // if heap_begin is NULL, then this must be the first
    // time that malloc has been called.  ask for a new
    // heap segment from the OS using mmap and initialize
    // the heap begin pointer.
    
    if (!heap_begin) {
        heap_begin = mmap(NULL, HEAPSIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        atexit(dump_memory_map);
        first_free = heap_begin;
        first_free[0] = HEAPSIZE;
        first_free[1] = -1;
    }
    int* curr = first_free;
    int* best = first_free;
    while(curr[1] > 0){ //should stop once we get to a big enough piece of memory
    	if(curr[0] < best[0] && curr[0] >= power){
    		best = curr;
    	}
    	curr = curr + curr[1];
   }
   while(best[0] > power){
   	int next = best[1];
   	best[0] = best[0]/2;
   	int size = best[0];
   	best[1] = best[0];
   	best = best + best[0]/sizeof(int);
   	best[0] = size;
   	best[1] = next - best[0];
   }
   best[1] = 0;
   void* toReturn = best[2];
   return toReturn;
}

void freeMine(void *memory_block) {
	int* curr = heap_begin;
	if(heap_begin == NULL){ return;}
	int* memory = memory_block;
	while(curr + curr[1] < memory){
		curr = curr + curr[1];
	}
	memory[1] = curr[1] - (memory-curr);
}

void dump_memory_map(void) {

	printf("Got into memory map\n");
	int* curr = heap_begin;
	int offset = 0;
	char* alloc;
	while(offset < HEAPSIZE){
		if(curr[1] == 0){
			alloc = "Allocated";
		}
		else alloc = "Free";
		printf("Block size: %d, offset %d, %s\n", curr[0], offset, alloc);
		offset = offset + curr[0];
		curr = curr + curr[0];
	}
}


