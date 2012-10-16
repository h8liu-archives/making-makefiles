#include "navy.h"

using std::map;

struct UserSession {
    uint64_t curId;
    uint64_t lastAccess;
    uint64_t start;
};

void markSessions(const char * timelineIn, const char * sessionOut) {
    Reader reader(timelineIn, sizeof(Access));
    Writer writer(sessionOut, sizeof(Access));

    map<uint32_t, UserSession> userSessions;
    uint64_t sessionId = 1; // session id starts from 1, 0 is reserved for old accesses

    Access a;

    while (reader.read(&a)) {
        uint32_t user = a.user;
        bool newSession = false;

        auto it = userSessions.find(user);
        if (it == userSessions.end()) { // not found
            newSession = true;
        } else {
            UserSession & s = it->second;
            if (a.time - s.lastAccess > uint64_t(3 * 60 * 1e6)) // too long interval
                newSession = true;
            else if (a.time - s.start > uint64_t(30 * 60 * 1e6))  // too long session
                newSession = true;
            else 
                newSession = false;
        }
        
        if (newSession) {
            UserSession s = { sessionId, a.time, a.time };
            userSessions[user] = s;
            a.session = sessionId;
            
            sessionId++;
        } else {
            UserSession & s = it->second;
            a.session = s.curId;
            s.lastAccess = a.time;
        }

        writer.write(&a);
    }

    writer.close();
}

// marking from access_2
void markSessions2(const char * timelineIn, const char * sessionOut) {
    Reader reader(timelineIn, sizeof(Access_2));
    Writer writer(sessionOut, sizeof(Access));

    map<uint32_t, UserSession> userSessions;
    uint64_t sessionId = 1; // session id starts from 1, 0 is reserved for old accesses

    Access_2 a;
    Access_3 a3;

    while (reader.read(&a)) {
        uint32_t user = a.user;
        bool newSession = false;

        auto it = userSessions.find(user);
        if (it == userSessions.end()) { // not found
            newSession = true;
        } else {
            UserSession & s = it->second;
            if (a.time - s.lastAccess > uint64_t(3 * 60 * 1e6)) // too long interval
                newSession = true;
            else if (a.time - s.start > uint64_t(30 * 60 * 1e6))  // too long session
                newSession = true;
            else 
                newSession = false;
        }
        
        if (newSession) {
            UserSession s = { sessionId, a.time, a.time };
            userSessions[user] = s;
            a3.session = sessionId;
            
            sessionId++;
        } else {
            UserSession & s = it->second;
            a3.session = s.curId;
            s.lastAccess = a.time;
        }
        
        a3.time = a.time;
        a3.user = a.user;
        a3.action = a.action;
        a3.block = a.block;
        writer.write(&a3);
    }

    writer.close();
}
