/**
 * Time Slime - Command Line Time Sheet
 *
 * Authors: Michael Rouse
 */
#include "timeslime.h"
#include "third_party/sqlite3/sqlite3.h"

/* Row result for internal library use */
struct TIMESLIME_INT_ROW_STRUCT {
    int ID;
    float HoursAdded;
    char HoursAddedDate[TIMESLIME_DATETIME_STR_LENGTH];
    char ClockInTime[TIMESLIME_DATETIME_STR_LENGTH];
    char ClockOutTime[TIMESLIME_DATETIME_STR_LENGTH];
};
typedef struct TIMESLIME_INT_ROW_STRUCT TIMESLIME_INTERNAL_ROW_t;


/* Variables */
static sqlite3 *db;
static char *database_file_path;
static TIMESLIME_INTERNAL_ROW_t **database_results;
static int number_of_results;
static int result_array_size;


/* Functions */
static TIMESLIME_STATUS_t _TimeSlime_CreateTables(void);
static TIMESLIME_STATUS_t _TimeSlime_InsertEntry(TIMESLIME_INTERNAL_ROW_t *row);
static TIMESLIME_STATUS_t _TimeSlime_UpdateEntry(TIMESLIME_INTERNAL_ROW_t *row);
static TIMESLIME_STATUS_t _TimeSlime_SelectEntries(int minID, char *whereClause);
static TIMESLIME_STATUS_t _TimeSlime_VerifyTimestamp(TIMESLIME_DATETIME_t time);
static TIMESLIME_STATUS_t _TimeSlime_VerifyDate(TIMESLIME_DATE_t date);

static int _TIMESLIME_SQLITE_CALLBACK_WRAPPER(void *ignoreMe, int numColumns, char **columns, char **columnNames);



/**
 * Initializes the Time Slime library
 */
TIMESLIME_STATUS_t TimeSlime_Initialize(char directory_for_database[])
{
    database_file_path = NULL;
    db = NULL;
    database_results = NULL;

    // Generate path for database file
    database_file_path = malloc((strlen(directory_for_database) + 1 + strlen(TIMESLIME_DATABASE_FILE_NAME)) * sizeof(char)); /* + 1 for the slash */
    if (database_file_path == NULL)
        return TIMESLIME_UNKOWN_ERROR;
    sprintf(database_file_path, "%s\\%s", directory_for_database, TIMESLIME_DATABASE_FILE_NAME); // Append the file name

    // Create database if it doesn't exist
    int rc;
    rc = sqlite3_open(database_file_path, &db);
    if (rc != SQLITE_OK)
    {
        return TIMESLIME_SQLITE_ERROR;
    }

    // Initialize the results array
    database_results = malloc(TIMESLIME_DEFAULT_RESULT_LIMIT * sizeof(TIMESLIME_INTERNAL_ROW_t*));
    if (database_results == NULL)
        return TIMESLIME_UNKOWN_ERROR;

    result_array_size = TIMESLIME_DEFAULT_RESULT_LIMIT;
    int i;
    for (i = 0; i < result_array_size; i++)
        database_results[i] = NULL;


    return _TimeSlime_CreateTables();
}

/**
 * Safely close the Time Slime library
 */
TIMESLIME_STATUS_t TimeSlime_Close(void)
{
    int rc;
    if (db != NULL)
    {
        rc = sqlite3_close(db);
        if (rc != SQLITE_OK)
        {
            printf("SQLITE CLOSING ERROR: %d\n", rc);
        }
    }

    if (database_file_path != NULL)
    {
        free(database_file_path);
        database_file_path = NULL;
    }

    if (database_results != NULL)
    {
        int i;
        for (i = 0; i < number_of_results; i++)
        {
            free(database_results[i]);
            database_results[i] = NULL;
        }

        free(database_results);
        database_results = NULL;
    }

    return TIMESLIME_OK;
}

/**
 *  Add a set amount of hours to the Time Slime time sheet
 */
