#include "h8.h"

void Counter::count(uint64_t x, uint64_t w) {
    if (UINT64_MAX - w < n) panic("too many counting");
    n += w;

    auto it = dat.find(x);
    if (it == dat.end())
        dat[x] = w;
    else
        it->second += w;
}

void Counter::fline(FILE * out) {
    fprintf(out, "%ld\n", dat.size());
    for (auto it : dat) 
        fprintf(out, "%ld %ld\n", it.first, it.second);
}

Counter::Counter() {
    clear();
}

