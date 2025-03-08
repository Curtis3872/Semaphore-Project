#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>
#include <semaphore.h>
#include "dungeon_info.h"

struct Dungeon* dungeon;
sem_t *sem2;

void decode_caesar_cipher(char *input, char *output, int shift) {
    int len = strlen(input);
    for (int i = 0; i < len; i++) {
        char c = input[i];
        if (islower(c)) {
            // Decode lowercase letters
            output[i] = ((c - 'a' - shift + 26 * 2) % 26) + 'a';
        } else if (isupper(c)) {
            // Decode uppercase letters
            output[i] = ((c - 'A' - shift + 26 * 2) % 26) + 'A';
        } else {
            // Leave non-alphabetical characters unchanged
            output[i] = c;
        }
    }
    output[len] = '\0';
}

void signal_handler(int sig) {
    if (sig == DUNGEON_SIGNAL) {   // receives dungeon signal
        int shift = (int)(dungeon->barrier.spell[0]) % 26; //use first character as shift and mod its asci value
        decode_caesar_cipher(dungeon->barrier.spell + 1, dungeon->wizard.spell, shift);
        printf("Decoded spell: %s\n", dungeon->wizard.spell);
        fflush(stdout);
}
    if (sig == SEMAPHORE_SIGNAL) {  // receives semaphore signal
	sem_wait(sem2);   // ups lever two
	}
}

int main() {
    int shm_fd;

    shm_fd = shm_open(dungeon_shm_name, O_RDWR, 0666);  // opening shared memory
    sem2 = sem_open(dungeon_lever_two,O_RDWR, 0666);   // opening lever_two semaphore

    //errors for if opening shared memory fails
    if (shm_fd == -1) {
        perror("Error opening shared memory in wizard");
        exit(EXIT_FAILURE);
    }
    
    // Map the shared memory object
    dungeon = mmap(0, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (dungeon == MAP_FAILED) {
        perror("Error mapping shared memory in wizard");
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


    while (dungeon->running) {  // keeps wizard open while dungeon is running
        usleep(1000);
    }
    // Close everything after
    sem_close(sem2);
    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
    return 0;
}
