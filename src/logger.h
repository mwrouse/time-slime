/**
 * Wrapper for logging
 */
#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdarg.h>
#include <stdio.h>


#define log_dull(format, args...) \
                printf(format, ## args); \
                printf("\n");

#define log_info(format, args...) \
                printf("INFO: ");   \
                printf(format, ## args); \
                printf("\n")

#define log_warning(format, args...) \
                printf("WARNING: "); \
                printf(format, ## args); \
                printf("\n");

#define log_error(format, args...) \
                printf("ERROR: "); \
                printf(format, ## args); \
                printf("\n");


#endif