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

int numOfCoreConfiged;
static int* numOfRunningProcess;
pthread_mutex_t * pmutex = NULL;
pthread_mutexattr_t attrmutex;//mutex lock inter-processes


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
void merge_sort_multiprocess(struct block *my_data){
    int numOfRunningProcessFromChild = 0;
    if (my_data->size > 1) {
        struct block left_block;
        struct block right_block;
        int s1,s2;
        left_block.size = my_data->size / 2;
        left_block.first = my_data->first;
        right_block.size = left_block.size + (my_data->size % 2);
        right_block.first = my_data->first + left_block.size;
        
        int pipefd[2];
        pid_t l_id;
        pid_t r_id;
        int pipeNum[2];
        pipe(pipefd);
        pipe(pipeNum);
        // printf("main numberOfRunningProcess is : %d\n",*numOfRunningProcess);
            r_id = fork();
            // printf("fork run\n");
            pthread_mutex_lock(pmutex);
            *numOfRunningProcess += 1 ;
            pthread_mutex_unlock(pmutex);
        
            if (r_id == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } 
            if (r_id == 0) {//child process

            if(*numOfRunningProcess < numOfCoreConfiged){
                merge_sort_multiprocess(&right_block);
            } else {
                merge_sort(&right_block);
            }

            write(pipeNum[1],right_block.first, right_block.size*sizeof(int));
            exit(EXIT_SUCCESS);
            //    printf("right block is: %d",right_block.size);
            }
            
            l_id = fork();
            // printf("fork run\n");
            pthread_mutex_lock(pmutex);
            *numOfRunningProcess += 1 ;
            pthread_mutex_unlock(pmutex);
        
            if (l_id == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } 
            if (l_id == 0) {//child process

                if(*numOfRunningProcess < numOfCoreConfiged){
                    merge_sort_multiprocess(&left_block);
                } else {
                    merge_sort(&left_block);
                }
            write(pipefd[1], left_block.first, left_block.size*sizeof(int));
            exit(EXIT_SUCCESS);
            //    printf("right block is: %d",right_block.size);
            }
            close(pipefd[1]);
	        close(pipeNum[1]);

            read(pipefd[0], left_block.first,left_block.size*sizeof(int));
	        read(pipeNum[0], right_block.first,right_block.size*sizeof(int));
            pthread_mutex_lock(pmutex);
            *numOfRunningProcess -= 2;
            pthread_mutex_unlock(pmutex);
            merge(&left_block, &right_block);
    }
}








int main(int argc, char *argv[]) {
    printf("stack size was: \n");
	long size;
	struct rlimit rl;
    int name = _SC_NPROCESSORS_CONF;
    long numOfCores = sysconf(name);
    numOfCoreConfiged = (int)numOfCores;
    printf("run here\n");
    printf("stack size was: \n");
    /* Obtain the current limits. */
    getrlimit (RLIMIT_STACK, &rl);
    printf("stack size was: %ld\n",rl.rlim_cur);
    /* Set the stack limit */
    rl.rlim_cur = 900000000;
    setrlimit (RLIMIT_STACK, &rl);
    printf("stack size is now: %ld\n",rl.rlim_cur);
    pthread_mutexattr_init(&attrmutex);
    pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);
    
    pmutex = (pthread_mutex_t *)mmap(NULL, sizeof(numOfRunningProcess),PROT_READ| PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,sysconf(_SC_PAGE_SIZE));
    pthread_mutex_init(pmutex, &attrmutex);
    
    numOfRunningProcess = (int *)mmap(NULL, sizeof(numOfRunningProcess),PROT_READ| PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,sysconf(_SC_PAGE_SIZE));
    *numOfRunningProcess = 1;
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
    merge_sort_multiprocess(&start_block);
    printf("---ending\n");
    printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
    exit(EXIT_SUCCESS);
}

