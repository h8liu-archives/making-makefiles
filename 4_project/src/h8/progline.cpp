#include "h8.h"

static inline uint64_t timeNow() { return uint64_t(time(NULL)); }
static inline void freport(FILE * out, uint64_t i, uint64_t n) {
    fprintf(out, "%ld of %ld, %.3f%%", i, n, 
            double(i) / n * 100);
}

static void felapse(FILE * out, uint64_t i, uint64_t n, uint64_t el) {
    DurationStr str;

    fprintf(out, "%s used", str(el));
    if (i < n && el > 3 && i > 0) {
        uint64_t left = uint64_t(float(el) * (n - i) / i);
        fprintf(out, ", %s left", str(left));
    }
}

static void lineBack(FILE * term) {
    fprintf(term, "\x1b[1A\x1b[1G\x1b[2K");
}

ProgLine::ProgLine(bool startNow) {
    lastReport = begin = 0;
    term = stdout;
    sameLine = true;
    if (startNow) start();
}

void ProgLine::start() {
    begin = timeNow();
    if (sameLine) fprintf(term, "\n");
}

void ProgLine::count(uint64_t i, uint64_t n) {
    auto now = timeNow();
    if (i == n || now > lastReport) reportAt(i, n, now);
}

void ProgLine::report(uint64_t i, uint64_t n) {
    reportAt(i, n, timeNow());
}

void ProgLine::reportAt(uint64_t i, uint64_t n, uint64_t t) {
    if (sameLine) lineBack(term);
    freport(term, i, n);
    fprintf(term, "   -   ");
    felapse(term, i, n, t - begin);
    fprintf(term, "\n");
    lastReport = t;
}
