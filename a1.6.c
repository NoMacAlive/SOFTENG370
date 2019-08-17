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
#include <pthread.h>


#define SIZE    2

struct block {
    int size;
    int *first;
};





// void print_block_data(struct block *blk) {
//     printf("size: %d address: %p\n", blk->size, blk->first);
// }

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
        int s1,s2;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;
        
        int pipefd[2];
        pid_t cpid;
        
        pipe(pipefd);
        cpid = fork();
           if (cpid == -1) {
               perror("fork");
               exit(EXIT_FAILURE);
           }
           if (cpid == 0) { 
               int buf[right_block.size];
               merge_sort_process(&right_block);
               for (int i = 0; i<right_block.size;i++){
                    buf[i]=right_block.first[i];
                    // printf("the right block num: %d\n",buf[i]);

                }
            //    /* Child writes merge sorted half to pipe */
               write(pipefd[1], &buf, sizeof(buf));
               printf("child wrote right block\n");
               exit(EXIT_SUCCESS);
            //    printf("right block is: %d",right_block.size);
            }else {
               
                
                  /* Parent reads from pipe */
           int buf[right_block.size];
               close(pipefd[1]);  
               merge_sort_process(&left_block);/* Close unused write end */
                // for (int i = 0; i<left_block.size;i++){
                //     printf("the left block num: %d\n",left_block.first[i]);
                // }
               while (read(pipefd[0], &buf, sizeof(buf)) > 0){
                   printf("parent process waiting...\n");
               }
               printf("parent read right block\n");
               for (int i = 0; i<right_block.size;i++){
                    right_block.first[i] = buf[i];
                }
            //    for (int i = 0; i<right_block.size;i++){
            //         printf("the right block sent to parent num: %d\n",buf[i]);
            //     }
            //    write(STDOUT_FILENO, "\n", 1);
            //    close(pipefd[0]);
                merge(&left_block, &right_block);

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
    int data[size];
    start_block.size = size;
    start_block.first = data;
    for (int i = 0; i < size; i++) {
        data[i] = rand();
    }
    printf("starting---\n");
    merge_sort_Two_processes(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}
