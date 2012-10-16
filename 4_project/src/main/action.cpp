#include "run.h"

void CountActionRate(const char * set) {
    mkdirs("bigdat/actionrate");
    Line in("bigdat/timeline/%s", set);
    Line out("bigdat/actionrate/%s", set);
    printf("countActionRate : %s -> %s\n", in(), out());
    countActionRate(in(), out(), uint64_t(1e6));
}

void CountActionRate(const char * set, const char * sub) {
    Line outDir("bigdat/%s/actionrate", sub);
    mkdirs(outDir());

    Line in("bigdat/%s/%s", sub, set);
    Line out("bigdat/%s/actionrate/%s", sub, set);
    printf("countActionRate : %s -> %s\n", in(), out());
    countActionRate(in(), out(), uint64_t(1e6));
}

void ActionCDF(const char * set) {
    o("cdfActionRate", set, cdfActionRate,
            "bigdat/actionrate", "dat/actioncdf");
}
void ActionCDF(const char * set, const char * sub) {
    Line in("bigdat/%s/actionrate", sub);
    Line out("dat/actioncdf/%s", sub);
    o("cdfActionRate", set, cdfActionRate, in(), out());
}
