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
int *heap_begin = NULL; 
int *first_free = NULL; 

void next(int** curr){
	if(*curr + *curr[0]/sizeof(int) < heap_begin+HEAPSIZE)
		*curr = *curr + *curr[0]/sizeof(int);
}


void next_free(int** curr){
	if(*curr[1]!=0)
		*curr = *curr + *curr[1]/sizeof(int);
}


int inBounds(int** curr){
	return *curr<heap_begin+HEAPSIZE/sizeof(int);
}

void *malloc(size_t request_size) {
//	printf("\nHello?");
	int power = MINIMUM_ALLOC;
    while(power < request_size+2*sizeof(int)){  //should give us the size of memory needed
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
    for(;inBounds(&curr);next(&curr)){ //traverse the entire freelist
    	if(curr[1] == 0) continue; //if we're in an allocated block, skip it
    	if(curr[0] < best[0] && curr[0] >= power){
    		best = curr;
    	}
   }

	if(best==first_free){
	        if(best[0]==power)
	 		first_free = best+best[1]/sizeof(int);
	}
	
	
	//int* free_before = NULL;
	
   while(best[0] > power){
   	int next = best[1];
   	best[0] = best[0]/2;
  	int size = best[0];
   	best[1] = size;
   	//free_before = best;
   	best = best + size/sizeof(int);
   	best[0] = size;
	best[1] = next - best[0];
   }
   
   best[1] = 0;
   
   int* last_free = NULL;
   for(curr = first_free;inBounds(&curr);next(&curr)){
   	if(curr[1] != 0) last_free = curr; //if the block isn't allocated, move last_free up
   }
   
   if(last_free!=NULL){
   	last_free[1] = -1; //if the whole block isn't allocated, set an "end of free-list" flag
   }
         
   void* toReturn = best;
   toReturn += 2;
   printf("returned pointer %p %p\n", toReturn);
   return toReturn;
}

void freeMine(void *memory_block) {
	if(heap_begin == NULL){ return;}
	int* memory = memory_block - 2/sizeof(int);
	if(memory < first_free){
		memory[1] = first_free-memory;
		first_free = memory;
	}else{
		int* curr = first_free;
		do{
			next(&curr);
		}while(curr[1]==0);
		int* before = first_free;
		while(curr<memory){
			before = curr;
			do{
				next(&curr);
			}while(curr[1]==0);
		}
		memory[1] = before + before[1]/sizeof(int) - memory;
		before[1] = memory-before;
	}
	//merge_buddies(heap_begin);//not implemented; function to merge buddies.  Recursion?
}

void dump_memory_map(void) {

	printf("\n--------------------Memory Map--------------------\n");
	int* curr = heap_begin;
	int offset = 0;
	char* alloc;
	for(;inBounds(&curr);next(&curr)){
		if(curr[1] == 0){
			alloc = "Allocated";
		}
		else alloc = "Free";
		printf("Block size: %d, offset %d, %s, %p\n", curr[0], offset, alloc, curr);
		offset = offset + curr[0];
		//curr = curr + curr[0]/sizeof(int);
	}
	printf("--------------------------------------------------\n");
}


