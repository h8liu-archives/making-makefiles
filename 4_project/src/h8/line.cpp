#include "h8.h"

#include <cstdarg>

Line::Line() {
    buf[0] = '\0';
}

Line::Line(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, LINE_MAX, fmt, args);
    va_end(args);
}

char * Line::format(const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, LINE_MAX, fmt, args);
    va_end(args);
    return buf;
}
