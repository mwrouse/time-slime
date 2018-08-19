/**
 * Wrapper around the timeslime library to allow use from the command line
 */
#include "../timeslime.h"
#include "logger.h"
#include "args/args.h"


/**
 * Entry point to the command line interface for Time Slime
 */
int main(int argc, char *argv[])
{
    TimeSlime_Initialize("build");

    TimeSlime_AddHours(2.3, TIMESLIME_DATE_NOW);
    TimeSlime_ClockIn(TIMESLIME_CLOCK_IN_NOW);
    if (TimeSlime_ClockIn(TIMESLIME_CLOCK_IN_NOW) != TIMESLIME_OK)
        printf(":(\n");
    TimeSlime_ClockOut(TIMESLIME_CLOCK_OUT_NOW);

    TimeSlime_Close();
    return 0;
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