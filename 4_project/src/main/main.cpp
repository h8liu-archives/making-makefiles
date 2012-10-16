#include "run.h"

int icmp(const void * a, const void * b) {
    uint64_t x = *(const uint64_t *)(a);
    uint64_t y = *(const uint64_t *)(b);
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

void test() {
    FILE * tmp = fopen("temp", "wb");
    // uint64_t i[] = { 5, 4, 3, 2, 1, 0 };
    uint64_t i[] = { 1, 5, 2, 3, 1, 7 };
    // uint64_t i[] = { 1, 2, 3, 4, 5, 6 };
    fwrite(i, sizeof(uint64_t), 6, tmp);
    fclose(tmp);

    ExtSorter sorter(sizeof(uint64_t), icmp, sizeof(uint64_t));
    sorter.sort("temp", "temp.sorted");

    auto sorted = sorter.check("temp");
    printf("temp %s\n", sorted?"is sorted":"is not sorted");
    sorted = sorter.check("temp.sorted");
    printf("temp.sorted %s\n", sorted?"is sorted":"is not sorted");
}

int main(int argc, char ** argv) {
    // test(); return 0;

    srand(time(NULL));

    const char * sets[] = {
        // "test",
        "lair62"
        /*
        "corp",
        "eng",
        "lair62",
        "lair62b",
        "deasna",
        "deasna2",
        */
    };

    struct ScaleInfo {
        const char * scaleName;
        uint64_t scale;
        const char * seed;
    } runScale;

    auto arglen = strlen(argv[1]);
    Line scaleName("%s", argv[1]);
    Line seedName("seed_a");

    if (strcmp(scaleName(), "orig") != 0) {
        seedName.format("seed_%c", argv[1][arglen - 1]);
    }

    assert(argc == 2);
    runScale.scaleName = scaleName();
    runScale.seed = seedName();
    argv[1][arglen - 1] = '\0';

    if (strcmp(scaleName(), "orig") == 0) {
        runScale.scale = 1;
    } else {
        runScale.scale = uint64_t(atoi(&(argv[1][1])));
    }

    printf("[%s] seed=%s scale=%ld\n", runScale.scaleName, runScale.seed,
            runScale.scale);

    struct ConfigInfo {
        const char * configName;
        double bandwidth;
    } configs[] = {
        { "10M", 0.01 }
    };

    for (auto set : sets) {
        // CreateScaleSeed(set);
        ScaleInfo & scale = runScale;
        // for (auto & scale : scales) 
        {
            for (auto & config : configs) {
                SimulateScale(set, 
                        scale.scaleName,
                        scale.scale, 
                        scale.seed,
                        config.configName, 
                        config.bandwidth);
                /*
                Line setScale("%s/%s", set, scale.scaleName);
                CdfLatency(setScale(), config.configName);
                CleanCdfLatency(setScale(), config.configName);
                */
            }
        }
    }

    return 0;
}
