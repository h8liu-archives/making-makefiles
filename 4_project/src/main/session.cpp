#include "run.h"

void SessionStat(const char * set) {
    o("sessionStat", set, sessionStat,
            "bigdat/timeline", "bigdat/sessionstat");
}

void SessionDurationDists(const char * set) {
    o("sessionDurationDists", set, sessionDurationDists,
            "bigdat/sessionstat", "dat/sessiondurs");
}

void SessionNactionDists(const char * set) {
    o("sessionNactionDists", set, sessionNactionDists,
            "bigdat/sessionstat", "dat/sessionacts");
}

void SessionStartTimeHist(const char * set) {
    o("sessionStartTimeHist", set, sessionStartTimeHist,
            "bigdat/sessionstat", "dat/sessionstarts");
}

void MarkSessions(const char * set) {
    o("markSessions", set, markSessions,
            "bigdat/timeline.old", "bigdat/timeline");
}
