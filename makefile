# Time Slime Make File
CC = gcc
CC_FLAGS = -g -Wall
CC_ENDFLAGS = -lpthread

BUILD_DIR = build
SHELL_OUT = $(BUILD_DIR)/timeslime

SHELL_SOURCES = shell/shell.c shell/args/args.c
SHELL_HEADERS = shell/args/args.h

LIBRARY_SOURCES = timeslime.c third_party/sqlite3/sqlite3.c
LIBRARY_HEADERS = timeslime.h third_party/sqlite3/sqlite3.h

MKDIR = mkdir

ifeq ($(OS),Windows_NT)
	RMDIR = cmd /C rmdir /S /Q
else
	RMDIR = rmdir -f -r
	CC_ENDFLAGS += -ldl
endif

.PHONY: all
all: build_executable

# Enable debugging output
debug: CC_FLAGS += -DDEBUG
debug: all


build_executable: $(LIBRARY_SOURCES) $(LIBRARY_HEADERS) $(SHELL_SOURCES)
	@$(CC) $(CC_FLAGS) $(LIBRARY_SOURCES) $(SHELL_SOURCES) -o $(SHELL_OUT) $(CC_ENDFLAGS)


prep:
	@$(MKDIR) $(BUILD_DIR)


clean: clean_all
clean_all:
	@$(RMDIR) $(SHELL_OUT)
