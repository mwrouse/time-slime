/**
 * File with directives that represent commands and such
 */
#ifndef __COMMANDS_H__
#define __COMMANDS_H__


/* ACTIONS */
#define ADD_ACTION      "add"
#define ADD_ACTION_DESCRIPTION "Add hours to your time sheet for a certain day. Defaults to the current date"

#define CLOCK_ACTION    "clock"
#define CLOCK_ACTION_DESCRIPTION "Clock in or clock out of your time sheet"

#define REPORT_ACTION   "report"
#define REPORT_ACTION_DESCRIPTION "Show all hours worked between two date ranges"

#define HELP_ACTION     "help"
#define HELP_ACTION_DESCRIPTION "Receive information on how to use the program or an action"


/* Misc. */
#define TODAY   "today"

#define CLOCK_IN    "in"
#define CLOCK_OUT   "out"



/* About */
#define PROGRAM_NAME    "timeslime"
#define AUTHOR          "Michael Rouse"
#define DESCRIPTION     "\nTime Slime is a command line utility to keep \ntrack of hours worked remotely. \nA time sheet in your terminal!"
#define PROGRAM_VERSION "2018.08.14"


#define DATABASE_FILE       "timeslime.db"
#define DATABASE_TABLE      "meow"

#endif