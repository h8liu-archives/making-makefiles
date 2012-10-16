#include "navy.h"

#include <h8/h8.h>

#include <vector>
using std::vector;

static uint64_t ONE_HOUR = uint64_t(1e6 * 3600);
static uint64_t ONE_DAY = ONE_HOUR * 24;
static uint64_t ONE_WEEK = ONE_DAY * 7;
static uint64_t ONE_MONTH = ONE_WEEK * 4;


static void printAgeDist(FILE * fout, vector<uint64_t> & cache,
        uint64_t hour) {

    uint64_t nhour = 0, nday = 0, nweek = 0, nmonth = 0;
    uint64_t nancient = 0;
    uint64_t now = (hour + 1) * ONE_HOUR;
    for (auto it : cache) {
        if (it == ~0ul) continue;
        uint64_t age = now - it;
        if (age < ONE_HOUR) nhour++;
        else if (age < ONE_DAY) nday++;
        else if (age < ONE_WEEK) nweek++;
        else if (age < ONE_MONTH) nmonth++;
        else nancient++;
    }

    fprintf(fout, "%ld %ld %ld %ld %ld %ld\n",
            hour,
            nhour, nday, nweek, nmonth, nancient);
}

void cacheAge(const char * timelineIn, const char * statOut) {
    vector<uint64_t> cache;

    Access a;
    Reader reader(timelineIn, sizeof(Access));

    uint64_t lastHour = ~0ul;
    FILE * fout = fopen(statOut, "w");
    ProgLine prog;

    while (reader.read(&a)) {
        readerProg(reader, prog);
        uint64_t curHour = a.time / ONE_HOUR;
        if (lastHour == ~0ul) lastHour = curHour; // update first

        if (curHour != lastHour) {
            assert(curHour > lastHour);
            while (lastHour < curHour) {
                printAgeDist(fout, cache, lastHour);
                lastHour++;
            }
        }
        
        while (cache.size() <= a.block) {
            cache.push_back(~0ul);
        }

        cache[a.block] = a.time;
    }
    
    if (lastHour != ~0ul)
        printAgeDist(fout, cache, lastHour);

    fclose(fout);
}
