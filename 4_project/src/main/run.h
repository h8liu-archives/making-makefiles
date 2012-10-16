#pragma once

#include <h8/h8.h>
#include <navy/navy.h>
#include <simu/simu.h>

// helper functions
static inline void o(const char * name,
        const char * set, 
        void (*f)(const char *, const char *),
        const char * inDir, const char * outDir) {
    mkdirs(outDir);
    Line in("%s/%s", inDir, set);
    Line out("%s/%s", outDir, set);
    printf("[%s] %s -> %s\n", name, in(), out());
    f(in(), out());
}

// steady access rate trace, for simulator validation
void MakeSteady();

// sessioning
void MarkSessions(const char * set);
void SessionStat(const char * set);
void SessionDurationDists(const char * set);
void SessionNactionDists(const char * set);
void SessionStartTimeHist(const char * set);

// transforming traces
void BlockSort(const char * set);
void UniformTime(const char * set);
void Retime(const char * set);

// sharing degree
void UserCountPerBlock(const char * set);
void ScatterBurstUsers(const char * set);
void ScatterBurstUsers(const char * set, const char * sub);

// entire trace activity
void CountActionRate(const char * set);
void CountActionRate(const char * set, const char * sub);
void ActionCDF(const char * set);
void ActionCDF(const char * set, const char * sub);


// user activity
void CountUserActionRate(const char * set);
void CountUserActionRate(const char * set, const char * sub);
void CountUserTimeUtilization(const char * set);

// user acvitity correlation
void ComputeUserActionCorrelation(const char * set);
void NormalizeUserActionCorrelation(const char * set);
void PrintUserActionCorrelation(const char * set);

// cache structure
void CacheAge(const char * set);

void RemarkAction(const char * set);

void StaticAnalyze(const char * set);

// mirroring and scaling up
void Mirror(const char * set, const char * from, const char * to);
void MirrorClean(const char * set, const char * scale);
void CountScale(const char * set);
void Merge(const char * set, const char * from1, uint64_t from1Scale,
        const char * from2, const char * to);
void IncScale(const char * set, const char * from,
        uint64_t scale, const char * to);
void MirrorRemarkX1(const char * set);

// simulation
void Simulate(const char * setScale, const char * config, double wanBandwidth);

void CdfLatency(const char * statDir);
void CdfLatency(const char * setScale, const char * config);
void CleanCdfLatency(const char * setScale, const char * config);

void CreateScaleSeed(const char * set);

// dynamic scaling
void SimulateScale(const char * set,
        const char * scaleName,
        uint64_t scale,
        const char * scaleSeed,
        const char * configName,
        double wanBandwidth);

// for testing
void testScaler();
