#include "navy.h"

using std::vector;

static uint64_t ONE_HOUR = uint64_t(1e6 * 3600);
static uint64_t ONE_DAY = ONE_HOUR * 24;
static uint64_t ONE_WEEK = ONE_DAY * 7;
static uint64_t ONE_MONTH = ONE_WEEK * 4;

class Module {
    FILE * out;
    protected:
    FILE * f() { return out; }
    public:
    void setPrintTo(FILE * f) { out = f; }
    void close() { fclose(out); }
    virtual void print(uint64_t hour) = 0;
    virtual void work(Access & a) = 0;
};

class CacheAge : public Module {
    vector<uint64_t> blockLastAccess;

    public:
    static Module * make() { return new CacheAge(); }

    void work(Access & a) {
        if (blockLastAccess.size() <= a.block)
            blockLastAccess.resize(a.block + 1, UINT64_MAX);
        blockLastAccess[a.block] = a.time;
    }

    void print(uint64_t hour) {
        uint64_t nhour = 0, nday = 0, nweek = 0, nmonth = 0;
        uint64_t nancient = 0;
        uint64_t now = (hour + 1) * ONE_HOUR;

        for (auto it : blockLastAccess) {
            if (it == UINT64_MAX) continue;
            uint64_t age = now - it;
            if (age < ONE_HOUR) nhour++;
            else if (age < ONE_DAY) nday++;
            else if (age < ONE_WEEK) nweek++;
            else if (age < ONE_MONTH) nmonth++;
            else nancient++;
        }

        fprintf(f(), "%ld %ld %ld %ld %ld %ld\n",
                hour,
                nhour, nday, nweek, nmonth, nancient);
    }
};

class Rater : public Module {
    uint64_t n;

    protected:
    void count(uint64_t w = 1) { n += w; }

    public:
    Rater() { n = 0; }
    
    void print(uint64_t hour) {
        fprintf(f(), "%ld %ld\n", hour, n);
        n = 0;
    }
};

class RateRead : public Rater {
    public:
    static Module * make() { return new RateRead(); }
    void work(Access & a) { 
        if (isRead(a.action)) count();
    }
};

class RateWrite : public Rater {
    public:
    static Module * make() { return new RateWrite(); }
    void work(Access & a) {
        if (isWrite(a.action)) count();
    }
};

class RateWarmRead : public Rater {
    public:
    static Module * make() { return new RateWarmRead(); }
    void work(Access & a) {
        if (isRead(a.action) && isWarm(a.action)) count();
    }
};

class RateRedundantRead : public Rater {
    public:
    static Module * make() { return new RateRedundantRead(); }
    void work(Access & a) {
        if (isRedundant(a.action) && isRead(a.action)) {
            count();
        }
    }
};

class RateSessionStart : public Rater {
    static const uint64_t SESSION_MAX = uint64_t(1e9);
    Bitmap sessionSeen;

    public:
    static Module * make() { return new RateSessionStart(); }

    RateSessionStart() : sessionSeen(SESSION_MAX) { }
    virtual void work(Access & a) {
        assert(a.session < SESSION_MAX);
        if (!sessionSeen.get(a.session)) {
            count();
            sessionSeen.set(a.session);
        }
    }
};

class SecRateHistogram : public Module {
    uint64_t counter[3600];
    
    void clear() {
        bzero(counter, sizeof(uint64_t) * 3600);
    }

    static uint64_t m(uint64_t i) {
        if (i < 256) return i;
        if (i < 2048) return i / 16 * 16;
        return i / 256 * 256;
    }

    protected:
    void count(uint64_t t, uint64_t w = 1) {
        uint64_t sec = t / uint64_t(1e6) % 3600;
        counter[sec] += w;
    }
    
    public:
    SecRateHistogram() { clear(); }

    void print(uint64_t hour) {
        Counter c;
        
        for (auto i : counter) {
            c.count(m(i));
        }

        fprintf(f(), "%ld\n", hour);
        c.fline(f());

        clear();
    }
};

class SecsRead : public SecRateHistogram {
    public:
    static Module * make() { return new SecsRead(); }
    void work(Access & a) {
        if (isRead(a.action)) count(a.time);
    }
};

class SecsWrite : public SecRateHistogram {
    public:
    static Module * make() { return new SecsWrite(); }
    void work(Access & a) {
        if (isWrite(a.action)) count(a.time);
    }
};

class SecsColdRead : public SecRateHistogram {
    public:
    static Module * make() { return new SecsColdRead(); }
    void work(Access & a) {
        if (isRead(a.action) && isCold(a.action)) count(a.time);
    }
};

class SecsActiveUser : public SecRateHistogram {
    static const uint64_t MAX_USER = uint64_t(1e9);
    Bitmap userSeen;
    uint64_t lastSec;

    public:
    static Module * make() { return new SecsActiveUser(); }
    SecsActiveUser() : userSeen(MAX_USER) { lastSec = UINT64_MAX; }
    void work(Access & a) {
        assert(a.user < MAX_USER);
        uint64_t sec = a.time / uint64_t(1e6);
        if (lastSec != sec) {
            userSeen.clearAll();
            lastSec = sec;
        }

        if (!userSeen.get(a.user)) {
            count(a.time, 1); // new user for this second
            userSeen.set(a.user);
        }
    }
};

