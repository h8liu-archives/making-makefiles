#include "run.h"

void CountUserActionRate(const char * set) {
    mkdirs("bigdat/actionrate");

    Line in("bigdat/timeline/%s", set);
    Line out("bigdat/useractionrate/%s", set);
    printf("countUserActionRate : %s -> %s\n", in(), out());
    countUserActionRate(in(), out(), uint64_t(1e6));
}

void CountUserActionRate(const char * set, const char * sub) {
    Line outDir("bigdat/%s/useractionrate", sub);
    mkdirs(outDir());

    Line in("bigdat/%s/%s", sub, set);
    Line out("bigdat/%s/useractionrate/%s", sub, set);
    printf("countUserActionRate : %s -> %s\n", in(), out());
    countUserActionRate(in(), out(), uint64_t(1e6));
}

void CountUserTimeUtilization(const char * set) {
    o("countUserTimeUtilization", set, countUserTimeUtilization,
            "bigdat/useractionrate", "dat/usertimeutil");
}
void ComputeUserActionCorrelation(const char * set) {
    o("computeUserActionCorrelation", set, computeUserActionCorrelation,
            "bigdat/useractionrate", "bigdat/usercorr");
}

void NormalizeUserActionCorrelation(const char * set) {
    o("normalizeUserActionCorrelation", set, normalizeUserActionCorrelation,
            "bigdat/usercorr", "bigdat/normusercorr");
}

void PrintUserActionCorrelation(const char * set) {
    Line in("bigdat/normusercorr/%s", set);
    printf("[%s]\n", set);
    printUserActionCorrelation(in());
}
