#include "h8.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

uint64_t fsize(const char * fname) {
    struct stat s;
    auto ret = stat(fname, &s);
    panicIf(ret != 0, "file stat error, file exists?");
    return uint64_t(s.st_size);
}

uint64_t fsize(FILE * f) {
    auto hold = ftell(f);
    assert(f);

    auto ret = fseek(f, 0, SEEK_END);
    assert(ret == 0);
    auto offset = ftell(f);
    assert(offset >= 0);

    // set back the cursor;
    ret = fseek(f, hold, SEEK_SET);
    assert(ret == 0);

    return uint64_t(offset);
}
