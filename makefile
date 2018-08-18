# Time Slime Make File
CC = gcc
CC_FLAGS = -g -Wall

BUILD_DIR = build
SHELL_OUT = $(BUILD_DIR)/timeslime

SHELL_SOURCES = shell/shell.c shell/args/args.c
SHELL_HEADERS = shell/args/args.h

LIBRARY_SOURCES = timeslime.c sqlite3.c
LIBRARY_HEADERS = timeslime.h sqlite3.h

MKDIR = mkdir

ifeq ($(OS),Windows_NT)
	RMDIR = cmd /C rmdir /S /Q
else
	RMDIR = rmdir -f -r
endif

.PHONY: all
all: build_executable

# Enable debugging output
debug: CC_FLAGS += -DDEBUG
debug: all


build_executable: prep $(LIBRARY_SOURCES) $(LIBRARY_HEADERS) $(SHELL_SOURCES)
	@$(CC) $(CC_FLAGS) $(LIBRARY_SOURCES) $(SHELL_SOURCES) -o $(SHELL_OUT)


prep: clean
	@$(MKDIR) $(BUILD_DIR)


clean: clean_all
clean_all:
	@$(RMDIR) $(BUILD_DIR)