TIMESLIME_STATUS_t TimeSlime_AddHours(float hours, TIMESLIME_DATE_t date)
{
    // Verify parameters
    TIMESLIME_STATUS_t paramTest = _TimeSlime_VerifyDate(date);
    if (paramTest != TIMESLIME_OK)
        return paramTest;

    // Create new row to be inserted
    TIMESLIME_INTERNAL_ROW_t entry;
    entry.HoursAdded = hours;
    strcpy(entry.ClockInTime, "NULL");
    strcpy(entry.ClockOutTime, "NULL");

    if ((date.year == 0) || (date.month == 0) || (date.day == 0))
    {
        // Use current database date
        strcpy(entry.HoursAddedDate, "DATE('now', 'localtime')");
    }
    else
    {
        // Use user-specified date
        sprintf(entry.HoursAddedDate, "strftime('%d-%d-%d')", date.year, date.month, date.day);
    }

    return _TimeSlime_InsertEntry(&entry);
}

/**
 *  Clock in to the Time Slime time sheet
 */
TIMESLIME_STATUS_t TimeSlime_ClockIn(TIMESLIME_DATETIME_t time)
{
    TIMESLIME_STATUS_t status;
    // Verify parameters are valid
    status = _TimeSlime_VerifyTimestamp(time);
    if (status != TIMESLIME_OK)
        return status;

    // Check if already clocked in
    _TimeSlime_SelectEntries(0, "ClockOutTime IS NULL AND ClockInTime IS NOT NULL");
    if (number_of_results > 0)
        return TIMESLIME_ALREADY_CLOCKED_IN;

    // Create new row to be inserted
    TIMESLIME_INTERNAL_ROW_t entry;
    entry.HoursAdded = 0;
    strcpy(entry.HoursAddedDate, "NULL");
    strcpy(entry.ClockOutTime, "NULL");

    if ((time.year == 0) || (time.month == 0) || (time.day == 0) || (time.hour == 0) || (time.minute == 0))
    {
        // Use current database date
        strcpy(entry.ClockInTime, "DATETIME('now', 'localtime')");
    }
    else
    {
        // Use user-specified date
        sprintf(entry.ClockInTime, "strftime('%d-%d-%d %d:%d:0')", time.year, time.month, time.day, time.hour, time.minute);
    }

    return _TimeSlime_InsertEntry(&entry);
}

/**
 *  Clock out of the Time Slime time sheet
 */
TIMESLIME_STATUS_t TimeSlime_ClockOut(TIMESLIME_DATETIME_t time)
{
     // Verify parameters are valid
    TIMESLIME_STATUS_t paramTest = _TimeSlime_VerifyTimestamp(time);
    if (paramTest != TIMESLIME_OK)
        return paramTest;

    // Check if already clocked in
    _TimeSlime_SelectEntries(0, "ClockOutTime IS NULL AND ClockInTime IS NOT NULL");
    if (number_of_results == 0 )
        return TIMESLIME_NOT_CLOCKED_IN;

    int id = number_of_results - 1; // Last row item ID

    // Prep for update
    char tmp[TIMESLIME_DATETIME_STR_LENGTH];
    strcpy(tmp, database_results[id]->ClockInTime);
    sprintf(database_results[id]->ClockInTime, "strftime('%s')", tmp);

    if ((time.year == 0) || (time.month == 0) || (time.day == 0) || (time.hour == 0) || (time.minute == 0))
    {
        // Use current database date
        strcpy(database_results[id]->ClockOutTime, "DATETIME('now', 'localtime')");
    }
    else
    {
        // Use user-specified date
        sprintf(database_results[id]->ClockOutTime, "strftime('%d-%d-%d %d:%d:0')", time.year, time.month, time.day, time.hour, time.minute);
    }

    return _TimeSlime_UpdateEntry(database_results[id]);
}

/**
 *  Gets the time sheet for a period of time
 */
TIMESLIME_STATUS_t TimeSlime_GetTimeSheet(TIMESLIME_DATE_t start, TIMESLIME_DATE_t end)
{
    return TIMESLIME_OK;
}

