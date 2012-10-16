#include "simu.h"

static const size_t ONCE = uint64_t(1024 * 1024) / sizeof(Job);

Job * JobPool::alloc() {
    if (pool.empty()) {
        Job * buf = new Job[ONCE];
        for (size_t i = 0; i < ONCE; i++) {
            pool.push_back(&buf[i]);
        }
        buffers.push_back(buf);
    }

    Job * ret = pool.back();
    pool.pop_back();
    
    return ret;
}

void JobPool::free(Job * job) {
    pool.push_back(job);
}

JobPool::~JobPool() {
    for (auto buf : buffers) {
        delete [] buf;
    }
}
