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
#include "commands.h"


/* Add to the time sheet */
void perform_add_action(timeslime_args args);

#endif