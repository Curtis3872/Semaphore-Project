#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#include "dungeon_info.h"

int main() {
    int shm_fd;
    pid_t wizard_pid, rogue_pid, barbarian_pid;

    struct Dungeon *dungeon;
    shm_fd = shm_open(dungeon_shm_name, O_CREAT | O_RDWR, 0666);    //creating/opening shared memory
    sem_t * sem1 = sem_open(dungeon_lever_one, O_CREAT, 0666,1);    //creating/opening dungeon_lever_one semaphore
    sem_t * sem2 = sem_open(dungeon_lever_two, O_CREAT, 0666,1);    //creating/opening dungeon_lever_two semaphore
    if (shm_fd == -1) {   // Error for if the shared memory isn't created
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(struct Dungeon)) == -1) {
        perror("Error configuring shared memory size");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Map the shared memory object
    dungeon = mmap(0, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (dungeon == MAP_FAILED) {
        perror("Error mapping shared memory");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // start the dungeon
    dungeon->running = true;

    // Fork and exec the wizard process
    wizard_pid = fork();
    if (wizard_pid == 0) {
        // Child process for wizard
        execl("./wizard", "wizard", NULL);
        perror("Failed to start wizard process");
        exit(EXIT_FAILURE);
    }

    // Fork and exec the rogue process
    rogue_pid = fork();
    if (rogue_pid == 0) {
        // Child process for rogue
        execl("./rogue", "rogue", NULL);
        perror("Failed to start rogue process");
        exit(EXIT_FAILURE);
    }

    // Fork and exec the barbarian process
    barbarian_pid = fork();
    if (barbarian_pid == 0) {
        // Child process for barbarian
        execl("./barbarian", "barbarian", NULL);
        perror("Failed to start barbarian process");
        exit(EXIT_FAILURE);
    }

    // Now call RunDungeon with the pids of the wizard, rogue, and barbarian
    if (wizard_pid > 0 && rogue_pid > 0 && barbarian_pid > 0) {
        printf("All character processes started successfully.\n");
	sleep(1);
        RunDungeon(wizard_pid, rogue_pid, barbarian_pid);  // adds the three classes to the dungeon
    } else {
        fprintf(stderr, "Error: Failed to start one or more character processes.\n");
    }
    
    // Close everything after
    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
    shm_unlink(dungeon_shm_name);
    return 0;
}
