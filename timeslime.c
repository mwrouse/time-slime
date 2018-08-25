/**
 * Time Slime - Command Line Time Sheet
 *
 * Authors: Michael Rouse
 */
#include "timeslime.h"

/* Row result for internal library use */
struct TIMESLIME_INT_ROW_STRUCT {
    int ID;
    float HoursAdded;
    float TotalHours;
    char HoursAddedDate[TIMESLIME_DATETIME_STR_LENGTH];
    char ClockInTime[TIMESLIME_DATETIME_STR_LENGTH];
    char ClockOutTime[TIMESLIME_DATETIME_STR_LENGTH];
    char TimeSheetDate[TIMESLIME_DATETIME_STR_LENGTH];
};
typedef struct TIMESLIME_INT_ROW_STRUCT TIMESLIME_INTERNAL_ROW_t;


/* Variables */
static sqlite3 *db;
static char *db_error;
static char *database_file_path;
static TIMESLIME_INTERNAL_ROW_t **database_results;
static int number_of_results;
static int result_array_size;
static int is_initialized;

/* Functions */
static TIMESLIME_STATUS_t _TimeSlime_CreateTables(void);
static TIMESLIME_STATUS_t _TimeSlime_InsertEntry(TIMESLIME_INTERNAL_ROW_t *row);
static TIMESLIME_STATUS_t _TimeSlime_UpdateEntry(TIMESLIME_INTERNAL_ROW_t *row);
static TIMESLIME_STATUS_t _TimeSlime_SelectEntries(int minID, char *whereClause);
static TIMESLIME_STATUS_t _TimeSlime_ExecuteQuery(char *sql, int useCallback);
static TIMESLIME_STATUS_t _TimeSlime_VerifyTimestamp(TIMESLIME_DATETIME_t time);
static TIMESLIME_STATUS_t _TimeSlime_VerifyDate(TIMESLIME_DATE_t date);

static int _TIMESLIME_SQLITE_CALLBACK_WRAPPER(void *ignoreMe, int numColumns, char **columns, char **columnNames);

static char* _TimeSlime_Time2Str(TIMESLIME_DATETIME_t time);
static char* _TimeSlime_Date2Str(TIMESLIME_DATE_t date);


/**
 * Initializes the Time Slime library
 */
TIMESLIME_STATUS_t TimeSlime_Initialize(char directory_for_database[])
{
    database_file_path = NULL;
    db = NULL;
    database_results = NULL;

    is_initialized = 1;

    // Generate path for database file
    database_file_path = malloc((strlen(directory_for_database) + 1 + strlen(TIMESLIME_DATABASE_FILE_NAME)) * sizeof(char)); /* + 1 for the slash */
    if (database_file_path == NULL)
        return TIMESLIME_UNKOWN_ERROR;

    sprintf(database_file_path, "%s%s%s", directory_for_database, TIMESLIME_FILE_PATH_SEPARATOR, TIMESLIME_DATABASE_FILE_NAME); // Append the file name
    // Create database if it doesn't exist
    int rc;
    rc = sqlite3_open(database_file_path, &db);
    if (rc != SQLITE_OK)
    {
        return TIMESLIME_SQLITE_ERROR;
    }

    // Initialize the results array
    database_results = (TIMESLIME_INTERNAL_ROW_t**)malloc(TIMESLIME_DEFAULT_RESULT_LIMIT * sizeof(TIMESLIME_INTERNAL_ROW_t*));
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
    if (!is_initialized)
        return TIMESLIME_NOT_INITIALIZED;

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
    if (!is_initialized)
        return TIMESLIME_NOT_INITIALIZED;

    // Verify parameters
    TIMESLIME_STATUS_t paramTest = _TimeSlime_VerifyDate(date);
    if (paramTest != TIMESLIME_OK)
        return paramTest;

    // Create new row to be inserted
    TIMESLIME_INTERNAL_ROW_t entry;
    entry.HoursAdded = hours;
    strcpy(entry.ClockInTime, "NULL");
    strcpy(entry.ClockOutTime, "NULL");

    char *dateStr = _TimeSlime_Date2Str(date);
    strcpy(entry.HoursAddedDate, dateStr);
    free(dateStr);

    return _TimeSlime_InsertEntry(&entry);
}

/**
 *  Clock in to the Time Slime time sheet
 */
TIMESLIME_STATUS_t TimeSlime_ClockIn(TIMESLIME_DATETIME_t time)
{
    if (!is_initialized)
        return TIMESLIME_NOT_INITIALIZED;

    TIMESLIME_STATUS_t status;
    // Verify parameters are valid
    status = _TimeSlime_VerifyTimestamp(time);
    if (status != TIMESLIME_OK)
        return status;

    // Check if already clocked in
    _TimeSlime_SelectEntries(0, __TS_QRY_GET_PARTIAL_CLOCK_ENTRIES);
    if (number_of_results > 0)
        return TIMESLIME_ALREADY_CLOCKED_IN;

    // Create new row to be inserted
    TIMESLIME_INTERNAL_ROW_t entry;
    entry.HoursAdded = 0;
    strcpy(entry.HoursAddedDate, "NULL");
    strcpy(entry.ClockOutTime, "NULL");

    char *timeStr = _TimeSlime_Time2Str(time);
    strcpy(entry.ClockInTime, timeStr);
    free(timeStr);

    return _TimeSlime_InsertEntry(&entry);
}

