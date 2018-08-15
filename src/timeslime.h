/**
 * Time Slime - Command Line Time Sheet
 *
 * Authors: Michael Rouse
 */
#ifndef __TIME_SLIME_H__
#define __TIME_SLIME_H__

#include <stdlib.h>
#include <string.h>

#include "args/args.h"
#include "logger.h"
#include "string_literals.h"


/* Add to the time sheet */
void perform_add_action(timeslime_args args);

/* Clock In or Out */
void perform_clock_action(timeslime_args args);

/* Show time worked for a period of time */
void perform_report_action(timeslime_args args);

/* Displays the help screen */
void display_help(void);

#endif