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
    //TimeSlime_ClockIn(TIMESLIME_CLOCK_IN_NOW);
    //TimeSlime_ClockOut(TIMESLIME_CLOCK_OUT_NOW);

    TimeSlime_Close();
    return 0;
}