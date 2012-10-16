#pragma once

#include <h8/h8.h>
#include <queue>

enum _Action : uint32_t {
    WARM_READ = 0,
    WARM_WRITE,
    COLD_READ,
    COLD_WRITE,
    
    N_ACTION,
};

const uint32_t F_WRITE = 0x1;
const uint32_t F_COLD = 0x2;
const uint32_t F_REDUNDANT = 0x4;

static inline bool isWrite(uint32_t a) { return a & F_WRITE; }
static inline bool isRead(uint32_t a) { return !isWrite(a); }
static inline bool isRedundant(uint32_t a) { return a & F_REDUNDANT; }
static inline bool isCold(uint32_t a) { return a & F_COLD; }
static inline bool isWarm(uint32_t a) { return !isCold(a); }

const uint32_t F_RESPONDED = 0xf00; // for simulation use

#define Access Access_3

struct Access_3 {
    uint64_t time;
    uint32_t user;
    uint32_t action;
    uint64_t block;
    uint64_t session;
};

struct Access_2 {
    uint64_t time;
    uint32_t user;
    uint32_t action;
    uint64_t block;
};

struct Access_1 {
    uint64_t time;
    uint16_t user;
    uint16_t action;
    uint32_t block;
};

// converters, for Access_x streams
void convertOne2Two(const char * pathIn, const char * pathOut);
void convertTwo2Three(const char * pathIn, const char * pathOut);

static const uint64_t BLOCK_SIZE = 4096;

// bandwidth is in gigabit per sec
static inline uint64_t moveTime(double bandwidth) {
    auto bandwidthInBytePerSec = bandwidth * 1e9 / 8;
    double timeInSec = BLOCK_SIZE / bandwidthInBytePerSec;
    return uint64_t(timeInSec * 1e6);
}

struct ActionRate {
    uint64_t counts[N_ACTION];
    uint64_t nusers;
};

struct UserActionRate {
    uint64_t time;
    uint64_t counts[N_ACTION];
    uint32_t user;
};

// count the action rate based on a specific timeslot
void countActionRate(const char * in, const char * out, uint64_t timeSlot);

// produce the CDF of action rate
void cdfActionRate(const char * in, const char * out);

// reassign cold actions in a timeline
void reassignColdActions(const char * pathin, const char * pathout);

// reassign the timestamps of a timeline from another timeline
// the two timelines must have the same number of accesses
void retime(const char * accessIn, const char * shapeIn, 
        const char * out);

// count the number of users for each block 
void userCountPerBlock(const char * pathin, const char * pathout);

// uniformly reassign the timestamp of a timeline
void uniformTime(const char * pathin, const char * pathtmp, 
            const char * pathout, bool verbose = true);

// produce the scattering of #users vs. #actions over #bursts
void scatterBurstUsers(const char * pathin, const char * statout);




// count action rate per user, similar to countActionRate but for each user
void countUserActionRate(const char * in, const char * out, uint64_t timeSlot);

// count the the time utilization, output the min, max timestamp and the number
// of active timeslots
void countUserTimeUtilization(const char * userActionRateIn, const char * out);

// compute the correlations among users, see if users accesses has synchronized
// patterns
void computeUserActionCorrelation(const char * userActionRateIn, const char *
        corrOut);

// normalize the correlation matrix
void normalizeUserActionCorrelation(const char * corrIn, const char * corrOut);

// print out the user correlation
void printUserActionCorrelation(const char * corrIn); 





// fill in the session field for action timelines, for Access_3
void markSessions(const char * timelineIn, const char * sessionOut);

// same function, but input is Access_2; output is still Access_3
void markSessions2(const char * timelineIn, const char * sessionOut);

struct SessionStat {
    uint64_t sessionId;
    uint64_t timeStart, timeEnd;
    uint64_t actionCounts[N_ACTION];
    uint32_t user;
};

// produce the stats for sessions
void sessionStat(const char * timelineIn, const char * statOut);

// produce the distribution of the durations of sessions
void sessionDurationDists(const char *, const char *);

// produce the distribution of the #actions of sessions
void sessionNactionDists(const char *, const char *);

// produce the histogram of the start time of the sessions
void sessionStartTimeHist(const char * sessionIn, const char * out);





// the distribution of the age of the blocks in the cache
void cacheAge(const char * timelineIn, const char * statOut);

// sort a sequence accesses into a timeline
void sortIntoTimeline(const char * in, const char * out);
void hashSortIntoTimeline(const char * in, const char * out);
bool isTimeline(const char * in);
void sortByBlock(const char * in, const char * out);


struct SteadyPacer {
    uint64_t block;
    uint64_t next;
    uint64_t delta;
};

class SteadyRate {
    SteadyPacer pacer[N_ACTION];
    ProgLine * progLine;

