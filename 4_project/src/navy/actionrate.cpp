#include "navy.h"

#include <set>

void countActionRate(const char * traceIn, const char * countOut, 
        uint64_t timeSlot) {
    Reader reader(traceIn, sizeof(Access));
    Writer writer(countOut, sizeof(ActionRate));
    
    Access a;
    uint64_t curSlot = ~0ul;
    ActionRate rate;
    std::set<uint32_t> users;

    bzero(&rate, sizeof(ActionRate));

    while (reader.read(&a)) {
        uint64_t thisSlot = a.time / timeSlot;
        if (curSlot != thisSlot && curSlot != ~0ul) {
            rate.nusers = uint64_t(users.size());
            writer.write(&rate);

            users.clear();
            bzero(&rate, sizeof(ActionRate));

            assert(thisSlot > curSlot);
            curSlot++;
            while (curSlot < thisSlot) {
                writer.write(&rate); // fill the blanks
                curSlot++;
            }
        } else if (curSlot == ~0ul) {
            curSlot = thisSlot;
        }
        
        rate.counts[a.action]++;
        users.insert(a.user);
    }

    if (curSlot != ~0ul) {
        rate.nusers = uint64_t(users.size());
        writer.write(&rate);
    }

    writer.close();
}
