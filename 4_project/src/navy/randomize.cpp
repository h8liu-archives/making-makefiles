#include "navy.h"

static uint64_t rand(uint64_t delta) {
    uint64_t r = ((uint64_t(::rand()) << 31) + uint64_t(::rand())); 
    return r % delta;
}

static void randomize(const char * pathin, const char * pathout) {
    Access a;
    
    // open reader
    Reader reader(pathin, sizeof(Access));
    uint64_t n = reader.total();
    panicIf(n == 0, "input file is empty");

    // prepare timing
    reader.read(&a);
    uint64_t tmin = a.time;
    reader.readLast(&a);
    uint64_t tmax = a.time;
    uint64_t delta = tmax - tmin + 1;

    // write to temp file
    Writer writer(pathout, sizeof(Access));
    reader.rewind();
    while (reader.read(&a)) {
        a.time = tmin + rand(delta);
        writer.write(&a);
    }
    writer.close();
}


void uniformTime(const char * pathin, const char * pathtmp, 
        const char * pathout, bool verbose) {
    srand(time(NULL));
    assert(RAND_MAX == INT32_MAX);

    if (verbose) printf("randomizing...\n");
    // randomize
    randomize(pathin, pathout);

    // sort
    if (verbose) printf("sorting...\n");
    sortIntoTimeline(pathout, pathtmp);

    // reassign actions
    if (verbose) printf("reassigning actions...\n");
    reassignColdActions(pathtmp, pathout);

    // delete temp file
    if (verbose) printf("cleanup temp files...\n");
    char buf[1024];
    sprintf(buf, "rm %s", pathtmp);
    int ret = system(buf);
    panicIf(ret, "delete tempfile");
}
