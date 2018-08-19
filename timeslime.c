/**
 * Time Slime - Command Line Time Sheet
 *
 * Authors: Michael Rouse
 */
#include "timeslime.h"
#include "sqlite3.h"

/* Variables */
static sqlite3 *db;
static char *database_file_path;


/* Functions */
static TIMESLIME_STATUS_t _TimeSlime_CreateTables(void);


/**
 * Initializes the Time Slime library
 */
TIMESLIME_STATUS_t TimeSlime_Initialize(char directory_for_database[])
{
    // Generate path for database file
    database_file_path = malloc((strlen(directory_for_database) + strlen("\\timeslime.db")) * sizeof(char));
    if (database_file_path == NULL)
        return TIMESLIME_UNKOWN_ERROR;
    sprintf(database_file_path, "%s\\%s", directory_for_database, "timeslime.db"); // Append the file name

    // Create database if it doesn't exist
    db = NULL;
    int rc = sqlite3_open(database_file_path, &db);
    if (rc != SQLITE_OK)
        return TIMESLIME_SQLITE_ERROR;

    return _TimeSlime_CreateTables();
}

/**
 * Safely close the Time Slime library
 */
TIMESLIME_STATUS_t TimeSlime_Close(void)
{
    if (db != NULL)
        sqlite3_close(db);

    if (database_file_path != NULL)
    {
        free(database_file_path);
        database_file_path = NULL;
    }

    return TIMESLIME_OK;
}

/* Add to the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_AddHours(float hours, int year, int month, int day)
{
    char *errMsg;
    char sql[1000];
    int rc;

    if (month < 0 || month > 12)
        return TIMESLIME_INVALID_MONTH;
    if (day < 0 || day > 31)
        return TIMESLIME_INVALID_DAY;

    if ((year == 0) || (month == 0) || (day == 0))
    {
        // Use current database date
        sprintf(sql,    "INSERT INTO TimeSheet " \
                        "(HoursAdded, HoursAddedDate) " \
                        "VALUES " \
                        "(%.2f, DATE('now', 'localtime'))",
            hours);
    }
    else
    {
        // Use user-specified date
        sprintf(sql,    "INSERT INTO TimeSheet " \
                        "(HoursAdded, HoursAddedDate) " \
                        "VALUES " \
                        "(%.2f, '%d/%d/%d')",
            hours, year, month, day);
    }

    rc = sqlite3_exec(db, sql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        return TIMESLIME_SQLITE_ERROR;
    }

    return TIMESLIME_OK;
}

/* Clock in to the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_ClockIn(int year, int month, int day, int hour, int minute)
{
    return TIMESLIME_OK;
}

/* Clock out of the Time Slime time sheet */
TIMESLIME_STATUS_t TimeSlime_ClockOut(int year, int month, int day, int hour, int minute)
{
    return TIMESLIME_OK;
}


