#include "navy.h"

#include <cmath>
#include <map>

using std::map;

static inline uint64_t nAllActions(UserActionRate & rate) {
    return rate.counts[COLD_READ] +
        rate.counts[COLD_WRITE] + rate.counts[WARM_READ] + 
        rate.counts[WARM_WRITE];
}

static inline uint64_t corrIndex(uint32_t user1, uint32_t user2) {
    if (user1 < user2) {
        uint32_t t = user1;
        user1 = user2;
        user2 = t;
    }

    return (uint64_t(user1) << 32) + user2;
}

struct ActionCorrelation {
    uint64_t userPair;
    double corr;
};

void computeUserActionCorrelation(const char * in, const char * corrOut) {
    Reader reader(in, sizeof(UserActionRate));
    
    UserActionRate rate;
    uint64_t curTime = ~0ul;
    
    map<uint32_t, uint64_t> curActions;
    map<uint64_t, double> corrs;

    while (reader.read(&rate)) {
        if (curTime == ~0ul) {
            curTime = rate.time;
        } else if (curTime != rate.time) {
            curActions.clear();
            curTime = rate.time;
        }

        auto it = curActions.find(rate.user);
        assert(it == curActions.end()); // user must not be inside;

        uint64_t n = nAllActions(rate);
        double dn = double(n);
        curActions[rate.user] = n;
        for (it = curActions.begin(); it != curActions.end(); it++) {
            corrs[corrIndex(it->first, rate.user)] += dn * it->second;
        }
    }

    ActionCorrelation corr;
    Writer writer(corrOut, sizeof(ActionCorrelation));
    
    for (auto it = corrs.begin(); it != corrs.end(); it++) {
        corr.userPair = it->first;
        corr.corr = it->second;
        writer.write(&corr);
    }

    writer.close();
}

static inline void breakIndex(uint64_t ind, uint32_t & user1, 
        uint32_t & user2) {
    user1 = uint32_t(ind >> 32);
    user2 = uint32_t(ind);
}

void normalizeUserActionCorrelation(const char * in, const char * out) {
    Reader reader(in, sizeof(ActionCorrelation));
    
    map<uint64_t, double> corrs;
    ActionCorrelation corr;
    while (reader.read(&corr)) {
        corrs[corr.userPair] = corr.corr;
    }

    map<uint64_t, double> normCorrs;
    for (auto it : corrs) {
        uint32_t u1, u2;
        breakIndex(it.first, u1, u2);
        if (u1 != u2) {
            assert(u1 > u2);
            normCorrs[it.first] = it.second / 
                sqrt(corrs[corrIndex(u1, u1)] * corrs[corrIndex(u2, u2)]);
        } else {
            normCorrs[it.first] = 1.l;
        }
    }

    Writer writer(out, sizeof(ActionCorrelation));
    for (auto it : normCorrs) {
        corr.userPair = it.first;
        corr.corr = it.second;
        writer.write(&corr);
    }
    writer.close();
}

void printUserActionCorrelation(const char * in) {
    Reader reader(in, sizeof(ActionCorrelation));
    ActionCorrelation corr;

    while (reader.read(&corr)) {
        uint32_t u1, u2;
        breakIndex(corr.userPair, u1, u2);
        if (u1 != u2 && corr.corr > 0.1) {
            printf("%d %d : %f\n", u2, u1, corr.corr);
        }
    }
}
