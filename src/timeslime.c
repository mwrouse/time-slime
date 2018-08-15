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
    log_dull("==== Time Slime ====\n");

    timeslime_args parsed_args;
    parsed_args = parse_args(argc, argv);

    if (parsed_args.help)
        printf("Helping\n");
    else
    {
        if (strcmp(ADD_ACTION, parsed_args.action) == 0)
            perform_add_action(parsed_args);
    }



    //double x = atof("-4");
    //printf("%f\n", x);
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
        printf("ERROR: 'add' action needs at least a second parameter.\n");
        return;
    }

    // Parse argument to determine how long to add
    float toAdd;
    toAdd = atof(args.modifier1);

    // Get the date to add onto the time sheet
    char *date = TODAY;
    if (args.modifier2 != NULL)
        date = args.modifier2;

    // Show message for what is about to happen
    log_info("Adding %.2f hours to the time sheet for %s. \n", toAdd, date);
}



