#ifndef __ARGS_H__
#define __ARGS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Constants */
#define HELP_ARG    "help"


#define False   0
#define True    1
typedef char bool;

typedef struct _timeslime_args_t {
    bool help;
    char *action;
    char *modifier1;
    char *modifier2;
    char *modifier3;

    int error;
} timeslime_args;


/* Parse the command line arguments */
timeslime_args parse_args(int argc, char **argv);

#endif