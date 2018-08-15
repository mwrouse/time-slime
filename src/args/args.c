#include "args.h"


timeslime_args parse_args(int argc, char **argv)
{
    // Copy the arguments
    char **args = NULL;
    args = (char**)malloc(sizeof(char*) * (argc + 1));
    int i;
    for (i = 1; i < argc; i++)
        args[i - 1] = argv[i];

    int num_args = argc - 1; // Adjust because the filename was dropped

    // Create parsed args result and set defaults
    timeslime_args result;
    result.help = False;
    result.error = 0;
    result.action = NULL;
    result.modifier1 = NULL;
    result.modifier2 = NULL;
    result.modifier3 = NULL;

    if (num_args > 0)
    {
        result.action = args[0];
        if (strcmp(HELP_ARG, args[0]) == 0)
        {
            result.help = True;
            return result; // Nothing else matters
        }

        if (num_args > 1)
            result.modifier1 = args[1];
        if (num_args > 2)
            result.modifier2 = args[2];
        if (num_args > 3)
            result.modifier3 = args[3];
    }


    return result;
}