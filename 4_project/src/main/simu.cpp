#include "run.h"

void Simulate(const char * setScale, const char * config, double wanBandwidth) {
    Line input("bigdat/s/%s/timeline", setScale);
    Line statDir("bigdat/s/%s/dynamic/%s", setScale, config);
    mkdirs(statDir());

    Server server;
    server.setWanBandwidth(wanBandwidth);
    server.setStatDir(statDir());
    printf("[simulate] %s -> %s\n", input(), statDir());
    server.run(input());
}

void CdfLatency(const char * statDir) {
    Line in;
    Line out;
    
    const char * stats[] = {
        "read_complete",
        "read_response",
        "write_complete",
        "write_response",
    };

    for (auto s : stats) {
        in.format("%s/%s", statDir, s);
        out.format("%s/%s.cdf", statDir, s);
        printf("[cdfLatency] %s -> %s\n", in(), out());
        cdfLatency(in(), out());
    }
}

void CdfLatency(const char * setScale, const char * config) {
    Line statDir("bigdat/s/%s/dynamic/%s", setScale, config);
    CdfLatency(statDir());
}

void CleanCdfLatency(const char * setScale, const char * config) {
    Line statDir("bigdat/s/%s/dynamic/%s", setScale, config);

    const char * stats[] = {
        "read_complete",
        "read_response",
        "write_complete",
        "write_response",
    };

    for (auto s : stats) {
        Line cmd("rm %s/%s", statDir(), s);
        printf("> %s\n", cmd());
        auto ret = system(cmd());
        assert(ret == 0);
    }
}
