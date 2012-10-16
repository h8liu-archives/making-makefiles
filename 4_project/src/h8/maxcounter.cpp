#include "h8.h"

void MaxCounter::count(uint64_t x, uint64_t y) {
    auto it = dat.find(x);
    if (it == dat.end())
        dat[x] = y;
    else if (it->second < y)
        it->second = y;
}


void MaxCounter::fline(FILE * out) {
    fprintf(out, "%ld\n", dat.size());
    for (auto it : dat)
        fprintf(out, "%ld %ld\n", it.first, it.second);
}
