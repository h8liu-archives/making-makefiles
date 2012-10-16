#include "run.h"

void BlockSort(const char * set) {
    o("blockSort", set, sortByBlock,
            "bigdat/timeline", "bigdat/blocksorted");
}

void UniformTime(const char * set) {
    mkdirs("bigdat/trand");
    mkdirs("bigdat/trand/tmp");
    Line pathin("bigdat/timeline/%s", set);
    Line pathout("bigdat/trand/%s", set);
    Line pathtmp("bigdat/trand/tmp/%s", set);
    printf("randTrace: %s -> %s -> %s\n", pathin(), pathtmp(), pathout());
    uniformTime(pathin(), pathtmp(), pathout());
}

void Retime(const char * set) {
    mkdirs("bigdat/trandshaped");
    Line pathin("bigdat/trand/%s", set);
    Line shapein("bigdat/timeline/%s", set);
    Line shapeout("bigdat/trandshaped/%s", set);
    printf("retime %s with %s -> %s\n", pathin(), shapein(), shapeout());
    retime(pathin(), shapein(), shapeout());
}