class RateActiveSecs : public Rater {
    uint64_t lastSec;

    public:
    static Module * make() { return new RateActiveSecs(); }
    RateActiveSecs() { lastSec = UINT64_MAX; }
    void work(Access & a) {
        uint64_t sec = a.time / uint64_t(1e6);
        if (sec != lastSec) { // count new second
            count();
            lastSec = sec;
        }
    }
};

class UserBasedRater : public Module {
    vector<uint64_t> counter;
    bool useDbAlign;

    protected:
    void count(uint32_t user, uint64_t w = 1) {
        if (counter.size() <= user)
            counter.resize(user + 1, 0);

        counter[user] += w;
    }

    protected:
    void setUseDbAlign(bool b) { useDbAlign = b; }

    public:
    UserBasedRater() { useDbAlign = true; }
    void clear() { 
        for (auto it = counter.begin(); it != counter.end(); it++) {
            *it = 0;
        }
    }

    void print(uint64_t hour) {
        uint64_t n = 0;
        for (uint32_t u = 0; u < (uint32_t)(counter.size()); u++) {
            if (counter[u] > 0) n++;
        }
        fprintf(f(), "%ld\n", hour);
        fprintf(f(), "%ld\n", n);
        
        for (uint32_t u = 0; u < (uint32_t)(counter.size()); u++) {
            uint64_t c = counter[u];
            if (c > 0) {
                if (useDbAlign) {
                    c = dbAlign(c);
                }
                fprintf(f(), "%d %ld\n", u, c);
            }
        }

        clear();
    }
};

class UserRead : public UserBasedRater {
    public:
    static Module * make() { return new UserRead(); }
    void work(Access & a) {
        if (isRead(a.action)) count(a.user);
    }
};

class UserWrite : public UserBasedRater {
    public:
    static Module * make() { return new UserWrite(); }
    void work(Access & a) {
        if (isWrite(a.action)) count(a.user);
    }
};

class UserActiveSecs : public UserBasedRater {
    vector<uint64_t> lastSecs;

    public:
    static Module * make() { return new UserActiveSecs(); }
    void work(Access & a) {
        if (lastSecs.size() <= a.user)
            lastSecs.resize(a.user + 1, UINT64_MAX);

        uint64_t sec = a.time / uint64_t(1e6);
        if (sec != lastSecs[a.user]) {
            count(a.user);
            lastSecs[a.user] = sec;
        }
    }
};



class StaticAnalyzer {
    static const uint64_t BLOCK_MAX = uint64_t(1e9) / 8;

    const char * traceDir;
    Line timelineFile;

    vector<Module *> modules;

    void openModules() {
        struct ToOpen {
            Module* (*mod)();
            const char * name;
        } toOpens[] = {
            { CacheAge::make, "cache_age" },
            { RateRead::make, "rate_read" },
            { RateWarmRead::make, "rate_warm_read" },
            { RateRedundantRead::make, "rate_redundant_read" },
            { RateWrite::make, "rate_write" },
            { RateSessionStart::make, "rate_session_start" },
            
            { SecsRead::make, "seconds_read" },
            { SecsWrite::make, "seconds_write" },
            { SecsColdRead::make, "seconds_cold_read" },
            { SecsActiveUser::make, "seconds_active_user" },
        
            { RateActiveSecs::make, "rate_active_secs" },

            { UserRead::make, "user_read" },
            { UserWrite::make, "user_write" },
            { UserActiveSecs::make, "user_active_secs" },
        };
        
        Line folder("%s/static", traceDir);
        mkdirs(folder());

        for (auto o : toOpens) {
            if (!o.mod) continue;
            auto mod = o.mod();

            Line buf("%s/%s", folder(), o.name);
            FILE * f = fopen(buf(), "w");
            assert(f);
            mod->setPrintTo(f);
            modules.push_back(mod);
        }
    }

    void closeModules() {
        for (auto m : modules) {
            m->close();
            delete m;
        }
    }
    void printAll() {
        for (auto m : modules) {
            m->print(lastHour);
        }
    }

    uint64_t lastHour;
    uint64_t curHour;

    public:
    void analyze(const char * traceDir) {
        this->traceDir = traceDir;

        timelineFile.format("%s/timeline", traceDir);
        Reader reader(timelineFile(), sizeof(Access));
        Access a;
        ProgLine prog;
    
        openModules();
        lastHour = UINT64_MAX;
    
        while (reader.read(&a)) {
            curHour = a.time / ONE_HOUR;
            if (lastHour == UINT64_MAX) lastHour = curHour;
            if (curHour != lastHour) {
                assert(curHour > lastHour);
                while (lastHour < curHour) {
                    printAll();
                    lastHour++;
                }
            }
            
            for (auto m : modules) {
                m->work(a);
            }

            readerProg(reader, prog);
        }

        if (lastHour != UINT64_MAX) 
            printAll();

        closeModules();
    }

};

void staticAnalyze(const char * traceDir) {
    StaticAnalyzer a;
    a.analyze(traceDir);
}