/**
 * Converts status to friendly status code (or returns SQLITE error string)
 */
char*  TimeSlime_StatusCode(TIMESLIME_STATUS_t status)
{
    switch (status)
    {
        case TIMESLIME_OK:
            return "K";
        case TIMESLIME_UNKOWN_ERROR:
            return "UKNOWN";
        case TIMESLIME_INVALID_YEAR:
            return "INVALID_YEAR";
        case TIMESLIME_INVALID_MONTH:
            return "INVALID_MONTH";
        case TIMESLIME_INVALID_DAY:
            return "INVALID_DAY";
        case TIMESLIME_INVALID_HOUR:
            return "INVALID_HOUR";
        case TIMESLIME_INVALID_MINUTE:
            return "INVALID_MINUTE";
        case TIMESLIME_INVALID_TIMESTAMP:
            return "INVALID_TIMESTAMP";
        case TIMESLIME_INVALID_DATE:
            return "INVALID_DATE";
        case TIMESLIME_ALREADY_CLOCKED_IN:
            return "ALREADY_CLOCKED_IN";
        case TIMESLIME_NOT_CLOCKED_IN:
            return "NOT_CLOCKED_IN";
        case TIMESLIME_SQLITE_ERROR:
            return (char*)sqlite3_errmsg(db);
        default:
            return "?";
    }

}



/**
 * Creates the SQL tables (only if the file is new)
 */
static TIMESLIME_STATUS_t _TimeSlime_CreateTables(void)
{
    char *errMsg = 0;
    int rc;
    char *sql;

    // Create time sheet table
    sql =   "CREATE TABLE IF NOT EXISTS TimeSheet(" \
                "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
                "HoursAdded REAL NOT NULL DEFAULT 0," \
                "HoursAddedDate DATE DEFAULT NULL," \
                "ClockInTime DATETIME DEFAULT NULL," \
                "ClockOutTime DATETIME DEFAULT NULL," \
                "CreationTime DATETIME DEFAULT (DATETIME('now', 'localtime')), " \
                "LastUpdateTime DATETIME DEFAULT (DATETIME('now', 'localtime'))" \
            "); " \
            "CREATE INDEX IF NOT EXISTS HoursAdded_Index ON TimeSheet (HoursAddedDate);" \
            "CREATE INDEX IF NOT EXISTS ClockIn_Index ON TimeSheet (ClockInTime);" \
            "CREATE INDEX IF NOT EXISTS ClockOut_Index ON TimeSheet (ClockOutTime);";

    rc = sqlite3_exec(db, sql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        return TIMESLIME_SQLITE_ERROR;
    }

    return TIMESLIME_OK;
}

/**
 * Inserts an entry into the Time Slime time sheet
 */
static TIMESLIME_STATUS_t _TimeSlime_InsertEntry(TIMESLIME_INTERNAL_ROW_t *row)
{
    char *errMsg;
    int rc;

    // Generate the SQL query
    char sql[1000];
    sprintf(sql,    "INSERT INTO TimeSheet " \
                    "(HoursAdded, HoursAddedDate, ClockInTime, ClockOutTime) " \
                    "VALUES " \
                    "(%.2f, %s, %s, %s)",
        row->HoursAdded,
        row->HoursAddedDate,
        row->ClockInTime,
        row->ClockOutTime
    );

    rc = sqlite3_exec(db, sql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("\nERROR: %s\n", errMsg);
        return TIMESLIME_SQLITE_ERROR;
    }

    return TIMESLIME_OK;
}

/**
 * Updates an entry in the Time Slime time sheet
 */
