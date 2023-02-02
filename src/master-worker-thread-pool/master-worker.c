#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce, curr_buf_size, nitems_consumed;
int total_items, max_buf_size, num_workers, num_masters;

pthread_mutex_t buflock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t emptybuf = PTHREAD_COND_INITIALIZER, fullbuf = PTHREAD_COND_INITIALIZER;
int* buffer;

void print_produced(int num, int master) {
    printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {
    printf("Consumed %d by worker %d\n", num, worker);
}

// produce items and place in buffer
// modify code below to synchronize correctly
void* generate_requests_loop(void* data) {
    int thread_id = *((int*)data);

    while (1) {
        pthread_mutex_lock(&buflock);

        while (curr_buf_size == max_buf_size && item_to_produce < total_items) {
            pthread_cond_wait(&fullbuf, &buflock);
        }

        if (item_to_produce >= total_items) {
            // task complete: one producer waking another
            pthread_cond_signal(&fullbuf);

            pthread_mutex_unlock(&buflock);
            break;
        } else {
            // produce
            buffer[curr_buf_size++] = item_to_produce;
            print_produced(item_to_produce++, thread_id);

            pthread_cond_signal(&emptybuf);
        }

        pthread_mutex_unlock(&buflock);
    }

    return 0;
}

// write function to be run by worker threads
// ensure that the workers call the function print_consumed when they consume an item
void* consume_requests_loop(void* data) {
    int thread_id = *((int*)data);

    while (1) {
        pthread_mutex_lock(&buflock);

        while (curr_buf_size == 0 && nitems_consumed < total_items) {
            pthread_cond_wait(&emptybuf, &buflock);
        }

        if (nitems_consumed >= total_items) {
            // task complete: one consumer waking another
            pthread_cond_signal(&emptybuf);

            pthread_mutex_unlock(&buflock);
            break;
        } else {
            // consume
            print_consumed(buffer[--curr_buf_size], thread_id);
            nitems_consumed++;

            pthread_cond_signal(&fullbuf);
        }

        pthread_mutex_unlock(&buflock);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int* master_thread_ids;
    pthread_t* master_threads;
    int* worker_thread_ids;
    pthread_t* worker_threads;
    item_to_produce = 0;
    nitems_consumed = 0;
    curr_buf_size = 0;

    int i;

    if (argc < 5) {
        printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
        exit(1);
    } else {
        num_masters = atoi(argv[4]);
        num_workers = atoi(argv[3]);
        total_items = atoi(argv[1]);
        max_buf_size = atoi(argv[2]);
    }

    buffer = (int*)malloc(sizeof(int) * max_buf_size);

    // create master producer threads
    master_thread_ids = (int*)malloc(sizeof(int) * num_masters);
    master_threads = (pthread_t*)malloc(sizeof(pthread_t) * num_masters);
    for (i = 0; i < num_masters; i++) {
        master_thread_ids[i] = i;
        pthread_create(&master_threads[i], NULL, generate_requests_loop, (void*)&master_thread_ids[i]);
    }

    // create worker consumer threads
    worker_thread_ids = (int*)malloc(sizeof(int) * num_workers);
    worker_threads = (pthread_t*)malloc(sizeof(pthread_t) * num_workers);
    for (i = 0; i < num_workers; i++) {
        worker_thread_ids[i] = i;
        pthread_create(&worker_threads[i], NULL, consume_requests_loop, &worker_thread_ids[i]);
    }

    // wait for all threads to complete
    for (i = 0; i < num_masters; i++) {
        pthread_join(master_threads[i], NULL);
        printf("master %d joined\n", i);
    }

    for (i = 0; i < num_workers; i++) {
        pthread_join(worker_threads[i], NULL);
        printf("worker %d joined\n", i);
    }

    /*----Deallocating Buffers---------------------*/
    free(buffer);
    free(master_thread_ids);
    free(master_threads);
    free(worker_thread_ids);
    free(worker_threads);

    return 0;
}