/**
 *  Clock out of the Time Slime time sheet
 */
TIMESLIME_STATUS_t TimeSlime_ClockOut(TIMESLIME_DATETIME_t time)
{
    if (!is_initialized)
        return TIMESLIME_NOT_INITIALIZED;

    // Verify parameters are valid
    TIMESLIME_STATUS_t paramTest = _TimeSlime_VerifyTimestamp(time);
    if (paramTest != TIMESLIME_OK)
        return paramTest;

    // Check if already clocked in
    _TimeSlime_SelectEntries(0, __TS_QRY_GET_PARTIAL_CLOCK_ENTRIES);
    if (number_of_results == 0 )
        return TIMESLIME_NOT_CLOCKED_IN;

    int id = number_of_results - 1; // Last row item ID

    // Prep for update
    char *timeStr = _TimeSlime_Time2Str(time);
    strcpy(database_results[id]->ClockOutTime, timeStr);
    free(timeStr);

    return _TimeSlime_UpdateEntry(database_results[id]);
}

/**
 *  Gets the time sheet for a period of time
 */
TIMESLIME_STATUS_t TimeSlime_GetReport(TIMESLIME_DATE_t start, TIMESLIME_DATE_t end, TIMESLIME_REPORT_t **out)
{
    if (!is_initialized)
        return TIMESLIME_NOT_INITIALIZED;

    int i;
    TIMESLIME_STATUS_t paramTest;
    char sql[1000];

    *out = NULL;

    // Test parameters and make sure they are valid dates
    paramTest = _TimeSlime_VerifyDate(start);
    if (paramTest != TIMESLIME_OK)
        return paramTest;

    paramTest = _TimeSlime_VerifyDate(end);
    if (paramTest != TIMESLIME_OK)
        return paramTest;

    char *startDate = _TimeSlime_Date2Str(start);
    char *endDate = _TimeSlime_Date2Str(end);

    // Get sum of all hours based on day in the date range
    // All we care about is the TotalHours and TimeSheetDate columns
    sprintf(sql, "SELECT " \
                    "ID, HoursAdded, HoursAddedDate, ClockInTime, ClockOutTime, " \
                    "SUM(case when HoursAdded <> 0.0 then HoursAdded else ((JULIANDAY(ClockOutTime) - JULIANDAY(ClockInTime)) * 24) end) AS TotalHours, " \
                    "case when HoursAddedDate IS NOT NULL then DATE(HoursAddedDate) else DATE(ClockInTime) end AS TimeSheetDate " \
                "FROM TimeSheet " \
                "WHERE (%s) AND (TimeSheetDate >= %s AND TimeSheetDate <= %s) " \
                "GROUP BY TimeSheetDate " \
                "ORDER BY TimeSheetDate ASC",
            __TS_QRY_GET_ALL_ENTIRES,
            startDate,
            endDate
        );

    free(startDate);
    startDate = NULL;
    free(endDate);
    endDate = NULL;

    number_of_results = 0;
    TIMESLIME_STATUS_t result = _TimeSlime_ExecuteQuery(sql, 1);
    if (result != TIMESLIME_OK)
        return result;

    // Generate report
    TIMESLIME_REPORT_t *report = (TIMESLIME_REPORT_t*)malloc(sizeof(TIMESLIME_REPORT_t) + (number_of_results * sizeof(TIMESLIME_REPORT_ENTRY_t)));
    if (report == NULL)
        return TIMESLIME_UNKOWN_ERROR;

    report->TotalHours = 0;
    report->NumberOfEntries = number_of_results;
    for (i = 0; i < number_of_results; i++)
    {
        // Build report entries
        report->Entries[i].Hours = database_results[i]->TotalHours;
        report->TotalHours += report->Entries[i].Hours;
        strcpy(report->Entries[i].Date, database_results[i]->TimeSheetDate);
    }

    *out = report;

    return TIMESLIME_OK;
}

/* Frees memory allocated for report used in report method */
void TimeSlime_FreeReport(TIMESLIME_REPORT_t **report)
{
    if (*report != NULL)
    {
        free(*report);
        *report = NULL;
    }
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
        case TIMESLIME_ALREADY_CLOCKED_IN:
            return "ALREADY_CLOCKED_IN";
        case TIMESLIME_NOT_CLOCKED_IN:
            return "NOT_CLOCKED_IN";
        case TIMESLIME_NO_ENTIRES:
            return "NO_TIMESHEET_ENTRIES";
        case TIMESLIME_NOT_INITIALIZED:
            return "NOT_INITIALIZED";
        case TIMESLIME_SQLITE_ERROR:
            return db_error;

        default:
            return "?";
    }

}







