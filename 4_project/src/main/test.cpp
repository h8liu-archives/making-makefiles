#include "run.h"

static void createTestTimeline() {
    mkdirs("bigdat/s/test/orig");
    Writer writer("bigdat/s/test/orig/timeline", sizeof(Access));

    Access a = { uint64_t(1e6) * 7200, 0, 0, 0, 1 };
    /*
    writer.write(&a);
    a.time = uint64_t(1e6) * 7200 * 2;
    a.action = F_WRITE;
    writer.write(&a);
    */
    
    a.action = 0;
    a.session = 2;
    a.time = uint64_t(1e6) * 7200 * 2;
    a.block = 1;
    writer.write(&a);
    a.time = uint64_t(1e6) * (7200 * 2);
    a.block = 2;
    writer.write(&a);
    a.block = 3;
    writer.write(&a);
    writer.close();

    Writer scaleInfo("bigdat/s/test/scale", sizeof(TraceScale));
    TraceScale scale;
    scale.nblock = 10;
    scale.nsession = 11;
    scale.nuser = 10;
    scaleInfo.write(&scale);
    scaleInfo.close();

    Writer userCount("bigdat/usercount/test", sizeof(uint8_t));
    uint8_t count = 1;
    userCount.write(&count);
    count = 5;
    userCount.write(&count);
    
    for (uint64_t i = 2; i < scale.nblock; i++) {
        // all private blocks
        userCount.write(&count);
    }
    userCount.close();
}

void testScaler() {
    createTestTimeline();
    // createScaleSeed("bigdat/s/test/scaleseeds", 10);

    // construct with timeline and scale info
    ScaleReader reader("bigdat/s/test/orig/timeline", "bigdat/s/test/scale");
    // setup to x4
    reader.setScale(2, "bigdat/s/test/scaleseeds");
    // load block type
    reader.loadBlockType("bigdat/usercount/test");

    // printf("starts simulation now\n");
    
    mkdirs("bigdat/s/test/x4/dynamic");
    Server server;
    server.setStatDir("bigdat/s/test/x4/dynamic");
    server.run(reader);
}

