#include <iostream>
#include <semaphore.h>
#include "../semaphores-pthreads/zemaphore.h"

zem_t counter_sem, oxy_sem, hydro_sem, bond_sem;
int nhydro = 0, nmol = 0;

int count = 0;
void bond(std::string atom, int thread_id) {
    if ((count % 3 == 2 && atom != "Oxygen") || (count % 3 != 2 && atom != "Hydrogen")) {
        std::cerr << "ERROR " << count << std::endl;
    }

    std::cout << atom << " thread " << thread_id << " bonding for molecule " << nmol << std::endl
        << (count % 3 == 2 ? "\n" : "");
    count++;
}

// let only oxygen post all sems, when bonding is done
// sem that allows 2 h, 1 o. when 3 of them are done, only o posts all sems.
// this allows another 2 h 1 o to pass. 

void* hydro_worker(void* args) {
    long long thread_id = (long long)args;

    zem_down(&hydro_sem); // 2 h can pass until 1 o passes

    zem_down(&counter_sem);
    nhydro++; // to only make the second hydrogen wake the oxygen
    bond("Hydrogen", thread_id); // to make printing atomic
    zem_up(&counter_sem);

    if (nhydro == 2) {
        zem_up(&bond_sem); // 2nd h signals o that it can bond
    }

    return NULL;
}

void* oxy_worker(void* args) {
    long long thread_id = (long long)args;

    zem_down(&oxy_sem); // only 1 o can pass

    zem_down(&bond_sem); // waiting for the 2nd h to pass

    zem_down(&counter_sem);
    bond("Oxygen", thread_id);
    nhydro -= 2;
    zem_up(&counter_sem);

    nmol++;

    // since o is the last atom to pass for a mol, it wakes up the next batch
    zem_up(&oxy_sem);
    zem_up(&hydro_sem);
    zem_up(&hydro_sem);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Expected argument count: 2, but got " << argc << ". Pass in molecule count." << std::endl;
        exit(1);
    }

    int num_o = std::atoi(argv[1]);

    zem_init(&counter_sem, 1); // used as mutex
    zem_init(&oxy_sem, 1);
    zem_init(&hydro_sem, 2);
    zem_init(&bond_sem, 0);

    pthread_t oxythreads[num_o], hydrothreads[2 * num_o];

    for (size_t i = 0; i < num_o; i++) {
        pthread_create(&oxythreads[i], NULL, oxy_worker, (void*)i);
        pthread_create(&hydrothreads[i], NULL, hydro_worker, (void*)i);
        pthread_create(&hydrothreads[num_o + i], NULL, hydro_worker, (void*)(num_o + i));
    }

    for (size_t i = 0; i < num_o; i++) {
        pthread_join(oxythreads[i], NULL);
        pthread_join(hydrothreads[i], NULL);
        pthread_join(hydrothreads[num_o + i], NULL);
    }

    return 0;
}