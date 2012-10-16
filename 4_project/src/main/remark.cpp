#include "run.h"

void RemarkAction(const char * set) {
    Line outdir("bigdat/s/%s", set);
    mkdirs(outdir());

    Line in("bigdat/timeline/%s", set);
    Line out("bigdat/s/%s/timeline", set);
    printf("[remarkAction] %s -> %s \n", in(), out());
    remarkAction(in(), out());
}
