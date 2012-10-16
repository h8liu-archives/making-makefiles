#include "navy.h"

using std::map;

void countUserActionRate(const char * in, const char * out, uint64_t timeSlot) {
    Reader reader(in, sizeof(Access));
    Writer writer(out, sizeof(UserActionRate));

    Access a;
    uint64_t curSlot = ~0ul;
    map<uint32_t, UserActionRate> counts;

    while (reader.read(&a)) {
        uint64_t thisSlot = a.time / timeSlot;
        if (curSlot != ~0ul && curSlot != thisSlot) {
            for (auto it = counts.begin(); it != counts.end(); it++) {
                writer.write(&(it->second));
            }

            counts.clear();

            assert(thisSlot > curSlot);
            curSlot = thisSlot;
        } else if (curSlot == ~0ul) {
            curSlot = thisSlot;
        }

        auto pcount = counts.find(a.user);
        
        if (pcount == counts.end()) {
            UserActionRate newRate;
            bzero(&newRate, sizeof(UserActionRate));
            newRate.user = a.user;
            newRate.time = thisSlot;
            counts[a.user] = newRate; // mem copy in

            pcount = counts.find(a.user);
        }

        assert(pcount != counts.end());
        
        pcount->second.counts[a.action]++;
    }

    if (curSlot != ~0ul) {
        for (auto it = counts.begin(); it != counts.end(); it++) {
            writer.write(&(it->second));
        }
    }
    
    writer.close();
}

struct UserTimeUtil {
    uint64_t timeMin, timeMax;
    uint64_t activeCount;
};

void countUserTimeUtilization(const char * in, const char * out) {
    map<uint32_t, UserTimeUtil> counts;

    Reader reader(in, sizeof(UserActionRate));
    UserActionRate rate;

    while (reader.read(&rate)) {
        auto it = counts.find(rate.user);
        if (it == counts.end()) {
            UserTimeUtil util;
            util.timeMin = rate.time;
            util.timeMax = rate.time;
            util.activeCount = 1;
            counts[rate.user] = util;
        } else {
            UserTimeUtil & util = it->second;
            if (rate.time < util.timeMin) util.timeMin = rate.time;
            if (rate.time > util.timeMax) util.timeMax = rate.time;
            util.activeCount++;
        }
    }

    FILE * fout = fopen(out, "w");
    assert(fout);

    for (auto it = counts.begin(); it != counts.end(); it++) {
        fprintf(fout, "%d %ld %ld %ld\n", it->first,
                it->second.timeMin, it->second.timeMax,
                it->second.activeCount);
    }

    fclose(fout);
}
