/**
 * Time Slime - Command Line Time Sheet
 *
 * Authors: Michael Rouse
 */
#include "timeslime.h"


/**
 * Entry point for program, parsed arguments and
 * acts according to the arguments
 */
int main(int argc, char *argv[])
{
    log_dull("==== Time Slime ====\n")

    timeslime_args parsed_args;
    parsed_args = parse_args(argc, argv);

    if (parsed_args.help)
        printf("Helping\n");
    else
    {
        if (strcmp(parsed_args.action, ADD_ACTION) == 0)
            perform_add_action(parsed_args);

        else if (strcmp(parsed_args.action, CLOCK_ACTION) == 0)
            perform_clock_action(parsed_args);

        else if (strcmp(parsed_args.action, REPORT_ACTION) == 0)
            perform_report_action(parsed_args);
    }


    printf("\n");
    return 0;
}


/**
 * Add to the time sheet
 */
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


/* Clock in and clock out of the time sheet */
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

/* Show all time worked */
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