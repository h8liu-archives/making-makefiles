#include "run.h"

void CacheAge(const char * set) {
    o("cacheAge", set, cacheAge,
            "bigdat/timeline", "dat/cacheage");
}
