#pragma once

#include <navy/navy.h>

#include <deque>

enum Step : uint8_t {
    RECVFROM_LAN = 0,
    SENDTO_LAN,
    RECVFROM_WAN,
    SENDTO_WAN,
    WRITE_DISK,
    READ_DISK,

    ACCESS_DISK,
    ACCESS_WAN,

    WRITEBACK_DELAY,

    N_MOD_STEP, // the above are handled by modules

    RESPOND,
    END,

    N_STEP,
};

// context for each access execution
struct Job {
    Access access;
    uint8_t * currentStep;
    uint64_t t;
};

const uint64_t NEVER = UINT64_MAX;
const uint64_t END_OF_TIME = UINT64_MAX - 1;

class JobPool;
class JobQueue {
    public: 
    static const size_t NBUF = size_t(1e9) / 8;
    
    private:
    std::deque<Job *> head;
    std::deque<Job *> tail;
    std::deque<FILE *> files;
    JobPool * pool;
    size_t n;

    public:
    JobQueue();
    void setPool(JobPool * p) { pool = p; }
    void push(Job *);
    Job * pull();
    size_t size() { return n; }
    bool empty() { return head.empty(); }
    Job * front() { return head.front(); }
    ~JobQueue();
};

class Module {
    public:
    virtual void setPool(JobPool * pool) { }
    virtual void push(Job * job) = 0;
    virtual uint64_t nextTime() = 0;
    virtual Job * pop() = 0;

    virtual bool hungry() { return false; }
    virtual JobQueue * queue() { return NULL; }
};

// delay a constant time period
class Delayer : public Module {
    uint64_t delay;
    JobQueue jobs; // since the delay is constant, it is a queue
    
    public:
    Delayer() { delay = 0; }
    void setDelay(uint64_t t) { delay = t; }

    void setPool(JobPool * pool) { jobs.setPool(pool); }
    void push(Job * job) {
        job->t += delay; 
        jobs.push(job); 
    }
    uint64_t nextTime() { 
        if (jobs.empty()) return NEVER;
        return jobs.front()->t;
    }
    // bool empty() { return jobs.empty(); }
    Job * pop() { return jobs.pull(); }
};

class Processor : public Module {
    uint64_t taskTime;
    Job * job;
    JobQueue * q;

    public:
    Processor() { taskTime = 0; job = NULL; }
    void setTaskTime(uint64_t t) { taskTime = t; }
    void setQueue(JobQueue * q) { this->q = q; }
    
    void setPool(JobPool * pool) { /* noop */ }
    void push(Job * job) {
        assert(this->job == NULL);
        job->t += taskTime;
        this->job = job;
    }

    uint64_t nextTime() { return job?(job->t):NEVER; }
    Job * pop() { auto ret = job; job = NULL; return ret; }
    bool hungry() { return job == NULL; }
    JobQueue * queue() { return q; }
};

// a job pool for job allocating
class JobPool {
    std::vector<Job *> buffers;
    std::vector<Job *> pool;

    public:
    Job * alloc();
    void free(Job * job);
    ~JobPool();
};

class TimelineReader {
    public:
    virtual bool read(Access * a) = 0;
    virtual uint64_t count() = 0;
    virtual uint64_t total() = 0;
};

// simulated NFS server
class Server {
    public:
    // default settings
    static const uint64_t DEFAULT_DISK_DELAY = 1000;
    static const uint64_t DEFAULT_WAN_DELAY = 2000;
    static const uint64_t DEFAULT_WRITEBACK_DELAY = uint64_t(5e6);

    static const double DEFAULT_WAN_BANDWIDTH;
    static const double DEFAULT_LAN_BANDWIDTH;
    static const double DEFAULT_DISK_BANDWIDTH;
    
    static const uint64_t DEFAULT_PERIOD = uint64_t(1e6*60*60); // an hour

    static const uint64_t BLOCK_MAX = uint64_t(1e9);

    private:
    JobQueue lanSendQueue, lanRecvQueue;
    JobQueue wanSendQueue, wanRecvQueue;
    Processor wanSender, wanReceiver;
    Processor lanSender, lanReceiver;
    
    JobQueue diskQueue;
    Processor diskProcesser;

    Delayer diskDelay;
    Delayer wanDelay;
    Delayer writeBackDelay;

    std::vector<Module *> modules;
    std::vector<Module *> execMap;

    Bitmap writeBackBuffer;

    JobPool jobPool;

    uint64_t currentPeriod;
    uint64_t periodInterval;
    uint64_t periodCount;

    FILE * fAbsorbedWrites;
    uint64_t nAbsorbedWrites;

    Writer * statReadComplete;
    Writer * statWriteComplete;
    Writer * statReadResponse;
    Writer * statWriteResponse;
    

    Line statDir;
    std::vector<FILE *> statFiles;
    std::vector<Writer *> statWriters;

    void step(Job * job);
    void simulateTo(uint64_t t);
    void finishUp();

    void period(uint64_t curTime);
    void periodFlush();
    void closeStatFiles();

    public:
    Server();
    void useDefaultSettings();

    void setWanBandwidth(double b) {
        wanSender.setTaskTime(moveTime(b));
        wanReceiver.setTaskTime(moveTime(b));
    }

    void setLanBandwidth(double b) {
        lanSender.setTaskTime(moveTime(b));
        lanReceiver.setTaskTime(moveTime(b));
    }

    void setDiskBandwidth(double b) { 
        diskProcesser.setTaskTime(moveTime(b)); 
    }

    void setDiskDelay(uint64_t t) { diskDelay.setDelay(t); }
    void setWanDelay(uint64_t t) { wanDelay.setDelay(t); }
    void setWriteBackDelay(uint64_t t) { writeBackDelay.setDelay(t); }

    void setPeriodInterval(uint64_t i) { periodInterval = i; }
    
    bool feed(TimelineReader & reader);

    void run(TimelineReader & reader);
    void run(const char * timeline);
    void run(ScaleReader & reader);

    void setStatDir(const char * dir);
};

class TimelineFileReader : public TimelineReader {
    Reader reader;

    public:
    TimelineFileReader(const char * timeline) :
        reader(timeline, sizeof(Access)) { }
    bool read(Access * a) { return reader.read(a); }
    uint64_t count() { return reader.count(); }
    uint64_t total() { return reader.total(); }
};

class TimelineScaleReader : public TimelineReader {
    ScaleReader * reader;

    public:
    TimelineScaleReader(ScaleReader * reader) : reader(reader) { }
    bool read(Access * a) { return reader->read(a); }
    uint64_t count() { return reader->count(); }
    uint64_t total() { return reader->total(); }
};

void cdfLatency(const char * flatencies, const char * out);
