#include "run.h"

void CreateScaleSeed(const char * set) {
    Line file("bigdat/s/%s/seed_a", set);
    createScaleSeed(file(), 1000);
    file.format("bigdat/s/%s/seed_b", set);
    createScaleSeed(file(), 1000);
    file.format("bigdat/s/%s/seed_c", set);
    createScaleSeed(file(), 1000);
}

void SimulateScale(const char * set,
        const char * scaleName,
        uint64_t scale,
        const char * scaleSeed,
        const char * config,
        double wanBandwidth) {
    Line timeline("bigdat/s/%s/orig/timeline", set);
    Line scaleInfo("bigdat/s/%s/scale", set);
    
    ScaleReader reader(timeline(), scaleInfo());
    Line seedFile("bigdat/s/%s/%s", set, scaleSeed);
    reader.setScale(scale - 1, seedFile());
    Line userCount("bigdat/s/%s/usercount", set);
    reader.loadBlockType(userCount());

    Line statDir("bigdat/s/%s/%s/dynamic/%s", set, scaleName, config);
    mkdirs(statDir());
    
    Server server;
    server.setWanBandwidth(wanBandwidth);
    server.setStatDir(statDir());
    printf("[simulate] %s x %ld -> %s\n", timeline(), scale, statDir());
    server.run(reader);

    Line setScale("%s/%s", set, scaleName);
    CdfLatency(setScale(), config);
    CleanCdfLatency(setScale(), config);
}

