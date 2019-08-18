/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Guangya Zhu
    UPI: gzhu282

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */


#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <pthread.h>


#define SIZE    2

struct block {
    int size;
    int *first;
};

int*shared_data;




void print_block_data(struct block *blk) {
    printf("size: %d address: %p\n", blk->size, blk->first);
}

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
	int combined[left->size + right->size];
	int dest = 0, l = 0, r = 0;
	while (l < left->size && r < right->size) {
		if (left->first[l] < right->first[r])
			combined[dest++] = left->first[l++];
		else
			combined[dest++] = right->first[r++];
	}
	while (l < left->size)
		combined[dest++] = left->first[l++];
	while (r < right->size)
		combined[dest++] = right->first[r++];
    memmove(left->first, combined, (left->size + right->size) * sizeof(int));
}
/* Merge sort the data. */
void merge_sort(struct block *my_data) {
    // print_block_data(my_data);
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;
        merge_sort(&left_block);
        merge_sort(&right_block);
        merge(&left_block, &right_block);
    }
}


/* Merge sort the data. */
void* merge_sort_process(void* my_data) {
    // print_block_data(my_data);
    //printf("Finishing first created.\n");
    struct block *data = my_data;
    if (data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = data->size / 2;
        left_block.first = data->first;
        right_block.size = left_block.size + (data->size % 2);
        right_block.first = data->first + left_block.size;
        merge_sort(&left_block);
        merge_sort(&right_block);
        merge(&left_block, &right_block);
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {
    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
            // printf("i = %d\n",data[i]);
    }
    return sorted;
}
void merge_sort_Two_processes(struct block *my_data){
    
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;

        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;
       
        // shared_right = (struct block *)mmap(NULL, sizeof(shared_right),PROT_READ| PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,sysconf(_SC_PAGE_SIZE));
        
        pid_t cpid = fork();
            if (cpid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }else if (cpid == 0) { //child
                merge_sort(&right_block);
                exit(0);
            }else {
                merge_sort(&left_block);
                wait(NULL); //wait for child
                printf("child finished\n");
                // for (int i = 0; i<left_block.size;i++){
                //     printf("the right block sent to parent num: %d\n",left_block.first[i]);
                // }
                // for (int i = 0; i<right_block.size;i++){
                //     printf("the left block sent to parent num: %d\n",right_block.first[i]);
                // }
                merge(&left_block,&right_block);
                printf("merged"); 
           }
     
    }
}







int main(int argc, char *argv[]) {
	long size;
	struct rlimit rl;
    

	 /* Obtain the current limits. */
	 getrlimit (RLIMIT_STACK, &rl);
	 /* Set the stack limit */
	 rl.rlim_cur = 2000000000;
	 setrlimit (RLIMIT_STACK, &rl);
    

	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}
    struct block start_block;
    shared_data = mmap(NULL, size*sizeof(int),PROT_READ| PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    int data[size];
    start_block.size = size;
    start_block.first = shared_data;
    for (int i = 0; i < size; i++) {
        shared_data[i] = rand();
    }
    printf("starting---\n");
    merge_sort_Two_processes(&start_block);
    printf("---ending\n");
    printf(is_sorted(shared_data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}
