#include "simu.h"

#include <algorithm>

static int icmp(const void * a, const void * b) {
    uint64_t x = *(const uint64_t *)(a);
    uint64_t y = *(const uint64_t *)(b);

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

static uint64_t readAt(FILE * f, size_t pos) {
    uint64_t ret;
    fseek(f, pos * sizeof(uint64_t), SEEK_SET);
    auto read = fread(&ret, sizeof(uint64_t), 1, f);
    assert(read == 1);
    return ret;
}

void cdfLatency(const char * flatencies, const char * out) {
    FILE * fin = fopen(flatencies, "rb");
    ExtSorter sorter(sizeof(uint64_t), icmp, 
            uint64_t(1.8 * 1024 * 1024 * 1024));
    FILE * ftmp = tmpfile();
    sorter.sort(fin, ftmp);

    size_t n = fsize(flatencies);
    assert(n % sizeof(uint64_t) == 0);
    n /= sizeof(uint64_t);
    double step = n / 100.l;

    FILE * fout = fopen(out, "w");
    assert(fout);

    fprintf(fout, "0 0 %ld\n", readAt(ftmp, 0));
    if (n == 0) return;

    for (size_t i = 1; i < 100; i++) {
        size_t index = size_t(i * step + 0.5);
        fprintf(fout, "%ld %ld %ld\n", i, index, readAt(ftmp, index));
    }
    fprintf(fout, "100 %ld %ld\n", n - 1, readAt(ftmp, n - 1));

    fclose(fout);
    fclose(ftmp);
}

