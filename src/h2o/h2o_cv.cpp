#include "../semaphores-pthreads/zemaphore.h"
#include <iostream>
#include <stdio.h>
#include <pthread.h>

/**
 * Whe
 * 
 */

pthread_t thread_oxy;
pthread_t thread_hydro;

pthread_mutex_t lock;
int oxy, hydro;
int count = 0;

pthread_cond_t oxy_cond;
pthread_cond_t hydro_cond;

void bond() {
    count++;
    std::cout << "bonding H2O " << count << std::endl;
}

void* oxygene(void* args) {
    pthread_mutex_lock(&lock);
    oxy++;

    if (hydro >= 2 && oxy >= 1) {
        bond();
        hydro = hydro - 2;
        oxy--;
        pthread_cond_signal(&oxy_cond);
        pthread_cond_signal(&hydro_cond);
        pthread_cond_signal(&hydro_cond);
    } else {
        pthread_cond_wait(&oxy_cond, &lock);
    }

    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

void* hydrogen(void* args) {
    pthread_mutex_lock(&lock);
    hydro++;

    if (hydro >= 2 && oxy >= 1) {
        bond();
        hydro = hydro - 2;
        oxy--;
        pthread_cond_signal(&oxy_cond);
        pthread_cond_signal(&hydro_cond);
        pthread_cond_signal(&hydro_cond);
    } else {

        pthread_cond_wait(&hydro_cond, &lock);
    }

    pthread_mutex_unlock(&lock);
    pthread_exit(0);
}

int main(int argc, char* argv[]) {
    int num_o = std::atoi(argv[1]);

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&oxy_cond, NULL);
    pthread_cond_init(&hydro_cond, NULL);
    printf("Starting..\r\n");

    for (int k = 0; k < num_o; k++) {
        if (pthread_create(&thread_oxy, NULL, oxygene, NULL)) {
            std::cout << "Error while creating thread" << std::endl;
        }
    }

    for (int k = 0; k < 2 * num_o; k++) {
        if (pthread_create(&thread_hydro, NULL, hydrogen, NULL)) {
            std::cout << "Error while creating thread" << std::endl;
        }
    }

    pthread_join(thread_oxy, NULL);
    pthread_join(thread_hydro, NULL);

    return 0;
}