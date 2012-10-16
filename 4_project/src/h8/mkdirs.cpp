#include "h8.h"

void mkdirs(const char * dir) {
    Line cmdLine("mkdir -p %s", dir);
    panicIf(system(cmdLine()), "mkdir");
}
