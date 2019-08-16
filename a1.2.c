/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Guangya Zhu
    UPI: gzhu282

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

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
void* merge_sort_thread(void* my_data) {
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


void merge_sort_Two_threaded(struct block *my_data){
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        int s1,s2;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;

        //first thread
        pthread_t thread1_id;
        pthread_attr_t thread1_attr;
        pthread_attr_init(&thread1_attr);
        pthread_attr_setstacksize(&thread1_attr, 256*1024*1024);
        s1=pthread_create(&thread1_id, &thread1_attr, merge_sort_thread,(void *)&left_block);
        //printf("thread 1 is %d",s);
        //second thread
        pthread_t thread2_id;
        pthread_attr_t thread2_attr;
        pthread_attr_init(&thread2_attr);
        pthread_attr_setstacksize(&thread2_attr, 256*1024*1024);
        s2=pthread_create(&thread2_id, &thread2_attr, merge_sort_thread,(void *)&right_block);
        //printf("thread 2 is %d",s);

        if(s1==0){
        pthread_join(thread1_id, NULL);
        printf("Finishing first thread.\n");
        }
        if(s2==0){
        pthread_join(thread2_id, NULL);
        printf("Finishing second thread.\n");
        }
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

int main(int argc, char *argv[]) {
	long size;
	struct rlimit rl;
    

	 /* Obtain the current limits. */
	 getrlimit (RLIMIT_STACK, &rl);
	 /* Set the stack limit */
	 rl.rlim_cur = 1024*1024*1024;
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
    merge_sort_Two_threaded(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}
