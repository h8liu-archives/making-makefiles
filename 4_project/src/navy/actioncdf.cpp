#include "navy.h"

#include <algorithm>

using std::vector;
using std::sort;
using std::max_element;
using std::min_element;

static inline uint64_t idelta(uint64_t a, uint64_t b) {
    if (a > b) return a - b;
    return b - a;
}

void cdfActionRate(const char * countIn, const char * cdfOut) {
    vector<uint64_t> coldreads;
    vector<uint64_t> warmreads;
    vector<uint64_t> coldwrites;
    vector<uint64_t> warmwrites;
    vector<uint64_t> coldactions;
    vector<uint64_t> allwrites;
    vector<uint64_t> allactions;

    Reader reader(countIn, sizeof(ActionRate));

    // load the file into memory
    ActionRate rate;
    while (reader.read(&rate)) {
        coldreads.push_back(rate.counts[COLD_READ]);
        warmreads.push_back(rate.counts[WARM_READ]);
        coldwrites.push_back(rate.counts[COLD_WRITE]);
        warmwrites.push_back(rate.counts[WARM_WRITE]);
        coldactions.push_back(rate.counts[COLD_READ] 
                + rate.counts[COLD_WRITE]);
        allwrites.push_back(rate.counts[COLD_WRITE] 
                + rate.counts[WARM_WRITE]);
        allactions.push_back(rate.counts[COLD_READ] 
                + rate.counts[COLD_WRITE]
                + rate.counts[WARM_READ]
                + rate.counts[WARM_WRITE]);
    }

    FILE * fout = fopen(cdfOut, "w");
    panicIf(!fout, "fopen for action cdf");
    
    fprintf(fout, "#coldreads\n");
    cdf(coldreads, fout);
    fprintf(fout, "#warmreads\n");
    cdf(warmreads, fout);
    fprintf(fout, "#coldwrites\n");
    cdf(coldwrites, fout);
    fprintf(fout, "#warmwrites\n");
    cdf(warmwrites, fout);
    fprintf(fout, "#coldactions\n");
    cdf(coldactions, fout);
    fprintf(fout, "#allwrites\n");
    cdf(allwrites, fout);
    fprintf(fout, "#allactions\n");
    cdf(allactions, fout);

    fclose(fout);
}

