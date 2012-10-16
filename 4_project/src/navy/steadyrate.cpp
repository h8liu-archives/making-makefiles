#include "navy.h"

bool SteadyRate::autoInc[] = {
    false, // WARM_READ
    false, // WARM_WRITE
    true, // COLD_READ
    true, // COLD_WRITE
    // false, // ABSORBED_WRITE
};

SteadyRate::SteadyRate() {
    for (uint32_t i = 0; i < N_ACTION; i++) {
        pacer[i].delta = 0;
    }
    progLine = NULL;
}

// bandwidth in gigabit per sec
void SteadyRate::setBandwCost(uint32_t a, double bandwidth) {
    pacer[a].delta = (bandwidth <= 0)?0:moveTime(bandwidth);
}

void SteadyRate::make(const char * out, uint64_t endTime) {
    reset();
    Writer writer(out, sizeof(Access));

    while (1) {
        uint32_t nextAction = N_ACTION;
        uint64_t nextTime = endTime;

        for (uint32_t i = 0; i < N_ACTION; i++) {
            SteadyPacer & thisPacer = pacer[i];
            if (thisPacer.delta == 0) continue; // not using
            if (thisPacer.next < nextTime) {
                nextTime = thisPacer.next;
                nextAction = i;
            }
        }

        if (nextTime == endTime) break; // it ends here
        assert(nextAction != N_ACTION);

        SteadyPacer & p = pacer[nextAction];
        
        Access access;
        access.time = p.next;
        access.user = 0;
        access.action = nextAction;
        access.block = p.block;

        writer.write(&access);

        if (autoInc[nextAction]) p.block++;
        p.next += p.delta;

        if (progLine) progLine->count(nextTime, endTime);
    }

    if (progLine) progLine->count(endTime, endTime);

    writer.close();
}


void SteadyRate::reset() {
    for (uint32_t i = 0; i < N_ACTION; i++) {
        SteadyPacer & p = pacer[i];
        p.block = 0;
        p.next = uint64_t(i) + 1; // no racing
    }
}


