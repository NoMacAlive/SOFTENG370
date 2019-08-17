/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: ZHANG MingZhe
    UPI: bzha841

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <pthread.h> 
#include <sys/wait.h> 

#define SIZE    2

struct block {
    int size;
    int *first;
};

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

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {
    bool sorted = true;
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Merge sort the data. */
void step_6_merge_sort(void *data) {
    struct block *my_data = data;
    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
    pid_t fpid;
    int fd1[2];
    pipe(fd1);  

    // print_block_data(my_data);
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;

        fpid=fork();

        if (fpid<0){
            printf("Fork error!");
        } else if (fpid == 0){     
            //child
            close(fd1[0]);
            merge_sort(&left_block);
            int sorted_value[left_block.size];
            printf("child:");
            printf(is_sorted(left_block.first, left_block.size) ? "sorted\n" : "not sorted\n");
            for (int i = 0; i<left_block.size;i++){
                sorted_value[i]=left_block.first[i];
            }
            write(fd1[1],sorted_value,sizeof(sorted_value));
            exit(0);
        } else {
            //parent
            close(fd1[1]); 
            merge_sort(&right_block);
            int sorted_value[left_block.size];
            printf("father:");
            printf(is_sorted(right_block.first, right_block.size) ? "sorted\n" : "not sorted\n");
            read(fd1[0],sorted_value,sizeof(sorted_value));
            for (int i = 0; i<left_block.size;i++){
                left_block.first[i] = sorted_value[i];
            }
        }   
        merge(&left_block, &right_block);
    }
}

int main(int argc, char *argv[]) {
	long size;

    /*Get the previous stack size then reset it to 2 Gb*/
    struct rlimit limit;
    const rlim_t kStackSize = 2048L * 1024L * 1024L;  // min stack size = 1Gb
    getrlimit (RLIMIT_STACK, &limit);
    printf ("Previous Stack Limit = %ldMb \n", limit.rlim_cur / (1024 * 1024));
    limit.rlim_cur = kStackSize;
    int result = setrlimit(RLIMIT_STACK, &limit);
    printf ("Current stack Limit = %ldMb \n", limit.rlim_cur / (1024 * 1024));
    

	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}

    struct block start_block;
    int data[size];
    start_block.size = size;
    start_block.first = data; 
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }

    printf("starting---\n");
    step_6_merge_sort(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}