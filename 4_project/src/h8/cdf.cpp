#include "h8.h"
#include <algorithm>

using std::vector;

void cdf(vector<uint64_t> & dat, FILE * fout) {
    uint64_t n = uint64_t(dat.size());
    if (n == 0) {
        fprintf(fout, "0\n0\n"); // average and size
        return;
    }

    sort(dat.begin(), dat.end());
    uint64_t minv = *min_element(dat.begin(), dat.end());
    uint64_t maxv = *max_element(dat.begin(), dat.end());
    uint64_t sum = 0;
    
    uint64_t lasti = 0;
    uint64_t lastv = 0;

    uint64_t gridi = n / 1000; 
    if (gridi == 0) gridi = 1;

    uint64_t gridv = (maxv - minv) / 1000; 
    if (gridv == 0) gridv = 1;

    vector<uint64_t> xs;
    vector<float> ys;

    for (uint64_t i = 0; i < n; i++) {
        uint64_t v = dat[i];
        sum += v;
        if (i == 0 || i == n - 1 // always print first and last
             || (i - lasti >= gridi) 
             || (v - lastv >= gridv) ) {
            xs.push_back(v); ys.push_back(float(i) / n * 100);
            lasti = i; lastv = v;
        }
    }

    uint64_t avg = sum / uint64_t(dat.size());
    
    fprintf(fout, "%ld\n", avg);
    fprintf(fout, "%ld\n", xs.size());
    for (size_t i = 0; i < xs.size(); i++) {
        fprintf(fout, "%ld %.3f\n", xs[i], ys[i]);
    }
}