/**
 * Entry point for program, parsed arguments and
 * acts according to the arguments
 *
int main2(int argc, char *argv[])
{
    int i;
    for (i = 0; i < argc; i++)
        log_dull(argv[i]);
    log_dull("==== Time Slime ====\n")

    setup_database();




    timeslime_args parsed_args;
    parsed_args = parse_args(argc, argv);

    if (parsed_args.help)
    {
        display_help();
    }
    else
    {
        if (strcmp(parsed_args.action, ADD_ACTION) == 0)
            perform_add_action(parsed_args);

        else if (strcmp(parsed_args.action, CLOCK_ACTION) == 0)
            perform_clock_action(parsed_args);

        else if (strcmp(parsed_args.action, REPORT_ACTION) == 0)
            perform_report_action(parsed_args);
    }

    sqlite3_close(db);
    printf("\n");
    return 0;
}


/**
 * Add to the time sheet
 *
void perform_add_action(timeslime_args args)
{
    if (args.modifier1 == NULL)
    {
        log_error("'add' action needs at least a second parameter.")
        return;
    }

    // Parse argument to determine how long to add
    float toAdd;
    toAdd = atof(args.modifier1);

    // Verify range of time to add
    if (toAdd == 0.0)
    {
        log_error("Invalid amount of time to add to timesheet. Must be > 0 or < 0");
        return;
    }

    // Get the date to add onto the time sheet
    char *date = TODAY;
    if (args.modifier2 != NULL)
        date = args.modifier2;

    // Show message for what is about to happen
    log_info("Adding %.2f hour(s) to the time sheet for %s", toAdd, date);
}


/* Clock in and clock out of the time sheet *
void perform_clock_action(timeslime_args args)
{
    if (args.modifier1 == NULL)
    {
        log_error("'clock' action needs an additional parameter: 'in' or 'out'")
        return;
    }

    char *direction = args.modifier1;

    if (strcmp(direction, CLOCK_IN) != 0 && strcmp(direction, CLOCK_OUT) != 0)
    {
        log_error("Invalid parameter on 'clock' action, must be 'in' or 'out'");
        return;
    }

    // Show message for what is about to happen
    log_info("Clocking %s", direction);
}

/* Show all time worked *
void perform_report_action(timeslime_args args)
{
    if (args.modifier1 == NULL)
    {
        log_error("'report' action needs another paramter, the start date");
        return;
    }

    if (args.modifier2 == NULL)
    {
        log_error("'report' action needs another parameter, the end date");
        return;
    }

    timeslime_date startDate = parse_date(args.modifier1);
    if (startDate.error)
        return;

    timeslime_date endDate = parse_date(args.modifier2);
    if (endDate.error)
        return;

    // Verify that dates are in the correct order
    if (
            (endDate.month < startDate.month && endDate.year == startDate.year) || // Same year, but earlier month
            (endDate.day < startDate.day && endDate.year == startDate.year && endDate.month == startDate.month) || // Same month and year, but earlier day
            (endDate.year < startDate.year) // Earlier year
        )
    {
        log_error("Dates in wrong order");
        return;
    }


    log_info("Running report between %s and %s", startDate.str, endDate.str);
}


/* Help Screen *
void display_help(void)
{
    printf("Author: %s\n", AUTHOR);
    printf("Version: %s\n", PROGRAM_VERSION);
    printf("%s\n\n\n", DESCRIPTION);

    printf("Usage:\n");
    printf("\t%s [action] [arguments...]\n\n", PROGRAM_NAME);

    printf("Actions:\n");

    printf("\t%s\t %s\n", ADD_ACTION, ADD_ACTION_DESCRIPTION);
    printf("\t%s\t %s\n", CLOCK_ACTION, CLOCK_ACTION_DESCRIPTION);
    printf("\t%s\t %s\n", REPORT_ACTION, REPORT_ACTION_DESCRIPTION);
    printf("\n");

    printf("\t%s\t %s\n\n", HELP_ACTION, HELP_ACTION_DESCRIPTION);

    printf("\n%s Action Usage:\n", ADD_ACTION);
    printf("\t%s add (+|-)[0-9]\n", PROGRAM_NAME);
    printf("\t%s add (+|-)[0-9] YYYY/MM/DD\n\n", PROGRAM_NAME);

    printf("%s Action Usage:\n", CLOCK_ACTION);
    printf("\t%s clock (in|out)\n\n", PROGRAM_NAME);

    printf("%s Action Usage:\n", REPORT_ACTION);
    printf("\t%s report YYYY/MM/DDD YYYY/MM/DDD\n", PROGRAM_NAME);

    printf("\n");
}
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}
*/

/**
 * Creates the SQL tables (only if the file is new)
 */
static TIMESLIME_STATUS_t _TimeSlime_CreateTables(void)
{
    char *errMsg = 0;
    int rc;
    char *sql;

    // Create time sheet table
    sql =   "CREATE TABLE TimeSheet(" \
                "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
                "HoursAdded REAL NOT NULL DEFAULT 0," \
                "HoursAddedDate DATE DEFAULT NULL," \
                "ClockInTime DATETIME DEFAULT NULL," \
                "ClockOutTime DATETIME DEFAULT NULL," \
                "Timestamp DATETIME DEFAULT (DATETIME('now', 'localtime'))" \
            "); " \
            "CREATE INDEX HoursAdded_Index ON TimeSheet (HoursAddedDate);" \
            "CREATE INDEX ClockIn_Index ON TimeSheet (ClockInTime);" \
            "CREATE INDEX ClockOut_Index ON TimeSheet (ClockOutTime);";

    rc = sqlite3_exec(db, sql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        return TIMESLIME_SQLITE_ERROR;
    }
    return TIMESLIME_OK;
}
