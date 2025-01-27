#include "args.h"
#include "../string_literals.h"
#include "../logger.h"


args_t args_parse(int argc, char **argv)
{
    // Copy the arguments
    char **args = NULL;
    args = (char**)malloc(sizeof(char*) * (argc + 1));
    int i;
    for (i = 1; i < argc; i++)
        args[i - 1] = argv[i];

    int num_args = argc - 1; // Adjust because the filename was dropped

    // Create parsed args result and set defaults
    args_t result;
    result.help = ARGS_False;
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
            result.help = ARGS_True;
            return result; // Nothing else matters
        }

        if (num_args > 1)
            result.modifier1 = args[1];
        if (num_args > 2)
            result.modifier2 = args[2];
        if (num_args > 3)
            result.modifier3 = args[3];
    }

    free(args);

    return result;
}

// Sections
#define YEAR    0
#define MONTH   1
#define DAY     2

/* Parse a date */
date_t args_parse_date(char *dateStr)
{
    log_debug("Parsing Date: %s", dateStr);

    date_t date;
    date.error = ARGS_False;
    date.month = 0;
    date.day = 0;
    date.year = 0;
    strcpy(date.str, dateStr);

    if (strcmp(dateStr, TODAY) == 0)
        return date;

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
                    date.error = ARGS_True;
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
                    date.error = ARGS_True;
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
                    date.error = ARGS_True;
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


char* args_get_directory_of_executable(char *name)
{
    char passed_path[PATH_MAX];
#ifndef _WIN32
    if (realpath("/proc/self/exe", passed_path) == NULL)
    {
        printf("realpath error: %s\n", strerror(errno));
        return NULL;
    }
#else
    strcpy(passed_path, name);
#endif

    char path[PATH_MAX];
    path[0] = '\0';

    char read_since_last_separator[PATH_MAX];
#ifdef _WIN32
    read_since_last_separator[0] = '\0';
#else
    read_since_last_separator[0] = '/';
    read_since_last_separator[1] = '\0';
#endif
    char separator;

    int i;
    int pos;
    int len = strlen(passed_path);
    for (i = 0; i < len; i++)
    {
        if (passed_path[i] == '/' || passed_path[i] == '\\')
        {
            separator = passed_path[i];
            if (strlen(path) > 1)
            {
                // Append the separator
                pos = strlen(path);
                path[pos] = separator;
                path[pos + 1] = '\0';
            }

            // Append everything read since last separator
            if (strlen(read_since_last_separator) > 0)
            {
                strcat(path, read_since_last_separator);
                read_since_last_separator[0] = '\0';
            }
        }
        else {
            // Add to the temp string until we reach the next separator
            pos = strlen(read_since_last_separator);
            read_since_last_separator[pos] = passed_path[i];
            read_since_last_separator[pos + 1] = '\0';
        }
    }

    // Return a pointer to the result
    char *result = malloc(sizeof(path));
    if (result == NULL)
        return NULL;

    strcpy(result, path);

    return result;
}