    void reset();

    static const uint64_t ONE_DAY = 24l * 60 * 60 * 1000000;
    static bool autoInc[N_ACTION];

    public:
    SteadyRate();
    void useProg(ProgLine * prog) { progLine = prog; }
    void setBandwCost(uint32_t a, double bandwidth);
    void make(const char * out, uint64_t endTime);
    void makeDay(const char * out) { make(out, ONE_DAY); }
};

// action remark
void remarkAction(const char * in, const char * out);
void remarkAction2(const char * in, const char * out);

// new static analyzer that contains a lot of stuff
void staticAnalyze(const char * traceDir);

// session shuffle
void shiftSession(const char * input, const char * output);
void countScale(const char * input, const char * output);
void mergeTrace(const char * scale, const char * userblock,
        const char * trace1, uint64_t trace1Scale,
        const char * trace2, const char * output);


class SessionIterator;
class SessionIterHolder;
class SessionIterHeap;

// a session queue is created 
class SessionQueue {
    private:
    std::deque<Access *> accesses;
    uint64_t baseCount;
    uint64_t total;
    uint64_t sessionId;
    std::vector<SessionIterator *> pendingIterators;

    public:
    static bool deleteIfEnded(SessionQueue * q) {
        auto ret = q->ended();
        if (ret) delete q; 
        return ret;
    }

    SessionQueue(uint64_t id);
    uint64_t id() { return sessionId; }

    // for the consumer
    Access * get(uint64_t i) const;
    void pop(Access * last);
    bool ended() const { return total == baseCount; }
    bool isLast(uint64_t index) const { 
        assert(index <= total);
        return index == total; 
    }
    void addPendingIterator(SessionIterator * it);

    // for the producer
    void push(Access * a, SessionIterHeap & iterHeap);
    static void close(SessionQueue * q);
};

class SessionIterator {
    private:
    SessionQueue * queue;
    uint64_t timeOffset;
    uint64_t copyId;
    uint64_t index;
    bool last;  // the last iterator need to pop the last
                // and when the session queue is empty
                // it needs to delete the session queue
    
    void reportPending() {
        assert(empty());
        queue->addPendingIterator(this);
    }

    public:
    static bool deleteIfEnded(SessionIterator * it);

    SessionIterator(SessionQueue * queue, uint64_t copy, uint64_t timeOffset);
    void setLast(bool b) { last = b; }
    Access * peak() const { return queue->get(index); }
    bool empty() const { return peak() == NULL; }
    uint64_t nextTime() const;
    void pop(Access * a, SessionIterHeap & iterHeap);
    bool ended() { return queue->isLast(index); }
    bool operator < (const SessionIterator & other) const;
    uint64_t getCopyId() { return copyId; }
    bool isLast() { return last; }
};

// for priority queue
struct SessionIterHolder {
    SessionIterator * iter;
    bool operator < (const SessionIterHolder & other) const {
        return *iter < *(other.iter);
    }
};

struct SessionIterHeap {
    private:
    std::priority_queue<SessionIterHolder> heap;

    public:
    SessionIterator * pop() {
        auto ret = heap.top().iter;
        heap.pop();
        return ret;
    }
    SessionIterator * top() { return heap.top().iter; }
    void push(SessionIterator * it) { 
        SessionIterHolder h; h.iter = it;
        heap.push(h);
    }
    bool empty() { return heap.empty(); }
    size_t size() { return heap.size(); }
};

struct TraceScale {
    uint64_t nblock;
    uint64_t nsession;
    uint32_t nuser;
};

class ScaleReader {
    private:
    Reader reader;
    TraceScale scale;
    std::vector<Random *> shiftGenerators;
    uint64_t inputTime;
    std::map<uint32_t, SessionQueue *> userSessions;
    SessionIterHeap iterHeap;
    Bitmap sharedBlocks;
    Bitmap cachedBlocks;

    uint64_t readerTotal;
    uint64_t readerCount;

    void close();
    void addIterators(SessionQueue * queue);
    uint64_t outputTime() {
        if (iterHeap.empty()) return UINT64_MAX;
        return iterHeap.top()->nextTime();
    }

    void readInput();
    void scaleUpAccess(Access * a, uint64_t copyId);
    bool isPrivateBlock(uint64_t block) { return !sharedBlocks.get(block); }
    void remarkAction(Access * a);

    public:
    ScaleReader(const char * timeline, const char * scaleInfo);
    void setScale(uint64_t nshifts, const char * shiftSeeds);
    void loadBlockType(const char * block);

    bool read(Access * a);

    // for progline
    uint64_t ncopy() { return shiftGenerators.size(); }
    uint64_t count() { return readerCount; }
    uint64_t total() { return readerTotal; }
};

void createScaleSeed(const char * f, uint64_t maxScale);
