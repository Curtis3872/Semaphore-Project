# Purpose: Makefile for the game, barbarian, wizard, and rogue executables.
# can do make clean and make run


# Compiler
CC = gcc

# Compiler flags
CFLAGS = -std=gnu99 -g 
# Linker flags
LDFLAGS = -lrt -lpthread 

# Executables to be built
EXECUTABLES = game barbarian wizard rogue

# Default target
all: $(EXECUTABLES)

# Specific rules for each executable
game: game.c dungeon.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

barbarian: barbarian.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

wizard: wizard.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

rogue: rogue.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Run the game
run: $(EXECUTABLES)
	./game

# Clean up
clean:
	rm -f $(EXECUTABLES)

.PHONY: all run clean