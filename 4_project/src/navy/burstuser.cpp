#include "navy.h"

static inline uint64_t naction(ActionRate & rate) {
    return rate.counts[COLD_READ] + rate.counts[COLD_WRITE]
        + rate.counts[WARM_READ] + rate.counts[WARM_WRITE];
}

void scatterBurstUsers(const char * in, const char * out) {
    Reader reader(in, sizeof(ActionRate));

    ActionRate rate;
    Scatter dat;
    ProgLine progline;

    while (reader.read(&rate)) {
        dat.count(rate.nusers, naction(rate));
        readerProg(reader, progline);
    }

    FILE * fout = fopen(out, "w");
    panicIf(!fout, "fopen in scatterBurstUsers");
    dat.freport(fout);
    fclose(fout);
}
