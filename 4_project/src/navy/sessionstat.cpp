#include "navy.h"

using std::map;
using std::vector;

void sessionStat(const char * timelineIn, const char * sessionOut) {
    Reader reader(timelineIn, sizeof(Access));
    Writer writer(sessionOut, sizeof(SessionStat));
    
    // input must be timeline, session for users must not interleave
    map<uint32_t, SessionStat> userStats;

    Access a;
    while (reader.read(&a)) {
        uint32_t user = a.user;

        auto it = userStats.find(user); // user
        if (it == userStats.end()) {
            // new user
            SessionStat newStat;
            newStat.sessionId = a.session;
            newStat.timeStart = a.time;
            newStat.timeEnd = a.time;
            newStat.user = a.user;
            for (uint32_t i = 0; i < N_ACTION; i++) {
                newStat.actionCounts[i] = 0;
            }
            newStat.actionCounts[a.action]++;

            userStats[a.user] = newStat;
        } else {
            SessionStat & st = userStats[a.user];
            assert(st.user == a.user);

            if (st.sessionId == a.session) { // same old session
                assert(a.time >= st.timeStart);
                if (a.time > st.timeEnd) st.timeEnd = a.time;
                st.actionCounts[a.action]++;
            } else { // new session
                // flush the old first
                writer.write(&st);

                // setup the new
                st.sessionId = a.session;
                st.timeStart = a.time;
                st.timeEnd = a.time;
                for (uint32_t i = 0; i < N_ACTION; i++) {
                    st.actionCounts[i] = 0;
                }
                st.actionCounts[a.action]++;
            }

        }
    }

    // flush the rest
    for (auto it : userStats) {
        writer.write(&(it.second));
    }

    writer.close();
}

void sessionDurationDists(const char * sessionIn, const char * out) {
    Reader reader(sessionIn, sizeof(SessionStat));
    
    vector<uint64_t> durations;
    SessionStat stat;
    while (reader.read(&stat)) {
        durations.push_back(stat.timeEnd - stat.timeStart);
    }
    
    FILE * fout = fopen(out, "w");
    assert(fout);
    cdf(durations, fout);
    fclose(fout);
}

void sessionNactionDists(const char * sessionIn, const char * out) {
    Reader reader(sessionIn, sizeof(SessionStat));

    vector<uint64_t> nactions;
    SessionStat stat;
    while (reader.read(&stat)) {
        uint64_t sum = 0;
        for (uint32_t i = 0; i < N_ACTION; i++) {
            sum += stat.actionCounts[i];
        }
        nactions.push_back(sum);
    }

    FILE * fout = fopen(out, "w");
    assert(fout);
    cdf(nactions, fout);
    fclose(fout);
}

void sessionStartTimeHist(const char * sessionIn, const char * out) {
    Reader reader(sessionIn, sizeof(SessionStat));
    SessionStat stat;
    Counter counter;   
    while (reader.read(&stat)) {
        counter.count(stat.timeStart / uint64_t(1e6 * 3600));
    }
    
    FILE * fout = fopen(out, "w");
    counter.fline(fout);
    fclose(fout);
}
