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
        if (strcmp(HELP_ACTION, args[0]) == 0)
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

// Sections
#define YEAR    0
#define MONTH   1
#define DAY     2

/* Parse a date */
timeslime_date parse_date(char *dateStr)
{
    log_debug("Parsing Date: %s", dateStr);

    timeslime_date date;
    date.error = False;
    date.month = 0;
    date.day = 0;
    date.year = 0;
    strcpy(date.str, dateStr);

    char running[256] = "";
    int tmp = 0;
    int section = YEAR;
    int i;
    for (i = 0; i <= strlen(dateStr); i++)
    {
        // - and / are valid separators
        if (dateStr[i] == '/' || dateStr[i] == '-' || dateStr[i] == '\0')
        {
            log_debug("Found Date Separator: %c, read: %s", dateStr[i], running);
            tmp = atoi(running);

            if (section == YEAR)
            {
                if (tmp < 1000)
                {
                    log_error("Invalid year, %d; dates must be in the format YYYY/MM/DD", tmp);
                    date.error = True;
                    return date;
                }

                date.year = tmp;
                log_debug("Year: %d", date.year);
            }
            else if (section == MONTH)
            {
                if (tmp < 1 || tmp > 12)
                {
                    log_error("Invalid month, %d; dates must be in the format YYYY/MM/DD", tmp);
                    date.error = True;
                    return date;
                }

                date.month = tmp;
                log_debug("Month: %d", date.month);
            }
            else if (section == DAY)
            {
                if (tmp < 1 || tmp > 31)
                {
                    log_error("Invalid day, %d; dates must be in the format YYYY/MM/DD", tmp);
                    date.error = True;
                    return date;
                }

                date.day = tmp;
                log_debug("Day: %d", date.day);
                break;
            }

            section++;
            running[0] = '\0';
        }
        else {
            // Add to running string
            int len = strlen(running);
            running[len] = dateStr[i];
            running[len + 1] = '\0';
        }
    }

#ifdef DEBUG
    log_dull("%c", '\0');
#endif

    return date;
}