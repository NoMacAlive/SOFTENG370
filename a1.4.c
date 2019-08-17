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
int numThreadsAvaliable = 0;
int numOfCoreConfiged = 8;
int numOfThreadsRunning = 0;
pthread_t threads[20];
pthread_attr_t attr;
pthread_mutex_t mut;
pthread_mutex_t mut1;

struct block {
    int size;
    int *first;
};

        //how to create a thread
        //pthread_t thread_id;
        //pthread_attr_t thread_attr;
        //pthread_attr_init(&thread_attr);
        //pthread_attr_setstacksize(&thread_attr, 5000000000);
        //s=pthread_create(&thread_id, &thread_attr, merge_sort_thread,(void *)&right_block);


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
void* merge_sort_threaded(void *my_data) {
    struct block *data = my_data;
    // printf("Number of thread running is: %d",numOfThreadsRunning);
    bool s1,s2 = false;
    // print_block_data(data)
    if (data->size > 1) {
        struct block left_block;
        struct block right_block;
        left_block.size = data->size / 2;
        left_block.first = data->first;
        right_block.size = left_block.size + (data->size % 2);
        right_block.first = data->first + left_block.size;

        pthread_t thread1, thread2;
        
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 256L*1024L*1024L);

        if (numOfThreadsRunning < numOfCoreConfiged){
            pthread_mutex_lock(&mut);
            numOfThreadsRunning+=1;
            // printf("New thread added.\n");
            pthread_mutex_unlock(&mut);
            s1 = true;
            pthread_create(&thread1, &attr, merge_sort_threaded, &left_block);
        if (s1){
            pthread_join(thread1, NULL);
            pthread_mutex_lock(&mut);
            numOfThreadsRunning-=1;
            pthread_mutex_unlock(&mut);
        }
        }else{
            merge_sort(&left_block);
        }

        if (numOfThreadsRunning < numOfCoreConfiged){
            pthread_mutex_lock(&mut);
            numOfThreadsRunning+=1;
            // printf("New thread added.\n");
            pthread_mutex_unlock(&mut);
            s2 = true;
            pthread_create(&thread2, &attr, merge_sort_threaded, &right_block);
            if(s2){
            pthread_join(thread2, NULL);
            pthread_mutex_lock(&mut);
            numOfThreadsRunning-=1;
            pthread_mutex_unlock(&mut);
        }
        }else{
            merge_sort(&right_block);
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
            // printf("i = %d, i+1 = %d\n",data[i],data[i+1]);
    }
    return sorted;
}


int main(int argc, char *argv[]) {
	long size;
    struct rlimit rl;
    int name = _SC_NPROCESSORS_CONF;
    pthread_mutex_init(&mut, NULL);
	 /* Obtain the current limits. */
	 getrlimit (RLIMIT_STACK, &rl);
	 /* Set the stack limit */
	 rl.rlim_cur = 900000000;
     printf("Current stack size is: %ld\n",rl.rlim_cur);
	 setrlimit (RLIMIT_STACK, &rl);

     //get all cores avaliable
     //and initialise global thread state variable
     long numOfCores = sysconf(name);
     numOfCoreConfiged = (int)numOfCores;
     numThreadsAvaliable = numOfCoreConfiged;
     printf("The number of cores configed is : %d\n",numOfCoreConfiged);
    
    //initialise the mutex lock using default value.
    
    // pthread_mutex_init(&mut1, NULL);
    

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
        // printf("data list were %d\n",data[i]);
    }
    printf("starting---\n");
    merge_sort_threaded(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}
