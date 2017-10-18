#include "timer.h"
#define _POSIX_C_SOURCE 199309L
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

uint64 current_time_ms()
{
    uint64          ms = 0; // Milliseconds
    //time_t          s;  // Seconds

#if _LINUX_
#define USE_CLOCK_GETTIME
#endif // _LINUX_

#ifdef USE_CLOCK_GETTIME
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    ms  = spec.tv_sec*1000;
    ms += round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
#else
    //time_t t = time(0x0);
    ms = (clock() / (CLOCKS_PER_SEC / 1000));
#endif // USE_CLOCK_GETTIME

    return ms;
}
