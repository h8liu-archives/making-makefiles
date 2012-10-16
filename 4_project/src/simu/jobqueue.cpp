#include "simu.h"

JobQueue::JobQueue() {
    pool = NULL;
    n = 0;
}

void JobQueue::push(Job * job) {
    n++;

    if (files.empty()) {
        if (head.size() < NBUF) {
            head.push_back(job);
            return;
        } 
    }

    assert(tail.size() < NBUF);
    tail.push_back(job);

    if (tail.size() == NBUF) {
        FILE * f = tmpfile();
        for (auto j : tail) {
            auto ret = fwrite(j, sizeof(Job), 1, f);
            assert(ret == 1);
            pool->free(j);
        }
        tail.clear();
        
        fflush(f);
        rewind(f);
        files.push_back(f);
    }
}

Job * JobQueue::pull() {
    if (empty()) {
        assert(files.empty());
        assert(tail.empty());
        return NULL; // we have nothing
    }

    n--; // will always pop one in this case
    Job * ret = head.front();
    head.pop_front();

    if (files.empty()) {
        if (!tail.empty()) {
            // keep head full
            Job * job = tail.front();
            tail.pop_front();

            head.push_back(job);
            assert(head.size() == NBUF);
        }
    } else {
        if (head.empty()) {
            FILE * f = files.front();
            files.pop_front();

            while (head.size() < NBUF) {
                Job * j = pool->alloc();
                auto ret = fread(j, sizeof(Job), 1, f);
                assert(ret == 1);
                head.push_back(j);
            }
            
            // make sure that the file is depleted
            uint8_t t; 
            auto ret = fread(&t, 1, 1, f);
            assert(ret == 0 && feof(f));

            fclose(f); // temp file is deleted here

            assert(head.size() == NBUF);
        }
    }

    return ret;
}

JobQueue::~JobQueue() {
    for (auto f : files) fclose(f);
}
