#include "navy.h"

using std::vector;

const size_t BLOCK_MAX = uint64_t(1e9);

void remarkAction(const char * in, const char * out) {
    Reader reader(in, sizeof(Access));
    Writer writer(out, sizeof(Access));
    Access a;
    
    Bitmap cache(BLOCK_MAX);
    vector<uint64_t> lastUser;

    uint64_t nAccess = 0;
    uint64_t nRedundant = 0;

    while (reader.read(&a)) {
        nAccess++;
        uint32_t newAction = 0;
        if (!(a.action == COLD_READ || a.action == WARM_READ)) {
            newAction |= F_WRITE;
        }

        if (!cache.get(a.block)) {
            newAction |= F_COLD;
        }
        cache.set(a.block);

        while (lastUser.size() <= a.block) {
            lastUser.push_back(UINT32_MAX);
        }
        if (lastUser[a.block] == a.user) {
            if (!(newAction & F_WRITE)) {
                nRedundant++;
            }
            newAction |= F_REDUNDANT;
        }
        lastUser[a.block] = a.user;
    
        a.action = newAction;
        writer.write(&a);
    }
    writer.close();

    printf("total=%ld redundant=%ld(%.3f%%)\n",
            nAccess, nRedundant, 100. * nRedundant / nAccess);
}

void remarkAction2(const char * in, const char * out) {
    Reader reader(in, sizeof(Access));
    Writer writer(out, sizeof(Access));
    Access a;
    
    Bitmap cache(BLOCK_MAX);
    vector<uint64_t> lastUser;

    uint64_t nAccess = 0;
    uint64_t nRedundant = 0;
    ProgLine prog;

    while (reader.read(&a)) {
        nAccess++;
        uint32_t newAction = 0;
        if (isWrite(a.action)) {
            newAction |= F_WRITE;
        }

        if (!cache.get(a.block)) {
            newAction |= F_COLD;
        }
        cache.set(a.block);

        if (lastUser.size() <= a.block) 
            lastUser.resize(a.block + 1, UINT32_MAX);

        if (lastUser[a.block] == a.user) {
            if (!(newAction & F_WRITE)) {
                nRedundant++;
            }
            newAction |= F_REDUNDANT;
        }
        lastUser[a.block] = a.user;
    
        a.action = newAction;
        writer.write(&a);
        readerProg(reader, prog);
    }
    writer.close();

    printf("total=%ld redundant=%ld(%.3f%%)\n",
            nAccess, nRedundant, 100. * nRedundant / nAccess);
}
