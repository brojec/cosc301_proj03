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
void merge_buddies(int*, int*);
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
	
	int* temp = best;
	if(best[1] == -1){
		temp = NULL;
	}
   	else do{
   		next(&temp);
   	}while(temp[1] == 0);
   	
   	if(best[0] == power){
	   	int* tester = first_free;
		do{
			next(&tester);
		}while(tester[1]==0);
		int* before = first_free;
		while(tester<best){
			before = tester;
			do{
				next(&tester);
			}while(tester[1]==0);
		}
		before[1] = before[1] + best[1];
		printf("empty\n");
	}
   	
   while(best[0] > power){ //cut it in half, move to the second half
   	best[0] = best[0]/2;
  	int size = best[0];
   	best[1] = size;
   	best = best + size/sizeof(int);
   	best[0] = size;
   	if(temp == NULL){
   		best[1] = -1;
   	}
   	else{
	best[1] = temp - best;
	}
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
   return toReturn;
}

void freeMine(void *memory_block) {
	printf("freeing %p\n", memory_block-2);
	if(heap_begin == NULL){ return;}

	int* memory = memory_block - 2;

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
		memory[1] = (before- memory)*sizeof(int) + before[1] ;
		before[1] = (memory-before)*sizeof(int);
		printf("empty\n");
	}
	int* be4 = first_free;//#yoloswag
	while(be4 + be4[0]/sizeof(int) < memory){
		next(&be4);
	}
	//printf("%p\n",be4);
	//printf("%p\n", memory);
	merge_buddies(memory,be4);
}

void merge_buddies(int* memory, int* before){
	dump_memory_map();
	int* after = memory;
	next(&after);
	if(after[1] == 0 || after == memory || after[0] != memory[0]){
		after = NULL;
	}
	if(before[1] == 0 || before == memory || before[0] != memory[0]){
		before = NULL;
	}
	int* curr = first_free;
	int jump_size = HEAPSIZE/2;
	while(curr != memory){
		if(curr > memory){
			if(curr == after){
				after = NULL;
			}
			curr = curr-jump_size/sizeof(int);
		}
		if(curr < memory){
			curr = curr + jump_size/sizeof(int);
		}
		if(curr != memory){
			jump_size = jump_size/2;
		}
		
	}
	int did_merge = 0;
	if(before != NULL){
		before[0] = before[0]*2;
		before[1] = memory[1] + memory[0];
		if(memory[1] == -1){
			before[1] = -1;
		}
		did_merge = 1;
		memory = before;
	}
	else if(after != NULL){
		memory[0] = memory[0]*2;
		memory[1] = after[1] + after[0];
		did_merge = 1;
		if(after[1] == -1){
			memory[1] = -1;
		}
	}
	int* be4 = first_free;
	int* test = first_free;
	while(test<memory){
		next(&test);
		if(test < memory){
			next(&be4);
		}
	}
	if(memory[0] == HEAPSIZE){
		return;
	}
	else if(did_merge){
		//dump_memory_map();
		//printf("just merged stuff\n");
		merge_buddies(memory,be4);
	}
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


