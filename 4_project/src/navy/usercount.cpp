#include "navy.h"

#include <set>

static uint8_t getUserCount(std::set<uint16_t> & users) {
    size_t n = users.size();
    if (n > 255) return 255;
    return uint8_t(n);
}

void userCountPerBlock(const char * pathin, const char * pathout) {
    Reader reader(pathin, sizeof(Access));
    Writer writer(pathout, sizeof(uint8_t));
    uint32_t curBlock = 0;
    std::set<uint16_t> users;

    ProgLine prog;
    Access a;
    while (reader.read(&a)) {
        while (a.block != curBlock) {
            assert(a.block > curBlock);
            uint8_t nuser = getUserCount(users);
            writer.write(&nuser);
            users.clear();
            curBlock++;
        }

        assert(curBlock == a.block);
        users.insert(a.user);
        
        readerProg(reader, prog);
    }
    
    uint8_t nuser = getUserCount(users);
    writer.write(&nuser);
    writer.close();
}

