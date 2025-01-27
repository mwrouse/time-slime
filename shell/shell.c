/**
 * Wrapper around the timeslime library to allow use from the command line
 */
#include "../timeslime.h"
#include "logger.h"
#include "args/args.h"
#include "string_literals.h"
#ifndef _WIN32
#include <limits.h>
#endif

static TIMESLIME_STATUS_t status;

static void perform_add_action(args_t args);
static void perform_clock_action(args_t args);
static void perform_report_action(args_t args);
static void display_help(void);

/**
 * Entry point to the command line interface for Time Slime
 */
int main(int argc, char *argv[])
{

    log_dull("==== Time Slime ====\n")

    // Parse command line arguments
    args_t parsed_args;
    parsed_args = args_parse(argc, argv);


    if (parsed_args.help)
    {
        display_help();
    }
    else {
        // Initialize the TimeSlime library with path for database file
        char *base_folder = args_get_directory_of_executable(argv[0]);
        if (base_folder == NULL)
        {
            printf("Unknown fatal error\n");
            return 0;
        }

        status = TimeSlime_Initialize(base_folder);
        if (status != TIMESLIME_OK)
        {
            printf("An error occured: %d => %s\n", status, TimeSlime_StatusCode(status));
            return -1;
        }

        if (parsed_args.action != NULL && strcmp(parsed_args.action, ADD_ACTION) == 0)
            perform_add_action(parsed_args);

        else if (parsed_args.action != NULL && strcmp(parsed_args.action, CLOCK_ACTION) == 0)
            perform_clock_action(parsed_args);

        else if (parsed_args.action != NULL && strcmp(parsed_args.action, REPORT_ACTION) == 0)
            perform_report_action(parsed_args);

        if (status != TIMESLIME_OK)
            printf("Error: %s\n", TimeSlime_StatusCode(status));

        TimeSlime_Close();
        free(base_folder);
        base_folder = NULL;
    }

    return 0;
}


/**
 * Add to the time sheet
 */
static void perform_add_action(args_t args)
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

    if (strcmp(date, TODAY) == 0)
    {
        status = TimeSlime_AddHours(toAdd, TIMESLIME_DATE_NOW);
    }
    else {
        date_t parsed = args_parse_date(date);
        if (!parsed.error)
        {
            status = TimeSlime_AddHours(toAdd, (TIMESLIME_DATE_t){ parsed.year, parsed.month, parsed.day });
        }
    }

}

/* Clock in and clock out of the time sheet */
static void perform_clock_action(args_t args)
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

    if (strcmp(direction, CLOCK_IN) == 0)
    {
        status = TimeSlime_ClockIn(TIMESLIME_TIME_NOW);
    }
    else {
        status = TimeSlime_ClockOut(TIMESLIME_TIME_NOW);
    }
}

/* Show all time worked */
static void perform_report_action(args_t args)
{
    if (args.modifier1 == NULL)
    {
        log_error("'report' action needs another paramter, the start date");
        return;
    }

    if (args.modifier2 == NULL)
    {
        args.modifier2 = TODAY;
        //log_error("'report' action needs another parameter, the end date");
        //return;
    }

    date_t startDate = args_parse_date(args.modifier1);
    if (startDate.error)
        return;

    date_t endDate = args_parse_date(args.modifier2);
    if (endDate.error)
        return;

    // Verify that dates are in the correct order
    if (strcmp(startDate.str, TODAY) != 0 && strcmp(endDate.str, TODAY) != 0)
    {
        if (
                (endDate.month < startDate.month && endDate.year == startDate.year) || // Same year, but earlier month
                (endDate.day < startDate.day && endDate.year == startDate.year && endDate.month == startDate.month) || // Same month and year, but earlier day
                (endDate.year < startDate.year) // Earlier year
            )
        {
            log_error("Dates in wrong order");
            return;
        }
    }

    TIMESLIME_REPORT_t *report;
    TIMESLIME_DATE_t start = { startDate.year, startDate.month, startDate.day };
    TIMESLIME_DATE_t end = { endDate.year, endDate.month, endDate.day };
    status = TimeSlime_GetReport(start, end, &report);

    log_dull("Time Slime report for %s to %s:", startDate.str, endDate.str);

    int i;
    for (i = 0; i < report->NumberOfEntries; i++)
    {
        printf("\t%s: %0.2f\n", report->Entries[i].Date, report->Entries[i].Hours);
    }

    printf("\nTotal Hours: %0.2f\n", report->TotalHours);
    printf("\n");
    TimeSlime_FreeReport(&report);
}


/* Help Screen */
void display_help(void)
{
    printf("Author: %s\n", AUTHOR);
    printf("Version: %s\n", TIMESLIME_VERSION_STR);
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