static TIMESLIME_STATUS_t _TimeSlime_UpdateEntry(TIMESLIME_INTERNAL_ROW_t *row)
{
    char *errMsg;
    int rc;

    // Generate the SQL query
    char sql[1000];
    sprintf(sql,    "UPDATE TimeSheet " \
                    "SET HoursAdded=%.2f, HoursAddedDate=%s, ClockInTime=%s, ClockOutTime=%s, LastUpdateTime=DATETIME('now', 'localtime') " \
                    "WHERE ID = %d",
        row->HoursAdded,
        row->HoursAddedDate,
        row->ClockInTime,
        row->ClockOutTime,
        row->ID
    );

    rc = sqlite3_exec(db, sql, NULL, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        printf("\nERROR: %s\n", errMsg);
        return TIMESLIME_SQLITE_ERROR;
    }

    return TIMESLIME_OK;
}

/**
 * Selects data from the database
 */
static TIMESLIME_STATUS_t _TimeSlime_SelectEntries(int minID, char *whereClause)
{
    number_of_results = 0;

    char sql[1000];
    sprintf(sql,    "SELECT " \
                    "ID, HoursAdded, HoursAddedDate, ClockInTime, ClockOutTime " \
                    "FROM TimeSheet " \
                    "WHERE ID > %d AND %s " \
                    "ORDER BY ID ASC",
                minID,
                (whereClause == NULL) ? "1=1" : whereClause
            );

    char *errMsg;
    if (sqlite3_exec(db, sql, _TIMESLIME_SQLITE_CALLBACK_WRAPPER, 0, &errMsg) != SQLITE_OK)
    {
        printf("ERROR: %s", errMsg);
    }

    return TIMESLIME_OK;
}


/**
 * Parses the return result from queries
 */
static int _TIMESLIME_SQLITE_CALLBACK_WRAPPER(void *ignoreMe, int numColumns, char **columns, char **columnNames)
{
    int i = number_of_results; // Current index

    if (database_results[i] == NULL)
        database_results[i] = malloc(sizeof(TIMESLIME_INTERNAL_ROW_t));

    database_results[i]->ID = atoi(columns[0]);
    database_results[i]->HoursAdded = atof(columns[1] ? columns[1] : "0");
    strcpy(database_results[i]->HoursAddedDate, columns[2] ? columns[2] : "NULL");
    strcpy(database_results[i]->ClockInTime, columns[3] ? columns[3] : "NULL");
    strcpy(database_results[i]->ClockOutTime,columns[4] ? columns[4] : "NULL");

    number_of_results++;

    // Increase results array if needed
    if (number_of_results >= result_array_size)
    {
        int old_size = result_array_size;
        result_array_size = result_array_size * 2.5;
        TIMESLIME_INTERNAL_ROW_t **new_results_pointer = realloc(database_results, result_array_size * sizeof(TIMESLIME_INTERNAL_ROW_t*));
        if (new_results_pointer != NULL)
        {
            database_results = new_results_pointer;

            for (i = old_size; i < result_array_size; i++)
                database_results[i] = NULL;
        }
    }

    return 0;
}


/**
 * Used for verifying function parameters
 */
static TIMESLIME_STATUS_t _TimeSlime_VerifyTimestamp(TIMESLIME_DATETIME_t time)
{
    TIMESLIME_STATUS_t dateResult = _TimeSlime_VerifyDate((TIMESLIME_DATE_t) { time.year, time.month, time.day });
    if (dateResult != TIMESLIME_OK)
        return dateResult;

    if (time.hour < 0 || time.hour > 24)
        return TIMESLIME_INVALID_HOUR;

    if (time.minute < 0 || time.minute > 60)
        return TIMESLIME_INVALID_MINUTE;

    return TIMESLIME_OK;
}

/**
 * Used for verifying function parmameters
 */
static TIMESLIME_STATUS_t _TimeSlime_VerifyDate(TIMESLIME_DATE_t date)
{
    if (date.year < 0 || date.year > 9999)
        return TIMESLIME_INVALID_YEAR;
    if (date.month < 0 || date.month > 12)
        return TIMESLIME_INVALID_MONTH;
    if (date.day < 0 || date.day > 31)
        return TIMESLIME_INVALID_DAY;

    return TIMESLIME_OK;
}
