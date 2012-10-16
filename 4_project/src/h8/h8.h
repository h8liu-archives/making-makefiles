#pragma once

#include <cstdint>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <vector>
#include <map>
#include <deque>

// panic
static inline void panic() { assert(false); }
static inline void panic(const char * s) {
    fprintf(stderr, "%s\n", s); panic(); 
}
static inline void panicIf(bool b, const char *s) {
    if (!b) return; panic(s);
}

// file size
uint64_t fsize(const char * f);
uint64_t fsize(FILE * f);

// a log scaler, using db as unit:
// y = log(x) / log(10) * 10)
const uint64_t DBVALUE_MAX = 192;
static inline uint64_t dbOf(uint64_t x) {
    if (x < 10) return x;
    return uint64_t(log10(double(x) + 0.1) * 10);
}
static inline uint64_t dbMinValue(uint64_t db) {
    if (db > DBVALUE_MAX) return ~0ul;
    if (db < 10) return db;
    return uint64_t(pow(10, double(db) / 10) - .1l) + 1;
}
static inline uint64_t dbMaxValue(uint64_t db) {
    if (db >= DBVALUE_MAX) return ~0ul;
    if (db < 10) return db;
    return dbMinValue(db + 1) - 1;
}
static inline uint64_t dbAlign(uint64_t x) {
    return dbMinValue(dbOf(x));
}

class Bitmap {
    std::vector<uint64_t> dat;
    uint64_t n;
    
    void reserve(uint64_t i);

    public:
    static const uint64_t DEFAULT_SIZE = 1024;
    Bitmap(uint64_t n = DEFAULT_SIZE);
    bool get(uint64_t i);
    void set(uint64_t i, bool b=true);
    void clear(uint64_t i) { set(i, false); }
    void clearAll() { 
        for (auto it = dat.begin(); it != dat.end(); it++) 
            *it = 0; 
    }
    uint64_t size() { return n; }
};

class Line {
    public: static const size_t LINE_MAX = 1024;
    private: char buf[LINE_MAX];

    public:
    Line();
    Line(const char * fmt, ...);
    char * format(const char * fmt, ...);
    char * s() { return buf; }
    char * operator() () { return buf; }
};

void cdf(std::vector<uint64_t> & dat, FILE * fout);

class Counter {
    uint64_t n;
    std::map<uint64_t, uint64_t> dat;

    public:
    Counter();
    void clear() { dat.clear(); n = 0; }
    uint64_t total() { return n; }
    void count(uint64_t x, uint64_t w = 1);
    void fline(FILE * out);
};

class MaxCounter {
    std::map<uint64_t, uint64_t> dat;

    public:
    void clear() { dat.clear(); }
    void count(uint64_t x, uint64_t y);
    void fline(FILE * out);
};

class Scatter {
    int xshift, yshift;
    uint32_t xmax, ymax;
    std::map<uint64_t, uint64_t> points;

    void xmerge();
    void ymerge();
    
    public:
    static const uint32_t DEFAULT_GRIDS = 2048;
    Scatter();
    void reset(uint32_t x=DEFAULT_GRIDS, uint32_t y=DEFAULT_GRIDS);
    void count(uint64_t x, uint64_t y, uint64_t w=1);
    void freport(FILE * out);
};

// external sort
class ExtSorter {
    size_t memSize; 
    size_t elementSize;
    int (*cmp)(const void * a, const void * b);

    uint64_t intSort(FILE * fin, void * buf, uint64_t nbuf, FILE * fout);

    public:
    static const size_t DEFAULT_MEMSIZE = 4ul * 1024 * 1024 * 1024; // 2G
    ExtSorter(size_t elementSize,
            int (*cmp)(const void * a, const void * b),
            size_t memSize = DEFAULT_MEMSIZE);
    
    void sort(const char * in, const char * out);
    void sort(FILE * in, FILE * out);
    bool check(const char * in);
};

class ProgLine {
    uint64_t lastReport, begin;
    FILE * term;
    bool sameLine;

    public:
    ProgLine(bool startNow = true);
    void start();
    void count(uint64_t i, uint64_t n);
    void report(uint64_t i, uint64_t n);
    void reportAt(uint64_t i, uint64_t n, uint64_t t);
};

class DurationStr {
    public: static const size_t DURATIONSTR_SIZE = 20;
    private: char buf[DURATIONSTR_SIZE];

    public:
    const char * operator () (uint64_t t);
};

class Reader {
    FILE * f;
    uint64_t i, n;
    size_t elementSize;
    bool autoClose;
    bool closed;

    public:
    Reader(const char * s, size_t elementSize, bool autoClose=true);
    Reader(FILE * f, size_t elementSize, bool autoClose=true);
    bool read(void * buf);
    size_t batchRead(void * buf, size_t n);
    bool readLast(void * buf);
    void rewind() { ::rewind(f); }
    uint64_t total() { return n; }
    uint64_t count() { return i; }
    void close();
    FILE * cfile() { return f; }
};

static inline void readerProg(Reader & reader, ProgLine & prog) {
    prog.count(reader.count(), reader.total());
}

class Writer {
    FILE * f;
    size_t elementSize;

    public:
    Writer(const char * s, size_t elementSize);
    Writer(FILE * f, size_t elementSize);
    void write(void * buf);
    void close() { fclose(f); }
    FILE * cfile() { return f; }
};

void mkdirs(const char * d);

class IntQueue {
    uint8_t * buf;
    size_t nbuf;
    size_t head;
    size_t count;
    size_t elementSize;

    public:
    IntQueue(size_t elementSize, size_t nbuf);
    bool push(void * e);
    bool pull(void * e);
    size_t size() { return count / elementSize; }
    bool full() { return count == nbuf; }
    bool empty() { return count == 0; }
    void clear();
    void dump(FILE * f);
    void load(FILE * f);
    ~IntQueue();
};

class ExtQueue {
    size_t elementSize;
    size_t nbuf;

    IntQueue headQueue;
    IntQueue tailQueue;
    std::deque<FILE *> fileQueue;

    uint8_t * transfer;

    public:
    ExtQueue(size_t elementSize, size_t nbuf);
    void push(void * e);
    bool pull(void * e);
    ~ExtQueue();
};

// a very simple pseudo-random number generator
class Random {
    static const uint64_t MODULUS = ((uint64_t(1) << 32) - 1);
    static const uint64_t A = 48271;
    
    uint64_t x;

    public:
    Random(uint64_t seed) : x((seed % MODULUS) * A) { }
    uint64_t next() { x = x * A % MODULUS; return x; }
    uint64_t operator() () { return next(); }
    uint64_t max() { return MODULUS; }
};

