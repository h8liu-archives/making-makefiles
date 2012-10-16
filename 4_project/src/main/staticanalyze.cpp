#include "run.h"

void StaticAnalyze(const char * set) {
    Line outdir("bigdat/s/%s", set);
    printf("[staticAnalyze] %s\n", outdir());
    staticAnalyze(outdir());
}
