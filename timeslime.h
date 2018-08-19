/**
 * Time Slime - Command Line Time Sheet
 *
 * Authors: Michael Rouse
 */
#ifndef __TIME_SLIME_H__
#define __TIME_SLIME_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Datatypes */
typedef int TIMESLIME_STATUS_t;

/* Constants */
/* These are used when you want to use the current date */
#define TIMESLIME_DATE_NOW          0, 0, 0
#define TIMESLIME_TIME_NOW          0, 0
#define TIMESLIME_CLOCK_IN_NOW      TIMESLIME_DATE_NOW, TIMESLIME_TIME_NOW
#define TIMESLIME_CLOCK_OUT_NOW     TIMESLIME_DATE_NOW, TIMESLIME_TIME_NOW

#ifndef TIMESLIME_DATABASE_FILE_NAME
#define TIMESLIME_DATABASE_FILE_NAME    "timeslime.db"
#endif

#ifndef TIMESLIME_DEFAULT_RESULT_LIMIT
#define TIMESLIME_DEFAULT_RESULT_LIMIT      1000
#endif

#define TIMESLIME_DATETIME_STR_LENGTH   50

/* Result Codes */
#define TIMESLIME_OK                0

#define TIMESLIME_UNKOWN_ERROR      100
#define TIMESLIME_SQLITE_ERROR      50

#define TIMESLIME_INVALID_YEAR      10
#define TIMESLIME_INVALID_MONTH     11
#define TIMESLIME_INVALID_DAY       12
#define TIMESLIME_INVALID_HOUR      13
#define TIMESLIME_INVALID_MINUTE    14
#define TIMESLIME_INVALID_TIMESTAMP 15
#define TIMESLIME_INVALID_DATE      16

#define TIMESLIME_ALREADY_CLOCKED_IN    60  /* When you try to clock in without clocking out */
#define TIMESLIME_NOT_CLOCKED_IN        61 /* When you try to clock out without clocking in */



/* Initialize the Time Slime library */
TIMESLIME_STATUS_t TimeSlime_Initialize(char directory_for_database[]);

/* Safely close out of the Time Slime library */
TIMESLIME_STATUS_t TimeSlime_Close(void);

/* Add to the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_AddHours(float hours, int year, int month, int day);

/* Clock in to the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_ClockIn(int year, int month, int day, int hour, int minute);

/* Clock out of the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_ClockOut(int year, int month, int day, int hour, int minute);

/* Gets the time sheet for a period of time */
TIMESLIME_STATUS_t TimeSlime_GetTimeSheet(void);


#endif