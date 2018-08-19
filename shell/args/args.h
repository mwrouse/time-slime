#ifndef __ARGS_H__
#define __ARGS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGS_False   0
#define ARGS_True    1
typedef char args_bool_t;

typedef struct _args_t {
    args_bool_t help;
    char *action;
    char *modifier1;
    char *modifier2;
    char *modifier3;

    int error;
} args_t;

typedef struct _date_t {
    args_bool_t error;
    int month;
    int day;
    int year;
    char str[256];
} date_t;

/* Parse the command line arguments */
args_t args_parse(int argc, char **argv);

/* Parse into a date */
date_t args_parse_date(char *date);

char* args_get_directory_of_executable(char *name);


#endif