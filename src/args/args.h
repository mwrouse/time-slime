#ifndef __ARGS_H__
#define __ARGS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../logger.h"
#include "../string_literals.h"


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

typedef struct _timeslime_date_t {
    bool error;
    int month;
    int day;
    int year;
    char str[256];
} timeslime_date;

/* Parse the command line arguments */
timeslime_args parse_args(int argc, char **argv);

/* Parse into a date */
timeslime_date parse_date(char *date);

#endif