/**
 * Creates the SQL tables (only if the file is new)
 */
static TIMESLIME_STATUS_t _TimeSlime_CreateTables(void)
{
    // Create time sheet table
    char *sql =   "CREATE TABLE IF NOT EXISTS TimeSheet(" \
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

    return _TimeSlime_ExecuteQuery(sql, 0);
}

/**
 * Inserts an entry into the Time Slime time sheet
 */
static TIMESLIME_STATUS_t _TimeSlime_InsertEntry(TIMESLIME_INTERNAL_ROW_t *row)
{
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

    return _TimeSlime_ExecuteQuery(sql, 0);
}

/**
 * Updates an entry in the Time Slime time sheet
 */
static TIMESLIME_STATUS_t _TimeSlime_UpdateEntry(TIMESLIME_INTERNAL_ROW_t *row)
{
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

    return _TimeSlime_ExecuteQuery(sql, 0);
}

/**
 * Selects data from the database
 */
static TIMESLIME_STATUS_t _TimeSlime_SelectEntries(int minID, char *whereClause)
{
    number_of_results = 0;

    char sql[1000];
    sprintf(sql,    "SELECT " \
                    "ID, HoursAdded, HoursAddedDate, ClockInTime, ClockOutTime, " \
                    "case when HoursAdded <> 0.0 then HoursAdded else ((JULIANDAY(ClockOutTime) - JULIANDAY(ClockInTime)) * 24) end AS TotalHours, " \
                    "case when HoursAddedDate IS NOT NULL then date(HoursAddedDate) else date(ClockInTime) end AS TimeSheetDate " \
                    "FROM TimeSheet " \
                    "WHERE ID > %d AND %s " \
                    "ORDER BY ID ASC",
                minID,
                (whereClause == NULL) ? "1=1" : whereClause
            );

    return _TimeSlime_ExecuteQuery(sql, 1);
}

/**
 * Executes a SQLITE query
 */
static TIMESLIME_STATUS_t _TimeSlime_ExecuteQuery(char *sql, int useCallback)
{
    if (sqlite3_exec(db, sql, (useCallback) ? _TIMESLIME_SQLITE_CALLBACK_WRAPPER : NULL, 0, &db_error) != SQLITE_OK)
        return TIMESLIME_SQLITE_ERROR;

    return TIMESLIME_OK;
}


/**
 * Parses the return result from queries
 */
static int _TIMESLIME_SQLITE_CALLBACK_WRAPPER(void *ignoreMe, int numColumns, char **columns, char **columnNames)
{
    int i = number_of_results; // Current index

    if (database_results[i] == NULL)
        database_results[i] = (TIMESLIME_INTERNAL_ROW_t*)malloc(sizeof(TIMESLIME_INTERNAL_ROW_t));

    database_results[i]->ID = atoi(columns[0]);
    database_results[i]->HoursAdded = atof(columns[1] ? columns[1] : "0");
    database_results[i]->TotalHours = atof(columns[5] ? columns[5] : "0");
    strcpy(database_results[i]->HoursAddedDate, columns[2] ? columns[2] : "NULL");
    strcpy(database_results[i]->ClockInTime, columns[3] ? columns[3] : "NULL");
    strcpy(database_results[i]->ClockOutTime, columns[4] ? columns[4] : "NULL");
    strcpy(database_results[i]->TimeSheetDate, columns[6] ? columns[6] : "NULL");

    number_of_results++;

    // Increase results array if needed
    if (number_of_results >= result_array_size)
    {
        int old_size = result_array_size;
        result_array_size = result_array_size * 2.5;
        TIMESLIME_INTERNAL_ROW_t **new_results_pointer = (TIMESLIME_INTERNAL_ROW_t**)realloc(database_results, result_array_size * sizeof(TIMESLIME_INTERNAL_ROW_t*));
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

    if (time.hour < -1 || time.hour > 24)
        return TIMESLIME_INVALID_HOUR;

    if (time.minute < -1 || time.minute > 60)
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

/* Convert Structs to strings */
static char* _TimeSlime_Time2Str(TIMESLIME_DATETIME_t time)
{
    char *result = (char*)malloc(TIMESLIME_DATETIME_STR_LENGTH * sizeof(char));
    if (_TIMESLIME_IS_TIME_NOW(time))
        strcpy(result, "DATETIME('now', 'localtime')");
    else
        sprintf(result, "DATETIME('%04d-%02d-%02d %02d:%02d:0')", time.year, time.month, time.day, time.hour, time.minute);
    return result;
}

static char* _TimeSlime_Date2Str(TIMESLIME_DATE_t date)
{
    char *result = (char*)malloc(TIMESLIME_DATETIME_STR_LENGTH * sizeof(char));
    if (_TIMESLIME_IS_DATE_TODAY(date))
        strcpy(result, "DATE('now', 'localtime')");
    else
        sprintf(result, "DATE('%04d-%02d-%02d')", date.year, date.month, date.day);
    return result;
}
