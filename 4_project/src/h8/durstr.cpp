#include "h8.h"

const char * DurationStr::operator () (uint64_t t) {
    if (t == 0) {
        return "0s";
    }

    char * b = buf;
    auto sec = t % 60;
    auto min = t / 60 % 60;
    auto hour = t / 3600 % 24;
    auto day = t / 3600 / 24;

    if (day > 999)
        b += sprintf(b, "999+d");
    else if (day > 0) 
        b += sprintf(b, "%ldd", day);
    if (hour > 0)
        b += sprintf(b, "%ldh", hour);
    if (min > 0)
        b += sprintf(b, "%ldm", min);
    if (sec > 0)
        b += sprintf(b, "%lds", sec);
    
    return buf;
}
