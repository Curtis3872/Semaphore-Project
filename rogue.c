#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <semaphore.h>
#include "dungeon_info.h"

struct Dungeon* dungeon;
sem_t *sem1;
sem_t *sem2;


void pick_lock() {
	//binary search
    float low = 0, high = MAX_PICK_ANGLE, mid = 0;
    while (dungeon->trap.locked) {
        float mid = (low + high) / 2.0;
        dungeon->rogue.pick = mid;  // Update the pick position
        dungeon->trap.direction = 't';
        usleep(TIME_BETWEEN_ROGUE_TICKS); // Simulate time between checks

        if (dungeon->trap.direction == 't') {
            continue;
        } else if (dungeon->trap.direction == 'u') {
            low = mid + 1;  // Adjust the range upwards
        } else if (dungeon->trap.direction == 'd') {
            high = mid - 1;  // Adjust the range downwards
        } else {
            break;
	}
    }
}

void signal_handler(int sig) {
    if (sig == DUNGEON_SIGNAL) { // receives dungeon signal
        pick_lock(); // start pick lock
}
    if (sig == SEMAPHORE_SIGNAL) {  // receives semaphore signal
	sleep(1);
	for (int i = 0; i < 4; i++){
	    while (dungeon->treasure[i] == '\0') { //for null space
	        sleep(1);
    	    }
		dungeon->spoils[i] = dungeon->treasure[i];  // collects treasure
	}
    // sems to close the doors
    sem_post(sem1);
    sem_post(sem2);
	}
}

int main() {
    int shm_fd;

    shm_fd = shm_open(dungeon_shm_name, O_RDWR, 0666);   // opening shared memory
    sem1 = sem_open(dungeon_lever_one, O_RDWR, 0666);  // opening lever_one semaphore
    sem2 = sem_open(dungeon_lever_two, O_RDWR, 0666);  // opening lever_two semaphore
    
    //errors for if opening shared memory fails
    if (shm_fd == -1) {
        perror("Error opening shared memory in rogue");
        exit(EXIT_FAILURE);
    }
    // Map the shared memory object
    dungeon = mmap(0, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (dungeon == MAP_FAILED) {
        perror("Error mapping shared memory in rogue");
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

    while (dungeon->running) {   // keeps rogue open while dungeon is running
        usleep(1000);
    }
    // Close everything after
    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
    sem_close(sem1);
    sem_close(sem2);
    return 0;
}
