#include "navy.h"

static int compare(const void * p1, const void * p2)
{
    const Access & a1 = *(const Access *)(p1);
    const Access & a2 = *(const Access *)(p2);

    if (a1.block < a2.block) return -1;
    if (a1.block > a2.block) return 1;
    if (a1.time < a2.time) return -1;
    if (a1.time > a2.time) return 1;
    if (a1.user < a2.user) return -1;
    if (a1.user > a2.user) return 1;
    if (a1.action < a2.action) return -1;
    if (a1.action > a2.action) return 1;
    return 0;
}

void sortByBlock(const char * in, const char * out) {
    ExtSorter sorter(sizeof(Access), compare);
    sorter.sort(in, out);
}
