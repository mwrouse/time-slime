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

/* Constants */
/* These are used when you want to use the current date */
#define TIMESLIME_DATE_NOW         (TIMESLIME_DATE_t){ 0, 0, 0}
#define TIMESLIME_TIME_NOW         (TIMESLIME_DATETIME_t) { 0, 0, 0, 0, 0 }

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

#define TIMESLIME_NO_ENTIRES        80


/* Datatypes */
typedef int TIMESLIME_STATUS_t;

struct TIMESLIME_DATE_STRUCT
{
    int year;
    int month;
    int day;
};
typedef struct TIMESLIME_DATE_STRUCT TIMESLIME_DATE_t;

struct TIMESLIME_DATETIME_STRUCT
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
};
typedef struct TIMESLIME_DATETIME_STRUCT TIMESLIME_DATETIME_t;

// Report Entry
struct TIMESLIME_REPORT_ENTRY_STRUCT
{
    float Hours;
    char Date[TIMESLIME_DATETIME_STR_LENGTH];
};
typedef struct TIMESLIME_REPORT_ENTRY_STRUCT TIMESLIME_REPORT_ENTRY_t;

// Time Sheet Report
struct TIMESLIME_REPORT_STRUCT
{
    int NumberOfEntries;
    TIMESLIME_REPORT_ENTRY_t Entries[];
};
typedef struct TIMESLIME_REPORT_STRUCT TIMESLIME_REPORT_t;



/* Initialize the Time Slime library */
TIMESLIME_STATUS_t TimeSlime_Initialize(char directory_for_database[]);

/* Safely close out of the Time Slime library */
TIMESLIME_STATUS_t TimeSlime_Close(void);

/* Add to the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_AddHours(float hours, TIMESLIME_DATE_t date);

/* Clock in to the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_ClockIn(TIMESLIME_DATETIME_t time);

/* Clock out of the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_ClockOut(TIMESLIME_DATETIME_t time);

/* Gets the time sheet for a period of time */
TIMESLIME_STATUS_t TimeSlime_GetReport(TIMESLIME_DATE_t start, TIMESLIME_DATE_t end, TIMESLIME_REPORT_t **out);

/* Frees memory allocated for report used in report method */
void TimeSlime_FreeReport(TIMESLIME_REPORT_t **report);

/* Converts status to friendly error code (or returns SQLITE error string) */
char*  TimeSlime_StatusCode(TIMESLIME_STATUS_t status);



/* Queries (Do not touch) */
#define __TS_QRY_GET_COMPLTED_CLOCK_ENTRIES "(ClockInTime IS NOT NULL AND ClockOutTime IS NOT NULL)"
#define __TS_QRY_GET_PARTIAL_CLOCK_ENTRIES "(ClockOutTime IS NULL AND ClockInTime IS NOT NULL)"
#define __TS_QRY_GET_ADDED_HOURS "(HoursAdded <> 0.0 AND HoursAddedDate IS NOT NULL)"

#define __TS_QRY_GET_ALL_ENTIRES __TS_QRY_GET_COMPLTED_CLOCK_ENTRIES " OR " __TS_QRY_GET_ADDED_HOURS

#endif