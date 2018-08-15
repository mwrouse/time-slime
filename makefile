# Time Slime Make File
CC=gcc
CFLAGS=-g -Wall
LANGUAGE_FLAGS=-x c
OUT=timeslime

# Files to compile (for all targets)
SOURCES=./src/*.c ./src/args/*.c



all:
	$(CC) $(CFLAGS) $(LANGUAGE_FLAGS) $(SOURCES) -o $(OUT)
