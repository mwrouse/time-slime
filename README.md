[![Time Slime][time-slime-logo]](#)

[![Time Slime - Scooby Doo Villain][time-slime-villain]](http://scoobydoo.wikia.com/wiki/Time_Slime)

Time Slime is a [C library](#library-documentation) for implementation of a basic time sheet program.

It uses an SQLITE database to store logs, and allows you to clock in/out as well as add a set number of hours.

There is also a [shell interface](#terminal-usage) for using only the library from the terminal.

&nbsp;

# Getting Started

To use this as a library in another program, you need the `timeslime.h` and `timeslime.c` files.

If your project already includes SQLITE, then replace this line, in `timeslime.h`:
```c
#include "third_party/sqlite3/sqlite3.h"
```
With:
```c
#include "path/to/sqlite3.h"
```
Pointing to your `sqlite3.h` file.

If your project is NOT using SQLITE, then you need to take `third_party/sqlite3/sqlite3.h` and `third_party/sqlite3/sqlite3.c` as well,
and change the path to `sqlite3.h` in `timeslime.h` based on where you place the files.

## Building
To build the command line utility, you can just run:

> make

on Windows or Linux.

&nbsp;

# Library Documentation

The Time Slime library has the following functions available for use:
```c
TIMESLIME_STATUS_t TimeSlime_Initialize(char directory_for_database[]);

TIMESLIME_STATUS_t TimeSlime_Close(void);

TIMESLIME_STATUS_t TimeSlime_AddHours(float hours, TIMESLIME_DATE_t date);

TIMESLIME_STATUS_t TimeSlime_ClockIn(TIMESLIME_DATETIME_t time);

TIMESLIME_STATUS_t TimeSlime_ClockOut(TIMESLIME_DATETIME_t time);

TIMESLIME_STATUS_t TimeSlime_GetReport(TIMESLIME_DATE_t start, TIMESLIME_DATE_t end, TIMESLIME_REPORT_t **out);

void TimeSlime_FreeReport(TIMESLIME_REPORT_t **report);

char*  TimeSlime_StatusCode(TIMESLIME_STATUS_t status);
```

### Time Slime Status
`TIMESLIME_STATUS_t` is a type alias for `int`, and can be one of the following (defined in `timeslime.h`):

|Value|Description|
|-----|------------|
|`TIMESLIME_OK`|No problems or errors|
|`TIMESLIME_UNKOWN_ERROR`|Unkown error prevented function from finishing|
|`TIMESLIME_SQLITE_ERROR`|Problem executing SQLITE actions|
|`TIMESLIME_INVALID_YEAR`|Invalid year in parameter object|
|`TIMESLIME_INVALID_MONTH`|Invalid month in parameter object|
|`TIMESLIME_INVALID_DAY`|Invalid day in parameter object|
|`TIMESLIME_INVALID_HOUR`|Invalid hour in parameter object|
|`TIMESLIME_INVALID_MINUTE`|Invalid minute in parameter object|
|`TIMESLIME_ALREADY_CLOCKED_IN`|Unable to clock in since a clock out action has not been performed|
|`TIMESLIME_NOT_CLOCKED_IN`|Unable to clock out since a clock in action has not been performed|
|`TIMESLIME_NO_ENTIRES`|No time sheet entries were found for a given date range|
|`TIMESLIME_NOT_INITIALIZED`|`TimeSlime_Initialize(char[])` has not been called yet|


If you want to get a string key that represents a status code,
use the `TimeSlime_StatusCode(TIMESLIME_STATUS_t)` method, and pass in the status code. A string will be returned.

## Inititialization
The `TimeSlime_Initialize(char[])` function needs to be called before any other Time Slime methods. This is responsible for creating the SQLITE database if it does not exist.

The parameter passed to this should be the directory to place the the `timeslime.db` file (**WITHOUT** a trailing slash).

## Closing
The `TimeSlime_Close()` function needs to be called before exiting your program, it is responsible for safely disposing of allocated memory.


## Adding Hours
It might be desired to add a set number of hours to a time sheet for a specific date (rather than clocking in and out).

This is where the `TimeSlime_AddHours(float, TIMESLIME_DATE_t)` functions comes in.

The function accepts a `float`, which is the number of hours worked. Then a `TIMESLIME_DATE_t` struct, which is the date to add the hours to.

[See more about `TIMESLIME_DATE_t`](#library-datatypes).

## Clocking In and Out
To clock in and out of the time sheet, use the `TimeSlime_ClockIn(TIMESLIME_DATETIME_t)` and `TimeSlime_ClockOut(TIMESLIME_DATETIME_t)` functions.

Each function accepts a `TIMESLIME_DATETIME_t` struct, which represents the date and time that the clock in, clock out should be performed on.

[See more about `TIMESLIME_DATETIME_t`](#library-datatypes).


## Reports
Generating a report will show you how many hours have been worked per day for a certain date range.

`TimeSlime_GetReport(TIMESLIME_DATE_t start, TIMESLIME_DATE_t end, TIMESLIME_REPORT_t **out)`
will generate a report between the `start` and `end` dates.

The result will be placed in the `TIMESLIME_REPORT_t` pointer, and this needs to be passed a pointer to that pointer.

When you are done, use `TimeSlime_FreeReport(TIMESLIME_REPORT_t**)` to clear allocated memory.

[See more about `TIMESLIME_DATE_t` and `TIMESLIME_REPORT_t`](#library-datatypes).

&nbsp;

# Library Datatypes
To avoid conflicts with other libraries, Time Slime defines custom datatypes for use with the library.

## Date and DateTime
The Date and DateTime structs are passed to several Time Slime functions.

```c
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
```

Time Slime also defines some helpful directives to easily create these for the current time.

```c
#define TIMESLIME_DATE_NOW         (TIMESLIME_DATE_t){ 0, 0, 0}
#define TIMESLIME_TIME_NOW         (TIMESLIME_DATETIME_t){ 0, 0, 0, -1, -1 }
```

## Report
The `TIMESLIME_STATUS_REPORT_t` struct looks like:
```c
// Report Entry
struct TIMESLIME_REPORT_ENTRY_STRUCT
{
    float Hours;
    char Date[];
};
typedef struct TIMESLIME_REPORT_ENTRY_STRUCT TIMESLIME_REPORT_ENTRY_t;

// Time Sheet Report
struct TIMESLIME_REPORT_STRUCT
{
    int NumberOfEntries;
    TIMESLIME_REPORT_ENTRY_t Entries[];
};
typedef struct TIMESLIME_REPORT_STRUCT TIMESLIME_REPORT_t;
```


&nbsp;

# Terminal Usage
Once build, if you add the executable (in the `build` folder) to your system `PATH`, you can run it with the following commands:

```shell
# Show information and command help
> timeslime help

# Add an amount of time to the current date
> timeslime add [hours]

# Add an amount of time to a specific date
> timeslime add [hours] [date]

# Clock in
> timeslime clock in

# Clock out
> timeslime clock out

# Run a report
> timeslime report [start-date] [end-date]
```

> **IMPORTANT**: All dates must be formatted as either `YYYY-MM-DD` or `YYYY/MM/DD`

&nbsp;

# Todo
- [ ] Better report formatting (done?)
- [ ] Allow second parameter of a report to be "today"
- [ ] Shell program to prompt user for choices if no parameters (or not all parameters) are given (but works on all systems)
- [ ] Imrpove logging


&nbsp;

&nbsp;

Font used in logo is [*Liquidism*](https://www.dafont.com/liquidism.font?fpp=50)


[time-slime-logo]: extras/images/logo.png
[time-slime-villain]: extras/images/Time_Slime.jpg