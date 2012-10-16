#include "navy.h"

struct SessionShift {
    uint64_t oldStart;
    uint64_t newStart;
};

using std::vector;

static inline int64_t shiftDelta() {
    // shifting for +- .5 hour
    return int64_t(1e6) * (rand() % (3600) - 1800) 
        + rand() % int64_t(1e6);
}

void shiftSession(const char * input, const char * output) {
    Reader reader(input, sizeof(Access));
    Writer writer(output, sizeof(Access));
    Access a;
    vector<SessionShift> sessions;

    SessionShift newShift = { UINT64_MAX, UINT64_MAX };

    while (reader.read(&a)) {
        auto s = a.session;
        if (sessions.size() <= s) 
            sessions.resize(s + 1, newShift);

        SessionShift & ss = sessions[s];

        if (ss.oldStart == UINT64_MAX) {
            ss.oldStart = a.time;
            ss.newStart = a.time + shiftDelta();
        }

        a.time = a.time - ss.oldStart + ss.newStart;
        
        writer.write(&a);
    }

    writer.close();
}

void countScale(const char * timeline, const char * statOut) {
    Reader reader(timeline, sizeof(Access));
    TraceScale scale;
    bzero(&scale, sizeof(scale));
    ProgLine prog;
    Access a;
    while (reader.read(&a)) {
        if (a.block > scale.nblock) scale.nblock = a.block; 
        if (a.user > scale.nuser) scale.nuser = a.user;
        if (a.session > scale.nsession) scale.nsession = a.session;
        readerProg(reader, prog);
    }

    scale.nblock++;
    scale.nuser++;
    scale.nsession++;

    printf("#block=%ld  #user=%d  #session=%ld\n",
            scale.nblock, scale.nuser, scale.nsession);

    Writer writer(statOut, sizeof(TraceScale));
    writer.write(&scale);
    writer.close();
}

void mergeTrace(const char * scaleFile, const char * userBlockFile,
        const char * trace1, uint64_t trace1Scale, 
        const char * trace2, const char * output) {
    TraceScale scale;
    Reader rScale(scaleFile, sizeof(scale));
    assert(rScale.total() == 1);
    bool ret = rScale.read(&scale);
    assert(ret);
    
    Bitmap sharedBlock(uint64_t(1e9));

    Reader rBlockUser(userBlockFile, sizeof(uint8_t));
    uint64_t bid = 0;
    uint8_t nuser;
    uint64_t userThreshold = scale.nuser / 30;
    if (userThreshold < 3) userThreshold = 3;
    while (rBlockUser.read(&nuser)) {
        if (nuser >= userThreshold) // this block is shared
            sharedBlock.set(bid);
        bid++;
    }
    assert(bid == scale.nblock);

    Reader reader1(trace1, sizeof(Access));
    Reader reader2(trace2, sizeof(Access));
    Writer writer(output, sizeof(Access));

    Access a1;
    Access a2;
    bool r1 = reader1.read(&a1);
    bool r2 = reader2.read(&a2);

    ProgLine prog;
    uint64_t total = reader1.total() + reader2.total();
    uint64_t count = 0;

    while (r1 || r2) {
        bool emitR2 = false;
        if (r1 && !r2) {
            emitR2 = false;
        } else if (r2 && !r1) {
            emitR2 = true;
        } else if (a1.time <= a2.time) {
            assert(r1 && r2);
            emitR2 = false;
        } else {
            assert(r1 && r2);
            assert(a1.time > a2.time);
            emitR2 = true;
        }

        if (!emitR2) {
            writer.write(&a1); // write out directly
            r1 = reader1.read(&a1); // update to next
        } else {
            // a2.time // remains the same
            a2.user += scale.nuser * trace1Scale; // new user
            a2.session += scale.nsession * trace1Scale; // new session
            uint64_t blockBase = a2.block % scale.nblock;
            if (!sharedBlock.get(blockBase)) { // only scale up private block
                a2.block += scale.nblock * trace1Scale;
            }
            writer.write(&a2);
            r2 = reader2.read(&a2);
        }
        prog.count(++count, total);
    }
    
    writer.close();
}
