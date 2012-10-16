#include "run.h"

static const int steadyBws[] = {
    1, 3, 5,
    10, 30, 50,
    100, 300, 500,
};

void MakeSteady() {
    mkdirs("bigdat/steady");

    for (auto bw: steadyBws) {
        Line path("bigdat/steady/coldrw%d", bw);
        printf("makeDay > %s\n", path());

        double doublebw = double(bw) / 1000;

        SteadyRate emitter;
        emitter.setBandwCost(COLD_READ, doublebw);
        emitter.setBandwCost(COLD_WRITE, doublebw);

        ProgLine progLine;
        emitter.useProg(&progLine);

        emitter.make(path(), 60ul * 60 * 1000000); // 1 hour
    }
}

/*
void simuSteady() {
    mkdirs("dat/simustat/steady");

    for (auto inputBw: steadyBws) {
        Line pathin("bigdat/steady/coldrw%d", inputBw);
        for (auto wanBw: bandwidths) {
            Line statout("dat/simustat/steady/coldrw%d-%d", inputBw, wanBw);
            printf("simulate %s bw=%dmbit/s > %s\n",
                    pathin(), wanBw, statout());
            simulate(pathin(), statout(), double(wanBw) / 1000);
        }
    }
}
*/

