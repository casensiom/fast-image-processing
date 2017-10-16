#include "timer.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

uint64 current_time_ms()
{
    uint64          ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    ms  = spec.tv_sec*1000;
    ms += round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    return ms;
}
