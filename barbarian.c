#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "dungeon_info.h"



struct Dungeon* dungeon;
sem_t *sem1;

void signal_handler(int sig) {
    if (sig == DUNGEON_SIGNAL) { // receives dungeon signal
         dungeon->barbarian.attack = dungeon->enemy.health;
	}
    if (sig == SEMAPHORE_SIGNAL) {   // receives semaphore signal
	sem_wait(sem1);  // ups lever one
	}
}

int main() {
    int shm_fd;

    // Open the existing shared memory object
    shm_fd = shm_open(dungeon_shm_name,O_RDWR, 0666); // opening shared memory
    sem1 = sem_open(dungeon_lever_one,O_RDWR, 0666); // opening lever_one semaphore
    //errors for if opening shared memory fails
    if (shm_fd == -1) {
        perror("Error opening shared memory in barbarian");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory object
    dungeon = mmap(0, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (dungeon == MAP_FAILED) {
        perror("Error mapping shared memory in barbarian");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    //send the signals to signal_handler
    sigaction(DUNGEON_SIGNAL, &sa, NULL);
    sigaction(SEMAPHORE_SIGNAL, &sa, NULL);


    while (dungeon->running) {  // keeps barbarian open while dungeon is running
        usleep(1000);
    }
    // Close everything after
    sem_close(sem1);
    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
    return 0;

}